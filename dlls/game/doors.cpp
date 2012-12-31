//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/doors.cpp                                 $
// $Revision:: 28                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Doors are environment objects that rotate open when activated by triggers
// or when used by the player.
//

#include "_pch_cpp.h"
#include "entity.h"
#include "trigger.h"
#include "mover.h"
#include "doors.h"
#include "sentient.h"
#include "scriptmaster.h"
#include "item.h"
#include "actor.h"
#include "player.h"
#include "g_utils.h"
#include <qcommon/gameplaymanager.h>

Event EV_Door_StopSound
(
	"sound_stop",
	EV_SCRIPTONLY,
	"s",
	"sound_stop",
	"Sets the sound to use when the door stops."
);
Event EV_Door_MoveSound
(
	"sound_move",
	EV_SCRIPTONLY,
	"s",
	"sound_move",
	"Sets the sound to use when the door moves."
);
Event EV_Door_MessageSound
(
	"sound_message",
	EV_SCRIPTONLY,
	"s",
	"sound_message",
	"Sets the sound to use when the door displays a message."
);
Event EV_Door_LockedSound
(
	"sound_locked",
	EV_SCRIPTONLY,
	"s",
	"sound_locked",
	"Sets the sound to use when the door is locked."
);
Event EV_Door_SetWait
(
	"wait",
	EV_SCRIPTONLY,
	"f",
	"wait",
	"Sets the amount of time to wait before automatically shutting."
);
Event EV_Door_SetDmg
(
	"dmg",
	EV_SCRIPTONLY,
	"i",
	"damage",
	"Sets the amount of damage the door will do to entities that get stuck in it."
);
Event EV_Door_TriggerFieldTouched
(
	"door_triggerfield",
	EV_SCRIPTONLY,
	"e",
	"other",
	"Is called when a doors trigger field is touched."
);
Event EV_Door_TryOpen
(
	"tryToOpen",
	EV_SCRIPTONLY,
	"e",
	"other",
	"Tries to open the door."
);
Event EV_Door_Close
(
	"close",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Closes the door."
);
Event EV_Door_Open
(
	"open",
	EV_SCRIPTONLY,
	"e",
	"other",
	"Opens the door."
);
Event EV_Door_DoClose
(
	"doclose",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Closes the door (special doors)."
);
Event EV_Door_DoOpen
(
	"doopen",
	EV_SCRIPTONLY,
	"e",
	"other",
	"Opens the door (special doors)."
);
Event EV_Door_CloseEnd
(
	"doorclosed",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the door finishes closing."
);
Event EV_Door_OpenEnd
(
	"dooropened",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the door finishes opening."
);
Event EV_Door_Fire
(
	"toggledoor",
	EV_SCRIPTONLY,
	"e",
	"other",
	"Toggles the state of the door (open/close)."
);
Event EV_Door_Link
(
	"linkdoor",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Link doors together."
);
Event EV_Door_SetTime
(
	"time",
	EV_SCRIPTONLY,
	"f",
	"traveltime",
	"Sets the time it takes for the door to open an close."
);
Event EV_Door_Lock
(
	"lock",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Lock the door."
);
Event EV_Door_Unlock
(
	"unlock",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Unlock the door."
);
Event EV_Door_ExtraTriggerSize
(
	"extraTriggerSize",
	EV_SCRIPTONLY,
	"v",
	"extraTriggerSize",
	"Sets the extra trigger size to use for a door, defaults to (0 0 0)."
);

#define DOOR_START_OPEN			1
#define DOOR_OPEN_DIRECTION		2
#define DOOR_DONT_LINK			4
#define DOOR_TOGGLE				32
#define DOOR_AUTO_OPEN			64
#define DOOR_TARGETED 			128

#define STATE_OPEN		1
#define STATE_OPENING	2
#define STATE_CLOSING   3
#define STATE_CLOSED	4

/*

Doors are similar to buttons, but can spawn a fat trigger field around them
to open without a touch, and they link together to form simultanious
double/quad doors.

Door.master is the master door.  If there is only one door, it points to itself.
If multiple doors, all will point to a single one.

Door.enemy chains from the master door through all doors linked in the chain.

*/

CLASS_DECLARATION( ScriptSlave, Door, "NormalDoor" )
{
	{ &EV_Door_StopSound,				&Door::SetStopSound },
	{ &EV_Door_MoveSound,				&Door::SetMoveSound },
	{ &EV_Door_MessageSound,			&Door::SetMessageSound },
	{ &EV_Door_LockedSound,				&Door::SetLockedSound },
	{ &EV_Door_SetWait,					&Door::SetWait },
	{ &EV_Door_SetDmg,					&Door::SetDmg },
	{ &EV_Door_TriggerFieldTouched,		&Door::FieldTouched },
	{ &EV_Trigger_Effect,				&Door::TryOpen },
	{ &EV_Activate,						&Door::TryOpen },
	{ &EV_Door_TryOpen,					&Door::TryOpen },
	{ &EV_Door_Close,		            &Door::Close },
	{ &EV_Door_Open,		            &Door::Open },
	{ &EV_Door_CloseEnd,	            &Door::CloseEnd },
	{ &EV_Door_OpenEnd,					&Door::OpenEnd },
	{ &EV_Door_Fire,		            &Door::DoorFire },
	{ &EV_Door_Link,		            &Door::LinkDoors },
	{ &EV_Door_SetTime,					&Door::SetTime },
	{ &EV_Use,							&Door::DoorUse },
	{ &EV_Killed,						&Door::DoorFire },
	{ &EV_Blocked,						&Door::DoorBlocked },
	{ &EV_Door_Lock,		            &Door::LockDoor },
	{ &EV_Door_Unlock,					&Door::UnlockDoor },
	{ &EV_SetAngle,						&Door::SetDir },
	{ &EV_Touch,						NULL },
	{ &EV_SetGameplayDamage,			&Door::setDamage },
	{ &EV_Door_ExtraTriggerSize,		&Door::setExtraTriggerSize },

	{ NULL, NULL }
};

