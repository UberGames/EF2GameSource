//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/torsoAimAndFireWeapon.cpp                 $
// $Revision:: 20                                                             $
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
#include "torsoAimAndFireWeapon.hpp"
#include <qcommon/gameplaymanager.h>

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, TorsoAimAndFireWeapon, NULL )
	{
		{ &EV_Behavior_Args,		&TorsoAimAndFireWeapon::SetArgs		},
		{ &EV_Behavior_AnimDone,	&TorsoAimAndFireWeapon::AnimDone	}, 
		{ NULL, NULL }
	};


//--------------------------------------------------------------
// Name:		TorsoAimAndFireWeapon()
// Class:		StationaryFireCombatEX
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
TorsoAimAndFireWeapon::TorsoAimAndFireWeapon()
{	
	_endFireTime		= 0.0f;
	_endAimTime			= 0.0f;
	_maxTorsoTurnSpeed	= 15.0f;
	_maxTorsoYaw		= 90.0;
	_maxTorsoPitch		= 40.0;
	_aimOnly			= false;
	_shots				= 1;
	_fireFailed			= false;
	_repeat				= false;

}	

//--------------------------------------------------------------
// Name:		~TorsoAimAndFireWeapon()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
TorsoAimAndFireWeapon::~TorsoAimAndFireWeapon()
{
}

//--------------------------------------------------------------
// Name:        SetArgs()
// Class:       TorsoAimAndFireWeapon
//
// Description: Sets Arguments for this behavior
//
// Parameters:  Event *ev -- Event holding the arguments
//
// Returns:     None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::SetArgs( Event *ev )
{	
	_forceAttack		= ev->GetBoolean( 1 );
	if ( ev->NumArgs() > 1 )
		_aimOnly = ev->GetBoolean( 2 );

	if ( ev->NumArgs() > 2 )
		_repeat = ev->GetBoolean( 3 );

	if ( ev->NumArgs() > 3 )
		_endOnAimFail = ev->GetBoolean( 4 );
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       TorsoAimAndFireWeapon
//
// Description: Handles an animation completion
//
// Parameters:  Event *ev -- Event holding the completion notification
//
// Returns:     None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::AnimDone( Event *ev )
{

	/*if ( _state == TORSO_AIM_AND_FIRE_POST_FIRE )
		{
		gi.WDPrintf( "-----------------------------------------\n");
		gi.WDPrintf( "PostFireAnimDone\n" );
		gi.WDPrintf( "-----------------------------------------\n");
		}*/

	_animDone = true;
}



//--------------------------------------------------------------
// Name:        Begin()
// Class:       TorsoAimAndFireWeapon
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::Begin( Actor &self )
{       
	init( self );

	if (_preFireAnim.length() && !_aimOnly )
		transitionToState ( TORSO_AIM_AND_FIRE_PRE_FIRE );
	else
		transitionToState ( TORSO_AIM_AND_FIRE_AIM );
}


//--------------------------------------------------------------
// Name:        Evaluate()
// Class:       TorsoAimAndFireWeapon
//
// Description: Evaluates the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     BehaviorReturnCode_t 
//--------------------------------------------------------------
BehaviorReturnCode_t TorsoAimAndFireWeapon::Evaluate( Actor &self )
{
	BehaviorReturnCode_t stateResult;

	think();
	
	switch ( _state )
		{
		//---------------------------------------------------------------------
		case TORSO_AIM_AND_FIRE_PRE_FIRE:
		//---------------------------------------------------------------------
			stateResult = evaluateStatePreFire();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
					transitionToState( TORSO_AIM_AND_FIRE_AIM );	
				}

			if ( stateResult == BEHAVIOR_FAILED )
				{
				transitionToState( TORSO_AIM_AND_FIRE_FAILED );
				}
		break;

		//---------------------------------------------------------------------
		case TORSO_AIM_AND_FIRE_AIM:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAim();
						
			if ( stateResult == BEHAVIOR_SUCCESS )
				{
					if ( _aimOnly )
						transitionToState(TORSO_AIM_AND_FIRE_AIM);
					else
						transitionToState( TORSO_AIM_AND_FIRE_ATTACK );
				}
			
			if ( stateResult == BEHAVIOR_FAILED )
				return BEHAVIOR_FAILED;

		break;


		//---------------------------------------------------------------------
		case TORSO_AIM_AND_FIRE_ATTACK:
		//---------------------------------------------------------------------
			stateResult = evaluateStateAttack();

			if ( stateResult == BEHAVIOR_FAILED )
				{
				_fireWeapon.End( *_self );

				if ( _postFireAnim.length() )
					transitionToState( TORSO_AIM_AND_FIRE_POST_FIRE );
				else
					transitionToState( TORSO_AIM_AND_FIRE_FAILED );
				}

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
				_fireWeapon.End( *_self );

				if ( _postFireAnim.length() )
					transitionToState( TORSO_AIM_AND_FIRE_POST_FIRE );
				else
					transitionToState( TORSO_AIM_AND_FIRE_SUCCESS );
				}
		break;

		//---------------------------------------------------------------------
		case TORSO_AIM_AND_FIRE_POST_FIRE:
		//---------------------------------------------------------------------
			stateResult = evaluateStatePostFire();

			if ( stateResult == BEHAVIOR_SUCCESS )
				{
					transitionToState( TORSO_AIM_AND_FIRE_SUCCESS );	
				}
		break;

		//---------------------------------------------------------------------
		case TORSO_AIM_AND_FIRE_SUCCESS:
		//---------------------------------------------------------------------
			_self->SetControllerAngles( ACTOR_TORSO_TAG, vec_zero );

			if ( _repeat )
				{
				Begin(self);
				return BEHAVIOR_EVALUATING;
				}

			return BEHAVIOR_SUCCESS;

		break;


		//---------------------------------------------------------------------
		case TORSO_AIM_AND_FIRE_FAILED:
		//---------------------------------------------------------------------
			if ( _repeat )
				{
				Begin(self);
				return BEHAVIOR_EVALUATING;
				}

			return BEHAVIOR_FAILED;

		break;


		}
	

	return BEHAVIOR_EVALUATING;   

}



