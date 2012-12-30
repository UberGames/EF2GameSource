//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/MoveRandomDirection.hpp		              $
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
// MoveRandomDirection Behavior Definition
//
//--------------------------------------------------------------------------------


//==============================
// Forward Declarations
//==============================
class MoveRandomDirection;

#ifndef __MOVE_RANDOM_DIRECTION___
#define __MOVE_RANDOM_DIRECTION___

#include "behavior.h"
#include "behaviors_general.h"


#define MIN_RANDOM_DIRECTION_DESTINATION 64.0f

//------------------------- CLASS ------------------------------
//
// Name:           MoveRandomDirection
// Base Class:     Behavior
//
// Description:    A replacement for Wander -- Utilizes fewer traces
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class MoveRandomDirection : public Behavior
	{
	public: 
		typedef enum
		{
		RANDOM_MOVE_ANYWHERE,
		RANDOM_MOVE_IN_FRONT,
		RANDOM_MOVE_IN_BACK,
		} randomMoveModes_t;

	private: // Parameters
		str							anim;

	protected:
		Vector		_chooseRandomDirection    ( Actor &self );
		float		_getDistanceToDestination ( Actor &self );
		void		findDestination			  ( Actor &self );
		void		setLegAnim				  ( Actor &self );
		void		setTorsoAnim			  ( Actor &self );

	public:
		CLASS_PROTOTYPE( MoveRandomDirection );

									MoveRandomDirection();
									~MoveRandomDirection();

		void						SetArgs( Event *ev );      
		void						Begin( Actor &self );		
		BehaviorReturnCode_t		Evaluate( Actor &self );
		void						End( Actor &self );
		virtual void				Archive( Archiver &arc );

		// Accessors
		void SetDistance( float dist );
		void SetMinDistance( float dist );
		void SetAnim( const str &moveAnim );
		void SetMode( unsigned int mode );

	private: 
		GotoPoint						_chase;

		Vector							_destination;
		unsigned int					_mode;
		float							_dist;
		float							_minDistance;
		float							_nextChangeTime;      
		bool							_foundGoodDestination;
		bool							_forever;
		bool							_faceEnemy;
		str								_torsoAnim;

	};

inline void MoveRandomDirection::Archive( Archiver &arc	)
	{
	Behavior::Archive( arc );

	// Archive Parameters
	arc.ArchiveString( &anim );

	// Archive Components
	arc.ArchiveObject( &_chase );

	// Archive Member Vars
	arc.ArchiveVector( &_destination );   
	arc.ArchiveUnsigned( &_mode );
	arc.ArchiveFloat( &_dist );
	arc.ArchiveFloat( &_minDistance );
	arc.ArchiveFloat( &_nextChangeTime );
	arc.ArchiveBool( &_foundGoodDestination );
	arc.ArchiveBool( &_forever );
	arc.ArchiveBool( &_faceEnemy );
	arc.ArchiveString( &_torsoAnim );
	}




#endif /* __MOVE_RANDOM_DIRECTION___ */

