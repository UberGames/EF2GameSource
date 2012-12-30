//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/behaviors_specific.cpp                         $
// $Revision:: 18                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// 
#include "_pch_cpp.h"
//#include "g_local.h"
#include "behavior.h"
#include "actor.h"
#include "doors.h"
#include "object.h"
#include "explosion.h"
#include "weaputils.h"
#include "player.h"
#include "behaviors_specific.h"

extern Container<HelperNode*> HelperNodes;


//==============================================================================
//                               PatrolWorkHibernate
//==============================================================================

// Init Static Vars
const float PatrolWorkHibernate::MIN_DISTANCE_TO_PATROL_NODE    = 512.0f;
const float PatrolWorkHibernate::MIN_DISTANCE_TO_WORK_NODE      = 512.0f;
const float PatrolWorkHibernate::MIN_DISTANCE_TO_HIBERNATE_NODE = 96.0f;

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, PatrolWorkHibernate, NULL )
	{
		{ &EV_Behavior_Args,			&PatrolWorkHibernate::SetArgs	},
		{ &EV_Behavior_AnimDone,		&PatrolWorkHibernate::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:         SetArgs()
// Class:        PatrolWorkHibernate
//
// Description:  Sets the arguments of the behavior 
//
// Parameters:   Event *ev
//
// Returns:      None
//--------------------------------------------------------------
void PatrolWorkHibernate::SetArgs ( Event *ev)
{

}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       PatrolWorkHibernate
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkHibernate::AnimDone( Event *ev )
{
	switch ( _state )
	{
	case PATROLWORKHIBERNATE_HIBERNATE:
		_hibernateComponent.AnimDone( ev );
	break;

	case PATROLWORKHIBERNATE_HACK:	   
		_state = PATROLWORKHIBERNATE_SELECT_STATE;
	break;

	case PATROLWORKHIBERNATE_HIBERNATE_HOLD:
		_state = PATROLWORKHIBERNATE_SELECT_STATE;
	break;

	case PATROLWORKHIBERNATE_PATROL:
		_patrolComponent.AnimDone( ev );
	break;

	case PATROLWORKHIBERNATE_WORK:
		_workComponent.AnimDone( ev );
	break;
	}
   
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       PatrolWorkHibernate
//
// Description: Begins the Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkHibernate::Begin( Actor &self )
{
	_init( self );
	if ( self.GetActorFlag ( ACTOR_FLAG_IN_ALCOVE ) )
		_state = PATROLWORKHIBERNATE_HIBERNATE_HOLD;
}



//--------------------------------------------------------------
// Name:        _init()
// Class:       PatrolWorkHibernate
//
// Description: Initializes memeber variables
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkHibernate::_init( Actor &self )
{
	_nextStateCheck			= 0.0f;
	_nextWorkCheck			= 0.0f;
	_nextHibernateCheck		= 0.0f;

	_state = PATROLWORKHIBERNATE_SELECT_STATE;
}



//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       PatrolWorkHibernate
//
// Description: Returns True Or False, and is run every frame
//              that the behavior
//
// Parameters:  Actor &self
//
// Returns:     True or False
//--------------------------------------------------------------
BehaviorReturnCode_t	PatrolWorkHibernate::Evaluate ( Actor &self )
{      
	float tendencyToHibernate;

	switch ( _state )
	{
		case PATROLWORKHIBERNATE_SELECT_STATE:
			_selectState( self );
		break;
   
		case PATROLWORKHIBERNATE_PATROL:
			_patrol( self );
		break;

		case PATROLWORKHIBERNATE_WORK:
			_work( self );
		break;

		case PATROLWORKHIBERNATE_HIBERNATE:
			tendencyToHibernate = self.personality->GetTendency( "hibernate" );

			if ( G_Random() > tendencyToHibernate )
				{
				self.SetAnim( "alcove_activate" , EV_Actor_NotifyBehavior );	
				_state = PATROLWORKHIBERNATE_HACK;
				return BEHAVIOR_EVALUATING;
				}
		
			_hibernate( self );
		break;

		case PATROLWORKHIBERNATE_HOLD:
			_hold( self );
		break;

		case PATROLWORKHIBERNATE_HIBERNATE_HOLD:
			self.SetAnim( "alcove_idle" );	
			tendencyToHibernate = self.personality->GetTendency( "hibernate" );

			if ( G_Random() > tendencyToHibernate )
				{
				self.SetAnim( "alcove_activate" , EV_Actor_NotifyBehavior );	
				}

		break;

		case PATROLWORKHIBERNATE_FAILED:
			return BEHAVIOR_FAILED;
		break;      
	}

	return BEHAVIOR_EVALUATING; 
}



//--------------------------------------------------------------
// Name:         End()
// Class:        PatrolWorkHibernate
//
// Description:  Ends the Behavior
//
// Parameters:   Actor &self
//
// Returns:      None
//--------------------------------------------------------------
void PatrolWorkHibernate::End ( Actor &self	)
{
}


//--------------------------------------------------------------
// Name:        _setupPatrol()
// Class:       PatrolWorkHibernate
//
// Description: Sets up our Patrol Component Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkHibernate::_setupPatrol( Actor &self )
{
	_patrolComponent.SetInitialNode( self.currentHelperNode.node );
	_patrolComponent.Begin( self );
}


//--------------------------------------------------------------
// Name:        _patrol()
// Class:       PatrolWorkHibernate
//
// Description: Evaluates our Patrol Component
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkHibernate::_patrol( Actor &self )
{
	// See if we want to hibernate
	if ( _wantsToHibernate( self ) )
		{
		_state = PATROLWORKHIBERNATE_HIBERNATE;
		_setupHibernate( self );
		_patrolComponent.End( self);
		return;
		}

	// See if we want to work
	if ( _wantsToWork( self ) )
		{
		_state = PATROLWORKHIBERNATE_WORK;
		_setupWork( self );
		_patrolComponent.End( self);
		return;
		}


	BehaviorReturnCode_t patrolResult;
	patrolResult = _patrolComponent.Evaluate( self );

	if ( patrolResult == BEHAVIOR_SUCCESS )
		_state = PATROLWORKHIBERNATE_SELECT_STATE;

	if ( patrolResult == BEHAVIOR_FAILED )
		_state = PATROLWORKHIBERNATE_FAILED;
}



//--------------------------------------------------------------
// Name:        _setupHibernate()
// Class:       PatrolWorkHibernate
//
// Description: Sets up our hibernate component
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------      
void PatrolWorkHibernate::_setupHibernate( Actor &self )
{
	_hibernateComponent.Begin( self );
}



//--------------------------------------------------------------
// Name:        _hibernate()
// Class:       PatrolWorkHibernate
//
// Description: Evaluates our Hibernate Component
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkHibernate::_hibernate( Actor &self )
{
	BehaviorReturnCode_t hibernateResult;
	hibernateResult = _hibernateComponent.Evaluate( self );

	if ( hibernateResult == BEHAVIOR_SUCCESS )
		_state = PATROLWORKHIBERNATE_SELECT_STATE;

	if ( hibernateResult == BEHAVIOR_FAILED )
		_state = PATROLWORKHIBERNATE_FAILED;
}


//--------------------------------------------------------------
// Name:        _setupWork()
// Class:       PatrolWorkHibernate
//
// Description: Sets up our Work Component
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------      
void PatrolWorkHibernate::_setupWork( Actor &self )
{	
	_workComponent.SetNode( self.currentHelperNode.node );
	_workComponent.Begin( self );
}


//--------------------------------------------------------------
// Name:        _work()
// Class:       PatrolWorkHibernate
//
// Description: Evaluates our Work Component
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkHibernate::_work( Actor &self )
{
	BehaviorReturnCode_t workResult;
	workResult = _workComponent.Evaluate( self );

	if ( workResult == BEHAVIOR_SUCCESS )
		_state = PATROLWORKHIBERNATE_SELECT_STATE;

	if ( workResult == BEHAVIOR_FAILED )
		_state = PATROLWORKHIBERNATE_FAILED;
}


void PatrolWorkHibernate::_setupHold( Actor &self )
{
	self.SetAnim( "idle" );
	_nextStateCheck = level.time + G_Random() + 1.0f;   
}

void PatrolWorkHibernate::_hold( Actor &self )
{
	if ( level.time >= _nextStateCheck )
		_state = PATROLWORKHIBERNATE_SELECT_STATE;
}

bool PatrolWorkHibernate::_wantsToHibernate( Actor &self )
{
	float tendencyToHibernate;

	if ( level.time > _nextHibernateCheck )
		{
		_nextHibernateCheck = level.time + G_Random() + 0.25f;

		HelperNode* hibNode;
		hibNode = HelperNode::FindClosestHelperNode( self , "hibernate" , MIN_DISTANCE_TO_HIBERNATE_NODE );

		if ( !hibNode )
			return false;

		if ( !self.WithinDistance( hibNode->origin , MIN_DISTANCE_TO_HIBERNATE_NODE ) )
			return false;

		if ( hibNode == _lastHibernateNodeChecked )
			return false;

		_lastHibernateNodeChecked = hibNode;

		tendencyToHibernate = self.personality->GetTendency( "hibernate" );

		if ( G_Random() <= tendencyToHibernate )
			return true;         

		}

	return false;
}


bool PatrolWorkHibernate::_wantsToWork( Actor &self )
{
	float tendencyToWork;

	if ( level.time > _nextWorkCheck )
		{
		_nextWorkCheck = level.time + G_Random() + 0.25f;

		tendencyToWork = self.personality->GetTendency( "work" );
		if ( G_Random() > tendencyToWork )
			return false;

		HelperNode* workNode = HelperNode::FindClosestHelperNode( self , NODETYPE_WORK , MIN_DISTANCE_TO_WORK_NODE  );
	
		if ( !workNode )
			return false;

		if ( !self.WithinDistance( workNode->origin , MIN_DISTANCE_TO_WORK_NODE ) )
			return false;

		if ( workNode == _lastWorkNodeChecked )
			{
			self.currentHelperNode.node->UnreserveNode();
			return false;
			}

		_lastWorkNodeChecked = workNode;
	
		self.currentHelperNode.node->UnreserveNode();
		return true;         
		}

	return false;
}

//--------------------------------------------------------------
// Name:        _selectState()
// Class:       PatrolWorkHibernate
//
// Description: Selects the state of our behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkHibernate::_selectState( Actor &self )
{
	// By default, we prefer to patrol.  
	// As we patrol, we'll keep looking for hibernate and work
	// nodes, and change our state appropriately
	HelperNode* patrolNode;
	patrolNode = HelperNode::FindClosestHelperNode( self , NODETYPE_PATROL , MIN_DISTANCE_TO_PATROL_NODE );

	if ( patrolNode && self.WithinDistance( patrolNode->origin , MIN_DISTANCE_TO_PATROL_NODE ) )
		{
		_state = PATROLWORKHIBERNATE_PATROL;
		_setupPatrol( self );
		return;
		}

	// See if we want to hibernate
	if ( _wantsToHibernate( self ) )
		{
		_state = PATROLWORKHIBERNATE_HIBERNATE;
		_setupHibernate( self );   
		return;
		}

	// See if we want to work
	if ( _wantsToWork( self ) )
		{
		_state = PATROLWORKHIBERNATE_WORK;
		_setupWork( self );      
		return;
		}

	_state = PATROLWORKHIBERNATE_HOLD;
	_setupHold( self );

}





//==============================================================================
//                               PatrolWorkWander
//==============================================================================

// Init Static Vars
const float PatrolWorkWander::MIN_DISTANCE_TO_PATROL_NODE    = 512.0f;
const float PatrolWorkWander::MIN_DISTANCE_TO_WORK_NODE      = 512.0f;


//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, PatrolWorkWander, NULL )
	{
		{ &EV_Behavior_Args,		&PatrolWorkWander::SetArgs	},
		{ &EV_Behavior_AnimDone,	&PatrolWorkWander::AnimDone	}, 
		{ NULL, NULL							}
	};


//--------------------------------------------------------------
// Name:         SetArgs()
// Class:        PatrolWorkWander
//
// Description:  Sets the arguments of the behavior 
//
// Parameters:   Event *ev
//
// Returns:      None
//--------------------------------------------------------------
void PatrolWorkWander::SetArgs ( Event *ev)
{
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       PatrolWorkWander
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkWander::AnimDone( Event *ev )
{
	switch ( _state )
	{
	   
	case PATROLWORKWANDER_PATROL:
		_patrolComponent.AnimDone( ev );
	break;

    case PATROLWORKWANDER_WORK:
		_workComponent.AnimDone( ev );
	break;
  
	}
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       PatrolWorkWander
//
// Description: Begins the Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkWander::Begin( Actor &self )
{
   _init( self );
}



//--------------------------------------------------------------
// Name:        _init()
// Class:       PatrolWorkHibernate
//
// Description: Initializes memeber variables
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkWander::_init( Actor &self )
{
   _nextStateCheck     = 0.0f;
   _nextWorkCheck      = 0.0f; 
   _nextPatrolCheck    = 0.0f;
   
   _wanderFailures = 0;
   _patrolFailures = 0;
   _workFailures   = 0;
   

   _state = PATROLWORKWANDER_SELECT_STATE;
}



//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       PatrolWorkWander
//
// Description: Returns True Or False, and is run every frame
//              that the behavior
//
// Parameters:  Actor &self
//
// Returns:     True or False
//--------------------------------------------------------------
BehaviorReturnCode_t PatrolWorkWander::Evaluate ( Actor &self )
{      
	switch ( _state )
		{
		case PATROLWORKWANDER_SELECT_STATE:
			_selectState( self );
		break;

		case PATROLWORKWANDER_PATROL:
			_patrol ( self );
		break;

		case PATROLWORKWANDER_WORK:
			_work( self );
		break;

		case PATROLWORKWANDER_WANDER:
			_wander( self );
		break;

		case PATROLWORKWANDER_HOLD:
			_hold( self );
		break;

		case PATROLWORKWANDER_FAILED:
			return BEHAVIOR_FAILED;
		break;
		}

	return BEHAVIOR_EVALUATING; 
}



//--------------------------------------------------------------
// Name:         End()
// Class:        PatrolWorkWander
//
// Description:  Ends the Behavior
//
// Parameters:   Actor &self
//
// Returns:      None
//--------------------------------------------------------------
void PatrolWorkWander::End ( Actor &self	)
{
   
}


//--------------------------------------------------------------
// Name:		_setupWander()
// Class:		PatrolWorkWander
//
// Description: Sets up our MoveRandomDirection Component Behavior
//
// Parameters:	Actor  &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkWander::_setupWander( Actor &self )
{
	_wanderComponent.SetAnim( "walk" );
	_wanderComponent.SetDistance( 256.0f );
	_wanderComponent.SetMinDistance( 96.0f );
	_wanderComponent.SetMode( MoveRandomDirection::RANDOM_MOVE_ANYWHERE );
	_wanderComponent.Begin( self );
}


//--------------------------------------------------------------
// Name:		_wander()
// Class:		PatrolWorkWander
//
// Description:	Evaluates our MoveRandomDirection Component
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void PatrolWorkWander::_wander( Actor &self )
{
   // See if we want to work
   if ( _wantsToWork( self ) )
      {
      _state = PATROLWORKWANDER_WORK;
      _setupWork( self );
      _wanderComponent.End( self);
      return;
      }

   BehaviorReturnCode_t wanderResult;
   wanderResult = _wanderComponent.Evaluate( self );

   if ( wanderResult == BEHAVIOR_SUCCESS )
		{
		_wanderFailures = 0;
		_setupHold( self );
		_state = PATROLWORKWANDER_HOLD;
		return;
		}

   if ( wanderResult == BEHAVIOR_FAILED ) 
		{
		_wanderFailed( self );
		return;
		}

}


//--------------------------------------------------------------
// Name:		_wanderFailed()
// Class:		PatrolWorkWander
//
// Description:	Failure Handler for the MoveRandomDirection Component
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void PatrolWorkWander::_wanderFailed( Actor &self )
{
	_wanderFailures++;
	
	_setupHold( self );
	_state = PATROLWORKWANDER_HOLD;
}

//--------------------------------------------------------------
// Name:        _setupPatrol()
// Class:       PatrolWorkWander
//
// Description: Sets up our Patrol Component Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkWander::_setupPatrol( Actor &self )
{
	_patrolComponent.Begin( self );
}


//--------------------------------------------------------------
// Name:        _patrol()
// Class:       PatrolWorkWander
//
// Description: Evaluates our Patrol Component
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkWander::_patrol( Actor &self )
{
   // See if we want to work
   if ( _wantsToWork( self ) )
      {
      _state = PATROLWORKWANDER_WORK;
      _setupWork( self );
      _patrolComponent.End( self);
      return;
      }


   BehaviorReturnCode_t patrolResult;
   patrolResult = _patrolComponent.Evaluate( self );

   if ( patrolResult == BEHAVIOR_SUCCESS )
		{
		_patrolFailures = 0;
		_setupHold( self );
		_state = PATROLWORKWANDER_HOLD;
		return;
		}
      

   if ( patrolResult == BEHAVIOR_FAILED )
		{
		_patrolFailed( self );
		return;
		}
	
}

//--------------------------------------------------------------
// Name:		_patrolFailed()
// Class:		PatrolWorkWander
//
// Description:	Failure Handler for
//
// Parameters:
//
// Returns:
//--------------------------------------------------------------
void PatrolWorkWander::_patrolFailed( Actor &self )
{
	_patrolFailures++;

	_setupHold( self );
	_state = PATROLWORKWANDER_HOLD;
}

//--------------------------------------------------------------
// Name:        _setupWork()
// Class:       PatrolWorkWander
//
// Description: Sets up our Work Component
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------      
void PatrolWorkWander::_setupWork( Actor &self )
{
   _workComponent.Begin( self );
}


//--------------------------------------------------------------
// Name:        _work()
// Class:       PatrolWorkWander
//
// Description: Evaluates our Work Component
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkWander::_work( Actor &self )
{
   BehaviorReturnCode_t workResult;
   workResult = _workComponent.Evaluate( self );

   if ( workResult == BEHAVIOR_SUCCESS )
		{
		_workFailures = 0;
		_setupHold( self );
		_state = PATROLWORKWANDER_HOLD;
		return;
		}
      

   if ( workResult == BEHAVIOR_FAILED )
		{
		_workFailed( self );
		return;
		}
   
}

//--------------------------------------------------------------
// Name:		_workFailed()
// Class:		PatrolWorkWander
//
// Description:	Failure Handler for the Work Component
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void PatrolWorkWander::_workFailed( Actor &self )
{
	_workFailures++;

	_setupHold( self );
	_state = PATROLWORKWANDER_HOLD;
}

//--------------------------------------------------------------
// Name:		_setupHold()
// Class:		PatrolWorkWander
//
// Description:	Sets up Hold State
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void PatrolWorkWander::_setupHold( Actor &self )
{
   self.SetAnim( "idle" );
   _nextStateCheck = level.time + G_Random( 5.0f ) + 1.0f;   
}

//--------------------------------------------------------------
// Name:		_hold()
// Class:		PatrolWorkWander
//
// Description:	Holds us in place until we need to change
//              states
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void PatrolWorkWander::_hold( Actor &self )
{
   if ( level.time >= _nextStateCheck )
      _state = PATROLWORKWANDER_SELECT_STATE;
}

//--------------------------------------------------------------
// Name:		_wantsToWork()
// Class:		PatrolWorkWander
//
// Description:	Checks if the actor "wants" to work
//
// Parameters:	Actor &self
//
// Returns:		True or False
//--------------------------------------------------------------
bool PatrolWorkWander::_wantsToWork( Actor &self )
{
   float tendencyToWork;
   
	if ( _workFailures > 5 )
		return false;

   if ( level.time > _nextWorkCheck )
      {
      _nextWorkCheck = level.time + G_Random() + 0.25f;

      HelperNode* workNode;
      workNode = HelperNode::FindClosestHelperNode( self , NODETYPE_WORK , MIN_DISTANCE_TO_WORK_NODE  );
      
      if ( !workNode )
         return false;

      if ( !self.WithinDistance( workNode->origin , MIN_DISTANCE_TO_WORK_NODE ) )
         return false;

      if ( workNode == _lastWorkNodeChecked )
         return false;

      _lastWorkNodeChecked = workNode;

      tendencyToWork = self.personality->GetTendency( "work" );
      
      if ( G_Random() <= tendencyToWork )
		{
       	   self.currentHelperNode.node->UnreserveNode();
		   return true;         
		}

	 

      }

   return false;
}

//--------------------------------------------------------------
// Name:		_wantsToPatrol()
// Class:		PatrolWorkWander
//
// Description:	Checks if the Actor "wants" to patrol
//
// Parameters:	Actor &self
//
// Returns:		True or False
//--------------------------------------------------------------
bool PatrolWorkWander::_wantsToPatrol( Actor &self )
{
   float tendencyToPatrol;
   
	if ( _patrolFailures > 5 )
		return false;

   if ( level.time >= _nextPatrolCheck )
      {
      _nextPatrolCheck = level.time + G_Random() + 0.25f;

      HelperNode* patrolNode;
      patrolNode = HelperNode::FindClosestHelperNode( self , NODETYPE_PATROL , MIN_DISTANCE_TO_PATROL_NODE  );
      
      if ( !patrolNode )
         return false;

      if ( !self.WithinDistance( patrolNode->origin , MIN_DISTANCE_TO_PATROL_NODE ) )
         return false;

      if ( patrolNode == _lastPatrolNodeChecked )
         return false;

      _lastPatrolNodeChecked = patrolNode;

      tendencyToPatrol = self.personality->GetTendency( "patrol" );
      
      if ( G_Random() <= tendencyToPatrol )
         return true;         

      }

   return false;
}

//--------------------------------------------------------------
// Name:		_wantsToWander()
// Class:		PatrolWorkWander
//
// Description:	Checks if the Actor "wants" to Wander
//
// Parameters:	Actor &self
//
// Returns:		True or False
//--------------------------------------------------------------
bool PatrolWorkWander::_wantsToWander( Actor &self )
{
	if ( _wanderFailures > 5 )
		return false;

	return true;
}
//--------------------------------------------------------------
// Name:        _selectState()
// Class:       PatrolWorkWander
//
// Description: Selects the state of our behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void PatrolWorkWander::_selectState( Actor &self )
{

	if ( _wantsToPatrol( self ) )
		{
		_setupPatrol( self );
		_state = PATROLWORKWANDER_PATROL;
		return;
		}

	if ( _wantsToWork( self ) )
		{
		_setupWork( self );
		_state = PATROLWORKWANDER_WORK;
		return;
		}

	if ( _wantsToWander( self ) )
		{
		_setupWander( self );
		_state = PATROLWORKWANDER_WANDER;
		return;
		}

	// Nothing else is working, so we'll just stand and play our
	// idle animation
   _state = PATROLWORKWANDER_HOLD;
   _setupHold( self );
}
