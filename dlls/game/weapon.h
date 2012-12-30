//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/weapon.h                                      $
// $Revision:: 121                                                            $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:43a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Header file for Weapon class.  The weapon class is the base class for
// all weapons in the game.  Any entity created from a class derived from the weapon
// class will be usable by any Sentient (players and monsters) as a weapon.
//
class Weapon;

#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "g_local.h"
#include "item.h"
#include "ammo.h"
#include "sentient.h"

extern Event EV_Weapon_GiveStartingAmmo;
extern Event EV_Weapon_GiveAmmoBoost;

typedef enum
	{
	FT_NONE,
	FT_BULLET,
	FT_PROJECTILE,
	FT_MELEE,
	FT_SPECIAL_PROJECTILE,
	FT_EXPLOSION,
	FT_TRIGGER_PROJECTILE,
	FT_CONTROL_PROJECTILE,
	FT_CONTROL_ZOOM
	} firetype_t;

typedef enum
	{
	WEAPON_ANIMATING,	
	WEAPON_READY,
	WEAPON_FIRING,
	WEAPON_LOWERING,
	WEAPON_RAISING,
	WEAPON_HOLSTERED,
	WEAPON_RELOADING,
	WEAPON_CHANGING,
	WEAPON_SWITCHINGMODE
	} weaponstate_t;

typedef enum
	{
	ACCURACY_STOPPED,
	ACCURACY_CHANGE,
	ACCURACY_WALK,
	ACCURACY_RUN,
	ACCURACY_CROUCH,
	MAX_ACCURACYTYPES
	} weaponaccuracy_t;


typedef enum
{
	ZOOM_NORMAL_FOV = 0,
	ZOOM_STAGE_1,
	ZOOM_STAGE_2,
	NUM_ZOOM_STAGES
} ZoomStage;

//========================================
// Bones used by weapon
//========================================
typedef enum
{
	WEAPONBONE_BARREL_TAG     
} weaponbones_t;

#define INITIALIZE_WEAPONMODE_VAR(var,value) \
	{ \
	int _ii; \
	for( _ii=0; _ii<MAX_FIREMODES; _ii++ ) \
		{ \
		var[_ii] = value; \
		} \
	}

class Player;

//------------------------- CLASS ------------------------------
//
// Name:          ActiveWeapon
// Base Class:    Class
//
// Description:   This is an active weapon.
//
// Method of Use: 
//--------------------------------------------------------------
class ActiveWeapon : public Class
   {
   public:
      WeaponPtr			weapon;
      weaponhand_t		hand;

      ActiveWeapon();
      void Archive( Archiver &arc );
   };

inline ActiveWeapon::ActiveWeapon()
   {
   weapon = NULL;
   hand	= WEAPON_ERROR;
   }

inline void ActiveWeapon::Archive( Archiver &arc )
   {
   arc.ArchiveSafePointer( &weapon );
   ArchiveEnum( hand, weaponhand_t );
   }

class WeaponProperty : public Class
{
	public:
									 WeaponProperty( const str &property ) : _property( property ) {  }
									~WeaponProperty() { }

		const str&					 getStringValue()					{ return _stringValue ; }
		float						 getFloatValue()					{ return _floatValue ; }

		void						 setStringValue( const str &value )	{ _stringValue	= value ; }
		void						 setFloatValue( float value )		{ _floatValue	= value ; }

	private:
		str							_property ;
		str							_stringValue ;
		float						_floatValue ;
};


class WeaponProperties : public Class
{
	public:
									 WeaponProperties();
									~WeaponProperties() { }

		WeaponProperty				 getWeaponProperty( const str &name );

		void						 addWeaponProperty ( const str &weaponProperty );

	private:
		Container<WeaponProperty>	_weaponProperties ;
};


