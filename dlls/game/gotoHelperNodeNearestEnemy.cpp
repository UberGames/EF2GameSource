//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/gotoHelperNodeNearestEnemy.cpp             $
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
//		str _movementAnim -- The animation to play while moving to the cover node
//
// ANIMATIONS:
//		_movementAnim : PARAMETER
//--------------------------------------------------------------------------------

#include "actor.h"
#include "gotoHelperNodeNearestEnemy.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, GotoHelperNodeNearestEnemy, NULL )
	{
		{ &EV_Behavior_Args,		&GotoHelperNodeNearestEnemy::SetArgs	},
		{ &EV_Behavior_AnimDone,	&GotoHelperNodeNearestEnemy::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		GotoHelperNodeNearestEnemy()
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GotoHelperNodeNearestEnemy::GotoHelperNodeNearestEnemy()
{	
}

//--------------------------------------------------------------
// Name:		GotoHelperNodeNearestEnemy()
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GotoHelperNodeNearestEnemy::~GotoHelperNodeNearestEnemy()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       GotoHelperNodeNearestEnemy
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::SetArgs( Event *ev )
{
	_nodeType = ev->GetString( 1 );
	_movementAnim = ev->GetString( 2 );
	_maxDistance = ev->GetFloat( 3 );
		
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       GotoHelperNodeNearestEnemy
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::AnimDone( Event *ev )
{
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       GotoHelperNodeNearestEnemy
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::Begin( Actor &self )
	{       
	init( self );
	transitionToState ( GOTO_HNODE_FIND_NODE );
	}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       GotoHelperNodeNearestEnemy
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t GotoHelperNodeNearestEnemy::Evaluate( Actor &self )
{
	BehaviorReturnCode_t stateResult;

	think();
	
	switch ( _state )
		{
		//---------------------------------------------------------------------
		case GOTO_HNODE_FIND_NODE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateFindNode();

			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( GOTO_HNODE_FAILED );
			else
				transitionToState( GOTO_HNODE_MOVE_TO_NODE );

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
// Class:       GotoHelperNodeNearestEnemy
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::End(Actor &self)
{   
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::transitionToState( GotoHelperNodeStates_t state )
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
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::setInternalState( GotoHelperNodeStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		init()
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::init( Actor &self )
{
	_self = &self;	
	updateEnemy();
}

//--------------------------------------------------------------
// Name:		think()
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::think()
{
}

//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::updateEnemy()
{
	Entity *currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !currentEnemy )
			{
			SetFailureReason( "GotoHelperNodeNearestEnemy::updateEnemy -- No Enemy" );
			transitionToState( GOTO_HNODE_FAILED );
			return;
			}
			
		}

	_currentEnemy = currentEnemy;
}


void GotoHelperNodeNearestEnemy::setupStateFindNode()
{
	unsigned int NodeMask = HelperNode::GetHelperNodeMask( _nodeType );

	if ( !_currentEnemy )
		return;

	_node = HelperNode::FindHelperNodeClosestTo( *_self , _currentEnemy , NodeMask , _maxDistance );

	if ( !_node )
		_node = HelperNode::FindHelperNodeClosestToWithoutPathing( *_self , _currentEnemy , NodeMask , _maxDistance );
				
}

BehaviorReturnCode_t GotoHelperNodeNearestEnemy::evaluateStateFindNode()
{
	if ( !_node )
		{
		str failureMsg = "No Node of Type: " + _nodeType + " in range";
		failureStateFindNode( failureMsg );
		return BEHAVIOR_FAILED;
		}

	return BEHAVIOR_SUCCESS;
}

void GotoHelperNodeNearestEnemy::failureStateFindNode(const str &failureReason )
{
	SetFailureReason( failureReason );
}

//--------------------------------------------------------------
// Name:		setupStateMoveToNode()
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Sets up the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::setupStateMoveToNode()
{
	_gotoHNode.SetNode( _node );
	_gotoHNode.SetMovementAnim( _movementAnim );
	_gotoHNode.Begin( *_self );
}


//--------------------------------------------------------------
// Name:		evaluateStateMoveToNode()
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Evaluates the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GotoHelperNodeNearestEnemy::evaluateStateMoveToNode()
{
	BehaviorReturnCode_t result = _gotoHNode.Evaluate( *_self );	
	
	if ( result == BEHAVIOR_FAILED )
		{
		str failureReason = "GotoHelperNodeNearestEnemy::evaluateStateMoveToNode -- _gotoHNode component returned: " + _gotoHNode.GetFailureReason();
		failureStateMoveToNode( failureReason );
		}	

	return result;		
	
}


//--------------------------------------------------------------
// Name:		failureStateMoveToNode
// Class:		GotoHelperNodeNearestEnemy
//
// Description:	Failure Handler for State Move To Cover
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeNearestEnemy::failureStateMoveToNode( const str &failureReason )
{
	SetFailureReason( failureReason );
}


























