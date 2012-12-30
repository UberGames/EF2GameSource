//------------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/mp_modeTeamDm.cpp                               $
// $Revision:: 26                                                              $
//   $Author:: Singlis                                                         $
//     $Date:: 5/17/03 6:41p                                                   $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// Description:
//

#include "_pch_cpp.h"
#include "mp_modeTeamDm.hpp"
#include "mp_manager.hpp"

CLASS_DECLARATION( ModeTeamBase, ModeTeamDeathmatch, NULL )
{
	{ NULL, NULL }
};

ModeTeamDeathmatch::ModeTeamDeathmatch()
{
	_redTeam  = AddTeam("Red");
	_blueTeam = AddTeam("Blue");
}

ModeTeamDeathmatch::~ModeTeamDeathmatch()
{
}

void ModeTeamDeathmatch::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	Team *team;
	int points;
	bool goodKill;

	if ( attackingPlayer && ( killedPlayer != attackingPlayer ) && 
		 ( _playerGameData[ attackingPlayer->entnum ]._currentTeam != _playerGameData[ killedPlayer->entnum ]._currentTeam ) )
		goodKill = true;
	else
		goodKill = false;

	handleKill( killedPlayer, attackingPlayer, inflictor, meansOfDeath, goodKill );
	//MultiplayerModeBase::playerKilled( killedPlayer, attackingPlayer, inflictor, meansOfDeath );

	if ( !attackingPlayer )
		attackingPlayer = killedPlayer;

	team = _playerGameData[ attackingPlayer->entnum ]._currentTeam;

	if ( team )
	{
		if ( multiplayerManager.checkRule( "team-pointsForKills", true, attackingPlayer ) )
		{
			// Setup the default points for this kill - 1 point to the attacking player's team (if victim's not 
			//   on same team), or lose 1 point if on the same team

			if ( _playerGameData[ killedPlayer->entnum ]._currentTeam != _playerGameData[ attackingPlayer->entnum ]._currentTeam )
			{
				points = _defaultPointsPerKill;

				// Get the real points to give (the modifiers can change it)

				points = multiplayerManager.getPointsForKill( killedPlayer, attackingPlayer, inflictor, meansOfDeath, points );

				// Give points to the team

				team->addPoints( attackingPlayer, points );
			}
		}
	}
}

bool ModeTeamDeathmatch::isEndOfMatch( void )
{
	// TODO: move this to ModeTeamBase ?

	// See if we have a gone over the point limit

	if ( getPointLimit() > 0 ) 
	{
		// Check team points

		if ( _redTeam->getPoints() >= getPointLimit() )
			return true;
		else if ( _blueTeam->getPoints() >= getPointLimit() )
			return true;
	}

	// See if we have a gone over the time limit

	if ( ( getTimeLimit() > 0.0f ) && ( multiplayerManager.getTime() - _gameStartTime > getTimeLimit() ) ) 
		return true;

	return false;
}

int ModeTeamDeathmatch::getTeamPoints( Player *player )
{
	Team *team;

	// Return the points for this team

	team = _playerGameData[ player->entnum ]._currentTeam;

	if ( team )
		return team->getPoints();
	else
		return 0;
}

void ModeTeamDeathmatch::setupMultiplayerUI( Player *player )
{
	// Todo: make this a function in MultiplayerModeBase, each subclass just sets a string to the hud name

	gi.SendServerCommand( player->entnum, "stufftext \"ui_removehuds all\"\n" );
	gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_console\"\n" );
	gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_teamhud\"\n" );
	
	if(mp_timelimit->integer)
	{
		gi.SendServerCommand( player->entnum, "stufftext \"globalwidgetcommand dmTimer enable\"\n");
	}
	else
	{
		gi.SendServerCommand( player->entnum, "stufftext \"globalwidgetcommand dmTimer disable\"\n");
	}
}

void ModeTeamDeathmatch::init( int maxPlayers )
{
	ModeTeamBase::init( maxPlayers );

	readMultiplayerConfig( "global/mp_team.cfg" );
}

void ModeTeamDeathmatch::score( const Player *player )
{
	char		   string[1400];
	char		   entry[1024];
	int            i;
	int            tempStringlength;
	int            count        = 0;
	int            stringlength = 0;
	Team			*team;
	str				teamName;
	//int				teamPoints;
	Player			*currentPlayer;
	int				redScore;
	int				blueScore;

	assert( player );
	if ( !player )
	{
		warning( "MultiplayerModeBase::score", "Null Player specified.\n" );
		return;
	}

	string[0]  = 0;
	entry[0]   = 0;

	// This for loop builds a string containing all the players scores.

	for ( i = 0 ; i < _maxPlayers ; i++ )
	{
		currentPlayer = multiplayerManager.getPlayer( i );

		if ( !currentPlayer )
			continue;

		team = multiplayerManager.getPlayersTeam( currentPlayer );

		if ( team )
		{
			teamName = team->getName();
			//teamPoints = team->getPoints();
		}
		else
		{
			teamName = "spectator";
			//teamPoints = 0;
		}

		Com_sprintf( entry, sizeof( entry ), "%i %i %i %i %s %i %i %d %d %d %d %d %d ", 
				multiplayerManager.getClientNum( _playerGameData[ i ]._entnum ),
				_playerGameData[ i ]._points, 
				_playerGameData[ i ]._numKills, 
				_playerGameData[ i ]._numDeaths, 
				//0 /*pl->GetMatchesWon() */,
				//0 /*pl->GetMatchesLost()*/, 
				teamName.c_str(),
				//teamPoints,
				(int)(multiplayerManager.getTime() - _playerGameData[ i ]._startTime ), 
				multiplayerManager.getClientPing( _playerGameData[ i ]._entnum ),
				multiplayerManager.getScoreIcon( currentPlayer, SCOREICON1 ), 
				multiplayerManager.getScoreIcon( currentPlayer, SCOREICON2 ), 
				multiplayerManager.getScoreIcon( currentPlayer, SCOREICON3 ), 
				multiplayerManager.getScoreIcon( currentPlayer, SCOREICON4 ), 
				multiplayerManager.getScoreIcon( currentPlayer, SCOREICON5 ), 
				multiplayerManager.getScoreIcon( currentPlayer, SCOREICON6 ) );

		tempStringlength = strlen( entry );

		// Make sure the string is not too big (take into account other stuff that gets prepended below also)

		if ( stringlength + tempStringlength > 975 )
			break;

		strcpy( string + stringlength, entry );

		stringlength += tempStringlength;
		count++;
	}

	redScore  = multiplayerManager.getTeamPoints( "Red" );
	blueScore = multiplayerManager.getTeamPoints( "Blue" );

	gi.SendServerCommand( player->edict-g_entities, "scores 1 %i %d %d %s", count, redScore, blueScore, string );
}

bool ModeTeamDeathmatch::checkGameType( const char *gameType )
{
	if ( stricmp( gameType, "teamdm" ) == 0 )
		return true;
	else
		return false;
}