class Weapon : public Item
	{
	protected:
		friend			class Player;
		friend			class WeaponDualWield;

	private:
		qboolean		attached;					// Is this weapon attached to something?
		float			nextweaponsoundtime;		// The next time this weapon should sound off
		str				current_attachToTag;		// The current name of the tag to attach itself to on the owner
		str				left_attachToTag;			// Tag to use when weapon is wielded in the left hand
		str				right_attachToTag;			// ...right hand
		str				dual_attachToTag;			// ...dual handed
		str				leftholster_attachToTag;	// Tag to use when weapon is put away from left hand
		str				rightholster_attachToTag;	// ...right hand
		str				dualholster_attachToTag;	// ...dual handed
		float			lastScale;					// Used for attaching to holster
		Vector			lastAngles;					// Used for attaching to holster
		qboolean		lastValid;					// Used for attaching to holster
		qboolean		auto_putaway;				// Weapon will put itself away when out of ammo
		qboolean		use_no_ammo;				// Weapon will be able to be used when it has no ammo
		qboolean		crosshair;					// Whether or not to display a crosshair with this weapon
		qboolean		torsoaim;					// Whether or not to torso aim with this weapon
		qboolean		special_move;				// Allows special movement or not
		EntityPtr		aim_target;					// Current target of the weapon
		firemode_t		curmode;					// The current weapon mode
		firemode_t		maxmode;					// The maximum valid mode for this weapon
		qboolean		switchmode;					// Speicifies that this is a switch mode weapon
		qboolean		targetidleflag;				// Flag to specify if we have just left a target idle
		int				chx;						// x screen coord crosshair offset
		int				chy;						// y screen coord crosshair offset
		Vector			realvieworg;				// real view origin (third person camera loc)
		qboolean		thirdperson;				// whether or not the player is in 3rd person
		bool			useActorAiming;
		float           _powerRating;              // how much damage per second the weapon will do
		float           _projectileDamage;         // how much the projectile launched from this weapon will do -- needs to match the damage in the projectile .tik file ( used by AI )
		float           _projectileSpeed;          // how fast the projectile launched from the weapon will move -- needs to match the damage in the projectile .tik file ( used by AI )
		bool			_arcProjectile;			   // whether or not to arc the projectile
		float			_lowArcRange;				// range at which the projectile will switch from high trajectory to normal
		bool			_playMissSound;				//Play snd_ricochet if the bullet impact hits a non-damagable entity

		Container<EntityPtr> meleeVictims;			// Melee victim list

	protected:
		float			maxrange;					// maximum effective firing distance (for AI)
		float			minrange;					// minimum safe firing distance (for AI)
		str 			viewmodel;					// the viewmodel of the weapon
		weaponstate_t	weaponstate;				// current state of the weapon
		int 			rank;						// rank of the weapon (relative to other weapons)
		int 			order;						// The order of this weapon in the inventory
		SentientPtr 	last_owner; 				// The last owner of the weapon
		float			last_owner_trigger_time;	// The time when the last owner may re-pickup this weapon
		qboolean		notdroppable;				// makes the weapon not able to be dropped
		int 			aimanim;					// The aim animation to use for this weapon (so it shoots straight)
		int 			aimframe;					// The aim frame to use for this weapon (so it shoots straight)
		Vector			leftHolsterAngles;			// Angles to set the weapon to when it's holstered
		Vector			rightHolsterAngles; 		// Angles to set the weapon to when it's holstered
		Vector			dualHolsterAngles;			// Angles to set the weapon to when it's holstered
		float			holsterScale;				// Scale the weapon should be set to when it's holstered
		float			_weildedScale;
		qboolean		quiet;						// Makes the weapon not alert actors
		float			next_noise_time;			// next time weapon will alert actors
		float			next_noammo_time;			// next time we can play out of ammo sound
		int 			burstcount; 				// The current amount of burst ammo used
		int 			burstcountmax;				// Max amount of ammo for a burst

		// Each of these arrays is used to describe the properties of the weapon
		// in its primary(index 0) and alternate(index 1) mode

		str 			ammo_type[MAX_FIREMODES];					// The type of ammo used
		int 			ammorequired[MAX_FIREMODES];				// The amount of ammo required to fire this weapon
		int 			startammo[MAX_FIREMODES];					// The starting amount of ammo when the weapon is picked up
		int 			_ammoBoost[ MAX_FIREMODES ];
		str 			projectileModel[MAX_FIREMODES];				// The model of the projectile fired
		float			bulletdamage[MAX_FIREMODES];				// The amount of damate a single bullet causes
		float			bulletcount[MAX_FIREMODES];					// The number of bullets the weapon fires
		float			bulletrange[MAX_FIREMODES];					// The range of the bullet
		float			bulletknockback[MAX_FIREMODES];				// The amount of knockback a bullet causes
		float			projectilespeed[MAX_FIREMODES];				// The speed of the projectile fired
		Vector			bulletspread[MAX_FIREMODES];				// The amount of spread bullets can have
		Vector			endbulletspread[MAX_FIREMODES];				// The final bullet spread (if different from initial) and time interval (in .z)
		firetype_t		firetype[MAX_FIREMODES];					// The type of fire (projectile or bullet)
		int 			ammo_clip_size[MAX_FIREMODES];				// The amount of rounds the clip can hold
		int 			ammo_in_clip[MAX_FIREMODES];				// The current amount of ammo in the clip
		float			max_charge_time[MAX_FIREMODES];				// The max amount of time the weapon may be charged.
		meansOfDeath_t	meansofdeath[MAX_FIREMODES];				// The means of death for this mode
		qboolean		loopfire[MAX_FIREMODES];					// The weapon loopfires and will not idle when shooting
		//qboolean		fullanimfire[MAX_FIREMODES];				// The weapon will play full fire anim even if key is released
		int 			action_level_increment[MAX_FIREMODES];		// Increments the action level everytime the weapon is fired
		str 			worldhitspawn[MAX_FIREMODES];				// The models to spawn when the weapon strikes the world
		float			next_fire_time[MAX_FIREMODES];				// The next time the weapon can fire
		float			fire_timer[MAX_FIREMODES];					// The times for each fire mode
		float			accuracy[MAX_FIREMODES][MAX_ACCURACYTYPES]; // Accuracy values for this weapon
		qboolean		burstmode[MAX_FIREMODES];					// This mode is a burst fire mode
		float			_burstModeDelay[MAX_FIREMODES];
		float			shootingMoveSpeedModifier[MAX_FIREMODES];	// Move speed modifier while shooting

		float			_viewShakeMagnitude[MAX_FIREMODES];
		float			_viewShakeDuration[MAX_FIREMODES];
		Vector			_viewMinShake[MAX_FIREMODES];
		Vector			_viewMaxShake[MAX_FIREMODES];
		bool			_viewShakeOverride[ MAX_FIREMODES ];
		bool			_noAmmoMode[MAX_FIREMODES];

		bool			_noDelay[MAX_FIREMODES];
		int				_chargedModels[MAX_FIREMODES];

		int				_spreadAnims[ MAX_FIREMODES ];
		float			_spreadTime[ MAX_FIREMODES ];

		float			_meleeWidth[ MAX_FIREMODES ];
		float			_meleeHeight[ MAX_FIREMODES ];
		float			_meleeLength[ MAX_FIREMODES ];

		Vector			_fireOffset[ MAX_FIREMODES ];

		int				_regenAmount[ MAX_FIREMODES ];
		float			_regenTime[ MAX_FIREMODES ];
		bool			_regenOnlyWhenIdle[ MAX_FIREMODES ];
		float			_nextRegenTime[ MAX_FIREMODES ];
     
		float			_maxViewShakeChange;

		float			startfiretime;								// The time that the trigger was first depressed in an anim
		int				autoAimTargetSelectionAngle;				// Whether or not the weapon will autoaim ( and the angle used to determine autoaiming)
		int				autoAimLockonAngle;							// Angle within which a target will be locked on to, allowing 
		float			charge_fraction;	// Fraction of a charge up time
		qboolean		putaway;			// This is set to true by the state system to signal a weapon to be putaway
		firemode_t		firemodeindex; 		// This is used as an internal index to indicate which mode to apply commands to
		weaponhand_t	hand;				// which hand the weapon may be wielded in

		float			reticuletime;		// Time it takes the reticule to settle.
		float			zoomfov;			// zoomfov for this weapon
		float			_lastZoomFov;
		float			startzoom;			// the zoom fov to start at.
		float			endzoom;			// the zoom fov to end at
		float			zoomtime;			// the amount of time to go from start zoom to end zoom
		float			startzoomtime;		// the start time of the zoom
		int 			aimtype;			// What accuracy modifiers we are using
		qboolean		usesameclip;		// Use the same clip for both fire modes
		float			chargetime; 		// Current charge time
		qboolean		targetidle; 		// Weapon has a specific target idle anim
		qboolean		donefiring;			// Whether or not the weapon is done firing;

		qboolean		zoomed;
		int 			targetingSkin;
		int 			shootingSkin;
		int 			_fullAmmoSkin;
		firemode_t		_fullAmmoMode;
		float			defaultMoveSpeedModifier;

		Vector			viewShake;

		bool			_controllingProjectile;
		bool			_controllingProjectileHidden;
		str				_controlEmitterName;
		str				_controlSoundName;
		bool			_canInterruptFiringState;

		ZoomStage		_zoomStage;

		bool			_autoReload;

		bool			_allowAutoSwitch;

		float			_nextSwitchTime;


		void			SetMaxRangeEvent( Event *ev );
		void			SetMinRangeEvent( Event *ev );
		void			SetSecondaryAmmo( const char *type, int amount, int startamount );
		virtual void	DetachGun( void );
		virtual void	AttachGun( weaponhand_t hand, qboolean holstering = false );
		void			PickupWeapon( Event *ev );
		void			DoneRaising( Event *ev );
		void			DoneFiring( Event *ev );
		void			DoneAnimating( Event *ev );
		void			Idle( Event *ev );
		qboolean		CheckReload( void );
		qboolean		CheckReload( firemode_t mode );
		void			DoneReloading( Event *ev );
		void			DoneReloadingBurst( Event *ev );
		void			SetAimAnim( Event *ev );
		virtual void	Shoot( Event *ev );
		void			ModeSet( Event *ev );
		void			SetFireType( Event *ev );
		void			SetProjectile( Event *ev );
		void			SetBulletDamage( Event *ev );
		void			SetBulletRange( Event *ev );
		void			SetBulletKnockback( Event *ev );
		void			SetBulletCount( Event *ev );
		void			SetBulletSpread( Event *ev );		
		void			SetAutoPutaway( Event *ev );
		void			SetRange( Event *ev );
		void			SetSpecialMove( Event *ev );
		void			SetUseNoAmmo( Event *ev );
		void			LeftAttachToTag( Event *ev );
		void			RightAttachToTag( Event *ev );
		void			DualAttachToTag( Event *ev );
		void			LeftHolsterAttachToTag( Event *ev );
		void			RightHolsterAttachToTag( Event *ev );
		void			DualHolsterAttachToTag( Event *ev );
		void			SetLeftHolsterAngles( Event *ev );
		void			SetRightHolsterAngles( Event *ev );
		void			SetDualHolsterAngles( Event *ev );
		void			SetHolsterScale( Event *ev );
		void			setWeildedScale( Event *ev );
		void			SetWorldHitSpawn( Event *ev );
		void			SetViewModel( Event *ev );
		void			DonePutaway( Event *ev );
		void			SetRegenAmmo( Event *ev );
		void			SetRegenOnlyWhenIdle( Event *ev );
		void			ChangeIdle( Event *ev );
		void			DrawBowStrain( Event *ev );
		void			AltDrawBowStrain( Event *ev );
		void			SetAccuracy( Event *ev );
		void			SetReticuleTime( Event *ev );
		void			SetZoomFOV( Event *ev );
		void			IncrementZoom(Event *ev);
		void			setZoomStage( Event* ev );
		void			changeZoomStage( float firstZoom, float secondZoom );

		void			SetStartZoom(Event *ev);
		void			SetEndZoom(Event *ev);
		void			SetZoomTime(Event *ev);
		void			SetAimType( Event *ev );
		void			SetFireTimer( Event *ev );
		void			UseSameClip( Event *ev );
		void			SetMaxModes( Event *ev );
		void			SetSwitchMode( Event *ev );
		void			DoneSwitching( Event *ev );
		void			DoneSwitchToMiddle( Event *ev );		
		void			TargetIdle( Event *ev );
		void			TargetIdleThink( Event *ev );
		void			SetBurstMode( Event *ev );
		void			setBurstModeDelay( Event *ev );
		void			UseActorAiming( Event *ev );
		void			PassToAnimate( Event *ev );
		void			SetArcProjectile( Event *ev );
		void			SetLowArcRange( Event *ev );
		void			SetPlayMissSound(Event *ev );
		virtual void	processGameplayData( Event *ev );

	public:
		CLASS_PROTOTYPE( Weapon );

		Weapon();
		Weapon( const char *file );
		~Weapon();

		bool					shouldArcProjectile();
		bool					shouldPlayMissSound();
		float					GetLowArcRange();
		int						GetRank( void );
		int 					GetOrder( void );
		void					SetRank( int order, int rank );
		float					GetMaxRange( void );
		float					GetMinRange( void );
		inline qboolean 		GetPutaway( void ){ return putaway; };
		inline void 			SetPutAway( qboolean p ){ putaway = p; };
		void					SetMaxRange( float val );
		void					SetMinRange( float val );
		void					SetHand( Event *ev );
		inline weaponhand_t		GetHand( void ){ return hand; };
		void					ForceIdle( void );
		void					SetAmmoRequired( Event *ev );
		void					SetStartAmmo( Event *ev );
		void					setAmmoBoost( Event *ev );
		int 					GetStartAmmo( firemode_t mode );
		int 					getAmmoBoost( firemode_t mode );
		int 					GetMaxAmmo( firemode_t mode );
		str 					GetAmmoType( firemode_t mode );
		firetype_t				GetFireType( firemode_t mode );
		void					SetAmmoType( Event *ev );
		void					SetAmmoAmount( int amount, firemode_t mode );
		void					UseAmmo( int amount, firemode_t mode );
		void					SetAmmoClipSize( Event *ev );
		void					SetAmmoInClip( Event *ev );
		void					SetModels( const char *world, const char *view );
		void					SetOwner( Sentient *ent );
		void					SetMaxChargeTime( Event *ev );
		void					SetAnim( const str &animName, Event *endevent = NULL, bodypart_t part = legs );
		void					SetAnim( const str &animName, const Event &endevent, bodypart_t part = legs );
		int 					AmmoAvailable( firemode_t mode );
		qboolean				UnlimitedAmmo( firemode_t mode );
		qboolean				HasAmmo( firemode_t mode, int numShots = 1 );

		qboolean				HasInvAmmo( firemode_t mode );
		qboolean				HasAmmoInClip( firemode_t mode, int numShots = 1 );
		int 					GetClipSize( firemode_t mode );
		int						GetRequiredAmmo( firemode_t mode );
		qboolean				ReadyToFire( firemode_t mode, qboolean playsound = true );
		virtual void			PutAway( void );
		qboolean				Drop( void );
		void					Fire( firemode_t mode );
		void					Charge( firemode_t mode );
		void					ReleaseFire( firemode_t mode, float chargetime );
		void					ClientFireDone( void );
		qboolean				Removable( void );
		qboolean				Pickupable( Entity *other );
		void					DetachFromOwner( void );
		virtual void			AttachToOwner( weaponhand_t hand );
		void					WeaponSound( Event *ev );
		void					GetMuzzlePosition( Vector *position, Vector *forward = NULL, Vector *right = NULL, Vector *up = NULL );
		void					GetActorMuzzlePosition( Vector *position, Vector *forward = NULL, Vector *right = NULL, Vector *up = NULL, const char* tagname = NULL);
		qboolean				AutoChange( void );
		int 					ClipAmmo( firemode_t mode );
		void					ProcessWeaponCommandsEvent(Event *ev);
		qboolean				IsDroppable( void );
		int 					ActionLevelIncrement( firemode_t mode );
		void					SetActionLevelIncrement( Event *ev );
		void					ForceState( weaponstate_t state );
		void					NotDroppableEvent( Event *ev );
		void					GiveStartingAmmo( Event *ev );
		void					giveAmmoBoost( Event *ev );
		void					AutoAim( Event *ev );
		void					Crosshair( Event *ev );
		void					TorsoAim( Event *ev );
		void					AttachToTag( Event *ev );
		void					SetQuiet( Event *ev );
		void					SetLoopFire( Event *ev );
		void					AddEffectsAnims( void );
		//void					SetFullAnimFire( Event *ev );

		// Apparently the special_projectile stuff is no longer used
		//inline virtual void  SpecialFireProjectile( const Vector &pos, const Vector &forward, const Vector &right, const Vector &up, Entity *owner, str projectileModel, float charge_fraction ) {};
		void					AttachToHolster( weaponhand_t hand );
		inline str				GetCurrentAttachToTag( void ){ return current_attachToTag; };
		inline void 			SetCurrentAttachToTag( const str &s ){ current_attachToTag = s; };
		inline str				GetLeftHolsterTag( void ){ return leftholster_attachToTag; };
		inline str				GetRightHolsterTag( void ){ return rightholster_attachToTag; };
		inline str				GetDualHolsterTag( void ){ return dualholster_attachToTag; };
		inline qboolean 		GetUseNoAmmo( void ){ return use_no_ammo; };
		void					SetMeansOfDeath( Event *ev );
		meansOfDeath_t			GetMeansOfDeath( firemode_t mode );
		void					SetAimTarget( Entity * );
		void					WorldHitSpawn( firemode_t mode, const Vector &org, const Vector &angles, float life );
		void					MakeNoise( Event *ev );
		virtual void			Archive(	Archiver &arc );
		inline void 			SetAimType ( int aim ) { aimtype = aim; }
		inline float			GetFireTime ( firemode_t mode ) { return next_fire_time[mode]; }
		inline qboolean 		GetSwitchMode() { return switchmode; }
		inline firemode_t		GetCurMode() { return curmode; }
		qboolean				ForceReload( );
		qboolean				HasFullClip();
		void					SwitchMode( );
		void					SetCHOffset(int chx, int chy);
		void					CheckForTargetedEntity();
		Vector					GetViewEndPoint();
		void					SetRealViewOrigin( const Vector &rv );
		void					SetThirdPerson( qboolean tp );		
		qboolean				IsDoneFiring();
		void					StartFiring(Event *ev);
		void					FinishedFiring( Event *ev );
		void					Zoom( Event *ev );
		void					endZoom( Event *ev );
		void					rezoom( Event *ev );
		void					endZoom( void );
		virtual void			ProcessTargetedEntity(EntityPtr entity);
		virtual void			Uninitialize(void);
		void					SetTargetingSkin( Event *ev );
		void					SetShootingSkin( Event *ev );
		void					setFullAmmoSkin( Event *ev );
		void					setMoveSpeedModifier( Event *ev );
		void					setShootingMoveSpeedModifier( Event *ev );
		float					getMoveSpeedModifier( void );
		void					playAnim( const str &animName, bool animatingFlag = true );

		void					setViewShakeInfo( Event *ev );
		void					setAdvancedViewShakeInfo( Event *ev );
		void					startViewShake( Event *ev );
		void					startViewShake( void );
		void					updateViewShake( void );
		void					reduceViewShake( Event *ev );
		void					clearViewShake( Event *ev );
		Vector					getViewShake( void );

		void                    SetPowerRating( Event *ev );
		void                    SetPowerRating( float damage );

		void                    SetProjectileSpeed( Event *ev );
		void                    SetProjectileSpeed( float speed );
		void                    SetProjectileDamage( Event *ev );
		void                    SetProjectileDamage( float damage );
		void					SetBulletSpread( float spreadX , float spreadY );

		float                   GetPowerRating();
		float                   GetBulletSpreadX(firemode_t idx);
		float                   GetRange(firemode_t idx);
		float                   GetProjectileDamage();
		float                   GetProjectileSpeed();
		

		void					ClearMeleeVictims() { meleeVictims.ClearObjectList(); }
		void					AdvancedMeleeAttack(const char* tag1, const char* tag2, bool critical=false);
		float					GetBulletDamage(firemode_t idx = FIRE_MODE1);

		Vector					getSpread( void );
		void					applySpread( Vector *forward, Vector *right, Vector *up );

		// Need a way to trick the weapon into thinking it's attached so 
		// that it will drop properly from the player.
		void					setAttached(bool flag) { attached = flag; }

		/* virtual */ float		RespawnTime( void );

		void					noAmmoMode( Event *ev );
		bool					hasNoAmmoMode( firemode_t mode );

		void					setNoDelay( Event *ev );
		bool					isModeNoDelay( firemode_t mode );

		void					pauseRegen( Event *ev );
		void					setChargedModels( Event *ev );

		void					setControllingProjectile( Event *ev );
		bool					getControllingProjectile( void );

		void					setCanInterruptFiringState( Event *ev );

		void					setSpreadAnimData( Event *ev );

		bool					canReload( void );

		int						getAmmoInClip( firemode_t mode );

		void					setMaxViewShakeChange( Event *ev );

		void					setControlParms( Event *ev );
		void					toggleProjectileControl( void );
		void					setProjectileControlHidden( Event *ev );

		void					setMeleeParms( Event *ev );

		void					setFireOffset( Event *ev );

		/* virtual */ void		cacheStrings( void );

		void					setAutoReload( Event *ev );
		void					setAllowAutoSwitch( Event *ev );
		void					forceReload( Event *ev );

		/* virtual */ void		Think( void );

		bool					shouldAutoSwitch( firemode_t mode );

		virtual	int				getStat( int statNum ) { return 0; }

		void					autoSwitch();

		str						getWeaponByPriority( int priority );
		int						getWeaponPriority();

		void					setNextSwitchTime( Event *ev );
		void					setNextSwitchTime( float time );

		/* virtual */ void		ArchivePersistantData( Archiver &arc );
   };

