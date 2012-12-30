//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/trigger.cpp                                   $
// $Revision:: 74                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:11a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Environment based triggers.
//

#include "_pch_cpp.h"
#include "entity.h"
#include "trigger.h"
#include "scriptmaster.h"
#include "worldspawn.h"
#include "misc.h"
#include "specialfx.h"
#include "sentient.h"
#include "item.h"
#include "player.h"
#include "camera.h"
#include "actor.h"
#include "g_utils.h"
#include "weaputils.h"
#include "mp_manager.hpp"
#include <qcommon/gameplaymanager.h>
#include "spawners.h"


Event EV_Trigger_ActivateTargets
(
	"activatetrigger",
	EV_SCRIPTONLY,
	"e",
	"triggering_entity",
	"Activates all of the targets for this trigger."
);
Event EV_Trigger_SetWait
(
	"wait",
	EV_SCRIPTONLY,
	"f",
	"wait_time",
	"Set the wait time (time bewteen triggerings) for this trigger"
);
Event EV_Trigger_SetDelay
(
	"delay",
	EV_SCRIPTONLY,
	"f",
	"delay_time",
	"Set the delay time (time between triggering and firing) for this trigger"
);
Event EV_Trigger_SetCount
(
	"cnt",
	EV_SCRIPTONLY,
	"i",
	"count",
	"Set the amount of times this trigger can be triggered"
);
Event EV_Trigger_SetMessage
(
	"message",
	EV_SCRIPTONLY,
	"s",
	"message",
	"Set a message to be displayed when this trigger is activated"
);
Event EV_Trigger_SetNoise
(
	"noise",
	EV_SCRIPTONLY,
	"s",
	"sound",
	"Set the sound to play when this trigger is activated"
);
Event EV_Trigger_SetSound
(
	"sound",
	EV_SCRIPTONLY,
	"s",
	"sound",
	"Set the sound to play when this trigger is activated"
);
Event EV_Trigger_SetThread
(
	"thread",
	EV_SCRIPTONLY,
	"s",
	"thread",
	"Set the thread to execute when this trigger is activated"
);
Event EV_Trigger_Effect
(
	"triggereffect",
	EV_CODEONLY,
	"e",
	"triggering_entity",
	"Send event to owner of trigger."
);
Event EV_Trigger_Effect_Alt
(
	"triggereffectalt",
	EV_CODEONLY,
	"e",
	"triggering_entity",
	"Send event to owner of trigger.  This event is only triggered when using a trigger\n"
	"as a multi-faceted edge trigger."
);
Event EV_Trigger_StartThread
(
	"triggerthread",
	EV_CODEONLY,
	NULL,
	NULL,
	"Start the trigger thread."
);
Event EV_Trigger_SetKey
(
	"key",
	EV_SCRIPTONLY,
	"s",
	"key",
	"Set the object needed by the sentient to activate this trigger"
);
Event EV_Trigger_SetTriggerable
(
	"triggerable",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Turn this trigger back on"
);
Event EV_Trigger_SetNotTriggerable
(
	"nottriggerable",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Turn this trigger off"
);
Event EV_Trigger_SetMultiFaceted
(
	"multifaceted",
	EV_SCRIPTONLY,
	"i",
	"facetDirection",
	"Make this trigger multifaceted.  If facet is 1, than trigger is North/South oriented.\n"
	"If facet is 2 than trigger is East/West oriented. If facet is 3 than trigger is Up/Down oriented."
);
Event EV_Trigger_SetEdgeTriggered
(
	"edgetriggered",
	EV_SCRIPTONLY,
	"b",
	"newEdgeTriggered",
	"If true, trigger will only trigger when object enters trigger, not when it is inside it."
);
Event EV_Trigger_SetTriggerCone
(
	"cone",
	EV_SCRIPTONLY,
	"f",
	"newTriggerCone",
	"Sets the cone in which directed triggers will trigger."
);
Event EV_Trigger_CheckEntList
(
	"checkEntList",
	EV_CODEONLY,
	"f",
	"check_the_list",
	"Checks the ent list to see if everyone is still in the bounding box"
);
Event EV_Trigger_TriggerExit
(
	"triggerexit",
	EV_CODEONLY,
	"f",
	"trigger_exit",
	"called when an entity has left the trigger bounding box"
);
Event EV_Trigger_SetPassEvent
(
	"setpassevent",
	EV_SCRIPTONLY,
	"sSSSSSSSS",
	"eventName string1 string2 string3 string4 string5 string6 string7 string8",
	"Sets up the event to be passed on to the group"
);
Event EV_Trigger_SetGroupNumber
(
	"setgroupnumber",
	EV_SCRIPTONLY,
	"i",
	"groupNumber",
	"Sets up the group number the event will be passed to"
);
Event EV_Trigger_SetRequiredEntity
(
	"requiredentity",
	EV_SCRIPTONLY,
	"s",
	"targetname",
	"sets this entity to be required for a trigger to work"
);

Event EV_Trigger_CheckReady
(
	"checkready",
	EV_CODEONLY,
	"",
	"",
	"event to check if trigger has all required ents in place and is ready"
);
Event EV_Trigger_SetEntryThread
(
	"entryThread",
	EV_SCRIPTONLY,
	"s",
	"thread_name",
	"thread to call when entity enters the trigger"
);
Event EV_Trigger_SetExitThread
(
	"exitThread",
	EV_SCRIPTONLY,
	"s",
	"thread_name",
	"thread to call when entity exits the trigger"
);
Event EV_Trigger_AltSetEntryThread
(
	"onEntry",
	EV_SCRIPTONLY,
	"s",
	"thread_name",
	"thread to call when entity enters the trigger"
);
Event EV_Trigger_AltSetExitThread
(
	"onExit",
	EV_SCRIPTONLY,
	"s",
	"thread_name",
	"thread to call when entity exits the trigger"
);
// 1ST PLAYABLE HACK
Event EV_Trigger_Hack_SetTriggerParms
(
	"settriggerparms",
	EV_DEFAULT,
	"ff",
	"force_field_number trigger_number",
	"HACK HACK HACK HACK HACK HACK HACK"
);
Event EV_Trigger_Hack_GetForceFieldNumber
(
	"getforcefieldnumber",
	EV_DEFAULT,
	"@f",
	"number",
	"HACK HACK HACK HACK HACK HACK HACK"
);
Event EV_Trigger_Hack_GetTriggerNumber
(
	"gettriggernumber",
	EV_DEFAULT,
	"@f",
	"number",
	"HACK HACK HACK HACK HACK HACK HACK"
);
Event EV_Trigger_SetTriggerOnDamage
(
	"triggerondamage",
	EV_DEFAULT,
	"b",
	"boolean",
	"Forces the thread to trigger when it takes damage"
);
Event EV_Trigger_SetTriggerOnDeath
(
	"triggerondeath",
	EV_DEFAULT,
	"b",
	"boolean",
	"Forces the thread to trigger when it dies"
);
Event EV_Trigger_SetDestructible
(
	"destructible",
	EV_DEFAULT,
	"b",
	"boolean",
	"Makes a trigger destructible.  By default this is false."
);
Event EV_Trigger_SetInstantDeath
(
	"instantdeath",
	EV_DEFAULT,
	"s",
	"damage_type",
	"Sets a damage type that will instantly kill the trigger, such"
	" as impact, sword, gas, bullet, etc."
);
Event EV_Trigger_SetEnter
(
	"triggerenter",
	EV_DEFAULT,
	NULL,
	NULL,
	"Marks the trigger as entered"
);
Event EV_Trigger_EntityExit
(
	"triggerentexit",
	EV_DEFAULT,
	NULL,
	NULL,
	"Marks the trigger as exited"
);
Event EV_Trigger_GetLastActivatingEntity
(
	"getLastActivatingEntity",
	EV_DEFAULT,
	"@e",
	"returned_entity",
	"Returns the activating entity for this object, or 0 if no activating entity."
);

//#define MULTI_ACTIVATE	1
//#define INVISIBLE			2

#define VISIBLE			1

#define TRIGGER_PLAYERS			4
#define TRIGGER_MONSTERS		8
#define TRIGGER_PROJECTILES		16
#define TRIGGER_SCRIPTSLAVE		32

/*****************************************************************************/
/*QUAKED trigger_multiple (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES SCRIPTSLAVE

Variable sized repeatable trigger.  Must be targeted at one or more entities.

If "delay" is set, the trigger waits some time after activating before firing.

If "destructible" is set, the trigger will take damage and lose health.
If "triggerondamage" is set, trigger will fire when ever it takes damage.
If "triggerondeath" is set, trigger will fire when it dies.  This forces destructible
to be on, since it would otherwise never die.
If "instantdeath" is set, trigger will die when it takes damage of this type.

To make a trigger fire every time it takes damage set triggerondamage to true.  To make a 
trigger fire everytime it takes damage and to eventually die, set triggerondamage
to true, destructible to true, and give it a health.  To make a trigger that
triggers on death only, set triggerondeath to be true.  This will also turn
on destructible.  To make a trigger that triggers everytime it takes damage
and when it dies, set both triggerondamage to be true and triggerondeath to
be true.

"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"wait" : Seconds between triggerings. (.2 default)
"cnt" how many times it can be triggered (infinite default)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)
if SCRIPTSLAVE is set, the trigger will respond to script slaves

set "message" to text string

******************************************************************************/

CLASS_DECLARATION( Entity, Trigger, "trigger_multiple" )
{
	{ &EV_Trigger_SetWait,					&Trigger::EventSetWait },
	{ &EV_Trigger_SetDelay,					&Trigger::EventSetDelay },
	{ &EV_Trigger_SetCount,					&Trigger::EventSetCount },
	{ &EV_Trigger_SetMessage,				&Trigger::EventSetMessage },
	{ &EV_Trigger_SetNoise,					&Trigger::EventSetNoise },
	{ &EV_Trigger_SetSound,					&Trigger::EventSetNoise },
	{ &EV_Trigger_SetThread,				&Trigger::EventSetThread },
	{ &EV_Trigger_SetTriggerOnDeath,		&Trigger::EventSetTriggerOnDeath },
	{ &EV_Trigger_SetTriggerOnDamage,		&Trigger::EventSetTriggerOnDamage },
	{ &EV_Trigger_SetInstantDeath,			&Trigger::EventSetInstantDeath },
	{ &EV_Trigger_SetDestructible,			&Trigger::EventSetDestructible },
	{ &EV_Trigger_GetLastActivatingEntity,	&Trigger::EventGetLastActivatingEntity },
	{ &EV_SetHealth,						&Trigger::EventSetHealth },
	{ &EV_Touch,							&Trigger::TriggerStuff },
	{ &EV_Killed,							&Trigger::HandleKilled },
	{ &EV_Activate,							&Trigger::TriggerStuff },
	{ &EV_Damage,							&Trigger::HandleDamage },
	{ &EV_Trigger_ActivateTargets,			&Trigger::ActivateTargets },
	{ &EV_Trigger_SetKey,					&Trigger::EventSetKey },
	{ &EV_Trigger_StartThread,				&Trigger::StartThread },
	{ &EV_Model,							&Trigger::SetModelEvent },
	{ &EV_SetAngle,							&Trigger::SetTriggerDir },
	{ &EV_Trigger_SetTriggerable,			&Trigger::SetTriggerable },
	{ &EV_Trigger_SetNotTriggerable,		&Trigger::SetNotTriggerable },
	{ &EV_Trigger_SetMultiFaceted,			&Trigger::SetMultiFaceted },
	{ &EV_Trigger_SetEdgeTriggered,			&Trigger::SetEdgeTriggered },
	{ &EV_Trigger_SetTriggerCone,			&Trigger::SetTriggerCone },
	{ &EV_Trigger_CheckEntList,				&Trigger::CheckEntList   },
	{ NULL, NULL }
};

Trigger::Trigger()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	triggerActivated	= false;
	activator			= NULL;
	trigger_time		= (float)0;
	
	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_TRIGGER );
	
	setContents( 0 );
	//edict->contents = CONTENTS_TRIGGER;		// replaces the -1 from gi.SetBrushModel
	edict->svflags |= SVF_NOCLIENT;
	
	delay = 0;
	wait  = 0.2;
	health = 0;
	max_health = 0;
	triggerCone = (float)cos( DEG2RAD( 60.0 ) );
	
	useTriggerDir = false;
	triggerDir = G_GetMovedir( 0.0f );
	
	triggerable = true;
	removable = true;
	instantdeath		= -1 ; // all
	triggerondamage		= false ;
	triggerondeath		= false ;
	destructible		= false ;
	
	trigger_time = 0.0f;
	// normal trigger to begin with
	multiFaceted = 0;
	// normally, not edge triggered
	edgeTriggered = false;
	
	count = -1;
	
	//noise = "environment/switch/switch2.wav";
	noise = "";
	
	respondto = spawnflags ^ TRIGGER_PLAYERS;
}

Trigger::~Trigger()
{
}

void Trigger::SetTriggerDir( float angle )
{
	triggerDirYaw = angle;
	triggerDir = G_GetMovedir( angle );
	useTriggerDir = true;
}

Vector Trigger::GetTriggerDir( void )
{
	return triggerDir;
}

void Trigger::SetTriggerCone( Event *ev )
{
	triggerCone = (float)cos( DEG2RAD( ev->GetFloat( 1 ) ) );
}

qboolean Trigger::UsingTriggerDir( void )
{
	return useTriggerDir;
}

void Trigger::SetTriggerDir( Event *ev )
{
	SetTriggerDir( ev->GetFloat( 1 ) );
}

void Trigger::EventSetTriggerOnDamage( Event *ev )
{
	if (!ev)			return ;
	triggerondamage		= ev->GetBoolean(1);
	
	if (triggerondamage) 
	{
		SetTakeDamage(true);
	}
}

