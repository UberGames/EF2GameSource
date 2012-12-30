//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/g_main.h                                      $
// $Revision:: 19                                                             $
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
// Global header file for g_main.cpp
//

#ifndef __G_MAIN_H__
#define __G_MAIN_H__

#include "g_local.h"
#include "gamecvars.h"
#include "level.h"

extern	Vector			vec_origin;
extern	Vector			vec_zero;

extern   qboolean       LoadingSavegame;
extern   qboolean       LoadingServer;

extern	game_import_t	gi;
extern	game_export_t	globals;

extern   gentity_t      *g_entities;
extern	gentity_t	   active_edicts;
extern	gentity_t	   free_edicts;

extern	int		      sv_numtraces;

extern   usercmd_t      *current_ucmd;

void			G_BeginIntermission( const char *map );
void			G_MoveClientToIntermission( Entity *client );
void			G_WriteClient( Archiver &arc, gclient_t *client );
void			G_AllocGameData( void );
void			G_DeAllocGameData( void );
void			G_ClientDrawBoundingBoxes( void );
void			G_ClientDrawSplines( void );

void			G_ExitWithError( const char *error );

void G_CheckIntermissionExit( void );
void G_BeginIntermission2( void );

extern "C" {
	void		G_SpawnEntities( const char *mapname, const char *entities, int time );
	void		G_PostLoad( void );
	void		G_SublevelPostLoad( const char *mapName );
	void		G_ClientEndServerFrames( void );
	void		G_ClientThink( gentity_t *ent, usercmd_t *cmd );
	const char *G_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot, qboolean checkPassword );
	void		G_ClientUserinfoChanged( gentity_t *ent, const char *userinfo );
	void		G_ClientDisconnect( gentity_t *ent );
	void		G_ClientBegin( gentity_t *ent, const usercmd_t *cmd );
	void		G_WritePersistant( const char *filename, qboolean sublevelTransition );
	qboolean	G_ReadPersistant( const char *filename, qboolean sublevelTransition );
	void		G_WriteLevel( const char *filename, qboolean autosave );
	qboolean	G_ReadLevel( const char *filename );
	qboolean	G_LevelArchiveValid( const char *filename );
	void		G_InitGame( int startTime, int randomSeed );
	void		G_ShutdownGame( void );
	void		G_CleanupGame( qboolean restart );
	void		G_RunFrame( int levelTime, int frametime );
	void		G_ServerCommand( void );
	void		G_ClientThink( gentity_t *ent, usercmd_t *ucmd );
	qboolean	G_SendEntity( gentity_t *clientEntity, gentity_t *entityToSend );
	void		G_UpdateEntityStateForClient( gentity_t *clientEntity, entityState_t *state );
	void		G_UpdatePlayerStateForClient( gentity_t *clientEntity, playerState_t *state );
	void		G_ExtraEntitiesToSend( gentity_t *clientEntity, int *numExtraEntities, int *extraEntities );
	void		G_AddEntityToExtraList(int entityNum);
	void		G_RemoveEntityFromExtraList(int entityNum);
	int			G_GetEntityCurrentAnimFrame( int entityNum, int bodyPart );
	int			G_GetTotalGameFrames( void );
	}

void     ClosePlayerLogFile( void );

qboolean    SV_FilterPacket( const char *from );
void        SVCmd_AddIP_f( void );
void        SVCmd_RemoveIP_f( void );
void        SVCmd_ListIP_f( void );
void        SVCmd_WriteIP_f( void );

#endif /* g_main.h */
