//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/camera.cpp                                    $
// $Revision:: 31                                                             $
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
// Camera.  Duh.
//

#include "_pch_cpp.h"
#include "entity.h"
#include "trigger.h"
#include "camera.h"
#include "bspline.h"
#include "player.h"
#include "camera.h"

//#define CAMERA_PATHFILE_VERSION 1

CameraManager CameraMan;


//---------------------------------------------------------------------------
// CameraMoveState::Initialize
//---------------------------------------------------------------------------
void CameraMoveState::Initialize( Camera * camera )
{
	assert( camera );
	if ( !camera )
		return;

	pos = camera->origin;
	angles = camera->angles;
	movedir = vec_zero;
	followEnt = NULL;
	orbitEnt = NULL;

	followingpath = false;
	cameraTime = 0;
	lastTime = 0;
	newTime = 0;
	cameraPath.Clear();
	splinePath = NULL;
	currentNode = NULL;
	loopNode = NULL;
}


//---------------------------------------------------------------------------
// CameraWatchState::Initialize
//---------------------------------------------------------------------------
void CameraWatchState::Initialize( Camera * camera )
{
	assert( camera );
	if ( !camera )
		return;

	watchAngles = camera->angles;
	watchEnt = NULL;
	watchNodes = true;
	watchPath = false;
}


//---------------------------------------------------------------------------
// CameraState::Initialize
//---------------------------------------------------------------------------
void CameraState::Initialize( Camera * camera )
{
	assert( camera );
	if ( !camera )
		return;

	move.Initialize( camera );
	watch.Initialize( camera );
	fov = camera->Fov();
}


//---------------------------------------------------------------------------
// CameraMoveState::DoNodeEvents
//---------------------------------------------------------------------------
void CameraMoveState::DoNodeEvents( Camera* camera )
{
	SplinePath  *node;
	Entity      *ent;
	Event       *event;

	assert( camera );

	if ( !camera )
		return;

	node = currentNode;
	if ( node )
	{
		float fadeTime;
		float newFov;

		fadeTime = node->GetFadeTime();
		if ( fadeTime == -1.0f )
		{
			fadeTime = camera->fadeTime;
		}

		if ( node->doWatch )
		{
			camera->Watch( node->GetWatch(), fadeTime );
		}

		newFov = node->GetFov();
		if ( newFov )
		{
			camera->SetFOV( newFov, fadeTime );
		}

		if ( node->thread != "" )
		{
			if ( !ExecuteThread( node->thread ) )
			{
				gi.Error( ERR_DROP, "Camera could not start thread '%s' from info_splinepath '%s'\n",
				node->thread.c_str(), node->targetname.c_str() );
			}
		}

		if ( node->triggertarget != "" )
		{
			ent = NULL;
			do
			{
				ent = G_FindTarget( ent, node->triggertarget.c_str() );
				if ( !ent )
					break;

				event = new Event( EV_Activate );
				event->AddEntity( camera );
				ent->PostEvent( event, 0.0f );
			}
			while ( 1 );
		}
	}
}


//---------------------------------------------------------------------------
// CameraMoveState::Evaluate
//---------------------------------------------------------------------------
void CameraMoveState::Evaluate( Camera* camera )
{
	Vector oldpos;
	float speed_multiplier;

	assert( camera );
	if ( !camera )
		return;

	oldpos = pos;
	//
	// check for node events
	// we explicitly skip the first node because we process that
	// when we begin the follow path command
	//
	if ( ( lastTime != newTime ) && currentNode )
	{
		if ( newTime > 1 )
		{
			DoNodeEvents( camera );
		}
		currentNode = currentNode->GetNext();
		if ( !currentNode )
		{
			currentNode = loopNode;
		}
	}
	lastTime = newTime;

	//
	// evaluate position
	//
	if ( followingpath )
	{
		speed_multiplier = cameraPath.Eval( cameraTime, pos, angles );

		cameraTime += level.fixedframetime * camera->camera_speed * speed_multiplier;

		if ( orbitEnt )
		{
			pos += orbitEnt->origin;
			if ( camera->orbit_dotrace )
			{
				trace_t trace;
				Vector start, back;

				start = orbitEnt->origin;
				start[ 2 ] += orbitEnt->maxs[ 2 ];

				back = start - pos;
				back.normalize();

				trace = G_Trace( start, vec_zero, vec_zero, pos, orbitEnt, camera->follow_mask, false, "Camera::EvaluatePosition" );

				if ( trace.fraction < 1.0f )
				{
					pos = trace.endpos;
					// step in a bit towards the followEng
					pos += back * 16.0f;
				}
			}
		}
	}
	else // if !( followingpath )
	{
		if ( followEnt )
		{
			trace_t trace;
			Vector start, end, ang, back;

			start = followEnt->origin;
			start[ 2 ] += followEnt->maxs[ 2 ];

			if ( camera->follow_yaw_fixed )
			{
				ang = vec_zero;
			}
			else
			{
				if ( followEnt->isSubclassOf( Player ) )
				{
					Entity * ent;
					ent = followEnt;
					( ( Player * )ent )->GetPlayerView( NULL, &ang );
				}
				else
				{
					ang = followEnt->angles;
				}
			}
			ang.y += camera->follow_yaw;
			ang.AngleVectors( &back, NULL, NULL );

			end = start - ( back * camera->follow_dist );
			end[ 2 ] += 12.0f;

			trace = G_Trace( start, vec_zero, vec_zero, end, followEnt, camera->follow_mask, false, "Camera::EvaluatePosition - Orbit" );

			pos = trace.endpos;
			// step in a bit towards the followEnt
			pos += back * 16.0f;
		}
	}

	//
	// update times for node events
	//
	newTime = (int)(cameraTime + 2.0f);

	if ( newTime < 0.0f )
	{
		newTime = 0;
	}
	//
	// set movedir
	//
	movedir = pos - oldpos;
}


//---------------------------------------------------------------------------
// CameraWatchState::Evaluate
//---------------------------------------------------------------------------
void CameraWatchState::Evaluate( const Camera* camera, const CameraMoveState* move )
{
	assert( camera );
	assert( move );
	if ( !camera || !move )
		return;

	//
	// evaluate orientation
	//
	if ( watchEnt )
	{
		Vector watchPos;

		watchPos.x = watchEnt->origin.x;
		watchPos.y = watchEnt->origin.y;
		watchPos.z = watchEnt->absmax.z;
		watchPos -= camera->origin;
		watchPos.normalize();
		watchAngles = watchPos.toAngles();
	}
	else if ( watchNodes )
	{
		watchAngles = move->angles;
	}
	else if ( watchPath )
	{
		float  length;
		Vector delta;

		delta = move->movedir;
		length = delta.length();
		if ( length > 0.05f )
		{
			delta *= 1.0f / length;
			watchAngles = delta.toAngles();
		}
	}
	watchAngles[ 0 ] = AngleMod( watchAngles[ 0 ] );
	watchAngles[ 1 ] = AngleMod( watchAngles[ 1 ] );
	watchAngles[ 2 ] = AngleMod( watchAngles[ 2 ] );
}


//---------------------------------------------------------------------------
// CameraState::Evaluate
//---------------------------------------------------------------------------
void CameraState::Evaluate( Camera* camera )
{
	move.Evaluate( camera );
	watch.Evaluate( camera, &move );
}


//---------------------------------------------------------------------------
// Events (EV_XXXXXXXX)
//---------------------------------------------------------------------------
Event EV_Camera_CameraThink
(
	"camera_think",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called each frame to allow the camera to adjust its position."
);
Event EV_Camera_StartMoving
(
	"start",
	EV_DEFAULT,
	NULL,
	NULL,
	"Start camera moving."
);
Event EV_Camera_Pause
(
	"pause",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Pause the camera."
);
Event EV_Camera_Continue
(
	"continue",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Continue the camera movement."
);
Event EV_Camera_StopMoving
( 
	"stop",
	EV_CONSOLE | EV_SCRIPTONLY,
	NULL,
	NULL,
	"Stop the camera movement."
);
Event EV_Camera_SetSpeed
(
	"speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Sets the camera speed."
);
Event EV_Camera_SetFOV
(
	"fov",
	EV_CONSOLE,
	"fF",
	"fov fadeTime",
	"Sets the camera's field of view (fov).\n"
	"if fadeTime is specified, camera will fade over that time\n"
	"if fov is less than 3, than an auto_fov will be assumed\n"
	"the value of fov will be the ratio used for keeping a watch\n"
	"entity in the view at the right scale"
);

Event EV_Camera_SetInterpolateFOV
(
	"interpolatefov",
	EV_DEFAULT,
	"ff",
	"fov interpolateTime",
	"This is just like the camera set fov, but this uses a different name\n"
	"To prevent naming conflicts."
);

Event EV_Camera_LoadKFC
(
	"loadKFC",
	EV_DEFAULT | EV_CONSOLE,
	"s",
	"kfcFileName",
	"Load KFC file, parse it, create an instance of a CameraPath"
);

//
// FOLLOW EVENTS
//
Event EV_Camera_Follow
(
	"follow",
	EV_SCRIPTONLY,
	"eE",
	"targetEnt targetWatchEnt",
	"Makes the camera follow an entity and optionally watch an entity."
);
Event EV_Camera_SetFollowDistance
(
	"follow_distance",
	EV_SCRIPTONLY,
	"f",
	"distance",
	"Sets the camera follow distance."
);
Event EV_Camera_SetFollowYaw
(
	"follow_yaw",
	EV_SCRIPTONLY,
	"f",
	"yaw",
	"Sets the yaw offset of the camera following an entity."
);
Event EV_Camera_AbsoluteYaw
(
	"follow_yaw_absolute",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Makes the follow camera yaw absolute."
);
Event EV_Camera_RelativeYaw
(
	"follow_yaw_relative",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Makes the follow camera yaw relative (not absolute)."
);

//
// ORBIT Events
//
Event EV_Camera_Orbit
(
	"orbit",
	EV_SCRIPTONLY,
	"eE",
	"targetEnt targetWatchEnt",
	"Makes the camera orbit around an entity and optionally watch an entity."
);
Event EV_Camera_SetOrbitHeight
(
	"orbit_height",
	EV_SCRIPTONLY,
	"f",
	"height",
	"Sets the orbit camera's height."
);

//
// Watch Events
//
Event EV_Camera_Watch
(
	"watch",
	EV_SCRIPTONLY,
	"eF",
	"watchEnt fadeTime",
	"Makes the camera watch an entity.\n"
	"if fadeTime is specified, camera will fade over that time"
);
Event EV_Camera_WatchPath
(
	"watchpath",
	EV_CONSOLE | EV_SCRIPTONLY,
	"F",
	"fadeTime",
	"Makes the camera look along the path of travel.\n"
	"if fadeTime is specified, camera will fade over that time"
);
Event EV_Camera_WatchNodes
(
	"watchnode",
	EV_CONSOLE | EV_SCRIPTONLY,
	"F",
	"fadeTime",
	"Makes the camera watch based on what is stored\n"
	"in the camera nodes.\n"
	"if fadeTime is specified, camera will fade over that time"
);
Event EV_Camera_NoWatch
(
	"nowatch",
	EV_CONSOLE | EV_SCRIPTONLY,
	"F",
	"fadeTime",
	"Stop watching an entity or looking along a path.\n"
	"Camera is now static as far as orientation.\n"
	"if fadeTime is specified, camera will fade over that time"
);

//
// Camera positioning events
//
Event EV_Camera_LookAt
(
	"lookat",
	EV_SCRIPTONLY,
	"e",
	"ent",
	"Makes the camera look at an entity."
);
Event EV_Camera_TurnTo
(
	"turnto",
	EV_SCRIPTONLY,
	"v",
	"angle",
	"Makes the camera look in the specified direction."
);
Event EV_Camera_MoveToEntity
(
	"moveto",
	EV_SCRIPTONLY,
	"e",
	"ent",
	"Move the camera's position to that of the specified entities."
);
Event EV_Camera_MoveToPos
(
	"movetopos",
	EV_SCRIPTONLY,
	"v",
	"position",
	"Move the camera's position to the specified position."
);

