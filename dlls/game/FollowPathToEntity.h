//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/FollowPathToEntity.h                          $
// $Revision:: 7                                                              $
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
//
// DESCRIPTION:
// Specialization of FollowPath used to follow moving entities
//

#ifndef __FOLLOW_PATH_TO_ENTITY_H__
#define __FOLLOW_PATH_TO_ENTITY_H__

#include "FollowPath.h"

class Actor;

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
class FollowPathToEntity : public FollowPath
{
public:
								CLASS_PROTOTYPE( FollowPathToEntity );
	
								FollowPathToEntity();
	virtual void				SetGoal( Entity *ent, const float radius, Actor &self );
	virtual const Vector &		GetGoalPosition(void) const { assert (_targetEntity != NULL); return _targetEntity->origin; }
	virtual const float 		GetGoalRadius(void) const { return Vector(_targetEntity->mins).lengthXY(); }
	virtual const ReturnValue	Evaluate( Actor &self );
	virtual void				Archive( Archiver &arc );
	
protected:
	virtual const bool			ClearTraceToGoal( Actor &self, const trace_t &traceToGoal, const float radius ) const;
	
private:
	EntityPtr					_targetEntity;
	Vector						_oldGoalPosition;
};

inline void FollowPathToEntity::Archive( Archiver &arc )
{
	FollowPath::Archive( arc );
	
	arc.ArchiveSafePointer( &_targetEntity );
	arc.ArchiveVector(		&_oldGoalPosition );
}

#endif // FollowPathToEntity.h
