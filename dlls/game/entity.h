//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/entity.h                                      $
// $Revision:: 124                                                            $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:43a                                                 $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Base class for all enities that are controlled by the game.  If you have any
// object that should be called on a periodic basis and it is not an entity,
// then you have to have an dummy entity that calls it.
//
// An entity in the game is any object that is not part of the world.  Any non-world
// object that is visible in the game is an entity, although it is not required that
// all entities be visible to the player.  Some objects are basically just virtual
// constructs that act as an instigator of certain actions, for example, some
// triggers are invisible and cannot be touched, but when activated by other
// objects can cause things to happen.
//
// All entities are capable of receiving messages from Sin or from other entities.
// Messages received by an entity may be ignored, passed on to their superclass,
// or acted upon by the entity itself.  The programmer must decide on the proper
// action for the entity to take to any message.  There will be many messages
// that are completely irrelevant to an entity and should be ignored.  Some messages
// may require certain states to exist and if they are received by an entity when
// it these states don't exist may indicate a logic error on the part of the
// programmer or map designer and should be reported as warnings (if the problem is
// not severe enough for the game to be halted) or as errors (if the problem should
// not be ignored at any cost).
//

class BindInfo;
class MorphInfo;
class Entity;

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "program.h"
#include "g_local.h"
#include "class.h"
#include "vector.h"
#include "script.h"
#include "listener.h"
#include "scriptvariable.h"
#include "DamageModification.hpp"
#include "UseData.h"

// modification flags
#define FLAG_IGNORE  0
#define FLAG_CLEAR   1
#define FLAG_ADD     2

typedef enum
	{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
	} damage_t;

//deadflag
#define DEAD_NO						0
#define DEAD_DYING					1
#define DEAD_DEAD					2
#define DEAD_RESPAWNABLE			3

// flags
#define	FL_FLY					(1<<0)
#define	FL_SWIM					(1<<1)      // implied immunity to drowining
#define	FL_INWATER				(1<<2)
#define	FL_GODMODE				(1<<3)
#define	FL_NOTARGET				(1<<4)
#define	FL_PARTIALGROUND		(1<<5)      // not all corners are valid
#define	FL_TEAMSLAVE			(1<<6)      // not the first on the team
#define	FL_NO_KNOCKBACK			(1<<7)
#define	FL_THINK				(1<<8)
#define FL_BLOOD				(1<<9)      // when hit, it should bleed.
#define FL_DIE_GIBS				(1<<10)     // when it dies, it should gib
#define FL_DIE_EXPLODE			(1<<11)     // when it dies, it will explode
#define FL_ROTATEDBOUNDS		(1<<12)     // model uses rotated mins and maxs
#define FL_DONTSAVE				(1<<13)     // don't add to the savegame
#define FL_IMMOBILE				(1<<14)     // entity has been immobolized somehow
#define FL_PARTIAL_IMMOBILE		(1<<15)     // entity has been immobolized somehow
#define FL_STUNNED				(1<<16)
#define FL_POSTTHINK			(1<<17)     // call a think function after the physics have been run
#define FL_TOUCH_TRIGGERS		(1<<18)     // should this entity touch triggers
#define FL_AUTOAIM				(1<<19)     // Autoaim on this entity

// Create falgs

#define ENTITY_CREATE_FLAG_ANIMATE	(1<<0)
#define ENTITY_CREATE_FLAG_MOVER	(1<<1)


// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_NO_SKILL			0x00000040  // damage is not affected by skill level

extern Event EV_ClientMove;
extern Event EV_ClientEndFrame;

// Generic entity events
extern Event EV_Classname;
extern Event EV_Activate;
extern Event EV_Use;
extern Event EV_FadeNoRemove;
extern Event EV_FadeOut;
extern Event EV_FadeIn;
extern Event EV_Fade;
extern Event EV_Killed;
extern Event EV_GotKill;
extern Event EV_Pain;
extern Event EV_Damage;
extern Event EV_Stun;
extern Event EV_Gib;
extern Event EV_Kill;
extern Event EV_DeathSinkStart;
extern Event EV_TouchTriggers;
extern Event EV_DoRadiusDamage;

