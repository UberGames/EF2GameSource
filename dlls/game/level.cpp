//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/level.cpp                                 $
// $Revision:: 90                                                             $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#include "_pch_cpp.h"
#include "level.h"
#include "scriptmaster.h"
#include "navigate.h"
#include "helper_node.h"
#include "gravpath.h"
#include "g_spawn.h"
#include "player.h"
#include "characterstate.h"
#include "mp_manager.hpp"
#include "armor.h"
#include "CinematicArmature.h"
#include "earthquake.h"
#include "teammateroster.hpp"

Level level;

extern Container<int> SpecialPathNodes;

CLASS_DECLARATION( Class, Level, NULL )
{
	{ NULL, NULL }
};

Level::Level()
{
	Init();
}

Level::~Level()
{
   _earthquakes.FreeObjectList();
}

void Level::Init( void )
{
	spawn_entnum = -1;
	
	restart = false;;
	
	framenum = 0;
	time     = 0;
	frametime = 0;
	
	level_name = "";
	mapname = "";
	spawnpoint = "";
	nextmap = "";
	
	playerfrozen = false;
	intermissiontime = 0;
	exitintermission = 0;
	
	next_edict = NULL;
	
	total_secrets = 0;
	found_secrets = 0;
	total_specialItems = 0;
	found_specialItems = 0;

	_totalEnemiesSpawned = 0;
	
	memset( &impact_trace, 0, sizeof( impact_trace ) );
	
	cinematic		= false;
	ai_on			= true;
	
	mission_failed	= false;
	died_already	= false;
	near_exit		= false;
	started			= false;
	
	water_color = vec_zero;
	water_alpha = 0;
	
	slime_color = vec_zero;
	slime_alpha = 0;
	
	lava_color  = vec_zero;
	lava_alpha = 0;
	
	saved_soundtrack = "";
	current_soundtrack = "";
	
	consoleThread = NULL;
	
	
	// clear out automatic cameras
	automatic_cameras.ClearObjectList();
	
	// init level script variables
	levelVars.ClearList();
	
	m_fade_time_start             = 0;
	m_fade_time                   = -1;
	m_fade_color                  = vec_zero;
	m_fade_alpha                  = 0;
	m_fade_style                  = additive;
	m_fade_type                   = fadein;
	m_letterbox_fraction          = 0;
	m_letterbox_time              = -1;
	m_letterbox_time_start        = 0;
	m_letterbox_dir               = letterbox_out;
	
	hNodeController = NULL;
	
    _cleanup = false;
	
	_showIntermission = true;

	 _saveOrientation = true;

	 currentInstanceNumber = 0;
}

//-----------------------------------------------------
//
// Name:		update
// Class:		Level
//
// Description:	Updates the level variables. This is called on the frame update.
//
// Parameters:	levelTime - the game clock time.
//				frameTime - the time the frame occured.
//
// Returns:		None
//-----------------------------------------------------
void Level::update( int levelTime, int frameTime )
{
	setTime(levelTime, frameTime);

	if ( level.intermissiontime && dedicated->integer )
	{
		if ( g_endintermission->integer > 0 )
		{
			g_endintermission->integer = 0;
			level.exitintermission = true;
		}

		// can exit intermission after 10 seconds (default)

		if ( ( ( level.time - level.intermissiontime ) > level.intermission_advancetime ) &&
			 ( level.intermission_advancetime != 0 ) )
		{
			if ( multiplayerManager.inMultiplayer() )
			{
				level.exitintermission = true;
			}
		}
	}
}


void Level::SetIntermissionAdvanceTime(float time)
{
	intermission_advancetime = time;
}

void Level::EndIntermission()
{
	exitintermission = true;
}

