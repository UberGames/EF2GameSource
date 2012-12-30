//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/closeInOnEnemy.hpp				              $
// $Revision:: 169                                                            $
//   $Author:: sketcher														  $
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
// CloseInOnEnemy Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class Patrol;

#ifndef __PATROL_HPP__
#define __PATROL_HPP__

//------------------------- CLASS ------------------------------
//
// Name:          Patrol
// Base Class:    Behavior
//
// Description:   Makes the actor use Patrol Helper Nodes
//
// Method of Use: Statemachine or another behavior
//--------------------------------------------------------------
class Patrol : public Behavior
   {
	//------------------------------------
	// States
	//------------------------------------
	public: 
		typedef enum
		{
			PATROL_FIND_CLOSEST_PATROL_NODE,
			PATROL_MOVING_TO_PATROL_NODE,
			PATROL_AT_PATROL_NODE,
			PATROL_WAITING_AT_NODE,
			PATROL_WAITING_AT_NODE_FOR_ANIM,
			PATROL_FIND_NEXT_PATROL_NODE,
			PATROL_HOLD,
			PATROL_FAILED,
			PATROL_SUCCESSFUL,
		} patrolStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private: 

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void		_init( Actor &self );      

		bool		_setupFindClosestPatrolNode            ( Actor &self );
		void		_setupFindClosestPatrolNodeFailed      ( Actor &self );
		void		_findClosestPatrolNode                 ( Actor &self );
		void		_findClosestPatrolNodeFailed           ( Actor &self );


		bool		_setupMovingToPatrolNode               ( Actor &self );
		void		_setupMovingToPatrolNodeFailed         ( Actor &self );
		void		_moveToPatrolNode                      ( Actor &self );
		void		_moveToPatrolNodeFailed                ( Actor &self );

		bool		_setupAtPatrolNode                     ( Actor &self );
		void		_setupAtPatrolNodeFailed               ( Actor &self );
		void		_atPatrolNode                          ( Actor &self );

		bool		_setupWaitingAtPatrolNode              ( Actor &self );
		void		_setupWaitingAtPatrolNodeFailed        ( Actor &self );
		void		_waitAtPatrolNode                      ( Actor &self );

		bool		_setupWaitingAtPatrolNodeForAnim       ( Actor &self );
		void		_setupWaitingAtPatrolNodeForAnimFailed ( Actor &self );
		void		_waitAtPatrolNodeForAnim               ( Actor &self );

		bool		_setupFindNextPatrolNode               ( Actor &self );
		void		_setupFindNextPatrolNodeFailed         ( Actor &self );
		void		_findNextPatrolNode                    ( Actor &self );

		bool		_setupHold                             ( Actor &self );
		void		_setupHoldFailed                       ( Actor &self );
		void		_hold                                  ( Actor &self );

	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( Patrol );

									Patrol();
								   ~Patrol();
		void						SetArgs  ( Event *ev );      
		void						AnimDone ( Event *ev );

		void						Begin    ( Actor &self );		
		BehaviorReturnCode_t		Evaluate ( Actor &self );
		void						End      ( Actor &self );

		virtual void				Archive  ( Archiver &arc );
		void						SetAnim	 ( const str& anim );
		void						SetInitialNode( HelperNode* node );

	//-------------------------------------
	// Components
	//-------------------------------------
	private: 
		GotoPoint					_gotoHelperNode;

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		HelperNodePtr				_node;
		HelperNodePtr				_lastNode;
		unsigned int				_state;
		int							_moveFailures;
		float						_nextMoveAttempt;
		float						_waitTime;
		str							_torsoAnim;
		str							_anim;
   };

inline void Patrol::SetInitialNode( HelperNode *node )
{
	_node = node;
}

inline void Patrol::SetAnim( const str &anim )
{
	_anim = anim;
}

inline void Patrol::Archive( Archiver &arc	)
   {
   Behavior::Archive      ( arc );	    
   
   // Archive Parmeters

   // Archive Components
   arc.ArchiveObject      ( &_gotoHelperNode  );

   // Archive Member Vars
   arc.ArchiveSafePointer ( &_node            );
   arc.ArchiveSafePointer ( &_lastNode        );
   arc.ArchiveUnsigned    ( &_state           );
   arc.ArchiveInteger     ( &_moveFailures    );
   arc.ArchiveFloat       ( &_nextMoveAttempt );   
   arc.ArchiveFloat       ( &_waitTime        );
   arc.ArchiveString	  ( &_torsoAnim		  );
   arc.ArchiveString	  ( &_anim			  );
   }

#endif /* PATROL_HPP */
