//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/CinematicArmature.cpp                      $
// $Revision:: 44                                                             $
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
//		Implementation of Cinematic Armature classes.
//

#include "_pch_cpp.h"
#include "CinematicArmature.h"

bool CinematicArmature::_debug = false ;

//----------------------------------------------------------------
//             C I N E M A T I C   A R M A T U R E
//----------------------------------------------------------------
CinematicArmature	theCinematicArmature ;

Event EV_CinematicArmature_PlayCinematic
(
	"playCinematic",
	EV_CONSOLE | EV_SCRIPTONLY,
	"sS",
	"cinematic originname",
	"Play the current cinematic or the specified one once."
);
Event EV_CinematicArmature_PlayCinematicAt
(
	"playCinematicAt",
	EV_CONSOLE | EV_SCRIPTONLY,
	"sVF",
	"cinematic origin yaw",
	"Play a cinematic at the specified origin with optional yaw rotatioin."
);
Event EV_CinematicArmature_Debug
(
	"debug",
	EV_CONSOLE | EV_SCRIPTONLY,
	NULL,
	NULL,
	"Turns on debugging information for cinematics."
);

CLASS_DECLARATION( Listener, CinematicArmature, NULL )
{
	{ &EV_CinematicArmature_PlayCinematic,		&CinematicArmature::playCinematic },
	{ &EV_CinematicArmature_PlayCinematicAt,	&CinematicArmature::playCinematicAt },
	{ &EV_CinematicArmature_Debug,				&CinematicArmature::debugCinematics },

	{ NULL, NULL }
};


//===============================================================
// Name:		CinematicArmature
// Class:		CinematicArmature
//
// Description: Constructor
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
CinematicArmature::CinematicArmature() :
	_cinematic( NULL )
{
}


//===============================================================
// Name:		~CinematicArmature
// Class:		CinematicArmature
//
// Description: Destructor
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
CinematicArmature::~CinematicArmature( void )
{
	deleteAllCinematics();
	_cinematicList.FreeObjectList();
	_cinematic = NULL ;
}


//===============================================================
// Name:		deleteAllCinematics
// Class:		CinematicArmature
//
// Description: Deletes all the cinematics currently in the
//				list.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicArmature::deleteAllCinematics( void )
{
	int numCinematics = _cinematicList.NumObjects();
	for (int cinematicIdx = 1; cinematicIdx <= numCinematics; cinematicIdx++)
	{
		Cinematic *cinematic = _cinematicList.ObjectAt(cinematicIdx);
		delete cinematic ;
	}
}


//===============================================================
// Name:		clearCinematicsList
// Class:		CinematicArmature
//
// Description: Removes all the outstanding cinematics in the
//				list.  Leaves the mediocre ones.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicArmature::clearCinematicsList( void )
{
	_cinematicList.ClearObjectList();
//	deleteAllCinematics();
}



//===============================================================
// Name:		getCinematic
// Class:		CinematicArmature
//
// Description: Retrieves the cinematic with the specified name.
// 
// Parameters:	const str& -- name of the cinematic to retrieve.
//
// Returns:		Cinematic* -- pointer to the specified cinematic.
//							  Returns 0 (NULL) if not found.
// 
//===============================================================
Cinematic* CinematicArmature::getCinematicByName( const str &cinematicName )
{
	int numCinematics = _cinematicList.NumObjects();
	for (int cinematicIndex = 1; cinematicIndex <= numCinematics; cinematicIndex++)
	{
		Cinematic *cinematic = _cinematicList.ObjectAt(cinematicIndex);
		if (cinematic->targetname == cinematicName)
			return cinematic ;
	}
	return 0 ; // NULL 
}



//===============================================================
// Name:		playCinematic
// Class:		CinematicArmature
//
// Description: Plays (starts) the specified cinematic.  This
//				event also loads the cinematic if it is not already
//				loaded (where as start will give an error).  This
//				event is a shorthand for loading and starting a
//				cinematic.
// 
// Parameters:	Event* -- EV_CinematicArmature_PlayCinematic
//
// Returns:		None
// 
//===============================================================
void CinematicArmature::playCinematic( Event *ev )
{
	if ( ev->NumArgs() >= 1)
	{
		str cinematicName = ev->GetString( 1 );
		_cinematic = getCinematicByName( cinematicName ) ;
		if ( !_cinematic )
		{
			if ( !loadCinematic( cinematicName ))
			{
				ev->Error("playCinematic: Unable to load cinematic %s\n", cinematicName.c_str() );
				return ;
			}
		}
	}
	else if ( !_cinematic )
	{
		ev->Error("startCinematic: No current cinematic to start!" );
		return ;
	}

	assert( _cinematic ) ;

	str originName ;
	if ( ev->NumArgs() == 2 )
	{
		originName = ev->GetString( 2 );
	}

	_cinematic->startAtNamedOrigin( originName );
}

//===============================================================
// Name:		playCinematicAt
// Class:		CinematicArmature
//
// Description: Plays a cinematic at a dynamically specified origin
//				and yaw rotation.
// 
// Parameters:	Event* -- EV_CinematicArmature_PlayCinematicAt
//
// Returns:		None
// 
//===============================================================
void CinematicArmature::playCinematicAt( Event *ev )
{
	if ( ev->NumArgs() >= 1)
	{
		str cinematicName = ev->GetString( 1 );
		_cinematic = getCinematicByName( cinematicName ) ;
		if ( !_cinematic )
		{
			if ( !loadCinematic( cinematicName ))
			{
				ev->Error("playCinematicat: Unable to load cinematic %s\n", cinematicName.c_str() );
				return ;
			}
		}
	}
	else if ( !_cinematic )
	{
		ev->Error("playCinematicat: No current cinematic to start!" );
		return ;
	}

	assert( _cinematic ) ;
	Vector	origin( 0.0f, 0.0f, 0.0f ) ;
	float	yaw	= 0.0f ;

	if ( ev->NumArgs() >= 2 )
	{
		origin = ev->GetVector( 2 );
	}

	if ( ev->NumArgs() == 3 )
	{
		yaw = ev->GetFloat( 3 );
	}

	_cinematic->startAtOrigin( origin, yaw );
}


//===============================================================
// Name:		loadCinematic
// Class:		CinematicArmature
//
// Description: Loads a cinematic from disk.  This will always
//				reload the cinematic.
// 
// Parameters:	const str& -- name of the cinematic to load (.cin file)
//
// Returns:		bool -- true if load was successful.
// 
//===============================================================
bool CinematicArmature::loadCinematic( const str &cinematicName )
{
	CinematicPtr	cinematic = 0 ;
	str				filename;

	filename  = "cins/";
	filename += cinematicName ;
	filename += ".cin";

	cinematic = new Cinematic ( filename );
	if ( !cinematic->load() )
	{
		return false ;
	}

	_cinematicList.AddObject(cinematic);
	_cinematic = cinematic ; // current cinematic gets set

	return true ;
}

//===============================================================
// Name:		debugCinematics
// Class:		CinematicArmature
//
// Description: Toggles debugging info on cinematics
// 
// Parameters:	Event * -- not used
//
// Returns:		None
// 
//===============================================================
void CinematicArmature::debugCinematics( Event *ev )
{
	_debug = !_debug ;
}


//===============================================================
// Name:		createCinematic
// Class:		CinematicArmature
//
// Description: Creates a new cinematic and returns it.  The cinematic
//				created is loaded from the .cin file with the same
//				name.
// 
// Parameters:	const str &cinematicName
//
// Returns:		Cinematic* -- the Cinematic created.  If the
//							  cinematic isn't found, returns 0.
// 
//===============================================================
Cinematic* CinematicArmature::createCinematic( const str &cinematicName )
{
	if ( loadCinematic( cinematicName ) )
	{
		return _cinematic ;
	}

	return 0;
}



//---------------------------------------------------------------
//             C I N E M A T I C   A C T O R
//---------------------------------------------------------------
CLASS_DECLARATION( Listener, CinematicActor, NULL )
{
	{ &EV_Actor_ControlLost,				&CinematicActor::actorControlLostEvent		},
	{ &EV_Actor_BehaviorFinished,			&CinematicActor::actorBehaviorFinishedEvent	},

	{ NULL, NULL }
};


//===============================================================
// Name:		CinematicActor
// Class:		CinematicActor
//
// Description: Constructor
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
CinematicActor::CinematicActor( void ) :	
	_name(""),
	_tiki(""),
	_anim(""),
	_moveAnim("walk"),
	_snapToSpot( false ),
	_ignorePain( true ),
	_ignoreSight( true ),
	_ignoreSound( true ),
	_origin(0.0f, 0.0f, 0.0f),
	_yaw( 0.0f ),
	_isAtSpot( false ),
	_isAnimDone( false ),
	_hasActorControl( false ),
	_rootActor( false ),
	_removeAfter( false ),
	_afterBehavior( CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_WITH_AI ),
	_state( CINEMATIC_ACTOR_STATE_INACTIVE ),
	_actor( 0 )
{
}


