//
//  $Logfile:: /Code/DLLs/game/actor_locomotion.cpp                           $
// $Revision:: 47                                                             $
//   $Author:: Sketcher                                                       $
//     $Date:: 4/26/03 4:24p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//

#include "_pch_cpp.h"
#include "actor_locomotion.h"
#include "player.h"
#include "object.h"

//======================================
// LocomotionController Implementation
//=====================================


//
// Name:        LocomotionController()
// Class:       LocomotionController
//
// Description: Default Constructor
//
// Parameters:  None
//
// Returns:     None
//
LocomotionController::LocomotionController()
{
	// Should always use other constructor
	gi.Error( ERR_FATAL, "LocomotionController::LocomotionController -- Default Constructor Called" );	
}


//
// Name:        LocomotionController()
// Class:       LocomotionController
//
// Description: Default Constructor
//
// Parameters:  Actor *actor
//
// Returns:     None
//
LocomotionController::LocomotionController( Actor *actor )
{
	//Initialize our Actor
	if ( actor )
		act = actor;
	else 
		gi.Error( ERR_DROP, "LocomotionController::LocomotionController -- actor is NULL" );   
	
	_init();
}


//
// Name:        ~LocomotionController()
// Class:        LocomotionController()
//
// Description:  Destructor
//
// Parameters:   None
//
// Returns:      None
//
LocomotionController::~LocomotionController()
{
	
}


//
// Name:        Begin()
// Class:       LocomotionController
//
// Description: Begins a session of locomotion
//
// Parameters:  None
//
// Returns:     None
//
void LocomotionController::Begin()
{
	_chase.Begin( *act );
}


//
// Name:        Evaluate()
// Class:       LocomotionController
//
// Description: Evaluation Iteration for the locomotion session
//
// Parameters:  None
//
// Returns:     None
//
void LocomotionController::Evaluate()
{
	_chase.Evaluate( *act );
}


//
// Name:        End()
// Class:       LocomotionController
//
// Description: Ends a locomotion session
//
// Parameters:  None
//
// Returns:     None
//
void LocomotionController::End()
{
	_chase.End( *act );
}


//
// Name:        SetMovementStyle()
// Class:       LocomotionController
//
// Description: Sets the _movementStyle
//
// Parameters:  MovementStyle Style -- The style to set
//
// Returns:     None
//
void LocomotionController::SetMovementStyle( MovementStyle style )
{
	_movementStyle = style;
}


//
// Name:        GetMovementStyle()
// Class:       LocomotionController
//
// Description: Gets the _movementStyle
//
// Parameters:  None
//
// Returns:     _movementStyle
//
MovementStyle LocomotionController::GetMovementStyle()
{
	return _movementStyle;
}


//
// Name:        DoArchive()
// Class:       LocomotionController
//
// Description: Sets the Actor pointer and calls Archive()
//
// Parameters:  Archiver &arc   -- The archiver object
//              Actor    *actor -- The actor
//
// Returns:     None
//
void LocomotionController::DoArchive( Archiver &arc , Actor *actor )
{
	Archive( arc );
	if ( actor )
		act = actor;
	else
		gi.Error( ERR_FATAL, "LocomotionController::DoArchive -- actor is NULL" );	
}

//
// Name:        Archive()
// Class:       LocomotionController
//
// Description: Archives the class
//
// Parameters:  Archiver &arc -- The archiver object
//
// Returns:     None
//
void LocomotionController::Archive( Archiver &arc )
{	
	ArchiveEnum( _movementStyle, MovementStyle );
	arc.ArchiveObject( &_chase );
}


//
// Name:        _init()
// Class:       LocomotionController
//
// Description: Initializes the class
//
// Parameters:  None
//
// Returns:     None
//
void LocomotionController::_init()
{  
	_movementStyle = MOVEMENT_STYLE_NONE;   
}





//======================================
// MovementSubsystem Implementation
//=====================================

// Init Static Vars
Vector MovementSubsystem::_step = Vector( 0.0f, 0.0f, STEPSIZE );


//
// Name:         MovementSubsystem()
// Class:        MovementSubsystem
//
// Description:  Default Constructor
//
// Parameters:   None
//
// Returns:      None
//
MovementSubsystem::MovementSubsystem()
{
	// Should always use other constructor
	gi.Error( ERR_FATAL, "MovementSubsystem::MovementSubsystem -- Default Constructor Called" );	
}


//
// Name:         MovementSubsystem()
// Class:        MovementSubsystem
//
// Description:  Constructor
//
// Parameters:   Actor *actor
//
// Returns:      None
//
MovementSubsystem::MovementSubsystem( Actor *actor )
{
	//Initialize our Actor
	if ( actor )
		act = actor;
	else 
		gi.Error( ERR_DROP, "MovementSubsystem::MovementSubsystem -- actor is NULL" );
	
	_init();
}


//
// Name:        ~MovementSubsystem()
// Class:        MovementSubsystem
//
// Description:  Destructor
//
// Parameters:   None
//
// Returns:      None
//
MovementSubsystem::~MovementSubsystem()
{
	if ( _path )
	{
		delete _path;
		_path = NULL;
	}
}


//
// Name:        CanMoveTo()
// Class:       MovementSubsystem
//
// Description: Pass through to _canMoveSimplePath()
//
// Parameters:  Vector &pos -- The location to check
//
// Returns:     True or False
//
qboolean MovementSubsystem::CanMoveTo ( const Vector &pos )
{
	return _canMoveSimplePath( act->mins, act->maxs , pos );
}


//
// Name:        CanWalkTowardsPoint()
// Class:       MovementSubsystem
//
// Description: Tests to see if Actor can move in the desired direction a small amount
//
// Parameters:  Vector	direction          -- Desired direction of movement
//
// Returns: whether the direction is clear and has a floor
//

bool MovementSubsystem::CanWalkTowardsPoint( const Vector &goalPoint, const int mask )
{
	Vector step( goalPoint - act->origin ); 
	const float distance = Vector::Distance( goalPoint, act->origin ); 
	step.normalize();
	step *= min( distance, max ( 64.0f, Vector::Distance( act->origin, act->last_origin ) ) );
	return CanWalkTo( act->origin + step, 0, ENTITYNUM_NONE, mask ) == 1;
}


//
// Name:        CanWalkTo()
// Class:       MovementSubsystem
//
// Description: Pass through to CanWalkToFrom()
//
// Parameters:  Vector &pos                -- The location to check
//              float   bounding_box_extra -- Additional volume to the bounding box
//              int     entnum             -- entity number 
//
// Returns:
//
qboolean MovementSubsystem::CanWalkTo( const Vector &pos, float bounding_box_extra, int entnum, const int mask )
{
	return CanWalkToFrom( act->origin, pos, bounding_box_extra, entnum, mask );
}


