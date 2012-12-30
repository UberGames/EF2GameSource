//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/mp_modifiers.cpp                           $
// $Revision:: 172                                                            $
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
#include "mp_modifiers.hpp"
#include "equipment.h"
#include "powerups.h"
#include "weaputils.h"
#include "health.h"
#include "armor.h"

// Setup constants

const float	ModifierInstantKill::_instantKillDamage = 1000.0f;

const float	ModifierInstantKill::_regenTime = 0.1f;
const int	ModifierInstantKill::_regenAmount = 1;

ModifierInstantKill::ModifierInstantKill()
{
	_lastRegenTime = 0.0f;
}

void ModifierInstantKill::init( int maxPlayers )
{
	multiplayerManager.cacheMultiplayerFiles( "mp_instantKill" );
}

float ModifierInstantKill::playerDamaged( Player *damagedPlayer, Player *attackingPlayer, float damage, int meansOfDeath )
{
	float realDamage = 0.0f;

	if ( damage > 0.0f )
	{
		switch( meansOfDeath )
		{
		case MOD_COMP_RIFLE:
		case MOD_IMOD_PRIMARY:
		case MOD_IMOD_SECONDARY:
		case MOD_DISRUPTOR:
		case MOD_SNIPER:
		case MOD_MELEE:
			realDamage = _instantKillDamage;
			break;
		default:
			realDamage = damage;
			break;
		}
	}

	return realDamage;
}

bool ModifierInstantKill::canGivePlayerItem( int entnum, const str &itemName )
{
	if ( strstr( itemName.c_str(), "phaser.tik" ) )
		return false;
	else if ( strstr( itemName.c_str(), "batleth.tik" ) )
		return false;
	else if ( strstr( itemName.c_str(), "compressionrifle.tik" ) )
		return false;

	return true;
}

bool ModifierInstantKill::checkRule( const char *rule, bool defaultValue, Player *player )
{
	if ( stricmp( rule, "dropWeapons" ) == 0 )
		return false;
	else
		return defaultValue;
}

void ModifierInstantKill::playerSpawned( Player *player )
{
	multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-sniperrifle.tik" );
	multiplayerManager.usePlayerItem( player->entnum, "FederationSniperRifle" );

	//multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-compressionrifle.tik" );
	//multiplayerManager.usePlayerItem( player->entnum, "CompressionRifle" );

	//multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-imod.tik" );
	//multiplayerManager.usePlayerItem( player->entnum, "I-Mod" );

	player->GiveAmmo( "Fed", 400, false );
}

bool ModifierInstantKill::shouldKeepNormalItem( Item *item )
{
	if ( item->isSubclassOf( Weapon ) )
		return false;
	else if ( item->isSubclassOf( Health ) )
		return false;
	else if ( item->isSubclassOf( Armor ) )
		return false;
	else if ( item->isSubclassOf( AmmoEntity ) )
		return false;
	else
		return true;
}

void ModifierInstantKill::update( float frameTime )
{
	int i;
	Player *player;

	if ( _lastRegenTime + _regenTime < multiplayerManager.getTime() )
	{
		for ( i = 0 ; i < multiplayerManager.getMaxPlayers() ; i++ )
		{
			player = multiplayerManager.getPlayer( i );

			if ( !player )
				continue;

			player->GiveAmmo( "Fed", _regenAmount, false );
			//player->GiveAmmo( "Plasma", _regenAmount, false );
		}

		_lastRegenTime = multiplayerManager.getTime();
	}
}

// Setup constants

const float	ModifierDestruction::_defaultObjectHealth = 500.0f;
const int	ModifierDestruction::_pointsForDestroyingObject = 100;
const float	ModifierDestruction::_objectHealRate = 75.0f;
const float	ModifierDestruction::_maxGuardingDist = 1000.0f;

const float	ModifierDestruction::_minDamageForPoints  = 100.0f;
const float	ModifierDestruction::_minHealingForPoints = 100.0f;

const int	ModifierDestruction::_pointsForDamage     = 5;
const int	ModifierDestruction::_pointsForHealing    = 5;
const int	ModifierDestruction::_pointsForDestroying = 25;
const int	ModifierDestruction::_pointsForGuarding   = 10;

ModifierDestruction::ModifierDestruction()
{
	_redDestructionObject  = NULL;
	_blueDestructionObject = NULL;

	_destructionPlayerData = NULL;

	_redLastDamageSoundTime  = 0.0f;
	_blueLastDamageSoundTime = 0.0f;

	_respawnTime = 0.0f;

	_blueObjectDestroyed = false;
	_redObjectDestroyed  = false;
}

ModifierDestruction::~ModifierDestruction()
{
	delete [] _destructionPlayerData;
}

void ModifierDestruction::init( int maxPlayers )
{
	_maxPlayers = maxPlayers;

	_destructionPlayerData = new DestructionPlayerData[ _maxPlayers ];

	multiplayerManager.cacheMultiplayerFiles( "mp_destruction" );
}

void ModifierDestruction::addPlayer( Player *player )
{
	// Put the destruction ui on the player's screen

	gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_destruction\"\n" );

	_destructionPlayerData[ player->entnum ].reset();
}

void ModifierDestruction::playerSpawned( Player *player )
{
	multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-tricorder.tik" );
}

void ModifierDestruction::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	Team *victimsTeam;
	Team *killersTeam;
	float distance;
	str printString;
	bool objectGuarded;

	if ( !attackingPlayer || ( killedPlayer == attackingPlayer ) )
		return;
	
	victimsTeam = multiplayerManager.getPlayersTeam( killedPlayer );
	killersTeam = multiplayerManager.getPlayersTeam( attackingPlayer );

	if ( victimsTeam && killersTeam && ( victimsTeam == killersTeam ) )
		return;

	// See if the player was guarding a object (either he or the killed player was close to his object)

	objectGuarded = false;

	distance = findDistanceToTeamsObject( killersTeam->getName(), attackingPlayer->origin );

	if ( ( distance > 0 ) && ( distance < _maxGuardingDist ) )
	{
		objectGuarded = true;
	}
	else
	{
		distance = findDistanceToTeamsObject( killersTeam->getName(), killedPlayer->origin );

		if ( ( distance > 0 ) && ( distance < _maxGuardingDist ) )
		{
			objectGuarded = true;
		}
	}

	if ( objectGuarded )
	{
		//multiplayerManager.playerEventNotification( "controlpoint-guarded", "", attackingPlayer );
		
		printString = "$$ObjectGuarded$$ ";
		printString += attackingPlayer->client->pers.netname;

		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_objectguarded.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.2f );

		multiplayerManager.centerPrintTeamClients( attackingPlayer, printString, CENTERPRINT_IMPORTANCE_HIGH );

		// Give points to the player for guarding his teams objects

		multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForGuarding );
	}
}

float ModifierDestruction::findDistanceToTeamsObject( const str &teamName, const Vector &position )
{
	Vector diff;
	float distance = -1.0f;
	MultiplayerItem *destructionObject = NULL;


	// Figure out which object to use

	if ( teamName == "Red" )
		destructionObject = _redDestructionObject;
	else if ( teamName == "Blue" )
		destructionObject = _blueDestructionObject;

	// Get the distance from the object to the specified point

	if ( destructionObject )
	{
		diff = position - destructionObject->origin;
		distance = diff.length();
	}

	// Return the distance

	return distance;
}

int ModifierDestruction::getStat( Player *player, int statNum, int value )
{
	float floatRealValue;

	if ( statNum == STAT_MP_GENERIC1 )
	{
		// Return the health of the red team's object

		if ( _redDestructionObject )
		{
			floatRealValue = ( _redDestructionObject->health / _redDestructionObject->max_health ) * 100.0f;

			if ( ( floatRealValue < 1.0f ) && ( _redDestructionObject->health > 1.0f ) )
			{
				floatRealValue = 1.0f;
			}

			return ( (int)floatRealValue );

		}
	}
	else if ( statNum == STAT_MP_GENERIC2 )
	{
		// Return the health of the blue team's object

		if ( _blueDestructionObject )
		{
			floatRealValue = ( _blueDestructionObject->health / _blueDestructionObject->max_health ) * 100.0f;

			if ( ( floatRealValue < 1.0f ) && ( _blueDestructionObject->health > 1.0f ) )
			{
				floatRealValue = 1.0f;
			}

			return ( (int)floatRealValue );

		}
	}
	
	return value;
}

bool ModifierDestruction::shouldKeepItem( MultiplayerItem *item )
{
	Event *event;

	// See if we care about this item

	if ( strnicmp( item->getName().c_str(), "DestructionObject", sizeof( "DestructionObject" ) - 1 ) == 0 )
	{
		// It's a destruction object

		if ( stricmp( item->getName().c_str(), "DestructionObject-red" ) == 0 )
			_redDestructionObject = item;
		else if ( stricmp( item->getName().c_str(), "DestructionObject-blue" ) == 0 )
			_blueDestructionObject = item;

		// Change the multiplayer item to suit our needs

		item->setSolidType( SOLID_BBOX );
		item->setContents( CONTENTS_SOLID );
		item->takedamage = DAMAGE_YES;

		event = new Event( EV_Trigger_SetDestructible );
		event->AddInteger( true );
		item->ProcessEvent( event );

		if ( item->health == 0 )
		{
			item->setMaxHealth( _defaultObjectHealth );
			item->setHealth( _defaultObjectHealth );
		}

		if ( stricmp( item->getName().c_str(), "DestructionObject-red" ) == 0 )
			_redObjectLasthealth = item->health;
		else if ( stricmp( item->getName().c_str(), "DestructionObject-blue" ) == 0 )
			_blueObjectLasthealth = item->health;


		// Tell the manager to keep the item

		return true;
	}

	return false;
}

float ModifierDestruction::itemDamaged( MultiplayerItem *item, Player *attackingPlayer, float damage, int meansOfDeath )
{
	Team *team;

	team = multiplayerManager.getPlayersTeam( attackingPlayer );

	if ( !team )
		return damage;

	if ( ( ( item == _redDestructionObject ) && ( team->getName() == "Red" ) ) ||
		 ( ( item == _blueDestructionObject ) && ( team->getName() == "Blue" ) ) )
	{
		// Can't hurt our own object

		return 0.0f;
	}

	if ( ( _respawnTime > 0.0f ) && ( _respawnTime > multiplayerManager.getTime() ) )
	{
		return 0.0f;
	}

	if ( _blueObjectDestroyed  || _redObjectDestroyed )
		return 0.0f;

	_destructionPlayerData[ attackingPlayer->entnum ]._damageDone += damage;

	while( _destructionPlayerData[ attackingPlayer->entnum ]._damageDone > _minDamageForPoints )
	{
		_destructionPlayerData[ attackingPlayer->entnum ]._damageDone -= _minDamageForPoints;

		multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForDamage );
	}

	// Play a sound to tell everyone the object was damaged

	if ( ( item == _redDestructionObject ) && ( multiplayerManager.getTime() > _redLastDamageSoundTime + 0.5 ) )
	{
		item->Sound( "impact_singularity", CHAN_AUTO, 1.0f, 500 );
		_redLastDamageSoundTime = multiplayerManager.getTime();
	}
	else if ( ( item == _blueDestructionObject ) && ( multiplayerManager.getTime() > _blueLastDamageSoundTime + 0.5 ) )
	{
		item->Sound( "impact_singularity", CHAN_AUTO, 1.0f, 500 );
		_blueLastDamageSoundTime = multiplayerManager.getTime();
	}

	// Scale the damage done to the singularity inversely to the number of players in the game

	damage /= (float)multiplayerManager.getTotalPlayers( false ) + 2.0f;

	return damage;
}

