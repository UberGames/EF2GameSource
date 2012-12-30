//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/gamecvars.cpp                             $
// $Revision:: 94                                                             $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Definitions for any cvars used by the game.
//

#include "_pch_cpp.h"
#include "gamecvars.h"

cvar_t *developer;
cvar_t *skill;
cvar_t *password;
cvar_t *g_needpass;
cvar_t *filterban;
//cvar_t	*flood_msgs;
//cvar_t	*flood_persecond;
//cvar_t	*flood_waitdelay;
cvar_t *maxclients;
cvar_t *maxentities;
cvar_t *nomonsters;
cvar_t *precache;
cvar_t *dedicated;
cvar_t *detail;
cvar_t *com_blood;
cvar_t *whereami;

cvar_t *bosshealth;
cvar_t *bossname;

cvar_t *sv_maxvelocity;
cvar_t *sv_rollspeed;
cvar_t *sv_rollangle;
cvar_t *sv_cheats;
cvar_t *sv_showbboxes;
cvar_t *sv_showcameras;
cvar_t *sv_showentnums;
cvar_t *sv_showactnums;
cvar_t *sv_showsplines;
cvar_t *sv_waterfriction;
cvar_t *sv_traceinfo;
cvar_t *sv_drawtrace;
cvar_t *sv_fps;
cvar_t *sv_cinematic;
//cvar_t *sv_maplist;
cvar_t *sv_nextmap;
cvar_t *sv_showdamagecolors;
cvar_t *sv_showdamageshake;

cvar_t	*sv_currentGravity;

// New server cvars
cvar_t *sv_maxspeed;
cvar_t *sv_stopspeed;
cvar_t *sv_friction;
cvar_t *sv_gravity;
cvar_t *sv_waterspeed;
cvar_t *sv_wateraccelerate;
cvar_t *sv_groundtracelength;
cvar_t *sv_jumpvelocity;
cvar_t *sv_crouchjumpvelocity;
cvar_t *sv_edgefriction;
cvar_t *sv_airmaxspeed;
cvar_t *sv_airaccelerate;
cvar_t *sv_crouchspeed;
cvar_t *sv_noclipspeed;
cvar_t *sv_accelerate;
cvar_t *sv_defaultviewheight;
cvar_t *sv_instantjump;
cvar_t *sv_cancrouch;
cvar_t *sv_useanimmovespeed;
cvar_t *sv_defaultFov;
cvar_t *sv_strafeJumpingAllowed;

cvar_t *csys_posx;
cvar_t *csys_posy;
cvar_t *csys_posz;
cvar_t *csys_x;
cvar_t *csys_y;
cvar_t *csys_z;
cvar_t *csys_draw;

cvar_t *g_showmem;
cvar_t *g_timeents;
cvar_t *g_showaxis;
cvar_t *g_showgravpath;
cvar_t *g_showplayerstate;
cvar_t *g_showplayeranim;
cvar_t *g_showplayerweapon;
cvar_t *g_legswingspeed;
cvar_t *g_intermissiontime;
cvar_t *g_endintermission;
cvar_t *g_legclampangle;
cvar_t *g_legclamptolerance;
cvar_t *g_legtolerance;
cvar_t *g_numdebuglines;
cvar_t *g_playermodel;
cvar_t *g_statefile;
cvar_t *g_showbullettrace;
cvar_t *g_showactortrace;
cvar_t *g_showactorpath;
cvar_t *s_debugmusic;
cvar_t *g_showautoaim;
cvar_t *g_debugtargets;
cvar_t *g_debugdamage;
cvar_t *g_logstats;
cvar_t *g_gametype;
cvar_t *g_rankedserver;
cvar_t *g_showaccuracymod;
cvar_t *g_aimviewangles;
cvar_t *g_debug;
cvar_t *g_armoradaptionlimit;

cvar_t *g_allowActionMusic;

cvar_t *g_secretCount;

cvar_t *scr_maxerrors;
cvar_t *scr_printfunccalls;
cvar_t *scr_printeventcalls;

cvar_t *ai_numactive;

cvar_t *ai_showfailure;

