//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/helper_node.h                               $
//  $Revision:: 36                                                         $
//  $Author:: Jmartel                                                      $
//  $Date:: 2/16/03 2:00p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// DESCRIPTION:

//
// Forward Declarations
//
class HelperNode;
class HelperNodeController;

// This is declared here, because we reference class actor, 
// but we can't just #include "actor.h" because of
// cyclical reference problem revolving around the
// the typedef'd HelperNodePtr.
class Actor;


#ifndef __HELPER_NODE_H__
#define __HELPER_NODE_H__  

#include "entity.h"
typedef SafePtr<HelperNode> HelperNodePtr;

extern Event EV_HelperNodeCommand;

// Spawn Flag Masks
#define NODETYPE_FLEE		(1<<0)
#define NODETYPE_WORK		(1<<1)
#define NODETYPE_ANIM	    (1<<2)
#define NODETYPE_COVER		(1<<3)
#define NODETYPE_PATROL		(1<<4)
#define NODETYPE_SNIPER		(1<<5)
#define NODETYPE_CUSTOM		(1<<6)
#define NODETYPE_COMBAT		(1<<7)

//Enums
typedef enum
	{
	COVER_TYPE_NONE,
	COVER_TYPE_CRATE,
	COVER_TYPE_WALL,
	COVER_TYPE_TOTAL_NUMBER
	} CoverType_t;

typedef enum
	{
	COVER_DIRECTION_NONE,
	COVER_DIRECTION_LEFT,
	COVER_DIRECTION_RIGHT,
	COVER_DIRECTION_ALL,
	COVER_DIRECTION_TOTAL_NUMBER
	} CoverDirection_t;

typedef enum
	{
	DESCRIPTOR_NONE,
	DESCRIPTOR_CORRIDOR,
	DESCRIPTOR_TOTAL_NUMBER
	} NodeDescriptorType_t;

typedef enum
	{
	WAITTYPE_ERROR,
	WAITTYPE_EVENT,
	WAITTYPE_ANIM,
	WAITTYPE_TIME,
	WAITTYPE_TOTAL_NUMBER
	} waitType_t;

typedef struct
	{
	str anim;
	waitType_t waitType;
	float	time;
	} customAnimListEntry_t;

// Created by the level to handle the archiving of HelperNodes
class HelperNodeController : public Entity
   {
   public:
		CLASS_PROTOTYPE( HelperNodeController );
							  HelperNodeController();
							 ~HelperNodeController();

			  void			  SendEventToNode( Event *ev );
      virtual void            Archive( Archiver &arc );
   };