void Trigger::EventSetTriggerOnDeath( Event *ev )
{
	if (!ev)			return ;
	triggerondeath	=	ev->GetBoolean(1);
	
	if (triggerondeath)	
	{
		destructible = true ; // force destructibility
		SetTakeDamage(true);
	}
}

void Trigger::EventSetDestructible( Event *ev )
{
	if (!ev) 
		return;
	destructible = ev->GetBoolean(1);
}


void Trigger::EventSetInstantDeath( Event *ev )
{
	if (!ev)	return ;
	instantdeath = MOD_NameToNum( ev->GetString(1) );
}

void Trigger::SetModelEvent( Event *ev )
{
	Entity::SetModelEvent( ev );
	setContents( 0 );
	//edict->contents = CONTENTS_TRIGGER;		// replaces the -1 from gi.SetBrushModel
	edict->svflags |= SVF_NOCLIENT;
	link();
}

void Trigger::StartThread( Event *ev )
{
	if ( thread.length() )
	{
		if ( !ExecuteThread( thread , true, this ) )
		{
			warning( "StartThread", "Null game script" );
		}
	}
}

qboolean Trigger::respondTo( Entity *other )
{
	return ( ( ( respondto & TRIGGER_PLAYERS ) && other->isClient() ) ||
			 ( ( respondto & TRIGGER_PLAYERS ) && other->isSubclassOf( Vehicle ) ) ||
			 ( ( respondto & TRIGGER_MONSTERS ) && other->isSubclassOf( Actor ) ) ||
			 ( ( respondto & TRIGGER_PROJECTILES ) && other->isSubclassOf( Projectile ) ) ||
			 ( ( respondto & TRIGGER_SCRIPTSLAVE ) && other->isSubclassOf( ScriptSlave )));
}

//===============================================================
// Name:		EventGetLastActivatingEntity
// Class:		Trigger
//
// Description: Retrieves in the event structure the last entity
//				that activated this trigger.
// 
// Parameters:	Event* -- the event.  The return field of this
//						  event will contain the entity that
//						  triggered this entity.
//
// Returns:		None
// 
//===============================================================
void Trigger::EventGetLastActivatingEntity( Event *ev )
{
	ev->ReturnEntity( activator );
}


Entity *Trigger::getActivator( Entity *other )
{
	return other;
}

//===============================================================
// Name:		setActivatingEntity
// Class:		Trigger
//
// Description: Sets the activating entity for a trigger.  Called
//				whenever a trigger is triggered.
// 
// Parameters:	Entity* -- the activating entity.
//
// Returns:		None
// 
//===============================================================
void Trigger::setActivatingEntity( Entity *activatingEntity )
{
	activator = activatingEntity ;
}

bool Trigger::CanTrigger( void )
{
	// if trigger is shut off return immediately
	if ( !triggerable ) return false ;
	
	// Don't bother with testing anything if we can't trigger yet
	if ( ( level.time < trigger_time ) || ( trigger_time == -1.0f ) )
	{
		// if we are edgeTriggered, we reset our time until we leave the trigger
		if ( edgeTriggered && ( trigger_time != -1.0f ) )
		{
			trigger_time = level.time + wait;
		}
		return false ;
	}
	return true ;
}


void Trigger::HandleDamage( Event *ev )
{
	if (!CanTrigger())		return ;
	if (!ev)				return ;
	
	float	damage			= ev->GetFloat(1) ;
	Entity *inflictor		= ev->GetEntity( 2 );
	Entity *attacker		= ev->GetEntity( 3 );
	
	if ( damageModSystem )
		{	
		::Damage modDamage(ev);
		damageModSystem->resolveDamage(modDamage);
		damage = modDamage.damage;
		}
	
	if (destructible)						health -= damage ;
	if (instantdeath == ev->GetInteger(9))	health = 0 ;
	
	if ( (health <= 0.0f))
	{
		Event *event = new Event( EV_Killed );
		event->AddEntity( attacker );
		event->AddFloat( damage );
		event->AddEntity( inflictor );
		ProcessEvent( event );
		return;
	}
	
	if (triggerondamage)	
	{
		TriggerStuff(ev);
	}
}

void Trigger::HandleKilled( Event *ev )
{
	if (!CanTrigger())		return ;
	if (triggerondeath)		TriggerStuff(ev);
}

void Trigger::TriggerStuff( Event *ev )
{
	Entity *other;
	Entity *activator;
	Event *event;
	int    whatToTrigger;
	
	if (!CanTrigger()) return ;
	
	
	if (triggerondamage || triggerondeath)
	{
		// if we're triggered by damage or death, and this is inappropriate event, jump out.
		bool isActiveEvent = ( (int)*ev == (int)EV_Activate)? true : false ;
		bool isDamageEvent = ( (int)*ev == (int)EV_Damage ) ? true : false ;
		bool isKilledEvent = ( (int)*ev == (int)EV_Killed ) ? true : false ;
		if ( !isActiveEvent && !(triggerondamage && isDamageEvent) && !(triggerondeath && isKilledEvent)) return ;
	}
	
	if ((int)*ev == (int)EV_Damage)
	{
		other = ev->GetEntity(3);
	}
	else
	{
		other = ev->GetEntity( 1 );
	}
	assert( other != this );
	
	// Always respond to activate messages from the world since they're probably from
	// the "trigger" command
	if ( !respondTo( other ) && !( ( other == world || other->isSubclassOf( SpawnChain ) ) && ( ( int )*ev == ( int )EV_Activate ) ) &&
		( !other || !other->isSubclassOf( Camera ) ) )
	{
		return;
	}
	
	AddOtherToEntList( other );
	Event *checkEvent;
	checkEvent = new Event(EV_Trigger_CheckEntList);
	checkEvent->AddInteger( other->entnum );
	PostEvent( checkEvent , CHECK_TIME );
	
	
	//
	// if we setup an angle for this trigger, only trigger if other is within ~60 degrees of the triggers origin
	// only test for this case if we were touched, activating or killed should never go through this code
	//
	if ( other->isSubclassOf( Player ) && useTriggerDir && ( ( int )*ev == ( int )EV_Touch ) )
	{
		Vector otherFwd( 0, 0, 0 );
		
		// do special case for looking up and down
		// triggerDir.z will be set=0.0 directly, so don't bother with epsilon
		if( triggerDir.z == 0.0f )
		{
			// only consider yaw
			otherFwd[YAW] = other->client->ps.viewangles[YAW];
		}
		else
		{
			// only consider pitch
			otherFwd[PITCH] = other->client->ps.viewangles[PITCH];
		}
		otherFwd.AngleVectors( &otherFwd );
		otherFwd.normalize();
		
		float dot = otherFwd * triggerDir;
		if( dot < triggerCone )
		{
			// don't retrigger for at least a second
			trigger_time = level.time + 0.05f;
			return;
		}
	}
	
	activator = getActivator( other );
	setActivatingEntity( activator );
	
	if ( key.length() )
	{
		if ( !activator->isSubclassOf( Sentient ) )
		{
			return;
		}
		if ( !( ( (Sentient *)activator )->HasItem( key.c_str() ) ) )
		{
			qboolean    setModel;
			Item        *item;
			ClassDef		*cls;
			str         dialog;
			
			cls = FindClass( key.c_str(), &setModel );
			if ( !cls || !checkInheritance( "Item", cls->classname ) )
			{
				gi.WDPrintf( "No item named '%s'\n", key.c_str() );
				return;
			}
			item = ( Item * )cls->newInstance();
			if ( setModel )
            {
				item->setModel( key.c_str() );
            }
			item->CancelEventsOfType( EV_Item_DropToFloor );
			item->CancelEventsOfType( EV_Remove );
			item->ProcessPendingEvents();
			dialog = item->GetDialogNeeded();
			if ( dialog.length() > 1 )
            {
				activator->Sound( dialog );
            }
			else
            {
				gi.centerprintf ( activator->edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$ItemNeeded$$%s", item->getName().c_str() );
            }
			delete item;
			return;
		}
	}
	
	if ( multiFaceted )
	{
		Vector delta;
		
		delta = other->origin - origin;
		switch( multiFaceted )
		{
		case 1:
            if ( delta[ 1 ] > 0.0f )
			{
				whatToTrigger = 0;
			}
            else
			{
				whatToTrigger = 1;
			}
            break;
		case 2:
            if ( delta[ 0 ] > 0.0f )
			{
				whatToTrigger = 0;
			}
            else
			{
				whatToTrigger = 1;
			}
            break;
		case 3:
		default:
            if ( delta[ 2 ] > 0.0f )
			{
				whatToTrigger = 0;
			}
            else
			{
				whatToTrigger = 1;
			}
            break;
		}
	}
	else
	{
		whatToTrigger = 0;
	}
	
	trigger_time = level.time + wait;
	
	if ( !whatToTrigger )
	{
		event = new Event( EV_Trigger_Effect );
		event->AddEntity( activator );
		PostEvent( event, delay );
	}
	else
	{
		event = new Event( EV_Trigger_Effect_Alt );
		event->AddEntity( activator );
		PostEvent( event, delay );
	}
	
	event = new Event( EV_Trigger_ActivateTargets );
	event->AddEntity( activator );
	PostEvent( event, delay );
	
	if ( thread.length() )
	{
		// don't trigger the thread if we were triggered by the world touching us
		if ( ( activator != world ) || ( ev->GetSource() != EV_FROM_CODE ) )
		{
			event = new Event( EV_Trigger_StartThread );
			if ( activator )
            {
				event->AddEntity( activator );
            }
			PostEvent( event, delay );
		}
	}
	
	if ( count > -1 )
	{
		count--;
		if ( count < 1 )
		{
			//
			// Don't allow it to trigger anymore
			//
			trigger_time = -1;
			
			//
			// Make sure we wait until we're done triggering things before removing
			//
			if ( removable )
            {
				PostEvent( EV_Remove, delay + FRAMETIME );
            }
		}
	}
}

//
//==============================
// ActivateTargets
//
// "other" should be set to the entity that initiated the firing.
//
// Centerprints any message to the activator.
//
// Removes all entities with a targetname that match killtarget,
// so some events can remove other triggers.
//
// Search in targetname of all entities that match target
// and send EVENT_ACTIVATE to there event handler
//==============================
//
void Trigger::ActivateTargets( Event *ev )
{
	Entity	*other;
	Entity	*ent;
	Event		*event;
	const char *name;
	
	other = ev->GetEntity( 1 );
	
	if ( !other )
		other = world;
	
	if ( triggerActivated )
	{
		//
		// Entity triggered itself.  Prevent an infinite loop
		//
		ev->Error( "Entity targeting itself--Targetname '%s'", TargetName() );
		return;
	}
	
	triggerActivated = true;
	activator = other;
	
	//
	// print the message
	//
	if ( message.length() && other && ( other->isClient() || other->isSubclassOf( Camera ) ) )
	{
		// HACK HACK HACK
		// if it is a camera, pass in default player
		if ( !other->isClient() )
		{
			gi.centerprintf( &g_entities[ 0 ], CENTERPRINT_IMPORTANCE_NORMAL, message.c_str() );
		}
		else
		{
			gi.centerprintf( other->edict, CENTERPRINT_IMPORTANCE_NORMAL, message.c_str() );
		}
		if ( Noise().length() )
		{
			other->Sound( noise.c_str(), CHAN_VOICE );
		}
	}
	
	//
	// kill the killtargets
	//
	name = KillTarget();
	if ( name && strcmp( name, "" ) )
	{
		ent = NULL;
		do
		{
			ent = G_FindTarget( ent, name );
			if ( !ent )
			{
				break;
			}
			ent->PostEvent( EV_Remove, 0.0f );
		}
		while ( 1 );
	}
	
	//
	// fire targets
	//
	name = Target();
	if ( name && strcmp( name, "" ) )
	{
		ent = NULL;
		do
		{
			ent = G_FindTarget( ent, name );
			if ( !ent )
			{
				break;
			}
			
			event = new Event( EV_Activate );
			event->AddEntity( other );
			ent->ProcessEvent( event );
		}
		while ( 1 );
	}
	
	triggerActivated = false;
}

void Trigger::EventSetWait( Event *ev )
{
	wait = ev->GetFloat( 1 );
}

void Trigger::EventSetDelay( Event *ev )
{
	delay = ev->GetFloat( 1 );
}

void Trigger::EventSetKey( Event *ev )
{
	key = ev->GetString( 1 );
}

void Trigger::EventSetThread( Event *ev )
{
	thread = ev->GetString( 1 );
}

void Trigger::EventSetHealth( Event *ev )
{
	health = ev->GetFloat( 1 );
	max_health = health;
	if ( health )
	{
		takedamage = DAMAGE_YES;
		setSolidType( SOLID_BBOX );
	}
	else
	{
		setMoveType( MOVETYPE_NONE );
		setSolidType( SOLID_TRIGGER );
	}
}

void Trigger::SetTakeDamage( bool cantakedamage )
{
	if (cantakedamage)
	{
		takedamage = DAMAGE_YES ;
		setSolidType( SOLID_BBOX );
	}
	else
	{
		takedamage = DAMAGE_NO ;
		setMoveType( MOVETYPE_NONE ); // no one knows why this is here
		setSolidType( SOLID_TRIGGER );
	}
}

void Trigger::EventSetCount( Event *ev )
{
	count = ev->GetInteger( 1 );
}

void Trigger::EventSetMessage( Event *ev )
{
	SetMessage( ev->GetString( 1 ) );
}

void Trigger::SetMessage( const char *text )
{
	if ( text )
	{
		message = str( text );
	}
	else
	{
		message = "";
	}
}

str &Trigger::Message( void )
{
	return message;
}

void Trigger::EventSetNoise( Event *ev )
{
	SetNoise( ev->GetString( 1 ) );
}

void Trigger::SetNoise( const char *text )
{
	if ( text )
	{
		noise = str( text );
		//
		// cache in the sound
		//
		CacheResource( noise.c_str(), this );
	}
}

str &Trigger::Noise( void )
{
	return noise;
}

void Trigger::SetMultiFaceted( int newFacet )
{
	multiFaceted = newFacet;
}

void Trigger::SetEdgeTriggered( qboolean newEdge )
{
	edgeTriggered = newEdge;
}

int Trigger::GetMultiFaceted( void )
{
	return multiFaceted;
}

qboolean Trigger::GetEdgeTriggered( void )
{
	return edgeTriggered;
}

void Trigger::SetMultiFaceted( Event *ev )
{
	SetMultiFaceted( ev->GetInteger( 1 ) );
}

void Trigger::SetEdgeTriggered( Event *ev )
{
	SetEdgeTriggered( ev->GetBoolean( 1 ) );
}

void Trigger::SetTriggerable( Event *ev )
{
	triggerable = true;
}

void Trigger::SetNotTriggerable( Event *ev )
{
	triggerable = false;
}

void Trigger::CheckEntList(Event *ev)
{
	CancelEventsOfType(EV_Trigger_CheckEntList);
	int entNum;
	Entity *other = NULL;
	
	entNum = ev->GetInteger( 1 );   
	other = G_GetEntity(entNum );
	
	if ( !other )
		return;
	
	if (edgeTriggered && entList.NumObjects() > 0 )   
	{
		Event *checkEV;
		checkEV = new Event(EV_Trigger_CheckEntList);
		checkEV->AddInteger( entNum );
		
		PostEvent( checkEV , CHECK_TIME );
	}
		
	Entity *ent;
	Event *exitEvent;
	
	for ( int i = entList.NumObjects() ; i >= 1 ; i-- )
	{
		entNum = entList.ObjectAt( i );
		ent = NULL;
		ent = G_GetEntity( entNum );
		
		if ( ent )
		{
			if ( !IsEntityInBoundingBox( ent ) )
            {
				entList.RemoveObjectAt( i );
				exitEvent = new Event ( EV_Trigger_TriggerExit );
				exitEvent->AddEntity( ent );
				PostEvent(exitEvent, .25);
            }
		}	
	}
}

void Trigger::AddOtherToEntList(Entity *other)
{
	int entNum;
	
	if ( !other )
		return;
	for ( int i = 1 ; i <= entList.NumObjects() ; i++ )
	{
		entNum = entList.ObjectAt( i );
		if ( entNum == other->entnum )
			return;
	}
	
	entList.AddObject( other->entnum );
}

qboolean Trigger::IsEntityInBoundingBox ( Entity *ent )
{
	if ( ( ent->absmin[0] > absmax[0] ) ||
		( ent->absmin[1] > absmax[1] ) ||
		( ent->absmin[2] > absmax[2] ) ||
		( ent->absmax[0] < absmin[0] ) ||
		( ent->absmax[1] < absmin[1] ) ||
		( ent->absmax[2] < absmin[2] ) ) 
	{
		return false;
		  }
	
	return true;
}


CLASS_DECLARATION( Trigger, TouchField, NULL )
{
	{ &EV_Trigger_Effect,				&TouchField::SendEvent },

	{ NULL, NULL }
};

TouchField::TouchField()
{
	ontouch = NULL;
}

void TouchField::Setup( Entity *ownerentity, const Event &touchevent, const Vector &min, const Vector &max, int respondto )
{
	assert( ownerentity );
	if ( !ownerentity )
	{
		error( "Setup", "Null owner" );
	}
	
	owner = ownerentity;
	if ( ontouch )
	{
		delete ontouch;
	}
	ontouch	= new Event( touchevent );
	setSize( min, max );
	
	setContents( 0 );
	setSolidType( SOLID_TRIGGER );
	link();
	
	this->respondto = respondto;
}

void TouchField::SendEvent( Event *ev )
{
	Event *event;
	
	// Check if our owner is still around
	if ( owner )
	{
		event = new Event( ontouch );
		event->AddEntity( ev->GetEntity( 1 ) );
		owner->PostEvent( event, delay );
	}
	else
	{
		// Our owner is gone!  It didn't delete us!
		// Guess we're no longer needed, so remove ourself.
		PostEvent( EV_Remove, 0.0f );
	}
}

/*****************************************************************************/
/*QUAKED trigger_once (1 0 0) ? NOTOUCH x NOT_PLAYERS MONSTERS PROJECTILES SCRIPTSLAVE

Variable sized trigger. Triggers once, then removes itself.
You must set the key "target" to the name of another object in the
level that has a matching

If "health" is set, the trigger must be killed to activate it.
If "delay" is set, the trigger waits some time after activating before firing.

"targetname".  If "health" is set, the trigger must be killed to activate.

"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

if "killtarget" is set, any objects that have a matching "target" will be
removed when the trigger is fired.

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"key" The item needed to activate this. (default nothing)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOTOUCH is set, trigger will not respond to touch
If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)
If SCRIPTSLAVE is set, the trigger will respond to script slaves

set "message" to text string

******************************************************************************/

CLASS_DECLARATION( Trigger, TriggerOnce, "trigger_once" )
{
	{ NULL, NULL }
};

TriggerOnce::TriggerOnce()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	//
	// no matter what, we only trigger once
	//
	count = 1;
	respondto = spawnflags ^ TRIGGER_PLAYERS;
	
	//
	// if it's not supposed to be touchable, clear the trigger
	//
	if ( spawnflags & 1 )
	{
		setSolidType( SOLID_NOT );
	}
}

