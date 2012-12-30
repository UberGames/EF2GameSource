//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/specialfx.cpp                             $
// $Revision:: 6                                                              $
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
// Special fx
//

#include "_pch_cpp.h"
//#include "g_local.h"
#include "specialfx.h"

/*****************************************************************************/
/*QUAKED func_fulcrum (0 0 1) ? X_AXIS_ONLY Y_AXIS_ONLY

This creates a fulcrum that when you stand on it, it will rotate due to
the weight exerted it will start rotating, when not standing on it, it
will return to its rest position.
"speed" - set the speed at which the fulcrum will operate (default is 48)
"resetspeed" - speed at whcih fulcrum resets, (default speed * 0.002)
"dampening" - dampen constant (default 0.95)
"limit" - limit the movement of the fulcrum (default 90 degrees)
"movesound" - sound to be played while fulcrum is moving

X_AXIS_ONLY - only adjust the X axis
Y_AXIS_ONLY - only adjust the Y axis

******************************************************************************/

#define X_AXIS_ONLY ( 1 << 0 )
#define Y_AXIS_ONLY ( 1 << 1 )

Event EV_Fulcrum_SetSpeed
(
	"speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Speed at which fulcrum operates itself."
);
Event EV_Fulcrum_Reset
(
	"reset",
	EV_CONSOLE,
	NULL,
	NULL,
	"Reset the fulcrum right now."
);
Event EV_Fulcrum_AdjustFulcrum
(
	"_adjust_fulcrum",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called periodically to adjust the frustum every frame and update its angular velocity."
);
Event EV_Fulcrum_SetResetSpeed
(
	"resetspeed",
	EV_SCRIPTONLY,
	"f",
	"newResetspeed",
	"Speed at which fulcrum resets itself, defaults to 0.002 * speed."
);
Event EV_Fulcrum_SetDampening
(
	"dampening",
	EV_SCRIPTONLY,
	"f",
	"newDampening",
	"dampening of fulcrum."
);
Event EV_Fulcrum_SetLimit
(
	"limit",
	EV_SCRIPTONLY,
	"f",
	"newLimit",
	"angular limit for the fulcrum."
);
Event EV_Fulcrum_Setup
(
	"_setup",
	EV_CODEONLY,
	NULL,
	NULL,
	"setup the fulcrum for the first time."
);
Event EV_Fulcrum_SetMoveSound
(
	"movesound",
	EV_SCRIPTONLY,
	"s",
	"newSinkSound",
	"Sound played when fulcrum is moving."
);

CLASS_DECLARATION( ScriptSlave, Fulcrum, "func_fulcrum" )
{
	{ &EV_Fulcrum_SetSpeed,					&Fulcrum::SetSpeed },
	{ &EV_Fulcrum_Reset,					&Fulcrum::Reset },
	{ &EV_Fulcrum_AdjustFulcrum,			&Fulcrum::Adjust },
	{ &EV_Touch,							&Fulcrum::Touched },
	{ &EV_Fulcrum_SetResetSpeed,			&Fulcrum::SetResetSpeed },
	{ &EV_Fulcrum_SetDampening,				&Fulcrum::SetDampening },
	{ &EV_Fulcrum_SetLimit,					&Fulcrum::SetLimit },
	{ &EV_Fulcrum_Setup,					&Fulcrum::Setup },
	{ &EV_Fulcrum_SetMoveSound,				&Fulcrum::SetMoveSound },

	{ NULL, NULL }
};

Fulcrum::Fulcrum()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	touched = false;
	speed = 48;
	resetspeed = speed * 0.002f;
	dampening = 0.95f;
	limit = 90;
	setMoveType( MOVETYPE_PUSH );
	PostEvent( EV_Fulcrum_Setup, FRAMETIME );
}

void Fulcrum::Setup( Event *ev )
{
	startangles = angles;
}

void Fulcrum::SetSpeed( Event *ev )
{
	speed = ev->GetFloat( 1 );
	resetspeed = speed * 0.002f;
}

void Fulcrum::SetResetSpeed( Event *ev )
{
	resetspeed = ev->GetFloat( 1 );
}

void Fulcrum::SetDampening( Event *ev )
{
	dampening = ev->GetFloat( 1 );
}

void Fulcrum::SetLimit( Event *ev )
{
	limit = ev->GetFloat( 1 );
}

void Fulcrum::SetMoveSound( Event *ev )
{
	movesound = ev->GetString( 1 );
}

void Fulcrum::Reset( Event *ev )
{
	StopLoopSound();
	touched = false;
	avelocity = vec_zero;
	setAngles( startangles );
	CancelEventsOfType( EV_Fulcrum_AdjustFulcrum );
}