void ModifierDestruction::itemDestroyed( Player *player, MultiplayerItem *item )
{
	Team *team;

	if ( strnicmp( item->getName().c_str(), "DestructionObject", sizeof( "DestructionObject" ) - 1 ) == 0 )
	{
		// Snap health to 0

		item->setHealth( 0.0f );

		if ( item->animate && item->animate->HasAnim( "idle" ) )
		{
			item->animate->RandomAnimate( "idle" );
		}

		if ( stricmp( item->getName().c_str(), "DestructionObject-red" ) == 0 )
			_redObjectLasthealth = item->health;
		else if ( stricmp( item->getName().c_str(), "DestructionObject-blue" ) == 0 )
			_blueObjectLasthealth = item->health;

		item->SpawnEffect( "models/fx/fx-explosion-singularity.tik", item->origin, item->angles, 1.0f ); 

		// Give the team points

		team = multiplayerManager.getPlayersTeam( player );

		// Give the player points

		multiplayerManager.addPoints( player->entnum, _pointsForDestroying );

		if ( ( ( team->getName() == "Red" ) && ( item == _blueDestructionObject ) ) ||
			 ( ( team->getName() == "Blue" ) && ( item == _redDestructionObject ) ) )
		{
			team->addPoints( NULL, _pointsForDestroyingObject );

			if ( team->getName() == "Red" )
			{
				multiplayerManager.centerPrintAllClients( "$$BlueObjectDestroyed$$!", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_btsestroy.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
				_blueObjectDestroyed = true;
			}
			else
			{
				multiplayerManager.centerPrintAllClients( "$$RedObjectDestroyed$$!", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rtsestroy.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
				_redObjectDestroyed = true;
			}

			multiplayerManager.playerEventNotification( "destruction", item->getName().c_str(), player );

			// Respawn all of the players in a few seconds

			_respawnTime = multiplayerManager.getTime() + 3.0f;
		}
	}
}

void ModifierDestruction::itemUsed( Entity *entity, MultiplayerItem *item )
{
	Player *player;
	Equipment *equipment;
	Team * team;


	// Can't heal singularity during respawn time

	if ( _blueObjectDestroyed  || _redObjectDestroyed )
		return;

	if ( ( _respawnTime > 0.0f ) && ( _respawnTime >= multiplayerManager.getTime() ) )
	{
		return;
	}

	// We only care about equipment using stuff (tricorders to be exact)

	if ( entity && entity->isSubclassOf( Player ) )
	{
		player = (Player *)entity;

		if ( !( player->edict->svflags & SVF_BOT ) )
		{
			player->loadUseItem( "tricorder" );
		}
		return;
	}

	if ( !entity || entity->isSubclassOf( Player ) )
		return;

	if ( !entity->isSubclassOf( Equipment ) )
		return;

	equipment = (Equipment *)entity;

	if ( stricmp( equipment->getTypeName().c_str(), "tricorder" ) != 0 )
	{
		return;
	}

	// The tricorder must have a player as the owner

	if ( !equipment->GetOwner() || !equipment->GetOwner()->isSubclassOf( Player ) )
		return;

	player = (Player *)equipment->GetOwner();
	team = multiplayerManager.getPlayersTeam( player );

	if ( !team )
		return;

	// Heal the destruction object if everything is ok

	if ( ( ( item == _redDestructionObject ) && ( team->getName() == "Red" ) ) ||
		 ( ( item == _blueDestructionObject ) && ( team->getName() == "Blue" ) ) )
	{
		if ( _destructionPlayerData[ player->entnum ]._lastHealTime != level.time )
		{
			_destructionPlayerData[ player->entnum ]._lastHealTime = level.time;

			// Scale the healed damage done to the singularity inversely to the number of players in the game

			item->addHealth( _objectHealRate * level.frametime / ( multiplayerManager.getTotalPlayers( false ) + 2.0f ) );

			// Give points to player for healing object

			_destructionPlayerData[ player->entnum ]._healthHealed += _objectHealRate * level.frametime;

			while( _destructionPlayerData[ player->entnum ]._healthHealed > _minHealingForPoints )
			{
				_destructionPlayerData[ player->entnum ]._healthHealed -= _minHealingForPoints;

				multiplayerManager.addPoints( player->entnum, _pointsForHealing );
			}
		}
	}
}

void ModifierDestruction::update( float frameTime )
{
	float redHealth;
	float redMaxHealth;
	float blueHealth;
	float blueMaxHealth;
	int currentStage;
	int lastStage;


	if ( _redDestructionObject )
	{
		redHealth    = _redDestructionObject->health;
		redMaxHealth = _redDestructionObject->max_health;

		updateObjectAnim( _redDestructionObject, redHealth, _redObjectLasthealth, redMaxHealth );

		currentStage = getStage( redHealth, redMaxHealth );
		lastStage    = getStage( _redObjectLasthealth, redMaxHealth );

		if ( currentStage > lastStage )
		{
			if ( currentStage == 4 )
			{
				multiplayerManager.centerPrintAllClients( "$$RedObjectCritical$$", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rtscl.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
			}
			else if ( currentStage == 3 )
			{
				multiplayerManager.centerPrintAllClients( "$$RedObject25$$", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rts25.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
			}
			else if ( currentStage == 2 )
			{
				multiplayerManager.centerPrintAllClients( "$$RedObject50$$", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rts50.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
			}
		}

		_redObjectLasthealth = redHealth;
	}

	if ( _blueDestructionObject )
	{
		blueHealth    = _blueDestructionObject->health;
		blueMaxHealth = _blueDestructionObject->max_health;

		updateObjectAnim( _blueDestructionObject, blueHealth, _blueObjectLasthealth, blueMaxHealth );

		currentStage = getStage( blueHealth, blueMaxHealth );
		lastStage    = getStage( _blueObjectLasthealth, blueMaxHealth );

		if ( currentStage > lastStage )
		{
			if ( currentStage == 4 )
			{
				multiplayerManager.centerPrintAllClients( "$$BlueObjectCritical$$", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_btscl.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
			}
			else if ( currentStage == 3 )
			{
				multiplayerManager.centerPrintAllClients( "$$BlueObject25$$", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bts25.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
			}
			else if ( currentStage == 2 )
			{
				multiplayerManager.centerPrintAllClients( "$$BlueObject50$$", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bts50.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
			}
		}

		_blueObjectLasthealth = blueHealth;
	}

	// See if we should respawn everyone

	if ( ( _respawnTime > 0.0f ) && ( _respawnTime < multiplayerManager.getTime() ) )
	{
		multiplayerManager.respawnAllPlayers();
		_respawnTime = 0.0f;

		// Reset health back up to the max for which ever singularity was destroyed

		if ( _blueObjectDestroyed )
		{
			_blueDestructionObject->setHealth( _blueDestructionObject->max_health );
			_blueObjectDestroyed = false;
		}

		if ( _redObjectDestroyed )
		{
			_redDestructionObject->setHealth( _redDestructionObject->max_health );
			_redObjectDestroyed = false;
		}

	}

	// Regenerate some health

	/* if ( _redDestructionObject && ( _redDestructionObject->health < _redDestructionObject->max_health ) )
	{
		_redDestructionObject->addHealth( 1 * frameTime );
	}

	if ( _blueDestructionObject && ( _blueDestructionObject->health < _blueDestructionObject->max_health ) )
	{
		_blueDestructionObject->addHealth( 1 * frameTime );
	} */
}

void ModifierDestruction::updateObjectAnim( MultiplayerItem *destructionObject, float health, float lastHealth, float maxHealth )
{
	int currentStage;
	int lastStage;
	str animName;


	// Make sure everything is ok

	if ( !destructionObject )
		return;

	// Get the current and the last stage

	currentStage = getStage( health, maxHealth );
	lastStage    = getStage( lastHealth, maxHealth );

	// See if we have changed stages

	if ( currentStage != lastStage )
	{
		// Figure out the correct animation to play

		if ( currentStage == 1 )
			animName = "idle";
		else if ( currentStage == 2 )
			animName = "stage2";
		else if ( currentStage == 3 )
			animName = "stage3";
		else if ( currentStage == 4 )
			animName = "stage4";

		// Play the new animation if it exists

		if ( destructionObject->animate && destructionObject->animate->HasAnim( animName ) )
		{
			destructionObject->animate->RandomAnimate( animName );
		}
	}
}

int	ModifierDestruction::getStage( float health, float maxHealth )
{
	if ( health < maxHealth * 0.1f )
		return 4;
	else if ( health < maxHealth * 0.25f )
		return 3;
	else if ( health < maxHealth * 0.5f )
		return 2;
	else
		return 1;
}

bool ModifierDestruction::checkRule( const char *rule, bool defaultValue, Player *player )
{
	// We want team spawnpoints

	if ( stricmp( rule, "spawnpoints-team" ) == 0 )
		return true;
	else if ( stricmp( rule, "keepflags" ) == 0 )
		return false;
	else
		return defaultValue;
}

bool ModifierDestruction::checkGameType( const char *gameType )
{
	if ( stricmp( gameType, "destruction" ) == 0 )
		return true;
	else
		return false;
}

bool ModifierOneFlag::shouldKeepItem( MultiplayerItem *item )
{
	// We need to keep the one flag

	if ( ( stricmp( item->getName().c_str(), "ctfflag-one" ) == 0 ) ||
		 ( stricmp( item->getName().c_str(), "ctfflag-baseone" ) == 0 ) )
	{
		if ( !multiplayerManager.checkRule ( "keepflags", true ) )
			return false;

		return true;
	}
	else
	{
		return false;
	}
}

bool ModifierOneFlag::checkRule( const char *rule, bool defaultValue, Player *player )
{
	// Check to see if we care about this rule

	if ( strnicmp( rule, "flagscore-", sizeof( "flagscore-" ) - 1 ) == 0 )
	{
		// See if this flag touch should generate a flag score

		if ( stricmp( rule, "flagscore-teamflag" ) == 0 )
			return false;
		else if ( stricmp( rule, "flagscore-enemyflag" ) == 0 )
			return true;
	}
	else if ( strnicmp( rule, "flagpickup-", sizeof( "flagpickup-" ) - 1 ) == 0 )
	{
		// See if this flag touch should generate a flag pickup

		if ( stricmp( rule, "flagpickup-enemyflag" ) == 0 )
			return false;
		else if ( stricmp( rule, "flagpickup-otherflag-ctfflag-one" ) == 0 )
			return true;
	}

	return defaultValue;
}

bool ModifierOneFlag::checkGameType( const char *gameType )
{
	if ( stricmp( gameType, "oneflag" ) == 0 )
		return true;
	else
		return false;
}

void ModifierOneFlag::addPlayer( Player *player )
{
	gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_oneflagstatus\"\n" );
}

const int ModifierElimination::_pointsForBeingLastAlive = 5;

ModifierElimination::ModifierElimination()
{
	_respawning = false;
	_playerEliminated = false;

	_playerEliminationData = NULL;

	_eliminatedIconIndex   = gi.imageindex( "sysimg/icons/mp/elimination_eliminated" );

	_needPlayers = true;

	_eliminatedTextIndex = G_FindConfigstringIndex( "$$EliminatedWait$$", CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true ) + CS_GENERAL_STRINGS;;
	_nextRoundTextIndex = G_FindConfigstringIndex( "$$NextRoundWait$$", CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true ) + CS_GENERAL_STRINGS;;
}

ModifierElimination::~ModifierElimination()
{
	delete [] _playerEliminationData;
}

void ModifierElimination::init( int maxPlayers )
{
	_maxPlayers = maxPlayers;
	_playerEliminationData = new EliminationPlayerData[ maxPlayers ];

	multiplayerManager.cacheMultiplayerFiles( "mp_elimination" );
}

void ModifierElimination::reset( void )
{
	int i;

	_playerEliminated = false;
	_matchOver = false;

	for ( i = 0 ; i < _maxPlayers ; i++ )
	{
		_playerEliminationData[ i ].reset();
	}
}

bool ModifierElimination::checkRule( const char *rule, bool defaultValue, Player *player )
{
	// Only let players respawn between matches

	if ( ( stricmp( rule, "respawnPlayer" ) == 0 ) )
	{
		if ( !player || _playerEliminationData[ player->entnum ]._eliminated )
			return false;
	}
	else if ( ( stricmp( rule, "spawnPlayer" ) == 0 ) || ( stricmp( rule, "changeTeams" ) == 0 ) )
	{
		// Don't spawn him if he has been eliminated

		if ( player && _playerEliminationData[ player->entnum ]._eliminated )
			return false;

		// Ok if in first 30 seconds of match

		if ( _matchStartTime + 5 > multiplayerManager.getTime() )
			return defaultValue;

		// See if we still need players

		if ( _needPlayers )
		{
			return defaultValue;
		}
		else
		{
			return false;
		}
	}

	return defaultValue;
}

void ModifierElimination::addPlayer( Player *player )
{
	_playerEliminationData[ player->entnum ].reset();

	if ( multiplayerManager.checkGameType( "dm" ) )
		gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_elimination\"\n" );
	else
		gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_eliminationteam\"\n" );

	if ( multiplayerManager.isPlayerSpectator( player ) && !multiplayerManager.isPlayerSpectatorByChoice( player ) )
	{
		_playerEliminationData[ player->entnum ]._eliminated = true;
	}
}

void ModifierElimination::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	Team *team;

	if ( !killedPlayer )
		return;

	_playerEliminated = true;

	_playerEliminationData[ killedPlayer->entnum ]._eliminated = true;

	team = multiplayerManager.getPlayersTeam( killedPlayer );

	if ( !team || ( numPlayersAliveOnTeam( team->getName() ) > 0 ) )
	{
		multiplayerManager.HUDPrintAllClients( va( "%s $$Eliminated$$\n", killedPlayer->client->pers.netname ) );
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_pyelim.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.0f );
	}
}

int ModifierElimination::getScoreIcon( Player *player, int index, int value )
{
	if ( index == SCOREICON3 )
	{
		if ( _playerEliminationData[ player->entnum ]._eliminated )
			return _eliminatedIconIndex;
		else 
			return 0;
	}

	return value;
}

int ModifierElimination::getStat( Player *player, int statNum, int value )
{
	int numAlive = 0;
	Team *team;
	Player *currentPlayer;
	gentity_t *edict;
	int i;

	if ( statNum == STAT_MP_GENERIC5 )
	{
		if ( multiplayerManager.checkGameType( "dm" ) )
		{
			for( i = 0 ; i < game.maxclients ; i++ )
			{
				edict = &g_entities[ i ];

				// Make sure this edict is a valid player

				if ( !edict->inuse || !edict->entity || !edict->entity->isSubclassOf( Player ) )
					continue;

				currentPlayer = ( Player * )edict->entity;

				if ( !_playerEliminationData[ currentPlayer->entnum ]._eliminated && !multiplayerManager.isPlayerSpectator( currentPlayer ) )
					numAlive++;
			}

			return numAlive;
		}
		else
		{
			for( i = 0 ; i < game.maxclients ; i++ )
			{
				edict = &g_entities[ i ];

				// Make sure this edict is a valid player

				if ( !edict->inuse || !edict->entity || !edict->entity->isSubclassOf( Player ) )
					continue;

				currentPlayer = ( Player * )edict->entity;

				// Get the player's team

				team = multiplayerManager.getPlayersTeam( currentPlayer );

				if ( team && team->getName() == "Red" && !_playerEliminationData[ currentPlayer->entnum ]._eliminated &&
					 !multiplayerManager.isPlayerSpectator( currentPlayer ) )
				{			 
					numAlive++;
				}
			}

			return numAlive;
		}
	}
	else if ( statNum == STAT_MP_GENERIC6 )
	{
		if ( !multiplayerManager.checkGameType( "dm" ) )
		{
			for( i = 0 ; i < game.maxclients ; i++ )
			{
				edict = &g_entities[ i ];

				// Make sure this edict is a valid player

				if ( !edict->inuse || !edict->entity || !edict->entity->isSubclassOf( Player ) )
					continue;

				currentPlayer = ( Player * )edict->entity;

				// Get the player's team

				team = multiplayerManager.getPlayersTeam( currentPlayer );

				if ( team && team->getName() == "Blue" && !_playerEliminationData[ currentPlayer->entnum ]._eliminated &&
					 !multiplayerManager.isPlayerSpectator( currentPlayer ) )
				{			 
					numAlive++;
				}
			}

			return numAlive;
		}
	}
	else if ( statNum == STAT_MP_STATE )
	{
		if ( _playerEliminationData[ player->entnum ]._eliminated )
			return _eliminatedTextIndex;
		
		if ( multiplayerManager.isPlayerSpectator( player ) && multiplayerManager.isFightingAllowed() )
			return _nextRoundTextIndex;
	}

	return value;
}

void ModifierElimination::matchStarting( void )
{
	reset();

	_matchStartTime = multiplayerManager.getTime();
	_needPlayers = true;
}

int ModifierElimination::numPlayersAliveOnTeam( const str &teamName )
{
	int i;
	Player *player;
	gentity_t *edict;
	int numPlayers;
	Team *team;


	numPlayers = 0;

	for( i = 0 ; i < game.maxclients ; i++ )
	{
		edict = &g_entities[ i ];

		// Make sure this edict is a valid player

		if ( !edict->inuse || !edict->entity || edict->entity->isSubclassOf( Player ) )
			continue;

		player = ( Player * )edict->entity;

		// Get the player's team

		team = multiplayerManager.getPlayersTeam( player );

		if ( team && ( team->getName() == teamName ) && !_playerEliminationData[ player->entnum ]._eliminated )
		{		
			numPlayers++;
		}
	}

	return numPlayers;
}

void ModifierElimination::update( float frameTime )
{
	int numAlive = 0;
	int numRedTeamAlive = 0;
	int numBlueTeamAlive = 0;
	Team *team;
	bool redTeam = false;
	bool blueTeam = false;
	bool usingTeams = false;
	bool respawnEveryone;
	Player *playerAlive = NULL;
	Player      *player;
	Entity      *entity;
	gentity_t   *edict;
	int i;


	if ( _matchOver )
		return;

	// Gather number of people alive and dead

	for( i = 0 ; i < game.maxclients ; i++ )
	{
		edict = &g_entities[ i ];

		// Make sure this edict is a valid player

		if ( !edict->inuse || !edict->entity )
			continue;

  		entity = edict->entity;

		if ( !entity->isSubclassOf( Player ) )
			continue;

		player = ( Player * )entity;

		// Get the player's team

		team = multiplayerManager.getPlayersTeam( player );

		redTeam = false;
		blueTeam = false;

		if ( team )
		{			 
			if ( stricmp( team->getName().c_str(), "red" ) == 0 )
			{
				redTeam = true;
				usingTeams = true;
			}
			else if ( stricmp( team->getName().c_str(), "blue" ) == 0 )
			{
				blueTeam = true;
				usingTeams = true;
			}
		}

		// Calculate number of people alive on each team

		if ( !_playerEliminationData[ player->entnum ]._eliminated && !multiplayerManager.isPlayerSpectator( player ) )
		{
			numAlive++;

			playerAlive = player;

			if ( redTeam )
				numRedTeamAlive++;
			else if ( blueTeam )
				numBlueTeamAlive++;
		}
	}

	// See if we need players

	if ( usingTeams )
	{
		if ( numRedTeamAlive == 0 || numBlueTeamAlive == 0 )
			_needPlayers = true;
		else
			_needPlayers = false;
			
	}
	else
	{
		if ( numAlive < 2  )
			_needPlayers = true;
		else
			_needPlayers = false;
	}

	if ( !_playerEliminated )
		return;

	// See if the round is over

	respawnEveryone = false;

	if ( usingTeams )
	{
		// Determine if only one team is alive

		if ( ( numRedTeamAlive > 0 ) && ( numBlueTeamAlive == 0 ) )
		{
			multiplayerManager.centerPrintAllClients( "$$BlueTeamEliminated$$", CENTERPRINT_IMPORTANCE_NORMAL );
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_btelim.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );

			if ( multiplayerManager.checkRule ( "endmatch-elimination-blue", true, NULL ) )
			{
				respawnEveryone = true;
				multiplayerManager.addTeamPoints( "Red", 1 );
			}
			else
			{
				_matchOver = true;
			}
		}
		else if ( ( numBlueTeamAlive > 0 ) && ( numRedTeamAlive == 0 ) )
		{
			multiplayerManager.centerPrintAllClients( "$$RedTeamEliminated$$", CENTERPRINT_IMPORTANCE_NORMAL );
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_rtelim.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );

			if ( multiplayerManager.checkRule ( "endmatch-elimination-red", true, NULL ) )
			{
				respawnEveryone = true;
				multiplayerManager.addTeamPoints( "Blue", 1 );
			}
			else
			{
				_matchOver = true;
			}
		}
		else if ( ( numBlueTeamAlive == 0 ) && ( numRedTeamAlive == 0 ) )
		{
			multiplayerManager.centerPrintAllClients( "$$NoOneWinsRound$$!", CENTERPRINT_IMPORTANCE_NORMAL );

			if ( multiplayerManager.checkRule ( "endmatch-elimination-both", true, NULL ) )
			{
				respawnEveryone = true;
			}
			else
			{
				_matchOver = true;
			}
		}
	}
	else
	{
		// Determine if one person won the match

		if ( numAlive == 1 )
		{
			str stringToPrint;

			stringToPrint = playerAlive->client->pers.netname;
			stringToPrint += " $$WinsRound$$!";

			multiplayerManager.centerPrintAllClients( stringToPrint.c_str(), CENTERPRINT_IMPORTANCE_NORMAL );
			respawnEveryone = true;

			multiplayerManager.addPoints( playerAlive->entnum, _pointsForBeingLastAlive );
		}
		else if ( numAlive == 0 )
		{
			multiplayerManager.centerPrintAllClients( "$$NoOneWinsRound$$!", CENTERPRINT_IMPORTANCE_NORMAL );
			respawnEveryone = true;
		}
	}

	if ( respawnEveryone )
	{
		// The match has been won, restart the match

		multiplayerManager.endMatch();
		//multiplayerManager.restartMatch();

		_matchOver = true;
	}
}

// Setup constants

const float	ModifierDiffusion::_timeNeededToArmBomb = 5.0f;
const float	ModifierDiffusion::_timeNeededToDisarmBomb = 5.0f;
const float	ModifierDiffusion::_maxGuardingDist = 750.0f;

const float	ModifierDiffusion::_maxArmingPause    = 1.0f;
const float	ModifierDiffusion::_maxDisarmingPause = 1.0f;

const float	ModifierDiffusion::_maxBombOnGroundTime = 30.0f;

const int ModifierDiffusion::_pointsForArmingBomb        = 25;
const int ModifierDiffusion::_pointsForExplodingBomb     = 50;
const int ModifierDiffusion::_pointsForDisarmingBomb     = 50;
const int ModifierDiffusion::_pointsForGuardingBase      = 5;
const int ModifierDiffusion::_pointsForGuardingTheBomber = 5;
const int ModifierDiffusion::_pointsForGuardingBomb      = 5;
const int ModifierDiffusion::_pointsForKillingTheBomber  = 10;

const int ModifierDiffusion::_teamPointsForBombing = 250;

DiffusionBombPlace::DiffusionBombPlace()
{
	_item = NULL;
	reset();
}

void DiffusionBombPlace::reset()
{
	_armed = false;

	_totalArmingTime    = 0.0f;
	_totalDisarmingTime = 0.0f;

	_lastArmingTime    = 0.0f;
	_lastDisarmingTime = 0.0f;

	_totalArmedTime = 0.0f;
};

ModifierDiffusion::ModifierDiffusion()
{
	_bomber = -1;
	_bombArmedByPlayer = -1;

	_lastBomber = -1;
	_bombDroppedTime = 0.0f;

	_bomb         = NULL;
	_tempBombItem = NULL;

	_bomberIconIndex = gi.imageindex( "sysimg/icons/mp/diffusion_bomber" );

	_redBombPlaceArmedIconIndex  = gi.imageindex( "sysimg/icons/mp/diffusion_bombArmed-red" );
	_blueBombPlaceArmedIconIndex = gi.imageindex( "sysimg/icons/mp/diffusion_bombArmed-blue" );

	_bombCarriedByRedTeamIconIndex  = gi.imageindex( "sysimg/icons/mp/diffusion_bombTaken-red" );
	_bombCarriedByBlueTeamIconIndex = gi.imageindex( "sysimg/icons/mp/diffusion_bombTaken-blue" );

	_bombInBaseIconIndex   = gi.imageindex( "sysimg/icons/mp/diffusion_bombNormal" );
	_bombOnGroundIconIndex = gi.imageindex( "sysimg/icons/mp/diffusion_bombOnground" );

	_respawnTime = 0.0f;
}

ModifierDiffusion::~ModifierDiffusion()
{
	multiplayerManager.resetRespawnTime();
}

void ModifierDiffusion::init( int maxPlayers )
{
	_maxPlayers = maxPlayers;

	_timeNeededForBombToExplode = mp_bombTime->value;

	if ( multiplayerManager.getRespawnTime() < 0.0f )
	{
		multiplayerManager.setRespawnTime( 5.0f );
	}

	multiplayerManager.cacheMultiplayerFiles( "mp_diffusion" );
}

void ModifierDiffusion::playerSpawned( Player *player )
{
	multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-tricorder.tik" );
	
	multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-compressionrifle.tik" );
	multiplayerManager.usePlayerItem( player->entnum, "CompressionRifle" );
}

void ModifierDiffusion::playerEventNotification( const char *eventName, const char *eventItemName, Player *eventPlayer )
{
	if ( stricmp( eventName, "use-HoldableItem" ) == 0 )
	{
		if ( stricmp( eventItemName, "Transporter" ) == 0 )
		{
			dropBomb( eventPlayer );
		}
	}
}

bool ModifierDiffusion::checkRule( const char *rule, bool defaultValue, Player *player )
{
	// We want team spawnpoints

	if ( stricmp( rule, "spawnpoints-team" ) == 0 )
	{
		return true;
	}

	return defaultValue;
}

bool ModifierDiffusion::shouldKeepItem( MultiplayerItem *item )
{
	if ( strnicmp( item->getName().c_str(), "Diffusion-", strlen( "Diffusion-" ) ) == 0 )
	{
		// This is a diffusion item so keep track of it

		if ( stricmp( item->getName().c_str(), "Diffusion-bombplace-red" ) == 0 )
			_redBombPlace._item = item;
		else if ( stricmp( item->getName().c_str(), "Diffusion-bombplace-blue" ) == 0 )
			_blueBombPlace._item = item;
		else if ( stricmp( item->getName().c_str(), "Diffusion-bomb" ) == 0 )
			_bomb = item;
		else
			return false;

		return true;
	}

	return false;
}

void ModifierDiffusion::itemTouched( Player *player, MultiplayerItem *item )
{
	str printString;


	// Can't pickup bomb during respawn period

	if ( _respawnTime > 0.0f )
		return;

	// Make sure this is a bomb

	if ( ( item != _bomb ) && ( item != _tempBombItem ) )
		return;

	// Make sure this isn't a bot

	if ( player->edict->svflags & SVF_BOT )
		return;

	// Make sure we can't pickup the bomb right after we dropped it

	if ( ( player->entnum == _lastBomber ) && ( _bombDroppedTime + 0.5 > multiplayerManager.getTime() ) )
		return;

	// Give the bomb to the player 

	makeBomber( player );

	// Tell everyone that the bomb has been picked up

	printString = "$$BombTaken$$ ";
	printString += player->client->pers.netname;

	printString += " ($$";
	printString += multiplayerManager.getPlayersTeam( player )->getName();
	printString += "$$ $$Team$$)!";

	multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_HIGH );

	if ( multiplayerManager.getPlayersTeam( player )->getName() == "Red" )
	{
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bombtakenred.mp3", CHAN_AUTO, DEFAULT_VOL, 
				DEFAULT_MIN_DIST, player, 1.5f );
	}
	else
	{
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bombtakenblue.mp3", CHAN_AUTO, DEFAULT_VOL, 
				DEFAULT_MIN_DIST, player, 1.5f );
	}

	if ( item == _tempBombItem )
	{
		// Get rid of the temporary bomb

		_tempBombItem->PostEvent( EV_Remove, 0.0f );
		_tempBombItem = NULL;
	}
	else
	{
		// Hide the item picked up

		_bomb->setSolidType( SOLID_NOT );
		_bomb->hideModel();
	}
}

