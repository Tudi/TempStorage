-- phpMyAdmin SQL Dump
-- version 4.0.10.7
-- http://www.phpmyadmin.net
--
-- Host: localhost:3306
-- Generation Time: Feb 23, 2016 at 03:46 PM
-- Server version: 5.1.73-14.12
-- PHP Version: 5.4.31

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `reur0702_ib`
--

-- --------------------------------------------------------

--
-- Table structure for table `comments`
--

CREATE TABLE IF NOT EXISTS `comments` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `UserId` int(10) unsigned DEFAULT NULL,
  `IdeaId` int(10) unsigned NOT NULL,
  `IdeaComment` varchar(2000) COLLATE utf8_unicode_ci DEFAULT NULL,
  `FingerPrint` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `CreateStamp` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `id_2` (`id`),
  KEY `id_3` (`id`),
  KEY `IdeaId` (`IdeaId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=44 ;

--
-- Dumping data for table `comments`
--

INSERT INTO `comments` (`id`, `UserId`, `IdeaId`, `IdeaComment`, `FingerPrint`, `CreateStamp`) VALUES
(4, 1, 5, 'c1', '', 1455542465),
(5, 0, 5, 'c2 2', '', 1455542470),
(6, 1, 9, 'na c1', '', 1455542817),
(7, 1, 9, 'na c2', '', 1455542830),
(8, 0, 19, 'test c', '', 1456128120),
(9, 0, 19, 'test 2', '', 1456128195),
(10, 5, 5, 'alextest 2', '', 1456156101),
(11, 5, 5, 'test 2', '', 1456156587),
(12, 1, 7, 'cgdhd', '', 1456156658),
(13, 1, 26, 'asdfg asdf', '', 1456159791),
(14, 1, 7, 'shdjf', '', 1456210977),
(15, 1, 7, 'rwtq', '', 1456211006),
(16, 1, 5, 'todel', '', 1456211731),
(17, 1, 31, 'nice comment', '', 1456212124),
(18, 1, 31, 'dSGSDG', '', 1456212180),
(19, 1, 32, 'da-mi un comment sa vad cum merge', '', 1456213132),
(20, 1, 7, 'pagina noua de comment', '86a29267d07e61e5bb47e4aba144291a', 1456213286),
(21, 1, 10, 'si cum merge?', '', 1456213340),
(25, 1, 5, 'todel 4', '', 1456215452),
(26, 1, 45, 'SGHSGH', '', 1456215524),
(27, 1, 45, 'faGAZFR', '', 1456215535),
(28, 1, 45, 'FGSZDG', '', 1456215552),
(29, 1, 20, 'test', '', 1456215711),
(32, 1, 32, 'stas!! good job', '86a29267d07e61e5bb47e4aba144291a', 1456216266),
(33, 1, 21, 'test', '', 1456223438),
(36, 1, 28, 'hdku', '3c09a8729fb9d477895db2843e74c337', 1456228334),
(37, 1, 47, 'hsdyhj', '', 1456228418),
(39, 0, 45, 'ogbu9p', '', 1456231759),
(40, 1, 50, 'not well enough', '', 1456231765),
(41, 1, 10, 'habar nu am', '', 1456232237),
(42, 0, 26, 'jyytj', '', 1456232915),
(43, 0, 51, 'adfg', '', 1456233771);

-- --------------------------------------------------------

--
-- Table structure for table `ideas`
--

CREATE TABLE IF NOT EXISTS `ideas` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `UserId` int(10) unsigned DEFAULT NULL,
  `Title` varchar(2000) COLLATE utf8_unicode_ci DEFAULT NULL,
  `Idea` varchar(2000) COLLATE utf8_unicode_ci DEFAULT NULL,
  `FingerPrint` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `CreateStamp` int(10) unsigned NOT NULL,
  `SiteId` int(10) unsigned DEFAULT NULL,
  `SponsorId` int(10) unsigned DEFAULT NULL,
  `StatusChangeStamp` int(10) unsigned NOT NULL,
  `StatusTypeId` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Id`),
  UNIQUE KEY `Id` (`Id`),
  KEY `Id_2` (`Id`),
  KEY `Id_3` (`Id`),
  KEY `UserId` (`UserId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=68 ;

--
-- Dumping data for table `ideas`
--

INSERT INTO `ideas` (`Id`, `UserId`, `Title`, `Idea`, `FingerPrint`, `CreateStamp`, `SiteId`, `SponsorId`, `StatusChangeStamp`, `StatusTypeId`) VALUES
(32, 1, 'titlu e prea mic si totusi mare', 'n-are sens ce scriu', '86a29267d07e61e5bb47e4aba144291a', 1456212954, 1, NULL, 0, 0),
(33, 0, 'ma aflu in milford acum', 'da da', '', 1456215119, 0, NULL, 0, 0),
(43, 1, 'ALTA IDEE PENTRU MILFORD', 'DA', '86a29267d07e61e5bb47e4aba144291a', 1456215194, 2, NULL, 0, 0),
(45, 1, 'M-AM INTORS LA BRASOV', 'NO KIDDING!', '86a29267d07e61e5bb47e4aba144291a', 1456215447, 1, NULL, 0, 0),
(46, 1, 'idee noua de frechen', 'frechen', '86a29267d07e61e5bb47e4aba144291a', 1456218464, 3, NULL, 0, 0),
(53, 1, 'Add a tablet or IPad to each meeting room displaying booking statusand allow booking status right form the meeting room. \\"Book now for x hours\\"', 'Add a tablet or IPad to each meeting room displaying booking status and allow booking status right form the meeting room. \\"Book now for x hours\\"', '3c09a8729fb9d477895db2843e74c337', 1456234778, 2, NULL, 0, 0),
(54, 1, 'Use evaluation tests for basis of customers facing IQ/OQ sell as Auto Validation tools', 'Use evaluation tests for basis of customers facing IQ/OQ sell as Auto Validation tools', '3c09a8729fb9d477895db2843e74c337', 1456234802, 2, NULL, 0, 0),
(55, 1, 'Use Paradigm to search for Empower help.', 'Use Paradigm to search for Empower help.', '3c09a8729fb9d477895db2843e74c337', 1456234819, 2, NULL, 0, 0),
(56, 1, 'Expand vCloud to fields support', 'Expand vCloud to fields support', '3c09a8729fb9d477895db2843e74c337', 1456234891, 2, NULL, 0, 0),
(57, 1, 'Create a guild for product management', 'Create a guild for product management', '3c09a8729fb9d477895db2843e74c337', 1456234914, 2, NULL, 0, 0),
(58, 1, 'Time to volunteer outside of Waters as a team.', 'Time to volunteer outside of Waters as a team.', '3c09a8729fb9d477895db2843e74c337', 1456234935, 2, NULL, 0, 0),
(59, 1, 'Provide a list of guilds/ contacts', 'Provide a list of guilds/ contacts', '3c09a8729fb9d477895db2843e74c337', 1456234973, 2, NULL, 0, 0),
(60, 1, 'Team Building', 'Team Building', '3c09a8729fb9d477895db2843e74c337', 1456235017, 1, NULL, 0, 0),
(61, 1, 'Sweets Machine', 'Sweets Machine', '3c09a8729fb9d477895db2843e74c337', 1456235044, 1, NULL, 0, 0),
(62, 1, 'Reduce taxes from 16% to 0% according to the recent change of the Romanian law', 'Reduce taxes from 16% to 0% according to the recent change of the Romanian law', '3c09a8729fb9d477895db2843e74c337', 1456235094, 1, NULL, 0, 0),
(63, 1, 'Install a video conference system in T1 conference room', 'Install a video conference system in T1 conference room', '3c09a8729fb9d477895db2843e74c337', 1456235111, 1, NULL, 0, 0),
(64, 1, '30 minute launch break -> work until 16:30.', '30 minute launch break -> work until 16:30.', '3c09a8729fb9d477895db2843e74c337', 1456235120, 1, NULL, 0, 0),
(65, 1, 'The ability to choose for a part-time program – 6 hours', 'The ability to choose for a part-time program – 6 hours', '3c09a8729fb9d477895db2843e74c337', 1456235142, 1, NULL, 0, 0),
(66, 1, 'Different fruits available in the kitchen', 'Different fruits available in the kitchen', '3c09a8729fb9d477895db2843e74c337', 1456235153, 1, NULL, 0, 0),
(67, 1, 'Parking place for bikes', 'Parking place for bikes', '3c09a8729fb9d477895db2843e74c337', 1456235166, 1, NULL, 0, 0);

-- --------------------------------------------------------

--
-- Table structure for table `sessions`
--

CREATE TABLE IF NOT EXISTS `sessions` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Token` varchar(200) COLLATE utf8_unicode_ci NOT NULL,
  `UserId` int(10) unsigned NOT NULL,
  `LastSwipedIdeaId` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `id_2` (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=6 ;

--
-- Dumping data for table `sessions`
--

INSERT INTO `sessions` (`id`, `Token`, `UserId`, `LastSwipedIdeaId`) VALUES
(2, 'c9964e0292a004073fd145378792042a9665d79cd09a1df9', 5, 0),
(3, 'c655391ee12dc373ca00ac4bbb15bc993b66ca0d990b71a9', 1, 0),
(4, 'c655391ee12dc373ca00ac4bbb15bc993b66ca0d990b71a9', 1, 0),
(5, '8376108d9f7f906eb29bd17196d49753d62b0b3cabab8133', 4, 0);

-- --------------------------------------------------------

--
-- Table structure for table `sites`
--

CREATE TABLE IF NOT EXISTS `sites` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Name` varchar(200) COLLATE utf8_unicode_ci NOT NULL,
  `CountryCode` varchar(4) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `id_2` (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=5 ;

--
-- Dumping data for table `sites`
--

INSERT INTO `sites` (`id`, `Name`, `CountryCode`) VALUES
(1, 'Brasov', 'RO'),
(2, 'Milford', 'US'),
(3, 'Frechen', 'DE'),
(4, 'Wimslow', 'GB');

-- --------------------------------------------------------

--
-- Table structure for table `statustype`
--

CREATE TABLE IF NOT EXISTS `statustype` (
  `id` int(10) NOT NULL,
  `StatusName` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `subscriptions`
--

CREATE TABLE IF NOT EXISTS `subscriptions` (
  `id` int(10) NOT NULL,
  `UserId` int(10) NOT NULL,
  `IdeaId` int(10) NOT NULL,
  `LastCommentId` int(10) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `id` (`id`),
  KEY `id_2` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Name` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `Password` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `Email` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `Confirmed` int(1) unsigned NOT NULL DEFAULT '0',
  `Rights` int(10) unsigned NOT NULL DEFAULT '0',
  `ConfirmationToken` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`Id`),
  UNIQUE KEY `Id` (`Id`),
  KEY `Id_2` (`Id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=8 ;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`Id`, `Name`, `Password`, `Email`, `Confirmed`, `Rights`, `ConfirmationToken`) VALUES
(1, 'a', 'a', 'jozsab1@gmail.com', 1, 0, 'b8e2e05b220c90829aad1b0c891b46e6e0b68f7225e24e2c'),
(3, 'Raluca', 'pass', 'raluca_popovici@waters.com', 0, 0, NULL),
(4, 'b', 'a', '', 0, 0, NULL),
(5, 'alex', 'alex', 'alex@alex.com', 0, 0, NULL),
(6, 'Corina', 'a', 'C@w.com', 0, 0, NULL),
(7, 'Ruxi', 'a', 'R@R', 0, 0, NULL);

-- --------------------------------------------------------

--
-- Table structure for table `uservotes`
--

CREATE TABLE IF NOT EXISTS `uservotes` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `UserId` int(10) unsigned NOT NULL,
  `IdeaId` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`),
  KEY `id_2` (`id`),
  KEY `id_3` (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=77 ;

--
-- Dumping data for table `uservotes`
--

INSERT INTO `uservotes` (`id`, `UserId`, `IdeaId`) VALUES
(29, 5, 10),
(53, 1, 19),
(54, 1, 20),
(56, 1, 26),
(58, 1, 29),
(72, 0, 45),
(73, 1, 28),
(76, 1, 10);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
