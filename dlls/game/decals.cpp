//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/decals.cpp                                    $
// $Revision:: 7                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 3/19/03 6:10p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Decal entities

#include "_pch_cpp.h"
#include "decals.h"


CLASS_DECLARATION( Entity, Decal, NULL )
{
	{ NULL, NULL }
};

Decal::Decal()
{
	edict->s.eType = ET_DECAL;
	edict->s.modelindex  = 1;	      // must be non-zero

	if ( !LoadingSavegame )
	{
		PostEvent( EV_Remove, FRAMETIME );
	}
}

void Decal::setDirection( const Vector &dir )
{
	edict->s.surfaces[0] = DirToByte( ( Vector )dir );
}

void Decal::setShader( const str &decal_shader )
{
	str temp_shader;
	
	shader = decal_shader;
	edict->s.tag_num = gi.imageindex( shader.c_str() );
	
	temp_shader = shader + ".spr";
	CacheResource( temp_shader, this );
}

void Decal::setOrientation( const str &deg )
{
	Vector ang;
	
	if ( !deg.icmp( "random" ) )
		ang[2] = random() * 360.0f;
	else
		ang[2] = (float)atof( deg );
	
	setAngles( ang );
}

void Decal::setRadius( float rad )
{
	edict->s.scale = rad;
}
