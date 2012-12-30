//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/coverCombatWithRangedWeapon.hpp                $
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
// CoverCombatWithRangedWeapon Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class SelectBestWeapon;

#ifndef __SELECT_BEST_WEAPON_HPP__
#define __SELECT_BEST_WEAPON_HPP__

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:           SelectBestWeapon
// Base Class:     Behavior
//
// Description:    Selects and "Equips" the "best" weapon in the
//				   Actor's inventory
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class SelectBestWeapon : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
			SBW_SELECT_WEAPON,
			SBW_PUT_AWAY_CURRENT_WEAPON,
			SBW_PULL_OUT_NEW_WEAPON,
			SBW_READY_NEW_WEAPON,			
			SBW_SUCCESS,
			SBW_FAILED
		} selectBestWeaponStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		EntityPtr	_currentEnemy;

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( selectBestWeaponStates_t state );
		void	setInternalState	( selectBestWeaponStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();		

		void					setupStateSelectWeapon ();
		BehaviorReturnCode_t	evaluateStateSelectWeapon();
		void					failureStateSelectWeapon( const str& failureReason );

		void					setupStatePutAwayCurrentWeapon();
		BehaviorReturnCode_t	evaluateStatePutAwayCurrentWeapon();
		void					failureStatePutAwayCurrentWeapon(const str& failureReason);

		void					setupStatePullOutNewWeapon();
		BehaviorReturnCode_t	evaluateStatePullOutNewWeapon();
		void					failureStatePullOutNewWeapon(const str& failureReason);

		void					setupStateReadyNewWeapon();
		BehaviorReturnCode_t	evaluateStateReadyNewWeapon();
		void					failureStateReadyNewWeapon( const str& failureReason );

	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( SelectBestWeapon );

										SelectBestWeapon();
									   ~SelectBestWeapon();

		void							SetArgs						( Event *ev );      
		void							AnimDone					( Event *ev );
		void							PostureDone					( Event *ev );

		void							Begin						( Actor &self );		
		BehaviorReturnCode_t			Evaluate					( Actor &self );
		void							End							( Actor &self );
		
		void							SetCurrentEnemy ( Entity* enemy );

		static	bool					CanExecute( Actor &self );
		virtual void					Archive  ( Archiver &arc );

		

	//-------------------------------------
	// Components
	//-------------------------------------
	private: 	


	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		selectBestWeaponStates_t	_state;
		Actor					   *_self;
		WeaponPtr					_bestWeapon;
		str							_currentWeaponName;
		str							_bestWeaponName;
		bool						_animDone;

	};

inline void	SelectBestWeapon::SetCurrentEnemy( Entity *enemy )
{
	_currentEnemy = enemy;
}

inline void SelectBestWeapon::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	     

	//
	// Archive Parameters
	//
	arc.ArchiveSafePointer( &_currentEnemy );
	
	//
	// Archive Components
	//

	//
	// Archive Member Variables
	//
	ArchiveEnum				( _state, selectBestWeaponStates_t   );
	arc.ArchiveObjectPointer( ( Class ** )&_self			);
	arc.ArchiveSafePointer	( &_bestWeapon );
	arc.ArchiveString		( &_currentWeaponName );
	arc.ArchiveString		( &_bestWeaponName );
	arc.ArchiveBool			( &_animDone );
}


#endif /* __SELECT_BEST_WEAPON_HPP__ */