//
// Camera Transitioning events
//
Event EV_Camera_FadeTime
(
	"fadetime",
	EV_SCRIPTONLY,
	"f",
	"fadetime",
	"Sets the fade time for camera transitioning."
);
Event EV_Camera_Cut
(
	"cut",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"switch camera states immediately, do not transition"
);
Event EV_Camera_SetNextCamera
(
	"nextcamera",
	EV_SCRIPTONLY,
	"s",
	"nextCamera",
	"Sets the next camera to use."
);
Event EV_Camera_SetAutoState
(
	"auto_state",
	EV_SCRIPTONLY,
	"sSSSSS",
	"state1 state2 state3 state4 state5 state6",
	"Sets the states the player needs to be in for this camera to activate."
);
Event EV_Camera_SetAutoRadius
(
	"auto_radius",
	EV_SCRIPTONLY,
	"f",
	"newRadius",
	"Sets the radius of the automatic camera."
);
Event EV_Camera_SetAutoActive
(
	"auto_active",
	EV_SCRIPTONLY,
	"b",
	"newActiveState",
	"Whether or not the auto camera is active."
);
Event EV_Camera_SetAutoStartTime
(
	"auto_starttime",
	EV_SCRIPTONLY,
	"f",
	"newTime",
	"Sets how long it takes for the camera to be switched to."
);
Event EV_Camera_SetAutoStopTime
(
	"auto_stoptime",
	EV_SCRIPTONLY,
	"f",
	"newTime",
	"Sets how long it takes for the camera switch back to the player."
);
Event EV_Camera_SetMaximumAutoFOV
(
	"auto_maxfov",
	EV_SCRIPTONLY,
	"f",
	"maxFOV",
	"Sets the maximum FOV that should be used when automatically calculating FOV."
);

//
// general setup functions
//
Event EV_Camera_SetThread
(
	"setthread",
	EV_SCRIPTONLY,
	"s",
	"thread",
	"Sets the thread to use."
);
Event EV_Camera_SetThread2
(
	"thread",
	EV_SCRIPTONLY,
	"s",
	"thread",
	"Sets the thread to use."
);
Event EV_Camera_SetupCamera
(
	"_setupcamera",
	EV_CODEONLY,
	NULL,
	NULL,
	"setup the camera, post spawn."
);

/*****************************************************************************/
/* QUAKED func_camera (0 0.25 0.5) (-8 -8 0) (8 8 16) ORBIT START_ON AUTOMATIC NO_TRACE NO_WATCH LEVEL_EXIT

Camera used for cinematic sequences.

"target" points to the target to orbit or follow.  If it points to a path,
the camera will follow the path.
"speed" specifies how fast to move on the path or orbit.  (default 1).
"fov" specifies fov of camera, default 90.
if fov is less than 3 than an auto-fov feature is assumed.  The fov will then
specify the ratio to be used to keep a watched entity zoomed in and on the screen
"follow_yaw" specifies yaw of the follow camera, default 0.
"follow_distance" the distance to follow or orbit if the target is not a path. (default 128).
"orbit_height" specifies height of camera from origin for orbiting, default 128.
"nextcamera" a link to the next camera in a chain of cameras
"thread" a thread label that will be fired when the camera is looked through
"auto_state" if specified, denotes the state the player must be in for the camera to engage
any number of states can be specified and only the first few letters need be specified as well
a state of 'pipe' would mean that any player state that started with 'pipe' would trigger this
camera
"auto_radius" the radius, in which the camera will automatically turn on (default 512)
"auto_starttime" how long it takes for the camera to be switched to (default 0.2)
"auto_stoptime" how long it takes for the camera to switch back to the player (default 0.2)
"auto_maxfov" Sets the maximum FOV that should be used when automatically calculating FOV. (default 90)

ORBIT tells the camera to create a circular path around the object it points to.
It the camera points to a path, it will loop when it gets to the end of the path.
START_ON causes the camera to be moving as soon as it is spawned.
AUTOMATIC causes the camera to be switched to automatically when the player is within
a certain radius, if "thread" is specified, that thread will be triggered when the camers is activated
NO_TRACE when the camera is in automatic mode, it will try to trace to the player before
switching automatically, this turns off that feature
NO_WATCH if this is an automatic camera, the camera will automatically watch the player
unless this is set, camera's "score" will be affected by how close to the middle of the camera
the player is.
LEVEL_EXIT if the camera is being used, the level exit state will be set

******************************************************************************/

CLASS_DECLARATION( Entity, Camera, "func_camera" )
{
	{ &EV_Camera_CameraThink,			&Camera::CameraThink },
	{ &EV_Activate,						&Camera::StartMoving },
	{ &EV_Camera_StartMoving,			&Camera::StartMoving },
	{ &EV_Camera_StopMoving,			&Camera::StopMoving },
	{ &EV_Camera_Pause,					&Camera::Pause },
	{ &EV_Camera_Continue,				&Camera::Continue },
	{ &EV_Camera_SetSpeed,				&Camera::SetSpeed },
	{ &EV_Camera_SetFollowDistance,		&Camera::SetFollowDistance },
	{ &EV_Camera_SetFollowYaw,			&Camera::SetFollowYaw },
	{ &EV_Camera_AbsoluteYaw,			&Camera::AbsoluteYaw },
	{ &EV_Camera_RelativeYaw,			&Camera::RelativeYaw },
	{ &EV_Camera_SetOrbitHeight,		&Camera::SetOrbitHeight },
	{ &EV_Camera_SetFOV,				&Camera::SetFOV },
	{ &EV_Camera_SetInterpolateFOV,		&Camera::SetInterpolateFOV },
	{ &EV_Camera_LoadKFC,				&Camera::LoadKFC },
	{ &EV_Camera_Orbit,					&Camera::OrbitEvent },
	{ &EV_Camera_Follow,				&Camera::FollowEvent },
	{ &EV_Camera_Watch,					&Camera::WatchEvent },
	{ &EV_Camera_WatchPath,				&Camera::WatchPathEvent },
	{ &EV_Camera_WatchNodes,			&Camera::WatchNodesEvent },
	{ &EV_Camera_NoWatch,				&Camera::NoWatchEvent },
	{ &EV_Camera_LookAt,				&Camera::LookAt },
	{ &EV_Camera_TurnTo,				&Camera::TurnTo },
	{ &EV_Camera_MoveToEntity,			&Camera::MoveToEntity },
	{ &EV_Camera_MoveToPos,				&Camera::MoveToPos },
	{ &EV_Camera_Cut,					&Camera::Cut },
	{ &EV_Camera_FadeTime,				&Camera::FadeTime },
	{ &EV_Camera_SetNextCamera,			&Camera::SetNextCamera },
	{ &EV_Camera_SetThread,				&Camera::SetThread },
	{ &EV_Camera_SetThread2,			&Camera::SetThread },
	{ &EV_Camera_SetupCamera,			&Camera::SetupCamera },
	{ &EV_SetAngles,					&Camera::SetAnglesEvent },
	{ &EV_Camera_SetAutoState,			&Camera::SetAutoStateEvent },
	{ &EV_Camera_SetAutoRadius,			&Camera::SetAutoRadiusEvent },
	{ &EV_Camera_SetAutoStartTime,		&Camera::SetAutoStartTimeEvent },
	{ &EV_Camera_SetAutoStopTime,		&Camera::SetAutoStopTimeEvent },
	{ &EV_Camera_SetMaximumAutoFOV,		&Camera::SetMaximumAutoFOVEvent },
	{ &EV_Camera_SetAutoActive,			&Camera::SetAutoActiveEvent },

	{ NULL, NULL }
};


//---------------------------------------------------------------------------
// Camera::Camera()
//---------------------------------------------------------------------------
Camera::Camera()
{
	Vector ang;

	camera_fov = 90;
	camera_speed = 1;
	orbit_height = 128;
	orbit_dotrace = true;
	follow_yaw = 0;
	follow_yaw_fixed = false;
	follow_dist = 128;
	follow_mask = MASK_SOLID;
	auto_fov = 0;
	automatic_maxFOV = 90;

	watchTime = 0;
	followTime = 0;
	fovTime = 0;

	fadeTime = 0.2f;

	setSolidType( SOLID_NOT );
	setMoveType( MOVETYPE_NONE );

	showcamera = sv_showcameras->integer;
	if ( showcamera )
	{
		setModel( "func_camera.tik" );
		showModel();
	}
	else
	{
		hideModel();
	}

	automatic_active = true;
	automatic_radius = 512;
	automatic_states.ClearObjectList();
	automatic_startTime = 0.7f;
	automatic_stopTime = 0.7f;

	newCameraPath = NULL;
	newCameraPathSeconds = 0.0f;

	fovFadeTime    = 1.0f;
	followFadeTime = 1.0f;
	watchFadeTime  = 1.0f;

	if ( !LoadingSavegame )
	{
		PostEvent( EV_Camera_SetupCamera, EV_POSTSPAWN );
	}
}


//---------------------------------------------------------------------------
// Camera::SetupCamera
//---------------------------------------------------------------------------
void Camera::SetupCamera( Event*  )
{
	currentstate.Initialize( this );
	newstate.Initialize( this );
	newCameraPathSeconds = 0.0f;

	if ( spawnflags & START_ON )
	{
		PostEvent( EV_Camera_StartMoving, 0.0f );
	}
	if ( spawnflags & AUTOMATIC )
	{
		level.AddAutomaticCamera( this );
	}
}


//---------------------------------------------------------------------------
// Camera::IsAutomatic
//---------------------------------------------------------------------------
qboolean Camera::IsAutomatic( void )
{
	return ( spawnflags & AUTOMATIC );
}


//---------------------------------------------------------------------------
// Camera::IsLevelExit
//---------------------------------------------------------------------------
qboolean Camera::IsLevelExit( void )
{
	return ( spawnflags & LEVEL_EXIT );
}


//---------------------------------------------------------------------------
// Camera::CalculateScore
//---------------------------------------------------------------------------
float Camera::CalculateScore( Entity* player, const str& state )
{
	int i;
	float range;
	float score;
	qboolean found;

	if ( !automatic_active )
	{
		return 10;
	}

	range = Vector( player->origin - origin ).length() / automatic_radius;
	// bias the range so that we don't immediately jump out of the camera if we are out of range
	range -= 0.1f;

	score = range;

	//
	// early exit if our score exceeds 1
	//
	if ( score > 1.0f )
		return score;

	// find out if we match a state
	found = false;
	for( i = 1; i <= automatic_states.NumObjects(); i++ )
	{
		str *auto_state;

		auto_state = &automatic_states.ObjectAt( i );
		if ( !state.icmpn( state, auto_state->c_str(), auto_state->length() ) )
		{
			found = true;
			break;
		}
	}

	// if we are comparing states and we haven't found a valid one...
	if ( automatic_states.NumObjects() && !found )
	{
		// if we aren't in the right state, push our score out significantly
		score += 2.0f;
		return score;
	}

	// perform a trace to the player if necessary
	if ( !( spawnflags & NO_TRACE ) && !( spawnflags & NO_WATCH ) )
	{
		trace_t trace;

		trace = G_Trace( origin, vec_zero, vec_zero, player->centroid, player, follow_mask, false, "Camera::CalculateScore" );
		if ( trace.startsolid || trace.allsolid || ( trace.fraction < 1.0f ) )
		{
			// if we are blocked, push our score out, but not too much since this may be a temporary thing
			if ( trace.startsolid || trace.allsolid )
			{
				score += 2.0f;
				return score;
			}
			else
			{
				score += 1.0f - trace.fraction;
			}
		}
	}

	// perform a dot product test for no watch cameras
	if ( spawnflags & NO_WATCH )
	{
		trace_t trace;
		float  limit;
		float  threshold;
		float  dot;
		Vector dir;

		dir = player->centroid - origin;
		dir.normalize();
		dot = dir * orientation[ 0 ];

		threshold = (float)cos( DEG2RAD( ( camera_fov * 0.25f ) ) );
		if ( dot <= threshold )
		{
			limit = (float)cos( DEG2RAD( ( camera_fov * 0.45f ) ) );
			if ( dot <= limit )
			{
				// we are outside the viewing cone
				score += 2.0f;
				return score;
			}
			else
			{
				// our score is a scale between the two values
				score += ( threshold - dot ) / ( limit );
			}
		}

		trace = G_Trace( origin, vec_zero, vec_zero, player->origin, player, follow_mask, false, "Camera::CalculateScore" );
		if ( trace.startsolid || trace.allsolid || ( trace.fraction < 1.0f ) )
		{
			// if we are blocked, push our score out, but not too much since this may be a temporary thing
			if ( trace.startsolid || trace.allsolid )
			{
				score += 2.0f;
				return score;
			}
			else
			{
				score += 1.0f - trace.fraction;
			}
		}
	}

	return score;
}


