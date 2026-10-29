/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 *
 * Bot features: helper bot buffs and free-for-all loot, the LAN party aura, Commander Valeria's
 * raid filling and the dungeon finder fill.
 */

#include "Containers.h"
#include "GameTime.h"
#include "Group.h"
#include "LFGMgr.h"
#include "LanCommon.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "StringFormat.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Opcodes.h"

#include <bit>
#include <unordered_map>

namespace
{
    constexpr uint32 BOT_UPDATE_INTERVAL = 1000;
    constexpr uint32 LFG_FILL_INTERVAL = 1000;
    constexpr time_t LFG_FILL_ADD_COOLDOWN = 1;          // one bot is prepared at a time, one per second
    constexpr time_t HELPER_LOGIN_TIMEOUT = 60;
    constexpr uint32 MAX_RAID_SIZE = 40;
    constexpr uint8 DEATH_KNIGHT_MIN_LEVEL = 55;

    std::unordered_set<ObjectGuid> _helperBots;
    uint32 _realPlayers = 0;
    uint32 _botTimer = 0;
    uint32 _lfgTimer = 0;

    struct PendingHelper
    {
        uint8 SpecIndex;
        ObjectGuid Master;
        time_t RequestedAt;
        time_t SeenAt = 0;
    };
    std::unordered_map<ObjectGuid, PendingHelper> _pendingHelpers;

    struct LfgFillEntry
    {
        time_t FirstSeen = 0;
        time_t LastAdd = 0;
        std::vector<std::pair<ObjectGuid, LanRole>> Bots;
    };
    std::unordered_map<ObjectGuid, LfgFillEntry> _lfgFill;

    // Level 80 dungeon finder bots in Tier 9 BiS (Lan.LfgFill.BestGear): at most 10 tanks, 10 healers and
    // 20 damage dealers at a time; the others get the usual random gear
    constexpr uint8 BEST_GEAR_LEVEL = 80;
    constexpr std::array<uint32, LAN_ROLE_MAX> BestGearRoster = { 10, 10, 20 };
    std::unordered_map<ObjectGuid, LanRole> _bestGearBots;

    // Roster places in use: bots that are still queued, in a group or in a dungeon
    uint32 BestGearBotsInUse(LanRole role)
    {
        uint32 count = 0;
        for (auto itr = _bestGearBots.begin(); itr != _bestGearBots.end();)
        {
            Player* bot = ObjectAccessor::FindConnectedPlayer(itr->first);
            bool inUse = bot && (bot->GetGroup() || bot->GetMap()->Instanceable() ||
                sLFGMgr->GetState(itr->first) != lfg::LFG_STATE_NONE);
            if (!inUse)
            {
                itr = _bestGearBots.erase(itr);
                continue;
            }

            if (itr->second == role)
                ++count;
            ++itr;
        }
        return count;
    }

    uint8 RoleToLfgMask(LanRole role)
    {
        switch (role)
        {
            case LAN_ROLE_TANK:   return lfg::PLAYER_ROLE_TANK;
            case LAN_ROLE_HEALER: return lfg::PLAYER_ROLE_HEALER;
            default:              return lfg::PLAYER_ROLE_DAMAGE;
        }
    }

    char const* RoleName(LanRole role)
    {
        switch (role)
        {
            case LAN_ROLE_TANK:   return "tank";
            case LAN_ROLE_HEALER: return "healer";
            default:              return "damage dealer";
        }
    }

    LanRole GetPlayerRole(Player* player)
    {
        if (PlayerbotAI::IsTank(player))
            return LAN_ROLE_TANK;
        if (PlayerbotAI::IsHeal(player))
            return LAN_ROLE_HEALER;
        return LAN_ROLE_DPS;
    }

    // Spec indexes (into LanSpecs) that fill a role, optionally limited to one class.
    std::vector<uint8> SpecsForRole(LanRole role, uint8 level, uint8 classId = 0)
    {
        std::vector<uint8> specs;
        for (uint8 i = 0; i < LanSpecs.size(); ++i)
        {
            LanSpecData const& spec = LanSpecs[i];
            if (spec.Role != role || (classId && spec.ClassId != classId))
                continue;
            if (spec.ClassId == CLASS_DEATH_KNIGHT && level < DEATH_KNIGHT_MIN_LEVEL)
                continue;
            specs.push_back(i);
        }
        return specs;
    }

