//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/explosion.cpp                             $
// $Revision:: 20                                                             $
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
// Standard explosion object that is spawned by other entites and not map designers.
// Explosion is used by many of the weapons for the blast effect, but is also used
// by the Exploder and MultiExploder triggers.  These triggers create one or more
// explosions each time they are activated.
//

#include "_pch_cpp.h"
#include "entity.h"
#include "trigger.h"
#include "explosion.h"
#include "weaputils.h"
#include <qcommon/gameplaymanager.h>

#define MULTI_USE       (1<<0)
#define RANDOM_TIME     (1<<1)
#define VISIBLE         (1<<2)
#define RANDOM_SCALE    (1<<3)
#define NO_EXPLOSIONS   (1<<4)

Event EV_Exploder_SetDmg
(
	"dmg",
	EV_SCRIPTONLY,
	"i",
	"damage",
	"Sets the damage the explosion does."
);
Event EV_Exploder_SetDuration
(
	"duration",
	EV_SCRIPTONLY,
	"f",
	"duration",
	"Sets the duration of the explosion."
);
Event EV_Exploder_SetWait
(
	"wait",
	EV_SCRIPTONLY,
	"f",
	"explodewait",
	"Sets the wait time of the explosion."
);
Event EV_Exploder_SetRandom
(
	"random",
	EV_DEFAULT,
	"f",
	"randomness",
	"Sets the randomness value of the explosion."
);

void CreateExplosion( const Vector &pos, float damage, Entity *inflictor, Entity *attacker, Entity *ignore, 
		const char *explosionModel, float scale, float radius )
{
	Explosion   *explosion;
	Event       *ev;
	
	assert( inflictor );
	
	if ( !inflictor )
	{
		return;
	}
	
	if ( !attacker )
	{
		attacker = world;
	}
	
	if ( !explosionModel )
	{
		explosionModel = "fx_explosion.tik";
	}
	
	explosion = new Explosion;
	
	// Create a new explosion entity and set it off
	explosion->setModel( explosionModel );
	
	explosion->setSolidType( SOLID_NOT );
	explosion->CancelEventsOfType( EV_ProcessInitCommands );
	explosion->ProcessInitCommands( explosion->edict->s.modelindex );
	
	explosion->owner             = attacker->entnum;
	explosion->edict->ownerNum   = attacker->entnum;
	explosion->setMoveType( MOVETYPE_FLYMISSILE );
	explosion->edict->clipmask = MASK_PROJECTILE;
	explosion->setSize( explosion->mins, explosion->maxs );
	explosion->setOrigin( pos );
	explosion->origin.copyTo( explosion->edict->s.origin2 );
	
	explosion->setScale( scale );
	
	if ( explosion->dlight_radius )
	{
		G_SetConstantLight( &explosion->edict->s.constantLight,
			&explosion->dlight_color[0],
			&explosion->dlight_color[1],
			&explosion->dlight_color[2],
			&explosion->dlight_radius
			);
	}
	
	explosion->BroadcastSound();
	
	explosion->animate->RandomAnimate( "idle", EV_StopAnimating );
	RadiusDamage( inflictor, attacker, damage, ignore, MOD_EXPLOSION, radius );
	
	if ( explosion->life )
	{
		ev = new Event( EV_Remove );
		explosion->PostEvent( ev, explosion->life );
	}
}

/*****************************************************************************/
/*QUAKED func_exploder (0 0.25 0.5) (0 0 0) (8 8 8)

  Spawns an explosion when triggered.  Triggers any targets.

  "dmg" specifies how much damage to cause. Default indicates 120.
  "key" The item needed to activate this. (default nothing)
  "thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
******************************************************************************/

CLASS_DECLARATION( Trigger, Exploder, "func_exploder" )
{
	{ &EV_Touch,								NULL },
	{ &EV_Trigger_Effect,						&Exploder::MakeExplosion },
	{ &EV_Exploder_SetDmg,						&Exploder::SetDmg },
	{ &EV_SetHealth,							&Exploder::SetHealth },
	{ &EV_SetGameplayDamage,					&Exploder::setDamage },

	{ NULL, NULL }
};

