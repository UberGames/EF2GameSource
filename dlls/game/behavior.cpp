//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/behavior.cpp                                  $
// $Revision:: 135                                                            $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:11a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// DESCRIPTION:
// Behaviors used by the AI.
//

#include "_pch_cpp.h"
//#include "g_local.h"
#include "behavior.h"
#include "actor.h"
#include "doors.h"
#include "object.h"
#include "explosion.h"
#include "weaputils.h"
#include "player.h"

Event EV_Behavior_Args
	(
	"args",
	EV_CODEONLY,
	"SSSSSS",
	"token1 token2 token3 token4 token5 token6",
	"Gives the current behavior arguments."
	);
Event EV_Behavior_AnimDone
	(
	"animdone",
	EV_CODEONLY,
	NULL,
	NULL,
	"Tells the behavior that the current animation is done, "
	"it is not meant to be called from script."
	);
Event EV_PostureChanged_Completed
	(
	"posturechangecompleted",
	EV_CODEONLY,
	NULL,
	NULL,
	"Tells the behavior that the requested posture is active"
	);

Vector ChooseRandomDirection( Actor &self, const Vector &previousdir, float *time, int disallowcontentsmask, qboolean usepitch );

/****************************************************************************

  Behavior Class Definition

****************************************************************************/

CLASS_DECLARATION( Listener, Behavior, NULL )
	{
		{ NULL, NULL }
	};

Behavior::Behavior()
:	_controller( 0 ) // NULL
	{
	_self = NULL;
	}

void Behavior::ShowInfo
	(
	Actor &
	)

	{
   if ( movegoal )
      {
      gi.Printf( "movegoal: ( %f, %f, %f ) - '%s'\n",
         movegoal->origin.x, movegoal->origin.y, movegoal->origin.z, movegoal->targetname.c_str() );
      }
   else
      {
      gi.Printf( "movegoal: NULL\n" );
      }
	}

void Behavior::Begin
	(
	Actor &
	)

	{
	}

BehaviorReturnCode_t	Behavior::Evaluate
	(
	Actor &
	)

	{
	return BEHAVIOR_SUCCESS;
	}

void Behavior::End
	(
	Actor &
	)

	{
	}



/****************************************************************************
*****************************************************************************

  General behaviors

*****************************************************************************
****************************************************************************/



/****************************************************************************

  Idle Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Idle, NULL )
	{
		{ NULL, NULL }
	};

void Idle::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nnexttwitch : %f\n", nexttwitch );
	}

void Idle::Begin
	(
	Actor &self
	)

	{
	nexttwitch = level.time + 10.0f + G_Random( 30.0f );
	self.SetAnim( "idle", EV_Actor_NotifyBehavior );
	}

BehaviorReturnCode_t	Idle::Evaluate
	(
	Actor &self
	)

	{
	if ( self.enemyManager->GetCurrentEnemy() )
		{
		return BEHAVIOR_EVALUATING;
		}

	if ( nexttwitch < level.time )
		{
		self.chattime += 10.0f;
		self.AddStateFlag( STATE_FLAG_TWITCH );
		return BEHAVIOR_EVALUATING;
		}
	else
		{
		//self.Chatter( "snd_idle", 1 );		
		}

	return BEHAVIOR_EVALUATING;
	}

void Idle::End
	(
	Actor &
	)

	{
	
	}

/****************************************************************************

  Pain Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Pain, NULL )
	{
		{ &EV_Behavior_AnimDone,	&Pain::AnimDone },
		{ NULL, NULL }
	};

void Pain::SetPainAnim
	(
	Actor &self,
	int new_pain_type,
	int new_anim_number
	)

	{
	str anim_name;

	// Determine which pain type to play

	if ( new_pain_type == PAIN_SMALL )
		anim_name = "pain_small";
	else
		anim_name = "pain";

	// Try to find the correct anim to play

	anim_name += new_anim_number;

	if ( !self.animate->HasAnim( anim_name.c_str() ) )
		{
		if ( new_pain_type == PAIN_SMALL )
			anim_name = "pain_small1";
		else
			anim_name = "pain1";

		pain_anim_number = 1;
		}

	anim_done = false;

	// Play the animation if we can

	if ( !self.animate->HasAnim( anim_name.c_str() ) )
		anim_done = true;
	else
		self.SetAnim( anim_name.c_str(), EV_Actor_NotifyBehavior );
	}

int Pain::GetNumberOfPainAnims
	(
	Actor &self,
	int new_pain_type
	)

	{
	str anim_name;
	str real_anim_name;
	int i;


	// Determine base animation name

	if ( new_pain_type == PAIN_SMALL )
		anim_name = "pain_small";
	else
		anim_name = "pain";

	// Find how many pain animations we have

	for( i = 1 ; i < 10 ; i++ )
		{
		real_anim_name = anim_name + i;

		if ( !self.animate->HasAnim( real_anim_name.c_str() ) )
			return( i - 1 );
		}

	return 9;
	}

void Pain::Begin
	(
	Actor &self
	)

	{
	str anim_name;
	int num_pain_anims;

	num_pain_anims = GetNumberOfPainAnims( self, self.pain_type );

	pain_anim_number = (int)G_Random( num_pain_anims ) + 1;

	// Figure out which type of pain to do

	if ( self.pain_type == PAIN_SMALL )
		SetPainAnim( self, PAIN_SMALL, pain_anim_number );
	else
		SetPainAnim( self, PAIN_BIG, pain_anim_number );

	current_pain_type = self.pain_type;
	number_of_pains   = 1;

	// Make sure we don't have pain any more

	self.state_flags &= ~STATE_FLAG_SMALL_PAIN;
	self.state_flags &= ~STATE_FLAG_IN_PAIN;

	max_pains = (int)G_Random( 4 ) + 4;
	}

void Pain::AnimDone
	(
	Event *
	)
	{
	anim_done = true;
	}

BehaviorReturnCode_t	Pain::Evaluate
	(
	Actor &self
	)

	{
	str anim_name;

	if ( self.state_flags & STATE_FLAG_SMALL_PAIN )
		{
		// See if we should play another pain animation

		if ( ( self.means_of_death != MOD_FIRE ) && ( self.means_of_death != MOD_ON_FIRE ) && ( self.means_of_death != MOD_FIRE_BLOCKABLE ) )
			{
			if ( ( self.pain_type == PAIN_SMALL ) && ( current_pain_type == PAIN_SMALL ) && ( number_of_pains < max_pains ) )
				{
				pain_anim_number++;

				number_of_pains++;

				SetPainAnim( self, PAIN_SMALL, pain_anim_number );
				}
			else if ( self.pain_type == PAIN_BIG )
				{
				pain_anim_number++;

				current_pain_type = PAIN_BIG;

				SetPainAnim( self, PAIN_BIG, pain_anim_number );
				}
			}

		// Reset pain stuff

		self.state_flags &= ~STATE_FLAG_SMALL_PAIN;
		self.state_flags &= ~STATE_FLAG_IN_PAIN;
		}

	// If the pain animation has finished, then we are done

	if ( anim_done )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void Pain::End
	(
	Actor &self
	)

	{
	self.bullet_hits = 0;
	}

/****************************************************************************

  Watch Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Watch, NULL )
	{
		{ &EV_Behavior_Args,			&Watch::SetArgs },
		{ NULL, NULL }
	};

Watch::Watch()
	{
	turn_speed = 360.0f;
	old_turn_speed = 0.0f;
	}

void Watch::SetArgs
	(
	Event *ev
	)

	{
	turn_speed = ev->GetFloat( 1 );
	}

void Watch::Begin
	(
	Actor &self
	)

	{
	ent_to_watch = self.enemyManager->GetCurrentEnemy();

	old_turn_speed = self.movementSubsystem->getTurnSpeed();
	self.movementSubsystem->setTurnSpeed( turn_speed );

	Vector test;
	Vector testAng;

	test = self.movementSubsystem->getMoveDir();
	testAng = test.toAngles();
	}

BehaviorReturnCode_t	Watch::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;

	Vector testAng;
	dir = self.movementSubsystem->getMoveDir();
	testAng = dir.toAngles();

	if ( !ent_to_watch )
		return BEHAVIOR_SUCCESS;

	if ( self.IsEntityAlive( ent_to_watch ) )
		{		
		dir = self.movementSubsystem->getMoveDir();

		testAng = dir.toAngles();

		self.movementSubsystem->Accelerate( 
			self.movementSubsystem->SteerTowardsPoint(ent_to_watch->centroid, vec_zero, dir, 1.0f)
			);
		}

	return BEHAVIOR_EVALUATING;
	}

void Watch::End
	(
	Actor &self
	)

	{
	self.movementSubsystem->setTurnSpeed( old_turn_speed );	
	}

/****************************************************************************

  Turn Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Turn, NULL )
	{
		{ &EV_Behavior_Args,			&Turn::SetArgs },
		{ NULL, NULL }
	};

Turn::Turn()
	{
	turn_speed = 5;
	}

void Turn::SetArgs
	(
	Event *ev
	)

	{
	turn_speed = ev->GetFloat( 1 );
	}

void Turn::Begin
	(
	Actor &
	)

	{
	}

BehaviorReturnCode_t	Turn::Evaluate
	(
	Actor &self
	)

	{
	self.angles[YAW] += turn_speed;

	self.setAngles( self.angles );

	return BEHAVIOR_EVALUATING;
	}

void Turn::End
	(
	Actor &
	)

	{
	}

/****************************************************************************

  Aim Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Aim, NULL )
	{
		{ NULL, NULL }
	};

void Aim::SetTarget
	(
	Entity *ent
	)

	{
	target = ent;
	}

void Aim::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nseek:\n" );
   if ( target )
      {
      gi.Printf( "\ntarget : #%d '%s'\n", target->entnum, target->targetname.c_str() );
      }
   else
      {
      gi.Printf( "\ntarget : NULL\n" );
      }
	}

void Aim::Begin
	(
	Actor &
	)

	{
	}

BehaviorReturnCode_t	Aim::Evaluate
	(
	Actor &self
	)

	{
   Vector dir;
   Vector ang;
   Vector pos;

	if ( !target )
		{
		return BEHAVIOR_SUCCESS;
		}

   //
   // get my gun pos
   //

	// Fixme ?
   //pos = self.GunPosition();
   //ang = self.MyGunAngles( pos, false );
	pos = self.centroid;
	ang = self.angles;
   ang.AngleVectors( &dir, NULL, NULL );
	Vector newSteeringDirection=self.movementSubsystem->SteerTowardsPoint(target->centroid, target->velocity, dir, 1400 + ( skill->value * 600 ));
	self.movementSubsystem->Accelerate( newSteeringDirection	);
	//self.Accelerate( seek.steeringforce );
	if ( newSteeringDirection.y < 0.25f )
		{
		// dead on
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING;
	}

void Aim::End
	(
	Actor &
	)

	{
	}

/****************************************************************************

  TurnTo Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, TurnTo, NULL )
	{
		{ &EV_Behavior_AnimDone,	&TurnTo::AnimDone },
		{ NULL, NULL }
	};

TurnTo::TurnTo()
	{
	dir = Vector( 1.0f, 0.0f, 0.0f );
	mode = 0;
	ent = NULL;
   yaw = 0;
	anim_done = true;
	useTurnAnim = true;
	_useAnims = true;
	}

void TurnTo::SetDirection
	(
	float new_yaw
	)

	{
	Vector ang;

	ang = Vector( 0.0f, new_yaw, 0.0f );
	yaw = anglemod( new_yaw );
	ang.AngleVectors( &dir, NULL, NULL );
	dir *= 100.0f;
	mode = 1;
	}

void TurnTo::useAnims( bool useAnims )
{
	_useAnims = useAnims;
}

void TurnTo::SetTarget
	(
	Entity *target_ent
	)

	{
	ent = target_ent;
	mode = 2;
	}

void TurnTo::AnimDone
	(
	Event *
	)

	{
	anim_done = true;
	}

void TurnTo::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nseek:\n" );

   if ( ent )
      {
      gi.Printf( "\nent: #%d '%s'\n", ent->entnum, ent->targetname.c_str() );
      }
   else
      {
      gi.Printf( "\nent: NULL\n" );
      }

   gi.Printf( "dir: ( %f, %f, %f )\n", dir.x, dir.y, dir.z );
   gi.Printf( "yaw: %f\n", yaw );
   gi.Printf( "mode: %d\n", mode );
	}

void TurnTo::Begin( Actor &self )
{
	extraFrames = 0;
	
	if ( _useAnims )
	{
		self.SetAnim( "idle" );
	}
}

BehaviorReturnCode_t	TurnTo::Evaluate( Actor &self )
{
	Vector delta;
	float ang;
	str anim_name;
	
	Vector targetPosition;
	Vector targetVelocity;
	
	if ( anim_done )
	{
		if ( _useAnims )
		{
			self.SetAnim( "idle" );
		}
		anim_done = false;
	}
	
	switch( mode )
	{
	case 1 :
		ang = angledist( yaw - self.angles.yaw() );
		if ( fabs( ang ) < 1 )
		{
			self.movementSubsystem->Accelerate( Vector( 0.0f, ang, 0.0f ) );
			
			// If turned all the way wait for animation to finish
			
			if ( anim_done || ( self.animname == "idle" ) || !_useAnims )
				return BEHAVIOR_SUCCESS;
			else
				return BEHAVIOR_EVALUATING;
		}
		
		targetPosition = self.origin + dir ;
		targetVelocity = vec_zero ;
		break;
		
	case 2 :
		if ( !ent )
		{
			return BEHAVIOR_SUCCESS;
		}
		
		delta = ent->origin - self.origin;
		yaw = delta.toYaw();
		
		targetPosition =  ent->origin;
		targetVelocity = vec_zero;
		break;
		
	default :
		return BEHAVIOR_SUCCESS;
	}
	
	Vector newSteeringForce = 
		self.movementSubsystem->SteerTowardsPoint(
		targetPosition, 
		targetVelocity, 
		self.movementSubsystem->getMoveDir(), 
		self.movementSubsystem->getMoveSpeed()
		);
	
	if ( _useAnims )
	{
		if ( extraFrames < 1 )
		{				
			if ( useTurnAnim )
			{
				if ( newSteeringForce[YAW] > 1.0f )
				{
					anim_name = "turn_left";
					anim_done = false;
				}
				else if ( newSteeringForce[YAW] < -1.0f )
				{
					anim_name = "turn_right";
					anim_done = false;
				}
				else if ( anim_done )
				{
					anim_name = "idle";
				}
				else
				{
					anim_name = self.animname;
				}
			}
			else
			{
				anim_name = "idle";
			}
			
			if ( gi.Anim_NumForName( self.edict->s.modelindex, anim_name.c_str() ) != -1 )
			{
				if ( anim_name != self.animname )
					self.SetAnim( anim_name.c_str(), EV_Actor_NotifyBehavior );
			}
			else
			{
				anim_done = true;
			}
		}
	}
	
	extraFrames++;
	if ( extraFrames > 4 || anim_name == "idle" )
	{
		extraFrames = 5;
		self.movementSubsystem->Accelerate( newSteeringForce );	
	}
	
	return BEHAVIOR_EVALUATING;
}

void TurnTo::End( Actor &self )
{
	if ( _useAnims )
	{
		self.SetAnim( "idle" );
	}

	self.movementSubsystem->setMoveSpeed( 1.0 );
}

void TurnTo::SetUseTurnAnim( bool useAnim )
	{
	useTurnAnim = useAnim;
	}

/****************************************************************************

  RotateToEnemy Class Definition -- Primarily a convience wrapper for TurnTo

****************************************************************************/
CLASS_DECLARATION( Behavior, RotateToEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&RotateToEnemy::SetArgs },
		{ NULL, NULL }
	};

void RotateToEnemy::SetArgs
   (
	Event *ev
	)

	{
	if (ev->NumArgs() > 0 )
		anim = ev->GetString(1);
	else
		anim = "idle";

	if (ev->NumArgs() > 1 )
		turnSpeed = ev->GetFloat(2);
	else
		turnSpeed = 10;
	}

void RotateToEnemy::Begin
   (
	Actor &
	)

	{

	}

BehaviorReturnCode_t RotateToEnemy::Evaluate
   (
	Actor &self
	)

	{
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	Vector dir = currentEnemy->origin - self.origin;
	float yaw_diff = AngleNormalize360( self.origin.toYaw() - dir.toYaw() );
	
	if (yaw_diff < 180.0f )
		self.angles[YAW]+=turnSpeed;

	if (yaw_diff > 180.0f )
		self.angles[YAW]-=turnSpeed;

	self.setAngles(self.angles);

	if (self.sensoryPerception->InFOV(currentEnemy) )
		return BEHAVIOR_SUCCESS;
	else
		return BEHAVIOR_EVALUATING;
	}

void RotateToEnemy::End
   (
	Actor &
	)

	{

	}

/****************************************************************************

  HeadWatch Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, HeadWatch, NULL )
	{
		{ &EV_Behavior_Args,	&HeadWatch::SetArgs },
		{ NULL, NULL }
	};

HeadWatch::HeadWatch()
	{
	max_speed = 15;
	forever = true;
	usingEyes = false;
	}

void HeadWatch::SetArgs
	(
	Event *ev
	)

	{
	ent_to_watch = ev->GetEntity( 1 );

	if ( ev->NumArgs() > 1 )
		max_speed = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		forever = ev->GetBoolean( 3 );
	}


void HeadWatch::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void HeadWatch::Begin
	(
	Actor &self
	)

	{
	self.SetControllerTag( ACTOR_HEAD_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Head" ) );

	current_head_angles = self.GetControllerAngles( ACTOR_HEAD_TAG );

	self.SetActorFlag( ACTOR_FLAG_TURNING_HEAD, true );
	}

BehaviorReturnCode_t	HeadWatch::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector angles;
	int tag_num;	
	Vector tag_pos;
	Vector watch_tag_pos;
	Vector angles_diff;
	Vector watch_position;
	Actor *act = NULL;
	Vector change;
	

	if ( ent_to_watch )
		{
		tag_num = gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Head" );

		if ( tag_num < 0 )
			return BEHAVIOR_SUCCESS;

		self.GetTag( "Bip01 Head", &tag_pos );

		if ( ent_to_watch->isSubclassOf( Actor ) )
			act = (Actor *)(Entity *)ent_to_watch;

		if ( act && ( act->watch_offset != vec_zero ) )
			{
			MatrixTransformVector( act->watch_offset, ent_to_watch->orientation, watch_position );
			watch_position += ent_to_watch->origin;
			}
		else
			{			
			tag_num = gi.Tag_NumForName( ent_to_watch->edict->s.modelindex, "Bip01 Head" );
			
			if ( tag_num < 0 )
				watch_position = ent_to_watch->centroid;
			else
				{
				ent_to_watch->GetTag( "Bip01 Head", &watch_tag_pos );
				watch_position = watch_tag_pos;
				}
			}

		dir = watch_position - tag_pos;
		angles = dir.toAngles();

		angles_diff = angles - self.angles;
		}
	else
		{
		angles_diff = vec_zero;
		}

	angles_diff[YAW]   = AngleNormalize180( angles_diff[YAW] );
	angles_diff[PITCH] = AngleNormalize180( angles_diff[PITCH] );

	if (usingEyes)
		{
		if (angles_diff[YAW] > (self.minEyeYawAngle + (self.minEyeYawAngle * .5f )) && angles_diff[YAW] < (self.maxEyeYawAngle + (self.maxEyeYawAngle * .5f )))
			angles_diff[YAW] = 0.0f;
		if (angles_diff[PITCH] > self.minEyePitchAngle && angles_diff[PITCH] < self.maxEyePitchAngle )
		   angles_diff[PITCH] = 0.0f;
		}

	// Make sure we don't turn neck too far
	
	if ( angles_diff[YAW] < -80.0f )
		angles_diff[YAW] = -80.0f;
	else if ( angles_diff[YAW] > 80.0f )
		angles_diff[YAW] = 80.0f;

	if ( angles_diff[PITCH] < -45.0f )
		angles_diff[PITCH] = -45.0f;
	else if ( angles_diff[PITCH] > 45.0f )
		angles_diff[PITCH] = 45.0f;

	angles_diff[ROLL] = 0.0f;

	// Make sure we don't change our head angles too much at once

	change = angles_diff - current_head_angles;
	float absYaw = fabs( change[ YAW ] );
	float absPitch = fabs( change[ PITCH ] );

	if( ( absYaw < fEpsilon() ) && ( absPitch < fEpsilon() ) )
	{
		if( forever )
		{
			self.SetControllerAngles( ACTOR_HEAD_TAG, current_head_angles );
			return BEHAVIOR_EVALUATING;
		}
		else
			return BEHAVIOR_SUCCESS;
	}

	//change = vec_zero;

	bool isFinished = true;
	if( absYaw >= absPitch )
	{
		float ratioPitchToYaw = change[ PITCH ] / change[ YAW ];

		if( change[ YAW ] > max_speed )
		{
			change[ YAW ] = max_speed;
			isFinished = false;
		}
		else if( change[ YAW ] < -max_speed )
		{
			change[ YAW ] = -max_speed;
			isFinished = false;
		}
		change[ PITCH ] = change[ YAW ] * ratioPitchToYaw;
	}
	else if( absPitch > absYaw )
	{
		float ratioYawToPitch = change[ YAW ] / change[ PITCH ];

		if( change[ PITCH ] > max_speed )
		{
			change[ PITCH ] = max_speed;
			isFinished = false;
		}
		else if( change[ PITCH ] < -max_speed )
		{
			change[ PITCH ] = -max_speed;
			isFinished = false;
		}
		change[ YAW ] = change[ PITCH ] * ratioYawToPitch;
	}

	angles_diff = current_head_angles + change;
	angles_diff[ ROLL ] = 0.0f;

/*
	if ( change[YAW] > max_speed )
		angles_diff[YAW] = current_head_angles[YAW] + max_speed;
	else if ( change[YAW] < -max_speed )
		angles_diff[YAW] = current_head_angles[YAW] - max_speed;

	if ( change[PITCH] > max_speed )
		angles_diff[PITCH] = current_head_angles[PITCH] + max_speed;
	else if ( change[PITCH] < -max_speed )
		angles_diff[PITCH] = current_head_angles[PITCH] - max_speed;
*/
	self.SetControllerAngles( ACTOR_HEAD_TAG, angles_diff );
	self.real_head_pitch = angles_diff[PITCH];

	current_head_angles = angles_diff;	
	
	if ( !ent_to_watch && ( current_head_angles == vec_zero ) )
		return BEHAVIOR_SUCCESS;

//	if ( !forever && ( change[YAW] < max_speed ) && ( change[YAW] > -max_speed ) && ( change[PITCH] < max_speed ) && ( change[PITCH] > -max_speed ) )
	if( !forever && isFinished )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void HeadWatch::useEyes 
   (
	qboolean moveEyes
	)

	{
	usingEyes = moveEyes;
	}

void HeadWatch::End
	(
	Actor &self
	)

	{
	// Snap head back into position if we have lost our target or we are doing a resethead

	self.SetControllerAngles( ACTOR_HEAD_TAG, vec_zero );
	self.real_head_pitch = 0;

	self.SetActorFlag( ACTOR_FLAG_TURNING_HEAD, false );
	}



/****************************************************************************

  HeadWatchEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, HeadWatchEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&HeadWatchEnemy::SetArgs },
		{ NULL, NULL }
	};

HeadWatchEnemy::HeadWatchEnemy()
	{
	max_speed = 20;
	forever = true;
	usingEyes = false;
	threshold = 0;
	}

void HeadWatchEnemy::SetArgs
	(
	Event *ev
	)

	{
	max_speed = ev->GetFloat( 1 );

	if ( ev->NumArgs() > 1 )
		forever = ev->GetBoolean( 2 );

	if ( ev->NumArgs() > 2 )
		threshold = ev->GetFloat( 3 );
	}


void HeadWatchEnemy::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void HeadWatchEnemy::Begin
	(
	Actor &self
	)

	{
	self.SetControllerTag( ACTOR_HEAD_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Head" ) );

	current_head_angles = self.GetControllerAngles( ACTOR_HEAD_TAG );

	self.SetActorFlag( ACTOR_FLAG_TURNING_HEAD, true );

	ent_to_watch = self.enemyManager->GetCurrentEnemy();
	}

BehaviorReturnCode_t	HeadWatchEnemy::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector angles;
	int tag_num;	
	Vector tag_pos;
	Vector watch_tag_pos;
	Vector angles_diff;
	Vector watch_position;
	Actor *act = NULL;
	Vector change;


   // Get our Torso Angles
   self.SetControllerTag( ACTOR_TORSO_TAG , gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Spine1" ) );
   current_torso_angles = self.GetControllerAngles( ACTOR_TORSO_TAG );

   //Reset our Controller Tag
   self.SetControllerTag( ACTOR_HEAD_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Head" ) );


   if ( !ent_to_watch )
      {
      ent_to_watch = self.enemyManager->GetCurrentEnemy();
      }

	if ( ent_to_watch )
		{
		tag_num = gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Head" );

		if ( tag_num < 0 )
			return BEHAVIOR_SUCCESS;

		self.GetTag( "Bip01 Head", &tag_pos );

		if ( ent_to_watch->isSubclassOf( Actor ) )
			act = (Actor *)(Entity *)ent_to_watch;

		if ( act && ( act->watch_offset != vec_zero ) )
			{
			MatrixTransformVector( act->watch_offset, ent_to_watch->orientation, watch_position );
			watch_position += ent_to_watch->origin;
			}
		else
			{			
			tag_num = gi.Tag_NumForName( ent_to_watch->edict->s.modelindex, "Bip01 Head" );
			
			if ( tag_num < 0 )
				watch_position = ent_to_watch->centroid;
			else
				{
				ent_to_watch->GetTag( "Bip01 Head", &watch_tag_pos );
				watch_position = watch_tag_pos;
				}
			}

		dir = watch_position - tag_pos;
		angles = dir.toAngles();

		angles_diff = angles - self.angles;
		}
	else
		{
		angles_diff = vec_zero;
		}

	angles_diff[YAW]   = AngleNormalize180( angles_diff[YAW] );
	angles_diff[PITCH] = AngleNormalize180( angles_diff[PITCH] );

	float yaw_change = angles_diff[YAW];
	float pitch_change = angles_diff[PITCH];

	if ( threshold && ( yaw_change < threshold ) && ( yaw_change > -threshold ) && ( pitch_change < threshold ) && ( pitch_change > -threshold ) )
		{
		if ( forever )
			return BEHAVIOR_EVALUATING;
		else
			return BEHAVIOR_SUCCESS;
		}

	if (usingEyes)
		{
		if (angles_diff[YAW] > (self.minEyeYawAngle + (self.minEyeYawAngle * .5f )) && angles_diff[YAW] < (self.maxEyeYawAngle + (self.maxEyeYawAngle * .5f )))
			angles_diff[YAW] = 0.0f;
		if (angles_diff[PITCH] > self.minEyePitchAngle && angles_diff[PITCH] < self.maxEyePitchAngle )
		   angles_diff[PITCH] = 0.0f;
		}

	// Make sure we don't turn neck too far
	
	if ( angles_diff[YAW] < -80.0f )
		angles_diff[YAW] = -80.0f;
	else if ( angles_diff[YAW] > 80.0f )
		angles_diff[YAW] = 80.0f;

	if ( angles_diff[PITCH] < -45.0f )
		angles_diff[PITCH] = -45.0f;
	else if ( angles_diff[PITCH] > 45.0f )
		angles_diff[PITCH] = 45.0f;

	angles_diff[ROLL] = 0.0f;

	// Make sure we don't change our head angles too much at once

   //current_head_angles[YAW] = current_head_angles[YAW] - current_torso_angles[YAW];
	change = angles_diff - current_head_angles;

	if ( change[YAW] > max_speed )
		angles_diff[YAW] = current_head_angles[YAW] + max_speed;
	else if ( change[YAW] < -max_speed )
		angles_diff[YAW] = current_head_angles[YAW] - max_speed;

	if ( change[PITCH] > max_speed )
		angles_diff[PITCH] = current_head_angles[PITCH] + max_speed;
	else if ( change[PITCH] < -max_speed )
		angles_diff[PITCH] = current_head_angles[PITCH] - max_speed;

   Vector FinalAngles;
   FinalAngles = angles_diff;
   FinalAngles[YAW] = angles_diff[YAW] - current_torso_angles[YAW];
   
	//self.SetControllerAngles( ACTOR_HEAD_TAG, angles_diff );
   self.SetControllerAngles( ACTOR_HEAD_TAG, FinalAngles );
	self.real_head_pitch = angles_diff[PITCH];

	current_head_angles = angles_diff;
	
   /*
	if ( !ent_to_watch && ( current_head_angles == vec_zero ) )
		return false;
   */

  	if ( !ent_to_watch )
		return BEHAVIOR_EVALUATING;

	if ( !forever && ( change[YAW] < max_speed ) && ( change[YAW] > -max_speed ) && ( change[PITCH] < max_speed ) && ( change[PITCH] > -max_speed ) )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void HeadWatchEnemy::useEyes 
   (
	qboolean moveEyes
	)

	{
	usingEyes = moveEyes;
	}

void HeadWatchEnemy::End
	(
	Actor &self
	)

	{
	// Snap head back into position if we have lost our target or we are doing a resethead

	self.SetControllerAngles( ACTOR_HEAD_TAG, vec_zero );
	self.real_head_pitch = 0;

	self.SetActorFlag( ACTOR_FLAG_TURNING_HEAD, false );
	}
	
	
