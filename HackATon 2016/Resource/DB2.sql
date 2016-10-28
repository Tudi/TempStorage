-- Adminer 3.5.1 MySQL dump

SET NAMES utf8;
SET foreign_key_checks = 0;
SET time_zone = 'SYSTEM';
SET sql_mode = 'NO_AUTO_VALUE_ON_ZERO';

CREATE TABLE `comments` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `UserId` int(10) unsigned DEFAULT NULL,
  `IdeaId` int(10) unsigned NOT NULL,
  `IdeaComment` varchar(2000) COLLATE utf8_unicode_ci DEFAULT NULL,
  `FingerPrint` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `CreateStamp` int(10) unsigned DEFAULT NULL,
  `EditToken` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `comments` (`id`, `UserId`, `IdeaId`, `IdeaComment`, `FingerPrint`, `CreateStamp`, `EditToken`) VALUES
(1,	0,	8,	'a c1\r\ne1',	'',	1455194173,	'7ea797d812a355f7254c2dfdd5fcebcd31a67fd00a4d9502'),
(2,	1,	8,	'na c2',	'',	1455194190,	'89e074022c37cb8cb2a820ce9b1b9b752e5e00df41a05499'),
(3,	0,	8,	'a c3',	'',	1455194240,	'5aafc4df16359ffe92a90cf00f094bdb6a9884f22988398b');

CREATE TABLE `ideas` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `UserId` int(10) unsigned DEFAULT NULL,
  `Idea` varchar(2000) COLLATE utf8_unicode_ci DEFAULT NULL,
  `AllowVote` int(1) unsigned NOT NULL DEFAULT '1',
  `RegisteredVotes` int(1) unsigned NOT NULL DEFAULT '1',
  `AllowComments` int(1) unsigned NOT NULL DEFAULT '1',
  `FingerPrint` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `CreateStamp` int(10) unsigned DEFAULT NULL,
  `EditToken` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `UpVoteCount` int(10) unsigned NOT NULL DEFAULT '0',
  `DownVoteCount` int(10) unsigned NOT NULL DEFAULT '0',
  `Status` int(10) unsigned NOT NULL DEFAULT '0',
  `HandledByUser` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `ideas` (`Id`, `UserId`, `Idea`, `AllowVote`, `RegisteredVotes`, `AllowComments`, `FingerPrint`, `CreateStamp`, `EditToken`, `UpVoteCount`, `DownVoteCount`, `Status`, `HandledByUser`) VALUES
(8,	1,	'na 1 2 3\r\ne1 e2 e3 e4 e5',	1,	1,	1,	'db953af4919dd881485d54d13a067315',	1455109684,	'8c836a01dd39f0d77477b3580db14350b997b57031e9606a',	0,	2,	0,	NULL),
(7,	0,	'a 3\r\ne1 e2 e3 e4 e5 e6',	1,	0,	1,	'db953af4919dd881485d54d13a067315',	1455109676,	'df1a9a7bcab357171634977b684a7c5bb18c32723df59577',	1,	1,	0,	NULL),
(6,	0,	'a 2',	1,	1,	0,	'db953af4919dd881485d54d13a067315',	1455109671,	'28fec20abac7f30aeb75988497fb8f03b0942eb244e32a74',	1,	0,	0,	NULL),
(5,	0,	'a 1',	1,	0,	0,	'db953af4919dd881485d54d13a067315',	1455109665,	'23dd30980cb886e487458b94e54cad9f115076f9e1b99d8e',	0,	1,	0,	NULL),
(9,	1,	'na 1 3',	1,	1,	0,	'db953af4919dd881485d54d13a067315',	1455109693,	'df274c48db2d9d8f5fbdd055a50a35149d9679ceb526a9e1',	1,	0,	0,	NULL),
(10,	0,	'Alex testeaza o idee',	1,	1,	1,	'99903883ae7bebf73be9b598191d665b',	1455116596,	'131ab4835714fcd1f0fe0081daab038f4cd3635f975c0da8',	0,	0,	0,	NULL);

CREATE TABLE `sessions` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Token` varchar(200) COLLATE utf8_unicode_ci NOT NULL,
  `UserId` int(10) unsigned NOT NULL,
  `LastSwipedIdeaId` int(10) unsigned NOT NULL,
  `LastIdeaCommentId` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `sessions` (`id`, `Token`, `UserId`, `LastSwipedIdeaId`, `LastIdeaCommentId`) VALUES
(2,	'e800e5374c56e99ca5c3113d6aa13daa3f29a93e2bf962aa',	1,	0,	0);

CREATE TABLE `userfingerprints` (
  `id` int(10) unsigned NOT NULL,
  `SessionId` int(10) unsigned NOT NULL,
  `Token` varchar(200) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


CREATE TABLE `users` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Name` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `Password` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `Email` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  `Confirmed` int(1) unsigned NOT NULL DEFAULT '0',
  `Rights` int(10) unsigned NOT NULL DEFAULT '0',
  `ConfirmationToken` varchar(200) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `users` (`Id`, `Name`, `Password`, `Email`, `Confirmed`, `Rights`, `ConfirmationToken`) VALUES
(1,	'a',	'a',	'jozsab1@gmail.com',	1,	0,	'b5b656919f38f1742f375551b89fc4360f283795ec13115f'),
(3,	'Raluca',	'pass',	'raluca_popovici@waters.com',	0,	0,	NULL);

CREATE TABLE `userswipes` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `SessionId` int(10) unsigned NOT NULL,
  `IdeaId` int(10) unsigned NOT NULL,
  `Swipe` int(3) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


CREATE TABLE `usertokens` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `SessionId` int(10) unsigned NOT NULL,
  `Token` varchar(200) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `usertokens` (`id`, `SessionId`, `Token`) VALUES
(8,	2,	'8c836a01dd39f0d77477b3580db14350b997b57031e9606a'),
(7,	2,	'df1a9a7bcab357171634977b684a7c5bb18c32723df59577'),
(6,	2,	'28fec20abac7f30aeb75988497fb8f03b0942eb244e32a74'),
(5,	2,	'23dd30980cb886e487458b94e54cad9f115076f9e1b99d8e'),
(9,	2,	'df274c48db2d9d8f5fbdd055a50a35149d9679ceb526a9e1'),
(10,	2,	'a9343dc3486ec6938aa664752afa746aaf1d26dcd5cccdb8'),
(11,	2,	'0a54f62eb8a35affcf1cb158ebfe058074232349f0cb51c8'),
(12,	2,	'3ead8fa17a1faa5f33cdcc0aa35136edfa27fc119c2aec40'),
(13,	2,	'7ea797d812a355f7254c2dfdd5fcebcd31a67fd00a4d9502'),
(14,	2,	'89e074022c37cb8cb2a820ce9b1b9b752e5e00df41a05499'),
(15,	2,	'5aafc4df16359ffe92a90cf00f094bdb6a9884f22988398b');

CREATE TABLE `uservotes` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `SessionId` int(10) unsigned NOT NULL,
  `IdeaId` int(10) unsigned NOT NULL,
  `VoteResult` int(2) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `uservotes` (`id`, `SessionId`, `IdeaId`, `VoteResult`) VALUES
(5,	2,	8,	-1),
(6,	2,	7,	1),
(7,	2,	6,	1),
(8,	2,	5,	-1),
(9,	2,	9,	1);

-- 2016-02-11 15:23:56
