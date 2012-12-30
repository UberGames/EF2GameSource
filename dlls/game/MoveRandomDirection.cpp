//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/MoveRandomDirection.cpp                 $
// $Revision:: 6                                                              $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//	MoveRandomDirection Implementation
// 
// PARAMETERS:
//
// ANIMATIONS:
//--------------------------------------------------------------------------------

#include "actor.h"
#include "MoveRandomDirection.hpp"
#include <qcommon/gameplaymanager.h>

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, MoveRandomDirection, NULL )
	{
		{ &EV_Behavior_Args,		&MoveRandomDirection::SetArgs		},
		{ NULL, NULL }
	};


MoveRandomDirection::MoveRandomDirection()
{
	anim = "";
	_forever = false;
	_faceEnemy = false;
	_dist = 1024.0f;
	_minDistance = MIN_RANDOM_DIRECTION_DESTINATION;
	_mode = RANDOM_MOVE_ANYWHERE;
	_forever = false;
	_faceEnemy = false;
}

MoveRandomDirection::~MoveRandomDirection()
{
}


//--------------------------------------------------------------
//
// Name:        SetArgs()
// Class:       MoveRandomDirection
//
// Description: Sets Parameters
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
//--------------------------------------------------------------
void MoveRandomDirection::SetArgs( Event *ev )
{
	int parmCount = ev->NumArgs();

	if ( parmCount > 0 ) anim = ev->GetString( 1 );
	if ( parmCount > 1 ) SetDistance( ev->GetFloat( 2 ) );
	if ( parmCount > 2 ) SetMinDistance( ev->GetFloat( 3 ) );
	if ( parmCount > 3 ) SetMode( ev->GetInteger( 4 ) );
	if ( parmCount > 4 ) _forever = ev->GetBoolean( 5 );
	if ( parmCount > 5 ) _faceEnemy = ev->GetBoolean( 6 );
}



//--------------------------------------------------------------
//
// Name:        Begin()
// Class:       MoveRandomDirection
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void MoveRandomDirection::Begin( Actor &self )
{      
	findDestination( self );

	if ( _foundGoodDestination )
		{
		float radius = 16.0f;

		if ( _faceEnemy )
			self.movementSubsystem->setFaceEnemy( true );
		
		setLegAnim( self );
		
		_chase.SetDistance( radius );
		_chase.SetPoint( _destination );
		_chase.SetAnim( anim );
		_chase.Begin( self );

	  	//Setup Torso Anim if appropriate
		if ( !self.torsoBehavior )
			setTorsoAnim( self );	
		}
}



//--------------------------------------------------------------
//
// Name:        Evaluate()
// Class:       MoveRandomDirection
//
// Description: Update for this behavior -- called every server frame
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     BehaviorReturnCode_t
//
//--------------------------------------------------------------
BehaviorReturnCode_t MoveRandomDirection::Evaluate( Actor &self )
{
	BehaviorReturnCode_t result;

	if ( !_foundGoodDestination )
		{
		self.SetAnim( "idle" , NULL , legs );
		if ( _forever )
			Begin( self );
		else
			return BEHAVIOR_FAILED;
		}

 	result = _chase.Evaluate( self );
	 if ( result == BEHAVIOR_SUCCESS )
		{
		if ( _forever )
			Begin( self );
		else
			return BEHAVIOR_SUCCESS;
		}
	
	if ( result != BEHAVIOR_EVALUATING )
		{
		if ( _forever )
			Begin( self );
		else
			return BEHAVIOR_FAILED;
		}		
	
	return BEHAVIOR_EVALUATING;

}



//--------------------------------------------------------------
//
// Name:        End()
// Class:       MoveRandomDirection
//
// Description: Ends this behavior -- cleans things up
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void MoveRandomDirection::End(Actor &self)
{   
	self.SetAnim( "idle" , NULL , legs );
	if ( _faceEnemy )
		self.movementSubsystem->setFaceEnemy( false );

	self.movementSubsystem->setMovingBackwards( false );
}



//--------------------------------------------------------------
// Name:        _chooseRandomDirection()
// Class:       MoveRandomDirection
//
// Description: Picks a random position Vector based on 
//              the _mode
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     Vector
//--------------------------------------------------------------
Vector MoveRandomDirection::_chooseRandomDirection(Actor &self)
{   
	float yaw;
	Vector destination;
	Vector start;
	trace_t trace;

	switch ( _mode )
		{
		case RANDOM_MOVE_IN_FRONT:
			yaw = G_Random( 90.0f ) - 45.0f;         
		break;

		case RANDOM_MOVE_IN_BACK:
			yaw = G_Random( 90.0f ) - 45.0f;
			yaw = AngleNormalize180( yaw + 180);
		break;

		default:
			yaw = G_Random(360.0f);
		break;
		}

	start = self.origin;
	start.z += 16.0f;
	destination = self.angles;   
	destination[YAW] = AngleNormalize180(destination[YAW]);
	destination[YAW] += yaw;
	destination.AngleVectors( &destination );

	Vector debug;
	debug = self.angles;
	debug.AngleVectors ( &debug );
	debug *= 296.0f;
	debug.z = start.z;
	debug += start;

	destination *= _dist;
	destination += start;
	destination.z = start.z;
	trace = G_Trace( start , self.mins, self.maxs, destination, &self, self.edict->clipmask, false, "MoveRandomDirection: _chooseRandomDirection" );

	Vector size1 = self.mins;
	Vector size2 = self.maxs;
	Vector actorSize;
	
	size1.z = 0;
	size2.z = 0;

	actorSize = size2 - size1;
	float mungeValue = actorSize.length();
	Vector startToDest = trace.endpos - start;
	float dist = startToDest.length();
	dist-= mungeValue;

	startToDest.normalize();
	startToDest *= dist;
	startToDest = startToDest + start;
	

	//destination = trace.endpos;
	destination = startToDest;
	//G_DebugLine( start , destination , 1.0 , 1.0 , 1.0 , 1.0);
	return destination;
}



