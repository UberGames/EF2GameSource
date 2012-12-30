//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/holdPosition.cpp                         $
// $Revision:: 7                                                              $
//   $Author:: Singlis                                                          $
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
// PARAMETERS:
//
// ANIMATIONS:
//
//--------------------------------------------------------------------------------

#include "actor.h"
#include "holdPosition.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, HoldPosition, NULL )
	{
		{ &EV_Behavior_Args,		&HoldPosition::SetArgs	},
		{ &EV_Behavior_AnimDone,	&HoldPosition::AnimDone	}, 
		{ NULL, NULL }
	};



//--------------------------------------------------------------
// Name:		RotateToEntity()
// Class:		RotateToEntity
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
HoldPosition::HoldPosition()
{
	_legAnim = "idle";
	_twitchAnim = "twitch";
	_weaponTwitchAnim = "";
	_torsoAnim = "";
	_holdTimeMin = 1.0f;
	_holdTimeMax = 1.5f;;
	_nextTwitchTime = 0.0f;
	_twitchInterval = 2.5f;
	_canTwitch = true;
	_animDone = false;
}


//--------------------------------------------------------------
// Name:		~RotateToEntity()
// Class:		RotateToEntity
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
HoldPosition::~HoldPosition()
{
}

//--------------------------------------------------------------
//
// Name:        SetArgs()
// Class:       RotateToEntity
//
// Description: Sets Variables based on arguments inside the event
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
//--------------------------------------------------------------
void HoldPosition::SetArgs( Event *ev )
{
}

void HoldPosition::AnimDone( Event *ev )
{
	_animDone = true;
}

//--------------------------------------------------------------
//
// Name:        Begin()
// Class:       RotateToEntity
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void HoldPosition::Begin( Actor &self )
{
	init(self);
}


//--------------------------------------------------------------
//
// Name:        Evaluate()
// Class:       RotateToEntity
//
// Description: Update for this behavior -- called every server frame
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     BehaviorReturnCode_t
//
//--------------------------------------------------------------
BehaviorReturnCode_t HoldPosition::Evaluate( Actor &self )
{
	BehaviorReturnCode_t stateResult;

	switch ( _state )
		{
		//---------------------------------------------------------------------
		case HOLD_POSITION_HOLD:
		//---------------------------------------------------------------------
			stateResult = evaluateStateHold();
			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				if ( level.time >= _nextTwitchTime && _canTwitch )
					{
					if ( _self->combatSubsystem->HaveWeapon() )
						{
						transitionToState(HOLD_POSITION_WEAPON_TWITCH);
						return BEHAVIOR_EVALUATING;
						}
					else
						{
						transitionToState(HOLD_POSITION_TWITCH);
						return BEHAVIOR_EVALUATING;
						}
					}

				transitionToState( HOLD_POSITION_HOLD );
				}

		break;

		//---------------------------------------------------------------------
		case HOLD_POSITION_TWITCH:
		//---------------------------------------------------------------------
			stateResult = evaluateStateTwitch();

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( HOLD_POSITION_HOLD );			
		break;

		//---------------------------------------------------------------------
		case HOLD_POSITION_WEAPON_TWITCH:
		//---------------------------------------------------------------------
			stateResult = evaluateStateWeaponTwitch();

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( HOLD_POSITION_HOLD );			

		break;

		//---------------------------------------------------------------------
		case HOLD_POSITION_SUCCESS:
		//---------------------------------------------------------------------
			return BEHAVIOR_SUCCESS;
		break;

		//---------------------------------------------------------------------
		case HOLD_POSITION_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;
		break;
		}

	return BEHAVIOR_EVALUATING;
}



//--------------------------------------------------------------
//
// Name:        End()
// Class:       RotateToEntity
//
// Description: Ends this behavior -- cleans things up
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void HoldPosition::End(Actor &self)
{
	_self->SetIgnoreWatchTarget( false );
}

