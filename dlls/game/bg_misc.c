//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/bg_misc.c                                     $
// $Revision:: 4                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/11/02 3:17a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Misc functions that are used in the client and server game dlls

// included in both game dll and client

#include "q_shared.h"
#include "bg_public.h"


char *gDLLNeededString = "xa37dd45ffe100bfffcc9753aabac325f07cb3fa231144fe2e33ae4783feead2b8a73ff021fac326df0ef9753ab9cdf6573ddff0312fab0b0ff39779eaff312x";

/*
================
EvaluateTrajectory

================
*/
void EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result ) {
	float		deltaTime;
	float		phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorCopy( tr->trBase, result );
		break;
	case TR_LINEAR:
		deltaTime = (float)( atTime - tr->trTime ) * 0.001f;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = sin( deltaTime * M_PI * 2.0 );
		VectorMA( tr->trBase, phase, tr->trDelta, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > ( tr->trTime + tr->trDuration ) ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = (float)( atTime - tr->trTime ) * 0.001f;	// milliseconds to seconds
		if ( deltaTime < 0.0f ) {
			deltaTime = 0.0f;
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = (float)( atTime - tr->trTime ) * 0.001f;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5f * DEFAULT_GRAVITY * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	default:
		Com_Error( ERR_DROP, "EvaluateTrajectory: unknown trType: %i", tr->trType );
		break;
	}
}

/*
================
EvaluateTrajectoryDelta

================
*/
void EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result ) {
	float	deltaTime;
	float	phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorClear( result );
		break;
	case TR_LINEAR:
		VectorCopy( tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = (float)( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = cos( deltaTime * M_PI * 2.0 );	// derivative of sin = cos
		phase *= 0.5f;
		VectorScale( tr->trDelta, phase, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > ( tr->trTime + tr->trDuration ) ) {
			VectorClear( result );
			return;
		}
		VectorCopy( tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = (float)( atTime - tr->trTime ) * 0.001f;	// milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= DEFAULT_GRAVITY * deltaTime;		// FIXME: local gravity...
		break;
	default:
		Com_Error( ERR_DROP, "EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime );
		break;
	}
}

