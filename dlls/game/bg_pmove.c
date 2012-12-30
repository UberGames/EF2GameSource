//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/bg_pmove.c                                    $
// $Revision:: 62                                                             $
//     $Date:: 10/13/03 9:42a                                                 $
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

#include "q_shared.h"
#include "bg_public.h"

// all of the locals will be zeroed before each
// pmove, just to make sure we don't have
// any differences when running on client or server
typedef struct {
	vec3_t		forward, left, up;
	vec3_t		flat_forward, flat_left, flat_up;
   //float       forward_speed, side_speed;
	float		frametime;

	int			msec;

	qboolean	walking;
	qboolean	groundPlane;
	trace_t		groundTrace;

	float		impactSpeed;

	vec3_t		previous_origin;
	vec3_t		previous_velocity;
	int			previous_waterlevel;
} pml_t;

pmove_t	*pm;
pml_t		pml;
qboolean slopeSlideFlag = qfalse;

// movement parameters
const float	pm_swimScale         = 0.50f;
const float	pm_wadeScale         = 0.70f;
const float	pm_slipperyfriction  = 0.25f;
const float pm_leaninspeed		 = 75.0f;
const float pm_leanoutspeed		 = 150.0f;
const float	pm_leanmaxdelta		 = 35.0f;

int c_pmove = 0;

void PM_CrashLand( void );
qboolean PM_ShouldCrashLand( void );

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt
   (
   int entityNum
   )

   {
	int i;

	if ( entityNum == ENTITYNUM_WORLD )
      {
		return;
	   }

	if ( pm->numtouch == MAXTOUCH )
      {
		return;
	   }

	// see if it is already added
	for( i = 0; i < pm->numtouch; i++ )
      {
		if ( pm->touchents[ i ] == entityNum )
         {
			return;
		   }
	   }

	// add it
	pm->touchents[ pm->numtouch ] = entityNum;
	pm->numtouch++;
   }

/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity
   (
   const vec3_t in,
   const vec3_t normal,
   vec3_t out,
   float overbounce
   )

   {
	float	backoff;
	float	change;
	int	i;

	backoff = DotProduct( in, normal );

	if ( backoff < 0.0f )
      {
		backoff *= overbounce;
	   }
   else
      {
		backoff /= overbounce;
	   }

	for( i = 0; i < 3; i++ )
      {
		change = normal[ i ] * backoff;
		out[ i ] = in[ i ] - change;
	   }
   }

/*
==================
PM_SlideMove

Returns qtrue if the velocity was clipped in some way
==================
*/
#define	MAX_CLIP_PLANES	5
qboolean	PM_SlideMove( qboolean gravity ) {
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity;
	vec3_t		clipVelocity;
	int			i, j, k;
	trace_t	trace;
	vec3_t		end;
	float		time_left;
	float		into;
	vec3_t		endVelocity;
	vec3_t		endClipVelocity;
	int gravtmp;

	numbumps = 4;

	VectorCopy (pm->ps->velocity, primal_velocity);

	if ( gravity ) {
		if ( pm->ps->pm_flags & PMF_FLIGHT )
			gravtmp = 0;
		else
			gravtmp = pm->ps->gravity;

		VectorCopy( pm->ps->velocity, endVelocity );
		endVelocity[2] -= gravtmp * pml.frametime;
		pm->ps->velocity[2] = ( pm->ps->velocity[2] + endVelocity[2] ) * 0.5f;
		primal_velocity[2] = endVelocity[2];
		if ( pml.groundPlane ) {
			// slide along the ground plane
			PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
				pm->ps->velocity, OVERCLIP );
		}
	}

	time_left = pml.frametime;

	// never turn against the ground plane
	if ( pml.groundPlane ) {
		numplanes = 1;
		VectorCopy( pml.groundTrace.plane.normal, planes[0] );
	} else {
		numplanes = 0;
	}

	// never turn against original velocity
	VectorNormalize2( pm->ps->velocity, planes[numplanes] );
	numplanes++;

	for ( bumpcount=0 ; bumpcount < numbumps ; bumpcount++ ) {

		// calculate position we are trying to move to
		VectorMA( pm->ps->origin, time_left, pm->ps->velocity, end );

		// add exact pull from gravity
	//	if ( gravity && !(pm->ps->pm_flags & PMF_FLIGHT) ) {
	//		pm->ps->origin[2] += 0.5 * time_left * time_left;
	//	}

		// see if we can make it there
		pm->trace ( &trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask, qtrue );

      if ( trace.startsolid && trace.entityNum != ENTITYNUM_WORLD )
         {
         // stuck in an entity, so try to pretend it's not there
         pm->trace ( &trace, pm->ps->origin, pm->mins, pm->maxs, end, trace.entityNum, pm->tracemask, qtrue );
         }

      if (trace.allsolid) {
			// entity is completely trapped in another solid
			pm->ps->velocity[2] = 0.0f;	// don't build up falling damage, but allow sideways acceleration
			return qtrue;
		}

      if ( pm->trypush && pm->trypush( trace.entityNum, pm->ps->origin, end ) )
			continue;

		if (trace.fraction > 0.0f) {
			// actually covered some distance
			VectorCopy (trace.endpos, pm->ps->origin);
		}

		if (trace.fraction == 1.0f) {
			 break;		// moved the entire distance
		}

      /*if ( ( trace.plane.normal[ 2 ] < MIN_WALK_NORMAL ) && ( trace.plane.normal[ 2 ] > 0 ) )
      {
         // treat steep walls as vertical
         trace.plane.normal[ 2 ] = 0;
         VectorNormalize( trace.plane.normal );
      }*/

		// save entity for contact
		PM_AddTouchEnt( trace.entityNum );

      time_left -= time_left * trace.fraction;

		if (numplanes >= MAX_CLIP_PLANES) {
			// this shouldn't really happen
			VectorClear( pm->ps->velocity );
			return qtrue;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for ( i = 0 ; i < numplanes ; i++ ) {
			if ( DotProduct( trace.plane.normal, planes[i] ) > 0.99f ) {
				VectorAdd( trace.plane.normal, pm->ps->velocity, pm->ps->velocity );
				break;
			}
		}
		if ( i < numplanes ) {
			continue;
		}
		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for ( i = 0 ; i < numplanes ; i++ ) {
			into = DotProduct( pm->ps->velocity, planes[i] );
			if ( into >= 0.1f ) {
				continue;		// move doesn't interact with the plane
			}

			// see how hard we are hitting things
			if ( -into > pml.impactSpeed ) {
				pml.impactSpeed = -into;
			}

			// slide along the plane
			PM_ClipVelocity (pm->ps->velocity, planes[i], clipVelocity, OVERCLIP );

			// slide along the plane
			PM_ClipVelocity (endVelocity, planes[i], endClipVelocity, OVERCLIP );

			// see if there is a second plane that the new move enters
			for ( j = 0 ; j < numplanes ; j++ ) {
				if ( j == i ) {
					continue;
				}
				if ( DotProduct( clipVelocity, planes[j] ) >= 0.1f ) {
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				PM_ClipVelocity( clipVelocity, planes[j], clipVelocity, OVERCLIP );
				PM_ClipVelocity( endClipVelocity, planes[j], endClipVelocity, OVERCLIP );

				// see if it goes back into the first clip plane
				if ( DotProduct( clipVelocity, planes[i] ) >= 0.0f ) {
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, pm->ps->velocity );
				VectorScale( dir, d, clipVelocity );

				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, endVelocity );
				VectorScale( dir, d, endClipVelocity );

				// see if there is a third plane the the new move enters
				for ( k = 0 ; k < numplanes ; k++ ) {
					if ( k == i || k == j ) {
						continue;
					}
					if ( DotProduct( clipVelocity, planes[k] ) >= 0.1f ) {
						continue;		// move doesn't interact with the plane
					}

					// stop dead at a triple plane interaction
					VectorClear( pm->ps->velocity );
					return qtrue;
				}
			}

			// if we have fixed all interactions, try another move
			VectorCopy( clipVelocity, pm->ps->velocity );
			VectorCopy( endClipVelocity, endVelocity );
			break;
		}
	}

	if ( gravity ) {
		VectorCopy( endVelocity, pm->ps->velocity );
	}

	// don't change velocity if in a timer (FIXME: is this correct?)
	if ( pm->ps->pm_time ) {
		VectorCopy( primal_velocity, pm->ps->velocity );
	}

	return ( bumpcount != 0 );
}
/*
void PM_StepSlideMove( qboolean gravity ) {
	vec3_t		start_o, start_v;
	trace_t		trace;
//	float		down_dist, up_dist;
//	vec3_t		delta, delta2;
	vec3_t		up, down;
	vec3_t		down_o, down_v;

	VectorCopy (pm->ps->origin, start_o);
	VectorCopy (pm->ps->velocity, start_v);

	if ( PM_SlideMove( gravity ) == 0 ) {
		return;		// we got exactly where we wanted to go first try
	}

	VectorCopy(start_o, down);
	down[2] -= STEPSIZE;
	pm->trace (&trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask, qtrue);
	VectorSet(up, 0, 0, 1);
	// never step up when you still have up velocity
	if ( pm->ps->velocity[2] > 0 && (trace.fraction == 1.0 ||
										DotProduct(trace.plane.normal, up) < 0.7)) {
		return;
	}

	VectorCopy (start_o, up);
	up[2] += STEPSIZE;

	// test the player position if they were a stepheight higher
	pm->trace (&trace, up, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask, qtrue);
	if ( trace.allsolid ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:bend can't step\n", c_pmove);
		}
		return;		// can't step up
	}

	// try slidemove from this position
	VectorCopy (up, pm->ps->origin);
	VectorCopy (start_v, pm->ps->velocity);

	PM_SlideMove( gravity );

	VectorCopy (pm->ps->origin, down_o);
	VectorCopy (pm->ps->velocity, down_v);

	// push down the final amount
	VectorCopy (pm->ps->origin, down);
	down[2] -= STEPSIZE;
	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask, qtrue );

	if ( !trace.allsolid )
		{
      if ( 	( trace.fraction < 1.0 ) && trace.plane.normal[ 2 ] < MIN_WALK_NORMAL )
			{
			slopeSlideFlag = qtrue;
         //VectorCopy (start_o, pm->ps->origin);
         //return;
			} else
				slopeSlideFlag = qfalse;
		}
   

	VectorCopy (trace.endpos, pm->ps->origin);

	if ( trace.fraction < 1.0 ) {
		PM_ClipVelocity( pm->ps->velocity, trace.plane.normal, pm->ps->velocity, OVERCLIP );
	}

	// use the step move
	pm->stepped = qtrue;
	if ( pm->debugLevel ) {
		Com_Printf("%i:stepped\n", c_pmove);
	}
}*/