// Physics events
extern Event EV_MoveDone;
extern Event EV_Touch;
extern Event EV_Contact;
extern Event EV_LostContact;
extern Event EV_Blocked;
extern Event EV_Attach;
extern Event EV_AttachModel;
extern Event EV_RemoveAttachedModel;
extern Event EV_RemoveAttachedModelByTargetname;
extern Event EV_Detach;
extern Event EV_UseBoundingBox;

// Animation events
extern Event EV_NewAnim;
extern Event EV_LastFrame;
extern Event EV_TakeDamage;
extern Event EV_NoDamage;
extern Event EV_SetCinematicAnim ;

// script stuff
extern Event EV_Model;
extern Event EV_Hide;
extern Event EV_Show;
extern Event EV_BecomeSolid;
extern Event EV_BecomeNonSolid;
extern Event EV_Sound;
extern Event EV_StopSound;
extern Event EV_Bind;
extern Event EV_Unbind;
extern Event EV_JoinTeam;
extern Event EV_QuitTeam;
extern Event EV_SetHealth;
extern Event EV_SetMaxHealth;
extern Event EV_SetSize;
extern Event EV_SetAlpha;
extern Event EV_SetOrigin;
extern Event EV_Warp;
extern Event EV_SetTargetName;
extern Event EV_SetTarget;
extern Event EV_SetKillTarget;
extern Event EV_SetAngles;
extern Event EV_SetAngle;
extern Event EV_RegisterAlias;
extern Event EV_Anim;
extern Event EV_StartAnimating;
extern Event EV_SurfaceModelEvent;
extern Event EV_ProcessInitCommands;
extern Event EV_Stop;
extern Event EV_StopLoopSound;
extern Event EV_SetControllerAngles;
extern Event EV_DisplayEffect;
extern Event EV_ForceAlpha;

extern Event EV_SetFloatVar;
extern Event EV_SetVectorVar;
extern Event EV_SetStringVar;

extern Event EV_GetFloatVar;
extern Event EV_GetVectorVar;
extern Event EV_GetStringVar;

extern Event EV_RemoveVariable;
extern Event EV_DoesVarExist;


extern Event EV_UseDataAnim;
extern Event EV_UseDataType;
extern Event EV_UseDataThread;
extern Event EV_UseDataMaxDist;
extern Event EV_UseData;

extern Event EV_CreateEarthquake;
extern Event EV_SpawnEffect;

// Morph stuff

extern Event EV_Morph;
extern Event EV_Unmorph;

// dir is 1
// power is 2
// minsize is 3
// maxsize is 4
// percentage is 5
// thickness 6
// entity is 7
// origin 8

// AI sound events
extern Event EV_BroadcastSound;
extern Event EV_HeardSound;
extern Event EV_Hurt;
extern Event EV_IfSkill;

extern Event EV_SetArchetype;
extern Event EV_SetGameplayHealth;
extern Event EV_SetGameplayDamage;
extern Event EV_ProcessGameplayData;

extern Event EV_StopStasis;

extern Event EV_AddHealthOverTime;

extern Event EV_SimplePlayDialog;
// Define ScriptMaster
class ScriptMaster;

//
// Spawn args
//
// "spawnflags"
// "alpha" default 1.0
// "model"
// "origin"
// "targetname"
// "target"
//
#define MAX_MODEL_CHILDREN 8

class Entity;
class Animate;
class Mover;

class BindInfo
	{
	public:	

	// Model Binding variables
	int				numchildren;
	int				children[MAX_MODEL_CHILDREN];

	// Team variables
	str				moveteam;
	Entity			*teamchain;
	Entity 			*teammaster;

	// Binding variables
	Entity 			*bindmaster;
	qboolean		bind_use_my_angles;
	//Vector		localorigin;
	//Vector		localangles;

	qboolean		detach_at_death;

	BindInfo();

	void			Archive( Archiver &arc );
	};

inline BindInfo::BindInfo()
	{
	int i;

	// model binding variables
	numchildren = 0;

	for( i = 0 ; i < MAX_MODEL_CHILDREN ; i++ )
		children[i] = ENTITYNUM_NONE;

	detach_at_death = true;

	// team variables
	teamchain	= NULL;
	teammaster	= NULL;

	// bind variables
	bindmaster = NULL;

	bind_use_my_angles = false;
	}

