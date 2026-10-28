/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 *
 * The adventure route of Pathfinder Maren:
 * 1. Four starting-area quests: 10 kills (heirloom weapon), 10 Wildflowers (off-hand), 15 kills (two rings) and
 *    Gorefang, a beast Maren lures to the player (two heirloom trinkets). After that Maren sells the rest of the
 *    Tier 6 gear per class and spec.
 * 2. "Have you been here before?" - "Yes": level 20 and the Deadmines, where the mobs are half as strong and drop
 *    random level 70 gear.
 * 3. "Wasn't that too easy?" - "Yes": level 60 and straight into the Hellfire Ramparts. Afterwards Maren sells
 *    level 80 blue gear.
 * 4. Utgarde Keep, Azjol-Nerub and Drak'Tharon Keep (70 to 80), then off to Dalaran.
 *
 * - Accepting a dungeon quest teleports the player and the group into that dungeon; group members without
 *   the quest get it too.
 * - Killing the last boss credits everyone in the instance and summons Maren next to it.
 * - Mobs in the route dungeons only fight back (Lan.Route.PassiveMobs).
 */

#include "Chat.h"
#include "Creature.h"
#include "Group.h"
#include "LanCommon.h"
#include "Log.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "SpellInfo.h"
#include "StringFormat.h"
#include "TemporarySummon.h"
#include "World.h"
#include "WorldSession.h"

#include <bitset>

namespace
{
    struct RouteDungeon
    {
        uint32 QuestId;
        char const* Name;
        uint32 MapId;
        uint32 EntranceTrigger;                         // areatrigger_teleport ID, the spot inside the entrance
        uint8 LevelOut;                                 // level after turning the quest in (0: unchanged)
        std::array<uint32, 2> Bosses;                   // [0] is the quest objective; the quest completes when the
                                                        // last of them dies (Ingvar changes entry, Vazruden and
                                                        // Nazan die separately)
    };

    std::array<RouteDungeon, 5> const RouteDungeons =
    {{
        { QUEST_LAN_DEADMINES,          "The Deadmines",            36,  78,   0,  { 639, 0 } },
        { QUEST_LAN_RAMPARTS,           "the Hellfire Ramparts",    543, 4150, 70, { 17537, 17536 } },
        { QUEST_LAN_UTGARDE_KEEP,       "Utgarde Keep",             574, 4745, 74, { 23954, 23980 } },
        { QUEST_LAN_AZJOL_NERUB,        "Azjol-Nerub",              601, 5117, 77, { 29120, 0 } },
        { QUEST_LAN_DRAKTHARON_KEEP,    "Drak'Tharon Keep",         600, 4998, 80, { 26632, 27696 } }
    }};

    constexpr uint8 LEVEL_BEEN_HERE = 20;               // "Have you been here before?" - "Yes"
    constexpr uint8 LEVEL_TOO_EASY = 60;                // "Wasn't that a bit too easy?" - "Yes"
    constexpr uint8 LEVEL_DEATH_KNIGHT = 60;
    constexpr uint32 MAP_DEADMINES = 36;
    constexpr float DEADMINES_STRENGTH = 0.5f;          // damage done by the Deadmines mobs; they take 1 / this
    constexpr uint32 BEAST_DESPAWN = 10 * MINUTE * IN_MILLISECONDS;

    // Krasus' Landing, in front of Pathfinder Maren
    constexpr float DALARAN_X = 5822.0f;
    constexpr float DALARAN_Y = 446.5f;
    constexpr float DALARAN_Z = 660.0f;
    constexpr float DALARAN_O = 1.57f;

    enum GuideGossip : uint32
    {
        SENDER_MENU             = GOSSIP_SENDER_MAIN,
        SENDER_T6_SPEC          = GOSSIP_SENDER_MAIN + 1,   // action = spec index
        SENDER_LEVEL80          = GOSSIP_SENDER_MAIN + 2,   // action = LanLevel80Gear category

