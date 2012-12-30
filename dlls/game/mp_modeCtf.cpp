//------------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/mp_modeCtf.cpp                              $
// $Revision:: 60                                                              $
//   $Author:: Singlis                                                         $
//     $Date:: 9/26/03 2:36p                                                   $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// Description:
//    Implementation of Team and TeamArena classes.  These classes provide
//    Team-based multiplayer support.
//

#include "_pch_cpp.h"
#include "mp_modeCtf.hpp"
#include "mp_manager.hpp"


//----------------------------------------------------------------
//          C A P T U R E   T H E   F L A G   A R E N A
//----------------------------------------------------------------

// Setup constants

const float	ModeCaptureTheFlag::_maxGuardingDist = 1000.0f;
const float	ModeCaptureTheFlag::_ctfCapturePoints = 100.0f;
const float	ModeCaptureTheFlag::_maxDroppedFlagTime = 15.0f;

const int ModeCaptureTheFlag::_pointsForFlagReturn			=  5;
const int ModeCaptureTheFlag::_pointsForKillingFlagCarrier	= 20;
const int ModeCaptureTheFlag::_pointsForAssist				= 15;
const int ModeCaptureTheFlag::_pointsForDefense				= 15;
const int ModeCaptureTheFlag::_pointsForTakingTheFlag		= 15;
const int ModeCaptureTheFlag::_pointsForCapturingTheFlag	= 85;


CLASS_DECLARATION( ModeTeamBase, ModeCaptureTheFlag, NULL )
{
	{ NULL, NULL }
};

//REGISTER_ARENA( ModeCaptureTheFlag );

//================================================================
// Name:        ModeCaptureTheFlag
// Class:       ModeCaptureTheFlag
//              
// Description: Constructor
//              
// Parameters:  const str& -- name of the arena
//              
// Returns:     None
//              
//================================================================
ModeCaptureTheFlag::ModeCaptureTheFlag()
{
	_redTeam  = AddTeam("Red");
	_blueTeam = AddTeam("Blue");

	_playerCtfData = NULL;

	_useTeamSpawnpoints = true;

	_flagCarrierIconIndex = gi.imageindex( "sysimg/icons/mp/ctf_flagcarrier" );

	_oneFlagTakenIconIndex   = gi.imageindex( "sysimg/icons/mp/ctf_oneFlagTaken" );
	_oneFlagMissingIconIndex = gi.imageindex( "sysimg/icons/mp/ctf_oneFlagMissing" );
	_oneFlagInBaseIconIndex  = gi.imageindex( "sysimg/icons/mp/ctf_oneFlagInBase" );

	_redFlagTakenIconIndex   = gi.imageindex( "sysimg/icons/mp/ctf_redFlagTaken" );
	_redFlagMissingIconIndex = gi.imageindex( "sysimg/icons/mp/ctf_redFlagMissing" );
	_redFlagInBaseIconIndex  = gi.imageindex( "sysimg/icons/mp/ctf_redFlagInBase" );

	_blueFlagTakenIconIndex   = gi.imageindex( "sysimg/icons/mp/ctf_blueFlagTaken" );
	_blueFlagMissingIconIndex = gi.imageindex( "sysimg/icons/mp/ctf_blueFlagMissing" );
	_blueFlagInBaseIconIndex  = gi.imageindex( "sysimg/icons/mp/ctf_blueFlagInBase" );
}


//================================================================
// Name:        ~ModeCaptureTheFlag
// Class:       ModeCaptureTheFlag
//              
// Description: Destructor
//              
// Parameters:  None
//              
// Returns:     None, dammit
//              
//================================================================
ModeCaptureTheFlag::~ModeCaptureTheFlag()
{
	delete [] _playerCtfData;
	_playerCtfData = NULL;

	_flags.FreeObjectList();
}

void ModeCaptureTheFlag::init( int maxPlayers )
{
	// Initialize all of our data

	ModeTeamBase::init( maxPlayers );

	_playerCtfData = new MultiplayerPlayerCtfData[ _maxPlayers ];

	_flags.FreeObjectList();

	// Read in the ctf data

	readMultiplayerConfig( "global/mp_ctf.cfg" );

	multiplayerManager.cacheMultiplayerFiles( "mp_ctf" );
}

