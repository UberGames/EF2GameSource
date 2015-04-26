//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/g_main.cpp                                $
// $Revision:: 104                                                            $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
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
#include "_pch_cpp.h"

#define SAVEGAME_VERSION 8
#define PERSISTANT_VERSION 1

#include "g_utils.h"
#include "gamecmds.h"
#include "entity.h"
#include "vector.h"
#include "scriptmaster.h"
#include "navigate.h"
#include "player.h"
#include "gravpath.h"
#include "camera.h"
#include "level.h"
#include "viewthing.h"
#include "ipfilter.h"
#include <qcommon/alias.h>
#include "mp_manager.hpp"
#include <qcommon/gameplaymanager.h>
#include <qcommon/stringresource.hpp>
#include "teammateroster.hpp"

Vector			vec_origin = Vector(0, 0, 0);
Vector			vec_zero = Vector(0, 0, 0);

qboolean		LoadingSavegame = false;
qboolean		LoadingServer = false;
game_import_t	gi;
game_export_t	globals;

gentity_t		*g_entities = NULL;
gentity_t		active_edicts;
gentity_t		free_edicts;

Container<int>	extraEntitiesList;

int				sv_numtraces;

usercmd_t		*current_ucmd;

void (*ServerError )( int level, const char *fmt, ... );

/*
===============
G_Error

Abort the server with a game error
===============
*/
void G_Error( int level, const char *fmt, ... )
{
	Q_UNUSED(level);

	va_list	argptr;
	char		error[ 4096 ];
	
	va_start( argptr, fmt );
	vsprintf( error, fmt, argptr );
	va_end( argptr );
	
	assert( 0 );
	
	throw error;
}

/*
===============
G_ExitWithError

Calls the server's error function with the last error that occurred.
Should only be called after an exception.
===============
*/
void G_ExitWithError( const char *error )
{
	static char G_ErrorMessage[ 4096 ];
	
	//ServerError( ERR_DROP, error );
	
	Q_strncpyz( G_ErrorMessage, error, sizeof( G_ErrorMessage ) );
	
	globals.error_message = G_ErrorMessage;
}

