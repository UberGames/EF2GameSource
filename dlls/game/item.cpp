//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/item.cpp                                  $
// $Revision:: 56                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Base class for respawnable, carryable objects.
//

#include "_pch_cpp.h"
#include "entity.h"
#include "trigger.h"
#include "item.h"
#include "inventoryitem.h"
#include "scriptmaster.h"
#include "health.h"
#include "mp_manager.hpp"
#include <qcommon/gameplaymanager.h>


// for bot code
#include "g_local.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_ea.h"
#include "be_ai_char.h"
#include "be_ai_chat.h"
#include "be_ai_gen.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"
#include "be_ai_weap.h"
#include "ai_main.h"
extern bot_state_t	*botstates[MAX_CLIENTS];


Event EV_Item_Pickup
(
	"item_pickup",
	EV_CODEONLY,
	"e",
	"item",
	"Pickup the specified item."
);
Event EV_Item_DropToFloor
(
	"item_droptofloor",
	EV_CODEONLY,
	NULL,
	NULL,
	"Drops the item to the ground."
);
Event EV_Item_Respawn
(
	"respawn",
	EV_CODEONLY,
	NULL,
	NULL,
	"Respawns the item."
);
Event EV_Item_SetRespawn
(
	"set_respawn",
	EV_DEFAULT,
	"i",
	"respawn",
	"Turns respawn on or off."
);
Event EV_Item_SetRespawnTime
(
	"set_respawn_time",
	EV_DEFAULT,
	"f",
	"respawn_time",
	"Sets the respawn time."
);
Event EV_Item_SetAmount
(
	"amount",
	EV_DEFAULT,
	"i",
	"amount",
	"Sets the amount of the item."
);
Event EV_Item_SetMaxAmount
(
	"maxamount",
	EV_DEFAULT,
	"i",
	"max_amount",
	"Sets the max amount of the item."
);
Event EV_Item_SetItemName
(
	"name",
	EV_DEFAULT,
	"s",
	"item_name",
	"Sets the item name."
);
Event EV_Item_RespawnSound
(
	"respawnsound",
	EV_DEFAULT,
	NULL,
	NULL,
	"Turns on the respawn sound for this item."
);
Event EV_Item_DialogNeeded
(
	"dialogneeded",
	EV_DEFAULT,
	"s",
	"dialog_needed",
	"Sets the dialog needed string."
);
Event EV_Item_NoRemove
(
	"no_remove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes it so the item is not removed from the world when it is picked up."
);
Event EV_Item_RespawnDone
(
	"respawn_done",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the item respawn is done."
);
Event EV_Item_PickupDone
(
	"pickup_done",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the item pickup is done."
);
Event EV_Item_SetPickupThread
(
	"pickup_thread",
	EV_SCRIPTONLY,
	"s",
	"labelName",
	"A thread that is called when an item is picked up."
);
Event EV_Item_SetBotInventoryIndex
(
	"bot_inventory",
	EV_TIKIONLY,
	"i",
	"botInventoryIndex",
	"sets the index used for bot inventory pickups"
);
Event EV_Item_CoolItem
(
	"coolitem",
	EV_DEFAULT,
	"SS",
	"dialog anim_to_play",
	"Specify that this is a cool item when we pick it up for the first time.\n"
	"If dialog is specified, than the dialog will be played during the pickup.\n"
	"If anim_to_play is specified, than the specified anim will be played after\n"
	"the initial cinematic."
);
Event EV_Item_ForceCoolItem
(
	"forcecoolitem",
	EV_DEFAULT,
	"SS",
	"dialog anim_to_play",
	"Specify that this is a cool item when we pick it up regardless of whether or not we have it.\n"
	"If dialog is specified, than the dialog will be played during the pickup.\n"
	"If anim_to_play is specified, than the specified anim will be played after\n"
	"the initial cinematic."
);
Event EV_Item_IconName
(
	"iconName",
	EV_DEFAULT,
	"s",
	"iconName",
	"Sets the name of the icon to use for this item."
);
Event EV_Item_SetMissingSkin
(
	"missingSkin",
	EV_TIKIONLY,
	"i",
	"skinNum",
	"Sets the skin number to use when its been picked up and is missing"
);
Event EV_Item_PostSpawn
(
	"itemPostSpawn",
	EV_CODEONLY,
	NULL,
	NULL,
	"Tells the item that it had spawned successfully"
);

