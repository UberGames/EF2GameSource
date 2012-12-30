//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/torsoAimAndFireWeapon.hpp		              $
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
// torsoAimAndFireWeapon Behavior Definition
//
//--------------------------------------------------------------------------------


//==============================
// Forward Declarations
//==============================
class TorsoAimAndFireWeapon;

#ifndef __TORSO_AIM_AND_FIRE_WEAPON___
#define __TORSO_AIM_AND_FIRE_WEAPON___

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:           TorsoAimAndFireWeapon
// Base Class:     Behavior
//
// Description:    Aims the torso at the current enemy ( If the
//				   current enemy is within the passed in limits )
//				   
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class TorsoAimAndFireWeapon : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
			TORSO_AIM_AND_FIRE_AIM,
			TORSO_AIM_AND_FIRE_PRE_FIRE,
			TORSO_AIM_AND_FIRE_ATTACK,	
			TORSO_AIM_AND_FIRE_POST_FIRE,
			TORSO_AIM_AND_FIRE_SUCCESS,
			TORSO_AIM_AND_FIRE_FAILED
		} TorsoAimAndFireStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		float			_aimTimeMin;
		float			_aimTimeMax;
		float			_fireTimeMin;
		float			_fireTimeMax;
		bool			_forceAttack;
		float			_maxTorsoYaw;
		float			_maxTorsoPitch;
		float			_maxTorsoTurnSpeed;
		int				_shots;
		bool			_aimOnly;
		bool			_repeat;
		
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( TorsoAimAndFireStates_t state );
		void	setInternalState	( TorsoAimAndFireStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();		
		void	updateEnemy			();
		void	AdjustTorsoAngles	( const Vector &tagPos , const Vector &watchPosition );            
		void	LerpTorsoBySpeed	( const Vector &angleDelta );	

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
		CLASS_PROTOTYPE( TorsoAimAndFireWeapon );

										TorsoAimAndFireWeapon();
									   ~TorsoAimAndFireWeapon();

		void							SetArgs			( Event *ev );      
		void							AnimDone		( Event *ev );
	
		void							Begin			( Actor &self );		
		BehaviorReturnCode_t			Evaluate		( Actor &self );
		void							End				( Actor &self );

		// Accessors		
		virtual void					Archive  ( Archiver &arc );

	//-------------------------------------
	// Components
	//-------------------------------------
	private: 	
		FireWeapon						_fireWeapon;

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		TorsoAimAndFireStates_t			_state;	
		Vector							_currentTorsoAngles;		
		float							_endFireTime;
		float							_endAimTime;
		EntityPtr						_currentEnemy;
		bool							_canAttack;
		bool							_animDone;
		str								_aimAnim;
		str								_preFireAnim;
		str								_fireAnim;	
		str								_postFireAnim;
		bool							_fireFailed;
		bool							_endOnAimFail;
		Actor						   *_self;		
		

	};

inline void TorsoAimAndFireWeapon::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	     

	//
	// Archive Parameters
	//	
	arc.ArchiveFloat( &_aimTimeMin );
	arc.ArchiveFloat( &_aimTimeMax );
	arc.ArchiveFloat( &_fireTimeMin );
	arc.ArchiveFloat( &_fireTimeMax );
	arc.ArchiveBool( &_forceAttack );
	arc.ArchiveFloat( &_maxTorsoYaw );
	arc.ArchiveFloat( &_maxTorsoPitch );
	arc.ArchiveFloat( &_maxTorsoTurnSpeed );
	arc.ArchiveBool( &_aimOnly );
	arc.ArchiveInteger( &_shots );
	arc.ArchiveBool( &_repeat );
	
	//
	// Archive Components
	//	
	arc.ArchiveObject		( &_fireWeapon		);

	//
	// Archive Member Variables
	//
	ArchiveEnum				( _state, TorsoAimAndFireStates_t	);
	arc.ArchiveVector		( &_currentTorsoAngles				);

	arc.ArchiveFloat( &_endFireTime );
	arc.ArchiveFloat( &_endAimTime );
	arc.ArchiveSafePointer( &_currentEnemy );
	arc.ArchiveBool( &_canAttack );
	arc.ArchiveBool( &_animDone );
	arc.ArchiveString( &_aimAnim );
	arc.ArchiveString( &_preFireAnim );
	arc.ArchiveString( &_fireAnim );
	arc.ArchiveString( &_postFireAnim );
	arc.ArchiveBool( &_fireFailed );
	arc.ArchiveBool( &_endOnAimFail );
	
	arc.ArchiveObjectPointer( ( Class ** )&_self				);
}


#endif /* __TORSO_AIM_AND_FIRE_WEAPON___ */

