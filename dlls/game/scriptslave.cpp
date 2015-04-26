//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/scriptslave.cpp                           $
// $Revision:: 64                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//

//
// DESCRIPTION:
// Standard scripted objects.  Controlled by ScriptThread.  These objects
// are bmodel objects created in the editor and controlled by an external
// text based script.  Commands are interpretted on by one and executed
// upon a signal from the script master.  The base script object can
// perform several different relative and specific rotations and translations
// and can cause other parts of the script to be executed when touched, damaged,
// touched, or used.
//

#include "_pch_cpp.h"
#include "class.h"
#include "mover.h"
#include "scriptmaster.h"
#include "scriptslave.h"
#include "sentient.h"
#include "item.h"
#include "gibs.h"
#include "explosion.h"
#include "equipment.h"
#include <qcommon/gameplaymanager.h>

/*****************************************************************************/
/*QUAKED script_object (0 0.5 1) ? NOT_SOLID

******************************************************************************/

Event EV_ScriptSlave_DoMove
(
	"processCommands",
	EV_CODEONLY,
	NULL,
	NULL,
	"Move the script slave."
);
Event EV_ScriptSlave_NewOrders
(
	"newOrders",
	EV_CODEONLY,
	NULL,
	NULL,
	"Inform script that it is about to get new orders."
);
Event EV_ScriptSlave_Angles
(
	"angles",
	EV_SCRIPTONLY,
	"v",
	"angles",
	"Sets the angles."
);
Event EV_ScriptSlave_Trigger
( 
	"trigger",
	EV_SCRIPTONLY,
	"s",
	"entname",
	"Trigger entities target."
);
Event EV_ScriptSlave_Next
(
	"next",
	EV_DEFAULT,
	NULL,
	NULL,
	"Goto the next waypoint."
);
Event EV_ScriptSlave_JumpTo
(
	"jumpto",
	EV_SCRIPTONLY,
	"sFF",
	"vector_or_entity token token",
	"Jump to specified vector or entity."
);
Event EV_ScriptSlave_MoveTo
(
	"moveto",
	EV_SCRIPTONLY,
	"e",
	"entity_to_move_to",
	"Move to the specified entity."
);
Event EV_ScriptSlave_MoveToPosition
(
	"movetopos",
	EV_SCRIPTONLY,
	"v",
	"position",
	"Move to the specified position."
);
Event EV_ScriptSlave_Speed
(
	"speed",
	EV_DEFAULT,
	"f",
	"speed",
	"Sets the speed."
);
Event EV_ScriptSlave_Time
(
	"time",
	EV_SCRIPTONLY,
	"f",
	"travel_time",
	"Sets the travel time."
);
Event EV_ScriptSlave_MoveUp
(
	"moveUp",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position up."
);
Event EV_ScriptSlave_MoveDown
(
	"moveDown",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position down."
);
Event EV_ScriptSlave_MoveNorth
(
	"moveNorth",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position north."
);
Event EV_ScriptSlave_MoveSouth
(
	"moveSouth",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position south."
);
Event EV_ScriptSlave_MoveEast
(
	"moveEast",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position east."
);
Event EV_ScriptSlave_MoveWest
(
	"moveWest",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position west."
);
Event EV_ScriptSlave_MoveForward
(
	"moveForward",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position forward."
);
Event EV_ScriptSlave_MoveBackward
(
	"moveBackward",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position backward."
);
Event EV_ScriptSlave_MoveLeft
(
	"moveLeft",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position left."
);
Event EV_ScriptSlave_MoveRight
(
	"moveRight",
	EV_SCRIPTONLY,
	"f",
	"dist",
	"Move the position right."
);
Event EV_ScriptSlave_RotateXDownTo
(
	"rotateXdownto",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the x down to angle."
);
Event EV_ScriptSlave_RotateYDownTo
(
	"rotateYdownto",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the y down to angle."
);
Event EV_ScriptSlave_RotateZDownTo
(
	"rotateZdownto",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the z down to angle."
);
Event EV_ScriptSlave_RotateAxisDownTo
(
	"rotateaxisdownto",
	EV_SCRIPTONLY,
	"if",
	"axis angle",
	"Rotate the specified axis down to angle."
);
Event EV_ScriptSlave_RotateXUpTo
(
	"rotateXupto",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the x up to angle."
);
Event EV_ScriptSlave_RotateYUpTo
(
	"rotateYupto",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the y up to angle."
);
Event EV_ScriptSlave_RotateZUpTo
(
	"rotateZupto",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the z up to angle."
);
Event EV_ScriptSlave_RotateAxisUpTo
(
	"rotateaxisupto",
	EV_SCRIPTONLY,
	"if",
	"axis angle",
	"Rotate the specified axis up to angle."
);
Event EV_ScriptSlave_RotateXDown
(
	"rotateXdown",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the x down by the specified amount."
);
Event EV_ScriptSlave_RotateYDown
(
	"rotateYdown",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the y down by the specified amount."
);
Event EV_ScriptSlave_RotateZDown
(
	"rotateZdown",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the z down by the specified amount."
);
Event EV_ScriptSlave_RotateAxisDown
(
	"rotateaxisdown",
	EV_SCRIPTONLY,
	"if",
	"axis angle",
	"Rotate the specified axis down by the specified amount."
);
Event EV_ScriptSlave_RotateXUp
(
	"rotateXup",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the x up by the specified amount."
);
Event EV_ScriptSlave_RotateYUp
(
	"rotateYup",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the y up by the specified amount."
);
Event EV_ScriptSlave_RotateZUp
(
	"rotateZup",
	EV_SCRIPTONLY,
	"f",
	"angle",
	"Rotate the z up by the specified amount."
);
Event EV_ScriptSlave_RotateAxisUp
(
	"rotateaxisup",
	EV_SCRIPTONLY,
	"if",
	"axis angle",
	"Rotate the specified axis up by the specified amount."
);
Event EV_ScriptSlave_RotateX
(
	"rotateX",
	EV_SCRIPTONLY,
	"f",
	"avelocity",
	"Rotate about the x axis at the specified angular velocity."
);
Event EV_ScriptSlave_RotateY
(
	"rotateY",
	EV_SCRIPTONLY,
	"f",
	"avelocity",
	"Rotate about the y axis at the specified angular velocity."
);
Event EV_ScriptSlave_RotateZ
(
	"rotateZ",
	EV_SCRIPTONLY,
	"f",
	"avelocity",
	"Rotate about the z axis at the specified angular velocity."
);
Event EV_ScriptSlave_RotateAxis
(
	"rotateaxis",
	EV_SCRIPTONLY,
	"if",
	"axis avelocity",
	"Rotate about the specified axis at the specified angular velocity."
);
Event EV_ScriptSlave_RotateDownTo
(
	"rotatedownto",
	EV_SCRIPTONLY,
	"v",
	"direction",
	"Rotate down to the specified direction."
);
Event EV_ScriptSlave_RotateUpTo
(
	"rotateupto",
	EV_SCRIPTONLY,
	"v",
	"direction",
	"Rotate up to the specified direction."
);
Event EV_ScriptSlave_RotateTo
(
	"rotateto",
	EV_SCRIPTONLY,
	"v",
	"direction",
	"Rotate to the specified direction."
);
Event EV_ScriptSlave_OnTouch
(
	"ontouch",
	EV_SCRIPTONLY,
	"s",
	"label",
	"Sets what label to jump to and process script at when touched."
);
Event EV_ScriptSlave_NoTouch
(
	"notouch",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Removes the ontouch thread."
);
Event EV_ScriptSlave_OnUse
(
	"onuse",
	EV_SCRIPTONLY,
	"s",
	"label",
	"Sets what label to jump to and process script at when used."
);
Event EV_ScriptSlave_NoUse
(
	"nouse",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Removes the onuse thread."
);
Event EV_ScriptSlave_OnBlock
(
	"onblock",
	EV_SCRIPTONLY,
	"s",
	"label",
	"Sets what label to jump to and process script at when blocked."
);
Event EV_ScriptSlave_NoBlock
(
	"noblock",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Removes the onblock thread."
);
Event EV_ScriptSlave_OnTrigger
(
	"ontrigger",
	EV_SCRIPTONLY,
	"s",
	"label",
	"Sets what label to jump to and process script at when triggered."
);
Event EV_ScriptSlave_NoTrigger
(
	"notrigger",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Removes the ontrigger thread."
);
Event EV_ScriptSlave_OnDamage
(
	"ondamage",
	EV_SCRIPTONLY,
	"s",
	"label",
	"Sets what label to jump to and process script at when damaged."
);
Event EV_ScriptSlave_NoDamage
(
	"nodamage",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes the ondamage thread."
);
Event EV_ScriptSlave_SetMeansOfDeath
(
	"setmeansofdeath",
	EV_SCRIPTONLY,
	"s",
	"means_of_death",
	"Set the damage means of death."
);
Event EV_ScriptSlave_SetDamageSpawn
(
	"dmg",
	EV_SCRIPTONLY,
	"f",
	"damage",
	"Set the damage."
);
Event EV_ScriptSlave_FollowPath
(
	"followpath",
	EV_SCRIPTONLY,
	"eSSSSSS",
	"path arg1 arg2 arg3 arg4 arg5 arg6",
	"Makes the script slave follow the specified path.  The allowable arguments are ignoreangles,\n"
	"normalangles, loop, and a number specifying the start time."
);
Event EV_ScriptSlave_EndPath
(
	"endpath",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Stop following the path"
);
Event EV_ScriptSlave_MoveDone
(
	"scriptslave_movedone",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the script slave is doen moving"
);
Event EV_ScriptSlave_FollowingPath
(
	"scriptslave_followingpath",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called every frame to actually follow the path"
);
Event EV_ScriptSlave_Explode
(
	"scriptSlave_explode",
	EV_SCRIPTONLY,
	"f",
	"damage",
	"Creates an explosion at the script slave's position"
);
Event EV_ScriptSlave_NotShootable
(
	"notshootable",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Makes the script slave not shootable"
);
Event EV_ScriptSlave_OpenAreaPortal
(
	"openportal",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Open the area portal enclosed in this object"
);
Event EV_ScriptSlave_CloseAreaPortal
(
	"closeportal",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Close the area portal enclosed in this object"
);
Event EV_ScriptSlave_PhysicsOn
(
	"physics_on",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Turn physics on this script object on"
);
Event EV_ScriptSlave_PhysicsOff
(
	"physics_off",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Turn physics off this script object on"
);
Event EV_ScriptSlave_PhysicsVelocity
(
	"physics_velocity",
	EV_SCRIPTONLY,
	"v",
	"impulseVector",
	"Add a physical impulse to an object when it is being physically simulated"
);
Event EV_ScriptSlave_StopEvent
(
	"stopspline",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"stops an scriptobject from moving on a spline"
);
Event EV_ScriptSlave_ContinueEvent
(
	"continuespline",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"makes a script object continue on a spline"
);