void ModifierDiffusion::dropBomb( Player *player )
{
	MultiplayerItem *newBomb;
	str printString;

	// Make sure everything is ok

	if ( player != getBomber() )
		return;

	// Create a new bomb to put on the ground

	newBomb = new MultiplayerItem;

	newBomb->setModel( _bomb->model );

	newBomb->angles = player->angles;
	newBomb->setAngles();

	newBomb->setOrigin( player->origin );

	newBomb->CancelEventsOfType( EV_ProcessInitCommands );
	newBomb->ProcessInitCommands( newBomb->edict->s.modelindex );

	newBomb->setName( "Diffusion-bomb-temp" );

	// Setup the _tempBombItem stuff

	_tempBombItem = newBomb;
	_tempBombItemTime = multiplayerManager.getTime();

	// Change the player back to not being a bomber

	clearBomber();
	_bombDroppedTime = multiplayerManager.getTime();

	// Tell the team that the bomb was dropped

	printString = "$$BombDropped$$";
	
	multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_HIGH );
}

void ModifierDiffusion::respawnBomb( bool quiet )
{
	str printString;

	if ( _tempBombItem )
	{
		_tempBombItem->PostEvent( EV_Remove, 0.0f );
		_tempBombItem = NULL;
	}

	if ( _bomb )
	{
		_bomb->showModel();
		_bomb->setSolidType( SOLID_TRIGGER );
	}

	if ( !quiet )
	{
		printString = "$$BombReturnedToBase$$";
		multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_HIGH );

		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bombbase.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
	}
}

void ModifierDiffusion::itemUsed( Entity *entity, MultiplayerItem *item )
{
	Equipment *equipment;
	Sentient *owner;
	Player *player;
	bool offense;
	bool defense;
	Team *team;
	DiffusionBombPlace *enemyBombPlace;
	DiffusionBombPlace *teamBombPlace;
	str printString;

	if ( entity && entity->isSubclassOf( Player ) )
	{
		player = (Player *)entity;

		if ( !( player->edict->svflags & SVF_BOT ) )
		{
			player->loadUseItem( "tricorder" );
		}

		return;
	}

	if ( !entity || !entity->isSubclassOf( Equipment ) )
		return;

	// Get the use info

	equipment = (Equipment *)entity;

	owner = equipment->GetOwner();

	if ( !owner || !owner->isSubclassOf( Player ) )
		return;

	player = (Player *)owner;

	if ( stricmp( equipment->getTypeName().c_str(), "tricorder" ) != 0 )
	{
		return;
	}

	// Figure out if the player is on offense or defense

	team = multiplayerManager.getPlayersTeam( player );

	if ( !team )
		return;

	offense = false;
	defense = false;

	enemyBombPlace = NULL;
	teamBombPlace  = NULL;

	if ( team->getName() == "Red" )
	{
		if ( item == _redBombPlace._item )
			defense = true;
		else if ( item == _blueBombPlace._item )
			offense = true;

		enemyBombPlace = &_blueBombPlace;
		teamBombPlace  = &_redBombPlace;
	}
	else if ( team->getName() == "Blue" )
	{
		if ( item == _blueBombPlace._item )
			defense = true;
		else if ( item == _redBombPlace._item )
			offense = true;

		enemyBombPlace = &_redBombPlace;
		teamBombPlace  = &_blueBombPlace;
	}

	if ( offense )
	{
		// Must be the bomber

		if ( player->entnum != _bomber )
			return;

		if ( !enemyBombPlace->_armed && ( enemyBombPlace->_lastArmingTime != level.time ) )
		{
			enemyBombPlace->_lastArmingTime = level.time;

			enemyBombPlace->_totalArmingTime += level.frametime;

			if ( enemyBombPlace->_totalArmingTime > _timeNeededToArmBomb )
			{
				SpawnArgs args;
				int tagNum;

				_bombArmedByPlayer = player->entnum;

				enemyBombPlace->_armed = true;

				enemyBombPlace->_lastDisarmingTime = 0.0f;
				enemyBombPlace->_totalDisarmingTime = 0.0f;
				enemyBombPlace->_totalArmedTime = 0.0f;

				clearBomber();

				// Tell everyone the bomb has been armed

				printString = "$$BombArmed$$ ";
				printString += player->client->pers.netname;
				printString += " ($$";
				printString += multiplayerManager.getPlayersTeam( player )->getName();
				printString += "$$ $$Team$$)!";

				multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_NORMAL );

				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bomba.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.0f );

				// Spawn a bomb in the bomb place

				args.setArg( "model", "models/item/mp_diffusion_bomb.tik" );

				_attachedBomb = args.Spawn();

				if ( !_attachedBomb )
					return;

				tagNum = gi.Tag_NumForName( enemyBombPlace->_item->edict->s.modelindex, "tag_bomb" );

				if ( tagNum >= 0 )
				{
					_attachedBomb->attach( enemyBombPlace->_item->entnum, tagNum );
				}
				else
				{
					_attachedBomb->setOrigin( enemyBombPlace->_item->origin );
				}

				_attachedBomb->ProcessPendingEvents();

				_attachedBomb->setSolidType( SOLID_NOT );

				_attachedBomb->SetTargetName( "attachedDiffusionBomb" );

				if ( !_attachedBomb->animate )
					_attachedBomb->animate = new Animate( _attachedBomb );

				_attachedBomb->animate->RandomAnimate( "idle" );
			}
		}
	}
	else if ( defense )
	{
		// Make sure the player's team's bomb place has been armed

		if ( !teamBombPlace->_armed )
			return;

		if ( teamBombPlace->_lastDisarmingTime != level.time )
		{
			teamBombPlace->_lastDisarmingTime = level.time;

			teamBombPlace->_totalDisarmingTime += level.frametime;

			if ( teamBombPlace->_totalDisarmingTime > _timeNeededToDisarmBomb )
			{
				// Disarmed the bomb

				teamBombPlace->_armed = false;

				teamBombPlace->_item->removeAttachedModelByTargetname( "attachedDiffusionBomb" );

				makeBomber( player );

				// Tell everyone the bomb has been armed

				printString = "$$BombDisarmed$$ ";
				printString += player->client->pers.netname;
				printString += " ($$";
				printString += multiplayerManager.getPlayersTeam( player )->getName();
				printString += "$$ $$Team$$)!";

				multiplayerManager.centerPrintAllClients( printString, CENTERPRINT_IMPORTANCE_NORMAL );

				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bombdisarmed.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, player, 1.0f );
			}
		}
	}
}

void ModifierDiffusion::playerChangedModel( Player *player )
{
	if ( player->entnum == _bomber )
	{
		attachBomb( player );
	}
}

