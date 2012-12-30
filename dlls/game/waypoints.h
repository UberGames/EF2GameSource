//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/waypoints.h                                   $
// $Revision:: 9                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:

//
// Forward Declarations
//
class WayPointNode;

#ifndef __WAYPOINTS_H__
#define __WAYPOINTS_H__

#include "entity.h"

typedef SafePtr<WayPointNode> WayPointNodePtr;

class WayPointNode : public Entity
  {
	public:
		CLASS_PROTOTYPE( WayPointNode );
							  WayPointNode();
							 ~WayPointNode();

		//Events
		virtual void  SetWayPointThread ( Event *ev );		
		virtual void  SetActorAnim      ( Event *ev );
		        void  SetTargetname     ( Event *ev );
              void  SetTarget         ( Event *ev );
		
		
		virtual void  SetWayPointThread ( const str &thread );
		virtual void  SetActorAnim ( const str &thread );

		virtual const str     &GetThread();
		virtual const str     &GetActorAnim();	
		
		virtual void  Archive( Archiver &arc );
			
	private:
      str Thread;
		str ActorAnim;				

  };

inline void WayPointNode::Archive( Archiver &arc )
{
	Entity::Archive( arc );

	arc.ArchiveString( &Thread );
	arc.ArchiveString( &ActorAnim );
}

class PatrolWayPointNode : public WayPointNode
	{
	public:
		CLASS_PROTOTYPE( PatrolWayPointNode );
							  PatrolWayPointNode();
							 ~PatrolWayPointNode();
	
	};

class CallVolumeWayPointNode : public WayPointNode
   {
	public:
		CLASS_PROTOTYPE( CallVolumeWayPointNode );
							  CallVolumeWayPointNode();
							 ~CallVolumeWayPointNode();    
   };

class PositionWayPointNode : public WayPointNode
   {
   private:
      qboolean         _reserved;

	public:
		CLASS_PROTOTYPE( PositionWayPointNode );
							  PositionWayPointNode();
							 ~PositionWayPointNode(); 

      void Reserve( qboolean reserve );
      qboolean IsReserved();

	  virtual void  Archive( Archiver &arc );
   };

inline void PositionWayPointNode::Archive( Archiver &arc )
{
	WayPointNode::Archive( arc );

	arc.ArchiveBoolean( &_reserved );
}

#endif /* __WAYPOINTS_H__ */
