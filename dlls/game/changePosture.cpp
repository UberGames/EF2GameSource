//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/changePosture.cpp                 $
// $Revision:: 3                                                              $
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
//	changePosture Implementation
// 
// PARAMETERS:
//		
//
// ANIMATIONS:
//		
//--------------------------------------------------------------------------------

#include "actor.h"
#include "changePosture.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, ChangePosture, NULL )
	{
		{ &EV_Behavior_Args,		&ChangePosture::SetArgs},  
		{ &EV_PostureChanged_Completed, &ChangePosture::PostureDone },
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		ChangePosture()
// Class:		ChangePosture
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
ChangePosture::ChangePosture()
{
	_posture = "";	
}

//--------------------------------------------------------------
// Name:		~ChangePosture()
// Class:		ChangePosture
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
ChangePosture::~ChangePosture()
{
}


//--------------------------------------------------------------
//
// Name:        SetArgs()
// Class:       ChangePosture
//
// Description: 
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
//--------------------------------------------------------------
void ChangePosture::SetArgs( Event *ev )
{
	_posture = ev->GetString( 1 );
}

void ChangePosture::PostureDone( Event *ev )
{
	_postureDone = true;
}

//--------------------------------------------------------------
//
// Name:        Begin()
// Class:       ChangePosture
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void ChangePosture::Begin( Actor &self )
{          
	init();
}



//--------------------------------------------------------------
//
// Name:        Evaluate()
// Class:       ChangePosture
//
// Description: Update for this behavior -- called every server frame
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     BehaviorReturnCode_t
//
//--------------------------------------------------------------
BehaviorReturnCode_t	ChangePosture::Evaluate( Actor &self )
{

	BehaviorReturnCode_t stateResult;


	think();

	switch ( _state )
		{

		//---------------------------------------------------------------------
		case CHANGE_POSTURE_SETUP:
		//---------------------------------------------------------------------
			stateResult = evaluateStateSetup();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( CHANGE_POSTURE_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( CHANGE_POSTURE_CHANGE );
		break;

		//---------------------------------------------------------------------
		case CHANGE_POSTURE_CHANGE:
		//---------------------------------------------------------------------
			stateResult = evaluateStateChange();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( CHANGE_POSTURE_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( CHANGE_POSTURE_SUCCESS );
		break;

		//---------------------------------------------------------------------
		case CHANGE_POSTURE_SUCCESS:
		//---------------------------------------------------------------------
			return BEHAVIOR_SUCCESS;
		break;

		//---------------------------------------------------------------------
		case CHANGE_POSTURE_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;
		break;
		}
	
	return BEHAVIOR_EVALUATING;
}



//--------------------------------------------------------------
//
// Name:        End()
// Class:       ChangePosture
//
// Description: Ends this behavior -- cleans things up
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void ChangePosture::End(Actor &self)
{   
}


//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		ChangePosture
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void ChangePosture::transitionToState( changePostureStates_t state )
{
	switch( state )
		{
		case CHANGE_POSTURE_SETUP:
			setupStateSetup();
			setInternalState( state , "CHANGE_POSTURE_SETUP" );
		break;

		case CHANGE_POSTURE_CHANGE:
			setupStateChange();
			setInternalState( state , "CHANGE_POSTURE_CHANGE" );
		break;

		case CHANGE_POSTURE_SUCCESS:
			setInternalState( state , "CHANGE_POSTURE_SUCCESS" );
		break;

		case CHANGE_POSTURE_FAILED:
			setInternalState( state , "CHANGE_POSTURE_FAILED" );
		break;
		}
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		ChangePosture
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void ChangePosture::setInternalState( changePostureStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );	
}

//--------------------------------------------------------------
// Name:		init()
// Class:		ChangePosture
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void ChangePosture::init()
{
	transitionToState(CHANGE_POSTURE_SETUP);
	_canChange = true;
	_postureDone = false;
}

//--------------------------------------------------------------
// Name:		think()
// Class:		ChangePosture
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void ChangePosture::think()
{	
}


//--------------------------------------------------------------
// Name:		setupStateSetup()
// Class:		ChangePosture
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void ChangePosture::setupStateSetup()
{
	_canChange = GetSelf()->postureController->requestPosture( _posture , this );
}

//--------------------------------------------------------------
// Name:		evaluateStateSetup()
// Class:		ChangePosture
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t ChangePosture::evaluateStateSetup()
{
	if ( !_canChange )
		return BEHAVIOR_FAILED;

	return BEHAVIOR_SUCCESS;
}

//--------------------------------------------------------------
// Name:		failureStateSetup()
// Class:		ChangePosture
//
// Description:	Failure Handler for State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void ChangePosture::failureStateSetup( const str& failureReason )
{
}

//--------------------------------------------------------------
// Name:		setupStateChange()
// Class:		ChangePosture
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void ChangePosture::setupStateChange()
{
}

//--------------------------------------------------------------
// Name:		evaluateStateChange()
// Class:		ChangePosture
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t ChangePosture::evaluateStateChange()
{	
	if ( _postureDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateChange()
// Class:		ChangePosture
//
// Description:	Failure Handler for State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void ChangePosture::failureStateChange( const str& failureReason )
{
}
