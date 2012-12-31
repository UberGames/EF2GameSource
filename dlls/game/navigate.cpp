//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/navigate.cpp                                  $
// $Revision:: 44                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:43a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// C++ implementation of the A* search algorithm.
//

#include "_pch_cpp.h"
#include "navigate.h"
#include "path.h"
#include "misc.h"
#include "doors.h"

#define PATHFILE_VERSION 10

Container<int> SpecialPathNodes;

Event EV_AI_SavePaths
(
	"ai_savepaths",
	EV_CHEAT,
	NULL,
	NULL,
	"Saves the path nodes under the default name."
);
Event EV_AI_SaveNodes
(
	"ai_save",
	EV_CHEAT,
	"s",
	"filename",
	"Save path nodes."
);
Event EV_AI_LoadNodes
(
	"ai_load",
	EV_CHEAT,
	"s",
	"filename",
	"Loads path nodes."
);
Event EV_AI_ClearNodes
(
	"ai_clearnodes",
	EV_CHEAT,
	NULL,
	NULL,
	"Clears all of the path nodes."
);
Event EV_AI_RecalcPaths
(
	"ai_recalcpaths",
	EV_CHEAT,
	"i",
	"nodenum",
	"Update the specified node."
);
Event EV_AI_CalcPath
(
	"ai_calcpath",
	EV_CHEAT,
	"ii",
	"nodenum1 nodenum2",
	"Calculate path from node1 to node2."
);
Event EV_AI_SetNodeFlags
(
	"ai_setflags",
	EV_CHEAT,
	"iSSSSSS",
	"nodenum token1 token2 token3 token4 token5 token6",
	"Set the flags for the specified node."	);
Event EV_AI_OptimizeNodes
(
	"ai_optimize",
	EV_CHEAT,
	NULL,
	NULL,
	"Optimizes the path nodes."
);
Event EV_AI_CalcAllPaths
(
	"ai_calcallpaths",
	EV_CODEONLY,
	NULL,
	NULL,
	"Calculates all of the paths."
);
Event EV_AI_ConnectNodes
(
	"ai_connectNodes",
	EV_CHEAT | EV_SCRIPTONLY,
	"ss",
	"node1 node2",
	"Connects the 2 specified nodes."
);
Event EV_AI_DisconnectNodes
(
	"ai_disconnectNodes",
	EV_CHEAT | EV_SCRIPTONLY,
	"ss",
	"node1 node2",
	"Disconnects the 2 specified nodes."
);

cvar_t	*ai_createnodes = NULL;
cvar_t	*ai_debugpath;
cvar_t	*ai_debuginfo;
cvar_t	*ai_showroutes;
cvar_t	*ai_showroutes_distance;
cvar_t   *ai_shownodenums;
cvar_t   *ai_timepaths;
cvar_t	*ai_advanced_pathfinding;

//static Entity  *IgnoreObjects[ MAX_GENTITIES ];
//static int		NumIgnoreObjects;

//static PathNode *pathnodes[ MAX_PATHNODES ];
//static qboolean pathnodesinitialized = false;
//static qboolean loadingarchive = false;
//static qboolean _pathNodesCalculated = false;
//int ai_maxnode;

//#define	MASK_PATHSOLID		(CONTENTS_SOLID|CONTENTS_MONSTERCLIP)

PathManager thePathManager;

int path_checksthisframe;

PathNode *PathManager::FindNode( const char *name	)
{
	int i;
	
	if ( !name )
	{
		return NULL;
	}
	
	if ( name[ 0 ] == '!' )
	{
		name++;
		return thePathManager.GetNode( atoi( name ) );
	}
	
	if ( name[ 0 ] == '$' )
	{
		name++;
	}
	
	for( i = 0; i < _pathNodes.NumObjects(); i++ )
	{
		if ( _pathNodes[ i ]->TargetName() == name )
		{
			return _pathNodes[ i ];
		}
	}
	
	return NULL;
}

PathNode *PathManager::GetNode( int num )
{
	return _pathNodes[ num ];
}

void PathManager::AddNode( PathNode *node )
{
	_pathNodes.AddObject( node );
}

void PathManager::RemoveNode( PathNode *node )
{
	assert( node );
	_pathNodes.RemoveObject( node );
}

void PathManager::ResetNodes(	void )
{
	PathNode *pathNode;

	for( int j = _pathNodes.NumObjects(); j > 0; j-- )
	{
		pathNode = _pathNodes.ObjectAt( j );
		delete pathNode;
	}
	_pathNodes.FreeObjectList();

	for( int k = _specialPathNodes.NumObjects() ; k > 0 ; k-- )
	{
		pathNode = _specialPathNodes.ObjectAt( k );
		delete pathNode;
	}
	_specialPathNodes.FreeObjectList();

	for( int i = 0; i < PATHMAP_GRIDSIZE; i++ )
	{
		for( int j = 0; j < PATHMAP_GRIDSIZE; j++ )
		{
			if ( _mapCells[i][j] )
			{
				delete _mapCells[i][j];
				_mapCells[i][j] = NULL;
			}
		}
	}
}

/*****************************************************************************/
/*QUAKED info_pathnode (1 0 0) (-24 -24 0) (24 24 32) FLEE DUCK COVER DOOR JUMP LADDER ACTION WORK HIBERNATE

FLEE marks the node as a safe place to flee to.  Actor will be removed when it reaches a flee node and is not visible to a player.

DUCK marks the node as a good place to duck behind during weapon fire.

COVER marks the node as a good place to hide behind during weapon fire.

DOOR marks the node as a door node.  If an adjacent node has DOOR marked as well, the actor will only use the path if the door in between them is unlocked.

JUMP marks the node as one to jump from when going to the node specified by target.
"target" the pathnode to jump to.

WORK marks the node as one that the Actor can do work (Animate) from

HIBERNATE marks the node as one that the Actor can go into hibernation ( Basically an AI Active Sleep ) at


******************************************************************************/

Event EV_Path_FindChildren
(
	"findchildren",
	EV_CODEONLY,
	NULL,
	NULL,
	"Adds this node to the path manager."
);
Event EV_Path_FindEntities
(
	"findentities",
	EV_CODEONLY,
	NULL,
	NULL,
	"Finds doors."
);
Event EV_Path_SetNodeFlags
(
	"spawnflags",
	EV_CODEONLY,
	"i",
	"node_flags",
	"Sets the path nodes flags."
);
Event EV_Path_SetOriginEvent
(
	"origin",
	EV_SCRIPTONLY,
	"v",
	"origin",
	"Sets the path node's origin."
);
Event EV_Path_SetAngles
(
	"angles",
	EV_SCRIPTONLY,
	"v",
	"angles",
	"Sets the path node's angles."
);
Event EV_Path_SetAnim
(
	"anim",
	EV_DEFAULT,
	"s",
	"animname",
	"Sets the animname used for this path node."
);
Event EV_Path_SetTargetname
(
	"targetname",
	EV_SCRIPTONLY,
	"s",
	"targetname",
	"Sets the target name for this path node."
);
Event EV_Path_SetJumpAngle
(
	"jumpangle",
	EV_SCRIPTONLY,
	"f",
	"jumpangle",
	"Sets the jump angle for this path node."
);
Event EV_Path_SetTarget
(
	"target",
	EV_SCRIPTONLY,
	"s",
	"target",
	"Sets the target for this path node."
);

CLASS_DECLARATION( Class, PathNodeConnection, "PathNodeConnection" )
{
	{ NULL, NULL }
};

CLASS_DECLARATION( Listener, PathNode, "info_pathnode" )
{
	{ &EV_Path_FindChildren,			&PathNode::FindChildren },
	{ &EV_Path_FindEntities,			&PathNode::FindEntities },
	{ &EV_Path_SetNodeFlags,			&PathNode::SetNodeFlags },
	{ &EV_Path_SetOriginEvent,			&PathNode::SetOriginEvent },
	{ &EV_SetAngle,						&PathNode::SetAngle },
	{ &EV_Path_SetAngles,				&PathNode::SetAngles },
	{ &EV_Path_SetAnim,					&PathNode::SetAnim },
	{ &EV_Path_SetTargetname,			&PathNode::SetTargetname },
	{ &EV_Path_SetJumpAngle,			&PathNode::SetJumpAngle },
	{ &EV_Path_SetTarget,				&PathNode::SetTarget },

	{ NULL, NULL }
};

static Vector     pathNodesChecksum;
static int        numLoadNodes = 0;
//static int			numNodes = 0;
//static PathNode	*NodeList = NULL;

