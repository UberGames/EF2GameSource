//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/path.cpp                                      $
// $Revision:: 7                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/06/02 7:08p                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#include "_pch_cpp.h"
#include "entity.h"
#include "path.h"
#include "container.h"
#include "navigate.h"
#include "misc.h"

CLASS_DECLARATION( Class, Path, NULL )
{
	{ NULL, NULL }
};

Path::Path()
{
	pathlength = 0;
	from = NULL;
	to = NULL;
	nextnode = 1;
}

Path::Path( int numnodes )
{
	pathlength = 0;
	from = NULL;
	to = NULL;
	nextnode = 1;
	pathlist.Resize( numnodes );
	dirToNextNode.Resize( numnodes );
	distanceToNextNode.Resize( numnodes );
}

void Path::Clear( void )
{
	nextnode = 1;
	pathlength = 0;
	from = NULL;
	to = NULL;
	pathlist.FreeObjectList();
	dirToNextNode.FreeObjectList();
	distanceToNextNode.FreeObjectList();
}

void Path::Reset( void )
{
	nextnode = 1;
}

PathNode *Path::Start( void )
{
	return from;
}

PathNode *Path::End( void )
{
	return to;
}

void Path::AddNode( PathNode *node )
{
	Vector dir;
	float len;
	int num;
	
	if ( !from )
	{
		from = node;
	}
	
	to = node;
	pathlist.AddObject( PathNodePtr( node ) );
	
	len = 0;
	distanceToNextNode.AddObject( len );
	dirToNextNode.AddObject( vec_zero );
	
	num = NumNodes();
	if ( num > 1 )
	{
		dir = node->origin - GetNode( num - 1 )->origin;
		len = dir.length();
		dir *= 1.0f / len;
		
		distanceToNextNode.SetObjectAt( num - 1, len );
		dirToNextNode.SetObjectAt( num - 1, dir );
		
		pathlength += len;
	}
}

void Path::_UpdateNodeValues(int const nodeNumber)
{
	if (nodeNumber < 1)
	{
		return;
	}
	
	Vector dir(vec_zero);
	float len=0.0f;
	
	if (nodeNumber < NumNodes())
	{
		dir = GetNode( nodeNumber + 1 )->origin - GetNode( nodeNumber )->origin;
		len = dir.length();
		dir *= 1.0f / len;
	}
	distanceToNextNode.SetObjectAt( nodeNumber, len );
	dirToNextNode.SetObjectAt( nodeNumber, dir );
}

void Path::_UpdatePathLength(void)
{
	int numNodes=NumNodes();
	pathlength=0.0f;
	for (int i=1; i<=numNodes;i++)
	{
		pathlength+=distanceToNextNode.ObjectAt(i);
	}
}

void Path::InsertNode( PathNode *node, int const insertionPoint )
{
	if (insertionPoint == 1)
	{
		from = node;
	}
	
	if (insertionPoint > NumNodes())
	{	
		to = node;
	}
	pathlist.InsertObjectAt(insertionPoint, PathNodePtr( node ) );
	distanceToNextNode.InsertObjectAt( insertionPoint, 0.0f );
	dirToNextNode.InsertObjectAt( insertionPoint, vec_zero );
	
	_UpdateNodeValues(insertionPoint-1);
	_UpdateNodeValues(insertionPoint);
	_UpdatePathLength();
}

void Path::RemoveNode( PathNode *node )
{
	
	int nodeIndex=GetNodeIndex(node);
	int numNodes=NumNodes();
	if (nodeIndex < 1)
	{
		return;
	}
	
	pathlist.RemoveObjectAt(nodeIndex);
	if (NumNodes() == 0)
	{
		to=NULL;
		from=NULL;
	}
	else
	{
		if (nodeIndex == numNodes)
		{
			to=GetNode(NumNodes());
		}
		if (nodeIndex == 1)
		{
			from=GetNode(1);
		}
		else
		{
			_UpdateNodeValues(nodeIndex-1);
		}
	}
	_UpdatePathLength();
}

PathNode *Path::GetNode( int num )
{
	PathNode *node;
	
	node = pathlist.ObjectAt( num );
	assert( node != NULL );
	if ( node == NULL )
	{
		error( "GetNode", "Null pointer in node list\n" );
	}
	
	return node;
}

int Path::GetNodeIndex( PathNode *node )
{
	int num = NumNodes();
	for( int i = 1; i <= num; i++ )
	{
		if ( pathlist.ObjectAt( i ) == node )
		{
			return i;
		}
	}
	return -1;
}

PathNode	*Path::NextNode( void )
{
	if ( nextnode <= NumNodes() )
	{
		return pathlist.ObjectAt( nextnode++ );
	}
	return NULL;
}

PathNode	*Path::NextNode( const PathNode	*node )
{
	int i;
	int num;
	PathNode *n;
	
	num = NumNodes();
	
	// NOTE: We specifically DON'T check the last object (hence the i < num instead
	// of the usual i <= num, so don't go doing something stupid like trying to fix
	// this without keeping this in mind!! :)
	for( i = 1; i < num; i++ )
	{
		n = pathlist.ObjectAt( i );
		if ( n == node )
		{
			// Since we only check up to num - 1, it's ok to do this.
			// We do this since the last node in the list has no next node (duh!).
			return pathlist.ObjectAt( i + 1 );
		}
	}
	
	return NULL;
}

