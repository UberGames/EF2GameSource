//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/stationaryFireCombat.cpp                  $
// $Revision:: 7                                                              $
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
//	StationaryFireCombat Implementation
// 
// PARAMETERS:
//
// ANIMATIONS:
//--------------------------------------------------------------------------------

#include "actor.h"
#include "stationaryFireCombat.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, StationaryFireCombat, NULL )
	{
		{ &EV_Behavior_Args,		&StationaryFireCombat::SetArgs	},
		{ &EV_Behavior_AnimDone,	&StationaryFireCombat::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		StationaryFireCombat()
// Class:		StationaryFireCombat
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
StationaryFireCombat::StationaryFireCombat()
{	
	_endFireTime		= 0.0f;
	_endAimTime			= 0.0f;
}	

//--------------------------------------------------------------
// Name:		~StationaryFireCombat()
// Class:		StationaryFireCombat
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
StationaryFireCombat::~StationaryFireCombat()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       StationaryFireCombat
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void StationaryFireCombat::SetArgs( Event *ev )
{		
	_aimAnim		= ev->GetString( 1 );
	_preFireAnim	= ev->GetString( 2 );
	_fireAnim		= ev->GetString( 3 );	
	_postFireAnim	= ev->GetString( 4 );
	_stance			= ev->GetString( 5 );
	_aimTimeMin		= ev->GetFloat( 6 );
	_aimTimeMax		= ev->GetFloat( 7 );
	_fireTimeMin	= ev->GetFloat( 8 );
	_fireTimeMax	= ev->GetFloat( 9 );	
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       StationaryFireCombat
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void StationaryFireCombat::AnimDone( Event *ev )
{
	_animDone = true;
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       StationaryFireCombat
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void StationaryFireCombat::Begin( Actor &self )
{       
	init( self );
	transitionToState ( STATIONARY_FIRE_AIM );
}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       StationaryFireCombat
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t StationaryFireCombat::Evaluate( Actor &self )
{
	BehaviorReturnCode_t stateResult;

	think();
	
	switch ( _state )
		{
		//---------------------------------------------------------------------
		case STATIONARY_FIRE_AIM:
		//---------------------------------------------------------------------		
			stateResult = evaluateStateAim();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				updateEnemy();
				if ( _preFireAnim.length() )
					transitionToState( STATIONARY_FIRE_PRE_FIRE );
				else
					transitionToState( STATIONARY_FIRE_ATTACK );
				}
				
		break;

		//---------------------------------------------------------------------
		case STATIONARY_FIRE_PRE_FIRE:
		//---------------------------------------------------------------------
			stateResult = evaluateStatePreFire();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
					transitionToState( STATIONARY_FIRE_ATTACK );	
				}

			if ( stateResult == BEHAVIOR_FAILED )
				{
				transitionToState( STATIONARY_FIRE_FAILED );
				}
			
		break;

		//---------------------------------------------------------------------
		case STATIONARY_FIRE_ATTACK:
		//---------------------------------------------------------------------		
			stateResult = evaluateStateAttack();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				_fireWeapon.End( *_self );

				if ( _postFireAnim.length() )
					transitionToState( STATIONARY_FIRE_POST_FIRE );
				else
					transitionToState( STATIONARY_FIRE_SUCCESS );
				}

			if ( stateResult == BEHAVIOR_FAILED )
				{
				_fireWeapon.End( *_self );

				if ( _postFireAnim.length() )
					transitionToState( STATIONARY_FIRE_POST_FIRE );
				else
					transitionToState( STATIONARY_FIRE_SUCCESS );
				}

		break;


		//---------------------------------------------------------------------
		case STATIONARY_FIRE_POST_FIRE:
		//---------------------------------------------------------------------
			stateResult = evaluateStatePostFire();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
					transitionToState( STATIONARY_FIRE_SUCCESS );	
				}
		break;

		//---------------------------------------------------------------------
		case STATIONARY_FIRE_SUCCESS:
		//---------------------------------------------------------------------
			return BEHAVIOR_SUCCESS;

		break;


		//---------------------------------------------------------------------
		case STATIONARY_FIRE_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;

		break;


		}
	

	return BEHAVIOR_EVALUATING;   

}



