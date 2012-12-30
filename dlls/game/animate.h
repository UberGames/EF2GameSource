//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/animate.h                                     $
// $Revision:: 10                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Animate class
//

#ifndef __ANIMATE_H__
#define __ANIMATE_H__

#include "g_local.h"

#ifndef __ENTITY_H__
#include "entity.h"
#endif

extern Event EV_SetFrame;
extern Event EV_StopAnimating;
extern Event EV_Torso_StopAnimating;

#define MINIMUM_DELTA_MOVEMENT 8.0f
#define MINIMUM_DELTA_MOVEMENT_PER_FRAME ( MINIMUM_DELTA_MOVEMENT / 20.0f )

class Animate;
class Entity;

typedef SafePtr<Animate> AnimatePtr;

class Animate : public Listener
	{
   private:
		Event					*animDoneEvent;
		Event					*torso_animDoneEvent;

      float             legs_animtime;
      float             torso_animtime;

      float             legs_starttime;
      float             torso_starttime;

      float             legs_frametime;
      float             torso_frametime;

      int               legs_numframes;
      int               torso_numframes;

      str               currentAnim;
	  float				oldAnimationRate;

		Entity				*self;

      void              FrameDeltaEvent( Event *ev );
      void              EndAnim( bodypart_t part );
      void              Legs_AnimDoneEvent( Event *ev );
      void              Legs_AnimEvent( Event *ev );
      void              Legs_SetFrameEvent( Event *ev );
      void              Legs_StopAnimating( Event *ev );
      void              Torso_AnimDoneEvent( Event *ev );
      void              Torso_AnimEvent( Event *ev );
      void              Torso_SetFrameEvent( Event *ev );
      void              Torso_StopAnimating( Event *ev );
      void              NewAnimEvent( Event *ev );
	public:

		// Animation variables
      Vector            frame_delta;   // current movement from this frame
      CLASS_PROTOTYPE( Animate );
							   Animate();
								Animate( Entity * ent );
								~Animate();

		void					RandomAnimate( const char *animname, Event *endevent = NULL, bodypart_t part = legs );
		void					RandomAnimate( const char *animname, const Event &endevent, bodypart_t part = legs );
		void					SetAnimationRate( const float animationRate );
		void					RestoreAnimationRate( void );
      void              NewAnim( int animnum, bodypart_t part = legs );
      void              NewAnim( int animnum, Event *endevent, bodypart_t part = legs );
      void              NewAnim( int animnum, Event &endevent, bodypart_t part = legs );
      void              SetFrame( int framenum, bodypart_t part = legs, int anim = -1 );
      qboolean          HasAnim( const char *animname );
      Event             *AnimDoneEvent( bodypart_t part = legs );
      void              SetAnimDoneEvent( const Event &event, bodypart_t part = legs );
      void              SetAnimDoneEvent( Event *event, bodypart_t part = legs );
      int               NumFrames( bodypart_t part = legs );
      int               NumAnims( void );
      const char        *AnimName( bodypart_t part = legs );
      float             AnimTime( bodypart_t part = legs );
		str					GetName();

      void              ClearLegsAnim( void );
      void              ClearTorsoAnim( void );

      virtual void      StopAnimating( bodypart_t part = legs );
		virtual void      StopAnimatingAtEnd( bodypart_t part = legs );

      virtual int       CurrentAnim( bodypart_t part = legs );
      virtual int       CurrentFrame( bodypart_t part = legs );

		virtual void	AddEffectAnim( const char *animName );
		virtual void	RemoveEffectAnim( const char *animName );
		virtual void	ClearAllEffectAnims( void );

      virtual void      Archive( Archiver &arc );
	};

inline void Animate::RandomAnimate
	(
   const char *animname,
	const Event &endevent,
   bodypart_t part
   )
   {
	Event *ev;

	ev = new Event( endevent );
	RandomAnimate( animname, ev, part );
   }

