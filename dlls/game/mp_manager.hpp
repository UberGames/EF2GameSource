//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/dm_manager.h                                  $
// $Revision:: 31                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 7/18/02 10:45a                                                 $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

// Forward declaration
class Player;
//class DM_Team ;

#ifndef __MP_MANAGER_HPP__
#define __MP_MANAGER_HPP__

// Typedefs
typedef SafePtr<Player> PlayerPtr;

#include "g_local.h"
#include "mp_modeBase.hpp"
#include "mp_modifiers.hpp"
#include "mp_awardsystem.hpp"
#include "mp_shared.hpp"
#include "equipment.h"

class MultiplayerManager;

extern MultiplayerManager multiplayerManager;

extern str playersLastTeam[ MAX_CLIENTS ];

typedef enum
{
	MP_CVAR_TYPE_INTEGER,
	MP_CVAR_TYPE_FLOAT,
	MP_CVAR_TYPE_BOOL
} MPCvarType;

class MultiplayerDialogData
{
public:
	str				_soundName;
	int				_channel;
	float			_volume;
	float			_minDist;
	float			_time;
};

class MultiplayerPlayerData
{
public:
	static const int	_maxDialogs;

	bool				_valid;
	int					_votecount;
	bool				_voted;
	bool				_spectator;
	bool				_spectatorByChoice;
	SpectatorTypes		_spectatorType;
	float				_spectatorTime;
	int					_spectatorPlayerNum;
	bool				_waitingForRespawn;
	float				_respawnTime;
	str					_teamHud;

	bool				_named;
	str					_name;

	MultiplayerDialogData*	_dialogData;
	int					_nextDialogSendSpot;
	int					_nextDialogAddSpot;
	float				_nextDialogSendTime;
	float				_nextTauntTime;

						MultiplayerPlayerData();
						~MultiplayerPlayerData();
	void				reset( void );

	int					getNextDialogAddSpot() { return _nextDialogAddSpot % _maxDialogs; }
	int					getNextDialogSendSpot() { return _nextDialogSendSpot % _maxDialogs; }
};

