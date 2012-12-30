//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/sentient.h                                    $
// $Revision:: 77                                                             $
//   $Author:: Sketcher                                                       $
//     $Date:: 5/04/03 5:49p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// DESCRIPTION:
// Base class of entity that can carry other entities, and use weapons.
//

#ifndef __SENTIENT_H__
#define __SENTIENT_H__

#include "g_local.h"
#include "container.h"
#include "animate.h"
#include "characterstate.h"

extern Event EV_Sentient_Attack;
extern Event EV_Sentient_StartChargeFire;
extern Event EV_Sentient_ReleaseAttack;
extern Event EV_Sentient_GiveWeapon;
extern Event EV_Sentient_GiveAmmo;
extern Event EV_Sentient_GiveAmmoOverTime;
extern Event EV_Sentient_GiveArmor;
extern Event EV_Sentient_GiveItem;
extern Event EV_Sentient_GiveTargetname;
extern Event EV_Sentient_GiveInventoryItem;
extern Event EV_Sentient_GiveHealth;
extern Event EV_Sentient_SetBloodModel;
extern Event EV_Sentient_TurnOffShadow;
extern Event EV_Sentient_TurnOnShadow;
extern Event EV_Sentient_AddImmunity;
extern Event EV_Sentient_AddResistance;
extern Event EV_Sentient_RemoveImmunity;
extern Event EV_Sentient_RemoveResistance;
extern Event EV_Sentient_UpdateOffsetColor;
extern Event EV_Sentient_JumpXY;
extern Event EV_Sentient_MeleeAttackStart;
extern Event EV_Sentient_MeleeAttackEnd;
extern Event EV_Sentient_BlockStart;
extern Event EV_Sentient_BlockEnd;
extern Event EV_Sentient_SetMouthAngle;
extern Event EV_Sentient_SpawnBloodyGibs;
extern Event EV_Sentient_StopOnFire;
extern Event EV_Sentient_AddMeleeAttacker;
extern Event EV_Sentient_HeadWatchAllowed;
extern Event EV_Sentient_WeaponAnim;
extern Event EV_Sentient_SetViewMode;
extern Event EV_Armor_SetActiveStatus;
extern Event EV_Armor_SetMultiplier;

extern Event EV_Weapon_DrawBowStrain;
extern Event EV_Weapon_AltDrawBowStrain;

extern Event EV_Sentient_SetMyArmorAmount;

// Shutup compiler
class Armor;
class Weapon;
class Item;
class InventoryItem;
class Ammo;

#define MAX_ACTIVE_WEAPONS NUM_ACTIVE_WEAPONS

#define REGIONAL_DAMAGE_BACK		(1<<0)
#define REGIONAL_DAMAGE_FRONT		(1<<1)

typedef SafePtr<Weapon> WeaponPtr;
typedef SafePtr<Armor> ArmorPtr;

typedef struct
	{
	int MODIndex;
	int ResistanceAmount;
	} Resistance;

typedef struct
{
	float duration;
	float startTime;
	float maxDamage;
	float currentDamage;	
} damagethreshold_t;

/* typedef enum
	{
	POWERUP_NONE,
	POWERUP_SPEED,
	POWERUP_STEALTH,
	POWERUP_PROTECTION,
	POWERUP_FLIGHT,
	POWERUP_STRENGTH,
	POWERUP_ACCURACY
	} powerup_t; */

typedef enum
   {
	ARMORTYPE_NONE,
	ARMORTYPE_HEAVY,
	ARMORTYPE_MEDIUM,
	ARMORTYPE_LIGHT,
	ARMORTYPE_SHARD
	} armortype_t;

