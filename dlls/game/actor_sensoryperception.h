//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actor_sensoryperception.h                      $
// $Revision:: 7                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// DESCRIPTION:
// SensoryPerception class for Actors -- Handles all the sensory related funtionality
//

class SensoryPerception;

#ifndef __ACTOR_SENSORYPERCEPTION_H__
#define __ACTOR_SENSORYPERCEPTION_H__

#include "actor.h"
#include "actorincludes.h"
#include "weapon.h"

//============================
// Class SensoryPerception
//============================
//
// Class used to handle all sensory perception by actors.
//
class SensoryPerception 
   {
	public:
		   SensoryPerception();
			SensoryPerception(Actor *actor );
		  ~SensoryPerception();		
		  
			// Sense functions
         void SenseEnemies();
			void SearchForEnemies();


         // Stimuli functions
		   void     Stimuli( int stimuli );
			void     Stimuli( int stimuli, Entity *ent );
		   void     Stimuli( int stimuli, const Vector &pos );
		   void     Stimuli( int stimuli, const Vector &pos, int sound_Type );
         void     RespondTo( const str &stimuli_name , qboolean respond );
			void     RespondTo( int stimuli, qboolean respond );
			void     PermanentlyRespondTo( const str &stimuli_name , qboolean respond );
			qboolean ShouldRespondToStimuli( int new_stimuli );
			

         // Vision functions
   		qboolean	WithinVisionDistance( const Entity *ent );
         qboolean WithinVisionDistance( const Vector &pos );
		   qboolean	InFOV( const Vector &pos, float check_fov, float check_fovdot );
		   qboolean	InFOV( const Vector &pos );
		   qboolean	InFOV( const Entity *ent );
		   
			// New Vision Functions -- Simplified Vision
			qboolean CanSeeEntity( Entity *start , const Entity *target, qboolean useFOV, qboolean useVisionDistance );
			qboolean CanSeeEntity( const Vector &start , const Entity *target, qboolean useFOV, qboolean useVisionDistance );
			
			// New Vision Functions -- More Sophisticated Vision
			qboolean CanSeeEntityComplex( Entity *start , Entity *target, qboolean useFOV, qboolean useVisionDistance );
	      qboolean CanSeeEntityComplex( Vector &start , Entity *target, qboolean useFOV, qboolean useVisionDistance );			

         qboolean CanSeePosition( const Vector &start, const Vector &position, qboolean useFOV, qboolean useVisionDistance );

         qboolean isInLineOfSight( const Vector &position , const int entNum );
         qboolean checkInLineOfSight( const Vector &position , const int entNum );

			// Debugging Functions
			void ShowInfo();

			// Accessors and Mutators
			void   SetNoisePosition( const Vector &pos );
         Vector GetNoisePosition();

			void   SetLastSoundType( int soundtype );
			int    GetLastSoundType();

			void   SetNoiseTime( float noisetime );
			float  GetNoiseTime();

			void   SetFOV( float fov );
			float  GetFOV();

			void   SetFOVdot( float fov_dot );
			float  GetFOVdot();

			void   SetVisionDistance( float vision_distance );
         float  GetVisionDistance();
			
			// Archiving
			virtual void Archive( Archiver &arc );
			void DoArchive( Archiver &arc , Actor *actor );
						
	private: //Functions
		   qboolean _CanSeeComplex( Vector &start , Entity *target , qboolean useFOV , qboolean useVisionDistance );
			qboolean _SenseEntity( Entity *ent );
         void     _init();

	private: //Member Variables
		
			 // Stimuli Variables
		    int	_stimuli;
		    int	_permanent_stimuli;

			 // Hearing Variables
			 Vector _noise_position;
			 int    _last_soundType;
			 float  _noise_time;
		
		    // Vision Stuff for "seeing"
		    float  _fov;
		    float  _fovdot;
		    float  _vision_distance;

			 float  _nextSenseTime;

          LineOfSight_t _lineOfSight;

			 Actor  *act;
		    
	};

#endif /* __ACTOR_SENSORYPERCEPTION_H__ */
