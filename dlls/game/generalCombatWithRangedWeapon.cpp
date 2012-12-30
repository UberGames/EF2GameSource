//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/generalCombatWithRangedWeapon.cpp          $
// $Revision:: 17                                                             $
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
//	GeneralCombatWithRangedWeapon Implementation
//	-- GCWRW will Strafe, Duck, Roll and fire a weapon at an enemy.  Also, it will
//		retreat and approach based on distance from the enemy.
// 
// PARAMETERS:
//
//
// ANIMATIONS:
//
//--------------------------------------------------------------------------------

#include "actor.h"
#include "generalCombatWithRangedWeapon.hpp"

extern Event EV_PostureChanged_Completed;

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, GeneralCombatWithRangedWeapon, NULL )
	{
		{ &EV_Behavior_Args,			&GeneralCombatWithRangedWeapon::SetArgs			},
		{ &EV_Behavior_AnimDone,		&GeneralCombatWithRangedWeapon::AnimDone		}, 
		{ &EV_PostureChanged_Completed, &GeneralCombatWithRangedWeapon::PostureDone		},
		{ NULL, NULL }
	};

//--------------------------------------------------------------
// Name:		GeneralCombatWithRangedWeapon()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GeneralCombatWithRangedWeapon::GeneralCombatWithRangedWeapon()
{
	_self = NULL;
	_movementAnim	= "walk";
	_torsoAnim		= "idle";
	_fireAnim		= "idle";
	_approachDist	= 500;
	_retreatDist	= 250;
	_strafeChance	= 1.0;
	_fireTimeMin	= 1.0;
	_fireTimeMax	= 1.5;
	_pauseTimeMin	= 1.0;
	_pauseTimeMax	= 1.5;
}

//--------------------------------------------------------------
// Name:		GeneralCombatWithRangedWeapon()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GeneralCombatWithRangedWeapon::~GeneralCombatWithRangedWeapon()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       GeneralCombatWithRangedWeapon
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::SetArgs( Event *ev )
{
	if ( ev->NumArgs() >  0 ) _movementAnim	= ev->GetString(  1 );
	if ( ev->NumArgs() >  1 ) _torsoAnim	= ev->GetString(  2 );
	if ( ev->NumArgs() >  2 ) _fireAnim		= ev->GetString(  3 );
	if ( ev->NumArgs() >  3 ) _approachDist	= ev->GetFloat (  4 );
	if ( ev->NumArgs() >  4 ) _retreatDist	= ev->GetFloat (  5 );
	if ( ev->NumArgs() >  5 ) _strafeChance	= ev->GetFloat (  6 );	
	if ( ev->NumArgs() >  6 ) _fireTimeMin	= ev->GetFloat (  7 );
	if ( ev->NumArgs() >  7 ) _fireTimeMax	= ev->GetFloat (  8 );
	if ( ev->NumArgs() >  8 ) _pauseTimeMin	= ev->GetFloat (  9 );
	if ( ev->NumArgs() >  9 ) _pauseTimeMax	= ev->GetFloat ( 10 );

}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       GeneralCombatWithRangedWeapon
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::AnimDone( Event *ev )
{
	switch ( _state )
		{
		case GC_WRW_STRAFE:			
			_strafeComponent.AnimDone( ev );			
		break;
		}
}


//--------------------------------------------------------------
// Name:		PostureDone()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Handles a Posture Done Event
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::PostureDone( Event *ev )
{
	_finishedPostureTransition = true;
}

//--------------------------------------------------------------
// Name:        Begin()
// Class:       GeneralCombatWithRangedWeapon
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::Begin( Actor &self )
{       
	init( self );
}



//--------------------------------------------------------------
// Name:        init()
// Class:       GeneralCombat
//
// Description: Initializes memeber variables
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::init( Actor &self )
{
	_self = &self;
	_finishedPostureTransition		= true;
	_randomAdvanceLockOut			= false;
	_randomRetreatLockOut			= false;
	_nextRotateTime					= 0.0f;
	_nextStrafeAttemptTime			= 0.0f;
	_nextPostureChange				= 0.0f;
	_nextClearRetreatLockOutTime	= 0.0f;
	_nextClearAdvanceLockOutTime	= 0.0f;
	_nextFireTime					= 0.0f;
	_nextPauseTime					= 0.0f;
	_randomAdvanceFailures			= 0;
	_randomRetreatFailures			= 0;

	transitionToState(GC_WRW_STAND);	
	updateEnemy();
}