/*****************************************************************************/
/*QUAKED trigger_relay (1 0 0) (-8 -8 -8) (8 8 8) x x NOT_PLAYERS MONSTERS PROJECTILES

This fixed size trigger cannot be touched, it can only be fired by other events.
It can contain killtargets, targets, delays, and messages.

If NOT_PLAYERS is set, the trigger does not respond to events triggered by players
If MONSTERS is set, the trigger will respond to events triggered by monsters
If PROJECTILES is set, the trigger will respond to events triggered by projectiles (rockets, grenades, etc.)

******************************************************************************/

CLASS_DECLARATION( Trigger, TriggerRelay, "trigger_relay" )
{
	{ &EV_Touch,   NULL },
	{ NULL,        NULL }
};

TriggerRelay::TriggerRelay()
{
	setSolidType( SOLID_NOT );
}

/*****************************************************************************/
/*QUAKED trigger_secret (1 0 0) ? NOTOUCH x NOT_PLAYERS MONSTERS PROJECTILES
Secret counter trigger.  Automatically sets and increments script variables \
level.total_secrets and level.found_secrets.

set "message" to text string

"key" The item needed to activate this. (default nothing)

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"thread" name of thread to trigger.  This can be in a different script file as well \
by using the '::' notation.  (defaults to "global/universal_script.scr::secret")

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOTOUCH is set, trigger will not respond to touch
If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

CLASS_DECLARATION( TriggerOnce, TriggerSecret, "trigger_secret" )
{
	{ &EV_Trigger_Effect,		&TriggerSecret::FoundSecret },
	{ NULL, NULL }
};

TriggerSecret::TriggerSecret()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	level.total_secrets++;
	levelVars.SetVariable( "total_secrets", level.total_secrets );
	respondto = spawnflags ^ TRIGGER_PLAYERS;
	
	// set the thread to trigger when secrets are found
	thread = "global/universal_script.scr::secret";
}

void TriggerSecret::FoundSecret( Event *ev )
{
	//
	// anything that causes the trigger to fire increments the number
	// of secrets found.  This way, if the level designer triggers the
	// secret from the script, the player still gets credit for finding
	// it.  This is to prevent a secret from becoming undiscoverable.
	//
	level.found_secrets++;
	levelVars.SetVariable( "found_secrets", level.found_secrets );

	gi.centerprintf ( &g_entities[0], CENTERPRINT_IMPORTANCE_NORMAL, "$$FoundSecretArea$$" );
}

/*****************************************************************************/
/*QUAKED trigger_setvariable (1 0 0) ? NOTOUCH LEVEL NOT_PLAYERS MONSTERS PROJECTILES

Sets a variable specified by "variable" and "value".
Variable is assumed to be of the "global" variety unless LEVEL is set.
Variable sized trigger. Triggers once by default.
You must set the key "target" to the name of another object in the
level that has a matching

"variable" - variable to set
"value" - value to set in variable, value can also be one of the following reserved\
tokens.
   - "increment" - add one to the variable
   - "decrement" - subtract one from the variable
   - "toggle" - if 1, then zero.  If zero then 1.

If "health" is set, the trigger must be killed to activate it.
If "delay" is set, the trigger waits some time after activating before firing.

"targetname".  If "health" is set, the trigger must be killed to activate.

"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

if "killtarget" is set, any objects that have a matching "target" will be
removed when the trigger is fired.

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"key" The item needed to activate this. (default nothing)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOTOUCH is set, trigger will not respond to touch
if LEVEL is set, variable will be a level variable otherwise it will be a game variable
If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

set "message" to text string

******************************************************************************/

#define LEVEL_VARIABLE ( 1 << 1 )

Event EV_TriggerSetVariable_SetVariable
(
	"variable",
	EV_SCRIPTONLY,
	"s",
	"variableName",
	"Set the name of the variable to set"
);
Event EV_TriggerSetVariable_SetVariableValue
(
	"value",
	EV_SCRIPTONLY,
	"s",
	"variableValue",
	"Set the value of the variable to set.\n"
	"values can also use one of the following reserved words:\n"
	"  increment - increment the variable\n"
	"  decrement - decrement the variable\n"
	"  toggle - toggle the value of the variable"
);
CLASS_DECLARATION( Trigger, TriggerSetVariable, "trigger_setvariable" )
{
	{ &EV_Trigger_Effect,						&TriggerSetVariable::SetVariable },
	{ &EV_TriggerSetVariable_SetVariable,		&TriggerSetVariable::SetVariableName },
	{ &EV_TriggerSetVariable_SetVariableValue,	&TriggerSetVariable::SetVariableValue },

	{ NULL, NULL }
};

TriggerSetVariable::TriggerSetVariable()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	//
	// no matter what, we only trigger once
	//
	count = 1;
	respondto = spawnflags ^ TRIGGER_PLAYERS;
	
	//
	// if it's not supposed to be touchable, clear the trigger
	//
	if ( spawnflags & 1 )
	{
		setSolidType( SOLID_NOT );
	}
	variableName = "undefined";
	variableValue = "notset";
	variableType = VAR_NORMAL;
}

void TriggerSetVariable::SetVariableName( Event *ev )
{
	variableName = ev->GetString( 1 );
}

void TriggerSetVariable::SetVariableValue( Event *ev )
{
	variableValue = ev->GetString( 1 );
	if ( variableValue == "increment" )
	{
		variableType = VAR_INCREMENT;
	}
	else if ( variableValue == "decrement" )
	{
		variableType = VAR_DECREMENT;
	}
	else if ( variableValue == "toggle" )
	{
		variableType = VAR_TOGGLE;
	}
	else
	{
		variableType = VAR_NORMAL;
	}
}

void TriggerSetVariable::SetVariable( Event *ev )
{
	ScriptVariable * var;
	int value;
	
	if ( spawnflags & LEVEL_VARIABLE )
	{
		var = levelVars.GetVariable( variableName );
		if ( !var )
		{
			var = levelVars.SetVariable( variableName, 0 );
		}
	}
	else
	{
		var = gameVars.GetVariable( variableName );
		if ( !var )
		{
			var = gameVars.SetVariable( variableName, 0 );
		}
	}
	
	assert( var );
	value = var->intValue();
	
	switch( variableType )
	{
	case VAR_INCREMENT:
		value++;
		break;
	case VAR_DECREMENT:
		value--;
		break;
	case VAR_TOGGLE:
		value = !value;
		break;
	}
	
	if ( variableType == VAR_NORMAL )
	{
		var->setStringValue( variableValue );
	}
	else
	{
		var->setIntValue( value );
	}
}

