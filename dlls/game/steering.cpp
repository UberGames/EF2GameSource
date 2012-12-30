//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/steering.cpp                                  $
// $Revision:: 29                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/01/02 5:47p                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Steering behaviors for AI.
//

#include "_pch_cpp.h"
#include "steering.h"
#include "actor.h"

//-----------------------------------------------------------------------------
//
//  Steering Class Definition
//
//-----------------------------------------------------------------------------
CLASS_DECLARATION( Listener, Steering, NULL )
{
	{ NULL, NULL }
};

//-----------------------------------------------------------------------------
Steering::Steering() :
_steeringForce(vec_zero),
_origin(vec_zero),
_moveDirection(vec_zero),
_maxSpeed(320)
{
}

//-----------------------------------------------------------------------------
void Steering::ShowInfo(Actor &self)
{
   gi.Printf( "steeringforce: ( %f, %f, %f )\n", _steeringForce.x, _steeringForce.y, _steeringForce.z );
   gi.Printf( "origin: ( %f, %f, %f )\n", _origin.x, _origin.y, _origin.z );
   gi.Printf( "movedir: ( %f, %f, %f )\n", _moveDirection.x, _moveDirection.y, _moveDirection.z );
   gi.Printf( "maxspeed: %f\n", _maxSpeed );
}

//-----------------------------------------------------------------------------
void Steering::Begin(Actor &self)
{
}

//-----------------------------------------------------------------------------
const Steering::ReturnValue Steering::Evaluate(Actor &self)
{
	return Steering::FAILED;
}

//-----------------------------------------------------------------------------
void Steering::End(Actor &self)
{
}

//-----------------------------------------------------------------------------
void Steering::DrawForces(void)
{
	G_Color3f( 0.3, 0.5, 1 );
	G_BeginLine();
	G_Vertex( _origin );
	G_Vertex( _origin + _steeringForce * FRAMETIME );
	G_EndLine();
	
	G_Color3f( 1, 0, 1 );
	G_BeginLine();
	G_Vertex( _origin );
	G_Vertex( _origin + _moveDirection * _maxSpeed * FRAMETIME );
	G_EndLine();
}

//-----------------------------------------------------------------------------
void Steering::ResetForces(void)
{
	_steeringForce = vec_zero;
}

//-----------------------------------------------------------------------------
const Vector &	Steering::GetSteeringForce( void )  const
{
	return _steeringForce;
}

//-----------------------------------------------------------------------------
void Steering::SetSteeringForce( const Vector &steeringForce )
{
	Vector tempSteeringForce( steeringForce );
	tempSteeringForce.EulerNormalize();
	_steeringForce=tempSteeringForce;
}

//-----------------------------------------------------------------------------
const Vector & Steering::GetMoveDirection(void) const
{
	return _moveDirection;
}

//-----------------------------------------------------------------------------
void Steering::SetMoveDirection(const Vector &dir)
{
	_moveDirection = dir;
}

//-----------------------------------------------------------------------------
const float Steering::GetMaxSpeed(void) const
{
	return _maxSpeed;
}

//-----------------------------------------------------------------------------
void Steering::SetMaxSpeed(float speed)
{
	_maxSpeed = speed;
}

//-----------------------------------------------------------------------------
const Vector & Steering::GetPosition(void) const
{
	return _origin;
}

//-----------------------------------------------------------------------------
void Steering::SetPosition(const Vector &pos)
{
	_origin = pos;
}

//-----------------------------------------------------------------------------
//
//  Jump Class Definition
//
//-----------------------------------------------------------------------------

CLASS_DECLARATION( Steering, Jump, NULL )
{
	{ NULL, NULL }
};

//-----------------------------------------------------------------------------
Jump::Jump():
_launchAngle( 0.0f ),
_endtime( 0.0f ),
_state( 0.0f )
{
}

//-----------------------------------------------------------------------------
void Jump::SetGoal( const Vector &goalPosition )
{
	_goal = goalPosition;
}

//-----------------------------------------------------------------------------
void Jump::SetEntity(Entity *entity_to_jump_to)
{
	if ( entity_to_jump_to )
		_goal = entity_to_jump_to->origin;
}

//-----------------------------------------------------------------------------
void Jump::Begin(Actor &self)
{
	self.SetAnim( "jump", EV_Anim_Done );
	
	float traveltime = self.movementSubsystem->JumpTo( _goal, _launchAngle );
	_endtime = traveltime + level.time;
	
	self.last_jump_time = _endtime;
	
	_state = 0;
}
//-----------------------------------------------------------------------------
const Steering::ReturnValue Jump::Evaluate(Actor &self)
{
	if (self.GetActorFlag( ACTOR_FLAG_ANIM_DONE ))
	{
		_animdone = true;
	}
	else
	{
		_animdone = false;
	}
	self.SetActorFlag( ACTOR_FLAG_ANIM_DONE, false );
	
	switch( _state )
	{
	case 0:
		_state = 1;
		// this is here so that we at least hit this function at least once
		// this gaves the character the chance to leave the ground, nulling out
		// self.groundentity
		break;
	case 1:
		if ( _animdone ) 
		{
			if ( self.animate->HasAnim( "fall" ) )
			{
				_animdone = false;
				self.SetAnim( "fall", EV_Anim_Done );
				_state = 2;
			}
		}
		if ( self.groundentity )
			_state = 2;
		break;
	case 2:
		//
		// wait for the character to hit the ground
		//
		if ( self.groundentity )
		{
			_state = 3;
			//
			// if we have an anim, we go to state 3
			//
			if ( self.animate->HasAnim( "land" ) )
			{
				_animdone = false;
				self.SetAnim( "land", EV_Anim_Done );
				_state = 4;
			}
			else
			{
				return Steering::EVALUATING;
			}
		}
		break;
	case 3:
		//
		// we are on the ground and waiting to timeout
		//
		if ( level.time > _endtime )
			return Steering::SUCCESS;
		break;
	case 4:
		//
		// we are on the ground and waiting for our landing animation to finish
		//
		if ( _animdone )
		{
			return Steering::SUCCESS;
		}
		break;
	}
	
	return Steering::EVALUATING;
}

//-----------------------------------------------------------------------------
void Jump::End(Actor &self)
{
	self.SetAnim( "idle" );
}
