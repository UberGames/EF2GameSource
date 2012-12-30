//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/teleportToEntity.h                               $
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
// TeleportToEntity Behavior Definition ( Behavior Name Needs To Be Changed )
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class AnimatedTeleportToPlayer;

#ifndef __TELEPORT_TO_ENTITY_H__
#define __TELEPORT_TO_ENTITY_H__

#include "behavior.h"

//--------------------------------------------------
// We need to change the name of this behavior to more appropriately
// reflect the file name AND we need to set it up properly so that it
// doesn't HAVE to be animated... Basically we need to generalize this
// behavior as soon as possible
//----------------------------------------------------

//------------------------- CLASS ------------------------------
//
// Name:			AnimatedTeleportToPlayer
// Base Class:		Behavior
//
// Description:		Teleports Actor to the player ( See .cpp for details )
//
// Method of Use:	State Machine or another behavior
//--------------------------------------------------------------
class AnimatedTeleportToPlayer : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:
		typedef enum
			{
			ANIM_TELEPORT_BEGIN,
			ANIM_TELEPORT_START_ANIM,
			ANIM_TELEPORT_START_ANIMATING,
			ANIM_TELEPORT_TELEPORT,
			ANIM_TELEPORT_END_ANIM,
			ANIM_TELEPORT_END_ANIMATING,
			} animTeleportStates_t;
		
		typedef enum
			{
			TELEPORT_BEHIND,
			TELEPORT_TOLEFT,
			TELEPORT_TORIGHT,
			TELEPORT_INFRONT,
			TELEPORT_NUMBER_OF_POSITIONS
			} animTeleportPositionModes_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
      str						_startAnim;
      str						_endAnim;

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:	
		bool					testPosition ( Actor &self, int test_pos, Vector &good_position, Entity* player, bool use_player_dir );


	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( AnimatedTeleportToPlayer );

		void					SetArgs  ( Event *ev      );		
		void					Begin    ( Actor &self    );
		
		BehaviorReturnCode_t	Evaluate ( Actor &self    );
		void					End      ( Actor &self    );
		virtual void			Archive  ( Archiver &arc );

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:		
		Vector					_teleportPosition;
		animTeleportStates_t	_state;
	};

inline void AnimatedTeleportToPlayer::Archive( Archiver &arc )
{
	Behavior::Archive ( arc );

	//-------------------------------------
	// Archive Parameters
	//-------------------------------------
	arc.ArchiveString (  &_startAnim                 );
	arc.ArchiveString (  &_endAnim                   );

	//-------------------------------------
	// Archive Member Variables
	//-------------------------------------
	arc.ArchiveVector ( &_teleportPosition           );
	ArchiveEnum       ( _state, animTeleportStates_t );
}



#endif /* __TELEPORT_TO_ENTITY_H__ */
