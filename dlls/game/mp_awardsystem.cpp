//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/mp_awardsystem.cpp                         $
// $Revision:: 22                                                             $
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
#include "mp_awardsystem.hpp"
#include "powerups.h"

// Setup constants

const float	AwardSystem::_minEfficiencyForEfficiencyAward = 0.5f;
const int	AwardSystem::_minImpressivesForSharpshooter = 5;
const float	AwardSystem::_minPercentForDemolitionist = 0.75f;
const float	AwardSystem::_minPercentForVaporizer = 0.75f;

const int	AwardSystem::_minKillsForChampion = 20;
const int	AwardSystem::_minKillsForMaster = 15;
const int	AwardSystem::_minKillsForExpert = 10;
const int	AwardSystem::_minKillsForAce = 5;

const float	AwardSystem::_maxExcellentTime = 2.0f;
const float	AwardSystem::_deniedDistance = 160.0f;

MultiplayerPlayerAwardData::MultiplayerPlayerAwardData()
{
	init();
}

void MultiplayerPlayerAwardData::init( void )
{
	_entnum = 0;
	_playing = false;

	reset();

	multiplayerManager.cacheMultiplayerFiles( "mp_awardsystem" );
}

void MultiplayerPlayerAwardData::reset( void )
{
	int i;

	_lastKillTime = 0.0f;
	_killStreak = 0;
	_highestKillStreak = 0;
	_shotsFired = 0;
	_shotsHit = 0;
	_lastHitTime = 0.0f;

	_weaponsKilledWith = 0;
	_powerupsUsed = 0;

	_numKills = 0;
	_numKillsWithExplosives = 0;

	_numFlagCaptures = 0;
	_numFlagReturns = 0;
	_numFlagGuardingKills = 0;

	_numImpressives = 0;

	_numDeaths = 0;

	_numFirstStrikeAwards = 0;
	_numImpressiveAwards  = 0;
	_numExcellentAwards   = 0;
	_numAceAwards         = 0;
	_numExpertAwards      = 0;
	_numMasterAwards      = 0;
	_numChampionAwards    = 0;
	_numDeniedAwards      = 0;

	_lastAwardIconIndex = 0;
	_lastAwardTime = 0.0f;

	for ( i = 0 ; i < MAX_SCORE_ICONS ; i++ )
	{
		_afterMatchAwardIndexes[ i ] = 0;
	}
}

AwardSystem::AwardSystem()
{
	_playerAwardData = NULL;

	// Save off icon indexes

	_efficiencyIconIndex   = gi.imageindex( "sysimg/icons/mp/award_efficiency" );
	_sharpshooterIconIndex = gi.imageindex( "sysimg/icons/mp/award_sharpshooter" );
	_untouchableIconIndex  = gi.imageindex( "sysimg/icons/mp/award_untouchable" );

	_logisticsIconIndex     = gi.imageindex( "sysimg/icons/mp/award_logistics" );
	_tacticianIconIndex     = gi.imageindex( "sysimg/icons/mp/award_tactician" );
	_demolitionistIconIndex = gi.imageindex( "sysimg/icons/mp/award_demolitionist" );

	_aceIconIndex      = gi.imageindex( "sysimg/icons/mp/award_ace" );
	_expertIconIndex   = gi.imageindex( "sysimg/icons/mp/award_expert" );
	_masterIconIndex   = gi.imageindex( "sysimg/icons/mp/award_master" );
	_championIconIndex = gi.imageindex( "sysimg/icons/mp/award_champion" );

	_mvpIconIndex         = gi.imageindex( "sysimg/icons/mp/award_mvp" );
	_defenderIconIndex    = gi.imageindex( "sysimg/icons/mp/award_defender" );
	_warriorIconIndex     = gi.imageindex( "sysimg/icons/mp/award_warrior" );
	_carrierIconIndex     = gi.imageindex( "sysimg/icons/mp/award_carrier" );
	_interceptorIconIndex = gi.imageindex( "sysimg/icons/mp/award_interceptor" );
	_braveryIconIndex     = gi.imageindex( "sysimg/icons/mp/award_bravery" );

	_firstStrikeIconIndex = gi.imageindex( "sysimg/icons/mp/award_firstStrike" );
	_impressiveIconIndex  = gi.imageindex( "sysimg/icons/mp/award_impressive" );
	_excellentIconIndex   = gi.imageindex( "sysimg/icons/mp/award_excellent" );
	_deniedIconIndex      = gi.imageindex( "sysimg/icons/mp/award_denied" );

	_powerups.FreeObjectList();
	_weapons.FreeObjectList();
}

