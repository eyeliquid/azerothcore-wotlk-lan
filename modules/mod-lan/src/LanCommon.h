/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#ifndef MOD_LAN_COMMON_H
#define MOD_LAN_COMMON_H

#include "Define.h"
#include "ObjectGuid.h"
#include "SharedDefines.h"

#include <array>
#include <string>
#include <unordered_set>
#include <vector>

class Creature;
class Player;

enum LanRole : uint8
{
    LAN_ROLE_TANK,
    LAN_ROLE_HEALER,
    LAN_ROLE_DPS,
    LAN_ROLE_MAX
};

enum LanStatType : uint8
{
    LAN_STAT_CASTER,
    LAN_STAT_HEALER,
    LAN_STAT_AGILITY,
    LAN_STAT_STRENGTH,
    LAN_STAT_TANK
};

#include "LanGearData.h"
#include "LanTeleData.h"

// Creature templates shipped in data/sql/db-world (keep in sync with the SQL).
enum LanCreatures : uint32
{
    NPC_LAN_OUTFITTER           = 7340001,              // starting-area Tier 6 vendor
    NPC_LAN_BAG_VENDOR          = 7340002,
    NPC_LAN_TELEPORTER          = 7340003,
    NPC_LAN_COMMANDER_VALERIA   = 7340004,
    NPC_LAN_FOOD_VENDOR         = 7340005,
    NPC_LAN_CONSUMABLES_VENDOR  = 7340006,              // starting-area potions, food, elixirs, flasks
    NPC_LAN_GEM_VENDOR          = 7340007,              // starting-area Burning Crusade gems
    NPC_LAN_MOUNT_VENDOR        = 7340008,              // starting areas and Dalaran
    NPC_LAN_DUNGEON_TELEPORTER  = 7340009,              // starting-area dungeon and raid teleporter
    NPC_LAN_PROFESSION_VENDOR   = 7340010,              // + profession index (14)
    NPC_LAN_PROFESSION_TRAINER  = 7340030,              // + profession index (14)
    NPC_LAN_GUIDE               = 7340050,              // adventure route quest giver (also summoned at dungeon bosses)
    NPC_LAN_KILL_CREDIT         = 7340051,              // kill credit of the starting-area quests (never spawned)
    NPC_LAN_SPEC_VENDOR         = 7340100,              // + spec index (33)
    NPC_LAN_FIRST               = 7340001,
    NPC_LAN_LAST                = 7340199
};

// Vendor lists that only exist in memory (ObjectMgr::AddVendorItem with persist = false).
enum LanVirtualVendors : uint32
{
    LAN_VENDOR_T6               = 9200000,              // + spec index
    LAN_VENDOR_BAGS             = 9200100,
    LAN_VENDOR_FOOD             = 9200101,
    LAN_VENDOR_CONSUMABLES      = 9200110,              // + category (potions, food, elixirs, flasks)
    LAN_VENDOR_GEMS             = 9200120,              // + gem color (ITEM_SUBCLASS_GEM_*)
    LAN_VENDOR_MOUNTS           = 9200130,              // + 0 ground, 1 flying
    LAN_VENDOR_T9               = 9200200,              // + spec index * 2 + team
    LAN_VENDOR_PROFESSION       = 9201000               // + profession index * 100 + page
};

// Adventure route quests and token items shipped in data/sql/db-world/mod_lan_route.sql (keep in sync).
enum LanRouteQuests : uint32
{
    QUEST_LAN_START_FIRST       = 7340001,              // 6 starting-area quests
    QUEST_LAN_START_LAST        = 7340006,
    QUEST_LAN_DUNGEON_FIRST     = 7340011,              // 7340011-7340020 classic and TBC, 7340031-7340035 WotLK
    QUEST_LAN_DUNGEON_LAST      = 7340035,
    QUEST_LAN_DEATH_KNIGHT      = 7340041               // death knights join the route at Stratholme
};

enum LanTokens : uint8
{
    LAN_TOKEN_HEAD,
    LAN_TOKEN_NECK,
    LAN_TOKEN_SHOULDERS,
    LAN_TOKEN_BACK,
    LAN_TOKEN_CHEST,
    LAN_TOKEN_WRISTS,
    LAN_TOKEN_HANDS,
    LAN_TOKEN_WAIST,
    LAN_TOKEN_LEGS,
    LAN_TOKEN_FEET,
    LAN_TOKEN_RING,
    LAN_TOKEN_TRINKET,
    LAN_TOKEN_MAIN_HAND,
    LAN_TOKEN_OFF_HAND,
    LAN_TOKEN_RANGED,
    LAN_TOKEN_FREE_CHOICE,
    LAN_TOKEN_COUNT
};