// 1ST PLAYABLE HACK
Event EV_ScriptSlave_Hack_SetTriggerParms
(
	"setobjectparms",
	EV_DEFAULT,
	"ff",
	"force_field_number trigger_number",
	"HACK HACK HACK HACK HACK HACK HACK"
);
Event EV_ScriptSlave_Hack_GetForceFieldNumber
(
	"getforcefieldnumber",
	EV_DEFAULT,
	"@f",
	"number",
	"HACK HACK HACK HACK HACK HACK HACK"
);
Event EV_ScriptSlave_Hack_GetTriggerNumber
(
	"gettriggernumber",
	EV_DEFAULT,
	"@f",
	"number",
	"HACK HACK HACK HACK HACK HACK HACK"
);
Event EV_ScriptSlave_Hack_GetScanner
(
	"getscanner",
	EV_DEFAULT,
	"@e",
	"scanner",
	"HACK HACK HACK HACK HACK HACK HACK"
);

Event EV_ScriptSlave_HandlesDamage
(
	"handlesdamage",
	EV_DEFAULT,
	"b",
	"damage_flag",
	"sets the handlesdamage flag on the script slave"
);
Event EV_ScriptSlave_DamageEffect
(
	"damageEffect",
	EV_DEFAULT,
	"s",
	"damageEffectModel",
	"Sets the damage effect model name."
);

Event EV_ScriptSlave_BloodModel
(
	"setBloodModel",
	EV_DEFAULT,
	"s",
	"bloodmodel",
	"Sets the blood model"
);

Event EV_ScriptSlave_AddRequiredDamageMOD
(
	"addrequireddamagemod",
	EV_DEFAULT,
	"s",
	"MOD_String",
	"Adds the required MOD for damage to be applied"
);

Event EV_ScriptSlave_SetCanBeAttackedByOtherScriptObjects
(
	"allowAttackFromOtherScriptObjects",
	EV_DEFAULT,
	"b",
	"allow_flag",
	"Sets the _canBeAttackedByOtherScriptSlaves variable"
);

CLASS_DECLARATION( Trigger, ScriptSlave, "script_object" )
{
	{ &EV_Bind,									&ScriptSlave::BindEvent },
	{ &EV_Unbind,								&ScriptSlave::EventUnbind },
	{ &EV_ScriptSlave_DoMove,					&ScriptSlave::DoMove },
	{ &EV_ScriptSlave_NewOrders,				&ScriptSlave::NewOrders },
	{ &EV_ScriptSlave_Angles,					&ScriptSlave::SetAnglesEvent },
	{ &EV_SetAngle,								&ScriptSlave::SetAngleEvent },
	{ &EV_Model,								&ScriptSlave::SetModelEvent },
	{ &EV_ScriptSlave_Trigger,					&ScriptSlave::TriggerEvent },
	{ &EV_ScriptSlave_Next,						&ScriptSlave::GotoNextWaypoint },
	{ &EV_ScriptSlave_JumpTo,					&ScriptSlave::JumpTo },
	{ &EV_ScriptSlave_MoveTo,					&ScriptSlave::MoveToEvent },
	{ &EV_ScriptSlave_MoveToPosition,			&ScriptSlave::MoveToPositionEvent },
	{ &EV_ScriptSlave_Speed,					&ScriptSlave::SetSpeed },
	{ &EV_ScriptSlave_Time,						&ScriptSlave::SetTime },
	{ &EV_ScriptSlave_MoveUp,					&ScriptSlave::MoveUp },
	{ &EV_ScriptSlave_MoveDown,					&ScriptSlave::MoveDown },
	{ &EV_ScriptSlave_MoveNorth,				&ScriptSlave::MoveNorth },
	{ &EV_ScriptSlave_MoveSouth,				&ScriptSlave::MoveSouth },
	{ &EV_ScriptSlave_MoveEast,					&ScriptSlave::MoveEast },
	{ &EV_ScriptSlave_MoveWest,					&ScriptSlave::MoveWest },
	{ &EV_ScriptSlave_MoveForward,				&ScriptSlave::MoveForward },
	{ &EV_ScriptSlave_MoveBackward,				&ScriptSlave::MoveBackward },
	{ &EV_ScriptSlave_MoveLeft,					&ScriptSlave::MoveLeft },
	{ &EV_ScriptSlave_MoveRight,				&ScriptSlave::MoveRight },
	{ &EV_ScriptSlave_RotateXDownTo,			&ScriptSlave::RotateXdownto },
	{ &EV_ScriptSlave_RotateYDownTo,			&ScriptSlave::RotateYdownto },
	{ &EV_ScriptSlave_RotateZDownTo,			&ScriptSlave::RotateZdownto },
	{ &EV_ScriptSlave_RotateXUpTo,				&ScriptSlave::RotateXupto },
	{ &EV_ScriptSlave_RotateYUpTo,				&ScriptSlave::RotateYupto },
	{ &EV_ScriptSlave_RotateZUpTo,				&ScriptSlave::RotateZupto },
	{ &EV_ScriptSlave_RotateXDown,				&ScriptSlave::RotateXdown },
	{ &EV_ScriptSlave_RotateYDown,				&ScriptSlave::RotateYdown },
	{ &EV_ScriptSlave_RotateZDown,				&ScriptSlave::RotateZdown },
	{ &EV_ScriptSlave_RotateXUp,				&ScriptSlave::RotateXup },
	{ &EV_ScriptSlave_RotateYUp,				&ScriptSlave::RotateYup },
	{ &EV_ScriptSlave_RotateZUp,				&ScriptSlave::RotateZup },
	{ &EV_ScriptSlave_RotateX,					&ScriptSlave::RotateX },
	{ &EV_ScriptSlave_RotateY,					&ScriptSlave::RotateY },
	{ &EV_ScriptSlave_RotateZ,					&ScriptSlave::RotateZ },
	{ &EV_ScriptSlave_RotateAxisDownTo,			&ScriptSlave::RotateAxisdownto },
	{ &EV_ScriptSlave_RotateAxisUpTo,			&ScriptSlave::RotateAxisupto },
	{ &EV_ScriptSlave_RotateAxisDown,			&ScriptSlave::RotateAxisdown },
	{ &EV_ScriptSlave_RotateAxisUp,				&ScriptSlave::RotateAxisup },
	{ &EV_ScriptSlave_RotateAxis,				&ScriptSlave::RotateZ },
	{ &EV_ScriptSlave_OnTouch,					&ScriptSlave::OnTouch },
	{ &EV_ScriptSlave_NoTouch,					&ScriptSlave::NoTouch },
	{ &EV_ScriptSlave_OnUse,					&ScriptSlave::OnUse },
	{ &EV_ScriptSlave_NoUse,					&ScriptSlave::NoUse },
	{ &EV_ScriptSlave_OnBlock,					&ScriptSlave::OnBlock },
	{ &EV_ScriptSlave_NoBlock,					&ScriptSlave::NoBlock },
	{ &EV_ScriptSlave_OnTrigger,				&ScriptSlave::OnTrigger },
	{ &EV_ScriptSlave_NoTrigger,				&ScriptSlave::NoTrigger },
	{ &EV_ScriptSlave_OnDamage,					&ScriptSlave::OnDamage },
	{ &EV_ScriptSlave_NoDamage,					&ScriptSlave::NoDamage },
	{ &EV_ScriptSlave_SetMeansOfDeath,			&ScriptSlave::SetMeansOfDeath },
	{ &EV_ScriptSlave_SetDamageSpawn,			&ScriptSlave::SetDamage },
	{ &EV_ScriptSlave_FollowPath,				&ScriptSlave::FollowPath },
	{ &EV_ScriptSlave_EndPath,					&ScriptSlave::EndPath },
	{ &EV_ScriptSlave_FollowingPath,			&ScriptSlave::FollowingPath },
	{ &EV_Touch,								&ScriptSlave::TouchFunc },
	{ &EV_Blocked,								&ScriptSlave::BlockFunc },
	{ &EV_Activate,								&ScriptSlave::TriggerFunc },
	{ &EV_Use,									&ScriptSlave::UseFunc },
	{ &EV_ScriptSlave_MoveDone,					&ScriptSlave::MoveEnd },
	{ &EV_Damage,								&ScriptSlave::DamageFunc },
	{ &EV_ScriptSlave_RotateDownTo,				&ScriptSlave::Rotatedownto },
	{ &EV_ScriptSlave_RotateUpTo,				&ScriptSlave::Rotateupto },
	{ &EV_ScriptSlave_RotateTo,					&ScriptSlave::Rotateto },
	{ &EV_ScriptSlave_Explode,					&ScriptSlave::Explode },
	{ &EV_ScriptSlave_NotShootable,				&ScriptSlave::NotShootable },
	{ &EV_ScriptSlave_OpenAreaPortal,			&ScriptSlave::OpenPortal },
	{ &EV_ScriptSlave_CloseAreaPortal,			&ScriptSlave::ClosePortal },
	{ &EV_ScriptSlave_PhysicsOn,				&ScriptSlave::PhysicsOn },
	{ &EV_ScriptSlave_PhysicsOff,				&ScriptSlave::PhysicsOff },
	{ &EV_ScriptSlave_PhysicsVelocity,			&ScriptSlave::PhysicsVelocity },
	{ &EV_ScriptSlave_StopEvent,				&ScriptSlave::StopEvent },
	{ &EV_ScriptSlave_ContinueEvent,			&ScriptSlave::ContinueEvent },
	{ &EV_SetGameplayDamage,					&ScriptSlave::setDamage },
	{ &EV_ScriptSlave_HandlesDamage,			&ScriptSlave::HandlesDamage },
	{ &EV_ScriptSlave_DamageEffect,				&ScriptSlave::setDamageEffect },
	{ &EV_ScriptSlave_BloodModel,				&ScriptSlave::setBloodModel },
	{ &EV_ScriptSlave_AddRequiredDamageMOD,		&ScriptSlave::addRequiredDamageMOD },
	{ &EV_ScriptSlave_SetCanBeAttackedByOtherScriptObjects, &ScriptSlave::setCanBeAttackedByOtherSlaves },

	//1ST PLAYABLE HACK STUFF
	{ &EV_ScriptSlave_Hack_SetTriggerParms,     &ScriptSlave::Hack_AddParms             },
	{ &EV_ScriptSlave_Hack_GetForceFieldNumber, &ScriptSlave::Hack_GetForceFieldNumber  },
	{ &EV_ScriptSlave_Hack_GetTriggerNumber,    &ScriptSlave::Hack_GetTriggerNumber     },
	{ &EV_ScriptSlave_Hack_GetScanner,          &ScriptSlave::Hack_GetScanner           },

	{ NULL, NULL }
};

