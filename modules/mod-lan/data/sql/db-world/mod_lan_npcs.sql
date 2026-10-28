-- mod-lan: NPC templates, trainers and spawns. Entries 7340001-7340199, spawn guids 7340001-7340399, trainers 7340-7353.

-- Remove the first version (entries 9100001-9100199), whose names game clients may have cached
DELETE FROM `creature` WHERE `id` BETWEEN 9100001 AND 9100199 AND `guid` BETWEEN 9100001 AND 9100199;
DELETE FROM `creature_template_model` WHERE `CreatureID` BETWEEN 9100001 AND 9100199;
DELETE FROM `creature_default_trainer` WHERE `CreatureId` BETWEEN 9100001 AND 9100199;
DELETE FROM `creature_template` WHERE `entry` BETWEEN 9100001 AND 9100199;
DELETE FROM `trainer_spell` WHERE `TrainerId` BETWEEN 9100 AND 9113;
DELETE FROM `trainer` WHERE `Id` BETWEEN 9100 AND 9113;

DELETE FROM `creature_template` WHERE `entry` BETWEEN 7340001 AND 7340199;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `unit_class`, `unit_flags`, `unit_flags2`, `type`, `flags_extra`, `ScriptName`) VALUES
(7340001, 'Gearwright Holloway', 'Tier 6 Outfitter', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_outfitter'),
(7340002, 'Pack Merchant Dunstan', 'Biggest Bags', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_bag_vendor'),
(7340003, 'Archmage Telvaris', 'LAN Teleporter', 80, 80, 2, 35, 1, 1, 768, 0, 7, 2, 'npc_lan_teleporter'),
(7340004, 'Commander Valeria', 'Bot Commander', 80, 80, 2, 35, 1, 1, 768, 0, 7, 2, 'npc_lan_commander_valeria'),
(7340005, 'Chef Rosalind', 'Food & Drink', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_food_vendor'),
(7340006, 'Apothecary Brennick', 'Potions, Food, Elixirs & Flasks', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_consumables_vendor'),
(7340007, 'Jeweler Saphira', 'Burning Crusade Gems', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_gem_vendor'),
(7340008, 'Stablemaster Korrin', 'Mounts', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_mount_vendor'),
(7340009, 'Portal Keeper Ilsa', 'Dungeon & Raid Teleporter', 80, 80, 2, 35, 1, 1, 768, 0, 7, 2, 'npc_lan_teleporter'),
(7340010, 'Alchemy Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340030, 'Alchemy Grand Master', 'Alchemy Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340011, 'Blacksmithing Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340031, 'Blacksmithing Grand Master', 'Blacksmithing Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340012, 'Enchanting Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340032, 'Enchanting Grand Master', 'Enchanting Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340013, 'Engineering Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340033, 'Engineering Grand Master', 'Engineering Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340014, 'Herbalism Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340034, 'Herbalism Grand Master', 'Herbalism Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340015, 'Inscription Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340035, 'Inscription Grand Master', 'Inscription Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340016, 'Jewelcrafting Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340036, 'Jewelcrafting Grand Master', 'Jewelcrafting Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340017, 'Leatherworking Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340037, 'Leatherworking Grand Master', 'Leatherworking Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340018, 'Mining Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340038, 'Mining Grand Master', 'Mining Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340019, 'Skinning Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340039, 'Skinning Grand Master', 'Skinning Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340020, 'Tailoring Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340040, 'Tailoring Grand Master', 'Tailoring Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340021, 'Cooking Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340041, 'Cooking Grand Master', 'Cooking Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340022, 'First Aid Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340042, 'First Aid Grand Master', 'First Aid Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340023, 'Fishing Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 0, 7, 2, 'npc_lan_profession_vendor'),
(7340043, 'Fishing Grand Master', 'Fishing Trainer', 80, 80, 2, 35, 80, 1, 768, 0, 7, 2, ''),
(7340050, 'Pathfinder Maren', 'LAN Adventure', 80, 80, 2, 35, 131, 1, 768, 0, 7, 2, 'npc_lan_guide'),
(7340051, 'Enemy defeated', '', 1, 1, 0, 35, 0, 1, 33554688, 0, 10, 128, ''),
(7340052, 'Gorefang', 'Beast of the Wilds', 6, 6, 0, 14, 0, 1, 512, 0, 1, 0, ''),
(7340100, 'Balance Druid Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340101, 'Feral (Cat) Druid Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340102, 'Feral (Bear) Druid Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340103, 'Restoration Druid Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340104, 'Blood (DPS) Death Knight Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340105, 'Blood (Tank) Death Knight Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340106, 'Frost Death Knight Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340107, 'Unholy Death Knight Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340108, 'Beast Mastery Hunter Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340109, 'Marksmanship Hunter Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340110, 'Survival Hunter Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340111, 'Arcane Mage Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340112, 'Fire Mage Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340113, 'Frostfire Mage Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340114, 'Frost Mage Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340115, 'Holy Paladin Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340116, 'Protection Paladin Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340117, 'Retribution Paladin Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340118, 'Discipline Priest Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340119, 'Holy Priest Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340120, 'Shadow Priest Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340121, 'Assassination Rogue Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340122, 'Combat Rogue Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340123, 'Subtlety Rogue Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340124, 'Elemental Shaman Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340125, 'Enhancement Shaman Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340126, 'Restoration Shaman Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340127, 'Affliction Warlock Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340128, 'Demonology Warlock Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340129, 'Destruction Warlock Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340130, 'Arms Warrior Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340131, 'Fury Warrior Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor'),
(7340132, 'Protection Warrior Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 16, 7, 2, 'npc_lan_spec_vendor');

DELETE FROM `creature_template_locale` WHERE `entry` BETWEEN 7340001 AND 7340199;

-- Gorefang (adventure route quest 4): an elite beast, tougher than its level. Immune to NPCs (unit_flags 512), so
-- guards and other NPCs leave it to the player.
UPDATE `creature_template` SET `rank` = 1, `HealthModifier` = 3, `DamageModifier` = 1, `family` = 1 WHERE `entry` = 7340052;

DELETE FROM `creature_template_model` WHERE `CreatureID` BETWEEN 7340001 AND 7340199;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(7340001, 0, 25611, 1, 1),
(7340002, 0, 10212, 1, 1),
(7340003, 0, 25166, 1, 1),
(7340004, 0, 24930, 1, 1),
(7340005, 0, 26330, 1, 1),
(7340006, 0, 25646, 1, 1),
(7340007, 0, 25882, 1, 1),
(7340008, 0, 25619, 1, 1),
(7340009, 0, 27448, 1, 1),
(7340010, 0, 28151, 1, 1),
(7340030, 0, 25947, 1, 1),
(7340011, 0, 28151, 1, 1),
(7340031, 0, 25947, 1, 1),
(7340012, 0, 28151, 1, 1),
(7340032, 0, 25947, 1, 1),
(7340013, 0, 28151, 1, 1),
(7340033, 0, 25947, 1, 1),
(7340014, 0, 28151, 1, 1),
(7340034, 0, 25947, 1, 1),
(7340015, 0, 28151, 1, 1),
(7340035, 0, 25947, 1, 1),
(7340016, 0, 28151, 1, 1),
(7340036, 0, 25947, 1, 1),
(7340017, 0, 28151, 1, 1),
(7340037, 0, 25947, 1, 1),
(7340018, 0, 28151, 1, 1),
(7340038, 0, 25947, 1, 1),
(7340019, 0, 28151, 1, 1),
(7340039, 0, 25947, 1, 1),
(7340020, 0, 28151, 1, 1),
(7340040, 0, 25947, 1, 1),
(7340021, 0, 28151, 1, 1),
(7340041, 0, 25947, 1, 1),
(7340022, 0, 28151, 1, 1),
(7340042, 0, 25947, 1, 1),
(7340023, 0, 28151, 1, 1),
(7340043, 0, 25947, 1, 1),
(7340050, 0, 28934, 1, 1),
(7340051, 0, 11686, 1, 1),
(7340052, 0, 903, 2, 1),
(7340100, 0, 56, 1, 1),
(7340101, 0, 55, 1, 1),
(7340102, 0, 59, 1, 1),
(7340103, 0, 56, 1, 1),
(7340104, 0, 49, 1, 1),
(7340105, 0, 51, 1, 1),
(7340106, 0, 15476, 1, 1),
(7340107, 0, 57, 1, 1),
(7340108, 0, 53, 1, 1),
(7340109, 0, 56, 1, 1),
(7340110, 0, 1478, 1, 1),
(7340111, 0, 50, 1, 1),
(7340112, 0, 15476, 1, 1),
(7340113, 0, 1563, 1, 1),
(7340114, 0, 49, 1, 1),
(7340115, 0, 15475, 1, 1),
(7340116, 0, 53, 1, 1),
(7340117, 0, 49, 1, 1),
(7340118, 0, 16126, 1, 1),
(7340119, 0, 50, 1, 1),
(7340120, 0, 58, 1, 1),
(7340121, 0, 15475, 1, 1),
(7340122, 0, 49, 1, 1),
(7340123, 0, 55, 1, 1),
(7340124, 0, 16125, 1, 1),
(7340125, 0, 51, 1, 1),
(7340126, 0, 60, 1, 1),
(7340127, 0, 57, 1, 1),
(7340128, 0, 1564, 1, 1),
(7340129, 0, 50, 1, 1),
(7340130, 0, 49, 1, 1),
(7340131, 0, 51, 1, 1),
(7340132, 0, 53, 1, 1);

-- Profession trainers teaching every rank (Apprentice to Grand Master) and recipe of the existing trainers
DELETE FROM `trainer` WHERE `Id` BETWEEN 7340 AND 7353;
INSERT INTO `trainer` (`Id`, `Type`, `Requirement`, `Greeting`) VALUES
(7340, 2, 0, 'I can teach you everything about alchemy, from Apprentice to Grand Master.'),
(7341, 2, 0, 'I can teach you everything about blacksmithing, from Apprentice to Grand Master.'),
(7342, 2, 0, 'I can teach you everything about enchanting, from Apprentice to Grand Master.'),
(7343, 2, 0, 'I can teach you everything about engineering, from Apprentice to Grand Master.'),
(7344, 2, 0, 'I can teach you everything about herbalism, from Apprentice to Grand Master.'),
(7345, 2, 0, 'I can teach you everything about inscription, from Apprentice to Grand Master.'),
(7346, 2, 0, 'I can teach you everything about jewelcrafting, from Apprentice to Grand Master.'),
(7347, 2, 0, 'I can teach you everything about leatherworking, from Apprentice to Grand Master.'),
(7348, 2, 0, 'I can teach you everything about mining, from Apprentice to Grand Master.'),
(7349, 2, 0, 'I can teach you everything about skinning, from Apprentice to Grand Master.'),
(7350, 2, 0, 'I can teach you everything about tailoring, from Apprentice to Grand Master.'),
(7351, 2, 0, 'I can teach you everything about cooking, from Apprentice to Grand Master.'),
(7352, 2, 0, 'I can teach you everything about first aid, from Apprentice to Grand Master.'),
(7353, 2, 0, 'I can teach you everything about fishing, from Apprentice to Grand Master.');
DELETE FROM `trainer_spell` WHERE `TrainerId` BETWEEN 7340 AND 7353;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7340, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 171) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7341, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 164) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7342, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 333) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7343, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 202) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7344, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 182) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7345, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 773) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7346, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 755) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7347, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 165) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7348, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 186) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7349, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 393) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7350, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 197) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7351, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 185) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7352, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 129) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 7353, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 7340 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 356) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
DELETE FROM `creature_default_trainer` WHERE `CreatureId` BETWEEN 7340030 AND 7340043;
INSERT INTO `creature_default_trainer` (`CreatureId`, `TrainerId`) VALUES
(7340030, 7340),
(7340031, 7341),
(7340032, 7342),
(7340033, 7343),
(7340034, 7344),
(7340035, 7345),
(7340036, 7346),
(7340037, 7347),
(7340038, 7348),
(7340039, 7349),
(7340040, 7350),
(7340041, 7351),
(7340042, 7352),
(7340043, 7353);

