/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 *
 * The adventure route: Pathfinder Maren hands out 10 starting-area quests (Dungeon Set 3, blue extras, a
 * weapon and an off-hand), then one quest per dungeon from the Deadmines to the Shadow Labyrinth (10
 * dungeons, level 20 to 72, two Tier 6 tokens each) and from Utgarde Keep to Utgarde Pinnacle (5 dungeons,
 * 72 to 80, Emblems of Heroism).
 *
 * - Accepting a dungeon quest teleports the player and the group into that dungeon; group members without
 *   the quest get it too.
 * - Killing the last boss credits everyone in the instance and summons Maren next to it.
 * - Turning a quest in raises the level to the next dungeon's level and opens the token exchange.
 * - Mobs in the route dungeons only fight back (Lan.Route.PassiveMobs).
 */

#include "Chat.h"
#include "CharacterDatabase.h"
#include "Creature.h"
#include "DBCStores.h"
#include "Group.h"
#include "Item.h"
#include "LanCommon.h"
#include "Log.h"
#include "Mail.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "StringFormat.h"
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
        uint8 LevelOut;                                 // level after turning the quest in
        std::array<uint32, 2> Bosses;                   // [0] is the quest objective; the quest completes when the
                                                        // last of them dies (Ingvar changes entry, Vazruden and
                                                        // Nazan die separately)
    };

    std::array<RouteDungeon, 15> const RouteDungeons =
    {{
        { 7340011, "The Deadmines",                     36,  78,   25, { 639, 0 } },
        { 7340012, "Shadowfang Keep",                   33,  145,  35, { 4275, 0 } },
        { 7340013, "the Scarlet Monastery Cathedral",   189, 610,  45, { 3977, 3976 } },
        { 7340014, "Uldaman",                           70,  286,  50, { 2748, 0 } },
        { 7340015, "Zul'Farrak",                        209, 924,  55, { 7267, 7797 } },
        { 7340016, "Blackrock Depths",                  230, 1466, 60, { 9019, 0 } },
        { 7340017, "Stratholme",                        329, 2214, 64, { 10440, 0 } },
        { 7340018, "the Hellfire Ramparts",             543, 4150, 68, { 17537, 17536 } },
        { 7340019, "the Underbog",                      546, 4363, 70, { 17882, 0 } },
        { 7340020, "the Shadow Labyrinth",              555, 4407, 72, { 18708, 0 } },
        { 7340031, "Utgarde Keep",                      574, 4745, 74, { 23954, 23980 } },
        { 7340032, "the Nexus",                         576, 4983, 75, { 26723, 0 } },
        { 7340033, "Azjol-Nerub",                       601, 5117, 78, { 29120, 0 } },
        { 7340034, "the Violet Hold",                   608, 5209, 79, { 31134, 0 } },
        { 7340035, "Utgarde Pinnacle",                  575, 4747, 80, { 26861, 0 } }
    }};

    constexpr uint32 QUEST_STRATHOLME = 7340017;            // death knights start here
    constexpr uint32 QUEST_LAST_TBC_DUNGEON = 7340020;      // Shadow Labyrinth: then off to Dalaran
    constexpr uint32 QUEST_LAST_WOTLK_DUNGEON = 7340035;    // Utgarde Pinnacle: then off to Naxxramas
    constexpr uint32 TRIGGER_NAXXRAMAS = 5191;
    constexpr uint8 LEVEL_AFTER_START = 20;
    constexpr uint8 LEVEL_DEATH_KNIGHT = 60;    // Stratholme's level

    // Krasus' Landing, in front of Pathfinder Maren
    constexpr float DALARAN_X = 5822.0f;
    constexpr float DALARAN_Y = 446.5f;
    constexpr float DALARAN_Z = 660.0f;
    constexpr float DALARAN_O = 1.57f;

    // What the starting-area quests and the dungeons before Stratholme would have given a death knight
    constexpr std::array<uint32, 9> DeathKnightGear =
    {
        28350, 27803, 28205, 27475, 27977,  // Dungeon Set 3 plate: Warhelm, Shoulderguards, Breastplate,
                                            // Gauntlets, Legplates of the Bold
        34789, 28375, 28318,                // Bracers of Slaughter, Rubium War-Girdle, Obsidian Clodstompers
        27524                               // Firemaul of Destruction
    };
    constexpr std::array<uint8, 12> DeathKnightTokens =
    {
        LAN_TOKEN_NECK, LAN_TOKEN_BACK, LAN_TOKEN_HANDS, LAN_TOKEN_RING, LAN_TOKEN_SHOULDERS, LAN_TOKEN_TRINKET,
        LAN_TOKEN_WAIST, LAN_TOKEN_RING, LAN_TOKEN_MAIN_HAND, LAN_TOKEN_OFF_HAND, LAN_TOKEN_WRISTS, LAN_TOKEN_TRINKET
    };

    // LanSpecData::T6 slots (WoWSims order) each token can become; rings and trinkets have two
    constexpr std::array<std::array<int8, 2>, LAN_TOKEN_COUNT> TokenSlots =
    {{
        { 0, -1 }, { 1, -1 }, { 2, -1 }, { 3, -1 }, { 4, -1 }, { 5, -1 }, { 6, -1 }, { 7, -1 }, { 8, -1 },
        { 9, -1 }, { 10, 11 }, { 12, 13 }, { 14, -1 }, { 15, -1 }, { 16, -1 }, { -1, -1 }
    }};

    enum GuideGossip : uint32
    {
        SENDER_MENU             = GOSSIP_SENDER_MAIN,
        SENDER_PICK             = GOSSIP_SENDER_MAIN + 1,   // action = token * 10000 + spec * 100 + slot
        SENDER_FREE_SPEC        = GOSSIP_SENDER_MAIN + 2,   // action = spec index

        ACTION_HELLO            = 1,
        ACTION_TELEPORT         = 2,
        ACTION_TOKEN            = 100                       // + token
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

    void RaiseLevel(Player* player, uint8 level)
    {
        level = std::min<uint8>(level, sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
        if (player->GetLevel() >= level)
            return;

        player->GiveLevel(level);
        player->InitTalentForLevel();
        player->SetUInt32Value(PLAYER_XP, 0);
    }

    // Items go to the bags; what doesn't fit arrives by mail (12 items per mail)
    void GiveItems(Player* player, std::vector<uint32> const& itemIds)
    {
        std::vector<uint32> mailed;
        for (uint32 itemId : itemIds)
        {
            ItemPosCountVec dest;
            if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1) == EQUIP_ERR_OK)
            {
                if (Item* item = player->StoreNewItem(dest, itemId, true))
                {
                    player->SendNewItem(item, 1, true, false);
                    continue;
                }
            }

            mailed.push_back(itemId);
        }

        for (std::size_t first = 0; first < mailed.size(); first += MAX_MAIL_ITEMS)
        {
            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
            MailDraft draft("LAN Adventure", "Your bags were full, so the rest of your items came by mail.");
            for (std::size_t i = first; i < std::min(mailed.size(), first + MAX_MAIL_ITEMS); ++i)
            {
                if (Item* item = Item::CreateItem(mailed[i], 1, player))
                {
                    item->SaveToDB(trans);
                    draft.AddItem(item);
                }
            }

            draft.SendMailTo(trans, MailReceiver(player), MailSender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM));
            CharacterDatabase.CommitTransaction(trans);
        }

        if (!mailed.empty())
            ChatHandler(player->GetSession()).SendNotification("Your bags are full: check your mailbox.");
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

    uint32 TokenItem(uint8 token)
    {
        return ITEM_LAN_TOKEN_FIRST + token;
    }

    bool IsTokenItem(uint32 itemId)
    {
        return itemId >= ITEM_LAN_TOKEN_FIRST && itemId < ITEM_LAN_TOKEN_FIRST + LAN_TOKEN_COUNT;
    }

    // The T6 slot a token becomes for this spec: for rings and trinkets the first one the player doesn't own yet
    int8 TokenSlotFor(Player* player, LanSpecData const& spec, uint8 token)
    {
        int8 fallback = -1;
        for (int8 slot : TokenSlots[token])
        {
            if (slot < 0 || !spec.T6[slot])
                continue;

            if (!player->HasItemCount(spec.T6[slot], 1, true))
                return slot;

            if (fallback < 0)
                fallback = slot;
        }
        return fallback;
    }

    bool TokenAllowsSlot(uint8 token, uint8 slot)
    {
        if (token == LAN_TOKEN_FREE_CHOICE)
            return true;

        for (int8 tokenSlot : TokenSlots[token])
            if (tokenSlot == int8(slot))
                return true;
        return false;
    }

    std::string ItemName(uint32 itemId)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        return proto ? proto->Name1 : Acore::StringFormat("item {}", itemId);
    }

    void ShowHello(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);
        player->PrepareQuestMenu(creature->GetGUID());

        if (RouteDungeon const* dungeon = ActiveDungeon(player))
            AddGossipItemFor(player, GOSSIP_ICON_TAXI, Acore::StringFormat("Take me and my group to {}.",
                dungeon->Name), SENDER_MENU, ACTION_TELEPORT);

        for (uint8 token = 0; token < LAN_TOKEN_COUNT; ++token)
            if (player->HasItemCount(TokenItem(token)))
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, Acore::StringFormat("Exchange my {}.",
                    ItemName(TokenItem(token))), SENDER_MENU, ACTION_TOKEN + token);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    void ShowTokenMenu(Player* player, Creature* creature, uint8 token)
    {
        ClearGossipMenuFor(player);
        for (uint8 specIndex = 0; specIndex < LanSpecs.size(); ++specIndex)
        {
            LanSpecData const& spec = LanSpecs[specIndex];
            if (spec.ClassId != player->getClass())
                continue;

            if (token == LAN_TOKEN_FREE_CHOICE)
            {
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, Acore::StringFormat("{} {}", spec.SpecName,
                    spec.ClassName), SENDER_FREE_SPEC, specIndex);
                continue;
            }

            int8 slot = TokenSlotFor(player, spec, token);
            if (slot < 0)
                continue;

            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, Acore::StringFormat("{}: {}", spec.SpecName,
                ItemName(spec.T6[slot])), SENDER_PICK, token * 10000 + specIndex * 100 + slot);
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", SENDER_MENU, ACTION_HELLO);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    // Free choice token, second step: every slot of the spec the player doesn't own yet
    void ShowFreeChoiceMenu(Player* player, Creature* creature, uint8 specIndex)
    {
        ClearGossipMenuFor(player);
        LanSpecData const& spec = LanSpecs[specIndex];
        for (uint8 slot = 0; slot < LAN_GEAR_SLOTS; ++slot)
        {
            uint32 itemId = spec.T6[slot];
            if (!itemId || player->HasItemCount(itemId, 1, true))
                continue;

            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, ItemName(itemId), SENDER_PICK,
                LAN_TOKEN_FREE_CHOICE * 10000 + specIndex * 100 + slot);
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", SENDER_MENU,
            uint32(ACTION_TOKEN) + LAN_TOKEN_FREE_CHOICE);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    // After an exchange: offer the next route quest right away, or go back to the main menu
    void ShowNextStep(Player* player, Creature* creature)
    {
        // Every dungeon gives two tokens: exchange the rest first
        for (uint8 token = 0; token < LAN_TOKEN_COUNT; ++token)
        {
            if (player->HasItemCount(TokenItem(token)))
            {
                ShowTokenMenu(player, creature, token);
                return;
            }
        }

        for (uint32 questId = QUEST_LAN_START_FIRST; questId <= QUEST_LAN_DEATH_KNIGHT; ++questId)
        {
            Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
            if (quest && player->GetQuestStatus(questId) == QUEST_STATUS_NONE && player->CanTakeQuest(quest, false))
            {
                player->PlayerTalkClass->SendQuestGiverQuestDetails(quest, creature->GetGUID(), true);
                return;
            }
        }

        ShowHello(player, creature);
    }

    void ExchangeToken(Player* player, Creature* creature, uint32 action)
    {
        uint8 token = action / 10000;
        uint8 specIndex = (action / 100) % 100;
        uint8 slot = action % 100;
        if (token >= LAN_TOKEN_COUNT || specIndex >= LanSpecs.size() || slot >= LAN_GEAR_SLOTS ||
            !TokenAllowsSlot(token, slot))
            return;

        LanSpecData const& spec = LanSpecs[specIndex];
        uint32 itemId = spec.T6[slot];
        if (spec.ClassId != player->getClass() || !itemId || !player->HasItemCount(TokenItem(token)))
        {
            CloseGossipMenuFor(player);
            return;
        }

        // Store first, so a full bag never costs the token
        if (!player->AddItem(itemId, 1))
        {
            CloseGossipMenuFor(player);
            return;
        }

        player->DestroyItemCount(TokenItem(token), 1, true);
        ShowNextStep(player, creature);
    }

    void GiveDeathKnightStart(Player* player)
    {
        // The starting-area quests and the dungeons before Stratholme count as done, so Stratholme is next
        for (uint32 questId = QUEST_LAN_START_FIRST; questId < QUEST_STRATHOLME; ++questId)
            if (!player->GetQuestRewardStatus(questId))
                player->SetRewardedQuest(questId);

        std::vector<uint32> items(DeathKnightGear.begin(), DeathKnightGear.end());
        for (uint8 token : DeathKnightTokens)
            items.push_back(TokenItem(token));
        GiveItems(player, items);
        RaiseLevel(player, LEVEL_DEATH_KNIGHT);
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
        switch (sender)
        {
            case SENDER_PICK:
                ExchangeToken(player, creature, action);
                return true;
            case SENDER_FREE_SPEC:
                if (action < LanSpecs.size() && LanSpecs[action].ClassId == player->getClass() &&
                    player->HasItemCount(TokenItem(LAN_TOKEN_FREE_CHOICE)))
                    ShowFreeChoiceMenu(player, creature, action);
                else
                    CloseGossipMenuFor(player);
                return true;
            default:
                break;
        }

        if (action == ACTION_TELEPORT)
        {
            CloseGossipMenuFor(player);
            if (RouteDungeon const* dungeon = ActiveDungeon(player))
                TeleportGroupToDungeon(player, *dungeon);
            return true;
        }

        if (action >= ACTION_TOKEN && action < uint32(ACTION_TOKEN) + LAN_TOKEN_COUNT &&
            player->HasItemCount(TokenItem(action - ACTION_TOKEN)))
        {
            ShowTokenMenu(player, creature, action - ACTION_TOKEN);
            return true;
        }

        ShowHello(player, creature);
        return true;
    }

    bool OnQuestAccept(Player* player, Creature* /*creature*/, Quest const* quest) override
    {
        if (!GetLanConfig().RouteEnabled)
            return false;

        if (RouteDungeon const* dungeon = FindDungeonByQuest(quest->GetQuestId()))
            TeleportGroupToDungeon(player, *dungeon);
        return false;
    }

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 /*opt*/) override
    {
        uint32 questId = quest->GetQuestId();
        if (questId == QUEST_LAN_DEATH_KNIGHT)
        {
            GiveDeathKnightStart(player);
            return false;                               // the core offers the Stratholme quest next
        }

        if (questId == QUEST_LAN_START_LAST)
        {
            RaiseLevel(player, LEVEL_AFTER_START);
            return false;                               // the core offers the Deadmines quest next
        }

        RouteDungeon const* dungeon = FindDungeonByQuest(questId);
        if (!dungeon)
            return false;

        RaiseLevel(player, dungeon->LevelOut);

        if (questId == QUEST_LAST_TBC_DUNGEON)
        {
            CloseGossipMenuFor(player);
            ChatHandler(player->GetSession()).SendSysMessage("Welcome to Dalaran! Exchange your token with "
                "Pathfinder Maren at Krasus' Landing, then your Northrend adventure begins.");
            player->TeleportTo(MAP_NORTHREND, DALARAN_X, DALARAN_Y, DALARAN_Z, DALARAN_O);
            return true;
        }

        if (questId == QUEST_LAST_WOTLK_DUNGEON)
        {
            CloseGossipMenuFor(player);
            if (AreaTriggerTeleport const* naxxramas = sObjectMgr->GetAreaTriggerTeleport(TRIGGER_NAXXRAMAS))
            {
                ChatHandler(player->GetSession()).SendSysMessage("You made it to level 80. Naxxramas awaits!");
                player->TeleportTo(naxxramas->target_mapId, naxxramas->target_X, naxxramas->target_Y,
                    naxxramas->target_Z, naxxramas->target_Orientation);
            }
            return true;
        }

        for (uint32 itemId : quest->RewardItemId)
        {
            if (IsTokenItem(itemId) && player->HasItemCount(itemId))
            {
                ShowTokenMenu(player, creature, itemId - ITEM_LAN_TOKEN_FIRST);
                return true;                            // the next quest follows the exchange
            }
        }

        return false;
    }
};

class LanRouteUnitScript : public UnitScript
{
public:
    LanRouteUnitScript() : UnitScript("LanRouteUnitScript", true, { UNITHOOK_ON_UNIT_DEATH }) { }

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

private:
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

    // Quest rewards (Dungeon Set 3, the blue extras, weapons) and the death knight start can be used at any level
    auto noLevelRequirement = [](uint32 itemId)
    {
        if (!itemId)
            return;

        if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId))
            const_cast<ItemTemplate*>(proto)->RequiredLevel = 0;
        else
            LOG_ERROR("module", "mod-lan: route reward item {} does not exist", itemId);
    };

    for (uint32 questId = QUEST_LAN_START_FIRST; questId <= QUEST_LAN_DEATH_KNIGHT; ++questId)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
        {
            LOG_ERROR("module", "mod-lan: route quest {} is missing (mod_lan_route.sql not applied?)", questId);
            continue;
        }

        for (uint32 itemId : quest->RewardChoiceItemId)
            noLevelRequirement(itemId);
        for (uint32 itemId : quest->RewardItemId)
            noLevelRequirement(itemId);
    }

    for (uint32 itemId : DeathKnightGear)
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
