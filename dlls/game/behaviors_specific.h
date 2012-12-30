//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/behaviors_specific.h                           $
// $Revision:: 12                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Specific Behaviors will go in this file.  Specific, meaning, behaviors
// that are meant to be used by specific actors
//

#ifndef __BEHAVIORS_SPECIFIC_H__
#define __BEHAVIORS_SPECIFIC_H__

#include "behavior.h"
#include "behaviors_general.h"
#include "work.hpp"
#include "patrol.hpp"
#include "helper_node.h"
#include "actor.h"


//------------------------- CLASS ------------------------------
//
// Name:          PatrolWorkHibernate
// Base Class:    Behavior
//
// Description:   Big Behavior that looks at tendenencies
//                on the actor to help decide if it is going
//                to work, patrol, or hibernate
//
// Method of Use: Statemachine or another behavior
//--------------------------------------------------------------
class PatrolWorkHibernate : public Behavior
   {
	private: // Parameters

   public:  // States
      enum
      {
      PATROLWORKHIBERNATE_SELECT_STATE,
      PATROLWORKHIBERNATE_PATROL,
      PATROLWORKHIBERNATE_WORK,
      PATROLWORKHIBERNATE_HIBERNATE,
      PATROLWORKHIBERNATE_HOLD,
	  PATROLWORKHIBERNATE_HIBERNATE_HOLD,
      PATROLWORKHIBERNATE_FAILED,
	  PATROLWORKHIBERNATE_HACK
      };

   protected:
      void                             _init             ( Actor &self );          
      void                             _selectState      ( Actor &self );      

      void                             _setupPatrol      ( Actor &self );      
      void                             _patrol           ( Actor &self );
      
      void                             _setupHibernate   ( Actor &self );
      void                             _hibernate        ( Actor &self );
      
      void                             _setupWork        ( Actor &self );
      void                             _work             ( Actor &self );

      void                             _setupHold        ( Actor &self );
      void                             _hold             ( Actor &self );

      bool                             _wantsToWork      ( Actor &self );
      bool                             _wantsToHibernate ( Actor &self );
      
      
   public:
      CLASS_PROTOTYPE( PatrolWorkHibernate );

		void						            SetArgs  ( Event *ev );      
      void                             AnimDone ( Event *ev );

		void						            Begin    ( Actor &self );		
		BehaviorReturnCode_t					Evaluate ( Actor &self );
		void						            End      ( Actor &self );

		virtual void                     Archive  ( Archiver &arc );

   private: // Component Behaviors
      Patrol                           _patrolComponent;
      Work                             _workComponent;
      Hibernate                        _hibernateComponent;

   private:
      // Member Variables
      unsigned int                     _state;
      HelperNodePtr                    _lastWorkNodeChecked;
      HelperNodePtr                    _lastHibernateNodeChecked;      

      float                            _nextStateCheck;
      float                            _nextWorkCheck;
      float                            _nextHibernateCheck;

      // Constants
      static const float               MIN_DISTANCE_TO_PATROL_NODE;
      static const float               MIN_DISTANCE_TO_WORK_NODE;
      static const float               MIN_DISTANCE_TO_HIBERNATE_NODE;
      
   };

inline void PatrolWorkHibernate::Archive( Archiver &arc	)
   {
   Behavior::Archive ( arc );	    
   
   // Archive Parameters

   // Archive Components
   arc.ArchiveObject ( &_patrolComponent );
   arc.ArchiveObject ( &_workComponent );
   arc.ArchiveObject ( &_hibernateComponent );

   // Archive Memeber Vars 
   arc.ArchiveUnsigned( &_state );
   arc.ArchiveSafePointer( &_lastWorkNodeChecked );
   arc.ArchiveSafePointer( &_lastHibernateNodeChecked );

   arc.ArchiveFloat( &_nextStateCheck );
   arc.ArchiveFloat( &_nextWorkCheck );
   arc.ArchiveFloat( &_nextHibernateCheck );
   }


//------------------------- CLASS ------------------------------
//
// Name:          PatrolWorkWander
// Base Class:    Behavior
//
// Description:   Big Behavior that looks at tendenencies
//                on the actor to help decide if it is going
//                to work, patrol.  It will wander by default.
//
// Method of Use: Statemachine or another behavior
//--------------------------------------------------------------
class PatrolWorkWander : public Behavior
{
	private: // Parameters

	public:  // States
		enum
			{
			PATROLWORKWANDER_SELECT_STATE,
			PATROLWORKWANDER_PATROL,
			PATROLWORKWANDER_WORK,
			PATROLWORKWANDER_WANDER,
			PATROLWORKWANDER_HOLD,	  
			PATROLWORKWANDER_FAILED
			};

	protected:
		void							_init				( Actor &self );          
		void							_selectState		( Actor &self );      

		void							_setupPatrol		( Actor &self );      
		void							_patrol				( Actor &self );
		void							_patrolFailed		( Actor &self );

      
		void							_setupWander		( Actor &self );
		void							_wander				( Actor &self );
		void							_wanderFailed		( Actor &self );
      
		void							_setupWork			( Actor &self );
		void							_work				( Actor &self );
		void							_workFailed			( Actor &self );

		void							_setupHold			( Actor &self );
		void							_hold				( Actor &self );

		bool							_wantsToWork		( Actor &self );
		bool							_wantsToPatrol		( Actor &self );
		bool							_wantsToWander		( Actor &self );
	  
      
      
	public:
		CLASS_PROTOTYPE( PatrolWorkWander );

		void							SetArgs				( Event *ev	);      
		void							AnimDone			( Event *ev );

		void							Begin				( Actor &self );		
		BehaviorReturnCode_t			Evaluate			( Actor &self );
		void							End					( Actor &self );

		virtual void					Archive				( Archiver &arc );

	private: // Component Behaviors
		Patrol                           _patrolComponent;
		Work                             _workComponent;
		MoveRandomDirection              _wanderComponent;

	private: // Member Variables
		unsigned int                     _state;
		HelperNodePtr                    _lastWorkNodeChecked;
		HelperNodePtr                    _lastPatrolNodeChecked;
      
		float                            _nextStateCheck;
		float                            _nextWorkCheck;
		float                            _nextPatrolCheck;
	  
		int                              _wanderFailures;	 
		int                              _patrolFailures;
		int                              _workFailures;

		// Constants
		static const float               MIN_DISTANCE_TO_PATROL_NODE;
		static const float               MIN_DISTANCE_TO_WORK_NODE;
      
};

inline void PatrolWorkWander::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	    
   
	// Archive Parameters

	// Archive Components
	arc.ArchiveObject      ( &_patrolComponent );
	arc.ArchiveObject      ( &_workComponent );
	arc.ArchiveObject      ( &_wanderComponent );

	// Archive Memeber Vars 
	arc.ArchiveUnsigned    ( &_state );
	arc.ArchiveSafePointer ( &_lastWorkNodeChecked );
	arc.ArchiveSafePointer ( &_lastPatrolNodeChecked );
	arc.ArchiveFloat       ( &_nextStateCheck );
	arc.ArchiveFloat       ( &_nextWorkCheck );   
	arc.ArchiveFloat       ( &_nextPatrolCheck );
	arc.ArchiveInteger     ( &_wanderFailures );
	arc.ArchiveInteger     ( &_patrolFailures ); 
	arc.ArchiveInteger     ( &_workFailures   );

}

#endif /* __BEHAVIORS_SPECIFIC_H__ */
