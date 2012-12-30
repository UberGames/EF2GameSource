//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/globalcmd.h                                    $
// $Revision:: 42                                                             $
//     $Date:: 4/14/03 4:33p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Global commands for scripts
//

#ifndef __GLOBALCMD_H__
#define __GLOBALCMD_H__

#include "interpreter.h"
#include "g_local.h"

extern Event EV_ProcessCommands;
extern Event EV_ScriptThread_Execute;
extern Event EV_ScriptThread_Callback;
extern Event EV_ScriptThread_ThreadCallback;
extern Event EV_ScriptThread_DeathCallback;
extern Event EV_ScriptThread_CreateThread;
extern Event EV_ScriptThread_TerminateThread;
extern Event EV_ScriptThread_ControlObject;
extern Event EV_ScriptThread_Goto;
extern Event EV_ScriptThread_Pause;
extern Event EV_ScriptThread_Wait;
extern Event EV_ScriptThread_WaitFor;
extern Event EV_ScriptThread_WaitForThread;
extern Event EV_ScriptThread_WaitForSound;
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
// Add from old ScriptVariable events
extern Event EV_ScriptThread_TargetOf;
extern Event EV_ScriptThread_GetCVar;
extern Event EV_ScriptThread_Mod;
extern Event EV_ScriptThread_Div;
extern Event EV_ScriptThread_Round;
extern Event EV_ScriptThread_VectorScale;
extern Event EV_ScriptThread_VectorNormalize;
extern Event EV_ScriptThread_VectorDot;
extern Event EV_ScriptThread_VectorCross;
extern Event EV_ScriptThread_VectorLength;
extern Event EV_ScriptThread_VectorX;
extern Event EV_ScriptThread_VectorY;
extern Event EV_ScriptThread_VectorZ;
extern Event EV_ScriptThread_VectorSetX;
extern Event EV_ScriptThread_VectorSetY;
extern Event EV_ScriptThread_VectorSetZ;
extern Event EV_ScriptThread_AnglesToForward;
extern Event EV_ScriptThread_AnglesToLeft;
extern Event EV_ScriptThread_AnglesToUp;
extern Event EV_ScriptThread_FloorEvent ;
extern Event EV_ScriptThread_CeilEvent ;
extern Event EV_ScriptThread_RoundEvent ;

class CThread : public Interpreter
{
	private:
		float				waitUntil;
		CThread				*waitingForThread;
		str					waitingForDeath;
		qboolean			waitingForPlayer;
		TargetList			*waitingFor;
		int					waitingNumObjects;
		EntityPtr			currentEntity;

	public:
		CLASS_PROTOTYPE( CThread );

								CThread();

		void					ExecuteFunc( Event *ev );
		void					ClearWaitFor( void );
		qboolean				WaitingFor( Entity *obj );
		void					ObjectMoveDone( Event *ev );
		void					CreateThread( Event *ev );
		void					TerminateThread( Event *ev );
		void					EventPause( Event *ev );
		void					EventWait( Event *ev );
		void					EventWaitFor( Event *ev );
		void					EventWaitForThread( Event *ev );
		void					EventWaitForDeath( Event *ev );
		void					EventWaitForSound( Event *ev );
		void					EventWaitForDialog( Event *ev );
		void					EventWaitDialogLength( Event *ev );
		void					EventWaitForAnimation( Event *ev );
		void					EventWaitForPlayer( Event *ev );
		void					CPrint( Event *ev );								
		void					Print( Event *ev );
		void					PrintInt( Event *ev );
		void					PrintFloat( Event *ev );
		void					PrintVector( Event *ev );
		void					NewLine( Event *ev );
		void					Assert( Event *ev );
		void					Break( Event *ev );
		void					ScriptCallback( Event *ev );
		void					ThreadCallback( Event *ev );
		void					DeathCallback( Event *ev );
		void					DoMove( void );
		void					TriggerEvent( Event *ev );
		void					TriggerEntityEvent( Event *ev );
		void					CacheResourceEvent( Event *ev );
		void					RegisterAlias( Event *ev );
		void					RegisterAliasAndCache( Event *ev );
		void					MapEvent( Event *ev );
		void					noIntermission( Event *ev );
		void					dontSaveOrientation( Event *ev );
		void					setPlayerDeathThread( Event *ev );
		void					endPlayerDeathThread( Event *ev );
		void					GetCvarEvent( Event *ev );
		void					GetCvarFloatEvent( Event *ev );
		void					GetCvarIntEvent( Event *ev );
		void					SetCvarEvent( Event *ev );
		void					CueCamera( Event *ev );
		void					CuePlayer( Event *ev );
		void			        FreezePlayer( Event *ev );
		void			        ReleasePlayer( Event *ev );
		void					FakePlayer( Event *ev );
		void					RemoveFakePlayer( Event *ev );
		void					Spawn( Event *ev );
		void					Letterbox( Event *ev );
		void					ClearLetterbox( Event *ev );
		void					FadeIn( Event *ev );
		void					ClearFade( Event *ev );
		void					FadeOut( Event *ev );
		void					FadeIsActive( Event *ev );
		void					MusicEvent( Event *ev );
		void					MusicVolumeEvent( Event *ev );
		void					RestoreMusicVolumeEvent( Event *ev );
		void					allowMusicDucking( Event *ev );
		void					allowActionMusic( Event *ev );
		void					ForceMusicEvent( Event *ev );
		void					SoundtrackEvent( Event *ev );
		void					RestoreSoundtrackEvent( Event *ev );
		void					SetCinematic( Event *ev );
		void					SetNonCinematic( Event *ev );
		void					SetLevelAI( Event *ev );
		void					SetSkipThread( Event *ev );
		void					PassToPathmanager( Event *ev );
		void					StuffCommand( Event *ev );
		void					KillEnt( Event *ev );
		void					RemoveEnt( Event *ev );
		void					KillClass( Event *ev );
		void					RemoveClass( Event *ev );
		void					RemoveActorsNamed( Event* ev );
		void					doesEntityExist( Event *ev );
		void					GetEntityEvent( Event *ev );
		void					GetNextEntityEvent( Event *ev );
		void					CosDegrees( Event *ev );
		void					SinDegrees( Event *ev );
		void					CosRadians( Event *ev );
		void					SinRadians( Event *ev );
		void					ArcTanDegrees( Event *ev );
		void					ScriptSqrt( Event *ev );
		void					ScriptLog( Event *ev );
		void					ScriptExp( Event *ev );
		void					RandomFloat( Event *ev );
		void					RandomInteger( Event *ev );
		void					CameraCommand( Event *ev );
		void					SetLightStyle( Event *ev );
		void					KillThreadEvent( Event *ev );
		void					SetThreadName( Event *ev );
		void					GetCurrentEntity( Event *ev );
		void					SetCurrentEntity( Entity *ent );

