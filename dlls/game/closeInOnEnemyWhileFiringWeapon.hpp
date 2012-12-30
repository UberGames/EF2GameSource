//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/closeInOnEnemyWhileFiringWeapon.hpp            $
// $Revision:: 169                                                            $
//   $Author:: sketcher														  $
//     $Date:: 4/26/02 2:22p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// CloseInOnEnemyWhileFiringWeapon Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class CloseInOnEnemyWhileFiringWeapon;

#ifndef __CLOSE_IN_ON_ENEMY_WHILE_FIRING_WEAPON_HPP__
#define __CLOSE_IN_ON_ENEMY_WHILE_FIRING_WEAPON_HPP__

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:           CloseInOnEnemyWhileFiringWeapon
// Base Class:     Behavior
//
// Description:    Makes the actor move closer to its current enemy
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class CloseInOnEnemyWhileFiringWeapon : public Behavior
{
	//------------------------------------
	// States
	//------------------------------------
	public:
		typedef enum
		{	
			CIWF_APPROACH_SETUP_APPROACH,
			CIWF_APPROACH_FIRE,
			CIWF_APPROACH_FIRE_PAUSE,			
			CIWF_SUCCESS,
			CIWF_FAILED
		} CIWFStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		str							_approachAnim;  
		str							_aimAnim;
		str							_fireAnim;
		float						_fireTimeMin;
		float						_fireTimeMax;
		float						_pauseTimeMin;
		float						_pauseTimeMax;
		float						_dist;

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( CIWFStates_t state );
		void	setInternalState	( CIWFStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();
		void	updateEnemy			();
		void	setTorsoAnim		();	
	
		void					setupStateSetupApproach		();
		BehaviorReturnCode_t	evaluateStateSetupApproach	();
		void					failureStateSetupApproach	( const str& failureReason );

		void					setupStateApproachFire		();
		BehaviorReturnCode_t	evaluateStateApproachFire	();
		void					failureStateApproachFire	( const str& failureReason );

		void					setupStateApproachFirePause		();
		BehaviorReturnCode_t	evaluateStateApproachFirePause	();
		void					failureStateApproachFirePause	( const str& failureReason );


	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( CloseInOnEnemyWhileFiringWeapon );

												CloseInOnEnemyWhileFiringWeapon();
											   ~CloseInOnEnemyWhileFiringWeapon();

		void									SetArgs		( Event *ev     );      
		void									Begin		( Actor &self   );		
		BehaviorReturnCode_t					Evaluate	( Actor &self   );
		void									End			( Actor &self   );
		virtual void							Archive		( Archiver &arc );

		void									setAnim		( const str &animName );
		void									setTorsoAnim( const str &animName );
		void									setDist		( float distance      );

	//-------------------------------------
	// Components
	//-------------------------------------
	private:      
		GotoEntity								_chaseEnemy;
		FireWeapon								_fireWeapon;

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		CIWFStates_t							_state;
		EntityPtr								_currentEnemy;
		float									_nextFireTime;
		float									_nextPauseTime;
		Actor								   *_self;

};

inline void CloseInOnEnemyWhileFiringWeapon::Archive( Archiver &arc	)
{
	Behavior::Archive( arc );	 
   
   // Archive Parameters
	arc.ArchiveString ( &_approachAnim );
	arc.ArchiveString ( &_aimAnim );
	arc.ArchiveString ( &_fireAnim );
	arc.ArchiveFloat  ( &_fireTimeMin );
	arc.ArchiveFloat  ( &_fireTimeMax );
	arc.ArchiveFloat  ( &_pauseTimeMin );
	arc.ArchiveFloat  ( &_pauseTimeMax );
	arc.ArchiveFloat  ( &_dist );

   // Archive Components
	arc.ArchiveObject ( &_chaseEnemy );
	arc.ArchiveObject ( &_fireWeapon );

   // Archive Member Variables
	ArchiveEnum					( _state, CIWFStates_t);
	arc.ArchiveSafePointer		( &_currentEnemy				);
	arc.ArchiveFloat			( &_nextFireTime				);
	arc.ArchiveFloat			( &_nextPauseTime				);
	arc.ArchiveObjectPointer	( ( Class ** )&_self			);
}  

#endif /* __CLOSE_IN_ON_ENEMY_WHILE_FIRING_WEAPON_HPP__ */
