//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/soundman.h                                    $
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
// Sound Manager
//

#ifndef __SOUND_MANAGER_H__
#define __SOUND_MANAGER_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"

class SoundManager : public Listener
   {
   protected:
      int               currentFacet;
      Entity            *current;
      Container<Entity *> soundList;

      void              AddSpeaker( Event *ev );
      void              AddRandomSpeaker( Event *ev );
      void              AddMusicTrigger( Event *ev );
      void              AddReverbTrigger( Event *ev );
      void              Replace( Event *ev );
      void              Delete( Event *ev );
      void              MovePlayer( Event *ev );
      void              Next( Event *ev );
      void              Previous( Event *ev );
      void              ShowingSounds( Event *ev );
      void              Show( Event *ev );
      void              Hide( Event *ev );
      void              Save( Event *ev );
      void              UpdateEvent( Event *ev );
      void              ResetEvent( Event *ev );
      void              GlobalTranslateEvent( Event *ev );
      void              SwitchFacetEvent( Event *ev );
      void              PreviewReverbEvent( Event *ev );
      void              ResetReverbEvent( Event *ev );

      void              Show( void );
      void              UpdateUI( void );
      void              Save( void );
      void              CurrentLostFocus( void );
      void              CurrentGainsFocus( void );
      void              UpdateSpeaker( TriggerSpeaker * speaker );
      void              UpdateRandomSpeaker( RandomSpeaker * speaker );
      void              UpdateTriggerMusic( TriggerMusic * music );
      void              UpdateTriggerReverb( TriggerReverb * reverb );

   public:
      CLASS_PROTOTYPE( SoundManager );

                        SoundManager();
		void              Init( void );
      void              Reset( void );
      void              Load( void );
      void              AddEntity( Entity * ent );
	   virtual void      Archive( Archiver &arc );
   };

inline void SoundManager::Archive
	(
	Archiver &arc
	)

   {
   int               i;
   int               num;
   int               currentFacet;

   Listener::Archive( arc );

   arc.ArchiveInteger( &currentFacet );
   arc.ArchiveObjectPointer( ( Class ** )&current );

   if ( arc.Saving() )
      {
      num = soundList.NumObjects();
      arc.ArchiveInteger( &num );
      }
   else
      {
      soundList.ClearObjectList();
      arc.ArchiveInteger( &num );
      soundList.Resize( num );
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveObjectPointer( ( Class ** )soundList.AddressOfObjectAt( i ) );
      }
   }

extern SoundManager SoundMan;

#endif /* camera.h */
