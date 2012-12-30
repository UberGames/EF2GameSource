//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/behaviors_general.h                            $
// $Revision:: 115                                                            $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:11a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Generalized Behaviors will go in this file.  Generalized, meaning, behaviors
// that are meant to be used easily by multiple actors
//

//==============================
// Forward Declarations
//==============================
class CircleStrafeEntity;
class DoAttack;

#ifndef __BEHAVIORS_GENERAL_H__
#define __BEHAVIORS_GENERAL_H__

#include "behavior.h"
#include "behaviors.h"
#include "rotateToEntity.hpp"
#include "teleportToEntity.hpp"
#include "teleportToPosition.hpp"
#include "doAttack.hpp"
//#include "generalCombatWithMeleeWeapon.hpp"
#include "helper_node.h"
#include "actor.h"
#include "GoDirectlyToPoint.h"

//=============================
// Enums for states
//=============================








//===================================================================================
//
// Known Working Behaviors -- May need some clean up, but they do work
//
//===================================================================================


//------------------------- CLASS ------------------------------
//
// Name:          WarpToPosition
// Base Class:    Behavior
//
// Description:   Attempts to Warp the Actor to the specified
//                position
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------

class WarpToPosition : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
		WARP_TO_POSITION_CHECK_POSITION,
		WARP_TO_POSITION_WARP,
		WARP_TO_POSITION_FAILED,
		WARP_TO_POSITION_SUCCESS,
		WARP_TO_POSITION_NUMBER_OF_STATES
		} WarpToPosStates_t;


	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		Vector                      _position;

	public:
		CLASS_PROTOTYPE( WarpToPosition );

		void						SetArgs              ( Event *ev      );		
		void						Begin                ( Actor &self    );
		BehaviorReturnCode_t		Evaluate             ( Actor &self    );
		void						End                  ( Actor &self    );
		virtual void		        Archive              ( Archiver &arc  );

		// Mutators       
		void						SetPosition              ( const Vector &position );
		
	
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:		
		void                        checkPosition        ( Actor &self );
		void                        checkPositionFailed  ( Actor &self );
		void                        warpToPosition       ( Actor &self );
		

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:
		// Member Vars
		int							_state;          //-- Maintains our Behavior's current State

	};

inline void WarpToPosition::SetPosition( const Vector &position )
	{
	_position = position;
	}

inline void WarpToPosition::Archive( Archiver &arc	)
	{
	Behavior::Archive( arc );

	// Archive Parameters
	arc.ArchiveVector  ( &_position           );

	// Archive Member Variables
	arc.ArchiveInteger ( &_state          );

	}


//------------------------- CLASS ------------------------------
//
// Name:          WarpToEntity
// Base Class:    Behavior
//
// Description:   Attempts to Warp the actor to the specified entity
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------

class WarpToEntity : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
		WARP_TO_ENTITY_SELECT_POSITION,		
		WARP_TO_ENTITY_WARP,
		WARP_TO_ENTITY_FAILED,
		WARP_TO_ENTITY_SUCCESS,
		WARP_TO_ENTITY_NUMBER_OF_STATES
		} WarpToEntStates_t;


		typedef enum
		{
		POSITION_REAR,
		POSITION_LEFT,
		POSITION_RIGHT,
		POSITION_FRONT,	
		POSITION_NUMBER_OF_POSITIONS
		} WarpToEntPositions_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		EntityPtr                      _entity;

	public:
		CLASS_PROTOTYPE( WarpToEntity );

		void						SetArgs               ( Event *ev      );		
		void						Begin                 ( Actor &self    );
		BehaviorReturnCode_t		Evaluate              ( Actor &self    );
		void						End                   ( Actor &self    );
		virtual void		        Archive               ( Archiver &arc  );

		// Mutators       
		void						SetEntity             ( Entity *ent );
		
	
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:		
		void                        selectPosition        ( Actor &self );		
		void                        setupWarp             ( Actor &self );
		void                        warpToPosition        ( Actor &self );
		void                        warpToPositionFailed  ( Actor &self );
		

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:
		// Components      
		WarpToPosition							 _warp;

		// Member Vars
		unsigned int							 _state;          
		unsigned int							 _position;
		Vector									 _destination;

		// Static Constants
		static const float						 DIST_TO_ENTITY;
	};

inline void WarpToEntity::SetEntity( Entity *ent )
	{
	_entity = ent;
	}

inline void WarpToEntity::Archive( Archiver &arc )
	{
	Behavior::Archive( arc );

	// Archive Parameters
	arc.ArchiveSafePointer ( &_entity         );

	// Archive Components
	arc.ArchiveObject      ( &_warp           );

	// Archive Member Variables
	arc.ArchiveUnsigned    ( &_state          );
	arc.ArchiveUnsigned    ( &_position       );
	arc.ArchiveVector      ( &_destination    );

	}



//------------------------- CLASS ------------------------------
//
// Name:          GotoEntity
// Base Class:    Behavior
//
// Description:   Makes the Actor move to the specified entity
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------
class GotoEntity : public Behavior
{
	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		str                              _anim;  
		float                            _dist;
		EntityPtr                        _entity;
      
	public:
		CLASS_PROTOTYPE( GotoEntity );

										GotoEntity();

		void							SetArgs     ( Event *ev           );      
		void							Begin       ( Actor &self         );		
		BehaviorReturnCode_t			Evaluate    ( Actor &self         );
		void							End         ( Actor &self         );
		virtual void					Archive     ( Archiver &arc       );
      
		// Accessors
		void							SetEntity   ( Actor &self, Entity *ent         );
		void							SetAnim     ( const str &animName );
		void							SetDistance ( float distance      );
		Entity*							GetEntity	();

   private:      
      // Components      
      FollowPathToEntity               _chase;	  
};

inline Entity* GotoEntity::GetEntity()
{
	return _entity;
}

inline void GotoEntity::SetEntity( Actor &self, Entity *ent )
{
	if ( ent )
	{
		_entity = ent;
		_chase.SetGoal( ent, _dist,  self );
		_chase.Begin( self );
	}
}

inline void GotoEntity::SetAnim( const str &animName )
{
	_anim = animName;   
}

inline void GotoEntity::SetDistance( float distance )
{
	_dist = distance;
}

inline void GotoEntity::Archive( Archiver &arc	)
{
	Behavior::Archive      ( arc );

   // Archive Paramters
	arc.ArchiveString      ( &_anim );
	arc.ArchiveFloat       ( &_dist );
	arc.ArchiveSafePointer ( &_entity );

   // Archive Components
	arc.ArchiveObject      ( &_chase );	

   // Archive Member Variables
}


//------------------------- CLASS ------------------------------
//
// Name:          GotoPoint
// Base Class:    Behavior
//
// Description:   Makes the Actor move to the specified position
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------
class GotoPoint : public Behavior
{
	private: // Parameters
		str                              anim;  
		float                            dist;
		Vector                           point;

   protected:
      
   public:
      CLASS_PROTOTYPE( GotoPoint );

		void						            SetArgs     ( Event *ev              );      
		void						            Begin       ( Actor &self            );		
		BehaviorReturnCode_t					Evaluate    ( Actor &self            );
		void						            End         ( Actor &self            );
		virtual void                     Archive     ( Archiver &arc          );
      
      // Accessors
      void                             SetPoint    ( const Vector &position );
      void                             SetAnim     ( const str &animName    );
      void                             SetDistance ( float distance         );

   private:      
      // Components      
      FollowPathToPoint               _chase;
	  bool							  _chaseFailed;

};

