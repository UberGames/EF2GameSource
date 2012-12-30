//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/healGroupMember.cpp                        $
// $Revision:: 6                                                              $
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
//		Generates a triage list from the actors group, then based on priority 
//		of patients ( right now, solely health based -- but distance is stored
//		so we can factor that in if we need to ) it will run to the patient
//		and play the specified "heal" animation... After the animation it will
//		send a heal event to the patient which should kick start the regen process
//	
// PARAMETERS:
//		_anim					-- The animation to play while "healing"		
//		_healDistance			-- How far away to get from the patient
//		_maxDistance			-- How far away to consider a patient 	
//		_initialHealPercentage	-- How much to heal right away
//		_regenHealPercentage	-- How much to heal at each interval
//		_regenInterval			-- How often to regen
//		_maxPercentage			-- How Much Total Health can be regened
//	
// ANIMATIONS:
//		Heal Animation	: Parameter
//		"run"			: TIKI Requirement
//		"duck"			: TIKI Requirement
//
//	
//--------------------------------------------------------------------------------

#include "actor.h"
#include "healGroupMember.hpp"

extern Event EV_Sentient_HealOverTime;
//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, HealGroupMember, NULL )
	{
		{ &EV_Behavior_Args,		&HealGroupMember::SetArgs	},
		{ &EV_Behavior_AnimDone,	&HealGroupMember::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		HealGroupMember()
// Class:		HealGroupMember
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
HealGroupMember::HealGroupMember()
{
	_nextTriageUpdate = 0.0f;
}

//--------------------------------------------------------------
// Name:		HealGroupMember()
// Class:		HealGroupMember
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
HealGroupMember::~HealGroupMember()
{
	triageEntry_t* checkEntry;

	for ( int i = _triageList.NumObjects() ; i > 0 ; i-- )
		{
		checkEntry = _triageList.ObjectAt( i );
		delete checkEntry;
		checkEntry = NULL;
		_triageList.RemoveObjectAt( i );
		}
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       HealGroupMember
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void HealGroupMember::SetArgs( Event *ev )
{
	_anim					= ev->GetString( 1 );	
	_healDistance			= ev->GetFloat ( 2 );
	_maxDistance			= ev->GetFloat ( 3 );
	_initialHealPercentage	= ev->GetFloat ( 4 );
	_regenHealPercentage	= ev->GetFloat ( 5 );
	_regenInterval			= ev->GetFloat ( 6 );
	_maxPercentage			= ev->GetFloat ( 7 );
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       HealGroupMember
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void HealGroupMember::AnimDone( Event *ev )
{
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       HealGroupMember
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void HealGroupMember::Begin( Actor &self )
{       
	init( self );
}



//--------------------------------------------------------------
// Name:        init()
// Class:       HealGroupMember
//
// Description: Initializes memeber variables
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void HealGroupMember::init( Actor &self )
{	
	updateTriageList( self );
	_currentPatient = findHighestPriorityPatient( self );
	setupGotoEntity( self );
	_state = HGM_STATE_GOTO_ENTITY;
	updateTriageList( self );
	
}



//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       HealGroupMember
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t HealGroupMember::Evaluate( Actor &self )
{  
	Event *injuredEvent;

	if ( !_currentPatient )
		{
		SetFailureReason( "No Group Member To Heal" );
		_state = HGM_STATE_FAILED;
		}

	switch ( _state )
		{
		case HGM_STATE_GOTO_ENTITY:
			doGotoEntity( self );
		break;

		case HGM_STATE_FACE_TARGET:
			doRotateToEntity( self );
		break;

		case HGM_STATE_ANIMATE:
			doAnimate( self );
		break;

		case HGM_STATE_HEAL:
			doHeal( self );
		break;

		case HGM_STATE_FAILED:
			return BEHAVIOR_FAILED;
		break;

		case HGM_STATE_SUCCESS:
			if ( allPatientsTreated( self ) )
				{
				injuredEvent = new Event ( EV_Sentient_GroupMemberInjured );
				injuredEvent ->AddInteger( 0 );
				groupcoordinator->SendEventToGroup( injuredEvent , self.GetGroupID() );
				return BEHAVIOR_SUCCESS;
				}				
			else
				{
				init( self );
				}
		break;

		}

	return BEHAVIOR_EVALUATING;   
}



//--------------------------------------------------------------
// Name:        End()
// Class:       HealGroupMember
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void HealGroupMember::End(Actor &self)
{   
}

//--------------------------------------------------------------
// Name:		setupGotoEntity()
// Class:		HealGroupMember
//
// Description:	Sets up the GotoEntity Component
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::setupGotoEntity( Actor &self )
{
	_gotoEntity.SetEntity	( self, _currentPatient );
	_gotoEntity.SetDistance ( _healDistance );
	_gotoEntity.SetAnim		( "run"			);
	_gotoEntity.Begin		( self			);
	
}

//--------------------------------------------------------------
// Name:		doGotoEntity()
// Class:		HealGroupMember
//
// Description:	Evaluates GotoEntity Component
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::doGotoEntity( Actor &self )
{
	BehaviorReturnCode_t result;
	result = _gotoEntity.Evaluate( self );
	
	// Check if we need to re-evaluate the triage list
	if ( level.time > _nextTriageUpdate )
		{
		EntityPtr lastPatient;
		lastPatient = _currentPatient;

		updateTriageList( self );
		_currentPatient = findHighestPriorityPatient( self );

		if ( _currentPatient != lastPatient )			
			setupGotoEntity( self );
						
		}

	//Check if we are close enough
	if ( self.WithinDistance( _currentPatient , _healDistance ) || result == BEHAVIOR_SUCCESS )
		{
		_gotoEntity.End( self );
		_state = HGM_STATE_FACE_TARGET;
		setupRotateToEntity( self );		
		return;
		}

	//Check for any failure condition
	if ( result != BEHAVIOR_EVALUATING )
		{		
		gotoEntityFailed( self );
		}
}

//--------------------------------------------------------------
// Name:		gotoEntityFailed()
// Class:		HealGroupMember
//
// Description:	Failure Handler for GotoEntity Component
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::gotoEntityFailed( Actor &self )
{
	str FailureReason;
	FailureReason = "GotoEntity Component Failed:  " + _gotoEntity.GetFailureReason() + "\n";	
	SetFailureReason( FailureReason );

	_gotoEntity.End( self );
	_state = HGM_STATE_FAILED;
}

//--------------------------------------------------------------
// Name:		doHeal()
// Class:		HealGroupMember
//
// Description:	Creates and HealOverTime event with the paramters
//				passed into the behavior and tells the "injured"
//				groupMember to process it
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::doHeal( Actor &self )
{
	Event *regenEvent;

	regenEvent = new Event ( EV_Sentient_HealOverTime );
	
	if ( !regenEvent)
		{
		assert(regenEvent);
		return;
		}

	regenEvent->AddFloat( _initialHealPercentage	);
	regenEvent->AddFloat( _regenHealPercentage		);
	regenEvent->AddFloat( _regenInterval			);
	regenEvent->AddFloat( _maxPercentage			);

	_currentPatient->ProcessEvent( regenEvent );
	treatedPatient( self, _currentPatient );

	_state = HGM_STATE_SUCCESS;

}

//--------------------------------------------------------------
// Name:		setupRotateToEntity( Actor &self )
// Class:		HealGroupMember
//
// Description:	Sets up the RotateToEntity Component
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::setupRotateToEntity( Actor &self )
{	
	_rotateToEntity.SetEntity( _currentPatient );
	_rotateToEntity.Begin( self );
}

//--------------------------------------------------------------
// Name:		doRotateToEntity( Actor &self )
// Class:		HealGroupMember
//
// Description:	Evaluates the RotateToEntity Component
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::doRotateToEntity( Actor &self )
{
	BehaviorReturnCode_t result;
	result = _rotateToEntity.Evaluate( self );

	if ( result == BEHAVIOR_SUCCESS )
		{
		_rotateToEntity.End( self );
		_state = HGM_STATE_ANIMATE;
		setupAnimate( self );
		return;
		}
	
	if ( result != BEHAVIOR_EVALUATING )
		{
		rotateToEntityFailed( self );
		}

}

//--------------------------------------------------------------
// Name:		rotateToEntityFailed( Actor &self )
// Class:		HealGroupMember
//
// Description:	Failure Handler for the RotateToEntity Component
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::rotateToEntityFailed( Actor &self )
{
	str FailureReason;
	FailureReason = "RotateToEntity Component Failed:  " + _rotateToEntity.GetFailureReason() + "\n";	
	SetFailureReason( FailureReason );

	_rotateToEntity.End( self );
	_state = HGM_STATE_FAILED;
}

//--------------------------------------------------------------
// Name:		setupAnimate()
// Class:		HealGroupMember
//
// Description:	Sets up the Animate State
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::setupAnimate( Actor &self )
{
/*
	PostureStates_t pState = _currentPatient->movementSubsystem->getPostureState();

	switch ( pState )
		{
		case POSTURE_TRANSITION_STAND_TO_CROUCH:
			_legAnim = "duck";
		break;

		case POSTURE_TRANSITION_STAND_TO_PRONE:
			_legAnim = "duck";
		break;

		case POSTURE_TRANSITION_CROUCH_TO_STAND:
			_legAnim = "idle";
		break;

		case POSTURE_TRANSITION_CROUCH_TO_PRONE:
			_legAnim = "duck";
		break;

		case POSTURE_TRANSITION_PRONE_TO_CROUCH:
			_legAnim = "duck";
		break;

		case POSTURE_TRANSITION_PRONE_TO_STAND:
			_legAnim = "idle";
		break;

   		case POSTURE_STAND:
			_legAnim = "idle";
		break;

		case POSTURE_CROUCH:
			_legAnim = "duck";
		break;

		case POSTURE_PRONE:
			_legAnim = "duck";
		break;

		default:
			_legAnim = "idle";
		break;

		}

*/

}

//--------------------------------------------------------------
// Name:		doAnimate()
// Class:		HealGroupMember
//
// Description:	Evaluates up the Animate State
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::doAnimate( Actor &self )
{
	self.SetAnim( _legAnim );
	self.SetAnim( _anim , NULL , torso );

	_state = HGM_STATE_HEAL;
	
}

//--------------------------------------------------------------
// Name:		animateFailed()
// Class:		HealGroupMember
//
// Description:	Failure Handler the Animate State
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::animateFailed( Actor &self )
{
	str FailureReason;
	FailureReason = "Animation Not Found\n";
	SetFailureReason ( FailureReason );

	_state = HGM_STATE_FAILED;
}

//--------------------------------------------------------------
// Name:		treatedPatient()
// Class:		HealGroupMember
//
// Description:	Sets the "treated" flag of the passed in entity's
//				triageEntry to true
//
// Parameters:	Actor &self
//				Entity *ent
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::treatedPatient( Actor &self , Entity *ent )
{
	triageEntry_t* checkEntry;

	//Lets check if this entity is already in the triage list
	for ( int i = _triageList.NumObjects() ; i > 0 ; i-- )
		{	
		checkEntry = _triageList.ObjectAt( i );
		if ( checkEntry->ent == ent )
			{
			checkEntry->treated = true;
			}
		}	
}

//--------------------------------------------------------------
// Name:		findHighestPriorityPatient()
// Class:		HealGroupMember
//
// Description:	Finds the entity in the triage list with the
//				lowest health
//
// Parameters:	Actor &self
//
// Returns:		Actor*
//--------------------------------------------------------------
Actor* HealGroupMember::findHighestPriorityPatient( Actor &self )
{
	triageEntry_t* checkEntry;
	Actor *highestPriorityPatient;

	float lowestHealth;

	lowestHealth = 9999999.9f;
	highestPriorityPatient = 0;

	//Lets check if this entity is already in the triage list
	for ( int i = _triageList.NumObjects() ; i > 0 ; i-- )
		{	
		checkEntry = _triageList.ObjectAt( i );
		if ( checkEntry->health < lowestHealth && checkEntry->dist <= _maxDistance )
			{
			lowestHealth = checkEntry->health;
			highestPriorityPatient = (Actor*)(Entity*)checkEntry->ent;
			}
		}

	return highestPriorityPatient;
}

//--------------------------------------------------------------
// Name:		allPatientsTreated()
// Class:		HealGroupMember
//
// Description:	Iterates through the triageList and checks if
//				every entry has it's "treated" flag set to true
//
// Parameters:	Actor &self
//
// Returns:		True or False
//--------------------------------------------------------------
bool HealGroupMember::allPatientsTreated( Actor &self )
{
	triageEntry_t* checkEntry;

	//Lets check if this entity is already in the triage list
	for ( int i = _triageList.NumObjects() ; i > 0 ; i-- )
		{	
		checkEntry = _triageList.ObjectAt( i );
		if (!checkEntry->treated )
				return false;		
		}

	return true;
}

//--------------------------------------------------------------
// Name:		patientTreated()
// Class:		HealGroupMember
//
// Description:	Checks if an individual patient has been 
//				treated
//
// Parameters:	Actor &self
//				Entity *ent
//
// Returns:		True or False
//--------------------------------------------------------------
bool HealGroupMember::patientTreated( Actor &self , Entity *ent )
{
	triageEntry_t* checkEntry;

	//Lets check if this entity is already in the triage list
	for ( int i = _triageList.NumObjects() ; i > 0 ; i-- )
		{	
		checkEntry = _triageList.ObjectAt( i );
		if ( checkEntry->ent == ent )
			{
			return checkEntry->treated;
			}
		}

	return false;
}

//--------------------------------------------------------------
// Name:		updateTriageList()
// Class:		HealGroupMember
//	
// Description:	Grabs the actor's group from the group list, then
//				iterates through it, updating the triageList as 
//				necessary.
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::updateTriageList( Actor &self )
{
	Group *group;
	Entity *groupEnt;
	float health , maxHealth;

	groupEnt = NULL;

	group = groupcoordinator->GetGroup( self.GetGroupID() );	

	groupEnt = group->GetNextMember( groupEnt );

	while ( groupEnt != NULL )
		{
		health = groupEnt->getHealth();
		maxHealth = groupEnt->getMaxHealth();
		
		// See if this group member is below 50%
		if ( ( health / maxHealth ) < .5 )
			updateTriageEntry( self , groupEnt );		
			
		groupEnt = group->GetNextMember( groupEnt );
		}

	_nextTriageUpdate = level.time + G_Random() + 2.5f;
}

//--------------------------------------------------------------
// Name:		updateTriageEntry()
// Class:		HealGroupMember
//
// Description:	Updates or Adds an indivdual triageEntry in the list
//
// Parameters:	Actor &self
//				Entity *ent
//
// Returns:		None
//--------------------------------------------------------------
void HealGroupMember::updateTriageEntry( Actor &self , Entity *ent)
{
	float entHealth;
	float entDist;
	triageEntry_t* checkEntry;

	entHealth = ent->getHealth();
	entDist = Vector::Distance( self.origin , ent->origin );

	//Lets check if this entity is already in the triage list
	for ( int i = _triageList.NumObjects() ; i > 0 ; i-- )
		{	
		checkEntry = _triageList.ObjectAt( i );
		if ( checkEntry->ent == ent )
			{
			checkEntry->health = entHealth;
			checkEntry->dist = entDist;
			return;
			}
		}

	//Well the entity wasn't in the list, so we need to add it
	checkEntry = new triageEntry_t;
	checkEntry->ent = ent;
	checkEntry->health = entHealth;
	checkEntry->dist = entDist;
	checkEntry->treated = false;

	_triageList.AddObject(checkEntry);
}
