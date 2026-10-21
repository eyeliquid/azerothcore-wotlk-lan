-- mod-lan: NPC templates, trainers and spawns. Entries 9100001-9100199, spawn guids 9100001-9100199, trainers 9100-9113.

DELETE FROM `creature_template` WHERE `entry` BETWEEN 9100001 AND 9100199;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `unit_class`, `unit_flags`, `type`, `flags_extra`, `ScriptName`) VALUES
(9100001, 'Gearwright Holloway', 'Tier 6 Outfitter', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_outfitter'),
(9100002, 'Pack Merchant Dunstan', 'Biggest Bags', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_bag_vendor'),
(9100003, 'Archmage Telvaris', 'LAN Teleporter', 80, 80, 2, 35, 1, 1, 768, 7, 2, 'npc_lan_teleporter'),
(9100004, 'Commander Valeria', 'Bot Commander', 80, 80, 2, 35, 1, 1, 768, 7, 2, 'npc_lan_commander_valeria'),
(9100010, 'Alchemy Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100030, 'Alchemy Grand Master', 'Alchemy Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100011, 'Blacksmithing Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100031, 'Blacksmithing Grand Master', 'Blacksmithing Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100012, 'Enchanting Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100032, 'Enchanting Grand Master', 'Enchanting Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100013, 'Engineering Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100033, 'Engineering Grand Master', 'Engineering Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100014, 'Herbalism Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100034, 'Herbalism Grand Master', 'Herbalism Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100015, 'Inscription Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100035, 'Inscription Grand Master', 'Inscription Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100016, 'Jewelcrafting Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100036, 'Jewelcrafting Grand Master', 'Jewelcrafting Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100017, 'Leatherworking Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100037, 'Leatherworking Grand Master', 'Leatherworking Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100018, 'Mining Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100038, 'Mining Grand Master', 'Mining Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100019, 'Skinning Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100039, 'Skinning Grand Master', 'Skinning Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100020, 'Tailoring Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100040, 'Tailoring Grand Master', 'Tailoring Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100021, 'Cooking Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100041, 'Cooking Grand Master', 'Cooking Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100022, 'First Aid Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100042, 'First Aid Grand Master', 'First Aid Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100023, 'Fishing Supplier', 'Free Recipes, Reagents & Tools', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_profession_vendor'),
(9100043, 'Fishing Grand Master', 'Fishing Trainer', 80, 80, 2, 35, 80, 1, 768, 7, 2, ''),
(9100100, 'Balance Druid Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100101, 'Feral (Cat) Druid Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100102, 'Feral (Bear) Druid Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100103, 'Restoration Druid Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100104, 'Blood (DPS) Death Knight Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100105, 'Blood (Tank) Death Knight Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100106, 'Frost Death Knight Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100107, 'Unholy Death Knight Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100108, 'Beast Mastery Hunter Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100109, 'Marksmanship Hunter Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100110, 'Survival Hunter Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100111, 'Arcane Mage Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100112, 'Fire Mage Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100113, 'Frostfire Mage Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100114, 'Frost Mage Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100115, 'Holy Paladin Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100116, 'Protection Paladin Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100117, 'Retribution Paladin Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100118, 'Discipline Priest Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100119, 'Holy Priest Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100120, 'Shadow Priest Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100121, 'Assassination Rogue Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100122, 'Combat Rogue Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100123, 'Subtlety Rogue Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100124, 'Elemental Shaman Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100125, 'Enhancement Shaman Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100126, 'Restoration Shaman Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100127, 'Affliction Warlock Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100128, 'Demonology Warlock Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100129, 'Destruction Warlock Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100130, 'Arms Warrior Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100131, 'Fury Warrior Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor'),
(9100132, 'Protection Warrior Quartermaster', 'Tier 9 Best-in-Slot', 80, 80, 2, 35, 129, 1, 768, 7, 2, 'npc_lan_spec_vendor');

DELETE FROM `creature_template_model` WHERE `CreatureID` BETWEEN 9100001 AND 9100199;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(9100001, 0, 25611, 1, 1),
(9100002, 0, 10212, 1, 1),
(9100003, 0, 25166, 1, 1),
(9100004, 0, 24930, 1, 1),
(9100010, 0, 28151, 1, 1),
(9100030, 0, 25947, 1, 1),
(9100011, 0, 28151, 1, 1),
(9100031, 0, 25947, 1, 1),
(9100012, 0, 28151, 1, 1),
(9100032, 0, 25947, 1, 1),
(9100013, 0, 28151, 1, 1),
(9100033, 0, 25947, 1, 1),
(9100014, 0, 28151, 1, 1),
(9100034, 0, 25947, 1, 1),
(9100015, 0, 28151, 1, 1),
(9100035, 0, 25947, 1, 1),
(9100016, 0, 28151, 1, 1),
(9100036, 0, 25947, 1, 1),
(9100017, 0, 28151, 1, 1),
(9100037, 0, 25947, 1, 1),
(9100018, 0, 28151, 1, 1),
(9100038, 0, 25947, 1, 1),
(9100019, 0, 28151, 1, 1),
(9100039, 0, 25947, 1, 1),
(9100020, 0, 28151, 1, 1),
(9100040, 0, 25947, 1, 1),
(9100021, 0, 28151, 1, 1),
(9100041, 0, 25947, 1, 1),
(9100022, 0, 28151, 1, 1),
(9100042, 0, 25947, 1, 1),
(9100023, 0, 28151, 1, 1),
(9100043, 0, 25947, 1, 1),
(9100100, 0, 27587, 1, 1),
(9100101, 0, 27587, 1, 1),
(9100102, 0, 27587, 1, 1),
(9100103, 0, 27587, 1, 1),
(9100104, 0, 27587, 1, 1),
(9100105, 0, 27587, 1, 1),
(9100106, 0, 27587, 1, 1),
(9100107, 0, 27587, 1, 1),
(9100108, 0, 27587, 1, 1),
(9100109, 0, 27587, 1, 1),
(9100110, 0, 27587, 1, 1),
(9100111, 0, 27587, 1, 1),
(9100112, 0, 27587, 1, 1),
(9100113, 0, 27587, 1, 1),
(9100114, 0, 27587, 1, 1),
(9100115, 0, 27587, 1, 1),
(9100116, 0, 27587, 1, 1),
(9100117, 0, 27587, 1, 1),
(9100118, 0, 27587, 1, 1),
(9100119, 0, 27587, 1, 1),
(9100120, 0, 27587, 1, 1),
(9100121, 0, 27587, 1, 1),
(9100122, 0, 27587, 1, 1),
(9100123, 0, 27587, 1, 1),
(9100124, 0, 27587, 1, 1),
(9100125, 0, 27587, 1, 1),
(9100126, 0, 27587, 1, 1),
(9100127, 0, 27587, 1, 1),
(9100128, 0, 27587, 1, 1),
(9100129, 0, 27587, 1, 1),
(9100130, 0, 27587, 1, 1),
(9100131, 0, 27587, 1, 1),
(9100132, 0, 27587, 1, 1);

-- Profession trainers teaching every rank (Apprentice to Grand Master) and recipe of the existing trainers
DELETE FROM `trainer` WHERE `Id` BETWEEN 9100 AND 9113;
INSERT INTO `trainer` (`Id`, `Type`, `Requirement`, `Greeting`) VALUES
(9100, 2, 0, 'I can teach you everything about alchemy, from Apprentice to Grand Master.'),
(9101, 2, 0, 'I can teach you everything about blacksmithing, from Apprentice to Grand Master.'),
(9102, 2, 0, 'I can teach you everything about enchanting, from Apprentice to Grand Master.'),
(9103, 2, 0, 'I can teach you everything about engineering, from Apprentice to Grand Master.'),
(9104, 2, 0, 'I can teach you everything about herbalism, from Apprentice to Grand Master.'),
(9105, 2, 0, 'I can teach you everything about inscription, from Apprentice to Grand Master.'),
(9106, 2, 0, 'I can teach you everything about jewelcrafting, from Apprentice to Grand Master.'),
(9107, 2, 0, 'I can teach you everything about leatherworking, from Apprentice to Grand Master.'),
(9108, 2, 0, 'I can teach you everything about mining, from Apprentice to Grand Master.'),
(9109, 2, 0, 'I can teach you everything about skinning, from Apprentice to Grand Master.'),
(9110, 2, 0, 'I can teach you everything about tailoring, from Apprentice to Grand Master.'),
(9111, 2, 0, 'I can teach you everything about cooking, from Apprentice to Grand Master.'),
(9112, 2, 0, 'I can teach you everything about first aid, from Apprentice to Grand Master.'),
(9113, 2, 0, 'I can teach you everything about fishing, from Apprentice to Grand Master.');
DELETE FROM `trainer_spell` WHERE `TrainerId` BETWEEN 9100 AND 9113;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9100, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 171) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9101, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 164) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9102, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 333) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9103, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 202) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9104, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 182) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9105, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 773) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9106, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 755) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9107, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 165) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9108, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 186) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9109, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 393) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9110, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 197) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9111, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 185) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9112, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 129) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`)
SELECT 9113, `ts`.`SpellId`, MAX(`ts`.`MoneyCost`), MAX(`ts`.`ReqSkillLine`), MAX(`ts`.`ReqSkillRank`), MAX(`ts`.`ReqAbility1`), MAX(`ts`.`ReqAbility2`), MAX(`ts`.`ReqAbility3`), MIN(`ts`.`ReqLevel`)
FROM `trainer_spell` `ts` JOIN (SELECT `TrainerId` FROM `trainer_spell` WHERE `TrainerId` < 9100 GROUP BY `TrainerId` HAVING MAX(`ReqSkillLine`) = 356) `t` ON `t`.`TrainerId` = `ts`.`TrainerId`
GROUP BY `ts`.`SpellId`;
DELETE FROM `creature_default_trainer` WHERE `CreatureId` BETWEEN 9100030 AND 9100043;
INSERT INTO `creature_default_trainer` (`CreatureId`, `TrainerId`) VALUES
(9100030, 9100),
(9100031, 9101),
(9100032, 9102),
(9100033, 9103),
(9100034, 9104),
(9100035, 9105),
(9100036, 9106),
(9100037, 9107),
(9100038, 9108),
(9100039, 9109),
(9100040, 9110),
(9100041, 9111),
(9100042, 9112),
(9100043, 9113);

DELETE FROM `creature` WHERE `id` BETWEEN 9100001 AND 9100199 AND `guid` BETWEEN 9100001 AND 9100199;
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`) VALUES
(9100001, 9100001, 0, 1, 1, -8944.950, -129.990, 84.530, 3.605, 300),
(9100002, 9100002, 0, 1, 1, -8944.950, -134.990, 84.530, 2.678, 300),
(9100003, 9100001, 0, 1, 1, -6235.089, 333.002, 383.760, 3.502, 300),
(9100004, 9100002, 0, 1, 1, -6235.604, 328.028, 383.760, 2.575, 300),
(9100005, 9100001, 0, 1, 1, 1671.123, 1678.131, 122.670, 0.032, 300),
(9100006, 9100002, 0, 1, 1, 1673.214, 1682.672, 122.670, 5.388, 300),
(9100007, 9100001, 1, 1, 1, -2912.580, -255.480, 54.000, 3.605, 300),
(9100008, 9100002, 1, 1, 1, -2912.580, -260.480, 54.000, 2.678, 300),
(9100009, 9100001, 1, 1, 1, -613.520, -4249.170, 39.720, 3.605, 300),
(9100010, 9100002, 1, 1, 1, -613.520, -4254.170, 39.720, 2.678, 300),
(9100011, 9100001, 1, 1, 1, 10316.850, 831.793, 1327.410, 3.022, 300),
(9100012, 9100002, 1, 1, 1, 10314.097, 827.620, 1327.410, 2.095, 300),
(9100013, 9100001, 530, 1, 1, -3966.260, -13928.053, 101.610, 5.685, 300),
(9100014, 9100002, 530, 1, 1, -3961.895, -13925.616, 101.610, 4.758, 300),
(9100015, 9100001, 530, 1, 1, 10354.507, -6359.968, 34.400, 2.642, 300),
(9100016, 9100002, 530, 1, 1, 10350.402, -6362.822, 34.400, 1.715, 300),
(9100017, 9100001, 609, 1, 4294967295, 2351.946, -5666.349, 427.030, 0.982, 300),
(9100018, 9100002, 609, 1, 4294967295, 2349.468, -5662.006, 427.030, 0.055, 300),
(9100019, 9100003, 0, 1, 1, -8831.940, 631.262, 95.010, 4.212, 300),
(9100020, 9100003, 0, 1, 1, -4916.930, -942.690, 502.560, 2.278, 300),
(9100021, 9100003, 1, 1, 1, 9949.472, 2287.209, 1342.400, 4.742, 300),
(9100022, 9100003, 530, 1, 1, -3963.720, -11651.346, -137.840, 3.992, 300),
(9100023, 9100003, 1, 1, 1, 1627.306, -4375.230, 32.560, 0.558, 300),
(9100024, 9100003, 0, 1, 1, 1587.138, 240.430, -51.150, 3.182, 300),
(9100025, 9100003, 1, 1, 1, -1275.912, 122.178, 132.290, 2.078, 300),
(9100026, 9100003, 530, 1, 1, 9490.667, -7279.569, 15.290, 3.018, 300),
(9100027, 9100003, 571, 1, 1, 5826.000, 449.000, 659.500, 4.481, 300),
(9100028, 9100004, 571, 1, 1, 5818.500, 448.500, 659.500, 4.921, 300),
(9100029, 9100100, 571, 1, 1, 5829.000, 432.000, 659.000, 3.142, 300),
(9100030, 9100101, 571, 1, 1, 5828.198, 435.253, 659.000, 3.625, 300),
(9100031, 9100102, 571, 1, 1, 5825.976, 437.761, 659.000, 4.108, 300),
(9100032, 9100103, 571, 1, 1, 5822.844, 438.949, 659.000, 4.592, 300),
(9100033, 9100104, 571, 1, 1, 5819.518, 438.545, 659.000, 5.075, 300),
(9100034, 9100105, 571, 1, 1, 5816.760, 436.642, 659.000, 5.558, 300),
(9100035, 9100106, 571, 1, 1, 5815.203, 433.675, 659.000, 6.042, 300),
(9100036, 9100107, 571, 1, 1, 5815.203, 430.325, 659.000, 0.242, 300),
(9100037, 9100108, 571, 1, 1, 5816.760, 427.358, 659.000, 0.725, 300),
(9100038, 9100109, 571, 1, 1, 5819.518, 425.455, 659.000, 1.208, 300),
(9100039, 9100110, 571, 1, 1, 5822.844, 425.051, 659.000, 1.692, 300),
(9100040, 9100111, 571, 1, 1, 5825.976, 426.239, 659.000, 2.175, 300),
(9100041, 9100112, 571, 1, 1, 5828.198, 428.747, 659.000, 2.658, 300),
(9100042, 9100113, 571, 1, 1, 5833.852, 433.877, 659.000, 3.299, 300),
(9100043, 9100114, 571, 1, 1, 5832.692, 437.448, 659.000, 3.613, 300),
(9100044, 9100115, 571, 1, 1, 5830.485, 440.485, 659.000, 3.927, 300),
(9100045, 9100116, 571, 1, 1, 5827.448, 442.692, 659.000, 4.241, 300),
(9100046, 9100117, 571, 1, 1, 5823.877, 443.852, 659.000, 4.555, 300),
(9100047, 9100118, 571, 1, 1, 5820.123, 443.852, 659.000, 4.869, 300),
(9100048, 9100119, 571, 1, 1, 5816.552, 442.692, 659.000, 5.184, 300),
(9100049, 9100120, 571, 1, 1, 5813.515, 440.485, 659.000, 5.498, 300),
(9100050, 9100121, 571, 1, 1, 5811.308, 437.448, 659.000, 5.812, 300),
(9100051, 9100122, 571, 1, 1, 5810.148, 433.877, 659.000, 6.126, 300),
(9100052, 9100123, 571, 1, 1, 5810.148, 430.123, 659.000, 0.157, 300),
(9100053, 9100124, 571, 1, 1, 5811.308, 426.552, 659.000, 0.471, 300),
(9100054, 9100125, 571, 1, 1, 5813.515, 423.515, 659.000, 0.785, 300),
(9100055, 9100126, 571, 1, 1, 5816.552, 421.308, 659.000, 1.100, 300),
(9100056, 9100127, 571, 1, 1, 5820.123, 420.148, 659.000, 1.414, 300),
(9100057, 9100128, 571, 1, 1, 5823.877, 420.148, 659.000, 1.728, 300),
(9100058, 9100129, 571, 1, 1, 5827.448, 421.308, 659.000, 2.042, 300),
(9100059, 9100130, 571, 1, 1, 5830.485, 423.515, 659.000, 2.356, 300),
(9100060, 9100131, 571, 1, 1, 5832.692, 426.552, 659.000, 2.670, 300),
(9100061, 9100132, 571, 1, 1, 5833.852, 430.123, 659.000, 2.985, 300),
(9100062, 9100030, 571, 1, 1, 5957.400, 671.000, 643.000, 3.142, 300),
(9100063, 9100010, 571, 1, 1, 5961.124, 673.448, 643.000, 3.366, 300),
(9100064, 9100031, 571, 1, 1, 5956.707, 674.037, 643.000, 3.590, 300),
(9100065, 9100011, 571, 1, 1, 5959.000, 677.858, 643.000, 3.815, 300),
(9100066, 9100032, 571, 1, 1, 5954.764, 676.473, 643.000, 4.039, 300),
(9100067, 9100012, 571, 1, 1, 5955.173, 680.911, 643.000, 4.264, 300),
(9100068, 9100033, 571, 1, 1, 5951.958, 677.824, 643.000, 4.488, 300),
(9100069, 9100013, 571, 1, 1, 5950.400, 682.000, 643.000, 4.712, 300),
(9100070, 9100034, 571, 1, 1, 5948.842, 677.824, 643.000, 4.937, 300),
(9100071, 9100014, 571, 1, 1, 5945.627, 680.911, 643.000, 5.161, 300),
(9100072, 9100035, 571, 1, 1, 5946.036, 676.473, 643.000, 5.386, 300),
(9100073, 9100015, 571, 1, 1, 5941.800, 677.858, 643.000, 5.610, 300),
(9100074, 9100036, 571, 1, 1, 5944.093, 674.037, 643.000, 5.834, 300),
(9100075, 9100016, 571, 1, 1, 5939.676, 673.448, 643.000, 6.059, 300),
(9100076, 9100037, 571, 1, 1, 5943.400, 671.000, 643.000, 0.000, 300),
(9100077, 9100017, 571, 1, 1, 5939.676, 668.552, 643.000, 0.224, 300),
(9100078, 9100038, 571, 1, 1, 5944.093, 667.963, 643.000, 0.449, 300),
(9100079, 9100018, 571, 1, 1, 5941.800, 664.142, 643.000, 0.673, 300),
(9100080, 9100039, 571, 1, 1, 5946.036, 665.527, 643.000, 0.898, 300),
(9100081, 9100019, 571, 1, 1, 5945.627, 661.089, 643.000, 1.122, 300),
(9100082, 9100040, 571, 1, 1, 5948.842, 664.176, 643.000, 1.346, 300),
(9100083, 9100020, 571, 1, 1, 5950.400, 660.000, 643.000, 1.571, 300),
(9100084, 9100041, 571, 1, 1, 5951.958, 664.176, 643.000, 1.795, 300),
(9100085, 9100021, 571, 1, 1, 5955.173, 661.089, 643.000, 2.020, 300),
(9100086, 9100042, 571, 1, 1, 5954.764, 665.527, 643.000, 2.244, 300),
(9100087, 9100022, 571, 1, 1, 5959.000, 664.142, 643.000, 2.468, 300),
(9100088, 9100043, 571, 1, 1, 5956.707, 667.963, 643.000, 2.693, 300),
(9100089, 9100023, 571, 1, 1, 5961.124, 668.552, 643.000, 2.917, 300);

-- Free riding training (the skills are also learned automatically on level-up)
UPDATE `trainer_spell` SET `MoneyCost` = 0 WHERE `SpellId` IN (33388, 33391, 34090, 34091, 54197);
