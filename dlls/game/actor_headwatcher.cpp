//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actor_headwatcher.cpp                          $
// $Revision:: 21                                                             $
//   $Author:: Sketcher                                                       $
//     $Date:: 5/04/03 5:49p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//

#include "_pch_cpp.h"
#include "actor_enemymanager.h"
#include "player.h"
#include "object.h"

HeadWatcher::HeadWatcher()
{
	// Should always use other constructor
	gi.Error( ERR_FATAL, "HeadWatcher::HeadWatcher -- Default Constructor Called" );
	
	
}

HeadWatcher::HeadWatcher( Actor *actor )
{
	//Initialize our Actor
	if ( actor )
		act = actor;
	else 
		gi.Error( ERR_DROP, "HeadWatcher::HeadWatcher -- actor is NULL" );
	
	_init();
	
}

HeadWatcher::~HeadWatcher()
{
	
}

void HeadWatcher::_init()
{
	act->SetControllerTag( ACTOR_HEAD_TAG, gi.Tag_NumForName( act->edict->s.modelindex, "Bip01 Head" ) );
	
	_currentHeadAngles = act->GetControllerAngles( ACTOR_HEAD_TAG );
	_watchTarget = NULL;
	
	// Numbers here for testing, should come from events so they can be set via tiki or script
	_maxHeadTurnSpeed = 30.0f;
	_turnThreshold    = 0.0f;
	_maxHeadYaw		= 60.0f;
	_maxHeadPitch	= 45.0f;
	
	_twitchHead = false;
	_nextTwitchHeadTime = 0.0f;
	_maxDistance = -1.0;

	_explicitSet = false;
	_ignoreWatchTarget = false;
}

void HeadWatcher::SetWatchTarget( Entity *ent )
{
	_watchTarget = ent;
	_explicitSet = true;
}

void HeadWatcher::SetWatchTarget( const str &targetName )
{
	TargetList		*tlist;
	int            numObjects;
	
	
	tlist = world->GetTargetList( targetName );
	numObjects = tlist->list.NumObjects();
	
	if ( numObjects == 0 )
	{
		//Hey, No targets with that name
		gi.WDPrintf( "No target with target name %s specified\n", targetName.c_str() );
		return;
	}
	else if ( numObjects > 1 )
	{
		//Uh Oh... We have more than one target... Let's throw up an error
		gi.WDPrintf( "More than one target with target name %s specified, grabbing first one\n", targetName.c_str() );
	}
	
	_watchTarget = tlist->list.ObjectAt( 1 );
	_explicitSet = true;
}

void HeadWatcher::SetWatchSpeed( float speed )
{
	_maxHeadTurnSpeed = speed;
}

void HeadWatcher::ClearWatchTarget()
{
	_watchTarget = NULL;
	_explicitSet = false;   
}

Entity *HeadWatcher::GetWatchTarget()
{
	return _watchTarget;
}

void HeadWatcher::HeadWatchTarget()
{
	int    tagNum;	
	Vector tagPos;	
	Vector watchPosition;
	Actor *actTarget;
	actTarget = NULL;
	
	
	tagNum = gi.Tag_NumForName( act->edict->s.modelindex, "Bip01 Head" );
	
	if ( tagNum < 0 )
		return;
	
	//Check if we even have an animation set yet
	if ( act->animate->CurrentAnim(legs) < 0 )
		return;

	act->GetTag( "Bip01 Head", &tagPos );
	
	if ( !_watchTarget || _ignoreWatchTarget )
	{
		if ( _twitchHead )
		{
			twitchHead();
			return;
		}
		else
		{
			LerpHeadBySpeed( vec_zero , false );      
			return;
		}
	}

	if (act->GetActorFlag( ACTOR_FLAG_DIALOG_PLAYING ) && act->GetActorFlag(ACTOR_FLAG_USING_HUD) )
		{
		LerpHeadBySpeed( vec_zero , false );
		return;
		}

	// Check if our _watchTarget is within distance )
	if ( _maxDistance > 0 )
		{
		Vector selfToTarget = _watchTarget->origin - act->origin;
		float dist = selfToTarget.length();

		if ( dist > _maxDistance )
			{
			LerpHeadBySpeed( vec_zero , false );
			return;
			}
		}

	if ( _watchTarget->isSubclassOf( Actor ) )
	{
		actTarget = (Actor *)(Entity *)_watchTarget;
		
		// Don't watch if the target is dead.
		if ( !actTarget->isThinkOn() )
		{
			_watchTarget = NULL;
			actTarget = NULL;
			return;
		}
	}
	
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
	
	
	AdjustHeadAngles( tagPos , watchPosition );
	
}

void HeadWatcher::AdjustHeadAngles( const Vector &tagPos , const Vector &watchPosition )
{
	Vector dir;
	Vector angles;
	Vector anglesDiff;
	float  yawChange;
	float  pitchChange;
	
	
	dir = watchPosition - tagPos;
	angles = dir.toAngles();
	
	anglesDiff = angles - act->angles;
	
	
	anglesDiff[YAW]   = AngleNormalize180( anglesDiff[YAW] );
	anglesDiff[PITCH] = AngleNormalize180( anglesDiff[PITCH] );
	
	yawChange = anglesDiff[YAW];
	pitchChange = anglesDiff[PITCH];
	
	if ( _turnThreshold && ( yawChange < _turnThreshold ) && ( yawChange > -_turnThreshold ) && ( pitchChange < _turnThreshold ) && ( pitchChange > -_turnThreshold ) )
	{
		return;
	}
	
	
	// Make sure we don't turn neck too far	
	if ( anglesDiff[YAW] < -_maxHeadYaw )
		anglesDiff[YAW] = -_maxHeadYaw;
	else if ( anglesDiff[YAW] > _maxHeadYaw )
		anglesDiff[YAW] = _maxHeadYaw;
	
	if ( anglesDiff[PITCH] < -_maxHeadPitch )
		anglesDiff[PITCH] = -_maxHeadPitch;
	else if ( anglesDiff[PITCH] > _maxHeadPitch )
		anglesDiff[PITCH] = _maxHeadPitch;
	
	anglesDiff[ROLL] = 0.0f;
	
	
	LerpHeadBySpeed( anglesDiff );   
	
}

