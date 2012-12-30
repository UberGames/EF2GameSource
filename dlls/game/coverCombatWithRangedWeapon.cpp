//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/coverCombatWithRangedWeapon.cpp            $
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
//	CoverCombatWithRangedWeapon Implementation
// 
// PARAMETERS:
//		str _movementAnim -- The animation to play while moving to the cover node
//
// ANIMATIONS:
//		_movementAnim : PARAMETER
//--------------------------------------------------------------------------------

#include "actor.h"
#include "coverCombatWithRangedWeapon.hpp"

extern Event EV_PostureChanged_Completed;

//--------------------------------------------------------------
//
// Init Static Vars
//
//--------------------------------------------------------------
const float CoverCombatWithRangedWeapon::NODE_RADIUS = 32.0f;


//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, CoverCombatWithRangedWeapon, NULL )
	{
		{ &EV_Behavior_Args,			&CoverCombatWithRangedWeapon::SetArgs		},
		{ &EV_Behavior_AnimDone,		&CoverCombatWithRangedWeapon::AnimDone		}, 
		{ &EV_PostureChanged_Completed, 	&CoverCombatWithRangedWeapon::PostureDone 	},
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		CoverCombatWithRangedWeapon()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CoverCombatWithRangedWeapon::CoverCombatWithRangedWeapon()
{	
	_self =  NULL;
}

//--------------------------------------------------------------
// Name:		CoverCombatWithRangedWeapon()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CoverCombatWithRangedWeapon::~CoverCombatWithRangedWeapon()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       CoverCombatWithRangedWeapon
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::SetArgs( Event *ev )
{
	_torsoAnim	  = ev->GetString ( 1 );
	_fireAnim	  = ev->GetString ( 2 );
	_movementAnim = ev->GetString ( 3 );
	_maxDistance  = ev->GetFloat  ( 4 );
	_fireTimeMin  = ev->GetFloat  ( 5 );
	_fireTimeMax  = ev->GetFloat  ( 6 );
	_pauseTimeMin = ev->GetFloat  ( 7 );
	_pauseTimeMax = ev->GetFloat  ( 8 );

}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       CoverCombatWithRangedWeapon
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::AnimDone( Event *ev )
{
}


//--------------------------------------------------------------
// Name:		PostureDone()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Handles a Posture Done Event
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::PostureDone( Event *ev )
{
	_finishedPostureTransition = true;
}


//--------------------------------------------------------------
// Name:        Begin()
// Class:       CoverCombatWithRangedWeapon
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::Begin( Actor &self )
{       
	init( self );
}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       CoverCombatWithRangedWeapon
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::Evaluate( Actor &self )
{
	BehaviorReturnCode_t stateResult;


	think();

	switch ( _state )
	{
	//---------------------------------------------------------------------
	case COVERCOMBAT_WRW_FIND_COVER:
	//---------------------------------------------------------------------
		stateResult = evaluateStateFindCover();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( COVERCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( COVERCOMBAT_WRW_MOVE_TO_COVER );
		
	break;

	//---------------------------------------------------------------------
	case COVERCOMBAT_WRW_FIND_BETTER_COVER:
	//---------------------------------------------------------------------
		stateResult = evaluateStateFindBetterCover();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( COVERCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( COVERCOMBAT_WRW_MOVE_TO_COVER );
	break;

	//---------------------------------------------------------------------
	case COVERCOMBAT_WRW_MOVE_TO_COVER:
	//---------------------------------------------------------------------
		stateResult = evaluateStateMoveToCover();
		
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( COVERCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( COVERCOMBAT_WRW_CHANGE_POSTURE_DUCK );

	break;

	//---------------------------------------------------------------------
	case COVERCOMBAT_WRW_CHANGE_POSTURE_DUCK:
	//---------------------------------------------------------------------
		if ( _spotted )
			{
			transitionToState( COVERCOMBAT_WRW_SPOTTED );
			BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateStateChangePostureDuck();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( COVERCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( COVERCOMBAT_WRW_DUCKED );
	break;

	//---------------------------------------------------------------------
	case COVERCOMBAT_WRW_CHANGE_POSTURE_STAND:
	//---------------------------------------------------------------------
		if ( _spotted )
			{
			transitionToState( COVERCOMBAT_WRW_SPOTTED );
			return BEHAVIOR_EVALUATING;
			}		
		
		stateResult = evaluateStateChangePostureStand();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( COVERCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( COVERCOMBAT_WRW_STANDING );
		
	break;

	//----------------------------------------------------------------------
	case COVERCOMBAT_WRW_FIRE_STANDING:
	//----------------------------------------------------------------------
		if ( _spotted )
			{
			_fireWeapon.End( *_self );
			setTorsoAnim();
			transitionToState( COVERCOMBAT_WRW_SPOTTED );
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateRotate();
		stateResult = evaluateStateFireStanding();

		// Check if we need to stop firing
		if ( stateResult == BEHAVIOR_FAILED )
			{
			transitionToState( COVERCOMBAT_WRW_FAILED );
			_fireWeapon.End( *_self );
			setTorsoAnim();
			return BEHAVIOR_EVALUATING;
			}

		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			transitionToState( COVERCOMBAT_WRW_FIRE_PAUSE_STANDING );
			_fireWeapon.End( *_self );
			setTorsoAnim();
			return BEHAVIOR_EVALUATING;
			}

		// See if we just need to duck again
		stateResult = evaluateStateStanding();

		if ( stateResult == BEHAVIOR_FAILED )
			{
			transitionToState( COVERCOMBAT_WRW_FAILED );			
			_fireWeapon.End( *_self );
			setTorsoAnim();
			}

		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			transitionToState( COVERCOMBAT_WRW_CHANGE_POSTURE_DUCK );
			_fireWeapon.End( *_self );
			setTorsoAnim();
			}
		
	break;

	//----------------------------------------------------------------------
	case COVERCOMBAT_WRW_FIRE_PAUSE_STANDING:
	//----------------------------------------------------------------------
		if ( _spotted )
			{
			transitionToState( COVERCOMBAT_WRW_SPOTTED );
			BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateRotate();
		stateResult = evaluateStateFirePauseStanding();
		if ( stateResult == BEHAVIOR_FAILED )
			{
			transitionToState( COVERCOMBAT_WRW_FAILED );			
			_fireWeapon.End( *_self );
			setTorsoAnim();
			return BEHAVIOR_EVALUATING;
			}

		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			transitionToState(COVERCOMBAT_WRW_FIRE_STANDING);
			return BEHAVIOR_EVALUATING;
			}
			
		// See if we just need to duck again
		stateResult = evaluateStateStanding();

		if ( stateResult == BEHAVIOR_FAILED )
			{
			transitionToState( COVERCOMBAT_WRW_FAILED );			
			_fireWeapon.End( *_self );
			setTorsoAnim();
			}

		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			transitionToState( COVERCOMBAT_WRW_CHANGE_POSTURE_DUCK );
			_fireWeapon.End( *_self );
			setTorsoAnim();
			}

	break;

	//----------------------------------------------------------------------
	case COVERCOMBAT_WRW_DUCKED:
	//----------------------------------------------------------------------
		if ( _spotted )
			{
			transitionToState( COVERCOMBAT_WRW_SPOTTED );
			BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateStateDucked();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( COVERCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( COVERCOMBAT_WRW_CHANGE_POSTURE_STAND );
	break;

	//----------------------------------------------------------------------
	case COVERCOMBAT_WRW_STANDING:
	//----------------------------------------------------------------------
		if ( _spotted )
			{
			transitionToState( COVERCOMBAT_WRW_SPOTTED );
			BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateRotate();
		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( COVERCOMBAT_WRW_FIRE_STANDING );			
			

	break;

	//----------------------------------------------------------------------
	case COVERCOMBAT_WRW_SPOTTED:
	//----------------------------------------------------------------------
		stateResult = evaluateStateSpotted();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( COVERCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( COVERCOMBAT_WRW_FIND_BETTER_COVER );
	break;

	//----------------------------------------------------------------------
	case COVERCOMBAT_WRW_SUCCESS:
	//----------------------------------------------------------------------
		return BEHAVIOR_SUCCESS;

	break;


	//----------------------------------------------------------------------
	case COVERCOMBAT_WRW_FAILED:
	//----------------------------------------------------------------------
		return BEHAVIOR_FAILED;

	break;
	}

	return BEHAVIOR_EVALUATING;   

}



//--------------------------------------------------------------
// Name:        End()
// Class:       CoverCombatWithRangedWeapon
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::End(Actor &self)
{   
	if ( !_self )
		return;

	_self->movementSubsystem->setMovingBackwards( false );
	_fireWeapon.End(*_self);
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::transitionToState( coverCombatStates_t state )
{
	switch ( state )
	{
	case COVERCOMBAT_WRW_FIND_COVER:
		setupStateFindCover();	
		setInternalState( state , "COVERCOMBAT_WRW_FIND_COVER" );
	break;

	case COVERCOMBAT_WRW_FIND_BETTER_COVER:
		setupStateFindBetterCover();	
		setInternalState( state , "COVERCOMBAT_WRW_FIND_BETTER_COVER" );
	break;

	case COVERCOMBAT_WRW_MOVE_TO_COVER:
		setupStateMoveToCover();
		setInternalState( state , "COVERCOMBAT_WRW_MOVE_TO_COVER" );
	break;

	case COVERCOMBAT_WRW_CHANGE_POSTURE_DUCK:
		setupStateChangePostureDuck();
		setInternalState( state , "COVERCOMBAT_WRW_CHANGE_POSTURE_DUCK" );
	break;

	case COVERCOMBAT_WRW_CHANGE_POSTURE_STAND:
		setupStateChangePostureStand();
		setInternalState( state , "COVERCOMBAT_WRW_CHANGE_POSTURE_DUCK" );
	break;

	case COVERCOMBAT_WRW_FIRE_STANDING:
		setupStateFireStanding();
		setInternalState( state , "COVERCOMBAT_WRW_FIRE_STANDING" );
	break;

	case COVERCOMBAT_WRW_FIRE_PAUSE_STANDING:
		setupStateFirePauseStanding();
		setInternalState( state , "COVERCOMBAT_WRW_FIRE_PAUSE_STANDING" );
	break;

	case COVERCOMBAT_WRW_DUCKED:
		setupStateDucked();
		setInternalState( state , "COVERCOMBAT_WRW_DUCKED" );
	break;

	case COVERCOMBAT_WRW_STANDING:
		setupStateStanding();
		setInternalState( state , "COVERCOMBAT_WRW_STANDING" );
	break;

	case COVERCOMBAT_WRW_SPOTTED:
		setupStateSpotted();
		setInternalState( state , "COVERCOMBAT_WRW_SPOTTED" );
	break;

	case COVERCOMBAT_WRW_SUCCESS:
		setInternalState( state , "COVERCOMBAT_WRW_SUCCESS" );
	break;

	case COVERCOMBAT_WRW_FAILED:
		setInternalState( state , "COVERCOMBAT_WRW_FAILED" );
	break;
	}
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setInternalState( coverCombatStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		init()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::init( Actor &self )
{
	_self = &self;
	transitionToState(COVERCOMBAT_WRW_FIND_COVER);
	_finishedPostureTransition = true;
	_nextStandTime = 0.0f;
	_nextDuckTime = 0.0f;
	_nextSpotCheck = 0.0f;	
	updateEnemy();	
}

//--------------------------------------------------------------
// Name:		think()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::think()
{
	trace_t trace;

	if ( level.time < _nextSpotCheck )
		return;

	if ( !_currentEnemy || !_node )
		return;

	updateEnemy();
	trace = G_Trace( _node->origin , vec_zero , vec_zero , _currentEnemy->origin , NULL , MASK_SOLID, false, "CoverCombatWithRangedWeapon::think");

	if ( trace.fraction >= .95 )
		_spotted = true;
	else
		_spotted = false;

	_nextSpotCheck = level.time + G_Random(0.5);
	
}


//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::updateEnemy()
{
	Entity *currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !currentEnemy )
			{
			SetFailureReason( "CoverCombatWithRangedWeapon::updateEnemy -- No Enemy" );
			transitionToState( COVERCOMBAT_WRW_FAILED );
			}
			
		}

	_currentEnemy = currentEnemy;
	setupRotate();
}

//--------------------------------------------------------------
// Name:		setupStateFindCover()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up the Find Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateFindCover()
{
}


//--------------------------------------------------------------
// Name:		evaluateStateFindCover()
// Class:		CoverCombatWithRangedWeapon
//
// Description: Evaluates the Find Cover State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateFindCover()
{
	//This Behavior's not being used anymore and should be removed
	//_node = HelperNode::FindClosestHelperNodeThatCannotSeeEntity( *_self , NODETYPE_COVER , _self->edict->clipmask , _maxDistance , 0.0f , _currentEnemy );

	if ( !_node )
		{		
		failureStateFindCover( "CoverCombatWithRangedWeapon::evaluateStateFindCover() -- Cannot Find Suitable Cover Node" );
		return BEHAVIOR_FAILED;
		}

	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateFindCover()
// Class:		CoverCombatWithRangedWeapon
//	
// Description:	Failure Handler for Find Cover State
//
// Parameters:	const str& failureReason	
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateFindCover(const str& failureReason)
{
	SetFailureReason( failureReason );
}



//--------------------------------------------------------------
// Name:		setupStateMoveToCover()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateMoveToCover()
{
	_gotoPoint.SetAnim( _movementAnim );
	_gotoPoint.SetDistance( NODE_RADIUS );
	_gotoPoint.SetPoint( _node->origin );
	_gotoPoint.Begin( *_self );
}


//--------------------------------------------------------------
// Name:		evaluateStateMoveToCover()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Evaluates the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateMoveToCover()
{
	BehaviorReturnCode_t result = _gotoPoint.Evaluate( *_self );
	str failureReason = "CoverCombatWithRangedWeapon::evaluateStateMoveToCover -- _gotoPoint component returned: " + _gotoPoint.GetFailureReason();
	
	if ( result == BEHAVIOR_FAILED )
		failureStateMoveToCover( failureReason );

	return result;		
	
}


//--------------------------------------------------------------
// Name:		failureStateMoveToCover
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Failure Handler for State Move To Cover
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateMoveToCover( const str &failureReason )
{
}


//--------------------------------------------------------------
// Name:		setupStateChangePostureDuck()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up the ChangePostureDuck state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateChangePostureDuck()
{
	bool canChange = _self->postureController->requestPosture( "DUCK" , this );

	if ( !canChange )
		{
		failureStateChangePostureDuck ( "Requested Posture is unavailable" );
		return;
		}

	_finishedPostureTransition = false;
}

//--------------------------------------------------------------
// Name:		evaluateStateChangePostureDuck()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Evaluates our ChangePostureDuck state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateChangePostureDuck()
{
	//Basically waiting for our check to come true
	if ( _finishedPostureTransition )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;

}

//--------------------------------------------------------------
// Name:		failureStateChangePostureDuck()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Failure Handler for ChangePostureDuck State
//
// Parameters:	const str &failureReason -- Why we're failing
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateChangePostureDuck( const str &failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateChangePostureStand()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up the ChangePostureStand state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateChangePostureStand()
{
	bool canChange = _self->postureController->requestPosture( "STAND" , this );

	if ( !canChange )
		{
		failureStateChangePostureStand ( "Requested Posture is unavailable" );
		return;
		}

	_finishedPostureTransition = false;
}

//--------------------------------------------------------------
// Name:		evaluateStateChangePostureStand()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Evaluates our ChangePostureStand state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateChangePostureStand()
{
	//Basically waiting for our check to come true
	if ( _finishedPostureTransition )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;

}

//--------------------------------------------------------------
// Name:		failureStateChangePostureStand()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Failure Handler for Change Posture Stand function
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateChangePostureStand( const str &failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateSpotted()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up the Spotted State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateSpotted()
{
	bool canChange = _self->postureController->requestPosture( "STAND" , this );

	if ( !canChange )
		{
		failureStateChangePostureStand ( "Requested Posture is unavailable" );
		return;
		}

	_finishedPostureTransition = false;
}

//--------------------------------------------------------------
// Name:		evaluateStateSpotted()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Evaluates our Spotted State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateSpotted()
{
	//Basically waiting for our check to come true
	if ( _finishedPostureTransition )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateSpotted()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Failure Handler for Spotted State
//
// Parameters:	const str &failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateSpotted( const str &failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateDucked()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up our Ducked State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateDucked()
{
	_nextStandTime = level.time + G_Random() + 3.0f;
}

//--------------------------------------------------------------
// Name:		evaluateStateDucked()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Evaluates our Ducked State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateDucked()
{
	if ( level.time > _nextStandTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateDucked()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Failure Handler for Ducked State
//
// Parameters:	const str &failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateDucked( const str &failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateStanding()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Setup for the Standing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateStanding()
{
	_nextDuckTime = level.time + G_Random() + 5.0f;
	_nextFireTime = 0.0f;
	_nextPauseTime = level.time + G_Random(_pauseTimeMax ) + _pauseTimeMin;
}

//--------------------------------------------------------------
// Name:		evaluateStateStanding()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Evaluates the Standing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateStanding()
{
	if ( level.time > _nextDuckTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateStanding()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Failure Handler for the Standing State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateStanding( const str &failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateFindBetterCover()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up the Find Better Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateFindBetterCover()
{
}


//--------------------------------------------------------------
// Name:		evaluateStateFindBetterCover()
// Class:		CoverCombatWithRangedWeapon
//
// Description: Evaluates the Find Better Cover State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateFindBetterCover()
{
	//Behavior Needs to be removed
//	_node = HelperNode::FindClosestHelperNodeThatCannotSeeEntity( *_self , NODETYPE_COVER , _self->edict->clipmask , _maxDistance , 512.0f , _currentEnemy );

	if ( !_node )
		{		
		failureStateFindCover( "CoverCombatWithRangedWeapon::evaluateStateFindBetterCover() -- Cannot Find Suitable Cover Node" );
		return BEHAVIOR_FAILED;
		}

	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateFindBetterCover()
// Class:		CoverCombatWithRangedWeapon
//	
// Description:	Failure Handler for Find Cover State
//
// Parameters:	const str& failureReason	
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateFindBetterCover(const str& failureReason)
{
	SetFailureReason( failureReason );
}

//--------------------------------------------------------------
// Name:		setupRotate
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up Rotate Component
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupRotate()
{
	_rotate.SetEntity( _currentEnemy ); 
	_rotate.Begin( *_self );
}


//--------------------------------------------------------------
// Name:		evaluateRotate()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Evaluates our Rotate Component
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateRotate()
{
	return _rotate.Evaluate( *_self );
}

//--------------------------------------------------------------
// Name:		setupStateFireStanding()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up our Fire Standing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateFireStanding()
{
	_nextFireTime = level.time + G_Random( _fireTimeMax ) + _fireTimeMin;
	if ( _self->combatSubsystem->CanAttackEnemy() )
		{
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );
		}
}

//--------------------------------------------------------------
// Name:		evaluateStateFireStanding()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Evaluates our Fire Standing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateFireStanding()
{
	_fireWeapon.Evaluate( *_self );
	_self->combatSubsystem->AimWeaponTag(_currentEnemy);	

	if ( level.time > _nextFireTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateFireStanding()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Failure Handler for Fire Standing State
//
// Parameters:	const str &failureReason -- Why we're failing
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateFireStanding( const str &failureReason )
{

}

//--------------------------------------------------------------
// Name:		setupStateFirePauseStanding()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets up our Pause Fire State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setupStateFirePauseStanding()
{
	_nextPauseTime = level.time + G_Random( _pauseTimeMax ) + _pauseTimeMin;
	setTorsoAnim();
}

//--------------------------------------------------------------
// Name:		evaluateStateFirePauseStanding()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Evaluates our Pause Fire State
//
// Parameters:	None
//	
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CoverCombatWithRangedWeapon::evaluateStateFirePauseStanding()
{
	if ( level.time > _nextPauseTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateFirePauseStanding()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Failure Handler for our Pause Standing State
//
// Parameters:	const str &failureReason -- Why we're failing
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::failureStateFirePauseStanding( const str &failureReason )
{
}


//--------------------------------------------------------------
// Name:		setTorsoAnim()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Sets our Torso Animation
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CoverCombatWithRangedWeapon::setTorsoAnim()
{
	_self->SetAnim( _torsoAnim , NULL , torso );
}

//--------------------------------------------------------------
// Name:		CanExecute()
// Class:		CoverCombatWithRangedWeapon
//
// Description:	Checks if the Behavior can execute
//
// Parameters:	Actor &self,
//				float maxDistance
//
// Returns:		true or false;
//--------------------------------------------------------------
bool CoverCombatWithRangedWeapon::CanExecute( Actor &self , float maxDistance )
{
	//HelperNode* node;
	Entity* currentEnemy;

	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if ( !currentEnemy )
		return false;
	
	//behavior needs to be removed
//	node = HelperNode::FindClosestHelperNodeThatCannotSeeEntity( self , NODETYPE_COVER , self.edict->clipmask , maxDistance , 0.0f , currentEnemy );

	//if ( node )
	//	return true;

	return false;
}






