//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/CinematicArmature.h                            $
// $Revision:: 24                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 5/16/03 8:27p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// DESCRIPTION: Classes related to supporting Cinematic Armature.  The
//				CinematicArmature itself is a singleton that governs
//				the loading, cacheing, playing, and stopping of 
//				cinematics.  The Cinematic class represents a single
//				cinematic with actors, cameras, objects, and associated
//				events.
//


// Forward declarations
class Cinematic ;
class CinematicArmature ;
class CinematicActor ;
class CinematicCamera ;

#ifndef __CINEMATIC_ARMATURE_H_
#define __CINEMATIC_ARMATURE_H_

#include "g_local.h"
#include "actor.h"
#include "container.h"

extern CinematicArmature	theCinematicArmature ;
extern Event				EV_Cinematic_CinematicThink;
extern Event				EV_Cinematic_Start ;
extern Event				EV_Cinematic_Stop ;
extern Event				EV_Cinematic_Loop ;
extern Event				EV_Cinematic_Pause ;
extern Event				EV_Cinematic_Continue ;

typedef SafePtr<Cinematic> CinematicPtr;


//-----------------------------------------------------------------
// CinematicActor -- Represents an actor in a cinematic.  This
//					 stores data about the actor participating in
//					 the cinematic, including their name, tiki,
//					 origin at the start of the cinematic, etc.
//					 It also stores a pointer to the Actor class
//					 once the cinematic begins.
//-----------------------------------------------------------------
class CinematicActor : public Listener
{
public:

	typedef enum
	{
		CINEMATIC_ACTOR_STATE_INACTIVE,
		CINEMATIC_ACTOR_STATE_MOVING,
		CINEMATIC_ACTOR_STATE_TURNING,
		CINEMATIC_ACTOR_STATE_PLAYING,
		CINEMATIC_ACTOR_STATE_FINISHED
	} CinematicActorState ;

	typedef enum
	{
		CINEMATIC_ACTOR_AFTER_BEHAVIOR_REMOVE_FROM_GAME,
		CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_WITH_AI,
		CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_NO_AI,
		CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_FREEZE,
		CINEMATIC_ACTOR_AFTER_BEHAVIOR_KILL,
		NUM_CINEMATIC_ACTOR_AFTER_BEHAVIORS
	} CinematicActorAfterBehavior ;

	CLASS_PROTOTYPE( CinematicActor );
								 CinematicActor();
								~CinematicActor()					{ }

	bool						isAtSpot( void )					{ return _isAtSpot ; }
	bool						isAnimDone( void )					{ return _isAnimDone ; }
	bool						isRootActor( void )					{ return _rootActor ; }
	bool						doesSnapToSpot( void )				{ return _snapToSpot ; }
	bool						doesIgnorePain( void )				{ return _ignorePain ; }
	bool						doesIgnoreSight( void )				{ return _ignoreSight ; }
	bool						doesIgnoreSound( void )				{ return _ignoreSound ; }
	bool						doesRemoveAfterCinematic( void )	{ return _removeAfter ; }

	const str&					getName()							{ return _name ; }
	const str&					getTiki()							{ return _tiki ; }
	const str&					getAnim()							{ return _anim ; }
	const str&					getMoveAnim()						{ return _moveAnim ; }
	Vector						getOrigin()							{ return _origin ; }
	ActorPtr					getActor()							{ return _actor ; }
	CinematicActorAfterBehavior getAfterBehavior()					{ return _afterBehavior ; }

	void						setName( const str &name)			{ _name			= name ; }
	void						setTiki( const str &tiki)			{ _tiki			= tiki ; }
	void						setMoveAnim( const str &anim)		{ _moveAnim		= anim ; }
	void						setOriginOffset( const Vector &off)	{ _originOffset	= off ; }
	void						setYawOffset( float yawOffset )		{ _yawOffset	= yawOffset ; }
	void						setSnapToSpot( bool snapToSpot )	{ _snapToSpot	= snapToSpot ; }
	void						setIgnorePain( bool ignorePain )	{ _ignorePain	= ignorePain ; }
	void						setIgnoreSight( bool ignoreSight )	{ _ignoreSight	= ignoreSight ; }
	void						setIgnoreSound( bool ignoreSound )	{ _ignoreSound	= ignoreSound ; }
	void						setRemoveAfter( bool removeAfter )	{ _removeAfter	= removeAfter ; }
	void						setRootActor( bool rootActor )		{ _rootActor	= rootActor ; }
	void						setAlwaysSpawn( bool alwaysSpawn )	{ _alwaysSpawn	= alwaysSpawn ; }
	void						setAfterBehavior( const str &s );
	void						setAnim( const str &anim);

