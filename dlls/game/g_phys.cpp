//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/g_phys.cpp                                $
// $Revision:: 32                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
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
#include "animate.h"
#include "sentient.h"
#include "actor.h"
#include "vehicle.h"
#include "weaputils.h"
#include "mp_manager.hpp"

/*


pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.

onground is set for toss objects when they come to a complete rest.  it is set for steping or walking objects

doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
corpses are SOLID_NOT and MOVETYPE_TOSS
crates are SOLID_BBOX and MOVETYPE_TOSS
walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY

solid_edge items only clip against bsp models.

*/

typedef struct
{
	Entity		*ent;
	Vector		localorigin;
	Vector		origin;
	Vector		localangles;
	Vector		angles;
	float		deltayaw;
} pushed_t;

pushed_t pushed[ MAX_GENTITIES ];
pushed_t *pushed_p;

Entity *obstacle;

/*
============
G_FixEntityPosition

============
*/
Entity *G_FixEntityPosition( Entity *ent )
{
	int mask;
	trace_t trace;
	Vector start;
	
	mask = ent->edict->clipmask;
	if ( !mask )
	{
		mask = MASK_SOLID;
	}
	
	start = ent->origin;
	start.z += 8.0f;
	if ( ent->client )
	{
		trace = G_Trace( start, ent->mins, ent->maxs, ent->origin, ent, mask, true, "G_TestEntityPosition1" );
	}
	else
	{
		trace = G_Trace( start, ent->mins, ent->maxs, ent->origin, ent, mask, false, "G_TestEntityPosition2" );
	}
	
	if ( trace.startsolid )
	{
		//return g_entities->entity;
		assert( trace.ent );
		assert( trace.ent->entity );
		return trace.ent->entity;
	}
	
	ent->setOrigin( trace.endpos );
	
	return NULL;
}

/*
============
G_TestEntityPosition

============
*/
Entity *G_TestEntityPosition( Entity *ent )
{
	int mask;
	trace_t trace;
	
	mask = ent->edict->clipmask;
	if ( !mask )
		mask = MASK_SOLID;
	
	if ( ent->client )
	{
		trace = G_Trace( ent->origin, ent->mins, ent->maxs, ent->origin, ent, mask, true, "G_TestEntityPosition1" );
	}
	else
	{
		trace = G_Trace( ent->origin, ent->mins, ent->maxs, ent->origin, ent, mask, false, "G_TestEntityPosition2" );
	}
	
	if ( trace.startsolid )
	{
		//return g_entities->entity;
		assert( trace.ent );
		assert( trace.ent->entity );
		return trace.ent->entity;
	}
	
	return NULL;
}

/*
================
G_CheckVelocity
================
*/
void G_CheckVelocity( Entity *ent )
{
	int i;
	
	//
	// bound velocity
	//
	for( i = 0; i < 3; i++ )
	{
		if ( ent->velocity[ i ] > sv_maxvelocity->value )
		{
			ent->velocity[ i ] = sv_maxvelocity->value;
		}
		else if ( ent->velocity[ i ] < -sv_maxvelocity->value )
		{
			ent->velocity[ i ] = -sv_maxvelocity->value;
		}
	}
}

/*
==================
G_Impact

Two entities have touched, so run their touch functions
==================
*/
void G_Impact( Entity *e1, const trace_t *trace )
{
	gentity_t   *e2;
	Event		   *ev;
	
	e2 = trace->ent;
	
	level.impact_trace = *trace;
	
	// touch anything, including the world
	if ( e1->edict->solid != SOLID_NOT )
	{
		ev = new Event( EV_Touch );
		ev->AddEntity( e2->entity );
		e1->ProcessEvent( ev );
	}
	
	// entity could have been removed, so check if he's in use before sending the event
	if (
		e2->entity &&
		( e2->solid != SOLID_NOT ) &&
		( !( e2->contents & CONTENTS_SHOOTABLE_ONLY ) ) &&
		( e2->entity != world )
		)
	{
		ev = new Event( EV_Touch );
		ev->AddEntity( e1 );
		e2->entity->ProcessEvent( ev );
	}
	
	memset( &level.impact_trace, 0, sizeof( level.impact_trace ) );
}

/*
==================
G_ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define STOP_EPSILON 0.1

int G_ClipVelocity( const Vector& in, const Vector& normal, Vector& out, float overbounce )
{
	int	i;
	int	blocked;
	float	backoff;
	
	blocked = 0;
	
	if ( normal[ 2 ] > 0.0f )
	{
		// floor
		blocked |= 1;
	}
	if ( !normal[ 2 ] )
	{
		// step
		blocked |= 2;
	}
	
	backoff = ( in * normal ) * overbounce;
	
	out = in - ( normal * backoff );
	
	for( i = 0; i < 3; i++ )
	{
		if ( ( out[ i ] > -STOP_EPSILON ) && ( out[ i ] < STOP_EPSILON ) )
		{
			out[ i ] = 0.0f;
		}
	}
	
	return blocked;
}

/*
============
G_FlyMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
1 = floor
2 = wall / step
4 = dead stop
============
*/
#define MAX_CLIP_PLANES 5