/****************************************************************************

  EyeWatch Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, EyeWatch, NULL )
	{
		{ &EV_Behavior_Args,			&EyeWatch::SetArgs },
		{ NULL, NULL }
	};

EyeWatch::EyeWatch()
	{
	ent_to_watch = NULL;
	max_speed = 10;
	forever = true;
	threshold = 0;
	}

void EyeWatch::SetArgs
	(
	Event *ev
	)

	{
	
	ent_to_watch = ev->GetEntity( 1 );

	if ( ev->NumArgs() > 1 )
		max_speed = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		forever = ev->GetBoolean( 3 );

	if ( ev->NumArgs() > 3 )
		threshold = ev->GetFloat( 4 );
	}


void EyeWatch::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void EyeWatch::Begin
	(
	Actor &self
	)

	{
	self.SetControllerTag( ACTOR_LEYE_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Face eyeballL" ) );
	self.SetControllerTag( ACTOR_REYE_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Face eyeballR" ) );
	

	current_left_eye_angles = self.GetControllerAngles( ACTOR_LEYE_TAG );
	current_right_eye_angles = self.GetControllerAngles( ACTOR_REYE_TAG );

	self.SetActorFlag( ACTOR_FLAG_MOVING_EYES, true );
	}

BehaviorReturnCode_t	EyeWatch::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector angles;
	int tag_num;		
	Vector tag_pos;
	Vector watch_tag_pos;
	Vector angles_diff;
	Vector watch_position;
	Actor *act = NULL;
	Vector change;

	if ( ent_to_watch )
		{
		tag_num = gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Head" );
		
		if ( tag_num < 0 )
			return BEHAVIOR_SUCCESS;

		self.GetTag( "Bip01 Head", &tag_pos );

		if ( ent_to_watch->isSubclassOf( Actor ) )
			act = (Actor *)(Entity *)ent_to_watch;

		if ( act && ( act->watch_offset != vec_zero ) )
			{
			MatrixTransformVector( act->watch_offset, ent_to_watch->orientation, watch_position );
			watch_position += ent_to_watch->origin;
			}
		else
			{			
			tag_num = gi.Tag_NumForName( ent_to_watch->edict->s.modelindex, "Bip01 Head" );
			
			if ( tag_num < 0 )
				watch_position = ent_to_watch->centroid;
			else
				{
				ent_to_watch->GetTag( "Bip01 Head", &watch_tag_pos );
				watch_position = watch_tag_pos;
				}
			}

		dir = watch_position - tag_pos;
		if ( dir.length() < 32.0f )
			{
			if ( !forever )
				return BEHAVIOR_SUCCESS;
			else 
			   return BEHAVIOR_EVALUATING;
			}

		angles = dir.toAngles();

		angles_diff = angles - self.angles;		
		}
	else
		{
		angles_diff = vec_zero;		
		}

	angles_diff[YAW]   = AngleNormalize180( angles_diff[YAW] );	
	
	// Make sure we don't turn eyes too far
	if ( angles_diff[YAW] < self.minEyeYawAngle )
		angles_diff[YAW] = self.minEyeYawAngle ;
	else if ( angles_diff[YAW] > self.maxEyeYawAngle  )
		angles_diff[YAW] = self.maxEyeYawAngle;

	if ( angles_diff[PITCH] < self.minEyePitchAngle  )
		angles_diff[PITCH] = self.minEyePitchAngle ;
	else if ( angles_diff[PITCH] > self.maxEyePitchAngle  )
		angles_diff[PITCH] = self.maxEyePitchAngle;

	angles_diff[ROLL] = 0;


	// Make sure we don't change our eye angles too much at once

	change = angles_diff - current_left_eye_angles;	

	
	// check left eye -- both eyes will be the same
	if ( change[YAW] > max_speed )
		angles_diff[YAW] = current_left_eye_angles[YAW] + max_speed;
	else if ( change[YAW] < -max_speed )
		angles_diff[YAW] = current_left_eye_angles[YAW] - max_speed;

	if ( change[PITCH] > max_speed )
		angles_diff[PITCH] = current_left_eye_angles[PITCH] + max_speed;
	else if ( change[PITCH] < -max_speed )
		angles_diff[PITCH] = current_left_eye_angles[PITCH] - max_speed;

	self.SetControllerAngles( ACTOR_LEYE_TAG, angles_diff );
	self.SetControllerAngles( ACTOR_REYE_TAG, angles_diff );

	current_left_eye_angles = angles_diff;
	current_right_eye_angles = angles_diff;

	
	if ( !ent_to_watch && ( current_left_eye_angles == vec_zero ) )
		return BEHAVIOR_SUCCESS;


	if ( !forever && ( change[YAW] < max_speed ) && ( change[YAW] > -max_speed ) &&
			( change[PITCH] < max_speed ) && ( change[PITCH] > -max_speed ) )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void EyeWatch::End
	(
	Actor &self
	)

	{
	// Snap head back into position if we have lost our target or we are doing a resethead

	self.SetControllerAngles( ACTOR_LEYE_TAG, vec_zero );
	self.SetControllerAngles( ACTOR_REYE_TAG, vec_zero );

	self.SetActorFlag( ACTOR_FLAG_MOVING_EYES, false );
	}




/****************************************************************************

  EyeWatchEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, EyeWatchEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&EyeWatchEnemy::SetArgs },
		{ NULL, NULL }
	};

EyeWatchEnemy::EyeWatchEnemy()
	{
	ent_to_watch = NULL;
	max_speed = 10;
	forever = true;
	threshold = 0;
	}

void EyeWatchEnemy::SetArgs
	(
	Event *ev
	)

	{
	
	max_speed = ev->GetFloat( 1 );

	if ( ev->NumArgs() > 1 )
		forever = ev->GetBoolean( 2 );

	if ( ev->NumArgs() > 2 )
		threshold = ev->GetFloat( 3 );
	}


void EyeWatchEnemy::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void EyeWatchEnemy::Begin
	(
	Actor &self
	)

	{
	self.SetControllerTag( ACTOR_LEYE_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Face eyeballL" ) );
	self.SetControllerTag( ACTOR_REYE_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Face eyeballR" ) );
	

	current_left_eye_angles = self.GetControllerAngles( ACTOR_LEYE_TAG );
	current_right_eye_angles = self.GetControllerAngles( ACTOR_REYE_TAG );

	self.SetActorFlag( ACTOR_FLAG_MOVING_EYES, true );
	}

BehaviorReturnCode_t	EyeWatchEnemy::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector angles;
	int tag_num;		
	Vector tag_pos;
	Vector watch_tag_pos;
	Vector angles_diff;
	Vector watch_position;
	Actor *act = NULL;
	Vector change;

	ent_to_watch = self.enemyManager->GetCurrentEnemy();

	if ( ent_to_watch )
		{
		tag_num = gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Head" );
		
		if ( tag_num < 0 )
			return BEHAVIOR_SUCCESS;

		self.GetTag( "Bip01 Head", &tag_pos );

		if ( ent_to_watch->isSubclassOf( Actor ) )
			act = (Actor *)(Entity *)ent_to_watch;

		if ( act && ( act->watch_offset != vec_zero ) )
			{
			MatrixTransformVector( act->watch_offset, ent_to_watch->orientation, watch_position );
			watch_position += ent_to_watch->origin;
			}
		else
			{			
			tag_num = gi.Tag_NumForName( ent_to_watch->edict->s.modelindex, "Bip01 Head" );
			
			if ( tag_num < 0 )
				watch_position = ent_to_watch->centroid;
			else
				{
				ent_to_watch->GetTag( "Bip01 Head", &watch_tag_pos );
				watch_position = watch_tag_pos;
				}
			}

		dir = watch_position - tag_pos;
		angles = dir.toAngles();

		angles_diff = angles - self.angles;		
		}
	else
		{
		angles_diff = vec_zero;		
		}

	angles_diff[YAW]   = AngleNormalize180( angles_diff[YAW] );	
	
	
	float yaw_change = angles_diff[YAW];
	float pitch_change = angles_diff[PITCH];
	if ( threshold && ( yaw_change < threshold ) && ( yaw_change > -threshold ) && 
			( pitch_change < threshold ) && ( pitch_change > -threshold ) )
		{
		if ( forever )
			return BEHAVIOR_EVALUATING;
		else
			return BEHAVIOR_SUCCESS;
		}

	
	// Make sure we don't turn eyes too far
	if ( angles_diff[YAW] < self.minEyeYawAngle )
		angles_diff[YAW] = self.minEyeYawAngle ;
	else if ( angles_diff[YAW] > self.maxEyeYawAngle  )
		angles_diff[YAW] = self.maxEyeYawAngle;

	if ( angles_diff[PITCH] < self.minEyePitchAngle  )
		angles_diff[PITCH] = self.minEyePitchAngle ;
	else if ( angles_diff[PITCH] > self.maxEyePitchAngle  )
		angles_diff[PITCH] = self.maxEyePitchAngle;

	angles_diff[ROLL] = 0;


	// Make sure we don't change our eye angles too much at once

	change = angles_diff - current_left_eye_angles;	

	
	// check left eye -- both eyes will be the same
	if ( change[YAW] > max_speed )
		angles_diff[YAW] = current_left_eye_angles[YAW] + max_speed;
	else if ( change[YAW] < -max_speed )
		angles_diff[YAW] = current_left_eye_angles[YAW] - max_speed;

	if ( change[PITCH] > max_speed )
		angles_diff[PITCH] = current_left_eye_angles[PITCH] + max_speed;
	else if ( change[PITCH] < -max_speed )
		angles_diff[PITCH] = current_left_eye_angles[PITCH] - max_speed;

	self.SetControllerAngles( ACTOR_LEYE_TAG, angles_diff );
	self.SetControllerAngles( ACTOR_REYE_TAG, angles_diff );
	//self.real_head_pitch = angles_diff[PITCH];

	current_left_eye_angles = angles_diff;
	current_right_eye_angles = angles_diff;
	//self.eyeposition = current_head_angles;
	
	if ( !ent_to_watch && ( current_left_eye_angles == vec_zero ) )
		return BEHAVIOR_SUCCESS;

	if ( !forever && ( change[YAW] < max_speed ) && ( change[YAW] > -max_speed ) &&
			( change[PITCH] < max_speed ) && ( change[PITCH] > -max_speed ) )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void EyeWatchEnemy::End
	(
	Actor &self
	)

	{
	// Snap head back into position if we have lost our target or we are doing a resethead

	//if ( !ent_to_watch )
	//	{
		self.SetControllerAngles( ACTOR_LEYE_TAG, vec_zero );
		self.SetControllerAngles( ACTOR_REYE_TAG, vec_zero );

		//self.real_head_pitch = 0;
	//	}

	self.SetActorFlag( ACTOR_FLAG_MOVING_EYES, false );
	}

/****************************************************************************

  HeadAndEyeWatch Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, HeadAndEyeWatch, NULL )
	{
		{ &EV_Behavior_Args,			&HeadAndEyeWatch::SetArgs },
		{ NULL, NULL }
	};

HeadAndEyeWatch::HeadAndEyeWatch()
	{
	}

void HeadAndEyeWatch::SetArgs
	(
	Event *ev
	)

	{
	headWatch.SetArgs(ev);
	eyeWatch.SetArgs(ev);

	headWatch.useEyes(true);
	}


void HeadAndEyeWatch::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );	
	headWatch.ShowInfo(self);
	eyeWatch.ShowInfo(self);
	}

void HeadAndEyeWatch::Begin
	(
	Actor &self
	)

	{
	headWatch.Begin(self);
	eyeWatch.Begin(self);
	}

BehaviorReturnCode_t	HeadAndEyeWatch::Evaluate
	(
	Actor &
	)

	{
   // if headWatch returns 0 and eyeWatch returns 0 -- returns 0
	// if headWatch returns 1 and eyeWatch returns 0 -- returns 0
	// if headWatch returns 0 and eyeWatch returns 1 -- returns 0
	// if headWatch returns 1 and eyeWatch returns 1 -- returns 1  
	return BEHAVIOR_SUCCESS;
	}

void HeadAndEyeWatch::End
	(
	Actor &self
	)

	{
	headWatch.End(self);
	eyeWatch.End(self);
	}


/****************************************************************************

  TorsoTurn Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, TorsoTurn, NULL )
	{
		{ &EV_Behavior_Args,			&TorsoTurn::SetArgs },
		{ NULL, NULL }
	};

void TorsoTurn::SetArgs
	(
	Event *ev
	)

	{
	turn_towards_enemy = ev->GetInteger( 1 );
	speed              = ev->GetFloat( 2 );
	forever            = ev->GetInteger( 3 );		

	if ( ev->NumArgs() > 3 )
		tag_name = ev->GetString( 4 );

	if ( ev->NumArgs() > 4 )
		tolerance = ev->GetFloat( 5 );
	else
		tolerance = 0;

	if ( ev->NumArgs() > 5 )
		offset = ev->GetFloat( 6 );
	else
		offset = 0;
	
	if ( ev->NumArgs() > 6 )
		use_pitch = ev->GetBoolean( 7 );
	else
		use_pitch = true;
	}

void TorsoTurn::SetRequiredParameters( int TurnTowardsEnemy , int Speed , int Forever )
	{
	turn_towards_enemy = TurnTowardsEnemy;
	speed = Speed;
	forever = Forever;
	}

void TorsoTurn::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void TorsoTurn::Begin
	(
	Actor &self
	)

	{
	Vector controller_angles;

	self.SetControllerTag( ACTOR_TORSO_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Spine1" ) );

	controller_angles = self.GetControllerAngles( ACTOR_TORSO_TAG );

	current_yaw   = controller_angles[YAW];
	current_pitch = controller_angles[PITCH];
	}

BehaviorReturnCode_t	TorsoTurn::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector angles;
	int tag_num;
	float yaw_diff;
	float pitch_diff;
	Vector new_angles;
	float yaw_change;
	float pitch_change;
	Vector tag_pos;
	Vector tag_forward;
	Vector tag_angles;
   
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	tag_num = gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Spine1" );

	if ( tag_num < 0 )
		return BEHAVIOR_SUCCESS;

	// Determine the angle we want to go to

	if ( turn_towards_enemy )
		{
		if ( !currentEnemy )
			return BEHAVIOR_SUCCESS;
		

		if ( tag_name.length() )
			{
			self.GetTag( tag_name.c_str(), &tag_pos, &tag_forward );
			tag_angles = tag_forward.toAngles();
			dir = currentEnemy->centroid - tag_pos;
			}
		else
			{
			dir = currentEnemy->centroid - self.centroid;
			}
		
	
	

		angles = dir.toAngles();
		
	
		
		angles[YAW] += offset;

		yaw_diff   = AngleNormalize180( angles[YAW] - self.angles[YAW] );
		pitch_diff = AngleNormalize180( angles[PITCH] - self.angles[PITCH] );
		}
	else
		{
		yaw_diff   = 0.0f;
		pitch_diff = 0.0f;
		}

	// Determine the angle change

	yaw_change   = AngleNormalize180( yaw_diff - current_yaw );
	pitch_change = AngleNormalize180( pitch_diff - current_pitch );

	if ( tolerance && ( yaw_change < tolerance ) && ( yaw_change > -tolerance ) && 
			( pitch_change < tolerance ) && ( pitch_change > -tolerance ) )
		{
		if ( forever )
			return BEHAVIOR_EVALUATING;
		else
			return BEHAVIOR_SUCCESS;
		}

	// Make sure we don't change our torso angles too much at once

	if ( yaw_change > speed )
		yaw_diff = current_yaw + speed;
	else if ( yaw_change < -speed )
		yaw_diff = current_yaw - speed;

	if ( pitch_change > speed )
		pitch_diff = current_pitch + speed;
	else if ( yaw_change < -speed )
		pitch_diff = current_pitch - speed;

	// Determine our new angles

	new_angles[YAW]   = yaw_diff;
	
	if ( use_pitch )
		new_angles[PITCH] = pitch_diff;
	else
		new_angles[PITCH] = 0.0f;
	
	new_angles[ROLL]  = 0.0f;

	// Make sure we don't turn too far

	if ( ( new_angles[YAW] > 100.0f ) || ( new_angles[YAW] < -100.0f ) )
		{
		if (!forever ) 
			return BEHAVIOR_SUCCESS;
		else
			{
 	 	   self.angles[YAW] = new_angles[YAW];
		   self.angles[ROLL] = 0;
		   self.setAngles( self.angles );
			return BEHAVIOR_EVALUATING;
			}
		   
		}
		

	if ( new_angles[PITCH] > 45 || new_angles[PITCH] < -45 )
		{
		if (!forever ) 
			return BEHAVIOR_SUCCESS;
		else
			return BEHAVIOR_EVALUATING;
		}


	// Set our new angles

	self.SetControllerAngles( ACTOR_TORSO_TAG, new_angles );

	current_yaw = yaw_diff;
	current_pitch = pitch_diff;

	// See if we are turned the correct direction now

	if ( !forever && (yaw_change < speed) && (yaw_change > -speed) && (pitch_change < speed) && (pitch_change > -speed) )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void TorsoTurn::End
	(
	Actor &self
	)

	{
	self.SetControllerAngles( ACTOR_TORSO_TAG, vec_zero );		
	}





/****************************************************************************

  TorsoWatchEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, TorsoWatchEnemy, NULL )
	{
		{ &EV_Behavior_Args,		&TorsoWatchEnemy::SetArgs },
		{ NULL, NULL }
	};

void TorsoWatchEnemy::SetArgs
	(
	Event *ev
	)

	{
	
	invert = false;   

	speed              = ev->GetFloat( 1 );
	forever            = ev->GetInteger( 2 );		
	threshold          = ev->GetFloat( 3 );

	if ( ev->NumArgs() > 3 )
		offset = ev->GetFloat( 4 );
	else
		offset = 0;
	
	if ( ev->NumArgs() > 4 )
		use_pitch = ev->GetBoolean( 5 );
	else
		use_pitch = true;
	
	if ( ev->NumArgs() > 5 )
		invert = ev->GetBoolean( 6 );

	if ( ev->NumArgs() > 6 )
		reset = ev->GetBoolean ( 7 );
	else
		reset = true;


	invertLegs = false;
	nextFlipTime = 0;

	}


void TorsoWatchEnemy::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void TorsoWatchEnemy::Begin
	(
	Actor &self
	)

	{
	Vector controller_angles;

	self.SetControllerTag( ACTOR_TORSO_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Spine1" ) );

	controller_angles = self.GetControllerAngles( ACTOR_TORSO_TAG );

	current_yaw   = controller_angles[YAW];
	current_pitch = controller_angles[PITCH];

   self.SetActorFlag( ACTOR_FLAG_OUT_OF_TORSO_RANGE , false );
	}

BehaviorReturnCode_t	TorsoWatchEnemy::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector angles;
	int tag_num;
	float yaw_diff;
	float pitch_diff;
	Vector new_angles;
	float yaw_change;
	float pitch_change;
	Vector tag_pos;
	Vector tag_forward;
	Vector tag_angles;
   


	tag_num = gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Spine1" );

	if ( tag_num < 0 )
		return BEHAVIOR_SUCCESS;

	// Determine the angle we want to go to

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		reset = true;
		return BEHAVIOR_EVALUATING;
		}
	
   /*
	if ( invertLegs && self.GetActorFlag( ACTOR_FLAG_ANIM_DONE ) )
      {
      self.SetAnim( "idle" );
      invertLegs = false;
      }
	*/	

	dir = currentEnemy->centroid - self.centroid;	
	
	if (invert)
		dir *= -1.0f;

	angles = dir.toAngles();
			
	angles[YAW] += offset;

   /*
	Vector controller_angles;
	controller_angles = self.GetControllerAngles( ACTOR_TORSO_TAG );

	current_yaw   = controller_angles[YAW];
	current_pitch = controller_angles[PITCH];
   */


	yaw_diff   = AngleNormalize180( angles[YAW] - self.angles[YAW] );
	pitch_diff = AngleNormalize180( angles[PITCH] - self.angles[PITCH] );
	
	//yaw_diff   = AngleNormalize180( angles[YAW] - test[YAW] );
	//pitch_diff = AngleNormalize180( angles[PITCH] - test[PITCH] );

	// Determine the angle change

	yaw_change   = AngleNormalize180( yaw_diff - current_yaw );
	pitch_change = AngleNormalize180( pitch_diff - current_pitch );

	/*
	if ( threshold && yaw_change < threshold && yaw_change > -threshold && pitch_change < threshold && pitch_change > -threshold )
		{
		if ( forever )
			return true;
		else
			return false;
		
		}
   */
	// Make sure we don't change our torso angles too much at once

	if ( yaw_change > speed )
		yaw_diff = current_yaw + speed;
	else if ( yaw_change < -speed )
		yaw_diff = current_yaw - speed;

	if ( pitch_change > speed )
		pitch_diff = current_pitch + speed;
	else if ( pitch_change < -speed )
		pitch_diff = current_pitch - speed;

	// Determine our new angles

	new_angles[YAW] = yaw_diff;
	
	if ( use_pitch )
		{
		new_angles[PITCH] = pitch_diff;
		}
	else
		new_angles[PITCH] = 0.0f;
	
	new_angles[ROLL] = 0.0f;

	// Make sure we don't turn too far

	if ( ( new_angles[YAW] > 100.0f ) || ( new_angles[YAW] < -100.0f ) )
		{
		if ( !forever ) 
			return BEHAVIOR_SUCCESS;
      else
			{
			if ( new_angles[YAW] < -100.0f || new_angles[YAW] > 100.0f )
				{            
            if ( nextFlipTime <= level.time )
               {               
               self.movementSubsystem->flipLegs();	
 	         	invertLegs = true;       
               nextFlipTime = level.time + .75;
               }
            
			   return BEHAVIOR_EVALUATING;
				}

			}	

		}

	if ( ( new_angles[PITCH] > 60.0f ) || ( new_angles[PITCH] < -60 ) )
		{
		if ( !forever ) 
			return BEHAVIOR_SUCCESS;
		else
			return BEHAVIOR_EVALUATING;
		}

	// Set our new angles

	self.SetControllerAngles( ACTOR_TORSO_TAG, new_angles );

	current_yaw   = yaw_diff;
	current_pitch = pitch_diff;

	// See if we are turned the correct direction now

	if ( !forever && (yaw_change < speed) && (yaw_change > -speed) && (pitch_change < speed) && (pitch_change > -speed) )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void TorsoWatchEnemy::End
	(
	Actor &self
	)

	{
   /*
   if ( invertLegs )
      self.movementSubsystem->flipLegs();
   */

   self.movementSubsystem->setMovingBackwards( false );   
   if ( reset )
		self.SetControllerAngles( ACTOR_TORSO_TAG, vec_zero );		
	}
/****************************************************************************

  GotoPathNode Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, GotoPathNode, NULL )
	{
		{ &EV_Behavior_AnimDone,	&GotoPathNode::AnimDone },
		{ &EV_Behavior_Args,		&GotoPathNode::SetArgs },
		{ NULL, NULL }
	};

GotoPathNode::GotoPathNode()
	{
	chase = NULL;
	usevec = false;
	movegoal = NULL;
	goal = vec_zero;
	goalent = NULL;
	state = 0;
	_followingEntity = false;
	}

void GotoPathNode::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->IsVectorAt( 2 ) )
		{
		goal = ev->GetVector( 2 );
		usevec = true;
		}
	else
		{
      usevec = false;
		movegoal = thePathManager.FindNode( ev->GetString( 2 ) );
		if ( !movegoal )
			{
			goalent = ev->GetEntity( 2 );
			}
		}

	if ( ev->NumArgs() > 2 )
		entity_to_watch = ev->GetEntity( 3 );
	}

void GotoPathNode::AnimDone
	(
	Event *
	)

	{
	turnto.ProcessEvent( EV_Behavior_AnimDone );
	}

void GotoPathNode::SetGoal
	(
	PathNode *node
	)

	{
	usevec = false;
	movegoal = node;
	}

void GotoPathNode::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nturnto:\n" );
   turnto.ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase->ShowInfo( self );

   gi.Printf( "\nstate: %d\n", state );
   gi.Printf( "usevec: %d\n", usevec );
   //gi.Printf( "time: %f\n", time );
   gi.Printf( "anim: %s\n", anim.c_str() );

   if ( goalent )
      {
      gi.Printf( "\ngoalent: #%d '%s'\n", goalent->entnum, goalent->targetname.c_str() );
      }
   else
      {
      gi.Printf( "\ngoalent: NULL\n" );
      }

   gi.Printf( "goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z );
	}

void GotoPathNode::Begin
	(
	Actor &self
	)

	{
	Event *ev;

   Vector dir;
	dir = self.movementSubsystem->getAnimDir();
	dir = dir.toAngles();
	self.setAngles( dir );

	turnto.Begin( self );
	float radius=12.0f;
	if ( goalent )
		{
		FollowPathToEntity *newFollowPath = new FollowPathToEntity();
		newFollowPath->SetGoal( goalent, radius, self );
		chase = newFollowPath;
		_followingEntity = true;
		}
	else if ( movegoal )
      {
		FollowPathToPoint *newFollowPath = new FollowPathToPoint();
		newFollowPath->SetGoal( movegoal->origin, radius, self );
		chase = newFollowPath;
      }
   else
		{
		FollowPathToPoint *newFollowPath = new FollowPathToPoint();
		newFollowPath->SetGoal( goal, radius, self );
		chase = newFollowPath;
		}

   chase->Begin( self );

   chase->Begin( self );

	if ( anim.length() )
		{
		self.SetAnim( anim );
		}

	// Setup head watch stuff

	head_watch.Begin( self );

	ev = new Event( EV_Behavior_Args );
	ev->AddEntity( entity_to_watch );
	head_watch.ProcessEvent( ev );
	}

BehaviorReturnCode_t	GotoPathNode::Evaluate
	(
	Actor &self
	)

	{
	float yaw;

	if ( !usevec && !goalent && !movegoal )
		{
		gi.DPrintf( "GotoPathNode::No goal\n" );
		return BEHAVIOR_SUCCESS;
		}

	if ( entity_to_watch )
		head_watch.Evaluate( self );

	switch( state )
		{
		case 0 :
			if ( chase->Evaluate( self ) == Steering::EVALUATING )
				{
				break;
				}

			state = 1;
			self.SetAnim( "idle" );

			// cascade down to case 1
			// lint -fallthrough
		case 1 :
			if ( !movegoal )
				{
				return BEHAVIOR_SUCCESS;
				}

			if ( movegoal->setangles )
				{
            yaw = movegoal->angles.yaw();
				turnto.SetDirection( yaw );
				if ( turnto.Evaluate( self ) )
					{
					break;
					}
				}

			if ( movegoal->animname == "" )
				{
            self.SetAnim( "idle" );
				return BEHAVIOR_SUCCESS;
				}

			self.SetAnim( movegoal->animname, EV_Actor_EndBehavior );
			state = 2;
			break;

		case 2 :
			break;
		}

	return BEHAVIOR_EVALUATING;
	}

void GotoPathNode::End
	(
	Actor &self
	)

	{
	chase->End( self );
	head_watch.End( self );
	}

/****************************************************************************

  Flee Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Flee, NULL )
	{
		{ &EV_Behavior_Args,			&Flee::SetArgs },
		{ NULL, NULL }
	};

void Flee::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	}

void Flee::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );
	}

void Flee::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
		{
		self.SetAnim( anim );
		}

	chase.Begin( self );
	FindFleeNode( self );
	}

void Flee::FindFleeNode
	(
	Actor &self
	)
	{
	int i;
	PathNode *start_node;
	PathNodeConnection *path;
	PathNode *current_node;
	int max_nodes_to_look_at;
	int nodes_looked_at;
	qboolean found;


	// Get closest node

	start_node = thePathManager.NearestNode( self.origin, &self );

	// Find a random node that is connected to this node

	found = false;

	if ( start_node )
		{
		max_nodes_to_look_at = (int)G_Random( 5.0f ) + 10;
		nodes_looked_at = 0;
		current_node = start_node;

		while( !found )
			{
			nodes_looked_at++;

			if ( ( nodes_looked_at >= max_nodes_to_look_at ) && ( current_node != start_node ) )
				{
				found = true;
				flee_node = current_node;
				}

			if ( current_node->NumberOfConnections() == 0 )
				break;

			path = &current_node->GetConnection( (int)G_Random( (float)current_node->NumberOfConnections() ) );
			current_node = thePathManager.GetNode( path->targetNodeIndex );
			}
		}

	if ( !found )
		{
		// If still not found, use old method

		for( i = 0; i < 5; i++ )
			{
			flee_node = thePathManager.GetNode( ( int )G_Random( (float)thePathManager.NumNodes() + 1.0f ) );
			if ( flee_node )
				break;
			}
		}
	}


BehaviorReturnCode_t	Flee::Evaluate
	(
	Actor &self
	)

	{
	// Make sure we have somewhere to flee to

	if ( !flee_node )
		return BEHAVIOR_SUCCESS;

	float radius=96.0f;
	chase.SetGoal( flee_node->origin, radius,  self );

	// Make a racket

	self.Chatter( "snd_panic", 3.0f );

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	// Try to get to flee node

	if ( chase.Evaluate( self ) != Steering::EVALUATING )
		{
		// See if we are done fleeing
		
		if ( !self.sensoryPerception->CanSeeEntity(&self, currentEnemy , true, true ))	
			return BEHAVIOR_SUCCESS;

		// Find a new spot to flee to

		FindFleeNode( self );
		}

	return BEHAVIOR_EVALUATING;
	}

void Flee::End
	(
	Actor &self
	)

	{
	chase.End( self );
	}

/****************************************************************************

  PlayAnim Class Definition

****************************************************************************/
/*
CLASS_DECLARATION( Behavior, PlayAnim, NULL )
	{
		{ &EV_Behavior_Args,			SetArgs },
		{ NULL, NULL }
	};

void PlayAnim::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	}

void PlayAnim::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nanim: %s\n", anim.c_str() );
	}

void PlayAnim::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
		{
		if ( !self.SetAnim( anim, EV_Actor_EndBehavior ) )
         {
         self.PostEvent( EV_Actor_EndBehavior, 0.0f );
         }
		}
	}

BehaviorReturnCode_t	PlayAnim::Evaluate
	(
	Actor &self
	)

	{
	return BEHAVIOR_EVALUATING;
	}

void PlayAnim::End
	(
	Actor &self
	)

	{
	self.RemoveAnimDoneEvent();
	}
*/


