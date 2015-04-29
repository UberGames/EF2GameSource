//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/snipeEnemy.cpp                            $
// $Revision:: 10                                                             $
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
//	torsoAimAndFireWeapon Implementation
// 
// PARAMETERS:
//
// ANIMATIONS:
//--------------------------------------------------------------------------------

#include "actor.h"
#include "snipeEnemy.hpp"
#include <qcommon/gameplaymanager.h>

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, SnipeEnemy, NULL )
	{
		{ &EV_Behavior_Args,		&SnipeEnemy::SetArgs	},
		{ &EV_Behavior_AnimDone,	&SnipeEnemy::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		SnipeEnemy()
// Class:		SnipeEnemy
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
SnipeEnemy::SnipeEnemy()
{	
	_maxTorsoTurnSpeed	= 15.0f;
	_maxTorsoYaw		= 90.0;
	_maxTorsoPitch		= 40.0;
	_shots				= 1;
	_fireFailed			= false;

}	

//--------------------------------------------------------------
// Name:		~SnipeEnemy()
// Class:		SnipeEnemy
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
SnipeEnemy::~SnipeEnemy()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       SnipeEnemy
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void SnipeEnemy::SetArgs( Event *ev )
{	
	_aimTime = ev->GetFloat( 1 );
	_lockDownTime = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		{
		_targetSpread.x = ev->GetFloat(3);
		_targetSpread.y	= ev->GetFloat(4);
		_targetSpread.z	= ev->GetFloat(5);
		}
	
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       SnipeEnemy
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void SnipeEnemy::AnimDone( Event * )
{
	_animDone = true;
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       SnipeEnemy
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void SnipeEnemy::Begin( Actor &self )
{       
	init( self );

	if (_preFireAnim.length())
		transitionToState ( SNIPE_AIM_AND_FIRE_PRE_FIRE );
	else
		transitionToState ( SNIPE_AIM_AND_FIRE_AIM );
}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       SnipeEnemy
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t SnipeEnemy::Evaluate( Actor & )
{
	BehaviorReturnCode_t stateResult;

	think();
	
	switch ( _state )
		{
		//---------------------------------------------------------------------
		case SNIPE_AIM_AND_FIRE_PRE_FIRE:
		//---------------------------------------------------------------------
			stateResult = evaluateStatePreFire();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
					transitionToState( SNIPE_AIM_AND_FIRE_AIM );	
				}

			if ( stateResult == BEHAVIOR_FAILED )
				{
				transitionToState( SNIPE_AIM_AND_FIRE_FAILED );
				}
		break;

		//---------------------------------------------------------------------
		case SNIPE_AIM_AND_FIRE_AIM:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAim();
						
			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				transitionToState( SNIPE_AIM_AND_FIRE_ATTACK );
				}
			
			if ( stateResult == BEHAVIOR_FAILED )
				{
				transitionToState( SNIPE_AIM_AND_FIRE_FAILED );
				}
		break;


		//---------------------------------------------------------------------
		case SNIPE_AIM_AND_FIRE_ATTACK:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAttack();

			if ( stateResult == BEHAVIOR_FAILED )
				{
				_fireWeapon.End( *_self );

				if ( _postFireAnim.length() )
					transitionToState( SNIPE_AIM_AND_FIRE_POST_FIRE );
				else
					transitionToState( SNIPE_AIM_AND_FIRE_FAILED );
				}

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				_fireWeapon.End( *_self );

				if ( _postFireAnim.length() )
					transitionToState( SNIPE_AIM_AND_FIRE_POST_FIRE );
				else
					transitionToState( SNIPE_AIM_AND_FIRE_SUCCESS );
				}
		break;

		//---------------------------------------------------------------------
		case SNIPE_AIM_AND_FIRE_POST_FIRE:
		//---------------------------------------------------------------------
			stateResult = evaluateStatePostFire();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
					transitionToState( SNIPE_AIM_AND_FIRE_SUCCESS );	
				}
		break;

		//---------------------------------------------------------------------
		case SNIPE_AIM_AND_FIRE_SUCCESS:
		//---------------------------------------------------------------------
			_self->SetControllerAngles( ActorTorsoTag, vec_zero );
			return BEHAVIOR_SUCCESS;

		break;


		//---------------------------------------------------------------------
		case SNIPE_AIM_AND_FIRE_FAILED:
		//---------------------------------------------------------------------
			return BEHAVIOR_FAILED;

		break;


		}
	

	return BEHAVIOR_EVALUATING;   

}



