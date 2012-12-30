//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/corridorCombatWithRangedWeapon.cpp         $
// $Revision:: 8                                                              $
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
//	CorridorCombatWithRangedWeapon Implementation
// 
// PARAMETERS:
//		str _movementAnim -- The animation to play while moving to the cover node
//
// ANIMATIONS:
//		_movementAnim : PARAMETER
//--------------------------------------------------------------------------------

#include "actor.h"
#include "corridorCombatWithRangedWeapon.hpp"

extern Event EV_PostureChanged_Completed;

//--------------------------------------------------------------
//
// Init Static Vars
//
//--------------------------------------------------------------
const float CorridorCombatWithRangedWeapon::NODE_RADIUS = 32.0f;


//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, CorridorCombatWithRangedWeapon, NULL )
	{
		{ &EV_Behavior_Args,		&CorridorCombatWithRangedWeapon::SetArgs	},
		{ &EV_Behavior_AnimDone,	&CorridorCombatWithRangedWeapon::AnimDone	}, 
		{ &EV_PostureChanged_Completed, &CorridorCombatWithRangedWeapon::PostureDone	},
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		CorridorCombatWithRangedWeapon()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CorridorCombatWithRangedWeapon::CorridorCombatWithRangedWeapon()
{	
	_self = NULL;
	_movementAnim			= "idle";
	_torsoAnim				= "idle";
	_fireAnim				= "idle";
	_preFireAnim			= "idle";
	_postFireAnim			= "idle";
	_postureChangeChance	= 0.0f;
	_maxDistance			= 0.0f;
	_retreatDistance		= 0.0f;
	_threatDistance			= 0.0f;
	_fireTimeMin			= 0.0f;
	_fireTimeMax			= 0.0f;
	_pauseTimeMin			= 0.0f;
	_pauseTimeMax			= 0.0f;

}

//--------------------------------------------------------------
// Name:		CorridorCombatWithRangedWeapon()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CorridorCombatWithRangedWeapon::~CorridorCombatWithRangedWeapon()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       CorridorCombatWithRangedWeapon
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::SetArgs( Event *ev )
{
	int num = ev->NumArgs();
	if ( num >  0 ) _movementAnim			= ev->GetString(  1 );
	if ( num >  1 ) _torsoAnim				= ev->GetString(  2 );
	if ( num >  2 ) _fireAnim				= ev->GetString(  3 );
	if ( num >  3 ) _preFireAnim			= ev->GetString(  4 );
	if ( num >  4 ) _postFireAnim			= ev->GetString(  5 );
	if ( num >  5 ) _postureChangeChance	= ev->GetFloat (  6 );
	if ( num >  6 ) _maxDistance			= ev->GetFloat (  7 );
	if ( num >  7 ) _retreatDistance		= ev->GetFloat (  8 );
	if ( num >  8 ) _threatDistance			= ev->GetFloat (  9 );
	if ( num >  9 ) _fireTimeMin			= ev->GetFloat ( 10 );
	if ( num > 10 ) _fireTimeMax			= ev->GetFloat ( 11 );
	if ( num > 11 ) _pauseTimeMin			= ev->GetFloat ( 12 );
	if ( num > 12 ) _pauseTimeMax			= ev->GetFloat ( 13 );	

}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       CorridorCombatWithRangedWeapon
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::AnimDone( Event *ev )
{
}


//--------------------------------------------------------------
// Name:		PostureDone()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Handles a Posture Done Event
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::PostureDone( Event *ev )
{
	_finishedPostureTransition = true;
}


//--------------------------------------------------------------
// Name:        Begin()
// Class:       CorridorCombatWithRangedWeapon
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::Begin( Actor &self )
{       
	init( self );
}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       CorridorCombatWithRangedWeapon
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::Evaluate( Actor &self )
{
	BehaviorReturnCode_t stateResult;


	think();

	switch ( _state )
	{
	//---------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_FINDNODE:
	//---------------------------------------------------------------------
		stateResult = evaluateStateFindNode();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( CORRIDORCOMBAT_WRW_STAND );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( CORRIDORCOMBAT_WRW_MOVETONODE );
	break;

	//---------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_MOVETONODE:
	//---------------------------------------------------------------------
		stateResult = evaluateStateMoveToNode();
	
		if ( stateResult == BEHAVIOR_FAILED )
			{		
			_self->SetAnim( "idle" , NULL , legs );
			transitionToState( CORRIDORCOMBAT_WRW_STAND );			
			}

		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			_self->SetAnim( "idle" , NULL , legs );
			stateResult = evaluateRotate();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				if ( checkShouldDuck() )
					transitionToState( CORRIDORCOMBAT_WRW_CHANGEPOSTURE_DUCK );
				else
					transitionToState( CORRIDORCOMBAT_WRW_STAND );
				}
			
			}

	break;

	//---------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_BACKPEDAL:
	//---------------------------------------------------------------------
		stateResult = evaluateStateBackPedal();

		if ( stateResult == BEHAVIOR_FAILED )
			{
			_self->movementSubsystem->setMovingBackwards( false );
			_self->SetAnim( "idle" , NULL , legs );
			_holdPositionTime = level.time + G_Random(2.0) + 2.0f;

			transitionToState( CORRIDORCOMBAT_WRW_STAND );			
			return BEHAVIOR_EVALUATING;
			}
		
		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			_self->movementSubsystem->setMovingBackwards( false );
			_self->SetAnim( "idle" , NULL , legs );
			transitionToState( CORRIDORCOMBAT_WRW_FINDBETTERNODE );			
			}
	break;

	//---------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_FINDBETTERNODE:
	//---------------------------------------------------------------------
		stateResult = evaluateStateFindBetterNode();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( CORRIDORCOMBAT_WRW_STAND );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( CORRIDORCOMBAT_WRW_MOVETOBETTERNODE );			

	break;

	//---------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_MOVETOBETTERNODE:
	//---------------------------------------------------------------------
		stateResult = evaluateStateMoveToBetterNode();		

		if ( stateResult == BEHAVIOR_FAILED )
			{
			_self->SetAnim( "idle" , NULL , legs );
			transitionToState( CORRIDORCOMBAT_WRW_FAILED );			
			}

		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			_self->SetAnim( "idle" , NULL , legs );
			stateResult = evaluateRotate();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				if ( checkShouldDuck() )
					transitionToState( CORRIDORCOMBAT_WRW_CHANGEPOSTURE_DUCK );
				else
					transitionToState( CORRIDORCOMBAT_WRW_STAND );
				}
			
			}
	break;

	//---------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_CHANGEPOSTURE_DUCK:
	//---------------------------------------------------------------------
		stateResult = evaluateStateChangePostureDuck();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( CORRIDORCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( CORRIDORCOMBAT_WRW_DUCKED );

	break;

	//---------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_CHANGEPOSTURE_STAND:
	//---------------------------------------------------------------------
		stateResult = evaluateStateChangePostureStand();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( CORRIDORCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( CORRIDORCOMBAT_WRW_STAND );

	break;

	//---------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_DUCKED:
	//---------------------------------------------------------------------
		if ( checkShouldStand() )	//Make sure we don't need to stand back up
			{
			transitionToState( CORRIDORCOMBAT_WRW_CHANGEPOSTURE_STAND );
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateRotate();
		stateResult = evaluateStateDucked();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( CORRIDORCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( CORRIDORCOMBAT_WRW_DUCKED_FIRING );
	break;

	//----------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_DUCKED_FIRING:
	//----------------------------------------------------------------------
		if ( checkShouldStand() )	//Make sure we don't need to stand back up
			{
			transitionToState( CORRIDORCOMBAT_WRW_CHANGEPOSTURE_STAND );
			_fireWeapon.End(*_self);
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateRotate();
		if ( stateResult != BEHAVIOR_SUCCESS )
			return BEHAVIOR_EVALUATING;


		stateResult = evaluateStateFireDucked();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( CORRIDORCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( CORRIDORCOMBAT_WRW_DUCKED );

	break;

	//----------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_STAND:
	//----------------------------------------------------------------------
		if ( checkShouldRetreat() )
			{
			transitionToState( CORRIDORCOMBAT_WRW_BACKPEDAL );
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateRotate();
		stateResult = evaluateStateStanding();
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( CORRIDORCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( CORRIDORCOMBAT_WRW_STAND_FIRING );
	break;

	//----------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_STAND_FIRING:
	//----------------------------------------------------------------------
		if ( checkShouldRetreat() )
			{
			transitionToState( CORRIDORCOMBAT_WRW_BACKPEDAL );
			_fireWeapon.End(*_self);
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateRotate();
		if ( stateResult != BEHAVIOR_SUCCESS )
			return BEHAVIOR_EVALUATING;

		stateResult = evaluateStateFireStanding();
		if ( stateResult == BEHAVIOR_FAILED )
			{
			_fireWeapon.End(*_self);
			transitionToState( CORRIDORCOMBAT_WRW_FAILED );
			}

		if ( stateResult == BEHAVIOR_SUCCESS )
			{
			_fireWeapon.End(*_self);
			transitionToState( CORRIDORCOMBAT_WRW_STAND );
			}

	break;

	//---------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_HOLD_POSITION:
	//---------------------------------------------------------------------
		stateResult = evaluateStateHoldPosition();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( CORRIDORCOMBAT_WRW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState(CORRIDORCOMBAT_WRW_FINDBETTERNODE);
	break;

	//----------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_SUCCESS:
	//----------------------------------------------------------------------
		return BEHAVIOR_SUCCESS;

	break;


	//----------------------------------------------------------------------
	case CORRIDORCOMBAT_WRW_FAILED:
	//----------------------------------------------------------------------
		return BEHAVIOR_FAILED;

	break;
	}

	return BEHAVIOR_EVALUATING;   

}



//--------------------------------------------------------------
// Name:        End()
// Class:       CorridorCombatWithRangedWeapon
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::End(Actor &self)
{   
	if ( !_self )
		return;

	_self->movementSubsystem->setMovingBackwards( false );
	_fireWeapon.End(*_self);
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::transitionToState( corridorCombatStates_t state )
{
	switch ( state )
	{
	case CORRIDORCOMBAT_WRW_FINDNODE:
		setupStateFindNode();	
		setInternalState( state , "CORRIDORCOMBAT_WRW_FINDNODE" );
	break;

	case CORRIDORCOMBAT_WRW_MOVETONODE:
		setupStateMoveToNode();	
		setInternalState( state , "CORRIDORCOMBAT_WRW_MOVETONODE" );
	break;

	case CORRIDORCOMBAT_WRW_BACKPEDAL:
		setupStateBackPedal();
		setInternalState( state , "CORRIDORCOMBAT_WRW_BACKPEDAL" );
	break;

	case CORRIDORCOMBAT_WRW_FINDBETTERNODE:
		setupStateFindBetterNode();
		setInternalState( state , "CORRIDORCOMBAT_WRW_FINDBETTERNODE" );
	break;

	case CORRIDORCOMBAT_WRW_MOVETOBETTERNODE:
		setupStateMoveToBetterNode();
		setInternalState( state , "CORRIDORCOMBAT_WRW_MOVETOBETTERNODE" );
	break;

	case CORRIDORCOMBAT_WRW_CHANGEPOSTURE_DUCK:
		setupStateChangePostureDuck();
		setInternalState( state , "CORRIDORCOMBAT_WRW_CHANGEPOSTURE_DUCK" );
	break;

	case CORRIDORCOMBAT_WRW_CHANGEPOSTURE_STAND:
		setupStateChangePostureStand();
		setInternalState( state , "CORRIDORCOMBAT_WRW_CHANGEPOSTURE_STAND" );
	break;

	case CORRIDORCOMBAT_WRW_DUCKED:
		setupStateDucked();
		setInternalState( state , "CORRIDORCOMBAT_WRW_DUCKED" );
	break;

	case CORRIDORCOMBAT_WRW_DUCKED_FIRING:
		setupStateFireDucked();
		setInternalState( state , "CORRIDORCOMBAT_WRW_DUCKED_FIRING" );
	break;

	case CORRIDORCOMBAT_WRW_STAND:
		setupStateStanding();
		setInternalState( state , "CORRIDORCOMBAT_WRW_STAND" );
	break;

	case CORRIDORCOMBAT_WRW_STAND_FIRING:
		setupStateFireStanding();
		setInternalState( state , "CORRIDORCOMBAT_WRW_STAND_FIRING" );
	break;

	case CORRIDORCOMBAT_WRW_HOLD_POSITION:
		setupStateHoldPosition();
		setInternalState( state , "CORRIDORCOMBAT_WRW_HOLD_POSITION" );
	break;

	case CORRIDORCOMBAT_WRW_SUCCESS:
		setInternalState( state , "CORRIDORCOMBAT_WRW_SUCCESS" );
	break;

	case CORRIDORCOMBAT_WRW_FAILED:
		setInternalState( state , "CORRIDORCOMBAT_WRW_FAILED" );
	break;
	}
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setInternalState( corridorCombatStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );	
}

//--------------------------------------------------------------
// Name:		init()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::init( Actor &self )
{
	_self = &self;
	transitionToState(CORRIDORCOMBAT_WRW_FINDNODE);
	_finishedPostureTransition = true;		
	_holdPositionTime = 0.0f;
	_enemyUpdateTime = 0.0f;
	updateEnemy();	
}

//--------------------------------------------------------------
// Name:		think()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::think()
{	
	updateEnemy();
}


//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::updateEnemy()
{
	if ( level.time < _enemyUpdateTime )
		return;

	Entity *currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !currentEnemy )
			{
			SetFailureReason( "CorridorCombatWithRangedWeapon::updateEnemy -- No Enemy" );
			transitionToState( CORRIDORCOMBAT_WRW_FAILED );
			}
			
		}

	_currentEnemy = currentEnemy;
	_enemyUpdateTime = level.time + G_Random() + 3.0f; 
	setupRotate();
}

//--------------------------------------------------------------
// Name:		setupStateFindNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up the Find Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateFindNode()
{
}


//--------------------------------------------------------------
// Name:		evaluateStateFindNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description: Evaluates the Find Cover State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateFindNode()
{
	if ( _node )
		_node->UnreserveNode();

	_node = HelperNode::FindClosestHelperNode( *_self , NODETYPE_COMBAT , DESCRIPTOR_CORRIDOR , _maxDistance );	

	if ( !_node )
		{		
		failureStateFindNode( "CorridorCombatWithRangedWeapon::evaluateStateFindCover() -- Cannot Find Suitable Cover Node" );
		return BEHAVIOR_FAILED;
		}

	_node->ReserveNode();
	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateFindNode()
// Class:		CorridorCombatWithRangedWeapon
//	
// Description:	Failure Handler for Find Cover State
//
// Parameters:	const str& failureReason	
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateFindNode(const str& failureReason)
{
	SetFailureReason( failureReason );
}



//--------------------------------------------------------------
// Name:		setupStateMoveToNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateMoveToNode()
{
	_gotoPoint.SetAnim( _movementAnim );
	_gotoPoint.SetDistance( NODE_RADIUS );
	_gotoPoint.SetPoint( _node->origin );
	_gotoPoint.Begin( *_self );

	setTorsoAnim();
}


//--------------------------------------------------------------
// Name:		evaluateStateMoveToNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateMoveToNode()
{
	BehaviorReturnCode_t result = _gotoPoint.Evaluate( *_self );
	str failureReason = "CorridorCombatWithRangedWeapon::evaluateStateMoveToCover -- _gotoPoint component returned: " + _gotoPoint.GetFailureReason();
	
	if ( result == BEHAVIOR_FAILED )
		failureStateMoveToNode( failureReason );

	return result;		
	
}


//--------------------------------------------------------------
// Name:		failureStateMoveToNode
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler for State Move To Cover
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateMoveToNode( const str &failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		setupStateBackPedal()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateBackPedal()
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
// Name:		evaluateStateBackPedal()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateBackPedal()
{
	BehaviorReturnCode_t moveResult = _moveRandomDir.Evaluate( *_self );

	if ( moveResult == BEHAVIOR_SUCCESS )
		{
		return BEHAVIOR_SUCCESS;            
		}

	if ( moveResult == BEHAVIOR_FAILED )
		{
		failureStateBackPedal( _moveRandomDir.GetFailureReason() );           		
		return BEHAVIOR_FAILED;
		}

	return BEHAVIOR_EVALUATING; 
	
}

//--------------------------------------------------------------
// Name:		failureStateBackPedal()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateBackPedal( const str& failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		setupStateFindBetterNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateFindBetterNode()
{
}

//--------------------------------------------------------------
// Name:		evaluateStateFindBetterNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateFindBetterNode()
{
	if ( _node )
		_node->UnreserveNode();

	_node = HelperNode::FindClosestHelperNodeAtDistanceFrom( *_self , _currentEnemy , NODETYPE_COMBAT , DESCRIPTOR_CORRIDOR , _maxDistance , _retreatDistance );

	if ( !_node )
		{		
		failureStateFindBetterNode( "CorridorCombatWithRangedWeapon::evaluateStateFindBetterNode() -- Cannot Find Suitable Cover Node" );
		return BEHAVIOR_FAILED;
		}

	_node->ReserveNode();
	return BEHAVIOR_SUCCESS; 
}

//--------------------------------------------------------------
// Name:		failureStateFindBetterNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateFindBetterNode( const str& failureReason )
{
	SetFailureReason( failureReason );
}

//--------------------------------------------------------------
// Name:		setupStateMoveToBetterNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateMoveToBetterNode()
{
	_gotoPoint.SetAnim( _movementAnim );
	_gotoPoint.SetDistance( NODE_RADIUS );
	_gotoPoint.SetPoint( _node->origin );
	_gotoPoint.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateMoveToBetterNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateMoveToBetterNode()
{
	BehaviorReturnCode_t result = _gotoPoint.Evaluate( *_self );
	str failureReason = "CorridorCombatWithRangedWeapon::evaluateStateMoveToBetterNode -- _gotoPoint component returned: " + _gotoPoint.GetFailureReason();
	
	if ( result == BEHAVIOR_FAILED )
		failureStateMoveToBetterNode( failureReason );

	return result;		
}

//--------------------------------------------------------------
// Name:		failureStateMoveToBetterNode()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler for state
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateMoveToBetterNode( const str& failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		setupStateChangePostureDuck()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up the ChangePostureDuck state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateChangePostureDuck()
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
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates our ChangePostureDuck state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateChangePostureDuck()
{
	//Basically waiting for our check to come true
	if ( _finishedPostureTransition )
		return BEHAVIOR_SUCCESS;


	return BEHAVIOR_EVALUATING;

}

//--------------------------------------------------------------
// Name:		failureStateChangePostureDuck()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler for ChangePostureDuck State
//
// Parameters:	const str &failureReason -- Why we're failing
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateChangePostureDuck( const str &failureReason )
{
	SetFailureReason( failureReason );
}

//--------------------------------------------------------------
// Name:		setupStateChangePostureStand()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up the ChangePostureStand state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateChangePostureStand()
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
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates our ChangePostureStand state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateChangePostureStand()
{
	//Basically waiting for our check to come true
	if ( _finishedPostureTransition )
		return BEHAVIOR_SUCCESS;
	
	return BEHAVIOR_EVALUATING;

}

//--------------------------------------------------------------
// Name:		failureStateChangePostureStand()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler for Change Posture Stand function
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateChangePostureStand( const str &failureReason )
{
	SetFailureReason( failureReason );
}

//--------------------------------------------------------------
// Name:		setupStateDucked()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up our Ducked State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateDucked()
{	
	_nextPauseTime = level.time + G_Random( _pauseTimeMax ) + _pauseTimeMin;
	setTorsoAnim();
	_self->SetAnim( "duck" , NULL , legs );
	_fireWeapon.End(*_self);
}

//--------------------------------------------------------------
// Name:		evaluateStateDucked()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates our Ducked State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateDucked()
{
	if ( level.time > _nextPauseTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;	

}

//--------------------------------------------------------------
// Name:		failureStateDucked()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler for Ducked State
//
// Parameters:	const str &failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateDucked( const str &failureReason )
{
	SetFailureReason( failureReason );
}

//--------------------------------------------------------------
// Name:		setupStateFireDucked()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateFireDucked()
{
	_nextFireTime = level.time + G_Random( _fireTimeMax ) + _fireTimeMin;
	if ( _self->combatSubsystem->CanAttackEnemy() )
		{
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );
		}
}

//--------------------------------------------------------------
// Name:		evaluateStateFireDucked()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateFireDucked()
{
	_fireWeapon.Evaluate( *_self );
	_self->combatSubsystem->AimWeaponTag(_currentEnemy);	
	
	if ( level.time > _nextFireTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateFireDucked()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler For State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateFireDucked( const str& failureReason )
{
	SetFailureReason( failureReason );
}



//--------------------------------------------------------------
// Name:		setupStateStanding()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Setup for the Standing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateStanding()
{	
	_self->SetAnim( "idle" , NULL , legs);
	_nextPauseTime = level.time + G_Random( _pauseTimeMax ) + _pauseTimeMin;
	setTorsoAnim();
}

//--------------------------------------------------------------
// Name:		evaluateStateStanding()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates the Standing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateStanding()
{
	if ( level.time > _nextPauseTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateStanding()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler for the Standing State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateStanding( const str &failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		setupStateFireStanding()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up our Fire Standing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateFireStanding()
{
	_self->SetAnim( "idle" , NULL , legs);
	_nextFireTime = level.time + G_Random( _fireTimeMax ) + _fireTimeMin;
	//if ( _self->combatSubsystem->CanAttackEnemy() )
	if ( _self->combatSubsystem->CanAttackTarget( _currentEnemy ) )
		{
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );
		}
}

//--------------------------------------------------------------
// Name:		evaluateStateFireStanding()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates our Fire Standing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateFireStanding()
{
	_fireWeapon.Evaluate( *_self );
	_self->combatSubsystem->AimWeaponTag(_currentEnemy);	

	if ( level.time > _nextFireTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateFireStanding()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler for Fire Standing State
//
// Parameters:	const str &failureReason -- Why we're failing
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateFireStanding( const str &failureReason )
{
	SetFailureReason( failureReason );
}

//--------------------------------------------------------------
// Name:		setupStateHoldPosition()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupStateHoldPosition()
{
	_self->SetAnim( "idle" );
	setTorsoAnim();
	_holdPositionTime = level.time + G_Random() + 1.0f;
}

//--------------------------------------------------------------
// Name:		evaluateStateHoldPosition()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateStateHoldPosition()
{
	if ( level.time >= _holdPositionTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateHoldPosition()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Failure Handler
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::failureStateHoldPosition( const str& failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		setupRotate
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets up Rotate Component
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setupRotate()
{
	_rotate.SetTurnSpeed( 30.0f );
	_rotate.SetEntity( _currentEnemy ); 
	_rotate.Begin( *_self );
}


//--------------------------------------------------------------
// Name:		evaluateRotate()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Evaluates our Rotate Component
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CorridorCombatWithRangedWeapon::evaluateRotate()
{
	return _rotate.Evaluate( *_self );
}


//--------------------------------------------------------------
// Name:		setTorsoAnim()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Sets our Torso Animation
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CorridorCombatWithRangedWeapon::setTorsoAnim()
{
	_self->SetAnim( _torsoAnim , NULL , torso );
}

//--------------------------------------------------------------
// Name:		checkShouldDuck()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Checks if actor should duck
//
// Parameters:	None
//	
// Returns:		true or false
//--------------------------------------------------------------
bool CorridorCombatWithRangedWeapon::checkShouldDuck()
{
	// We don't duck if the enemy is too close
	if ( _self->WithinDistanceXY( _currentEnemy , _threatDistance ) )
		return false;

	if ( G_Random() <= _postureChangeChance )
		return true;

	return false;
}

//--------------------------------------------------------------
// Name:		checkShouldStand()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Checks if actor should stand
//
// Parameters:	None
//
// Returns:		true or false
//--------------------------------------------------------------
bool CorridorCombatWithRangedWeapon::checkShouldStand()
{
	if ( _self->WithinDistanceXY( _currentEnemy , _threatDistance ) )
		return true;

	return false;
}

bool CorridorCombatWithRangedWeapon::checkShouldRetreat()
{
	if ( level.time >= _holdPositionTime && _self->WithinDistanceXY( _currentEnemy , _retreatDistance ) )
		return true;

	return false;
	
}

//--------------------------------------------------------------
// Name:		CanExecute()
// Class:		CorridorCombatWithRangedWeapon
//
// Description:	Checks if the behavior can execute
//
// Parameters:	float maxDistance
//				Actor &self
//
// Returns:		true or false
//--------------------------------------------------------------
bool CorridorCombatWithRangedWeapon::CanExecute( float maxDistance , Actor &self )
{
	HelperNodePtr node;
	node = HelperNode::FindClosestHelperNode( self , NODETYPE_COMBAT , DESCRIPTOR_CORRIDOR , maxDistance );	

	if ( node )
		return true;

	return false;
}


