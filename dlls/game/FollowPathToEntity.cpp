//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/FollowPathToEntity.cpp                    $
// $Revision:: 12                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// 
//
// DESCRIPTION:
// Specialization of FollowPath used to follow moving entities
//

#include "_pch_cpp.h"
#include "actor.h"
#include "FollowPathToEntity.h"

//------------------------- CLASS ------------------------------
//
// Name:		FollowPathToEntity
// Base Class:	FollowPath
//
// Description:	FollowPathToEntity is a specialization of
// FollowPath used when the Actor must move to a moving Entity
//
// Method of Use:	Behaviors Aggregate FollowPath classes to 
//					handle long term motion to a goal
//
//--------------------------------------------------------------
CLASS_DECLARATION( FollowPath, FollowPathToEntity, NULL )
{
	{ NULL, NULL }
};

//----------------------------------------------------------------
// Name:			FollowPathToEntity
// Class:			FollowPathToEntity
//
// Description:		default constructor
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
FollowPathToEntity::FollowPathToEntity():
	_targetEntity(NULL),
	_oldGoalPosition(0,0,0)
{
}

//----------------------------------------------------------------
// Name:			SetGoal
// Class:			FollowPathToEntity
//
// Description:		sets goal entity and creates a path to it
//
// Parameters:		
//					Entity entity - goal entity
//					float radius - how close actor needs to get to goal
//					Actor self - the Actor trying to get to the goal
//
// Returns:			None
//----------------------------------------------------------------
void FollowPathToEntity::SetGoal(Entity *entity, const float radius, Actor &self)
{
	SetRadius(radius);
	if (
			( _targetEntity == NULL ) ||
			( _targetEntity->entnum != entity->entnum)
		)
	{
		_targetEntity = entity;
		_oldGoalPosition = GetGoalPosition();
		GetPath().SetPath(self, self.origin, GetGoalPosition());
	}
}

//----------------------------------------------------------------
// Name:			ClearTraceToGoal
// Class:			FollowPathToEntity
//
// Description:		test to determine if Actor can move directly 
//					to the goal
//
// Parameters:		
//					Actor self - the Actor trying to get to the goal
//					trace_t trace - trace that travels from the 
//					Actor to the goal
//
// Returns:			bool that is true if the trace reaches the goal
//----------------------------------------------------------------
const bool FollowPathToEntity::ClearTraceToGoal( Actor &self, const trace_t &traceToGoal, const float radius ) const
{
	assert (_targetEntity != NULL);

	// Path is only obstructed by the goal object itself
	bool traceHitGoal = (
						(_targetEntity->entnum == traceToGoal.entityNum) && 
						(traceToGoal.entityNum != ENTITYNUM_NONE)
						);	
	

	return ( FollowPath::ClearTraceToGoal( self, traceToGoal, radius ) || (!traceToGoal.startsolid && traceHitGoal ) );
}

//----------------------------------------------------------------
// Name:			Evaluate
// Class:			FollowPathToEntity
//
// Description:		attempts to move the Actor to the goal position
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			Steering::ReturnValue returns reason for failure
//----------------------------------------------------------------
const Steering::ReturnValue FollowPathToEntity::Evaluate( Actor &self)
{
	if (Vector::Distance( _oldGoalPosition, GetGoalPosition()) > GetAvoidanceDistance() )
	{
		_oldGoalPosition = GetGoalPosition();
		GetPath().SetPath(self, self.origin, GetGoalPosition() );
	}


	return FollowPath::Evaluate( self);
}
