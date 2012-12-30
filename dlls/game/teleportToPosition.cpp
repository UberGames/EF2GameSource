//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/teleportToPosition.cpp                     $
// $Revision:: 5                                                              $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// TeleportToPosition Behavior Implementation.
//	-- Currently, this is misnamed as a result of the transition to individual
//	   files for each behavior.  Additionally, this behavior needs much refactoring
//	   to make it more generalized and flexible
//
//	-- What it does RIGHT NOW
//	   The Behavior iterates through "position" flagged pathnodes.  It picks a random
//	   one ( based on a naming convention ) and uses that as a teleport position.
//	   It then checks if this new position will hold the actor.  If that is true, then 
//	   it plays the "start" animation.  When that animation is completed, it rechecks
//	   the spot -- If it's still good, then it sets the actor's origin to that spot
//	   and plays the "end" animation
//
//	-- Naming Convention
//	   2 of the parameters ( _teleportPositionName , _numberOfTeleportPositions )
//	   build up a naming convention for the behavior to use.  What this means is
//	   you will need to coordinate with the level designers to make sure that they
//	   name the pathnodes appropriately.  "WarpNode1" is a good example.  For every
//	   teleport position, they should increment the number at the end.  "WarpNode3" ,
//	   "WarpNode2", etc...  
//
//	   For the state machine, ( using this example ) you would pass "WarpNode" and 3
//	   for the count of possible positions.  The behaivor then picks a random integer
//	   up to the count number, and concatenates with the Name to get a random node.
//
//	ANIMATIONS:
//		Start Animation : Parameter
//		End Animation   : Parameter
//--------------------------------------------------------------------------------

#include "actor.h"
#include "player.h"
#include "weaputils.h"
#include "teleportToPosition.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, AnimatedTeleportToPosition, NULL )
	{
		{ &EV_Behavior_Args,			&AnimatedTeleportToPosition::SetArgs },
		{ NULL, NULL }
	};

//--------------------------------------------------------------
// Name:		SetArgs()
// Class:		AnimatedTeleportToPosition
//
// Description:	Sets Variables based on arguments inside the event
//
// Parameters:	Event *ev 
//
// Returns:		None
//--------------------------------------------------------------
void AnimatedTeleportToPosition::SetArgs( Event *ev )
{
	_teleportPositionName		 = ev->GetString( 1 );
	_numberOfTeleportPositions	 = ev->GetInteger( 2 );
	_startAnim					 = ev->GetString( 3 );
	_endAnim					 = ev->GetString( 4 );
}


//--------------------------------------------------------------
// Name:		Begin()
// Class:		AnimatedTeleportToPosition
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void AnimatedTeleportToPosition::Begin( Actor &self )
{
	_state = ANIM_TELEPORT_BEGIN;
}

//--------------------------------------------------------------
// Name:		Evaluate()
// Class:		AnimatedTeleportToPosition
//
// Description:	Update for this behavior -- called every server frame
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t AnimatedTeleportToPosition::Evaluate ( Actor &self )
{
	Vector dir;
	Vector angles;
	trace_t trace;
	str pathnode_name;	
	Vector teleport_position;
	Vector attack_position;
	float half_height;	

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	switch ( _state )
		{
		case ANIM_TELEPORT_BEGIN:

			// Get the pathnode name to teleport to
			pathnode_name = _teleportPositionName;
			pathnode_name += (int)G_Random( (float)_numberOfTeleportPositions ) + 1 ;

			// Find the path node
			_goal = thePathManager.FindNode( pathnode_name );

			if ( !_goal )
				{
				gi.WDPrintf( "Can't find position %s\n", pathnode_name.c_str() );
				SetFailureReason( "Unable to find a teleport position" );
				return BEHAVIOR_FAILED;
				}

			_state = ANIM_TELEPORT_START_ANIM;
		break;

		case ANIM_TELEPORT_START_ANIM:
			self.SetAnim( _startAnim , EV_Anim_Done , legs );
			_state = ANIM_TELEPORT_START_ANIMATING;
		break;

		case ANIM_TELEPORT_START_ANIMATING:
			if ( self.GetActorFlag( ACTOR_FLAG_ANIM_DONE ) )
				_state = ANIM_TELEPORT_TELEPORT;
		break;

		case ANIM_TELEPORT_TELEPORT:

			// Set the teleport position
			teleport_position = _goal->origin;

			// Kill anything at this position
			half_height = self.maxs.z / 2.0f;
			attack_position = teleport_position;
			attack_position.z += half_height;

			MeleeAttack( attack_position, attack_position, 10000.0f, &self, MOD_TELEFRAG, self.maxs.x, -half_height, half_height, 0 );

			// Test to see if we can fit at the new position
			trace = G_Trace( teleport_position + Vector( "0 0 64" ), self.mins, self.maxs, teleport_position - Vector( "0 0 128" ), &self, MASK_PATHSOLID, false, "TeleportToPosition" );	

			if ( trace.allsolid )
				{
				gi.WDPrintf( "Failed to teleport to %s\n", _goal->targetname.c_str() );
				SetFailureReason( "Trace at my designated teleport position return All Solid" );
				return BEHAVIOR_FAILED;
				}

			teleport_position = trace.endpos;
			self.setOrigin( teleport_position );
			self.NoLerpThisFrame();

			if ( currentEnemy )
				{
				dir = currentEnemy->origin - teleport_position;
				angles = dir.toAngles();

				angles[ROLL] = 0.0f;
				angles[PITCH] = 0.0f;

				self.setAngles( angles );
				}

			_state = ANIM_TELEPORT_END_ANIM;
		break;

		case ANIM_TELEPORT_END_ANIM:
			self.SetAnim( _endAnim , EV_Anim_Done , legs );
			_state = ANIM_TELEPORT_END_ANIMATING;
		break;

		case ANIM_TELEPORT_END_ANIMATING:
			if ( self.GetActorFlag( ACTOR_FLAG_ANIM_DONE ) )
			return BEHAVIOR_SUCCESS;
		break;
	}

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		End()
// Class:		AnimatedTeleportToPosition
//
// Description:	Ends this behavior -- cleans things up
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void AnimatedTeleportToPosition::End( Actor &self )
{
}


