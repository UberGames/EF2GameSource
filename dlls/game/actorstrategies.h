//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/actorstrategies.h                              $
// $Revision:: 19                                                             $
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
// I am replacing the old way that Actor::Think was done by implementing a group of strategies for it
// instead.  This will form the foundation for new flexiblility within the actor class.  
//
// What I am trying to achieve is a specialized "think" for different types of actors.  A Boss, for instance,
// would use BossThink, an Non-Attacking-NPC could use NPCThink.  Using the event system we already have in place
// it will be easy to change thinking modalities on the fly, allowing us to make a cowardly NPC turn into a 
// roaring death machine if he gets shot.
//

#ifndef __ACTORSTRATEGIES_H__
#define __ACTORSTRATEGIES_H__

//============================
// Forward Declarations
//============================

class Actor;
class Entity;
class Archiver;

typedef SafePtr<Actor> ActorPtr;

//------------------------- CLASS ------------------------------
//
// Name:		ActorThink
// Base Class:	None
//
// Description:	Base class from which all Actor Think Strategies 
//				are derived.
//
// Method of Use:	
//				Deriving a new class from ActorThink is a good 
//				to allow Actor to exhibit new behavior. 
//
//--------------------------------------------------------------

class ActorThink 
{
public:
	virtual					~ActorThink() { }
	virtual void			DoArchive( Archiver &arc );
	virtual void			Think( Actor & ) = 0;
	virtual bool			isSimple( void ) { return false; }
	
protected:
	virtual void			ProcessBehaviors( Actor &actor );
	virtual void			DoMove( Actor &actor );
	virtual void			UpdateBossHealth( Actor &actor );
	virtual void			CheckGround( Actor &actor );
	virtual void			InanimateObject( Actor &actor );
	virtual void			TryDrown( Actor &actor );
	virtual void			ActorStateUpdate( Actor &actor );
	
};


//------------------------- CLASS ------------------------------
//
// Name:		DefaultThink
// Base Class:	ActorThink
//
// Description:	
//				Think class instantiated by most actors -- and 
//				instantiated in ALL actors by default
//
// Method of Use:	
//				This is the default behavior for Actor, nothing 
//				special need be done to use this class 
//
//--------------------------------------------------------------

class DefaultThink : public ActorThink
{
public:
	virtual void			Think( Actor &actor );
};

//------------------------- CLASS ------------------------------
//
// Name:		SimplifiedThink
// Base Class:	ActorThink
//
// Description:	
//				This is a light weight version of Default Think
//				The purpose of this class is to allow Actors that
//				move, have behaviors and accept messages without
//				the CPU overhead of doing DefaultThink
//
// Method of Use:	
//				Actors can be given this method of updating via
//				the script command "setsimplifiedthink"
//
//--------------------------------------------------------------

class SimplifiedThink : public ActorThink
{
public:
							SimplifiedThink( Actor *actor );
							~SimplifiedThink( void );
	virtual void			Think( Actor &actor );
	virtual void			DoArchive( Archiver &arc );
	virtual bool			isSimple( void ) { return true; }
							
protected:					
	virtual void			DoMove( Actor &actor );
private:
	const SimplifiedThink & operator=( const SimplifiedThink & );
	ActorPtr				_actor;
	int						_previousContents;
};

#endif /* __ACTORSTRATEGIES_H__ */