CLASS_DECLARATION( Trigger, Item, NULL )
{
	{ &EV_Trigger_Effect,				&Item::ItemTouch },
    { &EV_Item_DropToFloor,				&Item::DropToFloor },
	{ &EV_Item_Respawn,					&Item::Respawn },
	{ &EV_Item_SetAmount,				&Item::SetAmountEvent },
	{ &EV_Item_SetMaxAmount,			&Item::SetMaxAmount },
	{ &EV_Item_SetItemName,				&Item::SetItemName },
	{ &EV_Item_Pickup,					&Item::Pickup },
	{ &EV_Use,							&Item::TriggerStuff },
	{ &EV_Item_RespawnSound,			&Item::RespawnSound },
	{ &EV_Item_DialogNeeded,			&Item::DialogNeeded },
	{ &EV_Item_NoRemove,				&Item::SetNoRemove },
	{ &EV_Item_RespawnDone,				&Item::RespawnDone },
	{ &EV_Item_PickupDone,				&Item::PickupDone },
	{ &EV_Item_SetRespawn,				&Item::setRespawn },
	{ &EV_Item_SetRespawnTime,			&Item::setRespawnTime },
	{ &EV_Item_SetPickupThread,			&Item::SetPickupThread },
	{ &EV_Item_CoolItem,				&Item::CoolItemEvent },
	{ &EV_Item_ForceCoolItem,			&Item::ForceCoolItemEvent },
	{ &EV_Item_SetBotInventoryIndex,	&Item::SetBotInventoryIndex },
	{ &EV_Stop,							&Item::Landed },
	{ &EV_SetAngle,						&Item::SetAngleEvent },
	{ &EV_Item_IconName,				&Item::iconNameEvent },
	{ &EV_Item_SetMissingSkin,			&Item::setMissingSkin },
	{ &EV_Item_PostSpawn,				&Item::postSpawn },
	{ NULL, NULL }
};

//--------------------------------------------------------------
// Name:		Item()
// Class:		Item
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
Item::Item()
{
	str fullname;
	animate = new Animate( this );

	if ( LoadingSavegame )
		return;

	setSolidType( SOLID_NOT );

	// Set default respawn behavior
	// Derived classes should use setRespawn
	// if they want to override the default behavior
	setRespawn( false );
	setRespawnTime( 20 );

	if ( multiplayerManager.inMultiplayer() )
	{
		setRespawn( true );
		edict->s.renderfx |= RF_FULLBRIGHT;
	}

	//
	// we want the bounds of this model auto-rotated
	//
	flags |= FlagRotatedbounds;

	//
	// set a minimum mins and maxs for the model
	//
	if ( size.length() < 10.0f )
	{
		mins = Vector(-10, -10, 0);
		maxs = Vector(10, 10, 20);
	}

	//
	// reset the mins and maxs to pickup the FlagRotatedbounds flag
	//
	setSize( mins, maxs );

	if ( !LoadingSavegame )
	{
		// Items can't be immediately dropped to floor, because they might
		// be on an entity that hasn't spawned yet.
		PostEvent( EV_Item_DropToFloor, EV_POSTSPAWN );
	}

	respondto = TRIGGER_PLAYERS;

	// items should collide with everything that the player does
	edict->clipmask	 = MASK_PLAYERSOLID;

	bot_inventory_index = 0; // INVENTORY_NONE
	item_index = 0;
	maximum_amount = 1.0f;
	playrespawn = false;

	// this is an item entity

	if ( g_gametype->integer == GT_SINGLE_PLAYER )
		edict->s.eType = ET_MODELANIM;
	else
		edict->s.eType = ET_ITEM;

	// Set our default skill level 
	_skillLevel = 1.0f;
	amount = 1.0f;
	no_remove = false;
	setName( "Unknown Item" );

	look_at_me = true;
	coolitem = false;
	coolitemforced = false;

	has_been_looked_at = false;

	_nextPickupTime = 0.0f;

	_mpItemType = MP_ITEM_TYPE_NORMAL;

	_iconIndex = -1;

	_missingSkin = 0;

	if ( !LoadingSavegame )
	{
		PostEvent( EV_Item_PostSpawn, EV_POSTSPAWN );
	}
}

Item::~Item()
{
	if ( owner )
	{
		owner->RemoveItem( this );
		owner = NULL;
	}
}

void Item::postSpawn( Event * )
{
	cacheStrings();
}

void Item::SetNoRemove( Event * )
{
	no_remove = true;
}

