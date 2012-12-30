//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actor_sensoryperception.cpp                    $
// $Revision:: 21                                                             $
//   $Author:: Sketcher                                                       $
//     $Date:: 4/09/03 5:04p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//

#include "_pch_cpp.h"
#include "actor_sensoryperception.h"
#include "player.h"
#include "object.h"

//======================================
// SensoryPerception Implementation
//======================================

//
// Name:        SensoryPerception()
// Parameters:  None
// Description: Constructor
//
SensoryPerception::SensoryPerception()
{
	// Should always use other constructor
	gi.Error( ERR_FATAL, "SensoryPerception::SensoryPerception -- Default Constructor Called" );	
}

//
// Name:        SensoryPerception()
// Parameters:  Actor* actor -- The actor pointer to be assigned
//                              to the internal actor pointer
// Description: Constructor
//
SensoryPerception::SensoryPerception(Actor *actor )
{
	//Initialize our Actor
	if ( actor )
		act = actor;
	else 
		gi.Error( ERR_FATAL, "SensoryPerception::SensoryPerception -- actor is NULL" );
	
	_init();
}

//
// Name:        ~SensoryPerception()
// Parameters:  None
// Description: Destructor
//
SensoryPerception::~SensoryPerception()
{
	
}

qboolean SensoryPerception::isInLineOfSight( const Vector &position , const int entNum )
{
	trace_t trace;
	Vector startPos;
	Vector endPos;
	Entity *traceEnt;
	
	
	startPos = act->origin;
	startPos.z += 15;
	
	endPos = position;
	endPos.z += 15;
	
	Vector modMins;
	Vector modMaxs;
	
	modMins = act->mins;
	modMins *= 1.5;
	
	modMaxs = act->maxs;
	modMaxs *= 1.5;
	
	trace = G_Trace( startPos, modMins, modMaxs, endPos, act, act->edict->clipmask, false, "isInLineOfSight" );
	//G_DebugLine( startPos , trace.endpos, 1.0f, 0.0f, 1.0f, 1.0f );
	
	_lineOfSight.entNum = entNum;
	_lineOfSight.time   = level.time;
	
	if ( trace.entityNum == entNum || entNum == ENTITYNUM_NONE )
	{
		_lineOfSight.inLineOfSight = true;      
	}
	else
	{
		traceEnt = G_GetEntity( trace.entityNum );
		_lineOfSight.inLineOfSight = false;
		
		if ( traceEnt && traceEnt->isSubclassOf( Actor) )
		{
			Actor *traceActor;
			traceActor = (Actor*)traceEnt;
			
			_lineOfSight.inLineOfSight = traceActor->sensoryPerception->checkInLineOfSight( position , entNum );
		}            
	}
	
	return _lineOfSight.inLineOfSight;
}

qboolean SensoryPerception::checkInLineOfSight( const Vector &position , const int entNum )
{
	float timeDelta;
	
	timeDelta = level.time - _lineOfSight.time;
	
	if (  timeDelta > .5 || entNum != _lineOfSight.entNum )
		return isInLineOfSight( position , entNum );   
	else
		return _lineOfSight.inLineOfSight;
	
}

void SensoryPerception::_init()
{
	
	// Set up Stimuli
	_stimuli           = STIMULI_ALL;
	_permanent_stimuli = STIMULI_ALL;
	
	//Member Vars
	_noise_position  = vec_zero;
	_noise_time      = 0;
	_fov             = DEFAULT_FOV;
	_fovdot          = (float)cos( (double)_fov * 0.5 * M_PI / 180.0 );
	_vision_distance = DEFAULT_VISION_DISTANCE;
	_last_soundType  = SOUNDTYPE_NONE;
	_nextSenseTime   = 0.0f;
	
	_lineOfSight.entNum = 0;
	_lineOfSight.inLineOfSight = false;
	_lineOfSight.time = 0.0f;
}