void Level::CleanUp( qboolean restart )
{
	_cleanup = true;
	
	if ( multiplayerManager.inMultiplayer() )
	{
		multiplayerManager.cleanup( restart );
	}
	else
	{
		Player* p = GetPlayer(0);
		if( p )	p->LevelCleanup();
	}

	ClearCachedStatemaps();
	ClearCachedFuzzyEngines();  
	_playerDeathThread = "";
#ifndef DEDICATED
	gi.MObjective_ClearObjectiveList();
#endif
	
	AdaptiveArmor::ClearAdaptionList(); 
	theCinematicArmature.clearCinematicsList();
	
	HelperNode::CleanupHelperNodeList();
	
	assert( active_edicts.next );
	assert( active_edicts.next->prev == &active_edicts );
	assert( active_edicts.prev );
	assert( active_edicts.prev->next == &active_edicts );
	assert( free_edicts.next );
	assert( free_edicts.next->prev == &free_edicts );
	assert( free_edicts.prev );
	assert( free_edicts.prev->next == &free_edicts );
	
	while( active_edicts.next != &active_edicts )
	{
		assert( active_edicts.next != &free_edicts );
		assert( active_edicts.prev != &free_edicts );
		
		assert( active_edicts.next );
		assert( active_edicts.next->prev == &active_edicts );
		assert( active_edicts.prev );
		assert( active_edicts.prev->next == &active_edicts );
		assert( free_edicts.next );
		assert( free_edicts.next->prev == &free_edicts );
		assert( free_edicts.prev );
		assert( free_edicts.prev->next == &free_edicts );
		
		if ( active_edicts.next->entity )
		{
			delete active_edicts.next->entity;
		}
		else
		{
			FreeEdict( active_edicts.next );
		}
	}
	
	cinematic		= false;
	ai_on			= true;
	
	mission_failed	= false;
	died_already	= false;
	near_exit		= false;
	started			= false;
	
	globals.num_entities = game.maxclients + 1;
	
	// clear up all AI node information
	thePathManager.ResetNodes();
	
	// Reset the gravity paths
	gravPathManager.Reset();
	
	if ( consoleThread )
	{
		Director.KillThread( consoleThread->ThreadNum() );
		consoleThread = NULL;
	}
	
	// close all the scripts
	Director.CloseScript();
	
	// invalidate player readiness
	Director.PlayerNotReady();
	
	// clear out automatic cameras
	automatic_cameras.ClearObjectList();
	
	// clear out level script variables
	levelVars.ClearList();
	
	// initialize the game variables
	// these get restored by the persistant data, so we can safely clear them here
	gameVars.ClearList();
	
	// clearout any waiting events
	L_ClearEventList();
	
	ResetEdicts();
	
	// Reset the boss health cvar
	gi.cvar_set( "bosshealth", "0" );
	
	_earthquakes.ClearObjectList();
	
	_cleanup = false;
}

/*
==============
ResetEdicts
==============
*/
void Level::ResetEdicts( void )
{
	int i;
	
	memset( g_entities, 0, game.maxentities * sizeof( g_entities[ 0 ] ) );
	
	// Add all the edicts to the free list
	LL_Reset( &free_edicts, next, prev );
	LL_Reset( &active_edicts, next, prev );
	for( i = 0; i < game.maxentities; i++ )
	{
		LL_Add( &free_edicts, &g_entities[ i ], next, prev );
	}
	
	for( i = 0; i < game.maxclients; i++ )
	{
		//char savedTeamName[ 16 ];

		// set client fields on player ents
		g_entities[ i ].client = game.clients + i;
		
		//strcpy( savedTeamName, game.clients[i].pers.lastTeam );

		G_InitClientPersistant (&game.clients[i]);

		//strcpy( game.clients[i].pers.lastTeam, savedTeamName );
	}
	
	globals.num_entities = game.maxclients;
}

/*
==============
Start

Does all post-spawning setup.  This is NOT called for savegames.
==============
*/
void Level::Start( void )
{
	CThread *gamescript;

	// initialize secrets

	levelVars.SetVariable( "total_secrets", total_secrets );
	levelVars.SetVariable( "found_secrets", found_secrets );
	levelVars.SetVariable( "total_specialItems" , total_specialItems );
	levelVars.SetVariable( "found_specialItems" , found_specialItems );
	levelVars.SetVariable( "total_enemies_spawned", _totalEnemiesSpawned );
	

	FindTeams();

	// call the precache scripts
	
	Precache();

	// start executing the game script

	if ( game_script.length() )
	{
		gi.ProcessLoadingScreen( "$$LoadingScript$$" );

		program.Load( game_script );

		gi.ProcessLoadingScreen( "$$DoneLoadingScript$$" );

		// Create the main thread

		gamescript = Director.CreateThread( "main" );

		if ( gamescript )
		{
			// Run the precache thread if it exists

			if ( gamescript->labelExists( "precache" ) )
			{
				CThread *precache_script;
				precache_script = Director.CreateThread( "precache" );

				if ( precache_script )
					precache_script->DelayedStart( 0.0f );
			}

			// Run the main thread

			gamescript->DelayedStart( 0.0f );
		}
	}

	loadLevelStrings();
	started = true;
}

