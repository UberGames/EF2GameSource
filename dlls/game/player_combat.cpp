//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/player_combat.cpp                             $
// $Revision:: 40                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 5/17/03 3:09p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Player combat system and combat utility functions
//

#include "_pch_cpp.h"
#include "player.h"
#include "weaputils.h"
#include <qcommon/gameplaymanager.h>
#include "mp_manager.hpp"


Entity * Player::FindClosestEntityInRadius( const float horizontalFOVDegrees, const float verticalFOVDegrees, const float maxDistance )
{
	Vector torsoForward;
	Vector torsoRight;
	Vector torsoUp;

	torsoAngles.AngleVectors( &torsoForward, &torsoRight, &torsoUp );

	const float horizontalFOVComponent = static_cast<float>( sin( DEG2RAD( horizontalFOVDegrees / 2.0f ) ) );
	const float verticalFOVComponent = static_cast<float>( sin( DEG2RAD( verticalFOVDegrees / 2.0f ) ) );

	Entity   *bestEntity=NULL;
	float    bestDistance = maxDistance;
	
	// Find closest enemy in radius
	Entity   *currentEntity = NULL;
	
	while( ( currentEntity = findradius( currentEntity, centroid, maxDistance ) ) != NULL )
	{
		bool validEntity = false;
		
		if ( currentEntity->flags & FlagAutoaim )
		{
			validEntity = true;
		}
		else if ( currentEntity->isSubclassOf( Actor ) && !currentEntity->deadflag )
		{
			Actor *actor = static_cast<Actor *>( currentEntity );
			
			if ( ( actor->actortype == IS_ENEMY ) && actor->CanTarget() && !( actor->bind_info && actor->bind_info->bindmaster ) && ( actor->edict->s.parent == ENTITYNUM_NONE ) )
				validEntity = true;
		}
		
		if ( validEntity )
		{
			// Check to see if the enemy is closest to us
			Vector delta( currentEntity->centroid - centroid );

			const float dist = delta.length();
			
			if ( dist < bestDistance )
            {
				delta.normalize();
				
				// It's close, now check to see if it's in our FOV.
				const float forwardDot = DotProduct( torsoForward, delta );
				const float horizontalDot = DotProduct( torsoRight, delta );
				const float verticalDot = DotProduct( torsoUp, delta );
				
				if ( 
					forwardDot > 0.0f &&
					fabs(horizontalDot) < horizontalFOVComponent &&
					( (fabs(verticalDot) < verticalFOVComponent) || ( dist < 96.0f ) )
					)
				{
					trace_t trace;
					// Do a trace to see if we can get to it
					trace = G_Trace( centroid,
						vec_zero,
						vec_zero,
						currentEntity->centroid,
						NULL,
						MASK_OPAQUE,
						false,
						"FindClosestEntityInRadius" );
					
					if ( trace.ent || ( trace.fraction == 1 ) )
					{
						// dir = delta;
						bestEntity  = currentEntity;
						bestDistance = dist;
					}
				}
            }
		}
	}
	return bestEntity;
}

const bool IsValidHeadTarget( const Entity &entity)
{
	if ( entity.isSubclassOf( Actor ) && !entity.deadflag )
	{
		const Actor &actor = static_cast<const Actor &>( entity );
		if ( ( actor.actortype == IS_ENEMY ) && actor.CanTarget() && ! ( actor.bind_info && actor.bind_info->bindmaster ) && ( actor.edict->s.parent == ENTITYNUM_NONE ) )
		{
			return true;
		}
	}
	else if ( entity.isSubclassOf( Item ) )
	{
		const Item &item = static_cast<const Item &>( entity );
		
		if ( !item.GetOwner() && !item.has_been_looked_at && entity.look_at_me)
		{
			return true;
		}
	}
	return false;
}

