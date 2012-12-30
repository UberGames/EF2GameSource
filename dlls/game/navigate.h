//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/navigate.h                                    $
// $Revision:: 40                                                             $
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
// Potentially could be an C++ implementation of the A* search algorithm, but
// is currently unfinished.
//

#ifndef __NAVIGATE_H__
#define __NAVIGATE_H__

#include "g_local.h"
#include "class.h"
#include "entity.h"
#include "stack.h"
#include "container.h"
#include "doors.h"
#include <qcommon/qfiles.h>


extern Event EV_AI_SavePaths;
extern Event EV_AI_SaveNodes;
extern Event EV_AI_LoadNodes;
extern Event EV_AI_ClearNodes;
extern Event EV_AI_RecalcPaths;
extern Event EV_AI_CalcPath;

extern cvar_t	*ai_createnodes;
extern cvar_t	*ai_shownodes;
extern cvar_t	*ai_debugpath;
extern cvar_t	*ai_debuginfo;
extern cvar_t	*ai_showroutes;
extern cvar_t  *ai_timepaths;
extern cvar_t	*ai_advanced_pathfinding;

extern int		ai_maxnode;

#define MAX_PATHCHECKSPERFRAME 4

extern int path_checksthisframe;

#define MAX_PATH_LENGTH		512		// should be more than plenty
#define NUM_PATHSPERNODE	32

class Path;
class PathNode;

#define NUM_WIDTH_VALUES	16
#define WIDTH_STEP			8
#define MAX_WIDTH				( WIDTH_STEP * NUM_WIDTH_VALUES )
#define MAX_HEIGHT			128

#define CHECK_PATH( path, width, height ) \
	( ( ( ( width ) >= MAX_WIDTH ) || ( ( width ) < 0 ) ) ? false : \
	( ( int )( path )->maxheight[ ( ( width ) / WIDTH_STEP ) - 1 ] < ( int )( height ) ) )

class PathNodeConnection : public Class
{
public:
	CLASS_PROTOTYPE( PathNodeConnection );
	int					targetNodeIndex;
	short				moveCost;
	byte				maxheight[ NUM_WIDTH_VALUES ];
	int					door;

	virtual void		Archive( Archiver &arc )
	{
		arc.ArchiveInteger( &targetNodeIndex );
		arc.ArchiveShort( &moveCost );
		arc.ArchiveRaw( maxheight, sizeof( maxheight ) );
		arc.ArchiveInteger( &door );
	}
};


typedef enum { NOT_IN_LIST, IN_OPEN, IN_CLOSED } pathlist_t;

#define AI_FLEE		(1<<0)
#define AI_DUCK		(1<<1)
#define AI_COVER		(1<<2)
#define AI_DOOR		(1<<3)
#define AI_JUMP		(1<<4)
#define AI_LADDER		(1<<5)
#define AI_ACTION    (1<<6)
#define AI_WORK      (1<<7)
#define AI_HIBERNATE (1<<8)


