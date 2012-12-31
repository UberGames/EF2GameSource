//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/doAttack.cpp                               $
// $Revision:: 11                                                             $
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
// DoAttack Behavior Implementation
//	-- DoAttack will rotate the actor to face his current enemy, after the
//	   the rotation( with optional animation ) has completed, it will check
//	   if the actor is able to hit its current enemy with an attack.  If that
//	   check is successful, then it will play its specified attack animation.
//
//	   Currently, you must put the attack commands inside the animation.
//
//	ANIMATIONS:
//		Attack Animation   : Parameter
//		Rotation Animation : Optional 
//--------------------------------------------------------------------------------

#include "actor.h"
#include "doAttack.hpp"
#include <qcommon/gameplaymanager.h>

//--------------------------------------------------------------
//
// Class Declaration and Event Registration
//
//--------------------------------------------------------------
CLASS_DECLARATION( Behavior, DoAttack, NULL )
	{
		{ &EV_Behavior_Args,		&DoAttack::SetArgs	},
		{ &EV_Behavior_AnimDone,	&DoAttack::AnimDone	}, 
		{ NULL, NULL }
	};



//--------------------------------------------------------------
//
// Name:        SetArgs()
// Class:       DoAttack
//
// Description: 
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
//--------------------------------------------------------------
void DoAttack::SetArgs( Event *ev )
{
	_anim = ev->GetString( 1 );
	if ( ev->NumArgs() > 1 )
		_turnspeed = ev->GetFloat( 2 );
   else
		_turnspeed = 30.0f;

   if ( ev->NumArgs() > 2 )
		_forceAttack = ev->GetBoolean( 3 );
   else
		_forceAttack = false;

   if ( ev->NumArgs() > 3 )
		_rotateAnim = ev->GetString( 4 );
   else
		_rotateAnim = "";
}



//--------------------------------------------------------------
// Name:        AnimDone()
// Class:       DoAttack
//
// Description: AnimDone Event Handler
//
// Parameters:  Event *ev -- The AnimDone event
//
// Returns:     None
//--------------------------------------------------------------
void DoAttack::AnimDone( Event * )
{
	_state = ATTACK_STATE_COMPLETE;
}



//--------------------------------------------------------------
//
// Name:        Begin()
// Class:       DoAttack
//
// Description: Initializes the behavior
//
// Parameters:  Actor &self -- The actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void DoAttack::Begin( Actor &self )
{       	
	init ( self );
	_setupRotate( self );
	_state = ATTACK_STATE_ROTATE;   
}



//--------------------------------------------------------------
//
// Name:        Evaluate()
// Class:       DoAttack
//
// Description: Update for this behavior -- called every server frame
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     BehaviorReturnCode_t
//
//--------------------------------------------------------------
BehaviorReturnCode_t	DoAttack::Evaluate( Actor &self )
{
	if ( _state != ATTACK_STATE_ANIMATING )
		_rotate( self );

	if ( _state == ATTACK_STATE_START_ANIM )
		{
		_playAttackAnim( self );
		_state = ATTACK_STATE_ANIMATING;
		}

	if ( _state == ATTACK_STATE_FAILED )
		return BEHAVIOR_FAILED;

	if ( _state == ATTACK_STATE_COMPLETE )
		return BEHAVIOR_SUCCESS;

	return BEHAVIOR_EVALUATING;   
}



//--------------------------------------------------------------
//
// Name:        End()
// Class:       DoAttack
//
// Description: Ends this behavior -- cleans things up
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//
//--------------------------------------------------------------
void DoAttack::End(Actor &self)
{ 
	_rotateBehavior.End(self);
}




