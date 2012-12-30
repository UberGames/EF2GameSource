//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/dispenser.cpp                             $
// $Revision:: 20                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// DESCRIPTION:
// This is all the dispensing type objects in the world.  They basically work
// by the player using them.  As the player uses them they give the player
// something (health/ammo/etc.).
//

#include "_pch_cpp.h"
#include "dispenser.hpp"
#include "sentient.h"
#include "player.h"

Event EV_Dispenser_SetMaxAmount
(
	"dispenser_maxamount",
	EV_DEFAULT,
	"f",
	"maxAmount",
	"Sets the total amount that this dispenser can give out."
);
Event EV_Dispenser_SetDispenseRate
(
	"dispenser_rate",
	EV_DEFAULT,
	"f",
	"dispenseRate",
	"Sets the dispense rate (how many units per second to give out)."
);
Event EV_Dispenser_SetRegenRate
(
	"dispenser_regenrate",
	EV_DEFAULT,
	"f",
	"regenRate",
	"Sets the regen rate (how many units per second to regen)."
);
Event EV_Dispenser_SetType
(
	"dispenser_type",
	EV_DEFAULT,
	"s",
	"type",
	"Sets the type of thing to give out.\n"
	"Only health and ammo are valid currently."
);
Event EV_Dispenser_SetSubtype
(
	"dispenser_subtype",
	EV_DEFAULT,
	"s",
	"subtype",
	"Sets the subtype of thing to give out.\n"
	"There is no subtype for health, the subtype for ammo is the ammo type."
);
Event EV_Dispenser_SetOpenDistance
(
	"dispenser_openDistance",
	EV_DEFAULT,
	"f",
	"openDistance",
	"Sets the distance from the player that the dispenser will open/close\n"
);
Event EV_Dispenser_SetInstant
(
	"dispenser_instant",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the dispenser give out its stuff instantly\n"
);
Event EV_Dispenser_AnimDone
(
	"dispenser_animdone",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the dispenser's animation is done"
);
Event EV_Dispenser_SetSoundName
(
	"dispenser_soundName",
	EV_DEFAULT,
	"s",
	"soundName",
	"Makes the dispenser use the specified looping sound when its being used\n"
);

//------------------------- CLASS ------------------------------
//
// Name:		Dispenser
// Base Class:	Entity
//
// Description:	This is an entity that dispenses stuff (health, ammo, etc) 
//              to anything that uses it
//
// Method of Use:	Just like any entity.
//
//--------------------------------------------------------------

CLASS_DECLARATION( Entity, Dispenser, NULL )
{
	{ &EV_Dispenser_SetMaxAmount,		&Dispenser::setMaxAmount },
	{ &EV_Dispenser_SetDispenseRate,	&Dispenser::setDispenseRate },
	{ &EV_Dispenser_SetRegenRate,		&Dispenser::setRegenRate },
	{ &EV_Dispenser_SetType,			&Dispenser::setType },
	{ &EV_Dispenser_SetSubtype,			&Dispenser::setSubtype },
	{ &EV_Dispenser_SetOpenDistance,	&Dispenser::setOpenDistance },
	{ &EV_Dispenser_SetInstant,			&Dispenser::setInstant },
	{ &EV_Dispenser_AnimDone,			&Dispenser::animDone },
	{ &EV_Dispenser_SetSoundName,		&Dispenser::setSoundName },
	{ &EV_Use,							&Dispenser::useEvent },

	{ NULL, NULL }
};

//----------------------------------------------------------------
// Name:			Dispenser
// Class:			Dispenser
//
// Description:		constructor
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------

Dispenser::Dispenser()
{
	animate = new Animate( this );

	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_BBOX );
	setContents( CONTENTS_BODY );

	_maxAmount = 0;
	_amount    = 0;

	_dispenseRate = 0;
	_regenRate    = 0;

	_dispenseType = DISPENSE_TYPE_NONE;

	_lastTimeUsed = 0;

	edict->s.eType = ET_MODELANIM;

	_openDistance = 200.0f;

	if ( !LoadingSavegame )
	{
		Event *event = new Event( EV_Anim );
		event->AddString( "closed_idle" );
		PostEvent( event, 0.0f );
	}

	_state = DISPENSER_STATE_CLOSED;

	_instant = false;

	_entityDispensingTo = NULL;

	turnThinkOn();

	_playingSound = false;
	_lastSoundTime = 0.0f;
}

