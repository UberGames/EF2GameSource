//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/gibs.cpp                                  $
// $Revision:: 12                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Gibs - nuff said

#include "_pch_cpp.h"
#include "gibs.h"
#include "decals.h"
#include "mp_manager.hpp"

Event EV_ThrowGib
(
	"throwgib",
	EV_DEFAULT,
	"eff",
	"ent velocity scale",
	"Throw a gib."
);

CLASS_DECLARATION( Entity, Gib, "gib" )
{
	{ &EV_ThrowGib,					&Gib::Throw },
	{ &EV_Touch,					&Gib::Splat },
	{ &EV_Stop,						&Gib::Stop },
	{ &EV_Damage,					&Gib::Damage },

	{ NULL, NULL }
};

Gib::Gib( const str &name, qboolean blood_trail, const str &bloodtrailname, const str &bloodspurtname, 
		const str &bloodsplatname, float bloodsplatsize, float pitch )
{
	animate = new Animate( this );
	
	setSize(Vector(0, 0, 0), Vector(0, 0, 0));
	
	if ( name.length() )
		setModel( name.c_str() );
	
	setMoveType( MOVETYPE_GIB );
	setSolidType( SOLID_BBOX );
	takedamage = DAMAGE_YES;
	sprayed     = false;
	fadesplat   = true;
	scale       = 2.0f;
	
	next_bleed_time = 0;
	
	final_pitch	= pitch;
	
	if ( blood_trail )
	{
		// Make a blood emitter and bind it to the head
		blood = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
		
		if ( bloodtrailname.length() )
			blood->setModel( bloodtrailname.c_str() );
		
		blood->setMoveType( MOVETYPE_BOUNCE );
		blood->setSolidType( SOLID_NOT );
		blood->bind( this );
		
		// Save the blood spurt name
		
		if ( bloodspurtname.length() )
			blood_spurt_name = bloodspurtname;
		
		// Save the blood splat name
		
		if ( bloodsplatname.length() )
			blood_splat_name = bloodsplatname;
		
		blood_splat_size = bloodsplatsize;
	}
	else
	{
		blood = NULL;
	}
	
	Sound( "snd_decap", CHAN_BODY, 1.0f, 300.0f );
}

Gib::~Gib()
{
	if ( blood )
		blood->PostEvent( EV_Remove, 0.0f );
	blood = NULL;
}

Gib::Gib()
{
	animate = new Animate( this );
	
	if ( LoadingSavegame )
	{
		return;
	}
	
	setSize(Vector(0, 0, 0), Vector(0, 0, 0));
	setModel("gib1.def");
	setMoveType( MOVETYPE_GIB );
	setSolidType( SOLID_BBOX );
	sprayed           = 0;
	fadesplat         = true;
	scale             = 2.0f;
}

void Gib::Stop( Event * )
{
	//setSolidType( SOLID_NOT );
	if ( blood )
		blood->PostEvent( EV_Remove, 0.0f );
	blood = NULL;
}

void Gib::Splat( Event * )
{
	if ( multiplayerManager.inMultiplayer() )
		return;
	
	if ( sprayed > 3 )
	{
		//setSolidType(SOLID_NOT);
		return;
	}
	
	sprayed++;
	scale -= 0.2f;
	
	// Stop spinning / force to final pitch
	
	avelocity = vec_zero;
	
	if ( final_pitch != NO_FINAL_PITCH )
	{
		angles[PITCH] = final_pitch;
		setAngles( angles );
	}
	
	SprayBlood( origin );
	Sound( "snd_gibhit" );
}

void Gib::Damage( Event *ev )
{
	Vector direction;
	Entity *blood;
	Vector dir;
	
	if ( next_bleed_time > level.time )
		return;
	
	direction = ev->GetVector ( 5 );
	
	// Spawn a blood spurt
	
	if ( blood_spurt_name.length() > 0 )
	{
		blood = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
		blood->setModel( blood_spurt_name.c_str() );
		
		dir[0] = -direction[0];
		dir[1] = -direction[1];
		dir[2] = -direction[2];
		
		blood->angles = dir.toAngles();
		blood->setAngles( blood->angles );
		
		blood->setOrigin( centroid );
		blood->origin.copyTo( blood->edict->s.origin2 );
		blood->setSolidType( SOLID_NOT );
		
		blood->PostEvent( EV_Remove, 1.0f );
		
		next_bleed_time = level.time + 0.5f;
	}
}

void Gib::SprayBlood( const Vector &start )
{
	Vector      norm;
	trace_t		trace;
	Vector		trace_end;
	
	trace_end = velocity;
	trace_end.normalize();
	trace_end *= 1000.0f;
	trace_end += start;
	
	trace = G_Trace( start, vec_zero, vec_zero, trace_end, this, MASK_SOLID, false, "Gib::SprayBlood" );
	
	if ( HitSky( &level.impact_trace ) || ( !level.impact_trace.ent ) || ( level.impact_trace.ent->solid != SOLID_BSP ) )
	{
		return;
	}
	
	// Do a bloodsplat
	if ( blood_splat_name.length() )
	{
		Decal *decal = new Decal;
		decal->setShader( blood_splat_name );
		decal->setOrigin( Vector( trace.endpos ) + ( Vector( level.impact_trace.plane.normal ) * 0.2f ) );
		decal->setDirection( level.impact_trace.plane.normal );
		decal->setOrientation( "random" );
		decal->setRadius( blood_splat_size + G_Random( blood_splat_size ) );
	}
}

void Gib::ClipGibVelocity( void )
{
	if (velocity[0] < -400.0f)
		velocity[0] = -400.0f;
	else if (velocity[0] > 400.0f)
		velocity[0] = 400.0f;
	if (velocity[1] < -400.0f)
		velocity[1] = -400.0f;
	else if (velocity[1] > 400.0f)
		velocity[1] = 400.0f;
	if (velocity[2] < 200.0f)
		velocity[2] = 200.0f;	// always some upwards
	else if (velocity[2] > 600.0f)
		velocity[2] = 600.0f;
}

void Gib::SetVelocity( float damage )
{
	velocity[0] = 100.0f * crandom();
	velocity[1] = 100.0f * crandom();
	velocity[2] = 200.0f + 100.0f * random();
	
	avelocity = Vector( G_Random( 600.0f ), G_Random( 600.0f ), G_Random( 600.0f ) );
	
	if ( ( damage < -150.0f )  && ( G_Random() > 0.95f ) )
		velocity *= 2.0f;
	else if ( damage < -100.0f )
		velocity *= 1.5f;
	
	ClipGibVelocity();
}

void Gib::Throw( Event *ev )
{
	Entity *ent;
	
	ent = ev->GetEntity(1);
	
	if ( !ent )
		return;
	
	setOrigin(ent->centroid);
	origin.copyTo(edict->s.origin2);
	SetVelocity(ev->GetFloat(2));
	edict->s.scale = ev->GetFloat(3);
	PostEvent( EV_FadeOut, 10.0f + G_Random( 5.0f ) );
}

void CreateGibs( Entity * ent, float damage, float scale, int num, const char * modelname )
{
	Q_UNUSED(ent); Q_UNUSED(damage); Q_UNUSED(scale); Q_UNUSED(num); Q_UNUSED(modelname);
}
