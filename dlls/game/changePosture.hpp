//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/changePosture.hpp				              $
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
// CloseInOnEnemy Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class ChangePosture;

#ifndef __CHANGE_POSTURE_HPP__
#define __CHANGE_POSTURE_HPP__

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:           ChangePosture
// Base Class:     Behavior
//
// Description:    Makes the actor change posture
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class ChangePosture : public Behavior
{
	//------------------------------------
	// States
	//------------------------------------
	public:
		typedef enum
		{	
			CHANGE_POSTURE_SETUP,
			CHANGE_POSTURE_CHANGE,
			CHANGE_POSTURE_SUCCESS,
			CHANGE_POSTURE_FAILED
		} changePostureStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: // Parameters
		str							_posture;  
		
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( changePostureStates_t state );
		void	setInternalState	( changePostureStates_t state , const str &stateName );
		void	init				();
		void	think				();
		

		void					setupStateSetup		();
		BehaviorReturnCode_t	evaluateStateSetup	();
		void					failureStateSetup	( const str& failureReason );

		void					setupStateChange	();
		BehaviorReturnCode_t	evaluateStateChange	();
		void					failureStateChange	( const str& failureReason );
		

	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( ChangePosture );

												ChangePosture();
											   ~ChangePosture();

		void									SetArgs		( Event *ev     );      
		void									PostureDone	( Event *ev		);
		void									Begin		( Actor &self   );		
		BehaviorReturnCode_t					Evaluate	( Actor &self   );
		void									End			( Actor &self   );
		virtual void							Archive		( Archiver &arc );

		void									setPosture	( const str &postureName ) { _posture = postureName; }

	//-------------------------------------
	// Components
	//-------------------------------------
	
	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		changePostureStates_t					_state;
		bool									_postureDone;
		bool									_canChange;

};


inline void ChangePosture::Archive( Archiver &arc	)
{
	Behavior::Archive( arc );	 
   
   // Archive Parameters
	arc.ArchiveString ( &_posture );

   // Archive Components

   // Archive Member Variables
	ArchiveEnum					( _state, changePostureStates_t);
	arc.ArchiveBool				( &_postureDone );
	arc.ArchiveBool				( &_canChange );
}  

#endif /* __CHANGE_POSTURE_HPP__ */
