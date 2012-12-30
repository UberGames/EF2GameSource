//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/player.h                                      $
// $Revision:: 242                                                            $
//   $Author:: Steven                                                         $
//     $Date:: 9/18/03 9:02a                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Class definition of the player.
//

//==============================
// Forward Declarations
//==============================
class Player;
class Powerup;
class PowerupBase;
class Rune;
class HoldableItem;

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "g_local.h"
#include "vector.h"
#include "entity.h"
#include "weapon.h"
#include "WeaponDualWield.h"
#include "sentient.h"
#include "navigate.h"
#include "misc.h"
#include "bspline.h"
#include "camera.h"
#include "specialfx.h"
#include "characterstate.h"
#include "actor.h"
#include "vehicle.h"
#include "playerheuristics.h"

extern Event EV_Player_EndLevel;
extern Event EV_Player_GiveCheat;
extern Event EV_Player_GodCheat;
extern Event EV_Player_NoTargetCheat;
extern Event EV_Player_NoClipCheat;
extern Event EV_Player_GameVersion;
extern Event EV_Player_Fov;
extern Event EV_Player_WhatIs;
extern Event EV_Player_Respawn;
extern Event EV_Player_WatchActor;
extern Event EV_Player_StopWatchingActor;
extern Event EV_Player_DoStats;
extern Event EV_Player_DeadBody;
extern Event EV_Weapon_DonePutaway;
extern Event EV_Weapon_TargetIdleThink;
extern Event EV_Driver_AnimDone;
extern Event EV_Weapon_DoneAnimating;
extern Event EV_Player_DoUse;
extern Event EV_ViewThing_SpawnFromTS;
extern Event EV_Player_MissionFailed;
extern Event EV_Weapon_Shoot;
extern Event EV_DetachAllChildren;
extern Event EV_Player_RemovePowerup;
extern Event EV_Player_UseItem;
extern Event EV_Player_SetStat;
extern Event EV_Player_Weapon;

#define HEAD_TAG        0
#define TORSO_TAG       1
#define L_ARM_TAG       2
#define R_ARM_TAG       3
#define MOUTH_TAG		4

enum painDirection_t
   {
   PAIN_NONE,
   PAIN_FRONT,
   PAIN_LEFT,
   PAIN_RIGHT,
   PAIN_REAR
   };

typedef enum
{
	NONE,
	FRIEND, 
	ENEMY,
	OJBECT
} TargetTypes;

enum PlayerCameraModes
{
	PLAYER_CAMERA_MODE_NORMAL,
	PLAYER_CAMERA_MODE_ACTOR,
	PLAYER_CAMERA_MODE_ENTITY_WATCHING,
	PLAYER_CAMERA_MODE_NO_CLIP,
	PLAYER_CAMERA_MODE_CINEMATIC,
};

typedef void ( Player::*movecontrolfunc_t )( void );


//------------------------- CLASS ------------------------------
//
// Name:          FinishingMove
// Base Class:    Class
//
// Description:   Finishing move class
//
// Method of Use: 
//--------------------------------------------------------------
class FinishingMove : public Class
	{
	public:
		FinishingMove() { }
		~FinishingMove() { }

		str statename;
		int direction;
		float coneangle;
		float distance;
		float chance;
		float enemyyaw;
		void Archive( Archiver &arc );
	};

inline void FinishingMove::Archive( Archiver &arc )
   {
	Class::Archive( arc );

   arc.ArchiveString( &statename );
   arc.ArchiveInteger( &direction );
   arc.ArchiveFloat( &coneangle );
   arc.ArchiveFloat( &distance );
   arc.ArchiveFloat( &chance );
   arc.ArchiveFloat( &enemyyaw );
   }

//------------------------- CLASS ------------------------------
//
// Name:          WeaponSetItem
// Base Class:    Class
//
// Description:   This is a dual weapon
//
// Method of Use: 
//--------------------------------------------------------------
class WeaponSetItem : public Class
   {
   public:
      str name;
      weaponhand_t hand;
      void Archive( Archiver &arc );
   };

inline void WeaponSetItem::Archive( Archiver &arc )
   {
	Class::Archive( arc );
	
   arc.ArchiveString( &name );
   ArchiveEnum( hand, weaponhand_t );
   }


