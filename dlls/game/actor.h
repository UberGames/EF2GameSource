//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actor.h                                        $
// $Revision:: 280                                                            $
//   $Author:: Sketcher                                                       $
//     $Date:: 5/04/03 5:49p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Base class for character AI.
//

//==============================
// Forward Declarations
//==============================
class Actor;
class FindCoverMovement;
class FindMovement;
class FindFleeMovement;
class FindEnemyMovement;

#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "g_local.h"
#include "weapon.h"
#include "sentient.h"
#include "container.h"
#include "stack.h"
#include "navigate.h"
#include "behavior.h"
#include "behaviors_general.h"
#include "behaviors_specific.h"
#include "scriptmaster.h"
#include "characterstate.h"
#include "actorstrategies.h"
#include "actorgamecomponents.h"
#include "helper_node.h"
#include "actorutil.h"
#include "actor_sensoryperception.h"
#include "actor_enemymanager.h"
#include "actor_locomotion.h"
#include "actor_combatsubsystem.h"
#include "actor_headwatcher.h"
#include "actor_posturecontroller.hpp"
#include "actorincludes.h"
#include "RageAI.h"
#include "FollowPath.h"

//-------------------------------------------
// Global Functions
//-------------------------------------------
Actor *GetActor( const str &actor_name );
Player *GetPlayer(int index);
void AI_SenseEnemies( void );
void AI_DisplayInfo( void );


//-------------------------------------------
// Safe Pointer 
//-------------------------------------------
typedef SafePtr<Actor> ActorPtr;


//-------------------------------------------
// External Data
//-------------------------------------------
extern Event EV_Torso_Anim_Done;
extern Event EV_EntityFlags;
extern Event EV_Sentient_GroupMemberInjured;
extern Event EV_HelperNodeCommand;
extern Event EV_Actor_SetAnim ;
extern Event EV_Actor_Blink;


enum SteeringDirectionPreference
{
	STEER_RIGHT_ALWAYS,
	STEER_LEFT_ALWAYS,
	STEER_RANDOMLY,
	STEER_BEST,
};


//------------------------- CLASS ------------------------------
//
// Name:			Actor
// Base Class:		Sentient
//
// Description:		Defines an Actor
//
// Method of Use:	Spawned into the world
//--------------------------------------------------------------
class Actor : public Sentient
{
	public:

		// E3 2002 Hack Stuff
		EntityPtr forcedEnemy;

		typedef enum {
			ACTOR_CONTROL_NONE,
			ACTOR_CONTROL_AUTO_RELEASE,
			ACTOR_CONTROL_SHARED,
			ACTOR_CONTROL_LOCKED,
		} ActorControlType ;
		
		//-----------------------------------------------------
		// Animation Data:		
		// The first chunk is for "leg" animation, the second
		// chunck is for "torso" animations.  Currently, I know
		// of no way to "blend" animations, and you need to be
		// careful of which animations you play together.
		// Also, I believe there are still some issues with playing
		// simultaneous animations
		//
		// Note:  To play 2 animations simultaneously, the actors
		// tiki file _MUST_ have leg bones marked as LEGBONE instead
		// of just BONE.
		//-----------------------------------------------------
		str								 newanim;
		int								 newanimnum;
		int                              animnum;
		str								 animname;
		Event							*newanimevent;
		str								 last_anim_event_name;

		str								 newTorsoAnim;
		int								 newTorsoAnimNum;
		str								 TorsoAnimName;
		Event							*newTorsoAnimEvent;		
		str								 last_torso_anim_event_name;

		//-----------------------------------------------------
		// Range Data
		// This was original placed here so that level designers
		// could have a way to control the "leash" so to speak
		// on the distance actors followed other actors.  It turned
		// out this didn't work as well as I had wanted.  I'm leaving
		// it here for right now as I don't want to break any type
		// of legacy.  However, we should keep an eye on this to 
		// see if can be removed.
		//-----------------------------------------------------
		float						absoluteMin;
		float						absoluteMax;
		float						preferredMin;
		float						preferredMax;
		
		//-----------------------------------------------------
		// Actor and Target Types
		// the actortype is the primary determinant of who the
		// actor will "like" and "hate".  I think this system
		// is kind of restricting and should be replaced with a
		// "real" faction system in the future.
		//
		// targetType determines what the actor will attempt to
		// attack, i.e. other actors only, players only, or 
		// "interaction" objects only.  I think this needs to
		// be revamped as well.
		//
		// I'm not sure what validTarget is for.  It is likely
		// that this can be removed, but I'm not going to do it
		// right now for fear of breaking stuff
		//-----------------------------------------------------
		actortype_t					actortype;
		targetType_t				targetType;
		qboolean					validTarget;
		
		//-----------------------------------------------------
		// Behavior Data
		// We have the ability to play multiple behaviors simultaneously
		// however, I don't recommend it.  In all honestly headBehavior and
		// eyeBehavior really need to be removed.  I guess I could see
		// keeping a torsoBehavior as we do have leg and torso animation
		// sets.  However, I'm not convinced as of yet, that all the bugs
		// are worked out of using multiple behaviors ( especially when
		// relying on Anim Done or other such conditionals
		//--------------------------------------------------------
		BehaviorPtr						behavior;
		str								currentBehavior;
		str								behaviorFailureReason;
		BehaviorReturnCode_t			behaviorCode;

		BehaviorPtr						headBehavior;
		str								currentHeadBehavior;
		BehaviorReturnCode_t			headBehaviorCode;

		BehaviorPtr						eyeBehavior;
		str								currentEyeBehavior;
		BehaviorReturnCode_t			eyeBehaviorCode;

		BehaviorPtr						torsoBehavior;
		str								currentTorsoBehavior;
		BehaviorReturnCode_t			torsoBehaviorCode;

		//---------------------------------------------------------
		// Lead Factors
		//	These define a range - [minLeadFactor, maxLeadFactor] -
		//	which is used to determine how much - if any - the
		//	actor will lead its target when using a projectile
		//	weapon.  A factor of 0 indicates the position where
		//	the target is presently; a factor of 1 indicates the
		//	position where the target is predicted to be at
		//	impact, taking into account the target's current
		//	motion.  Each time the actor fires a projectile,
		//	a random number in this range is chosen and applied
		//	as the lead factor for that shot.  Thus, [0,0] means
		//	the actor never leads his target; [1,1] means he
		//	always leads it perfectly; [0,2] means he sometimes
		//	over-leads and sometimes under-leads it.
		//---------------------------------------------------------
		float							minLeadFactor;
		float							maxLeadFactor;
		
		//---------------------------------------------------------
		// ThrowObject Flag
		// This is here in case the actor has an object deemed as
		// "throwable".  This is so so weak.  I can say that, because
		// I'm the one who put it in here.  The whole "throw" object
		// stuff needs to be jetisoned and placed in a behavior
		// where it belongs. At a minimum, if we need to maintain
		// that we "have" a throw object, it should be made an bit
		// in the Actor Flag stuff not its own bool
		//----------------------------------------------------------
		qboolean                   haveThrowObject;

		//-----------------------------------------------------------
		// AnimSet -- Animation Sets can be setup in the GPD.  What
		// animset the actor is currently using is set up in script
		//-----------------------------------------------------------
		str							animset;

		//----------------------------------------------------------
		// Flags
		// Stores various bits of information about the actor.
		// We do have a bit vector class now, and sometime someone
		// should experiment with converting these flags into a 
		// bit vector
		//---------------------------------------------------------
		unsigned int				actor_flags1;
		unsigned int				actor_flags2;
		unsigned int				actor_flags3;	
		unsigned int				actor_flags4;	
		unsigned int				notify_flags1;
		unsigned int				state_flags;