void AwardSystem::init( int maxPlayers )
{
	_maxPlayers = maxPlayers;
	_playerAwardData = new MultiplayerPlayerAwardData[ _maxPlayers ];

	_hadFirstStrike = false;
}

AwardSystem::~AwardSystem()
{
	delete [] _playerAwardData;
	_playerAwardData = NULL;

	_powerups.FreeObjectList();
	_weapons.FreeObjectList();
}

void AwardSystem::initItems( void )
{
	Entity *entity;
	gentity_t *edict;
	int i;
		
	// Build the list of powerups and weapons

	for ( i = 0 ; i < MAX_GENTITIES ; i++ )
	{
		edict = &g_entities[ i ];

		if ( !edict->inuse || !edict->entity )
			continue;
				
		entity = edict->entity;

		if ( entity->isSubclassOf( Powerup ) )
		{
			Powerup *powerup;

			// Add powerup name to the list if it's not already in there

			powerup = (Powerup *)entity;

			if ( !getItemIndex( _powerups, powerup->getName() ) )
			{
				_powerups.AddObject( powerup->getName() );
			}
		}
		else if ( entity->isSubclassOf( Weapon ) )
		{
			Weapon *weapon;

			// Add weapon name to the list if it's not already in there

			weapon = (Weapon *)entity;

			if ( !getItemIndex( _weapons, weapon->getName() ) )
			{
				_weapons.AddObject( weapon->getName() );
			}
		}
	}
}

int AwardSystem::getItemIndex( Container<str> &itemContainer, const str &itemName )
{
	int i;
	int numItems;
	str name;

	numItems = itemContainer.NumObjects();

	// Find the index of this item

	for ( i = 1 ; i <= numItems ; i++ )
	{
		name = itemContainer.ObjectAt( i );

		if ( name == itemName )
		{
			return i;
		}
	}

	return 0;
}

int AwardSystem::getNumItems( Container<str> &itemContainer )
{
	return itemContainer.NumObjects();
}

int AwardSystem::getNumItemBits( unsigned int bits )
{
	int i;
	int numBits;

	numBits = 0;

	for ( i = 0 ; i <  32 ; i++ )
	{
		if ( bits & ( 1 << i ) )
		{
			numBits++;
		}
	}

	return numBits;
}

int AwardSystem::getAfterMatchAward( Player *player, int index )
{
	if ( !player || ( index < 0 ) || ( index >= MAX_SCORE_ICONS ) )
		return 0;

	return _playerAwardData[ player->entnum ]._afterMatchAwardIndexes[ index ];
}