-- Quartermasters wear their spec's weapons; their armor is sent by mod-lan as a mirror image
DELETE FROM `creature_equip_template` WHERE `CreatureID` BETWEEN 7340001 AND 7340199;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `ItemID2`, `ItemID3`) VALUES
(7340100, 1, 47206, 47064, 0),
(7340101, 1, 47239, 0, 0),
(7340102, 1, 47130, 0, 0),
(7340103, 1, 47206, 47958, 0),
(7340104, 1, 47078, 0, 0),
(7340105, 1, 47515, 0, 0),
(7340106, 1, 47526, 47526, 0),
(7340107, 1, 47526, 47526, 0),
(7340108, 1, 47239, 0, 47521),
(7340109, 1, 47239, 0, 47521),
(7340110, 1, 47239, 0, 47521),
(7340111, 1, 46980, 47958, 47922),
(7340112, 1, 47517, 47064, 45294),
(7340113, 1, 47517, 47064, 45294),
(7340114, 1, 47517, 47958, 45294),
(7340115, 1, 46017, 47085, 0),
(7340116, 1, 46097, 46964, 0),
(7340117, 1, 47519, 0, 0),
(7340118, 1, 47206, 47146, 45294),
(7340119, 1, 47206, 47146, 47922),
(7340120, 1, 47206, 47064, 47922),
(7340121, 1, 46969, 46969, 47521),
(7340122, 1, 47156, 47001, 47521),
(7340123, 1, 47156, 46969, 45296),
(7340124, 1, 46980, 47085, 0),
(7340125, 1, 47206, 47156, 0),
(7340126, 1, 46017, 47085, 0),
(7340127, 1, 46980, 47958, 45294),
(7340128, 1, 46980, 47146, 45294),
(7340129, 1, 46980, 47958, 45294),
(7340130, 1, 47078, 0, 46995),
(7340131, 1, 47078, 47078, 46995),
(7340132, 1, 47506, 46964, 47660);