/****************************************************************************

  FindCover Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, FindCover, NULL )
	{
		{ &EV_Behavior_Args,			&FindCover::SetArgs },
		{ NULL, NULL }
	};

void FindCover::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		crouch_anim = ev->GetString( 2 );
	}

PathNode *FindCover::FindCoverNode
(
	Actor &self
	)
	
{
	PathNode	*bestnode;
	float		bestdist;
	PathNode	*desperatebestnode;
	float		desperatebestdist;
	FindCoverPath find;
	Path		*path;
	Vector	delta;
	float		dist;
	Vector	pos;
	
	pos = self.origin;
	
	// greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
	bestdist = 999999999.0f;
	bestnode = NULL;
	
	// greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
	desperatebestdist = 999999999.0f;
	desperatebestnode = NULL;
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return NULL;
	
	PathNode *node = NULL;
	for ( int i = 1 ; i <= thePathManager.NumberOfSpecialNodes(); i++ )   
	{   
		node = thePathManager.GetSpecialNode( i );		
		
		if ( node && ( node->nodeflags & ( AI_DUCK | AI_COVER ) ) &&
			( ( node->occupiedTime <= level.time ) || ( node->entnum == self.entnum ) ) )
		{
			// get the distance squared (faster than getting real distance)
			delta = node->origin - pos;
			dist = delta * delta;
			
			if ( ( dist < bestdist ) && ( !self.sensoryPerception->CanSeeEntity ( node->origin , currentEnemy , true, true ) ||//) )//||
				( ( node->nodeflags & AI_DUCK ) && !self.sensoryPerception->CanSeeEntity( node->origin - Vector( 0.0f, 0.0f, 32.0f ) , currentEnemy , true , true ) ) ) )
			{
				bestnode = node;
				bestdist = dist;
			}
			else if ( ( dist < desperatebestdist ) && ( desperatebestdist > ( 64.0f * 64.0f ) ) )
			{
				desperatebestnode = node;
				desperatebestdist = dist; 
			}		
		}
	}
	
	if ( !bestnode )
	{
		bestnode = desperatebestnode;
	}
	
	if ( bestnode )
	{
		find.heuristic.self = &self;
		find.heuristic.setSize( self.size );
		find.heuristic.entnum = self.entnum;
		
		path = find.FindPath( self.origin, bestnode->origin );
		if ( path )
		{
			node = path->End();
			
			// Mark node as occupied for a short time
			node->occupiedTime = level.time + 1.5f;
			node->entnum = self.entnum;
			
			float radius=96.0f;
			chase.SetGoal( node->origin, radius, self );
			
			delete path;
			path = NULL;
			
			return node;
		}
	}
	
	return NULL;
}

void FindCover::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );

   gi.Printf( "\nstate: %d\n", state );
   gi.Printf( "anim: %s\n", anim.c_str() );
   gi.Printf( "nextsearch: %f\n", nextsearch );
	}

void FindCover::Begin
	(
	Actor &
	)

	{
	if ( !anim.length() )
		{
		anim = "run";
		}

	if ( !crouch_anim.length() )
		{
		crouch_anim = "crouch_down";
		}

	movegoal = NULL;
	state = 0;
	}

BehaviorReturnCode_t	FindCover::Evaluate
	(
	Actor &self
	)

	{
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;	

	if ( !movegoal )
		{
		state = 0;
		}

	switch( state )
		{
		case 0 :
			// Checking for cover
			chase.Begin( self );
			movegoal = FindCoverNode( self );
			if ( !movegoal )
				{
            // Couldn't find any!
				return BEHAVIOR_SUCCESS;
				}

			// Found cover, going to it
			if ( anim.length() && ( ( anim != self.animname ) || self.newanim.length() ) )
				{
				self.SetAnim( anim );
				}

			state = 1;
			nextsearch = level.time + 1.0f;

			// lint -fallthrough
		case 1 :
			if ( chase.Evaluate( self ) == Steering::EVALUATING )
				{
				if ( nextsearch < level.time )
					{
					state = 0;
					}
				return BEHAVIOR_EVALUATING;
				}
			
			// Reached cover
		   if ( self.sensoryPerception->CanSeeEntity ( &self , currentEnemy , true , true ) )
				{
				state = 0;
				}

			if ( movegoal->nodeflags & AI_DUCK )
				{
            // ducking
				self.SetAnim( crouch_anim.c_str() );
				}
			else
				{
            // standing
				self.SetAnim( "idle" );
				}

			chase.End( self );
			return BEHAVIOR_SUCCESS;
			break;
		}

	return BEHAVIOR_EVALUATING;
	}

void FindCover::End
	(
	Actor &self
	)

	{
	chase.End( self );
	}

/****************************************************************************

  FindFlee Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, FindFlee, NULL )
	{
		{ &EV_Behavior_Args,			&FindFlee::SetArgs },
		{ NULL, NULL }
	};

void FindFlee::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	}

PathNode *FindFlee::FindFleeNode
	(
	Actor &self
	)

	{
	PathNode	*bestnode;
	float		bestdist;
	PathNode	*desperatebestnode;
	float		desperatebestdist;
	FindFleePath find;
	Path		*path;
	Vector	delta;
	float		dist;
	Vector	pos;

   pos = self.origin;

   // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
	bestdist = 999999999.0f;
	bestnode = NULL;

   // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
	desperatebestdist = 999999999.0f;
   desperatebestnode = NULL;


	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return NULL;

	PathNode *node = NULL;
	for ( int i = 1 ; i <= thePathManager.NumberOfSpecialNodes(); i++ )   
	{   
		node = thePathManager.GetSpecialNode( i );		

		if ( node && ( node->nodeflags & AI_FLEE ) &&
			( ( node->occupiedTime <= level.time ) || ( node->entnum == self.entnum ) ) )
			{
			// get the distance squared (faster than getting real distance)
         delta = node->origin - pos;
			dist = delta * delta;
         
				if ( ( dist < bestdist ) && !self.sensoryPerception->CanSeeEntity( node->origin , currentEnemy , true, true  ) )
					{
					bestnode = node;
					bestdist = dist;
					}
				else if ( ( dist < desperatebestdist ) && ( desperatebestdist > ( 64.0f * 64.0f ) ) )
					{
					desperatebestnode = node;
					desperatebestdist = dist;
					}

			
			}
		}

	if ( !bestnode )
		{
		bestnode = desperatebestnode;
		}

	if ( bestnode )
		{
		find.heuristic.self = &self;
		find.heuristic.setSize( self.size );
		find.heuristic.entnum = self.entnum;

      path = find.FindPath( self.origin, bestnode->origin );
		if ( path )
			{
			node = path->End();

			// Mark node as occupied for a short time
			node->occupiedTime = level.time + 1.5f;
			node->entnum = self.entnum;

			float radius=96.0f;
			chase.SetGoal( node->origin, radius, self );

			delete path;
			path = NULL;

			return node;
			}
		}

	return NULL;
	}

void FindFlee::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );

   gi.Printf( "\nstate: %d\n", state );
   gi.Printf( "anim: %s\n", anim.c_str() );
   gi.Printf( "nextsearch: %f\n", nextsearch );
	}

void FindFlee::Begin
	(
	Actor &
	)

	{
	if ( !anim.length() )
		{
		anim = "run";
		}

	movegoal = NULL;
	state = 0;
	}

BehaviorReturnCode_t	FindFlee::Evaluate
	(
	Actor &self
	)

	{
	if ( !movegoal )
		{
		state = 0;
		}

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	switch( state )
		{
		case 0 :
         // Checking for flee node
			chase.Begin( self );
			movegoal = FindFleeNode( self );
			if ( !movegoal )
				{
            // Couldn't find any!
				return BEHAVIOR_SUCCESS;
				}

         // Found flee node, going to it
			if ( anim.length() && ( anim != self.animname || self.newanim.length() ) )
				{
				self.SetAnim( anim );
				}

			state = 1;
			nextsearch = level.time + 1.0f;

			// lint -fallthrough
		case 1 :
			if ( chase.Evaluate( self ) == Steering::EVALUATING )
				{
				if ( nextsearch < level.time )
					{
					state = 0;
					}
				return BEHAVIOR_EVALUATING;
				}

		if ( self.sensoryPerception )
			{
		   // Reached cover
			if ( self.sensoryPerception->CanSeeEntity( &self , currentEnemy , true , true ) )
				{
				state = 0;
				}
			else
				{
				// standing
   			self.SetAnim( "idle" );
	   		chase.End( self );
				return BEHAVIOR_SUCCESS;
				}
			}

		break;
		}

	return BEHAVIOR_EVALUATING;
	}

void FindFlee::End
	(
	Actor &self
	)

	{
	chase.End( self );
	}

/****************************************************************************

  FindEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, FindEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&FindEnemy::SetArgs },
		{ NULL, NULL }
	};

void FindEnemy::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	}

void FindEnemy::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );

   gi.Printf( "\nstate: %d\n", state );
   gi.Printf( "nextsearch: %f\n", nextsearch );
   gi.Printf( "anim: %s\n", anim.c_str() );
	}

void FindEnemy::Begin
	(
	Actor &
	)

	{
	if ( !anim.length() )
		{
		anim = "run";
		}

	movegoal = NULL;
   lastSearchNode = NULL;
	state = 0;
	}

PathNode *FindEnemy::FindClosestSightNode
	(
	Actor &self
	)

	{
	PathNode	*bestnode;
	float		bestdist;
	Vector	delta;
	float		dist;
	Vector	pos;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

   if ( currentEnemy )
      {
      pos = currentEnemy->origin;
      }
   else
      {
      pos = self.origin;
      }

   // greater than ( 8192 * sqr(2) ) ^ 2 -- maximum squared distance
	bestdist = 999999999;
	bestnode = NULL;

	for ( int i = 1 ; i <= thePathManager.NumberOfSpecialNodes(); i++ )   
	{   
		PathNode *node = thePathManager.GetSpecialNode( i );		

		if ( node && ( ( node->occupiedTime <= level.time ) || ( node->entnum != self.entnum ) ) )
			{
			// get the distance squared (faster than getting real distance)
         delta = node->origin - pos;
			dist = delta * delta;
			   if ( ( dist < bestdist ) && self.sensoryPerception->CanSeeEntity( node->origin, currentEnemy , true , true ) )
					{
					bestnode = node;
					bestdist = dist;
					}
			
			}
		}

	return bestnode;
	}

BehaviorReturnCode_t	FindEnemy::Evaluate
	(
	Actor &self
	)

	{
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( nextsearch < level.time )
		{
      // check if we should search for the first time
      if ( !lastSearchNode )
         {
         state = 0;
         }
      else
         {
         // search less often if we're far away
			nextsearch = self.DistanceTo( currentEnemy ) * ( 1.0f / 512.0f );
         if ( nextsearch < 1.0f )
            {
            nextsearch = 1.0f;
            }
         nextsearch += level.time;

         // don't search again if our enemy hasn't moved very far
         if ( currentEnemy->WithinDistance( lastSearchPos, 256.0f ) )
            {
            state = 0;
            }
         }
		}

	switch( state )
		{
		case 0 :
			// Searching for enemy
			chase.Begin( self );
         lastSearchPos = currentEnemy->origin;
         movegoal = thePathManager.NearestNode( lastSearchPos, &self );
         if ( !movegoal )
            {
            movegoal = thePathManager.NearestNode( lastSearchPos, &self, false );
            }

         lastSearchNode = movegoal;
			if ( movegoal )
				{
				Path *path;
				FindEnemyPath find;
            PathNode *from;

				find.heuristic.self = &self;
				find.heuristic.setSize( self.size );
				find.heuristic.entnum = self.entnum;

            from = thePathManager.NearestNode( self.origin, &self );
            if ( ( from == movegoal ) && ( self.DistanceTo( from->origin ) < 8.0f ) )
               {
               movegoal = NULL;
               from = NULL;
               }

            if ( from )
               {
               path = find.FindPath( from, movegoal );
				   if ( path )
					   {
						float radius=96.0f;
					   chase.SetGoal( movegoal->origin, radius,  self );

					   delete path;
					   path = NULL;
					   }
               else
                  {
                  movegoal = NULL;
                  }
               }
				}

			if ( !movegoal )
				{
				if ( self.sensoryPerception->CanSeeEntity( &self , currentEnemy , true , true  ) || ( !currentEnemy->groundentity && !self.waterlevel ) )
					{
					float radius=96.0f;
					chase.SetGoal( currentEnemy->origin, radius, self );
					}
				else
					{
					// Couldn't find enemy
					// since we can't reach em
					// clear out enemy state
					self.enemyManager->ClearCurrentEnemy();
					return BEHAVIOR_SUCCESS;
					}
				}

         // Found enemy, going to it
			if ( anim.length() && ( anim != self.animname || self.newanim.length() ) )
				{
				self.SetAnim( anim );
				}

			state = 1;

			// lint -fallthrough
		case 1 :
			if ( self.CanAttack( currentEnemy, false ) )
				{
				// Reached enemy
				chase.End( self );
				return BEHAVIOR_SUCCESS;
				}

			if ( chase.Evaluate( self ) != Steering::EVALUATING )
				{
            state = 0;
				nextsearch = 0;
				}
			break;
		}

	return BEHAVIOR_EVALUATING;
	}

void FindEnemy::End
	(
	Actor &self
	)

	{
	chase.End( self );
	}

/****************************************************************************

  AimAndShoot Class Definition

****************************************************************************/
#define START_AIM 0
#define AIM       1
#define FIRE      2

CLASS_DECLARATION( Behavior, AimAndShoot, NULL )
	{
		{ &EV_Behavior_Args,		&AimAndShoot::SetArgs },
		{ &EV_Behavior_AnimDone,	&AimAndShoot::AnimDone },
		{ NULL, NULL }
	};

AimAndShoot::AimAndShoot()
	{
	maxshots = 1;
	numshots = 0;
	aim_time = 0.0f;
	enemy_health = 0;
	animdone = false;
	}

void AimAndShoot::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\naim:\n" );
   aim.ShowInfo( self );

   gi.Printf( "\nmode: %d\n", mode );
   gi.Printf( "maxshots: %d\n", maxshots );
   gi.Printf( "numshots: %d\n", numshots );
   gi.Printf( "animdone: %d\n", animdone );
	}

void AimAndShoot::Begin
	(
	Actor &self
	)

	{
	if ( aimanim.length() )
		{
		self.SetAnim( aimanim.c_str() );
		mode = START_AIM;
		}
	else
		{
		self.SetAnim( "idle" );
		mode = AIM;
		}
   }

void AimAndShoot::SetMaxShots
	(
	int num
	)

	{
	maxshots = ( num / 2 ) + (int)G_Random( (float)num );
	}

void AimAndShoot::SetArgs
	(
	Event *ev
	)

	{
   fireanim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
      {
		maxshots = ev->GetInteger ( 2 );
      }

	if ( ev->NumArgs() > 2 )
      {
		aimanim = ev->GetString ( 3 );
      }
	
	int turnTowardsEnemy = ev->GetInteger( 3 );
	int speed = ev->GetInteger( 4 );
	
	torsoTurn.SetRequiredParameters( turnTowardsEnemy, speed, 0 );
	}

void AimAndShoot::AnimDone
	(
	Event *
	)

	{
	animdone = true;
	}

BehaviorReturnCode_t	AimAndShoot::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector angles;
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	switch( mode )
		{
		case START_AIM :     
         
			if ( !currentEnemy || !self.CanAttack( currentEnemy, false ))
            return BEHAVIOR_SUCCESS;
  			
			animdone = false;
         if ( aimanim.length() )
            self.SetAnim( aimanim.c_str() );
         
         // save off time, in case we aim for too long
         aim_time = level.time + 1.0f;
			mode = AIM;

			// lint -fallthrough
		case AIM :
			
			if ( aimanim.length() )
				{
				if ( !currentEnemy || ( aim_time < level.time ) )
					return BEHAVIOR_SUCCESS;

				aim.SetTarget( currentEnemy );

				if ( aim.Evaluate( self ) )
					{
					break;
					}
				}
			else
				{
				if ( self.IsEntityAlive( currentEnemy ) )
					{
					dir = currentEnemy->centroid - self.origin;
					angles = dir.toAngles();

					self.angles[YAW] = angles[YAW];
					self.setAngles( self.angles );
					}
				}

			// don't go into our firing animation until our weapon is ready, and we are on target
			if ( currentEnemy && self.CanAttack( currentEnemy, true ) )
				{
				animdone = false;
				self.Chatter( "snd_inmysights", 5.0f );
				self.SetAnim( fireanim.c_str(), EV_Actor_NotifyBehavior );
            enemy_health = currentEnemy->health;
				mode = 2;
				}
			else if ( !currentEnemy || currentEnemy->deadflag ||
				( currentEnemy->health <= 0.0f ) || !self.CanAttack( currentEnemy, false ) )
				{
				// either our enemy is dead, or we can't shoot the enemy from here
				return BEHAVIOR_SUCCESS;
				}
			break;

		case FIRE :
			// Fire
			if ( animdone )
				{
				aim.SetTarget( currentEnemy );
				aim.Evaluate( self );

				self.times_done++;

            if ( !currentEnemy || ( currentEnemy->health < enemy_health ) )
               {
					self.Chatter( "snd_attacktaunt", 4.0f );
               }
            else
               {
					self.Chatter( "snd_missed", 7.0f );
               }

				animdone = false;
				numshots++;

				if ( ( numshots >= maxshots ) || !currentEnemy || currentEnemy->deadflag ||
					( currentEnemy->health <= 0.0f ) || !self.CanAttack( currentEnemy, false ) )
					{
					// either we're out of shots, our enemy is dead, or we can't shoot the enemy from here

					return BEHAVIOR_SUCCESS;
					}
				else if ( !self.CanAttack( currentEnemy, false ) )
					{
					// weapon not ready or not aimed at enemy, so just keep trying to get enemy in our sights
               if ( aimanim.length() )
                  {
		   			self.SetAnim( aimanim.c_str() );
                  }
               //
               // save off time, in case we aim for too long
               //
               aim_time = level.time + 1.0f;
					mode = 1;
					}
				else
					{
					// keep firing
					self.SetAnim( fireanim.c_str(), EV_Actor_NotifyBehavior );
               enemy_health = currentEnemy->health;
					}
				}
			break;
		}

	return BEHAVIOR_EVALUATING;
	}

void AimAndShoot::End
	(
	Actor &self
	)

	{
	aim.End( self );
	//self.SetAnim( "idle" );
	}

/****************************************************************************

  AimAndMelee Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, AimAndMelee, NULL )
	{
		{ &EV_Behavior_Args,		&AimAndMelee::SetArgs },
		{ &EV_Behavior_AnimDone,	&AimAndMelee::AnimDone },
		{ NULL, NULL }
	};

AimAndMelee::AimAndMelee()
	{
	maxshots = 1;
	anim_name = "melee";
	mode = 0;
   numshots = 0;
	animdone = false;
	}

void AimAndMelee::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\naim:\n" );
   aim.ShowInfo( self );

   gi.Printf( "\nmode: %d\n", mode );
   gi.Printf( "maxshots: %d\n", maxshots );
   gi.Printf( "numshots: %d\n", numshots );
   gi.Printf( "animdone: %d\n", animdone );
	}

void AimAndMelee::Begin
	(
	Actor &self
	)

	{
	self.SetAnim( "idle" );
	}

void AimAndMelee::SetArgs
	(
	Event *ev
	)

	{
	anim_name = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
      {
		maxshots = ev->GetInteger( 2 );
      }
	}

void AimAndMelee::AnimDone
	(
	Event *
	)

	{
	animdone = true;
	}

BehaviorReturnCode_t	AimAndMelee::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	switch( mode )
		{
		case 0 :
			if ( !currentEnemy )
            {
            return BEHAVIOR_SUCCESS;
            }

			if ( self.IsEntityAlive( currentEnemy ) )
				{
				dir = currentEnemy->centroid - self.origin;
				self.angles[YAW] = dir.toYaw();
				self.setAngles( self.angles );
				}

         numshots++;
         animdone = false;

			// melee
			self.SetAnim( anim_name.c_str() , EV_Actor_NotifyBehavior );
			self.Chatter( "snd_attacktaunt", 4.0f );
         mode = 1;

			// lint -fallthrough
		case 1 :
			// finish up the attack
			if ( animdone )
				{
				self.times_done++;
            if ( numshots >= maxshots )
               {
				   return BEHAVIOR_SUCCESS;
               }

            mode = 0;
            }
			break;
		}

	return BEHAVIOR_EVALUATING;
	}

void AimAndMelee::End
	(
	Actor &self
	)

	{
	aim.End( self );
	}

/****************************************************************************

  FallToDeath Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, FallToDeath, NULL )
	{
		{ &EV_Behavior_Args,	&FallToDeath::SetArgs },
		{ NULL, NULL }
	};

FallToDeath::FallToDeath()
   {

	}

void FallToDeath::SetArgs
	(
	Event *ev
	)
	{
   // Get the Parameters
	forwardmove = ev->GetFloat( 1 );
   sidemove    = ev->GetFloat( 2 );
   speed       = ev->GetFloat( 3 );
	startAnim   = ev->GetString( 4 );
	fallAnim    = ev->GetString( 5 );
	deathAnim   = ev->GetString( 6 );
	
	if ( ev->NumArgs() > 6 )
		impulse_time = ( ev->GetFloat( 7 ) ) + level.time;
	else
		impulse_time = 0;

	animdone = false;
	state    = 0;
	did_impulse = false;
	}

void FallToDeath::Begin
	(
	Actor &self
	)

	{
	//Initialize our stuff


	self.SetAnim( startAnim.c_str() , EV_Anim_Done );
	self.ChangeAnim();
	}

BehaviorReturnCode_t	FallToDeath::Evaluate
	(
	Actor &self
	)

	{
   if ( !did_impulse && ( level.time >= impulse_time ) )
		{
		Vector( 0.0f, self.angles.y, 0.0f ).AngleVectors( &yaw_forward, &yaw_left );
		self.velocity = ( yaw_forward * forwardmove ) - ( yaw_left * sidemove );
	   distance = self.velocity.length();
	   self.velocity *= speed / distance;
	   time = distance / speed;
	   self.velocity[ 2 ] = sv_currentGravity->integer * time * 0.5f;
		did_impulse = true;
		}
	

	animdone = self.GetActorFlag( ACTOR_FLAG_ANIM_DONE );
	self.SetActorFlag( ACTOR_FLAG_ANIM_DONE, false );

   switch( state )
      {
      case 0:
         if ( did_impulse )
				state = 1;
         // this is here so that we at least hit this function at least once
         // this gaves the character the chance to leave the ground, nulling out
         // self.groundentity
         break;
		case 1:
			if ( animdone )
            {
            animdone = false;
         	self.SetAnim( fallAnim.c_str(), EV_Anim_Done );
            state = 2;
            }
			if ( self.groundentity )
				state = 2;
			break;
      case 2:
         //
         // wait for the character to hit the ground
         //
	      if ( self.groundentity )
            {
            //
            // if we have an anim, we go to state 3
            //
            animdone = false;
         	self.SetAnim( deathAnim.c_str(), EV_Actor_Dead );
            state = 3;            
            }
         break;
      case 3:
         //
         // we are on the ground and waiting for our landing animation to finish
         //
         
			if ( animdone )
            {
				return BEHAVIOR_SUCCESS;
            }			
         break;
      }

	return BEHAVIOR_EVALUATING;
	}

void FallToDeath::End
	(
	Actor &self
	)

	{
	Event *ev;
	ev = new Event( EV_Killed );

	ev->AddEntity( NULL );
	ev->AddFloat( 250.0f );
	ev->AddEntity( NULL );
	ev->AddInteger( MOD_FALLING );
	ev->AddInteger( 1 );

	self.PostEvent( ev , 0.0f );
	}

/****************************************************************************

  JumpToPathNode Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, JumpToPathNode, NULL )
	{
		{ &EV_Behavior_Args,		&JumpToPathNode::SetArgs },
		{ NULL, NULL }
	};

void JumpToPathNode::SetArgs( Event *ev )
{
	movegoal = thePathManager.FindNode( ev->GetString( 1 ) );
	
	if ( movegoal )
		jump.SetGoal( movegoal->origin );
	else
	{
		Entity *entity = ev->GetEntity( 1 );
		
		if ( entity )
			jump.SetEntity( entity );
	}
	
	if ( ev->NumArgs() > 1 )
		jump.SetLaunchAngle( ev->GetFloat( 2 ) );	
	
	
}

void JumpToPathNode::Begin( Actor &self )
{
	jump.Begin( self );
}

BehaviorReturnCode_t JumpToPathNode::Evaluate( Actor &self )
{
	if ( jump.Evaluate( self ) !=  Steering::SUCCESS )
		return BEHAVIOR_EVALUATING;
	
	return BEHAVIOR_SUCCESS;
}

void JumpToPathNode::End( Actor &self )
{
	jump.End( self );
}

/****************************************************************************

  LeapToEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, LeapToEnemy, NULL )
	{
		{ &EV_Behavior_Args,		&LeapToEnemy::SetArgs },
		{ NULL, NULL }
	};

void LeapToEnemy::SetArgs
	(
	Event *ev
	)

	{
	jump.SetLaunchAngle( ev->GetFloat( 1 ) );	
	}

void LeapToEnemy::Begin
	(
	Actor &self
	)

	{
	Entity* ent = self.enemyManager->GetCurrentEnemy();	
	jump.SetEntity(ent);
	jump.Begin(self);
	}

BehaviorReturnCode_t	LeapToEnemy::Evaluate
	(
	Actor &self
	)

	{
	if ( jump.Evaluate( self ) == Steering::EVALUATING )
      return BEHAVIOR_EVALUATING;

   return BEHAVIOR_SUCCESS;
	}

void LeapToEnemy::End
	(
	Actor &self
	)

	{
	jump.End(self);
	}

/****************************************************************************

  FlyToPoint Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyToPoint, NULL )
	{
		{ NULL, NULL }
	};

FlyToPoint::FlyToPoint()
	{
	turn_speed = 10.0;
	old_turn_speed = turn_speed;
	speed = 480.0;
	random_allowed = true;
	force_goal = false;
	adjustYawAndRoll = true;
	offsetOrigin = false;

	avoidtime = 0;
	stuck = 0;
	use_temp_goal = false;

	old_forward_speed = 0;
	}

void FlyToPoint::SetTurnSpeed( float new_turn_speed )
	{
	turn_speed = new_turn_speed;
	}

void FlyToPoint::SetGoalPoint( const Vector &goal_point )
	{
	if ( goal_point != goal )
		avoidtime = 0;

	goal = goal_point;
	}

void FlyToPoint::SetRandomAllowed( qboolean allowed )
	{
	random_allowed = allowed;
	}

void FlyToPoint::SetSpeed( float speed_value )
	{
	speed = speed_value;
	}

void FlyToPoint::ForceGoal( void )
	{
	force_goal = true;
	}

void FlyToPoint::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyToPoint::Begin
	(
	Actor &self
	)

	{
   avoidtime = 0;
	old_forward_speed = self.movementSubsystem->getForwardSpeed();	
	stuck = 0;
	use_temp_goal = false;
	}

BehaviorReturnCode_t	FlyToPoint::Evaluate
	(
	Actor &self
	)

	{
   trace_t trace;
   Vector dir;
   Vector ang;
	float time;
	float length;
	float old_yaw;
	float old_pitch;
	
	float x_offset = 0;
	float y_offset = 0;
	//float z_offset = 0;


   if (offsetOrigin)
		{
   	x_offset = self.centroid.x - self.origin.x;
   	y_offset = self.centroid.y - self.origin.y;
	   //z_offset = self.centroid.z - self.origin.z;
		
		self.origin.x+=x_offset;
	   self.origin.y+=y_offset;
	   //self.origin.z+=z_offset;
		}

	if ( self.movementSubsystem->getLastMove() != STEPMOVE_OK )
		stuck++;
	else
		stuck = 0;


	if ( stuck > 1 || ( avoidtime <= level.time ) )
      {
		time = G_Random( .3f ) + .3f;

		use_temp_goal = false;

		if ( !force_goal )
			{
			trace = G_Trace( self.origin, self.mins, self.maxs, goal, &self, self.edict->clipmask, false, "FlyToPoint" );
						
			if ( ( trace.fraction < 0.5f ) || ( stuck > 2 ) )
				{
				old_turn_speed = self.movementSubsystem->getTurnSpeed();
				self.movementSubsystem->setTurnSpeed( 60 );
				temp_goal = ChooseRandomDirection( self, goal, &time, MASK_WATER, false );
				
				self.movementSubsystem->setTurnSpeed( old_turn_speed );
				use_temp_goal = true;
				avoidtime = level.time + time;

				stuck = 0;
				}
			else
				{
				goal = trace.endpos;
				avoidtime = level.time + time;
				}
			}
		else
			{
			avoidtime = level.time + time;
			}
		
		if ( use_temp_goal )
			dir = temp_goal - self.origin;						
		else
			dir = goal - self.origin;			

		length = dir.length();
      dir.normalize();
		
      ang = dir.toAngles();
		
		if ( ( length > 150.0f ) && random_allowed && !use_temp_goal )
			{
			//ang[YAW]   += G_Random( 20 ) - 5.0;
			//ang[PITCH] += G_Random( 20 ) - 5.0;
			}

		target_angle = ang;

		target_angle[YAW]   = AngleNormalize360( target_angle[YAW] );
		target_angle[PITCH] = AngleNormalize360( target_angle[PITCH] );
      }

	if ( (self.angles[YAW] != target_angle[YAW]) || (self.angles[PITCH] != target_angle[PITCH]) )
      {
		self.movementSubsystem->setForwardSpeed( speed * 0.8f );
      }
	else
      {
		self.movementSubsystem->setForwardSpeed( speed );
      }

	old_yaw   = self.angles[YAW];
	old_pitch = self.angles[PITCH];
	
	ang[YAW]   = LerpAngle( self.angles[YAW],   target_angle[YAW],   turn_speed );
	ang[PITCH] = LerpAngle( self.angles[PITCH], target_angle[PITCH], turn_speed );
	ang[ROLL]  = 0;
	
	if( adjustYawAndRoll )
		{
	   
		if ( ( AngleDelta( ang[YAW], old_yaw ) > 0.0f ) && ( ( ang[ROLL] > 315.0f ) || ( ang[ROLL] <= 45.0f ) ) )
			{
			ang[ROLL] += 5.0f;
			}
		else if ( ( AngleDelta( ang[YAW], old_yaw ) < 0.0f ) && ( ( ang[ROLL] < 45.0f ) || ( ang[ROLL] >= 315.0f ) ) )
			{
			ang[ROLL] -= 5.0f;
			}
		else
			{
			if ( ang[ROLL] != 0.0f )
				{
				
				if ( ang[ROLL] < 5.0f || ang[ROLL] > 355.0f )
					{
					ang[ROLL] = 0.0f;
					}
				else
					{
					if ( ang[ROLL] < 180.0f )
						ang[ROLL] += 5.0f;
					else
						ang[ROLL] -= 5.0f;
					}
				}
			}
		}

	ang[YAW]   = AngleNormalize360( ang[YAW] );
	ang[PITCH] = AngleNormalize360( ang[PITCH] );
	ang[ROLL]  = AngleNormalize360( ang[ROLL] );

	// Don't get stuck if still turning

	if ( ( AngleDelta( ang[YAW], old_yaw ) > .5 ) || ( AngleDelta( ang[YAW], old_yaw ) < -.5 ) ||
		  ( AngleDelta( ang[PITCH], old_pitch ) > .5 ) || ( AngleDelta( ang[PITCH], old_pitch ) < -.5 ) )
		{
		stuck = 0;
		}

	self.setAngles( ang );

	if (offsetOrigin)
		{
   	self.origin.x-=x_offset;
	   self.origin.y-=y_offset;
	   //self.origin.z-=z_offset;
		}

	return BEHAVIOR_EVALUATING;
	}

float FlyToPoint::LerpAngle( float old_angle, float new_angle, float lerp_amount )
	{
	float diff;
	float abs_diff;
	float lerp_angle;

	new_angle = AngleNormalize360( new_angle );
	old_angle = AngleNormalize360( old_angle );

	diff = new_angle - old_angle;

	if ( diff > 180.0f )
      {
		diff -= 360.0f;
      }

	if ( diff < -180.0f )
      {
		diff += 360.0f;
      }

	lerp_angle = old_angle;
	
	abs_diff = diff;

	if ( abs_diff < 0.0f )
      {
		abs_diff = -abs_diff;
      }

	if ( abs_diff < lerp_amount )
      {
		lerp_amount = abs_diff;
      }

	if ( diff < 0.0f )
      {
		lerp_angle -= lerp_amount;
      }
	else if ( diff > 0.0f )
      {
		lerp_angle += lerp_amount;
      }
	
	lerp_angle = AngleNormalize360( lerp_angle );

	return lerp_angle;
	}

void FlyToPoint::End
	(
	Actor &self
	)

	{
	self.movementSubsystem->setForwardSpeed( old_forward_speed );	
	}

/****************************************************************************

  FlyCloseToEnemy Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyCloseToEnemy, NULL )
	{
		{ &EV_Behavior_Args,		&FlyCloseToEnemy::SetArgs },
		{ NULL, NULL }
	};

FlyCloseToEnemy::FlyCloseToEnemy()
	{
	speed = 0;
	turn_speed = 10.0;
	anim = "fly";
	adjustPitch = true;
	next_goal_time = 0;
	}

void FlyCloseToEnemy::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		turn_speed = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		speed = ev->GetFloat( 3 );

	if (ev->NumArgs() > 3 )
		fly.setAdjustYawAndRoll( ev->GetBoolean( 4 ) );	

	if (ev->NumArgs() > 4 )
		adjustPitch = ev->GetBoolean( 5 );
	}


void FlyCloseToEnemy::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyCloseToEnemy::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	fly.Begin( self );
	fly.SetTurnSpeed( turn_speed );

	if ( speed )
		fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyCloseToEnemy::Evaluate
	(
	Actor &self
	)

	{
   Vector goal;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

   if ( !self.IsEntityAlive( currentEnemy ) )
      return BEHAVIOR_SUCCESS;

	if ( next_goal_time <= level.time )
		{
		goal = currentEnemy->centroid;

		if (!adjustPitch)
			{
			goal.z = self.origin.z;
			}

		//goal[0] += G_Random( 30 ) - 15.0;
		//goal[1] += G_Random( 30 ) - 15.0;
		//goal[2] += G_Random( 60 ) - 30.0;

		fly.SetGoalPoint( goal );

		next_goal_time = level.time + .5f;
		}

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyCloseToEnemy::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  FlyCloseToPlayer Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyCloseToPlayer, NULL )
	{
		{ &EV_Behavior_Args,		&FlyCloseToPlayer::SetArgs },
		{ NULL, NULL }
	};

FlyCloseToPlayer::FlyCloseToPlayer()
	{
	speed = 0;
	turn_speed = 10.0;
	anim = "fly";
	next_goal_time = 0;
	}

void FlyCloseToPlayer::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		turn_speed = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		speed = ev->GetFloat( 3 );
	}

void FlyCloseToPlayer::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyCloseToPlayer::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	fly.Begin( self );
	fly.SetTurnSpeed( turn_speed );
		
	if ( speed )
		fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyCloseToPlayer::Evaluate
	(
	Actor &self
	)

	{
   Vector goal;
	Entity *player;


	player = g_entities[ 0 ].entity;


   if ( !self.IsEntityAlive( player ) )
      return BEHAVIOR_SUCCESS;

	if ( next_goal_time <= level.time )
		{
		goal = player->centroid;

		//goal[0] += G_Random( 30 ) - 15.0;
		//goal[1] += G_Random( 30 ) - 15.0;
		//goal[2] += G_Random( 60 ) - 30.0;

		fly.SetGoalPoint( goal );

		next_goal_time = level.time + .5f;
		}

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyCloseToPlayer::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}


/****************************************************************************

  FlyCloseToParent Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyCloseToParent, NULL )
	{
		{ &EV_Behavior_Args,		&FlyCloseToParent::SetArgs },
		{ NULL, NULL }
	};

FlyCloseToParent::FlyCloseToParent()
	{
	speed = 0.0f;
	turn_speed = 10.0f;
	anim = "fly";
	next_goal_time = 0.0f;
	}

void FlyCloseToParent::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		turn_speed = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		speed = ev->GetFloat( 3 );
	}

void FlyCloseToParent::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyCloseToParent::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	fly.Begin( self );
	fly.SetTurnSpeed( turn_speed );
		
	if ( speed )
		fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyCloseToParent::Evaluate
	(
	Actor &self
	)

	{
   Vector goal;


   if ( !self.spawnparent || !self.spawnparent->isSubclassOf( Actor ) )
		return BEHAVIOR_SUCCESS;
   
	if ( next_goal_time <= level.time )
		{
		Actor* act;

		act = (Actor*)self.spawnparent;
		goal = act->centroid;
		
		fly.SetGoalPoint( goal );

		next_goal_time = level.time + .5f;
		}

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyCloseToParent::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}


/****************************************************************************

  FlyDescend Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, FlyDescend, NULL )
	{
		{ &EV_Behavior_Args,	&FlyDescend::SetArgs },
		{ NULL, NULL }
	};

FlyDescend::FlyDescend()
	{
	anim = "fly";
	height = 500;
	speed = 0;
	next_height_check = level.time + 2.0f;
	last_check_height = 0.0f;
	}

void FlyDescend::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		height = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		speed = ev->GetFloat( 3 );
	}

void FlyDescend::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyDescend::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return;

	fly.Begin( self );

	height = currentEnemy->origin.z;

	goal = self.origin;
	goal.z = height;

	fly.SetTurnSpeed( 1.0f );

	fly.SetGoalPoint( goal );
	
	last_check_height = self.origin.z;

	if ( speed )
		fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyDescend::Evaluate
	(
	Actor &self
	)

	{
	if ( self.origin.z <= height || !self.enemyManager->GetCurrentEnemy())
      {
		return BEHAVIOR_SUCCESS;
      }

	if ( next_height_check < level.time )
		{
		if ( self.origin.z < ( last_check_height + 25.0f ) )
			return BEHAVIOR_SUCCESS;

		next_height_check = level.time + 2.0f;
		last_check_height = self.origin.z;
		}

	if ( self.movementSubsystem->getLastMove() == STEPMOVE_OK )
		fly.SetGoalPoint( goal );

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyDescend::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}



/****************************************************************************

  FlyWander Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyWander, NULL )
	{
		{ &EV_Behavior_Args,	&FlyWander::SetArgs },
		{ NULL, NULL }
	};

FlyWander::FlyWander()
	{
	turn_speed = 10.0;
	anim = "fly";
	change_course_time = 5.0;
	speed = 200;
	try_to_go_up = false;
	next_change_course_time = 0;
	original_z = 0;
	}

void FlyWander::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		speed = ev->GetFloat( 2 );
      
	if ( ev->NumArgs() > 2 )
      turn_speed = ev->GetFloat( 3 );
      
	if ( ev->NumArgs() > 3 )
      change_course_time = ev->GetFloat( 4 );
      
	if ( ev->NumArgs() > 4 )
      try_to_go_up = ev->GetBoolean( 5 );
      
	if (ev->NumArgs() > 5 )
		fly.setAdjustYawAndRoll( ev->GetBoolean( 6 ) );	

	if (ev->NumArgs() > 6 )
		fly.setOffsetOrigin( ev->GetBoolean ( 7 ) );

	}

void FlyWander::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyWander::Begin
	(
	Actor &self
	)

	{
	original_z = self.origin.z;

	if ( anim.length() )
		self.SetAnim( anim );

	fly.Begin( self );
	fly.SetTurnSpeed( turn_speed );
	fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyWander::Evaluate
	(
	Actor &self
	)

	{
	trace_t trace;
	Vector dir;
	float length;
	int goal_try;
	Vector temp_goal;
	float max_dist = 0.0f;


	dir = goal - self.origin;
	length = dir.length();

	if ( ( next_change_course_time <= level.time ) || ( length < 100.0f ) ) //self.lastmove != STEPMOVE_OK )
		{
		for( goal_try = 0 ; goal_try < 5 ; goal_try++ )
			{
			temp_goal = self.origin;

			temp_goal[0] += G_Random( 10000.0f ) - 5000.0f;
			temp_goal[1] += G_Random( 10000.0f ) - 5000.0f;

			if ( try_to_go_up )
				temp_goal[2] += G_Random( 1000.0f ) - 250.0f;
			else
				temp_goal[2] += G_Random( 100.0f ) - 50.0f;

			trace = G_Trace( self.origin, self.mins, self.maxs, temp_goal, &self, self.edict->clipmask, false, "FlyWander" );

			temp_goal = trace.endpos;

			dir = temp_goal - self.origin;
			length = dir.length();

			if ( length > max_dist )
				{
				max_dist = length;
				goal = temp_goal;

				if ( length > 1000.0f )
					break;
				}
			}

		fly.SetGoalPoint( goal );

		next_change_course_time = level.time + change_course_time;
		}

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyWander::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  FlyToNode Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyToNode, NULL )
	{
		{ &EV_Behavior_Args,	&FlyToNode::SetArgs },
		{ NULL, NULL }
	};

FlyToNode::FlyToNode()
	{
	turn_speed = 10.0;
	anim = "fly";
	speed = 200;
	NumberOfNodes = 0;
	}

void FlyToNode::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
      {
		speed = ev->GetFloat( 2 );
      }

	if ( ev->NumArgs() > 2 )
      {
		turn_speed = ev->GetFloat( 3 );
      }

	if ( ev->NumArgs() > 3 )
      {
		NodeType = ev->GetString( 4 );
      }

	if ( ev->NumArgs() > 4 )
		{
		NumberOfNodes = ev->GetInteger( 5 );
		}
	}

void FlyToNode::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyToNode::Begin
	(
	Actor &self
	)

	{
	
	if ( anim.length() )
		self.SetAnim( anim );

	fly.Begin( self );
	fly.SetTurnSpeed( turn_speed );
	fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyToNode::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector angles;
	str pathnode_name;
	PathNode *goal;
	Vector teleport_position;
	Vector attack_position;

	int bestNode = 0;
	float bestDistance = -1;
	
	for(int i = 0; i <= NumberOfNodes; i++)
		{
		pathnode_name = NodeType + i;

		goal = thePathManager.FindNode( pathnode_name );

		if( goal )
			{
			float distanceTest = self.origin.length()  - goal->origin.length();

			if ( bestDistance < 0.0f )
				{
				bestDistance = distanceTest;
				bestNode = i;
				}
			else if (distanceTest < bestDistance )
				{
				bestDistance = distanceTest;
				bestNode = i;
				}
			}
		}
	
	pathnode_name = NodeType + bestNode;
	goal = thePathManager.FindNode( pathnode_name );
	
	if ( !goal )
		{
		gi.WDPrintf( "Can't find position %s\n", pathnode_name.c_str() );
		return BEHAVIOR_SUCCESS;
		}

	fly.SetGoalPoint( goal->origin );
	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyToNode::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  FlyToRandomNode Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyToRandomNode, NULL )
	{
		{ &EV_Behavior_Args,	&FlyToRandomNode::SetArgs },
		{ NULL, NULL }
	};

FlyToRandomNode::FlyToRandomNode()
	{
	turn_speed = 10.0;
	anim = "fly";
	speed = 200;
	CurrentNode = -1;
	NeedNextNode = true;
	NumberOfNodes = 0;
	goal = NULL;
	}

void FlyToRandomNode::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
      {
		speed = ev->GetFloat( 2 );
      }

	if ( ev->NumArgs() > 2 )
      {
		turn_speed = ev->GetFloat( 3 );
      }

	if ( ev->NumArgs() > 3 )
      {
		NodeType = ev->GetString( 4 );
      }

	if ( ev->NumArgs() > 4 )
		{
		NumberOfNodes = ev->GetInteger( 5 );		
		}	
	}

void FlyToRandomNode::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyToRandomNode::Begin
	(
	Actor &self
	)

	{
	
	if ( anim.length() )
		self.SetAnim( anim );

	fly.Begin( self );
	fly.SetTurnSpeed( turn_speed );
	fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyToRandomNode::Evaluate
	(
	Actor &self
	)

	{
	str pathnode_name;
	Vector dir;
	
	if(NeedNextNode)
		{
		int x = (int)G_Random( (float)NumberOfNodes );
	   if (x == CurrentNode)
			x++;
	
	   CurrentNode = x;

		pathnode_name = NodeType + CurrentNode;
		
	   goal = thePathManager.FindNode( pathnode_name );

		NeedNextNode = false;
		}
   
	if ( !goal )
		{
		gi.WDPrintf( "Can't find position %s\n", pathnode_name.c_str() );
		return BEHAVIOR_SUCCESS;
		}

	fly.SetGoalPoint( goal->origin );
	fly.Evaluate( self );
   
	dir = self.origin - goal->origin;
	
	if ( dir.length() < 100.0f )
		{
		NeedNextNode = true;
		}

	return BEHAVIOR_EVALUATING;
	}

void FlyToRandomNode::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  FlyToNodeNearestPlayer Class Definition

****************************************************************************/
CLASS_DECLARATION( Behavior, FlyToNodeNearestPlayer, NULL )
	{
		{ &EV_Behavior_Args,	&FlyToNodeNearestPlayer::SetArgs },
		{ NULL, NULL }
	};