void Fulcrum::Touched( Event *ev )
{
	Vector   diff;
	vec3_t   dest;
	Vector   delta;
	Entity * other;
	
	other = ev->GetEntity( 1 );
	
	assert( other );
	if ( !other )
		return;
	
	//
	// only things resting on me will affect me
	//
	if ( other->groundentity != this->edict )
		return;
	
	if ( movesound.length() )
	{
		LoopSound( movesound );
	}
	
	delta = getLocalVector( other->origin - origin );
	
	//
	// clear out angular velocity
	//
	avelocity = vec_zero;
	
	// only look at x and y since z doesn't really concern us.
	if ( !( spawnflags & X_AXIS_ONLY ) )
	{
		avelocity[ PITCH ] += (float)cos( DEG2RAD( startangles[ YAW ] ) ) * speed * delta[ 0 ] / maxs[ 0 ];
		avelocity[ ROLL ] += (float)sin( DEG2RAD( startangles[ YAW ] ) ) * speed * delta[ 0 ] / maxs[ 0 ];
	}
	if ( !( spawnflags & Y_AXIS_ONLY ) )
	{
		avelocity[ ROLL ] += (float)cos( DEG2RAD( startangles[ YAW ] ) ) * -speed * delta[ 1 ] / maxs[ 1 ];
		avelocity[ PITCH ] += (float)sin( DEG2RAD( startangles[ YAW ] ) ) * -speed * delta[ 1 ] / maxs[ 1 ];
	}
	
	AnglesSubtract( startangles, angles, dest );
	diff = Vector( dest );
	if ( fabs( diff[ PITCH ] ) >= limit )
	{
		Vector newAngles;
		StopLoopSound();
		if ( diff[ PITCH ] > 0.0f )
		{
			if ( avelocity[ PITCH ] < 0.0f )
            {
				avelocity[ PITCH ] = 0.0f;
            }
		}
		else if ( diff[ PITCH ] < 0.0f )
		{
			if ( avelocity[ PITCH ] > 0.0f )
            {
				avelocity[ PITCH ] = 0.0f;
            }
		}
	}
	
	if ( fabs( diff[ ROLL ] ) >= limit )
	{
		StopLoopSound();
		if ( diff[ ROLL ] > 0.0f )
		{
			if ( avelocity[ ROLL ] < 0.0f )
            {
				avelocity[ ROLL ] = 0.0f;
            }
		}
		else if ( diff[ ROLL ] < 0.0f )
		{
			if ( avelocity[ ROLL ] > 0.0f )
            {
				avelocity[ ROLL ] = 0.0f;
            }
		}
	}
	
	touched = true;
	CancelEventsOfType( EV_Fulcrum_AdjustFulcrum );
	PostEvent( EV_Fulcrum_AdjustFulcrum, 0.0f );
}

void Fulcrum::Adjust( Event *ev )
{
	if ( !touched )
	{
		int      i;
		float    f;
		Vector   diff;
		vec3_t   dest;
		qboolean post;
		
		if ( movesound.length() )
		{
			LoopSound( movesound );
		}
		AnglesSubtract( startangles, angles, dest );
		diff = Vector( dest );
		post = false;
		
		for( i = 0; i < 3; i++ )
		{
			if ( diff[ i ] )
            {
				avelocity[ i ] += resetspeed * diff[ i ];
            }
			avelocity[ i ] *= dampening;
			f = fabs( avelocity[ i ] );
			if ( f > 0.01f )
            {
				post = true;
            }
		}
		if ( !post )
		{
			Reset( NULL );
			return;
		}
	}
	else
	{
		touched = false;
	}
	
	PostEvent( EV_Fulcrum_AdjustFulcrum, FRAMETIME );
}

/*****************************************************************************/
/*QUAKED func_runthrough (0 0 1) ?

This is a trigger field that the player can run through and spawn tiki models
at that position.  Used for releasing chaff from grass or butterflys from
flower gardens
"speed" - speed at which you have to be moving to trigger ( default 100 )
"delay" - time between triggering ( default 0.1 )
"chance" - chance that the trigger will spawn something( default 0.5 )
"lip" - how far below the surface of the trigger we should spawn these things ( default 3 )
"offset" - vector offset oriented along velocity vector( default "0 0 0" )
"spawnmodel" - thing to spawn when triggered

******************************************************************************/


