//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actorgamecomponents.cpp                        $
// $Revision:: 18                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/11/02 3:34a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// 
//
// DESCRIPTION:
// What I am trying to do here, is encapsulate any game specific pieces for actor.  Each game will 
// subclass off of the base class, then the Actor class will point to the component class it will
// use.  I am hoping this will make life much easier for mod makers as well
//

#include "_pch_cpp.h"
#include "actorgamecomponents.h"
#include "player.h"


//
// Name:       CLASS_DECLARATION
// Parameters: Listener -- Parent Class
//             ActorGameComponent -- This class
//             String -- Name
// Description: Macro
//
CLASS_DECLARATION( Listener , ActorGameComponent, "actor_game_component" )
{
	{ NULL, NULL }
};



//=========================================
// EFGameComponent Implementation
//=========================================

//
// Name:       CLASS_DECLARATION
// Parameters: ActorGameComponent -- Parent Class
//             EFGameComponent -- This class
//             String -- Name
// Description: Macro
//
CLASS_DECLARATION( ActorGameComponent, EFGameComponent, "ef_game_component" )
{
	{ NULL, NULL }
};


//
// Name:        EFGameComponent()
// Parameters:  None
// Description: Constructor
//
EFGameComponent::EFGameComponent()
{
	// Should always use other constructor
	gi.Error( ERR_FATAL, "EFGameComponent::EFGameComponent -- Default Constructor Called" );
}

//
// Name:        EFGameComponent()
// Parameters:  Actor *actor
// Description: Constructor
//
EFGameComponent::EFGameComponent( const Actor *actor )
{
	if ( actor )
		act = (Actor *)actor;
}

void EFGameComponent::DoArchive( Archiver &arc , const Actor *actor )
{
	if ( actor )
		act = (Actor *)actor;
	else
		gi.Error( ERR_FATAL, "EFGameComponnet::DoArchive -- actor is NULL" );
	
}
//
// Name:        HandleDeath()
// Parameters:  Entity *ent
// Description: Custom Game Death Handler
//
void EFGameComponent::HandleDeath( const Entity *ent )
{
	
}


//
// Name:        HandleArmorDamage()
// Parameters:  Event *ev
// Description: Custom Game Armor Damage Handler
//
void EFGameComponent::HandleArmorDamage( Event *ev )
{
	
	
}

//
// Name:        HandleThink()
// Parameters:  None
// Description: Custom Game Think Handler
//
void EFGameComponent::HandleThink()
{
	
	
}

//
// Name:        HandleEvent()
// Parameters:  Event *ev
// Description: Event Handler
//
void EFGameComponent::HandleEvent( Event *ev )
{
	Event *new_event;
	new_event = new Event( ev );
	ProcessEvent(new_event);
}


//
// Name:       DoCheck()
// Parameters: Conditional &condition
// Description: Custom Game conditional check handler
//
qboolean EFGameComponent::DoCheck ( const Conditional &condition )
{
	return false;
}

