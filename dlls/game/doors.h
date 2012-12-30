//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/doors.h                                       $
// $Revision:: 9                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 9/12/02 6:26p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Doors are environment objects that slide open when activated by triggers
// or when used by the player.
//

#ifndef __DOORS_H__
#define __DOORS_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "scriptslave.h"

extern Event EV_Door_TryOpen;
extern Event EV_Door_GoDown;
extern Event EV_Door_GoUp;
extern Event EV_Door_HitBottom;
extern Event EV_Door_HitTop;
extern Event EV_Door_Fire;
extern Event EV_Door_Link;
extern Event EV_Door_SetSpeed;
extern Event EV_Door_Lock;
extern Event EV_Door_Unlock;

class Door;

typedef SafePtr<Door> DoorPtr;

class Door : public ScriptSlave
{
	protected:
		str			sound_stop;
		str			sound_move;
		str			sound_message;
		str			sound_locked;
		float       lastblocktime;
		float		angle;
		Vector		dir;
		float       diropened;
		int			state;
		int         previous_state;
		int			trigger;
		int			nextdoor;
		DoorPtr		master;
		float		next_locked_time;

		Vector		_extraTriggerSize;

		void				SetDir( Event *ev );
		void				OpenEnd( Event *ev );
		void				CloseEnd( Event *ev );
		void				Close( Event *ev );
		void				Open( Event *ev );
		void				DoorUse( Event *ev );
		void				DoorFire( Event *ev );
		void				DoorBlocked( Event *ev );
		void				FieldTouched( Event *ev );
		void				TryOpen( Event *ev );
		void				SpawnTriggerField( const Vector &fmins, const Vector &fmaxs );
		qboolean			DoorTouches( const Door *e1 );
		void				LinkDoors( Event *ev );
		void				SetTime( Event *ev );
		void				LockDoor( Event *ev );
		void				UnlockDoor( Event *ev );
		void				SetStopSound( const str &sound );
		void				SetStopSound( Event *ev );
		void				SetMoveSound( const str &sound );
		void				SetMoveSound( Event *ev );
		void				SetMessageSound( const str &sound );
		void				SetMessageSound( Event *ev );
		void				SetLockedSound( const str &sound );
		void				SetLockedSound( Event *ev );
		void				SetWait( Event *ev );
		void				SetDmg( Event *ev );
		void				setDamage( Event *ev );

	public:
		CLASS_PROTOTYPE( Door );

		qboolean	locked;

							Door();
		qboolean			isOpen( void );
		qboolean			isCompletelyClosed( void );
		qboolean			CanBeOpenedBy( Entity *ent );
		int					GetState( void );
		int					GetNextDoor( void );

		void				setExtraTriggerSize( Event *ev ) { _extraTriggerSize = ev->GetVector( 1 ); }

		virtual void		Archive( Archiver &arc );
	};

inline int Door::GetState( void )
{
	return state;
}

inline int Door::GetNextDoor( void )
{
	return nextdoor;
}

inline void Door::Archive( Archiver &arc )
{
	ScriptSlave::Archive( arc );

	arc.ArchiveString( &sound_stop );
	arc.ArchiveString( &sound_move );
	arc.ArchiveString( &sound_message );
	arc.ArchiveString( &sound_locked );

	if ( arc.Loading() )
	{
		SetStopSound( sound_stop );
		SetMoveSound( sound_move );
		SetMessageSound( sound_message );
		SetLockedSound( sound_locked );
	}

	arc.ArchiveFloat( &lastblocktime );
	arc.ArchiveFloat( &angle );
	arc.ArchiveVector( &dir );
	arc.ArchiveFloat( &diropened );
	arc.ArchiveInteger( &state );
	arc.ArchiveInteger( &previous_state );
	arc.ArchiveInteger( &trigger );
	arc.ArchiveInteger( &nextdoor );
	arc.ArchiveSafePointer( &master );
	arc.ArchiveFloat( &next_locked_time );

	arc.ArchiveVector( &_extraTriggerSize );

	arc.ArchiveBoolean( &locked );
}

