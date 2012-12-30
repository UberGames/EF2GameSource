//-----------------------------------------------------------------------------
// 
//  $Logfile:: /EF2/Code/DLLs/game/rangedCombatWithWeapon.cpp                 $
// $Revision:: 16                                                             $
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
//
// ANIMATIONS:
//--------------------------------------------------------------------------------

#include "actor.h"
#include "rangedCombatWithWeapon.hpp"

extern Event EV_Actor_UseWeapon;
extern Event EV_PostureChanged_Completed;

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, RangedCombatWithWeapon, NULL )
	{
		{ &EV_Behavior_Args,			&RangedCombatWithWeapon::SetArgs		},
		{ &EV_Behavior_AnimDone,		&RangedCombatWithWeapon::AnimDone		},
		{ &EV_PostureChanged_Completed, &RangedCombatWithWeapon::PostureDone	},
		{ NULL,							NULL		}
	};


//--------------------------------------------------------------
// Name:		RangedCombatWithWeapon()
// Class:		RangedCombatWithWeapon
//
// Description: Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
RangedCombatWithWeapon::RangedCombatWithWeapon()
{
	_approachDist			= 384.00f;
	_retreatDist			= 256.00f;
	_strafeChance			=   0.85f;
	_postureChangeChance	=   0.15f;
}

//--------------------------------------------------------------
// Name:		~RangedCombatWithWeapon()
// Class:		RangedCombatWithWeapon
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
RangedCombatWithWeapon::~RangedCombatWithWeapon()
{
}

//--------------------------------------------------------------
// Name:         SetArgs()
// Class:        RangedCombatWithWeapon
//
// Description:  Sets the arguments of the behavior 
//
// Parameters:   Event *ev
//
// Returns:      None
//--------------------------------------------------------------
void RangedCombatWithWeapon::SetArgs ( Event *ev)
{
	int argCount = ev->NumArgs();

	_aimAnim = ev->GetString( 1 );
	_fireAnim = ev->GetString( 2 );
	
	if ( argCount > 2 )
		_approachDist = ev->GetFloat( 3 );

	if ( argCount > 3 )
		_retreatDist = ev->GetFloat( 4 );

	if ( argCount > 4 )
		_strafeChance = ev->GetFloat( 5 );

	if ( argCount > 5 )
		_postureChangeChance = ev->GetFloat( 6 );

}


//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       RangedCombatWithWeapon
//
// Description: Handler for Animation Done
//
// Parameters:  Event *ev
//
// Returns:     None
//--------------------------------------------------------------
void RangedCombatWithWeapon::AnimDone( Event *ev )
{
	switch ( _state )
		{
		case RCWW_CORRIDOR_COMBAT:
			_corridorCombat.AnimDone( ev );
		break;

		case RCWW_GENERAL_COMBAT:
			_generalCombat.AnimDone( ev );
		break;

		case RCWW_COVER_COMBAT:
			_coverCombat.AnimDone( ev );
		break;

		case RCWW_CHANGE_WEAPON:			
			_selectBestWeapon.AnimDone( ev );			
		break;
		}
}

