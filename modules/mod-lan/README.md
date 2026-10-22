# mod-lan

LAN party features for this AzerothCore + mod-playerbots server. Every option lives in
`conf/mod_lan.conf.dist`; the NPCs are created by `data/sql/db-world/mod_lan_npcs.sql`, which the
database updater applies on the next worldserver start.

## Leveling and start

- 200x experience for kills, quests and exploration (`Lan.XPRate`, overrides `Rate.XP.*`).
- Professions, weapon skills and defense level 4 times as fast: 4 points per skill-up (`Lan.SkillGain`).
- 3 talent points per level instead of 1 (`Lan.TalentRate`).
- No resurrection sickness (`Lan.NoResurrectionSickness`).
- New characters start with 150 gold (`Lan.StartGold`, overrides `StartPlayerMoney`).
- Class trainer spells, class quest spells (and the shaman totems) and riding up to Cold Weather Flying
  are learned automatically, for free, on login and level-up. Riding trainers are free as well.
- Armor and weapon proficiencies the class can use are learned at once, so the starter gear fits from level 1.
- Raids can be entered without a raid group (`Lan.SoloRaids`, sets `Instance.IgnoreRaid`).
- Every character knows all flight paths of its faction and the neutral ones (`Lan.AllFlightPaths`).
- Hunter pets are always happy, so they never need feeding (`Lan.HunterPets.AlwaysHappy`).

## LAN party aura

With 3 or more real (non-bot) players online every real player has Blessing of Kings (+10% stats) and
gets 10% more experience.

## NPCs

| Where | NPC | What |
| --- | --- | --- |
| Every starting area (also Ebon Hold) | Gearwright Holloway | Pick class and spec, buy its full Tier 6 set (all 8 pieces: head, shoulders, chest, hands, legs, wrists, waist, feet) plus best-in-slot neck, back, rings, trinkets and weapons, no level requirement. Death knights get the warrior Onslaught set (DPS or tank version) |
| Every starting area | Pack Merchant Dunstan | The largest bag of every kind |
| Every starting area | Jeweler Saphira | Pick a color: every Burning Crusade epic cut gem (Crimson Spinel, Empyrean Sapphire, Lionseye, Shadowsong Amethyst, Seaspray Emerald, Pyrestone) and every cut meta gem, free, no level requirement |
| Every starting area and Krasus' Landing | Stablemaster Korrin | 10 ground and 10 flying mounts, free, usable from level 1 (every riding skill is learned at once) |
| Every starting area and every dungeon and raid entrance | Portal Keeper Ilsa | Teleports to every classic, TBC and WotLK dungeon and raid entrance, the 14 major cities (capitals, Shattrath, Dalaran, Booty Bay, Gadgetzan, Ratchet, Everlook) and the main hubs of every zone on all four continents |
| Every starting area | Apothecary Brennick | Pick potions, food, elixirs or flasks: the best of each for free, usable from level 1 |
| The 8 capitals and Krasus' Landing | Archmage Telvaris | Everything Portal Keeper Ilsa offers, plus every zone |
| Krasus' Landing | Chef Rosalind | The best food and drink for free: feasts, every level 80 Well Fed food, the best food of every pet diet, the best drinks |
| Krasus' Landing | Commander Valeria | Fills your group to a 10 or 25 player raid, adds a tank, healer or damage dealer bot, dismisses bots |
| Krasus' Landing | 33 Quartermasters | One per class and spec: Tier 9 (ToC 25H) best-in-slot from the WoWSims Phase 3 presets in your faction's version, with its gems and enchant items, and they enchant your equipped gear. Each one wears its spec's Tier 9 set (your faction's version) and weapons |
| Antonidas Memorial | 14 Suppliers | One per profession: every recipe, reagent and tool for free; recipe reputation requirements are removed |
| Antonidas Memorial | 14 Grand Masters | One per profession: every rank from Apprentice to Grand Master and every trainer recipe, at the normal price |

Gear and bags cost 1 gold each (`Lan.Vendors.GearPrice`). Items the vendors sell below their vendor value
can't be sold back (`Lan.Vendors.BlockSellBack`).

The enchanting quartermasters use the spec's enchants. An enchant that needs a profession you don't have is
replaced by the best one anyone can use (Greater Inscriptions for the Master's Inscriptions, Arcanum for the
Mind Amplification Dish, regular cloak, bracer, glove and boot enchants for the profession ones); ring enchants
and belt tinkers are skipped.

## Bots (mod-playerbots)

- `Lan.RandomBots = 100` random bots; `-1` keeps the playerbots.conf values.
- Dungeon finder: after 2 minutes in the queue (`Lan.LfgFill.Delay`) random bots fill the missing tank,
  healer and damage slots, set to the right level, spec and gear.
- Commander Valeria uses the addclass characters (`Playerbots.AddClassAccountPoolSize`), which get the role's
  spec and gear for your level.
- Bots grouped with a real player deal 25% more damage and healing, take 20% less damage, and the group uses
  free-for-all loot so they never roll.

## Gear data

`src/LanGearData.h` is generated. Tier 9 comes from the WoWSims WotLK Phase 3 presets, converted to the other
faction through `player_factionchange_items`. Tier 6 comes from the WoWSims TBC Phase 3 presets; the healer
specs (no TBC sim) are picked by healing stat weights from Phase 3 items, and the death knight specs use the
warrior plate sets without shields and ranged weapons.

Every list is then checked against the world database and fixed where needed: each piece is the class's
own armor type (plate, mail, leather, cloth), wearable by the class, available to the faction, and needs no
reputation or profession. Items that fail are replaced by the best item for the slot that passes (stat
weights per spec, same tier of content). Jewelcrafter-only Dragon's Eyes are replaced by the matching
regular gem. Arcanums and shoulder inscriptions have no version without reputation, so their reputation
requirement is removed.