FlyToNodeNearestPlayer::FlyToNodeNearestPlayer()
	{
	turn_speed = 10.0;
	anim = "fly";
	speed = 200;
	CurrentNode = -1;
	NeedNextNode = true;
	NumberOfNodes = 0;
	goal = NULL;
	}

void FlyToNodeNearestPlayer::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
      {
		speed = ev->GetFloat( 2 );
      }

	if ( ev->NumArgs() > 2 )
      {
		turn_speed = ev->GetFloat( 3 );
      }

	if ( ev->NumArgs() > 3 )
      {
		NodeType = ev->GetString( 4 );
      }

	if ( ev->NumArgs() > 4 )
		{
		NumberOfNodes = ev->GetInteger( 5 );		
		}	
	}

void FlyToNodeNearestPlayer::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyToNodeNearestPlayer::Begin
	(
	Actor &self
	)

	{
	
	if ( anim.length() )
		self.SetAnim( anim );

	fly.Begin( self );
	fly.SetTurnSpeed( turn_speed );
	fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyToNodeNearestPlayer::Evaluate
	(
	Actor &self
	)

	{
	str pathnode_name;
	Vector dir;
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	
	if(NeedNextNode)
		{
		int ClosestNode = 0;
		float CheckLen = -1;
		PathNode* Pnode = 0;

		for ( int i = 1; i <= NumberOfNodes; i++ )
			{
			pathnode_name = NodeType + i;
			Pnode = thePathManager.FindNode(pathnode_name);
	
			if ( !Pnode )
				{
				gi.WDPrintf( "Can't find Pathnode %s\n", pathnode_name.c_str() );
				return BEHAVIOR_SUCCESS;
				}
			
			if( currentEnemy)
				{
				dir = Pnode->origin - currentEnemy->centroid;
				}
			else
				{
				Entity* player = g_entities[0].entity;
				dir = Pnode->origin - player->centroid;
				}

			if ( CheckLen < 0.0f )
				{
				CheckLen = dir.length();
				}
			else
				{
				if(dir.length() < CheckLen)
					{
					CheckLen = dir.length();
					ClosestNode = i;
					}
				}
			}
		
		pathnode_name = NodeType + ClosestNode;
		
	   goal = thePathManager.FindNode( pathnode_name );

		NeedNextNode = false;
		}

	fly.SetGoalPoint( goal->origin );
	fly.Evaluate( self );
   
	dir = self.origin - goal->origin;
	
	if ( dir.length() < 100.0f )
		{
		NeedNextNode = true;
		}

	return BEHAVIOR_EVALUATING;
	}

void FlyToNodeNearestPlayer::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  FlyNodePath Class Definition

****************************************************************************/
CLASS_DECLARATION( Behavior, FlyNodePath, NULL )
	{
		{ &EV_Behavior_Args,		&FlyNodePath::SetArgs },
		{ NULL, NULL }
	};

FlyNodePath::FlyNodePath()
	{
	turn_speed = 10.0f;
	anim = "fly";
	speed = 200.0f;
	CurrentNode = -1;
	NeedNextNode = true;
	NumberOfNodes = 0;
	goal = NULL;
	}

void FlyNodePath::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
      {
		speed = ev->GetFloat( 2 );
      }

	if ( ev->NumArgs() > 2 )
      {
		turn_speed = ev->GetFloat( 3 );
      }

	if ( ev->NumArgs() > 3 )
      {
		NodeType = ev->GetString( 4 );
      }

	if ( ev->NumArgs() > 4 )
		{
		NumberOfNodes = ev->GetInteger( 5 );		
		}	
	}

void FlyNodePath::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyNodePath::Begin
	(
	Actor &self
	)

	{
	
	if ( anim.length() )
		self.SetAnim( anim );

	fly.Begin( self );
	fly.SetTurnSpeed( turn_speed );
	fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyNodePath::Evaluate
	(
	Actor &self
	)

	{
	str pathnode_name;
	Vector dir;
	
	if(NeedNextNode)
		{
	   CurrentNode++;

		if(CurrentNode > NumberOfNodes)
			CurrentNode = 0;

		pathnode_name = NodeType + CurrentNode;
		
	   goal = thePathManager.FindNode( pathnode_name );

		NeedNextNode = false;
		}
   
	if ( !goal )
		{
		gi.WDPrintf( "Can't find position %s\n", pathnode_name.c_str() );
		return BEHAVIOR_SUCCESS;
		}

	fly.SetGoalPoint( goal->origin );
	fly.Evaluate( self );
   
	dir = self.origin - goal->origin;
	
	if ( dir.length() < 100.0f )
		{
		NeedNextNode = true;
		}
	
	if ( (CurrentNode == NumberOfNodes) && NeedNextNode )
		return BEHAVIOR_SUCCESS;
	
	return BEHAVIOR_EVALUATING;
	}

void FlyNodePath::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  FlyCircle Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyCircle, NULL )
	{
		{ &EV_Behavior_Args,	&FlyCircle::SetArgs },
		{ NULL, NULL }
	};

FlyCircle::FlyCircle()
	{
	anim = "fly";	
	fly_clockwise = true;
	circle_player = false;
	original_z = 0.0f;
	}

void FlyCircle::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	fly_clockwise = ev->GetBoolean( 2 );

	if (ev->NumArgs() > 2 )
		fly.setAdjustYawAndRoll( ev->GetBoolean( 3 ) );
	
	if (ev->NumArgs() > 3 )
		circle_player = ev->GetBoolean ( 4 );
	else
		circle_player = false;	
	
	if (ev->NumArgs() > 4 )
		fly.SetSpeed( ev->GetFloat( 5 ) );
	}

void FlyCircle::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyCircle::Begin
	(
	Actor &self
	)

	{
	original_z = self.origin.z;

	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	fly.Begin( self );
	fly.SetTurnSpeed( 5.0f );
	}

BehaviorReturnCode_t	FlyCircle::Evaluate
	(
	Actor &self
	)

	{
   Vector goal;
	trace_t trace;
	Vector dir;
	Vector angle;
	Vector left;
	qboolean too_far = false;
	Vector new_dir;
	Vector fly_dir;
	
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( !self.IsEntityAlive( currentEnemy ) && !circle_player )
      {
      return BEHAVIOR_SUCCESS;
      }


	if ( self.movementSubsystem->getLastMove() == STEPMOVE_OK )
		{
		fly.SetTurnSpeed( 5.0f );

		if (circle_player)
			{
			Player *player = NULL;
			Player *temp_player = NULL;

			// Make sure the player is alive and well
			for(int i = 0; i < game.maxclients; i++)
				{
				temp_player = GetPlayer(i);		
			
				// don't target while player is not in the game or he's in notarget
				if ( temp_player && !( temp_player->flags & FlagNotarget ) )
					{
					player = temp_player;
					break;
					}
				}

			if ( !player )
				return BEHAVIOR_SUCCESS;
						
			dir = player->centroid - self.origin;
			dir.z = 0.0f;

			if ( dir.length() > ( ( self.origin.z - player->centroid.z ) / .5f ) )
				{
				too_far = true;
				}

			}
		else
			{
			dir = currentEnemy->centroid - self.origin;
			dir.z = 0.0f;

			if ( dir.length() > ( ( self.origin.z - currentEnemy->centroid.z ) / 2.0f ) )
				{
				too_far = true;
				}

			}

		angle = dir.toAngles();

		angle.AngleVectors( NULL, &left, NULL );

		if ( fly_clockwise )
			fly_dir = left;
		else
			fly_dir = left * -1.0f;

		dir.normalize();

		if ( too_far )
			{
			new_dir = ( fly_dir * 0.5f ) + ( dir * 0.5f );
			new_dir.normalize();
			}
		else
			{
			new_dir = fly_dir;
			}

		//goal = self.origin + new_dir * 200;
		goal = self.origin + ( new_dir * 700.0f );

		trace = G_Trace( self.origin, self.mins, self.maxs, goal, &self, self.edict->clipmask, false, "FlyCircle" );

		if ( trace.fraction < 1.0f )
			{
			if ( too_far )
				trace.fraction /= 2.0f;

			new_dir = ( fly_dir * trace.fraction ) + ( dir * ( 1.0f - trace.fraction ) );
			new_dir.normalize();

			//goal = self.origin + new_dir * 200;
			goal = self.origin + ( new_dir * 700.0f );
			}
		else
			{
			goal = trace.endpos;
			}

		fly.SetGoalPoint( goal );
		}
	else
		{
		fly.SetTurnSpeed( 20.0f );
		}

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyCircle::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  FlyStrafe Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyStrafe, NULL )
	{
		{ &EV_Behavior_Args,	&FlyStrafe::SetArgs },
		{ NULL, NULL }
	};

FlyStrafe::FlyStrafe()
	{
	anim = "fly";	
	}

void FlyStrafe::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	speed = ev->GetFloat( 2 );
	right = ev->GetBoolean( 3 );
	roll  = ev->GetFloat( 4 );	
	}

void FlyStrafe::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyStrafe::Begin
	(
	Actor &self
	)

	{
	self.SetAnim( anim );
	self.movementSubsystem->setForwardSpeed( speed );

   /*
	if ( right )
		self.FlightDirection = FLY_RIGHT;
	else
		self.FlightDirection = FLY_LEFT;
   */
	}

BehaviorReturnCode_t	FlyStrafe::Evaluate
	(
	Actor &self
	)

	{
 	Vector dir;
	Vector delta;
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	dir = currentEnemy->centroid - self.origin;
	dir = dir.toAngles();

	if ( right )
		dir[ROLL] -= roll;
	else
		dir[ROLL] += roll;

	self.setAngles( dir );
	
   


	return BEHAVIOR_EVALUATING;
	}	
	

void FlyStrafe::End
	(
	Actor &
	)

	{
	//self.FlightDirection = FLY_FORWARD;
	}

