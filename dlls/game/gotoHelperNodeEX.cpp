//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/gotoHelperNodeEX.cpp                         $
// $Revision:: 3                                                              $
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
#include "gotoHelperNodeEX.hpp"
#include <qcommon/gameplaymanager.h>

//--------------------------------------------------------------
//
// Init Static Vars
//
//--------------------------------------------------------------
const float GotoHelperNodeEX::NODE_RADIUS = 32.0f;

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, GotoHelperNodeEX, NULL )
	{
		{ &EV_Behavior_Args,		&GotoHelperNodeEX::SetArgs		},
		{ &EV_Behavior_AnimDone,	&GotoHelperNodeEX::AnimDone		}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		GotoHelperNodeEX()
// Class:		GotoHelperNodeEX
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GotoHelperNodeEX::GotoHelperNodeEX()
{
	_movementAnim = "";
	_nodeType = "" ;
	_maxDistance = 256;
}

//--------------------------------------------------------------
// Name:		GotoHelperNodeEX()
// Class:		GotoHelperNodeEX
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GotoHelperNodeEX::~GotoHelperNodeEX()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       GotoHelperNodeEX
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNodeEX::SetArgs( Event *ev )
{
	_nodeType	  = ev->GetString( 1 );
	_maxDistance  = ev->GetFloat ( 2 );
}


//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       GotoHelperNodeEX
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNodeEX::AnimDone( Event *ev )
{
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       GotoHelperNodeEX
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNodeEX::Begin( Actor &self )
	{       
	init( self );
	transitionToState ( GOTO_HNODE_FIND_NODE );
	}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       GotoHelperNodeEX
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t GotoHelperNodeEX::Evaluate( Actor &self )
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
// Class:       GotoHelperNodeEX
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void GotoHelperNodeEX::End(Actor &self)
{   
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		GotoHelperNodeEX
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeEX::transitionToState( GotoHelperNodeStates_t state )
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
// Class:		GotoHelperNodeEX
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeEX::setInternalState( GotoHelperNodeStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		init()
// Class:		GotoHelperNodeEX
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeEX::init( Actor &self )
{
	str objname;
	str movementMode;
	str stateName;
	GameplayManager *gpm;

	//First check if we're even in the GPM
	gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(self.getArchetype()) )
		{
		_movementAnim = "run";
		return;
		}
	
	//Now we're going to check and see if we have state data
	//in the GPM for our current state that has our movement
	//mode in it.
	objname = self.getArchetype();
	stateName = self.currentState->getName();
	objname = objname + "." + stateName;

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
		else
			_movementAnim = "run";

		return;		
		}

	if ( movementMode == "WALK" )
		{
		if ( gpm->hasProperty(objname, "Walk") )
			_movementAnim = gpm->getStringValue( objname , "Walk" );
		else
			_movementAnim = "walk";

		return;				
		}
	
	_movementAnim = "run";

}

//--------------------------------------------------------------
// Name:		think()
// Class:		GotoHelperNodeEX
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeEX::think()
{
}



//--------------------------------------------------------------
// Name:		setupStateMoveToNode()
// Class:		GotoHelperNodeEX
//
// Description:	Sets up the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeEX::setupStateMoveToNode()
{
	_gotoPoint.SetAnim( _movementAnim );
	_gotoPoint.SetDistance( NODE_RADIUS );
	_gotoPoint.SetPoint( _node->origin );
	_gotoPoint.Begin( *GetSelf() );
}


//--------------------------------------------------------------
// Name:		evaluateStateMoveToNode()
// Class:		GotoHelperNodeEX
//
// Description:	Evaluates the Move To Cover State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t GotoHelperNodeEX::evaluateStateMoveToNode()
{
	BehaviorReturnCode_t result = _gotoPoint.Evaluate( *GetSelf() );
	str failureReason = "GotoHelperNode::evaluateStateMoveToNode -- _gotoPoint component returned: " + _gotoPoint.GetFailureReason();
	
	if ( result == BEHAVIOR_FAILED )
		failureStateMoveToNode( failureReason );

	return result;	
}


//--------------------------------------------------------------
// Name:		evaluateStateFindNode()
// Class:		GotoHelperNodeEX
//
// Description:	Evaluates the FindNode State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------

BehaviorReturnCode_t GotoHelperNodeEX::evaluateStateFindNode()
{
	BehaviorReturnCode_t result = BEHAVIOR_SUCCESS;
	str failureReason = "GotoHelperNode::evaluateStateFindNode -- could not find an appropriate node";

	unsigned int NodeMask = HelperNode::GetHelperNodeMask( _nodeType );

	_node = HelperNode::FindClosestHelperNode( *GetSelf() , NodeMask , _maxDistance );

	if ( !_node )
	{
		failureStateFindNode( failureReason );
		result = BEHAVIOR_FAILED;
	}


	return result;
}

//--------------------------------------------------------------
// Name:		setupStateFindNode()
// Class:		GotoHelperNodeEX
//
// Description:	Sets up the Find State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeEX::setupStateFindNode()
{
}


//--------------------------------------------------------------
// Name:		failureStateMoveToNode
// Class:		GotoHelperNodeEX
//
// Description:	Failure Handler for State Move To Cover
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeEX::failureStateMoveToNode( const str &failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		failureStateFindNode
// Class:		GotoHelperNodeEX
//
// Description:	Failure Handler for State Move To Cover
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void GotoHelperNodeEX::failureStateFindNode( const str &failureReason )
{
	SetFailureReason( failureReason );
}