//------------------------- CLASS ------------------------------
//
// Name:          Player
// Base Class:    Sentient
//
// Description:   This is the player.
//
// Method of Use: 
//--------------------------------------------------------------
class Player : public Sentient
	{
	public:
		CLASS_PROTOTYPE( Player );
		Player();
		~Player();

		static Condition<Player> Conditions[];

		char			lastTeam[ 16 ];
		qboolean		mp_savingDemo;
		float			userFov;

	private:
		friend class		Camera;
		friend class		Vehicle;
		friend class		HorseVehicle;

		static movecontrolfunc_t MoveStartFuncs[];

		StateMap			*statemap_Legs;
		StateMap			*statemap_Torso;

		State				*currentState_Legs;
		State				*currentState_Torso;
		str 				last_torso_anim_name;
		str 				last_leg_anim_name;
		movecontrol_t		movecontrol;
		int 				last_camera_type;

		ActiveWeapon		newActiveWeapon;
		EntityPtr			head_target;
		float				look_at_time;
		EntityPtr			targetEnemy;
		bool				targetEnemyLocked;

		qboolean			shield_active;

		qboolean						dual_wield_active;
		Container<WeaponSetItem *>	dual_wield_weaponlist;

		WeaponPtr			holsteredWeapons[MAX_ACTIVE_WEAPONS];

		str					partAnim[ 3 ];

		bool				animdone_Legs;
		bool				animdone_Torso;

		Vector				oldvelocity;
		Vector				old_v_angle;
		Vector				oldorigin;
		float 				animspeed;
		float 				airspeed;

		// Call Volume Stuff
		str					currentCallVolume;

		// Armor Stuff
		//float 			ArmorValue;

		// blend
		float				blend[ 4 ];		// rgba full screen effect
		float				fov;			// horizontal field of view
		float				_userFovChoice;

		// vehicle stuff
		VehiclePtr			vehicle;

		// aiming direction
		Vector				v_angle;

		int					buttons;
		int					new_buttons;
		float				respawn_time;

		int					last_attack_button;

		// damage blend
		float				damage_blood;
		float				damage_alpha;
		Vector				damage_blend;
		Vector				damage_from;		 // the direciton of incoming damage
		Vector				damage_angles;		 // the damage angle adjustment that should be applied to the player
		float				damage_count;		 // incoming damage which is decayed over time
		float				bonus_alpha;
		float				next_drown_time;	 // how long until we drown again
		float				next_painsound_time; // when we should make a pain sound again
		float				air_finished;
		int 				old_waterlevel;
		float				drown_damage;
		float				_flashAlpha;
		Vector				_flashBlend;
		float				_flashMaxTime;
		float				_flashMinTime;

		str					waitForState;		 // if not null, than player will clear waitforplayer when this state is hit
		CameraPtr			camera;
		CameraPtr			actor_camera;
		CameraPtr			cool_camera;
		EntityPtr			entity_to_watch;
		float				maximumAngleToWatchedEntity;
		bool				watchEntityForEntireDuration;
		int					playerCameraMode;

		// Music Stuff
		float 				action_level;
		int					music_current_mood;
		int					music_fallback_mood;
		float				music_current_volume;
		float				music_saved_volume;
		float				music_volume_fade_time;
		int 				reverb_type;
		float				reverb_level;
		qboolean			music_forced;
		bool				_allowMusicDucking;
		bool				_allowActionMusic;

		qboolean			gibbed;
		float 				pain;
		painDirection_t		pain_dir;
		meansOfDeath_t		pain_type;
		bool				take_pain;
		float 				accumulated_pain;
		float 				nextpaintime;
		bool				knockdown;

		bool				canfall;
		bool				falling;
		int					feetfalling;
		Vector				falldir;

		bool				hardimpact;
		usercmd_t 			last_ucmd;

		// Movement Variables
		//Vector			base_righthand_pos;
		//Vector			base_lefthand_pos;
		//Vector			righthand_pos;
		//Vector			lefthand_pos;
		Vector				base_rightfoot_pos;
		Vector				base_leftfoot_pos;
		Vector				rightfoot_pos;
		Vector				leftfoot_pos;

		bool				_onLadder;
		Vector				_ladderNormal;
		float				_ladderTop;
		float				_nextLadderTime;

		int					pm_lastruntime;   // the last runtime before Pmove

		float 				animheight;

		Vector				yaw_forward;
		Vector				yaw_left;

		EntityPtr 			atobject;
		float 				atobject_dist;
		Vector				atobject_dir;

		EntityPtr 			toucheduseanim;
		int					useanim_numloops; // number of times to loop the animation
		EntityPtr 			useitem_in_use;   // used so that we can trigger targets after the useitem is finished
		EntityPtr 			cool_item;		  // we picked up a cool item and are waiting to show it off
		str					cool_dialog;	  // dialog to play when we get it
		str					cool_anim;		  // anim to play after the cinematic

		int					moveresult;
		qboolean			do_rise;		  // whether or not the player should go into the rise animation
		qboolean			weapons_holstered_by_code; // whether or not we initiated a holstering for a specific animation

		qboolean			projdetonate;

		// leg angles
		qboolean			yawing;
		qboolean			yawing_left;
		qboolean			yawing_right;
		qboolean			adjust_torso;

		Vector				torsoAngles;
		Vector				headAngles;
		Vector				headAimAngles;
		Vector				torsoAimAngles;

		float				damage_multiplier;

		// dummyPlayer stuff
		qboolean			fakePlayer_active;
		ActorPtr			fakePlayer;
		
		bool				dont_turn_legs;
		float				specialMoveCharge;
		float				specialMoveEndTime;
		float				specialMoveChargeTime;
		int 				points;
		float				playerKnockback;
		float				knockbackMultiplier;
		bool				changedStanceTorso;
		bool				changedStanceLegs;
		int 				stanceNumber;
		bool				incomingMeleeAttack;
		float				bendTorsoMult;
		str 				lastActionType;
		int 				meleeAttackFlags;
		bool				changingChar;
		

		Container<EntityPtr>		meleeAttackerList;
		Container<Conditional *>	legs_conditionals;
		Container<Conditional *>	torso_conditionals;
		Container<ActorPtr>			finishableList;
		Container<FinishingMove *>	finishingMoveList;

		SafePtr<Powerup>		_powerup;
		SafePtr<Rune>			_rune;
		SafePtr<HoldableItem>	_holdableItem;

		float				_nextEnergyTransferTime;
		bool				_canTransferEnergy;

		bool				_doDamageScreenFlash;
		bool				_isThirdPerson;

		Actor*				_finishActor;
		str					_finishState;
		bool				_doingFinishingMove;

		bool				_autoSwitchWeapons;

		bool				_usingEntity;
		str					_attackType;
		int					_gameplayAnimIdx;
		bool				_disableUseWeapon;
		EntityPtr			_targetSelectedHighlight;
		EntityPtr			_targetLockedHighlight;

		bool				_infoHudOn;
		int					_nextRegenTime;
		float				_useEntityStartTimer;

		int					_objectiveNameIndex;
		unsigned int		_objectiveStates;
		unsigned int		_informationStates;

		EntityPtr			_targetedEntity;

		int					_dialogEntnum;
		int					_dialogSoundIndex;
		int					_dialogTextSoundIndex;

		float				_crossHairXOffset;
		float				_crossHairYOffset;

		float				_lastDamagedTimeFront;
		float				_lastDamagedTimeBack;
		float				_lastDamagedTimeLeft;
		float				_lastDamagedTimeRight;

		bool				_updateGameFrames;
		int					_totalGameFrames;

		float				_nextPainShaderTime;
		meansOfDeath_t		_lastPainShaderMod;

		int					_itemIcon;
		str					_itemText;

		str					_voteText;

		bool				_validPlayerModel;

		int					_secretsFound;

		Container<str>		_hudList;
		bool				_needToSendHuds;

		bool				_started;

		int					_skillLevel;

		pmtype_t			_forcedMoveType;
		
		ActorPtr			_branchDialogActor;
		bool				_needToSendBranchDialog;

		Vector				_backpackAttachOffset;
		Vector				_backpackAttachAngles;

		Vector				_flagAttachOffset;
		Vector				_flagAttachAngles;

		bool				_cameraCutThisFrame;

	// Conditional Functions
	public:
		qboolean			returntrue( Conditional &condition );
		qboolean			checkturnleft( Conditional &condition );
		qboolean			checkturnright( Conditional &condition );
		qboolean			checkforward( Conditional &condition );
		qboolean			checkbackward( Conditional &condition );
		qboolean			checkstrafeleft( Conditional &condition );
		qboolean			checkstraferight( Conditional &condition );
		qboolean			checkduck( Conditional &condition );
		qboolean			checkleanleft(Conditional &condition);
		qboolean			checkleanright(Conditional &condition);
		qboolean			checkjump( Conditional &condition );
		qboolean			checkcrouch( Conditional &condition );
		qboolean			checkjumpflip( Conditional &condition );
		qboolean			checkanimdone_legs( Conditional &condition );
		qboolean			checkanimdone_torso( Conditional &condition );
		qboolean			checkattackleft( Conditional &condition );
		qboolean			checkattackright( Conditional &condition );
		qboolean			checkattackbuttonleft( Conditional &condition );
		qboolean			checkattackbuttonright( Conditional &condition );
		qboolean			checksneak( Conditional &condition );
		qboolean			checkrun( Conditional &condition );
		qboolean			checkwasrunning( Conditional &condition );
		qboolean			checkholsterweapon( Conditional &condition );
		qboolean			checkuse( Conditional &condition );
		qboolean			checkcanturn( Conditional &condition );
		qboolean			checkcanwallhug( Conditional &condition );
		qboolean			checkblocked( Conditional &condition );
		qboolean			checkhangatwall( Conditional &condition );
		qboolean			checkonground( Conditional &condition );
		qboolean			check22degreeslope( Conditional &condition );
		qboolean			check45degreeslope( Conditional &condition );
		qboolean			checkrightleghigh( Conditional &condition );
		qboolean			checkleftleghigh( Conditional &condition );
		qboolean			checkfacingupslope( Conditional &condition );
		qboolean			checkfacingdownslope( Conditional &condition );
		qboolean			checkcanfall( Conditional &condition );
		qboolean			checkatdoor( Conditional &condition );
		qboolean			checkfalling( Conditional &condition );
		qboolean			checkgroundentity( Conditional &condition );
		qboolean			checkhardimpact( Conditional &condition );
		qboolean			checkdead( Conditional &condition );
		qboolean			checkhealth( Conditional &condition );
		qboolean			checkpain( Conditional &condition );
		qboolean			checkpaindirection( Conditional &condition );
		qboolean			checkaccumulatedpain( Conditional &condition );
		qboolean			checkpaintype( Conditional &condition );
		qboolean			checkpainthreshold( Conditional &condition );
		qboolean			checkknockdown( Conditional &condition );
		qboolean			checklegsstate( Conditional &condition );
		qboolean			checktorsostate( Conditional &condition );
		qboolean			checkatuseanim( Conditional &condition );
		qboolean			checkatuseentity( Conditional &condition );
		qboolean			checktouchuseanim( Conditional &condition );
		qboolean			checkatuseobject( Conditional &condition );
		qboolean			checkloopuseobject( Conditional &condition );
		qboolean			checkuseweaponleft( Conditional &condition );
		qboolean			checknewweapon( Conditional &condition );
		qboolean			checkuseweapon( Conditional &condition );
		qboolean			checkhasweapon( Conditional &condition );
		qboolean			checkweaponactive( Conditional &condition );
		qboolean			checkhasdualweapon( Conditional &condition );
		qboolean			checkweaponreload( Conditional &condition );
		qboolean			checkweaponswitchmode( Conditional &condition );
		qboolean			checkweaponinmode( Conditional &condition );
		qboolean			checkputawayleft( Conditional &condition );
		qboolean			checkputawayright( Conditional &condition );
		qboolean			checkputawayboth( Conditional &condition );
		qboolean			checktargetacquired( Conditional &condition );
		qboolean			checkanyweaponactive( Conditional &condition );
		qboolean			checkstatename( Conditional &condition );
		qboolean			checkattackblocked( Conditional &condition );
		qboolean			checkblockdelay( Conditional &condition );
		qboolean			checkcanstand( Conditional &condition );
		qboolean			checkpush( Conditional &condition );
		qboolean			checkpull( Conditional &condition );
		qboolean			checkOnLadder( Conditional &condition );
		qboolean			checkdualwield( Conditional &condition );
		qboolean			checkdualweapons( Conditional &condition );
		qboolean			checkuseanimfinished( Conditional &condition );
		qboolean			checkchance( Conditional &condition );
		qboolean			checkturnedleft( Conditional &condition );
		qboolean			checkturnedright( Conditional &condition );
		qboolean			checkinvehicle( Conditional &condition );
		qboolean			checksolidforward( Conditional &condition );
		qboolean			checkholstercomplete( Conditional &condition );
		qboolean			checkweaponhasammo( Conditional &condition );
		qboolean			checkweaponhasfullammo( Conditional &condition );
		qboolean			checkweaponhasinvammo( Conditional &condition );
		qboolean			checkrise( Conditional &condition );
		qboolean			checkweaponsholstered( Conditional &condition );
		qboolean			checkdualweaponreadytofire( Conditional &condition );
		qboolean			checkweaponreadytofire( Conditional &condition );
		qboolean			checkfakeplayeractive( Conditional &condition );
		qboolean			checkfakeplayerholster( Conditional &condition );
		qboolean			checkweaponlowered( Conditional &condition );
		qboolean			checkweaponfiretimer( Conditional &condition );
		qboolean			checkweaponforcereload( Conditional &condition );
		qboolean			checkweaponcanreload( Conditional &condition );
		qboolean			checkweaponfullclip( Conditional &condition );
		qboolean			checkweapondonefiring( Conditional &condition );
		qboolean			checkspecialmovecharge( Conditional &condition );
		qboolean			checkcharavailable( Conditional &condition );
		qboolean			checkstancechangedtorso( Conditional &condition );
		qboolean			checkstancechangedlegs( Conditional &condition );
		qboolean			checkstance( Conditional &condition );
		qboolean			checkpoints( Conditional &condition );
		qboolean			checkincomingmeleeattack( Conditional &condition );
		qboolean			checkfinishingmove( Conditional &condition );
		qboolean			checkusingentity( Conditional &condition );
		qboolean			checkthirdperson( Conditional &condition );
		qboolean			checkPropChance( Conditional &condition );
		qboolean			checkPropExists( Conditional &condition );
		qboolean			checkEndAnimChain( Conditional &condition );
		qboolean			checkWeaponType( Conditional &condition );
		qboolean			checkHasAnim( Conditional &condition );
		qboolean			checkIsWeaponControllingProjectile( Conditional &condition );

		// Movecontrol Functions
		void				StartPush( void );
		void				StartUseAnim( void );
		void				StartLoopUseAnim( void );
		void				SetupUseObject( void );
		void				StartUseObject( Event *ev );
		void				FinishUseObject( Event *ev );
		void				FinishUseAnim( Event *ev );
		void				StepUp( Event *ev );
		void				Turn( Event *ev );
		void				turnTowardsEntity( Event *ev );
		void				TurnUpdate( Event *ev );
		void				TurnLegs( Event *ev );
		void				DontTurnLegs( Event *ev );
		void				PassEventToVehicle( Event *ev );

		// Init Functions
		void				Init( void );
		void				InitSound( void );
		void				InitEdict( void );
		void				InitClient( void );
		void				InitPhysics( void );
		//void				InitPowerups( void );
		void				InitWorldEffects( void );
		void				InitWeapons( void );
		void				InitView( void );
		void				InitModel( const char *modelName = NULL );
		void				InitState( void );
		void				InitHealth( void );
		void				InitInventory( void );
		void				InitStats( void );
		void				ChooseSpawnPoint( void );
		void				EndLevel( Event *ev );
		void				Respawn( Event *ev );
		void				LevelCleanup( void );

		void				SetDeltaAngles( void );
		virtual void		setAngles( const Vector &ang );
		void				SetTargetedEntity(EntityPtr entity);
		EntityPtr			GetTargetedEntity(void) 				{ return _targetedEntity;	}

		void				CheckForTargetedEntity( void );
		void				ProcessTargetedEntity( void );

		void				DoUse( Event *ev );
		Actor *				getBestActorToUse( int *entityList, int count );
		void				Killed( Event *ev );
		
		void				Dead( Event *ev );
		void				Pain( Event *ev );

		void				TouchStuff( const pmove_t *pm );

		void				disableInventory( void );
		void				enableInventory( void );

		usercmd_t			GetLastUcmd(void);
		void				GetMoveInfo( pmove_t *pm );
		void				SetMoveInfo( pmove_t *pm, const usercmd_t *ucmd );
		pmtype_t			GetMovePlayerMoveType( void );
		void				ClientMove( usercmd_t *ucmd );
		void				ApplyPowerupEffects(int &moveSpeed );
		void				ClientMoveLadder( usercmd_t *ucmd );
		void				ClientMoveDuck( usercmd_t *ucmd );
		void				ClientMoveLean(usercmd_t *ucmd);
		void				CheckMoveFlags( void );
		void				ClientMoveFlagsAndSpeeds( int moveSpeed, int noclipSpeed, int crouchSpeed, int airSpeed );
		void				ClientMoveMisc( usercmd_t *ucmd );
		void				ClientThink( Event *ev );

		void				LoadStateTable( void );
		void				SetState(const str& legsstate, const str& torsostate);
		void				ResetState( Event *ev );
		void				EvaluateState( State *forceTorso=NULL, State *forceLegs=NULL );

		void				CheckGround( void );
		void				UpdateViewAngles( usercmd_t *cmd );
		qboolean			AnimMove( const Vector &move, Vector *endpos = NULL );
		qboolean			TestMove( const Vector &pos, Vector *endpos = NULL	);
		qboolean			CheckMove( const Vector &move, Vector *endpos = NULL );

		void				EndAnim_Legs( Event *ev );
		void				EndAnim_Torso( Event *ev );
		void				SetAnim( const char *anim, bodypart_t part = legs, bool force = false );

		void				EventUseItem( Event *ev );
		void				ChangeStance( Event *ev );
		void				ClearStanceTorso( Event *ev );
		void				ClearStanceLegs( Event *ev );
		void				TouchedUseAnim( Entity * ent );

		void				GiveCheat( Event *ev );
		void				GiveWeaponCheat( Event *ev );
		void				GiveAllCheat( Event *ev );
		void				GodCheat( Event *ev );
		void				NoTargetCheat( Event *ev );
		void				NoclipCheat( Event *ev );
		void				Kill( Event *ev );
		void				GibEvent( Event *ev );
		void				SpawnEntity( Event *ev );
		void				SpawnActor( Event *ev );
		void				ListInventoryEvent( Event *ev );
		void				SetViewAnglesEvent( Event *ev );
		void				GivePointsEvent( Event *ev );
		void				SetPointsEvent( Event *ev );
		void				SetCurrentCallVolume( const str &volumeName );
		str					GetCurrentCallVolume();

		void				GameVersion( Event *ev );
		void				Fov( Event *ev );

		void				GetPlayerView( Vector *pos, Vector *angle );

		float				CalcRoll( void );
		void				WorldEffects( void );
		void				AddBlend( float r, float g, float b, float a );
		void				SetBlend( float r, float g, float b, float a, int additive );
		void				SetBlend( int additive );
		void				CalcBlend( void );

		void				setFlash( const Vector &color, float alpha, float minTime, float maxTime );

		void				DamageFeedback( void );

		void				UpdateStats( void );
		void				UpdateMusic( void );
		void				UpdateReverb( void );
		void				UpdateMisc( void );
		void				UpdateObjectiveStatus( void );

		void				SetReverb( const str &type, float level );
		void				SetReverb( int type, float level );
		void				SetReverb( Event *ev );

		Camera				*CurrentCamera( void );
		void				SetCamera( Camera *ent, float switchTime );
		void				CameraCut( void );
		void				CameraCut( Camera *ent );

		void				SetClientViewAngles( const Vector &position, const float cameraoffset, const Vector &ang, const Vector &vel, const float camerafov ) const;
		void				ShakeCamera( void );
		void				SetPlayerViewUsingNoClipController( void );
		void				SetPlayerViewUsingActorController( Camera *camera );
		void				SetPlayerViewUsingCinematicController( Camera* camera );
		void				SetPlayerViewUsingEntityWatchingController( void );
		void				SetPlayerViewNormal( void );
		void				StopWatchingEntity( void );
		void				DestroyActorCamera( void );
		void				SetupView( void );
		void				AutomaticallySelectedNewCamera( void );

		void				ProcessPmoveEvents( int event );

		void				SwingAngles( float destination, float swingTolerance, float clampTolerance, float speed, float *angle, qboolean *swinging );
		Entity const *		GetTarget( void ) const;
		bool				GetProjectileLaunchAngles( Vector &launchAngles, const Vector &launchPoint, const float initialSpeed, const float gravity ) const;
		void				AcquireTarget( void );
		void				AutoAim( void );
		void				PlayerAngles( void );
		void				FinishMove( void );
		void				EndFrame( Event *ev );

		void				TestThread( Event *ev );
		bool				useWeapon( const char *weaponname, weaponhand_t hand );
		bool				useWeapon( Weapon *weapon, weaponhand_t hand );

		void				GotKill( Event *ev );
		void				SetPowerupTimer( Event *ev );
		void				UpdatePowerupTimer( Event *ev );

		void				WhatIs( Event *ev );
		void				ActorInfo( Event *ev );
		void				Taunt( Event *ev );

		void				ChangeMusic( const char * current, const char * fallback, qboolean force );
		void				ChangeMusicVolume( float volume , float fade_time);
		void				RestoreMusicVolume( float fade_time );

		void				allowMusicDucking( bool allowMusicDucking );
		void				allowActionMusic( bool allowActionMusic );

		void				GravityNodes( void );
		virtual void 		Archive( Archiver &arc );
		virtual void		ArchivePersistantData( Archiver &arc, qboolean sublevelTransition );

		void				GiveOxygen( float time );

		void				KillEnt( Event *ev );
		void				RemoveEnt( Event *ev );
		void				KillClass( Event *ev );
		void				RemoveClass( Event *ev );

		void				Jump( Event *ev );
		void				JumpXY( Event *ev );

		void				ActivateNewWeapon( Event *ev );
		void				ActivateDualWeapons( Event *ev );
		void				DeactivateWeapon( Event *ev );
		void				PutawayWeapon( Event *ev );
		void				DonePutaway( Event *ev );

		void				ActivateShield( Event *ev );
		void				DeactivateShield( Event *ev );
		qboolean			ShieldActive( void );
		qboolean			LargeShieldActive( void );

		void				StartFakePlayer( void );
		void				FakePlayer( qboolean holster );
		void				RemoveFakePlayer( void );
		void				SetViewAngles( Vector angles );
		Vector				getViewAngles( void ) { return v_angle; }
		void				SetFov( float newFov, bool forced = false );
		float				getDefaultFov( void );

		bool				IsNewActiveWeapon( void );
		Weapon				*GetNewActiveWeapon( void );
		weaponhand_t		GetNewActiveWeaponHand( void );
		void				ClearNewActiveWeapon( void );

		void				DumpState( Event *ev );
		void				ForceTorsoState( Event *ev );

		void				SetHeadTarget( Event *ev );
		Vector				GetAngleToTarget( const Entity *ent, const str &tag, float yawclamp, float pitchclamp, const Vector &baseangles );

		void				AcquireHeadTarget( void );

		void				SetCurrentCombo( Event *ev );

		qboolean			GetTagPositionAndOrientation( const str &tagname, orientation_t *new_or );
		qboolean			GetTagPositionAndOrientation( int tagnum, orientation_t *new_or );

		void				DebugWeaponTags( int controller_tag, Weapon *weapon, const str &weapon_tagname );
		void				ClearTarget( Event *ev );
		void				AdjustTorso( Event *ev );
		void				UseDualWield( Event *ev );
		void				DualWield( Event *ev );
		void				EvaluateTorsoAnim( Event *ev );
		void				CheckReloadWeapons( void );
		void				NextPainTime( Event *ev );
		void				SetTakePain( Event *ev );

		void				SetMouthAngle( Event *ev );

		void				EnterVehicle( Event *ev );
		void				ExitVehicle( Event *ev );
		void				Holster( Event *ev );
		void				HolsterToggle( Event *ev );
		void				NightvisionToggle (Event *ev);

		void				IncreaseActionLevel( float action_level_increase );

		void				WatchEntity( Event *ev );
		void				StopWatchingEntity(	Event *ev );
		void				WeaponCommand( Event *ev );
		void				PlayerDone( Event *ev );
		painDirection_t		Pain_string_to_int( const str &pain );
		inline Vector 		GetTorsoAngles( void ) { return torsoAngles; };
		inline Vector 		GetVAngles( void ){ return v_angle; }

		
		const str			getPainShader( meansOfDeath_t mod, bool takeArmorIntoAccount );
		const str			getPainShader( const char *MODName );

		void				SpawnDamageEffect( meansOfDeath_t mod );

		virtual void		GetStateAnims( Container<const char *> *c );
		virtual void		SetStateFile( Event *ev );

		virtual void		VelocityModified( void );
		int 				GetKnockback( int original_knockback, qboolean blocked );
		virtual void		ReceivedItem( Item * item );
		virtual void		RemovedItem( Item * item );
		virtual void		AmmoAmountChanged( Ammo * ammo, int inclip = 0 );
		qboolean			WeaponsOut( void );
		qboolean			IsDualWeaponActive( void );
		void				Holster( qboolean putaway );
		void				SafeHolster( qboolean putaway );
		inline float		GetFov() { return fov; }
		inline bool			GetTargetEnemyLocked( void ) const { return targetEnemyLocked; }

		void				StartCoolItem( Event *ev );
		void				StartCoolItemAnim( void );
		void				ShowCoolItem( Event *ev );
		void				HideCoolItem( Event *ev );
		void				StopCoolItem( Event *ev );
		void				WaitForState( Event *ev );
		void				SkipCinematic( Event *ev );
		void				SetDamageMultiplier( Event *ev );
		void				LogStats( Event *ev );
		void				WeaponsHolstered( void );
		void				WeaponsNotHolstered( void );
		void				Loaded( void );
		void				PlayerShowModel( Event *ev );
		virtual void		showModel( void );
		void				ResetHaveItem( Event *ev );
		qboolean			GetCrouch();

		int					getTotalGameFrames()			{ return _totalGameFrames;	}

		// Deathmatch arena stuff
		void				Score( Event *ev );

		void				WarpToPoint( const Entity *ent );
		void				ArmorDamage( Event *ev );
		void				Disconnect( void );
		void				CallVote( Event *ev );
		void				Vote( Event *ev );
		void				joinTeam( Event *ev );
		void				multiplayerCommand( Event *ev );
		void				DeadBody( Event *ev );
		void				Gib( void );

		void				FireWeapon(Event *ev);
		void				ReleaseFireWeapon(Event *ev);
		void				SetAimType(Event *ev);
		void				ReloadWeapon(Event *ev);
		void				AnimateWeapon(Event *ev );
		void				SwitchWeaponMode(Event *ev);

		void				UseSpecifiedEntity( Event *ev );

		void				SetupDialog( Event *ev );
		void				SetupDialog( Entity *entity, const str &soundName );

		void				handleTextDialogSetup( const str& soundName );
		void				handleDialogSetup( Entity* entity, const str& soundName );

		void				ClearDialog( Event *ev );
		void				ClearDialog( void );

		void				ClearTextDialog( Event* ev );
		void				ClearTextDialog( void );

		// Interface to the projdetonate member variable
		// Used for the grenade launcher, which has a detonate trigger
		qboolean			GetProjDetonate();
		void				SetProjDetonate(qboolean value);
		void				ProjDetonate(Event *ev);

		//Player Heuristics
		PlayerHeuristics	*p_heuristics;
		void				ShowHeuristics( Event *ev );

		void				ReloadTiki( Event *ev );

		//Objective Updates
		void				LoadObjectives( Event* ev );
		void				SetObjectiveComplete( Event *ev );
		void				SetObjectiveFailed( Event *ev );
		void				SetObjectiveShow( Event *ev );

		void				loadObjectives( const str& objectiveName );

		void				MissionFailed(Event* ev);
		void				setMissionFailed( void );

		//Information Updates
		void				SetInformationShow( Event *ev );
		void				SetStat( Event *ev );
		qboolean			ShouldSendToClient( Entity *entityToSend );
		void				UpdateEntityStateForClient( entityState_t *state );
		void				UpdatePlayerStateForClient( playerState_t *state );
		void				ExtraEntitiesToSendToClient( int *numExtraEntities, int *extraEntities );

		virtual void		setViewMode( const str &viewModeName );

		void				applyWeaponSpeedModifiers( int *moveSpeed );

		float				GetDamageMultiplier() { return damage_multiplier; }

		// Special Moves
		void				SpecialMoveChargeStart( Event *ev );
		void				SpecialMoveChargeEnd( Event *ev );
		void				SpecialMoveChargeTime( Event *ev );

		// Point System
		int 				AwardPoints(int numPoints);
		int 				TakePoints(int numPoints);
		void				ClearPoints() { points = 0; }
		int 				GetNumPoints() { return points; }

		void				ChangeChar( Event *ev );
		void				SetPlayerChar( Event *ev );
		void				PlayerKnockback( Event *ev );
		void				KnockbackMultiplier( Event *ev );
		void				MeleeEvent( Event *ev );
		float				GetKnockbackMultiplier() { return knockbackMultiplier; }
		float				GetPlayerKnockback() { return playerKnockback; }

		Vector				getWeaponViewShake( void );
		void				MeleeDamageStart( Event *ev );
		void				MeleeDamageEnd( Event *ev );
		void				AdvancedMeleeAttack( weaponhand_t hand );
		void				ChangeCharFadeIn( Event *ev );
		void				AddMeleeAttacker( Event *ev );
		void				SetIncomingMeleeAttack( bool flag ) { incomingMeleeAttack = flag; }
		void				ClearIncomingMelee( Event *ev );
		void				SetBendTorso( Event *ev );
		void				HeadWatchAllowed( Event *ev );

		float				getDamageDone( float damage, int meansOfDeath, bool inMelee );
		meansOfDeath_t		changetMeansOfDeath( meansOfDeath_t meansOfDeath );

		// Powerup stuff
		void				setPowerup( Powerup *powerup );
		void				removePowerup( void );
		void				removePowerupEvent( Event *ev );
		void				dropPowerup( void );

		// Rune Stuff
		bool				hasRune( void );
		void				setRune( Rune *rune );
		void				removeRune( void );
		void				dropRune( Event *ev );
		void				dropRune( void );

		// Holdable item Stuff
		void				setHoldableItem( HoldableItem *holdableItem );
		void				removeHoldableItem( void );
		void				useHoldableItem( void );
		HoldableItem*		getHoldableItem(void);

		// Energy Transfer
		void				transferEnergy( void );
		void				setCanTransferEnergy( Event * );

		void				setDoDamageScreenFlash( Event *ev );
		void				pointOfView( Event *ev );

		Entity *			FindClosestEntityInRadius( const float horizontalFOVDegrees, const float verticalFOVDegrees, const float maxDistance );
		Entity*				FindHeadTarget( const Vector &origin, const Vector &forward, const float fov, const float maxdist );
		void				HandleFinishableList();
		void				addFinishingMove( Event *ev );
		void				clearFinishingMove( Event *ev );
		void				doFinishingMove( Event *ev );
		void				forceTimeScale( Event *ev );
		void				freezePlayer( Event *ev );
		void				immobilizePlayer( Event *ev );
		void				doUseEntity( Event *ev );
		void				doneUseEntity( Event *ev );
		void				showObjectInfo();

		void				setAutoSwitchWeapons( bool autoSwitchWeapons ) { _autoSwitchWeapons = autoSwitchWeapons; }
		bool				getAutoSwitchWeapons( void ) { return _autoSwitchWeapons; }

		void				handleUseObject(UseObject *uo);
		void				handleUseEntity(Entity *ent, float v_dist);
		void				handlePickupItem(Item *item);
		void				clearActionType();
		void				setAttackType(Event *ev);

		const str&			getAttackType() { return _attackType; }
		const str			getGameplayAnim(const str& objname);

		void				nextGameplayAnim( Event *ev );
		void				setGameplayAnim( Event *ev );
		void				setDisableUseWeapon( Event* ev );
		void				setDisableInventory( Event* ev );
		void				skillChanged( const str& objname );
		void				equipItems( Event *ev );
		const str			getFreeInventorySlot();

		void				usePlayer( Event * );


		void				addRosterTeammate1( Event* ev );
		void				addRosterTeammate2( Event* ev );
		void				addRosterTeammate3( Event* ev );
		void				addRosterTeammate4( Event* ev );

		void				removeRosterTeammate1( Event* ev );
		void				removeRosterTeammate2( Event* ev );
		void				removeRosterTeammate3( Event* ev );
		void				removeRosterTeammate4( Event* ev );

		bool				isButtonDown( int button );

		void				notifyPlayerOfMultiplayerEvent( const char *eventName, const char *eventItemName, Player *eventPlayer );

		void				touchingLadder( Trigger *ladder, const Vector &normal, float top );

//		Vector				GetViewEndPoint( void );
		void				GetViewTrace( trace_t& trace, int contents, float maxDistance = 3000.0f );

		void				addPowerupEffect( PowerupBase *powerup );
		void				removePowerupEffect( PowerupBase *powerup );

		void				warp( Event *ev );

		void				hudPrint( Event *ev );
		void				hudPrint( const str &string );

		void				setTargeted( bool targeted );

		void				setItemText( int itemIcon, const str &itemText );
		void				clearItemText( void );
		void				clearItemText( Event *ev );

		void				setVoteText( const str &voteText );
		void				clearVoteText( void );

		void				shotFired( void );
		void				shotHit( void );

		void				cinematicStarted( void );
		void				cinematicStopped( void );

		void				loadUseItem( const str &item );

		void				setValidPlayerModel( Event *ev );

		void				incrementSecretsFound( void );
		void				isPlayerOnGround( Event *ev );

		void				addHud( Event *ev );
		void				addHud( const str &hudName );
		void				removeHud( Event *ev );
		void				removeHud( const str &hudName );
		bool				needToSendAllHudsToClient( void );
		void				sendAllHudsToClient( void );
		void				clearAllHuds( void );
		void				addHudToClient( const str &hudName );
		void				removeHudFromClient( const str &hudName );

		void				killAllDialog(Event *ev);
		void				killAllDialog();

		void				clearTempAttachments( void );

		void				setSkill( int skill );
		int					getSkill( void );

		void				forceMoveType( Event *ev );

		void				setBranchDialogActor( const Actor* actor);
		void				clearBranchDialogActor( void );

		void				setBackpackAttachOffset( Event *ev );
		void				setBackpackAttachAngles( Event *ev );

		Vector				getBackpackAttachOffset( void );
		Vector				getBackpackAttachAngles( void );

		void				setFlagAttachOffset( Event *ev );
		void				setFlagAttachAngles( Event *ev );

		Vector				getFlagAttachOffset( void );
		Vector				getFlagAttachAngles( void );

		bool				canRegenerate( void );

		void				modelChanged( void );

		void				setBackupModel( Event *ev );
		void				setBackupModel( const str &modelName );
	};