		//--------------------------------------------------------
		// Some Dialog and Babble Stuff
		// Okay, now that we have context dialog stuff in addition
		// to this, the dialog stuff is SCREAMING for it's own class
		// even if we do it as a helper class, it needs something
		// it really needs to be moved
		//---------------------------------------------------------
		float						 chattime;
		float						 nextsoundtime;
		DialogMode_t				 DialogMode;
		float						 radiusDialogRange;
		DialogNode_t				*dialog_list;
		float						 dialog_done_time;
		str							 dialog_state_name;
		str							 dialog_old_state_name;		
		bool						 _ignoreNextContext;
		str							 _nextContextToIgnore;
		float						 _nextContextTime;
		float						 _contextInterval;
		
		//---------------------------------------------------------
		// Thread Management
		// Here we need to take a long look at what we're doing with 
		// these.  To me it seems ridiculous that we should maintain
		// all these str when they are seldom used.  What I think
		// we need is a type of thread controller that keeps a container
		// of strings for threads we're using and an enum for type
		// then we'd only add a string to the container if it's used
		// it would also allow us to have many many more types of these
		// threads since they would only be stored "as required"
		//----------------------------------------------------------
		ThreadPtr					scriptthread;		
		str							kill_thread;
		str							escape_thread;
		str							captured_thread;
		str							activate_thread;
		str							onuse_thread_name;
		str							ondamage_thread;
		str							alert_thread;
		str							idle_thread;
		Container<threadlist_t *>	threadList;
		
		//----------------------------------------------------------
		// Pain and Miscellaneous Damage
		// I honestly don't know what some of these variables
		// are for.  We need to look at how we are handling "pain"
		// for actors and see if there's a better way to do it,
		// and verify that we need all of this stuff
		//----------------------------------------------------------
		float						pain_threshold;
		float						next_drown_time;
		float						air_finished;
		int							pain_type;
		Vector						pain_angles;
		int							bullet_hits;
		float						next_pain_time;
		float						min_pain_time;
		float						next_forced_pain_time;
		float						max_pain_time;
		str							_deathEffect;
				
		//----------------------------------------------------------
		// State map and "Thinking" stuff
		// Here's where we store all the stuff for the Actors state
		// map.  One thing to note is stateDebugType_t.  This is a
		// very very primitive debugging tool that will print
		// various information about state transitions to the console
		// for a specified actor.
		// We really really need better debuging tools for actors		
		//----------------------------------------------------------
		
		str							 masterstatemap_name;		
		StateMap					*masterstatemap;
		State						*currentMasterState;
		State						*lastMasterState;

		str							 statemap_name;
		StateMap					*statemap;
		State						*currentState;
		State						*lastState;

		State						*globalState;
		
		float						 state_time;
		float						 masterstate_time;
		int							 times_done;
		int							 masterstate_times_done;
		float						 state_done_time;
		float						 masterstate_done_time;
		float						 last_time_active;
		stateDebugType_t			 showStates;
		talkModeStates_t			 talkMode;
		bool						 useConvAnims;		


		static Condition<Actor>		 Conditions[];
		Container<Conditional *>	 conditionals;	
		Container<Conditional *>	 master_conditionals;

		
		//-----------------------------------------------------------
		// Fuzzy Engine Stuff
		// Here's where we store our fuzzy engine stuff.  The fuzzy
		// engine is sort of a state machine for state machines.  However
		// instead of direct transitions, each conditional in the
		// fuzzy engine is assigned a point value.  All the fuzzyvars
		// are evaluated and the one with the highest cumulative score
		// executes
		//------------------------------------------------------------
		str							 fuzzyengine_name;
		FuzzyEngine					*fuzzyEngine;
		qboolean					 fuzzyEngine_active;
		Container<Conditional *>	 fuzzy_conditionals;

		//------------------------------------------------------------
		// Eye Angle stuff
		// This is in here for when we were actually moving the eyes
		// with bone controllers.  I don't think its being used any more
		// nor do I think it should ever be used again.  I believe that
		// this "eye watch" thing is cool in theory, but would hardly ever
		// be seen by the player.  It also introduces a whole lot of problems
		// "chaining" watches together properly.  Basically, for this level
		// of detail to really be effective.  During a watch, the eyes would 
		// have to move, then the head, then the torso, then the legs.  
		// Currently we don't have a way to accomplish that chain easily
		// However, something to handle that might be good to look into 
		// for the future
		//--------------------------------------------------------------
		float                      maxEyeYawAngle;
		float                      minEyeYawAngle;
		float                      maxEyePitchAngle;
		float                      minEyePitchAngle;
		
		//--------------------------------------------------------------
		// Saved Items (from switching to talk mode and stuff)
		// This saved stuff presents some interesting possiblities for 
		// the future, however, for right now, it is used when the 
		// actor steps out of his normal "behavior" into a dialog mode
		// to talk, when the dialog is completed, they step back into
		// the behaivors they saved off here
		//---------------------------------------------------------------
		int								saved_mode;
		BehaviorPtr						saved_behavior;
		BehaviorPtr						saved_headBehavior;
		BehaviorPtr						saved_eyeBehavior;
		BehaviorPtr						saved_torsoBehavior;
		ThreadPtr						saved_scriptthread;
		ThreadPtr						saved_actorthread;
		str								saved_anim_name;
		str								saved_state_name;
		str								saved_anim_event_name;

		
		//---------------------------------------------------------------
		// Part Stuff
		// I'm not entirely sure what this is, or how to use it.  I 
		// remember being told that we could seperate actors into multiple
		// "parts" each executing its own state machines, but I'm not
		// sure how to go about setting such a thing up
		//----------------------------------------------------------------
		str								part_name;
		Container<part_t>				parts;
		

		//----------------------------------------------------------------
		// Incoming Attack Stuff
		// Here to let us handle incoming attacks -- Could probably
		// be improved
		//----------------------------------------------------------------
		EntityPtr							incoming_proj;
		float								incoming_time;
		qboolean							incoming_bullet;

		//-----------------------------------------------------------------
		// Basic Data about the Actor
		// All this should probably be made private and have accessors
		// We also need to take some time and verify we're using all of 
		// this stuff
		//-----------------------------------------------------------------
		str								name;
		float							max_inactive_time;
		Vector							eyeoffset;
		float							last_jump_time;
		str								enemytype;
		float							actorrange_time;
		float							last_height;
		EntityPtr						last_ent;
		float							canseeenemy_time;
		float							canseeplayer_time;
		int								stage;
		int								num_of_spawns;
		ActorPtr						spawnparent;
		Vector							last_attack_pos;
		Vector							last_attack_enemy_pos;
		EntityPtr						last_attack_entity_hit;
		Vector							last_attack_entity_hit_pos;
		int								mode;
		Vector							last_known_enemy_pos;
		Vector							last_known_player_pos;
		float							feet_width;
		Vector							last_origin;
		float							next_find_enemy_time;
		float							minimum_melee_height;
		float							damage_angles;
		float							real_head_pitch;
		float							next_pain_sound_time;
		float							last_ground_z;
		str								emotion;
		float							next_blink_time;
		float							actor_to_actor_damage_modifier;		
		float							last_used_time;
		float							hitscan_response_chance;
		int								shotsFired;
		int								ondamage_threshold;
		float							timeBetweenSleepChecks;
		int								saved_bone_hit;
		float							activationDelay;
		float							activationStart;
		float							deathKnockbackVerticalValue;
		float							deathKnockbackHorizontalValue;