Entity* Player::FindHeadTarget( const Vector &origin, const Vector &forward, const float fov, const float maxdist ) 
{
	const int maximumNumberOfCandidates = 10;
	int numberOfCandidates=0;
	float validTargetRadiusSquared = maxdist * maxdist;
	float fovdot = cos( DEG2RAD( fov * 0.5f) );

	if ( multiplayerManager.inMultiplayer() )
		return NULL;

	Container<Entity *> possibleHeadTargets;
	finishableList.FreeObjectList();

	for( gentity_t *currentEdict = active_edicts.next; currentEdict != &active_edicts && numberOfCandidates < maximumNumberOfCandidates; currentEdict = currentEdict->next )
	{
		Entity &currentEntity = *currentEdict->entity;
		if ( IsValidHeadTarget( currentEntity ) )
		{
      		Vector delta = ( currentEntity.centroid ) - origin;
			float lengthOfDeltaSquared = delta.lengthSquared();
			if ( lengthOfDeltaSquared < validTargetRadiusSquared )
			{
				// Headwatch Stuff
				delta.normalize();
				float dot = DotProduct( forward, delta );
				if ( dot > fovdot )
				{
					int insertionPoint = 1;
					const float distanceSquaredToCurrentEntity = Vector::DistanceSquared( origin, currentEntity.centroid );
					for ( ; insertionPoint < possibleHeadTargets.NumObjects(); insertionPoint++ )
					{
						if ( distanceSquaredToCurrentEntity < Vector::DistanceSquared( origin, possibleHeadTargets.ObjectAt( insertionPoint )->centroid ) )
						{
							break;
						}
					}
					numberOfCandidates++;
					possibleHeadTargets.InsertObjectAt( insertionPoint, &currentEntity );
				}

				// Finishable List -- If there's a finishable guy within 5 feet,
				// add him to the finishable list.
				if ( lengthOfDeltaSquared < 6400.0f ) // 5 feet
				{
					if ( currentEntity.isSubclassOf(Actor) )
					{
						Actor *act = (Actor*)&currentEntity;
						if ( act->IsFinishable() )
							finishableList.AddObject(act);
					}
				}
			}
		}
	}

	Entity *closestValidEntity = NULL;

	for (int i = 1; i <= possibleHeadTargets.NumObjects(); i++ )
	{
		Entity *currentCandidate = possibleHeadTargets.ObjectAt( i );
		trace_t trace = G_Trace( origin, vec_zero, vec_zero, currentCandidate->centroid, NULL, MASK_OPAQUE, false, "FindHeadTarget" );
		if ( ( trace.ent && trace.entityNum == currentCandidate->entnum ) || ( trace.fraction == 1.0f ) )
		{
			closestValidEntity = currentCandidate;
			break;
		}
	}
	return closestValidEntity;
}

//====================
//ActivateNewWeapon
//====================
void Player::ActivateNewWeapon( Event * )
{
	// Change the weapon to the currently active weapon as specified by useWeapon
	ChangeWeapon( newActiveWeapon.weapon, newActiveWeapon.hand );
	
	// Clear out the newActiveWeapon
	ClearNewActiveWeapon();
	
	// Clear out the holstered weapons
	holsteredWeapons[WEAPON_LEFT]  = NULL;
	holsteredWeapons[WEAPON_RIGHT] = NULL;
	holsteredWeapons[WEAPON_DUAL]  = NULL;
	
	// let the player know that our weapons are not holstered
	WeaponsNotHolstered();
}

//====================
//DeactivateWeapon
//====================
void Player::DeactivateWeapon( Event *ev )
{
	// Deactivate the weapon
	weaponhand_t hand;
	str   side;
	
	side = ev->GetString( 1 );
	
	hand = WeaponHandNameToNum( side );
	
	if ( hand == WEAPON_ERROR )
		return;
	
	Sentient::DeactivateWeapon( hand );
	
	if ( !GetActiveWeapon( WEAPON_LEFT ) && !GetActiveWeapon( WEAPON_RIGHT ) && !GetActiveWeapon( WEAPON_DUAL ) )
	{
		// let the player know our weapons are holstered
		WeaponsHolstered();
	}
}

