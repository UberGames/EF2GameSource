//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/nature.cpp                                $
// $Revision:: 9                                                              $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:

#include "_pch_cpp.h"
#include "nature.h"

/*****************************************************************************/
/*QUAKED func_emitter (0 0.25 0.5) ?

"emitter" - Name of emitter to use.
******************************************************************************/
Event EV_Emitter_EmitterName
( 
	"emitter",
	EV_DEFAULT,
	"s",
	"name",
	"Emitter to use"
);

CLASS_DECLARATION( Entity, Emitter, "func_emitter" )
{
	{ &EV_Emitter_EmitterName,					&Emitter::EmitterName },

	{ NULL, NULL }
};

Emitter::Emitter()
{
	edict->s.eType = ET_EMITTER;
}

void Emitter::setEmitter( const str &name )
{
	emitterName = name;
	edict->s.tag_num = gi.imageindex( emitterName );
}

void Emitter::EmitterName( Event *ev )
{
	setEmitter( ev->GetString( 1 ) );
}

/*****************************************************************************/
/*QUAKED func_rain (0 0.25 0.5) ?

This creates a raining effect in the brush

"emitter" - Name of emitter to use for the rain.
******************************************************************************/

CLASS_DECLARATION( Entity, Rain, "func_rain" )
{
	{ NULL, NULL }
};


Rain::Rain()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	setSolidType( SOLID_NOT );
	edict->s.eType = ET_RAIN;
	setRainName( "defaultrain" );
}

void Rain::setRainName( const str &name )
{
	rainName = name;
	edict->s.tag_num = gi.imageindex( rainName );
}


/*****************************************************************************/
/*       Plant Puffdaddy                                                     */
/*****************************************************************************/

Event EV_PuffDaddy_Idle
( 
	"idle",
	EV_DEFAULT,
	NULL,
	NULL,
	"Animates the puff daddy."
);

CLASS_DECLARATION( Entity, PuffDaddy, "plant_puffdaddy" )
{
	{ &EV_Touch,								&PuffDaddy::Touch },
	{ &EV_PuffDaddy_Idle,						&PuffDaddy::Idle },

	{ NULL, NULL }
};

void PuffDaddy::Idle( Event *ev )
{
	animate->RandomAnimate( "idle" );
}

void PuffDaddy::Touch( Event *ev )
{
	Entity *other;
	
	other = ev->GetEntity( 1 );
	
	if ( !other->inheritsFrom( "Sentient" ) )
		return;
	
	animate->RandomAnimate( "touch", EV_PuffDaddy_Idle );
	//SetFrame( 0 );
	
	SurfaceCommand( "puffdaddy", "+nodraw" );
	setSolidType( SOLID_NOT );
}

PuffDaddy::PuffDaddy()
{
	animate = new Animate( this );
	
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	setSolidType( SOLID_TRIGGER );
	edict->s.eType = ET_MODELANIM;
	setModel( "plant_puffdaddy.tik" );
	PostEvent( EV_Show, 0.0f );
	//showModel();
}
