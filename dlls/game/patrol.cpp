//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/patrol.cpp                      $
// $Revision:: 13                                                             $
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
//	CloseInOnEnemy Implementation
// 
// PARAMETERS:
//		
//
// ANIMATIONS:
//		
//--------------------------------------------------------------------------------

#include "actor.h"
#include "patrol.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, Patrol, NULL )
	{
		{ &EV_Behavior_Args,			&Patrol::SetArgs	},
		{ &EV_Behavior_AnimDone,		&Patrol::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		Patrol()
// Class:		Patrol
//	
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
Patrol::Patrol()
{
	_anim = "walk";
}

//--------------------------------------------------------------
// Name:		~Patrol()
// Class:		Patrol
//
// Description: Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
Patrol::~Patrol()
{
}

//--------------------------------------------------------------
// Name:         SetArgs()
// Class:        Patrol
//
// Description:  Sets the arguments of the behavior 
//
// Parameters:   Event *ev
//
// Returns:      None
//--------------------------------------------------------------
void Patrol::SetArgs ( Event *ev)
{
	_anim = ev->GetString( 1 );
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       Patrol
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::AnimDone( Event *ev )
{
	if ( _state == PATROL_WAITING_AT_NODE_FOR_ANIM )
		{
		_state = PATROL_FIND_NEXT_PATROL_NODE;
		}
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       Patrol
//
// Description: Begins the Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::Begin( Actor &self )
{
	_init( self );
}



//--------------------------------------------------------------
// Name:        _init()
// Class:       Patrol
//
// Description: Initializes memeber variables
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_init( Actor &self )
{
	_nextMoveAttempt = 0.0f;
	_waitTime        = 0.0f;
	_moveFailures     = 0;

	//Setup Torso Anim if appropriate
	_torsoAnim = self.combatSubsystem->GetAnimForMyWeapon( "Idle" );
	if ( _torsoAnim.length() )
		{
		self.SetAnim( _torsoAnim, NULL , torso );
		}

	_state = PATROL_FIND_CLOSEST_PATROL_NODE;
	if ( !_setupFindClosestPatrolNode( self ) )
		_setupFindClosestPatrolNodeFailed( self );
}



//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       Patrol
//
// Description: Returns True Or False, and is run every frame
//              that the behavior
//
// Parameters:  Actor &self
//
// Returns:     True or False
//--------------------------------------------------------------
BehaviorReturnCode_t	Patrol::Evaluate ( Actor &self )
{      
	switch ( _state )
	{
		case PATROL_FIND_CLOSEST_PATROL_NODE:
			_findClosestPatrolNode( self );
		break;

		case PATROL_MOVING_TO_PATROL_NODE:
			_moveToPatrolNode( self );
		break;

		case PATROL_AT_PATROL_NODE:
			_atPatrolNode( self );
		break;

		case PATROL_WAITING_AT_NODE:
			_waitAtPatrolNode( self );
		break;

		case PATROL_WAITING_AT_NODE_FOR_ANIM:
			_waitAtPatrolNodeForAnim( self );
		break;

		case PATROL_FIND_NEXT_PATROL_NODE:
			_findNextPatrolNode( self );
		break;

		case PATROL_HOLD:
			_hold( self );
		break;

		case PATROL_FAILED:
			return BEHAVIOR_FAILED;
		break;

		case PATROL_SUCCESSFUL:
			return BEHAVIOR_SUCCESS;
		break;
		}

	return BEHAVIOR_EVALUATING; 
}




//--------------------------------------------------------------
// Name:         End()
// Class:        Patrol
//
// Description:  Ends the Behavior
//
// Parameters:   Actor &self
//
// Returns:      None
//--------------------------------------------------------------
void Patrol::End ( Actor &self	)
{   
}



//--------------------------------------------------------------
// Name:        _setupFindClosestPatrolNode()
// Class:       Patrol
//
// Description: Sets up the behavior for the Find Node State
//
// Parameters:  Actor &self
//
// Returns:     true or false
//--------------------------------------------------------------
bool Patrol::_setupFindClosestPatrolNode( Actor &self )
{
	_node = NULL;
	return true;
}



//--------------------------------------------------------------
// Name:        _setupFindClosestPatrolNodeFailed()
// Class:       Patrol
//
// Description: Failure Handler for _setupFindClosestPatrolNode()
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_setupFindClosestPatrolNodeFailed( Actor &self )
{
	_state = PATROL_FAILED;
}



//--------------------------------------------------------------
// Name:        _findClosestPatrolNode
// Class:       Patrol
//
// Description: Finds the closest the Patrol Node, and changes
//              the behavior's state
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_findClosestPatrolNode( Actor &self )
{
	if ( _node )
		{
		_state = PATROL_MOVING_TO_PATROL_NODE;
		if (!_setupMovingToPatrolNode( self ) )
			_setupMovingToPatrolNodeFailed( self );
		return;
		}

	_node = HelperNode::FindClosestHelperNode( self , NODETYPE_PATROL , 512.0f, 0 );   

	if ( !_node )
		{	   
		_findClosestPatrolNodeFailed(self);
		return;
		}

	//Now Let's check if we're already "on the path" to help prevent backtracking
	str nextNodeTargetName;
	nextNodeTargetName = _node->target;

	// See if we hit the end of our path
	if ( nextNodeTargetName.length() )
		{
		HelperNode* testNode;
		testNode = HelperNode::GetTargetedHelperNode( nextNodeTargetName );
		if ( testNode )
			{
			Vector selfToTestNode = testNode->origin - self.origin;
			float selfToTestNodeDist = selfToTestNode.length();

			Vector currentNodeToTestNode = testNode->origin - _node->origin;
			float currentNodeToTestNodeDist = currentNodeToTestNode.length();

			//If we're closer to the next node than the distance between the "closest" node and the next node then
			//we'll use the next node instead to prevent backtracking
			if ( currentNodeToTestNodeDist > selfToTestNodeDist )
				_node = testNode;
			}
			
		}



	_state = PATROL_MOVING_TO_PATROL_NODE;
	if (!_setupMovingToPatrolNode( self ) )
		_setupMovingToPatrolNodeFailed( self );

}


//--------------------------------------------------------------
// Name:        _findClosestPatrolNodeFailed()
// Class:       Patrol
//
// Description: Failure Handler for _findClosestPatrolNode
//
// Parameters:
//
// Returns:
//--------------------------------------------------------------
void Patrol::_findClosestPatrolNodeFailed( Actor &self )
{
	SetFailureReason( "Unable to find a patrol node within 512.0 units" );
		_state = PATROL_FAILED;
}



//--------------------------------------------------------------
// Name:        _setupMovingToPatrolNode()
// Class:       Patrol
//
// Description: Sets up behavior for the Moving To Node State
//
// Parameters:  Actor &self
//
// Returns:     true or false
//--------------------------------------------------------------
bool Patrol::_setupMovingToPatrolNode( Actor &self )
{
	//If we have a _lastNode let's run our exit thread
	if ( _lastNode )
		_lastNode->RunExitThread();
  
	//Setup Torso Anim if appropriate
	_torsoAnim = self.combatSubsystem->GetAnimForMyWeapon( "Idle" );
	if ( _torsoAnim.length() )
		{
		self.SetAnim( _torsoAnim, NULL , torso );
		}

	_gotoHelperNode.SetDistance( 34.0f );
	_gotoHelperNode.SetAnim( _anim );
	_gotoHelperNode.SetPoint( _node->origin );
	_gotoHelperNode.Begin( self );

	return true;
}



//--------------------------------------------------------------
// Name:        _setupMovingToPatrolNodeFailed()
// Class:       Patrol
//
// Description: Failure Handler for _setupMovingToPatrolNode()
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_setupMovingToPatrolNodeFailed( Actor &self )
{
	_state = PATROL_FAILED;
}



//--------------------------------------------------------------
// Name:        _moveToPatrolNode()
// Class:       Patrol
//
// Description: Evaluates the GotoEntity Component Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_moveToPatrolNode( Actor &self )
{
	BehaviorReturnCode_t moveResult;

	moveResult = _gotoHelperNode.Evaluate( self );
   
	if ( moveResult == BEHAVIOR_SUCCESS )
		{
		_nextMoveAttempt = 0.0f;
		_moveFailures    = 0;
		_state = PATROL_AT_PATROL_NODE;
		if ( !_setupAtPatrolNode( self ) )
			_setupAtPatrolNodeFailed( self );
		}  
   else if ( moveResult == BEHAVIOR_FAILED )
	{
	_moveToPatrolNodeFailed( self );
	}
   /*
	else if ( moveResult == BEHAVIOR_EVALUATING )
	{
	//BULLET PROOFING--------------------------------------
	//Stupid Check to see if anim got stomped somewhere
	//BULLET PROOFING--------------------------------------
	str currentAnim = self.animate->GetName();
	if ( currentAnim != _anim )
		self.SetAnim( _anim, NULL , legs );
	}*/

}



//--------------------------------------------------------------
// Name:        _moveToPatrolNodeFailed()
// Class:       Patrol
//
// Description: Failure Handler for _moveToPatrolNode
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_moveToPatrolNodeFailed( Actor &self )
{
	_moveFailures++;

	//if ( _moveFailures > 10 )
	//  {
		_state = PATROL_FAILED;
	//   }
	//else
	//   {
	//   _state = PATROL_HOLD;
	//   if ( !_setupHold( self ) )
	//      _setupHoldFailed( self );
	//   }

}



//--------------------------------------------------------------
// Name:        _setupAtPatrolNode()
// Class:       Patrol
//
// Description: Sets behavior up to enter the At Node state
//
// Parameters:  Actor &self
//
// Returns:     true or false
//--------------------------------------------------------------
bool Patrol::_setupAtPatrolNode( Actor &self )
{
	//self.SetAnim( "walk" );
	return true;
}


//--------------------------------------------------------------
// Name:        _setupAtPatrolNodeFailed()
// Class:       Patrol
//
// Description: Failure Handler for _setupAtPatrolNodeFailed
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_setupAtPatrolNodeFailed( Actor &self )
{
	_state = PATROL_FAILED;
}



//--------------------------------------------------------------
// Name:        _AtPatrolNode()
// Class:       Patrol
//
// Description: Main Function for the At Node State
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_atPatrolNode( Actor &self )
{
	str nodeAnim;
	nodeAnim = _node->GetAnim();


	// Run this thread, if we have one
	_node->RunEntryThread();

	// First, See if we have a wait time or wait random time
	_waitTime = _node->GetWaitTime() + level.time;

	if ( _node->isWaitRandom() )
		_waitTime = level.time + G_Random( _waitTime ) + 1;

	if ( _waitTime > level.time )
		{
		if ( nodeAnim.length() && self.animate->HasAnim( nodeAnim.c_str() ) )
			{
			self.ClearLegAnim();
			self.ClearTorsoAnim();
			self.SetAnim( nodeAnim );
			}
		else 
			self.SetAnim( "idle" );

		_state = PATROL_WAITING_AT_NODE;
		if ( !_setupWaitingAtPatrolNode( self ) )
			_setupWaitingAtPatrolNodeFailed( self );

		return;
		}

	// We didn't have a wait time, let's see if we're supposed to wait for an animation
	if ( _node->isWaitForAnim() && nodeAnim.length() )
		{
		self.SetAnim( nodeAnim , EV_Actor_NotifyBehavior);
		_state = PATROL_WAITING_AT_NODE_FOR_ANIM;
		if ( !_setupWaitingAtPatrolNodeForAnim( self ) )
			_setupWaitingAtPatrolNodeForAnimFailed( self );

		return;
		}

	// We're not waiting here, so let's just get the next node
	_state = PATROL_FIND_NEXT_PATROL_NODE;
	if ( !_setupFindNextPatrolNode( self ) )
		_setupFindNextPatrolNodeFailed( self );
}




//--------------------------------------------------------------
// Name:        _setupWaitingAtPatrolNode()
// Class:       Patrol
//
// Description: Sets Behavior up for Wait State
//
// Parameters:  Actor &self
//
// Returns:     true or false
//--------------------------------------------------------------
bool Patrol::_setupWaitingAtPatrolNode( Actor &self )
{
	return true;
}



//--------------------------------------------------------------
// Name:        _setupWaitingAtPatrolNodeFailed()
// Class:       Patrol
//
// Description: Failure Handler for _setupWaitingAtPatrol
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_setupWaitingAtPatrolNodeFailed(Actor &self )
{
	_state = PATROL_FAILED;
}



//--------------------------------------------------------------
// Name:        _waitAtPatrolNode
// Class:       Patrol
//
// Description: Waits at the Patrol Node
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_waitAtPatrolNode(Actor &self )
{
	if ( level.time > _waitTime )
		{
		_state = PATROL_FIND_NEXT_PATROL_NODE;
		if ( !_setupFindNextPatrolNode( self ) )
			_setupFindNextPatrolNodeFailed( self );
		}
}



//--------------------------------------------------------------
// Name:        _setupWaitingAtPatrolNodeForAnim()
// Class:       Patrol
//
// Description: Sets up behavior to wait for anim
//
// Parameters:  Actor &self
//
// Returns:     true or false
//--------------------------------------------------------------
bool Patrol::_setupWaitingAtPatrolNodeForAnim( Actor &self )
{
	self.setAngles(_node->angles);
	return true;
}



//--------------------------------------------------------------
// Name:        _setupWaitingAtPatrolNodeForAnimFailed()
// Class:       Patrol
//
// Description: Failure Handler for _setupWaitingAtPatrolNode()
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_setupWaitingAtPatrolNodeForAnimFailed( Actor &self )
{
	_state = PATROL_FAILED;
}



//--------------------------------------------------------------
// Name:        _waitAtPatrolNodeForAnim()
// Class:       Patrol
//
// Description: Nothing now, here in case we need it
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_waitAtPatrolNodeForAnim( Actor &self )
{
	// Transition is handled by the AnimDone Event
}



//--------------------------------------------------------------
// Name:        _setupFindNextPatrolNode()
// Class:       Patrol
//
// Description: Sets behavior up to find the next patrol node
//
// Parameters:  Actor &self
//
// Returns:     true or false
//--------------------------------------------------------------
bool Patrol::_setupFindNextPatrolNode( Actor &self )
{
	return true;
}



//--------------------------------------------------------------
// Name:        _setupFindNextPatrolNodeFailed()
// Class:       Patrol
//
// Description: Failure Handler for _setupFindNextPatrolNode()
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_setupFindNextPatrolNodeFailed( Actor &self )
{
	SetFailureReason( "Unable to find the next Node in the patrol path" );
	_state = PATROL_FAILED;
}



//--------------------------------------------------------------
// Name:        _findNextPatrolNode()
// Class:       Patrol
//
// Description: Finds next patrol node
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_findNextPatrolNode( Actor &self )
{
	str nextNodeTargetName;
	nextNodeTargetName = _node->target;

	// See if we hit the end of our path
	if ( !nextNodeTargetName.length() )
		{
		_state = PATROL_SUCCESSFUL;
		return;
		}

	_lastNode = _node;
	_node = HelperNode::GetTargetedHelperNode( nextNodeTargetName );

	if ( !_node )
		{
		_state = PATROL_FAILED;
		return;
		}

	_state = PATROL_MOVING_TO_PATROL_NODE;
	if ( !_setupMovingToPatrolNode( self ) )
		_setupMovingToPatrolNodeFailed( self );
}  



//--------------------------------------------------------------
// Name:        _setupHold()
// Class:       Patrol
//
// Description: Sets Behavior up to hold
//
// Parameters:  Actor &self
//
// Returns:     true or false
//--------------------------------------------------------------
bool Patrol::_setupHold( Actor &self )
{
	_nextMoveAttempt = level.time + 0.5f + G_Random();
	return true;
}



//--------------------------------------------------------------
// Name:        _setupHoldFailed()
// Class:       Patrol
//
// Description: Failure Handler for _setupHold()
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_setupHoldFailed( Actor &self )
{
	_state = PATROL_FAILED;
}



//--------------------------------------------------------------
// Name:        _hold()
// Class:       Patrol
//
// Description: Holds the Actor in place for a brief time
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void Patrol::_hold( Actor &self )
{
	if ( level.time > _nextMoveAttempt )
		{
		_state = PATROL_MOVING_TO_PATROL_NODE;
		if ( !_setupMovingToPatrolNode( self ) )
			_setupMovingToPatrolNodeFailed( self );
		}
}
