//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/game.cpp                                      $
// $Revision:: 4                                                              $
//     $Date:: 10/08/02 8:27p                                                 $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#include "_pch_cpp.h"
#include "game.h"

Game game;

CLASS_DECLARATION( Class, Game, NULL )
{
	{ NULL, NULL }
};

void Game::Init( void )
{
	clients = NULL;
	
	autosaved = false;
	
	maxentities = 0;
	maxclients = 0;
}

void Game::Archive( Archiver &arc )
{
	int i;
	
	Class::Archive( arc );
	
	if ( arc.Loading() )
	{
		G_AllocGameData();
	}
	
	for( i = 0; i < maxclients; i++ )
	{
		arc.ArchiveRaw( &clients[ i ], sizeof( gclient_t ) );
	}
	
	arc.ArchiveBoolean( &autosaved );
	
	arc.ArchiveInteger( &maxentities );
	arc.ArchiveInteger( &maxclients );
}