bool ModeCaptureTheFlag::shouldKeepItem( MultiplayerItem *item )
{
	// See if we want to keep this item

	if ( strnicmp( item->getName().c_str(), "ctfflag", 7 ) == 0 )
	{
		if ( !multiplayerManager.checkRule ( "keepflags", true ) )
			return false;

		if ( ( stricmp( item->getName().c_str() + 8, "red" ) == 0 ) ||
			 ( stricmp( item->getName().c_str() + 8, "blue" ) == 0 ) || 
			 ( stricmp( item->getName().c_str() + 8, "base" ) == 0 ) )
		{
			// It is a red or blue flag so keep it

			return true;
		}
	}

	return false;
}

void ModeCaptureTheFlag::itemKept( MultiplayerItem *item )
{
	// See if we care about this item

	if ( strnicmp( item->getName().c_str(), "ctfflag", 7 ) == 0 )
	{
		CtfFlag ctfFlag;

		// It is a flag so keep track of it

		ctfFlag._realFlag = item;
		ctfFlag._tempFlag = NULL;

		if ( strnicmp( item->getName().c_str() + 8, "red", 3 ) == 0 )
		{
			ctfFlag._teamName = "Red";
		}
		else if ( strnicmp( item->getName().c_str() + 8, "blue", 4 ) == 0 )
		{
			ctfFlag._teamName = "Blue";
		}

		_flags.AddObject ( ctfFlag );
	}
}

void ModeCaptureTheFlag::addPlayerToTeam( Player *player, Team *team )
{
	ModeTeamBase::addPlayerToTeam( player, team );

	_playerCtfData[ player->entnum ].init();
}

void ModeCaptureTheFlag::RemovePlayer( Player *player )
{
	ModeTeamBase::RemovePlayer( player );

	putFlagBack( player );
}

void ModeCaptureTheFlag::setupMultiplayerUI( Player *player )
{
	// Todo: make this a function in MultiplayerModeBase, each subclass just sets a string to the hud name

	gi.SendServerCommand( player->entnum, "stufftext \"ui_removehuds all\"\n" );
	gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_console\"\n" );
	gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_teamhud\"\n" );

	if ( multiplayerManager.checkRule ( "flagpickup-enemyflag", true ) )
	{
		gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_flagstatus\"\n" );
	}
	
	if(mp_timelimit->integer)
	{
		gi.SendServerCommand( player->entnum, "stufftext \"globalwidgetcommand dmTimer enable\"\n");
	}
	else
	{
		gi.SendServerCommand( player->entnum, "stufftext \"globalwidgetcommand dmTimer disable\"\n");
	}
}