//--------------------------------------------------------------
// Name:        End()
// Class:       TorsoAimAndFireWeapon
//
// Description: Cleans Up the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::End(Actor &self)
{ 
	if ( !_self )
		return;

	_fireWeapon.End(*_self);
	//gi.Printf( "TorsoAimAndFireWeapon::End()\n");
	_self->SetControllerAngles( ACTOR_TORSO_TAG, vec_zero );	
	_self->ClearTorsoAnim();
	//_self->SetAnim( _aimAnim , NULL , torso );
}



//--------------------------------------------------------------
// Name:		transitionToState()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Transitions the behaviors state
//
// Parameters:	coverCombatStates_t state -- The state to transition to
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::transitionToState( TorsoAimAndFireStates_t state )
{	
	switch ( state )
		{	
		case TORSO_AIM_AND_FIRE_AIM:
			setupStateAim();
			setInternalState( state , "TORSO_AIM_AND_FIRE_AIM" );
		break;

		case TORSO_AIM_AND_FIRE_PRE_FIRE:
			setupStatePreFire();
			setInternalState( state , "TORSO_AIM_AND_FIRE_PRE_FIRE" );
		break;

		case TORSO_AIM_AND_FIRE_ATTACK:
			setupStateAttack();
			setInternalState( state , "TORSO_AIM_AND_FIRE_ATTACK" );
		break;

		case TORSO_AIM_AND_FIRE_POST_FIRE:
			setupStatePostFire();
			setInternalState( state , "TORSO_AIM_AND_FIRE_POST_FIRE" );
		break;

		case TORSO_AIM_AND_FIRE_SUCCESS:
			setInternalState( state , "TORSO_AIM_AND_FIRE_SUCCESS" );
		break;

		case TORSO_AIM_AND_FIRE_FAILED:
			setInternalState( state , "TORSO_AIM_AND_FIRE_FAILED" );
		break;
		}
	
}


