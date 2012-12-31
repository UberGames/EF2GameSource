//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/equipment.cpp                             $
// $Revision:: 53                                                             $
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

#include "_pch_cpp.h"
#include "equipment.h"
#include "player.h"
#include "weaputils.h"

#define DETAILED_SCAN_TIME 1.5
#define MAX_TARGET_DISTANCE			600.0f
#define MAX_TARGET_DISTANCE_SQUARED	MAX_TARGET_DISTANCE * MAX_TARGET_DISTANCE

Event EV_AirStrike
(
	"airstrike",
	EV_CODEONLY,
	NULL,
	NULL,
	"call airstrike from equipment"
);
Event EV_ScanStart
(
	"scanstart",
	EV_TIKIONLY,
	NULL,
	NULL,
	"The start of the scan"
);
Event EV_ScanEnd
(
	"scanend",
	EV_TIKIONLY,
	NULL,
	NULL,
	"The end of the scan"
);
Event EV_Scanner
(
	"hasscanner",
	EV_TIKIONLY,
	NULL,
	NULL,
	"The equipment has a scanner"
);
Event EV_Radar
(
	"hasradar",
	EV_TIKIONLY,
	NULL,
	NULL,
	"The equipment has a radar"
);
Event EV_Equipment_HasModes
(
	"hasmodes",
	EV_TIKIONLY,
	"sSSSSSSSSS",
	"mode1 mode2 mode3 mode4 mode5 mode6 mode7 mode8 mode9 mode10",
	"Specify the modes this equipment has."
);
Event EV_Equipment_ChangeMode
(
	"changemode",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Change to the next mode."
);
Event EV_Scan
(
	"scan",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Scan"
);
Event EV_Equipment_SetTypeName
(
	"typeName",
	EV_TIKIONLY,
	"s",
	"typeName",
	"Sets the type name of this equipment"
);

CLASS_DECLARATION( Weapon, Equipment, NULL)
{
	{ &EV_AirStrike,					&Equipment::airStrike },
	{ &EV_ScanStart,					&Equipment::scanStart },
	{ &EV_ScanEnd,						&Equipment::scanEnd },
	{ &EV_Scanner,						&Equipment::setScanner },
	{ &EV_Radar,						&Equipment::setRadar },
	{ &EV_Equipment_HasModes,			&Equipment::hasModes },
	{ &EV_Equipment_ChangeMode,			&Equipment::changeMode },
	{ &EV_Equipment_SetTypeName,		&Equipment::setTypeName },
	{ &EV_Scan,							&Equipment::scan },

	{ NULL, NULL }
};


Equipment::Equipment()
{
	init();
}


Equipment::Equipment( const char* file ) :
	Weapon(file)
{
	init();
}

void Equipment::init( void )
{
	scanning = false;
	scanner = false;
	radar = false;
	_active = false;

	scanTime = 0;
	_nextUseTime = 0.0f;

	_scannedEntity = NULL;

	_currentMode = 1;
	_lastMode    = 1;

	_scanEndFrame = -1;

	turnThinkOn();
}


Equipment::~Equipment()
{
	_modes.FreeObjectList();
}

void Equipment::Think()
{		
	if ( _modes.NumObjects() > 0 )
	{
		str *modeName;

		// Make sure this mode is still available

		modeName = &_modes.ObjectAt( _currentMode );
			
		if ( _currentMode != 1 && world->isAnyViewModeAvailable() && !world->isViewModeAvailable( modeName->c_str() ) )
		{
			Event *newEvent;
		
			newEvent = new Event( EV_Equipment_ChangeMode );
			newEvent->AddInteger( 1 );
			ProcessEvent( newEvent );
		}
	}
	
	Player* player = 0;
	if( owner && owner->isSubclassOf(Player) )
	{
		player= (Player*)(Sentient*)owner;
	}
	
	if ( player && _active == qtrue)
	{
		player->client->ps.pm_flags &= ~( PMF_RADAR_MODE | PMF_SCANNER );
		if( hasRadar() )
		{
			player->client->ps.pm_flags |= ( PMF_RADAR_MODE );
		}

		if( hasScanner() )
		{
			player->client->ps.pm_flags |= PMF_SCANNER;
		}
	}
}