inline void GotoPoint::Archive( Archiver &arc	)
{
	Behavior::Archive      ( arc );

   // Archive Paramters
	arc.ArchiveString      ( &anim  );
	arc.ArchiveFloat       ( &dist  );
	arc.ArchiveVector      ( &point );

   // Archive Components
	arc.ArchiveObject      ( &_chase );
	arc.ArchiveBool			( &_chaseFailed );
	

   // Archive Member Variables
}


#include "MoveRandomDirection.hpp"

//------------------------- CLASS ------------------------------
//
// Name:          MoveDirectlyToPoint
// Base Class:    Behavior
//
// Description:   Makes the Actor move to the specified position
//				  without regard for obstacle or worl avoidance
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------
class MoveDirectlyToPoint : public Behavior
{
public:
	CLASS_PROTOTYPE( MoveDirectlyToPoint );
	
	void							SetArgs     ( Event *ev              );      
	void							Begin       ( Actor &self            );		
	BehaviorReturnCode_t			Evaluate    ( Actor &self            );
	void							End         ( Actor &self            );
	virtual void					Archive     ( Archiver &arc          );
	
	// Accessors
	void							SetAnim     ( const str &animName    );
	void							SetPoint    ( const Vector &position );
	void							SetDistance ( const float distance   );
	
private:      
	str                             _anim;  
	float							_dist;
	// Components					
	GoDirectlyToPoint				_motion;
};

inline void MoveDirectlyToPoint::Archive( Archiver &arc	)
{
	Behavior::Archive      ( arc );

   // Archive Parameters
	arc.ArchiveString      ( &_anim  );
	arc.ArchiveFloat       ( &_dist  );

   // Archive Components
	arc.ArchiveObject      ( &_motion );

}


//------------------------- CLASS ------------------------------
//
// Name:          GotoSpecified
// Base Class:    Behavior
//
// Description:   Behavior that will the actor to a specified
//                entity, or pathnode.  This will mostly
//                be called from script, and will be a replacement
//                for the GotoPathNode Behavior
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------
class GotoSpecified : public Behavior
{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
			{
			GOTO_SPEC_CHASE_TARGET,			
			GOTO_SPEC_HOLD,
			GOTO_SPEC_WARP_TO_PATH,
			GOTO_SPEC_WARP_TO_DESTINATION,
			GOTO_SPEC_SUCCESS,			
			GOTO_SPEC_FAILED,
			GOTO_SPEC_NUMBER_OF_STATES
			} GotoSpecifiedStates_t;

	public:
		typedef enum
			{
			GOTO_SPEC_CHASE_POSITION,
			GOTO_SPEC_CHASE_ENTITY,
			GOTO_SPEC_CHASE_NUMBER_OF_MODES,
			} GotoSpecifiedModes_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		str								_anim;  
		EntityPtr						_targetEntity;
		Vector							_targetPosition;
		EntityPtr						_headwatchTarget;
		bool							_forceToTarget;
		bool							_turnAtEnd ;
		int								_maxFailures;
      
	public:
		CLASS_PROTOTYPE( GotoSpecified );

		void							SetArgs			   ( Event *ev              );      
		void                            AnimDone		   ( Event *ev              );
		void							Begin			   ( Actor &self            );		
		BehaviorReturnCode_t			Evaluate		   ( Actor &self            );
		void							End				   ( Actor &self            );
		virtual void					Archive            ( Archiver &arc          );
      
		// Accessors		
		void							SetAnim			   ( const str &animName    );
		void                            SetEntity		   ( Entity *ent            );
		void                            SetPosition		   ( const Vector &pos      );
		void                            SetHeadwatchTarget ( Entity *ent            );
		void                            SetForce           ( bool force             );
		

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void							setupChasePosition     ( Actor &self );
		void                            setupChaseEntity       ( Actor &self );
		void                            setupWarpToPathNode    ( Actor &self );
		void                            setupWarpToDestination ( Actor &self );
		void                            setupHold              ( Actor &self );

		void                            chasePosition          ( Actor &self );
		void                            chaseEntity            ( Actor &self );
		void                            warpToNearestPathNode  ( Actor &self );
		void                            warpToDestination      ( Actor &self );
		void                            hold                   ( Actor &self );
		void							setAngles			   ( Actor &self );

		void                            chaseFailed            ( Actor &self );



	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:      
		// Components      
		GotoPoint					_chasePosition;
		GotoEntity					_chaseEntity;
		WarpToEntity                _warpToEntity;
		WarpToPosition              _warpToPosition;

		// Member Variables
		unsigned int	            _state;
		unsigned int                _mode;		
		int                         _moveFailures;
		float                       _holdTime;
		bool                        _attemptedPathWarp;
		Vector						_endAngles;

		// Constants
		static const float          DIST_TO_TARGET_POSITION;
		static const float          DIST_TO_TARGET_ENTITY;
};

inline void GotoSpecified::SetAnim ( const str &animName )
{
	_anim = animName;
}

inline void GotoSpecified::SetEntity ( Entity *ent )
{
	_targetEntity = ent;
	_mode = GOTO_SPEC_CHASE_ENTITY;
}

inline void GotoSpecified::SetPosition ( const Vector &pos )
{
	_targetPosition = pos;
	_mode = GOTO_SPEC_CHASE_POSITION;
}

inline void GotoSpecified::SetHeadwatchTarget( Entity *ent )
{
	_headwatchTarget = ent;
}

inline void GotoSpecified::SetForce ( bool force )
{
	_forceToTarget = force;
}

inline void GotoSpecified::Archive( Archiver &arc	)
{
	Behavior::Archive      ( arc );

   // Archive Paramters
	arc.ArchiveString      ( &_anim              );
	arc.ArchiveSafePointer ( &_targetEntity      );
	arc.ArchiveVector      ( &_targetPosition    );
	arc.ArchiveSafePointer ( &_headwatchTarget   );
	arc.ArchiveBool        ( &_forceToTarget     );
	arc.ArchiveBool		   ( &_turnAtEnd		 );

   // Archive Components
	arc.ArchiveObject      ( &_chasePosition     );
	arc.ArchiveObject      ( &_chaseEntity       );
	arc.ArchiveObject      ( &_warpToEntity      );
	arc.ArchiveObject      ( &_warpToPosition    );

   // Archive Member Variables
	arc.ArchiveUnsigned    ( &_state             );	
	arc.ArchiveUnsigned    ( &_mode              );
	arc.ArchiveInteger     ( &_moveFailures      );
	arc.ArchiveFloat       ( &_holdTime          );
	arc.ArchiveBool        ( &_attemptedPathWarp );
	arc.ArchiveVector	   ( &_endAngles         );
	arc.ArchiveInteger	   ( &_maxFailures		 );
}

//------------------------- CLASS ------------------------------
//
// Name:          MoveFromConeOfFire
// Base Class:    Behavior
//
// Description:   The Actor does some traces to find a spot out
//                of the way of the player's cone of fire.  If
//                the traces fail, it tries to find a node that
//                satisfies that condition.  If that fails,
//                then the behavior fails.
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------

