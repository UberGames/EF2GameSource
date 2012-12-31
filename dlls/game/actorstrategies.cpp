//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/actorstrategies.cpp                        $
// $Revision:: 46                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:35p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// 
//
// DESCRIPTION:
// I am replacing the old way that Actor::Think was done by implementing a group of strategies for it
// instead.  This will form the foundation for new flexiblility within the actor class.  
//
// What I am trying to achieve is a specialized "think" for different types of actors.  A Boss, for instance,
// would use BossThink, an Non-Attacking-NPC could use NPCThink.  Using the event system we already have inplace
// it will be easy to change thinking modalities on the fly, allowing us to make a cowardly NPC turn into a 
// roaring death machine if he gets shot.
//

#include "_pch_cpp.h"
#include "actorstrategies.h"
#include "actor.h"
#include "entity.h"

extern cvar_t	*ai_showfailure;
//------------------------- CLASS ------------------------------
//
// Name:		ActorThink
// Base Class:	None
//
// Description:	Base class from which all Actor Think Strategies 
//				are derived.
//
// Method of Use:	
//				Deriving a new class from ActorThink is a good 
//				to allow Actor to exhibit new behavior. 
//
//--------------------------------------------------------------

//----------------------------------------------------------------
// Name:			DoArchive
// Class:			ActorThink
//
// Description:		Archives this instance of ActorThink
//
// Parameters:		
//					Archiver the class that receives/supplies 
//					archival info
//
// Returns:			None
//----------------------------------------------------------------
void ActorThink::DoArchive( Archiver & )
{
}

//----------------------------------------------------------------
// Name:			ProcessBehaviors
// Class:			ActorThink
//
// Description:		Processes all the Actors Behaviors
//
// Parameters:		
//					Actor in question
//
// Returns:			None
//----------------------------------------------------------------
void ActorThink::ProcessBehaviors( Actor &actor )
{
	bool showFailure;
	showFailure = ( ai_showfailure->integer != 0 );
	
	// Do the state machine for this creature
	actor.ProcessMasterStateMachine();
	actor.ProcessActorStateMachine();
	
	// Process the current behavior   
	
	if ( actor.behavior )      
	{
		actor.behaviorCode = actor.behavior->Evaluate( actor );
		
		if ( actor.behaviorCode == BEHAVIOR_FAILED )
		{
			actor.behaviorFailureReason = actor.behavior->GetFailureReason();
		}
		
		if ( actor.behaviorCode != BEHAVIOR_EVALUATING )
		{
			if ( stricmp( actor.behavior->getClassname(), "Talk" ) == 0 )
			{
				actor.EndBehavior();
				actor.EndMode();
			}
			else
			{
				actor.EndBehavior();
			}			
		}
		
		// Process state machine again because the behavior finished
		actor.ProcessActorStateMachine();
	}
	
	// Process the current head behavior
	if ( actor.headBehavior )
	{
		actor.headBehaviorCode = actor.headBehavior->Evaluate( actor );
		
		if ( actor.headBehaviorCode != BEHAVIOR_EVALUATING )
		{
			actor.EndHeadBehavior();
		}
		
		// Process state machine again because the behavior finished
		actor.ProcessActorStateMachine();
	}
	
	// Process the current eye behavior
	if ( actor.eyeBehavior )
	{
		actor.eyeBehaviorCode = actor.eyeBehavior->Evaluate( actor );
		
		if ( actor.eyeBehaviorCode != BEHAVIOR_EVALUATING )
		{
			actor.EndEyeBehavior();
		}
		
		// Process state machine again because the behavior finished
		actor.ProcessActorStateMachine();
	}
	
	// Process the current torso behavior
	if ( actor.torsoBehavior )
	{
		actor.torsoBehaviorCode = actor.torsoBehavior->Evaluate( actor );
		
		if ( actor.torsoBehaviorCode != BEHAVIOR_EVALUATING )
		{
			actor.EndTorsoBehavior();
		}
		
		// Process state machine again because the behavior finished
		actor.ProcessActorStateMachine();
	}
	
	// Reset the animation is done flag
	actor.SetActorFlag( ACTOR_FLAG_ANIM_DONE, false );
	actor.SetActorFlag( ACTOR_FLAG_TORSO_ANIM_DONE, false );
	
	// Change the animation if necessary
	if ( ( actor.newanimnum != -1 ) || ( actor.newTorsoAnimNum != -1 ) )
		actor.ChangeAnim();	
	
	if ( !showFailure )
		return;
	
	if ( actor.behaviorCode == BEHAVIOR_FAILED && !actor.GetActorFlag(ACTOR_FLAG_DISPLAYING_FAILURE_FX) )
	{
		Event* event;
		event = new Event( EV_DisplayEffect );
		event->AddString( "electric" );
		actor.ProcessEvent( event );
		actor.SetActorFlag( ACTOR_FLAG_DISPLAYING_FAILURE_FX , true );
		return;
	}
	else if ( actor.behaviorCode != BEHAVIOR_FAILED && actor.GetActorFlag(ACTOR_FLAG_DISPLAYING_FAILURE_FX) ) 
	{
		Event* event;
		event = new Event( EV_DisplayEffect );
		event->AddString( "noelectric" );
		actor.ProcessEvent( event );
		actor.SetActorFlag( ACTOR_FLAG_DISPLAYING_FAILURE_FX , false );
		return;
	}	
	
}

