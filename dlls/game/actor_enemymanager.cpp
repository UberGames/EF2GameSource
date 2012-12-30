//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/actor_enemymanager.cpp                     $
// $Revision:: 41                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:35p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//

#include "_pch_cpp.h"
#include "actor_enemymanager.h"
#include "player.h"
#include "object.h"

//======================================
// EnemyManager Implementation
//=====================================

//
// Name:        EnemyManager()
// Parameters:  None
// Description: Constructor
//
EnemyManager::EnemyManager()
{
	// Should always use other constructor
	gi.Error( ERR_FATAL, "EnemyManager::EnemyManager -- Default Constructor Called" );	
}

//
// Name:        EnemyManager()
// Parameters:  Actor *actor
// Description: Constructor
//
EnemyManager::EnemyManager( Actor *actor )
{
	//Initialize our Actor
	if ( actor )
		act = actor;
	else 
		gi.Error( ERR_DROP, "EnemyManager::EnemyManager -- actor is NULL" );
	
	_lockedOnCurrentEnemy = false;
		
}

//
// Name:       ~EnemyManager()
// Parameters:  None
// Description: Destructor
//
EnemyManager::~EnemyManager()
{
}


//
// Name:        GetCurrentEnemy()
// Parameters:  None
// Description: Returns the _currentEnemy
//
EntityPtr EnemyManager::GetCurrentEnemy()
{
	if ( act->forcedEnemy )
	{
		if ( act->forcedEnemy->health <= 0 )
		{
			act->forcedEnemy = NULL;
			_lockedOnCurrentEnemy = false;
			return _currentEnemy;
		}
		else
			return act->forcedEnemy;
	}		
	
	if ( !_currentEnemy )
		FindHighestHateEnemy();
	
	return _currentEnemy;
}

//
// Name:        SetCurrentEnemy()
// Parameters:  Entity *enemy
// Description: sets _currentEnemy
//
void EnemyManager::SetCurrentEnemy( Entity *enemy )
{
	if ( enemy )
		_currentEnemy = enemy;
	else
	{
		_lockedOnCurrentEnemy = false;
	}
}


EntityPtr EnemyManager::GetAlternateTarget()
{
	return _alternateTarget;
}

void EnemyManager::SetAlternateTarget( Entity *target )
{
	_alternateTarget = target;
}
//
// Name:        FindHightestHateEnemy()
// Parameters:  None
// Description: Sets the _currentEnemy to the highest enemy on the hate list
//
void EnemyManager::FindHighestHateEnemy()
{
	float hateValue = 0;   
	HateListEntry_t listIndex;
	float hateFactor;
	
	
	
	if ( IsLockedOnCurrentEnemy() )
		return;
	
	
	for ( int i = _hateList.NumObjects() ; i > 0 ; i-- )
	{
		hateFactor = 100;

		listIndex = _hateList.ObjectAt( i );
		listIndex.hate = listIndex.hate * ( 1 / listIndex.lastDistance );
		
		if ( !listIndex.enemy )
		{
			_hateList.RemoveObjectAt( i );
			continue;
		}
		
		//If the enemy is a player, make sure to modify our hate
		if ( listIndex.enemy->isSubclassOf( Sentient ) && !act->GetActorFlag(ACTOR_FLAG_UPDATE_HATE_WITH_ATTACKERS) )
		{
			Sentient *theEnemy;
			theEnemy = (Sentient*)(Entity*)listIndex.enemy;
			hateFactor *= theEnemy->GetHateModifier();
		}
			
		
		listIndex.hate *= hateFactor;
		
		if ( listIndex.hate >= hateValue && listIndex.hate > 0.0 )
		{
			_currentEnemy = listIndex.enemy;
			hateValue = listIndex.hate;
		}
		
	}
}

//
// Name:        FindClosestEnemy()
// Parameters:  None
// Description: Sets the _currentEnemy to the enemy closest to the actor
//
void EnemyManager::FindClosestEnemy()
{
	float distance = 99999999.9;
	
	for ( int i = 1 ; i <= _hateList.NumObjects() ; i++ )
	{
		EntityPtr enemy;
		HateListEntry_t listIndex;
		
		listIndex = _hateList.ObjectAt( i );
		
		if ( !act->combatSubsystem->CanAttackTarget( listIndex.enemy ) )
			continue;
		
		if ( listIndex.lastDistance < distance )
		{
			_currentEnemy = listIndex.enemy;
			distance = listIndex.lastDistance;
		}
	}	
}