class MoveFromConeOfFire : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
		MOVE_FCOF_SEARCHING_FOR_SPOT,        
		MOVE_FCOF_STATE_FOUND_SPOT,                
		MOVE_FCOF_STATE_SEARCHING_FOR_NODE,
		MOVE_FCOF_STATE_FOUND_NODE,                
		MOVE_FCOF_STATE_NO_SPOT,
		MOVE_FCOF_SUCCESS,
		MOVE_FCOF_FAILED,
		MOVE_FCOF_NUMBER_OF_STATES
		} MoveFCOFStates_t;


	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		str							_anim;		// --The animation that will be used ( Walk, Run, SideStep, etc )

	public:
		CLASS_PROTOTYPE( MoveFromConeOfFire );

		void						SetArgs              ( Event *ev      );		
		void						Begin                ( Actor &self    );
		BehaviorReturnCode_t		Evaluate             ( Actor &self    );
		void						End                  ( Actor &self    );
		virtual void		        Archive              ( Archiver &arc  );

		// Mutators       
		void						SetAnim              ( const str &anim );
		
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:		
		PathNode		           *_FindNode             ( Actor &self );
		void					    _setDirectionVectors  ( Actor &self );
		void                        _foundDestination     ( Actor &self );
		void                        _noDestination        ( Actor &self );
		void                        _searchForNode        ( Actor &self );
		bool                        _checkDesiredMovement ( Actor &self , const Vector &startPos , const Vector &endPos );
		

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:
		// Component Behaviors
		GotoPoint					_chase;          //-- Behavior that will steer us to our desired position

		// Member Vars
		int							_state;          //-- Maintains our Behavior's current State
		float						_nextsearch;	 //-- Next Time we are allowed to search for a position
		Vector						_newDestination; //-- Where we're going      
		Vector						_left;           //-- Holds Data for Left Direction Check
		Vector						_right;          //-- Holds Data for Right Direction Check
		bool						_stuckOnPlayer;  //-- Flag for being stuck on the player
		float						_oldTurnSpeed;
		Vector						_destination;
		str							_torsoAnim;
		bool						_nextToObstacle;

		// Constants
		static const float          CONE_OF_FIRE_RADIUS;
	};


inline void MoveFromConeOfFire::Archive( Archiver &arc	)
	{
	Behavior::Archive( arc );

	// Archive Parameters
	arc.ArchiveString  ( &_anim           );

	// Archive Components
	arc.ArchiveObject  ( &_chase          );

	// Archive Member Variables
	arc.ArchiveInteger ( &_state          );
	arc.ArchiveFloat   ( &_nextsearch     );	
	arc.ArchiveVector  ( &_newDestination );
	arc.ArchiveVector  ( &_left           );
	arc.ArchiveVector  ( &_right          );
	arc.ArchiveBool	   ( &_stuckOnPlayer  );
	arc.ArchiveFloat   ( &_oldTurnSpeed   );
	arc.ArchiveVector  ( &_destination	  );
	arc.ArchiveString  ( &_torsoAnim	  );
	arc.ArchiveBool	   ( &_nextToObstacle );
	}

inline void MoveFromConeOfFire::SetAnim( const str &anim )
	{
	_anim = anim;
	}

//------------------------- CLASS ------------------------------
//
// Name:          Strafe
// Base Class:    Behavior
//
// Description:   Makes the Actor strafe
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------
class Strafe : public Behavior
{
   public:  // Modes
      enum
      {
      STRAFE_LEFT,
      STRAFE_RIGHT,
      STRAFE_RANDOM,
      STRAFE_NUMBER_OF_MODES
      };

	private: // Parameters
      unsigned int                     mode;

   protected:
      void                             _init          ( Actor &self            );
      void                             _setAnim       ( Actor &self            );
      
   public:
      CLASS_PROTOTYPE( Strafe );

		void						            SetArgs     ( Event *ev              );
      void                             AnimDone    ( Event *ev              );
		void						            Begin       ( Actor &self            );		
		BehaviorReturnCode_t					Evaluate    ( Actor &self            );
		void						            End         ( Actor &self            );
		virtual void                     Archive     ( Archiver &arc          );
      
      // Accessors
      void                             SetMode ( unsigned int strafeMode         );

   private:
      str                              _anim;
      bool                             _canStrafe;
      bool                             _strafeComplete;

      
};

inline void Strafe::Archive( Archiver &arc	)
{
	Behavior::Archive      ( arc );

   // Archive Paramters
	arc.ArchiveUnsigned    ( &mode );

   // Archive Components
	
   // Archive Member Variables
   arc.ArchiveString      ( &_anim           );
   arc.ArchiveBool        ( &_canStrafe      );
   arc.ArchiveBool        ( &_strafeComplete );
}


//------------------------- CLASS ------------------------------
//
// Name:          CircleStrafeEntity
// Base Class:    Behavior
//
// Description:   Makes the Actor circle strafe
//
// Method of Use: State machine or another behavior
//
// ** Needs Additional Work **
//--------------------------------------------------------------
class CircleStrafeEntity   : public Behavior
	{
	private: //Parameters
		str                 _type;
		float               _radius;
		str                 _legAnim;
		bool                _clockwise;
		float				_testDistance;

				
	public:
		CLASS_PROTOTYPE( CircleStrafeEntity );

		void						SetArgs         ( Event *ev          );
		void						Begin           ( Actor &self        );
		BehaviorReturnCode_t		Evaluate        ( Actor &self        );
		void						End             ( Actor &self        );
		virtual void				Archive         ( Archiver &arc      );

		void						SetType         ( const str &type    );
		void						SetRadius       ( float radius       );
		void						SetAnim         ( const str &anim    );
		void						SetClockwise    ( bool clockwise     );
		void						SetTestDistance ( float testDistance );

	protected: //Functions
		Entity*						_getStrafeTarget( Actor &self , const str &target );
		void						_checkParameters(Event *ev);
		qboolean					_checkIfStuck(Actor &self);
		void						_init( Actor &self );
		
		
	private: //Member Vars
		Wander						_wander;
		EntityPtr					_strafeTarget;
		Vector						_lastPosition;
		int							_moveAttempts;      
		Vector						_holdAngles;
		bool						_failed;
		float						_recheckTime;

		float						_startWanderTime;      
				

	};

inline void CircleStrafeEntity::SetType( const str &type )
{  
	_type = type;
}

inline void CircleStrafeEntity::SetRadius( float radius )
{
	_radius = radius;
}

inline void CircleStrafeEntity::SetAnim( const str &anim )
{
	_legAnim = anim;
}

inline void CircleStrafeEntity::SetClockwise( bool clockwise )
{
	_clockwise = clockwise;
}

inline void CircleStrafeEntity::SetTestDistance( float testDistance )
{
	_testDistance = testDistance;
}

inline void CircleStrafeEntity::Archive (	Archiver &arc )
{
	Behavior::Archive( arc );
	
	// Archive Parameters
	arc.ArchiveString      ( &_type         );
	arc.ArchiveFloat       ( &_radius       );
	arc.ArchiveString      ( &_legAnim      );
	arc.ArchiveBool        ( &_clockwise    );
	arc.ArchiveFloat       ( &_testDistance );

	// Archive Member Vars
	arc.ArchiveObject      ( &_wander       );
	arc.ArchiveSafePointer ( &_strafeTarget );
	arc.ArchiveVector      ( &_lastPosition );
	arc.ArchiveInteger     ( &_moveAttempts );
	arc.ArchiveVector      ( &_holdAngles   );
	arc.ArchiveBool        ( &_failed       );
	arc.ArchiveFloat       ( &_recheckTime  );

	arc.ArchiveFloat( &_startWanderTime );
}




