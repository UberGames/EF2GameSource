//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/talk.cpp                                   $
// $Revision:: 16                                                             $
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
//	Talk Implementation
// 
// PARAMETERS:
//
// ANIMATIONS:
//--------------------------------------------------------------------------------

#include "actor.h"
#include "talk.hpp"
#include <qcommon/gameplaymanager.h>

Event EV_TalkBehavior_GreetingDone
	(
	"greetingdone",
	EV_DEFAULT,
	NULL,
	NULL,
	"Notifies the talk behavior the greeting dialog is done"
	);

/****************************************************************************

Talk Class Definition

****************************************************************************/
CLASS_DECLARATION( Behavior, Talk, NULL )
{
	{ &EV_TalkBehavior_GreetingDone, &Talk::GreetingDone	},
	{ &EV_Behavior_AnimDone,		 &Talk::AnimDone		},
	{ NULL, NULL }
};

void Talk::SetUser
	(
	Sentient *user
	)

{
	ent_listening = user;
}

void Talk::AnimDone
	(
	Event *
	)

{
	turnto.ProcessEvent( EV_Behavior_AnimDone );
	animDone = true;
}

void Talk::GreetingDone( Event * )
{
	mode = TALK_MODE_TURN_TO;
}

void Talk::Begin
	(
	Actor &self
	)

{
	Vector dir;
	Vector angles;
	const char *anim_name;
	oldAnimName = "";


	anim_name = self.animname;
	last_headwatch_target = self.headWatcher->GetWatchTarget();
	animDone = true;
	move_allowed = true;
	turnto.SetUseTurnAnim( false );
	/*
	if ( strncmp( anim_name, "sit_leanover", 12 ) == 0 )
	{
	move_allowed = false;
	}
	else if ( strncmp( anim_name, "sit", 3 ) == 0 )
	{
	move_allowed = false;
	self.SetAnim( "sit_talk" );
	}
	else if ( strncmp( anim_name, "talk_sit_stunned", 15 ) == 0 )
	{
	move_allowed = false;
	}
	else if ( strncmp( anim_name, "talk_headset", 12 ) == 0 )
	{
	move_allowed = true;
	}
	else if ( strncmp( anim_name, "stand_hypnotized", 16 ) == 0 )
	{
	move_allowed = false;
	}
	else if ( strncmp( anim_name, "talk_hipnotic", 13 ) == 0 )
	{
	move_allowed = false;
	}
	else if ( strncmp( anim_name, "rope", 4 ) == 0 )
	{
	move_allowed = false;
	}
	else
	{
	move_allowed = true;
	self.SetAnim( "talk" );
	}*/


	if ( self.talkMode == TALK_IGNORE )
	{
		move_allowed = false;
	}

	if ( self.talkMode == TALK_HEADWATCH )
	{
		move_allowed = false;		
		oldAnimName = self.animname;
	}

	if ( self.talkMode != TALK_IGNORE )
	{
		if ( ent_listening )
			self.headWatcher->SetWatchTarget( ent_listening );
	}

	mode = TALK_MODE_PLAY_GREETING;

	Entity *currentEnemy = NULL;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if ( currentEnemy )
		mode = TALK_MODE_COMBAT;

	original_yaw = self.angles[YAW];

	if ( ent_listening )
	{
		dir    = ent_listening->centroid - self.centroid;
		angles = dir.toAngles();
		yaw    = angles[YAW];
	}

	/*if ( !self.GetActorFlag(ACTOR_FLAG_PLAYING_DIALOG_ANIM) )
	{
	self.SetAnim( "conv" , EV_Actor_NotifyBehavior );		
	self.SetActorFlag( ACTOR_FLAG_PLAYING_DIALOG_ANIM , true );
	}
	*/

}

BehaviorReturnCode_t	Talk::Evaluate
	(
	Actor &self
	)