/****************************************************************************

  FlyCircleRandomPoint Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, FlyCircleRandomPoint, NULL )
	{
		{ &EV_Behavior_Args,	&FlyCircleRandomPoint::SetArgs },
		{ NULL, NULL }
	};

FlyCircleRandomPoint::FlyCircleRandomPoint()
	{
	anim = "fly";
	//original_z = 0.0f;
	fly_clockwise = true;
	change_course_time = 5;
	try_to_go_up = false;
	next_change_course_time = 0;
	}

void FlyCircleRandomPoint::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	fly_clockwise = ev->GetBoolean( 2 );

	if ( ev->NumArgs() > 2 )
      change_course_time = ev->GetFloat( 3 );
      
	if ( ev->NumArgs() > 3 )
      try_to_go_up = ev->GetBoolean( 4 );
      
	if (ev->NumArgs() > 4 )
		fly.setAdjustYawAndRoll( ev->GetBoolean( 5 ) );	

	if (ev->NumArgs() > 5 )
		fly.SetSpeed( ev->GetFloat( 6 ) );

	}

void FlyCircleRandomPoint::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyCircleRandomPoint::Begin
	(
	Actor &self
	)

	{
	//original_z = self.origin.z;

	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	fly.Begin( self );
	fly.SetTurnSpeed( 5.0f );
	fly.ForceGoal();
	}

BehaviorReturnCode_t	FlyCircleRandomPoint::Evaluate
	(
	Actor &self
	)

	{
   trace_t trace;
	Vector dir;
	Vector angle;
	Vector left;
	qboolean too_far = false;
	Vector new_dir;
	Vector fly_dir;
   float length;
	int goal_try;
	Vector temp_goal;
	float max_dist = 0;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( !self.IsEntityAlive( currentEnemy ) )
      {
      return BEHAVIOR_SUCCESS;
      }
	

	if ( self.movementSubsystem->getLastMove() == STEPMOVE_OK )
		{
		fly.SetTurnSpeed( 5.0f );
		
		if ( next_change_course_time <= level.time ) //self.lastmove != STEPMOVE_OK )
			{
			
			dir = goal - self.origin;
			dir.z = 0;
			length = dir.length();
			
			fly_clockwise = !fly_clockwise;

			for( goal_try = 0 ; goal_try < 5 ; goal_try++ )
				{
							
				temp_goal = self.origin;
				
				//temp_goal[0] += G_Random( 10000 ) - 2500.0;
				//temp_goal[1] += G_Random( 10000 ) - 2500.0;
								
				temp_goal[0] += G_Random( 10000.0f ) + 10000.0f;
				temp_goal[1] += G_Random( 10000.0f ) + 10000.0f;
				if ( try_to_go_up )
					temp_goal[2] += G_Random( 1000.0f ) - 250.0f;
				else
					temp_goal[2] += G_Random( 100.0f ) - 50.0f;

				trace = G_Trace( self.origin, self.mins, self.maxs, temp_goal, &self, self.edict->clipmask, false, "FlyCircleRandomPoint" );

				temp_goal = trace.endpos;

				dir = temp_goal - self.origin;
				length = dir.length();

				if ( length > max_dist )
					{
					max_dist = length;
					goal = temp_goal;

					if ( length > 1000.0f )
					   break;
					}

				}

			next_change_course_time = (level.time + change_course_time);
			}
		
		if ( dir.length() > ( (self.origin.z - goal.z ) / 2.0f ) )
			{
			too_far = true;
			}

		angle = dir.toAngles();

		angle.AngleVectors( NULL, &left, NULL );

		if ( fly_clockwise )
			fly_dir = left;
		else
			fly_dir = left * -1.0f;

		dir.normalize();

		if ( too_far )
			{
		   new_dir = ( fly_dir * 0.5f ) + ( dir * 0.5f );
			new_dir.normalize();
			}
		else
			{
			new_dir = fly_dir;
			}

		goal = self.origin + ( new_dir * 30.0f );
		//goal = self.origin + new_dir * 100;

		
		trace = G_Trace( self.origin, self.mins, self.maxs, goal, &self, self.edict->clipmask, false, "FlyCircleRandomPoint" );

		if ( trace.fraction < 1.0f )
			{
			if ( too_far )
				trace.fraction /= 2.0f;

			new_dir = ( fly_dir * trace.fraction ) + ( dir * ( 1.0f - trace.fraction ) );
			new_dir.normalize();

			//goal = self.origin + new_dir * 200;
			goal = self.origin + ( new_dir * 100.0f );
			}
		else
			{
			goal = trace.endpos;
			}

		fly.SetGoalPoint( goal );
		}
	else
		{
		fly.SetTurnSpeed( 30.0f );
		}
	
	
	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;

	}

void FlyCircleRandomPoint::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  FlyDive Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyDive, NULL )
	{
		{ &EV_Behavior_Args, &FlyDive::SetArgs },
		{ NULL, NULL }
	};

FlyDive::FlyDive()
	{
	anim = "fly";
	speed = 2000;
	damage = 0;
	}

void FlyDive::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
      {
		speed = ev->GetFloat( 2 );
      }

	if ( ev->NumArgs() > 2 )
      {
		damage = ev->GetFloat( 3 );
      }
	}

void FlyDive::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyDive::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
		self.SetAnim( anim );

	fly.Begin( self );

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return;

	if ( !self.IsEntityAlive( currentEnemy ) )
      {
		return;
      }

	goal = currentEnemy->centroid - self.origin;
	goal.normalize();
	goal *= 10000.0f;
	goal += self.origin;	
	
	self.movementSubsystem->setDiveDir( self.origin );

   fly.SetGoalPoint( goal );

	fly.SetTurnSpeed( 100.0f );
	fly.SetSpeed( speed );
	fly.SetRandomAllowed( false );
	fly.ForceGoal();
	}

BehaviorReturnCode_t	FlyDive::Evaluate
	(
	Actor &self
	)

	{
	trace_t trace;
	Vector dir;
	Entity *hit_entity;
	qboolean stuck;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( !self.IsEntityAlive( currentEnemy ) )
      return BEHAVIOR_SUCCESS;

	if ( self.origin.z < ( currentEnemy->origin.z + 50.0f ) )
		return BEHAVIOR_SUCCESS;

	if ( (self.movementSubsystem->getLastMove() == STEPMOVE_STUCK ) || ( self.movementSubsystem->getLastMove() == STEPMOVE_BLOCKED_BY_WATER ) )
		{
		stuck = true;

		dir = self.movementSubsystem->getMoveDir() * 100.0f;

		trace = G_Trace( self.origin, self.mins, self.maxs, self.origin + dir, &self, self.edict->clipmask, false, "FlyDive" );

		if ( trace.entityNum != ENTITYNUM_NONE )
			{
			hit_entity = G_GetEntity( trace.entityNum );

			// Damage entity hit
			//if ( hit_entity->isSubclassOf( Sentient ) )
			if ( hit_entity->takedamage )
				{
				//hit_entity->Damage( &self, &self, damage, Vector (0, 0, 0), Vector (0, 0, 0), Vector (0, 0, 0), 0, 0, MOD_CRUSH );
				dir.normalize();
				hit_entity->Damage( &self, &self, damage, self.centroid, dir, vec_zero, 0, 0, MOD_CRUSH );
				self.AddStateFlag( STATE_FLAG_MELEE_HIT );
				stuck = false;
				}
			}

		// Make sure we really are still stuck

		if ( trace.fraction > 0.05f )
			stuck = false;

		self.angles[PITCH] = 0.0f;
		self.setAngles( self.angles );

		if ( stuck )
			self.AddStateFlag( STATE_FLAG_STUCK );

		return BEHAVIOR_SUCCESS;
		}

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyDive::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}


/****************************************************************************

  FlyCharge Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyCharge, NULL )
	{
		{ &EV_Behavior_Args,	&FlyCharge::SetArgs },
		{ NULL, NULL }
	};

FlyCharge::FlyCharge()
	{
	anim = "fly";
	speed = 2000;
	damage = 0;
	}

void FlyCharge::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
      {
		speed = ev->GetFloat( 2 );
      }

	if ( ev->NumArgs() > 2 )
      {
		damage = ev->GetFloat( 3 );
      }
	}

void FlyCharge::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyCharge::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
		self.SetAnim( anim );

	fly.Begin( self );

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return;

	if ( !self.IsEntityAlive( currentEnemy ) )
      {
		return;
      }

	goal = currentEnemy->centroid - self.origin;
	goal.normalize();
	goal *= 10000.0f;
	goal += self.origin;
	goal.z = self.origin.z;
	
	self.movementSubsystem->setDiveDir( self.origin );

   fly.SetGoalPoint( goal );

	fly.SetTurnSpeed( 100.0f );
	fly.SetSpeed( speed );
	fly.SetRandomAllowed( false );
	fly.ForceGoal();
	}

BehaviorReturnCode_t	FlyCharge::Evaluate
	(
	Actor &self
	)

	{
	trace_t trace;
	Vector dir;
	Entity *hit_entity;
	qboolean stuck;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( !self.IsEntityAlive( currentEnemy ) )
      return BEHAVIOR_SUCCESS;

	if ( ( self.movementSubsystem->getLastMove() == STEPMOVE_STUCK ) || ( self.movementSubsystem->getLastMove() == STEPMOVE_BLOCKED_BY_WATER ) )
		{
		stuck = true;
				
		dir = self.movementSubsystem->getMoveDir() * 100.0f;

		trace = G_Trace( self.origin, self.mins, self.maxs, self.origin + dir, &self, self.edict->clipmask, false, "FlyDive" );

		if ( trace.entityNum != ENTITYNUM_NONE )
			{
			hit_entity = G_GetEntity( trace.entityNum );

			// Damage entity hit
			//if ( hit_entity->isSubclassOf( Sentient ) )
			if ( hit_entity->takedamage )
				{
				//hit_entity->Damage( &self, &self, damage, Vector (0, 0, 0), Vector (0, 0, 0), Vector (0, 0, 0), 0, 0, MOD_CRUSH );
				dir.normalize();
				hit_entity->Damage( &self, &self, damage, self.centroid, dir, vec_zero, 0, 0, MOD_CRUSH );
				self.AddStateFlag( STATE_FLAG_MELEE_HIT );
				stuck = false;
				}
			}

		// Make sure we really are still stuck

		if ( trace.fraction > 0.05f )
			stuck = false;

		self.angles[PITCH] = 0.0f;
		self.setAngles( self.angles );

		if ( stuck )
			self.AddStateFlag( STATE_FLAG_STUCK );

		return BEHAVIOR_SUCCESS;
		}

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyCharge::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}


/****************************************************************************

  FlyClimb Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyClimb, NULL )
	{
		{ &EV_Behavior_Args,	&FlyClimb::SetArgs },
		{ NULL, NULL }
	};

FlyClimb::FlyClimb()
	{
	anim = "fly";
	height = 500;
	speed = 0;
	collision_buffer = 0;
	next_height_check = level.time + 2.0;
	last_check_height = 0.0f;
	}

void FlyClimb::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		height = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		speed = ev->GetFloat( 3 );

	if ( ev->NumArgs() > 3 )
		collision_buffer = ev->GetFloat( 1 );	
	}

void FlyClimb::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyClimb::Begin
	(
	Actor &self
	)

	{
	
	Vector forward;
	trace_t trace;

	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	fly.Begin( self );

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();


	if ( currentEnemy )
		height = currentEnemy->origin.z + height;	   
	else
		height = self.origin.z + height;

	self.angles.AngleVectors( &forward );
	
	//Okay, here's what we are attempting.  We are first going to get our initial point, 
	//this will be our forward vector * height, then add the the height to the the Z.
	//Next we will trace out, to see how far along that vector we got.  We will use our 
	//trace.endpos as a goal position

	//Now, we don't want to use this goal position as a final vector, because if we ran into
	//something, then we will smack it when we get there... this would be bad, so, we run a 
	//a vector from our origin to our goal position and get the length, we then shave some off of
	//this length and multiply it by another vector and add it to the origin, this will get us
	//our final position

	goal = forward * height;
	goal.z = height;
	goal += self.origin;

	trace = G_Trace( self.centroid, self.mins, self.maxs, goal, NULL, MASK_SOLID, false, "flyclimb" );
	goal = trace.endpos;

	Vector dir;
	dir = self.origin - goal;

	float length;	

	length = dir.length();
	
	if ( collision_buffer )
		length -= collision_buffer;
	else
		length -= (length / 10.0f);

	if ( length > 0.0f )
		{
      goal = forward * length;
		goal.z = length;
		goal += self.origin;
		}
		

	//if ( !self.divedir.x && !self.divedir.y && !self.divedir.z )
	//	{
   //   goal = forward * height;
	//   goal.z = height;
	//   goal += self.origin;
	//	trace = G_Trace( self.centroid, self.mins, self.maxs, goal, NULL, MASK_SOLID, false, "flyclimb" );
		
		


	//	}
   //else
	//	{
   //   goal = self.divedir;
	//	goal.z = height;
	//	}
		
	   


	fly.SetTurnSpeed( 10.0f );

	fly.SetGoalPoint( goal );
	
	last_check_height = self.origin.z;

	if ( speed )
		fly.SetSpeed( speed );
	}

BehaviorReturnCode_t	FlyClimb::Evaluate
	(
	Actor &self
	)

	{

	if ( self.origin.z >= height )
      {
		return BEHAVIOR_SUCCESS;
      }

	if ( next_height_check < level.time )
		{
		if ( self.origin.z < ( last_check_height + 25.0f ) )
			return BEHAVIOR_SUCCESS;

		next_height_check = level.time + 2.0f;
		last_check_height = self.origin.z;
		}

	if ( self.movementSubsystem->getLastMove() == STEPMOVE_OK )
		fly.SetGoalPoint( goal );

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void FlyClimb::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  FlySplinePath Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, FlySplinePath, NULL )
	{
		{ &EV_Behavior_Args,	&FlySplinePath::SetArgs },
		{ NULL, NULL }
	};

FlySplinePath::FlySplinePath()
	{
	ent = NULL;
	clamp = true;
	ignoreAngles = false;
	splineAngles = true;	
	havePath = false;
	startTime = 0.0f;
	}

void FlySplinePath::SetArgs
	(
	Event *ev
	)

	{
   ent          = ev->GetEntity( 1 );
   ignoreAngles = ev->GetBoolean( 2 );
	splineAngles = ev->GetBoolean( 3 );
	clamp        = ev->GetBoolean( 4 );	
	}

void FlySplinePath::CreatePath
	(
	SplinePath *path,
	splinetype_t type
	)

	{
	
	SplinePath	*node;
   
	splinePath.Clear();
	splinePath.SetType( type );

	node = path;
	while( node != NULL )
		{
      splinePath.AppendControlPoint( node->origin, node->angles, node->speed );
		node = node->GetNext();

		if ( node == path )
			{
			break;
			}
		}
	
	}
void FlySplinePath::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlySplinePath::Begin
	(
	Actor &self
	)

	{
	
	if ( ent )
		havePath = true;
	else
		return;

	if ( ent->isSubclassOf( SplinePath ) )
		{

		SplinePath* path;
      
		path = (SplinePath*)(Entity*)ent;
		
		if ( clamp )			
			CreatePath( path, SPLINE_CLAMP );
		else
			CreatePath( path, SPLINE_LOOP );
		
		currentNode = path;
      
		if (currentNode->thread != "" )
			{
			if ( !ExecuteThread( currentNode->thread ) )
				{
				gi.Error( ERR_DROP, "FlySplinePath could not run thread '%s' from info_splinepath '%s'\n", 
					currentNode->thread.c_str(), currentNode->targetname.c_str() );
				}
			}
		
		if ( currentNode->triggertarget != "" )
			{
			Entity	*ent;
			Event		*event;

			ent = NULL;
			do
				{
				ent = G_FindTarget( ent, currentNode->triggertarget.c_str() );
				if ( !ent )
					{
					break;
					}
		   	event = new Event( EV_Activate );
				
				Actor* actorPtr = &self;
				Entity* entPtr = (Entity*)actorPtr;
				event->AddEntity( entPtr );
				ent->PostEvent( event, 0.0f );
				}
			while ( 1 );
			}
		
		}
   
	startTime = level.time + self.currentSplineTime;
	
	
	}

BehaviorReturnCode_t	FlySplinePath::Evaluate
	(
	Actor &self
	)

	{
   
	Vector goal;
	Vector dir;
	Vector angles;
	
	if(!havePath)
		return BEHAVIOR_SUCCESS;

	
	goal = splinePath.Eval( (level.time - startTime) );		

	if ( goal == oldGoal )
		{
		self.currentSplineTime = 0;
		return BEHAVIOR_SUCCESS;
		}
		

	oldGoal = goal;
	
	dir = goal - self.origin;
   angles = dir.toAngles();

	self.setOrigin(goal);
	self.setAngles(angles);

	
	return BEHAVIOR_EVALUATING;

	
	}

void FlySplinePath::End
	(
	Actor &self
	)

	{
   self.currentSplineTime = level.time - startTime;
   }

/****************************************************************************

  Land Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, Land, NULL )
	{
		{ &EV_Behavior_Args,	&Land::SetArgs },
		{ NULL, NULL }
	};

Land::Land()
	{
	anim = "fly";
	}

void Land::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	}

void Land::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void Land::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	self.velocity = Vector(0, 0, -40);
	}

BehaviorReturnCode_t	Land::Evaluate
	(
	Actor &self
	)

	{
	self.angles[PITCH] = 0.0f;
	self.angles[ROLL]  = 0.0f;

	self.setAngles( self.angles );

	self.velocity.z -= 20.0f;

	if ( self.velocity.z < -200.0f )
      {
		self.velocity.z = -200.0f;
      }

   if ( !self.groundentity )
      return BEHAVIOR_EVALUATING;

	return BEHAVIOR_SUCCESS;
	}

void Land::End
	(
	Actor &
	)

	{
	}


/****************************************************************************

  Vertical Takeoff Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, VerticalTakeOff, NULL )
	{
		{ &EV_Behavior_Args, &VerticalTakeOff::SetArgs },
		{ NULL, NULL }
	};

VerticalTakeOff::VerticalTakeOff()
	{
	anim = "fly";
	speed = 0;
	height = 0;
	}

void VerticalTakeOff::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	speed = ev->GetFloat( 2 );
	height = ev->GetFloat( 3 );
	}

void VerticalTakeOff::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void VerticalTakeOff::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
      {
		self.SetAnim( anim );
      }
	
	self.velocity.x = 0;
	self.velocity.y = 0;
	self.velocity.z = speed;
	
	}

BehaviorReturnCode_t	VerticalTakeOff::Evaluate
	(
	Actor &self
	)

	{
	self.angles[PITCH] = 0.0f;
	self.angles[ROLL]  = 0.0f;

	self.setAngles( self.angles );

	self.velocity.z += 20.0f;

	if ( self.velocity.z > 200.0f )
      {
		self.velocity.z = 200.0f;
      }

	if( self.origin.z >= height )
		return BEHAVIOR_SUCCESS;
	else
		return BEHAVIOR_EVALUATING;
	}

void VerticalTakeOff::End
	(
	Actor &
	)

	{
	}


/****************************************************************************

  Hover Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Hover, NULL )
	{
		{ &EV_Behavior_Args, &Hover::SetArgs },
		{ NULL, NULL }
	};

Hover::Hover()
	{
	anim = "fly";
	}

void Hover::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	}

void Hover::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void Hover::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
      {
		self.SetAnim( anim );
      }
   
	fly.Begin( self );	
	}

BehaviorReturnCode_t	Hover::Evaluate
	(
	Actor &self
	)

	{
	//self.angles[PITCH] = 0;
	self.angles[ROLL]  = 0;

	self.setAngles( self.angles );
	self.movementSubsystem->setForwardSpeed( 0 );

	return BEHAVIOR_EVALUATING;
	}

void Hover::End
	(
	Actor &self
	)

	{
	fly.End(self);
	}
/****************************************************************************

  Wander Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, Wander, NULL )
	{
		{ &EV_Behavior_Args,	&Wander::SetArgs },
		{ NULL, NULL }
	};

void Wander::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	}

void Wander::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nseek:\n" );

	}

void Wander::Begin
	(
	Actor &self
	)

	{
   avoidtime = 0;
   avoidvec = vec_zero;

	if ( anim.length() )
		{
		self.SetAnim( anim );
		}
	}


BehaviorReturnCode_t	Wander::Evaluate
	(
	Actor &self
	)

	{
   if ( ( self.movementSubsystem->getLastMove() != STEPMOVE_OK ) || ( avoidtime <= level.time ) )
      {
      Vector dir;
      Vector ang;
      float time;

      time = 5.0f;
		//self.Chatter( "snd_idle", 4 );
      avoidvec = ChooseRandomDirection( self, avoidvec, &time, 0, false );
      self.movementSubsystem->setMovingBackwards( false );
      avoidtime = level.time + time;
      }

	float maxSpeed = 100.0f;
	if ( self.movementSubsystem->getMoveSpeed() != 1.0f )
		{	
		maxSpeed = self.movementSubsystem->getMoveSpeed();
		}

	self.movementSubsystem->Accelerate( 
			self.movementSubsystem->SteerTowardsPoint( avoidvec, vec_zero, self.movementSubsystem->getMoveDir(), maxSpeed)
		);

	return BEHAVIOR_EVALUATING;
	}

void Wander::End
	(
	Actor &
	)

	{
	}

/****************************************************************************

  GetCloseToEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, GetCloseToEnemy, NULL )
	{
		{ &EV_Behavior_Args,	&GetCloseToEnemy::SetArgs },
		{ NULL, NULL }
	};

void GetCloseToEnemy::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		forever = ev->GetBoolean( 2 );
	else
		forever = true;
	}

void GetCloseToEnemy::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );
	}

void GetCloseToEnemy::Begin
	(
	Actor &self
	)

	{
   Vector dir;
	dir = self.movementSubsystem->getAnimDir();
	dir = dir.toAngles();
	self.setAngles( dir );

   if ( !anim.length() )
      {
		anim = "run";
      }

	if ( anim != self.animname || self.newanim.length() )
      {
		self.SetAnim( anim, EV_Actor_NotifyBehavior );
      }

	chase.Begin( self );
	wander.Begin( self );

	next_think_time = 0;
   
	}

BehaviorReturnCode_t	GetCloseToEnemy::Evaluate
	(
	Actor &self
	)

	{
	qboolean result;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( next_think_time <= level.time )
		{
		if ( self.groundentity && ( self.groundentity->s.number == currentEnemy->entnum ) )
			{
			wander.Evaluate( self );
			result = true;
			}
		else
			{
			float radius=96.0f;
			chase.SetGoal( currentEnemy, radius, self );			

			result = chase.Evaluate( self );
			}

		if ( self.GetActorFlag( ACTOR_FLAG_SIMPLE_PATHFINDING ) )
			next_think_time = level.time + ( 2.0f * FRAMETIME );
		else
			next_think_time = 0.0f;
		}
	else
		result = true;

	if ( !forever && !result )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void GetCloseToEnemy::End
	(
	Actor &self
	)

	{
	chase.End( self );
	wander.End( self );
	}

/****************************************************************************

  GetCloseToPlayer Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, GetCloseToPlayer, NULL )
	{
		{ &EV_Behavior_Args,	&GetCloseToPlayer::SetArgs },
		{ NULL, NULL }
	};

void GetCloseToPlayer::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		forever = ev->GetBoolean( 2 );
	else
		forever = true;

	if ( ev->NumArgs() > 2 )
		speed = ev->GetFloat( 3 );
	else
		speed = 100;
	}

void GetCloseToPlayer::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );
	}

void GetCloseToPlayer::Begin
	(
	Actor &self
	)

	{
	if ( !anim.length() )
      {
		anim = "run";
      }

	if ( anim != self.animname || self.newanim.length() )
      {
		self.SetAnim( anim, EV_Actor_NotifyBehavior );
      }

	chase.Begin( self );
	wander.Begin( self );
	
	self.movementSubsystem->setForwardSpeed( speed );

	next_think_time = 0;
	}

BehaviorReturnCode_t	GetCloseToPlayer::Evaluate
	(
	Actor &self
	)

	{
	qboolean result;
	
	Player *player = NULL;
	Player *temp_player = NULL;
	// Make sure the player is alive and well
	for(int i = 0; i < game.maxclients; i++)
		{
		temp_player = GetPlayer(i);		
		
		// don't target while player is not in the game or he's in notarget
		if( temp_player && !( temp_player->flags & FlagNotarget ) )
			{
			player = temp_player;
			break;
			}
		}

	if ( !player )
		return BEHAVIOR_SUCCESS;

	if ( next_think_time <= level.time )
		{
		if ( self.groundentity && ( self.groundentity->s.number == player->entnum ) )
			{
			wander.Evaluate( self );
			result = true;
			}
		else
			{
			float radius=96.0f;
			chase.SetGoal( player, radius, self );			

			result = chase.Evaluate( self );
			}

		if ( self.GetActorFlag( ACTOR_FLAG_SIMPLE_PATHFINDING ) )
			next_think_time = level.time + ( 2.0f * FRAMETIME );
		else
			next_think_time = 0.0f;
		}
	else
		result = true;

	if ( !forever && !result )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void GetCloseToPlayer::End
	(
	Actor &self
	)

	{
	chase.End( self );
	wander.End( self );
	self.movementSubsystem->setForwardSpeed( 0 );
	}

/****************************************************************************

  RetreatFromEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, RetreatFromEnemy, NULL )
	{
		{ &EV_Behavior_Args, &RetreatFromEnemy::SetArgs },
		{ NULL, NULL }
	};

void RetreatFromEnemy::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		forever = ev->GetBoolean( 2 );
	else
		forever = true;
	}

void RetreatFromEnemy::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );
	}

void RetreatFromEnemy::Begin
	(
	Actor &self
	)

	{
	if ( !anim.length() )
      {
		anim = "run";
      }

	if ( anim != self.animname || self.newanim.length() )
      {
		self.SetAnim( anim, EV_Actor_NotifyBehavior );
      }

	chase.Begin( self );
	wander.Begin( self );

	next_think_time = 0;
	}

BehaviorReturnCode_t	RetreatFromEnemy::Evaluate
	(
	Actor &self
	)

	{
	qboolean result;
	Entity *currentEnemy;

	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( next_think_time <= level.time )
		{
		if ( self.groundentity && ( self.groundentity->s.number == currentEnemy->entnum ) )
			{
			wander.Evaluate( self );
			result = true;
			}
		else
			{
			
			float currentZ = self.centroid.z; 
			Vector FleeVec = (self.centroid - currentEnemy->centroid) + self.centroid;
			FleeVec.z = currentZ;

			trace_t trace;
			trace = G_Trace( self.centroid, self.mins, self.maxs, FleeVec, NULL, MASK_SOLID, false, "RetreatFromEnemy" );
			
			float radius=96.0f;
			chase.SetGoal( trace.endpos, radius, self );

			result = chase.Evaluate( self );
			}

		if ( self.GetActorFlag( ACTOR_FLAG_SIMPLE_PATHFINDING ) )
			next_think_time = level.time + ( 2.0f * FRAMETIME );
		else
			next_think_time = 0.0f;
		}
	else
		result = true;

	if ( !forever && !result )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void RetreatFromEnemy::End
	(
	Actor &self
	)

	{
	chase.End( self );
	wander.End( self );
	}

/****************************************************************************

  GotoDeadEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, GotoDeadEnemy, NULL )
	{
		{ &EV_Behavior_Args,	&GotoDeadEnemy::SetArgs },
		{ NULL, NULL }
	};

void GotoDeadEnemy::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	}

void GotoDeadEnemy::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );
	}

void GotoDeadEnemy::Begin
	(
	Actor &self
	)

	{
	if ( !anim.length() )
      {
		anim = "run";
      }

	if ( anim != self.animname || self.newanim.length() )
      {
		self.SetAnim( anim, EV_Actor_NotifyBehavior );
      }

	chase.Begin( self );
	}

BehaviorReturnCode_t	GotoDeadEnemy::Evaluate
	(
	Actor &self
	)

	{
	qboolean result;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( !currentEnemy->deadflag )
		return BEHAVIOR_SUCCESS;

	float radius=96.0f;
	chase.SetGoal( currentEnemy->origin, radius, self );

	result = chase.Evaluate( self );

	if ( !result )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void GotoDeadEnemy::End
	(
	Actor &self
	)

	{
	chase.End( self );
	}

/****************************************************************************

  Investigate Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Investigate, NULL )
	{
		{ &EV_Behavior_Args,	&Investigate::SetArgs },
		{ NULL, NULL }
	};

#define INVESTIGATE_MODE_INVESTIGATE	0
#define INVESTIGATE_MODE_LOOKAROUND		1
#define INVESTIGATE_MODE_RETURN			2
#define INVESTIGATE_MODE_TURN2			3

Investigate::Investigate()
	{
	investigate_time = 10.0f;
	moveanim = "run";
	lookaroundanim = "idle";
	curioustime = 0.0f;
	lookaroundtime = 2.0f;
	mode = INVESTIGATE_MODE_INVESTIGATE;
	return_to_original_location = true;
	start_yaw = 0.0f;
	}

void Investigate::SetArgs
	(
	Event *ev
	)

	{
	moveanim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		investigate_time = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		return_to_original_location = ev->GetBoolean( 3 );
	else
		return_to_original_location = true;

	if( ev->NumArgs() > 3 )
		lookaroundanim = ev->GetString( 4 );

	if( ev->NumArgs() > 4 )
		lookaroundtime = ev->GetFloat( 5 );
	}

void Investigate::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );
   gi.Printf( "\nanim: %s\n", moveanim.c_str() );
   gi.Printf( "curioustime: %f\n", curioustime );
   gi.Printf( "goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z );
	}

void Investigate::Begin(
	Actor &self
	)
	
{
// Note: the state machine for this character needs to use "forgetenemies" or it will get
//  stuck in a loop.

	Vector trace_start_pos;
	Vector trace_end_pos;
	PathNode *goal_node;
	
	// Find the goal position
	if( self.sensoryPerception->GetLastSoundType() == SOUNDTYPE_NONE )
	{
		EntityPtr enemy = self.enemyManager->GetCurrentEnemy();
		if( enemy != NULL )
		{
			// Didn't hear a sound... must've seen someone
			goal = enemy->origin;
		}
		else
		{
			// Didn't hear or see anything?  wtf?!  Just hang out where you are, ai.
			goal = self.origin; 
		}
	}
	else
	{
		// Heard a sound
		goal = self.sensoryPerception->GetNoisePosition();
	}
	
	// Try to find a pathnode close to the goal position to ensure good pathfinding
	
	goal_node = self.NearestNodeInPVS( goal );
	if ( goal_node )
	{
		goal = goal_node->origin;
	}
	
	// Let anyone else who cares know that this guy is investigating
	
	self.SetActorFlag( ACTOR_FLAG_INVESTIGATING, true );
	
	// Only search for a set amount of time before returning to non-investigative state
	
	curioustime = level.time + investigate_time;

	// Make some sounds?
	
	self.Chatter( "snd_investigate" );
	
	// Setup movement code to move toward the goal position
	
	chase.Begin( self );
	chase.SetGoal( goal, 96.0f, self );

	// Turn toward ???

	turnto.Begin( self );
	
	// Don't start movement anim if it's so close there will be no actual movement
	
	if ( moveanim.length() && ( Vector::DistanceXY( goal, self.origin ) >= 100.0f ) )
	{
		self.SetAnim( moveanim );
	}
	
	start_pos = self.origin;
	start_yaw = self.angles[YAW];
}

BehaviorReturnCode_t Investigate::Evaluate(
	Actor &self
	)
{
	Vector dir;
	Vector angles;
	Steering::ReturnValue steeringResult;
	
	switch ( mode )
	{
	// Go to spot where the noise was heard
	case INVESTIGATE_MODE_INVESTIGATE:
		
		// Go to next mode if time elapsed or pathing failed
		
		steeringResult = chase.Evaluate( self );
		if ( (steeringResult != Steering::EVALUATING) || curioustime < level.time )
		{
			mode = INVESTIGATE_MODE_LOOKAROUND;
			self.SetAnim( lookaroundanim );
			lookaroundtime_end = level.time + lookaroundtime;
		}
		else
		{
			// Go to next mode if close enough to goal position
			
			float dist = Vector::DistanceXY( goal, self.origin );
			if( dist < 100.0f )
			{
				mode = INVESTIGATE_MODE_LOOKAROUND;
				self.SetAnim( lookaroundanim );
				lookaroundtime_end = level.time + lookaroundtime;
			}
		}
		
		break;
		
	// look around for stuff for a second
	case INVESTIGATE_MODE_LOOKAROUND:
		
		if( level.time > lookaroundtime_end )
		{
			curioustime = level.time + (investigate_time * 2.0f );
			mode = INVESTIGATE_MODE_RETURN;
			self.SetAnim( moveanim );
		}
		
		break;
		
	// go back from whence you came, foul beast
	case INVESTIGATE_MODE_RETURN :
		
		if ( !return_to_original_location )
			return BEHAVIOR_SUCCESS;
		
		// Return back to our original position
		chase.SetGoal( start_pos, 96.0f, self );
		
		if ( chase.Evaluate( self ) != Steering::EVALUATING )
			mode = INVESTIGATE_MODE_TURN2;
		
		if ( curioustime < level.time )
			return BEHAVIOR_SUCCESS;
		
		break;

	// got stuck going back to start position, just face original direction?
	case INVESTIGATE_MODE_TURN2 :
		
		self.SetAnim( "idle" );
		
		// Turn back to our original direction
		
		turnto.SetDirection( start_yaw );
		if ( !turnto.Evaluate( self ) )
			return BEHAVIOR_SUCCESS;
		
		break;
	}
	
	return BEHAVIOR_EVALUATING;
}

void Investigate::End
(
	Actor &self
	)
	
{
	self.SetActorFlag( ACTOR_FLAG_INVESTIGATING, false );
	self.SetActorFlag( ACTOR_FLAG_NOISE_HEARD, false );
	chase.End( self );
	turnto.End( self );
}

/****************************************************************************

  TurnInvestigate Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, TurnInvestigate, NULL )
	{
		{ &EV_Behavior_Args,	&TurnInvestigate::SetArgs },
		{ NULL, NULL }
	};

void TurnInvestigate::SetArgs
	(
	Event *ev
	)

	{
	left_anim  = ev->GetString( 1 );
	right_anim = ev->GetString( 2 );
	turn_speed = ev->GetFloat( 3 );
	}

void TurnInvestigate::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "goal: ( %f, %f, %f )\n", goal.x, goal.y, goal.z );
	}

void TurnInvestigate::Begin
	(
	Actor &self
	)

	{
	goal = self.sensoryPerception->GetNoisePosition();
	self.SetActorFlag( ACTOR_FLAG_INVESTIGATING, true );
	self.SetAnim( "idle" );
	}

BehaviorReturnCode_t	TurnInvestigate::Evaluate
	(
	Actor &self
	)

	{
	str turn_anim_name;

	Vector dir;
	self.angles.AngleVectors( &dir );

	Vector newSteeringForce = self.movementSubsystem->SteerTowardsPoint( goal, vec_zero, dir, 1.0f);

	// See if we have turned all of the way to the noise position

	if ( ( newSteeringForce[YAW] < .5f ) && ( newSteeringForce[YAW] > -.5f ) )
		return BEHAVIOR_SUCCESS;

	// Make sure we are not turning faster than out turn speed
	if ( newSteeringForce[YAW] > turn_speed )
		{
		newSteeringForce[YAW] = turn_speed;
		}

	if ( newSteeringForce[YAW] < -turn_speed )
		{
		newSteeringForce[YAW] = -turn_speed;
		}

	newSteeringForce[PITCH] = 0.0f;
	newSteeringForce[ROLL]  = 0.0f;

	// Set the correct animation (left or right)

	if ( newSteeringForce[YAW] > 0.0f )
		turn_anim_name = left_anim;
	else
		turn_anim_name = right_anim;

	if ( turn_anim_name != self.animname )
		self.SetAnim( turn_anim_name );

	// Actually turn here

	self.movementSubsystem->Accelerate( newSteeringForce );

	return BEHAVIOR_EVALUATING;
	}

void TurnInvestigate::End
	(
	Actor &self
	)

	{
	self.SetActorFlag( ACTOR_FLAG_INVESTIGATING, false );
	self.SetActorFlag( ACTOR_FLAG_NOISE_HEARD, false );
	}

/****************************************************************************

  TurnToEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, TurnToEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&TurnToEnemy::SetArgs },
		{ &EV_Behavior_AnimDone,		&TurnToEnemy::AnimDone },
		{ NULL, NULL }
	};

void TurnToEnemy::SetArgs
	(
	Event *ev
	)

	{
	left_anim  = ev->GetString( 1 );
	right_anim = ev->GetString( 2 );
	turn_speed = ev->GetFloat( 3 );
	forever    = ev->GetBoolean( 4 );

	if ( ev->NumArgs() > 4 )
		use_last_known_position = ev->GetBoolean( 5 );
	else
		use_last_known_position = false;
	}

void TurnToEnemy::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void TurnToEnemy::Begin
	(
	Actor &self
	)

	{

	self.SetAnim( "idle" );
	}

void TurnToEnemy::AnimDone
	(
	Event *
	)

	{
	anim_done = true;
	}


BehaviorReturnCode_t	TurnToEnemy::Evaluate
	(
	Actor &self
	)

	{

	Entity *currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	Vector dir;
	self.angles.AngleVectors( &dir );

	Vector targetPosition=currentEnemy->origin;
	if ( use_last_known_position )
		{
		targetPosition = self.last_known_enemy_pos;
		}

	Vector newSteeringForce = self.movementSubsystem->SteerTowardsPoint( targetPosition, vec_zero, dir, 1.0f );

	// See if we have turned all of the way to the enemy position
	if ( ( newSteeringForce[YAW] < .5f ) && ( newSteeringForce[YAW] > -.5f ) )
		newSteeringForce[YAW] = 0.0f;

	// Make sure we are not turning faster than out turn speed

	if ( newSteeringForce[YAW] > turn_speed )
		{
		newSteeringForce[YAW] = turn_speed;
		}

	if ( newSteeringForce[YAW] < -turn_speed )
		{
		newSteeringForce[YAW] = -turn_speed;
		}

	newSteeringForce[PITCH] = 0.0f;
	newSteeringForce[ROLL]  = 0.0f;

	// Set the correct animation (left or right)
	str turn_anim_name;
	if ( newSteeringForce[YAW] > 0.0f )
		turn_anim_name = left_anim;
	else if ( newSteeringForce[YAW] < 0.0f )
		turn_anim_name = right_anim;
	else if ( anim_done )
		turn_anim_name = "idle";
	else
		turn_anim_name = self.animname.c_str();

	if ( turn_anim_name != self.animname )
		self.SetAnim( turn_anim_name, EV_Actor_NotifyBehavior );

	// Actually turn here

	self.movementSubsystem->Accelerate( newSteeringForce );

	// See if we have turned all of the way to the enemy position

	if ( ( newSteeringForce[YAW] < turn_speed ) && ( newSteeringForce[YAW] > -turn_speed ) && !forever )
		return BEHAVIOR_SUCCESS;

	anim_done = false;

	return BEHAVIOR_EVALUATING;
	}

void TurnToEnemy::End
	(
	Actor &
	)

	{
	}

/****************************************************************************

  PickupEntity Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, PickupEntity, NULL )
	{
		{ &EV_Behavior_Args,		&PickupEntity::SetArgs },
		{ &EV_Behavior_AnimDone,	&PickupEntity::AnimDone },
		{ NULL, NULL }
	};

void PickupEntity::SetArgs
	(
	Event *ev
	)

	{
	ent_to_pickup    = ev->GetEntity( 1 );
	pickup_anim_name = ev->GetString( 2 );
	}

void PickupEntity::Begin
	(
	Actor &self
	)

	{
	anim_done = false;
	self.pickup_ent = ent_to_pickup;

	self.SetAnim( pickup_anim_name.c_str(), EV_Actor_NotifyBehavior );
	}

void PickupEntity::AnimDone
	(
	Event *
	)

	{
	anim_done = true;
	}


BehaviorReturnCode_t	PickupEntity::Evaluate
	(
	Actor &
	)

	{
	if ( !ent_to_pickup )
		return BEHAVIOR_SUCCESS;

	if ( anim_done )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void PickupEntity::End
	(
	Actor &self
	)

	{
	self.SetAnim( "idle" );
	self.pickup_ent = NULL;
	}

/****************************************************************************

  ThrowEntity Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, ThrowEntity, NULL )
	{
		{ &EV_Behavior_Args,		&ThrowEntity::SetArgs },
		{ &EV_Behavior_AnimDone,	&ThrowEntity::AnimDone },
		{ NULL, NULL }
	};

void ThrowEntity::SetArgs
	(
	Event *ev
	)

	{
	throw_anim_name = ev->GetString( 1 );
	}

void ThrowEntity::Begin
	(
	Actor &self
	)

	{
	anim_done = false;
	self.SetAnim( throw_anim_name, EV_Actor_NotifyBehavior );
	}

void ThrowEntity::AnimDone
	(
	Event *
	)

	{
	anim_done = true;
	}


BehaviorReturnCode_t	ThrowEntity::Evaluate
	(
	Actor &
	)

	{
	if ( anim_done )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
	}

void ThrowEntity::End
	(
	Actor &self
	)

	{
	self.SetAnim( "idle" );
	}



/****************************************************************************
*****************************************************************************

  Behaviors for specific creatures

*****************************************************************************
****************************************************************************/



/****************************************************************************

  BurrowAttack Class Definition

****************************************************************************/

#define BURROW_MODE_MOVING  0
#define BURROW_MODE_ATTACK  1

CLASS_DECLARATION( Behavior, BurrowAttack, NULL )
	{
		{ NULL, NULL }
	};

void BurrowAttack::SetArgs
	(
	Event *ev
	)

	{
	use_last_known_position = ev->GetBoolean( 1 );
	}

void BurrowAttack::Begin
	(
	Actor &self
	)

	{
	Vector attack_dir;
	Vector start_pos;
	Vector end_pos;
	trace_t trace;

	if ( self.animname != "idle_down" )
		self.SetAnim( "idle_down" );

	// Setup our goal point
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if ( currentEnemy )
		{
		if ( use_last_known_position )
			goal = self.last_known_enemy_pos;
		else
			goal = currentEnemy->origin;

		start_pos = goal + Vector( "0 0 10" );
		end_pos = start_pos + Vector( "0 0 -250" );

		trace = G_Trace( start_pos, vec_zero, vec_zero, end_pos, NULL, MASK_DEADSOLID, false, "BurrowAttack::Begin" );

		goal = trace.endpos;
		}

	// Setup our starting point, a little ways in front of our origin

	attack_dir = goal - self.origin;

	too_close = false;

	if ( attack_dir.length() < 300.0f )
		too_close = true;

	attack_dir.normalize();

	attack_origin = self.origin + ( attack_dir * 100.0f );

	burrow_mode = BURROW_MODE_MOVING;

	stage = self.stage;

	burrow_speed = 80.0f;

	if ( stage == 3 )
		{
		attacks_left = 2;
		}
	else if ( stage == 4 )
		{
		attacks_left = 3 + (int)G_Random( 3.0f );
		burrow_speed = 120.0f;
		}
	}

BehaviorReturnCode_t	BurrowAttack::Evaluate
	(
	Actor &self
	)

	{
	Vector attack_dir;
	Vector new_origin;
	float total_dist;
	Vector start_pos;
	Vector end_pos;
	Entity *dirt;
	trace_t trace;
	Vector temp_angles;
	int cont;
	Vector temp_endpos;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( too_close )
		return BEHAVIOR_SUCCESS;

	switch ( burrow_mode )
		{
		case BURROW_MODE_MOVING :

			attack_dir = goal - attack_origin;
			total_dist = attack_dir.length();
			attack_dir.normalize();

			if ( total_dist < burrow_speed )
				{
				new_origin = goal;
				}
			else
				{
				new_origin = attack_origin + ( attack_dir * burrow_speed );
				total_dist = burrow_speed;
				}

			// Spawn in dirt or water

			start_pos = attack_origin + attack_dir + Vector( "0 0 10" );
			end_pos = start_pos + Vector( "0 0 -250" );

			trace = G_Trace( start_pos, vec_zero, vec_zero, end_pos, NULL, MASK_DEADSOLID | MASK_WATER, false, "BurrowAttack" );

			temp_endpos = trace.endpos;
			temp_endpos -=  Vector( "0 0 5" );
			cont = gi.pointcontents( temp_endpos, 0 );

			dirt = new Entity( EntityCreateFlagAnimate );

			dirt->setOrigin( trace.endpos );

			temp_angles = vec_zero;

			if (cont & MASK_WATER)
				dirt->setModel( "fx_splashsmall.tik" );
			else
				dirt->setModel( "fx_dirtcloud.tik" );

			dirt->setAngles( temp_angles );

			dirt->animate->RandomAnimate( "idle" );

			dirt->PostEvent( EV_Remove, 5.0f );

			attack_origin = new_origin;

			if ( attack_origin == goal )
				{
				// Got to our goal position, do attack

				if ( stage == 1 )
					{
					SpawnArm( self, leg1, attack_origin + Vector( " 25  25 0" ), "attack1", 0.0f );
					SpawnArm( self, leg2, attack_origin + Vector( " 25 -25 0" ), "attack1", 0.0f );
					SpawnArm( self, leg3, attack_origin + Vector( "-25  25 0" ), "attack1", 0.0f );
					SpawnArm( self, leg4, attack_origin + Vector( "-25 -25 0" ), "attack1", 0.0f );
					}
				else if ( stage == 2 )
					{
					SpawnArm( self, leg1, attack_origin + Vector( " 25   0 0" ), "attack2",   0.0f );
					SpawnArm( self, leg2, attack_origin + Vector( "-25  25 0" ), "attack2", 120.0f );
					SpawnArm( self, leg3, attack_origin + Vector( "-25 -25 0" ), "attack2", 240.0f );
					}
				else
					{
					SpawnArm( self, leg1, attack_origin, "attack1", 0.0f );
					}

				burrow_mode = BURROW_MODE_ATTACK;
				}

			break;
		case BURROW_MODE_ATTACK :

			// Wait until all of the legs are done

			if ( !leg1 && !leg2 && !leg3 && !leg4 )
				{
				if ( self.animname != "idle_down" )
					self.SetAnim( "idle_down" );

				if ( ( stage == 1 ) || ( stage == 2 ) )
					{
					return BEHAVIOR_SUCCESS;
					}
				else
					{
					attacks_left--;

					if ( attacks_left > 0 )
						{
						if ( use_last_known_position )
							if ( self.sensoryPerception->CanSeeEntity( &self , currentEnemy , true , true ) )
								goal = currentEnemy->origin;
							else
								return BEHAVIOR_SUCCESS;
						else
							goal = currentEnemy->origin;

						burrow_mode = BURROW_MODE_MOVING;
						}
					else
						{
						return BEHAVIOR_SUCCESS;
						}
					}
				}

			break;
		}

	return BEHAVIOR_EVALUATING;
	}