int G_FlyMove( Entity *ent, const Vector &basevel, float time, int mask )
{
	Entity	*hit;
	gentity_t  *edict;
	int		bumpcount, numbumps;
	Vector	dir;
	float		d;
	int		numplanes;
	vec3_t	planes[ MAX_CLIP_PLANES ];
	Vector	primal_velocity, original_velocity, new_velocity;
	int		i, j;
	trace_t	trace;
	Vector	end;
	float		time_left;
	int		blocked;
/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#if 0
	Vector   move;
	Vector	v;
#endif

****************************************************************************/


	edict = ent->edict;

	numbumps = 4;

	blocked = 0;
	original_velocity = ent->velocity;
	primal_velocity = ent->velocity;
	numplanes = 0;

/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#if 1

****************************************************************************/

	time_left = time;
/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#else
	time_left = 1.0;//time;

	v = ent->total_delta;
	v[ 1 ] = -v[ 1 ];  // sigh...
	MatrixTransformVector( v, ent->orientation, move );
	move += ent->velocity * time;
	ent->total_delta = vec_zero;
#endif

****************************************************************************/


	ent->groundentity = NULL;
	for( bumpcount = 0; bumpcount < numbumps; bumpcount++ )
		{
/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#if 1

****************************************************************************/

		end = ent->origin + ( time_left * ( ent->velocity + basevel ) );
/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#else
      end = ent->origin + time_left * move;
#endif

****************************************************************************/


		Vector test;
		test = ent->origin;
		test[2] += 25.0f;
			
		trace = G_Trace( ent->origin, ent->mins, ent->maxs, end, ent, mask, false, "G_FlyMove" );
		//trace = G_Trace( test, ent->mins, ent->maxs, end, ent, mask |CONTENTS_MONSTERCLIP|CONTENTS_PLAYERCLIP , false, "G_FlyMove" );
		/*
		if (
			( trace.allsolid ) ||
			(
			( trace.startsolid ) &&
			( ent->movetype == MOVETYPE_VEHICLE )
			)
		   )
		{
			// entity is trapped in another solid
			ent->velocity = vec_zero;
			return 3;
		}
		*/
		if ( trace.fraction > 0.0f )
		{
			// actually covered some distance
			ent->setOrigin( trace.endpos );
			original_velocity = ent->velocity;
			numplanes = 0;
		}
		
		if ( trace.fraction == 1.0f )
		{
			// moved the entire distance
			break;
		}
		
		
		assert( trace.ent );
		hit = trace.ent->entity;
		
		if ( trace.plane.normal[ 2 ] > 0.7f )
		{
			// floor
			blocked |= 1;
			if ( hit->getSolidType() == SOLID_BSP )
			{
				ent->groundentity = hit->edict;
				ent->groundplane = trace.plane;
				ent->groundcontents = trace.contents;
			}
		}
		
		if ( !trace.plane.normal[ 2 ] )
		{
			// step
			blocked |= 2;
		}
		
		//
		// run the impact function
		//
		G_Impact( ent, &trace );
		if ( !edict->inuse )
		{
			break;		// removed by the impact function
		}
		
		time_left -= time_left * trace.fraction;
		
		// cliped to another plane
		if ( numplanes >= MAX_CLIP_PLANES )
		{
			// this shouldn't really happen
			ent->velocity = vec_zero;
			return 3;
		}
		
		VectorCopy( trace.plane.normal, planes[ numplanes ] );
		numplanes++;
		
		//
		// modify original_velocity so it parallels all of the clip planes
		//
		for( i = 0; i < numplanes; i++ )
		{
			G_ClipVelocity( original_velocity, Vector( planes[ i ] ), new_velocity, 1.01f );
			for( j = 0; j < numplanes; j++ )
			{
				if ( j != i )
				{
					if ( ( new_velocity * planes[ j ] ) < 0.0f )
					{
						// not ok
						break;
					}
				}
			}
			
			if ( j == numplanes )
			{
				break;
			}
		}
		
		if ( i != numplanes )
		{
			// go along this plane
			ent->velocity = new_velocity;
		}
		else
		{
			// go along the crease
			if ( numplanes != 2 )
			{
				ent->velocity = vec_zero;
				return 7;
			}
			CrossProduct( planes[ 0 ], planes[ 1 ], dir );
			d = dir * ent->velocity;
			ent->velocity = dir * d;
		}
		
		//
		// if original velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if ( ( ent->velocity * primal_velocity ) <= 0.0f )
		{
			ent->velocity = vec_zero;
			return blocked;
		}
	}
		
	return blocked;
}


/*
============
G_AddGravity

============
*/
void G_AddGravity( Entity *ent )
{
	float grav;
	
	if ( ent->waterlevel > 2 )
	{
		grav = ent->gravity * 60.0f * level.frametime;
	}
	else
	{
		grav = ent->gravity * sv_currentGravity->value * level.frametime;
	}
	
	if ( ent->isSubclassOf( Vehicle ) )
		ent->velocity[ 2 ] -= grav + 500.0f;
	else
		ent->velocity[ 2 ] -= grav;	
}

/*
===============================================================================

PUSHMOVE

===============================================================================
*/