//--------------------------------------------------------------
//
// Name:			setDamage
// Class:			Exploder
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
void Exploder::setDamage( Event *ev )
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
	Event *newev = new Event(EV_Exploder_SetDmg);
	newev->AddInteger((int)finaldamage);
	ProcessEvent(newev);
}


void Exploder::MakeExplosion( Event *ev )
{
	CreateExplosion( origin, damage, this, ev->GetEntity( 1 ), this );
}

Exploder::Exploder()
{
	damage      = 120.0f;
	respondto   = TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES;
}

void Exploder::SetDmg( Event *ev )
{
	damage = ev->GetInteger( 1 );
	if ( damage < 0 )
	{
		damage = 0;
	}
}

void Exploder::SetHealth( Event *ev )
{
	Trigger::EventSetHealth(ev);
	triggerondeath = true ;
}

/*****************************************************************************/
/*QUAKED func_multi_exploder (0 0.25 0.5) ? MULTI_USE RANDOM_TIME VISIBLE RANDOM_SCALE

  Spawns an explosion when triggered.  Triggers any targets.
  size of brush determines where explosions will occur.

  "dmg" specifies how much damage to cause from each explosion. (Default 120)
  "delay" delay before exploding (Default 0 seconds)
  "duration" how long to explode for (Default 1 second)
  "wait" time between each explosion (default 0.25 seconds)
  "random" random factor (default 0.25)
  "key" The item needed to activate this. (default nothing)
  "thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
  "health" makes the object damageable
  "scale" set the maximum size for spawned debris and explosions.

  MULTI_USE allows the func_multi_exploder to be used more than once
  RANDOM_TIME adjusts the wait between each explosion by the random factor
  VISIBLE allows you to make the trigger visible
  RANDOM_SCALE scale explosions randomly. size will be between 0.25 and 1 times scale

******************************************************************************/

CLASS_DECLARATION( Trigger, MultiExploder, "func_multi_exploder" )
{
	{ &EV_Touch,									NULL },
	{ &EV_Trigger_Effect,							&MultiExploder::MakeExplosion },
	{ &EV_Exploder_SetDmg,							&MultiExploder::SetDmg },
	{ &EV_Exploder_SetDuration,						&MultiExploder::SetDuration },
	{ &EV_Exploder_SetWait,							&MultiExploder::SetWait },
	{ &EV_Exploder_SetRandom,						&MultiExploder::SetRandom },
	{ &EV_SetHealth,								&MultiExploder::SetHealth },
	{ &EV_SetGameplayDamage,						&MultiExploder::setDamage },

	{ NULL, NULL }
};


//--------------------------------------------------------------
//
// Name:			setDamage
// Class:			MultiExploder
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
void MultiExploder::setDamage( Event *ev )
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
	Event *newev = new Event(EV_Exploder_SetDmg);
	newev->AddInteger((int)finaldamage);
	ProcessEvent(newev);
}