//--------------------------------------------------------------
// Name:        End()
// Class:       SnipeEnemy
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void SnipeEnemy::End(Actor &)
{ 
	if ( !_self )
		return;


	_fireWeapon.End(*_self);
	//gi.Printf( "TorsoAimAndFireWeapon::End()\n");
	_self->SetControllerAngles( ActorTorsoTag, vec_zero );	
	_self->ClearTorsoAnim();
	_self->SetEnemyTargeted( false );
	//_self->SetAnim( _aimAnim , NULL , torso );
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		SnipeEnemy
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::transitionToState( SnipeAimAndFireStates_t state )
{	
	switch ( state )
		{	
		case SNIPE_AIM_AND_FIRE_AIM:
			setupStateAim();
			setInternalState( state , "SNIPE_AIM_AND_FIRE_AIM" );
		break;

		case SNIPE_AIM_AND_FIRE_PRE_FIRE:
			setupStatePreFire();
			setInternalState( state , "SNIPE_AIM_AND_FIRE_PRE_FIRE" );
		break;

		case SNIPE_AIM_AND_FIRE_ATTACK:
			setupStateAttack();
			setInternalState( state , "SNIPE_AIM_AND_FIRE_ATTACK" );
		break;

		case SNIPE_AIM_AND_FIRE_POST_FIRE:
			setupStatePostFire();
			setInternalState( state , "SNIPE_AIM_AND_FIRE_POST_FIRE" );
		break;

		case SNIPE_AIM_AND_FIRE_SUCCESS:
			setInternalState( state , "SNIPE_AIM_AND_FIRE_SUCCESS" );
		break;

		case SNIPE_AIM_AND_FIRE_FAILED:
			setInternalState( state , "SNIPE_AIM_AND_FIRE_FAILED" );
		break;
		}
	
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		SnipeEnemy
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::setInternalState( SnipeAimAndFireStates_t state , const str &stateName )
{
	_state = state;
	SetInternalStateName( stateName );
}

//--------------------------------------------------------------
// Name:		init()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Initializes the behavior
//
// Parameters:	Actor &self
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::init( Actor &self )
{
	_self = &self;

	//Set Our Controller Tag and set up our angles
	self.SetControllerTag( ActorTorsoTag, gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Spine1" ) );
	_currentTorsoAngles = self.GetControllerAngles( ActorTorsoTag );

	_animDone = false;
	_canAttack = true;

	_aimAnim		= "idle";
	_preFireAnim	= "idle";
	_fireAnim		= "idle";
	_postFireAnim	= "idle";

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(self.getArchetype()) )
		return;
		
	str objname = self.combatSubsystem->GetActiveWeaponArchetype();	
	objname = "Hold" + objname;

	if ( gpm->hasProperty(objname, "Aim") )
		_aimAnim = gpm->getStringValue( objname , "Aim" );
	
	if ( gpm->hasProperty( objname , "PreFire" ) )
		_preFireAnim = gpm->getStringValue( objname , "PreFire" );

	if ( gpm->hasProperty( objname , "Fire" ) )
		_fireAnim = gpm->getStringValue( objname , "Fire" );

	if ( gpm->hasProperty( objname , "PostFire" ) )
		_postFireAnim = gpm->getStringValue( objname , "PostFire" );

	if ( gpm->hasProperty( objname , "ShotCount" ) )
		_shots = (int)gpm->getFloatValue( objname , "ShotCount" );

	float spreadX, spreadY;
	spreadX = self.combatSubsystem->GetDataForMyWeapon( "spreadx" );
	spreadY = self.combatSubsystem->GetDataForMyWeapon( "spready" );
	self.combatSubsystem->OverrideSpread( spreadX , spreadY );


	//Clear Out Our VolleyCount
	_self->shotsFiredThisVolley = 0;

	updateEnemy();
	
}

//--------------------------------------------------------------
// Name:		think()
// Class:		SnipeEnemy
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::think()
{
	int    tagNum;	
	Vector tagPos;	
	Vector watchPosition;
	Actor *actTarget;
   actTarget = NULL;
   str targetBone;


  	tagNum = gi.Tag_NumForName( _self->edict->s.modelindex, "Bip01 Spine1" );

	if ( tagNum < 0 )
		return;

	_self->GetTag( "Bip01 Spine1", &tagPos );

   if ( !_currentEnemy )
      {
      LerpTorsoBySpeed( vec_zero );      
      return;
      }

	if ( _currentEnemy->isSubclassOf( Actor ) )
		{
		actTarget = (Actor *)(Entity *)_currentEnemy;

		// Don't watch if the target is dead.
		if ( !actTarget->isThinkOn() )
			{
			_currentEnemy = NULL;
			actTarget = NULL;
			return;
			}
		}

	targetBone = _currentEnemy->getTargetPos();

	watchPosition = _currentEnemy->centroid;

	if ( targetBone.length() )
		{
		if ( gi.Tag_NumForName( _currentEnemy->edict->s.modelindex , targetBone ) > 0 )
			{
			_currentEnemy->GetTag( targetBone.c_str() , &watchPosition , NULL , NULL , NULL );
			}		
		}

	


   AdjustTorsoAngles( tagPos , watchPosition );

}


//--------------------------------------------------------------
// Name:		updateEnemy()
// Class:		SnipeEnemy
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::updateEnemy()
{
	//
	// First, we try and get our current enemy.  If we don't have one
	// then we ask the enemyManager to evaluate and try again.
	// If that fails, then we look at our _forceAttack status.
	// If we are forcing and attack, then that means we want to
	// fire pretty much no matter what... So if we don't have a 
	// currentEnemy, then I'm going to see if the Player is a valid
	// target, and set that to be the current enemy
	
	Entity *currentEnemy = _self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		{
		_self->enemyManager->FindHighestHateEnemy();
		currentEnemy = _self->enemyManager->GetCurrentEnemy();
		if ( !currentEnemy )
			{
			Player* player;
			player = GetPlayer( 0 );
			}
			
		}

	_currentEnemy = currentEnemy;
}

//--------------------------------------------------------------
// Name:		LerpTorsoBySpeed()
// Class:		SnipeEnemy
//
// Description: Lerps the torso
//
// Parameters:	const Vector &angleDelta
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::LerpTorsoBySpeed( const Vector &angleDelta )
   {
   Vector anglesDiff;
   Vector change;
   Vector finalAngles;
   Vector currentTorsoAngles;

   anglesDiff = angleDelta;

   //Reset our Controller Tag
   _self->SetControllerTag( ActorTorsoTag, gi.Tag_NumForName( _self->edict->s.modelindex, "Bip01 Spine1" ) );


   // Make sure we don't change our head angles too much at once   
	change = anglesDiff - _currentTorsoAngles;

	if ( change[YAW] > _maxTorsoTurnSpeed )
		anglesDiff[YAW] = _currentTorsoAngles[YAW] + _maxTorsoTurnSpeed;
	else if ( change[YAW] < -_maxTorsoTurnSpeed )
		anglesDiff[YAW] = _currentTorsoAngles[YAW] - _maxTorsoTurnSpeed;

	if ( change[PITCH] > _maxTorsoTurnSpeed )
		anglesDiff[PITCH] = _currentTorsoAngles[PITCH] + _maxTorsoTurnSpeed;
	else if ( change[PITCH] < -_maxTorsoTurnSpeed )
		anglesDiff[PITCH] = _currentTorsoAngles[PITCH] - _maxTorsoTurnSpeed;

   
   finalAngles = anglesDiff;
   
   _self->SetControllerAngles( ActorTorsoTag, finalAngles );


	_currentTorsoAngles = anglesDiff;

   }

//--------------------------------------------------------------
// Name:		AdjustTorsoAngles()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Adjusts the Torso Angles
//
// Parameters:	const Vector &tagPos
//				const Vector &watchPos
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::AdjustTorsoAngles( const Vector &tagPos , const Vector &watchPosition )
   {
	Vector dir;
	Vector angles;
	Vector anglesDiff;
   float  yawChange;
   float  pitchChange;

   
	dir = watchPosition - tagPos;
	angles = dir.toAngles();

	anglesDiff = angles - _self->angles;
		

	anglesDiff[YAW]   = AngleNormalize180( anglesDiff[YAW] );
	anglesDiff[PITCH] = AngleNormalize180( anglesDiff[PITCH] );

	yawChange = anglesDiff[YAW];
	pitchChange = anglesDiff[PITCH];

	// Make sure we don't turn torso too far	
	if ( anglesDiff[YAW] < -_maxTorsoYaw )
		anglesDiff[YAW] = -_maxTorsoYaw;
	else if ( anglesDiff[YAW] > _maxTorsoYaw )
		anglesDiff[YAW] = _maxTorsoYaw;

	if ( anglesDiff[PITCH] < -_maxTorsoPitch )
		anglesDiff[PITCH] = -_maxTorsoPitch;
	else if ( anglesDiff[PITCH] > _maxTorsoPitch )
		anglesDiff[PITCH] = _maxTorsoPitch;

	anglesDiff[ROLL] = 0.0f;


   LerpTorsoBySpeed( anglesDiff );   

   }


//--------------------------------------------------------------
// Name:		setupStateAim()
// Class:		SnipeEnemy
//
// Description:	Sets up the Aim State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::setupStateAim()
{
	Weapon			*weapon;
	_animDone = false;
	_endAimTime = level.time + _aimTime;
	_endLockDownTime = level.time + _lockDownTime;
	_self->SetAnim( _aimAnim , EV_Actor_NotifyTorsoBehavior , torso );	
	_self->SetEnemyTargeted( true );

	weapon = _self->GetActiveWeapon( WeaponHandNameToNum( "dual" ) );
	
	if ( weapon )
		weapon->playAnim( "aim" );
}

//--------------------------------------------------------------
// Name:		evaluateStateAim()
// Class:		SnipeEnemy
//
// Description:	Evaluates the Aim State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t SnipeEnemy::evaluateStateAim()
{
	str targetBone;

	if ( level.time > _endAimTime )
		{
		_self->SetEnemyTargeted( false );
		return BEHAVIOR_SUCCESS;
		}

	if ( _self->combatSubsystem->CanAttackEnemy() )
		{
		targetBone = _currentEnemy->getTargetPos();
		_lastGoodPosition = _currentEnemy->centroid;

		if ( targetBone.length() )
			{
			if ( gi.Tag_NumForName( _currentEnemy->edict->s.modelindex , targetBone ) > 0 )
				{
				_currentEnemy->GetTag( targetBone.c_str() , &_lastGoodPosition , NULL , NULL , NULL );
				}		
			}
		}
	else
		{
		if ( level.time < _endLockDownTime )
			{
			_self->SetEnemyTargeted( false );
			return BEHAVIOR_FAILED;
			}
		}

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAim()
// Class:		SnipeEnemy
//
// Description:	Failure Handler for the Failure State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::failureStateAim( const str& failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		setupStatePreFire()
// Class:		SnipeEnemy
//
// Description: Sets up the PreFire State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::setupStatePreFire()
{
	_animDone = false;
	_self->SetAnim( _preFireAnim , EV_Actor_NotifyTorsoBehavior , torso );
}

//--------------------------------------------------------------
// Name:		evaluateStatePreFire()
// Class:		SnipeEnemy
//
// Description:	Evaluates the Pre Fire state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t SnipeEnemy::evaluateStatePreFire()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	if ( !_canAttack )
		return BEHAVIOR_FAILED;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStatePreFire()
// Class:		SnipeEnemy
//
// Description: Failure Handler for the Pre Fire State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::failureStatePreFire( const str& failureReason )
{
	Q_UNUSED(failureReason);
}


//--------------------------------------------------------------
// Name:		 setupStateAttack()
// Class:		 SnipeEnemy
//
// Description:	 Setup Attack State
//
// Parameters:	 None
//
// Returns:		 None
//--------------------------------------------------------------
void SnipeEnemy::setupStateAttack()
{
	float spreadx = G_CRandom(_targetSpread.x ) + _targetSpread.x;
	float spready = G_CRandom(_targetSpread.y ) + _targetSpread.y;
	float spreadz = G_CRandom(_targetSpread.z ) + _targetSpread.z;

	_lastGoodPosition.x = _lastGoodPosition.x + spreadx;
	_lastGoodPosition.y = _lastGoodPosition.y + spready;
	_lastGoodPosition.z = _lastGoodPosition.z + spreadz;
	
	_animDone = false;
	_fireWeapon.SetTargetPosition(_lastGoodPosition);
	_fireWeapon.SetAnim( _fireAnim ); 
	_fireWeapon.Begin( *_self );

}

//--------------------------------------------------------------
// Name:		evaluateStateAttack()
// Class:		SnipeEnemy
//
// Description:	Evaluates Attack State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t SnipeEnemy::evaluateStateAttack()
{
	BehaviorReturnCode_t result;

	if ( _fireFailed )
		return BEHAVIOR_FAILED;

	result = _fireWeapon.Evaluate( *_self );
	if ( _self->shotsFiredThisVolley >= _shots )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAttack()
// Class:		SnipeEnemy
//
// Description: Failure Handler for Attack State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::failureStateAttack( const str& failureReason )
{
	SetFailureReason( failureReason );
	_fireWeapon.End( *_self );
}

//--------------------------------------------------------------
// Name:		setupStatePostFire()
// Class:		SnipeEnemy
//
// Description:	Sets up Post Fire State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::setupStatePostFire()
{
	_animDone = false;
	_self->SetAnim( _postFireAnim , EV_Actor_NotifyTorsoBehavior , torso );
}

//--------------------------------------------------------------
// Name:		evaluateStatePostFire()
// Class:		SnipeEnemy
//
// Description:	Evaluates State Post Fire
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t SnipeEnemy::evaluateStatePostFire()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_FAILED;
}

//--------------------------------------------------------------
// Name:		failureStatePostFire()
// Class:		SnipeEnemy
//
// Description: Failure State Post Fire
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void SnipeEnemy::failureStatePostFire( const str& failureReason )
{
	Q_UNUSED(failureReason);
}

