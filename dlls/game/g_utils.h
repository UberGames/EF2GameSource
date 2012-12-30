//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/g_utils.h                                     $
// $Revision:: 24                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#ifndef __G_UTILS_H__
#define __G_UTILS_H__

class Archiver;

void        G_ArchiveEdict( Archiver &arc, gentity_t *edict );

#include "entity.h"

int G_FindConfigstringIndex( const char *name, int start, int max, qboolean create );

void        G_AllocDebugLines( void );
void        G_DeAllocDebugLines( void );

void		   G_TouchTriggers (Entity *ent);
void		   G_TouchTeleporters (Entity *ent);
void			G_TouchSolids (Entity *ent);

Entity      *G_FindClass( const Entity *ent, const char *classname );
//Entity		*G_NextEntity( const Entity *ent );

void        G_CalcBoundsOfMove( const Vector &start, const Vector &end, const Vector &mins, const Vector &maxs, Vector *minbounds, Vector *maxbounds );

void        G_ShowTrace( const trace_t *trace, gentity_t *passent, const char *reason, Vector &start, Vector &end );
trace_t		G_Trace( const Vector &start, const Vector &mins, const Vector &maxs, const Vector &end, const Entity *passent, int contentmask, qboolean cylindrical, const char *reason, qboolean fulltrace = false );
trace_t     G_Trace( vec3_t start, const vec3_t mins, const vec3_t maxs, vec3_t end, gentity_t *passent, int contentmask, qboolean cylindrical, const char *reason, qboolean fulltrace = false );
trace_t		G_FullTrace( const Vector &start, const Vector &mins, const Vector &maxs, const Vector &end, Entity *passent, int contentmask, qboolean cylindrical, const char *reason );
trace_t     G_FullTrace( vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, gentity_t *passent, int contentmask, qboolean cylindrical, const char *reason );
void			G_TraceEntities(	const Vector &start, const Vector &mins, const Vector &maxs, const Vector &end, Container<Entity *>*victimlist, int contentmask );

void			SelectSpawnPoint( Vector &org, Vector &angles, str &thread );

Entity      *G_FindTarget( Entity *ent, const char *name );
Entity		*G_NextEntity( const Entity *ent );

qboolean	   M_CheckBottom( Entity *ent );

Vector		G_GetMovedir( float angle );
qboolean	   KillBox( Entity *ent );
qboolean	   IsNumeric( const char *str );

Entity		*findradius( const Entity *startent, const Vector &org, float rad );
Entity		*findclientsinradius( const Entity *startent, const Vector &org, float rad );

Vector		G_CalculateImpulse( const Vector &start, const Vector &end, float speed, float gravity );
Vector      G_PredictPosition( const Vector &start, const Vector &target, const Vector &targetvelocity, float speed );

qboolean    G_LoadAndExecScript( const char *filename, const char *label = NULL, qboolean quiet = qfalse );
CThread     *ExecuteThread( const str &thread_name, qboolean start = true, Entity *currentEnt = NULL );

int			MOD_NameToNum( const str &meansOfDeath );
const char *MOD_NumToName( int meansOfDeath );
qboolean    MOD_matches( int incoming_damage, int damage_type );

int			Soundtype_string_to_int( const str &soundtype_string );
int         Context_string_to_int( const str& context_string );

void G_MissionFailed( const str& reason );
void G_FinishMissionFailed( void );
void G_FadeOut( float delaytime );
void G_FadeSound( float delaytime );
void G_RestartLevelWithDelay( float delaytime );
void G_AutoFadeIn( void );
void G_ClearFade( void );
void G_StartCinematic( void );
void G_StopCinematic( void );
int G_NumClients( void );

void G_DrawXYBox( const Vector &center, float width, float red, float green, float blue, float alpha );

// copy the entity exactly including all its children
void        CloneEntity( Entity * dest, const Entity * src );

qboolean    OnSameTeam( const Entity *ent1, const Entity *ent2 );

//
// caching commands
//
void CacheResource( const char * stuff, Entity * ent = NULL );
void G_CacheStateMachineAnims( Entity *ent, const char *stateMachineName );
int modelIndex( const char *mdl );