PathNode::PathNode()
{
	numLoadNodes++;
	if ( !thePathManager.IsLoadingArchive() )
	{
		// our archive function will take care of this stuff
		thePathManager.AddNode( this );
		
		thePathManager.CancelEventsOfType( EV_AI_CalcAllPaths );
		thePathManager.PostEvent( EV_AI_CalcAllPaths, 0.0f );
		
		//PostEvent( EV_Path_FindChildren, 0.0f );
	}
	
	occupiedTime = 0;
	
	nodeflags = 0;
	setangles = false;
	drawtime = 0;
	contents = 0;
	
	occupiedTime = 0;
	entnum = 0;
	
	// crouch hieght
	setSize( Vector(-24, -24, 0), Vector(24, 24, 40) );
	
	f = 0;
	h = 0;
	g = 0;
	
	gridX = 0;
	gridY = 0;
	inlist = NOT_IN_LIST;
	
	// reject is used to indicate that a node is unfit for ending on during a search
	reject = false;
	
	Parent	= NULL;
	NextNode = NULL;
}

PathNode::~PathNode()
{
	thePathManager.RemoveNodeFromGrid( this );
	
	thePathManager.RemoveNode( this );
	
	for ( int i = 0; i < NumberOfConnections();i++ )
	{
		delete _connections[ i ];
	}

	_connections.ClearObjectList();

}

void PathNode::SetNodeFlags( Event *ev )
{
	nodeflags = ev->GetInteger( 1 );
}

void PathNode::SetOriginEvent( Event *ev )
{
	setOrigin( ev->GetVector( 1 ) );
	pathNodesChecksum += origin;
}

void PathNode::SetAngle( Event *ev )
{
	Vector movedir;
	setangles = true;
	
	movedir = G_GetMovedir( ev->GetFloat( 1 ) );
	setAngles( movedir.toAngles() );
}

void PathNode::SetAngles( Event *ev )
{
	setangles = true;
	setAngles( ev->GetVector( 1 ) );
}

void PathNode::SetAnim( Event *ev )
{
	animname = ev->GetString( 1 );
}

void PathNode::SetTargetname( Event *ev )
{
	targetname = ev->GetString( 1 );
}

void PathNode::SetJumpAngle( Event *ev )
{
	jumpAngle = ev->GetFloat( 1 );
}

void PathNode::SetTarget( Event *ev )
{
	target = ev->GetString( 1 );
}

str &PathNode::TargetName( void )
{
	return targetname;
}

void PathNode::setAngles( const Vector &ang )
{
	angles = ang;
}

void PathNode::setOrigin( const Vector &org )
{
	origin = org;
	contents = gi.pointcontents( origin, 0 );
}

void PathNode::setSize( const Vector &min, const Vector &max )
{
	mins = min;
	maxs = max;
}

void PathNode::Setup( const Vector &pos )
{
	CancelEventsOfType( EV_Path_FindChildren );
	
	setOrigin( pos );
	
	ProcessEvent( EV_Path_FindChildren );
}

void PathNode::Archive( Archiver &arc )
{
	int i;
	
	Listener::Archive( arc );
	
	// ???
	//int            contents;
	
	arc.ArchiveUnsigned( &nodeflags );
	arc.ArchiveFloat( &jumpAngle );
	arc.ArchiveVector( &origin );
	arc.ArchiveVector( &angles );
	
	if ( arc.Loading() )
	{
		setOrigin( origin );
		setAngles( angles );
	}
	
	// ???
	// Vector			mins;
	// Vector			maxs;
	
	arc.ArchiveBoolean( &setangles );
	arc.ArchiveString( &target );
	arc.ArchiveString( &targetname );
	arc.ArchiveString( &animname );
	
	// Don't archive, just for debugging
	// float				drawtime;
	
	arc.ArchiveFloat( &occupiedTime );
	arc.ArchiveInteger( &entnum );
	
	// Don't archive, only used between frames
	//pathlist_t		inlist;
	
	// Don't archive, not used
	//qboolean			reject;
	
	// Don't archive, already taken care of
	// EntityPtr		targetEntity;
	
	if ( arc.Loading() && !LoadingSavegame )
	{
		occupiedTime = 0;
		entnum       = 0;
	}
	
	int numChildren = NumberOfConnections();
	arc.ArchiveInteger( &numChildren );
	if ( arc.Loading() )
	{
		for( i = 0; i < numChildren; i++ )
		{
			PathNodeConnection *newConnection = new PathNodeConnection();
			arc.ArchiveInteger( &newConnection->targetNodeIndex );
			arc.ArchiveShort( &newConnection->moveCost );
			arc.ArchiveRaw( newConnection->maxheight, sizeof( newConnection->maxheight ) );
			arc.ArchiveInteger( &newConnection->door );
			_connections.AddObject( newConnection );
		}
	}
	else
	{
		for( i = 0; i < numChildren; i++ )
		{
			arc.ArchiveInteger( &_connections[i]->targetNodeIndex );
			arc.ArchiveShort( &_connections[i]->moveCost );
			arc.ArchiveRaw( _connections[i]->maxheight, sizeof( _connections[i]->maxheight ) );
			arc.ArchiveInteger( &_connections[i]->door );
		}
	}
	
	if ( arc.Loading() )
	{
		if ( !LoadingSavegame )
		{
			// Fixup the doors
			PostEvent( EV_Path_FindEntities, 0.0f );
		}
		thePathManager.AddNode( this );
		thePathManager.AddNodeToGrid( this );
	}
}

void PathNode::FindEntities( Event * )
{
	for( int i = 0; i < NumberOfConnections(); i++ )
	{
		PathNodeConnection *connection = _connections[ i ];
		if ( connection->door )
		{
			PathNode *node = thePathManager.GetNode( connection->targetNodeIndex );
			
			assert( node );
			
			Door *door = CheckDoor( node->origin );
			if ( door )
            {
				connection->door = door->entnum;
            }
			else
            {
				connection->door = 0;
            }
		}
	}
}

qboolean PathNode::TestMove( Entity *ent, const Vector &original_start, const Vector &original_end, const Vector &min,
		const Vector &max, qboolean allowdoors, qboolean fulltest )
{
	// NOTE: TestMove may allow wide paths to succeed when they shouldn't since it
	// may place the lower node above obstacles that actors can't step over.
	// Since any path that's wide enough for large boxes must also allow
	// thinner boxes to go through, you must ignore the results of TestMove
	// when thinner checks have already failed.
	trace_t  trace;
	Vector   end_trace;
	Vector   pos;
	Vector   dir;
	float    t;
	float    dist;
	Vector	start;
	Vector	end;

	Q_UNUSED(allowdoors);
	
	// By requiring that paths have STEPSIZE headroom above the path, we simplify the test
	// to see if an actor can move to a node down to a simple trace.  By stepping up the start
	// and end points, we account for the actor's ability to step up any geometry lower than
	// STEPSIZE in height.
	
	start = original_start;
	end   = original_end;
	
	start.z += STEPSIZE;
	end.z   += STEPSIZE;
	
	// Check the move
	trace = G_Trace( start, min, max, end, ent, MASK_PATHSOLID, false, "PathNode::TestMove 1" );
	if ( trace.startsolid || ( trace.fraction != 1.0f ) )
	{
		// No direct path available.  The actor will most likely not be able to move through here.
		return false;
	}
	
	if ( !fulltest )
	{
		// Since we're not doing a full test (full tests are only done when we're connecting nodes to save time),
		// we test to see if the midpoint of the move would only cause a change in height of STEPSIZE
		// from the predicted height.  This prevents most cases where a dropoff lies between a actor and a node.
		Vector pos;
		
		// Since we start and end are already STEPSIZE above the ground, we have to check twice STEPSIZE below
		// the midpoint to see if the midpoint is on the ground.
		dir = end - start;
		pos = start + ( dir * 0.5f );
		end_trace = pos;
		end_trace.z -= STEPSIZE * 2.0f;
		
		// Check that the midpos is onground.  This may fail on ok moves, but a true test would be too slow
		// to do in real time.  Also, we may miss cases where a dropoff exists before or after the midpoint.
		// Once the actor is close enough to the drop off, it will discover the fall and hopefully try
		// another route.
		trace = G_Trace( pos, min, max, end_trace, ent, MASK_PATHSOLID, false, "PathNode::TestMove 2" );
		if ( trace.startsolid || ( trace.fraction == 1.0f ) )
		{
			// We're not on the ground, so there's a good posibility that we can't make this move without falling.
			return false;
		}
	}
	else //if ( !( contents & MASK_WATER ) )
	{
		// When we're creating the paths during load time, we do a much more exhaustive test to see if the
		// path is valid.  This test takes a bounding box and moves it 8 units at a time closer to the goal,
		// testing the ground after each move.  The test involves checking whether we will fall more than
		// STEPSIZE to the ground (since we've raised the start and end points STEPSIZE above the ground,
		// we must actually test 2 * STEPSIZE down to see if we're on the ground).  After each test, we set
		// the new height of the box to be STEPSIZE above the ground.  Each move closer to the goal is only
		// done horizontally to simulate how the actors normally move.  This method ensures that any actor
		// wider than 8 units in X and Y will be able to move from start to end.
		//
		// NOTE: This may allow wide paths to succeed when they shouldn't since it
		// may place the lower node above obstacles that actors can't step over.
		// Since any path that's wide enough for large boxes must also allow
		// thinner boxes to go through, you must ignore the results of TestMove
		// when thinner checks have already failed.
		
		dir = end - start;
		dir.z = 0.0f;
		dist = dir.length();
		dir *= 1.0f / dist;
		
		// check the entire move
		pos = start;
		for( t = 0.0f ; t < dist; t += 8.0f )
		{
			// Move the box to our position along the path and make our downward trace vector
			end_trace.x = pos.x = start.x + ( t * dir.x );
			end_trace.y = pos.y = start.y + ( t * dir.y );
			end_trace.z = pos.z - ( STEPSIZE * 2.0f );
			
			// check the ground
			trace = G_Trace( pos, min, max, end_trace, ent, MASK_PATHSOLID, false, "PathNode::TestMove 3" );
			if ( trace.startsolid || ( trace.fraction == 1.0f ) )
			{
				// Either we're stuck in something solid, or we would fall farther than STEPSIZE to the ground,
				// so the path is not acceptable.
				return false;
			}
			
			// move the box to STEPSIZE above the ground.
			pos.z = trace.endpos[ 2 ] + STEPSIZE;
		}
	}
	
	return true;
}

