//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/FollowPath.h                                  $
// $Revision:: 20                                                             $
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
// Provides base functionality for path following with avoidance
//

#ifndef __FOLLOW_PATH_H__
#define __FOLLOW_PATH_H__

#include "g_local.h"
#include "entity.h"
#include "path.h"
#include "steering.h"

class Actor;

//------------------------- CLASS ------------------------------
//
// Name:		FollowNode
// Base Class:	Class
//
// Description:	FollowNodes are a lightweight class meant to 
// store translation information for the FollowNodePath class
//
// Method of Use:	FollowNodes should only be created by 
// FollowNodePaths but other classes my need access a node's 
// translation.
//
//--------------------------------------------------------------

class FollowNode : public Class
{
public:
								CLASS_PROTOTYPE( FollowNode );

								FollowNode( void );
								FollowNode( const Vector &position, const bool isTemporary=false, const bool isJumpNode=false, const float jumpAngle=45.0f  );
	Vector const &				GetPosition( void ) const { return _position; }
	const bool					GetIsTemporary( void ) const { return _isTemporary; }
	const bool					GetIsJumpNode( void ) const { return _isJumpNode; }
	void						SetIsJumpNode( const bool isJumpNode ) { _isJumpNode = isJumpNode; }
	const float					GetJumpAngle( void ) const { return _jumpAngle; }
	void						SetJumpAngle( const float jumpAngle ) 
								{ 
									_jumpAngle = jumpAngle; 
								}

	virtual void				Archive( Archiver &arc );
	
private:
	Vector						_position;
	bool						_isTemporary;
	bool						_isJumpNode;
	float						_jumpAngle;
	
};

inline void FollowNode::Archive( Archiver &arc )
{
	Class::Archive( arc );
	
	arc.ArchiveVector(	&_position );
	arc.ArchiveBool(	&_isTemporary );
	arc.ArchiveBool(	&_isJumpNode );
	arc.ArchiveFloat(	&_jumpAngle );
}
typedef SafePtr<FollowNode> FollowNodePtr;

//------------------------- CLASS ------------------------------
//
// Name:		FollowNodePath
// Base Class:	Class
//
// Description:	FollowNodePaths are simple paths that support the
// concept of temporary path nodes (allowing temporary insertion 
// nodes into the path).
//
// Method of Use:	FollowNodePaths should primarily be used by 
// FollowPath classes, although other classes may need access to
// most of the members of the class
//
//--------------------------------------------------------------

enum PathCreationReturnCodes
{
	PATH_CREATION_SUCCESS,
	PATH_CREATION_FAILED_DEGENERATE_PATH,
	PATH_CREATION_FAILED_NODES_NOT_CONNECTED,
	PATH_CREATION_FAILED_START_NODE_NOT_FOUND,
	PATH_CREATION_FAILED_END_NODE_NOT_FOUND,
};
class FollowNodePath : public Class
{
public:
										CLASS_PROTOTYPE( FollowNodePath );

										FollowNodePath();
										~FollowNodePath() { Clear(); }
	void								AddNode(FollowNodePtr node) { _nodes.AddObject(node); }
	void								RemoveNode(FollowNodePtr node);
	void								InsertNode(FollowNodePtr node, const int index);
	FollowNodePtr						GetPreviousNode(const FollowNodePtr node) const;
	FollowNodePtr						GetNextNode(const FollowNodePtr node) const;
	const	Steering::ReturnValue		Evaluate( Actor &self, const Vector &goalPosition );
	unsigned int const					SetPath( Actor &self, const Vector &from, const Vector &to );
	void								Clear();
	void								Draw( void ) const;

	void								SetCurrentNode(FollowNodePtr node) { _currentNode = node; }
	FollowNodePtr						GetCurrentNode(void) const { return _currentNode; }
	int									GetCurrentNodeIndex(void) const { return GetNodeIndex( GetCurrentNode() ); }
	FollowNodePtr						GetNodeAt(const int index) const { return _nodes.ObjectAt(index); }
	int const							GetNodeIndex(const FollowNodePtr node) const { return _nodes.IndexOfObject(node); }
	FollowNodePtr						FirstNode(void) const { return _nodes.ObjectAt(1); }
	FollowNodePtr						LastNode(void) const { return _nodes.ObjectAt(NumNodes()); }
	int const							NumNodes(void) const { return _nodes.NumObjects(); }
	
	virtual void						Archive( Archiver &arc );
	   
private:
	void								BuildFromPathNodes(Path *path, const Actor &self);
	const bool							FindNewCurrentNode( const Actor &self );
	const Steering::ReturnValue			AdvanceCurrentNode( const Actor &self );
	const Steering::ReturnValue			RetreatCurrentNode( Actor &self, const Vector &goalPosition );
	
	Container<FollowNode *>				_nodes;
	FollowNodePtr						_currentNode;
	
};

