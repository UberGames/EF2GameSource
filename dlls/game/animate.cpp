//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/animate.cpp                               $
// $Revision:: 23                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:35p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION: Animate Class
//

#include "_pch_cpp.h"
#include "animate.h"
#include "player.h"
#include <qcommon/qfiles.h>

// Leg Animation events
Event EV_Anim
(
	"anim",
	EV_DEFAULT,
	"s",
	"animName",
	"set the legs animation to animName."
);
Event EV_SetFrame
(
	"setframe",
	EV_CODEONLY,
	"iS",
	"frameNumber animName",
	"Set the frame on the legs, if anim is not specified, current is assumed."
);
Event EV_AnimDone
(
	"animdone",
	EV_CODEONLY,
	NULL,
	NULL,
	"Legs animation has finished, not for script use."
);
Event EV_FrameDelta
(
	"setmovedelta",
	EV_CODEONLY,
	"v",
	"moveDelta",
	"movement from animation, not for script use."
);
Event EV_StopAnimating
(
	"stopanimating",
	EV_CODEONLY,
	NULL,
	NULL,
	"stop the legs from animating.  Animation will end at the end of current cycle."
);

// Torso Animation events
Event EV_Torso_Anim
(
	"torso_anim",
	EV_CODEONLY,
	"s",
	"animName",
	"set the torso animation to animName."
);
Event EV_Torso_SetFrame
(
	"torso_setframe",
	EV_CODEONLY,
	"iS",
	"frameNumber animName",
	"Set the frame on the torso, if anim is not specified, current is assumed."
);
Event EV_Torso_AnimDone
(
	"torso_animdone",
	EV_CODEONLY,
	NULL,
	NULL,
	"Torso animation has finished, not for script use."
);
Event EV_Torso_StopAnimating
(
	"torso_stopanimating",
	EV_CODEONLY,
	NULL,
	NULL,
	"stop the torso from animating.  Animation will end at the end of current cycle."
);
Event EV_NewAnim
(
	"animate_newanim",
	EV_CODEONLY,
	"ii",
	"animNum bodyPart",
	"Start a new animation, not for script use."
);

CLASS_DECLARATION( Listener, Animate, "animate" )
{
	{ &EV_Anim,						&Animate::Legs_AnimEvent },
	{ &EV_SetFrame,					&Animate::Legs_SetFrameEvent },
	{ &EV_AnimDone,					&Animate::Legs_AnimDoneEvent },
	{ &EV_StopAnimating,			&Animate::Legs_StopAnimating },
	{ &EV_FrameDelta,				&Animate::FrameDeltaEvent },
	{ &EV_NewAnim,					&Animate::NewAnimEvent },	
	{ &EV_Torso_Anim,				&Animate::Torso_AnimEvent },
	{ &EV_Torso_SetFrame,			&Animate::Torso_SetFrameEvent },
	{ &EV_Torso_AnimDone,			&Animate::Torso_AnimDoneEvent },
	{ &EV_Torso_StopAnimating,		&Animate::Torso_StopAnimating },
	
	{ NULL, NULL }
};

Animate::Animate()
{
	// Should always use other constructor
	
	assert( 0 );
}

Animate::Animate( Entity *ent )
{
	// Animation variables
	frame_delta    = Vector(0, 0, 0);
	animDoneEvent	= NULL;
	torso_animDoneEvent	= NULL;
	
	legs_animtime = 0;
	torso_animtime = 0;
	
	legs_starttime = 0;
	torso_starttime = 0;
	
	legs_numframes = 0;
	torso_numframes = 0;
	
	legs_frametime = 0;
	torso_frametime = 0;
	
	self = ent;
	
	if ( self )
	{
		self->edict->s.animationRate = 1.0f;
		self->edict->s.anim        |= (ANIM_SERVER_EXITCOMMANDS_PROCESSED);
		self->edict->s.torso_anim  |= (ANIM_SERVER_EXITCOMMANDS_PROCESSED);
		self->edict->s.effectsAnims[ 0 ] = -1 ;
		self->edict->s.effectsAnims[ 1 ] = -1 ;
		self->edict->s.effectsAnims[ 2 ] = -1 ;
		self->edict->s.effectsAnims[ 3 ] = -1 ;
	}
}