//
// Name:        CanWalkToFrom()
// Class:       MovementSubsystem
//
// Description: Checks if a location is reachable
//
// Parameters:  const Vector &origin             -- Starting position
//              const Vector &pos                -- Ending position
//                    float   bounding_box_extra -- Extra volume added to the bounding box
//                    int     entnum             -- Entity number
//
// Returns:     True or False
//
qboolean MovementSubsystem::CanWalkToFrom ( const Vector &origin, const Vector &pos, float bounding_box_extra, int entnum, const int mask )
{
	Vector	real_pos;
	Vector	test_mins;
	Vector	test_maxs;
	
	
	// Setup bounding box
	test_mins = act->mins;
	test_maxs = act->maxs;
	
	test_mins.x -= bounding_box_extra;
	test_mins.y -= bounding_box_extra;
	
	test_maxs.x += bounding_box_extra;
	test_maxs.y += bounding_box_extra;
	
	
	// Calculate the real position we have to get to
	if ( entnum != ENTITYNUM_NONE )
		real_pos = _getRealDestinationPosition( pos );	
	else
		real_pos = pos;
	
	Vector startPos;
	startPos = act->origin;
	startPos.z += 15;
	
	// Do simple CanWalkTo if specified
	
	if ( act->GetActorFlag( ACTOR_FLAG_SIMPLE_PATHFINDING ) )
		return _canMoveSimplePath( test_mins, test_maxs, real_pos );
	
	// Check to make sure the ground is good each step of the move
	return _checkHaveGroundEachStep( origin, real_pos, test_mins, test_maxs, mask );
	
	
}


//
// Name:        Acclerate()
// Class:       MovementSubsystem
//
// Description: Applies steering force to the actor's movement
//
// Parameters:  const Vector &original_steering -- steering force from the steering classes
//
// Returns:     None
//
void MovementSubsystem::Accelerate( const Vector &original_steering )
{
	Vector steering = original_steering;
	Vector newDir = _movedir.toAngles();
	
	
	// If we didn't move last frame ( stopped ) then we can just set our moveDir to where we
	// want to go, however, if we are moving, we want to adjust our turning based on our turnspeed
	// so that we can't just turn 90 degrees at a snap
	
	if ( !act->GetActorFlag(ACTOR_FLAG_HAVE_MOVED) )
	{
	/*  if ( steering.y > _turnspeed )
	steering.y = _turnspeed;
	else if ( steering.y < -_turnspeed )
	steering.y = -_turnspeed;        
		*/
	}
	
	if( steering.x )
		steering.x = steering.x;
	
	if( steering.y )
		newDir.y += steering.y;   
	
	newDir.EulerNormalize();
	
	
	if ( act->animate->frame_delta.x > 4.0f )
	{
		// make him lean into the turn a bit
		newDir.z = _movespeed * ( 0.4f / 320.0f ) * steering.y;
		
		if ( ( act->flags & FL_FLY ) || ( ( act->flags & FL_SWIM ) && act->waterlevel > 0 ) )
			newDir.z = bound( act->angles.z, -2.0f, 2.0f );
		else
			newDir.z = bound( act->angles.z, -5.0f, 5.0f );		
	}
	else
		newDir.z = 0.0f;
	
		/*
		if ( _movingBackwards )
		newDir[YAW] = AngleNormalize180( newDir[YAW] - 180.0f );
	*/
	
	newDir.AngleVectors( &_movedir );
	
	//Set my turn angles;
	Vector newAng = _animdir.toAngles();
	
	float newDirYaw;
	if ( act->bind_info && act->bind_info->bindmaster )
	{
		float				orientation[3][3];
		float				parentOrientation[3][3];
		float	mat[3][3];
		AnglesToAxis( newDir, mat );
		Vector				parentAngles;
		
		MatrixToEulerAngles( act->bind_info->bindmaster->orientation, parentAngles );
		parentAngles[ YAW ] = AngleNormalize180( -parentAngles[ YAW ] );
		AnglesToAxis( parentAngles, parentOrientation );
		
		R_ConcatRotations( mat, parentOrientation, orientation );
		MatrixToEulerAngles( orientation, newDir );
		
		
		AnglesToAxis( newAng, mat );
		R_ConcatRotations( mat, parentOrientation, orientation );
		MatrixToEulerAngles( orientation, newAng );
	}
	
	newDirYaw = AngleNormalize180(newDir.yaw() );
	float newAngYaw = AngleNormalize180(newAng.yaw() );
	
	
	if ( _movingBackwards )
		newDirYaw = AngleNormalize180( newDirYaw - 180.0f );   
	
	
	float AngleDiff =  newDirYaw - newAngYaw;
	AngleDiff = AngleNormalize180(AngleDiff);
	
	//First check if we're close enough just to set our angles
	if ( ( ( AngleDiff >= 0.0f )  && ( AngleDiff < _turnspeed ) ) || ( ( AngleDiff <= 0.0f ) && ( AngleDiff > -_turnspeed ) ) )
	{
		newAng[YAW] = AngleNormalize360(newDirYaw);
		
		if ( _faceEnemy )
		{
			Entity *currentEnemy;
			currentEnemy = act->enemyManager->GetCurrentEnemy();
			
			if ( !currentEnemy )
			{
				act->enemyManager->FindHighestHateEnemy();
				currentEnemy = act->enemyManager->GetCurrentEnemy();
			}	
			
			if ( currentEnemy )
			{
				Vector selfToEnemy;
				selfToEnemy = currentEnemy->origin - act->origin;
				selfToEnemy = selfToEnemy.toAngles();
				selfToEnemy[PITCH] = 0.0f;
				
				newAng = selfToEnemy;
			}
		}
		
		if ( _adjustAnimDir )
			act->setAngles( newAng );      
		return;
	}
	
	//Update our angles	
	if ( AngleDiff > 0.0f )
		newAng[YAW] = AngleNormalize360( newAng[YAW] += _turnspeed );
	else
		newAng[YAW] = AngleNormalize360( newAng[YAW] -= _turnspeed );
	
	
	
		/*
		if ( _fliplegs )
		newAng[YAW] += 180;
	*/
	
	
	
	if ( _movingBackwards )
		newAng[YAW] = AngleNormalize180( newDir[YAW] - 180.0f );
	
	
	if ( _faceEnemy )
	{
		Entity *currentEnemy;
		currentEnemy = act->enemyManager->GetCurrentEnemy();
		
		if ( !currentEnemy )
		{
			act->enemyManager->FindHighestHateEnemy();
			currentEnemy = act->enemyManager->GetCurrentEnemy();
		}	
		
		if ( currentEnemy )
		{
			Vector selfToEnemy;
			selfToEnemy = currentEnemy->origin - act->origin;
			selfToEnemy = selfToEnemy.toAngles();
			selfToEnemy[PITCH] = 0.0f;
			
			newAng = selfToEnemy;
		}
	}
	
	if ( _adjustAnimDir )
		act->setAngles( newAng );   
	
}

