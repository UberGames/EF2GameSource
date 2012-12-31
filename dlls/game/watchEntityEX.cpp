//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/watchEntityEX.cpp                          $
// $Revision:: 4                                                              $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//		Keeps the actor rotated to face the entity ( enemy by default ) for the specified
//		amount of time ( or forever, if time is -1 )
//	
// PARAMETERS:
//		_time      -- The time to keep rotated
//		_turnspeed -- How fast to rotate
//		_anim	   -- Animation to play while rotating
//		_ent	   -- Entity to watch ( Must be set by another behavior -- defaults to enemy )
//	
// ANIMATIONS:
//		Rotation Anim -- Parameter
//	
//--------------------------------------------------------------------------------

#include "actor.h"
#include "watchEntityEX.hpp"
#include <qcommon/gameplaymanager.h>

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, WatchEntityEX, NULL )
{
	{ &EV_Behavior_Args,		&WatchEntityEX::SetArgs		},
	{ &EV_Behavior_AnimDone,	&WatchEntityEX::AnimDone	}, 
	{ NULL, NULL }
};


//--------------------------------------------------------------
// Name:		WatchEntityEX()
// Class:		WatchEntityEX
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
WatchEntityEX::WatchEntityEX()
{
	_stance			= "";
	_shuffleAnim	= "";
	_torsoAnim		= "";
	_ent			= NULL;
	_time			= 0.0;
	_turnspeed		= 0.0;	
}

//--------------------------------------------------------------
// Name:		~WatchEntityEX()
// Class:		WatchEntityEX
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
WatchEntityEX::~WatchEntityEX()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       WatchEntityEX
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void WatchEntityEX::SetArgs( Event *ev )
{
	int numargs = ev->NumArgs();

	if ( numargs > 0 ) _stance		= ev->GetString( 1 );
	if ( numargs > 1 ) _torsoAnim	= ev->GetString( 2 );
	if ( numargs > 2 ) _shuffleAnim	= ev->GetString( 3 );
	if ( numargs > 3 ) _turnspeed	= ev->GetFloat ( 4 );
	if ( numargs > 4 ) _time		= ev->GetFloat ( 5 );
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       WatchEntityEX
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void WatchEntityEX::AnimDone( Event * )
{
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       WatchEntityEX
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void WatchEntityEX::Begin( Actor &self )
{       
	init( self );
}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       WatchEntityEX
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t WatchEntityEX::Evaluate( Actor & )
{  
	BehaviorReturnCode_t stateResult;

	think();
	
	switch ( _state )
		{
		//---------------------------------------------------------------------
		case WATCH_ENTITY_EX_SETUP:
		//---------------------------------------------------------------------
			stateResult = evaluateStateSetup();

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WATCH_ENTITY_EX_ROTATE );

			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WATCH_ENTITY_EX_FAILED );
		break;

		//---------------------------------------------------------------------
		case WATCH_ENTITY_EX_ROTATE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateRotate();

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WATCH_ENTITY_EX_HOLD );
			
			if ( stateResult == BEHAVIOR_FAILED)
				transitionToState( WATCH_ENTITY_EX_FAILED );
		break;


		//---------------------------------------------------------------------
		case WATCH_ENTITY_EX_HOLD:
		//---------------------------------------------------------------------
			stateResult = evaluateStateHold();

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( WATCH_ENTITY_EX_ROTATE );

			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( WATCH_ENTITY_EX_FAILED );
		break;


		//---------------------------------------------------------------------
		case WATCH_ENTITY_EX_SUCCESS:
		//---------------------------------------------------------------------
			return BEHAVIOR_SUCCESS;

		break;


		//---------------------------------------------------------------------
		case WATCH_ENTITY_EX_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;

		break;


		}
	

	return BEHAVIOR_EVALUATING;   

}



//--------------------------------------------------------------
// Name:        End()
// Class:       WatchEntityEX
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void WatchEntityEX::End(Actor &)
{   
}


