/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 *
 * More enemies in the eight starting zones (Lan.StartZones.SpawnMultiplier): every hostile or neutral
 * normal mob spawn there gets extra copies a few yards away. The copies only exist in memory: they are
 * added to the grids at startup, respawn like the original and are never saved.
 */

#include "Creature.h"
#include "GameObject.h"
#include "DBCStores.h"
#include "LanCommon.h"
#include "Log.h"
#include "Map.h"
#include "MapMgr.h"
#include "ObjectMgr.h"
#include "PoolMgr.h"
#include "Random.h"
#include "ScriptMgr.h"

#include <unordered_set>

namespace
{
    // Elwynn Forest, Dun Morogh, Tirisfal Glades, Durotar, Mulgore, Teldrassil, Azuremyst Isle, Eversong Woods.
    // The capitals are zones of their own, so they are not part of these.
    constexpr std::array<uint32, 8> StartZones = { 12, 1, 85, 14, 215, 141, 3524, 3430 };

    constexpr uint8 MAX_START_ZONE_MOB_LEVEL = 15;
    constexpr float COPY_DISTANCE = 4.0f;               // yards from the original spawn
    constexpr float COPY_WANDER_DISTANCE = 5.0f;        // for copies of mobs that walk a waypoint path
    constexpr uint32 FACTION_TEMPLATE_HUMAN = 1;        // an Alliance player
    constexpr uint32 FACTION_TEMPLATE_ORC = 2;          // a Horde player

    std::unordered_set<ObjectGuid::LowType> _copies;

    bool IsInStartZone(SpawnData const& data)
    {
        if (data.mapid != MAP_EASTERN_KINGDOMS && data.mapid != MAP_KALIMDOR && data.mapid != MAP_OUTLAND)
            return false;

        for (uint32 zone : StartZones)
        {
            // Cheap bounding box first, so terrain is only read for spawns that can be inside
            float x = data.posX;
            float y = data.posY;
            Map2ZoneCoordinates(x, y, zone);
            if (x < 0.0f || x > 100.0f || y < 0.0f || y > 100.0f)
                continue;

            if (sMapMgr->GetZoneId(data.phaseMask, data.mapid, data.posX, data.posY, data.posZ) == zone)
                return true;
        }
        return false;
    }

    // Mobs a player can fight: not friendly to either faction, no services, no critters, triggers, rares or
    // elites, and not part of a pool, a linked respawn or a scripted spawn
    bool IsEnemySpawn(CreatureData const& data, FactionTemplateEntry const* alliance, FactionTemplateEntry const* horde)
    {
        if (data.id2 || data.id3 || data.poolId || data.ScriptId || data.phaseMask != PHASEMASK_NORMAL ||
            sPoolMgr->IsPartOfAPool<Creature>(data.spawnId))
            return false;

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(data.id);
        if (!cInfo || cInfo->maxlevel > MAX_START_ZONE_MOB_LEVEL || cInfo->rank != CREATURE_ELITE_NORMAL ||
            cInfo->npcflag || data.npcflag || cInfo->type == CREATURE_TYPE_CRITTER || cInfo->ScriptID ||
            cInfo->HasFlagsExtra(CREATURE_FLAG_EXTRA_TRIGGER) ||
            (cInfo->unit_flags & (UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE)))
            return false;

        FactionTemplateEntry const* faction = sFactionTemplateStore.LookupEntry(cInfo->faction);
        if (!faction || faction->IsFriendlyTo(*alliance) || faction->IsFriendlyTo(*horde))
            return false;

        ObjectGuid guid = ObjectGuid::Create<HighGuid::Unit>(data.id, data.spawnId);
        return sObjectMgr->GetLinkedRespawnGuid(guid).IsEmpty();
    }