//----------------------------------------------------------------
// Name:			postLoad
// Class:			
//
// Description:		Does everything necessary to the level after a load has happened
//
// Parameters:		None
//
// Returns:			none
//----------------------------------------------------------------

void Level::postLoad( void )
{
	thePathManager.FindAllTargets();
	TeammateRoster::getInstance()->clearTeammates();
 	//thePathManager.InsertNodesIntoGrid();
 	//thePathManager.ConnectPathNodes();
	//thePathManager.OptimizeNodes( NULL );

	thePathManager.SavePaths();
}

//----------------------------------------------------------------
// Name:			postSublevelLoad
// Class:			Level
//
// Description:		Does everything necessary to the level after a sublevel has been loaded
//
// Parameters:		const char *mapName			- map name string (also possibly contains spawn position)
//
// Returns:			none
//----------------------------------------------------------------

void Level::postSublevelLoad( const char *spawnPosName )
{
	int i;
	gentity_t *ent;
	Player *player;


	// Save off the spawn position

	spawnpoint = spawnPosName;

	// Make sure the player starts in the correct place

	for( i = 0; i < game.maxclients; i++ )
	{
		ent = &g_entities[ i ];

		if ( !ent->inuse || !ent->client || !ent->entity )
			continue;

		if ( ent->entity->isSubclassOf( Player ) )
		{
			player = (Player *)ent->entity;

			player->ChooseSpawnPoint();
		}
	}


	// Get rid of any fading

	G_ClearFade();
}

qboolean Level::inhibitEntity( int spawnflags )
{
	if ( !developer->integer && ( spawnflags & SPAWNFLAG_DEVELOPMENT ) )
	{
		return true;
	}
	
	if ( !detail->integer && ( spawnflags & SPAWNFLAG_DETAIL ) )
	{
		return true;
	}
	
	if ( multiplayerManager.inMultiplayer() )
	{
		if ( spawnflags & SPAWNFLAG_NOT_DEATHMATCH )
		{
			return true;
		}
		
		return false;
	}
	
	switch( skill->integer )
	{
	case 0 :
		return ( spawnflags & SPAWNFLAG_NOT_EASY ) != 0;
		break;
		
	case 1 :
		return ( spawnflags & SPAWNFLAG_NOT_MEDIUM ) != 0;
		break;
		
	case 2 :
	case 3 :
		return ( spawnflags & SPAWNFLAG_NOT_HARD );
		break;
	}
	
	return false;
}

void Level::setSkill( int value )
{
	int skill_level;
	
	skill_level = (int) floor( (float)value );
	skill_level = bound( skill_level, 0, 3 );
	
	gi.cvar_set( "skill", va( "%d", skill_level ) );
	
	gameVars.SetVariable( "skill", skill_level );
}

int Level::getSkill( void )
{
	ScriptVariable* skill_var = gameVars.GetVariable("skill");
	return  skill_var->intValue();
}

