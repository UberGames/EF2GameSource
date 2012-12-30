//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actorutil.h                                    $
// $Revision:: 28                                                             $
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
//
// DESCRIPTION:
// Utility Functions and classes for Actor


class FindCoverMovement;
class FindFleeMovement;
class FindEnemyMovement;

#ifndef __ACTORUTIL_H__
#define __ACTORUTIL_H__

#include "actor.h"
#include "actorincludes.h"
#include "weapon.h"

//============================
// Global Functions
//============================
qboolean EntityIsValidTarget( const Entity *ent );

//============================
// Class FindMovement
//============================
//
// Standard Movement, finds a path from the starting position to the ending position
//
class FindMovement : public StandardMovement
	{
	public:
		Actor			*self;

	qboolean validpath (	PathNode *node, int i );
	qboolean done ( PathNode *node, const PathNode *end );
	};



//============================
// Class FindCoverMovement
//============================
//
// Set destination to node with duck or cover set.  Class will find a path to that node, or a closer one.
//
class FindCoverMovement : public StandardMovement
	{
	public:
		Actor			*self;

	qboolean validpath (	PathNode *node, int i );
	qboolean done ( PathNode *node, const PathNode *end );
	};



//============================
// Class FindFleeMovement
//============================
//
// Set destination to node with flee set.  Class will find a path to that node, or a closer one.
//
class FindFleeMovement : public StandardMovement
	{
	public:
		Actor			*self;

	qboolean validpath ( PathNode *node, int i	);
	qboolean done	( PathNode *node,	const PathNode *end );

	};

//============================
// Class FindEnemyMovement
//============================
//
// Not sure what this is for at the moment.
//
class FindEnemyMovement : public StandardMovement
	{
	public:
		Actor			*self;

	qboolean done	( PathNode *node, const PathNode *end );

	};


#endif /* __ACTORUTIL_H__ */
