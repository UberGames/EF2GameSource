//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/mp_modeTeamBase.cpp                        $
// $Revision:: 59                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
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
#include "mp_modeBase.hpp"
#include "mp_modeTeamBase.hpp"


CLASS_DECLARATION( MultiplayerModeBase, ModeTeamBase, NULL )
{
	{ NULL, NULL }
};


//-----------------------------------------------------------------
//                   T E A M   A R E N A
//-----------------------------------------------------------------

//================================================================
// Name:        ModeTeamBase
// Class:       ModeTeamBase
//              
// Description: Constructor
//              
// Parameters:  const str& -- name of the arena
//              
// Returns:     None
//              
//================================================================
ModeTeamBase::ModeTeamBase()
{
	_maxTeams = 2;

	_leadTeam = NULL;

	_useTeamSpawnpoints = false;

	_redTeamIconIndex	= gi.imageindex( "sysimg/icons/mp/team_red" );
	_blueTeamIconIndex	= gi.imageindex( "sysimg/icons/mp/team_blue" );

	_redTeamHudIconIndex	= gi.imageindex( "sysimg/icons/mp/team_red_hud" );
	_blueTeamHudIconIndex	= gi.imageindex( "sysimg/icons/mp/team_blue_hud" );

	_redTeamSpectatorHudIconIndex	= gi.imageindex( "sysimg/icons/mp/team_red_spectator" );
	_blueTeamSpectatorHudIconIndex	= gi.imageindex( "sysimg/icons/mp/team_blue_spectator" );
}


//================================================================
// Name:        ~ModeTeamBase
// Class:       ModeTeamBase
//              
// Description: Destructor
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================

ModeTeamBase::~ModeTeamBase()
{
	_deleteTeams();
}

void ModeTeamBase::init( int maxPlayers )
{
	MultiplayerModeBase::init( maxPlayers );

	multiplayerManager.cacheMultiplayerFiles( "mp_teamBase" );
}

//================================================================
// Name:        isEndOfMatch   
// Class:       ModeTeamBase
//              
// Description: Determines if either team has met the frag limit.
//              If so, announces a winner (should the base team 
//              arena be so bold?). 
//
//              If there is no frag limit set, no check is made.
//              
// Parameters:  None
//              
// Returns:     bool -- true if the match has ended based on fraglimit
//              
//================================================================
bool ModeTeamBase::isEndOfMatch( void )
{
	if (!getPointLimit())
	{
		return false ;
	}

	for (int idx=1; idx <= _teamList.NumObjects(); idx++) 
	{
		Team* team = _teamList.ObjectAt(idx);
		if (static_cast<int>(team->getDeaths()) > getPointLimit())
		{
			multiplayerManager.centerPrintAllClients(va("$$%s$$ $$TeamLoses$$\n", team->getName().c_str() ), CENTERPRINT_IMPORTANCE_NORMAL );

			if ( team->getName() == "Red" )
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_redtlose.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
			else
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bltlose.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
		
			return true ;
		}
	}
	return false ;
}

//================================================================
// Name:        AddPlayer
// Class:       ModeTeamBase
//              
// Description: Adds a player to the arena.  If force join is on,
//              this player will be force to join with the fewest
//              players.
//              
// Parameters:  Player* -- player to add
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::AddPlayer( Player *player )
{	
	// Make sure everything is ok

	assert(player);

	if ( !player )
	{
		warning("ModeDeathmatch::AddPlayer", "NULL Player\n");
		return ; 
	}

	// Make sure player hasn't alrady been added

	if ( !needToAddPlayer( player ) )
		return;

	MultiplayerModeBase::AddPlayer(player);	

	// Add player to a team or make him a spectator

	if ( playersLastTeam[ player->entnum ].length() )
	{
		addPlayerToTeam( player, getTeam( playersLastTeam[ player->entnum ] ) );
	}
	else if ( !multiplayerManager.checkFlag( MP_FLAG_NO_AUTO_JOIN_TEAM ) || multiplayerManager.checkFlag( MP_FLAG_AUTO_BALANCE_TEAMS ) || ( player->edict->svflags & SVF_BOT ) )
	{
		Team* pickedTeam   = 0 ;
		Team* team         = 0 ;
		int minimumPlayers = 0 ;

		// Automatically add this player to one of the teams

		for (int idx = 1; idx <= _teamList.NumObjects(); idx++)
		{
			team    = _teamList.ObjectAt(idx);
			
			assert(team);
			if (!team)
			{
				continue ;
			}
			
			int players = team->getActivePlayers();
			if ( (players < minimumPlayers) || (!pickedTeam) )
			{
				pickedTeam     = team ;
				minimumPlayers = players ;
			}
		}

		if (pickedTeam)
		{
			addPlayerToTeam( player, pickedTeam );
		}
	}
	else
	{
		// Not force joining of teams, start as a spectator

		addPlayerToTeam( player, NULL );
	}

	//if ( _playerGameData[ player->entnum ]._currentTeam )
	//	addPlayerToTeam( player, _playerGameData[ player->entnum ]._currentTeam );

	// If the game hasn't started yet just make the player a spectator

	if ( !_gameStarted )
	{
		multiplayerManager.makePlayerSpectator( player );
	}
}

