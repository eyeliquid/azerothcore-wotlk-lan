/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 *
 * Automatic spell learning (class trainer spells, class quest spells, riding) and the
 * player-side hooks of the LAN party aura and the vendors.
 */

#include "Chat.h"
#include "LanCommon.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Trainer.h"

#include <bit>

namespace
{
    // Riding skill ranks: Apprentice, Journeyman, Expert, Artisan, Cold Weather Flying.
    constexpr std::array<uint32, 5> RidingSpells = { 33388, 33391, 34090, 34091, 54197 };

    // Armor and weapon proficiencies, so the Tier 6 gear of the starting areas can be worn from level 1:
    // Mail, Plate Mail, Shield, one- and two-hand axes, maces and swords, polearms, staves, bows, guns,
    // daggers, thrown, crossbows, fist weapons, wands.
    constexpr std::array<uint32, 18> ProficiencySpells =
    {
        8737, 750, 9116, 196, 197, 198, 199, 200, 201, 202, 227, 264, 266, 1180, 2567, 5011, 15590, 5009
    };
    constexpr uint32 SPELL_DUAL_WIELD = 674;

    struct LearnCache
    {
        bool Built = false;
        std::array<std::vector<Trainer::Trainer const*>, MAX_CLASSES> ClassTrainers;
        std::vector<std::pair<Trainer::Trainer const*, Trainer::Spell const*>> Riding;
        std::vector<Quest const*> ClassQuests;
    };

    LearnCache _cache;

    void BuildLearnCache()
    {
        if (_cache.Built)
            return;

        _cache.Built = true;

        std::unordered_set<Trainer::Trainer const*> seen;
        for (auto const& [entry, creatureTemplate] : *sObjectMgr->GetCreatureTemplates())
        {
            Trainer::Trainer const* trainer = sObjectMgr->GetTrainer(entry);
            if (!trainer || !seen.insert(trainer).second)
                continue;

            if (trainer->GetTrainerType() == Trainer::Type::Class && trainer->GetTrainerRequirement() < MAX_CLASSES)
                _cache.ClassTrainers[trainer->GetTrainerRequirement()].push_back(trainer);

            for (Trainer::Spell const& spell : trainer->GetSpells())
            {
                if (std::find(RidingSpells.begin(), RidingSpells.end(), spell.SpellId) == RidingSpells.end())
                    continue;

                bool known = std::any_of(_cache.Riding.begin(), _cache.Riding.end(),
                    [&spell](auto const& pair) { return pair.second->SpellId == spell.SpellId; });
                if (!known)
                    _cache.Riding.emplace_back(trainer, &spell);
            }
        }

        // Class quests: limited to a few classes and rewarding a spell
        for (auto const& [questId, quest] : sObjectMgr->GetQuestTemplates())
        {
            uint32 classes = quest->GetRequiredClasses() & CLASSMASK_ALL_PLAYABLE;
            if (!classes || std::popcount(classes) > 3)
                continue;

            if (quest->GetRewSpellCast() > 0 || quest->GetRewSpell() > 0)
                _cache.ClassQuests.push_back(quest);
        }
    }

    bool TeachTrainerSpell(Player* player, Trainer::Trainer const* trainer, Trainer::Spell const* spell)
    {
        if (!trainer->CanTeachSpell(player, spell))
            return false;

        if (!spell->IsCastable())
            player->learnSpell(spell->SpellId, false);
        else if (sScriptMgr->OnPlayerCanLearnSpell(player, spell->SpellId))
            player->CastSpell(player, spell->SpellId, true);
        else
            return false;

        return true;
    }

    void LearnTrainerSpells(Player* player)
    {
        LanConfig const& cfg = GetLanConfig();

        // A rank can depend on the previous one, so repeat until nothing new is learned
        for (uint8 pass = 0; pass < 8; ++pass)
        {
            bool learned = false;

            if (cfg.AutoLearnClassSpells)
                for (Trainer::Trainer const* trainer : _cache.ClassTrainers[player->getClass()])
                    for (Trainer::Spell const& spell : trainer->GetSpells())
                        learned |= TeachTrainerSpell(player, trainer, &spell);

            if (cfg.AutoLearnRiding)
                for (auto const& [trainer, spell] : _cache.Riding)
                    learned |= TeachTrainerSpell(player, trainer, spell);

            if (!learned)
                break;
        }
    }

