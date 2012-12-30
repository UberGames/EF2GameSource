//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/suppressionWithRangedWeapon.hpp                $
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
// SuppressionFireCombat Behavior Definition
//
//--------------------------------------------------------------------------------


//==============================
// Forward Declarations
//==============================
class SuppressionFireCombat;

#ifndef __SUPPRESSION_FIRE_COMBAT___
#define __SUPPRESSION_FIRE_COMBAT___

#include "behavior.h"
#include "behaviors_general.h"
#include "gotoHelperNode.hpp"
#include "rotateToEntity.hpp"

//------------------------- CLASS ------------------------------
//
// Name:           CoverCombatWithRangedWeapon
// Base Class:     Behavior
//
// Description:   
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class SuppressionFireCombat : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
			SUPPRESSION_FIRE_FIND_NODE,
			SUPPRESSION_FIRE_MOVE_TO_NODE,
			SUPPRESSION_FIRE_ATTACK,
			SUPPRESSION_FIRE_PAUSE,
			SUPPRESSION_FIRE_SUCCESS,
			SUPPRESSION_FIRE_FAILED
		} SuppressionFireStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		str				_movementAnim;
		str				_torsoIdleAnim;
		str				_torsoAttackAnim;
		float			_maxDistance;
		float			_pauseTimeMin;
		float			_pauseTimeMax;
		float			_fireTimeMin;
		float			_fireTimeMax;
		

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( SuppressionFireStates_t state );
		void	setInternalState	( SuppressionFireStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();		
		void	updateEnemy			();
		void	faceEnemy			();

		void					setupStateFindNode			();
		BehaviorReturnCode_t	evaluateStateFindNode		();
		void					failureStateFindNode		( const str& failureReason );

		void					setupStateMoveToNode		();
		BehaviorReturnCode_t	evaluateStateMoveToNode		();
		void					failureStateMoveToNode		( const str& failureReason );

		void					setupStateAttack			();
		BehaviorReturnCode_t	evaluateStateAttack			();
		void					failureStateAttack			( const str& failureReason );

		void					setupStatePause				();
		BehaviorReturnCode_t	evaluateStatePause			();
		void					failureStatePause			( const str& failureReason );




	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( SuppressionFireCombat );

										SuppressionFireCombat();
									   ~SuppressionFireCombat();

		void							SetArgs			( Event *ev );      
		void							AnimDone		( Event *ev );
	
		void							Begin			( Actor &self );		
		BehaviorReturnCode_t			Evaluate		( Actor &self );
		void							End				( Actor &self );

		// Accessors		
		void							SetMovementAnim		( const str &anim );
		void							SetTorsoIdleAnim	( const str &anim );
		void							SetTorsoAttackAnim	( const str &anim );


		virtual void					Archive  ( Archiver &arc );

	//-------------------------------------
	// Components
	//-------------------------------------
	private: 	
		GotoHelperNode					_gotoHNode;
		FireWeapon						_fireWeapon;
		RotateToEntity					_rotateToEntity;


	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		SuppressionFireStates_t			_state;	
		bool							_atNode;
		float							_nextMoveAttempt;
		float							_endFireTime;
		float							_endPauseTime;
		HelperNodePtr					_node;
		EntityPtr						_currentEnemy;
		Actor						   *_self;		
		

	};

inline void	SuppressionFireCombat::SetMovementAnim ( const str &anim )
{
	_movementAnim = anim;
}

inline void	SuppressionFireCombat::SetTorsoIdleAnim ( const str &anim )
{
	_torsoIdleAnim = anim;
}

inline void	SuppressionFireCombat::SetTorsoAttackAnim ( const str &anim )
{
	_torsoAttackAnim = anim;
}

inline void SuppressionFireCombat::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	     

	//
	// Archive Parameters
	//
	arc.ArchiveString		( &_movementAnim	);
	arc.ArchiveString		( &_torsoIdleAnim	);
	arc.ArchiveString		( &_torsoAttackAnim );
	arc.ArchiveFloat		( &_maxDistance		);
	arc.ArchiveFloat		( &_pauseTimeMin	);
	arc.ArchiveFloat		( &_pauseTimeMax	);
	arc.ArchiveFloat		( &_fireTimeMin		);
	arc.ArchiveFloat		( &_fireTimeMax		);
	
	//
	// Archive Components
	//
	arc.ArchiveObject		( &_gotoHNode );
	arc.ArchiveObject		( &_fireWeapon );
	arc.ArchiveObject		( &_rotateToEntity );

	//
	// Archive Member Variables
	//
	ArchiveEnum				( _state, SuppressionFireStates_t   );
	arc.ArchiveBool			( &_atNode							);
	arc.ArchiveFloat		( &_nextMoveAttempt					);
	arc.ArchiveFloat		( &_endFireTime						);
	arc.ArchiveFloat		( &_endPauseTime					);
	arc.ArchiveSafePointer	( &_node							);
	arc.ArchiveSafePointer	( &_currentEnemy					);
	arc.ArchiveObjectPointer( ( Class ** )&_self				);
}


#endif /* __SUPPRESSION_FIRE_COMBAT___ */

