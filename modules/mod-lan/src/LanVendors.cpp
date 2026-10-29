/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 *
 * Vendors: the starting-area outfitter (Tier 6 per class and spec), the bag vendor, the Dalaran
 * spec vendors (Tier 9 with gems, enchant items and enchanting) and the profession vendors.
 * Their lists are built in memory at startup and opened through gossip.
 */

#include "Chat.h"
#include "Creature.h"
#include "DBCStores.h"
#include "LanCommon.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "StringFormat.h"
#include "WorldSession.h"

#include <map>
#include <unordered_map>

namespace
{
    // Equipment slot for each WoWSims slot index (see LanGearData.h)
    constexpr std::array<uint8, LAN_GEAR_SLOTS> GearSlotToEquipSlot =
    {
        EQUIPMENT_SLOT_HEAD, EQUIPMENT_SLOT_NECK, EQUIPMENT_SLOT_SHOULDERS, EQUIPMENT_SLOT_BACK,
        EQUIPMENT_SLOT_CHEST, EQUIPMENT_SLOT_WRISTS, EQUIPMENT_SLOT_HANDS, EQUIPMENT_SLOT_WAIST,
        EQUIPMENT_SLOT_LEGS, EQUIPMENT_SLOT_FEET, EQUIPMENT_SLOT_FINGER1, EQUIPMENT_SLOT_FINGER2,
        EQUIPMENT_SLOT_TRINKET1, EQUIPMENT_SLOT_TRINKET2, EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND,
        EQUIPMENT_SLOT_RANGED
    };

    enum GearSlotIndex : uint8
    {
        GEAR_HEAD = 0, GEAR_SHOULDER = 2, GEAR_BACK = 3, GEAR_WRIST = 5, GEAR_HANDS = 6, GEAR_WAIST = 7, GEAR_FEET = 9
    };

    constexpr uint32 ITEM_NIGHTBLADE = 1982;

    // The largest bag of every kind (general, soul, herb, enchanting, engineering, gem, mining,
    // leatherworking, inscription, quiver, ammo pouch)
    // Hunter ammo: the best arrows and bullets, and the level 72 ones (all usable from level 1)
    constexpr std::array<uint32, 4> BestAmmo =
    {
        52021,  // Iceblade Arrow
        52020,  // Shatter Rounds
        41165,  // Saronite Razorheads
        41164   // Mammoth Cutters
    };

    // Level 80 thrown weapons (no PvP or faction ones)
    constexpr std::array<uint32, 5> BestThrown =
    {
        47660,  // Blades of the Sable Cross
        47659,  // Crimson Star
        45296,  // Twirling Blades
        45086,  // Rising Sun
        40190   // Spinning Fate
    };

    // The best rogue poisons
    constexpr std::array<uint32, 6> BestPoisons =
    {
        43231,  // Instant Poison IX
        43233,  // Deadly Poison IX
        43235,  // Wound Poison VII
        43237,  // Anesthetic Poison II
        3775,   // Crippling Poison
        5237    // Mind-numbing Poison
    };

    // Spell reagents of every class, for every rank of the spells that need one
    constexpr std::array<uint32, 25> Reagents =
    {
        21177,  // Symbol of Kings (paladin)
        17033,  // Symbol of Divinity (paladin)
        17028,  // Holy Candle (priest)
        17029,  // Sacred Candle (priest)
        44615,  // Devout Candle (priest)
        17056,  // Light Feather (priest, mage)
        17020,  // Arcane Powder (mage)
        17031,  // Rune of Teleportation (mage)
        17032,  // Rune of Portals (mage)
        17030,  // Ankh (shaman)
        6265,   // Soul Shard (warlock)
        5565,   // Infernal Stone (warlock)
        16583,  // Demonic Figurine (warlock)
        5140,   // Flash Powder (rogue)
        37201,  // Corpse Dust (death knight)
        17021,  // Wild Berries (druid)
        17026,  // Wild Thornroot (druid)
        22148,  // Wild Quillvine (druid)
        44605,  // Wild Spineleaf (druid)
        17034,  // Maple Seed (druid)
        17035,  // Stranglethorn Seed (druid)
        17036,  // Ashwood Seed (druid)
        17037,  // Hornbeam Seed (druid)
        17038,  // Ironwood Seed (druid)
        44614   // Starleaf Seed (druid)
    };

    constexpr std::array<uint32, 11> BiggestBags =
    {
        51809,  // Portable Hole (24)
        41597,  // Abyssal Bag (32)
        45773,  // Emerald Bag (32)
        41598,  // Mysterious Bag (32)
        23775,  // Titanium Toolbox (32)
        24270,  // Bag of Jewels (24)
        38347,  // Mammoth Mining Bag (32)
        38399,  // Trapper's Traveling Pack (28)
        44446,  // Pack of Endless Pockets (32)
        44448,  // Nerubian Reinforced Quiver (28)
        44447   // Dragonscale Ammo Pouch (28)
    };

    // The best food and drink: feasts, the level 80 Well Fed foods, the pet buff treat, the best plain
    // food of every pet diet (meat, fish, cheese, bread, fungus, fruit) and the best drinks
    constexpr std::array<uint32, 31> BestFood =
    {
        43015,  // Fish Feast
        34753,  // Great Feast
        43000,  // Dragonfin Filet
        42999,  // Blackened Dragonfin
        34767,  // Firecracker Salmon
        34755,  // Tender Shoveltusk Steak
        34754,  // Mega Mammoth Meal
        34766,  // Poached Northern Sculpin
        34756,  // Spiced Worm Burger
        34768,  // Spicy Blue Nettlefish
        34757,  // Very Burnt Worg
        34769,  // Imperial Manta Steak
        42993,  // Spicy Fried Herring
        42996,  // Snapper Extreme
        44953,  // Worg Tartare
        42994,  // Rhinolicious Wormsteak
        42995,  // Hearty Rhino
        42997,  // Blackened Worg Steak
        42998,  // Cuttlesteak
        34758,  // Mighty Rhino Dogs
        43001,  // Tracker Snacks
        43005,  // Spiced Mammoth Treats
        35953,  // Mead Basted Caribou (meat)
        35951,  // Poached Emperor Salmon (fish)
        35952,  // Briny Hardcheese (cheese)
        35950,  // Sweet Potato Bread (bread)
        35947,  // Sparkling Frostcap (fungus)
        35948,  // Savory Snowplum (fruit)
        33445,  // Honeymint Tea (drink)
        41731,  // Yeti Milk (drink)
        42777   // Crusader's Waterskin (drink)
    };