void AwardSystem::matchOver( void )
{
	int i;

	// Calculate all after match awards

	for ( i = 0 ; i < _maxPlayers ; i++ )
	{
		if ( _playerAwardData[ i ]._playing )
		{
			// Efficiency award

			if ( ( _playerAwardData[ i ]._shotsFired > 0 ) )
			{
				if ( (float)_playerAwardData[ i ]._shotsHit / (float)_playerAwardData[ i ]._shotsFired > _minEfficiencyForEfficiencyAward )
				{
					_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON1 ] = _efficiencyIconIndex;
				}
			}

			// Sharpshooter award

			if ( _playerAwardData[ i ]._numImpressives >= _minImpressivesForSharpshooter )
			{
				_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON2 ] = _sharpshooterIconIndex;
			}

			// Untouchable award

			if ( _playerAwardData[ i ]._numDeaths == 0 )
			{
				_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON3 ] = _untouchableIconIndex;
			}

			// Player type award

			if ( getNumItems( _weapons ) && ( getNumItemBits( _playerAwardData[ i ]._weaponsKilledWith ) == getNumItems( _weapons ) ) )
			{
				_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON4 ] = _tacticianIconIndex;
			}
			else if ( getNumItems( _powerups ) && ( getNumItemBits( _playerAwardData[ i ]._powerupsUsed ) == getNumItems( _powerups ) ) )
			{
				_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON4 ] = _logisticsIconIndex;
			}
			else if ( _playerAwardData->_numKills && ( _playerAwardData->_numKillsWithExplosives / _playerAwardData->_numKills > _minPercentForDemolitionist ) )
			{
				_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON4 ] = _demolitionistIconIndex;
			}

			// Streak award

			if ( _playerAwardData[ i ]._highestKillStreak >= _minKillsForChampion )
				_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON5 ] = _championIconIndex;
			else if ( _playerAwardData[ i ]._highestKillStreak >= _minKillsForMaster )
				_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON5 ] = _masterIconIndex;
			else if ( _playerAwardData[ i ]._highestKillStreak >= _minKillsForExpert )
				_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON5 ] = _expertIconIndex;
			else if ( _playerAwardData[ i ]._highestKillStreak >= _minKillsForAce )
				_playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON5 ] = _aceIconIndex;
		}
	}

	// Team player awards

	awardTeamAward( AFTERMATCH_TEAM_AWARD_MVP, "Red" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_DEFENDER, "Red" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_WARRIOR, "Red" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_CARRIER, "Red" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_INTERCEPTOR, "Red" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_BRAVERY, "Red" );

	awardTeamAward( AFTERMATCH_TEAM_AWARD_MVP, "Blue" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_DEFENDER, "Blue" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_WARRIOR, "Blue" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_CARRIER, "Blue" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_INTERCEPTOR, "Blue" );
	awardTeamAward( AFTERMATCH_TEAM_AWARD_BRAVERY, "Blue" );
}

void AwardSystem::awardTeamAward( AfterMatchTeamAwardType teamAward, const char *teamName )
{
	int i;
	Player *player;
	Player *bestPlayer = NULL;
	Team *team;
	int bestNumber = 0;
	int numToUse;
	int iconIndex;

	// Go through all of the player and calculate this award for this team

	for ( i = 0 ; i < _maxPlayers ; i++ )
	{
		if ( !_playerAwardData[ i ]._playing )
			continue;

		// Make sure this player hasn't already gotten a team award

		if ( _playerAwardData[ i ]._afterMatchAwardIndexes[ SCOREICON6 ] >= 0 )
			continue;

		player = (Player *)g_entities[ _playerAwardData[ i ]._entnum ].entity;

		team = multiplayerManager.getPlayersTeam( player );

		if ( !team )
			continue;

		// Make sure this player is on the correct team

		if ( stricmp( team->getName().c_str(), teamName ) != 0 )
			continue;

		// Get the number to use for comparision purposes

		numToUse = 0;

		switch( teamAward )
		{
			case AFTERMATCH_TEAM_AWARD_MVP :
				numToUse = multiplayerManager.getPoints( player );
				break;
			case AFTERMATCH_TEAM_AWARD_DEFENDER :
				numToUse = _playerAwardData[ i ]._numFlagGuardingKills;
				break;
			case AFTERMATCH_TEAM_AWARD_WARRIOR :
				numToUse = _playerAwardData[ i ]._numKills;
				break;
			case AFTERMATCH_TEAM_AWARD_CARRIER :
				numToUse = _playerAwardData[ i ]._numFlagCaptures;
				break;
			case AFTERMATCH_TEAM_AWARD_INTERCEPTOR :
				numToUse = _playerAwardData[ i ]._numFlagReturns;
				break;
			case AFTERMATCH_TEAM_AWARD_BRAVERY :
				numToUse = _playerAwardData[ i ]._numDeaths;
				break;
		}

		// See if this player has the highest score for this type

		if ( numToUse > bestNumber )
		{
			bestPlayer = player;
			bestNumber = numToUse;
		}
	}

	if ( bestPlayer )
	{
		// Get the icon index to use for this award

		iconIndex = -1;

		switch( teamAward )
		{
			case AFTERMATCH_TEAM_AWARD_MVP :
				iconIndex = _mvpIconIndex;
				break;
			case AFTERMATCH_TEAM_AWARD_DEFENDER :
				iconIndex = _defenderIconIndex;
				break;
			case AFTERMATCH_TEAM_AWARD_WARRIOR :
				iconIndex = _warriorIconIndex;
				break;
			case AFTERMATCH_TEAM_AWARD_CARRIER :
				iconIndex = _carrierIconIndex;
				break;
			case AFTERMATCH_TEAM_AWARD_INTERCEPTOR :
				iconIndex = _interceptorIconIndex;
				break;
			case AFTERMATCH_TEAM_AWARD_BRAVERY :
				iconIndex = _braveryIconIndex;
				break;
		}

		// Give the player the award

		_playerAwardData[ bestPlayer->entnum ]._afterMatchAwardIndexes[ SCOREICON6 ] = iconIndex;
	}
}