class PathNode : public Listener
	{
	public:
		Container<PathNodeConnection *>
								_connections;	// these are the real connections between nodex

		// These variables are all used during the search
		int				f;
		int				h;
		int				g;

		int				gridX;
		int				gridY;

		float				drawtime;
		float				occupiedTime;
		int				entnum;

		pathlist_t		inlist;

		// reject is used to indicate that a node is unfit for ending on during a search
		qboolean			reject;

		PathNode			*Parent;

		// For the open and closed lists
		PathNode			*NextNode;

		unsigned int	nodeflags;
		float			jumpAngle;

		EntityPtr		targetEntity;

		friend class   PathSearch;
		friend void		DrawAllConnections( void );
		qboolean			ConnectedTo( PathNode *node );
		void				ConnectTo( PathNode *node, const byte maxheight[ NUM_WIDTH_VALUES ], float cost, const Door *door = NULL );
		void				ConnectTo( PathNode *node, const byte maxheight[ NUM_WIDTH_VALUES ] );

	private :
      qboolean       TestMove( Entity *ent, const Vector &start, const Vector &end, const Vector &min, const Vector &max, qboolean allowdoors = false, qboolean fulltest = false );


		void				FindChildren( Event *ev );
      void           FindEntities( Event *ev );
      void           SetNodeFlags( Event *ev );
      void           SetOriginEvent( Event *ev );
      void           SetAngle( Event *ev );
      void           SetAngles( Event *ev );
      void           SetAnim( Event *ev );
      void           SetTargetname( Event *ev );
      void           SetJumpAngle( Event *ev );
      void           SetTarget( Event *ev );

	public:
      CLASS_PROTOTYPE( PathNode );

      int            contents;
      Vector         origin;
      Vector         angles;
		Vector			mins;
		Vector			maxs;
		str				targetname;
		str				target;
		qboolean			setangles;
		str				animname;

							PathNode();
							~PathNode();

		void				Setup( const Vector &pos );
		void				setAngles( const Vector &ang );
		void				setOrigin( const Vector &org );
		void				setSize( const Vector &min, const Vector &max );
		str				&TargetName( void );
		virtual void	Archive( Archiver &arc );

		qboolean			CheckPath( const PathNode *node, const Vector &min, const Vector &max, qboolean fulltest = true );
		Door				*CheckDoor( const Vector &pos );

      qboolean       CheckMove( Entity *ent, const Vector &pos, const Vector &min, const Vector &max, qboolean allowdoors = false, qboolean fulltest = false );
		qboolean			CheckMove( const Vector &pos, const Vector &min, const Vector &max );
		qboolean			ClearPathTo( PathNode *node, byte maxheight[ NUM_WIDTH_VALUES ], qboolean fulltest = true );
		qboolean			LadderTo( const PathNode *node, byte maxheight[ NUM_WIDTH_VALUES ] );
		void				DrawConnections( void );
		friend bool				operator==( const PathNode &node1, const PathNode &node2 ) { return (node1.origin == node2.origin) != 0; }
		friend bool				operator!=( const PathNode &node1, const PathNode &node2 ) { return !( node1 == node2 ); }
		int						NumberOfConnections( void ) const { return _connections.NumObjects(); }
		PathNodeConnection &	GetConnection( const int index ) const { return *_connections[index]; }
		void					AddConnection( PathNodeConnection &connection ) { _connections.AddObject( &connection ); }
		void					RemoveConnection( const int index ) { delete _connections[index]; _connections.RemoveObjectAt( index + 1 ); }
	};

typedef SafePtr<PathNode> PathNodePtr;

//#define PATHMAP_CELLSIZE	2048		// Was 256
#define PATHMAP_CELLSIZE	1024		// Was 256
#define PATHMAP_MAX_DIST	256.0f
#define PATHMAP_GRIDSIZE	( WORLD_SIZE / PATHMAP_CELLSIZE )	// 65536 / 2048 = 32 was 8192 * 2 / 256 = 64

//#define PATHMAP_NODES		126			// 128 - sizeof( int ) / sizeof( short )
#define PATHMAP_NODES		( 512 - sizeof( int ) / sizeof( short ) )

class MapCell : public Class
	{
	private :
		Container<PathNode *>	_pathNodes;
		//int	numnodes;
		//short	nodes[ PATHMAP_NODES ];

	public :
								MapCell();
								~MapCell();
		void					Init( void );
		qboolean				AddNode( PathNode *node );
		qboolean				RemoveNode( PathNode *node );
		PathNode				*GetNode( int index );
		int					NumNodes( void );
	};

class PathManager : public Listener
{
public:
							CLASS_PROTOTYPE( PathManager );

