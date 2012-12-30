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
class CoverCombatWithRangedWeapon;

#ifndef __COVERCOMBAT_WITH_RANGEDWEAPON_HPP__
#define __COVERCOMBAT_WITH_RANGEDWEAPON_HPP__

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:           CoverCombatWithRangedWeapon
// Base Class:     Behavior
//
// Description:   
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class CoverCombatWithRangedWeapon : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
			COVERCOMBAT_WRW_FIND_COVER,
			COVERCOMBAT_WRW_FIND_BETTER_COVER,
			COVERCOMBAT_WRW_MOVE_TO_COVER,						
			COVERCOMBAT_WRW_CHANGE_POSTURE_DUCK,
			COVERCOMBAT_WRW_CHANGE_POSTURE_STAND,
			COVERCOMBAT_WRW_FIRE_STANDING,
			COVERCOMBAT_WRW_FIRE_PAUSE_STANDING,
			COVERCOMBAT_WRW_SPOTTED,
			COVERCOMBAT_WRW_DUCKED,
			COVERCOMBAT_WRW_STANDING,
			COVERCOMBAT_WRW_SUCCESS,
			COVERCOMBAT_WRW_FAILED
		} coverCombatStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		str		_movementAnim;
		str		_torsoAnim;
		str		_fireAnim;
		float	_maxDistance;
		float	_fireTimeMin;
		float	_fireTimeMax;
		float	_pauseTimeMin;
		float	_pauseTimeMax;


	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( coverCombatStates_t state );
		void	setInternalState	( coverCombatStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();
		void	updateEnemy			();
		void	setTorsoAnim		();	

		void							setupRotate();
		BehaviorReturnCode_t			evaluateRotate();

		void							setupStateFindCover				();
		BehaviorReturnCode_t			evaluateStateFindCover			();
		void							failureStateFindCover			( const str& failureReason );

		void							setupStateMoveToCover			();
		BehaviorReturnCode_t			evaluateStateMoveToCover		();
		void							failureStateMoveToCover			( const str& failureReason );

		void							setupStateChangePostureDuck		();
		BehaviorReturnCode_t			evaluateStateChangePostureDuck	();
		void							failureStateChangePostureDuck	( const str& failureReason );

		void							setupStateChangePostureStand	();
		BehaviorReturnCode_t			evaluateStateChangePostureStand	();
		void							failureStateChangePostureStand	( const str& failureReason );

		void							setupStateSpotted				();
		BehaviorReturnCode_t			evaluateStateSpotted			();
		void							failureStateSpotted				( const str& failureReason );

		void							setupStateDucked				();
		BehaviorReturnCode_t			evaluateStateDucked				();
		void							failureStateDucked				( const str& failureReason );

		void							setupStateStanding				();
		BehaviorReturnCode_t			evaluateStateStanding			();
		void							failureStateStanding			( const str& failureReason );

		void							setupStateFindBetterCover		();
		BehaviorReturnCode_t			evaluateStateFindBetterCover	();
		void							failureStateFindBetterCover		( const str& failureReason );

		void							setupStateFireStanding			();
		BehaviorReturnCode_t			evaluateStateFireStanding		();
		void							failureStateFireStanding		( const str& failureReason );

		void							setupStateFirePauseStanding		();
		BehaviorReturnCode_t			evaluateStateFirePauseStanding	();
		void							failureStateFirePauseStanding	( const str& failureReason );



	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( CoverCombatWithRangedWeapon );

										CoverCombatWithRangedWeapon();
									   ~CoverCombatWithRangedWeapon();

		void							SetArgs						( Event *ev );      
		void							AnimDone					( Event *ev );
		void							PostureDone					( Event *ev );

		void							Begin						( Actor &self );		
		BehaviorReturnCode_t			Evaluate					( Actor &self );
		void							End							( Actor &self );

		void							SetMovementAnim( const str& movementAnim );
		const str&						GetMovementAnim();

		void							SetTorsoAnim( const str& torsoAnim );
		const str&						GetTorsoAnim();

		void							SetFireAnim( const str& fireAnim );
		const str&						GetFireAnim();

		void							SetMaxDistance( float maxDistance );
		float							GetMaxDistance();

		void							SetFireTimeMin( float fireTimeMin );
		float							GetFireTimeMin();

		void							SetFireTimeMax( float fireTimeMax );
		float							GetFireTimeMax();

		void							SetPauseTimeMin( float pauseTimeMin );
		float							GetPauseTimeMin();

		void							SetPauseTimeMax( float pauseTimeMax );
		float							GetPauseTimeMax();
		
		static	bool					CanExecute( Actor &self , float maxDistance );
		virtual void					Archive  ( Archiver &arc );

	//-------------------------------------
	// Components
	//-------------------------------------
	private: 	
		GotoPoint						_gotoPoint;
		FireWeapon						_fireWeapon;
		RotateToEntity					_rotate;


	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		coverCombatStates_t				_state;
		HelperNodePtr					_node; 		
		Actor						   *_self;
		EntityPtr						_currentEnemy;
		bool							_finishedPostureTransition;
		bool							_spotted;
		float							_nextSpotCheck;
		float							_nextStandTime;
		float							_nextDuckTime;
		float							_nextFireTime;
		float							_nextPauseTime;

		static const float				NODE_RADIUS;


	};