//
// Name:        SenseEnemies()
// Parameters:  None
// Description: Checks for players and teammates, to see if the actor
//              needs to wake up
//
void SensoryPerception::SenseEnemies()
{
	int		i;
   	
	if ( _nextSenseTime > level.time )
		return;
	
	_nextSenseTime = level.time + 0.5f + G_Random( 0.2f );
	//_nextSenseTime = 0;
	
	//Well, who your enemies are depends on what side your on
	if ( ( act->actortype == IS_ENEMY ) || ( act->actortype == IS_MONSTER ) )
	{
		//
		//If we an ENEMY or a MONSTER than teammates are our enemies
		//
		Sentient *teammate;
		for ( i = 1 ; i <= TeamMateList.NumObjects() ; i++ )
		{
			teammate = TeamMateList.ObjectAt( i );
			
			if ( _SenseEntity( teammate ) ) return;				
			
		}
		
	}
	else
	{
		//
		//If we an CIVILIAN, FRIEND, or TEAMMATE our potiential enemies are active monsters
		//
		Entity *ent;
		
		for ( i = 1 ; i <= ActiveList.NumObjects() ; i++ )
		{
			ent = ActiveList.ObjectAt( i );
			_SenseEntity(ent);
		}
		
		//In case we didn't find an enemy, but if the players nearby, we want to wake up anyway
		_SenseEntity( GetPlayer( 0 ) );
	}	
	
}

qboolean SensoryPerception::_SenseEntity( Entity *ent )
{
	// Dont want to target the enemy if he's not a valid target
	
	if (!ent)
		return false;
	
	if ( !EntityIsValidTarget(ent) )
		return false;
	
	// Dont wake ourselves up
	if ( ent->entnum == act->entnum )
		return false;
	
	if ( ent->isSubclassOf ( Actor ) )
	{
		if ( !ent->isThinkOn() )
			return false;
	}
	
	/*
	// Check if we're in the PVS
	if ( gi.inPVS( ent->centroid, act->centroid ) )		
	{
	// Check if we can see the enemy
	if ( CanSeeEntity ( act , ent , true , true ) )
	Stimuli( STIMULI_SIGHT , ent );
	else
	{
	// Lets not idle for teammates, just players
	if ( ent->isSubclassOf( Player ) )
	{
	// We couldn't see the enemy, but we're in the PVS, so we need to wake up and idle a bit.	
	if ( ( world->farplane_distance == 0 ) || ( Distance( ent->centroid, act->centroid ) < world->farplane_distance ) )
	{
	act->Wakeup();
	//act->ActivateAI();
	}
	
	  
		}
		else
		return false;
		}
		
		  return true;
		  }
	*/
	if ( gi.inPVS( ent->centroid, act->centroid ) )
	{
		Vector enemyToSelf;
		enemyToSelf = act->origin - ent->origin;
		
		float visionDistance = GetVisionDistance();
		if ( enemyToSelf.length() < visionDistance )
		{
			if ( CanSeeEntity( act, ent, true, true ) )
				Stimuli( STIMULI_SIGHT , ent );
		}         
		
	}
	
	
	return false;
}

//
// Name:        Stimuli()
// Parameters:  int stimuli -- The Stimuli
// Description: Wakes up the actor if it should respond to the stimuli
//
void SensoryPerception::Stimuli( int stimuli )
{
	if ( ShouldRespondToStimuli( stimuli ) )
	{
		act->Wakeup();
		act->ActivateAI();
	}
}

//
// Name:        Stimuli()
// Parameters:  int new_stimuli -- The Stimuli
//              Entity ent      -- The Entity
//              qboolean force  -- Force the actor to make the entity an enemy
// Description: Wakes up the actor if it should respond to the stimuli
//
void SensoryPerception::Stimuli( int new_stimuli, Entity *ent )
{
	if ( !ent ) 
		return;
	
	if ( ShouldRespondToStimuli( new_stimuli ) && EntityIsValidTarget(ent))
	{
		act->enemyManager->TryToAddToHateList( ent );		
		act->Wakeup();
		act->ActivateAI();
	}
	
}

//
// Name:        Stimuli()
// Parameters:  int new_stimuli   -- The Stimuli
//              const Vector &pos -- The location of the stimuli
// Description: Wakes up the actor if it should respond to the stimuli
//
void SensoryPerception::Stimuli( int new_stimuli, const Vector &pos )
{
	
	if ( ShouldRespondToStimuli( new_stimuli ) )
	{
		// Investigate the position
		
		if ( !act->GetActorFlag( ACTOR_FLAG_NOISE_HEARD ) )
		{
			_noise_position = pos;
			_noise_time = level.time;
			act->SetActorFlag( ACTOR_FLAG_NOISE_HEARD, true );
		}
		
		if ( act->last_time_active + 10.0f < level.time )
		{
			act->Wakeup();
			act->ActivateAI();
		}
	}
	
}