void EnemyManager::FindNextEnemy()
{
	HateListEntry_t listIndex;	
	float hateValue;
	
	hateValue = 0;	
	for ( int i = 1 ; i <= _hateList.NumObjects() ; i++ )
	{
		listIndex = _hateList.ObjectAt( i );
		if ( listIndex.hate <= _currentEnemyHate && listIndex.hate > hateValue )
		{			
			hateValue = listIndex.hate;
			_lastEnemy = _currentEnemy;
			_currentEnemy = listIndex.enemy;
			_currentEnemyHate = hateValue;			
		}
	}
}

//
// Name:        ClearCurrentEnemy()
// Parameters:  None
// Description: sets the _currentEnemy to NULL
//
void EnemyManager::ClearCurrentEnemy()
{
	_lockedOnCurrentEnemy = false;
	_currentEnemy = NULL;
}

//
// Name:        ClearHateList()
// Parameters:  None
// Description: Wipes out the hate list
//
void EnemyManager::ClearHateList()
{
	for ( int i = _hateList.NumObjects() ; i > 0 ; i-- )
	{
		_hateList.RemoveObjectAt( i );
	}
	
	_hateList.FreeObjectList();
}

//
// Name:        Likes()
// Parameters:  Entity *ent
// Description: Checks if Actor likes the entity
//
qboolean EnemyManager::Likes( Entity *ent )
{
	Actor *actor;
	
	if ( !ent )
		return false;
	
	if ( ent->isClient() )
	{
		return ( act->actortype == IS_FRIEND );
	}
	
	if ( act->actortype == IS_MONSTER )
	{
		// Monsters don't like anyone
		return false;
	}
	
	if ( ent->isSubclassOf( Actor ) )
	{
		actor = ( Actor * )ent;
		
		if ( actor->enemytype == act->enemytype )
			return true;
		
		if  ( actor->actortype != act->actortype )
			return false;
		else if ( act->actortype == IS_FRIEND )
			return true;
	}
	
	return false;
}

//
// Name:        Hates()
// Parameters:  Entity *ent
// Description: Checks if Actor hates the entity
//
qboolean EnemyManager::Hates( Entity *ent )
{
	Actor *actor;
	
	if ( !ent )
		return false;
	
	if ( ent->isClient() )
	{
		return ( act->actortype != IS_CIVILIAN ) &&  (( act->actortype != IS_FRIEND ) && ( act->actortype != IS_TEAMMATE));
	}
	else if ( ent->isSubclassOf( Actor ) && ( act->actortype != IS_INANIMATE ) )
	{
		actor = ( Actor * )ent;
		if ( ( actor->actortype <= IS_ENEMY ) && ( act->actortype <= IS_ENEMY ) )
		{
			return false;
		}
		if ( ( actor->actortype == IS_FRIEND ) && ( act->actortype <= IS_ENEMY ) )
		{
			return true;
		}
		if ( ( actor->actortype == IS_TEAMMATE ) && ( act->actortype <= IS_ENEMY ) )
		{
			return true;
		}
		if ( ( actor->actortype <= IS_ENEMY ) && ( act->actortype == IS_FRIEND ) )
		{
			return true;
		}
		if ( ( actor->actortype <= IS_ENEMY ) && ( act->actortype == IS_TEAMMATE ) )
		{
			return true;
		}
		if ( ( actor->actortype == IS_TEAMMATE ) && ( act->actortype == IS_TEAMMATE ) )
		{
			return false;
		}
		
	}
	
	return false;
}

//
// Name:        IsValidEnemy
// Parameters:  Entity *enemy
// Description: Checks if the passed in entity is a valid target or not
//
qboolean EnemyManager::IsValidEnemy( Entity *enemy )
{
	
	if ( !enemy || ( enemy == world ) || ( enemy == act ) || ( enemy->flags & FL_NOTARGET ) || ( enemy->takedamage == DAMAGE_NO ) || enemy->deadflag )
		return false;
	
	if ( !Hates( enemy ) )
		return false;
	
	if ( !CanAttack( enemy ) )
		return false;
	
	if ((enemy->edict->s.renderfx & RF_DONTDRAW) )
		return false;

	return true;
}