float PM_TryStepSlideMove( qboolean gravity, float stepSize )
{
	trace_t		trace;
	vec3_t		up, down;
	vec3_t		diff;
	float		distance;

	VectorCopy( pm->ps->origin, up );
	up[2] += stepSize;

	// test the player position if they were a stepheight higher
	pm->trace( &trace, up, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask, qtrue );

	if ( trace.allsolid ) 
	{
		if ( pm->debugLevel ) 
		{
			Com_WPrintf("%i:bend can't step\n", c_pmove);
		}
		return 0.0f;		// can't step up
	}

	// try slidemove from this position
	VectorCopy( up, pm->ps->origin );
	//VectorCopy( start_v, pm->ps->velocity );

	PM_SlideMove( gravity );

	VectorSubtract( pm->ps->origin, up, diff );
	distance = VectorLength( diff );

	// push down the final amount
	VectorCopy (pm->ps->origin, down);
	down[2] -= stepSize;
	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask, qtrue );

	if ( !trace.allsolid ) 
	{
		VectorCopy (trace.endpos, pm->ps->origin);
	}

	if ( trace.fraction < 1.0f ) 
	{
		PM_ClipVelocity( pm->ps->velocity, trace.plane.normal, pm->ps->velocity, OVERCLIP );
	}

	return distance;
}

void PM_StepSlideMove( qboolean gravity ) 
{
	vec3_t		start_o, start_v;
	float		try1Distance, try2Distance, simpleTryDistance;
	vec3_t		try1_o, try1_v;
	vec3_t		simpleTry1_o, simpleTry1_v;
	vec3_t		simpleDiff;

	// Save off the starting info

	VectorCopy( pm->ps->origin, start_o );
	VectorCopy( pm->ps->velocity, start_v );

	// Try the movement directly forward first

	if ( PM_SlideMove( gravity ) == 0 ) 
	{
		return;		// we got exactly where we wanted to go first try	
	}

	// Save off attempt

	VectorCopy( pm->ps->origin, simpleTry1_o );
	VectorCopy( pm->ps->velocity, simpleTry1_v );

	VectorSubtract( simpleTry1_o, start_o, simpleDiff );
	simpleTryDistance = VectorLength( simpleDiff );

	// Reset everything that could have changed so far

	VectorCopy( start_o, pm->ps->origin );
	VectorCopy( start_v, pm->ps->velocity );

	// Try movement when stepping up a full stepsize

	try1Distance = PM_TryStepSlideMove( gravity, STEPSIZE );

	// Save off attempt

	VectorCopy( pm->ps->origin, try1_o );
	VectorCopy( pm->ps->velocity, try1_v );

	// Reset everything that could have changed so far

	VectorCopy( start_o, pm->ps->origin );
	VectorCopy( start_v, pm->ps->velocity );

	// Try movement when stepping up half the stepsize

	try2Distance = PM_TryStepSlideMove( gravity, STEPSIZE / 2.0f );

	// Pick the move that went the furthest forward

	if ( try1Distance > try2Distance )
	{
		VectorCopy( try1_o, pm->ps->origin );
		VectorCopy( try1_v, pm->ps->velocity );
	}

	if ( ( try1Distance < 0.001f ) && ( try2Distance < 0.001f ) && ( simpleTryDistance > 0.0f ) )
	{
		VectorCopy( simpleTry1_o, pm->ps->origin );
		VectorCopy( simpleTry1_v, pm->ps->velocity );
	}

	pm->stepped = qtrue;		// allow client to smooth out the step

	if ( pm->debugLevel ) 
	{
		Com_Printf("%i:stepped\n", c_pmove);
	}
}

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
void PM_Friction
   (
   void
   )

   {
	vec3_t	vec;
	float	   *vel;
	float	   speed;
   float	   newspeed;
   float	   control;
	float	   drop;

	vel = pm->ps->velocity;

	VectorCopy( vel, vec );
	if ( pml.walking )
      {
      // ignore slope movement
		vec[ 2 ] = 0.0f;
	   }

	speed = VectorLength( vec );
	if ( speed < 1.0f )
      {
      // allow sinking underwater
		vel[ 0 ] = 0;
		vel[ 1 ] = 0;

		return;
	   }

	drop = 0;

	// apply ground friction
	if ( pml.walking )
      {
		// if getting knocked back, no friction
		if ( !( pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) )
         {
			control = ( speed < (float)pm->ps->pm_stopspeed ) ? (float)pm->ps->pm_stopspeed : speed;
         if ( pml.groundTrace.surfaceFlags & SURF_SLICK )
            {
   			drop += control * pm_slipperyfriction * pml.frametime;
            }
         else
            {
   			drop += control * (float)pm->ps->pm_friction * pml.frametime;
            }
		   }
	   }

	// apply water friction
	if ( pm->waterlevel )
      {
      if ( pm->watertype & CONTENTS_SLIME )
         {
   		drop += speed * (float)pm->ps->pm_waterfriction * 5.0f * pm->waterlevel * pml.frametime;
   		//drop += speed * pm->ps->pm_waterfriction * 2 * pml.frametime;
         }
      else
         {
   		drop += speed * (float)pm->ps->pm_waterfriction * (float)pm->waterlevel * pml.frametime;
         }
	   }

	// scale the velocity
	newspeed = speed - drop;
	if ( newspeed < 0.0f )
      {
		newspeed = 0;
	   }

	newspeed /= speed;

	vel[0] = vel[ 0 ] * newspeed;
	vel[1] = vel[ 1 ] * newspeed;
	vel[2] = vel[ 2 ] * newspeed;
   }


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
void PM_Accelerate
   (
   const vec3_t wishdir,
   float wishspeed,
   float accel
   )

   {
	int	i;
	float	addspeed;
   float accelspeed;
   float currentspeed;

	currentspeed = DotProduct( pm->ps->velocity, wishdir );
	addspeed = wishspeed - currentspeed;
	if ( addspeed <= 0.0f )
      {
		return;
	   }

	accelspeed = accel * pml.frametime * wishspeed;
	if ( accelspeed > addspeed )
      {
		accelspeed = addspeed;
	   }

	for( i = 0; i < 3; i++ )
      {
		pm->ps->velocity[ i ] += accelspeed * wishdir[ i ];
	   }
   }