void Level::setTime( int levelTime, int frameTime )
{
	inttime        = levelTime;
	fixedframetime = 1.0f / sv_fps->value;
	frametime      = ( ( float )frameTime / 1000.0f );
	time           = ( ( float )levelTime / 1000.0f );
	
	if(intermissiontime == 0.0f && mission_failed == false)
		timeInLevel = time;
}

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void Level::SpawnEntities( const char *themapname, const char *entities, int levelTime )
{
	int			inhibit,count=0;
	const char	*value;
	SpawnArgs   args;
	char        *spawnpos;
	
	// Init the level variables
	Init();
	
	spawnpos = (char *)strchr( themapname, '$' );
	if ( spawnpos )
	{
		mapname = str( themapname, 0, spawnpos - themapname );
		spawnpoint = spawnpos + 1;
	}
	else
	{
		mapname = themapname;
		spawnpoint = "";
	}
	
	// set up time so functions still have valid times
	setTime( levelTime, 1000 / 20 );
	
	if ( !LoadingServer )
	{
		// Get rid of anything left over from the last level
		//CleanUp( false );
		
		// Set up for a new map
		thePathManager.Init( mapname );
		
	}
	
	
	setSkill( skill->integer );
	
	// reset out count of the number of game traces
	sv_numtraces = 0;
	
	// parse world
	entities = args.Parse( entities );
	spawn_entnum = ENTITYNUM_WORLD;
	args.Spawn();
	
	if ( !world )
		Com_Error( ERR_FATAL, "No world\n" );
	
	if ( g_gametype->integer == GT_MULTIPLAYER || g_gametype->integer == GT_BOT_SINGLE_PLAYER )
	{
		multiplayerManager.initMultiplayerGame();
	}
	
	// parse ents
	inhibit = 0;
	for( entities = args.Parse( entities ); entities != NULL; entities = args.Parse( entities ) )
	{
		// remove things (except the world) from different skill levels or deathmatch
		spawnflags = 0;
		value = args.getArg( "spawnflags" );
		if ( value )
		{
			spawnflags = atoi( value );
			if ( inhibitEntity( spawnflags ) )
            {
				inhibit++;
				continue;
			}
		}
		
		args.Spawn();
		count++;

		gi.ProcessLoadingScreen( "$$SpawningEntities$$" );
	}
	
	gi.DPrintf( "%i entities spawned\n", count );
	gi.DPrintf( "%i entities inhibited\n", inhibit );
	
	// Process the spawn events
	L_ProcessPendingEvents();
	
	if ( multiplayerManager.inMultiplayer() )
	{
		multiplayerManager.initItems();
	}

	// Setup bots

	if ( gi.Cvar_VariableIntegerValue( "bot_enable" ) && multiplayerManager.inMultiplayer() ) 
	{
		BotAIShutdown( 0 );

		BotAISetup( 0 );
		BotAILoadMap( 0 );
		G_InitBots( 0 );
	}
	
	if ( !LoadingServer || game.autosaved )
	{
		Start();
	}
	
	postLoad();
   	
	
	//-------------------------------------------------------------------------------
    //
    // Deletion Note:
    // Since hNodeController is an Entity, it is deleted
    // when all the other entities are deleted in the clean up function
    // specifically the line
    //
	//
   	//	if ( active_edicts.next->entity )
	//		{
	//		delete active_edicts.next->entity;
	//		}
    //
	//
    // Since it is already being deleted like this
    // We do not need to explcitily delete the controller... In fact
    // you will error out if you try to.
	//--------------------------------------------------------------------------------
	hNodeController = new HelperNodeController;
	if ( hNodeController )
		hNodeController->SetTargetName( "HelperNodeController" );
	
	
	
	//
	// if this is a single player game, spawn the single player in now
	// this allows us to read persistant data into the player before the client
	// is completely ready
	//
	if ( game.maxclients == 1 )
	{
		spawn_entnum = 0;
		new Player;
	}
}

void Level::NewMap( const char *mapname, const char *entities, int levelTime )
{
	theCinematicArmature.deleteAllCinematics();
	current_map = mapname;
	current_entities = entities;
	
	SpawnEntities( current_map, current_entities, levelTime );
}

void Level::Restart( void )
{
	theCinematicArmature.deleteAllCinematics();
	
	SpawnEntities( current_map, current_entities, inttime );
	
	L_ProcessPendingEvents();
	
	G_ClientConnect( 0, true, false, true );
	G_ClientBegin( &g_entities[ 0 ], NULL );
}

void Level::PlayerRestart( void )
{
	// we need to restart through the server code
	gi.SendConsoleCommand( "restart\n" );
	//restart = true;
	level.mission_failed = false;
}