//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       GeneralCombatWithRangedWeapon
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::Evaluate( Actor &self )
{
	BehaviorReturnCode_t stateResult;
	str	currentPostureState = _self->postureController->getCurrentPostureName();	
	float distToEnemy = self.enemyManager->GetDistanceFromEnemy();

	think();
	switch ( _state )	
	{	
	//---------------------------------------------------------------------		
	case GC_WRW_CHANGE_POSTURE_TO_ADVANCE:
	//---------------------------------------------------------------------		
		stateResult = evaluateStateChangePostureToAdvance();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_ADVANCE );		
	break;

	//---------------------------------------------------------------------		
	case GC_WRW_CHANGE_POSTURE_TO_RETREAT:
	//---------------------------------------------------------------------		
		stateResult = evaluateStateChangePostureToRetreat();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_RETREAT );
	break;

	//---------------------------------------------------------------------		
	case GC_WRW_ADVANCE:
	//---------------------------------------------------------------------
		if ( !checkShouldApproach( distToEnemy ) )
			{
			transitionToState( GC_WRW_STAND );
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateStateAdvance();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_STAND );						

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_ADVANCE_FIRING );

	break;

	//---------------------------------------------------------------------
	case GC_WRW_ADVANCE_FIRING:
	//---------------------------------------------------------------------
		if ( !checkShouldApproach( distToEnemy ) )
			{
			transitionToState( GC_WRW_STAND );
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateStateAdvanceFiring();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_STAND );	

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_ADVANCE );
	break;

	//---------------------------------------------------------------------
	case GC_WRW_RETREAT:
	//---------------------------------------------------------------------
		if ( !checkShouldRetreat( distToEnemy ) )
			{
			transitionToState( GC_WRW_STAND );
			_self->movementSubsystem->setMovingBackwards( false );
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateStateRetreat();
		if ( stateResult == BEHAVIOR_FAILED )
			{
			transitionToState( GC_WRW_STAND );
			_self->movementSubsystem->setMovingBackwards( false );
			}

		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			_self->movementSubsystem->setMovingBackwards( false );
			transitionToState( GC_WRW_RETREAT_FIRING );			
			}

	break;

	//---------------------------------------------------------------------
	case GC_WRW_RETREAT_FIRING:
	//---------------------------------------------------------------------
		if ( !checkShouldRetreat( distToEnemy ) )
			{
			_self->movementSubsystem->setMovingBackwards( false );
			transitionToState( GC_WRW_STAND );
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateStateRetreatFiring();
		if ( stateResult == BEHAVIOR_FAILED )
			{
			transitionToState( GC_WRW_STAND );
			_self->movementSubsystem->setMovingBackwards( false );
			}

		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			_self->movementSubsystem->setMovingBackwards( false );
			transitionToState( GC_WRW_RETREAT );
			}
	break;

	//---------------------------------------------------------------------
	case GC_WRW_STRAFE:
	//---------------------------------------------------------------------
		stateResult = evaluateStateStrafe();

		//It's perfectly understandable that strafe might
		//fail, so instead of failing the behavior entirely
		//we'll just change states
		if ( stateResult != BEHAVIOR_EVALUATING )
			{
			if ( currentPostureState == "DUCK" )
				transitionToState( GC_WRW_DUCKED );

			if ( currentPostureState == "STAND" )
				transitionToState( GC_WRW_STAND );
			}		

	break;

	//---------------------------------------------------------------------
	case GC_WRW_CHANGE_POSTURE_DUCK:
	//---------------------------------------------------------------------
		stateResult = evaluateStateChangePostureDuck();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_DUCKED );
	break;

	//---------------------------------------------------------------------
	case GC_WRW_DUCKED:	
	//---------------------------------------------------------------------	

	//
	// First Check if we are within our _approachDist.  If 
	// not, then we need to close in on our enemy
	//
	if ( checkShouldApproach( distToEnemy ) )
		{
		transitionToState( GC_WRW_CHANGE_POSTURE_TO_ADVANCE );
		return BEHAVIOR_EVALUATING;
		}

	//
	// Now Check if wer are within our _retreatDist.  If 
	// we are, then we need to retreat in on our enemy
	//
	if ( checkShouldRetreat( distToEnemy ) )
		{
		transitionToState( GC_WRW_CHANGE_POSTURE_TO_RETREAT );
		return BEHAVIOR_EVALUATING;
		}

	//
	// Lets check if we need to strafe
	//
	if ( checkShouldStrafe( distToEnemy ) )
		{
		transitionToState( GC_WRW_STRAFE );
		return BEHAVIOR_EVALUATING;
		}
	

	//
	//Now let's check if we need to change posture
	//
	if ( checkShouldChangePosture( distToEnemy ) )
		{
		transitionToState( GC_WRW_CHANGE_POSTURE_STAND );
		return BEHAVIOR_EVALUATING;
		}

	//
	//Well we don't have anything else to do, so let's evaluate our state
	//
	stateResult = evaluateRotate();

	if ( stateResult == BEHAVIOR_SUCCESS )
		{
		stateResult = evaluateStateStand();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_DUCKED_FIRING );

		}

	break;

	//---------------------------------------------------------------------
	case GC_WRW_DUCKED_FIRING:
	//---------------------------------------------------------------------
		stateResult = evaluateRotate();
		stateResult = evaluateStateDuckedFiring();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_DUCKED );
	break;

	//---------------------------------------------------------------------
	case GC_WRW_CHANGE_POSTURE_STAND:
	//---------------------------------------------------------------------
		stateResult = evaluateStateChangePostureStand();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_STAND );
	break;

	//---------------------------------------------------------------------
	case GC_WRW_STAND:
	//---------------------------------------------------------------------	

	//
	// First Check if we are within our _approachDist.  If 
	// not, then we need to close in on our enemy
	//
	if ( checkShouldApproach( distToEnemy ) )
		{
		transitionToState( GC_WRW_CHANGE_POSTURE_TO_ADVANCE );
		return BEHAVIOR_EVALUATING;
		}

	//
	// Now Check if wer are within our _retreatDist.  If 
	// we are, then we need to retreat in on our enemy
	//
	if ( checkShouldRetreat( distToEnemy ) )
		{
		transitionToState( GC_WRW_CHANGE_POSTURE_TO_RETREAT );
		return BEHAVIOR_EVALUATING;
		}

	//
	// Lets check if we need to strafe
	//
	if ( checkShouldStrafe( distToEnemy ) )
		{
		transitionToState( GC_WRW_STRAFE );
		return BEHAVIOR_EVALUATING;
		}
	

	//
	//Now let's check if we need to change posture
	//
	if ( checkShouldChangePosture( distToEnemy ) )
		{
		transitionToState( GC_WRW_CHANGE_POSTURE_DUCK );
		return BEHAVIOR_EVALUATING;
		}

	//
	//Well we don't have anything else to do, so let's evaluate our state
	//
	stateResult = evaluateRotate();

	if ( stateResult == BEHAVIOR_SUCCESS )
		{
		stateResult = evaluateStateStand();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_STAND_FIRING );

		}

	break;

	//---------------------------------------------------------------------
	case GC_WRW_STAND_FIRING:
	//---------------------------------------------------------------------
		stateResult = evaluateRotate();
		stateResult = evaluateStateStandFiring();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( GC_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( GC_WRW_STAND );
	break;
	}


	return BEHAVIOR_EVALUATING;   
}