	bool						parse( Script &cinematicFile );
	void						getToPosition( void );
	void						takeControlOfActor( const Vector &origin=Vector( 0.0f, 0.0f, 0.0f ), float yaw=0.0f );
	void						releaseControlOfActor( void );
	void						playAnimation( void );
	void						reset( void );
	void						debug( void );
	void						Archive( Archiver &arc );

	// Event handlers
	void						actorControlLostEvent( Event *ev );
	void						actorBehaviorFinishedEvent( Event *ev );

protected:
	void						_spawn();
	void						_turnActor( bool useAnims );
	void						_locateActor();

private:
	str							_name ;
	str							_tiki ;
	str							_anim ;
	str							_moveAnim ;
	Vector						_origin ;
	Vector						_originOffset ;
	ActorPtr					_actor ;
	float						_yaw ;
	float						_yawOffset ;
	bool						_snapToSpot ;
	bool						_ignorePain ;
	bool						_ignoreSight ;
	bool						_ignoreSound ;
	bool						_isAtSpot ;
	bool						_isAnimDone ;
	bool						_hasActorControl ;
	bool						_rootActor ;
	bool						_removeAfter ;
	bool						_alwaysSpawn ;
	CinematicActorAfterBehavior _afterBehavior ;
	CinematicActorState			_state ;
};

//===============================================================
// Name:		Archive
// Class:		CinematicActor
//
// Description: Archives (loads or saves) data of a Cinematic actor.
// 
// Parameters:	Archiver& -- reference to archiver object to store the data.
//
// Returns:		None
// 
//===============================================================
inline void CinematicActor::Archive
(
	Archiver &arc
)
{
	Listener::Archive( arc );

	arc.ArchiveString( &_name );
	arc.ArchiveString( &_tiki );
	arc.ArchiveString( &_anim );
	arc.ArchiveString( &_moveAnim );
	arc.ArchiveVector( &_origin );
	arc.ArchiveVector( &_originOffset );

	// Archive safe pointer to the actor ( actor is already archived as an entity)
	arc.ArchiveSafePointer( &_actor );

	arc.ArchiveFloat( &_yaw );
	arc.ArchiveFloat( &_yawOffset );
	arc.ArchiveBool( &_snapToSpot );
	arc.ArchiveBool( &_ignorePain );
	arc.ArchiveBool( &_ignoreSight );
	arc.ArchiveBool( &_ignoreSound );
	arc.ArchiveBool( &_isAtSpot );
	arc.ArchiveBool( &_isAnimDone );
	arc.ArchiveBool( &_hasActorControl );
	arc.ArchiveBool( &_rootActor );
	arc.ArchiveBool( &_removeAfter );
	arc.ArchiveBool( &_alwaysSpawn );
	
	// Enumerations
	ArchiveEnum( _afterBehavior,	CinematicActorAfterBehavior );
	ArchiveEnum( _state,			CinematicActorState );
}




//-----------------------------------------------------------------
// CinematicCamera -- Represents a single cinematic camera.  This is
//					  a wrapper around a regular camera.  The cinematic
//					  camera is given stage directions, including when
//					  to cut to the next camera.  It is also usually
//					  instantiated at load time, and delays actual
//					  instantiation of the real camera until the
//					  cinematic begins.
//-----------------------------------------------------------------
class CinematicCamera : public Listener
{
	public:
		typedef enum
		{
			CAMERA_MOVE_TYPE_STATIC,
			CAMERA_MOVE_TYPE_FOLLOW_ANIM,
			CAMERA_MOVE_TYPE_FOLLOW_PLAYER,
			CAMERA_MOVE_TYPE_FOLLOW_ACTOR,
		} CameraMoveType ;

		typedef enum
		{
			CAMERA_LOOK_TYPE_WATCH_ANIM,
			CAMERA_LOOK_TYPE_WATCH_PLAYER,
			CAMERA_LOOK_TYPE_WATCH_ACTOR,
		} CameraLookType ;

								 CLASS_PROTOTYPE( CinematicCamera );
								 CinematicCamera();
								~CinematicCamera()								{ }

		bool					 isAnimDone()									{ return !_playing ; }
		bool					 isSelfRemoving( void )							{ return _selfRemoving ; }