//
// Name:        Stimuli()
// Parameters:  int new_stimuli   -- The stimuli
//              const Vector &pos -- The location of the stimuli
//              int sound_Type    -- sound type of the stimuli
// Description: Wakes up the actor if it should respond to the stimuli
//
void SensoryPerception::Stimuli( int new_stimuli, const Vector &pos, int sound_Type )
{
	
	if ( ShouldRespondToStimuli( new_stimuli ) )
	{
		
		if ( !act->GetActorFlag( ACTOR_FLAG_NOISE_HEARD ) )
		{
			_noise_position = pos;
			_last_soundType = sound_Type;	
			_noise_time = level.time;
			act->SetActorFlag( ACTOR_FLAG_NOISE_HEARD, true );
		}
		
		if ( act->last_time_active + 10.0f < level.time )
		{
			act->Wakeup();
			act->ActivateAI();
		}
	}
}

//
// Name:        RespondTo()
// Parameters:  const str &stimuli_name -- string name for the stimuli
//              qboolean respond -- Respond or Not to the stimuli
// Description: sets if the actor will respond to a particular stimulus
//
void SensoryPerception::RespondTo(const str &stimuli_name , qboolean respond )
{
	if ( !Q_stricmp( stimuli_name.c_str() , "sight" ) )
		RespondTo(STIMULI_SIGHT , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "sound" ) )
		RespondTo(STIMULI_SOUND , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "pain" ) )
		RespondTo(STIMULI_PAIN , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "script" ) )
		RespondTo(STIMULI_SCRIPT , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "all" ) )
		RespondTo(STIMULI_ALL , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "none" ) )
		RespondTo(STIMULI_NONE , respond );
}

//
// Name:        RespondTo()
// Parameters:  int stimuli  -- the stimuli
//              qboolean respond -- Respond or Not to the stimuli
// Description: sets if the actor will respond to a particular stimulus
//
void SensoryPerception::RespondTo( int stimuli , qboolean respond )
{
	if ( stimuli == STIMULI_ALL )
	{
		if ( respond )
			_stimuli = STIMULI_ALL;
		else
			_stimuli = STIMULI_NONE;
		
		return;
	}
	
	if ( stimuli == STIMULI_NONE )
	{
		if ( respond )
			_stimuli = STIMULI_NONE;
		else
			_stimuli = STIMULI_ALL;
		
		return;
	}
	
	if ( respond )
		_stimuli |= stimuli;
	else
		_stimuli &= ~stimuli;
}

//
// Name:        PermanentlyRespondTo
// Parameters:  const str &stimuli_name -- string name for the stimuli
//              qboolean respond -- Respond or Not to the stimuli
// Description: Allows the actor to ALWAYS respond or not to a particular stimulus
//              this allows us to make actors that are deaf or blind... etc
//
void SensoryPerception::PermanentlyRespondTo(const str &stimuli_name , qboolean respond )
{
	if ( !Q_stricmp( stimuli_name.c_str() , "sight" ) )
	{
		if ( respond )
			_permanent_stimuli |= STIMULI_SIGHT;
		else
			_permanent_stimuli &= ~STIMULI_SIGHT;
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "sound" ) )
	{
		if ( respond )
			_permanent_stimuli |= STIMULI_SOUND;
		else
			_permanent_stimuli &= ~STIMULI_SOUND;		
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "pain" ) )
	{
		if ( respond )
			_permanent_stimuli |= STIMULI_PAIN;
		else
			_permanent_stimuli &= ~STIMULI_PAIN;		
		
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "script" ) )
	{
		if ( respond )
			_permanent_stimuli |= STIMULI_SCRIPT;
		else
			_permanent_stimuli &= ~STIMULI_SCRIPT;		
		
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "all" ) )
	{
		if ( respond )
			_permanent_stimuli = STIMULI_ALL;
		else
			_permanent_stimuli = STIMULI_NONE;		
		
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "none" ) )
	{
		if ( respond )
			_permanent_stimuli = STIMULI_NONE;
		else
			_permanent_stimuli = STIMULI_ALL;
	}
}

