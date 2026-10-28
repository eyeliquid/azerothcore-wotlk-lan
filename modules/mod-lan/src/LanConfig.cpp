/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Config.h"
#include "LanCommon.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "World.h"

#include <algorithm>
#include <limits>

namespace
{
    LanConfig _config;

    constexpr uint32 ITEM_HEARTHSTONE = 6948;

    // The item's own cooldown wins over the spell's 60 minutes, and it is sent to the client with the item
    void SetHearthstoneCooldown(uint32 seconds)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(ITEM_HEARTHSTONE);
        if (!proto || !seconds)
            return;

        ItemTemplate* hearthstone = const_cast<ItemTemplate*>(proto);
        hearthstone->Spells[0].SpellCooldown = int32(seconds * IN_MILLISECONDS);
        hearthstone->Spells[0].SpellCategoryCooldown = -1;
        LOG_INFO("module", "mod-lan: Hearthstone cooldown {} seconds", seconds);
    }
}

LanConfig const& GetLanConfig()
{
    return _config;
}

void LoadLanConfig()
{
    LanConfig cfg;
    cfg.Enabled = sConfigMgr->GetOption<bool>("Lan.Enable", true);
    cfg.StartGold = sConfigMgr->GetOption<uint32>("Lan.StartGold", 150);
    cfg.SkillGain = sConfigMgr->GetOption<uint32>("Lan.SkillGain", 4);
    cfg.TalentRate = sConfigMgr->GetOption<float>("Lan.TalentRate", 3.0f);
    cfg.NoResurrectionSickness = sConfigMgr->GetOption<bool>("Lan.NoResurrectionSickness", true);
    cfg.AlwaysRested = sConfigMgr->GetOption<bool>("Lan.AlwaysRested", true);
    cfg.StartZoneSpawnMultiplier = sConfigMgr->GetOption<uint32>("Lan.StartZones.SpawnMultiplier", 3);
    cfg.HearthstoneCooldown = sConfigMgr->GetOption<uint32>("Lan.HearthstoneCooldown", 300);
    cfg.SoloRaids = sConfigMgr->GetOption<bool>("Lan.SoloRaids", true);

    cfg.AutoLearnClassSpells = sConfigMgr->GetOption<bool>("Lan.AutoLearn.ClassSpells", true);
    cfg.AutoLearnQuestSpells = sConfigMgr->GetOption<bool>("Lan.AutoLearn.QuestSpells", true);
    cfg.AutoLearnRiding = sConfigMgr->GetOption<bool>("Lan.AutoLearn.Riding", true);
    cfg.RidingFromLevel1 = sConfigMgr->GetOption<bool>("Lan.AutoLearn.RidingFromLevel1", true);

    cfg.HunterPetsAlwaysHappy = sConfigMgr->GetOption<bool>("Lan.HunterPets.AlwaysHappy", true);
    cfg.AllFlightPaths = sConfigMgr->GetOption<bool>("Lan.AllFlightPaths", true);

    cfg.PartyAuraEnabled = sConfigMgr->GetOption<bool>("Lan.PartyAura.Enable", true);
    cfg.PartyAuraMinPlayers = sConfigMgr->GetOption<uint32>("Lan.PartyAura.MinPlayers", 3);
    cfg.PartyAuraSpell = sConfigMgr->GetOption<uint32>("Lan.PartyAura.Spell", 20217);
    cfg.PartyAuraXPBonus = sConfigMgr->GetOption<float>("Lan.PartyAura.XPBonus", 10.0f) / 100.0f;

    cfg.GearPrice = sConfigMgr->GetOption<uint32>("Lan.Vendors.GearPrice", 10000);
    cfg.FoodPrice = sConfigMgr->GetOption<uint32>("Lan.Vendors.FoodPrice", 0);
    cfg.GemPrice = sConfigMgr->GetOption<uint32>("Lan.Vendors.GemPrice", 0);
    cfg.MountPrice = sConfigMgr->GetOption<uint32>("Lan.Vendors.MountPrice", 0);
    cfg.BlockSellBack = sConfigMgr->GetOption<bool>("Lan.Vendors.BlockSellBack", true);

    cfg.LfgFillEnabled = sConfigMgr->GetOption<bool>("Lan.LfgFill.Enable", true);
    cfg.LfgFillDelay = sConfigMgr->GetOption<uint32>("Lan.LfgFill.Delay", 120);
    cfg.HelperBotsBuff = sConfigMgr->GetOption<bool>("Lan.HelperBots.Buff", true);
    cfg.HelperDamageDone = 1.0f + sConfigMgr->GetOption<float>("Lan.HelperBots.DamageDone", 25.0f) / 100.0f;
    cfg.HelperHealingDone = 1.0f + sConfigMgr->GetOption<float>("Lan.HelperBots.HealingDone", 25.0f) / 100.0f;
    cfg.HelperDamageTaken = 1.0f - sConfigMgr->GetOption<float>("Lan.HelperBots.DamageTakenReduction", 20.0f) / 100.0f;
    cfg.HelperFreeForAllLoot = sConfigMgr->GetOption<bool>("Lan.HelperBots.FreeForAllLoot", true);

    cfg.RouteEnabled = sConfigMgr->GetOption<bool>("Lan.Route.Enable", true);
    cfg.RoutePassiveMobs = sConfigMgr->GetOption<bool>("Lan.Route.PassiveMobs", true);
    cfg.RouteKeeperDespawn = sConfigMgr->GetOption<uint32>("Lan.Route.KeeperDespawn", 3600);
    uint32 deadmines = sConfigMgr->GetOption<uint32>("Lan.Route.DeadminesStrength", 50);
    uint32 ramparts = sConfigMgr->GetOption<uint32>("Lan.Route.RampartsStrength", 10);
    cfg.RouteDeadminesStrength = std::clamp(deadmines, 1u, 100u) / 100.0f;
    cfg.RouteRampartsStrength = std::clamp(ramparts, 1u, 100u) / 100.0f;
    uint32 northrend = sConfigMgr->GetOption<uint32>("Lan.Route.NorthrendStrength", 10);
    cfg.RouteNorthrendStrength = std::clamp(northrend, 1u, 100u) / 100.0f;

    _config = cfg;
}