    void AddCopy(CreatureData const& original, float angle)
    {
        Map* map = sMapMgr->CreateBaseMap(original.mapid);
        if (!map)
            return;

        float x = original.posX + COPY_DISTANCE * std::cos(angle);
        float y = original.posY + COPY_DISTANCE * std::sin(angle);

        ObjectGuid::LowType spawnId = sObjectMgr->GenerateCreatureSpawnId();
        CreatureData& data = sObjectMgr->NewOrExistCreatureData(spawnId);
        data.spawnId = spawnId;
        data.id = original.id;
        data.id2 = 0;
        data.id3 = 0;
        data.mapid = original.mapid;
        data.phaseMask = original.phaseMask;
        data.spawnMask = original.spawnMask;
        data.posX = x;
        data.posY = y;
        data.posZ = original.posZ;                      // put on the ground when it enters the world
        data.orientation = Position::NormalizeOrientation(original.orientation + angle);
        data.displayid = original.displayid;
        data.equipmentId = original.equipmentId;
        data.spawntimesecs = original.spawntimesecs;
        data.currentwaypoint = 0;
        data.curhealth = original.curhealth;
        data.curmana = original.curmana;
        data.npcflag = original.npcflag;
        data.unit_flags = original.unit_flags;
        data.dynamicflags = original.dynamicflags;
        data.dbData = false;                            // never saved, no respawn time in the database

        // The waypoint path belongs to the original spawn: the copies wander around instead
        if (original.movementType == WAYPOINT_MOTION_TYPE)
        {
            data.movementType = RANDOM_MOTION_TYPE;
            data.wander_distance = COPY_WANDER_DISTANCE;
        }
        else
        {
            data.movementType = original.movementType;
            data.wander_distance = original.wander_distance;
        }

        _copies.insert(spawnId);
        sObjectMgr->AddCreatureToGrid(spawnId, &data);

        // Grids that are already loaded (preloaded maps) get the copy right away
        if (map->IsGridLoaded(x, y))
        {
            Creature* creature = new Creature();
            if (!creature->LoadCreatureFromDB(spawnId, map, true, true))
                delete creature;
        }
    }
}

// Wildflowers for the adventure route's quest 2: three around every Peacebloom, Silverleaf, Earthroot and
// Bloodthistle spawn point in the starting zones (also the pooled ones that are not up right now), and one near
// every enemy spawn there. They only exist in memory, like the extra enemies.
static void AddWildflower(uint32 mapId, float x, float y, float z, float o)
{
    constexpr uint32 WILDFLOWER_RESPAWN = 60;       // seconds

    // On the ground at that spot, so flowers on a slope don't float or sink
    if (Map* map = sMapMgr->CreateBaseMap(mapId))
    {
        float ground = map->GetHeight(PHASEMASK_NORMAL, x, y, z + 5.0f, true, 15.0f);
        if (ground > INVALID_HEIGHT)
            z = ground;
    }

    sObjectMgr->AddGOData(GO_LAN_WILDFLOWER, mapId, x, y, z, o, WILDFLOWER_RESPAWN);
}

