//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/gamecvars.h                                   $
// $Revision:: 62                                                             $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#ifndef __GAMECVARS_H__
#define __GAMECVARS_H__

#include "g_local.h"

extern	cvar_t	*developer;
extern	cvar_t	*skill;
extern	cvar_t	*password;
extern	cvar_t	*g_needpass;
extern	cvar_t	*filterban;
//extern	cvar_t	*flood_msgs;
//extern	cvar_t	*flood_persecond;
//extern	cvar_t	*flood_waitdelay;
extern	cvar_t	*maxclients;
extern	cvar_t	*maxentities;
extern	cvar_t	*nomonsters;
extern	cvar_t	*precache;
extern	cvar_t	*dedicated;
extern	cvar_t	*detail;
extern	cvar_t	*com_blood;
extern   cvar_t   *whereami;
extern   cvar_t   *bosshealth;
extern   cvar_t   *bossname;

extern	cvar_t	*sv_maxvelocity;
extern	cvar_t	*sv_rollspeed;
extern  cvar_t  *sv_showdamageshake;
extern	cvar_t	*sv_rollangle;
extern	cvar_t	*sv_cheats;
extern	cvar_t	*sv_showbboxes;
extern	cvar_t	*sv_showcameras;
extern	cvar_t	*sv_showsplines;
extern	cvar_t	*sv_showentnums;
extern	cvar_t	*sv_showactnums;
extern  cvar_t	*sv_showdamagecolors ;
extern	cvar_t	*sv_waterfriction;
extern	cvar_t	*sv_traceinfo;
extern	cvar_t	*sv_drawtrace;
extern	cvar_t	*sv_fps;
extern   cvar_t   *sv_cinematic;
//extern   cvar_t   *sv_maplist;
extern   cvar_t   *sv_nextmap;

extern cvar_t	*sv_currentGravity;

// New server cvars
extern cvar_t	*sv_maxspeed;
extern cvar_t	*sv_stopspeed;
extern cvar_t	*sv_friction;
extern cvar_t	*sv_gravity;
extern cvar_t	*sv_waterspeed;
extern cvar_t	*sv_wateraccelerate;
extern cvar_t	*sv_groundtracelength;
extern cvar_t	*sv_jumpvelocity;
extern cvar_t	*sv_crouchjumpvelocity;
extern cvar_t	*sv_edgefriction;
extern cvar_t	*sv_airmaxspeed;
extern cvar_t	*sv_airaccelerate;
extern cvar_t	*sv_crouchspeed;
extern cvar_t	*sv_noclipspeed;
extern cvar_t	*sv_accelerate;
extern cvar_t	*sv_defaultviewheight;
extern cvar_t	*sv_instantjump;
extern cvar_t	*sv_cancrouch;
extern cvar_t	*sv_useanimmovespeed;
extern cvar_t	*sv_defaultFov;
extern cvar_t	*sv_strafeJumpingAllowed;

extern	cvar_t	*csys_posx;
extern	cvar_t	*csys_posy;
extern	cvar_t	*csys_posz;
extern	cvar_t	*csys_x;
extern	cvar_t	*csys_y;
extern	cvar_t	*csys_z;
extern	cvar_t	*csys_draw;

extern	cvar_t	*g_showmem;
extern	cvar_t	*g_timeents;
extern	cvar_t	*g_showaxis;
extern	cvar_t	*g_showgravpath;
extern	cvar_t	*g_showplayerstate;
extern	cvar_t	*g_showplayeranim;
extern	cvar_t	*g_showplayerweapon;
extern   cvar_t   *g_showbullettrace;
extern	cvar_t   *g_showactortrace;
extern	cvar_t   *g_showactorpath;
extern	cvar_t	*g_legswingspeed;
extern cvar_t	*g_intermissiontime;
extern cvar_t	*g_endintermission;
extern	cvar_t	*g_legclampangle;
extern	cvar_t	*g_legclamptolerance;
extern	cvar_t	*g_legtolerance;
extern	cvar_t	*g_numdebuglines;
extern	cvar_t	*g_playermodel;
extern	cvar_t	*g_statefile;
extern	cvar_t	*g_showautoaim;
extern   cvar_t   *g_debugtargets;
extern   cvar_t   *g_debugdamage;
extern	cvar_t	*s_debugmusic;
extern	cvar_t	*g_logstats;
extern   cvar_t   *g_gametype;
extern	cvar_t	*g_showaccuracymod;
extern	cvar_t	*g_aimviewangles;
extern   cvar_t   *g_debug;

extern cvar_t *g_secretCount;

//extern   cvar_t   *g_allowActionMusic;

extern	cvar_t	*g_allowActionMusic;

extern   cvar_t   *scr_maxerrors;
extern   cvar_t   *scr_printfunccalls;
extern   cvar_t   *scr_printeventcalls;

extern   cvar_t   *ai_numactive;

extern cvar_t *mp_gametype;
extern cvar_t *mp_flags;
extern cvar_t *mp_pointlimit;
extern cvar_t *mp_timelimit;
extern cvar_t *mp_itemRespawnMultiplier;
extern cvar_t *mp_weaponRespawnMultiplier;
extern cvar_t *mp_powerupRespawnMultiplier;
extern cvar_t *mp_knockbackMultiplier;
extern cvar_t *mp_damageMultiplier;
extern cvar_t *mp_respawnInvincibilityTime;
extern cvar_t *mp_warmUpTime;
extern cvar_t *mp_minPlayers;
extern cvar_t *mp_bigGunMode;
extern cvar_t *mp_respawnTime;
extern cvar_t *mp_intermissionTime;
extern cvar_t *mp_bombTime;
extern cvar_t *mp_maxVotes;
extern cvar_t *mp_skipWeaponReloads;
extern cvar_t *mp_minTauntTime;

extern cvar_t *mp_useMapList;
extern cvar_t *mp_mapList;
extern cvar_t *mp_currentPosInMapList;

extern	cvar_t	*sv_showinfo;
extern	cvar_t	*sv_showinfodist;

void CVAR_Init( void );

#endif /* !__GAMECVARS_H__ */
