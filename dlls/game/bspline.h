//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/bspline.h                                     $
// $Revision:: 9                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Uniform non-rational bspline class.
//

#ifndef __BSPLINE_H__
#define __BSPLINE_H__

#include "g_local.h"
#include "vector.h"

typedef enum
	{
	SPLINE_NORMAL,
	SPLINE_LOOP,
	SPLINE_CLAMP
	} splinetype_t;

class BSplineControlPoint : public Class
	{
   private:
      float          roll;
      Vector         position;
      Vector         orientation;
      float          speed;

	public:
							BSplineControlPoint();
							BSplineControlPoint( const Vector &pos, Vector orient, float speed );
							BSplineControlPoint( const Vector &pos );
		void				Clear( void );
		void				Set( const Vector &pos );
		void				Set( const Vector &pos, float speed );
		void				Set( const Vector &pos, Vector orient, float speed );
		void				Get( Vector& pos, Vector& orient, float& speed );
		void				Get( Vector& pos );
      Vector         *GetPosition( void );
      Vector         *GetOrientation( void );
      float          *GetRoll( void );
      float          *GetSpeed( void );
		void		      operator=( const BSplineControlPoint &point );
      virtual void   Archive( Archiver &arc );
	};

inline void BSplineControlPoint::Archive( Archiver &arc )
{
	arc.ArchiveFloat( &roll );
	arc.ArchiveVector( &position );
	arc.ArchiveVector( &orientation );
	arc.ArchiveFloat( &speed );
}

inline void BSplineControlPoint::operator=
	(
	const BSplineControlPoint &point
	)

	{
   position = point.position;
   orientation = point.orientation;
   speed = point.speed;
   roll = point.roll;
	}

inline BSplineControlPoint::BSplineControlPoint()
	{
   roll = 0;
   speed = 1;
	}

inline BSplineControlPoint::BSplineControlPoint
   (
   const Vector &pos
   )

	{
	roll = 0;
   speed = 1;
   position = pos;
	}

inline BSplineControlPoint::BSplineControlPoint
   (
   const Vector &pos,
   Vector orient,
   float init_speed
   )

	{
   position = pos;
	orient.AngleVectors( &orientation, NULL, NULL );
   roll = orient[ ROLL ];
   if ( roll > 180.0f )
      {
      roll -= 360.0f;
      }
   if ( roll < -180.0f )
      {
      roll += 360.0f;
      }
   speed = init_speed;
	}

inline void BSplineControlPoint::Clear
   (
   void
   )

	{
   roll = 0;
   position = Vector(0, 0, 0);
   vec_zero.AngleVectors( &orientation, NULL, NULL );
   speed = 1.0f;
	}

inline void BSplineControlPoint::Set
   (
   const Vector &pos
   )

	{
   speed = 1;
   position = pos;
	}

inline void BSplineControlPoint::Set
   (
   const Vector &pos,
   float pointspeed
   )

	{
   speed = pointspeed;
   position = pos;
	}

inline void BSplineControlPoint::Set
   (
   const Vector &pos,
   Vector orient,
   float new_speed
   )

	{
   position = pos;
	orient.AngleVectors( &orientation, NULL, NULL );
   roll = orient[ ROLL ];
   if ( roll > 180.0f )
      {
      roll -= 360.0f;
      }
   if ( roll < -180.0f )
      {
      roll += 360.0f;
      }
   speed = new_speed;
	}

inline void BSplineControlPoint::Get
   (
   Vector& pos
   )
	{
   pos = position;
	}

inline Vector *BSplineControlPoint::GetPosition
   (
   void
   )

	{
   return &position;
	}

inline void BSplineControlPoint::Get
   (
   Vector& pos,
   Vector& orient,
   float& spd
   )

	{
   pos    = position;
   orient = orientation;
   spd    = speed;
	}

inline Vector *BSplineControlPoint::GetOrientation
   (
   void
   )
	{
   return &orientation;
	}

inline float *BSplineControlPoint::GetRoll
   (
   void
   )
	{
   return &roll;
	}

inline float *BSplineControlPoint::GetSpeed
   (
   void
   )
	{
   return &speed;
	}