//--------------------------------------------------------------
// Name:        _setupRotate()
// Class:       DoAttack
//
// Description: Sets up the Rotate Component Behavior
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void DoAttack::_setupRotate( Actor &self )
{
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();
	
	if ( currentEnemy )
		{
		 _rotateBehavior.SetEntity( currentEnemy );
		_rotateBehavior.SetTurnSpeed( _turnspeed ); 
		if ( _rotateAnim.length() )
			_rotateBehavior.SetAnim( _rotateAnim );

		_rotateBehavior.Begin( self );	 
		}         
}



//--------------------------------------------------------------
// Name:        _rotate()
// Class:       DoAttack
//
// Description: Evaluates the Rotate Component Behavior
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void DoAttack::_rotate( Actor &self )
{  
	BehaviorReturnCode_t result;

	result = _rotateBehavior.Evaluate( self );

	// If we get a failure from rotate, chances are it didn't have
	// an entity to rotate to... Let's give it one more shot.
	if ( result == BEHAVIOR_FAILED )
		{
		_setupRotate( self );
		result = _rotateBehavior.Evaluate( self );

		// If we have a failure again, then we DO have a serious
		// problem
		if ( result == BEHAVIOR_FAILED )
			{
			str failureReason;
			failureReason = "Rotate Component Failed: Returned -- ";
			failureReason += _rotateBehavior.GetFailureReason();
			SetFailureReason( failureReason );
			_state = ATTACK_STATE_FAILED;
			}

		}

	if ( result == BEHAVIOR_SUCCESS )
		{
		if ( _canAttack( self ) )
			{
			if ( _state != ATTACK_STATE_ANIMATING && _state != ATTACK_STATE_COMPLETE )
				{
				_state = ATTACK_STATE_START_ANIM;
				return;
				}		
			}
		
	else
		_state = ATTACK_STATE_FAILED;
		
	}

}



//--------------------------------------------------------------
// Name:        _playAttackAnim()
// Class:       DoAttack
//
// Description: Plays the specified attack animation
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
void DoAttack::_playAttackAnim( Actor &self )
{
	self.ClearTorsoAnim();
	self.SetAnim( _anim , EV_Actor_NotifyBehavior);
	_state = ATTACK_STATE_ANIMATING;
}



//--------------------------------------------------------------
// Name:        _canAttack()
// Class:       DoAttack
//
// Description: Checks if the actor can attack
//
// Parameters:  Actor &self -- Actor executing this behavior
//
// Returns:     None
//--------------------------------------------------------------
bool DoAttack::_canAttack( Actor &self )
	{  
	Entity *currentEnemy;
	currentEnemy = self.enemyManager->GetCurrentEnemy();

	if ( _forceAttack )
		return true;

	if ( !currentEnemy)
		return false;
   
	if ( self.combatSubsystem->CanAttackTarget( currentEnemy ) )
		return true;
	else
		return false;
	}

void DoAttack::init( Actor &self )
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(self.getArchetype()) )
		return;

	if ( !self.combatSubsystem->HaveWeapon() )
		return;

	str objname = self.combatSubsystem->GetActiveWeaponArchetype();	
	objname = "Hold" + objname;

	if ( gpm->hasProperty(objname, "Melee") )
		_anim = gpm->getStringValue( objname , "Melee" );
	else
		{
		str weaponName = self.combatSubsystem->GetActiveWeaponName();
	
		if ( !weaponName.length() )
			{
			weaponName = "UNSPECIFIED";
			}

		gi.WDPrintf( "\n\n\nDoAttack: TargetName %s ( Entnum %i ) has a weapon, but does not have a MeleeProperty ( Trying Weapon: %s ) in the GPD,  Please notify a programmer of this error\n\n\n" , self.TargetName() , self.entnum, weaponName.c_str() );
		//gi.Error( ERR_DROP , "\n\n\nDoAttack: TargetName %s ( Entnum %i ) has a weapon, but does not have a MeleeProperty ( Trying Weapon: %s ) in the GPD,  Please notify a programmer of this error\n\n\n" , self.TargetName() , self.entnum, weaponName.c_str() );
		}
	
}
