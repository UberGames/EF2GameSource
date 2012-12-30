//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/misc.h                                        $
// $Revision:: 18                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Basically the big stew pot of the DLLs, or maybe a garbage bin, whichever
// metaphore you prefer.  This really should be cleaned up.  Anyway, this
// should contain utility functions that could be used by any entity.
// Right now it contains everything from entities that could be in their
// own file to my mother pot roast recipes.
//

#ifndef __MISC_H__
#define __MISC_H__

#include "g_local.h"
#include "entity.h"
#include "mover.h"
#include "animate.h"

class InfoNull : public Listener
	{
	public:
      CLASS_PROTOTYPE( InfoNull );

		InfoNull();
	};

class FuncRemove : public Entity
	{
	public:
      CLASS_PROTOTYPE( FuncRemove );

		FuncRemove();
	};

class MiscModel : public Entity
	{
	public:
      CLASS_PROTOTYPE( MiscModel );

		MiscModel();
	};

class InfoNotNull : public Entity
	{
	public:
      CLASS_PROTOTYPE( InfoNotNull );
	};

class ExplodingWall : public Trigger
	{
	protected:
		int      dmg;
		int      explosions;
		float    attack_finished;
      Vector   land_angles;
      float    land_radius;
      float    angle_speed;
      int      state;
      Vector   base_velocity;
      Vector   random_velocity;
      Vector   orig_mins, orig_maxs;
      qboolean on_ground;

	public:
      CLASS_PROTOTYPE( ExplodingWall );

							ExplodingWall();
      void           Setup( Event *ev );
      void           AngleSpeed( Event *ev );
      void           LandRadius( Event *ev );
      void           LandAngles( Event *ev );
      void           BaseVelocity( Event *ev );
      void           RandomVelocity( Event *ev );
      void           SetDmg( Event *ev );
      void           SetExplosions( Event *ev );
      void           SetupSecondStage( void );
		void	         Explode( Event *ev );
		void	         DamageEvent( Event *ev );
		void	         GroundDamage( Event *ev );
		void	         TouchFunc( Event *ev );
		void	         StopRotating( Event *ev );
		void	         CheckOnGround( Event *ev );
		void			setDamage( Event *ev );
	   virtual void   Archive( Archiver &arc );
	};

inline void ExplodingWall::Archive
	(
	Archiver &arc
	)
   {
   Trigger::Archive( arc );

   arc.ArchiveInteger( &dmg );
   arc.ArchiveInteger( &explosions );
   arc.ArchiveFloat( &attack_finished );
   arc.ArchiveVector( &land_angles );
   arc.ArchiveFloat( &land_radius );
   arc.ArchiveFloat( &angle_speed );
   arc.ArchiveInteger( &state );
   arc.ArchiveVector( &base_velocity );
   arc.ArchiveVector( &random_velocity );
   arc.ArchiveVector( &orig_mins );
   arc.ArchiveVector( &orig_maxs );
   arc.ArchiveBoolean( &on_ground );
   }

class Teleporter : public Trigger
	{
	public:
		str					teleport_thread;
		qboolean				in_use;

      CLASS_PROTOTYPE( Teleporter );

								Teleporter();
		virtual void		StartTeleport( Event *ev );
		virtual void		Teleport( Event *ev );
		virtual void		StopTeleport( Event *ev );
		void					SetThread( Event *ev );
		bool				doFullTeleport( Entity *entity );

		virtual void      Archive( Archiver &arc );
	};

inline void Teleporter::Archive
	(
	Archiver &arc
	)
   {
   Trigger::Archive( arc );

   arc.ArchiveString( &teleport_thread );
	arc.ArchiveBoolean( &in_use );
   }

class TeleporterDestination : public Entity
	{
	public:
		Vector				movedir;

      CLASS_PROTOTYPE( TeleporterDestination );

						TeleporterDestination();
		void	            SetMoveDir( Event *ev );
	   virtual void      Archive( Archiver &arc );
	};

inline void TeleporterDestination::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveVector( &movedir );
   }