//===============================================================
// Name:		reset
// Class:		CinematicActor
//
// Description: Resets the internal state of the actor to what it
//				is before a cinematic begins.  Useful for ensuring
//				that the cinematic can be replayed correctly.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicActor::reset( void )
{
	_state				= CINEMATIC_ACTOR_STATE_INACTIVE ;
	_hasActorControl	= false ;
	_isAtSpot			= false ;
	_isAnimDone			= false ;
}

//===============================================================
// Name:		setAnim
// Class:		CinematicActor
//
// Description: Sets the animation for this actor. Also caches
//				the animation.
// 
// Parameters:	const str& -- the animation name.
//
// Returns:		None
// 
//===============================================================
void CinematicActor::setAnim( const str &anim )
{
	_anim = anim ;
	CacheResource( _anim );
}

//===============================================================
// Name:		setAfterBehavior
// Class:		CinematicActor
//
// Description: Sets the behavior enumeration from the specified
//				string for the actor.  The after behavior controls
//				what the actor does after the cinematic completes.
// 
// Parameters:	const str& -- string description of after behaviors.
//
// Returns:		None
// 
//===============================================================
void CinematicActor::setAfterBehavior( const str &actorBehaviorName )
{
	if ( actorBehaviorName == "REMOVE" )
	{
		_afterBehavior	= CINEMATIC_ACTOR_AFTER_BEHAVIOR_REMOVE_FROM_GAME ;
		_removeAfter	= true ;
	}
	else if ( actorBehaviorName == "LEAVE" || actorBehaviorName == "AI" ) // for legacy reasons, leave means leave with ai
	{
		_afterBehavior	= CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_WITH_AI ;
	}
	else if ( actorBehaviorName == "NOAI" )
	{
		_afterBehavior = CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_NO_AI ;
	}
	else if ( actorBehaviorName == "FREEZE" )
	{
		_afterBehavior = CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_FREEZE ;
	}
	else if ( actorBehaviorName == "KILL" )
	{
		_afterBehavior = CINEMATIC_ACTOR_AFTER_BEHAVIOR_KILL ;
	}
	else // for legacy reasons default is to kill them.
	{
		_afterBehavior = CINEMATIC_ACTOR_AFTER_BEHAVIOR_KILL ;
	}
}




//===============================================================
// Name:		handleControlLostEvent
// Class:		CinematicActor
//
// Description: Handles the EV_Actor_ControlLost event.  
//				This disables the control flag.
// 
// Parameters:	Event* -- the event caught
//
// Returns:		None
// 
//===============================================================
void CinematicActor::actorControlLostEvent( Event *ev )
{
	_hasActorControl = false ;
}


//===============================================================
// Name:		actorBehaviorFinishedEvent
// Class:		CinematicActor
//
// Description: Handles the EV_Actor_BehaviorFinished event.
//				Snaps the actor to the final origin.
// 
// Parameters:	Event* -- the event that triggered this function.
//
// Returns:		None
// 
//===============================================================
void CinematicActor::actorBehaviorFinishedEvent( Event *ev )
{
	assert( _actor );

	switch ( _state )
	{
		case CINEMATIC_ACTOR_STATE_MOVING:
			_actor->setOrigin( _origin );
			_turnActor( true );
			break ;
		case CINEMATIC_ACTOR_STATE_TURNING:
			_isAtSpot = true ;
			_state = CINEMATIC_ACTOR_STATE_PLAYING ;
			break ;
		case CINEMATIC_ACTOR_STATE_PLAYING:
			_isAnimDone = true ;
			_state = CINEMATIC_ACTOR_STATE_FINISHED ;
			break ;
		default:
			// if its not one of the above, probably something
			// has gone terribly wrong (actor killed) and we need
			// to mark this CinematicActor as no longer available.
			break ;
	}
}


//===============================================================
// Name:		turnActor
// Class:		CinematicActor
//
// Description: Turns the actor to face the starting orientation.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicActor::_turnActor( bool useAnims )
{
	assert( _actor );

	Event *ev = new Event( EV_Actor_TurnTo );
	ev->AddFloat( _yaw );
	ev->AddInteger( useAnims );
	_actor->PostEvent( ev, 0.0f );
	_state = CINEMATIC_ACTOR_STATE_TURNING ;
}	


//===============================================================
// Name:		locateActor
// Class:		CinematicActor
//
// Description: Locates or spawns the real actor.  If the actor
//				is not already around, spawns a new one.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicActor::_locateActor( void )
{
	if ( !_alwaysSpawn )
	{
		if ( _actor ) return ;
		_actor = Actor::FindActorByName( _name );
	}
	else
	{
		_actor = 0 ;
	}

	if ( !_actor || _actor->checkActorDead() )
	{
		_spawn();
		_isAtSpot = true ;
	}

	assert( _actor );
	if ( !_actor)
	{
		gi.Error( ERR_DROP, "Unable to locate or spawn cinematic actor %s\n", _name.c_str() );
	}
}



//===============================================================
// Name:		takeControlOfActor
// Class:		CinematicActor
//
// Description: Takes control of the actor.  Turns off AI.  Sets
//				the stimuli the actor responds to.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicActor::takeControlOfActor( const Vector &origin, float yaw )
{
	Event	*ev = 0 ;

	_origin = origin + _originOffset ;
	_yaw	= yaw + _yawOffset ;

	_locateActor();
	_actor->TurnAIOff();
	_actor->RequestControl( this, Actor::ACTOR_CONTROL_LOCKED );

	ev = new Event( EV_Actor_RespondTo );
	ev->AddString( "pain" );
	ev->AddInteger( doesIgnorePain() ? 0 : 1 );
	_actor->ProcessEvent( ev );

	ev = new Event( EV_Actor_PermanentlyRespondTo );
	ev->AddString( "pain" );
	ev->AddInteger( doesIgnorePain() ? 0 : 1 );
	_actor->ProcessEvent( ev );

	ev = new Event( EV_Actor_RespondTo );
	ev->AddString( "sight" );
	ev->AddInteger( doesIgnoreSight() ? 0 : 1 );
	_actor->ProcessEvent( ev );

	ev = new Event( EV_Actor_PermanentlyRespondTo );
	ev->AddString( "sight" );
	ev->AddInteger( doesIgnoreSight() ? 0 : 1 );
	_actor->ProcessEvent( ev );

	ev = new Event( EV_Actor_RespondTo );
	ev->AddString( "sound" );
	ev->AddInteger( doesIgnoreSound() ? 0 : 1 );
	_actor->ProcessEvent( ev );

	ev = new Event( EV_Actor_PermanentlyRespondTo );
	ev->AddString( "sound" );
	ev->AddInteger( doesIgnoreSound() ? 0 : 1 );
	_actor->ProcessEvent( ev );

	_actor->edict->svflags |= SVF_BROADCAST;
}


