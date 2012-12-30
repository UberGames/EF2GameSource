//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/closeInOnEnemyWhileFiringWeapon.cpp              $
// $Revision:: 5                                                              $
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
//	CloseInOnEnemyWhileFiringWeapon Implementation
// 
// PARAMETERS:
//		
//
// ANIMATIONS:
//		
//--------------------------------------------------------------------------------

#include "actor.h"
#include "closeInOnEnemyWhileFiringWeapon.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, CloseInOnEnemyWhileFiringWeapon, NULL )
	{
		{ &EV_Behavior_Args,			&CloseInOnEnemyWhileFiringWeapon::SetArgs },      
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		CloseInOnEnemyWhileFiringWeapon()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CloseInOnEnemyWhileFiringWeapon::CloseInOnEnemyWhileFiringWeapon()
{
	_approachAnim	= "";  
	_aimAnim		= "";
	_fireAnim		= "";
	_fireTimeMin	= 0.0f;
	_fireTimeMax	= 0.0f;
	_pauseTimeMin	= 0.0f;
	_pauseTimeMax	= 0.0f;
	_dist			= 0.0f;
	_nextFireTime	= 0.0f;
	_nextPauseTime	= 0.0f;
}

//--------------------------------------------------------------
// Name:		~CloseInOnEnemyWhileFiringWeapon()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CloseInOnEnemyWhileFiringWeapon::~CloseInOnEnemyWhileFiringWeapon()
{
}


//--------------------------------------------------------------
//
// Name:        SetArgs()
// Class:       CloseInOnEnemyWhileFiringWeapon
//
// Description: 
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::SetArgs( Event *ev )
{
	_approachAnim	= ev->GetString( 1 );  
	_aimAnim		= ev->GetString( 2 );
	_fireAnim		= ev->GetString( 3 );
	_fireTimeMin	= ev->GetFloat( 4 );
	_fireTimeMax	= ev->GetFloat( 5 );
	_pauseTimeMin	= ev->GetFloat( 6 );
	_pauseTimeMax	= ev->GetFloat( 7 );
	_dist			= ev->GetFloat( 8 );
}



//--------------------------------------------------------------
//
// Name:        Begin()
// Class:       CloseInOnEnemyWhileFiringWeapon
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::Begin( Actor &self )
{          
	init( self );
}



//--------------------------------------------------------------
//
// Name:        Evaluate()
// Class:       CloseInOnEnemyWhileFiringWeapon
//
// Description: Update for this behavior -- called every server frame
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     BehaviorReturnCode_t
//
//--------------------------------------------------------------
BehaviorReturnCode_t CloseInOnEnemyWhileFiringWeapon::Evaluate( Actor &self )
{

	BehaviorReturnCode_t stateResult;

	think();

	switch ( _state )
		{
		//---------------------------------------------------------------------
		case CIWF_APPROACH_SETUP_APPROACH:
		//---------------------------------------------------------------------
			stateResult = evaluateStateSetupApproach();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( CIWF_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( CIWF_APPROACH_FIRE );
		break;

		//---------------------------------------------------------------------
		case CIWF_APPROACH_FIRE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateApproachFire();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( CIWF_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( CIWF_APPROACH_FIRE_PAUSE );
		break;		

		//---------------------------------------------------------------------
		case CIWF_APPROACH_FIRE_PAUSE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateApproachFirePause();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( CIWF_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( CIWF_APPROACH_FIRE );
		break;	

		//---------------------------------------------------------------------
		case CIWF_SUCCESS:
		//---------------------------------------------------------------------
			return BEHAVIOR_SUCCESS;
		break;

		//---------------------------------------------------------------------
		case CIWF_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;
		break;
		}
	
	return BEHAVIOR_EVALUATING;
}



//--------------------------------------------------------------
//
// Name:        End()
// Class:       CloseInOnEnemyWhileFiringWeapon
//
// Description: Ends this behavior -- cleans things up
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::End(Actor &self)
{   
	_chaseEnemy.End( *_self );
	_fireWeapon.End( *_self );
}


//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::transitionToState( CIWFStates_t state )
{
	switch( state )
		{
		case CIWF_APPROACH_SETUP_APPROACH:
			setupStateSetupApproach();
			setInternalState( state , "CIWF_APPROACH_SETUP_APPROACH" );
		break;

		case CIWF_APPROACH_FIRE:
			setupStateApproachFire();
			setInternalState( state , "CIWF_APPROACH_FIRE" );
		break;

		case CIWF_APPROACH_FIRE_PAUSE:
			setupStateApproachFirePause();
			setInternalState( state , "CIWF_APPROACH_FIRE_PAUSE" );
		break;

		case CIWF_SUCCESS:
			setInternalState( state , "CIWF_SUCCESS" );
		break;

		case CIWF_FAILED:
			setInternalState( state , "CIWF_FAILED" );
		break;
		}
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::setInternalState( CIWFStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );	
}

//--------------------------------------------------------------
// Name:		init()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::init( Actor &self )
{
	_self = &self;
	updateEnemy();	
	transitionToState(CIWF_APPROACH_SETUP_APPROACH);
}

//--------------------------------------------------------------
// Name:		think()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::think()
{	
}


//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::updateEnemy()
{
	Entity *currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !currentEnemy )
			{
			SetFailureReason( "CloseInOnEnemyWhileFiringWeapon::updateEnemy -- No Enemy" );
			transitionToState( CIWF_FAILED );
			}
			
		}

	_currentEnemy = currentEnemy;	
}

//--------------------------------------------------------------
// Name:		setTorsoAnim()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Sets our Torso Animation
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::setTorsoAnim()
{
	_self->SetAnim( _aimAnim , NULL , torso );
}

//--------------------------------------------------------------
// Name:		setupStateSetupApproach()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::setupStateSetupApproach()
{
   _chaseEnemy.SetAnim( _approachAnim );
   _chaseEnemy.SetDistance( _dist );   
   _chaseEnemy.SetEntity( *_self, _currentEnemy );

   setTorsoAnim();
   _chaseEnemy.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateApproach()
// Class:		evaluateStateSetupApproach
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t CloseInOnEnemyWhileFiringWeapon::evaluateStateSetupApproach()
{
	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateSetupApproach()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Failure Handler for State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::failureStateSetupApproach( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateApproachFire()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Sets up Stand Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::setupStateApproachFire()
{
	_nextFireTime = level.time + G_Random( _fireTimeMax ) + _fireTimeMin;
	_fireWeapon.SetAnim( _fireAnim ); 
	_fireWeapon.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateApproachFire()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Evaluates Stand Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CloseInOnEnemyWhileFiringWeapon::evaluateStateApproachFire()
{
	BehaviorReturnCode_t result;

	//Evaluate our chase first
	result = _chaseEnemy.Evaluate( *_self );

	if ( result != BEHAVIOR_SUCCESS &&
		 result != BEHAVIOR_EVALUATING
	   )
		{
		return BEHAVIOR_FAILED;
		}

	_fireWeapon.Evaluate( *_self );
	_self->combatSubsystem->AimWeaponTag(_currentEnemy);	
	
	if ( level.time > _nextFireTime )
		return BEHAVIOR_SUCCESS;	

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateStandFiring()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Failure Handler for Stand Firing State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::failureStateApproachFire( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateApproachFirePause()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Sets up Stand Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::setupStateApproachFirePause()
{
	_nextPauseTime = level.time + G_Random( _pauseTimeMax ) + _pauseTimeMin;
	_fireWeapon.End( *_self );
	setTorsoAnim();
}

//--------------------------------------------------------------
// Name:		evaluateStateApproachFirePause()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Evaluates Stand Firing State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t CloseInOnEnemyWhileFiringWeapon::evaluateStateApproachFirePause()
{
	BehaviorReturnCode_t result;

	//Evaluate our chase first
	result = _chaseEnemy.Evaluate( *_self );

	if ( result != BEHAVIOR_SUCCESS &&
		 result != BEHAVIOR_EVALUATING
	   )
		{
		return BEHAVIOR_FAILED;
		}

	
	if ( level.time > _nextPauseTime )
		return BEHAVIOR_SUCCESS;	

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateApproachFirePause()
// Class:		CloseInOnEnemyWhileFiringWeapon
//
// Description:	Failure Handler for Stand Firing State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnEnemyWhileFiringWeapon::failureStateApproachFirePause( const str& failureReason )
{
}