//------------------------- CLASS ------------------------------
//
// Name:         FollowInFormation
// Base Class:   Behavior
//
// Description:  Works with other followers to follow in a single file line
//
// Method Of Use: State Machine or other Behaviors
//---------------------------------------------------------------
class FollowInFormation : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
		FOLLOW_TARGET_STATE_SELECT_STATE,
		FOLLOW_TARGET_STATE_FIND_FOLLOW_TARGET,
		FOLLOW_TARGET_STATE_FOLLOW_TARGET,
		FOLLOW_TARGET_STATE_FOLLOW_HOLD,
		FOLLOW_TARGET_STATE_FOLLOW_WARP_TO_NEAREST_PATHNODE,
		FOLLOW_TARGET_STATE_FOLLOW_WARP_TO_FOLLOW_TARGET,
		FOLLOW_TARGET_STATE_FOLLOW_FAILED,
		FOLLOW_TARGET_STATE_FOLLOW_NO_TARGET,
		FOLLOW_TARGET_STATE_FOLLOW_SUCCESS,
		FOLLOW_TARGET_NUMBER_OF_STATES
		} followTargetStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		str						_anim;      		
		float					_emergencyDistance;
		float					_catchupSpeed;

	public:
		CLASS_PROTOTYPE( FollowInFormation );

		void					SetArgs					 ( Event *ev       );		
		void					Begin					 ( Actor &self     );
		BehaviorReturnCode_t	Evaluate				 ( Actor &self     );
		void					End						 ( Actor &self     );
		virtual void			Archive					 ( Archiver &arc   );

		void                    SetDefaultFollowTarget   ( Actor &self     );		
		// Mutators
		void					SetAnim					 ( const str &anim );
		void					SetEmergencyDistance     ( float dist );
		void					SetCatchupSpeed			 ( float speed );
		

		// Accessors
		float                   GetFollowRange           ( );
		unsigned int            GetState                 ( );
      
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void				findFollowTarget	( Actor &self );

		void				selectState			( Actor &self );

		void				setupFollow			( Actor &self );
		void				follow				( Actor &self );		
		void				followFailed		( Actor &self );

		void                setupWarpToPathNode ( Actor &self );
		void                warpToPathNode      ( Actor &self );

		void                setupWarpToTarget   ( Actor &self );
		void                warpToTarget        ( Actor &self );

		void				setupHold			( Actor &self );
		void				hold				( Actor &self );

		void				checkSpeed			( Actor &self );

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:
		// Components
		GotoEntity            _followEntity;  
		WarpToEntity          _warpToEntity;
		WarpToPosition        _warpToPosition;

		// Member Vars
		unsigned int          _state;     
		float                 _followFailureTime;
		float                 _nextFollowAttemptTime;
		float                 _followDist;
		float                 _followDistMin;
		float                 _nextTargetCheckTime;
		float                 _endHold;
		bool                  _selectedFollowTarget;
		bool                  _attemptedWarpToPath;
		bool                  _setFollowFailureTime;
		float				  _oldForwardSpeed;
		float				  _oldTurnSpeed;
		bool				  _codeDriven;


   };

inline void FollowInFormation::SetEmergencyDistance( float dist )
{
	_emergencyDistance = dist;
}

inline void FollowInFormation::SetCatchupSpeed( float speed )
{
	_catchupSpeed = speed;
}

inline void FollowInFormation::Archive( Archiver &arc )
   {
   Behavior::Archive( arc );

	// Archive Parameters
   arc.ArchiveString   ( &_anim                 );
   arc.ArchiveFloat	   ( &_emergencyDistance    );  
   arc.ArchiveFloat	   ( &_catchupSpeed			);

   // Archive Components
   arc.ArchiveObject   ( &_followEntity   );
   arc.ArchiveObject   ( &_warpToEntity   );
   arc.ArchiveObject   ( &_warpToPosition );

	// Archive Member Vars
   arc.ArchiveUnsigned ( &_state                 );
   arc.ArchiveFloat    ( &_followFailureTime     );
   arc.ArchiveFloat    ( &_nextFollowAttemptTime );
   arc.ArchiveFloat    ( &_followDist            );
   arc.ArchiveFloat    ( &_followDistMin         );
   arc.ArchiveFloat    ( &_nextTargetCheckTime   );
   arc.ArchiveFloat    ( &_endHold               );
   arc.ArchiveBool     ( &_selectedFollowTarget  );
   arc.ArchiveBool     ( &_attemptedWarpToPath   );
   arc.ArchiveBool     ( &_setFollowFailureTime  );
   arc.ArchiveFloat	   ( &_oldForwardSpeed );
   arc.ArchiveFloat		( &_oldTurnSpeed );
   arc.ArchiveBool		( &_codeDriven );

   }

inline void FollowInFormation::SetAnim(const str &anim )
	{
	_anim = anim;
	}

inline float FollowInFormation::GetFollowRange()
	{
	return _followDist;
	}

inline unsigned int FollowInFormation::GetState()
	{
	return _state;
	}


//------------------------- CLASS ------------------------------
//
// Name:         GroupFollow
// Base Class:   Behavior
//
// Description:  Works with other followers to follow in a single file line
//
// Method Of Use: State Machine or other Behaviors
//---------------------------------------------------------------
class GroupFollow : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
		CLOSE_WITH_TARGET,
		PACE_TARGET,
		HOLD,
		WANDER,
		NUMBER_OF_STATES,
		INVALID = -1,
		} States;

	//------------------------------------
	// Parameters
	//------------------------------------
	public:
		CLASS_PROTOTYPE( GroupFollow );

		void					SetArgs( Event *ev );		
		void					AnimDone( Event *ev );
		void					SetArgs ( const float stopDistance, const float paceDistance );
		void					Begin( Actor &self );
		BehaviorReturnCode_t	Evaluate ( Actor &self );
		void					End( Actor &self );
		virtual void			Archive( Archiver &arc );

		// Accessors
		int						GetState( void );
      
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		BehaviorReturnCode_t	FindFollowTarget( Actor &self );
		BehaviorReturnCode_t	Hold( Actor &self );
		BehaviorReturnCode_t	PaceTarget( Actor &self );		
		BehaviorReturnCode_t	CloseWithTarget( Actor &self );	
		BehaviorReturnCode_t	Wander(Actor &self );

		float					ComputePaceAnimationRateMultiplier( Actor &self );
		float					ComputeAnimationRate( Actor &self, const str &animationName, const float scale );
		void					GotoHoldState( Actor &self );
		void					GotoPaceTargetState( Actor &self );
		void					GotoCloseWithTargetState( Actor &self );
		void					GotoWanderState(Actor &self);


	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:
		static const float		_minRangeMultiplier;
		static const float		_maxRangeMultiplier;
		str						_idleAnimation;
		str						_paceAnimation;
		str						_closeAnimation;
		str						_torsoAnimation;
		GotoEntity				_follow; 
		MoveRandomDirection		_wander;
		int						_state;
		bool					_animationRateNeedsUpdate;

		float					_stopDistance;
		float					_paceDistance;
		float					_endHold;
		float					_oldForwardSpeed;
		float					_nextFindFollowTime;
		float					_nextPathLenCheckTime;
		float					_nextPathLenCheckTime2;

   };

inline void GroupFollow::Archive( Archiver &arc )
   {
   Behavior::Archive( arc );

   arc.ArchiveString( &_idleAnimation );
   arc.ArchiveString( &_paceAnimation );
   arc.ArchiveString( &_closeAnimation );
   arc.ArchiveString( &_torsoAnimation );
   arc.ArchiveObject( &_follow );
   arc.ArchiveObject( &_wander );
   arc.ArchiveInteger( &_state );
   arc.ArchiveBool( &_animationRateNeedsUpdate );

   arc.ArchiveFloat( &_stopDistance );
   arc.ArchiveFloat( &_paceDistance );

   arc.ArchiveFloat( &_endHold );
   arc.ArchiveFloat( &_oldForwardSpeed );
   arc.ArchiveFloat( &_nextFindFollowTime );
   arc.ArchiveFloat( &_nextPathLenCheckTime );
   arc.ArchiveFloat( &_nextPathLenCheckTime2 );
   }