qboolean PathNode::CheckMove( Entity *ent, const Vector &pos, const Vector &minPos, const Vector &maxPos, qboolean allowdoors,
		qboolean fulltest )
{
	// Since we need to support actors of variable widths, we need to do some special checks when a potential
	// path goes up or down stairs.  Placed pathnodes are only 16x16 in width, so when they are dropped to the
	// ground, they may end in a position where a larger box would not fit.  Making the pathnodes larger
	// would make it hard to place paths where smaller actors could go, and making paths of various sizes would
	// be overkill (more work for the level designer, or a lot of redundant data).  The solution is to treat
	// paths with verticle movement differently than paths that are purely horizontal.  For horizontal moves,
	// a simple trace STEPSIZE above the ground will be sufficient to prove that we can travel from one node
	// to another, in either direction.  For moves that have some change in height, we can check that we have
	// a clear path by tracing horizontally from the higher node to a point where larger bounding box actors
	// could then move at a slope downward to the lower node.  This fixes the problem where path points that
	// are larger than the depth of a step would have to intersect with the step in order to get the center
	// of the box on solid ground.  If you're still confused, well, tough. :)  Think about the problem of
	// larger bounding boxes going up stairs for a bit and you should see the problem.  You can also read
	// section 8.4, "Translating a Convex Polygon", from Computational Geometry in C (O'Rourke) (a
	// great book, BTW) for information on similar problems (which is also a good explanation of how
	// Quake's collision detection works).
	trace_t trace;
	int height;
	
	height = ( int )fabs( pos.z - origin.z );
	// Check if the path is strictly horizontal
	if ( !height )
	{
		// We do two traces for the strictly horizontal test.  One normal, and one STEPSIZE
		// above.  The normal trace is needed because most doors in the game aren't tall enough
		// to allow actors to trace STEPSIZE above the ground.  This means that failed horizontal
		// tests require two traces.  Annoying.
		trace = G_Trace( origin, minPos, maxPos, pos, ent, MASK_PATHSOLID, false, "PathNode::CheckMove 1" );
		if ( !trace.startsolid && ( trace.fraction == 1.0f ) )
		{
			return true;
		}
		
		// Do the step test
		return TestMove( ent, pos, origin, minPos, maxPos, allowdoors, fulltest );
	}
	
	Vector size;
	float  width;
	
	size = maxPos - minPos;
	width = max( size.x, size.y );
	
	// if our bounding box is smaller than that of the pathnode, we can do the standard trace.
	if ( width <= 32.0f )
	{
		return TestMove( ent, pos, origin, minPos, maxPos, allowdoors, fulltest );
	}
	
	Vector start;
	Vector end;
	Vector delta;
	float  radius;
	float  len;
	
	// We calculate the radius of the smallest cylinder that would contain the bounding box.
	// In some cases, this would make the first horizontal move longer than it needs to be, but
	// that shouldn't be a problem.
	
	// multiply the width by 1/2 the square root of 2 to get radius
	radius = width * 1.415f * 0.5f;
	
	// Make sure that our starting position is the higher node since it doesn't matter which
	// direction the move is in.
	if ( pos.z < origin.z )
	{
		start = origin;
		end = pos;
	}
	else
	{
		start = pos;
		end = origin;
	}
	
	// If the distance between the two points is less than the radius of the bounding box,
	// then we only have to do the horizontal test since larger bounding boxes would not fall.
	delta = end - start;
	len = delta.length();
	if ( len <= radius )
	{
		end.z = start.z;
		return TestMove( ent, start, end, minPos, maxPos, allowdoors, fulltest );
	}
	
	Vector mid;
	
	// normalize delta and multiply by radius (saving a few multiplies by combining into one).
	delta *= radius / len;
	
	mid = start;
	mid.x += delta.x;
	mid.y += delta.y;
	
	// Check the horizontal move
	if ( !TestMove( ent, start, mid, minPos, maxPos, allowdoors, fulltest ) )
	{
		return false;
	}
	
	// Calculate our new endpoint
	end.z -= delta.z;
	
	// Check our new sloping move
	return TestMove( ent, mid, end, minPos, maxPos, allowdoors, fulltest );
}

Door *PathNode::CheckDoor( const Vector &pos )
{
	trace_t	trace;
	Entity	*ent;
	
	trace = G_Trace( origin, vec_zero, vec_zero, pos, NULL, MASK_PATHSOLID, false, "PathNode::CheckDoor" );
	
	if ( trace.ent )
	{
		ent = trace.ent->entity;
	}
	else
	{
		ent = NULL;
	}
	if ( ent && ent->isSubclassOf( Door ) )
	{
		return ( Door * )ent;
	}
	
	return NULL;
}

qboolean PathNode::CheckMove( const Vector &pos, const Vector &min, const Vector &max )
{
	Q_UNUSED(pos); Q_UNUSED(min); Q_UNUSED(max);
	return true;
}

qboolean PathNode::CheckPath( const PathNode *node, const Vector &min, const Vector &max, qboolean fulltest )
{
	Vector	delta;
	qboolean allowdoors;
	qboolean result;
	
	delta = node->origin - origin;
	
	// Make sure nodes aren't too high above or below each other
	
	if ( ( delta[ 2 ] < -PATHMAP_MAX_DIST ) || ( delta[ 2 ] > PATHMAP_MAX_DIST ) )
		return false;
	
	delta[ 2 ] = 0;
	
	// Make sure nodes aren't too far from each other
	
	if ( delta.length() >= PATHMAP_MAX_DIST )
	{
		return false;
	}
	
	allowdoors = ( nodeflags & AI_DOOR ) && ( node->nodeflags & AI_DOOR );
	
	result = CheckMove( NULL, node->origin, min, max, allowdoors, fulltest );
	//RestoreEnts();
	
	return result;
}