void Level::Archive( Archiver &arc )
{
	int num;
	int i;

	Class::Archive( arc );

	if ( arc.Saving() )
	{
		SafePtr<Earthquake> ent;

		num = _earthquakes.NumObjects();
		arc.ArchiveInteger( &num );

		for ( i = 1 ; i <= num ; i++ )
		{
			ent = _earthquakes.ObjectAt( i );		
			arc.ArchiveSafePointer( &ent );		
		}
	}
	else
	{
		SafePtr<Earthquake> ent;
		SafePtr<Earthquake> *entityPointer;

		arc.ArchiveInteger( &num );

		_earthquakes.ClearObjectList();
		_earthquakes.Resize( num );

		for ( i = 1 ; i <= num ; i++ )
		{		
			_earthquakes.AddObject( ent );

			entityPointer = &_earthquakes.ObjectAt( i );

			arc.ArchiveSafePointer( entityPointer );
		}
	}

	arc.ArchiveInteger( &_totalEnemiesSpawned );

	// Don't archive these

	//const char     *current_map;
	//const char     *current_entities;

	//int			   spawn_entnum;
	arc.ArchiveInteger( &currentInstanceNumber );
    //int            spawnflags;

	arc.ArchiveInteger( &framenum );
	arc.ArchiveInteger( &inttime );
	arc.ArchiveFloat( &time );
	arc.ArchiveFloat( &timeInLevel );
	arc.ArchiveFloat( &frametime );
	arc.ArchiveFloat( &fixedframetime );
	arc.ArchiveInteger( &startTime );

	arc.ArchiveString( &level_name );
	arc.ArchiveString( &mapname );
	arc.ArchiveString( &spawnpoint );
	arc.ArchiveString( &nextmap );

	arc.ArchiveBoolean( &restart );
	arc.ArchiveBoolean( &started );

	arc.ArchiveBoolean( &playerfrozen );

	arc.ArchiveFloat( &intermissiontime );
	arc.ArchiveInteger( &exitintermission );
	arc.ArchiveFloat( &intermission_advancetime );
	arc.ArchiveBool( &_showIntermission );
	arc.ArchiveBool( &_saveOrientation );

	// Don't archive
	//gentity_s	   *next_edict;

	arc.ArchiveInteger( &total_secrets );
	arc.ArchiveInteger( &found_secrets );
	arc.ArchiveInteger( &total_specialItems );
	arc.ArchiveInteger( &found_specialItems );

	arc.ArchiveString( &game_script );

	// Don't archive
	//trace_t        impact_trace;

	arc.ArchiveBoolean( &cinematic );
	arc.ArchiveBoolean( &ai_on );

	arc.ArchiveBoolean( &mission_failed );
	arc.ArchiveBoolean( &died_already );

	arc.ArchiveBoolean( &near_exit );

	arc.ArchiveVector( &water_color );
	arc.ArchiveFloat( &water_alpha );

	arc.ArchiveVector( &slime_color );
	arc.ArchiveFloat( &slime_alpha );

	arc.ArchiveVector( &lava_color );
	arc.ArchiveFloat( &lava_alpha );

	arc.ArchiveString( &current_soundtrack );
	arc.ArchiveString( &saved_soundtrack );

	arc.ArchiveObjectPointer( ( Class ** )&consoleThread );

	arc.ArchiveVector( &m_fade_color );
	arc.ArchiveFloat( &m_fade_alpha );
	arc.ArchiveFloat( &m_fade_time );
	arc.ArchiveFloat( & m_fade_time_start );
	ArchiveEnum( m_fade_type, fadetype_t );
	ArchiveEnum( m_fade_style, fadestyle_t );

	arc.ArchiveFloat( &m_letterbox_fraction );
	arc.ArchiveFloat( &m_letterbox_time );
	arc.ArchiveFloat( &m_letterbox_time_start );
	ArchiveEnum( m_letterbox_dir, letterboxdir_t );

	arc.ArchiveBool( &_cleanup );

	arc.ArchiveString( &_playerDeathThread );

	arc.ArchiveObjectPointer( ( Class ** )&hNodeController );

	// Don't archive, will already be setup from camera code
	// Container<Camera *>	automatic_cameras;

	arc.ArchiveVector( & m_intermission_origin );
	arc.ArchiveVector( & m_intermission_angle );

	if ( arc.Loading() )
	{
		str temp_soundtrack;

		// Change the sound track to the one just loaded

		temp_soundtrack = saved_soundtrack;
		ChangeSoundtrack( current_soundtrack.c_str() );
		saved_soundtrack = temp_soundtrack;

		// not archived since we can't save mid-frame
		next_edict = NULL;
		// not archived since we can't save mid-frame
		memset( &impact_trace, 0, sizeof( impact_trace ) );

		loadLevelStrings();
	}
	
}