		const str&				 getName( void )								{ return _name ; }
		const str&				 getCamFIle( void )								{ return _camFile ; }
		const str&				 getMoveActor( void )							{ return _moveActor ; }
		const str&				 getLookActor( void )							{ return _lookActor ; }
		void					 getToPosition( void );

		void					 setName( const str &name )						{ _name			= name ; }
		void					 setCamFile( const str &camFile )				{ _camFile		= camFile ; }
		void					 setOriginOffset( const Vector &originOffset )	{ _originOffset	= originOffset ; }
		void					 setYawOffset( float yawOffset )				{ _yawOffset	= yawOffset ; }
		void					 setMoveActor( const str &moveActor )			{ _moveActor	= moveActor ; }
		void					 setLookActor( const str &lookActor )			{ _lookActor	= lookActor ; }
		void					 setMoveType( CameraMoveType moveType )			{ _moveType		= moveType ; }
		void					 setLookType( CameraLookType lookType )			{ _lookType		= lookType ; }
		void					 setSelfRemovingFlag( bool selfRemoving )		{ _selfRemoving	= selfRemoving ; }
		void					 setMoveType( const str &moveType );
		void					 setLookType( const str &lookType );

		void					 start( void );
		void					 cut( void );
		void					 takeControlOfCamera( const Vector &origin=Vector( 0.0f, 0.0f, 0.0f ), float yaw=0.0f );
		void					 releaseControlOfCamera();
		bool					 parse( Script &cinematicFile );
		void					 reset( void );	
		void					 debug( void )							{ }

		void					 Archive( Archiver &arc );

	protected:
		void					 _locateCamera( void );
		void					 _spawn( void );
		void					 _handleStopPlayingEvent( Event *ev );

	private:
		CameraMoveType			 _moveType ;
		CameraLookType			 _lookType ;
		CameraPtr				 _camera ;
		Vector					 _originOffset ;
		float					 _yawOffset ;
		bool					 _playing ;
		bool					 _selfRemoving ;
		str						 _name ;
		str						 _camFile ;
		str						 _moveActor ;
		str						 _lookActor ;

};


//===============================================================
// Name:		Archive
// Class:		CinematicCut
//
// Description: Archives (loads or saves) the data of a Cinematic Camera
// 
// Parameters:	Archiver& -- reference to Archiver that stores the data.
//
// Returns:		None
// 
//===============================================================
inline void CinematicCamera::Archive
(
	Archiver &arc
)
{
	Listener::Archive( arc );

	// Enumerations
	ArchiveEnum( _moveType, CameraMoveType );
	ArchiveEnum( _lookType, CameraLookType );

	// Archive off pointer to the camera (camera is archived as normal entity
	arc.ArchiveSafePointer( &_camera );

	arc.ArchiveVector( &_originOffset );
	arc.ArchiveFloat( &_yawOffset );
	arc.ArchiveBool( &_playing );
	arc.ArchiveBool( &_selfRemoving );
	arc.ArchiveString( &_name );
	arc.ArchiveString( &_camFile );
	arc.ArchiveString( &_moveActor );
	arc.ArchiveString( &_lookActor );
}



//-----------------------------------------------------------------
// CinematicCut -- Represents a single cinematic camera cut. 
//-----------------------------------------------------------------
class CinematicCut : public Listener
{
	public:
		CLASS_PROTOTYPE( CinematicCut );
		CinematicCut();
		~CinematicCut()			{ }

		bool					 doesLerp( void )								{ return _lerpFlag ; }

		CinematicCamera			*getCinematicCamera( void )						{ return _cinematicCamera ; }
		const str				&getCameraName( void )							{ return _cameraName ; }
		int						 getFrame( void )								{ return _frame ; }
		int						 getFadeOut( void )								{ return _fadeOut ; }
		int						 getFadeIn( void )								{ return _fadeIn ; }

		void					 setCinematicCamera( CinematicCamera *camera )	{ _cinematicCamera	= camera ; }
		void					 setCameraName( const str &name )				{ _cameraName		= name ; }
		void					 setFrame( unsigned int frame )					{ _frame			= frame ; }
		void					 setFadeOut( unsigned int fadeOut )				{ _fadeOut			= fadeOut ; }
		void					 setFadeIn( unsigned int fadeIn )				{ _fadeIn			= fadeIn ; }
		void					 setLerpFlag( bool lerpFlag )					{ _lerpFlag			= lerpFlag ; }

