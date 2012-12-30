//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/misc.cpp                                      $
// $Revision:: 13                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 2/18/02 1:35p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// DESCRIPTION:
// This is all the dispensing type objects in the world.  They basically work
// by the player using them.  As the player uses them they give the player
// something (health/ammo/etc.).
//

#ifndef __DISPENSER_HPP__
#define __DISPENSER_HPP__

//#include "g_local.h"
#include "entity.h"

//------------------------- CLASS ------------------------------
//
// Name:		Dispenser
// Base Class:	Entity
//
// Description:	This is an entity that dispenses stuff (health, ammo, etc) 
//              to anything that uses it
//
// Method of Use:	Just like any entity.
//
//--------------------------------------------------------------

class Dispenser : public Entity
{
	typedef enum
	{
		DISPENSE_TYPE_NONE,
		DISPENSE_TYPE_HEALTH,
		DISPENSE_TYPE_AMMO,
		DISPENSE_TYPE_ARMOR
	} DispenseType;

	typedef enum
	{
		DISPENSER_STATE_CLOSED,
		DISPENSER_STATE_CLOSING,
		DISPENSER_STATE_OPEN,
		DISPENSER_STATE_OPENING,
		DISPENSER_STATE_EMPTY,
		DISPENSER_STATE_EMPTYING
	} DispenserState;

private:
	float			_maxAmount;
	float			_amount;
	float			_dispenseRate;
	float			_regenRate;
	DispenseType	_dispenseType;
	str				_dispenseSubtype;
	float			_lastTimeUsed;
	DispenserState	_state;
	float			_openDistance;
	bool			_instant;
	SentientPtr		_entityDispensingTo;
	str				_useSoundName;
	bool			_playingSound;
	float			_lastSoundTime;

protected:
	void						setDispenserAlpha( void );
	bool						dispenseStuff( Sentient *sent );

public:

	CLASS_PROTOTYPE( Dispenser );

								Dispenser();

	/* virtual */ void			Think( void );
	void						useEvent( Event *ev );

	void						setMaxAmount( Event *ev );
	void						setDispenseRate( Event *ev );
	void						setRegenRate( Event *ev );
	void						setType( Event *ev );
	void						setSubtype( Event *ev );
	void						setOpenDistance( Event *ev );
	void						setInstant( Event *ev );
	void						setSoundName( Event *ev );

	void						animDone( Event * );

	virtual void				Archive( Archiver &arc );
};

inline void Dispenser::Archive( Archiver &arc )
{
	Entity::Archive( arc );

	arc.ArchiveFloat( &_maxAmount );
	arc.ArchiveFloat( &_amount );
	arc.ArchiveFloat( &_dispenseRate );
	arc.ArchiveFloat( &_regenRate );

	ArchiveEnum( _dispenseType, DispenseType );

	arc.ArchiveString( &_dispenseSubtype );
	arc.ArchiveFloat( &_lastTimeUsed );

	ArchiveEnum( _state, DispenserState );

	arc.ArchiveFloat( &_openDistance );

	arc.ArchiveBool( &_instant );

	arc.ArchiveSafePointer( &_entityDispensingTo );

	arc.ArchiveString( &_useSoundName );
	arc.ArchiveBool( &_playingSound );
	arc.ArchiveFloat( &_lastSoundTime );
}

#endif /* Dispenser.h */