//--------------------------------------------------------------
// Name:        End()
// Class:       GeneralCombatWithRangedWeapon
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::End(Actor &self)
{   
	if ( !_self )
		return;

	_self->movementSubsystem->setMovingBackwards( false );
	_fireWeapon.End(*_self);
}

//--------------------------------------------------------------
// Name:		think()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::think()
{
	if ( level.time > _nextClearAdvanceLockOutTime && _randomAdvanceLockOut )
		_randomAdvanceLockOut = false;

	if ( level.time > _nextClearRetreatLockOutTime && _randomRetreatLockOut )
		_randomRetreatLockOut = false;
}

//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::updateEnemy()
{
	
	Entity *currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !currentEnemy )
			{
			SetFailureReason( "GeneralCombatWithRangedWeapon::updateEnemy -- No Enemy" );
			transitionToState( GC_WRW_FAILED );
			}
			
		}

	_currentEnemy = currentEnemy;
	_self->SetHeadWatchTarget( _currentEnemy );
	setupRotate();
}

//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::transitionToState( generalCombatStates_t state )
{
	
	switch ( state )
	{
	case GC_WRW_CHANGE_POSTURE_TO_ADVANCE:
		setupStateChangePostureToAdvance();
		setInternalState( state , "GC_WRW_CHANGE_POSTURE_TO_ADVANCE" );
	break;

	case GC_WRW_CHANGE_POSTURE_TO_RETREAT:
		setupStateChangePostureToRetreat();
		setInternalState( state , "GC_WRW_CHANGE_POSTURE_TO_RETREAT" );
	break;

	case GC_WRW_ADVANCE:
		setupStateAdvance();
		setInternalState( state , "GC_WRW_ADVANCE" );
	break;

	case GC_WRW_ADVANCE_FIRING:
		setupStateAdvanceFiring();
		setInternalState( state , "GC_WRW_ADVANCE_FIRING" );
	break;

	case GC_WRW_RETREAT:
		setupStateRetreat();
		setInternalState( state , "GC_WRW_RETREAT" );
	break;

	case GC_WRW_RETREAT_FIRING:
		setupStateRetreatFiring();
		setInternalState( state , "GC_WRW_RETREAT_FIRING" );
	break;

	case GC_WRW_STRAFE:
		setupStateStrafe();
		setInternalState( state , "GC_WRW_STRAFE" );
	break;

	case GC_WRW_CHANGE_POSTURE_DUCK:
		setupStateChangePostureDuck();
		setInternalState( state , "GC_WRW_CHANGE_POSTURE_DUCK" );
	break;

	case GC_WRW_DUCKED:
		setupStateDucked();
		setInternalState( state , "GC_WRW_DUCKED" );
	break;

	case GC_WRW_DUCKED_FIRING:
		setupStateDuckedFiring();
		setInternalState( state , "GC_WRW_DUCKED_FIRING" );
	break;

	case GC_WRW_CHANGE_POSTURE_STAND:
		setupStateChangePostureStand();
		setInternalState( state , "GC_WRW_CHANGE_POSTURE_STAND" );
	break;

	case GC_WRW_STAND:
		setupStateStand();
		setInternalState( state , "GC_WRW_STAND" );
	break;

	case GC_WRW_STAND_FIRING:
		setupStateStandFiring();
		setInternalState( state , "GC_WRW_STAND_FIRING" );
	break;

	case GC_WRW_FAILED:
		setInternalState( state , "GC_WRW_FAILED" );
	break;
	}
	
}