//
// Name:        ShouldRespondToStimuli()
// Parameters:  int new_stimuli -- the stimuli to check
// Description: Checks if the actor should respond to the stimuli
//
qboolean SensoryPerception::ShouldRespondToStimuli( int new_stimuli )
{
	if( ( act->targetType == ATTACK_SCRIPTED_ONLY ) && ( new_stimuli != STIMULI_SCRIPT ) ) return false;
	
	if ( _stimuli == STIMULI_ALL )
		return true;
	
	if ( _stimuli == STIMULI_NONE )
		return false;
	
	return ( (( new_stimuli & _stimuli ) && ( new_stimuli & _permanent_stimuli )) || ( new_stimuli == STIMULI_SCRIPT ) );
}


//
// Name:        ShowInfo()
// Parameters:  None
// Description: Prints sensoryinformation to the console
//
void SensoryPerception::ShowInfo()
{
	if ( ShouldRespondToStimuli( STIMULI_ALL ) )
	{
		gi.Printf( "Actor is Responding To: ALL" );
		return;
	}
	
	if ( ShouldRespondToStimuli( STIMULI_NONE ) )
	{
		gi.Printf( "Actor is Responding To: NONE" );
		return;
	}
	
	if ( ShouldRespondToStimuli( STIMULI_SIGHT ) )
		gi.Printf( "Actor is Responding To: SIGHT" );
	
	if ( ShouldRespondToStimuli( STIMULI_SOUND ) )
		gi.Printf( "Actor is Responding To: SOUND" );
	
	if ( ShouldRespondToStimuli( STIMULI_PAIN ) )
		gi.Printf( "Actor is Responding To: PAIN" );
	
	if ( ShouldRespondToStimuli( STIMULI_SCRIPT ) )
		gi.Printf( "Actor is Responding To: SCRIPT" );
	
}


//
// Name:        WithinVisionDistance()
// Parameters:  Entity *ent -- The entity to be checked
// Description: Checks if the passed in entity is 
//              within the vision_distance of the actor, or the 
//              farplane_distance of the world ( whichever is smaller )
//
qboolean SensoryPerception::WithinVisionDistance( const Entity *ent )
{
	float distance;
	
	if ( !ent )
		return false;
	
	if ( !act )
		gi.Error( ERR_DROP, "SensoryPerception::WithinVisionDistance -- actor is NULL" );
	
	
	// Use whichever is less : the actor's vision distance or the distance of the farplane (fog)
	if ( ( world->farplane_distance != 0.0f ) && ( world->farplane_distance < _vision_distance ) )
		distance = world->farplane_distance;
	else
		distance = _vision_distance;
	
	return act->WithinDistance( ent, distance );
}

qboolean SensoryPerception::WithinVisionDistance( const Vector &pos )
{
	float distance;
	
	if ( !act )
		gi.Error( ERR_DROP, "SensoryPerception::WithinVisionDistance -- actor is NULL" );
	
	// Use whichever is less : the actor's vision distance or the distance of the farplane (fog)
	if ( ( world->farplane_distance != 0.0f ) && ( world->farplane_distance < _vision_distance ) )
		distance = world->farplane_distance;
	else
		distance = _vision_distance;
	
	return act->WithinDistance( pos, distance );   
}

//
// Name:        InFOV()
// Parameters:  Vector &pos -- The position vector to be checked
//              float check_fov -- The fov number to be used
//              float check_fovdot -- the dot product
// Description: Checks if the passed in pos to see if it is in the FOV
//
qboolean SensoryPerception::InFOV( const Vector &pos, float check_fov, float check_fovdot	)
{
	Vector delta;
	float	 dot;
	Vector temp;
	int tagNum;
	
	
	if ( !act )
		gi.Error( ERR_DROP, "SensoryPerception::InFOV -- actor is NULL" );
	
	if ( check_fov == 360.0f )
		return true;
	temp = act->EyePosition();
	delta = pos - act->EyePosition();
	
	if ( !delta.x && !delta.y )
	{
		// special case for straight up and down
		return true;
	}
	
	// give better vertical vision
	delta.z = 0;
	
	delta.normalize();
	
	tagNum = gi.Tag_NumForName( act->edict->s.modelindex, "tag_eyes" );
	
	if ( tagNum >= 0 )
	{
		Vector tag_pos;
		Vector forward;
		
		act->GetTag( tagNum, &tag_pos, &forward );
		dot = DotProduct( &forward[0] , delta );
	}
	else
	{
		dot = DotProduct( act->orientation[ 0 ], delta );
	}
	
	return ( dot > check_fovdot );
	
}


