//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/spawners.h                                    $
// $Revision:: 18                                                             $
//   $Author:: Jmartel                                                        $
//     $Date:: 9/26/02 7:07p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Various spawning entities

#ifndef __SPAWNWERS_H__
#define __SPAWNWERS_H__

#include "g_local.h"
#include "scriptslave.h"
#include "actor.h"

class Spawn : public ScriptSlave 
{
private:
	Container<str>		_modelNames;
	str					_spawnTargetName;
	str					_spawnTarget;
	str					_pickupThread;
	str					_spawnItem;
	float				_spawnChance;
	int					_attackMode;
	qboolean			_startHidden;
	str					_effectType;
	str					_effectName;
	str					_animName;
	Vector				_velocity;
	int					_spawnGroupID;
	str					_masterStateMap;
	str					_spawnGroupDeathThread;
	bool				_checkForSpace;

	Container<str>		_keys;
	Container<str>		_values;

	void				SetAngleEvent( Event *ev );
	void				SetPickupThread( Event *ev );
	void				ModelName( Event *ev );
	void				SpawnTargetName( Event *ev );
	void				SpawnTarget( Event *ev );
	void				AttackMode( Event *ev );
	void				SetSpawnItem( Event *ev );
	void				SetSpawnChance( Event *ev );
	void				SetStartHidden( Event *ev );
	void				SetSpawnEffect( Event *ev );
	void				SetAnimName( Event *ev );
	void				SpawnNow( Event *ev );
	void				SetSpawnVelocity( Event *ev );
	void				SetSpawnGroupID( Event *ev );
	void				SetSpawnMasterStateMap( Event *ev );
	void				SetSpawnGroupDeathThread( Event *ev );
	void				setCheckForSpace( Event *ev );

	void				setSpawnKeyValue( Event *ev );
	void				clearSpawnKeyValues( Event *ev );
		
protected:
	virtual 			~Spawn();
	void				SetArgs( SpawnArgs &args );
	virtual void		DoSpawn( Event *ev );
	void				postSpawn( Entity *spawn );
	bool				checkStuck( Entity *spawn );
	
public:
	CLASS_PROTOTYPE( Spawn );
	
	
	Spawn();
	   virtual void   Archive( Archiver &arc );
};

inline void Spawn::Archive
(
	Archiver &arc
	)
{
	ScriptSlave::Archive( arc );

	_modelNames.Archive( arc );

	arc.ArchiveString(	&_spawnTargetName );
	arc.ArchiveString(	&_spawnTarget );
	arc.ArchiveString(	&_pickupThread );
	arc.ArchiveString(	&_spawnItem );
	arc.ArchiveFloat(	&_spawnChance );
	arc.ArchiveInteger( &_attackMode );
	arc.ArchiveBoolean( &_startHidden );
	arc.ArchiveString(	&_effectType );
	arc.ArchiveString(	&_effectName );
	arc.ArchiveString(	&_animName );
	arc.ArchiveVector ( &_velocity );
	arc.ArchiveInteger ( &_spawnGroupID );
	arc.ArchiveString ( &_masterStateMap );
	arc.ArchiveString ( &_spawnGroupDeathThread );
	arc.ArchiveBool( &_checkForSpace );

	_keys.Archive( arc );
	_values.Archive( arc );
}

class RandomSpawn : public Spawn
	{
   private:
      float    min_time;
      float    max_time;

      void           MinTime( Event *ev );
      void           MaxTime( Event *ev );
      void           ToggleSpawn( Event *ev );
      void           Think( Event *ev );

	public:
      CLASS_PROTOTYPE( RandomSpawn );


							RandomSpawn();
	   virtual void   Archive( Archiver &arc );
	};

inline void RandomSpawn::Archive
	(
	Archiver &arc
	)
   {
   Spawn::Archive( arc );

   arc.ArchiveFloat( &min_time );
   arc.ArchiveFloat( &max_time );
   }

class ReSpawn : public Spawn
	{
   protected:
		virtual void	DoSpawn( Event *ev );
 	public:
      CLASS_PROTOTYPE( ReSpawn );
	};

class SpawnOutOfSight : public Spawn
	{
	private:
		bool checkFOV;
	protected:
		virtual void	DoSpawn( Event *ev );
		void			CheckFOV( Event* ev );
 	public:
      CLASS_PROTOTYPE( SpawnOutOfSight );
	  SpawnOutOfSight();
	  virtual void Archive( Archiver& arc );
	};
inline void SpawnOutOfSight::Archive( Archiver& arc )
{
	Spawn::Archive( arc );
	arc.ArchiveBool( &checkFOV );
}

class SpawnChain : public Spawn
	{
	private:
		bool			use3rdPersonCamera;
   protected:
		virtual void	DoSpawn( Event *ev );
 	public:
		CLASS_PROTOTYPE( SpawnChain );
						SpawnChain( void );
		void			Use3rdPersonCamera( Event *ev );
		virtual void   Archive( Archiver &arc );
	};

inline void SpawnChain::Archive( Archiver &arc )
{
   Spawn::Archive( arc );

   arc.ArchiveBool( &use3rdPersonCamera );
}

#endif //__SPAWNWERS_H__