/*
=================
G_CleanupGame

Frees up resources from current level
=================
*/
extern "C" void G_CleanupGame( qboolean restart )
{
	try
	{
		gi.DPrintf ("==== CleanupGame ====\n");
		
		level.CleanUp( restart );
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

int BotAIShutdown( int restart );
/*
=================
G_ShutdownGame

Frees up any resources
=================
*/
extern "C" void G_ShutdownGame( void )
{
	try
	{
		gi.DPrintf ("==== ShutdownGame ====\n");
		
		// close the player log file if necessary
		ClosePlayerLogFile();
		
		// shut down bots
		if ( gi.Cvar_VariableIntegerValue( "bot_enable" ) ) {
			BotAIShutdown( qfalse ); // BOTLIB was restart
		}
		
		// clear out the behavior package list
		ClearBehaviorPackageList();
		
		level.CleanUp( false );
		
		L_ShutdownEvents();
		
		// destroy the game data
		G_DeAllocGameData();
		
		// Delete the GameplayManager
		GameplayManager::shutdown();
	}
	
	
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

/*
============
G_InitGame

This will be called when the dll is first loaded, which
only happens when a new game is begun
============
*/
extern "C" void G_InitGame( int startTime, int randomSeed )
{
	gi.DPrintf ("==== InitGame ====\n");
	
	// Install our own error handler, since we can't
	// call the EXE's handler from within a C++ class
	ServerError = gi.Error;
	gi.Error    = G_Error;
	
	// If we get an error, call the server's error function
	try
	{
		srand( randomSeed );
		
		// setup all the cvars the game needs
		CVAR_Init();
		
		// Need to fill the BehaviorPackageList from the file
		FillBehaviorPackageList();
		
		// initialize the game variables
		gameVars.ClearList();
		level.intermission_advancetime= (float)g_intermissiontime->integer;
		level.fixedframetime = ( 1.0f / sv_fps->value );
		level.startTime = startTime;
		
		G_InitConsoleCommands();
		L_InitEvents();
		
		sv_numtraces = 0;
		
		// setup ViewMaster
		Viewmodel.Init();
		
		// Initialize deathmatch manager
		multiplayerManager.init();
		
		game.maxentities = maxentities->integer;
		if (maxclients->integer * 8 > game.maxentities)
		{
			game.maxentities = maxclients->integer * 8;
		}
		game.maxclients = maxclients->integer;
		
		GameplayManager::create();
		
		G_AllocGameData();
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

void G_AllocGameData( void )
{
	int i;
	
	// de-allocate from previous level
	G_DeAllocGameData();
	
	// Initialize debug lines
	G_AllocDebugLines();
	
	// initialize all entities for this game
	game.maxentities     = maxentities->integer;
	g_entities           = ( gentity_t * )gi.Malloc (game.maxentities * sizeof(g_entities[0]) );
	
	// clear out the entities
	memset( g_entities, 0, sizeof( g_entities[ 0 ] ) * game.maxentities );
	globals.gentities    = g_entities;
	globals.max_entities = game.maxentities;
	
	// Add all the edicts to the free list
	LL_Reset( &free_edicts, next, prev );
	LL_Reset( &active_edicts, next, prev );
	for( i = 0; i < game.maxentities; i++ )
	{
		LL_Add( &free_edicts, &g_entities[ i ], next, prev );
	}
	
	// initialize all clients for this game
	game.clients = ( gclient_t * )gi.Malloc( game.maxclients * sizeof( game.clients[ 0 ] ) );
	memset( game.clients, 0, game.maxclients * sizeof( game.clients[ 0 ] ) );
	for( i = 0; i < game.maxclients; i++ )
	{
		// set client fields on player ents
		g_entities[ i ].client = game.clients + i;
		
		G_InitClientPersistant (&game.clients[i]);

		playersLastTeam[ i ] = "";
	}
	globals.num_entities = game.maxclients;
	
	// Tell the server about our data
	gi.LocateGameData( g_entities, globals.num_entities, sizeof( gentity_t ), &game.clients[0].ps, sizeof( game.clients[0] ) );
	
	if ( gi.Cvar_VariableIntegerValue( "bot_enable" ) ) {
		BotAISetup( 0);//restart );
		BotAILoadMap( 0);//restart );
		G_InitBots( 0);//restart );
	}
	
}

void G_DeAllocGameData( void )
{
	// Initialize debug lines
	G_DeAllocDebugLines();
	
	// free up the entities
	if ( g_entities )
	{
		gi.Free( g_entities );
		g_entities = NULL;
	}
	
	// free up the clients
	if ( game.clients )
	{
		gi.Free( game.clients );
		game.clients = NULL;
	}
}

extern "C" void G_SpawnEntities( const char *mapname, const char *entities, int levelTime )
{
	try
	{
		level.NewMap( mapname, entities, levelTime );
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

//----------------------------------------------------------------
// Name:			G_PostLoad
// Class:			
//
// Description:		Does everything necessary after a load has happened
//
// Parameters:		None
//
// Returns:			none
//----------------------------------------------------------------

extern "C" void G_PostLoad( void )
{
	try
	{
		level.postLoad();
	}

	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

//----------------------------------------------------------------
// Name:			G_PostSublevelLoad
// Class:			
//
// Description:		Does everything necessary after a sublevel has beend loaded
//
// Parameters:		const char *mapName		- map name string (also possibly contains spawn position)
//
// Returns:			none
//----------------------------------------------------------------

extern "C" void G_PostSublevelLoad( const char *spawnPosName )
{
	try
	{
		level.postSublevelLoad( spawnPosName );
	}

	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

void G_ArchivePersistantData( Archiver &arc, qboolean sublevelTransition )
{
	gentity_t   *ed;
	int         i;
	
	for( i = 0; i < game.maxclients; i++ )
	{
		Entity   *ent;
		
		ed = &g_entities[ i ];
		if ( !ed->inuse || !ed->entity )
			continue;
		
		ent = ed->entity;
		if ( !ent->isSubclassOf( Player ) )
			continue;
		( ( Player * )ent )->ArchivePersistantData( arc, sublevelTransition );
	}
}

qboolean G_ArchivePersistant( const char *name, qboolean loading, qboolean sublevelTransition )
{
	int version;
	Archiver arc;
	
	if ( loading )
	{
		if ( !arc.Read( name, false ) )
		{
			return false;
		}
		
		arc.ArchiveInteger( &version );
		if ( version < PERSISTANT_VERSION )
		{
			gi.Printf( "Persistant data from an older version (%d) of %s.\n", version, GAME_NAME );
			arc.Close();
			return false;
		}
		else if ( version > PERSISTANT_VERSION )
		{
			gi.DPrintf( "Persistant data from newer version %d of %s.\n", version, GAME_NAME );
			arc.Close();
			return false;
		}
	}
	else
	{
		arc.Create( name );
		
		version = PERSISTANT_VERSION;
		arc.ArchiveInteger( &version );
	}
	
	
	arc.ArchiveObject( &gameVars );
	G_ArchivePersistantData( arc, sublevelTransition );
	
	arc.Close();
	return true;
}


extern "C" qboolean G_ReadPersistant( const char *name, qboolean sublevelTransition )
{
	try
	{
		return G_ArchivePersistant( name, true, sublevelTransition );
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
	return false;
}

/*
============
G_WritePersistant

This will be called whenever the game goes to a new level,

A single player death will automatically restore from the
last save position.
============
*/

extern "C" void G_WritePersistant( const char *name, qboolean sublevelTransition )
{
	try
	{
		G_ArchivePersistant( name, false, sublevelTransition );
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}


/*
=================
LevelArchiveValid
=================
*/
qboolean LevelArchiveValid( Archiver &arc )
{
	int      version;
	int      savegame_version;
	
	// read the version number
	arc.ArchiveInteger( &version );
	arc.ArchiveInteger( &savegame_version );
	
	if ( version < GAME_API_VERSION )
	{
		gi.Printf( "Savegame from an older version (%d) of %s.\n", version, GAME_NAME );
		return false;
	}
	else if ( version > GAME_API_VERSION )
	{
		gi.Printf( "Savegame from version %d of %s.\n", version, GAME_NAME );
		return false;
	}
	
	if ( savegame_version < SAVEGAME_VERSION )
	{
		gi.Printf( "Savegame from an older version (%d) of %s.\n", version, GAME_NAME );
		return false;
	}
	else if ( savegame_version > SAVEGAME_VERSION )
	{
		gi.Printf( "Savegame from version %d of %s.\n", version, GAME_NAME );
		return false;
	}
	return true;
}

str getAliasParmString( AliasListNode_t *aliasNode )
{
	str parms;

	if ( aliasNode->global_flag )
	{
		parms += "global ";
	}

	if ( aliasNode->stop_flag )
	{
		parms += "stop ";
	}

	if ( aliasNode->timeout > 0.0f )
	{
		parms += "timeout ";
		parms += aliasNode->timeout;
		parms += " ";
	}

	if ( aliasNode->maximum_use > 0 )
	{
		parms += "maxuse ";
		parms += aliasNode->maximum_use;
		parms += " ";
	}

	if ( aliasNode->weight > 0 )
	{
		parms += "weight ";
		parms += aliasNode->weight;
		parms += " ";
	}

	if ( strlen( aliasNode->anim_name ) > 0 )
	{
		parms += "anim ";
		parms += aliasNode->anim_name;
		parms += " ";
	}

	if ( aliasNode->loop_anim )
	{
		parms += "loopanim 1 ";
	}

	return parms;
}

void ArchiveAliases( Archiver &arc )
{
	int i;
	byte another;
	AliasList_t *alias_list;
	AliasListNode_t *alias_node;
	AliasActorNode_t *actor_node;
	str alias_name;
	str realName;
	str model_name;
	const char *name;
	int model_index;
	int actor_number;
	int number_of_times_played;
	byte been_played_this_loop;
	int last_time_played;
	
	
	if ( arc.Saving() )
	{
		for( i = 0 ; i < MAX_MODELS ; i++ )
		{
			alias_list = (AliasList_t *)gi.Alias_GetList( i );
			
			if ( alias_list )
				alias_node = alias_list->data_list;
			else
				alias_node = NULL;
			
			if ( alias_node )
			{
				name = gi.NameForNum( i );
				
				if ( name )
				{
					another = true;
					arc.ArchiveByte( &another );
					
					model_name = name;
					arc.ArchiveString( &model_name );
					
					// Go through all aliases in this model
					
					while ( alias_node )
					{
						str parms;

						another = true;
						arc.ArchiveByte( &another );
						
						alias_name = alias_node->alias_name;
						realName = alias_node->real_name;
						
						arc.ArchiveString( &alias_name );

						arc.ArchiveString( &realName );

						parms = getAliasParmString( alias_node );

						arc.ArchiveString( &parms );
						
						arc.ArchiveInteger( &alias_node->number_of_times_played );
						arc.ArchiveByte( &alias_node->been_played_this_loop );
						arc.ArchiveInteger( &alias_node->last_time_played );
						
						actor_node = alias_node->actor_list;
						
						// Go through actor info
						
						while ( actor_node )
						{
							another = true;
							arc.ArchiveByte( &another );
							
							arc.ArchiveInteger( &actor_node->actor_number );
							arc.ArchiveInteger( &actor_node->number_of_times_played );
							arc.ArchiveByte( &actor_node->been_played_this_loop );
							arc.ArchiveInteger( &actor_node->last_time_played );
							
							actor_node = actor_node->next;
						}
						
						another = false;
						arc.ArchiveByte( &another );
						
						alias_node = alias_node->next;
					}
					
					another = false;
					arc.ArchiveByte( &another );
				}
			}
		}
		
		another = false;
		arc.ArchiveByte( &another );
	}
	else
	{
		arc.ArchiveByte( &another );
		
		while( another )
		{
			arc.ArchiveString( &model_name );
			
			model_index = gi.modelindex( model_name.c_str() );
			
			arc.ArchiveByte( &another );
			
			while( another )
			{
				str parms;

				// Read in aliases
				
				arc.ArchiveString( &alias_name );

				arc.ArchiveString( &realName );
				arc.ArchiveString( &parms );

				arc.ArchiveInteger( &number_of_times_played );
				arc.ArchiveByte( &been_played_this_loop );
				arc.ArchiveInteger( &last_time_played );
				
				// If the alias doesn't exist yet, add it

				if ( !gi.Alias_Find( model_index, alias_name.c_str() ) )
				{
					gi.Alias_Add( model_index, alias_name, realName, parms );
				}

				// Now update the alias

				gi.Alias_UpdateDialog( model_index, alias_name.c_str(), number_of_times_played, been_played_this_loop, last_time_played );
				
				arc.ArchiveByte( &another );
				
				while( another )
				{
					// Read in actor infos
					
					arc.ArchiveInteger( &actor_number );
					arc.ArchiveInteger( &number_of_times_played );
					arc.ArchiveByte( &been_played_this_loop );
					arc.ArchiveInteger( &last_time_played );
					
					gi.Alias_AddActorDialog( model_index, alias_name.c_str(), actor_number, number_of_times_played, been_played_this_loop, last_time_played );
					
					arc.ArchiveByte( &another );
				}
				
				arc.ArchiveByte( &another );
			}
			
			arc.ArchiveByte( &another );
		}
	}
}

/*
=================
G_ArchiveLevel

=================
*/
qboolean G_ArchiveLevel( const char *filename, qboolean autosave, qboolean loading )
{
	try
	{
		int		i;
		int      num;
		Archiver arc;
		gentity_t  *edict;
		
		// Heuristic Stuff /////////////////////////////////////
		edict = &g_entities[ 0 ];
		
		if ( edict->inuse && edict->entity )
		{
			Player *player = (Player *)edict->entity;
			if ( player->p_heuristics )
				player->p_heuristics->SaveHeuristics( player );			
		}
		//////////////////////////////////////////////
		
		
		if ( loading )
		{
			LoadingSavegame = true;
			LoadingServer = true;
			
			// Get rid of anything left over from the last level
			level.CleanUp( false );
			
			arc.Read( filename );
			
			if ( !LevelArchiveValid( arc ) )
            {
				arc.Close();
				return false;
            }
			
			// Read in the pending events.  These are read in first in case
			// later objects need to post events.
			L_UnarchiveEvents( arc );
		}
		else
		{
			int temp;
			
			if ( autosave )
            {
				for( i = 0; i < game.maxclients; i++ )
				{
					edict = &g_entities[ i ];
					if ( !edict->inuse && !edict->entity )
					{
						continue;
					}
					
					delete edict->entity;
				}
            }
			
			arc.Create( filename );
			
			// write out the version number
			temp = GAME_API_VERSION;
			arc.ArchiveInteger( &temp );
			temp = SAVEGAME_VERSION;
			arc.ArchiveInteger( &temp );
			
			
			
			// Write out the pending events.  These are written first in case
			// later objects need to post events when reading the archive.
			L_ArchiveEvents( arc );
		}
		// archive the game object
		arc.ArchiveObject( &game );
		
		// archive the game variables
		arc.ArchiveObject( &gameVars );
		
		// archive Level
		arc.ArchiveObject( &level );
		
		if ( arc.Loading() )
		{
			// Set up for a new map
			thePathManager.Init( level.mapname.c_str() );
		}
		
		// archive script librarian
		arc.ArchiveObject( &ScriptLib );
		
		// archive gravity paths
		arc.ArchiveObject( &gravPathManager );
		
		// archive camera paths
		arc.ArchiveObject( &CameraMan );
		
		// archive paths
		arc.ArchiveObject( &thePathManager );
		
		// archive script controller
		arc.ArchiveObject( &Director );
		
		// archive lightstyles
		arc.ArchiveObject( &lightStyles );

		// Archive the level vars

		if ( arc.Loading() )
		{
			levelVars.ClearList();
		}

		arc.ArchiveObject( &levelVars );
		
		if ( arc.Saving() )
		{
			// count the entities
			num = 0;
			for( i = 0; i < globals.num_entities; i++ )
			{
				edict = &g_entities[ i ];
				if ( edict->inuse && edict->entity && !( edict->entity->flags & FlagDontsave ) )
				{
					num++;
				}
            }
		}
		
		// archive all the entities
		arc.ArchiveInteger( &globals.num_entities );
		arc.ArchiveInteger( &num );
		
		if ( arc.Saving() )
		{
			// write out the world
			arc.ArchiveObject( world );
			
			for( i = 0; i < globals.num_entities; i++ )
			{
				edict = &g_entities[ i ];
				if ( !edict->inuse || !edict->entity || ( edict->entity->flags & FlagDontsave ) )
				{
					continue;
				}
				
				arc.ArchiveObject( edict->entity );
			}
		}
		else
		{
			// Tell the server about our data
			gi.LocateGameData( g_entities, globals.num_entities, sizeof( gentity_t ), &game.clients[0].ps, sizeof( game.clients[0] ) );
			
			// read in the world
			arc.ReadObject();
			
			for( i = 0; i < num; i++ )
			{
				arc.ReadObject();
			}
		}
		
		if ( arc.Loading() )
			program.FreeData();
		
		arc.ArchiveObject( &program );
		
		// archive script controller
		//arc.ArchiveObject( &Director );
		
		ArchiveAliases( arc );
		
		//Archive the teammate roster
		TeammateRoster::getInstance()->Archive( arc );
		
		// Archive the gameplay manager's database deltas.
		GameplayManager::getTheGameplayManager()->Archive( arc );
		
		arc.Close();
		
		if ( arc.Loading() )
		{
			LoadingSavegame = false;
			
			// call the precache scripts
			level.Precache();
		}
		else
		{
			gi.centerprintf( &g_entities[ 0 ], CENTERPRINT_IMPORTANCE_NORMAL, "@textures/menu/gamesaved" );
		}
		
		if ( arc.Loading() )
		{
			// Make sure all code that needs to setup the player after they have been loaded is run
			
			for( i = 0; i < game.maxclients; i++ )
			{
				edict = &g_entities[ i ];
				
				if ( edict->inuse && edict->entity )
				{
					Player *player = (Player *)edict->entity;
					player->Loaded();
				}
			}
		}
		
		return true;
	}
	  
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
	return false;
}

/*
=================
G_WriteLevel

=================
*/
extern "C" void G_WriteLevel( const char *filename, qboolean autosave )
{
	game.autosaved = autosave;
	G_ArchiveLevel( filename, autosave, false );
	game.autosaved = false;
}

/*
=================
G_ReadLevel

SpawnEntities will already have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
extern "C" qboolean G_ReadLevel( const char *filename )
{
	qboolean status;
	
	status = G_ArchiveLevel( filename, false, true );
	// if the level load failed make sure that these variables are not set
	if ( !status )
	{
		LoadingSavegame = false;
		LoadingServer = false;
	}
	return status;
}

/*
=================
G_LevelArchiveValid
=================
*/
extern "C" qboolean G_LevelArchiveValid( const char *filename )
{
	try
	{
		qboolean ret;
		
		Archiver arc;
		
		if ( !arc.Read( filename ) )
		{
			return false;
		}
		
		ret = LevelArchiveValid( arc );
		
		arc.Close();
		
		return ret;
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
		return false;
	}
}

extern "C" qboolean G_InMultiplayerGame( void )
{
	return multiplayerManager.inMultiplayer();
}

// BOTLIB additions
void SV_AddBot_f( void );
int BotAIStartFrame(int time);
//

/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
extern "C" game_export_t *GetGameAPI( game_import_t *import )
{
	gi = *import;
	
	globals.apiversion					= GAME_API_VERSION;
	globals.Init						= G_InitGame;
	globals.Shutdown					= G_ShutdownGame;
	globals.Cleanup						= G_CleanupGame;
	globals.SpawnEntities				= G_SpawnEntities;
	globals.PostLoad					= G_PostLoad;
	globals.PostSublevelLoad			= G_PostSublevelLoad;
	
	globals.WritePersistant				= G_WritePersistant;
	globals.ReadPersistant				= G_ReadPersistant;
	globals.WriteLevel					= G_WriteLevel;
	globals.ReadLevel					= G_ReadLevel;
	globals.LevelArchiveValid			= G_LevelArchiveValid;
	
	globals.inMultiplayerGame			= G_InMultiplayerGame;
	
	globals.ClientThink					= G_ClientThink;
	globals.ClientConnect				= G_ClientConnect;
	globals.ClientUserinfoChanged		= G_ClientUserinfoChanged;
	globals.ClientDisconnect			= G_ClientDisconnect;
	globals.ClientBegin					= G_ClientBegin;
	globals.ClientCommand				= G_ClientCommand;
	
	globals.ConsoleCommand				= G_ConsoleCommand;
	globals.RunFrame					= G_RunFrame;
	globals.SendEntity					= G_SendEntity;
	globals.UpdateEntityStateForClient	= G_UpdateEntityStateForClient;
	globals.UpdatePlayerStateForClient	= G_UpdatePlayerStateForClient;
	globals.ExtraEntitiesToSend			= G_ExtraEntitiesToSend;
	
	globals.GetEntityCurrentAnimFrame	= G_GetEntityCurrentAnimFrame;
	
	// the following are added for BOTLIB support
	globals.AddBot_f					= SV_AddBot_f;
	globals.BotAIStartFrame				= BotAIStartFrame;
	// end botlib additions
	
	globals.GetTotalGameFrames			= G_GetTotalGameFrames;
	
	globals.gentitySize					= sizeof(gentity_t);
	globals.error_message				= NULL;
	
	return &globals;
}

/*
=================
G_ClientEndServerFrames
=================
*/
void G_ClientEndServerFrames( void )
{
	int		i;
	gentity_t  *ent;
	
	// calc the player views now that all pushing
	// and damage has been added
	for( i = 0; i < maxclients->integer; i++ )
	{
		ent = g_entities + i;
		if ( !ent->inuse || !ent->client || !ent->entity )
		{
			continue;
		}
		
		ent->entity->ProcessEvent( EV_ClientEndFrame );
	}
}

//==================
//FindIntermissionPoint
//==================
void FindIntermissionPoint( void )
{
	Entity   *ent, *target;
	vec3_t	dir;
	
	// find the intermission spot
	ent = G_FindClass( NULL, "info_player_intermission" );
	
	if ( ent )
	{
		level.m_intermission_origin = ent->origin;
		level.m_intermission_angle  = ent->angles;
		
		// if it has a target, look towards it
		if ( ent->target )
		{
			target = G_FindTarget( NULL, ent->Target() );
			if ( target )
            {
				VectorSubtract( target->origin, level.m_intermission_origin, dir );
				vectoangles( dir, level.m_intermission_angle );
			}
		}
	   }
	else
	{
		level.m_intermission_origin = vec_zero;
		level.m_intermission_angle  = vec_zero;
	}
}

void G_MoveClientToIntermission( Entity *ent )
{
	assert( ent );
	
	if ( !ent )
		return;
	
	if ( ent->isClient() )
	{
		Player *player = ( Player * )ent;

		if ( multiplayerManager.inMultiplayer() && ( level.m_intermission_angle != vec_zero ) && ( level.m_intermission_origin != vec_zero ) )
		{
			// Each client should now be at the intermission spot
			player->setOrigin( level.m_intermission_origin );
			player->setAngles( level.m_intermission_angle  );
			player->SetViewAngles( level.m_intermission_angle  );
			player->CameraCut();
		}
		
		// Set a PMF flag to allow them to see the scoreboard
		player->client->ps.pm_flags |= PMF_INTERMISSION;
		player->flags |= FlagImmobile;
	}
}

// This is for multiplayer exiting
void G_BeginIntermission2( void )
{
	gentity_t   *client;
	Entity      *ent;
	int         i;
	
	if ( level.intermissiontime )
	{
		// already activated
		return;
	}
	
	// Save the time intermission started
	level.intermissiontime = level.time;
	// Freeze all players
	level.playerfrozen = true;
	// Find an intermission spot
	FindIntermissionPoint();
	
	// find an intermission spot
	ent = G_FindClass( NULL, "info_player_intermission" );
	
	// Only do the camera stuff if the node exists.
	if ( ent )
	{
		SetCamera( ent, CAMERA_SWITCHTIME );
	}
	
	// Display scores for all the clients
	for( i = 0; i < maxclients->integer; i++ )
	{
		client = g_entities + i;
		if ( !client->inuse )
		{
			continue;
		}
		
		ent = G_GetEntity( client->s.number );
		G_MoveClientToIntermission( ent );
	}
	
	// tell the script that the player's not ready so that if we return to this map,
	// we can do something about it.
	Director.PlayerNotReady();
}

void G_BeginIntermission( const char *map )
{
	gentity_t   *client;
	Entity      *ent;
	Entity      *path;
	int         i;
	Event       *event;
	
	assert( map );
	if ( !map )
	{
		gi.WDPrintf( "G_BeginIntermission : Null map name\n" );
		return;
	}
	
	if ( level.intermissiontime )
	{
		// already activated
		return;
	}
	
	
	if ( multiplayerManager.inMultiplayer() )
	{
		level.intermissiontime = level.time + mp_intermissionTime->integer;
	}
	else
	{
		level.intermissiontime = level.time + 1.0f;
		G_FadeOut( 1.0f );
		G_FadeSound( 1.0f );
	}
	
	level.nextmap = map;
	
	if ( gi.areSublevels( level.mapname, map ) || !level._showIntermission )
	{
		// We don't want a intermission (sublevel or something has specified no intermission )
		
		level.intermissiontime = level.time;
		level.exitintermission = true;
		
		Director.PlayerNotReady();
		
		return;
	}
	
	// find an intermission spot
	ent = G_FindClass( NULL, "info_player_intermission" );
	
	// Only do the camera stuff if the node exists.
	if ( ent )
	{
		SetCamera( ent, CAMERA_SWITCHTIME );
		event = new Event( EV_Camera_Orbit );
		
		// Find the end node
		path = G_FindTarget( NULL, "endnode1" );
		if ( path )
		{
			event->AddEntity( path );
			ent->ProcessEvent( event );
			event = new Event( EV_Camera_Cut );
			ent->ProcessEvent( event );
		}
	}
	
	// Display scores for all the clients
	for( i = 0; i < maxclients->integer; i++ )
	{
		client = g_entities + i;
		if ( !client->inuse )
		{
			continue;
		}
		
		ent = G_GetEntity( client->s.number );
		G_MoveClientToIntermission( ent );
	}
	
	// tell the script that the player's not ready so that if we return to this map,
	// we can do something about it.
	Director.PlayerNotReady();
}

/*
=============
G_ExitLevel
=============
*/
void G_ExitLevel( void )
{
	static const char *seps = " ,\n\r";
	char command[ 256 ];
	int j;
	gentity_t *ent;
	

	// Don't allow exit level if the mission was failed

	if ( level.mission_failed )
		return;

	// close the player log file if necessary
	ClosePlayerLogFile();
	
	// kill the sounds
	Com_sprintf( command, sizeof( command ), "stopsound\n" );
	gi.SendConsoleCommand( command );
	
	if ( multiplayerManager.inMultiplayer() )
	{
		if ( strlen( sv_nextmap->string ) )
		{
			// The nextmap cvar was set (possibly by a vote - so go ahead and use it)
			level.nextmap = sv_nextmap->string;
			gi.cvar_set( "nextmap", "" );
		}
		/* if ( strlen( sv_maplist->string ) ) // Use the next map in the maplist
		{
			char *s,*f,*t;
			
			f = NULL;
			s = strdup( sv_maplist->string );
			t = strtok( s, seps );
			while ( t != NULL )
            {
				if ( !stricmp( t, level.mapname.c_str() ) )
				{
					// it's in the list, go to the next one
					t = strtok( NULL, seps );
					if ( t == NULL ) // end of list, go to first one
					{
						if ( f == NULL ) // there isn't a first one, same level
						{
							level.nextmap = level.mapname;
						}
						else
						{
							level.nextmap = f;
						}
					}
					else
					{
						level.nextmap = t;
					}
					free( s );
					goto out;
				}
				
				// set the first map
				if (!f)
				{
					f = t;
				}
				t = strtok(NULL, seps);
            }
			free( s );
		} 
out: */
		// level.nextmap should be set now, but if it isn't use the same map
		if ( level.nextmap.length() == 0 )
		{
			// Stay on the same map since no nextmap was set
			Com_sprintf( command, sizeof( command ), "gamemap \"%s\"\n", level.mapname.c_str() );
			gi.SendConsoleCommand( command );
		}
		else // use the level.nextmap variable
		{
			Com_sprintf( command, sizeof( command ), "gamemap \"%s\"\n", level.nextmap.c_str() );
			gi.SendConsoleCommand( command );
		}
	}
	else
	{
		Com_sprintf( command, sizeof( command ), "gamemap \"%s\"\n", level.nextmap.c_str() );
		gi.SendConsoleCommand( command );
	}
	
	// Tell all the clients that the level is done
	for( j = 0; j < game.maxclients; j++ )
	{
		ent = &g_entities[ j ];
		if ( !ent->inuse || !ent->entity )
		{
			continue;
		}
		
		ent->entity->ProcessEvent( EV_Player_EndLevel );
	}

	level.nextmap = "";
	
	level.exitintermission = 0;
	level.intermissiontime = 0;
	
	G_ClientEndServerFrames();
	
	// tell the script that the player's not ready so that if we return to this map,
	// we can do something about it.
	Director.PlayerNotReady();
}

void G_CheckIntermissionExit( void )
{
	// Nobody clicked
	if ( !level.exitintermission )
	{
		return;
	}
	
	G_ExitLevel();
}

/*
================
G_RunFrame

Advances the world by one server frame
================
*/
extern "C" void G_RunFrame( int levelTime, int frameTime )
{
	gentity_t  *edict;
	Entity	*ent;
	int		num;
	qboolean showentnums;
	qboolean showactnums;
	int      start;
	int      end;
	
	try
	{
		if ( level.restart )
		{
			level.Restart();
		}
		
		level.update(levelTime, frameTime);
		
		if ( g_showmem->integer )
		{
			DisplayMemoryUsage();
		}
		
		// exit intermissions
		if ( level.exitintermission )
		{
			G_ExitLevel();
			return;
		}
		
		// Reset debug lines
		G_InitDebugLines();
		
		// testing coordinate system
		if ( csys_draw->integer )
		{
			G_DrawCSystem();
		}
		
		thePathManager.ShowNodes();
		
		AI_DisplayInfo();
		
		// don't show entnums during deathmatch
		showentnums = ( sv_showentnums->integer && ( !multiplayerManager.inMultiplayer() || sv_cheats->integer ) );
		showactnums = ( sv_showactnums->integer && ( !multiplayerManager.inMultiplayer() || sv_cheats->integer ) );
		
		// Wake up any monsters in the area
		AI_SenseEnemies();
		
		// Process most of the events before the physics are run
		// so that we can affect the physics immediately
		L_ProcessPendingEvents();
		
		//
		// treat each object in turn
		//
		for( edict = active_edicts.next, num = 0; edict != &active_edicts; edict = level.next_edict, num++ )
		{
			assert( edict );
			assert( edict->inuse );
			assert( edict->entity );
			
			level.next_edict = edict->next;
			
			// Paranoia - It's a way of life
			assert( num <= MAX_GENTITIES );
			if ( num > MAX_GENTITIES )
            {
				gi.WDPrintf( "Possible infinite loop in G_RunFrame.\n");
				break;
            }
			
			ent = edict->entity;
			if ( g_timeents->integer )
            {
				start = gi.Milliseconds();
				G_RunEntity( ent );
				end = gi.Milliseconds();
				
				if ( g_timeents->value <= (float)( end - start ) )
				{
					gi.DPrintf( "%d: '%s'(%d) : %d\n", level.framenum, ent->targetname.c_str(), ent->entnum, end - start );
				}
            }
			else
            {
				G_RunEntity( ent );
            }
			
			if ( ( edict->svflags & SVF_SENDONCE ) && ( edict->svflags & SVF_SENT ) )
            {
				// Entity has been sent once, and is marked as such, then remove it
				ent->PostEvent( EV_Remove, 0.0f );
            }
			
			if ( showentnums )
			{
				G_DrawDebugNumber( ent->origin + Vector( 0.0f, 0.0f, ent->maxs.z + 2.0f ), ent->entnum, 2.0f, 1.0f, 1.0f, 0.0f );
			}
			if ( showactnums && ent->isSubclassOf( Actor ) )
			{
				G_DrawDebugNumber( ent->origin + Vector( 0.0f, 0.0f, ent->maxs.z + 2.0f ), ent->entnum, 2.0f, 1.0f, 0.0f, 0.0f );
			}
		}
		
		multiplayerManager.update( level.frametime );
		
		// Process any pending events that got posted during the physics code.
		L_ProcessPendingEvents();
		
		// build the playerstate_t structures for all players
		G_ClientEndServerFrames();
		
		// see if we should draw the bounding boxes
		G_ClientDrawBoundingBoxes();
		
		// see if we should draw all of the splines 
		G_ClientDrawSplines();
		
		// show how many traces the game code is doing
		if ( sv_traceinfo->integer )
		{
			if ( sv_traceinfo->integer == 3 )
			{
				gi.DebugPrintf( "%0.2f : Total traces %d\n", level.time, sv_numtraces );
			}
			else
			{
				gi.DPrintf( "%0.2f : Total traces %d\n", level.time, sv_numtraces );
			}
		}
		
		// reset out count of the number of game traces
		sv_numtraces = 0;
		
		level.framenum++;
		
		// we increment time so that events that occurr before we think again are automatically on the next frame
		levelTime += frameTime;
		level.setTime( levelTime, frameTime );
	}
	  
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

extern "C" qboolean G_SendEntity( gentity_t *clientEntity, gentity_t *entityToSend )
{
	try
	{
		if ( clientEntity->entity && clientEntity->entity->isSubclassOf( Player ) && entityToSend->entity )
		{
			Player *player;
			
			player = ( Player *)clientEntity->entity;
			
			return player->ShouldSendToClient( entityToSend->entity );
		}
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
	
	return false;
}

extern "C" void G_UpdateEntityStateForClient( gentity_t *clientEntity, entityState_t *state )
{
	try
	{
		if ( clientEntity->entity && clientEntity->entity->isSubclassOf( Player ) )
		{
			Player *player;
			
			player = ( Player *)clientEntity->entity;
			
			player->UpdateEntityStateForClient( state );
		}
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
	
}

extern "C" void G_UpdatePlayerStateForClient( gentity_t *clientEntity, playerState_t *state )
{
	try
	{
		if ( clientEntity->entity && clientEntity->entity->isSubclassOf( Player ) )
		{
			Player *player;
			
			player = ( Player *)clientEntity->entity;
			
			player->UpdatePlayerStateForClient( state );
		}
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
	
}

extern "C" void G_ExtraEntitiesToSend( gentity_t *clientEntity, int *numExtraEntities, int *extraEntities )
{
	try
	{
		*numExtraEntities = 0;
		
		if ( !multiplayerManager.inMultiplayer() && clientEntity->entity && clientEntity->entity->isSubclassOf( Player ) )
		{
			Player *player;
			
			player = ( Player *)clientEntity->entity;
			
			player->ExtraEntitiesToSendToClient( numExtraEntities, extraEntities );
			
			//add the extra entities from the extra entity list...
			for(int i = 1; i <= extraEntitiesList.NumObjects(); i++)
			{
				extraEntities[*numExtraEntities] = extraEntitiesList.ObjectAt(i);
				(*numExtraEntities)++;
			}
		}
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

void G_AddEntityToExtraList( int entityNum )
{
	extraEntitiesList.AddUniqueObject( entityNum );
}


void G_RemoveEntityFromExtraList( int entityNum )
{
	int index;

	index = extraEntitiesList.IndexOfObject( entityNum );

	if ( index > 0 )
	{
		extraEntitiesList.RemoveObjectAt( index );
	}
}

extern "C" int G_GetEntityCurrentAnimFrame( int entnum, int bodyPart )
{
	try
	{
		if ( entnum < 0 || entnum >= ENTITYNUM_NONE )
		{
			return -1;
		}

		if ( !g_entities[ entnum ].inuse || !g_entities[ entnum ].entity )
		{
			return -1;
		}

		return g_entities[ entnum ].entity->CurrentFrame( (bodypart_t)bodyPart );
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}

	return -1;
}

extern "C" void G_ClientThink( gentity_t *ent, usercmd_t *ucmd )
{
	try
	{
		if ( ent->entity )
		{
			current_ucmd = ucmd;
			ent->entity->ProcessEvent( EV_ClientMove );
			current_ucmd = NULL;
		}
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}


void G_SentInitialMessages( void )
{
}


/*
===========
G_ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
extern "C" void G_ClientBegin( gentity_t *ent, const usercmd_t *cmd )
{
	Q_UNUSED(cmd);

	try
	{
		if ( ent->inuse && ent->entity )
		{
			// the client has cleared the client side viewangles upon
			// connecting to the server, which is different than the
			// state when the game is saved, so we need to compensate
			// with deltaangles
			if( ent->client->ps.pm_type != PM_SECRET_MOVE_MODE )
				ent->entity->SetDeltaAngles();
		}
		else
		{
			Player *player;
			
			// a spawn point will completely reinitialize the entity
			level.spawn_entnum = ent->s.number;
			player = new Player;
		}
		
		if ( level.intermissiontime && ent->entity )
		{
			G_MoveClientToIntermission( ent->entity );
		}
		else
		{
			// Record the time entered
			ent->client->pers.enterTime = level.time;
			// send effect if in a multiplayer game
			if ( game.maxclients > 1 )
			{
				gi.Printf ( "%s entered the game\n", ent->client->pers.netname );
			}
		}
		
		// make sure all view stuff is valid
		if ( ent->entity )
		{
			ent->entity->ProcessEvent( EV_ClientEndFrame );
			
			if ( !Director.PlayerReady() )
            {
				// let any threads waiting on us know they can go ahead
				Director.PlayerSpawned();
            }
			
			GameplayManager::getTheGameplayManager()->processPendingMessages();
			G_SentInitialMessages( );
		}
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}



qboolean G_duplicateName( const char *name )
{
	gentity_t *edict;
	int i;

	for ( i = 0; i < game.maxclients; i++ )
	{
		edict = &g_entities[i];
		
		if ( !edict->inuse || !edict->entity || !edict->client )
			continue;

		if ( strcmp( edict->client->pers.netname, name ) == 0 )
		{
			return true;
		}

	}

	return false;
}

void G_BuildUniqueName( const char *newName, char *name )
{
	char builtName[ MAX_NETNAME ];
	int i;

	for ( i = 2 ; i <= 9 ; i++ )
	{
		sprintf( builtName, "%s%d", newName, i );

		if ( !G_duplicateName( builtName ) )
		{
			strcpy( name, builtName );
			return;
		}
	}

	sprintf( builtName, "%s?", newName );

	strcpy( name, builtName );
}

/*
===========
G_ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
extern "C" void G_ClientUserinfoChanged( gentity_t *ent, const char *userinfo )
{
	const char	*s;
	int			playernum;
	Player      *player;
	//float       fov;
	//Event       *ev;
	bool			autoSwitchWeapons;
	char tempName[ MAX_NETNAME ];
	bool validName;
	
	try
	{
		if ( !ent )
		{
			assert( 0 );
			return;
		}
		
		player = ( Player * )ent->entity;
		
		/*
		if ( !player )
		{
		assert( 0 );
		return;
		}
		*/
		
		// set name
		s = Info_ValueForKey( userinfo, "name" );
		if ( !s )
		{
			assert( 0 );
			return;
		}

		// Setup the name

		strncpy( tempName, s, MAX_NETNAME - 1 - 3 );

		// Make sure there is a valid name

		validName = false;

		for ( unsigned i = 0 ; i < strlen( tempName ) ; i++ )
		{
			if ( ( tempName[ i ] == '^' ) && ( tempName[ i + 1 ] >= '0' ) && ( tempName[ i + 1 ] <= '9' ) )
			{
				i++;
				continue;
			}

			validName = true;
		}

		if ( !validName )
		{
			strcpy( tempName, "RedShirt" );
		}

		if ( strcmp( ent->client->pers.netname, tempName ) != 0 )
		{
			if ( G_duplicateName( tempName ) )
				G_BuildUniqueName( tempName, ent->client->pers.netname );
			else
				strncpy( ent->client->pers.netname, tempName, MAX_NETNAME - 1 - 3 );
		}

		// Strip out bad characters

		for ( unsigned i = 0 ; i < strlen( ent->client->pers.netname ) ; i++ )
		{
			if ( ent->client->pers.netname[ i ] == ':' )
			{
				ent->client->pers.netname[ i ] = '.';
			}
		}

		// Make sure color returns to normal at end of the name

		if ( strstr( ent->client->pers.netname, "^" ) )
		{
			int length = strlen( ent->client->pers.netname );

			if ( ( length < 2 ) || ( ent->client->pers.netname[ length - 2 ] != '^' ) || ( ent->client->pers.netname[ length - 1 ] != '8' ) )
			{
				ent->client->pers.netname[ length     ] = '^';
				ent->client->pers.netname[ length + 1 ] = '8';
				ent->client->pers.netname[ length + 2 ] = 0;
			}
		}
		
		// set deathmatch model
		s = Info_ValueForKey( userinfo, "mp_playermodel" );
		if ( !s )
		{
			assert( 0 );
			return;
		}
		
		multiplayerManager.changePlayerModel( player, s );
		
		strncpy( ent->client->pers.mp_playermodel, s, sizeof( ent->client->pers.mp_playermodel ) - 1 );
		
		// Low bandwidth
		s = Info_ValueForKey( userinfo, "mp_lowBandwidth" );
		if ( !s )
		{
			assert( 0 );
			return;
		}

		ent->client->pers.mp_lowBandwidth = atoi( s );

		// Persistant morph controllers
		s = Info_ValueForKey( userinfo, "dm_morph_c1" );
		if ( !s )
		{
			assert( 0 );
			return;
		}
		strncpy( ent->client->pers.dm_morph_c1, s, sizeof( ent->client->pers.dm_morph_c1 ) - 1 );
		
		// send over a subset of the userinfo keys so other clients can
		// print scoreboards, display models, and play custom sounds
		playernum = ent - g_entities;
		gi.setConfigstring( CS_PLAYERS + playernum, va( "name\\%s", ent->client->pers.netname ) );
		//gi.setConfigstring( CS_PLAYERS + playernum, va( "%s", ent->client->pers.netname ) );

		multiplayerManager.changePlayerName( player, ent->client->pers.netname );
		
		if ( player )
		{
			float fov;
			Event *ev;

			// Fov
			
			fov = (float)atof( Info_ValueForKey( userinfo, "userFov" ) );
			if ( fov < 1.0f )
            {
				fov = sv_defaultFov->value;
            }
			else if ( fov > 160.0f )
            {
				fov = 160.0f;
            }

			player->userFov = fov;

			ev = new Event( EV_Player_Fov );
			ev->AddFloat( fov );
			player->ProcessEvent( ev );
			
			// autoSwitchWeapons
			
			if ( atoi( Info_ValueForKey( userinfo, "mp_autoSwitchWeapons" ) ) )
				autoSwitchWeapons = true;
			else
				autoSwitchWeapons = false;
			
			player->setAutoSwitchWeapons( autoSwitchWeapons );

			// Saving demo
			s = Info_ValueForKey( userinfo, "mp_savingDemo" );

			if ( !s )
			{
				assert( 0 );
				return;
			}

			player->mp_savingDemo = atoi( s );
		}
		
		// save off the userinfo in case we want to check something later
		strncpy( ent->client->pers.userinfo, userinfo, sizeof( ent->client->pers.userinfo ) - 1 );

		Info_SetValueForKey(ent->client->pers.userinfo, "name", ent->client->pers.netname);
		gi.setUserinfo(playernum, ent->client->pers.userinfo);
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

qboolean G_BotConnect( int clientNum, qboolean restart );

/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be true the very first time a client connects
to the server machine, but false on map changes and tournement
restarts.
============
*/
extern "C" const char *G_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot, qboolean checkPassword )
{
	const char *value;
	gentity_t	*ent;
	char		   userinfo[ MAX_INFO_STRING ];
	gclient_t	*client;
	
	try
	{
		ent = &g_entities[ clientNum ];
		gi.getUserinfo( clientNum, userinfo, sizeof( userinfo ) );
		
		// check to see if they are on the banned IP list
		value = Info_ValueForKey( userinfo, "ip" );
		if ( SV_FilterPacket( value ) )
		{
			return "$$BannedIP$$";
		}
		
		if ( checkPassword )
		{
			// check for a password
			value = Info_ValueForKey( userinfo, "password" );
			if ( strcmp( password->string, value ) != 0 )
			{
				return "$$InvalidPassword$$";
			}
		}
		
		// they can connect
		ent->client = game.clients + clientNum;
		client = ent->client;
		
		// read or initialize the session data
		// if there is already a body waiting for us (a loadgame), just
		// take it, otherwise spawn one from scratch
		if ( firstTime && !ent->entity )
		{
			memset( client, 0, sizeof( *client ) );
			
			// clear the respawning variables
			if ( !game.autosaved )
            {
				G_InitClientPersistant( client );
				playersLastTeam[ clientNum ] = "";
            }
		}
		
		
		if( isBot ) {
			ent->svflags |= SVF_BOT;
			//ent->inuse = qtrue;
			if( !G_BotConnect( clientNum, !firstTime ) ) {
				return "BotConnectfailed";
			}
		}
		
		
		
		G_ClientUserinfoChanged( ent, userinfo );
		
		if ( firstTime && ( game.maxclients > 1 ) )
		{
			gi.Printf( "%s connected\n", ent->client->pers.netname );
		}
		
		LoadingServer = false;
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
	
	return NULL;
}

/*
===========
G_ClientDisconnect

called when a player drops from the server

============
*/
extern "C" void G_ClientDisconnect( gentity_t *ent )
{
	try
	{
		if ( !ent || ( !ent->client ) || ( !ent->entity ) )
		{
			return;
		}
		
		Player *player;
		
		player = ( Player * )ent->entity;
		player->Disconnect();
		
		delete ent->entity;
		ent->entity = NULL;
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

/*
=================
G_ClientDrawSplines
=================
*/
void G_ClientDrawSplines( void )
{
	int i;
	Entity *entity;
	gentity_t *ed;
	
	
	if ( !sv_showsplines->integer )
		return;
	
	for ( i = 0; i < MAX_GENTITIES; i++ )
	{
		ed = &g_entities[i];
		
		if ( !ed->inuse || !ed->entity )
			continue;
		
		entity = g_entities[i].entity;
		
		if ( entity->isSubclassOf( SplinePath ) )
		{
			SplinePath *path = ( SplinePath *)entity;
			
			SplinePath	*node;
			BSpline	*splinePath;
			
			splinePath = new BSpline;
			splinePath->SetType( SPLINE_CLAMP );
			
			node = path;
			
			while( node != NULL )
			{
				splinePath->AppendControlPoint( node->origin, node->angles, node->speed );
				
				node = node->GetNext();
				
				if ( node == path )
					break;
			}
			
			G_Color3f( 1.0f, 1.0f, 0.0f );
			splinePath->DrawCurve( 10 );
			
			delete splinePath;
			continue;
		}
	}
}

/*
=================
G_ClientDrawBoundingBoxes
=================
*/
void G_ClientDrawBoundingBoxes( void )
{
	gentity_t  *edict;
	Entity	*ent;
	Vector	eye;
	
	// don't show bboxes during deathmatch
	if (
        ( !g_showgravpath->integer && !sv_showbboxes->integer ) ||
        ( multiplayerManager.inMultiplayer() && !sv_cheats->integer )
		)
	{
		return;
	}
	
	if ( sv_showbboxes->integer )
	{
		edict = g_entities;
		ent = edict->entity;
		if ( ent )
		{
			eye = ent->origin;
			ent = findradius( NULL, eye, 1000.0f );
			while( ent )
			{
				switch ((int)sv_showbboxes->integer)
				{
				case 1:
					if ( ent->edict != edict && ent->edict->s.solid)
					{
						G_DebugBBox( ent->origin, ent->mins, ent->maxs, 1.0f, 1.0f, 0.0f, 1.0f );
					}
					break;
				case 2:
					if ( ent->edict != edict && ent->edict->s.solid)
					{
						G_DebugBBox( vec_zero, ent->edict->absmin, ent->edict->absmax, 1.0f, 0.0f, 1.0f, 1.0f );
					}
					break;
				case 3:
					if ( ent->edict->s.modelindex && !(ent->edict->s.renderfx & RF_DONTDRAW) )
						G_DebugBBox( ent->origin, ent->mins, ent->maxs, 1.0f, 1.0f, 0.0f, 1.0f );
					break;
				case 4:
					G_DebugBBox( ent->origin, ent->mins, ent->maxs, 1.0f, 1.0f, 0.0f, 1.0f );
					break;
				case 5:
				default:
					if ( ent->animate && gi.IsModel( ent->edict->s.modelindex ) )
					{
						vec3_t mins, maxs;
						
						gi.Frame_Bounds( ent->edict->s.modelindex, ent->CurrentAnim(), ent->CurrentFrame(), ent->edict->s.scale, mins, maxs );
						G_DebugBBox( ent->origin, mins, maxs, 0.0f, 1.0f, 0.0f, 1.0f );
					}
					else
					{
						G_DebugBBox( ent->origin, ent->mins, ent->maxs, 1.0f, 1.0f, 0.0f, 1.0f );
					}
					break;
				}
				ent = findradius( ent, eye, 1000.0f );
			}
		}
	}
	
	if ( g_showgravpath->integer )
	{
		// Draw the gravity node paths
		gravPathManager.DrawGravPaths();
	}
}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
extern "C" int G_GetTotalGameFrames( void )
{
	Player* player = GetPlayer(0);

	if(player != 0)
		return player->getTotalGameFrames();

	return 0;
}

//======================================================================

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Com_Error ( int level, const char *error, ... ) 
{
	va_list	argptr;
	char		text[4096];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	gi.Error( level, "%s", text);
}

void Sys_Error( const char *error, ... )
{
	va_list	argptr;
	char		text[4096];
	
	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);
	
	gi.Error (ERR_FATAL, "%s", text);
}

void Com_Printf( const char *msg, ... )
{
	va_list	argptr;
	char		text[4096];
	
	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);
	
	gi.DPrintf ("%s", text);
}

void Com_WPrintf( const char *msg, ... )
{
	va_list	argptr;
	char		text[4096];
	
	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);
	
	gi.WDPrintf ("%s", text);
}

#endif