        ACTION_HELLO            = 1,
        ACTION_TELEPORT         = 2,
        ACTION_BEAST            = 3,
        ACTION_T6               = 4,
        ACTION_LEVEL80          = 5,
        ACTION_BEEN_HERE        = 6,
        ACTION_TOO_EASY         = 7
    };

    std::bitset<1024> _routeMaps;

    RouteDungeon const* FindDungeonByQuest(uint32 questId)
    {
        for (RouteDungeon const& dungeon : RouteDungeons)
            if (dungeon.QuestId == questId)
                return &dungeon;
        return nullptr;
    }

    RouteDungeon const* FindDungeonByMap(uint32 mapId)
    {
        for (RouteDungeon const& dungeon : RouteDungeons)
            if (dungeon.MapId == mapId)
                return &dungeon;
        return nullptr;
    }

    // The dungeon quest in the player's log that isn't done yet
    RouteDungeon const* ActiveDungeon(Player* player)
    {
        for (RouteDungeon const& dungeon : RouteDungeons)
            if (player->GetQuestStatus(dungeon.QuestId) == QUEST_STATUS_INCOMPLETE)
                return &dungeon;
        return nullptr;
    }

    bool HasStartQuest(Player* player)
    {
        for (uint32 questId = QUEST_LAN_START_FIRST; questId <= QUEST_LAN_START_LAST; ++questId)
            if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                return true;
        return false;
    }

    bool Rewarded(Player* player, uint32 questId)
    {
        return player->GetQuestRewardStatus(questId);
    }

    bool CanBuyTier6(Player* player)
    {
        return Rewarded(player, QUEST_LAN_START_BEAST) || Rewarded(player, QUEST_LAN_DEATH_KNIGHT);
    }

    void RaiseLevel(Player* player, uint8 level)
    {
        level = std::min<uint8>(level, sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
        if (!level || player->GetLevel() >= level)
            return;

        player->GiveLevel(level);
        player->InitTalentForLevel();
        player->SetUInt32Value(PLAYER_XP, 0);

        // Weapon skills, defense and the other level-bound skills jump to the new maximum too (not professions
        // or riding); the class spells follow through LanPlayer's level-up hook
        player->UpdateSkillsToMaxSkillsForLevel();
    }

    // The player and every group member on the same map go into the dungeon. Real players without the
    // quest get it as well (when they can take it); bots just come along.
    void TeleportGroupToDungeon(Player* player, RouteDungeon const& dungeon)
    {
        AreaTriggerTeleport const* entrance = sObjectMgr->GetAreaTriggerTeleport(dungeon.EntranceTrigger);
        Quest const* quest = sObjectMgr->GetQuestTemplate(dungeon.QuestId);
        if (!entrance || !quest)
        {
            LOG_ERROR("module", "mod-lan: route dungeon {} has no entrance trigger {} or quest {}", dungeon.Name,
                dungeon.EntranceTrigger, dungeon.QuestId);
            return;
        }

        std::vector<Player*> travellers = { player };
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (member && member != player && member->IsInWorld() && member->GetMapId() == player->GetMapId())
                    travellers.push_back(member);
            }
        }