inline void BindInfo::Archive(Archiver &arc)
	{
	arc.ArchiveInteger( &numchildren );
	arc.ArchiveRaw( children, sizeof( children ) );
	arc.ArchiveString( &moveteam );
	arc.ArchiveObjectPointer( ( Class ** )&teamchain );
	arc.ArchiveObjectPointer( ( Class ** )&teammaster );
	arc.ArchiveObjectPointer( ( Class ** )&bindmaster );
	arc.ArchiveBoolean( &bind_use_my_angles );
	arc.ArchiveBoolean( &detach_at_death );
	}

BindInfo *CreateBindInfo( void );

typedef struct 
	{
	int			index;
	float		current_percent;
	float		speed;
	float		final_percent;
	qboolean	return_to_zero;
	int			channel;
	} morph_t;

class MorphInfo
	{
	public:
		MorphInfo();

		morph_t		controllers[ NUM_MORPH_CONTROLLERS ];
		qboolean	controller_on;

		void		Archive( Archiver &arc );
	};

inline MorphInfo::MorphInfo()
	{
	int i;

	for( i = 0 ; i < NUM_MORPH_CONTROLLERS ; i++ )
		{
		controllers[ i ].index = -1;
		controllers[ i ].current_percent = 0.0;
		}

	controller_on = false;
	}

inline void MorphInfo::Archive(Archiver &arc)
	{
	int i;

	for( i = 0 ; i < NUM_MORPH_CONTROLLERS ; i++ )
		{
		arc.ArchiveInteger( &controllers[ i ].index );
		arc.ArchiveFloat( &controllers[ i ].current_percent );
		arc.ArchiveFloat( &controllers[ i ].speed );
		arc.ArchiveFloat( &controllers[ i ].final_percent );
		arc.ArchiveBoolean( &controllers[ i ].return_to_zero );
		arc.ArchiveInteger( &controllers[ i ].channel );
		}

	arc.ArchiveBoolean( &controller_on );
	}

MorphInfo *CreateMorphInfo( void );

typedef SafePtr<Entity> EntityPtr;

