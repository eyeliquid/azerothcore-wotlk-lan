/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Config.h"
#include "LanCommon.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "World.h"

namespace
{
    LanConfig _config;
}

LanConfig const& GetLanConfig()
{
    return _config;
}

void LoadLanConfig()
{
    LanConfig cfg;
    cfg.Enabled = sConfigMgr->GetOption<bool>("Lan.Enable", true);
    cfg.XPRate = sConfigMgr->GetOption<float>("Lan.XPRate", 200.0f);
    cfg.StartGold = sConfigMgr->GetOption<uint32>("Lan.StartGold", 150);
    cfg.SoloRaids = sConfigMgr->GetOption<bool>("Lan.SoloRaids", true);

    cfg.AutoLearnClassSpells = sConfigMgr->GetOption<bool>("Lan.AutoLearn.ClassSpells", true);
    cfg.AutoLearnQuestSpells = sConfigMgr->GetOption<bool>("Lan.AutoLearn.QuestSpells", true);
    cfg.AutoLearnRiding = sConfigMgr->GetOption<bool>("Lan.AutoLearn.Riding", true);

    cfg.HunterPetsAlwaysHappy = sConfigMgr->GetOption<bool>("Lan.HunterPets.AlwaysHappy", true);

    cfg.PartyAuraEnabled = sConfigMgr->GetOption<bool>("Lan.PartyAura.Enable", true);
    cfg.PartyAuraMinPlayers = sConfigMgr->GetOption<uint32>("Lan.PartyAura.MinPlayers", 3);
    cfg.PartyAuraSpell = sConfigMgr->GetOption<uint32>("Lan.PartyAura.Spell", 20217);
    cfg.PartyAuraXPBonus = sConfigMgr->GetOption<float>("Lan.PartyAura.XPBonus", 10.0f) / 100.0f;

    cfg.GearPrice = sConfigMgr->GetOption<uint32>("Lan.Vendors.GearPrice", 10000);
    cfg.FoodPrice = sConfigMgr->GetOption<uint32>("Lan.Vendors.FoodPrice", 0);
    cfg.BlockSellBack = sConfigMgr->GetOption<bool>("Lan.Vendors.BlockSellBack", true);

    cfg.LfgFillEnabled = sConfigMgr->GetOption<bool>("Lan.LfgFill.Enable", true);
    cfg.LfgFillDelay = sConfigMgr->GetOption<uint32>("Lan.LfgFill.Delay", 120);
    cfg.HelperBotsBuff = sConfigMgr->GetOption<bool>("Lan.HelperBots.Buff", true);
    cfg.HelperDamageDone = 1.0f + sConfigMgr->GetOption<float>("Lan.HelperBots.DamageDone", 25.0f) / 100.0f;
    cfg.HelperHealingDone = 1.0f + sConfigMgr->GetOption<float>("Lan.HelperBots.HealingDone", 25.0f) / 100.0f;
    cfg.HelperDamageTaken = 1.0f - sConfigMgr->GetOption<float>("Lan.HelperBots.DamageTakenReduction", 20.0f) / 100.0f;
    cfg.HelperFreeForAllLoot = sConfigMgr->GetOption<bool>("Lan.HelperBots.FreeForAllLoot", true);

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

        if (cfg.XPRate > 0.0f)
        {
            sWorld->setRate(RATE_XP_KILL, cfg.XPRate);
            sWorld->setRate(RATE_XP_QUEST, cfg.XPRate);
            sWorld->setRate(RATE_XP_QUEST_DF, cfg.XPRate);
            sWorld->setRate(RATE_XP_EXPLORE, cfg.XPRate);
        }

        sWorld->setIntConfig(CONFIG_START_PLAYER_MONEY, std::min<uint32>(cfg.StartGold * GOLD, MAX_MONEY_AMOUNT));

        if (cfg.SoloRaids)
            sWorld->setBoolConfig(CONFIG_INSTANCE_IGNORE_RAID, true);

        LOG_INFO("module", "mod-lan: XP x{}, start gold {}, solo raids {}", cfg.XPRate, cfg.StartGold, cfg.SoloRaids);
    }

    void OnStartup() override
    {
        if (GetLanConfig().Enabled)
            LanBuildVendors();
    }
};

void AddSC_lan_config()
{
    new LanConfigWorldScript();
}