/*****************************************************************************/
/*QUAKED trigger_push (1 0 0) ? x x NOT_PLAYERS NOT_MONSTERS NOT_PROJECTILES

Pushes entities as if they were caught in a heavy wind.

"speed" indicates the rate that entities are pushed (default 1000).

"angle" indicates the direction the wind is blowing (-1 is up, -2 is down)

"key" The item needed to activate this. (default nothing)

"target" if target is set, then a velocity will be calculated based on speed

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not push players
If NOT_MONSTERS is set, the trigger will not push monsters
If NOT_PROJECTILES is set, the trigger will not push projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerPush_SetPushSpeed
(
	"speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Set the push speed of the TriggerPush"
);

CLASS_DECLARATION( Trigger, TriggerPush, "trigger_push" )
{
	{ &EV_Trigger_Effect,				&TriggerPush::Push },
	{ &EV_SetAngle,						&TriggerPush::SetPushDir },
	{ &EV_TriggerPush_SetPushSpeed,		&TriggerPush::SetPushSpeed },

	{ NULL, NULL }
};

void TriggerPush::Push( Event *ev )
{
	Entity *other;
	
	other = ev->GetEntity( 1 );
	if ( other )
	{
		const char * targ;
		Entity *ent;
		
		targ = Target ();
		if ( targ[ 0 ] )
		{
			ent = G_FindTarget( NULL, Target() );
			if ( ent )
            {
				other->velocity = G_CalculateImpulse
					(
					other->origin,
					ent->origin,
					speed,
					other->gravity
					);
            }
		}
		else
		{
			float dot;
			
			// find out how much velocity we have in this direction
			dot = triggerDir * other->velocity;
			// subtract it out and add in our velocity
			other->velocity += ( speed - dot ) * triggerDir;
		}
		
		other->VelocityModified();
		movetype = MOVETYPE_TOSS; // Set movetype so actors can get pushed.
	}
}

void TriggerPush::SetPushDir( Event *ev )
{
	float angle;
	
	angle = ev->GetFloat( 1 );
	// this is used, since we won't need elsewhere
	triggerDir = G_GetMovedir( angle );
}


void TriggerPush::SetPushSpeed( Event *ev )
{
	speed = ev->GetFloat( 1 );
}

TriggerPush::TriggerPush()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	speed = 1000;
	respondto = spawnflags ^ ( TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES );
}

/*****************************************************************************/
/*QUAKED trigger_pushany (1 0 0) ? x x NOT_PLAYERS NOT_MONSTERS NOT_PROJECTILES

Pushes entities as if they were caught in a heavy wind.

"speed" indicates the rate that entities are pushed (default 1000).
"angles" indicates the direction of the push
"key" The item needed to activate this. (default nothing)
"target" if target is set, then a velocity will be calculated based on speed

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not push players
If NOT_MONSTERS is set, the trigger will not push monsters
If NOT_PROJECTILES is set, the trigger will not push projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerPushAny_SetSpeed
(
	"speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Set the speed."
);

CLASS_DECLARATION( Trigger, TriggerPushAny, "trigger_pushany" )
{
	{ &EV_TriggerPushAny_SetSpeed,		&TriggerPushAny::SetSpeed },
	{ &EV_Trigger_Effect,				&TriggerPushAny::Push },

	{ NULL, NULL }
};

TriggerPushAny::TriggerPushAny()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	speed = 1000;
	respondto = spawnflags ^ ( TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES );
}

void TriggerPushAny::Push( Event *ev )
{
	Entity *other;
	
	other = ev->GetEntity( 1 );
	if ( other )
	{
		Entity *ent;
		
		if ( target.length() )
		{
			ent = G_FindTarget( NULL, target.c_str() );
			if ( ent )
            {
				other->velocity = G_CalculateImpulse
					(
					other->origin,
					ent->origin,
					speed,
					other->gravity
					);
            }
		}
		else
		{
			other->velocity = Vector( orientation[ 0 ] ) * speed;
		}
		other->VelocityModified();
	}
}

void TriggerPushAny::SetSpeed( Event *ev )
{
	speed = ev->GetFloat( 1 );
}

//================================================================================================
//
// TriggerSound stuff
//
//================================================================================================

Event EV_TriggerPlaySound_SetVolume
(
	"volume",
	EV_DEFAULT,
	"f",
	"volume",
	"Sets the volume."
);
Event EV_TriggerPlaySound_SetMinDist
(
	"min_dist",
	EV_DEFAULT,
	"f",
	"min_dist",
	"Sets the minimum distance."
);
Event EV_TriggerPlaySound_SetChannel
(
	"channel",
	EV_DEFAULT,
	"i",
	"channel",
	"Sets the sound channel to play on."
);

CLASS_DECLARATION( Trigger, TriggerPlaySound, "play_sound_triggered" )
{
	{ &EV_Trigger_Effect,						&TriggerPlaySound::ToggleSound },
	{ &EV_TriggerPlaySound_SetVolume,			&TriggerPlaySound::SetVolume },
	{ &EV_TriggerPlaySound_SetMinDist,			&TriggerPlaySound::SetMinDist },
	{ &EV_TriggerPlaySound_SetChannel,			&TriggerPlaySound::SetChannel },
	{ &EV_Touch,								NULL },

	{ NULL, NULL }
};

void TriggerPlaySound::ToggleSound( Event *ev )
{
	if ( !state )
	{
		// noise should already be initialized
		if ( Noise().length() )
		{
			if ( ambient || ( spawnflags & TOGGLESOUND ) )
            {
				state = 1;
            }
			
			if ( ambient )
            {
				LoopSound( Noise().c_str(), volume, min_dist );
            }
			else
            {
				Sound( Noise(), channel, volume, min_dist );
            }
		}
	}
	else
	{
		state = 0;
		if ( ambient )
		{
			StopLoopSound();
		}
		else
		{
			StopSound( channel );
		}
	}
}

void TriggerPlaySound::StartSound( void )
{
	// turn the current one off
	state = 1;
	ToggleSound( NULL );
	
	// start it up again
	state = 0;
	ToggleSound( NULL );
}

void TriggerPlaySound::SetVolume( float vol )
{
	volume = vol;
}

void TriggerPlaySound::SetVolume( Event *ev )
{
	volume = ev->GetFloat( 1 );
}

void TriggerPlaySound::SetMinDist( float dist )
{
	min_dist = dist;
}

void TriggerPlaySound::SetMinDist( Event *ev )
{
	min_dist = ev->GetFloat( 1 );
}

void TriggerPlaySound::SetChannel( Event *ev )
{
	channel = ev->GetInteger( 1 );
}

TriggerPlaySound::TriggerPlaySound()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	//
	// this is here so that it gets sent over at least once
	//
	PostEvent( EV_Show, EV_POSTSPAWN );
	
	ambient     = false;
	volume		= DEFAULT_VOL;
	channel		= CHAN_VOICE;
	state			= 0;
	respondto	= spawnflags ^ TRIGGER_PLAYERS;
	min_dist		= DEFAULT_MIN_DIST;
	
	if ( spawnflags & ( AMBIENT_ON | AMBIENT_OFF ) )
	{
		ambient = true;
		if ( spawnflags & AMBIENT_ON )
		{
			PostEvent( EV_Trigger_Effect, EV_POSTSPAWN );
		}
	}
}

/*****************************************************************************/
/*QUAKED sound_speaker (0 0.75 0.75) (-8 -8 -8) (8 8 8) AMBIENT-ON AMBIENT-OFF NOT_PLAYERS MONSTERS PROJECTILES TOGGLE

play a sound when it is used

AMBIENT-ON specifies an ambient sound that starts on
AMBIENT-OFF specifies an ambient sound that starts off
TOGGLE specifies that the speaker toggles on triggering

if (AMBIENT-?) is not set, then the sound is sent over explicitly this creates more net traffic

"volume" how loud 0-4 (1 default full volume, ambients do not respond to volume)
"noise" sound to play
"channel" channel on which to play sound\
(0 auto, 1 weapon, 2 voice, 3 item, 4 body, 8 don't use PHS) (voice is default)
"key" The item needed to activate this. (default nothing)
"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

Normal sounds play each time the target is used.

Ambient Looped sounds have a volume 1, and the use function toggles it on/off.

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

CLASS_DECLARATION( TriggerPlaySound, TriggerSpeaker, "sound_speaker" )
{
	{ &EV_Touch,	NULL },
	{ NULL, NULL }
};

TriggerSpeaker::TriggerSpeaker()
{
}

/*****************************************************************************/
/*QUAKED sound_randomspeaker (0 0.75 0.75) (-8 -8 -8) (8 8 8) x x NOT_PLAYERS MONSTERS PROJECTILES x x

play a sound at random times

"mindelay" minimum delay between sound triggers (default 3)
"maxdelay" maximum delay between sound triggers (default 10)
"chance" chance that sound will play when fired (default 1)
"volume" how loud 0-4 (1 default full volume)
"noise" sound to play
"channel" channel on which to play sound\
(0 auto, 1 weapon, 2 voice, 3 item, 4 body, 8 don't use PHS) (voice is default)
"key"          The item needed to activate this. (default nothing)

Normal sounds play each time the target is used.

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerRandomSpeaker_TriggerSound
(
	"triggersound",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Triggers the sound to play and schedules the next time to play."
);
Event EV_Trigger_SetMinDelay
(
	"mindelay",
	EV_SCRIPTONLY,
	"f",
	"min_delay",
	"Sets the minimum time between playings."
);
Event EV_Trigger_SetMaxDelay
(
	"maxdelay",
	EV_SCRIPTONLY,
	"f",
	"max_delay",
	"Sets the maximum time between playings."
);
Event EV_Trigger_SetChance
(
	"chance",
	EV_DEFAULT,
	"f[0,1]",
	"newChance",
	"Sets the chance that the sound will play when triggered."
);

CLASS_DECLARATION( TriggerSpeaker, RandomSpeaker, "sound_randomspeaker" )
{
	{ &EV_Trigger_Effect,					&RandomSpeaker::TriggerSound },
	{ &EV_Trigger_SetMinDelay,				&RandomSpeaker::SetMinDelay },
	{ &EV_Trigger_SetMaxDelay,				&RandomSpeaker::SetMaxDelay },
	{ &EV_Trigger_SetChance,				&RandomSpeaker::SetChance },
	{ &EV_Touch,							NULL },

	{ NULL, NULL }
};

void RandomSpeaker::TriggerSound( Event *ev )
{
	ScheduleSound();
	if ( G_Random( 1.0f ) <= chance )
		TriggerPlaySound::ToggleSound( ev );
}

void RandomSpeaker::ScheduleSound( void )
{
	CancelEventsOfType( EV_Trigger_Effect );
	PostEvent( EV_Trigger_Effect, mindelay + G_Random( maxdelay-mindelay ) );
}

void RandomSpeaker::SetMinDelay( Event *ev )
{
	mindelay = ev->GetFloat( 1 );
}

void RandomSpeaker::SetMaxDelay( Event *ev )
{
	maxdelay = ev->GetFloat( 1 );
}

void RandomSpeaker::SetChance( Event *ev )
{
	chance = ev->GetFloat( 1 );
}

void RandomSpeaker::SetMinDelay( float newMinDelay )
{
	mindelay = newMinDelay;
}

void RandomSpeaker::SetMaxDelay( float newMaxDelay )
{
	maxdelay = newMaxDelay;
}

void RandomSpeaker::SetChance( float newChance )
{
	chance = newChance;
}

RandomSpeaker::RandomSpeaker()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	ambient = false;
	chance = 1.0f;
	mindelay = 3;
	maxdelay = 10;
	
	ScheduleSound();
}

/*****************************************************************************/
/*QUAKED trigger_changelevel (1 0 0) ? NO_INTERMISSION x NOT_PLAYERS MONSTERS PROJECTILES

When the player touches this, he gets sent to the map listed in the "map" variable.
Unless the NO_INTERMISSION flag is set, the view will go to the info_intermission
spot and display stats.

"spawnspot"  name of the spawn location to start at in next map.
"key"          The item needed to activate this. (default nothing)
"thread" This defaults to "LevelComplete" and should point to a thread that is called just
before the level ends.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerChangeLevel_Map
(
	"map",
	EV_SCRIPTONLY,
	"s",
	"map_name",
	"Sets the map to change to when triggered."
);
Event EV_TriggerChangeLevel_SpawnSpot
(
	"spawnspot",
	EV_SCRIPTONLY,
	"s",
	"spawn_spot",
	"Sets the spawn spot to use."
);

CLASS_DECLARATION( Trigger, TriggerChangeLevel, "trigger_changelevel" )
{
	{ &EV_Trigger_Effect,					&TriggerChangeLevel::ChangeLevel },
	{ &EV_TriggerChangeLevel_Map,			&TriggerChangeLevel::SetMap },
	{ &EV_TriggerChangeLevel_SpawnSpot,		&TriggerChangeLevel::SetSpawnSpot },
	{ &EV_Trigger_SetThread,				&TriggerChangeLevel::SetThread },

	{ NULL, NULL }
};

TriggerChangeLevel::TriggerChangeLevel()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	// default level change thread
	changethread = "LevelComplete";
	respondto = spawnflags ^ TRIGGER_PLAYERS;
}

void TriggerChangeLevel::SetMap( Event *ev )
{
	map = ev->GetString( 1 );
}

void TriggerChangeLevel::SetSpawnSpot( Event *ev )
{
	spawnspot = ev->GetString( 1 );
}

void TriggerChangeLevel::SetThread( Event *ev )
{
	// We have to handle calling the thread ourselves, so clear out Trigger's thread variable
	thread = "";
	changethread = ev->GetString( 1 );
}

void TriggerChangeLevel::ChangeLevel( Event *ev )
{
	Entity *other;
	
	other = ev->GetEntity( 1 );
	
	if ( level.intermissiontime )
	{
		// already activated
		return;
	}
	
	// if noexit, do a ton of damage to other
	if ( multiplayerManager.inMultiplayer() && multiplayerManager.checkFlag( MP_FLAG_SAME_LEVEL ) && ( other != world ) )
	{
		other->Damage( this, other, 10.0f * other->max_health, other->origin, vec_zero, vec_zero, 1000, 0, MOD_CRUSH );
		return;
	}
	
	// tell the script that the player's not ready so that if we return to this map,
	// we can do something about it.
	Director.PlayerNotReady();
	
	//
	// make sure we execute the thread before changing
	//
	if ( changethread.length() )
	{
		ExecuteThread( changethread , true , this );
	}
	
	if ( spawnspot.length() )
	{
		G_BeginIntermission( ( map + "$" + spawnspot ).c_str() );
	}
	else
	{
		G_BeginIntermission( map.c_str() );
	}
}

const char *TriggerChangeLevel::Map( void )
{
	return map.c_str();
}

const char *TriggerChangeLevel::SpawnSpot( void )
{
	return spawnspot.c_str();
}

/*****************************************************************************/
/*QUAKED trigger_use (1 0 0) ? VISIBLE x NOT_PLAYERS MONSTERS

Activates targets when 'used' by an entity
"key"          The item needed to activate this. (default nothing)
"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters

******************************************************************************/

