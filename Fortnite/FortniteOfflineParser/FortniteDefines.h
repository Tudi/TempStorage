#pragma once


#define FILE_MAGIC 0x1CA2E27F
#define NETWORK_MAGIC 0x2CF5A13D

#define	EV_PLAYER_ELIMINATION "playerElim"
#define	EV_MATCH_STATS "AthenaMatchStats"
#define	EV_TEAM_STATS "AthenaMatchTeamStats"
#define	EV_ENCRYPTION_KEY "PlayerStateEncryptionKey"
#define	EV_CHARACTER_SAMPLE "CharacterSampleMeta"
#define	EV_ZONE_UPDATE "ZoneUpdate"
#define	EV_BATTLE_BUS "BattleBusFlight"

enum FortniteChunkTypes
{
  FN_CHUNK_HEADER = 0,
  FN_CHUNK_CHECKPOINT = 1,
  FN_CHUNK_REPLAY = 2,
  FN_CHUNK_EVENT = 3,
};

enum WeaponTypes
{
	STORM = 0,
	FALL = 1,
	PISTOL = 2,
	SHOTGUN = 3,
	AR = 4,
	SMG = 5,
	SNIPER = 6,
	PICKAXE = 7,
	GRENADE = 8,
	GRENADELAUNCHER = 10,
	RPG = 11,
	MINIGUN = 12,
	//BOW = 13,
	TRAP = 14,
	FINALLYELIMINATED = 15,
	//UNKNOWN16 = 16,
	//UNKNOWN17 = 17, // bleed out by storm ?
	VEHICLE = 21,
	LMG = 22,
	GASNADE = 23,
	OUTOFBOUND = 24,
	//TURRET = 25,
	TEAMSWITCH = 26,
	//UNKNOWN27 = 27, // TURRET HEADSHOT ?
	//BIPLANE_GUNS = 38,
	//BIPLANE_GUNS = 39,
	MISSING = 99,
};

enum HystoryTypes
{
	HISTORY_INITIAL = 0,
	HISTORY_FIXEDSIZE_FRIENDLY_NAME = 1,
	HISTORY_COMPRESSION = 2,
	HISTORY_RECORDED_TIMESTAMP = 3,
};
/*
		@dataclass
		class TeamStats :
		""" Team stats from a replay """
		unknown : int
		position : int
		total_players : int
		*/