//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/WeaponDualWield.cpp     $
// $Revision:: 6                                                             $
//   $Author:: Singlis                                                 $
//     $Date:: 9/26/03 2:36p                                     $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//


#include "WeaponDualWield.h"
#include <qcommon/gameplaymanager.h>

CLASS_DECLARATION( Weapon, WeaponDualWield, NULL )
{
	{ &EV_Anim,									&WeaponDualWield::PassToAnimate },
	{ &EV_NewAnim,								&WeaponDualWield::PassToAnimate },

	{ NULL, NULL }
};

WeaponDualWield::WeaponDualWield()
	:	_leftweapon(0),
		_rightweapon(0)
{
}

WeaponDualWield::~WeaponDualWield()
{

}

//--------------------------------------------------------------
//
// Name:			AttachToOwner
// Class:			WeaponDualWield
//
// Description:		Attach to the owner.
//
// Parameters:		weaponhand_t hand -- hand
//
// Returns:			None
//
//--------------------------------------------------------------
void WeaponDualWield::AttachToOwner( weaponhand_t )
{
	_leftweapon->SetOwner(owner);
	_rightweapon->SetOwner(owner);
	_leftweapon->AttachGun( WEAPON_LEFT );
	_rightweapon->AttachGun( WEAPON_RIGHT );
	_rightweapon->ForceIdle();
	_leftweapon->ForceIdle();
}

//--------------------------------------------------------------
//
// Name:			processGameplayData
// Class:			WeaponDualWield
//
// Description:		Process gameplay data
//
// Parameters:		Event *ev -- not used
//
// Returns:			None
//
//--------------------------------------------------------------
void WeaponDualWield::processGameplayData( Event * )
{
	ClassDef *cls;

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(getArchetype()) )
		return;

	str leftmodel = gpm->getStringValue(getArchetype(), "leftmodel");
	str rightmodel = gpm->getStringValue(getArchetype(), "rightmodel");

	if ( !_leftweapon )
	{
		cls = getClass( leftmodel );
		if ( !cls )
		{
			SpawnArgs args;
			args.setArg( "model", leftmodel );
			cls = args.getClassDef();
			if ( !cls )
				return;
		}
		_leftweapon = ( Weapon * )cls->newInstance();
		_leftweapon->setModel( leftmodel );
		_leftweapon->ProcessPendingEvents();
		_leftweapon->hideModel();
	}

	if ( !_rightweapon )
	{
		cls = getClass( rightmodel );
		if ( !cls )
		{
			SpawnArgs args;
			args.setArg( "model", rightmodel );
			cls = args.getClassDef();
			if ( !cls )
				return;
		}
		_rightweapon = ( Weapon * )cls->newInstance();
		_rightweapon->setModel( rightmodel );
		_rightweapon->ProcessPendingEvents();
		_rightweapon->hideModel();
	}
	
	// Process gameplay data on myself.
	Weapon::processGameplayData( NULL );
}

//===============================================================
// Name:		PassToAnimate
// Class:		WeaponDualWield
//
// Description: Passes animation events on to both the left
//				and right weapons.
// 
// Parameters:	Event* -- the event to pass on.
//
// Returns:		None
// 
//===============================================================
void WeaponDualWield::PassToAnimate( Event *ev )
{
	if ( _leftweapon )
	{
		Event *leftEvent = new Event( ev );
		_leftweapon->ProcessEvent( leftEvent );
	}
	if ( _rightweapon )
	{
		Event *rightEvent = new Event( ev );
		_rightweapon->ProcessEvent( rightEvent );
	}
}