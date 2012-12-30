//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/useAlarm.cpp            $
// $Revision:: 10                                                             $
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
#include "useAlarm.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, UseAlarm, NULL )
{
	{ &EV_Behavior_Args,		&UseAlarm::SetArgs				},
	{ &EV_Behavior_AnimDone,	&UseAlarm::AnimDone				}, 
	{ &EV_HelperNodeCommand,	&UseAlarm::HandleNodeCommand	},
	{ NULL, NULL }
};

//--------------------------------------------------------------
// Name:		UseAlarm()
// Class:		UseAlarm()
//	
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
UseAlarm::UseAlarm()
{
	_movementAnimName = "walk";
	_maxDistance	  = 1024.0f;
}

//--------------------------------------------------------------
// Name:		~UseAlarm()
// Class:		UseAlarm
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
UseAlarm::~UseAlarm()
{
}

//--------------------------------------------------------------
// Name:         SetArgs()
// Class:        UseAlarm
//
// Description:  Sets the arguments of the behavior 
//
// Parameters:   Event *ev
//
// Returns:      None
//--------------------------------------------------------------
void UseAlarm::SetArgs ( Event *ev)
{
	if (ev->NumArgs() == 1)
	{
		_movementAnimName = ev->GetString( 1 );
	}
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       UseAlarm
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void UseAlarm::AnimDone( Event *ev )
{
	switch( _state )
		{
		case USE_ALARM_WAIT_ON_ANIM:
			_animDone = true;
		break;
		}

}

//--------------------------------------------------------------
// Name:		HandleNodeCommand()
// Class:		UseAlarm
//
// Description:	Handles a command event from a helper node
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::HandleNodeCommand( Event *ev )
{
}

//--------------------------------------------------------------
// Name:        Begin()
// Class:       UseAlarm
//
// Description: Begins the Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void UseAlarm::Begin( Actor &self )
{
	init( self );
}

//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       UseAlarm
//
// Description: Returns True Or False, and is run every frame
//              that the behavior
//
// Parameters:  Actor &self
//
// Returns:     True or False
//--------------------------------------------------------------
BehaviorReturnCode_t UseAlarm::Evaluate ( Actor &self )
{      
	BehaviorReturnCode_t stateResult;

	think();

	switch ( _state )
		{
		//---------------------------------------------------------------------
		case USE_ALARM_FIND_NODE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateFindNode();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( USE_ALARM_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( USE_ALARM_MOVE_TO_NODE ); 
		break;
		

		//---------------------------------------------------------------------
		case USE_ALARM_MOVE_TO_NODE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateMoveToNode();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( USE_ALARM_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( USE_ALARM_AT_NODE ); 

		break;

		//---------------------------------------------------------------------
		case USE_ALARM_AT_NODE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAtNode();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( USE_ALARM_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( USE_ALARM_WAIT_ON_ANIM ); 

		break;

		//---------------------------------------------------------------------
		case USE_ALARM_WAIT_ON_ANIM:
		//---------------------------------------------------------------------
			stateResult = evaluateStateWaitOnAnim();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( USE_ALARM_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( USE_ALARM_ROTATE_TO_ENEMY ); 
		break;

		//---------------------------------------------------------------------
		case USE_ALARM_ROTATE_TO_ENEMY:
		//---------------------------------------------------------------------
			stateResult = evaluateStateRotateToEnemy();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( USE_ALARM_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( USE_ALARM_SUCCESSFUL ); 

		break;

		//---------------------------------------------------------------------
		case USE_ALARM_SUCCESSFUL:
		//---------------------------------------------------------------------
			_node->RunExitThread();
			return BEHAVIOR_SUCCESS;
		break;

		//---------------------------------------------------------------------
		case USE_ALARM_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;
		break;

		}

   return BEHAVIOR_EVALUATING; 
}



//--------------------------------------------------------------
// Name:         End()
// Class:        UseAlarm
//
// Description:  Ends the Behavior
//
// Parameters:   Actor &self
//
// Returns:      None
//--------------------------------------------------------------
void UseAlarm::End ( Actor &self	)
{
	if ( _node )
	{
		_node->SetUser( NULL );
	}
}

//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		UseAlarm
//
// Description:	Transitions Internal State
//
// Parameters:	workStates_t state
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::transitionToState ( useAlarmStates_t state )
{
	switch ( state )
		{
		case USE_ALARM_FIND_NODE:
			setupStateFindNode();
			setInternalState( state , "USE_ALARM_FIND_NODE" );
		break;

		case USE_ALARM_MOVE_TO_NODE:
			setupStateMoveToNode();
			setInternalState( state , "USE_ALARM_MOVE_TO_NODE" );
		break;

		case USE_ALARM_AT_NODE:
			setupStateAtNode();
			setInternalState( state , "USE_ALARM_AT_NODE" );
		break;

		case USE_ALARM_WAIT_ON_ANIM:
			setupStateWaitOnAnim();
			setInternalState( state , "WORK_ANIMATE_WAIT_ON_ANIM" );
		break;

		case USE_ALARM_ROTATE_TO_ENEMY:
			setupStateRotateToEnemy();
			setInternalState( state, "USE_ALARM_ROTATE_TO_ENEMY" );
		break;

		case USE_ALARM_SUCCESSFUL:			
			setInternalState( state , "WORK_SUCCESSFUL" );
		break;

		case USE_ALARM_FAILED:
			setInternalState( state , "WORK_FAILED" );
		break;		
		}

}

//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		UseAlarm
//
// Description:	Sets internal state of the behavior
//
// Parameters:	workStates_t state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::setInternalState ( useAlarmStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}



//--------------------------------------------------------------
// Name:        init()
// Class:       UseAlarm
//
// Description: Initializes memeber variables
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void UseAlarm::init( Actor &self )
{
	_self			= &self;
	_animDone		= false;	
	transitionToState( USE_ALARM_FIND_NODE );
}

//--------------------------------------------------------------
// Name:		think()
// Class:		UseAlarm
//
// Description:	Think function called every frame by evaluate
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::think()
{
}

//--------------------------------------------------------------
// Name:		setupStateFindNode()
// Class:		UseAlarm
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::setupStateFindNode()
{
	str currentNodeCustomType;
	if ( _self->currentHelperNode.node )
		currentNodeCustomType = _self->currentHelperNode.node->GetCustomType();

	if ( currentNodeCustomType == "alarm" )
		_node = &(*_self->currentHelperNode.node);
	else
		_node = HelperNode::FindClosestHelperNode( *_self , "alarm" , _maxDistance );	
}

//--------------------------------------------------------------
// Name:		evaluateStateFindNode()
// Class:		UseAlarm
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t UseAlarm::evaluateStateFindNode()
{
	if ( !_node )
		return BEHAVIOR_FAILED;

	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateFindNode()
// Class:		UseAlarm
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::failureStateFindNode( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateMoveToNode()
// Class:		UseAlarm
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::setupStateMoveToNode()
{
	_gotoHelperNode.SetNode( _node );
	_gotoHelperNode.SetMovementAnim ( _movementAnimName );
	_gotoHelperNode.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateMoveToNode()
// Class:		UseAlarm
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t UseAlarm::evaluateStateMoveToNode()
{
	return _gotoHelperNode.Evaluate( *_self );
}

//--------------------------------------------------------------
// Name:		failureStateMoveToNode()
// Class:		UseAlarm
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::failureStateMoveToNode( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateAtNode()
// Class:		UseAlarm
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::setupStateAtNode()
{
	_self->SetAnim( "idle" );
	_self->RunThread( _node->GetEntryThread() );
}

//--------------------------------------------------------------
// Name:		evaluateStateAtNode()
// Class:		UseAlarm
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t UseAlarm::evaluateStateAtNode()
{
	Vector nodeAngles = _node->angles;
	
	_self->movementSubsystem->setAnimDir( nodeAngles );					
	_self->setAngles( nodeAngles );

	_node->SetUser( this );
	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateAtNode()
// Class:		UseAlarm
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::failureStateAtNode( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateWaitOnAnim()
// Class:		UseAlarm
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::setupStateWaitOnAnim()
{
	if( ! _self->SetAnim( _node->GetAnim() , EV_Actor_NotifyBehavior , legs ) )
	{
		// setanim returns false if the node-specified anim doesn't exist, so 
		// set _animDone=true to avoid them just standing there.
		_animDone = true;
	}
	else
	{
		// anim was set successfully, so let the behavior know it should wait for the
		// anim to finish before continuing.
		_animDone = false;
	}
}

//--------------------------------------------------------------
// Name:		evaluateStateWaitOnAnim()
// Class:		UseAlarm
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t UseAlarm::evaluateStateWaitOnAnim()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateWaitOnAnim()
// Class:		UseAlarm
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::failureStateWaitOnAnim( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateRotateToEnemy()
// Class:		UseAlarm
//
// Description:	Sets up state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::setupStateRotateToEnemy()
{
	Entity *enemy = _self->enemyManager->GetCurrentEnemy();

	if ( !enemy )
		return;

	_rotateToEntity.SetAnim( "idle" );
	_rotateToEntity.SetEntity( enemy );
	_rotateToEntity.SetTurnSpeed( 25.0f );
	_rotateToEntity.Begin( *_self );

}

//--------------------------------------------------------------
// Name:		evaluateStateRotateToEnemy()
// Class:		UseAlarm
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t UseAlarm::evaluateStateRotateToEnemy()
{
	return _rotateToEntity.Evaluate( *_self );
}

//--------------------------------------------------------------
// Name:		failureStateRotateToEnemy()
// Class:		UseAlarm
//
// Description:	Failure Handler for State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void UseAlarm::failureStateRotateToEnemy( const str &failureReason )
{

}