void MultiExploder::MakeExplosion( Event *ev )
{
	Vector pos;
	float t, scale;
	Entity *other;
	Event *event;
	
	other = ev->GetEntity( 1 );
	
	// make sure other is valid
	if ( !other )
	{
		other = world;
	}
	
	// prevent the trigger from triggering again
	trigger_time = -1;
	
	if ( !explode_time )
	{
		hideModel();
		explode_time = level.time + duration;
	}
	
	if ( spawnflags & RANDOM_TIME )
	{
		t = explodewait * ( 1.0f + G_CRandom( randomness ) );
	}
	else
	{
		t = explodewait;
	}
	
	event = new Event( EV_Trigger_Effect );
	event->AddEntity( other );
	PostEvent( event, t );
	
	if ( level.time > explode_time )
	{
		if ( spawnflags & MULTI_USE )
		{
			//
			// reset the trigger in a half second
			//
			trigger_time = level.time + 0.5f;
			explode_time = 0;
			CancelEventsOfType( EV_Trigger_Effect );
			//
			// reset health if necessary
			//
			health = max_health;
			return;
		}
		else
		{
			PostEvent( EV_Remove, 0.0f );
			return;
		}
	}
	
	pos[ 0 ] = absmin[ 0 ] + G_Random( absmax[ 0 ] - absmin[ 0 ] );
	pos[ 1 ] = absmin[ 1 ] + G_Random( absmax[ 1 ] - absmin[ 1 ] );
	pos[ 2 ] = absmin[ 2 ] + G_Random( absmax[ 2 ] - absmin[ 2 ] );
	
	if ( spawnflags & RANDOM_SCALE )
	{
		scale = edict->s.scale * 0.25f;
		scale += G_Random( 3.0f * scale );
	}
	else
	{
		scale = 1.0f;
	}
	
	CreateExplosion( pos, damage, this, other, this, NULL, scale );
}

MultiExploder::MultiExploder()
{
	if ( LoadingSavegame )
	{
		return;
	}
	
	damage         = 120;
	duration			= 1.0;
	explodewait		= 0.25;
	randomness		= 0.25;
	explode_time	= 0;
	
	if ( spawnflags & VISIBLE )
	{
		PostEvent( EV_Show, EV_POSTSPAWN );
	}
	else
	{
		PostEvent( EV_Hide, EV_POSTSPAWN );
	}
	
	// So that we don't get deleted after we're triggered
	count = -1;
	
	respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES;
}

void MultiExploder::SetDmg( Event *ev )
{
	damage = ev->GetInteger( 1 );
	if ( damage < 0 )
	{
		damage = 0;
	}
}

void MultiExploder::SetDuration( Event *ev )
{
	duration = ev->GetFloat( 1 );
}

void MultiExploder::SetWait( Event *ev )
{
	explodewait = ev->GetFloat( 1 );
}

void MultiExploder::SetRandom( Event *ev )
{
	randomness = ev->GetFloat( 1 );
}

void MultiExploder::SetHealth( Event *ev )
{
	Trigger::EventSetHealth(ev);
	triggerondeath = true ;
}



#define METAL_DEBRIS (1<<5)
#define ROCK_DEBRIS  (1<<6)
#define NOTSOLID     (1<<7)

/*****************************************************************************/
/*QUAKED func_explodeobject (0 0.25 0.5) ? MULTI_USE RANDOM_TIME VISIBLE RANDOM_SCALE NO_EXPLOSIONS METAL_DEBRIS ROCK_DEBRIS NOTSOLID

  Spawns different kinds of debris when triggered.  Triggers any targets.
  size of brush determines where explosions and debris will be spawned.

  "dmg" specifies how much damage to cause from each explosion. (Default 120)
  "delay" delay before exploding (Default 0 seconds)
  "duration" how long to explode for (Default 1 second)
  "wait" time between each explosion (default 0.25 seconds)
  "random" random factor (default 0.25)
  "health" if specified, object must be damaged to trigger
  "key" The item needed to activate this. (default nothing)
  "severity" how violent the debris should be ejected from the object( default 1.0 )
  "debrismodel" What kind of debris to spawn (default nothing)
  "amount" how much debris to spawn for each explosion (default 4)
  "thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
  "health" makes the object damageable
  "scale" set the maximum size for spawned debris and explosions
  "doexplosion" specifies to use an explosion (can also specify explosion model and explosion radius)

  MULTI_USE allows the func_explodeobject to be used more than once
  RANDOM_TIME adjusts the wait between each explosion by the random factor
  VISIBLE allows you to make the trigger visible
  RANDOM_SCALE scale explosions and debris randomly. size will be between 0.25 and 1 times scale
  NO_EXPLOSIONS, if checked no explosions will be created
  METAL_DEBRIS automatically spawn metal debris, no need for debrismodel to be set
  ROCK_DEBRIS automatically spawn rock debris, no need for debrismodel to be set
  NOTSOLID debris is not solid

  other valid tiki files include:

  obj_debris_glass1-4.tik
  obj_debris_wood1-4.tik

******************************************************************************/