inline int GroupFollow::GetState()
	{
	return _state;
	}
//------------------------- CLASS ------------------------------
//
// Name:          MoveToDistanceFromEnemy
// Base Class:    Behavior
//
// Description:   Takes information from all the enemies in the
//                actors hate list and uses them to get the
//                best vector away from them.
//
// Method of Use: State Machine or other Behaviors
//--------------------------------------------------------------
class MoveToDistanceFromEnemy : public Behavior
   {
	public: // States		
		typedef enum
			{
			MOVE_TO_DISTANCE_STATE_SEARCHING_FOR_SPOT,        
			MOVE_TO_DISTANCE_STATE_FOUND_SPOT,                
			MOVE_TO_DISTANCE_STATE_SEARCHING_FOR_NODE,
			MOVE_TO_DISTANCE_STATE_FOUND_NODE,                
			MOVE_TO_DISTANCE_STATE_NO_SPOT,
			MOVE_TO_DISTANCE_NUMBER_OF_STATES
			} MoveToDistanceStates_t;

   private:
      str   _anim;
      float _distance;

	public:
      CLASS_PROTOTYPE( MoveToDistanceFromEnemy );

		void						SetArgs     ( Event *ev     );		
		void						Begin       ( Actor &self   );
		BehaviorReturnCode_t	Evaluate    ( Actor &self   );
		void						End         ( Actor &self   );
	   virtual void         Archive     ( Archiver &arc );

      // Accessors
      void                 SetAnim     ( const str &anim );
      void                 SetDistance ( float distance  );
		

	protected:
		void                 _checkParameters ( Event *ev   );
      PathNode            *_FindNode        ( Actor &self );      

	private:
      // Components
      FollowPathToPoint		_chase;

      // Member Vars
		int						_state;
		float						_nextsearch;
      Vector               _away;
      Vector               _toTeam;

   };

inline void MoveToDistanceFromEnemy::Archive( Archiver &arc )
   {
   Behavior::Archive( arc );

	// Archive Parameters
   arc.ArchiveString ( &_anim );
   arc.ArchiveFloat  ( &_distance );
   
   // Archive Components
   arc.ArchiveObject ( &_chase      );

	// Archive Member Vars   
   arc.ArchiveInteger( &_state      );
   arc.ArchiveFloat  ( &_nextsearch );	
   arc.ArchiveVector ( &_away       );
   arc.ArchiveVector ( &_toTeam     );
   }




//Bleh
#include "MoveRandomDirection.hpp"


//------------------------- CLASS ------------------------------
//
// Name:          BackAwayFromEnemy
// Base Class:    Behavior
//
// Description:   Template that can be copied and pasted to
//                generate new behavior
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------

class BackAwayFromEnemy : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
		BAFE_SELECT_STATE,
		BAFE_BACK_AWAY,
		BAFE_BACK_AWAY_FAILED,
		BAFE_BACK_AWAY_SUCCESS,
		BAFE_NUMBER_OF_STATES
		} BAFEStates_t;


	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		str							_anim;		
		float						_dist;
		float						_minDist;

	public:
		CLASS_PROTOTYPE( BackAwayFromEnemy );

		void						SetArgs              ( Event *ev      );		
		void						AnimDone			 ( Event *ev      );
		void						Begin                ( Actor &self    );
		BehaviorReturnCode_t		Evaluate             ( Actor &self    );
		void						End                  ( Actor &self    );
		virtual void		        Archive              ( Archiver &arc  );

		// Mutators       
		void						SetAnim              ( const str &anim );
		void						SetDist				 ( float dist      );
		void						SetMinDist			 ( float minDist   );
		
	
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:		
		void						selectState			( Actor &self );
		void						setupMoveRandom		( Actor &self );
		void						moveRandom			( Actor &self );
		void						moveRandomFailed    ( Actor &self );

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:
		// Component Behaviors
		MoveRandomDirection			_moveRandom;     //-- Behavior that will steer us to our desired position

		// Member Vars
		int							_state;          //-- Maintains our Behavior's current State

	};


inline void BackAwayFromEnemy::Archive( Archiver &arc	)
{
	Behavior::Archive( arc );

	// Archive Parameters
	arc.ArchiveString  ( &_anim           );
	arc.ArchiveFloat   ( &_dist			  );
	arc.ArchiveFloat   ( &_minDist		  );

	// Archive Components
	arc.ArchiveObject  ( &_moveRandom     );

	// Archive Member Variables
	arc.ArchiveInteger ( &_state          );

}

inline void BackAwayFromEnemy::SetAnim( const str &anim )
{
	_anim = anim;
}

inline void BackAwayFromEnemy::SetDist( float dist )
{
	_dist = dist;
}

inline void BackAwayFromEnemy::SetMinDist( float dist )
{
	_minDist = dist;
}





//------------------------- CLASS ------------------------------
//
// Name:          AlertIdle
// Base Class:    Behavior
//
// Description:   The Actor is idle, but willing to get out of 
//                the way, follow the player, and (eventually)
//                investigate.  
//
// Method of Use: Statemachine or another behavior
//--------------------------------------------------------------
class AlertIdle : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public: 
		typedef enum
		{
		ALERT_IDLE_SELECT_STATE,
		ALERT_IDLE_IN_THE_WAY,
		ALERT_IDLE_FOLLOW,
		ALERT_IDLE_HOLD,	
		ALERT_IDLE_WANDER,
		ALERT_IDLE_NUMBER_OF_STATES
	   } AlertIdleStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		str								_followAnim;    // Animation to use when following
		str                             _torsoAnim;     // Torso animation to play
		float                           _baseIdleTime;  // Our base time to idle, before wandering
		float							_emergencyDist; // Distance at which we switch to code driven speed to catch up
		float							_followDist;
		float							_wanderDist;
		 
		

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void							_init				  ( Actor &self );
		void							_selectState		  ( Actor &self );

		void							_setupGetOutOfTheWay  ( Actor &self );
		void							_doGetOutOfTheWay	  ( Actor &self );

		void							_setupFollow		  ( Actor &self );
		void							_doFollow			  ( Actor &self );

		bool                            _tryWander            ( Actor &self );
		void                            _setupWander          ( Actor &self );
		void                            _doWander             ( Actor &self );

		void                            _setupHold            ( Actor &self );
		void							_hold				  ( Actor &self );

		void                            _setNextWanderTime    ( Actor &self );
		bool                            _checkInTheWay        ( Actor &self );

		void                            _setTorsoAnim         ( Actor &self );
		void							checkDistance		  ( Actor &self );

	public:
		CLASS_PROTOTYPE( AlertIdle );

		void							SetArgs  ( Event *ev );      
		void							AnimDone ( Event *ev );

		void							Begin    ( Actor &self );		
		BehaviorReturnCode_t			Evaluate ( Actor &self );
		void							End      ( Actor &self );

		virtual void					Archive  ( Archiver &arc );

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		// Components
		MoveFromConeOfFire				_getOutOfTheWay;
		FollowInFormation				_follow;		
		MoveRandomDirection             _wander;

		// Member Vars
		float                           _nextFollowAttempt;
		float                           _nextWanderTime;
		
		unsigned int					_state;      
		bool                            _useTorsoAnim;
		bool							_unableToFollow;
		Actor*							_self;		
   };