/*
============
PlaceItem

Puts an item back in the world
============
*/
void Item::PlaceItem( void )
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( gpm->hasProperty(getArchetype(), "noautopickup") )
	{
		setContents( CONTENTS_USABLE );
		setSolidType( SOLID_BBOX );
	}
	else
	{
		setSolidType( SOLID_TRIGGER );
	}
	
	setMoveType( MOVETYPE_TOSS );
	showModel();
	
	groundentity = NULL;
}

/*
============
DropToFloor

plants the object on the floor
============
*/
void Item::DropToFloor( Event * )
{
	str fullname;
	Vector save;
	
	PlaceItem();
	
	addOrigin( Vector(0, 0, 1) );
	
	save = origin;
	
	if ( gravity > 0.0f )
	{
		if ( !droptofloor( 8192.0f ) )
		{
			gi.WDPrintf( "%s (%d) stuck in world at '%5.1f %5.1f %5.1f'\n",
				getClassID(), entnum, origin.x, origin.y, origin.z );
			setOrigin( save );
			setMoveType( MOVETYPE_NONE );
		}
		else
		{
			setMoveType( MOVETYPE_NONE );
		}
	}
	
	//
	// if the our global variable doesn't exist, lets zero it out
	//
	fullname = str( "playeritem_" ) + getName();
	if ( !gameVars.VariableExists( fullname.c_str() ) )
	{
		gameVars.SetVariable( fullname.c_str(), 0 );
	}
	
	if ( !levelVars.VariableExists( fullname.c_str() ) )
	{
		levelVars.SetVariable( fullname.c_str(), 0 );
	}

	if ( multiplayerManager.inMultiplayer() && gi.Anim_NumForName( edict->s.modelindex, "idle_onground" ) >= 0 )
	{
		animate->RandomAnimate( "idle_onground" );
		edict->s.eType = ET_MODELANIM;
	}
}

qboolean Item::Drop( void )
{
	if ( !owner )
	{
		return false;
	}
	
	setOrigin( owner->origin + Vector( "0 0 40" ) );
	
	// drop the item
	PlaceItem();
	velocity = owner->velocity * 0.5f + Vector( G_CRandom( 50.0f ), G_CRandom( 50.0f ), 100.0f );
	setAngles( owner->angles );
	avelocity = Vector( 0.0f, G_CRandom( 360.0f ), 0.0f );
	
	trigger_time = level.time + 1.0f;
	
	if ( owner->isClient() )
	{
		spawnflags |= DROPPED_PLAYER_ITEM;
	}
	else
	{
		spawnflags |= DROPPED_ITEM;
	}
	
	// Remove this from the owner's item list
	owner->RemoveItem( this );
	owner = NULL;
	
	return true;
}


void Item::ItemTouch( Event *ev )
{
	Entity	*other;
	Event		*e;
	
	if ( owner )
	{
		// Don't respond to trigger events after item is picked up.
		// we really don't need to see this.
		//gi.DPrintf( "%s with targetname of %s was triggered unexpectedly.\n", getClassID(), TargetName() );
		return;
	}
	
	other = ev->GetEntity( 1 );
	
	e = new Event( EV_Item_Pickup );
	e->AddEntity( other );
	ProcessEvent( e );
}

void Item::SetOwner( Sentient *ent )
{
	assert( ent );
	if ( !ent )
	{
		// return to avoid any buggy behaviour
		return;
	}
	
	owner = ent;
	edict->s.parent = ent->entnum;
	setRespawn( false );
	
	setSolidType( SOLID_NOT );
	hideModel();
	CancelEventsOfType( EV_Touch );
	CancelEventsOfType( EV_Item_DropToFloor );
	CancelEventsOfType( EV_Remove );
	//	ItemPickup( ent );
}

void Item::SetBotInventoryIndex( Event *ev )
{
	bot_inventory_index = ev->GetInteger(1);
}

int Item::GetBotInventoryIndex( void )
{
	return bot_inventory_index;	
}

Sentient *Item::GetOwner( void ) const
{
	return owner;
}

