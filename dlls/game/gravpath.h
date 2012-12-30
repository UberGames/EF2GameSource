//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/gravpath.h                                    $
// $Revision:: 5                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Gravity path - Used for underwater currents and wells.

#ifndef __GRAVPATH_H__
#define __GRAVPATH_H__

#include "g_local.h"
#include "class.h"
#include "container.h"


class GravPathNode : public Entity
   {
   private:
      float    speed;
      float    radius;
      qboolean headnode;
		float		max_speed;

   public:
      qboolean       active;

      CLASS_PROTOTYPE(GravPathNode);
                     GravPathNode();
      void           SetSpeed( Event *ev );
		void           SetMaxSpeed( Event *ev );
      void           SetRadius( Event *ev );
      void           CreatePath( Event *ev );
      void           Activate( Event *ev );
      void           Deactivate( Event *ev );
      float          Speed( void );
		float          MaxSpeed( void );
      float          Radius( void ) { return radius; };
	   virtual void   Archive( Archiver &arc );
   };

inline void GravPathNode::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveFloat( &speed );
   arc.ArchiveFloat( &radius );
   arc.ArchiveBoolean( &headnode );
	arc.ArchiveFloat( &max_speed );

	arc.ArchiveBoolean( &active );
   }

typedef SafePtr<GravPathNode> GravPathNodePtr;

class GravPath : public Listener
	{
	private:
		Container<GravPathNodePtr>	pathlist;
		float							   pathlength;

		GravPathNodePtr    	      from;
		GravPathNodePtr   		   to;
		int							   nextnode;

	public:
      CLASS_PROTOTYPE( GravPath );

						      GravPath();
                        ~GravPath();
		void			      Clear(void);
		void			      Reset(void);
		void			      AddNode(GravPathNode *node);
		GravPathNode      *GetNode(int num);
		GravPathNode      *NextNode(void);
		Vector		      ClosestPointOnPath(const Vector &pos, Entity &ent,float *bestdist,float *speed,float *radius);
		float			      DistanceAlongPath(const Vector &pos, float *speed);
		Vector		      PointAtDistance( const Vector &pos, float dist, qboolean is_player, float *max_distance );
		void			      DrawPath(float r, float g, float b);
		int			      NumNodes(void);
		float			      Length(void);
		GravPathNode		*Start(void);
		GravPathNode		*End(void);
	   virtual void      Archive( Archiver &arc );

      Vector                     mins;
      Vector                     maxs;
      Vector                     origin;
      qboolean                   force;
	};

inline void GravPath::Archive
	(
	Archiver &arc
	)

   {
   GravPathNodePtr *tempPtr;
   int i, num;

   Listener::Archive( arc );

   if ( arc.Loading() )
      {
      Reset();
      }
   else
      {
      num = pathlist.NumObjects();
      }
   arc.ArchiveInteger( &num );
   if ( arc.Loading() )
      {
      pathlist.Resize( num );
      }

   for ( i = 1; i <= num; i++ )
      {
      tempPtr = pathlist.AddressOfObjectAt( i );
      arc.ArchiveSafePointer( tempPtr );
      }

   arc.ArchiveFloat( &pathlength );
   arc.ArchiveSafePointer( &from );
   arc.ArchiveSafePointer( &to );
   arc.ArchiveInteger( &nextnode );
   arc.ArchiveVector( &mins );
   arc.ArchiveVector( &maxs );
   arc.ArchiveVector( &origin );
   arc.ArchiveBoolean( &force );
   }

class GravPathManager : public Class
   {
	private:
	   Container<GravPath *>	pathList;

	public:
      CLASS_PROTOTYPE( GravPathManager );
               ~GravPathManager();
      void     Reset( void );
      void     AddPath(GravPath *p);
      void     RemovePath(GravPath *p);
      Vector   CalculateGravityPull(Entity &ent, const Vector &position, qboolean *force, float *max_speed);
      void     DrawGravPaths( void );
	   virtual void Archive( Archiver &arc );
   };

inline void GravPathManager::Archive
	(
	Archiver &arc
	)
   {
   GravPath * ptr;
   int i, num;

   Class::Archive( arc );

   if ( arc.Saving() )
      {
      num = pathList.NumObjects();
      }
   else
      {
      Reset();
      }
   arc.ArchiveInteger( &num );
   for ( i = 1; i <= num; i++ )
      {
      if ( arc.Saving() )
         {
         ptr = pathList.ObjectAt( i );
         }
      else
         {
         ptr = new GravPath;
         }
      arc.ArchiveObject( ptr );
      if ( arc.Loading() )
         {
         pathList.AddObject( ptr );
         }
      }
   }

extern GravPathManager gravPathManager;

#endif /* gravpath.h */
