//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/FollowPath.cpp                            $
// $Revision:: 47                                                             $
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
// Provides base functionality for path following with avoidance
//

#include "_pch_cpp.h"
#include "actor.h"
#include "FollowPath.h"

//----------------------------------------------------------------
// Name:			DrawBox
// Class:			none
//
// Description:		draws a box in the xy plane 
//
// Parameters:		
//					Vector center - the point about which the box is centered
//					float width - the width of the box
//					float r, g, b - the color values for the box
//					float alpha - the amount of alpha applied to the box
//
// Returns:			None
//----------------------------------------------------------------

void DrawBox(const Vector &center, const float width, const float r, const float g, const float b, const float alpha )
{
	float squareSize=width/2.0f;
	Vector topLeftPoint		= center + Vector(-squareSize, -squareSize, 0);
	Vector topRightPoint		= center + Vector(+squareSize, -squareSize, 0);
	Vector bottomRightPoint	= center + Vector(+squareSize, +squareSize, 0);
	Vector bottomLeftPoint	= center + Vector(-squareSize, +squareSize, 0);
	G_DebugLine( topLeftPoint,			topRightPoint, r, g, b, alpha );
	G_DebugLine( topRightPoint,		bottomRightPoint, r, g, b, alpha );
	G_DebugLine( bottomRightPoint,	bottomLeftPoint, r, g, b, alpha );
	G_DebugLine( bottomLeftPoint,		topLeftPoint, r, g, b, alpha );
}

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

CLASS_DECLARATION( Class, FollowNode, NULL )
{
	{ NULL, NULL }
};

//----------------------------------------------------------------
// Name:			FollowNode
// Class:			FollowNode
//
// Description:		default constructor
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
FollowNode::FollowNode ( void ):
	_position( 0, 0, 0 ),
	_isTemporary( false ),
	_isJumpNode( false ),
	_jumpAngle( 45.0f )
{
}

//----------------------------------------------------------------
// Name:			FollowNode
// Class:			FollowNode
//
// Description:		constructor
//
// Parameters:		
//					Vector position - translation of the FollowNode
//					bool isTemporary - flag identifying temporary nodes
//
// Returns:			None
//----------------------------------------------------------------
FollowNode::FollowNode ( const Vector &position, const bool isTemporary, const bool isJumpNode, const float jumpAngle ):
	_position( position ),
	_isTemporary( isTemporary ),
	_isJumpNode( isJumpNode ),
	_jumpAngle( jumpAngle )

{
}

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
CLASS_DECLARATION( Class, FollowNodePath, NULL )
{
	{ NULL, NULL }
};

//----------------------------------------------------------------
// Name:			FollowNodePath
// Class:			FollowNodePath
//
// Description:		default constructor
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
FollowNodePath::FollowNodePath():
	_nodes(),
	_currentNode(NULL)
{
}

//----------------------------------------------------------------
// Name:			RemoveNode
// Class:			FollowNodePath
//
// Description:		removes node from path while maintaining 
//					the correct current node
//
// Parameters:		FollowNodePtr node - pointer to the node to 
//					be removed
//
// Returns:			None
//----------------------------------------------------------------
void FollowNodePath::RemoveNode(FollowNodePtr node)
{
	if (node == GetCurrentNode())
	{
		FollowNodePtr newCurrentNode=GetNextNode(GetCurrentNode());
		if (newCurrentNode == NULL)
		{
			newCurrentNode=GetPreviousNode(GetCurrentNode());
		}
		SetCurrentNode(newCurrentNode);
	}
	_nodes.RemoveObject(node);
}


//----------------------------------------------------------------
// Name:			InsertNode
// Class:			FollowNodePath
//
// Description:		inserts a  node into the path while maintaining 
//					the correct current node
//
// Parameters:		
//					FollowNodePtr node - pointer to the node to 
//					be inserted
//					int index - the index in the list where the 
//					node is to be inserted
//
// Returns:			None
//----------------------------------------------------------------
void FollowNodePath::InsertNode(FollowNodePtr node, const int index)
{
	int currentNodeIndex=GetNodeIndex(GetCurrentNode());
	_nodes.InsertObjectAt(index, node);
	if (currentNodeIndex >= index)
	{
		SetCurrentNode(GetNodeAt(currentNodeIndex+1));
	}
}

//----------------------------------------------------------------
// Name:			GetPreviousNode
// Class:			FollowNodePath
//
// Description:		returns a pointer to the node preceding the 
//					one passed to the method
//
// Parameters:		
//					FollowNodePtr node - node following the node 
//					to be returned
//
// Returns:			the node preceding 'node'
//----------------------------------------------------------------
FollowNodePtr FollowNodePath::GetPreviousNode(const FollowNodePtr node ) const
{
	int nodeIndex=GetNodeIndex(node);
	if (nodeIndex > 1)
	{
		return GetNodeAt(nodeIndex - 1);
	}
	return NULL;
}

