/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 *
 * The teleporter in the capitals and Dalaran, Commander Valeria at Krasus' Landing, and
 * placing the LAN NPCs on the ground when they spawn.
 */

#include "Chat.h"
#include "Creature.h"
#include "LanCommon.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

namespace
{
    enum TeleporterActions : uint32
    {
        ACTION_TELE_MAIN        = 1,
        ACTION_TELE_CATEGORY    = 100,          // + category index
        ACTION_TELE_LOCATION    = 10000         // + category index * 100 + location index
    };

    enum ValeriaActions : uint32
    {
        ACTION_FILL_10          = 1,
        ACTION_FILL_25,
        ACTION_ADD_TANK,
        ACTION_ADD_HEALER,
        ACTION_ADD_DPS,
        ACTION_DISMISS
    };
}

class npc_lan_teleporter : public CreatureScript
{
public:
    npc_lan_teleporter() : CreatureScript("npc_lan_teleporter") { }

    static void ShowCategories(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);
        for (uint32 i = 0; i < LanTeleCategories.size(); ++i)
            AddGossipItemFor(player, GOSSIP_ICON_TAXI, LanTeleCategories[i].Name, GOSSIP_SENDER_MAIN,
                ACTION_TELE_CATEGORY + i);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ShowCategories(player, creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (action >= ACTION_TELE_LOCATION)
        {
            CloseGossipMenuFor(player);
            uint32 category = (action - ACTION_TELE_LOCATION) / 100;
            uint32 index = (action - ACTION_TELE_LOCATION) % 100;
            if (category >= LanTeleCategories.size() || index >= LanTeleCategories[category].Locations.size())
                return true;

            if (player->IsInCombat())
            {
                ChatHandler(player->GetSession()).SendNotification("You are in combat.");
                return true;
            }

            LanTeleLocation const& location = LanTeleCategories[category].Locations[index];
            player->TeleportTo(location.Map, location.X, location.Y, location.Z, location.O);
            return true;
        }

        if (action >= ACTION_TELE_CATEGORY && action < ACTION_TELE_CATEGORY + LanTeleCategories.size())
        {
            uint32 category = action - ACTION_TELE_CATEGORY;
            ClearGossipMenuFor(player);
            std::vector<LanTeleLocation> const& locations = LanTeleCategories[category].Locations;
            for (uint32 i = 0; i < locations.size(); ++i)
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, locations[i].Name, GOSSIP_SENDER_MAIN,
                    ACTION_TELE_LOCATION + category * 100 + i);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, ACTION_TELE_MAIN);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        ShowCategories(player, creature);
        return true;
    }
};

class npc_lan_commander_valeria : public CreatureScript
{
public:
    npc_lan_commander_valeria() : CreatureScript("npc_lan_commander_valeria") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Fill my group to a 10-player raid", GOSSIP_SENDER_MAIN,
            ACTION_FILL_10);
        AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Fill my group to a 25-player raid", GOSSIP_SENDER_MAIN,
            ACTION_FILL_25);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Add a tank", GOSSIP_SENDER_MAIN, ACTION_ADD_TANK);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Add a healer", GOSSIP_SENDER_MAIN, ACTION_ADD_HEALER);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Add a damage dealer", GOSSIP_SENDER_MAIN, ACTION_ADD_DPS);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Dismiss all my bots", GOSSIP_SENDER_MAIN, ACTION_DISMISS,
            "Send all bots in your group away?", 0, false);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        CloseGossipMenuFor(player);

        LanBotRequest request;
        switch (action)
        {
            case ACTION_FILL_10:    request = LanBotRequest::Fill10; break;
            case ACTION_FILL_25:    request = LanBotRequest::Fill25; break;
            case ACTION_ADD_TANK:   request = LanBotRequest::Tank; break;
            case ACTION_ADD_HEALER: request = LanBotRequest::Healer; break;
            case ACTION_ADD_DPS:    request = LanBotRequest::Dps; break;
            case ACTION_DISMISS:    request = LanBotRequest::DismissAll; break;
            default:                return true;
        }

        std::string reply = LanHandleBotRequest(player, request);
        creature->Whisper(reply, LANG_UNIVERSAL, player);
        return true;
    }
};

// The NPC spawns carry approximate heights; put them on the floor below when they enter the map.
class LanNpcGroundScript : public AllCreatureScript
{
public:
    LanNpcGroundScript() : AllCreatureScript("LanNpcGroundScript") { }

    void OnCreatureAddWorld(Creature* creature) override
    {
        if (creature->GetEntry() < NPC_LAN_FIRST || creature->GetEntry() > NPC_LAN_LAST)
            return;

        Position home = creature->GetHomePosition();
        float ground = creature->GetMap()->GetHeight(creature->GetPhaseMask(), home.GetPositionX(), home.GetPositionY(),
            home.GetPositionZ() + 3.0f, true, 10.0f);
        if (ground <= INVALID_HEIGHT || std::fabs(ground - home.GetPositionZ()) < 0.05f)
            return;

        creature->Relocate(home.GetPositionX(), home.GetPositionY(), ground, home.GetOrientation());
        creature->SetHomePosition(home.GetPositionX(), home.GetPositionY(), ground, home.GetOrientation());
    }
};

void AddSC_lan_npcs()
{
    new npc_lan_teleporter();
    new npc_lan_commander_valeria();
    new LanNpcGroundScript();
}