//===============================================================
// Name:		releaseControlOfActor
// Class:		CinematicActor
//
// Description: Releases control of the actor and turns AI
//				back on.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicActor::releaseControlOfActor( void )
{
	assert( _actor );
	Event *ev = 0 ;

	_actor->ReleaseControl( this );

	_actor->edict->svflags &= ~SVF_BROADCAST;

	switch ( _afterBehavior )
	{
		case CINEMATIC_ACTOR_AFTER_BEHAVIOR_REMOVE_FROM_GAME:
			_actor->ProcessEvent( EV_Remove );
			_actor = 0 ;
			break ;

		case CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_FREEZE:
			ev = new Event( EV_Actor_SetStickToGround );
			ev->AddInteger( 1 );
			_actor->ProcessEvent( ev );

			ev = new Event( EV_Actor_SetSimplifiedThink ) ;
			ev->AddInteger( 0 );
			_actor->ProcessEvent( ev );

			ev = new Event( EV_Actor_SetUseGravity );
			ev->AddInteger( 1 );
			_actor->ProcessEvent( ev );

			ev = new Event( EV_Actor_SetMovementMode );
			ev->AddString( "normal" );
			_actor->ProcessEvent( ev );

			// This little magic freeze the current animation on the
			// last frame of the cinematic anim.  This is intended
			// for cinematics which need to appear to pause at
			// the end (like for a dialog choice).

			if ( _actor->edict->s.torso_anim & ANIM_BLEND )
				_actor->animate->StopAnimatingAtEnd( torso );
			
			if ( _actor->edict->s.anim & ANIM_BLEND )
				_actor->animate->StopAnimatingAtEnd( legs );

			break ;

		case CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_NO_AI:
			ev = new Event( EV_Actor_SetStickToGround );
			ev->AddInteger( 1 );
			_actor->ProcessEvent( ev );

			ev = new Event( EV_Actor_SetSimplifiedThink ) ;
			ev->AddInteger( 0 );
			_actor->ProcessEvent( ev );

			ev = new Event( EV_Actor_SetUseGravity );
			ev->AddInteger( 1 );
			_actor->ProcessEvent( ev );

			ev = new Event( EV_Actor_SetMovementMode );
			ev->AddString( "normal" );
			_actor->ProcessEvent( ev );
			break ;

		case CINEMATIC_ACTOR_AFTER_BEHAVIOR_LEAVE_WITH_AI:
			ev = new Event( EV_Actor_SetStickToGround );
			ev->AddInteger( 1 );
			_actor->ProcessEvent( ev );

			ev = new Event( EV_Actor_SetSimplifiedThink ) ;
			ev->AddInteger( 0 );
			_actor->ProcessEvent( ev );

			ev = new Event( EV_Actor_SetUseGravity );
			ev->AddInteger( 1 );
			_actor->ProcessEvent( ev );

			ev = new Event( EV_Actor_SetMovementMode );
			ev->AddString( "normal" );
			_actor->ProcessEvent( ev );

			_actor->TurnAIOn();
			break ;

		case CINEMATIC_ACTOR_AFTER_BEHAVIOR_KILL:
			_actor->ProcessEvent( EV_Actor_Dead );
			_actor = 0 ;
			break ;

		default:
			break ;
	}
}



//===============================================================
// Name:		getToPosition
// Class:		CinematicActor
//
// Description: Moves a cinematic actor to their starting spot
//				for a cinematic.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicActor::getToPosition( void )
{
	if ( !doesSnapToSpot() && !isAtSpot() )
	{
		// Move to our starting spot
		Event *ev = new Event( EV_Actor_WalkTo );
		ev->AddVector( _origin );
		ev->AddString( _moveAnim );
		_actor->PostEvent( ev, 0.1f );
		_state = CINEMATIC_ACTOR_STATE_MOVING ;
	}
	else if ( !isAtSpot() )
	{
		_actor->setOrigin( _origin );
		_actor->NoLerpThisFrame();
		_turnActor( !doesSnapToSpot() );
	}
	else
	{
		_turnActor( !doesSnapToSpot() );
	}
}



//===============================================================
// Name:		spawn
// Class:		CinematicActor
//
// Description: Spawns an actor with the specified tiki.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicActor::_spawn( void )
{
	ClassDef	*cls;
	Event		*ev;
	SpawnArgs	 args;

	if ( !strstr( _tiki.c_str(), ".tik" ) )
	{
		_tiki += ".tik";
	}

	args.setArg( "model", _tiki.c_str() );
	cls = args.getClassDef();

	if ( !cls || !checkInheritance( &Actor::ClassInfo, cls ) )
	{
		gi.Error(ERR_DROP, "%s is not a valid Actor", _tiki.c_str() );
		return;
	}

	_actor = ( Actor* )cls->newInstance();

	ev = new Event( EV_Model );
	ev->AddString( _tiki.c_str() );
	_actor->ProcessEvent( ev ); //, EV_SPAWNARG );

	ev = new Event( EV_SetOrigin );
	ev->AddVector( _origin );
	_actor->ProcessEvent( ev ); //, EV_SPAWNARG );

	ev = new Event( EV_SetAngles );
	ev->AddVector( Vector( 0.0f, _yaw, 0.0f ) );
	_actor->ProcessEvent( ev ); //, EV_SPAWNARG );

	ev = new Event( EV_Actor_SetUseGravity );
	ev->AddInteger( 0 );
	_actor->ProcessEvent( ev );

	_actor->SetTargetName( _name.c_str() );
}



//===============================================================
// Name:		playAnimation
// Class:		CinematicActor
//
// Description: Kicks off this actor's cinematic animation as
//				specified in the cinematic file.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicActor::playAnimation( void )
{
	Event *ev ;
	_actor->velocity = Vector::Identity();

	ev = new Event( EV_Actor_SetUseGravity );
	ev->AddInteger( 0 );
	_actor->ProcessEvent( ev );


	ev = new Event( EV_Actor_SetSimplifiedThink ) ;
	ev->AddInteger( 1 );
	_actor->ProcessEvent( ev );

	ev = new Event( EV_Actor_SetStickToGround );
	ev->AddInteger( 0 );
	_actor->ProcessEvent( ev );

	ev = new Event( EV_Actor_SetMovementMode );
	ev->AddString( "anim" );
	_actor->ProcessEvent( ev );

	ev = new Event( EV_Actor_Anim );
	ev->AddString( _anim );
	_actor->ProcessEvent( ev );
}


//===============================================================
// Name:		debug
// Class:		CinematicActor
//
// Description: Draws a debug box around the actor's origin.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicActor::debug( void )
{
	G_DrawXYBox( _origin, 8.0f, 0.8f, 0.6f, 0.3f, 1.0f );
}


//===============================================================
// Name:		parse
// Class:		CinematicActor
//
// Description: Parses the data for an actor out of the specified
//				Cinematic file.
// 
// Parameters:	Script& -- the cinematic file being parsed.  We
//						   depend on being at the start of the
//						   actor data (after the opening).
//
// Returns:		bool -- true upon success
// 
//===============================================================
bool CinematicActor::parse( Script &cinematicFile )
{
	const char *token = cinematicFile.GetToken( false );
	Vector		origin;

	while ( token )
	{
		if ( stricmp( token, "name" )==0 )
		{
			setName( cinematicFile.GetString( false ) );
		}
		else if ( stricmp( token, "tiki" )==0 )
		{
			setTiki( cinematicFile.GetString( false ) );
		}
		else if ( stricmp( token, "anim" )==0 )
		{
			setAnim( cinematicFile.GetString( false ) );
		}
		else if ( stricmp( token, "origin")==0 )
		{
			setOriginOffset( cinematicFile.GetVector( false ) );
		}
		else if ( stricmp( token, "yaw" )==0 )
		{
			setYawOffset( cinematicFile.GetFloat( false ) );
		}
		else if ( stricmp( token, "moveanim" )==0 )
		{
			setMoveAnim( cinematicFile.GetString( false ) );
		}
		else if ( stricmp( token, "snap" )==0 )
		{
			setSnapToSpot( cinematicFile.GetBoolean( false ) ? true : false );
		}
		else if ( stricmp( token, "nopain" )==0 )
		{
			setIgnorePain( cinematicFile.GetBoolean( false ) ? true : false );
		}
		else if ( stricmp( token, "nosight")==0 )
		{
			setIgnoreSight( cinematicFile.GetBoolean( false ) ? true : false );
		}
		else if ( stricmp( token, "nosound")== 0 )
		{
			setIgnoreSound( cinematicFile.GetBoolean( false ) ? true : false );
		}
		else if ( stricmp( token, "removeAfter") == 0 )
		{
			setRemoveAfter( cinematicFile.GetBoolean( false ) ? true : false );
		}
		else if ( stricmp( token, "rootActor" ) == 0 )
		{
			setRootActor( cinematicFile.GetBoolean( false ) ? true : false );
		}
		else if ( stricmp( token, "afterBehavior") == 0 )
		{
			setAfterBehavior( cinematicFile.GetToken( false ) );
		}
		else if ( stricmp( token, "alwaysSpawn" ) == 0 )
		{
			setAlwaysSpawn( cinematicFile.GetBoolean( false ) ? true : false );
		}
		else if (stricmp( token, "}")==0 )
		{
			break ;
		}

		token = cinematicFile.GetToken( true );
	}

	return true ;
}



//---------------------------------------------------------------
//             C I N E M A T I C   C A M E R A
//---------------------------------------------------------------
Event EV_CinematicCamera_StopPlaying
(
	"cinematicCameraStopPlaying",
	EV_CODEONLY,
	NULL,
	NULL,
	"Stops a cinematic camera."
);


CLASS_DECLARATION( Listener, CinematicCamera, NULL )
{
	{ &EV_CinematicCamera_StopPlaying,		&CinematicCamera::_handleStopPlayingEvent },

	{ NULL, NULL }
};