ScriptSlave::ScriptSlave()
{
	mover = new Mover( this );
	
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	// this is a normal entity
	edict->s.eType = ET_GENERAL;
	
	speed             = 100;
	takedamage			= DamageYes;
	waypoint				= NULL;
	SetNewAngles( localangles );
	SetNewPosition( GetLocalOrigin() );
	traveltime			= 0;
	commandswaiting	= false;
	movethread			= NULL;
	touchthread			= NULL;
	blockthread			= NULL;
	damagethread		= NULL;
	triggerthread		= NULL;
	usethread			= NULL;
	splinePath        = NULL;
	splineangles      = false;
	attack_finished   = 0;
	thinking          = true;
	
	dmg					 = 2;
	dmg_means_of_death = MOD_CRUSH;
	
	setMoveType( MOVETYPE_PUSH );
	setSolidType( SOLID_NOT );
	
	//1ST PLAYABLE HACK
	_forcefieldNumber  = -1.0f;
	_triggerNumber     = -1.0f;
	_scanner = NULL;
	
	_portalOpen = false;
	_handlesDamage = false;

	_nextNeedToUseTime = 0.0f;
	_canBeAttackedByOtherScriptSlaves = true;
	
	if ( spawnflags & 1 )
	{
		PostEvent( EV_BecomeNonSolid, EV_POSTSPAWN );
	}
}

ScriptSlave::~ScriptSlave()
{
	if ( splinePath )
	{
		delete splinePath;
		splinePath = NULL;
	}
}

//--------------------------------------------------------------
//
// Name:			setDamage
// Class:			ScriptSlave
//
// Description:		This function acts as a filter to the real function.
//					It gets data from the database, and then passes it
//					along to the original event.  This is here as an attempt
//					to sway people into using the database standard instead of
//					hardcoded numbers.
//
// Parameters:		Event *ev
//						str -- The value keyword from the database (low, medium, high, etc).
//
// Returns:			None
//
//--------------------------------------------------------------
void ScriptSlave::setDamage( Event *ev )
{
	if ( ev->NumArgs() < 1 )
		return;

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();	
	if ( !gpm->hasFormula("OffensiveDamage") )
		return;

	str damagestr = ev->GetString( 1 );
	float damagemod = 1.0f;
	if ( gpm->getDefine(damagestr) != "" )
		damagemod = (float)atof(gpm->getDefine(damagestr));
	GameplayFormulaData fd(this, 0, 0, "");
	float finaldamage = gpm->calculate("OffensiveDamage", fd, damagemod);
	Event *newev = new Event(EV_ScriptSlave_SetDamageSpawn);
	newev->AddFloat(finaldamage);
	ProcessEvent(newev);
}

void ScriptSlave::setOrigin( const Vector &point )
{
	Trigger::setOrigin( point );
	SetNewPosition( GetLocalOrigin() );
}

void ScriptSlave::setOrigin( void )
{
	Trigger::setOrigin();
	SetNewPosition( GetLocalOrigin() );
}

void ScriptSlave::NewOrders( Event * )
{
	// make sure position and angles are current
	SetNewAngles( localangles );
	SetNewPosition( GetLocalOrigin() );
}

void ScriptSlave::BindEvent( Event *ev )
{
	Entity *ent;
	
	ent = ev->GetEntity( 1 );
	if ( ent )
	{
		bind( ent );
	}
	
	// make sure position and angles are current
	SetNewAngles( localangles );
	SetNewPosition( GetLocalOrigin() );
}

void ScriptSlave::EventUnbind( Event * )
{
	unbind();
	
	// make sure position and angles are current
	SetNewAngles( localangles );
	SetNewPosition( GetLocalOrigin() );
}

void ScriptSlave::DoMove( Event *ev )
{
	float dist;
	CThread *thread;
	Event *event;
	
	thread = ev->GetThread();
	assert( thread );
	if ( thread && thread->WaitingFor( this ) )
	{
		if ( movethread && ( movethread != thread ) )
		{
			// warn the user
			ev->Error( "Overriding previous move commands for '%s'\n", TargetName() );
			
			// Yeah, we're not REALLY done, but we tell our old thread
			// that we are so that it doesn't wait forever
			event = new Event( EV_MoveDone );
			event->AddEntity( this );
			movethread->ProcessEvent( event );
		}
		
		movethread = thread;
	}
	if ( commandswaiting )
	{
		if ( splinePath )
		{
			moving = true;
			PostEvent( EV_ScriptSlave_FollowingPath, 0.0f );
		}
		else
		{
			float t = traveltime;
			if ( t == 0.0f )
			{
				dist = Vector( GetNewPosition() - GetLocalOrigin() ).length();
				t = dist / speed;
			}
			moving = true;
			mover->LinearInterpolate( GetNewPosition(), GetNewAngles(), t, EV_ScriptSlave_MoveDone );
		}
		commandswaiting = false;
	}
	else if ( movethread && ( movethread == thread ) && !moving )
	{
		// No commands, so tell the master that we're done
		PostEvent( EV_ScriptSlave_MoveDone, 0.0f );
	}
}