qboolean PathNode::ClearPathTo( PathNode *node, byte maxheight[ NUM_WIDTH_VALUES ], qboolean fulltest )
{
	int      i;
	int      width;
	int      bottom;
	int      top;
	Vector   min;
	Vector   max;
	Vector   bmin;
	Vector   bmax;
	qboolean path;
	int      touch[ MAX_GENTITIES ];
	Entity   *ent;
	int      num;
	
	path = false;
	for( i = 0; i < NUM_WIDTH_VALUES; i++ )
	{
		maxheight[ i ] = 0;
	}
	
	width = (int)(NUM_WIDTH_VALUES * WIDTH_STEP * 0.5f);
	min = Vector( -width, -width, 0.0f );
	max = Vector( width, width, MAX_HEIGHT );
	G_CalcBoundsOfMove( origin, node->origin, min, max, &bmin, &bmax );
	
	num = gi.AreaEntities( bmin, bmax, touch, MAX_GENTITIES, qfalse );
	//num = gi.BoxEdicts( bmin, bmax, touch, MAX_GENTITIES, AREA_SOLID );
	for( i = 0; i < num; i++ )
	{
		ent = g_entities[ touch[ i ] ].entity;
		if ( ent && ent->isSubclassOf( Door ) )
		{
			ent->unlink();
		}
	}
	
	for( i = 0; i < NUM_WIDTH_VALUES; i++ )
	{
		width = (int)(( i + 1 ) * WIDTH_STEP * 0.5f);
		
		min.x = min.y = -width;
		max.x = max.y = width;
		
		// Perform a binary search to find the height of the path.  Neat, eh? :)
		bottom = 0;
		top = MAX_HEIGHT;
		int height = MAX_HEIGHT & ~0x3;
		while( top >= bottom )
		{
			//         height = ( ( bottom + top + 3 ) >> 1 ) & ~0x3;
			if ( !height )
            {
				break;
            }
			
			max.z = ( float )height;
			if ( !CheckPath( node, min, max, fulltest ) )
            {
				top = height - 4;
            }
			else
            {
				bottom = height + 4;
				maxheight[ i ] = height;
            }
			height = ( ( bottom + top + 3 ) >> 1 ) & ~0x3;
		}
		
		if ( !maxheight[ i ] )
		{
			// If no paths were available at this width, don't allow any wider paths.
			// CheckPath uses TestMove which may allow wide paths to succeed when they
			// shouldn't since it may place the lower node above obstacles that actors
			// can't step over.  Since any path that's wide enough for large boxes must
			// also allow thinner boxes to go through, this check avoids the hole in
			// TestMove's functioning.
			break;
		}
		
		path = true;
	}
	
	// Restore the doors
	for( i = 0; i < num; i++ )
	{
		ent = g_entities[ touch[ i ] ].entity;
		if ( ent && ent->isSubclassOf( Door ) )
		{
			ent->link();
		}
	}
	
	return path;
}

qboolean PathNode::LadderTo( const PathNode *node, byte maxheight[ NUM_WIDTH_VALUES ] )
{
	int i;
	int j;
	int m;
	int width;
	Vector min;
	Vector max;
	qboolean path;
	
	trace_t trace;
	
	if ( !( nodeflags & AI_LADDER ) || !( node->nodeflags & AI_LADDER ) )
	{
		return false;
	}
	
	if ( ( origin.x != node->origin.x ) || ( origin.y != node->origin.y ) )
	{
		return false;
	}
	
	path = false;
	
	for( i = 0; i < NUM_WIDTH_VALUES; i++ )
	{
		width = (int)(( i + 1 ) * WIDTH_STEP * 0.5f);
		min = Vector( -width, -width, 12.0f );
		max = Vector( width, width, 40.0f );
		
		trace = G_Trace( origin, min, max, node->origin, NULL, MASK_PATHSOLID, false, "PathNode::LadderTo 1" );
		if ( ( trace.fraction != 1.0f ) || trace.startsolid )
		{
			maxheight[ i ] = 0;
			continue;
		}
		
		path = true;
		
		m = 40;
		for( j = 48; j < MAX_HEIGHT; j+= 8 )
		{
			max.z = j;
			trace = G_Trace( origin, min, max, node->origin, NULL, MASK_PATHSOLID, false, "PathNode::LadderTo 2" );
			if ( ( trace.fraction != 1.0f ) || trace.startsolid )
			{
				break;
			}
			
			m = j;
		}
		
		maxheight[ i ] = m;
	}
	
	return path;
}

qboolean PathNode::ConnectedTo( PathNode *node )
{
	int i;
	
	for( i = 0; i < NumberOfConnections(); i++ )
	{
		if ( thePathManager.GetNode( _connections[ i ]->targetNodeIndex ) == node )
		{
			return true;
		}
	}
	
	return false;
}

void PathNode::ConnectTo( PathNode *node, const byte maxheight[ NUM_WIDTH_VALUES ], float cost, const Door *door )
{
	int i;
	
	if ( node != this )
	{
		PathNodeConnection *newConnection = new PathNodeConnection();
		newConnection->targetNodeIndex = thePathManager.GetPathNodeIndex( *node );
		for( i = 0; i < NUM_WIDTH_VALUES; i++ )
		{
			newConnection->maxheight[ i ] = maxheight[ i ];
		}
		newConnection->moveCost = ( int )cost;
		newConnection->door = door ? door->entnum : 0;
		AddConnection( *newConnection );
	}
}

void PathNode::ConnectTo( PathNode *node, const byte maxheight[ NUM_WIDTH_VALUES ] )
{
	Vector delta;
	Door *door;
	
	door = CheckDoor( node->origin );
	delta = node->origin - origin;
	ConnectTo( node, maxheight, delta.length(), door );
}

void PathNode::FindChildren( Event * )
{
	trace_t	trace;
	Vector	end;
	Vector	start;
	
	thePathManager.SetPathNodesCalculated( true );
	
	origin.x = (float)( ( int )( origin.x * 0.125f ) * 8 );
	origin.y = (float)( ( int )( origin.y * 0.125f ) * 8 );
	setOrigin( origin );
	
	//if ( !( contents & MASK_WATER ) )
	{
		start = origin + Vector( "0 0 1" );
		end = origin;
		end[ 2 ] -= 256.0f;
		
		trace = G_Trace( start, mins, maxs, end, NULL, MASK_PATHSOLID, false, "PathNode::FindChildren" );
		if ( ( trace.fraction != 1.0f ) && !trace.allsolid )
		{
			setOrigin( trace.endpos );
		}
	}
	
	thePathManager.AddNodeToGrid( this );
	
	thePathManager.CancelEventsOfType( EV_AI_OptimizeNodes );
	thePathManager.PostEvent( EV_AI_OptimizeNodes, 0.0f );
}

void PathNode::DrawConnections(	void )
{
	for( int i = 0; i < NumberOfConnections(); i++ )
	{
		PathNode *node = thePathManager.GetNode( _connections[ i ]->targetNodeIndex );
		
		G_DebugLine( origin + Vector( 0, 0, 24 ), node->origin + Vector( 0, 0, 24 ), 0.7f, 0.7f, 0.0f, 1.0f );
	}
}