void ModifierDiffusion::attachBomb( Player *player )
{
	Event *attachEvent;
	str tagName;

	player->removeAttachedModelByTargetname( "attachedDiffusionBomb" );

	if ( gi.Tag_NumForName( player->edict->s.modelindex, "Bip01 spine2" ) >= 0 )
		tagName = "Bip01 spine2";
	else if ( gi.Tag_NumForName( player->edict->s.modelindex, "Bip01 spine1" ) >= 0 )
		tagName = "Bip01 spine1";
	else if ( gi.Tag_NumForName( player->edict->s.modelindex, "Bip01 Head" ) >= 0 )
		tagName = "Bip01 Head";

	attachEvent = new Event( EV_AttachModel );

	attachEvent->AddString( "models/item/mp_diffusion_bomb.tik" );
	attachEvent->AddString( tagName );
	attachEvent->AddFloat( 1.0f );
	attachEvent->AddString( "attachedDiffusionBomb" );
	attachEvent->AddInteger( 0 );
	attachEvent->AddFloat( -1.0f );
	attachEvent->AddFloat( 0.0f );
	attachEvent->AddFloat( -1.0f );
	attachEvent->AddFloat( -1.0f );
	attachEvent->AddVector( player->getBackpackAttachOffset() );
	attachEvent->AddVector( player->getBackpackAttachAngles() );

	player->ProcessEvent( attachEvent );
}

int ModifierDiffusion::getStat( Player *player, int statNum, int value )
{
	if ( statNum == STAT_MP_GENERIC1 )
	{
		if ( _redBombPlace._armed )
		{
			return (int)( 100.0f - ( _redBombPlace._totalDisarmingTime / _timeNeededToDisarmBomb ) * 100.0f );
		}
		else
		{
			return (int)( ( _redBombPlace._totalArmingTime / _timeNeededToArmBomb ) * 100.0f );
		}
	}
	else if ( statNum == STAT_MP_GENERIC3 )
	{
		if ( _blueBombPlace._armed )
		{
			return (int)( 100.0f - ( _blueBombPlace._totalDisarmingTime / _timeNeededToDisarmBomb ) * 100.0f );
		}
		else
		{
			return (int)( ( _blueBombPlace._totalArmingTime / _timeNeededToArmBomb ) * 100.0f );
		}
	}
	else if ( statNum == STAT_MP_GENERIC2 )
	{

		if ( _redBombPlace._armed )
		{
			return (int)ceil( _timeNeededForBombToExplode - _redBombPlace._totalArmedTime );
		}
		else if ( _blueBombPlace._armed )
		{
			return (int)ceil( _timeNeededForBombToExplode - _blueBombPlace._totalArmedTime );
		}
		else
		{
			return 999;
		}
	}
	else if ( statNum == STAT_MP_GENERIC7 )
	{
		Player *bomber = getBomber();
		str bomberTeamName;

		if ( bomber )
		{
			bomberTeamName = multiplayerManager.getPlayersTeam( bomber )->getName();
		}

		if ( _redBombPlace._armed )
			return _redBombPlaceArmedIconIndex;
		else if ( _blueBombPlace._armed )
			return _blueBombPlaceArmedIconIndex;
		else if ( bomber && ( bomberTeamName == "Red" ) )
			return _bombCarriedByRedTeamIconIndex;
		else if ( bomber && ( bomberTeamName == "Blue" ) )
			return _bombCarriedByBlueTeamIconIndex;
		else if ( _bomb->getSolidType() != SOLID_NOT )
			return _bombInBaseIconIndex;
		else
			return _bombOnGroundIconIndex;
	}
	
	return value;
}

void ModifierDiffusion::addPlayer( Player *player )
{
	gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_diffusion\"\n" );
}

void ModifierDiffusion::removePlayer( Player *player )
{
	// Put any items back that are necessary

	if ( player->entnum == _bomber )
	{
		dropBomb( player );
	}

	if ( _bombArmedByPlayer == player->entnum )
	{
		_bombArmedByPlayer = -1;
	}
}

void ModifierDiffusion::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	bool victimWasBomber;

	if ( killedPlayer->entnum == _bomber )
	{
		dropBomb( killedPlayer );

		victimWasBomber = true;
	}
	else
	{
		victimWasBomber = false;
	}

	// Make sure this was a good kill

	if ( !attackingPlayer || ( attackingPlayer == killedPlayer ) )
		return;

	if ( multiplayerManager.getPlayersTeam( killedPlayer ) && multiplayerManager.getPlayersTeam( attackingPlayer ) &&
		 ( multiplayerManager.getPlayersTeam( killedPlayer ) == multiplayerManager.getPlayersTeam( attackingPlayer ) ) )
		return;

	// Award extra points (if any)

	if ( victimWasBomber )
	{
		// Give points to the player that killed the bomber

		multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForKillingTheBomber );
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bomberKilled.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.0f );
	}
	
	// Give more points possibly

	if ( playerGuardedBomber( attackingPlayer, killedPlayer ) )
	{
		// Give points to the player that guarded the bomber

		multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForGuardingTheBomber );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_bomberguarded.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.2f );
	}
	else if ( playerGuardedBase( attackingPlayer, killedPlayer ) )
	{
		// Give points to the player that guarded the armed bomb

		multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForGuardingBase );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_baseguarded.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.2f );
	}
	else if ( playerGuardedBomb( attackingPlayer, killedPlayer ) )
	{
		// Give points to the player that guarded the armed bomb

		multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForGuardingBomb );
		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_bombguarded.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.2f );
	}
}

Player *ModifierDiffusion::getBomber( void )
{
	if ( _bomber >= 0 )
		return multiplayerManager.getPlayer( _bomber );
	else
		return NULL;
}

bool ModifierDiffusion::playerGuardedBomber( Player *attackingPlayer, Player *killedPlayer )
{
	Player *bomber;

	// Get the bomber

	bomber = getBomber();

	if ( !bomber )
		return false;

	// Make sure bomber is on the same team as the attacker

	if ( multiplayerManager.getPlayersTeam( attackingPlayer ) != multiplayerManager.getPlayersTeam( bomber ) )
		return false;

	//  See if the attacking or killed player is within the guarding distance of the bomber

	if ( withinGuardDistance( bomber->origin, attackingPlayer->origin ) || 
		 withinGuardDistance( bomber->origin, killedPlayer->origin ) )
		 return true;
	else
		return false;
}

bool ModifierDiffusion::playerGuardedBase( Player *attackingPlayer, Player *killedPlayer )
{
	MultiplayerItem *bombPlace;
	Team *team;

	team = multiplayerManager.getPlayersTeam( attackingPlayer );

	if ( !team )
		return false;

	// Get the player's bomb place

	if ( team->getName() == "Red" )
		bombPlace = _redBombPlace._item;
	else
		bombPlace = _blueBombPlace._item;

	if ( !bombPlace )
		return false;

	//  See if the attacking or killed player is within the guarding distance of the bomb place

	if ( withinGuardDistance( bombPlace->origin, attackingPlayer->origin ) || 
		 withinGuardDistance( bombPlace->origin, killedPlayer->origin ) )
		return true;
	else
		return false;
}

bool ModifierDiffusion::playerGuardedBomb( Player *attackingPlayer, Player *killedPlayer )
{
	DiffusionBombPlace *bombPlace;
	Team *team;

	team = multiplayerManager.getPlayersTeam( attackingPlayer );

	if ( !team )
		return false;

	// Get the player's bomb place

	if ( team->getName() == "Red" )
		bombPlace = &_blueBombPlace;
	else
		bombPlace = &_redBombPlace;

	if ( !bombPlace )
		return false;

	if ( !bombPlace->_armed )
		return false;

	//  See if the attacking or killed player is within the guarding distance of the bomb

	if ( withinGuardDistance( bombPlace->_item->origin, attackingPlayer->origin ) || 
		 withinGuardDistance( bombPlace->_item->origin, killedPlayer->origin ) )
		return true;
	else
		return false;
}

bool ModifierDiffusion::withinGuardDistance( const Vector &origin1, const Vector &origin2 )
{
	Vector diff;
	float distance;

	diff = origin1 - origin2;
	distance = diff.length();

	if ( distance < _maxGuardingDist )
		return true;
	else
		return false;
}


void ModifierDiffusion::playerCommand( Player *player, const char *command, const char *parm )
{
	if ( stricmp( command, "dropItem" ) == 0 )
	{
		if ( player == getBomber() )
		{
			dropBomb( player );
		}
	}
}

void ModifierDiffusion::makeBomber( Player *player )
{
	_bomber     = player->entnum;
	_lastBomber = player->entnum;

	multiplayerManager.playerSound( player->entnum, "localization/sound/dialog/dm/comp_bomber.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.5f );

	// Attach the bomb to the player

	attachBomb( player );
}

void ModifierDiffusion::clearBomber( void )
{
	if ( _bomber >= 0 )
	{
		Player *player;

		player = multiplayerManager.getPlayer( _bomber );

		if ( player )
		{
			player->removeAttachedModelByTargetname( "attachedDiffusionBomb" );
		}
	}

	_bomber = -1;
}

void ModifierDiffusion::update( float frameTime )
{
	float currentTime;


	currentTime = multiplayerManager.getTime();

	// Make sure the red bomb place has been armed recently enough

	if ( _redBombPlace._lastArmingTime + _maxArmingPause < currentTime )
	{
		_redBombPlace._lastArmingTime  = 0.0f;
		_redBombPlace._totalArmingTime = 0.0f;
	}

	// Make sure the red bomb place has been diffused recently enough

	if ( _redBombPlace._lastDisarmingTime + _maxDisarmingPause < currentTime )
	{
		_redBombPlace._lastDisarmingTime  = 0.0f;
		_redBombPlace._totalDisarmingTime = 0.0f;
	}

	// Make sure the blue bomb place has been armed recently enough

	if ( _blueBombPlace._lastArmingTime + _maxArmingPause < currentTime )
	{
		_blueBombPlace._lastArmingTime  = 0.0f;
		_blueBombPlace._totalArmingTime = 0.0f;
	}

	// Make sure the blue bomb place has been diffused recently enough

	if ( _blueBombPlace._lastDisarmingTime + _maxDisarmingPause < currentTime )
	{
		_blueBombPlace._lastDisarmingTime  = 0.0f;
		_blueBombPlace._totalDisarmingTime = 0.0f;
	}

	// Return the bomb back to the base if it has been sitting on the ground too long

	if ( _tempBombItem && ( _tempBombItemTime + _maxBombOnGroundTime < multiplayerManager.getTime() ) )
	{
		respawnBomb();
	}

	// See if the bomb has exploded

	if ( _redBombPlace._armed || _blueBombPlace._armed )
	{
		int lastTimeLeft;
		int timeLeft;
		DiffusionBombPlace *armedBombPlace;

		if ( _redBombPlace._armed )
			armedBombPlace = &_redBombPlace;
		else
			armedBombPlace = &_blueBombPlace;

		lastTimeLeft = (int )( _timeNeededForBombToExplode - armedBombPlace->_totalArmedTime + 1 );

		armedBombPlace->_totalArmedTime += frameTime;

		timeLeft = (int)( _timeNeededForBombToExplode - armedBombPlace->_totalArmedTime + 1 );

		// Play computer dialog for time left (if any)

		if ( ( timeLeft <= 1 ) && ( lastTimeLeft > 1 ) && ( _timeNeededForBombToExplode > 1 ) )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_1.mp3", CHAN_AUTO, DEFAULT_VOL, 
					DEFAULT_MIN_DIST, NULL, 0.5f  );
		else if ( ( timeLeft <= 2 ) && ( lastTimeLeft > 2 ) && ( _timeNeededForBombToExplode > 2 ) )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_2.mp3", CHAN_AUTO, DEFAULT_VOL, 
					DEFAULT_MIN_DIST, NULL, 0.5f  );
		else if ( ( timeLeft <= 3 ) && ( lastTimeLeft > 3 ) && ( _timeNeededForBombToExplode > 3 ) )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_3.mp3", CHAN_AUTO, DEFAULT_VOL, 
					DEFAULT_MIN_DIST, NULL, 0.5f  );
		else if ( ( timeLeft <= 4 ) && ( lastTimeLeft > 4 ) && ( _timeNeededForBombToExplode > 4 ) )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_4.mp3", CHAN_AUTO, DEFAULT_VOL, 
					DEFAULT_MIN_DIST, NULL, 0.5f  );
		else if ( ( timeLeft <= 5 ) && ( lastTimeLeft > 5 ) && ( _timeNeededForBombToExplode > 5 ) )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_5.mp3", CHAN_AUTO, DEFAULT_VOL, 
					DEFAULT_MIN_DIST, NULL, 0.5f  );
		else if ( ( timeLeft <= 7 ) && ( lastTimeLeft > 7 ) && ( _timeNeededForBombToExplode > 7 ) )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bombtime.mp3", CHAN_AUTO, DEFAULT_VOL, 
					DEFAULT_MIN_DIST, NULL, 0.5f  );
		else if ( ( timeLeft <= 15 ) && ( lastTimeLeft > 15 ) && ( _timeNeededForBombToExplode > 15 ) )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bombtime15.mp3" );
		else if ( ( timeLeft <= 30 ) && ( lastTimeLeft > 30 ) && ( _timeNeededForBombToExplode > 30 ) )
			multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bombtime30.mp3" );

		if ( armedBombPlace->_totalArmedTime > _timeNeededForBombToExplode )
		{
			str printString;

			// The bomb has exploded

			armedBombPlace->_item->SpawnEffect( "models/fx/fx-explosion-bomb.tik", armedBombPlace->_item->origin, armedBombPlace->_item->angles, 1.0f ); 

			if ( _bombArmedByPlayer >= 0 )
			{
				// Give points to the player that armed the bomb

				multiplayerManager.addPoints( _bombArmedByPlayer, _pointsForExplodingBomb );
			}

			// Give the bombing team a lot of points and tell everyone what happened

			if ( armedBombPlace == &_redBombPlace )
			{
				multiplayerManager.addTeamPoints( "Blue", _teamPointsForBombing );

				multiplayerManager.centerPrintAllClients( "$$RedBaseDestroyed$$", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bombexplodered.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 2.0f );
			}
			else
			{
				multiplayerManager.addTeamPoints( "Red", _teamPointsForBombing );

				multiplayerManager.centerPrintAllClients( "$$BlueBaseDestroyed$$", CENTERPRINT_IMPORTANCE_NORMAL );
				multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_bombexplodeblue.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 2.0f );
			}

			armedBombPlace->_armed = false;

			respawnBomb( true );

			armedBombPlace->_item->removeAttachedModelByTargetname( "attachedDiffusionBomb" );
			_attachedBomb->PostEvent( EV_Remove, 0.0f );
			_attachedBomb = NULL;

			armedBombPlace->_totalArmedTime    = 0.0f;
			armedBombPlace->_lastArmingTime    = 0.0f;
			armedBombPlace->_lastDisarmingTime = 0.0f;

			// Respawn all of the players in a few seconds

			_respawnTime = multiplayerManager.getTime() + 3.0f;
		}
	}

	// See if we should respawn everyone

	if ( ( _respawnTime > 0.0f ) && ( _respawnTime < multiplayerManager.getTime() ) )
	{
		multiplayerManager.respawnAllPlayers();
		_respawnTime = 0.0f;
	}
}

int ModifierDiffusion::getIcon( Player *player, int statNum, int value )
{
	if ( statNum == STAT_MP_MODE_ICON )
	{
		//  Return the icon for the player's specialty

		if ( player == getBomber() )
			return _bomberIconIndex;
		else 
			return -1;
	}

	return value;
}