class Program;
class Entity : public Listener
	{
	private:
		Vector					_localOrigin;
		Container<EntityPtr>	_lastTouchedList ;
		bool					_fulltrace ;
		int						_groupID;
		str						_archetype;
		bool					_missionObjective;
		str						_targetPos;

		bool					_networkDetail;

	protected:
		void					buildUseData();

	public:
		CLASS_PROTOTYPE( Entity );

		// Construction / destruction
		Entity();
		Entity( int create_flag );
		virtual ~Entity();

		// Spawning variables
		int					entnum;
		gentity_t			*edict;
		gclient_t			*client;
		int					spawnflags;

		// Standard variables
		str					model;

		// Physics variables
		Vector				total_delta;   // total unprocessed movement
		Vector				mins;
		Vector				maxs;
		Vector				absmin;
		Vector				absmax;
		Vector				centroid;
		Vector				velocity;
		Vector				avelocity;
		Vector				origin;
		Vector				angles;
		Vector				size;
		int					movetype;
		int					mass;
		float				gravity;			// per entity gravity multiplier (1.0 is normal)
		float				orientation[3][3];
		Vector				localangles;

		// Ground variables
		gentity_t			*groundentity;
		cplane_t			groundplane;
		int					groundcontents;

		// Surface variables
		int					numsurfaces;

		// Light variables
		float				lightRadius;

		// Targeting variables
		str					target;
		str					targetname;
		str					killtarget;

		// Character state
		float				health;
		float				max_health;
		int					deadflag;
		int					flags;

		// underwater variables
		int					watertype;
		int					waterlevel;

		// Pain and damage variables
		damage_t			takedamage;
		int					damage_type;

		qboolean			look_at_me;
		bool				projectilesCanStickToMe;

		str					explosionModel;

		ScriptVariableList	entityVars;

		unsigned int		_affectingViewModes;
		Vector				watch_offset;

		// Pluggable modules

		Animate						*animate;
		Mover						*mover;
		BindInfo					*bind_info;
		MorphInfo					*morph_info;
		Program						*ObjectProgram;
		DamageModificationSystem	*damageModSystem;
		UseData						*useData;

		void				Setup();

		static Entity*		FindEntityByName( const str &entityName );
		void				SetEntNum( int num );
		void				ClassnameEvent( Event *ev );
		void				SpawnFlagsEvent( Event *ev );

		const Vector		InterceptTarget( const Vector &targetPosition, const Vector &targetVelocity, const float maxSpeed) const;
		const Vector		InterceptTargetXY( const Vector &targetPosition, const Vector &targetVelocity, const float maxSpeed) const;
		float				DistanceTo( const Vector &pos );
		float				DistanceTo( const Entity *ent );
		qboolean			WithinDistance( const Vector &pos, float dist );
		qboolean			WithinDistance( const Entity *ent, float dist );
		bool				WithinDistanceXY( const Vector &pos , float dist );
		bool				WithinDistanceXY( const Entity *ent , float dist );

		const char			*Target( void );
		void				SetTarget( const char *target );
		qboolean			Targeted( void );
		const char			*TargetName( void );
		void				SetTargetName( const char *target );
		void				SetKillTarget( const char *killtarget );
		const char			*KillTarget( void );

		const Vector &		GetLocalOrigin( void ) const { return _localOrigin; }
		void				SetLocalOrigin( const Vector &localOrigin ) { _localOrigin = localOrigin; }

		virtual void		setModel( const char *model );
		void		        setModel( const str &mdl );
		virtual void		setViewModel( const char *model );
		void		        setViewModel( const str &mdl );
		void				SetModelEvent( Event *ev );
		void				SetTeamEvent( Event *ev );
		virtual void		TriggerEvent( Event *ev );
		void				hideModel( void );
		void				EventHideModel( Event *ev );
		virtual void		showModel( void );
		void				EventShowModel( Event *ev );
		qboolean			hidden( void );
		void				ProcessInitCommandsEvent( Event *ev );
		void				ProcessInitCommands( int index, qboolean cache = false );

		void				setAlpha( float alpha );
		float				alpha( void );

		void				setMoveType( int type );
		void				setMoveType( Event *ev );

		int					getMoveType( void );

		void				setSolidType( solid_t type );
		int					getSolidType( void );

		virtual Vector		getParentVector( const Vector &vec );
		Vector				getLocalVector( const Vector &vec );

		virtual void		setSize( Vector min, Vector max );
		virtual void		setOrigin( const Vector &org );
		virtual void		setOrigin( void );
		virtual void		addOrigin( const Vector &org );
		virtual void		setOriginEveryFrame( Event *ev );		

		void				GetRawTag( int tagnum, orientation_t * orient, bodypart_t part = legs );
		qboolean			GetRawTag( const char * tagname, orientation_t * orient, bodypart_t part = legs );

		void				GetTag( int tagnum, orientation_t * orient );
		qboolean			GetTag( const char *name, orientation_t * orient );
		void  				GetTag( int tagnum, Vector *pos, Vector *forward = NULL, Vector *left = NULL, Vector *up = NULL );
		qboolean			GetTag( const char *name, Vector *pos, Vector *forward = NULL, Vector *left = NULL, Vector *up = NULL );

		virtual int			CurrentFrame( bodypart_t part = legs );
		virtual int			CurrentAnim( bodypart_t part = legs );

		virtual void		setAngles( const Vector &ang );
		virtual void		setAngles( void );
		virtual void		SetOrigin( Event *ev );
		void				GetOrigin( Event *ev );

		Vector				GetControllerAngles( int num );
		void				SetControllerAngles( int num, vec3_t angles );
		void				SetControllerAngles( Event *ev );
		void				SetControllerTag( int num, int tag_num );

		void				link( void );
		void				unlink( void );

		void				setContents( int type );
		int					getContents( void );
		void				setScale( float scale );

		qboolean			droptofloor( float maxfall );
		qboolean			isClient( void );

		virtual void		SetDeltaAngles( void );
		virtual void		DamageEvent( Event *event );

		void				Damage( Entity *inflictor,
									Entity *attacker,
									float damage,
									const Vector &position,
									const Vector &direction,
									const Vector &normal,
									int knockback,
									int flags,
									int meansofdeath,
									int surface_number = -1,
									int bone_number = -1,
									Entity *weapon = 0);

		void						Stun( float time );

		void				DamageType( Event *ev );
		virtual qboolean	CanDamage( const Entity *target, const Entity *skip_ent = NULL );

		qboolean			IsTouching( const Entity *e1 );

		void				FadeNoRemove( Event *ev );
		void				FadeOut( Event *ev );
		void				FadeIn( Event *ev );
		void				Fade( Event *ev );

		virtual void		CheckGround( void );
		virtual qboolean	HitSky( const trace_t *trace );
		virtual qboolean	HitSky( void );

		void				BecomeSolid( Event *ev );
		void				BecomeNonSolid( Event *ev );
		void				SetHealth( Event *ev );
		void				GetHealth( Event *ev );
		void				SetMaxHealth( Event *ev );
		void				SetSize( Event *ev );
		void				SetMins( Event *ev );
		void				SetMaxs( Event *ev );
		void				GetMins( Event* ev );
		void				GetMaxs( Event* ev );
		void				SetScale( Event *ev );
		void				setRandomScale( Event *ev );
		void				SetAlpha( Event *ev );
		void				SetTargetName( Event *ev );
		void				GetTargetName( Event *ev );
		void				GetRawTargetName( Event *ev );
		void				SetTarget( Event *ev );
		void				getTarget( Event *ev );
		void				GetTargetEntity( Event *ev );
		void				SetKillTarget( Event *ev );
		void				GetModelName(Event* ev);
		void				SetAngles( Event *ev );
		void				GetAngles(Event* ev);
		void				SetAngleEvent( Event *ev );
		void				TouchTriggersEvent( Event *ev );
		void				IncreaseShotCount( Event *ev );
		void				GetVelocity( Event *ev );
		void				SetVelocity( Event *ev );
		
		// Support for checking/setting fulltrace flag
		void				SetFullTraceEvent( Event *ev );
		void				setFullTrace( bool fulltrace )			{ _fulltrace = fulltrace; }
		bool				usesFullTrace()							{ return _fulltrace ; }

		Vector				GetClosestCorner( const Vector &position );

		str					GetRandomAlias( const str &name );
		void				SetWaterType( void );

		// model binding functions
		qboolean			attach( int parent_entity_num, int tag_num, qboolean use_angles = true, Vector attach_offset = Vector(0, 0, 0), Vector attach_angles_offset = Vector(0, 0, 0) );
		void				detach( void );

		void				RegisterAlias( Event *ev );
		void				RegisterAliasAndCache( Event *ev );
		void				Cache( Event *ev );

		qboolean			GlobalAliasExists( const char *name );
		qboolean			AliasExists( const char *name );

		// Sound Stuff
		void				Sound( Event *ev );
		virtual void		Sound( const str &sound_name, int channel = CHAN_BODY, float volume = -1.0f, float min_dist = -1.0f, Vector *origin = NULL, float pitch_modifier = 1.0f, qboolean onlySendToThisEntity = false );
		void				StopSound( int channel );
		void				StopSound( Event *ev );
		void				LoopSound( Event *ev );
		void				LoopSound( const str &sound_name, float volume = -1.0f, float min_dist = -1.0f );
		void				StopLoopSound( Event *ev );
		void				StopLoopSound( void );

		// Light Stuff
		void				SetLight(Event *ev);
		void				LightOn(Event *ev);
		void				LightOff(Event *ev);
		void				LightRed(Event *ev);
		void				LightGreen(Event *ev);
		void				LightBlue(Event *ev);
		void				LightRadius(Event *ev);
		void				LightStyle(Event *ev);
		void				Flags( Event *ev );
		void				Effects( Event *ev );
		void				RenderEffects( Event *ev );
		void				SVFlags( Event *ev );

		void				BroadcastSound( float pos = SOUND_RADIUS, int soundType = SOUNDTYPE_GENERAL );
		void				BroadcastSound( Event *ev );
		float				ModifyFootstepSoundRadius( float radius , int soundTypeIdx );
		void				Kill( Event *ev );
		void				SurfaceModelEvent( Event *ev );
		void				SurfaceCommand( const char * surf_name, const char * token );

		virtual void		Postthink( void );
		virtual void		Think( void );
		void				DamageSkin( trace_t * trace, float damage );

		void				AttachEvent( Event *ev );
		void				AttachModelEvent( Event *ev );
		void				RemoveAttachedModelEvent( Event *ev );
		void				removeAttachedModelByTargetname( Event *ev );
		void				removeAttachedModelByTargetname( const str &targetNameToRemove );
		void				DetachEvent( Event *ev );
		void				TakeDamageEvent( Event *ev );
		void				NoDamageEvent( Event *ev );
		void				Gravity( Event *ev );
		//void              GiveOxygen( float time );
		void				UseBoundingBoxEvent( Event *ev );
		void				HurtEvent( Event *ev );
		void				IfSkillEvent( Event *ev );
		void				SetMassEvent( Event *ev );
		void				Censor( Event *ev );
		void				Ghost( Event *ev );

		void				StationaryEvent( Event *ev );
		void				Explosion( Event *ev );
		void				SelfDetonate( Event *ev );
		void				DoRadiusDamage( Event *ev );

		void				Shader( Event *ev );

		void				KillAttach( Event *ev );
		//void					SetBloodModel( Event *ev );

		void				DropToFloorEvent( Event *ev );
		void				SetAnimOnAttachedModel( Event *ev );
		void				SetAnimOnAttachedModel( const str &AnimName, const str &TagName );
		void				SetEntityExplosionModel( Event *ev );

		virtual void		SetCinematicAnim( const str &AnimName);
		virtual void		SetCinematicAnim( Event *ev );
		virtual void		CinematicAnimDone( void );
		virtual void		CinematicAnimDone( Event *ev );

		// Binding methods
		void				joinTeam( Entity *teammember );
		void				quitTeam( void );
		qboolean			isBoundTo( const Entity *master );
		virtual void		bind( Entity *master, qboolean use_my_angles=false );
		virtual void		unbind( void );

		void				JoinTeam( Event *ev );
		void				EventQuitTeam( Event *ev );
		void				BindEvent( Event *ev );
		void				EventUnbind( Event *ev );
		void				AddToSoundManager( Event *ev );
		void				NoLerpThisFrame( void );

		virtual void		addAngles( const Vector &add );

		void				DeathSinkStart( Event *ev );
		void				DeathSink( Event *ev );

		void				LookAtMe( Event *ev );
		void				ProjectilesCanStickToMe( Event *ev );
		void				DetachAllChildren( Event *ev );

		void				MorphEvent( Event *ev );
		void				UnmorphEvent( Event *ev );
		void				MorphControl( Event *ev );
		int					GetMorphChannel( const char *morph_name );
		void				StartMorphController( void );
		qboolean			MorphChannelMatches(	int morph_channel1, int morph_channel2	);

		void				ProjectileAtk( Event *ev );
		void				ProjectileAttackPoint( Event *ev );
		void				ProjectileAttackEntity( Event *ev );
		void				ProjectileAttackFromTag( Event *ev );
		void				ProjectileAttackFromPoint( Event *ev );
		void				TraceAtk( Event *ev );

		virtual void		VelocityModified( void );
		virtual void		Archive( Archiver &arc );

		virtual void		PassToAnimate( Event *ev );
		void				SetObjectProgram( Event *ev );
		void				SetWatchOffset( Event *ev );
		void				ExecuteProgram( Event *ev );

		void				Contents(Event* ev);
		void				setMask(Event* ev);

		void				hideFeaturesForFade( void );
		void				showFeaturesForFade( void );

		void				DisplayEffect( Event *ev );
		void				clearDisplayEffects( void );

		void				getCustomShaderInfo( const str &customShader, str &shaderName, str &soundName );

		void				setCustomShader( const char *customShader );
		void				setCustomShader( Event *ev );
		void				clearCustomShader( const char *customShader = NULL );
		void				clearCustomShader( Event *ev );
		bool				hasCustomShader( const char *customShader = NULL );

		void				setCustomEmitter( const char *customEmitter );
		void				setCustomEmitter( Event *ev );
		void				clearCustomEmitter( const char *customEmitter = NULL );
		void				clearCustomEmitter( Event *ev );

		Entity				*SpawnEffect( const str &name, const Vector &origin, const Vector &angles, float removeTime );
		Entity				*SpawnSound( const str &sound, const Vector &pos, float volume , float removeTime );

		void				SpawnEffect( Event *ev );

		void				attachEffect( const str &modelName, const str &tagName, float removeTime );
		void				attachEffect( Event *ev );

		void				ForceAlpha( Event *ev );
		void				CreateEarthquake( Event *ev );

		void				SetFloatVar( Event *ev );
		void				SetVectorVar( Event *ev );
		void				SetStringVar( Event *ev );
		void				GetFloatVar( Event *ev );
		void				doesVarExist( Event *ev );
		void				RemoveVariable( Event* ev );
		void				GetVectorVar( Event *ev );
		void				GetStringVar( Event *ev );
		void				SetUserVar1( Event *ev );
		void				SetUserVar2( Event *ev );
		void				SetUserVar3( Event *ev );
		void				SetUserVar4( Event *ev );
		void				isWithinDistanceOf( Event *ev );

		void				affectingViewMode( Event *ev );
		void				addAffectingViewModes( unsigned int mask );
		void				removeAffectingViewModes( unsigned int mask );

		void				TikiNote( Event *ev );
		void				TikiTodo( Event *ev );

		void				AddDamageModifier( Event *ev );
		void				ResolveDamage( ::Damage &damage );

		// Health interface
		float				getHealth( void ) { return health; };
		float				getMaxHealth( void ) { return max_health; };
		void				setHealth( float newHealth ) { health = newHealth; };
		void				setMaxHealth( float newMaxHealth ) { max_health = newMaxHealth; };
		void				addHealth( float healthToAdd, float maxHealth = 0.0f );
		void				addHealthOverTime( Event *ev );

		// Group Number Interface
		void				SetGroupID(Event *ev);
		void				AddToGroup( int ID );
		void				SetGroupID(int ID) { _groupID = ID; }
		int					GetGroupID() { return _groupID; };

		void				MultiplayerEvent( Event *ev );

		Container<EntityPtr>&	GetLastTouchedList() { return _lastTouchedList ; }

		// Archetype name
		virtual void		setArchetype( Event *ev );
		const str			getArchetype() const;
		virtual const str	getName() const { return ""; }

		void				setMissionObjective(Event* ev);
		
		// Usable Entity functions
		void		useDataAnim( Event *ev );
		void		useDataType( Event *ev );
		void		useDataThread( Event *ev );
		void		useDataMaxDist( Event *ev );
		void		useDataCount( Event *ev );
		void		useDataEvent( Event *ev );
		bool		hasUseData(); 


		// GameplayManager interfaces to health and damage
		void			setGameplayHealth( Event *ev );
		void			setGameplayDamage( Event *ev );
		virtual void	processGameplayData( Event *ev ) {}

		// Think interface

		void			turnThinkOn( void ) { flags |= FL_THINK; }
		void			turnThinkOff( void ) { flags &= ~FL_THINK; }
		bool			isThinkOn( void ) { return (flags & FL_THINK) ? true : false; }

		void			startStasis( void );
		void			stopStasis( void );
		void			startStasis( Event *ev );
		void			stopStasis( Event *ev );
		void			setTargetPos( Event *ev );
		void			setTargetPos( const str &targetPos );
		str				getTargetPos();

		void			simplePlayDialog( Event *ev );

		void			warp( Event *ev );

		void			traceHitsEntity( Event *ev );

		void			setNetworkDetail( Event *ev );
		bool			isNetworkDetail( void );
	};