    constexpr std::array<uint32, 7> BestPotions =
    {
        33447,  // Runic Healing Potion
        33448,  // Runic Mana Potion
        40211,  // Potion of Speed
        40212,  // Potion of Wild Magic
        40093,  // Indestructible Potion
        40081,  // Potion of Nightmares
        13442   // Mighty Rage Potion
    };

    // Battle and guardian elixirs
    constexpr std::array<uint32, 16> BestElixirs =
    {
        40073,  // Elixir of Mighty Strength
        39666,  // Elixir of Mighty Agility
        40070,  // Spellpower Elixir
        40068,  // Wrath Elixir
        40076,  // Guru's Elixir
        44325,  // Elixir of Accuracy
        44327,  // Elixir of Deadly Strikes
        44329,  // Elixir of Expertise
        44330,  // Elixir of Armor Piercing
        44331,  // Elixir of Lightning Speed
        40078,  // Elixir of Mighty Fortitude
        40097,  // Elixir of Protection
        44328,  // Elixir of Mighty Defense
        40109,  // Elixir of Mighty Mageblood
        44332,  // Elixir of Mighty Thoughts
        40072   // Elixir of Spirit
    };

    constexpr std::array<uint32, 6> BestFlasks =
    {
        46377,  // Flask of Endless Rage
        46376,  // Flask of the Frost Wyrm
        46378,  // Flask of Pure Mojo
        46379,  // Flask of Stoneblood
        40079,  // Lesser Flask of Toughness
        44939   // Lesser Flask of Resistance
    };

    struct LanGem
    {
        uint32 ItemId;
        uint32 Color;                                   // ITEM_SUBCLASS_GEM_*
    };

    // Burning Crusade: every Sunwell epic cut gem and every cut meta gem
    constexpr std::array<LanGem, 53> BestGems =
    {{
        { 32193, 0 }, // Bold Crimson Spinel
        { 32197, 0 }, // Bright Crimson Spinel
        { 32194, 0 }, // Delicate Crimson Spinel
        { 32199, 0 }, // Flashing Crimson Spinel
        { 32196, 0 }, // Runed Crimson Spinel
        { 32198, 0 }, // Subtle Crimson Spinel
        { 32195, 0 }, // Teardrop Crimson Spinel
        { 32202, 1 }, // Lustrous Empyrean Sapphire
        { 32200, 1 }, // Solid Empyrean Sapphire
        { 32201, 1 }, // Sparkling Empyrean Sapphire
        { 32203, 1 }, // Stormy Empyrean Sapphire
        { 32204, 2 }, // Brilliant Lionseye
        { 32207, 2 }, // Gleaming Lionseye
        { 32210, 2 }, // Great Lionseye
        { 32209, 2 }, // Mystic Lionseye
        { 35761, 2 }, // Quick Lionseye
        { 32206, 2 }, // Rigid Lionseye
        { 32205, 2 }, // Smooth Lionseye
        { 32208, 2 }, // Thick Lionseye
        { 32213, 3 }, // Balanced Shadowsong Amethyst
        { 32215, 3 }, // Glowing Shadowsong Amethyst
        { 32214, 3 }, // Infused Shadowsong Amethyst
        { 37503, 3 }, // Purified Shadowsong Amethyst
        { 32216, 3 }, // Royal Shadowsong Amethyst
        { 32212, 3 }, // Shifting Shadowsong Amethyst
        { 32211, 3 }, // Sovereign Shadowsong Amethyst
        { 32225, 4 }, // Dazzling Seaspray Emerald
        { 32223, 4 }, // Enduring Seaspray Emerald
        { 35759, 4 }, // Forceful Seaspray Emerald
        { 32226, 4 }, // Jagged Seaspray Emerald
        { 32224, 4 }, // Radiant Seaspray Emerald
        { 35758, 4 }, // Steady Seaspray Emerald
        { 32220, 5 }, // Glinting Pyrestone
        { 32217, 5 }, // Inscribed Pyrestone
        { 32219, 5 }, // Luminous Pyrestone
        { 32218, 5 }, // Potent Pyrestone
        { 35760, 5 }, // Reckless Pyrestone
        { 32221, 5 }, // Veiled Pyrestone
        { 32222, 5 }, // Wicked Pyrestone
        { 25897, 6 }, // Bracing Earthstorm Diamond
        { 25899, 6 }, // Brutal Earthstorm Diamond
        { 34220, 6 }, // Chaotic Skyfire Diamond
        { 25890, 6 }, // Destructive Skyfire Diamond
        { 35503, 6 }, // Ember Skyfire Diamond
        { 25895, 6 }, // Enigmatic Skyfire Diamond
        { 35501, 6 }, // Eternal Earthstorm Diamond
        { 25901, 6 }, // Insightful Earthstorm Diamond
        { 25893, 6 }, // Mystical Skyfire Diamond
        { 25896, 6 }, // Powerful Earthstorm Diamond
        { 32409, 6 }, // Relentless Earthstorm Diamond
        { 25894, 6 }, // Swift Skyfire Diamond
        { 25898, 6 }, // Tenacious Earthstorm Diamond
        { 32410, 6 }, // Thundering Skyfire Diamond
    }};

    struct GemColor
    {
        uint32 Subclass;
        char const* Name;
    };

    constexpr std::array<GemColor, 7> GemColors =
    {{
        { ITEM_SUBCLASS_GEM_RED, "Red gems (Crimson Spinel)" },
        { ITEM_SUBCLASS_GEM_BLUE, "Blue gems (Empyrean Sapphire)" },
        { ITEM_SUBCLASS_GEM_YELLOW, "Yellow gems (Lionseye)" },
        { ITEM_SUBCLASS_GEM_PURPLE, "Purple gems (Shadowsong Amethyst)" },
        { ITEM_SUBCLASS_GEM_GREEN, "Green gems (Seaspray Emerald)" },
        { ITEM_SUBCLASS_GEM_ORANGE, "Orange gems (Pyrestone)" },
        { ITEM_SUBCLASS_GEM_META, "Meta gems (Earthstorm and Skyfire Diamonds)" }
    }};