/*
============
G_PushEntity

Does not change the entities velocity at all
============
*/
trace_t G_PushEntity( Entity *ent, const Vector &push )
{
	trace_t	trace;
	Vector	start;
	Vector	end;
	int		mask;
	gentity_t  *edict;
	
	start = ent->origin;
	end = start + push;
	
retry:
	if ( ent->edict->clipmask )
	{
		mask = ent->edict->clipmask;
	}
	else
	{
		mask = MASK_SOLID;
	}
	
	if ( ent->usesFullTrace() && ( !multiplayerManager.inMultiplayer() || multiplayerManager.fullCollision() ) )
		trace = G_FullTrace( start, ent->mins, ent->maxs, end, ent, mask, false, "G_PushEntity" );
	else
		trace = G_Trace( start, ent->mins, ent->maxs, end, ent, mask, false, "G_PushEntity" );
	
	edict = ent->edict;
	
	ent->setOrigin( trace.endpos );
	
	if ( trace.fraction != 1.0f || ( trace.startsolid && trace.ent && trace.ent->entity != world ) )
	{
		G_Impact( ent, &trace );
		
		// if the pushed entity went away and the pusher is still there
		if ( ( !trace.ent || !trace.ent->inuse ) && edict->inuse )
		{
			// move the pusher back and try again
			ent->setOrigin( start );
			goto retry;
		}
	}
	
	if ( edict && ( edict != ent->edict ) )
	{
		if ( ent->flags & FL_TOUCH_TRIGGERS )
		{
			G_TouchTriggers( ent );
		}
	}
	
	return trace;
}

/*
============
G_SlideEntity
============
*/
trace_t G_SlideEntity( Entity *ent, const Vector &push )
{
	trace_t	trace;
	Vector	start;
	Vector	end;
	int		mask;
	
	start = ent->origin;
	end = start + push;
	
	if ( ent->edict->clipmask )
	{
		mask = ent->edict->clipmask;
	}
	else
	{
		mask = MASK_SOLID;
	}
	
	trace = G_Trace( start, ent->mins, ent->maxs, end, ent, mask, false, "G_SlideEntity" );
	
	ent->setOrigin( trace.endpos );
	
	return trace;
}


/*
================
G_SnapPosition

================
*/
/*
qboolean G_SnapPosition
   (
   Entity *ent
   )

   {
   int    x, y, z;
   Vector offset( 0, -1, 1 );
   Vector base;

   base = ent->origin;
   for ( z = 0; z < 3; z++ )
      {
      ent->origin.z = base.z + offset[ z ];
      for ( y = 0; y < 3; y++ )
         {
         ent->origin.y = base.y + offset[ y ];
         for ( x = 0; x < 3; x++ )
            {
            ent->origin.x = base.x + offset[ x ];
            if ( G_TestEntityPosition( ent ) )
               {
               ent->origin.x += offset[ x ];
               ent->origin.y += offset[ y ];
               ent->origin.z += offset[ z ];
               ent->setOrigin( ent->origin );
               return true;
               }
            }
         }
      }

   // can't find a good position, so put him back.
   ent->origin = base;

   return false;
   }
*/