void ModeCaptureTheFlag::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	Team *team;
	//int points;
	float distance;
	str printString;
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

	/* if ( team )
	{
		if ( multiplayerManager.checkRule( "team-pointsForKills", true, attackingPlayer ) )
		{
			// Award a point to the attacking player's team (if victim's not on same team)
			// Lose a point if on the same team

			if ( _playerGameData[ killedPlayer->entnum ]._currentTeam != _playerGameData[ attackingPlayer->entnum ]._currentTeam )
			{
				points = _defaultPointsPerKill;

				points = multiplayerManager.getPointsForKill( killedPlayer, attackingPlayer, inflictor, meansOfDeath, points );
				team->addPoints( attackingPlayer, points );
			}
		}
	} */

	// See if the killed player was carrying the flag

	if ( _playerCtfData[ killedPlayer->entnum ]._hasFlag )
	{
		if ( _playerGameData[ killedPlayer->entnum ]._currentTeam != _playerGameData[ attackingPlayer->entnum ]._currentTeam )
		{
			CtfFlag *ctfFlag;
			str printString;

			printString = "$$FlagCarrierKilled$$ ";
			printString += attackingPlayer->client->pers.netname;
			printString += " ($$";
			printString += _playerGameData[ attackingPlayer->entnum ]._currentTeam->getName();
			printString += "$$ $$Team$$)!";

			multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_NORMAL );

			ctfFlag = findFlag( _playerCtfData[ killedPlayer->entnum ]._flag );

			if ( ctfFlag )
			{
				if ( ctfFlag->_teamName == "Red" )
					multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rfk.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
				else if ( ctfFlag->_teamName == "Blue" )
					multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bfk.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
				else
					multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_flagcarrierkilled.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
			}

			// Give points to the player 

			multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForKillingFlagCarrier );
		}

		dropFlag( killedPlayer );
	}

	// See if the player was guarding the flag (either he or the killed player was close to his flag)

	if ( _playerGameData[ killedPlayer->entnum ]._currentTeam != _playerGameData[ attackingPlayer->entnum ]._currentTeam )
	{
		bool flagGuarded;

		distance = findNearestTeamFlagDist( team->getName(), attackingPlayer->origin );

		flagGuarded = false;

		if ( ( distance > 0 ) && ( distance < _maxGuardingDist ) )
		{
			flagGuarded = true;
		}
		else
		{
			distance = findNearestTeamFlagDist( team->getName(), killedPlayer->origin );

			if ( ( distance > 0 ) && ( distance < _maxGuardingDist ) )
			{
				flagGuarded = true;
			}
		}

		if ( flagGuarded )
		{
			multiplayerManager.playerEventNotification( "flag-guarded", "", attackingPlayer );
			
			printString = "$$FlagGuarded$$ ";
			printString += attackingPlayer->client->pers.netname;

			multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_flagguarded.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );

			multiplayerManager.centerPrintTeamClients( attackingPlayer, printString, CENTERPRINT_IMPORTANCE_HIGH );

			// Give points to the player

			multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForDefense );
		}
	}

	// See if the player was guarding his team's flag carrier (either he or the killed player was close to the flag carrier)

	if ( ( _playerGameData[ killedPlayer->entnum ]._currentTeam != _playerGameData[ attackingPlayer->entnum ]._currentTeam ) &&
		 !_playerCtfData[ attackingPlayer->entnum ]._hasFlag )
	{
		bool flagCarrierGuarded;

		distance = findNearestTeamFlagCarrierDist( team->getName(), attackingPlayer->origin );

		flagCarrierGuarded = false;

		if ( ( distance > 0 ) && ( distance < _maxGuardingDist ) )
		{
			flagCarrierGuarded = true;
		}
		else
		{
			distance = findNearestTeamFlagCarrierDist( team->getName(), killedPlayer->origin );

			if ( ( distance > 0 ) && ( distance < _maxGuardingDist ) )
			{
				flagCarrierGuarded = true;
			}
		}

		if ( flagCarrierGuarded )
		{
			//multiplayerManager.playerEventNotification( "flag-carrierguarded", "", attackingPlayer );
			
			printString = "$$FlagAssist$$ ";
			printString += attackingPlayer->client->pers.netname;

			multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_flagassist.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );

			multiplayerManager.centerPrintTeamClients( attackingPlayer, printString, CENTERPRINT_IMPORTANCE_HIGH );

			// Give points to the player

			multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForAssist );
		}
	}
}

CtfFlag *ModeCaptureTheFlag::findFlag( MultiplayerItem *item )
{
	int i;
	CtfFlag *ctfFlag;

	for ( i = 1 ; i <= _flags.NumObjects() ; i++ )
	{
		ctfFlag = &_flags.ObjectAt( i );

		if ( ( ctfFlag->_realFlag == item ) || ( ctfFlag->_tempFlag == item ) )
			return ctfFlag;
	}

	return NULL;
}