//--------------------------------------------------------------
// Name:        End()
// Class:       StationaryFireCombat
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void StationaryFireCombat::End(Actor &self)
{ 
	if ( !_self )
		return;

	_fireWeapon.End(*_self);
	_self->SetAnim( _aimAnim , NULL , torso );

	_self->ClearTorsoAnim();
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		StationaryFireCombat
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::transitionToState( StationaryFireStates_t state )
{	
	switch ( state )
		{	
		case STATIONARY_FIRE_AIM:
			setupStateAim();
			setInternalState( state , "STATIONARY_FIRE_AIM" );
		break;

		case STATIONARY_FIRE_PRE_FIRE:
			setupStatePreFire();
			setInternalState( state , "STATIONARY_FIRE_PRE_FIRE" );
		break;

		case STATIONARY_FIRE_ATTACK:
			setupStateAttack();
			setInternalState( state , "STATIONARY_FIRE_ATTACK" );
		break;

		case STATIONARY_FIRE_POST_FIRE:
			setupStatePostFire();
			setInternalState( state , "STATIONARY_FIRE_POST_FIRE" );
		break;

		case STATIONARY_FIRE_SUCCESS:
			setInternalState( state , "STATIONARY_FIRE_SUCCESS" );
		break;

		case STATIONARY_FIRE_FAILED:
			setInternalState( state , "SUPPRESSION_FIRE_FAILED" );
		break;
		}
	
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		StationaryFireCombat
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::setInternalState( StationaryFireStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		init()
// Class:		StationaryFireCombat
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::init( Actor &self )
{
	_self = &self;	
	_animDone = false;
	_canAttack = true;
	updateEnemy();
}

//--------------------------------------------------------------
// Name:		think()
// Class:		StationaryFireCombat
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::think()
{
}

//--------------------------------------------------------------
// Name:		StationaryFireCombat()
// Class:		setupStateAttack
//
// Description:	Sets up our Attack State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::setupStateAttack()
{
	faceEnemy();
	_endFireTime = level.time + G_Random(_fireTimeMax - _fireTimeMin) + _fireTimeMin;

	if ( _self->combatSubsystem->CanAttackEnemy() )
		{	
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );
		}

}

//--------------------------------------------------------------
// Name:		evaluateStateAttack()
// Class:		StationaryFireCombat
//
// Description:	Evaluates our Attack State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t StationaryFireCombat::evaluateStateAttack()
{
	BehaviorReturnCode_t result;
	_rotateToEntity.Evaluate( *_self );
	
	if ( _self->combatSubsystem->CanAttackEnemy() )
		result = _fireWeapon.Evaluate( *_self );
	else
		{
		_fireWeapon.End( *_self );
		result = BEHAVIOR_FAILED;
		}

	if ( result == BEHAVIOR_FAILED )
		failureStateAttack( "StationaryFireCombat::evaluateStateAttack -- FAILED" );

	if ( level.time > _endFireTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAttack()
// Class:		StationaryFireCombat
//
// Description:	Failure Handler for Attack State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::failureStateAttack( const str &failureReason )
{
	SetFailureReason( failureReason );
	_fireWeapon.End( *_self );
}

//--------------------------------------------------------------
// Name:		setupStatePause()
// Class:		StationaryFireCombat
//
// Description:	Sets up our Pause State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::setupStateAim()
{
	faceEnemy();
	_endAimTime = level.time + G_Random(_aimTimeMax - _aimTimeMin ) + _aimTimeMin;
	_self->SetAnim( _aimAnim , NULL , torso );
	_self->SetAnim( _stance , NULL , legs );

}

//--------------------------------------------------------------
// Name:		evaluateStatePause()
// Class:		StationaryFireCombat
//
// Description:	Evaluates our Pause State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t StationaryFireCombat::evaluateStateAim()
{
	_rotateToEntity.Evaluate( *_self );
	if ( level.time > _endAimTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}


//--------------------------------------------------------------
// Name:		failureStatePause()
// Class:		StationaryFireCombat
//
// Description:	Failure Handler for our Pause State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::failureStateAim( const str &failureReason )
{
	SetFailureReason( failureReason );
}


void StationaryFireCombat::setupStatePreFire()
{
	if ( _self->combatSubsystem->CanAttackEnemy() )
		{
		_animDone = false;
		_self->SetAnim( _preFireAnim , EV_Actor_NotifyBehavior , torso );
		}
	else
		_canAttack = false;
}

BehaviorReturnCode_t StationaryFireCombat::evaluateStatePreFire()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	if ( !_canAttack )
		return BEHAVIOR_FAILED;

	return BEHAVIOR_EVALUATING;
}

void StationaryFireCombat::failureStatePreFire( const str &failureReason )
{
}

void StationaryFireCombat::setupStatePostFire()
{
	_animDone = false;
	_self->SetAnim( _postFireAnim , EV_Actor_NotifyBehavior , torso );

}

BehaviorReturnCode_t StationaryFireCombat::evaluateStatePostFire()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_FAILED;

}

void StationaryFireCombat::failureStatePostFire( const str &failureReason )
{
}

//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		StationaryFireCombat
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::updateEnemy()
{
	Entity *currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !currentEnemy )
			{
			SetFailureReason( "STATIONARY_FIRE_FAILED::updateEnemy -- No Enemy" );
			transitionToState( STATIONARY_FIRE_FAILED );
			return;
			}
			
		}

	_currentEnemy = currentEnemy;
	//_self->turnTowardsEntity( _currentEnemy, 0.0f );	
}

//--------------------------------------------------------------
// Name:		faceEnemy()
// Class:		StationaryFireCombat
//
// Description:	Sets up the Rotate Component
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombat::faceEnemy()
{
	_rotateToEntity.SetEntity( _currentEnemy );
	_rotateToEntity.SetTurnSpeed( 30.0f );
	_rotateToEntity.Begin( *_self );	

}






