							PathManager();
							~PathManager();
	void					Archive( Archiver &arc );
	void					AddToGrid( PathNode *node, int x, int y );
	void					AddNode( PathNode *node );
	void					AddNodeToGrid( PathNode *node );
	qboolean				RemoveFromGrid( PathNode *node, int x, int y );
	void					RemoveNodeFromGrid( PathNode *node );
	void					RemoveNode( PathNode *node );
	void					UpdateNode( PathNode *node );
	MapCell *				LookupMapCell( const int x, const int y ) const;
	MapCell *				GetMapCell( const int x, const int y ) ;
	MapCell	*				GetMapCell( const Vector &pos );
	PathNode *				NearestNode( const Vector &pos, Entity *ent = NULL, const bool usebbox = true, const bool checkWalk = true );
	void					Teleport( const Entity *teleportee,	const Vector &from,	const Vector &to );
	void					ShowNodes( void );
	void					Checksum( Vector &checksum );
	void					SavePaths( void );
	void					Init( const char *mapname );
	void					InsertNodeIntoGrid( PathNode &node );
	void					RemoveNodeFromGrid( PathNode &node );
	void					InsertNodesIntoGrid( void );
	void					ConnectToNeighbors( PathNode &node );
//	void					ConnectPathNodes( void );
	void					FindAllTargets( void );
	PathNode *				FindNode( const char *name );
	PathNode *				GetNode( int num );
	void					RemoveNode( const PathNode *node );
	void					ResetNodes( void );
	int						NumNodes( void );
	int						NumLoadNodes( void );
	bool					IsLoadingArchive( void ) const { return _loadingArchive; }
	void					SetPathNodesCalculated( const bool pathNodesCalculated ) { _pathNodesCalculated = pathNodesCalculated; }
	bool					GetPathNodesCalculated( void ) const { return _pathNodesCalculated; }
	int						GetPathNodeIndex( PathNode &pathNode ) const { const int index = _pathNodes.IndexOfObject( &pathNode ) - 1 ; assert( index >= 0 ); return index;}
	int						NumberOfSpecialNodes( void ) const;
	PathNode *				GetSpecialNode( const int index ) const;
	void					AddSpecialNode( PathNode &pathNode );
	void					OptimizeNodes( Event *ev );
	void					DrawAllConnections( void );

	void					connectNodes( Event *ev );
	void					connectNodes( const str &nodeName1, const str &nodeName2 );
	void					connectConnection( PathNode *node1, PathNode *node2 );

	void					disconnectNodes( Event *ev );
	void					disconnectNodes( const str &nodeName1, const str &nodeName2 );
	void					disconnectConnection( PathNode *node1, PathNode *node2 );

private:
	void					ConnectToNeighbor( PathNode &node, const int x, const int y );
	bool					RemoveFromGrid( PathNode &node, const int x, const int y );
	int						NodeCoordinate( float coord );
	int						GridCoordinate( float coord );
	void					ClearNodes( Event *ev );
	void					LoadNodes( Event *ev );
	void					SaveNodes( Event *ev );
	qboolean				CanDropPath( const PathNode *node, const PathNode *node2, const PathNodeConnection *path );
	qboolean				ArchiveNodes( const str &name, qboolean save );
	void					SavePathsEvent( Event *ev );
	void					SetNodeFlagsEvent( Event *ev );
	void					RecalcPathsEvent( Event *ev );
	void					CalcPathEvent( Event *ev );
	void					CalcAllPaths( Event *ev );

	Container<PathNode *>	_pathNodes;
	Container<PathNode *>	_specialPathNodes;
	bool					_loadingArchive;
	bool					_pathNodesCalculated;

	MapCell *				_mapCells[PATHMAP_GRIDSIZE][PATHMAP_GRIDSIZE];
};

extern PathManager thePathManager;

//#define MAX_PATHNODES 2048

#include "path.h"

template<class Heuristic>
class PathFinder : public Class
	{
	private:
	   Stack<PathNode *>	stack;
	   PathNode				*OPEN;
	   PathNode				*CLOSED;
		PathNode				*endnode;
		int						_maxNodesToCheck;
		int						_checkedNodes;

		void					ClearPath( void );
		void					ClearOPEN( void );
		void					ClearCLOSED( void );
		PathNode				*ReturnBestNode( void );
		void					GenerateSuccessors( PathNode *BestNode );
		void					Insert( PathNode *Successor );
		void					PropagateDown( PathNode *Old );
		Path					*CreatePath( PathNode *startnode );

	public:
		Heuristic			heuristic;

								PathFinder();
								~PathFinder<Heuristic>();
		Path					*FindPath( PathNode *from, PathNode *to, int maxNodesToCheck = 0 );
		Path					*FindPath( const Vector &start, const Vector &end, int maxNodesToCheck = 0 );
	};