//----------------------------------------------------------------
// Name:			airStrike
// Class:			Equipment
//
// Description:		does a few traces to find a sane location, places a model 
//					against the edge of the skybox, and starts it up to do an air strike
//
// Parameters:		none
//
// Returns:			none
//----------------------------------------------------------------
void Equipment::airStrike(Event *)
{
	str modeName;
	Player* player;
	str airStrikeSound;


	// Make sure the scanning stuff is ok

	if ( !scanner || !scanning )
		return;

	// Make sure we are still in the correct mode

	if ( ( _currentMode > _modes.NumObjects() ) || ( _currentMode < 1 ) )
		return;

	modeName = _modes.ObjectAt( _currentMode );

	if ( stricmp( modeName, "torpedostrike") != 0 )
		return;

	// Make sure our owner is a player

	assert( owner );	

	if ( !owner || !owner->isSubclassOf( Player ) )
		return;
		
	player = (Player*)(Sentient*)owner;
							
	// Try to do the torpedo strike

	trace_t trace;
	Vector start;
	Vector forward,right;
	Vector end;
	int i;
	Vector angles;
	Vector dir;
	bool hitSky;
	Vector mins;
	Vector maxs;
	float bestFraction = 0.0f;
	Vector skyPosition;
	Vector bestSkyPosition;

	player->GetViewTrace( trace, MASK_PROJECTILE, 5000.0f );

	start = trace.endpos;
	hitSky = false;

	mins = Vector( -5.0f, -5.0f, -5.0f );
	maxs = Vector( 5.0f, 5.0f, 5.0f );

	for ( i = 0 ; i <= 360 ; i += 45 )
	{
		end = start + Vector( 0.0f, 0.0f, 10000.0f );

		if ( i != 360 )
		{
			angles = vec_zero;
			angles[ YAW ] = i;
			angles.AngleVectors( &dir );

			end += dir * 7500.0f;
		}

		// Try to trace for sky (that means we're outside and in range)

		trace = G_Trace( start, vec_zero, vec_zero, end, player, MASK_PROJECTILE, false, "airStrike::skyspot1" );

		/* gentity_t *loopent = NULL;

		// if we hit a bmodel, move to the top of its box and try again

		while ( (trace.ent) && (trace.ent->bmodel) && (loopent != trace.ent) ) 
		{
			loopent = trace.ent;
			start[2] = loopent->absmax[2] + 5.0f;
			trace = G_Trace( start, vec_zero,vec_zero, end, player, MASK_PROJECTILE, false, "airStrike::skyspot1" );
		} */

		if ( trace.surfaceFlags & SURF_SKY )
		{
			skyPosition = trace.endpos;

			trace = G_Trace( skyPosition, mins, maxs, start, player, MASK_PROJECTILE, false, "airStrike::skyspot2" );

			if ( ( bestFraction == 0.0f ) || ( trace.fraction > bestFraction ) )
			{
				bestFraction = trace.fraction;
				bestSkyPosition = skyPosition;
			}

			hitSky = true;
			//break;
		}
	}

	// Show effects and play sound based on whether or not we hit

	if ( hitSky )
	{
		right = bestSkyPosition;
		forward = right - start;
		
		if (forward.normalize() > 4000)
			right = start + (forward * 4000); // abusing right for final start point
		else
			right = trace.endpos;
		
		Projectile		*airstrike;
		airstrike = new Projectile;		//this needs to be projectile so triggers respond properly (they won't respond to Entity)
		airstrike->setModel("models/weapons/projectile_airstrike.tik");
		airstrike->setOrigin(right);
		forward *= -1.0f; // airstrike model tag is backward, easier to just reverse the direction here
		airstrike->setAngles(forward.toAngles());
		airstrike->PostEvent(EV_Remove,30);

		airStrikeSound = GetRandomAlias( "snd_strikeincoming" );	// play this sound at beginning of airstrike

		_nextUseTime = level.time + 30.0f;

	}
	else
	{
		airStrikeSound = GetRandomAlias( "snd_strikeblocked" );		// play this sound if we can't see sky
	}

	Sound( airStrikeSound );
}