static void LanBuildWildflowers(FactionTemplateEntry const* alliance, FactionTemplateEntry const* horde)
{
    constexpr std::array<uint32, 7> Herbs = { 1617, 1618, 1619, 3724, 3725, 3726, 181166 };
    constexpr uint32 FLOWERS_PER_HERB = 3;
    constexpr float HERB_DISTANCE = 2.5f;           // yards from the herb
    constexpr float ENEMY_DISTANCE = 7.0f;          // yards from the enemy spawn

    // Collect first: adding flowers changes the gameobject data store
    struct Spot
    {
        uint32 MapId;
        float X, Y, Z;
    };

    std::vector<Spot> herbs;
    for (auto const& [spawnId, data] : sObjectMgr->GetAllGOData())
        if (std::find(Herbs.begin(), Herbs.end(), data.id) != Herbs.end() && IsInStartZone(data))
            herbs.push_back({ data.mapid, data.posX, data.posY, data.posZ });

    std::vector<Spot> enemies;
    for (auto const& [spawnId, data] : sObjectMgr->GetAllCreatureData())
        if (data.dbData && IsEnemySpawn(data, alliance, horde) && IsInStartZone(data))
            enemies.push_back({ data.mapid, data.posX, data.posY, data.posZ });

    uint32 flowers = 0;
    for (Spot const& herb : herbs)
    {
        for (uint32 i = 0; i < FLOWERS_PER_HERB; ++i)
        {
            float angle = 2.0f * float(M_PI) * float(i) / float(FLOWERS_PER_HERB);
            AddWildflower(herb.MapId, herb.X + HERB_DISTANCE * std::cos(angle),
                herb.Y + HERB_DISTANCE * std::sin(angle), herb.Z, angle);
            ++flowers;
        }
    }

    for (Spot const& enemy : enemies)
    {
        float angle = frand(0.0f, 2.0f * float(M_PI));
        AddWildflower(enemy.MapId, enemy.X + ENEMY_DISTANCE * std::cos(angle),
            enemy.Y + ENEMY_DISTANCE * std::sin(angle), enemy.Z, angle);
        ++flowers;
    }

    LOG_INFO("module", "mod-lan: {} Wildflowers in the starting zones ({} herb spots, {} enemy spawns)", flowers,
        herbs.size(), enemies.size());
}

void LanBuildStartZoneSpawns()
{
    FactionTemplateEntry const* alliance = sFactionTemplateStore.LookupEntry(FACTION_TEMPLATE_HUMAN);
    FactionTemplateEntry const* horde = sFactionTemplateStore.LookupEntry(FACTION_TEMPLATE_ORC);
    if (!alliance || !horde)
        return;

    // Before the extra enemies, so the flowers follow the original spawns only
    if (GetLanConfig().RouteEnabled)
        LanBuildWildflowers(alliance, horde);

    uint32 multiplier = GetLanConfig().StartZoneSpawnMultiplier;
    if (multiplier <= 1)
        return;

    // Collect first: adding copies changes the creature data store
    std::vector<ObjectGuid::LowType> originals;
    for (auto const& [spawnId, data] : sObjectMgr->GetAllCreatureData())
        if (data.dbData && IsEnemySpawn(data, alliance, horde) && IsInStartZone(data))
            originals.push_back(spawnId);

    std::sort(originals.begin(), originals.end());

    uint32 copies = 0;
    for (ObjectGuid::LowType spawnId : originals)
    {
        CreatureData const* original = sObjectMgr->GetCreatureData(spawnId);
        if (!original)
            continue;

        // Spread the copies evenly around the original
        CreatureData const snapshot = *original;
        for (uint32 i = 1; i < multiplier; ++i)
        {
            AddCopy(snapshot, 2.0f * float(M_PI) * float(i) / float(multiplier));
            ++copies;
        }
    }

    LOG_INFO("module", "mod-lan: {} enemy spawns in the starting zones, {} copies added (x{})", originals.size(),
        copies, multiplier);
}

// The copies stand a few yards from the original; put them on the ground below or above that spot
class LanStartZoneSpawnScript : public AllCreatureScript
{
public:
    LanStartZoneSpawnScript() : AllCreatureScript("LanStartZoneSpawnScript") { }

    void OnCreatureAddWorld(Creature* creature) override
    {
        if (_copies.empty() || !_copies.count(creature->GetSpawnId()))
            return;

        Position home = creature->GetHomePosition();
        float ground = creature->GetMap()->GetHeight(creature->GetPhaseMask(), home.GetPositionX(),
            home.GetPositionY(), home.GetPositionZ() + 5.0f, true, 15.0f);
        if (ground <= INVALID_HEIGHT || std::fabs(ground - home.GetPositionZ()) < 0.05f)
            return;

        creature->Relocate(home.GetPositionX(), home.GetPositionY(), ground, home.GetOrientation());
        creature->SetHomePosition(home.GetPositionX(), home.GetPositionY(), ground, home.GetOrientation());
    }
};

void AddSC_lan_spawns()
{
    new LanStartZoneSpawnScript();
}