    constexpr std::array<uint32, 10> GroundMounts =
    {
        13335,  // Deathcharger's Reins
        19902,  // Swift Zulian Tiger
        19872,  // Swift Razzashi Raptor
        30480,  // Fiery Warhorse's Reins
        32768,  // Reins of the Raven Lord
        33809,  // Amani War Bear
        35513,  // Swift White Hawkstrider
        49284,  // Reins of the Swift Spectral Tiger
        38576,  // Big Battle Bear
        46778   // Magic Rooster Egg
    };

    constexpr std::array<uint32, 10> FlyingMounts =
    {
        50818,  // Invincible's Reins
        32458,  // Ashes of Al'ar
        45693,  // Mimiron's Head
        54811,  // Celestial Steed
        49636,  // Reins of the Onyxian Drake
        43986,  // Reins of the Black Drake
        43954,  // Reins of the Twilight Drake
        43952,  // Reins of the Azure Drake
        44175,  // Reins of the Plagued Proto-Drake
        45801   // Reins of the Ironbound Proto-Drake
    };

    // Consumable categories (offset on LAN_VENDOR_CONSUMABLES)
    constexpr uint32 CONSUMABLE_POTIONS = 0;
    constexpr uint32 CONSUMABLE_FOOD = 1;
    constexpr uint32 CONSUMABLE_ELIXIRS = 2;
    constexpr uint32 CONSUMABLE_FLASKS = 3;
    constexpr uint32 CONSUMABLE_POISONS = 4;
    constexpr uint32 CONSUMABLE_REAGENTS = 5;
    constexpr uint32 CONSUMABLE_MAX = 6;

    constexpr uint32 VENDOR_PAGE_SIZE = MAX_VENDOR_ITEMS;
    constexpr uint32 PROFESSION_PAGES_RECIPES = 0;
    constexpr uint32 PROFESSION_PAGES_REAGENTS = 40;
    constexpr uint32 PROFESSION_PAGES_TOOLS = 80;

    struct ProfessionPages
    {
        uint32 Recipes = 0;
        uint32 Reagents = 0;
        uint32 Tools = 0;
    };

    std::unordered_set<uint32> _vendorItems;                    // sold for less than they sell for
    std::unordered_map<uint32, uint32> _enchantSpells;          // enchant id -> spell that applies it
    std::unordered_map<uint32, uint32> _enchantItems;           // enchant id -> item that applies it
    std::array<ProfessionPages, LanProfessions.size()> _professionPages;

    bool IsJunkName(std::string const& name)
    {
        static std::array<char const*, 10> const tags =
            { "Test", "TEST", "QA", "Deprecated", "DEPRECATED", "[PH]", "OLD", "Unused", "Monster -", "zz" };
        for (char const* tag : tags)
            if (name.find(tag) != std::string::npos)
                return true;
        return false;
    }

    bool AddVendorItem(uint32 vendorEntry, uint32 itemId, uint32 price)
    {
        ItemTemplate const* proto = itemId ? sObjectMgr->GetItemTemplate(itemId) : nullptr;
        if (!proto)
            return false;

        VendorItemData const* list = sObjectMgr->GetNpcVendorItemList(vendorEntry);
        if (list && (list->FindItemCostPair(itemId, 0) || list->GetItemCount() >= VENDOR_PAGE_SIZE))
            return false;

        sObjectMgr->AddVendorItem(vendorEntry, itemId, 0, 0, 0, false);
        list = sObjectMgr->GetNpcVendorItemList(vendorEntry);
        list->GetItem(list->GetItemCount() - 1)->PriceOverride = int32(price);

        if (price < proto->SellPrice)
            _vendorItems.insert(itemId);
        return true;
    }

    // Splits the items over vendor pages of 150; returns the number of pages.
    uint32 AddPagedItems(uint32 firstVendorEntry, std::vector<uint32> const& items, uint32 price, uint32 maxPages)
    {
        uint32 pages = 0;
        for (std::size_t i = 0; i < items.size() && i / VENDOR_PAGE_SIZE < maxPages; ++i)
        {
            uint32 page = i / VENDOR_PAGE_SIZE;
            if (AddVendorItem(firstVendorEntry + page, items[i], price))
                pages = page + 1;
        }
        return pages;
    }

    void BuildEnchantMaps()
    {
        std::unordered_set<uint32> wanted;
        for (LanSpecData const& spec : LanSpecs)
            for (auto const& team : spec.T9Enchant)
                for (uint32 enchant : team)
                    if (enchant)
                        wanted.insert(enchant);

        for (uint32 spellId = 1; spellId < sSpellMgr->GetSpellInfoStoreSize(); ++spellId)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            for (SpellEffectInfo const& effect : spellInfo->GetEffects())
                if (effect.IsEffect(SPELL_EFFECT_ENCHANT_ITEM) && effect.MiscValue > 0)
                    _enchantSpells.emplace(uint32(effect.MiscValue), spellId);   // keeps the lowest spell id
        }

