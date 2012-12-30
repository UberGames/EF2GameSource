//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/generalCombatWithMeleeWeapon.cpp           $
// $Revision:: 16                                                             $
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
//	GeneralCombatWithMeleeWeapon Implementation
//	-- GCWMW will run the actor towards its enemy until it is within melee range.
//		it will work to keep the actor rotated to face its enemy and play the
//		specified attack animations.  There is also rudimentary support for changing
//		"posture" which will allow the actor to duck and lean out of the way.  Additionally
//		there is very minimal support for blocking... Though this will really need to 
//		be fleshed out for it to work.
//
// PARAMETERS:
//		Primary Torso Anim ( the Idle Animation )	-- Required
//		Attack Anim									-- Required
//		MaxDistanceToEngage							-- Optional
//		MeleeDistance								-- Optional
//		Rush Anim									-- Optional
//		Allow Rush Failure							-- Optional
//		Rotation Anim								-- Optional
//		Strafe Chance								-- Optional
//		Block Chance								-- Optional
//		Attack Chance								-- Optional
//		Posture Change Chance						-- Optional	
//
// ANIMATIONS:
//		Primary Torso Animation ( Idle ) : Parameter
//		Attack Animation                 : Parameter		
//		Rotation Animation				 : Parameter -- Defaults to "rotate"
//		Rush Animation					 : Parameter -- Defaults to "run"
//		"strafe_left"					 : TIKI Requirement -- If Strafing
//		"strafe_right"					 : TIKI Requirement -- If Strafing
//		"roll_left"						 : TIKI Requirement -- If Strafing
//		"roll_right"					 : TIKI Requirement -- If Strafing
//		"block"							 : TIKI Requirement -- If Blocking
//		"lean_left_dual_stand"			 : TIKI Requirement -- If Changing Posture
//		"lean_right_dual_stand"			 : TIKI Requirement -- If Changing Posture
//		"lean_left_dual_stand_end"		 : TIKI Requirement -- If Changing Posture
//		"lean_right_dual_stand_end"		 : TIKI Requirement -- If Changing Posture
//		"lean_left_dual_stand_start"	 : TIKI Requirement -- If Changing Posture
//		"lean_right_dual_stand_start"    : TIKI Requirement -- If Changing Posture
//--------------------------------------------------------------------------------