inline int Animate::CurrentAnim
	(
   bodypart_t part
   )
   {
   switch( part )
      {
      case legs:
		if ( self->edict->s.anim & ANIM_BLEND )
			return self->edict->s.anim & ANIM_MASK;
		else
			return -1;
         break;
      case torso:
		if ( self->edict->s.torso_anim & ANIM_BLEND )
			return self->edict->s.torso_anim & ANIM_MASK;
		else
			return -1;
         break;
      default:
         warning( "CurrentAnim", "Unknown body part %d", part );
         return -1;
         break;
      }
   }

inline int Animate::CurrentFrame
	(
   bodypart_t part
   )
   {
   int frame;

   switch( part )
      {
      case legs:
         if ( self->edict->s.frame & FRAME_EXPLICIT )
            {
            frame = self->edict->s.frame & FRAME_MASK;
            }
         else
            {
            if ( legs_numframes )
               {
               frame = ( int )( ( float )( ( level.time - legs_starttime ) * legs_numframes ) / legs_animtime + 0.5f );
               while ( frame >= legs_numframes )
                  frame -= legs_numframes;
               }
            else
               {
               frame = 0;
               }
            }
         break;
      case torso:
         if ( self->edict->s.torso_frame & FRAME_EXPLICIT )
            {
            frame = self->edict->s.torso_frame & FRAME_MASK;
            }
         else
            {
            if ( torso_numframes )
               {
               frame = ( int )( ( float )( ( level.time - torso_starttime ) * torso_numframes ) / torso_animtime + 0.5f );
               while ( frame >= torso_numframes )
                  frame -= torso_numframes;
               }
            else
               {
               frame = 0;
               }
            }
         break;
      default:
         warning( "CurrentFrame", "Unknown body part %d", part );
         frame = 0;
         break;
      }
   return frame;
   }

inline int Animate::NumFrames
	(
   bodypart_t part
   )
   {
   switch( part )
      {
      case legs:
         return legs_numframes;
         break;
      case torso:
         return torso_numframes;
         break;
      default:
         warning( "NumFrames", "Unknown body part %d", part );
         return 0;
         break;
      }
   }

inline float Animate::AnimTime
	(
   bodypart_t part
   )
   {
   switch( part )
      {
      case legs:
         return legs_animtime;
         break;
      case torso:
         return torso_animtime;
         break;
      default:
         warning( "AnimTime", "Unknown body part %d", part );
         return 0;
         break;
      }
   }

inline int Animate::NumAnims
	(
   void
   )

   {
   return gi.NumAnims( self->edict->s.modelindex );
   }

inline const char *Animate::AnimName
	(
   bodypart_t part
   )
   {
   switch( part )
      {
      case legs:
         return gi.Anim_NameForNum( self->edict->s.modelindex, CurrentAnim( part ) );
         break;
      case torso:
         return gi.Anim_NameForNum( self->edict->s.modelindex, CurrentAnim( part ) );
         break;
      default:
         warning( "AnimName", "Unknown body part %d", part );
         return NULL;
         break;
      }
   }

inline Event * Animate::AnimDoneEvent
	(
   bodypart_t part
   )
   {
   switch( part )
      {
      case legs:
         if ( animDoneEvent )
            return new Event( animDoneEvent );
         else
            return NULL;
         break;
      case torso:
         if ( torso_animDoneEvent )
            return new Event( torso_animDoneEvent );
         else
            return NULL;
         break;
      default:
         warning( "AnimDoneEvent", "Unknown body part %d", part );
         return NULL;
         break;
      }
   }

inline void Animate::Archive( Archiver &arc )
{
	Listener::Archive( arc );

	arc.ArchiveEventPointer( &animDoneEvent );
	arc.ArchiveEventPointer( &torso_animDoneEvent );

	arc.ArchiveFloat( &legs_animtime );
	arc.ArchiveFloat( &torso_animtime );

	arc.ArchiveFloat( &legs_starttime );
	arc.ArchiveFloat( &torso_starttime );

	arc.ArchiveFloat( &legs_frametime );
	arc.ArchiveFloat( &torso_frametime );

	arc.ArchiveInteger( &legs_numframes );
	arc.ArchiveInteger( &torso_numframes );

	arc.ArchiveString( &currentAnim );
	arc.ArchiveFloat( &oldAnimationRate );
	arc.ArchiveVector( &frame_delta );
}

#endif /* animate.h */
