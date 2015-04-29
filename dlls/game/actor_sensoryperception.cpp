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
  auto startPos = act->origin;
	startPos.z += 15;
	
	auto endPos = position;
	endPos.z += 15;
	
	Vector modMins;
	Vector modMaxs;
	
	modMins = act->mins;
	modMins *= 1.5;
	
	modMaxs = act->maxs;
	modMaxs *= 1.5;
	
	auto trace = G_Trace( startPos, modMins, modMaxs, endPos, act, act->edict->clipmask, false, "isInLineOfSight" );
	//G_DebugLine( startPos , trace.endpos, 1.0f, 0.0f, 1.0f, 1.0f );
	
	_lineOfSight.entNum = entNum;
	_lineOfSight.time   = level.time;
	
	if ( trace.entityNum == entNum || entNum == ENTITYNUM_NONE )
	{
		_lineOfSight.inLineOfSight = true;      
	}
	else
	{
		auto traceEnt = G_GetEntity( trace.entityNum );
		_lineOfSight.inLineOfSight = false;
		
		if ( traceEnt && traceEnt->isSubclassOf( Actor) )
		{
			Actor *traceActor;
			traceActor = dynamic_cast<Actor*>(traceEnt);
			
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
  
  return _lineOfSight.inLineOfSight;
}

void SensoryPerception::_init()
{
	
	// Set up Stimuli
	_stimuli           = StimuliAll;
	_permanent_stimuli = StimuliAll;
	
	//Member Vars
	_noise_position  = vec_zero;
	_noise_time      = 0;
	_fov             = DEFAULT_FOV;
	_fovdot          = float(cos( double(_fov) * 0.5 * M_PI / 180.0 ));
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
	if ( _nextSenseTime > level.time )
		return;
	
	_nextSenseTime = level.time + 0.5f + G_Random( 0.2f );
	//_nextSenseTime = 0;
	
	//Well, who your enemies are depends on what side your on
	if ( act->actortype == IS_ENEMY || act->actortype == IS_MONSTER )
	{
		//
		//If we an ENEMY or a MONSTER than teammates are our enemies
		//
		Sentient *teammate;
		for ( auto i = 1 ; i <= TeamMateList.NumObjects() ; i++ )
		{
			teammate = TeamMateList.ObjectAt( i );
			
			if ( _SenseEntity( teammate ) ) return;				
			
		}
		
	}
	else
	{
		//
		//If we an CIVILIAN, FRIEND, or TEAMMATE our potential enemies are active monsters
		//
		for (auto i = 1 ; i <= ActiveList.NumObjects() ; i++ )
		{
      _SenseEntity(ActiveList.ObjectAt(i));
		}
		
		//In case we didn't find an enemy, but if the players nearby, we want to wake up anyway
		_SenseEntity( GetPlayer( 0 ) );
	}	
	
}

qboolean SensoryPerception::_SenseEntity( Entity *ent )
{
	// Don't want to target the enemy if he's not a valid target
	
	if (!ent)
		return false;
	
	if ( !EntityIsValidTarget(ent) )
		return false;
	
	// Don't wake ourselves up
	if ( ent->entnum == act->entnum )
		return false;
	
	if ( ent->isSubclassOf ( Actor ) )
	{
		if ( !ent->isThinkOn() )
			return false;
	}
	
	if ( gi.inPVS( ent->centroid, act->centroid ) )
	{
		auto enemyToSelf = act->origin - ent->origin;	
		auto visionDistance = GetVisionDistance();

		if ( enemyToSelf.length() < visionDistance )
		{
			if ( CanSeeEntity( act, ent, true, true ) )
				Stimuli( StimuliSight , ent );
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
		RespondTo(StimuliSight , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "sound" ) )
		RespondTo(StimuliSound , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "pain" ) )
		RespondTo(StimuliPain , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "script" ) )
		RespondTo(StimuliScript , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "all" ) )
		RespondTo(StimuliAll , respond );
	else if ( !Q_stricmp( stimuli_name.c_str() , "none" ) )
		RespondTo(StimuliNone , respond );
}

//
// Name:        RespondTo()
// Parameters:  int stimuli  -- the stimuli
//              qboolean respond -- Respond or Not to the stimuli
// Description: sets if the actor will respond to a particular stimulus
//
void SensoryPerception::RespondTo( int stimuli , qboolean respond )
{
	if ( stimuli == StimuliAll )
	{
    _stimuli = respond ? StimuliAll : StimuliNone;
		return;
	}
	
	if ( stimuli == StimuliNone )
	{
    _stimuli = respond ? StimuliNone : StimuliAll;
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
			_permanent_stimuli |= StimuliSight;
		else
			_permanent_stimuli &= ~StimuliSight;
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "sound" ) )
	{
		if ( respond )
			_permanent_stimuli |= StimuliSound;
		else
			_permanent_stimuli &= ~StimuliSound;		
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "pain" ) )
	{
		if ( respond )
			_permanent_stimuli |= StimuliPain;
		else
			_permanent_stimuli &= ~StimuliPain;		
		
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "script" ) )
	{
		if ( respond )
			_permanent_stimuli |= StimuliScript;
		else
			_permanent_stimuli &= ~StimuliScript;		
		
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "all" ) )
	{
		if ( respond )
			_permanent_stimuli = StimuliAll;
		else
			_permanent_stimuli = StimuliNone;		
		
	}
	else if ( !Q_stricmp( stimuli_name.c_str() , "none" ) )
	{
		if ( respond )
			_permanent_stimuli = StimuliNone;
		else
			_permanent_stimuli = StimuliAll;
	}
}