/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/

float PM_CmdScale
   (
   const usercmd_t *cmd
   )

   {
	int	max;
	float	total;
	float	scale;

	max = abs( cmd->forwardmove );
	if ( abs( cmd->rightmove ) > max )
      {
		max = abs( cmd->rightmove );
	   }

	if ( abs( cmd->upmove ) > max )
      {
		max = abs( cmd->upmove );
	   }

	if ( !max )
      {
		return 0;
	   }

	if ( ( pm->ps->pm_flags & PMF_DUCKED ) && ( cmd->upmove >= 0 ) )
	{
		// This means we are being forced to duck (take into account this forced down movement)
		total = sqrt( ( cmd->forwardmove * cmd->forwardmove ) + ( cmd->rightmove * cmd->rightmove ) + ( -127 * -127 ) );
	}
	else
	{
		// This is the normal case 
		total = sqrt( ( cmd->forwardmove * cmd->forwardmove ) + ( cmd->rightmove * cmd->rightmove ) + ( cmd->upmove * cmd->upmove ) );
	}

	scale = (float)pm->ps->speed * (float)max / ( 127.0f * total );

	return scale;
   }

/*
=============
PM_CheckJump
=============
*/

qboolean PM_CheckJump( void ) {

	//If we are ducked, then we can't jump
	//PMF_DUCKED means the player is either 
	//pressing duck, or the player is ducked in
	//an area where they cannot stand up.
	if(pm->ps->pm_flags & PMF_DUCKED)
		return qfalse;

/*	jhefty/jwaters -- another strafe-jump fix
	if (pm->ps->commandTime < pm->ps->pm_landtime)
		return qfalse;
*/
	/* if ( pm->ps->pm_flags & PMF_TIME_LAND ) {
		// hasn't been long enough since landing to jump again
		return qfalse;
	} */

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		return qfalse;
	}

	// must wait for jump to be released
	if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
		return qfalse;
	}

	pml.groundPlane = qfalse;		// jumping away
 	pml.walking = qfalse;
	pm->ps->pm_flags |= PMF_JUMP_HELD;

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pm->ps->velocity[2] = pm->ps->jumpvelocity;

	return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
qboolean	PM_CheckWaterJump
   (
   void
   )

   {
	vec3_t	spot;
	int		cont;

	if ( pm->ps->pm_time )
      {
		return qfalse;
	   }

	// check for water jump
	if ( ( pm->waterlevel < 2 ) )
      {
		return qfalse;
	   }
   // if we are below the surface and not in slime, return
	if ( !( pm->watertype & CONTENTS_SLIME ) && ( pm->waterlevel == 3 ) )
      {
		return qfalse;
	   }

   VectorMA( pm->ps->origin, 80.0f, pml.flat_forward, spot );
	spot[ 2 ] += pm->ps->viewheight - 16.0f;
	cont = pm->pointcontents( spot, pm->ps->clientNum );
	if ( !( cont & pm->tracemask ) )
      {
		return qfalse;
	   }

	spot[ 2 ] += 48.0f;
	cont = pm->pointcontents( spot, pm->ps->clientNum );
	if ( cont )
      {
		return qfalse;
	   }

	// jump out of water
	VectorScale( pml.flat_forward, 150, pm->ps->velocity );
	pm->ps->velocity[ 2 ] = 600;

	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 2000;

	return qtrue;
   }

//============================================================================


/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
void PM_WaterJumpMove
   (
   void
   )

   {
	// waterjump has no control, but falls
	PM_StepSlideMove( !( pm->ps->pm_flags & PMF_NO_GRAVITY ) );

	pm->ps->velocity[ 2 ] -= (float)pm->ps->gravity * pml.frametime;
	if ( pm->ps->velocity[ 2 ] < 0.0f )
      {
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	   }
   }

#define SLIME_SINK_SPEED -10.0f
/*
===================
PM_WaterMove

===================
*/
void PM_WaterMove
   (
   void
   )

   {
	int		i;
	vec3_t	wishvel;
	float	   wishspeed;
	vec3_t	wishdir;
	float	   scale;

	if ( PM_CheckWaterJump() )
      {
		PM_WaterJumpMove();
		return;
   	}

   //
   // clamp our speed if we are in slime
   //
   if ( pm->watertype & CONTENTS_SLIME )
      {
      if ( pm->ps->velocity[ 2 ] < SLIME_SINK_SPEED )
         {
         pm->ps->velocity[ 2 ] = SLIME_SINK_SPEED;
         }
      }

	PM_Friction();

	scale = PM_CmdScale( &pm->cmd );

	//
	// user intentions
	//
	if ( !scale )
      {
		wishvel[ 0 ] = 0;
		wishvel[ 1 ] = 0;
      if ( pm->watertype & CONTENTS_SLIME )
         {
	      wishvel[ 2 ] = SLIME_SINK_SPEED;		// sink towards bottom
         }
      else
         {
		   wishvel[ 2 ] = -60;		// sink towards bottom
         }
	   }
   else
      {
		for( i = 0; i < 3; i++ )
         {
			wishvel[ i ] = ( scale * pml.flat_forward[ i ] * pm->cmd.forwardmove ) - ( scale * pml.flat_left[ i ] * pm->cmd.rightmove );
         }

		wishvel[ 2 ] += scale * pm->cmd.upmove;
	   }
   if ( ( pm->watertype & CONTENTS_SLIME ) && ( pm->waterlevel > 2 ) && ( wishvel[ 2 ] < 0.0f ) )
      {
      wishvel[ 2 ] = 0;
      }

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );

	if ( wishspeed > ( pm->ps->speed * pm_swimScale ) )
      {
		wishspeed = pm->ps->speed * pm_swimScale;
	   }

	PM_Accelerate( wishdir, wishspeed, pm->ps->pm_wateraccelerate );

	PM_SlideMove( qfalse );
   }

/*
===================
PM_StuckJumpMove

Flying out of someplace we were stuck
===================
*/
void PM_StuckJumpMove
   (
   void
   )

   {
	// stuckjump has no control, but falls
	PM_StepSlideMove( !( pm->ps->pm_flags & PMF_NO_GRAVITY ) );

	pm->ps->velocity[ 2 ] -= pm->ps->gravity * pml.frametime;
	if ( pm->ps->velocity[ 2 ] < -48.0f )
      {
		// cancel as soon as we are falling at decent clip again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	   }
   }

/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
void PM_FlyMove( void ) {
	float	   speed;
   float    drop;
   float    friction;
   float    control;
   float    newspeed;
	int		i;
	vec3_t	wishvel;
	float		fmove;
   float    smove;
	vec3_t	wishdir;
	float		wishspeed;
	float		scale;

	// friction

	speed = VectorLength( pm->ps->velocity );
	if ( speed < 1.0f )
	   {
		VectorCopy( vec3_origin, pm->ps->velocity );
	   }
	else
	   {
		drop = 0;

      // extra friction
		friction = pm->ps->pm_friction * 1.5f;

		control = speed < pm->ps->pm_stopspeed ? pm->ps->pm_stopspeed : speed;
		drop += control * friction * pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if ( newspeed < 0.0f )
         {
			newspeed = 0;
         }
		newspeed /= speed;

		VectorScale( pm->ps->velocity, newspeed, pm->ps->velocity );
	   }

	// accelerate
	scale = PM_CmdScale( &pm->cmd );

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
   pm->ps->pm_runtime = 0;

	for( i = 0; i < 3; i++ )
      {
		wishvel[ i ] = ( pml.flat_forward[ i ] * fmove ) - ( pml.flat_left[ i ] * smove );
      }

	wishvel[ 2 ] += pm->cmd.upmove;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm->ps->pm_accelerate );

	// move
	PM_StepSlideMove( qtrue );
	//VectorMA( pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin );
}