void BurrowAttack::SpawnArm
	(
	Actor &self,
	EntityPtr &leg,
	const Vector &original_arm_origin,
	const char *anim_name,
	float angle
	)

	{
	Vector angles;
	trace_t trace;
	Vector start_pos;
	Vector end_pos;
	str anim_to_play;
	Entity *leg_animate_ptr;
	Vector dir;
	Entity *dirt;
	Vector arm_origin;


	// Find correct spot to spawn

	arm_origin = original_arm_origin;

	arm_origin[2] = -575.0f;

	start_pos = arm_origin + Vector( "0 0 64" );
	end_pos = arm_origin - Vector( "0 0 100" );

	//trace = G_Trace( start_pos, Vector(-5, -5, 0), Vector(5, 5, 0), end_pos, NULL, MASK_DEADSOLID, false, "BurrowAttack" );
	trace = G_Trace( start_pos, Vector(-10.0f, -10.0f, 0.0f), Vector(10.0f, 10.0f, 0.0f), end_pos, NULL, MASK_DEADSOLID, false, "BurrowAttack" );

	arm_origin = trace.endpos;

	// Make sure can spawn here

	end_pos = arm_origin + Vector( "0 0 50" );

	trace = G_Trace( arm_origin, Vector(-10.0f, -10.0f, 0.0f), Vector(10.0f, 10.0f, 0.0f), end_pos, NULL, MASK_DEADSOLID, false, "BurrowAttack" );

	if ( ( trace.fraction < 1.0f ) || trace.startsolid || trace.allsolid )
		{
		if ( trace.entityNum == ENTITYNUM_WORLD || !( trace.ent && trace.ent->entity && trace.ent->entity->takedamage ) )
			return;
		}

	// Spawn some dirt

	dirt = new Entity( EntityCreateFlagAnimate );

	dirt->setOrigin( arm_origin );
	dirt->setModel( "fx_dirtcloud.tik" );
	dirt->setAngles( vec_zero );
	dirt->animate->RandomAnimate( "idle" );
	dirt->PostEvent( EV_Remove, 5.0f );

	// Spawn leg

	leg = new Entity( EntityCreateFlagAnimate );

	leg->setModel( "vmama_arm.tik" );
	leg->setOrigin( arm_origin );

	leg->ProcessPendingEvents();

	//leg->edict->clipmask	= MASK_MONSTERSOLID;
	leg->setContents( 0 );
	leg->setSolidType( SOLID_NOT );

	leg->PostEvent( EV_BecomeNonSolid, 0.0f );

	angles = vec_zero;
	angles[YAW] = angle;

	leg->setAngles( angles );

	anim_to_play = anim_name;

	// See if we should get stuck or not

	if ( strcmp( anim_name, "attack1" ) == 0 )
		{
		end_pos = arm_origin + Vector( "0 0 250" );

		leg_animate_ptr = leg;

		trace = G_Trace( arm_origin, Vector(-5.0f, -5.0f, 0.0f), Vector(5.0f, 5.0f, 0.0f), end_pos, leg_animate_ptr, MASK_DEADSOLID, false, "BurrowAttack" );

		if ( trace.fraction != 1.0f )
			{
			if ( self.animname != "struggle" )
				self.SetAnim( "struggle" );

			anim_to_play = "getstuck";
			}
		}

	// Damage entities in way

	if ( strcmp( anim_name, "attack1" ) == 0 )
		{
		start_pos = arm_origin;
		end_pos = arm_origin + Vector( "0 0 250" );

		dir = Vector ( G_CRandom( 5.0f ), G_CRandom( 5.0f ), 10.0f );
		}
	else
		{
		start_pos = arm_origin + Vector( "0 0 10" );

		angles.AngleVectors( &dir );

		end_pos = start_pos + ( dir * 250.0f );
		}

	leg_animate_ptr = leg;

	MeleeAttack( start_pos, end_pos, 50.0f, &self, MOD_IMPALE, 10.0f, 0.0f, 0.0f, 100.0f );

	leg->animate->RandomAnimate( anim_to_play.c_str(), EV_Remove );
	}

void BurrowAttack::End
	(
	Actor &
	)

	{
	}

/****************************************************************************

  CircleEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, CircleEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&CircleEnemy::SetArgs },
		{ NULL, NULL }
	};

void CircleEnemy::SetArgs
	(
	Event *ev
	)

	{
	center_part_name = ev->GetString( 1 );
	}

void CircleEnemy::Begin
	(
	Actor &self
	)

	{
	ent_to_circle = self.enemyManager->GetCurrentEnemy();
	last_angle_change = 0;
	}

float CircleEnemy::GetAngleDiff
	(
	const Actor &self,
	const Actor *center_actor,
	const Vector &origin
	)

	{
	Vector dir;
	Vector enemy_angles;
	Vector actor_angles;
	float angle_diff;

	Q_UNUSED(self);

	if ( !ent_to_circle )
		return 0.0f;

	dir = ent_to_circle->origin - center_actor->origin;
	enemy_angles = dir.toAngles();

	dir = origin - center_actor->origin;
	actor_angles = dir.toAngles();

	angle_diff = AngleDelta( actor_angles[YAW], enemy_angles[YAW] );

	return angle_diff;
	}

#define MAX_CIRCLE_ACCELERATION  .125
#define MAX_CIRCLE_VELOCITY  10

BehaviorReturnCode_t	CircleEnemy::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Actor *center_actor;
	Vector actor_angles;
	float angle_diff;
	float other_angle_diff;
	float abs_angle_diff;
	float other_abs_angle_diff = 180.0f;
	float angle_change = MAX_CIRCLE_VELOCITY;
	float length;
	float real_angle_change;
	Actor *other;


	if ( !ent_to_circle )
		return BEHAVIOR_SUCCESS;

	center_actor = self.FindPartActor( center_part_name.c_str() );

	if ( !center_actor )
		return BEHAVIOR_SUCCESS;

	angle_diff = GetAngleDiff( self, center_actor, self.origin );

	if ( angle_diff < 0.0f )
		abs_angle_diff = -angle_diff;
	else
		abs_angle_diff = angle_diff;

	other = self.FindPartActor( self.part_name );

	if ( other )
		{
		other_angle_diff = GetAngleDiff( self, center_actor, other->origin );

		if ( other_angle_diff < 0.0f )
			other_abs_angle_diff = -other_angle_diff;
		else
			other_abs_angle_diff = other_angle_diff;
		}

	if ( abs_angle_diff < other_abs_angle_diff )
		{
		// Turn towards enemy

		if ( abs_angle_diff < angle_change )
			angle_change = abs_angle_diff;

		if ( angle_diff < 0.0f )
			real_angle_change = angle_change;
		else
			real_angle_change = -angle_change;
		}
	else
		{
		// Turn away from enemy

		if ( 180.0f - abs_angle_diff < angle_change )
			angle_change = 180.0f - abs_angle_diff;

		if ( angle_diff < 0.0f )
			real_angle_change = -angle_change;
		else
			real_angle_change = angle_change;
		}

	if ( ( real_angle_change < 1.0f ) && ( real_angle_change > -1.0f ) )
		real_angle_change = 0.0f;

	if ( real_angle_change > 0.0f )
		{
		if ( real_angle_change > ( last_angle_change + MAX_CIRCLE_ACCELERATION ) )
			real_angle_change = last_angle_change + MAX_CIRCLE_ACCELERATION;
		}
	else if ( real_angle_change < 0.0f )
		{
		if ( real_angle_change < ( last_angle_change - MAX_CIRCLE_ACCELERATION ) )
			real_angle_change = last_angle_change - MAX_CIRCLE_ACCELERATION;
		}

	last_angle_change = real_angle_change;

	dir = self.origin - center_actor->origin;
	length = dir.length();

	actor_angles = dir.toAngles();
	actor_angles[YAW] += real_angle_change;

	// Find new position

	actor_angles.AngleVectors( &dir, NULL, NULL );

	dir *= length;
	dir.z = 0.0f;

	self.setOrigin( center_actor->origin + dir );

	// Set the actors angle to look at the center

	dir[0] = -dir[0];
	dir[1] = -dir[1];
	dir[2] = -dir[2];

	self.angles[YAW] = dir.toYaw();
	self.setAngles( self.angles );

	return BEHAVIOR_EVALUATING;
	}

void CircleEnemy::End
	(
	Actor &
	)

	{
	}
/****************************************************************************

  CircleCurrentEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, CircleCurrentEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&CircleCurrentEnemy::SetArgs },
		{ NULL, NULL }
	};

void CircleCurrentEnemy::SetArgs
	(
	Event *ev
	)

	{
	anim = "walk";
	
	radius = 0;
	maintainDistance = true;
	clockwise = true;
	
	if (ev->NumArgs() > 0 )
		anim = ev->GetString( 1 );

	if (ev->NumArgs() > 1 )
		radius = ev->GetFloat( 2 );

	if (ev->NumArgs() > 2 )
		maintainDistance = ev->GetBoolean( 3 );

	if (ev->NumArgs() > 3 )
		clockwise = ev->GetBoolean( 4 );
	
	}

void CircleCurrentEnemy::Begin
	(
	Actor &self
	)

	{
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return;
	
	self.SetAnim(anim);
	dirToEnemy = self.origin - currentEnemy->origin;

	if(!radius)
		radius = dirToEnemy.length();
	
	stuck = 0;
	stuckCheck = 0;

	if (clockwise)
		{
		turnAngle = 90;
		}
	else
		turnAngle = -90;

	oldAngle = 0;
	angleAdjusted = false;
	}


BehaviorReturnCode_t	CircleCurrentEnemy::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector dirAngles;
	Vector Angles;

	float dirYaw;
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	dirToEnemy = currentEnemy->origin - self.origin;
	dirYaw = dirToEnemy.toYaw();
	
	if(!stuck)
		{
		float len = dirToEnemy.length();
		if(len > radius && maintainDistance && !angleAdjusted)
			{
			oldAngle = turnAngle;
			turnAngle*=.5f;
			angleAdjusted = true;
			}

		if(len <= radius && angleAdjusted)
			{
			turnAngle = oldAngle;
			angleAdjusted = false;
			}
		}

	dirYaw+=turnAngle;

	self.angles[YAW] = dirYaw;

	if ( self.movementSubsystem->getLastMove() != STEPMOVE_OK ) 
      {
		if(stuck >= 2)
			{
			if(stuck < 3 )
				{
				turnAngle = -turnAngle;
				}
			else
				{
				self.angles[YAW] = dirYaw + angleStep;
				angleStep += 45.0f;
				}
			}

		stuck++;
      }
	
	stuckCheck++;
	if(stuckCheck > stuck)
		{
		stuckCheck = 0;
		stuck = 0;
		angleStep = 45.0f;
		}

	self.setAngles(self.angles);
		
	return BEHAVIOR_EVALUATING;
	}

void CircleCurrentEnemy::End
	(
	Actor &
	)

	{
	
	}

/****************************************************************************

  ChaoticDodge Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, ChaoticDodge, NULL )
	{
		{ &EV_Behavior_Args,			&ChaoticDodge::SetArgs },
		{ NULL, NULL }
	};

void ChaoticDodge::SetArgs
	(
	Event *ev
	)

	{
	anim = "walk";
	stuck = 0;
	time = .5;
	changeTime = 0;
	turnspeed = 45;
	adjusting = false;
	turnAngle = 0;
	
	if (ev->NumArgs() > 0 )
		anim = ev->GetString( 1 );

	if (ev->NumArgs() > 1 )
		time = ev->GetFloat( 2 );

	if (ev->NumArgs() > 2 )
		turnspeed = ev->GetFloat( 3 );
	
	}

void ChaoticDodge::Begin
	(
	Actor &self
	)

	{
	self.SetAnim(anim);

	}


BehaviorReturnCode_t	ChaoticDodge::Evaluate
	(
	Actor &self
	)

	{

	float dirYaw = GetNewYaw();

	float CurrentYaw = self.origin.toYaw();
		
	if ( self.movementSubsystem->getLastMove() != STEPMOVE_OK ) 
      {
		stuck++;
		turnAngle += 30.0f;
		if (stuck > 3 )
			{
			self.angles[YAW]+=turnAngle;
			self.setAngles(self.angles);
			time=1;
			changeTime = level.time + time;
			stuck = 0;
			turnAngle = 0.0f;
			}
		}

	if ( level.time <= changeTime )
		return BEHAVIOR_EVALUATING;
	
	dirYaw+=CurrentYaw;

	self.angles[YAW] = dirYaw;
	self.setAngles(self.angles);	
	time = G_Random ( .15f ) +.15f;
	changeTime = level.time + time;
		
	return BEHAVIOR_EVALUATING;
	}

float ChaoticDodge::GetNewYaw
   (
	void
	)

	{
	float dirYaw = G_Random( 20.0f ) + turnspeed;
	float randomSide = G_Random();
		if (randomSide < .5f )
			randomSide = 1.0f;
		else
			randomSide = -1.0f;
	
	dirYaw*=randomSide;
	
	return dirYaw;
	}

void ChaoticDodge::End
	(
	Actor &
	)

	{
	
	}
/****************************************************************************

  ShockWater Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, ShockWater, NULL )
	{
		{ NULL, NULL }
	};

ShockWater::ShockWater()
	{
	left_beam   = NULL;
	right_beam  = NULL;
	center_beam = NULL;
	already_started = false;
	}

void ShockWater::Begin
	(
	Actor &
	)

	{
	}

BehaviorReturnCode_t	ShockWater::Evaluate
	(
	Actor &self
	)

	{
	Vector left_tag_orig;
	Vector right_tag_orig;
	Vector end_pos;
	Vector center_point;
	Actor *center_actor;
	trace_t  trace;
	Entity *hit_entity;
	Vector diff_vect;
	float diff;
	Vector dir;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	center_actor = self.FindPartActor( "body" );

	if ( !center_actor )
		return BEHAVIOR_SUCCESS;

	if ( ( self.newanimnum == -1 ) && !already_started )
		{
		// Get tag positions
		self.GetTag( "tag_left",  &left_tag_orig );
		self.GetTag( "tag_right", &right_tag_orig );

		// Get end position
		end_pos = left_tag_orig + right_tag_orig;
		end_pos *= .5f;
		end_pos[2] -= 120.0f;

		dir = end_pos - self.origin;
		dir.z = 0.0f;
		dir *= 0.5f;

		end_pos += dir;

		// Add the left and right beams
      left_beam   = CreateBeam( NULL, "emap1", left_tag_orig, end_pos, 10, 1.5f, 0.2f );
		right_beam  = CreateBeam( NULL, "emap1", right_tag_orig, end_pos, 10, 1.5f, 0.2f );

		center_point = center_actor->origin;

		trace = G_Trace( center_point, vec_zero, vec_zero, end_pos, &self, MASK_SHOT, false, "ShockAttack" );

		if ( ( trace.fraction < 1.0f ) && ( trace.entityNum != center_actor->entnum ) )
			{
			hit_entity = G_GetEntity( trace.entityNum );
			if ( hit_entity )
            {
				center_point = hit_entity->origin;
            }
			}
		else
			{
			// Shock head
			center_actor->AddStateFlag( STATE_FLAG_IN_PAIN );

			center_actor->SpawnEffect( "fx_elecstrike.tik", center_actor->origin, vec_zero, 2.0f );
			center_actor->Sound( "sound/weapons/sword/electric/hitmix2.wav", 0, 1.0f, 500.0f );
			}

      // create the center beam
      center_beam = CreateBeam( NULL, "emap1", end_pos, center_point, 20, 3.0f, 0.2f );

		// Damage player if in water
	
		diff_vect = currentEnemy->origin - center_actor->origin;
		diff_vect[2] = 0.0f;

		diff = diff_vect.length();
		//if ( diff < 240 && self.currentEnemy->groundentity )
		if ( ( diff < 350.0f ) && currentEnemy->groundentity )
			{
			currentEnemy->Damage( &self, &self, 10, Vector (0.0f, 0.0f, 0.0f), Vector (0.0f, 0.0f, 0.0f), Vector (0.0f, 0.0f, 0.0f), 0, 0, MOD_ELECTRICWATER );
			}

		already_started = true;
		}

	return BEHAVIOR_EVALUATING;
	}

void ShockWater::End
	(
	Actor &
	)

	{
   delete left_beam;
	delete right_beam;
	delete center_beam;
	}

/****************************************************************************

  Shock Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Shock, NULL )
	{
		{ &EV_Behavior_Args,			&Shock::SetArgs },
		{ NULL, NULL }
	};

Shock::Shock()
	{
	beam = NULL;
	damage = 10;
	already_started = false;
	random_angle = 0;
	beamShader = "emap1";
	z_offset = 100;
	}

void Shock::SetArgs
	(
	Event *ev
	)

	{
	tag_name = ev->GetString( 1 );
	beamShader = ev->GetString( 2 );

	if ( ev->NumArgs() > 3 )
      {
		damage = ev->GetInteger( 3 );
      }

	if ( ev->NumArgs() > 3 )
      {
		random_angle = ev->GetFloat( 4 );
      }
	
	if ( ev->NumArgs() > 4 )
		z_offset = ev->GetFloat( 5 );
	}

void Shock::Begin
	(
	Actor &
	)

	{
	}

BehaviorReturnCode_t	Shock::Evaluate
	(
	Actor &self
	)

	{
	Vector tag_orig;
	Vector angles;
	Vector end_pos;
	trace_t  trace;
	Vector dir;
	float yaw_diff;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( ( self.newanimnum == -1 ) && !already_started )
		{
		// Get tag position
		if ( tag_name.length() == 0 )
         {
			return BEHAVIOR_SUCCESS;
         }

		self.GetTag( tag_name.c_str(), &tag_orig );

		// See if the enemy is in front of us

		dir = currentEnemy->centroid - self.centroid;
		angles = dir.toAngles();

		yaw_diff = AngleNormalize180( angles[YAW] - self.angles[YAW] );

		//if ( ( yaw_diff < 60.0f ) && ( yaw_diff > -60.0f ) )
		//	{
			// The enemy is in front of us

			angles[YAW] += G_CRandom( random_angle );

			angles.AngleVectors( &end_pos, NULL, NULL );
			end_pos *= 500.0f;
			end_pos += tag_orig;
			end_pos.z -= z_offset;
		//	}
      /*
		else
			{
			// Get end position
			angles = self.angles;

			angles[YAW] += G_Random( random_angle ) - ( random_angle / 2.0f );
			angles[PITCH] = 0.0f;
			angles[ROLL] = 0.0f;

			angles.AngleVectors( &end_pos, NULL, NULL );
			end_pos *= 500.0f;
			end_pos += tag_orig;
			end_pos.z -= z_offset;
			}
      */
		trace = G_Trace( tag_orig, Vector (-15.0f, -15.0f, -15.0f), Vector (15.0f, 15.0f, 15.0f), end_pos, &self, MASK_SHOT, false, "ShockAttack" );

		if ( ( trace.fraction < 1.0f ) && ( trace.entityNum == currentEnemy->entnum ) )
			{
			end_pos = currentEnemy->centroid;
			dir = end_pos - tag_orig;
			dir.normalize();
			currentEnemy->Damage( &self, &self, damage, vec_zero, dir, vec_zero, 0, 0, MOD_ELECTRIC );
			}

		// Add the beam
		beam = CreateBeam( NULL, beamShader.c_str(), tag_orig, end_pos, 2, 1.5f, 0.25f );

      already_started = true;
		}
	else if ( already_started )
		{
		self.GetTag( tag_name.c_str(), &tag_orig );
      if ( beam )
         beam->setOrigin( tag_orig );         
		}

	return BEHAVIOR_EVALUATING;
	}

void Shock::End
	(
	Actor &
	)

	{
	if ( beam )
		{
		beam->ProcessEvent( EV_Remove );
		beam = NULL;
		}
	}

/****************************************************************************

  MultiShock Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, MultiShock, NULL )
	{
		{ &EV_Behavior_Args,			&MultiShock::SetArgs },
		{ NULL, NULL }
	};

MultiShock::MultiShock()
	{
	beam1 = NULL;
	beam2 = NULL;
	damage = 10;
	already_started = false;
	random_angle = 0;
	beamShader = "emap1";
	z_offset = 100;
	}

void MultiShock::SetArgs
	(
	Event *ev
	)

	{
	tag_name1 = ev->GetString( 1 );
	tag_name2 = ev->GetString( 2 );
	beamShader = ev->GetString( 3 );
	damage = ev->GetFloat( 4 );
	random_angle = ev->GetFloat( 5 );
	z_offset = ev->GetFloat( 6 );
	}
	

void MultiShock::Begin
	(
	Actor &
	)

	{
	}

BehaviorReturnCode_t	MultiShock::Evaluate
	(
	Actor &self
	)

	{
	Vector tag1_orig;
	Vector tag2_orig;

	Vector angles;
	Vector end_pos1;
	Vector end_pos2;

	trace_t  trace;
	Vector dir;
	float yaw_diff;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;
	
	//if ( self.newanimnum == -1 && !already_started )
	if ( !already_started )
		{
		// Get tag position
		if ( tag_name1.length() == 0 )
			return BEHAVIOR_SUCCESS;
         
		if ( tag_name2.length() == 0 )
			return BEHAVIOR_SUCCESS;

		self.GetTag( tag_name1.c_str(), &tag1_orig );
		self.GetTag( tag_name2.c_str(), &tag2_orig );

		// See if the enemy is in front of us

		dir = currentEnemy->origin - self.origin;
		angles = dir.toAngles();

		yaw_diff = AngleNormalize180( angles[YAW] - self.angles[YAW] );

		if ( ( yaw_diff < 60.0f ) && ( yaw_diff > -60.0f ) )
			{
			// The enemy is in front of us

			angles[YAW] += G_CRandom( random_angle );

			angles.AngleVectors( &end_pos1, NULL, NULL );
			end_pos1 *= 500.0f;
			end_pos1 += tag1_orig;
			end_pos1.z -= z_offset;

			angles.AngleVectors( &end_pos2, NULL, NULL );
			end_pos2 *= 500.0f;
			end_pos2 += tag1_orig;
			end_pos2.z -= z_offset;
			}
		else
			{
			// Get end position
			angles = self.angles;

			angles[YAW] += G_Random( random_angle ) - ( random_angle / 2.0f );
			angles[PITCH] = 0.0f;
			angles[ROLL] = 0.0f;

			angles.AngleVectors( &end_pos1, NULL, NULL );
			end_pos1 *= 500.0f;
			end_pos1 += tag1_orig;
			end_pos1.z -= z_offset;
			
			angles.AngleVectors( &end_pos2, NULL, NULL );
			end_pos2 *= 500.0f;
			end_pos2 += tag1_orig;
			end_pos2.z -= z_offset;
			}

		trace = G_Trace( tag1_orig, Vector (-15.0f, -15.0f, -15.0f), Vector (15.0f, 15.0f, 15.0f), end_pos1, &self, MASK_SHOT, false, "ShockAttack" );

		if ( ( trace.fraction < 1.0f ) && ( trace.entityNum == currentEnemy->entnum ) )
			{
			end_pos1 = currentEnemy->centroid;
			dir = end_pos1 - tag1_orig;
			dir.normalize();
			currentEnemy->Damage( &self, &self, damage, vec_zero, dir, vec_zero, 0, 0, MOD_ELECTRIC );
			}

		// Add the beam
		beam1 = CreateBeam( NULL, beamShader.c_str(), tag1_orig, end_pos1, 20, 1.5f, 0.2f );

		trace = G_Trace( tag2_orig, Vector (-15.0f, -15.0f, -15.0f), Vector (15.0f, 15.0f, 15.0f), end_pos2, &self, MASK_SHOT, false, "ShockAttack" );

		if ( ( trace.fraction < 1.0f ) && ( trace.entityNum == currentEnemy->entnum ) )
			{
			end_pos2 = currentEnemy->centroid;
			dir = end_pos2 - tag2_orig;
			dir.normalize();
			currentEnemy->Damage( &self, &self, damage, vec_zero, dir, vec_zero, 0, 0, MOD_ELECTRIC );
			}

		// Add the beam
		beam2 = CreateBeam( NULL, beamShader.c_str(), tag2_orig, end_pos2, 20, 1.5f, 0.2f );

      already_started = true;
		}
	else
		{
		self.GetTag( tag_name1.c_str(), &tag1_orig );
		self.GetTag( tag_name2.c_str(), &tag2_orig );

		if ( beam1 )
			beam1->setOrigin( tag1_orig );

		if ( beam2 )
			beam2->setOrigin( tag2_orig );
		}

	return BEHAVIOR_EVALUATING;
	}

void MultiShock::End
	(
	Actor &
	)

	{
	if ( beam1 )
		{
		beam1->ProcessEvent( EV_Remove );
		beam1 = NULL;
		}
	
	if ( beam2 )
		{
		beam2->ProcessEvent( EV_Remove );
		beam2 = NULL;
		}
	}

/****************************************************************************

  ShockDown Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, ShockDown, NULL )
	{
		{ &EV_Behavior_Args,			&ShockDown::SetArgs },
		{ NULL, NULL }
	};

ShockDown::ShockDown()
	{
	beam = NULL;
	damage = 10;
	already_started = false;	
	beamShader = "emap1";	
	}

void ShockDown::SetArgs
	(
	Event *ev
	)

	{
	tag_name = ev->GetString( 1 );
	beamShader = ev->GetString( 2 );

	if ( ev->NumArgs() > 3 )
      {
		damage = ev->GetInteger( 3 );
      }

	}

void ShockDown::Begin
	(
	Actor &
	)

	{
	}

BehaviorReturnCode_t	ShockDown::Evaluate
	(
	Actor &self
	)

	{
	Vector tag_orig;
	Vector angles;
	Vector end_pos;
	trace_t  trace;
	Vector dir;
	

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( ( self.newanimnum == -1 ) && !already_started )
		{
		// Get tag position
		if ( tag_name.length() == 0 )
         {
			return BEHAVIOR_SUCCESS;
         }

		self.GetTag( tag_name.c_str(), &tag_orig );

		end_pos = self.origin;
		end_pos.z -= 10000.0f;
		
		trace = G_Trace( tag_orig, Vector (-15.0f, -15.0f, -15.0f), Vector (15.0f, 15.0f, 15.0f), end_pos, &self, MASK_SHOT, false, "ShockAttack" );
		end_pos = trace.endpos;

		dir = end_pos - tag_orig;
		angles = dir.toAngles();

		if ( ( trace.fraction < 1.0f ) && ( trace.entityNum == currentEnemy->entnum ) )
			{
			end_pos = currentEnemy->centroid;
			dir = end_pos - tag_orig;
			dir.normalize();
			currentEnemy->Damage( &self, &self, damage, vec_zero, dir, vec_zero, 0, 0, MOD_ELECTRIC );
			}

		// Add the beam
		beam = CreateBeam( NULL, beamShader.c_str(), tag_orig, end_pos, 20, 1.5f, 0.2f );

      already_started = true;
		}
	else if ( already_started )
		{
		self.GetTag( tag_name.c_str(), &tag_orig );

		if ( beam )
			beam->setOrigin( tag_orig );
		}

	return BEHAVIOR_EVALUATING;
	}

void ShockDown::End
	(
	Actor &
	)

	{
	if ( beam )
		{
		beam->ProcessEvent( EV_Remove );
		beam = NULL;
		}
	}
/****************************************************************************

  CircleAttack Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, CircleAttack, NULL )
	{
		{ &EV_Behavior_Args,			&CircleAttack::SetArgs },
		{ NULL, NULL }
	};

CircleAttack::CircleAttack()
	{
	next_time = 0.0f;
	current_direction = 1;
	number_of_attacks = 1;
	}

void CircleAttack::SetArgs
	(
	Event *ev
	)

	{
	command   = ev->GetString( 1 );
	direction = ev->GetString( 2 );
	}

Actor *CircleAttack::FindClosestPart
	(
	const Actor &self,
	float angle
	)
	{
	float closest_diff = 360.0f;
	int i;
	part_t *part;
	Entity *partent;
	Actor *partact;
	Vector dir;
	Vector angles;
	float angle_diff;
	Actor *closest_part = NULL;

	for( i = 1 ; i <= self.parts.NumObjects(); i++ )
		{
		part = &self.parts.ObjectAt( i );

		partent = part->ent;
		partact = ( Actor * )partent;

		if ( partact && ( partact->part_name == "smallarm" ) )
			{
			dir = partact->origin - self.origin;
			angles = dir.toAngles();

			angle_diff = AngleDelta( angles[ YAW ], angle );

			if ( angle_diff < 0.0f )
            {
				angle_diff = -angle_diff;
            }

			if ( angle_diff < closest_diff )
				{
				closest_part = partact;
				closest_diff = angle_diff;
				}
			}
		}

	return closest_part;
	}

void CircleAttack::Begin
	(
	Actor &self
	)

	{
	float random_direction;
	Actor *closest_part;

	// Pick which arm to start with
	random_direction = G_Random( 360.0f );

	closest_part = FindClosestPart( self, random_direction );
	if ( closest_part != NULL )
		{
		first_part   = closest_part;
		current_part = closest_part;

		closest_part->command = command;
		next_time = level.time + 0.2f;
		}

	current_direction = 1;

	if ( direction == "counterclockwise" )
      {
		current_direction = 0;
      }
	}

BehaviorReturnCode_t	CircleAttack::Evaluate
	(
	Actor &self
	)

	{
	Entity *current_part_entity;
	Actor *current_part_actor;
	Vector dir;
	Vector angles;
	Actor *closest_part;

	if ( level.time >= next_time )
		{
		current_part_entity = current_part;
		current_part_actor = ( Actor * )current_part_entity;

		// Find the next part

		dir = current_part_actor->origin - self.origin;
		angles = dir.toAngles();

		if ( direction == "changing" )
			{
			if ( number_of_attacks >= 20 )
            {
				return BEHAVIOR_SUCCESS;
            }

			if ( G_Random( 1.0f ) < .3f )
            {
				current_direction = !current_direction;
            }
			}

		if ( current_direction == 1 )
         {
			angles[YAW] -= 360.0f / 8.0f;
         }
		else
         {
			angles[YAW] += 360.0f / 8.0f;
         }

		closest_part = FindClosestPart( self, angles[YAW] );

		if ( ( closest_part == first_part ) && ( direction != "changing" ) )
         {
			return BEHAVIOR_SUCCESS;
         }

		current_part = closest_part;

		closest_part->command = command;
		next_time = level.time + 0.2f;

		number_of_attacks++;
		}

	return BEHAVIOR_EVALUATING;
	}

void CircleAttack::End
	(
	Actor &
	)

	{
	}

/****************************************************************************

  DragEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, DragEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&DragEnemy::SetArgs },
		{ NULL, NULL }
	};

void DragEnemy::SetArgs
	(
	Event *ev
	)

	{
	tag_name = ev->GetString( 1 );
	damage   = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		drop = ev->GetBoolean( 3 );
	else
		drop = false;
	}

void DragEnemy::Begin
	(
	Actor &self
	)

	{
	Actor *body;
	Vector dir;
	Vector angles;

	ent_to_drag = self.enemyManager->GetCurrentEnemy();

	if ( !ent_to_drag )
		return;

	attached = false;

	if ( damage > 0.0f && !drop )
      {
		ent_to_drag->Damage( &self, &self, damage, vec_zero, vec_zero, vec_zero, 0, 0, MOD_EAT );
      }

	body = self.FindPartActor( "body" );

	if ( body )
		{
		dir = body->origin - self.origin;
		angles = dir.toAngles();

		target_yaw = angles[ YAW ];
		last_turn_time = level.time;
		}
	else
		{
		target_yaw = self.angles[ YAW ];
		}
	}

BehaviorReturnCode_t	DragEnemy::Evaluate
	(
	Actor &self
	)

	{
	Vector orig;
	str anim_name;

	if ( !ent_to_drag )
		return BEHAVIOR_SUCCESS;

	if ( drop && ( damage > 0.0f ) )
		{
		if ( self.GetTag( tag_name.c_str(), &orig ) )
			{
			if ( ent_to_drag->isClient() )
            {
				orig[ 2 ] -= 85.0f;
            }
			else
				{
				offset[ 2 ] = ( ent_to_drag->absmin[ 2 ] - ent_to_drag->absmax[ 2 ] ) * 0.5f;

				if ( offset[ 2 ] < -40.0f )
               {
					offset[ 2 ] -= 25.0f;
               }
				orig += offset;
				}

			ent_to_drag->setOrigin( orig );
			}

		ent_to_drag->Damage( &self, &self, damage, vec_zero, vec_zero, vec_zero, 0, 0, MOD_EAT );
		}

	if ( ent_to_drag->deadflag )
		return BEHAVIOR_SUCCESS;

	anim_name = self.animate->AnimName();

	if ( ( anim_name == "raise" ) && ( self.newanim == "" ) )
		{
		if ( !attached )
			{
			Event *ev;

			if ( damage > 0.0f )
				{
				if ( ent_to_drag->isClient() )
               {
					offset[ 2 ] -= 85.0f;
               }
				else
					{
					offset[ 2 ] = ( ent_to_drag->absmin[ 2 ] - ent_to_drag->absmax[ 2 ] ) * 0.5f;

					if ( offset[ 2 ] < -40.0f )
                  {
						offset[ 2 ] -= 25.0f;
                  }
					}

				ev = new Event( EV_Attach );
				ev->AddEntity( &self );
				ev->AddString( tag_name.c_str() );
				ev->AddInteger( false );
				ev->AddVector( offset );
				ent_to_drag->ProcessEvent( ev );

				ent_to_drag->flags |= FlagPartialImmobile;
				}

			attached = true;
			}

		if ( target_yaw != self.angles[ YAW ] )
			{
			float yaw_diff = target_yaw - self.angles[ YAW ];

			if ( yaw_diff > 180.0f )
				{
				target_yaw -= 360.0f;
				yaw_diff -= 360.0f;
				}

			if ( yaw_diff < -180.0f )
				{
				target_yaw += 360.0f;
				yaw_diff += 360.0f;
				}

			if ( yaw_diff < 0.0f )
				{
				self.angles[ YAW ] -= 90.0f * (level.time - last_turn_time);

				if ( self.angles[ YAW ] < 0.0f )
               {
					self.angles[ YAW ] += 360.0f;
               }

				if ( self.angles[ YAW ] < target_yaw )
               {
					self.angles[ YAW ] = target_yaw;
               }

				self.setAngles( self.angles );
				}
			else if ( yaw_diff > 0.0f )
				{
				self.angles[ YAW ] += 90.0f * (level.time - last_turn_time);

				if ( self.angles[ YAW ] > 360.0f )
               {
					self.angles[ YAW ] -= 360.0f;
               }

				if ( self.angles[ YAW ] > target_yaw )
               {
					self.angles[ YAW ] = target_yaw;
               }

				self.setAngles( self.angles );
				}

			last_turn_time = level.time;
			}
		}

	return BEHAVIOR_EVALUATING;
	}

void DragEnemy::End
	(
	Actor &self
	)

	{
	Vector orig;
	Event *ev;
	trace_t trace;


	if ( !ent_to_drag )
		return;

	if ( drop || ( strcmp( self.currentState->getName(), "SUICIDE" ) == 0 ) )
		{
		ent_to_drag->flags &= ~FlagPartialImmobile;

		ev = new Event( EV_Detach );
		ent_to_drag->ProcessEvent( ev );

		ent_to_drag->setSolidType( SOLID_BBOX );

		if ( self.GetTag( tag_name.c_str(), &orig ) )
			{
			trace = G_Trace( orig - Vector( "0 0 100" ), ent_to_drag->mins, ent_to_drag->maxs, orig, ent_to_drag, ent_to_drag->edict->clipmask, false, "DragEnemy" );

			if ( trace.allsolid )
				gi.WDPrintf( "Dropping entity into a solid!\n" );

			ent_to_drag->setOrigin( trace.endpos );

			/* if ( ent_to_drag->isClient() )
            {
				offset[2] = -85;
            }
			else
				{
				offset[2] = ( ent_to_drag->absmin[2] - ent_to_drag->absmax[2] ) * 0.5;

				if ( offset[2] < -40 )
               {
					offset[2] -= 25;
               }
				}

			ent_to_drag->setOrigin( orig + offset ); */
			}
		}

	ent_to_drag->velocity = Vector(0, 0, -20);
	}