Item * Item::ItemPickup( Entity *other, qboolean add_to_inventory, qboolean checkautopickup )
{
	Sentient * sent;
	Item * item = NULL;
	str realname;
	
	// Query the gameplay manager and see if we should not auto-pickup this item
	if ( checkautopickup )
	{
		GameplayManager *gpm = GameplayManager::getTheGameplayManager();
		if ( gpm->hasProperty(getArchetype(), "noautopickup") )
			return NULL;
	}
	
	if ( !Pickupable( other ) )
	{
		return NULL;
	}
	
	sent = ( Sentient * )other;
	
	if ( add_to_inventory )
	{
		item = sent->giveItem( model, getAmount(), true );
		
		
		if ( !item )
			return NULL;
	}
	else
	{
		item = this;
	}
	
	//
	// make sure to copy over the coolness factor :)
	//
	item->coolitem = coolitem;
	item->cool_dialog = cool_dialog;
	item->cool_anim = cool_anim;
	item->coolitemforced = coolitemforced;
	
	//
	// let our sent know they received it
	// we put this here so we can transfer information from the original item we picked up
	//

	if ( !isSubclassOf( Weapon ) || add_to_inventory )
		sent->ReceivedItem( item );
	
	realname = GetRandomAlias( "snd_pickup" );
	if ( realname.length() > 1 )
		sent->Sound( realname, CHAN_ITEM );
	
	if ( !Removable() )
	{
		// leave the item for others to pickup
		return item;
	}
	
	look_at_me = false;
	
	CancelEventsOfType( EV_Item_DropToFloor );
	CancelEventsOfType( EV_Item_Respawn );
	CancelEventsOfType( EV_FadeOut );
	
	setSolidType( SOLID_NOT );
	
	if ( animate && animate->HasAnim( "pickup" ) )
		animate->RandomAnimate( "pickup", EV_Item_PickupDone );
	else
	{
		if ( !no_remove )
		{
			if ( _missingSkin )
			{
				ChangeSkin( _missingSkin, true );
			}
			else
			{
				hideModel();
			}
			
			if ( !Respawnable() )
				PostEvent( EV_Remove, FRAMETIME );
		}
	}
	
	if ( Respawnable() )
		PostEvent( EV_Item_Respawn, RespawnTime() );
	
	// fire off any pickup_thread's
	if ( pickup_thread.length() )
	{
		ExecuteThread( pickup_thread );
	}
	
	
	if ( item && multiplayerManager.checkFlag( MP_FLAG_INSTANT_ITEMS ) )
	{
		Event *ev;
		
		ev = new Event( EV_InventoryItem_Use );
		ev->AddEntity( other );
		
		item->ProcessEvent( ev );
	}
	
	return item;
}

void Item::Respawn( Event * )
{
	if ( _missingSkin )
	{
		ChangeSkin( _missingSkin, false );
	}
	else
	{
		showModel();
	}
	
	// allow it to be touched again
	setSolidType( SOLID_TRIGGER );
	
	// play respawn sound
	if ( playrespawn )
	{
		Sound( "snd_itemspawn" );
	}
	
	setOrigin();
	
	if ( animate->HasAnim( "respawn" ) )
		animate->RandomAnimate( "respawn", EV_Item_RespawnDone );
	
	look_at_me = true;
	has_been_looked_at = false;
}

void Item::setRespawn( Event *ev )
{
	if ( ev->NumArgs() < 1 )
		return;
	
	setRespawn( ev->GetInteger( 1 ) );
}

void Item::setRespawnTime( Event *ev )
{
	if ( ev->NumArgs() < 1 )
		return;
	
	setRespawnTime( ev->GetFloat( 1 ) );
}

void Item::RespawnDone( Event * )
{
	animate->RandomAnimate( "idle" );
}

void Item::PickupDone( Event * )
{
	if ( !no_remove )
	{
		hideModel();
		
		if ( !Respawnable() )
			PostEvent( EV_Remove, FRAMETIME );
	}
	else
	{
		if ( animate->HasAnim( "pickup_idle" ) )
			animate->RandomAnimate( "pickup_idle" );
		else
			animate->RandomAnimate( "pickup" );
	}
}

void Item::setRespawn( qboolean flag )
{
	respawnable = flag;
}

qboolean Item::Respawnable( void )
{
	return respawnable;
}

void Item::setRespawnTime( float time )
{
	respawntime = time;
}

float Item::RespawnTime( void )
{
	if ( multiplayerManager.inMultiplayer() )
		return respawntime * multiplayerManager.getItemRespawnMultiplayer();
	else
		return respawntime;
}

float Item::getAmount( void )
{
	return amount;
}

float Item::MaxAmount( void )
{
	return maximum_amount;
}

