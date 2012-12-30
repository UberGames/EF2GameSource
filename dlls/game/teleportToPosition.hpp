//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/teleportToPosition.h                           $
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
// TeleportToPosition Behavior Definition ( Behavior Name Needs To Be Changed )
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class AnimatedTeleportToPosition;

#ifndef __TELEPORT_TO_POSITION_H__
#define __TELEPORT_TO_POSITION_H__

#include "behavior.h"

//--------------------------------------------------
// We need to change the name of this behavior to more appropriately
// reflect the file name AND we need to set it up properly so that it
// doesn't HAVE to be animated... Basically we need to generalize this
// behavior as soon as possible
//----------------------------------------------------

//------------------------- CLASS ------------------------------
//
// Name:			AnimatedTeleportToPosition
// Base Class:		Behavior
//
// Description:		Teleports Actor to the player ( See .cpp for details )
//
// Method of Use:	State Machine or another behavior
//--------------------------------------------------------------
class AnimatedTeleportToPosition : public Behavior
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

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		str						_teleportPositionName;
		int						_numberOfTeleportPositions;
		str						_startAnim;
		str						_endAnim;


	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( AnimatedTeleportToPosition );

		void					SetArgs( Event *ev );
		void					Begin( Actor &self );
		BehaviorReturnCode_t	Evaluate( Actor &self );
		void					End( Actor &self );
		virtual void			Archive( Archiver &arc );

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:		
		animTeleportStates_t	_state;
		PathNodePtr				_goal;

	};

inline void AnimatedTeleportToPosition::Archive( Archiver &arc )
{
	Behavior::Archive( arc );

	//-------------------------------------
	// Archive Parameters
	//-------------------------------------
	arc.ArchiveString		( &_teleportPositionName	    );
	arc.ArchiveInteger		( &_numberOfTeleportPositions   );
	arc.ArchiveString		( &_startAnim				    );
	arc.ArchiveString		( &_endAnim				        );

	//-------------------------------------
	// Archive Member Variables
	//-------------------------------------
	ArchiveEnum				(  _state, animTeleportStates_t );
	arc.ArchiveSafePointer	( &_goal					    );
}



#endif /*__TELEPORT_TO_POSITION_H__ */