		//-----------------------------------------------------
		// Pain Sound Stuff
		//-----------------------------------------------------
		float							_nextPlayPainSoundTime;	
		float							_playPainSoundInterval;


		//-----------------------------------------------------
		// Controller stuff
		// If set, this is the listener controlling the actor.
		// This controller is notifyied whenever the actor ends
		// a behavior.
		//-----------------------------------------------------
		ListenerPtr						_controller ;
		ActorControlType				_controlType ;
		
		
		//-----------------------------------------------------
		// Group Number
		// Right now, groups are assigned in script or tiki
		// and are not used very much yet.  I will be implementing
		// a group data controller very soon which should handle
		// all the book keeping for group stuff, and allow
		// actors to ask questions about their group
		//------------------------------------------------------
		//int								groupnumber;


		//------------------------------------------------------
		// Current Helper Node
		// Data structure that holds information about the Actor's
		// "current" helper node.
		//-------------------------------------------------------
		CurrentHelperNodeData_t			currentHelperNode;
		IgnoreHelperNodeData_t			ignoreHelperNode;

		//--------------------------------------------------------
		// Follow Target
		// This data structure maintains who the actor would
		// currently follow if it decided to execute a follow behavior
		// it also maintains who it is currently following if it
		// is following someone else who is currently following the same
		// target.
		//--------------------------------------------------------
		FollowTargetData_t				followTarget;
		int								_steeringDirectionPreference;



		//-----------------------------------------------------
		// General Stuff (stuff I can't think of a group for)
		// Like other stuff, we need to take a long look
		// at these things
		//-----------------------------------------------------
		Container<StateVar *>			stateVarList;
		EntityPtr						trigger;				
		str								command;		
		str								idle_state_name;
		str								master_idle_state_name;
		str								global_state_name;
		float							next_player_near;		
		EntityPtr						pickup_ent;
		float							stunned_end_time;
		Container<str>					spawn_items;
		float							spawn_chance;
		str								bounce_off_effect;
		float							bounce_off_velocity;
		Container<int>					can_be_finsihed_by_mods;		
		float							max_boss_health;
		qboolean						haveAttached;
		float							currentSplineTime;
		float							_dialogMorphMult;
		weaponhand_t					_useWeaponDamage;
		float							_nextCheckForWorkNodeTime;
		float							_nextCheckForHibernateNodeTime;
		float							_nextCheckForEnemyPath;
		bool							_havePathToEnemy;		
		float							_nextPathDistanceToFollowTargetCheck;
		bool							_checkedChance;
		bool							_levelAIOff;

		//------------------------------------------------------
		// Helper Classes
		// These are classes that help the actor function
		// We need a couple more of these, specifically one 
		// for dialog, and another for "thread" management
		//------------------------------------------------------
		ActorThink						*thinkStrategy;
		ActorGameComponent				*gameComponent;
		SensoryPerception				*sensoryPerception;
		Strategos						*strategos;
		EnemyManager					*enemyManager;
		PackageManager					*packageManager;
		MovementSubsystem				*movementSubsystem;
		Personality						*personality;
		CombatSubsystem					*combatSubsystem;
		HeadWatcher						*headWatcher;
		PostureController				*postureController;
		
		//------------------------------------------------------
		// 1st Playable Hack
		// Obviously, this needs to be removed as soon as possible
		//-------------------------------------------------------
		float                      lastPathCheck_Work;
		float                      lastPathCheck_Flee;
		float                      lastPathCheck_Patrol;
		qboolean                   testing;

		str							_branchDialogName;		

		//------------------------------------------------------
		// Class Prototype
		//------------------------------------------------------
		CLASS_PROTOTYPE( Actor );


		//-------------------------------------------------------
		// Initialization functions
		//-------------------------------------------------------
											 Actor();
											~Actor();
		void								 Start( Event *ev );
		void								 Sleep( void );
		void								 Sleep( Event *ev );
		void								 Wakeup( void );
		void								 Wakeup( Event *ev );
		void								 InitGameComponent( void );
		void								 InitThinkStrategy( void );
		void								 InitSensoryPerception( void );
		void								 InitStrategos( void );
		void								 InitEnemyManager( void );
		void								 InitPackageManager( void );
		void								 InitMovementSubsystem( void );
		void								 InitPersonality( void );
		void								 InitCombatSubsystem( void );
		void								 InitHeadWatcher( void );
		void								 InitPostureController( void );

		//---------------------------------------------------------
		// Event Interface for Accessor Functions
		//---------------------------------------------------------
		void								SetTargetType ( Event *ev );	
		void								SetDamageAngles( Event *ev );
		void								SetImmortal( Event *ev );
		void								SetTakeDamage( Event *ev );
		void								SetEnemyAttached ( Event *ev );
		void								SetEnemyType( Event *ev );
		void								SetOnUseThread( Event *ev );
		void								SetRadiusDialogRange( Event *ev );
		void								SetDialogMode( Event *ev );		
		void								SetActivateThread( Event *ev );
		void								SetAlertThread( Event *ev );
		void								SetValidTarget( Event *ev );
		void								SetTurretMode( Event *ev );
		void								SetOnDamageThread( Event *ev );
		void								SetTimeBetweenSleepChecks( Event *ev );
		void								SetDieCompletely( Event *ev );
		void								SetBleedAfterDeath( Event *ev );
		void								SetPainThresholdEvent( Event *ev );
		void								SetKillThreadEvent( Event *ev );
		void								SetDeathSize( Event *ev );
		void								SetCanWalkOnOthers( Event *ev );
		void								SetFeetWidth( Event *ev );
		void								SetCanBeFinishedBy( Event *ev );
		void								SetIdleThread( Event *ev );
		void								SetMaxBossHealth( Event *ev );
		void								SetTargetable( Event *ev );
		void								SetSpawnChance( Event *ev );
		void								SetNotAllowedToKill( Event *ev );
		void								SetUseGravity( Event *ev );
		void								SetAllowFall( Event *ev );
		void								SetUseMovement( Event *ev );
		void								SetHaveThing( Event *ev );
		void								SetHaveThing( int thing_number,qboolean thing_bool );
		void								SetBounceOff( Event *ev );
		void								SetBounceOffEffect( Event *ev );
		void								SetWatchOffset( Event *ev );
		void								SetMaxInactiveTime( Event *ev );
		void								SetTurnSpeed( Event *ev );
		void								SetHealth( Event *ev );
		void								SetMaxHealth( Event *ev );
		void								SetVar( Event *ev );
		void								SetVarTime( Event *ev );
		void								SetMask( Event *ev );
		void								SetWeaponReady( Event *ev );
		void								SetSimplifiedThink( Event *ev );
		void								SetActorToActorDamageModifier( Event *ev );
		void								SetEmotion( Event *ev );
		void								SetEyeAngles( Event *ev );
		void								SetHitscanResponse( Event *ev );		
		void								SetFOV( Event *ev );
		void								SetVisionDistance( Event *ev );
		void								SetAimLeadFactors( Event *ev );
		void								SetActorType( Event *ev );
		void								SetAbsoluteMax( Event *ev );
		void								SetAbsoluteMin( Event *ev );
		void								SetPreferredMax( Event *ev );
		void								SetPreferredMin( Event *ev );
		void								SetDisabled( Event *ev );
		void								SetCrippled( Event *ev );
		void								SetInAlcove( Event *ev );
		void								SetGroupNumber( Event *ev );
		void								SetMovementMode( Event *ev );
		void								SetHeadWatchTarget( Event *ev );      
		void								SetHeadWatchMaxDistance( Event *ev );
		void								SetHeadWatchSpeed( Event *ev );
		void								setHeadTwitch( Event *ev );
		void								SetFuzzyEngineActive( Event *ev ); 
		void								SetNodeID( Event *ev );
		void								SetFollowTarget( Event *ev );
		void								SetFollowRange( Event *ev );
		void								SetFollowRangeMin( Event *ev );
		void								SetFollowCombatRange( Event *ev );
		void								SetFollowCombatRangeMin( Event *ev );
		void								SetSteeringDirectionPreference( Event *ev );
		void								SetIgnoreNextContext( Event *ev );		
		void								SetGroupDeathThread( Event *ev );
		void								SaveOffLastHitBone( Event *ev );
		void								SetMinPainTime( Event *ev );
		void								SetMinPainTime( float time );
		void								SetBounceOffVelocity( Event *ev );
		