inline bool Entity::hasUseData()
	{
	if ( useData )
		return true;
	return false;
	}

inline int Entity::getSolidType()
	{
	return edict->solid;
	}

inline const Vector Entity::InterceptTarget( const Vector &targetPosition, const Vector &targetVelocity, const float maxSpeed) const
	{
	Vector myPosition (origin);
	Vector predictedPosition = targetPosition + targetVelocity * ( Vector::Distance(targetPosition, myPosition) / maxSpeed );
	Vector desiredDirection = predictedPosition - myPosition;
	
	return desiredDirection.toAngles();
	}

inline const Vector Entity::InterceptTargetXY( const Vector &targetPosition, const Vector &targetVelocity, const float maxSpeed) const
	{
	Vector myPosition (origin);
	float distanceToTargetPosition = Vector::Distance(targetPosition, myPosition);
	if ( !fSmallEnough( distanceToTargetPosition, 0.1f ) )
		{
		Vector predictedPosition = targetPosition + targetVelocity * ( distanceToTargetPosition / maxSpeed );
		Vector desiredDirection = predictedPosition - myPosition;
		desiredDirection.z = 0.0f;
		return desiredDirection.toAngles();
		}
	return angles;
	}

inline float Entity::DistanceTo(const Vector &pos)
	{
	Vector delta;

	delta = origin - pos;
	return delta.length();
	}