/*
=============
PM_CheckStuckJump
=============
*/
//#define MAX_XY_VELOCITY 50
qboolean	PM_CheckStuckJump
   (
   void
   )

   {
   vec3_t   diff;

	if ( pm->ps->pm_time )
      {
		return qfalse;
	   }

	if ( pm->waterlevel > 1 )
      {
		return qfalse;
	   }

   VectorSubtract( pm->ps->origin, pml.previous_origin, diff );
   if ( VectorLength( diff ) )
      {
      return qfalse;
      }
   if ( VectorLength( pm->ps->velocity ) < 100.0f )
      {
      return qfalse;
      }

   // we have been falling, we haven't moved and our velocity is getting dangerously high
   // let's give ourselves a boost straight up and opposite our current velocity
  /* pm->ps->velocity[ 0 ] = -pm->ps->velocity[ 0 ];
   if ( pm->ps->velocity[ 0 ] > MAX_XY_VELOCITY )
      pm->ps->velocity[ 0 ] = MAX_XY_VELOCITY;
   else if ( pm->ps->velocity[ 0 ] < -MAX_XY_VELOCITY )
      pm->ps->velocity[ 0 ] = -MAX_XY_VELOCITY;

   pm->ps->velocity[ 1 ] = -pm->ps->velocity[ 1 ];
   if ( pm->ps->velocity[ 1 ] > MAX_XY_VELOCITY )
      pm->ps->velocity[ 1 ] = MAX_XY_VELOCITY;
   else if ( pm->ps->velocity[ 1 ] < -MAX_XY_VELOCITY )
      pm->ps->velocity[ 1 ] = -MAX_XY_VELOCITY;

	pm->ps->velocity[ 2 ] = 500;

	pm->ps->pm_flags |= PMF_TIME_STUCKJUMP;
	pm->ps->pm_time = 2000;*/

	return qtrue;
   }

/*
=============
PM_CheckTerminalVelocity
=============
*/
#define TERMINAL_VELOCITY 1200.0f

void PM_CheckTerminalVelocity
   (
   void
   )
   {
   float oldspeed;
   float speed;

   //
   // how fast were we falling
   //
   oldspeed = -pml.previous_velocity[ 2 ];

   //
   // how fast are we falling
   //
   speed = -pm->ps->velocity[ 2 ];

	if ( speed <= 0.0f )
      {
		return;
	   }

	if ( ( oldspeed <= TERMINAL_VELOCITY ) && ( speed > TERMINAL_VELOCITY ) )
      {
		pm->pmoveEvent = EV_TERMINAL_VELOCITY;
   	}
   }

/*
===================
PM_AirMove

===================
*/
void PM_AirMove
   (
   void
   )

   {
	vec3_t		wishvel;
	float		   fmove;
   float       smove;
	vec3_t		wishdir;
	float		   wishspeed;
	float		   scale;
	usercmd_t	cmd;

	vec3_t		original_start;
	vec3_t		original_end;
	float			old_speed;
	vec3_t original_diff;
	float original_dist;
	vec3_t new_diff;
	float new_dist;
	float new_speed;
	float max_new_speed;

	// Save some information about original state of things

	VectorCopy( pm->ps->origin, original_start );
	VectorMA( pm->ps->origin, pml.frametime, pm->ps->velocity, original_end );
	old_speed = VectorLength( pm->ps->velocity );

   //PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
   pm->ps->pm_runtime = 0;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

   wishvel[ 0 ] = ( pml.flat_forward[ 0 ] * fmove ) - ( pml.flat_left[ 0 ] * smove );
   wishvel[ 1 ] = ( pml.flat_forward[ 1 ] * fmove ) - ( pml.flat_left[ 1 ] * smove );
	wishvel[ 2 ] = 0.0f;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	PM_Accelerate( wishdir, wishspeed, pm->ps->pm_airaccelerate );

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if ( pml.groundPlane )
      {
		PM_ClipVelocity( pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP );
	   }

   if ( !pml.walking && pml.groundPlane )
      {
      vec3_t vel;

		VectorCopy( pm->ps->velocity, vel );
		vel[ 2 ] -= pm->ps->gravity * pml.frametime;
		pm->ps->velocity[ 2 ] = ( pm->ps->velocity[ 2 ] + vel[ 2 ] ) * 0.5f;
      PM_SlideMove( qfalse );
      VectorCopy( vel, pm->ps->velocity );
      }
   else
      {
      PM_SlideMove( !( pm->ps->pm_flags & PMF_NO_GRAVITY ) );
      }
	
	if ( PM_CheckStuckJump() )
      {
		PM_StuckJumpMove();
   	}

	// Get the distance we tried to move

	VectorSubtract( original_start, original_end, original_diff );
	original_dist = VectorLength( original_diff );

	// Get the distance we actually moved

	VectorSubtract( original_start, pm->ps->origin, new_diff );
	new_dist = VectorLength( new_diff );

	if ( ( new_dist < original_dist ) && ( new_dist > 0 ) )
		{
		// Modify our velocity based on how far we got to move

		max_new_speed = old_speed * ( new_dist / original_dist );

		new_speed = VectorLength( pm->ps->velocity );

		if ( new_speed > max_new_speed )
			{
			VectorNormalize( pm->ps->velocity );
			VectorScale( pm->ps->velocity, max_new_speed, pm->ps->velocity );
			}
		}

   PM_CheckTerminalVelocity();
   }

void AddPlane
   (
   const vec3_t norm,
   vec3_t planes[ MAX_CLIP_PLANES ],
   int *numplanes
   )

   {
   int i;

   if ( *numplanes >= MAX_CLIP_PLANES )
      {
      return;
      }

   for( i = 0; i < *numplanes; i++ )
      {
      if ( VectorCompare( planes[ i ], norm ) )
         {
         // don't add the plane twice
         return;
         }
      }

   VectorCopy( norm, planes[ *numplanes ] );
	( *numplanes )++;
   }

void PM_StepMove
   (
   void
   )

   {
   trace_t trace;
   vec3_t up;
   vec3_t down;
   vec3_t oldvelocity;
   vec3_t oldorigin;
   vec3_t velocity1;
   vec3_t origin1;

   VectorCopy( pm->ps->velocity, oldvelocity );
   VectorCopy( pm->ps->origin, oldorigin );

   if ( PM_SlideMove( !( pm->ps->pm_flags & PMF_NO_GRAVITY ) ) )
      {
      VectorCopy( pm->ps->velocity, velocity1 );
      VectorCopy( pm->ps->origin, origin1 );

      VectorCopy( oldvelocity, pm->ps->velocity );

      VectorCopy( oldorigin, up );
      up[ 2 ] += STEPSIZE;

      //pm->trace( &trace, oldorigin, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask, qtrue );
      pm->trace( &trace, up, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask, qtrue );
      VectorCopy( trace.endpos, pm->ps->origin );

      PM_SlideMove( !( pm->ps->pm_flags & PMF_NO_GRAVITY ) );

      VectorCopy( pm->ps->origin, down );
      down[ 2 ] = oldorigin[ 2 ];

      pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask, qtrue );
      if ( trace.plane.normal[ 2 ] < MIN_WALK_NORMAL )
         {
         // use the first move
         VectorCopy( velocity1, pm->ps->velocity );
         VectorCopy( origin1, pm->ps->origin );
         }
      else
         {
         VectorCopy( trace.endpos, pm->ps->origin );
         pm->stepped = qtrue;
         }
      }
   }

/*
===================
PM_WalkMove

===================
*/