//--------------------------------------------------------------
// Name:        CalcMove()
// Class:       MovementSubsystem
//
// Description: Calculates the _move Vector
//
// Parameters:  None
//
// Returns:     None
//--------------------------------------------------------------
void MovementSubsystem::CalcMove ( void )
{
	
	// Use total_delta from the animation if we can, but
	// over-ride it if we set a forward speed ( When the 
	// new animation stuff is availiable )
	_totallen = 0;
	
	if ( act->total_delta != vec_zero )
		_totallen = act->total_delta.length();
	
	if ( _forwardspeed )
	{
		_totallen = _forwardspeed;
	}
	
	if ( _movementType == MOVEMENT_TYPE_ANIM )
	{
		MatrixTransformVector( act->total_delta, act->orientation, _move );
	}
	else
	{
		_movedir.normalize();
		_move = _movedir;
		_move *= _totallen;
	}
	
	// If we are not allowed to move, make sure we set the length of our 
	// movement vector to 0.0
	if ( act->movetype == MOVETYPE_NONE )
		_move *= 0.0f;
	
	act->total_delta = vec_zero;
	_animdir = act->orientation[0];  
	
}


//
// Name:        WaterMove()
// Class:       MovementSubsystem
//
// Description: Attempts to move based on being in water
//
// Parameters:  None
//
// Returns:     stepmoveresult_t
//
stepmoveresult_t MovementSubsystem::WaterMove ( void )
{
	Vector	oldorg;
	Vector	neworg;
	trace_t	trace;
	int      oldwater;
	
	if ( ( _totallen <= 0.01f ) || ( _move == vec_zero ) )
	{
		return STEPMOVE_OK;
	}
	
	// try the move
	oldorg = act->origin;
	neworg = act->origin + _move;
	
	trace = G_Trace( oldorg, act->mins, act->maxs, neworg, act, act->edict->clipmask, false, "Actor::WaterMove 1" );
	if ( trace.fraction == 0.0f )
		return STEPMOVE_STUCK;
	
	oldwater = act->waterlevel;
	
	act->setOrigin( trace.endpos );
	
	CheckWater();
	
	// swim monsters don't exit water voluntarily
	if ( ( oldwater > 1 ) && ( act->waterlevel < 2 ) )
	   {
		act->waterlevel = oldwater;
		act->setOrigin( oldorg );
		return STEPMOVE_STUCK;
   	}
	
	return STEPMOVE_OK;
}


//
// Name:        AirMove()
// Class:       MovementSubsystem
//
// Description: Attempts to move based on being in the air
//
// Parameters:  None
//
// Returns:     stepmoveresult_t
//
stepmoveresult_t MovementSubsystem::AirMove ( void )
{
	Vector	oldorg;
	Vector	neworg;
	trace_t	trace;
	int      oldwater;
	
	if ( ( _totallen <= 0.01f ) || ( _move == vec_zero ) )
	{
		return STEPMOVE_OK;
	}
	
	// try the move
	oldorg = act->origin;
	neworg = act->origin + _move;
	
	trace = G_Trace( oldorg, act->mins, act->maxs, neworg, act, act->edict->clipmask, false, "Actor::AirMove 1" );
	if ( trace.fraction < 0.0001f )
	{
		return STEPMOVE_BLOCKED_BY_WATER;
	}
	
	oldwater = act->waterlevel;
	
	act->setOrigin( trace.endpos );
	
	if ( !act->GetActorFlag( ACTOR_FLAG_IGNORE_WATER ) )
	{
		CheckWater();
		
		// fly monsters don't enter water voluntarily
		if ( !oldwater && act->waterlevel )
		{
			act->waterlevel = oldwater;
			act->setOrigin( oldorg );
			return STEPMOVE_STUCK;
		}
	}
	
	return STEPMOVE_OK;
}


//
// Name:        TryMove()
// Class:       MovementSubsystem
//
// Description: Tries to move based on being on the ground
//
// Parameters:  None
//
// Returns:     stepmoveresult_t
//
stepmoveresult_t MovementSubsystem::IsMoveValid( trace_t &horizontalTrace, trace_t &verticalTrace, const Vector &moveBegin, const Vector &moveEnd )
{
	horizontalTrace = act->Trace( moveBegin, moveEnd, "MovementSubsystem::IsMoveValid" );
	
	if ( horizontalTrace.startsolid )
	{
		horizontalTrace = G_Trace( moveBegin, act->mins, act->maxs, moveBegin + _move, act, act->edict->clipmask, false, "MovementSubsystem::IsMoveValid2" );
	}
	
	if ( horizontalTrace.startsolid || ( horizontalTrace.fraction < 0.0001f ) )
		return STEPMOVE_STUCK;      
	
	// See if we are blocked by a door
	if ( _isBlockedByDoor(horizontalTrace) )
		return STEPMOVE_BLOCKED_BY_DOOR;
	
	// Don't step down an extra step if gravity is turned off for this actor right now or the actor is dead
	if ( !act->GetActorFlag( ACTOR_FLAG_USE_GRAVITY ) || (act->gravity == 0) || act->deadflag )
	{
		return _noGravityTryMove( horizontalTrace.endpos, verticalTrace );
	}
	
	stepmoveresult_t result = STEPMOVE_OK;
	if ( horizontalTrace.fraction < 1.0f )
	{
		if ( horizontalTrace.entityNum == ENTITYNUM_WORLD )
		{
			result = STEPMOVE_BLOCKED_BY_WORLD;
		}
		else
		{
			result = STEPMOVE_BLOCKED_BY_ENTITY;
			_blockingEntity = horizontalTrace.ent->entity;
			//if ( _blockingEntity->isSubclassOf( Player ) )
			//	act->InContext( "blockedbyplayer" );
			
			act->AddStateFlag(STATE_FLAG_BLOCKED_BY_ENTITY);
		}
	}
	
	// Phase 2:  Send a trace down from the end of the first trace to try and found ground -- and thus
	// what should be our new origin
	Vector traceBegin( horizontalTrace.endpos );
	Vector traceEnd( traceBegin - ( _step * 2.0f ) );
	
	verticalTrace = G_Trace( traceBegin, act->mins, act->maxs, traceEnd, act, act->edict->clipmask, false, "MovementSubsystem::IsMoveValid3" );
	// Check if we are blocked by a fall
	if ( _isBlockedByFall( verticalTrace ) )
		return STEPMOVE_BLOCKED_BY_FALL;
	
	
	// If the feet width is set make sure the actor's feet are really on the ground
	if ( act->feet_width )
	{
		verticalTrace = _feetWidthTrace( verticalTrace.endpos , verticalTrace.endpos - ( _step * 3.0f ) , verticalTrace.endpos, act->edict->clipmask );
		
		// Check if we are blocked by a fall
		if ( _isBlockedByFall( verticalTrace ) )
			return STEPMOVE_BLOCKED_BY_FALL;
		
	}
	
	/* Experiment to deal with uneven floor geometry
	if ( result == STEPMOVE_OK )
	{
		if ( !_checkHaveGroundEachStep ( moveBegin , moveEnd , act->mins , act->maxs ))
			return STEPMOVE_BLOCKED_BY_FALL;
	}	
	*/
	
	return result;
}
//
// Name:        TryMove()
// Class:       MovementSubsystem
//
// Description: Tries to move based on being on the ground
//
// Parameters:  None
//
// Returns:     stepmoveresult_t
//
stepmoveresult_t MovementSubsystem::TryMove ( void	)
{
	
	// See if we should bother doing any movement
	if ( !_shouldTryMove() )
		return STEPMOVE_OK;
	
	
	// Phase 1:  Send a trace out from our origin ( plus stepsize ) along our move Vector.
	Vector moveBegin = act->origin;
	
	trace_t horizontalTrace;
	trace_t verticalTrace;
	verticalTrace.ent = 0 ;
	stepmoveresult_t returnValue = IsMoveValid( horizontalTrace, verticalTrace, moveBegin, moveBegin + _move );
	if ( returnValue == STEPMOVE_OK )
	{
		// The move is ok
		act->setOrigin( verticalTrace.endpos );
		
		// Save the ground information now so we don't have to do it later
		if ( verticalTrace.fraction < 1.0f )
			_saveGroundInformation( verticalTrace );
		
		
		act->flags &= ~FL_PARTIALGROUND;  // set in ActorThink
		CheckWater();
		
	}
	else if ( returnValue == STEPMOVE_BLOCKED_BY_FALL )
	{
		act->AddStateFlag( STATE_FLAG_STUCK );
	}
	return returnValue;
}