constexpr uint32 ITEM_LAN_TOKEN_FIRST = 7340001;        // + LanTokens

struct LanProfession
{
    char const* Name;
    uint32 SkillId;
};

inline std::array<LanProfession, 14> const LanProfessions =
{{
    { "Alchemy",        SKILL_ALCHEMY },
    { "Blacksmithing",  SKILL_BLACKSMITHING },
    { "Enchanting",     SKILL_ENCHANTING },
    { "Engineering",    SKILL_ENGINEERING },
    { "Herbalism",      SKILL_HERBALISM },
    { "Inscription",    SKILL_INSCRIPTION },
    { "Jewelcrafting",  SKILL_JEWELCRAFTING },
    { "Leatherworking", SKILL_LEATHERWORKING },
    { "Mining",         SKILL_MINING },
    { "Skinning",       SKILL_SKINNING },
    { "Tailoring",      SKILL_TAILORING },
    { "Cooking",        SKILL_COOKING },
    { "First Aid",      SKILL_FIRST_AID },
    { "Fishing",        SKILL_FISHING }
}};

struct LanConfig
{
    bool Enabled = true;
    uint32 StartGold = 150;
    uint32 SkillGain = 4;                               // skill points per skill-up
    float TalentRate = 3.0f;                            // talent points per level
    bool NoResurrectionSickness = true;
    bool AlwaysRested = true;                           // rested XP is topped up to the maximum all the time
    uint32 StartZoneSpawnMultiplier = 3;                // enemy spawns in the starting zones (1 = unchanged)
    uint32 HearthstoneCooldown = 300;                   // seconds (0 keeps the normal 60 minutes)
    bool SoloRaids = true;

    bool AutoLearnClassSpells = true;
    bool AutoLearnQuestSpells = true;
    bool AutoLearnRiding = true;
    bool RidingFromLevel1 = true;                       // all riding skills right away, at any level

    bool HunterPetsAlwaysHappy = true;
    bool AllFlightPaths = true;

    bool PartyAuraEnabled = true;
    uint32 PartyAuraMinPlayers = 3;
    uint32 PartyAuraSpell = 20217;                      // Blessing of Kings (+10% stats)
    float PartyAuraXPBonus = 0.10f;

    uint32 GearPrice = 10000;                           // copper per item at the gear and bag vendors
    uint32 FoodPrice = 0;                               // copper per item at the food and consumables vendors
    uint32 GemPrice = 0;                                // copper per gem at the gem vendor
    uint32 MountPrice = 0;                              // copper per mount at the mount vendor
    bool BlockSellBack = true;

    bool LfgFillEnabled = true;
    uint32 LfgFillDelay = 120;                          // seconds in queue before bots fill
    bool HelperBotsBuff = true;
    float HelperDamageDone = 1.25f;
    float HelperHealingDone = 1.25f;
    float HelperDamageTaken = 0.80f;
    bool HelperFreeForAllLoot = true;

    bool RouteEnabled = true;
    bool RoutePassiveMobs = true;                       // route dungeon mobs only fight back
    uint32 RouteKeeperDespawn = 3600;                   // seconds the guide stays at a killed dungeon boss
};

LanConfig const& GetLanConfig();
void LoadLanConfig();

// Bots
bool LanIsBot(Player const* player);
bool LanIsRealPlayer(Player const* player);
bool LanIsHelperBot(ObjectGuid guid);
uint32 LanRealPlayersOnline();

// Spells
void LanLearnSpells(Player* player);

// Vendors
void LanBuildVendors();
bool LanIsVendorItem(uint32 itemId);
uint32 LanFindEnchantSpell(uint32 enchantId);
uint32 LanFindEnchantItem(uint32 enchantId);

// Adventure route
void LanBuildRoute();

// Starting zones
void LanBuildStartZoneSpawns();

// Helper bots
enum class LanBotRequest : uint8
{
    Fill10,
    Fill25,
    Tank,
    Healer,
    Dps,
    DismissAll
};
std::string LanHandleBotRequest(Player* master, LanBotRequest request);
void LanUpdateBots(uint32 diff);

#endif