//----------------------------------------------------------------
// Name:			Think
// Class:			Dispenser
//
// Description:		Does everything the dispenser needs to do for this frame
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::Think( void )
{
	bool playerNearby;
	Player *player;
	Vector dir;
	float distance;

	if ( _playingSound && level.time > _lastSoundTime + 0.25f )
	{
		StopLoopSound();
		_playingSound = false;
	}

	if ( _entityDispensingTo )
	{
		bool dispensedStuff;

		dispensedStuff = dispenseStuff( _entityDispensingTo );

		if ( dispensedStuff )
			return;
		else
		{
			_entityDispensingTo = NULL;
		}
	}

	// Regenerate if we are suppose to 

	if ( _regenRate > 0.0f )
	{
		_amount += _regenRate * level.frametime;

		if ( _amount > _maxAmount )
		{
			_amount = _maxAmount;
		}

		setDispenserAlpha();
	}

	// Do necessary animation stuff

	playerNearby = false;

	if ( _state != DISPENSER_STATE_EMPTY )
	{
		// Get the player

		player = (Player *)g_entities[ 0 ].entity;

		// See if the player is nearby

		if ( player )
		{
			dir = player->origin - origin;
			distance = dir.length();

			if ( distance < _openDistance )
			{
				playerNearby = true;
			}
		}
	}

	if ( _state == DISPENSER_STATE_CLOSED )
	{
		if ( playerNearby )
		{
			animate->RandomAnimate( "opening", EV_Dispenser_AnimDone );
			_state = DISPENSER_STATE_OPENING;
		}
	}
	else if ( _state == DISPENSER_STATE_OPEN )
	{
		if ( !playerNearby )
		{
			animate->RandomAnimate( "closing", EV_Dispenser_AnimDone );
			_state = DISPENSER_STATE_CLOSING;
		}
	}
}

//----------------------------------------------------------------
// Name:			useEvent
// Class:			Dispenser
//
// Description:		This is what is called when the dispenser is used by something.
//					This function dispenses whatever it needs to here.
//
// Parameters:		Event *ev		- event (contains using entity)
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::useEvent( Event *ev )
{
	Sentient *sent;
	Entity *ent;


	// Make sure we should bother trying to give something out

	if ( _amount <= 0 )
		return;

	if ( _lastTimeUsed == level.time )
		return;

	//if ( _state != DISPENSER_STATE_OPEN )
	//	return;

	// Get the entity that is using us

	ent = ev->GetEntity( 1 );

	if ( !ent->isSubclassOf( Sentient ) )
		return;

	sent = (Sentient *)ent;

	if ( _instant )
	{
		_entityDispensingTo = sent;
	}
	else
	{
		dispenseStuff( sent );
	}

	_lastTimeUsed = level.time;
}

bool Dispenser::dispenseStuff( Sentient *sent )
{
	DispenseType currentDispenseType = DISPENSE_TYPE_HEALTH;
	float maxAmountToGive;
	float amountToGive;
	float entityMaxAmount = 0.0f;
	float entityCurrentAmount = 0.0f;

	if ( !_playingSound )
	{
		LoopSound( _useSoundName );
		_playingSound = true;
	}

	_lastSoundTime = level.time;

	// Get the max amount/current amount of this type of thing for this entity

	if ( _dispenseType == DISPENSE_TYPE_HEALTH )
	{
		entityMaxAmount     = sent->getMaxHealth();
		entityCurrentAmount = sent->getHealth();

		currentDispenseType = DISPENSE_TYPE_HEALTH;

		if ( entityCurrentAmount == entityMaxAmount )
		{
			entityMaxAmount     = 100.0f;
			entityCurrentAmount = sent->GetArmorValue();

			currentDispenseType = DISPENSE_TYPE_ARMOR;
		}
	}
	else if ( _dispenseType == DISPENSE_TYPE_AMMO )
	{
		entityMaxAmount     = sent->MaxAmmoCount( _dispenseSubtype );
		entityCurrentAmount = sent->AmmoCount( _dispenseSubtype );

		currentDispenseType = DISPENSE_TYPE_AMMO;
	}

	// Calculate the amount to give

	maxAmountToGive = _dispenseRate * level.frametime;

	if ( _dispenseType == DISPENSE_TYPE_AMMO )
	{
		amountToGive = maxAmountToGive;
	}
	else
	{
		if ( maxAmountToGive > entityMaxAmount - entityCurrentAmount )
			amountToGive = entityMaxAmount - entityCurrentAmount;
		else
			amountToGive = maxAmountToGive;
	}

	if ( amountToGive > _amount )
		amountToGive = _amount;

	if ( amountToGive < 0.0f )
		amountToGive = 0.0f;

	// Give the calculated amount to the entity

	if ( amountToGive )
	{
		if ( currentDispenseType == DISPENSE_TYPE_HEALTH )
		{
			sent->setHealth( entityCurrentAmount + amountToGive );
		}
		else if ( currentDispenseType == DISPENSE_TYPE_ARMOR )
		{
			Event *armorEvent = new Event( EV_Sentient_GiveArmor );
			armorEvent->AddString( "BasicArmor" );
			armorEvent->AddFloat( amountToGive );
			sent->ProcessEvent( armorEvent );
		}
		else if ( currentDispenseType == DISPENSE_TYPE_AMMO )
		{
			int ammoGiven;
			ammoGiven = (int) sent->GiveAmmo( _dispenseSubtype, (int)amountToGive, false );
			amountToGive = ammoGiven;
		}
	}

	// Use up our amount

	_amount -= amountToGive;

	setDispenserAlpha();

	// See if we are empty now

	if ( ( _amount <= 0.0f ) && ( _regenRate == 0.0f ) )
	{
		animate->RandomAnimate( "emptying", EV_Dispenser_AnimDone );
		_state = DISPENSER_STATE_EMPTYING;
	}

	if ( amountToGive > 0 )
		return true;
	else
		return false;
}