class LanConfigWorldScript : public WorldScript
{
public:
    LanConfigWorldScript() : WorldScript("LanConfigWorldScript", {
        WORLDHOOK_ON_AFTER_CONFIG_LOAD,
        WORLDHOOK_ON_STARTUP
    }) { }

    // Runs after the core read worldserver.conf, so these values win over it.
    void OnAfterConfigLoad(bool /*reload*/) override
    {
        LoadLanConfig();
        LanConfig const& cfg = GetLanConfig();
        if (!cfg.Enabled)
            return;

        // Professions (crafting, gathering, fishing), weapon skills and defense
        if (cfg.SkillGain > 0)
        {
            sWorld->setIntConfig(CONFIG_SKILL_GAIN_CRAFTING, cfg.SkillGain);
            sWorld->setIntConfig(CONFIG_SKILL_GAIN_GATHERING, cfg.SkillGain);
            sWorld->setIntConfig(CONFIG_SKILL_GAIN_WEAPON, cfg.SkillGain);
            sWorld->setIntConfig(CONFIG_SKILL_GAIN_DEFENSE, cfg.SkillGain);
        }

        if (cfg.TalentRate > 0.0f)
            sWorld->setRate(RATE_TALENT, cfg.TalentRate);

        // Resurrection sickness starts at this level, so no character ever gets it
        if (cfg.NoResurrectionSickness)
            sWorld->setIntConfig(CONFIG_DEATH_SICKNESS_LEVEL, std::numeric_limits<int32>::max());

        sWorld->setIntConfig(CONFIG_START_PLAYER_MONEY, std::min<uint32>(cfg.StartGold * GOLD, MAX_MONEY_AMOUNT));

        if (cfg.SoloRaids)
            sWorld->setBoolConfig(CONFIG_INSTANCE_IGNORE_RAID, true);

        LOG_INFO("module", "mod-lan: skill gain x{}, start gold {}, solo raids {}", cfg.SkillGain,
            cfg.StartGold, cfg.SoloRaids);
    }

    void OnStartup() override
    {
        if (!GetLanConfig().Enabled)
            return;

        // The route's Tier 6 tokens rely on the vendors removing the T6 level requirement
        LanBuildVendors();
        LanBuildRoute();
        LanBuildStartZoneSpawns();
        SetHearthstoneCooldown(GetLanConfig().HearthstoneCooldown);
    }
};

void AddSC_lan_config()
{
    new LanConfigWorldScript();
}