//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setInternalState( generalCombatStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
	//gi.WDPrintf( "State: " + stateName + "\n" );
}

//--------------------------------------------------------------
// Name:		setTorsoAnim()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets our Torso Animation
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setTorsoAnim()
{
	_self->SetAnim( _torsoAnim , NULL , torso );
}


//--------------------------------------------------------------
// Name:		setupRotate
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up Rotate Component
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupRotate()
{
	_rotate.SetEntity( _currentEnemy ); 
	_rotate.Begin( *_self );
}


//--------------------------------------------------------------
// Name:		evaluateRotate()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates our Rotate Component
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateRotate()
{
	BehaviorReturnCode_t rotateResult;

	if ( level.time < _nextRotateTime )
		return BEHAVIOR_EVALUATING;
	
	rotateResult = _rotate.Evaluate( *_self );

	if ( rotateResult == BEHAVIOR_SUCCESS )
		_nextRotateTime = level.time + .75f;

	return rotateResult;
}

//--------------------------------------------------------------
// Name:		setupStateChangePostureToAdvance()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up the Change Posture To Advance state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateChangePostureToAdvance()
{
	bool canChange = _self->postureController->requestPosture( "STAND" , this );

	if ( !canChange )
		{
		failureStateChangePostureDuck ( "Requested Posture is unavailable" );
		return;
		}

	_finishedPostureTransition = false;
}