void ScriptSlave::MoveEnd( Event * )
{
	Event *event;
	
	moving = false;
	commandswaiting = false;
	SetNewAngles( localangles );
	SetNewPosition( GetLocalOrigin() );
	
	if ( movethread )
	{
		event = new Event( EV_MoveDone );
		event->AddEntity( this );
		movethread->ProcessEvent( event );
		movethread = NULL;
	}
}

void ScriptSlave::SetAnglesEvent( Event *ev )
{
	commandswaiting = true;
	setAngles( ev->GetVector( 1 ) );
	SetNewAngles( localangles );
}

void ScriptSlave::SetAngleEvent( Event *ev )
{
	float angle;
	
	angle = ev->GetFloat( 1 );
	if ( angle == -1.0f )
	{
		ForwardDir = Vector( 0.0f, 0.0f, 90.0f );
	}
	else if ( angle == -2.0f )
	{
		ForwardDir = Vector( 0.0f, 0.0f, -90.0f );
	}
	else
	{
		ForwardDir = Vector( 0.0f, angle, 0.0f );
	}
}

void ScriptSlave::SetModelEvent( Event *ev )
{
	const char *m;
	
	m = ev->GetString( 1 );
	
	setModel( m );
	showModel();
	
	if ( !edict->s.modelindex )
	{
		hideModel();
		setSolidType( SOLID_NOT );
	}
	else if ( !m || strstr( m, ".tik" ) )
	{
		setSolidType( SOLID_BBOX );
	}
	else if ( strstr( m, ".spr" ) )
	{
		setSolidType( SOLID_NOT );
	}
	else
	{
		setSolidType( SOLID_BSP );
	}
}

void ScriptSlave::TriggerEvent( Event *ev )
{
	Entity	*ent;
	Event		*e;
	str		name;
	
	name = ev->GetString(1);
	ent = G_FindTarget(NULL, name);
	
	if ( ent )
	{
		SetTarget( ent->TargetName() );
		
		e = new Event( EV_Trigger_ActivateTargets );
		//fixme
		//get "other"
		e->AddEntity( world );
		ProcessEvent( e );
	}
	else
	{
		gi.WDPrintf( "Invalid entity reference '%s'.\n", name.c_str() );
	}
}

void ScriptSlave::GotoNextWaypoint( Event *ev )
{
	commandswaiting = true;
	
	if ( !waypoint )
	{
		ev->Error( "%s is currently not at a waypoint", TargetName() );
		return;
	}
	
	waypoint = ( Waypoint * )G_FindTarget( NULL, waypoint->Target() );
	if ( !waypoint )
	{
		ev->Error( "%s could not find waypoint %s", TargetName(), waypoint->Target() );
		return;
	}
	else
	{
		SetNewPosition( waypoint->origin );
	}
}

void ScriptSlave::JumpTo( Event *ev )
{
	Entity *part;
	
	//
	// see if it is a vector
	//
	if ( ev->IsVectorAt( 1 ) )
	{
		SetNewPosition( ev->GetVector( 1 ) );
		if ( bind_info && bind_info->bindmaster )
		{
			SetLocalOrigin( bind_info->bindmaster->getLocalVector( GetNewPosition() - bind_info->bindmaster->origin ) );
		}
		else
		{
			SetLocalOrigin( GetNewPosition() );
		}
		
		part = this;
		
		while( part )
		{
			part->setOrigin();
			part->origin.copyTo( part->edict->s.origin2 );
			part->edict->s.renderfx |= RF_FRAMELERP;
			
			if ( part->bind_info )
				part = part->bind_info->teamchain;
			else
				part = NULL;
		}
	}
	else
	{
		waypoint = ( Waypoint * )ev->GetEntity( 1 );
		if ( waypoint )
		{
			SetNewPosition( waypoint->GetLocalOrigin() );
			if ( bind_info && bind_info->bindmaster )
			{
				SetLocalOrigin( bind_info->bindmaster->getLocalVector( GetNewPosition() - bind_info->bindmaster->origin ) );
			}
			else
			{
				SetLocalOrigin( GetNewPosition() );
			}
			
			part = this;
			
			while( part )
			{
				part->setOrigin();
				part->origin.copyTo( part->edict->s.origin2 );
				part->edict->s.renderfx |= RF_FRAMELERP;
				
				if ( part->bind_info )
					part = part->bind_info->teamchain;
				else
					part = NULL;
			}
		}
	}
}

void ScriptSlave::MoveToEvent( Event *ev )
{
	commandswaiting = true;
	
	waypoint = ( Waypoint * )ev->GetEntity( 1 );
	
	if ( waypoint )
	{
		SetNewPosition( waypoint->origin );
	}
}

void ScriptSlave::MoveToPositionEvent( Event *ev )
{
	commandswaiting = true;
	
	SetNewPosition( ev->GetVector( 1 ) );
}

void ScriptSlave::SetSpeed( Event *ev )
{
	speed = ev->GetFloat( 1 );
	traveltime = 0;
}

void ScriptSlave::SetTime( Event *ev )
{
	traveltime = ev->GetFloat( 1 );
}

// Relative move commands

void ScriptSlave::MoveUp( Event *ev )
{
	commandswaiting = true;
	SetNewPosition( Vector( GetNewPosition().x, GetNewPosition().y, GetNewPosition().z + ev->GetFloat( 1 ) ) );
}

void ScriptSlave::MoveDown( Event *ev )
{
	commandswaiting = true;
	SetNewPosition( Vector( GetNewPosition().x, GetNewPosition().y, GetNewPosition().z - ev->GetFloat( 1 ) ) );
}

void ScriptSlave::MoveNorth( Event *ev )
{
	commandswaiting = true;
	SetNewPosition( Vector( GetNewPosition().x, GetNewPosition().y + ev->GetFloat( 1 ), GetNewPosition().z ) );
}

void ScriptSlave::MoveSouth( Event *ev )
{
	commandswaiting = true;
	SetNewPosition( Vector( GetNewPosition().x, GetNewPosition().y - ev->GetFloat( 1 ), GetNewPosition().z ) );
}

void ScriptSlave::MoveEast( Event *ev )
{
	commandswaiting = true;
	SetNewPosition( Vector( GetNewPosition().x + ev->GetFloat( 1 ), GetNewPosition().y, GetNewPosition().z ) );
}

void ScriptSlave::MoveWest( Event *ev )
{
	commandswaiting = true;
	SetNewPosition( Vector( GetNewPosition().x - ev->GetFloat( 1 ), GetNewPosition().y, GetNewPosition().z ) );
}

void ScriptSlave::MoveForward( Event *ev )
{
	Vector v;
	Vector t;
	
	commandswaiting = true;
	
	t = GetNewAngles() + ForwardDir;
	t.AngleVectors( &v, NULL, NULL );
	
	SetNewPosition( GetNewPosition() + v * ev->GetFloat( 1 ) );
}

void ScriptSlave::MoveBackward( Event *ev )
{
	Vector v;
	Vector t;
	
	commandswaiting = true;
	
	t = GetNewAngles() + ForwardDir;
	t.AngleVectors( &v, NULL, NULL );
	
	SetNewPosition( GetNewPosition() - v * ev->GetFloat( 1 ) );
}

void ScriptSlave::MoveLeft( Event *ev )
{
	Vector v;
	Vector t;
	
	commandswaiting = true;
	
	t = GetNewAngles() + ForwardDir;
	t.AngleVectors( NULL, &v, NULL );
	
	SetNewPosition( GetNewPosition() + v * ev->GetFloat( 1 ) );
}

void ScriptSlave::MoveRight( Event *ev )
{
	Vector t;
	Vector v;
	
	commandswaiting = true;
	
	t = GetNewAngles() + ForwardDir;
	t.AngleVectors( NULL, &v, NULL );
	
	SetNewPosition( GetNewPosition() - v * ev->GetFloat( 1 ) );
}

// exact rotate commands