inline void AlertIdle::Archive( Archiver &arc	)
	{
	Behavior::Archive ( arc );
   
	// Archive Parameters
	arc.ArchiveString        ( &_followAnim            );
	arc.ArchiveString        ( &_torsoAnim             );
	arc.ArchiveFloat         ( &_baseIdleTime          );
	arc.ArchiveFloat		 ( &_emergencyDist		   );	
	arc.ArchiveFloat		 ( &_followDist			   );
	arc.ArchiveFloat		 ( &_wanderDist			   );

	// Archive Components
	arc.ArchiveObject        ( &_getOutOfTheWay        );
	arc.ArchiveObject        ( &_follow                );
	arc.ArchiveObject        ( &_wander                );
	


	// Archive Member Vars
	arc.ArchiveFloat         ( &_nextFollowAttempt     );	
	arc.ArchiveFloat         ( &_nextWanderTime        );
	arc.ArchiveUnsigned      ( &_state                 );
	arc.ArchiveBool          ( &_useTorsoAnim          );
	arc.ArchiveBool			 ( &_unableToFollow		   );
	
	arc.ArchiveObjectPointer ( ( Class ** )&_self      );
   
   }






//------------------------- CLASS ------------------------------
//
// Name:          DoBeamAttack
// Base Class:    Behavior
//
// Description:   Rotates the Actor towards its enemy then
//                plays the specified attack animation
//                Similar to the Do Attack Behavior,
//                This one is setup to use and do beam/laser
//                types of attacks
//                
//
// Method of Use: State machine or called from another behavior
//--------------------------------------------------------------
class DoBeamAttack : public Behavior
	{
	private: // Parameters
		str								tagName;
		str								beamShader;
		str								impactModel;
		str								flashModel;
		str								anim;      
		float							damage;
		float							time;
		float							turnspeed;       
		bool							trackEnemy;
		int								beamCount;
		bool							useRotation;
      

	public: // States
		enum
			{
			BEAMATTACK_SETUP,
			BEAMATTACK_ROTATE,
			BEAMATTACK_START_ANIM,
			BEAMATTACK_START_ATTACK,
			BEAMATTACK_ATTACKING,
			BEAMATTACK_COMPLETE,
			BEAMATTACK_FAILED,
			};

	protected:
		void							_setupRotate	( Actor &self );
		void							_rotate			( Actor &self );
		void							_playAttackAnim	( Actor &self );
		void							_startAttack	( Actor &self );      
		void							_createBeam		( Actor &self );
		void							_updateBeam		( Actor &self );
		void							_attackFailed	( Actor &self );

		bool							_canAttack		( Actor &self );
		trace_t							_beamAttackTrace( Actor &self , const Vector &startPos );

	public:
		CLASS_PROTOTYPE( DoBeamAttack );

		void							SetArgs		( Event *ev     );      
		void							AnimDone	( Event *ev     );
		void							Begin		( Actor &self   );		
		BehaviorReturnCode_t			Evaluate	( Actor &self   );
		void							End			( Actor &self   );
		virtual void					Archive		( Archiver &arc );

	private:
		unsigned int					_state;
		RotateToEntity					_rotateBehavior;
		float							_endTime;
		bool							_initialRotationComplete;
		//EntityPtr						_beam;
		Vector							_beamEndPos;
		Container<EntityPtr>			_beamList;
      

	static const float					BEAMATTACK_SPREADFACTOR;
       
	};

inline void DoBeamAttack::Archive( Archiver &arc	)
{
	int num , i;
	Behavior::Archive( arc );	    
	EntityPtr beam;

	// Archive Parameters
	arc.ArchiveString		( &tagName					);
	arc.ArchiveString		( &beamShader				);
	arc.ArchiveString		( &impactModel				);
	arc.ArchiveString		( &flashModel				);
	arc.ArchiveString		( &anim						);
	arc.ArchiveFloat		( &damage					);
	arc.ArchiveFloat		( &time						);
	arc.ArchiveFloat		( &turnspeed				);   
	arc.ArchiveBool			( &trackEnemy				);
	arc.ArchiveInteger		( &beamCount				);
	arc.ArchiveBool			( &useRotation				);
   

   // Archive Components

	// Archive Member Vars
	arc.ArchiveUnsigned		( &_state					);
	arc.ArchiveObject		( &_rotateBehavior			);
	arc.ArchiveFloat		( &_endTime					);
	arc.ArchiveBool			( &_initialRotationComplete	);	

	//arc.ArchiveSafePointer( &_beam	);

	arc.ArchiveVector		( &_beamEndPos				);

	if ( arc.Saving() )
		{
		num = _beamList.NumObjects();
		arc.ArchiveInteger( &num );


		for ( i = 1 ; i <= num; i++ )
			{
			beam = _beamList.ObjectAt( i );
			arc.ArchiveSafePointer( &beam );
			}
		}
	else
		{
		EntityPtr *beamPtr;

		arc.ArchiveInteger( &num );

		_beamList.ClearObjectList();
		_beamList.Resize( num );

		for ( i = 1 ; i <= num ; i++ )
			{
			_beamList.AddObject( beam );

			beamPtr = &_beamList.ObjectAt( i );

			arc.ArchiveSafePointer( beamPtr );
			}

		}

}

//------------------------- CLASS ------------------------------
//
// Name:          FireWeapon
// Base Class:    Behavior
//
// Description:   Behavior that plays the specified Fire Animation                
//
// Method of Use: Called From StateMachine or another behavior
//--------------------------------------------------------------
class FireWeapon : public Behavior
   {
   private: //Parameters
      str                  _anim;      
      
   public:
      CLASS_PROTOTYPE( FireWeapon );

							FireWeapon();

		void						SetArgs  ( Event *ev       );		
		void						Begin    ( Actor &self     );
		BehaviorReturnCode_t	Evaluate ( Actor &self     );
		void						End      ( Actor &self     );
	   virtual void         Archive  ( Archiver &arc   );
      void                 SetAnim  ( const str &anim );
	  void					SetTarget ( Entity *target ) { _target = target; }
	  void					SetTargetPosition( const Vector &targetPos );
	  
      

   protected:      
      void                 _stopFire ( Actor &self );
   
   private: //Member Vars
      int                  _totalShots;
      int                  _frameCount;      
      float                _currentYaw;
      float                _currentPitch;
      Vector               _aimAngles;
      Vector               _targetPosition;      
      EntityPtr            _target;	  
      qboolean             _attacking;
	  bool				   _havePosition;

      
      

   };

inline void FireWeapon::Archive( Archiver &arc )
   {
   Behavior::Archive ( arc );
   
   // Archive Parameters
   arc.ArchiveString      ( &_anim );      
   
   // Archive Components

   // Archive Member Vars
   arc.ArchiveInteger     ( &_totalShots     );
   arc.ArchiveInteger     ( &_frameCount     );
   arc.ArchiveFloat       ( &_currentYaw     );
   arc.ArchiveFloat       ( &_currentPitch   );
   arc.ArchiveVector      ( &_aimAngles      );
   arc.ArchiveVector      ( &_targetPosition );
   arc.ArchiveSafePointer ( &_target         );
   arc.ArchiveBoolean     ( &_attacking      );
   arc.ArchiveBool		  ( &_havePosition   );
   
   }



//
// Sigh
//
#include "closeInOnEnemy.hpp"
//
// ========================================================================
// MetaBehaviors 
// New ( Larger Scope ) Behaviors like "UseCover" or "GeneralCombat"
// The reason for these is that constraints with the state machine are 
// preventing me from achieving the kinds of results that we need.
//
// I am shooting for a "building block" methodolgy here, I hope to utilize
// many of the behaviors we already have ( likely with a few accessor/mutator
// additions ) to build up these large meta behaviors
// ========================================================================
//