void PathManager::DrawAllConnections( void )
{
	PathNode    *n;
	Vector      down;
	Vector      up;
	Vector      dir;
	Vector      p1;
	Vector      p2;
	Vector      p3;
	Vector      playerorigin;
	qboolean    showroutes;
	qboolean    shownums;
	qboolean    draw;
	int         maxheight;
	int         pathnum;
	
	showroutes = ( ai_showroutes->integer != 0 );
	shownums = ( ai_shownodenums->integer != 0 );
	
	if ( ( ai_showroutes->integer == 1 ) || ( ai_showroutes->integer == 0 ) )
	{
		pathnum = ( 32 / WIDTH_STEP ) - 1;
	}
	else
	{
		pathnum = ( ( ( int )ai_showroutes->integer ) / WIDTH_STEP ) - 1;
	}
	
	if ( ( pathnum < 0 ) || ( pathnum >= MAX_WIDTH ) )
	{
		gi.Printf( "ai_showroutes: Value out of range\n" );
		gi.cvar_set( "ai_showroutes", "0" );
		return;
	}
	
	// Figure out where the camera is
	
	if ( !g_entities[ 0 ].client )
		return;
	
	playerorigin.x = g_entities[ 0 ].client->ps.origin[ 0 ];
	playerorigin.y = g_entities[ 0 ].client->ps.origin[ 1 ];
	playerorigin.z = g_entities[ 0 ].client->ps.origin[ 2 ];
	
	playerorigin[ 2 ] += g_entities[ 0 ].client->ps.viewheight;
	
	for( int nodeNum = 0; nodeNum < _pathNodes.NumObjects(); nodeNum++ )
	{
		PathNode    *node = _pathNodes[ nodeNum ];
		if ( Vector( node->origin - playerorigin ).length() > ai_showroutes_distance->value )
		{
			continue;
		}
		
		if ( shownums )
		{
			G_DrawDebugNumber( node->origin + Vector( 0.0f, 0.0f, 14.0f ), thePathManager.GetPathNodeIndex( *node ), 1.5f, 1.0f, 1.0f, 0.0f );
		}
		
		draw = false;
		for( int i = 0; i < node->NumberOfConnections(); i++ )
		{
			n = thePathManager.GetNode( node->_connections[ i ]->targetNodeIndex );
			
			maxheight = node->_connections[ i ]->maxheight[ pathnum ];
			if ( maxheight == 0 )
            {
				continue;
            }
			
			draw = true;
			
			if ( !showroutes )
            {
				continue;
            }
			
			// don't draw the path if it's already been drawn by the destination node
			if ( ( n->drawtime < level.time ) || !n->ConnectedTo( node ) )
            {
				down.z = 2;
				up.z = maxheight;
				
				if ( ( n->nodeflags & AI_JUMP ) && ( node->nodeflags & AI_JUMP ) )
				{
					// These are jump nodes draw, them in blue instead of green
					G_DebugLine( node->origin + down, n->origin + down,     0.0f, 0.0f, 1.0f, 1.0f );
					G_DebugLine( n->origin + down,    n->origin + up,       0.0f, 0.0f, 1.0f, 1.0f );
					G_DebugLine( node->origin + up,   n->origin + up,       0.0f, 0.0f, 1.0f, 1.0f );
					G_DebugLine( node->origin + up,   node->origin + down,  0.0f, 0.0f, 1.0f, 1.0f );
				}
				else
				{
					G_DebugLine( node->origin + down, n->origin + down,     0.0f, 1.0f, 0.0f, 1.0f );
					G_DebugLine( n->origin + down,    n->origin + up,       0.0f, 1.0f, 0.0f, 1.0f );
					G_DebugLine( node->origin + up,   n->origin + up,       0.0f, 1.0f, 0.0f, 1.0f );
					G_DebugLine( node->origin + up,   node->origin + down,  0.0f, 1.0f, 0.0f, 1.0f );
				}
            }
			
			// draw an arrow for the direction
			dir.x = n->origin.x - node->origin.x;
			dir.y = n->origin.y - node->origin.y;
			dir.normalize();
			
			p1 = node->origin;
			p1.z += maxheight * 0.5f;
			p2 = dir * 8.0f;
			p3 = p1 + p2 * 2.0f;
			
			G_DebugLine( p1, p3, 0.0f, 1.0f, 0.0f, 1.0f );
			
			p2.z += 8.0f;
			G_DebugLine( p3, p3 - p2, 0.0f, 1.0f, 0.0f, 1.0f );
			
			p2.z -= 16.0f;
			G_DebugLine( p3, p3 - p2, 0.0f, 1.0f, 0.0f, 1.0f );
		}
		
		if ( !draw )
		{
			// Put a little X where the node is to show that it had no connections
			p1 = node->origin;
			p1.z += 2.0f;
			
			p2 = Vector( 12.0f, 12.0f, 0.0f );
			G_DebugLine( p1 - p2, p1 + p2, 1.0f, 0.0f, 0.0f, 1.0f );
			
			p2.x = -12.0f;
			G_DebugLine( p1 - p2, p1 + p2, 1.0f, 0.0f, 0.0f, 1.0f );
		}
		
		node->drawtime = level.time;
	}
}

MapCell::MapCell()
{
	Init();
}

MapCell::~MapCell()
{
	_pathNodes.ClearObjectList();
}

void MapCell::Init( void )
{
}

qboolean MapCell::AddNode( PathNode *node )
{
	_pathNodes.AddObject( node );
	return true;
}

qboolean MapCell::RemoveNode( PathNode *node )
{
	// Remove the node from the global list if it is still in the list
	
	if ( _pathNodes.ObjectInList( node ) )
		_pathNodes.RemoveObject( node );
	
	return true;
}

PathNode *MapCell::GetNode( int index )
{
	return _pathNodes[ index ];
}

int MapCell::NumNodes( void )
{
	return _pathNodes.NumObjects();
}

/*                         All
                     work and no play
                 makes Jim a dull boy. All
               work and no  play makes Jim a
             dull boy. All  work and no  play
           makes Jim a dull boy. All work and no
         play makes Jim a dull  boy. All work and
        no play makes Jim a dull boy. All work and
       no play makes Jim a dull boy. All work and no
      play makes Jim a dull boy. All work and no play
     makes Jim a dull boy. All work and no play makes
    Jim a dull boy.  All work and no  play makes Jim a
   dull boy. All work and no play makes Jim a dull boy.
   All work and no play makes  Jim a dull boy. All work
  and no play makes Jim a dull boy. All work and no play
  makes Jim a dull boy. All work and no play makes Jim a
 dull boy. All work and no play makes Jim a dull boy. All
 work and no play makes  Jim a dull boy. All  work and no
 play makes Jim a dull boy. All work and no play makes Jim
 a dull boy. All work  and no play makes Jim  a dull boy.
 All work and no play makes Jim  a dull boy. All work and
 no play makes Jim a dull boy. All work and no play makes
 Jim a dull boy.  All work and no  play makes Jim a  dull
 boy. All work and no play makes Jim a dull boy. All work
 and no play makes Jim  a dull boy. All work  and no play
 makes Jim a dull boy.  All work and no play  makes Jim a
 dull boy. All work and no play makes Jim a dull boy. All
  work and no play makes Jim a dull boy. All work and no
  play makes Jim a dull boy.  All work and no play makes
   Jim a dull boy. All work and no play makes Jim a dull
   boy. All work and no play  makes Jim a dull boy. All
    work and no play makes Jim a dull boy. All work and
     no play makes  Jim a dull  boy. All work  and no
      play makes Jim a dull boy. All work and no play
       makes Jim a dull  boy. All work and  no play
        makes Jim a dull boy. All work and no play
         makes Jim a  dull boy. All  work and no
           play makes Jim a  dull boy. All work
             and no play makes Jim a dull boy.
               All work  and no  play makes
                 Jim a dull boy. All work
                     and no play makes
                          Jim  a
*/

CLASS_DECLARATION( Listener, PathManager, NULL )
{
	{ &EV_AI_SavePaths,						&PathManager::SavePathsEvent },
	{ &EV_AI_LoadNodes,						&PathManager::LoadNodes },
	{ &EV_AI_SaveNodes,						&PathManager::SaveNodes },
	{ &EV_AI_ClearNodes,					&PathManager::ClearNodes },
	{ &EV_AI_OptimizeNodes,					&PathManager::OptimizeNodes },
	{ &EV_AI_SetNodeFlags,					&PathManager::SetNodeFlagsEvent },
	{ &EV_AI_RecalcPaths,					&PathManager::RecalcPathsEvent },
	{ &EV_AI_CalcPath,						&PathManager::CalcPathEvent },
	{ &EV_AI_CalcAllPaths,					&PathManager::CalcAllPaths },

	{ &EV_AI_ConnectNodes,					&PathManager::connectNodes },
	{ &EV_AI_DisconnectNodes,				&PathManager::disconnectNodes },
	
	{ NULL, NULL }
};


PathManager::PathManager()
{
	thePathManager.ResetNodes();
}

PathManager::~PathManager()
{
	thePathManager.ResetNodes();
}

void PathManager::AddToGrid( PathNode *node, int x, int y )
{
	PathNode *node2;
	MapCell *cell;
	int numnodes;
	int i;
	int j;
	byte maxheight[ NUM_WIDTH_VALUES ];
	
	cell = GetMapCell( x, y );
	if ( !cell )
	{
		return;
	}
	
	if ( !cell->AddNode( node ) )
	{
		warning( "AddToGrid", "Node overflow at ( %d, %d )\n", x, y );
		return;
	}
	
	if ( !IsLoadingArchive() )
	{
		//
		// explicitly link up the targets and their destinations
		//
		if ( node->nodeflags & AI_JUMP )
		{
			if ( node->target.length() > 1 )
            {
				node2 = FindNode( node->target.c_str() );
				if ( node2 && !node->ConnectedTo( node2 ) )
				{
					for( j = 0; j < NUM_WIDTH_VALUES; j++ )
					{
						maxheight[ j ] = MAX_HEIGHT;
					}
					
					if ( node != node2 )
						node->ConnectTo( node2, maxheight );
					else
						gi.WDPrintf( "Can't connect pathnode to itself (%s)", node->target.c_str() );
				}
            }
		}
		
		// Connect the node to its neighbors
		numnodes = cell->NumNodes();
		for( i = 0; i < numnodes; i++ )
		{
			node2 = ( PathNode * )cell->GetNode( i );
			if ( node2 == node )
			{
				continue;
			}
			
			if ( !node->ConnectedTo( node2 ) )
            {
				if ( node->ClearPathTo( node2, maxheight ) || node->LadderTo( node2, maxheight ) )
				{
					node->ConnectTo( node2, maxheight );
				}
            }
			
			if ( !node2->ConnectedTo( node ) )
            {
				if ( node2->ClearPathTo( node, maxheight ) || node2->LadderTo( node, maxheight ) )
				{
					node2->ConnectTo( node, maxheight );
				}
            }
		}
	}
}