int ModifierDiffusion::getScoreIcon( Player *player, int index, int value )
{
	if ( index == SCOREICON5 )
	{
		if ( player == getBomber() )
			return _bomberIconIndex;
		else 
			return 0;
	}

	return value;
}

void ModifierDiffusion::matchStarted( void )
{
	if ( _bomb )
	{
		_bomb->showModel();
		_bomb->setSolidType( SOLID_TRIGGER );
	}

	if ( _redBombPlace._item )
	{
		_redBombPlace._item->showModel();
		_redBombPlace._item->setSolidType( SOLID_BBOX );
	}

	if ( _blueBombPlace._item )
	{
		_blueBombPlace._item->showModel();
		_blueBombPlace._item->setSolidType( SOLID_BBOX );
	}
}

// Setup constants

const float	ModifierSpecialties::_infiltratorMinRespawnTime		=  0.0f;
const float	ModifierSpecialties::_medicMinRespawnTime			= 30.0f;
const float	ModifierSpecialties::_technicianMinRespawnTime		= 30.0f;
const float	ModifierSpecialties::_demolitionistMinRespawnTime	= 15.0f;
const float	ModifierSpecialties::_heavyweaponsMinRespawnTime	=  0.0f;
const float	ModifierSpecialties::_sniperMinRespawnTime			=  0.0f;

const float	ModifierSpecialties::_startingInfiltratorHealth	= 100.0f;
const float	ModifierSpecialties::_startingInfiltratorMassModifier = 0.5f;
const float	ModifierSpecialties::_startingMedicArmor = 75.0f;
const float	ModifierSpecialties::_startingTechnicianArmor = 50.0f;
const float	ModifierSpecialties::_startingDemolitionistArmor = 50.0f;
const float	ModifierSpecialties::_startingHeavyWeaponsHealth = 200.0f;
const float	ModifierSpecialties::_startingHeavyWeaponsArmor = 100.0f;
const float	ModifierSpecialties::_startingHeavyWeaponsMassModifier = 2.0f;
const float	ModifierSpecialties::_startingSniperArmor = 25.0f;
const float	ModifierSpecialties::_startingNormalHealth = 100.0f;

const float	ModifierSpecialties::_medicHealOtherRate = 20.0f;
const float	ModifierSpecialties::_medicHealSelfRate = 2.0f;
const float	ModifierSpecialties::_infiltratorMoveSpeedModifier = 1.25f;
const float	ModifierSpecialties::_heavyWeaponsMoveSpeedModifier = .75f;
const float	ModifierSpecialties::_infiltratorJumpSpeedModifier = 1.1f;
const float	ModifierSpecialties::_infiltratorAirAccelerationModifier = 1.5f;

const float	ModifierSpecialties::_technicianHoldableItemRegenTime = 20.0f;
const float	ModifierSpecialties::_demolitionistHoldableItemRegenTime = 20.0f;

const bool ModifierSpecialties::_medicCanSeeEnemyHealth = true;

const float	ModifierSpecialties::_amountOfHealingForPoints = 10.0f;
const int ModifierSpecialties::_pointsForHealing = 1;

const bool ModifierSpecialties::_removeItems = false;

const SpecialtyType SpecialtyPlayerData::_defaultSpecialty = SPECIALTY_INFILTRATOR;

SpecialtyPlayerData::SpecialtyPlayerData() 
{  
	_forced = false; 
	reset(); 
}
void SpecialtyPlayerData::reset( void ) 
{ 
	if ( _forced )
		return;

	_specialty = _defaultSpecialty;
	_item = NULL;
	_lastUseTime = 0.0f;
	_amountOfHealing = 0.0f;
	_lastPlayer = -1;
}

ModifierSpecialties::ModifierSpecialties()
{
	_playerSpecialtyData = NULL;

	_infiltratorIconIndex	= gi.imageindex( "sysimg/icons/mp/specialty_infiltrator" );
	_medicIconIndex			= gi.imageindex( "sysimg/icons/mp/specialty_medic" );
	_technicianIconIndex	= gi.imageindex( "sysimg/icons/mp/specialty_technician" );
	_demolitionistIconIndex	= gi.imageindex( "sysimg/icons/mp/specialty_demolitionist" );
	_heavyweaponsIconIndex	= gi.imageindex( "sysimg/icons/mp/specialty_heavyweapons" );
	_sniperIconIndex		= gi.imageindex( "sysimg/icons/mp/specialty_sniper" );

	_specialtyItems.FreeObjectList();
}

ModifierSpecialties::~ModifierSpecialties()
{
	delete [] _playerSpecialtyData;
	_playerSpecialtyData = NULL;

	_specialtyItems.FreeObjectList();
}

void ModifierSpecialties::init( int maxPlayers )
{
	_maxPlayers = maxPlayers;
	_playerSpecialtyData = new SpecialtyPlayerData[ maxPlayers ];

	multiplayerManager.cacheMultiplayerFiles( "mp_specialties" );
}

bool ModifierSpecialties::checkRule( const char *rule, bool defaultValue, Player *player )
{
	// We want team spawn points

	if ( stricmp( rule, "spawnpoints-team" ) == 0 )
		return true;
	else if ( stricmp( rule, "dropWeapons" ) == 0 )
		return false;
	else if ( stricmp( rule, "spawnpoints-special" ) == 0 )
	{
		// We want special spawnpoints in this mode but not for bots

		if ( player->edict->svflags & SVF_BOT )
			return defaultValue;
		else
			return true;
	}
	else
		return defaultValue;
}

str ModifierSpecialties::getSpawnPointType( Player *player )
{
	Team *team;

	team = multiplayerManager.getPlayersTeam( player );

	if ( team && team->getName() == "Red" )
		return "specialty-red";
	else if ( team && team->getName() == "Blue" )
		return "specialty-blue";
	else
		return "specialty";
}

void ModifierSpecialties::playerCommand( Player *player, const char *command, const char *parm )
{
	if ( stricmp( command, "setSpecialty" ) == 0 )
	{
		SpecialtyType specialty;

		// Figure out which specialty we want to set

		if ( stricmp( parm, "infiltrator" ) == 0 )
			specialty = SPECIALTY_INFILTRATOR;
		else if ( stricmp( parm, "medic" ) == 0 )
			specialty = SPECIALTY_MEDIC;
		else if ( stricmp( parm, "technician" ) == 0 )
			specialty = SPECIALTY_TECHNICIAN;
		else if ( stricmp( parm, "demolitionist" ) == 0 )
			specialty = SPECIALTY_DEMOLITIONIST;
		else if ( ( stricmp( parm, "heavyweapons" ) == 0 ) || ( stricmp( parm, "heavy" ) == 0 ) )
			specialty = SPECIALTY_HEAVY_WEAPONS;
		else if ( stricmp( parm, "sniper" ) == 0 )
			specialty = SPECIALTY_SNIPER;
		else
			specialty = SPECIALTY_NONE;
		
		// Setup the specialty

		if ( ( specialty == SPECIALTY_NONE ) || ( player->edict->svflags & SVF_BOT ) )
		{
			setupSpecialty( player, specialty, true );

			_playerSpecialtyData[ player->entnum ]._specialty = specialty;

			if ( specialty == SPECIALTY_NONE )
				_playerSpecialtyData[ player->entnum ]._forced = false;
			else
				_playerSpecialtyData[ player->entnum ]._forced = true;
		}
	}
}

bool ModifierSpecialties::shouldKeepItem( MultiplayerItem *item )
{
	str itemName;
	const char *dash1;
	const char *dash2;

	itemName = item->getName();

	if ( strnicmp( itemName.c_str(), "specialty-", strlen( "specialty-" ) ) == 0 )
	{
		SpecialtyItem specialtyItem;

		// This is a specialty item so keep track of it

		specialtyItem._item = item;
		specialtyItem._needToRespawn = false;
		specialtyItem._respawnTime = 0.0f;
		specialtyItem._pickedupTime = 0.0f;

		if ( strnicmp( itemName.c_str(), "specialty-infiltrator-", strlen( "specialty-infiltrator-" ) ) == 0 )
		{
			specialtyItem._type = SPECIALTY_INFILTRATOR;
			specialtyItem._minimumRespawnTime = _infiltratorMinRespawnTime;
		}
		else if ( strnicmp( itemName.c_str(), "specialty-medic-", strlen( "specialty-medic-" ) ) == 0 )
		{
			specialtyItem._type = SPECIALTY_MEDIC;
			specialtyItem._minimumRespawnTime = _medicMinRespawnTime;
		}
		else if ( strnicmp( itemName.c_str(), "specialty-technician-", strlen( "specialty-technician-" ) ) == 0 )
		{
			specialtyItem._type = SPECIALTY_TECHNICIAN;
			specialtyItem._minimumRespawnTime = _technicianMinRespawnTime;
		}
		else if ( strnicmp( itemName.c_str(), "specialty-demolitionist-", strlen( "specialty-demolitionist-" ) ) == 0 )
		{
			specialtyItem._type = SPECIALTY_DEMOLITIONIST;
			specialtyItem._minimumRespawnTime = _demolitionistMinRespawnTime;
		}
		else if ( strnicmp( itemName.c_str(), "specialty-heavyweapons-", strlen( "specialty-heavyweapons-" ) ) == 0 )
		{
			specialtyItem._type = SPECIALTY_HEAVY_WEAPONS;
			specialtyItem._minimumRespawnTime = _heavyweaponsMinRespawnTime;
		}
		else if ( strnicmp( itemName.c_str(), "specialty-sniper-", strlen( "specialty-sniper-" ) ) == 0 )
		{
			specialtyItem._type = SPECIALTY_SNIPER;
			specialtyItem._minimumRespawnTime = _sniperMinRespawnTime;
		}
		else
		{
			specialtyItem._type = SPECIALTY_NONE;
		}

		// Get the team name

		dash1 = strstr( itemName.c_str(), "-" );

		if ( dash1 )
		{
			dash2 = strstr( dash1 + 1, "-" );

			if ( dash2 )
			{
				specialtyItem._teamName = dash2 + 1;
			}
		}

		// Add the item to the list

		_specialtyItems.AddObject( specialtyItem );

		return true;
	}
	else
	{
		return false;
	}
}

bool ModifierSpecialties::shouldKeepNormalItem( Item *item )
{
	if ( item->isSubclassOf( Weapon ) )
		return false;
	else
		return true;
}

SpecialtyItem *ModifierSpecialties::findSpecialtyItem( MultiplayerItem *item )
{
	int i;
	SpecialtyItem *specialtyItem;

	// Go through the list and find the item

	for ( i = 1 ; i <= _specialtyItems.NumObjects() ; i++ )
	{
		specialtyItem = &_specialtyItems.ObjectAt( i );

		if ( specialtyItem->_item == item )
		{
			return specialtyItem;
		}
	}

	return NULL;
}

void ModifierSpecialties::addPlayer( Player *player )
{
	putItemBack( player );

	_playerSpecialtyData[ player->entnum ].reset();

	if ( _playerSpecialtyData[ player->entnum ]._forced )
	{
		setupSpecialty( player, _playerSpecialtyData[ player->entnum ]._specialty, false );
	}

	gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_specialties\"\n" );
}

void ModifierSpecialties::itemTouched( Player *player, MultiplayerItem *item )
{
	SpecialtyType specialty;
	bool sameTeam = false;
	Team *team;
	const char *teamName;
	SpecialtyItem *specialtyItem;

	// Make sure player doesn't already have a class

	//if ( _playerSpecialtyData[ player->entnum ]._specialty != SPECIALTY_NONE )
	//	return;

	// Make sure this isn't a bot

	if ( player->edict->svflags & SVF_BOT )
		return;

	// Make sure this is a specialty item

	specialtyItem = findSpecialtyItem( item );

	if ( !specialtyItem )
		return;

	specialty = specialtyItem->_type;

	// Only bother if the player isn't already the new specialty

	if ( _playerSpecialtyData[ player->entnum ]._specialty == specialty )
		return;

	// Make sure the item and the player are on the same team

	team = multiplayerManager.getPlayersTeam( player );

	if ( !team )
		return;

	teamName = team->getName().c_str();

	if ( stricmp( teamName, specialtyItem->_teamName ) == 0 )
		sameTeam = true;

	if ( !sameTeam )
		return;

	if ( !multiplayerManager.checkRule ( "specialty-pickup", true, player ) )
		return;

	if ( _playerSpecialtyData[ player->entnum ]._specialty != SPECIALTY_NONE )
	{
		putItemBack( player );
	}

	// Pickup the item and change the player's specialty

	_playerSpecialtyData[ player->entnum ]._specialty = specialty;
	_playerSpecialtyData[ player->entnum ]._item = item;

	specialtyItem->_pickedupTime = multiplayerManager.getTime();

	setupSpecialty( player, specialty, true );

	removeItem( item );
}

