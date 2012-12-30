//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/ammo.cpp                                  $
// $Revision:: 15                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:35p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Base class for all ammunition for entities derived from the Weapon class.
//
// AmmoEntity is the Class which represents ammo that the player "sees" and "
// picks up" in the game
//
// Ammo is the Class which is used to keep track of how much ammo a player has
// in his inventory

#include "_pch_cpp.h"
#include "ammo.h"
#include "player.h"

CLASS_DECLARATION( Item, AmmoEntity, NULL )
{
	{ NULL, NULL }
};

AmmoEntity::AmmoEntity()
{
	if ( LoadingSavegame )
	{
		// all data will be setup by the archive function
		return;
	}
	setName( "UnknownAmmo" );
	amount       = 0;

	_lastPrintTime = 0.0f;
}

Item *AmmoEntity::ItemPickup( Entity *other, qboolean add_to_inventory, qboolean )
{
	Sentient *player;
	str      realname;
	int amountUsed;
	
	if ( !other->isSubclassOf( Player ) )
		return NULL;

	if ( !Pickupable( other ) )
		return NULL;
	
	player = ( Sentient * )other;

	// Give the ammo to the player
	amountUsed = player->GiveAmmo( item_name, (int) amount, true );

	if ( amountUsed == 0 )
	{
		if ( level.time > _lastPrintTime + 1.0f )
		{
			_lastPrintTime = level.time;
			((Player *)other)->setItemText( getIcon(), va( "$$CouldNotPickUp$$ $$Ammo-%s$$ $$FullAmmo$$", item_name.c_str() ) );
			//gi.centerprintf ( other->edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$CouldNotPickUp$$ %s $$FullAmmo$$", item_name.c_str() );
		}

		return NULL;
	}
	
	// Play pickup sound
	realname = GetRandomAlias( "snd_pickup" );
	if ( realname.length() > 1 )
		player->Sound( realname, CHAN_ITEM );
	
	// Cancel some events
	CancelEventsOfType( EV_Item_DropToFloor );
	CancelEventsOfType( EV_Item_Respawn );
	CancelEventsOfType( EV_FadeOut );
	
	// Hide the model
	setSolidType( SOLID_NOT );

	if ( _missingSkin )
	{
		ChangeSkin( _missingSkin, true );
	}
	else
	{
		hideModel();
	}
	
	// Respawn?
	if ( !Respawnable() )
		PostEvent( EV_Remove, FRAMETIME );
	else
		PostEvent( EV_Item_Respawn, RespawnTime() );
	
	// fire off any pickup_thread's
	if ( pickup_thread.length() )
	{
		ExecuteThread( pickup_thread );
	}
	
	return NULL; // This doesn't create any items
}

void AmmoEntity::cacheStrings( void )
{
	G_FindConfigstringIndex( va( "$$CouldNotPickUp$$ $$Ammo-%s$$ $$FullAmmo$$", item_name.c_str() ), CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true );
	G_FindConfigstringIndex( va( "$$PickedUp$$ %d $$Ammo-%s$$\n", (int)amount, item_name.c_str() ), CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true );	
}

// This is the Class that is used to keep track of ammo in the player's inventory.
// It is not an entity, just a name and an amount.

CLASS_DECLARATION( Class, Ammo, NULL )
{
	{NULL, NULL}
};

Ammo::Ammo()
{
	if ( LoadingSavegame )
	{
		// all data will be setup by the archive function
		return;
	}
	setName( "UnknownAmmo" );
	setAmount( 0 );
	setMaxAmount( 100 );
}

void Ammo::setAmount( int a )
{
	amount = a;
	
	if ( ( maxamount > 0 ) && ( amount > maxamount ) )
		amount = maxamount;
}

int Ammo::getAmount( void )
{
	return amount;
}

void Ammo::setMaxAmount( int a )
{
	maxamount = a;
}

int Ammo::getMaxAmount( void )
{
	return maxamount;
}

void Ammo::setName( const str &n )
{
	name = n;
	name_index = gi.itemindex( name );
}

str Ammo::getName( void )
{
	return name;
}

int Ammo::getIndex( void )
{
	return name_index;
}