CLASS_DECLARATION( Trigger, TriggerUse, "trigger_use" )
{
	{ &EV_Use,						&TriggerUse::TriggerStuff },
	{ &EV_Touch,					NULL },

	{ NULL, NULL }
};

TriggerUse::TriggerUse()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	if ( spawnflags & VISIBLE )
	{
		showModel();
		setMoveType( MOVETYPE_PUSH );
		setSolidType( SOLID_BSP );
	}
	
	respondto = ( spawnflags ^ TRIGGER_PLAYERS ) & ~TRIGGER_PROJECTILES;
}

/*****************************************************************************/
/*QUAKED trigger_useonce (1 0 0) ? VISIBLE x NOT_PLAYERS MONSTERS

Activates targets when 'used' by an entity, but only once
"key"          The item needed to activate this. (default nothing)
"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters

******************************************************************************/

CLASS_DECLARATION( TriggerUse, TriggerUseOnce, "trigger_useonce" )
{
	{ &EV_Touch,				NULL },

	{ NULL, NULL }
};

TriggerUseOnce::TriggerUseOnce()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	// Only allow 1 use.
	count = 1;
	
	respondto = ( spawnflags ^ TRIGGER_PLAYERS ) & ~TRIGGER_PROJECTILES;
}

/*****************************************************************************/
/*QUAKED trigger_hurt (1 0 0) ? x x NOT_PLAYERS NOT_MONSTERS PROJECTILES

"damage" amount of damage to cause. (default 10)
"key"          The item needed to activate this. (default nothing)
"damagetype" what kind of damage should be given to the player. (default CRUSH)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not hurt players
If NOT_MONSTERS is set, the trigger does not hurt monsters
If PROJECTILES is set, the trigger will hurt projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerHurt_SetDamage
(
	"damage",
	EV_SCRIPTONLY,
	"f",
	"damage",
	"Sets the amount of damage to do."
);
Event EV_TriggerHurt_SetDamageType
(
	"damagetype",
	EV_SCRIPTONLY,
	"s",
	"damageType",
	"Sets the type of damage a TriggerHurt delivers."
);

CLASS_DECLARATION( TriggerUse, TriggerHurt, "trigger_hurt" )
{
	{ &EV_Trigger_Effect,						&TriggerHurt::Hurt },
	{ &EV_TriggerHurt_SetDamage,				&TriggerHurt::SetDamage },
	{ &EV_SetGameplayDamage,					&TriggerHurt::setDamage },
	{ &EV_TriggerHurt_SetDamageType,			&TriggerHurt::DamageType },
	{ &EV_Touch,								&Trigger::TriggerStuff },

	{ NULL, NULL }
};

TriggerHurt::TriggerHurt()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	damage = 10;
	damage_type = MOD_CRUSH;
	respondto = spawnflags ^ ( TRIGGER_PLAYERS | TRIGGER_MONSTERS );
}

//--------------------------------------------------------------
//
// Name:			setDamage
// Class:			TriggerHurt
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
void TriggerHurt::setDamage( Event *ev )
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
	Event *newev = new Event(EV_TriggerHurt_SetDamage);
	newev->AddFloat(finaldamage);
	ProcessEvent(newev);
}


void TriggerHurt::SetDamage( Event *ev )
{
	damage = ev->GetFloat( 1 );
}

void TriggerHurt::Hurt( Event *ev )
{
	Entity *other;
	
	other = ev->GetEntity( 1 );
	
	if ( ( damage != 0 ) && !other->deadflag && !( other->flags & FL_GODMODE ) )
	{
		other->Damage( this, world, damage, other->origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR|DAMAGE_NO_SKILL, damage_type );
	}
}

/*****************************************************************************/
/*QUAKED trigger_damagetargets (1 0 0) ? SOLID x NOT_PLAYERS NOT_MONSTERS PROJECTILES

"damage" amount of damage to cause. If no damage is specified, objects\
are damaged by the current health+1

"key"          The item needed to activate this. (default nothing)

if a trigger_damagetargets is shot at and the SOLID flag is set,\
the damage is passed on to the targets

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not hurt players
If NOT_MONSTERS is set, the trigger does not hurt monsters
If PROJECTILES is set, the trigger will hurt projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerDamageTargets_SetDamage
(
	"damage",
	EV_SCRIPTONLY,
	"f",
	"damage",
	"Sets the amount of damage to do."
);

CLASS_DECLARATION( Trigger, TriggerDamageTargets, "trigger_damagetargets" )
{
	{ &EV_Trigger_ActivateTargets,				&TriggerDamageTargets::DamageTargets },
	{ &EV_TriggerDamageTargets_SetDamage,		&TriggerDamageTargets::SetDamage },
	{ &EV_SetGameplayDamage,					&TriggerDamageTargets::setDamage },
	{ &EV_Damage,								&TriggerDamageTargets::PassDamage },
	{ &EV_Touch,								NULL },

	{ NULL, NULL }
};

TriggerDamageTargets::TriggerDamageTargets()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	damage = 0;
	respondto = spawnflags ^ ( TRIGGER_PLAYERS | TRIGGER_MONSTERS );
	
	if ( spawnflags & 1 )
	{
		health = 60;
		max_health = health;
		takedamage = DAMAGE_YES;
		setSolidType( SOLID_BBOX );
	}
	else
	{
		takedamage = DAMAGE_NO;
		setSolidType( SOLID_NOT );
	}
}

//--------------------------------------------------------------
//
// Name:			setDamage
// Class:			TriggerDamageTargets
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
void TriggerDamageTargets::setDamage( Event *ev )
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
	Event *newev = new Event(EV_TriggerDamageTargets_SetDamage);
	newev->AddFloat((int)finaldamage);
	ProcessEvent(newev);
}

void TriggerDamageTargets::SetDamage( Event *ev )
{
	damage = ev->GetInteger( 1 );
}

void TriggerDamageTargets::PassDamage( Event *ev )
{
	Entity		*attacker;
	float			dmg;
	Entity	   *ent;
	const char	*name;
	
	dmg		= ev->GetFloat( 1 );
	attacker = ev->GetEntity( 3 );
	
	//
	// damage the targets
	//
	name = Target();
	if ( name && strcmp( name, "" ) )
	{
		ent = NULL;
		do
		{
			ent = G_FindTarget( ent, name );
			if ( !ent )
			{
				break;
			}
			
			if ( !ent->deadflag && !( ent->flags & FL_GODMODE ) )
			{
				ent->Damage( this, attacker, dmg, ent->origin, vec_zero, vec_zero, 0, 0, MOD_CRUSH );
			}
		}
		while ( 1 );
	}
}
//
//==============================
// DamageTargets
//==============================
//

void TriggerDamageTargets::DamageTargets( Event *ev )
{
	Entity	*other;
	Entity	*ent;
	const char		*name;
	
	other = ev->GetEntity( 1 );
	
	if ( triggerActivated )
	{
		//
		// Entity triggered itself.  Prevent an infinite loop
		//
		ev->Error( "Entity targeting itself--Targetname '%s'", TargetName() );
		return;
	}
	
	triggerActivated = true;
	activator = other;
	
	//
	// print the message
	//
	if ( message.length() && other && other->isClient() )
	{
		gi.centerprintf( other->edict, CENTERPRINT_IMPORTANCE_NORMAL, message.c_str() );
		if ( Noise().length() )
		{
			other->Sound( Noise().c_str(), CHAN_VOICE );
		}
	}
	
	//
	// damage the targets
	//
	name = Target();
	if ( name && strcmp( name, "" ) )
	{
		ent = NULL;
		do
		{
			ent = G_FindTarget( ent, name );
			if ( !ent )
			{
				break;
			}
			
			if ( !ent->deadflag && !( ent->flags & FL_GODMODE ) )
			{
				if (damage)
					ent->Damage( this, other, damage, ent->origin, vec_zero, vec_zero, 0, 0, MOD_CRUSH );
				else
					ent->Damage( this, other, ent->health + 1.0f, ent->origin, vec_zero, vec_zero, 0, 0, MOD_CRUSH );
			}
		}
		while ( 1 );
	}
	
	triggerActivated = false;
}

/*****************************************************************************/
/*QUAKED trigger_camerause (1 0 0) ? VISIBLE x NOT_PLAYERS MONSTERS

Activates 'targeted' camera when 'used'
If activated, toggles through cameras
"key"          The item needed to activate this. (default nothing)
"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters

******************************************************************************/

CLASS_DECLARATION( TriggerUse, TriggerCameraUse, "trigger_camerause" )
{
	{ &EV_Use,					&TriggerCameraUse::TriggerCamera },
	{ &EV_Touch,				NULL },

	{ NULL, NULL }
};

void TriggerCameraUse::TriggerCamera( Event *ev )
{
	str    camthread;
	Entity *other;
	
	other = ev->GetEntity( 1 );
	if ( other->isClient() )
	{
		Player * client;
		Entity * ent;
		Camera * cam;
		
		client = ( Player * )other;
		cam = client->CurrentCamera();
		if ( cam != NULL )
		{
			str nextcam;
			
			nextcam = cam->NextCamera();
			if ( nextcam.length() )
            {
				ent = G_FindTarget( NULL, nextcam.c_str() );
				
				if ( ent )
				{
					if ( ent->isSubclassOf( Camera ) )
					{
						cam = (Camera *)ent;
						camthread = cam->Thread();
						client->SetCamera( cam, CAMERA_SWITCHTIME );
					}
				}
            }
		}
		else
		{
			ent = G_FindTarget( NULL, Target() );
			if ( ent )
            {
				if ( ent->isSubclassOf( Camera ) )
				{
					cam = (Camera *)ent;
					camthread = cam->Thread();
					client->SetCamera( cam, CAMERA_SWITCHTIME );
				}
				else
				{
					warning( "TriggerCamera", "%s is not a camera", Target() );
				}
            }
			
		}
		if ( camthread.length() > 1 )
		{
			if ( !ExecuteThread( camthread , true , this ) )
            {
				warning( "TriggerCamera", "Null game script" );
            }
		}
	}
}

/*****************************************************************************/
/*QUAKED trigger_exit (1 0 0) ?

When the player touches this, an exit icon will be displayed in his hud.
This is to inform him that he is near an exit.

"triggerable" turn trigger on
"nottriggerable" turn trigger off

******************************************************************************/

Event EV_TriggerExit_TurnExitOff
(
	"_turnexitoff",
	EV_CODEONLY,
	NULL,
	NULL,
	"Internal event that turns the exit sign off."
);

CLASS_DECLARATION( Trigger, TriggerExit, "trigger_exit" )
{
	{ &EV_Trigger_Effect,					&TriggerExit::DisplayExitSign },
	{ &EV_TriggerExit_TurnExitOff,			&TriggerExit::TurnExitSignOff },

	{ NULL, NULL }
};

TriggerExit::TriggerExit()
{
	wait = 1;
	respondto = TRIGGER_PLAYERS;
}

void TriggerExit::TurnExitSignOff( Event *ev )
{
	level.near_exit = false;
}

void TriggerExit::DisplayExitSign( Event *ev )
{
	level.near_exit = true;
	
	CancelEventsOfType( EV_TriggerExit_TurnExitOff );
	PostEvent( EV_TriggerExit_TurnExitOff, 1.1f );
}

/*****************************************************************************/
/*       trigger_box (0.5 0.5 0.5) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Variable sized repeatable trigger.  Must be targeted at one or more entities.  Made to
be spawned via script.

If "health" is set, the trigger must be killed to activate each time.
If "delay" is set, the trigger waits some time after activating before firing.

"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

"wait" : Seconds between triggerings. (.2 default)
"cnt" how many times it can be triggered (infinite default)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

set "message" to text string

******************************************************************************/

Event EV_TriggerBox_SetMins
(
	"mins",
	EV_SCRIPTONLY,
	"v",
	"mins",
	"Sets the minimum bounds of the trigger box."
);
Event EV_TriggerBox_SetMaxs
(
	"maxs",
	EV_SCRIPTONLY,
	"v",
	"maxs",
	"Sets the maximum bounds of the trigger box."
);

CLASS_DECLARATION( Trigger, TriggerBox, "trigger_box" )
{
	{ &EV_TriggerBox_SetMins,				&TriggerBox::SetMins },
	{ &EV_TriggerBox_SetMaxs,				&TriggerBox::SetMaxs },

	{ NULL, NULL }
};

void TriggerBox::SetMins( Event *ev )
{
	Vector org;
	
	mins = ev->GetVector( 1 );
	org = ( mins + maxs ) * 0.5f;
	
	setSize( mins - org, maxs - org );
	setOrigin( org );
}

void TriggerBox::SetMaxs( Event *ev )
{
	Vector org;
	
	maxs = ev->GetVector( 1 );
	org = ( mins + maxs ) * 0.5f;
	
	setSize( mins - org, maxs - org );
	setOrigin( org );
}

/*****************************************************************************/
/*QUAKED trigger_music (1 0 0) ? NORMAL ACTION NOT_PLAYERS MONSTERS PROJECTILES SUSPENSE MYSTERY SURPRISE

Variable sized repeatable trigger to change the music mood.

If "delay" is set, the trigger waits some time after activating before firing.
"current" can be used to set the current mood
"fallback" can be used to set the fallback mood
"altcurrent" can be used to set the current mood of the opposite face, if multiFaceted
"altfallback" can be used to set the fallback mood of the opposite face, if multiFaceted
"edgeTriggerable" trigger only fires when entering a trigger
"multiFaceted" if 1, then trigger is North/South separate triggerable\
if 2, then trigger East/West separate triggerable

"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
"wait" : Seconds between triggerings. (1.0 default)
"cnt" how many times it can be triggered (infinite default)
"oneshot" make this a one time trigger

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

NORMAL, ACTION, SUSPENSE, MYSTERY, and SURPRISE are the moods that can be triggered

******************************************************************************/