//----------------------------------------------------------------
// Name:			setMaxAmount
// Class:			Dispenser
//
// Description:		Sets the maximum amount that we can dispense. It
//					also sets the current amount to the maximum amount.
//
// Parameters:		Event *ev		- event (contains max amount)
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::setMaxAmount( Event *ev )
{
	_maxAmount = ev->GetFloat( 1 );

	_amount = _maxAmount;
}

//----------------------------------------------------------------
// Name:			setDispenseRate
// Class:			Dispenser
//
// Description:		Sets the rate that we will dispense stuff in units per second.
//
// Parameters:		Event *ev		- event (contains dispense rate)
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::setDispenseRate( Event *ev )
{
	_dispenseRate = ev->GetFloat( 1 );
}

//----------------------------------------------------------------
// Name:			setRegenRate
// Class:			Dispenser
//
// Description:		Sets the regen rate that we will regenerate our 
//					amount we can give out.  Only does this is below its maximum
//					amount.  This is also in units per second.
//
// Parameters:		Event *ev		- event (contains regen rate)
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::setRegenRate( Event *ev )
{
	_regenRate = ev->GetFloat( 1 );
}

//----------------------------------------------------------------
// Name:			setType
// Class:			Dispenser
//
// Description:		Sets the type of stuff we are giving out (health, ammo, etc.)
//
// Parameters:		Event *ev		- event (contains type)
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::setType( Event *ev )
{
	str typeName;

	typeName = ev->GetString( 1 );

	if ( typeName == "health" )
	{
		_dispenseType = DISPENSE_TYPE_HEALTH;
	}
	else if ( typeName == "ammo" )
	{
		_dispenseType = DISPENSE_TYPE_AMMO;
	}
	else
	{
		gi.WDPrintf( "Unknown dispense type %s\n", typeName.c_str() );
		_dispenseType = DISPENSE_TYPE_NONE;
	}
}

//----------------------------------------------------------------
// Name:			setSubtype
// Class:			Dispenser
//
// Description:		Sets the subtype of the thing we are giving out.
//					For ammo - this is the ammo type
//
// Parameters:		Event *ev		- event (contains subtype)
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::setSubtype( Event *ev )
{
	_dispenseSubtype = ev->GetString( 1 );
}

//----------------------------------------------------------------
// Name:			setOpenDistance
// Class:			Dispenser
//
// Description:		Sets the distance from the player that the dispenser will open/close
//
// Parameters:		Event *ev		- event (contains open distance)
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::setOpenDistance( Event *ev )
{
	_openDistance = ev->GetFloat( 1 );
}

//----------------------------------------------------------------
// Name:			animDone
// Class:			Dispenser
//
// Description:		Called when on of the dispenser's animation is done playing
//
// Parameters:		Event *						- not used
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::animDone( Event * )
{
	// The animation is done playing so, move the dispenser to the correct state and play the appropriate animation

	if ( _state == DISPENSER_STATE_CLOSING )
	{
		animate->RandomAnimate( "closed_idle" );
		_state = DISPENSER_STATE_CLOSED;
	}
	else if ( _state == DISPENSER_STATE_OPENING )
	{
		animate->RandomAnimate( "open_idle" );
		_state = DISPENSER_STATE_OPEN;
	}
	else if ( _state == DISPENSER_STATE_EMPTYING )
	{
		animate->RandomAnimate( "empty_idle" );
		_state = DISPENSER_STATE_EMPTY;
	}
}

//----------------------------------------------------------------
// Name:			setDispenserAlpha
// Class:			Dispenser
//
// Description:		Sets the alpha of the dispenser based on how much amount is left
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::setDispenserAlpha( void )
{
	float percentLeft;

	if ( _maxAmount )
	{
		percentLeft = _amount / _maxAmount;

		setAlpha( percentLeft );
		G_SetConstantLight( &edict->s.constantLight, &percentLeft, &percentLeft, &percentLeft, 0 );
	}
}

//----------------------------------------------------------------
// Name:			setInstant
// Class:			Dispenser
//
// Description:		Sets this dispenser to give out stuff instant (only have to hit use once)
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------

void Dispenser::setInstant( Event *ev )
{
	_instant = qtrue;
}

void Dispenser::setSoundName( Event *ev )
{
	_useSoundName = ev->GetString( 1 );
}