//--------------------------------------------------------------
// Name:		setInternalState()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Sets the internal state of the behavior
//
// Parameters:	unsigned int state
//				const str &stateName
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::setInternalState( TorsoAimAndFireStates_t state , const str &stateName )
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
void TorsoAimAndFireWeapon::init( Actor &self )
{
	_self = &self;

	//Set Our Controller Tag and set up our angles
	self.SetControllerTag( ACTOR_TORSO_TAG, gi.Tag_NumForName( self.edict->s.modelindex, "Bip01 Spine1" ) );
	_currentTorsoAngles = self.GetControllerAngles( ACTOR_TORSO_TAG );

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

	
	if ( gpm->hasProperty( objname , "AimTimeMin" ) )
		_aimTimeMin = gpm->getFloatValue( objname , "AimTimeMin" );

	if ( gpm->hasProperty( objname , "AimTimeMax" ) )
		_aimTimeMax = gpm->getFloatValue( objname , "AimTimeMax" );

	if ( gpm->hasProperty( objname , "FireTimeMin" ) )
		_fireTimeMin = gpm->getFloatValue( objname , "FireTimeMin" );

	if ( gpm->hasProperty( objname , "FireTimeMax" ) )
		_fireTimeMax = gpm->getFloatValue( objname , "FireTimeMax" );


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
// Class:		TorsoAimAndFireWeapon
//
// Description:	Does any processing required before evaluating states
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::think()
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

	/*
	if ( actTarget && ( actTarget->watch_offset != vec_zero ) )
		{
		MatrixTransformVector( actTarget->watch_offset, _watchTarget->orientation, watchPosition );
      watchPosition += _watchTarget->origin;
		}
	else
		{			
		tagNum = gi.Tag_NumForName( _watchTarget->edict->s.modelindex, "Bip01 Head" );
		
		if ( tagNum < 0 )
			watchPosition = _watchTarget->centroid;
		else
			{
			_watchTarget->GetTag( "Bip01 Head", &watchPosition );				
			}
		}
	*/
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
// Class:		TorsoAimAndFireWeapon
//
// Description:	Sets our _currentEnemy
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::updateEnemy()
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
			
			/*
			if ( player && GetSelf()->enemyManager->Hates(player) && _forceAttack )
				{
				currentEnemy = player;
				GetSelf()->enemyManager->SetCurrentEnemy( currentEnemy );
				}
			else
				{
				SetFailureReason( "TORSO_AIM_AND_FIRE_FAILED::updateEnemy -- No Enemy" );
				transitionToState( TORSO_AIM_AND_FIRE_FAILED );
				return;
				}
			*/
			}
			
		}

	_currentEnemy = currentEnemy;
	//_self->SetControllerAngles( ACTOR_TORSO_TAG, vec_zero );
	//gi.Printf( "TorsoAimAndFireWeapon::_turnTowardsEntity()\n");
	//_self->turnTowardsEntity( _currentEnemy, 0.0f );	

}