class UseAnim : public Entity
	{
	public:
      int               count;
      qboolean          active;
      str               thread;
      str               triggertarget;
      int               num_loops;
      str               state;
      str               camera;
      str               anim;
      str               key;
      float             delay;
      float             last_active_time;
	  str				action_type;

      CLASS_PROTOTYPE( UseAnim );

								UseAnim();
		virtual void		Touched( Event *ev );
      void              Reset( Event *ev );
      void              SetThread( Event * ev );
      void              SetTriggerTarget( Event * ev );
      void              SetCount( Event * ev );
		void	            SetAnim( Event *ev );
		void	            SetState( Event *ev );
		void	            SetKey( Event *ev );
		void	            SetCamera( Event *ev );
		void	            SetNumLoops( Event *ev );
		void	            SetDelay( Event *ev );
      bool              canBeUsed( Entity *activator );
      bool              GetInformation( const Entity *activator, Vector * org, Vector * angles, str * animatoin, int * loopcount, str * state, str * camera );
      void              TriggerTargets( Entity *activator );
	  void				SetActionType( Event *ev );
	   virtual void      Archive( Archiver &arc );
	};

inline void UseAnim::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveInteger( &count );
   arc.ArchiveBoolean( &active );
   arc.ArchiveString( &thread );
   arc.ArchiveString( &triggertarget );
   arc.ArchiveInteger( &num_loops );
   arc.ArchiveString( &state );
   arc.ArchiveString( &camera );
   arc.ArchiveString( &anim );
   arc.ArchiveString( &key );
   arc.ArchiveFloat( &delay );
   arc.ArchiveFloat( &last_active_time );
   arc.ArchiveString( &action_type );
   }

class TouchAnim : public UseAnim
	{
	public:

      CLASS_PROTOTYPE( TouchAnim );

								TouchAnim();
	};


class UseAnimDestination : public Entity
	{
	public:
      int               num_loops;
      str               state;
      str               anim;

      CLASS_PROTOTYPE( UseAnimDestination );

						      UseAnimDestination();
		void	            SetAnim( Event *ev );
		void	            SetState( Event *ev );
		void	            SetNumLoops( Event *ev );
		int	            GetNumLoops( void );
		str               GetAnim( void );
		str               GetState( void );
	   virtual void      Archive( Archiver &arc );
	};

inline void UseAnimDestination::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveInteger( &num_loops );
   arc.ArchiveString( &state );
   arc.ArchiveString( &anim );
   }

class UseObject : public Entity
	{
	public:
      str               move_thread;
      str               stop_thread;
      str               reset_thread;
      str               triggertarget;
      Vector            offset;
      float             yaw_offset;
      int               count;
      float             cone;
      str               state;
      str               state_backwards;
      str               useMaterial;
      int               objectState;
      float             reset_time;
	  str				action_type;
      qboolean          active;
	  bool				movetheplayer;

      CLASS_PROTOTYPE( UseObject );

								UseObject();
      void              SetMoveThread( Event * ev );
      void              SetStopThread( Event * ev );
      void              SetResetThread( Event * ev );
      void              SetTriggerTarget( Event * ev );
      void              SetOffset( Event * ev );
      void              SetYawOffset( Event * ev );
      void              SetCount( Event * ev );
      void              SetCone( Event * ev );
      void              SetState( Event * ev );
      void              SetBackwardsState( Event * ev );
      void              SetResetTime( Event * ev );
      void              Reset( Event * ev );
      void              Resetting( Event * ev );
      void              DamageTriggered( Event * ev );
      void              DamageFunc( Event * ev );
      bool              canBeUsed( const Vector &org, const Vector &dir );
      void              Setup( const Entity *activator, Vector *org, Vector *ang, str *newstate );
      void              Start( Event * ev = NULL );
      bool              Loop( void );
      void              SetActiveState( Event *ev );
      void              Stop( Entity *activator );
      void              ActivateEvent( Event *ev );
      void              DeactivateEvent( Event *ev );
      void              UseMaterialEvent( Event *ev );
	  void				SetActionType( Event *ev );
	  void				MoveThePlayer( Event *ev );
	   virtual void      Archive( Archiver &arc );
	};