//----------------------------------------------------------------
// Name:			DoMove
// Class:			ActorThink
//
// Description:		Does Movement Stuff for Actor
//
// Parameters:		
//					Actor in question
//
// Returns:			None
//----------------------------------------------------------------
void ActorThink::DoMove( Actor &actor )
{
	if ( ( actor.flags & FL_IMMOBILE ) || ( actor.flags & FL_PARTIAL_IMMOBILE ) )
	{
		actor.animate->StopAnimating();
		return;
	}
	
	actor.movementSubsystem->CalcMove();
	actor.movementSubsystem->setLastMove( STEPMOVE_STUCK );
	
	stepmoveresult_t lastMove;
	
	if ( actor.flags & FL_SWIM )		
		lastMove = actor.movementSubsystem->WaterMove();
	else if ( actor.flags & FL_FLY )
		lastMove = actor.movementSubsystem->AirMove();
	else
		lastMove = actor.movementSubsystem->TryMove();
	
	actor.movementSubsystem->setLastMove( lastMove );
	
	if (
        ( actor.movetype != MOVETYPE_NONE )             &&
		( actor.movetype != MOVETYPE_STATIONARY )       &&
		actor.GetActorFlag( ACTOR_FLAG_TOUCH_TRIGGERS ) &&
		actor.GetActorFlag( ACTOR_FLAG_HAVE_MOVED )
		)
		G_TouchTriggers( &actor );
	
	if ( actor.groundentity && ( actor.groundentity->entity != world ) && !M_CheckBottom( &actor ) )
		actor.flags |= FL_PARTIALGROUND;
}

//----------------------------------------------------------------
// Name:			UpdateBossHealth
// Class:			ActorThink
//
// Description:		Handles Boss Specific behavior
//
// Parameters:		
//					Actor that is the boss in question
//
// Returns:			None
//----------------------------------------------------------------
void ActorThink::UpdateBossHealth( Actor &actor )
{
	char bosshealth_string[20];
	
	sprintf( bosshealth_string, "%.5f", actor.health / actor.max_boss_health );
	gi.cvar_set( "bosshealth", bosshealth_string );

	gi.cvar_set( "bossname", actor.getName() );
}

//----------------------------------------------------------------
// Name:			CheckGround
// Class:			ActorThink
//
// Description:		Checks that the actor is on the ground, and does
//					Falling Damage if necessary
//
// Parameters:		
//					Actor in question
//
// Returns:			None
//----------------------------------------------------------------
void ActorThink::CheckGround( Actor &actor )
{
	
	if ( actor.GetActorFlag( ACTOR_FLAG_HAVE_MOVED ) ||
		( actor.groundentity && actor.groundentity->entity && ( actor.groundentity->entity->entnum != ENTITYNUM_WORLD ) ) )
		actor.CheckGround();
	
	// Add Fall Damage if necessary
	if ( actor.groundentity )
	{
		if ( !actor.Immune( MOD_FALLING ) && !( actor.flags & FL_FLY ) && ( actor.origin.z + 1000.0f < actor.last_ground_z ) )
			actor.Damage( world, world, 1000.0f, actor.origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_FALLING );
		
		actor.last_ground_z = actor.origin.z;
	}
}