/*
============
G_Push

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
============
*/
qboolean G_Push( Entity *pusher, const Vector &pushermove, const Vector &pusheramove )
{
	Entity		*check, *block;
	gentity_t   *edict;
	Vector		move, amove;
	Vector		mins, maxs;
	Vector		save;
	pushed_t		*p;
	Vector		org, org2, move2;
	Vector	   norm;
	float			mat[ 3 ][ 3 ];
	pushed_t		*pusher_p;
	float       radius;
	int         i, num;
	int         touch[ MAX_GENTITIES ];
	gentity_t   *next;
	
	// save the pusher's original position
	pusher_p = pushed_p;
	pushed_p->ent				= pusher;
	pushed_p->localorigin = pusher->GetLocalOrigin();
	pushed_p->origin        = pusher->origin;
	pushed_p->localangles	= pusher->localangles;
	pushed_p->angles        = pusher->angles;
	
	if ( pusher->client )
	{
		pushed_p->deltayaw = pusher->client->ps.delta_angles[ YAW ];
	}
	
	pushed_p++;
	
	if ( pushed_p >= &pushed[ MAX_GENTITIES ] )
	{
		gi.Error( ERR_FATAL, "Pushed too many entities." );
	}
	
	// move the pusher to it's final position
	pusher->addAngles( pusheramove );
	pusher->addOrigin( pushermove );
	
	if ( pusher->edict->solid == SOLID_NOT )
	{
		// Doesn't push anything
		return true;
	}
	
	// change the move to worldspace
	move  = pusher->origin - pusher_p->origin;
	amove = pusher->angles - pusher_p->angles;
	
	// we need this for pushing things later
	AnglesToAxis( amove, mat );
	
	// find the bounding box
	mins = pusher->absmin;
	maxs = pusher->absmax;
	
	// Add in entities that are within the pusher
	
	num = gi.AreaEntities( mins, maxs, touch, MAX_GENTITIES, qfalse );
	
	// Add in entities that are standing on the pusher
	
	for( edict = active_edicts.next; edict != &active_edicts; edict = next )
	{
		assert( edict );
		assert( edict->inuse );
		assert( edict->entity );
		
		next = edict->next;
		check = edict->entity;
		
		if ( check->groundentity == pusher->edict )
		{
			// Make sure not in list already
			
			for( i = 0; i < num; i++ )
			{
				if ( touch [ i ] == check->edict - g_entities )
					break;
			}
			
			if ( i == num )
			{
				touch[ num ] = check->edict - g_entities;
				num++;
			}
		}
	}
	
	for( i = 0; i < num; i++ )
	{
		edict = &g_entities[ touch[ i ] ];
		assert( edict );
		assert( edict->inuse );
		assert( edict->entity );
		check = edict->entity;
		
		if ( ( check->movetype == MOVETYPE_PUSH ) ||
			( check->movetype == MOVETYPE_STOP ) ||
			( check->movetype == MOVETYPE_NONE ) ||
			( check->movetype == MOVETYPE_NOCLIP ) )
		{
			continue;
		}
		
		if ( ( check->edict->contents == CONTENTS_SHOOTABLE_ONLY ) || ( check->edict->contents == CONTENTS_CORPSE ) )
			continue;
		
		// if the entity is standing on the pusher, it will definitely be moved
		if ( check->groundentity != pusher->edict )
		{
			// Only move triggers and non-solid objects if they're sitting on a moving object
			if ( ( check->edict->solid == SOLID_TRIGGER ) || ( check->edict->solid == SOLID_NOT ) )
			{
				continue;
			}
			
			// see if the ent needs to be tested
			if ( ( check->absmin[ 0 ] >= maxs[ 0 ] ) ||
				( check->absmin[ 1 ] >= maxs[ 1 ] ) ||
				( check->absmin[ 2 ] >= maxs[ 2 ] ) ||
				( check->absmax[ 0 ] <= mins[ 0 ] ) ||
				( check->absmax[ 1 ] <= mins[ 1 ] ) ||
				( check->absmax[ 2 ] <= mins[ 2 ] ) )
			{
				continue;
			}
			
			// see if the ent's bbox is inside the pusher's final position
			if ( !G_TestEntityPosition( check ) )
			{
				continue;
			}
		}
		
		if (
            ( pusher->movetype == MOVETYPE_PUSH ) ||
            ( check->groundentity == pusher->edict )
			)
		{
			pushed_p->localorigin = check->GetLocalOrigin();
			pushed_p->localangles = check->localangles;
			// move this entity
			pushed_p->ent				= check;
			pushed_p->origin        = check->origin;
			pushed_p->angles        = check->angles;
			pushed_p++;
			
			if ( pushed_p >= &pushed[ MAX_GENTITIES ] )
			{
				gi.Error( ERR_FATAL, "Pushed too many entities." );
			}
			
			// save off the origin
			save = check->GetLocalOrigin();
			
			// try moving the contacted entity
			move2 = move;
			
			// FIXME: doesn't rotate monsters?
			if ( check->client )
			{
				check->client->ps.delta_angles[YAW] += ANGLE2SHORT( amove[ YAW ] );
			}
			
			// get the radius of the entity
			if ( check->size.x > check->size.z )
            {
				radius = check->size.z * 0.5f;
            }
			else
            {
				radius = check->size.x * 0.5f;
            }
			
			// figure movement due to the pusher's amove
			org = check->origin - pusher->origin;
			org.z += radius;
			
			MatrixTransformVector( org, mat, org2 );
			
			move2 += org2 - org;
			
			//FIXME
			// We should probably do a flymove here so that we slide against other objects
			check->addOrigin( check->getParentVector( move2 ) );
			
			// may have pushed them off an edge
			if ( check->groundentity != pusher->edict )
			{
				check->groundentity = NULL;
			}
			
			block = G_TestEntityPosition( check );
			if ( block )
            {
				block = G_FixEntityPosition( check );
            }
			if ( !block )
			{
				// pushed ok
				check->link();
				
				// impact?
				continue;
			}
			
			// try to snap it to a good position
			/*
			if ( G_SnapPosition( check ) )
            {
	            // snapped ok.  we don't have to link since G_SnapPosition does it for us.
		        continue;
            }
			*/
			
			// if it is ok to leave in the old position, do it
			// this is only relevent for riding entities, not pushed
			// FIXME: this doesn't acount for rotation
			check->setOrigin( save );
			block = G_TestEntityPosition( check );
			if ( !block )
			{
				pushed_p--;
				continue;
			}
		}
		
		if ( check->edict->solid == SOLID_NOT )
			continue;
		
		// save off the obstacle so we can call the block function
		obstacle = check;
		
		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for( p = pushed_p - 1; p >= pushed; p-- )
		{
			p->ent->angles       = p->angles;
			p->ent->origin       = p->origin;
			
			p->ent->localangles = p->localangles;
			p->ent->SetLocalOrigin( p->localorigin );
			
			if ( p->ent->client )
			{
				p->ent->client->ps.delta_angles[ YAW ] = (int) p->deltayaw;
			}
		}
		
		// Only "really" move it in order so that the bound coordinate system is correct
		for( p = pushed; p < pushed_p; p++ )
		{
			p->ent->setAngles();
			p->ent->setOrigin();
		}
		
		return false;
	}
		
	//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
	for( p = pushed_p - 1; p >= pushed; p-- )
	{
		if ( p->ent->flags & FL_TOUCH_TRIGGERS )
		{
			G_TouchTriggers( p->ent );
		}
	}
	
	return true;
}