//
// Name:        ShouldRespondToStimuli()
// Parameters:  int new_stimuli -- the stimuli to check
// Description: Checks if the actor should respond to the stimuli
//
qboolean SensoryPerception::ShouldRespondToStimuli( int new_stimuli )
{
	if( act->targetType == ATTACK_SCRIPTED_ONLY && new_stimuli != StimuliScript ) return false;
	
	if ( _stimuli == StimuliAll )
		return true;
	
	if ( _stimuli == StimuliNone )
		return false;
	
	return new_stimuli & _stimuli && new_stimuli & _permanent_stimuli || new_stimuli == StimuliScript;
}


//
// Name:        ShowInfo()
// Parameters:  None
// Description: Prints sensory information to the console
//
void SensoryPerception::ShowInfo()
{
	if ( ShouldRespondToStimuli( StimuliAll ) )
	{
		gi.Printf( "Actor is Responding To: ALL" );
		return;
	}
	
	if ( ShouldRespondToStimuli( StimuliNone ) )
	{
		gi.Printf( "Actor is Responding To: NONE" );
		return;
	}
	
	if ( ShouldRespondToStimuli( StimuliSight ) )
		gi.Printf( "Actor is Responding To: SIGHT" );
	
	if ( ShouldRespondToStimuli( StimuliSound ) )
		gi.Printf( "Actor is Responding To: SOUND" );
	
	if ( ShouldRespondToStimuli( StimuliPain ) )
		gi.Printf( "Actor is Responding To: PAIN" );
	
	if ( ShouldRespondToStimuli( StimuliScript ) )
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
	
	
	// Use whichever is less : the actor's vision distance or the distance of the far plane (fog)
	if ( world->farplane_distance != 0.0f && world->farplane_distance < _vision_distance )
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
	
	// Use whichever is less : the actor's vision distance or the distance of the far plane (fog)
	if ( world->farplane_distance != 0.0f && world->farplane_distance < _vision_distance )
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
	if ( !act )
		gi.Error( ERR_DROP, "SensoryPerception::InFOV -- actor is NULL" );
	
	if ( check_fov == 360.0f )
		return true;

	auto delta = pos - act->EyePosition();
	
	if ( !delta.x && !delta.y )
	{
		// special case for straight up and down
		return true;
	}
	
	// give better vertical vision
	delta.z = 0;
	
	delta.normalize();
	
	auto tagNum = gi.Tag_NumForName( act->edict->s.modelindex, "tag_eyes" );
	
  float	dot;
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
	
	return dot > check_fovdot;
	
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
	if ( !ShouldRespondToStimuli( StimuliSight ) )
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
	auto p = target->centroid;
	auto eyePos = vec_zero;
	
	// If the start entity is an actor, then we want to add in the eyeposition
	if ( start->isSubclassOf ( Actor ) )
	{
		Actor *a;
		a = dynamic_cast<Actor*>(start);
		
		if ( !a )
			return false;
		
		eyePos = a->EyePosition();
		
	}
	
	// Check if he's visible
	auto trace = G_Trace( eyePos, vec_zero, vec_zero, p, target, MASK_OPAQUE, false, "SensoryPerception::CanSeeEntity" );
	
	if ( trace.fraction == 1.0f || trace.ent == target->edict )
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
	// Check for NULL
	if ( !target )
		return false;
	
	// Check if This Actor can even see at all
	if ( !ShouldRespondToStimuli( StimuliSight ) )
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
	auto realStart = start;	
	auto p = target->centroid;
	
	// Add in the eye offset
	
	auto eyePos = act->EyePosition() - act->origin;
	realStart += eyePos;
	
	// Check if he's visible
	auto trace = G_Trace( realStart, vec_zero, vec_zero, p, act, MASK_OPAQUE, false, "SensoryPerception::CanSeeEntity" );
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
	if ( !ShouldRespondToStimuli( StimuliSight ) )
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
	if ( !ShouldRespondToStimuli( StimuliSight ) )
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
	if ( !ShouldRespondToStimuli( StimuliSight ) )
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
	// Check if he's visible
	auto trace = G_Trace( start, vec_zero, vec_zero, position, act, MASK_OPAQUE, false, "SensoryPerception::CanSeeEntity" );
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
	if ( !target )
		return false;
	
	auto d = target->centroid - start;
	d.z = 0;
	d.normalize();
	
  Vector	p1;
  Vector	p2;
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