//---------------------------------------------------------------------------
// Camera::AutomaticStart
//---------------------------------------------------------------------------
float Camera::AutomaticStart( Entity* player )
{
	if ( !( spawnflags & NO_WATCH ) && player )
	{
		Watch( player, 0.0f );
		Cut( NULL );
	}

	if ( thread.length() )
	{
		ExecuteThread( thread );
	}

	return automatic_startTime;
}


//---------------------------------------------------------------------------
// Camera::AutomaticStop
//---------------------------------------------------------------------------
float Camera::AutomaticStop( Entity* player )
{
	Q_UNUSED(player);

	Stop();
	return automatic_stopTime;
}


//---------------------------------------------------------------------------
// Camera::UpdateStates
//---------------------------------------------------------------------------
void Camera::UpdateStates( void )
{
	if ( followTime && watchTime )
	{
		newstate.Evaluate( this );
	}
	else if ( watchTime )
	{
		newstate.watch.Evaluate( this, &currentstate.move );
	}
	else if ( followTime )
	{
		newstate.move.Evaluate( this );
	}
	currentstate.Evaluate( this );
}



//---------------------------------------------------------------------------
// Camera::CalculatePosition
//---------------------------------------------------------------------------
Vector Camera::CalculatePosition( void )
{
	int      i;
	float    t;
	Vector   pos;

	//
	// calcualte position
	//
	if ( followTime )
	{
		t = followTime - level.time;
		//
		// are we still fading?
		//
		if ( t <= 0.0f )
		{
			//
			// if not zero out the fade
			//
			t = 0.0f;
			currentstate.move = newstate.move;
			newstate.move.Initialize( this );
			followTime = 0;
			pos = currentstate.move.pos;
		}
		else
		{
			//
			// if we are lerp over followFadeTime
			//
			t = ( followFadeTime - t ) / followFadeTime;

			for ( i = 0; i < 3; i++ )
			{
				pos[ i ] = currentstate.move.pos[ i ] + ( t * ( newstate.move.pos[ i ] - currentstate.move.pos[ i ] ) );
			}
		}
	}
	else
	{
		pos = currentstate.move.pos;
	}

	return pos;
}


//---------------------------------------------------------------------------
// Camera::CalculateOrientation
//---------------------------------------------------------------------------
Vector Camera::CalculateOrientation( void )
{
	int      i;
	float    t;
	Vector   ang;

	//
	// calculate orientation
	//
	if ( watchTime )
	{
		t = watchTime - level.time;
		//
		// are we still fading?
		//
		if ( t <= 0.0f )
		{
			//
			// if not zero out the fade
			//
			t = 0.0f;
			currentstate.watch = newstate.watch;
			newstate.watch.Initialize( this );
			watchTime = 0;
			ang = currentstate.watch.watchAngles;
		}
		else
		{
			t = ( watchFadeTime - t ) / watchFadeTime;

			for ( i=0; i<3; i++ )
			{
				ang[ i ] = LerpAngleFromCurrent( currentstate.watch.watchAngles[ i ], newstate.watch.watchAngles[ i ], this->angles[ i ], t );
			}
		}
	}
	else
	{
		ang = currentstate.watch.watchAngles;
	}

	return ang;
}


//---------------------------------------------------------------------------
// Camera::CalculateFov
//---------------------------------------------------------------------------
float Camera::CalculateFov( void )
{
	float    fov;
	float    t;

	//
	// calculate fov
	//
	// check if we have an auto_fov
	if ( auto_fov > 0.0f )
	{
		if ( currentstate.watch.watchEnt )
		{
			float distance;
			float size;
			float new_fov;
			Entity * ent;

			ent = currentstate.watch.watchEnt;
			size = ent->maxs[ 2 ] - ent->mins[ 2 ];
			size = ent->edict->radius / 2.0f;
			// cap the size
			if ( size < 16.0f )
				size = 16.0f;

			distance = Vector( ent->centroid - origin ).length();
			new_fov = RAD2DEG( 2.0f * atan2( size, distance * auto_fov ) );
			if ( new_fov > automatic_maxFOV )
				new_fov = automatic_maxFOV;
			else if ( new_fov < 5.0f )
				new_fov = 5.0f;

			return new_fov;
		}
		else
		{
			return 90.0f;
		}
	}

	// if we get here, we don't have an auto_fov, or we have an invalid watch target
	if ( fovTime )
	{
		t = fovTime - level.time;
		//
		// are we still fading?
		//
		if ( t <= 0.0f )
		{
			//
			// if not zero out the fade
			//
			t = 0.0f;
			currentstate.fov = newstate.fov;
			fovTime = 0;
			fov = currentstate.fov;
		}
		else
		{
			//
			// if we are lerp over fovFadeTime
			//
			t = ( fovFadeTime - t ) / fovFadeTime;
			fov = currentstate.fov + ( t * ( newstate.fov - currentstate.fov ) );
		}
	}
	else
	{
		fov = currentstate.fov;
	}

	return fov;
}


//---------------------------------------------------------------------------
// Camera::GetPathLengthInSeconds
//---------------------------------------------------------------------------
float Camera::GetPathLengthInSeconds( void )
{
	return newCameraPath->GetPathLengthInSeconds();
}

//---------------------------------------------------------------------------
// Camera::GetCameraTime
//---------------------------------------------------------------------------
float
Camera::GetCameraTime( 
	void
	)
{
	return currentstate.move.cameraTime;
}


//---------------------------------------------------------------------------
// Camera::EvaluateCameraKeyFramePath
//---------------------------------------------------------------------------
void Camera::EvaluateCameraKeyFramePath( void )
{
	/// Get a key frame of the camera path when evaluated at this time
	CameraKeyFrame frame = newCameraPath->GetInterpolatedFrameForTime( newCameraPathSeconds );

	/// Set the camera's current position, orientation, and fov
	setOrigin( frame.GetPosition() );
	setAngles( frame.GetEulerAngles() );
	camera_fov = frame.GetFOV();

	/// Advance time by 1 server frame
	newCameraPathSeconds += level.frametime;
}


//---------------------------------------------------------------------------
// Camera::CameraThink
//---------------------------------------------------------------------------
void Camera::CameraThink( Event*  )
{
	/// Check if this camera is using a key-framed camera path
	if( newCameraPath )
	{
		assert( newCameraPath->IsLoaded() );
		EvaluateCameraKeyFramePath();
	}
	else
	{
		UpdateStates();

		setOrigin( CalculatePosition() );
		setAngles( CalculateOrientation() );
		camera_fov = CalculateFov();
	}

	//
	// debug info
	//
	if ( sv_showcameras->integer != showcamera )
	{
		showcamera = sv_showcameras->integer;
		if ( showcamera )
		{
			setModel( "func_camera.tik" );
			showModel();
		}
		else
		{
			hideModel();
		}
	}

	if ( sv_showcameras->integer != showcamera )
	{
		showcamera = sv_showcameras->integer;
		if ( showcamera )
		{
			setModel( "func_camera.tik" );
			showModel();
		}
		else
		{
			hideModel();
		}
	}
	if ( showcamera && currentstate.move.followingpath )
	{
		G_Color3f( 1.0f, 1.0f, 0.0f );
		if ( currentstate.move.orbitEnt )
		{
			currentstate.move.cameraPath.DrawCurve( currentstate.move.orbitEnt->origin, 10 );
		}
		else
		{
			currentstate.move.cameraPath.DrawCurve( 10 );
		}
	}

	CancelEventsOfType( EV_Camera_CameraThink );
	PostEvent( EV_Camera_CameraThink, FRAMETIME );
}


//---------------------------------------------------------------------------
// Camera::LookAt
//---------------------------------------------------------------------------
void Camera::LookAt( Event* ev )
{
   Vector pos, delta;
   Entity * ent;

	ent = ev->GetEntity( 1 );

   if ( !ent )
      return;

   pos.x = ent->origin.x;
   pos.y = ent->origin.y;
	pos.z = ent->absmax.z;
   delta = pos - origin;
   delta.normalize();

   currentstate.watch.watchAngles = delta.toAngles();
   setAngles( currentstate.watch.watchAngles );
}


//---------------------------------------------------------------------------
// Camera::TurnTo
//---------------------------------------------------------------------------
void Camera::TurnTo( Event* ev )
{
	currentstate.watch.watchAngles = ev->GetVector( 1 );
	setAngles( currentstate.watch.watchAngles );
}


//---------------------------------------------------------------------------
// Camera::MoveToEntity
//---------------------------------------------------------------------------
void Camera::MoveToEntity( Event* ev )
{
	Entity * ent;

	ent = ev->GetEntity( 1 );
	if ( ent )
		currentstate.move.pos = ent->origin;

	setOrigin( currentstate.move.pos );
}


//---------------------------------------------------------------------------
// Camera::MoveToPos
//---------------------------------------------------------------------------
void Camera::MoveToPos( Event* ev )
{
	currentstate.move.pos = ev->GetVector( 1 );
	setOrigin( currentstate.move.pos );
}


//---------------------------------------------------------------------------
// Camera::Stop
//---------------------------------------------------------------------------
void Camera::Stop( void )
{
	if ( followTime )
	{
		currentstate.move = newstate.move;
		newstate.move.Initialize( this );
	}

	if ( watchTime )
	{
		currentstate.watch = newstate.watch;
		newstate.watch.Initialize( this );
	}

	CancelEventsOfType( EV_Camera_CameraThink );

	watchTime = 0;
	followTime = 0;
}


//---------------------------------------------------------------------------
// Camera::CreateOrbit
//---------------------------------------------------------------------------
void Camera::CreateOrbit( const Vector& pos, float radius, const Vector& forward, const Vector& left )
{
	newstate.move.cameraPath.Clear();
	newstate.move.cameraPath.SetType( SPLINE_LOOP );

	newstate.move.cameraPath.AppendControlPoint( pos + ( radius * forward ) );
	newstate.move.cameraPath.AppendControlPoint( pos + ( radius * left ) );
	newstate.move.cameraPath.AppendControlPoint( pos - ( radius * forward ) );
	newstate.move.cameraPath.AppendControlPoint( pos - ( radius * left ) );
}


