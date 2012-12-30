//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/powerups.h                                    $
// $Revision:: 30                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 5/16/03 8:41p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:

#ifndef __POWERUP_H__
#define __POWERUP_H__

class Powerup;
class PowerupBase;
class Rune;
class HoldableItem;

#include "g_local.h"
#include "item.h"


class PowerupBase : public Item
{
	protected:

		Sentient				*_owner;
		str						_modelName;

		str						_modelToAttachOnUse;
		str						_modelToAttachOnUseTag;
		float					_modelToAttachOnUseRemoveTime;
		str						_modelToSpawn;
		str						_shaderToDisplayOnUse;

		void					init( const str &modelname, Sentient *owner );
				

	public:
		CLASS_PROTOTYPE( PowerupBase );

								PowerupBase();
		virtual					~PowerupBase() {}

		void					realNameEvent( Event *ev );
		virtual void			amountEvent( Event *ev );

		virtual void			update( float frameTime ) { specificUpdate( frameTime ); }
		virtual void			specificUpdate( float frameTime ) {};

		virtual float			getMoveMultiplier( void ) { return 1.0f; };
		virtual float			getDamageDone( float damage, int meansOfDeath ) { return damage; };
		virtual float			getDamageTaken( Entity *attacker, float damage, int meansOfDeath ) { return damage; };

		virtual meansOfDeath_t	changetMeansOfDeath( meansOfDeath_t meansOfDeath ) { return meansOfDeath; };

		virtual void			spawn( const Vector &origin ) {};

		/* virtual */ Item *	ItemPickup( Entity *other, qboolean add_to_inventory, qboolean );

		virtual	void			givePlayerItem( Player *player ) {};

		/* virtual */ float		RespawnTime( void );

		void					setModelToAttachOnUse( Event *ev );
		void					setModelToSpawnOnUse( Event *ev );
		void					setShaderToDisplayOnUse( Event *ev );

		void					getModelToAttachOnUse( str &modelName, str &tagName, float &modelRemoveTime );
		void					getShaderToDisplayOnUse( str &modelName );

		virtual bool			canDrop( void ) { return true; }

		virtual bool			canOwnerRegenerate( void ) { return true; }

		/* virtual */ void		Archive( Archiver &arc );
};

inline void PowerupBase::Archive( Archiver &arc )
{
	Item::Archive( arc );

	arc.ArchiveObjectPointer( ( Class ** )&_owner );
	arc.ArchiveString( &_modelName );
	arc.ArchiveString( &_modelToAttachOnUse );
	arc.ArchiveString( &_modelToAttachOnUseTag );
	arc.ArchiveFloat( &_modelToAttachOnUseRemoveTime );
	arc.ArchiveString( &_shaderToDisplayOnUse );
}

class Powerup : public PowerupBase
{
	protected:

		float					_timeLeft;

	public:
		CLASS_PROTOTYPE( Powerup );

								Powerup();
		virtual					~Powerup() {}

		/* virtual */ void		amountEvent( Event *ev );

		float					getTimeLeft( void ) const { return _timeLeft; };

		/* virtual */ void		update( float frameTime );

		/* virtual */ void		spawn( const Vector &origin );

		/* virtual */	void	givePlayerItem( Player *player );

		static Powerup *		CreatePowerup( const str &className, const str &modelName, Sentient *sentient );

		/* virtual */ void		cacheStrings( void );

		float					getTimeLeft( void ) { return _timeLeft; }
		void					setTimeLeft( float timeLeft ) { _timeLeft = timeLeft; }

		virtual	bool			canStack( void ) { return true; }

		/* virtual */ void		Archive( Archiver &arc );
};

inline void Powerup::Archive( Archiver &arc )
{
	PowerupBase::Archive( arc );

	arc.ArchiveFloat( &_timeLeft );
}

class PowerupSpeed : public Powerup
{
	private:

	protected:

	public:
		CLASS_PROTOTYPE( PowerupSpeed );