//
// Name:        CanAttack
// Parameters:  Entity *ent
// Description: Checks if the actor is allowed to attack the passed in entity
//
qboolean EnemyManager::CanAttack( Entity *ent )
{
	
	if ( !ent )
		return false;
	
	if ( act->targetType == ATTACK_ANY )
		return true;
	
	
	if ( ent->isSubclassOf( Player ) && ( act->targetType == ATTACK_PLAYER_ONLY ) )
		return true;
	
	if ( ent->isSubclassOf( Actor ) && ( act->targetType == ATTACK_ACTORS_ONLY ) )
		return true;
	
	if ( ent->isSubclassOf ( LevelInteractionTrigger ) && ( act->targetType == ATTACK_LEVEL_INTERACTION ) )
		return true;
	
	
	return false;
	
}

//
// Name:         CanAttackAnyEnemy
// Parameters:   None
// Description:  Checks if we can shoot any of our enemies in our list
qboolean EnemyManager::CanAttackAnyEnemy()
{
	HateListEntry_t *listIndex;
	Entity *target;
	
	for ( int i = 1 ; i <= _hateList.NumObjects() ; i++ )
	{
		listIndex = &_hateList.ObjectAt( i );
		
		target = listIndex->enemy;
		if ( target )
		{
			if ( act->combatSubsystem->CanAttackTarget( target ) )
				return true;
		}
		
	}
	
	return false;
}

//
// Name:        AdjustHate()
// Parameters:  Entity *enemy    -- The entity to adjust
//              float adjustment -- how much to adjust
// Description: Adjusts the hate value of the enemy
//
void EnemyManager::AdjustHate( Entity *enemy , float adjustment )
{
	HateListEntry_t *listIndex;
	
	if ( !enemy )
		return;
	
	
	for ( int i = 1 ; i <= _hateList.NumObjects() ; i++ )
	{
		listIndex = &_hateList.ObjectAt( i );
		
		if ( listIndex->enemy == enemy )
		{
			listIndex->hate += adjustment;
			if ( listIndex->hate < 0.0f )
				listIndex->hate = 0.0f;
			
			return;
		}
		
	}
	
	
}

//
// Name:        AdjustDamageCaused()
// Parameters:  Entity *enemy    -- The entity to adjust
//              float adjustment -- how much to adjust
// Description: Adjusts the damage caused
//
void EnemyManager::AdjustDamageCaused( Entity *enemy , float adjustment )
{
	HateListEntry_t listIndex;
	
	if ( !enemy )
		return;
	
	for ( int i = 1 ; i <= _hateList.NumObjects() ; i++ )
	{
		listIndex = _hateList.ObjectAt( i );
		
		if ( listIndex.enemy == enemy )
		{
			listIndex.damageCaused += adjustment;
			return;
		}
		
	}
}

//
// Name:        TryToAddToHateList
// Parameters:  Entity *enemy 
// Description: Checks if the passed in entity should be added to the hate list
//              and calls _AddToHateList if it does
//
void EnemyManager::TryToAddToHateList( Entity *enemy )
{
	if ( !enemy )
		return;
	
	if ( IsValidEnemy( enemy ) && !IsInHateList( enemy ) )
	{
		if ( !act->combatSubsystem->CanAttackTarget( enemy ) && !CanGetToEntity( enemy ) )
		{
			return;			
		}
		
		_AddToHateList( enemy );
	}
	
	
}

//
// Name:        _AddToHateList()
// Parameters:  Entity *enemy -- The enemy to add
// Description: Adds the enemy to the hate list
//
void EnemyManager::_AddToHateList( Entity *enemy )
{
	HateListEntry_t listIndex;
	
	if ( !enemy )
		return;
	
	listIndex.enemy = enemy;
	listIndex.damageCaused = 0;
	listIndex.hate = DEFAULT_INITIAL_HATE;
	listIndex.lastSightTime = level.time ;
	listIndex.canSee = false;
	//listIndex.lastDistance = -1;
	listIndex.nextSightTime = level.time + MIN_SIGHT_DELAY;
	UpdateDistance( &listIndex );
	_hateList.AddObject(listIndex);
	
}

