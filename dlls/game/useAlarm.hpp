//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/work.hpp										  $
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
// CooridorCombatWithRangedWeapon Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class UseAlarm;

#ifndef __USE_ALARM_HPP__
#define __USE_ALARM_HPP__

#include "behavior.h"
#include "behaviors_general.h"
#include "gotoHelperNode.hpp"

//------------------------- CLASS ------------------------------
//
// Name:          UseAlarm
// Base Class:    Behavior
//
// Description:   Makes the Actor use Work Helper Nodes
//
// Method of Use: Statemachine or another behavior
//--------------------------------------------------------------
class UseAlarm : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:
		typedef enum
		{
		USE_ALARM_FIND_NODE,
		USE_ALARM_MOVE_TO_NODE,
		USE_ALARM_AT_NODE,		
		USE_ALARM_WAIT_ON_ANIM,
		USE_ALARM_ROTATE_TO_ENEMY,
		USE_ALARM_SUCCESSFUL,
		USE_ALARM_FAILED
		} useAlarmStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: // Parameters
		str							_movementAnimName ; // anim to play to move to work node, default is "walk"
		float						_maxDistance;	    // maximum distance to look for node

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( useAlarmStates_t state );
		void	setInternalState	( useAlarmStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();				

		void					setupStateFindNode						();
		BehaviorReturnCode_t	evaluateStateFindNode					();
		void					failureStateFindNode					( const str& failureReason );

		void					setupStateMoveToNode					();
		BehaviorReturnCode_t	evaluateStateMoveToNode					();
		void					failureStateMoveToNode					( const str& failureReason );

		void					setupStateAtNode						();
		BehaviorReturnCode_t	evaluateStateAtNode						();
		void					failureStateAtNode						( const str& failureReason );

		void					setupStateWaitOnAnim					();
		BehaviorReturnCode_t	evaluateStateWaitOnAnim					();
		void					failureStateWaitOnAnim					( const str& failureReason );

		void					setupStateRotateToEnemy					();
		BehaviorReturnCode_t	evaluateStateRotateToEnemy				();
		void					failureStateRotateToEnemy				( const str& failureReason );


	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( UseAlarm );
												UseAlarm();
											   ~UseAlarm();

		void									SetArgs				( Event *ev );      
		void									AnimDone			( Event *ev );
		void									HandleNodeCommand	( Event *ev );

		void									Begin				( Actor &self );		
		BehaviorReturnCode_t					Evaluate			( Actor &self );
		void									End					( Actor &self );

		virtual void							Archive  ( Archiver &arc );

	private: // Component Behaviors
		GotoHelperNode							_gotoHelperNode;
		RotateToEntity							_rotateToEntity;

	private: // Member Variables
		HelperNodePtr							_node;            
		unsigned int							_state;
		bool									_animDone;
		Actor*									_self;
		
	};

inline void UseAlarm::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	    
   
	// Archive Parameters
	arc.ArchiveString			( &_movementAnimName);
	arc.ArchiveFloat			( &_maxDistance     );

	// Archive Components
	arc.ArchiveObject			( &_gotoHelperNode  );
	arc.ArchiveObject			( &_rotateToEntity  );
	

	// Archive Member Vars
	arc.ArchiveSafePointer		( &_node				);
	arc.ArchiveUnsigned			( &_state				);
	arc.ArchiveBool				( &_animDone			);
	arc.ArchiveObjectPointer	( ( Class ** )&_self	);
}

#endif /* __USE_ALARM__ */