Event EV_TriggerMusic_CurrentMood
(
	"current",
	EV_SCRIPTONLY,
	"s",
	"current_mood",
	"Sets the current mood to use when triggered."
);
Event EV_TriggerMusic_FallbackMood
(
	"fallback",
	EV_SCRIPTONLY,
	"s",
	"fallback_mood",
	"Sets the fallback mood to use when triggered."
);
Event EV_TriggerMusic_AltCurrentMood
(
	"altcurrent",
	EV_SCRIPTONLY,
	"s",
	"alternate_current_mood",
	"Sets the alternate current mood to use when triggered."
);
Event EV_TriggerMusic_AltFallbackMood
(
	"altfallback",
	EV_SCRIPTONLY,
	"s",
	"alterante_fallback_mood",
	"Sets the alternate fallback mood to use when triggered."
);
Event EV_TriggerMusic_OneShot
(
	"oneshot",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Make this a one time trigger."
);

CLASS_DECLARATION( Trigger, TriggerMusic, "trigger_music" )
{
	{ &EV_TriggerMusic_CurrentMood,				&TriggerMusic::SetCurrentMood },
	{ &EV_TriggerMusic_FallbackMood,			&TriggerMusic::SetFallbackMood },
	{ &EV_TriggerMusic_AltCurrentMood,			&TriggerMusic::SetAltCurrentMood },
	{ &EV_TriggerMusic_AltFallbackMood,			&TriggerMusic::SetAltFallbackMood },
	{ &EV_TriggerMusic_OneShot,					&TriggerMusic::SetOneShot },
	{ &EV_Trigger_Effect,						&TriggerMusic::ChangeMood },
	{ &EV_Trigger_Effect_Alt,					&TriggerMusic::AltChangeMood },

	{ NULL, NULL }
};

TriggerMusic::TriggerMusic()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	removable         = false;
	triggerActivated	= false;
	activator			= NULL;
	trigger_time		= 0.0f;
	edgeTriggered     = true;
	
	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_TRIGGER );
	
	setContents( 0 );
	edict->svflags |= SVF_NOCLIENT;
	
	delay = 0;
	wait  = 1.0f;
	health = 0;
	max_health = 0;
	
	trigger_time = (float)0;
	
	SetOneShot( false );
	
	noise = "";
	respondto = spawnflags ^ TRIGGER_PLAYERS;
	
	current  = "normal";
	fallback = "normal";
	
	altcurrent  = "normal";
	altfallback = "normal";
	
	// setup sound based on spawn flags
	if ( spawnflags & 1 )
		current = "normal";
	else if ( spawnflags & 2 )
		current = "action";
	else if ( spawnflags & 32 )
		current = "suspense";
	else if ( spawnflags & 64 )
		current = "mystery";
	else if ( spawnflags & 128 )
		current = "surprise";
}

void TriggerMusic::SetMood( const str &crnt, const str &fback )
{
	current = crnt;
	fallback = fback;
}

void TriggerMusic::SetAltMood( const str &crnt, const str &fback )
{
	altcurrent = crnt;
	altfallback = fback;
}

void TriggerMusic::SetCurrentMood( Event *ev )
{
	current = ev->GetString( 1 );
}

void TriggerMusic::SetFallbackMood( Event *ev )
{
	fallback = ev->GetString( 1 );
}

void TriggerMusic::SetAltCurrentMood( Event *ev )
{
	altcurrent = ev->GetString( 1 );
}

void TriggerMusic::SetAltFallbackMood( Event *ev )
{
	altfallback = ev->GetString( 1 );
}

void TriggerMusic::ChangeMood( Event *ev )
{
	ChangeMusic( current.c_str(), fallback.c_str(), false );
}

void TriggerMusic::AltChangeMood( Event *ev )
{
	ChangeMusic( altcurrent.c_str(), altfallback.c_str(), false );
}

void TriggerMusic::SetOneShot( qboolean once )
{
	trigger_time = 0.0f;
	oneshot = once;
	if ( oneshot )
		count = 1;
	else
		count = -1;
}

void TriggerMusic::SetOneShot( Event *ev )
{
	SetOneShot( true );
}

/*****************************************************************************/
/*QUAKED trigger_reverb (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Variable sized repeatable trigger to change the reverb level in the game

If "delay" is set, the trigger waits some time after activating before firing.
"reverbtype" what kind of reverb should be used
"reverblevel" how much of the reverb effect should be applied
"altreverbtype" what kind of reverb should be used
"altreverblevel" how much of the reverb effect should be applied
"edgeTriggerable" trigger only fires when entering a trigger
"multiFaceted" if 1, then trigger is North/South separate triggerable\
if 2, then trigger East/West separate triggerable

"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
"wait" : Seconds between triggerings. (1.0 default)
"cnt" how many times it can be triggered (infinite default)
"oneshot" make this a one time trigger

"triggerable" turn trigger on
"nottriggerable" turn trigger off

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/

Event EV_TriggerReverb_ReverbType
(
	"reverbtype",
	EV_SCRIPTONLY,
	"i",
	"reverbType",
	"Sets the reverb type."
);
Event EV_TriggerReverb_ReverbLevel
(
	"reverblevel",
	EV_SCRIPTONLY,
	"f",
	"reverbLevel",
	"Sets the reverb level to be used when triggered."
);
Event EV_TriggerReverb_AltReverbType
(
	"altreverbtype",
	EV_SCRIPTONLY,
	"i",
	"reverbType",
	"Sets the reverb type."
);
Event EV_TriggerReverb_AltReverbLevel
(
	"altreverblevel",
	EV_SCRIPTONLY,
	"f",
	"reverbLevel",
	"Sets the reverb level to be used when triggered."
);
Event EV_TriggerReverb_OneShot
(
	"oneshot",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Make this a one time trigger."
);

CLASS_DECLARATION( Trigger, TriggerReverb, "trigger_music" )
{
	{ &EV_TriggerReverb_ReverbType,				&TriggerReverb::SetReverbType },
	{ &EV_TriggerReverb_ReverbLevel,			&TriggerReverb::SetReverbLevel },
	{ &EV_TriggerReverb_AltReverbType,			&TriggerReverb::SetAltReverbType },
	{ &EV_TriggerReverb_AltReverbLevel,			&TriggerReverb::SetAltReverbLevel },
	{ &EV_TriggerReverb_OneShot,				&TriggerReverb::SetOneShot },
	{ &EV_Trigger_Effect,						&TriggerReverb::ChangeReverb },
	{ &EV_Trigger_Effect_Alt,					&TriggerReverb::AltChangeReverb },

	{ NULL, NULL }
};

TriggerReverb::TriggerReverb()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	removable         = false;
	triggerActivated	= false;
	activator			= NULL;
	trigger_time		= (float)0;
	edgeTriggered     = true;
	
	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_TRIGGER );
	
	setContents( 0 );
	edict->svflags |= SVF_NOCLIENT;
	
	delay = 0;
	wait  = 1.0f;
	health = 0;
	max_health = 0;
	
	trigger_time = (float)0;
	
	SetOneShot( false );
	
	noise = "";
	respondto = spawnflags ^ TRIGGER_PLAYERS;
	
	reverbtype = 0;
	altreverbtype = 0;
	reverblevel = 0.5f;
	altreverblevel = 0.5f;
}

void TriggerReverb::SetReverb( int type, float level )
{
	reverbtype = type;
	reverblevel = level;
}

void TriggerReverb::SetAltReverb( int type, float level )
{
	altreverbtype = type;
	altreverblevel = level;
}

void TriggerReverb::SetReverbType( Event *ev )
{
	reverbtype = ev->GetInteger( 1 );
}

void TriggerReverb::SetReverbLevel( Event *ev )
{
	reverblevel = ev->GetFloat( 1 );
}

void TriggerReverb::SetAltReverbType( Event *ev )
{
	altreverbtype = ev->GetInteger( 1 );
}

void TriggerReverb::SetAltReverbLevel( Event *ev )
{
	altreverblevel = ev->GetFloat( 1 );
}

void TriggerReverb::ChangeReverb( Event *ev )
{
	Entity * other;
	
	other = ev->GetEntity( 1 );
	if ( other && other->isClient() )
	{
		Player *client;
		
		client = ( Player * )other;
		client->SetReverb( reverbtype, reverblevel );

		if ( !multiplayerManager.inMultiplayer() )
		{
			gi.DPrintf( "reverb set to %s at level %.2f\n", EAXMode_NumToName( reverbtype ), reverblevel );
		}
	}
}

void TriggerReverb::AltChangeReverb( Event *ev )
{
	Entity * other;
	
	other = ev->GetEntity( 1 );
	if ( other && other->isClient() )
	{
		Player *client;
		
		client = ( Player * )other;
		client->SetReverb( altreverbtype, altreverblevel );

		if ( !multiplayerManager.inMultiplayer() )
		{
			gi.DPrintf( "reverb set to %s at level %.2f\n", EAXMode_NumToName( altreverbtype ), altreverblevel );
		}
	}
}

void TriggerReverb::SetOneShot( qboolean once )
{
	trigger_time = 0.0f;
	oneshot = once;
	if ( oneshot )
		count = 1;
	else
		count = -1;
}

void TriggerReverb::SetOneShot( Event *ev )
{
	SetOneShot( true );
}

/*****************************************************************************/
/*QUAKED trigger_pushobject (1 0 0) ?
Special trigger that can only be triggered by a push object.

"triggername" if set, trigger only responds to objects with a targetname the same as triggername.
"cnt" how many times it can be triggered (default 1, use -1 for infinite)
******************************************************************************/

Event EV_TriggerByPushObject_TriggerName
(
	"triggername",
	EV_SCRIPTONLY,
	"s",
	"targetname_of_object",
	"If set, trigger will only respond to objects with specified name."
);

CLASS_DECLARATION( TriggerOnce, TriggerByPushObject, "trigger_pushobject" )
{
	{ &EV_TriggerByPushObject_TriggerName,			&TriggerByPushObject::setTriggerName },

	{ NULL, NULL }
};

void TriggerByPushObject::setTriggerName( Event *event )
{
	triggername = event->GetString( 1 );
}

qboolean TriggerByPushObject::respondTo( Entity *other )
{
	if ( other->isSubclassOf( PushObject ) )
	{
		if ( triggername.length() )
		{
			return ( triggername == other->TargetName() );
		}
		
		return true;
	}
	
	return false;
}

Entity *TriggerByPushObject::getActivator( Entity *other )
{
	Entity *owner;
	
	if ( other->isSubclassOf( PushObject ) )
	{
		owner = ( ( PushObject * )other )->getOwner();
		
		if ( owner )
		{
			return owner;
		}
	}
	
	return other;
}

/*****************************************************************************/
/*QUAKED trigger_givepowerup (1 0 0) ? x x NOT_PLAYERS MONSTERS x

Variable sized repeatable trigger to give a powerup to the player

"oneshot" makes this triggerable only once
"powerupname" sets the name of the powerup to give to the player

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters

******************************************************************************/

Event EV_TriggerGivePowerup_OneShot
(
	"oneshot",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Make this a one time trigger."
);
Event EV_TriggerGivePowerup_PowerupName
(
	"powerupname",
	EV_SCRIPTONLY,
	"s",
	"powerup_name",
	"Specifies the powerup to give to the sentient."
);

CLASS_DECLARATION( Trigger, TriggerGivePowerup, "trigger_givepowerup" )
{
	{ &EV_TriggerGivePowerup_OneShot,				&TriggerGivePowerup::SetOneShot },
	{ &EV_TriggerGivePowerup_PowerupName,			&TriggerGivePowerup::SetPowerupName },
	{ &EV_Trigger_Effect,							&TriggerGivePowerup::GivePowerup },

	{ NULL, NULL }
};

TriggerGivePowerup::TriggerGivePowerup()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	removable         = false;
	triggerActivated	= false;
	activator			= NULL;
	trigger_time		= (float)0;
	edgeTriggered     = false;
	
	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_TRIGGER );
	
	setContents( 0 );
	edict->svflags |= SVF_NOCLIENT;
	
	delay = 0;
	wait  = 1.0f;
	health = 0;
	max_health = 0;
	
	trigger_time = 0.0;
	oneshot = false;
	count = -1;
	
	noise = "";
	respondto = spawnflags ^ (TRIGGER_PLAYERS | TRIGGER_MONSTERS );
}

void TriggerGivePowerup::SetOneShot( Event *ev )
{
	trigger_time = 0.0f;
	oneshot = true;
	count = 1;
}

void TriggerGivePowerup::SetPowerupName( Event *ev )
{
	powerup_name = ev->GetString( 1 );
}

void TriggerGivePowerup::GivePowerup( Event *ev )
{
	Entity *other;
	
	other = ev->GetEntity( 1 );
	
	if ( other->isSubclassOf( Sentient ) )
	{
		Sentient *sent;
		
		sent = ( Sentient * )other;
		
		if ( powerup_name.length() && !sent->FindItem( powerup_name.c_str() ) )
			sent->giveItem( powerup_name );
	}
}

/*****************************************************************************/
/*QUAKED trigger_worktrigger (1 0 0) ?

Trigger will be used to tell the AI how to do "work".  When the AI activates
trigger, the thread you specifiy will execute.  The AI will also play the 
Animation you set in the trigger.

"triggerable" turn trigger on
"nottriggerable" turn trigger off
"setworkanim" Sets the animation the Actor will use when "working"


******************************************************************************/

