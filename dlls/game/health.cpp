//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/health.cpp                                $
// $Revision:: 21                                                             $
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
// Health powerup
//

#include "_pch_cpp.h"
#include "item.h"
#include "inventoryitem.h"
#include "sentient.h"
#include "health.h"
#include "weaputils.h"
#include "player.h"
#include "mp_manager.hpp"
#include <qcommon/gameplaymanager.h>

CLASS_DECLARATION( InventoryItem, HealthInventoryItem, "" )
{
	{ &EV_InventoryItem_Use,							&HealthInventoryItem::Use },
	{ &EV_ProcessGameplayData,							&HealthInventoryItem::processGameplayData },

	{ NULL, NULL }
};


void HealthInventoryItem::Use( Event *ev )
{
	Entity      *other;
	Sentient    *sen;
	//Event			*event;
	str			sound_name;
	
	
	other = ev->GetEntity( 1 );
	if ( !other || !other->isSubclassOf( Sentient ) )
	{
		return;
	}
	
	sen = ( Sentient * )other;
	
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( gpm->hasObject(getArchetype()) )
	{
		if ( gpm->hasFormula("HealthPotion") )
		{
			GameplayFormulaData fd(this);
			amount = gpm->calculate("HealthPotion", fd, sen->max_health);
		}
		str snd = gpm->getStringValue(getArchetype() + ".Use", "wav");
		if ( snd.length() )
		{
			int channel = CHAN_BODY;
			float volume = -1.0f;
			float mindist = -1.0f;
			if ( gpm->hasProperty(getArchetype() + ".Use","channel") )
				channel = (int)gpm->getFloatValue(getArchetype() + ".Use", "channel");
			if ( gpm->hasProperty(getArchetype() + ".Use","volume") )
				volume = (int)gpm->getFloatValue(getArchetype() + ".Use", "volume");
			if ( gpm->hasProperty(getArchetype() + ".Use","mindist") )
				mindist = (int)gpm->getFloatValue(getArchetype() + ".Use", "mindist");
			Sound(snd, channel, volume, mindist);
		}
	}
	
	sen->health += (float)amount;
	
	if ( sen->health > sen->max_health )
	{
		sen->health = sen->max_health;
	}
	
	// If we are on fire stop it
	
	sen->ProcessEvent( EV_Sentient_StopOnFire );
	
	// Spawn special effect around sentient
	
	// Uncomment if we need it
	/*sound_name = GetRandomAlias( "snd_use" );
	
	if ( sound_name )
	other->Sound( sound_name.c_str(), 0 );
	  
	event = new Event( EV_AttachModel );
	event->AddString( "models/fx_tikifx2.tik" );
	event->AddString( "Bip01 Spine" );
	// set scale
	event->AddFloat( 1.0f );
	// set targetname
	event->AddString( "regen" );
	// set detach_at_death
	event->AddInteger( 1 );
	// set remove_time
	event->AddFloat( 2.0f );
	other->ProcessEvent( event );*/
	
	PostEvent( EV_Remove, 0.0f );
}

CLASS_DECLARATION( Item, Health, "health_020" )
{
	{ &EV_Item_Pickup,      				&Health::PickupHealth },

	{ NULL, NULL }
};

Health::Health()
{
	if ( multiplayerManager.checkFlag( MP_FLAG_NO_HEALTH ) )
	{
		PostEvent( EV_Remove, EV_REMOVE );
		return;
	}
	
	setAmount( 20 );
	
	item_name = "health";
}

void Health::PickupHealth( Event *ev )
{
	Sentient *sen;
	Entity *other;
	
	other = ev->GetEntity( 1 );
	if ( !other || !other->isSubclassOf( Sentient ) )
	{
		return;
	}
	
	sen = ( Sentient * )other;
	
	if ( sen->health >= sen->max_health )
		return;
	
	if ( !ItemPickup( other, false ) )
	{
		return;
	}
	
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( gpm->hasObject(getArchetype()) )
	{
		if ( gpm->hasFormula("HealthPotion") )
		{
			GameplayFormulaData fd(this);
			amount = gpm->calculate("HealthPotion", fd, sen->max_health);
		}
		str snd = gpm->getStringValue(getArchetype() + ".Use", "wav");
		if ( snd.length() )
		{
			int channel = CHAN_BODY;
			float volume = -1.0f;
			float mindist = -1.0f;
			if ( gpm->hasProperty(getArchetype() + ".Use","channel") )
				channel = (int)gpm->getFloatValue(getArchetype() + ".Use", "channel");
			if ( gpm->hasProperty(getArchetype() + ".Use","volume") )
				volume = (int)gpm->getFloatValue(getArchetype() + ".Use", "volume");
			if ( gpm->hasProperty(getArchetype() + ".Use","mindist") )
				mindist = (int)gpm->getFloatValue(getArchetype() + ".Use", "mindist");
			Sound(snd, channel, volume, mindist);
		}
	}
	
	sen->health += (float)amount;
	
	if ( sen->health > sen->max_health )
	{
		sen->health = sen->max_health;
	}
	
	// If we are on fire stop it
	
	sen->ProcessEvent( EV_Sentient_StopOnFire );
}


//--------------------------------------------------------------
//
// Name:			processGameplayData
// Class:			HealthInventoryItem
//
// Description:		Called usually from the tiki file after all other
//					server side events are called.
//
// Parameters:		Event *ev -- not used
//
// Returns:			None
//
//--------------------------------------------------------------
void HealthInventoryItem::processGameplayData( Event * )
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(getArchetype()) )
		return;

	str objname = getArchetype();
	str useanim, usetype, usethread;
	if ( gpm->hasProperty(objname, "useanim") )
		useanim = gpm->getStringValue(objname, "useanim");
	if ( gpm->hasProperty(objname, "usethread") )
		usethread = gpm->getStringValue(objname, "usethread");
	if ( gpm->hasProperty(objname + ".Pickup", "icon") )
		usetype = gpm->getStringValue(objname + ".Pickup", "icon");

	// If any of these strings were set, add to our UseData object.
	if ( useanim.length() || usethread.length() || usetype.length() )
	{
		if ( !useData )
			useData = new UseData();
		
		useData->setUseAnim(useanim);
		useData->setUseThread(usethread);
		useData->setUseType(usetype);
	}
}
