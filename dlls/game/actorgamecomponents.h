//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actorgamecomponents.h                          $
// $Revision:: 12                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// What I am trying to do here, is encapsulate any game specific pieces for actor.  Each game will 
// subclass off of the base class, then the Actor class will point to the component class it will
// use.  I am hoping this will make life much easier for mod makers as well
//
//============================
// Forward Declarations
//============================
class ActorGameComponent;
class RedemptionGameComponent;

#ifndef __ACTORGAMECOMPONENTS_H__
#define __ACTORGAMECOMPONENTS_H__

#include "actor.h"

//============================
// Class ActorGameComponent
//============================
//
// Base class from which all Actor Game Components are derived.
//
class ActorGameComponent : public Listener
	{
	public:
		CLASS_PROTOTYPE( ActorGameComponent );

		                  ActorGameComponent()              {}							
		virtual void	   HandleEvent( Event *ev )          {}
		virtual void	   HandleArmorDamage( Event *ev )    {}
		virtual void	   HandleDeath( const Entity *ent )  {}
		virtual void	   HandleThink()                     {}
		
		virtual qboolean  DoCheck( const Conditional &condition ) { return false; }		
		virtual void      DoArchive( Archiver &arc, const Actor *act )           {}

	};

/*
EF Specific Stuff
*/
//============================
// Class EFGameComponent
//============================
class EFGameComponent : public ActorGameComponent
	{
	public:
		CLASS_PROTOTYPE( EFGameComponent );

				EFGameComponent();
				EFGameComponent( const Actor *actor );

		void  HandleEvent( Event *ev );		
		void  HandleArmorDamage( Event *ev );		
		void  HandleDeath( const Entity *ent);
		void  HandleThink();

		qboolean DoCheck( const Conditional &condition );
		void  DoArchive( Archiver &arc , const Actor *act );

	private:
		Actor *act;
	};

#endif /*__ACTORGAMECOMPONENTS_H__*/
