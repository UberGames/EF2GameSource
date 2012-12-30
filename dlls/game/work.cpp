//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/work.cpp         $
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
//	CorridorCombatWithRangedWeapon Implementation
// 
// PARAMETERS:
//		str _movementAnim -- The animation to play while moving to the cover node
//
// ANIMATIONS:
//		_movementAnim : PARAMETER
//--------------------------------------------------------------------------------

#include "actor.h"
#include "work.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, Work, NULL )
{
	{ &EV_Behavior_Args,		&Work::SetArgs				},
	{ &EV_Behavior_AnimDone,	&Work::AnimDone				}, 
	{ &EV_HelperNodeCommand,	&Work::HandleNodeCommand	},

	{ NULL, NULL }
};

//--------------------------------------------------------------
// Name:		Work()
// Class:		Work()
//	
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
Work::Work()
{
	_gotoWorkAnimName = "walk";
	_maxDistance	  = 1024.0f;
}

//--------------------------------------------------------------
// Name:		~Work()
// Class:		Work
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
Work::~Work()
{
}

//--------------------------------------------------------------
// Name:         SetArgs()
// Class:        Work
//
// Description:  Sets the arguments of the behavior 
//
// Parameters:   Event *ev
//
// Returns:      None
//--------------------------------------------------------------
void Work::SetArgs ( Event *ev)
{
	if (ev->NumArgs() == 1)
	{
		_gotoWorkAnimName = ev->GetString( 1 );
	}
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       Work
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void Work::AnimDone( Event *ev )
{
	switch( _state )
		{
		case WORK_ANIMATE_WAIT_ON_ANIM:
			_animDone = true;
		break;

		case WORK_ANIMATE_LIST_WAIT_ON_ANIM:
			_animDone = true;
		break;		
		}

}

//--------------------------------------------------------------
// Name:		HandleNodeCommand()
// Class:		Work
//
// Description:	Handles a command event from a helper node
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Work::HandleNodeCommand( Event *ev )
{
	switch( _state )
		{
		case WORK_ANIMATE_WAIT_ON_SIGNAL:
			_animDone = true;
		break;

		case WORK_ANIMATE_LIST_WAIT_ON_SIGNAL:
			_animDone = true;
		break;
		}
}

//--------------------------------------------------------------
// Name:        Begin()
// Class:       Work
//
// Description: Begins the Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Work::Begin( Actor &self )
{
	init( self );
}

//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       Work
//
// Description: Returns True Or False, and is run every frame
//              that the behavior
//
// Parameters:  Actor &self
//
// Returns:     True or False
//--------------------------------------------------------------
BehaviorReturnCode_t	Work::Evaluate ( Actor &self )
{      
	BehaviorReturnCode_t stateResult;

	think();

	switch ( _state )
		{
		//---------------------------------------------------------------------
		case WORK_FIND_NODE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateFindNode();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WORK_MOVE_TO_NODE ); 
		break;

		//---------------------------------------------------------------------
		case WORK_MOVE_TO_NODE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateMoveToNode();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WORK_AT_NODE ); 

		break;

		//---------------------------------------------------------------------
		case WORK_AT_NODE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAtNode();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WORK_SELECT_ANIM_MODE ); 

		break;

		//---------------------------------------------------------------------
		case WORK_SELECT_ANIM_MODE:
		//---------------------------------------------------------------------
			if ( _node->isUsingAnimList() )
				{
				customAnimListEntry_t* animEntry = _node->GetCurrentAnimEntryFromList();
				if ( !animEntry )
					{
					transitionToState( WORK_FAILED );
					return BEHAVIOR_EVALUATING;
					}

				if ( animEntry->waitType == WAITTYPE_EVENT )
					transitionToState( WORK_ANIMATE_LIST_WAIT_ON_SIGNAL );
					
				if ( animEntry->waitType == WAITTYPE_ANIM )
					transitionToState( WORK_ANIMATE_LIST_WAIT_ON_ANIM );

				if ( animEntry->waitType == WAITTYPE_TIME )
					transitionToState( WORK_ANIMATE_LIST_WAIT_ON_TIME );

				return BEHAVIOR_EVALUATING;					
				}

			if ( _node->isWaitForAnim() )
				{
				transitionToState( WORK_ANIMATE_WAIT_ON_ANIM );
				return BEHAVIOR_EVALUATING;
				}

			if ( _node->GetWaitTime() > 0 )
				{
				transitionToState( WORK_ANIMATE_WAIT_ON_TIME );
				return BEHAVIOR_EVALUATING;
				}

			// We default to constant working
			transitionToState( WORK_ANIMATE_CONSTANT );
			
			
		break;

		//---------------------------------------------------------------------
		case WORK_ANIMATE_WAIT_ON_TIME:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAnimateWaitOnTime();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WORK_SUCCESSFUL ); 
		break;

		//---------------------------------------------------------------------
		case WORK_ANIMATE_WAIT_ON_ANIM:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAnimateWaitOnAnim();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WORK_SUCCESSFUL ); 
		break;

		//---------------------------------------------------------------------
		case WORK_ANIMATE_WAIT_ON_SIGNAL:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAnimateWaitOnSignal();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WORK_SUCCESSFUL ); 
		break;

		//---------------------------------------------------------------------
		case WORK_ANIMATE_CONSTANT:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAnimateConstant();

			//
			// Should Never get to either of these
			//
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WORK_SUCCESSFUL ); 
		break;

		//---------------------------------------------------------------------
		case WORK_ANIMATE_LIST_WAIT_ON_TIME:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAnimateListWaitOnTime();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				if (_node->isAnimListFinished() )
					transitionToState( WORK_SUCCESSFUL ); 				
				else
					transitionToState( WORK_SELECT_ANIM_MODE );				
				}
				
		break;

		//---------------------------------------------------------------------
		case WORK_ANIMATE_LIST_WAIT_ON_ANIM:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAnimateListWaitOnAnim();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				if (_node->isAnimListFinished() )
					transitionToState( WORK_SUCCESSFUL ); 				
				else
					transitionToState( WORK_SELECT_ANIM_MODE );				
				}
		break;

		//---------------------------------------------------------------------
		case WORK_ANIMATE_LIST_WAIT_ON_SIGNAL:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAnimateListWaitOnSignal();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WORK_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				if (_node->isAnimListFinished() )
					transitionToState( WORK_SUCCESSFUL ); 				
				else
					transitionToState( WORK_SELECT_ANIM_MODE );				
				}
		break;

		//---------------------------------------------------------------------
		case WORK_SUCCESSFUL:
		//---------------------------------------------------------------------
			_node->RunExitThread();
			self.ignoreHelperNode.node = _node;
			return BEHAVIOR_SUCCESS;
		break;

		//---------------------------------------------------------------------
		case WORK_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;
		break;

		}

   return BEHAVIOR_EVALUATING; 
}



