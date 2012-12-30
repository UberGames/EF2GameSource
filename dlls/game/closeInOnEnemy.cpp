//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/closeInOnEnemy.cpp              $
// $Revision:: 7                                                              $
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
//	CloseInOnEnemy Implementation
// 
// PARAMETERS:
//		
//
// ANIMATIONS:
//		
//--------------------------------------------------------------------------------

#include "actor.h"
#include "closeInOnEnemy.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, CloseInOnEnemy, NULL )
	{
		{ &EV_Behavior_Args,			&CloseInOnEnemy::SetArgs },      
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		CloseInOnEnemy()
// Class:		CloseInOnEnemy
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CloseInOnEnemy::CloseInOnEnemy()
{
	_anim = "";
	_torsoAnim = "";
	_dist = 64.0f;
}

//--------------------------------------------------------------
// Name:		~CloseInOnEnemy()
// Class:		CloseInOnEnemy
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CloseInOnEnemy::~CloseInOnEnemy()
{
}


//--------------------------------------------------------------
//
// Name:        SetArgs()
// Class:       CloseInOnEnemy
//
// Description: 
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
//--------------------------------------------------------------
void CloseInOnEnemy::SetArgs( Event *ev )
{
	_anim = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		_dist = ev->GetFloat( 2 );
 
	if ( ev->NumArgs() > 2 )
		_torsoAnim = ev->GetString( 3 );
}



//--------------------------------------------------------------
//
// Name:        Begin()
// Class:       CloseInOnEnemy
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void CloseInOnEnemy::Begin( Actor &self )
{          
	init( self );
}



//--------------------------------------------------------------
//
// Name:        Evaluate()
// Class:       CloseInOnEnemy
//
// Description: Update for this behavior -- called every server frame
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     BehaviorReturnCode_t
//
//--------------------------------------------------------------
BehaviorReturnCode_t	CloseInOnEnemy::Evaluate( Actor &self )
{

	BehaviorReturnCode_t stateResult;


	think();

	switch ( _state )
		{
		//---------------------------------------------------------------------
		case CLOSE_IN_ON_ENEMY_APPROACH:
		//---------------------------------------------------------------------
			stateResult = evaluateStateApproach();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( CLOSE_IN_ON_ENEMY_FAILED );

			if ( stateResult == BEHAVIOR_FAILED_STEERING_NO_PATH )
				transitionToState( CLOSE_IN_ON_ENEMY_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( CLOSE_IN_ON_ENEMY_SUCCESS );
		break;

		//---------------------------------------------------------------------
		case CLOSE_IN_ON_ENEMY_SUCCESS:
		//---------------------------------------------------------------------
			return BEHAVIOR_SUCCESS;
		break;

		//---------------------------------------------------------------------
		case CLOSE_IN_ON_ENEMY_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;
		break;
		}
	
	return BEHAVIOR_EVALUATING;
}



//--------------------------------------------------------------
//
// Name:        End()
// Class:       CloseInOnEnemy
//
// Description: Ends this behavior -- cleans things up
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void CloseInOnEnemy::End(Actor &self)
{   
	_chaseEnemy.End( self );
	self.SetAnim( "idle" );
}


//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		CloseInOnEnemy
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemy::transitionToState( closeInOnEnemyStates_t state )
{
	switch( state )
		{
		case CLOSE_IN_ON_ENEMY_APPROACH:
			setupStateApproach();
			setInternalState( state , "CLOSE_IN_ON_ENEMY_APPROACH" );
		break;

		case CLOSE_IN_ON_ENEMY_SUCCESS:
			setInternalState( state , "CLOSE_IN_ON_ENEMY_SUCCESS" );
		break;

		case CLOSE_IN_ON_ENEMY_FAILED:
			setInternalState( state , "CLOSE_IN_ON_ENEMY_FAILED" );
		break;
		}
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		CloseInOnEnemy
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemy::setInternalState( closeInOnEnemyStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );	
}

//--------------------------------------------------------------
// Name:		init()
// Class:		CloseInOnEnemy
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemy::init( Actor &self )
{
	_self = &self;
	updateEnemy();	
	transitionToState(CLOSE_IN_ON_ENEMY_APPROACH);
}

//--------------------------------------------------------------
// Name:		think()
// Class:		CloseInOnEnemy
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemy::think()
{	
	if ( !_currentEnemy )
		{
		SetFailureReason( "CloseInOnEnemy::updateEnemy -- No Enemy" );
		transitionToState( CLOSE_IN_ON_ENEMY_FAILED );
		}
}


//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		CloseInOnEnemy
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemy::updateEnemy()
{
	Entity *currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !currentEnemy )
			{
			SetFailureReason( "CloseInOnEnemy::updateEnemy -- No Enemy" );
			transitionToState( CLOSE_IN_ON_ENEMY_FAILED );
			}
			
		}

	_currentEnemy = currentEnemy;	
}

//--------------------------------------------------------------
// Name:		setTorsoAnim()
// Class:		CloseInOnEnemy
//
// Description:	Sets our Torso Animation
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemy::setTorsoAnim()
{
	_self->SetAnim( _torsoAnim , NULL , torso );
}

//--------------------------------------------------------------
// Name:		setupStateApproach()
// Class:		CloseInOnEnemy
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemy::setupStateApproach()
{
   _chaseEnemy.SetAnim( _anim );
   _chaseEnemy.SetDistance( _dist );   
   _chaseEnemy.SetEntity( *_self, _currentEnemy );

   if ( _torsoAnim.length() > 0 )
	   setTorsoAnim();

   _chaseEnemy.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateApproach()
// Class:		CloseInOnEnemy
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t CloseInOnEnemy::evaluateStateApproach()
{
	return _chaseEnemy.Evaluate( *_self );
}

//--------------------------------------------------------------
// Name:		failureStateApproach()
// Class:		CloseInOnEnemy
//
// Description:	Failure Handler for State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemy::failureStateApproach( const str& failureReason )
{
}