/*
================
G_PushMove
================
*/
qboolean G_PushMove( Entity *ent, const Vector &move, const Vector &amove )
{
	Entity *part;
	Vector m, a;
	Event	 *ev;
	
	m = move;
	a = amove;
	
	pushed_p = pushed;
	
	part = ent;
	
	while( part )
	{
		if ( !G_Push( part, m, a ) )
		{
			// move was blocked
			// call the pusher's "blocked" function
			// otherwise, just stay in place until the obstacle is gone
			ev = new Event( EV_Blocked );
			ev->AddEntity( obstacle );
			part->ProcessEvent( ev );
			return false;
		}
		
		m = vec_zero;
		a = vec_zero;
		
		if ( part->bind_info )
			part = part->bind_info->teamchain;
		else
			part = NULL;
	}
	
	return true;
}

/*
================
G_Physics_Pusher

Bmodel objects don't interact with each other, but
push all box objects
================
*/
void G_Physics_Pusher( Entity *ent )
{
	Vector	move, amove;
	Entity   *part, *mv;
	Event		*ev;
	
	// team slaves are only moved by their captains
	if ( ent->flags & FL_TEAMSLAVE )
	{
		return;
	}
	
	// Check if anyone on the team is moving
	part = ent;
	
	while( part )
	{
		if ( ( part->velocity != vec_zero ) || ( part->avelocity != vec_zero ) )
		{
			break;
		}
		
		if ( part->bind_info )
			part = part->bind_info->teamchain;
		else
			part = NULL;
	}
	
	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	pushed_p = pushed;
	while( part )
	{
		move  = part->velocity  * level.frametime;
		amove = part->avelocity * level.frametime;
		
		if ( !G_Push( part, move, amove ) )
		{
			// move was blocked
			break;
		}
		
		if ( part->bind_info )
			part = part->bind_info->teamchain;
		else
			part = NULL;
	}
	
	if ( part )
	{
		// the move failed, bump all movedone times
		mv = ent;
		
		while( mv )
		{
			mv->PostponeEvent( EV_MoveDone, FRAMETIME );
			
			if ( mv->bind_info )
				mv = mv->bind_info->teamchain;
			else
				mv = NULL;
		}
		
		// if the pusher has a "blocked" function, call it
		// otherwise, just stay in place until the obstacle is gone
		ev = new Event( EV_Blocked );
		ev->AddEntity( obstacle );
		part->ProcessEvent( ev );
	}
}

//==================================================================

/*
=============
G_Physics_Noclip

A moving object that doesn't obey physics
=============
*/
void G_Physics_Noclip( Entity *ent )
{
	ent->angles += ent->avelocity * level.frametime;
	ent->origin += ent->velocity * level.frametime;
	ent->link();
}

/*
==============================================================================

TOSS / BOUNCE

==============================================================================
*/