Door::Door()
{
	float t;
	const char *text;

	if ( LoadingSavegame )
	{
		return;
	}

	nextdoor = 0;
	trigger = 0;
	locked = false;
	master = this;
	lastblocktime = 0;
	diropened = 0;

	dir = G_GetMovedir( 0.0f );
	t = dir[ 0 ];
	dir[ 0 ] = -dir[ 1 ];
	dir[ 1 ] = t;

	showModel();

	text = gi.GlobalAlias_FindRandom( "door_stop" );
	if ( text )
	{
		SetStopSound( text );
	}

	text = gi.GlobalAlias_FindRandom( "door_moving" );
	if ( text )
	{
		SetMoveSound( text );
	}

	text = gi.GlobalAlias_FindRandom( "snd_locked" );
	if ( text )
	{
		SetLockedSound( text );
	}

	traveltime = 0.3;
	speed = 1.0f / traveltime;

	wait = ( spawnflags & DOOR_TOGGLE ) ? 0 : 3;
	dmg = 0;

	setSize( mins, maxs );

	setOrigin( GetLocalOrigin() );

	// DOOR_START_OPEN is to allow an entity to be lighted in the closed position
	// but spawn in the open position
	if ( spawnflags & DOOR_START_OPEN )
	{
		state = STATE_OPEN;
		PostEvent( EV_Door_Open, EV_POSTSPAWN );
	}
	else
	{
		state = STATE_CLOSED;
	}
	previous_state = state;

	if ( health )
	{
		takedamage = DAMAGE_YES;
	}
	else
	{
		takedamage = DAMAGE_NO;
	}

	// LinkDoors can't be done until all of the doors have been spawned, so
	// the sizes can be detected properly.
	nextdoor = 0;
	PostEvent( EV_Door_Link, EV_LINKDOORS );

	// Default to work with monsters and players
	respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS;
	if ( spawnflags & 8 )
	{
		respondto &= ~TRIGGER_PLAYERS;
	}
	if ( spawnflags & 16 )
	{
		respondto &= ~TRIGGER_MONSTERS;
	}

	next_locked_time = 0;
}

void Door::SetDir( Event *ev )
{
	float t;
	float angle;

	angle = ev->GetFloat( 1 );
	dir = G_GetMovedir( angle );
	t = dir[ 0 ];
	dir[ 0 ] = -dir[ 1 ];
	dir[ 1 ] = t;
}

void Door::SetStopSound( const str &sound )
{
	sound_stop = sound;
	if ( sound_stop.length() > 1 )
	{
		CacheResource( sound_stop.c_str(), this );
	}
}

void Door::SetMoveSound( const str &sound )
{
	sound_move = sound;
	if ( sound_move.length() > 1 )
	{
		CacheResource( sound_move.c_str(), this );
	}
}

void Door::SetMessageSound( const str &sound )
{
	sound_message = sound;
	if ( sound_message.length() > 1 )
	{
		CacheResource( sound_message.c_str(), this );
	}
}

void Door::SetLockedSound( const str &sound )
{
	sound_locked = sound;
	if ( sound_locked.length() > 1 )
	{
		CacheResource( sound_locked.c_str(), this );
	}
}

void Door::SetStopSound( Event *ev )
{
	SetStopSound( ev->GetString( 1 ) );
}

void Door::SetMoveSound( Event *ev )
{
	SetMoveSound( ev->GetString( 1 ) );
}

void Door::SetMessageSound( Event *ev )
{
	SetMessageSound( ev->GetString( 1 ) );
}

void Door::SetLockedSound( Event *ev )
{
	SetLockedSound( ev->GetString( 1 ) );
}

void Door::SetWait( Event *ev )
{
	wait = ev->GetFloat( 1 );
}

void Door::SetDmg( Event *ev )
{
	dmg = ev->GetInteger( 1 );
}

//--------------------------------------------------------------
//
// Name:			setDamage
// Class:			Door
//
// Description:		This function acts as a filter to the real function.
//					It gets data from the database, and then passes it
//					along to the original event.  This is here as an attempt
//					to sway people into using the database standard instead of
//					hardcoded numbers.
//
// Parameters:		Event *ev
//						str -- The value keyword from the database (low, medium, high, etc).
//
// Returns:			None
//
//--------------------------------------------------------------
void Door::setDamage( Event *ev )
{
	if ( ev->NumArgs() < 1 )
		return;

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();	
	if ( !gpm->hasFormula("OffensiveDamage") )
		return;

	str damagestr = ev->GetString( 1 );
	float damagemod = 1.0f;
	if ( gpm->getDefine(damagestr) != "" )
		damagemod = (float)atof(gpm->getDefine(damagestr));
	GameplayFormulaData fd(this, 0, 0, "");
	float finaldamage = gpm->calculate("OffensiveDamage", fd, damagemod);
	Event *newev = new Event(EV_Door_SetDmg);
	newev->AddInteger((int)finaldamage);
	ProcessEvent(newev);
}

qboolean Door::isOpen( void )
{
	return ( state == STATE_OPEN );
}

qboolean Door::isCompletelyClosed( void )
{
	return ( state == STATE_CLOSED );
}

void Door::OpenEnd( Event * )
{
	if ( sound_stop.length() > 1 )
	{
		BroadcastSound();
		Sound( sound_stop, CHAN_VOICE );
	}
	else
	{
		StopSound( CHAN_VOICE );
	}

	previous_state = state;
	state = STATE_OPEN;

	if ( spawnflags & DOOR_TOGGLE )
	{
		// don't close automatically
		return;
	}

	if ( ( wait > 0.0f ) && ( master == this ) )
	{
		PostEvent( EV_Door_Close, wait );
	}
}

void Door::CloseEnd( Event * )
{
	if ( sound_stop.length() > 1 )
	{
		BroadcastSound();
		Sound( sound_stop, CHAN_VOICE );
	}
	else
	{
		StopSound( CHAN_VOICE );
	}

	if ( ( master == this ) && ( state != STATE_CLOSED ) )
	{
		gi.AdjustAreaPortalState( this->edict, false );
	}

	previous_state = state;
	state = STATE_CLOSED;
}

void Door::Close( Event * )
{
	Door *door;

	CancelEventsOfType( EV_Door_Close );

	if ( isCompletelyClosed() )
		return;

	previous_state = state;
	state = STATE_CLOSING;

	ProcessEvent( EV_Door_DoClose );

	if ( sound_move.length() > 1 )
	{
		BroadcastSound();
		Sound( sound_move, CHAN_VOICE );
	}
	if ( master == this )
	{
		if ( max_health )
		{
			takedamage	= DAMAGE_YES;
			health		= max_health;
		}

		// trigger all paired doors
		door = ( Door * )G_GetEntity( nextdoor );
		assert( door->isSubclassOf( Door ) );
		while( door && ( door != this ) )
		{
			door->ProcessEvent( EV_Door_Close );
			door = ( Door * )G_GetEntity( door->nextdoor );
			assert( door->isSubclassOf( Door ) );
		}
	}
}

