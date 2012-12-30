//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/g_phys.h                                      $
// $Revision:: 9                                                              $
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
// Global header file for g_phys.cpp
//

#ifndef __G_PHYS_H__
#define __G_PHYS_H__

#include "g_local.h"
#include "entity.h"

typedef enum
	{
	STEPMOVE_OK,
	STEPMOVE_BLOCKED_BY_ENTITY,
	STEPMOVE_BLOCKED_BY_WORLD,
	STEPMOVE_BLOCKED_BY_WATER,
	STEPMOVE_BLOCKED_BY_FALL,
	STEPMOVE_BLOCKED_BY_DOOR,
	STEPMOVE_STUCK
	} stepmoveresult_t;

// movetype values
typedef enum
	{
	MOVETYPE_NONE,			// never moves
   MOVETYPE_STATIONARY, // never moves but does collide agains push objects
	MOVETYPE_NOCLIP,		// origin and angles change with no interaction
	MOVETYPE_PUSH,			// no clip to world, push on box contact
	MOVETYPE_STOP,			// no clip to world, stops on box contact
	MOVETYPE_WALK,			// gravity
	MOVETYPE_STEP,			// gravity, special edge handling
	MOVETYPE_FLY,
	MOVETYPE_TOSS,			// gravity
	MOVETYPE_FLYMISSILE,	// extra size to monsters
	MOVETYPE_BOUNCE,
   MOVETYPE_SLIDE,
   MOVETYPE_ROPE,
	MOVETYPE_GIB,
   MOVETYPE_VEHICLE
	} movetype_t;

void		G_RunEntity( Entity *ent );
void		G_Impact( Entity *e1, const trace_t *trace );
qboolean	G_PushMove( Entity *pusher, const Vector &move, const Vector &amove );
void     G_CheckWater( Entity *ent );

//------------------------- CLASS ------------------------------
//
// Name:		Angle
// Base Class:	None
//
// Description:	Angles and floats are not the same thing and should
// not be treated as such. In the long run we need to make this a 
// real class. For now it is just a parameter for Trajectory
//
// Method of Use:	Parameter type for Trajectory
//
//--------------------------------------------------------------
class Angle
{
public:
	Angle( const float value):_value( value ) {}
	operator float () { return _value; }
	operator float const () const { return _value; }
	float GetValue( void ) const { return _value; }
	float & GetValue( void ) { return _value; }

private:
	float	_value;
};

//------------------------- CLASS ------------------------------
//
// Name:		Trajectory
// Base Class:	None
//
// Description:	This class computes trajectory information based 
// on various known inputs. Each known input has a specilized ctor
// that computes all other values from those that are given
//
// Method of Use:	This class should not be aggegrated. It is intended
// for local/parameter use 
//
//--------------------------------------------------------------
class Trajectory
{
public:
						Trajectory( const Vector &launchPoint, const Vector &targetPoint, const Angle launchAngle, const float gravity );
						Trajectory( const Vector &launchPoint, const Vector &targetPoint, const float initialSpeed, const float gravity, const bool useHighTrajectory = false );
	const Vector &		GetLaunchPoint( void ) const { return _launchPoint; }
	const Vector &		GetTargetPoint( void ) const { return _targetPoint; }
	float				GetLaunchAngle( void ) const { return _launchAngle; }
	float				GetTravelTime( void ) const { return _travelTime; }
	const Vector &		GetInitialVelocity( void ) const { return _initialVelocity; }
	float				GetGravity( void ) const { return _gravity; }

private:
	Vector				_launchPoint;
	Vector				_targetPoint;
	float				_launchAngle;
	float				_travelTime;
	Vector				_initialVelocity;
	float				_gravity;
};
#endif /* g_phys.h */
