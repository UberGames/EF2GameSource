//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/goo.h                                         $
// $Revision:: 3                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Goo Gun Projectile

#ifndef __GOO_H__
#define __GOO_H__

#include "weapon.h"
#include "weaputils.h"

class GooProjectile : public Projectile
   {
   private:
      str               m_debrismodel;
      int               m_debriscount;

   public:
      CLASS_PROTOTYPE( GooProjectile );

                        GooProjectile();
      void              Explode( Event *ev );
      void              SetDebrisModel( Event *ev );
      void              SetDebrisCount( Event *ev );
      void              Archive( Archiver &arc );
   };

void GooProjectile::Archive
   (
   Archiver &arc
   )

   {
   Projectile::Archive( arc );
   arc.ArchiveString( &m_debrismodel );
   arc.ArchiveInteger( &m_debriscount );
   }


class GooDebris : public Projectile
   {
   private:
      float          nexttouch;

   public:
      CLASS_PROTOTYPE( GooDebris );

                     GooDebris();
      void           Touch( Event *ev );
      void           Prethink( Event *ev );
      void           Archive( Archiver &arc );
   };

void GooDebris::Archive
   (
   Archiver &arc
   )

   {
   Projectile::Archive( arc );
   arc.ArchiveFloat( &nexttouch );
   }

#endif // __GOO_H__
