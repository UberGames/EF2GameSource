//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/coverCombatWithRangedWeapon.hpp                $
// $Revision:: 169                                                            $
//   $Author:: sketcher                                                   $
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
// CoverCombatWithRangedWeapon Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class GotoHelperNodeNearestEnemy;

#ifndef __GOTO_HELPER_NODE_NEAREST_ENEMY___
#define __GOTO_HELPER_NODE_NEAREST_ENEMY__

#include "behavior.h"
#include "behaviors_general.h"
#include "gotoHelperNode.hpp"

//------------------------- CLASS ------------------------------
//
// Name:           CoverCombatWithRangedWeapon
// Base Class:     Behavior
//
// Description:   
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class GotoHelperNodeNearestEnemy : public Behavior {
	public:
	//------------------------------------
	// States
	//------------------------------------
	typedef enum {
		GOTO_HNODE_FIND_NODE,
		GOTO_HNODE_MOVE_TO_NODE,
		GOTO_HNODE_SUCCESS,
		GOTO_HNODE_FAILED
	} GotoHelperNodeStates_t;

	//-------------------------------------
	// Public Interface
	//-------------------------------------
	CLASS_PROTOTYPE(GotoHelperNodeNearestEnemy);

	GotoHelperNodeNearestEnemy();
	~GotoHelperNodeNearestEnemy();

	void SetArgs(Event* ev);
	void AnimDone(Event* ev);

	void Begin(Actor& self);
	BehaviorReturnCode_t Evaluate(Actor& self);
	void End(Actor& self);

	// Accessors
	void SetNode(HelperNode* node);
	void SetMovementAnim(const str& anim);

	virtual void Archive(Archiver& arc);

	protected:
	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	void transitionToState(GotoHelperNodeStates_t state);
	void setInternalState(GotoHelperNodeStates_t state, const str& stateName);
	void init(Actor &self);
	void think();
	void updateEnemy();


	void setupStateFindNode();
	BehaviorReturnCode_t evaluateStateFindNode();
	void failureStateFindNode(const str& failureReason);

	void setupStateMoveToNode();
	BehaviorReturnCode_t evaluateStateMoveToNode();
	void failureStateMoveToNode(const str& failureReason);

	private:
	//------------------------------------
	// Parameters
	//------------------------------------
	str _nodeType;
	str _movementAnim;
	float _maxDistance;

	//-------------------------------------
	// Components
	//-------------------------------------
	GotoHelperNode _gotoHNode;

	//-------------------------------------
	// Member Variables
	//-------------------------------------
	GotoHelperNodeStates_t _state;
	HelperNodePtr _node;
	EntityPtr _currentEnemy;

	Actor* _self;
};



inline void GotoHelperNodeNearestEnemy::Archive(Archiver& arc) {
	Behavior::Archive(arc);

	//
	// Archive Parameters
	//
	arc.ArchiveString(&_nodeType);
	arc.ArchiveString(&_movementAnim);
	arc.ArchiveFloat(&_maxDistance);

	//
	// Archive Components
	//
	arc.ArchiveObject(&_gotoHNode);

	//
	// Archive Member Variables
	//
	ArchiveEnum(_state, GotoHelperNodeStates_t);
	arc.ArchiveSafePointer(&_node);
	arc.ArchiveSafePointer(&_currentEnemy);
	arc.ArchiveObjectPointer((Class **)&_self);
}

#endif /* __GOTO_HELPER_NODE_NEAREST_ENEMY__ */