void ModeTeamBase::addPlayerToTeam( Player *player, Team *team )
{
	Team *oldTeam;
	Entity *spawnPoint;


	MultiplayerModeBase::AddPlayer(player);	

	oldTeam = _playerGameData[ player->entnum ]._currentTeam;

	if ( team && ( oldTeam != team ) )
	{
		// Inform all of the players that the player has changed teams

		multiplayerManager.HUDPrintAllClients(va("%s $$Joined$$ $$%s$$ $$Team$$.\n", player->client->pers.netname, team->getName().c_str() ) );
	}

	if ( oldTeam )
	{
		_playerGameData[ player->entnum ]._currentTeam = NULL;

		player->SurfaceCommand( "all", "-skin1" );
		player->SurfaceCommand( "all", "-skin2" );

		oldTeam->RemovePlayer(player);
	}

	if ( team )
	{
		// Since the player is now on a team add him to the game

		if ( _gameStarted )
			multiplayerManager.playerEnterArena( player->entnum, player->health );
		else
			multiplayerManager.makePlayerSpectator( player );

		// Add the player to the team

		team->AddPlayer(player);

		_playerGameData[ player->entnum ]._currentTeam = team;

		updatePlayerSkin( player );

		if ( team->getName() == "Red" )
		{
			if ( multiplayerManager.isPlayerSpectator( player ) )
				multiplayerManager.setTeamHud( player, "mp_teamredspec" );
			else
				multiplayerManager.setTeamHud( player, "mp_teamred" );
		}
		else
		{
			if ( multiplayerManager.isPlayerSpectator( player ) )
				multiplayerManager.setTeamHud( player, "mp_teambluespec" );
			else
				multiplayerManager.setTeamHud( player, "mp_teamblue" );
		}

		multiplayerManager.playerSpawned( player );

		playersLastTeam[ player->entnum ] = team->getName();
	}
	else
	{
		// No team selected so make the player a spectator

		multiplayerManager.makePlayerSpectator( player, SPECTATOR_TYPE_FOLLOW, true );

		/* team = _playerGameData[ player->entnum ]._currentTeam;

		if ( !team )
			multiplayerManager.setTeamHud( player, "mp_teamspec" );
		else if ( team->getName() == "Red" )
			multiplayerManager.setTeamHud( player, "mp_teamredspec" );
		else
			multiplayerManager.setTeamHud( player, "mp_teambluespec" ); */
	}

	// Warp player to a spawn point

	spawnPoint = getSpawnPoint( player );

	if ( spawnPoint )
	{
		player->WarpToPoint( spawnPoint );
	}

	if ( team && _gameStarted )
	{
		KillBox( player );

		ActivatePlayer( player );
	}
}

void ModeTeamBase::changeTeams( Player *player, Team *team )
{
	RemovePlayer( player );
	addPlayerToTeam( player, team );
}