    void LearnProficiencies(Player* player)
    {
        for (uint32 spellId : ProficiencySpells)
            if (!player->HasSpell(spellId) && player->IsSpellFitByClassAndRace(spellId))
                player->learnSpell(spellId, false);

        // Classes that learn Dual Wield from their trainer (for shamans it is a talent)
        uint8 classId = player->getClass();
        bool trainsDualWield = classId == CLASS_WARRIOR || classId == CLASS_ROGUE || classId == CLASS_HUNTER ||
            classId == CLASS_DEATH_KNIGHT;
        if (trainsDualWield && !player->HasSpell(SPELL_DUAL_WIELD))
            player->learnSpell(SPELL_DUAL_WIELD, false);
    }

    void LearnQuestSpells(Player* player)
    {
        uint32 classMask = player->getClassMask();
        uint32 raceMask = player->getRaceMask();

        for (Quest const* quest : _cache.ClassQuests)
        {
            if (!(quest->GetRequiredClasses() & classMask))
                continue;

            if (quest->GetAllowableRaces() && !(quest->GetAllowableRaces() & raceMask))
                continue;

            if (player->GetLevel() < quest->GetMinLevel())
                continue;

            uint32 spellId = quest->GetRewSpellCast() > 0 ? uint32(quest->GetRewSpellCast()) : quest->GetRewSpell();
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            for (SpellEffectInfo const& effect : spellInfo->GetEffects())
            {
                if (!effect.IsEffect(SPELL_EFFECT_LEARN_SPELL) || !effect.TriggerSpell)
                    continue;

                if (player->HasSpell(effect.TriggerSpell) || !player->IsSpellFitByClassAndRace(effect.TriggerSpell))
                    continue;

                if (!sScriptMgr->OnPlayerCanLearnSpell(player, effect.TriggerSpell))
                    continue;

                player->learnSpell(effect.TriggerSpell, false);
            }

            // Class quests that hand out tools the class needs, e.g. the shaman totems
            for (uint32 itemId : quest->RewardItemId)
            {
                ItemTemplate const* proto = itemId ? sObjectMgr->GetItemTemplate(itemId) : nullptr;
                if (!proto || !proto->TotemCategory || player->HasItemCount(itemId, 1, true))
                    continue;

                player->AddItem(itemId, 1);
            }
        }
    }
}

void LanLearnSpells(Player* player)
{
    LanConfig const& cfg = GetLanConfig();
    if (!cfg.Enabled || !player)
        return;

    BuildLearnCache();
    LearnProficiencies(player);
    LearnTrainerSpells(player);

    if (cfg.AutoLearnQuestSpells)
        LearnQuestSpells(player);
}

class LanPlayerScript : public PlayerScript
{
public:
    LanPlayerScript() : PlayerScript("LanPlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_LEVEL_CHANGED,
        PLAYERHOOK_ON_GIVE_EXP,
        PLAYERHOOK_CAN_SELL_ITEM
    }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (LanIsRealPlayer(player))
            LanLearnSpells(player);
    }

    void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        if (LanIsRealPlayer(player))
            LanLearnSpells(player);
    }

    void OnPlayerGiveXP(Player* player, uint32& amount, Unit* /*victim*/, uint8 /*xpSource*/) override
    {
        LanConfig const& cfg = GetLanConfig();
        if (!cfg.Enabled || !cfg.PartyAuraEnabled || !LanIsRealPlayer(player))
            return;

        if (LanRealPlayersOnline() >= cfg.PartyAuraMinPlayers)
            amount = uint32(amount * (1.0f + cfg.PartyAuraXPBonus));
    }

    bool OnPlayerCanSellItem(Player* player, Item* item, Creature* /*creature*/) override
    {
        LanConfig const& cfg = GetLanConfig();
        if (!cfg.Enabled || !cfg.BlockSellBack || !item || !LanIsVendorItem(item->GetEntry()))
            return true;

        ChatHandler(player->GetSession()).SendNotification("Items sold by the LAN vendors can't be sold back.");
        return false;
    }
};

void AddSC_lan_player()
{
    new LanPlayerScript();
}
