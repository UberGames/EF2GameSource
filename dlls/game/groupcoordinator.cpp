//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/groupcoordinator.cpp                           $
// $Revision:: 22                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 5/17/03 10:29p                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//------------------------------------------------------------------------------

#include "_pch_cpp.h"
#include "groupcoordinator.hpp"
#include "actor.h"

//==============================================================================
//								        Group
//==============================================================================
CLASS_DECLARATION( Listener, Group, "group" )
{
	{ NULL, NULL }
};


//--------------------------------------------------------------
// Name:		Group()
// Class:		Group
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
Group::Group()
{
}


//--------------------------------------------------------------
// Name:		~Group()
// Class:		Group()
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
Group::~Group()
{
	ClearList();
}


//--------------------------------------------------------------
// Name:		AddMember()
// Class:		Group
//
// Description:	First Checks if entity to be added is already in
//				the list, if it is not, then it adds it.  
//				After it adds it to the list, it sets that entity's 
//				group number
//
// Parameters:	Entity *entity 
//
// Returns:		None
//--------------------------------------------------------------
void Group::AddMember( Entity *entity )
{
	// First Check if entity is in the list -- There might be
	// a faster way to do this inside the Container Class
	Entity *checkEntity;
	for ( int i = 1 ; i <= _memberList.NumObjects() ; i++ )
	{
		checkEntity = _memberList.ObjectAt( i );
		if ( !checkEntity )
			continue;
		
		if ( checkEntity == entity )
			return;
	}
	
	// We didn't find a match, so we'll add it.
	_memberList.AddObject( entity );
	entity->SetGroupID( _id );
}


//--------------------------------------------------------------
// Name:		RemoveMember( Entity *entity )
// Class:		Group
//
// Description:	Removes the entity from the list
//
// Parameters:	Entity *entity
//
// Returns:		None
//--------------------------------------------------------------
void Group::RemoveMember( Entity *entity )
{
	if ( _memberList.ObjectInList( entity ) )
	{
		_memberList.RemoveObject( entity );
	}
}


//--------------------------------------------------------------
// Name:		ClearList()
// Class:		Group
//
// Description:	Clears the member list
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void Group::ClearList()
{
	_memberList.FreeObjectList();
}


//--------------------------------------------------------------
// Name:		IsThisTargetNameInGroup()
// Class:		Group
//
// Description:	Checks if an entity with the specified target name
//				is in the group
//
// Parameters:	const str &name
//
// Returns:		true or false
//--------------------------------------------------------------
bool Group::IsThisTargetNameInGroup( const str &name )
{	
	Entity *checkEntity;
	str checkName;
	
	for ( int i = 1 ; i <= _memberList.NumObjects() ; i++ )
	{
		checkEntity = _memberList.ObjectAt( i );
		if ( !checkEntity )
			continue;
		
		if ( !stricmp( name.c_str() , checkEntity->TargetName() ) )
			return true;		
	}
	
	return false;
}


//--------------------------------------------------------------
// Name:		IsThisEntityInGroup()
// Class:		Group
//	
// Description:	Checks if this entity is in the group
//
// Parameters:	Entity *entity
//
// Returns:		true or false
//--------------------------------------------------------------
bool Group::IsThisEntityInGroup( Entity *entity )
{
	Entity *checkEntity;
	
	for ( int i = 1 ; i <= _memberList.NumObjects() ; i++ )
	{
		checkEntity = _memberList.ObjectAt( i );
		if ( !checkEntity )
			continue;
		
		if ( checkEntity == entity )
			return true;		
	}
	
	return false;
}

//--------------------------------------------------------------
// Name:		SendEventToGroup()
// Class:		Group
//
// Description:	Sends the specified event to every member of the 
//				group
//
// Parameters:	Event *ev -- The event to send
//
// Returns:		None
//--------------------------------------------------------------
void Group::SendEventToGroup( Event *ev )
{
	Entity *entity;
	Event  *copiedEvent;
	
	for ( int i = 1 ; i <= _memberList.NumObjects() ; i++ )
	{
		entity = _memberList.ObjectAt( i );
		
		if ( !entity )
			continue;
		
		copiedEvent = new Event( ev );
		entity->ProcessEvent( copiedEvent );
	}
}