void ModeTeamBase::respawnPlayer( Player *player )
{
	Entity *spawnPoint;

	if ( !player )
	{
		assert(player);
		warning("ModeDeathmatch::respawnPlayer", "NULL player\n");
		return;
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

	MultiplayerModeBase::respawnPlayer( player );

	multiplayerManager.initPlayer( player );

	spawnPoint = getSpawnPoint( player );

	if ( spawnPoint )
	{
		player->WarpToPoint( spawnPoint );
	}

	KillBox( player );

	ActivatePlayer( player );
}

Entity *ModeTeamBase::getSpawnPoint( Player *player )
{
	Entity *spawnPoint = NULL;
	int i;
	int numSpawnPoints = 0;
	str spawnpointName;
	int randomStartingSpot;
	int spawnPointIndex;


	// Determine what kind of spawn point we want to use

	if ( multiplayerManager.checkRule( "spawnpoints-special", false, player ) )
	{
		spawnpointName = multiplayerManager.getSpawnPointType( player );

		numSpawnPoints = getNumNamedSpawnpoints( spawnpointName );
	}

	if ( !numSpawnPoints && multiplayerManager.checkRule( "spawnpoints-team", _useTeamSpawnpoints ) && _playerGameData[ player->entnum ]._currentTeam )
	{
		if ( stricmp( _playerGameData[ player->entnum ]._currentTeam->getName(), "red" ) == 0 )
			spawnpointName = "red";
		else
			spawnpointName = "blue";

		numSpawnPoints = getNumNamedSpawnpoints( spawnpointName );
	}

	if ( !numSpawnPoints )
	{
		spawnpointName = "";

		numSpawnPoints = getNumNamedSpawnpoints( spawnpointName );
	}

	randomStartingSpot = ( (int)( G_Random() * numSpawnPoints ) );

	for( i = 0 ; i < numSpawnPoints ; i++ )
	{
		spawnPointIndex = ( randomStartingSpot + i ) % numSpawnPoints;

		spawnPoint = getNamedSpawnpointbyIndex( spawnpointName, spawnPointIndex );

		// Make sure we don't telefrag someone on our team

		if ( spawnPoint )
		{
			int j;
			int num;
			int touch[ MAX_GENTITIES ];
			gentity_t *hit;
			Vector min;	
			Vector max;
			bool badSpot;
	
			min = spawnPoint->origin + player->mins + Vector( 0, 0, 1 );
			max = spawnPoint->origin + player->maxs + Vector( 0, 0, 1 );
	
			num = gi.AreaEntities( min, max, touch, MAX_GENTITIES, qfalse );

			badSpot = false;
	
			for( j = 0 ; j < num ; j++ )
			{
				hit = &g_entities[ touch[ j ] ];
		
				if ( !hit->inuse || ( hit->entity == player ) || !hit->entity || ( hit->entity == world ) || ( !hit->entity->edict->solid ) )
				{
					continue;
				}

				if ( hit->entity->isSubclassOf( Player ) )
				{
					badSpot = true;
					break;
				}
			}

			if ( badSpot )
			{
				continue;
			}
		}

		// This is a good spawn point so use it

		return spawnPoint;
	}

	// Just return the last spawn point found

	return spawnPoint;
}

//================================================================
// Name:        AddTeam
// Class:       ModeTeamBase
//              
// Description: Adds a team to the arena.  Checks to ensure that the
//              max number of teams has not yet been exceeded.
//              
// Parameters:  const str& -- new name of team
//              
// Returns:     Team* -- Created team (NULL if failed to create)
//              
//================================================================
Team* ModeTeamBase::AddTeam( const str& teamName )
{
	Team* team = _findTeamByName(teamName);

	if (team)
	{
		warning( "ModeTeamBase::AddTeam", va("Team %s already exists\n", teamName.c_str() ) );
		return NULL ;
	}

	team = new Team(teamName);
	team->setMaxPlayers(getMaxPlayers() / getMaxTeams());
	_teamList.AddObject( team );

	return team ;
}

Team *ModeTeamBase::getTeam( const str & teamName )
{
	int i;
	Team *team;

	for ( i = 1 ; i <= _teamList.NumObjects() ; i++ )
	{
		team = _teamList.ObjectAt( i );

		if ( team->getName() == teamName )
		{
			return team;
		}
	}

	return NULL;
}


//================================================================
// Name:        RemoveTeam
// Class:       ModeTeamBase
//              
// Description: Removes and deletes the team specified by name from
//              the arena.  Calls _deleteTeam to do actual removal.
//              
// Parameters:  
//              
// Returns:     
//              
//================================================================
void ModeTeamBase::RemoveTeam( const str& teamName )
{
	Team* team = _findTeamByName(teamName);
	_deleteTeam(team);
}



//================================================================
// Name:        AddTeamStartingAmmo
// Class:       ModeTeamBase
//              
// Description: Adds team-specific ammo.  This enables a particular
//              team to receive a specific amount and/or type of ammo.
//              Calls _addTeamStartingAmmo() to actually give it to the team.
//              
// Parameters:  const str& teamName -- team to receive the ammo
//              const str& ammoName -- name of ammo to give
//              int        amount   -- amount of ammo to give
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::AddTeamStartingAmmo( const str& teamName, const str& ammoName, int amount )
{
	Team* team = _findTeamByName(teamName);
	SimpleAmmoType ammoType(ammoName, amount);
	_addTeamStartingAmmo(team, ammoType);
}

//================================================================
// Name:        AddTeamStartingWeapon
// Class:       ModeTeamBase
//              
// Description: Adds team-specific weapon.  This enables a particular
//              team to receive a specific weapon. Calls _addTeamStartingWeapon() 
//              to actually give it to the team.
//              
// Parameters:  const str& teamName   -- team to receive the ammo
//              const str& weaponName -- name of weapon to give
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::AddTeamStartingWeapon( const str& teamName, const str& weaponName )
{
	Team* team = _findTeamByName(teamName);
	_addTeamStartingWeapon(team, weaponName);
}

//================================================================
// Name:        setTeamStartingHealth
// Class:       ModeTeamBase
//              
// Description: Sets the starting health for the team.  Calls
//              _setTeamStartingHealth to do actual work.
//              
// Parameters:  const str&   -- team name
//              unsigned int -- amount of health
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::SetTeamStartingHealth( const str &teamName, unsigned int startingHealth )
{
	Team* team = _findTeamByName(teamName);
	_setTeamStartingHealth(team, startingHealth);
}

//----------------------------------------------------------------
//             P R O T E C T E D   M E T H O D S 
//----------------------------------------------------------------


//================================================================
// Name:        _addTeamStartingAmmo
// Class:       ModeTeamBase
//              
// Description: Adds the specified ammo to the specified team.
//              
// Parameters:  Team*          -- team to receive the ammo
//              SimpleAmmoType -- ammo to receive 
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::_addTeamStartingAmmo( Team* team, const SimpleAmmoType &ammo )
{
	// Check for valid team
	assert(team);
	if (!team)
	{
		warning("ModeTeamBase::_addTeamStartingAmmo", "NULL team passed\n");
		return ;
	}

	// Check for valid ammo type 
	assert(ammo.type.length());
	if (!ammo.type.length())
	{
		warning("ModeTeamBase::_addTeamStartingAmmo", "No ammo type specified\n");
		return ;
	}

	// Check for valid amount
	assert(ammo.amount > 0);
	if (ammo.amount < 0)
	{
		warning("ModeTeamBase::_addTeamStartingAmmo", "Negative ammo amount specified\n");
		return ;
	}

	team->AddStartingAmmo(ammo);
}


//================================================================
// Name:        _addTeamStartingWeapon
// Class:       ModeTeamBase
//              
// Description: Adds the specified weapon to the specified team.
//              
// Parameters:  Team*       -- team to receive the ammo
//              const str&  -- weapon to receive 
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::_addTeamStartingWeapon( Team* team, const str& weaponName )
{
	// Check for valid team
	assert(team);
	if (!team)
	{
		warning("ModeTeamBase::_addTeamStartingWeapon", "NULL team passed\n");
		return ;
	}

	// Check for valid weaponName
	assert(weaponName.length());
	if (!weaponName.length())
	{
		warning("ModeTeamBase::_addTeamStartingWeapon", "No weapon specified\n");
		return ;
	}

	team->AddStartingWeapon(weaponName);
}


//================================================================
// Name:        _setTeamStartingHealth
// Class:       ModeTeamBase
//              
// Description: Sets the specified team's starting health to the
//              specified value.  A starting health of 0 means to
//              use the arena's default value (typicaly 100).
//              
// Parameters:  Team*        -- team to affect
//              unsigned int -- amount of starting health (must be >= 0)
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::_setTeamStartingHealth( Team* team, unsigned int startingHealth )
{
	// Check for valid team
	assert(team);
	if (!team)
	{
		warning("ModeTeamBase::_addTeamStartingWeapon", "NULL team passed\n");
		return ;
	}

	/* assert(startingHealth >= 0);
	if (startingHealth < 0)
	{
		warning("ModeTeamBase::_setTeamStartingHealth", va("Cannot set health to %d\n", startingHealth));
		return ;
	} */

	team->setStartingHealth(startingHealth);
}

//================================================================
// Name:        _deleteTeams
// Class:       ModeTeamBase
//              
// Description: Deletes all the teams currently in the arena.
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::_deleteTeams()
{
	for (int idx = 1; idx <= _teamList.NumObjects(); idx++)
	{
		Team* team = _teamList.ObjectAt(idx);
		_deleteTeam(team);
	}
}

//================================================================
// Name:        _deleteTeam
// Class:       ModeTeamBase
//              
// Description: Removes the specified team from the arena and deletes it.
//              Minimal error checking since it is an internal function.
//              
// Parameters:  Team* -- team to remove and delete
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::_deleteTeam( Team* team )
{
	assert(team);
	if (!team)
	{
		warning("ModeTeamBase::_removeTeam", "NULL Team passed\n");
		return ;
	}

	_teamList.RemoveObject(team);
	delete team ;
}



//================================================================
// Name:        _beginMatch
// Class:       ModeTeamBase
//              
// Description: Called when a match begins.  Calls BeginMatch on
//              every team in the arena.  The teams are responsible
//              for preparing the participants for battle.
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::_beginMatch()
{
	for (int idx = 1; idx <= _teamList.NumObjects(); idx++)
	{
		Team* team = _teamList.ObjectAt(idx);
		team->BeginMatch();
	}
}


//================================================================
// Name:        _endMatch
// Class:       ModeTeamBase
//              
// Description: Called when a match ends.  Calls EndMatch on
//              every team in the arena.  The teams are responsible
//              for removing the players from battle-readiness.
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void ModeTeamBase::_endMatch()
{
	for (int idx = 1; idx <= _teamList.NumObjects(); idx++)
	{
		Team* team = _teamList.ObjectAt(idx);
		team->EndMatch();
	}
}

void ModeTeamBase::declareWinner( void )
{
	int redPoints;
	int bluePoints;

	redPoints  = getTeamPoints( "Red" );
	bluePoints = getTeamPoints( "Blue" );

	if ( redPoints > bluePoints )
	{
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rtwins.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
	}
	else if ( bluePoints > redPoints )
	{
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_btwins.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
	}
	/* else
	{
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_matover.mp3" );
	} */
}


//================================================================
// Name:        _findTeamByName
// Class:       ModeTeamBase
//              
// Description: Retrieves a pointer to the team having the specified name.
//              
// Parameters:  const str& -- name of the team to retrieve
//              
// Returns:     Team* -- pointer to team found (NULL if not found)
//              
//================================================================
Team* ModeTeamBase::_findTeamByName( const str& teamName )
{
	assert(teamName.length());
	if (!teamName.length())
	{
		warning("ModeTeamBase::_findTeamByName", "Team name not specified\n");
		return NULL ;
	}

	Team* team = NULL ;
	for (int idx = 1; idx <= _teamList.NumObjects(); idx++)
	{
		team = _teamList.ObjectAt(idx);
		if ( stricmp( team->getName().c_str(), teamName.c_str() ) == 0 )
		{
			return team ;
		}
	}

	return NULL ;
}

int ModeTeamBase::getIcon( Player *player, int statNum, int value )
{
	/* if ( statNum == STAT_MP_TEAMHUD_ICON )
	{
		Team *team;

		team = _playerGameData[ player->entnum ]._currentTeam;

		if ( team )
		{
			// Player is on a team so return the appropriate team icon

			if ( multiplayerManager.isPlayerSpectator( player ) )
			{
				if ( team->getName() == "Red" )
					return _redTeamSpectatorHudIconIndex;
				else if ( team->getName() == "Blue" )
					return _blueTeamSpectatorHudIconIndex;
			}
			else
			{
				if ( team->getName() == "Red" )
					return _redTeamHudIconIndex;
				else if ( team->getName() == "Blue" )
					return _blueTeamHudIconIndex;
			}
		}
	}
	else */ if ( statNum == STAT_MP_TEAM_ICON )
	{
		Team *team;

		team = _playerGameData[ player->entnum ]._currentTeam;

		if ( team )
		{
			// Player is on a team so return the appropriate team icon

			if ( team->getName() == "Red" )
				return _redTeamIconIndex;
			else if ( team->getName() == "Blue" )
				return _blueTeamIconIndex;
		}
	}
	else if ( statNum == STAT_MP_OTHERTEAM_ICON )
	{
		Team *team;

		team = _playerGameData[ player->entnum ]._currentTeam;

		if ( team )
		{
			// Player is on a team so return the appropriate team icon

			if ( team->getName() == "Red" )
				return _blueTeamIconIndex;
			else if ( team->getName() == "Blue" )
				return _redTeamIconIndex;
		}
	}

	// We didn't return an icon so let the base mode have a chance

	return MultiplayerModeBase::getIcon( player, statNum, value );
}

int ModeTeamBase::getInfoIcon( Player *player )
{
	Team *team;

	// Make sure entity is not invisible

	if ( player->_affectingViewModes & gi.GetViewModeMask( "forcevisible" ) )
		return 0;

	team = _playerGameData[ player->entnum ]._currentTeam;

	if ( team )
	{
		if ( team->getName() == "Red" )
			return _redTeamIconIndex;
		else if ( team->getName() == "Blue" )
			return _blueTeamIconIndex;
	}

	return 0;
}

bool ModeTeamBase::canJoinTeam( Player *player, const str &teamName )
{
	Team *team;

	if ( multiplayerManager.checkFlag( MP_FLAG_AUTO_BALANCE_TEAMS ) )
	{
		Team *team;
		Team *redTeam;
		Team *blueTeam;
		int redTeamPlayers;
		int blueTeamPlayers;

		// Get all of the teams

		team = getTeam( teamName );
		redTeam = getTeam( "Red" );
		blueTeam = getTeam( "Blue" );

		// Get the number of players

		redTeamPlayers  = redTeam->getActivePlayers();
		blueTeamPlayers = blueTeam->getActivePlayers();

		if ( _playerGameData[ player->entnum ]._currentTeam == redTeam )
			redTeamPlayers -= 1;
		else if ( _playerGameData[ player->entnum ]._currentTeam == blueTeam )
			blueTeamPlayers -= 1;

		// Don't allow switch unless it's to the team that has less people

		if ( ( redTeamPlayers > blueTeamPlayers ) && ( team == redTeam ) )
		{
			multiplayerManager.centerPrint( player->entnum, "$$CannotJoinAutoBalanceRed$$", CENTERPRINT_IMPORTANCE_HIGH );
			return false;
		}
		else if ( ( blueTeamPlayers > redTeamPlayers ) && ( team == blueTeam ) )
		{
			multiplayerManager.centerPrint( player->entnum, "$$CannotJoinAutoBalanceBlue$$", CENTERPRINT_IMPORTANCE_HIGH );
			return false;
		}
	}

	if ( !multiplayerManager.checkRule( "respawnPlayer", true, player ) )
	{
		return false;
	}

	team = _findTeamByName( teamName );

	if ( _playerGameData[ player->entnum ]._currentTeam == team )
		return false;
	else
		return true;
}

void ModeTeamBase::joinTeam( Player *player, const str &teamName )
{
	Team *team;

	team = _findTeamByName( teamName );

	changeTeams( player, team );
}

float ModeTeamBase::playerDamaged( Player *damagedPlayer, Player *attackingPlayer, float damage, int meansOfDeath )
{
	// Always take telefrag damage

	if ( meansOfDeath == MOD_TELEFRAG )
		return damage;

	// Player can always hurt himself

	if ( damagedPlayer == attackingPlayer )
		return damage;

	// If on same team and not allowing team damage

	if ( ( _playerGameData[ damagedPlayer->entnum ]._currentTeam == _playerGameData[ attackingPlayer->entnum ]._currentTeam ) &&
		 ( !multiplayerManager.checkFlag( MP_FLAG_FRIENDLY_FIRE ) ) )
		return 0;
	else
		return damage;
}

int ModeTeamBase::getTeamPoints( Player *player )
{
	Team *team;

	// Return the points for this team

	team = _playerGameData[ player->entnum ]._currentTeam;

	if ( team )
		return team->getPoints();
	else
		return 0;
}

int ModeTeamBase::getTeamPoints( const str & teamName )
{
	int i;
	Team *team;

	// Return the points for this team

	for ( i = 1 ; i <= _teamList.NumObjects() ; i++ )
	{
		team = _teamList.ObjectAt( i );

		if ( team->getName() == teamName )
		{
			return team->getPoints();
		}
	}

	return 0;
}

void ModeTeamBase::addTeamPoints( const str & teamName, int points )
{
	int i;
	Team *team;

	// Return the points for this team

	for ( i = 1 ; i <= _teamList.NumObjects() ; i++ )
	{
		team = _teamList.ObjectAt( i );

		if ( team->getName() == teamName )
		{
			team->addPoints( NULL, points );
		}
	}
}

void ModeTeamBase::teamPointsChanged( Team *team, int oldPoints, int newPoints )
{
	Team *otherTeam;
	int otherTeamPoints;

	if ( team->getName() == "Red" )
		otherTeam = getTeam( "Blue" );
	else
		otherTeam = getTeam( "Red" );

	otherTeamPoints = otherTeam->getPoints();

	// Play a dialog for this team if necessary

	if ( ( newPoints == otherTeamPoints ) && ( oldPoints != otherTeamPoints ) )
	{
		// Now tied for the lead
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_teamstied.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.0f );
	}
	else if ( ( newPoints > otherTeamPoints ) && ( oldPoints <= otherTeamPoints ) )
	{
		// Now in first place
		multiplayerManager.teamSound( team, "localization/sound/dialog/dm/comp_team1stplace.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );

		// No longer in the lead
		multiplayerManager.teamSound( otherTeam, "localization/sound/dialog/dm/comp_teamnolead.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
	}
	else if ( ( newPoints < otherTeamPoints ) && ( oldPoints >= otherTeamPoints ) )
	{
		// No longer in the lead
		multiplayerManager.teamSound( team, "localization/sound/dialog/dm/comp_teamnolead.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );

		// Now in first place
		multiplayerManager.teamSound( otherTeam, "localization/sound/dialog/dm/comp_team1stplace.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
	}
}

void ModeTeamBase::RemovePlayer( Player *player )
{
	Team *team;

	team = _playerGameData[ player->entnum ]._currentTeam;

	if ( team )
	{
		team->RemovePlayer( player );
	}

	MultiplayerModeBase::RemovePlayer( player );
}

int ModeTeamBase::getHighestPoints( void )
{
	int i;
	int highestPoints = -999999999;
	Team *team;

	for ( i = 1 ; i <= _teamList.NumObjects() ; i++ )
	{
		team = _teamList.ObjectAt( i );

		if ( team->getPoints() > highestPoints )
		{
			highestPoints = team->getPoints();
		}
	}

	return highestPoints;
}

void ModeTeamBase::updatePlayerSkin( Player *player )
{
	Team *team;

	team = getPlayersTeam( player );

	player->SurfaceCommand( "all", "-skin1" );
	player->SurfaceCommand( "all", "-skin2" );

	if ( !team )
		return;

	if ( team->getName() == "Red" )
	{
		player->SurfaceCommand( "all", "+skin1" );
	}
	else
	{
		player->SurfaceCommand( "all", "+skin2" );
	}
}

void ModeTeamBase::playerChangedModel( Player *player )
{
	updatePlayerSkin( player );
}

