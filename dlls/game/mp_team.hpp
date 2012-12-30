//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/multiplayerArena.cpp                           $
// $Revision:: 48                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 7/23/02 2:55p                                                  $
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

#include "mp_shared.hpp"

//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/multiplayerArena.cpp                           $
// $Revision:: 48                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 7/23/02 2:55p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// Description:
//

#ifndef __MP_TEAM_HPP__
#define __MP_TEAM_HPP__

#include "g_local.h"          // common game stuff
#include "player.h"           // for Player
#include "item.h"
#include "PlayerStart.h"      // for PlayerDeathmatchStart
#include "container.h"        // for Container
#include "str.h"              // for str

//-------------------------------------------------------------------
// SimpleAmmoType -- Defines a type of ammo and a quantity.  Used for
//                   starting players with specified ammo in multiplayer.
//-------------------------------------------------------------------
class SimpleAmmoType : public Class
	{
	public:
		SimpleAmmoType() : type(""), amount(0) { } 
		SimpleAmmoType(const str& ammoType, int ammoAmount) : type(ammoType), amount(ammoAmount) { }
		str   type;
		int   amount;
	};

//-----------------------------------------------------------------------
// Team -- A multiplayer team.  The team has a name, starting ammo, weapons,
//         and health, and skins.  The team class tracks kills and deaths
//         on a team basis.
//-----------------------------------------------------------------------
class Team : public Class
	{
	private:
		Container<Player*>                  _playerList ;       // Players on this team
		Container<str>                      _weaponList ;       // starting weapons for just this team
		Container<SimpleAmmoType>           _ammoList ;         // starting ammo for just this team
		Container<PlayerDeathmatchStart*>   _spawnpointList;    // spawnpoints for just this team
		unsigned int                        _maxPlayers ;       // Maximum players on this team
		unsigned int                        _startingHealth ;   // starting health for just this team
		unsigned int                        _wins ;             // matches won
		unsigned int                        _losses ;           // matches lost
		unsigned int                        _kills ;            // kills of an opposing team
		int									_points;
		unsigned int                        _deaths ;           // kills of this team
		bool                                _isFighting ;       // true if match has begun
		str                                 _name ;             // can be used to reference the team thru events

	protected:
		void                                _giveInitialConditions(Player *player); // gives initial health, ammo, and weapons to player
		void                                _activatePlayer(Player* player);        // prepares player for fight
		void                                _deactivatePlayer(Player* player);      // turns off fighting for player

	public:
		CLASS_PROTOTYPE( Team );
		Team(const str& name="Unnamed Team");
		virtual ~Team();

		// Queries
		bool                                isStartingHealth()                  { return (_startingHealth > 0) ? true : false ; }
		bool                                isFighting()                        { return _isFighting ; }

		// Gets
		const str&                          getName()                           { return _name ; }
		unsigned int                        getKills()                          { return _kills ; }
		unsigned int                        getDeaths()                         { return _deaths ; }
		int			                        getPoints()                         { return _points ; }
		unsigned int                        getWins()                           { return _wins ; }
		unsigned int                        getLosses()                         { return _losses ; }
		unsigned int                        getMaxPlayers()                     { return _maxPlayers ; }
		unsigned int                        getActivePlayers()                  { return _playerList.NumObjects(); }

		// Sets
		void                                setStartingHealth(unsigned int h)   { _startingHealth = h ; }
		void                                setMaxPlayers(unsigned int players) { _maxPlayers = players ; }
		
		// Match functions
		void                                BeginMatch();
		void                                EndMatch();

		// Player functions
		void                                AddPlayer(Player* player);
		void                                RemovePlayer(Player* player);

		// Initial conditions functions
		void                                AddStartingWeapon(const str& weaponName);
		void                                AddStartingAmmo( const SimpleAmmoType &ammo);
		void                                AddSpawnpoint(PlayerDeathmatchStart* spawnpoint);

		// Stat functions
		void                                AddKill(Player* killer, Player* victim=NULL);  // try to track who we killed
		void                                AddDeath(Player* victim, Player* killer=NULL); // try to track who killed us
		void								addPoints( Player *killer, int points );
		void                                Win();
		void                                Lose();
};

#endif // __MP_TEAM_HPP__
