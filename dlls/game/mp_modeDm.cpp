//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/mp_modeDm.cpp                                  $
// $Revision:: 19                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 8/08/03 1:26p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// Description:
//

#include "_pch_cpp.h"

#include "mp_manager.hpp"
#include "mp_modeDm.hpp"


CLASS_DECLARATION( MultiplayerModeBase, ModeDeathmatch, NULL )
{
	{ NULL, NULL }
};

//================================================================
// Name:        ModeDeathmatch
// Class:       ModeDeathmatch
//              
// Description: Constructor
//              
// Parameters:  const str& -- name of the arena
//              
// Returns:     None
//              
//================================================================
ModeDeathmatch::ModeDeathmatch()
{
}

//================================================================
// Name:        ~ModeDeathmatch
// Class:       ModeDeathmatch
//              
// Description: Destructor
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
ModeDeathmatch::~ModeDeathmatch()
{
}

void ModeDeathmatch::respawnPlayer( Player *player )
{
	Entity *spawnPoint;

	assert(player);

	if ( !player )
	{
		warning("ModeDeathmatch::respawnPlayer", "NULL player\n");
		return ;
	}

	// Make sure we are allowed to respawn the player

	if ( !multiplayerManager.checkRule( "respawnPlayer", true, player ) )
	{
		if ( multiplayerManager.checkRule( "allowSpectator", true ) )
		{
			multiplayerManager.makePlayerSpectator( player );
		}

		return;
	}

	// Respawn the player

	MultiplayerModeBase::respawnPlayer(player);

	multiplayerManager.initPlayer( player );

	// Warp the player to a spawn point

	spawnPoint = getSpawnPoint( player );

	if ( spawnPoint )
	{
		player->WarpToPoint( spawnPoint );
	}

	KillBox( player );

	ActivatePlayer(player);
}


//================================================================
// Name:        AddPlayer
// Class:       ModeDeathmatch
//              
// Description: Adds a player to the arena.  In deathmatch, this
//              also sets their spawn point and immediately sets 
//              them up for fighting.
//              
// Parameters:  Player* -- Player to add and prepare for fighting
//              
// Returns:     None
//              
//================================================================
void ModeDeathmatch::AddPlayer( Player *player )
{
	Entity *spawnPoint;

	assert(player);

	if ( !player )
	{
		warning("ModeDeathmatch::AddPlayer", "NULL Player\n");
		return ; 
	}

	if ( !needToAddPlayer( player ) )
		return;

	MultiplayerModeBase::AddPlayer(player);

	if ( !multiplayerManager.checkRule( "spawnPlayer", true, player ) )
	{
		if ( multiplayerManager.checkRule( "allowSpectator", true ) )
		{
			multiplayerManager.makePlayerSpectator( player );
		}

		return;
	}

	// Warp the player to a spawn point

	spawnPoint = getSpawnPoint( player );

	if ( spawnPoint )
	{
		player->WarpToPoint( spawnPoint );
	}

	KillBox( player );

	ActivatePlayer(player);

	// If the game hasn't started yet just make the player a spectator

	if ( !_gameStarted )
	{
		multiplayerManager.makePlayerSpectator( player );
	}
}

void ModeDeathmatch::init( int maxPlayers )
{
	MultiplayerModeBase::init( maxPlayers );

	readMultiplayerConfig( "global/mp_dm.cfg" );

	multiplayerManager.cacheMultiplayerFiles( "mp_dm" );
}

bool ModeDeathmatch::checkGameType( const char *gameType )
{
	if ( stricmp( gameType, "dm" ) == 0 )
		return true;
	else
		return false;
}

int ModeDeathmatch::getHighestPoints( int entnum )
{
	int i;
	int highestPoints = -999999999;

	for ( i = 0 ; i < _maxPlayers ; i++ )
	{
		if ( _playerGameData[ i ]._playing && ( _playerGameData[ i ]._entnum != entnum ) )
		{
			if ( _playerGameData[ i ]._points > highestPoints )
			{
				highestPoints = _playerGameData[ i ]._points;
			}
		}
	}

	return highestPoints;
}

bool ModeDeathmatch::checkRule( const char *rule, bool defaultValue, Player *player )
{
	if ( stricmp( rule, "usingIndividualScore" ) == 0 )
		return true;
	else
		return defaultValue;
}

void ModeDeathmatch::update( float frameTime )
{
	int i;
	Player *player;
	int place;
	bool tied;

	MultiplayerModeBase::update( frameTime );

	// Update the players about their ranking

	if ( _gameStarted )
	{
		for ( i = 0 ; i < _maxPlayers ; i++ )
		{
			player = multiplayerManager.getPlayer( i );

			if ( !player )
				continue;

			place = getPlace( player, &tied );

			if ( ( place != _playerGameData[ i ]._lastPlace ) || ( tied != _playerGameData[ i ]._lastTied ) )
			{
				if ( _playerGameData[ i ]._lastPlace != -1 )
				{
					if ( ( place == 1 ) && ( !tied ) )
					{
						// Now in first place
						multiplayerManager.playerSound( _playerGameData[ i ]._entnum, "localization/sound/dialog/dm/comp_1stplace.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
					}
					else if ( ( place == 1 ) && ( tied ) )
					{
						// Now tied for the lead
						multiplayerManager.playerSound( _playerGameData[ i ]._entnum, "localization/sound/dialog/dm/comp_tiedlead.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
					}
					else if ( ( _playerGameData[ i ]._lastPlace == 1 ) && ( ( place != 1 ) || ( tied ) ) )
					{
						// No longer in the lead
						multiplayerManager.playerSound( _playerGameData[ i ]._entnum, "localization/sound/dialog/dm/comp_nolead.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
					}
				}

				_playerGameData[ i ]._lastPlace = place;
				_playerGameData[ i ]._lastTied  = tied;
			}
		}
	}
}