//
// Name:        SimpleMove()
// Class:       MovementSubsystem
//
// Description: Move with no collision and no ground following
//
// Parameters:  None
//
// Returns:     stepmoveresult_t
//
stepmoveresult_t MovementSubsystem::SimpleMove( const bool stickToGround )
{
	Vector newPosition ( act->origin + _move );
	if ( stickToGround )
	{
		Vector traceBegin( newPosition + _step );
		Vector traceEnd( traceBegin - ( _step * 2.0f ) );
		
		trace_t trace = G_Trace( traceBegin, act->mins, act->maxs, traceEnd, act, MASK_PATHSOLID, false, "MovementSubsystem::SimpleMove" );
		newPosition = trace.endpos;
	}
	act->setOrigin( newPosition );
	return STEPMOVE_OK;
}


//
// Name:        Push()
// Class:       MovementSubsystem
//
// Description: Moves the actor based on the push direction
//
// Parameters:  const Vector &dir -- The direction of the push
//
// Returns:     True or False
//
qboolean MovementSubsystem::Push ( const Vector &dir )
{
	Vector oldorg;
	Vector neworg;
	trace_t trace;
	int i;
	
	
	if ( !act->GetActorFlag( ACTOR_FLAG_PUSHABLE ) )
		return false;
	
	for( i = 0 ; i < 5 ; i++ )
	{
		oldorg = act->origin + _step;
		neworg = oldorg + dir;
		
		trace = G_Trace( oldorg, act->mins, act->maxs, neworg, act, act->edict->clipmask, false, "Actor::Push 1" );
		
		if ( trace.startsolid )
		{
			oldorg = act->origin;
			neworg = oldorg + dir;
			
			trace = G_Trace( oldorg, act->mins, act->maxs, neworg, act, act->edict->clipmask, false, "Actor::Push 2" );
			
			if ( trace.startsolid )
				return false;
		}
		
		if ( trace.ent && trace.ent->entity->isSubclassOf( Actor ) )
		{
			Actor *actor = (Actor *) trace.ent->entity;
			actor->Push( dir );
			continue;
		}
		else
			break;
	}
	
	if ( trace.endpos == oldorg )
		return false;
	
	// Step down to a step height below our original height to account for gravity
	
	oldorg = trace.endpos;
	
	if ( act->flags & FL_FLY )
		neworg = oldorg - _step;
	else
		neworg = oldorg - _step * 2.0f;
	
	trace = G_Trace( oldorg, act->mins, act->maxs, neworg, act, act->edict->clipmask, false, "Actor::Push 3" );
	
	act->setOrigin( trace.endpos );
	
	return true;
}


//
// Name:        CheckWater()
// Class:       MovementSubsystem
//
// Description: Sets the waterlevel
//              Waterlevel 1 means actor's feet are in the water
//              Waterlevel 2 means actor's waist is in the water
//              Waterlevel 3 means actor's eyes are in the water -- Important
//                           because at level 3, we start to choke.
//
// Parameters:  None
//
// Returns:     None
//
void MovementSubsystem::CheckWater( void )
{
	Vector sample[3];
	int cont;
	
	//
	// get waterlevel and type
	//
	act->waterlevel = 0;
	act->watertype = 0;
	
	sample[ 0 ] = act->origin;
	sample[ 2 ] = act->EyePosition();
	sample[ 1 ] = ( sample[ 0 ] + sample[ 2 ] ) * 0.5f;
	
	cont = gi.pointcontents( sample[ 0 ], 0 );
	
	if ( ( cont != -1 ) && ( cont & MASK_WATER ) )
	   {
		act->watertype = cont;
		act->waterlevel = 1;
		cont = gi.pointcontents( sample[ 2 ], 0 );
		if (cont & MASK_WATER)
		{
			act->waterlevel = 3;
		}
		else
		{
			cont = gi.pointcontents( sample[ 1 ], 0 );
			if (cont & MASK_WATER)
			{
				act->waterlevel = 2;
            }
		}
	}
}


//
// Name:        JumpTo()
// Class:       MovementSubsystem
//
// Description: Jumps the actor to the specified target
//
// Parameters:  const Vector &targ         -- The target
//                    float speed          -- Jump speed
//                    float vertical_speed -- Vertical speed
//
// Returns:     float traveltime
//
float MovementSubsystem::JumpTo( const Vector &targetPosition, const Angle angle )
{
	Trajectory trajectory( act->origin, targetPosition, angle, act->gravity * -sv_currentGravity->value );
	act->velocity = trajectory.GetInitialVelocity();
	
	Vector directionXY( targetPosition - act->origin );
	directionXY.z = 0.0f;
	act->setAngles( directionXY.toAngles() );
	_movedir = directionXY;
	
	act->groundentity = NULL;
	return trajectory.GetTravelTime();
}


//
// Name:        JumpTo()
// Class:       MovementSubsystem
//
// Description: Wrapper for JumpTo
//
// Parameters:  PathNode *goal       -- Jump target
//              float speed          -- Jump speed
//              float vertical_speed -- Vertical jump speed
//
// Returns:     float JumpTo result
//
float MovementSubsystem::JumpTo ( PathNode *goal, const Angle angle )
{
	if ( goal )
		return JumpTo( goal->origin, angle );
	else
		return 0;
}