float ModeCaptureTheFlag::findNearestTeamFlagDist( const str &teamName, const Vector &position )
{
	int i;
	CtfFlag *ctfFlag;
	CtfFlag *nearestCtfFlag = NULL;
	float nearestDistance = -1.0f;
	Vector diff;
	float distance;

	// Go through all of the flags

	for ( i = 1 ; i <= _flags.NumObjects() ; i++ )
	{
		ctfFlag = &_flags.ObjectAt( i );

		// Make sure everything is ok

		if ( !ctfFlag->_realFlag )
			continue;

		if ( ctfFlag->_teamName != teamName )
			continue;

		// Get the distance to the flag

		diff = position - ctfFlag->_realFlag->origin;
		distance = diff.length();

		// See if this is the nearest flag

		if ( !nearestCtfFlag || ( distance < nearestDistance ) )
		{
			nearestCtfFlag = ctfFlag;
			nearestDistance = distance;
		}
	}

	return nearestDistance;
}

float ModeCaptureTheFlag::findNearestTeamFlagCarrierDist( const str &teamName, const Vector &position )
{
	int i;
	Player *currentPlayer;
	Player *nearestFlagCarrier = NULL;
	float nearestDistance = -1.0f;
	Team *team;
	Vector diff;
	float distance;

	// Go through all of the flags

	for ( i = 0 ; i < _maxPlayers ; i++ )
	{
		if ( !_playerGameData[ i ]._playing )
			continue;

		currentPlayer = (Player *)g_entities[ _playerGameData[ i ]._entnum ].entity;

		team = multiplayerManager.getPlayersTeam( currentPlayer );

		// Make sure this player is one the correct team and has the flag

		if ( !team )
			continue;

		if ( team->getName() != teamName )
			continue;

		if ( !_playerCtfData[ currentPlayer->entnum ]._hasFlag )
			continue;

		// Get the distance to the flag carrier

		diff = position - currentPlayer->origin;
		distance = diff.length();

		// See if this is the nearest flag carrier

		if ( !nearestFlagCarrier || ( distance < nearestDistance ) )
		{
			nearestFlagCarrier = currentPlayer;
			nearestDistance = distance;
		}
	}

	return nearestDistance;
}

void ModeCaptureTheFlag::itemTouched( Player *player, MultiplayerItem *item )
{
	CtfFlag *ctfFlag;
	bool myFlag;
	bool teamFlag;
	bool tempFlag;

	// Make sure this item is one of the flags

	ctfFlag = findFlag( item );

	if ( !ctfFlag )
		return;

	// Determine if this is my team's flag or not

	if ( !_playerGameData[ player->entnum ]._currentTeam )
		return;

	if ( ( ( ctfFlag->_teamName == "Red" ) && ( _playerGameData[ player->entnum ]._currentTeam == _redTeam ) ) ||
		 ( ( ctfFlag->_teamName == "Blue" ) && ( _playerGameData[ player->entnum ]._currentTeam == _blueTeam ) ) )
		myFlag = true;
	else
		myFlag = false;

	// Determine if this is one of the team flags (red or blue)

	if ( ( ctfFlag->_teamName == "Red" ) || ( ctfFlag->_teamName == "Blue" ) )
		teamFlag = true;
	else
		teamFlag = false;

	// Determine if this is a temporary flag (one dropped)

	if ( item == ctfFlag->_tempFlag )
		tempFlag = true;
	else
		tempFlag = false;

	// Determine what to do about touching the flag

	if ( myFlag && teamFlag && tempFlag && multiplayerManager.checkRule ( "flagreturn-teamflag", true ) )
	{
		// Return this temporary flag back to the flag position

		returnFlag( item, player );
	}
	else if ( _playerCtfData[ player->entnum ]._hasFlag )
	{
		// The player has a flag, see if we score or not

		if ( !tempFlag && myFlag && multiplayerManager.checkRule ( "flagscore-teamflag", true ) )
		{
			score( player );
		}
		else if ( !tempFlag && !myFlag && teamFlag && multiplayerManager.checkRule ( "flagscore-enemyflag", false ) )
		{
			score( player );
		}
		else if ( !tempFlag && !teamFlag )
		{
			str rule;

			rule = "flagscore-otherflag-";
			rule += item->getName();

			if ( multiplayerManager.checkRule ( rule.c_str(), false ) )
			{
				score( player );
			}
		}
	}
	else
	{
		// The player does not have a flag, see if we can grab the flag

		if ( !myFlag && teamFlag && multiplayerManager.checkRule ( "flagpickup-enemyflag", true ) )
		{
			grabTheFlag( player, item );
		}
		else if ( myFlag && teamFlag && multiplayerManager.checkRule ( "flagpickup-teamflag", false ) )
		{
			grabTheFlag( player, item );
		}
		else if ( !teamFlag )
		{
			str rule;

			rule = "flagpickup-otherflag-";
			rule += ctfFlag->_realFlag->getName();

			if ( multiplayerManager.checkRule ( rule.c_str(), false ) )
			{
				grabTheFlag( player, item );
			}
		}
	}
}

