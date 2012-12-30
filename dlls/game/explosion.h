//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/explosion.h                                   $
// $Revision:: 12                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Standard explosion object that is spawned by other entites and not map designers.
// Explosion is used by many of the weapons for the blast effect, but is also used
// by the Exploder and MultiExploder triggers.  These triggers create one or more
// explosions each time they are activated.
//

#ifndef __EXPLOSION_H__
#define __EXPLOSION_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"


class Exploder : public Trigger
	{
	private:
		float			damage;

		void			MakeExplosion( Event *ev );
		void			SetDmg( Event *ev );
		void			SetHealth( Event *ev );
		void			setDamage( Event *ev );

	public:
		CLASS_PROTOTYPE( Exploder );

						Exploder();
		virtual void	Archive( Archiver &arc );
	};


inline void Exploder::Archive
	(
	Archiver &arc
	)

   {
   Trigger::Archive( arc );

   arc.ArchiveFloat( &damage );
   }

class MultiExploder : public Trigger
	{
	protected:
		float			explodewait;
		float			explode_time;
		float			duration;
		int				damage;
		float			randomness;

		void			MakeExplosion( Event *ev );
		void			SetDmg( Event *ev );
		void			SetDuration( Event *ev );
		void			SetWait( Event *ev );
		void			SetRandom( Event *ev );
		void			SetHealth( Event *ev );
		void			setDamage( Event *ev );

	public:
		CLASS_PROTOTYPE( MultiExploder );

						MultiExploder();
	   virtual void		Archive( Archiver &arc );
	};

inline void MultiExploder::Archive
	(
	Archiver &arc
	)
   {
   Trigger::Archive( arc );

   arc.ArchiveFloat( &explodewait );
   arc.ArchiveFloat( &explode_time );
   arc.ArchiveFloat( &duration );
   arc.ArchiveInteger( &damage );
   arc.ArchiveFloat( &randomness );
   }

void CreateExplosion
   (
   const Vector &pos,
   float  damage = 120,
   Entity *inflictor = NULL,
   Entity *attacker = NULL,
   Entity *ignore = NULL,
   const char *explosionModel = NULL,
   float  scale = 1.0f,
   float  radius = 0.0f
   );

class ExplodeObject : public MultiExploder
	{
	private:
      Container<str> debrismodels;
      int            debrisamount;
      float          severity;
		str				debrissound;
		str				explosionmodel;
		float			explosionradius;

		bool		_spawnEarthquake;
		bool		_haveSpawnedEarthquake;
		float		_earthquakeMagnitude;
		float		_earthquakeDuration;
		float		_earthquakeDistance;

      void           SetDebrisModel( Event *ev );
      void           SetSeverity( Event *ev );
      void           SetDebrisAmount( Event *ev );
      void           MakeExplosion( Event *ev );
		void           ExplodeTiki( Event *ev );
		void				Damage( Event *ev );
		void				SetSound( Event *ev );
		void				SetExplosion( Event *ev );

		void				setSpawnEarthquake( Event *ev );
		void				setEarthquakeDuration( Event *ev );
		void				setEarthquakeMagnitude( Event *ev );
		void				setEarthquakeDistance( Event *ev );

	public:
      CLASS_PROTOTYPE( ExplodeObject );

		               ExplodeObject();
	   virtual void   Archive( Archiver &arc );
	};

inline void ExplodeObject::Archive( Archiver &arc )
{
	MultiExploder::Archive( arc );

	debrismodels.Archive( arc );
	arc.ArchiveInteger( &debrisamount );
	arc.ArchiveFloat( &severity );
	arc.ArchiveString( &debrissound );
	arc.ArchiveString( &explosionmodel );
	arc.ArchiveFloat( &explosionradius );

	arc.ArchiveBool( &_spawnEarthquake );
	arc.ArchiveBool( &_haveSpawnedEarthquake );
	arc.ArchiveFloat( &_earthquakeMagnitude );
	arc.ArchiveFloat( &_earthquakeDuration );
	arc.ArchiveFloat( &_earthquakeDistance );
}

#endif /* explosion.h */