class BSpline : public Class
	{
	private:
		BSplineControlPoint  *control_points;
		int				      num_control_points;
      int                  loop_control_point;
		splinetype_t	      curvetype;
      qboolean             has_orientation;

		float  			EvalNormal( float u, Vector &pos, Vector& orient );
		float          EvalLoop( float u, Vector &pos, Vector& orient );
		float 			EvalClamp( float u, Vector &pos, Vector& orient );

	public:
							BSpline();
							~BSpline();
							BSpline( Vector *control_points_, int num_control_points_, splinetype_t type );
							BSpline( Vector *control_points_, Vector *control_orients_, float *control_speeds_, int num_control_points_, splinetype_t type );
		void		      operator=( const BSpline &spline );
		void				SetType( splinetype_t type );
		int   			GetType( void );
		void				Clear( void );
		void				Set( const Vector *control_points_, int num_control_points_, splinetype_t type );
		void				Set( const Vector *control_points_, const Vector *control_orients_, const float *control_speeds_, int num_control_points_, splinetype_t type );
		void				AppendControlPoint( const Vector& new_control_point );
		void				AppendControlPoint( const Vector& new_control_point, const float& speed );
		void				AppendControlPoint( const Vector& new_control_point, const Vector& new_control_orient, const float& speed );
		Vector			Eval( float u );
		float  			Eval( float u, Vector& pos, Vector& orient );

		void				DrawControlSegments( void );
		void				DrawCurve( int num_subdivisions );
		void				DrawCurve( const Vector &offset, int num_subdivisions );

      void           SetLoopPoint( const Vector& pos );

		float				EndPoint( void );

		// return the index of the control point picked or -1 if none.
		int				PickControlPoint( const Vector& window_point, float pick_size );

		Vector			*GetControlPoint( int id );
		void           GetControlPoint( int id, Vector& pos, Vector& orient, float& speed );
		void				SetControlPoint( int id, const Vector& new_control_point );
		void				SetControlPoint( int id, const Vector& new_control_point, const Vector& new_control_orient, const float& speed );
      virtual void   Archive( Archiver &arc );
	};

inline BSpline::BSpline()
	{
   has_orientation = false;
	control_points = NULL;
	num_control_points = 0;
   loop_control_point = 0;
	curvetype = SPLINE_NORMAL;
	}

inline BSpline::~BSpline()
	{
   if ( control_points )
      {
      delete [] control_points;
      control_points = NULL;
      }
	}

inline BSpline::BSpline
	(
	Vector *control_points_,
	int num_control_points_,
	splinetype_t type
	)

	{
   has_orientation = false;
	control_points = NULL;
	num_control_points = 0;
   loop_control_point = 0;
	curvetype = SPLINE_NORMAL;

   Set( control_points_, num_control_points_, type );
	}

inline BSpline::BSpline
	(
	Vector *control_points_,
	Vector *control_orients_,
	float  *control_speeds_,
	int num_control_points_,
	splinetype_t type
	)

	{
   has_orientation = false;
	control_points = NULL;
	num_control_points = 0;
   loop_control_point = 0;
	curvetype = SPLINE_NORMAL;

   Set( control_points_, control_orients_, control_speeds_, num_control_points_, type );
	}

inline void BSpline::operator=
	(
	const BSpline &spline
	)

	{
   int i;

   Clear();
   num_control_points = spline.num_control_points;
   loop_control_point = spline.loop_control_point;
   curvetype = spline.curvetype;
   has_orientation = spline.has_orientation;

   if ( num_control_points )
      {
	   control_points = new BSplineControlPoint[num_control_points];
	   assert( control_points );
      for ( i = 0; i < num_control_points ; i++ )
         control_points[ i ] = spline.control_points[ i ];
      }
   else
      {
      control_points = NULL;
      }
	}

inline void BSpline::SetType
	(
	splinetype_t type
	)

	{
	curvetype = type;
	}

inline int BSpline::GetType
	(
   void
	)

	{
	return curvetype;
	}

inline float BSpline::EndPoint
	(
	void
	)

	{
	return num_control_points;
	}

inline Vector *BSpline::GetControlPoint
	(
	int id
	)

	{
   assert( id >= 0 );
	assert( id < num_control_points );
   if ( ( id < 0 ) && ( id >= num_control_points ) )
      {
      // probably wrong, but if we're in release mode we have no recourse
      id = 0;
      }

   return control_points[ id ].GetPosition();
  	}