//----------------------------------------------------------------
// Name:			GetNextNode
// Class:			FollowNodePath
//
// Description:		returns a pointer to the node following the 
//					one passed to the method
//
// Parameters:		
//					FollowNodePtr node - node preceding the node 
//					to be returned
//
// Returns:			the node following 'node'
//----------------------------------------------------------------
FollowNodePtr FollowNodePath::GetNextNode(const FollowNodePtr node ) const
{
	int nodeIndex=GetNodeIndex(node);
	if (nodeIndex < NumNodes())
	{
		return GetNodeAt(nodeIndex + 1);
	}
	return NULL;
}

//----------------------------------------------------------------
// Name:			Evaluate
// Class:			FollowNodePath
//
// Description:		ensures that the path maintains the correct
//					current node as the actor follows the path
//
// Parameters:		
//					Actor self - the actor following the path
//					Vector goalPosition - the place that the path 
//					is taking the Actor
//
// Returns:			Steering::ReturnValue used to return success 
//					or reason for failure
//----------------------------------------------------------------
const	Steering::ReturnValue FollowNodePath::Evaluate( Actor &self, const Vector &goalPosition )
{
	Steering::ReturnValue returnValue = Steering::EVALUATING;
	if (
			( NumNodes() < 1 ) &&
			( SetPath( self, self.origin, goalPosition ) != PATH_CREATION_SUCCESS )
		)
	{
		return Steering::FAILED_NO_PATH;
	}
	else 
	{
		returnValue = AdvanceCurrentNode( self );

		if (RetreatCurrentNode( self, goalPosition ) == Steering::FAILED_NO_PATH)
		{
			returnValue = Steering::FAILED_NO_PATH;
		}

		if ( g_showactorpath->integer )
		{
			Draw();
		}
	}	
	return returnValue;
}

//----------------------------------------------------------------
// Name:			SetPath
// Class:			FollowNodePath
//
// Description:		builds a new path for an Actor to follow
//
// Parameters:		
//					Actor self - the actor following the path
//					Vector from - the place that the path starts
//					Vector to - the place that the path leads to
//
// Returns:			bool that is false if no path is created
//----------------------------------------------------------------
unsigned int const FollowNodePath::SetPath(Actor &self, const Vector &from, const Vector &to)
{
	// Find the end node
	PathNode *goalNode = thePathManager.NearestNode( to, &self );
	
	if ( !goalNode )
	{
		return PATH_CREATION_FAILED_END_NODE_NOT_FOUND;
	}
	
	// Find the start node
	PathNode *node = thePathManager.NearestNode( from, &self );
	if ( !node || ( goalNode == node ) )
	{
		return PATH_CREATION_FAILED_START_NODE_NOT_FOUND;
	}
	
	// Find the best _path
	StandardMovePath find;
	find.heuristic.setSize( self.size );
	find.heuristic.entnum = self.entnum;
	find.heuristic.can_jump = self.CanJump() != 0;
	
	Clear();

	Path *newpath = find.FindPath( node, goalNode );
	if ( newpath != NULL )
	{
		BuildFromPathNodes( newpath, self );
		delete newpath;
		newpath = NULL;
		if ( self.GetActorFlag( ACTOR_FLAG_STRICTLY_FOLLOW_PATHS ) )
		{
			SetCurrentNode( FirstNode() );
		}
		else
		{
			if ( !FindNewCurrentNode( self ) )
			{
				SetCurrentNode( FirstNode() );
			}
		}
	}
	else
	{
		SetCurrentNode( NULL );
		return PATH_CREATION_FAILED_NODES_NOT_CONNECTED;
	}

	assert ( NumNodes() > 0 );
	//AddNode( new FollowNode( to, false ) );
	return PATH_CREATION_SUCCESS;
}

//----------------------------------------------------------------
// Name:			Clear
// Class:			FollowNodePath
//
// Description:		removes all FollowNodes from the path and 
//					deletes them
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
void FollowNodePath::Clear()
{
	for (int i=NumNodes(); i>0; i--)
	{
		FollowNodePtr node=GetNodeAt(i);
		RemoveNode( node );
		delete node;
	}
	_currentNode=NULL;
}

//----------------------------------------------------------------
// Name:			Draw
// Class:			FollowNodePath
//
// Description:		draws all nodes, all connections and the goal
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
void FollowNodePath::Draw( void ) const
{
	if (NumNodes() == 0)
	{
		return;
	}
	
	float squareSize=8.0f;
	Vector verticalOffset(0,0,16);
	DrawBox(GetNodeAt(1)->GetPosition() + verticalOffset, squareSize , 0, 1, 0, 1);
	for (int i=NumNodes(); i>1; i--)
	{
		if ( GetNodeAt(i)->GetIsTemporary() )
		{
			DrawBox(GetNodeAt(i)->GetPosition() + verticalOffset, squareSize , 1, 0, 1, 1);
		}
		else
		{
			DrawBox(GetNodeAt(i)->GetPosition() + verticalOffset, squareSize , 0, 1, 0, 1);
		}

		if (i%2)
		{
			G_DebugLine( GetNodeAt(i)->GetPosition() + verticalOffset, GetNodeAt(i-1)->GetPosition() + verticalOffset, 0.0f, 1.0f, 0.0f, 1.0f );
		}
		else
		{
			G_DebugLine( GetNodeAt(i)->GetPosition() + verticalOffset, GetNodeAt(i-1)->GetPosition() + verticalOffset, 0.0f, 0.5f, 0.0f, 1.0f );
		}
	}
	if (_currentNode!=NULL)
	{
		DrawBox(_currentNode->GetPosition() + verticalOffset, squareSize * 2, 1, 1, 1, 1);
	}
	DrawBox(LastNode()->GetPosition() + verticalOffset, squareSize * 4, 1, 0, 0, 1);
}