        for (Player* traveller : travellers)
        {
            if (traveller != player && LanIsRealPlayer(traveller) &&
                traveller->GetQuestStatus(dungeon.QuestId) == QUEST_STATUS_NONE &&
                traveller->CanTakeQuest(quest, false) && traveller->CanAddQuest(quest, false))
                traveller->AddQuestAndCheckCompletion(quest, nullptr);

            if (traveller->IsInCombat())
            {
                ChatHandler(traveller->GetSession()).SendNotification("You are in combat: ask Pathfinder Maren "
                    "to take you to the dungeon later.");
                continue;
            }

            traveller->TeleportTo(entrance->target_mapId, entrance->target_X, entrance->target_Y,
                entrance->target_Z, entrance->target_Orientation);
        }
    }

    // Gorefang comes running at the player from a little way off
    void SummonBeast(Player* player)
    {
        if (player->FindNearestCreature(NPC_LAN_BEAST, 80.0f))
            return;

        Position pos = player->GetNearPosition(20.0f, 0.0f);
        TempSummon* beast = player->SummonCreature(NPC_LAN_BEAST, pos, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, BEAST_DESPAWN);
        if (!beast)
            return;

        beast->SetImmuneToNPC(true);                   // guards and other NPCs leave it to the player
        beast->SetFacingToObject(player);
        beast->AI()->AttackStart(player);
        ChatHandler(player->GetSession()).SendNotification("Gorefang is coming for you!");
    }

    void SendQuestion(Player* player, Creature* creature, uint32 textId, char const* answer, uint32 action)
    {
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, answer, SENDER_MENU, action);
        SendGossipMenuFor(player, textId, creature->GetGUID());
    }

    void ShowHello(Player* player, Creature* creature)
    {
        // After the beast: have you been here before? After the Deadmines: wasn't that too easy?
        if (Rewarded(player, QUEST_LAN_START_BEAST) && player->GetQuestStatus(QUEST_LAN_DEADMINES) == QUEST_STATUS_NONE)
        {
            SendQuestion(player, creature, NPC_TEXT_LAN_BEEN_HERE, "Yes, I have done this many times.",
                ACTION_BEEN_HERE);
            return;
        }

        if (Rewarded(player, QUEST_LAN_DEADMINES) && player->GetQuestStatus(QUEST_LAN_RAMPARTS) == QUEST_STATUS_NONE)
        {
            SendQuestion(player, creature, NPC_TEXT_LAN_TOO_EASY, "Yes, far too easy.", ACTION_TOO_EASY);
            return;
        }

        ClearGossipMenuFor(player);
        player->PrepareQuestMenu(creature->GetGUID());

        if (RouteDungeon const* dungeon = ActiveDungeon(player))
            AddGossipItemFor(player, GOSSIP_ICON_TAXI, Acore::StringFormat("Take me and my group to {}.",
                dungeon->Name), SENDER_MENU, ACTION_TELEPORT);

        if (player->GetQuestStatus(QUEST_LAN_START_BEAST) == QUEST_STATUS_INCOMPLETE &&
            !player->FindNearestCreature(NPC_LAN_BEAST, 80.0f))
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Lure Gorefang back.", SENDER_MENU, ACTION_BEAST);

        if (CanBuyTier6(player))
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Show me the Tier 6 gear for my class.", SENDER_MENU,
                ACTION_T6);

        if (Rewarded(player, QUEST_LAN_RAMPARTS))
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Show me your level 80 gear.", SENDER_MENU, ACTION_LEVEL80);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    void ShowTier6Specs(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);
        for (uint8 specIndex = 0; specIndex < LanSpecs.size(); ++specIndex)
            if (LanSpecs[specIndex].ClassId == player->getClass())
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, Acore::StringFormat("{} {}", LanSpecs[specIndex].SpecName,
                    LanSpecs[specIndex].ClassName), SENDER_T6_SPEC, specIndex);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", SENDER_MENU, ACTION_HELLO);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    void ShowLevel80Categories(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);
        for (uint32 category = 0; category < LanLevel80Gear.size(); ++category)
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, LanLevel80Gear[category].Name, SENDER_LEVEL80, category);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", SENDER_MENU, ACTION_HELLO);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    void OpenVendor(Player* player, Creature* creature, uint32 vendorEntry)
    {
        CloseGossipMenuFor(player);
        player->GetSession()->SendListInventory(creature->GetGUID(), vendorEntry);
    }

    void GiveDeathKnightStart(Player* player)
    {
        // The starting-area quests and the Deadmines count as done: Tier 6 is for sale, the Ramparts are next
        for (uint32 questId = QUEST_LAN_START_FIRST; questId <= QUEST_LAN_DEADMINES; ++questId)
            if (sObjectMgr->GetQuestTemplate(questId) && !player->GetQuestRewardStatus(questId))
                player->SetRewardedQuest(questId);

        RaiseLevel(player, LEVEL_DEATH_KNIGHT);
    }

    bool InDeadmines(Unit const* unit)
    {
        return unit && unit->GetMapId() == MAP_DEADMINES;
    }

    bool IsPlayerSide(Unit const* unit)
    {
        return unit && (unit->IsPlayer() || unit->IsControlledByPlayer() || unit->GetOwnerGUID().IsPlayer());
    }
}

