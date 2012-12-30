//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/level.h                                       $
// $Revision:: 31                                                             $
//     $Date:: 3/02/03 8:36p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "g_local.h"
#include "container.h"

class CThread;
class Camera;
class HelperNodeController;
class Earthquake;

//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//

enum fadetype_t      { fadein, fadeout };
enum fadestyle_t     { alphablend, additive };
enum letterboxdir_t  { letterbox_in, letterbox_out };

class Level : public Class
{
	private:
		Container< SafePtr<Earthquake> >	_earthquakes;

		int									_totalEnemiesSpawned;

	public:
		const char		*current_map;
		const char		*current_entities;      
						
		int				spawn_entnum;
		int				currentInstanceNumber;
		int				spawnflags;
						
		int				framenum;
		int				inttime;                // level time in millisecond integer form
		float			time;
		float			timeInLevel;
		float			frametime;
		float			fixedframetime;         // preset frame time based on sv_fps
		int				startTime;				   // level.time the map was started
						
		str				level_name;	            // the descriptive name (Outer Base, etc)
		str				mapname;		            // the server name (base1, etc)
		str				spawnpoint;             // targetname of spawnpoint
		str				nextmap;		            // go here when fraglimit is hit
						
		qboolean		restart;                // set true when game loop should restart
		qboolean		started;				// set when the level is started.
						
		// used for		cinematics
		qboolean		playerfrozen;
						
		// intermiss	ion state
		float			intermissiontime;			// time the intermission was started
		int				exitintermission;
		float			intermission_advancetime;
		bool			_showIntermission;
		bool			_saveOrientation;
						
		gentity_s		*next_edict;				// Used to keep track of the next edict to process in G_RunFrame
						
		int				total_secrets;
		int				found_secrets;
		int				total_specialItems;
		int				found_specialItems;

		str				game_script;
						
		// FIXME - r	emove this later when it is passed in the event.
		trace_t			impact_trace;
						
		qboolean		cinematic;
						
		qboolean		ai_on;
						
		qboolean		mission_failed;
		qboolean		died_already;
						
		qboolean		near_exit;
						
		// Blending		color for water, light volumes,lava
		Vector			water_color;
		float			water_alpha;
						
		Vector			slime_color;
		float			slime_alpha;
						
		Vector			lava_color;
		float			lava_alpha;
						
		str				current_soundtrack;
		str				saved_soundtrack;
						
		CThread			*consoleThread;

		Vector			m_fade_color;
		float			m_fade_alpha;
		float			m_fade_time;
		float			m_fade_time_start;
		fadetype_t		m_fade_type;
		fadestyle_t		m_fade_style;

		float			m_letterbox_fraction;
		float			m_letterbox_time;
		float			m_letterbox_time_start;
		letterboxdir_t	m_letterbox_dir;

		bool			_cleanup;

		str				_playerDeathThread;

		HelperNodeController* hNodeController;
		//
		// list of automatic_cameras on the level
		//
		Container<Camera *>	automatic_cameras;

		Vector         m_intermission_origin;
		Vector         m_intermission_angle;

		CLASS_PROTOTYPE( Level );

                     Level();
					 ~Level();

      void           Init( void );
      void           CleanUp( qboolean restart );

      void           ResetEdicts( void );
      gentity_t      *AllocEdict( Entity *ent );
      void           FreeEdict( gentity_t *ed );
      void           InitEdict( gentity_t *e );

      void           Start( void );
      void           Restart( void );
      void           PlayerRestart( void );
	  void			 update(int levelTime, int frameTime);
      void           Precache( void );
      void			   FindTeams( void );

      void           SpawnEntities( const char *mapname, const char *entities, int levelTime );
      void           NewMap( const char *mapname, const char *entities, int levelTime );
	  void			postLoad( void );
	  void			postSublevelLoad( const char *spawnPosName );

      qboolean       inhibitEntity( int spawnflags );
      void           setSkill( int value );
		int            getSkill( void );
      void           setTime( int levelTime, int frameTime );
		void			   SetGameScript( const char *scriptname );
      void           AddAutomaticCamera( Camera * cam );
	   virtual void   Archive( Archiver &arc );
	   
	   void			loadLevelStrings( void );

	void						addEarthquake( Earthquake *earthquake );
	void						removeEarthquake( Earthquake *earthquake );
	float						getEarthquakeMagnitudeAtPosition( const Vector &origin );

	void			enemySpawned( Entity *enemy );		
      //1st Playable hack
	  void			SetIntermissionAdvanceTime(float);
	  void			EndIntermission();

	  void			setPlayerDeathThread( const str &threadName );
	  str			getPlayerDeathThread( void );
	};

extern Level level;

#endif /* !__LEVEL_H__ */
