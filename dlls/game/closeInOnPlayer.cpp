//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/closeInOnPlayer.cpp              $
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
//	CloseInOnPlayer Implementation
// 
// PARAMETERS:
//		
//
// ANIMATIONS:
//		
//--------------------------------------------------------------------------------

#include "actor.h"
#include "closeInOnPlayer.hpp"

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, CloseInOnPlayer, NULL )
	{
		{ &EV_Behavior_Args,			&CloseInOnPlayer::SetArgs },      
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		CloseInOnPlayer()
// Class:		CloseInOnPlayer
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CloseInOnPlayer::CloseInOnPlayer()
{
	_anim = "";
	_torsoAnim = "";
	_dist = 64.0f;
}

//--------------------------------------------------------------
// Name:		~CloseInOnPlayer()
// Class:		CloseInOnPlayer
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
CloseInOnPlayer::~CloseInOnPlayer()
{
}


//--------------------------------------------------------------
//
// Name:        SetArgs()
// Class:       CloseInOnPlayer
//
// Description: 
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
//--------------------------------------------------------------
void CloseInOnPlayer::SetArgs( Event *ev )
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
// Class:       CloseInOnPlayer
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void CloseInOnPlayer::Begin( Actor &self )
{          
	init( self );
}



//--------------------------------------------------------------
//
// Name:        Evaluate()
// Class:       CloseInOnPlayer
//
// Description: Update for this behavior -- called every server frame
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     BehaviorReturnCode_t
//
//--------------------------------------------------------------
BehaviorReturnCode_t	CloseInOnPlayer::Evaluate( Actor & )
{

	BehaviorReturnCode_t stateResult;


	think();

	switch ( _state )
		{
		//---------------------------------------------------------------------
		case CLOSE_IN_ON_PLAYER_APPROACH:
		//---------------------------------------------------------------------
			stateResult = evaluateStateApproach();
			if ( stateResult == BEHAVIOR_FAILED )
				transitionToState( CLOSE_IN_ON_PLAYER_FAILED );

			if ( stateResult == BEHAVIOR_FAILED_STEERING_NO_PATH )
				transitionToState( CLOSE_IN_ON_PLAYER_FAILED );

			if ( stateResult == BEHAVIOR_SUCCESS )
				transitionToState( CLOSE_IN_ON_PLAYER_SUCCESS );
		break;

		//---------------------------------------------------------------------
		case CLOSE_IN_ON_PLAYER_SUCCESS:
		//---------------------------------------------------------------------
			return BEHAVIOR_SUCCESS;
		break;

		//---------------------------------------------------------------------
		case CLOSE_IN_ON_PLAYER_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;
		break;
		}
	
	return BEHAVIOR_EVALUATING;
}



//--------------------------------------------------------------
//
// Name:        End()
// Class:       CloseInOnPlayer
//
// Description: Ends this behavior -- cleans things up
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void CloseInOnPlayer::End(Actor &self)
{   
	_chase.End( self );
}


//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		CloseInOnPlayer
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnPlayer::transitionToState( closeInOnPlayerStates_t state )
{
	switch( state )
		{
		case CLOSE_IN_ON_PLAYER_APPROACH:
			setupStateApproach();
			setInternalState( state , "CLOSE_IN_ON_PLAYER_APPROACH" );
		break;

		case CLOSE_IN_ON_PLAYER_SUCCESS:
			setInternalState( state , "CLOSE_IN_ON_PLAYER_SUCCESS" );
		break;

		case CLOSE_IN_ON_PLAYER_FAILED:
			setInternalState( state , "CLOSE_IN_ON_PLAYER_FAILED" );
		break;
		}
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		CloseInOnPlayer
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnPlayer::setInternalState( closeInOnPlayerStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );	
}

//--------------------------------------------------------------
// Name:		init()
// Class:		CloseInOnPlayer
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnPlayer::init( Actor &self )
{
	_self = &self;
	findPlayer();	
	transitionToState(CLOSE_IN_ON_PLAYER_APPROACH);
}

//--------------------------------------------------------------
// Name:		think()
// Class:		CloseInOnPlayer
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnPlayer::think()
{	
}


//--------------------------------------------------------------
// Name:		findPlayer()
// Class:		CloseInOnPlayer
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnPlayer::findPlayer()
{
	Entity *player = (Entity*)GetPlayer(0);

	if ( !player || player->flags & FL_NOTARGET )
		{
		SetFailureReason( "CloseInOnPlayer::findPlayer -- No Player" );
		transitionToState( CLOSE_IN_ON_PLAYER_FAILED );
		return;
		}

	_player = player;

}

//--------------------------------------------------------------
// Name:		setTorsoAnim()
// Class:		CloseInOnPlayer
//
// Description:	Sets our Torso Animation
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnPlayer::setTorsoAnim()
{
	_self->SetAnim( _torsoAnim , NULL , torso );
}

//--------------------------------------------------------------
// Name:		setupStateApproach()
// Class:		CloseInOnPlayer
//
// Description:	Sets up State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnPlayer::setupStateApproach()
{
   _chase.SetAnim( _anim );
   _chase.SetDistance( _dist );   
   _chase.SetEntity( *_self, _player );

   if ( _torsoAnim.length() > 0 )
	   setTorsoAnim();

   _chase.Begin( *_self );
}

//--------------------------------------------------------------
// Name:		evaluateStateApproach()
// Class:		CloseInOnPlayer
//
// Description:	Evaluates State
//
// Parameters:	None
//
// Returns:		BehaviorReturnCode_t
//--------------------------------------------------------------
BehaviorReturnCode_t CloseInOnPlayer::evaluateStateApproach()
{
	return _chase.Evaluate( *_self );
}

//--------------------------------------------------------------
// Name:		failureStateApproach()
// Class:		CloseInOnPlayer
//
// Description:	Failure Handler for State
//
// Parameters:	const str &failureReason
//
// Returns:		None
//--------------------------------------------------------------
void CloseInOnPlayer::failureStateApproach( const str& failureReason )
{
	Q_UNUSED(failureReason);
}