//
// Name:        IsInHateList()
// Parameters:  Entity *enemy
// Description: Checks if enemy is in the hate list
//
qboolean EnemyManager::IsInHateList( Entity *enemy )
{
	int index = _findEntityInHateList( enemy );
	return index > 0;
}

//----------------------------------------------------------------
// Name:			IsLastInHateList
// Class:			EnemyManager
//
// Description:		Determine if the Entity passed is in the hate list, 
//					and if it is at the end of the list.
//
// Parameters:		Entity* enemy - the entity to look for	
//
// Returns:			qboolean
//----------------------------------------------------------------
qboolean EnemyManager::IsLastInHateList( Entity* enemy )
{
	int index = _findEntityInHateList( enemy );
	return index == _hateList.NumObjects();
}


//----------------------------------------------------------------
// Name:			_findEntityInHateList
// Class:			EnemyManager
//
// Description:		Search the hate list for the passed entity.
//
// Parameters:		Entity* searchEnt - the entity to look for	
//
// Returns:			The index of the entity in the list
//					0 means it isn't in the list.
//----------------------------------------------------------------
int EnemyManager::_findEntityInHateList( Entity *searchEnt )
{
	if( !searchEnt )
	{
		return false;
	}
	
	// manually iterate through the hatelist
	HateListEntry_t listIndex;
	for( int i = 1; i <= _hateList.NumObjects(); i++ )
	{
		listIndex = _hateList.ObjectAt( i );
		
		if ( listIndex.enemy == searchEnt )
		{
			return i;
		}
	}

	// was not found, return 0
	return 0;
}


//----------------------------------------------------------------
// Name:			TrivialUpdate
// Class:			EnemyManager
//
// Description:		Minimal Hatelist update required for Actors
//
// Parameters:		
//					None
//
// Returns:			None
//----------------------------------------------------------------
void EnemyManager::TrivialUpdate()
{
	for ( int i = _hateList.NumObjects() ; i > 0 ; i-- )
	{
		_hateList.ObjectAt( i ).lastDistance = 0.0f;
	}
}

//
// Name:        Update()
// Parameters:  None
// Description: Updates HateListEntry_ts
//
void EnemyManager::Update()
{
	HateListEntry_t *listIndex;
	
	for ( int i = _hateList.NumObjects() ; i > 0 ; i-- )
	{
		listIndex = &_hateList.ObjectAt( i );
		
		//Check if enemy is alive
		if ( !act->IsEntityAlive( listIndex->enemy ) )			
		{
			if ( listIndex->enemy == _currentEnemy )
				ClearCurrentEnemy();				
			
			_hateList.RemoveObjectAt(i);
			continue;
		}
		
		UpdateDistance( listIndex );
		UpdateCanSee( listIndex );

		// Bump the player's action level, TODO: this needs to change to a general you have been targeted call sometime

		if ( listIndex->enemy && listIndex->enemy->isSubclassOf( Player ) )
		{
			if ( act->GetActorFlag(ACTOR_FLAG_UPDATE_ACTION_LEVEL) )
				{
				Player *player = (Player *)(Entity *)listIndex->enemy;
				player->IncreaseActionLevel( 100 );
				}
		}

		if ( act->GetActorFlag(ACTOR_FLAG_UPDATE_HATE_WITH_ATTACKERS) )
			UpdateAttackers( listIndex );
		
			/*if ( !act->combatSubsystem->CanAttackTarget( listIndex->enemy ) )
			{
			listIndex->hate = 0.0f;
			if ( listIndex->enemy == _currentEnemy )
			ClearCurrentEnemy();
			
			  _hateList.RemoveObjectAt(i);
			  }
		*/
		
		
		//If we haven't seen 
		if ( 
			( act->timeBetweenSleepChecks > 0.0f ) && 
			( listIndex->lastSightTime + act->timeBetweenSleepChecks <= level.time && !gi.inPVS( listIndex->enemy->centroid, act->centroid ) ) 
			)
		{
			if ( listIndex->enemy == _currentEnemy && !_lockedOnCurrentEnemy )
				ClearCurrentEnemy();
			_hateList.RemoveObjectAt(i);
		}
		
	}
	// Try and go back to sleep
	TrySleep();
	
	
	//If we don't have an enemy, tell our senses to keep looking
	act->sensoryPerception->SenseEnemies();
	
	
	if ( GetCurrentEnemy() || act->GetActorFlag( ACTOR_FLAG_INVESTIGATING ) || ( act->mode == ACTOR_MODE_SCRIPT ) || ( act->mode == ACTOR_MODE_TALK ) )
		act->last_time_active = level.time;
	
	// Take care of enemies
	act->strategos->Evaluate();
	
}

