//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/PlayAnim.hpp									  $
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
class PlayAnim;

#ifndef __PLAY_ANIM_HPP__
#define __PLAY_ANIM_HPP__

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
class PlayAnim : public Behavior
{
	//------------------------------------
	// States
	//------------------------------------
	public:
		typedef enum
		{	
			PLAYANIM_SETUP,
			PLAYANIM_ANIMATE,
			PLAYANIM_SUCCESS,
			PLAYANIM_FAILED
		} PlayAnimStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		str							_legAnim;  
		str							_torsoAnim;
		float						_minTime;
		float						_maxTime;

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( PlayAnimStates_t state );
		void	setInternalState	( PlayAnimStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();
	
		void					setupStateSetup				();
		BehaviorReturnCode_t	evaluateStateSetup			();
		void					failureStateSetup			( const str& failureReason );

		void					setupStateAnimate			();
		BehaviorReturnCode_t	evaluateStateAnimate		();
		void					failureStateAnimate			( const str& failureReason );


	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( PlayAnim );

												PlayAnim();
											   ~PlayAnim();

		void									SetArgs		( Event *ev     );      
		void									Begin		( Actor &self   );		
		BehaviorReturnCode_t					Evaluate	( Actor &self   );
		void									End			( Actor &self   );
		virtual void							Archive		( Archiver &arc );

		void									setAnim		( const str &animName ) { _legAnim = animName;		}
		void									setTorsoAnim( const str &animName ) { _torsoAnim = animName;	}
		void									setMinTime	( float minTime		  ) { _minTime = minTime;		}
		void									setMaxTime	( float maxTime       ) { _maxTime = maxTime;		}

	//-------------------------------------
	// Components
	//-------------------------------------

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		PlayAnimStates_t						_state;	
		float									_endTime;
		
};

inline void PlayAnim::Archive( Archiver &arc	)
{
	Behavior::Archive( arc );	 
   
   // Archive Parameters
	arc.ArchiveString ( &_legAnim );
	arc.ArchiveString ( &_torsoAnim );
	arc.ArchiveFloat  ( &_minTime );
	arc.ArchiveFloat  ( &_maxTime);


   // Archive Components

   // Archive Member Variables
	ArchiveEnum					( _state, PlayAnimStates_t		);
	arc.ArchiveFloat			( &_endTime						);
}  

#endif /* __PLAY_ANIM_HPP__ */