//--------------------------------------------------------------
// Name:		SendEventToGroup()
// Class:		Group
//
// Description:	Sends the specified event to every member of the 
//				group
//
// Parameters:	Event &ev -- The event to send
//
// Returns:		None
//--------------------------------------------------------------
void Group::SendEventToGroup( Event &ev )
{
	Entity *entity;
	Event  *copiedEvent;
	
	for ( int i = 1 ; i <= _memberList.NumObjects() ; i++ )
	{
		entity = _memberList.ObjectAt( i );
		
		if ( !entity )
			continue;
		
		copiedEvent = new Event( ev );
		entity->ProcessEvent( copiedEvent );
	}
}

//--------------------------------------------------------------
// Name:		GroupCancelEventsOfType()
// Class:		Group
//
// Description:	Has each memeber of the group cancel its events
//				of the specified type
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Group::GroupCancelEventsOfType( Event *ev )
{
	Entity *entity;	
	
	for ( int i = 1 ; i <= _memberList.NumObjects() ; i++ )
	{
		entity = _memberList.ObjectAt( i );
		
		if ( !entity )
			continue;
		
		entity->CancelEventsOfType( ev );
	}
}

//--------------------------------------------------------------
// Name:		Archive()
// Class:		Group
//
// Description:	Archives the class
//
// Parameters:	Archive  &arc
//	
// Returns:		None
//----------------------------------------------------------
void Group::Archive( Archiver &arc )
{
	int num , i;
	EntityPtr ent;
	EntityPtr *entityPointer;
	
	Listener::Archive( arc );
	
	ent = NULL;
	if ( arc.Saving() )
	{
		num = _memberList.NumObjects();
		arc.ArchiveInteger( &num );
		
		for ( i = 1 ; i <= num ; i++ )
		{
			ent = _memberList.ObjectAt( i );		
			arc.ArchiveSafePointer( &ent );		
		}
	}
	else
	{
		arc.ArchiveInteger( &num );
		
		_memberList.ClearObjectList();
		_memberList.Resize( num );
		
		for ( i = 1 ; i <= num ; i++ )
		{		
			_memberList.AddObject( ent );
			
			entityPointer = &_memberList.ObjectAt( i );
			
			arc.ArchiveSafePointer( entityPointer );
		}
	}
	
	arc.ArchiveInteger( &_id );
	arc.ArchiveString( &_groupDeathThread );
}


//--------------------------------------------------------------
// Name:		GetNextMember()
// Class:		Group
//
// Description:	For return the next entity in the list after
//				the entity passed in
//
// Parameters:	Entity *entity -- The entity to use as a base index
//
// Returns:		Entity*
//--------------------------------------------------------------
Entity* Group::GetNextMember( Entity *entity )
{
	Entity *ent;
	
	if ( _memberList.NumObjects() == 0 )
		return NULL;
	
	if ( !entity )
		return _memberList.ObjectAt( 1 );
	
	
	for ( int i = 1 ; i <= _memberList.NumObjects() ; i++ )
	{
		ent = _memberList.ObjectAt( i );
		
		if ( i == _memberList.NumObjects() )
			return NULL;
		
		if ( ent == entity )
			return _memberList.ObjectAt( i + 1 );
	}
	
	return NULL;
}


//--------------------------------------------------------------
// Name:		GetPreviousMember
// Class:		Group
//
// Description:	Returns the entity in the list that occurs
//				just previous to the entity passed in
//
// Parameters:	Entity *entity
//
// Returns:		Entity*
//--------------------------------------------------------------
Entity* Group::GetPreviousMember( Entity *entity )
{
	Entity *ent;
	
	if ( _memberList.NumObjects() == 0 )
		return NULL;
	
	if ( !entity )
		return _memberList.ObjectAt( _memberList.NumObjects() );
	
	for ( int i = _memberList.NumObjects() ; i > 0 ; i-- )
	{
		ent = _memberList.ObjectAt( i );
		
		if ( i == 1 )
			return NULL;
		
		if ( ent == entity )
			return _memberList.ObjectAt( i - 1 );
	}
	
	return NULL;
}