//
// Name:        UpdateDistance
// Parameters:  HateListEntry_t -- The List Entry to update
// Description: Updates the distance of the enemy
//
void EnemyManager::UpdateDistance( HateListEntry_t *listIndex  )
{
	Vector distance;
	
	//float distanceLength;
	
	if ( !listIndex || !listIndex->enemy )
		return;
	
	Vector enemyDistance; 
	enemyDistance = listIndex->enemy->origin;
	
	distance = enemyDistance - act->origin;
	//distanceLength = distance.length();
	
	listIndex->lastDistance = distance.length();		
	
}

void EnemyManager::UpdateAttackers( HateListEntry_t *listIndex )
{
	if ( _currentEnemy == listIndex->enemy )
		return;
	
	int numberOfAttackers;
	ActorGroup* group;	
	
	group = (ActorGroup*)groupcoordinator->GetGroup( act->GetGroupID() );
	
	if ( !group )
		return;
	
	numberOfAttackers = group->CountMembersAttackingEnemy( listIndex->enemy );
	
	listIndex->hate = listIndex->hate / (numberOfAttackers + 1);
	
}

//
// Name:        UpdateCanSee
// Parameters:  HateListEntry_t -- The List Entry to update
// Description: Updates the cansee of the enemy
//
void EnemyManager::UpdateCanSee( HateListEntry_t *listIndex )
{
	if ( !listIndex || ( listIndex->nextSightTime > level.time ) )
		return;
	
	qboolean canSee;
	canSee = act->sensoryPerception->CanSeeEntity( act , listIndex->enemy , true , true );
	
	// Notify Strategos of status change
	if ( canSee != listIndex->canSee )
		act->strategos->NotifySightStatusChanged( listIndex->enemy , canSee );
	
	// Update the SightCheckTimes
	listIndex->canSee = canSee;
	
	// We use a faster delay for our current enemy than everyone else
	float next_sight_delay;
	if ( listIndex->enemy == _currentEnemy )
		next_sight_delay = act->max_inactive_time / 5.0f + G_CRandom( 0.5f );
	else
		next_sight_delay = MIN_SIGHT_DELAY + ( act->max_inactive_time / 2.0f + G_CRandom( 0.5f ) );
	
	if ( next_sight_delay < 1.0f )
		next_sight_delay = 1.0f;
	
	listIndex->nextSightTime = level.time + next_sight_delay;
	
	if ( canSee )
		listIndex->lastSightTime = level.time;
	
	if ( listIndex->lastSightTime + act->max_inactive_time < level.time )
		ClearCurrentEnemy();
}


//
// Name:        LockOnCurrentEnemy
// Parameters:  qboolean lock
// Description: Sets the _lockedOnCurrentEnemy flag
//
void EnemyManager::LockOnCurrentEnemy( qboolean lock )
{
	_lockedOnCurrentEnemy = lock;
}

//
// Name:        IsLockedOnCurrentEnemy
// Parameters:  None
// Description: Returns the _lockedOnCurrentEnemy flag
//
qboolean EnemyManager::IsLockedOnCurrentEnemy()
{
	return _lockedOnCurrentEnemy;   
}