//
// Name:        InFOV()
// Parameters:  Vector &pos -- The position to be checked
// Description: Calls another version of InFOV
//
qboolean SensoryPerception::InFOV( const Vector &pos )
{
	return InFOV( pos, _fov, _fovdot );
}


//
// Name:        InFOV()
// Parameters:  Entity* ent -- Provides the Position to check
// Description: Calls another version InFOV 
//
qboolean SensoryPerception::InFOV( const Entity *ent )
{
	return InFOV( ent->centroid );
}

//
// Name:        CanSeeEntity()
// Parameters:  Entity *start - The entity trying to see
//              Entity *target - The entity that needs to be seen
//              qboolean useFOV - take FOV into consideration or not
//              qboolean useVisionDistance - take visionDistance into consideration
// Description: Wraps a lot of the different CanSee Functions into one
//
qboolean SensoryPerception::CanSeeEntity( Entity *start, const Entity *target, qboolean useFOV, qboolean useVisionDistance )
{
	
	// Check for NULL
	if ( !start || !target )
		return false;
	
	// Check if This Actor can even see at all
	if ( !ShouldRespondToStimuli( STIMULI_SIGHT ) )
		return false;
	
	// Check for FOV
	if ( useFOV )
	{
		if ( !InFOV( target ) )
			return false;
	}
	
	// Check for vision distance
	if ( useVisionDistance )
	{
		if ( !WithinVisionDistance( target ) )
			return false;
	}
	
	// Do Trace
	trace_t trace;
	Vector p;
	Vector eyePos;
	
	p = target->centroid;
	eyePos = vec_zero;
	
	// If the start entity is an actor, then we want to add in the eyeposition
	if ( start->isSubclassOf ( Actor ) )
	{
		Actor *a;
		a = (Actor*)start;
		
		if ( !a )
			return false;
		
		eyePos = a->EyePosition();
		
	}
	
	// Check if he's visible
	trace = G_Trace( eyePos, vec_zero, vec_zero, p, target, MASK_OPAQUE, false, "SensoryPerception::CanSeeEntity" );

	//if ( act->actortype == IS_TEAMMATE )
	//	{
	//	G_DebugLine( eyePos , target->centroid, 0.0f, 0.0f, 1.0f, 1.0f );	
	//	G_DebugLine( eyePos , trace.endpos , 1.0f, 0.0f, 1.0f, 1.0f );	
	//	}
	
	if ( ( trace.fraction == 1.0f ) || ( trace.ent == target->edict ) )
		return true;
	
	// Check if his head is visible
	p.z = target->absmax.z;
	trace = G_Trace( eyePos, vec_zero, vec_zero, p, target, MASK_OPAQUE, false, "SensoryPerception::CanSeeEntity" );
	if ( trace.fraction == 1.0f || trace.ent == target->edict )
		return true;			
	
	return false;
	
}


//
// Name:        CanSeeEntity()
// Parameters:  Vector &start -- The starting position
//              Entity *target - The entity that needs to be seen
//              qboolean useFOV - take FOV into consideration or not
//              qboolean useVisionDistance - take visionDistance into consideration
// Description: Wraps a lot of the different CanSee Functions into one
//
qboolean SensoryPerception::CanSeeEntity( const Vector &start , const Entity *target, qboolean useFOV , qboolean useVisionDistance )
{
	Vector realStart;
	
	// Check for NULL
	if ( !target )
		return false;
	
	// Check if This Actor can even see at all
	if ( !ShouldRespondToStimuli( STIMULI_SIGHT ) )
		return false;
	
	
	// Check for FOV
	if ( useFOV )
	{
		if ( !InFOV( target ) )
			return false;
	}
	
	// Check for vision distance
	if ( useVisionDistance )
	{
		if ( !WithinVisionDistance( target ) )
			return false;
	}
	
	// Do Trace
	trace_t trace;
	Vector p;
	Vector eyePos;
	
	realStart = start;
	
	p = target->centroid;
	
	// Add in the eye offset
	
	eyePos = act->EyePosition() - act->origin;
	realStart += eyePos;
	
	// Check if he's visible
	trace = G_Trace( realStart, vec_zero, vec_zero, p, act, MASK_OPAQUE, false, "SensoryPerception::CanSeeEntity" );
	if ( trace.fraction == 1.0f || trace.ent == target->edict )
		return true;
	
	// Check if his head is visible
	p.z = target->absmax.z;
	trace = G_Trace( realStart, vec_zero, vec_zero, p, act, MASK_OPAQUE, false, "SensoryPerception::CanSeeEntity" );
	if ( trace.fraction == 1.0f || trace.ent == target->edict )
		return true;			
	
	
	return false;
}