/****************************************************************************

  PickupEnemy Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, PickupEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&PickupEnemy::SetArgs },
		{ NULL, NULL }
	};

void PickupEnemy::SetArgs
	(
	Event *ev
	)

	{
	tag_name = ev->GetString( 1 );
	damage   = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		drop = ev->GetBoolean( 3 );
	else
		drop = false;
	}

void PickupEnemy::Begin
	(
	Actor &self
	)

	{
	Actor *body;
	Vector dir;
	Vector angles;

	ent_to_drag = self.enemyManager->GetCurrentEnemy();

	if ( !ent_to_drag )
		return;

	attached = false;

	if ( ( damage > 0.0f ) && !drop )
      {
		ent_to_drag->Damage( &self, &self, damage, vec_zero, vec_zero, vec_zero, 0, 0, MOD_EAT );
      }

	body = self.FindPartActor( "body" );

	if ( body )
		{
		dir = body->origin - self.origin;
		angles = dir.toAngles();

		target_yaw = angles[ YAW ];
		last_turn_time = level.time;
		}
	else
		{
		target_yaw = self.angles[ YAW ];
		}
	}

BehaviorReturnCode_t	PickupEnemy::Evaluate
	(
	Actor &self
	)

	{
	Vector orig;
	str anim_name;

	if ( !ent_to_drag )
		return BEHAVIOR_SUCCESS;

	if ( drop && ( damage > 0.0f ) )
		{
		if ( self.GetTag( tag_name.c_str(), &orig ) )
			{
			if ( ent_to_drag->isClient() )
            {
				orig[ 2 ] -= 85.0f;
            }
			else
				{
				offset[ 2 ] = ( ent_to_drag->absmin[ 2 ] - ent_to_drag->absmax[ 2 ] ) * 0.5f;

				if ( offset[ 2 ] < -40.0f )
               {
					offset[ 2 ] -= 25.0f;
               }
				orig += offset;
				}

			ent_to_drag->setOrigin( orig );
			}

		ent_to_drag->Damage( &self, &self, damage, vec_zero, vec_zero, vec_zero, 0, 0, MOD_EAT );
		}

	if ( ent_to_drag->deadflag )
		return BEHAVIOR_SUCCESS;

	anim_name = self.animate->AnimName();

	if ( !attached )
		{
		Event *ev;

		if ( damage > 0.0f )
			{
			if ( ent_to_drag->isClient() )
            {
				offset[ 2 ] -= 85.0f;
            }
			else
				{
				offset[ 2 ] = ( ent_to_drag->absmin[ 2 ] - ent_to_drag->absmax[ 2 ] ) * 0.5f;

				if ( offset[ 2 ] < -40.0f )
               {
			      offset[ 2 ] -= 25.0f;
               }
				}

			ev = new Event( EV_Attach );
			ev->AddEntity( &self );
			ev->AddString( tag_name.c_str() );
			ev->AddInteger( false );
			ev->AddVector( offset );
			ent_to_drag->ProcessEvent( ev );

			ent_to_drag->flags |= FlagPartialImmobile;
			}

		attached = true;
		}

	if ( target_yaw != self.angles[ YAW ] )
		{
		float yaw_diff = target_yaw - self.angles[ YAW ];

		if ( yaw_diff > 180.0f )
			{
			target_yaw -= 360.0f;
			yaw_diff -= 360.0f;
			}

		if ( yaw_diff < -180.0f )
			{
			target_yaw += 360.0f;
			yaw_diff += 360.0f;
			}

		if ( yaw_diff < 0.0f )
			{
			self.angles[ YAW ] -= 90.0f * (level.time - last_turn_time);

			if ( self.angles[ YAW ] < 0.0f )
            {
				self.angles[ YAW ] += 360.0f;
            }

			if ( self.angles[ YAW ] < target_yaw )
            {
				self.angles[ YAW ] = target_yaw;
            }

			self.setAngles( self.angles );
			}
		else if ( yaw_diff > 0.0f )
			{
			self.angles[ YAW ] += 90.0f * (level.time - last_turn_time);

			if ( self.angles[ YAW ] > 360.0f )
            {
				self.angles[ YAW ] -= 360.0f;
            }

			if ( self.angles[ YAW ] > target_yaw )
            {
				self.angles[ YAW ] = target_yaw;
            }

			self.setAngles( self.angles );
			}

		last_turn_time = level.time;
		}

	return BEHAVIOR_EVALUATING;
	}

void PickupEnemy::End
	(
	Actor &self
	)

	{
	Vector orig;
	Event *ev;
	trace_t trace;


	if ( !ent_to_drag )
		return;

	if ( drop || ( strcmp( self.currentState->getName(), "SUICIDE" ) == 0 ) )
		{
		ent_to_drag->flags &= ~FlagPartialImmobile;

		ev = new Event( EV_Detach );
		ent_to_drag->ProcessEvent( ev );

		ent_to_drag->setSolidType( SOLID_BBOX );

		if ( self.GetTag( tag_name.c_str(), &orig ) )
			{
			trace = G_Trace( orig - Vector( "0 0 100" ), ent_to_drag->mins, ent_to_drag->maxs, orig, ent_to_drag, ent_to_drag->edict->clipmask, false, "DragEnemy" );

			if ( trace.allsolid )
				gi.WDPrintf( "Dropping entity into a solid!\n" );

			ent_to_drag->setOrigin( trace.endpos );

			/* if ( ent_to_drag->isClient() )
            {
				offset[2] = -85;
            }
			else
				{
				offset[2] = ( ent_to_drag->absmin[2] - ent_to_drag->absmax[2] ) * 0.5;

				if ( offset[2] < -40 )
               {
					offset[2] -= 25;
               }
				}

			ent_to_drag->setOrigin( orig + offset ); */
			}
		}

	ent_to_drag->velocity = Vector(0, 0, -20);
	}

/****************************************************************************

  Teleport Class Definition

****************************************************************************/

#define TELEPORT_BEHIND   0
#define TELEPORT_TOLEFT   1
#define TELEPORT_TORIGHT  2
#define TELEPORT_INFRONT  3

#define TELEPORT_NUMBER_OF_POSITIONS  4

CLASS_DECLARATION( Behavior, Teleport, NULL )
	{
		{ NULL, NULL }
	};


void Teleport::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void Teleport::Begin
	(
	Actor &
	)

	{

	}

qboolean	Teleport::TestPosition
	(
	Actor &self,
	int test_pos,
	Vector &good_position,
	qboolean	use_enemy_dir
	)
	{
	Vector test_position;
	Vector enemy_angles;
	Vector enemy_forward;
	Vector enemy_left;
	trace_t trace;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return false;

	// Get the position to test

	if ( use_enemy_dir )
		{
		// Get the enemy direction info

		enemy_angles = currentEnemy->angles;
		enemy_angles.AngleVectors( &enemy_forward, &enemy_left );

		test_position = currentEnemy->origin;

		if ( test_pos == TELEPORT_BEHIND )
			test_position -= enemy_forward * 135.0f;
		else if ( test_pos == TELEPORT_INFRONT )
			test_position += enemy_forward * 135.0f;
		else if ( test_pos == TELEPORT_TOLEFT )
			test_position += enemy_left * 135.0f;
		else
			test_position -= enemy_left * 135.0f;
		}
	else
		{
		test_position = currentEnemy->origin;

		if ( test_pos == TELEPORT_BEHIND )
			test_position += Vector(-110, 0, 0);
		else if ( test_pos == TELEPORT_INFRONT )
			test_position += Vector(110, 0, 0);
		else if ( test_pos == TELEPORT_TOLEFT )
			test_position += Vector(0, -110, 0);
		else
			test_position += Vector(0, 110, 0);
		}

	test_position += Vector(0, 0, 64);

	// Test to see if we can fit at the new position

	trace = G_Trace( test_position, self.mins, self.maxs, test_position - Vector( "0 0 1000" ), &self, self.edict->clipmask, false, "Teleport::TestPosition" );

	if ( trace.allsolid || trace.startsolid )
	//if ( trace.allsolid )
		return false;

	// Make sure we can see the enemy from this position

	if ( !self.IsEntityAlive( currentEnemy ) || !self.sensoryPerception->CanSeeEntity( Vector( trace.endpos ), currentEnemy , true , true ) )			
			return false;
	
	
	// This is a good position

	good_position = trace.endpos;
	return true;
	}

BehaviorReturnCode_t	Teleport::Evaluate
	(
	Actor &self
	)

	{
	int current_position;
	float random_number;
	Vector teleport_position;
	qboolean teleport_position_found;
	Vector new_position;
	int i;
	Vector dir;
	Vector angles;


	// Make sure we stll have an enemy to teleport near

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	// Default the teleport position to where we are now

	teleport_position = self.origin;
	teleport_position_found = false;

	// Determine which position to try first

	random_number = G_Random();

	if ( random_number < .5f )
		current_position = TELEPORT_BEHIND;
	else if ( random_number < .7f )
		current_position = TELEPORT_TOLEFT;
	else if ( random_number < .9f )
		current_position = TELEPORT_TORIGHT;
	else
		current_position = TELEPORT_INFRONT;

	// Try positions

	for( i = 0 ; i < TELEPORT_NUMBER_OF_POSITIONS ; i++ )
		{
		// Test this position

		if ( TestPosition( self, current_position, new_position, true ) )
			{
			teleport_position = new_position;
			teleport_position_found = true;
			break;
			}

		// Try the next position

		current_position++;

		if ( current_position >= TELEPORT_NUMBER_OF_POSITIONS )
			current_position = 0;
		}

	if ( !teleport_position_found )
		{
		// Try again with not using the enemies angles

		if ( current_position >= TELEPORT_NUMBER_OF_POSITIONS )
			current_position = 0;

		for( i = 0 ; i < TELEPORT_NUMBER_OF_POSITIONS ; i++ )
			{
			// Test this position

			if ( TestPosition( self, current_position, new_position, false ) )
				{
				teleport_position = new_position;
				teleport_position_found = true;
				break;
				}

			// Try the next position

			current_position++;

			if ( current_position >= TELEPORT_NUMBER_OF_POSITIONS )
				current_position = 0;
			}
		}

	// Do teleport stuff

	if ( teleport_position_found )
		{
		self.setOrigin( teleport_position );
		self.NoLerpThisFrame();

		dir = currentEnemy->origin - teleport_position;
		angles = dir.toAngles();

		angles[ROLL] = 0;
		angles[PITCH] = 0;

		self.setAngles( angles );
		}

	return BEHAVIOR_SUCCESS;
	}

void Teleport::End
	(
	Actor &
	)

	{
	}

/****************************************************************************

  TeleportToPlayer Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, TeleportToPlayer, NULL )
	{
		{ NULL, NULL }
	};


void TeleportToPlayer::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void TeleportToPlayer::Begin
	(
	Actor &
	)

	{

	}

qboolean	TeleportToPlayer::TestPosition
	(
	Actor &self,
	int test_pos,
	Vector &good_position,
	Entity* player,
	qboolean	use_player_dir
	)
	{
	Vector test_position;
	Vector player_angles;
	Vector player_forward;
	Vector player_left;
	trace_t trace;

	Q_UNUSED(test_pos);

	// Get the position to test
	test_position = player->origin;
	
	if ( use_player_dir )
		{
		// Get the player direction info

		player_angles = player->angles;
		player_angles.AngleVectors( &player_forward, &player_left );
		
		// Check Behind the Player
		test_position -= player_forward * 75.0f;
		}
	else
		{
		
		// Check Behind the Player
		test_position += Vector(-60, 0, 0);
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
   
	if ( !self.IsEntityAlive( player ) || !self.sensoryPerception->CanSeeEntity( Vector( trace.endpos ), player , true , true ) )
			return false;


	// This is a good position

	good_position = trace.endpos;
	return true;
	}

BehaviorReturnCode_t	TeleportToPlayer::Evaluate
	(
	Actor &self
	)

	{
	int current_position;
	Vector teleport_position;
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
		if ( temp_player && !( temp_player->flags & FlagNotarget ) )
			{
			player = temp_player;
			break;
			}
		}

	if ( !player )
		return BEHAVIOR_SUCCESS;
	
	// Default the teleport position to where we are now

	teleport_position = self.origin;
	teleport_position_found = false;

	// Always teleport BEHIND the player - - we don't want him to see us pop in.
	current_position = TELEPORT_BEHIND;

	// Test this position
	if ( TestPosition( self, current_position, new_position, player, true ) )
		{
		teleport_position = new_position;
		teleport_position_found = true;
		}
	else
		{
			if ( TestPosition( self, current_position, new_position, player, false ) )
				{
				teleport_position = new_position;
				teleport_position_found = true;
				}
		}

	// Do teleport stuff
	if ( teleport_position_found )
		{
		self.setOrigin( teleport_position );
		self.NoLerpThisFrame();

		dir = player->origin - teleport_position;
		angles = dir.toAngles();

		angles[ROLL] = 0.0f;
		angles[PITCH] = 0.0f;

		self.setAngles( angles );
		}

	return BEHAVIOR_SUCCESS;
	}

void TeleportToPlayer::End
	(
	Actor &
	)

	{
	}

/****************************************************************************

  TeleportToPosition Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, TeleportToPosition, NULL )
	{
		{ &EV_Behavior_Args,			&TeleportToPosition::SetArgs },
		{ NULL, NULL }
	};

void TeleportToPosition::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void TeleportToPosition::SetArgs
	(
	Event *ev
	)

	{
	teleport_position_name = ev->GetString( 1 );
	number_of_teleport_positions = ev->GetInteger( 2 );
	}

void TeleportToPosition::Begin
	(
	Actor &
	)

	{
	}

BehaviorReturnCode_t	TeleportToPosition::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	Vector angles;
	trace_t trace;
	str pathnode_name;
	PathNode *goal;
	Vector teleport_position;
	Vector attack_position;
	float half_height;
	//Entity *effect;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();


	// Get the pathnode name to teleport to

	pathnode_name = teleport_position_name;
	pathnode_name += (int)G_Random( (float)number_of_teleport_positions ) + 1 ;

	// Find the path node

	goal = thePathManager.FindNode( pathnode_name );

	if ( !goal )
		{
		gi.WDPrintf( "Can't find position %s\n", pathnode_name.c_str() );
		return BEHAVIOR_SUCCESS;
		}

	// Set the teleport position

	teleport_position = goal->origin;

	// Kill anything at this position

	half_height = self.maxs.z / 2.0f;
	attack_position = teleport_position;
	attack_position.z += half_height;

	MeleeAttack( attack_position, attack_position, 10000.0f, &self, MOD_TELEFRAG, self.maxs.x, -half_height, half_height, 0 );

	// Test to see if we can fit at the new position

	trace = G_Trace( teleport_position + Vector( "0 0 64" ), self.mins, self.maxs, teleport_position - Vector( "0 0 128" ), &self, MASK_PATHSOLID, false, "TeleportToPosition" );
	//trace = G_Trace( teleport_position, self.mins, self.maxs, teleport_position, &self, MASK_PATHSOLID, false, "TeleportToPosition" );

	if ( trace.allsolid )
		{
		gi.WDPrintf( "Failed to teleport to %s\n", goal->targetname.c_str() );
		return BEHAVIOR_SUCCESS;
		}

	teleport_position = trace.endpos;

	// Do teleport stuff

   /*
	// Spawn in teleport effect at old positiond

	effect = new Entity( EntityCreateFlagAnimate );
	effect->setModel( "fx_teleport3.tik" );
	effect->setOrigin( self.origin );
	effect->setScale( 2.0f );
	effect->setSolidType( SOLID_NOT );
	effect->animate->RandomAnimate( "idle", EV_Remove );
	//effect->Sound( "snd_teleport" );
   */

	// Set new position

	self.setOrigin( teleport_position );

	// Spawn in teleport effect at new position
   /*
	effect = new Entity( EntityCreateFlagAnimate );
	effect->setModel( "fx_teleport3.tik" );
	effect->setOrigin( self.origin );
	effect->setScale( 2.0f );
	effect->setSolidType( SOLID_NOT );
	effect->animate->RandomAnimate( "idle", EV_Remove );
	//effect->Sound( "snd_teleport" );
   */

	self.NoLerpThisFrame();

	if ( currentEnemy )
		{
		dir = currentEnemy->origin - teleport_position;
		angles = dir.toAngles();

		angles[ROLL] = 0.0f;
		angles[PITCH] = 0.0f;

		self.setAngles( angles );
		}

	return BEHAVIOR_SUCCESS;
	}

void TeleportToPosition::End
	(
	Actor &
	)

	{
	}

/****************************************************************************

  GhostAttack Class Definition

****************************************************************************/

#define GHOST_ATTACK_START  0
#define GHOST_ATTACK_END    1

#define GHOST_ATTACK_SPEED  350.0f

CLASS_DECLARATION( Behavior, GhostAttack, NULL )
	{
		{ &EV_Behavior_Args,			&GhostAttack::SetArgs },
		{ NULL, NULL }
	};

void GhostAttack::SetArgs
	(
	Event *ev
	)

	{
	real_attack = ev->GetBoolean( 1 );
	}


void GhostAttack::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void GhostAttack::Begin
	(
	Actor &self
	)

	{
	mode = GHOST_ATTACK_START;

	fly.Begin( self );
	fly.SetTurnSpeed( 25.0f );
	fly.SetRandomAllowed( false );
	fly.SetSpeed( GHOST_ATTACK_SPEED );

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if ( currentEnemy )
		attack_position = currentEnemy->centroid;	
	else
		attack_position = self.origin;

	if ( !real_attack )
		{
		attack_position[0] += G_CRandom( 300.0f );
		attack_position[1] += G_CRandom( 300.0f );
		attack_position[2] += G_Random( 100.0f );
		}

	fly.SetGoalPoint( attack_position );

	attack_dir = attack_position - self.origin;

	if ( attack_dir == vec_zero )
		attack_dir = Vector(1, 1, 0);

	attack_dir.normalize();

	self.Sound( "snd_fadein", CHAN_VOICE );
	}

BehaviorReturnCode_t	GhostAttack::Evaluate
	(
	Actor &self
	)

	{
	Vector dir;
	float dist;
	float zdist;
	Vector new_pos;
	float new_alpha;
	float light_radius;
	float r, g, b;
	qboolean success;
	Vector start;
	Vector end;
	Event *event;


	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( mode == GHOST_ATTACK_START )
		{
		// Move closer to the enemy

		fly.Evaluate( self );

		// Get the new distance info

		dir = attack_position - self.origin;

		dist = dir.length();

		zdist = dir.z;

		if ( zdist < 0.0f )
			zdist = -zdist;

		dir.z = 0.0f;

		// If we are close enough change to shootable_only

		if ( real_attack && ( dist < 200.0f ) )
			{
			// Attackable now

			self.setSolidType( SOLID_BBOX );
			self.setContents( CONTENTS_SHOOTABLE_ONLY );

			event = new Event( EV_Actor_SetTargetable );
			event->AddInteger( 1 );
			self.ProcessEvent( event );
			}

		// If we are close enough damage enemy and goto end mode

		start = self.origin;
		end   = self.origin + ( attack_dir * 1.0f );

		if ( real_attack )
			{
			success = MeleeAttack( start, end, 7.5f, &self, MOD_LIFEDRAIN, 32.0f, 0.0f, 64.0f, 0.0f );
			self.AddStateFlag( STATE_FLAG_MELEE_HIT );
			}
		else
			success = false;

		if ( success || ( dist <= GHOST_ATTACK_SPEED / 40.0f ) )
			{
			// Attack mode is done go to retreat mode

			if ( self.attack_blocked && ( self.attack_blocked_time == level.time ) )
				{
				Vector retreat_angles;

				dir = attack_dir * -1.0f;

				retreat_angles = dir.toAngles();
				retreat_angles[YAW] += G_CRandom( 45.0f );
				retreat_angles[PITCH] += G_CRandom( 30.0f );
				retreat_angles.AngleVectors( &dir );

				dir *= 500.0f;

				self.setAngles( retreat_angles );
				}
			else
				{
				dir = attack_dir;

				dir.z = 0.0f;
				dir.normalize();
				dir *= 1000.0f;
				dir.z = 300.0f;
				}

			retreat_position = self.origin + dir;

			fly.SetGoalPoint( retreat_position );

			mode = GHOST_ATTACK_END;
			}

		// Fade in depending on how close we are to attack position

		if ( dist > 400.0f )
			new_alpha = 0.0f;
		else
			new_alpha = ( 400.0f - dist ) / 400.0f;

		if ( new_alpha > 0.4f )
			new_alpha = 0.4f;

		r = new_alpha / 0.4f;
		g = new_alpha / 0.4f;
		b = new_alpha / 0.4f;

		light_radius = 0.0f;

		G_SetConstantLight( &self.edict->s.constantLight, &r, &g, &b, &light_radius );

		self.setAlpha( new_alpha );
		}
	else
		{
		// Move away from enemy

		fly.Evaluate( self );

		// Get the new distance info

		dir = attack_position - self.origin;
		dist = dir.length();

		if ( real_attack && ( dist > 200.0f ) )
			{
			// Not attackable again

			self.setSolidType( SOLID_NOT );
			self.setContents( 0 );

			event = new Event( EV_Actor_SetTargetable );
			event->AddInteger( 0 );
			self.ProcessEvent( event );
			}

		// Fade out depending on how far we are from the attack position

		if ( dist > 400.0f )
			new_alpha = 0.0f;
		else
			new_alpha = ( 400.0f - dist ) / 400.0f;

		if ( new_alpha > 0.4f )
			new_alpha = 0.4f;

		r = new_alpha / 0.4f;
		g = new_alpha / 0.4f;
		b = new_alpha / 0.4f;

		light_radius = 0.0f;

		G_SetConstantLight( &self.edict->s.constantLight, &r, &g, &b, &light_radius );

		self.setAlpha( new_alpha );

		// See if we are far enough to be done

		if ( new_alpha == 0.0f )
			return BEHAVIOR_SUCCESS;
		}
	return BEHAVIOR_EVALUATING;
	}

void GhostAttack::End
	(
	Actor &self
	)

	{
	// Make sure we can't be shot any more

	self.setSolidType( SOLID_NOT );
	self.setContents( 0 );

	fly.End( self );
	}

/****************************************************************************

  Levitate Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, Levitate, NULL )
	{
		{ &EV_Behavior_Args,			&Levitate::SetArgs },
		{ NULL, NULL }
	};

void Levitate::SetArgs
	(
	Event *ev
	)

	{
	distance = ev->GetFloat( 1 );
	speed = ev->GetFloat( 2 );
	}


void Levitate::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void Levitate::Begin
	(
	Actor &self
	)

	{
	final_z = self.origin.z + distance;
	}

BehaviorReturnCode_t	Levitate::Evaluate
	(
	Actor &self
	)

	{
	trace_t trace;
	Vector start;
	Vector end;


	start = self.origin;
	end   = self.origin;

	if ( final_z < self.origin.z )
		{
		end.z -= speed;

		if ( end.z < final_z )
			end.z = final_z;
		}
	else
		{
		end.z += speed;

		if ( end.z > final_z )
			end.z = final_z;
		}

	trace = G_Trace( start, self.mins, self.maxs, end, &self, self.edict->clipmask, false, "Levitate" );

	if ( trace.fraction != 1.0 )
		return BEHAVIOR_SUCCESS;

	if ( end.z == final_z )
		return BEHAVIOR_SUCCESS;

	self.setOrigin( trace.endpos );

	return BEHAVIOR_EVALUATING;
	}

void Levitate::End
	(
	Actor &
	)

	{
	}

/****************************************************************************
*****************************************************************************

  Utility functions

*****************************************************************************
****************************************************************************/



Vector ChooseRandomDirection
   (
   Actor &self,
   const Vector &previousdir,
   float *time,
   int disallowcontentsmask,
   qboolean usepitch
   )
   {
   //static float x[ 9 ] = { 0, 22, -22, 45, -45, 0, 22, -22, 45 };
	static float x[ 5 ] = { 0, 22, -22, 0, 22 };
   Vector dir;
   Vector ang;
   Vector bestdir;
   Vector newdir;
   Vector step;
	Vector endpos;
	Vector groundend;
   float bestfraction;
   trace_t trace;
	trace_t groundtrace;
   int i;
   int k;
   int t;
   int u;
   int contents;
   Vector centroid;
	float random_yaw;
	float movespeed;

	//if ( self.movespeed != 1 )
	if ( self.movementSubsystem->getMoveSpeed() != 1.0f )
		movespeed = self.movementSubsystem->getMoveSpeed();
	else
		movespeed = 100.0f;

   centroid = self.centroid - self.origin;

	step = Vector( 0.0f, 0.0f, STEPSIZE );
   bestfraction = -1.0f;
   bestdir = self.origin;

	random_yaw = G_Random( 360.0f );

   for( i = 0; i <= 8; i++ )
      {
      t = (int)random_yaw + ( i * 45 );

      ang.y = self.angles.y + (float)t;

      if ( usepitch )
         {
         u = ( int )G_Random( 5.0f );

         //for( k = 0; k < 5; k++ )
			for( k = 0; k < 3; k++ )
            {
            ang.x = x[ k + u ];
            ang.AngleVectors( &dir, NULL, NULL );

            dir *= movespeed * (*time);
            dir += self.origin;
            trace = G_Trace( self.origin, self.mins, self.maxs, dir, &self, self.edict->clipmask, false, "ChooseRandomDirection 1" );

            if ( !trace.startsolid && !trace.allsolid )
               {
               newdir = Vector( trace.endpos );

					if ( disallowcontentsmask )
						{
						contents = gi.pointcontents( ( newdir + centroid ), 0 );

						if ( contents & disallowcontentsmask )
							continue;
						}

               if (
                     ( trace.fraction > bestfraction ) &&
                     ( newdir != bestdir ) &&
                     ( newdir != previousdir )
                  )
                  {
                  bestdir = newdir;
                  bestfraction = trace.fraction;

						if ( bestfraction > .9f )
							{
							*time *= bestfraction;

							return bestdir;
							}
                  }
               }
            }
         }
      else
         {
         ang.x = 0.0f;
         ang.AngleVectors( &dir, NULL, NULL );

			endpos = self.origin + ( dir * movespeed * (*time) ) + step;

         trace = G_Trace( self.origin + step, self.mins, self.maxs, endpos, &self, self.edict->clipmask, false, "ChooseRandomDirection 2" );

         if ( !trace.startsolid && !trace.allsolid )
            {
            newdir = Vector( trace.endpos );

            if ( disallowcontentsmask )
               {
               contents = gi.pointcontents( ( newdir + centroid ), 0 );

					if ( contents & disallowcontentsmask )
						continue;
					}

            if (
                  ( trace.fraction > bestfraction ) &&
                  ( newdir != bestdir ) &&
                  ( newdir != previousdir )
               )
               {
					groundend = endpos - ( step * 2.0f );

					groundtrace = G_Trace( endpos, self.mins, self.maxs, groundend, &self, self.edict->clipmask, false, "Chase::ChooseRandomDirection 3" );

					if ( groundtrace.fraction == 1.0f )
						trace.fraction /= 2.0f;

					if ( trace.fraction > bestfraction )
						{
						bestdir = newdir;
						bestfraction = trace.fraction;

						if ( bestfraction > .9f )
							{
							*time *= bestfraction;

							return bestdir;
							}
						}
               }
            }
         }
      }

   if ( bestfraction > 0.0f )
      {
      *time *= bestfraction;
      }
   else
      {
      *time = 0.0f;
      }

   return bestdir;
   }