void ModifierSpecialties::setupSpecialty( Player *player, SpecialtyType specialty, bool chosen )
{
	float armorToGive;
	Event *event;
	bool hadTricorder;
	str specialtyName;
	str specialtySoundName;

	armorToGive = 0.0f;

	// Change the player based on the specialty


	// Take away holdable item

	player->removeHoldableItem();

	// Take away all of the weapons except the tricorder

	if ( player->FindItemByModelname( "models/weapons/worldmodel-tricorder.tik" ) )
		hadTricorder = true;
	else
		hadTricorder = false;

	if ( !( player->edict->svflags & SVF_BOT ) )
	{
		player->FreeInventory();
	}

	multiplayerManager.resetPlayerStateMachine( player );

	if ( hadTricorder )
	{
		multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-tricorder.tik" );
	}

	// Take away any armor

	Event *armorEvent = new Event( EV_Sentient_SetMyArmorAmount );
	armorEvent->AddFloat( 0.0f );
	player->ProcessEvent( armorEvent );

	_playerSpecialtyData[ player->entnum ]._regenHoldableItemTime = 0.0f;

	switch( specialty )
	{
		case SPECIALTY_NONE :
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-phaser.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-batleth.tik" );
			multiplayerManager.usePlayerItem( player->entnum, "Phaser" );

			player->setMaxHealth( _startingNormalHealth );

			if ( player->getHealth() > _startingNormalHealth )
				player->setHealth( _startingNormalHealth );

			break;
		case SPECIALTY_INFILTRATOR :

			specialtyName = "$$Infiltrator$$\n";
			specialtySoundName = "localization/sound/dialog/dm/comp_infil.mp3";

			player->setMaxHealth( _startingInfiltratorHealth );

			if ( player->getHealth() > _startingInfiltratorHealth )
				player->setHealth( (int)_startingInfiltratorHealth );

			player->mass *= (int)_startingInfiltratorMassModifier;

			event = new Event( EV_Sentient_AddResistance );
			event->AddString( "falling" );
			event->AddInteger( 100 );
			player->ProcessEvent( event );

			player->setHoldableItem( HoldableItem::createHoldableItem( "Transporter", "models/item/holdable_transporter.tik", player ) );

			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-phaser.tik" );
			//multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-batleth.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-compressionrifle.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "weapons/worldmodel-imod.tik" );
			multiplayerManager.usePlayerItem( player->entnum, "CompressionRifle" );

			player->GiveAmmo( "Plasma", 400, false, 400 );

			break;
		case SPECIALTY_MEDIC :

			specialtyName = "$$Medic$$\n";
			specialtySoundName = "localization/sound/dialog/dm/comp_medic.mp3";

			player->setMaxHealth( _startingNormalHealth );

			if ( player->getHealth() > _startingNormalHealth )
				player->setHealth( _startingNormalHealth );

			armorToGive = _startingMedicArmor;

			//player->setHoldableItem( HoldableItem::createHoldableItem( "Health", "models/item/holdable_medkit.tik", player ) );

			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-phaser.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-tricorder.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-batleth.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-burstrifle.tik" );
			multiplayerManager.usePlayerItem( player->entnum, "BurstRifle" );

			player->GiveAmmo( "Plasma", 400, false, 400 );

			break;
		case SPECIALTY_TECHNICIAN :

			specialtyName = "$$Technician$$\n";
			specialtySoundName = "localization/sound/dialog/dm/comp_tech.mp3";

			player->setMaxHealth( _startingNormalHealth );

			if ( player->getHealth() > _startingNormalHealth )
				player->setHealth( _startingNormalHealth );

			armorToGive = _startingTechnicianArmor;

			//event = new Event( EV_Sentient_AddResistance );
			//event->AddString( "drown" );
			//event->AddInteger( 100 );
			//player->ProcessEvent( event );

			//player->setHoldableItem( HoldableItem::createHoldableItem( "Protection", "models/item/holdable_forcefield.tik", player ) );

			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-phaser.tik" );
			//multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-batleth.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-attrex-rifle.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-rom-disruptor.tik" );
			multiplayerManager.usePlayerItem( player->entnum, "AttrexianRifle" );

			_playerSpecialtyData[ player->entnum ]._regenHoldableItemTime = multiplayerManager.getTime() + _technicianHoldableItemRegenTime;

			player->GiveAmmo( "Idryll", 400, false, 400 );

			break;
		case SPECIALTY_DEMOLITIONIST :

			specialtyName = "$$Demolitionist$$\n";
			specialtySoundName = "localization/sound/dialog/dm/comp_demoli.mp3";

			player->setMaxHealth( _startingNormalHealth );

			if ( player->getHealth() > _startingNormalHealth )
				player->setHealth( _startingNormalHealth );

			armorToGive = _startingDemolitionistArmor;

			player->setHoldableItem( HoldableItem::createHoldableItem( "Explosive", "models/item/holdable_explosive.tik", player ) );

			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-phaser.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-batleth.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-fieldassaultrifle.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-grenadelauncher.tik" );
			multiplayerManager.usePlayerItem( player->entnum, "GrenadeLauncher" );

			player->GiveAmmo( "Fed", 400, false, 400 );

			break;
		case SPECIALTY_HEAVY_WEAPONS :

			specialtyName = "$$HeavyWeapons$$\n";
			specialtySoundName = "localization/sound/dialog/dm/comp_hw.mp3";

			player->setMaxHealth( _startingHeavyWeaponsHealth );

			if ( player->getHealth() > _startingHeavyWeaponsHealth )
				player->setHealth( _startingHeavyWeaponsHealth );

			armorToGive = (int) _startingHeavyWeaponsArmor;
			player->mass *= (int)_startingHeavyWeaponsMassModifier;

			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-phaser.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-batleth.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-tetryon.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-photon.tik" );
			multiplayerManager.usePlayerItem( player->entnum, "PhotonBurst" );

			player->GiveAmmo( "Fed", 400, false, 400 );
			player->GiveAmmo( "Plasma", 100, false, 400 );

			break;
		case SPECIALTY_SNIPER :

			specialtyName = "$$Sniper$$\n";
			specialtySoundName = "localization/sound/dialog/dm/comp_sniper.mp3";

			player->setMaxHealth( _startingNormalHealth );

			if ( player->getHealth() > _startingNormalHealth )
				player->setHealth( _startingNormalHealth );

			armorToGive = _startingSniperArmor;

			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-phaser.tik" );
			//multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-batleth.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-sniperrifle.tik" );
			multiplayerManager.givePlayerItem( player->entnum, "models/weapons/worldmodel-compressionrifle.tik" );
			multiplayerManager.usePlayerItem( player->entnum, "FederationSniperRifle" );

			player->GiveAmmo( "Fed", 400, false, 400 );

			// TODO decoy ?
			//player->giveItem( "models/item/decoy.tik", 1, false );
			break;
	}

	// Give the player some armor

	if ( armorToGive > 0.0f )
	{
		Event *armorEvent;

		armorEvent = new Event( EV_Sentient_GiveArmor );
		armorEvent->AddString( "BasicArmor" );
		armorEvent->AddFloat( armorToGive );
		armorEvent->AddInteger( false );

		player->ProcessEvent( armorEvent );	
	}

	// Tell the player his specialty

	if ( !multiplayerManager.isPlayerSpectator( player ) )
	{
		if ( specialtyName.length() > 0 )
		{
			// Tell player his new specialty

			multiplayerManager.centerPrint( player->entnum, specialtyName.c_str(), CENTERPRINT_IMPORTANCE_HIGH );

			// Tell teammates his new specialty

			if ( chosen )
			{
				str stringToPrint;

				stringToPrint = player->client->pers.netname;
				stringToPrint += " $$BecomesA$$ ";
				stringToPrint += specialtyName;

				multiplayerManager.HUDPrintTeamClients( player, stringToPrint.c_str() );
			}
		}

		if ( ( specialtySoundName.length() > 0 ) && ( chosen ) )
		{
			multiplayerManager.playerSound( player->entnum, specialtySoundName, CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.0f );
		}
	}

	attachBackpack( player );
}

void ModifierSpecialties::playerChangedModel( Player *player )
{
	attachBackpack( player );
}

void ModifierSpecialties::attachBackpack( Player *player )
{
	str attachModelName;
	str tagName;

	player->removeAttachedModelByTargetname( "attachedSpecialityBackpack" );

	switch( _playerSpecialtyData[ player->entnum ]._specialty )
	{
		case SPECIALTY_INFILTRATOR :
			attachModelName = "models/item/mp_specialty_infiltrator.tik";
			break;
		case SPECIALTY_MEDIC :
			attachModelName = "models/item/mp_specialty_medic.tik";
			break;
		case SPECIALTY_TECHNICIAN :
			attachModelName = "models/item/mp_specialty_technician.tik";
			break;
		case SPECIALTY_DEMOLITIONIST :
			attachModelName = "models/item/mp_specialty_demolitionist.tik";
			break;
		case SPECIALTY_HEAVY_WEAPONS :
			attachModelName = "models/item/mp_specialty_heavyweapons.tik";
			break;
		case SPECIALTY_SNIPER :
			attachModelName = "models/item/mp_specialty_sniper.tik";
			break;
	}

	if ( gi.Tag_NumForName( player->edict->s.modelindex, "Bip01 spine2" ) >= 0 )
		tagName = "Bip01 spine2";
	else if ( gi.Tag_NumForName( player->edict->s.modelindex, "Bip01 spine1" ) >= 0 )
		tagName = "Bip01 spine1";
	else if ( gi.Tag_NumForName( player->edict->s.modelindex, "Bip01 Head" ) >= 0 )
		tagName = "Bip01 Head";

	if ( attachModelName.length() )
	{
		Event *attachEvent;

		attachEvent = new Event( EV_AttachModel );
		attachEvent->AddString( attachModelName.c_str() );
		attachEvent->AddString( tagName );
		attachEvent->AddFloat( 1.0f );
		attachEvent->AddString( "attachedSpecialityBackpack" );
		attachEvent->AddInteger( 0 );
		attachEvent->AddFloat( -1.0f );
		attachEvent->AddFloat( 0.0f );
		attachEvent->AddFloat( -1.0f );
		attachEvent->AddFloat( -1.0f );
		attachEvent->AddVector( player->getBackpackAttachOffset() );
		attachEvent->AddVector( player->getBackpackAttachAngles() );

		player->ProcessEvent( attachEvent );	
	}
}

void ModifierSpecialties::playerUsed( Player *usedPlayer, Player *usingPlayer, Equipment *equipment )
{
	// Make sure everything is ok

	if ( !usedPlayer || !usingPlayer || !equipment )
		return;

	// Give health to the player if everything is ok and this is a medic

	if ( ( _playerSpecialtyData[ usingPlayer->entnum ]._specialty == SPECIALTY_MEDIC ) &&
		 ( _playerSpecialtyData[ usingPlayer->entnum ]._lastUseTime < multiplayerManager.getTime() ) &&
		 ( multiplayerManager.getPlayersTeam( usedPlayer ) == multiplayerManager.getPlayersTeam( usingPlayer ) ) &&
		 ( stricmp( equipment->getTypeName().c_str(), "tricorder" ) == 0 ) &&
		 ( usedPlayer != usingPlayer ) )
	{
		float healthToAdd;
		float originalHealth;
		float healthAdded;


		_playerSpecialtyData[ usingPlayer->entnum ]._lastUseTime = multiplayerManager.getTime();

		healthToAdd = _medicHealOtherRate * level.frametime;

		originalHealth = usedPlayer->getHealth();

		usedPlayer->addHealth( healthToAdd, 100.0f );

		healthAdded = usedPlayer->getHealth() - originalHealth;

		// Give the healing player some points if necessary

		_playerSpecialtyData[ usingPlayer->entnum ]._amountOfHealing += healthAdded;

		while ( _playerSpecialtyData[ usingPlayer->entnum ]._amountOfHealing >= _amountOfHealingForPoints )
		{
			_playerSpecialtyData[ usingPlayer->entnum ]._amountOfHealing -= _amountOfHealingForPoints; 
			multiplayerManager.addPoints( usingPlayer->entnum, _pointsForHealing );
		}
	}
	/* else if ( ( _playerSpecialtyData[ usingPlayer->entnum ]._specialty == SPECIALTY_TECHNICIAN ) &&
		 ( multiplayerManager.getPlayersTeam( usedPlayer ) == multiplayerManager.getPlayersTeam( usingPlayer ) ) )
	{
		if ( _playerSpecialtyData[ usingPlayer->entnum ]._lastUseTime == 0.0f )
		{
			Powerup *powerup;

			_playerSpecialtyData[ usingPlayer->entnum ]._lastUseTime = multiplayerManager.getTime();

			powerup = Powerup::CreatePowerup( "Invisibility", "models/item/powerup_invisibility.tik", usedPlayer );

			if ( powerup )
			{
				powerup->CancelEventsOfType( EV_ProcessInitCommands );
				powerup->ProcessInitCommands( powerup->edict->s.modelindex );

				usedPlayer->setPowerup( powerup );
			}
		}
		else
		{
			usingPlayer->Sound( "snd_noammo" );
		}
	} */
}

void ModifierSpecialties::update( float frameTime )
{
	Player      *player;
	Entity      *entity;
	gentity_t   *edict;
	int i;
	SpecialtyItem *specialtyItem;
	HoldableItem *holdableItem;

	// Update all of the players based on their specialty

	for( i = 0 ; i < game.maxclients ; i++ )
	{
		edict = &g_entities[ i ];

		if ( !edict->inuse || !edict->entity )
			continue;

  		entity = edict->entity;

		if ( !entity->isSubclassOf( Player ) )
			continue;

		player = ( Player * )entity;

		if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_MEDIC )
		{
			player->addHealth( _medicHealSelfRate * frameTime );
		}
		else if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_TECHNICIAN )
		{
			Event *newEvent = new Event( EV_Sentient_SetViewMode );
			newEvent->AddString( "forcevisible" );
			newEvent->AddInteger( 0 );
			player->ProcessEvent( newEvent );
		}

		// Respawn holdable items

		holdableItem = player->getHoldableItem();

		if ( !holdableItem )
		{
			if ( _playerSpecialtyData[ player->entnum ]._regenHoldableItemTime > 0.0f )
			{
				if ( _playerSpecialtyData[ player->entnum ]._regenHoldableItemTime < multiplayerManager.getTime() )
				{
					// Respawn holdable items

					if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_DEMOLITIONIST )
					{
						player->setHoldableItem( HoldableItem::createHoldableItem( "Explosive", "models/item/holdable_explosive.tik", player ) );
					}
					else if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_TECHNICIAN )
					{
						float randomNum;

						randomNum = G_Random( 1 );

						if ( randomNum < 0.25f )
							player->setHoldableItem( HoldableItem::createHoldableItem( "SpawnPowerup", "models/item/holdable_spawnInvis.tik", player ) );
						else if ( randomNum < 0.50f )
							player->setHoldableItem( HoldableItem::createHoldableItem( "SpawnPowerup", "models/item/holdable_spawnRegen.tik", player ) );
						else if ( randomNum < 0.75f )
							player->setHoldableItem( HoldableItem::createHoldableItem( "SpawnPowerup", "models/item/holdable_spawnSpeed.tik", player ) );
						else
							player->setHoldableItem( HoldableItem::createHoldableItem( "SpawnPowerup", "models/item/holdable_spawnStrength.tik", player ) );
					}

					_playerSpecialtyData[ player->entnum ]._regenHoldableItemTime = 0.0f;

					player->Sound( "snd_holdableRespawn" );
				}
			}
			else
			{
				// The player has no holdable item and no timer is setup

				if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_DEMOLITIONIST )
				{
					_playerSpecialtyData[ player->entnum ]._regenHoldableItemTime = multiplayerManager.getTime() +
							_demolitionistHoldableItemRegenTime;
				}
				else if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_TECHNICIAN )
				{
					_playerSpecialtyData[ player->entnum ]._regenHoldableItemTime = multiplayerManager.getTime() +
							_technicianHoldableItemRegenTime;
				}
			}
		}
	}

	// Respawn any specialty items that need to be

	for ( i = 1 ; i <= _specialtyItems.NumObjects() ; i++ )
	{
		specialtyItem = &_specialtyItems.ObjectAt( i );

		if ( specialtyItem->_needToRespawn && ( specialtyItem->_respawnTime < multiplayerManager.getTime() ) )
		{
			specialtyItem->_needToRespawn = false;
			specialtyItem->_item->showModel();
			specialtyItem->_item->setSolidType( SOLID_TRIGGER );
		}
	}
}

void ModifierSpecialties::removePlayer( Player *player )
{
	int i;

	putItemBack( player );

	// Wipe out any saved data for the player

	for ( i = 0 ; i < _maxPlayers ; i++ )
	{
		_playerSpecialtyData[ player->entnum ]._lastPlayer = -1;
	}
}

void ModifierSpecialties::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	putItemBack( killedPlayer );
}

void ModifierSpecialties::playerSpawned( Player *player )
{
	if ( _playerSpecialtyData[ player->entnum ]._forced )
	{
		setupSpecialty( player, _playerSpecialtyData[ player->entnum ]._specialty, false );
	}
	else
	{
		putItemBack( player );

		_playerSpecialtyData[ player->entnum ].reset();

		setupSpecialty( player, _playerSpecialtyData[ player->entnum ]._specialty, false );
	}
}

void ModifierSpecialties::applySpeedModifiers( Player *player, int *moveSpeed )
{
	if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_INFILTRATOR )
		*moveSpeed *= (int) (_infiltratorMoveSpeedModifier);
	else if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_HEAVY_WEAPONS )
		*moveSpeed = (int)( *moveSpeed * _heavyWeaponsMoveSpeedModifier );
}

void ModifierSpecialties::applyJumpModifiers( Player *player, int *jumpSpeed )
{
	if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_INFILTRATOR )
		*jumpSpeed = (int)( *jumpSpeed * _infiltratorJumpSpeedModifier );
}

void ModifierSpecialties::applyAirAccelerationModifiers( Player *player, int *airAcceleration )
{
	if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_INFILTRATOR )
		*airAcceleration = (int)( *airAcceleration * _infiltratorAirAccelerationModifier );
}