/*
=============
G_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
void G_Physics_Toss( Entity *ent )
{
	trace_t	trace;
	Vector	move;
	float		backoff;
	Entity	*slave;
	qboolean	wasinwater;
	qboolean	isinwater;
	Vector   origin2;
	Vector	basevel;
	gentity_t  *edict;
	qboolean onconveyor;
	
	// if not a team captain, so movement will be handled elsewhere
	if ( ent->flags & FL_TEAMSLAVE )
	{
		return;
	}
	
	if ( ent->velocity[ 2 ] > 0 )
	{
		ent->groundentity = NULL;
	}
	
	// check for the groundentity going away
	if ( ent->groundentity && !ent->groundentity->inuse )
	{
		ent->groundentity = NULL;
	}
	
	onconveyor = ( basevel != vec_zero );
	
	// if onground, return without moving
	if ( ent->groundentity && !onconveyor && ( ent->movetype != MOVETYPE_VEHICLE ) )
	{
		if ( ent->avelocity.length() )
		{
			// move angles
			ent->setAngles( ent->angles + ( ent->avelocity * level.frametime ) );
		}
		ent->velocity = vec_zero;
		return;
	}
	
	origin2 = ent->origin;
	
	//G_CheckVelocity( ent );
	
	// add gravity
	if ( !onconveyor && ( ent->movetype != MOVETYPE_FLY ) && ( ent->movetype != MOVETYPE_FLYMISSILE ) )
	{
		if ( !( ent->flags & FL_FLY ) ) 
			G_AddGravity( ent );			
	}
	
	// move angles
	ent->setAngles( ent->angles + ( ent->avelocity * level.frametime ) );
	
	// move origin
	move = ( ent->velocity + basevel ) * level.frametime;
	
	edict = ent->edict;
	if ( ent->movetype == MOVETYPE_VEHICLE )
	{
		int mask;
		
		if ( ent->edict->clipmask )
		{
			mask = ent->edict->clipmask;
		}
		else
		{
			mask = MASK_MONSTERSOLID;
		}
		G_FlyMove( ent, basevel, FRAMETIME, mask );
		if ( ent->flags & FL_TOUCH_TRIGGERS )
		{
			G_TouchTriggers( ent );
		}
		return;
	}
	else
	{
		trace = G_PushEntity( ent, move );
	}
	
	if ( ( trace.fraction == 0.0f ) && ( ent->movetype == MOVETYPE_SLIDE ) )
	{
		// Check for slide by removing the downward velocity
		Vector slide;
		
		slide[ 0 ] = move[ 0 ] * 0.7f;
		slide[ 1 ] = move[ 1 ] * 0.7f;
		slide[ 2 ] = 0;
		
		G_PushEntity( ent, slide );
	}
	
	if ( !edict->inuse )
	{
		return;
	}
	
	if ( trace.fraction < 1.0f )
	{
		if ( ( ent->movetype == MOVETYPE_BOUNCE ) || ( ent->movetype == MOVETYPE_GIB ) )
		{
			backoff = 1.5;
		}
		else
		{
			backoff = 1;
		}
		
		if( ent->movetype != MOVETYPE_NONE)
			G_ClipVelocity( ent->velocity, Vector( trace.plane.normal ), ent->velocity, backoff );
		
		// stop if on ground
		if ( trace.plane.normal[ 2 ] > 0.7f )
		{
			//if ( ( ( ent->velocity[ 2 ] < 30.0f ) || ( ( ent->movetype != MOVETYPE_BOUNCE ) && ( ent->movetype != MOVETYPE_GIB ) ) ) &&
			if ( ( ( VectorLength( ent->velocity ) < 10.0f ) || ( ( ent->movetype != MOVETYPE_BOUNCE ) && ( ent->movetype != MOVETYPE_GIB ) ) ) &&
				( ent->movetype != MOVETYPE_SLIDE ) )
            {
				ent->groundentity = trace.ent;
				ent->groundplane = trace.plane;
				ent->groundcontents = trace.contents;
				ent->velocity = vec_zero;
				ent->avelocity = vec_zero;
				ent->ProcessEvent( EV_Stop );
			}
			else if ( ent->movetype == MOVETYPE_GIB )
			{
				// Stop spinning after we bounce on the ground
				ent->avelocity = vec_zero;
			}
		}
	}
	
	if ( ( move[ 2 ] == 0.0f ) && onconveyor )
	{
		// Check if we still have a ground
		ent->CheckGround();
	}
	
	// check for water transition
	wasinwater = ( ent->watertype & MASK_WATER );
	ent->watertype = gi.pointcontents( ent->origin, 0 );
	isinwater = ent->watertype & MASK_WATER;
	
	if ( isinwater )
	{
		ent->waterlevel = 1;
	}
	else
	{
		ent->waterlevel = 0;
	}
	
	if ( ( edict->spawntime < ( level.time - FRAMETIME ) ) && ( ent->mass > 0 ) )
	{
		if ( !wasinwater && isinwater )
		{
			ent->Sound( "impact_watersplash", CHAN_BODY, DEFAULT_VOL, DEFAULT_MIN_DIST, &origin2 );
		}
		else if ( wasinwater && !isinwater )
		{
			ent->Sound( "impact_leavewater", CHAN_BODY, DEFAULT_VOL, DEFAULT_MIN_DIST, &origin2 );
		}
	}
	
	// GAMEFIX - Is this necessary?
	// move teamslaves
	if ( ent->bind_info )
	{
		for( slave = ent->bind_info->teamchain ; slave ; slave = slave->bind_info->teamchain )
		{
			slave->setOrigin( slave->GetLocalOrigin() );
			slave->setAngles( slave->localangles );
		}
	}
	
	if ( ent->flags & FL_TOUCH_TRIGGERS )
	{
		G_TouchTriggers( ent );
	}
}

/*
===============================================================================

STEPPING MOVEMENT

===============================================================================
*/

void G_AddRotationalFriction( Entity *ent )
{
	int	n;
	float	adjustment;
	
	ent->angles += level.frametime * ent->avelocity;
	adjustment = level.frametime * sv_stopspeed->value * sv_friction->value;
	for( n = 0; n < 3; n++ )
	{
		if ( ent->avelocity[ n ] > 0.0f )
		{
			ent->avelocity[ n ] -= adjustment;
			if ( ent->avelocity[ n ] < 0.0f )
			{
				ent->avelocity[ n ] = 0.0f;
			}
		}
		else
		{
			ent->avelocity[ n ] += adjustment;
			if ( ent->avelocity[ n ] > 0.0f )
			{
				ent->avelocity[ n ] = 0.0f;
			}
		}
	}
}

/*
=============
G_CheckWater

=============
*/

void G_CheckWater( Entity *ent )
{
	if ( ent->isSubclassOf( Actor ) )
	{
		( ( Actor * )ent )->movementSubsystem->CheckWater();
	}
	else
	{
		ent->watertype = gi.pointcontents( ent->origin, 0 );
		if ( ent->watertype & MASK_WATER )
		{
			ent->waterlevel = 1;
		}
		else
		{
			ent->waterlevel = 0;
		}
	}
}

/*
=============
G_Physics_Step

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
FIXME: is this true?
=============
*/