template <class Heuristic>
PathFinder<Heuristic>::PathFinder()
	{
	OPEN   = NULL;
	CLOSED = NULL;
	}

template <class Heuristic>
PathFinder<Heuristic>::~PathFinder()
	{
	ClearPath();
	}

template <class Heuristic>
void PathFinder<Heuristic>::ClearOPEN
	(
	void
	)

	{
	PathNode *node;

	while( OPEN )
		{
		node = OPEN;
		OPEN = node->NextNode;

		node->inlist = NOT_IN_LIST;
		node->NextNode = NULL;
		node->Parent = NULL;

		// reject is used to indicate that a node is unfit for ending on during a search
		node->reject = false;
		}
	}

template <class Heuristic>
void PathFinder<Heuristic>::ClearCLOSED
	(
	void
	)

	{
	PathNode *node;

	while( CLOSED )
		{
		node = CLOSED;
		CLOSED = node->NextNode;

		node->inlist = NOT_IN_LIST;
		node->NextNode = NULL;
		node->Parent = NULL;

		// reject is used to indicate that a node is unfit for ending on during a search
		node->reject = false;
		}
	}

template <class Heuristic>
void PathFinder<Heuristic>::ClearPath
	(
	void
	)

	{
	stack.Clear();
	ClearOPEN();
	ClearCLOSED();
	}

template <class Heuristic>
Path *PathFinder<Heuristic>::FindPath
	(
	PathNode *from,
	PathNode *to,
	int maxNodesToCheck
	)

	{
	Path		*path;
	PathNode	*node;
   int start = 0;
   int end;
   int count;
   qboolean checktime;

   checktime = false;
   if ( ai_timepaths->integer )
      {
      start = gi.Milliseconds();
      checktime = true;
      }

	OPEN   = NULL;
	CLOSED = NULL;

	endnode = to;

	// Should always be NULL at this point
	assert( !from->NextNode );

	// make Open List point to first node
	OPEN = from;
	from->g = 0;
	from->h = heuristic.dist( from, endnode );
	from->NextNode = NULL;

	_maxNodesToCheck = maxNodesToCheck;
	_checkedNodes = 0;

	node = ReturnBestNode();
   
   count = 0;
	while( node && !heuristic.done( node, endnode ) )
		{
		count++;
      GenerateSuccessors( node );
		node = ReturnBestNode();
		}

	//assert ( count < MAX_PATH_LENGTH );

	if ( !node )
		{
		path = NULL;
      if ( ai_debugpath->integer )
			{
         gi.WDPrintf( "Search failed--no path found.\n" );
			}
		}
	else
		{
		path = CreatePath( node );
		}

	ClearPath();

   if ( checktime )
      {
      end = gi.Milliseconds();
      if ( ai_timepaths->integer <= ( end - start ) )
         {
         gi.DebugPrintf( "%d: ent #%d : %d\n", level.framenum, heuristic.entnum, end - start );
         }
      }

	return path;
	}

template <class Heuristic>
Path *PathFinder<Heuristic>::FindPath
	(
	const Vector &start,
	const Vector &end,
	int maxNodesToCheck
	)

	{
	PathNode *from;
	PathNode *to;
   Entity *ent;

   ent = G_GetEntity( heuristic.entnum );
	from = thePathManager.NearestNode( start, ent );
	to = thePathManager.NearestNode( end, ent );

	if ( !from )
		{
      if ( ai_debugpath->integer )
			{
         gi.WDPrintf( "Search failed--couldn't find closest source.\n" );
			}
		return NULL;
		}

	if ( !from || !to )
		{
      if ( ai_debugpath->integer )
			{
         gi.WDPrintf( "Search failed--couldn't find closest destination.\n" );
			}
		return NULL;
		}

	return FindPath( from, to, maxNodesToCheck );
	}