//--------------------------------------------------------------
// Name:		LerpTorsoBySpeed()
// Class:		TorsoAimAndFireWeapon
//
// Description: Lerps the torso
//
// Parameters:	const Vector &angleDelta
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::LerpTorsoBySpeed( const Vector &angleDelta )
   {
   Vector anglesDiff;
   Vector change;
   Vector finalAngles;
   Vector currentTorsoAngles;

   anglesDiff = angleDelta;

   //Reset our Controller Tag
   _self->SetControllerTag( ACTOR_TORSO_TAG, gi.Tag_NumForName( _self->edict->s.modelindex, "Bip01 Spine1" ) );


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
   
   _self->SetControllerAngles( ACTOR_TORSO_TAG, finalAngles );


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
void TorsoAimAndFireWeapon::AdjustTorsoAngles( const Vector &tagPos , const Vector &watchPosition )
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
// Class:		TorsoAimAndFireWeapon
//
// Description:	Sets up the Aim State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::setupStateAim()
{
	_animDone = false;

	if ( _aimTimeMax + _aimTimeMin > 0 )
		_endAimTime = level.time + G_Random(_aimTimeMax - _aimTimeMin ) + _aimTimeMin;
	else
		_endAimTime = -1.0;
	
	_self->SetAnim( _aimAnim , EV_Actor_NotifyTorsoBehavior , torso );	
}

//--------------------------------------------------------------
// Name:		evaluateStateAim()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Evaluates the Aim State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t TorsoAimAndFireWeapon::evaluateStateAim()
{
	bool tryToFire = false;

	if ( _endAimTime > 0 && level.time > _endAimTime )
		tryToFire = true;
		

	if ( _endAimTime < 0 && _animDone )
		tryToFire = true;

	if ( tryToFire )
		{
		if ( _self->combatSubsystem->CanAttackEnemy() || _forceAttack )
			return BEHAVIOR_SUCCESS;	
		
		if ( _endOnAimFail )
			return BEHAVIOR_FAILED;
		}
	
	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAim()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Failure Handler for the Failure State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::failureStateAim( const str& failureReason )
{
	SetFailureReason( failureReason );
}


//--------------------------------------------------------------
// Name:		setupStatePreFire()
// Class:		TorsoAimAndFireWeapon
//
// Description: Sets up the PreFire State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::setupStatePreFire()
{
	/*
	if ( _self->combatSubsystem->CanAttackEnemy() || _forceAttack )
		{
		_animDone = false;
		_self->SetAnim( _preFireAnim , EV_Actor_NotifyTorsoBehavior , torso );
		}
	else
		_canAttack = false;
	*/

	_animDone = false;
	_self->SetAnim( _preFireAnim , EV_Actor_NotifyTorsoBehavior , torso );


}

//--------------------------------------------------------------
// Name:		evaluateStatePreFire()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Evaluates the Pre Fire state
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t TorsoAimAndFireWeapon::evaluateStatePreFire()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	if ( !_canAttack )
		return BEHAVIOR_FAILED;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStatePreFire()
// Class:		TorsoAimAndFireWeapon
//
// Description: Failure Handler for the Pre Fire State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::failureStatePreFire( const str& failureReason )
{

}


//--------------------------------------------------------------
// Name:		 setupStateAttack()
// Class:		 TorsoAimAndFireWeapon
//
// Description:	 Setup Attack State
//
// Parameters:	 None
//
// Returns:		 None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::setupStateAttack()
{
	_animDone = false;

	if ( _fireTimeMax + _fireTimeMax > 0 )
		_endFireTime = level.time + G_Random(_fireTimeMax - _fireTimeMax) + _fireTimeMin;
	else
		_endFireTime = -1.0;


	if ( _self->combatSubsystem->CanAttackEnemy() || _forceAttack )
		{	
		_fireWeapon.SetTarget(_currentEnemy);
		_fireWeapon.SetAnim( _fireAnim ); 
		_fireWeapon.Begin( *_self );
		}
	else
		{
		_fireWeapon.End( *_self );
		_fireFailed = true;
		}

}

//--------------------------------------------------------------
// Name:		evaluateStateAttack()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Evaluates Attack State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t TorsoAimAndFireWeapon::evaluateStateAttack()
{
	BehaviorReturnCode_t result;
		
	/*if ( _self->combatSubsystem->CanAttackEnemy() || _forceAttack )
		result = _fireWeapon.Evaluate( *_self );
	else
		{
		_fireWeapon.End( *_self );
		result = BEHAVIOR_FAILED;
		}
	

	//_fireWeapon.End( *_self );

	if ( result == BEHAVIOR_FAILED )
		{
		failureStateAttack( "StationaryFireCombat::evaluateStateAttack -- FAILED" );
		return result;
		}
	*/
	/*
	if ( _endFireTime > 0 && level.time > _endFireTime )
		return BEHAVIOR_SUCCESS;

	if ( _endFireTime < 0 && _animDone )
		return BEHAVIOR_SUCCESS;
	*/

	if ( _fireFailed )
		return BEHAVIOR_FAILED;

	result = _fireWeapon.Evaluate( *_self );
	if ( _self->shotsFiredThisVolley >= _shots )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;
}

//--------------------------------------------------------------
// Name:		failureStateAttack()
// Class:		TorsoAimAndFireWeapon
//
// Description: Failure Handler for Attack State
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::failureStateAttack( const str& failureReason )
{
	SetFailureReason( failureReason );
	_fireWeapon.End( *_self );
}

//--------------------------------------------------------------
// Name:		setupStatePostFire()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Sets up Post Fire State
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::setupStatePostFire()
{
	_animDone = false;
	//gi.WDPrintf( "-----------------------------------------\n");
	//gi.WDPrintf( "setupStatePostFire\n" );
	//gi.WDPrintf( "-----------------------------------------\n");

	_self->SetAnim( _postFireAnim , EV_Actor_NotifyTorsoBehavior , torso );
}

//--------------------------------------------------------------
// Name:		evaluateStatePostFire()
// Class:		TorsoAimAndFireWeapon
//
// Description:	Evaluates State Post Fire
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
BehaviorReturnCode_t TorsoAimAndFireWeapon::evaluateStatePostFire()
{
	if ( _animDone )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_FAILED;
}

//--------------------------------------------------------------
// Name:		failureStatePostFire()
// Class:		TorsoAimAndFireWeapon
//
// Description: Failure State Post Fire
//
// Parameters:	const str& failureReason
//
// Returns:		None
//--------------------------------------------------------------
void TorsoAimAndFireWeapon::failureStatePostFire( const str& failureReason )
{

}