{
	Vector dir;
	Vector angles;
	str greetingDialog;
	str combatDialog;
	float greetingDialogLength;
	Event *greetingEvent;
	char localizedDialogName[MAX_QPATH];

	//Event *event;

	if ( !ent_listening )
		mode = TALK_MODE_TURN_BACK;

	if ( !self.GetActorFlag(ACTOR_FLAG_PLAYING_DIALOG_ANIM) && animDone && self.GetActorFlag(ACTOR_FLAG_DIALOG_PLAYING) )
	{
		if ( self.useConvAnims )
		{
			//self.SetActorFlag(ACTOR_FLAG_CAN_CHANGE_ANIM , true);
			self.SetAnim( "talk" , EV_Actor_NotifyBehavior );		
			self.SetActorFlag( ACTOR_FLAG_PLAYING_DIALOG_ANIM , true );
			self.SetActorFlag(ACTOR_FLAG_CAN_CHANGE_ANIM , false );
			animDone = false;
		}
	}

	if ( self.useConvAnims && animDone && mode != TALK_MODE_WAIT && mode != TALK_MODE_TURN_TO )
	{
		self.SetActorFlag(ACTOR_FLAG_CAN_CHANGE_ANIM , true);
		self.SetAnim( "conv-idle" , EV_Actor_NotifyBehavior );		
		//self.SetActorFlag(ACTOR_FLAG_CAN_CHANGE_ANIM , false );
		//self.SetAnim( "idle" );
		animDone = false;		
	}

	if ( self.useConvAnims && /*animDone &&*/ mode == TALK_MODE_WAIT )
	{
		self.SetActorFlag(ACTOR_FLAG_CAN_CHANGE_ANIM , true);
		if ( oldAnimName.length() )
			self.SetAnim( oldAnimName );
		else
			self.SetAnim( "idle" );
		animDone = false;		
	}

	if ( !self.GetActorFlag(ACTOR_FLAG_PLAYING_DIALOG_ANIM) && !self.GetActorFlag(ACTOR_FLAG_DIALOG_PLAYING) && !self.talkMode == TALK_HEADWATCH )
	{
		self.SetAnim( "idle" );
		animDone = true;
	}

	if ( self.GetActorFlag(ACTOR_FLAG_PLAYING_DIALOG_ANIM) && animDone )
	{
		if ( oldAnimName.length() )
		{
			self.SetActorFlag(ACTOR_FLAG_CAN_CHANGE_ANIM , true );
			self.SetAnim( oldAnimName );
			self.SetActorFlag(ACTOR_FLAG_CAN_CHANGE_ANIM , false );
			if ( mode != TALK_MODE_TURN_BACK )
				mode = TALK_MODE_WAIT;

		}
	}

	switch( mode )
	{
	case TALK_MODE_COMBAT:
		combatDialog = self.FindDialog( ent_listening, DIALOG_TYPE_COMBAT );
		if ( !combatDialog.length() )
			return BEHAVIOR_SUCCESS;

		self.PlayDialog( ent_listening , -1.0f, -1.0f, combatDialog.c_str() );
		return BEHAVIOR_SUCCESS;

		break;

	case TALK_MODE_PLAY_GREETING:
		greetingDialog = self.FindDialog( ent_listening, DIALOG_TYPE_GREETING );
		if ( !greetingDialog.length() )
		{
			mode = TALK_MODE_TURN_TO;
			return BEHAVIOR_EVALUATING;
		}

		gi.LocalizeFilePath( greetingDialog.c_str(), localizedDialogName );
		greetingDialogLength = gi.SoundLength( localizedDialogName );

		if ( greetingDialogLength > 0 )
		{
			greetingEvent = new Event(EV_TalkBehavior_GreetingDone);
			PostEvent(greetingEvent , greetingDialogLength );

			Event *dialogEvent = new Event( EV_SimplePlayDialog );
			dialogEvent->AddString( greetingDialog.c_str() );
			ent_listening->ProcessEvent( dialogEvent );
			//ent_listening->Sound( greetingDialog );

			mode = TALK_MODE_WAIT_FOR_GREETING;
		}
		else
		{
			mode = TALK_MODE_TURN_TO;
		}

		break;			

	case TALK_MODE_WAIT_FOR_GREETING:
		//Waiting on the Greeting Done Event Here
		break;

	case TALK_MODE_TURN_TO :
		if ( move_allowed )
		{
			turnto.SetDirection( yaw );

			if ( !turnto.Evaluate( self ) )
			{
				mode = TALK_MODE_TALK;
				self.PlayDialog( ent_listening );

				/* event = new Event( EV_Player_WatchActor );
				event->AddEntity( &self );
				ent_listening->PostEvent(  event, 0.05 ); */
			}
		}
		else
		{
			mode = TALK_MODE_TALK;
			self.PlayDialog( ent_listening );

			/* event = new Event( EV_Player_WatchActor );
			event->AddEntity( &self );
			ent_listening->PostEvent(  event, 0.05 ); */
		}
		break;
	case TALK_MODE_TALK :

		if ( move_allowed )
		{
			dir    = ent_listening->centroid - self.centroid;
			angles = dir.toAngles();
			turnto.SetDirection( angles[YAW] );
			turnto.Evaluate( self );
		}

		if ( !self.GetActorFlag( ACTOR_FLAG_DIALOG_PLAYING ) )
		{
			mode = TALK_MODE_WAIT;
			self.state_flags &= ~STATE_FLAG_USED;

			// Tell player to stop watching us

			/* event = new Event( EV_Player_StopWatchingActor );
			event->AddEntity( &self );
			ent_listening->PostEvent(  event, 0 ); */

			//ent_listening->CancelEventsOfType( EV_Player_WatchActor );
		}
		else if ( !self.GetActorFlag( ACTOR_FLAG_RADIUS_DIALOG_PLAYING ) )
		{
			if ( !self.WithinDistance( ent_listening, self.radiusDialogRange  ) )
			{
				self.PlayRadiusDialog( ent_listening );
				/*
				int postive_response = true;
				str check_alias;
				if (postive_response)
				{
				check_alias = self.GetRandomAlias("radiusdialog_positive");
				if(check_alias.length())
				{
				self.PlayRadiusDialog(ent_listening, "radiusdialog_positive");
				}				
				}
				else
				{
				check_alias = self.GetRandomAlias("radiusdialog_negative");
				if(check_alias.length())
				{
				self.PlayRadiusDialog(ent_listening, "radiusdialog_negative");
				}			
				}
				*/
			}				
		}
		break;
	case TALK_MODE_WAIT :

		if ( move_allowed )
		{
			dir    = ent_listening->centroid - self.centroid;
			angles = dir.toAngles();
			turnto.SetDirection( angles[YAW] );
			turnto.Evaluate( self );
		}



		if ( !self.WithinDistance( ent_listening, 100.0f ) )
			mode = TALK_MODE_TURN_BACK;

		if ( self.state_flags & STATE_FLAG_USED )
		{
			mode = TALK_MODE_TURN_TO;
			self.SetActorFlag(ACTOR_FLAG_PLAYING_DIALOG_ANIM, false );

			dir    = ent_listening->centroid - self.centroid;
			angles = dir.toAngles();
			yaw    = angles[YAW];

			self.state_flags &= ~STATE_FLAG_USED;

			/* event = new Event( EV_Player_WatchActor );
			event->AddEntity( &self );
			ent_listening->PostEvent(  event, 0.05 ); */
		}
		break;
	case TALK_MODE_TURN_BACK :
		if ( move_allowed )
		{
			turnto.SetDirection( original_yaw );

			if ( !turnto.Evaluate( self ) )
				return BEHAVIOR_SUCCESS;
		}
		else
		{
			return BEHAVIOR_SUCCESS;
		}

		break;
	}

	return BEHAVIOR_EVALUATING;
}

void Talk::End( Actor &self	)
{
	self.SetActorFlag( ACTOR_FLAG_PLAYING_DIALOG_ANIM , false );
	self.SetActorFlag(ACTOR_FLAG_CAN_CHANGE_ANIM , true );

	self.ClearLegAnim();
	self.ClearTorsoAnim();

	if ( oldAnimName.length() )
	{
		self.SetAnim( oldAnimName );
	}
	else
	{
		self.SetAnim( "idle" );
	}

	if ( last_headwatch_target )
		self.headWatcher->SetWatchTarget( last_headwatch_target );
	else
		self.headWatcher->SetWatchTarget( NULL );
}