//
// Name:        JumpTo()
// Class:       MovementSubsystem
//
// Description: Wrapper for JumpTo
//
// Parameters:  Entity *goal         -- Jump target
//              float speed          -- Jump speed
//              float vertical_speed -- Vertical jump speed
//
// Returns:     float JumpTo result
//
float MovementSubsystem::JumpTo ( Entity *goal, const Angle angle )
{
	if ( goal )
		return JumpTo( goal->origin, angle );
	else
		return 0;
}

//
// Name:        SteerTowardsPoint()
// Class:       MovementSubsystem
//
// Description: Basic Seek/Intercept Steering
//
// Parameters:  const Vector &targetPosition	- Point to steer towards
//              const Vector &targetVelocity - current velocity used to predict future position of target point
//              const Vector &moveDirection  - the actors current direction of movement
//					 const float maxSpeed			- maximum speed of actor
//
// Returns:     Euler angles containing rotation that will point actor in a direction to intercept the target
//
//				x - predictedPosition
//				A - myPosition( moveDirection is towards the point of the "A")
//				+ - centerOfTurn
//
//				  
//					   _x 
//					 /
//					|
//					A   + 
//			   
//
//
const Vector MovementSubsystem::SteerTowardsPoint( const Vector &targetPosition, const Vector &targetVelocity, const Vector &moveDirection, const float maxSpeed, const bool adjustSpeed)
{
	assert(act != NULL);
	Vector myPosition (act->origin);
	Vector predictedPosition = targetPosition + targetVelocity * ( Vector::Distance(targetPosition, myPosition) / maxSpeed );
	Vector desiredDirection = predictedPosition - myPosition;
	
	Vector newSteeringForce = Vector::AnglesBetween(desiredDirection, moveDirection);
	newSteeringForce[ROLL]=0.0f;
	newSteeringForce.EulerNormalize();
	
	if ( adjustSpeed )
	{
		const float currentSpeed = _forwardspeed;
		const float turnRate = _turnspeed;
		const float turnRadius = currentSpeed / turnRate;
		
		Vector right;
		right.CrossProduct( moveDirection, Vector( 0, 0, 1 ) );
		const Vector centerOfTurn( myPosition + ( right * DotProduct( right, desiredDirection ) ) );
		
		const float forwardComponent = DotProduct( desiredDirection, moveDirection );
		const float rightComponent = DotProduct( desiredDirection, right );
		
		if ( Vector::DistanceXY( centerOfTurn, predictedPosition ) < turnRadius )
		{
			const float newTurnRadius = ( ( rightComponent * rightComponent ) + ( forwardComponent * forwardComponent ) ) / ( 2.0f * rightComponent );
			_forwardspeed = 2.0f * turnRate * newTurnRadius;
		}
	}
	return newSteeringForce;
}

//
// Name:        _canMoveSimplePath()
// Class:       MovementSubsystem
//
// Description: Checks if an Actor can do a simple move to the specified destination
//
// Parameters:  const Vector &mins -- mins of the actor
//              const Vector &maxs -- maxs of the actor
//              const Vector &pos  -- target destination
//
// Returns:     True or False
//
qboolean MovementSubsystem::_canMoveSimplePath( const Vector &mins, const Vector &maxs , const Vector &pos ) const
{
	trace_t trace = act->Trace( act->origin + _step, pos + _step, "Actor::_canMoveSimplePath" );
	if ( trace.fraction == 1.0f )
		return true;
	
	return false;
}


//
// Name:        _getRealDestinationPosition()
// Class:       MovementSubsystem
//
// Description: uses the mins and maxs to calculate the actual final position
//
// Parameters:  const Vector &pos
//
// Returns:     Vector target destination
//
Vector MovementSubsystem::_getRealDestinationPosition( const Vector &pos ) const
{
	Vector temp_dir = pos - act->origin;
	float temp_length = temp_dir.length();
	
	temp_length -= sqrt( act->maxs.x * act->maxs.x * 2.0f ) + 5.0f;
	
	if ( temp_length < 0.0f )
		temp_length = 0.0f;
	
	temp_dir.normalize();
	temp_dir *= temp_length;
	
	return act->origin + temp_dir;
}


//
// Name:        _feetWidthTrace()
// Class:       MovementSubsystem
//
// Description: Does a trace based off of the actor's feet -- to help prevent corner
//              of the boundingbox Wile E Coyote problems.
//
// Parameters:  const Vector &currentLoc -- Starting position
//              const Vector &bottom     -- The end point of the trace
//              const Vector &endPos     -- The end position being copied over
//                            
// Returns:     trace_t trace
//
trace_t MovementSubsystem::_feetWidthTrace( const Vector &currentLoc, const Vector &bottom, const Vector &endPos, const int mask ) const
{
	trace_t trace;
	Vector temp_mins;
	Vector temp_maxs;
	Vector saved_endpos;
	
	temp_mins[0] = -act->feet_width;
	temp_mins[1] = -act->feet_width;
	temp_mins[2] = act->mins[2];
	
	temp_maxs[0] = act->feet_width;
	temp_maxs[1] = act->feet_width;
	temp_maxs[2] = act->maxs[2];
	
	trace = G_Trace( currentLoc, temp_mins, temp_maxs, bottom, act, mask, false, "Actor::CanWalkTo2" );
	
	saved_endpos = endPos;
	saved_endpos.copyTo( trace.endpos );
	
	return trace;
}


//
// Name:        _getTraceStep()
// Class:       MovementSubsystem
//
// Description: Calculates the trace step size
//
// Parameters:  None
//
// Returns:     float trace_step
//
float MovementSubsystem::_getTraceStep() const
{
	// Find the step amount
	float trace_step = act->maxs[0] * 2.0f; //was 2.0
	
	// Make sure trace_step is divisible by 8
	trace_step = ( (int)(trace_step / 8.0f ) ) * 8.0f;
	
	if ( trace_step < 8.0f )
		trace_step = 8.0f;
	
	return trace_step;
}