void Door::Open( Event *ev )
{
	Door *door;
	Event *e;
	Entity *other;

	if ( ev->NumArgs() < 1 )
	{
		ev->Error( "No entity specified to open door.  Door may open the wrong way." );
		other = world;
	}
	else
	{
		other = ev->GetEntity( 1 );
	}

	if ( state == STATE_OPENING )
	{
		// already going up
		return;
	}

	if ( state == STATE_OPEN )
	{
		// reset top wait time
		if ( wait > 0.0f )
		{
			CancelEventsOfType( EV_Door_Close );
			PostEvent( EV_Door_Close, wait );
		}
		return;
	}

	previous_state = state;
	state = STATE_OPENING;

	e = new Event( EV_Door_DoOpen );
	e->AddEntity( other );
	ProcessEvent( e );

	if ( sound_move.length() > 1 )
	{
		BroadcastSound();
		Sound( sound_move, CHAN_VOICE );
	}
	if ( master == this )
	{
		// trigger all paired doors
		door = ( Door * )G_GetEntity( nextdoor );
		assert( door->isSubclassOf( Door ) );
		while( door && ( door != this ) )
		{
			e = new Event( EV_Door_Open );
			e->AddEntity( other );
			door->ProcessEvent( e );
			door = ( Door * )G_GetEntity( door->nextdoor );
			assert( door->isSubclassOf( Door ) );
		}

		if ( previous_state == STATE_CLOSED )
		{
			gi.AdjustAreaPortalState( this->edict, true );
		}
	}
}

void Door::DoorUse( Event *ev )
{
	Entity *other;
	qboolean respond;
	Event *e;

	other = ev->GetEntity( 1 );

	if ( !other )
		return;

	respond = ( ( ( respondto & TRIGGER_PLAYERS ) && other->isClient() ) ||
			    ( ( respondto & TRIGGER_MONSTERS ) && other->isSubclassOf( Actor ) ) );

	if ( !respond )
	{
		return;
	}

	// only allow use when not triggerd by other events
	if ( health || ( spawnflags & ( DOOR_AUTO_OPEN | DOOR_TARGETED ) ) )
	{
		if ( other->isSubclassOf( Sentient ) && ( state == STATE_CLOSED ) )
		{
			if ( health )
			{
				gi.SendServerCommand( (int)NULL, "print \"This door is jammed.\"" );
			}
			else if ( spawnflags & DOOR_TARGETED )
			{
				Sound( "door_triggered", CHAN_VOICE );
            }
		}

		if ( spawnflags & DOOR_AUTO_OPEN && locked && other->isSubclassOf( Player ) && sound_locked.length() )
		{
			other->Sound( sound_locked, CHAN_VOICE );
		}

		return;
	}

	assert( master );
	if ( !master )
	{
		// bulletproofing
		master = this;
	}

	if ( master->state == STATE_CLOSED )
	{
		e = new Event( EV_Door_TryOpen );
		e->AddEntity( other );
		master->ProcessEvent( e );
	}
	else if ( master->state == STATE_OPEN )
	{
		e = new Event( EV_Door_Close );
		e->AddEntity( other );
		master->ProcessEvent( e );
	}
}

void Door::DoorFire( Event *ev )
{
	Event *e;
	Entity *other;

	other = ev->GetEntity( 1 );

	assert( master == this );
	if ( master != this )
	{
		gi.Error( ERR_DROP, "DoorFire: master != self" );
	}

	// no more messages
	SetMessage( NULL );

	// reset health in case we were damage triggered
	health = max_health;

	// will be reset upon return
	takedamage = DAMAGE_NO;

	if ( ( spawnflags & ( DOOR_TOGGLE | DOOR_START_OPEN ) ) && ( ( state == STATE_OPENING ) || ( state == STATE_OPEN ) ) )
	{
		spawnflags &= ~DOOR_START_OPEN;
		ProcessEvent( EV_Door_Close );
	}
	else
	{
		e = new Event( EV_Door_Open );
		e->AddEntity( other );
		ProcessEvent( e );
	}
}

void Door::DoorBlocked( Event *ev )
{
	Event *e;
	Entity *other;

	assert( master );
	if ( ( master ) && ( master != this ) )
	{
		master->ProcessEvent( new Event( ev ) );
		return;
	}

	if ( lastblocktime > level.time )
	{
		return;
	}

	lastblocktime = level.time + 0.3f;

	other = ev->GetEntity( 1 );

	if ( dmg && other )
	{
		other->Damage( this, this, dmg, origin, vec_zero, vec_zero, 0, 0, MOD_CRUSH );

		// if we killed him, lets keep on going

		if ( other->deadflag )
		{
			return;
		}
	}

	if ( ( state == STATE_OPENING ) || ( state == STATE_OPEN ) )
	{
		spawnflags &= ~DOOR_START_OPEN;
		ProcessEvent( EV_Door_Close );
	}
	else
	{
		e = new Event( EV_Door_Open );
		e->AddEntity( other );
		ProcessEvent( e );
	}
}

void Door::FieldTouched( Event *ev )
{
	Entity *other;

	other = ev->GetEntity( 1 );

	if ( !other )
		return;

	if ( ( state != STATE_OPEN ) && !( spawnflags & DOOR_AUTO_OPEN ) && !other->isSubclassOf( Actor ) )
		return;

	TryOpen( ev );
}

qboolean	Door::CanBeOpenedBy( Entity *ent )
{
	assert( master );
	if ( ( master ) && ( master != this ) )
	{
		return master->CanBeOpenedBy( ent );
	}

	if ( !locked && !key.length() )
	{
		return true;
	}

	if ( ent && ent->isSubclassOf( Sentient ) && ( ( Sentient * )ent )->HasItem( key.c_str() ) )
	{
		return true;
	}

	return false;
}