qboolean PathManager::RemoveFromGrid( PathNode *node, int x, int y )
{
	MapCell	*cell = GetMapCell( x, y );
	if ( !cell->RemoveNode( node ) )
	{
		return false;
	}
	
	return true;
}

int PathManager::NodeCoordinate( float coord )
{
	return ( ( int )coord + MAX_WORLD_COORD - ( PATHMAP_CELLSIZE / 2 ) ) / PATHMAP_CELLSIZE;
}

int PathManager::GridCoordinate( float coord )
{
	return ( ( int )coord + MAX_WORLD_COORD ) / PATHMAP_CELLSIZE;
}

void PathManager::AddNodeToGrid( PathNode *node )
{
	int x;
	int y;
	
	assert( node );
	
	if ( !IsLoadingArchive() )
	{
		gi.DPrintf( "." );
	}
	
	
	x = NodeCoordinate( node->origin[ 0 ] );
	y = NodeCoordinate( node->origin[ 1 ] );
	
	AddToGrid( node, x, y );
	AddToGrid( node, x + 1, y );
	AddToGrid( node, x,	y + 1 );
	AddToGrid( node, x + 1, y + 1 );
	
	node->gridX = x;
	node->gridY = y;
}

void PathManager::RemoveNodeFromGrid( PathNode *node )
{
	int x;
	int y;
	
	assert( node );
	
	x = node->gridX;
	y = node->gridY;
	
	RemoveFromGrid( node, x, y );
	RemoveFromGrid( node, x + 1, y );
	RemoveFromGrid( node, x, y + 1 );
	RemoveFromGrid( node, x + 1, y + 1 );
}

void PathManager::UpdateNode( PathNode *node )
{
	int x;
	int y;
	int mx;
	int my;
	
	assert( node );
	
	x = NodeCoordinate( node->origin[ 0 ] );
	y = NodeCoordinate( node->origin[ 1 ] );
	
	mx = node->gridX;
	my = node->gridY;
	
	RemoveFromGrid( node, mx, my );
	RemoveFromGrid( node, mx + 1, my );
	RemoveFromGrid( node, mx, my + 1 );
	RemoveFromGrid( node, mx + 1, my + 1 );
	
	AddToGrid( node, x, y );
	AddToGrid( node, x + 1, y );
	AddToGrid( node, x,	y + 1 );
	AddToGrid( node, x + 1, y + 1 );
	
	node->gridX = x;
	node->gridY = y;
}

MapCell *PathManager::GetMapCell( int x, int y )
{
	if ( _mapCells[x][y] == NULL )
	{
		_mapCells[x][y] = new MapCell();
	}
	return _mapCells[x][y];
}

MapCell *PathManager::GetMapCell( const Vector &pos )
{
	int x;
	int y;
	
	x = GridCoordinate( pos[ 0 ] );
	y = GridCoordinate( pos[ 1 ] );
	
	return GetMapCell( x, y );
}

PathNode *PathManager::NearestNode( const Vector &pos, Entity *ent, const bool usebbox, const bool checkWalk )
{
	Vector	delta;
	PathNode	*node;
	PathNode	*bestnode;
	float		bestdist;
	float		dist;
	int		n;
	int		i;
	MapCell	*cell;
	Vector	min;
	Vector	max;
	
	cell = GetMapCell( pos );
	if ( !cell )
	{
		return NULL;
	}
	
	if ( ent && usebbox )
	{
		min = ent->mins;
		max = ent->maxs;
	}
	else
	{
		min = Vector( -16.0f, -16.0f, 12.0f );
		max = Vector( 16.0f, 16.0f, 40.0f );
	}
	
	n = cell->NumNodes();
	
	if ( ai_debugpath->integer )
	{
		gi.DPrintf( "NearestNode: Checking %d nodes\n", n );
	}
	
	bestnode = NULL;
	bestdist = 999999999; // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
	for( i = 0; i < n; i++ )
	{
		node = ( PathNode * )cell->GetNode( i );
		if ( !node )
		{
			continue;
		}
		
		if ( node->NumberOfConnections() == 0 )
			continue;
		
		delta = node->origin - pos;
		
		// get the distance squared (faster than getting real distance)
		dist = delta * delta;
		
		if ( abs( (int)delta[ 2 ] ) > 128 )
			continue;
		
		if ( dist > 256 * 256 )
			continue;
		
		if ( ( dist < bestdist ) )// && node->CheckMove( ent, pos, min, max, false, false ) )
		{
			if ( checkWalk && !node->CheckMove( ent, pos, min, max, false, false ) )
				continue;
			
			bestnode = node;
			bestdist = dist;
			
			// if we're close enough, early exit
			if ( dist < 16.0f )
            {
				break;
            }
		}
	}
	
	return bestnode;
}

void PathManager::Teleport( const Entity *teleportee, const Vector &from, const Vector &to )
{
	PathNode	*node1;
	PathNode	*node2;
	byte maxheight[ NUM_WIDTH_VALUES ];
	int j;

	Q_UNUSED(teleportee);
	
	if ( ai_createnodes->integer )
	{
		node1 = new PathNode;
		node1->Setup( from );
		
		node2 = new PathNode;
		node2->Setup( to );
		
		// FIXME
		// shouldn't hard-code width and height
		for( j = 0; j < NUM_WIDTH_VALUES; j++ )
		{
			maxheight[ j ] = 72;
		}
		
		// connect with 0 cost
		node1->ConnectTo( node2, maxheight, 0.0f );
	}
}

void PathManager::ShowNodes( void )
{
	if ( ai_showroutes->integer || ai_shownodenums->integer )
	{
		DrawAllConnections();
	}
}

int PathManager::NumNodes( void )
{
	return _pathNodes.NumObjects();
}

int PathManager::NumLoadNodes( void )
{
	return numLoadNodes;
}

void PathManager::Checksum( Vector &checksum )
{
	checksum = pathNodesChecksum;
}

void PathManager::Archive( Archiver &arc )
{
	int i;
	
	if ( arc.Saving() )
	{
		int num = _pathNodes.NumObjects();
		arc.ArchiveInteger( &num );
		for( i = 0; i < _pathNodes.NumObjects(); i++ )
		{
			arc.ArchiveObject( _pathNodes[ i ] );
		}
		
		if ( ai_debuginfo->integer )
		{
			gi.DPrintf( "Wrote %d path nodes\n", num );
		}
	}
	else
	{
		int x;
		int y;
		
		_loadingArchive = true;
		
		// Get rid of the nodes that were spawned by the map
		thePathManager.ResetNodes();
		CancelEventsOfType( EV_AI_CalcAllPaths );
		
		// Init the grid
		for( x = 0; x < PATHMAP_GRIDSIZE; x++ )
		{
			for( y = 0; y < PATHMAP_GRIDSIZE; y++ )
			{
				GetMapCell( x, y )->Init();
			}
		}
		int num;
		arc.ArchiveInteger( &num );
		
		for( i = 0; i < num; i++ )
		{
			arc.ReadObject();
		}
		
		if ( ai_debuginfo && ai_debuginfo->integer )
		{
			gi.DPrintf( "Path nodes loaded: %d\n", NumNodes() );
		}
		
		_loadingArchive = false;
	}
}

void PathManager::ClearNodes( Event * )
{
	for( int i = 0; i < _pathNodes.NumObjects(); i++ )
	{
		_pathNodes[ i ]->PostEvent( EV_Remove, 0.0f );
	}
	
	if ( ai_debuginfo->integer )
	{
		gi.DPrintf( "Deleted %d path nodes\n", _pathNodes.NumObjects() );
	}
}