//-----------------------------------------------------
//
// Name:		loadLevelStrings
// Class:		Level
//
// Description:	Loads the string resource file for the level.
//
// Parameters:	None
//
// Returns:		
//-----------------------------------------------------
void Level::loadLevelStrings( void )
{
	const char* sublevelName;
	const char* levelName;
	const char* environmentName;
	
	gi.GetLevelDefs(mapname, &environmentName, &levelName, &sublevelName);
	gi.SR_LoadLevelStrings(environmentName);
}

/*
==============
Precache

Calls precache scripts
==============
*/
void Level::Precache( void )
{
	str         filename;
	const char *environmentName;
	const char *levelName;
	const char *sublevelName;
	int i;
	str mapName;
	const char *spawnPoint;
	
	//
	// load in global0-9.scr
	//
	for( i = 0; i < 100; i++ )
	{
		filename = va( "global/global%i.scr", i );
		
		if ( gi.FS_ReadFile( filename.c_str(), NULL, true ) != -1 )
			level.consoleThread->Parse( filename.c_str() );
		else
			break;
	}
	
	// Get the level defs
	if( !current_map )
		return;
	
	mapName = current_map;
	spawnPoint = strstr( mapName.c_str(), "$" );

	if ( spawnPoint )
	{
		mapName.CapLength( spawnPoint - mapName.c_str() );
	}
	
	gi.GetLevelDefs( mapName, &environmentName, &levelName, &sublevelName );
	
	// Precache global stuff
	
	for( i = 0 ; i < 10 ; i++ )
	{
		if ( i == 0 )
			filename = va( "precache/server/global.txt" );
		else
			filename = va( "precache/server/global%d.txt", i );
		
		if ( gi.FS_ReadFile( filename.c_str(), NULL, true ) != -1 )
			level.consoleThread->Parse( filename.c_str() );
		else
			break;
	}
	
	// Precache environment stuff
	
	if ( strlen( environmentName ) )
	{
		for( i = 0 ; i < 10 ; i++ )
		{
			if ( i == 0 )
				filename = va( "precache/server/%s.txt", environmentName );
			else
				filename = va( "precache/server/%s%d.txt", environmentName, i );
			
			if ( gi.FS_ReadFile( filename.c_str(), NULL, true ) != -1 )
				level.consoleThread->Parse( filename.c_str() );
			else
				break;
		}
	}
	
	// Precache level stuff
	
	if ( strlen( levelName ) )
	{
		for( i = 0 ; i < 10 ; i++ )
		{
			if ( i == 0 )
				filename = va( "precache/server/%s.txt", levelName );
			else
				filename = va( "precache/server/%s%d.txt", levelName, i );
			
			if ( gi.FS_ReadFile( filename.c_str(), NULL, true ) != -1 )
				level.consoleThread->Parse( filename.c_str() );
			else
				break;
		}
	}
	
	// Precache sublevel stuff
	
	if ( strlen( sublevelName ) && ( stricmp( levelName, sublevelName ) != 0 ) )
	{
		for( i = 0 ; i < 10 ; i++ )
		{
			if ( i == 0 )
				filename = va( "precache/server/%s.txt", sublevelName );
			else
				filename = va( "precache/server/%s%d.txt", sublevelName, i );
			
			if ( gi.FS_ReadFile( filename.c_str(), NULL, true ) != -1 )
				level.consoleThread->Parse( filename.c_str() );
			else
				break;
		}
	}
	
	// load in universal_script.scr
	
	//G_LoadAndExecScript( "global/universal_script.scr", "precache:", true );
}