Vector EnemyManager::GetAwayFromEnemies()
{
	EntityPtr enemy;
	HateListEntry_t listIndex;
	
	Vector enemyToSelf;
	Vector temp;
	float mag;
	
	Vector bestAwayVector;
	
	for ( int i = 1 ; i <= _hateList.NumObjects() ; i++ )
	{
		listIndex = _hateList.ObjectAt( i );
		enemy = listIndex.enemy;
		
		enemyToSelf = act->origin - enemy->origin;
		mag = enemyToSelf.lengthSquared();
		
		temp = enemyToSelf * (1 / mag);
		
		bestAwayVector = bestAwayVector + temp;
	}
	
	bestAwayVector.normalize();
	
	return bestAwayVector;
}

qboolean EnemyManager::InEnemyLineOfFire()
{
	Actor  *enemyActor  = NULL;
	Player *enemyPlayer = NULL;
	
	Vector enemyToSelf;
	
	float angleDiff = 0.0f;
	
	
	if ( !_currentEnemy )
		return false;
	
	enemyToSelf = act->origin - _currentEnemy->origin;
	enemyToSelf = enemyToSelf.toAngles();
	
	if ( _currentEnemy->isSubclassOf(Actor) )
	{
		enemyActor = (Actor*)(Entity*)_currentEnemy;
		
		if ( !enemyActor )
			return false;
		
		angleDiff = abs( (int) AngleNormalize180(enemyActor->angles[YAW] - enemyToSelf[YAW]) );
	}
	
	if ( _currentEnemy->isSubclassOf(Player) )
	{
		enemyPlayer = (Player*)(Entity*)_currentEnemy;
		
		if ( !enemyPlayer )
			return false;
		
		Vector pAngles = enemyPlayer->GetVAngles();
		
		angleDiff = abs( (int) AngleNormalize180(pAngles[YAW] - enemyToSelf[YAW]) );         
		
	}
	
	if ( angleDiff < 10.0f )
		return true;   
	
	
	return false;
	
}

float EnemyManager::GetDistanceFromEnemy()
{
	Vector selfToEnemy;
	
	if ( !_currentEnemy )
		return -1;
	
	selfToEnemy = _currentEnemy->origin - act->origin;
	
	return selfToEnemy.length();
	
}

//
// Name:       DoArchive
// Parameters: Archiver &arc -- The archiver object
//             Actor *actor  -- The actor
// Description: Sets the act pointer and calls Archive
//
void EnemyManager::DoArchive( Archiver &arc , Actor *actor )
{
	Archive( arc );
	if ( actor )
		act = actor;
	else
		gi.Error( ERR_FATAL, "EnemyManager::DoArchive -- actor is NULL" );
}

//
// Name:        Archive()
// Parameters:  Archiver &arc -- The archiver object
// Description: Archives the class
//
void EnemyManager::Archive ( Archiver &arc )
{
	HateListEntry_t hateEntry;
	HateListEntry_t *realHateEntry;	
	int numEntries;
	
	
	if ( arc.Saving() )
	{
		numEntries = _hateList.NumObjects();
		arc.ArchiveInteger( &numEntries );
		
		for ( int i = 1 ; i <= numEntries ; i++ )
		{
			hateEntry = _hateList.ObjectAt( i );
			
			arc.ArchiveSafePointer( &hateEntry.enemy );	
			arc.ArchiveFloat( &hateEntry.lastSightTime );
			arc.ArchiveFloat( &hateEntry.nextSightTime );
			arc.ArchiveBoolean( &hateEntry.canSee );
			arc.ArchiveFloat( &hateEntry.damageCaused );
			arc.ArchiveFloat( &hateEntry.hate );
			arc.ArchiveFloat( &hateEntry.lastDistance );		
		}
	}
	else
	{
		arc.ArchiveInteger( &numEntries );
		
		_hateList.Resize( numEntries );
		
		for ( int i = 1 ; i <= numEntries ; i++ )
		{
			_hateList.AddObject( hateEntry );
			
			realHateEntry = &_hateList.ObjectAt( i );
			
			arc.ArchiveSafePointer( &realHateEntry->enemy );
			arc.ArchiveFloat( &realHateEntry->lastSightTime );
			arc.ArchiveFloat( &realHateEntry->nextSightTime );
			arc.ArchiveBoolean( &realHateEntry->canSee );
			arc.ArchiveFloat( &realHateEntry->damageCaused );
			arc.ArchiveFloat( &realHateEntry->hate );
			arc.ArchiveFloat( &realHateEntry->lastDistance );
		}
	}
	
	arc.ArchiveSafePointer( &_currentEnemy );
	arc.ArchiveSafePointer( &_lastEnemy );
	
	arc.ArchiveSafePointer( &_alternateTarget );
	
	arc.ArchiveBoolean( &_lockedOnCurrentEnemy );
	arc.ArchiveFloat( &_currentEnemyHate );
}

