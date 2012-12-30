//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/scriptmaster.h                                $
// $Revision:: 9                                                              $
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
// Script masters are invisible entities that are spawned at the begining of each
// map.  They simple parse the script and send commands to the specified objects
// at the apropriate time.  Using a combination of simple commands, very complex
// scripted events can occur.
//

#ifndef __SCRIPTMASTER_H__
#define __SCRIPTMASTER_H__

#include "g_local.h"
#include "entity.h"
//#include "trigger.h"
#include "gamescript.h"
#include "container.h"
#include "scriptvariable.h"
//#include "worldspawn.h"
#include "program.h"
#include "globalcmd.h"

extern Program program;

#define MAX_COMMANDS 20

extern ScriptVariableList gameVars;
extern ScriptVariableList levelVars;

/* typedef enum
	{
	LEVEL_SCRIPT,
	MODEL_SCRIPT,
   CONSOLE_SCRIPT
	} scripttype_t;

extern ScriptVariableList gameVars;
extern ScriptVariableList levelVars;
extern ScriptVariableList parmVars;

extern Event EV_ProcessCommands;
extern Event EV_ScriptThread_Execute;
extern Event EV_ScriptThread_Callback;
extern Event EV_ScriptThread_CreateThread;
extern Event EV_ScriptThread_TerminateThread;
extern Event EV_ScriptThread_ControlObject;
extern Event EV_ScriptThread_Goto;
extern Event EV_ScriptThread_Pause;
extern Event EV_ScriptThread_Wait;
extern Event EV_ScriptThread_WaitFor;
extern Event EV_ScriptThread_WaitForThread;
extern Event EV_ScriptThread_WaitForSound;
extern Event EV_ScriptThread_End;
extern Event EV_ScriptThread_Print;
extern Event EV_ScriptThread_PrintInt;
extern Event EV_ScriptThread_PrintFloat;
extern Event EV_ScriptThread_PrintVector;
extern Event EV_ScriptThread_NewLine;
extern Event EV_ScriptThread_Clear;
extern Event EV_ScriptThread_Assert;
extern Event EV_ScriptThread_Break;
extern Event EV_ScriptThread_Clear;
extern Event EV_ScriptThread_Trigger;
extern Event EV_ScriptThread_Spawn;
extern Event EV_ScriptThread_Map;
extern Event EV_ScriptThread_SetCvar;
extern Event EV_ScriptThread_CueCamera;
extern Event EV_ScriptThread_CuePlayer;
extern Event EV_ScriptThread_FreezePlayer;
extern Event EV_ScriptThread_ReleasePlayer;
extern Event EV_ScriptThread_SetCinematic;
extern Event EV_ScriptThread_SetNonCinematic;
extern Event EV_ScriptThread_SetSkipThread;
extern Event EV_ScriptThread_MissionFailed;

class ScriptThread;

typedef SafePtr<ScriptThread> ThreadPtr;

class ThreadMarker;

class ScriptThread : public Listener
	{
	protected:
		int						threadNum;
		str						threadName;

		scripttype_t			type;
		GameScript				script;
      Container<TargetList *> targets;

		int						linenumber;
		qboolean					doneProcessing;
		qboolean					threadDying;

		Container<int>			updateList;
		float						waitUntil;
		str						waitingFor;
		ScriptThread			*waitingForThread;
      str						waitingForVariable;
      str						waitingForDeath;
      qboolean             waitingForPlayer;
		int						waitingNumObjects;
		ScriptVariableList	localVars;

		void						ObjectMoveDone( Event *ev );
		void 						CreateThread( Event *ev );
		void 						TerminateThread( Event *ev );
		void 						ControlObject( Event *ev );
		void 						EventGoto( Event *ev );
		void 						EventPause( Event *ev );
		void 						EventWait( Event *ev );
		void 						EventWaitFor( Event *ev );
		void						EventWaitForThread( Event *ev );
      void						EventWaitForVariable( Event *ev );
      void						EventWaitForDeath( Event *ev );
      void						EventWaitForSound( Event *ev );
		void						EventWaitForDialog( Event *ev );
      void						EventWaitForPlayer( Event *ev );
		void 						EventEnd( Event *ev );
		void 						Print( Event *ev );
		void 						PrintInt( Event *ev );
		void 						PrintFloat( Event *ev );
		void 						PrintVector( Event *ev );
		void 						NewLine( Event *ev );
		void 						Assert( Event *ev );
		void 						Break( Event *ev );
		void 						Clear( Event *ev );
		void 						ScriptCallback( Event *ev );
		void 						ThreadCallback( Event *ev );
      void 						VariableCallback( Event *ev );
      void 						DeathCallback( Event *ev );
		void 						DoMove( void );
		void 						Execute( Event *ev );
		void						TriggerEvent( Event *ev );
		void						ServerEvent( Event *ev );
		void						ClientEvent( Event *ev );
      void                 CacheResourceEvent( Event *ev );
      void                 RegisterAlias( Event *ev );
      void                 RegisterAliasAndCache( Event *ev );
		void						MapEvent( Event *ev );
		void						SetCvarEvent( Event *ev );
      void                 SetThreadName( Event *ev );

      TargetList           *GetTargetList( const str &targetname );

		void						CueCamera( Event *ev );
		void						CuePlayer( Event *ev );
		void						FreezePlayer( Event *ev );
		void						ReleasePlayer( Event *ev );
      void                 Spawn( Event *ev );
      void                 FadeIn( Event *ev	);
      void                 FadeOut( Event *ev );
      void                 FadeSound( Event *ev );
      void                 ClearFade( Event *ev );
      void                 Letterbox( Event *ev );
      void                 ClearLetterbox( Event *ev );
      void                 MusicEvent( Event *ev	);
      void                 ForceMusicEvent( Event *ev	);
		void						MusicVolumeEvent( Event *ev );
		void						RestoreMusicVolumeEvent( Event *ev );
      void                 SoundtrackEvent( Event *ev );
		void                 RestoreSoundtrackEvent( Event *ev );
		void						ScriptError( const char *fmt, ... );
		void						SetCinematic( Event *ev );
		void						SetNonCinematic( Event *ev );
		void						SetAllAIOff( Event *ev );
		void						SetAllAIOn( Event *ev );
		void						SetSkipThread( Event *ev );
      void                 PassToPathmanager( Event *ev );
      void                 StuffCommand( Event *ev );
		void		            KillEnt( Event *ev );
		void		            RemoveEnt( Event *ev );
		void		            KillClass( Event *ev );
		void		            RemoveClass( Event *ev );
      void                 CameraCommand( Event *ev );
      void                 FakePlayer( Event *ev );
      void                 RemoveFakePlayer( Event *ev );
      void                 SetDialogScript( Event *ev );
      void                 SetLightStyle( Event *ev );
      void                 KillThreadEvent( Event *ev );
      void                 CenterPrint( Event *ev );
      void                 LocationPrint( Event *ev );
		void                 MissionFailed( Event *ev );
      void                 ArenaCommand( Event *ev );

	public:
      CLASS_PROTOTYPE( ScriptThread );

									ScriptThread();
									~ScriptThread();
		void						ClearWaitFor( void );
		void						SetType( scripttype_t newtype );
		scripttype_t			GetType( void );
		void						SetThreadNum( int num );
		int						ThreadNum( void );
		const char				*ThreadName( void );
		int						CurrentLine( void );
		const char				*Filename( void );
		qboolean					WaitingFor( Entity *obj );
		ScriptThread			*WaitingOnThread( void );
      const char				*WaitingOnVariable( void );
      const char				*WaitingOnDeath( void );
      qboolean             WaitingOnPlayer( void );
		ScriptVariableList	*Vars( void );
		qboolean					Setup( int num, GameScript *scr, const char *label );
		qboolean					SetScript( const char *name );
		qboolean					Goto( const char *name );
		qboolean					labelExists( const char *name );
		void                 Start( float delay );
		void                 StartImmediately( void );

		void						Mark( ThreadMarker *mark );
		void						Restore( ThreadMarker *mark );

		void						SendCommandToSlaves( const char *name, Event *ev );
		qboolean					FindEvent( const char *name );
		void						ProcessCommand( int argc, const char **argv );
		void						ProcessCommandFromEvent( Event *ev, int startarg );
      virtual void         Archive( Archiver &arc );
	};

inline void ScriptThread::Archive
	(
	Archiver &arc
	)

   {
   Listener::Archive( arc );

   arc.ArchiveInteger( &threadNum );
   arc.ArchiveString( &threadName );

   ArchiveEnum( type, scripttype_t );

   arc.ArchiveObject( &script );

   // targets
   // don't need to save out targets
   if ( arc.Loading() )
      {
      targets.ClearObjectList();
      }

   arc.ArchiveInteger( &linenumber );
   arc.ArchiveBoolean( &doneProcessing );
   arc.ArchiveBoolean( &threadDying );

   // updateList
   // don't need to save out updatelist
   if ( arc.Loading() )
      {
      updateList.ClearObjectList();
      }

   arc.ArchiveFloat( &waitUntil );
   arc.ArchiveString( &waitingFor );
   arc.ArchiveObjectPointer( ( Class ** )&waitingForThread );
   arc.ArchiveString( &waitingForVariable );
   arc.ArchiveString( &waitingForDeath );
   arc.ArchiveBoolean( &waitingForPlayer );
   arc.ArchiveInteger( &waitingNumObjects );
   arc.ArchiveObject( &localVars );
   }

class ThreadMarker : public Class
	{
	public:
      CLASS_PROTOTYPE( ThreadMarker );

		int						linenumber;
		qboolean					doneProcessing;
		float						waitUntil;
		str						waitingFor;
		ScriptThread			*waitingForThread;
      str						waitingForVariable;
      str						waitingForDeath;
      qboolean             waitingForPlayer;
		int						waitingNumObjects;
		GameScriptMarker		scriptmarker;
      virtual void         Archive( Archiver &arc );
	};

inline void ThreadMarker::Archive
	(
	Archiver &arc
	)
   {
   Class::Archive( arc );

   arc.ArchiveInteger( &linenumber );
   arc.ArchiveBoolean( &doneProcessing );
   arc.ArchiveFloat( &waitUntil );
   arc.ArchiveString( &waitingFor );
   arc.ArchiveObjectPointer( ( Class ** )&waitingForThread );
   arc.ArchiveString( &waitingForVariable );
   arc.ArchiveString( &waitingForDeath );
   arc.ArchiveBoolean( &waitingForPlayer );
   arc.ArchiveInteger( &waitingNumObjects );
   arc.ArchiveObject( &scriptmarker );
   } */