Event EV_TriggerSetWorkAnimation
(
	"setworkanim",
	EV_SCRIPTONLY,
	"s",
	"work_anim",
	"Tells the actor to play this specified animation when using the trigger"
);
Event EV_TriggerSetWorkTime
(
	"setworktime",
	EV_SCRIPTONLY,
	"f",
	"work_time",
	"sets how long to work at node ( -1 ) is infinite"
);
Event EV_TriggerAddWorker
(
	"addworker",
	EV_SCRIPTONLY,
	"s",
	"worker_name",
	"adds a valid worker to the trigger"
);

CLASS_DECLARATION( Trigger, WorkTrigger, "trigger_worktrigger" )
{
	{ &EV_Use,									&WorkTrigger::TriggerStuff },
	{ &EV_TriggerSetWorkAnimation,				&WorkTrigger::SetAnimation },
	{ &EV_TriggerSetWorkTime,					&WorkTrigger::SetTime      },
	{ &EV_TriggerAddWorker,						&WorkTrigger::AddWorker    },

	{ NULL, NULL }
};

WorkTrigger::WorkTrigger()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	_time = 0.0f;
	_reserved = false;
	_currentEnt = 0;
	
	respondto = spawnflags ^ ( TRIGGER_MONSTERS );
}

void WorkTrigger::SetAnimation( Event *ev )
{
	_animation = ev->GetString( 1 );
}

str WorkTrigger::GetAnimation()
{
	return _animation;
}

void WorkTrigger::SetTime( Event *ev )
{
	_time = ev->GetFloat( 1 );
}

float WorkTrigger::GetTime()
{
	return _time;
}

void WorkTrigger::AddWorker( Event *ev )
{
	str workerName;
	str name;
	
	workerName = ev->GetString( 1 );
	
	for ( int i = 1 ; i <= _allowedWorkers.NumObjects() ; i++ )
	{
		name = _allowedWorkers.ObjectAt( i );
		if ( !Q_stricmp( name.c_str() , workerName.c_str() ) )
			return;
	}
	
	_allowedWorkers.AddObject( workerName );
}

qboolean WorkTrigger::isAllowedToWork( const str &name , int entNum )
{
	str workerName;
	
	if ( _reserved && entNum != _currentEnt )
		return false;
	
	if ( _allowedWorkers.NumObjects() < 1 )
		return true;
	
	
	for ( int i = 1 ; i <= _allowedWorkers.NumObjects() ; i++ )
	{
		workerName = _allowedWorkers.ObjectAt( i );
		if ( !Q_stricmp( name.c_str() , workerName.c_str() ) )
			return true;
	}
	
	return false;
}

qboolean WorkTrigger::isReserved()
{
	return _reserved;
}

void WorkTrigger::Reserve(int entNum)
{
	_reserved = true;
	_currentEnt = entNum;
}

void WorkTrigger::Release()
{
	_reserved = false;
	_currentEnt = 0;
}

/*****************************************************************************/
/*QUAKED trigger_levelinteraction (1 0 0) ?

Trigger that will be damaged by the AI to fire off a "level_interaction" thread
for example, a boss that shoots crates and knocks them over onto players

"triggerable" turn trigger on
"nottriggerable" turn trigger off
"health" should be set so that the trigger can take damage

******************************************************************************/
CLASS_DECLARATION( Trigger, LevelInteractionTrigger, "trigger_levelinteraction" )
{
	{ NULL, NULL }
};

LevelInteractionTrigger::LevelInteractionTrigger()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	respondto = spawnflags ^ ( TRIGGER_MONSTERS | TRIGGER_PROJECTILES );
}

qboolean LevelInteractionTrigger::respondTo( Entity *other )
{
	if (other->isClient()) return false;
	if (other->isSubclassOf( Actor)) return true;
	
	if (other->isSubclassOf( Projectile ) )
	{
		Projectile* proj = (Projectile*)other;
		Entity* ent = G_GetEntity(proj->owner);
		
		if (ent->isSubclassOf( Actor ) ) return true;		  
		
	}
	
	return false;
}

/*****************************************************************************/
/*QUAKED trigger_groupevent (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Variable sized repeatable trigger.  Must be targeted at one or more entities.

If "health" is set, the trigger must be killed to activate each time.
If "delay" is set, the trigger waits some time after activating before firing.

"thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.

if "angle" is set, the trigger will only fire when someone is facing the
direction of the angle.
"cone" the cone in which a directed trigger can be triggered (default 60 degrees)

"wait" : Seconds between triggerings. (.2 default)
"cnt" how many times it can be triggered (infinite default)

"triggerable" turn trigger on
"nottriggerable" turn trigger off

"setpassevent" Sets the event you wish to pass to the group.  
"setgroupnumber" Sets the number of the group you wish to pass the event to.

If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

set "message" to text string

******************************************************************************/

CLASS_DECLARATION( Trigger, TriggerGroupEvent, "trigger_groupevent" )
{
	{ &EV_Trigger_SetPassEvent,					&TriggerGroupEvent::SetPassEvent   },
	{ &EV_Trigger_SetGroupNumber,				&TriggerGroupEvent::SetGroupNumber },
	{ &EV_Trigger_ActivateTargets,				&TriggerGroupEvent::PassEvent      },      
	{ &EV_Activate,								&TriggerGroupEvent::TriggerStuff   },

	{ NULL, NULL }
};

TriggerGroupEvent::TriggerGroupEvent()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	groupNumber = 0;
	respondto = spawnflags ^ TRIGGER_PLAYERS;
}

void TriggerGroupEvent::TriggerStuff( Event *ev )
{
	Trigger::TriggerStuff( ev );
}

void TriggerGroupEvent::SetPassEvent( Event *ev )
{
	//Events coming from radiant like this one, seem to
	//come as one string, regardless of how many parameters
	//there are, so we're going to have to parse the string ourselves;
	str eventString;
	str character;
	str parameter;
	int len;
	int i;
	
	eventString = ev->GetString( 1 );
	parameter = "";
	len = eventString.length();
	
	
	for ( i = 0 ; i < len ; i++ )
	{
		character = eventString[i];
		
		if ( character != " " )
			parameter += character;
		else
		{
			passEvent.AddToken( parameter.c_str() );
			parameter = "";
		}
		
	}
	
	//Add The Last Parameter
	if ( parameter.length() )
		passEvent.AddToken( parameter.c_str() );
	
}

void TriggerGroupEvent::SetGroupNumber( Event *ev )
{
	groupNumber = ev->GetInteger( 1 );
}

void TriggerGroupEvent::PassEvent( Event *ev )
{
	//groupeventmanager->SendEvent( groupNumber, passEvent );
	
	Event *newEvent;
    newEvent = new Event ( passEvent.GetString( 1 ) );
    for ( int j=2; j<= passEvent.NumArgs() ; j++ )
	{
        newEvent->AddToken( passEvent.GetToken( j ) );
	}
	
	groupcoordinator->SendEventToGroup( newEvent, groupNumber );
	
}

/*****************************************************************************/
/*QUAKED trigger_volume_callvolume (1 0 0) ?

Allows you to specify a list of actors that are required to be inside this
volume before the thread fires.  This is a good utility if you need to "herd"
a group of actors into one spot for an event to occur ( Such as Teammates and
Lifts ).  When the player enters this trigger, a flag is set on all the actors
in the required list that notifies them that the player is in the volume.  The AI
for those actors is then responsible for getting them into the volume.  Once
all the required actors that are still in the level ( in case some were killed or
otherwise removed ) the specified thread will fire


******************************************************************************/

//==============================================================================
//                        TriggerCallVolume
//==============================================================================

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------

CLASS_DECLARATION( Trigger, TriggerCallVolume, "trigger_volume_callvolume" )
{
	{ &EV_Touch,								&TriggerCallVolume::TriggerStuff      },
	{ &EV_Trigger_SetRequiredEntity,			&TriggerCallVolume::AddRequiredEntity },
	{ &EV_Trigger_TriggerExit,					&TriggerCallVolume::EntityLeftVolume  },
	{ &EV_Trigger_SetExitThread,				&TriggerCallVolume::SetExitThread	  },
	{ &EV_Trigger_CheckReady,					&TriggerCallVolume::CheckReady        },
	{ &EV_Trigger_SetTriggerable,				&TriggerCallVolume::SetTriggerable },
	{ &EV_Trigger_SetNotTriggerable,			&TriggerCallVolume::SetNotTriggerable },

	{ NULL, NULL	}
};


//--------------------------------------------------------------
// Name:        TriggerCallVolume()
// Class:       TriggerCallVolume
//
// Description: Constructor
//
// Parameters:  None
//
// Returns:     None
//--------------------------------------------------------------
TriggerCallVolume::TriggerCallVolume()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}   
	
	respondto = spawnflags ^ (TRIGGER_PLAYERS | TRIGGER_MONSTERS );
	_ready = false;
}

//--------------------------------------------------------------
// Name:        ~TriggerCallVolume()
// Class:        TriggerCallVolume
//
// Description:  Destructor
//
// Parameters:   None
//
// Returns:      None
//--------------------------------------------------------------
TriggerCallVolume::~TriggerCallVolume()
{
	_requiredEntities.FreeObjectList();
}


//--------------------------------------------------------------
// Name:        TriggerStuff()
// Class:       TriggerCallVolume
//
// Description: Checks if the trigger is ready ( i.e. all available
//              required entites are within the volume ) then
//              calls the base class TriggerStuff()
//
// Parameters:  Event *ev
//
// Returns:     None
//--------------------------------------------------------------
void TriggerCallVolume::TriggerStuff( Event *ev )
{
	Entity *other;   
	Actor  *actor;
	
	other  = NULL;   
	actor  = NULL;
	
	if (!CanTrigger()) return;
	
	if ((int)*ev == (int)EV_Damage)
	{
		other = ev->GetEntity(3);
	}
	else
	{
		other = ev->GetEntity( 1 );
	}
	
	assert( other != this );
	
	// Always respond to activate messages from the world since they're probably from
	// the "trigger" command
	if ( !respondTo( other ) && !( ( other == world ) && ( ( int )*ev == ( int )EV_Activate ) ) &&
		( !other || !other->isSubclassOf( Camera ) ) )
	{
		return;
	}
	
	if ( _ready )
	{
		Trigger::TriggerStuff( ev );
		_ready = false;
		return;
	}
	
	//If we got here, we're still waiting on additional entities to arrive, so
	//we'll repost our check event so that we can continue to monitor
	Event *checkEvent;
	checkEvent = new Event(EV_Trigger_CheckEntList);
	checkEvent->AddInteger( other->entnum );
	PostEvent( checkEvent , CHECK_TIME );
	
	int testEntNum;
	for ( int i = 1 ; i <= entList.NumObjects() ; i++ )
	{
		testEntNum = entList.ObjectAt( i );

		if ( testEntNum == other->entnum && !_ready )
			return;
	}
	
	AddOtherToEntList( other );
	
	if ( other->isSubclassOf( Player ) )
	{      
		_checkForRequiredEnts();
		_notifyRequiredEnts(true);

		Event* checkEV;
		checkEV = new Event(EV_Trigger_CheckReady);
		PostEvent( checkEV , CHECK_TIME );
	}
	
	
	if ( other->isSubclassOf( Actor ) && _isRequiredEnt( other ) )
	{
		actor = (Actor*)other;
		actor->SetActorFlag( ACTOR_FLAG_IN_CALL_VOLUME , true );
		
		Event* checkEV;
		checkEV = new Event(EV_Trigger_CheckReady);
		PostEvent( checkEV , CHECK_TIME );
	}            
}


//--------------------------------------------------------------
// Name:        AddRequiredEntity() 
// Class:       TriggerCallVolume
//
// Description: Adds the targetname of the entity to the required
//              entity list
//
// Parameters:  Event *ev
//
// Returns:     None
//--------------------------------------------------------------
void TriggerCallVolume::AddRequiredEntity( Event *ev )
{
	str name;
	str entName;
	entName = ev->GetString( 1 );
	
	// Check if name has already been added
	for ( int i = 1; i < _requiredEntities.NumObjects() ; i++ )
	{
		name = _requiredEntities.ObjectAt( i );
		
		if ( !Q_stricmp( name.c_str() , entName.c_str() ) )
			return;
		
	}
	
	_requiredEntities.AddObject ( entName );
}

//--------------------------------------------------------------
// Name:        _getEntity()
// Class:       TriggerCallVolume
//
// Description: Exchanges the targetname for an Entity Pointer
//
// Parameters:  const str &name -- The targetname
//
// Returns:     Entity* -- The entity with the specified targetname
//--------------------------------------------------------------
Entity* TriggerCallVolume::_getEntity( const str &name )
{
	Entity* ent_in_range;
	gentity_t *ed;
	
	for ( int i = 0; i < MAX_GENTITIES; i++ )
	{
		ed = &g_entities[i];
		
		if ( !ed->inuse || !ed->entity )		
			continue;
		
		
		ent_in_range = g_entities[i].entity;
		
		if (!Q_stricmp(ent_in_range->targetname.c_str() , name.c_str() ))			
			return ent_in_range;			
		
	}
	
	return NULL;
}

//--------------------------------------------------------------
// Name:        EntityLeftVolume()
// Class:       TriggerCallVolume
//
// Description: Clears the ACTOR_FLAG_IN_CALL_VOLUME flag on all
//              the actors in the required list
//
// Parameters:  Event *ev
//
// Returns:     None
//--------------------------------------------------------------
void TriggerCallVolume::EntityLeftVolume( Event *ev )
{
	Entity *other;   
	Actor  *actor;
	Player *player;
	
	other = NULL;
	other = ev->GetEntity( 1 );
	
	if ( !other )
		return;
	
	if ( other->isSubclassOf( Player ) )
	{
		_notifyRequiredEnts(false);
		
		// Null out the currentCallVolume on the Player		
		player = ( Player* )other;
		player->SetCurrentCallVolume( TargetName() );

		if ( _exitThread.length() )
			{
			if ( !ExecuteThread( _exitThread , true , this  ) )
				warning( "StartThread", "Null game script" );
			}
			
			int entNum;
			for ( int i = entList.NumObjects() ; i > 0  ; i-- )
			{
			entNum = entList.ObjectAt( i );
			if ( entNum == other->entnum )
				{
				entList.RemoveObjectAt( i );
				}
				
			}
	}
	
	if ( other->isSubclassOf( Actor ) )
	{
		actor = (Actor*)other;
		actor->SetActorFlag( ACTOR_FLAG_IN_CALL_VOLUME , false );
	}
}

