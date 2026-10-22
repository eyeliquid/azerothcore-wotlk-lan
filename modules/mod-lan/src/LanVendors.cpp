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

    // The largest bag of every kind (general, soul, herb, enchanting, engineering, gem, mining,
    // leatherworking, inscription, quiver, ammo pouch)
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
            }

            // Tier 9 with its gems and enchant items, once per faction
            for (uint8 team = 0; team < 2; ++team)
            {
                uint32 vendor = LAN_VENDOR_T9 + specIndex * 2 + team;
                for (uint32 itemId : spec.T9[team])
                    AddVendorItem(vendor, itemId, cfg.GearPrice);
                for (uint32 gemId : spec.T9Gems[team])
                    AddVendorItem(vendor, gemId, cfg.GearPrice);
                for (uint32 enchant : spec.T9Enchant[team])
                    if (auto itr = _enchantItems.find(enchant); itr != _enchantItems.end())
                        AddVendorItem(vendor, itr->second, cfg.GearPrice);
            }
        }

        for (uint32 bagId : BiggestBags)
            AddVendorItem(LAN_VENDOR_BAGS, bagId, cfg.GearPrice);

        for (uint32 foodId : BestFood)
            AddVendorItem(LAN_VENDOR_FOOD, foodId, cfg.FoodPrice);
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
    new npc_lan_spec_vendor();
    new npc_lan_profession_vendor();
}
