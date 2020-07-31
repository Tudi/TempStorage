-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               5.6.10 - MySQL Community Server (GPL)
-- Server OS:                    Win64
-- HeidiSQL Version:             11.0.0.5919
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping structure for table rum_lm.monstertypes
CREATE TABLE IF NOT EXISTS `monstertypes` (
  `MonsterType` int(11) NOT NULL,
  `MonsterName` varchar(32) COLLATE utf8_unicode_ci DEFAULT NULL,
  `MonsterLevel` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`MonsterType`),
  UNIQUE KEY `MonsterType_2` (`MonsterType`),
  KEY `MonsterType` (`MonsterType`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Dumping data for table rum_lm.monstertypes: 15 rows
/*!40000 ALTER TABLE `monstertypes` DISABLE KEYS */;
REPLACE INTO `monstertypes` (`MonsterType`, `MonsterName`, `MonsterLevel`) VALUES
	(22, 'Mega Maggot lvl 1', 1),
	(23, 'Mega Maggot lvl 2', 2),
	(24, 'Mega Maggot lvl 3', 3),
	(25, 'Mega Maggot lvl 4', 4),
	(26, 'Mega Maggot lvl 5', 5),
	(39, 'Hell drider lvl 1', 1),
	(40, 'Hell drider lvl 2', 2),
	(41, 'Hell drider lvl 3', 3),
	(42, 'Hell drider lvl 4', 4),
	(43, 'Hell drider lvl 5', 5),
	(195, 'Voodoo shaman lvl 1', 1),
	(196, 'Voodoo shaman lvl 2', 2),
	(197, 'Voodoo shaman lvl 3', 3),
	(198, 'Voodoo shaman lvl 4', 4),
	(199, 'Voodoo shaman lvl 5', 5),
	(210, 'Blue Bonus', 0);
/*!40000 ALTER TABLE `monstertypes` ENABLE KEYS */;

-- Dumping structure for table rum_lm.playerhunts
CREATE TABLE IF NOT EXISTS `playerhunts` (
  `RowId` int(11) NOT NULL AUTO_INCREMENT,
  `PlayerId` int(11) DEFAULT NULL,
  `Year` int(11) NOT NULL,
  `Day` int(11) NOT NULL,
  `Lvl1` int(11) NOT NULL DEFAULT '0',
  `Lvl2` int(11) NOT NULL DEFAULT '0',
  `Lvl3` int(11) NOT NULL DEFAULT '0',
  `Lvl4` int(11) NOT NULL DEFAULT '0',
  `Lvl5` int(11) NOT NULL DEFAULT '0',
  `PlayerName` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`RowId`),
  KEY `PlayerNamei` (`PlayerName`)
) ENGINE=InnoDB AUTO_INCREMENT=22 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Dumping data for table rum_lm.playerhunts: ~3 rows (approximately)
/*!40000 ALTER TABLE `playerhunts` DISABLE KEYS */;
REPLACE INTO `playerhunts` (`RowId`, `PlayerId`, `Year`, `Day`, `Lvl1`, `Lvl2`, `Lvl3`, `Lvl4`, `Lvl5`, `PlayerName`) VALUES
	(3, NULL, 2020, 212, 4, 1, 0, 0, 0, 'Tudi seconda'),
	(4, NULL, 2020, 212, 0, 3, 0, 0, 0, 'big king 02'),
	(5, NULL, 2020, 212, 1, 2, 0, 0, 0, 'Mr Legit'),
	(6, NULL, 2020, 212, 0, 1, 0, 0, 0, 'ZixY'),
	(7, NULL, 2020, 212, 10, 0, 1, 0, 0, 'TommyEgan'),
	(8, NULL, 2020, 212, 0, 0, 4, 0, 0, 'LafondleMe23'),
	(9, NULL, 2020, 212, 0, 0, 2, 0, 0, 'DeepWater TR'),
	(10, NULL, 2020, 212, 0, 2, 0, 0, 0, 'Avenger CO'),
	(11, NULL, 2020, 212, 0, 0, 1, 0, 0, 'Drewvy'),
	(12, NULL, 2020, 212, 0, 0, 1, 0, 0, 'Qalanna'),
	(13, NULL, 2020, 212, 0, 2, 0, 0, 0, 'WaveRider'),
	(14, NULL, 2020, 212, 0, 2, 0, 0, 0, 'CrAzI NiNjA'),
	(15, NULL, 2020, 212, 0, 1, 0, 0, 0, 'Tudi69'),
	(16, NULL, 2020, 212, 0, 0, 1, 0, 0, 'lizzardboy'),
	(17, NULL, 2020, 212, 0, 2, 0, 0, 0, 'KRAL LORD1'),
	(18, NULL, 2020, 212, 0, 2, 0, 0, 0, 'Lord gNgstr'),
	(19, NULL, 2020, 212, 4, 0, 0, 0, 0, 'ALGERIANOO'),
	(20, NULL, 2020, 212, 0, 1, 0, 0, 0, 'Byefendi'),
	(21, NULL, 2020, 212, 1, 0, 0, 0, 0, 'xBlueprintx');
/*!40000 ALTER TABLE `playerhunts` ENABLE KEYS */;

-- Dumping structure for table rum_lm.playernames
CREATE TABLE IF NOT EXISTS `playernames` (
  `RowId` int(11) NOT NULL,
  `HashedName` int(11) NOT NULL,
  `PlayerName` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `IsActive` smallint(6) NOT NULL DEFAULT '1',
  `HuntsMissed` int(11) NOT NULL DEFAULT '0',
  UNIQUE KEY `RowId` (`RowId`),
  KEY `HashedName` (`HashedName`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Dumping data for table rum_lm.playernames: 0 rows
/*!40000 ALTER TABLE `playernames` DISABLE KEYS */;
/*!40000 ALTER TABLE `playernames` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