void Equipment::scanStart(Event* )
{
	if(scanner == true)
		scanning = qtrue;

	//if we end scan and start scan on the same frame, 
	//then ignore the end scan
	if(_scanEndFrame == level.framenum)
	{
		PostEvent( EV_Scan, 0.05f );
		return;
	}

	if ( shootingSkin )
		ChangeSkin( shootingSkin, true );

	PostEvent( EV_Scan, 0.25f );

	if ( level.time > _nextUseTime )
	{
		CancelEventsOfType( EV_AirStrike );
		PostEvent(EV_AirStrike,5.0f);
	}
}


void Equipment::scanEnd(Event* )
{
	if(scanner == true)
		scanning = false;

	_scanEndFrame = level.framenum;

	if ( shootingSkin )
		ChangeSkin( shootingSkin, false );

	CancelEventsOfType( EV_Scan );
}

void Equipment::scan( Event * )
{
	
	if ( owner && owner->isSubclassOf( Player ) )
	{
		Event *newEvent;
		Player *player = (Player *)(Entity *)owner;
		
		newEvent = new Event( EV_Player_DoUse );
		newEvent->AddEntity( this );
		player->ProcessEvent( newEvent );
	}
	
	CancelEventsOfType( EV_Scan );
	PostEvent( EV_Scan, 0.05f );
	
}


void Equipment::setScanner(Event* ev)
{
	if(ev == 0)
		return;

	scanner = true;
}


void Equipment::setRadar(Event* ev)
{
	if(ev == 0)
		return;

	radar = true;
}

void Equipment::PutAway(void)
{
	Weapon::PutAway();
}


void Equipment::ProcessTargetedEntity(EntityPtr entity)
{
	if(_scannedEntity != entity && entity != 0)
	{
		Player* player = (Player*)(Sentient*)owner;
		float distanceSquared = DistanceSquared(entity->origin, player->client->ps.origin);
		if(distanceSquared > MAX_TARGET_DISTANCE_SQUARED)
			scanTime = (int) level.time;
	}
	
	if(_scannedEntity != entity)
		scanTime = (int) level.time;
	
	_scannedEntity = entity;
	if(_scannedEntity == 0)
		return;
	
	Weapon::ProcessTargetedEntity(entity);
	
	if(isScanning())
	{
		entity->edict->s.eFlags &= ~EF_DISPLAY_DESC1;	//turn off description 1 if displaying description 2.
		entity->edict->s.eFlags |= EF_DISPLAY_DESC2;
		
		long timeElapsed = (long)(level.time - scanTime);
		if(timeElapsed >= DETAILED_SCAN_TIME)
			entity->edict->s.eFlags |= EF_DISPLAY_DESC3;
	}
	else 
	{
		scanTime = (int)level.time;
		entity->edict->s.eFlags &= ~EF_DISPLAY_DESC2;
		entity->edict->s.eFlags &= ~EF_DISPLAY_DESC3;
	}	
}

void Equipment::AttachToOwner(weaponhand_t hand)
{
	_active = true;
	Weapon::AttachToOwner(hand);

	if ( _lastMode > 1 )
	{
		changeMode( _lastMode );
	}
}

void Equipment::hasModes( Event *ev )
{
	int i;
	str newMode;
	
	for ( i = 1 ; i <= ev->NumArgs() ; i++ )
	{
		newMode = ev->GetString( i );
		
		_modes.AddObject( newMode );
	}
}

void Equipment::changeMode( Event *ev )
{
	int newMode;
	
	if ( ev->NumArgs() > 0 )
	{
		newMode = ev->GetInteger( 1 );
	}
	else
	{
		newMode = _currentMode + 1;
	}

	changeMode( newMode );
}