    // Gives the bot the talents of the spec and gear that suits it.
    void ApplySpec(Player* bot, uint8 specIndex)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
            return;

        LanSpecData const& spec = LanSpecs[specIndex];
        PlayerbotFactory::InitTalentsBySpecNo(bot, spec.BotSpecNo, true);
        sRandomPlayerbotMgr.SetValue(bot, "specNo", spec.BotSpecNo + 1);

        PlayerbotFactory factory(bot, bot->GetLevel(), ITEM_QUALITY_EPIC);
        factory.InitAvailableSpells();
        factory.InitEquipment(false);

        botAI->ResetStrategies();
        bot->SaveToDB(false, false);
    }

    // Hunter pets stay at full happiness (no feeding needed, +25% pet damage)
    void KeepHunterPetsHappy()
    {
        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
        {
            if (!player || !player->IsInWorld() || player->getClass() != CLASS_HUNTER)
                continue;

            Pet* pet = player->GetPet();
            if (!pet || pet->getPetType() != HUNTER_PET || !pet->IsAlive())
                continue;

            uint32 maxHappiness = pet->GetMaxPower(POWER_HAPPINESS);
            if (pet->GetPower(POWER_HAPPINESS) < maxHappiness)
                pet->SetPower(POWER_HAPPINESS, maxHappiness);
        }
    }

    void RefreshHelpersAndAura()
    {
        LanConfig const& cfg = GetLanConfig();

        _helperBots.clear();
        _realPlayers = 0;

        std::vector<Player*> realPlayers;
        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
        {
            if (!player || !player->IsInWorld() || !LanIsRealPlayer(player))
                continue;

            ++_realPlayers;
            realPlayers.push_back(player);
        }

        std::unordered_set<Group*> groups;
        for (Player* player : realPlayers)
            if (Group* group = player->GetGroup())
                groups.insert(group);

        for (Group* group : groups)
        {
            bool hasBot = false;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && LanIsBot(member))
                {
                    _helperBots.insert(member->GetGUID());
                    hasBot = true;
                }
            }

            if (hasBot && cfg.HelperFreeForAllLoot && group->GetLootMethod() != FREE_FOR_ALL)
            {
                group->SetLootMethod(FREE_FOR_ALL);
                group->SendUpdate();
            }
        }

        if (!cfg.PartyAuraEnabled || !cfg.PartyAuraSpell)
            return;

        bool active = _realPlayers >= cfg.PartyAuraMinPlayers;
        for (Player* player : realPlayers)
        {
            if (active)
            {
                if (!player->IsAlive() || player->HasAura(cfg.PartyAuraSpell))
                    continue;

                if (Aura* aura = player->AddAura(cfg.PartyAuraSpell, player))
                {
                    aura->SetMaxDuration(-1);
                    aura->SetDuration(-1);
                }
            }
            else
                player->RemoveAurasDueToSpell(cfg.PartyAuraSpell, player->GetGUID());
        }
    }

    // Logs in an addclass bot of the class of the spec. It joins the master's group on login.
    bool SummonHelper(Player* master, PlayerbotMgr* mgr, uint8 specIndex)
    {
        LanSpecData const& spec = LanSpecs[specIndex];
        auto const& pool = sRandomPlayerbotMgr.addclassCache[
            RandomPlayerbotMgr::GetTeamClassIdx(master->GetTeamId(true) == TEAM_ALLIANCE, spec.ClassId)];

        std::vector<ObjectGuid> candidates;
        for (ObjectGuid const& guid : pool)
            if (!ObjectAccessor::FindConnectedPlayer(guid) && !_pendingHelpers.count(guid))
                candidates.push_back(guid);

        if (candidates.empty())
            return false;

        ObjectGuid picked = Acore::Containers::SelectRandomContainerElement(candidates);
        _pendingHelpers[picked] = { specIndex, master->GetGUID(), GameTime::GetGameTime().count() };
        mgr->AddPlayerBot(picked, master->GetSession()->GetAccountId());
        return true;
    }

    bool SummonHelperForRole(Player* master, PlayerbotMgr* mgr, LanRole role)
    {
        std::vector<uint8> specs = SpecsForRole(role, master->GetLevel());
        Acore::Containers::RandomShuffle(specs);
        for (uint8 specIndex : specs)
            if (SummonHelper(master, mgr, specIndex))
                return true;
        return false;
    }

    void UpdatePendingHelpers()
    {
        time_t now = GameTime::GetGameTime().count();
        for (auto itr = _pendingHelpers.begin(); itr != _pendingHelpers.end();)
        {
            Player* bot = ObjectAccessor::FindPlayer(itr->first);
            if (!bot || !bot->IsInWorld() || !GET_PLAYERBOT_AI(bot))
            {
                if (now - itr->second.RequestedAt > HELPER_LOGIN_TIMEOUT)
                    itr = _pendingHelpers.erase(itr);
                else
                    ++itr;
                continue;
            }

            // Give the bot's own login handling (gear for the master's level, joining the group) a moment
            if (!itr->second.SeenAt)
                itr->second.SeenAt = now;
            if (now - itr->second.SeenAt < 3 || bot->IsInCombat())
            {
                ++itr;
                continue;
            }

            ApplySpec(bot, itr->second.SpecIndex);
            itr = _pendingHelpers.erase(itr);
        }
    }

    // --- Dungeon finder fill -------------------------------------------------------------

    bool IsIdleRandomBot(Player* bot, TeamId team)
    {
        if (!bot || !bot->IsInWorld() || !bot->IsAlive() || bot->IsInCombat() || bot->GetGroup())
            return false;
        if (bot->GetTeamId() != team || bot->IsBeingTeleported() || bot->IsInFlight())
            return false;
        if (bot->InBattleground() || bot->InBattlegroundQueue() || bot->GetMap()->Instanceable())
            return false;
        return sLFGMgr->GetState(bot->GetGUID()) == lfg::LFG_STATE_NONE && GET_PLAYERBOT_AI(bot);
    }

    bool QueueFillBot(Player* leader, LanRole role, lfg::LfgDungeonSet dungeons, LfgFillEntry& entry)
    {
        uint8 level = leader->GetLevel();
        std::vector<std::pair<Player*, uint8>> candidates;
        for (auto const& [guid, bot] : sRandomPlayerbotMgr.GetAllBots())
        {
            if (!IsIdleRandomBot(bot, leader->GetTeamId()))
                continue;

            for (uint8 specIndex : SpecsForRole(role, level, bot->getClass()))
                candidates.emplace_back(bot, specIndex);
        }

        if (candidates.empty())
            return false;

        auto [bot, specIndex] = Acore::Containers::SelectRandomContainerElement(candidates);

        // Level, spells and gear for the dungeon, then the talents and gear of the spec
        PlayerbotFactory factory(bot, level, ITEM_QUALITY_EPIC);
        factory.Randomize(false);
        ApplySpec(bot, specIndex);

        // Level 80: the spec's Tier 9 BiS with gems and enchants, as long as the roster has room
        if (GetLanConfig().LfgFillBestGear && level >= BEST_GEAR_LEVEL &&
            BestGearBotsInUse(role) < BestGearRoster[role] && LanEquipBestGear(bot, specIndex))
            _bestGearBots[bot->GetGUID()] = role;

        sLFGMgr->JoinLfg(bot, RoleToLfgMask(role), dungeons, "");
        if (sLFGMgr->GetState(bot->GetGUID()) != lfg::LFG_STATE_QUEUED)
            return false;

        entry.Bots.emplace_back(bot->GetGUID(), role);
        LOG_INFO("module", "mod-lan: {} queued as {} ({} {}) to fill the group of {}", bot->GetName(), RoleName(role),
            LanSpecs[specIndex].SpecName, LanSpecs[specIndex].ClassName, leader->GetName());
        return true;
    }

    void UpdateLfgFill()
    {
        LanConfig const& cfg = GetLanConfig();
        time_t now = GameTime::GetGameTime().count();

        // Queue entries (a solo player or a group) that contain a real player
        std::unordered_map<ObjectGuid, Player*> queued;
        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
        {
            if (!player || !player->IsInWorld() || !LanIsRealPlayer(player))
                continue;

            if (sLFGMgr->GetState(player->GetGUID()) != lfg::LFG_STATE_QUEUED)
                continue;

            Group* group = player->GetGroup();
            ObjectGuid key = group ? group->GetGUID() : player->GetGUID();
            Player* leader = group ? ObjectAccessor::FindPlayer(group->GetLeaderGUID()) : player;
            queued[key] = leader ? leader : player;
        }

        for (auto itr = _lfgFill.begin(); itr != _lfgFill.end();)
        {
            if (queued.count(itr->first))
            {
                ++itr;
                continue;
            }

            // The real players left the queue or got a dungeon; take unused bots out of the queue
            for (auto const& [botGuid, role] : itr->second.Bots)
                if (sLFGMgr->GetState(botGuid) == lfg::LFG_STATE_QUEUED)
                    sLFGMgr->LeaveLfg(botGuid);
            itr = _lfgFill.erase(itr);
        }

        for (auto const& [key, leader] : queued)
        {
            LfgFillEntry& entry = _lfgFill[key];
            if (!entry.FirstSeen)
                entry.FirstSeen = now;

            if (now - entry.FirstSeen < time_t(cfg.LfgFillDelay) || now - entry.LastAdd < LFG_FILL_ADD_COOLDOWN)
                continue;

            // Roles still open: 1 tank, 1 healer, 3 damage dealers
            std::array<int32, LAN_ROLE_MAX> open = { 1, 1, 3 };
            std::vector<uint8> memberRoles;
            if (key.IsGroup())
            {
                if (Group* group = leader->GetGroup())
                    for (Group::MemberSlot const& slot : group->GetMemberSlots())
                        memberRoles.push_back(sLFGMgr->GetRoles(slot.guid));
            }
            else
                memberRoles.push_back(sLFGMgr->GetRoles(key));

            // Single-role members first, then the flexible ones
            std::sort(memberRoles.begin(), memberRoles.end(), [](uint8 a, uint8 b)
            {
                uint32 rolesA = a & ~lfg::PLAYER_ROLE_LEADER;
                uint32 rolesB = b & ~lfg::PLAYER_ROLE_LEADER;
                return std::popcount(rolesA) < std::popcount(rolesB);
            });
            for (uint8 roles : memberRoles)
            {
                if ((roles & lfg::PLAYER_ROLE_TANK) && open[LAN_ROLE_TANK] > 0)
                    --open[LAN_ROLE_TANK];
                else if ((roles & lfg::PLAYER_ROLE_HEALER) && open[LAN_ROLE_HEALER] > 0)
                    --open[LAN_ROLE_HEALER];
                else if (open[LAN_ROLE_DPS] > 0)
                    --open[LAN_ROLE_DPS];
            }

            entry.Bots.erase(std::remove_if(entry.Bots.begin(), entry.Bots.end(), [](auto const& bot)
            {
                lfg::LfgState state = sLFGMgr->GetState(bot.first);
                return state != lfg::LFG_STATE_QUEUED && state != lfg::LFG_STATE_PROPOSAL;
            }), entry.Bots.end());
            for (auto const& [botGuid, role] : entry.Bots)
                --open[role];

            for (LanRole role : { LAN_ROLE_TANK, LAN_ROLE_HEALER, LAN_ROLE_DPS })
            {
                if (open[role] <= 0)
                    continue;

                lfg::LfgDungeonSet dungeons = sLFGMgr->GetSelectedDungeons(key);
                if (dungeons.empty())
                    dungeons = sLFGMgr->GetSelectedDungeons(leader->GetGUID());
                if (dungeons.empty())
                    break;

                entry.LastAdd = now;
                QueueFillBot(leader, role, dungeons, entry);
                break;
            }
        }
    }
}