void ModeCaptureTheFlag::score( Player *player )
{
	Team *team;
	str printString;
	CtfFlag *ctfFlag;

	ctfFlag = findFlag( _playerCtfData[ player->entnum ]._flag );

	if ( !ctfFlag )
		return;

	// Put the flag back where it is suppose to be

	putFlagBack( player );

	// Give the player's team points

	// Todo: hide all team stuff in ModeTeamBase ?

	team = _playerGameData[ player->entnum ]._currentTeam;

	// Tell everyone the flag was captured

	printString = "$$FlagCaptured$$ ";
	printString += player->client->pers.netname;
	printString += " ($$";
	printString += team->getName();
	printString += "$$ $$Team$$)!";

	multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_NORMAL );

	if ( strnicmp( ctfFlag->_realFlag->getName().c_str(), "ctfflag-red", strlen( "ctfflag-red" ) ) == 0 )
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_frc.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
	else if ( strnicmp( ctfFlag->_realFlag->getName().c_str(), "ctfflag-blue", strlen( "ctfflag-blue" ) ) == 0 )
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bfc.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
	else
	{
		if ( team->getName() == "Red" )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_fcrt.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
		else
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_fcbt.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
	}

	// Tell the other multiplayer systems that the flag was captured

	multiplayerManager.playerEventNotification( "flag-captured", ctfFlag->_realFlag->getName().c_str(), player );

	// Give the team and the player points

	team->addPoints( player,(int) _ctfCapturePoints );

	multiplayerManager.addPoints( player->entnum, _pointsForCapturingTheFlag );
}

void ModeCaptureTheFlag::dropFlag( Player *player )
{
	CtfFlag *ctfFlag;
	MultiplayerItem *newFlag;

	// Make sure everything is ok

	if ( !_playerCtfData[ player->entnum ]._hasFlag )
		return;

	ctfFlag = findFlag( _playerCtfData[ player->entnum ]._flag );

	if ( !ctfFlag )
		return;

	// Create a new flag to put on the ground

	newFlag = new MultiplayerItem;

	newFlag->setModel( ctfFlag->_realFlag->model );

	newFlag->angles = player->angles;
	newFlag->setAngles();

	newFlag->setOrigin( player->origin );

	newFlag->CancelEventsOfType( EV_ProcessInitCommands );
	newFlag->ProcessInitCommands( newFlag->edict->s.modelindex );

	newFlag->setName( "ctfflag-temp" );

	// Save off some info about the dropped flag

	ctfFlag->_tempFlag = newFlag;
	ctfFlag->_tempFlagTime = multiplayerManager.getTime();

	// Remove the flag from the player

	if ( _playerCtfData[ player->entnum ]._carriedFlag )
	{
		_playerCtfData[ player->entnum ]._carriedFlag->PostEvent( EV_Detach, 0.0f );
		_playerCtfData[ player->entnum ]._carriedFlag->PostEvent( EV_Remove, FRAMETIME );
		_playerCtfData[ player->entnum ]._carriedFlag = NULL;
	}

	_playerCtfData[ player->entnum ]._hasFlag = false;

	multiplayerManager.playerEventNotification( "flag-dropped", ctfFlag->_realFlag->getName().c_str(), player );
}