inline bool Weapon::shouldArcProjectile()
{
	return _arcProjectile;
}

inline float Weapon::GetLowArcRange()
{
	return _lowArcRange;
}

inline bool Weapon::shouldPlayMissSound()
{
	return  _playMissSound;
}
inline float Weapon::GetBulletDamage(firemode_t idx)
{
	return bulletdamage[idx];
}

inline void Weapon::SetPowerRating( float damage )
{
	_powerRating = damage;
}

inline float Weapon::GetPowerRating()
{
	if ( world && world->isThingBroken( item_name.c_str() ) )
		return 0.0f;

	return _powerRating;
}

inline void Weapon::SetProjectileSpeed( float speed )
{
	_projectileSpeed = speed;
}

inline float Weapon::GetProjectileSpeed()
{
	return _projectileSpeed;
}

inline void Weapon::SetProjectileDamage( float damage )
{
	_projectileDamage = damage;
}

inline float Weapon::GetProjectileDamage()
{
	return _projectileDamage;
}

inline float Weapon::GetBulletSpreadX( firemode_t idx )
{
	return bulletspread[idx].x;
}

inline float Weapon::GetRange( firemode_t idx )
{
	return bulletrange[idx];
}

inline void Weapon::Archive( Archiver &arc )
{
	int i,j;

	Item::Archive( arc );

	arc.ArchiveBoolean( &attached );

	arc.ArchiveFloat( &nextweaponsoundtime );
	arc.ArchiveString( &current_attachToTag );

	arc.ArchiveString( &left_attachToTag );
	arc.ArchiveString( &right_attachToTag );
	arc.ArchiveString( &dual_attachToTag );
	arc.ArchiveString( &leftholster_attachToTag );
	arc.ArchiveString( &rightholster_attachToTag );
	arc.ArchiveString( &dualholster_attachToTag );

	arc.ArchiveFloat( &lastScale );
	arc.ArchiveVector( &lastAngles );
	arc.ArchiveBoolean( &lastValid );
	arc.ArchiveBoolean( &auto_putaway );
	arc.ArchiveBoolean( &use_no_ammo );
	arc.ArchiveBoolean( &crosshair );
	arc.ArchiveBoolean( &torsoaim );

	arc.ArchiveBoolean( &special_move );
	arc.ArchiveSafePointer( &aim_target );

	ArchiveEnum( curmode, firemode_t );
	ArchiveEnum( maxmode, firemode_t );

	arc.ArchiveBoolean( &switchmode );
	arc.ArchiveBoolean( &targetidleflag );

	arc.ArchiveInteger( &chx );
	arc.ArchiveInteger( &chy );

	arc.ArchiveVector( &realvieworg );
	arc.ArchiveBoolean( &thirdperson );

	arc.ArchiveBool( &useActorAiming );

	arc.ArchiveFloat( &_powerRating );
	arc.ArchiveFloat( &_projectileDamage );
	arc.ArchiveFloat( &_projectileSpeed );

	arc.ArchiveBool( &_arcProjectile );
	arc.ArchiveFloat( &_lowArcRange );
	arc.ArchiveBool( &_playMissSound );

	int numEntries;
	if ( arc.Saving() )
		{
		numEntries = meleeVictims.NumObjects();
		arc.ArchiveInteger( &numEntries );

		EntityPtr eptr;
		for ( int i = 1 ; i <= numEntries ; i++ )
			{
			eptr = meleeVictims.ObjectAt( i );
			arc.ArchiveSafePointer( &eptr );			
			}
		}
	else
		{
		EntityPtr eptr;
		EntityPtr *eptrptr;
		arc.ArchiveInteger( &numEntries );

		meleeVictims.Resize( numEntries );

		for ( int i = 1 ; i <= numEntries ; i++ )
			{
			meleeVictims.AddObject( eptr );
			eptrptr = &meleeVictims.ObjectAt( i );
			arc.ArchiveSafePointer( eptrptr );		
			}
		}

	arc.ArchiveFloat( &maxrange );
	arc.ArchiveFloat( &minrange );
	arc.ArchiveString( &viewmodel );

	if ( arc.Loading() )
		gi.setviewmodel( edict, viewmodel.c_str() );

	ArchiveEnum( weaponstate, weaponstate_t );
	arc.ArchiveInteger( &rank );
	arc.ArchiveInteger( &order );

	arc.ArchiveSafePointer( &last_owner );
	arc.ArchiveFloat( &last_owner_trigger_time );
	arc.ArchiveBoolean( &notdroppable );
	arc.ArchiveInteger( &aimanim );
	arc.ArchiveInteger( &aimframe );
	arc.ArchiveVector( &leftHolsterAngles );
	arc.ArchiveVector( &rightHolsterAngles );
	arc.ArchiveVector( &dualHolsterAngles );
	arc.ArchiveFloat( &holsterScale );
	arc.ArchiveFloat( &_weildedScale );
	arc.ArchiveBoolean( &quiet );
	arc.ArchiveFloat( &next_noise_time );
	arc.ArchiveFloat( &next_noammo_time );

	arc.ArchiveInteger( &burstcount );
	arc.ArchiveInteger( &burstcountmax );
	

	for ( i=0;i<MAX_FIREMODES;i++ )
		{
		arc.ArchiveString( &ammo_type[i] );
		arc.ArchiveInteger( &ammorequired[i] );
		arc.ArchiveInteger( &startammo[i] );
		arc.ArchiveInteger( &_ammoBoost[ i ] );
		arc.ArchiveString( &projectileModel[i] );
		arc.ArchiveFloat( &bulletdamage[i] );
		arc.ArchiveFloat( &bulletcount[i] );
		arc.ArchiveFloat( &bulletrange[i] );
		arc.ArchiveFloat( &bulletknockback[i] );
		arc.ArchiveFloat( &projectilespeed[i] );
		arc.ArchiveVector( &bulletspread[i] );
		arc.ArchiveVector( &endbulletspread[i] );
		ArchiveEnum( firetype[i], firetype_t );
		arc.ArchiveInteger( &ammo_clip_size[i] );
		arc.ArchiveInteger( &ammo_in_clip[i] );
		arc.ArchiveFloat( &max_charge_time[i] );
		ArchiveEnum( meansofdeath[i], meansOfDeath_t );
		arc.ArchiveBoolean( &loopfire[ i ] );

		arc.ArchiveInteger( &action_level_increment[i] );

		arc.ArchiveString( &worldhitspawn[i] );

		arc.ArchiveFloat( &next_fire_time[i] );
		arc.ArchiveFloat( &fire_timer[i] );

		for ( j=0;j<MAX_ACCURACYTYPES;j++ )
			{
			arc.ArchiveFloat( &accuracy[i][j] );
			arc.ArchiveFloat( &accuracy[i][j] );
			arc.ArchiveFloat( &accuracy[i][j] );
			arc.ArchiveFloat( &accuracy[i][j] );
			arc.ArchiveFloat( &accuracy[i][j] );
			}

		arc.ArchiveBoolean( &burstmode[ i ] );
		arc.ArchiveFloat( &_burstModeDelay[ i ] );
		arc.ArchiveFloat( &shootingMoveSpeedModifier[i] );
		arc.ArchiveFloat( &_viewShakeMagnitude[i] );
		arc.ArchiveFloat( &_viewShakeDuration[i] );
		arc.ArchiveVector( &_viewMinShake[i] );
		arc.ArchiveVector( &_viewMaxShake[i] );
		arc.ArchiveBool( &_viewShakeOverride[i] );
		arc.ArchiveBool( &_noAmmoMode[i] );
		arc.ArchiveBool( &_noDelay[i] );
		arc.ArchiveInteger( &_chargedModels[i] );

		arc.ArchiveInteger( &_spreadAnims[i] );
		arc.ArchiveFloat( &_spreadTime[i] );
		arc.ArchiveFloat( &_meleeWidth[i] );
		arc.ArchiveFloat( &_meleeHeight[i] );
		arc.ArchiveFloat( &_meleeLength[i] );

		arc.ArchiveVector( &_fireOffset[ i ] );

		arc.ArchiveInteger( &_regenAmount[ i ] );
		arc.ArchiveFloat( &_regenTime[ i ] );
		arc.ArchiveBool( &_regenOnlyWhenIdle[ i ] );
		arc.ArchiveFloat( &_nextRegenTime[ i ] );
		}

	arc.ArchiveFloat( &_maxViewShakeChange );

	arc.ArchiveFloat( &startfiretime );
	arc.ArchiveInteger( &autoAimTargetSelectionAngle );
	arc.ArchiveInteger( &autoAimLockonAngle );
	arc.ArchiveFloat( &charge_fraction );
	arc.ArchiveBoolean( &putaway );
	ArchiveEnum( firemodeindex, firemode_t );
	ArchiveEnum( hand, weaponhand_t );

	arc.ArchiveFloat( &reticuletime );
	arc.ArchiveFloat( &zoomfov );
	arc.ArchiveFloat( &_lastZoomFov );

	arc.ArchiveFloat( &startzoom );
	arc.ArchiveFloat( &endzoom );
	arc.ArchiveFloat( &zoomtime );
	arc.ArchiveFloat( &startzoomtime );

	arc.ArchiveInteger( &aimtype );
	arc.ArchiveBoolean( &usesameclip );
	arc.ArchiveFloat( &chargetime );
	arc.ArchiveBoolean( &targetidle );

	arc.ArchiveBoolean( &donefiring );

	arc.ArchiveBoolean( &zoomed );

	arc.ArchiveInteger( &targetingSkin );
	arc.ArchiveInteger( &shootingSkin );
	arc.ArchiveInteger( &_fullAmmoSkin );
	ArchiveEnum( _fullAmmoMode, firemode_t );

	arc.ArchiveFloat( &defaultMoveSpeedModifier );

	arc.ArchiveVector( &viewShake );

	arc.ArchiveBool( &_controllingProjectile );
	arc.ArchiveBool( &_controllingProjectileHidden );
	arc.ArchiveString( &_controlEmitterName );
	arc.ArchiveString( &_controlSoundName );
	arc.ArchiveBool( &_canInterruptFiringState );

	ArchiveEnum( _zoomStage, ZoomStage );

	arc.ArchiveBool( &_autoReload );

	arc.ArchiveBool( &_allowAutoSwitch );

	arc.ArchiveFloat( &_nextSwitchTime );
   }

inline void Weapon::ArchivePersistantData( Archiver &arc )
{
	int i;

	for ( i = 0 ; i < MAX_FIREMODES ; i++ )
	{
		arc.ArchiveInteger( &ammo_in_clip[i] );
	}
}

typedef SafePtr<Weapon> WeaponPtr;

#endif /* weapon.h */