void PathManager::SetNodeFlagsEvent( Event *ev )
{
	const char * token;
	int i, argnum;
	int mask;
	int action;
	int nodenum;
	PathNode *node;
	
	nodenum = ev->GetInteger( 1 );
	node = thePathManager.GetNode( nodenum );
	
	if ( !node )
	{
		ev->Error( "Node not found." );
		return;
	}
	
	argnum = 2;
	for ( i = argnum; i <= ev->NumArgs() ; i++ )
	{
		token = ev->GetString( i );
		action = 0;
		switch( token[0] )
		{
		case '+':
            action = FLAG_ADD;
            token++;
            break;
		case '-':
            action = FLAG_CLEAR;
            token++;
            break;
		default:
            ev->Error( "PathManager::SetNodeFlagsEvent", "First character is not '+' or '-', assuming '+'\n" );
            action = FLAG_ADD;
            break;
		}
		
		if (!strcmpi( token, "flee"))
		{
			mask = AI_FLEE;
		}
		else if (!strcmpi (token, "duck"))
		{
			mask = AI_DUCK;
		}
		else if (!strcmpi (token, "cover"))
		{
			mask = AI_COVER;
		}
		else if (!strcmpi (token, "door"))
		{
			mask = AI_DOOR;
		}
		else if (!strcmpi (token, "jump"))
		{
			mask = AI_JUMP;
		}
		else if (!strcmpi (token, "ladder"))
		{
			mask = AI_LADDER;
		}
		else if (!strcmpi (token, "action"))
		{
			mask = AI_ACTION;
		}
		else if (!strcmpi (token, "work"))
		{
			mask = AI_WORK;
		}
		else if (!strcmpi (token, "hibernate"))
		{
			mask = AI_HIBERNATE;
		}
		else
		{
			mask = 0;
			action = FLAG_IGNORE;
			ev->Error( "Unknown token %s.", token );
		}
		
		switch (action)
		{
		case FLAG_ADD:
            node->nodeflags |= mask;
            break;
			
		case FLAG_CLEAR:
            node->nodeflags &= ~mask;
            break;
			
		case FLAG_IGNORE:
            break;
		}
	}
}

void PathManager::CalcPathEvent( Event *ev )
{
	int nodenum;
	PathNode *node;
	PathNode *node2;
	int j;
	byte maxheight[ NUM_WIDTH_VALUES ];
	
	nodenum = ev->GetInteger( 1 );
	node = thePathManager.GetNode( nodenum );
	
	nodenum = ev->GetInteger( 2 );
	node2 = thePathManager.GetNode( nodenum );
	
	if ( !node || !node2 )
	{
		ev->Error( "Node not found." );
		return;
	}
	
	if ( !node->ConnectedTo( node2 ) )
	{
		if ( node->ClearPathTo( node2, maxheight, false ) || node->LadderTo( node2, maxheight ) )
		{
			node->ConnectTo( node2, maxheight );
		}
		else if ( ( node->nodeflags & AI_JUMP ) && ( node->target == node2->targetname ) )
		{
			//FIXME
			// don't hardcode size
			for( j = 0; j < NUM_WIDTH_VALUES; j++ )
			{
				maxheight[ j ] = MAX_HEIGHT;
			}
			node->ConnectTo( node2, maxheight );
		}
	}
	
	if ( !node2->ConnectedTo( node ) )
	{
		if ( node2->ClearPathTo( node, maxheight, false ) || node2->LadderTo( node, maxheight ) )
		{
			node2->ConnectTo( node, maxheight );
		}
		else if ( ( node2->nodeflags & AI_JUMP ) && ( node2->target == node->targetname ) )
		{
			//FIXME
			// don't hardcode size
			for( j = 0; j < NUM_WIDTH_VALUES; j++ )
			{
				maxheight[ j ] = MAX_HEIGHT;
			}
			node2->ConnectTo( node, maxheight );
		}
	}
}

void PathManager::RecalcPathsEvent( Event *ev )
{
	int nodenum;
	PathNode *node;
	
	nodenum = ev->GetInteger( 1 );
	node = thePathManager.GetNode( nodenum );
	if ( node )
	{
		UpdateNode( node );
	}
	else
	{
		ev->Error( "Node not found." );
	}
}

qboolean PathManager::CanDropPath( const PathNode *node, const PathNode *node2, const PathNodeConnection *path )
{
	int i, j, k;
	PathNode *inbetween_node;
	PathNode *temp_node;
	float dist;
	float test_dist;
	Vector delta;
	qboolean found_better_path;
	
	// Calculate dist
	
	delta = node2->origin - node->origin;
	dist = delta.length();
	
	// Try to find an inbetween node
	
	for( i = 0 ; i < node->NumberOfConnections() ; i++ )
	{
		inbetween_node = thePathManager.GetNode( node->_connections[ i ]->targetNodeIndex );
		
		for( j = 0 ; j < inbetween_node->NumberOfConnections() ; j++ )
		{
			temp_node = thePathManager.GetNode( inbetween_node->_connections[ j ]->targetNodeIndex );
			
			if ( temp_node == node2 )
			{
				found_better_path = true;
				
				// Calculate distance through inbetween node
				
				delta = node2->origin - inbetween_node->origin;
				test_dist = delta.length();
				
				delta = inbetween_node->origin - node->origin;
				test_dist += delta.length();
				
				// Make sure not too much longer
				
				if ( test_dist > ( dist * 1.25f ) )
					found_better_path = false;
				
				for( k = 0 ; k < NUM_WIDTH_VALUES ; k++ )
				{
					// Make sure path is at least as wide
					
					if ( ( path->maxheight[ k ] > node->_connections[ i ]->maxheight[ k ] ) || 
						( path->maxheight[ k ] > inbetween_node->_connections[ j ]->maxheight[ k ] ) )
					{
						found_better_path = false;
						break;
					}
				}
				
				// If we found a better path, we can drop this one
				
				if ( found_better_path )
					return true;
			}
		}
	}
	
	// We didn't find a better path
	
	return false;
}

void PathManager::OptimizeNodes( Event * )
{
	
	gi.ProcessLoadingScreen( "$$OptimizingPathnodeConnections$$" );
	
	for( int i = 0; i < _pathNodes.NumObjects(); i++ )
	{
		if ( _pathNodes[ i ] )
		{
			PathNode *node = _pathNodes[ i ];
			
			// See if we can drop any paths between this node and any of its children
			for( int j = node->NumberOfConnections() - 1; j >= 0  ; j-- )
			{
				PathNode *node2 = thePathManager.GetNode( node->_connections[ j ]->targetNodeIndex );
				if ( CanDropPath( node, node2, node->_connections[ j ] ) )
				{
					node->RemoveConnection( j );
				}
			}
		}
	}
	
	// Make sure nodes are saved
	
	_pathNodesCalculated = true;
	
	Com_Printf( "\nOptimized path nodes\n" );
}

qboolean PathManager::ArchiveNodes( const str &name, qboolean save )
{
	Archiver arc;
	qboolean success;
	
	if ( save )
	{
		Vector   checksum;
		int      tempInt;
		
		if ( !arc.Create( name, false ) )
		{
			gi.WDPrintf( "***********************************\n" );
			gi.WDPrintf( "****** Couldn't write out pathfile\n" );
			gi.WDPrintf( "***********************************\n" );
			return false;
		}
		
		tempInt = PATHFILE_VERSION;
		arc.ArchiveInteger( &tempInt );
		tempInt = NumNodes();
		arc.ArchiveInteger( &tempInt );
		Checksum( checksum );
		arc.ArchiveVector( &checksum );
		arc.ArchiveObject( this );
		success = true;
	}
	else
	{
		int		version;
		
		success = false;
		arc.Read( name, false );
		arc.ArchiveInteger( &version );
		if ( version == PATHFILE_VERSION )
		{
			int numnodes, file_numnodes;
			Vector checksum, file_checksum;
			
			// get current values
			numnodes = NumLoadNodes();
			Checksum( checksum );
			
			// get file values
			arc.ArchiveInteger( &file_numnodes );
			arc.ArchiveVector( &file_checksum );
			if (
				( numnodes == file_numnodes ) &&
				( checksum == file_checksum )
				)
            {
				arc.ArchiveObject( this );
				if ( arc.NoErrors() )
				{
					success = true;
				}
            }
			else
            {
				gi.Printf( "Pathnodes have changed, rebuilding.\n" );
            }
		}
		else
		{
			gi.Printf( "Expecting version %d path file.  Path file is version %d. ", PATHFILE_VERSION, version );
		}
		
	}
	arc.Close();
	return success;
}

void PathManager::SaveNodes( Event *ev )
{
	str name;
	
	if ( ev->NumArgs() != 1 )
	{
		gi.WPrintf( "Usage: ai_save [filename]\n" );
		return;
	}
	
	name = ev->GetString( 1 );
	
	gi.Printf( "Archiving\n" );
	
	ArchiveNodes( name, true );
	
	gi.Printf( "done.\n" );
	_pathNodesCalculated = false;
}

