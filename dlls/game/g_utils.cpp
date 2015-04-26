//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/g_utils.cpp                                   $
// $Revision:: 84                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 4/16/03 6:29p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#include "_pch_cpp.h"
#include "g_utils.h"
#include "ctype.h"
#include "worldspawn.h"
#include "scriptmaster.h"
#include "player.h"
#include "PlayerStart.h"
#include "mp_manager.hpp"
#include <qcommon/gameplaymanager.h>

char means_of_death_strings[ MOD_TOTAL_NUMBER ][ 32 ] =
{
	"none",
	"drown",
	"suicide",
	"crush",
	"crush_every_frame",
	"telefrag",
	"lava",
	"slime",
	"falling",
	"last_self_inflicted",
	"explosion",
	"explodewall",
	"electric",
	"electricwater",
	"thrownobject",
	"beam",
	"rocket",
	"impact",
	"gas",
	"gas_blockable",
	"acid",
	"sword",
	"plasma",
	"plasmabeam",
	"plasmashotgun",
	"sting",
	"sting2",
	"sling",
	"bullet",
	"fast_bullet",
	"vehicle",
	"fire",
	"fire_blockable",
	"vortex",
	"lifedrain",
	"flashbang",
	"poo_explosion",
	"axe",
	"chainsword",
	"on_fire",
	"firesword",
	"electricsword",
	"circleofprotection",
	"radiation",
	"lightsword",
	"gib",
	"impale",
	"uppercut",
	"poison",
	"eat",
	"redemption",
	"stasis",
	
	// Added for EF
	"phaser",
	"vaporize",
	"comp_rifle",
	"vaporize_comp",
	"imod_primary",
	"imod_secondary",
	"small_explosion",
	"tetryon",
	"disruptor",	
	"vaporize_disruptor",
	"vaporize_photon",
	"sniper",
	"melee",
	"alien_melee",
	"klingon_melee",
	"turret",
	
	// Powerups/runes
		
	"deathQuad",
	"empathyShield",
	"armorPiercing"
};

char soundtype_strings[ SOUNDTYPE_TOTAL_NUMBER ][ 32 ] =
{
	// General Sound Types
	"none",
	"general",
	"explosion",
	"weaponfire",
	"alert",
	"footsteps_walk",
	"footsteps_run",
	"fall",
		
	// Context Dialog Sound Types
	"dialog_context_spotted_enemy",
	"dialog_context_injured",
	"dialog_context_in_combat",
	"dialog_context_weapon_useless",
	"dialog_context_investigating"
};

char context_strings[CONTEXT_TOTAL_NUMBER][32] =
{
	"spotted_enemy",
	"injured",
	"in_combat",
	"weapon_useless",
	"investigating"
};

int Soundtype_string_to_int ( const str &soundtype_string )
{
	for (int i = 0 ; i < SOUNDTYPE_TOTAL_NUMBER ; i++ )
	{
		if ( !soundtype_string.icmp( soundtype_strings[ i ] ) )
			return i;
	}
	
	gi.WDPrintf( "Unknown soundtype - %s\n", soundtype_string.c_str() );
	return -1;
}

int Context_string_to_int ( const str &context_string )
{
	for (int i = 0 ; i < CONTEXT_TOTAL_NUMBER ; i++ )
	{
		if ( !context_string.icmp( context_strings[ i ] ) )
			return i;
	}
	
	gi.WDPrintf( "Unknown context - %s\n", context_string.c_str() );
	return -1;
}

int MOD_NameToNum( const str &meansOfDeath )
{
	int i;
	
	for ( i = 0 ; i < MOD_TOTAL_NUMBER ; i++ )
	{
		if ( !meansOfDeath.icmp( means_of_death_strings[ i ] ) )
			return i;
	}
	
	gi.WDPrintf( "Unknown means of death - %s\n", meansOfDeath.c_str() );
	return -1;
}

const char *MOD_NumToName( int meansOfDeath )
{
	if ( ( meansOfDeath > MOD_TOTAL_NUMBER ) || ( meansOfDeath < 0 ) )
	{
		gi.WDPrintf( "Unknown means of death num - %d\n", meansOfDeath );
		return "";
	}

	return means_of_death_strings[ meansOfDeath ];
}

qboolean MOD_matches( int incoming_damage, int damage_type )
{
	if ( damage_type == -1 )
	{
		return true;
	}
	
	// special case the sword
	if ( damage_type == MOD_SWORD )
	{
		if (
            ( incoming_damage == MOD_SWORD ) ||
            ( incoming_damage == MOD_ELECTRICSWORD ) ||
            ( incoming_damage == MOD_LIGHTSWORD ) ||
            ( incoming_damage == MOD_FIRESWORD )
			)
		{
			return true;
		}
	}
	else if ( damage_type == incoming_damage )
	{
		return true;
	}
	
	return false;
}

/*
============
G_TouchTriggers

============
*/
void G_TouchTriggers( Entity *ent )
{
	int		i;
	int		num;
	int      touch[ MAX_GENTITIES ];
	gentity_t  *hit;
	Event		*ev;
	
	// dead things don't activate triggers!
	if ( ( ent->client || ( ent->edict->svflags & SVF_MONSTER ) ) && ( ent->health <= 0.0f ) )
	{
		return;
	}
	
	// Get a list of g_entity_t's that this entity touched
	num = gi.AreaEntities( ent->absmin, ent->absmax, touch, MAX_GENTITIES, qfalse );
	
	Container<EntityPtr>	&lastTouchedList	= ent->GetLastTouchedList();
	Container<EntityPtr>	 newTouchedList ;
	Container<EntityPtr>	 newLastTouchedList ;
	int						 numTouchedEntities	= lastTouchedList.NumObjects();
	int						 touchedEntityIdx	= 1 ;
	
	// Make a new list of entities that are valid touch targets
	for( i = 0; i < num; i++ )
	{
		hit = &g_entities[ touch[ i ] ];
		if ( !hit->inuse || ( hit->entity == ent ) || ( hit->solid != SOLID_TRIGGER ) ) continue;
		
		assert( hit->entity );
		newTouchedList.AddObject( hit->entity );
	}
	
	// If there are entities in our lastTouchedList that we are no longer touching, notify them
	for ( touchedEntityIdx=1; touchedEntityIdx <= numTouchedEntities; ++touchedEntityIdx )
	{
		Entity *touchedEntity = lastTouchedList.ObjectAt( touchedEntityIdx );
		if (!touchedEntity) continue ;
		if ( !newTouchedList.ObjectInList( touchedEntity ) )
		{
			ev = new Event ( EV_LostContact ); // call me!
			ev->AddEntity( ent );
			touchedEntity->ProcessEvent( ev );
		}
		else
		{
			ev = new Event ( EV_Touch );
			ev->AddEntity( ent );
			touchedEntity->ProcessEvent( ev );
			newTouchedList.RemoveObject( touchedEntity );
			newLastTouchedList.AddObject( touchedEntity );
		}
	}
	
	
	// For each new object we're touching, notify it, and add it to our last touched list.
	lastTouchedList.ClearObjectList();
	numTouchedEntities = newTouchedList.NumObjects();
	for ( touchedEntityIdx = 1; touchedEntityIdx <= numTouchedEntities; ++touchedEntityIdx )
	{
		Entity *touchedEntity = newTouchedList.ObjectAt( touchedEntityIdx );
		lastTouchedList.AddObject( touchedEntity );
		
		// This is for legacy reasons
		ev = new Event( EV_Touch );
		ev->AddEntity( ent );
		touchedEntity->ProcessEvent( ev );
		
		ev = new Event( EV_Contact );
		ev->AddEntity( ent );
		touchedEntity->ProcessEvent( ev );
	}
	
	numTouchedEntities = newLastTouchedList.NumObjects();
	for ( touchedEntityIdx = 1; touchedEntityIdx <= numTouchedEntities; ++touchedEntityIdx )
	{
		lastTouchedList.AddObject( newLastTouchedList.ObjectAt( touchedEntityIdx ) );
	}
}