Event EV_ExplodeObject_SetSeverity
(
	"severity",
	EV_DEFAULT,
	"f",
	"newSeverity",
	"How violently the debris should be ejected."
);
Event EV_ExplodeObject_SetDebrisModel
(
	"debrismodel",
	EV_DEFAULT,
	"s",
	"debrisModel",
	"What kind of debris to spawn when triggered."
);
Event EV_ExplodeObject_SetDebrisAmount
(
	"amount",
	EV_DEFAULT,
	"i",
	"amountOfDebris",
	"How much debris to spawn each time."
);
Event EV_ExplodeObject_SetSound
(
	"setsound",
	EV_DEFAULT,
	"s",
	"soundname",
	"Sound played when the debris explosion occurs"
);
Event EV_ExplodeObject_SetupExplodeTiki
(
	"explodetiki",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Specifies that this is an exploding tiki file (visible and solid).  Use only in tiki files"
);
Event EV_ExplodeObject_SetExplosion
(
	"doexplosion",
	EV_DEFAULT,
	"SF",
	"explosionModel explosionRadius",
	"Causes the object to explode (use dmg to set the damage)"
);
Event EV_ExplodeObject_UseEarthquake
(
	"explodeObject_UseEarthquake",
	EV_DEFAULT,
	NULL,
	NULL,
	"Causes the object to spawn an earthquake when it explodes"
);
Event EV_ExplodeObject_SetEarthquakeDuration
(
	"explodeObject_EarthquakeDuration",
	EV_DEFAULT,
	"f",
	"earthquakeDuration",
	"Sets the earthquake duration"
);
Event EV_ExplodeObject_SetEarthquakeMagnitude
(
	"explodeObject_EarthquakeMagnitude",
	EV_DEFAULT,
	"f",
	"earthquakeMagnitude",
	"Sets the earthquake magnitude"
);
Event EV_ExplodeObject_SetEarthquakeDistance
(
	"explodeObject_EarthquakeDistance",
	EV_DEFAULT,
	"f",
	"earthquakeDistance",
	"Sets the earthquake distance"
);

CLASS_DECLARATION( MultiExploder, ExplodeObject, "func_explodeobject" )
{
	{ &EV_Touch,										NULL },
	{ &EV_Trigger_Effect,								&ExplodeObject::MakeExplosion },
	{ &EV_ExplodeObject_SetSeverity,					&ExplodeObject::SetSeverity },
	{ &EV_ExplodeObject_SetDebrisModel,					&ExplodeObject::SetDebrisModel },
	{ &EV_ExplodeObject_SetDebrisAmount,				&ExplodeObject::SetDebrisAmount },
	{ &EV_ExplodeObject_SetupExplodeTiki,				&ExplodeObject::ExplodeTiki },
	{ &EV_Damage,										&ExplodeObject::Damage },
	{ &EV_ExplodeObject_SetSound,						&ExplodeObject::SetSound },
	{ &EV_ExplodeObject_SetExplosion,  					&ExplodeObject::SetExplosion },
	{ &EV_ExplodeObject_UseEarthquake, 					&ExplodeObject::setSpawnEarthquake },
	{ &EV_ExplodeObject_SetEarthquakeDuration,			&ExplodeObject::setEarthquakeDuration },
	{ &EV_ExplodeObject_SetEarthquakeMagnitude,			&ExplodeObject::setEarthquakeMagnitude },
	{ &EV_ExplodeObject_SetEarthquakeDistance,			&ExplodeObject::setEarthquakeDistance },

	{ NULL, NULL }
};

void ExplodeObject::SetSound( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		debrissound = ev->GetString(1);
}