void Group::RunGroupDeathThread(Entity *entity)
{
	if ( !_groupDeathThread.length() )
		return;
	
	ExecuteThread(_groupDeathThread, true, entity );
	
}

//==============================================================================
//								  Actor Group
//==============================================================================
CLASS_DECLARATION( Group, ActorGroup, "actorgroup" )
{
	{ NULL, NULL }
};

//--------------------------------------------------------------
// Name:		ActorGroup()
// Class:		ActorGroup
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
ActorGroup::ActorGroup()
{
}

//--------------------------------------------------------------
// Name:		~ActorGroup()
// Class:		ActorGroup
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
ActorGroup::~ActorGroup()
{
}


//--------------------------------------------------------------
// Name:		CountMembersWithThisName( const str &name )
// Class:		ActorGroup
//
// Description:	Loops through the group list and returns a count
//				of how many have a name that matches the passed
//				in name
//
// Parameters:	const str &name
//
// Returns:		int
//--------------------------------------------------------------
int ActorGroup::CountMembersWithThisName( const str &name )
{
	Entity *entity = 0;
	Actor *actor = 0;
	int count = 0;
	
	entity = GetNextMember( entity );
	if ( !entity )
		return 0;
	if ( !entity->isSubclassOf( Actor ) )
		return 0;
	
	while ( entity != NULL )
	{
		// Safeguard
		if ( entity->isSubclassOf( Actor ) )
		{
			actor = (Actor*)entity;
			if (!stricmp(name.c_str() , actor->name.c_str() ) )
				count++;
		}
		
		entity = GetNextMember( entity );
	}
	
	return count;
}


//--------------------------------------------------------------
//
// Name:			CountMembersAttackingEnemy
// Class:			ActorGroup
//
// Description:		Counts the number of actors in this group attacking
//					the specified enemy.  If the enemy is NULL, it counts
//					all actors in this group with the ACTOR_FLAG_ATTACKING_ENEMY 
//					set.
//
// Parameters:		Entity *enemy -- Enemy to test against
//
// Returns:			int 
//
//--------------------------------------------------------------
int ActorGroup::CountMembersAttackingEnemy( Entity *enemy )
{
	Entity *entity = 0;	
	Actor *actor = 0;
	
	int count = 0;
	
	entity = GetNextMember( entity );
	if ( !entity )
		return 0;
	if ( !entity->isSubclassOf( Actor ) )
		return 0;
	
	while ( entity != NULL )
	{
		// Safeguard
		if ( entity->isSubclassOf( Actor ) )
		{
			
			// This actor isn't attacking... NEXT!
			actor = (Actor*)entity;
			if ( !actor->GetActorFlag(ACTOR_FLAG_ATTACKING_ENEMY) )
			{
				entity = GetNextMember( entity );
				continue;
			}
			
			// If we were not supplied an enemy to check against, just 
			// increment the count, otherwise check to see if the enemy
			// matches.
			if ( !enemy )
				count++;
			else if ( actor->enemyManager->GetCurrentEnemy() == enemy )
				count++;
			
		}
		entity = GetNextMember( entity );
	}
	
	return count;
}

//==============================================================================
//								  Group Coordinator
//==============================================================================
CLASS_DECLARATION( Listener, GroupCoordinator, "groupcoordinator" )
{
	{ NULL, NULL }
};

//Global Safe Pointer
GroupCoordinatorPtr groupcoordinator;

//--------------------------------------------------------------
// Name:		GroupCoordinator()
// Class:		GroupCoordinator
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GroupCoordinator::GroupCoordinator()
{
}


//--------------------------------------------------------------
// Name:		~GroupCoordinator()
// Class:		GroupCoordinator
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
GroupCoordinator::~GroupCoordinator()
{
	ClearGroupList();
}