//
// Name:        CanSeeEntityComplex
// Parameters:  Entity *start - The entity trying to see
//              Entity *target - The entity that needs to be seen
//              qboolean useFOV - take FOV into consideration or not
//              qboolean useVisionDistance - take visionDistance into consideration
// Description: More detailed can see check
//
qboolean SensoryPerception::CanSeeEntityComplex(Entity *start, Entity *target, qboolean useFOV, qboolean useVisionDistance )
{
	
	// Check for NULL
	if ( !start || !target )
		return false;
	
	// Check if This Actor can even see at all
	if ( !ShouldRespondToStimuli( STIMULI_SIGHT ) )
		return false;
	
	if ( !act )
		gi.Error( ERR_DROP, "SensoryPerception::CanSeeEntityComplex -- actor is NULL" );
	
	// Check if target is alive
	if ( !act->IsEntityAlive( target ) )
		return false;
	
	return _CanSeeComplex(start->centroid, target, useFOV, useVisionDistance );
	
}



//
// Name:        CanSeeEntityComplex
// Parameters:  Entity *start - The entity trying to see
//              Entity *target - The entity that needs to be seen
//              qboolean useFOV - take FOV into consideration or not
//              qboolean useVisionDistance - take visionDistance into consideration
// Description: More detailed can see check
//
qboolean SensoryPerception::CanSeeEntityComplex( Vector &start, Entity *target, qboolean useFOV , qboolean useVisionDistance )
{
	// Check for NULL
	if ( !target )
		return false;
	
	// Check if This Actor can even see at all
	if ( !ShouldRespondToStimuli( STIMULI_SIGHT ) )
		return false;
	
	if ( !act )
		gi.Error( ERR_DROP, "SensoryPerception::CanSeeEntityComplex -- actor is NULL" );
	
	
	// Check if target is alive
	if ( !act->IsEntityAlive( target ) )
		return false;
	
	return _CanSeeComplex(start, target, useFOV, useVisionDistance );
	
}

qboolean SensoryPerception::CanSeePosition( const Vector &start, const Vector &position, qboolean useFOV , qboolean useVisionDistance )
{
	
	// Check if This Actor can even see at all
	if ( !ShouldRespondToStimuli( STIMULI_SIGHT ) )
		return false;
	
	// Check for FOV
	if ( useFOV )
	{
		if ( !InFOV( position ) )
			return false;
	}
	
	// Check for vision distance
	if ( useVisionDistance )
	{
		if ( !WithinVisionDistance( position ) )
			return false;
	}
	
	// Do Trace
	trace_t trace;
	Vector eyePos;
	
	eyePos = vec_zero;
	
	// Check if he's visible
	trace = G_Trace( start, vec_zero, vec_zero, position, act, MASK_OPAQUE, false, "SensoryPerception::CanSeeEntity" );
	if ( trace.fraction == 1.0f )
		return true;
	
	return false;
}

//
// Name:        _CanSeeComplex
// Parameters:  Vector &start - start position
//              Vector &end   - end position
//              qboolean useFOV - take FOV into consideration or not
//              qboolean useVisionDistance             
// Description: Creates a plane to do a more complex check of vision
//
qboolean SensoryPerception::_CanSeeComplex( Vector &start, Entity *target , qboolean useFOV, qboolean useVisionDistance )
{
	Vector	d;
	Vector	p1;
	Vector	p2;
	
	if ( !target )
		return false;
	
	d = target->centroid - start;
	d.z = 0;
	d.normalize();
	
	p1.x = -d.y;
	p1.y = d.x;
	p1 *= max( act->size.x, act->size.y ) * 1.44f * 0.5f;
	p2 = p1;
	
	p1.z = act->mins.z;
	p2.z = act->maxs.z;	
	if ( CanSeeEntity( start + p1, target , useFOV , useVisionDistance ) )
	{
		return true;
	}
	if ( CanSeeEntity( start + p2, target , useFOV , useVisionDistance ) )
	{
		return true;
	}
	
	p1.z = -p1.z;
	p2.z = -p2.z;
	if ( CanSeeEntity( start - p1, target , useFOV , useVisionDistance ) )
	{
		return true;
	}
	if ( CanSeeEntity( start - p2, target , useFOV , useVisionDistance ) )
	{
		return true;
	}
	
	return false;
}