void Equipment::updateMode( void )
{
	changeMode( _currentMode );
}

void Equipment::changeMode( int newMode )
{
	int numModes;
	Event *newEvent;
	str *modeName;
	
	numModes = _modes.NumObjects();
	
	if ( !numModes )
		return;
	
	// Set the desired mode
	
	_currentMode = newMode;
	
	if ( _currentMode > numModes )
		_currentMode = 1;
	
	// Continue until we have a valid mode set
	
	while( 1 )
	{
		if (_currentMode == 1) // if mode is unset, query player to see if we want nightvision instead (divorced from equipment)
		{
			assert(owner);	
			Player* player;
			
			if ( owner->isSubclassOf(Player))
				player = (Player*)(Sentient*)owner;
			else
				player = NULL;
			
			if ( (player) && (player->client->ps.pm_flags & PMF_NIGHTVISION) )
			{
				newEvent = new Event( EV_Sentient_SetViewMode );
				newEvent->AddString( "nightvision" );
				owner->ProcessEvent( newEvent );
				
				return;
			}
		}
		
		modeName = &_modes.ObjectAt( _currentMode );
		
		// Make sure this mode is available
		
		if ( _currentMode != 1 && world->isAnyViewModeAvailable() && !world->isViewModeAvailable( modeName->c_str() ) )
		{
			// This mode isn't allowed
			
			_currentMode++;
			
			if ( _currentMode > numModes )
				_currentMode = 1;
			
			continue;
		}
		
		// We have a valid mode so set it
		
		newEvent = new Event( EV_Sentient_SetViewMode );
		newEvent->AddString( modeName->c_str() );
		owner->ProcessEvent( newEvent );
		
		return;
	}
}

void Equipment::resetMode( void )
{	
	changeMode( 1 );
}

void Equipment::Uninitialize( void )
{
	if ( !_active )
		return;

	Weapon::Uninitialize();

	if ( !owner )
		return;

	if ( !owner->isSubclassOf( Player ) )
		return;

	Player* player = (Player*)(Sentient*)owner;
	if ( player )
	{
		player->client->ps.pm_flags &= ~(PMF_SCANNER | PMF_RADAR_MODE);
	}

	_lastMode = _currentMode;
	
	resetMode();

	_active = false;
}

void Equipment::Archive( Archiver &arc )
{
	Weapon::Archive( arc );
	
	arc.ArchiveBool( &scanner );
	arc.ArchiveBool( &scanning );
	arc.ArchiveInteger( &_scanEndFrame );
	arc.ArchiveInteger( &scanTime );
	arc.ArchiveBool( &radar );
	
	_modes.Archive( arc );
	
	arc.ArchiveInteger( &_currentMode );
	arc.ArchiveInteger( &_lastMode );
	
	arc.ArchiveString( &_typeName );

	arc.ArchiveSafePointer( &_scannedEntity );

	arc.ArchiveBool( &_active );

	arc.ArchiveFloat( &_nextUseTime );
}

void Equipment::setTypeName( Event *ev )
{
	_typeName = ev->GetString( 1 );
}

int Equipment::getStat( int statNum )
{
	if ( statNum == STAT_WEAPON_GENERIC1 )
	{
		// Return the locking percentage for the torpedo strike

		str modeName;

		if ( !scanner || !scanning )
			return 0;

		if ( level.time < _nextUseTime )
			return 0;

		modeName = _modes.ObjectAt( _currentMode );

		if ( modeName == "torpedostrike" )
		{
			return (int)( ( ( level.time - scanTime ) / 5.0f ) * 100.0f );
		}
	}
	else if ( statNum == STAT_WEAPON_GENERIC2 )
	{
		if ( level.time > _nextUseTime )
			return 100;

		return (int)( 100.0f - ( ( _nextUseTime - level.time ) / 30.0f ) * 100.0f );
	}

	return 0;
}