//
// Name:        _checkHaveGroundEachStep()
// Class:       MovementSubsystem
//
// Description: Checks each step required to traverse the vector, and checks if there is ground to step on
//
// Parameters:  const Vector &start     -- starting position
//              const Vector &end       -- target destination
// 				 const Vector &test_mins -- mins used in the check
//              const Vector &test_maxs -- maxs used in the check
//
//
// Returns:     True or False
//
qboolean MovementSubsystem::_checkHaveGroundEachStep( const Vector &start, const Vector &end, const Vector &test_mins, const Vector &test_maxs, const int mask ) const
{
	int clipMask = act->edict->clipmask;
	if ( mask >= 0 )
	{
		clipMask = mask;
	}
	
	// Find the vector to walk
	Vector dir = end - start;
	float length = dir.length();
	dir.normalize();
	
	// Get Trace Steps;
	float trace_step = _getTraceStep();
	float small_trace_step = 8;
	
	// Test each step to see if the ground is not too far below
	float last_height = end[2];
	
	Vector last_loc = Vector::Identity();
	//	Vector last_loc = start;
	for( float i = 0 ; i < length ; i += trace_step )
	{
		Vector current_loc    = start + ( dir * i );
		current_loc[2] = last_height + STEPSIZE;
		
		Vector bottom    = current_loc;
		
		if ( !act->GetActorFlag( ACTOR_FLAG_ALLOW_FALL ) )
			bottom[2] = last_height - STEPSIZE;
		else
			bottom[2] = last_height - 1000.0f;
		
		trace_t trace = G_Trace( current_loc, test_mins, test_maxs, bottom, act, clipMask, false, "Actor::CanWalkTo1" );
		
		if ( !( ( trace.fraction == 1.0f ) || trace.startsolid || trace.allsolid ) && act->feet_width )
			trace = _feetWidthTrace( current_loc , bottom , trace.endpos, clipMask );
		
		if ( ( trace.fraction == 1.0f ) || trace.startsolid || trace.allsolid )
		{
			// The wide one failed, do the small traces for this segment
			if ( ( i == 0 ) || ( trace_step == small_trace_step ) )
				return false;
			
			for( float j = small_trace_step ; j <= trace_step ; j += small_trace_step )
			{
				current_loc    = last_loc + ( dir * j );
				current_loc[2] = last_height + STEPSIZE;
				
				bottom    = current_loc;
				bottom[2] = last_height - STEPSIZE;
				
				trace = G_Trace( current_loc, test_mins, test_maxs, bottom, act, clipMask, false, "Actor::CanWalkTo3" );
				
				if ( ( trace.fraction == 1.0f ) || trace.startsolid || trace.allsolid ||
					( trace.ent && trace.ent->entity->isSubclassOf( Sentient ) && !act->GetActorFlag( ACTOR_FLAG_CAN_WALK_ON_OTHERS ) ) )
					return false;
				
				if ( act->feet_width )
					trace = _feetWidthTrace( current_loc , bottom , trace.endpos, clipMask );
				
				last_height = trace.endpos[2];
			}
		}
		
		last_height = trace.endpos[2];
		last_loc = current_loc;
	}
	
	if ( ( last_height > ( end.z + ( STEPSIZE * 2.0f ) ) ) || ( last_height < ( end.z - ( STEPSIZE * 2.0f ) ) ) )
		return false;
	
	return true;
}


//
// Name:        _isBlockedByDoor()
// Class:       MovementSubsystem
//
// Description: Checks if the actor is blocked by a door
//
// Parameters:  trace_t &trace -- the trace 
//
// Returns:     True or False
//
qboolean MovementSubsystem::_isBlockedByDoor( trace_t &trace ) const
{
	Door     *door;
	
	if ( !act->deadflag && trace.ent )
	{
		// Check if we hit a door
		
		if ( trace.ent->entity->isSubclassOf( Door ) )
		{
			door = ( Door * )trace.ent->entity;
			if ( !door->locked && !door->isOpen() )
				return true;
		}
	}
	
	return false;
}


//
// Name:        _noGravityTryMove()
// Class:       MovementSubsystem
//
// Description: Traces to see if actor can be positioned in space, without worrying if 
//              ground is underneath -- Should be used by flying creatures
//
// Parameters:  const Vector &oldorg -- The origin before trying the move
//
// Returns:     stepmoveresult_t
//
stepmoveresult_t MovementSubsystem::_noGravityTryMove( const Vector &oldorg, trace_t &verticalTrace ) const
{
	Vector neworg = oldorg - _step;
	//trace_t newTrace;
	
	// try stepping down
	verticalTrace = G_Trace( oldorg, act->mins, act->maxs, neworg, act, act->edict->clipmask, false, "Actor::TryMove 2" );
	
	if ( verticalTrace.startsolid )
		return STEPMOVE_STUCK;
	
	
	//act->setOrigin( verticalTrace.endpos );
	
	return STEPMOVE_OK;
}


//
// Name:        _isBlockedByFall()
// Class:       MovementSubsystem
//
// Description: Checks if the Actor is blocked by fall
//
// Parameters:  trace_t &trace
//
// Returns:     True or False
//
qboolean MovementSubsystem::_isBlockedByFall( trace_t &trace ) const
{
	// Determine if we should allow the actor to fall
	qboolean allow_fall = _allowFall();
	
	// We never want to step on a flying creature, even if we are allowed to
	// step on sentients in general.  This is because, if we were to step on a flying
	// creature, and the flying creature moved, then we might fall, where had we not
	// stepped on the flying creature we would still be safe and sound
	if ( trace.ent && ( trace.ent->entity->flags & FL_FLY ) && !allow_fall )
		return true;
	
	// Don't voluntarilty step on sentients
	if ( trace.ent && trace.ent->entity->isSubclassOf( Sentient ) &&
		!allow_fall && !act->GetActorFlag( ACTOR_FLAG_CAN_WALK_ON_OTHERS ) )
		return true;
	
	// Check if the move places us on solid ground
	if ( trace.fraction == 1.0f )
	{
		if ( allow_fall )
		{
			// don't let guys get stuck standing on other guys
			// if monster had the ground pulled out, go ahead and fall
			act->groundentity = NULL;
			return false;
		}
		else
		{
			// walked off an edge
			return true;
		}
	}
	
	// Make sure ground is not too slopped or we will just slide off
	
	if ( ( trace.plane.normal[ 2 ] <= 0.7f ) && !allow_fall )
		return true;
	
	return false;
}



//
// Name:        _allowFall()
// Class:       MovementSubsystem
//
// Description: Checks if the actor is allowed to fall
//
// Parameters:  None
//
// Returns:     True or False
//
qboolean MovementSubsystem::_allowFall() const
{
	if ( ( act->flags & FL_PARTIALGROUND ) ||
		( act->groundentity && act->groundentity->entity && ( act->groundentity->entity->isSubclassOf( Sentient ) ) ) ||
		( act->GetActorFlag( ACTOR_FLAG_ALLOW_FALL ) ) )
		return true;
	
	return false;
}


//
// Name:        _shouldTryMove()
// Class:       MovementSubsystem
//
// Description: Checks if the Actor should even try to move
//
// Parameters:  None
//
// Returns:     True or False
//
qboolean MovementSubsystem::_shouldTryMove() const
{
	// We have a velocity so movement of the actor is done in physics
	if ( ( act->velocity != vec_zero ) && !act->deadflag )
		return false;
	
	if ( ( _totallen <= 0.01f ) || ( _move == vec_zero ) )
		return false;
	
	return true;
}


//
// Name:        _saveGroundInformation()
// Class:       MovementSubsystem
//
// Description: Saves trace information on the actor
//
// Parameters:  trace_t &trace
//
// Returns:     None
//
void MovementSubsystem::_saveGroundInformation(trace_t &trace)
{
	act->groundentity = trace.ent;
	act->groundplane = trace.plane;
	act->groundcontents = trace.contents;
	act->last_origin = act->origin;
	act->SetActorFlag( ACTOR_FLAG_HAVE_MOVED, true );
}