qboolean Item::Pickupable( Entity *other )
{
	if ( level.time < _nextPickupTime )
		return false;

	if ( multiplayerManager.inMultiplayer() && other->isSubclassOf( Player ) )
	{
		if ( !multiplayerManager.canPickup( (Player *)other, getMultiplayerItemType(), getName().c_str() ) )
			return false;
	}

	if ( getSolidType() == SOLID_NOT )
	{
		return NULL;
	}
	
	if ( !other->isSubclassOf( Sentient ) )
	{
		return false;
	}
	else
	{
		Sentient * sent;
		Item * item;
		
		sent = ( Sentient * )other;

		if ( sent->deadflag || sent->health <= 0.0f )
		{
			return false;
		}
		
		item = sent->FindItem( getName() );
		
		if ( item && ( item->getAmount() >= item->MaxAmount() ) )
		{
			return false;
		}
		
		// TODO : fixme
		// If deathmatch and already in a powerup, don't pickup anymore when DF_INSTANT_ITEMS is on
		/* if ( multiplayerManager.checkFlag( MP_FLAG_INSTANT_ITEMS ) &&
		this->isSubclassOf( InventoryItem ) &&
		sent->PowerupActive()
		)
		{
		return false;
		} */
	}
	return true;
}

void Item::Pickup( Event * ev )
{
	ItemPickup( ev->GetEntity( 1 ) );
}

void Item::setName( const char *i )
{
	item_name = i;
	item_index = gi.itemindex( i );
	strcpy( edict->entname, i );
}

int Item::getIndex( void )
{
	return item_index;
}

void Item::setAmount( float startamount )
{
	amount = startamount;
	if ( amount >= MaxAmount() )
	{
		SetMax( (int) amount );
	}
}

void Item::SetMax( int maxamount )
{
	maximum_amount = maxamount;
}

void Item::SetAmountEvent( Event *ev )
{
	setAmount( ev->GetInteger( 1 ) );
}

void Item::SetMaxAmount( Event *ev )
{
	SetMax( ev->GetInteger( 1 ) );
}

void Item::SetItemName( Event *ev )
{
	setName( ev->GetString( 1 ) );
}

void Item::Add( int num )
{
	amount += num;
	if ( amount >= MaxAmount() )
		amount = MaxAmount();
}

void Item::Remove( int num )
{
	amount -= num;
	if (amount < 0)
		amount = 0;
}


qboolean Item::Use( int num )
{
	if ( num > amount )
	{
		return false;
	}
	
	amount -= num;
	return true;
}

qboolean Item::Removable( void )
{
	return true;
}

void Item::RespawnSound( Event * )
{
	playrespawn = true;
}

void Item::DialogNeeded( Event *ev )
{
	//
	// if this item is needed for a trigger, play this dialog
	//
	dialog_needed = ev->GetString( 1 );
}

str Item::GetDialogNeeded( void )
{
	return dialog_needed;
}

//
// once item has landed on the floor, go to movetype none
//
void Item::Landed( Event * )
{
	if ( groundentity && ( groundentity->entity != world ) )
	{
		warning( "Item::Landed", "Item %d has landed on an entity that might move\n", entnum );
	}
	setMoveType( MOVETYPE_NONE );
}

void Item::SetPickupThread( Event *ev )
{
	pickup_thread = ev->GetString( 1 );
}

void Item::SetCoolItem( qboolean cool, const str &dialog, const str &anim )
{
	coolitem = cool;
	cool_dialog = dialog;
	if ( cool_dialog.length() )
	{
		CacheResource( cool_dialog, this );
	}
	
	CacheResource( "models/fx_coolitem.tik", this );
	CacheResource( "models/fx_coolitem_reverse.tik", this );
	
	cool_anim = anim;
}

void Item::CoolItemEvent( Event *ev )
{
	qboolean cool;
	str dialog, anim;
	
	cool = true;
	if ( ev->NumArgs() > 0 )
	{
		dialog = ev->GetString( 1 );
	}
	if ( ev->NumArgs() > 1 )
	{
		anim = ev->GetString( 2 );
	}
	SetCoolItem( cool, dialog, anim );
}

void Item::ForceCoolItemEvent( Event *ev )
{
	coolitemforced = true;
	CoolItemEvent( ev );
}

qboolean Item::IsItemCool( str * dialog, str * anim, qboolean * forced )
{
	*dialog = cool_dialog;
	*anim = cool_anim;
	*forced = coolitemforced;
	return coolitem;
}

MultiplayerItemType Item::getMultiplayerItemType( void )
{
	return _mpItemType;
}

void Item::iconNameEvent( Event *ev )
{
	str iconName;

	iconName = ev->GetString( 1 );

	_iconIndex = gi.imageindex( iconName.c_str() );
}