        for (ItemTemplate const* proto : *sObjectMgr->GetItemTemplateStoreFast())
        {
            if (!proto || proto->Class == ITEM_CLASS_RECIPE || IsJunkName(proto->Name1))
                continue;

            int32 itemSpell = proto->Spells[0].SpellId;
            SpellInfo const* spellInfo = itemSpell > 0 ? sSpellMgr->GetSpellInfo(itemSpell) : nullptr;
            if (!spellInfo)
                continue;

            for (SpellEffectInfo const& effect : spellInfo->GetEffects())
                if (effect.IsEffect(SPELL_EFFECT_ENCHANT_ITEM) && wanted.count(uint32(effect.MiscValue)))
                    _enchantItems.emplace(uint32(effect.MiscValue), proto->ItemId);
        }
    }

    void BuildGearVendors()
    {
        LanConfig const& cfg = GetLanConfig();

        for (uint8 specIndex = 0; specIndex < LanSpecs.size(); ++specIndex)
        {
            LanSpecData const& spec = LanSpecs[specIndex];

            // Tier 6 for the starting areas, usable from level 1
            for (uint32 itemId : spec.T6)
            {
                if (!AddVendorItem(LAN_VENDOR_T6 + specIndex, itemId, cfg.GearPrice))
                    continue;

                ItemTemplate* proto = const_cast<ItemTemplate*>(sObjectMgr->GetItemTemplate(itemId));
                proto->RequiredLevel = 0;

                // Death knights wear the warrior Onslaught set (TBC has no death knight tier)
                proto->AllowableClass |= 1 << (spec.ClassId - 1);
            }

            // Pathfinder Maren (adventure route): head to feet plus neck and back; the route quests give the
            // weapons, rings and trinkets
            for (uint8 slot = 0; slot <= 9; ++slot)
                AddVendorItem(LAN_VENDOR_T6_ARMOR + specIndex, spec.T6[slot], cfg.GearPrice);

            // Tier 9 with its gems and enchant items, once per faction
            for (uint8 team = 0; team < 2; ++team)
            {
                uint32 vendor = LAN_VENDOR_T9 + specIndex * 2 + team;
                for (uint32 itemId : spec.T9[team])
                    AddVendorItem(vendor, itemId, cfg.GearPrice);
                for (uint32 gemId : spec.T9Gems[team])
                    AddVendorItem(vendor, gemId, cfg.GearPrice);
                for (uint32 enchant : spec.T9Enchant[team])
                {
                    auto itr = _enchantItems.find(enchant);
                    if (itr == _enchantItems.end() || !AddVendorItem(vendor, itr->second, cfg.GearPrice))
                        continue;

                    // Arcanums and shoulder inscriptions need reputation and have no alternative without it
                    ItemTemplate* proto = const_cast<ItemTemplate*>(sObjectMgr->GetItemTemplate(itr->second));
                    proto->RequiredReputationFaction = 0;
                    proto->RequiredReputationRank = 0;
                }
            }
        }

        for (uint32 bagId : BiggestBags)
            AddVendorItem(LAN_VENDOR_BAGS, bagId, cfg.GearPrice);

        // Arrows and bullets at the bag vendors (next to the quivers and ammo pouches) and at the Dalaran food
        // vendor, priced like food
        for (uint32 ammoId : BestAmmo)
        {
            bool bags = AddVendorItem(LAN_VENDOR_BAGS, ammoId, cfg.FoodPrice);
            bool dalaran = AddVendorItem(LAN_VENDOR_FOOD, ammoId, cfg.FoodPrice);
            if (!bags && !dalaran)
                continue;

            ItemTemplate* proto = const_cast<ItemTemplate*>(sObjectMgr->GetItemTemplate(ammoId));
            proto->RequiredLevel = 0;
        }

        // Pathfinder Maren after the Hellfire Ramparts: level 80 blue gear, usable at any level
        for (uint32 category = 0; category < LanLevel80Gear.size(); ++category)
        {
            LanLevel80Category const& gear = LanLevel80Gear[category];
            for (std::size_t i = 0; i < gear.Count; ++i)
            {
                if (!AddVendorItem(LAN_VENDOR_LEVEL80 + category, gear.Items[i], cfg.GearPrice))
                    continue;

                ItemTemplate* proto = const_cast<ItemTemplate*>(sObjectMgr->GetItemTemplate(gear.Items[i]));
                proto->RequiredLevel = 0;
            }
        }

        // The LAN party sword, free at the bag vendor
        if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(ITEM_NIGHTBLADE))
        {
            ItemTemplate* nightblade = const_cast<ItemTemplate*>(proto);
            nightblade->RequiredLevel = 0;
            nightblade->Delay = 500;                    // 0.5 speed
            nightblade->Damage[0].DamageMin = 3000.0f;
            nightblade->Damage[0].DamageMax = 4000.0f;

            constexpr std::array<uint32, 5> AllStats =
            {
                ITEM_MOD_STRENGTH, ITEM_MOD_AGILITY, ITEM_MOD_STAMINA, ITEM_MOD_INTELLECT, ITEM_MOD_SPIRIT
            };
            for (uint8 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
            {
                nightblade->ItemStat[i].ItemStatType = i < AllStats.size() ? AllStats[i] : 0;
                nightblade->ItemStat[i].ItemStatValue = i < AllStats.size() ? 1000 : 0;
            }
            nightblade->StatsCount = AllStats.size();

            AddVendorItem(LAN_VENDOR_BAGS, ITEM_NIGHTBLADE, 0);
        }

        for (uint32 foodId : BestFood)
            AddVendorItem(LAN_VENDOR_FOOD, foodId, cfg.FoodPrice);

        // Starting areas: the same food plus potions, elixirs and flasks, all usable from level 1
        auto addConsumables = [&cfg](uint32 category, auto const& items)
        {
            for (uint32 itemId : items)
            {
                if (!AddVendorItem(uint32(LAN_VENDOR_CONSUMABLES) + category, itemId, cfg.FoodPrice))
                    continue;

                ItemTemplate* proto = const_cast<ItemTemplate*>(sObjectMgr->GetItemTemplate(itemId));
                proto->RequiredLevel = 0;
            }
        };
        addConsumables(CONSUMABLE_POTIONS, BestPotions);
        addConsumables(CONSUMABLE_FOOD, BestFood);
        addConsumables(CONSUMABLE_ELIXIRS, BestElixirs);
        addConsumables(CONSUMABLE_FLASKS, BestFlasks);

        // Poisons, reagents and thrown weapons: always free and usable at any level. Apothecary Brennick in the
        // starting areas, Pack Merchant Dunstan (thrown, next to the ammo) and Chef Rosalind in Dalaran (all three).
        auto addFree = [](uint32 vendor, auto const& items)
        {
            for (uint32 itemId : items)
            {
                if (!AddVendorItem(vendor, itemId, 0))
                    continue;

                ItemTemplate* proto = const_cast<ItemTemplate*>(sObjectMgr->GetItemTemplate(itemId));
                proto->RequiredLevel = 0;
            }
        };
        addFree(uint32(LAN_VENDOR_CONSUMABLES) + CONSUMABLE_POISONS, BestPoisons);
        addFree(uint32(LAN_VENDOR_CONSUMABLES) + CONSUMABLE_REAGENTS, Reagents);
        addFree(LAN_VENDOR_BAGS, BestThrown);
        addFree(LAN_VENDOR_FOOD, BestThrown);
        addFree(LAN_VENDOR_FOOD, BestPoisons);
        addFree(LAN_VENDOR_FOOD, Reagents);

        // Starting areas: Burning Crusade gems, no level requirement
        for (LanGem const& gem : BestGems)
        {
            if (!AddVendorItem(uint32(LAN_VENDOR_GEMS) + gem.Color, gem.ItemId, cfg.GemPrice))
                continue;

            ItemTemplate* proto = const_cast<ItemTemplate*>(sObjectMgr->GetItemTemplate(gem.ItemId));
            proto->RequiredLevel = 0;
        }

        // Mounts, usable at any level and riding skill
        auto addMounts = [&cfg](uint32 list, auto const& items)
        {
            for (uint32 itemId : items)
            {
                if (!AddVendorItem(uint32(LAN_VENDOR_MOUNTS) + list, itemId, cfg.MountPrice))
                    continue;

                ItemTemplate* proto = const_cast<ItemTemplate*>(sObjectMgr->GetItemTemplate(itemId));
                proto->RequiredLevel = 0;
                proto->RequiredSkill = 0;
                proto->RequiredSkillRank = 0;
            }
        };
        addMounts(0, GroundMounts);
        addMounts(1, FlyingMounts);
    }

    void BuildProfessionVendors()
    {
        // Spells of every profession
        std::unordered_map<uint32, std::size_t> spellProfession;
        for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); ++i)
        {
            SkillLineAbilityEntry const* ability = sSkillLineAbilityStore.LookupEntry(i);
            if (!ability)
                continue;

            for (std::size_t p = 0; p < LanProfessions.size(); ++p)
                if (ability->SkillLine == LanProfessions[p].SkillId)
                    spellProfession.emplace(ability->Spell, p);
        }

        std::array<std::set<uint32>, LanProfessions.size()> reagents;
        std::array<std::set<uint32>, LanProfessions.size()> tools;
        std::array<std::set<uint32>, LanProfessions.size()> totemCategories;
        for (auto const& [spellId, p] : spellProfession)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            for (int32 reagent : spellInfo->Reagent)
                if (reagent > 0)
                    reagents[p].insert(reagent);
            for (uint32 totem : spellInfo->Totem)
                if (totem)
                    tools[p].insert(totem);
            for (uint32 category : spellInfo->TotemCategory)
                if (category)
                    totemCategories[p].insert(category);
        }

        // Recipes: items that teach a spell of the profession. Reputation requirements are removed.
        std::array<std::vector<ItemTemplate const*>, LanProfessions.size()> recipes;
        for (ItemTemplate const* proto : *sObjectMgr->GetItemTemplateStoreFast())
        {
            if (!proto || IsJunkName(proto->Name1))
                continue;

            for (std::size_t p = 0; p < LanProfessions.size(); ++p)
            {
                if (proto->TotemCategory && totemCategories[p].count(proto->TotemCategory))
                    tools[p].insert(proto->ItemId);

                if (proto->Class == ITEM_CLASS_TRADE_GOODS &&
                    ((p == 4 && proto->SubClass == ITEM_SUBCLASS_HERB) ||                  // Herbalism
                     (p == 8 && proto->SubClass == ITEM_SUBCLASS_METAL_STONE) ||           // Mining
                     (p == 9 && proto->SubClass == ITEM_SUBCLASS_LEATHER)) &&              // Skinning
                    proto->Quality >= ITEM_QUALITY_NORMAL)
                    reagents[p].insert(proto->ItemId);

                bool fishingPole = proto->Class == ITEM_CLASS_WEAPON &&
                    proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
                if (p == 13 && fishingPole)
                    tools[p].insert(proto->ItemId);
            }

            if (proto->Class != ITEM_CLASS_RECIPE)
                continue;

            for (_Spell const& itemSpell : proto->Spells)
            {
                if (itemSpell.SpellId <= 0)
                    continue;

                auto itr = spellProfession.find(itemSpell.SpellId);
                if (itr == spellProfession.end())
                    continue;

                recipes[itr->second].push_back(proto);
                ItemTemplate* editable = const_cast<ItemTemplate*>(proto);
                editable->RequiredReputationFaction = 0;
                editable->RequiredReputationRank = 0;
                break;
            }
        }

        // Pick axe and skinning knife for the gathering professions
        tools[8].insert(2901);
        tools[9].insert(7005);

        for (std::size_t p = 0; p < LanProfessions.size(); ++p)
        {
            uint32 base = LAN_VENDOR_PROFESSION + p * 100;

            std::sort(recipes[p].begin(), recipes[p].end(), [](ItemTemplate const* a, ItemTemplate const* b)
            {
                return std::tie(a->RequiredSkillRank, a->Name1) < std::tie(b->RequiredSkillRank, b->Name1);
            });
            std::vector<uint32> recipeIds;
            for (ItemTemplate const* proto : recipes[p])
                recipeIds.push_back(proto->ItemId);

            auto sortByLevel = [](std::set<uint32> const& ids)
            {
                std::vector<ItemTemplate const*> protos;
                for (uint32 id : ids)
                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(id); proto && !IsJunkName(proto->Name1))
                        protos.push_back(proto);
                std::sort(protos.begin(), protos.end(), [](ItemTemplate const* a, ItemTemplate const* b)
                {
                    return std::tie(a->ItemLevel, a->Name1) < std::tie(b->ItemLevel, b->Name1);
                });
                std::vector<uint32> result;
                for (ItemTemplate const* proto : protos)
                    result.push_back(proto->ItemId);
                return result;
            };

            _professionPages[p].Recipes = AddPagedItems(base + PROFESSION_PAGES_RECIPES, recipeIds, 0,
                PROFESSION_PAGES_REAGENTS);
            _professionPages[p].Reagents = AddPagedItems(base + PROFESSION_PAGES_REAGENTS, sortByLevel(reagents[p]), 0,
                PROFESSION_PAGES_TOOLS - PROFESSION_PAGES_REAGENTS);
            _professionPages[p].Tools = AddPagedItems(base + PROFESSION_PAGES_TOOLS, sortByLevel(tools[p]), 0,
                100 - PROFESSION_PAGES_TOOLS);

            LOG_INFO("module", "mod-lan: {} vendor: {} recipes, {} reagents, {} tools", LanProfessions[p].Name,
                recipeIds.size(), reagents[p].size(), tools[p].size());
        }
    }

    void OpenVendor(Player* player, Creature* creature, uint32 vendorEntry)
    {
        CloseGossipMenuFor(player);
        player->GetSession()->SendListInventory(creature->GetGUID(), vendorEntry);
    }

    // Best enchant for the slot that needs no profession, for when the spec's enchant does.
    uint32 FallbackEnchant(uint8 gearSlot, uint32 enchant, LanStatType stat)
    {
        bool caster = stat == LAN_STAT_CASTER || stat == LAN_STAT_HEALER;
        switch (gearSlot)
        {
            case GEAR_SHOULDER:     // Master's Inscriptions -> Greater Inscriptions of the Sons of Hodir
                switch (enchant)
                {
                    case 3835: return 3808;     // Axe
                    case 3836: return 3809;     // Crag
                    case 3837: return 3811;     // Pinnacle
                    case 3838: return 3810;     // Storm
                    default:   return 0;
                }
            case GEAR_HEAD:         // Mind Amplification Dish -> Arcanum
                return caster ? (stat == LAN_STAT_HEALER ? 3819 : 3820) : (stat == LAN_STAT_TANK ? 3818 : 3817);
            case GEAR_BACK:         // embroideries and engineering cloak tinkers
                return caster ? 3831 : (stat == LAN_STAT_TANK ? 3294 : 1099);
            case GEAR_WRIST:        // fur linings and the blacksmith socket
                return caster ? 2332 : (stat == LAN_STAT_TANK ? 3850 : 3845);
            case GEAR_HANDS:        // engineering glove tinkers and the blacksmith socket
                return caster ? 3246 : (stat == LAN_STAT_TANK ? 3253 : 1603);
            case GEAR_FEET:         // Nitro Boosts
                return stat == LAN_STAT_HEALER ? 3244 : (stat == LAN_STAT_TANK ? 3232 : 3826);
            default:                // rings (enchanting only) and belt tinkers are skipped
                return 0;
        }
    }

    bool PlayerHasEnchantSkill(Player* player, SpellItemEnchantmentEntry const* entry)
    {
        return !entry->requiredSkill || player->GetSkillValue(entry->requiredSkill) >= entry->requiredSkillValue;
    }

    void ApplySpecEnchants(Player* player, uint8 specIndex)
    {
        LanSpecData const& spec = LanSpecs[specIndex];
        uint8 team = player->GetTeamId(true) == TEAM_HORDE ? 1 : 0;

        uint32 applied = 0;
        uint32 replaced = 0;
        std::vector<std::string> skipped;
        for (uint8 gearSlot = 0; gearSlot < LAN_GEAR_SLOTS; ++gearSlot)
        {
            uint32 enchant = spec.T9Enchant[team][gearSlot];
            if (!enchant)
                continue;

            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, GearSlotToEquipSlot[gearSlot]);
            if (!item)
                continue;

            SpellItemEnchantmentEntry const* entry = sSpellItemEnchantmentStore.LookupEntry(enchant);
            if (!entry)
                continue;

            if (!PlayerHasEnchantSkill(player, entry))
            {
                uint32 fallback = gearSlot == GEAR_WAIST ? 0 : FallbackEnchant(gearSlot, enchant, spec.Stat);
                SpellItemEnchantmentEntry const* fallbackEntry =
                    fallback ? sSpellItemEnchantmentStore.LookupEntry(fallback) : nullptr;
                if (!fallbackEntry || !PlayerHasEnchantSkill(player, fallbackEntry))
                {
                    skipped.push_back(item->GetTemplate()->Name1);
                    continue;
                }

                enchant = fallback;
                entry = fallbackEntry;
                ++replaced;
            }

            // The enchant has to fit the equipped item (e.g. two-hand weapon enchants, shield enchants)
            if (auto itr = _enchantSpells.find(enchant); itr != _enchantSpells.end())
                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->second);
                    spellInfo && !item->IsFitToSpellRequirements(spellInfo))
                {
                    skipped.push_back(item->GetTemplate()->Name1);
                    continue;
                }

            if (item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT) == enchant)
                continue;

            player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, false);
            item->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchant, 0, 0, player->GetGUID());
            player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, true);
            item->SetState(ITEM_CHANGED, player);
            ++applied;
        }

        ChatHandler handler(player->GetSession());
        handler.PSendSysMessage("Applied {} {} enchants to your gear "
            "({} replaced by an enchant that needs no profession).",
            applied, spec.SpecName, replaced);
        if (!skipped.empty())
        {
            std::string list;
            for (std::string const& name : skipped)
                list += (list.empty() ? "" : ", ") + name;
            handler.PSendSysMessage("Skipped: {}.", list);
        }
    }

    enum GossipActions : uint32
    {
        ACTION_MAIN             = 1,
        ACTION_CLASS            = 100,          // + class id
        ACTION_SPEC             = 200,          // + spec index
        ACTION_BROWSE           = 300,
        ACTION_ENCHANT          = 301,
        ACTION_PAGE             = 1000          // + page
    };

    char const* ClassNameOf(uint8 classId)
    {
        for (LanSpecData const& spec : LanSpecs)
            if (spec.ClassId == classId)
                return spec.ClassName;
        return "Unknown";
    }
}