//
// Name:        _init()
// Class:       MovementSubsystem
//
// Description: Initializes the class
//
// Parameters:  None
//
// Returns:     None
//
void MovementSubsystem::_init()
{
	_lastmove = STEPMOVE_OK;
	_path     = NULL;
	_turnspeed = TURN_SPEED;
	_startpos  = act->origin;
	_forwardspeed = 0;
	_divedir = vec_zero;
	act->angles.AngleVectors( &_movedir );
	
	//Set our internal step var
	_step = Vector( 0.0f, 0.0f, STEPSIZE );
	_movespeed = 1.0f;
	
	_fliplegs = false;
	_movingBackwards = false;
	_faceEnemy = false;
	_adjustAnimDir = true;
	_useCodeDrivenSpeed = false;
	
	_movementType = MOVEMENT_TYPE_NORMAL;     
	_stickToGround = true;
	CheckWater();
	
}


//
// Name:        setMove()
// Class:       MovementSubsystem
//
// Description: Sets the _move
//
// Parameters:  const Vector &move -- The move to set
//
// Returns:     None
//
void MovementSubsystem::setMove( const Vector &move )
{
	_move = move;
}


//
// Name:        getMove()
// Class:       MovementSubsystem
//
// Description: Returns _move
//
// Parameters:  None
//
// Returns:     Vector _move
//
Vector MovementSubsystem::getMove()
{
	return _move;
}



//
// Name:        setMoveDir()
// Class:       MovementSubsystem
//
// Description: Sets _moveDir
//
// Parameters:  const Vector &moveDir -- The move dir to set
//
// Returns:     None
//
void MovementSubsystem::setMoveDir( const Vector &moveDir )
{
	_movedir = moveDir;
}


//
// Name:        getMoveDir()
// Class:       MovementSubsystem
//
// Description: Returns _movedir
//
// Parameters:  None
//
// Returns:     Vector _movedir
//
Vector MovementSubsystem::getMoveDir()
{
	return _movedir;
}



//
// Name:        setMoveVelocity()
// Class:       const Vector &moveVelocity
//
// Description: Sets _movevelocity
//
// Parameters:  const Vector &moveVelocity -- the move velocity to set
//
// Returns:     None
//
void MovementSubsystem::setMoveVelocity( const Vector &moveVelocity )
{
	_movevelocity = moveVelocity;
}


//
// Name:        getMoveVelocity()
// Class:       MovementSubsystem
//
// Description: Returns _movevelocity
//
// Parameters:  None
//
// Returns:     None
//
Vector MovementSubsystem::getMoveVelocity()
{
	return _movevelocity;
}


//
// Name:        setAnimDir()
// Class:       MovementSubsystem
//
// Description: Sets _animdir
//
// Parameters:  const Vector &animDir
//
// Returns:     None
//
void MovementSubsystem::setAnimDir( const Vector &animDir )
{
	_animdir = animDir;
}


//
// Name:        getAnimDir()
// Class:       MovementSubsystem
//
// Description: Returns _animdir
//
// Parameters:  None
//
// Returns:     Vector _animdir
//
Vector MovementSubsystem::getAnimDir()
{
	return _animdir;
}


//
// Name:        setDiveDir()
// Class:       const Vector &diveDir
//
// Description: Sets _divedir
//
// Parameters:  const Vector &diveDir -- the dive dir to set
//
// Returns:     None
//
void MovementSubsystem::setDiveDir( const Vector &diveDir )
{
	_divedir = diveDir;
}


//
// Name:        getDiveDir()
// Class:       MovementSubsystem
//
// Description: returns _divedir
//
// Parameters:  None
//
// Returns:     Vector _divedir
//
Vector MovementSubsystem::getDiveDir()
{
	return _divedir;
}


//
// Name:        setStartPos()
// Class:       MovementSubsystem
//
// Description: Sets _startpos
//
// Parameters:  const Vector &startPos -- the start pos to set
//
// Returns:     None
//
void MovementSubsystem::setStartPos( const Vector &startPos )
{
	_startpos = startPos;
}


//
// Name:        getStartPos
// Class:       MovementSubsystem
//
// Description: Returns _startpos
//
// Parameters:  None
//
// Returns:     Vector _startpos
//
Vector MovementSubsystem::getStartPos()
{
	return _startpos;
}



//
// Name:        setTotalLen()
// Class:       MovementSubsystem
//
// Description: sets _totallen
//
// Parameters:  float totallen -- the total len to set
//
// Returns:     None
//
void MovementSubsystem::setTotalLen( float totalLen )
{
	_totallen = totalLen;
}


//
// Name:        getTotalLen()
// Class:       MovementSubsystem
//
// Description: Returns _totallen
//
// Parameters:  None
//
// Returns:     float _totallen
//
float MovementSubsystem::getTotalLen()
{
	return _totallen;
}


//
// Name:        setTurnSpeed()
// Class:       MovementSubsystem
//
// Description: Sets _turnspeed
//
// Parameters:  float turnSpeed -- the turn speed to set
//
// Returns:     None
//
void MovementSubsystem::setTurnSpeed( float turnSpeed )
{
	_turnspeed = turnSpeed;
}


//
// Name:        getTurnSpeed()
// Class:       MovementSubsystem
//
// Description: Returns _turnspeed
//
// Parameters:  None
//
// Returns:     float _turnspeed
//
float MovementSubsystem::getTurnSpeed()
{
	return _turnspeed;
}


//
// Name:        setForwardSpeed()
// Class:       MovementSubsystem
//
// Description: Sets _forwardspeed
//
// Parameters:  float forwardSpeed
//
// Returns:     None
//
void MovementSubsystem::setForwardSpeed( float forwardSpeed )
{
	_forwardspeed = forwardSpeed;
}


//
// Name:        getForwardSpeed()
// Class:       MovementSubsystem
//
// Description: Returns _forwardspeed
//
// Parameters:  None
//
// Returns:     float _forwardspeed
//
float MovementSubsystem::getForwardSpeed()
{
	return _forwardspeed;
}



//
// Name:        setMoveSpeed()
// Class:       MovementSubsystem
//
// Description: Sets _movespeed
//
// Parameters:  float moveSpeed
//
// Returns:     None
//
void MovementSubsystem::setMoveSpeed( float moveSpeed )
{
	_movespeed = moveSpeed;
}


//
// Name:        getMoveSpeed()
// Class:       MovementSubsystem
//
// Description: Returns _movespeed
//
// Parameters:  None
//
// Returns:     float _movespeed
//
float MovementSubsystem::getMoveSpeed()
{
	return _movespeed;
}