class RotatingDoor : public Door
{
	protected:
		float		angle;
		Vector		startangle;
		int			init_door_direction;

	public:
		CLASS_PROTOTYPE( RotatingDoor );

							RotatingDoor();

		void				DoOpen( Event *ev );
		void				DoClose( Event *ev );
		void				OpenAngle( Event *ev );
		virtual void		Archive( Archiver &arc );
};

inline void RotatingDoor::Archive( Archiver &arc )
{
	Door::Archive( arc );

	arc.ArchiveFloat( &angle );
	arc.ArchiveVector( &startangle );
	arc.ArchiveInteger( &init_door_direction );
}

class SlidingDoor : public Door
{
	protected:
		float		totalmove;
		float		lip;
		Vector		pos1;
		Vector		pos2;
		float		basespeed;
		Vector		movedir;

	public:
		CLASS_PROTOTYPE( SlidingDoor );

							SlidingDoor();

		void				SetMoveDir( Event *ev );
		void				Setup( Event *ev );
		void				SetLip( Event *ev );
		void				SetSpeed( Event *ev );
		void				DoOpen( Event *ev );
		void				DoClose( Event *ev );
		virtual void		Archive( Archiver &arc );
};

inline void SlidingDoor::Archive( Archiver &arc )
{
	Door::Archive( arc );

	arc.ArchiveFloat( &totalmove );
	arc.ArchiveFloat( &lip );
	arc.ArchiveVector( &pos1 );
	arc.ArchiveVector( &pos2 );
	arc.ArchiveFloat( &basespeed );
	arc.ArchiveVector( &movedir );
}

class ScriptDoor : public Door
{
	protected:
		bool	_toggle;
		bool	_targeted;
		float	_openPercentage;
		float	_currentPercentage;
		float	_totalMove;
		float	_currentMove;
		float	_baseSpeed;
		float	_speed;
		Vector	_originalPos;
		Vector	_currentPos;
		Vector	_destinationPos;
		Vector	_moveDir;

	public:
		CLASS_PROTOTYPE( ScriptDoor );

						ScriptDoor();

		void			SetToggle( Event *ev );
		void			SetToggle( bool toggle );

		void			SetTargeted( Event *ev );
		void			SetTargeted( bool targeted );

		void			SetOpenPercentage( Event *ev );
		void			SetOpenPercentage( float openPercentage );

		void			SetMoveDir( Event *ev );
		void			SetMoveDir( float moveDir );

		void			DoClose( Event *ev );
		void			DoClose();

		void			DoOpen( Event *ev );
		void			DoOpen();

		void			ScriptDoorGetState( Event *ev );

		void			SetUpMovement( Event *ev );
		void			SetUpMovement();

		void			SetUpScriptDoor( Event *ev );
		void			SetUpScriptDoor();

		void			ScriptDoorUse ( Event *ev );
		void			ScriptDoorOpen ( Event *ev );
		void			ScriptDoorClose ( Event *ev );
		void			ScriptDoorFieldTouched ( Event *ev );

		void			ScriptDoorForceOpen ( Event *ev );
		void			ScriptDoorForceClose ( Event *ev );

		virtual void	Archive( Archiver &arc );
};

inline void ScriptDoor::Archive( Archiver &arc )
{
	Door::Archive( arc );

	arc.ArchiveBool		( &_toggle			);
	arc.ArchiveBool		( &_targeted		);
	arc.ArchiveFloat	( &_openPercentage	);
	arc.ArchiveFloat	( &_currentPercentage );
	arc.ArchiveFloat	( &_totalMove		);
	arc.ArchiveFloat	( &_currentMove );
	arc.ArchiveFloat	( &_baseSpeed );
	arc.ArchiveFloat	( &_speed );
	arc.ArchiveVector	( &_originalPos		);
	arc.ArchiveVector	( &_currentPos );
	arc.ArchiveVector	( &_destinationPos );
	arc.ArchiveVector	( &_moveDir );
}

#endif /* doors.h */