//---------------------------------------------------------------------------
// Camera::CreatePath
//---------------------------------------------------------------------------
void Camera::CreatePath( SplinePath* path, splinetype_t type )
{
	SplinePath	*node;
	SplinePath	*loop;

	newstate.move.cameraPath.Clear();
	newstate.move.cameraPath.SetType( type );

	newstate.move.splinePath = path;
	newstate.move.currentNode = path;
	newstate.move.loopNode = NULL;

	node = path;
	while( node != NULL )
	{
		newstate.move.cameraPath.AppendControlPoint( node->origin, node->angles, node->speed );
		loop = node->GetLoop();
		if ( loop && ( type == SPLINE_LOOP ) )
		{
			newstate.move.loopNode = loop;
			newstate.move.cameraPath.SetLoopPoint( loop->origin );
		}
		node = node->GetNext();

		if ( node == path )
			break;
	}

	if ( ( type == SPLINE_LOOP ) && ( !newstate.move.loopNode ) )
	{
		newstate.move.loopNode = path;
	}
}


//---------------------------------------------------------------------------
// Camera::FollowPath
//---------------------------------------------------------------------------
void Camera::FollowPath( SplinePath* path, qboolean loop, Entity* watch )
{
	// make sure we process any setup events before continuing
	ProcessPendingEvents();

	Stop();
	if ( loop )
	{
		CreatePath( path, SPLINE_LOOP );
	}
	else
	{
		CreatePath( path, SPLINE_CLAMP );
	}

	newstate.move.cameraTime = -2;
	newstate.move.lastTime = 0;
	newstate.move.newTime = 0;
	newstate.move.currentNode = path;
	// evaluate the first node events
	newstate.move.DoNodeEvents( this );

	if ( watch )
	{
		newstate.watch.watchEnt = watch;
	}
	else
	{
		Watch( newstate.move.currentNode->GetWatch(), newstate.move.currentNode->GetFadeTime() );
	}

	followFadeTime = fadeTime;
	watchFadeTime = fadeTime;

	newstate.move.followingpath = true;
	followTime = level.time + followFadeTime;
	watchTime = level.time + watchFadeTime;

	PostEvent( EV_Camera_CameraThink, FRAMETIME );
}


//---------------------------------------------------------------------------
// Camera::Orbit
//---------------------------------------------------------------------------
void Camera::Orbit
(
	Entity *ent,
	float dist,
	Entity *watch,
	float yaw_offset,
	qboolean dotrace
)
{
	Vector ang, forward, left;

	// make sure we process any setup events before continuing
	ProcessPendingEvents();

	Stop();

	if ( watch )
	{
		ang = watch->angles;
		ang.y += yaw_offset;
	}
	else
	{
		ang = vec_zero;
		ang.y += yaw_offset;
	}
	ang.AngleVectors( &forward, &left, NULL );

	orbit_dotrace = dotrace;

	CreateOrbit( Vector( 0.0f, 0.0f, orbit_height ), dist, forward, left );
	newstate.move.cameraTime = -2;
	newstate.move.lastTime = 0;
	newstate.move.newTime = 0;

	newstate.move.orbitEnt = ent;

	followFadeTime = fadeTime;
	watchFadeTime = fadeTime;

	newstate.move.followingpath = true;
	followTime = level.time + followFadeTime;
	watchTime = level.time + watchFadeTime;
	newstate.move.currentNode = NULL;

	if ( watch )
	{
		newstate.watch.watchEnt = watch;
	}
	else
	{
		newstate.watch.watchEnt = ent;
	}

	PostEvent( EV_Camera_CameraThink, FRAMETIME );
}


//---------------------------------------------------------------------------
// Camera::FollowEntity
//---------------------------------------------------------------------------
void Camera::FollowEntity( Entity* ent, float dist, int mask, Entity* watch )
{
	// make sure we process any setup events before continuing
	ProcessPendingEvents();

	assert( ent );

	Stop();

	if ( ent )
	{
		newstate.move.followEnt = ent;
		newstate.move.followingpath = false;

		followFadeTime = fadeTime;
		watchFadeTime = fadeTime;

		newstate.move.cameraTime = -2;
		newstate.move.lastTime = 0;
		newstate.move.newTime = 0;
		newstate.move.currentNode = NULL;

		followTime = level.time + followFadeTime;
		watchTime = level.time + watchFadeTime;
		if ( watch )
		{
			newstate.watch.watchEnt = watch;
		}
		else
		{
			newstate.watch.watchEnt = ent;
		}
		follow_dist = dist;
		follow_mask = mask;
		PostEvent( EV_Camera_CameraThink, 0.0f );
	}
}


//---------------------------------------------------------------------------
// Camera::StartMoving
//---------------------------------------------------------------------------
void Camera::StartMoving( Event* ev )
{
	Entity *targetEnt;
	Entity *targetWatchEnt;
	Entity *ent;
	SplinePath *path;

	newCameraPathSeconds = 0.0f;

	if ( ev->NumArgs() > 0 )
	{
		targetEnt = ev->GetEntity( 1 );
	}
	else
	{
		targetEnt = NULL;
	}

	if ( ev->NumArgs() > 1 )
	{
		targetWatchEnt = ev->GetEntity( 2 );
	}
	else
	{
		targetWatchEnt = NULL;
	}

	if ( ( spawnflags & START_ON ) && ( !Q_stricmp( Target(), "" ) ) )
	{
		gi.Error( ERR_DROP, "Camera '%s' with START_ON selected, but no target specified.", TargetName() );
	}

	if ( !targetEnt )
	{
		ent = G_FindTarget( NULL, Target() );
		if ( !ent )
		{
			gi.Error( ERR_DROP, "Can't find target '%s' for camera\n", Target() );
			return;
		}
	}
	else
	{
		ent = targetEnt;
	}

	if ( ent->isSubclassOf( SplinePath ) )
	{
		path = ( SplinePath * )ent;
		FollowPath( path, spawnflags & ORBIT, targetWatchEnt );
	}
	else
	{
		if ( spawnflags & ORBIT )
		{
			Orbit( ent, follow_dist, targetWatchEnt );
		}
		else
		{
			FollowEntity( ent, follow_dist, follow_mask, targetWatchEnt );
		}
	}
}


//---------------------------------------------------------------------------
// Camera::SetAutoStateEvent
//---------------------------------------------------------------------------
void Camera::SetAutoStateEvent( Event* ev )
{
	int i;

	for( i = 1; i <= ev->NumArgs(); i++ )
	{
		char  *buffer;
		char	com_token[ MAX_QPATH ];
		char  com_buffer[MAX_STRING_CHARS];

		strcpy( com_buffer, ev->GetString( i ) );
		buffer = com_buffer;
		// get the rest of the line
		while( 1 )
		{
			strcpy( com_token, COM_ParseExt( &buffer, false ) );
			if (!com_token[0])
			break;

			automatic_states.AddUniqueObject( str( com_token ) );
		}
	}
}


//---------------------------------------------------------------------------
// Camera::SetMaximumAutoFOVEvent
//---------------------------------------------------------------------------
void Camera::SetMaximumAutoFOVEvent( Event* ev )
{
	automatic_maxFOV = ev->GetFloat( 1 );
}


//---------------------------------------------------------------------------
// Camera::SetAutoRadiusEvent
//---------------------------------------------------------------------------
void Camera::SetAutoRadiusEvent( Event* ev )
{
	automatic_radius = ev->GetFloat( 1 );
}


//---------------------------------------------------------------------------
// Camera::SetAutoActiveEvent
//---------------------------------------------------------------------------
void Camera::SetAutoActiveEvent( Event* ev )
{
	automatic_active = ev->GetBoolean( 1 );
}


//---------------------------------------------------------------------------
// Camera::SetAutoStartTimeEvent
//---------------------------------------------------------------------------
void Camera::SetAutoStartTimeEvent( Event* ev )
{
	automatic_startTime = ev->GetFloat( 1 );
}


//---------------------------------------------------------------------------
// Camera::SetAutoStopTimeEvent
//---------------------------------------------------------------------------
void Camera::SetAutoStopTimeEvent( Event* ev )
{
	automatic_stopTime = ev->GetFloat( 1 );
}


//---------------------------------------------------------------------------
// Camera::StopMoving
//---------------------------------------------------------------------------
void Camera::StopMoving( Event*  )
{
	Stop();
}


//---------------------------------------------------------------------------
// Camera::Pause
//---------------------------------------------------------------------------
void Camera::Pause( Event*  )
{
	CancelEventsOfType( EV_Camera_CameraThink );
}


//---------------------------------------------------------------------------
// Camera::Continue
//---------------------------------------------------------------------------
void Camera::Continue( Event*  )
{
	CancelEventsOfType( EV_Camera_CameraThink );
	PostEvent( EV_Camera_CameraThink, 0.0f );
}


//---------------------------------------------------------------------------
// Camera::SetAnglesEvent
//---------------------------------------------------------------------------
void Camera::SetAnglesEvent( Event* ev )
{
	Vector ang;

	ang = ev->GetVector( 1 );
	setAngles( ang );
}


//---------------------------------------------------------------------------
// Camera::SetSpeed
//---------------------------------------------------------------------------
void Camera::SetSpeed( Event* ev )
{
	camera_speed = ev->GetFloat( 1 );
}


//---------------------------------------------------------------------------
// Camera::SetFollowDistance
//---------------------------------------------------------------------------
void Camera::SetFollowDistance( Event* ev )
{
	follow_dist = ev->GetFloat( 1 );
}


//---------------------------------------------------------------------------
// Camera::SetOrbitHeight
//---------------------------------------------------------------------------
void Camera::SetOrbitHeight( float height )
{
	orbit_height = height;
}


//---------------------------------------------------------------------------
// Camera::SetOrbitHeight
//---------------------------------------------------------------------------
void Camera::SetOrbitHeight( Event* ev )
{
	orbit_height = ev->GetFloat( 1 );
}


//---------------------------------------------------------------------------
// Camera::SetFollowYaw
//---------------------------------------------------------------------------
void Camera::SetFollowYaw( Event* ev )
{
	follow_yaw = ev->GetFloat( 1 );
}


//---------------------------------------------------------------------------
// Camera::AbsoluteYaw
//---------------------------------------------------------------------------
void Camera::AbsoluteYaw( Event*  )
{
	follow_yaw_fixed = true;
}


//---------------------------------------------------------------------------
// Camera::RelativeYaw
//---------------------------------------------------------------------------
void Camera::RelativeYaw( Event*  )
{
	follow_yaw_fixed = false;
}


//---------------------------------------------------------------------------
// Camera::SetNextCamera
//---------------------------------------------------------------------------
void Camera::SetNextCamera( Event* ev )
{
	nextCamera = ev->GetString( 1 );
}


//---------------------------------------------------------------------------
// Camera::Cut
//---------------------------------------------------------------------------
void Camera::Cut( Event*  )
{
	int         j;

	if ( followTime )
	{
		currentstate.move = newstate.move;
		newstate.move.Initialize( this );
		followTime = 0;
	}

	if ( watchTime )
	{
		currentstate.watch = newstate.watch;
		newstate.watch.Initialize( this );
		watchTime = 0;
	}

	if ( fovTime )
	{
		currentstate.fov = newstate.fov;
		//currentstate.fov = g_entities[0].entity->client->ps.fov;
		newstate.fov = camera_fov;
		fovTime = 0;
	}

	CancelEventsOfType( EV_Camera_CameraThink );
	ProcessEvent( EV_Camera_CameraThink );

	//
	// let any clients know that this camera has just cut
	//
	for( j = 0; j < game.maxclients; j++ )
	{
		gentity_t   *other;
		Player      *client;

		other = &g_entities[ j ];
		if ( other->inuse && other->client )
		{
			client = ( Player * )other->entity;
			client->CameraCut( this );
		}
	}
}


//---------------------------------------------------------------------------
// Camera::FadeTime
//---------------------------------------------------------------------------
void Camera::FadeTime( Event* ev )
{
	fadeTime = ev->GetFloat( 1 );
}