//===============================================================
// Name:		CinematicCamera
// Class:		CinematicCamera
//
// Description: Constructor
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
CinematicCamera::CinematicCamera( void ) :
	_moveType( CAMERA_MOVE_TYPE_FOLLOW_ANIM ),
	_lookType( CAMERA_LOOK_TYPE_WATCH_ANIM ),
	_camera( 0 ),
	_playing( false ),
	_selfRemoving( true ),
	_name( "" ),
	_camFile( "" ),
	_moveActor( "" ),
	_lookActor( "" ),
	_originOffset( 0.0f, 0.0f, 0.0f ),
	_yawOffset( 0.0f )
{
}

//===============================================================
// Name:		setMoveType
// Class:		CinematicCamera
//
// Description: Sets the movement type of a cinematic camera.
//				Sets it by string value.  The movement type
//				determines how this camera moves during the 
//				cinematic.
// 
// Parameters:	const str& -- the string version of a movement type.
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::setMoveType( const str &moveType )
{
	if ( moveType == "STATIC" )
	{
		_moveType = CAMERA_MOVE_TYPE_STATIC ;
	}
	else if ( moveType == "ANIM" )
	{
		_moveType = CAMERA_MOVE_TYPE_FOLLOW_ANIM ;
	}
	else if ( moveType == "PLAYER" )
	{
		_moveType = CAMERA_MOVE_TYPE_FOLLOW_PLAYER ;
	}
	else if ( moveType == "ACTOR" )
	{
		_moveType = CAMERA_MOVE_TYPE_FOLLOW_ACTOR ;
	}
}


//===============================================================
// Name:		setLookType
// Class:		CinematicCamera
//
// Description: Sets the look type of a cinematic camera.
//				Sets it by string value.  The look type
//				determines how this camera watches during the 
//				cinematic.
// 
// Parameters:	const str& -- the string version of a look type.
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::setLookType( const str &lookType )
{
	if ( lookType == "ANIM" )
	{
		_lookType = CAMERA_LOOK_TYPE_WATCH_ANIM ;
	}
	else if ( lookType == "PLAYER" )
	{
		_lookType = CAMERA_LOOK_TYPE_WATCH_PLAYER ;
	}
	else if ( lookType == "ACTOR" )
	{
		_lookType = CAMERA_LOOK_TYPE_WATCH_ACTOR ;
	}
}



//===============================================================
// Name:		startMoving
// Class:		CinematicCamera
//
// Description: Starts the camera moving along its path
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::start( void )
{
	_playing = true ;
	_camera->PostEvent( EV_Camera_CameraThink, FRAMETIME );
}


//===============================================================
// Name:		cut
// Class:		CinematicCamera
//
// Description: Cues this camera for playing.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::cut( void )
{
	_camera->Cut( NULL );
	SetCamera( _camera, 0.0f ); // switch time
}


//===============================================================
// Name:		reset
// Class:		CinematicCamera
//
// Description: Resets the internal state of the camera back to
//				the start.  Currently this doesn't do much.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::reset( void )
{
	_playing = false ;
}



//===============================================================
// Name:		takeControlOfCamera
// Class:		CinematicCamera
//
// Description: Takes control of the camera.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::takeControlOfCamera( const Vector &origin, float yaw )
{
	_locateCamera();
	_camera->SetPlaybackOffsets( _yawOffset + yaw, _originOffset + origin );
}

//===============================================================
// Name:		releaseControlOfCamera
// Class:		CinematicCamera
//
// Description: Release the camera
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::releaseControlOfCamera( void )
{
	if ( !_camera) return ;

	CancelPendingEvents();
	if ( isSelfRemoving() )
	{
		_camera->PostEvent( EV_Remove, 0.0f );
	}
	_camera = 0 ;
	_playing = 0 ;
}


//===============================================================
// Name:		_locateCamera
// Class:		CinematicCamera
//
// Description: Locates or spawns the real camera.  If the camera
//				is not already around, spawns a new one.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::_locateCamera( void )
{
	if ( _camera ) return ;

	if ( !_camera )
	{
		_spawn();
	}

	return ;
}


//===============================================================
// Name:		_spawn
// Class:		CinematicCamera
//
// Description: Spawns a camera with the specified tiki.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::_spawn( void )
{
	_camera = new Camera ;
	assert( _camera );
        
	str cameraName( _name );
	cameraName += "Camera" ;

	_camera->SetTargetName( cameraName.c_str() );
    _camera->ProcessPendingEvents();
	if ( _moveType == CAMERA_MOVE_TYPE_FOLLOW_ANIM )
	{
		Event *event = new Event( EV_Camera_LoadKFC );
		event->AddString( _camFile );
		_camera->ProcessEvent( event );

		float pathLengthInSeconds = _camera->GetPathLengthInSeconds();
		event = new Event( EV_CinematicCamera_StopPlaying );
		PostEvent( event, pathLengthInSeconds );
	}
}


//===============================================================
// Name:		_handleStopPlayingEvent
// Class:		CinematicCamera
//
// Description: Handles the stop playing event.  This event tells
//				the camara that it is done playing its cinematic
//				sequence.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCamera::_handleStopPlayingEvent( Event *ev )
{
	_playing = false ;
}



//===============================================================
// Name:		parse
// Class:		CinematicCamera
//
// Description: Parses the data for a camera out of the specified
//				Cinematic file.
// 
// Parameters:	Script& -- the cinematic file being parsed.  We
//						   depend on being at the start of the
//						   camera data (after the opening).
//
// Returns:		bool -- true upon success
// 
//===============================================================
bool CinematicCamera::parse( Script &cinematicFile )
{
	const char *token = cinematicFile.GetToken( false );

	while ( token )
	{
		if ( stricmp( token, "name" )==0 )
		{
			token = cinematicFile.GetString( false );
			setName( token );
		}
		else if ( stricmp( token, "camfile" )==0 )
		{
			token = cinematicFile.GetString( false );
			setCamFile( token );
		}
		else if ( stricmp( token, "movetype" ) == 0 )
		{
			token = cinematicFile.GetString( false );
			setMoveType( token );
		}
		else if ( stricmp( token, "looktype" ) == 0 )
		{
			token = cinematicFile.GetString( false );
			setLookType( token );
		}
		else if ( stricmp( token, "moveactor" ) == 0 )
		{
			token = cinematicFile.GetString( false );
			setMoveActor( token );
		}
		else if ( stricmp( token, "lookactor" ) == 0 )
		{
			token = cinematicFile.GetString( false );
			setLookActor( token );
		}
		else if ( stricmp( token, "origin" ) == 0 )
		{
			setOriginOffset( cinematicFile.GetVector( false ) );
		}
		else if ( stricmp( token, "yaw" ) == 0 )
		{
			setYawOffset( cinematicFile.GetFloat( false ) ) ;
		}
		else if ( stricmp( token, "selfRemoving" ) == 0 )
		{
			setSelfRemovingFlag( cinematicFile.GetBoolean( false ) ? true : false );
		}
		else if (stricmp( token, "}")==0 )
		{
			break ;
		}

		token = cinematicFile.GetToken( true );
	}

	return true ;
}



//---------------------------------------------------------------
//             C I N E M A T I C   C U T
//---------------------------------------------------------------
Event EV_CinematicCut_FadeOut
(
	"cinematiccutfadeout",
	EV_CODEONLY,
	"",
	"",
	"Fades out leading to a cinematic camera cut."
);
Event EV_CinematicCut_Cut
(
	"cinematiccut",
	EV_CODEONLY,
	"",
	"",
	"Makes a cinematic camera cut."
);

CLASS_DECLARATION( Listener, CinematicCut, NULL )
{
	{ &EV_CinematicCut_FadeOut,			&CinematicCut::_handleFadeOutEvent },
	{ &EV_CinematicCut_Cut,				&CinematicCut::_handleCutEvent },

	{ NULL, NULL }
};


//===============================================================
// Name:		CinematicCut
// Class:		CinematicCut
//
// Description: Constructor
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
CinematicCut::CinematicCut( void ) :
	_cinematicCamera( 0 ),
	_frame( 0 ),
	_fadeOut( 0 ),
	_fadeIn( 0 ),
	_cameraName( "" )
{
}



//===============================================================
// Name:		cut
// Class:		CinematicCut
//
// Description: Invokes this camera cut
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCut::postEvents( void )
{
	Event	*ev = 0 ;
	
	if ( _fadeOut )
	{
		ev = new Event( EV_CinematicCut_FadeOut );
		int	fadeOutFrame = _frame - _fadeOut ;
		PostEvent( ev, fadeOutFrame * 0.05 );
	}

	ev = new Event( EV_CinematicCut_Cut );
	PostEvent( ev, _frame * 0.05 );
}



//===============================================================
// Name:		stop
// Class:		CinematicCut
//
// Description: Cancels all outstanding events.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicCut::stop( void )
{
	CancelPendingEvents();
}