class npc_lan_guide : public CreatureScript
{
public:
    npc_lan_guide() : CreatureScript("npc_lan_guide") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ShowHello(player, creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (sender == SENDER_T6_SPEC)
        {
            if (action < LanSpecs.size() && LanSpecs[action].ClassId == player->getClass() && CanBuyTier6(player))
                OpenVendor(player, creature, LAN_VENDOR_T6_ARMOR + action);
            else
                CloseGossipMenuFor(player);
            return true;
        }

        if (sender == SENDER_LEVEL80)
        {
            if (action < LanLevel80Gear.size() && Rewarded(player, QUEST_LAN_RAMPARTS))
                OpenVendor(player, creature, LAN_VENDOR_LEVEL80 + action);
            else
                CloseGossipMenuFor(player);
            return true;
        }

        switch (action)
        {
            case ACTION_TELEPORT:
                CloseGossipMenuFor(player);
                if (RouteDungeon const* dungeon = ActiveDungeon(player))
                    TeleportGroupToDungeon(player, *dungeon);
                return true;
            case ACTION_BEAST:
                CloseGossipMenuFor(player);
                if (player->GetQuestStatus(QUEST_LAN_START_BEAST) == QUEST_STATUS_INCOMPLETE)
                    SummonBeast(player);
                return true;
            case ACTION_T6:
                if (CanBuyTier6(player))
                    ShowTier6Specs(player, creature);
                else
                    CloseGossipMenuFor(player);
                return true;
            case ACTION_LEVEL80:
                if (Rewarded(player, QUEST_LAN_RAMPARTS))
                    ShowLevel80Categories(player, creature);
                else
                    CloseGossipMenuFor(player);
                return true;
            case ACTION_BEEN_HERE:
                if (Rewarded(player, QUEST_LAN_START_BEAST) &&
                    player->GetQuestStatus(QUEST_LAN_DEADMINES) == QUEST_STATUS_NONE)
                {
                    RaiseLevel(player, LEVEL_BEEN_HERE);
                    if (Quest const* deadmines = sObjectMgr->GetQuestTemplate(QUEST_LAN_DEADMINES))
                    {
                        player->PlayerTalkClass->SendQuestGiverQuestDetails(deadmines, creature->GetGUID(), true);
                        return true;
                    }
                }
                CloseGossipMenuFor(player);
                return true;
            case ACTION_TOO_EASY:
                CloseGossipMenuFor(player);
                if (Rewarded(player, QUEST_LAN_DEADMINES) &&
                    player->GetQuestStatus(QUEST_LAN_RAMPARTS) == QUEST_STATUS_NONE)
                {
                    RaiseLevel(player, LEVEL_TOO_EASY);
                    Quest const* ramparts = sObjectMgr->GetQuestTemplate(QUEST_LAN_RAMPARTS);
                    if (ramparts && player->CanAddQuest(ramparts, true))
                        player->AddQuestAndCheckCompletion(ramparts, creature);     // teleports, see OnQuestAccept
                }
                return true;
            default:
                ShowHello(player, creature);
                return true;
        }
    }