bool ModifierSpecialties::canPickup( Player *player, MultiplayerItemType itemType, const char *item_name )
{
	// The infiltrator can't pickup the speed powerup

	if ( itemType == MP_ITEM_TYPE_ARMOR )
	{
		if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_INFILTRATOR )
		{
			// Infiltrator can't pick up armor

			return false;
		}
	}
	else if ( itemType == MP_ITEM_TYPE_POWERUP )
	{
		/* if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_INFILTRATOR )
		{
			if ( stricmp( item_name, "Speed" ) == 0 )
			{
				return false;
			}
		}

		// No one can pickup the strength powerup except the heavy weapons class

		if ( stricmp( item_name, "Strength" ) == 0 )
		{
			if ( _playerSpecialtyData[ player->entnum ]._specialty != SPECIALTY_HEAVY_WEAPONS )
			{
				return false;
			}
		} */
	}
	else if ( itemType == MP_ITEM_TYPE_WEAPON )
	{
		// Specialities can't pickup any weapons (they have already been given the weapons their allowed to have)

		if ( _playerSpecialtyData[ player->entnum ]._specialty != SPECIALTY_NONE )
			return false;
	}

	return true;
}

void ModifierSpecialties::removeItem( MultiplayerItem *item )
{
	if ( !_removeItems )
		return;

	if ( item )
	{
		item->setSolidType( SOLID_NOT );
		item->hideModel();
	}
}

void ModifierSpecialties::putItemBack( Player *player )
{
	MultiplayerItem *item;
	SpecialtyItem *specialtyItem;

	if ( _playerSpecialtyData[ player->entnum ]._forced )
		return;

	if ( _removeItems )
	{
		// Find the item in question

		item = _playerSpecialtyData[ player->entnum ]._item;

		specialtyItem = findSpecialtyItem( item );

		// Put the item back

		if ( specialtyItem )
		{
			HoldableItem* holdableItem;
			bool respawnNow = false;

			holdableItem = player->getHoldableItem();

			if ( holdableItem )
			{
				if ( specialtyItem->_type == SPECIALTY_DEMOLITIONIST )
				{
					if ( holdableItem->getName() == "Explosive" )
						respawnNow = true;
				}
				else if ( specialtyItem->_type == SPECIALTY_MEDIC )
				{
					if ( ( holdableItem->getName() == "Health" ) && ( _playerSpecialtyData[ player->entnum ]._lastUseTime == 0.0f ) )
						respawnNow = true;
				}
				/* else if ( specialtyItem->_type == SPECIALTY_TECHNICIAN )
				{
					if ( ( holdableItem->getName() == "Protection" ) && ( _playerSpecialtyData[ player->entnum ]._lastUseTime == 0.0f ) )
						respawnNow = true;
				} */
			}

			if ( respawnNow )
				specialtyItem->_respawnTime = specialtyItem->_pickedupTime + 0.0f;
			else
				specialtyItem->_respawnTime = specialtyItem->_pickedupTime + specialtyItem->_minimumRespawnTime;

			specialtyItem->_needToRespawn = true;
		}
	}

	_playerSpecialtyData[ player->entnum ].reset();
}

int ModifierSpecialties::getStat( Player *player, int statNum, int value )
{
	Entity *target;
	Player *targetPlayer;
	int lastPlayer;
	float lastPlayerTime;

	if ( statNum == STAT_MP_GENERIC4 )
	{
		if ( _playerSpecialtyData[ player->entnum ]._specialty == SPECIALTY_MEDIC )
		{
			lastPlayer     = _playerSpecialtyData[ player->entnum ]._lastPlayer;
			lastPlayerTime = _playerSpecialtyData[ player->entnum ]._lastPlayerTime;

			target = player->GetTargetedEntity();

			if ( target && target->isSubclassOf( Player ) && 
				 ( ( multiplayerManager.getPlayersTeam( player ) == multiplayerManager.getPlayersTeam( (Player*)target ) ) ||
				   ( _medicCanSeeEnemyHealth ) ) )
			{
				_playerSpecialtyData[ player->entnum ]._lastPlayer     = target->entnum;
				_playerSpecialtyData[ player->entnum ]._lastPlayerTime = multiplayerManager.getTime();

				return (int)( target->getHealth() + 0.99f );
			}
			else if ( ( lastPlayer >= 0 ) && ( lastPlayerTime + 1.0f > multiplayerManager.getTime() ) )
			{
				targetPlayer = multiplayerManager.getPlayer( lastPlayer );

				if ( targetPlayer )
				{
					return (int)( targetPlayer->getHealth() + 0.99f );
				}
			}
			else
			{
				_playerSpecialtyData[ player->entnum ]._lastPlayer = -1;
			}
		}
	}

	return value;
}

int ModifierSpecialties::getIcon( Player *player, int statNum, int value )
{
	if ( statNum == STAT_MP_SPECIALTY_ICON )
	{
		// Return the icon for the player's specialty

		switch( _playerSpecialtyData[ player->entnum ]._specialty )
		{
			case SPECIALTY_NONE :
				return value;
			case SPECIALTY_INFILTRATOR :
				return _infiltratorIconIndex;
			case SPECIALTY_MEDIC :
				return _medicIconIndex;
			case SPECIALTY_TECHNICIAN :
				return _technicianIconIndex;
			case SPECIALTY_DEMOLITIONIST :
				return _demolitionistIconIndex;
			case SPECIALTY_HEAVY_WEAPONS :
				return _heavyweaponsIconIndex;
			case SPECIALTY_SNIPER :
				return _sniperIconIndex;
		}
	
	}

	return value;
}

int ModifierSpecialties::getScoreIcon( Player *player, int index, int value )
{
	if ( index == SCOREICON4 )
	{
		// Return the icon for the player's specialty

		switch( _playerSpecialtyData[ player->entnum ]._specialty )
		{
			case SPECIALTY_NONE :
				return value;
			case SPECIALTY_INFILTRATOR :
				return _infiltratorIconIndex;
			case SPECIALTY_MEDIC :
				return _medicIconIndex;
			case SPECIALTY_TECHNICIAN :
				return _technicianIconIndex;
			case SPECIALTY_DEMOLITIONIST :
				return _demolitionistIconIndex;
			case SPECIALTY_HEAVY_WEAPONS :
				return _heavyweaponsIconIndex;
			case SPECIALTY_SNIPER :
				return _sniperIconIndex;
		}
	}

	return value;
}

// Setup constants

const float ModifierControlPoints::_timeNeededToControl = 5.0f;

const int ModifierControlPoints::_maxNumberAwardsForPlayerCapture	= 5;
const int ModifierControlPoints::_pointsForEachAwardForCapture		= 5;
const int ModifierControlPoints::_pointsForProtectingControlPoint	= 5;
const int ModifierControlPoints::_maxGuardingDist					= 750;

ModifierControlPoints::ModifierControlPoints()
{
	_alphaControlPointRedControlledIndex     = gi.imageindex( "sysimg/icons/mp/cp_alpha_red" );
	_alphaControlPointBlueControlledIndex    = gi.imageindex( "sysimg/icons/mp/cp_alpha_blue" );
	_alphaControlPointNeutralControlledIndex = gi.imageindex( "sysimg/icons/mp/cp_alpha_neutral" );

	_betaControlPointRedControlledIndex     = gi.imageindex( "sysimg/icons/mp/cp_beta_red" );
	_betaControlPointBlueControlledIndex    = gi.imageindex( "sysimg/icons/mp/cp_beta_blue" );
	_betaControlPointNeutralControlledIndex = gi.imageindex( "sysimg/icons/mp/cp_beta_neutral" );

	_deltaControlPointRedControlledIndex     = gi.imageindex( "sysimg/icons/mp/cp_delta_red" );
	_deltaControlPointBlueControlledIndex    = gi.imageindex( "sysimg/icons/mp/cp_delta_blue" );
	_deltaControlPointNeutralControlledIndex = gi.imageindex( "sysimg/icons/mp/cp_delta_neutral" );

	_gammaControlPointRedControlledIndex     = gi.imageindex( "sysimg/icons/mp/cp_gamma_red" );
	_gammaControlPointBlueControlledIndex    = gi.imageindex( "sysimg/icons/mp/cp_gamma_blue" );
	_gammaControlPointNeutralControlledIndex = gi.imageindex( "sysimg/icons/mp/cp_gamma_neutral" );
}

ModifierControlPoints::~ModifierControlPoints()
{
	_controlPoints.FreeObjectList();
}

void ModifierControlPoints::init( int maxPlayers )
{
	multiplayerManager.cacheMultiplayerFiles( "mp_controlPoints" );
}

void ModifierControlPoints::addPlayer( Player *player )
{
	gi.SendServerCommand( player->entnum, "stufftext \"ui_addhud mp_control\"\n" );
}

str ModifierControlPoints::getSpawnPointType( Player *player )
{
	return "control";
}

bool ModifierControlPoints::checkRule( const char *rule, bool defaultValue, Player *player )
{
	if ( stricmp( rule, "spawnpoints-special" ) == 0 )
		return true;
	else
		return defaultValue;
}

/* int ModifierControlPoints::getStat( Player *player, int statNum, int value )
{
	int redControlPoints;
	int blueControlPoints;
	int i;
	ControlPointData *controlPoint;

	if ( statNum == STAT_MP_GENERIC1 )
	{
		// Return the number of control points controlled by the red team

		redControlPoints = 0;

		for ( i = 1 ; i <= _controlPoints.NumObjects() ; i++ )
		{
			controlPoint = _controlPoints.AddressOfObjectAt( i );

			if ( controlPoint && controlPoint->_controllingTeam && controlPoint->_controllingTeam->getName() == "Red" )
			{
				redControlPoints++;
			}
		}
		
		return redControlPoints;
	}
	else if ( statNum == STAT_MP_GENERIC2 )
	{
		// Return the number of control points controlled by the blue team

		blueControlPoints = 0;

		for ( i = 1 ; i <= _controlPoints.NumObjects() ; i++ )
		{
			controlPoint = _controlPoints.AddressOfObjectAt( i );

			if ( controlPoint && controlPoint->_controllingTeam && controlPoint->_controllingTeam->getName() == "Blue" )
			{
				blueControlPoints++;
			}
		}
		
		return blueControlPoints;
	}
	
	return value;
} */

bool ModifierControlPoints::shouldKeepItem( MultiplayerItem *item )
{
	ControlPointData controlPoint;

	if ( strnicmp( item->getName().c_str(), "controlpoint-", strlen( "controlpoint-" ) ) == 0 )
	//if ( strnicmp( item->getName().c_str(), "controlpoint", strlen( "controlpoint" ) ) == 0 )
	{
		// This is a control point, add it to the list

		controlPoint._controlPoint = item;
		controlPoint._controllingTeam = NULL;
		controlPoint._lastControllingTeam = NULL;
		controlPoint._lastTime = 0.0f;

		controlPoint._controlPointType = getControlPointType( item->getName() );
		//controlPoint._controlPointType = (ControlPointType)( _controlPoints.NumObjects() + 1);

		_controlPoints.AddObject( controlPoint );

		return true;
	}
	else
		return false;
}

ControlPointType ModifierControlPoints::getControlPointType( const str &name )
{
	if ( name == "controlpoint-alpha" )
		return CONTROL_POINT_ALPHA;
	else if ( name == "controlpoint-beta" )
		return CONTROL_POINT_BETA;
	else if ( name == "controlpoint-delta" )
		return CONTROL_POINT_DELTA;
	else if ( name == "controlpoint-gamma" )
		return CONTROL_POINT_GAMMA;
	else 
		return CONTROL_POINT_NONE;
}