		void					 postEvents( void );
		bool					 parse( Script &cinematicFile );
		void					 reset( void );
		void					 stop( void );
		void					 debug( void )								{ }

		void					 Archive( Archiver &arc );

	protected:
		void					 _locateCamera( void );
		void					 _spawn( void );
		void					 _handleFadeOutEvent( Event *event );
		void					 _handleCutEvent( Event *event );

	private:
		CinematicCamera			*_cinematicCamera ;
		bool					 _lerpFlag ;
		int						 _frame ;
		int						 _fadeOut ;
		int						 _fadeIn ;
		str						 _cameraName ;
};


//===============================================================
// Name:		Archive
// Class:		CinematicCut
//
// Description: Archives (loads or saves) a Cinematic Cut's data.
// 
// Parameters:	Archiver& -- reference to object archiving the data.
//
// Returns:		None
// 
//===============================================================
inline void CinematicCut::Archive
(
	Archiver &arc
)
{
	Listener::Archive( arc );

	arc.ArchiveObjectPointer( ( Class**) &_cinematicCamera );
	arc.ArchiveBool( &_lerpFlag );
	arc.ArchiveInteger( &_frame );
	arc.ArchiveInteger( &_fadeOut );
	arc.ArchiveInteger( &_fadeIn );
	arc.ArchiveString( &_cameraName );
}



//------------------------------------------------------------------
// CinematicOrigin -- Specifies an origin for a given cinematic.
//					  This enables the cinematic to be played
//					  relative to this origin, rather than in
//					  absolute coordinates on a map.
//------------------------------------------------------------------
class CinematicOrigin : public Listener
{
	public:
		CLASS_PROTOTYPE( CinematicOrigin );
		CinematicOrigin();
		~CinematicOrigin()		 { }

		Vector					 getOrigin( void )								{ return _origin ; }
		const str&				 getName( void )								{ return _name ; }
		float					 getYaw( void )									{ return _yaw ; }

		void					 setOrigin( const Vector origin )				{ _origin	= origin ; }
		void					 setYaw( float yaw )							{ _yaw		= yaw ; }
		void					 setName( const str &name )						{ _name		= name ; }

		bool					 parse( Script &cinematicFile );
		void					 debug( void );

		void					 Archive( Archiver &arc );
	private:
		Vector					 _origin ;
		str						 _name ;
		float					 _yaw ;
};



//===============================================================
// Name:		Archive
// Class:		CinematicOrigin
//
// Description: Arhives (loads or saves) a cinematic origin.
// 
// Parameters:	Archiver& -- reference to archive object.
//
// Returns:		None
// 
//===============================================================
inline void CinematicOrigin::Archive
(
	Archiver &arc
)
{
	Listener::Archive( arc );

	arc.ArchiveVector( &_origin );
	arc.ArchiveString( &_name );
	arc.ArchiveFloat(  &_yaw );
}



//-----------------------------------------------------------------
// Cinematic -- Represents a single cinematic.  A cinematic is made
//				up of actors, cameras, and other objects.  The 
//				cinematic is responsible for coordinating these 
//				entities working together to recreate the cinematic
//				in the game.  The data for the cinematic is read
//				from a text file when the Cinematic object is
//				instantiated by the CinematicArmature (the coordinator
//				of all cinematics).
//-----------------------------------------------------------------
class Cinematic : public Entity
{
public:
	CLASS_PROTOTYPE( Cinematic );

	typedef enum
	{
		CINEMATIC_STAGE_UNLOADED,
		CINEMATIC_STAGE_READY,
		CINEMATIC_STAGE_WAITING_FOR_ACTORS,
		CINEMATIC_STAGE_ANIMATING,
		CINEMATIC_STAGE_FINISHED,
	} CinematicStage ;

								 Cinematic();
								 Cinematic( const str &filename );
	virtual						~Cinematic();

	bool						 doesResetCamera( void )					{ return _resetCamera ; }

	str&						 getStartThread( void )						{ return _startThread ; }
	str&						 getStopThread( void )						{ return _stopThread ; }
	str&						 getFilename( void )						{ return _filename ; }

	void						 setResetCameraFlag( bool resetCamera )		{ _resetCamera	= resetCamera ; }
	void						 setLooping( bool looping )					{ _looping		= looping ; }
	void						 setFilename( const str &filename )			{ _filename		= filename ; }
	void						 setStartThread( const str &startThread )	{ _startThread	= startThread ; }
	void						 setStopThread( const str &stopThread )		{ _stopThread	= stopThread ; }
	void						 setStartThreadEvent( Event *ev );
	void						 setStopThreadEvent( Event *ev );