//--------------------------------------------------------------
// Name:		GetGroup()
// Class:		GroupCoordinator
//
// Description:	Returns a group pointer to the group with the 
//				specified ID
//
// Parameters:	int ID
//
// Returns:		Group*
//--------------------------------------------------------------
Group* GroupCoordinator::GetGroup( int ID )
{
	Group *group;
	group = NULL;
	
	for ( int i = 1; i <= _groupList.NumObjects() ; i++ )
	{
		group = _groupList.ObjectAt( i );
		
		if (  !group )
			continue;
		
		if ( group->GetGroupID() == ID )
			return group;
	}
	
	return NULL;
}

void GroupCoordinator::MemberDied( Entity *entity , int ID  )
{
	Group *group;	
	str threadName;
	int i;
	
	// Now see if the requested group exists
	for ( i = 1; i <= _groupList.NumObjects() ; i++ )
	{
		group = _groupList.ObjectAt( i );
		
		if ( !group )
			continue;
		
		if ( group->GetGroupID() == ID )
		{
			if (group->CountMembers() == 1 )
			{
				group->RunGroupDeathThread(entity);							
				break;
			}
		}
	}
	
	RemoveEntityFromGroup( entity , ID );
	
}

void GroupCoordinator::SetGroupDeathThread( const str &threadName , int ID )
{
	Group *group;	
	int i;
	
	// Now see if the requested group exists
	for ( i = 1; i <= _groupList.NumObjects() ; i++ )
	{
		group = _groupList.ObjectAt( i );
		
		if ( !group )
			continue;
		
		if ( group->GetGroupID() == ID )
		{
			group->SetGroupDeathThread( threadName );
			return;
		}
	}
}

//--------------------------------------------------------------
// Name:		AddEntityToGroup()
// Class:		GroupCoordinator
//
// Description:	If the group exists, it tells the the group to 
//				add the specified entity, if it does not exist
//				a new group is created and is then told to 
//				add the specified entity
//
// Parameters:	Entity *ent
//				int ID
//
// Returns:		None
//--------------------------------------------------------------
void GroupCoordinator::AddEntityToGroup( Entity *ent , int ID )
{
	Group *group;	
	int i;
	
	// First see if the entity is in any other group
	for ( i = 1; i <= _groupList.NumObjects() ; i++ )
	{
		group = _groupList.ObjectAt( i );
		
		if ( !group )
			continue;
		
		if ( group->IsThisEntityInGroup( ent ) && group->GetGroupID() != ID )
			group->RemoveMember( ent );
	}
	
	// Now see if the requested group exists
	for ( i = 1; i <= _groupList.NumObjects() ; i++ )
	{
		group = _groupList.ObjectAt( i );
		
		if ( !group )
			continue;
		
		if ( group->GetGroupID() == ID )
		{
			group->AddMember( ent );
			return;
		}
	}
	
	// Group didn't already exist, so lets make a new one
	group = new Group;
	group->SetGroupID( ID );
	group->AddMember( ent );
	
	_groupList.AddObject( group );
}

void GroupCoordinator::RemoveEntityFromGroup( Entity *ent , int ID )
{
	Group *group;	
	int i;
	
	// Now see if the requested group exists
	for ( i = 1; i <= _groupList.NumObjects() ; i++ )
	{
		group = _groupList.ObjectAt( i );
		
		if ( !group )
			continue;
		
		if ( group->GetGroupID() == ID )
		{
			group->RemoveMember( ent );
			return;
		}
	}
}

//--------------------------------------------------------------
// Name:		RemoveGroup()
// Class:		GroupCoordinator
//
// Description:	Removes a group from the group list
//
// Parameters:	int ID
//
// Returns:		None
//--------------------------------------------------------------
void GroupCoordinator::RemoveGroup( int ID )
{
	Group *group;
	
	
	for ( int i = _groupList.NumObjects(); i > 0  ; i-- )
	{
		group = _groupList.ObjectAt( i );
		
		if (  !group )
			continue;
		
		if ( group->GetGroupID() == ID )
		{
			_groupList.RemoveObjectAt( i );
			delete group;
			group = NULL;									
			return;
		}
	}	
}