void G_SetTrajectory( gentity_t *ent, const vec3_t org );
void G_SetConstantLight
   (
   unsigned int * constantlight,
   const float * red,
   const float * green,
   const float * blue,
   const float * radius,
   const int   * lightstyle = NULL
   );

void ChangeMusic
	(
   const char *current,
   const char *fallback,
   qboolean force
	);

void ChangeMusicVolume
	(
   float volume,
   float fade_time
   );

void RestoreMusicVolume
	(
   float fade_time
   );

void G_AllowMusicDucking( bool allowMusicDucking );
void G_AllowActionMusic( bool allowActionMusic );

void ChangeSoundtrack
	(
   const char * soundtrack
	);

void RestoreSoundtrack
	(
   void
	);

void G_BroadcastSound( Entity *ent, const Vector &origin, float radius = SOUND_RADIUS, int soundType = SOUNDTYPE_GENERAL);
void G_BroadcastAlert( Entity *ent, const Vector &orignin, const Vector &enemy, float radius = SOUND_RADIUS );

//==================================================================
//
// Inline functions
//
//==================================================================

/*
=================
G_GetEntity

Takes an index to an entity and returns pointer to it.
=================
*/

inline Entity *G_GetEntity
	(
	int entnum
	)

	{
   if ( ( entnum < 0 ) || ( entnum >= globals.max_entities ) )
		{
		gi.Error( ERR_DROP, "G_GetEntity: %d out of valid range.", entnum );
		}

   return ( Entity * )g_entities[ entnum ].entity;
	}

/*
=================
G_Random

Returns a number from 0<= num < 1

random()
=================
*/

inline float G_Random
	(
	void
	)

	{
	return ( ( float )( rand() & 0x7fff ) ) / ( ( float )0x8000 );
	}

/*
=================
G_Random

Returns a number from 0 <= num < n

random()
=================
*/

inline float G_Random
	(
	float n
	)

	{
	return G_Random() * n;
	}

/*
=================
G_CRandom

Returns a number from -1 <= num < 1

crandom()
=================
*/

inline float G_CRandom
	(
	void
	)

	{
	return G_Random( 2 ) - 1.0f;
	}

/*
=================
G_CRandom

Returns a number from -n <= num < n

crandom()
=================
*/

inline float G_CRandom
	(
	float n
	)

	{
	return G_CRandom() * n;
	}

/*
=================
G_FixSlashes

Converts all backslashes in a string to forward slashes.
Used to make filenames consistant.
=================
*/

inline str G_FixSlashes
   (
   const char *filename
   )

   {
	int i;
	int len;
	str text;

   if ( filename )
      {
	   // Convert all forward slashes to back slashes
	   text = filename;
	   len = text.length();
	   for( i = 0; i < len; i++ )
		   {
		   if ( text[ i ] == '\\' )
			   {
			   text[ i ] = '/';
			   }
		   }
      }

   return text;
   }

typedef enum
   {
   WEAPON_RIGHT,
   WEAPON_LEFT,
   WEAPON_DUAL,
   WEAPON_ANY,
   WEAPON_ERROR
   } weaponhand_t;

#define NUM_ACTIVE_WEAPONS WEAPON_ANY

typedef enum
   {
   FIRE_MODE1,
   FIRE_MODE2,
	FIRE_MODE3,
   MAX_FIREMODES,
   FIRE_ERROR
   } firemode_t;

firemode_t     WeaponModeNameToNum( const str &mode );
const char     *WeaponHandNumToName( weaponhand_t hand );
weaponhand_t   WeaponHandNameToNum( const str &side );
void           G_DebugTargets( Entity *e, const str &from );
void           G_DebugDamage( float damage, Entity *victim, Entity *attacker, Entity *inflictor );

float G_GetDatabaseFloat( const str &prefix, const str &objectName, const str &varName );
str G_GetDatabaseString( const str &prefix, const str &objectName, const str &varName );

#endif /* g_utils.h */