//------------------------- CLASS ------------------------------
//
// Name:          SimpleMelee
// Base Class:    Behavior
//
// Description:   The Actor will rush at the enemy until it
//                is within the specified melee range.  Then
//                it will do melee attacks until it can't
//                see the enemy.  If it can't see the enemy or
//                can't get to the enemy, it will attempt to
//                circle strafe until it can get in range 
//
// Method of Use: Statemachine or another behavior
//
// Required Animations:
//				 -- Component Requirements
				 
// Component Behaviors:
//				 -- CloseInOnEnemy
//				 --	CircleStrafeEntity
//				 -- DoAttack
//
//--------------------------------------------------------------
class SimpleMelee : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public: 
		enum
			{
			SIMPLE_MELEE_SELECT_STATE,
			SIMPLE_MELEE_RUSH_ENEMY,
			SIMPLE_MELEE_CIRCLE_STRAFE,
			SIMPLE_MELEE_ATTACK,		 
			SIMPLE_MELEE_HOLD
			};

	//------------------------------------
	// Parameters
	//------------------------------------
	private: // Parameters
		str                              rushAnim;
		str                              attackAnim;
		float                            meleeDist;
		float                            turnSpeed;

	public:
		CLASS_PROTOTYPE( SimpleMelee );

		void							SetArgs       ( Event *ev       );      
		void							AnimDone      ( Event *ev       );
		void							Begin         ( Actor &self     );		
		BehaviorReturnCode_t			Evaluate      ( Actor &self     );
		void							End           ( Actor &self     );
		void							SetRushAnim   ( const str& anim );
		void							SetAttackAnim ( const str& anim );
		void							SetMeleeDist  ( float dist      );
		void							SetTurnSpeed  ( float turnspeed );

		virtual void					Archive       ( Archiver &arc   );

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void                             _init             ( Actor &self );
		void                             _selectState      ( Actor &self );

		void                             _setupRush        ( Actor &self );
		void                             _rush             ( Actor &self );

		void                             _setupStrafe      ( Actor &self );
		void                             _strafe           ( Actor &self );

		void                             _setupMeleeAttack ( Actor &self );
		void                             _meleeAttack      ( Actor &self );

		void                             _setupHold        ( Actor &self );
		void                             _hold             ( Actor &self );


	//-------------------------------------
	// Member Variables
	//-------------------------------------

	private: 
		// Component Behaviors
		CloseInOnEnemy                  _rushEnemy;
		CircleStrafeEntity              _circleStrafe;
		DoAttack                        _attack;

		// Member Variables
		unsigned int					_state; 
		float							_holdTime;
		int								_holdCount;
		bool							_strafeClockwise;
		int								_strafeAttempts;
		float							_nextStrafeTime;
		float							_nextEnemyCheckTime;
		Actor*							_self;

   };


inline void SimpleMelee::Archive( Archiver &arc	)
   {
   Behavior::Archive ( arc );
   
   // Archive Parameters
   arc.ArchiveString        ( &rushAnim               );
   arc.ArchiveString        ( &attackAnim             );
   arc.ArchiveFloat         ( &meleeDist              );
   arc.ArchiveFloat         ( &turnSpeed              );

   // Archive Components
   arc.ArchiveObject        ( &_rushEnemy             );
   arc.ArchiveObject        ( &_circleStrafe          );
   arc.ArchiveObject        ( &_attack                );

   // Archive Member Vars
   arc.ArchiveUnsigned      ( &_state                 );
   arc.ArchiveFloat			( &_holdTime			  );
   arc.ArchiveInteger       ( &_holdCount             );
   arc.ArchiveBool          ( &_strafeClockwise       );
   arc.ArchiveInteger       ( &_strafeAttempts        );
   arc.ArchiveFloat         ( &_nextStrafeTime        );
   arc.ArchiveFloat         ( &_nextEnemyCheckTime    );
   
   arc.ArchiveObjectPointer ( ( Class ** )&_self      );
   
   }





//------------------------- CLASS ------------------------------
//
// Name:          FollowPathBlindly
// Base Class:    Behavior
//
// Description:   Makes the actor use FollowPathBlindly Helper Nodes
//
// Method of Use: Statemachine or another behavior
//--------------------------------------------------------------
class FollowPathBlindly : public Behavior
{
public:						
							CLASS_PROTOTYPE( FollowPathBlindly );
							
	void					SetArgs ( Event *ev );
	void					Begin    ( Actor &self );		
	BehaviorReturnCode_t	Evaluate ( Actor &self );
	void					End      ( Actor &self );
							
	virtual void			Archive  ( Archiver &arc );

protected:							
	HelperNodePtr			FindNextNode( Actor &self );
	HelperNodePtr			FindNearestNode( Actor &self );
	void					SetNode( Actor &self, HelperNodePtr node );
	const bool				AdvanceNode( Actor &self );
	const Vector			ComputeTargetPoint( const HelperNodePtr lastNode, const HelperNodePtr currentNode, const HelperNodePtr nextNode ) const;

private:					
	// Component Behaviors	
	MoveDirectlyToPoint		_gotoHelperNode;
							
	// Member Variables	
	str						_animName;
	HelperNodePtr			_node;
	HelperNodePtr			_nextNode;
	float					_offset;
	str						_startNodeName;
};

inline void FollowPathBlindly::Archive( Archiver &arc	)
{
	Behavior::Archive		( arc );	    
	
	// Archive Parmeters
	
	// Archive Components
	arc.ArchiveObject		( &_gotoHelperNode  );
	
	// Archive Member Vars
	arc.ArchiveString		( &_animName		);
	arc.ArchiveSafePointer	( &_node            );
	arc.ArchiveSafePointer	( &_nextNode        );
	arc.ArchiveFloat		( &_offset			);
	arc.ArchiveString		( &_startNodeName	);
}








//------------------------- CLASS ------------------------------
//
// Name:          Hibernate
// Base Class:    Behavior
//
// Description:   Makes the Actor use Work Helper Nodes
//                Note, this behavior makes extensive use of 
//                the CUSTOM helper node and REQUIRES that
//                the Node's customType = "hibernate"
//
// Method of Use: Statemachine or another behavior
//--------------------------------------------------------------
class Hibernate : public Behavior
   {
   public: // States
      typedef enum
      {
      HIBERNATE_FIND_CLOSEST_NODE,
      HIBERNATE_MOVING_TO_NODE,
      HIBERNATE_AT_NODE,
      HIBERNATE_START_HIBERNATE,
      HIBERNATE_HIBERNATE,
      HIBERNATE_END_HIBERNATE,
      HIBERNATE_HOLD,
	  HIBERNATE_WAIT,
      HIBERNATE_SUCCESSFUL,
      HIBERNATE_FAILED
      } hibernateStates_t;

	private: // Parameters
      
   protected:
      void                             _init                                ( Actor &self );      

      bool                             _setupFindClosestHibernateNode       ( Actor &self );
      void                             _setupFindClosestHibernateNodeFailed ( Actor &self );
      void                             _findClosestHibernateNode            ( Actor &self );
      void                             _findClosestHibernateNodeFailed      ( Actor &self );

      bool                             _setupMovingToHibernateNode          ( Actor &self );
      void                             _setupMovingToHibernateNodeFailed    ( Actor &self );
      void                             _moveToHibernateNode                 ( Actor &self );
      void                             _moveToHibernateNodeFailed           ( Actor &self );

      bool                             _setupAtHibernateNode                ( Actor &self );
      void                             _setupAtHibernateNodeFailed          ( Actor &self );
      void                             _atHibernateNode                     ( Actor &self );

      bool                             _setupHold                           ( Actor &self );
      void                             _setupHoldFailed                     ( Actor &self );
      void                             _hold                                ( Actor &self );

      void                             _startHibernate                      ( Actor &self );
      void                             _hibernate                           ( Actor &self );      
      void                             _endHibernate                        ( Actor &self );
	  void							   _wait								( Actor &self );
      
      
   public:
      CLASS_PROTOTYPE( Hibernate );

		void						            SetArgs  ( Event *ev );      
      void                             AnimDone ( Event *ev );

		void						            Begin    ( Actor &self );		
		BehaviorReturnCode_t					Evaluate ( Actor &self );
		void						            End      ( Actor &self );

		virtual void                     Archive  ( Archiver &arc );

   private: // Component Behaviors
      GotoPoint                        _gotoHelperNode;

   private: // Member Variables
      HelperNodePtr						   _node;            

      unsigned int                     _state;

      int                              _moveFailures; 

      float                            _nextMoveAttempt;   
      
      
      
   };

