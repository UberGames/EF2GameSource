//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/earthquake.h                                  $
// $Revision:: 6                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 9/08/02 10:43a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
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

#ifndef __EARTHQUAKE_H__
#define __EARTHQUAKE_H__

#include "g_local.h"
#include "trigger.h"

#define EARTHQUAKE_STRENGTH 50

extern Event EV_Earthquake_SetDuration;
extern Event EV_Earthquake_SetMagnitude;
extern Event EV_Earthquake_SetDistance;

class Earthquake : public Trigger
{
	protected:
		float		_startTime;
		bool		_active;
		float		_magnitude;
		float		_duration;
		float		_distance;
		float		_currentMagnitude;
		bool		_playSound;

	public:
		CLASS_PROTOTYPE( Earthquake );

									Earthquake();
									~Earthquake();
		void						Activate( Event *ev );
		void						Deactivate( Event *ev );
		void						SetDuration( Event *ev );
		void						SetMagnitude( Event *ev );
		void						SetDistance( Event *ev );
		/* virtual */ void			Think( void );
		bool						EarthquakeActive() { return _active; };
		/* virtual */ void			Archive( Archiver &arc );
		float						getMagnitudeAtPosition( const Vector &position );
		void						setPlaySound( Event *ev );
	};

inline void Earthquake::Archive( Archiver &arc )
{
	Trigger::Archive( arc );

	arc.ArchiveFloat( &_startTime );
	arc.ArchiveBool( &_active );
	arc.ArchiveFloat( &_magnitude );
	arc.ArchiveFloat( &_duration );
	arc.ArchiveFloat( &_distance );
	arc.ArchiveFloat( &_currentMagnitude );
	arc.ArchiveBool( &_playSound );
}

#endif /* Earthquake.h */