		void								SetSelfDetonateModel ( Event *ev );

		void								SetCombatTraceInterval( Event *ev );

		void								SetHeadWatchTarget( Entity *ent );
		void								SetHeadWatchSpeed( float speed );
		void								ResetTorso( Event *ev );
		void								EyeOffset( Event *ev );
		void								ForceSetClip( Event *ev );		
		void								GroupMemberInjured( Event *ev );
		void								StrictlyFollowPath( Event *ev );
		void								SetIgnoreWatchTarget( bool ignore );

		void								HelperNodeCommand( Event *ev );

		void								SetPlayPainSoundInterval( Event *ev );

		void								SetBehaviorPackage( Event *ev );
		void								SetBehaviorPackage( const str &packageName );

		void								SetMaxHeadYaw( Event *ev );
		void								SetMaxHeadPitch( Event *ev );

		void								SetPlayerHateModifier( Event *ev );

		void								UseBehaviorPackage( Event *ev );
		void								UseBehaviorPackage( const str &packageName );

		void								ChildUseBehaviorPackage( Event *ev );
		void								ChildUseBehaviorPackage( const str &childName , const str &packageName );

		void								ChildSetAnim( Event *ev );
		void								ChildSetAnim( const str &childName , const str &animName );

		void								ChildSuicide( Event *ev );

		void								GroupAttack( Event *ev );
		void								GroupActorType( Event *ev );

		void								SetMasterState( Event *ev );

		void								PrintDebugMessage( Event *ev );

		void								SelectNextEnemy( Event* ev );
		void								SelectClosestEnemy ( Event *ev );
		void								SetAnimSet( Event *ev );
		void								SetAnimSet( const str &animSet );
		const str&							GetAnimSet();

		void								SetTalkWatchMode( Event *ev );
		void								SetPostureState( Event *ev );		
		/*virtual*/void						processGameplayData( Event *ev );

		void								UnreserveCurrentHelperNode( Event *ev );
		void								UnreserveCurrentHelperNode();
		
		//-----------------------------------------------------------------
		// Note:  This Needs to be its own class
		//-----------------------------------------------------------------
		void								AddCustomThread( Event *ev );
		void								AddCustomThread( const str& threadType , const str& threadName );
		bool								HaveCustomThread( const str& threadType );
		void								RunCustomThread( const str& threadType );
		const str							GetCustomThread( const str& threadType );								

		//----------------------------------------------------------
		// Personality Tendencies
		//----------------------------------------------------------
		void								SetAggressiveness( Event *ev );
		void								SetTalkiness( Event *ev );
		void								SetTendency( Event *ev );

		void								RegisterBehaviorPackage( Event *ev );
		void								UnRegisterBehaviorPackage( Event *ev );
		void								SetPackageTendency( Event *ev );

		//-----------------------------------------------------------
		// Clear Anim Functions
		//-----------------------------------------------------------
		void								ClearTorsoAnim();
		void								ClearTorsoAnim( Event *ev );

		void								ClearLegAnim();

		//-----------------------------------------------------------
		// Debug Functions
		//-----------------------------------------------------------
		void								DebugStates( Event *ev );
		void								ShowInfo( void );
		void								WhatAreYouDoing( Event *ev );
		void								WhatsWrong( Event *ev );
		void								PrintMasterStateInfo();
		void								PrintBehaviorPackageInfo();
		void								PrintStateMapInfo();


		//-----------------------------------------------------------
		// Set Stimuli Response
		//-----------------------------------------------------------
		void								RespondTo( Event *ev );
		void								PermanentlyRespondTo( Event *ev );

		//-----------------------------------------------------------
		// Combat functions
		//-----------------------------------------------------------
		void								MeleeEvent( Event *ev );
		void								ChargeWater( Event *ev );
		void								DamageOnceStart( Event *ev );
		void								DamageOnceStop( Event *ev );
		void								DamageAllowed( Event *ev );
		virtual qboolean					CanAttackFrom( const Vector &pos, const Entity *ent, qboolean usecurrentangles );
		virtual qboolean					CanAttack( Entity *ent, qboolean usecurrentangles );
		void								FireProjectile( Event *ev );
		void								FireBullet( Event *ev );
		void								FireRadiusAttack( Event *ev );
		void								SaveAttack( const Vector &orig, const Vector &dir );
		qboolean							TestAttack( const str &tag_name );
		void								IncomingProjectile( Event *ev );
		void								ProjectileClose( Event *ev );
		qboolean							EntityHasFireType( Entity *ent, firetype_t fire_type	);
		void								DamageEnemy( Event *ev );
		void								DamageSelf( Event *ev );
		void								TurnTowardsEnemy( Event *ev );
		void								TurnTowardsPlayer( Event *ev );				
		void								TurnTowardsEntity( Event *ev );
		void								SetMinimumMeleeHeight( Event *ev );

		qboolean							IsImmortal( void );
		qboolean							TakeDamage( void );

		void								FireWeapon( Event *ev );
		void								StopFireWeapon( Event *ev );		
		void								ClearArmorAdaptions( Event *ev );

		//-----------------------------------------------------------------
		// Enemy management
		//-----------------------------------------------------------------
		void								ClearCurrentEnemy( Event *ev ); 
			
		//----------------------------------------------------------------
		// Targeting functions
		//----------------------------------------------------------------
		qboolean							CloseToEnemy( const Vector &pos, float howclose );
		qboolean							EntityInRange( Entity *ent, float range, float min_height, float max_height , bool XYOnly = false );
		

		
		void								ClearOnUseThread( Event *ev );
		void								AttachCurrentEnemy( Event *ev );
		void								AttachActor( Event *ev );
		void								PickupThrowObject( Event *ev );
		void								TossThrowObject( Event *ev );

		//---------------------------------------------------------------
		// State machine functions
		//--------------------------------------------------------------
		void								SetIdleStateName( Event *ev	);
		void								SetState( const char *state_name );
		void								resetStateMachine( void );
		void								SetMasterState( const str &state_name );
		void								SetGlobalState( const char *state_name );
		void								InitState( void );
		void								InitMasterState( void );
		void								LoadStateMap( Event *ev	);
		void								LoadMasterStateMap( Event *ev );
		void								ProcessActorStateMachine( void );
		void								ProcessMasterStateMachine( void );
		void								LoadFuzzyEngine( Event *ev );

		void								LoadPostureStateMachine( Event *ev );
		
		//--------------------------------------------------------------
		// Thread management	
		//--------------------------------------------------------------
		void								RunThread( Event *ev );
		void								RunThread( const str &thread_name );
		void								SetThread( const str &filename, const str &label );		
		void								RunDamageThread( void );
		void								RunAlertThread( Event *ev );		