void LanBuildVendors()
{
    BuildEnchantMaps();
    BuildGearVendors();
    BuildProfessionVendors();
    LOG_INFO("module", "mod-lan: vendor lists ready, {} enchant items found", _enchantItems.size());
}

bool LanIsVendorItem(uint32 itemId)
{
    return _vendorItems.count(itemId) != 0;
}

uint32 LanFindEnchantSpell(uint32 enchantId)
{
    auto itr = _enchantSpells.find(enchantId);
    return itr != _enchantSpells.end() ? itr->second : 0;
}

uint32 LanFindEnchantItem(uint32 enchantId)
{
    auto itr = _enchantItems.find(enchantId);
    return itr != _enchantItems.end() ? itr->second : 0;
}

// Dungeon finder bots at level 80: the spec's Tier 9 best-in-slot set (the bot's faction version), every
// socket filled with the spec's gems and every slot enchanted like the Dalaran quartermasters do it
bool LanEquipBestGear(Player* bot, uint8 specIndex)
{
    if (!bot || specIndex >= LanSpecs.size())
        return false;

    LanSpecData const& spec = LanSpecs[specIndex];
    uint8 team = bot->GetTeamId(true) == TEAM_HORDE ? 1 : 0;

    // Gems of the spec: the meta gem for meta sockets, a gem of the socket's color for the others, else the
    // spec's first regular gem
    std::vector<std::pair<uint32, uint32>> gems;                // enchant id, gem color mask
    for (uint32 gemId : spec.T9Gems[team])
    {
        ItemTemplate const* gem = gemId ? sObjectMgr->GetItemTemplate(gemId) : nullptr;
        GemPropertiesEntry const* properties = gem ? sGemPropertiesStore.LookupEntry(gem->GemProperties) : nullptr;
        if (properties && properties->spellitemenchantement)
            gems.emplace_back(properties->spellitemenchantement, properties->color);
    }

    auto gemFor = [&gems](uint32 socketColor) -> uint32
    {
        for (auto const& [enchant, color] : gems)
            if (color & socketColor)
                return enchant;
        if (socketColor == SOCKET_COLOR_META)
            return 0;
        for (auto const& [enchant, color] : gems)
            if (color != SOCKET_COLOR_META)
                return enchant;
        return 0;
    };

    // Empty every slot first, so e.g. a two-hander isn't blocked by an old off-hand
    for (uint8 equipSlot : GearSlotToEquipSlot)
        bot->DestroyItem(INVENTORY_SLOT_BAG_0, equipSlot, true);

    uint32 equipped = 0;
    for (uint8 gearSlot = 0; gearSlot < LAN_GEAR_SLOTS; ++gearSlot)
    {
        uint32 itemId = spec.T9[team][gearSlot];
        uint8 equipSlot = GearSlotToEquipSlot[gearSlot];
        if (!itemId)
            continue;

        uint16 dest = 0;
        if (bot->CanEquipNewItem(equipSlot, dest, itemId, false) != EQUIP_ERR_OK)
            continue;

        Item* item = bot->EquipNewItem(dest, itemId, true);
        if (!item)
            continue;

        ++equipped;
        ItemTemplate const* proto = item->GetTemplate();
        for (uint8 socket = 0; socket < MAX_GEM_SOCKETS; ++socket)
        {
            uint32 gem = proto->Socket[socket].Color ? gemFor(proto->Socket[socket].Color) : 0;
            if (!gem)
                continue;

            EnchantmentSlot slot = EnchantmentSlot(SOCK_ENCHANTMENT_SLOT + socket);
            bot->ApplyEnchantment(item, slot, false);
            item->SetEnchantment(slot, gem, 0, 0, bot->GetGUID());
            bot->ApplyEnchantment(item, slot, true);
        }

        uint32 enchant = spec.T9Enchant[team][gearSlot];
        SpellItemEnchantmentEntry const* entry = enchant ? sSpellItemEnchantmentStore.LookupEntry(enchant) : nullptr;
        if (entry && !PlayerHasEnchantSkill(bot, entry))
        {
            enchant = gearSlot == GEAR_WAIST ? 0 : FallbackEnchant(gearSlot, enchant, spec.Stat);
            entry = enchant ? sSpellItemEnchantmentStore.LookupEntry(enchant) : nullptr;
            if (entry && !PlayerHasEnchantSkill(bot, entry))
                entry = nullptr;
        }

        if (entry)
        {
            if (auto itr = _enchantSpells.find(enchant); itr != _enchantSpells.end())
                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->second);
                    spellInfo && !item->IsFitToSpellRequirements(spellInfo))
                    entry = nullptr;
        }

        if (entry)
        {
            bot->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, false);
            item->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchant, 0, 0, bot->GetGUID());
            bot->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, true);
        }

        item->SetState(ITEM_CHANGED, bot);
    }

    bot->SaveToDB(false, false);
    return equipped > 0;
}