void ModeCaptureTheFlag::returnFlag( MultiplayerItem *item, Player *player )
{
	CtfFlag *ctfFlag;
	str printString;

	ctfFlag = findFlag( item );

	if ( !ctfFlag )
		return;

	// Remove the dropped flag from the ground

	if ( ctfFlag->_tempFlag )
	{
		ctfFlag->_tempFlag->PostEvent( EV_Remove, FRAMETIME );

		ctfFlag->_tempFlag = NULL;
	}

	// Put the flag back to where it belongs (really just show it again)

	if ( ctfFlag->_realFlag )
	{
		ctfFlag->_realFlag->showModel();
		ctfFlag->_realFlag->setSolidType( SOLID_TRIGGER );
	}

	if ( player )
	{
		// Tell everyone that the player returned the flag

		printString = "$$FlagReturnedBy$$ ";
		printString += player->client->pers.netname;
		printString += " ($$";
		printString += _playerGameData[ player->entnum ]._currentTeam->getName();
		printString += "$$ $$Team$$)!";

		multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_NORMAL );

		if ( strnicmp( ctfFlag->_realFlag->getName().c_str(), "ctfflag-red", strlen( "ctfflag-red" ) ) == 0 )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rfr.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
		else if ( strnicmp( ctfFlag->_realFlag->getName().c_str(), "ctfflag-blue", strlen( "ctfflag-blue" ) ) == 0 )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bfr.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
		else
		{
			if ( _playerGameData[ player->entnum ]._currentTeam->getName() == "Red" )
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_frbrt.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
			else
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_frbbt.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
		}

		// Tell all of the other multiplayer systems that the flag was returned

		multiplayerManager.playerEventNotification( "flag-returned", ctfFlag->_realFlag->getName().c_str(), player );

		// Give points to the player 

		multiplayerManager.addPoints( player->entnum, _pointsForFlagReturn );
	}
	else
	{
		printString = "$$FlagReturned$$";

		multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_NORMAL );

		if ( strnicmp( ctfFlag->_realFlag->getName().c_str(), "ctfflag-red", strlen( "ctfflag-red" ) ) == 0 )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rfr.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
		else if ( strnicmp( ctfFlag->_realFlag->getName().c_str(), "ctfflag-blue", strlen( "ctfflag-blue" ) ) == 0 )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bfr.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
		
	}
}

void ModeCaptureTheFlag::putFlagBack( Player *player )
{
	CtfFlag *ctfFlag;
	MultiplayerItem *flag;

	// Make sure everything is ok

	if ( !_playerCtfData[ player->entnum ]._hasFlag )
		return;

	ctfFlag = findFlag( _playerCtfData[ player->entnum ]._flag );

	if ( !ctfFlag )
		return;

	flag = ctfFlag->_realFlag;

	// Put the flag back

	if ( flag )
	{
		flag->showModel();
		flag->setSolidType( SOLID_TRIGGER );
	}

	// Remove flag from the player

	if ( _playerCtfData[ player->entnum ]._carriedFlag )
	{
		_playerCtfData[ player->entnum ]._carriedFlag->PostEvent( EV_Detach, 0.0f );
		_playerCtfData[ player->entnum ]._carriedFlag->PostEvent( EV_Remove, FRAMETIME );
		_playerCtfData[ player->entnum ]._carriedFlag = NULL;
	}

	_playerCtfData[ player->entnum ]._hasFlag = false;
}

