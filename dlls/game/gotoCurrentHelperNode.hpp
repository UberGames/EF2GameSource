//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/gotoCurrentHelperNode.hpp					 $
// $Revision:: 169															 $
//   $Author:: sketcher														 $
//     $Date:: 4/26/02 2:22p                                                 $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// gotoCurrentHelperNode Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class GotoCurrentHelperNode;

#ifndef __GOTO_CURRENT_HELPER_NODE__
#define __GOTO_CURRENT_HELPER_NODE__

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:           GotoHelperNodeEX
// Base Class:     Behavior
//
// Description:    Moves the Actor to the currentHelper node of the 
//				   Actor
//
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class GotoCurrentHelperNode : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{
			GOTO_HNODE_FIND_NODE,
			GOTO_HNODE_MOVE_TO_NODE,
			GOTO_HNODE_SUCCESS,
			GOTO_HNODE_FAILED
		} GotoHelperNodeStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		str				_movementAnim;		
		
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( GotoHelperNodeStates_t state );
		void	setInternalState	( GotoHelperNodeStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();		
	

		void					setupStateMoveToNode		();
		BehaviorReturnCode_t	evaluateStateMoveToNode		();
		void					failureStateFindNode		( const str& failureReason );

		void					setupStateFindNode		    ();
		BehaviorReturnCode_t    evaluateStateFindNode       ();
		void					failureStateMoveToNode		( const str& failureReason );

	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( GotoCurrentHelperNode );

										GotoCurrentHelperNode();
									   ~GotoCurrentHelperNode();

		void							SetArgs			( Event *ev );      
		void							AnimDone		( Event *ev );
	
		void							Begin			( Actor &self );		
		BehaviorReturnCode_t			Evaluate		( Actor &self );
		void							End				( Actor &self );

		// Accessors		
		void							SetMovementAnim ( const str &anim );


		virtual void					Archive  ( Archiver &arc );

	//-------------------------------------
	// Components
	//-------------------------------------
	private: 	
		GotoPoint						_gotoPoint;


	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		GotoHelperNodeStates_t			_state;		 				
		HelperNodePtr					_node;
		bool							_faceEnemy;		
		static const float				NODE_RADIUS;

	};

inline void	GotoCurrentHelperNode::SetMovementAnim ( const str &anim )
{
	_movementAnim = anim;
}


inline void GotoCurrentHelperNode::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	     

	//
	// Archive Parameters
	//
	arc.ArchiveString		( &_movementAnim );	
	//
	// Archive Components
	//
	arc.ArchiveObject		( &_gotoPoint );

	//
	// Archive Member Variables
	//
	ArchiveEnum				( _state, GotoHelperNodeStates_t   );			
	arc.ArchiveSafePointer	( &_node		);
	arc.ArchiveBool			( &_faceEnemy );
}

#endif /* __GOTO_CURRENT_HELPER_NODE__ */

