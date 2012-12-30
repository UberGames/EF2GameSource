//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/path.h                                        $
// $Revision:: 5                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
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

#ifndef __PATH_H__
#define __PATH_H__

#include "g_local.h"
#include "class.h"
#include "container.h"
#include "navigate.h"

class Path : public Class
	{
	private:
		Container<PathNodePtr>	pathlist;
      Container<float>	      distanceToNextNode;
      Container<Vector>	      dirToNextNode;
		float							pathlength;
		PathNodePtr					from;
		PathNodePtr					to;
		int							nextnode;
		void _UpdateNodeValues(int const nodeNumber);
		void _UpdatePathLength(void);

	public:
      CLASS_PROTOTYPE( Path );

						Path();
						Path( int numnodes );
		void			Clear( void );
		void			Reset( void );
		void			AddNode( PathNode *node );
		void			InsertNode( PathNode *node, int const insertionPoint );
		void			RemoveNode( PathNode *node );
		PathNode		*GetNode( int num );
		int			GetNodeIndex( PathNode *node );
		PathNode		*PreviousNode( void );
		PathNode		*NextNode( void );
      PathNode		*NextNode( const PathNode *node );
		Vector		ClosestPointOnPath( const Vector &pos );
		float			DistanceAlongPath( const Vector &pos );
		Vector		PointAtDistance( float dist );
		PathNode		*NextNode( float dist );
		void			DrawPath( float r, float g, float b, float time );
		int			NumNodes( void );
		float			Length( void );
		PathNode		*Start( void );
		PathNode		*End( void );
	   virtual void Archive( Archiver &arc );
	};

inline void Path::Archive
	(
	Archiver &arc
	)
   {
   PathNodePtr node;
   int i, num;

   Class::Archive( arc );

   if ( arc.Saving() )
      {
      num = pathlist.NumObjects();
      }
   arc.ArchiveInteger( &num );
   if ( arc.Loading() )
      {
      pathlist.FreeObjectList();
      if ( num )
         pathlist.Resize( num );
      distanceToNextNode.FreeObjectList();
      dirToNextNode.FreeObjectList();
      }
   for ( i = 1; i <= num; i++ )
      {
		if ( arc.Loading() )
			pathlist.AddObject( node );

      arc.ArchiveSafePointer( pathlist.AddressOfObjectAt( i ) );
      }

	distanceToNextNode.Archive( arc );
   dirToNextNode.Archive( arc );

   arc.ArchiveFloat( &pathlength );
   arc.ArchiveSafePointer( &from );
   arc.ArchiveSafePointer( &to );
   arc.ArchiveInteger( &nextnode );
   }

typedef SafePtr<Path> PathPtr;

#endif /* path.h */
