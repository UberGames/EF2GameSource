//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/portal.cpp                                $
// $Revision:: 6                                                              $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Portals - surfaces that are mirrors or cameras

#include "_pch_cpp.h"
#include "portal.h"

/*QUAKED portal_surface (1 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted portal_camera, or a mirror view if untargeted.
*/

Event EV_Portal_LocateCamera
(
	"locatecamera",
	EV_CODEONLY,
	NULL,
	NULL,
	"Locates the camera position."
);

CLASS_DECLARATION( Entity, PortalSurface, "portal_surface" )
{
	{ &EV_Portal_LocateCamera,					&PortalSurface::LocateCamera },

	{ NULL, NULL }
};

void PortalSurface::LocateCamera( Event *ev )
{
	Entity   *owner;
	Entity   *target;
	Vector   dir;
	
	owner = G_FindTarget( NULL, Target() );
	
	if ( !owner )
	{
		// No target, just a mirror
		VectorCopy( edict->s.origin, edict->s.origin2 );
		return;
	}
	
	// frame holds the rotate speed
	if ( owner->spawnflags & 1 )
	{
		edict->s.frame = 25;
	}
	else if ( owner->spawnflags & 2 )
	{
		edict->s.frame = 75;
	}
	
	// skinNum holds the rotate offset
	edict->s.skinNum = owner->edict->s.skinNum;
	
	VectorCopy( owner->origin, edict->s.origin2 );
	
	// see if the portal_camera has a target
	target = G_FindTarget( NULL, owner->Target() );
	
	if ( target )
	{
		dir = target->origin - owner->origin;
		dir.normalize();
		setAngles( dir.toAngles() );
	}
	else
	{
		setAngles( owner->angles );
		dir = owner->orientation[ 0 ];
	}
}

PortalSurface::PortalSurface()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	VectorClear( edict->mins );
	VectorClear( edict->maxs );
	
	gi.linkentity( edict );
	
	edict->svflags = SVF_PORTAL | SVF_SENDPVS;
	edict->s.eType = ET_PORTAL;
	
	PostEvent( EV_Portal_LocateCamera, EV_POSTSPAWN );
}

/*QUAKED portal_camera (1 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate
The target for a portal_surface.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/

Event EV_Portal_Roll
(
	"roll",
	EV_SCRIPTONLY,
	"f",
	"roll",
	"Sets the portal camera's roll."
);

CLASS_DECLARATION( Entity, PortalCamera, "portal_camera" )
{
	{ &EV_Portal_Roll,						&PortalCamera::Roll },

	{ NULL, NULL }
};

void PortalCamera::Roll( Event *ev )
{
	float roll = ev->GetFloat( 1 );
	
	// skinNum holds the roll
	edict->s.skinNum = ( roll / 360.0f ) * 256.0f;
}

PortalCamera::PortalCamera()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	VectorClear( edict->mins );
	VectorClear( edict->maxs );
	
	// No roll on the camera by default
	edict->s.skinNum = 0;
	
	gi.linkentity( edict );
}