void AwardSystem::playerEventNotification( const char *eventName, const char *eventItemName, Player *eventPlayer )
{
	// See if we care about this player notification

	if ( stricmp( eventName, "flag-captured" ) == 0 )
	{
		_playerAwardData[ eventPlayer->entnum ]._numFlagCaptures++;
	}
	else if ( stricmp( eventName, "flag-returned" ) == 0 )
	{
		_playerAwardData[ eventPlayer->entnum ]._numFlagReturns++;
	}
	else if ( stricmp( eventName, "flag-guarded" ) == 0 )
	{
		_playerAwardData[ eventPlayer->entnum ]._numFlagGuardingKills++;
	}
}

void AwardSystem::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	str weaponName;
	int itemIndex;

	MultiplayerPlayerAwardData *attackerAwardData;

	// Modify the killed player's stats

	_playerAwardData[ killedPlayer->entnum ]._killStreak = 0;
	_playerAwardData[ killedPlayer->entnum ]._numDeaths++;

	// Make sure this isn't a suicide

	if ( killedPlayer == attackingPlayer )
		return;

	if ( !attackingPlayer )
		return;

	// Make sure this isn't one teammate killing another

	if ( multiplayerManager.getPlayersTeam( attackingPlayer ) && multiplayerManager.getPlayersTeam( killedPlayer ) &&
		 ( multiplayerManager.getPlayersTeam( attackingPlayer ) == multiplayerManager.getPlayersTeam( killedPlayer ) ) )
		return;

	attackerAwardData = &_playerAwardData[ attackingPlayer->entnum ];

	// Test for humiliation

	if ( meansOfDeath == MOD_SWORD )
	{
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_humil.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
		multiplayerManager.playerSound( killedPlayer->entnum, "localization/sound/dialog/dm/comp_humil.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
	}

	// Test for first strike award (first kill of the match)

	if ( !_hadFirstStrike )
	{
		multiplayerManager.centerPrint( attackingPlayer->entnum, "$$FirstStrike$$\n", CENTERPRINT_IMPORTANCE_NORMAL );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_firstst.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );

		attackerAwardData->_numFirstStrikeAwards++;

		attackerAwardData->_lastAwardIconIndex = _firstStrikeIconIndex;
		attackerAwardData->_lastAwardTime = multiplayerManager.getTime();

		_hadFirstStrike = true;
	}

	// Test for impressive award (multiple kills at same time)
	//   and test for excellent award (multiple kills within 2 seconds)

	if ( attackerAwardData->_lastKillTime == level.time )
	{
		multiplayerManager.centerPrint( attackingPlayer->entnum, "$$Impressive$$\n", CENTERPRINT_IMPORTANCE_NORMAL );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_impress.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );

		attackerAwardData->_numImpressives++;

		attackerAwardData->_numImpressiveAwards++;

		attackerAwardData->_lastAwardIconIndex = _impressiveIconIndex;
		attackerAwardData->_lastAwardTime = multiplayerManager.getTime();
	}
	else if ( ( attackerAwardData->_lastKillTime > 0.0f ) && ( attackerAwardData->_lastKillTime + _maxExcellentTime > level.time ) )
	{
		multiplayerManager.centerPrint( attackingPlayer->entnum, "$$Excellent$$\n", CENTERPRINT_IMPORTANCE_NORMAL );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_excell.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );

		attackerAwardData->_numExcellentAwards++;

		attackerAwardData->_lastAwardIconIndex = _excellentIconIndex;
		attackerAwardData->_lastAwardTime = multiplayerManager.getTime();
	}

	// Modify some of the attacker's stats

	attackerAwardData->_lastKillTime = level.time;
	attackerAwardData->_killStreak++;

	attackerAwardData->_numKills++;

	if ( meansOfDeath == MOD_EXPLOSION )
	{
		attackerAwardData->_numKillsWithExplosives++;
	}

	// See if this is the highest kill streak for this player

	if ( attackerAwardData->_killStreak > attackerAwardData->_highestKillStreak)
		attackerAwardData->_highestKillStreak = attackerAwardData->_killStreak;

	// See if we should give a kill streak award

	if ( attackerAwardData->_killStreak == _minKillsForChampion )
	{
		multiplayerManager.centerPrint( attackingPlayer->entnum, "$$Champion$$\n", CENTERPRINT_IMPORTANCE_NORMAL );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_champ.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
		attackerAwardData->_numChampionAwards++;

		attackerAwardData->_lastAwardIconIndex = _championIconIndex;
		attackerAwardData->_lastAwardTime = multiplayerManager.getTime();
	}
	else if ( attackerAwardData->_killStreak == _minKillsForMaster )
	{
		multiplayerManager.centerPrint( attackingPlayer->entnum, "$$Master$$\n", CENTERPRINT_IMPORTANCE_NORMAL );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_master.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
		attackerAwardData->_numMasterAwards++;

		attackerAwardData->_lastAwardIconIndex = _masterIconIndex;
		attackerAwardData->_lastAwardTime = multiplayerManager.getTime();
	}
	else if ( attackerAwardData->_killStreak == _minKillsForExpert )
	{
		multiplayerManager.centerPrint( attackingPlayer->entnum, "$$Expert$$\n", CENTERPRINT_IMPORTANCE_NORMAL );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_expert.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
		attackerAwardData->_numExpertAwards++;

		attackerAwardData->_lastAwardIconIndex = _expertIconIndex;
		attackerAwardData->_lastAwardTime = multiplayerManager.getTime();
	}
	else if ( attackerAwardData->_killStreak == _minKillsForAce )
	{
		multiplayerManager.centerPrint( attackingPlayer->entnum, "$$Ace$$\n", CENTERPRINT_IMPORTANCE_NORMAL );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_ace.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
		attackerAwardData->_numAceAwards++;

		attackerAwardData->_lastAwardIconIndex = _aceIconIndex;
		attackerAwardData->_lastAwardTime = multiplayerManager.getTime();
	}

	// Keep track of which weapons this player killed things with

	attackingPlayer->getActiveWeaponName( WEAPON_ANY, weaponName );

	itemIndex = getItemIndex( _weapons, weaponName );
	itemIndex -= 1;

	if ( ( itemIndex >= 0 ) && ( itemIndex < 32 ) )
	{
		attackerAwardData->_weaponsKilledWith |= ( 1 << itemIndex );
	}
}

