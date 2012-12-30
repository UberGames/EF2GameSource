//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/earthquake.cpp                            $
// $Revision:: 11                                                             $
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
// Earthquake trigger causes a localized earthquake when triggered.
// The earthquake effect is visible to the user as the shaking of his screen.
//
#include "_pch_cpp.h"
#include "earthquake.h"

#define EARTHQUAKE_NO_RAMPUP   ( 1 << 0 )
#define EARTHQUAKE_NO_RAMPDOWN ( 1 << 1 )

/*****************************************************************************/
/*QUAKED func_earthquake (0 0.25 0.5) (-8 -8 -8) (8 8 8) NO_RAMPUP NO_RAMPDOWN
 Causes an earthquake
"duration" is the duration of the earthquake.  Default is 0.8 seconds.
"magnitude" severity of the quake.  Default 1.0
******************************************************************************/

Event EV_Earthquake_Deactivate
(
	"earthquake_deactivate",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Stops the earthquake."
);
Event EV_Earthquake_SetDuration
(
	"duration",
	EV_SCRIPTONLY,
	"f",
	"duration",
	"Sets the duration of the earthquake."
);
Event EV_Earthquake_SetMagnitude
(
	"magnitude",
	EV_SCRIPTONLY,
	"f",
	"theMagnitude",
	"Sets the magnitude of the earthquake."
);
Event EV_Earthquake_SetDistance
(
	"distance",
	EV_SCRIPTONLY,
	"f",
	"distance",
	"Sets the distance that an earthquake will effect things."
);
Event EV_Earthquake_SetPlaySound
(
	"earthquake_playsound",
	EV_SCRIPTONLY,
	"B",
	"bool",
	"Sets whether or not the earthquake plays its looping sound or not (defaults to true)."
);

CLASS_DECLARATION( Trigger, Earthquake, "func_earthquake" )
{
	{ &EV_Touch,							NULL },
	{ &EV_Trigger_Effect,					&Earthquake::Activate },
	{ &EV_Earthquake_Deactivate,			&Earthquake::Deactivate },
	{ &EV_Earthquake_SetDuration,			&Earthquake::SetDuration },
	{ &EV_Earthquake_SetMagnitude,			&Earthquake::SetMagnitude },
	{ &EV_Earthquake_SetDistance,			&Earthquake::SetDistance },
	{ &EV_Earthquake_SetPlaySound,			&Earthquake::setPlaySound },

	{ NULL, NULL }
};

Earthquake::Earthquake( void )
{
	// cache in the quake sound
	CacheResource( "snd_earthquake", this );

	if ( LoadingSavegame )
	{
		return;
	}

	// Setup some default values

	_duration = 0.8f;
	_magnitude = 1.0f;
	_distance = 0.0f;
	_active = false;
	_playSound = false;

	// Make sure it gets sent to the client (so the player hears the sound)

	edict->svflags &= ~SVF_NOCLIENT;
}

Earthquake::~Earthquake()
{
	level.removeEarthquake( this );

	StopLoopSound();
}

void Earthquake::SetDuration( Event *ev )
{
	_duration = ev->GetFloat( 1 );
}

void Earthquake::SetMagnitude( Event *ev )
{
	_magnitude = ev->GetFloat( 1 );
}

void Earthquake::SetDistance( Event *ev )
{
	_distance = ev->GetFloat( 1 );
}

void Earthquake::Activate( Event *ev )
{
	// The eartquake has been activated

	if ( _duration > 0.0f )
	{
		// Add ourselves to the level list of earthquakes

		level.addEarthquake( this );

		// Setup everything

		_startTime = level.time;

		_currentMagnitude = _magnitude;

		_active = true;

		if ( _playSound )
		{
			LoopSound( "snd_earthquake", DEFAULT_VOL, LEVEL_WIDE_MIN_DIST );
		}

		PostEvent( EV_Earthquake_Deactivate, _duration );

		turnThinkOn();
	}
}

void Earthquake::Think( void )
{
	float timeDelta;

	if ( !_active )
	{
		return;
	}

	timeDelta = level.time - _startTime;

	// we are in the first half of the earthquake
	if ( timeDelta < ( _duration * 0.5f ) )
	{
		if ( !( spawnflags & EARTHQUAKE_NO_RAMPUP ) )
		{
			float rampUpTime;

			rampUpTime = _startTime + ( _duration * 0.33f );

			if ( level.time < rampUpTime )
			{
				float scale;

				scale = ( timeDelta ) / ( _duration * 0.33f );
				_currentMagnitude = _magnitude * scale;
				edict->s.loopSoundVolume = scale;
			}
			else
			{
				_currentMagnitude = _magnitude;
				edict->s.loopSoundVolume = 1.0f;
			}
		}
	}
	// we are in the second half of the earthquake
	else
	{
		if ( !( spawnflags & EARTHQUAKE_NO_RAMPDOWN ) )
		{
			float rampDownTime;

			rampDownTime = _startTime + ( _duration * 0.66f );

			if ( level.time > rampDownTime )
			{
				float scale;

				scale = 1.0f - ( ( level.time - rampDownTime ) / ( _duration * 0.33f ) );
				_currentMagnitude = _magnitude * scale;
				edict->s.loopSoundVolume = scale;
			}
			else
			{
				_currentMagnitude = _magnitude;
				edict->s.loopSoundVolume = 1.0f;
			}
		}
	}
}


void Earthquake::Deactivate( Event *ev )
{
	// Remove ourselves from the level list of earthquakes

	level.removeEarthquake( this );

	// Stop everything

	_active = false;
	_currentMagnitude = 0.0f;

	StopLoopSound();
	turnThinkOff();
}

void Earthquake::setPlaySound( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		_playSound = ev->GetBoolean( 1 );
	else
		_playSound = true;
}

float Earthquake::getMagnitudeAtPosition( const Vector &position )
{
	float magnitudeAtPosition = 0.0f;

	// Make sure we are active

	if ( _active )
	{
		// We default to our full current magnitude

		magnitudeAtPosition = _currentMagnitude;

		// See if we should diminish our magnitude over distance
		
		if ( _distance )
		{
			Vector dir = origin - position;
			const float distance = dir.length();

			// Modify the magnitude over the distance from the position
			
			if ( distance > _distance )
			{
				magnitudeAtPosition = 0.0f;
			}
			else
			{
				magnitudeAtPosition *= 1.0f - ( distance / _distance );
			}
		}
	}

	return magnitudeAtPosition;
}
