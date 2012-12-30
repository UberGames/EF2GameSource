//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actorutil.cpp                                  $
// $Revision:: 44                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/11/02 3:27a                                                 $
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
// These classes will be used to help simplify the Actor class, and move some of the large subsystems it contained into
// these smaller classes.  
//

#include "_pch_cpp.h"
#include "actorutil.h"
#include "actor_sensoryperception.h"
#include "actor_enemymanager.h"
#include "player.h"
#include "object.h"
















//===============================================================
//
// Movement Utility Functions
//
//===============================================================
qboolean FindMovement::validpath( PathNode *node, int i )
{
	if ( !StandardMovement::validpath( node, i ) )
		return false;
	
	return true;
	
}

qboolean FindMovement::done( PathNode *node , const PathNode *end )
{	
	if ( node == end )
		return true;
	
	return false;
}

qboolean FindCoverMovement::validpath( PathNode *node, int i )
{
	PathNodeConnection *path;
	PathNode *n;
	
	path = &node->GetConnection( i );
	
	if ( !StandardMovement::validpath( node, i ) )
	{
		return false;
	}
	
	n = thePathManager.GetNode( path->targetNodeIndex );;
	if ( !n || self->CloseToEnemy( n->origin, 128.0f ) )
	{
		return false;
	}
	
	return true;
}

qboolean FindCoverMovement::done( PathNode *node , const PathNode *end )
{
	// Get our current enemy
	Entity *currentEnemy;
	currentEnemy = self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return true;
	
	if ( node == end )
	{
		return true;
	}
	
	if ( !( node->nodeflags & ( AI_DUCK | AI_COVER ) ) )
	{
		return false;
	}
	
	if ( self )
	{
		return true;
	}
	
	return false;
}

qboolean FindFleeMovement::validpath( PathNode *node , int i )
{
	PathNodeConnection *path;
	PathNode *n;
	
	path = &node->GetConnection( i );
	if ( !StandardMovement::validpath( node, i ) )
	{
		return false;
	}
	
	n = thePathManager.GetNode( path->targetNodeIndex );;
	if ( !n || self->CloseToEnemy( n->origin, 128.0f ) )
	{
		return false;
	}
	
	return true;
}

qboolean FindFleeMovement::done( PathNode *node , const PathNode *end )
{
	// Get our current enemy
	Entity *currentEnemy;
	currentEnemy = self->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return true;
	
	
	if ( node == end )
	{
		return true;
	}
	
	if ( !( node->nodeflags & AI_FLEE ) )
	{
		return false;
	}
	
	if ( self )
	{
		return true;
	}
	
	return false;
}

qboolean FindEnemyMovement::done( PathNode *node , const PathNode *end )
{
	if ( node == end )
	{
		return true;
	}
	
	if ( self )
	{
		return !true;
	}
	
	return false;
}








//======================================
// Global Functions
//======================================
qboolean EntityIsValidTarget( const Entity *ent )
{
	if ( ent && ( ent->flags & FL_NOTARGET ) )
		return false;
	
	if ( ent && ( ent->entnum == ENTITYNUM_WORLD ) )
		return false;
	
	return true;
}