void ModeCaptureTheFlag::grabTheFlag( Player *player, MultiplayerItem *item )
{
	CtfFlag *ctfFlag;
	Entity *obj;
	str printString;
	Team *team;
	MultiplayerItem *flag;
	bool tempFlag;

	ctfFlag = findFlag( item );

	flag = ctfFlag->_realFlag;

	// Get rid of the dropped flag (if any)

	if ( ctfFlag->_tempFlag )
	{
		ctfFlag->_tempFlag->PostEvent( EV_Remove, FRAMETIME );

		ctfFlag->_tempFlag = NULL;

		tempFlag = true;
	}
	else
	{
		tempFlag = false;
	}

	// Give the flag to the player

	obj = new Entity( ENTITY_CREATE_FLAG_ANIMATE );

	obj->setModel( flag->model );

	obj->animate->RandomAnimate( "idle" );

	attachFlag( player, obj );

	_playerCtfData[ player->entnum ]._hasFlag = true;
	_playerCtfData[ player->entnum ]._carriedFlag = obj;
	_playerCtfData[ player->entnum ]._flag = flag;

	// Hide the real flag and make it not solid

	flag->setSolidType( SOLID_NOT );
	flag->hideModel();

	// Give the player some points for taking the flag (only if it was taken from the base)

	if ( !tempFlag )
	{
		multiplayerManager.addPoints( player->entnum, _pointsForTakingTheFlag );
	}

	// Tell everyone that the flag was taken

	team = _playerGameData[ player->entnum ]._currentTeam;

	printString = "$$FlagTaken$$ ";
	printString += player->client->pers.netname;
	printString += " ($$";
	printString += team->getName();
	printString += "$$ $$Team$$)!";

	multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_NORMAL );

	if ( strnicmp( ctfFlag->_realFlag->getName().c_str(), "ctfflag-red", strlen( "ctfflag-red" ) ) == 0 )
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rft.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
	else if ( strnicmp( ctfFlag->_realFlag->getName().c_str(), "ctfflag-blue", strlen( "ctfflag-blue" ) ) == 0 )
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bft.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
	else
	{
		if ( team->getName() == "Red" )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_ftbrt.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
		else
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_ftbbt.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );
	}

	multiplayerManager.playerEventNotification( "flag-taken", ctfFlag->_realFlag->getName().c_str(), player );
}

void ModeCaptureTheFlag::update( float frameTime )
{
	CtfFlag *ctfFlag;
	int i;

	MultiplayerModeBase::update( frameTime );

	// Return any flags that have been sitting on the ground too long

	for ( i = 1 ; i <= _flags.NumObjects() ; i++ )
	{
		ctfFlag = &_flags.ObjectAt( i );

		if ( ( ctfFlag->_tempFlag ) && ( ctfFlag->_tempFlagTime + _maxDroppedFlagTime < multiplayerManager.getTime() ) )
		{
			returnFlag( ctfFlag->_tempFlag, NULL );
		}
	}
}

bool ModeCaptureTheFlag::isEndOfMatch( void )
{
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

int ModeCaptureTheFlag::getIcon( Player *player, int statNum, int value )
{
	if ( statNum == STAT_MP_MODE_ICON )
	{
		if ( _playerCtfData[ player->entnum ]._hasFlag )
			return _flagCarrierIconIndex;
		else
			return -1;
	}

	return ModeTeamBase::getIcon( player, statNum, value );
}

int ModeCaptureTheFlag::getScoreIcon( Player *player, int index, int value )
{
	if ( index == SCOREICON1 )
	{
		if ( _playerCtfData[ player->entnum ]._hasFlag )
			return _flagCarrierIconIndex;
		else
			return 0;
	}

	return value;
}

void ModeCaptureTheFlag::score( const Player *player )
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
				(int)(multiplayerManager.getTime() - _playerGameData[i]._startTime), 
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

	gi.SendServerCommand( player->edict-g_entities, "scores 2 %i %d %d %s", count, redScore, blueScore, string );
}

//----------------------------------------------------------------
//              P R O T E C T E D   M E T H O D S
//----------------------------------------------------------------
/* void ModeCaptureTheFlag::_endMatch(	void )
{
	multiplayerManager.centerPrintAllClients( "$$MatchOver$$\n", CENTERPRINT_IMPORTANCE_NORMAL );

	multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_matover.mp3" );
} */


bool ModeCaptureTheFlag::checkGameType( const char *gameType )
{
	if ( stricmp( gameType, "ctf" ) == 0 )
		return true;
	else
		return false;
}

bool ModeCaptureTheFlag::doesPlayerHaveItem( Player *player, const char *itemName )
{
	// We only care about flags

	if ( strnicmp( itemName, "ctfflag", 7 ) == 0 )
	{
		// See if the player has a flag

		if ( _playerCtfData[ player->entnum ]._hasFlag )
		{
			CtfFlag *ctfFlag;

			// Get the flag that the player has

			ctfFlag = findFlag( _playerCtfData[ player->entnum ]._flag );

			if ( !ctfFlag )
				return false;

			if ( ctfFlag->_realFlag )
			{
				// See if this is the flag in question

				if ( stricmp( ctfFlag->_realFlag->getName().c_str(), itemName ) == 0 )
				{
					return true;
				}
			}
		}
	}

	return false;
}