void Camera::OrbitEvent( Event* ev )
{
	Entity *ent;

	spawnflags |= ORBIT;
	ent = ev->GetEntity( 1 );
	if ( ent )
	{
		Event * event;

		event = new Event( EV_Camera_StartMoving );
		event->AddEntity( ent );
		if ( ev->NumArgs() > 1 )
		{
			event->AddEntity( ev->GetEntity( 2 ) );
		}

		Stop();
		ProcessEvent( event );
	}
}


//---------------------------------------------------------------------------
// Camera::FollowEvent
//---------------------------------------------------------------------------
void Camera::FollowEvent( Event* ev )
{
	Entity *ent;

	spawnflags &= ~ORBIT;
	ent = ev->GetEntity( 1 );
	if ( ent )
	{
		Event * event;

		event = new Event( EV_Camera_StartMoving );
		event->AddEntity( ent );
		if ( ev->NumArgs() > 1 )
		{
			event->AddEntity( ev->GetEntity( 2 ) );
		}

		Stop();
		ProcessEvent( event );
	}
	else
	{
		gi.WDPrintf( "FollowEvent :: Entity does not exist." );
	}
}


//---------------------------------------------------------------------------
// Camera::SetFOV
//---------------------------------------------------------------------------
void Camera::SetFOV( Event* ev )
{
	float time;

	if ( ev->NumArgs() > 1 )
	{
		time = ev->GetFloat( 2 );
	}
	else
	{
		time = fadeTime;
	}

	SetFOV( ev->GetFloat( 1 ), time );
}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Camera::SetInterpolateFOV( Event* ev )
{
	SetFOV( ev->GetFloat(1), ev->GetFloat(2));
}

//---------------------------------------------------------------------------
// Camera::LoadKFC
//---------------------------------------------------------------------------
void Camera::LoadKFC( Event* ev )
{
	str kfcFileName = ev->GetString( 1 );
	delete newCameraPath; // destroy any prior CameraPath instance that may be lingering
	newCameraPath = new CameraPath( kfcFileName );
	newCameraPathSeconds = 0.0f;
}


//---------------------------------------------------------------------------
// Camera::WatchEvent
//---------------------------------------------------------------------------
void Camera::WatchEvent( Event* ev )
{
	float time;

	if ( ev->NumArgs() > 1 )
	{
		time = ev->GetFloat( 2 );
	}
	else
	{
		time = fadeTime;
	}

	Watch( ev->GetString( 1 ), time );
}


//---------------------------------------------------------------------------
// GetWatchEntity
//---------------------------------------------------------------------------
Entity * GetWatchEntity( const str& watch )
{
	const char *name;
	Entity * ent;

	//
	// if empty just return
	//
	if ( watch == "" )
		return NULL;

	//
	// ignore all the reserved words
	//
	if (( watch == "path" ) ||
		( watch == "none" ) ||
		( watch == "node" ) )
	{
		return NULL;
	}

	name = watch.c_str();

	if ( name[ 0 ] == '*' )
	{
		if ( !IsNumeric( &name[ 1 ] ) )
		{
			gi.WDPrintf( "GetWatchEntity :: Expecting a numeric value but found '%s'.", &name[ 1 ] );
			return NULL;
		}
		else
		{
			ent = G_GetEntity( atoi( &name[ 1 ] ) );
			if ( !ent )
			{
				gi.WDPrintf( "GetWatchEntity :: Entity with targetname of '%s' not found", &name[ 1 ] );
				return NULL;
			}
		}
	}
	else if ( name[ 0 ] == '$' )
	{
		ent = G_FindTarget( NULL, &name[ 1 ] );
		if ( !ent )
		{
			gi.WDPrintf( "GetWatchEntity :: Entity with targetname of '%s' not found", &name[ 1 ] );
			return NULL;
		}
	}
	else
	{
		gi.WDPrintf( "GetWatchEntity :: Entity with targetname of '%s' not found", name );
		return NULL;
	}

	return ent;
}


//---------------------------------------------------------------------------
// Camera::Watch
//---------------------------------------------------------------------------
void Camera::Watch( const str& watch, float time )
{
	// make sure we process any setup events before continuing
	ProcessPendingEvents();

	//
	// if empty just return
	//
	if ( watch == "" )
		return;

	//
	// clear out the watch variables
	//
	watchFadeTime = time;
	newstate.watch.watchPath = false;
	newstate.watch.watchNodes = false;
	newstate.watch.watchEnt = NULL;
	watchTime = level.time + watchFadeTime;

	//
	// really a watchpath command
	//
	if ( watch == "path" )
	{
		newstate.watch.watchPath = true;
	}

	//
	// really a watchnodes command
	//
	else if ( watch == "node" )
	{
		newstate.watch.watchNodes = true;
	}

	//
	// really a watchnodes command
	//
	else if ( watch == "none" )
	{
		// intentionally blank
	}

	//
	// just a normal watch command
	//
	else
	{
		Entity * ent = GetWatchEntity( watch );
		newstate.watch.watchEnt = ent;
	}
}


//---------------------------------------------------------------------------
// Camera::Watch
//---------------------------------------------------------------------------
void Camera::Watch( Entity* ent, float time )
{
	//
	// clear out the watch variables
	//
	watchFadeTime = time;
	newstate.watch.watchPath = false;
	newstate.watch.watchNodes = false;
	watchTime = level.time + watchFadeTime;
	newstate.watch.watchEnt = ent;
}


//---------------------------------------------------------------------------
// Camera::SetFOV
//---------------------------------------------------------------------------
void Camera::SetFOV( float fov, float time )
{
	// if it is less than 3, then we are setting an auto_fov state
	if ( fov < 3.0f )
	{
		auto_fov = fov;
	}
	else
	{
		// if we are explicitly setting the fov, turn the auto_fov off
		auto_fov = 0;

		fovFadeTime = time;
		fovTime = level.time + fovFadeTime;
		//currentstate.fov = newstate.fov;
		currentstate.fov = g_entities[0].entity->client->ps.fov;
		newstate.fov = fov;
	}
}


//---------------------------------------------------------------------------
// Camera::WatchPathEvent
//---------------------------------------------------------------------------
void Camera::WatchPathEvent( Event* ev )
{
	if ( ev->NumArgs() > 1 )
	{
		watchFadeTime = ev->GetFloat( 2 );
	}
	else
	{
		watchFadeTime = fadeTime;
	}

	watchTime = level.time + watchFadeTime;
	newstate.watch.watchEnt = NULL;
	newstate.watch.watchNodes = false;
	newstate.watch.watchPath = true;
}


//---------------------------------------------------------------------------
// Camera::WatchNodesEvent
//---------------------------------------------------------------------------
void Camera::WatchNodesEvent( Event* ev )
{
	if ( ev->NumArgs() > 1 )
	{
		watchFadeTime = ev->GetFloat( 2 );
	}
	else
	{
		watchFadeTime = fadeTime;
	}

	watchTime = level.time + watchFadeTime;
	newstate.watch.watchEnt = NULL;
	newstate.watch.watchPath = false;
	newstate.watch.watchNodes = true;
}


//---------------------------------------------------------------------------
// Camera::NoWatchEvent
//---------------------------------------------------------------------------
void Camera::NoWatchEvent( Event* ev )
{
	if ( ev->NumArgs() > 1 )
	{
		watchFadeTime = ev->GetFloat( 2 );
	}
	else
	{
		watchFadeTime = fadeTime;
	}

	watchTime = level.time + watchFadeTime;
	newstate.watch.watchEnt = NULL;
	newstate.watch.watchPath = false;
	newstate.watch.watchNodes = false;
}


//---------------------------------------------------------------------------
// SetCamera
//---------------------------------------------------------------------------
void SetCamera( Entity* ent, float switchTime )
{
	int         j;
	gentity_t   *other;
	Camera      *cam;
	Player      *client;

	if ( ent && !ent->isSubclassOf( Camera ) )
		return;

	cam = ( Camera * )ent;
	for( j = 0; j < game.maxclients; j++ )
	{
		other = &g_entities[ j ];
		if ( other->inuse && other->client )
		{
			client = ( Player * )other->entity;
			client->SetCamera( cam, switchTime );
		}
	}
}


//---------------------------------------------------------------------------
// Camera::NextCamera
//---------------------------------------------------------------------------
str& Camera::NextCamera( void )
{
	return nextCamera;
}


//---------------------------------------------------------------------------
// Camera::SetThread
//---------------------------------------------------------------------------
void Camera::SetThread( Event* ev )
{
	thread = ev->GetString( 1 );
}


//---------------------------------------------------------------------------
// Camera::Thread
//---------------------------------------------------------------------------
str& Camera::Thread( void )
{
	return thread;
}


//-----------------------------------------------------------------------------------------------
// Name:		SetPlaybackOffsets	
// Class:		Camera
//				
// Description:	Sets the playback offsets for this camera instance.  All positions and
//				orientations included in the camera's CameraPath will be rotated (about
//				the Z axis by <yawOffsetDegrees>) and translated (by <originOffset>).
//				
// Parameters:	const float yawOffsetDegrees - rotation for camera's position & orientation
//				const Vector& originOffset - translation for camera's position (applied second)
//				
// Returns:		void 
//				
//-----------------------------------------------------------------------------------------------
void Camera::SetPlaybackOffsets( const float yawOffsetDegrees, const Vector& originOffset )
{
	/// Check if a key-framed camera path is being used
	if( newCameraPath )
	{
		/// Apply the offsets to the CameraPath object
		newCameraPath->SetPlaybackOffsets( yawOffsetDegrees, originOffset );
	}
}


//---------------------------------------------------------------------------
// Camera::Fov
//---------------------------------------------------------------------------
float Camera::Fov( void )
{
	return camera_fov;
}


//---------------------------------------------------------------------------
// Camera::Reset
//---------------------------------------------------------------------------
void Camera::Reset( const Vector& org, const Vector& ang )
{
	setOrigin( org );
	setAngles( ang );
	currentstate.Initialize( this );
	newstate.Initialize( this );
}


//---------------------------------------------------------------------------
// Camera::bind
//---------------------------------------------------------------------------
void Camera::bind( Entity* master, qboolean use_my_angles )
{
	Entity::bind( master, use_my_angles );
	currentstate.move.pos = GetLocalOrigin();
}


//---------------------------------------------------------------------------
// Camera::unbind
//---------------------------------------------------------------------------
void Camera::unbind( void )
{
	Entity::unbind();
	currentstate.move.pos = origin;
}


/******************************************************************************

  Camera Manager

******************************************************************************/