//--------------------------------------------------------------
// Name:        _getDistanceToDestination()
// Class:       MoveRandomDirection
//
// Description: Returns the the distance to the destination
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     float
//--------------------------------------------------------------
float MoveRandomDirection::_getDistanceToDestination(Actor &self)
{
	Vector selfToDestination;

	selfToDestination = _destination - self.origin;

	return selfToDestination.length();
}



//--------------------------------------------------------------
// Name:        SetDistance()
// Class:       MoveRandomDirection
//
// Description: Mutator
//
// Parameters:  float dist
//
// Returns:     None
//--------------------------------------------------------------
void MoveRandomDirection::SetDistance( float dist )
{
	_dist = dist;
}



//--------------------------------------------------------------
// Name:        SetAnim()
// Class:       MoveRandomDirection
//
// Description: Mutator
//
// Parameters:  const str &moveAnim
//
// Returns:     None
//--------------------------------------------------------------
void MoveRandomDirection::SetAnim( const str &moveAnim )
{
	anim = moveAnim;
}



//--------------------------------------------------------------
// Name:        SetMode()
// Class:       MoveRandomDirection
//
// Description: Mutator
//
// Parameters:  unsigned int mode
//
// Returns:     None
//--------------------------------------------------------------
void MoveRandomDirection::SetMode( unsigned int mode )
{
	_mode = mode;
}



//--------------------------------------------------------------
// Name:        SetMinDistance()
// Class:       MoveRandomDirection
//
// Description: Mutator
//
// Parameters:  float dist
//
// Returns:      None
//--------------------------------------------------------------
void MoveRandomDirection::SetMinDistance( float dist )
{
	_minDistance = dist;
}

void MoveRandomDirection::findDestination( Actor &self )
{
	int attempts;

	_destination = _chooseRandomDirection(self);

	attempts = 0;
	_foundGoodDestination = true;
	while ( _getDistanceToDestination(self) < _minDistance && attempts < 5 )
		{
		_destination = _chooseRandomDirection(self);
		_foundGoodDestination = false;
		attempts += 1;
		if ( _getDistanceToDestination(self) > _minDistance )
			_foundGoodDestination = true;
		}
}

void MoveRandomDirection::setLegAnim( Actor &self )
{
	str newAnim;

	if ( _faceEnemy )
		{
		Vector selfToDestinationAngles = _destination - self.origin;
		Vector animAngles = self.movementSubsystem->getAnimDir();
		float yawDiff;

		selfToDestinationAngles = selfToDestinationAngles.toAngles();
		animAngles = animAngles.toAngles();
		yawDiff = AngleNormalize180(selfToDestinationAngles[YAW] - animAngles[YAW] );

		if ( yawDiff >= -25.0 && yawDiff <= 25.0 )
			{
			newAnim = self.combatSubsystem->GetAnimForMyWeapon( "CombatWalk" );
			}

		if ( yawDiff >= -135.0 && yawDiff <= -25.0 )
			{
			newAnim = self.combatSubsystem->GetAnimForMyWeapon( "CombatRStrafe" );
			}

		if ( yawDiff >= 25.0 && yawDiff <= 135.0f )
			{
			newAnim = self.combatSubsystem->GetAnimForMyWeapon( "CombatLStrafe" );
			}
		
		if ( yawDiff >= 135.0 && yawDiff <= 180.0f )
			{
			newAnim = self.combatSubsystem->GetAnimForMyWeapon( "CombatBackpedal" );
			self.movementSubsystem->setMovingBackwards( true );
			}

		if ( yawDiff <= -135.0 && yawDiff >= -180.0 )
			{
			newAnim = self.combatSubsystem->GetAnimForMyWeapon( "CombatBackpedal" );
			self.movementSubsystem->setMovingBackwards( true );
			}
			
		}

	if ( newAnim.length() )
		anim = newAnim;
}

void MoveRandomDirection::setTorsoAnim( Actor &self )
{
	if ( self.enemyManager->HasEnemy() )
		_torsoAnim = self.combatSubsystem->GetAnimForMyWeapon( "CombatGunIdle" );
	else
		_torsoAnim = self.combatSubsystem->GetAnimForMyWeapon( "IdleGunIdle" );

	if ( _torsoAnim.length() )
		{
		self.SetAnim( _torsoAnim, NULL , torso );
		}
}

