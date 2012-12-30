//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/nature.h                                      $
// $Revision:: 5                                                              $
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

#include "g_local.h"
#include "trigger.h"

class Emitter : public Entity
   {
   private:
      str  emitterName;
      void setEmitter( const str &name );
      void EmitterName( Event *ev );
   public:
      CLASS_PROTOTYPE( Emitter );
      Emitter();
      virtual void   Archive( Archiver &arc );
   };

inline void Emitter::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveString( &emitterName );
   if ( arc.Loading() )
      {
      setEmitter( emitterName );
      }
   }


class Rain : public Emitter
   {
   private:
      str  rainName;
      void setRainName( const str &name );
   public:
      CLASS_PROTOTYPE( Rain );
      Rain();
      virtual void   Archive( Archiver &arc );
   };

inline void Rain::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveString( &rainName );
   if ( arc.Loading() )
      {
      setRainName( rainName );
      }
   }


class PuffDaddy : public Entity
   {
   private:
      void Touch( Event *ev );
      void Idle( Event *ev );

   public:
      CLASS_PROTOTYPE( PuffDaddy );
      PuffDaddy();
   };