inline void FollowNodePath::Archive( Archiver &arc )
{
	Class::Archive( arc );
	
	int i;
	int num;
	FollowNode *followNode;
	if ( arc.Saving() )
	{
		num = _nodes.NumObjects();
		
		arc.ArchiveInteger( &num );
		
		for( i = 1 ; i <= num ; i++ )
		{
			followNode = _nodes.ObjectAt( i );
			arc.ArchiveObject( followNode );
		}
	}
	else
	{
		arc.ArchiveInteger( &num );
		
		_nodes.ClearObjectList();
		_nodes.Resize( num );
		
		for( i = 1 ; i <= num ; i++ )
		{
			followNode = new FollowNode;
			_nodes.AddObject( followNode );
			arc.ArchiveObject( followNode );
		}
	}
	
	arc.ArchiveSafePointer( &_currentNode );
}

//------------------------- CLASS ------------------------------
//
// Name:		FollowPath
// Base Class:	Steering
//
// Description:	FollowPath is the base class for and Steering 
// classes that need to navigate any significant distance through
// the level. 
//
// Method of Use:	Never instantiate an object of type FollowPath.
// If the TikiEngine architecture allowed, this cless would be an
// interface class. If you need FollowPath behavior either use an
// existing FollowPath subclass or derive a new class from a 
// FollowPath class.
//
//--------------------------------------------------------------
class FollowPath : public Steering
{
public:
								CLASS_PROTOTYPE( FollowPath );

								FollowPath();
								virtual						~FollowPath() { DeleteTemporaryPathNodes(); }
	virtual const Vector &		GetGoalPosition(void) const { assert (false); return vec_zero; }
	virtual const float 		GetGoalRadius(void) const { assert (false); return 0.0f; }
	virtual FollowNodePath &	GetPath(void) { return _path; }
	virtual const float			GetRadius (void) const { return _radius; }
	virtual void				SetRadius (const float radius) { _radius = radius; }
	virtual const float			GetAvoidanceDistance (void) const { return _avoidanceDistance; }
	virtual void				SetAvoidanceDistance (const float avoidanceDistance) { _avoidanceDistance = avoidanceDistance; }
	virtual void				SetSteeringForceAndDesiredHeading( Actor &self, const Vector &steeringForce );
	
	virtual void				Begin( Actor &self );
	virtual const ReturnValue	Evaluate( Actor &self );
	virtual void				ShowInfo( Actor &self );
	
	virtual void				Archive( Archiver &arc );
	
	
protected:
	virtual void				DeleteTemporaryPathNodes(void);
	virtual const bool			ClearTraceToGoal( Actor &self, const trace_t &traceToGoal, const float radius ) const;
	virtual const bool			DoneTurning( Actor &self ) const;
	virtual const ReturnValue	GotoCurrentNode( Actor &self );
	virtual const ReturnValue	GotoGoal( Actor &self );
	virtual const bool			BuildAvoidancePath(Actor  &self, const bool passOnTheLeft, const Vector &obstaclePosition, const float avoidanceRadius, const bool pursueGoal );
	virtual const ReturnValue	AvoidObstacle( Actor &self, const trace_t & trace, const bool pursueGoal=false );
	virtual const bool			TraceBlockedByEntity(Actor &self, const trace_t & trace ) const;
	virtual const bool			CheckBlocked(Actor &self);
	virtual const ReturnValue	ReturnBlockingObject(const trace_t &trace) const;
	virtual const bool			AtDestination(Actor &self) const;
	virtual void				SteerToCurrentNode(Actor &self);
	virtual void				SteerToGoal( Actor &self );
	virtual void				FreePath( void ) { _path.Clear(); }
	virtual void				AddPathNode(FollowNodePtr node) { GetPath().AddNode( node ); }
	virtual void				InsertPathNode(FollowNodePtr node, const int index) { 	GetPath().InsertNode(node, index); SetCurrentNode(node); }
	virtual FollowNodePtr		GetCurrentNode(void) const { return _path.GetCurrentNode(); }
	virtual void				SetCurrentNode(FollowNodePtr node) { _path.SetCurrentNode(node); }
	
private:
	float						_radius;
	FollowNodePath				_path;
	float						_avoidanceDistance;
	Vector						_desiredHeading;
	Jump						_jump;
	qboolean					_jumping;
	str							_oldAnim;
};

inline void FollowPath::Archive
(
	Archiver &arc
	)
	
{
	Steering::Archive( arc );
	
	arc.ArchiveFloat(	&_radius );
	arc.ArchiveObject(	&_path );
	arc.ArchiveFloat(	&_avoidanceDistance );
	arc.ArchiveVector(  &_desiredHeading );
	arc.ArchiveObject(	&_jump );
	arc.ArchiveBoolean(	&_jumping );
	arc.ArchiveString(	&_oldAnim );
}

#endif // FollowPath