	void						 debug( void );
	bool						 load( void );
	void						 start( bool callStartThread = true );
	void						 stop( bool callEndThread = true );
	void				 		 reset( void );
	void						 startAtNamedOrigin( const str &originName, bool callStartThread = true );
	void						 startAtOrigin( const Vector& origin=Vector( 0.0f, 0.0f, 0.0f ), float yaw=0.0f, bool callStartThread = true );
	
	void						 Think();
	virtual void				 Archive( Archiver &arc );

protected:
	CinematicActor				*getCinematicActorByName( const str &name );
	CinematicCamera				*getCinematicCameraByName( const str &cameraName );
	CinematicOrigin				*getCinematicOriginByName( const str &originName );

	void						 handleBeginEvent( Event *ev );
	void						 handleBeginAtEvent( Event *ev );
	void						 handleEndEvent( Event *ev );
	void						 handleSetBeginThreadEvent( Event *ev );
	void						 handleSetEndThreadEvent( Event *ev );

	void						 init();

	bool						 parseActors( Script &cinematicFile );
	bool						 parseActor( Script &cinematicFile );
	bool						 parseCameras( Script &cinematicFile );
	bool						 parseCamera( Script &cinematicFile );
	bool						 parseCut( Script &cinematicFile );
	bool						 parseObjects( Script &cinematicFile );
	bool						 parseObject( Script &cinematicFile );
	bool						 parseOrigins( Script &cinematicFile );
	bool						 parseOrigin( Script &cinematicFile );
	bool						 parseOpenBlock( Script &cinematicFile, const str &blockName, const str &openToken="{" );
	bool						 parseCloseBlock( Script &cinematicFile, const str &blockName, const str &closeToken="}" );

	bool						 areActorsAtTheirPlaces();
	bool						 checkForCompletion();

	void						 _startAnimation();
	void						 _endAnimation();

private:
	Container<CinematicActor*>	_actorList ;
	Container<CinematicCamera*>	_cameraList ;
	Container<CinematicCut*>	_cutList ;
	Container<CinematicOrigin*>	_originList ;
	str							_filename ;
	str							_startThread;
	str							_stopThread ;
	bool						_looping ;
	bool						_playing ;
	bool						_callStartThreadFlag ;
	bool						_resetCamera ;
	CinematicStage				_stage ;
};


