//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/teleportToEntity.cpp                       $
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
// TeleportToEntity Behavior Implementation.
//	-- Currently, this is misnamed as a result of the transition to individual
//	   files for each behavior.  Additionally, this behavior needs much refactoring
//	   to make it more generalized and flexible
//
//	-- What it does RIGHT NOW
//	   The Behavior grabs the player's position, offsets it ( Preferring to go behind )
//	   It checks if this new position will hold the actor.  If that is true, then 
//	   it plays the "start" animation.  When that animation is completed, it rechecks
//	   the spot -- If it's still good, then it sets the actor's origin to that spot
//	   and plays the "end" animation
//
//	ANIMATIONS:
//		Start Animation : Parameter
//		End Animation   : Parameter
//--------------------------------------------------------------------------------

#include "actor.h"
#include "player.h"
#include "teleportToEntity.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, AnimatedTeleportToPlayer, NULL )
	{
		{ &EV_Behavior_Args, &AnimatedTeleportToPlayer::SetArgs },
		{ NULL, NULL			     }
	};


//--------------------------------------------------------------
// Name:		SetArgs()	
// Class:		AnimatedTeleportToPlayer
//
// Description:	Sets Variables based on arguments inside the event
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void AnimatedTeleportToPlayer::SetArgs( Event *ev )
{
	_startAnim = ev->GetString( 1 );
	_endAnim   = ev->GetString( 2 );
}

//--------------------------------------------------------------
// Name:		Begin()
// Class:		AnimatedTeleportToPlayer
//
// Description: Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void AnimatedTeleportToPlayer::Begin( Actor &self )
{
	_state = ANIM_TELEPORT_BEGIN;
}

//--------------------------------------------------------------
// Name:		Evaluate()
// Class:		AnimatedTeleportToPlayer
//
// Description:	Update for this behavior -- called every server frame
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t AnimatedTeleportToPlayer::Evaluate ( Actor &self )
{
	int current_position;	
	qboolean teleport_position_found;
	Vector new_position;
	int i;
	Vector dir;
	Vector angles;
	
	Player *player = NULL;
	Player *temp_player = NULL;
	// Make sure the player is alive and well
	for(i = 0; i < game.maxclients; i++)
		{
		player = GetPlayer(i);		
		
		// don't target while player is not in the game or he's in notarget
		if ( temp_player && !( temp_player->flags & FL_NOTARGET ) )
			{
			player = temp_player;
			break;
			}
		}

	if ( !player )
		return BEHAVIOR_SUCCESS;
	
	switch ( _state )
		{
		case ANIM_TELEPORT_BEGIN:
			// Default the teleport position to where we are now

			_teleportPosition = self.origin;
			teleport_position_found = false;

			// Always teleport BEHIND the player - - we don't want him to see us pop in.
			current_position = TELEPORT_BEHIND;

			// Test this position
			if ( testPosition( self, current_position, new_position, player, true ) )
				{
				_teleportPosition = new_position;
				teleport_position_found = true;
				}
			else
				{
				if ( testPosition( self, current_position, new_position, player, false ) )
					{
					_teleportPosition = new_position;
					teleport_position_found = true;
					}
				}

			if ( !teleport_position_found )
				return BEHAVIOR_FAILED;

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
			_teleportPosition = self.origin;
			teleport_position_found = false;

			// Always teleport BEHIND the player - - we don't want him to see us pop in.
			current_position = TELEPORT_BEHIND;

			// Test this position
			if ( testPosition( self, current_position, new_position, player, true ) )
				{
				_teleportPosition = new_position;
				teleport_position_found = true;
				}
			else
				{
				if ( testPosition( self, current_position, new_position, player, false ) )
					{
					_teleportPosition = new_position;
					teleport_position_found = true;
					}
				}

				if ( !teleport_position_found )
					{
					_state = ANIM_TELEPORT_END_ANIM;
					break;
					}

			self.setOrigin( _teleportPosition );
			self.NoLerpThisFrame();

			dir = player->origin - _teleportPosition;
			angles = dir.toAngles();

			angles[ROLL] = 0.0f;
			angles[PITCH] = 0.0f;

			self.setAngles( angles );
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
// Class:		AnimatedTeleportToPlayer
//
// Description:	Ends this behavior -- cleans things up
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void AnimatedTeleportToPlayer::End( Actor &self )
{
}


//--------------------------------------------------------------
// Name:		testPosition()
// Class:		AnimatedTeleportToPlayer
//
// Description:	Tests if the given position is valid for teleporting into
//
// Parameters:	Actor &self
//				int test_pos
//				Vector &good_position
//				Entity *player,
//				bool use_player_dir
//
// Returns:		true or false
//--------------------------------------------------------------
bool AnimatedTeleportToPlayer::testPosition( Actor &self, int test_pos, Vector &good_position, Entity* player, bool use_player_dir )
{
	Vector test_position;
	Vector player_angles;
	Vector player_forward;
	Vector player_left;
	trace_t trace;


	// Get the position to test
	test_position = player->origin;
	
	if ( use_player_dir )
		{
		// Get the player direction info

		player_angles = player->angles;
		player_angles.AngleVectors( &player_forward, &player_left );
		
		// Check Behind the Player
		test_position -= player_forward * 128.0f;
		}
	else
		{
		// Check Behind the Player
		test_position += Vector(-128, 0, 0);
		}

	// Final Tweaking
	test_position += Vector(0, 0, 64);

	// Test to see if we can fit at the new position

	trace = G_Trace( test_position, self.mins, self.maxs, test_position - Vector( "0 0 250" ), &self, self.edict->clipmask, false, "Teleport::TestPosition" );

	if ( trace.allsolid || trace.startsolid )
		return false;

	if ( trace.fraction == 1.0 )
		return false;

	// Make sure we can see the Player from this position

	/*if ( !self.IsEntityAlive( player ) || !self.sensoryPerception->CanSeeEntity( Vector( trace.endpos ), player , true , true ) )
			return false;*/


	// This is a good position
	good_position = trace.endpos;
	return true;
}