//====================
//PutawayWeapon
//====================
void Player::PutawayWeapon( Event *ev )
{
	Weapon * weapon;
	weaponhand_t hand;
	str   side;
	
	side = ev->GetString( 1 );
	
	hand = WeaponHandNameToNum( side );
	
	if ( hand == WEAPON_ERROR )
		return;
	
	weapon = GetActiveWeapon( hand );
	if ( !weapon ) return;
	
	if ( weapon->isSubclassOf( Weapon ) )
	{
		weapon->targetidleflag = false;
		CancelEventsOfType( EV_Weapon_TargetIdleThink );

		weapon->PutAway();

		if ( gi.Anim_NumForName( weapon->edict->s.modelindex, "putaway" ) != -1 )
			weapon->SetAnim( "putaway", EV_Weapon_DonePutaway );
		else
			weapon->PostEvent( EV_Weapon_DonePutaway, 0.0f );
	}
}


//--------------------------------------------------------------
//
// Name:			useWeapon
// Class:			Player
//
// Description:		Find the weapon by name and use it in the specifed
//					hand.
//
// Parameters:		const char *weaponname -- name of the weapon to use
//					weaponhand_t hand -- Hand to use it in
//
// Returns:			None
//
//--------------------------------------------------------------
bool Player::useWeapon( const char *weaponname, weaponhand_t hand )
{
	Weapon *weapon;
	str name(weaponname);

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( gpm )
	{
		str objectName("CurrentPlayer.");
		objectName += name ;
		if ( gpm->hasProperty( objectName, "name" ) )
		{
			name = gpm->getStringValue( objectName, "name" );
		}
	}

	weapon = ( Weapon * )FindItem( name );

	// Check to see if player has the weapon
	if ( !weapon )
	{
		warning( "Player::useWeapon", "Player does not have weapon %s", weaponname );
		return false;
	}

	return useWeapon( weapon, hand );
}

bool Player::useWeapon( Weapon *weapon, weaponhand_t hand )
{
	Weapon * activeWeapon;
	
	if ( !weapon )
	{
		warning( "Player::useWeapon", "Null weapon used.\n" );
		return false;
	}
	
	// Check to see if we are already in the process of using a new weapon.
	//if ( newActiveWeapon.weapon )
	//{
	//	return false;
	//}
	
	// Check to see if weapon has ammo and if useNoAmmo is allowed
	if ( !weapon->HasAmmo( FIRE_MODE1 ) && !weapon->HasAmmo( FIRE_MODE2 ) && !weapon->GetUseNoAmmo() )
	{
		Sound( "snd_noammo" );
		return false;
	}
	
	// Check to see if the hand is allowed to have that weapon
	
	// WEAPON_ANY can be used in WEAPON_LEFT or WEAPON_RIGHT but not as a WEAPON_DUAL, so check for that first
	if ( ( hand == WEAPON_DUAL ) && ( weapon->GetHand() != hand ) )
    {
		warning( "Player::useWeapon", "Weapon %s is not allowed in %s", weapon->getName().c_str(), WeaponHandNumToName( hand ) );
		return false;
    }
	else if ( ( weapon->GetHand() != WEAPON_ANY ) && ( weapon->GetHand() != hand ) )
    {
		warning( "Player::useWeapon", "Weapon %s is not allowed in %s", weapon->getName().c_str(), WeaponHandNumToName( hand ) );
		return false;
    }
	
	// If the weapon we are wielding is a WEAPON_DUAL, then put away the left and right ones
	if ( weapon->GetHand() == WEAPON_DUAL )
	{
		activeWeapon = GetActiveWeapon( WEAPON_LEFT );
		if ( activeWeapon )
			activeWeapon->PutAway();
		activeWeapon = GetActiveWeapon( WEAPON_RIGHT );
		if ( activeWeapon )
			activeWeapon->PutAway();
	}
	
	// Check to see if a WEAPON_DUAL is being used and put it away if needed
	activeWeapon = GetActiveWeapon( WEAPON_DUAL );
	
	if ( activeWeapon && activeWeapon != weapon )
	{
		activeWeapon->PutAway();
		// we just want to put the dual handed weapon away
		if ( activeWeapon == weapon )
		{
			return false;
		}
	}
	
	// Now get the active weapon in the specified hand
	activeWeapon = GetActiveWeapon( hand );
	
	// Check to see if this weapon is already being used in this hand and just put it away and return
	if ( ( activeWeapon == weapon ) && ( !newActiveWeapon.weapon || newActiveWeapon.weapon == weapon ) )
	{
		// Set the putaway flag to true.  The state machine will then play the correct animation to put away the active weapon
		//activeWeapon->PutAway();
		return true;
	}
	
	// If activeWeapon is set, and it's not == weapon then put away this weapon
	/* 
	if ( activeWeapon )
	{
		// Set the putaway flag to true.  The state machine will then play the correct animation to put away the active weapon
		activeWeapon->PutAway();
	} 
	*/
	
	// Check to see if this weapon is being used in a different hand and put it away as well (if it's in a different hand)
	if ( IsActiveWeapon( weapon ) )
	{
		weapon->PutAway();
	}
	
	// Set the newActiveWeapon as the weapon specified, the state machine will play the appropriate animation and
	// trigger when to attach it to the player model.
	newActiveWeapon.weapon = weapon;
	newActiveWeapon.hand   = hand;
	return true;
}

