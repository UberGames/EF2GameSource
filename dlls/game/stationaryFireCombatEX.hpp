//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/stationaryFireCombatEX.hpp		              $
// $Revision:: 169                                                            $
//   $Author:: sketcher                                                       $
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
// StationaryFireCombat Behavior Definition
//
//--------------------------------------------------------------------------------


//==============================
// Forward Declarations
//==============================
class StationaryFireCombatEX;

#ifndef __STATIONARY_FIRE_COMBAT_EX___
#define __STATIONARY_FIRE_COMBAT_EX___

#include "behavior.h"
#include "behaviors_general.h"
#include "gotoHelperNode.hpp"
#include "rotateToEntity.hpp"

//------------------------- CLASS ------------------------------
//
// Name:           StationaryFireCombatEx
// Base Class:     Behavior
//
// Description:   
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class StationaryFireCombatEX : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
			STATIONARY_FIRE_AIM,
			STATIONARY_FIRE_PRE_FIRE,
			STATIONARY_FIRE_ATTACK,	
			STATIONARY_FIRE_POST_FIRE,
			STATIONARY_FIRE_SUCCESS,
			STATIONARY_FIRE_FAILED
		} StationaryFireStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		str				_stance;
		float			_aimTimeMin;
		float			_aimTimeMax;
		float			_fireTimeMin;
		float			_fireTimeMax;
		bool			_forceAttack;
		

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( StationaryFireStates_t state );
		void	setInternalState	( StationaryFireStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();		
		void	updateEnemy			();
		void	faceEnemy			();

		void					setupStateAim				();
		BehaviorReturnCode_t	evaluateStateAim			();
		void					failureStateAim				( const str& failureReason );

		void					setupStatePreFire			();
		BehaviorReturnCode_t	evaluateStatePreFire		();
		void					failureStatePreFire			( const str& failureReason );

		void					setupStateAttack			();
		BehaviorReturnCode_t	evaluateStateAttack			();
		void					failureStateAttack			( const str& failureReason );

		void					setupStatePostFire			();
		BehaviorReturnCode_t	evaluateStatePostFire		();
		void					failureStatePostFire		( const str& failureReason );




	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( StationaryFireCombatEX );

										StationaryFireCombatEX();
									   ~StationaryFireCombatEX();

		void							SetArgs			( Event *ev );      
		void							AnimDone		( Event *ev );
	
		void							Begin			( Actor &self );		
		BehaviorReturnCode_t			Evaluate		( Actor &self );
		void							End				( Actor &self );

		// Accessors		
		void							SetAimAnim		( const str &anim )	{ _aimAnim = anim;		  }
		void							SetFireAnim		( const str &anim ) { _fireAnim = anim;		  }
		void							SetPreFireAnim	( const str &anim ) { _preFireAnim = anim;    }
		void							SetPostFireAnim ( const str &anim ) { _postFireAnim = anim;   }
		void							SetStance		( const str &anim ) { _stance = anim;		  }
		void							SetAimTimeMin	( float minTime )	{ _aimTimeMin = minTime;  }
		void							SetAimTimeMax	( float maxTime )	{ _aimTimeMax = maxTime;  }
		void							SetFireTimeMin	( float minTime )	{ _fireTimeMin = minTime; }
		void							SetFireTimeMax  ( float maxTime )	{ _fireTimeMax = maxTime; }



		virtual void					Archive  ( Archiver &arc );

	//-------------------------------------
	// Components
	//-------------------------------------
	private: 	
		FireWeapon						_fireWeapon;
		RotateToEntity					_rotateToEntity;


	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		StationaryFireStates_t			_state;	
		float							_endFireTime;
		float							_endAimTime;
		EntityPtr						_currentEnemy;
		bool							_animDone;	
		bool							_canAttack;
		str								_aimAnim;
		str								_preFireAnim;
		str								_fireAnim;	
		str								_postFireAnim;

		Actor						   *_self;		
		

	};

inline void StationaryFireCombatEX::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	     

	//
	// Archive Parameters
	//		
	arc.ArchiveString		( &_stance			);
	arc.ArchiveFloat		( &_aimTimeMin		);
	arc.ArchiveFloat		( &_aimTimeMax		);	
	arc.ArchiveFloat		( &_fireTimeMin		);
	arc.ArchiveFloat		( &_fireTimeMax		);
	arc.ArchiveBool			( &_forceAttack		);
	
	//
	// Archive Components
	//	
	arc.ArchiveObject		( &_fireWeapon		);
	arc.ArchiveObject		( &_rotateToEntity	);

	//
	// Archive Member Variables
	//
	ArchiveEnum				( _state, StationaryFireStates_t	);
	arc.ArchiveFloat		( &_endFireTime						);
	arc.ArchiveFloat		( &_endAimTime						);	
	arc.ArchiveSafePointer	( &_currentEnemy					);
	arc.ArchiveBool			( &_animDone						);
	arc.ArchiveBool			( &_canAttack						);
	arc.ArchiveString		( &_aimAnim							);
	arc.ArchiveString		( &_preFireAnim						);
	arc.ArchiveString		( &_fireAnim						);
	arc.ArchiveString		( &_postFireAnim					);

	arc.ArchiveObjectPointer( ( Class ** )&_self				);
}


#endif /* __STATIONARY_FIRE_COMBAT_EX___ */