void PM_WalkMove
   (
   void
   )

   {
   int         i;
	vec3_t		wishvel;
	float		   fmove;
   float       smove;
	vec3_t		wishdir;
	float		   wishspeed;
	float		   scale;
	usercmd_t	cmd;
	float		   accelerate;
   float	      waterScale;

	if ( ( pm->waterlevel > 2 ) && ( DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0.0f ) )
      {
		// begin swimming
		PM_WaterMove();

		return;
	   }

	if ( pm->ps->instantJump )
		if ( PM_CheckJump () ) {
			// jumped away
			pm->ps->jumped = qtrue;
			pm->ps->pm_time = 150;
			pm->ps->pm_flags |= PMF_TIME_JUMP_START;
			if ( pm->waterlevel > 1 ) {
				PM_WaterMove();
			} else {
				PM_AirMove();
			}
			return;
		}

	PM_Friction();

   fmove = pm->cmd.forwardmove;
   smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

   if ( ( pm->cmd.buttons & BUTTON_RUN ) && fmove && !smove )
      {
      pm->ps->pm_runtime += pml.msec;
      }
   else
      {
      pm->ps->pm_runtime = 0;
      }

   //
   // only run faster if we have exceeded our running time
   //
/*   if ( ( pm->ps->stats[STAT_WATER_LEVEL] >= MINIMUM_WATER_FOR_TURBO ) && ( pm->ps->pm_runtime > WATER_TURBO_TIME ) )
      {
      scale *= WATER_TURBO_SPEED;
      }*/

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity (pml.flat_forward, pml.groundTrace.plane.normal, pml.flat_forward, OVERCLIP );
	PM_ClipVelocity (pml.flat_left, pml.groundTrace.plane.normal, pml.flat_left, OVERCLIP );

	VectorNormalize (pml.flat_forward);
	VectorNormalize (pml.flat_left);

	for( i = 0 ; i < 3 ; i++ )
      {
		wishvel[ i ] = pml.flat_forward[ i ] * fmove - pml.flat_left[ i ] * smove;
	   }

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	// clamp the speed lower if wading or walking on the bottom
	if ( pm->waterlevel )
      {
		waterScale = pm->waterlevel / 3.0f;
		waterScale = 1.0f - ( 1.0f - pm_swimScale ) * waterScale;
		if ( wishspeed > ( pm->ps->speed * waterScale ) )
         {
			wishspeed = pm->ps->speed * waterScale;
		   }
	   }

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || ( pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) )
      {
		accelerate = pm->ps->pm_airaccelerate;
	   }
   else
      {
		accelerate = pm->ps->pm_accelerate;
	   }

	PM_Accelerate( wishdir, wishspeed, accelerate );

	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || ( pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) )
      {
		pm->ps->velocity[ 2 ] -= pm->ps->gravity * pml.frametime;
	   }

	// don't do anything if standing still
	if ( !pm->ps->velocity[ 0 ] && !pm->ps->velocity[ 1 ] && !pm->ps->velocity[ 2 ] )
		return;

   PM_StepSlideMove ( !( pm->ps->pm_flags & PMF_NO_GRAVITY ) );
   }

/*
==============
PM_DeadMove
==============
*/
void PM_DeadMove
   (
   void
   )

   {
	float	forward;

	if ( !pml.walking )
      {
		return;
	   }

	// extra friction
	forward = VectorLength( pm->ps->velocity );
	forward -= 20.0f;
	if ( forward <= 0.0f )
      {
		VectorClear( pm->ps->velocity );
	   }
   else
      {
		VectorNormalize( pm->ps->velocity );
		VectorScale( pm->ps->velocity, forward, pm->ps->velocity );
	   }
   }


/*
===============
PM_NoclipMove
===============
*/
void PM_VehicleMove
   (
   void
   )

   {
	
  	vec3_t		wishvel;
	//float		   fmove;
   //float       smove;
	vec3_t		wishdir;
	float		   wishspeed;
	float		   scale;
	usercmd_t	cmd;
	float		   accelerate;
  
   //fmove = pm->cmd.forwardmove;
   //smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

   pm->ps->pm_runtime = 0;

	wishvel[0] = pm->ps->velocity[0];
	wishvel[1] = pm->ps->velocity[1];
	wishvel[2] = pm->ps->velocity[2];

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;
	
	accelerate = pm->ps->pm_accelerate;
	PM_Accelerate( wishdir, wishspeed, accelerate );

	// don't do anything if standing still
	if ( !pm->ps->velocity[ 0 ] && !pm->ps->velocity[ 1 ] )
		return;

   //PM_StepSlideMove ( !( pm->ps->pm_flags & PMF_NO_GRAVITY ) );
   }



/*
===============
PM_NoclipMove
===============
*/
void PM_NoclipMove
   (
   void
   )

   {
	float	   speed;
   float    drop;
   float    friction;
   float    control;
   float    newspeed;
	int		i;
	vec3_t	wishvel;
	float		fmove;
   float    smove;
	vec3_t	wishdir;
	float		wishspeed;
	float		scale;

	pm->ps->viewheight = pm->ps->pm_defaultviewheight;

	// friction

	speed = VectorLength( pm->ps->velocity );
	if ( speed < 1.0f )
	   {
		VectorCopy( vec3_origin, pm->ps->velocity );
	   }
	else
	   {
		drop = 0;

      // extra friction
		friction = pm->ps->pm_friction * 1.5f;

		control = speed < pm->ps->pm_stopspeed ? pm->ps->pm_stopspeed : speed;
		drop += control * friction * pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if ( newspeed < 0.0f )
         {
			newspeed = 0;
         }
		newspeed /= speed;

		VectorScale( pm->ps->velocity, newspeed, pm->ps->velocity );
	   }

	// accelerate
   // allow the player to move twice as fast in noclip
	scale = PM_CmdScale( &pm->cmd ) * 2.0f;

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
   pm->ps->pm_runtime = 0;

	for( i = 0; i < 3; i++ )
      {
		wishvel[ i ] = ( pml.flat_forward[ i ] * fmove ) - ( pml.flat_left[ i ] * smove );
      }

	wishvel[ 2 ] += pm->cmd.upmove;

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm->ps->pm_accelerate );

	// move
	VectorMA( pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin );
   }

//============================================================================

/*
=============
PM_CorrectAllSolid
=============
*/
void PM_CorrectAllSolid
   (
   void
   )

   {
	if ( pm->debugLevel )
      {
		Com_Printf( "%i:allsolid\n", c_pmove );
	   }

	// FIXME: jitter around
	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
   }