    bool OnQuestAccept(Player* player, Creature* /*creature*/, Quest const* quest) override
    {
        if (!GetLanConfig().RouteEnabled)
            return false;

        if (quest->GetQuestId() == QUEST_LAN_START_BEAST)
            SummonBeast(player);
        else if (RouteDungeon const* dungeon = FindDungeonByQuest(quest->GetQuestId()))
            TeleportGroupToDungeon(player, *dungeon);
        return false;
    }

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 /*opt*/) override
    {
        switch (quest->GetQuestId())
        {
            case QUEST_LAN_DEATH_KNIGHT:
                GiveDeathKnightStart(player);
                return false;                           // the core offers the Ramparts quest next
            case QUEST_LAN_START_BEAST:                 // "Have you been here before?"
            case QUEST_LAN_DEADMINES:                   // "Wasn't that a bit too easy?"
                ShowHello(player, creature);
                return true;
            default:
                break;
        }

        RouteDungeon const* dungeon = FindDungeonByQuest(quest->GetQuestId());
        if (!dungeon)
            return false;

        RaiseLevel(player, dungeon->LevelOut);

        if (quest->GetQuestId() == QUEST_LAN_RAMPARTS)
        {
            ShowLevel80Categories(player, creature);
            return true;
        }

        if (quest->GetQuestId() == QUEST_LAN_DRAKTHARON_KEEP)
        {
            CloseGossipMenuFor(player);
            ChatHandler(player->GetSession()).SendSysMessage("Your adventure is complete. Welcome to Dalaran!");
            player->TeleportTo(MAP_NORTHREND, DALARAN_X, DALARAN_Y, DALARAN_Z, DALARAN_O);
            return true;
        }

        return false;                                   // the core offers the next quest
    }
};

class LanRouteUnitScript : public UnitScript
{
public:
    LanRouteUnitScript() : UnitScript("LanRouteUnitScript", true, {
        UNITHOOK_ON_UNIT_DEATH,
        UNITHOOK_MODIFY_MELEE_DAMAGE,
        UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN,
        UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK
    }) { }

    void OnUnitDeath(Unit* unit, Unit* killer) override
    {
        LanConfig const& cfg = GetLanConfig();
        if (!cfg.Enabled || !cfg.RouteEnabled)
            return;

        Creature* creature = unit->ToCreature();
        if (!creature)
            return;

        if (creature->GetMap()->IsDungeon())
            HandleBossDeath(creature, cfg);
        else
            HandleStartAreaKill(creature, killer);
    }

    // The Deadmines mobs (bosses too) are half as strong: they deal half damage and take double damage
    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
    {
        damage = uint32(damage * DeadminesFactor(target, attacker));
    }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* /*spellInfo*/) override
    {
        damage = int32(damage * DeadminesFactor(target, attacker));
    }

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage,
        SpellInfo const* /*spellInfo*/) override
    {
        damage = uint32(damage * DeadminesFactor(target, attacker));
    }

private:
    static float DeadminesFactor(Unit* target, Unit* attacker)
    {
        if (!InDeadmines(target) || !attacker || !GetLanConfig().RouteEnabled)
            return 1.0f;

        bool playerTarget = IsPlayerSide(target);
        bool playerAttacker = IsPlayerSide(attacker);
        if (playerAttacker && !playerTarget)
            return 1.0f / DEADMINES_STRENGTH;
        if (!playerAttacker && playerTarget)
            return DEADMINES_STRENGTH;
        return 1.0f;
    }

    // Any kill counts for the starting-area quests, for the killer and the group members nearby
    static void HandleStartAreaKill(Creature* creature, Unit* killer)
    {
        if (!killer || creature->IsCritter() || creature->IsTotem() || creature->IsControlledByPlayer())
            return;

        Player* player = killer->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!player)
            return;

        auto credit = [creature](Player* member)
        {
            if (member && HasStartQuest(member) && member->IsAtGroupRewardDistance(creature))
                member->KilledMonsterCredit(NPC_LAN_KILL_CREDIT);
        };

        if (Group* group = player->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
                credit(itr->GetSource());
        }
        else
            credit(player);
    }

    static void HandleBossDeath(Creature* creature, LanConfig const& cfg)
    {
        RouteDungeon const* dungeon = FindDungeonByMap(creature->GetMapId());
        if (!dungeon)
            return;

        auto isBoss = [dungeon](uint32 entry)
        {
            return entry && std::find(dungeon->Bosses.begin(), dungeon->Bosses.end(), entry) != dungeon->Bosses.end();
        };
        if (!isBoss(creature->GetEntry()) && !isBoss(creature->GetOriginalEntry()))
            return;

        // Wait for the last boss of a pair
        for (uint32 other : dungeon->Bosses)
            if (other && other != creature->GetEntry() && creature->FindNearestCreature(other, 250.0f, true))
                return;

        // Everyone in the instance gets the credit, wherever they stand and whoever landed the killing blow
        uint32 questId = dungeon->QuestId;
        uint32 creditEntry = dungeon->Bosses[0];
        creature->GetMap()->DoForAllPlayers([questId, creditEntry](Player* player)
        {
            if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                player->KilledMonsterCredit(creditEntry);
        });

        if (creature->FindNearestCreature(NPC_LAN_GUIDE, 100.0f))
            return;

        Position pos = creature->GetNearPosition(4.0f, 0.0f);
        creature->SummonCreature(NPC_LAN_GUIDE, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),
            pos.GetOrientation() + float(M_PI), TEMPSUMMON_TIMED_DESPAWN, cfg.RouteKeeperDespawn * IN_MILLISECONDS);
    }
};