//===============================================================
// Name:		parse
// Class:		CinematicCut
//
// Description: Parses the data for a cut out of the specified
//				Cinematic file.
// 
// Parameters:	Script& -- the cinematic file being parsed.  We
//						   depend on being at the start of the
//						   cut block (after the opening).
//
// Returns:		bool -- true upon success
// 
//===============================================================
bool CinematicCut::parse( Script &cinematicFile )
{
	const char *token	= cinematicFile.GetToken( false );
	bool		lerp	= false ;
	int			value	= 0 ;

	while ( token )
	{
		if ( stricmp( token, "toCamera" )==0 )
		{
			token = cinematicFile.GetString( false );
			setCameraName( token );
		}
		else if ( stricmp( token, "fadeOut" )==0 )
		{
			value = cinematicFile.GetInteger( false );
			setFadeOut( value );
		}
		else if ( stricmp( token, "fadeIn" ) == 0 )
		{
			value = cinematicFile.GetInteger( false );
			setFadeIn( value );
		}
		else if ( stricmp( token, "frame" ) == 0 )
		{
			value = cinematicFile.GetInteger( false );
			setFrame( value );
		}
		else if ( stricmp( token, "lerp" ) == 0 )
		{
			lerp = cinematicFile.GetBoolean( false ) ?  true : false ;
			setLerpFlag( lerp );
		}
		else if (stricmp( token, "}")==0 )
		{
			break ;
		}

		token = cinematicFile.GetToken( true );
	}

	return true ;
}


//---------------------------------------------------------------
//          P R O T E C T E D   M E T H O D S 
//---------------------------------------------------------------


//===============================================================
// Name:		handleFadeOutEvent
// Class:		CinematicCut
//
// Description: Handles the EV_CinematicCut_FadeOut event.  This
//				triggers the fade out on the current camera, leading
//				up to this camera cut.
// 
// Parameters:	Event* -- the event that triggered this call.
//
// Returns:		
// 
//===============================================================
void CinematicCut::_handleFadeOutEvent( Event *ev )
{
	level.m_fade_color		= Vector( 0.0f, 0.0f, 0.0f );
	level.m_fade_time_start	= _fadeOut * 0.05 ;
	level.m_fade_time		= _fadeOut * 0.05 ;
	level.m_fade_alpha      = 1.0f ;
	level.m_fade_type       = fadeout;
	level.m_fade_style      = alphablend;
}


//===============================================================
// Name:		handleCutEvent
// Class:		CinematicCut
//
// Description: Handles the EV_CinematicCut_Cut event.  This
//				triggers the actual camera cut, plus does the
//				fade in if any specified.
// 
// Parameters:	Event* -- the event that triggered this call.
//
// Returns:		None
// 
//===============================================================
void CinematicCut::_handleCutEvent( Event *ev )
{
	if ( _cinematicCamera )
	{
		_cinematicCamera->cut();
	}
	else
	{
		SetCamera( NULL, 0.0f );
	}

	if ( _fadeIn )
	{
		level.m_fade_color		= Vector( 0.0f, 0.0f, 0.0f );
		level.m_fade_time_start	= _fadeIn * 0.05 ;
		level.m_fade_time		= _fadeIn * 0.05 ;
		level.m_fade_alpha      = 1.0f ;
		level.m_fade_type       = fadein ;
		level.m_fade_style      = alphablend ;
	}
}



//---------------------------------------------------------------
//             C I N E M A T I C   O R I G I N
//---------------------------------------------------------------
CLASS_DECLARATION( Listener, CinematicOrigin, NULL )
{
	{ NULL, NULL }
};


//===============================================================
// Name:		CinematicOrigin
// Class:		CinematicOrigin
//
// Description: Constructor
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
CinematicOrigin::CinematicOrigin( void ) :
	_origin( 0.0f, 0.0f, 0.0f ),
	_yaw( 0.0f ),
	_name( "" )
{
}


//===============================================================
// Name:		debug
// Class:		CinematicOrigin
//
// Description: Draws a debug box around this origin.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void CinematicOrigin::debug( void )
{
	G_DrawXYBox( _origin, 8.0f, 0.8f, 0.6f, 0.8f, 1.0f );
}



//===============================================================
// Name:		parse
// Class:		CinematicOrigin
//
// Description: Parses the data for an origin out of the specified
//				Cinematic file.
// 
// Parameters:	Script& -- the cinematic file being parsed.  We
//						   depend on being at the start of the
//						   origin block (after the opening).
//
// Returns:		bool -- true upon success
// 
//===============================================================
bool CinematicOrigin::parse( Script &cinematicFile )
{
	const char *token	= cinematicFile.GetToken( false );

	while ( token )
	{
		if ( stricmp( token, "name" )==0 )
		{
			setName( cinematicFile.GetString( false ) );
		}
		else if ( stricmp( token, "origin" )==0 )
		{
			setOrigin( cinematicFile.GetVector( false ) );
		}
		else if ( stricmp( token, "yaw" ) == 0 )
		{
			setYaw( cinematicFile.GetFloat( false ) );
		}
		else if (stricmp( token, "}")==0 )
		{
			break ;
		}

		token = cinematicFile.GetToken( true );
	}

	return true ;
}


//-----------------------------------------------------------
//                C I N E M A T I C
//-----------------------------------------------------------
Event EV_Cinematic_Begin
(
	"beginCinematic",
	EV_DEFAULT,
	"SB",
	"originName callStartThreadFlag",
	"Begins the cinematic at the specified named origin. If the"
	"origin name isn't specified, begins at 0,0,0 with 0 degrees of yaw rotation."
);

Event EV_Cinematic_BeginAt
(
	"beginCinematicAt",
	EV_DEFAULT,
	"VFB",
	"originVector yawRotation callStartThreadFlag",
	"Begins the cinematic at the specified origin with the specified rotation."
	"If they are not specified, begins at 0,0,0 with 0 degrees of yaw rotation."
);

Event EV_Cinematic_End
( 
	"endCinematic",
	EV_DEFAULT | EV_CONSOLE,
	"B",
	"callEndThreadFlag",
	"Stop the cinematic."
);

Event EV_Cinematic_SetBeginThread
(
	"setbeginthread",
	EV_DEFAULT,
	"s",
	"threadName",
	"Sets the thread to call when the cinematic begins."
);

Event EV_Cinematic_SetEndThread
(
	"setendthread",
	EV_DEFAULT,
	"s",
	"threadName",
	"Sets the thread to call when the cinematic ends."
);


CLASS_DECLARATION( Entity, Cinematic, "cinematic" )
{
	{ &EV_Cinematic_Begin,			&Cinematic::handleBeginEvent },
	{ &EV_Cinematic_BeginAt,		&Cinematic::handleBeginAtEvent },
	{ &EV_Cinematic_End,			&Cinematic::handleEndEvent },
	{ &EV_Cinematic_SetBeginThread,	&Cinematic::handleSetBeginThreadEvent },
	{ &EV_Cinematic_SetEndThread,	&Cinematic::handleSetEndThreadEvent },

	{ NULL,							NULL }
};


//===============================================================
// Name:		Cinematic
// Class:		Cinematic
//
// Description: Constructor
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
Cinematic::Cinematic( )
{
	init();
}


//===============================================================
// Name:		Cinematic
// Class:		Cinematic
//
// Description: Constructor.
// 
// Parameters:	const str& -- filename of cinematic file.
//
// Returns:		None
// 
//===============================================================
Cinematic::Cinematic( const str &filename ) :
	_filename( filename),
	_looping( false ),
	_playing( false ),
	_callStartThreadFlag( true ),
	_resetCamera( true ),
	_stage( CINEMATIC_STAGE_UNLOADED )
{
	init();
}

//===============================================================
// Name:		~Cinematic
// Class:		Cinematic
//
// Description: Destructor
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
Cinematic::~Cinematic( void )
{
	_actorList.FreeObjectList();
	_cameraList.FreeObjectList();
	_cutList.FreeObjectList();
	_originList.FreeObjectList();
}


//===============================================================
// Name:		init
// Class:		Cinematic
//
// Description: Initializes a newly constructed cinematic.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Cinematic::init()
{
	turnThinkOn();

	_stage = CINEMATIC_STAGE_UNLOADED ;

	setSolidType( SOLID_NOT );
	setMoveType( MOVETYPE_NONE );

	if ( sv_showcameras->integer )
	{
		setModel( "cinematic.tik" );
		showModel();
	}
	else
	{
		hideModel();
	}
}