void Item::setMissingSkin( Event *ev )
{
	_missingSkin = ev->GetInteger( 1 );
}

void Item::ChangeSkin( int skinNum, qboolean state )
{
	str command;
	
	// Build the command
	
	if ( state )
		command = "+";
	else
		command = "-";
	
	command += "skin";
	
	command += skinNum;
	
	// Change the skin
	
	SurfaceCommand( "all", command.c_str() );
}

void Item::cacheStrings( void )
{
	G_FindConfigstringIndex( va( "$$PickedUp$$ %d $$Item-%s$$\n", (int)getAmount(), getName().c_str() ), CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true );
	G_FindConfigstringIndex( va( "$$PickedUp$$ %d $$Item-%s$$s\n", (int)getAmount(), getName().c_str() ), CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true );
	G_FindConfigstringIndex( va( "$$PickedUpThe$$ $$Item-%s$$\n", getName().c_str() ), CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true );
}

CLASS_DECLARATION( Item, MultiplayerItem, NULL )
{
	{ &EV_Trigger_Effect,			&MultiplayerItem::notifyMultiplayerItemTriggered },
	{ &EV_Killed,					&MultiplayerItem::notifyMultiplayerItemDestroyed },
	{ &EV_Use,						&MultiplayerItem::notifyMultiplayerItemUsed },
	{ &EV_Damage,					&MultiplayerItem::damageEvent },

	{ NULL, NULL }
};

MultiplayerItem::MultiplayerItem()
{
	edict->s.eType = ET_MODELANIM;
}

void MultiplayerItem::notifyMultiplayerItemTriggered( Event *ev )
{
	Entity *entity;

	entity = ev->GetEntity( 1 );

	if ( entity->isSubclassOf( Player ) )
		multiplayerManager.itemTouched( (Player *)entity, this );
}

void MultiplayerItem::notifyMultiplayerItemDestroyed( Event *ev )
{
	Entity *entity;

	entity = ev->GetEntity( 1 );

	if ( entity->isSubclassOf( Player ) )
		multiplayerManager.itemDestroyed( (Player *)entity, this );
}

void MultiplayerItem::notifyMultiplayerItemUsed( Event *ev )
{
	Entity *entity;

	entity = ev->GetEntity( 1 );

	multiplayerManager.itemUsed( entity, this );
}

void MultiplayerItem::damageEvent( Event *ev )
{
	Entity *attacker;
	int meansOfDeath;
	float originalDamage;
	float realDamage;

	// Get the parms

	originalDamage = ev->GetFloat( 1 );
	attacker       = ev->GetEntity( 3 );
	meansOfDeath   = ev->GetInteger( 9 );

	// See what the real damage done is

	realDamage = originalDamage;

	if ( attacker->isSubclassOf( Player ) )
	{
		realDamage = multiplayerManager.itemDamaged( this, (Player *)attacker, originalDamage, meansOfDeath );

		// Change the event if the damage has changed

		if ( realDamage != originalDamage )
		{
			ev->SetFloat( 1, realDamage );
		}
	}

	if ( realDamage == 0.0f )
		return;

	// Do the real damage here

	Entity::DamageEvent( ev );
}

void MultiplayerItem::cacheStrings( void )
{
}

CLASS_DECLARATION( Item, SecretItem, NULL )
{
	{ NULL, NULL }
};

SecretItem::SecretItem()
{
	if ( LoadingSavegame )
		return;

	level.total_specialItems++;
	levelVars.SetVariable( "total_specialItems" , level.total_specialItems );

	edict->s.eType = ET_ITEM;
}

Item* SecretItem::ItemPickup( Entity *other, qboolean add_to_inventory = true, qboolean checkautopickup = true )
{
	str pickupSoundName;

	Q_UNUSED(checkautopickup);
	Q_UNUSED(add_to_inventory);

	PostEvent( EV_Remove, 0.0f );

	level.found_specialItems++;
	levelVars.SetVariable( "found_specialItems" , level.found_specialItems );

	gi.centerprintf ( other->edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$FoundSecretItem$$" );

	pickupSoundName = GetRandomAlias( "snd_pickup" );

	if ( pickupSoundName.length() > 1 )
	{
		other->Sound( pickupSoundName, CHAN_ITEM );
	}

	if ( other && other->isSubclassOf( Player ) )
	{
		Player *player = (Player *)other;

		player->incrementSecretsFound();
	}

	return NULL;
}

void SecretItem::cacheStrings( void )
{
}