//--------------------------------------------------------------
// Name:		evaluateStateChangePostureToAdvance()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates the Change Posture To Advance State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateChangePostureToAdvance()
{
	//Basically waiting for our check to come true
	if ( _finishedPostureTransition )
		{
		_nextPostureChange = level.time + G_Random() + 2.5f;
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING;
	
}

//--------------------------------------------------------------
// Name:		failureStateChangePostureToAdvance()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for the Change Posture To Advance State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateChangePostureToAdvance( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateChangePostureToRetreat()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up the Change Posture To Retreat State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateChangePostureToRetreat()
{
	bool canChange = _self->postureController->requestPosture( "STAND" , this );

	if ( !canChange )
		{
		failureStateChangePostureDuck ( "Requested Posture is unavailable" );
		return;
		}

	_finishedPostureTransition = false;
}

//--------------------------------------------------------------
// Name:		evaluateStateChangePostureToRetreat()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates the ChangePostureToRetreat State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateChangePostureToRetreat()
{
	//Basically waiting for our check to come true
	if ( _finishedPostureTransition )
		{
		_nextPostureChange = level.time + G_Random() + 2.5f;
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateChangePostureToRetreat()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for the ChangePostureToRetreat State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateChangePostureToRetreat( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateAdvance()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up the Advance State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateAdvance()
{
	Vector selfToEnemy;	
	float dist;

	dist = G_Random ( 64.0f ) + 96.0f;

	if ( !_currentEnemy )
	{
		updateEnemy();
		return;
	}

	// Lets get oriented towards our enemy
	selfToEnemy = _currentEnemy->origin - _self->origin;
	selfToEnemy = selfToEnemy.toAngles();
	_self->setAngles( selfToEnemy );
	selfToEnemy.AngleVectors( &selfToEnemy );
	_self->movementSubsystem->setMoveDir( selfToEnemy );
	_self->movementSubsystem->setAnimDir( selfToEnemy );

	_moveRandomDir.SetDistance( dist );
	_moveRandomDir.SetAnim( "run" );
	_moveRandomDir.SetMode(MoveRandomDirection::RANDOM_MOVE_IN_FRONT);
	_moveRandomDir.Begin(*_self);
	_moveRandomDir.SetMinDistance( dist * .75 );
	
}

//--------------------------------------------------------------
// Name:		evaluateStateAdvance()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates our Advance State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateAdvance()
{
	BehaviorReturnCode_t moveResult = _moveRandomDir.Evaluate( *_self );

	if ( moveResult == BEHAVIOR_SUCCESS )
		{
		_randomAdvanceFailures = 0;
		return BEHAVIOR_SUCCESS;            
		}

	if ( moveResult == BEHAVIOR_FAILED )		
		failureStateAdvance( _moveRandomDir.GetFailureReason() );           		

	return BEHAVIOR_EVALUATING; 
}

//--------------------------------------------------------------
// Name:		failureStateAdvance()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for the Advance State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateAdvance( const str& failureReason )
{
	str FailureReason = "_moveRandomDir returned: " + failureReason;
	
	_randomAdvanceFailures++;
	if ( _randomAdvanceFailures > 10 )
		{
		_randomAdvanceLockOut = true;
		_nextClearAdvanceLockOutTime = level.time + G_Random( 2.0f ) + 1.0f;
		}
}

//--------------------------------------------------------------
// Name:		setupStateAdvanceFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up Advance Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateAdvanceFiring()
{
	setupStateAdvance();
	_nextFireTime = level.time + G_Random( _fireTimeMax ) + _fireTimeMin;

	if ( _self->combatSubsystem->CanAttackEnemy() )
		{
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );	
		}
}

//--------------------------------------------------------------
// Name:		evaluateStateAdvanceFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates State Advance Firing
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateAdvanceFiring()
{
	BehaviorReturnCode_t result;

	if ( _self->combatSubsystem->CanAttackEnemy() )
		_fireWeapon.Evaluate( *_self );
	else
		_fireWeapon.End( *_self );

	_self->combatSubsystem->AimWeaponTag(_currentEnemy);	

	result = evaluateStateAdvance();
	if ( result == BEHAVIOR_SUCCESS )
		setupStateAdvance();

	if ( level.time > _nextFireTime )
		return BEHAVIOR_SUCCESS;

	if ( result == BEHAVIOR_FAILED )
		{
		failureStateAdvance( _moveRandomDir.GetFailureReason() );           		
		return BEHAVIOR_FAILED;
		}

	return BEHAVIOR_EVALUATING;
	
}

//--------------------------------------------------------------
// Name:		failureStateAdvanceFiring()
// Class:		GeneralCombatWithRangedWeapon
//	
// Description:	Failure Handler for State Advance Firing
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateAdvanceFiring( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateRetreat()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up the Retreat State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateRetreat()
{
	Vector selfToEnemy;	
	float dist;

	dist = G_Random ( 64.0f ) + 96.0f;

	selfToEnemy = _currentEnemy->origin - _self->origin;
	selfToEnemy = selfToEnemy.toAngles();
	selfToEnemy[PITCH] = 0;
	selfToEnemy[ROLL] = 0;
	_self->setAngles( selfToEnemy );
	selfToEnemy.AngleVectors( &selfToEnemy );
	_self->movementSubsystem->setMoveDir( selfToEnemy );
	_self->movementSubsystem->setAnimDir( selfToEnemy );

	_moveRandomDir.SetDistance( dist );
	_moveRandomDir.SetAnim( "backpedal" );
	_moveRandomDir.SetMode(MoveRandomDirection::RANDOM_MOVE_IN_BACK);
	_moveRandomDir.Begin(*_self);
	_moveRandomDir.SetMinDistance( dist * .75 );
	_self->movementSubsystem->setMovingBackwards( true );

}

//--------------------------------------------------------------
// Name:		evaluateStateRetreat()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates the Retreat State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateRetreat()
{
	BehaviorReturnCode_t moveResult = _moveRandomDir.Evaluate( *_self );

	if ( moveResult == BEHAVIOR_SUCCESS )
		{
		_randomAdvanceFailures = 0;
		return BEHAVIOR_SUCCESS;            
		}

	if ( moveResult == BEHAVIOR_FAILED )
		{
		failureStateRetreat( _moveRandomDir.GetFailureReason() );           		
		return BEHAVIOR_FAILED;
		}

	return BEHAVIOR_EVALUATING; 
}

//--------------------------------------------------------------
// Name:		failureStateRetreat()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for Retreat
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateRetreat( const str& failureReason )
{
	str FailureReason = "_moveRandomDir returned: " + failureReason;
	
	_randomAdvanceFailures++;
	if ( _randomAdvanceFailures > 10 )
		{
		_randomAdvanceLockOut = true;
		_nextClearAdvanceLockOutTime = level.time + G_Random( 2.0f ) + 1.0f;
		}
}

//--------------------------------------------------------------
// Name:		setupStateRetreatFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up Retreat Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateRetreatFiring()
{
	setupStateRetreat();
	_nextFireTime = level.time + G_Random( _fireTimeMax ) + _fireTimeMin;
	if ( _self->combatSubsystem->CanAttackEnemy() )
		{
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );
		}

	_self->movementSubsystem->setMovingBackwards( true );
}

//--------------------------------------------------------------
// Name:		evaluateStateRetreatFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluate Retreat Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateRetreatFiring()
{
	BehaviorReturnCode_t result;

	if ( _self->combatSubsystem->CanAttackEnemy() )
		_fireWeapon.Evaluate( *_self );
	else
		_fireWeapon.End( *_self );

	_self->combatSubsystem->AimWeaponTag(_currentEnemy);	

	result = evaluateStateRetreat();
	if ( result == BEHAVIOR_SUCCESS )
		setupStateRetreat();

	if ( result == BEHAVIOR_FAILED )
		{
		failureStateRetreat( _moveRandomDir.GetFailureReason() );           		
		return BEHAVIOR_FAILED;
		}

	if ( level.time > _nextFireTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateRetreatFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure handler for Retreat Firing State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateRetreatFiring( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateStrafe()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up Strafe State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateStrafe()
{
	_strafeComponent.SetMode( Strafe::STRAFE_RANDOM );
	_strafeComponent.Begin( *_self );
}


//--------------------------------------------------------------
// Name:		evaluateStateStrafe()
// Class:		GeneralCombatWithRangedWeapon
//	
// Description:	Evaluates Strafe State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateStrafe()
{
	BehaviorReturnCode_t result;

	result = _strafeComponent.Evaluate( *_self );

	if ( result == BEHAVIOR_FAILED )
		{
		failureStateStrafe( _strafeComponent.GetFailureReason() );
		return BEHAVIOR_FAILED;
		}
	else if ( result == BEHAVIOR_SUCCESS )
		{
		_nextStrafeAttemptTime = level.time + G_Random( 1.0 ) + 1.0f;
		return BEHAVIOR_SUCCESS;		
		}

	return BEHAVIOR_EVALUATING; 
}

//--------------------------------------------------------------
// Name:		failureStateStrafe()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for Strafe State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateStrafe( const str& failureReason )
{
	_nextStrafeAttemptTime = level.time + G_Random( 1.0 ) + 3.0f;	
}

//--------------------------------------------------------------
// Name:		setupStateChangePostureDuck()
// Class:		GeneralCombatWithRangedWeapon
//	
// Description:	Sets up Change Posture Duck State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateChangePostureDuck()
{
	bool canChange = _self->postureController->requestPosture( "DUCK" , this );

	if ( !canChange )
		{
		// If we got here, there's a good chance we don't have a posture
		// state machine.  Check that first.
		failureStateChangePostureDuck ( "Requested Posture is unavailable" );
		return;
		}

	_finishedPostureTransition = false;
}

//--------------------------------------------------------------
// Name:		evaluateStateChangePostureDuck()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates our ChangePostureDuck state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateChangePostureDuck()
{
	//Basically waiting for our check to come true
	if ( _finishedPostureTransition )
		{
		_nextPostureChange = level.time + G_Random() + 2.5f;
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateChangePostureDuck()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	failureHandler for ChangePostureDuck
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateChangePostureDuck( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateDucked()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up Ducked State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateDucked()
{
	_nextPauseTime = level.time + G_Random( _pauseTimeMax ) + _pauseTimeMin;
	setTorsoAnim();
	_self->SetAnim( "duck" , NULL , legs );
	_fireWeapon.End(*_self);
}

//--------------------------------------------------------------
// Name:		evaluateStateDucked()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates Duck State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateDucked()
{
	if ( level.time > _nextPauseTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;	
}

//--------------------------------------------------------------
// Name:		failureStateDucked()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for Ducked State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateDucked( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateDuckedFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	sets up Ducked Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateDuckedFiring()
{
	_nextFireTime = level.time + G_Random( _fireTimeMax ) + _fireTimeMin;
	if ( _self->combatSubsystem->CanAttackEnemy() )
		{	
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );
		}
}

//--------------------------------------------------------------
// Name:		evaluateStateDuckedFiring()
// Class:		GeneralCombatWithRangedWeapon
//		
// Description:	Evaluates Ducked Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateDuckedFiring()
{

	if ( _self->combatSubsystem->CanAttackEnemy() )
		_fireWeapon.Evaluate( *_self );
	else
		_fireWeapon.End( *_self );

	_self->combatSubsystem->AimWeaponTag(_currentEnemy);	
	
	if ( level.time > _nextFireTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateDuckedFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for Ducked Firing State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateDuckedFiring( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateChangePostureStand()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up Change Posture Stand State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateChangePostureStand()
{
	bool canChange = _self->postureController->requestPosture( "STAND" , this );

	if ( !canChange )
		{
		// If we got here, there's a good chance we don't have a posture
		// state machine.  Check that first.
		failureStateChangePostureDuck ( "Requested Posture is unavailable" );
		return;
		}

	_finishedPostureTransition = false;
}

//--------------------------------------------------------------
// Name:		evaluateStateChangePostureStand()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates Change Posture Stand State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateChangePostureStand()
{
	//Basically waiting for our check to come true
	if ( _finishedPostureTransition )
		{
		_nextPostureChange = level.time + G_Random() + 2.5f;
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING; 
}

//--------------------------------------------------------------
// Name:		failureStateChangePostureStand
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for Change Posture Stand
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateChangePostureStand( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateStand()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up Stand State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateStand()
{
	_nextPauseTime = level.time + G_Random( _pauseTimeMax ) + _pauseTimeMin;
	setTorsoAnim();
	_self->SetAnim( _torsoAnim , NULL , legs );
	_fireWeapon.End(*_self);
}

//--------------------------------------------------------------
// Name:		evaluateStateStand()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates Stand State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateStand()
{
	if ( level.time > _nextPauseTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;	 
}

//--------------------------------------------------------------
// Name:		failureStateStand()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for Stand State
//
// Parameters:	const str& failureHandler
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateStand( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateStandFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Sets up Stand Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::setupStateStandFiring()
{
	_nextFireTime = level.time + G_Random( _fireTimeMax ) + _fireTimeMin;
	if ( _self->combatSubsystem->CanAttackEnemy() )
		{
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );
		}
}

//--------------------------------------------------------------
// Name:		evaluateStateStandFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Evaluates Stand Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GeneralCombatWithRangedWeapon::evaluateStateStandFiring()
{
	if ( _self->combatSubsystem->CanAttackEnemy() )
		_fireWeapon.Evaluate( *_self );
	else
		_fireWeapon.End( *_self );

	_self->combatSubsystem->AimWeaponTag(_currentEnemy);	
	
	if ( level.time > _nextFireTime )
		return BEHAVIOR_SUCCESS;	

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateStandFiring()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Failure Handler for Stand Firing State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GeneralCombatWithRangedWeapon::failureStateStandFiring( const str& failureReason )
{
}


//--------------------------------------------------------------
// Name:		checkShouldApproach()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Checks if we should change our state to approach
//
// Parameters:	float distToEnemy
//
// Returns:		true or false
//--------------------------------------------------------------
bool GeneralCombatWithRangedWeapon::checkShouldApproach( float distToEnemy )
{
	if ( distToEnemy >= _approachDist && !_randomAdvanceLockOut )
		return true;

	return false;
}

//--------------------------------------------------------------
// Name:		checkShouldRetreat()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Checks if we should change our state to retreat
//
// Parameters:	float distToEnemy
//
// Returns:		true or false
//--------------------------------------------------------------
bool GeneralCombatWithRangedWeapon::checkShouldRetreat( float distToEnemy )
{
	if ( distToEnemy <= _retreatDist && !_randomRetreatLockOut )
		return true;

	return false;
}

//--------------------------------------------------------------
// Name:		checkShouldStrafe()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Checks if we should strafe
//
// Parameters:	float distToEnemy 
//
// Returns:		true or false
//--------------------------------------------------------------
bool GeneralCombatWithRangedWeapon::checkShouldStrafe( float distToEnemy )
{
	//
	// First Check Timer
	//
	if ( level.time < _nextStrafeAttemptTime )
		return false;


	//
	//Next check if we need to strafe because we're too close to the enemy and our retreats have
	//have been locked.
	//
	if ( distToEnemy <= _retreatDist && _randomRetreatLockOut )
		return true;

	
	//
	//Now let's check if we need to strafe based on fire line
	//

	if ( _self->enemyManager->InEnemyLineOfFire() && G_Random() < _strafeChance )
		return true;
		
	return false;

}

//--------------------------------------------------------------
// Name:		checkShouldChangePosture()
// Class:		GeneralCombatWithRangedWeapon
//
// Description:	Checks if we should change posture
//
// Parameters:	float distToEnemy
//
// Returns:		true or false
//--------------------------------------------------------------
bool GeneralCombatWithRangedWeapon::checkShouldChangePosture( float distToEnemy )
{
	//
	// First Check Timer
	//
	if ( level.time < _nextPostureChange )
		return false;

	if ( G_Random() < _postureChangeChance && distToEnemy > _retreatDist )
		return true;
	
	return false;
}