inline void Hibernate::Archive( Archiver &arc	)
   {
   Behavior::Archive ( arc );	    
   
   // Archive Parameters

   // Archive Components
   arc.ArchiveObject      ( &_gotoHelperNode  );

   // Archive Member Vars
   arc.ArchiveSafePointer ( &_node            );

   arc.ArchiveUnsigned    ( &_state           );

   arc.ArchiveInteger     ( &_moveFailures    );

   arc.ArchiveFloat       ( &_nextMoveAttempt );  
   
   
   }



//------------------------- CLASS ------------------------------
//
// Name:          GotoLiftPosition
// Base Class:    Behavior
//
// Description:   Makes the Actor use Lift Helper Nodes
//                Note, this behavior makes extensive use of 
//                the CUSTOM helper node and REQUIRES that
//                the Node's customType = "lift"
//
// Method of Use: Statemachine or another behavior
//--------------------------------------------------------------
class GotoLiftPosition : public Behavior
	{
	public: // States
		typedef enum
		{
		GLP_FIND_NODE,
		GLP_MOVING_TO_NODE,
		GLP_MOVE_FAILED,
		GLP_AT_NODE,		
		GLP_SUCCESSFUL,
		GLP_FAILED
		} GotoLiftPositionStates_t;

	private: // Parameters
      
	protected:
		void                             _init                                ( Actor &self );      

		void                             _findLiftNode                        ( Actor &self );		

		void                             _setupMovingToLiftNode               ( Actor &self );		
		void                             _moveToLiftNode                      ( Actor &self );

		void                             _setupMoveToLiftFailed               ( Actor &self );
		void                             _moveToLiftNodeFailed                ( Actor &self );
		

		void                             _setupAtLiftNode		                ( Actor &self );		
		void                             _atLiftNode		                    ( Actor &self );

		void                             _setupHold                           ( Actor &self );		
		void                             _hold                                ( Actor &self );

		void                             _warpToLiftNode                       ( Actor &self );
		void                             _warpToPathNode                       ( Actor &self );
      
      
	public:
		CLASS_PROTOTYPE( GotoLiftPosition );

		void								SetArgs  ( Event *ev );      
		void								AnimDone ( Event *ev );

		void								Begin    ( Actor &self );		
		BehaviorReturnCode_t				Evaluate ( Actor &self );
		void								End      ( Actor &self );
		void								FindNodes( Actor &self );

		virtual void						Archive  ( Archiver &arc );

   private: // Component Behaviors
      GotoPoint								_gotoHelperNode;

   private: // Member Variables
      HelperNodePtr							_node; 
      unsigned int							_state;
      int									_moveFailures; 
      float									_nextMoveAttempt;
	  Container<HelperNodePtr>				_availableNodes;
	  Container<HelperNodePtr>				_attemptedNodes;
      
   };

inline void GotoLiftPosition::Archive( Archiver &arc	)
   {
   Behavior::Archive ( arc );	    
   
   // Archive Parameters

   // Archive Components
   arc.ArchiveObject      ( &_gotoHelperNode  );

   // Archive Member Vars
   arc.ArchiveSafePointer ( &_node            );
   arc.ArchiveUnsigned    ( &_state           );
   arc.ArchiveInteger     ( &_moveFailures    );
   arc.ArchiveFloat       ( &_nextMoveAttempt );   

   int num , i;
   if ( arc.Saving() )
	{
		num = _availableNodes.NumObjects();

		arc.ArchiveInteger( &num );
		HelperNodePtr theNode;
		for( i = 1 ; i <= num ; i++ )
		{
			theNode = _availableNodes.ObjectAt(i);
			arc.ArchiveSafePointer( &theNode );
		}
	}
	else
	{
		arc.ArchiveInteger( &num );

		_availableNodes.Resize( num );

		for ( i = 1; i <= num; i++ )
		{
			HelperNodePtr theNode;
			HelperNodePtr *ptrTheNode;

			_availableNodes.AddObject( theNode );

			ptrTheNode = &_availableNodes.ObjectAt( i );

			arc.ArchiveSafePointer( ptrTheNode );
		}
	}

   if ( arc.Saving() )
	{
		num = _attemptedNodes.NumObjects();

		arc.ArchiveInteger( &num );
		HelperNodePtr theNode;
		for( i = 1 ; i <= num ; i++ )
		{
			theNode = _attemptedNodes.ObjectAt(i);
			arc.ArchiveSafePointer( &theNode );
		}
	}
	else
	{
		arc.ArchiveInteger( &num );

		for ( i = 1; i <= num; i++ )
		{
			HelperNodePtr theNode;
			arc.ArchiveSafePointer( &theNode );
			_attemptedNodes.AddObject( theNode );
		}
	}
	   
   }

//
// Oh great googly moogly this is weak.  Yes yes, I know this is a horrible
// horrible thing to do... But it's only temporary until I get all the
// behaviors split out into their own files.
//
#include "generalCombatWithRangedWeapon.hpp"






//
// ========================================================================
// Global Functions
// ========================================================================
//

// Yes, Actor should be const, but find.heuristic stuff complains if it is
PathNode* FindNode ( Actor &self, FollowPath *chase, unsigned int mask , PathNode* (*check)(PathNode* node) );





/*

//------------------------- CLASS ------------------------------
//
// Name:          Template
// Base Class:    Behavior
//
// Description:   Template that can be copied and pasted to
//                generate new behavior
//
// Method of Use: State machine or another behavior
//--------------------------------------------------------------

class Template : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
		TEMPLATE_NUMBER_OF_STATES
		} MoveFCOFStates_t;


	//------------------------------------
	// Parameters
	//------------------------------------
	private: 
		str							_anim;		// --The animation that will be used ( Walk, Run, SideStep, etc )

	public:
		CLASS_PROTOTYPE( Template );

		void						SetArgs              ( Event *ev      );		
		void						Begin                ( Actor &self    );
		BehaviorReturnCode_t		Evaluate             ( Actor &self    );
		void						End                  ( Actor &self    );
		virtual void		        Archive              ( Archiver &arc  );

		// Mutators       
		void						SetAnim              ( const str &anim );
		
	
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:		
		

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private:
		// Component Behaviors
		GotoPoint					_chase;          //-- Behavior that will steer us to our desired position

		// Member Vars
		int							_state;          //-- Maintains our Behavior's current State

		// Constants
		static const float          THE_CONSTANT;
	};


inline void Template::Archive( Archiver &arc	)
	{
	Behavior::Archive( arc );

	// Archive Parameters
	arc.ArchiveString  ( &_anim           );

	// Archive Components
	arc.ArchiveObject  ( &_chase          );

	// Archive Member Variables
	arc.ArchiveInteger ( &_state          );

	}

inline void Template::SetAnim( const str &anim )
	{
	_anim = anim;
	}


*/











#endif /* __BEHAVIORS_GENERAL_H__ */