//----------------------------------------------------------------
// Name:			InanimateObject
// Class:			ActorThink
//
// Description:		Handles behavior ending, if the Actor is an 
//					InanimateObject
//
// Parameters:		
//					Actor in question
//
// Returns:			None
//----------------------------------------------------------------
void ActorThink::InanimateObject( Actor &actor )
{
	if ( actor.behavior && actor.behavior->Evaluate( actor ) != BEHAVIOR_EVALUATING )
	{
		actor.EndBehavior();
		
		// stop thinking
		actor.turnThinkOff();
		ActiveList.RemoveObject( &actor );
	}
	
	if ( actor.headBehavior && actor.headBehavior->Evaluate( actor ) != BEHAVIOR_EVALUATING )
	{
		actor.EndHeadBehavior();
		
		// stop thinking
		actor.turnThinkOff();
		ActiveList.RemoveObject( &actor );
	}
	
	if ( actor.eyeBehavior && actor.eyeBehavior->Evaluate( actor ) != BEHAVIOR_EVALUATING )
	{
		actor.EndEyeBehavior();
		
		// stop thinking
		actor.turnThinkOff();
		ActiveList.RemoveObject( &actor );
	}
	
	if ( actor.torsoBehavior && actor.torsoBehavior->Evaluate( actor ) != BEHAVIOR_EVALUATING )
	{
		actor.EndTorsoBehavior();
		
		// stop thinking
		actor.turnThinkOff();
		ActiveList.RemoveObject( &actor );
	}
	
}


//----------------------------------------------------------------
// Name:			TryDrown
// Class:			ActorThink
//
// Description:		Damages the Actor if they are drowning
//
// Parameters:		
//					Actor in question
//
// Returns:			None
//----------------------------------------------------------------
void ActorThink::TryDrown( Actor &actor )
{
	if ( actor.waterlevel == 3 && !( actor.flags & FL_SWIM ) )
	{
		// if out of air, start drowning
		if ( actor.air_finished < level.time )
		{
			// we may have been in a water brush when we spawned, so check our water level again to be sure
			actor.movementSubsystem->CheckWater();
			if ( actor.waterlevel < 3 )
			{
				// we're ok, so reset our air
				actor.air_finished = level.time + 5.0f;
			}
			else if ( ( actor.next_drown_time < level.time ) && ( actor.health > 0 ) )
			{
				// drown!
				actor.next_drown_time = level.time + 1.0f;
				
				//Sound( "snd_uwchoke", CHAN_VOICE );
				actor.BroadcastSound();
				
				actor.Damage( world, world, 15.0f, actor.origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_DROWN );
			}
		}
	}
	else
	{
		actor.air_finished = level.time + 5.0f;
	}
}

//----------------------------------------------------------------
// Name:			ActorStateUpdate
// Class:			ActorThink
//
// Description:		Updates miscellaneous actor state variables
//
// Parameters:		
//					Actor in question
//
// Returns:			None
//----------------------------------------------------------------
void ActorThink::ActorStateUpdate( Actor &actor )
{
	// Update move status
	if ( actor.last_origin != actor.origin )
		actor.SetActorFlag( ACTOR_FLAG_HAVE_MOVED, true );
	else
		actor.SetActorFlag( ACTOR_FLAG_HAVE_MOVED, false );
	
	// Set Origins;
	actor.last_origin = actor.origin;
	
	// Check for the ground
	if ( !( actor.flags & FL_SWIM ) && !( actor.flags & FL_FLY ) && actor.GetStickToGround() )
		CheckGround( actor );
	
	if ( !actor.deadflag )
	{
		// Check to see if stunned
		actor.CheckStun();
		
		// Handle Game Specific Stuff
		actor.gameComponent->HandleThink();
		
		// See if can talk to the player
		if(actor.DialogMode == DIALOG_MODE_ANXIOUS)
			actor.TryTalkToPlayer();
		
		// Blink -- Emotions
		if ( actor.GetActorFlag( ACTOR_FLAG_SHOULD_BLINK ) )
			actor.TryBlink();

		if ( actor.getHeadWatchAllowed() )
			actor.headWatcher->HeadWatchTarget();
		
		//Update Eyeposition
		actor.eyeposition[ 2 ] = actor.maxs[ 2 ] + actor.eyeoffset[ 2 ];
		
		// See if we should damage the actor because of waterlevel
		TryDrown( actor );
		
		if ( actor.groundentity && ( actor.groundentity->entity != world ) && !M_CheckBottom( &actor ) )
			actor.flags |= FL_PARTIALGROUND;
	}
}