void ModifierControlPoints::itemTouched( Player *player, MultiplayerItem *item )
{
	int i;
	ControlPointData *controlPoint;
	Team *team;
	str animName;
	str printString;

	// Find the control point

	for ( i = 1 ; i <= _controlPoints.NumObjects() ; i++ )
	{
		controlPoint = _controlPoints.AddressOfObjectAt( i );

		if ( controlPoint->_controlPoint == item )
		{
			// This is the control point that was touched

			team = multiplayerManager.getPlayersTeam( player );

			// Transfer control if a different team

			if ( team != controlPoint->_controllingTeam )
			{
				controlPoint->_controllingTeam = team;
				controlPoint->_lastTime = multiplayerManager.getTime();
				// TODO, clear this if the player quits
				controlPoint->_lastPlayerToTouch = player->entnum;
				controlPoint->_playerPointsAwardedForCapture = 0;

				// Change the control point based on who controls it

				animName = "control_";
				animName += controlPoint->_controllingTeam->getName();

				controlPoint->_controlPoint->animate->RandomAnimate( animName );

				multiplayerManager.playerSound( player->entnum, "localization/sound/dialog/dm/comp_ctrltaken.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.2f );
			}
		}
	}
}

void ModifierControlPoints::update( float frameTime )
{
	int i;
	ControlPointData *controlPoint;

	// Loop through all control points

	for ( i = 1 ; i <= _controlPoints.NumObjects() ; i++ )
	{
		controlPoint = _controlPoints.AddressOfObjectAt( i );

		// See if this control point is controlled by a team

		if ( controlPoint->_controllingTeam )
		{
			if ( multiplayerManager.getTime() > controlPoint->_lastTime + _timeNeededToControl )
			{
				// The team has controlled it for long enough

				controlPoint->_controllingTeam->addPoints( NULL, 1 );

				controlPoint->_lastTime = multiplayerManager.getTime();

				if ( controlPoint->_playerPointsAwardedForCapture < _maxNumberAwardsForPlayerCapture )
				{
					if ( controlPoint->_playerPointsAwardedForCapture == 0 )
					{
						if ( controlPoint->_controllingTeam && ( controlPoint->_lastControllingTeam != controlPoint->_controllingTeam ) )
						{
							str soundName;

							if ( controlPoint->_controllingTeam->getName() == "Red" )
							{
								if ( controlPoint->_controlPointType == CONTROL_POINT_ALPHA )
									soundName = "localization/sound/dialog/dm/comp_ctrlredalpha.mp3";
								else if ( controlPoint->_controlPointType == CONTROL_POINT_BETA )
									soundName = "localization/sound/dialog/dm/comp_ctrlredbeta.mp3";
								else if ( controlPoint->_controlPointType == CONTROL_POINT_DELTA )
									soundName = "localization/sound/dialog/dm/comp_ctrlreddelta.mp3";
								else if ( controlPoint->_controlPointType == CONTROL_POINT_GAMMA )
									soundName = "localization/sound/dialog/dm/comp_ctrlredgamma.mp3";
							}
							else
							{
								if ( controlPoint->_controlPointType == CONTROL_POINT_ALPHA )
									soundName = "localization/sound/dialog/dm/comp_ctrlbluealpha.mp3";
								else if ( controlPoint->_controlPointType == CONTROL_POINT_BETA )
									soundName = "localization/sound/dialog/dm/comp_ctrlbluebeta.mp3";
								else if ( controlPoint->_controlPointType == CONTROL_POINT_DELTA )
									soundName = "localization/sound/dialog/dm/comp_ctrlbluedelta.mp3";
								else if ( controlPoint->_controlPointType == CONTROL_POINT_GAMMA )
									soundName = "localization/sound/dialog/dm/comp_ctrlbluegamma.mp3";
							}

							multiplayerManager.broadcastSound( soundName, CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.5f );
						}
					}

					controlPoint->_lastControllingTeam = controlPoint->_controllingTeam;

					controlPoint->_playerPointsAwardedForCapture++;

					if ( controlPoint->_lastPlayerToTouch >= 0 )
					{
						multiplayerManager.addPoints( controlPoint->_lastPlayerToTouch, _pointsForEachAwardForCapture );
					}
				}
			}
		}
	}
}

void ModifierControlPoints::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	Team *victimsTeam;
	Team *killersTeam;
	float distance;
	str printString;
	bool controlPointGuarded;

	if ( !attackingPlayer || ( killedPlayer == attackingPlayer ) )
		return;
	
	victimsTeam = multiplayerManager.getPlayersTeam( killedPlayer );
	killersTeam = multiplayerManager.getPlayersTeam( attackingPlayer );

	if ( victimsTeam && killersTeam && ( victimsTeam == killersTeam ) )
		return;

	// See if the player was guarding a control point (either he or the killed player was close to his flag)

	controlPointGuarded = false;

	distance = findNearestControlledControlPoint( killersTeam->getName(), attackingPlayer->origin );

	if ( ( distance > 0 ) && ( distance < _maxGuardingDist ) )
	{
		controlPointGuarded = true;
	}
	else
	{
		distance = findNearestControlledControlPoint( killersTeam->getName(), killedPlayer->origin );

		if ( ( distance > 0 ) && ( distance < _maxGuardingDist ) )
		{
			controlPointGuarded = true;
		}
	}

	if ( controlPointGuarded )
	{
		//multiplayerManager.playerEventNotification( "controlpoint-guarded", "", attackingPlayer );
		
		printString = "$$ControlPointGuarded$$ ";
		printString += attackingPlayer->client->pers.netname;

		multiplayerManager.playerSound( attackingPlayer->entnum, "localization/sound/dialog/dm/comp_ctrlguarded.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, 1.5f );

		multiplayerManager.centerPrintTeamClients( attackingPlayer, printString, CENTERPRINT_IMPORTANCE_HIGH );

		// Give points to the player

		multiplayerManager.addPoints( attackingPlayer->entnum, _pointsForProtectingControlPoint );
	}
}

float ModifierControlPoints::findNearestControlledControlPoint( const str &teamName, const Vector &position )
{
	int i;
	float nearestDistance = -1.0f;
	Vector diff;
	float distance;
	ControlPointData *controlPoint;
	ControlPointData *nearestControlledControlPoint = NULL;

	// Loop through all of the control points

	for ( i = 1 ; i <= _controlPoints.NumObjects() ; i++ )
	{
		controlPoint = _controlPoints.AddressOfObjectAt( i );

		// See if this control point is controlled by the specified team

		if ( controlPoint->_controllingTeam && ( controlPoint->_controllingTeam->getName() == teamName ) && 
			 ( controlPoint->_playerPointsAwardedForCapture > 0 ) )
		{
			// Get the distance from the control point to the specified point

			diff = position - controlPoint->_controlPoint->origin;
			distance = diff.length();

			// See if this is the nearest controlled control point

			if ( !nearestControlledControlPoint || ( distance < nearestDistance ) )
			{
				nearestControlledControlPoint = controlPoint;
				nearestDistance = distance;
			}
		}
	}

	return nearestDistance;
}

int ModifierControlPoints::getStat( Player *player, int statNum, int value )
{
	int redControlledIndex;
	int blueControlledIndex;
	int	neutralControlledIndex;
	ControlPointData *controlPoint;
	int i;
	ControlPointType controlPointType;


	// Figure out which control point we are referring to (if any) and the set of icons to use

	if ( ( statNum == STAT_MP_GENERIC5 ) )
	{
		controlPointType = CONTROL_POINT_ALPHA;
		redControlledIndex     = _alphaControlPointRedControlledIndex;
		blueControlledIndex    = _alphaControlPointBlueControlledIndex;
		neutralControlledIndex = _alphaControlPointNeutralControlledIndex;
	}
	else if ( ( statNum == STAT_MP_GENERIC6 ) )
	{
		controlPointType = CONTROL_POINT_BETA;
		redControlledIndex     = _betaControlPointRedControlledIndex;
		blueControlledIndex    = _betaControlPointBlueControlledIndex;
		neutralControlledIndex = _betaControlPointNeutralControlledIndex;
	}
	else if ( ( statNum == STAT_MP_GENERIC7 ) )
	{
		controlPointType = CONTROL_POINT_DELTA;
		redControlledIndex     = _deltaControlPointRedControlledIndex;
		blueControlledIndex    = _deltaControlPointBlueControlledIndex;
		neutralControlledIndex = _deltaControlPointNeutralControlledIndex;
	}
	else if ( ( statNum == STAT_MP_GENERIC8 ) )
	{
		controlPointType = CONTROL_POINT_GAMMA;
		redControlledIndex     = _gammaControlPointRedControlledIndex;
		blueControlledIndex    = _gammaControlPointBlueControlledIndex;
		neutralControlledIndex = _gammaControlPointNeutralControlledIndex;
	}
	else
	{
		return value;
	}

	// Find this control point

	for ( i = 1 ; i <= _controlPoints.NumObjects() ; i++ )
	{
		controlPoint = _controlPoints.AddressOfObjectAt( i );

		if ( controlPoint->_controlPointType == controlPointType )
		{
			// Return the correct index based on which team controls this control point

			if ( !controlPoint->_controllingTeam )
			{
				return neutralControlledIndex;
			}
			else if ( controlPoint->_controllingTeam->getName() == "Red" )
			{
				return redControlledIndex;
			}
			else if ( controlPoint->_controllingTeam->getName() == "Blue" )
			{
				return blueControlledIndex;
			}		
		}
	}

	return value;
}

/* int ModifierControlPoints::getPointsForKill( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath, int points )
{
	// Players do not get points for kills in control points mode
	return 0;
} */

// Setup constants

const float	ModifierAutoHandicap::_attackerHandicapModifier = 1.05f;
const float	ModifierAutoHandicap::_victimHandicapModifier = 1.05f;

ModifierAutoHandicap::ModifierAutoHandicap()
{
	_playerHandicapData = NULL;
}
					
ModifierAutoHandicap::~ModifierAutoHandicap()
{
	delete [] _playerHandicapData;
	_playerHandicapData = NULL;
}

void ModifierAutoHandicap::init( int maxPlayers )
{
	_playerHandicapData = new HandicapPlayerData[ maxPlayers ];
}

void ModifierAutoHandicap::addPlayer( Player *player )
{
	_playerHandicapData[ player->entnum ].reset();
}

float ModifierAutoHandicap::playerDamaged( Player *damagedPlayer, Player *attackingPlayer, float damage, int meansOfDeath )
{
	if ( damagedPlayer == attackingPlayer )
		return damage;

	// Change the damage based on the damaged and the attacking player's current handicap

	return damage * _playerHandicapData[ attackingPlayer->entnum ]._handicap /
			_playerHandicapData[ damagedPlayer->entnum ]._handicap;
}

void ModifierAutoHandicap::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	if ( killedPlayer == attackingPlayer || !attackingPlayer )
		return;

	// Make it harder for the attacker to get another kill

	_playerHandicapData[ attackingPlayer->entnum ]._handicap /= _attackerHandicapModifier;

	// Make it easier for the victim to get kills

	_playerHandicapData[ killedPlayer->entnum ]._handicap *= _victimHandicapModifier;
}

// Setup constants

const float	ModifierActionHero::_actionHeroRegenRate = 5.0f;
const int	ModifierActionHero::_extraPointsForKillingActionHero = 4;

ModifierActionHero::ModifierActionHero()
{
	_actionHeroNum = -1;
	_healthRegenRate = _actionHeroRegenRate;

	_actionHeroIconIndex     = gi.imageindex( "sysimg/icons/mp/actionhero" );
	_actionHeroInfoIconIndex = gi.imageindex( "sysimg/icons/mp/actionhero_icon" );
}

void ModifierActionHero::init( int maxPlayers )
{
	multiplayerManager.cacheMultiplayerFiles( "mp_actionhero" );
}

void ModifierActionHero::matchStarting( void )
{
	_actionHeroNum = -1;
}

void ModifierActionHero::removePlayer( Player *player )
{
	if ( player->entnum	== _actionHeroNum )
	{
		// Action hero left the game so make action hero up for grabs

		multiplayerManager.centerPrintAllClients( "$$ActionHeroLeft$$", CENTERPRINT_IMPORTANCE_NORMAL );

		_actionHeroNum = -1;
	}
}

void ModifierActionHero::update( float frameTime )
{
	float healthToRegen;
	Player *player;

	// Regen the action hero's health a little

	if ( _actionHeroNum != -1 )
	{
		healthToRegen = _healthRegenRate * frameTime;

		player = multiplayerManager.getPlayer( _actionHeroNum );

		if ( player )
		{
			if ( player->canRegenerate() )
			{
				multiplayerManager.addPlayerHealth( _actionHeroNum, healthToRegen );
			}
		}
	}
}

void ModifierActionHero::playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath )
{
	// Make sure everything is ok

	if ( !killedPlayer )
		return;

	// Always stop being the action hero if you die for any reason

	if ( killedPlayer->entnum == _actionHeroNum  )
	{
		Event *removeAttachedModel = new Event( EV_RemoveAttachedModelByTargetname );
		removeAttachedModel->AddString( "actionHero" );
		killedPlayer->ProcessEvent( removeAttachedModel );
	}

	if ( !attackingPlayer || ( killedPlayer == attackingPlayer ) )
	{
		if ( killedPlayer->entnum == _actionHeroNum  )
		{
			_actionHeroNum = -1;

			multiplayerManager.centerPrintAllClients( "$$ActionHeroDead$$", CENTERPRINT_IMPORTANCE_NORMAL );
		}

		return;
	}

	if ( ( _actionHeroNum == -1 ) || ( killedPlayer->entnum == _actionHeroNum ) )
	{
		// This is either the first kill or the attacking player killed the action hero, either way we have
		//   a new actio hero

		// Give extra points if the last action hero was killed

		if ( _actionHeroNum != -1 )
		{
			multiplayerManager.addPoints( attackingPlayer->entnum, _extraPointsForKillingActionHero );
		}

		// Make the killer the action hero

		_actionHeroNum = attackingPlayer->entnum;

		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-attrex-rifle.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-burstrifle.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-compressionrifle.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-drull-staff.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-fieldassaultrifle.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-grenadelauncher.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-imod.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-photon.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-rom-disruptor.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-rom-radgun.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-sniperrifle.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-tetryon.tik" );
		multiplayerManager.givePlayerItem( attackingPlayer->entnum, "models/weapons/worldmodel-tricorder.tik" );

		attackingPlayer->GiveAmmo( "Plasma", 400, false, 400 );
		attackingPlayer->GiveAmmo( "Fed", 400, false, 400 );
		attackingPlayer->GiveAmmo( "Idryll", 400, false, 400 );

		multiplayerManager.centerPrintAllClients( va( "%s $$NowActionHero$$", attackingPlayer->client->pers.netname ), CENTERPRINT_IMPORTANCE_NORMAL );
		multiplayerManager.broadcastSound( "localization/sound/dialog/dm/comp_nah.mp3", CHAN_AUTO, DEFAULT_VOL, DEFAULT_MIN_DIST, NULL, 1.2f );

		Event *attachModel = new Event( EV_AttachModel );
		attachModel->AddString( "models/fx/fx-actionHero.tik" );
		attachModel->AddString( "Bip01" );
		attachModel->AddFloat( 1.0f );
		attachModel->AddString( "actionHero" );
		attackingPlayer->ProcessEvent( attachModel );

		// Max the player's health

		attackingPlayer->addHealth( 100.0f );

		// Max the player's armor

		Event *armorEvent = new Event( EV_Sentient_GiveArmor );
		armorEvent->AddString( "BasicArmor" );
		armorEvent->AddFloat( 200.0f );
		armorEvent->AddInteger( false );
		attackingPlayer->ProcessEvent( armorEvent );
	}
}

int ModifierActionHero::getIcon( Player *player, int statNum, int value )
{
	if ( statNum == STAT_MP_SPECIALTY_ICON )
	{
		// Return the action hero icon if this player is the action hero

		if ( _actionHeroNum == player->entnum )
			return _actionHeroIconIndex;
		else 
			return -1;
	}

	return value;
}

int ModifierActionHero::getInfoIcon( Player *player )
{
	if ( _actionHeroNum == player->entnum )
		return _actionHeroInfoIconIndex;
	else
		return 0;
}

int ModifierActionHero::getScoreIcon( Player *player, int index, int value )
{
	if ( index == SCOREICON2 )
	{
		if ( _actionHeroNum == player->entnum )
			return _actionHeroIconIndex;
		else 
			return 0;
	}

	return value;
}

ModifierPointsPerWeapon::~ModifierPointsPerWeapon()
{
	_weapons.FreeObjectList();
	_projectiles.FreeObjectList();
}

void ModifierPointsPerWeapon::init( int maxPlayers )
{
	// Read in all of the projectile/weapon points data

	readMultiplayerConfig( "global/mp_PointsPerWeapon.cfg" );
}

void ModifierPointsPerWeapon::readMultiplayerConfig( const char *configName )
{
	Script buffer;
	const char *token;

	// Make sure the file exists

	if ( !gi.FS_Exists( configName ) )
		return;

	// Load the file

	buffer.LoadFile( configName );

	// Parse the file

	while ( buffer.TokenAvailable( true ) )
	{
		token = buffer.GetToken( true );

		if ( !parseConfigToken( token, &buffer ) )
		{
			gi.DPrintf( "Token %s from %s not handled by anyone\n", token, configName );
		}
	}
}

bool ModifierPointsPerWeapon::parseConfigToken( const char *key, Script *buffer )
{
	PointsPerWeaponData pointsPerWeaponData;

	if ( stricmp( key, "projectile" ) == 0 )
	{
		// Get projectile name

		if ( !buffer->TokenAvailable( false ) )
			return false;

		pointsPerWeaponData._name = buffer->GetToken( false );

		// Get points for this projectile

		if ( !buffer->TokenAvailable( false ) )
			return false;

		pointsPerWeaponData._points = atoi( buffer->GetToken( false ) );

		// Add projectile to the projectile list

		_projectiles.AddObject( pointsPerWeaponData );

		return true;
	}
	else if ( stricmp( key, "weapon" ) == 0 )
	{
		// Get weapon name

		if ( !buffer->TokenAvailable( false ) )
			return false;

		pointsPerWeaponData._name = buffer->GetToken( false );

		// Get points for this projectile

		if ( !buffer->TokenAvailable( false ) )
			return false;

		pointsPerWeaponData._points = atoi( buffer->GetToken( false ) );

		// Add weapon to the weapon list

		_weapons.AddObject( pointsPerWeaponData );
	
		return true;
	}

	return false;	
}

int ModifierPointsPerWeapon::getPointsForKill( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath, int points )
{
	PointsPerWeaponData *pointsPerWeaponData;
	int i;

	// Make sure everything is ok

	if ( killedPlayer == attackingPlayer )
		return points;

	if ( points <= 0 )
		return points;

	// See if the kill was by a projectile or directly from a weapon

	if ( inflictor && inflictor->isSubclassOf( Projectile ) )
	{
		Projectile *proj;

		proj = (Projectile *)inflictor;

		// Determine the points based off of the projectile

		for ( i = 1 ; i <= _projectiles.NumObjects() ; i++ )
		{
			pointsPerWeaponData = &_projectiles.ObjectAt( i );

			if ( stricmp( proj->model.c_str(), pointsPerWeaponData->_name.c_str() ) == 0 )
				return pointsPerWeaponData->_points;
		}

		// Didn't find it so just return the default

		return points;
	}
	else
	{
		str weaponName;

		// Get the name of the weapon

		attackingPlayer->getActiveWeaponName( WEAPON_ANY, weaponName );

		// Determine the points based off of the weapon

		for ( i = 1 ; i <= _weapons.NumObjects() ; i++ )
		{
			pointsPerWeaponData = &_weapons.ObjectAt( i );

			if ( stricmp( weaponName.c_str(), pointsPerWeaponData->_name.c_str() ) == 0 )
				return pointsPerWeaponData->_points;
		}

		// Didn't find it so just return the default

		return points;
	}
}

