//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actor_combatsubsystem.h                        $
// $Revision:: 21                                                             $
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
// Combat Related Classes
//

class CombatSubsystem;

#ifndef __ACTOR_COMBAT_SUBSYSTEM_H__
#define __ACTOR_COMBAT_SUBSYSTEM_H__

#include "actor.h"
#include "actorincludes.h"
#include "weapon.h"

//============================
// Class CombatSubsystem
//============================
//
// Encapsulates combat related data and functionality for the actor
//
class CombatSubsystem 
	{
	public:
		CombatSubsystem();
		CombatSubsystem( Actor *actor );
		~CombatSubsystem();
		
		bool CanAttackTarget			 ( Entity *target							);
		bool CanAttackTargetFrom		 ( Entity *target , const Vector &startPos );		      		
		bool IsTargetInWeaponRange		 ( Entity *target							);		
		bool UsingWeaponNamed			 ( const str &weaponName						);
		bool WeaponIsFireType			 ( firetype_t fire_type							);
		bool HaveWeapon					 ();
		bool CanAttackEnemy				 ();     

		void UseActorWeapon				 (const str &weaponName , weaponhand_t hand		);
		void SetTraceInterval			 ( float interval								);
		void FireWeapon					 ();
		void StopFireWeapon				 ();
		void AimWeaponTag				 (Entity *target);
		void AimWeaponTag				 (const Vector &targetPos);
		void ClearAim					 ();
		
		void GetGunPositionData			 ( Vector *pos , Vector *forward = NULL, Vector *right = NULL, Vector *up = NULL );		
		float GetAimGunYaw				 ( const Vector &target							 );
		float GetAimGunPitch			 ( const Vector &target							 );
		WeaponPtr GetBestAvailableWeapon ( Entity *target                                );
		float GetActiveWeaponPowerRating ( Entity *target								 );
		str GetActiveWeaponName	 ();
		str GetActiveWeaponArchetype();

		bool GetProjectileLaunchAngles( Vector &launchAngles, const Vector &launchPoint, const float initialSpeed, const float gravity , const bool useHighTrajectory = false ) const;
		bool shouldArcProjectile();
		float GetLowArcRange();

		void OverrideSpread ( float spreadX , float spreadY );

		const str	GetAnimForMyWeapon( const str& property );
		float GetDataForMyWeapon( const str& property );

		Vector GetLeadingTargetPos( float projSpeed , Vector originalTargetPos , Entity *target );

		// Archiving
		void         DoArchive		( Archiver &arc , Actor *actor );
		virtual void Archive		( Archiver &arc );		 

	protected:
		void        _init();
		bool	    _traceHitTarget	( Entity *target , const Vector &startPos );
		float       getModifiedPowerRating ( Entity *target , Weapon *weapon );
		

	private:		
		ActiveWeapon	_activeWeapon;
		float			_nextTimeTracedToTarget;
		float			_traceInterval;
		bool			_canShootTarget;
		float			_yawDiff;
				
		Actor *act;
	};


#endif /* __ACTOR_COMBAT_SUBSYSTEM_H__ */
