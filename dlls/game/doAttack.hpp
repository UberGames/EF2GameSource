//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/doAttack.hpp                                   $
// $Revision:: 169                                                            $
//   $Author:: Bschofield                                                     $
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
// DoAttack Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class RotateToEntity;

#ifndef __DO_ATTACK_H__
#define __DO_ATTACK_H__

#include "behavior.h"
#include "rotateToEntity.hpp"

//------------------------- CLASS ------------------------------
//
// Name:          DoAttack
// Base Class:    Behavior
//
// Description:   Rotates the Actor towards its enemy then
//                plays the specified attack animation
//                This is good for actors that aren't using
//                "real" weapons and are relying on animations
//                to attack
//
// Method of Use: State machine or called from another behavior
//--------------------------------------------------------------
class DoAttack : public Behavior
	{
	public:  // States
		typedef enum
		{
		ATTACK_STATE_SETUP,
		ATTACK_STATE_ROTATE,
		ATTACK_STATE_START_ANIM,
		ATTACK_STATE_ANIMATING,
		ATTACK_STATE_COMPLETE,
		ATTACK_STATE_FAILED   
		} attackStates_t;

	private: // Parameters
		str								_anim;
		float							_turnspeed; 
		bool							_forceAttack;
		str								_rotateAnim;
      
	protected:
		void							_setupRotate    ( Actor &self );
		void							_rotate         ( Actor &self );
		void							_playAttackAnim ( Actor &self );

		bool							_canAttack      ( Actor &self );
		void							init			( Actor &self );

   public:
      CLASS_PROTOTYPE( DoAttack );

		void							SetArgs  ( Event *ev     );      
		void							AnimDone ( Event *ev     );
		void							Begin    ( Actor &self   );		
		BehaviorReturnCode_t			Evaluate ( Actor &self   );
		void							End      ( Actor &self   );
		virtual void					Archive  ( Archiver &arc );

		void							SetAnim      ( const str &animName );
		void							SetTurnSpeed ( float turnSpeed     );
		void							SetForceAttack ( bool force        );
		void							SetRotateAnim ( const str &animName );

	private:
		unsigned int                     _state;
		RotateToEntity                   _rotateBehavior;	  
	};


inline void DoAttack::SetAnim( const str &animName )
{
	_anim = animName;
}

inline void DoAttack::SetTurnSpeed( float turnSpeed )
{
	_turnspeed = turnSpeed;
}

inline void DoAttack::SetForceAttack( bool force )
{
	_forceAttack = force;
}

inline void DoAttack::SetRotateAnim( const str &animName )
{
	_rotateAnim = animName;
}

inline void DoAttack::Archive( Archiver &arc	)
{
	Behavior::Archive( arc );	    

   // Archive Parameters
	arc.ArchiveString		( &_anim            );
	arc.ArchiveFloat		( &_turnspeed       );
	arc.ArchiveBool			( &_forceAttack     );
	arc.ArchiveString		( &_rotateAnim      );

   // Archive Components

   // Archive Member Vars
	arc.ArchiveUnsigned		( &_state          );
	arc.ArchiveObject		( &_rotateBehavior );	
}

#endif /* __DO_ATTACK_H__ */
