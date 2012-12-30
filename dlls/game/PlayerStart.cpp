//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/PlayerStart.cpp                           $
// $Revision:: 7                                                              $
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
// Player start location entity declarations
//

#include "_pch_cpp.h"
#include "entity.h"
#include "trigger.h"
#include "PlayerStart.h"

/*****************************************************************************/
/*QUAKED info_player_start (0.75 0.75 0) (-16 -16 0) (16 16 96)

The normal starting point for a level.

"angle" - the direction the player should face
"thread" - the thread that should be called when spawned at this position

******************************************************************************/

Event EV_PlayerStart_SetThread
(
	"thread",
	EV_SCRIPTONLY,
	"s",
	"thread",
	"Set the thread to execute when this player start is used"
);

CLASS_DECLARATION( Entity, PlayerStart, "info_player_start" )
{
	{ &EV_SetAngle,								&PlayerStart::SetAngle },
	{ &EV_PlayerStart_SetThread,				&PlayerStart::SetThread },

	{ NULL, NULL }
};

void PlayerStart::SetAngle( Event *ev )
{
	angles = Vector( 0.0f, ev->GetFloat( 1 ), 0.0f );
}

void PlayerStart::SetThread( Event *ev )
{
	thread = ev->GetString( 1 );
}

str PlayerStart::getThread( void )
{
	return thread;
}

/*****************************************************************************/
/*  saved out by quaked in region mode

******************************************************************************/

CLASS_DECLARATION( PlayerStart, TestPlayerStart, "testplayerstart" )
{
	{ NULL, NULL }
};

/*****************************************************************************/
/*QUAKED info_player_deathmatch (0.75 0.75 1) (-16 -16 0) (16 16 96)

potential spawning position for deathmatch games

"angle" - the direction the player should face
"thread" - the thread that should be called when spawned at this position
"spawnpoint_type" - the named type of this spawnpoint
******************************************************************************/

Event EV_PlayerDeathmatchStart_SetType
(
	"spawnpoint_type",
	EV_DEFAULT,
	"s",
	"spawnpointType",
	"Sets the named type of this spawnpoint"
);

CLASS_DECLARATION( PlayerStart, PlayerDeathmatchStart, "info_player_deathmatch" )
{
	{ &EV_PlayerDeathmatchStart_SetType,					&PlayerDeathmatchStart::SetType },

	{ NULL, NULL }
};

void PlayerDeathmatchStart::SetType( Event *ev )
{
	_type = ev->GetString( 1 );
}

/*****************************************************************************/
/*QUAKED info_player_intermission (0.75 0.75 0) (-16 -16 0) (16 16 96)

viewing point in between deathmatch levels

******************************************************************************/

CLASS_DECLARATION( Camera, PlayerIntermission, "info_player_intermission" )
{
	{ NULL, NULL }
};

PlayerIntermission::PlayerIntermission()
{
	currentstate.watch.watchPath = false;
}