void AwardSystem::pickedupItem( Player *player, MultiplayerItemType itemType, const char *itemName )
{
	int itemIndex;
	int i;
	Player *playerToCheck;
	Vector diff;
	float distance;

	// We only care about powerups

	if ( itemType != MP_ITEM_TYPE_POWERUP )
		return;

	// Mark that we have used this powerup

	itemIndex = getItemIndex( _powerups, itemName );
	itemIndex -= 1;

	if ( ( itemIndex >= 0 ) && ( itemIndex < 32 ) )
	{
		_playerAwardData[ player->entnum ]._powerupsUsed |= ( 1 << itemIndex );
	}

	// See if the player just denied someone else this powerup

	for ( i = 0 ; i < _maxPlayers ; i++ )
	{
		playerToCheck = multiplayerManager.getPlayer( i );

		if ( !playerToCheck )
			continue;

		// Make sure not to check the player that picked up the item

		if ( playerToCheck == player )
			continue;

		// Make sure the player isn't a spectator

		if ( multiplayerManager.isPlayerSpectator( playerToCheck ) )
			continue;

		// Make sure the players' aren't on the same team

		if ( multiplayerManager.getPlayersTeam( player ) == multiplayerManager.getPlayersTeam( playerToCheck ) )
			continue;

		// See if the player is really close by

		diff = playerToCheck->origin - player->origin;
		distance = diff.length();

		if ( distance < _deniedDistance )
		{
			// Denied award (really a taunt)

			multiplayerManager.centerPrint( playerToCheck->entnum, "$$Denied$$\n", CENTERPRINT_IMPORTANCE_NORMAL );
			multiplayerManager.playerSound( playerToCheck->entnum, "localization/sound/dialog/dm/comp_denied.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
			_playerAwardData[ i ]._numDeniedAwards++;

			_playerAwardData[ i ]._lastAwardIconIndex = _deniedIconIndex;
			_playerAwardData[ i ]._lastAwardTime = multiplayerManager.getTime();
		}
	}
}

void AwardSystem::playerFired( Player *attackingPlayer )
{
	_playerAwardData[ attackingPlayer->entnum ]._shotsFired++;
}

void AwardSystem::playerDamaged( Player *damagedPlayer, Player *attackingPlayer, float damage, int meansOfDeath )
{
	if ( !attackingPlayer )
		return;

	// Make sure this is a new valid hit

	if ( ( damagedPlayer != attackingPlayer ) && 
		 ( _playerAwardData[ attackingPlayer->entnum ]._lastHitTime != multiplayerManager.getTime() ) )
	{
		// Player actually hit someone, record it as a shot hit

		_playerAwardData[ attackingPlayer->entnum ]._shotsHit++;
		_playerAwardData[ attackingPlayer->entnum ]._lastHitTime = multiplayerManager.getTime();
	}
}

void AwardSystem::addPlayer( Player *player )
{
	_playerAwardData[ player->entnum ].reset();
	_playerAwardData[ player->entnum ]._playing = true;
}

void AwardSystem::removePlayer( Player *player )
{
	_playerAwardData[ player->entnum ].reset();
}

int AwardSystem::getStat( Player *player, int statNum, int value )
{
	if ( statNum == STAT_MP_AWARD_COUNT )
	{
		if ( _playerAwardData[ player->entnum ]._lastAwardTime + 5.0f >= multiplayerManager.getTime() )
		{
			return getLastAwardCount( player );
		}
	}

	return value;
}

int AwardSystem::getIcon( Player *player, int statNum, int value )
{
	if ( statNum == STAT_MP_AWARD_ICON )
	{
		int icon;

		icon = getLastAward( player );

		if ( icon > 0 )
			return icon;
		else
			return value;
	}

	return value;
}

int AwardSystem::getInfoIcon( Player *player )
{
	return getLastAward( player );
}

int AwardSystem::getLastAward( Player *player )
{
	if ( _playerAwardData[ player->entnum ]._lastAwardTime + 5.0f >= multiplayerManager.getTime() )
	{
		return _playerAwardData[ player->entnum ]._lastAwardIconIndex;
	}
	else
	{
		return 0;
	}
}

int AwardSystem::getLastAwardCount( Player *player )
{
	int lastAward;

	if ( _playerAwardData[ player->entnum ]._lastAwardTime + 5.0f >= multiplayerManager.getTime() )
	{
		lastAward = getLastAward( player );

		if ( lastAward == _firstStrikeIconIndex )
			return _playerAwardData[ player->entnum ]._numFirstStrikeAwards;
		else if ( lastAward == _impressiveIconIndex )
			return _playerAwardData[ player->entnum ]._numImpressiveAwards;
		else if ( lastAward == _excellentIconIndex )
			return _playerAwardData[ player->entnum ]._numExcellentAwards;
		else if ( lastAward == _aceIconIndex )
			return _playerAwardData[ player->entnum ]._numAceAwards;
		else if ( lastAward == _expertIconIndex )
			return _playerAwardData[ player->entnum ]._numExpertAwards;
		else if ( lastAward == _masterIconIndex )
			return _playerAwardData[ player->entnum ]._numMasterAwards;
		else if ( lastAward == _championIconIndex )
			return _playerAwardData[ player->entnum ]._numChampionAwards;
		else if ( lastAward == _deniedIconIndex )
			return _playerAwardData[ player->entnum ]._numDeniedAwards;
	}

	return 0;
}

