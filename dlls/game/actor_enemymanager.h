//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actor_enemymanager.h                           $
// $Revision:: 16                                                             $
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
// EnemyManager class for Actors -- Handles all the hatelist management
//

class EnemyManager;

#ifndef __ACTOR_ENEMYMANAGER_H__
#define __ACTOR_ENEMYMANAGER_H__

#include "actor.h"
#include "actorincludes.h"
#include "weapon.h"

//============================
// Class EnemyManager
//============================
//
// Class used to handle all enemy management by actors.
//
// Notes:
// _currentEnemy and _alternateTarget -- _currentEnemy should always be used to 
// house the Sentient ( i.e. Actor or Player ) at the top of the hatelist.  
// _alternateTarget should be used for "non-living" things ( Like ThrowObjects or
// ExplodingObjects... things like that.  
//
// As behaviors get used, and proves necessary, we can add a flag that will allow the
// state machine to set and use the _alternateTarget
class EnemyManager 
	{
	public: 
		 EnemyManager();
		 EnemyManager( Actor *actor );
		~EnemyManager();

		void			FindHighestHateEnemy();
		void			FindNextEnemy();
		void			FindClosestEnemy();
		void			ClearCurrentEnemy();
		void			ClearHateList();
		qboolean		Hates( Entity *ent );
		qboolean		Likes( Entity *ent );	
		qboolean		CanAttack( Entity *ent );
		qboolean		CanAttackAnyEnemy();

		EntityPtr		GetCurrentEnemy();
		void			SetCurrentEnemy( Entity *enemy );

		bool			CanGetToEntity(Entity *enemy);

		EntityPtr		GetAlternateTarget();
		void			SetAlternateTarget( Entity *target );
		
		void			TryToAddToHateList( Entity *enemy );		  
		qboolean		IsInHateList( Entity *enemy );
		qboolean		IsLastInHateList( Entity* enemy );
		
		void			AdjustHate( Entity *enemy , float adjustment );
		void			AdjustDamageCaused( Entity *enemy, float adjustment );

		void			TrivialUpdate();
		void			Update();
		void			UpdateDistance( HateListEntry_t *listIndex );
		void			UpdateCanSee( HateListEntry_t *listIndex );
		void			UpdateAttackers( HateListEntry_t *listIndex );

		void			LockOnCurrentEnemy( qboolean lock );
		qboolean		IsLockedOnCurrentEnemy();

		Vector			GetAwayFromEnemies();
		qboolean		InEnemyLineOfFire();
		float			GetDistanceFromEnemy();
		void			TrySleep( void );
		  
		bool			HasEnemy();

		// Utility Functions
		qboolean IsValidEnemy( Entity *enemy );
		bool	IsAnyEnemyInRange( float range );
		float	getEnemyCount();


		// Archiving
		virtual void	Archive( Archiver &arc );
		void			DoArchive ( Archiver &arc , Actor *actor );

   
	protected: //Member Functions			
			void		_AddToHateList( Entity *enemy );						
			int			_findEntityInHateList( Entity *searchEnt );
			

	private: //Member Variables
			Container<HateListEntry_t> _hateList;
			EntityPtr                  _currentEnemy;
			EntityPtr                  _lastEnemy;
			EntityPtr                  _alternateTarget;
			qboolean                   _lockedOnCurrentEnemy;
			float                      _currentEnemyHate;
			

			Actor                      *act;
	};



#endif /* __ACTOR_ENEMYMANAGER_H__ */