//===============================================================
// Name:		reset
// Class:		Cinematic
//
// Description: Resets the internal state of the cinematic to
//				its state before the cinematic begins.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Cinematic::reset( void )
{
	int numCinematicActors = _actorList.NumObjects();
	for ( int cinematicActorIdx = 1; cinematicActorIdx <= numCinematicActors; ++cinematicActorIdx )
	{
		CinematicActor *cinematicActor = _actorList.ObjectAt( cinematicActorIdx );
		cinematicActor->reset();
	}

	int numCinematicCameras = _cameraList.NumObjects();
	for ( int cinematicCameraIdx = 1; cinematicCameraIdx <= numCinematicCameras; ++cinematicCameraIdx )
	{
		CinematicCamera *cinematicCamera = _cameraList.ObjectAt( cinematicCameraIdx );
		cinematicCamera->reset();
	}

	_stage		= CINEMATIC_STAGE_READY ;
	_playing	= false ;
}



//===============================================================
// Name:		Think
// Class:		Cinematic
//
// Description: Think routine for a cinematic
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Cinematic::Think()
{
	switch ( _stage )
	{
	case CINEMATIC_STAGE_WAITING_FOR_ACTORS:
		if ( areActorsAtTheirPlaces() ) 
			start() ;
		break ;
	case CINEMATIC_STAGE_ANIMATING:
		if ( checkForCompletion() ) 
			stop() ;
		break ;
	default: 
		break ;
	}

	if ( CinematicArmature::isInDebugMode() )
	{
		debug();
	}
}


//===============================================================
// Name:		getCinematicActorByName
// Class:		Cinematic
//
// Description: Retrieve an actor by its name.  Returns 0 (NULL)
//				if the actor isn't found.
//
//				This retrieves the first actor with the name.
// 
// Parameters:	const str& -- name of the actor to retrieve.
//
// Returns:		CinematicCamera* -- pointer to the actor with the
//									specified name.  0 (NULL) if
//									not found.
// 
//===============================================================
CinematicActor* Cinematic::getCinematicActorByName( const str &actorName )
{
	int numActors = _actorList.NumObjects();
	for ( int actorIdx = 1; actorIdx <= numActors; ++actorIdx )
	{
		CinematicActor *cinematicActor = _actorList.ObjectAt( actorIdx );
		if ( cinematicActor->getName() == actorName )
		{
			return cinematicActor ;
		}
	}

	return 0 ; // NULL 
}


//===============================================================
// Name:		getCinematicCameraByName
// Class:		Cinematic
//
// Description: Retrieve a camera from its name.  Returns 0 (NULL)
//				if the camera isn't found.  This is used by the
//				cinematic to attach cameras to camera cuts.
//
//				This retrieves the first camera with the name.
// 
// Parameters:	const str& -- name of the camera to retrieve.
//
// Returns:		CinematicCamera* -- pointer to the camera with the
//									specified name.  0 (NULL) if
//									not found.
// 
//===============================================================
CinematicCamera* Cinematic::getCinematicCameraByName( const str &cameraName )
{
	int numCameras = _cameraList.NumObjects();
	for ( int cameraIdx = 1; cameraIdx <= numCameras; ++cameraIdx )
	{
		CinematicCamera *cinematicCamera = _cameraList.ObjectAt( cameraIdx );
		if ( cinematicCamera->getName() == cameraName )
		{
			return cinematicCamera ;
		}
	}

	return 0 ; // NULL 
}

//===============================================================
// Name:		getCinematicOriginByName
// Class:		Cinematic
//
// Description: Retrieve an origin by its name.  Returns 0 (NULL)
//				if the origin isn't found.  This is used by the
//				cinematic to support the playback of a cinematic
//				at multiple origins in the world.
//
//				This retrieves the first origin with the name.
// 
// Parameters:	const str& -- name of the origin to retrieve.
//
// Returns:		CinematicOrigin* -- pointer to the origin with the
//									specified name.  0 (NULL) if
//									not found.
// 
//===============================================================
CinematicOrigin* Cinematic::getCinematicOriginByName( const str &originName )
{
	int numOrigins = _originList.NumObjects();
	for ( int originIdx = 1; originIdx <= numOrigins; ++originIdx )
	{
		CinematicOrigin *cinematicOrigin = _originList.ObjectAt( originIdx );
		if ( str::icmp( cinematicOrigin->getName().c_str(), originName.c_str() )==0 )
		{
			return cinematicOrigin ;
		}
	}

	return 0 ; // NULL 
}



//===============================================================
// Name:		_startAnimation
// Class:		Cinematic
//
// Description: Begins a cinematic actually playing.  At the point
//				this is called, all actors are in their places.
//				All cameras are loaded.  Now we tell all the actors
//				to begin their animations, and the cameras to 
//				begin their playing.
//
//				We set our state variable accordingly.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Cinematic::_startAnimation( void )
{
	int numCinematicActors = _actorList.NumObjects();
	for (int cinematicActorIdx = 1; cinematicActorIdx <= numCinematicActors; ++cinematicActorIdx )
	{
		CinematicActor *cinematicActor = _actorList.ObjectAt( cinematicActorIdx );
		cinematicActor->playAnimation();
	}

	int numCameras = _cameraList.NumObjects();
	for ( int cameraIdx = 1; cameraIdx <= numCameras; ++cameraIdx )
	{
		CinematicCamera *cinematicCamera = _cameraList.ObjectAt( cameraIdx );
		cinematicCamera->start();
	}

	int numCuts = _cutList.NumObjects();
	for ( int cutIdx = 1; cutIdx <= numCuts; ++cutIdx )
	{
		CinematicCut *cinematicCut = _cutList.ObjectAt( cutIdx );
		cinematicCut->postEvents();
	}
}

//===============================================================
// Name:		_endAnimation
// Class:		Cinematic
//
// Description: Ends the currently playing cinematic
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Cinematic::_endAnimation( void )
{
	int numCinematicActors = _actorList.NumObjects();
	for (int cinematicActorIdx = 1; cinematicActorIdx <= numCinematicActors; ++cinematicActorIdx )
	{
		CinematicActor *cinematicActor = _actorList.ObjectAt( cinematicActorIdx );
		cinematicActor->releaseControlOfActor();
	}

	// Tell CinematicCameras to delete their cameras here
	int numCinematicCameras = _cameraList.NumObjects();
	for ( int cinematicCameraIdx = 1; cinematicCameraIdx <= numCinematicCameras; ++cinematicCameraIdx )
	{
		CinematicCamera *cinematicCamera = _cameraList.ObjectAt( cinematicCameraIdx );
		cinematicCamera->releaseControlOfCamera();
	}

	int numCinematicCuts = _cutList.NumObjects();
	for ( int cinematicCutIdx = 1; cinematicCutIdx <= numCinematicCuts; ++cinematicCutIdx )
	{
		CinematicCut *cinematicCut = _cutList.ObjectAt( cinematicCutIdx );
		cinematicCut->stop();
	}
	
	// Restore the player's camera immediately if we've taken it
	if ( doesResetCamera() && numCinematicCuts )
	{
		SetCamera( NULL, 0.0f );
	}
}


//===============================================================
// Name:		isCompletion
// Class:		Cinematic
//
// Description: Checks to see if a cinematic is completed, done,
//				finished, over, kaput, no more, el tora mea braca.
// 
// Parameters:	None
//
// Returns:		bool -- true if completed.
// 
//===============================================================
bool Cinematic::checkForCompletion( void )
{
	int numCinematicActors = _actorList.NumObjects();
	for (int cinematicActorIdx = 1; cinematicActorIdx <= numCinematicActors; ++cinematicActorIdx )
	{
		CinematicActor *cinematicActor = _actorList.ObjectAt( cinematicActorIdx );
		if ( !cinematicActor->isAnimDone() ) return false ;
	}

	int numCinematicCameras = _cameraList.NumObjects();
	for ( int cinematicCameraIdx = 1; cinematicCameraIdx <= numCinematicCameras; ++cinematicCameraIdx )
	{
		CinematicCamera *cinematicCamera = _cameraList.ObjectAt( cinematicCameraIdx );
		if ( !cinematicCamera->isAnimDone() ) return false ;
	}

	return true ;
}



//===============================================================
// Name:		areActorsAtTheirPlaces
// Class:		Cinematic
//
// Description: Checks to see if all the actors are at their
//				places.
// 
// Parameters:	None
//
// Returns:		bool -- true
// 
//===============================================================
bool Cinematic::areActorsAtTheirPlaces( void )
{
	int numActors = _actorList.NumObjects();
	for (int actorIdx = 1; actorIdx <= numActors; actorIdx++)
	{
		CinematicActor *actor = _actorList.ObjectAt( actorIdx );
		if ( !actor->isAtSpot() ) return false ;
	}

	return true ;
}


