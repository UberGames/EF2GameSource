//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/g_spawn.h                                     $
// $Revision:: 2                                                              $
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
//

#ifndef __G_SPAWN_H__
#define __G_SPAWN_H__

#include "entity.h"

// spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM			0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_DEVELOPMENT		0x00002000
#define	SPAWNFLAG_DETAIL		      0x00004000

class SpawnArgs : public Class
   {
   private:
      Container<str> keyList;
      Container<str> valueList;

   public:
                     CLASS_PROTOTYPE( SpawnArgs );

                     SpawnArgs();
                     SpawnArgs( SpawnArgs &arglist );

      void           Clear( void );

      const char     *Parse( const char *data );
      const char     *getArg( const char *key, const char *defaultValue = NULL );
      void           setArg( const char *key, const char *value );

      int            NumArgs( void );
      const char     *getKey( int index );
      const char     *getValue( int index );
		void           operator=( SpawnArgs &a );

      ClassDef       *getClassDef( qboolean *tikiWasStatic = NULL );
      Entity		   *Spawn( void );

	   virtual void   Archive( Archiver &arc );
	};

void			G_InitClientPersistant( gclient_t *client );
ClassDef    *FindClass( const char *name, qboolean *isModel	);

#endif
