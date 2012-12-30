//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/body.cpp                                  $
// $Revision:: 13                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:35p                                                  $
//
// Copyright (C) 2000 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Dead bodies

#include "_pch_cpp.h"
#include "animate.h"
#include "gibs.h"
#include "body.h"

CLASS_DECLARATION( Entity, Body, NULL )
{
	{ &EV_Damage,    &Body::Damage },

	{ NULL,          NULL }
};

//=============================================================
//Body::Body
//=============================================================
Body::Body()
{
	Event *newEvent;
	
	takedamage         = DAMAGE_YES;
	edict->s.eType     = ET_MODELANIM;
	health             = 10;
	edict->clipmask    = MASK_DEADSOLID;
	edict->svflags    |= SVF_DEADMONSTER;
	
	setSolidType( SOLID_NOT );
	//setSolidType( SOLID_BBOX );
	//setContents( CONTENTS_CORPSE );
	setMoveType( MOVETYPE_NONE );
	
	//PostEvent( EV_FadeOut, 6.0f );
	
	newEvent = new Event( EV_DisplayEffect );
	newEvent->AddString( "TransportOut" );
	newEvent->AddString( "Multiplayer" );
	PostEvent( newEvent, 0.0f );
	
	PostEvent( EV_Remove, 5.0f );
	
	//Event *transport_event = new Event( EV_DisplayEffect );
	//transport_event->AddString( "transport_out" );
	//ProcessEvent( transport_event );
	
	animate = new Animate( this );
}

//=============================================================
//Body::Damage
//=============================================================
void Body::Damage( Event *ev )
{
	/* str gib_name;
	int number_of_gibs;
	float scale;
	Entity *ent;
	str real_gib_name;

	if ( !com_blood->integer )
		return;

	gib_name       = "fx_rgib";
	number_of_gibs = 5;
	scale          = 1.2;

	// Spawn the gibs
	real_gib_name = gib_name;
	real_gib_name += number_of_gibs;
	real_gib_name += ".tik";

	ent = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
	ent->setModel( real_gib_name.c_str() );
	ent->setScale( scale );
	ent->setOrigin( centroid );
	ent->animate->RandomAnimate( "idle" );
	ent->PostEvent( EV_Remove, 1.0f );

	Sound( "snd_decap", CHAN_BODY, 1.0f, 300.0f );

	this->hideModel();
	this->takedamage = DAMAGE_NO; */
}