class MultiplayerManager : public Class
   {
	private:
		static const int				_playerFreezeTime;
		//static const int				_maxVoteCount;
		static const float				_maxVoteTime;
		static const int				_maxSayStringLength;
		static const float				_inBetweenMatchTime;

		bool							_inMultiplayerGame;
		bool							_gameStarted;
		bool							_gameOver;
		MultiplayerModeBase				*_multiplayerGame;
		bool							_allowFighting;
		Container<MultiplayerModifier *>	_modifiers;
		MultiplayerPlayerData			*_playerData;
		AwardSystem						*_awardSystem;

		float							_voteTime;
		str								_voteString;
		int								_voteYes;
		int								_voteNo;
		int								_numVoters;

		float							_declareWinnerTime;
		bool							_declaredWinner;

		bool							_needToAddBots;

		bool							_inMatch;

		int								_talkingIconIndex;
		int								_waitingToRespawnIconIndex;

		float							_restartMatchTime;

		float							_respawnTime;

		int								_oldFlags;

		gclient_s *						getClient( int entnum );

		void							start( void );
		void							addBots( void );

		void							matchOver( void );
		void							restartMatch( void );

		void							setNextMap( void );
		str								getNextMap( void );

		void							addSoundToQueue( Player *player, str soundName, int channel, float volume, float minDist, float time );
		void							sendNextPlayerSound( Player *player );

	protected:

	public:
		CLASS_PROTOTYPE( MultiplayerManager );

										MultiplayerManager();
										~MultiplayerManager();

		//
		// Interface for game dll
		//

		void							init( void );
		void							cleanup( qboolean restart );
		void							update( float frameTime );
		bool							inMultiplayer( void );
		bool							checkFlag( unsigned int flag );
		bool							isFightingAllowed( void );
		bool							fullCollision( void );

		void							initMultiplayerGame( void );
		void							initItems( void );
		void							resetItems( void );
		void							addPlayer( Player *player );
		void							removePlayer( Player *player );

		void							applySpeedModifiers( Player *player,  int *moveSpeed );
		void							applyJumpModifiers( Player *player,  int *jumpSpeed );
		void							applyAirAccelerationModifiers( Player *player,  int *airAcceleration );

		bool							canPickup( Player *player, MultiplayerItemType itemType, const char *item_name );
		void							pickedupItem( Player *player, MultiplayerItemType itemType, const char *item_name );

		void							changePlayerModel( Player *player, const char *modelName, bool force = false );
		void							resetPlayerStateMachine( Player *player );
		void							changePlayerName( Player *player, const str &PlayerName );
		void							playerDead( Player *player );
		void							playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath );
		float							playerDamaged( Player *damagedPlayer, Player *attackingPlayer, float damage, int meansOfDeath );
		void							playerTookDamage( Player *damagedPlayer, Player *attackingPlayer, float damage, int meansOfDeath );
		void							playerFired( Player *attackingPlayer );
		float							getModifiedKnockback( Player *damagedPlayer, Player *attackingPlayer, float knockback );

		void							itemTouched( Player *player, MultiplayerItem *item );
		void							itemDestroyed( Player *player, MultiplayerItem *item );
		float							itemDamaged( MultiplayerItem *item, Player *attackingPlayer, float damage, int meansOfDeath );
		void							itemUsed( Entity *entity, MultiplayerItem *item );

		void							playerUsed( Player *usedPlayer, Player *usingPlayer, Equipment *equipment );

		void							score( Player *player );

		void							respawnPlayer( Player *player, bool forced );
		void							respawnAllPlayers( void );
		Entity *						getSpawnPoint( Player *player );

		bool							isPlayerSpectator( Player *player, SpectatorTypes spectatorType = SPECTATOR_TYPE_ANY );
		bool							isPlayerSpectatorByChoice( Player *player );
		Player *						getPlayerSpectating( Player *player );
		void							makePlayerSpectator( Player *player, SpectatorTypes spectatorType = SPECTATOR_TYPE_FOLLOW, bool byChoice = false );
		void							makePlayerSpectateNextPlayer( Player *player );
		void							makePlayerSpectatePrevPlayer( Player *player );
		void							makePlayerSpectatePlayer( Player *player, Player *playerToSpectate );
		void							playerEnterArena( int entnum, float health );
		void							playerSpawned( Player *player );

		int								getPoints( Player *player );
		int								getKills( Player *player );
		int								getDeaths( Player *player );
		int								getTeamPoints( Player *player );
		int								getTeamPoints( const str & teamName );
		void							addTeamPoints( const str & teamName, int points );

		int								getStat( Player *player, int statNum );
		int								getIcon( Player *player, int statNum );
		int								getScoreIcon( Player *player, int index );
		int								getInfoIcon( Player *player, int buttons );

		Player *						getLastKilledByPlayer( Player *player, int *meansOfDeath = NULL );
		Player *						getLastKillerOfPlayer( Player *player, int *meansOfDeath = NULL );

		Team*							getPlayersTeam( const Player *player );

		int								getPointsForKill( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath, int points );

		void							callVote( Player *player, const str &command, const str &arg );
		void							vote( Player *player, const str &vote );
		void							checkVote( void );

		void							joinTeam( Player *player, const str &teamName );

		void							say( Player *player, const str &text, bool team );
		void							tell( Player *player, const str &text, int entnum );

		float							getItemRespawnMultiplayer( void );
		float							getWeaponRespawnMultiplayer( void );
		float							getPowerupRespawnMultiplayer( void );

		bool							checkGameType( const char *rule );
		bool							doesPlayerHaveItem( Player *player, const char *itemName );

		void							playerCommand( Player *player, const char *command, const char *parm );
		void							playerInput( Player *player, int newButtons );

		//
		// Modifier stuff
		//

		void							addModifiers( void );
		void							tryAddModifier( const str &modifierName );
		void							addModifier( const str &modifierName );

		void							addPoints( int entnum, int points );

		//
		// Interface for modes
		//

		Player *						getPlayer( int entnum );
		int								getMaxPlayers( void );
		int								getTotalPlayers( bool countSpectators = false );

		int								getClientNum( int entnum );
		int								getClientPing( int entnum );
		float							getTime( void );
		void							centerPrint( int entnum, const str &string, CenterPrintImportance importance );
		void							HUDSay( int entnum, const str &string );
		void							HUDPrint( int entnum, const str &string );
		void							statusPrint( int entnum, const str &string );
		void							instantPlayerSound( int entnum, const str &soundName, int channel = CHAN_AUTO, float volume = DEFAULT_VOL, float minDist = DEFAULT_MIN_DIST );
		void							broadcastInstantSound( const str &soundName, int channel  = CHAN_AUTO, float volume  = DEFAULT_VOL, float minDist  = DEFAULT_MIN_DIST, Player *except = NULL );
		void							playerSound( int entnum, const str &soundName, int channel = CHAN_AUTO, float volume = DEFAULT_VOL, float minDist = DEFAULT_MIN_DIST, float time = 2.5f );
		void							teamSound( Team *team, const str &soundName, int channel = CHAN_AUTO, float volume = DEFAULT_VOL, float minDist = DEFAULT_MIN_DIST, float time = 2.5f );
		void							broadcastSound( const str &soundName, int channel = CHAN_AUTO, float volume = DEFAULT_VOL, float minDist = DEFAULT_MIN_DIST, Player *except = NULL, float time = 2.5f );

		void							addPlayerHealth( int entnum, float healthToAdd );

		void							allowFighting( bool allowFighting );

		bool							checkRule( const char *rule, bool defaultValue, Player *player = NULL );

		void							playerEventNotification( const char *eventName, const char *eventItemName, Player *eventPlayer );

		void							initPlayer( Player *player );

		void							teamPointsChanged( Team *team, int oldPoints, int newPoints );

		void							cacheMultiplayerFiles( const str &cacheFileName );

		void							setTeamHud( Player *player, const str &teamHudName );

		str								getSpawnPointType( Player *player );

		//
		// Shared Interface (game dll and modes)
		//

		void							centerPrintAllClients( const str &string, CenterPrintImportance importance );
		void							HUDPrintAllClients( const str &string );

		void							centerPrintTeamClients( Player *player, const str &string, CenterPrintImportance importance );
		void							HUDPrintTeamClients( Player *player, const str &string );

		void							HUDPrintSpectators( Player *player, const str &string, bool team );

		bool							canGivePlayerItem( int entnum, const str &itemName );
		void							givePlayerItem( int entnum, const str &itemName );
		void							usePlayerItem( int entnum, const str &itemName );

		int								getAfterMatchAward( Player *player, int index );

		void							startMatch( void );
		void							endMatch( void );

		str								getPlaceName( Player *player );

		bool							isValidPlayerModel( Player *player, str modelToUse );
		str								getDefaultPlayerModel( Player *player );

		float							getRespawnTime( void );
		void							setRespawnTime( float time );
		void							resetRespawnTime( void );

		void							checkModifiedCvars( bool informPlayers );
		void							checkCvar( cvar_t *mp_cvarToCheck, str optionName, MPCvarType cvarType, int bitToCheck = -1 );
		bool							hasFlagChanged( int bitToCheck );

		bool							skipWeaponReloads( void );
		void							playerTaunt( Player *player, const str &tauntName );
	};

#endif // __MP_MANAGER_HPP__