-- Pathfinder Maren (adventure route) took Gearwright Holloway's place in the starting areas and Ebon Hold; one more
-- stands at the Deadmines entrance, to buy the Tier 6 gear on the way in
DELETE FROM `creature` WHERE `id` BETWEEN 7340001 AND 7340199 AND `guid` BETWEEN 7340001 AND 7340399;
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `equipment_id`) VALUES
(7340001, 7340050, 0, 1, 1, -8944.950, -129.990, 84.530, 3.605, 300, 0),
(7340002, 7340002, 0, 1, 1, -8944.950, -134.990, 84.530, 2.678, 300, 0),
(7340003, 7340006, 0, 1, 1, -8942.950, -132.490, 84.530, 3.142, 300, 0),
(7340004, 7340007, 0, 1, 1, -8944.950, -126.990, 84.530, 3.975, 300, 0),
(7340005, 7340008, 0, 1, 1, -8944.950, -137.990, 84.530, 2.309, 300, 0),
(7340006, 7340009, 0, 1, 1, -8940.450, -132.490, 84.530, 3.142, 300, 0),
(7340007, 7340050, 0, 1, 1, -6235.089, 333.002, 383.760, 3.502, 300, 0),
(7340008, 7340002, 0, 1, 1, -6235.604, 328.028, 383.760, 2.575, 300, 0),
(7340009, 7340006, 0, 1, 1, -6233.357, 330.309, 383.760, 3.038, 300, 0),
(7340010, 7340007, 0, 1, 1, -6234.780, 335.986, 383.760, 3.871, 300, 0),
(7340011, 7340008, 0, 1, 1, -6235.913, 325.044, 383.760, 2.205, 300, 0),
(7340012, 7340009, 0, 1, 1, -6230.871, 330.051, 383.760, 3.038, 300, 0),
(7340013, 7340050, 0, 1, 1, 1671.123, 1678.131, 122.670, 0.032, 300, 0),
(7340014, 7340002, 0, 1, 1, 1673.214, 1682.672, 122.670, 5.388, 300, 0),
(7340015, 7340006, 0, 1, 1, 1670.352, 1681.238, 122.670, 5.852, 300, 0),
(7340016, 7340007, 0, 1, 1, 1669.868, 1675.406, 122.670, 0.401, 300, 0),
(7340017, 7340008, 0, 1, 1, 1674.469, 1685.397, 122.670, 5.019, 300, 0),
(7340018, 7340009, 0, 1, 1, 1668.081, 1682.284, 122.670, 5.852, 300, 0),
(7340019, 7340050, 1, 1, 1, -2912.580, -255.480, 54.000, 3.605, 300, 0),
(7340020, 7340002, 1, 1, 1, -2912.580, -260.480, 54.000, 2.678, 300, 0),
(7340021, 7340006, 1, 1, 1, -2910.580, -257.980, 54.000, 3.142, 300, 0),
(7340022, 7340007, 1, 1, 1, -2912.580, -252.480, 54.000, 3.975, 300, 0),
(7340023, 7340008, 1, 1, 1, -2912.580, -263.480, 54.000, 2.309, 300, 0),
(7340024, 7340009, 1, 1, 1, -2908.080, -257.980, 54.000, 3.142, 300, 0),
(7340025, 7340050, 1, 1, 1, -613.520, -4249.170, 39.720, 3.605, 300, 0),
(7340026, 7340002, 1, 1, 1, -613.520, -4254.170, 39.720, 2.678, 300, 0),
(7340027, 7340006, 1, 1, 1, -611.520, -4251.670, 39.720, 3.142, 300, 0),
(7340028, 7340007, 1, 1, 1, -613.520, -4246.170, 39.720, 3.975, 300, 0),
(7340029, 7340008, 1, 1, 1, -613.520, -4257.170, 39.720, 2.309, 300, 0),
(7340030, 7340009, 1, 1, 1, -609.020, -4251.670, 39.720, 3.142, 300, 0),
(7340031, 7340050, 1, 1, 1, 10316.850, 831.793, 1327.410, 3.022, 300, 0),
(7340032, 7340002, 1, 1, 1, 10314.097, 827.620, 1327.410, 2.095, 300, 0),
(7340033, 7340006, 1, 1, 1, 10317.143, 828.605, 1327.410, 2.558, 300, 0),
(7340034, 7340007, 1, 1, 1, 10318.502, 834.297, 1327.410, 3.391, 300, 0),
(7340035, 7340008, 1, 1, 1, 10312.445, 825.116, 1327.410, 1.725, 300, 0),
(7340036, 7340009, 1, 1, 1, 10319.230, 827.228, 1327.410, 2.558, 300, 0),
(7340037, 7340050, 530, 1, 1, -3966.260, -13928.053, 101.610, 5.685, 300, 0),
(7340038, 7340002, 530, 1, 1, -3961.895, -13925.616, 101.610, 4.758, 300, 0),
(7340039, 7340006, 530, 1, 1, -3965.052, -13925.088, 101.610, 5.222, 300, 0),
(7340040, 7340007, 530, 1, 1, -3968.880, -13929.515, 101.610, 6.055, 300, 0),
(7340041, 7340008, 530, 1, 1, -3959.275, -13924.153, 101.610, 4.389, 300, 0),
(7340042, 7340009, 530, 1, 1, -3966.271, -13922.905, 101.610, 5.222, 300, 0),
(7340043, 7340050, 530, 1, 1, 10354.507, -6359.968, 34.400, 2.642, 300, 0),
(7340044, 7340002, 530, 1, 1, 10350.402, -6362.822, 34.400, 1.715, 300, 0),
(7340045, 7340006, 530, 1, 1, 10353.596, -6363.037, 34.400, 2.178, 300, 0),
(7340046, 7340007, 530, 1, 1, 10356.970, -6358.255, 34.400, 3.011, 300, 0),
(7340047, 7340008, 530, 1, 1, 10347.939, -6364.535, 34.400, 1.345, 300, 0),
(7340048, 7340009, 530, 1, 1, 10355.024, -6365.090, 34.400, 2.178, 300, 0),
(7340049, 7340050, 609, 1, 4294967295, 2351.946, -5666.349, 427.030, 0.982, 300, 0),
(7340050, 7340002, 609, 1, 4294967295, 2349.468, -5662.006, 427.030, 0.055, 300, 0),
(7340051, 7340006, 609, 1, 4294967295, 2348.970, -5665.168, 427.030, 0.518, 300, 0),
(7340052, 7340007, 609, 1, 4294967295, 2353.432, -5668.955, 427.030, 1.351, 300, 0),
(7340053, 7340008, 609, 1, 4294967295, 2347.982, -5659.400, 427.030, 5.969, 300, 0),
(7340054, 7340009, 609, 1, 4294967295, 2346.798, -5666.407, 427.030, 0.518, 300, 0),
(7340055, 7340003, 0, 1, 1, -8831.940, 631.262, 95.010, 4.212, 300, 0),
(7340056, 7340003, 0, 1, 1, -4916.930, -942.690, 502.560, 2.278, 300, 0),
(7340057, 7340003, 1, 1, 1, 9949.472, 2287.209, 1342.400, 4.742, 300, 0),
(7340058, 7340003, 530, 1, 1, -3963.720, -11651.346, -137.840, 3.992, 300, 0),
(7340059, 7340003, 1, 1, 1, 1627.306, -4375.230, 32.560, 0.558, 300, 0),
(7340060, 7340003, 0, 1, 1, 1587.138, 240.430, -51.150, 3.182, 300, 0),
(7340061, 7340003, 1, 1, 1, -1275.912, 122.178, 132.290, 2.078, 300, 0),
(7340062, 7340003, 530, 1, 1, 9490.667, -7279.569, 15.290, 3.018, 300, 0),
(7340063, 7340003, 571, 1, 1, 5826.000, 449.000, 659.500, 4.481, 300, 0),
(7340064, 7340004, 571, 1, 1, 5818.500, 448.500, 659.500, 4.921, 300, 0),
(7340065, 7340005, 571, 1, 1, 5833.000, 446.000, 659.500, 4.046, 300, 0),
(7340066, 7340008, 571, 1, 1, 5838.000, 452.000, 659.500, 4.038, 300, 0),
(7340067, 7340100, 571, 1, 1, 5829.000, 432.000, 659.000, 3.142, 300, 1),
(7340068, 7340101, 571, 1, 1, 5828.198, 435.253, 659.000, 3.625, 300, 1),
(7340069, 7340102, 571, 1, 1, 5825.976, 437.761, 659.000, 4.108, 300, 1),
(7340070, 7340103, 571, 1, 1, 5822.844, 438.949, 659.000, 4.592, 300, 1),
(7340071, 7340104, 571, 1, 1, 5819.518, 438.545, 659.000, 5.075, 300, 1),
(7340072, 7340105, 571, 1, 1, 5816.760, 436.642, 659.000, 5.558, 300, 1),
(7340073, 7340106, 571, 1, 1, 5815.203, 433.675, 659.000, 6.042, 300, 1),
(7340074, 7340107, 571, 1, 1, 5815.203, 430.325, 659.000, 0.242, 300, 1),
(7340075, 7340108, 571, 1, 1, 5816.760, 427.358, 659.000, 0.725, 300, 1),
(7340076, 7340109, 571, 1, 1, 5819.518, 425.455, 659.000, 1.208, 300, 1),
(7340077, 7340110, 571, 1, 1, 5822.844, 425.051, 659.000, 1.692, 300, 1),
(7340078, 7340111, 571, 1, 1, 5825.976, 426.239, 659.000, 2.175, 300, 1),
(7340079, 7340112, 571, 1, 1, 5828.198, 428.747, 659.000, 2.658, 300, 1),
(7340080, 7340113, 571, 1, 1, 5833.852, 433.877, 659.000, 3.299, 300, 1),
(7340081, 7340114, 571, 1, 1, 5832.692, 437.448, 659.000, 3.613, 300, 1),
(7340082, 7340115, 571, 1, 1, 5830.485, 440.485, 659.000, 3.927, 300, 1),
(7340083, 7340116, 571, 1, 1, 5827.448, 442.692, 659.000, 4.241, 300, 1),
(7340084, 7340117, 571, 1, 1, 5823.877, 443.852, 659.000, 4.555, 300, 1),
(7340085, 7340118, 571, 1, 1, 5820.123, 443.852, 659.000, 4.869, 300, 1),
(7340086, 7340119, 571, 1, 1, 5816.552, 442.692, 659.000, 5.184, 300, 1),
(7340087, 7340120, 571, 1, 1, 5813.515, 440.485, 659.000, 5.498, 300, 1),
(7340088, 7340121, 571, 1, 1, 5811.308, 437.448, 659.000, 5.812, 300, 1),
(7340089, 7340122, 571, 1, 1, 5810.148, 433.877, 659.000, 6.126, 300, 1),
(7340090, 7340123, 571, 1, 1, 5810.148, 430.123, 659.000, 0.157, 300, 1),
(7340091, 7340124, 571, 1, 1, 5811.308, 426.552, 659.000, 0.471, 300, 1),
(7340092, 7340125, 571, 1, 1, 5813.515, 423.515, 659.000, 0.785, 300, 1),
(7340093, 7340126, 571, 1, 1, 5816.552, 421.308, 659.000, 1.100, 300, 1),
(7340094, 7340127, 571, 1, 1, 5820.123, 420.148, 659.000, 1.414, 300, 1),
(7340095, 7340128, 571, 1, 1, 5823.877, 420.148, 659.000, 1.728, 300, 1),
(7340096, 7340129, 571, 1, 1, 5827.448, 421.308, 659.000, 2.042, 300, 1),
(7340097, 7340130, 571, 1, 1, 5830.485, 423.515, 659.000, 2.356, 300, 1),
(7340098, 7340131, 571, 1, 1, 5832.692, 426.552, 659.000, 2.670, 300, 1),
(7340099, 7340132, 571, 1, 1, 5833.852, 430.123, 659.000, 2.985, 300, 1),
(7340100, 7340030, 571, 1, 1, 5957.400, 671.000, 643.000, 3.142, 300, 0),
(7340101, 7340010, 571, 1, 1, 5961.124, 673.448, 643.000, 3.366, 300, 0),
(7340102, 7340031, 571, 1, 1, 5956.707, 674.037, 643.000, 3.590, 300, 0),
(7340103, 7340011, 571, 1, 1, 5959.000, 677.858, 643.000, 3.815, 300, 0),
(7340104, 7340032, 571, 1, 1, 5954.764, 676.473, 643.000, 4.039, 300, 0),
(7340105, 7340012, 571, 1, 1, 5955.173, 680.911, 643.000, 4.264, 300, 0),
(7340106, 7340033, 571, 1, 1, 5951.958, 677.824, 643.000, 4.488, 300, 0),
(7340107, 7340013, 571, 1, 1, 5950.400, 682.000, 643.000, 4.712, 300, 0),
(7340108, 7340034, 571, 1, 1, 5948.842, 677.824, 643.000, 4.937, 300, 0),
(7340109, 7340014, 571, 1, 1, 5945.627, 680.911, 643.000, 5.161, 300, 0),
(7340110, 7340035, 571, 1, 1, 5946.036, 676.473, 643.000, 5.386, 300, 0),
(7340111, 7340015, 571, 1, 1, 5941.800, 677.858, 643.000, 5.610, 300, 0),
(7340112, 7340036, 571, 1, 1, 5944.093, 674.037, 643.000, 5.834, 300, 0),
(7340113, 7340016, 571, 1, 1, 5939.676, 673.448, 643.000, 6.059, 300, 0),
(7340114, 7340037, 571, 1, 1, 5943.400, 671.000, 643.000, 0.000, 300, 0),
(7340115, 7340017, 571, 1, 1, 5939.676, 668.552, 643.000, 0.224, 300, 0),
(7340116, 7340038, 571, 1, 1, 5944.093, 667.963, 643.000, 0.449, 300, 0),
(7340117, 7340018, 571, 1, 1, 5941.800, 664.142, 643.000, 0.673, 300, 0),
(7340118, 7340039, 571, 1, 1, 5946.036, 665.527, 643.000, 0.898, 300, 0),
(7340119, 7340019, 571, 1, 1, 5945.627, 661.089, 643.000, 1.122, 300, 0),
(7340120, 7340040, 571, 1, 1, 5948.842, 664.176, 643.000, 1.346, 300, 0),
(7340121, 7340020, 571, 1, 1, 5950.400, 660.000, 643.000, 1.571, 300, 0),
(7340122, 7340041, 571, 1, 1, 5951.958, 664.176, 643.000, 1.795, 300, 0),
(7340123, 7340021, 571, 1, 1, 5955.173, 661.089, 643.000, 2.020, 300, 0),
(7340124, 7340042, 571, 1, 1, 5954.764, 665.527, 643.000, 2.244, 300, 0),
(7340125, 7340022, 571, 1, 1, 5959.000, 664.142, 643.000, 2.468, 300, 0),
(7340126, 7340043, 571, 1, 1, 5956.707, 667.963, 643.000, 2.693, 300, 0),
(7340127, 7340023, 571, 1, 1, 5961.124, 668.552, 643.000, 2.917, 300, 0),
(7340128, 7340009, 1, 1, 1, 1814.430, -4409.094, -17.970, 3.629, 300, 0),
(7340129, 7340009, 1, 1, 1, -732.671, -2221.196, 17.530, 1.209, 300, 0),
(7340130, 7340009, 0, 1, 1, -11211.694, 1673.702, 25.140, 6.222, 300, 0),
(7340131, 7340009, 0, 1, 1, -237.517, 1562.604, 77.390, 5.952, 300, 0),
(7340132, 7340009, 1, 1, 1, 4247.070, 740.786, -25.170, 6.052, 300, 0),
(7340133, 7340009, 0, 1, 1, -8781.716, 836.738, 95.180, 5.362, 300, 0),
(7340134, 7340009, 0, 1, 1, -5166.540, 925.422, 257.680, 6.282, 300, 0),
(7340135, 7340009, 1, 1, 1, -4473.030, -1676.572, 81.890, 5.872, 300, 0),
(7340136, 7340009, 0, 1, 1, 2875.421, -763.378, 160.830, 3.489, 300, 0),
(7340137, 7340009, 1, 1, 1, -4654.339, -2519.835, 81.550, 2.979, 300, 0),
(7340138, 7340009, 0, 1, 1, -6071.430, -2952.161, 210.280, 4.732, 300, 0),
(7340139, 7340009, 1, 1, 1, -6801.668, -2890.058, 9.500, 4.872, 300, 0),
(7340140, 7340009, 1, 1, 1, -1185.560, 2880.660, 86.290, 3.499, 300, 0),
(7340141, 7340009, 0, 1, 1, -10177.120, -3992.003, -110.740, 4.449, 300, 0),
(7340142, 7340009, 0, 1, 1, -7176.563, -920.076, 166.320, 3.529, 300, 0),
(7340143, 7340009, 0, 1, 1, -7524.553, -1225.107, 286.230, 3.729, 300, 0),
(7340144, 7340009, 1, 1, 1, -3977.800, 789.023, 161.510, 3.149, 300, 0),
(7340145, 7340009, 1, 1, 1, -3827.805, 1247.227, 160.730, 1.639, 300, 0),
(7340146, 7340009, 1, 1, 1, -3518.290, 1085.253, 161.600, 3.159, 300, 0),
(7340147, 7340009, 0, 1, 1, 1267.897, -2553.768, 94.110, 5.332, 300, 0),
(7340148, 7340009, 0, 1, 1, 3352.990, -3376.031, 145.280, 4.689, 300, 0),
(7340149, 7340009, 0, 1, 1, -7514.749, -1041.549, 182.050, 5.622, 300, 0),
(7340150, 7340009, 1, 1, 1, -4706.605, -3730.135, 55.060, 2.159, 300, 0),
(7340151, 7340009, 0, 1, 1, -11913.700, -1215.697, 92.790, 3.149, 300, 0),
(7340152, 7340009, 1, 1, 1, -8411.567, 1496.621, 28.220, 0.949, 300, 0),
(7340153, 7340009, 1, 1, 1, -8242.513, 1993.089, 129.570, 5.652, 300, 0),
(7340154, 7340009, 530, 1, 1, -363.518, 3070.959, -14.600, 0.319, 300, 0),
(7340155, 7340009, 530, 1, 1, -293.616, 3147.169, 32.050, 0.699, 300, 0),
(7340156, 7340009, 530, 1, 1, -308.638, 3060.689, -2.040, 0.319, 300, 0),
(7340157, 7340009, 530, 1, 1, 714.288, 6980.082, -72.530, 6.212, 300, 0),
(7340158, 7340009, 546, 1, 1, 11.557, -13.836, -2.250, 4.049, 300, 0),
(7340159, 7340009, 530, 1, 1, 794.062, 6930.772, -79.980, 4.872, 300, 0),
(7340160, 7340009, 530, 1, 1, -3103.991, 4948.514, -101.010, 4.649, 300, 0),
(7340161, 7340009, 530, 1, 1, -3365.038, 5209.732, -100.550, 0.039, 300, 0),
(7340162, 7340009, 530, 1, 1, -3359.204, 4663.963, -100.550, 3.089, 300, 0),
(7340163, 7340009, 530, 1, 1, -3627.845, 4938.981, -100.550, 1.589, 300, 0),
(7340164, 7340009, 1, 1, 1, -8405.013, -4067.706, -208.090, 4.952, 300, 0),
(7340165, 7340009, 1, 1, 1, -8736.794, -4231.777, -209.000, 0.589, 300, 0),
(7340166, 7340009, 530, 1, 1, 2869.003, 1547.084, 252.660, 2.249, 300, 0),
(7340167, 7340009, 530, 1, 1, 3409.004, 1490.807, 183.340, 4.029, 300, 0),
(7340168, 7340009, 530, 1, 1, 3311.836, 1341.426, 506.060, 3.379, 300, 0),
(7340169, 7340009, 530, 1, 1, 12887.588, -7317.428, 66.000, 3.229, 300, 0),
(7340170, 7340009, 0, 1, 1, -11120.716, -2007.942, 47.580, 5.362, 300, 0),
(7340171, 7340009, 530, 1, 1, 3532.155, 5106.227, 4.010, 3.939, 300, 0),
(7340172, 7340009, 530, 1, 1, -310.036, 3088.639, -116.020, 3.619, 300, 0),
(7340173, 7340009, 530, 1, 1, 820.030, 6867.930, -66.260, 4.709, 300, 0),
(7340174, 7340009, 530, 1, 1, 3091.477, 1381.293, 185.360, 3.049, 300, 0),
(7340175, 7340009, 1, 1, 1, -8180.259, -4179.389, -167.050, 5.622, 300, 0),
(7340176, 7340009, 530, 1, 1, -3650.521, 314.531, 35.780, 1.369, 300, 0),
(7340177, 7340009, 530, 1, 1, 6854.774, -7972.757, 179.740, 3.079, 300, 0),
(7340178, 7340009, 530, 1, 1, 12574.095, -6777.810, 15.590, 1.569, 300, 0),
(7340179, 7340009, 571, 1, 1, 1218.805, -4862.423, 41.750, 5.022, 300, 0),
(7340180, 7340009, 571, 1, 1, 3893.520, 6988.330, 69.990, 4.709, 300, 0),
(7340181, 7340009, 571, 1, 1, 3675.293, 2164.701, 36.310, 0.729, 300, 0),
(7340182, 7340009, 571, 1, 1, 3646.104, 2035.418, 2.290, 2.769, 300, 0),
(7340183, 7340009, 571, 1, 1, 4771.601, -2032.978, 229.650, 0.019, 300, 0),
(7340184, 7340009, 571, 1, 1, 5687.828, 491.628, 653.090, 2.459, 300, 0),
(7340185, 7340009, 571, 1, 1, 6950.127, -4417.912, 450.580, 5.522, 300, 0),
(7340186, 7340009, 571, 1, 1, 8918.911, -993.438, 1039.910, 6.262, 300, 0),
(7340187, 7340009, 571, 1, 1, 9184.860, -1382.532, 1110.710, 4.009, 300, 0),
(7340188, 7340009, 571, 1, 1, 3880.135, 6981.625, 106.810, 1.629, 300, 0),
(7340189, 7340009, 571, 1, 1, 1260.326, -4854.850, 216.260, 1.909, 300, 0),
(7340190, 7340009, 1, 1, 1, -8747.924, -4443.179, -198.760, 2.809, 300, 0),
(7340191, 7340009, 571, 1, 1, 8588.715, 788.905, 558.740, 1.669, 300, 0),
(7340192, 7340009, 571, 1, 1, 5668.510, 2011.173, 798.540, 3.859, 300, 0),
(7340193, 7340009, 571, 1, 1, 5600.599, 2013.496, 798.540, 2.239, 300, 0),
(7340194, 7340009, 571, 1, 1, 5633.418, 1993.644, 798.560, 3.019, 300, 0),
(7340195, 7340009, 571, 1, 1, 3671.711, -1262.227, 244.120, 3.219, 300, 0),
(7340196, 7340009, 571, 1, 1, 3457.524, 259.419, -113.320, 1.709, 300, 0),
(7340197, 7340009, 571, 1, 1, 3860.834, 6992.507, 152.540, 4.229, 300, 0),
(7340198, 7340009, 571, 1, 1, 5453.720, 2843.790, 421.780, 4.712, 300, 0),
(7340199, 7340009, 571, 1, 1, 9327.250, -1111.640, 1245.650, 4.712, 300, 0),
(7340200, 7340009, 571, 1, 1, 8512.680, 716.982, 558.750, 6.282, 300, 0),
(7340201, 7340009, 571, 1, 1, 5875.011, 2108.227, 636.510, 1.979, 300, 0),
(7340202, 7340009, 571, 1, 1, 3602.997, 199.003, -113.260, 3.729, 300, 0),
(7340203, 7340050, 571, 1, 1, 5822.000, 452.500, 659.500, 4.712, 300, 0),
(7340204, 7340050, 36, 1, 1, -17.541, -379.236, 61.780, 5.002, 300, 0);

-- Free riding training (the skills are also learned automatically on level-up)
UPDATE `trainer_spell` SET `MoneyCost` = 0 WHERE `SpellId` IN (33388, 33391, 34090, 34091, 54197);