//
// Name:        SetNoisePosition()
// Parameters:  Vector pos
// Description: Sets the _noise_position vector
//
void SensoryPerception::SetNoisePosition( const Vector &pos )
{
	_noise_position = pos;
}

//
// Name:        GetNoisePosition()
// Parameters:  None
// Description: Returns _noise_position
//
Vector SensoryPerception::GetNoisePosition()
{
	return _noise_position;
}

//
// Name:       SetLastSoundType()
// Parameters: int soundtype
// Description: sets _last_soundType
//
void SensoryPerception::SetLastSoundType( int soundtype )
{
	_last_soundType = soundtype;
}

//
// Name:       GetLastSoundType()
// Parameters: None
// Description: returns _last_soundType
//
int SensoryPerception::GetLastSoundType()
{
	return _last_soundType;
}

//
// Name:        SetNoiseTime()
// Parameters:  float noisetime
// Description: sets _noise_time
//
void SensoryPerception::SetNoiseTime( float noisetime )
{
	_noise_time = noisetime;
}

//
// Name:       GetNoiseTime()
// Parameters: None
// Description: returns _noise_time
//
float SensoryPerception::GetNoiseTime()
{
	return _noise_time;
}

//
// Name:       SetFOV()
// Parameters: float fov
// Description: sets _fov
//
void SensoryPerception::SetFOV( float fov )
{
	_fov = fov;
}

//
// Name:       GetFOV()
// Parameters: None
// Description: returns _fov
//
float SensoryPerception::GetFOV()
{
	return _fov;
}

//
// Name:        setFOVdot
// Parameters:  float fov_dot
// Description: _fovdot
//
void SensoryPerception::SetFOVdot( float fov_dot )
{
	_fovdot = fov_dot;
}

//
// Name:        GetFOVdot()
// Parameters:  none
// Description: returns _fovdot
//
float SensoryPerception::GetFOVdot()
{
	return _fovdot;
}

//
// Name:       SetVisionDistance()
// Parameters: float vision_distance
// Description: Sets _vision_distance
//
void SensoryPerception::SetVisionDistance( float vision_distance )
{
	_vision_distance = vision_distance;
}

//
// Name:        GetVisionDistance()
// Parameters:  None
// Description: returns _vision_distance
//
float SensoryPerception::GetVisionDistance()
{
	return _vision_distance;
}

//
// Name:       DoArchive
// Parameters: Archiver &arc -- The archiver object
//             Actor *actor  -- The actor
// Description: Sets the act pointer and calls Archive
//
void SensoryPerception::DoArchive( Archiver &arc , Actor *actor )
{
	Archive( arc );
	if ( actor )
		act = actor;
	else
		gi.Error( ERR_FATAL, "SensoryPerception::DoArchive -- actor is NULL" );
}

//
// Name:        Archive()
// Parameters:  Archiver &arc -- The archiver object
// Description: Archives the class
//
void SensoryPerception::Archive ( Archiver &arc )
{	
	arc.ArchiveInteger ( &_stimuli                   );
	arc.ArchiveInteger ( &_permanent_stimuli         );
	arc.ArchiveVector  ( &_noise_position            );
	arc.ArchiveInteger ( &_last_soundType            );
	arc.ArchiveFloat   ( &_noise_time                );
	arc.ArchiveFloat   ( &_fov                       );
	arc.ArchiveFloat   ( &_fovdot                    );
	arc.ArchiveFloat   ( &_vision_distance           );
	arc.ArchiveFloat   ( &_nextSenseTime	          );
	
	arc.ArchiveInteger( &_lineOfSight.entNum );
	arc.ArchiveFloat( &_lineOfSight.time );
	arc.ArchiveBoolean( &_lineOfSight.inLineOfSight );
}
