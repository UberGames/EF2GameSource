//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/gotoCurrentHelperNode.cpp                  $
// $Revision:: 5                                                              $
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
//		str _movementAnim  -- The animation to play while moving to the node
//
//--------------------------------------------------------------------------------

#include "actor.h"
#include "gotoCurrentHelperNode.hpp"
#include <qcommon/gameplaymanager.h>

//--------------------------------------------------------------
//
// Init Static Vars
//
//--------------------------------------------------------------
const float GotoCurrentHelperNode::NODE_RADIUS = 32.0f;

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, GotoCurrentHelperNode, NULL )
	{
		{ &EV_Behavior_Args,		&GotoCurrentHelperNode::SetArgs		},
		{ &EV_Behavior_AnimDone,	&GotoCurrentHelperNode::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		GotoCurrentHelperNode()
// Class:		GotoCurrentHelperNode
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GotoCurrentHelperNode::GotoCurrentHelperNode()
{
	_movementAnim = "run";
	_faceEnemy = false;
}

//--------------------------------------------------------------
// Name:		GotoCurrentHelperNode()
// Class:		GotoCurrentHelperNode
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GotoCurrentHelperNode::~GotoCurrentHelperNode()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       GotoCurrentHelperNode
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void GotoCurrentHelperNode::SetArgs( Event *ev )
{
	_movementAnim	  = ev->GetString( 1 );
	if ( ev->NumArgs() > 1 )
		_faceEnemy = ev->GetBoolean( 2 );

}


//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       GotoCurrentHelperNode
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void GotoCurrentHelperNode::AnimDone( Event *ev )
{
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       GotoCurrentHelperNode
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GotoCurrentHelperNode::Begin( Actor &self )
{       
	init( self );
	transitionToState ( GOTO_HNODE_FIND_NODE );
}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       GotoCurrentHelperNode
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t GotoCurrentHelperNode::Evaluate( Actor &self )
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
// Class:       GotoCurrentHelperNode
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GotoCurrentHelperNode::End(Actor &self)
{
	if ( _faceEnemy )
			self.movementSubsystem->setFaceEnemy( false );
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		GotoCurrentHelperNode
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void GotoCurrentHelperNode::transitionToState( GotoHelperNodeStates_t state )
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
// Class:		GotoCurrentHelperNode
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void GotoCurrentHelperNode::setInternalState( GotoHelperNodeStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		init()
// Class:		GotoCurrentHelperNode
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void GotoCurrentHelperNode::init( Actor &self )
{
	str objname;
	str movementMode;
	str stateName;
	GameplayManager *gpm;

	self.movementSubsystem->setFaceEnemy( _faceEnemy );
	//
	// Animation Selection
	// 1st We check if we have an explicit animation in the GPD for our state, if we
	// do, we're going to use that.
	//
	// Next, we'll check for StateVar, and pull the animation from that...
	// If we have nothing there, then we'll just assume the animation passed in
	// is the animation we are going to try and use

	//
	//First check if we're even in the GPM
	gpm = GameplayManager::getTheGameplayManager();
	if ( gpm->hasObject(self.getArchetype()) )
		{
			//Now we're going to check and see if we have state data
			//in the GPM for our current state that has our movement
			//mode in it.
			objname = self.getArchetype();
			stateName = self.currentState->getName();
			objname = objname + "." + stateName;
			
			//If we have a MovementMode property, use its value, else use
			//the default value of RUN
			if ( gpm->hasProperty( objname , "MovementMode" ) )
				movementMode = gpm->getStringValue( objname , "MovementMode" );
			else
				movementMode = "RUN";


			//Now, depending on what our movementMode is, we set a proper
			//movement animation
			objname = self.getArchetype();
			objname = objname + "." + self.GetAnimSet();

			if ( movementMode == "RUN" )
				{
				if ( gpm->hasProperty(objname, "Run") )
					_movementAnim = gpm->getStringValue( objname , "Run" );

				return;		
				}

			if ( movementMode == "WALK" )
				{
				if ( gpm->hasProperty(objname, "Walk") )
					_movementAnim = gpm->getStringValue( objname , "Walk" );
				return;				
				}

			}

	// We didn't find anything in the GPD, so let's check for 
	// state vars
	str movementAnim = self.GetStateVar( _movementAnim );

	if ( movementAnim.length() )
		_movementAnim = movementAnim;	

}

//--------------------------------------------------------------
// Name:		think()
// Class:		GotoCurrentHelperNode
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoCurrentHelperNode::think()
{
}



//--------------------------------------------------------------
// Name:		setupStateMoveToNode()
// Class:		GotoCurrentHelperNode
//
// Description:	Sets up the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoCurrentHelperNode::setupStateMoveToNode()
{
	_gotoPoint.SetAnim( _movementAnim );
	_gotoPoint.SetDistance( NODE_RADIUS );
	_gotoPoint.SetPoint( _node->origin );
	_gotoPoint.Begin( *GetSelf() );
}


//--------------------------------------------------------------
// Name:		evaluateStateMoveToNode()
// Class:		GotoCurrentHelperNode
//
// Description:	Evaluates the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GotoCurrentHelperNode::evaluateStateMoveToNode()
{
	BehaviorReturnCode_t result = _gotoPoint.Evaluate( *GetSelf() );
	str failureReason = "GotoCurrentHelperNode::evaluateStateMoveToNode -- _gotoPoint component returned: " + _gotoPoint.GetFailureReason();
	
	if ( result == BEHAVIOR_FAILED )
		failureStateMoveToNode( failureReason );

	return result;	
}


//--------------------------------------------------------------
// Name:		evaluateStateFindNode()
// Class:		GotoCurrentHelperNode
//
// Description:	Evaluates the FindNode State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------

BehaviorReturnCode_t GotoCurrentHelperNode::evaluateStateFindNode()
{
	BehaviorReturnCode_t result = BEHAVIOR_SUCCESS;
	str failureReason = "GotoCurrentHelperNode::evaluateStateFindNode -- could not find an appropriate node";

	if ( !_node )
	{
		failureStateFindNode( failureReason );
		result = BEHAVIOR_FAILED;
	}


	return result;
}

//--------------------------------------------------------------
// Name:		setupStateFindNode()
// Class:		GotoCurrentHelperNode
//
// Description:	Sets up the Find State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoCurrentHelperNode::setupStateFindNode()
{
	_node = GetSelf()->currentHelperNode.node;

}


//--------------------------------------------------------------
// Name:		failureStateMoveToNode
// Class:		GotoCurrentHelperNode
//
// Description:	Failure Handler for State Move To Cover
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GotoCurrentHelperNode::failureStateMoveToNode( const str &failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		failureStateFindNode
// Class:		GotoCurrentHelperNode
//
// Description:	Failure Handler for State Move To Cover
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GotoCurrentHelperNode::failureStateFindNode( const str &failureReason )
{
	SetFailureReason( failureReason );
}