		CThread					*WaitingOnThread( void );
		const char				*WaitingOnDeath( void );
		qboolean				WaitingOnPlayer( void );

		void					DelayedStart( float delay );
		void					Start( void );

		void					SendCommandToSlaves( const char *name, Event *ev );
		void					Parse( const char *filename );
		void					ProcessCommand( int argc, const char **argv );

		void					Archive( Archiver &arc );

		void					SetFloatVar( Event *ev );
		void					SetVectorVar( Event *ev );
		void					SetStringVar( Event *ev );

		void					doesVarExist( Event *ev );
		void					RemoveVariable( Event* ev );

		void					GetFloatVar( Event *ev );
		void					GetVectorVar( Event *ev );
		void					GetStringVar( Event *ev );

		void					CenterPrint( Event *ev );
		void					isActorDead( Event *ev );
		qboolean				isActorDead( const str &actor_name );

		void					isActorAlive( Event *ev );
		qboolean				isActorAlive( const str &actor_name );

		void					SendClientCommand( Event *ev );
		void					GetNumFreeReliableServerCommands( Event* ev );
		void					SendClientVar( Event *ev );
		void					ModEvent( Event *ev );
		void					DivEvent( Event *ev );
		void					VectorScaleEvent( Event *ev );
		void					VectorDotEvent( Event *ev );
		void					VectorCrossEvent( Event *ev );
		void					VectorNormalizeEvent( Event *ev );
		void					VectorLengthEvent( Event *ev );
		void					VectorSetXEvent( Event *ev );
		void					VectorSetYEvent( Event *ev );
		void					VectorSetZEvent( Event *ev );
		void					VectorGetXEvent( Event *ev );
		void					VectorGetYEvent( Event *ev );
		void					VectorGetZEvent( Event *ev );
		void					VectorForwardEvent( Event *ev );
		void					VectorLeftEvent( Event *ev );
		void					VectorUpEvent( Event *ev );
		void					vectorToString( Event *ev );
		void					TargetOfEvent( Event *ev );
		void					FloorEvent( Event *ev );
		void					CeilEvent( Event *ev );
		void					RoundEvent( Event *ev );
		void					GetGameplayFloat( Event *ev );
		void					GetGameplayString( Event *ev );
		void					SetGameplayFloat( Event *ev );
		void					SetGameplayString( Event *ev );
		void					GetIntegerFromString( Event *ev );

		void					CreateCinematic( Event *ev );
		void					GetLevelTime( Event* ev );

		void					connectPathnodes( Event *ev );
		void					disconnectPathnodes( Event *ev );
   };

typedef SafePtr<CThread> ThreadPtr;

inline void CThread::DelayedStart( float delay )
{
	CancelEventsOfType( EV_ScriptThread_Execute );
	PostEvent( EV_ScriptThread_Execute, delay );
}

inline void CThread::Start( void )
{
	CancelEventsOfType( EV_ScriptThread_Execute );
	ProcessEvent( EV_ScriptThread_Execute );
}

inline CThread *CThread::WaitingOnThread( void )
{
	return waitingForThread;
}

inline const char *CThread::WaitingOnDeath( void )
{
	return waitingForDeath.c_str();
}

inline qboolean CThread::WaitingOnPlayer( void )
{
	return waitingForPlayer;
}

#endif