class ScriptMaster : public Listener
	{
	protected:
		CThread							    *currentThread;
		Container<CThread *>			    Threads;		
		int								    threadIndex;
      qboolean                       player_ready;

	public:
      CLASS_PROTOTYPE( ScriptMaster );

											ScriptMaster();
											~ScriptMaster();
		void								CloseScript( void );
		qboolean							NotifyOtherThreads( int num );
		void								KillThreads( void );
		void								KillThread( const str &name );
		qboolean							KillThread( int num );
		qboolean							RemoveThread( int num );

		CThread     					*CurrentThread( void );
		void								SetCurrentThread( CThread *thread );
      CThread                    *CreateThread( const char *label );
		CThread                    *CreateThread( const char *label , Program *program );

		CThread			      		*CreateThread( void );
		CThread                    *CreateThread( Program *program );

		CThread		      			*GetThread( int num );

      bool                       isVarGroup( const char *name );
      void                       DeathMessage( const char *name );
      void                       PlayerSpawned( void );
      qboolean                   PlayerReady( void );
      void                       PlayerNotReady( void );
      qboolean                   Goto( GameScript * scr, const char *name );
      qboolean                   labelExists( GameScript * scr, const char *name );
      int                        GetUniqueThreadNumber( void );
      void                       FindLabels( void );
      virtual void               Archive( Archiver &arc );

	};