int ModeCaptureTheFlag::getStat( Player *player, int statNum, int value )
{
	CtfFlag *ctfFlag;
	int i;
	str flagName;
	bool oneFlag;
	int takenIconIndex;
	int missingIconIndex;
	int inBaseIconIndex;

	if ( ( statNum == STAT_MP_GENERIC5 ) || ( statNum == STAT_MP_GENERIC6 ) )
	{
		// Make sure not one flag

		oneFlag = !multiplayerManager.checkRule ( "flagpickup-enemyflag", true );

		if ( oneFlag && statNum == STAT_MP_GENERIC6 )
			return value;

		if ( oneFlag )
		{
			flagName = "ctfflag-one";
			takenIconIndex   = _oneFlagTakenIconIndex;
			missingIconIndex = _oneFlagMissingIconIndex;
			inBaseIconIndex  = _oneFlagInBaseIconIndex;
		}
		else if ( statNum == STAT_MP_GENERIC5 )
		{
			flagName = "ctfflag-red";
			takenIconIndex   = _redFlagTakenIconIndex;
			missingIconIndex = _redFlagMissingIconIndex;
			inBaseIconIndex  = _redFlagInBaseIconIndex;
		}
		else
		{
			flagName = "ctfflag-blue";
			takenIconIndex   = _blueFlagTakenIconIndex;
			missingIconIndex = _blueFlagMissingIconIndex;
			inBaseIconIndex  = _blueFlagInBaseIconIndex;
		}

		for ( i = 1 ; i <= _flags.NumObjects() ; i++ )
		{
			ctfFlag = &_flags.ObjectAt( i );

			if ( strnicmp( ctfFlag->_realFlag->getName().c_str(), flagName.c_str(), flagName.length() ) == 0 )
			{
				if ( ctfFlag->_tempFlag )
				{
					return missingIconIndex;
				}
				else if ( ctfFlag->_realFlag->getSolidType() == SOLID_NOT )
				{
					return takenIconIndex;
				}
				else
				{
					return inBaseIconIndex;
				}
			}
		}
	}
	else if ( statNum == STAT_MP_STATE )
	{
		return MultiplayerModeBase::getStat( player, statNum, value );
	}

	return value;
}

void ModeCaptureTheFlag::playerEventNotification( const char *eventName, const char *eventItemName, Player *eventPlayer )
{
	if ( stricmp( eventName, "use-HoldableItem" ) == 0 )
	{
		if ( stricmp( eventItemName, "Transporter" ) == 0 )
		{
			dropFlag( eventPlayer );
		}
	}
}

void ModeCaptureTheFlag::playerChangedModel( Player *player )
{
	Entity *flag;

	if ( _playerCtfData[ player->entnum ]._hasFlag && _playerCtfData[ player->entnum ]._carriedFlag )
	{
		flag = _playerCtfData[ player->entnum ]._carriedFlag;

		flag->detach();

		attachFlag( player, flag );
	}

	updatePlayerSkin( player );
}

void ModeCaptureTheFlag::attachFlag( Player *player, Entity *obj )
{
	int tagnum;

	tagnum = gi.Tag_NumForName( player->edict->s.modelindex, "Bip01 spine2" );

	if ( tagnum < 0 )
		tagnum = gi.Tag_NumForName( player->edict->s.modelindex, "Bip01 spine1" );

	if ( tagnum < 0 )
		tagnum = gi.Tag_NumForName( player->edict->s.modelindex, "Bip01 Head" );

	if ( ( tagnum < 0 ) || !obj->attach( player->entnum, tagnum, false, player->getFlagAttachOffset(), player->getFlagAttachAngles() ) )
	{
		warning( "ModeCaptureTheFlag::attachFlag", "Could not attach model %s", obj->model.c_str() );
		delete obj;
		return;
	}
}

