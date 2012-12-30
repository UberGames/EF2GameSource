//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/decals.h                                      $
// $Revision:: 5                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Decal entities

#ifndef __DECAL_H__
#define __DECAL_H__

#include "g_local.h"

class Decal : public Entity
	{
   private:
      str            shader;

	public:
      CLASS_PROTOTYPE( Decal );

	                  Decal();
      void           setDirection( const Vector &dir );
      void           setShader( const str &shader );
      void           setOrientation( const str &deg );
      void           setRadius( float rad );
      virtual void   Archive( Archiver &arc );
	};

inline void Decal::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveString( &shader );
   if ( arc.Loading() )
      {
      setShader( shader );
      }
   }

#endif // __DECAL_H__