inline float Entity::DistanceTo(const Entity *ent)
	{
	Vector delta;

	assert( ent );

	if ( !ent )
		return 999999.0f; // "Infinite" distance

	delta = origin - ent->origin;
	return delta.length();
	}

inline qboolean Entity::WithinDistance(const Vector &pos, float dist)
	{
	Vector delta;
	delta = origin - pos;

	// check squared distance
	return ( ( delta * delta ) < ( dist * dist ) );
	}

inline qboolean Entity::WithinDistance(const Entity *ent, float dist)
	{
	Vector delta;

	//assert( ent );

	if ( !ent )
		return false;
		
	delta = origin - ent->origin;

	// check squared distance
	return ( ( delta * delta ) < ( dist * dist ) );
	}

inline bool Entity::WithinDistanceXY( const Vector &pos, float dist )
{
	float distance = Vector::DistanceXY( origin , pos );
	return distance <= dist;
}

inline bool Entity::WithinDistanceXY( const Entity *ent , float dist )
{
	if ( !ent )
		return false;

	float distance = Vector::DistanceXY( origin , ent->origin );
	return distance <= dist;
}

inline const char *Entity::Target()
	{
	return target.c_str();
	}

inline qboolean Entity::Targeted()
	{
	if ( !targetname.length() )
		return false;

	return true;
	}

