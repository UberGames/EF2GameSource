//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/watchEntityEX.hpp				              $
// $Revision:: 1                                                              $
//   $Author:: Sketcher                                                       $
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
// watchEntityEX Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class WatchEntityEX;

#ifndef __WATCH_ENTITY_EX_HPP__
#define __WATCH_ENTITY_EX_HPP__

#include "behavior.h"
#include "behaviors_general.h"
#include "rotateToEntity.hpp"

//------------------------- CLASS ------------------------------
//
// Name:           WatchEntity
// Base Class:     Behavior
//
// Description:    Will Rotate To, and Continue Rotating To 
//				   an entity for a specified amount of time.
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class WatchEntityEX : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
			WATCH_ENTITY_EX_SETUP,
			WATCH_ENTITY_EX_ROTATE,
			WATCH_ENTITY_EX_HOLD,			
			WATCH_ENTITY_EX_SUCCESS,
			WATCH_ENTITY_EX_FAILED,
		} watchEntityStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		str								_stance;
		str								_shuffleAnim;
		str								_torsoAnim;
		EntityPtr						_ent;

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( watchEntityStates_t state );
		void	setInternalState	( watchEntityStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();		
		
		void					setupStateSetup				();
		BehaviorReturnCode_t	evaluateStateSetup			();
		void					failureStateSetup			( const str& failureReason );

		void					setupStateRotate			();
		BehaviorReturnCode_t	evaluateStateRotate			();
		void					failureStateRotate			( const str& failureReason );

		void					setupStateHold				();
		BehaviorReturnCode_t	evaluateStateHold			();
		void					failureStateHold			( const str& failureReason );


	

	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( WatchEntityEX );

										WatchEntityEX();
									   ~WatchEntityEX();

		void							SetArgs					( Event *ev );      
		void							AnimDone				( Event *ev );

		void							Begin					( Actor &self );		
		BehaviorReturnCode_t			Evaluate				( Actor &self );
		void							End						( Actor &self );
		
		void							SetEntity				( Entity *ent );

		virtual void					Archive  ( Archiver &arc );

	//-------------------------------------
	// Components
	//-------------------------------------
	private: 		
		RotateToEntity					_rotateToEntity;

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		unsigned int					_state;		
		float							_time;
		float							_turnspeed;	

	};

inline void WatchEntityEX::SetEntity( Entity *ent )
{
	if ( ent )
		_ent = ent;
}

inline void WatchEntityEX::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	     

	// Archive Parameters
	arc.ArchiveString			( &_stance	  );
	arc.ArchiveString			( &_shuffleAnim	  );
	arc.ArchiveString			( &_torsoAnim );
	arc.ArchiveSafePointer		( &_ent		  );

	// Archive Components	
	arc.ArchiveObject			( &_rotateToEntity			);

	// Archive Member Vars      
	arc.ArchiveUnsigned			( &_state					);
	arc.ArchiveFloat			( &_time					);
	arc.ArchiveFloat			( &_turnspeed				);
	
}



#endif /* __WATCH_ENTITY_EX_HPP__ */

