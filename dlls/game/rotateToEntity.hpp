//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/rotateToEntity.h                               $
// $Revision:: 169                                                            $
//   $Author:: Bschofield                                                     $
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
// RotateToEntity Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class RotateToEntity;

#ifndef __ROTATE_TO_ENTITY_H__
#define __ROTATE_TO_ENTITY_H__

#include "behavior.h"

//------------------------- CLASS ------------------------------
//
// Name:          RotateToEntity
// Base Class:    Behavior
//
// Description:   Rotates the actor to face the passed in entity
//
// Method of Use: Should be aggregated by other behaviors
//--------------------------------------------------------------
class RotateToEntity : public Behavior
	{

	//------------------------------------
	// Parameters
	//------------------------------------
	private:      
		float							_turnspeed;
		EntityPtr						_ent;
		str								_anim;
		str								_entityType;

	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( RotateToEntity );

										RotateToEntity();
									   ~RotateToEntity();


		void							SetArgs      ( Event *ev                          );      
		void							Begin        ( Actor &self                        );		
		BehaviorReturnCode_t			Evaluate     ( Actor &self                        );
		void							End          ( Actor &self                        );
		virtual void					Archive      ( Archiver &arc                      );
		
		void							SetAnim      ( const str& animName                );
		void							SetTurnSpeed ( float turnSpeed                    );
		void							SetEntity    ( Entity *entity                     );      
		void							SetEntityType( const str &ent					  );

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:		
		float							_oldTurnSpeed;
		
	
	};

inline void RotateToEntity::SetTurnSpeed( float turnSpeed )
	{
	_turnspeed = turnSpeed;
	}


inline void RotateToEntity::SetEntity( Entity *entity )
	{
	if ( entity )
		_ent = entity;
	}

inline void RotateToEntity::SetAnim( const str &animName )
	{
	_anim = animName;   
	}

inline void RotateToEntity::SetEntityType( const str &ent )
	{
		if  ( ent == "player" || ent == "enemy" )
			_entityType = ent;
	}

inline void RotateToEntity::Archive( Archiver &arc	)
{
	Behavior::Archive( arc );

	//-------------------------------------
	// Archive Parameters
	//-------------------------------------
	arc.ArchiveFloat       ( &_turnspeed );
	arc.ArchiveSafePointer ( &_ent       );
	arc.ArchiveString      ( &_anim      );
	arc.ArchiveString( &_entityType );

	//-------------------------------------
	// Archive Member Variables
	//-------------------------------------	
	arc.ArchiveFloat	   ( &_oldTurnSpeed );
}


#endif /* __ROTATE_TO_ENTITY_H__ */