//
// WayPoint Stuff
//
/****************************************************************************

  GotoWayPoint Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, GotoWayPoint, NULL )
	{
		{ &EV_Behavior_Args,			       &GotoWayPoint::SetArgs          },
		{ NULL, NULL }
	};

void GotoWayPoint::SetArgs
	(
	Event *ev
	)

	{
	path_name = ev->GetString( 1 );
	anim = ev->GetString( 2 );
	current_waypoint_name = path_name;
	current_waypoint = NULL;
	}

void GotoWayPoint::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );
	}

void GotoWayPoint::Begin
	(
	Actor &self
	)

	{
	if ( !anim.length() )
      {
		anim = "run";
      }

	if ( anim != self.animname || self.newanim.length() )
      {
		self.SetAnim( anim );
      }

	chase.Begin( self );	

	next_think_time = 0.0f;
	}

BehaviorReturnCode_t	GotoWayPoint::Evaluate
	(
	Actor &self
	)

	{
	//First Check if we are at a way point;
	Vector dest;
	Vector check;
	
	if (!current_waypoint)
		{
		current_waypoint = GetWayPoint( current_waypoint_name );
		if (!current_waypoint)
			return BEHAVIOR_SUCCESS;
		}
	
	dest = current_waypoint->origin;
	check = dest - self.origin;

	// Check if we're close enough
	if ( check.length() < 10.0f )
		{
		// Run Our Thread
		str waypointThread;
		waypointThread = current_waypoint->GetThread();

		if (waypointThread.length() )
			self.RunThread( waypointThread );

      // See if we have another point to go to
		if ( current_waypoint->target.length() == 0 )
			{
			return BEHAVIOR_SUCCESS;
			}

		// Check for new anim
		anim = "";
		anim = current_waypoint->GetActorAnim();
		
		if ( anim.length() )
			self.SetAnim( anim );			
		
		// Go To the Next Point
		current_waypoint_name = current_waypoint->target;
		current_waypoint = GetWayPoint( current_waypoint_name );		

		if (!current_waypoint)
			return BEHAVIOR_SUCCESS;

		dest = current_waypoint->origin;

		//Clear out anim strings
		anim = "";		
	
			
		}

	float radius=96.0f;
	chase.SetGoal( current_waypoint->origin, radius, self );
	chase.Evaluate(self);
   return BEHAVIOR_EVALUATING;
	}

void GotoWayPoint::End
	(
	Actor &self
	)

	{
	chase.End( self );		
	}

WayPointNode* GotoWayPoint::GetWayPoint
   (
	const str &waypoint_name
	)

	{
	Entity* ent_in_range;
	gentity_t *ed;

	for ( int i = 0; i < MAX_GENTITIES; i++ )
		{
		ed = &g_entities[i];

		if ( !ed->inuse || !ed->entity )
			{
			continue;
			}
				
			
		ent_in_range = g_entities[i].entity;

		if( ent_in_range->isSubclassOf( WayPointNode ) )
			{
			if (!Q_stricmp(ent_in_range->targetname.c_str() , waypoint_name.c_str() ))
				{
				return (WayPointNode*)ent_in_range;
				}
			}
		}
	
	return NULL;
	}

/****************************************************************************

  FlyCircleAroundWaypoint Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, FlyCircleAroundWaypoint, NULL )
	{
		{ &EV_Behavior_Args,			&FlyCircleAroundWaypoint::SetArgs },
		{ NULL, NULL }
	};

FlyCircleAroundWaypoint::FlyCircleAroundWaypoint()
	{
	anim = "fly";	
	nearestPlayer = false;
	fly_clockwise = true;
	}

void FlyCircleAroundWaypoint::SetArgs
	(
	Event *ev
	)

	{
   anim = ev->GetString( 1 );
	fly_clockwise = ev->GetBoolean( 2 );
	waypointname = ev->GetString( 3 );
	
   if (ev->NumArgs() > 3 )
		fly.SetSpeed( ev->GetFloat( 4 ) );

   if (ev->NumArgs() > 4 )
		fly.setAdjustYawAndRoll( ev->GetBoolean( 5 ) );
	
	if ( !Q_stricmp(waypointname.c_str() , "player" ) )
		nearestPlayer = true;
	
	}

void FlyCircleAroundWaypoint::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void FlyCircleAroundWaypoint::Begin
	(
	Actor &self
	)

	{
	//original_z = self.origin.z;

	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	fly.Begin( self );
	fly.SetTurnSpeed( 5.0f );
	}

BehaviorReturnCode_t	FlyCircleAroundWaypoint::Evaluate
	(
	Actor &self
	)

	{
   Vector goal;
	trace_t trace;
	Vector dir;
	Vector angle;
	Vector left;
	qboolean too_far = false;
	Vector new_dir;
	Vector fly_dir;
	WayPointNode *goalNode;
	
	
	goalNode = NULL;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( !self.IsEntityAlive( currentEnemy ) )
      {
      return BEHAVIOR_SUCCESS;
      }

	//if ( self.lastmove == STEPMOVE_OK )
	if ( self.movementSubsystem->getLastMove() == STEPMOVE_OK )
		{
		fly.SetTurnSpeed( 5.0f );
		
		if ( nearestPlayer )
			goalNode = GetWayPointNearestPlayer( self );
		else
			goalNode = GetWayPoint( self );

		if ( !goalNode )
			return BEHAVIOR_SUCCESS;
		
		dir = goalNode->origin - self.origin;
		dir.z = 0;

   	if ( dir.length() > 200.0f ) //radius
			{
			too_far = true;
			}


		angle = dir.toAngles();
		angle.AngleVectors( NULL, &left, NULL );

		if ( fly_clockwise )
			fly_dir = left;
		else
			fly_dir = left * -1.0f;

		dir.normalize();

		if ( too_far )
			{
			new_dir = ( fly_dir * 0.5f ) + ( dir * 0.5f );
			new_dir.normalize();
			}
		else
			{
			new_dir = fly_dir;
			}

		goal = self.origin + ( new_dir * 200.0f );

		trace = G_Trace( self.origin, self.mins, self.maxs, goal, &self, self.edict->clipmask, false, "FlyCircle" );

		if ( trace.fraction < 1.0f )
			{
			if ( too_far )
				trace.fraction /= 2.0f;

			new_dir = ( fly_dir * trace.fraction ) + ( dir * ( 1.0f - trace.fraction ) );
			new_dir.normalize();
   		goal = self.origin + ( new_dir * 200.0f );
			}
		else
			{
			goal = trace.endpos;
			}

		fly.SetGoalPoint( goal );
		}
	else
		{
		fly.SetTurnSpeed( 20.0f );
		}

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

WayPointNode *FlyCircleAroundWaypoint::GetWayPoint( Actor & )
	{
	Entity* ent_in_range;
	gentity_t *ed;

	for ( int i = 0; i < MAX_GENTITIES; i++ )
		{
		ed = &g_entities[i];

		if ( !ed->inuse || !ed->entity )
			{
			continue;
			}
				
			
		ent_in_range = g_entities[i].entity;

		if( ent_in_range->isSubclassOf( WayPointNode ) )
			{
			if (!Q_stricmp(ent_in_range->targetname.c_str() , waypointname.c_str() ))
				{
				return (WayPointNode*)ent_in_range;
				}
			}
		}
	
	return NULL;
	}

WayPointNode *FlyCircleAroundWaypoint::GetWayPointNearestPlayer( Actor &self)
	{
	Vector DistanceFromPlayer;
	float distance;
	Entity* ent_in_range;
	Entity* best_ent;

	gentity_t *ed;
	
	ent_in_range = NULL;
	best_ent = NULL;
	distance = 100000000;
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return NULL;

	for ( int i = 0; i < MAX_GENTITIES; i++ )
		{
		ed = &g_entities[i];

		if ( !ed->inuse || !ed->entity )
			{
			continue;
			}
				
			
		ent_in_range = g_entities[i].entity;

		if( ent_in_range->isSubclassOf( WayPointNode ) )
			{
			DistanceFromPlayer = ent_in_range->origin - currentEnemy->origin;
			if (DistanceFromPlayer.length() < distance )
				{
				best_ent = ent_in_range;
				distance = DistanceFromPlayer.length();
				}
			}
		}
	
	return (WayPointNode*)best_ent;
	}

void FlyCircleAroundWaypoint::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}

/****************************************************************************

  HelicopterFlyToPoint Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, HelicopterFlyToPoint, NULL )
	{
		{ NULL, NULL }
	};

HelicopterFlyToPoint::HelicopterFlyToPoint()
	{
	turn_speed = 10.0;
	old_turn_speed = turn_speed;
	speed = 480.0;
	random_allowed = true;
	force_goal = false;
	adjustYawAndRoll = true;
	offsetOrigin = false;
	avoidtime = 0;
	old_forward_speed = 0;
	stuck = 0;
	use_temp_goal = false;
	}

void HelicopterFlyToPoint::SetTurnSpeed( float new_turn_speed )
	{
	turn_speed = new_turn_speed;
	}

void HelicopterFlyToPoint::SetGoalPoint( const Vector &goal_point )
	{
	if ( goal_point != goal )
		avoidtime = 0;

	goal = goal_point;
	}

void HelicopterFlyToPoint::SetRandomAllowed( qboolean allowed )
	{
	random_allowed = allowed;
	}

void HelicopterFlyToPoint::SetSpeed( float speed_value )
	{
	speed = speed_value;
	}

void HelicopterFlyToPoint::ForceGoal( void )
	{
	force_goal = true;
	}

void HelicopterFlyToPoint::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void HelicopterFlyToPoint::Begin
	(
	Actor &self
	)

	{
   avoidtime = 0;
	//old_forward_speed = self.forwardspeed;
	old_forward_speed = self.movementSubsystem->getForwardSpeed();
	stuck = 0;
	use_temp_goal = false;
	}

BehaviorReturnCode_t	HelicopterFlyToPoint::Evaluate
	(
	Actor &self
	)

	{
   trace_t trace;
   Vector dir;
   Vector ang;
	float time;
	float length;
	float old_yaw;
	float old_pitch;
	
	float x_offset = 0.0f;
	float y_offset = 0.0f;
	//float z_offset = 0.0f;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();


   if (offsetOrigin)
		{
   	x_offset = self.centroid.x - self.origin.x;
   	y_offset = self.centroid.y - self.origin.y;
	   //z_offset = self.centroid.z - self.origin.z;
		
		self.origin.x+=x_offset;
	   self.origin.y+=y_offset;
	   //self.origin.z+=z_offset;
		}

	//if ( self.lastmove != STEPMOVE_OK )
	if ( self.movementSubsystem->getLastMove() != STEPMOVE_OK )
		stuck++;
	else
		stuck = 0;
	
	if ( ( stuck > 1 ) || ( avoidtime <= level.time ) )
      {
		time = G_Random( .3f ) + .3f;

		use_temp_goal = false;

		if ( !force_goal )
			{
			trace = G_Trace( self.origin, self.mins, self.maxs, goal, &self, self.edict->clipmask, false, "FlyToPoint" );
						
			if ( ( trace.fraction < 0.5f ) || ( stuck > 2 ) )
				{
				old_turn_speed = self.movementSubsystem->getTurnSpeed();
			   self.movementSubsystem->setTurnSpeed( 60.0f );
				//self.turnspeed = 60;
				temp_goal = ChooseRandomDirection( self, goal, &time, MASK_WATER, false );
				self.movementSubsystem->setTurnSpeed( old_turn_speed );
				//self.turnspeed = old_turn_speed;
				use_temp_goal = true;
				avoidtime = level.time + time;

				stuck = 0;
				}
			else
				{
				goal = trace.endpos;
				avoidtime = level.time + time;
				}
			}
		else
			{
			avoidtime = level.time + time;
			}
		
		if ( use_temp_goal )
			dir = temp_goal - self.origin;
		else
			dir = goal - self.origin;			

		length = dir.length();
      dir.normalize();
		
		//new stuff
		//self.FlightDir = dir;


      ang = dir.toAngles();
		
		if ( ( length > 150.0f ) && random_allowed && !use_temp_goal )
			{
			//ang[YAW]   += G_Random( 20 ) - 5.0;
			//ang[PITCH] += G_Random( 20 ) - 5.0;
			}

		target_angle = ang;

		target_angle[YAW]   = AngleNormalize180( target_angle[YAW] );
		target_angle[PITCH] = AngleNormalize180( target_angle[PITCH] );

		if ( currentEnemy )
			{
			Vector enemyDir;
			enemyDir = currentEnemy->origin - self.origin;
			enemyDir.normalize();
			target_angle[PITCH] = AngleNormalize180(-enemyDir.toPitch());
			if ( target_angle[PITCH] < -40.0f )
				target_angle[PITCH] = -40.0f;

			if ( target_angle[PITCH] > 40.0f )
				target_angle[PITCH] = 40.0f;

			target_angle[YAW] = enemyDir.toYaw();
			target_angle[ROLL] = 0.0f;
			}
      }

	target_angle[YAW]   = AngleNormalize360( target_angle[YAW] );
	target_angle[PITCH] = AngleNormalize360( target_angle[PITCH] );

	if ( (self.angles[YAW] != target_angle[YAW]) || (self.angles[PITCH] != target_angle[PITCH]) )
      {
		self.movementSubsystem->setForwardSpeed( speed * 0.8f );
		//self.forwardspeed = speed * 0.8f;
      }
	else
      {
		self.movementSubsystem->setForwardSpeed( speed );
		//self.forwardspeed = speed;
      }

	old_yaw   = self.angles[YAW];
	old_pitch = self.angles[PITCH];
	
	ang[YAW]   = LerpAngle( self.angles[YAW],   target_angle[YAW],   turn_speed );
	ang[PITCH] = LerpAngle( self.angles[PITCH], target_angle[PITCH], turn_speed );
	ang[ROLL]  = self.angles[ROLL];
	
	/*
	if( adjustYawAndRoll )
		{
	   
		if ( (AngleDelta( ang[YAW], old_yaw ) > 0) && (ang[ROLL] > 315 || ang[ROLL] <= 45) )
			{
			ang[ROLL] += 5;
			}
		else if ( (AngleDelta( ang[YAW], old_yaw ) < 0) && (ang[ROLL] < 45 || ang[ROLL] >= 315) )
			{
			ang[ROLL] -= 5;
			}
		else if ( (AngleDelta( ang[YAW], old_yaw ) == 0 ) )
			{
			if ( ang[ROLL] != 0 )
				{
				
				if ( ang[ROLL] < 5 || ang[ROLL] > 355 )
					{
					ang[ROLL] = 0;
					}
				else
					{
					if ( ang[ROLL] < 180 )
						ang[ROLL] += 5;
					else
						ang[ROLL] -= 5;
					}

				}
			}
		}
   */
	
	if ( adjustYawAndRoll )
		{
		//Vector travelAngleDelta = dir.toAngles();
		float yawDelta = dir[YAW] - self.angles[YAW];

		yawDelta = AngleNormalize360(yawDelta);

		if ( ( yawDelta > 0.0f ) && ( yawDelta <= 180.0f ) )
			{
			ang[ROLL] = self.angles[ROLL] + 2.0f;
			if ( AngleNormalize180(ang[ROLL]) > 30.0f )
				ang[ROLL] = 30.0f;
			}
	
		if ( ( yawDelta >= 180.0f ) && ( yawDelta <= 359.0f ) )
			{
			ang[ROLL] = self.angles[ROLL] - 2.0f;
			if ( AngleNormalize180(ang[ROLL]) < -30.0f )
				ang[ROLL] = -30.0f;
			}

		}
   
	ang[YAW]   = AngleNormalize360( ang[YAW] );
	ang[PITCH] = AngleNormalize360( ang[PITCH] );
	ang[ROLL]  = AngleNormalize360( ang[ROLL] );

	// Don't get stuck if still turning

	if ( ( AngleDelta( ang[YAW], old_yaw ) > .5f ) || ( AngleDelta( ang[YAW], old_yaw ) < -.5f ) ||
		  ( AngleDelta( ang[PITCH], old_pitch ) > .5f ) || ( AngleDelta( ang[PITCH], old_pitch ) < -.5f ) )
		{
		stuck = 0;
		}

	self.setAngles( ang );

	if (offsetOrigin)
		{
   	self.origin.x-=x_offset;
	   self.origin.y-=y_offset;
	   //self.origin.z-=z_offset;
		}

	return BEHAVIOR_EVALUATING;
	}

float HelicopterFlyToPoint::LerpAngle( float old_angle, float new_angle, float lerp_amount )
	{
	float diff;
	float abs_diff;
	float lerp_angle;

	new_angle = AngleNormalize360( new_angle );
	old_angle = AngleNormalize360( old_angle );

	diff = new_angle - old_angle;

	if ( diff > 180.0f )
      {
		diff -= 360.0f;
      }

	if ( diff < -180.0f )
      {
		diff += 360.0f;
      }

	lerp_angle = old_angle;
	
	abs_diff = diff;

	if ( abs_diff < 0.0f )
      {
		abs_diff = -abs_diff;
      }

	if ( abs_diff < lerp_amount )
      {
		lerp_amount = abs_diff;
      }

	if ( diff < 0.0f )
      {
		lerp_angle -= lerp_amount;
      }
	else if ( diff > 0.0f )
      {
		lerp_angle += lerp_amount;
      }
	
	lerp_angle = AngleNormalize360( lerp_angle );

	return lerp_angle;
	}

void HelicopterFlyToPoint::End
	(
	Actor &self
	)

	{
	self.movementSubsystem->setForwardSpeed( old_forward_speed );
	//self.FlightDir = vec_zero;
	}

/****************************************************************************

  FlyCircle Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, HelicopterFlyCircle, NULL )
	{
		{ &EV_Behavior_Args,			&HelicopterFlyCircle::SetArgs },
		{ NULL, NULL }
	};

HelicopterFlyCircle::HelicopterFlyCircle()
	{
	anim = "fly";	
	fly_clockwise = true;
	circle_player = false;
	}

void HelicopterFlyCircle::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	fly_clockwise = ev->GetBoolean( 2 );

	if (ev->NumArgs() > 2 )
		fly.setAdjustYawAndRoll( false );
		//fly.setAdjustYawAndRoll( ev->GetBoolean( 3 ) );
	
	if (ev->NumArgs() > 3 )
		circle_player = ev->GetBoolean ( 4 );
	else
		circle_player = false;	
	
	if (ev->NumArgs() > 4 )
		fly.SetSpeed( ev->GetFloat( 5 ) );
	}

void HelicopterFlyCircle::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void HelicopterFlyCircle::Begin
	(
	Actor &self
	)

	{
	//original_z = self.origin.z;

	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	fly.Begin( self );
	fly.SetTurnSpeed( 5.0f );
	}

BehaviorReturnCode_t	HelicopterFlyCircle::Evaluate
	(
	Actor &self
	)

	{
   Vector goal;
	trace_t trace;
	Vector dir;
	Vector angle;
	Vector left;
	qboolean too_far = false;
	Vector new_dir;
	Vector fly_dir;
	
	//Vector or;

	//or = self.currentEnemy->centroid - self.origin;
	//or = or.toAngles();

	//self.angles = or;
	//self.setAngles( or );
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( !self.IsEntityAlive( currentEnemy ) && !circle_player )
      {
      return BEHAVIOR_SUCCESS;
      }

	//if ( self.lastmove == STEPMOVE_OK )
	if ( self.movementSubsystem->getLastMove() == STEPMOVE_OK )
		{
		fly.SetTurnSpeed( 5.0f );

		if (circle_player)
			{
			Player *player = NULL;
			Player *temp_player = NULL;
			// Make sure the player is alive and well
			for(int i = 0; i < game.maxclients; i++)
				{
				temp_player = GetPlayer(i);		
			
				// don't target while player is not in the game or he's in notarget
				if ( temp_player && !( temp_player->flags & FlagNotarget ) )
					{
					player = temp_player;
					break;
					}
				}

			if ( !player )
				return BEHAVIOR_SUCCESS;
						
			dir = player->centroid - self.origin;
			dir.z = 0;

			//if ( dir.length() > (self.origin.z - player->centroid.z) / .5 )
			if ( dir.length() > 600.0f )
				{
				too_far = true;
				}

			}
		else
			{
			dir = currentEnemy->centroid - self.origin;
			dir.z = 0;

			if ( dir.length() > ( ( self.origin.z - currentEnemy->centroid.z) / 2.0f ) )
				{
				too_far = true;
				}

			}

		angle = dir.toAngles();

		angle.AngleVectors( NULL, &left, NULL );
		
		Vector newAngles = self.angles;
		if ( fly_clockwise )
			{
			fly_dir = left;

			newAngles[ROLL] -= 5.0f;
			if ( AngleNormalize180(newAngles[ROLL]) < -30.0f )
				newAngles[ROLL] = -30.0f;
			
			}			
		else
			{
			fly_dir = left * -1.0f;
			newAngles[ROLL] += 5.0f;
			if ( AngleNormalize180(newAngles[ROLL]) > 30.0f )
				newAngles[ROLL] = 30.0f;

			}
			
		self.setAngles(newAngles);

		dir.normalize();

		if ( too_far )
			{
			new_dir = ( fly_dir * 0.5f ) + ( dir * 0.5f );
			new_dir.normalize();
			}
		else
			{
			new_dir = fly_dir;
			}

		goal = self.origin + ( new_dir * 200.0f );

		trace = G_Trace( self.origin, self.mins, self.maxs, goal, &self, self.edict->clipmask, false, "FlyCircle" );

		if ( trace.fraction < 1.0f )
			{
			if ( too_far )
				trace.fraction /= 2.0f;

			new_dir = ( fly_dir * trace.fraction ) + ( dir * ( 1.0f - trace.fraction ) );
			new_dir.normalize();

			goal = self.origin + ( new_dir * 200.0f );
			}
		else
			{
			goal = trace.endpos;
			}

		fly.SetGoalPoint( goal );
		}
	else
		{
		fly.SetTurnSpeed( 20.0f );
		}

	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

void HelicopterFlyCircle::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}


/****************************************************************************

  HelicopterStrafeAttack Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, HelicopterStrafeAttack, NULL )
	{
		{ &EV_Behavior_Args,			&HelicopterStrafeAttack::SetArgs },
		{ NULL, NULL }
	};

HelicopterStrafeAttack::HelicopterStrafeAttack()
	{
	anim = "fly";
	turnTime = 5;
	setUpLerp = false;
	completedLerp = false;

	lerpStart = 0.0f;
	lerpEnd = 0.0f;

	startYaw = 0.0f;
	endYaw = 0.0f;
	}

void HelicopterStrafeAttack::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );
	}

void HelicopterStrafeAttack::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void HelicopterStrafeAttack::Begin
	(
	Actor &self
	)

	{
   self.SetAnim(anim);

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return;

	//First we find the waypoint closest to the player
	//These waypoints should be pointed along the axis
	//that we want the helicopter to strafe
	WayPointNode *wp = GetWayPointNearestPlayer(self);
	
	str tName = wp->Target();
	WayPointNode *target = GetWayPoint(self , tName );

	//Set up goal location
	//goal = target->origin;
	goal = wp->origin;
	goal.z = self.origin.z;
	
	//We need the cross product
	Vector wpForward;
	Vector wpUp;
	Vector cross;

   wpForward = target->origin - wp->origin;
	wpForward.normalize();
	

	wpUp.x = 0;
	wpUp.y = 0;
	wpUp.z = 1;
	
	CrossProduct(wpForward, wpUp, cross );

	
	
	//wpOrigin.AngleVectors(NULL,&left,NULL);

	//Get the vector from the player to the actor
	Vector playerToActor;
	playerToActor = self.origin - currentEnemy->origin;


	//Normalize the vectors
	playerToActor.normalize();
	cross.normalize();

	//Now we get the Dot product to see if towardPlayer
	float dot;
	dot = DotProduct(cross,playerToActor);
	

	//Now we get our actual direction Vector;	
	if ( dot > 0.0f )
		dir = cross * -1.0f;
	else
		dir = cross;

	
	//Now set our angles appropriately
	targetAngles = self.angles;
	targetAngles[YAW] = dir.toYaw();
	
	startYaw = AngleNormalize360(self.angles[YAW]);
	endYaw = AngleNormalize360(targetAngles[YAW]);
	
	if ( ( endYaw == 0.0f ) && ( startYaw > 180.0f ) )
		endYaw = 360.0f;

	}

BehaviorReturnCode_t	HelicopterStrafeAttack::Evaluate
	(
	Actor &self
	)

	{
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;
	
	if ( !setUpLerp )
		{
		lerpStart = level.time;
		lerpEnd = lerpStart + turnTime;
		setUpLerp = true;
		}
	
	if ( !completedLerp )
		LerpToNewAngle( self );


	//Get our travel destination
	Vector travelDir;
	travelDir = goal - self.origin;

	//Check if we are at our destination
	float length = travelDir.length();
	
	if ( length < 25.0f ) //We're close enough
		return BEHAVIOR_SUCCESS;
	
	travelDir.normalize();
	//self.FlightDir = travelDir;
	self.movementSubsystem->setForwardSpeed( 300.0f );
		

	return BEHAVIOR_EVALUATING;
	}

WayPointNode *HelicopterStrafeAttack::GetWayPointNearestPlayer( Actor &self)
	{
	Vector DistanceFromPlayer;
	float distance;
	Entity* ent_in_range;
	Entity* best_ent;

	gentity_t *ed;
	
	ent_in_range = NULL;
	best_ent = NULL;
	distance = 100000000;
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return NULL;

	for ( int i = 0; i < MAX_GENTITIES; i++ )
		{
		ed = &g_entities[i];

		if ( !ed->inuse || !ed->entity )
			{
			continue;
			}
				
			
		ent_in_range = g_entities[i].entity;

		if( ent_in_range->isSubclassOf( WayPointNode ) )
			{
			DistanceFromPlayer = ent_in_range->origin - currentEnemy->origin;
			if (DistanceFromPlayer.length() < distance )
				{
				best_ent = ent_in_range;
				distance = DistanceFromPlayer.length();
				}
			}
		}
	
	return (WayPointNode*)best_ent;
	}

WayPointNode *HelicopterStrafeAttack::GetWayPoint( Actor &self , const str &name )
	{
	Entity* ent_in_range;
	gentity_t *ed;

	Q_UNUSED(self);

	for ( int i = 0; i < MAX_GENTITIES; i++ )
		{
		ed = &g_entities[i];

		if ( !ed->inuse || !ed->entity )
			{
			continue;
			}
				
			
		ent_in_range = g_entities[i].entity;

		if( ent_in_range->isSubclassOf( WayPointNode ) )
			{
			if (!Q_stricmp(ent_in_range->targetname.c_str() , name.c_str() ))
				{
				return (WayPointNode*)ent_in_range;
				}
			}
		}
	
	return NULL;
	}

void HelicopterStrafeAttack::LerpToNewAngle( Actor &self )
	{
	Vector ang;
	float newYaw;

	ang = self.angles;
	
	newYaw = ( endYaw - startYaw ) / ( lerpEnd - lerpStart );
	newYaw = newYaw * (level.time - lerpStart );
	newYaw = newYaw + startYaw;
		
	ang[YAW] = newYaw;
	// Clamp it down in the end
	if ( level.time >= lerpEnd )
		{
		ang[YAW] = endYaw;
		completedLerp = true;
		}

	self.setAngles(ang);

	}

void HelicopterStrafeAttack::End
	(
	Actor &
	)

	{
	//self.FlightDir = vec_zero;
	}


/****************************************************************************

  HelicopterFlyToWaypoint Class Definition

****************************************************************************/


CLASS_DECLARATION( Behavior, HelicopterFlyToWaypoint, NULL )
	{
		{ &EV_Behavior_Args,			&HelicopterFlyToWaypoint::SetArgs },
		{ NULL, NULL }
	};

HelicopterFlyToWaypoint::HelicopterFlyToWaypoint()
	{
	anim = "fly";	
	nearestPlayer = false;
	nearestPlayerTarget = false;

	}

void HelicopterFlyToWaypoint::SetArgs
	(
	Event *ev
	)

	{
   anim = ev->GetString( 1 );
	waypointname = ev->GetString( 2 );
	
   if (ev->NumArgs() > 2 )
		fly.SetSpeed( ev->GetFloat( 3 ) );

   if (ev->NumArgs() > 3 )
		fly.setAdjustYawAndRoll( ev->GetBoolean( 4 ) );
	
	if ( !Q_stricmp(waypointname.c_str() , "player" ) )
		nearestPlayer = true;

	if ( !Q_stricmp(waypointname.c_str() , "target" ) )
		nearestPlayerTarget = true;
	
	}

void HelicopterFlyToWaypoint::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );
	}

void HelicopterFlyToWaypoint::Begin
	(
	Actor &self
	)

	{
	if ( anim.length() )
      {
		self.SetAnim( anim );
      }

	fly.Begin( self );
	fly.SetTurnSpeed( 5.0f );
	}

BehaviorReturnCode_t	HelicopterFlyToWaypoint::Evaluate
	(
	Actor &self
	)

	{
   Vector goal;
	WayPointNode *goalNode;
	goalNode = NULL;

	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return BEHAVIOR_SUCCESS;

	if ( !self.IsEntityAlive( currentEnemy ) )
      {
      return BEHAVIOR_SUCCESS;
      }

	//if ( self.lastmove == STEPMOVE_OK )
	if ( self.movementSubsystem->getLastMove() == STEPMOVE_OK )
		{
		fly.SetTurnSpeed( 5.0f );
		
		if ( nearestPlayer )
			goalNode = GetWayPointNearestPlayer( self );
		else if ( nearestPlayerTarget )
			{
			goalNode = GetWayPointNearestPlayer( self );
			waypointname = goalNode->TargetName();			
			}
		
		if ( !nearestPlayer )
			goalNode = GetWayPoint( self );

		}
	

	if ( !goalNode )
		return BEHAVIOR_SUCCESS;

	goal = goalNode->origin;
	goal.z = self.origin.z;

	//Check if we have arrived
	Vector dir;
	dir = self.origin - goal;
	
	if ( dir.length() <= 25.0f )
		return BEHAVIOR_SUCCESS;
	
	fly.SetGoalPoint( goal );
	fly.Evaluate( self );

	return BEHAVIOR_EVALUATING;
	}

WayPointNode *HelicopterFlyToWaypoint::GetWayPoint( Actor & )
	{
	Entity* ent_in_range;
	gentity_t *ed;

	for ( int i = 0; i < MAX_GENTITIES; i++ )
		{
		ed = &g_entities[i];

		if ( !ed->inuse || !ed->entity )
			{
			continue;
			}
				
			
		ent_in_range = g_entities[i].entity;

		if( ent_in_range->isSubclassOf( WayPointNode ) )
			{
			if (!Q_stricmp(ent_in_range->targetname.c_str() , waypointname.c_str() ))
				{
				return (WayPointNode*)ent_in_range;
				}
			}
		}
	
	return NULL;
	}

WayPointNode *HelicopterFlyToWaypoint::GetWayPointNearestPlayer( Actor &self)
	{
	Vector DistanceFromPlayer;
	float distance;
	Entity* ent_in_range;
	Entity* best_ent;

	gentity_t *ed;
	
	ent_in_range = NULL;
	best_ent = NULL;
	distance = 100000000;
	
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return NULL;

	for ( int i = 0; i < MAX_GENTITIES; i++ )
		{
		ed = &g_entities[i];

		if ( !ed->inuse || !ed->entity )
			{
			continue;
			}
				
			
		ent_in_range = g_entities[i].entity;

		if( ent_in_range->isSubclassOf( WayPointNode ) )
			{
			DistanceFromPlayer = ent_in_range->origin - currentEnemy->origin;
			if (DistanceFromPlayer.length() < distance )
				{
				best_ent = ent_in_range;
				distance = DistanceFromPlayer.length();
				}
			}
		}
	
	return (WayPointNode*)best_ent;
	}

void HelicopterFlyToWaypoint::End
	(
	Actor &self
	)

	{
	fly.End( self );
	}


/****************************************************************************

  GetWithinRangeOfPlayer Class Definition

****************************************************************************/

CLASS_DECLARATION( Behavior, GetWithinRangeOfPlayer, NULL )
	{
		{ &EV_Behavior_Args,			&GetWithinRangeOfPlayer::SetArgs },
		{ NULL, NULL }
	};

void GetWithinRangeOfPlayer::SetArgs
	(
	Event *ev
	)

	{
	anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		forever = ev->GetBoolean( 2 );
	else
		forever = true;

	if ( ev->NumArgs() > 2 )
		speed = ev->GetFloat( 3 );
	else
		speed = 100;

   if ( ev->NumArgs() > 3 )
      chase.SetRadius( ev->GetFloat( 4 ) );

	}

void GetWithinRangeOfPlayer::ShowInfo
	(
	Actor &self
	)

	{
   Behavior::ShowInfo( self );

   gi.Printf( "\nchase:\n" );
   chase.ShowInfo( self );
	}

void GetWithinRangeOfPlayer::Begin
	(
	Actor &self
	)

	{
	if ( !anim.length() )
      {
		anim = "run";
      }

	if ( anim != self.animname || self.newanim.length() )
      {
		self.SetAnim( anim, EV_Actor_NotifyBehavior );
      }

	chase.Begin( self );
	wander.Begin( self );
	
	self.movementSubsystem->setForwardSpeed( speed );

	next_think_time = 0;

   startRangeMin = self.preferredMax;
   startRangeMax = startRangeMin * 1.25;
	}

BehaviorReturnCode_t	GetWithinRangeOfPlayer::Evaluate
	(
	Actor &self
	)

	{
	Steering::ReturnValue result=Steering::EVALUATING;
	
	Player *player = NULL;
	Player *temp_player = NULL;
	// Make sure the player is alive and well
	for(int i = 0; i < game.maxclients; i++)
		{
		temp_player = GetPlayer(i);		
		
		// don't target while player is not in the game or he's in notarget
		if( temp_player && !( temp_player->flags & FlagNotarget ) )
			{
			player = temp_player;
			break;
			}
		}

	if ( !player )
		return BEHAVIOR_SUCCESS;

	if ( next_think_time <= level.time )
		{
		if ( self.groundentity && ( self.groundentity->s.number == player->entnum ) )
			{
			wander.Evaluate( self );
			result = Steering::SUCCESS;
			}
		else
			{
			float radius=96.0f;
			chase.SetGoal( player, radius, self );			

			result = chase.Evaluate( self );
			}

		if ( self.GetActorFlag( ACTOR_FLAG_SIMPLE_PATHFINDING ) )
			next_think_time = level.time + ( 2.0f * FRAMETIME );
		else
			next_think_time = 0.0f;
		}
	else
		result = Steering::SUCCESS;

	if ( !forever && ( result == Steering::SUCCESS) )
		return BEHAVIOR_SUCCESS;

	if ( !forever)
		{
		switch (result)
			{
			case Steering::EVALUATING:
				return BEHAVIOR_EVALUATING;
				break;
			
			case Steering::SUCCESS:
				return BEHAVIOR_SUCCESS;
				break;
			
			case Steering::FAILED_BLOCKED_BY_ENEMY:
			// lint -fallthrough
			case Steering::FAILED_BLOCKED_BY_CIVILIAN:
			// lint -fallthrough
			case Steering::FAILED_BLOCKED_BY_FRIEND:
			// lint -fallthrough
			case Steering::FAILED_BLOCKED_BY_TEAMMATE:
			// lint -fallthrough
			case Steering::FAILED_BLOCKED_BY_WORLD:
			// lint -fallthrough
			case Steering::FAILED_BLOCKED_BY_DOOR:
			// lint -fallthrough
			case Steering::FAILED_NO_PATH:
			// lint -fallthrough
			case Steering::FAILED:
			// lint -fallthrough
				return BEHAVIOR_FAILED;
				break;
			}
		}
	
	return BEHAVIOR_EVALUATING;
	}

void GetWithinRangeOfPlayer::End
	(
	Actor &self
	)

	{
	chase.End( self );
	wander.End( self );
	self.movementSubsystem->setForwardSpeed( 0 );
	}
