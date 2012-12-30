//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/torsoAimAndFireWeapon.hpp		              $
// $Revision:: 169                                                            $
//   $Author:: sketcher                                                       $
//     $Date:: 4/26/02 2:22p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// torsoAimAndFireWeapon Behavior Definition
//
//--------------------------------------------------------------------------------


//==============================
// Forward Declarations
//==============================
class Talk;

#ifndef __TALK_HPP___
#define __TALK_HPP___

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:           Talk
// Base Class:     Behavior
//
// Description:    Makes the actor play appropriate dialog
//					and make appropriate turntos.
//				   
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class Talk : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
			TALK_MODE_TURN_TO,
			TALK_MODE_COMBAT,
			TALK_MODE_PLAY_GREETING,
			TALK_MODE_WAIT_FOR_GREETING,
			TALK_MODE_TALK,
			TALK_MODE_WAIT,
			TALK_MODE_TURN_BACK,
			TALK_SUCCESS,
			TALK_FAILED
		} TalkStates_t;

	private:
		TurnTo					turnto;
		SentientPtr				ent_listening;
		EntityPtr				last_headwatch_target;
		float					original_yaw;
		float					yaw;
		int						mode;
		qboolean				move_allowed;
		bool					animDone;
		str						oldAnimName;

	public:
		CLASS_PROTOTYPE( Talk );

		void						SetUser( Sentient *user );
		void						GreetingDone( Event *ev );
		void						AnimDone( Event *ev );
		void						Begin( Actor &self );
		BehaviorReturnCode_t		Evaluate( Actor &self );
		void						End( Actor &self );
		virtual void				Archive( Archiver &arc );
	};

inline void Talk::Archive( Archiver &arc )
	{
	Behavior::Archive( arc );

	arc.ArchiveObject( &turnto );
	arc.ArchiveSafePointer( &ent_listening );
	arc.ArchiveSafePointer( &last_headwatch_target );
	arc.ArchiveFloat( &original_yaw );
	arc.ArchiveFloat( &yaw );
	arc.ArchiveInteger( &mode );
	arc.ArchiveBoolean( &move_allowed );
	arc.ArchiveBool( &animDone );
	arc.ArchiveString( &oldAnimName );	
	}

#endif /* __TALK_HPP___ */