void G_TouchTeleporters( Entity *ent )
{
	int		i;
	int		num;
	int      touch[ MAX_GENTITIES ];
	gentity_t  *hit;
	Event		*ev;
	
	// dead things don't activate triggers!

	if ( ( ent->client || ( ent->edict->svflags & SVF_MONSTER ) ) && ( ent->health <= 0.0f ) )
	{
		return;
	}
	
	// Get a list of g_entity_t's that this entity touched

	num = gi.AreaEntities( ent->absmin, ent->absmax, touch, MAX_GENTITIES, qfalse );
	
	// Touch all in the list that are teleporters

	for( i = 0 ; i < num ; i++ )
	{
		hit = &g_entities[ touch[ i ] ];

		if ( !hit->inuse || ( hit->entity == ent ) || ( hit->solid != SOLID_TRIGGER ) ) 
			continue;
		
		if ( hit->entity && hit->entity->isSubclassOf( Teleporter ) )
		{
			ev = new Event( EV_Touch );
			ev->AddEntity( ent );
			hit->entity->ProcessEvent( ev );
		}
	}
}


/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void G_TouchSolids( Entity *ent )
{
	int		i;
	int		num;
	int      touch[ MAX_GENTITIES ];
	gentity_t  *hit;
	Event		*ev;
	
	num = gi.AreaEntities( ent->absmin, ent->absmax, touch, MAX_GENTITIES, qfalse );
	
	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for( i = 0; i < num; i++ )
	{
		hit = &g_entities[ touch[ i ] ];
		if ( !hit->inuse )
		{
			continue;
		}
		
		assert( hit->entity );
		
		//FIXME
		// should we post the events so that we don't have to worry about any entities going away
		ev = new Event( EV_Touch );
		ev->AddEntity( ent );
		hit->entity->ProcessEvent( ev );
	}
}

void G_ShowTrace( const trace_t *trace, gentity_t *passent, const char *reason, Vector &start, Vector &end )
{
	str text;
	str pass;
	str hit;
	Vector delta;
	float dist;
	
	assert( reason );
	assert( trace );
	
	if ( passent )
	{
		pass = va( "'%s'(%d)", passent->entname, passent->s.number );
	}
	else
	{
		pass = "NULL";
	}
	
	if ( trace->ent )
	{
		hit = va( "'%s'(%d)", trace->ent->entname, trace->ent->s.number );
	}
	else
	{
		hit = "NULL";
	}
	
	delta = end - start;
	dist = delta.length();
	
	text = va( "%0.2f : Pass %s Frac %f Hit %s len %f: '%s'\n",
		level.time, pass.c_str(), trace->fraction, hit.c_str(), dist, reason ? reason : "" );
	
	if ( sv_traceinfo->integer == 3 )
	{
		gi.DebugPrintf( text.c_str() );
	}
	else
	{
		gi.DPrintf( "%s", text.c_str() );
	}
}

void G_CalcBoundsOfMove( const Vector &start, const Vector &end, const Vector &mins, const Vector &maxs, Vector *minbounds,
		Vector *maxbounds )
{
	Vector bmin;
	Vector bmax;
	
	ClearBounds( bmin, bmax );
	AddPointToBounds( ( Vector )start, bmin, bmax );
	AddPointToBounds( ( Vector )end, bmin, bmax );
	bmin += mins;
	bmax += maxs;
	
	if ( minbounds )
	{
		*minbounds = bmin;
	}
	
	if ( maxbounds )
	{
		*maxbounds = bmax;
	}
}

trace_t G_FullTrace( vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, gentity_t *passent, int contentmask,
		qboolean cylinder, const char *reason )
{
	return G_Trace( start, mins, maxs, end, passent, contentmask, cylinder, reason, true );
}

trace_t G_FullTrace( const Vector &start, const Vector &mins, const Vector &maxs, const Vector &end, Entity *passent,
		int contentmask, qboolean cylinder, const char *reason )
{
	return G_Trace( start, mins, maxs, end, passent, contentmask, cylinder, reason, true );
}

trace_t G_Trace( vec3_t start, const vec3_t mins, const vec3_t maxs, vec3_t end, gentity_t *passent, int contentmask,
		qboolean cylinder, const char *reason, qboolean fulltrace )
{
	int entnum;
	trace_t trace;
	
	if ( passent )
	{
		entnum = passent->s.number;
	}
	else
	{
		entnum = ENTITYNUM_NONE;
	}
	
	if ( fulltrace )
		gi.fulltrace( &trace, start, mins, maxs, end, entnum, contentmask, cylinder );
	else
		gi.trace( &trace, start, mins, maxs, end, entnum, contentmask, cylinder );
	
	if ( trace.entityNum == ENTITYNUM_NONE )
	{
		trace.ent = NULL;
	}
	else
	{
		trace.ent = &g_entities[ trace.entityNum ];
	}
	
	if ( sv_traceinfo->integer > 1 )
	{
		Vector start_vector = start;
		Vector end_vector   = end;
		
		G_ShowTrace( &trace, passent, reason, start_vector, end_vector );
	}
	sv_numtraces++;
	
	if ( sv_drawtrace->integer )
	{
		G_DebugLine( Vector( start ), Vector( end ), 1.0f, 1.0f, 0.0f, 1.0f );
	}
	
	return trace;
}

trace_t G_Trace( const Vector &start, const Vector &mins, const Vector &maxs, const Vector &end, const Entity *passent,
		int contentmask, qboolean cylinder, const char *reason, qboolean fulltrace )
{
	gentity_t *ent;
	int entnum;
	trace_t trace;
	
	assert( reason );
	
	if ( passent == NULL )
	{
		ent = NULL;
		entnum = ENTITYNUM_NONE;
	}
	else
	{
		ent = passent->edict;
		entnum = ent->s.number;
	}
	
	if ( fulltrace )
		gi.fulltrace( &trace, ( Vector )start, ( Vector )mins, ( Vector )maxs, ( Vector )end, entnum, contentmask, cylinder );
	else
		gi.trace( &trace, ( Vector )start, ( Vector )mins, ( Vector )maxs, ( Vector )end, entnum, contentmask, cylinder );
	
	if ( trace.entityNum == ENTITYNUM_NONE )
	{
		trace.ent = NULL;
	}
	else
	{
		trace.ent = &g_entities[ trace.entityNum ];
	}
	
	if ( sv_traceinfo->integer > 1 )
	{
		Vector start_vector = start;
		Vector end_vector   = end;
		G_ShowTrace( &trace, ent, reason, start_vector, end_vector );
	}
	
	sv_numtraces++;
	
	if ( sv_drawtrace->integer )
	{
		G_DebugLine( start, end, 1.0f, 1.0f, 0.0f, 1.0f );
	}
	
	return trace;
}

