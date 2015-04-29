//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actor_posturecontroller.cpp                    $
// $Revision:: 8                                                              $
//   $Author:: Steven                                                      $
//     $Date:: 5/04/03 2:02p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//-----------------------------------------------------------------------------

#include "_pch_cpp.h"

extern Event EV_Actor_SetPostureStateMap;

//--------------------------------------------------------------
// Name:		PostureController()
// Class:		PostureController
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
PostureController::PostureController()
{
	// Should always use other constructor
	gi.Error( ERR_FATAL, "PostureController::PostureController -- Default Constructor Called" );	
}

//--------------------------------------------------------------
// Name:		PostureController()
// Class:		PostureController
//
// Description:	Constructor
//
// Parameters:	Actor *actor
//
// Returns:		None
//--------------------------------------------------------------
PostureController::PostureController( Actor *actor )
{
	if ( actor )
		act = actor;
	else 
		gi.Error( ERR_DROP, "PostureController::PostureController -- actor is NULL" );   

   init();
}


//--------------------------------------------------------------
// Name:		~PostureController()
// Class:		PostureController
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
PostureController::~PostureController()
{
	act->freeConditionals( _postureConditionals );
}


//--------------------------------------------------------------
// Name:		DoArchive()
// Class:		PostureController
//
// Description:	Archives our Actor and Calls our Archive Function
//
// Parameters:	Archiver &arc -- The Archiver Object
//				Actor *actor -- The pointer to our actor
//
// Returns:		None
//--------------------------------------------------------------
void PostureController::DoArchive( Archiver &arc, Actor *actor )
{
	Archive( arc );
	if ( actor )
		act = actor;
	else
		gi.Error( ERR_FATAL, "PostureController::DoArchive -- actor is NULL" );	
	
}

//--------------------------------------------------------------
// Name:		Archive()
// Class:		PostureController
//
// Description:	Archives the class
//
// Parameters:	Archiver &arc
//
// Returns:		None
//--------------------------------------------------------------
void PostureController::Archive( Archiver &arc )
{
	arc.ArchiveString( &_postureStateMap_Name );
	arc.ArchiveString( &_currentPostureState_Name );
	arc.ArchiveString( &_requestedPostureState_Name );
	arc.ArchiveSafePointer ( &_requestor );

	if ( !arc.Saving() )
	{
		if ( _postureStateMap_Name.length() )
		{
			Event *event;

			event = new Event( EV_Actor_SetPostureStateMap );							  
			event->AddString( _postureStateMap_Name );
			event->AddInteger( 1 );
			act->ProcessEvent ( event );

			_currentPostureState = _postureStateMap->FindState( _currentPostureState_Name.c_str() );
			_requestedPostureState = _postureStateMap->FindState(_requestedPostureState_Name.c_str() );
		}
	}		
}

//--------------------------------------------------------------
// Name:		init()
// Class:		PostureController
//
// Description:	Initializes the class
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void PostureController::init()
{
	_postureStateMap = nullptr;
	_postureStateMap_Name = "";

	_currentPostureState = nullptr;
	_currentPostureState_Name = "";

	_requestedPostureState = nullptr;
	_requestedPostureState_Name = "";

	_requestor = nullptr;
}


//--------------------------------------------------------------
// Name:		evaluate()
// Class:		PostureController
//
// Description:	Evaluation Routine, Called Every Frame
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void PostureController::evaluate()
{
	int32_t count;
	State	*laststate;	
	str currentanim;
	
	auto stateLegAnim = act->animname;
	count = 0;
	
	if ( !_postureStateMap )
		return;
	
	if ( act->deadflag || !_currentPostureState )
		return;
	
	do
	{
		// Since we could get into an infinite loop here, do a check to make sure we don't.
		
		count++;
		if ( count > 10 )
		{
			gi.WDPrintf( "Possible infinite loop in posture state '%s'\n", _currentPostureState->getName() );
			if ( count > 50 )
			{
				gi.Error( ERR_DROP, "Stopping due to possible infinite state loop\n" );
				break;
			}
		}
		
		// Determine the next state to go to
		
		laststate = _currentPostureState;
		_currentPostureState = _currentPostureState->Evaluate( *act, &_postureConditionals );					
		
		if ( !_currentPostureState )
			return;
		
		_currentPostureState_Name = _currentPostureState->getName();
		
		// Change the behavior if the state has changed
		if ( laststate != _currentPostureState )
		{			
            // Initialize some stuff for changing states
			act->SetActorFlag( ACTOR_FLAG_POSTURE_ANIM_DONE, false );			
		}
		
		if ( _currentPostureState == _requestedPostureState )
		{
			if ( _requestor )
				_requestor->ProcessEvent( EV_PostureChanged_Completed );
		}
		
		// Change the animation if it has changed
		currentanim = _currentPostureState->getLegAnim( *act, &_postureConditionals );
		
		if ( currentanim.length() && stricmp( stateLegAnim , currentanim.c_str() ) != 0 )
		{
			act->SetAnim( currentanim, EV_Posture_Anim_Done, legs );
			stateLegAnim = currentanim;
		}
	}
	while( laststate != _currentPostureState );
	
	
	
}


//--------------------------------------------------------------
// Name:		requestPosture()
// Class:		PostureController
//
// Description:	Request Handler for a specific Posture
//
// Parameters:	PostureStates_t postureState
//
// Returns:		true  -- if the posture is viable
//				false -- if the posture in not viable
//--------------------------------------------------------------
bool PostureController::requestPosture( const str &postureState , Listener *requestor )
{
	if ( !_postureStateMap )
		return false;
	
	_requestedPostureState = _postureStateMap->FindState( postureState.c_str() );
	
	if ( _requestedPostureState )
	{
		_requestor = requestor;
		_requestedPostureState_Name = postureState;
		return true;
	}
	
	
	
	return false;
}

void PostureController::setPostureStateMap( const str &stateMap , bool loading )
{
	// Load the new state map
	_postureStateMap_Name = stateMap;

	//_postureConditionals.FreeObjectList();
	act->freeConditionals( _postureConditionals );
	_postureStateMap = GetStatemap( _postureStateMap_Name, reinterpret_cast<Condition<Class> * >(act->Conditions), &_postureConditionals, false );

	// Set the first state
	_currentPostureState_Name = "START";

	// Initialize the actors first animation
	if ( !loading )
		setPostureState( _currentPostureState_Name.c_str() );		
}

void PostureController::setPostureState( const str &postureState )
{

	if ( !_postureStateMap )
		return;

	if ( act->deadflag )
		return;

   _currentPostureState = _postureStateMap->FindState( postureState.c_str() );
	

}

void PostureController::setPostureState( const str &postureState, const str &requestedState )
{
	if ( !_postureStateMap )
		return;

	if ( act->deadflag )
		return;

	_currentPostureState = _postureStateMap->FindState( postureState.c_str() );
	_requestedPostureState = _postureStateMap->FindState( requestedState.c_str() );
	_requestedPostureState_Name = requestedState.c_str();
}
