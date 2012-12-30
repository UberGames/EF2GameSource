//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actor_locomotion.h                             $
// $Revision:: 22                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Motion and Movement Related Classes
//

class MovementSubsystem;
class LocomotionController;

#ifndef __ACTOR_LOCOMOTION_H__
#define __ACTOR_LOCOMOTION_H__

#include "actor.h"
#include "actorincludes.h"
#include "weapon.h"
#include "path.h"
#include "steering.h"

class LocomotionController
	{
	public:
		LocomotionController();
		LocomotionController( Actor *actor );
		~LocomotionController();

		void Begin();
		void Evaluate();
		void End();
         
		// Accessors & Mutators
		void						SetMovementStyle( MovementStyle style );
		MovementStyle				GetMovementStyle();
       
		// Archiving
		virtual void				Archive( Archiver &arc );		 
		void						DoArchive( Archiver &arc , Actor *actor );

	protected:
		void						_init();

	private:
		Actor						*act;
		MovementStyle				_movementStyle;
		FollowPath					_chase;

	};

//============================
// Class MovementSubsystem
//============================
//
// Encapsulates movement related data and functionality for the actor
//
class MovementSubsystem 
	{
	public:
		 MovementSubsystem();
		 MovementSubsystem( Actor *actor );
		~MovementSubsystem();

		qboolean					CanMoveTo( const Vector &pos );
		bool						CanWalkTowardsPoint( const Vector &goalPoint, const int mask = -1);
		qboolean					CanWalkTo( const Vector &pos, float bounding_box_extra = 0, int entnum = ENTITYNUM_NONE, const int mask = -1 );
		qboolean					CanWalkToFrom( const Vector &origin, const Vector &pos, float bounding_box_extra = 0, int entnum = ENTITYNUM_NONE, const int mask = -1 );
		void						Accelerate( const Vector &steering );
		void						CalcMove( void );		
		stepmoveresult_t			WaterMove( void );
		stepmoveresult_t			AirMove( void );
		stepmoveresult_t			IsMoveValid( trace_t &horizontalTrace, trace_t &verticalTrace, const Vector &moveBegin, const Vector &moveEnd );
		stepmoveresult_t			TryMove( void );
		stepmoveresult_t			SimpleMove( const bool stickToGround );

		qboolean					Push( const Vector &dir );
		void						CheckWater( void );
		float						JumpTo( PathNode * goal, const Angle angle = 45.0f );
		float						JumpTo( Entity * goal, const Angle angle = 45.0f );
		float						JumpTo( const Vector &targ, const Angle angle = 45.0f );
		const Vector				SteerTowardsPoint(const Vector &targetPosition, const Vector &targetVelocity, const Vector &moveDirection, const float maxSpeed=1.0f, const bool adjustSpeed = false);


		// Archiving
		virtual void				Archive( Archiver &arc );		 
		void						DoArchive( Archiver &arc , Actor *actor );

		// Accessors and Mutators
		void						setStep( const Vector &step );
		const Vector &				getStep() const;
		
		void						setLastMove( stepmoveresult_t lastMove );
		stepmoveresult_t			getLastMove();
		
		void						setMove( const Vector &move );
		Vector						getMove();

		void						setMoveDir( const Vector &moveDir );
		Vector						getMoveDir();

		void						setMoveVelocity( const Vector &moveVelocity );
		Vector						getMoveVelocity();

		void						setAnimDir( const Vector &animDir );
		Vector						getAnimDir();

		void						setDiveDir( const Vector &diveDir );
		Vector						getDiveDir();

		void						setStartPos( const Vector &startPos );
		Vector						getStartPos();

 		void						setTotalLen( float totalLen );
		float						getTotalLen();

		void						setTurnSpeed( float turnSpeed );
		float						getTurnSpeed();

 		void						setForwardSpeed( float forwardSpeed );
		float						getForwardSpeed();

		void						setMoveSpeed( float moveSpeed );
		float						getMoveSpeed();

 		void						setPath( Path* path );
		Path*						getPath();

		void						setFlipLegs( qboolean flip );
		qboolean					getFlipLegs();
		void						flipLegs();

		void						setMovingBackwards( qboolean backwards );  
		qboolean					getMovingBackwards();

		void						setFaceEnemy ( bool faceEnemy );
		bool						getFaceEnemy ();

		void						setAdjustAnimDir( bool adjustAnimDir );
		bool						getAdjustAnimDir();


		void						setMovementType( MovementType_t mType );
		MovementType_t				getMovementType();

		void						SetStickToGround( const bool stick );
		const bool					GetStickToGround( void ) const;

		void						setUseCodeDrivenSpeed( bool useCode );
		bool						getUseCodeDrivenSpeed();

		Entity*						getBlockingEntity();
		void						clearBlockingEntity();

		 
   protected: // Functions
		
		Vector						_getRealDestinationPosition( const Vector &pos ) const;
		stepmoveresult_t			_noGravityTryMove( const Vector &oldorg, trace_t &verticalTrace ) const;
		trace_t						_feetWidthTrace( const Vector &currentLoc , const Vector &bottom , const Vector &endPos, const int mask ) const;
		float						_getTraceStep() const;
		void						_saveGroundInformation(trace_t &trace);
		qboolean					_isBlockedByDoor(trace_t &trace ) const;		
		qboolean					_canMoveSimplePath(const Vector &mins, const Vector &maxs, const Vector &pos ) const;
		qboolean					_isBlockedByFall(trace_t &trace ) const;
		qboolean					_allowFall() const;
		qboolean					_shouldTryMove() const;		
		qboolean					_checkHaveGroundEachStep( const Vector &start, const Vector &end, const Vector &test_mins , const Vector &test_maxs, const int mask = -1 ) const;
		void						_init();

	private: // Member Variables
		static Vector				_step;

		stepmoveresult_t			_lastmove;
		float						_forwardspeed;
		PathPtr						_path;
		Vector						_move;
		Vector						_movedir;
		float						_movespeed;
		Vector						_movevelocity;
		float						_totallen;
		float						_turnspeed;
		Vector						_animdir;
		Vector						_divedir;
		Vector						_startpos;
		qboolean					_fliplegs;
		qboolean					_movingBackwards;
		bool						_faceEnemy;
		bool						_adjustAnimDir;
		MovementType_t				_movementType;		
		bool						_stickToGround;
		bool						_useCodeDrivenSpeed;

		Actor						*act;
		EntityPtr					_blockingEntity;

	};

inline void MovementSubsystem::setUseCodeDrivenSpeed( bool useCode )
{
	_useCodeDrivenSpeed = useCode;
}

inline bool MovementSubsystem::getUseCodeDrivenSpeed()
{
	return _useCodeDrivenSpeed;
}

#endif /* __ACTOR_LOCOMOTION_H__ */