bool LanIsBot(Player const* player)
{
    if (!player)
        return false;

    Player* bot = const_cast<Player*>(player);
    return GET_PLAYERBOT_AI(bot) && !IsSelfBot(bot);
}

bool LanIsRealPlayer(Player const* player)
{
    return player && !LanIsBot(player);
}

bool LanIsHelperBot(ObjectGuid guid)
{
    return _helperBots.count(guid) != 0;
}

uint32 LanRealPlayersOnline()
{
    return _realPlayers;
}

std::string LanHandleBotRequest(Player* master, LanBotRequest request)
{
    if (!sPlayerbotAIConfig.Enabled)
        return "The bot system is disabled on this server.";

    PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(master);
    if (!mgr)
        return "You can't command bots right now.";

    Group* group = master->GetGroup();
    if (group && !group->IsLeader(master->GetGUID()))
        return "Only your group leader can call in or dismiss bots.";

    if (group && (group->isLFGGroup() || group->isBGGroup() || group->isBFGroup()))
        return "That doesn't work in a dungeon finder or battleground group.";

    if (master->IsInCombat())
        return "Finish your fight first.";

    if (request == LanBotRequest::DismissAll)
    {
        uint32 dismissed = 0;
        if (group)
        {
            std::vector<ObjectGuid> bots;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                if (Player* member = ref->GetSource(); member && LanIsBot(member))
                    bots.push_back(member->GetGUID());

            for (ObjectGuid const& guid : bots)
            {
                if (mgr->GetPlayerBot(guid))
                    mgr->LogoutPlayerBot(guid);
                else if (Group* current = master->GetGroup())
                    current->RemoveMember(guid);
                ++dismissed;
            }
        }

        for (auto itr = _pendingHelpers.begin(); itr != _pendingHelpers.end();)
        {
            if (itr->second.Master == master->GetGUID())
                itr = _pendingHelpers.erase(itr);
            else
                ++itr;
        }

        return dismissed ? Acore::StringFormat("{} bots dismissed.", dismissed) : "You have no bots with you.";
    }

    // Who is there already (members plus bots that are still logging in)
    std::array<uint32, LAN_ROLE_MAX> have = {};
    uint32 members = 0;
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                ++have[GetPlayerRole(member)];
        members = group->GetMembersCount();
    }
    else
    {
        ++have[GetPlayerRole(master)];
        members = 1;
    }

    for (auto const& [guid, pending] : _pendingHelpers)
    {
        if (pending.Master != master->GetGUID())
            continue;
        ++have[LanSpecs[pending.SpecIndex].Role];
        ++members;
    }

    std::vector<LanRole> toAdd;
    switch (request)
    {
        case LanBotRequest::Fill10:
        case LanBotRequest::Fill25:
        {
            uint32 size = request == LanBotRequest::Fill10 ? 10 : 25;
            std::array<uint32, LAN_ROLE_MAX> target = request == LanBotRequest::Fill10
                ? std::array<uint32, LAN_ROLE_MAX>{ 2, 3, 5 }
                : std::array<uint32, LAN_ROLE_MAX>{ 3, 6, 16 };

            if (members >= size)
                return Acore::StringFormat("Your group already has {} members.", members);

            uint32 room = size - members;
            for (LanRole role : { LAN_ROLE_TANK, LAN_ROLE_HEALER, LAN_ROLE_DPS })
                for (uint32 i = have[role]; i < target[role] && toAdd.size() < room; ++i)
                    toAdd.push_back(role);
            while (toAdd.size() < room)
                toAdd.push_back(LAN_ROLE_DPS);
            break;
        }
        case LanBotRequest::Tank:
            toAdd.push_back(LAN_ROLE_TANK);
            break;
        case LanBotRequest::Healer:
            toAdd.push_back(LAN_ROLE_HEALER);
            break;
        default:
            toAdd.push_back(LAN_ROLE_DPS);
            break;
    }

    if (members + toAdd.size() > MAX_RAID_SIZE)
        return "A raid can't hold more than 40 members.";

    std::array<uint32, LAN_ROLE_MAX> added = {};
    uint32 failed = 0;
    for (LanRole role : toAdd)
    {
        if (SummonHelperForRole(master, mgr, role))
            ++added[role];
        else
            ++failed;
    }

    uint32 total = added[LAN_ROLE_TANK] + added[LAN_ROLE_HEALER] + added[LAN_ROLE_DPS];
    if (!total)
        return "No bots are available right now. Check Playerbots.AddClassAccountPoolSize in playerbots.conf.";

    std::string message = Acore::StringFormat("On their way: {} tank(s), {} healer(s), {} damage dealer(s).",
        added[LAN_ROLE_TANK], added[LAN_ROLE_HEALER], added[LAN_ROLE_DPS]);
    if (failed)
        message += Acore::StringFormat(" {} could not be found.", failed);
    return message;
}