//----------------------------------------------------------------
// Name:			TrySleep
// Class:			EnemyManager
//
// Description:		Puts the actor to sleep if it can
//
// Parameters:		
//					None
//
// Returns:			None
//----------------------------------------------------------------
void EnemyManager::TrySleep( void )
{
	const Entity *currentEnemy = GetCurrentEnemy();
	// This is here because, if the ai gets turned off in the level, we don't want to go to sleep for if/when
	// the ai gets turned back on in the level
	if ( !level.ai_on )
		act->last_time_active = level.time;
	
	// See if we should go back to sleep -- If max_inactive_time < 1 , then the actor will never go to sleep
	// If we DO have a max_inactive_time > 0 AND our last_time_active == 0 ( meaning it's our first time in here ) we
	// should go ahead and try to fall asleep even though we have hit our max_inactive_time
	if ( !currentEnemy && ( act->max_inactive_time > 0.0f ) && ( (act->last_time_active + act->max_inactive_time < level.time)))		
	{
		//Well, we haven't had an enemy for a while, so let's see if the player is nearby
		Player *player = 0;
		for(int i = 0; i < game.maxclients; i++)
		{
			player = GetPlayer(i);
			
			if ( player )
			{
				//We should NOT fall asleep if:
				// our mode is ACTOR_MODE_IDLE _AND_ player flags are not equal to FL_NOTARGET 
				// OR
				// Player is within our vision distance
				// OR
				// Player is within the PVS
				
				//
				// 7/15/02 -- SK
				// Added via && the "gi.inPVS( player->centroid, act->centroid )" to the ACTOR_MODE_IDLE 
				// checks because, without it, all AI_OFF'd actors would still be considered "active"
				// and this was ruining the framerate... I do not believe this is going to have any 
				// detrimental side effects. 
				//
				if ( ( act->mode == ACTOR_MODE_IDLE ) && !(player->flags & FL_NOTARGET) && gi.inPVS( player->centroid, act->centroid ) || act->sensoryPerception->WithinVisionDistance(player) || gi.inPVS( player->centroid, act->centroid ) )
				{
					act->last_time_active = level.time;
				}
				
				else
				{
					if ( act->mode == ACTOR_MODE_AI )
					{
						act->EndMode();
						
						act->Sleep();
						
						if ( act->idle_thread.length() > 1 )
						{
							ExecuteThread( act->idle_thread, false );
						}
					}
					else
					{
						act->Sleep();
					}
				}
			}
		}
	}
}

bool EnemyManager::IsAnyEnemyInRange( float range )
{
	HateListEntry_t *listIndex;
	
	for ( int i = _hateList.NumObjects() ; i > 0 ; i-- )
	{
		listIndex = &_hateList.ObjectAt( i );
		
		//Check if enemy is alive
		if ( !act->IsEntityAlive( listIndex->enemy ) )			
		{
			if ( listIndex->enemy == _currentEnemy )
				ClearCurrentEnemy();				
			
			_hateList.RemoveObjectAt(i);
			continue;
		}
		
		if ( act->WithinDistance( listIndex->enemy , range ) )
			return true;     
		
	}
	
	return false;
	
}

float EnemyManager::getEnemyCount()
{
	return _hateList.NumObjects();
}

bool EnemyManager::CanGetToEntity(Entity *ent)
{
	FindMovementPath	find;
	Path				*path;
	bool				success;
	
	success = false;
	// Set up our pathing heuristics
	find.heuristic.self = act;
	find.heuristic.setSize( act->size );
	find.heuristic.entnum = act->entnum;
	
	path = find.FindPath( act->origin, ent->origin );
	
	if ( path )
	{
		success = true;
	}
	
	delete path;
	path = NULL;
	
	return success;
}

bool EnemyManager::HasEnemy()
{
	if ( _currentEnemy )
		return true;

	return false;
}