Animate::~Animate()
{
	if ( animDoneEvent )
	{
		delete animDoneEvent;
		animDoneEvent = NULL;
	}

	if ( torso_animDoneEvent )
	{
		delete torso_animDoneEvent;
		torso_animDoneEvent = NULL;
	}
}

void Animate::NewAnim( int animnum, bodypart_t part )
{
	//
	// this is an animating model, we set this here so that non-animating models
	// don't have to go through the animating code path
	//
	if ( self->edict->s.eType == ET_GENERAL )
	{
		self->edict->s.eType = ET_MODELANIM;
	}
	
	if ( LoadingSavegame )
	{
		// if we are loading a game, we don't need to start animating, that will be automatic
		// all these events would mess everything up anyway
		return;
	}
	
	
	int   *anim;
	int   flags;
	float *animtime;
	float *starttime;
	float *frametime;
	int   *numframes;
	switch( part )
	{
	case legs:
		anim = &self->edict->s.anim;
		self->edict->s.frame &= ~FRAME_EXPLICIT;
		flags = EVENT_LEGS_ANIM;
		animtime = &legs_animtime;
		starttime = &legs_starttime;
		frametime = &legs_frametime;
		numframes = &legs_numframes;
		break;
	case torso:
		anim = &self->edict->s.torso_anim;
		self->edict->s.torso_frame &= ~FRAME_EXPLICIT;
		flags = EVENT_TORSO_ANIM;
		animtime = &torso_animtime;
		starttime = &torso_starttime;
		frametime = &torso_frametime;
		numframes = &torso_numframes;
		break;
	default:
		warning( "NewAnim", "Unknown body part %d", part );
		return;
		break;
	}
	
	int last_anim = *anim & ANIM_MASK;
	int last_anim_flags = *anim;
	
	float lastStartTime = *starttime;
	
	//
	// if the animations were different we need to process the entry and exit events
	//
	if ( ( last_anim != animnum ) && !( last_anim_flags & ANIM_SERVER_EXITCOMMANDS_PROCESSED ) )
	{
		// exit the previous animation
		tiki_cmd_t cmds;
		if ( gi.Frame_Commands( self->edict->s.modelindex, last_anim, TIKI_FRAME_CMD_EXIT, &cmds ) )
		{
			for( int ii = 0; ii < cmds.num_cmds; ii++ )
            {
				Event *ev = new Event( cmds.cmds[ ii ].args[ 0 ] );
				
				ev->SetSource( EV_FROM_ANIMATION );
				ev->SetAnimationNumber( last_anim );
				ev->SetAnimationFrame( 0 );
				
				for( int j = 1; j < cmds.cmds[ ii ].num_args; j++ )
				{
					ev->AddToken( cmds.cmds[ ii ].args[ j ] );
				}
				self->ProcessEvent( ev );
            }
		}
	}
	
	if ( ( animnum >= 0 ) && ( animnum < gi.NumAnims( self->edict->s.modelindex ) ) )
	{
		if ( *starttime == level.time )
		{
			// don't toggle the togglebit if we've already had an animation set this frame
			*anim = ( *anim & ANIM_TOGGLEBIT ) | animnum | ANIM_BLEND;
		}
		else
		{
			*anim = ( ( *anim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | animnum | ANIM_BLEND;
		}
	}
	else
	{
		// bad value
		return;
	}
	
	// get rid of old anim events
	CancelFlaggedEvents( flags );
	self->CancelFlaggedEvents( flags );
	
	// get total time of animation
	float totaltime = gi.Anim_Time( self->edict->s.modelindex, animnum );
	totaltime /= self->edict->s.animationRate;
	
	// set the total time for the animation
	*animtime = totaltime;
	
	// set the start time of the animation
	*starttime = level.time;
	
	// set the number of frames for the animation
	*numframes = gi.Anim_NumFrames( self->edict->s.modelindex, animnum );
	
	// set the time for each animation frame
	*frametime = gi.Frame_Time( self->edict->s.modelindex, animnum, 0 );
	*frametime /= self->edict->s.animationRate;
	
	bool has_commands = (gi.Anim_HasCommands( self->edict->s.modelindex, animnum ) != 0);
	
	if ( self->edict->s.eFlags & EF_DONT_PROCESS_COMMANDS )
		has_commands = false;
	
	if ( ( ( last_anim != animnum ) || ( lastStartTime == 0 ) ) && ( has_commands ) )
	{
		ClearAllEffectAnims();
		// enter this animation
		tiki_cmd_t cmds;
		if ( gi.Frame_Commands( self->edict->s.modelindex, animnum, TIKI_FRAME_CMD_ENTRY, &cmds ) )
		{
			for( int ii = 0; ii < cmds.num_cmds; ii++ )
            {
				Event *ev = new Event( cmds.cmds[ ii ].args[ 0 ] );
				ev->SetSource( EV_FROM_ANIMATION );
				ev->SetAnimationNumber( animnum );
				ev->SetAnimationFrame( 0 );
				
				for( int j = 1; j < cmds.cmds[ ii ].num_args; j++ )
				{
					ev->AddToken( cmds.cmds[ ii ].args[ j ] );
				}
				self->ProcessEvent( ev );
            }
		}
	}
	
	
	// initially don't post move delta stuff
	bool dodelta = false;
	
	// if it is the legs, find out if there is a delta and if it is a delta_driven animation
	if ( part == legs )
	{
		Vector totaldelta;
		gi.Anim_AbsoluteDelta( self->edict->s.modelindex, animnum, totaldelta );
		
		float length = totaldelta.length();
		if ( length > MINIMUM_DELTA_MOVEMENT )
		{
			int flags = gi.Anim_Flags( self->edict->s.modelindex, animnum );
			if ( !( flags & MDL_ANIM_DELTA_DRIVEN ) )
            {
				length /= (float)*numframes;
				if ( length > MINIMUM_DELTA_MOVEMENT_PER_FRAME )
				{
					dodelta = true;
				}
            }
		}
	}
	
	if ( has_commands || dodelta )
	{
		float time = 0;
		
		for( int i = 0; i < *numframes; i++ )
		{
			if ( has_commands )
            {
				// we want normal frame commands to occur right on the frame
				tiki_cmd_t cmds;
				if ( gi.Frame_Commands( self->edict->s.modelindex, animnum, i, &cmds ) )
				{
					for( int ii = 0; ii < cmds.num_cmds; ii++ )
					{
						Event *ev = new Event( cmds.cmds[ ii ].args[ 0 ] );
						
						ev->SetSource( EV_FROM_ANIMATION );
						ev->SetAnimationNumber( animnum );
						ev->SetAnimationFrame( i );
						
						for( int j = 1; j < cmds.cmds[ ii ].num_args; j++ )
						{
							ev->AddToken( cmds.cmds[ ii ].args[ j ] );
						}
						self->PostEvent( ev, time, flags );
					}
				}
            }
			
			// add to time
			time += *frametime;
			
			// we want deltas to occur at the end of the frame
			// only add deltas on the legs
			if ( dodelta )
            {
				Vector delta;
				
				// get the current frame delta
				gi.Frame_Delta( self->edict->s.modelindex, animnum, i, delta );
				
				if ( *frametime > FRAMETIME )
				{
					float time_offset;
					
					VectorScale( delta, ( FRAMETIME / *frametime ), delta );
					
					for ( time_offset = 0; time_offset < *frametime; time_offset += FRAMETIME )
					{
						Event *ev = new Event( EV_FrameDelta );
						ev->AddVector( delta );
						PostEvent( ev, time + time_offset, flags );
					}
				}
				else
				{
					Event *ev = new Event( EV_FrameDelta );
					ev->AddVector( delta );
					ev->AddFloat( *frametime );
					PostEvent( ev, time, flags );
				}
            }
			
		}
	}
	
	//
	// if we have a 1 frame animation, which has no commands,
	// we aren't a subclass of Sentient and our animation time is the same as frametime
	// there is no reason for us to constantly animate, since nothing will change
	// lets get the hell out of dodge!
	if (
		( *numframes == 1 ) &&
		!self->isSubclassOf( Player ) &&
		!has_commands &&
		( *frametime <= FRAMETIME )
		)
	{
		switch( part )
		{
		case legs:
			if ( animDoneEvent )
			{
				self->PostEvent( *animDoneEvent, 0.0f );
			}
            break;
		case torso:
			if ( torso_animDoneEvent )
			{
				self->PostEvent( *torso_animDoneEvent, 0.0f );
			}
            break;
		default:
            break;
		}
		return;
	}
	
	// Note: Below we post the ANIMDONE on the last frame of the animation if we're switching 
	// animations.  If we're playing a looped anim, the ANIMDONE event is posted one frame early.
	// No idea why it makes a difference, but it removes hitching on the last frame of a looped anim.
	switch( part )
	{
	case legs:
		if ( totaltime > *frametime && last_anim == animnum )
            PostEvent( EV_AnimDone, totaltime - *frametime, flags );
		else
            PostEvent( EV_AnimDone, totaltime, flags );
		break;
	case torso:
		if ( totaltime > *frametime && last_anim == animnum )
            PostEvent( EV_Torso_AnimDone, totaltime - *frametime, flags );
		else
            PostEvent( EV_Torso_AnimDone, totaltime, flags );
		break;
	default:
		warning( "NewAnim", "Unknown body part %d", part );
		return;
		break;
	}
}
   
void Animate::NewAnim( int animnum, Event &newevent, bodypart_t part )
{
	SetAnimDoneEvent( newevent, part );
	NewAnim( animnum, part );
}

void Animate::NewAnim( int animnum, Event *newevent, bodypart_t part )
{
	SetAnimDoneEvent( newevent, part );
	NewAnim( animnum, part );
}

void Animate::FrameDeltaEvent( Event *ev )
{
	frame_delta = ev->GetVector( 1 );
	self->total_delta += frame_delta * self->edict->s.scale ;
}

void Animate::EndAnim( bodypart_t part )
{
	Event * ev;
	
	switch( part )
	{
	case legs:
		if ( animDoneEvent )
		      {
			self->PostEvent( *animDoneEvent, 0.0f );
		      }
		ev = new Event( EV_NewAnim );
		ev->AddInteger( self->edict->s.anim );
		ev->AddInteger( part );
		
		if ( legs_animtime > legs_frametime )
            PostEvent( ev, legs_frametime, EVENT_LEGS_ANIM );
		else
            PostEvent( ev, 0.0f, EVENT_LEGS_ANIM );
		break;
	case torso:
		if ( torso_animDoneEvent )
		      {
			self->PostEvent( *torso_animDoneEvent, 0.0f );
		      }
		ev = new Event( EV_NewAnim );
		ev->AddInteger( self->edict->s.torso_anim );
		ev->AddInteger( part );
		
		if ( torso_animtime > torso_frametime )
            PostEvent( ev, torso_frametime, EVENT_TORSO_ANIM );
		else
            PostEvent( ev, 0.0f, EVENT_TORSO_ANIM );
		break;
	default:
		warning( "EndAnim", "Unknown body part %d", part );
		return;
		break;
	}
}

void Animate::SetAnimDoneEvent( Event *event, bodypart_t part )
{
	Event **doneevent;
	
	switch( part )
	{
	case legs:
		doneevent = &animDoneEvent;
		break;
	case torso:
		doneevent = &torso_animDoneEvent;
		break;
	default:
		warning( "SetAnimDoneEvent", "Unknown body part %d", part );
		return;
		break;
	}
	if ( *doneevent )
	{
		delete *doneevent;
	}
	
	*doneevent = event;
}

void Animate::SetAnimDoneEvent( const Event &event, bodypart_t part )
{
	SetAnimDoneEvent( new Event( event ), part );
}

str Animate::GetName()
{
	return currentAnim;
}

//===============================================================
// Name:		AddEffectAnim
// Class:		Animate
//
// Description: Add an effects anim to be layered about a normal
//				anim.
// 
// Parameters:	const char* -- the anim to whose effects you want to add.
//
// Returns:		None
// 
//===============================================================
void Animate::AddEffectAnim( const char *animname )
{
	int num = gi.Anim_Random( self->edict->s.modelindex, animname );
	if ( num != -1 )
	{
		for ( int effectAnimIdx = 0; effectAnimIdx < NUM_EFFECTS_ANIMS; ++effectAnimIdx )
		{
			int effectAnimNum = self->edict->s.effectsAnims[ effectAnimIdx ] ;
			if ( effectAnimNum == num ) return ;
			if ( effectAnimNum == -1 )
			{
				self->edict->s.effectsAnims[ effectAnimIdx ] = num ;
				return ;
			}
		}
	}
}


//===============================================================
// Name:		RemoveEffectAnim
// Class:		Animate
//
// Description: Removes an anim effect
// 
// Parameters:	const char *animname
//
// Returns:		None
// 
//===============================================================
void Animate::RemoveEffectAnim( const char *animname )
{
	int num = gi.Anim_Random( self->edict->s.modelindex, animname );
	if ( num != -1 )
	{
		for ( int effectAnimIdx = 0; effectAnimIdx < NUM_EFFECTS_ANIMS; ++effectAnimIdx )
		{
			if ( self->edict->s.effectsAnims[ effectAnimIdx ] == num )
			{
				self->edict->s.effectsAnims[ effectAnimIdx ] = -1 ;
			}
		}
	}
}

//===============================================================
// Name:		ClearAllEffectAnims
// Class:		Animate
//
// Description: Clears the list of effect anims.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Animate::ClearAllEffectAnims( void )
{
	for ( int effectAnimIdx = 0; effectAnimIdx < NUM_EFFECTS_ANIMS; ++effectAnimIdx )
	{
		self->edict->s.effectsAnims[ effectAnimIdx ] = -1 ;
	}
}

void Animate::RandomAnimate( const char *animname, Event *endevent, bodypart_t part )
{
	Event *event_to_post;
	int   num;
	int   flags;
	qboolean allparts;
	
	assert( animname );
	if ( !animname )
	{
		return;
	}
	
	allparts = false;
	if ( part == all )
	{
		allparts = true;
	}
	
	do {
		if ( allparts )
		{
			switch( part )
            {
            case all:
				part = legs;
				break;
            case legs:
				part = torso;
				break;
            case torso:
				return;
				break;
            }
		}
		switch( part )
		{
		case legs:
            flags = EVENT_LEGS_ANIM;
            break;
		case torso:
            flags = EVENT_TORSO_ANIM;
            break;
		default:
            warning( "RandomAnimate", "Unknown body part %d", part );
            return;
            break;
		}
		num = gi.Anim_Random( self->edict->s.modelindex, animname );
		
		currentAnim = animname;
		
		//
		// this is here to ensure that multiple distinct events are posted for each body part
		//
		if ( allparts && ( part != legs ) )
		{
			if ( endevent )
            {
				event_to_post = new Event( endevent );
            }
			else
            {
				event_to_post = NULL;
            }
		}
		else
		{
			event_to_post = endevent;
		}
		
		//
		// see if we even have a valid animation at all
		//
		if ( num == -1 )
		{
			if ( event_to_post )
			{
				self->PostEvent( event_to_post, FRAMETIME, flags );
			}
		}
		else
		{
			SetAnimDoneEvent( event_to_post, part );
			NewAnim( num, part );
		}
	} while( allparts );
}

void Animate::SetAnimationRate( const float animationRate ) 
{ 
	oldAnimationRate = self->edict->s.animationRate; 
	self->edict->s.animationRate = animationRate;
}

void Animate::RestoreAnimationRate( void ) 
{ 
	self->edict->s.animationRate = oldAnimationRate; 
}

void Animate::SetFrame( int framenum, bodypart_t part, int anim )
{
	int   flags;
	int   numframes;
	int   *panim;
	int   *frame;
	
	switch( part )
	{
	case legs:
		frame = &self->edict->s.frame;
		flags = EVENT_LEGS_ANIM;
		numframes = legs_numframes;
		panim = &self->edict->s.anim;
		break;
	case torso:
		frame = &self->edict->s.torso_frame;
		flags = EVENT_TORSO_ANIM;
		numframes = torso_numframes;
		panim = &self->edict->s.torso_anim;
		break;
	default:
		warning( "SetFrame", "Unknown body part %d", part );
		return;
		break;
	}
	
	if ( anim >= 0 )
	{
		numframes = gi.Anim_NumFrames( self->edict->s.modelindex, anim );
	}
	
	if ( framenum < 0 || ( framenum >= numframes ) )
	{
		warning( "SetFrame","Frame %d is out of range on model %s", framenum, self->model.c_str() );
		return;
	}
	
	// get rid of old anim events so we don't animate
	CancelFlaggedEvents( flags );
	self->CancelFlaggedEvents( flags );
	
	*frame = framenum | FRAME_EXPLICIT;
	
	// if we have a frame override, make sure to set the animation as well
	if ( anim >= 0 )
	{
		*panim = anim | ANIM_BLEND;
	}
}

qboolean Animate::HasAnim( const char *animname )
{
	int num;
	
	num = gi.Anim_Random( self->edict->s.modelindex, animname );
	return ( num >= 0 );
}

void Animate::NewAnimEvent( Event *ev )
{
	NewAnim( ev->GetInteger( 1 ) & ANIM_MASK, (bodypart_t) ev->GetInteger( 2 )  );
}

void Animate::StopAnimating( bodypart_t part )
{
	int frame;
	int anim;
	
	if ( part == all )
	{
		// legs
		frame = CurrentFrame( legs );
		anim = CurrentAnim( legs );
		SetFrame( frame, legs, anim );
		// torso
		frame = CurrentFrame( torso );
		anim = CurrentAnim( torso );
		SetFrame( frame, torso, anim );
	}
	else
	{
		frame = CurrentFrame( part );
		anim = CurrentAnim( part );
		SetFrame( frame, part, anim );
	}
}

void Animate::StopAnimatingAtEnd( bodypart_t part )
{
	int anim;
	
	if ( part == all )
	{
		StopAnimatingAtEnd( legs );
		StopAnimatingAtEnd( torso );
	}
	else if ( part == legs )
	{
		anim = CurrentAnim( part );
		SetFrame( legs_numframes - 1, part, anim );
	}
	else if ( part == torso )
	{
		anim = CurrentAnim( part );
		SetFrame( torso_numframes - 1, part, anim );
	}
}

////////////////////////////
//
// BODY PART SPECIFIC EVENTS
//
////////////////////////////

// Legs

void Animate::Legs_AnimDoneEvent( Event * )
{
	EndAnim( legs );
}

void Animate::Legs_AnimEvent( Event *ev )
{
	RandomAnimate( ev->GetString( 1 ), NULL, legs );
}

void Animate::Legs_SetFrameEvent( Event *ev )
{
	int framenum;
	int animnum;
	
	framenum = ev->GetInteger( 1 );
	if ( ev->NumArgs() > 1 )
	{
		animnum = gi.Anim_NumForName( self->edict->s.modelindex, ev->GetString( 2 ) );
	}
	else
	{
		animnum = -1;
	}
	
	SetFrame( framenum, legs, animnum );
}

// HACK HACK HACK
void Animate::Legs_StopAnimating( Event * )
{
	CancelFlaggedEvents( EVENT_LEGS_ANIM );
	self->CancelFlaggedEvents( EVENT_LEGS_ANIM );
}

// Torso

void Animate::Torso_AnimDoneEvent( Event * )
{
	EndAnim( torso );
}

void Animate::Torso_AnimEvent( Event *ev )
{
	RandomAnimate( ev->GetString( 1 ), NULL, torso );
}

void Animate::Torso_SetFrameEvent( Event *ev )
{
	int framenum;
	int animnum;
	
	framenum = ev->GetInteger( 1 );
	if ( ev->NumArgs() > 1 )
	{
		animnum = gi.Anim_NumForName( self->edict->s.modelindex, ev->GetString( 2 ) );
	}
	else
	{
		animnum = -1;
	}
	
	SetFrame( framenum, torso, animnum );
}

// HACK HACK HACK
void Animate::Torso_StopAnimating( Event * )
{
	CancelFlaggedEvents( EVENT_TORSO_ANIM );
	self->CancelFlaggedEvents( EVENT_TORSO_ANIM );
}

void Animate::ClearTorsoAnim( void )
{
	tiki_cmd_t  cmds;
	int         last_anim;
	static qboolean clearing = false;
	
	last_anim = self->edict->s.torso_anim & ANIM_MASK;
	
	if ( ( self->edict->s.torso_anim & ANIM_BLEND ) && !( self->edict->s.torso_anim & ANIM_SERVER_EXITCOMMANDS_PROCESSED ) && !clearing )
	{
		if ( gi.Frame_Commands( self->edict->s.modelindex, last_anim, TIKI_FRAME_CMD_EXIT, &cmds ) )
		{
			int ii, j;
			
			clearing = true;
			for( ii = 0; ii < cmds.num_cmds; ii++ )
            {
				Event *ev = new Event( cmds.cmds[ ii ].args[ 0 ] );
				
				ev->SetSource( EV_FROM_ANIMATION );
				ev->SetAnimationNumber( last_anim );
				ev->SetAnimationFrame( 0 );
				
				for( j = 1; j < cmds.cmds[ ii ].num_args; j++ )
				{
					ev->AddToken( cmds.cmds[ ii ].args[ j ] );
				}
				self->ProcessEvent( ev );
            }
			clearing = false;
		}
		self->edict->s.torso_anim |= ANIM_SERVER_EXITCOMMANDS_PROCESSED;
	}
	
	CancelFlaggedEvents( EVENT_TORSO_ANIM );
	self->CancelFlaggedEvents( EVENT_TORSO_ANIM );
	self->edict->s.torso_anim &= ~ANIM_BLEND;
}

void Animate::ClearLegsAnim( void )
{
	tiki_cmd_t        cmds;
	int               last_anim;
	//static qboolean   clearing = false;
	
	if ( self->edict->s.anim & ANIM_SERVER_EXITCOMMANDS_PROCESSED )
	{
		last_anim = self->edict->s.anim & ANIM_MASK;
		
		if ( gi.Frame_Commands( self->edict->s.modelindex, last_anim, TIKI_FRAME_CMD_EXIT, &cmds ) )
		{
			int ii, j;
			
			//clearing = true;
			for( ii = 0; ii < cmds.num_cmds; ii++ )
            {
				Event *ev = new Event( cmds.cmds[ ii ].args[ 0 ] );
				
				ev->SetSource( EV_FROM_ANIMATION );
				ev->SetAnimationNumber( last_anim );
				ev->SetAnimationFrame( 0 );
				
				for( j = 1; j < cmds.cmds[ ii ].num_args; j++ )
				{
					ev->AddToken( cmds.cmds[ ii ].args[ j ] );
				}
				self->ProcessEvent( ev );
            }
			//clearing = false;
		}
		self->edict->s.anim |= ANIM_SERVER_EXITCOMMANDS_PROCESSED;
	}
	
	CancelFlaggedEvents( EVENT_LEGS_ANIM );
	self->CancelFlaggedEvents( EVENT_LEGS_ANIM );
	
	self->edict->s.anim &= ~ANIM_BLEND;
}