void LanUpdateBots(uint32 diff)
{
    LanConfig const& cfg = GetLanConfig();
    if (!cfg.Enabled)
        return;

    _botTimer += diff;
    if (_botTimer >= BOT_UPDATE_INTERVAL)
    {
        _botTimer = 0;
        RefreshHelpersAndAura();
        if (cfg.HunterPetsAlwaysHappy)
            KeepHunterPetsHappy();
        if (sPlayerbotAIConfig.Enabled)
            UpdatePendingHelpers();
    }

    _lfgTimer += diff;
    if (_lfgTimer >= LFG_FILL_INTERVAL)
    {
        _lfgTimer = 0;
        if (cfg.LfgFillEnabled && sPlayerbotAIConfig.Enabled)
            UpdateLfgFill();
    }
}

class LanBotsWorldScript : public WorldScript
{
public:
    LanBotsWorldScript() : WorldScript("LanBotsWorldScript", { WORLDHOOK_ON_UPDATE }) { }

    void OnUpdate(uint32 diff) override
    {
        LanUpdateBots(diff);
    }
};

class LanHelperUnitScript : public UnitScript
{
public:
    // DealDamage is called on every UnitScript, it has no hook entry
    LanHelperUnitScript() : UnitScript("LanHelperUnitScript", true, { UNITHOOK_MODIFY_HEAL_RECEIVED }) { }