//===============================================================
// Name:		startAtNamedOrigin
// Class:		Cinematic
//
// Description: Starts the cinematic at the origin specified by
//				name.  If the name isn't specified (or found) 
//				the cinematic's origin is the world origin.
// 
//				Cinematics can have origins named in their cinematic
//				file.  This enables an artist to determine the best
//				spots for the cinematic to play, and to alias them
//				to a name the game scripter can use to play them
//				back.
//
// Parameters:	const str& -- the name of the origin to use.
//
// Returns:		None
// 
//===============================================================
void Cinematic::startAtNamedOrigin( const str &originName, bool callStartThread )
{
	CinematicOrigin *cinematicOrigin = getCinematicOriginByName( originName );
	Vector			 origin( 0.0f, 0.0f, 0.0f );
	float			 yaw			 = 0.0f ;

	if ( cinematicOrigin )
	{
		origin	= cinematicOrigin->getOrigin();
		yaw		= cinematicOrigin->getYaw();
	}

	startAtOrigin( origin, yaw, callStartThread );
}



//===============================================================
// Name:		prepareToStart
// Class:		Cinematic
//
// Description: This begins by 
//				finding the actors involved in the scene.  Actors
//				that are not available are spawned at their 
//				designated spot. 
//
//				Actors are then told to run to their spots (marks
//				in stage terminology).  Once all actors are at
//				their marks, the actual animation begins.
// 
// Parameters:	None
//
// Returns:		float -- the level time the cinematic was begun.
// 
//===============================================================
void Cinematic::startAtOrigin( const Vector &origin, float yaw, bool callStartThread )
{
	reset();

	int numActors = _actorList.NumObjects();
	for ( int actorIdx = 1; actorIdx <= numActors; ++actorIdx )
	{
		CinematicActor *cinematicActor = _actorList.ObjectAt( actorIdx );
		cinematicActor->takeControlOfActor( origin, yaw );
		cinematicActor->getToPosition();
	}

	int numCameras = _cameraList.NumObjects();
	for ( int cameraIdx = 1; cameraIdx <= numCameras; ++cameraIdx )
	{
		CinematicCamera *cinematicCamera = _cameraList.ObjectAt( cameraIdx );
		cinematicCamera->takeControlOfCamera( origin, yaw );
	}

	int numCuts = _cutList.NumObjects();
	for ( int cutIdx = 1; cutIdx <= numCuts; ++cutIdx )
	{
		CinematicCut	*cut		= _cutList.ObjectAt( cutIdx );
		CinematicCamera *camera		= getCinematicCameraByName( cut->getCameraName() );

		cut->setCinematicCamera( camera );
	}

	_stage					= CINEMATIC_STAGE_WAITING_FOR_ACTORS ;
	_playing				= true ;
	_callStartThreadFlag	= callStartThread ;
}


//===============================================================
// Name:		start
// Class:		Cinematic
//
// Description: Starts the cinematic animation.  Calls the
//				start thread if a start thread has been specified
//				and start thread call flag is set (passed in).
//
//				When this function is called, the real cinematic
//				can begin because all of the actors are found and
//				at their places.
// 
// Parameters:	bool -- call the start thread if specified (defaults
//						to true).
//
// Returns:		None
// 
//===============================================================
void Cinematic::start( bool callStartThread )
{
	if ( _startThread.length() && callStartThread )
	{
		ExecuteThread( _startThread );
	}

	_stage = CINEMATIC_STAGE_ANIMATING ;

	_startAnimation();
}



//===============================================================
// Name:		stop
// Class:		Cinematic
//
// Description: Stops the playing of the current cinematic.  All
//				actors are switched back to AI mode.  Remaining
//				events are removed from the queue.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Cinematic::stop( bool callEndThread )
{
	// Make sure we don't stop this cinematic more than once

	if ( !_playing )
		return;

	_playing = false;

	_endAnimation();

	if ( _stopThread.length() && callEndThread )
	{
		ExecuteThread( _stopThread );
	}

	_stage = CINEMATIC_STAGE_FINISHED ;
}


//===============================================================
// Name:		handleBeginEvent
// Class:		Cinematic
//
// Description: Starts the playback of this cinematic.
// 
// Parameters:	Event* -- this event.  No arguments.
//
// Returns:		None
// 
//===============================================================
void Cinematic::handleBeginEvent( Event *ev )
{
	bool	callStartThread = true ;
	str		originName ;

	if ( ev->NumArgs() == 1 )
	{
		originName = ev->GetString( 1 );
	}
	if ( ev->NumArgs() == 2 )
	{
		callStartThread = ev->GetBoolean( 2 );
	}

	startAtNamedOrigin( originName, callStartThread );
}

//===============================================================
// Name:		handleBeginAtEvent
// Class:		Cinematic
//
// Description: Begins the playback of this cinematic at the
//				optionally specified origin and yaw rotation.
// 
// Parameters:	Event* -- EV_Cinematic_BeginAt event.
//							- Arg 1 Optional vector origin (default 0,0,0)
//							- Arg 2 Optional float yaw rotation (default 0 degrees)
//
// Returns:		None
// 
//===============================================================
void Cinematic::handleBeginAtEvent( Event *ev )
{
	Vector	origin( 0.0f, 0.0f, 0.0f );
	float	yawRotation		= 0.0f ;
	bool	callStartThread	= true ;

	if ( ev->NumArgs() >= 1 )
	{
		origin = ev->GetVector( 1 );
	}
	if ( ev->NumArgs() == 2 )
	{
		yawRotation = ev->GetFloat( 2 );
	}
	if ( ev->NumArgs() == 3 )
	{
		callStartThread = ev->GetBoolean( 3 );
	}

	startAtOrigin( origin, yawRotation, callStartThread );
}



//===============================================================
// Name:		handleEndEvent
// Class:		Cinematic
//
// Description: Ends the playback of this cinematic.
// 
// Parameters:	Event* -- EV_Cinematic_End
//
// Returns:		None
// 
//===============================================================
void Cinematic::handleEndEvent( Event *ev )
{
	bool callEndThread = true ;
	
	if ( ev->NumArgs() == 1 )	
	{
		callEndThread = ev->GetBoolean( 1 );
	}
	
	stop( callEndThread );
}


//===============================================================
// Name:		handleSetBeginThreadEvent
// Class:		Cinematic
//
// Description: Sets the name of the thread for this cinematic.
//				This thread will get called at the beginning of the
//				cinematic (after the actors reach their places).
// 
// Parameters:	Event* -- the event that triggered this call.  The
//						  first argument must be a string, naming
//						  the play thread.
//
// Returns:		None
// 
//===============================================================
void Cinematic::handleSetBeginThreadEvent( Event *ev )
{
	assert( ev );

	if (ev->NumArgs() != 1)
	{
		ev->Error( "Usage: setStartThread <threadname>");
		return ;
	}

	setStartThread( ev->GetString( 1 ) );
}


//===============================================================
// Name:		handleSetEndThreadEvent
// Class:		Cinematic
//
// Description: Sets the name of the stop thread for this cinematic.
//				This thread will get called when the cinematic
//				stops.
// 
// Parameters:	Event* -- the event that triggered this call.  The 
//						  first argument must be a string, naming 
//						  the stop thread.
//
// Returns:		None
// 
//===============================================================
void Cinematic::handleSetEndThreadEvent( Event *ev )
{
	assert( ev );

	if (ev->NumArgs() != 1)
	{
		ev->Error( "Usage: setStopThread <threadname>");
		return ;
	}

	setStopThread( ev->GetString( 1 ) );
}


//===============================================================
// Name:		debug
// Class:		Cinematic
//
// Description: Calls debug on all actors, cameras, and cuts.
//				Opportunity for them to output debug information
//				such as drawing lines on the screen.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Cinematic::debug( void )
{
	int numActors = _actorList.NumObjects();
	for ( int actorIdx = 1; actorIdx <= numActors; ++actorIdx )
	{
		CinematicActor *cinematicActor = _actorList.ObjectAt( actorIdx );
		cinematicActor->debug();
	}

	int numCameras = _cameraList.NumObjects();
	for ( int cameraIdx = 1; cameraIdx <= numCameras; ++numCameras )
	{
		CinematicCamera *cinematicCamera = _cameraList.ObjectAt( cameraIdx );
		cinematicCamera->debug();
	}

	int numCuts = _cutList.NumObjects();
	for ( int cutIdx = 1; cutIdx <= numCuts; ++numCuts )
	{
		CinematicCut *cinematicCut = _cutList.ObjectAt( cutIdx );
		cinematicCut->debug();
	}

	int numOrigins = _originList.NumObjects();
	for ( int originIdx = 1; originIdx <= numOrigins; ++numOrigins )
	{
		CinematicOrigin *cinematicOrigin = _originList.ObjectAt( originIdx );
		cinematicOrigin->debug();
	}
}