//====================
//ActivateShield
//====================
void Player::ActivateShield( Event * )
{
	shield_active = true;
}

//====================
//DeactivateShield
//====================
void Player::DeactivateShield( Event * )
{
	shield_active = false;
}

//====================
//ShieldActive
//====================
qboolean Player::ShieldActive( void )
{
	return shield_active;
}

//====================
//LargeShieldActive
//====================
qboolean Player::LargeShieldActive( void )
{
	Weapon *weapon;
	qboolean large_shield_active=false;
	
	weapon = GetActiveWeapon( WEAPON_LEFT );
	if ( weapon && !str::icmp( weapon->item_name, "LargeShield" ) )
		large_shield_active = true;
	
	return shield_active && large_shield_active;
}

void Player::AcquireHeadTarget( void )
{
	vec3_t mat[3];
	Entity *new_head_target;
	Entity *ent;
	Item	 *item;
	
	// Find a good target
	
	if ( targetEnemy )
		head_target = targetEnemy;
	else
	{
		AnglesToAxis( headAngles, mat );
		
		// Make sure not to look at items too long
		
		if ( ( look_at_time <= level.time ) && head_target && head_target->isSubclassOf( Item ) )
		{
			item = (Item *)(Entity *)head_target;
			item->has_been_looked_at = true;
		}
		
		// Get the new head target
		new_head_target = FindHeadTarget( this->centroid, mat[0], 160.0f, 1000.0f );
		
		if ( !new_head_target )
		{
			head_target = NULL;
			return;
		}
		
		if ( new_head_target != head_target )
		{
			// If we were looking at an item and are not now, mark it as has been looked at
			
			if ( head_target && head_target->isSubclassOf( Item ) )
			{
				item = (Item *)(Entity *)head_target;
				item->has_been_looked_at = true;
			}
			
			// Set up new head target
			
			head_target = new_head_target;
			
			look_at_time = level.time + 5.0f;
			
			// Mark items near this one as looked at
			
			if ( head_target && head_target->isSubclassOf( Item ) )
			{
				ent = findradius( NULL, head_target->origin, 50.0f );
				
				while( ent )
				{
					if ( ent != head_target && ent->isSubclassOf( Item ) )
					{
						item = (Item *)ent;
						item->has_been_looked_at = true;
					}
					
					ent = findradius( ent, head_target->origin, 50.0f );
				}
			}
		}
	}
}