class HelperNode : public Listener
  {
	public:
		CLASS_PROTOTYPE( HelperNode );
							  HelperNode();
							 ~HelperNode();

		// Core Functionality
		void				SetTarget		( Event *ev );
		void				SetTargetName	( Event *ev );
		void				SetAngle		( Event *ev );
		void				SetOrigin		( Event *ev );

		void				SetTarget		( const str &name   );
		void				SetTargetName	( const str &name   );
		void				SetAngle		( const Vector &ang );
		void				SetOrigin		( const Vector &pos );


		//Command
		virtual void		SendCommand			( Event *ev );

		//Add Animations to list
		virtual void		AddAnimation		( Event *ev );
		virtual void		AddAnimation		( const str &anim , waitType_t waitType , float time );
		

		//Event Interface
		virtual void		SetEntryThread		( Event *ev );				
		virtual void		SetExitThread		( Event *ev );				
		virtual void		SetAnim				( Event *ev );
		virtual void		SetAnimTarget		( Event *ev );
		virtual void		SetKillTarget		( Event *ev );
		virtual void		SetCustomType		( Event *ev );
		virtual void		SetID				( Event *ev );
		virtual void		SetAnimCount		( Event *ev );
		virtual void		SetMaxKills			( Event *ev );
		virtual void		SetWait				( Event *ev );
		virtual void		SetWaitRandom		( Event *ev );
		virtual void		SetMinHealth		( Event *ev );
		virtual void		SetMinEnemyRange	( Event *ev );
		virtual void		SetActivationRange	( Event *ev );
		virtual void		SetFlags			( Event *ev );
		virtual void		SetAnimActive		( Event *ev );
		virtual void		SetCoverActive		( Event *ev );
		virtual void		SetCoverType		( Event *ev );
		virtual void		SetCoverDir			( Event *ev );
		virtual void		SetWaitForAnim		( Event *ev );
		virtual void		SetCriticalChange	( Event *ev );
		virtual void		SetDescriptor		( Event *ev );   
		virtual void		SetPriority			( Event *ev );
		      

		//Mutators      
		virtual void		SetEntryThread		( const str& thread );
		virtual void		SetExitThread		( const str& thread );
		virtual void		SetAnim				( const str& anim );
		virtual void		SetAnimTarget		( const str& animtarget );
		virtual void		SetKillTarget		( const str& killtarget );
		virtual void		SetCustomType		( const str& customtype );
		virtual void		SetID				( int ID );
		virtual void		SetAnimCount		( int animcount );
		virtual void		SetMaxKills			( int maxkills );
		virtual void		SetWait				( float wait );
		virtual void		SetWaitRandom		( float wait );
		virtual void		SetMinHealth		( float health );
		virtual void		SetMinEnemyRange	( float range );
		virtual void		SetActivationRange	( float range );
		virtual void		SetFlags			( unsigned int flags );
		virtual void		SetAnimActive		( qboolean animactive );
		virtual void		SetCoverActive		( qboolean coveractive );
		virtual void		SetWaitForAnim		( qboolean wait );
		virtual void		SetCoverType		( CoverType_t covertype );
		virtual void		SetCoverDir			( CoverDirection_t coverdir );
		virtual void		SetCriticalChange	( qboolean change );
		virtual void		SetDescriptor		( NodeDescriptorType_t descriptor );
		virtual void		SetPriority			( float priority );
		virtual void		SetUser				( Listener *user );
		virtual void		SetLastUseTime		( float time )	{ _lastUseTime = time; }

      

		//Accessors
		const str&			GetEntryThread();
		const str&			GetExitThread();
		const str&			GetAnim();
		const str&			GetAnimTarget();
		const str&			GetKillTarget();
		const str&			GetCustomType();
		

		int					GetID();
		int					GetAnimCount();
		int					GetMaxKills();	
		int					GetAnimListIndex();
		int					GetAnimListCount();

		float				GetWaitTime();
		float				GetMinHealth();
		float				GetMinEnemyRange();
		float				GetActivationRange();
		float				GetPriority();
		float				GetLastUseTime() { return _lastUseTime; }


		CoverType_t				GetCoverType();
		CoverDirection_t		GetCoverDirection();
		NodeDescriptorType_t	GetDescriptor();
		customAnimListEntry_t*	GetNextAnimEntryFromList();
		customAnimListEntry_t*  GetCurrentAnimEntryFromList();
      
		//Queries
		qboolean			isOfType( unsigned int mask );
		qboolean			isAnimActive();
		qboolean			isCoverActive();
		qboolean			isWaitRandom();
		qboolean			isWaitForAnim();
		qboolean			isChanged();
		qboolean			isReserved();
		bool				isUsingAnimList();
		bool				isAnimListFinished();

		//Utility
		void				RunEntryThread();
		void				RunExitThread();
		void				ReserveNode();
		void				UnreserveNode();
		void				NextAnim();
      
		//For Convience
		Vector				origin;
		str					target;
		str					targetname;
		Vector				angles;
  
		//Static Functions
		static HelperNode* FindClosestHelperNode( Actor &self , int mask , float  maxDist , float minDistanceFromPlayer, bool unreserveCurrentNode = true );
		static HelperNode* FindClosestHelperNode( Actor &self , int mask , NodeDescriptorType_t descriptor , float maxDist );
		static HelperNode* FindClosestHelperNodeAtDistanceFrom( Actor &self , Entity *ent , int mask , NodeDescriptorType_t descriptor , float maxDistFromSelf , float minDistFromEnt );
		static HelperNode* FindClosestHelperNodeWithoutPathing( Actor &self, float  maxDist );	
		static HelperNode* FindClosestHelperNode( Actor &self , const str& customType , float maxDist );
		static HelperNode* FindClosestHelperNode( Actor &self , const str& targetName );
		static HelperNode* FindClosestHelperNodeThatCannotSeeEntity( Actor &self , int mask , unsigned int clipMask, float maxDist , float minDist , Entity* ent , float minDistFromPlayer );
		static HelperNode* FindHighestPriorityNode( Actor& self , const str& customType );
		static HelperNode* FindHighestPriorityNode( Actor& self, const str& customType , const str& targetedTo );		
		static HelperNode* GetTargetedHelperNode( const str& targetName );
		static HelperNode* FindHelperNodeClosestTo( Actor &self , Entity *ent , int mask , float maxDist );
		static HelperNode* FindHelperNodeClosestToWithoutPathing( Actor &self , Entity *ent , int mask , float maxDist );
		static waitType_t GetWaitType( const str& waitType );
		static void CleanupHelperNodeList();

		static int GetHelperNodeMask( const str& type );				
		static bool isHelperNodeInRange( Actor &self , int mask , float range );

		
		// Archiving
		void Archive( Archiver &arc );

	protected:
		void					_init();
		CoverType_t				_CoverTypeForString( const str& covertype );
		CoverDirection_t		_CoverDirectionForString( const str& coverdir );
		NodeDescriptorType_t	_DescriptorForString( const str& descriptor );
      
			
	private:
		str						_entryThread;
		str						_exitThread;
		str						_anim;
		str						_animtarget;
		str						_killtarget;
		str						_customType;

		int						_id;
		int						_animcount;
		int						_maxkills;

		qboolean				_animactive;
		qboolean				_coveractive;
		qboolean				_waitrandom;
		qboolean				_waitforanim;
		qboolean				_criticalchange;
		bool					_reserved;
 
		float					_waittime;
		float					_minhealth;
		float					_minenemyrange;
		float					_activationRange;
		float					_priority;
		float					_lastUseTime;
      
		CoverType_t				_covertype;
		CoverDirection_t		_coverdir;

		unsigned int			_nodeflags;  
		NodeDescriptorType_t	_descriptor;    
		
		
		ListenerPtr				_user;
		int						_customAnimListIndex;
		bool					_usingCustomAnimList;
		Container<customAnimListEntry_t*>		_customAnimList;
		
		

  };