inline const char *Entity::TargetName()
	{
	return targetname.c_str();
	}

inline const char * Entity::KillTarget()
	{
	return killtarget.c_str();
	}

inline qboolean Entity::hidden()
	{
	if ( edict->s.renderfx & RF_DONTDRAW )
		return true;

	return false;
	}

inline void Entity::setModel(const str &mdl)
	{
	setModel( mdl.c_str() );
	}

inline void Entity::setViewModel(const str &mdl)
	{
	setViewModel( mdl.c_str() );
	}

inline void Entity::SetModelEvent(Event *ev)
	{
	char modelname[256] ;
	strcpy(modelname, ev->GetString( 1 ) );
	char *tmpPtr = strstr(modelname, "*");
	if (tmpPtr)
		{
		ev->SetString( 1, tmpPtr);
		}

	setModel( ev->GetString( 1 ) );
	}

inline void Entity::hideModel()
	{
	edict->s.renderfx |= RF_DONTDRAW;
	if ( getSolidType() <= SOLID_TRIGGER )
		edict->svflags |= SVF_NOCLIENT;
	}

inline void Entity::showModel()
	{
	edict->s.renderfx &= ~RF_DONTDRAW;
	edict->svflags &= ~SVF_NOCLIENT;
	}

inline float Entity::alpha()
	{
	return edict->s.alpha;
	}