void HoldPosition::transitionToState( HoldPositionStates_t state )
{
	switch ( state )
		{	
		case HOLD_POSITION_HOLD:
			setupStateHold();
			setInternalState( state , "HOLD_POSITION_HOLD" );
		break;

		case HOLD_POSITION_TWITCH:
			setupStateTwitch();
			setInternalState( state , "HOLD_POSITION_TWITCH" );
		break;

		case HOLD_POSITION_WEAPON_TWITCH:
			setupStateWeaponTwitch();
			setInternalState( state , "HOLD_POSITION_WEAPON_TWITCH" );
		break;

		case HOLD_POSITION_SUCCESS:			
			setInternalState( state , "HOLD_POSITION_SUCCESS" );
		break;

		case HOLD_POSITION_FAILED:
			setInternalState( state , "HOLD_POSITION_FAILED" );
		break;
		}
}

void HoldPosition::setInternalState( HoldPositionStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

void HoldPosition::init( Actor &self )
{
	_self = &self;

	transitionToState(HOLD_POSITION_HOLD);
}

void HoldPosition::think()
{
}

void HoldPosition::setupStateHold()
{
	_endHoldTime = level.time + G_Random ( _holdTimeMax + _holdTimeMin );
	if ( !_self->combatSubsystem->HaveWeapon() )
	{
		_self->SetAnim( _legAnim , EV_Actor_NotifyTorsoBehavior , legs );
		return;
	}

	if ( _self->enemyManager->HasEnemy() )
		_torsoAnim = _self->combatSubsystem->GetAnimForMyWeapon( "CombatGunIdle" );
	else
		_torsoAnim = _self->combatSubsystem->GetAnimForMyWeapon( "IdleGunIdle" );

	if ( _torsoAnim.length() )
		{
		_self->SetAnim( _torsoAnim, NULL , torso );
		}
	else
		{
		_self->SetAnim( _legAnim , EV_Actor_NotifyTorsoBehavior , legs );
		}

	

}

BehaviorReturnCode_t HoldPosition::evaluateStateHold()
{
	if ( level.time >= _endHoldTime )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

void HoldPosition::failureStateHold( const str& failureReason )
{
}

void HoldPosition::	setupStateTwitch()
{
	_self->SetIgnoreWatchTarget( true );
	_animDone = false;
	_self->SetAnim( _legAnim , EV_Actor_NotifyTorsoBehavior , legs );
}

BehaviorReturnCode_t HoldPosition::evaluateStateTwitch()
{
	if ( _animDone )
		{
		_nextTwitchTime = level.time + _twitchInterval;
		_self->SetIgnoreWatchTarget( false );		
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING;
}

void HoldPosition::	failureStateTwitch( const str& failureReason )
{
}


void HoldPosition::	setupStateWeaponTwitch()
{
	_animDone = false;
	if ( _self->enemyManager->HasEnemy() )
		_torsoAnim = _self->combatSubsystem->GetAnimForMyWeapon( "CombatGunTwitch" );
	else
		_torsoAnim = _self->combatSubsystem->GetAnimForMyWeapon( "IdleGunTwitch" );

	if ( _torsoAnim.length() )
		{
		_self->SetIgnoreWatchTarget( true );
		//_self->SetAnim( _torsoAnim, NULL , torso );
		_self->ClearLegAnim();
		_self->ClearTorsoAnim();
		_self->SetAnim( _torsoAnim, EV_Actor_NotifyBehavior , legs );
		}
	else
		_canTwitch = false;

}

BehaviorReturnCode_t HoldPosition::evaluateStateWeaponTwitch()
{
	if ( _animDone )
		{
		_nextTwitchTime = level.time + _twitchInterval;
		_self->SetIgnoreWatchTarget( false );
		return BEHAVIOR_SUCCESS;
		}

	return BEHAVIOR_EVALUATING;
}

void HoldPosition::	failureStateWeaponTwitch( const str& failureReason )
{
}
