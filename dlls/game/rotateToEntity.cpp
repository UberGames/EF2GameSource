//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/rotateToEntity.cpp                         $
// $Revision:: 12                                                             $
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
// RotateToEntity Behavior Implementation.
//	-- Rotates the actor to face an entity.  If no entity or entityType is specified, then it
//	   will rotate the actor to face its currentEnemy
//	
// PARAMETERS:
//		Optional turnspeed
//		Optional entityType to turn to, either "enemy" (default) or "player"
//
// ANIMATIONS:
//		Rotation Animation : Optional
//--------------------------------------------------------------------------------

#include "actor.h"
#include "rotateToEntity.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, RotateToEntity, NULL )
	{
		{ &EV_Behavior_Args,			&RotateToEntity::SetArgs },
		{ NULL, NULL }
	};



//--------------------------------------------------------------
// Name:		RotateToEntity()
// Class:		RotateToEntity
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
RotateToEntity::RotateToEntity()
{
	_anim = "";
	_turnspeed = 10.0f;
	_ent = NULL;
	_entityType = "enemy";
}


//--------------------------------------------------------------
// Name:		~RotateToEntity()
// Class:		RotateToEntity
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
RotateToEntity::~RotateToEntity()
{
}

//--------------------------------------------------------------
//
// Name:        SetArgs()
// Class:       RotateToEntity
//
// Description: Sets Variables based on arguments inside the event
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
//--------------------------------------------------------------
void RotateToEntity::SetArgs( Event *ev )
	{
	// The Parmeters we're catching now are for turn speed 
	// and Animation ( both of which are optional ).  We do not
	// catch an entity here, because, I don't think there's a way
	// to pass in an entity from the state machine.  However, other
	// behaviors CAN set the entity, so the functionality is here
	// We also now take an entityType to turn to ("player" or "enemy")

	int parmCount= ev->NumArgs();

	if ( parmCount > 0 ) // Grab the turnSpeed
		_turnspeed = ev->GetFloat( 1 );

	if ( parmCount > 1 ) // Grab the animation
		_anim = ev->GetString( 2 );

	if ( parmCount > 2 ) // Grab the type of entity to find
		_entityType = ev->GetString( 3 );

	}



//--------------------------------------------------------------
//
// Name:        Begin()
// Class:       RotateToEntity
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void RotateToEntity::Begin( Actor &self )
	{       
	
	// Parameters should be set BEFORE we call begin, either through
	// the SetArgs() or explicitly set through accessors
	if ( !_ent )
		{
		if ( _entityType == "enemy" )
			{
			self.enemyManager->FindHighestHateEnemy();
			_ent = self.enemyManager->GetCurrentEnemy();
			}
		else
			{
			Entity *player = (Entity*)GetPlayer(0);

			if ( player && !(player->flags & FlagNotarget) )
				{
				_ent = player;
				}
			}
		}
  
	// Set our Animation if Appropriate
	if ( _anim.length() && self.animate->HasAnim( _anim.c_str() ) )
		self.SetAnim( _anim );

	// Set up our turnSpeed
	_oldTurnSpeed = self.movementSubsystem->getTurnSpeed();
	self.movementSubsystem->setTurnSpeed( _turnspeed );
	
	}


//--------------------------------------------------------------
//
// Name:        Evaluate()
// Class:       RotateToEntity
//
// Description: Update for this behavior -- called every server frame
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     BehaviorReturnCode_t
//
//--------------------------------------------------------------
BehaviorReturnCode_t RotateToEntity::Evaluate( Actor &self )
	{
	Vector dir;
	Vector selfToEntity;
	
	float yawDiff;

	if ( !_ent )
		{
		SetFailureReason( "RotateToEntity: Has no Entity to Rotate To" );
		//self.SetAnim( "idle" );
		return BEHAVIOR_FAILED;
		}

	//
	// Previously, we were just steering towards _ent->centroid.  This worked, 
	// however it also has the unfortunate side-effect of pitching the actor
	// if the entity it is rotating to is above it.  This, of course, looks
	// like poo... So, for an interim fix, we're going to set our target
	// destination's z value to the actor's centroid z value, this, should
	// prevent the pitch problem
	//
	Vector entityPos;
	entityPos = _ent->centroid;
	entityPos.z = self.centroid.z;

	if ( self.IsEntityAlive( _ent ) )
		{		
		dir = self.movementSubsystem->getMoveDir();
		self.movementSubsystem->Accelerate( 
			self.movementSubsystem->SteerTowardsPoint( entityPos, vec_zero, dir, 1.0f)
			);
		}

	//
	// Here, again, I replaced the _ent->centroid with the z-value modified entityPos
	//
	selfToEntity = entityPos - self.centroid;
	selfToEntity = selfToEntity.toAngles();	

	yawDiff = selfToEntity[YAW] - self.angles[YAW];

	yawDiff = AngleNormalize180(yawDiff);
	if ( yawDiff > -1.5 && yawDiff < 1.5 )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;

     
   }



//--------------------------------------------------------------
//
// Name:        End()
// Class:       RotateToEntity
//
// Description: Ends this behavior -- cleans things up
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void RotateToEntity::End(Actor &self)
{
	self.movementSubsystem->setTurnSpeed( _oldTurnSpeed );
}