// Starting areas: pick a class and spec, then buy its Tier 6 set
class npc_lan_outfitter : public CreatureScript
{
public:
    npc_lan_outfitter() : CreatureScript("npc_lan_outfitter") { }

    static void ShowClasses(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);

        // The player's own class first
        std::vector<uint8> classes = { player->getClass() };
        for (LanSpecData const& spec : LanSpecs)
            if (std::find(classes.begin(), classes.end(), spec.ClassId) == classes.end())
                classes.push_back(spec.ClassId);

        for (uint8 classId : classes)
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, Acore::StringFormat("{} gear (Tier 6)", ClassNameOf(classId)),
                GOSSIP_SENDER_MAIN, ACTION_CLASS + classId);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ShowClasses(player, creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (action == ACTION_MAIN)
        {
            ShowClasses(player, creature);
            return true;
        }

        if (action >= ACTION_SPEC && action < ACTION_SPEC + LanSpecs.size())
        {
            OpenVendor(player, creature, LAN_VENDOR_T6 + action - ACTION_SPEC);
            return true;
        }

        if (action > ACTION_CLASS && action < ACTION_CLASS + MAX_CLASSES)
        {
            ClearGossipMenuFor(player);
            uint8 classId = action - ACTION_CLASS;
            for (uint8 i = 0; i < LanSpecs.size(); ++i)
                if (LanSpecs[i].ClassId == classId)
                    AddGossipItemFor(player, GOSSIP_ICON_VENDOR,
                        Acore::StringFormat("{} {}", LanSpecs[i].SpecName, LanSpecs[i].ClassName), GOSSIP_SENDER_MAIN,
                        ACTION_SPEC + i);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, ACTION_MAIN);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        CloseGossipMenuFor(player);
        return true;
    }
};