		//--------------------------------------------------------------
		// Behavior management
		//--------------------------------------------------------------
		void								SendMoveDone( CThread *script_thread );
		void								EndBehavior( void );
		void								EndHeadBehavior( void );
		void								EndEyeBehavior( void );
		void								EndTorsoBehavior( void );
		void								SetBehavior( Behavior *newbehavior, Event *argevent = NULL, CThread *thread = NULL );
		void								SetHeadBehavior( Behavior *newbehavior, Event *argevent = NULL, CThread *thread = NULL );
		void								SetEyeBehavior( Behavior *newbehavior, Event *argevent = NULL, CThread *thread = NULL );
		void								SetTorsoBehavior( Behavior *newbehavior, Event *argevent = NULL, CThread *thread = NULL );
		void								EndBehaviorEvent( Event *ev );
		void								EndHeadBehaviorEvent( Event *ev );
		void								EndEyeBehaviorEvent( Event *ev );
		void								EndTorsoBehaviorEvent( Event *ev );
		void								NotifyBehavior( Event *ev );
		void								NotifyHeadBehavior( Event *ev );
		void								NotifyEyeBehavior( Event *ev );
		void								NotifyTorsoBehavior( Event *ev );
		void								EndAllBehaviors( void );


		//-----------------------------------------------------------------
		// Path and node management
		//-----------------------------------------------------------------
		PathNode							*NearestNodeInPVS( const Vector &pos );
		void								SetPath( Path *newpath );
		void								ReserveNodeEvent( Event *ev );
		void								ReleaseNodeEvent( Event *ev );
		trace_t								Trace(const Vector &end, const char *reason) const;
		trace_t								Trace(const float distance, const char *reason) const;
		trace_t								Trace(const float angle, const float distance, const char *reason) const;
		trace_t								Trace(const Vector &begin, const Vector &end, const char *reason) const;
		trace_t								Trace(const Vector &begin, const Vector &end, const int contentMask, const char *reason ) const;


		//-----------------------------------------------------------------
		// Animation control functions
		//-----------------------------------------------------------------
		void								RemoveAnimDoneEvent( void );
		void								ChangeAnim( void );
		qboolean							SetAnim( const str &anim, Event *ev = NULL, bodypart_t part = legs, const float animationRate = 1.0f );
		qboolean							SetAnim( const str &anim, Event &ev, bodypart_t part = legs, const float animationRate = 1.0f );
		void								SetAnim( Event *ev );
		void								AnimDone( Event *ev );
		void								TorsoAnimDone( Event *ev );
		void								SetCinematicAnim( const str &anim );
		void								CinematicAnimDone( );
		void								PostureAnimDone( Event *ev );
	
		//-----------------------------------------------------------------
		// Script commands
		//-----------------------------------------------------------------
		void								GoIdle( Event *ev );
		void								LookAt( Event *ev );
		void								TurnToEvent( Event *ev );
		void								HeadAndEyeWatchEvent ( Event *ev );
		void								HeadWatchEvent( Event *ev );		
		void								ResetHeadEvent( Event *ev );
		void								EyeWatchEvent( Event *ev );
		void								ResetEyeEvent( Event *ev );
		void								ResetTorsoEvent( Event *ev );
		void								FallToDeathEvent( Event *ev );
		void								WalkTo( Event *ev );		
		void								BlindlyFollowPath ( Event *ev );
		void								WalkWatch( Event *ev );	
		void								WarpTo( Event *ev );
		void								PickupEnt( Event *ev );
		void								ThrowEnt( Event *ev );
		void								AttackEntity( Event *ev );
		void								AttackPlayer( Event *ev );
		void								JumpToEvent( Event *ev );      
		void								Anim( Event *ev );
		void								RepostEvent( Event *ev, const Event &event_type );
		void								FollowWayPoints( Event *ev );


		//-----------------------------------------------------------------
		// Weapon Stuff
		//-----------------------------------------------------------------
		void								GiveActorWeapon( Event *ev );
		void								RemoveActorWeapon( Event *ev );		
		void								UseActorWeapon( Event *ev );
		void								PutawayWeapon( Event *ev );
		void								UseWeaponDamage( Event *ev );
	
		void								AttachModelToTag( const str &modelName , const str &tagName );
		void								DetachModelFromTag( const str &tagName );


		//-----------------------------------------------------------------
		// Script conditionals
		//-----------------------------------------------------------------
		void								IfEnemyVisibleEvent( Event *ev );
		void								IfNearEvent( Event *ev );
		void								IfCanHideAtEvent( Event *ev );
		void								IfEnemyWithinEvent( Event *ev );


		//-----------------------------------------------------------------
		// Sound reaction functions
		//-----------------------------------------------------------------
		void								NoPainSounds( Event *ev );
		void								HeardSound( Event *ev );
		void								BroadcastAlert( float pos = SOUND_RADIUS );
		void								BroadcastAlert( Event *ev );
		void								BroadcastAlert( float pos, int soundtype );
		

		//-----------------------------------------------------------------
		// Pain and death related functions
		//-----------------------------------------------------------------
		void								Pain( Event *ev );
		void								StunEvent( Event *ev );
		void								CheckStun( void );
		void								Dead( Event *ev );
		void								Killed( Event *ev );
		void								KilledEffects( Entity *attacker );
		void								RemoveUselessBody( Event *ev );
		void								DeathFadeEvent( Event *ev );
		void								setDeathEffect( Event *ev );
		void								DeathShrinkEvent( Event *ev );
		void								DeathSinkEvent( Event *ev );
		void								StaySolidEvent( Event *ev );
		void								SpawnGib( Event *ev );
		void								SpawnGibAtTag( Event *ev );
		void								RealSpawnGib( qboolean use_tag, Event *ev );
		void								SpawnNamedGib( Event *ev );
		float								SpawnGetTime( float vel, const Vector &orig, const Vector &gib_mins, const Vector &gib_maxs );
		void								SpawnBlood( Event *ev );
		void								Suicide( Event *ev );		
		void								FadeEvent( Event *ev );
		qboolean							RespondToHitscan( void );		
		bool								canBeDamagedBy(meansOfDeath_t MeansOfDeath );

		//-----------------------------------------------------------------
		// Movement functions
		//-----------------------------------------------------------------
		void								SimplePathfinding( Event *ev );
		void								ForwardSpeedEvent( Event *ev );
		void								SwimEvent( Event *ev );
		void								FlyEvent( Event *ev );
		void								NotLandEvent( Event *ev );
		void								Push( Event *ev );
		void								Push( const Vector &dir );
		void								Pushable( Event *ev );

		//-----------------------------------------------------------------
		// AI Functions
		//-----------------------------------------------------------------
		void								ActivateAI( void );
		void								TurnAIOn( Event *ev );
		void								TurnAIOn( void );
		void								TurnAIOff( Event *ev );
		void								TurnAIOff( void );		
		void								LevelAIOff();
		void								LevelAIOn();