template <class Heuristic>
Path *PathFinder<Heuristic>::CreatePath
	(
	PathNode *startnode
	)

	{
	PathNode *node;
	Path *p;
	int	i;
	int	n;
	PathNode *reverse[ MAX_PATH_LENGTH ];

	// unfortunately, the list goes goes from end to start, so we have to reverse it
	for( node = startnode, n = 0; ( node != NULL ) && ( n < MAX_PATH_LENGTH ); node = node->Parent, n++ )
		{
		assert( n < MAX_PATH_LENGTH );
		reverse[ n ] = node;
		}

	p = new Path( n );
	for( i = n - 1; i >= 0; i-- )
		{
		p->AddNode( reverse[ i ] );
		}

   if ( ai_debugpath->integer )
		{
      gi.DPrintf( "%d nodes in path\n", n );
      gi.DPrintf( "%d nodes allocated\n", thePathManager.NumNodes() );
		}

	return p;
	}

template <class Heuristic>
PathNode *PathFinder<Heuristic>::ReturnBestNode
	(
	void
	)

	{
	PathNode *bestnode;

	if ( !OPEN || ( _maxNodesToCheck && _checkedNodes > _maxNodesToCheck ) )
		{
		// No more nodes on OPEN
		return NULL;
		}

	// Pick node with lowest f, in this case it's the first node in list
	// because we sort the OPEN list wrt lowest f. Call it BESTNODE.

	bestnode = OPEN;					// point to first node on OPEN
	OPEN = bestnode->NextNode;		// Make OPEN point to nextnode or NULL.

	// Next take BESTNODE (or temp in this case) and put it on CLOSED
	bestnode->NextNode	= CLOSED;
	CLOSED					= bestnode;

	bestnode->inlist = IN_CLOSED;

	return( bestnode );
	}

template <class Heuristic>
void PathFinder<Heuristic>::GenerateSuccessors
	(
	PathNode *BestNode
	)

	{
	int			i;
	int			g;					// total path cost - as stored in the linked lists.
	PathNode		*node;
	PathNodeConnection	*path;

	for( i = 0; i < BestNode->NumberOfConnections(); i++ )
		{
		path = &BestNode->GetConnection( i );
		node = thePathManager.GetNode( path->targetNodeIndex );

		// g(Successor)=g(BestNode)+cost of getting from BestNode to Successor
		g = BestNode->g + heuristic.cost( BestNode, i );

		switch( node->inlist )
			{
			case NOT_IN_LIST :
				// Only allow this if it's valid
				if ( heuristic.validpath( BestNode, i ) )
					{
					node->Parent	= BestNode;
					node->g			= g;
					node->h			= heuristic.dist( node, endnode );
					node->f			= g + node->h;

					// Insert Successor on OPEN list wrt f
					Insert( node );

					_checkedNodes++;
					}
				break;

			case IN_OPEN :
				// if our new g value is < node's then reset node's parent to point to BestNode
    			if ( g < node->g )
    				{
    				node->Parent	= BestNode;
    				node->g			= g;
    				node->f			= g + node->h;
    				}
				break;

			case IN_CLOSED :
				// if our new g value is < Old's then reset Old's parent to point to BestNode
    			if ( g < node->g )
    				{
    				node->Parent	= BestNode;
    				node->g			= g;
    				node->f			= g + node->h;

					// Since we changed the g value of Old, we need
    				// to propagate this new value downwards, i.e.
    				// do a Depth-First traversal of the tree!
    				PropagateDown( node );
    				}
				break;

			default :
				// shouldn't happen, but try to catch it during debugging phase
				assert( NULL );
				gi.Error( ERR_DROP, "Corrupted path node" );
				break;
			}
		}
	}

