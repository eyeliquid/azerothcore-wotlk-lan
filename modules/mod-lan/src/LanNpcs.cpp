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
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "WorldPacket.h"
#include "WorldSession.h"

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

    // The starting-area teleporter offers only the first six categories: the dungeons and raids
    static uint32 CategoryCount(Creature* creature)
    {
        return creature->GetEntry() == NPC_LAN_DUNGEON_TELEPORTER ? LAN_TELE_DUNGEON_CATEGORIES
                                                                   : uint32(LanTeleCategories.size());
    }

    static void ShowCategories(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);
        for (uint32 i = 0; i < CategoryCount(creature); ++i)
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
            if (category >= CategoryCount(creature) || index >= LanTeleCategories[category].Locations.size())
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

        if (action >= ACTION_TELE_CATEGORY && action < ACTION_TELE_CATEGORY + CategoryCount(creature))
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

// The 33 Dalaran quartermasters look like a player of their spec in its Tier 9 set. They have
// UNIT_FLAG2_MIRROR_IMAGE and a player race model (mod_lan_npcs.sql), so the client asks for their
// appearance with CMSG_GET_MIRRORIMAGE_DATA; the answer carries the armor of the asking player's
// faction version. The weapons are their creature_equip_template.
class LanOutfitServerScript : public ServerScript
{
public:
    LanOutfitServerScript() : ServerScript("LanOutfitServerScript", { SERVERHOOK_CAN_PACKET_RECEIVE }) { }

    struct Look
    {
        uint8 Race;
        uint8 Gender;
    };

    static constexpr std::array<Look, 33> Looks =
    {{
        { RACE_NIGHTELF, GENDER_FEMALE },       // Balance Druid
        { RACE_NIGHTELF, GENDER_MALE },         // Feral (Cat) Druid
        { RACE_TAUREN, GENDER_MALE },           // Feral (Bear) Druid
        { RACE_NIGHTELF, GENDER_FEMALE },       // Restoration Druid
        { RACE_HUMAN, GENDER_MALE },            // Blood (DPS) Death Knight
        { RACE_ORC, GENDER_MALE },              // Blood (Tank) Death Knight
        { RACE_BLOODELF, GENDER_MALE },         // Frost Death Knight
        { RACE_UNDEAD_PLAYER, GENDER_MALE },    // Unholy Death Knight
        { RACE_DWARF, GENDER_MALE },            // Beast Mastery Hunter
        { RACE_NIGHTELF, GENDER_FEMALE },       // Marksmanship Hunter
        { RACE_TROLL, GENDER_MALE },            // Survival Hunter
        { RACE_HUMAN, GENDER_FEMALE },          // Arcane Mage
        { RACE_BLOODELF, GENDER_MALE },         // Fire Mage
        { RACE_GNOME, GENDER_MALE },            // Frostfire Mage
        { RACE_HUMAN, GENDER_MALE },            // Frost Mage
        { RACE_BLOODELF, GENDER_FEMALE },       // Holy Paladin
        { RACE_DWARF, GENDER_MALE },            // Protection Paladin
        { RACE_HUMAN, GENDER_MALE },            // Retribution Paladin
        { RACE_DRAENEI, GENDER_FEMALE },        // Discipline Priest
        { RACE_HUMAN, GENDER_FEMALE },          // Holy Priest
        { RACE_UNDEAD_PLAYER, GENDER_FEMALE },  // Shadow Priest
        { RACE_BLOODELF, GENDER_FEMALE },       // Assassination Rogue
        { RACE_HUMAN, GENDER_MALE },            // Combat Rogue
        { RACE_NIGHTELF, GENDER_MALE },         // Subtlety Rogue
        { RACE_DRAENEI, GENDER_MALE },          // Elemental Shaman
        { RACE_ORC, GENDER_MALE },              // Enhancement Shaman
        { RACE_TAUREN, GENDER_FEMALE },         // Restoration Shaman
        { RACE_UNDEAD_PLAYER, GENDER_MALE },    // Affliction Warlock
        { RACE_GNOME, GENDER_FEMALE },          // Demonology Warlock
        { RACE_HUMAN, GENDER_FEMALE },          // Destruction Warlock
        { RACE_HUMAN, GENDER_MALE },            // Arms Warrior
        { RACE_ORC, GENDER_MALE },              // Fury Warrior
        { RACE_DWARF, GENDER_MALE },            // Protection Warrior
    }};

    bool CanPacketReceive(WorldSession* session, WorldPacket const& packet) override
    {
        if (packet.GetOpcode() != CMSG_GET_MIRRORIMAGE_DATA || !session)
            return true;

        Player* player = session->GetPlayer();
        if (!player || packet.size() < sizeof(uint64))
            return true;

        WorldPacket copy(packet);
        ObjectGuid guid;
        copy >> guid;

        Creature* creature = ObjectAccessor::GetCreature(*player, guid);
        if (!creature || creature->GetEntry() < NPC_LAN_SPEC_VENDOR ||
            creature->GetEntry() >= NPC_LAN_SPEC_VENDOR + LanSpecs.size())
            return true;

        uint8 specIndex = creature->GetEntry() - NPC_LAN_SPEC_VENDOR;
        LanSpecData const& spec = LanSpecs[specIndex];
        uint8 team = player->GetTeamId(true) == TEAM_HORDE ? 1 : 0;

        WorldPacket data(SMSG_MIRRORIMAGE_DATA, 68);
        data << guid;
        data << uint32(creature->GetDisplayId());
        data << uint8(Looks[specIndex].Race);
        data << uint8(Looks[specIndex].Gender);
        data << uint8(spec.ClassId);
        data << uint8(0);       // skin
        data << uint8(0);       // face
        data << uint8(1);       // hair style
        data << uint8(1);       // hair color
        data << uint8(0);       // facial hair
        data << uint32(0);      // guild

        // Head, shoulders, shirt, chest, waist, legs, feet, wrists, hands, back, tabard as LanGearData slots
        static constexpr std::array<int8, 11> GearSlots = { 0, 2, -1, 4, 7, 8, 9, 5, 6, 3, -1 };
        for (int8 gearSlot : GearSlots)
        {
            ItemTemplate const* proto = gearSlot >= 0 ? sObjectMgr->GetItemTemplate(spec.T9[team][gearSlot]) : nullptr;
            data << uint32(proto ? proto->DisplayInfoID : 0);
        }

        session->SendPacket(&data);
        return false;
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
    new LanOutfitServerScript();
}