								PowerupSpeed() {};
								~PowerupSpeed() {};

		virtual float			getMoveMultiplier( void ) { return 1.5f; };
};

class PowerupStrength : public Powerup
{
	private:

	protected:

	public:
		CLASS_PROTOTYPE( PowerupStrength );

								PowerupStrength() {};
								~PowerupStrength() {};

		/* virtual */ float		getDamageDone( float damage, int meansOfDeath ) { return damage * 2.0f; };
};

class PowerupProtection : public Powerup
{
	private:

	protected:

	public:
		CLASS_PROTOTYPE( PowerupProtection );

								PowerupProtection() {};
								~PowerupProtection() {};

		/* virtual */ float		getDamageTaken( Entity *attacker, float damage, int meansOfDeath );
};

class PowerupProtectionTemp : public PowerupProtection
{
	private:

	protected:

	public:
		CLASS_PROTOTYPE( PowerupProtectionTemp );

								PowerupProtectionTemp() {};
								~PowerupProtectionTemp() {};

		/* virtual */ float		getDamageDone( float damage, int meansOfDeath );
		/* virtual */ bool		canDrop( void ) { return false; }
		/* virtual */ bool		canStack( void ) { return false; }
};

class PowerupRegen : public Powerup
{
	private:

		static const float REGEN_SPEED;

	protected:

	public:
		CLASS_PROTOTYPE( PowerupRegen );

								PowerupRegen() {};
								~PowerupRegen() {};

		/* virtual */ void		specificUpdate( float frameTime );
};

class PowerupInvisibility : public Powerup
{
	private:
		bool			_started;

	protected:

	public:
		CLASS_PROTOTYPE( PowerupInvisibility );

								PowerupInvisibility();
								~PowerupInvisibility();

		/* virtual */ void		specificUpdate( float frameTime );

		/* virtual */ void		Archive( Archiver &arc );
};

inline void PowerupInvisibility::Archive( Archiver &arc )
{
	Powerup::Archive( arc );

	arc.ArchiveBool( &_started );
}

class Rune : public PowerupBase
{
	protected:
		Vector					_originalOrigin;
		bool					_originalOriginSet;
				
	public:
		CLASS_PROTOTYPE( Rune );

								Rune();
		virtual					~Rune() {}

		/* virtual */ void		setOrigin( const Vector &point );
		void					setOriginalOrigin( const Vector &point, bool force );
		Vector					getOriginalOrigin( void) { return _originalOrigin;}

		/* virtual */ void		spawn( const Vector &origin );

		/* virtual */ Item *	ItemPickup( Entity *other, qboolean add_to_inventory, qboolean );

		/* virtual */	void	givePlayerItem( Player *player );

		static Rune	*			CreateRune( const str &className, const str &modelName, Sentient *sentient );

		/* virtual */ void		cacheStrings( void );

		void					respawnAtOriginalOrigin( Event *ev );

		/* virtual */ void		Archive( Archiver &arc );
};

inline void Rune::Archive( Archiver &arc )
{
	PowerupBase::Archive( arc );

	arc.ArchiveVector( &_originalOrigin );
	arc.ArchiveBool( &_originalOriginSet );
}

class RuneDeathQuad : public Rune
{
	private:

	protected:

	public:
		CLASS_PROTOTYPE( RuneDeathQuad );

								RuneDeathQuad() {};
								~RuneDeathQuad() {};

		/* virtual */ void		specificUpdate( float frameTime );
		/* virtual */ float		getDamageDone( float damage, int meansOfDeath ) { return damage * 4.0f; };
		/* virtual */ bool		canOwnerRegenerate( void ) { return false; }

};

class RuneAmmoRegen : public Rune
{
	private:
		float					_nextGiveTime;

	protected:

	public:
		CLASS_PROTOTYPE( RuneAmmoRegen );

								RuneAmmoRegen();
								~RuneAmmoRegen() {};

		/* virtual */ void		specificUpdate( float frameTime );