inline void CoverCombatWithRangedWeapon::SetMovementAnim( const str& movementAnim )
{
	_movementAnim = movementAnim;
}

inline const str& CoverCombatWithRangedWeapon::GetMovementAnim()
{
	return _movementAnim;
}

inline void CoverCombatWithRangedWeapon::SetTorsoAnim( const str& torsoAnim )
{
	_torsoAnim = torsoAnim;
}

inline const str& CoverCombatWithRangedWeapon::GetTorsoAnim()
{
	return _torsoAnim;
}

inline void CoverCombatWithRangedWeapon::SetFireAnim( const str& fireAnim )
{
	_fireAnim = fireAnim;
}

inline const str& CoverCombatWithRangedWeapon::GetFireAnim()
{
	return _fireAnim;
}

inline void CoverCombatWithRangedWeapon::SetMaxDistance( float maxDistance )
{
	_maxDistance = maxDistance;
}

inline float CoverCombatWithRangedWeapon::GetMaxDistance()
{
	return _maxDistance;
}

inline void CoverCombatWithRangedWeapon::SetFireTimeMin( float fireTimeMin )
{
	_fireTimeMin = fireTimeMin;
}

inline float CoverCombatWithRangedWeapon::GetFireTimeMin()
{
	return _fireTimeMin;
}

inline void CoverCombatWithRangedWeapon::SetFireTimeMax( float fireTimeMax )
{
	_fireTimeMax = fireTimeMax;
}

inline float CoverCombatWithRangedWeapon::GetFireTimeMax()
{
	return _fireTimeMax;
}

inline void CoverCombatWithRangedWeapon::SetPauseTimeMin( float pauseTimeMin )
{
	_pauseTimeMin = pauseTimeMin;
}

inline float CoverCombatWithRangedWeapon::GetPauseTimeMin()
{
	return _pauseTimeMin;
}

inline void CoverCombatWithRangedWeapon::SetPauseTimeMax( float pauseTimeMax )
{
	_pauseTimeMax = pauseTimeMax;
}

inline float CoverCombatWithRangedWeapon::GetPauseTimeMax()
{
	return _pauseTimeMax;
}

inline void CoverCombatWithRangedWeapon::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	     

	//
	// Archive Parameters
	//
	arc.ArchiveString		( &_movementAnim );
	arc.ArchiveString		( &_torsoAnim	 );
	arc.ArchiveString		( &_fireAnim	 );
	arc.ArchiveFloat		( &_maxDistance	 );
	arc.ArchiveFloat		( &_fireTimeMin	 );
	arc.ArchiveFloat		( &_fireTimeMax	 );
	arc.ArchiveFloat		( &_pauseTimeMin );
	arc.ArchiveFloat		( &_pauseTimeMax );

	//
	// Archive Components
	//
	arc.ArchiveObject		( &_gotoPoint		);
	arc.ArchiveObject		( &_fireWeapon		);
	arc.ArchiveObject		( &_rotate			);

	//
	// Archive Member Variables
	//
	ArchiveEnum				( _state, coverCombatStates_t   );
	arc.ArchiveSafePointer	( &_node						);
	arc.ArchiveObjectPointer( ( Class ** )&_self			);
	arc.ArchiveSafePointer	( &_currentEnemy				);
	arc.ArchiveBool			( &_finishedPostureTransition	);
	arc.ArchiveBool			( &_spotted						);
	arc.ArchiveFloat		( &_nextSpotCheck				);
	arc.ArchiveFloat		( &_nextStandTime				);
	arc.ArchiveFloat		( &_nextDuckTime				);
	arc.ArchiveFloat		( &_nextFireTime				);
	arc.ArchiveFloat		( &_nextPauseTime				);
}


#endif /* __COVERCOMBAT_WITH_RANGEDWEAPON_HPP__ */