//--------------------------------------------------------------
// Name:        CheckReady()
// Class:       TriggerCallVolume
//
// Description: Checks if all the required entities are within
//              the volume
//
// Parameters:  Event *ev
//
// Returns:     None
//--------------------------------------------------------------
void TriggerCallVolume::CheckReady( Event *ev )
{
	str name;
	Entity *ent;   
	Player *player;
	
	_ready = true;
	
	player = GetPlayer( 0 );
	
	if ( !player )
	{
		_ready = false;
		return;
	}
	
	if ( !IsEntityInBoundingBox( player ) )
	{
		_ready = false;
		return;
	}
	
	for ( int i = 1; i <= _requiredEntities.NumObjects() ; i++ )
	{
		name = _requiredEntities.ObjectAt( i );
		ent = _getEntity( name );
		
		// If we have an entity and it's not in the bounding box
		// we are false.  If don't have an entity at all we are
		// also false
		if ( ent )
		{
			if ( !IsEntityInBoundingBox( ent ) )
				_ready = false;         
		}
	}   
}

//--------------------------------------------------------------
// Name:        _notifyRequiredEnts()
// Class:       TriggerCallVolume
//
// Description: Sets the ACTOR_FLAG_PLAYER_IN_CALL_VOLUME flag
//              on all the actors in the required list
//
// Parameters:  None
//
// Returns:     None
//--------------------------------------------------------------
void TriggerCallVolume::_notifyRequiredEnts(bool inCallVolume )
{
	str name;
	Entity *ent; 
	Actor  *actor;
	
	// Set the currentCallVolume on the Player
	Player *player;
	player = GetPlayer( 0 );
	player->SetCurrentCallVolume( TargetName() );
	
	for ( int i = 1; i <= _requiredEntities.NumObjects() ; i++ )
	{
		name = _requiredEntities.ObjectAt( i );
		ent = _getEntity( name );
		
		if ( ent && ent->isSubclassOf(Actor ) )
		{
			actor = (Actor*)ent;
			actor->SetActorFlag(ACTOR_FLAG_PLAYER_IN_CALL_VOLUME , inCallVolume );            
		}
	}      
}

//--------------------------------------------------------------
// Name:        _checkForRequiredEnts()
// Class:       TriggerCallVolume
//
// Description: Checks if we have any required actors available
//
// Parameters:  None
//
// Returns:     None
//--------------------------------------------------------------
void TriggerCallVolume::_checkForRequiredEnts()
{
	str name;
	Entity *ent; 
	qboolean haveEnts;
	
	haveEnts = false;
	for ( int i = 1; i <= _requiredEntities.NumObjects() ; i++ )
	{
		name = _requiredEntities.ObjectAt( i );
		ent = _getEntity( name );
		
		if ( ent )
		{
			haveEnts = true;
		}
	}         
	
	if ( !haveEnts )
		_ready = true;
}

//--------------------------------------------------------------
// Name:        _isRequiredEnt()
// Class:       TriggerCallVolume
//
// Description: Checks if a entity is in the required list
//
// Parameters:  Entity *other
//
// Returns:     True or False
//--------------------------------------------------------------
bool TriggerCallVolume::_isRequiredEnt(Entity *other )
{
	str name;
	Entity *ent;  
	
	
	for ( int i = 1; i <= _requiredEntities.NumObjects() ; i++ )
	{
		name = _requiredEntities.ObjectAt( i );
		ent = _getEntity( name );
		
		if ( ent )
		{
			if ( !Q_stricmp(ent->targetname.c_str() , other->targetname.c_str() ) )
				return true;
		}
	}         
	
	return false;
}

/*****************************************************************************/
/*QUAKED trigger_EntryAndExit (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Triggers entryThread when entity enters the trigger, calls thread while inside,
and calls exitThread when entity leaves the trigger.

Key Value Pairs:
thread <ThreadName>
entryThread <EntryThreadName>
exitThread <ExitThreadName>

******************************************************************************/


CLASS_DECLARATION( Trigger, TriggerEntryAndExit, "trigger_EntryAndExit" )
{
	{ &EV_Touch,								&TriggerEntryAndExit::TriggerStuff		},
	{ &EV_Trigger_TriggerExit,					&TriggerEntryAndExit::EntityLeftVolume	},
	{ &EV_Trigger_SetEntryThread,				&TriggerEntryAndExit::SetEntryThread		},
	{ &EV_Trigger_SetExitThread,				&TriggerEntryAndExit::SetExitThread		},
	{ &EV_Trigger_AltSetEntryThread,			&TriggerEntryAndExit::SetEntryThread		},
	{ &EV_Trigger_AltSetExitThread,				&TriggerEntryAndExit::SetExitThread		},
	{ &EV_Trigger_SetEnter,						&TriggerEntryAndExit::EnterTrigger		},
	{ &EV_Trigger_EntityExit,					&TriggerEntryAndExit::ExitTrigger			},

	// 1ST PLAYABLE HACK STUFF
	{ &EV_Trigger_Hack_SetTriggerParms,			&TriggerEntryAndExit::HackSetParms				},
	{ &EV_Trigger_Hack_GetForceFieldNumber,		&TriggerEntryAndExit::HackGetForceFieldTrigger	},
	{ &EV_Trigger_Hack_GetTriggerNumber,		&TriggerEntryAndExit::HackGetTriggerNumber		},

	{ NULL, NULL	}
};

//--------------------------------------------------------------
// Name:		TriggerEntryAndExit()
// Class:		TriggerEntryAndExit
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
TriggerEntryAndExit::TriggerEntryAndExit()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	// 1ST PLAYABLE HACK
	_forcefieldtrigger = -1;
	_triggernumber     = -1;
	//========================
	
	respondto = spawnflags ^ TRIGGER_PLAYERS;
	_entered = false;
}

//--------------------------------------------------------------
// Name:		~TriggerEntryAndExit()
// Class:		TriggerEntryAndExit
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
TriggerEntryAndExit::~TriggerEntryAndExit()
{
}

//--------------------------------------------------------------
// Name:		TriggerStuff()
// Class:		TriggerEntryAndExit
//
// Description:	Overloaded TriggerStuff Function, handles custom
//				stuff then calls the base class version
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void TriggerEntryAndExit::TriggerStuff( Event *ev )
{
	Entity *other;   
	//Actor  *actor;
	Event *cancelEvent;   
	int	ID;
	
	other  = NULL;   
	//actor  = NULL;
	ID = GetGroupID();
	
	if ((int)*ev == (int)EV_Damage)
	{
		other = ev->GetEntity(3);
	}
	else
	{
		other = ev->GetEntity( 1 );
	}
	assert( other != this );
	
	// Always respond to activate messages from the world since they're probably from
	// the "trigger" command
	if ( !respondTo( other ) && !( ( other == world ) && ( ( int )*ev == ( int )EV_Activate ) ) &&
		( !other || !other->isSubclassOf( Camera ) ) )
	{
		return;
	}	
	
	cancelEvent = new Event ( EV_Trigger_TriggerExit );
	groupcoordinator->GroupCancelEventsOfType( cancelEvent , ID );
	
	if ( !_entered )
	{	  
		Event *enteredEvent;
		enteredEvent = new Event ( EV_Trigger_SetEnter );
		groupcoordinator->SendEventToGroup( enteredEvent , ID );
		if ( _entryThread.length() )
		{
			if ( !ExecuteThread( _entryThread , true, this ) )
			{
				warning( "StartThread", "Null game script" );
			}
		}
		
		_entered = true;
	}
	
	Trigger::TriggerStuff( ev );
	return;
}

//--------------------------------------------------------------	
// Name:		EntityLeftVolume()
// Class:		TriggerEntryAndExit
//
// Description:	Gets called when the trigger detects that an
//				entity has left
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void TriggerEntryAndExit::EntityLeftVolume( Event *ev )
{
	Event *exitEvent;
	exitEvent = new Event( EV_Trigger_EntityExit );
	groupcoordinator->SendEventToGroup( exitEvent, GetGroupID() );
	
	if ( _exitThread.length() )
	{
		if ( !ExecuteThread( _exitThread , true , this  ) )
		{
			warning( "StartThread", "Null game script" );
		}
	}
	
	_entered = false;
}

//=====================================================
// 1ST PLAYABLE HACKES -- REMOVE ME, PLEASE
//=====================================================
void TriggerEntryAndExit::HackSetParms( Event *ev )
{
	_forcefieldtrigger = ev->GetFloat( 1 );
	_triggernumber     = ev->GetFloat( 2 );
}

void TriggerEntryAndExit::HackGetForceFieldTrigger( Event *ev )
{
	ev->ReturnFloat( _forcefieldtrigger );
}

void TriggerEntryAndExit::HackGetTriggerNumber( Event *ev )
{
	ev->ReturnFloat( _triggernumber );
}



//---------------------------------------------------------
//             T R I G G E R   V O L U M E
//---------------------------------------------------------
/*****************************************************************************/
/*QUAKED trigger_volume (1 0 0) ? x x NOT_PLAYERS MONSTERS PROJECTILES

Triggers entryThread when entity enters the trigger, calls thread while inside,
and calls exitThread when entity leaves the trigger.

Key Value Pairs:
thread <ThreadName>
entryThread <EntryThreadName>
exitThread <ExitThreadName>

******************************************************************************/

CLASS_DECLARATION( Trigger, TriggerVolume, "trigger_volume" )
{
	{ &EV_Touch,						NULL						},
	{ &EV_Contact,						&TriggerVolume::HandleContactEvent			},
	{ &EV_LostContact,					&TriggerVolume::HandleLostContactEvent		},
	{ &EV_Trigger_SetEntryThread,		&TriggerVolume::HandleSetEntryThreadEvent	},
	{ &EV_Trigger_SetExitThread,		&TriggerVolume::HandleSetExitThreadEvent	},

	{ NULL, NULL	}
};



//===============================================================
// Name:		HandleSetEntryThread
// Class:		TriggerVolume
//
// Description: Handles the SetEntryThread event and sets the entry
//				thread to the specified thread name.
// 
// Parameters:	Event* -- the event that triggered this
//
// Returns:		None
// 
//===============================================================
void TriggerVolume::HandleSetEntryThreadEvent( Event *ev )
{
	assert( ev );

	if ( ev->NumArgs() != 1 ) 
	{
		ev->Error("No entry threadname specified\n");
		return ;
	}

	_entryThread = ev->GetString( 1 );
}



//===============================================================
// Name:		HandleSetExitThread
// Class:		TriggerVolume
//
// Description: Handles the SetExitThread event and sets the exit
//				thread to the specified thread name.
// 
// Parameters:	Event* -- the event that triggered this
//
// Returns:		None
// 
//===============================================================
void TriggerVolume::HandleSetExitThreadEvent( Event *ev )
{
	assert( ev );

	if ( ev->NumArgs() != 1 ) 
	{
		ev->Error("No exit threadname specified\n");
		return ;
	}

	_exitThread = ev->GetString( 1 );
}


//===============================================================
// Name:		HandleContactEvent
// Class:		TriggerVolume
//
// Description: Handles an EV_Contact event.  This function calls
//				its entry_thread (if set) whenever an entity enters
//				the event for the first time.
// 
// Parameters:	Event -- the event that triggered this call.  The
//						 first argument must be an entity.
//
// Returns:		None
// 
//===============================================================
void TriggerVolume::HandleContactEvent( Event *ev )
{
	assert( ev );


	if (_entryThread.length())
	{
		Entity *entityTouched = ev->GetEntity( 1 );
		ExecuteThread( _entryThread, true, entityTouched );
		return ;
	}

	Trigger::TriggerStuff( ev );
}


//===============================================================
// Name:		HandleTriggerExitEvent
// Class:		TriggerVolume
//
// Description: Handles an EV_LostContact event.  Called when we
//				lose contact with an entity.  This function calls
//				an exit thread if set.
// 
// Parameters:	Event* -- the event that triggered this
//
// Returns:		None
// 
//===============================================================
void TriggerVolume::HandleLostContactEvent( Event *ev )
{
	if (_exitThread.length())
	{
		Entity *entityLost = ev->GetEntity( 1 );
		ExecuteThread( _exitThread, true, entityLost );
		return ;
	}

	Trigger::TriggerStuff( ev );
}

/*****************************************************************************/
/*QUAKED trigger_ladder (1 0 0) ? x x NOT_PLAYERS MONSTERS

Tells any entity that touches it that it is on a ladder

Angle/Angles specifies the direction away from the ladder

******************************************************************************/

CLASS_DECLARATION( Trigger, TriggerLadder, "trigger_ladder" )
{
	{ &EV_Touch,						&TriggerLadder::handleTouchEvent	},

	{ NULL, NULL	}
};

void TriggerLadder::handleTouchEvent( Event *ev )
{
	Player *player;
	Entity *entity;

	if( !triggerable )
		return;
	
	entity = ev->GetEntity( 1 );

	if ( !entity || !entity->isSubclassOf( Player ) )
		return;

	player = (Player *)entity;

	player->touchingLadder( this, triggerDir, absmax[2] );
}