		//-----------------------------------------------------------------
		// Parts functions
		//-----------------------------------------------------------------
		void								RegisterParts( Event *ev );
		void								RegisterSelf( Event *ev );
		void								PartName( Event *ev );
		Actor								*FindPartActor( const char *name );
		void								SendCommand( Event *ev );


		
		//-----------------------------------------------------------------
		// Dialog Functionality
		//-----------------------------------------------------------------
		DialogNode_t						*NewDialogNode(void);
		void								FreeDialogList( void );
		void								AddDialogParms( DialogNode_t *dialog_node, Event *ev );      
		void								AddDialog ( Event *ev );
		void								DialogDone( Event *ev );   
		void								DialogAnimDone( Event *ev );
		void								setDialogMorphMult( Event *ev );     
		void								PlayDialog( Event *ev );      
		void								StopDialog( Event *ev );
		void								PlayRadiusDialog( Sentient *user );		
		void								StopDialog( void );
		void								PlayDialog( Sentient *user, float volume = -1.0f, float min_dist = -1.0f,
														const char *dialog_name = NULL, const char *state_name = NULL, qboolean headDisplay = false , bool useTalk = false , bool important = false );
		qboolean							DialogExists( const str &aliasName );
		float								GetDialogRemainingTime( void );
		const str							FindDialog( Sentient *user, DialogType_t dialogType, const str& context = "" );


		void								BranchDialog(Event* ev);
		void								BranchOptionSelected(Event* ev);

		//-----------------------------------------------------------------
		// Branch Dialog Functionality
		//----------------------------------------------------------------
		void clearBranchDialog( void );
		void setBranchDialog( void );
		

		//-----------------------------------------------------------------
		// Context Dialog Functionality
		//-----------------------------------------------------------------
		void								InContext                 ( Event *ev );
		void								InContext				  ( const str& theContext , bool useDefaultMinDist  );


		//-----------------------------------------------------------------
		// Context Dialog Functions -- Resolve the alias and actually play the sound, and broadcast the soundtype to
		// nearby actors
		//-----------------------------------------------------------------
		void								BroadcastDialog   ( Event *ev );  
		qboolean							WantsToTalk();
      
	
		//-------------------------------------------------------------------
		// Twitch Functions
		//-------------------------------------------------------------------
		void								SetMouthAngle( Event *ev );
		

		//-----------------------------------------------------------------
		// Mode functions
		//-----------------------------------------------------------------
		qboolean							ModeAllowed( int new_mode );
		void								StartMode( int new_mode );
		void								EndMode( void );
		void								SaveMode( void );
		void								RestoreMode( void );

		//-----------------------------------------------------------------
		// Finishing functions
		//-----------------------------------------------------------------
		qboolean							CanBeFinished( void );
		qboolean							CanBeFinishedBy( int meansofdeath );
		
		void								Finish( int meansofdeath );
		void								StartLimbo( void );
		qboolean							InLimbo( void );


		//-------------------------------------------------------------------
		// Controller functions
		//-------------------------------------------------------------------
		bool								RequestControl( Listener *controller, ActorControlType controlType=ACTOR_CONTROL_AUTO_RELEASE );
		bool								ReleaseControl( Listener *controller );


		const str							GetStateVar ( const str& varName );

		//-----------------------------------------------------------------
		// General functions
		//-----------------------------------------------------------------
		static Actor*						FindActorByName(const str &name);
		static unsigned int					ActorTypeStringToInt( const str &type );
		void								IgnorePainFromActors( Event *ev );
		void								UpdateBossHealth( Event *ev );
		void								TouchTriggers( Event *ev );
		void								IgnoreWater( Event *ev );
		void								IgnorePlacementWarning( Event *ev );
		qboolean							CanTarget( void ) const;
		void								AddSpawnItem( Event *ev );
		void								ClearSpawnItems( Event *ev );
		void								SpawnItems( void );
		void								SpawnItem( const str &spawn_item_name );
		qboolean							CanJump( void );
		void								SetActorFlag( const str &flag_name, qboolean flag_value );
		void								SetStickToGround( const bool stick );
		void								SetStickToGround( Event *ev );
		const bool							GetStickToGround( void ) const;
		void								SetActorFlag( int flag, qboolean flag_value );
		void								SetActorFlag( Event *ev );
		qboolean							GetActorFlag( const int flag ) const;
		qboolean							GetActorFlag( const str &flag_name ) const;
		void								SetNotifyFlag( const str &flag_name, qboolean flag_value );
		void								SetNotifyFlag( int flag, qboolean flag_value );
		void								SetNotifyFlag( Event *ev );
		qboolean							GetNotifyFlag( int flag );
		void								BounceOffEvent( Event *ev );
		void								NotifyOthersAtDeath( Event *ev );
		void								NotifyOthersOfDeath( void );
		void								GotoNextStage( Event *ev );
		void								GotoPrevStage( Event *ev );
		void								GotoStage( Event *ev );
		void								GetStage( Event *ev );
		void								Pickup( Event *ev );
		void								Throw( Event *ev );
		virtual void						setSize( Vector min, Vector max );
		void								NoChatterEvent( Event *ev );
		virtual void						Chatter( const char *sound, float chance = 10, float volume = 1.0f, int channel = CHAN_VOICE );
		void								ActivateEvent( Event *ev );
		void								UseEvent( Event *ev );
		void								Think( void );
		void								Active( Event *ev );
		bool								IsEntityAlive( const Entity *ent );
		void								Name( Event *ev );
		void								SetupTriggerField( Event *ev );
		void								TriggerTouched( Event *ev );
		qboolean							GetClosestTag( const str &tag_name, int number_of_tags, const Vector &target, Vector *orig );
		void								AddStateFlag( unsigned int flag );
		void								ClearStateFlags( void );		
		void								SpawnActorAtTag( Event *ev );
		void								SpawnActorAtLocation( Event *ev );
		void								SpawnActorAboveEnemy( Event *ev );
		void								SpawnActor( const str &model_name, const Vector &orig, const Vector &ang, int	how_many, qboolean attack, float width, float height, qboolean force = false );
		void								TryTalkToPlayer( void );
		void								AllowTalk( Event *ev );
		void								AllowHangBack( Event *ev );
		void								SolidMask( Event *ev );
		void								IgnoreMonsterClip( Event *ev );
		void								NotSolidMask( Event *ev );
		void								NoMask( Event *ev );
		void								ResetMoveDir( Event *ev );
		int									ActorFlag_string_to_int( const str &actorflagstr ) const;
		int									NotifyFlag_string_to_int( const str &notifyflagstr );
		void								ArmorDamage( Event *ev );
		qboolean							CheckBottom( void );
		void								ChangeType( Event *ev );
		void								GetStateAnims( Container<const char *> *c );
		void								Touched( Event *ev );
		void								TryBlink( void );
		void								SetBlink( Event *ev );
		void								ReturnProjectile ( Event *ev );
		void								checkActorDead( Event *ev );
		qboolean							checkActorDead( void );
		qboolean							checkplayerrange ( float range , float height = 0 );		
		void								SetFlagOnEnemy( Event *ev );
		void								TurnOnEnemyAI( Event *ev );
		void								TurnOffEnemyAI( Event *ev );
		void								HandleGameSpecificEvent( Event *ev );
		void								EvaluateEnemies( Event *ev );
		void								ForgetEnemies( Event* ev );
		bool								IsFinishable();
		const str							getName() const {	return name; }
		void								SendEventToGroup( Event *ev );
		Actor*								GetAttachedChildActor( const str& childName );
		void								turnTowardsEntity(Entity *ent , float extraYaw );

		void								PrepareToFailMission( Event *ev );
		void								FailMission( Event *ev );

		void								DebugEvent( Event *ev );
		void								StartTalkBehavior(Sentient *user);

		void								SetContextInterval( Event *ev );
		void								SetContextInterval( float interval );

		void								SetEnemyTargeted( Event *ev );
		void								SetEnemyTargeted( bool targeted );