inline void UseObject::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveString( &move_thread );
   arc.ArchiveString( &stop_thread );
   arc.ArchiveString( &reset_thread );
   arc.ArchiveString( &triggertarget );
   arc.ArchiveVector( &offset );
   arc.ArchiveFloat( &yaw_offset );
   arc.ArchiveInteger( &count );
   arc.ArchiveFloat( &cone );
   arc.ArchiveString( &state );
   arc.ArchiveString( &state_backwards );
   arc.ArchiveString( &useMaterial );
   arc.ArchiveInteger( &objectState );
   arc.ArchiveFloat( &reset_time );
   arc.ArchiveString( &action_type );
   arc.ArchiveBoolean( &active );
   arc.ArchiveBool( &movetheplayer );
   
   }


class Waypoint : public Entity
	{
	public:
      CLASS_PROTOTYPE( Waypoint );
	};

class TossObject : public Entity
	{
   private:
      str            bouncesound;
      float          bouncesoundchance;
      void           Stop( Event *ev );
      void           Touch( Event *ev );
      void           SetBounceSound( Event *ev );
      void           SetBounceSoundChance( Event *ev );
   public:
      CLASS_PROTOTYPE( TossObject );

                     TossObject();
                     TossObject( const str &modelname );
      void           SetBounceSound( const str &bounce );
      void           SetBounceSoundChance( float chance );
      void           SetVelocity( float severity );
      virtual void   Archive( Archiver &arc );
   };

inline void TossObject::Archive
	(
	Archiver &arc
	)

   {
   Entity::Archive( arc );

   arc.ArchiveString( &bouncesound );
   arc.ArchiveFloat( &bouncesoundchance );
   }


class PushObject : public Entity
	{
   private:
      EntityPtr      owner;
		float			   attack_finished;
		int				dmg;
      str            pushsound;

 	public:
      CLASS_PROTOTYPE( PushObject );

                     PushObject();
      void           Start( Event *ev );
      void           SetDamage( Event *ev );
      void           BlockFunc( Event *ev );
      void           SetPushSound( Event *ev );
      qboolean       Push( Entity *pusher, const Vector &move );
      qboolean       canPush( const Vector &dir );
	  void			setDamage( Event *ev );

      Entity         *getOwner( void );

      virtual void   Archive( Archiver &arc );
	};

inline void PushObject::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveSafePointer( &owner );
   arc.ArchiveFloat( &attack_finished );
   arc.ArchiveInteger( &dmg );
   arc.ArchiveString( &pushsound );
   }

class FallingRock : public Entity
	{
   private:
      int            active;
      Vector         start_origin;
      Vector         last_bounce_origin;
      Entity         *current;
      Entity         *activator;
      Vector         bounce_dir;
      Vector         rotateaxis;
      float          attack_finished;
      float          wait;
      float          speed;
      int            dmg;
      str            bouncesound;

      void           Touch( Event *ev );
      void           Bounce( Event *ev );
      void           Rotate( Event *ev );
      void           Activate( Event *ev );
      void           NextBounce( void );
      void           StartFalling( Event *ev );
      void           SetWait( Event *ev );
      void           SetSpeed( Event *ev );
      void           SetDmg( Event *ev );
      Entity         *SetNextBounceDir( void );
      void           SetBounceSound( const str &sound );
      void           SetBounceSound( Event *ev );
	  void			setDamage( Event *ev );
      virtual void   Archive( Archiver &arc );

   public:
      CLASS_PROTOTYPE( FallingRock );

                     FallingRock();
   };

inline void FallingRock::Archive
	(
	Archiver &arc
	)

   {
   Entity::Archive( arc );

   arc.ArchiveInteger( &active );
   arc.ArchiveVector( &start_origin );
   arc.ArchiveVector( &last_bounce_origin );
   arc.ArchiveObjectPointer( ( Class ** )&current );
   arc.ArchiveObjectPointer( ( Class ** )&activator );
   arc.ArchiveVector( &bounce_dir );
   arc.ArchiveVector( &rotateaxis );
   arc.ArchiveFloat( &attack_finished );
   arc.ArchiveFloat( &wait );
   arc.ArchiveFloat( &speed );
   arc.ArchiveInteger( &dmg );
   arc.ArchiveString( &bouncesound );
   if ( arc.Loading() )
      {
      SetBounceSound( bouncesound );
      }
   }

#endif /* misc.h */