template <class Heuristic>
void PathFinder<Heuristic>::Insert
	(
	PathNode *node
	)

	{
	PathNode *prev;
	PathNode *next;
	int		f;

	node->inlist = IN_OPEN;
	f = node->f;

	// special case for if the list is empty, or it should go at head of list (lowest f)
	if ( ( OPEN == NULL ) || ( f < OPEN->f ) )
		{
		node->NextNode = OPEN;
		OPEN = node;
		return;
		}

	// do sorted insertion into OPEN list in order of ascending f
	prev = OPEN;
	next = OPEN->NextNode;
	while( ( next != NULL ) && ( next->f < f ) )
		{
		prev = next;
		next = next->NextNode;
		}

	// insert it between the two nodes
	node->NextNode = next;
	prev->NextNode = node;
	}

template <class Heuristic>
void PathFinder<Heuristic>::PropagateDown
	(
	PathNode *node
	)

	{
	int			c;
	unsigned		g;
	unsigned		movecost;
	PathNode		*child;
	PathNode		*parent;
	PathNodeConnection	*path;
	int			n;

	g = node->g;
	n = node->NumberOfConnections();
	for( c = 0; c < n; c++ )
		{
		path = &node->GetConnection( c );
		child = thePathManager.GetNode( path->targetNodeIndex );

		movecost = g + heuristic.cost( node, c );
		if ( movecost < child->g )
			{
			child->g = movecost;
			child->f = child->g + child->h;
			child->Parent = node;

			// reset parent to new path.
			// Now the _connections's branch need to be
			// checked out. Remember the new cost must be propagated down.
			stack.Push( child );
			}
		}

	while( !stack.Empty() )
		{
    	parent = stack.Pop();
		n = parent->NumberOfConnections();
		for( c = 0; c < n; c++ )
    		{
			path = &parent->GetConnection( c );
			child = thePathManager.GetNode( path->targetNodeIndex );

			// we stop the propagation when the g value of the child is equal or better than
			// the cost we're propagating
			movecost = parent->g + path->moveCost;
			if ( movecost < child->g )
    			{
				child->g = movecost;
				child->f = child->g + child->h;
    			child->Parent = parent;
    			stack.Push( child );
    			}
    		}
		}
	}

class StandardMovement : public Class
	{
	public:
		int		minwidth;
		int		minheight;
		int		entnum;
		qboolean can_jump;

	inline void setSize
		(
		const Vector &size
		)

		{
		minwidth = (int)max( size.x, size.y );
		minheight = (int)size.z;
		}

	inline int dist
		(
		const PathNode *node,
		const PathNode *end
		)

		{
		Vector	delta;
		int		d1;
		int		d2;
		int		d3;
		int		h;

      delta = node->origin - end->origin;
		d1		= abs( ( int )delta[ 0 ] );
		d2		= abs( ( int )delta[ 1 ] );
		d3		= abs( ( int )delta[ 2 ] );
		h		= max( d1, d2 );
		h		= max( d3, h );

		return h;
		}

	inline qboolean validpath
		(
		PathNode *node,
		int i
		)

		{
		PathNodeConnection *path;
		PathNode *n;

		path = &node->GetConnection( i );

		if ( minwidth < WIDTH_STEP )
		{
			minwidth = WIDTH_STEP ;
//			return false;
		}

		if ( CHECK_PATH( path, minwidth, minheight ) )
			{
			return false;
			}

		if ( path->door )
			{
			Entity *entity;
			Door *door;

			entity = G_GetEntity( path->door );

			if ( entity && entity->isSubclassOf( Door ) )
				{
				door = ( Door * )entity;
				if ( !door->CanBeOpenedBy( G_GetEntity( entnum ) ) )
					{
					return false;
					}
				}
			}

		n = thePathManager.GetNode( path->targetNodeIndex );

		if ( ( node->nodeflags & AI_JUMP ) && ( n->nodeflags & AI_JUMP ) && ( !can_jump ) )
			return false;

		if ( n && ( n->occupiedTime > level.time ) && ( n->entnum != entnum ) )
			{
			return false;
			}

		return true;
		}

	inline int cost
		(
		const PathNode *node,
		int i
		)

		{
		return node->GetConnection( i ).moveCost;
		}

	inline qboolean done
		(
		const PathNode *node,
		const PathNode *end
		)

		{
		return node == end;
		}
	};

typedef PathFinder<StandardMovement> StandardMovePath;

#endif /* navigate.h */
