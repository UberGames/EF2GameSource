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

#ifndef __MP_AWARDSYSTEM_HPP__
#define __MP_AWARDSYSTEM_HPP__

#include "g_local.h"          // common game stuff
#include "player.h"           // for Player
#include "item.h"
#include "mp_shared.hpp"

/* typedef enum
{
	AFTERMATCH_AWARD1,
	AFTERMATCH_AWARD2,
	AFTERMATCH_AWARD3,
	AFTERMATCH_AWARD4,
	AFTERMATCH_AWARD5,
	AFTERMATCH_AWARD6,
	MAX_AFTER_MATCH_AWARDS
} AfterMatchAwardType; */

typedef enum
{
	AFTERMATCH_TEAM_AWARD_MVP,
	AFTERMATCH_TEAM_AWARD_DEFENDER,
	AFTERMATCH_TEAM_AWARD_WARRIOR,
	AFTERMATCH_TEAM_AWARD_CARRIER,
	AFTERMATCH_TEAM_AWARD_INTERCEPTOR,
	AFTERMATCH_TEAM_AWARD_BRAVERY
} AfterMatchTeamAwardType;

class MultiplayerPlayerAwardData
{
public:
	float				_lastKillTime;
	int					_killStreak;
	int					_highestKillStreak;
	int					_numImpressives;
	int					_numDeaths;
	int					_shotsFired;
	int					_shotsHit;
	float				_lastHitTime;
	unsigned int		_weaponsKilledWith;
	unsigned int		_powerupsUsed;
	int					_numFlagCaptures;
	int					_numFlagReturns;
	int					_numFlagGuardingKills;

	int					_numKills;
	int					_numKillsWithExplosives;
	int					_numVaporizedKills;

	int					_entnum;

	bool				_playing;

	int					_numFirstStrikeAwards;
	int					_numImpressiveAwards;
	int					_numExcellentAwards;
	int					_numAceAwards;
	int					_numExpertAwards;
	int					_numMasterAwards;
	int					_numChampionAwards;
	int					_numDeniedAwards;

	int					_lastAwardIconIndex;
	float				_lastAwardTime;

	int					_afterMatchAwardIndexes[ MAX_SCORE_ICONS ];

						MultiplayerPlayerAwardData();
	void				init( void );
	void				reset( void );
};

class AwardSystem : public Class
{
private:
	static const float			_minEfficiencyForEfficiencyAward;
	static const int			_minImpressivesForSharpshooter;
	static const float			_minPercentForDemolitionist;
	static const float			_minPercentForVaporizer;
	static const int			_minKillsForChampion;
	static const int			_minKillsForMaster;
	static const int			_minKillsForExpert;
	static const int			_minKillsForAce;
	static const float			_maxExcellentTime;
	static const float			_deniedDistance;

	int							_maxPlayers;
	bool						_hadFirstStrike;
	MultiplayerPlayerAwardData	*_playerAwardData;

	int							_efficiencyIconIndex;
	int							_sharpshooterIconIndex;
	int							_untouchableIconIndex;

	int							_logisticsIconIndex;
	int							_tacticianIconIndex;
	int							_demolitionistIconIndex;

	int							_aceIconIndex;
	int							_expertIconIndex;
	int							_masterIconIndex;
	int							_championIconIndex;

	int							_mvpIconIndex;
	int							_defenderIconIndex;
	int							_warriorIconIndex;
	int							_carrierIconIndex;
	int							_interceptorIconIndex;
	int							_braveryIconIndex;

	int							_firstStrikeIconIndex;
	int							_impressiveIconIndex;
	int							_excellentIconIndex;
	int							_deniedIconIndex;

	Container<str>				_powerups;
	Container<str>				_weapons;

	int							getItemIndex( Container<str> &itemContainer, const str &itemName );
	int							getNumItems( Container<str> &itemContainer );
	int							getNumItemBits( unsigned int bits );

	void						awardTeamAward( AfterMatchTeamAwardType teamAward, const char *teamName );
	int							getLastAward( Player *player );
	int							getLastAwardCount( Player *player );

public:
								AwardSystem(); 
								~AwardSystem(); 
	void						init( int maxPlayers );
	void						initItems( void );
	virtual void				playerDamaged( Player *damagedPlayer, Player *attackingPlayer, float damage, int meansOfDeath );
	virtual void				playerFired( Player *attackingPlayer );
	virtual void				playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath );
	virtual void				addPlayer( Player *player );
	virtual void				removePlayer( Player *player );
	virtual void				update( float frameTime ) {};

	virtual int					getStat( Player *player, int statNum, int value );
	virtual int					getIcon( Player *player, int statNum, int value );
	virtual int					getInfoIcon( Player *player );
	int							getAfterMatchAward( Player *player, int index );

	void						pickedupItem( Player *player, MultiplayerItemType itemType, const char *itemName );

	void						playerEventNotification( const char *eventName, const char *eventItemName, Player *eventPlayer );

	virtual void				matchOver( void );
};

#endif // __MP_AWARDSYSTEM_HPP__
