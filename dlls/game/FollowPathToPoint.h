//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/FollowPathToPoint.h                          $
// $Revision:: 6                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Specialization of FollowPath used to follow a path to a static point
//

#ifndef __FOLLOW_PATH_TO_POINT_H__
#define __FOLLOW_PATH_TO_POINT_H__

#include "FollowPath.h"

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
class FollowPathToPoint : public FollowPath
{
public:
							CLASS_PROTOTYPE( FollowPathToPoint );
	
							FollowPathToPoint();
	virtual void			SetGoal( const Vector &targetPoint, const float radius, Actor &self );
	virtual const Vector &	GetGoalPosition( void ) const { return _targetPoint; }
	virtual const float 	GetGoalRadius(void) const { return 16.0f; }
	virtual void			Archive( Archiver &arc );
	   
private:
	Vector					_targetPoint;
};

inline void FollowPathToPoint::Archive( Archiver &arc )
{
	FollowPath::Archive( arc );
	
	arc.ArchiveVector( &_targetPoint );
}

#endif // FollowPathToPoint.h
