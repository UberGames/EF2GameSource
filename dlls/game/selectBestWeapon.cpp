//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/selectBestWeapon.cpp              $
// $Revision:: 12                                                             $
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
//	SelectBestWeapon Implementation
// 
// PARAMETERS:
//
// ANIMATIONS:
//
//--------------------------------------------------------------------------------

#include "actor.h"
#include "selectBestWeapon.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, SelectBestWeapon, NULL )
	{
		{ &EV_Behavior_Args,			&SelectBestWeapon::SetArgs	},
		{ &EV_Behavior_AnimDone,		&SelectBestWeapon::AnimDone	}, 		
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		SelectBestWeapon()
// Class:		SelectBestWeapon
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
SelectBestWeapon::SelectBestWeapon()
{
	_self = NULL;
}

//--------------------------------------------------------------
// Name:		SelectBestWeapon()
// Class:		SelectBestWeapon
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
SelectBestWeapon::~SelectBestWeapon()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       SelectBestWeapon
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void SelectBestWeapon::SetArgs( Event *ev )
{
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       SelectBestWeapon
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void SelectBestWeapon::AnimDone( Event *ev )
{
	_animDone = true;
}


//--------------------------------------------------------------
// Name:        Begin()
// Class:       SelectBestWeapon
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void SelectBestWeapon::Begin( Actor &self )
	{       
	init( self );
	}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       SelectBestWeapon
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t SelectBestWeapon::Evaluate( Actor &self )
{
	BehaviorReturnCode_t stateResult;

	think();

	switch ( _state )
	{
	//---------------------------------------------------------------------
	case SBW_SELECT_WEAPON:
	//---------------------------------------------------------------------
		stateResult = evaluateStateSelectWeapon();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( SBW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			{	
			if ( _self->combatSubsystem->UsingWeaponNamed( _bestWeaponName ) )
				transitionToState( SBW_SUCCESS );								
			else
				transitionToState( SBW_PUT_AWAY_CURRENT_WEAPON );

			}

	break;

	//---------------------------------------------------------------------
	case SBW_PUT_AWAY_CURRENT_WEAPON:
	//---------------------------------------------------------------------
		stateResult = evaluateStatePutAwayCurrentWeapon();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( SBW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( SBW_PULL_OUT_NEW_WEAPON );

	break;

	//---------------------------------------------------------------------
	case SBW_PULL_OUT_NEW_WEAPON:
	//---------------------------------------------------------------------
		stateResult = evaluateStatePullOutNewWeapon();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( SBW_FAILED );

		//if ( stateResult == BEHAVIOR_SUCCESS )
		//	transitionToState( SBW_READY_NEW_WEAPON );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( SBW_SUCCESS );

	break;

	//---------------------------------------------------------------------
	case SBW_READY_NEW_WEAPON:
	//---------------------------------------------------------------------
		stateResult = evaluateStateReadyNewWeapon();

		if ( stateResult == BEHAVIOR_FAILED )
			transitionToState( SBW_FAILED );

		if ( stateResult == BEHAVIOR_SUCCESS )
			transitionToState( SBW_SUCCESS );

	break;

	//---------------------------------------------------------------------
	case SBW_SUCCESS:
	//---------------------------------------------------------------------
		return BEHAVIOR_SUCCESS;
	break;

	//---------------------------------------------------------------------
	case SBW_FAILED:
	//---------------------------------------------------------------------
		return BEHAVIOR_FAILED;
	break;
	}
	

	return BEHAVIOR_EVALUATING;   

}



//--------------------------------------------------------------
// Name:        End()
// Class:       SelectBestWeapon
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void SelectBestWeapon::End(Actor &self)
{   
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		SelectBestWeapon
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::transitionToState( selectBestWeaponStates_t state )
{
	
	switch ( state )
	{	
	case SBW_SELECT_WEAPON:
		setupStateSelectWeapon();
		setInternalState( state , "SBW_SELECT_WEAPON" );
	break;		
			
	case SBW_PUT_AWAY_CURRENT_WEAPON:
		setupStatePutAwayCurrentWeapon();
		setInternalState( state , "SBW_PUT_AWAY_CURRENT_WEAPON" );
	break;

	case SBW_PULL_OUT_NEW_WEAPON:
		setupStatePullOutNewWeapon();
		setInternalState( state , "SWB_PULL_OUT_NEW_WEAPON" );
	break;

	case SBW_READY_NEW_WEAPON:
		setupStateReadyNewWeapon();
		setInternalState( state , "SBW_READY_NEW_WEAPON" );
	break;

	case SBW_SUCCESS:
		setInternalState( state , "SBW_SUCCESS" );
	break;

	case SBW_FAILED:
		setInternalState( state , "SBW_FAILED" );
	break;
	}
	
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		SelectBestWeapon
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::setInternalState( selectBestWeaponStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		init()
// Class:		SelectBestWeapon
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::init( Actor &self )
{
	_self = &self;	
	_animDone = false;
	transitionToState( SBW_SELECT_WEAPON );
	_currentWeaponName = self.combatSubsystem->GetActiveWeaponName();
	
}

//--------------------------------------------------------------
// Name:		think()
// Class:		SelectBestWeapon
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::think()
{
	if ( !_bestWeapon )
		{
		failureStateSelectWeapon( "No Best Weapon" );
		transitionToState( SBW_SUCCESS );
		}
}

//--------------------------------------------------------------
// Name:		setupStateSelectWeapon()
// Class:		SelectBestWeapon
//
// Description:	Sets up the state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::setupStateSelectWeapon()
{
	if ( !_currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		_currentEnemy = _self->enemyManager->GetCurrentEnemy();
		}
}

//--------------------------------------------------------------
// Name:		evaluateStateSelectWeapon()
// Class:		SelectBestWeapon
//
// Description:	Evaluates the State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t SelectBestWeapon::evaluateStateSelectWeapon()
{
	str bestWeaponName;
	float powerRating;
	str currentPostureState;


	//First Check if our current weapon is useless
	powerRating = _self->combatSubsystem->GetActiveWeaponPowerRating( _currentEnemy );
	if ( powerRating < .05 )
		{
		_self->InContext( "weaponuseless" , 0 );
		//Put Weapon Useless Context Here
		}


	_bestWeapon = _self->combatSubsystem->GetBestAvailableWeapon( _currentEnemy );

	if ( !_bestWeapon )
		{
		failureStateSelectWeapon( "No Best Weapon" );
		return BEHAVIOR_SUCCESS;
		}

	_bestWeaponName = _bestWeapon->getName();


	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateSelectWeapon()
// Class:		SelectBestWeapon
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::failureStateSelectWeapon(const str& failureReason)
{
}


//--------------------------------------------------------------
// Name:		setupStatePutAwayCurrentWeapon()
// Class:		SelectBestWeapon
//
// Description:	Sets up the state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::setupStatePutAwayCurrentWeapon()
{
	str anim;
	anim = _currentWeaponName + "_putaway";
	_self->SetAnim( anim , EV_Actor_NotifyBehavior, torso );
	_animDone = false;
}

//--------------------------------------------------------------
// Name:		evaluateStatePutAwayCurrentWeapon()
// Class:		SelectBestWeapon
//
// Description:	Evaluates the State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t SelectBestWeapon::evaluateStatePutAwayCurrentWeapon()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStatePutAwayCurrentWeapon()
// Class:		SelectBestWeapon
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::failureStatePutAwayCurrentWeapon(const str& failureReason)
{
}

//--------------------------------------------------------------
// Name:		setupStatePullOutNewWeapon()
// Class:		SelectBestWeapon
//
// Description:	Sets up the state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::setupStatePullOutNewWeapon()
{
	str anim;
	anim = _bestWeaponName + "_draw";
	_self->SetAnim( anim , EV_Actor_NotifyBehavior , torso );
	_animDone = false;
}

//--------------------------------------------------------------
// Name:		evaluateStatePullOutNewWeapon()
// Class:		SelectBestWeapon
//
// Description:	Evaluates the State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t SelectBestWeapon::evaluateStatePullOutNewWeapon()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStatePullOutNewWeapon()
// Class:		SelectBestWeapon
//
// Description:	Failure Handler for State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void SelectBestWeapon::failureStatePullOutNewWeapon(const str& failureReason)
{
}

void SelectBestWeapon::setupStateReadyNewWeapon()
{
	str anim;
	anim = _bestWeaponName + "_idle";
	_self->SetAnim( anim , EV_Actor_NotifyBehavior , torso );
	_animDone = false;
}

BehaviorReturnCode_t SelectBestWeapon::evaluateStateReadyNewWeapon()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

void SelectBestWeapon::failureStateReadyNewWeapon( const str &failureReason )
{
}

bool SelectBestWeapon::CanExecute( Actor &self )
{
	str currentPosture;
	currentPosture = self.postureController->getCurrentPostureName();

	if ( currentPosture == "STAND" )
		return true;

	return false;
}