#include "actor.h"
#include "generalCombatWithMeleeWeapon.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, GeneralCombatWithMeleeWeapon, NULL )
	{
		{ &EV_Behavior_Args,			&GeneralCombatWithMeleeWeapon::SetArgs  },
		{ &EV_Behavior_AnimDone,		&GeneralCombatWithMeleeWeapon::AnimDone }, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		GeneralCombatWithMeleeWeapon()
// Class:		GeneralCombatWithMeleeWeapon
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GeneralCombatWithMeleeWeapon::GeneralCombatWithMeleeWeapon()
{
	_primaryTorsoAnim		= "idle";      
	_attackAnim				= "idle";
	_rotationAnim           = "rotate";
	_rushAnim				= "run";
	_maxDistanceToEngage	= 0.0f;
	_meleeDistance			= 128.0f;
	_strafeChance			= 0.50f;
	_blockChance			= 0.50f;
	_attackChance			= 0.85f;
	_postureChangeChance	= 0.0f;
	_allowRushFailure		= false;
}

//--------------------------------------------------------------
// Name:		~GeneralCombatWithMeleeWeapon()
// Class:		GeneralCombatWithMeleeWeapon
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GeneralCombatWithMeleeWeapon::~GeneralCombatWithMeleeWeapon()
{
}

//--------------------------------------------------------------
// Name:         SetArgs()
// Class:        GeneralCombatWithMeleeWeapon
//
// Description:  Sets the arguments of the behavior 
//
// Parameters:   Event *ev
//
// Returns:      None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::SetArgs ( Event *ev)
{
	// Required Parameters
	_primaryTorsoAnim = ev->GetString( 1 );
	_attackAnim       = ev->GetString( 2 );

	// Optional Parameters
	if ( ev->NumArgs() > 2 )
		_maxDistanceToEngage = ev->GetFloat( 3 );

	if ( ev->NumArgs() > 3 )
		_meleeDistance = ev->GetFloat( 4 );

	if ( ev->NumArgs() > 4 )
		_rushAnim = ev->GetString( 5 );

	if ( ev->NumArgs() > 5 )
		_allowRushFailure = ev->GetBoolean( 6 );

	if ( ev->NumArgs() > 6 ) 
		_rotationAnim = ev->GetString( 7 );

	if ( ev->NumArgs() > 7 )
		_strafeChance = ev->GetFloat( 8 );

	if ( ev->NumArgs() > 8 )
		_blockChance = ev->GetFloat( 9 );

	if ( ev->NumArgs() > 9 )
		_attackChance = ev->GetFloat( 10 );

	if ( ev->NumArgs() > 10 )
		_postureChangeChance = ev->GetFloat( 11 );


}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::AnimDone( Event *ev )
{
	switch ( _state )
		{
		case GENERAL_COMBAT_MELEE_STRAFE:
			_strafeComponent.AnimDone( ev );         
			_nextRushAttemptTime = 0.0f;
		break;

		case GENERAL_COMBAT_MELEE_ATTACK:
			_self->SetAnim( _primaryTorsoAnim );
			_state = GENERAL_COMBAT_MELEE_SELECT_STATE;
			_nextRushAttemptTime = 0.0f;
		break;

		/*
		case GENERAL_COMBAT_MELEE_CHANGE_POSTURE:
			switch ( _self->movementSubsystem->getPostureState() )
				{
				case POSTURE_TRANSITION_STAND_TO_LEAN_LEFT_DUAL:
					_self->movementSubsystem->setPostureState( POSTURE_LEAN_LEFT_DUAL );                        
					_self->SetAnim( "lean_left_dual_stand" , EV_Actor_NotifyBehavior );                        
				break;

				case POSTURE_TRANSITION_STAND_TO_LEAN_RIGHT_DUAL:
					_self->movementSubsystem->setPostureState( POSTURE_LEAN_RIGHT_DUAL );                        
					_self->SetAnim( "lean_right_dual_stand" , EV_Actor_NotifyBehavior );
				break;

				case POSTURE_LEAN_LEFT_DUAL:
					_self->movementSubsystem->setPostureState( POSTURE_TRANSITION_LEAN_LEFT_DUAL_TO_STAND );
					_self->SetAnim( "lean_left_dual_stand_end" , EV_Actor_NotifyBehavior );
				break;

				case POSTURE_LEAN_RIGHT_DUAL:
					_self->movementSubsystem->setPostureState( POSTURE_TRANSITION_LEAN_RIGHT_DUAL_TO_STAND );
					_self->SetAnim( "lean_right_dual_stand_end" , EV_Actor_NotifyBehavior );
				break;

				case POSTURE_TRANSITION_LEAN_LEFT_DUAL_TO_STAND:
					_self->movementSubsystem->setPostureState( POSTURE_STAND );
					setTorsoAnim( *_self );
					_state = GENERAL_COMBAT_MELEE_SELECT_STATE;  
					_nextPostureChangeTime = level.time + G_Random ( 2.0f ) + 0.5f;
				break;

				case POSTURE_TRANSITION_LEAN_RIGHT_DUAL_TO_STAND:
					_self->movementSubsystem->setPostureState( POSTURE_STAND );
					setTorsoAnim( *_self );;
					_state = GENERAL_COMBAT_MELEE_SELECT_STATE;   
					_nextPostureChangeTime = level.time + G_Random ( 2.0f ) + 0.5f;
				break;
				}
				
		break;
		*/

		}
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Begins the Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::Begin( Actor &self )
{
	init( self );

	setUpRotate( self );  
	setHeadWatchTarget( self );
}



//--------------------------------------------------------------
// Name:        init()
// Class:       GeneralCombatWithRangedWeapon
//
// Description: Initializes memeber variables
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::init( Actor &self )
{
	_self = &self;
	_state = GENERAL_COMBAT_MELEE_SELECT_STATE;

	_nextRotateTime			= 0.0f;
	_nextStrafeAttemptTime	= 0.0f;
	_nextRushAttemptTime	= 0.0f;
	_exitHoldTime			= 0.0f;
	_exitBlockTime			= 0.0f;
	_nextPostureChangeTime	= 0.0f;	
}



//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Returns True Or False, and is run every frame
//              that the behavior
//
// Parameters:  Actor &self
//
// Returns:     True or False
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithMeleeWeapon::Evaluate ( Actor &self )
{      
	Entity *currentEnemy;

	// Make sure our rotate and headwatch targets are up to date
	if ( _state != GENERAL_COMBAT_MELEE_ATTACK && _state != GENERAL_COMBAT_MELEE_CHANGE_POSTURE )
		setUpRotate( self );        
		
	faceEnemy( self );
	setHeadWatchTarget( self );


	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if (currentEnemy && _maxDistanceToEngage && !self.WithinDistance( currentEnemy, _maxDistanceToEngage))
		{
		self.enemyManager->ClearCurrentEnemy();
		SetFailureReason( "My Current Enemy is farther away than my _maxDistanceToEngage\n" );
		return BEHAVIOR_FAILED; // I may have an enemy, but he's too far away.  Ignore him.
		}
 
	if ( _state == GENERAL_COMBAT_MELEE_FAILED )
		return BEHAVIOR_FAILED;

	if ( currentEnemy && !self.WithinDistance( currentEnemy , _meleeDistance ) && level.time > _nextRushAttemptTime )
		{
		setupRushEnemy( self );
		_state = GENERAL_COMBAT_MELEE_RUSH_ENEMY;      
		}

	switch ( _state )
		{
		case GENERAL_COMBAT_MELEE_SELECT_STATE:
			//setTorsoAnim( self ); // Breaks the initial attack anim
			selectState( self );
		break;

		case GENERAL_COMBAT_MELEE_RUSH_ENEMY:
			rushEnemy( self );
		break;

		case GENERAL_COMBAT_MELEE_STRAFE:
			setTorsoAnim( self );
			strafe(self);
			setTorsoAnim( self );
		break;

		case GENERAL_COMBAT_MELEE_ATTACK:      
			attack( self );
		break;

		case GENERAL_COMBAT_MELEE_BLOCK:
			block( self );
		break;

		case GENERAL_COMBAT_MELEE_CHANGE_POSTURE:
			changePosture(self);
		break;

		case GENERAL_COMBAT_MELEE_HOLD:
			setTorsoAnim( self );
			hold( self );
		break;

		case GENERAL_COMBAT_MELEE_FAILED:
			return BEHAVIOR_FAILED;
		}

	return BEHAVIOR_EVALUATING; 
}



//--------------------------------------------------------------
// Name:         End()
// Class:        GeneralCombatWithMeleeWeapon
//
// Description:  Ends the Behavior
//
// Parameters:   Actor &self
//
// Returns:      None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::End ( Actor &self	)
{
}



//--------------------------------------------------------------
// Name:        setUpRotate
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Sets up the Rotate Component Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::setUpRotate( Actor &self )
{
	Entity *currentEnemy;
	currentEnemy = NULL;

	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if ( !currentEnemy ) 
		return;

	_rotate.SetEntity( currentEnemy );  
}



//--------------------------------------------------------------
// Name:        faceEnemy
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Evaluates the Rotate Component Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::faceEnemy( Actor &self )
{
	BehaviorReturnCode_t rotateResult;	

	//We set the _nextRotateTime after we get a successul rotate
	//evaluation... This is give our headwatch a chance to kick
	//into action... If we just always spin to face the enemy
	//we'll never see the headwatch.
	if ( level.time < _nextRotateTime )
		return;

	_rotate.Begin( self );
	if ( _state == GENERAL_COMBAT_MELEE_ATTACK )
		_rotate.SetAnim( _rotationAnim );      
		
	rotateResult = _rotate.Evaluate( self );

	if ( rotateResult == BEHAVIOR_SUCCESS )
		_nextRotateTime = level.time + .75f;
}



//--------------------------------------------------------------
// Name:        setTorsoAnim()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Sets the animation for the actor's torso
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::setTorsoAnim( Actor &self )
{
	Event *torsoAnimEvent;
	torsoAnimEvent = new Event( EV_Torso_Anim_Done );
 
	self.SetAnim( _primaryTorsoAnim , torsoAnimEvent , torso );

}



//--------------------------------------------------------------
// Name:        setHeadWatchTarget()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Sets the headwatch target for the actor's 
//              headwatcher object
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::setHeadWatchTarget( Actor &self )
{
	Entity *currentEnemy;
	currentEnemy = NULL;

	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if ( !currentEnemy ) 
		return;
   
	self.SetHeadWatchTarget( currentEnemy );
}



//--------------------------------------------------------------
// Name:        setupStrafe()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Sets ourselves up to execute the STRAFE state
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     true or false
//--------------------------------------------------------------
bool GeneralCombatWithMeleeWeapon::setupStrafe( Actor &self )
{
	_strafeComponent.SetMode( Strafe::STRAFE_RANDOM );
	_strafeComponent.Begin( self );

	return true;
}



//--------------------------------------------------------------
// Name:        strafe()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Evaluation State for STRAFE
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::strafe( Actor &self )
{
	BehaviorReturnCode_t result;

	result = _strafeComponent.Evaluate( self );

	if ( result == BEHAVIOR_FAILED )
		strafeFailed( self );
	else if ( result == BEHAVIOR_SUCCESS )
		_state = GENERAL_COMBAT_MELEE_SELECT_STATE;
}



//--------------------------------------------------------------
// Name:        setupStrafeFailed()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Failure Handler for _setupStrafe()
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::strafeFailed( Actor &self )
{
	_nextStrafeAttemptTime = level.time + G_Random( 1.0 ) + 3.0f;
	_state = GENERAL_COMBAT_MELEE_SELECT_STATE;
}



//--------------------------------------------------------------
// Name:        setupRushEnemy
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Sets up the GotoEntity Component Behavior
//
// Parameters:  Actor &self
//
// Returns:     true or false
//--------------------------------------------------------------
bool GeneralCombatWithMeleeWeapon::setupRushEnemy( Actor &self )
{
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if ( !currentEnemy )
		return false;

	_nextRushAttemptTime = level.time + G_Random( 0.25 ) + 1.25f;
	_rush.SetAnim ( _rushAnim );
	_rush.SetEntity( self, currentEnemy );
	_rush.SetDistance ( 96.0f );
	_rush.Begin( self );


	return true;
}



//--------------------------------------------------------------
// Name:        setupRushEnemyFailed()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Failure Handler for _setupRushEnemy
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::setupRushEnemyFailed ( Actor &self )
{
 	if ( !_allowRushFailure )
		{
		SetFailureReason( "RushToEnemy Failed" );
		_state = GENERAL_COMBAT_MELEE_FAILED;
		return;
		}

	_nextRushAttemptTime = level.time + G_Random( 0.25 ) + 1.25f;
	_state = GENERAL_COMBAT_MELEE_SELECT_STATE;   
}



//--------------------------------------------------------------
// Name:        rushEnemy()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Evaluates the GotoEntity Component Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::rushEnemy ( Actor &self )
{
	BehaviorReturnCode_t rushResult;

	rushResult = _rush.Evaluate( self );

	Entity* currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( self.WithinDistance( currentEnemy , _meleeDistance ) )
		rushResult = BEHAVIOR_SUCCESS;

	if ( rushResult == BEHAVIOR_SUCCESS )
		{
		_state = GENERAL_COMBAT_MELEE_SELECT_STATE;   
		return;
		}
   
	if ( rushResult == BEHAVIOR_FAILED )
		setupRushEnemyFailed( self );

}



//--------------------------------------------------------------
// Name:        selectState()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Selects the state for the behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::selectState( Actor &self )
{
	float chance;
	Entity* currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	// If we don't have an enemy, then let's ask our enemy manager
	// to try and find one
	if ( !currentEnemy )
		self.enemyManager->FindHighestHateEnemy();

	// If we still don't have an enemy, then we need to hold
	if ( !currentEnemy )
		{
		if ( !setupHold( self ))
			return;

		_state = GENERAL_COMBAT_MELEE_HOLD;
		return;
		}

	chance = G_Random();
	if ( self.checkincomingmeleeattack() && chance < _blockChance )
		{
		_state = GENERAL_COMBAT_MELEE_BLOCK;

		if ( !setupBlock( self ) )
			setupBlockFailed( self );
		return;
		}

	chance = G_Random();
	if ( currentEnemy && self.enemyManager->InEnemyLineOfFire() && chance < _strafeChance && self.WithinDistance( currentEnemy , _meleeDistance ) )
		{
		_state = GENERAL_COMBAT_MELEE_STRAFE;

		if ( !setupStrafe( self ) )
			strafeFailed(self);

		return;      
		}




	//
	// The Change Posture stuff is commented out because it looks really weak right now
	// I need to keep it here though, so that when the animations look better or I figure out
	// how to better utilize them it will be easy to re-enable
	//
	//if ( currentEnemy && chance < _postureChangeChance && level.time > _nextPostureChangeTime && self.WithinDistance( currentEnemy , _meleeDistance ) )
	//   {
	//   _state = GENERAL_COMBAT_MELEE_CHANGE_POSTURE;
	//
	//   if ( !_setupChangePosture(self) )
	//      _setupChangePostureFailed(self);
	//
	//   return;
	//   }

	chance = G_Random();
	if ( currentEnemy && chance < _attackChance && self.WithinDistance( currentEnemy , _meleeDistance ) )
		{
		_state = GENERAL_COMBAT_MELEE_ATTACK;

		if (!setupAttack( self ))
			setupAttackFailed( self );

		return;
		}

	setupHold( self );
	_state = GENERAL_COMBAT_MELEE_HOLD;

}



//--------------------------------------------------------------
// Name:        setupAttack
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Sets up the behavior to enter the Attack state
//
// Parameters:  Actor &self
//
// Returns:     true or false
//--------------------------------------------------------------
bool GeneralCombatWithMeleeWeapon::setupAttack( Actor &self )
{   
	self.animate->ClearTorsoAnim();
	self.SetAnim( _attackAnim , EV_Actor_NotifyBehavior );
	return true;
}



//--------------------------------------------------------------
// Name:        setupAttackFailed()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Failure Handler for _setupAttack()
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::setupAttackFailed( Actor &self )
{
	_state = GENERAL_COMBAT_MELEE_SELECT_STATE;   
}



//--------------------------------------------------------------
// Name:        attack()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Does Nothing right now, here in case need for 
//              for expansion, and to keep with precedent
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::attack( Actor &self )
{
	// The transition back to SELECT_STATE is
	// handled by AnimDone
}



//--------------------------------------------------------------
// Name:        setupHold()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Set ourselves up to execute the HOLD state
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     true or false
//--------------------------------------------------------------
bool GeneralCombatWithMeleeWeapon::setupHold( Actor &self )
{
	self.SetAnim( _primaryTorsoAnim );   
	_exitHoldTime = level.time + G_Random ( 0.15f ) + 0.25f;   

	return true;
}



//--------------------------------------------------------------
// Name:        hold()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Evaluation State for HOLD
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::hold( Actor &self )
	{
	if ( level.time > _exitHoldTime )
		_state = GENERAL_COMBAT_MELEE_SELECT_STATE;
	}



//--------------------------------------------------------------
// Name:        setupHoldFailed()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Failure Handler for _setupHold
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::setupHoldFailed( Actor &self )
{
	//if we get here, we got a real bad problem
	self.SetAnim( _primaryTorsoAnim );
	_state = GENERAL_COMBAT_MELEE_SELECT_STATE;
}



//--------------------------------------------------------------
// Name:        setupBlock()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Sets the behavior up to enter the Block State
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
bool GeneralCombatWithMeleeWeapon::setupBlock( Actor &self )
{
	self.SetAnim( "block" );
	_exitBlockTime = level.time + G_Random ( 1.00f ) + 0.25f;  
	return true;
}



//--------------------------------------------------------------
// Name:        _setupBlockFailed()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Failure Handler for _setupBlock()
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::setupBlockFailed( Actor &self )
{
	//if we get here, we got a real bad problem
	self.SetAnim( _primaryTorsoAnim );
	_state = GENERAL_COMBAT_MELEE_SELECT_STATE;   
}



//--------------------------------------------------------------
// Name:        block()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Main Function for the Block state
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::block( Actor &self )
{
	//if ( !self.checkincomingmeleeattack() ) // Old condition, do we need this anymore?
	if ( level.time > _exitBlockTime )
		_state = GENERAL_COMBAT_MELEE_SELECT_STATE; 
}



//--------------------------------------------------------------
// Name:        setupChangePosture()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Sets ourselves up to execute the CHANGE_POSTURE state
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     true or false
//--------------------------------------------------------------
bool GeneralCombatWithMeleeWeapon::setupChangePosture( Actor &self )
{
	float chance;
	chance = G_Random();

	/*
	if ( chance < .5 )
		{
		_postureTransitionAnim = "lean_left_dual_stand_start";
		_nextPostureState = POSTURE_TRANSITION_STAND_TO_LEAN_LEFT_DUAL;
		}
	else
		{
		_postureTransitionAnim = "lean_right_dual_stand_start";
		_nextPostureState = POSTURE_TRANSITION_STAND_TO_LEAN_RIGHT_DUAL;      
		}

	self.SetAnim( _postureTransitionAnim , EV_Actor_NotifyBehavior );        
	self.movementSubsystem->setPostureState( (PostureStates_t)_nextPostureState );   
*/
	return true;
}



//--------------------------------------------------------------
// Name:        changePosture()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Evaluation State for CHANGE_POSTURE state
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::changePosture( Actor &self )
{
	// Transitions handled in AnimDone
}



//--------------------------------------------------------------
// Name:        _setupChangePostureFailed()
// Class:       GeneralCombatWithMeleeWeapon
//
// Description: Failure Handler for _setupChangePosture()
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithMeleeWeapon::setupChangePostureFailed( Actor &self )
{
	//if we get here, we got a real bad problem
	 self.SetAnim( _primaryTorsoAnim );
	 _state = GENERAL_COMBAT_MELEE_SELECT_STATE;   
}

