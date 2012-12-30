//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/groupcoordinator.h                             $
// $Revision:: 158                                                            $
//   $Author:: Sketcher                                                       $
//     $Date:: 4/11/02 11:34a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Classes for groups and group coordination.
//


#ifndef __GROUP_COORDINATOR_HPP__
#define __GROUP_COORDINATOR_HPP__

#include "entity.h"

//------------------------- CLASS ------------------------------
//
// Name:			Group
// Base Class:		None
//
// Description:		Base Class for Groups
//
// Method of Use:	Instantiated by the group coordinator
//					to maintain group data
//--------------------------------------------------------------
class Group	: public Listener
{
	public:
		CLASS_PROTOTYPE( Group );
		 Group();
		~Group();

		virtual void AddMember				 ( Entity *entity );
		virtual void RemoveMember			 ( Entity *entity );
		virtual void SetGroupDeathThread	 ( const str &threadName );		
		virtual void ClearList				 ();
		virtual void RunGroupDeathThread	 ( Entity *entity );
		virtual int  CountMembers			 ();
		bool		 IsThisTargetNameInGroup ( const str &name );
		bool		 IsThisEntityInGroup     ( Entity *entity  );
		void		 SendEventToGroup		 ( Event *ev );	
		void		 SendEventToGroup		 ( Event &ev );
		void		 GroupCancelEventsOfType ( Event *ev );

		int			 GetGroupID				 ();
		void         SetGroupID				 ( int ID );
		Entity*		 GetNextMember			 ( Entity *entity );
		Entity*		 GetPreviousMember		 ( Entity *entity );

		virtual void Archive				 ( Archiver &arc );

	protected:

	private:
		Container< EntityPtr >	_memberList;		
		int						_id;
		str						_groupDeathThread;
};

inline int Group::CountMembers()
{
	return _memberList.NumObjects();
}

inline int Group::GetGroupID()
{
	return _id;
}

inline void Group::SetGroupID( int ID )
{
	_id = ID;
}

inline void Group::SetGroupDeathThread( const str &threadName )
{
	_groupDeathThread = threadName;
}





//------------------------- CLASS ------------------------------
//
// Name:			ActorGroup
// Base Class:		Group
//
// Description:		Group specifically for Actors
//
// Method of Use:	Instantiated by the group coordinator
//
//--------------------------------------------------------------
class ActorGroup : public Group
{
	public:
		CLASS_PROTOTYPE( ActorGroup );
		 ActorGroup();
		~ActorGroup();

		int CountMembersWithThisName( const str &name );
		int CountMembersAttackingEnemy( Entity *enemy = 0);

	protected:
	private:	
};







//------------------------- CLASS ------------------------------
//
// Name:			GroupCoordinator
// Base Class:		None
//
// Description:		Maintains and handles groups
//
// Method of Use:	Instantiated by the level.
//--------------------------------------------------------------
class GroupCoordinator : public Listener
{
	public:
		CLASS_PROTOTYPE( GroupCoordinator );
		 GroupCoordinator();
		~GroupCoordinator();

		Group* GetGroup			( int ID );
		void   AddEntityToGroup	( Entity *ent , int ID );
		void   RemoveEntityFromGroup ( Entity *ent , int ID );		
		int    CountGroups		();
		void   RemoveGroup		( int ID );
		void   ClearGroupList	();
		void   SendEventToGroup	( Event *ev , int ID );	
		void   SendEventToGroup ( Event &ev , int ID );
		void   GroupCancelEventsOfType ( Event *ev , int ID );
		void   MemberDied			 ( Entity *entity , int ID );
		void   SetGroupDeathThread	 ( const str &threadName , int ID );	
		virtual void Archive	( Archiver &arc );

	protected:		
		
	private:
		Container< Group* > _groupList;
};

inline int GroupCoordinator::CountGroups()
{
	return _groupList.NumObjects();
}


typedef SafePtr<GroupCoordinator> GroupCoordinatorPtr;
extern GroupCoordinatorPtr groupcoordinator;

#endif /* __GROUP_COORDINATOR_HPP__ */