Event EV_RunThrough_SetSpeed
(
	"speed",
	EV_DEFAULT,
	"f",
	"speed",
	"threshold speed at which RunThrough is activated."
);
Event EV_RunThrough_SetDelay
(
	"delay",
	EV_DEFAULT,
	"f",
	"delay",
	"time between RunThrough being activated."
);
Event EV_RunThrough_SetChance
(
	"chance",
	EV_DEFAULT,
	"f",
	"chance",
	"chance that trigger will spawn something."
);
Event EV_RunThrough_SetLip
(
	"lip",
	EV_DEFAULT,
	"f",
	"lip",
	"distance below trigger we should spawn things."
);
Event EV_RunThrough_SetSpawnModel
(
	"spawnmodel",
	EV_DEFAULT,
	"s",
	"model_to_spawn",
	"When triggered, what to spawn."
);
Event EV_RunThrough_SetOffset
(
	"offset",
	EV_DEFAULT,
	"v",
	"spawn_offset",
	"When triggered, what to offset the spawned object by."
);

CLASS_DECLARATION( Entity, RunThrough, "func_runthrough" )
{
	{ &EV_RunThrough_SetSpeed,					&RunThrough::SetSpeed },
	{ &EV_RunThrough_SetDelay,					&RunThrough::SetDelay },
	{ &EV_RunThrough_SetChance,					&RunThrough::SetChance },
	{ &EV_RunThrough_SetLip,					&RunThrough::SetLip },
	{ &EV_RunThrough_SetSpawnModel,				&RunThrough::SetSpawnModel },
	{ &EV_RunThrough_SetOffset,					&RunThrough::SetOffset },
	{ &EV_Touch,								&RunThrough::Touched },

	{ NULL, NULL }
};

RunThrough::RunThrough()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	offset = vec_zero;
	speed = 100.0f;
	chance = 0.5f;
	delay = 0.1f;
	lip = 3.0f;
	spawnmodel = "";
	lasttriggertime = 0;
	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_TRIGGER );
}

void RunThrough::SetSpeed( Event *ev )
{
	speed = ev->GetFloat( 1 );
}

void RunThrough::SetChance( Event *ev )
{
	chance = ev->GetFloat( 1 );
}

void RunThrough::SetDelay( Event *ev )
{
	delay = ev->GetFloat( 1 );
}

void RunThrough::SetLip( Event *ev )
{
	lip = ev->GetFloat( 1 );
}

void RunThrough::SetOffset( Event *ev )
{
	offset = ev->GetVector( 1 );
}

void RunThrough::SetSpawnModel( Event *ev )
{
	spawnmodel = ev->GetString( 1 );
	CacheResource( spawnmodel, this );
}

void RunThrough::Touched( Event *ev )
{
	Vector   forward, left, up;
	Vector   ang;
	Vector   org;
	Entity * other;
	
	other = ev->GetEntity( 1 );
	
	assert( other );
	if ( !other )
		return;
	
	// don't trigger on the world
	if ( other == world )
		return;
	
	// don't re-trigger too soon
	if ( level.time < lasttriggertime )
		return;
	
	// don't trigger if not moving fast enough
	if ( other->velocity.length() < speed )
		return;
	
	// don't trigger if chance says not to.
	if ( G_Random( 1.0f ) > chance )
		return;
	
	ang = other->velocity.toAngles();
	ang.AngleVectors( &forward, &left, &up );
	
	// get the origin from the thing that triggered me
	org = other->origin;
	
	// set the height of the origin based on the bounds of this trigger minus the lip
	org.z = absmax.z - lip;
	
	org += offset.x * forward;
	org += offset.y * left;
	org += offset.z * up;
	
	if ( spawnmodel.length() )
	{
		Entity * ent;
		// purposely declared here so that we don't do it every frame
		SpawnArgs args;
		
		args.setArg( "origin", va( "%f %f %f", org[ 0 ], org[ 1 ], org[ 2 ] ) );
		args.setArg( "angles", va( "%f %f %f", ang[ 0 ], ang[ 1 ], ang[ 2 ] ) );
		args.setArg( "model", spawnmodel.c_str() );
		
		ent = args.Spawn();
		if ( ent )
		{
			ent->ProcessPendingEvents();
		}
	}
	// set trigger time for next time
	lasttriggertime = level.time + delay;
}

/*****************************************************************************/
/*QUAKED func_sinkobject (0 0 1) ? x FALLAWAY NO_RESET

This creates an object which gradually sinks downward when stepped on.
"delay" - delay between when object starts reacting towards weight (default 0 seconds)
"speed" - set the speed at which sinkobject sinks (default is 50)
"resetspeed" - speed at which sinkobject resets its position, (default speed * 0.1)
"dampening" - dampening constant to mitigate acceleration (default 0.95)
"limit" - limit the movement of the sinkobject how far down it should go (default 1000 units)
"resetdelay" - time between player gets off platform, and platform starts resetting itself.
"sinksound" - sound to be played while platform is sinking.
"resetsound" - sound to be played while platform is resetting.
"active" - make the sink object active
"notactive" - make the sink object not active

FALLAWAY - the sink object will progressively fall down faster and faster
NO_RESET - the sink object will not reset, only move downward

******************************************************************************/