class npc_lan_bag_vendor : public CreatureScript
{
public:
    npc_lan_bag_vendor() : CreatureScript("npc_lan_bag_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        OpenVendor(player, creature, LAN_VENDOR_BAGS);
        return true;
    }
};

class npc_lan_food_vendor : public CreatureScript
{
public:
    npc_lan_food_vendor() : CreatureScript("npc_lan_food_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        OpenVendor(player, creature, LAN_VENDOR_FOOD);
        return true;
    }
};

// Starting areas: pick potions, food, elixirs or flasks
class npc_lan_consumables_vendor : public CreatureScript
{
public:
    npc_lan_consumables_vendor() : CreatureScript("npc_lan_consumables_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Potions", GOSSIP_SENDER_MAIN,
            uint32(ACTION_PAGE) + CONSUMABLE_POTIONS);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Food & drink", GOSSIP_SENDER_MAIN,
            uint32(ACTION_PAGE) + CONSUMABLE_FOOD);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Elixirs", GOSSIP_SENDER_MAIN,
            uint32(ACTION_PAGE) + CONSUMABLE_ELIXIRS);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Flasks", GOSSIP_SENDER_MAIN,
            uint32(ACTION_PAGE) + CONSUMABLE_FLASKS);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Poisons", GOSSIP_SENDER_MAIN,
            uint32(ACTION_PAGE) + CONSUMABLE_POISONS);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Reagents", GOSSIP_SENDER_MAIN,
            uint32(ACTION_PAGE) + CONSUMABLE_REAGENTS);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (action >= ACTION_PAGE && action < uint32(ACTION_PAGE) + CONSUMABLE_MAX)
        {
            OpenVendor(player, creature, uint32(LAN_VENDOR_CONSUMABLES) + action - ACTION_PAGE);
            return true;
        }

        CloseGossipMenuFor(player);
        return true;
    }
};