//--------------------------------------------------------------
// Name:         End()
// Class:        Work
//
// Description:  Ends the Behavior
//
// Parameters:   Actor &self
//
// Returns:      None
//--------------------------------------------------------------
void Work::End ( Actor &self	)
{
	if ( _node )
	{
		_node->SetUser( NULL );
	}
}

//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		Work
//
// Description:	Transitions Internal State
//
// Parameters:	workStates_t state
//
// Returns:		None
//--------------------------------------------------------------
void Work::transitionToState ( workStates_t state )
{
	switch ( state )
		{
		case WORK_FIND_NODE:
			setupStateFindNode();
			setInternalState( state , "WORK_FIND_NODE" );
		break;

		case WORK_MOVE_TO_NODE:
			setupStateMoveToNode();
			setInternalState( state , "WORK_MOVE_TO_NODE" );
		break;

		case WORK_AT_NODE:
			setupStateAtNode();
			setInternalState( state , "WORK_AT_NODE" );
		break;

		case WORK_ANIMATE_WAIT_ON_TIME:
			setupStateAnimateWaitOnTime();
			setInternalState( state , "WORK_ANIMATE_WAIT_ON_TIME" );
		break;

		case WORK_ANIMATE_WAIT_ON_ANIM:
			setupStateAnimateWaitOnAnim();
			setInternalState( state , "WORK_ANIMATE_WAIT_ON_ANIM" );
		break;

		case WORK_ANIMATE_WAIT_ON_SIGNAL:
			setupStateAnimateWaitOnSignal();
			setInternalState( state , "WORK_ANIMATE_WAIT_ON_SIGNAL" );
		break;

		case WORK_ANIMATE_CONSTANT:
			setupStateAnimateConstant();
			setInternalState( state , "WORK_ANIMATE_CONSTANT" );
		break;

		case WORK_ANIMATE_LIST_WAIT_ON_TIME:
			setupStateAnimateListWaitOnTime();
			setInternalState( state , "WORK_ANIMATE_LIST_WAIT_ON_TIME" );
		break;

		case WORK_ANIMATE_LIST_WAIT_ON_ANIM:
			setupStateAnimateListWaitOnAnim();
			setInternalState( state , "WORK_ANIMATE_LIST_WAIT_ON_ANIM" );
		break;

		case WORK_ANIMATE_LIST_WAIT_ON_SIGNAL:
			setupStateAnimateListWaitOnSignal();
			setInternalState( state , "WORK_ANIMATE_LIST_WAIT_ON_SIGNAL" );
		break;

		case WORK_SELECT_ANIM_MODE:
			setInternalState( state, "WORK_SELECT_ANIM_MODE," );
		break;

		case WORK_SUCCESSFUL:
			setInternalState( state , "WORK_SUCCESSFUL" );
		break;

		case WORK_FAILED:
			setInternalState( state , "WORK_FAILED" );
		break;
		}

}