    static bool IsHelper(Unit const* unit)
    {
        if (!unit)
            return false;
        Player const* player = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
        return player && LanIsHelperBot(player->GetGUID());
    }

    uint32 DealDamage(Unit* attacker, Unit* victim, uint32 damage, DamageEffectType /*damageType*/) override
    {
        LanConfig const& cfg = GetLanConfig();
        if (!cfg.Enabled || !cfg.HelperBotsBuff || !damage || _helperBots.empty())
            return damage;

        if (attacker && attacker != victim && IsHelper(attacker))
            damage = uint32(damage * cfg.HelperDamageDone);

        if (IsHelper(victim))
            damage = uint32(damage * cfg.HelperDamageTaken);

        return damage;
    }

    void ModifyHealReceived(Unit* /*target*/, Unit* healer, uint32& heal, SpellInfo const* /*spellInfo*/) override
    {
        LanConfig const& cfg = GetLanConfig();
        if (!cfg.Enabled || !cfg.HelperBotsBuff || _helperBots.empty())
            return;

        if (IsHelper(healer))
            heal = uint32(heal * cfg.HelperHealingDone);
    }
};

// Bots decline a dungeon finder proposal when they are in combat or dead at that moment (mod-playerbots'
// LfgAcceptAction), which happens a lot to random bots that roam the world. A bot's "no" is turned into a "yes":
// it leaves combat (or comes back to life) and accepts, so the group still forms.
class LanLfgAcceptScript : public ServerScript
{
public:
    LanLfgAcceptScript() : ServerScript("LanLfgAcceptScript", { SERVERHOOK_CAN_PACKET_RECEIVE }) { }

    bool CanPacketReceive(WorldSession* session, WorldPacket const& packet) override
    {
        if (packet.GetOpcode() != CMSG_LFG_PROPOSAL_RESULT || !session)
            return true;

        Player* bot = session->GetPlayer();
        if (!bot || !LanIsBot(bot) || !GetLanConfig().Enabled || !GetLanConfig().LfgFillEnabled ||
            packet.size() < sizeof(uint32) + sizeof(uint8))
            return true;

        WorldPacket copy(packet);
        uint32 proposalId;
        bool accept;
        copy >> proposalId >> accept;
        if (accept)
            return true;

        if (bot->isDead())
        {
            bot->ResurrectPlayer(1.0f);
            bot->SpawnCorpseBones();
        }
        bot->CombatStopWithPets(true);

        sLFGMgr->UpdateProposal(proposalId, bot->GetGUID(), true);
        LOG_DEBUG("module", "mod-lan: {} accepted dungeon finder proposal {} instead of declining", bot->GetName(),
            proposalId);
        return false;
    }
};

void AddSC_lan_bots()
{
    new LanBotsWorldScript();
    new LanHelperUnitScript();
    new LanLfgAcceptScript();
}