//--------------------------------------------------------------
// Name:        init()
// Class:       WatchEntityEX
//
// Description: Initializes memeber variables
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void WatchEntityEX::init( Actor &self )
{	
	if ( _time > 0 )
		_time = level.time + _time;

	if ( !_ent )
		{
		self.enemyManager->FindHighestHateEnemy();
		_ent = self.enemyManager->GetCurrentEnemy();
		}

	_stance = self.GetStateVar( _stance );

	if ( !_stance.length() )
		_stance = "idle";

	transitionToState( WATCH_ENTITY_EX_SETUP );

}


void WatchEntityEX::transitionToState( watchEntityStates_t state )
{
	switch ( state )
		{	
		case WATCH_ENTITY_EX_SETUP:
			setupStateSetup();
			setInternalState( state , "WATCH_ENTITY_EX_SETUP" );
		break;

		case WATCH_ENTITY_EX_ROTATE:
			setupStateRotate();
			setInternalState( state , "WATCH_ENTITY_EX_ROTATE" );
		break;

		case WATCH_ENTITY_EX_HOLD:
			setupStateHold();
			setInternalState( state , "WATCH_ENTITY_EX_HOLD" );
		break;

		case WATCH_ENTITY_EX_SUCCESS:
			setInternalState( state , "WATCH_ENTITY_EX_SUCCESS" );
		break;

		case WATCH_ENTITY_EX_FAILED:
			setInternalState( state , "WATCH_ENTITY_EX_FAILED" );
		break;
		}
}

void WatchEntityEX::setInternalState( watchEntityStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}


void WatchEntityEX::think()
{
	if ( _time > 0 && level.time >= _time )
		transitionToState( WATCH_ENTITY_EX_SUCCESS );
}



void WatchEntityEX::setupStateSetup()
{	
}

BehaviorReturnCode_t WatchEntityEX::evaluateStateSetup()
{
	if ( !_ent )
		{
		failureStateSetup("No Entity To Rotate To" );
		return BEHAVIOR_FAILED;
		}

	if ( _torsoAnim.length() )
		GetSelf()->SetAnim( _torsoAnim , NULL , torso );

	return BEHAVIOR_SUCCESS;   
}

void WatchEntityEX::failureStateSetup( const str& failureReason )
{
	Q_UNUSED(failureReason);
}



void WatchEntityEX::setupStateRotate()
{
	_rotateToEntity.SetEntity( _ent );
	_rotateToEntity.SetAnim( _shuffleAnim );
	_rotateToEntity.SetTurnSpeed( _turnspeed );	

	_rotateToEntity.Begin( *GetSelf() );
}

BehaviorReturnCode_t WatchEntityEX::evaluateStateRotate()
{
	BehaviorReturnCode_t result;
	result = _rotateToEntity.Evaluate( *GetSelf() );
	
	if ( result == BEHAVIOR_SUCCESS )
		{
		_rotateToEntity.End( *GetSelf() );				
		return BEHAVIOR_SUCCESS;
		}
	
	if ( result != BEHAVIOR_EVALUATING )
		{
		failureStateRotate( "Rotate Component Failed" );
		return BEHAVIOR_FAILED;
		}

	return BEHAVIOR_EVALUATING;   
}

void WatchEntityEX::failureStateRotate( const str& failureReason )
{
	Q_UNUSED(failureReason);
}




void WatchEntityEX::setupStateHold()
{
	GetSelf()->SetAnim( _stance , NULL , legs );
}

BehaviorReturnCode_t WatchEntityEX::evaluateStateHold()
{
	Vector selfToEntity = _ent->centroid - GetSelf()->centroid;
	selfToEntity = selfToEntity.toAngles();	

	float yawDiff = selfToEntity[YAW] - GetSelf()->angles[YAW];

	if ( yawDiff > -5.5 && yawDiff < 5.5 )
		return BEHAVIOR_EVALUATING;

	//if Not facing enemy
	return BEHAVIOR_SUCCESS;   
}

void WatchEntityEX::failureStateHold( const str& failureReason )
{
	Q_UNUSED(failureReason);
}