void Door::TryOpen( Event *ev )
{
	Entity *other;
	Event *event;

	//FIXME
	// hack so that doors aren't triggered by guys when game starts.
	// have to fix delay that guys go through before setting up their threads
	if ( level.time < 0.4f )
	{
		return;
	}

	other = ev->GetEntity( 1 );

	assert( master );
	if ( master && ( this != master ) )
	{
		event = new Event( EV_Door_TryOpen );
		event->AddEntity( other );
		master->ProcessEvent( event );
		return;
	}

	if ( !other || other->deadflag )
	{
		return;
	}

	if ( locked )
	{
		if ( next_locked_time <= level.time )
		{
			if ( sound_locked.length() > 1 && !other->isSubclassOf( Actor ) )
			{
				other->Sound( sound_locked, CHAN_VOICE );
			}
			else if ( other->isSubclassOf( Player ) )
			{
				other->Sound( sound_locked, CHAN_VOICE );
//				gi.centerprintf ( other->edict, "This door is locked." );
			}
		}

		// Always increment next locked time

		next_locked_time = level.time + 0.5f;

		// locked doors don't open for anyone
		return;
	}

	if ( !CanBeOpenedBy( other ) )
	{
		Item        *item;
		ClassDef		*cls;

		if ( other->isClient() )
		{
			cls = getClass( key.c_str() );
			if ( !cls )
			{
				gi.WDPrintf( "No item named '%s'\n", key.c_str() );
				return;
			}
			item = ( Item * )cls->newInstance();
			item->CancelEventsOfType( EV_Item_DropToFloor );
			item->CancelEventsOfType( EV_Remove );
			item->ProcessPendingEvents();
			gi.centerprintf ( other->edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$UnlockItem$$%s", item->getName().c_str() );
			delete item;
		}
		return;
	}

	// once we're opened by an item, we no longer need that item to open the door
	key = "";

	if ( Message().length() )
	{
		gi.centerprintf( other->edict, CENTERPRINT_IMPORTANCE_NORMAL, Message().c_str() );
		Sound( sound_message, CHAN_VOICE );
	}

	event = new Event( EV_Door_Fire );
	event->AddEntity( other );
	ProcessEvent( event );
}

void Door::SpawnTriggerField( const Vector &fmins, const Vector &fmaxs )
{
	TouchField *trig;
	Vector min;
	Vector max;

	min = fmins - Vector( 60, 60, 8 ) - _extraTriggerSize;
	max = fmaxs + Vector( 60, 60, 8 ) + _extraTriggerSize;

	trig = new TouchField;
	trig->Setup( this, EV_Door_TriggerFieldTouched, min, max, respondto );

	trigger = trig->entnum;
}

qboolean Door::DoorTouches( const Door *e1 )
{
	if ( e1->absmin.x > absmax.x )
	{
		return false;
	}
	if ( e1->absmin.y > absmax.y )
	{
		return false;
	}
	if ( e1->absmin.z > absmax.z )
	{
		return false;
	}
	if ( e1->absmax.x < absmin.x )
	{
		return false;
	}
	if ( e1->absmax.y < absmin.y )
	{
		return false;
	}
	if ( e1->absmax.z < absmin.z )
	{
		return false;
	}

	return true;
}

void Door::LinkDoors( Event * )
{
	Entity *entptr;
	Door	*ent;
	Door	*next;
	Vector			cmins;
	Vector			cmaxs;
	int				i;

	setSolidType( SOLID_BSP );
	setMoveType( MOVETYPE_PUSH );

	if ( nextdoor )
	{
		// already linked by another door
		return;
	}

	// master doors own themselves
	master = this;

	if ( spawnflags & DOOR_DONT_LINK )
	{
		// don't want to link this door
		nextdoor = entnum;
		return;
	}

	cmins = absmin;
	cmaxs = absmax;

	ent = this;
	for( entptr = this; entptr; entptr = G_FindClass( entptr, getClassID() ) )
	{
		str targetName = TargetName();
		next = ( Door * )entptr;
		if ( !ent->DoorTouches( next ) )
		{
			continue;
		}

		if ( next->nextdoor )
		{
			error( "cross connected doors.  Targetname = %s entity %d\n", targetName.c_str(), entnum );
		}

		ent->nextdoor = next->entnum;
		ent = next;

		for( i = 0; i < 3; i++ )
		{
			if ( ent->absmin[ i ] < cmins[ i ] )
			{
				cmins[ i ] = ent->absmin[ i ];
			}
			if ( ent->absmax[ i ] > cmaxs[ i ] )
			{
				cmaxs[ i ] = ent->absmax[ i ];
			}
		}

		// set master door
		ent->master = this;

		if ( ent->health )
		{
			health = ent->health;
		}

		if ( ent->Targeted() )
		{

			str entTargetname = ent->TargetName();
			if ( !Targeted() )
			{
				SetTargetName( ent->TargetName() );
			}
			else if ( strcmp( TargetName(), ent->TargetName() ) )
			{
				// not a critical error, but let them know about it.
			    gi.WDPrintf( "cross connected doors: %s to %s\n",targetName.c_str(), entTargetname.c_str());

				ent->SetTargetName( TargetName() );
			}
		}

		if ( ent->Message().length() )
		{
			if ( Message().length() && !strcmp( Message().c_str(), ent->Message().c_str() ) )
			{
				// not a critical error, but let them know about it.
			    gi.WDPrintf( "Different messages on linked doors.  Targetname = %s", TargetName() );
			}

			// only master should have a message
			SetMessage( ent->Message().c_str() );
			ent->SetMessage( NULL );
		}
	}

	// make the chain a loop
	ent->nextdoor = entnum;

	// open up any portals we control
	if ( spawnflags & DOOR_START_OPEN )
	{
		gi.AdjustAreaPortalState( this->edict, true );
	}

	// shootable or targeted doors don't need a trigger
	if ( health || ( spawnflags & DOOR_TARGETED ) )
	{
		// Don't let the player trigger the door
		return;
	}

	// Don't spawn trigger field when set to toggle
	if ( !( spawnflags & DOOR_TOGGLE ) )
	{
		SpawnTriggerField( cmins, cmaxs );
	}
}

void Door::SetTime( Event *ev )
{
	traveltime = ev->GetFloat( 1 );
	if ( traveltime < FRAMETIME )
	{
		traveltime = FRAMETIME;
	}

	speed = 1.0f / traveltime;
}

void Door::LockDoor( Event * )
{
	locked = true;
}

void Door::UnlockDoor( Event * )
{
	locked = false;
}

/*****************************************************************************/
/*QUAKED func_rotatingdoor (0 0.25 0.5) ? START_OPEN OPEN_DIRECTION DOOR_DONT_LINK NOT_PLAYERS NOT_MONSTERS TOGGLE AUTO_OPEN TARGETED
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.
DOOR_DONT_LINK is for when you have two doors that are touching but you want to operate independently.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not usefull for touch or takedamage doors).
OPEN_DIRECTION indicates which direction to open when START_OPEN is set.
AUTO_OPEN causes the door to open when a player is near instead of waiting for the player to use the door.
TARGETED door is only operational from triggers or script

"message"		is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"openangle"    how wide to open the door
"angle"			determines the opening direction.  point toward the middle of the door (away from the hinge)
"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"			if set, door must be shot open
"time"			move time (0.3 default)
"wait"			wait before returning (3 default, -1 = never return)
"dmg"				damage to inflict when blocked (0 default)
"key"          The item needed to open this door (default nothing)

"sound_stop"		Specify the sound that plays when the door stops moving (default global door_stop)
"sound_move"		Specify the sound that plays when the door opens or closes (default global door_moving)
"sound_message"	Specify the sound that plays when the door displays a message
"sound_locked"	   Specify the sound that plays when the door is locked

******************************************************************************/

Event EV_RotatingDoor_OpenAngle
(
	"openangle",
	EV_DEFAULT,
	"f",
	"open_angle",
	"Sets the open angle of the door."
);

CLASS_DECLARATION( Door, RotatingDoor, "func_rotatingdoor" )
{
	{ &EV_Door_DoClose,		            &RotatingDoor::DoClose },
	{ &EV_Door_DoOpen,		            &RotatingDoor::DoOpen },
	{ &EV_RotatingDoor_OpenAngle,       &RotatingDoor::OpenAngle },
	{ NULL, NULL }
};

void RotatingDoor::DoOpen( Event *ev )
{
	Vector ang;

	if ( previous_state == STATE_CLOSED )
	{
		if ( ev->NumArgs() > 0 )
		{
			Entity *other;
			Vector p;

			other = ev->GetEntity( 1 );

			if ( other )
			{
				p = other->origin - origin;
				p.z = 0;
				diropened = dir * p;
			}
			else
			{
				diropened = 0 - init_door_direction;
			}
		}
		else
		{
			diropened = 0 - init_door_direction;
		}
	}

	if ( diropened < 0.0f )
	{
		ang = startangle + Vector( 0.0f, angle, 0.0f );
	}
	else
	{
		ang = startangle - Vector( 0.0f, angle, 0.0f );
	}

	mover->MoveTo( origin, ang, fabs( speed*angle ), EV_Door_OpenEnd );
}

void RotatingDoor::DoClose( Event * )
{
	mover->MoveTo( origin, startangle, fabs( speed*angle ), EV_Door_CloseEnd );
}

void RotatingDoor::OpenAngle( Event *ev )
{
	angle = ev->GetFloat( 1 );
}

RotatingDoor::RotatingDoor()
{
	if ( LoadingSavegame )
	{
		return;
	}
	startangle = angles;

	angle = 90;

	init_door_direction = (spawnflags & DOOR_OPEN_DIRECTION);
}

/*****************************************************************************/
/*QUAKED func_door (0 0.25 0.5) ? START_OPEN x DOOR_DONT_LINK NOT_PLAYERS NOT_MONSTERS TOGGLE AUTO_OPEN TARGETED
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.
DOOR_DONT_LINK is for when you have two doors that are touching but you want to operate independently.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not usefull for touch or takedamage doors).
OPEN_DIRECTION indicates which direction to open when START_OPEN is set.
AUTO_OPEN causes the door to open when a player is near instead of waiting for the player to use the door.
TARGETED door is only operational from triggers or script

"message"		is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"			determines the opening direction.  point toward the middle of the door (away from the hinge)
"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"			if set, door must be shot open
"speed"			move speed (100 default)
"time"			move time (1/speed default, overides speed)
"wait"			wait before returning (3 default, -1 = never return)
"lip"				lip remaining at end of move (8 default)
"dmg"				damage to inflict when blocked (0 default)
"key"          The item needed to open this door (default nothing)
"extraTriggerSize"   The extra size for the door trigger (defaults to 0 0 0)

"sound_stop"		Specify the sound that plays when the door stops moving (default global door_stop)
"sound_move"		Specify the sound that plays when the door opens or closes (default global door_moving)
"sound_message"	Specify the sound that plays when the door displays a message
"sound_locked"	   Specify the sound that plays when the door is locked

******************************************************************************/

Event EV_SlidingDoor_Setup
(
	"setup",
	EV_CODEONLY,
	NULL,
	NULL,
	"Sets up the sliding door."
);
Event EV_SlidingDoor_SetLip
(
	"lip",
	EV_SCRIPTONLY,
	"f",
	"lip",
	"Sets the lip of the sliding door."
);
Event EV_SlidingDoor_SetSpeed
(
	"speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Sets the speed of the sliding door."
);

CLASS_DECLARATION( Door, SlidingDoor, "func_door" )
{
	{ &EV_Door_DoClose,		            &SlidingDoor::DoClose },
	{ &EV_Door_DoOpen,		            &SlidingDoor::DoOpen },
	{ &EV_SlidingDoor_Setup,            &SlidingDoor::Setup },
	{ &EV_SlidingDoor_SetLip,           &SlidingDoor::SetLip },
	{ &EV_SlidingDoor_SetSpeed,         &SlidingDoor::SetSpeed },
	{ &EV_SetAngle,                     &SlidingDoor::SetMoveDir },

	{ NULL, NULL }
};

void SlidingDoor::SetMoveDir( Event *ev )
{
	float t;
	float angle;

	angle = ev->GetFloat( 1 );
	movedir = G_GetMovedir( angle );
	dir = movedir;
	t = dir[ 0 ];
	dir[ 0 ] = -dir[ 1 ];
	dir[ 1 ] = t;
}

void SlidingDoor::DoOpen( Event * )
{
	mover->MoveTo( pos2, angles, speed*totalmove, EV_Door_OpenEnd );
}

void SlidingDoor::DoClose( Event * )
{
	mover->MoveTo( pos1, angles, speed*totalmove, EV_Door_CloseEnd );
}

void SlidingDoor::SetLip( Event *ev )
{
	lip = ev->GetFloat( 1 );
	CancelEventsOfType( EV_SlidingDoor_Setup );
	PostEvent( EV_SlidingDoor_Setup, EV_POSTSPAWN );
}

void SlidingDoor::SetSpeed( Event *ev )
{
	basespeed = ev->GetFloat( 1 );
	CancelEventsOfType( EV_SlidingDoor_Setup );
	PostEvent( EV_SlidingDoor_Setup, EV_POSTSPAWN );
}

void SlidingDoor::Setup( Event * )
{
	totalmove = fabs( movedir[0] * size[0] ) + fabs( movedir[1] * size[1] ) + fabs( movedir[2] * size[2] ) - lip;
	pos1 = origin;
	pos2 = pos1 + ( movedir * totalmove );

	if ( basespeed )
	{
		speed = basespeed / totalmove;
	}
	edict->s.eType = ET_MOVER; // added for BOTLIB
}

SlidingDoor::SlidingDoor()
{
	if ( LoadingSavegame )
	{
		return;
	}
	lip = 8;
	basespeed = 0;
	movedir = G_GetMovedir( 0.0f );

	PostEvent( EV_SlidingDoor_Setup, EV_POSTSPAWN );

	totalmove = 0.0f;
}



/*****************************************************************************/
/*QUAKED script_door (0 0.5 1) ? X x DOOR_DONT_LINK NOT_PLAYERS NOT_MONSTERS x AUTO_OPEN x
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.
DOOR_DONT_LINK is for when you have two doors that are touching but you want to operate independently.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not usefull for touch or takedamage doors).
OPEN_DIRECTION indicates which direction to open when START_OPEN is set.
AUTO_OPEN causes the door to open when a player is near instead of waiting for the player to use the door.
TARGETED door is only operational from triggers or script

"message"		 is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"			 determines the opening direction.  point toward the middle of the door (away from the hinge)
"targetname"	 targetname of the door
"health"		 if set, door must be shot open
"speed"			 move speed (100 default)
"wait"			 wait before returning (3 default, -1 = never return)
"dmg"			 damage to inflict when blocked (0 default)
"key"			 The item needed to open this door (default nothing)
"openpercentage" The percentage amount of the length of the door to open
"targeted"		 Sets the door to targeted mode -- Meaning it must be triggered from another entity
"toggle"		 Makes the door stay in its current state until forced to change
"sound_stop"		Specify the sound that plays when the door stops moving (default global door_stop)
"sound_move"		Specify the sound that plays when the door opens or closes (default global door_moving)
"sound_message"	Specify the sound that plays when the door displays a message
"sound_locked"	   Specify the sound that plays when the door is locked

******************************************************************************/

Event EV_ScriptDoor_SetToggle
(
	"toggle",
	EV_SCRIPTONLY,
	"b",
	"flag",
	"Sets Toggle Mode ( 1 ) is true, ( 0 ) is false"
);

Event EV_ScriptDoor_SetTargeted
(
	"targeted",
	EV_SCRIPTONLY,
	"b",
	"flag",
	"Sets the targeted flag ( 1 ) is true , ( 0 ) is false"
);

Event EV_ScriptDoor_SetOpenPercentage
(
	"openpercentage",
	EV_SCRIPTONLY,
	"f",
	"percentage",
	"Sets the percentage for the door to open "
);

Event EV_ScriptDoor_SetupMovement
(
	"scriptdoorsetupmovement",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Sets up Door Movement"
);

Event EV_ScriptDoor_Setup
(
	"scriptdoorsetup",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Sets up Door "
);

Event EV_ScriptDoor_Open
(
	"dooropen",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Opens the script door"
);

Event EV_ScriptDoor_Close
(
	"doorclose",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Closes the script door"
);

Event EV_ScriptDoor_ForceOpen
(
	"forceopen",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Closes the script door"
);

Event EV_ScriptDoor_ForceClose
(
	"forceclose",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Closes the script door"
);

Event EV_ScriptDoor_GetState
(
	"getdoorstate",
	EV_SCRIPTONLY,
	"@s",
	"state",
	"Returns the current state"
);

CLASS_DECLARATION( Door, ScriptDoor, "script_door" )
{
	{ &EV_ScriptDoor_SetToggle ,			&ScriptDoor::SetToggle				},
	{ &EV_ScriptDoor_SetTargeted ,			&ScriptDoor::SetTargeted			},
	{ &EV_ScriptDoor_SetOpenPercentage ,	&ScriptDoor::SetOpenPercentage		},
	{ &EV_SetAngle,							&ScriptDoor::SetMoveDir				},
	{ &EV_Door_DoClose,						&ScriptDoor::DoClose				},
	{ &EV_Door_DoOpen,						&ScriptDoor::DoOpen					},
	{ &EV_ScriptDoor_SetupMovement,			&ScriptDoor::SetUpMovement			},
	{ &EV_ScriptDoor_Setup,					&ScriptDoor::SetUpScriptDoor		},
	{ &EV_Use,								&ScriptDoor::ScriptDoorUse			},
	{ &EV_Door_Close,						&ScriptDoor::ScriptDoorClose		},
	{ &EV_Door_Open,						&ScriptDoor::ScriptDoorOpen			},
	{ &EV_ScriptDoor_ForceOpen,				&ScriptDoor::ScriptDoorForceOpen	},
	{ &EV_ScriptDoor_ForceClose,			&ScriptDoor::ScriptDoorForceClose	},
	{ &EV_Door_TriggerFieldTouched,			&ScriptDoor::ScriptDoorFieldTouched	},
	{ &EV_ScriptDoor_GetState,				&ScriptDoor::ScriptDoorGetState		},

	{ NULL, NULL }
};


ScriptDoor::ScriptDoor()
{
	PostEvent( EV_ScriptDoor_Setup, EV_POSTSPAWN );
}

void ScriptDoor::SetToggle( Event *ev )
{
	SetToggle( ev->GetBoolean( 1 ) );
}

void ScriptDoor::SetToggle( bool toggle )
{
	_toggle = toggle;
}

void ScriptDoor::ScriptDoorGetState( Event *ev )
{
	str currentState;

	currentState = "STATE_UNKNOWN";
	switch ( master->GetState() )
	{
		case STATE_OPEN:
			currentState = "STATE_OPEN";
		break;
		
		case STATE_OPENING:
			currentState = "STATE_OPENING";
		break;

		case STATE_CLOSING:
			currentState = "STATE_CLOSING";
		break;

		case STATE_CLOSED:
			currentState = "STATE_CLOSED";
		break;
	}

	ev->ReturnString( currentState.c_str() );
}

void ScriptDoor::SetTargeted( Event *ev )
{
	SetTargeted( ev->GetBoolean( 1 ) );
}

void ScriptDoor::SetTargeted( bool targeted )
{
	_targeted = targeted;
}

void ScriptDoor::SetOpenPercentage( Event *ev )
{
	SetOpenPercentage( ev->GetFloat( 1 ) );
}

void ScriptDoor::SetOpenPercentage( float openPercentage )
{
	_openPercentage = openPercentage;

	CancelEventsOfType( EV_ScriptDoor_SetupMovement );
	PostEvent( EV_ScriptDoor_SetupMovement, EV_POSTSPAWN );
}

void ScriptDoor::SetMoveDir( Event *ev )
{
	SetMoveDir( ev->GetFloat( 1 ) );
}

void ScriptDoor::SetMoveDir( float moveDir )
{
	float t;
	float angle;

	angle = moveDir;
	_moveDir = G_GetMovedir( angle );
	dir = _moveDir;
	t = dir[ 0 ];
	dir[ 0 ] = -dir[ 1 ];
	dir[ 1 ] = t;
}

void ScriptDoor::SetUpMovement(Event *)
{
	SetUpMovement();
}

void ScriptDoor::SetUpMovement()
{
	_totalMove = fabs( _moveDir[0] * size[0] ) + fabs( _moveDir[1] * size[1] ) + fabs( _moveDir[2] * size[2] );
	_currentMove = _totalMove * _openPercentage;

	_currentPos = origin;
	_destinationPos = _originalPos + ( _moveDir * _currentMove );

	_speed = _baseSpeed / _totalMove;
	
}

void ScriptDoor::SetUpScriptDoor( Event * )
{
	SetUpScriptDoor();
}

void ScriptDoor::SetUpScriptDoor()
{
	_baseSpeed = 50;
	_originalPos = origin;
	_currentPercentage = 0.0;
	_openPercentage = 1.0;
	_targeted = false;
	_toggle = false;
}

void ScriptDoor::DoOpen( Event * )
{
	DoOpen();
}

void ScriptDoor::DoOpen()
{
	mover->MoveTo( _destinationPos, angles, _speed*_totalMove, EV_Door_OpenEnd );
	_currentPercentage = _openPercentage;
}

void ScriptDoor::DoClose( Event * )
{
	DoClose();
}

void ScriptDoor::DoClose()
{
	mover->MoveTo( _originalPos, angles, _speed*_totalMove, EV_Door_CloseEnd );
}


void ScriptDoor::ScriptDoorUse( Event *ev )
{
	Entity *other;
	qboolean respond;
	Event *e;

	other = ev->GetEntity( 1 );

	if ( !other )
		return;

	if ( master != this )
	{
		e = new Event ( ev );
		master->ProcessEvent( e );
	}
		

	if ( _targeted )
		return;

	respond = ( ( ( respondto & TRIGGER_PLAYERS ) && other->isClient() ) ||
				( ( respondto & TRIGGER_MONSTERS ) && other->isSubclassOf( Actor ) ) );

	if ( !respond )
		return;

	// only allow use when not triggerd by other events
	if ( health || ( spawnflags & ( DOOR_AUTO_OPEN | DOOR_TARGETED ) ) )
	{
		if ( other->isSubclassOf( Sentient ) && ( state == STATE_CLOSED ) )
		{
			if ( health )
				gi.SendServerCommand( (int)NULL, "print \"This door is jammed.\"" );
			else if ( spawnflags & DOOR_TARGETED )
				Sound( "door_triggered", CHAN_VOICE );
		}

		if ( spawnflags & DOOR_AUTO_OPEN && locked && other->isSubclassOf( Player ) && sound_locked.length() )
		{
			other->Sound( sound_locked, CHAN_VOICE );
		}

		return;
	}

	assert( master );
	if ( !master )
	{
		// bulletproofing
		master = this;
	}

	if ( master->GetState() == STATE_CLOSED )
	{
		e = new Event( EV_Door_TryOpen );
		e->AddEntity( other );
		master->ProcessEvent( e );
	}
	else if ( master->GetState() == STATE_OPEN )
	{
		if ( _currentPercentage != _openPercentage )
		{
			e = new Event( EV_Door_TryOpen );
			e->AddEntity( other );
			master->ProcessEvent( e );
		}
		else
		{
			e = new Event( EV_Door_Close );
			e->AddEntity( other );
			master->ProcessEvent( e );
		}
	}
}

void ScriptDoor::ScriptDoorOpen( Event *ev )
{
	Door *door;
	Event *e;
	Entity *other;

	if ( ev->NumArgs() < 1 )
	{
		ev->Error( "No entity specified to open door.  Door may open the wrong way." );
		other = world;
	}
	else
	{
		other = ev->GetEntity( 1 );
	}

	if ( _toggle )
		return;

	if ( state == STATE_OPENING )
	{
		// already going up
		return;
	}

	if ( state == STATE_OPEN )
	{
		// reset top wait time
		if ( wait > 0.0f )
		{
			CancelEventsOfType( EV_Door_Close );
			if ( _currentPercentage == _openPercentage )
				PostEvent( EV_Door_Close, wait );
		}

		if ( _currentPercentage == _openPercentage )
			return;
	}

	previous_state = state;
	state = STATE_OPENING;

	e = new Event( EV_Door_DoOpen );
	e->AddEntity( other );
	ProcessEvent( e );

	if ( sound_move.length() > 1 )
	{
		BroadcastSound();
		Sound( sound_move, CHAN_VOICE );
	}
	if ( master == this )
	{
		// trigger all paired doors
		door = ( Door * )G_GetEntity( nextdoor );
		assert( door->isSubclassOf( Door ) );
		while( door && ( door != this ) )
		{
			e = new Event( EV_Door_Open );
			e->AddEntity( other );
			door->ProcessEvent( e );
			door = ( Door * )G_GetEntity( door->GetNextDoor() );
			assert( door->isSubclassOf( Door ) );
		}

		if ( previous_state == STATE_CLOSED )
		{
			gi.AdjustAreaPortalState( this->edict, true );
		}
	}
}

void ScriptDoor::ScriptDoorClose( Event * )
{
	Door *door;

	if ( _toggle )
		return;

	CancelEventsOfType( EV_Door_Close );

	previous_state = state;
	state = STATE_CLOSING;

	ProcessEvent( EV_Door_DoClose );

	if ( sound_move.length() > 1 )
	{
		BroadcastSound();
		Sound( sound_move, CHAN_VOICE );
	}
	if ( master == this )
	{
		if ( max_health )
		{
			takedamage	= DAMAGE_YES;
			health		= max_health;
		}

		// trigger all paired doors
		door = ( Door * )G_GetEntity( nextdoor );
		assert( door->isSubclassOf( Door ) );
		while( door && ( door != this ) )
		{
			door->ProcessEvent( EV_Door_Close );
			door = ( Door * )G_GetEntity( door->GetNextDoor() );
			assert( door->isSubclassOf( Door ) );
		}
	}
}

void ScriptDoor::ScriptDoorFieldTouched( Event *ev )
{
	Entity *other;

	other = ev->GetEntity( 1 );

	if ( !other )
		return;

	if ( _targeted )
		return;

	if ( ( state != STATE_OPEN ) && !( spawnflags & DOOR_AUTO_OPEN ) && !other->isSubclassOf( Actor ) )
		return;

	TryOpen( ev );
}

void ScriptDoor::ScriptDoorForceOpen( Event * )
{
	DoOpen();
}

void ScriptDoor::ScriptDoorForceClose( Event * )
{
	DoClose();
}


/*
Event EV_ScriptDoor_DoInit
	(
	"doinit",
	EV_DEFAULT,
   NULL,
   NULL,
   "Sets up the script door."
	);
Event EV_ScriptDoor_SetOpenThread
	(
	"openthread",
	EV_DEFAULT,
   "s",
   "openthread",
   "Set the thread to run when the door is opened (required)."
	);
Event EV_ScriptDoor_SetCloseThread
	(
	"closethread",
	EV_DEFAULT,
   "s",
   "closethread",
   "Set the thread to run when the door is closed (required)."
	);
Event EV_ScriptDoor_SetInitThread
	(
	"initthread",
	EV_DEFAULT,
   "s",
   "initthread",
   "Set the thread to run when the door is initialized (optional)."
	);

CLASS_DECLARATION( Door, ScriptDoor, "script_door" )
	{
	   { &EV_ScriptDoor_DoInit,		      DoInit },
	   { &EV_Door_DoClose,		            DoClose },
	   { &EV_Door_DoOpen,		            DoOpen },
	   { &EV_ScriptDoor_SetInitThread,     SetInitThread },
	   { &EV_ScriptDoor_SetOpenThread,     SetOpenThread },
	   { &EV_ScriptDoor_SetCloseThread,    SetCloseThread },
      { &EV_SetAngle,                     SetMoveDir },
		{ NULL, NULL }
	};

void ScriptDoor::SetMoveDir
   (
   Event *ev
   )

   {
   float t;
   float angle;

   angle = ev->GetFloat( 1 );
	movedir = G_GetMovedir( angle );
   dir = movedir;
	t = dir[ 0 ];
	dir[ 0 ] = -dir[ 1 ];
	dir[ 1 ] = t;
   }

void ScriptDoor::SetOpenThread
   (
 	Event *ev
   )
	{
   openthreadname = ev->GetString( 1 );
   }

void ScriptDoor::SetCloseThread
   (
 	Event *ev
   )
	{
   closethreadname = ev->GetString( 1 );
   }

void ScriptDoor::DoInit
   (
 	Event *ev
   )
	{
   const char * label = NULL;
   GameScript * s;
   const char * tname;

   startorigin = origin;
	doorsize = fabs( movedir * size );

   //
   // see if we have an openthread
   //
   if ( !openthreadname.length() )
      {
      warning( "ScriptDoor", "No openthread defined for door at %.2f %.2f %.2f", origin[0], origin[1], origin[2] );
      }

   //
   // see if we have an closethread
   //
   if ( !closethreadname.length() )
      {
      warning( "ScriptDoor", "No closethread defined for door at %.2f %.2f %.2f", origin[0], origin[1], origin[2] );
      }

	if(!ScriptLib.HasGameScript())
      {
      warning( "DoInit", "Null game script" );
      return;
      }

   s = ScriptLib.GetScript( ScriptLib.GetGameScript() );
   if ( !s )
      {
      warning( "DoInit", "Null game script" );
      return;
      }

   if ( initthreadname.length() )
      label = initthreadname.c_str();

	doorthread = Director.CreateThread( s, label, MODEL_SCRIPT );
   if ( !doorthread )
      {
      warning( "DoInit", "Could not allocate thread." );
      return;
      }
	doorthread->Vars()->SetVariable( "self", this );
   tname = TargetName();
   if ( tname && tname[ 0 ] )
      {
      str name;
      name = "$" + str( tname );
	   doorthread->Vars()->SetVariable( "targetname", name.c_str() );
      }
	doorthread->Vars()->SetVariable( "startorigin", startorigin );
	doorthread->Vars()->SetVariable( "startangles", startangle );
	doorthread->Vars()->SetVariable( "movedir", movedir );
	doorthread->Vars()->SetVariable( "doorsize", doorsize );
   if ( initthreadname.length() )
      {
      // start right away
      doorthread->StartImmediately();
      }
   }

void ScriptDoor::DoOpen
   (
 	Event *ev
   )

	{
   if ( !doorthread )
      {
      warning( "DoOpen", "No Thread allocated." );
      return;
      }
   else
      {
      if ( !doorthread->Goto( openthreadname.c_str() ) )
         {
         warning( "DoOpen", "Could not goto %s", openthreadname.c_str() );
         return;
         }
      }

   if ( previous_state == STATE_CLOSED )
      {
      diropened = 0;
      if ( ev->NumArgs() > 0 )
         {
   	   Entity *other;
	      Vector p;

	      other = ev->GetEntity( 1 );
         p = other->origin - origin;
         p.z = 0;
         diropened = dir * p;
         }
      }
   doorthread->Vars()->SetVariable( "origin", origin );
	doorthread->Vars()->SetVariable( "opendot", diropened );
   doorthread->Start( 0 );
   }

void ScriptDoor::DoClose
   (
 	Event *ev
   )
	{
   if ( !doorthread )
      {
      warning( "DoClose", "No Thread allocated." );
      return;
      }
   else
      {
      if ( !doorthread->Goto( closethreadname.c_str() ) )
         {
         warning( "DoOpen", "Could not goto %s", closethreadname.c_str() );
         }
      }
   doorthread->Vars()->SetVariable( "origin", origin );
   doorthread->Start( 0 );
   }

void ScriptDoor::SetInitThread
   (
   Event *ev
   )

   {
   initthreadname = ev->GetString( 1 );
   }

ScriptDoor::ScriptDoor()
	{
   if ( LoadingSavegame )
      {
      return;
      }
	startangle = angles;

   //
   // clear out the sounds if necessary
   // scripted doors typically have their own sounds
   //
   sound_stop = "";
   sound_move = "";

	movedir = G_GetMovedir( 0 );

   PostEvent( EV_ScriptDoor_DoInit, EV_POSTSPAWN );
	}

*/