Event EV_CameraManager_NewPath
(
	"new",
	EV_CONSOLE,
	NULL,
	NULL,
	"Starts a new path."
);
Event EV_CameraManager_SetPath
(
	"setpath",
	EV_CONSOLE,
	"e",
	"path",
	"Sets the new path."
);
Event EV_CameraManager_SetTargetName
(
	"settargetname",
	EV_CONSOLE,
	"s",
	"targetname",
	"Set the targetname."
);
Event EV_CameraManager_SetTarget
(
	"settarget",
	EV_CONSOLE,
	"s",
	"target",
	"Set the trigger target."
);
Event EV_CameraManager_AddPoint
(
	"add",
	EV_CONSOLE,
	NULL,
	NULL,
	"Add a new point to the camera path where the player is standing."
);
Event EV_CameraManager_DeletePoint
(
	"delete",
	EV_CONSOLE,
	NULL,
	NULL,
	"Delete the current path node."
);
Event EV_CameraManager_MovePlayer
(
	"moveplayer",
	EV_CONSOLE,
	NULL,
	NULL,
	"Move the player to the current path node position."
);
Event EV_CameraManager_ReplacePoint
(
	"replace",
	EV_CONSOLE,
	NULL,
	NULL,
	"Replace the current path node position/angle with the player's."
);	
Event EV_CameraManager_NextPoint
(
	"next",
	EV_CONSOLE,
	NULL,
	NULL,
	"Go to the next path node."
);
Event EV_CameraManager_PreviousPoint
(
	"prev",
	EV_CONSOLE,
	NULL,
	NULL,
	"Go to the previous path node."
);
Event EV_CameraManager_ShowPath
(
	"showpath",
	EV_CONSOLE,
	"E",
	"path",
	"Shows the specified path."
);
Event EV_CameraManager_ShowingPath
(
	"_showing_path",
	EV_CONSOLE,
	NULL,
	NULL,
	"Internal event for showing the path."
);
Event EV_CameraManager_HidePath
(
	"hidepath",
	EV_CONSOLE,
	NULL,
	NULL,
	"Hides the paths."
);
Event EV_CameraManager_PlayPath
(
	"play",
	EV_CONSOLE,
	"E",
	"path",
	"Play the current path or the specified one once."
);
Event EV_CameraManager_PlayKFCPath
(
	"playKFCpath",
	EV_CONSOLE,
	"sFV",
	"kfc_filename yaw_offset origin_offset",
	"Loads a key-framed camera path (.KFC) file.  Yaw_offset and origin_offset are optional."
);
Event EV_CameraManager_DestroyKFCPath
(
	"destroyKFCpath",
	EV_CODEONLY,
	"E",
	"path",
	"Delete the specified .kfc file."
);
Event EV_CameraManager_LoopPath
(
	"loopPath",
	EV_CONSOLE,
	"E",
	"path",
	"Loop the current path or the specified one."
);
Event EV_CameraManager_StopPlayback
(
	"stop",
	EV_CONSOLE | EV_SCRIPTONLY,
	NULL,
	NULL,
	"Stop the camera playing path."
);
Event EV_CameraManager_Watch
(
	"watch",
	EV_SCRIPTONLY,
	"s",
	"watch",
	"Set the current path node to watch something."
);
Event EV_CameraManager_NoWatch
(
	"nowatch",
	EV_CONSOLE | EV_SCRIPTONLY,
	NULL,
	NULL,
	"Set the current path node to watch nothing."
);
Event EV_CameraManager_Fov
(
	"setfov",
	EV_CONSOLE,
	"s",
	"newFOV",
	"Set the fov at the current path node."
);
Event EV_CameraManager_FadeTime
(
	"setfadetime",
	EV_CONSOLE,
	"f",
	"newFadeTime",
	"Set the fadetime of the current path node."
);
Event EV_CameraManager_Speed
(
	"setspeed",
	EV_CONSOLE,
	"f",
	"speed",
	"Set the speed of the camera at the current path node."
);
Event EV_CameraManager_Save
(
	"save",
	EV_CONSOLE,
	"s",
	"filename",
	"Saves the camera path."
);
Event EV_CameraManager_Load
(
	"load",
	EV_CONSOLE,
	"s",
	"filename",
	"Loads a camera path."
);
Event EV_CameraManager_SaveMap
(
	"savemap",
	EV_CONSOLE,
	"s",
	"filename",
	"Saves the camera path to a map file."
);
Event EV_CameraManager_SetThread
(
	"setthread",
	EV_SCRIPTONLY,
	"s",
	"thread",
	"Sets the thread for the current path node."
);
Event EV_CameraManager_UpdateInput
(
	"updateinput",
	EV_CONSOLE,
	NULL,
	NULL,
	"Updates the current node with user interface values."
);
Event EV_CameraManager_NextPath
(
	"nextpath",
	EV_CONSOLE,
	NULL,
	NULL,
	"Go to the next path."
);
Event EV_CameraManager_PreviousPath
(
	"prevpath",
	EV_CONSOLE,
	NULL,
	NULL,
	"Go to the previous path."
);
Event EV_CameraManager_RenamePath
(
	"renamepath",
	EV_CONSOLE,
	"s",
	"newName",
	"Rename the path to the new name."
);

CLASS_DECLARATION( Listener, CameraManager, NULL )
{
	{ &EV_CameraManager_NewPath,			&CameraManager::NewPath },
	{ &EV_CameraManager_SetPath,			&CameraManager::SetPath },
	{ &EV_CameraManager_SetTargetName,		&CameraManager::SetTargetName },
	{ &EV_CameraManager_SetTarget,			&CameraManager::SetTarget },
	{ &EV_CameraManager_SetThread,			&CameraManager::SetThread },
	{ &EV_CameraManager_SetPath,			&CameraManager::SetPath },
	{ &EV_CameraManager_AddPoint,			&CameraManager::AddPoint },
	{ &EV_CameraManager_ReplacePoint,		&CameraManager::ReplacePoint },
	{ &EV_CameraManager_DeletePoint,		&CameraManager::DeletePoint },
	{ &EV_CameraManager_MovePlayer,			&CameraManager::MovePlayer },
	{ &EV_CameraManager_NextPoint,			&CameraManager::NextPoint },
	{ &EV_CameraManager_PreviousPoint,		&CameraManager::PreviousPoint },
	{ &EV_CameraManager_ShowPath,			&CameraManager::ShowPath },
	{ &EV_CameraManager_ShowingPath,		&CameraManager::ShowingPath },
	{ &EV_CameraManager_HidePath,			&CameraManager::HidePath },
	{ &EV_CameraManager_PlayPath,			&CameraManager::PlayPath },
	{ &EV_CameraManager_PlayKFCPath,		&CameraManager::PlayKFCPath },
	{ &EV_CameraManager_DestroyKFCPath,		&CameraManager::DestroyKFCPath },
	{ &EV_CameraManager_LoopPath,			&CameraManager::LoopPath },
	{ &EV_CameraManager_StopPlayback,		&CameraManager::StopPlayback },
	{ &EV_CameraManager_Watch,				&CameraManager::Watch },
	{ &EV_CameraManager_NoWatch,			&CameraManager::NoWatch },
	{ &EV_CameraManager_Fov,				&CameraManager::Fov },
	{ &EV_CameraManager_FadeTime,			&CameraManager::FadeTime },
	{ &EV_CameraManager_Speed,				&CameraManager::Speed },
	{ &EV_CameraManager_Save,				&CameraManager::Save },
	{ &EV_CameraManager_Load,				&CameraManager::Load },
	{ &EV_CameraManager_SaveMap,			&CameraManager::SaveMap },
	{ &EV_CameraManager_UpdateInput,		&CameraManager::UpdateEvent },
	{ &EV_CameraManager_NextPath,			&CameraManager::NextPath },
	{ &EV_CameraManager_PreviousPath,		&CameraManager::PreviousPath },
	{ &EV_CameraManager_RenamePath,			&CameraManager::RenamePath },

	{ NULL, NULL }
};


//---------------------------------------------------------------------------
// CameraManager_GetPlayer
//---------------------------------------------------------------------------
Player *CameraManager_GetPlayer( void )
{
	assert( g_entities[ 0 ].entity && g_entities[ 0 ].entity->isSubclassOf( Player ) );
	if ( !g_entities[ 0 ].entity || !( g_entities[ 0 ].entity->isSubclassOf( Player ) ) )
	{
		gi.WPrintf( "No player found.\n" );
		return NULL;
	}

	return ( Player * )g_entities[ 0 ].entity;
}


//---------------------------------------------------------------------------
// CameraManager::CameraManager()
//---------------------------------------------------------------------------
CameraManager::CameraManager()
{
	pathList.ClearObjectList();
	path = NULL;
	current = NULL;
	cameraPath_dirty = true;
	speed = 1;
	watch = 0;
	cam = NULL;
	isPreviewPlaybackRunning = false;
}


//---------------------------------------------------------------------------
// CameraManager::UpdateUI
//---------------------------------------------------------------------------
void CameraManager::UpdateUI( void )
{
	int   num;
	SplinePath *next;
	float temp;

	//
	// set path name
	//
	gi.cvar_set( "cam_filename", pathName );
	if ( current )
	{
		gi.cvar_set( "cam_origin", va( "%.2f %.2f %.2f", current->origin[ 0 ], current->origin[ 1 ], current->origin[ 2 ] ) );
		gi.cvar_set( "cam_angles_yaw", va( "%.1f", current->angles[ YAW ] ) );
		gi.cvar_set( "cam_angles_pitch", va( "%.1f", current->angles[ PITCH ] ) );
		gi.cvar_set( "cam_angles_roll", va( "%.1f", current->angles[ ROLL ] ) );
		gi.cvar_set( "cam_thread", current->thread.c_str() );
		gi.cvar_set( "cam_target", current->triggertarget.c_str() );
		gi.cvar_set( "cam_watch", current->watchEnt.c_str() );
		gi.cvar_set( "cam_playbacktime", va( "0" ) );
		temp = current->GetFov();
		if ( temp )
		{
			gi.cvar_set( "cam_fov", va( "%.1f", temp ) );
		}
		else
		{
			gi.cvar_set( "cam_fov", "Default" );
		}

		temp = current->GetFadeTime();
		if ( temp != -1 )
		{
			gi.cvar_set( "cam_fadetime", va( "%.2f", temp ) );
		}
		else
		{
			gi.cvar_set( "cam_fadetime", "Default" );
		}

		gi.cvar_set( "cam_speed", va( "%.1f", current->speed ) );
		if ( EventPending( EV_CameraManager_ShowingPath ) )
		{
			gi.cvar_set( "cam_hiddenstate", "visible" );
		}
		else
		{
			gi.cvar_set( "cam_hiddenstate", "hidden" );
		}

		//
		// set node num
		//
		num = 0;
		next = path;
		while ( next && ( next != current ) )
		{
			next = next->GetNext();
			num++;
		}
		gi.cvar_set( "cam_nodenum", va( "%d", num ) );
	}
}


//---------------------------------------------------------------------------
// CameraManager::UpdateEvent
//---------------------------------------------------------------------------
void CameraManager::UpdateEvent( Event*  )
{
	Vector tempvec;
	cvar_t * cvar;

	if ( !current )
		return;

	// get origin
	cvar = gi.cvar( "cam_origin", "", 0 );
	sscanf( cvar->string, "%f %f %f", &tempvec[ 0 ], &tempvec[ 1 ], &tempvec[ 2 ] );
	current->setOrigin( tempvec );

	// get angles yaw
	cvar = gi.cvar( "cam_angles_yaw", "", 0 );
	current->angles[ YAW ] = cvar->value;

	// get angles pitch
	cvar = gi.cvar( "cam_angles_pitch", "", 0 );
	current->angles[ PITCH ] = cvar->value;

	// get angles roll
	cvar = gi.cvar( "cam_angles_roll", "", 0 );
	current->angles[ ROLL ] = cvar->value;

	current->setAngles( current->angles );

	// get thread
	cvar = gi.cvar( "cam_thread", "", 0 );
	current->SetThread( cvar->string );

	// get target
	cvar = gi.cvar( "cam_target", "", 0 );
	current->SetTriggerTarget( cvar->string );

	// get watch
	cvar = gi.cvar( "cam_watch", "", 0 );
	current->SetWatch( cvar->string );

	// get fov
	cvar = gi.cvar( "cam_fov", "", 0 );
	current->SetFov( cvar->value );

	// get fadetime
	cvar = gi.cvar( "cam_fadetime", "", 0 );
	current->SetFadeTime( cvar->value );

	// get speed
	cvar = gi.cvar( "cam_speed", "", 0 );
	current->speed = cvar->value;
	speed = current->speed;
}