class Sentient : public Entity
	{
	private:
		float				_criticalHealthPercentage;
		damagethreshold_t	_damageThreshold;
		float				_hateModifier;


	protected:
		Container<int>		inventory;
		Container<Ammo *>	ammo_inventory;

		WeaponPtr				newWeapon;
		ArmorPtr				currentBaseArmor;
		//int					firing_frame;
		//int					firing_anim;
		Vector					offset_color;
		Vector					offset_delta;
		float					offset_time;
		float					knock_start_time;
		str						blood_model;
		int						last_surface_hit;
		int						last_bone_hit;
		unsigned int			last_region_hit;
		Container<int>			immunities;
		Container<Resistance *>	resistances;
		WeaponPtr				activeWeaponList[ MAX_ACTIVE_WEAPONS ];
		unsigned int			_viewMode;
		bool					_canSendInjuredEvent;
		bool					_headWatchAllowed;
		bool					_displayFireEffect;


		virtual void			EventTake				( Event *ev );
		virtual void			EventGiveAmmo			( Event *ev );
		virtual void			giveAmmoOverTime		( Event *ev );
		virtual void			EventGiveItem			( Event *ev );
		virtual void			EventGiveArmor			( Event *ev );
		virtual void			EventGiveHealth			( Event *ev );		
		virtual void			EventGiveTargetname		( Event *ev );
		virtual void			TurnOffShadow			( Event *ev );
		virtual void			TurnOnShadow			( Event *ev );
		virtual void			AddImmunity				( Event *ev );
		virtual void			RemoveImmunity			( Event *ev );
		virtual void			AddResistance			( Event *ev );
		virtual void			RemoveResistance		( Event *ev );
		void					AddHealth				( Event *ev );
		void					SetBloodModel			( Event *ev );
		void					UpdateOffsetColor		( Event *ev );
		void					CheckAnimations			( Event *ev );

		void					ArmorEvent				( Event *ev );
		void					ArmorDamage				( ::Damage &damage );
		virtual void			ArmorDamage				( Event *ev );
		void					ArmorDamage				( float damage, Entity *inflictor, Entity *attacker, const Vector &position, 
														  const Vector &direction, const Vector &normal, int knockback, int dflags, 
														  int meansofdeath, int surfaceNumber, int boneNumber, Entity *weapon , bool showPain );

		void					SetRegionalDamage		( int surface_number, int bone_number, Vector direction );
		virtual qboolean		CanBlock				( int meansofdeath, qboolean full_block	);
		void					AddBloodSpurt			( const Vector &position, const Vector &direction, int bone_number );
		qboolean				ShouldBleed				( int meansofdeath, qboolean dead );
		qboolean				ShouldGib				( int meansofdeath, float damage );
		str						GetBloodSpurtName		( void );
		str						GetBloodSplatName		( void );
		float					GetBloodSplatSize		( void );
		str						GetGibName				( void );
		virtual void			WeaponKnockedFromHands	( void );


	public:
		Vector					gunoffset;
		Vector					eyeposition;
		int						viewheight;
		int						means_of_death;
		qboolean				in_melee_attack;
		qboolean				in_ranged_attack;
		qboolean				in_block;
		qboolean				in_stun;
		qboolean				on_fire;
		float					on_fire_stop_time;
		float					next_catch_on_fire_time;
		int						on_fire_tagnums[3];
		EntityPtr				fire_owner;
		qboolean				attack_blocked;
		float					attack_blocked_time;
		float					max_mouth_angle;
		int						max_gibs;
		float					next_bleed_time;
		float					shotsFiredThisVolley;

		CLASS_PROTOTYPE( Sentient );

								Sentient();
		virtual					~Sentient();
		Vector					EyePosition					( void );
		virtual Vector			GunPosition					( void );

		//
		// Event Interface
		//
		void					BeginAttack					( Event *ev );
		void					EndAttack					( Event *ev );
		virtual void			FireWeapon					( Event *ev );
		virtual void			StopFireWeapon				( Event *ev );
		void					StartChargeFire				( Event *ev );
		virtual void			ReleaseFireWeapon			( Event *ev );
		void					JumpXY						( Event *ev );
		void					MeleeAttackStart			( Event *ev );
		void					MeleeAttackEnd				( Event *ev );
		void					RangedAttackStart			( Event *ev );
		void					RangedAttackEnd				( Event *ev );
		void					BlockStart					( Event *ev );
		void					BlockEnd					( Event *ev );
		void					StunStart					( Event *ev );
		void					StunEnd						( Event *ev );
		void					SetMaxMouthAngle			( Event *ev );
		void					OnFire						( Event *ev );
		void					StopOnFire					( Event *ev );
		void					SpawnBloodyGibs				( Event *ev );
		void					SetMaxGibs					( Event *ev );
		virtual void			SetStateFile				( Event *ev );
		void					setViewMode					( Event *ev );
		void					getActiveWeaponName			( Event *ev );
		void					getActiveWeaponName			( weaponhand_t hand, str& weaponName );
		void					CatchOnFire					( Event *ev );
		void					SwipeOn						( Event *ev );
		void					SwipeOff					( Event *ev );
		void					AddHealthOverTime			( Event *ev );
		void					AddHealthAtInterval			( Event *ev );
		void					SetCriticalHealthPercentage ( Event *ev );
		void					HeadWatchAllowed			( Event *ev );
		void					HeadWatchAllowed			( bool allowed );
		void					SetWeaponAnim				( Event *ev );
		void					SetDamageThreshold			( Event *ev );
		void					DisplayFireEffect			( Event *ev );
		void					DropItemEvent				( Event *ev );

		void					SetArmorActiveStatus		( Event *ev );
		void					SetArmorActiveStatus		( bool status );

		void					SetArmorMultiplier			( Event *ev );
		void					SetArmorMultiplier			( float multiplier );

		void					AddToMyArmor				( Event *ev );
		void					AddToMyArmor				( float amountToAdd );

		void					SetMyArmorAmount			( Event *ev );
		void					SetMyArmorAmount			( float amount );

		void					ClearDamageThreshold		( Event *ev );
		void					ClearDamageThreshold		();


		void					SetHateModifier			( Event *ev );
		void					SetHateModifier			( float modifier );
		float					GetHateModifier			();


		void					DropItemsOnDeath			();
		void					DropItem					( Item *itemToDrop );
		bool					ChangeWeapon				( Weapon *weapon, weaponhand_t hand );
		Weapon					*GetActiveWeapon			( weaponhand_t hand );
		Weapon					*BestWeapon					( Weapon *ignore = NULL );
		Weapon					*NextWeapon					( Weapon *weapon );
		Weapon					*PreviousWeapon				( Weapon *weapon );
		virtual bool			useWeapon					( const char *weaponname, weaponhand_t hand );
		int						NumWeapons					( void );
		//inline  int			GetFiringFrame( void ){ return firing_frame; };
		//inline  int			GetFiringAnim( void ){ return firing_anim; };
		int						AmmoCount					( const str &ammo_type );
		int						MaxAmmoCount				( const str &ammo_type );
		int						AmmoIndex					( const str &ammo_type );
		int						UseAmmo						( const str &ammo_type, int amount );
		int						GiveAmmo					( const str &type, int amount, bool pickedUp, int max_amount=-1 );
		Ammo					*FindAmmoByName				( const str &name );
		Item					*giveItem					( const str &itemname , int amount = 1, bool pickedUp = false, float skillLevel = 1.0f );
		void					takeItem					( const char *itemname );
		void					AddItem						( const Item *object );
		void					RemoveItem					( Item *object );
		Item					*FindItemByClassName		( const char *classname, Item *current = 0 );
		Item					*FindItemByModelname		( const char *modelname, Item *current = 0 );
		Item					*FindItemByExternalName		( const char *externalname, Item *current = 0 );
		Item					*FindItem					( const char *itemname, Item *current = 0 );
		Item					*FindBaseArmor				();
		void					FreeInventory				( void );
		void					FreeInventory				( Event *ev );
		qboolean				HasItem						( const char *itemname );
		int						NumInventoryItems			( void );
		Item					*NextItem					( Item *item );
		Item					*PrevItem					( Item *item );
		virtual void			DropInventoryItems			( void );
		void					ListInventory				( void );
		virtual void			setModel					( const char *model );
		virtual void			Archive						( Archiver &arc );
		virtual void			ArchivePersistantData		( Archiver &arc, qboolean sublevelTransition );      
		virtual qboolean		DoGib						( int meansofdeath );
		qboolean				Immune						( int meansofdeath );
		qboolean				Resistant					( int meansofdeath );
		int						GetResistanceModifier		( int meansofdeath );		
		void					SetMeleeAttack				( bool value );
		void					SetAttackBlocked			( qboolean blocked );
		void					SetOffsetColor				( float r, float g, float b, float time );
		virtual void			ReceivedItem				( Item * item );
		virtual void			RemovedItem					( Item * item );
		virtual void			AmmoAmountChanged			( Ammo * ammo, int inclip = 0 );
		void					AmmoAmountInClipChanged		( const str &ammo_type, int amount );
		void					TryLightOnFire				( int meansofdeath, Entity *attacker );
		virtual void			GetStateAnims				( Container<const char *> *c );
		void					SetArmorValue				( int armorVal );
		int						GetArmorValue				( void );
		virtual void			setViewMode					( const str &viewModeName );
		unsigned int			getViewMode					( void );
		void					AddHealth					( float healthToAdd, float maxHealth = 0.0f );
		void					DetachAllActiveWeapons		( void );
		void					AttachAllActiveWeapons		( void );
		qboolean				IsActiveWeapon				( const Weapon *weapon );
		void					ActivateWeapon				( Weapon *weapon, weaponhand_t hand );
		void					DeactivateWeapon			( Weapon *weapon );
		void					DeactivateWeapon			( weaponhand_t hand );
		void					SetHealth					( float newHealth );
		void					CheckDamageThreshold		( float damageValue );
		void					WeaponEffectsAndSound		( Entity *weapon, const str& objname, Vector pos );

		void					cacheStateMachineAnims( Event *ev );

		//
		// Accessors
		//
		void					SetCriticalHealthPercentage ( float percentage );
		float					GetCriticalHealthPercentage ();
		bool					getHeadWatchAllowed			()	{ return _headWatchAllowed; }
		virtual const str		getName() const { return ""; }

		void					freeConditionals( Container<Conditional *> &conditionalsToDelete );
	};


inline void Sentient::SetCriticalHealthPercentage( float percentage )
{
	_criticalHealthPercentage = percentage;
}

inline void Sentient::SetCriticalHealthPercentage( Event *ev )
{
	SetCriticalHealthPercentage(ev->GetFloat( 1 ) );
}

inline float Sentient::GetCriticalHealthPercentage()
{
	return _criticalHealthPercentage;
}

typedef SafePtr<Sentient> SentientPtr;

extern Container<Sentient *> SentientList;

#endif /* sentient.h */