/*
================
FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
void Level::FindTeams( void )
{
	gentity_t  *e;
	gentity_t  *e2;
	gentity_t  *next;
	gentity_t  *next2;
	Entity	*chain;
	Entity	*ent;
	Entity	*ent2;
	int		c;
	int		c2;
	
	c = 0;
	c2 = 0;
	
	for( e = active_edicts.next; e != &active_edicts; e = next )
	{
		assert( e );
		assert( e->inuse );
		assert( e->entity );
		
		next = e->next;
		
		ent = e->entity;
		if ( !ent->bind_info || ( ent->bind_info->moveteam.length() == 0 ) )
		{
			continue;
		}
		
		if ( ent->flags & FL_TEAMSLAVE )
		{
			continue;
		}
		
		chain = ent;
		ent->bind_info->teammaster = ent;
		c++;
		c2++;
		for( e2 = next; e2 != &active_edicts; e2 = next2 )
		{
			assert( e2 );
			assert( e2->inuse );
			assert( e2->entity );
			
			next2 = e2->next;
			
			ent2 = e2->entity;
			if ( !ent2->bind_info || ( ent2->bind_info->moveteam.length() == 0 ) )
			{
				continue;
			}
			
			if ( ent2->flags & FL_TEAMSLAVE )
			{
				continue;
			}
			
			if ( ent->bind_info->moveteam == ent2->bind_info->moveteam )
			{
				c2++;
				chain->bind_info->teamchain = ent2;
				ent2->bind_info->teammaster = ent;
				chain = ent2;
				ent2->flags |= FL_TEAMSLAVE;
			}
		}
	}
	
	gi.DPrintf( "%i teams with %i entities\n", c, c2 );
}

/*
=================
AllocEdict

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
gentity_t *Level::AllocEdict( Entity *ent )
{
	int		   i;
	gentity_t   *edict;
	
	if ( spawn_entnum >= 0 )
	{
		edict = &g_entities[ spawn_entnum ];
		spawn_entnum = -1;
		
		assert( !edict->inuse && !edict->entity );
		
		// free up the entity pointer in case we took one that still exists
		if ( edict->inuse && edict->entity )
		{
			delete edict->entity;
		}
	}
	else
	{
		edict = &g_entities[ maxclients->integer ];
		for ( i = maxclients->integer; i < globals.num_entities; i++, edict++ )
		{
			// the first couple seconds of server time can involve a lot of
			// freeing and allocating, so relax the replacement policy
			if (
				!edict->inuse &&
				(
				( edict->freetime < ( 2.0f + startTime ) ) ||
				( time - edict->freetime > 0.5f )
				)
				)
			{
				break;
			}
		}
		
		// allow two spots for none and world
		if ( i == game.maxentities - 2.0f )
		{
			// Try one more time before failing, relax timing completely

			edict = &g_entities[ maxclients->integer ];

			for ( i = maxclients->integer; i < globals.num_entities; i++, edict++ )
			{
				if ( !edict->inuse )
				{
					break;
				}
			}

			if ( i == game.maxentities - 2.0f )
			{
				gi.Error( ERR_DROP, "Level::AllocEdict: no free edicts" );
			}
		}
	}
	
	assert( edict->next );
	assert( edict->prev );
	LL_Remove( edict, next, prev );
	InitEdict( edict );
	assert( active_edicts.next );
	assert( active_edicts.prev );
	LL_Add( &active_edicts, edict, next, prev );
	assert( edict->next );
	assert( edict->prev );
	
	assert( edict->next != &free_edicts );
	assert( edict->prev != &free_edicts );
	
	// Tell the server about our data since we just spawned something
	if ( ( edict->s.number < ENTITYNUM_WORLD ) && ( globals.num_entities <= edict->s.number ) )
	{
		globals.num_entities = edict->s.number + 1;
		gi.LocateGameData( g_entities, globals.num_entities, sizeof( gentity_t ), &game.clients[ 0 ].ps, sizeof( game.clients[ 0 ] ) );
	}
	
	edict->entity = ent;
	edict->s.instanceNumber = currentInstanceNumber;
	currentInstanceNumber++;

	if ( currentInstanceNumber < 0 )
		currentInstanceNumber = 0;
	
	return edict;
}

/*
=================
FreeEdict

Marks the edict as free
=================
*/
void Level::FreeEdict( gentity_t *ed )
{
	gclient_t *client;
	
	assert( ed != &free_edicts );
	
	// unlink from world
	gi.unlinkentity ( ed );
	
	assert( ed->next );
	assert( ed->prev );
	
	if ( next_edict == ed )
	{
		next_edict = ed->next;
	}
	
	LL_Remove( ed, next, prev );
	
	assert( ed->next == ed );
	assert( ed->prev == ed );
	assert( free_edicts.next );
	assert( free_edicts.prev );
	
	client = ed->client;
	memset( ed, 0, sizeof( *ed ) );
	ed->client = client;
	ed->freetime = time;
	ed->inuse = false;
	ed->s.number = ed - g_entities;
	
	assert( free_edicts.next );
	assert( free_edicts.prev );
	
	LL_Add( &free_edicts, ed, next, prev );
	
	assert( ed->next );
	assert( ed->prev );
}