void ExplodeObject::SetExplosion( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		explosionmodel = ev->GetString( 1 );
	if ( ev->NumArgs() > 1 )
		explosionradius = ev->GetFloat( 2 );
	
	spawnflags &= ~NO_EXPLOSIONS;
}

void ExplodeObject::Damage( Event *ev )
{
	Entity::DamageEvent( ev );
}

void ExplodeObject::SetDebrisModel( Event *ev )
{
	char   string[ 1024 ];
	const char *ptr;
	
	// there could be multiple space delimited models, so we need to search for the spaces.
	strcpy( string,  ev->GetString( 1 ) );
	ptr = strtok( string, " " );
	while ( ptr )
	{
		debrismodels.AddUniqueObject( str( ptr ) );
		CacheResource( ptr, this );
		ptr = strtok( NULL, " " );
	}
}

void ExplodeObject::SetSeverity( Event *ev )
{
	severity = ev->GetFloat( 1 );
}

void ExplodeObject::ExplodeTiki( Event * )
{
	spawnflags |= VISIBLE;
	spawnflags |= NO_EXPLOSIONS;
	spawnflags |= NOTSOLID;
	PostEvent( EV_Show, EV_POSTSPAWN );
	setSolidType( SOLID_BBOX );
}

void ExplodeObject::SetDebrisAmount( Event *ev )
{
	debrisamount = ev->GetInteger( 1 );
}

void ExplodeObject::MakeExplosion( Event *ev )
{
	Vector pos;
	float t, scale;
	Entity *other;
	Event *event;
	
	other = ev->GetEntity( 1 );
	
	// make sure other is valid
	if ( !other )
	{
		other = world;
	}
	
	// prevent the trigger from triggering again
	trigger_time = -1;
	
	if ( !explode_time )
	{
		setSolidType( SOLID_NOT );
		hideModel();
		explode_time = level.time + duration;
	}

	if ( _spawnEarthquake && !_haveSpawnedEarthquake )
	{
		Event *earthquakeEvent = new Event( EV_CreateEarthquake );
		
		earthquakeEvent->AddFloat( _earthquakeMagnitude );
		earthquakeEvent->AddFloat( _earthquakeDuration );
		earthquakeEvent->AddFloat( _earthquakeDistance );

		ProcessEvent( earthquakeEvent );

		_haveSpawnedEarthquake = true;
	}
	
	if ( spawnflags & RANDOM_TIME )
	{
		t = explodewait * ( 1.0f + G_CRandom( randomness ) );
	}
	else
	{
		t = explodewait;
	}
	
	event = new Event( EV_Trigger_Effect );
	event->AddEntity( other );
	PostEvent( event, t );
	
	if ( level.time > explode_time )
	{
		if ( spawnflags & MULTI_USE )
		{
			//
			// reset the trigger in a half second
			//
			trigger_time = level.time + 0.5f;
			explode_time = 0;
			CancelEventsOfType( EV_Trigger_Effect );
			//
			// reset health if necessary
			//
			health = max_health;
			if ( health )
            {
				setSolidType( SOLID_BBOX );
            }
			if ( spawnflags & VISIBLE )
            {
				PostEvent( EV_Show, 0.5f );
            }

			_haveSpawnedEarthquake = false;
			return;
		}
		else
		{
			PostEvent( EV_Remove, 0.0f );
			return;
		}
	}
	
	pos[ 0 ] = absmin[ 0 ] + G_Random( absmax[ 0 ] - absmin[ 0 ] );
	pos[ 1 ] = absmin[ 1 ] + G_Random( absmax[ 1 ] - absmin[ 1 ] );
	pos[ 2 ] = absmin[ 2 ] + G_Random( absmax[ 2 ] - absmin[ 2 ] );
	
	if ( spawnflags & RANDOM_SCALE )
	{
		scale = edict->s.scale * 0.25f;
		scale += G_Random( 3.0f * scale );
	}
	else
	{
		scale = 1.0f;
	}
	
	if ( !( spawnflags & NO_EXPLOSIONS ) )
	{
		CreateExplosion
			(
			pos,
			damage,
			this,
			other,
			this,
			explosionmodel.c_str(),
			scale,
			explosionradius
			);
	}
	if ( debrismodels.NumObjects() )
	{
		TossObject *to;
		int i;

		if ( debrissound.length() )
		{
			Sound( debrissound );
		}

		for ( i = 0; i < debrisamount; i++ )
		{
			int num;
			
			if ( spawnflags & RANDOM_SCALE )
            {
				scale = edict->s.scale * 0.25f;
				scale += G_Random( 3.0f * scale );
            }
			else
            {
				scale = 1.0f;
            }
			
			num = (int)G_Random( (float)debrismodels.NumObjects() ) + 1;
			to = new TossObject( debrismodels.ObjectAt( num ) );
			to->setScale( scale );
			to->setOrigin( pos );
			to->SetVelocity( severity );
			if ( spawnflags & NOTSOLID )
            {
				to->setSolidType( SOLID_NOT );
            }
			pos[ 0 ] = absmin[ 0 ] + G_Random( absmax[ 0 ] - absmin[ 0 ] );
			pos[ 1 ] = absmin[ 1 ] + G_Random( absmax[ 1 ] - absmin[ 1 ] );
			pos[ 2 ] = absmin[ 2 ] + G_Random( absmax[ 2 ] - absmin[ 2 ] );
		}
	}	
}