//--------------------------------------------------------------
// Name:		ClearGroupList()
// Class:		GroupCoordinator
//
// Description:	Clears the Group List
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void GroupCoordinator::ClearGroupList()
{
	Group *group;
	
	for ( int i = _groupList.NumObjects(); i > 0  ; i-- )
	{
		group = _groupList.ObjectAt( i );
		
		if (  !group )
			continue;
		
		RemoveGroup( group->GetGroupID() );
	}	
}


//--------------------------------------------------------------
// Name:		SendEventToGroup
// Class:		GroupCoordinator
//
// Description:	Sends the specified event to the group
//				so that the group will send it to everyone. This function
//				always deletes the original event when it's done with it.
//
// Parameters:	Event *ev
//				int ID
//
// Returns:		None
//--------------------------------------------------------------
void GroupCoordinator::SendEventToGroup( Event *ev , int ID)
{
	Group *group;
	
	if ( !ev )
		return;

	for ( int i = 1; i <= _groupList.NumObjects() ; i++ )
	{
		group = _groupList.ObjectAt( i );
		
		if (  !group )
			continue;
		
		if ( group->GetGroupID() == ID )
		{
			group->SendEventToGroup( ev );
			break;
		}
	}	
	
	// Clean up the event;
	delete ev;
	ev = NULL;
}

//--------------------------------------------------------------
// Name:		SendEventToGroup
// Class:		GroupCoordinator
//
// Description:	Sends the specified event to the group
//				so that the group will send it to everyone
//
// Parameters:	Event &ev
//				int ID
//
// Returns:		None
//--------------------------------------------------------------
void GroupCoordinator::SendEventToGroup( Event &ev , int ID )
{
	Group *group;

	for ( int i = 1; i <= _groupList.NumObjects() ; i++ )
	{
		group = _groupList.ObjectAt( i );
		
		if (  !group )
			continue;
		
		if ( group->GetGroupID() == ID )
		{
			group->SendEventToGroup( ev );
			return;
		}
	}	
}

//--------------------------------------------------------------
// Name:		GroupCancelEventsOfType()
// Class:		GroupCoordinator
//
// Description:	Gets the appropriate group, and has the 
//				group cancel events of the specified type
//
// Parameters:	Event *ev
//				int ID
//
// Returns:		None
//--------------------------------------------------------------
void GroupCoordinator::GroupCancelEventsOfType( Event *ev , int ID )
{
	Group *group;
	
	if ( !ev )
		return;

	for ( int i = 1; i <= _groupList.NumObjects() ; i++ )
	{
		group = _groupList.ObjectAt( i );
		
		if (  !group )
			continue;
		
		if ( group->GetGroupID() == ID )
		{
			group->GroupCancelEventsOfType( ev );
			break;
		}
	}
	
	delete ev;
}

//--------------------------------------------------------------
// Name:		Archive()
// Class:		GroupCoordinator
//
// Description:	Archives the class
//
// Parameters:	Archiver &arc
//	
// Returns:		None
//----------------------------------------------------------
void GroupCoordinator::Archive( Archiver &arc )
{
	int num , i;
	Group *group;
	//Group **groupPointer;
	Listener::Archive( arc );
	
	group = NULL;
	if ( arc.Saving() )
	{
		num = _groupList.NumObjects();
		arc.ArchiveInteger( &num );
		
		for ( i = 1 ; i <= num ; i++ )
		{
			group = _groupList.ObjectAt( i );
			arc.ArchiveObject( group );
		}
	}
	else
	{
		arc.ArchiveInteger( &num );
		
		_groupList.ClearObjectList();
		_groupList.Resize( num );
		
		for ( i = 1 ; i <= num ; i++ )
		{		
			group = new Group;
			
			_groupList.AddObject( group );
			
			arc.ArchiveObject( group );
		}
	}
}