void Level::InitEdict( gentity_t *e )
{
	int i;
	
	e->inuse = true;
	e->s.number = e - g_entities;
	
	// make sure a default scale gets set
	e->s.scale = 1.0f;
	// make sure the default constantlight gets set, initalize to r 1.0, g 1.0, b 1.0, r 0
	e->s.constantLight = 0xffffff;
	e->s.renderfx |= RF_FRAMELERP;
	e->spawntime = time;
	e->s.frame = 0;
	
	e->svflags = 0;
	
	for( i = 0; i < NUM_BONE_CONTROLLERS; i++ )
	{
		e->s.bone_tag[ i ] = -1;
		VectorClear( e->s.bone_angles[ i ] );
		EulerToQuat( e->s.bone_angles[ i ], e->s.bone_quat[ i ] );
	}
	
	for( i = 0; i < NUM_MORPH_CONTROLLERS; i++ )
	{
		e->s.morph_controllers[ i ].index = -1;
		e->s.morph_controllers[ i ].percent = 0.0;
	}

	e->s.animationRate = 1.0f;
}

void Level::AddAutomaticCamera( Camera *cam )
{
	automatic_cameras.AddUniqueObject( cam );
}

void Level::SetGameScript( const char *scriptname )
{
	game_script = scriptname;
}

//----------------------------------------------------------------
// Name:			addEarthQuake
// Class:			Level
//
// Description:		Adds an earthquake to the list
//
// Parameters:		Entity *earthquake					- earthquake to add to the list
//
// Returns:			none
//----------------------------------------------------------------

void Level::addEarthquake( Earthquake *earthquake )
{
	if ( !_earthquakes.ObjectInList( earthquake ) )
	{
		_earthquakes.AddObject( earthquake );
	}
}

//----------------------------------------------------------------
// Name:			removeEarthQuake
// Class:			Level
//
// Description:		Removes an earthquake from the list
//
// Parameters:		Entity *earthquake					- earthquake to remove from the list
//
// Returns:			none
//----------------------------------------------------------------

void Level::removeEarthquake( Earthquake *earthquake )
{
	if ( _earthquakes.ObjectInList( earthquake ) )
	{
		_earthquakes.RemoveObject( earthquake );
	}
}

//----------------------------------------------------------------
// Name:			getEarthquakeMagnitudeAtPosition
// Class:			Level
//
// Description:		Gets the total magnitude of all earthquakes from a particular position
//
// Parameters:		const Vector &origin			- position to test against
//
// Returns:			float							- total magnitude of all the earthquakes at the given position
//----------------------------------------------------------------

float Level::getEarthquakeMagnitudeAtPosition( const Vector &origin )
{
	int i;
	float totalMagnitude;
	Earthquake *earthquake;

	totalMagnitude = 0.0f;

	// Add up all of the earthquakes magnitudes to get the total

	for( i = 1 ; i <= _earthquakes.NumObjects() ; i++ )
	{
		// This should be a safe cast since only Earthquakes are allowed to be added to this list

		earthquake = _earthquakes.ObjectAt( i );

		if ( earthquake )
		{
			// Add this earthquake to the total

			totalMagnitude += earthquake->getMagnitudeAtPosition( origin );
		}
	}
	
	// Return the total magnitude of all the earthquakes

	return totalMagnitude;
}

void Level::enemySpawned( Entity *enemy )
{
	_totalEnemiesSpawned++;
	levelVars.SetVariable( "total_enemies_spawned", _totalEnemiesSpawned );
}

void Level::setPlayerDeathThread( const str &threadName )
{
	_playerDeathThread = threadName;
}

str	Level::getPlayerDeathThread( void )
{
	return _playerDeathThread;
}
