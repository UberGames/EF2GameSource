//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/stationaryFireCombatEX.cpp                 $
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
//	StationaryFireCombatEX Implementation
// 
// PARAMETERS:
//
// ANIMATIONS:
//--------------------------------------------------------------------------------

#include "actor.h"
#include "stationaryFireCombatEX.hpp"
#include <qcommon/gameplaymanager.h>

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, StationaryFireCombatEX, NULL )
	{
		{ &EV_Behavior_Args,		&StationaryFireCombatEX::SetArgs	},
		{ &EV_Behavior_AnimDone,	&StationaryFireCombatEX::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		StationaryFireCombatEX()
// Class:		StationaryFireCombatEX
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
StationaryFireCombatEX::StationaryFireCombatEX()
{	
	_endFireTime		= 0.0f;
	_endAimTime			= 0.0f;
}	

//--------------------------------------------------------------
// Name:		~StationaryFireCombatEX()
// Class:		StationaryFireCombatEX
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
StationaryFireCombatEX::~StationaryFireCombatEX()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       StationaryFireCombatEX
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void StationaryFireCombatEX::SetArgs( Event *ev )
{		
	_stance			= ev->GetString( 1 );
	_aimTimeMin		= ev->GetFloat( 2 );
	_aimTimeMax		= ev->GetFloat( 3 );
	_fireTimeMin	= ev->GetFloat( 4 );
	_fireTimeMax	= ev->GetFloat( 5 );
	_forceAttack	= ev->GetBoolean( 6 );	

}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       StationaryFireCombatEX
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void StationaryFireCombatEX::AnimDone( Event * )
{
	_animDone = true;
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       StationaryFireCombatEX
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void StationaryFireCombatEX::Begin( Actor &self )
{       
	init( self );
	transitionToState ( STATIONARY_FIRE_AIM );
}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       StationaryFireCombatEX
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t StationaryFireCombatEX::Evaluate( Actor & )
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
// Class:       StationaryFireCombatEX
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void StationaryFireCombatEX::End(Actor &)
{ 
	if ( !_self )
		return;

	_fireWeapon.End(*_self);
	_self->SetAnim( _aimAnim , NULL , torso );
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		StationaryFireCombatEX
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombatEX::transitionToState( StationaryFireStates_t state )
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
// Class:		StationaryFireCombatEX
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombatEX::setInternalState( StationaryFireStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		init()
// Class:		StationaryFireCombatEX
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombatEX::init( Actor &self )
{
	_self = &self;	
	_animDone = false;
	_canAttack = true;

	_aimAnim		= "idle";
	_preFireAnim	= "idle";
	_fireAnim		= "idle";
	_postFireAnim	= "idle";

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(self.getArchetype()) )
		return;
		
	str objname = self.combatSubsystem->GetActiveWeaponArchetype();	
	objname = "Hold" + objname;

	if ( gpm->hasProperty(objname, "Idle") )
		_aimAnim = gpm->getStringValue( objname , "Idle" );
	
	if ( gpm->hasProperty( objname , "PreFire" ) )
		_preFireAnim = gpm->getStringValue( objname , "PreFire" );

	if ( gpm->hasProperty( objname , "Fire" ) )
		_fireAnim = gpm->getStringValue( objname , "Fire" );

	if ( gpm->hasProperty( objname , "PostFire" ) )
		_postFireAnim = gpm->getStringValue( objname , "PostFire" );

	_stance = self.GetStateVar( _stance );

	if ( !_stance.length() )
		_stance = "idle";

	updateEnemy();
}

//--------------------------------------------------------------
// Name:		think()
// Class:		StationaryFireCombatEX
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombatEX::think()
{
}

//--------------------------------------------------------------
// Name:		StationaryFireCombat()
// Class:		StationaryFireCombatEX
//
// Description:	Sets up our Attack State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombatEX::setupStateAttack()
{
	faceEnemy();
	_endFireTime = level.time + G_Random(_fireTimeMax - _fireTimeMin) + _fireTimeMin;

	if ( _self->combatSubsystem->CanAttackEnemy() || _forceAttack )
		{	
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );
		}

}

//--------------------------------------------------------------
// Name:		evaluateStateAttack()
// Class:		StationaryFireCombatEX
//
// Description:	Evaluates our Attack State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t StationaryFireCombatEX::evaluateStateAttack()
{
	BehaviorReturnCode_t result;
	_rotateToEntity.Evaluate( *_self );
	
	if ( _self->combatSubsystem->CanAttackEnemy() || _forceAttack )
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
// Class:		StationaryFireCombatEX
//
// Description:	Failure Handler for Attack State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombatEX::failureStateAttack( const str &failureReason )
{
	SetFailureReason( failureReason );
	_fireWeapon.End( *_self );
}

//--------------------------------------------------------------
// Name:		setupStatePause()
// Class:		StationaryFireCombatEX
//
// Description:	Sets up our Pause State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombatEX::setupStateAim()
{
	faceEnemy();
	_endAimTime = level.time + G_Random(_aimTimeMax - _aimTimeMin ) + _aimTimeMin;
	_self->SetAnim( _aimAnim , NULL , torso );
	_self->SetAnim( _stance , NULL , legs );

}

//--------------------------------------------------------------
// Name:		evaluateStatePause()
// Class:		StationaryFireCombatEX
//
// Description:	Evaluates our Pause State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t StationaryFireCombatEX::evaluateStateAim()
{
	_rotateToEntity.Evaluate( *_self );
	if ( level.time > _endAimTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}


//--------------------------------------------------------------
// Name:		failureStatePause()
// Class:		StationaryFireCombatEX
//
// Description:	Failure Handler for our Pause State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombatEX::failureStateAim( const str &failureReason )
{
	SetFailureReason( failureReason );
}


void StationaryFireCombatEX::setupStatePreFire()
{
	if ( _self->combatSubsystem->CanAttackEnemy() || _forceAttack )
		{
		_animDone = false;
		_self->SetAnim( _preFireAnim , EV_Actor_NotifyBehavior , torso );
		}
	else
		_canAttack = false;
}

BehaviorReturnCode_t StationaryFireCombatEX::evaluateStatePreFire()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	if ( !_canAttack )
		return BEHAVIOR_FAILED;

	return BEHAVIOR_EVALUATING;
}

void StationaryFireCombatEX::failureStatePreFire( const str &failureReason )
{
	Q_UNUSED(failureReason);
}

void StationaryFireCombatEX::setupStatePostFire()
{
	_animDone = false;
	_self->SetAnim( _postFireAnim , EV_Actor_NotifyBehavior , torso );

}

BehaviorReturnCode_t StationaryFireCombatEX::evaluateStatePostFire()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_FAILED;

}

void StationaryFireCombatEX::failureStatePostFire( const str &failureReason )
{
	Q_UNUSED(failureReason);
}

//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		StationaryFireCombatEX
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void StationaryFireCombatEX::updateEnemy()
{
	//
	// First, we try and get our current enemy.  If we don't have one
	// then we ask the enemyManager to evaluate and try again.
	// If that fails, then we look at our _forceAttack status.
	// If we are forcing and attack, then that means we want to
	// fire pretty much no matter what... So if we don't have a 
	// currentEnemy, then I'm going to see if the Player is a valid
	// target, and set that to be the current enemy
	Entity *currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !currentEnemy )
			{
			Player* player;
			player = GetPlayer( 0 );

			if ( player && GetSelf()->enemyManager->Hates(player) && _forceAttack )
				{
				currentEnemy = player;
				GetSelf()->enemyManager->SetCurrentEnemy( currentEnemy );
				}
			else
				{
				SetFailureReason( "STATIONARY_FIRE_FAILED::updateEnemy -- No Enemy" );
				transitionToState( STATIONARY_FIRE_FAILED );
				return;
				}
			}
			
		}

	_currentEnemy = currentEnemy;
	_self->turnTowardsEntity( _currentEnemy, 0.0f );	
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
void StationaryFireCombatEX::faceEnemy()
{
	_rotateToEntity.SetEntity( _currentEnemy );
	_rotateToEntity.SetTurnSpeed( 30.0f );
	_rotateToEntity.Begin( *_self );	

}






















