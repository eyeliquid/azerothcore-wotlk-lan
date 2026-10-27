# mod-lan

LAN party features for this AzerothCore + mod-playerbots server. Every option lives in
`conf/mod_lan.conf.dist`; the NPCs are created by `data/sql/db-world/mod_lan_npcs.sql`, which the
database updater applies on the next worldserver start.

## Leveling and start

- Professions, weapon skills and defense level 4 times as fast: 4 points per skill-up (`Lan.SkillGain`).
- 3 talent points per level instead of 1 (`Lan.TalentRate`).
- No resurrection sickness (`Lan.NoResurrectionSickness`).
- Always fully rested: kills give double experience, the rested bonus is topped up all the time (`Lan.AlwaysRested`).
- Three times as many enemies in the eight starting zones (`Lan.StartZones.SpawnMultiplier`): every normal
  mob up to level 15 gets two copies a few yards away. The quest kill counts stay the same.
- Hearthstone cooldown of 5 minutes instead of 60 (`Lan.HearthstoneCooldown`).
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
| Every starting area, Ebon Hold and Krasus' Landing | Pathfinder Maren | The adventure route (below): quests from level 1 to 80, the dungeon teleports and the Tier 6 token exchange |
| Every starting area | Pack Merchant Dunstan | The largest bag of every kind, and the Nightblade for free: 0.5 speed, 3000-4000 damage, +1000 to every stat, no level requirement |
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

## Adventure route

Pathfinder Maren (every starting area, Ebon Hold and Krasus' Landing) leads a new character from level 1 to 80.

1. **Starting area (level 1-20):** 6 quests, each "defeat X enemies" (any kill counts, also for the group).
   Quests 1-4 each reward a choice of six satchels, one per armor type: cloth, leather (agility or caster), mail
   (agility or caster) or plate. A satchel holds two pieces (helm and shoulders, chest and gloves, leggings and
   bracers, belt and boots: Dungeon Set 3 plus blue extras). Quest 5 gives a weapon, quest 6 an off-hand. All of it
   is usable from level 1. The quests bring you to level 4, 7, 10, 14, 17 and 20.
2. **Deadmines to the Shadow Labyrinth (level 20-72):** one quest per dungeon (Deadmines, Shadowfang Keep,
   Scarlet Monastery Armory, Uldaman, Zul'Farrak, Blackrock Depths, Stratholme, Hellfire Ramparts, Underbog,
   Shadow Labyrinth): kill the last boss. Each quest gives two Tier 6 tokens; Maren exchanges them for the items
   of the spec you pick (the 17 slots of the Tier 6 lists in `LanGearData.h`, plus three free-choice tokens).
   The Shadow Labyrinth quest ends with a teleport to Dalaran.
3. **Utgarde Keep to Utgarde Pinnacle (level 72-80):** 5 WotLK dungeons (Utgarde Keep, Nexus, Azjol-Nerub,
   Violet Hold, Utgarde Pinnacle), 120 Emblems of Heroism each (Naxxramas 10-player level gear at the Dalaran
   emblem quartermasters). The last quest teleports you into Naxxramas.

- Accepting a dungeon quest teleports you and every group member on the same map inside the dungeon; group
  members who can take the quest get it too. Maren's "Take me and my group to ..." does the same later.
- The boss kill counts for everyone in the instance, and Maren appears next to the boss for an hour
  (`Lan.Route.KeeperDespawn`). Quests can be turned in with any Maren.
- Every mob in the Scarlet Monastery drops The Scarlet Key (for the Armory door) for players who don't have
  it yet (`mod_lan_scarlet_key.sql`).
- Every mob in Zul'Farrak drops an Executioner's Key (for the troll cages before Chief Ukorz Sandscalp) for players
  who don't have one (`mod_lan_zulfarrak.sql`).
- Scarlet Commander Mograine fights alone: he no longer pulls every mob in the Cathedral hall along
  (`mod_lan_scarlet_key.sql`).
- Turning a quest in raises your level to the next dungeon's level (no level is ever taken away).
- Mobs in the 15 route dungeons don't attack on their own; they only fight back (`Lan.Route.PassiveMobs`).
- The dungeons scale to the group size through mod-autobalance (5-player dungeons are enabled for it).
- Death knights get "A New Road" in Ebon Hold: the gear and tokens of the starting area and the first six
  dungeons, level 60, and the Stratholme quest next.

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
