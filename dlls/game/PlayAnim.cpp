//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/PlayAnim.cpp                   $
// $Revision:: 7                                                              $
//   $Author:: Singlis                                                       $
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
//	PlayAnim Implementation
// 
// PARAMETERS:
//		
//
// ANIMATIONS:
//		
//--------------------------------------------------------------------------------

#include "actor.h"
#include "PlayAnim.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, PlayAnim, NULL )
	{
		{ &EV_Behavior_Args,			&PlayAnim::SetArgs		},   		
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		PlayAnim()
// Class:		PlayAnim
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
PlayAnim::PlayAnim()
{
	_legAnim = "idle";
	_torsoAnim = "";
	_minTime = 0.0f;
	_maxTime = 0.0f;
	_endTime = 0.0f;
}

//--------------------------------------------------------------
// Name:		~PlayAnim()
// Class:		PlayAnim
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
PlayAnim::~PlayAnim()
{
}


//--------------------------------------------------------------
//
// Name:        SetArgs()
// Class:       PlayAnim
//
// Description: 
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
//--------------------------------------------------------------
void PlayAnim::SetArgs( Event *ev )
{
	_legAnim = ev->GetString( 1 );
	if ( ev->NumArgs() > 1 ) _torsoAnim = ev->GetString( 2 );
	if ( ev->NumArgs() > 2 ) _minTime = ev->GetFloat( 3 );
	if ( ev->NumArgs() > 3 ) _maxTime = ev->GetFloat( 4 );	

}


//--------------------------------------------------------------
//
// Name:        Begin()
// Class:       PlayAnim
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void PlayAnim::Begin( Actor &self )
{          
	init( self );
}



//--------------------------------------------------------------
//
// Name:        Evaluate()
// Class:       PlayAnim
//
// Description: Update for this behavior -- called every server frame
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     BehaviorReturnCode_t
//
//--------------------------------------------------------------
BehaviorReturnCode_t	PlayAnim::Evaluate( Actor &self )
{

	BehaviorReturnCode_t stateResult;


	think();

	switch ( _state )
		{
		//---------------------------------------------------------------------
		case PLAYANIM_SETUP:
		//---------------------------------------------------------------------
			stateResult = evaluateStateSetup();

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( PLAYANIM_ANIMATE );
						
		break;

		//---------------------------------------------------------------------
		case PLAYANIM_ANIMATE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAnimate();

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( PLAYANIM_SUCCESS );
		break;
			
		//---------------------------------------------------------------------
		case PLAYANIM_SUCCESS:
		//---------------------------------------------------------------------
			return BEHAVIOR_SUCCESS;
		break;

		//---------------------------------------------------------------------
		case PLAYANIM_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;
		break;
		}
	
	return BEHAVIOR_EVALUATING;
}



//--------------------------------------------------------------
//
// Name:        End()
// Class:       PlayAnim
//
// Description: Ends this behavior -- cleans things up
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void PlayAnim::End(Actor &self)
{   
	self.RemoveAnimDoneEvent();
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
void PlayAnim::transitionToState( PlayAnimStates_t state )
{
	switch( state )
		{
		case PLAYANIM_SETUP:
			setInternalState( state , "PLAYANIM_SETUP" );
			setupStateSetup();
		break;

		case PLAYANIM_ANIMATE:
			setInternalState( state , "PLAYANIM_ANIMATE" );
			setupStateAnimate();
		break;

		case PLAYANIM_SUCCESS:
			setInternalState( state , "PLAYANIM_SUCCESS" );
		break;

		case PLAYANIM_FAILED:
			setInternalState( state , "PLAYANIM_FAILED" );
		break;
		}
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		PlayAnim
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void PlayAnim::setInternalState( PlayAnimStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );	
}

//--------------------------------------------------------------
// Name:		init()
// Class:		PlayAnim
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void PlayAnim::init( Actor &self )
{
	str legAnim, torsoAnim;
	ScriptVariable *var = NULL;

	//Check for State Var's first
	legAnim = self.GetStateVar( _legAnim );
	if ( legAnim.length() )
		_legAnim = legAnim;

	
	torsoAnim = self.GetStateVar( _torsoAnim );
	if ( torsoAnim.length() )
		_torsoAnim = torsoAnim;


	var = self.entityVars.GetVariable( _legAnim );
	if ( var )
		{
		legAnim = var->stringValue();
		if ( legAnim.length() )
			_legAnim = legAnim;
		}


	var = self.entityVars.GetVariable( _torsoAnim );
	if ( var )
		{
		torsoAnim = var->stringValue();
		if ( torsoAnim.length() )
			_torsoAnim = torsoAnim;
		}


	transitionToState(PLAYANIM_SETUP);	
}

//--------------------------------------------------------------
// Name:		think()
// Class:		PlayAnim
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void PlayAnim::think()
{	
}


//--------------------------------------------------------------
// Name:		setupStateSetup()
// Class:		PlayAnim
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void PlayAnim::setupStateSetup()
{
	if ( _minTime )
		_endTime = level.time + _minTime;

	if ( _maxTime )
		_endTime = _endTime + G_Random( _maxTime - _minTime );

 	if ( _legAnim.length() )
		{
		if ( !_endTime )
			{
			if ( !GetSelf()->SetAnim( _legAnim, EV_Actor_EndBehavior ) )
				{
				GetSelf()->PostEvent( EV_Actor_EndBehavior, 0.0f );
				}
			}
		else
			{	
			if ( !GetSelf()->SetAnim( _legAnim, NULL ) )
				{
				GetSelf()->PostEvent( EV_Actor_EndBehavior, 0.0f );
				}
			}

		GetSelf()->ClearTorsoAnim();
		}

	if ( _torsoAnim.length() )
		{
		if ( !_endTime )
			{
			if ( !GetSelf()->SetAnim( _torsoAnim, EV_Actor_EndBehavior ) )
				{
				GetSelf()->PostEvent( EV_Actor_EndBehavior, 0.0f );
				}
			}
		else
			{	
			if ( !GetSelf()->SetAnim( _torsoAnim, NULL ) )
				{
				GetSelf()->PostEvent( EV_Actor_EndBehavior, 0.0f );
				}
			}
		}
}

//--------------------------------------------------------------
// Name:		evaluateStateApproach()
// Class:		PlayAnim
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t PlayAnim::evaluateStateSetup()
{
	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateApproach()
// Class:		PlayAnim
//
// Description:	Failure Handler for State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void PlayAnim::failureStateSetup( const str& failureReason )
{
}


//--------------------------------------------------------------
// Name:		setupStateSetup()
// Class:		PlayAnim
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void PlayAnim::setupStateAnimate()
{ 
}

//--------------------------------------------------------------
// Name:		evaluateStateApproach()
// Class:		PlayAnim
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t PlayAnim::evaluateStateAnimate()
{
	if ( _endTime )
		{
		if ( level.time >= _endTime )
			return BEHAVIOR_SUCCESS;
		}
	

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateApproach()
// Class:		PlayAnim
//
// Description:	Failure Handler for State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void PlayAnim::failureStateAnimate( const str& failureReason )
{
}

