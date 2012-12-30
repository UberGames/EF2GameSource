//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/FollowPathToPoint.cpp                    $
// $Revision:: 9                                                              $
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
#include "FollowPathToPoint.h"
#include "actor.h"

//------------------------- CLASS ------------------------------
//
// Name:		FollowPathToPoint
// Base Class:	FollowPath
//
// Description:	FollowPathToEntity is a specialization of
// FollowPath used when the Actor must move to a stationary point
//
// Method of Use:	Behaviors Aggregate FollowPath classes to 
//					handle long term motion to a goal
//
//--------------------------------------------------------------
CLASS_DECLARATION( FollowPath, FollowPathToPoint, NULL )
{
	{ NULL, NULL }
};

//----------------------------------------------------------------
// Name:			FollowPathToPoint
// Class:			FollowPathToPoint
//
// Description:		default constructor
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
FollowPathToPoint::FollowPathToPoint():_targetPoint(0,0,0)
{
}

//----------------------------------------------------------------
// Name:			SetGoal
// Class:			FollowPathToPoint
//
// Description:		sets goal point and creates a path to it
//
// Parameters:		
//					Vector targetPoint - goal position
//					float radius - how close actor needs to get to goal
//					Actor self - the Actor trying to get to the goal
//
// Returns:			None
//----------------------------------------------------------------
void FollowPathToPoint::SetGoal(const Vector &targetPoint, const float radius, Actor &self)
{
	SetRadius(radius);
	if (!Vector::CloseEnough(_targetPoint, targetPoint, 1.0f))
	{
		_targetPoint = targetPoint;
		GetPath().SetPath(self, self.origin, GetGoalPosition());
	}
}