void G_Physics_Step( Entity *ent )
{
	qboolean	wasonground;
	qboolean	hitsound = false;
	Vector	vel;
	float		speed, newspeed, control;
	float		friction;
	int		mask;
	Vector	basevel;
	
	// airborn monsters should always check for ground
	if ( !ent->groundentity )
	{
		ent->CheckGround();
	}
	
	if ( ent->groundentity )
	{
		wasonground = true;
		// Add this change in ?
		ent->velocity[ 2 ] = 0.0;
	}
	else
	{
		wasonground = false;
	}
	
	G_CheckVelocity( ent );
	
	if ( ent->avelocity != vec_zero )
	{
		G_AddRotationalFriction( ent );
	}
	
	// add gravity except:
	//   flying monsters
	//   swimming monsters who are in the water
	if ( !wasonground )
	{
		if ( !( ent->flags & FL_FLY ) )
		{
			if ( !( ( ent->flags & FL_SWIM ) && ( ent->waterlevel > 2 ) ) )
			{
				if ( ent->velocity[ 2 ] < ( sv_currentGravity->value * ent->gravity * -0.1f ) )
				{
					hitsound = true;
				}
				
				// Testing water gravity.  If this doesn't work, just restore the uncommented lines
				//if ( ent->waterlevel == 0 )
				//{
				G_AddGravity( ent );
				//}
			}
		}
	}
	
	// friction for flying monsters that have been given vertical velocity
	if ( ( ent->flags & FL_FLY ) && ( ent->velocity.z != 0.0f ) )
	{
		speed = fabs( ent->velocity.z );
		control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
		friction = sv_friction->value / 3.0f;
		newspeed = speed - ( level.frametime * control * friction );
		if ( newspeed < 0.0f )
		{
			newspeed = 0.0f;
		}
		newspeed /= speed;
		ent->velocity.z *= newspeed;
	}
	
	// friction for flying monsters that have been given vertical velocity
	if ( ( ent->flags & FL_SWIM ) && ( ent->velocity.z != 0.0f ) )
	{
		speed = fabs( ent->velocity.z );
		control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
		newspeed = speed - ( level.frametime * control * sv_waterfriction->value * ent->waterlevel );
		if ( newspeed < 0.0f )
		{
			newspeed = 0.0f;
		}
		newspeed /= speed;
		ent->velocity.z *= newspeed;
	}
	
	if ( ent->velocity != vec_zero )
	{
		// apply friction
		// let dead monsters who aren't completely onground slide
		if ( ( wasonground ) || ( ent->flags & ( FL_SWIM | FL_FLY ) ) )
		{
			if ( !( ( ent->health <= 0.0f ) && !M_CheckBottom( ent ) ) )
			{
				vel = ent->velocity;
				vel.z = 0.0f;
				speed = vel.length();
				if ( speed )
				{
					friction = sv_friction->value;
					
					control = speed < sv_stopspeed->value ? sv_stopspeed->value : speed;
					newspeed = speed - ( level.frametime * control * friction );
					
					if ( newspeed < 0.0f )
					{
						newspeed = 0.0f;
					}
					
					newspeed /= speed;
					
					ent->velocity.x *= newspeed;
					ent->velocity.y *= newspeed;
				}
			}
		}
	}
	
	if ( ( basevel != vec_zero ) || ( ent->velocity != vec_zero ) || ( ent->total_delta != vec_zero ) )
	{
		if ( ent->edict->svflags & SVF_MONSTER )
		{
			mask = ent->edict->clipmask;
		}
		else
		{
			mask = MASK_SOLID;
		}
		
		G_FlyMove( ent, basevel, level.frametime, mask );
		
		ent->link();
		
		G_CheckWater( ent );
		if ( ent->flags & FL_TOUCH_TRIGGERS )
		{
			G_TouchTriggers( ent );
		}
		
		if ( ent->groundentity && !wasonground && hitsound )
		{
			ent->Sound( "impact_softland", CHAN_BODY, 0.5f );
		}
	}
}

//============================================================================

/*
================
G_RunEntity

================
*/
void G_RunEntity( Entity *ent )
{
	gentity_t *edict;
	
	edict = ent->edict;
	
	if ( !edict->inuse )
		return;
	
	if ( ent->isThinkOn() )
	{
		ent->Think();
	}
	
	// Only run physics if in use, not bound, and not immobilized
	
	if ( ( edict->s.parent == ENTITYNUM_NONE ) && !(ent->flags & FL_IMMOBILE) && !(ent->flags & FL_PARTIAL_IMMOBILE) )
	{
		switch ( ( int )ent->movetype )
		{
		case MOVETYPE_PUSH:
		case MOVETYPE_STOP:
			G_Physics_Pusher( ent );
			break;
		case MOVETYPE_NONE:
		case MOVETYPE_STATIONARY:
		case MOVETYPE_WALK:
			break;
		case MOVETYPE_NOCLIP:
			G_Physics_Noclip( ent );
			break;
		case MOVETYPE_STEP:
			G_Physics_Step( ent );
			break;
		case MOVETYPE_TOSS:
		case MOVETYPE_BOUNCE:
		case MOVETYPE_GIB:
		case MOVETYPE_FLY:
		case MOVETYPE_FLYMISSILE:
		case MOVETYPE_SLIDE:
		case MOVETYPE_VEHICLE:
			G_Physics_Toss( ent );				
			//G_Physics_Step( ent );				
			break;
		default:
			gi.Error( ERR_DROP, "G_Physics: bad movetype %i", ( int )ent->movetype );
		}
	}
	
	if ( ent->flags & FL_POSTTHINK )
	{
		ent->Postthink();
	}
}


