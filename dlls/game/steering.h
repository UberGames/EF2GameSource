//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/steering.h                                    $
// $Revision:: 21                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Steering behaviors for AI.
//

#ifndef __STEERING2_H__
#define __STEERING2_H__

#include "g_local.h"
#include "entity.h"

class Actor;

//-----------------------------------------------------------------------------
//
// Steering Class Declaration
//
//-----------------------------------------------------------------------------

class Steering : public Listener
	{
	public:
		enum ReturnValue
		{
			EVALUATING,
			EVALUATING_REACHED_NODE,
			SUCCESS,
			FAILED_BLOCKED_BY_ENEMY,
			FAILED_BLOCKED_BY_CIVILIAN,
			FAILED_BLOCKED_BY_FRIEND,
			FAILED_BLOCKED_BY_TEAMMATE,
			FAILED_BLOCKED_BY_WORLD,
			FAILED_BLOCKED_BY_FALL,
			FAILED_BLOCKED_BY_DOOR,
			FAILED_CANNOT_GET_TO_PATH,
			FAILED_NO_PATH,
			FAILED,
			ERROR,
			NUMBER_OF_RETURN_VALUES,
		};
											CLASS_PROTOTYPE( Steering );
											Steering();
      virtual void					ShowInfo( Actor &self );
		virtual void					Begin( Actor &self );
		virtual const ReturnValue	Evaluate( Actor &self );
		virtual void					End( Actor &self );

      virtual void					ResetForces( void );
		
		virtual const Vector &		GetSteeringForce( void )  const;
		virtual void					SetSteeringForce( const Vector &steeringForce );
		virtual const Vector &		GetPosition( void ) const;
		virtual void					SetPosition( const Vector &pos );
		virtual const Vector &		GetMoveDirection( void ) const;
		virtual void					SetMoveDirection( const Vector &dir );
		virtual const float			GetMaxSpeed( void ) const;
		virtual void					SetMaxSpeed( float speed );
		virtual void					DrawForces( void );
	   virtual void					Archive( Archiver &arc );
	
	private:
		Vector							_steeringForce;
		Vector							_origin;
		Vector							_moveDirection;
		float								_maxSpeed;
	};

inline void Steering::Archive
	(
	Archiver &arc
	)

   {
   Listener::Archive( arc );

   arc.ArchiveVector( &_steeringForce );
	arc.ArchiveVector( &_origin );
	arc.ArchiveVector( &_moveDirection );
	arc.ArchiveFloat(  &_maxSpeed );
   }

enum JumpStates
{
	JUMP_PREPARE,
	JUMP_LAUNCH,
	JUMP_FALL,
	JUMP_LANDING
};

class Jump : public Steering
	{
	public:
											CLASS_PROTOTYPE( Jump );

											Jump();
		void								SetGoal( const Vector &goalPosition );
		void								SetEntity( Entity *ent_to_jump_to );
		void								SetLaunchAngle( const float launchAngle) { _launchAngle = launchAngle; }
		void								Begin( Actor &self );
		virtual const ReturnValue			Evaluate( Actor &self );
		void								End( Actor &self );
		virtual void						Archive( Archiver &arc );

	private:
		float								_endtime;
		Angle								_launchAngle;
		str									_anim;
		int									_state;
		bool								_animdone;
		Vector								_goal;

	};

inline void Jump::Archive
	(
	Archiver &arc
	)
   {
   Steering::Archive( arc );

   arc.ArchiveFloat(	&_endtime );
   arc.ArchiveFloat(	&_launchAngle.GetValue() );
   arc.ArchiveString(	&_anim );
   arc.ArchiveInteger(	&_state );
   arc.ArchiveBool(		&_animdone );
   arc.ArchiveVector(	&_goal );
   }

#endif // steering.h