Vector Path::ClosestPointOnPath( const Vector &pos )
{
	PathNode	*s;
	PathNode	*e;
	int		num;
	int		i;
	float		bestdist;
	Vector	bestpoint;
	float		dist;
	float		segmentlength;
	Vector	delta;
	Vector	p1;
	Vector	p2;
	Vector	p3;
	float		t;
	
	num = NumNodes();
	s = GetNode( 1 );
	
	bestpoint = s->origin;
	delta = bestpoint - pos;
	bestdist = delta * delta;
	
	for( i = 2; i <= num; i++ )
	{
		e = GetNode( i );
		
		// check if we're closest to the endpoint
		delta = e->origin - pos;
		dist = delta * delta;
		
		if ( dist < bestdist )
		{
			bestdist = dist;
			bestpoint = e->origin;
		}
		
		// check if we're closest to the segment
		segmentlength = distanceToNextNode.ObjectAt( i - 1 );
		p1 = dirToNextNode.ObjectAt( i - 1 );
		p2 = pos - s->origin;
		
		t = p1 * p2;
		if ( ( t > 0.0f ) && ( t < segmentlength ) )
		{
			p3 = ( p1 * t ) + s->origin;
			
			delta = p3 - pos;
			dist = delta * delta;
			if ( dist < bestdist )
			{
				bestdist = dist;
				bestpoint = p3;
			}
		}
		
		s = e;
	}
	
	return bestpoint;
}

float Path::DistanceAlongPath( const Vector &pos )
{
	PathNode	*s;
	PathNode	*e;
	int		num;
	int		i;
	float		bestdist;
	float		dist;
	float		segmentlength;
	Vector	delta;
	Vector	p1;
	Vector	p2;
	Vector	p3;
	float		t;
	float		pathdist;
	float		bestdistalongpath;
	
	pathdist = 0;
	
	num = NumNodes();
	s = GetNode( 1 );
	delta = s->origin - pos;
	bestdist = delta * delta;
	bestdistalongpath = 0;
	
	for( i = 2; i <= num; i++ )
	{
		e = GetNode( i );
		
		segmentlength = distanceToNextNode.ObjectAt( i - 1 );
		
		// check if we're closest to the endpoint
		delta = e->origin - pos;
		dist = delta * delta;
		
		if ( dist < bestdist )
		{
			bestdist = dist;
			bestdistalongpath = pathdist + segmentlength;
		}
		
		// check if we're closest to the segment
		p1 = dirToNextNode.ObjectAt( i - 1 );
		p2 = pos - s->origin;
		
		t = p1 * p2;
		if ( ( t > 0.0f ) && ( t < segmentlength ) )
		{
			p3 = ( p1 * t ) + s->origin;
			
			delta = p3 - pos;
			dist = delta * delta;
			if ( dist < bestdist )
			{
				bestdist = dist;
				bestdistalongpath = pathdist + t;
			}
		}
		
		s = e;
		
		pathdist += segmentlength;
	}
	
	return bestdistalongpath;
}

Vector Path::PointAtDistance( float dist )
{
	PathNode	*s;
	PathNode	*e;
	int		num;
	int		i;
	float		t;
	float		pathdist;
	float		segmentlength;
	
	num = NumNodes();
	s = GetNode( 1 );
	pathdist = 0;
	
	for( i = 2; i <= num; i++ )
	{
		e = GetNode( i );
		
		segmentlength = distanceToNextNode.ObjectAt( i - 1 );
		if ( ( pathdist + segmentlength ) > dist )
		{
			t = dist - pathdist;
			return s->origin + ( dirToNextNode.ObjectAt( i - 1 ) * t );
		}
		
		s = e;
		pathdist += segmentlength;
	}
	
	// cap it off at start or end of path
	return s->origin;
}

PathNode *Path::NextNode( float dist )
{
	PathNode	*s;
	PathNode	*e;
	int		num;
	int		i;
	float		pathdist;
	float		segmentlength;
	
	num = NumNodes();
	s = GetNode( 1 );
	pathdist = 0;
	
	for( i = 2; i <= num; i++ )
	{
		e = GetNode( i );
		
		segmentlength = distanceToNextNode.ObjectAt( i - 1 );
		if ( ( pathdist + segmentlength ) > dist )
		{
			return e;
		}
		
		s = e;
		pathdist += segmentlength;
	}
	
	// cap it off at start or end of path
	return s;
}

void Path::DrawPath( float r, float g, float b, float time )
{
	Vector	s;
	Vector	e;
	Vector	offset;
	PathNode	*node;
	int		num;
	int		i;
	
	num = NumNodes();
	
	node = GetNode( 1 );
	s = node->origin;
	
	offset = Vector( r, g, b ) * 4.0f + Vector( 0.0f, 0.0f, 20.0f );
	for( i = 2; i <= num; i++ )
	{
		node = GetNode( i );
		e = node->origin;
		
		G_DebugLine( s + offset, e + offset, r, g, b, 1.0f );
		s = e;
	}
}

int Path::NumNodes( void )
{
	return pathlist.NumObjects();
}

float Path::Length( void )
{
	return pathlength;
}