#define FALLAWAY ( 1 << 1 )
#define NO_RESET ( 1 << 2 )

Event EV_SinkObject_SetSpeed
(
	"speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Speed at which SinkObject starts falling."
);
Event EV_SinkObject_SetDelay
(
	"delay",
	EV_SCRIPTONLY,
	"f",
	"delay",
	"Delay until SinkObject starts falling."
);
Event EV_SinkObject_Reset
(
	"reset",
	EV_CONSOLE,
	NULL,
	NULL,
	"Reset the SinkObject right now."
);
Event EV_SinkObject_AdjustSinkObject
(
	"_adjust_SinkObject",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called periodically to adjust the sinkobject every frame and adjust its velocity."
);
Event EV_SinkObject_Fall
(
	"_fall_SinkObject",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called periodically to make a sink object fall away."
);
Event EV_SinkObject_SetResetSpeed
(
	"resetspeed",
	EV_SCRIPTONLY,
	"f",
	"newResetspeed",
	"Speed at which SinkObject resets itself, defaults to 0.002 * speed."
);
Event EV_SinkObject_SetResetDelay
(
	"resetdelay",
	EV_SCRIPTONLY,
	"f",
	"newResetDelay",
	"Delay between when sinkobject starts resetting."
);
Event EV_SinkObject_SetSinkSound
(
	"sinksound",
	EV_SCRIPTONLY,
	"s",
	"newSinkSound",
	"Sound played when sinkobject is sinking."
);
Event EV_SinkObject_SetResetSound
(
	"resetsound",
	EV_SCRIPTONLY,
	"s",
	"newResetSound",
	"Sound played when sinkobject is resetting."
);
Event EV_SinkObject_SetDampening
(
	"dampening",
	EV_SCRIPTONLY,
	"f",
	"newDampening",
	"dampening of SinkObject."
);
Event EV_SinkObject_SetLimit
(
	"limit",
	EV_SCRIPTONLY,
	"f",
	"newLimit",
	"maximum displacement of the SinkObject."
);
Event EV_SinkObject_Setup
(
	"_setup",
	EV_CODEONLY,
	NULL,
	NULL,
	"setup the SinkObject for the first time."
);
Event EV_SinkObject_MakeActive
(
	"active",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"make the SinkObject active, so that it will respond to players touching it."
);
Event EV_SinkObject_MakeNonActive
(
	"notactive",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"make the SinkObject not active, so that it won't respond to players touching it."
);

CLASS_DECLARATION( ScriptSlave, SinkObject, "func_sinkobject" )
{
	{ &EV_SinkObject_SetSpeed,						&SinkObject::SetSpeed },
	{ &EV_SinkObject_SetDelay,						&SinkObject::SetDelay },
	{ &EV_SinkObject_Reset,							&SinkObject::Reset },
	{ &EV_SinkObject_AdjustSinkObject,				&SinkObject::Adjust },
	{ &EV_SinkObject_Fall,							&SinkObject::Fall },
	{ &EV_Touch,									&SinkObject::Touched },
	{ &EV_SinkObject_SetResetSpeed,					&SinkObject::SetResetSpeed },
	{ &EV_SinkObject_SetDampening,					&SinkObject::SetDampening },
	{ &EV_SinkObject_SetLimit,						&SinkObject::SetLimit },
	{ &EV_SinkObject_Setup,							&SinkObject::Setup },
	{ &EV_SinkObject_SetResetDelay,					&SinkObject::SetResetDelay },
	{ &EV_SinkObject_SetResetSound,					&SinkObject::SetResetSound },
	{ &EV_SinkObject_SetSinkSound,					&SinkObject::SetSinkSound },
	{ &EV_SinkObject_MakeActive,					&SinkObject::MakeActive },
	{ &EV_SinkObject_MakeNonActive,					&SinkObject::MakeNonActive },

	{ NULL, NULL }
};

#define RESET_SCALE 0.1f
#define SPEED_DIVISOR ( 1.0f / 250.0f )

SinkObject::SinkObject()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	active = true;
	sinksound = "";
	resetsound = "";
	resetdelay = 0;
	delay = 0;
	touched = false;
	speed = 50.0f * SPEED_DIVISOR;
	resetspeed = speed * RESET_SCALE;
	dampening = 0.95f;
	limit = 1000;
	time_touched = -1;
	time_reset = -1;
	setMoveType( MOVETYPE_PUSH );
	PostEvent( EV_SinkObject_Setup, FRAMETIME );
}