//===============================================================
// Name:		load
// Class:		Cinematic
//
// Description: Loads the specified cinematic from file.
// 
// Parameters:	None
//
// Returns:		bool -- true if successfully loaded.  False otherwise.
// 
//===============================================================
bool Cinematic::load( void )
{
	Script		 cinematicFile ;
	const char	*token ;	

	if ( !gi.FS_Exists( _filename.c_str() ) )
	{
		gi.Printf( "File %s not found.\n", _filename.c_str() );
		return false ;
	}

	cinematicFile.LoadFile( _filename.c_str());
	while ( cinematicFile.TokenAvailable( true ) )
	{
		token = cinematicFile.GetToken( true );
		if (stricmp( token, "Actors")==0)
		{
			if ( !parseActors( cinematicFile ) ) return false ;
		}
		else if (stricmp( token, "Cameras")==0)
		{
			if ( !parseCameras( cinematicFile ) ) return false ;
		}
		else if (stricmp( token, "Objects")==0)
		{
			if ( !parseObjects( cinematicFile ) ) return false ;
		}
		else if (stricmp( token, "Origins")==0)
		{
			if ( !parseOrigins( cinematicFile ) ) return false ;
		}
		else 
		{
			gi.Printf("Unexpected token %s in cinematic file %s.\n", token, _filename.c_str());
			return false ;
		}
	}

	_stage = CINEMATIC_STAGE_READY ;
	return true ;
}


//===============================================================
// Name:		parseActors
// Class:		Cinematic
//
// Description: Parse the Actors block from a cinematic file.
// 
// Parameters:	Script&	-- the cinematic file being parsed.
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseActors( Script &cinematicFile )
{
	if (!parseOpenBlock(cinematicFile, "Actors")) return false ;
	const char *token = cinematicFile.GetToken( true );
	while ( token )
	{
		if ( stricmp( token, "}" ) == 0 )		break ;
		if ( stricmp( token, "Actor" ) == 0)
		{
			if (!parseActor( cinematicFile )) 
				return false ;
		}

		token = cinematicFile.GetToken( true );
	}

	return true ;
}


//===============================================================
// Name:		parseActor
// Class:		Cinematic
//
// Description: Parse an actor block from a cinematic file.
// 
// Parameters:	Script&	-- the cinematic file being parsed.
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseActor( Script &cinematicFile )
{
	if (!parseOpenBlock( cinematicFile, "Actor" ) ) return false ;

	CinematicActor *actor = new CinematicActor ;
	if (!actor->parse( cinematicFile ))	return false ; 

	_actorList.AddObject( actor );
	return true ;
}


//===============================================================
// Name:		parseCameras
// Class:		Cinematic
//
// Description: Parses the cameras block from a cinematic file.
// 
// Parameters:	Script& -- the cinematic file being parsed
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseCameras( Script &cinematicFile )
{
	if (!parseOpenBlock(cinematicFile, "Cameras")) return false ;
	const char *token = cinematicFile.GetToken( true );
	while ( token )
	{
		if ( strcmp( token, "}" ) == 0 ) break ;

		if ( stricmp( token, "Camera" ) == 0)	
		{
			if ( !parseCamera( cinematicFile )) return false ;
		}
		else if ( stricmp( token, "Cut" ) == 0 )
		{
			if ( !parseCut( cinematicFile ) )return false ;
		}
		else if ( stricmp( token, "ResetCamera" ) == 0 )
		{
			setResetCameraFlag( cinematicFile.GetBoolean( false ) ? true : false );
		}

		token = cinematicFile.GetToken( true );
	}

	return true ;
}


//===============================================================
// Name:		parseCamera
// Class:		Cinematic
//
// Description: Parses a camera block.
// 
// Parameters:	Script& -- the cinematic file being parsed.
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseCamera( Script &cinematicFile )
{
	if ( !parseOpenBlock( cinematicFile, "Camera" ) ) return false ;

	CinematicCamera *cinematicCamera = new CinematicCamera();
	if (!cinematicCamera->parse( cinematicFile ))	return false ; 

	_cameraList.AddObject( cinematicCamera ); 
	return true ;	
}

//===============================================================
// Name:		parseCut
// Class:		Cinematic
//
// Description: Parses a cut block.
// 
// Parameters:	Script& -- the cinematic file being parsed.
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseCut( Script &cinematicFile )
{
	if ( !parseOpenBlock( cinematicFile, "Cut" ) ) return false ;

	CinematicCut *cinematicCut = new CinematicCut();
	if (!cinematicCut->parse( cinematicFile ))	return false ; 

	_cutList.AddObject( cinematicCut ); 
	return true ;	
}


//===============================================================
// Name:		parseObjects
// Class:		Cinematic
//
// Description: Parse the objects block from a cinematic file.
// 
// Parameters:	Script& -- the cinematic file being parsed
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseObjects( Script &cinematicFile )
{
	if (!parseOpenBlock(cinematicFile, "Objects")) return false ;
	const char *token = cinematicFile.GetToken( true );
	while ( token )
	{
		if (strcmp( token, "}") == 0)			break ;
		if (stricmp( token, "Object" ) == 0)	
		{
			if (!parseObject( cinematicFile ))
				return false ;
		}

		token = cinematicFile.GetToken( true );
	}

	return true ;
}


//===============================================================
// Name:		parseObject
// Class:		Cinematic
//
// Description: Parses an object block.
// 
// Parameters:	Script& -- the cinematic file being parsed.
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseObject( Script &cinematicFile )
{
	if (!parseOpenBlock( cinematicFile, "Object" ) ) return false ;
	// 
	return true ;	
}

//===============================================================
// Name:		parseOrigins
// Class:		Cinematic
//
// Description: Parse the Origins block from a cinematic file.
// 
// Parameters:	Script&	-- the cinematic file being parsed.
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseOrigins( Script &cinematicFile )
{
	if (!parseOpenBlock(cinematicFile, "Origins")) return false ;
	const char *token = cinematicFile.GetToken( true );
	while ( token )
	{
		if ( stricmp( token, "}" ) == 0 )		break ;
		if ( stricmp( token, "Origin" ) == 0)
		{
			if ( !parseOrigin( cinematicFile )) 
				return false ;
		}

		token = cinematicFile.GetToken( true );
	}

	return true ;
}


//===============================================================
// Name:		parseOrigin
// Class:		Cinematic
//
// Description: Parse an origin block from a cinematic file.
// 
// Parameters:	Script&	-- the cinematic file being parsed.
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseOrigin( Script &cinematicFile )
{
	if ( !parseOpenBlock( cinematicFile, "Origin" ) ) return false ;

	CinematicOrigin *cinematicOrigin = new CinematicOrigin();
	if ( !cinematicOrigin->parse( cinematicFile ))	return false ; 

	_originList.AddObject( cinematicOrigin );
	return true ;
}


//===============================================================
// Name:		parseOpenBlock
// Class:		Cinematic
//
// Description: Parses the next token and determines if it is the
//				expected open block token (default is "{").  Returns
//				true on success, false otherwise.
// 
// Parameters:	Script&		-- the script to pull tokens from
//				const str&	-- the name of the block we're opening
//				const str&	-- the expected open token (default "{")
//
// Returns:		bool -- true upon success.
// 
//===============================================================
bool Cinematic::parseOpenBlock( Script &cinematicFile, const str &blockName, const str &openToken )
{
	const char *token = cinematicFile.GetToken( true );

	if (strcmp( token, openToken.c_str()) != 0)
	{
		gi.Error(ERR_DROP, "Expected %s to open %s block, found %s.  Aborting parsing.", openToken.c_str(), blockName.c_str(), token);
		return false ;
	}

	return true ;
}


//===============================================================
// Name:		parseCloseBlock
// Class:		Cinematic
//
// Description: Parses the next token and determines if it is the
//				expected close block token (default is "}").  Returns
//				true on success, false otherwise.
// 
// Parameters:	Script&		-- the script to pull tokens from
//				const str&	-- the name of the block we're closing
//				const str&	-- the expected open token (default "}")
//
// Returns:		bool -- true upon success
// 
//===============================================================
bool Cinematic::parseCloseBlock( Script &cinematicFile, const str &blockName, const str &closeToken )
{
	const char *token = cinematicFile.GetToken( true );

	if (strcmp( token, closeToken.c_str()) != 0)
	{
		gi.Error(ERR_DROP, "Expected %s to close %s block, found %s.  Aborting parsing.", closeToken.c_str(), blockName.c_str(), token);
		return false ;
	}

	return false ;
}