ExplodeObject::ExplodeObject()
{
	if ( !LoadingSavegame )
	{
		duration			= 1;
		explodewait		= 0.25f;
		severity       = 1.0f;
		explosionradius = 0.0f;
		debrismodels.ClearObjectList();
		explosionmodel = "fx_explosion.tik";
		debrisamount   = 2;
		if ( spawnflags & METAL_DEBRIS )
		{
			debrismodels.AddUniqueObject( str( "fx/fx_debris_metal1.tik" ) );
			debrismodels.AddUniqueObject( str( "fx/fx_debris_metal2.tik" ) );
			debrismodels.AddUniqueObject( str( "fx/fx_debris_metal3.tik" ) );
			CacheResource( "fx/fx_debris_metal1.tik", this );
			CacheResource( "fx/fx_debris_metal2.tik", this );
			CacheResource( "fx/fx_debris_metal3.tik", this );
		}
		else if ( spawnflags & ROCK_DEBRIS )
		{
			debrismodels.AddUniqueObject( str( "fx/fx_debris_rock1.tik" ) );
			debrismodels.AddUniqueObject( str( "fx/fx_debris_rock2.tik" ) );
			debrismodels.AddUniqueObject( str( "fx/fx_debris_rock3.tik" ) );
			debrismodels.AddUniqueObject( str( "fx/fx_debris_rock4.tik" ) );
			CacheResource( "fx/fx_debris_rock1.tik", this );
			CacheResource( "fx/fx_debris_rock2.tik", this );
			CacheResource( "fx/fx_debris_rock3.tik", this );
			CacheResource( "fx/fx_debris_rock4.tik", this );
		}
	}

	_spawnEarthquake = false;
	_haveSpawnedEarthquake = false;
	_earthquakeMagnitude = 1.0f;
	_earthquakeDuration = 1.0f;
	_earthquakeDistance = 500.0f;
}

void ExplodeObject::setSpawnEarthquake( Event * )
{ 
	_spawnEarthquake = true;
}

void ExplodeObject::setEarthquakeDuration( Event *ev ) 
{ 
	_spawnEarthquake = true;
	_earthquakeDuration = ev->GetFloat( 1 ); 
}

void ExplodeObject::setEarthquakeMagnitude( Event *ev )
{ 
	_spawnEarthquake = true;
	_earthquakeMagnitude = ev->GetFloat( 1 ); 
}

void ExplodeObject::setEarthquakeDistance( Event *ev )
{ 
	_spawnEarthquake = true;
	_earthquakeDistance = ev->GetFloat( 1 );
}