class npc_lan_gem_vendor : public CreatureScript
{
public:
    npc_lan_gem_vendor() : CreatureScript("npc_lan_gem_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);
        for (GemColor const& color : GemColors)
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, color.Name, GOSSIP_SENDER_MAIN,
                uint32(ACTION_PAGE) + color.Subclass);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (action >= ACTION_PAGE && action <= uint32(ACTION_PAGE) + ITEM_SUBCLASS_GEM_META)
        {
            OpenVendor(player, creature, uint32(LAN_VENDOR_GEMS) + action - ACTION_PAGE);
            return true;
        }

        CloseGossipMenuFor(player);
        return true;
    }
};

class npc_lan_mount_vendor : public CreatureScript
{
public:
    npc_lan_mount_vendor() : CreatureScript("npc_lan_mount_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Ground mounts", GOSSIP_SENDER_MAIN, ACTION_PAGE);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Flying mounts", GOSSIP_SENDER_MAIN, uint32(ACTION_PAGE) + 1);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (action == ACTION_PAGE || action == uint32(ACTION_PAGE) + 1)
        {
            OpenVendor(player, creature, uint32(LAN_VENDOR_MOUNTS) + action - ACTION_PAGE);
            return true;
        }

        CloseGossipMenuFor(player);
        return true;
    }
};

// Dalaran: one per spec, Tier 9 gear in the player's faction version plus gems and enchants
class npc_lan_spec_vendor : public CreatureScript
{
public:
    npc_lan_spec_vendor() : CreatureScript("npc_lan_spec_vendor") { }

    static uint8 SpecIndexOf(Creature* creature)
    {
        uint32 index = creature->GetEntry() - NPC_LAN_SPEC_VENDOR;
        return index < LanSpecs.size() ? index : 0;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        LanSpecData const& spec = LanSpecs[SpecIndexOf(creature)];
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR,
            Acore::StringFormat("Show me the {} {} gear, gems and enchants", spec.SpecName, spec.ClassName),
            GOSSIP_SENDER_MAIN, ACTION_BROWSE);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,
            Acore::StringFormat("Put the {} enchants on my equipped gear", spec.SpecName), GOSSIP_SENDER_MAIN,
            ACTION_ENCHANT, "Replace the permanent enchants on your equipped gear?", 0, false);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        uint8 specIndex = SpecIndexOf(creature);
        if (action == ACTION_BROWSE)
        {
            uint8 team = player->GetTeamId(true) == TEAM_HORDE ? 1 : 0;
            OpenVendor(player, creature, LAN_VENDOR_T9 + specIndex * 2 + team);
            return true;
        }

        CloseGossipMenuFor(player);
        if (action == ACTION_ENCHANT)
            ApplySpecEnchants(player, specIndex);
        return true;
    }
};

// Antonidas Memorial: everything a profession uses, for free
class npc_lan_profession_vendor : public CreatureScript
{
public:
    npc_lan_profession_vendor() : CreatureScript("npc_lan_profession_vendor") { }

    static std::size_t ProfessionOf(Creature* creature)
    {
        uint32 index = creature->GetEntry() - NPC_LAN_PROFESSION_VENDOR;
        return index < LanProfessions.size() ? index : 0;
    }

    static void AddPages(Player* player, char const* label, uint32 firstPage, uint32 pages)
    {
        for (uint32 page = 0; page < pages; ++page)
        {
            std::string text = pages > 1 ? Acore::StringFormat("{} ({}/{})", label, page + 1, pages)
                                         : std::string(label);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, text, GOSSIP_SENDER_MAIN, ACTION_PAGE + firstPage + page);
        }
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ProfessionPages const& pages = _professionPages[ProfessionOf(creature)];
        ClearGossipMenuFor(player);
        AddPages(player, "Recipes", PROFESSION_PAGES_RECIPES, pages.Recipes);
        AddPages(player, "Reagents", PROFESSION_PAGES_REAGENTS, pages.Reagents);
        AddPages(player, "Tools", PROFESSION_PAGES_TOOLS, pages.Tools);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (action >= ACTION_PAGE && action < ACTION_PAGE + 100)
        {
            OpenVendor(player, creature, LAN_VENDOR_PROFESSION + ProfessionOf(creature) * 100 + action - ACTION_PAGE);
            return true;
        }

        CloseGossipMenuFor(player);
        return true;
    }
};

void AddSC_lan_vendors()
{
    new npc_lan_outfitter();
    new npc_lan_bag_vendor();
    new npc_lan_food_vendor();
    new npc_lan_consumables_vendor();
    new npc_lan_gem_vendor();
    new npc_lan_mount_vendor();
    new npc_lan_spec_vendor();
    new npc_lan_profession_vendor();
}