		void								SetActivationDelay( Event *ev );
		void								SetActivationDelay( float delay );

		void								SetActivationStart( Event *ev );
		void								SetActivationStart();

		void								SetCheckConeOfFireDistance( Event* ev );
		void								SetCheckConeOfFireDistance( float distance );

		void								AnimateOnce( Event *ev );

		void								SetDeathKnockbackValues( Event *ev );
		

		//-----------------------------------------------------------------
		// State machine conditions
		//-----------------------------------------------------------------
		qboolean							returntrue( Conditional &condition );
		qboolean							checkanimname( Conditional &condition );
		qboolean							checkinactive( Conditional &condition );
		qboolean							checkanimdone( Conditional &condition );
		qboolean							checktorsoanimdone( Conditional &condition );
		qboolean							checkdead( Conditional &condition );
		qboolean							checkhaveenemy( Conditional &condition );
		qboolean							checkenemydead( Conditional &condition );
		qboolean							checkenemydead( void );
		qboolean							checkenemynoclip( Conditional &condition );
		qboolean							checkcanseeenemy( Conditional &condition );	  
		qboolean							checkcanseeplayer( Conditional &condition );	  
		qboolean							checkcanshootenemy( Conditional &condition );		
		qboolean							checkenemyinfov( Conditional &condition );
		qboolean							checkenemyonground( Conditional &condition );
		qboolean							checkenemyrelativeyaw( Conditional &condition );
		qboolean							checkenemyyawrange( Conditional &condition );
		qboolean							checkenemyrange( Conditional &condition );
		qboolean							checkcanjumptoenemy( Conditional &condition );
		qboolean							checkcanflytoenemy( Conditional &condition );
		qboolean							checkinpain( Conditional &condition );
		qboolean							checksmallpain( Conditional &condition );
		qboolean							checkpainyaw( Conditional &condition );
		qboolean							checkpainpitch( Conditional &condition );
		qboolean							checkstunned( Conditional &condition );
		qboolean							checkfinished( Conditional &condition );
		qboolean							checkmeleehit( Conditional &condition );
		qboolean							checkblockedhit( Conditional &condition );
		qboolean							checkblocked( Conditional &condition );
		qboolean							checkBlockedByEnemy( Conditional &condition );
		qboolean							checkonfire( Conditional &condition );
		qboolean							checkotherdied( Conditional &condition );
		qboolean							checkstuck( Conditional &condition );
		qboolean							checknopath( Conditional &condition );
		qboolean							checkSteeringFailed( Conditional &condition );
		qboolean							checkbehaviordone( Conditional &condition );
		qboolean							checkheadbehaviordone( Conditional &condition );
		qboolean							checkeyebehaviordone( Conditional &condtion );
		qboolean							checktorsobehaviordone( Conditional &condition );
		qboolean							checktorsobehaviorfailed( Conditional &condition );
		qboolean							checktorsobehaviorsuccess( Conditional &condition );
		qboolean							checkbehaviorsuccess( Conditional &condition );
		qboolean							checkbehaviorfailed( Conditional &condition );
		qboolean							checktimedone( Conditional &condition );
		qboolean							checkdone( Conditional &condition );		
		qboolean							checkplayerrange( Conditional &condition );
		qboolean							checkparentrange ( Conditional &condition );
		qboolean							checkmovingactorrange( Conditional &condition );
		qboolean							checkchance( Conditional &condition );
		qboolean							checkstatetime( Conditional &condition );
		qboolean							checktimesdone( Conditional &condition );
		qboolean							checkmeansofdeath( Conditional &condition );
		qboolean							checknoiseheard( Conditional &condition );
		qboolean							checkpartstate( Conditional &condition );
		qboolean							checkpartflag( Conditional &condition );
		qboolean							checkpartdead( Conditional &condition );
		qboolean							checknumspawns( Conditional &condition );
		qboolean							checkcommand( Conditional &condition );
		qboolean							checktouched( Conditional &condition );
		qboolean							checktouchedbyplayer( Conditional &condition );
		qboolean							checktouchedbyplayer();
		qboolean							checkactivated( Conditional &condition );
		qboolean							checkused( Conditional &condition );
		qboolean							checktwitch( Conditional &condition );
		qboolean							checkhealth( Conditional &condition );
		qboolean							checkhealthpercent( Conditional &condition );
		qboolean							checkhealthpercentinrange( Conditional &condition );
		qboolean							checkonground( Conditional &condition );
		qboolean							checkinwater( Conditional &condition );
		qboolean							checkincomingmeleeattack( Conditional &condition );
		qboolean							checkincomingmeleeattack();
		qboolean							checkincomingrangedattack( Conditional &condition );
		qboolean							checkincomingprojectile( Conditional &condition );
		qboolean							checkenemystunned( Conditional &condition );
		qboolean							checkenemyinpath( Conditional &condition );
		qboolean							checkstage( Conditional &condition );
		qboolean							checkheld( Conditional &condition );
		qboolean							checkenemymelee( Conditional &condition );
		qboolean							checkenemyranged( Conditional &condition );
		qboolean							checkhasthing( Conditional &condition );
		qboolean							checkatcovernode( Conditional &condition );
		qboolean							checkallowhangback( Conditional &condition );
		qboolean							checkname( Conditional &condition );
		qboolean							checkVar( Conditional &condtion );
		qboolean							checkNodeExists( Conditional &condition );
		qboolean							checkCoverNodes( Conditional &condition );
		qboolean							checkSurfaceDamaged( Conditional &condition );
		qboolean							checkBoneDamaged( Conditional &condition );
		qboolean							checkRegionDamaged( Conditional &condition );
		qboolean							checkCaptured( Conditional &condition );
		qboolean							checkEnemyAttached( Conditional &condition );
		qboolean							checkCanWalkForward( Conditional &condition );
		qboolean							checkHasThrowObject( Conditional &condition );
		qboolean							checkEnemyIsThrowObject( Conditional &condition );
		qboolean							checkTurretMode( Conditional &condition );
		qboolean							checkMeleeHitWorld( Conditional &condition );
		qboolean							checkGameSpecific( Conditional &condition );
		qboolean							checkWeaponReady( Conditional &condition );			
		qboolean							checkPlayerValid( Conditional &condition );
		qboolean							checkInPreferredRange( Conditional &condtion );
		qboolean							checkInAbsoluteRange( Conditional &condition );
		qboolean							checkDisabled( Conditional &condition );
		qboolean							checkCrippled( Conditional &condition );
		qboolean							checkInAlcove( Conditional &condition ); 
		qboolean							checkInConeOfFire( Conditional &condition );
		qboolean							checkInPlayerConeOfFire( Conditional &condition );
		qboolean							checkPatrolWaypointNodeInDistance( Conditional &condition );
		qboolean							checkPathNodeTypeInDistance( Conditional &condition );
		qboolean							checkPlayerInCallVolume( Conditional &condition );
		qboolean							checkInCallVolume( Conditional &condition );
		qboolean							checkUsingWeaponNamed( Conditional &condition );
		qboolean							checkUsingWeaponNamed ( const str &name );
		qboolean							checkHaveActiveWeapon( Conditional &condition );
		qboolean							checkOutOfTorsoRange(Conditional &condition );
		qboolean							checkCanAttackAnyEnemy(Conditional &condition );
		qboolean							checkActorFlag( Conditional &condition );
		qboolean							checkplayerranged( Conditional &condition );
		qboolean							checkplayerranged();
		qboolean							checkForwardDirectionClear( Conditional &condition );
		qboolean							checkRearDirectionClear( Conditional &condition );
		qboolean							checkLeftDirectionClear( Conditional &condition );
		qboolean							checkRightDirectionClear( Conditional &condition );
		qboolean							checkForwardDirectionClear(float dist);
		qboolean							checkRearDirectionClear(float dist);
		qboolean							checkLeftDirectionClear(float dist);
		qboolean							checkRightDirectionClear(float dist);
		qboolean							checkLastState( Conditional &condition );
		qboolean							checkGroupMememberRange ( Conditional &condition );
		qboolean							checkActorType( Conditional &condition );
		qboolean							checkIsTeammate( Conditional &condition );
		qboolean							checkWeaponIsMelee( Conditional &condition );
		qboolean							checkWeaponChanged( Conditional &condition );
		qboolean							checkPersonality( Conditional &condition );
		qboolean							checkVarTimeDifference( Conditional &condition );
		qboolean							checkRequestedPosture( Conditional &condition );
		qboolean							checkPostureAnimDone( Conditional &condition );