inline void Entity::setMoveType(int type)
	{
	movetype = type;
	}

inline int Entity::getMoveType()
	{
	return movetype;
	}

inline void Entity::unlink()
	{
	gi.unlinkentity( edict );
	}

inline void Entity::setContents(int type)
	{
	edict->contents = type;
	}

inline int Entity::getContents()
	{
	return edict->contents;
	}

inline qboolean Entity::isClient()
	{
	if ( client )
		return true;

	return false;
	}

inline void Entity::SetDeltaAngles()
	{
	int i;

	if ( !client )
		return;

	for( i = 0; i < 3; i++ )
		client->ps.delta_angles[ i ] = ANGLE2SHORT( client->ps.viewangles[ i ] );
	}

inline qboolean Entity::GlobalAliasExists(const char *name)
	{
	assert( name );
	return ( gi.GlobalAlias_FindRandom( name ) != NULL );
	}

inline qboolean Entity::AliasExists(const char *name)
	{
	assert( name );
	return ( gi.Alias_FindRandom( edict->s.modelindex, name ) != NULL );
	}

inline str Entity::GetRandomAlias(const str &name)
	{
	str realname;
	const char *s;

	s = gi.Alias_FindRandom( edict->s.modelindex, name.c_str() );
	if ( s )
		realname = s;
	else
		{
		s = gi.GlobalAlias_FindRandom( name.c_str() );
		if ( s )
			realname = s;
		}

	return realname;
	}

#include "worldspawn.h"

#endif