//----------------------------------------------------------------
// Name:			BuildFromPathNodes
// Class:			FollowNodePath
//
// Description:		builds a FollowNode path from a PathNode path
//
// Parameters:		
//					Actor self - the actor following the path
//					Vector from - the place that the path starts
//					Vector to - the place that the path leads to
//
// Returns:			bool that is false if no path is created
//----------------------------------------------------------------
void FollowNodePath::BuildFromPathNodes(Path *path, const Actor &self)
{
	assert(path->NumNodes() > 0);
	
	int sourceNodeNumber=1;
	int sourceNumNodes=path->NumNodes();
	
	
	AddNode (new FollowNode(path->GetNode(sourceNodeNumber++)->origin));
	PathNodePtr currentPathNode = NULL;
	PathNodePtr nextPathNode = NULL;
	while(sourceNodeNumber < sourceNumNodes)
	{
		bool isJumpNode = false;
		float jumpAngle = 0.0f;

		currentPathNode = path->GetNode(sourceNodeNumber);
		
		if ( sourceNodeNumber < path->NumNodes()  && (currentPathNode->nodeflags & AI_JUMP) && currentPathNode->target.length() )
		{
			nextPathNode = path->GetNode(sourceNodeNumber + 1);
			if (currentPathNode->target == nextPathNode->targetname)
			{
				isJumpNode = true;
				jumpAngle = currentPathNode->jumpAngle;
			}
		}

		AddNode ( new FollowNode( currentPathNode->origin, false, isJumpNode, jumpAngle ) );
		sourceNodeNumber++;
	}
	AddNode (new FollowNode( path->GetNode(sourceNumNodes)->origin));
}