void ScriptSlave::RotateXdownto( Event *ev )
{
	commandswaiting = true;
	
	Vector newAngles( GetNewAngles() );
	newAngles[ 0 ] = ev->GetFloat( 1 );
	if ( newAngles[ 0 ] < localangles[ 0 ] )
	{
		newAngles[ 0 ] -= 360.0f;
	}
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateYdownto( Event *ev )
{
	commandswaiting = true;
	
	Vector newAngles( GetNewAngles() );
	newAngles[ 1 ] = ev->GetFloat( 1 );
	if ( newAngles[ 1 ] < localangles[ 1 ] )
	{
		newAngles[ 1 ] -= 360.0f;
	}
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateZdownto( Event *ev )
{
	commandswaiting = true;
	
	Vector newAngles( GetNewAngles() );
	newAngles[ 2 ] = ev->GetFloat( 1 );
	if ( newAngles[ 2 ] < localangles[ 2 ] )
	{
		newAngles[ 2 ] -= 360.0f;
	}
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateAxisdownto( Event *ev )
{
	int axis;
	commandswaiting = true;
	
	axis = ev->GetInteger( 1 );
	
	Vector newAngles( GetNewAngles() );
	newAngles[ axis ] = ev->GetFloat( 1 );
	if ( newAngles[ axis ] < localangles[ axis ] )
	{
		newAngles[ axis ] -= 360.0f;
	}
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateXupto( Event *ev )
{
	commandswaiting = true;
	Vector newAngles( GetNewAngles() );
	newAngles[ 0 ] = ev->GetFloat( 1 );
	if ( newAngles[ 0 ] < localangles[ 0 ] )
	{
		newAngles[ 0 ] += 360.0f;
	}
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateYupto( Event *ev )
{
	commandswaiting = true;
	
	Vector newAngles( GetNewAngles() );
	newAngles[ 1 ] = ev->GetFloat( 1 );
	if ( newAngles[ 1 ] < localangles[ 1 ] )
	{
		newAngles[ 1 ] += 360.0f;
	}
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateZupto( Event *ev )
{
	commandswaiting = true;
	
	Vector newAngles( GetNewAngles() );
	newAngles[ 2 ] = ev->GetFloat( 1 );
	if ( newAngles[ 2 ] < localangles[ 2 ] )
	{
		newAngles[ 2 ] += 360.0f;
	}
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateAxisupto( Event *ev )
{
	int axis;
	commandswaiting = true;
	
	axis = ev->GetInteger( 1 );
	Vector newAngles( GetNewAngles() );
	newAngles[ axis ] = ev->GetFloat( 1 );
	if ( newAngles[ axis ] < localangles[ axis ] )
	{
		newAngles[ axis ] += 360.0f;
	}
	SetNewAngles( newAngles );
}

// full vector rotation

void ScriptSlave::Rotatedownto( Event *ev )
{
	Vector ang;
	commandswaiting = true;
	
	ang = ev->GetVector( 1 );
	
	Vector newAngles( GetNewAngles() );
	newAngles[ 0 ] = ang[ 0 ];
	if ( newAngles[ 0 ] > localangles[ 0 ] )
	{
		newAngles[ 0 ] -= 360.0f;
	}
	newAngles[ 1 ] = ang[ 1 ];
	if ( newAngles[ 1 ] > localangles[ 1 ] )
	{
		newAngles[ 1 ] -= 360.0f;
	}
	newAngles[ 2 ] = ang[ 2 ];
	if ( newAngles[ 2 ] > localangles[ 2 ] )
	{
		newAngles[ 2 ] -= 360.0f;
	}
	SetNewAngles( newAngles );
}

void ScriptSlave::Rotateupto( Event *ev )
{
	Vector ang;
	commandswaiting = true;
	
	ang = ev->GetVector( 1 );
	
	Vector newAngles( GetNewAngles() );
	newAngles[ 0 ] = ang[ 0 ];
	if ( newAngles[ 0 ] < localangles[ 0 ] )
	{
		newAngles[ 0 ] += 360.0f;
	}
	newAngles[ 1 ] = ang[ 1 ];
	if ( newAngles[ 1 ] < localangles[ 1 ] )
	{
		newAngles[ 1 ] += 360.0f;
	}
	newAngles[ 2 ] = ang[ 2 ];
	if ( newAngles[ 2 ] < localangles[ 2 ] )
	{
		newAngles[ 2 ] += 360.0f;
	}
	SetNewAngles( newAngles );
}

void ScriptSlave::Rotateto( Event *ev )
{
	commandswaiting = true;
	
	Vector ang = ev->GetVector( 1 );
	
	SetNewAngles( ang );
}

// Relative rotate commands

void ScriptSlave::RotateXdown( Event *ev )
{
	commandswaiting = true;
	Vector newAngles( GetNewAngles() );
	newAngles[ 0 ] = localangles[ 0 ] - ev->GetFloat( 1 );
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateYdown( Event *ev )
{
	commandswaiting = true;
	Vector newAngles( GetNewAngles() );
	newAngles[ 1 ] = localangles[ 1 ] - ev->GetFloat( 1 );
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateZdown( Event *ev )
{
	commandswaiting = true;
	Vector newAngles( GetNewAngles() );
	newAngles[ 2 ] = localangles[ 2 ] - ev->GetFloat( 1 );
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateAxisdown( Event *ev )
{
	int axis;
	commandswaiting = true;
	
	axis = ev->GetInteger( 1 );
	Vector newAngles( GetNewAngles() );
	newAngles[ axis ] = localangles[ axis ] - ev->GetFloat( 1 );
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateXup( Event *ev )
{
	commandswaiting = true;
	Vector newAngles( GetNewAngles() );
	newAngles[ 0 ] = localangles[ 0 ] + ev->GetFloat( 1 );
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateYup( Event *ev )
{
	commandswaiting = true;
	Vector newAngles( GetNewAngles() );
	newAngles[ 1 ] = localangles[ 1 ] + ev->GetFloat( 1 );
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateZup( Event *ev )
{
	commandswaiting = true;
	Vector newAngles( GetNewAngles() );
	newAngles[ 2 ] = localangles[ 2 ] + ev->GetFloat( 1 );
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateAxisup( Event *ev )
{
	int axis;
	commandswaiting = true;
	
	axis = ev->GetInteger( 1 );
	Vector newAngles( GetNewAngles() );
	newAngles[ axis ] = localangles[ axis ] + ev->GetFloat( 1 );
	SetNewAngles( newAngles );
}

void ScriptSlave::RotateX( Event *ev )
{
	avelocity[ 0 ] = ev->GetFloat( 1 );
}

void ScriptSlave::RotateY( Event *ev )
{
	avelocity[ 1 ] = ev->GetFloat( 1 );
}

void ScriptSlave::RotateZ( Event *ev )
{
	avelocity[ 2 ] = ev->GetFloat( 1 );
}

void ScriptSlave::RotateAxis( Event *ev )
{
	int axis;
	
	axis = ev->GetInteger( 1 );
	avelocity[ axis ] = ev->GetFloat( 2 );
}


void ScriptSlave::OnTouch( Event *ev )
{
	const char *jumpto;
	
	touchlabel = "";
	
	jumpto = ev->GetString( 1 );
	touchthread = ev->GetThread();
	
	assert( jumpto && touchthread );
	if ( touchthread && !touchthread->labelExists( jumpto ) )
	{
		ev->Error( "Label '%s' not found", jumpto );
		return;
	}
	
	// if it isn't solid than we need to change it to a trigger
	if ( getSolidType() == SOLID_NOT )
	{
		setSolidType( SOLID_TRIGGER );
	}
	
	touchlabel = jumpto;
}

void ScriptSlave::NoTouch( Event * )
{
	touchlabel = "";
	// if it is a trigger than it wasn't solid, so restore that condition
	if ( getSolidType() == SOLID_TRIGGER )
	{
		setSolidType( SOLID_NOT );
	}
}

void ScriptSlave::TouchFunc( Event *ev )
{
	Event *e;
	Entity *other;
	
	if ( touchlabel.length() )
	{
		// since we use a SafePtr, the thread pointer will be NULL if the thread has ended
		// so we should just clear our label and continue
		if ( !touchthread )
		{
			touchlabel = "";
			return;
		}
		
		other = ev->GetEntity( 1 );
		
		setActivatingEntity( other );
		
		e = new Event( EV_ScriptThread_Callback );
		e->AddEntity( this );
		e->AddString( touchlabel );
		e->AddEntity( other );
		touchthread->ProcessEvent( e );
	}
}

void ScriptSlave::OnBlock( Event *ev )
{
	const char *jumpto;
	
	blocklabel = "";
	
	jumpto = ev->GetString( 1 );
	blockthread = ev->GetThread();
	
	assert( jumpto && blockthread );
	if ( blockthread && !blockthread->labelExists( jumpto ) )
	{
		ev->Error( "Label '%s' not found", jumpto );
		return;
	}
	
	blocklabel = jumpto;
}

void ScriptSlave::NoBlock( Event * )
{
	blocklabel = "";
}

void ScriptSlave::BlockFunc( Event *ev )
{
	Event *e;
	Entity *other;
	
	other = ev->GetEntity( 1 );
	if ( level.time >= attack_finished )
	{
		attack_finished = level.time + ( float )0.5;
		if ( dmg != 0 )
		{
			other->Damage( this, this, dmg, origin, vec_zero, vec_zero, 0, 0, dmg_means_of_death );
		}
	}
	
	if ( blocklabel.length() )
	{
		// since we use a SafePtr, the thread pointer will be NULL if the thread has ended
		// so we should just clear our label and continue
		if ( !blockthread )
		{
			blocklabel = "";
			return;
		}
		
		setActivatingEntity( other );
		
		e = new Event( EV_ScriptThread_Callback );
		e->AddEntity( this );
		e->AddString( blocklabel );
		e->AddEntity( other );
		blockthread->ProcessEvent( e );
	}
}

void ScriptSlave::OnTrigger( Event *ev )
{
	const char *jumpto;
	
	triggerlabel = "";
	
	jumpto = ev->GetString( 1 );
	triggerthread = ev->GetThread();
	
	assert( jumpto && triggerthread );
	
	if ( triggerthread && !triggerthread->labelExists( jumpto ) )
	{
		ev->Error( "Label '%s' not found", jumpto );
		return;
	}
	
	triggerlabel = jumpto;
}

void ScriptSlave::NoTrigger( Event * )
{
	triggerlabel = "";
}

void ScriptSlave::TriggerFunc( Event *ev )
{
	Event *e;
	Entity *other;
	
	if ( triggerlabel.length() )
	{
		// since we use a SafePtr, the thread pointer will be NULL if the thread has ended
		// so we should just clear our label and continue
		if ( !triggerthread )
		{
			triggerlabel = "";
			return;
		}
		
		other = ev->GetEntity( 1 );
		
		setActivatingEntity( other );
		
		e = new Event( EV_ScriptThread_Callback );
		e->AddEntity( this );
		e->AddString( triggerlabel );
		e->AddEntity( other );
		
		triggerthread->ProcessEvent( e );
	}
}

void ScriptSlave::OnUse( Event *ev )
{
	const char *jumpto;
	
	uselabel = "";
	
	jumpto = ev->GetString( 1 );
	usethread = ev->GetThread();
	
	assert( jumpto && usethread );
	
	if ( usethread && !usethread->labelExists( jumpto ) )
	{
		ev->Error( "Label '%s' not found", jumpto );
		return;
	}
	
	uselabel = jumpto;
}

void ScriptSlave::NoUse( Event * )
{
	uselabel = "";
}

void ScriptSlave::UseFunc( Event *ev )
{
	Event *e;
	Entity *other;
	
	other = ev->GetEntity( 1 );
	
	// See if object == key
	
	if ( other->isSubclassOf( Equipment ) )
	{
		Equipment *equipment = (Equipment *)other;
		
		if ( equipment->getTypeName() != key )
			return;
	}
	else if ( key.length() )
	{
		if ( !other->isSubclassOf( Sentient ) || !( ( (Sentient *)other )->HasItem( key.c_str() ) ) )
		{
			Item        *item;
			ClassDef		*cls;
			
			cls = getClass( key.c_str() );
			if ( !cls )
			{
				gi.WDPrintf( "No item named '%s'\n", key.c_str() );
				return;
			}
			item = ( Item * )cls->newInstance();
			item->CancelEventsOfType( EV_Item_DropToFloor );
			item->CancelEventsOfType( EV_Remove );
			item->ProcessPendingEvents();
			gi.centerprintf ( other->edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$ItemNeeded$$%s", item->getName().c_str() );
			delete item;
			return;
		}
		else if ( other->isSubclassOf( Sentient ) )
		{
			Item *item = ( (Sentient *)other )->FindItem( key.c_str() );
			
			if ( !item )
				return;

			if ( item->isSubclassOf( Equipment ) )
			{
				if ( _nextNeedToUseTime < level.time )
				{
					gi.centerprintf ( other->edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$NeedToUse$$ %s", key.c_str() );

					if ( other->isSubclassOf( Player ) )
					{
						Player *player = (Player *)other;

						player->loadUseItem( key );
					}

					_nextNeedToUseTime = level.time + 1.0f;
				}

				return;
			}
		}
	}
	
	if ( uselabel.length() )
	{
		//ScriptVariableList *vars;
		
		// since we use a SafePtr, the thread pointer will be NULL if the thread has ended
		// so we should just clear our label and continue
		if ( !usethread )
		{
			uselabel = "";
			return;
		}
		
		setActivatingEntity( other );
		
		e = new Event( EV_ScriptThread_Callback );
		e->AddEntity( this );
		e->AddString( uselabel );
		e->AddEntity( other );
		
		/* vars = usethread->Vars();
		vars->SetVariable( "other", other );
		if ( key.length() )
		{
		vars->SetVariable( "key", key.c_str() );
	} */
		usethread->ProcessEvent( e );
	}
}

void ScriptSlave::OnDamage( Event *ev )
{
	const char *jumpto;
	
	damagelabel = "";
	
	jumpto = ev->GetString( 1 );
	damagethread = ev->GetThread();
	
	assert( jumpto && damagethread );
	
	if ( damagethread && !damagethread->labelExists( jumpto ) )
	{
		ev->Error( "Label '%s' not found", jumpto );
		return;
	}
	
	damagelabel = jumpto;
}

void ScriptSlave::NoDamage( Event * )
{
	damagelabel = "";
}

void ScriptSlave::DamageFunc( Event *ev )
{
	Event			*e;
	//Entity		*inflictor;
	Entity		*attacker;
	//int			damage;
	//Vector		position;
	//Vector		direction;
	//ScriptVariableList *vars;
	
	attacker	= ev->GetEntity( 3 );

	if ( !_canBeAttackedByOtherScriptSlaves )
		{
		if ( attacker->isSubclassOf(ScriptSlave) )
			return;
		}

	if ( _handlesDamage )
	{
		Event *newDamageEvent = new Event (ev);
		Entity::DamageEvent(newDamageEvent);
	}
	
	if ( damagelabel.length() )
	{
		// since we use a SafePtr, the thread pointer will be NULL if the thread has ended
		// so we should just clear our label and continue
		if ( !damagethread )
		{
			damagelabel = "";
			return;
		}
		
		if( _requiredMODlist.NumObjects() != 0 )
		{
			bool modMatches = false;
			for( int i = 1;	!modMatches && i <= _requiredMODlist.NumObjects(); i++ )
			{
				str& modname = _requiredMODlist.ObjectAt( i );
				int modID = MOD_NameToNum( modname );
				int damageMOD = ev->GetInteger( 9 );

				if ( modID == damageMOD )
					modMatches = true;
			}

			if( !modMatches )
				return;
		}
		
		
		setActivatingEntity( attacker );
		
		e = new Event( EV_ScriptThread_Callback );
		e->AddEntity( this );
		e->AddString( damagelabel );
		e->AddEntity( attacker );
		
		/* damage		= ev->GetInteger( 1 );
		inflictor	= ev->GetEntity( 2 );
		position		= ev->GetVector( 4 );
		direction	= ev->GetVector( 5 ); */
		
		/* vars = damagethread->Vars();
		vars->SetVariable( "damage", damage );
		vars->SetVariable( "inflictor", inflictor );
		vars->SetVariable( "attacker", attacker );
		vars->SetVariable( "position", position );
		vars->SetVariable( "direction", direction ); */
		damagethread->ProcessEvent( e );
	}
	
	if ( _bloodModel.length() )
	{
		SpawnEffect( _bloodModel , ev->GetVector( 4 ) , ev->GetVector( 6 ) , 1.0f );
	}

	if ( _damageEffect.length() )
	{
		//SpawnEffect( "models/fx/fx-impactburn-sniperrifle.tik", ev->GetVector( 4 ), ev->GetVector( 6 ), 1.0f );
		SpawnEffect( "models/fx/fx-electricitymesh-impactpoint.tik", ev->GetVector( 4 ), ev->GetVector( 6 ), 1.0f );
	}
}

void ScriptSlave::SetDamage( Event *ev )
{
	dmg = ev->GetFloat( 1 );
}

void ScriptSlave::SetMeansOfDeath( Event *ev )
{
	dmg_means_of_death = MOD_NameToNum( ev->GetString( 1 ) );
}

void ScriptSlave::CreatePath( SplinePath *path, splinetype_t type )
{
	SplinePath* node;
	SplinePath* nextNode;
	
	if ( !splinePath )
	{
		splinePath = new BSpline;
	}
	
	splinePath->Clear();
	splinePath->SetType( type );
	
	node = path;
	while( node != NULL )
	{
		splinePath->AppendControlPoint( node->origin, node->angles, node->speed );
		
		// get the next node and check it before using it
		nextNode = node->GetNext();
		if( nextNode == node )
		{
			gi.Error( ERR_DROP, "info_splinepath '%s' targets itself\n", node->targetname.c_str() );
		}
		
		// don't loop
		if ( nextNode == path )
		{
			break;
		}

		// move to the next node
		node = nextNode;
	}
}

void ScriptSlave::FollowPath( Event *ev )
{
	int i, argnum;
	Entity * ent;
	const char * token;
	SplinePath *path;
	qboolean clamp;
	float starttime;
	
	
	ent = ev->GetEntity( 1 );
	argnum = 2;
	starttime = -2;
	clamp = true;
	ignoreangles = false;
	splineangles = true;
	for ( i = argnum; i <= ev->NumArgs() ; i++ )
	{
		token = ev->GetString( i );
		if (!strcmpi( token, "ignoreangles"))
		{
			ignoreangles = true;
		}
		else if (!strcmpi( token, "normalangles"))
		{
			splineangles = false;
		}
		else if (!strcmpi (token, "loop"))
		{
			clamp = false;
		}
		else if ( IsNumeric( token ) )
		{
			starttime = (float)atof( token );
		}
		else
		{
			ev->Error( "Unknown followpath command %s.", token );
		}
	}
	if ( ent && ent->isSubclassOf( SplinePath ) )
	{
		commandswaiting = true;
		path = ( SplinePath * )ent;
		if ( clamp )
			CreatePath( path, SPLINE_CLAMP );
		else
			CreatePath( path, SPLINE_LOOP );
		
		currentNode = path;
		
		if ( currentNode->thread != "" )
		{
			if ( !ExecuteThread( currentNode->thread , true , this) )
			{
				gi.Error( ERR_DROP, "Scriptslave could not start thread '%s' from info_splinepath '%s'\n", 
					currentNode->thread.c_str(), currentNode->targetname.c_str() );
			}
		}
		
		if ( currentNode->triggertarget != "" )
		{
			Entity	*ent;
			Event		*event;
			
			ent = NULL;
			do
			{
				ent = G_FindTarget( ent, currentNode->triggertarget.c_str() );
				if ( !ent )
				{
					break;
				}
				event = new Event( EV_Activate );
				event->AddEntity( this );
				ent->PostEvent( event, 0.0f );
			}
			while ( 1 );
		}
		
		splineTime = starttime;
		lastTime = (int)(splineTime + 2.0f);
		newTime = (int)(splineTime + 2.0f);
		CancelEventsOfType( EV_ScriptSlave_FollowingPath );
		if ( !ignoreangles )
		{
			avelocity = vec_zero;
		}
		velocity = vec_zero;
	}
}

void ScriptSlave::EndPath( Event * )
{
	if ( !splinePath )
		return;
	
	delete splinePath;
	splinePath = NULL;
	velocity = vec_zero;
	if ( !ignoreangles )
	{
		avelocity = vec_zero;
	}
}

void ScriptSlave::FollowingPath( Event * )
{
	Vector	pos;
	Vector	orient;
	float    speed_multiplier;
	
	if ( !splinePath )
		return;
	
	if ( !thinking )
	   {
		velocity = vec_zero;
		if ( !ignoreangles )
		{
			avelocity = vec_zero;
		}
		
		return;
		//PostEvent( EV_ScriptSlave_FollowingPath, level.frametime );
	}
	
	if ( ( splinePath->GetType() == SPLINE_CLAMP ) && ( splineTime > ( splinePath->EndPoint() - 2.0f ) ) )
	{
		delete splinePath;
		splinePath = NULL;
		velocity = vec_zero;
		if ( !ignoreangles )
		{
			avelocity = vec_zero;
		}
		moving = false;
		ProcessEvent( EV_ScriptSlave_MoveDone );
		return;
	}
	
	if ( ( lastTime != newTime ) && currentNode )
	{
		if ( newTime > 1 )
		{
			
			if ( currentNode->thread != "" )
			{
				if ( !ExecuteThread( currentNode->thread ,true , this) )
				{
					gi.Error( ERR_DROP, "Scriptslave could not start thread '%s' from info_splinepath '%s'\n", 
						currentNode->thread.c_str(), currentNode->targetname.c_str() );
				}
			}
			
			if ( currentNode->triggertarget != "" )
			{
				Entity	*ent;
				Event		*event;
				
				ent = NULL;
				do
				{
					ent = G_FindTarget( ent, currentNode->triggertarget.c_str() );
					if ( !ent )
					{
						break;
					}
					event = new Event( EV_Activate );
					event->AddEntity( this );
					ent->PostEvent( event, 0.0f );
				}
				while ( 1 );
			}
			
		}
		
		currentNode = currentNode->GetNext();
	}
	
	lastTime = newTime;
	
	speed_multiplier = splinePath->Eval( splineTime, pos, orient );
	
	splineTime += level.frametime * speed_multiplier;
	
	velocity = ( pos - origin ) * ( 1.0f / level.frametime );
	if ( !ignoreangles )
	{
		if ( splineangles )
		{
			avelocity = ( orient - angles ) * ( 1.0f / level.frametime );
		}
		else
		{
			float len;
			
			len = velocity.length();
			if ( len > 0.05f )
            {
				Vector ang;
				Vector dir;
				float  aroll;
				
				aroll = avelocity[ ROLL ];
				dir = velocity * ( 1.0f / len );
				ang = dir.toAngles();
				avelocity = ( ang - angles ) * ( 1.0f / level.frametime );
				avelocity[ ROLL ] = aroll;
            }
			else
				avelocity = vec_zero;
		}
	}
	
	newTime = splineTime + 2.0f;
	
	if ( newTime < 0 )
	{
		newTime = 0;
	}
	
	PostEvent( EV_ScriptSlave_FollowingPath, level.frametime );	
	
}

void ScriptSlave::Explode( Event *ev )
{
	float damage;
	
	if ( ev->NumArgs() )
	{
		damage = ev->GetFloat( 1 );
	}
	else
	{
		damage = 120.0f;
	}
	
	CreateExplosion( origin, damage, this, this, this );
}

void ScriptSlave::NotShootable( Event * )
{
	setContents( 0 );
}

void ScriptSlave::OpenPortal( Event * )
{
	if ( !_portalOpen )
	{
 		gi.AdjustAreaPortalState( this->edict, true );
		_portalOpen = true;
	}
}

void ScriptSlave::ClosePortal( Event * )
{
	if ( _portalOpen )
	{
		gi.AdjustAreaPortalState( this->edict, false );
		_portalOpen = false;
	}
}

void ScriptSlave::PhysicsOn( Event * )
{
	commandswaiting = false;
	setMoveType( MOVETYPE_BOUNCE );
	setSolidType( SOLID_BBOX );
	velocity = Vector(0, 0, 1);
	edict->clipmask = MASK_SOLID|CONTENTS_BODY;
}

void ScriptSlave::PhysicsOff( Event * )
{
	Event * event;
	
	commandswaiting = false;
	setMoveType( MOVETYPE_PUSH );
	edict->clipmask = 0;
	// become solid again
	event = new Event( EV_Model );
	event->AddString( model );
	PostEvent( event, 0.0f );
}

void ScriptSlave::PhysicsVelocity( Event *ev )
{
	velocity += ev->GetVector( 1 );
}

void ScriptSlave::StopEvent( Event * )
{
	Stop();
}

void ScriptSlave::Stop( void )
{
	thinking = false;
}

void ScriptSlave::ContinueEvent( Event * )
{
	Continue();
}

void ScriptSlave::Continue( void )
{
	thinking = true;
	PostEvent( EV_ScriptSlave_FollowingPath, level.frametime );
}

//-----------------------------------------------------
//
// Name:		Think
// Class:		ScriptSlave
//
// Description:	Processes the updates for the Script Slave.
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void ScriptSlave::Think()
{
	if(flags & FlagTouchTriggers)
	{
		G_TouchTriggers(this);
	}
}

/*****************************************************************************/
/*QUAKED script_model (0 0.5 1) (0 0 0) (0 0 0) NOT_SOLID LIT_STATIC

******************************************************************************/

Event EV_ScriptModel_SetAnim
(
	"anim",
	EV_DEFAULT,
	"s",
	"anim_name",
	"Sets the script model's animation"
);
Event EV_ScriptModel_SetAnimDriven
(
	"animdriven",
	EV_DEFAULT,
	"B",
	"boolean",
	"Sets the script model to be anim driven"
);
Event EV_ScriptModel_AnimOnce
(
	"animonce",
	EV_SCRIPTONLY,
	"s",
	"anim_name",
	"Sets the script model's animation but only plays it once"
);

CLASS_DECLARATION( ScriptSlave, ScriptModel, "script_model" )
{
	{ &EV_Gib,							&ScriptModel::GibEvent },
	{ &EV_SetAngle,						&ScriptModel::SetAngleEvent },
	{ &EV_ScriptModel_SetAnim,			&ScriptModel::SetAnimEvent },
	{ &EV_ScriptModel_SetAnimDriven,	&ScriptModel::SetAnimDrivenEvent },
	{ &EV_ScriptModel_AnimOnce,			&ScriptModel::AnimOnceEvent },
	{ &EV_Model,						&ScriptModel::SetModelEvent },

	{ NULL, NULL },
};

ScriptModel::ScriptModel()
{
	// this is a tiki model
	edict->s.eType = ET_MODELANIM;
	animationDriven = false;
	
	animate = new Animate( this );
}

void ScriptModel::Think( void )
{
	total_delta = vec_zero;
	ScriptSlave::Think();
	
	if(animationDriven)
	{
		localangles = InterceptTargetXY( GetNewPosition(), Vector::Identity(), velocity.lengthXY() );
		SetNewAngles( localangles );
		setAngles( localangles );
		
		// Set velocity based on current delta move
		Vector forward;
		Vector left;
		Vector up;
		localangles.AngleVectors( &forward, &left, &up );
		
		total_delta /= level.frametime;
		velocity = ( forward * total_delta[0] ) + ( left * total_delta[1] ) + ( up * total_delta[2] );
	}
}

void ScriptModel::SetModelEvent( Event *ev )
{
	char	 modelname[256] ;
	char	*tmpPtr ;
	
	strcpy(modelname, ev->GetString( 1 ) );
	tmpPtr = strstr(modelname, "*");
	
	if (tmpPtr)
	{
		ev->SetString( 1 , tmpPtr );
	}
	
	ScriptSlave::SetModelEvent( ev );
	setSolidType( SOLID_BBOX );
	
	if ( ( gi.IsModel( edict->s.modelindex ) ) && !mins.length() && !maxs.length() )
	{
		gi.CalculateBounds( edict->s.modelindex, edict->s.scale, mins, maxs );
	}
}

void ScriptModel::SetAnimEvent( Event *ev )
{
	const char * const animname = ev->GetString( 1 );
	if ( animname && strlen( animname ) && gi.IsModel( edict->s.modelindex ) )
	{
		int animnum = gi.Anim_NumForName( edict->s.modelindex, animname );
		
		if ( animnum >= 0 )
		{
			animate->NewAnim( animnum );
		}
	}
}

void ScriptModel::SetAnimDrivenEvent( Event *ev )
{
	if ( (ev->NumArgs() > 0) && ( ev->GetBoolean( 1 ) == false ) )
	{
		animationDriven = false;
	}
	else
	{
		animationDriven = true;
		turnThinkOn();
	}
}

void ScriptModel::AnimOnceEvent( Event *ev )
{
	const char * animname;
	
	animname = ev->GetString( 1 );
	if ( animname && strlen( animname ) && gi.IsModel( edict->s.modelindex ) )
	{
		animate->RandomAnimate( animname, EV_StopAnimating );
	}
}

void ScriptModel::SetAngleEvent( Event *ev )
{
	float angle;
	
	angle = ev->GetFloat( 1 );
	if ( angle == -1.0f )
	{
		ForwardDir = Vector( 0.0f, 0.0f, 90.0f );
		localangles = Vector( -90.0f, 0.0f, 0.0f );
	}
	else if ( angle == -2.0f )
	{
		ForwardDir = Vector( 0.0f, 0.0f, -90.0f );
		localangles = Vector( 90.0f, 0.0f, 0.0f );
	}
	else
	{
		ForwardDir = Vector( 0.0f, angle, 0.0f );
		localangles = Vector( 0.0f, angle, 0.0f );
	}
	
	setAngles( localangles );
}

void ScriptModel::GibEvent( Event *ev )
{
	int      num,power;
	float    scale;
	
	setSolidType( SOLID_NOT );
	hideModel();
	
	if ( !com_blood->integer )
	{
		PostEvent( EV_Remove, 0.0f );
		return;
	}
	
	num = ev->GetInteger( 1 );
	power = ev->GetInteger( 2 );
	scale = ev->GetFloat( 3 );
	
	power = -power;
	
	if ( ev->NumArgs() > 3 )
	{
		CreateGibs( this, power, scale, num, ev->GetString( 4 ) );
	}
	else
	{
		CreateGibs( this, power, scale, num );
	}
	
	PostEvent( EV_Remove, 0.0f );
}

/*****************************************************************************/
/*QUAKED script_origin (1.0 0 0) (-8 -8 -8) (8 8 8)

Used as an alternate origin for objects.  Bind the object to the script_origin
in order to simulate changing that object's origin.
******************************************************************************/

CLASS_DECLARATION( ScriptSlave, ScriptOrigin, "script_origin" )
{
	{ &EV_Model,				&ScriptOrigin::SetModelEvent },

	{ NULL, NULL }
};

ScriptOrigin::ScriptOrigin()
{
	setContents( 0 );
	setSolidType( SOLID_NOT );
}

/*****************************************************************************/
/*QUAKED script_skyorigin (1.0 0 0) ?

Used to specify the origin of a portal sky
******************************************************************************/

Event EV_ScriptSkyOrigin_SetBasePosition
(
	"baseposition",
	EV_SCRIPTONLY,
	"v",
	"base_position",
	"Sets the base position of the sky origin."
);
Event EV_ScriptSkyOrigin_SetPlayerBasePosition
(
	"playerbaseposition",
	EV_SCRIPTONLY,
	"v",
	"base_position",
	"Sets the base position for the player for the sky origin."
);
Event EV_ScriptSkyOrigin_SetTranslationMultiplier
(
	"translationmult",
	EV_SCRIPTONLY,
	"f",
	"translation_multiplier",
	"Sets the translation multiplier for the sky origin."
);
Event EV_ScriptSkyOrigin_SetMaxDistance
(
	"maxtranslationdist",
	EV_SCRIPTONLY,
	"f",
	"max_translation_distance",
	"Sets the maximum distance the sky origin will translate."
);

CLASS_DECLARATION( ScriptSlave, ScriptSkyOrigin, "script_skyorigin" )
{
	{ &EV_ScriptSkyOrigin_SetBasePosition,					&ScriptSkyOrigin::SetBasePosition },
	{ &EV_ScriptSkyOrigin_SetPlayerBasePosition,			&ScriptSkyOrigin::SetPlayerBasePosition },
	{ &EV_ScriptSkyOrigin_SetTranslationMultiplier,			&ScriptSkyOrigin::SetTranslationMultiplier },
	{ &EV_ScriptSkyOrigin_SetMaxDistance,					&ScriptSkyOrigin::SetMaxDistance },

	{ NULL, NULL }
};

ScriptSkyOrigin::ScriptSkyOrigin()
{
	edict->s.renderfx |= RF_SKYORIGIN;
	setContents( 0 );
	setSolidType( SOLID_NOT );
	turnThinkOn();
	
	use_base_position = false;
	use_player_base_position = false;
	translation_multiplier = 0;
	max_distance = 0;
}

void ScriptSkyOrigin::Think( void )
{
	Vector delta;
	Entity *player;
	Vector new_origin;
	
	new_origin = origin;
	
	if ( use_base_position )
	{
		if ( translation_multiplier  )
		{
			// Get player
			
			player = g_entities[ 0 ].entity;
			
			if ( !use_player_base_position )
			{
				// Get current player position
				
				player_base_position = player->origin;
				
				use_player_base_position = true;
			}
			
			// Calculate the new origin
			
			delta = player->origin - player_base_position;
			delta *= translation_multiplier;
			
			if ( max_distance && ( delta.length() > max_distance ) )
			{
				delta.normalize();
				delta *= max_distance;
			}
			
			new_origin = base_position + delta;
		}
		else
		{
			new_origin = base_position;
		}
	}
	
	setOrigin( new_origin );
}

void ScriptSkyOrigin::SetBasePosition( Event *ev )
{
	use_base_position = true;
	base_position = ev->GetVector( 1 );
	use_player_base_position = false;
}

void ScriptSkyOrigin::SetPlayerBasePosition( Event *ev )
{
	use_player_base_position = true;
	player_base_position = ev->GetVector( 1 );
}

void ScriptSkyOrigin::SetTranslationMultiplier( Event *ev )
{
	translation_multiplier = ev->GetFloat( 1 );
}

void ScriptSkyOrigin::SetMaxDistance( Event *ev )
{
	max_distance = ev->GetFloat( 1 );
}


//1ST PLAYABLE HACK STUFF
void ScriptSlave::Hack_AddParms( Event *ev )
{
	_forcefieldNumber = ev->GetFloat( 1 );
	_triggerNumber   = ev->GetFloat( 2 );   
}

void ScriptSlave::Hack_GetForceFieldNumber( Event *ev )
{
	ev->ReturnFloat( _forcefieldNumber );
}

void ScriptSlave::Hack_GetScanner( Event *ev )
{
	ev->ReturnEntity( _scanner );   
}

void ScriptSlave::Hack_GetTriggerNumber( Event *ev )
{
	ev->ReturnFloat( _triggerNumber );       
}

void ScriptSlave::HandlesDamage( Event *ev )
{
	_handlesDamage = ev->GetBoolean( 1 );
}

void ScriptSlave::setDamageEffect( Event *ev )
{
	_damageEffect = ev->GetString( 1 );
}

void ScriptSlave::setBloodModel( Event *ev )
{
	_bloodModel = ev->GetString( 1 );	
}

void ScriptSlave::addRequiredDamageMOD( Event *ev )
{
	str modname( ev->GetString( 1 ) );
	assert( modname.length() > 0 );
	_requiredMODlist.AddObject( modname );

}

void ScriptSlave::setCanBeAttackedByOtherSlaves(Event *ev)
{
	_canBeAttackedByOtherScriptSlaves = ev->GetBoolean( 1 );
}