//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		Work
//
// Description:	Sets internal state of the behavior
//
// Parameters:	workStates_t state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void Work::setInternalState ( workStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}



//--------------------------------------------------------------
// Name:        init()
// Class:       Work
//
// Description: Initializes memeber variables
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void Work::init( Actor &self )
{
	_self			= &self;
	_animDone		= false;
	_endTime		= 0.0f;
	_node = self.currentHelperNode.node;
	transitionToState( WORK_FIND_NODE );
		
}

//--------------------------------------------------------------
// Name:		think()
// Class:		Work
//
// Description:	Think function called every frame by evaluate
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::think()
{
	// We need to mindful of changes to our node,
	// so we'll keep an eye out for them.  If 
	// we note a change, then we'll re-evaluate
	if ( _node && _node->isChanged() )
		{
		_node->SetCriticalChange( false );      
		}
}

//--------------------------------------------------------------
// Name:		setupStateFindNode()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateFindNode()
{
	if ( !_node )
		_node = HelperNode::FindClosestHelperNode( *_self , NODETYPE_WORK , _maxDistance );
}

//--------------------------------------------------------------
// Name:		evaluateStateFindNode()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateFindNode()
{
	if ( !_node )
		return BEHAVIOR_FAILED;

	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateFindNode()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateFindNode( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateMoveToNode()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateMoveToNode()
{
	_gotoHelperNode.SetNode( _node );
	_gotoHelperNode.SetMovementAnim ( _gotoWorkAnimName );
	_gotoHelperNode.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateMoveToNode()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateMoveToNode()
{
	return _gotoHelperNode.Evaluate( *_self );
}

//--------------------------------------------------------------
// Name:		failureStateMoveToNode()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateMoveToNode( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateAtNode()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateAtNode()
{
	_self->RunThread( _node->GetEntryThread() );
}

//--------------------------------------------------------------
// Name:		evaluateStateAtNode()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateAtNode()
{
	Vector nodeAngles = _node->angles;
	
	_self->movementSubsystem->setAnimDir( nodeAngles );					
	_self->setAngles( nodeAngles );

	_node->SetUser( this );
	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateAtNode()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateAtNode( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateAnimateWaitOnTime()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateAnimateWaitOnTime()
{
	if ( _node->isWaitRandom() )
		_endTime = level.time + G_Random(_node->GetWaitTime());
	else
		_endTime = level.time + _node->GetWaitTime();

	_self->SetAnim( _node->GetAnim() , EV_Actor_NotifyBehavior , legs );

}

//--------------------------------------------------------------
// Name:		evaluateStateAnimateWaitOnTime()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateAnimateWaitOnTime()
{
	if ( level.time >= _endTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAnimateWaitOnTime()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateAnimateWaitOnTime( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateAnimateWaitOnAnim()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateAnimateWaitOnAnim()
{
	_animDone = false;
	_self->SetAnim( _node->GetAnim() , EV_Actor_NotifyBehavior , legs );
}

//--------------------------------------------------------------
// Name:		evaluateStateAnimateWaitOnAnim()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateAnimateWaitOnAnim()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAnimateWaitOnAnim()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateAnimateWaitOnAnim( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateAnimateWaitOnSignal()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateAnimateWaitOnSignal()
{
	_animDone = false;
	_self->SetAnim( _node->GetAnim() , EV_Actor_NotifyBehavior , legs );
}

//--------------------------------------------------------------
// Name:		evaluateStateAnimateWaitOnSignal()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateAnimateWaitOnSignal()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAnimateWaitOnSignal()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateAnimateWaitOnSignal( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateAnimateConstant()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateAnimateConstant()
{
	_self->SetAnim( _node->GetAnim() , EV_Actor_NotifyBehavior , legs );
}

//--------------------------------------------------------------
// Name:		evaluateStateAnimateConstant()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateAnimateConstant()
{
	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAnimateConstant()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateAnimateConstant( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateAnimateListWaitOnTime()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateAnimateListWaitOnTime()
{
	customAnimListEntry_t* animListEntry;
	animListEntry = _node->GetCurrentAnimEntryFromList();

	if ( !animListEntry )
		{
		_endTime = 0.0f;
		return;
		}
		
	_endTime = level.time + animListEntry->time;
	_self->SetAnim( animListEntry->anim , EV_Actor_NotifyBehavior , legs );
}

//--------------------------------------------------------------
// Name:		evaluateStateAnimateListWaitOnTime()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateAnimateListWaitOnTime()
{
	if ( level.time >= _endTime )
		{
		_node->NextAnim();
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAnimateListWaitOnTime()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateAnimateListWaitOnTime( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateAnimateListWaitOnAnim()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateAnimateListWaitOnAnim()
{
	customAnimListEntry_t* animListEntry;
	animListEntry = _node->GetCurrentAnimEntryFromList();

	if ( !animListEntry )
		{
		_animDone = true;
		return;
		}

	_animDone = false;
	_self->SetAnim( animListEntry->anim , EV_Actor_NotifyBehavior , legs );
}

//--------------------------------------------------------------
// Name:		evaluateStateAnimateListWaitOnAnim()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateAnimateListWaitOnAnim()
{
	if ( _animDone )
		{
		_node->NextAnim();
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAnimateListWaitOnAnim()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateAnimateListWaitOnAnim( const str& failureReason )
{
}



//--------------------------------------------------------------
// Name:		setupStateAnimateListWaitOnSignal()
// Class:		Work
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Work::setupStateAnimateListWaitOnSignal()
{
	customAnimListEntry_t* animListEntry;
	animListEntry = _node->GetCurrentAnimEntryFromList();

	if ( !animListEntry )
		{
		_animDone = true;
		return;
		}

	_animDone = false;
	_self->SetAnim( animListEntry->anim , EV_Actor_NotifyBehavior , legs );
}

//--------------------------------------------------------------
// Name:		evaluateStateAnimateListWaitOnSignal()
// Class:		Work
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t Work::evaluateStateAnimateListWaitOnSignal()
{
	if ( _animDone )
		{
		_node->NextAnim();
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAnimateListWaitOnSignal()
// Class:		Work
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason -- Why we failed
//
// Returns:		None
//--------------------------------------------------------------
void Work::failureStateAnimateListWaitOnSignal( const str& failureReason )
{
}



