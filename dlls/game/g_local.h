//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/g_local.h                                     $
// $Revision:: 20                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// local definitions for game module
//

#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__

#include "q_shared.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and gentity_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "g_public.h"
#include "bg_public.h"
#include "container.h"
#include "str.h"

// times for posting events
// Even though negative times technically don't make sense, the effect is to
// sort events that take place at the start of a map so that they are executed
// in the proper order.  For example, spawnargs must occur before any script
// commands take place, while unused entities must be removed before the spawnargs
// are parsed.

#define  EV_REMOVE            -12.0f    // remove any unused entities before spawnargs are parsed
#define  EV_PRIORITY_SPAWNARG -11.0f    // for priority spawn args passed in by the bsp file
#define  EV_SPAWNARG          -10.0f    // for spawn args passed in by the bsp file
#define  EV_LINKDOORS         -9.0f     // for finding out which doors are linked together
#define  EV_LINKBEAMS         -9.0f     // for finding out the endpoints of beams
#define  EV_SETUP_ROPEPIECE   -8.0f
#define  EV_SETUP_ROPEBASE    -7.0f
#define  EV_PROCESS_INIT      -6.0f

#define  EV_POSTSPAWN         -1.0f     // for any processing that must occur after all objects are spawned

#define SOUND_RADIUS          1500.0f    // Sound travel distance for AI

#define	random()					((rand () & 0x7fff) / ((float)0x7fff))
#define	crandom()				(2.0f * (random() - 0.5f))

// predefine Entity so that we can add it to gentity_t without any errors
class Entity;

#define MAX_NETNAME 36

// client data that stays across multiple level loads
typedef struct
	{
	char			userinfo[MAX_INFO_STRING];
	char			netname[MAX_NETNAME];
	char			mp_playermodel[MAX_QPATH];
	char			dm_morph_c1[MAX_INFO_STRING];
	//char			lastTeam[ 16 ];
	float			enterTime;

	qboolean		mp_lowBandwidth;
	//qboolean		mp_savingDemo;
	//float			userFov;

	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;

	} client_persistant_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
typedef struct gclient_s
	{
	// known to server
	playerState_t			ps;				// communicated by server to clients
	int						ping;

	// private to game
	client_persistant_t	pers;
   vec3_t		         cmd_angles;	   // angles sent over in the last command
   } gclient_t;

struct gentity_s
	{
	entityState_t	            s;				// communicated by server to clients
	struct         gclient_s	*client;	   // NULL if not a player
	qboolean	                  inuse;
	qboolean	                  linked;		// qfalse if not in any good cluster
	int		   	            linkcount;

	int			               svflags;		// SVF_NOCLIENT, SVF_BROADCAST, etc

	qboolean	                  bmodel;		// if false, assume an explicit mins / maxs bounding box
									               // only set by gi.SetBrushModel
	vec3_t		               mins, maxs;
	int			               contents;	// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									               // a non-solid entity should set to 0

	vec3_t		               absmin, absmax;   // derived from mins/maxs and origin + rotation

   float                      radius;     // radius of object
   vec3_t                     centroid;   // centroid, to be used with radius
   int                        areanum;    // areanum needs to be seen inside the game as well

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		               currentOrigin;
	vec3_t		               currentAngles;

	int			               ownerNum;			// objects never interact with their owners, to
									                     // prevent player missiles from immediately
									                     // colliding with their owner

   solid_t			            solid;   // Added for FAKK2

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================

   Entity			*entity;
	float				freetime;			// svs.time when the object was freed
	float				spawntime;			// svs.time when the object was spawned

	float				radius2;				// squared radius of object.  Used in findradius in g_utils.cpp

	char				entname[ 64 ];

   // GAMEFIX Moved some of the old fields here for the game code.   These
   // might still be needed or might not :-)
   int				clipmask;

	gentity_t		*next;
	gentity_t		*prev;
	};

typedef enum
   {
   legs,
   torso,
   all
   } bodypart_t;

//bot settings from i
typedef struct bot_settings_s
{
	char characterfile[MAX_QPATH];
	float skill;
	char team[MAX_QPATH];
} bot_settings_t;

// for bot chats
#define SAY_ALL		0
#define SAY_TEAM	1
#define SAY_TELL	2

typedef enum
{
	MP_ITEM_TYPE_NORMAL,
	MP_ITEM_TYPE_POWERUP,
	MP_ITEM_TYPE_RUNE,
	MP_ITEM_TYPE_WEAPON,
	MP_ITEM_TYPE_ARMOR
} MultiplayerItemType;

#include "vector.h"
#include "Linklist.h"
#include "class.h"
#include "game.h"
#include "g_main.h"
#include "listener.h"
#include "g_utils.h"
#include "g_spawn.h"
#include "g_phys.h"
#include "debuglines.h"

#endif // __G_LOCAL_H__