		/* virtual */ void		Archive( Archiver &arc );
};

inline void RuneAmmoRegen::Archive( Archiver &arc )
{
	Rune::Archive( arc );

	arc.ArchiveFloat( &_nextGiveTime );
}

class RuneEmpathyShield : public Rune
{
	private:

	protected:

	public:
		CLASS_PROTOTYPE( RuneEmpathyShield );

								RuneEmpathyShield() {};
								~RuneEmpathyShield() {};

		/* virtual */  float	getDamageTaken( Entity *attacker, float damage, int meansOfDeath );
};

class RuneArmorPiercing : public Rune
{
	private:

	protected:

	public:
		CLASS_PROTOTYPE( RuneArmorPiercing );

								RuneArmorPiercing() {};
								~RuneArmorPiercing() {};

		/* virtual */ meansOfDeath_t	changetMeansOfDeath( meansOfDeath_t meansOfDeath ) { return MOD_ARMOR_PIERCING; };
};

class HoldableItem : public PowerupBase
{
	protected:

	public:
		CLASS_PROTOTYPE( HoldableItem );

								HoldableItem();
		virtual					~HoldableItem() {}

		virtual bool			use( void ) { return true; }

		/* virtual */ void		givePlayerItem( Player *player );

		static HoldableItem	*createHoldableItem( const str &className, const str &modelName, Sentient *sentient );

		/* virtual */ void		cacheStrings( void );
};

class HoldableItemHealth : public HoldableItem
{
public:
	CLASS_PROTOTYPE( HoldableItemHealth );

								HoldableItemHealth() {};
								~HoldableItemHealth() {};

	/* virtual */ bool			use( void );
};

class HoldableItemProtection : public HoldableItem
{
public:
	CLASS_PROTOTYPE( HoldableItemProtection );

								HoldableItemProtection() {};
								~HoldableItemProtection() {};

	/* virtual */ bool			use( void );
};

class HoldableItemTransporter : public HoldableItem
{
public:
	CLASS_PROTOTYPE( HoldableItemTransporter );

								HoldableItemTransporter() {};
								~HoldableItemTransporter() {};

	/* virtual */ bool			use( void );
};

class HoldableItemExplosive : public HoldableItem
{
private:
	bool						_explosiveArmed;
	bool						_explosiveAlive;
	float						_explosiveArmTime;
	EntityPtr					_explosive;
	float						_lastSoundTime;
	float						_nextProximitySoundTime;

	bool						findPlaceToSet( Vector &newOrigin, Vector &newAngles );
	bool						isPlayerInRange( const Vector &position, float maxDistance );

public:
	CLASS_PROTOTYPE( HoldableItemExplosive );

								HoldableItemExplosive();
								~HoldableItemExplosive();

	/* virtual */ bool			use( void );
	/* virtual */ void			specificUpdate( float frameTime );

	/* virtual */ void			Archive( Archiver &arc );
};

inline void HoldableItemExplosive::Archive( Archiver &arc )
{
	HoldableItem::Archive( arc );

	arc.ArchiveBool( &_explosiveArmed );
	arc.ArchiveBool( &_explosiveAlive );
	arc.ArchiveFloat( &_explosiveArmTime );
	arc.ArchiveSafePointer( &_explosive );

	arc.ArchiveFloat( &_lastSoundTime );
	arc.ArchiveFloat( &_nextProximitySoundTime );
}

class HoldableItemSpawnPowerup : public HoldableItem
{
private:
	str							_powerupToSpawn;

public:
	CLASS_PROTOTYPE( HoldableItemSpawnPowerup );

	/* virtual */ bool			use( void );

	void						powerupToSpawn( Event *ev );

	/* virtual */ void			Archive( Archiver &arc );
};

inline void HoldableItemSpawnPowerup::Archive( Archiver &arc )
{
	HoldableItem::Archive( arc );

	arc.ArchiveString( &_powerupToSpawn );
}

#endif /* Powerup.h */
