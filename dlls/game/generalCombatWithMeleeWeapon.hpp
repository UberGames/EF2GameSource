//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/generalCombatWithMeleeWeapon.hpp               $
// $Revision:: 169                                                            $
//   $Author:: Bschofield                                                     $
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
// GeneralCombatWithMeleeWeapon Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class GeneralCombatWithMeleeWeapon;

#ifndef __GENERALCOMBAT_WITH_MELEEWEAPON_HPP__
#define __GENERALCOMBAT_WITH_MELEEWEAPON_HPP__

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:          GeneralCombatWithMeleeWeapon
// Base Class:    Behavior
//
// Description:   Allows the Actor to do general combat
//                with a melee weapon ( best used on open areas )
//
// Method of Use: Statemachine or another behavior
//--------------------------------------------------------------
class GeneralCombatWithMeleeWeapon : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
			{
			GENERAL_COMBAT_MELEE_SELECT_STATE,
			GENERAL_COMBAT_MELEE_RUSH_ENEMY,
			GENERAL_COMBAT_MELEE_STRAFE,
			GENERAL_COMBAT_MELEE_ATTACK, 
			GENERAL_COMBAT_MELEE_BLOCK,
			GENERAL_COMBAT_MELEE_CHANGE_POSTURE,
			GENERAL_COMBAT_MELEE_HOLD,
			GENERAL_COMBAT_MELEE_FAILED
			} generalMeleeCombatStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		str								_primaryTorsoAnim;      
		str								_attackAnim;
		str								_rotationAnim;
		str								_rushAnim;
		float							_maxDistanceToEngage;
		float							_meleeDistance;
		float							_strafeChance;
		float							_blockChance;
		float							_attackChance;
		float							_postureChangeChance;
		bool							_allowRushFailure;


	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void							init						( Actor &self );
		void							selectState					( Actor &self );
		void							setUpRotate					( Actor &self );
		void							setTorsoAnim				( Actor &self );
		void							setHeadWatchTarget			( Actor &self );

		bool							setupStrafe					( Actor &self );
		void							strafeFailed				( Actor &self );
		void							strafe						( Actor &self );

		bool							setupRushEnemy				( Actor &self );
		void							setupRushEnemyFailed		( Actor &self );
		void							rushEnemy					( Actor &self );

		bool							setupHold					( Actor &self );
		void							setupHoldFailed				( Actor &self );
		void							hold						( Actor &self );

		bool							setupAttack					( Actor &self );
		void							setupAttackFailed 			( Actor &self );
		void							attack						( Actor &self );

		bool							setupBlock					( Actor &self );
		void							setupBlockFailed			( Actor &self );
		void							block 						( Actor &self );

		bool							setupChangePosture			( Actor &self );
		void							setupChangePostureFailed	( Actor &self );
		void							changePosture 				( Actor &self );

		void							faceEnemy 					( Actor &self ); 	 

	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( GeneralCombatWithMeleeWeapon );

										GeneralCombatWithMeleeWeapon();
									   ~GeneralCombatWithMeleeWeapon();

		void							SetArgs( Event *ev ); 	 
		void							AnimDone ( Event *ev );

		void							Begin	( Actor &self );		
		BehaviorReturnCode_t			Evaluate( Actor &self );
		void							End 	( Actor &self );

		virtual void					Archive  ( Archiver &arc );

 
	//-------------------------------------
	// Member Variables
	//-------------------------------------
   private:  
	  str								_blockAttack;
	  str								_postureTransitionAnim;

	  unsigned int						_state;
	  unsigned int						_nextPostureState;		
	  
	  float 							_nextRotateTime;
	  float 							_nextStrafeAttemptTime;
	  float 							_nextRushAttemptTime;
	  float 							_nextPostureChangeTime;
	  float 							_exitHoldTime;
	  float								_exitBlockTime;

	  Actor*							_self;

	  // Components
	  RotateToEntity					_rotate;
	  GotoEntity						_rush;
	  Strafe							_strafeComponent;
	  
   };


inline void GeneralCombatWithMeleeWeapon::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );

	// Archive Parameters
	arc.ArchiveString		( &_primaryTorsoAnim	  );
	arc.ArchiveString		( &_attackAnim			  );
	arc.ArchiveString		( &_rotationAnim		  );
	arc.ArchiveString		( &_rushAnim			  );
	
	arc.ArchiveFloat 		( &_maxDistanceToEngage   );
	arc.ArchiveFloat		( &_meleeDistance		  );
	arc.ArchiveFloat		( &_strafeChance		  );
	arc.ArchiveFloat		( &_blockChance			  );
	arc.ArchiveFloat		( &_attackChance		  );
	arc.ArchiveFloat		( &_postureChangeChance   );
	arc.ArchiveBool			( &_allowRushFailure      );
	arc.ArchiveString		( &_blockAttack 		  );
	arc.ArchiveString		( &_postureTransitionAnim );

	arc.ArchiveUnsigned		( &_state				  );
	arc.ArchiveUnsigned		( &_nextPostureState	  );

	arc.ArchiveFloat 		( &_nextRotateTime		  );
	arc.ArchiveFloat 		( &_nextStrafeAttemptTime );
	arc.ArchiveFloat 		( &_nextRushAttemptTime   );
	arc.ArchiveFloat 		( &_nextPostureChangeTime );
	arc.ArchiveFloat 		( &_exitHoldTime		  );
	arc.ArchiveFloat 		( &_exitBlockTime		  );

	arc.ArchiveObjectPointer ( ( Class ** )&_self	  );

	// Archive Components
	arc.ArchiveObject		( &_rotate				  );
	arc.ArchiveObject		( &_rush				  );
	arc.ArchiveObject		( &_strafeComponent 	  );
}


#endif /* __GENERALCOMBAT_WITH_MELEEWEAPON_HPP__ */