//===============================================================
// Name:		Archive
// Class:		Cinematic
//
// Description: Archives (loads or saves) a cinematic's data.
// 
// Parameters:	Archiver& -- reference to object archiving data.
//
// Returns:		None
// 
//===============================================================
inline void Cinematic::Archive
(
	Archiver &arc
)
{
	Entity::Archive( arc );

	// Archive actors
	int numActors = _actorList.NumObjects();
	arc.ArchiveInteger( &numActors );

	if ( arc.Loading() )
	{
		_actorList.Resize( numActors );
		for ( int actorIdx = 1; actorIdx <= numActors; ++actorIdx )
		{
			CinematicActor *actor = 0 ;
			_actorList.AddObject( actor );
			CinematicActor **actorPtr = &_actorList.ObjectAt( actorIdx );
			*actorPtr = (CinematicActor*)arc.ReadObject();
		}
	}
	else
	{
		for ( int actorIdx = 1; actorIdx <= numActors; ++actorIdx )
		{
			CinematicActor *actor = _actorList.ObjectAt( actorIdx );
			arc.ArchiveObject( actor );
		}
	}

	// Archive cameras
	int numCameras = _cameraList.NumObjects();
	arc.ArchiveInteger( &numCameras );

	if ( arc.Loading() )
	{
		_cameraList.Resize( numCameras );
		for ( int cameraIdx = 1; cameraIdx <= numCameras; ++cameraIdx )
		{
			CinematicCamera *camera = 0 ;
			_cameraList.AddObject( camera );
			CinematicCamera **cameraPtr = &_cameraList.ObjectAt( cameraIdx );
			*cameraPtr = (CinematicCamera*)arc.ReadObject();
		}
	}
	else
	{
		for ( int cameraIdx = 1; cameraIdx <= numCameras; ++cameraIdx )
		{
			CinematicCamera *camera = _cameraList.ObjectAt( cameraIdx );
			arc.ArchiveObject( camera );
		}
	}

	// Archive cuts
	int numCuts = _cutList.NumObjects();
	arc.ArchiveInteger( &numCuts );

	if ( arc.Loading() )
	{
		_cutList.Resize( numCuts );
		for ( int cutIdx = 1; cutIdx <= numCuts; ++cutIdx )
		{
			CinematicCut *cut = 0 ;
			_cutList.AddObject( cut );
			CinematicCut **cutPtr = &_cutList.ObjectAt( cutIdx );
			*cutPtr = (CinematicCut*)arc.ReadObject();
		}
	}
	else
	{
		for ( int cutIdx = 1; cutIdx <= numCuts; ++cutIdx )
		{
			CinematicCut *cut = _cutList.ObjectAt( cutIdx );
			arc.ArchiveObject( cut );
		}
	}

	// Archive origins
	int numOrigins = _originList.NumObjects();
	arc.ArchiveInteger( &numOrigins );

	if ( arc.Loading() )
	{
		_cameraList.Resize( numCameras );
		for ( int originIdx = 1; originIdx <= numOrigins; ++originIdx )
		{
			CinematicOrigin *origin = 0 ;
			_originList.AddObject( origin );
			CinematicOrigin **originPtr = &_originList.ObjectAt( originIdx );
			*originPtr = (CinematicOrigin*)arc.ReadObject();
		}
	}
	else
	{
		for ( int originIdx = 1; originIdx <= numOrigins; ++originIdx )
		{
			CinematicOrigin *origin = _originList.ObjectAt( originIdx );
			arc.ArchiveObject( origin );
		}
	}

	// Archive private members
	arc.ArchiveString( &_filename );
	arc.ArchiveString( &_startThread );
	arc.ArchiveString( &_stopThread );
	arc.ArchiveBool( &_playing );
	arc.ArchiveBool( &_looping );
	arc.ArchiveBool( &_callStartThreadFlag );
	arc.ArchiveBool( &_resetCamera );

	// Enumerations
	ArchiveEnum( _stage, CinematicStage );
}


//----------------------------------------------------------------
// CinematicArmature -- Coordinates the creation, playing, and
//						removal of cinematics.
//----------------------------------------------------------------
class CinematicArmature : public Listener
{
	public:
		CLASS_PROTOTYPE(CinematicArmature );

								 CinematicArmature();
								 ~CinematicArmature();

		static bool				 isInDebugMode( void )					{ return _debug ; }

		virtual void			 Archive( Archiver &arc );
		void					 clearCinematicsList();

		Cinematic*				 createCinematic( const str &cinematicName );
		void					 deleteAllCinematics();

	protected:
		Cinematic*				 getCinematicByName( const str &cinematicName );

		void					 setStartThread( Event *ev );
		void					 setStopThread( Event *ev );

		void					 debugCinematics( Event *ev );
		void					 playCinematic( Event *ev );
		void					 playCinematicAt( Event *ev );
		void					 loadCinematic( Event *ev );
		bool					 loadCinematic( const str &cinematicName );

	private:
		Container<CinematicPtr>	 _cinematicList ;
		Cinematic				*_cinematic ;
		static bool				 _debug ;
};


inline void CinematicArmature::Archive
(
	Archiver &arc
)
{
	Listener::Archive( arc );

	int numCinematics = _cinematicList.NumObjects();
	arc.ArchiveInteger( &numCinematics );

	if ( arc.Loading() )
	{
		_cinematicList.Resize( numCinematics );
		for ( int cinematicIdx = 1; cinematicIdx <= numCinematics; ++cinematicIdx )
		{
			Cinematic		*cinematic		= 0 ;
			CinematicPtr	*cinematicPtr	= 0 ;

			_cinematicList.AddObject( cinematic );
			cinematicPtr = &_cinematicList.ObjectAt( cinematicIdx );
			arc.ArchiveSafePointer( cinematicPtr );
		}
	}
	else
	{
		for ( int cinematicIdx = 1; cinematicIdx <= numCinematics; ++cinematicIdx )
		{
			CinematicPtr *cinematic = &_cinematicList.ObjectAt( cinematicIdx );
			arc.ArchiveSafePointer( cinematic );
		}
	}

	arc.ArchiveObjectPointer( (Class**) &_cinematic );
	arc.ArchiveBool( &_debug );
}



#endif /* _CINEMATIC_ARMATURE_H_ */
