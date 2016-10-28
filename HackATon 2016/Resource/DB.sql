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
(8,	1,	'na 1 2 3',	1,	1,	1,	'db953af4919dd881485d54d13a067315',	1455109684,	'8c836a01dd39f0d77477b3580db14350b997b57031e9606a',	0,	2,	0,	NULL),
(7,	0,	'a 3',	1,	0,	1,	'db953af4919dd881485d54d13a067315',	1455109676,	'df1a9a7bcab357171634977b684a7c5bb18c32723df59577',	1,	1,	0,	NULL),
(6,	0,	'a 2',	1,	1,	0,	'db953af4919dd881485d54d13a067315',	1455109671,	'28fec20abac7f30aeb75988497fb8f03b0942eb244e32a74',	1,	0,	0,	NULL),
(5,	0,	'a 1',	1,	0,	0,	'db953af4919dd881485d54d13a067315',	1455109665,	'23dd30980cb886e487458b94e54cad9f115076f9e1b99d8e',	0,	1,	0,	NULL),
(9,	1,	'na 1 3',	1,	1,	0,	'db953af4919dd881485d54d13a067315',	1455109693,	'df274c48db2d9d8f5fbdd055a50a35149d9679ceb526a9e1',	1,	0,	0,	NULL);

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
(1,	'a',	'a',	'jozsab1@gmail.com',	1,	0,	'b5b656919f38f1742f375551b89fc4360f283795ec13115f');

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
(9,	2,	'df274c48db2d9d8f5fbdd055a50a35149d9679ceb526a9e1');

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

-- 2016-02-10 16:34:43