//------------------------- CLASS ------------------------------
//
// Name:		Trajectory
// Base Class:	None
//
// Description:	This class computes trajectory information based 
// on various known inputs. Each known input has a specilized ctor
// that computes all other values from those that ar given
//
// Method of Use:	This class should not be aggegrated. It is intended
// for local/parameter use 
//
//--------------------------------------------------------------

//----------------------------------------------------------------
// Name:			Trajectory
// Class:			Trajectory
//
// Description:		launchAngle based constructor
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
Trajectory::Trajectory( const Vector &launchPoint, const Vector &targetPoint, const Angle launchAngle, const float gravity ):
	_launchPoint( launchPoint ),
	_targetPoint( targetPoint ),
	_launchAngle( launchAngle ),
	_gravity( gravity ),
	_travelTime( -1.0f ),
	_initialVelocity( Vector::Identity() )
{
	assert( gravity <= 0.0f );
	const Vector direction( targetPoint - launchPoint );
	const float horizontalDistance = direction.lengthXY();
	const float verticalDistance = -direction.z;
	const float cosineOfLaunchAngle = cos( DEG2RAD( _launchAngle ) );
	const float sineOfLaunchAngle = sin( DEG2RAD( _launchAngle ) );

	// Formula derived for information provided in "Physics for Game Developers" by David M. Bourrg
	const float travelTimeSquared = ( ( 2.0f * horizontalDistance * sineOfLaunchAngle) / ( -gravity * cosineOfLaunchAngle) ) +
										( 2.0f * verticalDistance / -gravity);

	if ( travelTimeSquared > 0.0f )
	{
		_travelTime = sqrt( travelTimeSquared );
		_initialVelocity = direction;
		_initialVelocity.z = 0.0f;
		_initialVelocity.normalize();
		_initialVelocity *= cosineOfLaunchAngle;
		_initialVelocity.z = sineOfLaunchAngle;
		_initialVelocity.normalize();

		const float initialSpeed = horizontalDistance / ( cosineOfLaunchAngle * _travelTime );
		_initialVelocity *= initialSpeed;

	}
	else
	{
		// launch angle and direction of travel do not allow projectile to reach destination
		gi.WPrintf( "Failed Trajectory: LaunchPoint<%f,%f,%f>, Target Point<%f,%f,%f>, LaunchAngle<%f>\n",
			launchPoint.x, launchPoint.y, launchPoint.z,
			targetPoint.x, targetPoint.y, targetPoint.z,
			launchAngle.GetValue()
			);
	}
}

//----------------------------------------------------------------
// Name:			Trajectory
// Class:			Trajectory
//
// Description:		initialSpeed based constructor
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
Trajectory::Trajectory( const Vector &launchPoint, const Vector &targetPoint, const float initialSpeed, const float gravity, const bool useHighTrajectory ):
	_launchPoint( launchPoint ),
	_targetPoint( targetPoint ),
	_gravity( gravity ),
	_travelTime( -1.0f ),
	_initialVelocity( Vector::Identity() )
{
	// Complex formula with multipage derivation. Math by Squirrel
	const Vector direction( targetPoint - launchPoint );
	
	Vector directionXY( direction );
	directionXY.z = 0.0f;
	directionXY.normalize();

	const double X = direction.lengthXY();
	const double Y = direction.z;
	const double R = initialSpeed;
	assert( gravity <= 0.0f );
	const double a = gravity; 

	const double X2 = X * X;
	const double Y2 = Y * Y;
	const double R2 = R * R;
	const double a2 = a * a;

	const double R4 = R2 * R2;
	const double Ya = Y * a;

	const double denominator = 2.0 * ( 1.0 + (Y2 / X2) );

	const bool test = (2*R2*Ya - X2*a2 > R2);
	const double radical = R4 + 2*R2*Ya - X2*a2;

	const double numerator1 = R2 + Ya + sqrt( radical );
	const double numerator2 = R2 + Ya - sqrt( radical );

	const double Vx2_1 = numerator1 / denominator;
	const double Vx2_2 = numerator2 / denominator;

	const double Vx2 = useHighTrajectory ? Vx2_2 : Vx2_1; 

	const double Vx = sqrt( Vx2 );
	double Vy = sqrt( R2 - Vx2 );
	if ( test && ! useHighTrajectory )
	{
		Vy *= -1.0f;
	}
	const double launchRadians = atan2( Vy, Vx );
	const double launchQuakePitchDegrees = -RAD2DEG( launchRadians );

	const double travelTime = X / Vx;
	if( travelTime < 0.0 )
		return;

	_launchAngle = launchQuakePitchDegrees;
	_initialVelocity = (directionXY * Vx) + Vector( 0.0f, 0.0f, Vy );
	_initialVelocity.normalize();
	_initialVelocity *= initialSpeed;
	_travelTime = travelTime;
}