void SinkObject::Setup( Event *ev )
{
	startpos = origin;
}

void SinkObject::SetSpeed( Event *ev )
{
	speed = ev->GetFloat( 1 ) * SPEED_DIVISOR;
	resetspeed = speed * RESET_SCALE;
}

void SinkObject::SetDelay( Event *ev )
{
	delay = ev->GetFloat( 1 );
}

void SinkObject::SetResetSpeed( Event *ev )
{
	resetspeed = ev->GetFloat( 1 ) * SPEED_DIVISOR;
}

void SinkObject::SetResetDelay( Event *ev )
{
	resetdelay = ev->GetFloat( 1 );
}

void SinkObject::SetSinkSound( Event *ev )
{
	sinksound = ev->GetString( 1 );
}

void SinkObject::SetResetSound( Event *ev )
{
	resetsound = ev->GetString( 1 );
}

void SinkObject::SetDampening( Event *ev )
{
	dampening = ev->GetFloat( 1 );
}

void SinkObject::SetLimit( Event *ev )
{
	limit = ev->GetFloat( 1 );
}

void SinkObject::Reset( Event *ev )
{
	time_reset = -1;
	time_touched = -1;
	touched = false;
	velocity = vec_zero;
	setOrigin( startpos );
	CancelEventsOfType( EV_SinkObject_AdjustSinkObject );
	StopLoopSound();
}

void SinkObject::Touched( Event *ev )
{
	Entity * other;
	
	// if we aren't active, don't let anyone touch us
	if ( !active )
		return;
	
	other = ev->GetEntity( 1 );
	
	assert( other );
	if ( !other )
		return;
	
	//
	// only things resting on me will affect me
	//
	if ( other->groundentity != this->edict )
		return;
	
	if ( delay )
	{
		if ( time_touched == -1.0f )
			time_touched = level.time + delay;
		//
		// not time yet
		//
		if ( level.time < time_touched )
			return;
	}
	if ( sinksound.length() )
	{
		LoopSound( sinksound );
	}
	
	velocity.z -= speed;
	
	if ( origin.z < ( startpos.z - limit ) )
	{
		origin.z = startpos.z - limit;
		setOrigin( origin );
		velocity = vec_zero;
	}
	
	touched = true;
	
	if ( spawnflags & FALLAWAY )
	{
		CancelEventsOfType( EV_SinkObject_Fall );
		PostEvent( EV_SinkObject_Fall, FRAMETIME );
		return;
	}
	
	CancelEventsOfType( EV_SinkObject_AdjustSinkObject );
	PostEvent( EV_SinkObject_AdjustSinkObject, FRAMETIME );
}

void SinkObject::Fall( Event *ev )
{
	velocity.z -= speed;
	
	if ( origin.z < ( startpos.z - limit ) )
	{
		origin.z = startpos.z - limit;
		setOrigin( origin );
		velocity = vec_zero;
		StopLoopSound();
	}
	else
	{
		CancelEventsOfType( EV_SinkObject_Fall );
		PostEvent( EV_SinkObject_Fall, FRAMETIME );
	}
}

void SinkObject::Adjust( Event *ev )
{
	if ( !touched )
	{
		float    diff;
		
		if ( spawnflags & NO_RESET )
		{
			StopLoopSound();
			time_touched = -1;
			velocity.z = 0;
			return;
		}
		else
		{
			if ( resetdelay )
            {
				// stop the object
				velocity.z = 0;
				// kill its sound
				StopLoopSound();
				if ( time_reset == -1.0f )
					time_reset = level.time + resetdelay;
				//
				// not time yet
				//
				if ( level.time < time_reset )
				{
					PostEvent( EV_SinkObject_AdjustSinkObject, FRAMETIME );
					return;
				}
            }
			if ( resetsound.length() )
            {
				LoopSound( resetsound );
            }
			diff = startpos.z - origin.z;
			
			velocity.z += resetspeed * diff;
			velocity.z *= dampening;
			if ( ( fabs( diff ) < 0.5f ) && ( fabs( velocity.z ) < 1.0f ) )
            {
				Reset( NULL );
				return;
            }
		}
	}
	else
	{
		touched = false;
	}
	
	PostEvent( EV_SinkObject_AdjustSinkObject, FRAMETIME );
}

void SinkObject::MakeActive( Event *ev )
{
	active = true;
}

void SinkObject::MakeNonActive( Event *ev )
{
	active = false;
}