void RangedCombatWithWeapon::PostureDone( Event *ev )
{
	switch ( _state )
		{
		case RCWW_CORRIDOR_COMBAT:
			_corridorCombat.PostureDone( ev );
		break;

		case RCWW_GENERAL_COMBAT:
			_generalCombat.PostureDone( ev );
		break;
		}
}
//--------------------------------------------------------------
// Name:        Begin()
// Class:       RangedCombatWithWeapon
//
// Description: Begins the Behavior
//
// Parameters:  Actor &self
//
// Returns:     None
//--------------------------------------------------------------
void RangedCombatWithWeapon::Begin( Actor &self )
{
	init( self );
	updateEnemy();
	transitionToState( RCWW_CHANGE_WEAPON );	
}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       RangedCombatWithWeapon
//
// Description: Returns True Or False, and is run every frame
//              that the behavior
//
// Parameters:  Actor &self
//
// Returns:     True or False
//--------------------------------------------------------------
BehaviorReturnCode_t RangedCombatWithWeapon::Evaluate ( Actor &self )
{	
	BehaviorReturnCode_t stateResult;
	think();
	
	switch ( _state )
	{		
	//---------------------------------------------------------------------		
	case RCWW_EVALUATE_OPTIONS:
	//---------------------------------------------------------------------		
		//First Check if we need to change weapons
		if ( checkShouldCheckWeapon() )
			{
			transitionToState( RCWW_CHANGE_WEAPON );
			return BEHAVIOR_EVALUATING;
			}

		//Check if we should do corridor combat
		if ( checkShouldDoCorridorCombat() )
			{
			transitionToState( RCWW_CORRIDOR_COMBAT );
			return BEHAVIOR_EVALUATING;
			}

		if ( checkShouldDoCoverCombat() )
			{
			transitionToState( RCWW_COVER_COMBAT );
			return BEHAVIOR_EVALUATING;
			}

		//None of the above, so we need to do general combat
		_recheckTime = level.time + G_Random() + 3.0f;
		transitionToState( RCWW_GENERAL_COMBAT );

	break;

	//---------------------------------------------------------------------		
	case RCWW_CHANGE_WEAPON:
	//---------------------------------------------------------------------
		stateResult = evaluateStateChangeWeapon();		
		
		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( RCWW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( RCWW_EVALUATE_OPTIONS );

	break;

	//---------------------------------------------------------------------
	case RCWW_GENERAL_COMBAT:
	//---------------------------------------------------------------------
		//Check if we need to change weapons
		if ( checkShouldCheckWeapon() )
			{
			transitionToState( RCWW_CHANGE_WEAPON );
			return BEHAVIOR_EVALUATING;
			}

		if ( level.time >= _recheckTime )
			{
			transitionToState( RCWW_EVALUATE_OPTIONS );
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateStateGeneralCombat();	

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( RCWW_FAILED );
	break;

	//---------------------------------------------------------------------
	case RCWW_CORRIDOR_COMBAT:
	//---------------------------------------------------------------------
		//First Check if we need to change weapons
		if ( checkShouldCheckWeapon() )
			{
			transitionToState( RCWW_CHANGE_WEAPON );
			return BEHAVIOR_EVALUATING;
			}

		stateResult = evaluateStateCorridorCombat();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( RCWW_GENERAL_COMBAT );
	break;

	//---------------------------------------------------------------------
	case RCWW_COVER_COMBAT:
	//---------------------------------------------------------------------
		//First Check if we need to changing weapons during cover combat
		//is a pain.  I'm taking it out for now.


		stateResult = evaluateStateCoverCombat();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( RCWW_GENERAL_COMBAT );
	break;

	//---------------------------------------------------------------------
	case RCWW_FAILED:
	//---------------------------------------------------------------------
		return BEHAVIOR_FAILED;
	break;
	}

	return BEHAVIOR_EVALUATING;	
}


//--------------------------------------------------------------
// Name:         End()
// Class:        RangedCombatWithWeapon
//
// Description:  Ends the Behavior
//
// Parameters:   Actor &self
//
// Returns:      None
//--------------------------------------------------------------
void RangedCombatWithWeapon::End ( Actor &self	)
{
	_generalCombat.End( self );
	_corridorCombat.End( self );
	_coverCombat.End( self );
	self.postureController->setPostureState( "STAND" , "STAND" );	
}



//--------------------------------------------------------------
// Name:		init()
// Class:		RangedCombatWithWeapon
//
// Description: Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::init(Actor &self)
{
	_self = &self;
	_nextSelectWeaponTime = 0.0f;
	_recheckTime = -1.0f;
}

//--------------------------------------------------------------
// Name:		think()
// Class:		RangedCombatWithWeapon
//
// Description:	Think Function Called Every Frame
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::think()
{
}

//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		RangedCombatWithWeapon
//
// Description:	Updates our Current Enemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::updateEnemy()
{
	_currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !_currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		_currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !_currentEnemy )
			{
			SetFailureReason( "RangedCombatWithWeapon::updateEnemy -- No Enemy" );
			transitionToState( RCWW_FAILED );
			}
			
		}
}