inline void ScriptMaster::Archive
	(
	Archiver &arc
	)
   {
   CThread * ptr;
   int i, num;

   Listener::Archive( arc );

   arc.ArchiveObjectPointer( ( Class ** )&currentThread );

   if ( arc.Saving() )
      num = Threads.NumObjects();
	else
		Threads.FreeObjectList();

	arc.ArchiveInteger( &num );

   for ( i = 1; i <= num; i++ )
      {
      if ( arc.Loading() )
         {
         ptr = new CThread;
         Threads.AddObject( ptr );
         }
      else
         {
         ptr = Threads.ObjectAt( i );
         }
      arc.ArchiveObject( ptr );
      }

   arc.ArchiveInteger( &threadIndex );
   arc.ArchiveBoolean( &player_ready );
   }

inline qboolean ScriptMaster::PlayerReady
	(
   void
	)
   {
   return player_ready;
   }

inline void ScriptMaster::PlayerNotReady
	(
   void
	)
   {
   player_ready = false;
   }

extern ScriptMaster Director;

class LightStyleClass : public Class
	{
	private:
      CLASS_PROTOTYPE( LightStyleClass );

		str            styles[ MAX_LIGHTSTYLES ];

	public:

   void              SetLightStyle( int index, const str &style );
   void              Archive( Archiver &arc );
   };

extern LightStyleClass lightStyles;

#endif /* scriptmaster.h */