//------------------------- CLASS ------------------------------
//
// Name:		DefaultThink
// Base Class:	ActorThink
//
// Description:	
//				Think class instantiated by most actors -- and 
//				instantiated in ALL actors by default
//
// Method of Use:	
//				This is the default behavior for Actor, nothing 
//				special need be done to use this class 
//
//--------------------------------------------------------------

//----------------------------------------------------------------
// Name:			Think
// Class:			DefaultThink
//
// Description:		Main Think Function for Actor
//
// Parameters:		
//					Actor in question
//
// Returns:			None
//----------------------------------------------------------------
void DefaultThink::Think( Actor &actor )
{
	if ( actor.flags & FL_IMMOBILE )
	{
		// Update boss health if necessary
		if ( (actor.GetActorFlag( ACTOR_FLAG_UPDATE_BOSS_HEALTH ) && actor.max_boss_health && ( actor.mode == ACTOR_MODE_AI )) || actor.GetActorFlag(ACTOR_FLAG_FORCE_LIFEBAR) )
			UpdateBossHealth( actor );

		if ( actor.statemap )
			{
			actor.last_time_active = level.time;
			return;
			}
	}

	// Update boss health if necessary
	if ( (actor.GetActorFlag( ACTOR_FLAG_UPDATE_BOSS_HEALTH ) && actor.max_boss_health && ( actor.mode == ACTOR_MODE_AI )) || actor.GetActorFlag(ACTOR_FLAG_FORCE_LIFEBAR) )
		UpdateBossHealth( actor );	

	if ( actor.postureController )
		actor.postureController->evaluate();

	ActorStateUpdate( actor );

	if ( !actor.deadflag )
	{
		// Update the hate list
		actor.enemyManager->Update();
		
		// Do the movement
		DoMove( actor );
		
		if ( actor.actortype == IS_INANIMATE )
		{
			InanimateObject( actor );			
			return;
		}
		
		//Process our behaviors
		ProcessBehaviors( actor );		
	}

}


//------------------------- CLASS ------------------------------
//
// Name:		SimplifiedThink
// Base Class:	ActorThink
//
// Description:	
//				This is a light weight version of Default Think
//				The purpose of this class is to allow Actors that
//				move, have behaviors and accept messages without
//				the CPU overhead of doing DefaultThink
//
// Method of Use:	
//				Actors can be given this method of updating via
//				the script command "setsimplifiedthink"
//
//--------------------------------------------------------------
SimplifiedThink::SimplifiedThink( Actor *actor ) : 
	_actor( actor ), 
	_previousContents( actor->getContents() )
{
	actor->setContents( 0 );
}

SimplifiedThink::~SimplifiedThink( void )
{
	_actor->setContents( _previousContents );
}

//----------------------------------------------------------------
// Name:			DoMove
// Class:			SimplifiedThink
//
// Description:		Does Movement Stuff for Actor
//
// Parameters:		
//					Actor in question
//
// Returns:			None
//----------------------------------------------------------------
void SimplifiedThink::DoMove( Actor &actor )
{
	actor.last_origin = actor.origin;
	actor.movementSubsystem->CalcMove();
	actor.movementSubsystem->setLastMove( actor.movementSubsystem->SimpleMove( actor.GetStickToGround() ) );
}

void SimplifiedThink::DoArchive( Archiver &arc )
{
	ActorThink::DoArchive( arc );
	arc.ArchiveInteger ( &_previousContents );
	arc.ArchiveSafePointer( &_actor );
}

//----------------------------------------------------------------
// Name:			Think
// Class:			SimplifiedThink
//
// Description:		Main Think Function for Actor
//
// Parameters:		
//					Actor in question
//
// Returns:			None
//----------------------------------------------------------------
void SimplifiedThink::Think( Actor &actor )
{
	// Update the hate list
	actor.enemyManager->TrivialUpdate();

	if ( actor.GetActorFlag( ACTOR_FLAG_SHOULD_BLINK ) )
		actor.TryBlink();
	
	//Process our behaviors
	ProcessBehaviors( actor );
	
	// Do the movement
	DoMove( actor );
}
