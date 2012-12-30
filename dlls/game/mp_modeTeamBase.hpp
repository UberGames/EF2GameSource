//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/multiplayerArena.h                             $
// $Revision:: 38                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 7/23/02 3:55p                                                  $
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


#ifndef __MP_MODETEAMBASE_HPP__
#define __MP_MODETEAMBASE_HPP__

#include "mp_modeBase.hpp"

//-----------------------------------------------------------------------
// ModeTeamBase --  Abstract base class providing common team-based 
//               multiplayer functionality.  Uses the Team class to 
//               compose multiple teams of players.  Tracks team
//               kills and assigns skins and such.
//-----------------------------------------------------------------------
class ModeTeamBase : public MultiplayerModeBase
	{
	protected:
		Container<Team *>					_teamList;      // list of teams
		unsigned int                        _maxTeams ;     // maximum number of teams allowed (default is 2)
		Team*								_leadTeam ;     // If set, this team is currently "winning"
		bool								_useTeamSpawnpoints;

		int									_redTeamIconIndex;
		int									_blueTeamIconIndex;

		int									_redTeamHudIconIndex;
		int									_blueTeamHudIconIndex;

		int									_redTeamSpectatorHudIconIndex;
		int									_blueTeamSpectatorHudIconIndex;

		// Abstract constructor
											ModeTeamBase();
	
		/* virtual */ void					init( int maxPlayers );

		/* virtual */ void					declareWinner( void );

		/* virtual */ void					_endMatch();    // Notifies teams of end of match
		/* virtual */ void                  _beginMatch();  // Notifies teams of start of match

		// Utility functions
		Team*                               _findTeamByName(const str& teamName);
		void                                _deleteTeams();
		void                                _deleteTeam(Team* team);
		void                                _addTeamStartingAmmo(Team* team, const SimpleAmmoType &ammo);
		void                                _addTeamStartingWeapon(Team* team, const str& weapon);
		void                                _setTeamStartingHealth(Team* team, unsigned int startingHealth);

		void								updatePlayerSkin( Player *player );

	public:
		CLASS_PROTOTYPE( ModeTeamBase );
		virtual	~ModeTeamBase();

		// Queries
		/* virtual */ bool					isEndOfMatch();                            // Checks for team frag limits met

		// Gets
		unsigned int                        getMaxTeams()                              { return _maxTeams ; }
		Team*                               getLeadTeam()                              { return _leadTeam ; }

		/* virtual */ int					getIcon( Player *player, int statNum, int value );
		/* virtual */ int					getInfoIcon( Player *player );

		// Sets
		virtual void                        SetMaxTeams(unsigned int maxTeams)         { _maxTeams = maxTeams ; }
		virtual void                        SetTeamStartingHealth(const str& teamName, unsigned int startingHealth);

		/* virtual */ bool					canJoinTeam( Player *player, const str &teamName );
		/* virtual */ void					joinTeam( Player *player, const str &teamName );


		// Player specific functions
		/* virtual */ void					AddPlayer( Player *player );                    // if force join is set, will stick on team with fewest members
		/* virtual */ void					RemovePlayer( Player *player );

		/* virtual */ float					playerDamaged( Player *damagedPlayer, Player *attackingPlayer, float damage, int meansOfDeath );

		virtual void						addPlayerToTeam( Player *player, Team *team );
		void								changeTeams( Player *player, Team *team );

		/* virtual */ void					respawnPlayer( Player *player );
		/* virtual */ Entity *				getSpawnPoint( Player *player );

		// Team specific functions
		virtual void                        AddTeamStartingWeapon(const str& teamName, const str& startingWeapon);
		virtual void                        AddTeamStartingAmmo(const str& teamName,    const str& ammoName, int amount);
		virtual Team*                       AddTeam(const str& name);
		Team *								getTeam( const str & teamName );
		virtual void                        RemoveTeam(const str& name);

		int									getTeamPoints( Player *player );
		int									getTeamPoints( const str & teamName );
		void								addTeamPoints( const str & teamName, int points );

		void								teamPointsChanged( Team *team, int oldPoints, int newPoints );

		/* virtual */ int					getHighestPoints( void );

		/* virtual */ void					playerChangedModel( Player *player );
	};

#endif // __MP_MODETEAMBASE_HPP__
