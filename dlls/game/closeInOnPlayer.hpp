//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/closeInOnEnemy.hpp				              $
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
class CloseInOnPlayer;

#ifndef __CLOSE_IN_ON_PLAYER_HPP__
#define __CLOSE_IN_ON_PLAYER_HPP__

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:           CloseInOnPlayer
// Base Class:     Behavior
//
// Description:    Makes the actor move closer to the player enemy
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class CloseInOnPlayer : public Behavior
{
	//------------------------------------
	// States
	//------------------------------------
	public:
		typedef enum
		{	
			CLOSE_IN_ON_PLAYER_APPROACH,
			CLOSE_IN_ON_PLAYER_SUCCESS,
			CLOSE_IN_ON_PLAYER_FAILED
		} closeInOnPlayerStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: // Parameters
		str							_anim;  
		str							_torsoAnim;
		float						_dist;

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( closeInOnPlayerStates_t state );
		void	setInternalState	( closeInOnPlayerStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();
		void	findPlayer			();
		void	setTorsoAnim		();	

		void					setupStateApproach		();
		BehaviorReturnCode_t	evaluateStateApproach	();
		void					failureStateApproach	( const str& failureReason );

	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( CloseInOnPlayer );

												CloseInOnPlayer();
											   ~CloseInOnPlayer();

		void									SetArgs		( Event *ev     );      
		void									Begin		( Actor &self   );		
		BehaviorReturnCode_t					Evaluate	( Actor &self   );
		void									End			( Actor &self   );
		virtual void							Archive		( Archiver &arc );

		void									setAnim		( const str &animName );
		void									setTorsoAnim( const str &animName );
		void									setDist		( float distance      );

	//-------------------------------------
	// Components
	//-------------------------------------
	private:      
		GotoEntity								_chase;

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		closeInOnPlayerStates_t					_state;
		EntityPtr								_player;
		Actor								   *_self;

};

inline void CloseInOnPlayer::setAnim( const str &animName )
{
	_anim = animName;
}

inline void CloseInOnPlayer::setTorsoAnim( const str &animName )
{
	_torsoAnim = animName;
}

inline void CloseInOnPlayer::setDist( float distance )
{
	_dist = distance;
}

inline void CloseInOnPlayer::Archive( Archiver &arc	)
{
	Behavior::Archive( arc );	 
   
   // Archive Parameters
	arc.ArchiveString ( &_anim );
	arc.ArchiveString ( &_torsoAnim );
	arc.ArchiveFloat  ( &_dist );

   // Archive Components
	arc.ArchiveObject ( &_chase );

   // Archive Member Variables
	ArchiveEnum					( _state, closeInOnPlayerStates_t);
	arc.ArchiveSafePointer		( &_player						);
	arc.ArchiveObjectPointer	( ( Class ** )&_self			);
}  

#endif /* CLOSE_IN_ON_PLAYER_HPP */