/*
=============
PM_GroundTrace
=============
*/
void PM_GroundTrace
   (
   qboolean onlyTrace
   )

   {
	vec3_t   point;
	vec3_t	tmporg;
	trace_t  trace;

	point[ 0 ] = pm->ps->origin[ 0 ];
	point[ 1 ] = pm->ps->origin[ 1 ];
	point[ 2 ] = pm->ps->origin[ 2 ] - 9.0f; // long trace to avoid potential terrain hitches

	pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask, qtrue );

	if (!(trace.surfaceFlags & SURF_TERRAIN )) // if we're not on terrain, go back to 0.25f trace distance
	{
		point[ 2 ] = pm->ps->origin[ 2 ] - 0.25f;
		pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask, qtrue );
	}

	pml.groundTrace = trace;
	pm->ps->groundTrace = trace;

	// do something corrective if the trace starts in a solid...
	if ( trace.allsolid )
      {
		// We're gonna start with our origin a tad higher and see if it works
		tmporg[ 0 ] = pm->ps->origin[ 0 ];
		tmporg[ 1 ] = pm->ps->origin[ 1 ];
		tmporg[ 2 ] = pm->ps->origin[ 2 ] + 2.0f;

		// Trace down to the same point
		pm->trace( &trace, tmporg, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask, qtrue );
		pml.groundTrace = trace;
		pm->ps->groundTrace = trace;

		// It's hopeless if we start all solid again
		if ( trace.allsolid )
			{
			PM_CorrectAllSolid();
			pm->ps->walking = pml.walking;
			pm->ps->groundPlane = pml.groundPlane;

			return;
			}
	   }

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0f )
      {
		if ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
         {
			if ( pm->debugLevel )
            {
				Com_Printf( "%i:lift\n", c_pmove );
			   }
		   }

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;

      pm->ps->walking = pml.walking;
      pm->ps->groundPlane = pml.groundPlane;

      return;
	   }

	// slopes that are too steep will not be considered onground
	// check slopeSlideFlag to avoid the "falling" bug
	if ( trace.plane.normal[ 2 ] < MIN_WALK_NORMAL )
      {
      vec3_t oldvel;
      float d;

		if ( pm->debugLevel )
         {
			Com_Printf( "%i:steep\n", c_pmove );
		   }

		// if they can't slide down the slope, let them
		// walk (sharp crevices)
      VectorCopy( pm->ps->velocity, oldvel );
      VectorSet( pm->ps->velocity, 0.0f, 0.0f, -1.0f / pml.frametime );
      PM_SlideMove( qfalse );

      d = VectorLength( pm->ps->velocity );
      VectorCopy( oldvel, pm->ps->velocity );
      if ( d > ( 0.1f / pml.frametime ) )
         {
		   pm->ps->groundEntityNum = ENTITYNUM_NONE;
		   pml.groundPlane = qtrue;
		   pml.walking = qfalse;

         pm->ps->walking = pml.walking;
         pm->ps->groundPlane = pml.groundPlane;

		   return;
         }
	   }

	// check if getting thrown off the ground
	if ( ( pm->ps->velocity[ 2 ] > 0.0f ) && ( DotProduct( pm->ps->velocity, trace.plane.normal ) > 10.0f ) )
      {
		if ( pm->debugLevel )
         {
			Com_Printf( "%i:kickoff\n", c_pmove );
		   }
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;

      pm->ps->walking = pml.walking;
      pm->ps->groundPlane = pml.groundPlane;

		return;
	   }

	pml.groundPlane = qtrue;
	pml.walking = qtrue;

	// hitting solid ground will end a waterjump
	if ( pm->ps->pm_flags & PMF_TIME_WATERJUMP )
	   {
		//pm->ps->pm_flags &= ~( PMF_TIME_WATERJUMP | PMF_TIME_LAND);
		pm->ps->pm_flags &= ~( PMF_TIME_WATERJUMP );
		pm->ps->pm_time = 0;
	   }

	// hitting solid ground will end a stuckjump
	if ( pm->ps->pm_flags & PMF_TIME_STUCKJUMP )
	   {
		pm->ps->pm_flags &= ~( PMF_TIME_STUCKJUMP );
		pm->ps->pm_time = 0;
	   }

	if ( !onlyTrace && !( pml.groundTrace.surfaceFlags & SURF_SLICK ) )
		pm->ps->velocity[ 2 ] = 0;

	pm->ps->groundEntityNum = trace.entityNum;

	PM_AddTouchEnt( trace.entityNum );

   pm->ps->walking = pml.walking;
   pm->ps->groundPlane = pml.groundPlane;
   }


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
void PM_SetWaterLevel
   (
   void
   )

   {
	vec3_t  point;
	int	  cont;
	int	  sample1;
	int	  sample2;

   //
   // get waterlevel, accounting for ducking
   //
	pm->waterlevel = 0;
	pm->watertype = 0;

	sample2 = pm->ps->viewheight - MINS_Z;
	sample1 = sample2 * 3 / 4;

   VectorCopy( pm->ps->origin, point );
   point[ 2 ] += MINS_Z + 1.0f;
	cont = pm->pointcontents( point, pm->ps->clientNum );
	if ( cont & MASK_WATER )
	   {
		pm->watertype = cont;
		pm->waterlevel = 1;

      point[ 2 ] = pm->ps->origin[ 2 ] + MINS_Z + sample1;
		cont = pm->pointcontents( point, 0 );
		if ( cont & MASK_WATER )
		   {
			pm->waterlevel = 2;
         point[ 2 ] = pm->ps->origin[ 2 ] + MINS_Z + sample2;
			cont = pm->pointcontents( point, 0 );
			if ( cont & MASK_WATER )
            {
				pm->waterlevel = 3;
            }
		   }
	   }
   }


/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
void PM_CheckDuck
   (
   void
   )

   {
	pm->mins[ 0 ] = MINS_X;
	pm->mins[ 1 ] = MINS_Y;
	pm->mins[ 2 ] = MINS_Z;
	pm->maxs[ 0 ] = MAXS_X;
	pm->maxs[ 1 ] = MAXS_Y;
	pm->maxs[ 2 ] = MAXS_Z;

	pm->ps->viewheight = pm->ps->pm_defaultviewheight;
	
	if ( pm->ps->pm_type == PM_DEAD )
      {
		pm->maxs[ 2 ] = DEAD_MINS_Z;
		return;
	   }

	if ( pm->ps->pm_flags & PMF_DUCKED )
	   {
		
			pm->maxs[ 2 ] = CROUCH_MAXS_Z;
			pm->ps->viewheight = CROUCH_VIEWHEIGHT;
	   }
   }


//-----------------------------------------------------
//
// Name:		PM_CheckCrouchJump
// Class:		None
//
// Description:	Checks to see if the player is ducking and jumping, which is a crouch jump.
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void PM_CheckCrouchJump(void)
{
	//if the player is not on the ground and is ducking,
	//set the crouch jump flag.
	if ( pm->ps->jumped && 
		 ( ( ( pm->ps->pm_flags & PMF_DUCKED ) && ( pm->ps->pm_flags & PMF_TIME_JUMP_START ) ) || 
		   ( !( pm->ps->pm_flags & PMF_TIME_JUMP_START ) && ( pm->ps->pm_flags & PMF_JUMP_HELD ) ) ) )
	{
 		if(pm->ps->crouchjumpset == qfalse)
		{
			pm->ps->pm_flags |= PMF_TIME_CROUCH_JUMP;
			pm->ps->crouchjumpset = qtrue;
		}
	}

	//This really sucks to put this code here, since the 
	//primary function of this procedure is to check if
	//crouch jump is active. Since CheckDuck sets the 
	//default height down if duck is pressed, this moves
	//it back up if the player is crouch jumping
	if(pm->ps->crouchjumpset == qtrue)
	{
		pm->ps->viewheight = pm->ps->pm_defaultviewheight;	
		pm->maxs[ 2 ] = MAXS_Z;
	}

	if(pm->ps->groundPlane)
	{
		pm->ps->crouchjumpset = qfalse;
	}
}


//-----------------------------------------------------
//
// Name:		PM_CrouchJumpMove
// Class:		None
//
// Description:	Allows the player to clear objects a little higher than a normal jump could. 
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void PM_CrouchJumpMove(void)
{
	PM_StepSlideMove( !( pm->ps->pm_flags & PMF_NO_GRAVITY ) );
	pm->ps->velocity[2] += pm->ps->crouchjumpvelocity;

	//turn off the jump crouch flag.
	pm->ps->pm_flags &= ~PMF_TIME_CROUCH_JUMP;
}

//===================================================================

qboolean PM_ShouldCrashLand( void )
{

	
/*	jhefty/jwaters -- another strafe-jump fix
	if (pml.impactSpeed)
		pm->ps->pm_landtime = pm->ps->commandTime + 175;
*/
	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
	{
		//vec3_t diff;
		float delta;

		// We're on the ground, see if we hit it hard enough for a crash land

		//VectorSubtract( pm->ps->velocity, pml.previous_velocity, diff );
		//delta = VectorLength( diff );
		//delta = abs( pm->ps->velocity[ 2 ] - pml.previous_velocity[ 2 ] );
		delta = pm->ps->velocity[ 2 ] - pml.previous_velocity[ 2 ];

		if ( delta < 0.0f || pml.previous_velocity[ 2 ] > -400 )
			return qfalse;

		if ( delta > 600.0f )
		{
			return qtrue;
		}
	}

	return qfalse;
}

