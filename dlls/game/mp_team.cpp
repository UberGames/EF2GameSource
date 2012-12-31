//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/mp_team.cpp                                    $
// $Revision:: 8                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 5/20/03 11:39a                                                 $
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
#include "mp_team.hpp"

//------------------------------------------------------------------
//                        T E A M
//------------------------------------------------------------------

// Connect events to class methods
CLASS_DECLARATION( Class, Team, NULL )
{
	{ NULL, NULL }
};

//================================================================
// Name:        Team
// Class:       Team
//              
// Description: Constructor
//              
// Parameters:  const str& -- name of the team
//              
// Returns:     None
//              
//================================================================
Team::Team( const str& name ) :	
	_name(name)
{
	_startingHealth = 0;
	_maxPlayers = 5;
	_wins = 0;
	_losses = 0;
	_kills = 0;
	_deaths = 0;
	_points = 0;
	_isFighting = false;
}


//================================================================
// Name:        ~Team
// Class:       Team
//              
// Description: Destructor
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
Team::~Team()
{
}

//================================================================
// Name:        AddDeath
// Class:       Team
//              
// Description: Adds a death for this team if the specified victim
//              is indeed on the team.  Also notifies all participants
//              in the arena of the event.
//              
// Parameters:  Player *victim -- this team's player who died
//              Player *killer -- opposing team's player who made the kill.  Not used in base class
//              
// Returns:     None
//              
//================================================================
void Team::AddDeath( Player* victim, Player* )
{
	assert(victim);
	if (!victim)
	{
		warning("Team::AddDeath", "NULL Player\n");
		return ;
	}

	if (!_playerList.ObjectInList(victim))
	{
		warning("Team::AddDeath", va("%s is not a member of %s", victim->client->pers.netname, getName().c_str() ) );
		return ;
	}

	_deaths++ ;		
}

//================================================================
// Name:        AddKill
// Class:       Team
//              
// Description: Adds a kill for this team if the specified killer
//              is indeed on the team.
//              
// Parameters:  Player* killer -- this team's player who made the kill
//              Player* victim -- opposing team's player who died.  Not used base class
//              
// Returns:     None
//              
//================================================================
void Team::AddKill( Player *killer, Player * )
{
	assert(killer);
	if (!killer)
	{
		warning("Team::AddKill", "NULL Player\n");
		return ;
	}

	if (!_playerList.ObjectInList(killer))
	{
		warning("Team::AddKill", va("%s is not a member of %s", killer->client->pers.netname, getName().c_str() ) );
		return ;
	}

	_kills++ ;
}

void Team::addPoints( Player *killer, int points )
{
	int oldPoints;

	Q_UNUSED(killer);

	/* if ( killer && !_playerList.ObjectInList(killer))
	{
		assert( !"Player not in list" );
		warning("Team::AddKill", va("%s is not a member of %s", killer->client->pers.netname, getName().c_str() ) );
		return ;
	} */

	oldPoints = _points;

	_points += points;

	multiplayerManager.teamPointsChanged( this, oldPoints, _points );
}

//================================================================
// Name:        Win
// Class:       Team
//              
// Description: Increments number of wins for this team.
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void Team::Win()
{
	_wins++ ;
}



//================================================================
// Name:        Lose
// Class:       Team
//              
// Description: Increments number of losses for this team
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void Team::Lose()
{
	_losses++ ;
}

//================================================================
// Name:        BeginMatch
// Class:       Team
//              
// Description: Activates all the players on the team
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void Team::BeginMatch()
{
	for (int idx = 1; idx <= _playerList.NumObjects(); idx++)
	{
		Player *player = _playerList.ObjectAt(idx);
		assert(player);
		if (!player)
		{
			warning("Team::endMatch", "NULL Player in team list\n");
			continue ;
		}
		_activatePlayer(player);
	}
}

//================================================================
// Name:        EndMatch
// Class:       Team
//              
// Description: Deactivates all the players on the team
//              (turns off fighting).
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void Team::EndMatch()
{
	for (int idx = 1; idx <= _playerList.NumObjects(); idx++)
	{
		Player *player = _playerList.ObjectAt(idx);
		assert(player);
		if (!player)
		{
			warning("Team::endMatch", "NULL Player in team list\n");
			continue ;
		}
		_deactivatePlayer(player);
	}
}