void G_TraceEntities( const Vector &start, const Vector &mins, const Vector &maxs, const Vector &end, 
		Container<Entity *>*victimlist, int contentmask )
{
	trace_t trace;
	vec3_t boxmins;
	vec3_t boxmaxs;
	int num;
	int touchlist[MAX_GENTITIES];
	gentity_t *touch;
	int i;
	
	
	// Find the bounding box
	
	for ( i=0 ; i<3 ; i++ )
	{
		if ( end[i] > start[i] )
		{
			boxmins[i] = start[i] + mins[i] - 1.0f;
			boxmaxs[i] = end[i] + maxs[i] + 1.0f;
		}
		else
		{
			boxmins[i] = end[i] + mins[i] - 1.0f;
			boxmaxs[i] = start[i] + maxs[i] + 1.0f;
		}
	}
	
	// Find the list of entites
	
	num = gi.AreaEntities( boxmins, boxmaxs, touchlist, MAX_GENTITIES, qfalse );
	
	for ( i=0 ; i<num ; i++ )
	{
		touch = &g_entities[ touchlist[ i ] ];
		
		// see if we should ignore this entity
		if (touch->solid == SOLID_NOT)
			continue;
		if (touch->solid == SOLID_TRIGGER)
			continue;
		
		gi.ClipToEntity( &trace, ( Vector )start, ( Vector )mins, ( Vector )maxs, ( Vector )end, touchlist[i], contentmask );
		
		if ( trace.entityNum == touchlist[i] )
			victimlist->AddObject( touch->entity );
	}
}

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot( const Entity *spot )
{
	Entity	*player;
	float		bestplayerdistance;
	Vector	v;
	int		n;
	float		playerdistance;
	
	bestplayerdistance = 9999999.0f;
	for( n = 0; n < maxclients->integer; n++ )
	{
		if ( !g_entities[ n ].inuse || !g_entities[ n ].entity )
		{
			continue;
		}
		
		player = g_entities[ n ].entity;
		if ( player->health <= 0.0f )
		{
			continue;
		}
		
		v = spot->origin - player->origin;
		playerdistance = v.length();
		
		if ( playerdistance < bestplayerdistance )
		{
			bestplayerdistance = playerdistance;
		}
	}
	
	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
Entity *SelectRandomDeathmatchSpawnPoint( const str &spawnpoint_type )
{
	Entity	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float		range, range1, range2;
	
	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;
	
	for( spot = G_FindClass( spot, spawnpoint_type ); spot ; spot = G_FindClass( spot, spawnpoint_type ) )
	{
		count++;
		range = PlayersRangeFromSpot( spot );
		if ( range < range1 )
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}
	
	if ( !count )
	{
		return NULL;
	}
	
	if ( count <= 2 )
	{
		spot1 = spot2 = NULL;
	}
	else
	{
		count -= 2;
	}
	
	selection = rand() % count;
	
	spot = NULL;
	do
	{
		spot = G_FindClass( spot, spawnpoint_type );
		
		// if there are no more, break out
		if ( !spot )
			break;
		
		if ( ( spot == spot1 ) || ( spot == spot2 ) )
		{
			selection++;
		}
	}
	while( selection-- );
	
	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
Entity *SelectFarthestDeathmatchSpawnPoint( void )
{
	Entity	*bestspot;
	float		bestdistance;
	float		bestplayerdistance;
	Entity	*spot;
	
	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	for( spot = G_FindClass( spot, "info_player_deathmatch" ); spot ; spot = G_FindClass( spot, "info_player_deathmatch" ) )
	{
		bestplayerdistance = PlayersRangeFromSpot( spot );
		if ( bestplayerdistance > bestdistance )
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}
	
	if ( bestspot )
	{
		return bestspot;
	}
	
	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_FindClass( NULL, "info_player_deathmatch" );
	
	return spot;
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
void SelectSpawnPoint( Vector &org, Vector &ang, str &thread )
{
	Entity *spot = NULL;
	
	// find a single player start spot
	
	while( ( spot = G_FindClass( spot, "info_player_start" ) ) != NULL )
	{
		if ( level.spawnpoint.icmp( spot->TargetName() ) == 0 )
		{
			break;
		}
	}
	
	if ( !spot && !level.spawnpoint.length() )
	{
		// there wasn't a spawnpoint without a target, so use any
		spot = G_FindClass( NULL, "info_player_start" );
	}
	
	if ( !spot )
	{
		gi.Error( ERR_DROP, "No player spawn position named '%s'.  Can't spawn player.\n", level.spawnpoint.c_str() );
	}
	
	org = spot->origin;
	ang = spot->angles;
	//
	// see if we have a thread
	//
	if ( spot->isSubclassOf( PlayerStart ) )
	{
		thread = ( ( PlayerStart * )spot )->getThread();
	}
}

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
//int c_yes, c_no;

qboolean M_CheckBottom( Entity *ent )
{
	Vector	mins, maxs, start, stop;
	trace_t	trace;
	int		x, y;
	float		mid, bottom;
	
	mins = ent->origin + ( ent->mins * 0.5f );
	maxs = ent->origin + ( ent->maxs * 0.5f );
	
	// if all of the points under the corners are solid world, don't bother
	// with the tougher checks
	// the corners must be within 16 of the midpoint
	start[ 2 ] = mins[ 2 ] - 1.0f;
	
	for( x = 0; x <= 1; x++ )
	{
		for( y = 0; y <= 1; y++ )
		{
			start[ 0 ] = x ? maxs[ 0 ] : mins[ 0 ];
			start[ 1 ] = y ? maxs[ 1 ] : mins[ 1 ];
			if ( gi.pointcontents( start, 0 ) != CONTENTS_SOLID )
			{
				goto realcheck;
			}
		}
	}
	
	//c_yes++;
	return true;		// we got out easy
	
realcheck:
	
	//c_no++;
	
	//
	// check it for real...
	//
	start[ 2 ] = mins[ 2 ];
	
	// the midpoint must be within 16 of the bottom
	start[ 0 ] = stop[ 0 ] = ( mins[ 0 ] + maxs[ 0 ] ) * 0.5f;
	start[ 1 ] = stop[ 1 ] = ( mins[ 1 ] + maxs[ 1 ] ) * 0.5f;
	stop[ 2 ] = start[ 2 ] - 3.0f * STEPSIZE;//2 * STEPSIZE;
	
	trace = G_Trace( start, vec_zero, vec_zero, stop, ent, MASK_MONSTERSOLID, false, "M_CheckBottom 1" );
	
	if ( trace.fraction == 1.0f )
	{
		return false;
	}
	
	mid = bottom = trace.endpos[ 2 ];
	
	// the corners must be within 16 of the midpoint
	/*
	for( x = 0; x <= 1; x++ )
	{
		for( y = 0; y <= 1; y++ )
		{
			start[ 0 ] = stop[ 0 ] = ( x ? maxs[ 0 ] : mins[ 0 ] );
			start[ 1 ] = stop[ 1 ] = ( y ? maxs[ 1 ] : mins[ 1 ] );
		
			trace = G_Trace( start, vec_zero, vec_zero, stop, ent, MASK_MONSTERSOLID, false, "M_CheckBottom 2" );
		  
			if ( ( trace.fraction != 1.0f ) && ( trace.endpos[ 2 ] > bottom ) )
			{
				bottom = trace.endpos[ 2 ];
			}
			
			if ( ( trace.fraction == 1.0f ) || ( ( mid - trace.endpos[ 2 ] ) > STEPSIZE ) )
			{
				return false;
			}
		}
	}
	*/
	//c_yes++;
	return true;
}

Entity * G_FindClass( const Entity * ent, const char *classname )
{
	int         entnum;
	gentity_t   *from;
	
	if ( ent )
	{
		entnum = ent->entnum;
	}
	else
	{
		entnum = -1;
	}
	
	for ( from = &g_entities[ entnum + 1 ]; from < &g_entities[ globals.num_entities ] ; from++ )
	{
		if ( !from->inuse )
		{
			continue;
		}
		if ( !Q_stricmp ( from->entity->getClassID(), classname ) )
		{
			return from->entity;
		}
	}
	
	return NULL;
}

Entity * G_FindTarget( Entity * ent, const char *name )
{
	Entity      *next;
	
	if ( name && name[ 0 ] )
	{
		if ( name[ 0 ] == '$' )
			name++;
		
		next = world->GetNextEntity( str( name ), ent );
		if ( next )
		{
			return next;
		}
	}
	
	return NULL;
}

Entity *G_NextEntity( const Entity *ent )
{
	gentity_t *from;
	
	if ( !g_entities )
	{
		return NULL;
	}
	
	if ( !ent )
	{
		from = g_entities;
	}
	else
	{
		from = ent->edict + 1;
	}
	
	if ( !from )
	{
		return NULL;
	}
	
	for ( ; from < &g_entities[ globals.num_entities ] ; from++ )
	{
		if ( !from->inuse || !from->entity )
		{
			continue;
		}
		
		return from->entity;
	}
	
	return NULL;
}

//
// QuakeEd only writes a single float for angles (bad idea), so up and down are
// just constant angles.
//
Vector G_GetMovedir( float angle )
{
	if ( angle == -1.0f )
	{
		return Vector( 0.0f, 0.0f, 1.0f );
	}
	else if ( angle == -2.0f )
	{
		return Vector( 0.0f, 0.0f, -1.0f );
	}
	
	angle *= ( M_PI * 2.0f / 360.0f );
	return Vector( cos( angle ), sin( angle ), 0.0f );
}

/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox( Entity *ent )
{
	int		   i;
	int	   	num;
	int         touch[ MAX_GENTITIES ];
	gentity_t   *hit;
	Vector      min;
	Vector      max;
	int         fail;
	
	fail = 0;
	
	min = ent->origin + ent->mins;
	max = ent->origin + ent->maxs;
	
	num = gi.AreaEntities( min, max, touch, MAX_GENTITIES, qfalse );
	
	for( i = 0; i < num; i++ )
	{
		hit = &g_entities[ touch[ i ] ];
		
		if ( !hit->inuse || ( hit->entity == ent ) || !hit->entity || ( hit->entity == world ) || ( !hit->entity->edict->solid ) )
		{
			continue;
		}
		
		hit->entity->Damage( ent, ent, hit->entity->health + 100000.0f, ent->origin, vec_zero, vec_zero,
			0, DamageNoProtection, MOD_TELEFRAG );
		
		//
		// if we didn't kill it, fail
		//
		if ( ( hit->entity->getSolidType() != SOLID_NOT ) && ( hit->entity->health > 0.0f ) )
		{
			fail++;
		}
	}
	
	//
	// all clear
	//
	return !fail;
}

qboolean IsNumeric( const char *str )
{
	int len;
	int i;
	qboolean dot;
	
	if ( *str == '-' )
	{
		str++;
	}
	
	dot = false;
	len = strlen( str );
	for( i = 0; i < len; i++ )
	{
		if ( !isdigit( str[ i ] ) )
		{
			if ( ( str[ i ] == '.' ) && !dot )
			{
				dot = true;
				continue;
			}
			return false;
		}
	}
	
	return true;
}

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (org, radius)
=================
*/
Entity *findradius( const Entity *startent, const Vector &org, float rad )
{
	Vector		eorg;
	gentity_t	*from;
	float			r2, distance;
	
	if ( !startent )
	{
		from = active_edicts.next;
	}
	else
	{
		from = startent->edict->next;
	}
	
	assert( from );
	if ( !from )
	   {
		return NULL;
	}
	
	assert( ( from == &active_edicts ) || ( from->inuse ) );
	
	// square the radius so that we don't have to do a square root
	r2 = rad * rad;
	
	for( ; from != &active_edicts; from = from->next )
	   {
		assert( from->inuse );
		assert( from->entity );
		
		eorg = org - from->entity->centroid;
		
		// dot product returns length squared
		distance = eorg * eorg;
		
		if ( distance <= r2 )
		{
			return from->entity;
		}
		else
		{
			// subtract the object's own radius from this distance
			distance -= from->radius2;
			if ( distance <= r2 )
			{
				return from->entity;
			}
		}
	}
	
	return NULL;
}

/*
=================
findclientinradius

Returns clients that have origins within a spherical area

findclientinradius (org, radius)
=================
*/
Entity *findclientsinradius( const Entity *startent, const Vector &org, float rad )
{
	Vector	 eorg;
	gentity_t *ed;
	float		 r2;
	int       i;
	
	// square the radius so that we don't have to do a square root
	r2 = rad * rad;
	
	if ( !startent )
	{
		i = 0;
	}
	else
	{
		i = startent->entnum + 1;
	}
	
	for( ; i < game.maxclients; i++ )
	{
		ed = &g_entities[ i ];
		
		if ( !ed->inuse || !ed->entity )
		{
			continue;
		}
		
		eorg = org - ed->entity->centroid;
		
		// dot product returns length squared
		if ( ( eorg * eorg ) <= r2 )
		{
			return ed->entity;
		}
	}
	
	return NULL;
}

Vector G_CalculateImpulse( const Vector &start, const Vector &end, float speed, float gravity )
{
	float traveltime, vertical_speed;
	Vector dir, xydir, velocity;
	
	dir = end - start;
	xydir = dir;
	xydir.z = 0;
	traveltime = xydir.length() / speed;
	vertical_speed = ( dir.z / traveltime ) + ( 0.5f * gravity * sv_currentGravity->value * traveltime );
	xydir.normalize();
	
	velocity = speed * xydir;
	velocity.z = vertical_speed;
	return velocity;
}

Vector G_PredictPosition( const Vector &start, const Vector &target, const Vector &targetvelocity, float  speed )
{
	Vector projected;
	float traveltime;
	Vector dir, xydir;
	
	dir = target - start;
	xydir = dir;
	xydir.z = 0;
	traveltime = xydir.length() / speed;
	projected = target + ( targetvelocity * traveltime );
	
	return projected;
}

char *ClientTeam( const gentity_t *ent )
{
	static char	value[512];
	
	value[0] = 0;
	
	if (!ent->client)
		return value;
	
	if ( multiplayerManager.checkFlag( MP_FLAG_MODELTEAMS ) )
		COM_StripExtension( Info_ValueForKey( ent->client->pers.userinfo, "model" ), value );
	else if ( multiplayerManager.checkFlag( MP_FLAG_SKINTEAMS ) )
		COM_StripExtension( Info_ValueForKey( ent->client->pers.userinfo, "skin" ), value );
	
	return( value );
}

qboolean OnSameTeam( const Entity *ent1, const Entity *ent2 )
{
	char	ent1Team [512];
	char	ent2Team [512];
	
	if ( !multiplayerManager.checkFlag( MP_FLAG_MODELTEAMS | MP_FLAG_SKINTEAMS ) )
		return false;
	
	strcpy (ent1Team, ClientTeam (ent1->edict));
	strcpy (ent2Team, ClientTeam (ent2->edict));
	
	if ( !strcmp( ent1Team, ent2Team ) )
		return true;
	
	return false;
}

/*
==============
G_LoadAndExecScript

Like the man says...
==============
*/
qboolean G_LoadAndExecScript( const char *filename, const char *label, qboolean quiet )
{
	CThread *pThread;
	
	if ( gi.FS_ReadFile( filename, NULL, quiet ) != -1 )
	{
		pThread = Director.CreateThread( label );
		if ( pThread )
		{
			// start right away
			pThread->Start();
			return true;
		}
		else
		{
			gi.WDPrintf( "G_LoadAndExecScript : %s could not create thread.\n", filename );
			return false;
		}
	}
	
	return false;
}

CThread *ExecuteThread( const str &thread_name, qboolean start, Entity *currentEnt )
{
	if ( thread_name.length() )
	{
		CThread *pThread;
		
		pThread = Director.CreateThread( thread_name.c_str() );
		if ( pThread )
		{
			if ( currentEnt )
				pThread->SetCurrentEntity(currentEnt);
			
			if ( start )
            {
				// start right away
				pThread->Start();
            }
		}
		else
		{
			gi.WDPrintf( "StartThread::unable to go to %s\n", thread_name.c_str() );
			return NULL;
		}
		
		return pThread;
	}
	
	return NULL;
}

/*
==============
G_ArchiveEdict
==============
*/
void G_ArchiveEdict( Archiver &arc, gentity_t *edict )
{
	int    i;
	str    tempStr;
	
	assert( edict );
	
	//
	// this is written funny because it is used for both saving and loading
	//
	
	if ( edict->client )
	{
		arc.ArchiveRaw( edict->client, sizeof( *edict->client ) );
	}

	// Don't archive int number;

	arc.ArchiveInteger( &edict->s.instanceNumber );
	
	arc.ArchiveInteger( &edict->s.eType );
	arc.ArchiveInteger( &edict->s.eFlags );

	// Don't archive
	//trajectory_t	pos;
	//trajectory_t	apos;
	
	arc.ArchiveVec3( edict->s.netorigin );
	arc.ArchiveVec3( edict->s.origin );
	arc.ArchiveVec3( edict->s.origin2 );
	arc.ArchiveVec3( edict->s.netangles );
	arc.ArchiveVec3( edict->s.angles );
	
	arc.ArchiveVec3( edict->s.viewangles );
	
	arc.ArchiveUnsigned( &edict->s.constantLight );
	
	if ( arc.Saving() )
	{
		if ( edict->s.loopSound )
			tempStr = gi.getConfigstring( CS_SOUNDS + edict->s.loopSound );
		else
			tempStr = "";
		
		arc.ArchiveString( &tempStr );
	}
	else
	{
		arc.ArchiveString( &tempStr );
		
		if ( tempStr.length() )
			edict->s.loopSound = gi.soundindex( tempStr.c_str() );
		else
			edict->s.loopSound = 0;
	}
	
	arc.ArchiveFloat( &edict->s.loopSoundVolume );
	arc.ArchiveFloat( &edict->s.loopSoundMinDist );
	
	arc.ArchiveInteger( &edict->s.parent );
	arc.ArchiveInteger( &edict->s.tag_num );
	arc.ArchiveBoolean( &edict->s.attach_use_angles );
	arc.ArchiveVec3( edict->s.attach_offset );
	arc.ArchiveVec3( edict->s.attach_angles_offset );
	
	arc.ArchiveInteger( &edict->s.modelindex );
	arc.ArchiveInteger( &edict->s.viewmodelindex );
	//arc.ArchiveInteger( &edict->s.worldmodelindex );
	arc.ArchiveInteger( &edict->s.skinNum );
	
	if ( arc.Saving() )
	{
		if ( edict->s.customShader )
			tempStr = gi.getConfigstring( CS_IMAGES + edict->s.customShader );
		else
			tempStr = "";
		
		arc.ArchiveString( &tempStr );
	}
	else
	{
		arc.ArchiveString( &tempStr );
		
		if ( tempStr.length() )
			edict->s.customShader = gi.imageindex( tempStr.c_str() );
		else
			edict->s.customShader = 0;
	}

	if ( arc.Saving() )
	{
		if ( edict->s.customEmitter )
			tempStr = gi.getConfigstring( CS_IMAGES + edict->s.customEmitter );
		else
			tempStr = "";
		
		arc.ArchiveString( &tempStr );
	}
	else
	{
		arc.ArchiveString( &tempStr );
		
		if ( tempStr.length() )
			edict->s.customEmitter = gi.imageindex( tempStr.c_str() );
		else
			edict->s.customEmitter = 0;
	}
	
	arc.ArchiveFloat( &edict->s.animationRate );
	arc.ArchiveInteger( &edict->s.anim );
	arc.ArchiveInteger( &edict->s.frame );
	
	arc.ArchiveInteger( &edict->s.crossblend_time );
	
	arc.ArchiveInteger( &edict->s.torso_anim );
	arc.ArchiveInteger( &edict->s.torso_frame );
	arc.ArchiveInteger( &edict->s.torso_crossblend_time );
	
	for( i = 0; i < NUM_BONE_CONTROLLERS; i++ )
	{
		arc.ArchiveInteger( &edict->s.bone_tag[ i ] );
		arc.ArchiveVec3( edict->s.bone_angles[ i ] );
		arc.ArchiveVec4( edict->s.bone_quat[ i ] );
	}
	
	arc.ArchiveRaw( &edict->s.morph_controllers, sizeof( edict->s.morph_controllers ) );
	
	arc.ArchiveRaw( &edict->s.surfaces, sizeof( edict->s.surfaces ) );
	
	arc.ArchiveInteger( &edict->s.clientNum );
	arc.ArchiveInteger( &edict->s.groundEntityNum );
	arc.ArchiveInteger( &edict->s.solid );
	
	arc.ArchiveFloat( &edict->s.scale );
	arc.ArchiveFloat( &edict->s.alpha );
	arc.ArchiveInteger( &edict->s.renderfx );
	
	arc.ArchiveUnsigned( &edict->s.affectingViewModes );

	arc.ArchiveInteger( &edict->s.archeTypeIndex );
	arc.ArchiveInteger( &edict->s.missionObjective );

	arc.ArchiveInteger( &edict->s.infoIcon );
	
	arc.ArchiveRaw( &edict->s.effectsAnims, sizeof( edict->s.effectsAnims ) );
	
	arc.ArchiveInteger( &edict->s.bindparent );
	
	arc.ArchiveVec4( edict->s.quat );
	arc.ArchiveRaw( &edict->s.mat, sizeof( edict->s.mat ) );
	
	arc.ArchiveInteger( &edict->svflags );
	
	arc.ArchiveVec3( edict->mins );
	arc.ArchiveVec3( edict->maxs );
	arc.ArchiveInteger( &edict->contents );
	arc.ArchiveVec3( edict->absmin );
	arc.ArchiveVec3( edict->absmax );
	arc.ArchiveFloat( &edict->radius );
	
	if ( arc.Loading() ) 
	{
		edict->radius2 = edict->radius * edict->radius;
	}
	
	arc.ArchiveVec3( edict->centroid );
	
	arc.ArchiveVec3( edict->currentOrigin );
	arc.ArchiveVec3( edict->currentAngles );
	
	arc.ArchiveInteger( &edict->ownerNum );
	ArchiveEnum( edict->solid, solid_t );
	arc.ArchiveFloat( &edict->freetime );
	arc.ArchiveFloat( &edict->spawntime );
	
	tempStr = str( edict->entname );
	arc.ArchiveString( &tempStr );
	strncpy( edict->entname, tempStr.c_str(), sizeof( edict->entname ) - 1 );
	
	arc.ArchiveInteger( &edict->clipmask );
	
	if ( arc.Loading() )
	{
		gi.linkentity( edict );
	}
}

/*
=========================================================================

model / sound configstring indexes

=========================================================================
*/

/*
=======================
G_FindConfigstringIndex
=======================
*/
int G_FindConfigstringIndex( const char *name, int start, int max, qboolean create )
{
	int		i;
	char	   *s;
	
	if ( !name || !name[0] )
	{
		return 0;
	   }
	
	for ( i=1 ; i<max ; i++ )
	{
		s = gi.getConfigstring( start + i );
		if ( !s || !s[0] )
		{
			break;
		}
		if ( !strcmp( s, name ) )
		{
			return i;
		}
	   }
	
	if ( !create )
	{
		return 0;
	   }
	
	if ( i == max )
	{
		gi.Error( ERR_DROP, "G_FindConfigstringIndex: overflow" );
	   }
	
	gi.setConfigstring( start + i, name );
	return i;
}

int G_ModelIndex( const char *name )
{
	return G_FindConfigstringIndex (name, CS_MODELS, MAX_MODELS, true);
}

int G_SoundIndex( const char *name )
{
	return G_FindConfigstringIndex (name, CS_SOUNDS, MAX_SOUNDS, true);
}


/*
===============
G_SetTrajectory

Sets the pos trajectory for a fixed position
===============
*/
void G_SetTrajectory( gentity_t *ent, const vec3_t org )
{
	VectorCopy( org, ent->s.pos.trBase );
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	VectorClear( ent->s.pos.trDelta );
	
	VectorCopy( org, ent->currentOrigin );
	VectorCopy( org, ent->s.origin );
}

/*
===============
G_SetConstantLight

Sets the encoded constant light parameter for entities
===============
*/
void G_SetConstantLight( unsigned int * constantlight, const float * red, const float * green, const float * blue,
		const float * radius, const int * lightStyle )
{
	int ir, ig, ib, iradius;
	
	if ( !constantlight )
		return;
	
	ir = (*constantlight) & 255;
	ig = ( (*constantlight) >> 8 ) & 255;
	ib = ( (*constantlight) >> 16 ) & 255;
	iradius = ( (*constantlight) >> 24 ) & 255;
	
	if ( red )
	{
		ir = *red * 255;
		if ( ir > 255 )
			ir = 255;
	}
	
	if ( green )
	{
		ig = *green * 255;
		if ( ig > 255 )
			ig = 255;
	}
	
	if ( blue )
	{
		ib = *blue * 255;
		if ( ib > 255 )
			ib = 255;
	}
	
	if ( radius )
	{
		iradius = *radius / CONSTANTLIGHT_RADIUS_SCALE;
		if ( iradius > 255 )
			iradius = 255;
	}
	
	if ( lightStyle )
	{
		ir = *lightStyle;
		if ( ir > 255 )
			ir = 255;
	}
	*constantlight = ( ir ) + ( ig << 8 ) + ( ib << 16 ) + ( iradius << 24 );
}

//
// caching commands
//
int modelIndex( const char *mdl )
{
	str name;
	
	assert( mdl );
	
	if ( !mdl )
	{
		return 0;
	}
	
	// Prepend 'models/' to make things easier
	//if ( !strchr( mdl, '*' ) && !strchr( mdl, '\\' ) && !strchr( mdl, '/' ) )
	if ( ( strlen( mdl ) > 0 ) && !strchr( mdl, '*' ) && strnicmp( "models/", mdl, 7 ) && strstr( mdl, ".tik" ) )
	{
		name = "models/";
	}
	
	name += mdl;
	
	return gi.modelindex( name.c_str() );
}

void CacheResource( const char * stuff, Entity * ent )
{
	str real_stuff;

	if ( !stuff || !ent )
		return;

	if ( !strchr( stuff, '.' ) )
	{
		// must be a global alias
		stuff = gi.GlobalAlias_FindRandom( stuff );
		if ( !stuff )
		{
			return;
		}
	}

	real_stuff = stuff;
	real_stuff.tolower();

	if ( strstr( real_stuff.c_str(), ".wav" ) )
	{
		gi.soundindex( real_stuff.c_str() );
	}
	else if ( strstr( real_stuff.c_str(), ".mp3" ) )
	{
		gi.soundindex( real_stuff.c_str() );
	}
	else if ( strstr( real_stuff.c_str(), ".tik" ) )
	{
		int index;

		index = modelIndex( real_stuff.c_str() );

		if ( index > 0 )
		{
			 if ( !ent )
			 {
				 ent = new Entity;

				 ent->ProcessInitCommands( index, true );

				 delete ent;
				 ent = NULL;
			 }
			 else
			 {
				 ent->ProcessInitCommands( index, true );
			 }
		}
	}
	else if ( strstr( real_stuff.c_str(), ".spr" ) )
	{
		gi.modelindex( real_stuff.c_str() );
	}
	else if ( strstr( real_stuff.c_str(), ".ska" ) )
	{
		gi.TIKI_CacheAnim( real_stuff.c_str() );
	}
	else if ( strstr( real_stuff.c_str(), ".st" ) )
	{
		if ( strncmp( real_stuff.c_str(), "ai/", 3 ) == 0 )
		{
			CacheStatemap( stuff, ( Condition<Class> * )Actor::Conditions );
		}
	}
	else if ( strstr( real_stuff.c_str(), ".img" ) )
	{
		real_stuff.CapLength( real_stuff.length() - 4 );
		gi.imageindex( real_stuff.c_str() );
	}
	else if ( strstr( real_stuff.c_str(), ".itm" ) )
	{
		real_stuff.CapLength( real_stuff.length() - 4 );
		gi.itemindex( real_stuff.c_str() );
	}
	else if ( strstr( real_stuff.c_str(), ".arc" ) )
	{
		real_stuff.CapLength( real_stuff.length() - 4 );
		gi.archetypeindex( real_stuff.c_str() );
	}
}

void G_CacheStateMachineAnims( Entity *ent, const char *stateMachineName )
{
	Container<const char *> animNames;
	const char *animName;
	StateMap *stateMap;
	int i;

	// Cache the statemap

	if ( ent->isSubclassOf( Player ) )
		stateMap = GetStatemap( stateMachineName, ( Condition<Class> * )Player::Conditions, NULL, false, true );
	else if ( ent->isSubclassOf( Actor ) )
		stateMap = GetStatemap( stateMachineName, ( Condition<Class> * )Actor::Conditions, NULL, false, true );
	else
		return;

	// Get all of the anims in the statemachine

	stateMap->GetAllAnims( &animNames );

	// Loop through all of the anims in this statemachine and cache them all

	for ( i = 1 ; i <= animNames.NumObjects() ; i++ )
	{
		animName = animNames.ObjectAt( i );

		// This forces the caching of every anim that matches this anim name

		gi.Anim_Random( ent->edict->s.modelindex, animName );
	}
}

void ChangeMusic( const char *current, const char *fallback, qboolean force )
{
	int      j;
	gentity_t  *other;
	
	if ( current || fallback )
	{
		for( j = 0; j < game.maxclients; j++ )
		{
			other = &g_entities[ j ];
			if ( other->inuse && other->client )
			{
				Player *client;
				
				client = ( Player * )other->entity;
				client->ChangeMusic( current, fallback, force );
			}
		}
		if ( current && fallback )
		{
			gi.DPrintf( "music set to %s with fallback %s\n", current, fallback );
		}
	}
}

void ChangeMusicVolume( float volume, float fade_time )
{
	int      j;
	gentity_t  *other;
	
	for( j = 0; j < game.maxclients; j++ )
	{
		other = &g_entities[ j ];
		if ( other->inuse && other->client )
		{
			Player *client;
			
			client = ( Player * )other->entity;
			client->ChangeMusicVolume( volume, fade_time );
		}
	}
	gi.DPrintf( "music volume set to %.2f, fade time %.2f\n", volume, fade_time );
}

void RestoreMusicVolume( float fade_time )
{
	int      j;
	gentity_t  *other;
	
	for( j = 0; j < game.maxclients; j++ )
	{
		other = &g_entities[ j ];
		if ( other->inuse && other->client )
		{
			Player *client;
			
			client = ( Player * )other->entity;
			client->RestoreMusicVolume( fade_time );
		}
	}
}

//================================================================
// Name:			G_AllowMusicDucking
// Class:			
//
// Description:		Tells each player whether or not music ducking is allowed
//
// Parameters:		bool allowMusicDucking				- whether or not music ducking is allowed
//
// Returns:			none
//================================================================

void G_AllowMusicDucking( bool allowMusicDucking )
{
	int      j;
	gentity_t  *other;

	for( j = 0; j < game.maxclients; j++ )
	{
		other = &g_entities[ j ];

		if ( other->inuse && other->client )
		{
			Player *player;

			player = ( Player * )other->entity;
			player->allowMusicDucking( allowMusicDucking );
		}
	}
}

//================================================================
// Name:			G_AllowActionMusic
// Class:			
//
// Description:		Tells each player whether or not action music is allowed
//
// Parameters:		bool allowActionMusic				- whether or not action music is allowed
//
// Returns:			none
//================================================================

void G_AllowActionMusic( bool allowActionMusic )
{
	int      j;
	gentity_t  *other;

	for( j = 0; j < game.maxclients; j++ )
	{
		other = &g_entities[ j ];

		if ( other->inuse && other->client )
		{
			Player *player;

			player = ( Player * )other->entity;
			player->allowActionMusic( allowActionMusic );
		}
	}
}


void ChangeSoundtrack( const char * soundtrack )
{
	level.saved_soundtrack = level.current_soundtrack;
	level.current_soundtrack = soundtrack;
	
	gi.setConfigstring( CS_SOUNDTRACK, soundtrack );
	gi.DPrintf( "soundtrack switched to %s.\n", soundtrack );
}

void RestoreSoundtrack( void )
{
	if ( level.saved_soundtrack.length() )
	{
		level.current_soundtrack = level.saved_soundtrack;
		level.saved_soundtrack = "";
		gi.setConfigstring( CS_SOUNDTRACK, level.current_soundtrack.c_str() );
		gi.DPrintf( "soundtrack restored %s.\n", level.current_soundtrack.c_str() );
	}
}

void G_BroadcastSound( Entity *soundent, const Vector &origin, float radius, int soundType )
{
	Sentient *ent;
	Vector	delta;
	Event		*ev;
	str		name;
	float    r2;
	float    dist2;
	int		i;
	int		n;

/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment
	
#if 0
	int		count;
	  
	count = 0;
#endif
		
****************************************************************************/
	
	
	//Putting in the mechanisims for a SoundType within Broadcast sound
	//However it won't actually do anything yet.
	assert( soundent );
	if ( soundent && !( soundent->flags & FlagNotarget ) )
	{
		r2 = radius * radius;
		n = SentientList.NumObjects();
		for( i = 1; i <= n; i++ )
		{
			ent = SentientList.ObjectAt( i );
			if ( ( ent == soundent ) || ent->deadflag )
            {
				continue;
            }
			
			if ( ent->isSubclassOf( Actor ) )
			{
				delta = origin - ent->centroid;
				
				// dot product returns length squared
				dist2 = delta * delta;
				if (
					( dist2 <= r2 ) &&
					(
					( soundent->edict->areanum == ent->edict->areanum ) ||
					( gi.AreasConnected( soundent->edict->areanum, ent->edict->areanum ) )
					)
					)
					
				{
					ev = new Event( EV_HeardSound );
					ev->AddEntity( soundent );
					ev->AddVector( origin );
					ev->AddInteger( soundType );
					ent->PostEvent( ev, 0.0f );

/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment
					
#if 0
					count++;
#endif
					  
****************************************************************************/
					
				}
				
			}
		}
		
/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment
		
#if 0
		gi.DPrintf( "Broadcast event %s to %d entities\n", ev->getName(), count );
#endif
		  
****************************************************************************/
		
	}
}

void G_BroadcastAlert( Entity *soundent, const Vector &origin, const Vector &enemy,	float radius )
{
	Sentient *ent;
	Vector	delta;
	Event		*ev;
	str		name;
	float    r2;
	float    dist2;
	int		i;
	int		n;
/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#if 0
	int		count;

	count = 0;
#endif

****************************************************************************/

	
	//int	   soundType = SOUNDTYPE_ALERT;
	
	assert( soundent );
	if ( soundent && !( soundent->flags & FlagNotarget ) )
	{
		r2 = radius * radius;
		n = SentientList.NumObjects();
		for( i = 1; i <= n; i++ )
		{
			ent = SentientList.ObjectAt( i );
			if ( ( ent == soundent ) || ent->deadflag )
            {
				continue;
            }
			
			if ( ent->isSubclassOf( Actor ) )
			{
				delta = origin - ent->centroid;
				
				// dot product returns length squared
				dist2 = delta * delta;
				if (
					( dist2 <= r2 ) &&
					(
					( soundent->edict->areanum == ent->edict->areanum ) ||
					( gi.AreasConnected( soundent->edict->areanum, ent->edict->areanum ) )
					)
					)
					
				{
					ev = new Event( EV_HeardSound );
					ev->AddEntity( soundent );
					ev->AddVector( enemy );
					ent->PostEvent( ev, 0.0f );

/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#if 0
						count++;
#endif

****************************************************************************/

				}		
			}
		}

/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#if 0
      gi.DPrintf( "Broadcast event %s to %d entities\n", ev->getName(), count );
#endif

****************************************************************************/

	}
}

void CloneEntity( Entity * dest, const Entity * src )
{
	int     i, num;
	
	dest->setModel( src->model );
	// don't process our init commands
	//dest->CancelEventsOfType( EV_ProcessInitCommands );
	dest->setOrigin( src->origin );
	dest->setAngles( src->angles );
	dest->setScale( src->edict->s.scale );
	dest->setAlpha( src->edict->s.alpha );
	dest->health = src->health;
	// copy the surfaces
	memcpy( dest->edict->s.surfaces, src->edict->s.surfaces, sizeof( src->edict->s.surfaces ) );
	dest->edict->s.constantLight = src->edict->s.constantLight;
	//dest->edict->s.eFlags = src->edict->s.eFlags;
	dest->edict->s.renderfx = src->edict->s.renderfx;
	dest->edict->s.anim = src->edict->s.anim;
	dest->edict->s.frame = src->edict->s.frame;
	
	if ( src->bind_info )
	{
		num = src->bind_info->numchildren;
		for( i = 0; ( i < MAX_MODEL_CHILDREN ) && num; i++ )
		{
			Entity * clone;
			Entity * child;
			
			// duplicate the children
			if ( !src->bind_info->children[ i ] )
			{
				continue;
			}
			child = G_GetEntity( src->bind_info->children[ i ] );
			if ( child )
			{
				clone = new Entity( EntityCreateFlagAnimate );
				CloneEntity( clone, child );
				clone->attach( dest->entnum, child->edict->s.tag_num );
			}
			num--;
		}
	}
	dest->ProcessPendingEvents();
}

weaponhand_t WeaponHandNameToNum( const str &side )
{
	if ( !side.length() )
	{
		gi.DPrintf( "WeaponHandNameToNum : Weapon hand not specified\n" );
		return WEAPON_ERROR;
	}
	
	if ( !side.icmp( "righthand" ) || !side.icmp( "right" ) )
	{
		return WEAPON_RIGHT;
	}
	else if ( !side.icmp( "lefthand" ) || !side.icmp( "left" ) )
	{
		return WEAPON_LEFT;
	}
	else if ( !side.icmp( "dualhand" ) || !side.icmp( "dual" ) )
	{
		return WEAPON_DUAL;
	}
	else
	{
		return (weaponhand_t)atoi( side );
	}
}

const char *WeaponHandNumToName( weaponhand_t hand )
{
	switch( hand )
	{
	case WEAPON_RIGHT:
		return "righthand";
	case WEAPON_LEFT:
		return "lefthand";
	case WEAPON_DUAL:
		return "dualhand";
	case WEAPON_ANY:
		return "anyhand";
	default:
		return "Invalid Hand";
	}
}

firemode_t WeaponModeNameToNum( const str &mode )
{
	if ( !mode.length() )
	{
		gi.DPrintf( "WeaponModeNameToNum : Weapon mode not specified\n" );
		return FIRE_ERROR;
	}
	
	if ( !mode.icmp( "primary" ) )
	{
		return FIRE_MODE1;
	}
	
	if ( !mode.icmp( "alternate" ) )
	{
		return FIRE_MODE2;
	}
	
	if ( !mode.icmp( "MODE1" ) || !mode.icmp( "FIRE_MODE1" ) )
	{
		return FIRE_MODE1;
	}
	
	if ( !mode.icmp( "MODE2" ) || !mode.icmp( "FIRE_MODE2" ) )
	{ 
		return FIRE_MODE2;
	}
	
	if ( !mode.icmp( "MODE3" ) || !mode.icmp( "FIRE_MODE3" ) )
	{
		return FIRE_MODE3;
	}
	
	return (firemode_t)atoi( mode );
}

void G_DebugTargets( Entity *e, const str &from )
{
	gi.DPrintf( "DEBUGTARGETS:%s ", from.c_str() );
	
	if ( e->TargetName() && strlen( e->TargetName() ) )
	{
		gi.DPrintf( "Targetname=\"%s\"\n", e->TargetName() );
	}
	else
	{
		gi.DPrintf( "Targetname=\"None\"\n" );
	}
	
	if ( e->Target() && strlen( e->Target() ) )
	{
		gi.DPrintf( "Target=\"%s\"\n", e->Target() );
	}
	else
	{
		gi.DPrintf( "Target=\"None\"\n" );
	}
}

void G_DebugDamage( float damage, Entity *victim, Entity *attacker, Entity *inflictor )
{
	gi.DPrintf( "Victim:%s Attacker:%s Inflictor:%s Damage:%f\n", victim->getClassname(), attacker->getClassname(), inflictor->getClassname(), damage );
}

void G_FadeOut( float delaytime )
{
	// Make sure we are not already faded or fading out

	if ( ( level.m_fade_type == fadeout ) && ( level.m_fade_time_start > 0.0f ) )
	{
		float alpha;

		alpha = 1.0f - ( level.m_fade_time / level.m_fade_time_start );

		if ( alpha > 0.0f )
			return;
	}

	// Fade the screen out
	level.m_fade_color      = Vector( 0.0f, 0.0f, 0.0f );
	level.m_fade_alpha      = 1.0f;
	level.m_fade_time       = delaytime;
	level.m_fade_time_start = delaytime;
	level.m_fade_type       = fadeout;
	level.m_fade_style      = alphablend;
}

void G_AutoFadeIn( void )
{
	level.m_fade_time_start = 1;
	level.m_fade_time       = 1;
	level.m_fade_color[0]   = 0;
	level.m_fade_color[1]   = 0;
	level.m_fade_color[2]   = 0;
	level.m_fade_alpha      = 1;
	level.m_fade_type       = fadein;
	level.m_fade_style      = alphablend;
}

void G_ClearFade( void )
{
	level.m_fade_time = -1;
	level.m_fade_type = fadein;
}

void G_FadeSound( float delaytime )
{
	float time;
	
	// Fade the screen out
	time = delaytime * 1000.0f;
	gi.SendServerCommand( NULL, va( "fadesound %0.2f", time ) );
}

//
// restarts the game after delaytime
//
void G_RestartLevelWithDelay( float delaytime )
{
	int i;
	
	if ( multiplayerManager.inMultiplayer() )
		return;
	
	if ( level.died_already )
		return;
	
	level.died_already = true;
	
	// Restart the level soon
	for( i = 0; i < game.maxclients; i++ )
	   {
		if ( g_entities[ i ].inuse )
		{
			if ( g_entities[ i ].entity )
            {
				g_entities[ i ].entity->PostEvent( EV_Player_Respawn, delaytime );
            }
		}
	}	
}

void G_MissionFailed( const str& reason )
{
	str playerDeathThread;


	// Make sure we haven't already failed the mission

	if ( level.mission_failed )
		return;

	// Make the music system play the failure music for this level

	ChangeMusic( "failure", "normal", true );

	// Set our failure reason in the config string
	gi.failedcondition( reason.c_str() );		

	playerDeathThread = level.getPlayerDeathThread();

	if ( ( strnicmp( reason.c_str(), "PlayerKilled", strlen( "PlayerKilled" ) ) == 0 ) && ( playerDeathThread.length() ) )
	{
		ExecuteThread( playerDeathThread, true, NULL );
	}
	else
	{
		G_FinishMissionFailed();
	}
}

void G_FinishMissionFailed( void )
{
	Entity* entity;
	Player* player;

	// Get the player
	
	entity = g_entities[ 0 ].entity;

	if ( !entity->isSubclassOf( Player ) )
		return;

	player = (Player *)entity;

	// Fade everything out
	G_FadeOut( 1.0f );
	G_FadeSound( 1.0f );
	
	// Fail the mission

	player->setMissionFailed();
	
	level.mission_failed = true;
	level.playerfrozen = true;

	// Get out of letterbox mode if we are in it

	if ( level.m_letterbox_dir == letterbox_in )
	{
		level.m_letterbox_time_start = 0.5f;
		level.m_letterbox_dir = letterbox_out;
		
		level.m_letterbox_time = 0.5f;
		level.m_letterbox_fraction = 1.0f/8.0f;
	}
}

void G_StartCinematic( void )
{
	Entity *entity;

	level.cinematic = true;
	gi.cvar_set( "sv_cinematic", "1" );

	entity = g_entities[ 0 ].entity;

	if ( entity->isSubclassOf( Player ) )
	{
		Player *player = (Player *)entity;

		player->cinematicStarted();
	}
}

void G_StopCinematic( void )
{
	Entity *entity;

	// clear out the skip thread
	world->skipthread = "";
	level.cinematic = false;
	gi.cvar_set( "sv_cinematic", "0" );

	entity = g_entities[ 0 ].entity;

	if ( entity->isSubclassOf( Player ) )
	{
		Player *player = (Player *)entity;

		player->cinematicStopped();
	}
}

int G_NumClients( void )
{
	int i,count=0;
	for( i = 0; i < maxclients->integer; i++ )
	{
		gentity_t *ent = g_entities + i;
		if ( !ent->inuse || !ent->client || !ent->entity )
		{
			continue;
		}
		count++;
	}
	return count;
}

//----------------------------------------------------------------
// Name:			G_DrawXYBox
// Class:			None
//
// Description:		draws a box in the xy plane 
//
// Parameters:		
//					Vector center	-- the point about which the box is centered
//					float width		-- the width of the box
//					float r, g, b	-- the color values for the box
//					float alpha		-- the amount of alpha applied to the box
//
// Returns:			None
//----------------------------------------------------------------
void G_DrawXYBox(const Vector &center, float width, float r, float g, float b, float alpha )
{
	float squareSize=width/2.0f;
	Vector topLeftPoint		= center + Vector(-squareSize, -squareSize, 0);
	Vector topRightPoint	= center + Vector(+squareSize, -squareSize, 0);
	Vector bottomRightPoint	= center + Vector(+squareSize, +squareSize, 0);
	Vector bottomLeftPoint	= center + Vector(-squareSize, +squareSize, 0);
	G_DebugLine( topLeftPoint,		topRightPoint, r, g, b, alpha );
	G_DebugLine( topRightPoint,		bottomRightPoint, r, g, b, alpha );
	G_DebugLine( bottomRightPoint,	bottomLeftPoint, r, g, b, alpha );
	G_DebugLine( bottomLeftPoint,	topLeftPoint, r, g, b, alpha );
}

float G_GetDatabaseFloat( const str &prefix, const str &objectName, const str &varName )
{
	GameplayManager *gpm;
	str gameplayObjectName;

	// Get the gameplay manager

	gpm = GameplayManager::getTheGameplayManager();

	if ( !gpm )
		return 0.0f;

	// Build the object name

	gameplayObjectName = prefix + objectName;

	return gpm->getFloatValue( gameplayObjectName, varName );
}

str G_GetDatabaseString( const str &prefix, const str &objectName, const str &varName )
{
	GameplayManager *gpm;
	str gameplayObjectName;

	// Get the gameplay manager

	gpm = GameplayManager::getTheGameplayManager();

	if ( !gpm )
		return 0.0f;

	// Build the object name

	gameplayObjectName = prefix + objectName;

	return gpm->getStringValue( gameplayObjectName, varName );
}