/*
=================
PM_CrashLand

Check for hard landings that generate sound events

  fall from 128: 400 = 160000
  fall from 256: 580 = 336400
  fall from 384: 720 = 518400
  fall from 512: 800 = 640000
  fall from 640: 960 =

  damage = deltavelocity*deltavelocity  * 0.0001

=================
*/
void PM_CrashLand
   (
   void
   )

   {
	float delta;
	/* float	dist;
	float	vel;
   float	acc;
	float	t;
	float	a, b, c, den; */
	//vec3_t diff;

	// calculate the exact velocity on landing
	/* dist = pm->ps->origin[ 2 ] - pml.previous_origin[ 2 ];
	vel = pml.previous_velocity[ 2 ];
	acc = -pm->ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

	den =  b * b - 4 * a * c;
	if ( den < 0 )
      {
		return;
	   }

	t = ( -b - sqrt( den ) ) / ( 2 * a );

	delta = vel + t * acc;

	if (delta < -200)
		{
		pm->ps->pm_flags |= PMF_TIME_LAND;
		// don't allow another jump for a little while
		if (delta < -400)
			pm->ps->pm_time = 200;	
		else
			pm->ps->pm_time = 144;
		} */

	// Get the change in speed

	//VectorSubtract( pm->ps->velocity, pml.previous_velocity, diff );
	//delta = VectorLength( diff );
	//delta = abs( pm->ps->velocity[ 2 ] - pml.previous_velocity[ 2 ] );

	delta = pm->ps->velocity[ 2 ] - pml.previous_velocity[ 2 ];

	if ( delta < 0.0f || pml.previous_velocity[ 2 ] > -400 )
		return;

	//delta = delta * delta * 0.0001f;

	// never take falling damage if completely underwater
	if ( pm->waterlevel == 3 )
      {
		return;
	   }

	// reduce falling damage if there is standing water
	if ( pm->waterlevel == 2 )
      {
		delta *= 0.25f;
	   }

	if ( pm->waterlevel == 1 )
      {
		delta *= 0.5f;
	   }

	if ( delta < 1.0f )
      {
		return;
	   }

	pm->landed = qtrue;
	pm->landedVelocity = pml.previous_velocity[2];

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	if ( !( pml.groundTrace.surfaceFlags & SURF_NODAMAGE ) )
	{
		if ( delta > 1500.0f )
		{
			pm->pmoveEvent = EV_FALL_FATAL;
		}
		else if ( delta > 1300.0f )
		{
			pm->pmoveEvent = EV_FALL_VERY_FAR;
		}
		else if ( delta > 1100.0f )
		{
			pm->pmoveEvent = EV_FALL_FAR;
		}
		else if ( delta > 900.0f )
		{
			pm->pmoveEvent = EV_FALL_MEDIUM;
		}
		else if ( delta > 750.0f )
		{
			pm->pmoveEvent = EV_FALL_SHORT;
		}
		else if ( delta > 600.0f )
		{
			pm->pmoveEvent = EV_FALL_VERY_SHORT;
		}
	}

   }

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
void PM_WaterEvents
   (
   void
   )

   {

	if ( ( pm->ps->pm_type == PM_SPECTATOR ) || ( pm->ps->pm_type == PM_SPECTATOR_FOLLOW ) )
		return;


   // FIXME?
	//
	// if just entered a water volume, play a sound
	//
	if ( !pml.previous_waterlevel && pm->waterlevel )
      {
		pm->pmoveEvent = EV_WATER_TOUCH;
	   }

	//
	// if just completely exited a water volume, play a sound
	//
	if ( pml.previous_waterlevel && ! pm->waterlevel )
      {
		pm->pmoveEvent = EV_WATER_LEAVE;
	   }

	//
	// check for head just going under water
	//
	if ( ( pml.previous_waterlevel != 3 ) && ( pm->waterlevel == 3 ) )
      {
		pm->pmoveEvent = EV_WATER_UNDER;
	   }

	//
	// check for head just coming out of water
	//
	if ( ( pml.previous_waterlevel == 3 ) && ( pm->waterlevel != 3 ) )
      {
		pm->pmoveEvent = EV_WATER_CLEAR;
	   }
   }

/*
================
PM_DropTimers
================
*/
void PM_DropTimers
   (
   void
   )

   {
	// drop misc timing counter
	if ( pm->ps->pm_time )
      {
		if ( pml.msec >= pm->ps->pm_time )
         {
			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		   }
      else
         {
			pm->ps->pm_time -= pml.msec;
		   }
	   }
   }



//-----------------------------------------------------
//
// Name:		PM_UpdateLeanIn
// Class:		None
//
// Description:	Calculates the lean in movement
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void PM_UpdateLeanIn( playerState_t* ps, const usercmd_t* cmd )
{
	vec3_t	viewOrigin;
	vec3_t	leanTraceMins;
	vec3_t	leanTraceMaxs;
	vec3_t	leftVector;
	vec3_t	leanTraceEnd;
	vec3_t	playerViewAngles;
	trace_t leanTrace;

	float leanDelta = ps->leanDelta;

	if( cmd->lean > 0 )
	{
		//We are leaning left, so calculate the new lean delta.
		//If lean delta is greater then the max lean delta
		//then set lean delta to be the max.
		leanDelta += ((( float ) pml.msec / ( float ) pm_leaninspeed) * pm_leanmaxdelta);
		if( leanDelta > pm_leanmaxdelta )
			leanDelta = pm_leanmaxdelta;
	}
	else
	{
		//We are leaning right, so calculate the new lean delta.
		//If lean delta is less than the negative max lean delta
		//then set lean delta to be the negative max lean delta.
		leanDelta -= ((( float ) pml.msec / ( float ) pm_leaninspeed) * pm_leanmaxdelta);
		if( leanDelta < -pm_leanmaxdelta )
			leanDelta = -pm_leanmaxdelta;
	}

	//We calculate our lean by scaling our view origin vector by the amount of the lean delta, 
	//then adding the left vector to get the delta amount of the lean.

	//Get the view origin
	VectorCopy( ps->origin, viewOrigin );
	viewOrigin[2] += ps->viewheight;

	//Get the view angles, remove all roll from the view angles.
	VectorCopy( ps->viewangles, playerViewAngles );
	playerViewAngles[ROLL] = 0;

	AngleVectors( ps->viewangles, NULL, leftVector, NULL );
	VectorMA( viewOrigin, leanDelta, leftVector, leanTraceEnd );
	
	
	//Run a trace to check if we collide with any object when we lean.
	VectorSet( leanTraceMins, -5, -5, -4 );
	VectorSet( leanTraceMaxs, 5, 5, 4 );

	if( pm != 0)
	{
		pm->trace(&leanTrace, viewOrigin, leanTraceMins, leanTraceMaxs, 
						leanTraceEnd, ps->clientNum, MASK_PLAYERSOLID, qfalse);

		//Scale the lean delta by the end result of the trace. 
		//This will scale our delta to the object that the trace has hit.
		leanDelta *= leanTrace.fraction;
	}

	ps->leanDelta = leanDelta;
}


//-----------------------------------------------------
//
// Name:		PM_UpdateLeanOut
// Class:		
//
// Description:	Calculates the lean out movement.
//
// Parameters:	ps - the current player state.
//
// Returns:		
//-----------------------------------------------------
void PM_UpdateLeanOut( playerState_t* ps )
{
	float leanDelta = ps->leanDelta;

	if( leanDelta > 0 )
	{
		leanDelta -= (( (float) pml.msec / (float) pm_leanoutspeed) * pm_leanmaxdelta);
		if( leanDelta < 0 )
			leanDelta = 0;
	}
	else if ( leanDelta < 0 )
	{
		leanDelta += (((float)pml.msec / ( float ) pm_leanoutspeed) * pm_leanmaxdelta);
		if( leanDelta > 0)
			leanDelta = 0;
	}

	ps->leanDelta = leanDelta;
}


//-----------------------------------------------------
//
// Name:		PM_UpdateLeanView
// Class:		
//
// Description:	Calculates the lean movement.
//
// Parameters:	ps  - the player state 
//				cmd - the input commands struct.
//
// Returns:		None
//-----------------------------------------------------
void PM_UpdateLeanView( playerState_t* ps, const usercmd_t* cmd )
{
	// We are not leaning, so move back to center position
	if(	cmd->lean == 0 )
	{
		PM_UpdateLeanOut(ps);
	}
	else 
	{
		PM_UpdateLeanIn(ps, cmd);
	}
}