inline void HelperNode::SetUser( Listener *user )
{
	_user = user;
}

inline int HelperNode::GetAnimListIndex()
{
	return _customAnimListIndex;
}

inline int HelperNode::GetAnimListCount()
{
	return _customAnimList.NumObjects();
}

inline bool HelperNode::isUsingAnimList()
{
	return _usingCustomAnimList;
}

inline void HelperNode::ReserveNode()
	{
	_reserved = true;
	}

inline void HelperNode::UnreserveNode()
	{
	_reserved = false;
	}

inline void HelperNode::Archive( Archiver &arc )
{
	int num , i;	
	customAnimListEntry_t* animEntry;

	Listener::Archive( arc );

	arc.ArchiveVector	( &origin );
	arc.ArchiveString	( &target );
	arc.ArchiveString	( &targetname );
	arc.ArchiveVector	( &angles );

	arc.ArchiveString	( &_entryThread	);
	arc.ArchiveString	( &_exitThread	);
	arc.ArchiveString	( &_anim );
	arc.ArchiveString	( &_animtarget );
	arc.ArchiveString	( &_killtarget );
	arc.ArchiveString	( &_customType );

	arc.ArchiveInteger	( &_id );
	arc.ArchiveInteger	( &_animcount );
	arc.ArchiveInteger	( &_maxkills );

	arc.ArchiveBoolean	( &_animactive );
	arc.ArchiveBoolean	( &_coveractive );
	arc.ArchiveBoolean	( &_waitrandom );
	arc.ArchiveBoolean	( &_waitforanim );
	arc.ArchiveBoolean	( &_criticalchange );
	arc.ArchiveBool		( &_reserved );

	arc.ArchiveFloat	( &_waittime );
	arc.ArchiveFloat	( &_minhealth );
	arc.ArchiveFloat	( &_minenemyrange );
	arc.ArchiveFloat	( &_activationRange );
	arc.ArchiveFloat    ( &_priority  );
	arc.ArchiveFloat	( &_lastUseTime );

	ArchiveEnum( _covertype,  CoverType_t          );
	ArchiveEnum( _coverdir,   CoverDirection_t     );

	arc.ArchiveUnsigned	( &_nodeflags  );
	ArchiveEnum( _descriptor, NodeDescriptorType_t );

	arc.ArchiveSafePointer( &_user );
	arc.ArchiveInteger ( &_customAnimListIndex );
	arc.ArchiveBool    ( &_usingCustomAnimList );

   	if ( arc.Saving() )
		{
		num = _customAnimList.NumObjects();

		arc.ArchiveInteger( &num );

		for ( i = 1 ; i <= num ; i++ )
			{
			animEntry = _customAnimList.ObjectAt( i );
			arc.ArchiveString ( &animEntry->anim );
			ArchiveEnum( animEntry->waitType, waitType_t );
			arc.ArchiveFloat( &animEntry->time );
			}
		}
	else
		{
		arc.ArchiveInteger( &num );
		_customAnimList.ClearObjectList();
		_customAnimList.Resize( num );

		for ( i = 1 ; i<= num ; i++ )
			{
			animEntry = new customAnimListEntry_t;
			arc.ArchiveString( &animEntry->anim );
			ArchiveEnum( animEntry->waitType, waitType_t );
			arc.ArchiveFloat( &animEntry->time );
			_customAnimList.AddObject( animEntry );
			}

		}

}



// Global Utility Functions
void AddHelperNodeToList( HelperNode* node );
void RemoveHelperNodeFromList( HelperNode* node );



#endif /* __HELPER_NODE_H__ */