//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		RangedCombatWithWeapon
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::transitionToState( RangedCombatWithWeapon_t state )
{
	switch ( state )
	{	
	case RCWW_EVALUATE_OPTIONS:
		setupStateEvaluateOptions();
		setInternalState( state , "RCWW_EVALUATE_OPTIONS" );
	break;

	case RCWW_CHANGE_WEAPON:
		setupStateChangeWeapon();
		setInternalState( state , "RCWW_CHANGE_WEAPON" );
	break;

	case RCWW_GENERAL_COMBAT:
		setupStateGeneralCombat();
		setInternalState( state , "RCWW_GENERAL_COMBAT" );
	break;
	
	case RCWW_CORRIDOR_COMBAT:
		setupStateCorridorCombat();
		setInternalState( state , "RCWW_CORRIDOR_COMBAT" );
	break;

	case RCWW_COVER_COMBAT:
		setupStateCoverCombat();
		setInternalState( state , "RCWW_COVER_COMBAT" );
	break;

	case RCWW_FAILED:
		setInternalState( state , "RCWW_FAILED" );
	break;
	}
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		RangedCombatWithWeapon
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::setInternalState( RangedCombatWithWeapon_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		setupStateChangeWeapon()
// Class:		RangedCombatWithWeapon
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::setupStateChangeWeapon()
{
	_nextSelectWeaponTime = level.time + G_Random() + 5.0f;
	_selectBestWeapon.SetCurrentEnemy( _currentEnemy );
	_selectBestWeapon.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateChangeWeapon()
// Class:		RangedCombatWithWeapon
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t RangedCombatWithWeapon::evaluateStateChangeWeapon()
{
	return _selectBestWeapon.Evaluate( *_self );
}

//--------------------------------------------------------------
// Name:		failureStateChangeWeapon()
// Class:		RangedCombatWithWeapon
//
// Description:	Failure Handler for State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::failureStateChangeWeapon()
{
}

void RangedCombatWithWeapon::setupStateEvaluateOptions()
{
}

BehaviorReturnCode_t RangedCombatWithWeapon::evaluateStateEvaluateOptions()
{
	return BEHAVIOR_EVALUATING;
}

void RangedCombatWithWeapon::failureStateEvaluateOptions()
{
}

//--------------------------------------------------------------
// Name:		setupStateGeneralCombat()
// Class:		RangedCombatWithWeapon
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::setupStateGeneralCombat()
{
	_generalCombat.SetTorsoAnim( _aimAnim );	
	_generalCombat.SetFireAnim( _fireAnim );
	_generalCombat.SetApproachDist( _approachDist );
	_generalCombat.SetRetreatDist ( _retreatDist );
	_generalCombat.SetStrafeChance ( _strafeChance );
	_generalCombat.SetPostureChangeChance ( _postureChangeChance );
	_generalCombat.SetFireTimeMin( 2.0f );
	_generalCombat.SetFireTimeMax( 2.5f );
	_generalCombat.SetPauseTimeMin( 0.5f );
	_generalCombat.SetPauseTimeMax( 0.75f );
	
	_generalCombat.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateGeneralCombat()
// Class:		RangedCombatWithWeapon
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t RangedCombatWithWeapon::evaluateStateGeneralCombat()
{
	return _generalCombat.Evaluate( *_self );

}

//--------------------------------------------------------------
// Name:		failureStateGeneralCombat()
// Class:		RangedCombatWithWeapon
//
// Description:	Failure Handler for State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::failureStateGeneralCombat()
{
}


//--------------------------------------------------------------
// Name:		setupStateCorridorCombat()
// Class:		RangedCombatWithWeapon
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::setupStateCorridorCombat()
{
	_corridorCombat.SetMovementAnim( "walk_rifle" );
	_corridorCombat.SetTorsoAnim( _aimAnim );
	_corridorCombat.SetFireAnim( _fireAnim );
	_corridorCombat.SetPreFireAnim( "" );
	_corridorCombat.SetPostFireAnim( "" );
	_corridorCombat.SetPostureChangeChance( .65 );
	_corridorCombat.SetMaxDistance( 128.0f );
	_corridorCombat.SetRetreatDistance( 96.0f );
	_corridorCombat.SetThreatDistance( 160.0f );
	_corridorCombat.SetFireTimeMin( 2.0f );
	_corridorCombat.SetFireTimeMax( 2.5f );
	_corridorCombat.SetPauseTimeMin( 0.5f );
	_corridorCombat.SetPauseTimeMax( 0.75f );
	
	_corridorCombat.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateCorridorCombat()
// Class:		RangedCombatWithWeapon
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t RangedCombatWithWeapon::evaluateStateCorridorCombat()
{
	return _corridorCombat.Evaluate( *_self );

}

//--------------------------------------------------------------
// Name:		failureStateCorridorCombat()
// Class:		RangedCombatWithWeapon
//
// Description:	Failure Handler for State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::failureStateCorridorCombat()
{
}

//--------------------------------------------------------------
// Name:		setupStateCoverCombat()
// Class:		RangedCombatWithWeapon
//
// Description:	Sets up state 
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void RangedCombatWithWeapon::setupStateCoverCombat()
{
	_coverCombat.SetMovementAnim( "run" );
	_coverCombat.SetTorsoAnim( _aimAnim );
	_coverCombat.SetFireAnim( _fireAnim );
	_coverCombat.SetMaxDistance( 1024.0f );
	_coverCombat.SetFireTimeMin( 2.0f );
	_coverCombat.SetFireTimeMax( 2.5f );
	_coverCombat.SetPauseTimeMin( 0.5f );
	_coverCombat.SetPauseTimeMax( 0.75f );
	_coverCombat.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateCoverCombat()
// Class:		RangedComb
//
// Description:
//
// Parameters:
//
// Returns:
//--------------------------------------------------------------
BehaviorReturnCode_t RangedCombatWithWeapon::evaluateStateCoverCombat()
{
	return _coverCombat.Evaluate( *_self );
}

void RangedCombatWithWeapon::failureStateCoverCombat()
{
}

//--------------------------------------------------------------
// Name:		checkShouldDoCorridorCombat()
// Class:		RangedCombatWithWeapon
//
// Description:	Checks if the actor should do corridor combat
//
// Parameters:	None
//
// Returns:		true or false;
//--------------------------------------------------------------
bool RangedCombatWithWeapon::checkShouldDoCorridorCombat()
{	
	if ( CorridorCombatWithRangedWeapon::CanExecute( 1024.0f , *_self ) )
		return true;

	return false;
}

//--------------------------------------------------------------
// Name:		checkShouldDoGeneralCombat()
// Class:		RangedCombatWithWeapon
//
// Description:	Checks if the actor should do general combat
//
// Parameters:	None
//
// Returns:		true or false
//--------------------------------------------------------------
bool RangedCombatWithWeapon::checkShouldDoGeneralCombat()
{
	// We're pretty much ALWAYS able to do general combat... 
	// That's what its for
	return true;
}

//--------------------------------------------------------------
// Name:		checkShouldCheckWeapon()
// Class:		RangedCombatWithWeapon
//
// Description:	Checks if the actor should do SelectBestWeapon
//
// Parameters:	None
//
// Returns:		True or False
//--------------------------------------------------------------
bool RangedCombatWithWeapon::checkShouldCheckWeapon()
{
	if ( (level.time > _nextSelectWeaponTime) && SelectBestWeapon::CanExecute( *_self ) )
		{
		if ( _self->checkHaveBestWeapon() )
			{
			_nextSelectWeaponTime = level.time + G_Random() + 5.0f;
			return false;
			}

		return true;
		}	

	return false;
}

bool RangedCombatWithWeapon::checkShouldDoCoverCombat()
{
	return CoverCombatWithRangedWeapon::CanExecute( *_self , 1024.0f );
		
}