/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void PM_UpdateViewAngles
   (
   playerState_t *ps,
   const usercmd_t *cmd
   )

   {
	short	temp;
	int	i;

	if ( ps->pm_flags & PMF_FROZEN )
      {
      // no view changes at all
		return;
	   }

   /*
	if ( ps->stats[ STAT_HEALTH ] <= 0 )
      {
      // no view changes at all
		return;
	   }
   */

	// circularly clamp the angles with deltas
	for( i = 0; i < 3; i++ )
      {
		temp = cmd->angles[ i ] + ps->delta_angles[ i ];
		if ( i == PITCH )
         {
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 )
            {
				ps->delta_angles[ i ] = 16000 - cmd->angles[ i ];
				temp = 16000;
			   }
         else if ( temp < -16000 )
            {
				ps->delta_angles[ i ] = -16000 - cmd->angles[ i ];
				temp = -16000;
			   }
		   }

		ps->viewangles[ i ] = SHORT2ANGLE( temp );
	   }


	PM_UpdateLeanView( ps, cmd );
   }


void Pmove_GroundTrace
   (
   pmove_t *pmove
   )

   {
   memset (&pml, 0, sizeof(pml));
   pml.msec = 1;
	pml.frametime = 0.001f;
	pm = pmove;
	if ( !( pm->ps->pm_flags & PMF_NO_MOVE ) )
	{
		PM_CheckDuck();
		PM_CheckCrouchJump();
	}

   PM_GroundTrace( qtrue );
   }


/*
================
Pmove

Can be called by either the server or the client
================
*/
void PmoveSingle (pmove_t *pmove)
   {
   vec3_t tempVec;
   qboolean walking;
   vec3_t temp;

	pm = pmove;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;
	pm->landed = qfalse;

	if(pm->ps->groundPlane)
	{
		//only set this to false if we are on the ground. 
		//when jump is hit, this is set to true. This 
		//allows the code to know if the player jumped or not.
		pm->ps->jumped = qfalse;
	}

	if ( pm->ps->stats[STAT_HEALTH] <= 0 )
      {
		pm->tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
	   }

	// adding fake talk balloons
	/* if ( pmove->cmd.buttons & BUTTON_TALK )
      {
		pmove->cmd.buttons = 0;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
		pmove->cmd.lean = 0;
	   } */

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
	if ( pml.msec < 1 )
      {
		pml.msec = 1;
	   }
   else if ( pml.msec > 200 )
      {
		pml.msec = 200;
	   }

	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy( pm->ps->origin, pml.previous_origin );

	// save old velocity for crashlanding
	VectorCopy( pm->ps->velocity, pml.previous_velocity );
	VectorCopy( pm->ps->velocity, temp );

	pml.frametime = pml.msec * 0.001f;

	// update the viewangles
	if (!pm->ps->in_vehicle )
		{
		PM_UpdateViewAngles( pm->ps, &pm->cmd );

		AngleVectors( pm->ps->viewangles, pml.forward, pml.left, pml.up );
	   VectorClear( tempVec );
	   tempVec[ YAW ] = pm->ps->viewangles[ YAW ];
		AngleVectors( tempVec, pml.flat_forward, pml.flat_left, pml.flat_up );
		}

	if ( pm->ps->in_vehicle )
		{
		PM_VehicleMove();
		return;
		}

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}

	if ( pm->ps->pm_type == PM_DEAD )
      {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
		pm->cmd.lean = 0;
	   }

	if ( pm->ps->pm_type == PM_NOCLIP )
      {
		PM_NoclipMove ();
		PM_DropTimers ();
 		return;
	   }

	if ( ( pm->ps->pm_flags & PMF_FROZEN ) || ( pm->ps->pm_flags & PMF_NO_MOVE ) )
      {
		return;		// no movement at all
	   }

	PM_CheckDuck();

	PM_CheckCrouchJump();

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	// set groundentity
	PM_GroundTrace( qfalse );

	if ( pm->ps->pm_type == PM_DEAD )
      {
		PM_DeadMove();
	   }

	PM_DropTimers();

	if ( pm->ps->pm_flags & PMF_FLIGHT )
		// flight powerup doesn't allow jump and has different friction
		PM_FlyMove();
   else if ( pm->ps->pm_flags & PMF_TIME_WATERJUMP )
      {
		PM_WaterJumpMove();
	   }
   else if ( pml.walking )
      {
		// walking on ground
		PM_WalkMove();
	   }
   else if ( pm->waterlevel > 1 )
      {
		// swimming
		PM_WaterMove();
	   }
   else if ( pm->ps->pm_flags & PMF_TIME_STUCKJUMP )
      {
		PM_StuckJumpMove();
	   }
   else if( pm->ps->pm_flags & PMF_TIME_CROUCH_JUMP)
   {
	   PM_CrouchJumpMove();
   }
   else
      {
		// airborne
		if ( !pm->ps->in_vehicle )
			PM_AirMove();
	   }

   walking = pml.walking;

	// set groundentity, watertype, and waterlevel
	PM_GroundTrace( qfalse );
	PM_SetWaterLevel();

   // don't fall down stairs or do really short falls
   if ( !pml.walking && ( walking || ( ( pml.previous_velocity[ 2 ] >= 0.0f ) && ( pm->ps->velocity[ 2 ] <= 0.0f ) ) ) )
      {
	   vec3_t   point;
	   trace_t  trace;

	   point[ 0 ] = pm->ps->origin[ 0 ];
	   point[ 1 ] = pm->ps->origin[ 1 ];
	   point[ 2 ] = pm->ps->origin[ 2 ] - 0.5f;

	   pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask, qtrue );
      if ( ( trace.fraction < 1.0f ) && ( !trace.allsolid ) )
         {
   		VectorCopy( trace.endpos, pm->ps->origin );

         // allow client to smooth out the step
   		pm->stepped = qtrue;

         // requantify the player's position
         PM_GroundTrace( qfalse );
   	   PM_SetWaterLevel();
         }
      }

	// entering / leaving water splashes
	PM_WaterEvents();


	// test stuff
	
	// snap some parts of playerstate to save network bandwidth
	SnapVector( pm->ps->velocity );

	// If the player is off the ground cap his xy velocity so he can't strafe-jump and use other cheats
	//   to go too fast

	if ( !pm->ps->strafeJumpingAllowed && ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) && ( pm->ps->groundEntityNum == ENTITYNUM_WORLD ) )
	{
		float speed;
		vec3_t xyVelocity;

		// Get xy velocity

		//jhefty/jwaters modifications

		VectorCopy( pm->ps->velocity, xyVelocity );
		//xyVelocity[ 2 ] = 0.0f;

		// See if the xy velocity is too fast

		speed = VectorLength( xyVelocity );

		if ( speed > pm->ps->speed )
		{
			// Cap the real velocity

			VectorNormalize( xyVelocity );
			VectorScale( xyVelocity, pm->ps->speed, pm->ps->velocity );


			//pm->ps->velocity[ 0 ] = xyVelocity[ 0 ];
			//pm->ps->velocity[ 1 ] = xyVelocity[ 1 ];
			
		}
	}

	if ( PM_ShouldCrashLand() )
	{
		// just hit the ground
		if ( pm->debugLevel )
		{
			Com_Printf( "%i:Land\n", c_pmove );
		}

		PM_CrashLand();
	}

   }

/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove) {
	int			finalTime;

	finalTime = pmove->cmd.serverTime;

	if ( finalTime < pmove->ps->commandTime ) {
		return;	// should not happen
	}

	if ( finalTime > ( pmove->ps->commandTime + 1000 ) ) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( pmove->ps->commandTime != finalTime ) {
		int		msec;

		msec = finalTime - pmove->ps->commandTime;

		if ( msec > 50 ) {
			msec = 50;
		}
		pmove->cmd.serverTime = pmove->ps->commandTime + msec;
		PmoveSingle( pmove );
		//Com_Printf("Vel: %f\n",pm->ps->velocity[2]);
	}

}