//---------------------------------------------------------------------------
// CameraManager::SetPathName
//---------------------------------------------------------------------------
void CameraManager::SetPathName( const str& name )
{
	pathName = name;
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::NewPath
//---------------------------------------------------------------------------
void CameraManager::NewPath( Event*  )
{
	if ( path )
	{
		cameraPath_dirty = true;
		path = NULL;
		current = NULL;
	}
	SetPathName( "untitled" );
	ShowPath();
}


//---------------------------------------------------------------------------
// CameraManager::RenamePath
//---------------------------------------------------------------------------
void CameraManager::RenamePath( Event* ev )
{
	str name;

	if ( !ev->NumArgs() )
	{
		cvar_t * cvar;

		//
		// get the path name from the cvar
		//
		cvar = gi.cvar( "cam_filename", "", 0 );
		if ( cvar->string[ 0 ] )
		{
			name = cvar->string;
		}
		else
		{
			ev->Error( "Usage: cam renamepath [pathname]" );
			return;
		}
	}
	else
	{
		name = ev->GetString( 1 );
	}

	if ( pathList.ObjectInList( name ) )
	{
		// remove the old name
		pathList.RemoveObject( name );
	}
	SetPathName( name );
	pathList.AddUniqueObject( name );
}


//---------------------------------------------------------------------------
// CameraManager::SetPath
//---------------------------------------------------------------------------
void CameraManager::SetPath( const str& pathName )
{
	Entity * ent;
	SplinePath *node;

	ent = G_FindTarget( NULL, pathName );

	if ( !ent )
	{
		warning( "SetPath", "Could not find path named '%s'.", pathName.c_str() );
		return;
	}

	if ( !ent->isSubclassOf( SplinePath ) )
	{
		warning( "SetPath", "'%s' is not a camera path.", pathName.c_str() );
		return;
	}

	node = ( SplinePath * )ent;

	SetPathName( pathName );
	cameraPath_dirty = true;
	path = node;
	current = node;
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::SetPath
//---------------------------------------------------------------------------
void CameraManager::SetPath( Event* ev )
{
	if ( !ev->NumArgs() )
	{
		ev->Error( "Usage: cam setpath [pathname]" );
		return;
	}

	SetPath( ev->GetString( 1 ) );
}


//---------------------------------------------------------------------------
// CameraManager::SetTargetName
//---------------------------------------------------------------------------
void CameraManager::SetTargetName( Event* ev )
{
	if ( ev->NumArgs() != 1 )
	{
		ev->Error( "Usage: cam targetname [name]" );
		return;
	}

	if ( !path )
	{
		ev->Error( "Camera path not set." );
		return;
	}

	path->SetTargetName( ev->GetString( 1 ) );
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::SetTarget
//---------------------------------------------------------------------------
void CameraManager::SetTarget( Event* ev )
{
	if ( ev->NumArgs() != 1 )
	{
		ev->Error( "Usage: cam target [name]" );
		return;
	}

	if ( !current )
	{
		ev->Error( "Camera path not set." );
		return;
	}

	current->SetTriggerTarget( ev->GetString( 1 ) );
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::SetThread
//---------------------------------------------------------------------------
void CameraManager::SetThread( Event* ev )
{
	if ( ev->NumArgs() != 1 )
	{
		ev->Error( "Usage: cam thread [name]" );
		return;
	}

	if ( !current )
	{
		ev->Error( "Camera path not set." );
		return;
	}

	current->SetThread( ev->GetString( 1 ) );
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::AddPoint
//---------------------------------------------------------------------------
void CameraManager::AddPoint( Event*  )
{
	Player *player;
	SplinePath *prev;
	SplinePath *next;
	Vector ang;
	Vector pos;

	player = CameraManager_GetPlayer();
	if ( player )
	{
		prev = current;
		if ( current )
		{
			next = current->GetNext();
		}
		else
		{
			next = NULL;
		}

		player->GetPlayerView( &pos, &ang );

		current = new SplinePath;
		current->setOrigin( pos );
		current->setAngles( ang );
		current->speed = speed;
		current->SetPrev( prev );
		current->SetNext( next );

		if ( !path )
		{
			path = current;
		}

		ShowPath();
	}
	cameraPath_dirty = true;
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::ReplacePoint
//---------------------------------------------------------------------------
void CameraManager::ReplacePoint( Event*  )
{
	Player *player;
	Vector ang;
	Vector pos;

	player = CameraManager_GetPlayer();
	if ( current && player )
	{
		player->GetPlayerView( &pos, &ang );

		current->setOrigin( pos );
		current->setAngles( ang );
		current->speed = speed;
	}
	cameraPath_dirty = true;
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::DeletePoint
//---------------------------------------------------------------------------
void CameraManager::DeletePoint( Event*  )
{
	SplinePath *node;

	if ( current )
	{
		node = current->GetNext();
		if ( !node )
		{
			node = current->GetPrev();
		}

		if ( ( SplinePath * )path == ( SplinePath * )current )
		{
			path = current->GetNext();
		}

		delete current;
		current = node;
	}
	cameraPath_dirty = true;
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::MovePlayer
//---------------------------------------------------------------------------
void CameraManager::MovePlayer( Event*  )
{
	Player *player;
	Vector pos;

	player = CameraManager_GetPlayer();
	if ( current && player )
	{
		player->GetPlayerView( &pos, NULL );
		player->setOrigin( current->origin - pos + player->origin );
		player->SetViewAngles( current->angles );
		player->SetFov( current->fov );
	}
}


//---------------------------------------------------------------------------
// CameraManager::NextPoint
//---------------------------------------------------------------------------
void CameraManager::NextPoint( Event*  )
{
	SplinePath *next;

	if ( current )
	{
		next = current->GetNext();
		if ( next )
		{
			current = next;
		}
	}
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::PreviousPoint
//---------------------------------------------------------------------------
void CameraManager::PreviousPoint( Event*  )
{
	SplinePath *prev;

	if ( current )
	{
		prev = current->GetPrev();
		if ( prev )
		{
			current = prev;
		}
	}
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::NextPath
//---------------------------------------------------------------------------
void CameraManager::NextPath( Event*  )
{
	int index;

	//
	// find current path in container of paths
	//
	index = pathList.IndexOfObject( pathName );
	if ( index < pathList.NumObjects() )
		index++;

	if ( index > 0 )
	{
		SetPath( pathList.ObjectAt( index ) );
		UpdateUI();
	}
}


//---------------------------------------------------------------------------
// CameraManager::PreviousPath
//---------------------------------------------------------------------------
void CameraManager::PreviousPath( Event*  )
{
	int index;

	//
	// find current path in container of paths
	//
	index = pathList.IndexOfObject( pathName );
	if ( index > 1 )
		index--;

	if ( index > 0 )
	{
		SetPath( pathList.ObjectAt( index ) );
		UpdateUI();
	}
}


//---------------------------------------------------------------------------
// CameraManager::ShowingPath
//---------------------------------------------------------------------------
void CameraManager::ShowingPath( Event*  )
{
	int        count;
	SplinePath *node;
	SplinePath *prev;
	Vector mins( -8.0f, -8.0f, -8.0f );
	Vector maxs( 8.0f, 8.0f, 8.0f );

	prev = NULL;
	for( node = path; node != NULL; node = node->GetNext() )
	{
		if ( prev )
		{
			G_LineStipple( 4, ( unsigned short )( 0xF0F0F0F0 >> ( 7 - ( level.framenum & 7 ) ) ) );
			G_DebugLine( prev->origin, node->origin, 0.4f, 0.4f, 0.4f, 0.1f );
			G_LineStipple( 1, 0xffff );
		}

		if ( current == node )
		{
			G_DrawDebugNumber( node->origin + Vector( 0.0f, 0.0f, 20.0f ), node->speed, 0.5f, 0.0f, 1.0f, 0.0f, 1 );
			if ( current->GetFov() )
				G_DrawDebugNumber( node->origin + Vector( 0.0f, 0.0f, 30.0f ), node->GetFov(), 0.5f, 0.0f, 0.0f, 1.0f, 0 );

			G_DebugBBox( node->origin, mins, maxs, 1.0f, 1.0f, 0.0f, 1.0f );
		}
		else
		{
			G_DebugBBox( node->origin, mins, maxs, 1.0f, 0.0f, 0.0f, 1.0f );
		}

		//
		// draw watch
		//
		if ( node->doWatch )
		{
			Entity *watchEnt;
			Vector ang;
			Vector delta;
			Vector left;
			Vector up;
			Vector endpoint;

			watchEnt = GetWatchEntity( node->GetWatch() );
			if ( watchEnt )
			{
				delta.x = watchEnt->origin.x;
				delta.y = watchEnt->origin.y;
				delta.z = watchEnt->absmax.z;
				delta -= node->origin;
				delta.normalize();
				ang = delta.toAngles();
				ang.AngleVectors( NULL, &left, &up );

				G_LineWidth( 1.0f );
				endpoint = node->origin + ( delta * 48.0f );
				G_DebugLine( node->origin, endpoint, 0.5, 1, 1, 1 );
				G_DebugLine( endpoint, endpoint + (left * 8) - (delta * 8), 0.5f, 1.0f, 1.0f, 1.0f );
				G_DebugLine( endpoint, endpoint - (left * 8) - (delta * 8), 0.5f, 1.0f, 1.0f, 1.0f );
				G_DebugLine( endpoint, endpoint - (up * 8) - (delta * 8), 0.5f, 1.0f, 1.0f, 1.0f );
				G_DebugLine( endpoint, endpoint + (up * 8) - (delta * 8), 0.5f, 1.0f, 1.0f, 1.0f );
			}
		}

		G_LineWidth( 3.0f );
		G_DebugLine( node->origin, node->origin + ( Vector( node->orientation[ 0 ] ) * 16 ), 1.0f, 0.0f, 0.0f, 1.0f );
		G_DebugLine( node->origin, node->origin + ( Vector( node->orientation[ 1 ] ) * 16 ), 0.0f, 1.0f, 0.0f, 1.0f );
		G_DebugLine( node->origin, node->origin + ( Vector( node->orientation[ 2 ] ) * 16 ), 0.0f, 0.0f, 1.0f, 1.0f );
		G_LineWidth( 1.0f );

		prev = node;
	} // for

	if ( cameraPath_dirty )
	{
		cameraPath_dirty = false;
		cameraPath.Clear();
		cameraPath.SetType( SPLINE_CLAMP );

		node = path;
		while( node != NULL )
		{
			cameraPath.AppendControlPoint( node->origin, node->angles, node->speed );
			node = node->GetNext();

			if ( node == path )
				break;
		}
	}

	// draw the curve itself
	G_Color3f( 1.0f, 1.0f, 0.0f );
	cameraPath.DrawCurve( 10 );

	// draw all the nodes
	for( node = path, count = -1; node != NULL; node = node->GetNext(), count++ )
	{
		Vector dir, angles;

		dir = cameraPath.Eval( ( float )count - 0.9f ) - cameraPath.Eval( count - 1 );
		angles = dir.toAngles();
		if ( node->doWatch || node->GetFov() || ( node->thread != "" ) || ( node->triggertarget != "" ) )
		{
			G_DebugOrientedCircle( cameraPath.Eval( count - 1 ), 5.0f, 0.0f, 1.0f, 1.0f, 1.0f, angles );
		}
		else
		{
			G_DebugOrientedCircle( cameraPath.Eval( count - 1 ), 2.0f, 0.0f, 0.0f, 1.0f, 0.2f, angles );
		}

		// if we are the first node, we need to skip the count so that we properly go to the next node
		if ( count == -1 )
		{
			count = 0;
		}
	}

	// update time console variable
	if( isPreviewPlaybackRunning )
		gi.cvar_set( "cam_playbacktime", va( "%.2f", level.time - playbackStartTime ) );

	PostEvent( EV_CameraManager_ShowingPath, FRAMETIME );
}


//---------------------------------------------------------------------------
// CameraManager::ShowPath
//---------------------------------------------------------------------------
void CameraManager::ShowPath( void )
{
	CancelEventsOfType( EV_CameraManager_ShowingPath );
	PostEvent( EV_CameraManager_ShowingPath, FRAMETIME );
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::ShowPath
//---------------------------------------------------------------------------
void CameraManager::ShowPath( Event* ev )
{
	if ( ev->NumArgs() )
	{
		SetPath( ev->GetString( 1 ) );
	}

	ShowPath();
}


//---------------------------------------------------------------------------
// CameraManager::HidePath
//---------------------------------------------------------------------------
void CameraManager::HidePath( Event*  )
{
	CancelEventsOfType( EV_CameraManager_ShowingPath );
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::StopPlayback
//---------------------------------------------------------------------------
void CameraManager::StopPlayback( Event*  )
{
	isPreviewPlaybackRunning = false;

	if ( cam )
	{
		cam->Stop();
		SetCamera( NULL, 0.0f );
	}
}


//---------------------------------------------------------------------------
// CameraManager::PlayPath
//---------------------------------------------------------------------------
void CameraManager::PlayPath( Event* ev )
{
	if ( cam )
	{
		SetCamera( NULL, 0.0f );
	}

	if ( ev->NumArgs() )
	{
		SetPath( ev->GetString( 1 ) );
	}

	if ( path )
	{
		if ( !cam )
		{
			cam = new Camera;
			cam->SetTargetName( "_loadedcamera" );
			cam->ProcessPendingEvents();
		}

		isPreviewPlaybackRunning = true;
		playbackStartTime = level.time;

		cam->Reset( path->origin, path->angles );
		cam->FollowPath( path, false, NULL );
		cam->Cut( NULL );
		SetCamera( cam, 0.0f );
	}
}


//---------------------------------------------------------------------------
// CameraManager::PlayKFCPath
//---------------------------------------------------------------------------
void CameraManager::PlayKFCPath( Event* ev )
{
	cam = new Camera;
	cam->SetTargetName( "_loadedcamera" );
	cam->ProcessPendingEvents();

	str kfc_filename = ev->GetString( 1 );
	Event* camEv = new Event( EV_Camera_LoadKFC );
	camEv->AddString( kfc_filename );
	cam->ProcessEvent( camEv );

	/// Check if optional playback offset parameters were included
	if( ev->NumArgs() >= 3 )
	{
		float yawOffsetDegrees = ev->GetFloat( 2 );
		Vector originOffset = ev->GetVector( 3 );
		cam->SetPlaybackOffsets( yawOffsetDegrees, originOffset );
	}

	cam->Cut( NULL );
	SetCamera( cam, 0.0f );

	/// Post a future event to the Camera Manager to destroy this camera
	Event* camKillEv = new Event( EV_CameraManager_DestroyKFCPath );
	camKillEv->AddEntity( (Entity*)cam );
	float timeDelaySeconds = cam->GetPathLengthInSeconds();
	PostEvent( camKillEv, timeDelaySeconds );
}


//---------------------------------------------------------------------------
// CameraManager::DestroyKFCPath
//---------------------------------------------------------------------------
void CameraManager::DestroyKFCPath( Event* ev )
{
	Camera* cam;
	cam = (Camera*) ev->GetEntity( 1 );
	delete cam;
	SetCamera( NULL, 0.0f );
}


//---------------------------------------------------------------------------
// CameraManager::LoopPath
//---------------------------------------------------------------------------
void CameraManager::LoopPath( Event* ev )
{
	if ( cam )
	{
		SetCamera( NULL, 0.0f );
	}

	if ( ev->NumArgs() )
	{
		SetPath( ev->GetString( 1 ) );
	}

	if ( path )
	{
		if ( !cam )
		{
			cam = new Camera;
			cam->SetTargetName( "_loadedcamera" );
			cam->ProcessPendingEvents();
		}

		cam->Reset( path->origin, path->angles );
		cam->FollowPath( path, true, NULL );
		cam->Cut( NULL );
		SetCamera( cam, 0.0f );
	}
}


//---------------------------------------------------------------------------
// CameraManager::Watch
//---------------------------------------------------------------------------
void CameraManager::Watch( Event* ev )
{
	if ( current )
	{
		current->SetWatch( ev->GetString( 1 ) );
	}

	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::NoWatch
//---------------------------------------------------------------------------
void CameraManager::NoWatch( Event*  )
{
	if ( current )
	{
		current->NoWatch();
	}

	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::Fov
//---------------------------------------------------------------------------
void CameraManager::Fov( Event* ev )
{
	if ( current )
	{
		current->SetFov( ev->GetFloat( 1 ) );
	}

	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::FadeTime
//---------------------------------------------------------------------------
void CameraManager::FadeTime( Event* ev )
{
	if ( current )
	{
		current->SetFadeTime( ev->GetFloat( 1 ) );
	}

	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::Speed
//---------------------------------------------------------------------------
void CameraManager::Speed( Event* ev )
{
	speed = ev->GetFloat( 1 );
	if ( current )
	{
		current->speed = speed;
	}

	cameraPath_dirty = true;
	UpdateUI();
}


//---------------------------------------------------------------------------
// CameraManager::SavePath
//---------------------------------------------------------------------------
void CameraManager::SavePath( const str& pathName )
{
	SplinePath  *node;
	str         buf;
	str         filename;
	int         num;
	int         index;

	num = 0;
	for( node = path; node != NULL; node = node->GetNext() )
	{
		num++;
	}

	if ( num == 0 )
	{
		warning( "CameraManager::SavePath", "Can't save.  No points in path." );
		return;
	}

	filename = "cams/";
	filename += pathName;
	filename += ".cam";

	path->SetTargetName( pathName );

	gi.Printf( "Saving camera path to '%s'...\n", filename.c_str() );

	buf = "";
	buf += va( "//\n" );
	buf += va( "// Camera Path \"%s\", %d Nodes.\n", pathName.c_str(), num );
	buf += va( "//\n" );

	index = 0;
	for( node = path; node != NULL; node = node->GetNext() )
	{
		//
		// start off the spawn command
		//
		buf += "spawn SplinePath";

		//
		// set the targetname
		//
		if ( !index )
		{
			buf += va( " targetname %s", pathName.c_str() );
		}
		else
		{
			buf += va( " targetname camnode_%s_%d", pathName.c_str(), index );
		}

		//
		// set the target
		//
		if ( index < ( num - 1 ) )
		{
			buf += va( " target camnode_%s_%d", pathName.c_str(), index + 1 );
		}

		//
		// set the trigger target
		//
		if ( node->triggertarget != "" )
		{
			buf += va( " triggertarget %s", node->triggertarget.c_str() );
		}

		//
		// set the thread
		//
		if ( node->thread != "" )
		{
			buf += va( " thread %s", node->thread.c_str() );
		}

		//
		// set the origin
		//
		buf += va( " origin \"%.2f %.2f %.2f\"", node->origin.x, node->origin.y, node->origin.z );

		//
		// set the angles
		//
		buf += va( " angles \"%.1f %.1f %.1f\"", AngleMod( node->angles.x ), AngleMod( node->angles.y ), AngleMod( node->angles.z ) );

		//
		// set the speed
		//
		buf += va( " speed %.1f", node->speed );

		//
		// set the watch
		//
		if ( node->doWatch && ( node->watchEnt != "" ) )
		{
			buf += va( " watch %s", node->watchEnt.c_str() );
		}

		//
		// set the fov
		//
		if ( node->GetFov() )
		{
			buf += va( " fov %.1f", node->GetFov() );
		}

		//
		// set the fadetime
		//
		if ( node->GetFadeTime() )
		{
			buf += va( " fadetime %.1f", node->GetFadeTime() );
		}

		buf += "\n";
		index++;
	}
	buf += "end\n";

	gi.FS_WriteFile( filename.c_str(), buf.c_str(), buf.length() + 1 );
	gi.Printf( "done.\n" );
}


//---------------------------------------------------------------------------
// CameraManager::Save
//---------------------------------------------------------------------------
void CameraManager::Save( Event* ev )
{
	str filename;
	str name;

	if ( ev->NumArgs() != 1 )
	{
		cvar_t * cvar;

		//
		// get the path name from the cvar
		//
		cvar = gi.cvar( "cam_filename", "", 0 );
		if ( cvar->string[ 0 ] )
		{
			name = cvar->string;
		}
		else
		{
			ev->Error( "Usage: cam save [filename]" );
			return;
		}
	}
	else
	{
		name = ev->GetString( 1 );
	}
	SavePath( name );
	pathList.AddUniqueObject( name );
}


//---------------------------------------------------------------------------
// CameraManager::Load
//---------------------------------------------------------------------------
void CameraManager::Load( Event* ev )
{
	qboolean show;
	str		filename;
	str      name;

	if ( ev->NumArgs() != 1 )
	{
		cvar_t * cvar;

		//
		// get the path name from the cvar
		//
		cvar = gi.cvar( "cam_filename", "", 0 );
		if ( cvar->string[ 0 ] )
		{
			show = true;
			name = cvar->string;
		}
		else
		{
			ev->Error( "Usage: cam load [filename]" );
			return;
		}
	}
	else
	{
		show = false;
		name = ev->GetString( 1 );
	}

	if ( pathList.ObjectInList( name ) && show )
	{
		gi.Printf( "Camera path '%s' already loaded...\n", name.c_str() );
		return;
	}

	filename = "cams/";
	filename += name;
	filename += ".cam";

	gi.Printf( "Loading camera path from '%s'...\n", filename.c_str() );

	level.consoleThread->Parse( filename.c_str() );

	pathList.AddUniqueObject( name );
	if ( show )
	{
		Event * ev;

		ev = new Event( EV_CameraManager_SetPath );
		ev->AddString( name );
		PostEvent( ev, 0.0f );
		ShowPath();
	}
}


//---------------------------------------------------------------------------
// CameraManager::SaveMap
//---------------------------------------------------------------------------
void CameraManager::SaveMap( Event* ev )
{
	SplinePath  *node;
	str         buf;
	str         filename;
	int         num;
	int         index;

	if ( ev->NumArgs() != 1 )
	{
		ev->Error( "Usage: cam savemap [filename]" );
		return;
	}

	num = 0;
	for( node = path; node != NULL; node = node->GetNext() )
	{
		num++;
	}

	if ( num == 0 )
	{
		ev->Error( "Can't save.  No points in path." );
		return;
	}

	filename = "cams/";
	filename += ev->GetString( 1 );
	filename += ".map";

	if ( !path->targetname.length() )
	{
		path->SetTargetName( ev->GetString( 1 ) );
		gi.Printf( "Targetname set to '%s'\n", path->targetname.c_str() );
	}

	gi.Printf( "Saving camera path to map '%s'...\n", filename.c_str() );

	buf = "";
	index = 0;
	for( node = path; node != NULL; node = node->GetNext() )
	{
		buf += va( "// pathnode %d\n", index );
		buf += "{\n";
		buf += va( "\"classname\" \"info_splinepath\"\n" );
		if ( index < ( num - 1 ) )
		{
			buf += va( "\"target\" \"camnode_%s_%d\"\n", ev->GetString( 1 ), index + 1 );
		}

		if ( !index )
		{
			buf += va( "\"targetname\" \"%s\"\n", ev->GetString( 1 ) );
		}
		else
		{
			buf += va( "\"targetname\" \"camnode_%s_%d\"\n", ev->GetString( 1 ), index );
		}

		if ( node->triggertarget != "" )
		{
			buf += va( "\"triggertarget\" \"%s\"\n", node->triggertarget.c_str() );
		}

		if ( node->thread != "" )
		{
			buf += va( "\"thread\" \"%s\"\n", node->thread.c_str() );
		}

		buf += va( "\"origin\" \"%d %d %d\"\n", ( int )node->origin.x, ( int )node->origin.y, ( int )node->origin.z );
		buf += va( "\"angles\" \"%d %d %d\"\n", ( int )AngleMod( node->angles.x ), ( int )AngleMod( node->angles.y ), ( int )AngleMod( node->angles.z ) );
		buf += va( "\"speed\" \"%.3f\"\n", node->speed );
		buf += "}\n";
		index++;
	}

	gi.FS_WriteFile( filename.c_str(), buf.c_str(), buf.length() + 1 );
	gi.Printf( "done.\n" );
}