inline void BSpline::GetControlPoint
	(
	int id,
   Vector& pos,
   Vector& orient,
   float& speed
	)

	{
   assert( id >= 0 );
	assert( id < num_control_points );

	if ( !control_points )
		return;

   if ( ( id >= 0 ) && ( id < num_control_points ) )
      {
      control_points[ id ].Get( pos, orient, speed );
      }
	}

inline void BSpline::SetControlPoint
	(
	int id,
	const Vector& new_control_point
	)

	{
   assert( id >= 0 );
	assert( id < num_control_points );

	if ( !control_points )
		return;

   if ( ( id >= 0 ) && ( id < num_control_points ) )
      {
      control_points[ id ].Set( new_control_point );
      }
	}

inline void BSpline::SetControlPoint
	(
	int id,
	const Vector& new_control_point,
	const Vector& new_control_orient,
   const float& speed
	)

	{
   assert( id >= 0 );
	assert( id < num_control_points );

	if ( !control_points )
		return;

   if ( ( id >= 0 ) && ( id < num_control_points ) )
      {
      control_points[ id ].Set( new_control_point, new_control_orient, speed );
      }
	}

inline void BSpline::Archive
	(
	Archiver &arc
	)
   {
   int i;

   arc.ArchiveInteger( &num_control_points );
   if ( arc.Loading() )
      {
		if ( num_control_points )
			control_points = new BSplineControlPoint[ num_control_points ];
      }

   arc.ArchiveInteger( &loop_control_point );

   i = curvetype;
   arc.ArchiveInteger( &i );
   curvetype = ( splinetype_t )i;

   arc.ArchiveBoolean( &has_orientation );
   for( i = 0; i < num_control_points; i++ )
      {
      control_points[ i ].Archive( arc );
      }
   }

extern Event EV_SplinePath_Create;
extern Event EV_SplinePath_Loop;
extern Event EV_SplinePath_Speed;

class SplinePath : public Entity
	{
	protected:
		SplinePath        *owner;
		SplinePath        *next;
      SplinePath        *loop;
      str               loop_name;

		void					CreatePath( Event *ev );
      void              SetLoop( Event *ev );
      void              SetSpeed( Event *ev );
      void              SetTriggerTarget( Event *ev );
      void              SetThread( Event *ev );
      void              SetFov( Event *ev );
      void              SetWatch( Event *ev );
      void              SetFadeTime( Event *ev );
		void              MoveSpline( Event *ev );
		void              OffsetSpline( Event *ev );
		void              TurnSpline( Event *ev );

	public:
      float             speed;
      float             fov;
      float             fadeTime;
      qboolean          doWatch;
      str               watchEnt;
      str               triggertarget;
      str               thread;

      CLASS_PROTOTYPE( SplinePath );

								SplinePath();
								~SplinePath();
		SplinePath			*GetNext( void );
		SplinePath			*GetPrev( void );
		SplinePath			*GetLoop( void );
      void              SetFadeTime( float newFadeTime );
      void              SetFov( float theFov );
      void              SetWatch( const char *name );
      void              SetThread( const char *name );
      void              SetTriggerTarget( const char *name );
      void              NoWatch( void );
      str               GetWatch( void );
      float             GetFadeTime( void );
      float             GetFov( void );
		void              SetNext( SplinePath *node );
		void              SetPrev( SplinePath *node );
      virtual void      Archive( Archiver &arc );
	};

typedef SafePtr<SplinePath> SplinePathPtr;

inline void SplinePath::Archive
	(
	Archiver &arc
	)

   {
   Entity::Archive( arc );

   arc.ArchiveObjectPointer( ( Class ** )&owner );
   arc.ArchiveObjectPointer( ( Class ** )&next );
   arc.ArchiveObjectPointer( ( Class ** )&loop );
   arc.ArchiveString( &loop_name );
   arc.ArchiveFloat( &speed );
   arc.ArchiveFloat( &fov );
   arc.ArchiveFloat( &fadeTime );
   arc.ArchiveBoolean( &doWatch );
   arc.ArchiveString( &watchEnt );
   arc.ArchiveString( &thread );
   arc.ArchiveString( &triggertarget );
   if ( arc.Loading() )
      {
      CancelEventsOfType( EV_SplinePath_Create );
      }
   }

inline void SplinePath::SetThread
   (
   const char *name
   )

   {
   thread = name;
   }

inline void SplinePath::SetTriggerTarget
   (
   const char *name
   )

   {
   triggertarget = name;
   }

#endif /* __BSPLINE_H__ */