inline bool Player::IsNewActiveWeapon()
   {
   return ( newActiveWeapon.weapon != NULL );
   }

inline weaponhand_t Player::GetNewActiveWeaponHand()
   {
   return newActiveWeapon.hand;
   }

inline Weapon *Player::GetNewActiveWeapon()
   {
   return newActiveWeapon.weapon;
   }

inline void Player::ClearNewActiveWeapon()
   {
   newActiveWeapon.weapon = NULL;
   newActiveWeapon.hand   = WEAPON_ERROR;
   }

inline void Player::Archive( Archiver &arc )
	{
	str tempStr;
	int i, num;
	WeaponSetItem *tempDualWeapon;

	Sentient::Archive( arc );

	// Don't archive
	//static Condition<Player> Conditions[];
	//static movecontrolfunc_t MoveStartFuncs[];

	// make sure we have the state machine loaded up
	if ( arc.Loading() )
		{
		LoadStateTable();
		}

	if ( arc.Saving() )
		{
		if ( currentState_Legs )
			tempStr = currentState_Legs->getName();
		else
			tempStr = "NULL";

		arc.ArchiveString( &tempStr );

		if ( currentState_Torso )
			tempStr = currentState_Torso->getName();
		else
			tempStr = "NULL";

		arc.ArchiveString( &tempStr );
		}
	else
		{
		arc.ArchiveString( &tempStr );

		if ( tempStr != "NULL" )
			currentState_Legs = statemap_Legs->FindState( tempStr );
		else
			currentState_Legs = NULL;

		arc.ArchiveString( &tempStr );

		if ( tempStr != "NULL" )
			currentState_Torso = statemap_Torso->FindState( tempStr );
		else
			currentState_Torso = NULL;
		}

	arc.ArchiveString( &last_torso_anim_name );
	arc.ArchiveString( &last_leg_anim_name );

	ArchiveEnum( movecontrol, movecontrol_t );

	arc.ArchiveInteger( &last_camera_type );
	if ( arc.Loading() )
		{
		// make sure the camera gets reset
		last_camera_type = -1;
		}

	newActiveWeapon.Archive( arc );

	arc.ArchiveSafePointer( &head_target );
	arc.ArchiveFloat( &look_at_time );

	arc.ArchiveSafePointer( &targetEnemy );
	arc.ArchiveBool( &targetEnemyLocked );

	arc.ArchiveBoolean( &shield_active );

	arc.ArchiveBoolean( &dual_wield_active );

	if ( arc.Saving() )
		num = dual_wield_weaponlist.NumObjects();
	else
		dual_wield_weaponlist.ClearObjectList();

	arc.ArchiveInteger( &num );

	for( i = 1; i <= num; i++ )
		{
		if ( arc.Saving() )
			{
			tempDualWeapon = dual_wield_weaponlist.ObjectAt( i );
			}
		else
			{
			tempDualWeapon = new WeaponSetItem;
			dual_wield_weaponlist.AddObject( tempDualWeapon );
			}
		tempDualWeapon->Archive( arc );
		}

	for( i = 0; i < MAX_ACTIVE_WEAPONS; i++ )
		{
		arc.ArchiveSafePointer( &holsteredWeapons[ i ] );
		}

	arc.ArchiveString( &partAnim[ 0 ] );
	arc.ArchiveString( &partAnim[ 1 ] );
	arc.ArchiveString( &partAnim[ 2 ] );

	arc.ArchiveBool( &animdone_Legs );
	arc.ArchiveBool( &animdone_Torso );

	arc.ArchiveVector( &oldvelocity );
	arc.ArchiveVector( &old_v_angle );
	arc.ArchiveVector( &oldorigin );
	arc.ArchiveFloat( &animspeed );
	arc.ArchiveFloat( &airspeed );

	arc.ArchiveString( &currentCallVolume );

	arc.ArchiveRaw( blend, sizeof( blend ) );
	arc.ArchiveFloat( &fov );
	arc.ArchiveFloat( &_userFovChoice );

	arc.ArchiveSafePointer( &vehicle );
	arc.ArchiveVector( &v_angle );

	arc.ArchiveInteger( &buttons );
	arc.ArchiveInteger( &new_buttons );
	arc.ArchiveFloat( &respawn_time );

	arc.ArchiveInteger( &last_attack_button );

	arc.ArchiveFloat( &damage_blood );
	arc.ArchiveFloat( &damage_alpha );
	arc.ArchiveVector( &damage_blend );
	arc.ArchiveVector( &damage_from );
	arc.ArchiveVector( &damage_angles );
	arc.ArchiveFloat( &damage_count );

	arc.ArchiveFloat( &bonus_alpha );

	arc.ArchiveFloat( &next_drown_time );
	arc.ArchiveFloat( &next_painsound_time );
	arc.ArchiveFloat( &air_finished );

	arc.ArchiveInteger( &old_waterlevel );
	arc.ArchiveFloat( &drown_damage );

	arc.ArchiveFloat( &_flashAlpha );
	arc.ArchiveVector( &_flashBlend );
	arc.ArchiveFloat( &_flashMaxTime );
	arc.ArchiveFloat( &_flashMinTime );

	arc.ArchiveString( &waitForState );

	arc.ArchiveSafePointer( &camera );
	arc.ArchiveSafePointer( &actor_camera );
	arc.ArchiveSafePointer( &cool_camera );
	arc.ArchiveSafePointer( &entity_to_watch );
	arc.ArchiveFloat( &maximumAngleToWatchedEntity );
	arc.ArchiveBool( &watchEntityForEntireDuration );
	arc.ArchiveInteger( &playerCameraMode );

	arc.ArchiveFloat( &action_level );
	arc.ArchiveInteger( &music_current_mood );
	arc.ArchiveInteger( &music_fallback_mood );
	arc.ArchiveFloat( &music_current_volume );
	arc.ArchiveFloat( &music_saved_volume );
	arc.ArchiveFloat( &music_volume_fade_time );
	arc.ArchiveInteger( &reverb_type );
	arc.ArchiveFloat( &reverb_level );
	arc.ArchiveBoolean( &music_forced );
	arc.ArchiveBool( &_allowMusicDucking );
	arc.ArchiveBool( &_allowActionMusic );

	arc.ArchiveBoolean( &gibbed );
	arc.ArchiveFloat( &pain );

	ArchiveEnum( pain_dir, painDirection_t );
	ArchiveEnum( pain_type, meansOfDeath_t );
	arc.ArchiveBool( &take_pain );

	arc.ArchiveFloat( &accumulated_pain );
	arc.ArchiveFloat( &nextpaintime );

	arc.ArchiveBool( &knockdown );
	arc.ArchiveBool( &canfall );
	arc.ArchiveBool( &falling );
	arc.ArchiveInteger( &feetfalling );
	arc.ArchiveVector( &falldir );

	arc.ArchiveBool( &hardimpact );

	arc.ArchiveRaw( &last_ucmd, sizeof( last_ucmd ) );

	//arc.ArchiveVector( &base_righthand_pos );
	//arc.ArchiveVector( &base_lefthand_pos );
	//arc.ArchiveVector( &righthand_pos );
	//arc.ArchiveVector( &lefthand_pos );

	arc.ArchiveVector( &base_rightfoot_pos );
	arc.ArchiveVector( &base_leftfoot_pos );
	arc.ArchiveVector( &rightfoot_pos );
	arc.ArchiveVector( &leftfoot_pos );

	arc.ArchiveBool( &_onLadder );
	arc.ArchiveVector( &_ladderNormal );
	arc.ArchiveFloat( &_ladderTop );
	arc.ArchiveFloat( &_nextLadderTime );

	arc.ArchiveInteger( &pm_lastruntime );
	arc.ArchiveFloat( &animheight );

	arc.ArchiveVector( &yaw_forward );
	arc.ArchiveVector( &yaw_left );

	arc.ArchiveSafePointer( &atobject );
	arc.ArchiveFloat( &atobject_dist );
	arc.ArchiveVector( &atobject_dir );

	arc.ArchiveSafePointer( &toucheduseanim );
	arc.ArchiveInteger( &useanim_numloops );
	arc.ArchiveSafePointer( &useitem_in_use );
	arc.ArchiveSafePointer( &cool_item );
	arc.ArchiveString( &cool_dialog );
	arc.ArchiveString( &cool_anim );

	arc.ArchiveInteger( &moveresult );

	arc.ArchiveBoolean( &do_rise );
	arc.ArchiveBoolean( &weapons_holstered_by_code );

	arc.ArchiveBoolean( &projdetonate );

	arc.ArchiveBoolean( &yawing );
	arc.ArchiveBoolean( &yawing_left );
	arc.ArchiveBoolean( &yawing_right );
	arc.ArchiveBoolean( &adjust_torso );

	arc.ArchiveVector( &torsoAngles );
	arc.ArchiveVector( &headAngles );
	arc.ArchiveVector( &headAimAngles );
	arc.ArchiveVector( &torsoAimAngles );

	arc.ArchiveFloat( &damage_multiplier );

	// Don't save multiplayer stuff

	arc.ArchiveBoolean( &fakePlayer_active );

	arc.ArchiveSafePointer( &fakePlayer );

	arc.ArchiveBool( &dont_turn_legs );
	arc.ArchiveFloat(&specialMoveCharge);
	arc.ArchiveFloat(&specialMoveEndTime);
	arc.ArchiveFloat(&specialMoveChargeTime);
	arc.ArchiveInteger(&points);
	arc.ArchiveFloat(&playerKnockback);
	arc.ArchiveFloat(&knockbackMultiplier);
	arc.ArchiveBool(&changedStanceTorso);
	arc.ArchiveBool(&changedStanceLegs);
	arc.ArchiveInteger(&stanceNumber);
	arc.ArchiveBool(&incomingMeleeAttack);
	arc.ArchiveFloat(&bendTorsoMult);
	arc.ArchiveString(&lastActionType);
	arc.ArchiveInteger(&meleeAttackFlags);
	arc.ArchiveBool(&changingChar);

	int numEntries;
	if ( arc.Saving() )
		{
		numEntries = meleeAttackerList.NumObjects();
		arc.ArchiveInteger( &numEntries );

		EntityPtr eptr;
		for ( int i = 1 ; i <= numEntries ; i++ )
			{
			eptr = meleeAttackerList.ObjectAt( i );
			arc.ArchiveSafePointer( &eptr );			
			}
		}
	else
		{
		EntityPtr eptr;
		EntityPtr *eptrptr;
		arc.ArchiveInteger( &numEntries );

		meleeAttackerList.Resize( numEntries );

		for ( int i = 1 ; i <= numEntries ; i++ )
			{
			meleeAttackerList.AddObject( eptr );
			eptrptr = &meleeAttackerList.ObjectAt( i );
			arc.ArchiveSafePointer( eptrptr );		
			}
		}

	// Don't archive
	//Container<Conditional *>	legs_conditionals;
	//Container<Conditional *>	torso_conditionals;

	if ( arc.Saving() )
		{
		numEntries = finishableList.NumObjects();
		arc.ArchiveInteger( &numEntries );

		ActorPtr eptr;
		for ( int i = 1 ; i <= numEntries ; i++ )
			{
			eptr = finishableList.ObjectAt( i );
			arc.ArchiveSafePointer( &eptr );			
			}
		}
	else
		{
		ActorPtr eptr;
		ActorPtr *eptrptr;
		arc.ArchiveInteger( &numEntries );

		finishableList.Resize( numEntries );

		for ( int i = 1 ; i <= numEntries ; i++ )
			{
			finishableList.AddObject( eptr );
			eptrptr = &finishableList.ObjectAt( i );
			arc.ArchiveSafePointer( eptrptr );		
			}
		}

	if ( arc.Saving() )
		{
		numEntries = finishingMoveList.NumObjects();
		arc.ArchiveInteger( &numEntries );

		FinishingMove * eptr;
		for ( int i = 1 ; i <= numEntries ; i++ )
			{
			eptr = finishingMoveList.ObjectAt( i );
			arc.ArchiveObjectPointer( (Class **)&eptr );			
			}
		}
	else
		{
		FinishingMove *eptr = NULL;
		FinishingMove **eptrptr;
		arc.ArchiveInteger( &numEntries );

		finishingMoveList.Resize( numEntries );

		for ( int i = 1 ; i <= numEntries ; i++ )
			{
			finishingMoveList.AddObject( eptr );
			eptrptr = &finishingMoveList.ObjectAt( i );
			arc.ArchiveObjectPointer( (Class **)eptrptr );		
			}
		}

	arc.ArchiveSafePointer( &_powerup );
	arc.ArchiveSafePointer( &_rune );
	arc.ArchiveSafePointer( &_holdableItem );

	arc.ArchiveFloat( &_nextEnergyTransferTime );
	arc.ArchiveBool( &_canTransferEnergy );
	arc.ArchiveBool( &_doDamageScreenFlash );
	arc.ArchiveBool( &_isThirdPerson );
	arc.ArchiveObjectPointer( (Class **)&_finishActor );
	arc.ArchiveString( &_finishState );
	arc.ArchiveBool( &_doingFinishingMove );

	arc.ArchiveBool( &_autoSwitchWeapons );
	
	arc.ArchiveBool( &_usingEntity );
	arc.ArchiveString( &_attackType );
	arc.ArchiveInteger( &_gameplayAnimIdx );

	arc.ArchiveBool( &_disableUseWeapon );
	arc.ArchiveSafePointer( &_targetSelectedHighlight );
	arc.ArchiveSafePointer( &_targetLockedHighlight );
	arc.ArchiveBool( &_infoHudOn );
	arc.ArchiveInteger( &_nextRegenTime );
	arc.ArchiveFloat( &_useEntityStartTimer );

	//mission objective stuff
	str objectiveName;
	if(	arc.Saving() )
	{
		objectiveName = gi.getConfigstring( CS_OBJECTIVE_NAME + _objectiveNameIndex);
		arc.ArchiveString( &objectiveName );
	}

	if( arc.Loading() )
	{
		arc.ArchiveString( &objectiveName );
		loadObjectives(objectiveName);
	}

	arc.ArchiveUnsigned( &_objectiveStates   );
	arc.ArchiveUnsigned( &_informationStates );

	arc.ArchiveSafePointer( &_targetedEntity);

	//dialog entity string and dialog num
	str dialogSound;
	str dialogTextSound;
	if(arc.Saving() )
	{
		arc.ArchiveInteger(&_dialogEntnum);
		dialogSound = gi.getConfigstring( CS_SOUNDS + _dialogSoundIndex);
		arc.ArchiveString( &dialogSound );

		dialogTextSound = gi.getConfigstring( CS_SOUNDS + _dialogTextSoundIndex );
		arc.ArchiveString( &dialogTextSound );
	}

	if( arc.Loading() )
	{
		arc.ArchiveInteger(&_dialogEntnum);
		arc.ArchiveString( &dialogSound );
		_dialogSoundIndex = gi.soundindex( dialogSound );

		arc.ArchiveString( &dialogTextSound );
		_dialogTextSoundIndex = gi.soundindex( dialogTextSound );
	}
	
	
	arc.ArchiveFloat( &_crossHairXOffset );
	arc.ArchiveFloat( &_crossHairYOffset );

	arc.ArchiveFloat( &_lastDamagedTimeFront );
	arc.ArchiveFloat( &_lastDamagedTimeBack );
	arc.ArchiveFloat( &_lastDamagedTimeLeft );
	arc.ArchiveFloat( &_lastDamagedTimeRight );

	arc.ArchiveInteger(&_totalGameFrames);

	arc.ArchiveFloat( &_nextPainShaderTime );
	ArchiveEnum( _lastPainShaderMod, meansOfDeath_t );

	arc.ArchiveInteger( &_itemIcon );
	arc.ArchiveString( &_itemText );

	arc.ArchiveString( &_voteText );

	arc.ArchiveBool( &_validPlayerModel );

	arc.ArchiveInteger( &_secretsFound );

	_hudList.Archive( arc );

	// _needToSendHuds is not archived but set directly

	if ( arc.Loading() && ( _hudList.NumObjects() > 0 ) )
		_needToSendHuds = true;
	else
		_needToSendHuds = false;

	// _started is not archived but set directly
	_started = false;

	arc.ArchiveInteger( &_skillLevel );

	ArchiveEnum( _forcedMoveType, pmtype_t );

	arc.ArchiveSafePointer( &_branchDialogActor);
	_needToSendBranchDialog = true;

	// Don't save these 4, only used in multiplayer

	//Vector				_backpackAttachOffset;
	//Vector				_backpackAttachAngles;
	//Vector				_flagAttachOffset;
	//Vector				_flagAttachAngles;

	arc.ArchiveBool( &_cameraCutThisFrame );
	}

inline Camera *Player::CurrentCamera()
	{
	return camera;
	}

inline void Player::CameraCut()
	{
	// toggle the camera cut bit
	if ( !client )
		return;

	if ( _cameraCutThisFrame )
		return;

	client->ps.camera_flags =
		( ( client->ps.camera_flags & CF_CAMERA_CUT_BIT ) ^ CF_CAMERA_CUT_BIT ) |
		( client->ps.camera_flags & ~CF_CAMERA_CUT_BIT );

	_cameraCutThisFrame = true;
	}

inline void Player::CameraCut( Camera * ent )
	{
	if ( ent == camera )
		{
		// if the camera we are currently looking through cut, than toggle the cut bits
		CameraCut();
		}
	}

inline void Player::SetCamera( Camera *ent, float switchTime )
	{
	if ( !client )
		return;

	camera = ent;
	client->ps.camera_time = switchTime;
	if ( switchTime <= 0.0f )
		{
		CameraCut();
		}
	}

#endif /* player.h */