//
// Name:        setFlipLegs()
// Class:       MovementSubsystem
//
// Description: Sets _fliplegs
//
// Parameters:  qboolean flip
//
// Returns:     None
//
void MovementSubsystem::setFlipLegs( qboolean flip )
{
	_fliplegs = flip;
}


//
// Name:        getFlipLegs()
// Class:       MovementSubsystem
//
// Description: Returns _fliplegs
//
// Parameters:  None
//
// Returns:     qboolean _fliplegs
//
qboolean MovementSubsystem::getFlipLegs()
{
	return _fliplegs;
}


//
// Name:        flipLegs()
// Class:       MovementSubsystem
//
// Description: Turns the  actor around, and allows for backward movement
//
// Parameters:  None
//
// Returns:     None
//
void MovementSubsystem::flipLegs()
{
	Vector Angles;
	
	_fliplegs = !_fliplegs;
	
	
	if ( _fliplegs )
		_movingBackwards = true;
	else
		_movingBackwards = false;
	
	
	Angles = _animdir;
	Angles = Angles.toAngles();
	
	Angles[PITCH] = AngleNormalize180( Angles[PITCH] );
	Angles[ROLL]  = AngleNormalize180( Angles[ROLL]  );
	
	Angles[YAW]   = AngleNormalize180( Angles[YAW]  + 180.0f );
	
	Angles.AngleVectors( &_animdir );
	
	//act->setAngles( Angles );      
	
}


//
// Name:        setMovingBackwards()
// Class:       MovementSubsystem
//
// Description: Sets _movingBackwards
//
// Parameters:  qboolean backwards
//
// Returns:     None
//
void MovementSubsystem::setMovingBackwards( qboolean backwards )
{
	_movingBackwards = backwards;
}


void MovementSubsystem::setFaceEnemy( bool faceEnemy )
{
	_faceEnemy = faceEnemy;
}

void MovementSubsystem::setAdjustAnimDir( bool adjustAnimDir )
{
	_adjustAnimDir = adjustAnimDir;
}

bool MovementSubsystem::getAdjustAnimDir()
{
	return _adjustAnimDir;
}

//
// Name:        getMovingBackwards
// Class:       MovementSubsystem
//
// Description: Returns _movingBackwards
//
// Parameters:  None
//
// Returns:     None
//
qboolean MovementSubsystem::getMovingBackwards()
{
	return _movingBackwards;
}

bool MovementSubsystem::getFaceEnemy()
{
	return _faceEnemy;
}

//
// Name:        setPath()
// Class:       MovementSubsystem
//
// Description: Sets _path
//
// Parameters:  Path *path
//
// Returns:     None
//
void MovementSubsystem::setPath( Path *path )
{
	if ( _path && ( _path != path ) )
		delete _path;
	
	_path = path;
}


//
// Name:        getPath()
// Class:       MovementSubsystem
//
// Description: Returns _path
//
// Parameters:  None
//
// Returns:     Path *_path
//
Path* MovementSubsystem::getPath()
{
	return _path;
}

//
// Name:        setStep()
// Class:       MovementSubsystem
//
// Description: Sets _step
//
// Parameters:  const Vector & lastMove
//
// Returns:     None
//
void MovementSubsystem::setStep( const Vector &step )
{
	_step = step;
}


//
// Name:        getLastMove()
// Class:       MovementSubsystem
//
// Description: Returns _step
//
// Parameters:  None
//
// Returns:     stepmoveresult_t _lastmove
//
const Vector & MovementSubsystem::getStep() const
{
	return _step;
}

//
// Name:        setLastMove()
// Class:       MovementSubsystem
//
// Description: Sets _lastmove
//
// Parameters:  stepmoveresult_t lastMove
//
// Returns:     None
//
void MovementSubsystem::setLastMove( stepmoveresult_t lastMove )
{
	_lastmove = lastMove;
}


//
// Name:        getLastMove()
// Class:       MovementSubsystem
//
// Description: Returns _lastmove
//
// Parameters:  None
//
// Returns:     stepmoveresult_t _lastmove
//
stepmoveresult_t MovementSubsystem::getLastMove()
{
	return _lastmove;
}

//
// Name:        setMovementType()
// Class:       MovementSubsystem
//
// Description: Sets the _movementType
//
// Parameters:  MovementType_t mType -- the type to set
//
// Returns:     None
//
void MovementSubsystem::setMovementType( MovementType_t mType )
{
	_movementType = mType;
}

//
// Name:        getMovementType()
// Class:       MovementSubsystem
//
// Description: Returns the _movementType
//
// Parameters:  None
//
// Returns:     MovementType_t _movementType;
//
MovementType_t MovementSubsystem::getMovementType()
{
   return _movementType;
}

void MovementSubsystem::SetStickToGround( const bool stick )
{
	_stickToGround = stick;
}

const bool MovementSubsystem::GetStickToGround( void ) const
{
	return _stickToGround;
}

Entity* MovementSubsystem::getBlockingEntity()
{
	return _blockingEntity;
}

void MovementSubsystem::clearBlockingEntity()
{
	_blockingEntity = NULL;
}

//
// Name:        DoArchive()
// Class:       MovementSubsystem
//
// Description: Sets up the class for archiving
//
// Parameters:  Archiver &arc -- the archiving object
//              Actor *actor  -- the class' actor pointer
//
// Returns:     None
//
void MovementSubsystem::DoArchive( Archiver &arc , Actor *actor )
{
	Archive( arc );
	if ( actor )
		act = actor;
	else
		gi.Error( ERR_FATAL, "MovementSubsystem::DoArchive -- actor is NULL" );
}


//
// Name:        Archive()
// Class:       MovementSubsystem
//
// Description: Archives the class
//
// Parameters:  Archiver &arc -- the archiving object
//
// Returns:     None
//
void MovementSubsystem::Archive( Archiver &arc )
{
	// Don't archive
	//static Vector				_step;

	ArchiveEnum( _lastmove, stepmoveresult_t );
	arc.ArchiveFloat( &_forwardspeed );
	arc.ArchiveSafePointer( &_path );
	arc.ArchiveVector( &_move );
	arc.ArchiveVector( &_movedir );
	arc.ArchiveFloat( &_movespeed );
	arc.ArchiveVector( &_movevelocity );
	arc.ArchiveFloat( &_totallen );
	arc.ArchiveFloat( &_turnspeed );
	arc.ArchiveVector( &_animdir );
	arc.ArchiveVector( &_divedir );
	arc.ArchiveVector( &_startpos );
	arc.ArchiveBoolean( &_fliplegs );
	arc.ArchiveBoolean( &_movingBackwards );
	arc.ArchiveBool ( &_faceEnemy );
	arc.ArchiveBool	( &_adjustAnimDir );
	ArchiveEnum( _movementType, MovementType_t );
	arc.ArchiveBool( &_stickToGround );
	arc.ArchiveBool( &_useCodeDrivenSpeed );
	arc.ArchiveSafePointer( &_blockingEntity );
}