void PathManager::LoadNodes( Event *ev )
{
	Archiver arc;
	str		name;
	bool     rebuild;
	
	if ( ev->NumArgs() != 1 )
	{
		gi.WPrintf( "Usage: ai_load [filename]\n" );
		return;
	}
	
	gi.Printf( "Loading nodes...\n" );
	
	name = ev->GetString( 1 );
	
	rebuild = !ArchiveNodes( name, false );
	
	if ( rebuild )
	{
		// Only replace the file if this event was called from our init function (as opposed to the user
		// calling us from the console)
		if ( ( ev->GetSource() == EV_FROM_CODE ) )
		{
			gi.Printf( "Replacing file.\n\n" );
			
			// At this point, the nodes are still scheduled to find their neighbors, because we posted this event
			// before we the nodes were spawned.  Post the event with 0 delay so that it gets processed after all
			// the nodes find their neighbors.
			PostEvent( EV_AI_SavePaths, 0.0f );
		}
		else
		{
			// otherwise, just let them know that the path file needs to be replaced.
			gi.Printf( "Type 'ai_savepaths' at the console to replace the current path file.\n" );
		}
		
		// Print out something fairly obvious
		gi.DPrintf( "***********************************\n"
			"***********************************\n"
			"\n"
			"Creating paths...\n"
			"\n"
			"***********************************\n"
			"***********************************\n" );
	}
}

void PathManager::SavePaths( void )
{
	str filename;
	Event *ev;
	
	if ( IsLoadingArchive() )
	{
		// force it to zero since we probably had an error
		gi.cvar_set( "ai_createnodes", "0" );
	}
	
	if (
		!IsLoadingArchive() &&
		(
		( ai_createnodes && ai_createnodes->integer ) ||
		( _pathNodesCalculated )
		)
		)
	{
		filename = "maps/";
		filename += level.mapname;
		filename += ".pth";
		
		gi.DPrintf( "\nSaving path nodes to '%s'\n", filename.c_str() );
		
		ev = new Event( EV_AI_SaveNodes );
		ev->AddString( filename );
		ProcessEvent( ev );
	}
}

void PathManager::SavePathsEvent( Event * )
{
	str temp;
	
	temp = ai_createnodes->string;
	gi.cvar_set( "ai_createnodes", "1" );
	
	SavePaths();
	
	gi.cvar_set( "ai_createnodes", temp.c_str() );
}

void PathManager::Init( const char *mapname )
{
	int x;
	int y;
	str filename;
	Event *ev;
	
	pathNodesChecksum = vec_zero;
	_pathNodesCalculated = false;
	
	gi.AddCommand( "ai_savepaths" );
	gi.AddCommand( "ai_save" );
	gi.AddCommand( "ai_load" );
	gi.AddCommand( "ai_clearnodes" );
	gi.AddCommand( "ai_recalcpaths" );

	gi.AddCommand( "ai_connectNodes" );
	gi.AddCommand( "ai_disconnectNodes" );

	ai_createnodes		= gi.cvar ("ai_createnodes", "0", 0);
	ai_debugpath		= gi.cvar ("ai_debugpath", "0", 0);
	ai_debuginfo		= gi.cvar ("ai_debuginfo", "0", 0);
	ai_showroutes		= gi.cvar ("ai_showroutes", "0", 0);
	ai_showroutes_distance = gi.cvar ("ai_showroutes_distance", "1000", 0);
	ai_shownodenums   = gi.cvar ("ai_shownodenums", "0", 0);
	ai_timepaths      = gi.cvar ("ai_timepaths", "0", 0);
	ai_advanced_pathfinding = gi.cvar ("ai_advanced_pathfinding", "1", 0);
	
	numLoadNodes = 0;
	_loadingArchive = false;
	
	// Get rid of the nodes that were spawned by the map
	thePathManager.ResetNodes();
	
	// Init the grid
	for( x = 0; x < PATHMAP_GRIDSIZE; x++ )
	{
		for( y = 0; y < PATHMAP_GRIDSIZE; y++ )
		{
			GetMapCell( x, y )->Init();
		}
	}
	
	if ( LoadingSavegame )
	{
		// no need to go further here
		return;
	}
	
	if ( mapname )
	{
		filename = "maps/";
		filename += mapname;
		filename += ".pth";
		if ( gi.FS_ReadFile( filename.c_str(), NULL, true ) != -1 )
		{
			ev = new Event( EV_AI_LoadNodes );
			ev->AddString( filename );
			
			// This can't happen until the world is spawned
			PostEvent( ev, 0.0f );
		}
		else
		{
			// Print out something fairly obvious
			gi.DPrintf( "***********************************\n"
				"***********************************\n"
				"\n"
				"No paths found.  Creating paths...\n"
				"\n"
				"***********************************\n"
				"***********************************\n" );
		}
	}
}

void PathManager::FindAllTargets( void )
{
	for( int i = 0; i < _pathNodes.NumObjects(); i++ )
	{
		PathNode *node = _pathNodes[ i ];
		
		for ( int j = 0; j < MAX_GENTITIES; j++ )
		{
			gentity_t *ed = &g_entities[ j ];
			
			if ( !ed->inuse || !ed->entity )
				continue;			      
			
			Entity *entity = g_entities[ j ].entity;
			
			if ( entity->targetname == node->target )
				node->targetEntity = entity;
		}
	}
}

void PathManager::CalcAllPaths( Event * )
{
	for( int i = 0; i < _pathNodes.NumObjects(); i++ )
	{
		PathNode *node = _pathNodes[ i ];
		
		if ( node )
		{
			str loadingStatus;
			loadingStatus = "$$GeneratingPathnodeConnections$$";
			loadingStatus += i;
			loadingStatus += " of ";
			loadingStatus += _pathNodes.NumObjects();
			gi.ProcessLoadingScreen( loadingStatus.c_str() );
			
			node->ProcessEvent( EV_Path_FindChildren );
		}
	}
}

int	PathManager::NumberOfSpecialNodes( void ) const
{
	return _specialPathNodes.NumObjects();
}

PathNode * PathManager::GetSpecialNode( const int index ) const
{
	return (_specialPathNodes.ObjectAt( index ) );
}

void PathManager::AddSpecialNode( PathNode &pathNode )
{
	_specialPathNodes.AddObject( &pathNode );
}

void PathManager::connectNodes( Event *ev )
{
	connectNodes( ev->GetString( 1 ), ev->GetString( 2 ) );
}

void PathManager::connectNodes( const str &nodeName1, const str &nodeName2 )
{
	PathNode *node1;
	PathNode *node2;

	node1 = FindNode( nodeName1 );
	node2 = FindNode( nodeName2 );

	connectConnection( node1, node2 );
	connectConnection( node2, node1 );
}

void PathManager::connectConnection( PathNode *node1, PathNode *node2 )
{
	byte maxheight[ NUM_WIDTH_VALUES ];

	if ( !node1 || !node2 )
		return;

	if ( node1 == node2 )
		return;
			
	if ( !node1->ConnectedTo( node2 ) )
	{
		if ( node1->ClearPathTo( node2, maxheight ) || node1->LadderTo( node2, maxheight ) )
		{
			node1->ConnectTo( node2, maxheight );
		}
	}
}

void PathManager::disconnectNodes( Event *ev )
{
	disconnectNodes( ev->GetString( 1 ), ev->GetString( 2 ) );
}

void PathManager::disconnectNodes( const str &nodeName1, const str &nodeName2 )
{
	PathNode *node1;
	PathNode *node2;

	node1 = FindNode( nodeName1 );
	node2 = FindNode( nodeName2 );

	disconnectConnection( node1, node2 );
	disconnectConnection( node2, node1 );
}

void PathManager::disconnectConnection( PathNode *node1, PathNode *node2 )
{
	int i;
	PathNodeConnection *connection;
	PathNode *nodeToCheck;

	if ( !node1 || !node2 )
		return;
			
	for( i = 0 ; i < node1->NumberOfConnections() ; i++ )
	{
		connection = node1->_connections[ i ];

		if ( _pathNodes[ connection->targetNodeIndex ] )
		{
			nodeToCheck = _pathNodes[ connection->targetNodeIndex ];

			if ( nodeToCheck == node2 )
			{
				node1->RemoveConnection( i );
				return;
			}
		}
	}
}
