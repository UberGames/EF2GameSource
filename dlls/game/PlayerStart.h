//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/PlayerStart.h                                 $
// $Revision:: 4                                                              $
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
// Player start location entity declarations
//

#ifndef __PLAYERSTART_H__
#define __PLAYERSTART_H__

#include "g_local.h"
#include "entity.h"
#include "camera.h"
#include "navigate.h"

class PlayerStart : public Entity
	{
   private:
      str            thread;
	public:
      CLASS_PROTOTYPE( PlayerStart );

		void           SetAngle( Event *ev );
      void           SetThread( Event *ev );
      str            getThread( void );
	   virtual void   Archive(Archiver &arc);
	};

inline void PlayerStart::Archive (Archiver &arc)
   {
	Entity::Archive( arc );

	arc.ArchiveString(&thread);
   }

class TestPlayerStart : public PlayerStart
	{
	public:
		CLASS_PROTOTYPE( TestPlayerStart );
	};

class PlayerDeathmatchStart : public PlayerStart
	{
   private:
      void           SetType( Event *ev );

	public:
      str			_type;

      PlayerDeathmatchStart() {};

		CLASS_PROTOTYPE( PlayerDeathmatchStart );

	};

class PlayerIntermission : public Camera
	{
	public:
		CLASS_PROTOTYPE( PlayerIntermission );
      PlayerIntermission();
	};

#endif /* PlayerStart.h */