cvar_t *mp_gametype;
cvar_t *mp_flags;
cvar_t *mp_pointlimit;
cvar_t *mp_timelimit;
cvar_t *mp_itemRespawnMultiplier;
cvar_t *mp_weaponRespawnMultiplier;
cvar_t *mp_powerupRespawnMultiplier;
cvar_t *mp_knockbackMultiplier;
cvar_t *mp_damageMultiplier;
cvar_t *mp_respawnInvincibilityTime;
cvar_t *mp_warmUpTime;
cvar_t *mp_minPlayers;
cvar_t *mp_bigGunMode;
cvar_t *mp_respawnTime;
cvar_t *mp_intermissionTime;
cvar_t *mp_bombTime;
cvar_t *mp_maxVotes;
cvar_t *mp_useMapList;
cvar_t *mp_mapList;
cvar_t *mp_currentPosInMapList;
cvar_t *mp_skipWeaponReloads;
cvar_t *mp_minTauntTime;

cvar_t *sv_showinfo;
cvar_t *sv_showinfodist;

void CVAR_Init( void )
{
	
	developer						= gi.cvar( "developer",						"0",		CVAR_CHEAT );
	precache						= gi.cvar( "sv_precache",					"1",		0 );
	dedicated						= gi.cvar( "dedicated",						"0",		CVAR_LATCH );
	skill							= gi.cvar( "skill",							"1",		CVAR_SERVERINFO|CVAR_LATCH );
#ifdef DEDICATED
	maxclients						= gi.cvar( "sv_maxclients",					"16",		CVAR_SERVERINFO | CVAR_LATCH );
	sv_maxspeed						= gi.cvar( "sv_maxspeed",					"400",		0 );
#else
	maxclients						= gi.cvar( "sv_maxclients",					"1",		CVAR_SERVERINFO | CVAR_LATCH );
	sv_maxspeed						= gi.cvar( "sv_maxspeed",					"350",		0 );
#endif
	maxentities						= gi.cvar( "maxentities",					"1024",		CVAR_LATCH );
	password						= gi.cvar( "password",						"",			CVAR_USERINFO );
	g_needpass						= gi.cvar( "g_needpass",					"0",		CVAR_SERVERINFO );
	filterban						= gi.cvar( "filterban",						"1",		0 );
	nomonsters						= gi.cvar( "nomonsters",					"0",		CVAR_SERVERINFO );
	//	flood_msgs           = gi.cvar( "flood_msgs",            "4",     0 );
	//	flood_persecond      = gi.cvar( "flood_persecond",       "4",     0 );
	//	flood_waitdelay      = gi.cvar( "flood_waitdelay",       "10",    0 );
	detail							= gi.cvar( "detail",						"1",		CVAR_ARCHIVE );
	com_blood						= gi.cvar( "com_blood",						"1",		CVAR_USERINFO|CVAR_SERVERINFO|CVAR_ARCHIVE );
	whereami						= gi.cvar( "whereami",						"0",		0 );
	
	bosshealth						= gi.cvar( "bosshealth",					"0",		0 );
	bossname						= gi.cvar( "bossname",						"",			0 );
	
	sv_rollspeed					= gi.cvar( "sv_rollspeed",					"200",		0 );
	sv_showdamageshake				= gi.cvar( "sv_showdamageshake",			"1",		0 );
	sv_rollangle					= gi.cvar( "sv_rollangle",					"2",		0 );
	sv_maxvelocity					= gi.cvar( "sv_maxvelocity",				"2000",		0 );
	sv_gravity						= gi.cvar( "sv_gravity",					"800",		0 );
	sv_currentGravity				= gi.cvar( "sv_currentGravity",				"800",		0 );
	sv_traceinfo					= gi.cvar( "sv_traceinfo",					"0",		0 );
	sv_drawtrace					= gi.cvar( "sv_drawtrace",					"0",		0 );
	sv_showbboxes					= gi.cvar( "sv_showbboxes",					"0",		CVAR_CHEAT );
	sv_showcameras					= gi.cvar( "sv_showcameras",				"0",		0 );
	sv_showsplines					= gi.cvar( "sv_showsplines",				"0",		0 );
	sv_showentnums					= gi.cvar( "sv_showentnums",				"0",		CVAR_CHEAT );
	sv_showactnums					= gi.cvar( "sv_showactnums",				"0",		0 );
	sv_showdamagecolors				= gi.cvar( "sv_showdamagecolors",			"1",		0 );
	sv_friction						= gi.cvar( "sv_friction",					"6",		CVAR_SERVERINFO );
	sv_stopspeed					= gi.cvar( "sv_stopspeed",					"50",		CVAR_SERVERINFO );
	sv_waterfriction				= gi.cvar( "sv_waterfriction",				"1",		CVAR_SERVERINFO );
	sv_waterspeed					= gi.cvar( "sv_waterspeed",					"300",		CVAR_SERVERINFO );
	sv_cheats						= gi.cvar( "cheats",						"0",		CVAR_SERVERINFO|CVAR_LATCH );
	sv_fps							= gi.cvar( "sv_fps",						"20",		CVAR_SERVERINFO );
	sv_cinematic					= gi.cvar( "sv_cinematic",					"0",		CVAR_SERVERINFO|CVAR_ROM );
	//sv_maplist						= gi.cvar( "sv_maplist",					"",			CVAR_ARCHIVE );
	sv_nextmap						= gi.cvar( "nextmap",						"",			0 );
	sv_wateraccelerate				= gi.cvar( "sv_wateraccelerate",			"6",		0 );
	sv_groundtracelength			= gi.cvar( "sv_groundtracelength",			"0.25",		0 );
	sv_jumpvelocity					= gi.cvar( "sv_jumpvelocity",				"350",		0 );
	sv_crouchjumpvelocity			= gi.cvar( "sv_crouchjumpvelocity",			"45",		0 );
	sv_edgefriction					= gi.cvar( "sv_edgefriction",				"8",		0 );
	sv_airmaxspeed					= gi.cvar( "sv_airmaxspeed",				"0",		0 );
	sv_airaccelerate				= gi.cvar( "sv_airaccelerate",				"2",		0 );
	sv_crouchspeed					= gi.cvar( "sv_crouchspeed",				"0",		0 );
	sv_noclipspeed					= gi.cvar( "sv_noclipspeed",				"400",		0 );
	sv_accelerate					= gi.cvar( "sv_accelerate",					"10",		0 );
	sv_defaultviewheight			= gi.cvar( "sv_defaultviewheight",			"85",		0 );
	sv_instantjump					= gi.cvar( "sv_instantjump",				"1",		CVAR_ROM);
	sv_useanimmovespeed				= gi.cvar( "sv_useanimmovespeed",			"0",		CVAR_ROM);
	sv_cancrouch					= gi.cvar( "sv_cancrouch",					"1",		CVAR_ROM);
	sv_defaultFov					= gi.cvar( "sv_defaultFov",					"90",		0 );
	sv_strafeJumpingAllowed			= gi.cvar( "sv_strafeJumpingAllowed",		"1",		0 );
	
	
	g_showmem						= gi.cvar( "g_showmem",						"0",		0 );
	g_timeents						= gi.cvar( "g_timeents",					"0",		0 );
	g_showaxis						= gi.cvar( "g_showaxis",					"0",		0 );
	g_showgravpath					= gi.cvar( "g_drawgravpath",				"0",		0 );
	g_showplayerstate				= gi.cvar( "g_showplayerstate",				"0",		0 );
	g_showplayeranim				= gi.cvar( "g_showplayeranim",				"0",		0 );
	g_showplayerweapon				= gi.cvar( "g_showplayerweapon",			"0",		0 );
	g_showbullettrace				= gi.cvar( "g_showbullettrace",				"0",		0 );
	g_showactortrace				= gi.cvar( "g_showactortrace",				"0",		0 );
	g_showactorpath					= gi.cvar( "g_showactorpath",				"0",		0 );
	g_showaccuracymod				= gi.cvar( "g_showaccuracymod",				"0",		0 );
	g_aimviewangles					= gi.cvar( "g_aimviewangles",				"0",		0 );
	
	g_legswingspeed					= gi.cvar( "g_legswingspeed",				"200",		0 );
	g_intermissiontime				= gi.cvar( "g_intermissiontime",			"10",		0 );
	g_endintermission				= gi.cvar( "g_endintermission",				"0",		0 );
	g_legclampangle					= gi.cvar( "g_legclampangle",				"90",		0 );
	g_legclamptolerance				= gi.cvar( "g_legclamptolerance",			"50",		0 );
	g_legtolerance					= gi.cvar( "g_legtolerance",				"40",		0 );
	
	g_numdebuglines					= gi.cvar( "g_numdebuglines",				"4096",  CVAR_LATCH );
	g_playermodel					= gi.cvar( "g_playermodel",					"char/mainchar", 0 );
	g_statefile						= gi.cvar( "g_statefile",					"char/mainchar", 0 );
	g_showautoaim					= gi.cvar( "g_showautoaim",					"0",		0 );
	g_debugtargets					= gi.cvar( "g_debugtargets",				"0",		0 );
	g_debugdamage					= gi.cvar( "g_debugdamage",					"0",		0 );
	g_logstats						= gi.cvar( "g_logstats",					"0",		0 );
	g_gametype						= gi.cvar( "g_gametype",					"0",		CVAR_SERVERINFO|CVAR_LATCH );
	g_rankedserver  				= gi.cvar( "g_rankedserver",				"0",		0 );
	
	g_allowActionMusic				= gi.cvar( "g_allowActionMusic",			"1",		0 );
	g_armoradaptionlimit			= gi.cvar( "g_armoradaptionlimit",			"300" ,		0 );

	g_secretCount					= gi.cvar( "g_secretCount",					"0" ,		CVAR_ARCHIVE | CVAR_ROM );
	//g_secretCount					= gi.cvar( "g_secretCount",					"0" ,		CVAR_ARCHIVE  );
	
	csys_posx						= gi.cvar( "csys_posx",						"0",		0 );
	csys_posy						= gi.cvar( "csys_posy",						"0",		0 );
	csys_posz						= gi.cvar( "csys_posz",						"0",		0 );
	csys_x							= gi.cvar( "csys_x",						"0",		0 );
	csys_y							= gi.cvar( "csys_y",						"0",		0 );
	csys_z							= gi.cvar( "csys_z",						"0",		0 );
	csys_draw						= gi.cvar( "csys_draw",						"0",		0 );
	
	s_debugmusic					= gi.cvar( "s_debugmusic",					"0",		0 );
	
	g_debug							= gi.cvar( "g_debug",						"0",		0 );
	
	scr_maxerrors					= gi.cvar( "scr_maxerrors",					"10",		0 );
	scr_printfunccalls				= gi.cvar( "scr_printfunccalls",			"0",		0 );
	scr_printeventcalls				= gi.cvar( "scr_printeventcalls",			"0",		0 );
	
	ai_numactive					= gi.cvar( "ai_numactive",					"0",		0 );
	
	ai_showfailure					= gi.cvar( "ai_showfailure",				"0",		0 );
	
	mp_gametype						= gi.cvar( "mp_gametype",					"0",		CVAR_SERVERINFO );
	mp_flags						= gi.cvar( "mp_flags",						"0",		CVAR_SERVERINFO );
	mp_pointlimit					= gi.cvar( "mp_pointlimit",					"0",		CVAR_SERVERINFO );
	mp_timelimit					= gi.cvar( "mp_timelimit",					"0",		CVAR_SERVERINFO );
	mp_itemRespawnMultiplier		= gi.cvar( "mp_itemRespawnMultiplier",		"1.0",		CVAR_SERVERINFO );
	mp_weaponRespawnMultiplier		= gi.cvar( "mp_weaponRespawnMultiplier",	"1.0",		CVAR_SERVERINFO );
	mp_powerupRespawnMultiplier		= gi.cvar( "mp_powerupRespawnMultiplier",	"1.0",		CVAR_SERVERINFO );
	mp_knockbackMultiplier			= gi.cvar( "mp_knockbackMultiplier",		"1.0",		CVAR_SERVERINFO );
	mp_damageMultiplier				= gi.cvar( "mp_damageMultiplier",			"1.0",		CVAR_SERVERINFO );
	mp_respawnInvincibilityTime		= gi.cvar( "mp_respawnInvincibilityTime",	"4",		CVAR_SERVERINFO );
	mp_warmUpTime					= gi.cvar( "mp_warmUpTime",					"10",		CVAR_SERVERINFO );
	mp_minPlayers					= gi.cvar( "mp_minPlayers",					"0",		CVAR_SERVERINFO );
	mp_bigGunMode					= gi.cvar( "mp_bigGunMode",					"0",		CVAR_SERVERINFO );
	mp_respawnTime					= gi.cvar( "mp_respawnTime",				"-1",		CVAR_SERVERINFO );
	mp_intermissionTime				= gi.cvar( "mp_intermissionTime",			"30",		0 );

	mp_bombTime						= gi.cvar( "mp_bombTime",					"30",		0 );
	mp_maxVotes						= gi.cvar( "mp_maxVotes",					"3",		0 );
	mp_skipWeaponReloads			= gi.cvar( "mp_skipWeaponReloads",			"0",		0 );
	mp_minTauntTime					= gi.cvar( "mp_minTauntTime",				"4",		CVAR_ARCHIVE );

	mp_useMapList					= gi.cvar( "mp_useMapList",					"0",		0 );
	mp_mapList						= gi.cvar( "mp_mapList",					"",			CVAR_ARCHIVE );
	mp_currentPosInMapList			= gi.cvar( "mp_currentPosInMapList",		"0",		0 );
	
	sv_showinfo	    				= gi.cvar( "sv_showinfo",					"0",		0 );
	sv_showinfodist					= gi.cvar( "sv_showinfodist",				"256",		0 );
}
