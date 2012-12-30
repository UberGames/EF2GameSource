//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/gotoHelperNode.cpp                         $
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
//	CoverCombatWithRangedWeapon Implementation
// 
// PARAMETERS:
//		str _nodeType	   -- The name of the node to move to
//		str _movementAnim  -- The animation to play while moving to the node
//		float _maxDistance -- The maximum distance to look for a node
//
// ANIMATIONS:
//		_movementAnim : PARAMETER
//--------------------------------------------------------------------------------

#include "actor.h"
#include "gotoHelperNode.hpp"

//--------------------------------------------------------------
//
// Init Static Vars
//
//--------------------------------------------------------------
const float GotoHelperNode::NODE_RADIUS = 32.0f;

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, GotoHelperNode, NULL )
	{
		{ &EV_Behavior_Args,		&GotoHelperNode::SetArgs	},
		{ &EV_Behavior_AnimDone,	&GotoHelperNode::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		GotoHelperNode()
// Class:		GotoHelperNode
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GotoHelperNode::GotoHelperNode()
{
	_movementAnim = "";
	_nodeType = "" ;
	_maxDistance = 256;
}

//--------------------------------------------------------------
// Name:		GotoHelperNode()
// Class:		GotoHelperNode
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GotoHelperNode::~GotoHelperNode()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       GotoHelperNode
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNode::SetArgs( Event *ev )
{
	_nodeType	  = ev->GetString( 1 );
	_movementAnim = ev->GetString( 2 );
	_maxDistance  = ev->GetFloat ( 3 );
}


//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       GotoHelperNode
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNode::AnimDone( Event *ev )
{
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       GotoHelperNode
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNode::Begin( Actor &self )
	{       
	init( self );
	transitionToState ( GOTO_HNODE_FIND_NODE );
	}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       GotoHelperNode
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t GotoHelperNode::Evaluate( Actor &self )
{
	BehaviorReturnCode_t stateResult;

	think();
	
	switch ( _state )
		{
		//---------------------------------------------------------------------
		case GOTO_HNODE_FIND_NODE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateFindNode();

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( GOTO_HNODE_MOVE_TO_NODE );
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( GOTO_HNODE_FAILED );
		break;

		//---------------------------------------------------------------------
		case GOTO_HNODE_MOVE_TO_NODE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateMoveToNode();

			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( GOTO_HNODE_FAILED );
			
			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( GOTO_HNODE_SUCCESS );
		break;


		//---------------------------------------------------------------------
		case GOTO_HNODE_SUCCESS:
		//---------------------------------------------------------------------
			return BEHAVIOR_SUCCESS;

		break;


		//---------------------------------------------------------------------
		case GOTO_HNODE_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;

		break;


		}
	

	return BEHAVIOR_EVALUATING;   

}



//--------------------------------------------------------------
// Name:        End()
// Class:       GotoHelperNode
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNode::End(Actor &self)
{   
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		GotoHelperNode
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNode::transitionToState( GotoHelperNodeStates_t state )
{	
	switch ( state )
		{	
		case GOTO_HNODE_FIND_NODE:
			setupStateFindNode();
			setInternalState( state , "GOTO_HNODE_FIND_NODE" );
		break;

		case GOTO_HNODE_MOVE_TO_NODE:
			setupStateMoveToNode();
			setInternalState( state , "GOTO_HNODE_MOVE_TO_NODE" );
		break;

		case GOTO_HNODE_SUCCESS:
			setInternalState( state , "GOTO_HNODE_SUCCESS" );
		break;

		case GOTO_HNODE_FAILED:
			setInternalState( state , "GOTO_HNODE_FAILED" );
		break;
		}
	
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		GotoHelperNode
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNode::setInternalState( GotoHelperNodeStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		init()
// Class:		GotoHelperNode
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNode::init( Actor &self )
{
	_self = &self;	
}

//--------------------------------------------------------------
// Name:		think()
// Class:		GotoHelperNode
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNode::think()
{
}



//--------------------------------------------------------------
// Name:		setupStateMoveToNode()
// Class:		GotoHelperNode
//
// Description:	Sets up the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNode::setupStateMoveToNode()
{
	_gotoPoint.SetAnim( _movementAnim );
	_gotoPoint.SetDistance( NODE_RADIUS );
	_gotoPoint.SetPoint( _node->origin );
	_gotoPoint.Begin( *_self );
}


//--------------------------------------------------------------
// Name:		evaluateStateMoveToNode()
// Class:		GotoHelperNode
//
// Description:	Evaluates the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GotoHelperNode::evaluateStateMoveToNode()
{
	BehaviorReturnCode_t result = _gotoPoint.Evaluate( *_self );
	str failureReason = "GotoHelperNode::evaluateStateMoveToNode -- _gotoPoint component returned: " + _gotoPoint.GetFailureReason();
	
	if ( result == BEHAVIOR_FAILED )
		failureStateMoveToNode( failureReason );

	return result;	
}


//--------------------------------------------------------------
// Name:		evaluateStateFindNode()
// Class:		GotoHelperNode
//
// Description:	Evaluates the FindNode State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------

BehaviorReturnCode_t GotoHelperNode::evaluateStateFindNode()
{
	if ( _node )
		return BEHAVIOR_SUCCESS;

	BehaviorReturnCode_t result = BEHAVIOR_SUCCESS;
	str failureReason = "GotoHelperNode::evaluateStateFindNode -- could not find an appropriate node";

	unsigned int NodeMask = HelperNode::GetHelperNodeMask( _nodeType );

	_node = HelperNode::FindClosestHelperNode( *_self , NodeMask , _maxDistance );

	if ( !_node )
	{
		failureStateFindNode( failureReason );
		result = BEHAVIOR_FAILED;
	}


	return result;
}

//--------------------------------------------------------------
// Name:		setupStateFindNode()
// Class:		GotoHelperNode
//
// Description:	Sets up the Find State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNode::setupStateFindNode()
{
}


//--------------------------------------------------------------
// Name:		failureStateMoveToNode
// Class:		GotoHelperNode
//
// Description:	Failure Handler for State Move To Cover
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNode::failureStateMoveToNode( const str &failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		failureStateFindNode
// Class:		GotoHelperNode
//
// Description:	Failure Handler for State Move To Cover
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNode::failureStateFindNode( const str &failureReason )
{
	SetFailureReason( failureReason );
}