		qboolean							checkCurrentEnemyLastInList( Conditional &condition );
		qboolean							checkGroupAttackerCount( Conditional &condition );
		qboolean							checkCurrentEnemyGroupAttackerCount( Conditional &condition );
		qboolean							checkGroupAttackerCountForEntity( Conditional& condition, Entity* attackTarget );
		qboolean							checkGroupAttackerCountForEntity( int checkValue, Entity* attackTarget );		


		qboolean							checkCountOfIdenticalNamesInGroup( Conditional &condition );
		qboolean							checkCountOfIdenticalNamesInGroup( const str &checkName , int checkValue );

		qboolean							checkCanAttackEnemy( Conditional &condition );
		qboolean							checkCanAttackEnemy();

		qboolean							checkDamageThresholdExceeded( Conditional &condition );
		qboolean							checkDamageThresholdExceeded();

		qboolean							checkAttacked( Conditional &condition );
		qboolean							checkAttacked();

		qboolean							checkAttackedByPlayer( Conditional &condition );
		qboolean							checkAttackedByPlayer();

		qboolean							checkHelperNodeWithFlagInRange( Conditional &condition );
		qboolean							checkHelperNodeWithFlagInRange( const str& flag, float range );

		qboolean							checkEnemyWeaponNamed( Conditional &condition );
		qboolean							checkEnemyWeaponNamed( const str &name );

		qboolean							checkPlayerWeaponNamed( Conditional &condition );
		qboolean							checkPlayerWeaponNamed( const str &name );

		qboolean							checkEnemyWithinRange( Conditional &condition );
		qboolean							checkEnemyWithinRange( float min , float max );

		qboolean							checkPropChance( Conditional &condition );
		qboolean							checkPropExists( Conditional &condition );

		qboolean							checkShowPain( Conditional &condition );
		qboolean							checkShowPain();

		qboolean							checkPropEnemyRange( Conditional &condition );
		qboolean							checkPropEnemyRange( const str& objname , const str& propname );

		qboolean							checkHaveBestWeapon( Conditional &condition );
		qboolean							checkHaveBestWeapon();

		qboolean							checkPosture( Conditional &condition );
		qboolean							checkPosture( const str& postureName );

		qboolean							checkAnyEnemyInRange( Conditional &condition );
		qboolean							checkAnyEnemyInRange( float range );

		qboolean							checkValidCoverNodeInRange( Conditional &condition );
		qboolean							checkValidCoverNodeInRange( float maxDistanceFromSelf, float minDistanceFromCurrentEnemy, float minDistanceFromPlayer );

		qboolean							checkValidCombatNodeInRange( Conditional &condition );
		qboolean							checkValidCombatNodeInRange( float maxDistanceFromSelf, float minDistanceFromPlayer, bool unreserveCurrentNode = true );

		qboolean							checkValidWorkNodeInRange( Conditional &condition );
		qboolean							checkValidWorkNodeInRange( float maxDistanceFromSelf, bool unreserveCurrentNode = true);

		qboolean							checkValidHibernateNodeInRange( Conditional &condition );
		qboolean							checkValidHibernateNodeInRange( float maxDistanceFromSelf );

		qboolean							checkValidPatrolNodeInRange( Conditional &condition );
		qboolean							checkValidPatrolNodeInRange( float maxDistanceFromSelf );

		qboolean							checkValidSniperNodeInRange( Conditional &condition );
		qboolean							checkValidSniperNodeInRange( float maxDistanceFromSelf );

		qboolean							checkValidCustomNodeInRange( Conditional &condition );
		qboolean							checkValidCustomNodeInRange( const str &customType , float maxDistanceFromSelf );

		qboolean							checkEnemyCanSeeCurrentNode( Conditional &condition );
		qboolean							checkEnemyCanSeeCurrentNode();

		qboolean							checkSpecifiedFollowTargetOutOfRange( Conditional &condition );
		qboolean							checkSpecifiedFollowTargetOutOfRange();

		qboolean							checkCurrentNodeHasThisCoverType( Conditional &condition );
		qboolean							checkCurrentNodeHasThisCoverType( const str &coverType );

		qboolean							checkShouldDoAction( Conditional &condition );
		qboolean							checkShouldDoAction( const str &tendencyName );

		qboolean							checkInTheWay( Conditional &condition );
		qboolean							checkInTheWay();

		qboolean							checkHaveArmor( Conditional &condition );
		qboolean							checkHaveArmor();

		qboolean							checkWithinFollowRangeMin( Conditional &condition );
		qboolean							checkWithinFollowRangeMin();

		qboolean							checkAllowedToMeleeEnemy( Conditional &condition );
		qboolean							checkAllowedToMeleeEnemy();
		
		qboolean							checkHavePathToEnemy( Conditional &condition );
		qboolean							checkHavePathToEnemy();

		qboolean							checkEnemyProjectileClose( Conditional &condition );
		qboolean							checkEnemyProjectileClose();

		qboolean							checkActivationDelayTime( Conditional &condition );
		qboolean							checkActivationDelayTime();
		
		qboolean							checkTalking( Conditional &condition );
		qboolean							checkTalking();

		qboolean							checkEnemiesNearby( Conditional &condition );
		qboolean							checkEnemiesNearby( float distance );

		//-----------------------------------------------------------------
		// Tendency Checks
		//-----------------------------------------------------------------
		qboolean							checkIsAggressive ( Conditional &condition );
		qboolean							checkWantsToExecutePackage( Conditional &condition );
		qboolean							checkExecutedPackageInLastTimeFrame( Conditional &condition );

		
		//-----------------------------------------------------------------
		// Temporary
		//-----------------------------------------------------------------
		qboolean							checkInAIMode( Conditional &condition );
		

		virtual void						Archive( Archiver &arc );

      
		protected: 
		void								_dropActorToGround();		
		void								_printDebugInfo(const str &laststate , const str &currentState , const str &legAnim , const str &torsoAnim);
		void								_notifyGroupOfDamage();
		void								_notifyGroupSpottedEnemy();
		void								_notifyGroupOfKilled();
		void								_notifyGroupOfEnemy();
		qboolean							_isWorkNodeValid(PathNode* node);

		qboolean							_WorkNodeInDistance( float dist );
		qboolean							_FleeNodeInDistance( float dist );
};

typedef PathFinder<FindMovement> FindMovementPath;
typedef PathFinder<FindCoverMovement> FindCoverPath;
typedef PathFinder<FindFleeMovement> FindFleePath;
typedef PathFinder<FindEnemyMovement> FindEnemyPath;



#endif /* actor.h */
