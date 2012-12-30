//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/mover.h                                       $
// $Revision:: 7                                                              $
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
// Base class for any object that needs to move to specific locations over a
// period of time.  This class is kept separate from most entities to keep
// class size down for objects that don't need such behavior.
//

#ifndef __MOVER_H__
#define __MOVER_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"

#define MOVE_ANGLES 1
#define MOVE_ORIGIN 2

class Mover : public Listener
	{
	private:
      Vector				finaldest;
		Vector				angledest;
		Event					*endevent;
		int					moveflags;

		Entity				*self;

	public:
		CLASS_PROTOTYPE( Mover );

								Mover();
								Mover( Entity *ent );
		virtual				~Mover();
		void					MoveDone( Event *ev );
		void					MoveTo( const Vector &tdest, const Vector &angdest, float tspeed, Event &event );
		void					LinearInterpolate( const Vector &tdest, const Vector &angdest, float time, Event &event );
		const int				GetMoveFlags( void ) const { return moveflags; }
		void					SetMoveFlags( const int moveFlags ) { moveflags = moveFlags; }
		void					SetEndEvent( const int endEvent );
		void					SetFinalDestination( const Vector &finalDestination) { finaldest = finalDestination; }
	   virtual void      Archive( Archiver &arc );
   };

inline void Mover::Archive
	(
	Archiver &arc
	)
   {
   Listener::Archive( arc );

   arc.ArchiveVector( &finaldest );
   arc.ArchiveVector( &angledest );
   arc.ArchiveEventPointer( &endevent );
   arc.ArchiveInteger( &moveflags );
   }

#endif