//================================================================
// Name:        AddPlayer
// Class:       Team
//              
// Description: Adds a player to the team.
//              
// Parameters:  Player* -- player to add
//              
// Returns:     None
//              
//================================================================
void Team::AddPlayer( Player* player )
{
	assert(player);
	if (!player)
	{
		warning("Team::AddPlayer", va("Attempt to add NULL player to %s\n", getName().c_str() ) );
		return ;
	}

	if (_playerList.NumObjects() == _maxPlayers)
	{
		warning("Team::AddPlayer", va("The %s team is full!\n", getName().c_str() ) );
		return ;
	}
	_playerList.AddObject(player);
	//player->SetMultiplayerTeam(this);
	
	// If we are in the middle of a match, prepare them for fighting
	if (isFighting())
	{
		_activatePlayer( player );
	}
}


//================================================================
// Name:        RemovePlayer
// Class:       Team
//              
// Description: Removes a player from the team
//              
// Parameters:  Player* -- player to remove
//              
// Returns:     None
//              
//================================================================
void Team::RemovePlayer( Player *player )
{
	assert(player);
	if (!player)
	{
		warning("Team::RemovePlayer", va("Attempt to add NULL player to %s\n", getName().c_str() ) );
		return ;
	}

	if ( !_playerList.ObjectInList( player ) )
	{
		warning("Team::RemovePlayer", va("Player %s is not on team %s\n", player->client->pers.netname, getName().c_str() ) );
		return ;
	}
	_playerList.RemoveObject(player);
	//player->SetMultiplayerTeam(NULL);
}


//================================================================
// Name:        AddStartingAmmo
// Class:       Team
//              
// Description: Adds the specified ammo to the list of initial ammo
//              to give players on this team.
//              
// Parameters:  SimpleAmmoType -- ammo to add
//              
// Returns:     None
//              
//================================================================
void Team::AddStartingAmmo( const SimpleAmmoType &ammo )
{
	_ammoList.AddObject(ammo);
}


//================================================================
// Name:        AddStartingWeapon
// Class:       Team
//              
// Description: Adds the specified starting weapon to the list of
//              weapons.
//              
// Parameters:  const str& -- weapon to add
//              
// Returns:     None
//              
//================================================================
void Team::AddStartingWeapon( const str& weaponName )
{
	_weaponList.AddObject(weaponName);
}

//----------------------------------------------------------------
//               P R O T E C T E D   M E T H O D S
//----------------------------------------------------------------


//================================================================
// Name:        _giveInitialConditions
// Class:       Team
//              
// Description: Gives the specified player the starting health, ammo,
//              and weapons for this team.  If starting health is 0,
//              no health is applied to the player meaning the player
//              will get the arena default (typically 100).
//              
// Parameters:  Player* -- player to recieve initial conditions
//              
// Returns:     None
//              
//================================================================
void Team::_giveInitialConditions( Player *player )
{
	assert(player);
	if (!player)
	{
		warning("Team::_giveWeaponsAndAmmo", "NULL Player passed\n");
		return ;
	}

	if (isStartingHealth())
	{
//			Event* healthEvent = new Event(EV_Set_Health);
//			healthEvent->AddInteger(getStartingHealth());
//			player->ProcessEvent(healthEvent);
	}

	int idx = 0;
	// Give weapons
	for (idx=1; idx <= _weaponList.NumObjects(); idx++)
	{
		multiplayerManager.givePlayerItem( player->entnum, _weaponList.ObjectAt(idx) );
		//player->giveItem( _weaponList.ObjectAt(idx), 1 );
	}

	// Give ammo
	for (idx=1; idx <= _ammoList.NumObjects(); idx++)
	{
		SimpleAmmoType& ammo = _ammoList.ObjectAt(idx);
		player->giveItem( ammo.type, ammo.amount );
	}
}


//================================================================
// Name:        _activatePlayer
// Class:       Team
//              
// Description: Activates the specified player.  This player is 
//              given any special weapons and ammo and the appropriate
//              starting health (if any).
//
//              This routine also attempts to give the player a 
//              team speicfic spawn point if there is one.
//
//              
// Parameters:  Player* -- player to activate
//              
// Returns:     None
//              
//================================================================
void Team::_activatePlayer( Player *player )
{
	_giveInitialConditions(player);
	multiplayerManager.allowFighting( true );
	player->takedamage = DAMAGE_YES ;
}	



//================================================================
// Name:        _deactivatePlayer
// Class:       Team
//              
// Description: Removes the player from battle.  For now, this is
//              simply turning off the fighting enabled flag and
//              putting them into spectator mode.
//              
// Parameters:  Player* -- player to deactivate
//              
// Returns:     None
//              
//================================================================
void Team::_deactivatePlayer( Player *player )
{
	assert(player);
	if (!player)
	{
		warning("Team::_deactivatePlayer", va("NULL Player passed to %s team\n", getName().c_str() ) );
		return ;
	}

	multiplayerManager.allowFighting( false );
	player->takedamage = DAMAGE_NO ;
}