//----------------------------------------------------------------
// Name:			FindNewCurrentNode
// Class:			FollowNodePath
//
// Description:		sets the current node to the node furthest 
//					down the path that the Actor can see
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			bool that is true if a new current node is set
//----------------------------------------------------------------
const bool FollowNodePath::FindNewCurrentNode(const Actor &self)
{
	int mask;
	int currentNodeIndex = GetNodeIndex( GetCurrentNode() );
	for ( 
		int currentTestNodeNumber = NumNodes(); 
		currentTestNodeNumber > currentNodeIndex;
		currentTestNodeNumber--
		)
	{
		FollowNodePtr testnode = GetNodeAt(currentTestNodeNumber);
		if (currentTestNodeNumber == 1)
		{
			SetCurrentNode(testnode);
			return true;
		}
		
		mask = self.edict->clipmask & ~CONTENTS_BODY;

		trace_t trace = self.Trace( self.origin, testnode->GetPosition(), mask, "FollowPath" );      
		
		if ( !trace.startsolid && ( trace.fraction == 1.0f ) && self.movementSubsystem->CanWalkTo( testnode->GetPosition() ) )
		{
			SetCurrentNode(testnode);
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------
// Name:			AdvanceCurrentNode
// Class:			FollowNodePath
//
// Description:		sets the current node to the next node if the 
//					Actor gets close to the current node
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			Steering::ReturnValue returns reason for failure
//----------------------------------------------------------------
const Steering::ReturnValue FollowNodePath::AdvanceCurrentNode( Actor const &self )
{
	if ( GetCurrentNode() != NULL )
	{
		// Advance current node if possible
		FollowNodePtr nextNode = GetNextNode( GetCurrentNode() );

		// check if the distance remaining is less than the distance we'll travel
		float radius = max( self.total_delta.lengthXY(), 16.0f );
		if ( Vector::DistanceXY( self.origin, GetCurrentNode()->GetPosition() ) <= radius )
		{
			if ( nextNode != NULL )
			{
				SetCurrentNode ( nextNode );
				return Steering::EVALUATING_REACHED_NODE;
			}
			else
			{
				Clear();
				return Steering::SUCCESS;
			}
		}
		return Steering::EVALUATING;
	}
	return Steering::FAILED_NO_PATH;
}

//----------------------------------------------------------------
// Name:			RetreatCurrentNode
// Class:			FollowNodePath
//
// Description:		search backwards from the current node searching
//					for a node that is not blocked by the world
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			Steering::ReturnValue returns reason for failure
//----------------------------------------------------------------
const Steering::ReturnValue FollowNodePath::RetreatCurrentNode(Actor &self, const Vector &goalPosition)
{
	int mask;

	if ( GetCurrentNode() == NULL )
	{
		return Steering::FAILED_NO_PATH;
	}

	if ( GetCurrentNodeIndex() == 1 )
	{
		return Steering::EVALUATING;
	}

	// Build the correct mask (the actor's normal mask without body)

	mask = self.edict->clipmask & ~CONTENTS_BODY;

	if ( !GetPreviousNode(GetCurrentNode() )->GetIsJumpNode() )
	{
		//Vector myPosition( self.origin );
		trace_t traceToCurrentNode = self.Trace( self.origin, GetCurrentNode()->GetPosition(), mask,  "FollowPath" );
		
		if ( traceToCurrentNode.startsolid || ( traceToCurrentNode.fraction < 1.0f ) )
		{
			gi.WDPrintf( "Couldn't complete path, building new one for %s (%d)\n" , self.TargetName(), self.entnum );

			if ( SetPath( self, self.origin, goalPosition ) != PATH_CREATION_SUCCESS )
			{
				return Steering::FAILED_NO_PATH;
			}
		}
	}
	return Steering::EVALUATING;
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
CLASS_DECLARATION( Steering, FollowPath, NULL )
{
	{ NULL, NULL }
};

//----------------------------------------------------------------
// Name:			FollowPath
// Class:			FollowPath
//
// Description:		default constructor
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
FollowPath::FollowPath():
	_avoidanceDistance(96.0f),
	_jumping( false )
{
}

//----------------------------------------------------------------
// Name:			Begin
// Class:			FollowPath
//
// Description:		Initializes variables necessary to Evaluate
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			None
//----------------------------------------------------------------
void FollowPath::Begin(Actor &self)
{
	_desiredHeading = self.movementSubsystem->getMoveDir().toAngles();
	_jumping = false;
}

//----------------------------------------------------------------
// Name:			SetSteeringForceAndDesiredHeading
// Class:			FollowPath
//
// Description:		Sets absolute direction that Actor should turn to
//
// Parameters:		
//					Actor self - the actor following the path
//					EulerAngles steeringForce - deltaRotation that
//					Actor should turn
//
// Returns:			None
//----------------------------------------------------------------
void FollowPath::SetSteeringForceAndDesiredHeading( Actor &self, const Vector &steeringForce )
{
	_desiredHeading = self.movementSubsystem->getMoveDir().toAngles() + steeringForce;
	_desiredHeading.EulerNormalize360();
	SetSteeringForce( steeringForce );
}

//----------------------------------------------------------------
// Name:			DoneTurning
// Class:			FollowPath
//
// Description:		test used to determine if the Actor is facing
//					the desired heading
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			bool - true if the Actor is facing the desired 
//					direction
//----------------------------------------------------------------
const bool FollowPath::DoneTurning( Actor &self ) const
{
	Vector deltaHeading( _desiredHeading - self.movementSubsystem->getMoveDir().toAngles() );
	deltaHeading.EulerNormalize();
	return fabs( deltaHeading[YAW] ) < 1.0f ;
}


//----------------------------------------------------------------
// Name:			GotoCurrentNode
// Class:			FollowPath
//
// Description:		steers the Actor towards the current FollowNode
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			Steering::ReturnValue returns reason for failure
//----------------------------------------------------------------
const Steering::ReturnValue FollowPath::GotoCurrentNode( Actor &self ) 
{
	float traceLength = min(Vector::DistanceXY( self.origin, GetCurrentNode()->GetPosition() ), GetAvoidanceDistance() );
	Vector traceEnd = self.origin + ( self.movementSubsystem->getMoveDir() * traceLength );
	trace_t horizontalTrace;
	trace_t verticalTrace;
	stepmoveresult_t result = self.movementSubsystem->IsMoveValid( horizontalTrace, verticalTrace, self.origin, traceEnd );
	if ( result == STEPMOVE_STUCK )
	{
		return Steering::FAILED;
	}
	if ( result == STEPMOVE_BLOCKED_BY_ENTITY )
	{
		return AvoidObstacle( self, horizontalTrace );
	}

	SteerToCurrentNode( self );
	return Steering::EVALUATING;

}

//----------------------------------------------------------------
// Name:			GotoGoal
// Class:			FollowPath
//
// Description:		steers the Actor towards the final goal
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			Steering::ReturnValue returns reason for failure
//----------------------------------------------------------------
const Steering::ReturnValue FollowPath::GotoGoal( Actor &self ) 
{
	float traceLength = min(Vector::DistanceXY( self.origin, GetGoalPosition() ), GetAvoidanceDistance() );
	Vector traceEnd = self.origin + ( self.movementSubsystem->getMoveDir() * traceLength );
	trace_t horizontalTrace;
	trace_t verticalTrace;
	stepmoveresult_t result = self.movementSubsystem->IsMoveValid( horizontalTrace, verticalTrace, self.origin, traceEnd );
	if ( result == STEPMOVE_STUCK )
	{
		return Steering::FAILED;
	}
	if ( result == STEPMOVE_BLOCKED_BY_ENTITY )
	{
		return AvoidObstacle( self, horizontalTrace, true );
	}

	SetSteeringForceAndDesiredHeading
									( 
										self, 
										self.movementSubsystem->SteerTowardsPoint
										( 
											GetGoalPosition(),			
											vec_zero,											
											self.movementSubsystem->getMoveDir(),		
											self.movementSubsystem->getMoveSpeed()		
										) 
									);
	return Steering::EVALUATING;
}

//----------------------------------------------------------------
// Name:			Evaluate
// Class:			FollowPath
//
// Description:		attempts to move the Actor to the goal position
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			Steering::ReturnValue returns reason for failure
//----------------------------------------------------------------
const Steering::ReturnValue FollowPath::Evaluate( Actor &self )
{
	ResetForces();


	if ( _jumping )
	{
		if ( _jump.Evaluate( self ) == Steering::SUCCESS)
		{
			_jump.End( self );
			_jumping = false;
			self.SetAnim( _oldAnim );
		}
		return Steering::EVALUATING;
	}
	

	if (AtDestination( self ))
	{
		FreePath();	

		//if ( g_showactorpath->integer )
		//	gi.WDPrintf("Success %d\n" , self.entnum );

		return Steering::SUCCESS;
	}


	Steering::ReturnValue returnValue = Steering::EVALUATING;

	trace_t traceToGoal;
	trace_t verticalTrace;
	stepmoveresult_t moveResult = self.movementSubsystem->IsMoveValid( traceToGoal, verticalTrace, self.origin, GetGoalPosition() );

	if  ( 
			(
			!self.GetActorFlag(ACTOR_FLAG_STRICTLY_FOLLOW_PATHS) &&
			(( moveResult == STEPMOVE_OK) || 
			( traceToGoal.entityNum != ENTITYNUM_WORLD && traceToGoal.entityNum != ENTITYNUM_NONE && ClearTraceToGoal( self, traceToGoal, GetGoalRadius() ) ) 
			) && self.movementSubsystem->CanWalkTowardsPoint( GetGoalPosition() ) )
		)
	{
		FreePath();
		SteerToGoal( self );
	}
	else
	{
		if ( DoneTurning( self ) )
		{
			returnValue = GetPath().Evaluate( self, GetGoalPosition() ); 

			int currentNodeIndex = GetPath().GetCurrentNodeIndex();
			if ( currentNodeIndex > 1 )
			{
				FollowNodePtr lastNode = GetPath().GetNodeAt( currentNodeIndex - 1 );
				const Vector currentNodePosition = GetPath().GetCurrentNode()->GetPosition();
				const float distanceFromActorToCurrentNode = Vector::DistanceXY( self.origin, currentNodePosition );
				const float distanceBetweenNodes = Vector::DistanceXY( lastNode->GetPosition(), currentNodePosition );
				if ( lastNode->GetIsJumpNode() && ( distanceFromActorToCurrentNode > 0.5f * distanceBetweenNodes ) )
				{
					_jumping = true;
					_oldAnim = self.animname;
					_jump.SetGoal( GetCurrentNode()->GetPosition() );
					_jump.SetLaunchAngle( lastNode->GetJumpAngle() );
					_jump.Begin( self );
					return Steering::EVALUATING;
				}
			}
			if ( returnValue == Steering::EVALUATING )
			{
				returnValue = GotoCurrentNode( self );
			}

			else if ( returnValue == Steering::FAILED_NO_PATH && moveResult != STEPMOVE_BLOCKED_BY_WORLD )
			{
				returnValue = GotoGoal( self );
			}
			
		}
		else
		{
			Vector newSteeringForce( _desiredHeading - self.movementSubsystem->getMoveDir().toAngles() );
			newSteeringForce.EulerNormalize();
			SetSteeringForce( newSteeringForce );
		}
	}


	if ( returnValue == Steering::EVALUATING )
	{
		self.movementSubsystem->Accelerate( GetSteeringForce() );
		return Steering::EVALUATING;
	}
	else
	{
		return returnValue;
	}
}

//----------------------------------------------------------------
// Name:			ShowInfo
// Class:			FollowPath
//
// Description:		prints out useful debug info for the class
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			None
//----------------------------------------------------------------
void FollowPath::ShowInfo( Actor &self )
{
   Steering::ShowInfo( self );
	
   gi.Printf( "\npath : ( %f, %f, %f ) to ( %f, %f, %f )\n",
      _path.FirstNode()->GetPosition().x, _path.FirstNode()->GetPosition().y, _path.FirstNode()->GetPosition().z,
      _path.LastNode()->GetPosition().x, _path.LastNode()->GetPosition().y, _path.LastNode()->GetPosition().z );
	
   if ( GetCurrentNode() )
	{
      gi.Printf( "currentNode: ( %f, %f, %f )\n",
         GetCurrentNode()->GetPosition().x, GetCurrentNode()->GetPosition().y, GetCurrentNode()->GetPosition().z );
	}
   else
	{
      gi.Printf( "currentNode: NULL\n" );
	}
}

//----------------------------------------------------------------
// Name:			DeleteTemporaryPathNodes
// Class:			FollowPath
//
// Description:		deletes all temporary FollowNodes in the path
//					and removes them from the path
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
void FollowPath::DeleteTemporaryPathNodes(void)
{
	int numNodes=GetPath().NumNodes();
	int i;
	for (i=numNodes; i>0; i--)
	{
		FollowNodePtr nodeToRemove=GetPath().GetNodeAt(i);
		if (nodeToRemove->GetIsTemporary())
		{
			GetPath().RemoveNode(nodeToRemove);
		}
	}
}

//----------------------------------------------------------------
// Name:			ClearTraceToGoal
// Class:			FollowPath
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
const bool FollowPath::ClearTraceToGoal( Actor &self, const trace_t &traceToGoal, const float radius ) const
{
	// Totally clear path
	bool traceToGoalIsClear = traceToGoal.fraction == 1.0f;																			
	
	// Path is blocked but obstracle is inside goal radius
	float distanceToEdgeOfGoal = Vector::DistanceXY( self.origin, GetGoalPosition() ) - radius;  
	float lengthOfTrace = Vector::DistanceXY( self.origin, Vector( traceToGoal.endpos ) );
	bool traceGotInsideGoalRadius = ( distanceToEdgeOfGoal < lengthOfTrace );														

	return ( !traceToGoal.startsolid && ( traceToGoalIsClear || traceGotInsideGoalRadius) );
}

//----------------------------------------------------------------
// Name:			BuildAvoidancePath
// Class:			FollowPath
//
// Description:		attempts to build a temporary path around one 
//					side of the obstacle
//
// Parameters:		
//					Actor self - the actor following the path
//					bool passOnTheLeft - should the path be created 
//					on the left side if the obstacle
//					Vector obstaclePosition - center point of the 
//					obstacle
//					float avoidanceRadius - distance used to avoid 
//					obstacle
//
// Returns:			bool that is true if a path is successfully 
//					created
//----------------------------------------------------------------
const bool FollowPath::BuildAvoidancePath(Actor  &self, const bool passOnTheLeft, const Vector &obstaclePosition, const float avoidanceRadius, const bool pursueGoal )
{
	DeleteTemporaryPathNodes();
	Vector myPosition(self.origin);
	float distanceToGoal=Vector::DistanceXY(myPosition, GetGoalPosition());
	Vector parallelDirection(self.movementSubsystem->getMoveDir());
	if ( distanceToGoal < avoidanceRadius * 2.0f || pursueGoal )
	{
		parallelDirection = GetGoalPosition() - myPosition;
	}
	else
	{
		int currentNodeIndex = GetPath().GetNodeIndex(GetPath().GetCurrentNode());
		if (currentNodeIndex < GetPath().NumNodes())
		{
			parallelDirection = GetPath().GetNodeAt(currentNodeIndex + 1)->GetPosition() - GetPath().GetCurrentNode()->GetPosition();
		}
	}

	parallelDirection.normalize();

	Vector perpendicularDirection;	
	perpendicularDirection.CrossProduct(parallelDirection, Vector(0,0,1));
	if (passOnTheLeft)
	{
		perpendicularDirection *= -1.0f;
	}

	Vector firstPathNodeLocation(obstaclePosition + (avoidanceRadius * (perpendicularDirection - parallelDirection)));
	trace_t testTraceForFirstPoint;
	trace_t verticalTrace;
	stepmoveresult_t firstMoveResult = self.movementSubsystem->IsMoveValid( testTraceForFirstPoint, verticalTrace, self.origin, firstPathNodeLocation );

	if ( ( firstMoveResult == STEPMOVE_OK || ClearTraceToGoal( self, testTraceForFirstPoint, avoidanceRadius/2.0f ) ) )
	{
		Vector secondPathNodeLocation(obstaclePosition + (avoidanceRadius * (perpendicularDirection + parallelDirection)));
		trace_t testTraceForSecondPoint;
		stepmoveresult_t secondMoveResult = self.movementSubsystem->IsMoveValid( testTraceForSecondPoint, verticalTrace, firstPathNodeLocation, secondPathNodeLocation );
		
		if ( ( secondMoveResult == STEPMOVE_OK || ClearTraceToGoal( self, testTraceForSecondPoint, avoidanceRadius/2.0f ) ) )
		{
			Vector thirdPathNodeLocation(obstaclePosition + (2.0f * avoidanceRadius * parallelDirection));
			trace_t testTraceForThirdPoint;
			stepmoveresult_t thirdMoveResult = self.movementSubsystem->IsMoveValid( testTraceForThirdPoint, verticalTrace, secondPathNodeLocation, thirdPathNodeLocation );
			
			if ( ( thirdMoveResult == STEPMOVE_OK || ClearTraceToGoal( self, testTraceForThirdPoint, avoidanceRadius/2.0f ) ) )
			{
				int insertionPoint=GetPath().GetNodeIndex(GetPath().GetCurrentNode());
				if (insertionPoint > 0)
				{
					FollowNodePtr nextNode=GetPath().GetNodeAt(insertionPoint);
					while (nextNode != NULL && (Vector::DistanceXY(nextNode->GetPosition(), myPosition) < Vector::DistanceXY(myPosition, thirdPathNodeLocation)))
					{
						insertionPoint++;
						if (insertionPoint <= GetPath().NumNodes())
						{
							nextNode=GetPath().GetNodeAt(insertionPoint);
						}
						else
						{
							nextNode=NULL;
						}
					}
				}
				else
				{
					insertionPoint=1;
				}
				if ( Vector::DistanceXY(myPosition, firstPathNodeLocation) < distanceToGoal)
				{
					if (Vector::DistanceXY(myPosition, secondPathNodeLocation) < distanceToGoal)
					{
						if (Vector::DistanceXY(myPosition, thirdPathNodeLocation) < distanceToGoal)
						{
							InsertPathNode(new FollowNode(thirdPathNodeLocation, true), insertionPoint);
						}
						InsertPathNode(new FollowNode(secondPathNodeLocation, true), insertionPoint);
					}
					InsertPathNode(new FollowNode(firstPathNodeLocation, true), insertionPoint);
					SetSteeringForceAndDesiredHeading( self,
														self.movementSubsystem->SteerTowardsPoint (
														firstPathNodeLocation,							// Target Position
														vec_zero,											// Target Acceleration
														self.movementSubsystem->getMoveDir(),		// Actor Move Direction
														self.movementSubsystem->getMoveSpeed())	// Actor Move Speed
														);
					return true;
				}
			}
		}
	}
	return false;
}

//----------------------------------------------------------------
// Name:			AvoidObstacle
// Class:			FollowPath
//
// Description:		attempts to build a temporary path around one 
//					side of the obstacle
//
// Parameters:		
//					Actor self - the actor following the path
//					trace_t trace - the trace that hits the obstacle
//
// Returns:			Steering::ReturnValue returns reason for failure
//----------------------------------------------------------------
//		
//  Given that there is an obstacle in the actor's way, the actor will attempt 
//  trace a route around the obstacle and then insert new temporary path nodes 
//  into the path to allow the actor to navigate around the obstacle without 
//  having to do larges amounts of complex steering every frame.
//
//  The plan is to pick a point (#1) that will get the actor away from the obstacle 
//  and will also allow the actor to pass the obstacle. Point 2 is passed the 
//  obstacle in the sense that it is further down a path that is parallel to the
//  direction from the actor to the obstacle. The third point is chosen to safely
//  get the actor back on the path.
//		
//		
//-----------------------------------------------------------------------------
//		
//		
//             O->   X    3---------->
//					  \      /
//                 1---2
//
//
//   Actor                     O->
//   Obstacle                  X 
//   First New Path Node       1 
//   Second New Path Node      2 
//   Third New Path Node       3 
//
//
//-----------------------------------------------------------------------------
const Steering::ReturnValue FollowPath::AvoidObstacle(Actor &self, const trace_t &trace, const bool pursueGoal )	
{
	Entity *hitEntity = G_GetEntity( trace.entityNum );
	assert( hitEntity->entnum != ENTITYNUM_WORLD );
	if ( hitEntity == NULL )
	{
		return Steering::FAILED;
	}

	Vector obstaclePosition( hitEntity->origin );
	float avoidanceRadius( hitEntity->mins.lengthXY() + self.mins.lengthXY() );

	bool passOnLeft = false;
	switch( self._steeringDirectionPreference )
	{
	case STEER_RIGHT_ALWAYS:
		passOnLeft = false;
		break;
	case STEER_LEFT_ALWAYS:
		passOnLeft = true;
		break;
	case STEER_RANDOMLY:
		{
			int random = rand() % 2;
			if ( random )
			{
				passOnLeft = false;
			}
			else
			{
				passOnLeft = true;
			}
		}
		break;
	case STEER_BEST:
		// To be implemented
		// Intended to allow obstable avoidance to make a guess as to which way will get to goal better
		passOnLeft = false;
		break;
	}

	bool foundPath = BuildAvoidancePath(self, passOnLeft, obstaclePosition, avoidanceRadius, pursueGoal );
	
	if (!foundPath)
	{
		foundPath = BuildAvoidancePath(self, !passOnLeft, obstaclePosition, avoidanceRadius, pursueGoal );
	}
	if (foundPath)
	{
		return Steering::EVALUATING;
	}
	else
	{
		return Steering::FAILED;
	}
}

//----------------------------------------------------------------
// Name:			TraceBlockedByEntity
// Class:			FollowPath
//
// Description:		test that determines if there is an entity that
//					we will collide with anywhere along the trace
//
// Parameters:		
//					Actor self - the actor following the path
//					trace_t trace - the trace that hits the obstacle
//
// Returns:			bool that is true if there is a blocking Entity
//----------------------------------------------------------------
const bool FollowPath::TraceBlockedByEntity(Actor &self, const trace_t & trace ) const
{
	if ( !trace.startsolid && (trace.fraction == 1.0f || trace.entityNum == ENTITYNUM_WORLD)) 
	{
		return false;
	}

	Entity *obstacleEntity = G_GetEntity( trace.entityNum );
	assert( obstacleEntity != NULL );

	if (obstacleEntity->velocity.lengthXY() < Vector::Epsilon())
	{
		return true;
	}

	Vector relativeVelocity = self.velocity - obstacleEntity->velocity;

	float rateOfClosure = self.movementSubsystem->getMoveDir() * relativeVelocity;

	// This means that the two entities are moving towards each other
	if ( rateOfClosure >= 0.0f ) 
	{
		/* Code to check if the obstacle will move out of the way quickly enough
		Vector myPosition(self.origin);
		float myRadius = self.maxs.length();

		Vector obstaclePosition = trace.endpos;
		float obstacleRadius = obstacleEntity->maxs.length();

		Vector obstacleDirection = obstaclePosition - myPosition;
		float obstacleDistance = obstacleDirection.length();
		obstacleDirection /= obstacleDistance; // normalize
		
		Vector perpendicularDirection; 
		perpendicularDirection.CrossProduct(obstacleDirection, Vector(0,0,1));

		float timeUntilCollision = obstacleDistance / self.total_delta.length(); // Adequate approximation

		Vector obstacleVelocity(obstacleEntity->velocity);
		float parallelSpeed=Vector::Dot(obstacleVelocity, obstacleDirection);
		float perpendicularSpeed=Vector::Dot(obstacleVelocity, perpendicularDirection);
		
		float parallelMove = parallelSpeed * timeUntilCollision;
		float perpendicularMove = perpendicularSpeed * timeUntilCollision;
		
		// If this is true then the obstacle entity will not get out of the way fast 
		// enoughm so we need to avoid them
		if (perpendicularMove < parallelMove + myRadius + obstacleRadius)
		*/
		{
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------
// Name:			CheckBlocked
// Class:			FollowPath
//
// Description:		test that determines if the Actor is blocked 
//					by a door
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			bool that is true if the Actor is blocked 
//----------------------------------------------------------------
const bool FollowPath::CheckBlocked( Actor &self )
{
	trace_t trace = self.Trace( GetAvoidanceDistance() , "FollowPath" );
	
	if ( trace.ent )
	{
		Entity *door = trace.ent->entity;
		if ( door && door->isSubclassOf( Door ) )
		{
			Event *ev = new Event( EV_Use );
			ev->AddEntity( &self );
			door->PostEvent( ev, 0 );
			
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------
// Name:			ReturnBlockingObject
// Class:			FollowPath
//
// Description:		returns a code based on the type of Entity
//					that is blocking the Actor
//
// Parameters:		
//					trace_t trace - the trace that hits the obstacle
//
// Returns:			Steering::ReturnValue returns reason for failure
//----------------------------------------------------------------
const Steering::ReturnValue	FollowPath::ReturnBlockingObject(const trace_t &trace) const
{
	if (trace.entityNum == ENTITYNUM_WORLD)
	{
		return Steering::FAILED_BLOCKED_BY_WORLD;
	}
	else
	{
		Entity *traceEnt = G_GetEntity( trace.entityNum );
		if ( traceEnt && traceEnt->isSubclassOf( Actor) )
		{
			Actor *traceActor = static_cast<Actor *>(traceEnt);
			switch (traceActor->actortype)
			{
			case IS_ENEMY:
				return Steering::FAILED_BLOCKED_BY_ENEMY;
				break;
			case IS_CIVILIAN:
				return Steering::FAILED_BLOCKED_BY_CIVILIAN;
				break;
			case IS_FRIEND:
				return Steering::FAILED_BLOCKED_BY_FRIEND;
				break;
			case IS_TEAMMATE:
				return Steering::FAILED_BLOCKED_BY_TEAMMATE;
				break;
			}
		}
	}
	return Steering::ERROR;
}

//----------------------------------------------------------------
// Name:			AtDestination
// Class:			FollowPath
//
// Description:		test that determines if the Actor is close to
//					the goal
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			bool that is true if the Actor is at the goal
//----------------------------------------------------------------
const bool FollowPath::AtDestination(Actor &self) const
{
	//float dist = Vector::DistanceXY(GetGoalPosition(), self.origin);
	//gi.WDPrintf( "Distance: %f , %d\n" , dist , self.entnum );


	//Check if we are in range 
	if ((Vector::DistanceXY(GetGoalPosition(), self.origin)) <= GetRadius())
	{
		return true;
	}

	return false;
}

//----------------------------------------------------------------
// Name:			SteerToCurrentNode
// Class:			FollowPath
//
// Description:		steers the Actor towards the current FollowNode
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			None
//----------------------------------------------------------------
void FollowPath::SteerToCurrentNode(Actor &self )
{
   assert ( GetCurrentNode() != NULL );

	SetSteeringForceAndDesiredHeading
									( 
										self, 
										self.movementSubsystem->SteerTowardsPoint
										( 
											GetCurrentNode()->GetPosition(),			
											vec_zero,											
											self.movementSubsystem->getMoveDir(),		
											self.movementSubsystem->getMoveSpeed()		
										) 
									);
}

//----------------------------------------------------------------
// Name:			SteerToGoal
// Class:			FollowPath
//
// Description:		steers the Actor towards the goal
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			None
//----------------------------------------------------------------
void FollowPath::SteerToGoal( Actor &self )
{
    Vector newSteeringForce(self.movementSubsystem->SteerTowardsPoint(
																GetGoalPosition(), 
																vec_zero, 
																self.movementSubsystem->getMoveDir(), 
																self.movementSubsystem->getMoveSpeed() 
																) );

	SetSteeringForceAndDesiredHeading( self, newSteeringForce );
}
