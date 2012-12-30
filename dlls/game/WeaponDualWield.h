//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/WeaponDualWield.h           $
// $Revision:: 5                                                             $
//   $Author:: Steven                                                  $
//     $Date:: 10/11/02 4:05a                                    $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//

class WeaponDualWield;

#ifndef __WEAPONDUALWIELD_H__
#define __WEAPONDUALWIELD_H__

#include "g_local.h"
#include "item.h"
#include "ammo.h"
#include "sentient.h"
#include "weapon.h"

class WeaponDualWield : public Weapon
{
private:
	Weapon			*_leftweapon;
	Weapon			*_rightweapon;
	
protected:
	void			PassToAnimate( Event *ev );
	
public:
	CLASS_PROTOTYPE( WeaponDualWield );
	
						WeaponDualWield();
						~WeaponDualWield();
	
	Weapon*				getRightWeapon()	{ return _rightweapon; }
	Weapon*				getLeftWeapon()		{ return _leftweapon; }
	
	void				processGameplayData( Event *ev );
	
	virtual void		AttachToOwner( weaponhand_t hand );
	void				Archive(	Archiver &arc );
};

inline void WeaponDualWield::Archive( Archiver &arc )
{
	Weapon::Archive( arc );

	arc.ArchiveObjectPointer( (Class **)&_leftweapon );
	arc.ArchiveObjectPointer( (Class **)&_rightweapon );
}

#endif