void HeadWatcher::LerpHeadBySpeed( const Vector &angleDelta , bool useTorsoAngles )
{
	Vector anglesDiff;
	Vector change;
	Vector finalAngles;
	Vector currentTorsoAngles;
	
	anglesDiff = angleDelta;
	
	// Get our Torso Angles
	act->SetControllerTag( ACTOR_TORSO_TAG , gi.Tag_NumForName( act->edict->s.modelindex, "Bip01 Spine1" ) );
	currentTorsoAngles = act->GetControllerAngles( ACTOR_TORSO_TAG );
	
	//Reset our Controller Tag
	act->SetControllerTag( ACTOR_HEAD_TAG, gi.Tag_NumForName( act->edict->s.modelindex, "Bip01 Head" ) );
	
	
	// Make sure we don't change our head angles too much at once   
	change = anglesDiff - _currentHeadAngles;
	
	if ( change[YAW] > _maxHeadTurnSpeed )
		anglesDiff[YAW] = _currentHeadAngles[YAW] + _maxHeadTurnSpeed;
	else if ( change[YAW] < -_maxHeadTurnSpeed )
		anglesDiff[YAW] = _currentHeadAngles[YAW] - _maxHeadTurnSpeed;
	
	if ( change[PITCH] > _maxHeadTurnSpeed )
		anglesDiff[PITCH] = _currentHeadAngles[PITCH] + _maxHeadTurnSpeed;
	else if ( change[PITCH] < -_maxHeadTurnSpeed )
		anglesDiff[PITCH] = _currentHeadAngles[PITCH] - _maxHeadTurnSpeed;
	
	if ( change[ROLL] > _maxHeadTurnSpeed )
		anglesDiff[ROLL] = _currentHeadAngles[ROLL] + _maxHeadTurnSpeed;
	else if ( change[ROLL] < -_maxHeadTurnSpeed )
		anglesDiff[ROLL] = _currentHeadAngles[ROLL] - _maxHeadTurnSpeed;
	
	
	finalAngles = anglesDiff;
	
	if ( useTorsoAngles )
		finalAngles[YAW] = anglesDiff[YAW] - currentTorsoAngles[YAW];
	else
		finalAngles[YAW] = anglesDiff[YAW];
	
	act->SetControllerAngles( ACTOR_HEAD_TAG, finalAngles );
	act->real_head_pitch = anglesDiff[PITCH];
	
	_currentHeadAngles = anglesDiff;
	
}

void HeadWatcher::setHeadTwitch( bool twitchHead )
{
	_twitchHead = twitchHead;

	if ( _twitchHead )
	{
		_nextTwitchHeadTime = 0.0f;
	}
}

void HeadWatcher::twitchHead( void )
{
	Vector headAngles;
	float oldMaxHeadTurnSpeed;


	if ( level.time > _nextTwitchHeadTime )
	{
		_headTwitchAngles = Vector( G_CRandom( 4.0f ), G_CRandom( 4.0f ), G_CRandom( 4.0f ) );

		_nextTwitchHeadTime = level.time + 1.0f + G_CRandom( 0.5f );
	}

	oldMaxHeadTurnSpeed = _maxHeadTurnSpeed;

	_maxHeadTurnSpeed = 0.25f;

	LerpHeadBySpeed( _headTwitchAngles, false );

	_maxHeadTurnSpeed = oldMaxHeadTurnSpeed;
}


//
// Name:        DoArchive()
// Parameters:  Archiver &arc
//              Actor *actor
// Description: Sets the Actor Pointer and Calls Archive()
//
void HeadWatcher::DoArchive( Archiver &arc , Actor *actor )
{
	Archive( arc );
	if ( actor )
		act = actor;
	else
		gi.Error( ERR_FATAL, "HeadWatcher::DoArchive -- actor is NULL" );	
	
}

//
// Name:        Archive()
// Parameters:  Archiver &arc
// Description: Archives Class Data
//
void HeadWatcher::Archive( Archiver &arc )
{
	arc.ArchiveSafePointer( &_watchTarget );
	arc.ArchiveVector( &_currentHeadAngles );
	arc.ArchiveFloat( &_maxHeadTurnSpeed );
	arc.ArchiveFloat( &_turnThreshold );
	arc.ArchiveBoolean( &_explicitSet );
	arc.ArchiveFloat( &_maxHeadYaw );
	arc.ArchiveFloat( &_maxHeadPitch );
	arc.ArchiveBool( &_twitchHead );
	arc.ArchiveFloat( &_nextTwitchHeadTime );
	arc.ArchiveVector( &_headTwitchAngles );
	arc.ArchiveFloat( &_maxDistance );
	arc.ArchiveBool( &_ignoreWatchTarget );
}