// Mobs in the route dungeons don't pull themselves: aggressive becomes defensive (they still fight back,
// call for help and follow their scripts). Checked every update, because scripts set the state on reset.
class LanRoutePassiveScript : public AllCreatureScript
{
public:
    LanRoutePassiveScript() : AllCreatureScript("LanRoutePassiveScript") { }

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature->HasReactState(REACT_AGGRESSIVE))
            return;

        uint32 mapId = creature->GetMapId();
        if (mapId >= _routeMaps.size() || !_routeMaps.test(mapId))
            return;

        LanConfig const& cfg = GetLanConfig();
        if (!cfg.Enabled || !cfg.RouteEnabled || !cfg.RoutePassiveMobs)
            return;

        if (creature->IsControlledByPlayer() || creature->GetOwnerGUID().IsPlayer() || creature->GetNpcFlags())
            return;

        creature->SetReactState(REACT_DEFENSIVE);
    }
};

void LanBuildRoute()
{
    LanConfig const& cfg = GetLanConfig();
    if (!cfg.RouteEnabled)
        return;

    _routeMaps.reset();
    if (cfg.RoutePassiveMobs)
        for (RouteDungeon const& dungeon : RouteDungeons)
            _routeMaps.set(dungeon.MapId);

    // Quest rewards and the Deadmines loot can be used at any level (the Tier 6 rings in the ring satchels and the
    // level 80 vendor items lose theirs in LanVendors.cpp)
    auto noLevelRequirement = [](uint32 itemId)
    {
        if (!itemId)
            return;

        if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId))
            const_cast<ItemTemplate*>(proto)->RequiredLevel = 0;
        else
            LOG_ERROR("module", "mod-lan: route item {} does not exist", itemId);
    };

    for (uint32 questId = QUEST_LAN_START_FIRST; questId <= QUEST_LAN_DEATH_KNIGHT; ++questId)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        for (uint32 itemId : quest->RewardChoiceItemId)
            noLevelRequirement(itemId);
        for (uint32 itemId : quest->RewardItemId)
            noLevelRequirement(itemId);
    }

    if (!sObjectMgr->GetQuestTemplate(QUEST_LAN_START_FIRST))
        LOG_ERROR("module", "mod-lan: the route quests are missing (mod_lan_route.sql not applied?)");

    for (uint32 itemId : LanDeadminesLoot)
        noLevelRequirement(itemId);

    // Group members of any level can be taken along
    for (RouteDungeon const& dungeon : RouteDungeons)
    {
        if (!sObjectMgr->GetAreaTriggerTeleport(dungeon.EntranceTrigger))
            LOG_ERROR("module", "mod-lan: entrance trigger {} of {} is missing", dungeon.EntranceTrigger, dungeon.Name);

        if (DungeonProgressionRequirements const* requirement =
            sObjectMgr->GetAccessRequirement(dungeon.MapId, DUNGEON_DIFFICULTY_NORMAL))
            const_cast<DungeonProgressionRequirements*>(requirement)->levelMin = 0;
    }

    LOG_INFO("module", "mod-lan: adventure route with {} dungeons, passive mobs {}", RouteDungeons.size(),
        cfg.RoutePassiveMobs);
}

void AddSC_lan_route()
{
    new npc_lan_guide();
    new LanRouteUnitScript();
    new LanRoutePassiveScript();
}
