//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/helper_node.cpp                            $
// $Revision:: 50                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//


#include "_pch_cpp.h"
//#include "g_local.h"
#include "helper_node.h"
#include "actor.h"

extern Event EV_SpawnFlags;
//
//======================================
// Helper Node Event Declaration
//======================================
//

Event EV_HelperNodeController_SendEvent
(
	"sendevent",
	EV_DEFAULT,
	"ssSSSSSSS",
	"targetname eventName token1 token2 token3 token4 token5 token6 token7",
	"Sends the event to the specified node"
);
Event EV_HelperNode_SetNodeAnim
(
	"setnodeanim",
	EV_SCRIPTONLY,
	"s",
	"animation",
	"Sets the animation that will be played by the actor on this node"
);
Event EV_HelperNode_SetNodeEntryThread
(
	"setnodeentrythread",
	EV_SCRIPTONLY,
	"s",
	"thread",
	"Sets the thread that will be called by the actor when it reaches this node"
);
Event EV_HelperNode_SetNodeExitThread
(
	"setnodeexitthread",
	EV_SCRIPTONLY,
	"s",
	"thread",
	"Sets the thread that will be called by the actor when it leaves this node"
);
Event EV_HelperNode_SetAnimCount
(
	"animcount",
	EV_SCRIPTONLY,
	"i",
	"num_times",
	"Sets the max number of times that an animation can be set on this node"
);
Event EV_HelperNode_SetAnimTarget
(
	"animtarget",
	EV_SCRIPTONLY,
	"s",
	"targetname",
	"Allows you to specify a target name to apply the animation to"
);
Event EV_HelperNode_SetAnimActive
(
	"animactive",
	EV_SCRIPTONLY,
	"b",
	"active",
	"Specifies if the animation component is active"
);
Event EV_HelperNode_SetCoverType
(
	"setcovertype",
	EV_SCRIPTONLY,
	"s",
	"cover_type",
	"Specifies the covertype of the node"
);
Event EV_HelperNode_SetCoverDirection
(
	"setcoverdir",
	EV_SCRIPTONLY,
	"s",
	"cover_dir",
	"Specifies the direction from the cover that the AI can attack"
);
Event EV_HelperNode_SetWait
(
	"wait",
	EV_SCRIPTONLY,
	"f",
	"time",
	"Makes a patrolling AI pause at this node for the specified time"
);
Event EV_HelperNode_SetWaitRandom
(
	"waitrandom",
	EV_SCRIPTONLY,
	"f",
	"max_time",
	"Makes a patrolling AI pause randomly for a min time of 1 to a max time of specified"
);
Event EV_HelperNode_WaitForAnim
(
	"waitforanim",
	EV_SCRIPTONLY,
	"b",
	"wait_for_anim",
	"Makes the AI wait until the anim is finished before finishing a behavior"
);
Event EV_HelperNode_SetKillTarget
(
	"attacktarget",
	EV_SCRIPTONLY,
	"s",
	"targetname",
	"Specifies a target for a sniping AI to attack"
);
Event EV_HelperNode_SetCustomType
(
	"setcustomtype",
	EV_SCRIPTONLY,
	"s",
	"type",
	"Specifies a custom helper node type"
);
Event EV_HelperNode_SetMaxKills
(
	"maxkills",
	EV_SCRIPTONLY,
	"i",
	"max_kills",
	"Specifies the maximum number of kills that can be done from this node"
);
Event EV_HelperNode_SetMinHealth
(
	"minhealth",
	EV_SCRIPTONLY,
	"f",
	"min_health",
	"Specifies the minimum health an actor is allowed to get to before leaving this node"
);
Event EV_HelperNode_SetMinEnemyRange
(
	"minenemyrange",
	EV_SCRIPTONLY,
	"f",
	"min_range",
	"Specifies the minimum range an enemy is allowed to get before actor leaves this node"
);
Event EV_HelperNode_SetActivationRange
(
	"activationrange",
	EV_SCRIPTONLY,
	"f",
	"activation_range",
	"Specifies the range an actor needs to be to consider using the node"
);
Event EV_HelperNode_SetPriority
(
	"priority",
	EV_SCRIPTONLY,
	"f",
	"priority",
	"Specfies the priority for this node"
);
Event EV_HelperNode_SetNodeID
(
	"nodeID",
	EV_SCRIPTONLY,
	"i",
	"node_id",
	"Assigns an ID to this node"
);
Event EV_HelperNode_SetCriticalChange
(
	"criticalchange",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Alerts the node to a critical change"
);
Event EV_HelperNode_SetDescriptor
( 
	"descriptor",
	EV_SCRIPTONLY,
	"s",
	"descriptorValue",
	"sets an additional descriptor for a node"
);
Event EV_HelperNode_SendCommand
(
	"sendnodecommand",
	EV_SCRIPTONLY,
	"s",
	"commandToSend",
	"sends an event to the user of the node"
);
Event EV_HelperNode_AddAnimation
(
	"addanim",
	EV_SCRIPTONLY,
	"ss",
	"anim waittype",
	"Adds an animation to the custom anim list"
);

//======================================
// Helper Node Container
//======================================

Container<HelperNode*> HelperNodes;

char CoverTypeStrings[ COVER_TYPE_TOTAL_NUMBER ][ 32 ] =
{
	"none",
	"crate",
	"wall"
};

char CoverDirStrings[ COVER_DIRECTION_TOTAL_NUMBER ][ 32 ] =
{
	"none",
	"left",
	"right",
	"all"
};

char DescriptorTypeStrings[ DESCRIPTOR_TOTAL_NUMBER ][ 32 ] =
{
	"none",
	"corridor"
};



//======================================
// Helper Node Initialization
//======================================
CLASS_DECLARATION( Entity, HelperNodeController, "helpernodecontroller" )
{
	{ &EV_HelperNodeController_SendEvent,				&HelperNodeController::SendEventToNode },

	{ NULL, NULL }
};

HelperNodeController::HelperNodeController()
{
	targetname = "nodecontroller";
}

HelperNodeController::~HelperNodeController()
{
}


void HelperNodeController::SendEventToNode( Event *ev )
{
	str nodeTargetName;	
	str eventName;
	Event *nodeEvent;
	HelperNode* node;
	int i;
	
	//First grab our node target and event
	nodeTargetName = ev->GetString( 1 );
	eventName	   = ev->GetString( 2 );
	
	node = NULL;
	//Now lets find our target
	for ( i = HelperNodes.NumObjects() ; i > 0 ; i-- )
	{
		node = HelperNodes.ObjectAt( i );
		if ( node->targetname == nodeTargetName )
			break;            
	}
	
	if ( !node )
		return;
	
	// Create our event
	nodeEvent = new Event( eventName.c_str() );
	if ( !nodeEvent )
		return;
	
	
	//Load our event with parameters	
	for ( i = 3 ; i <= ev->NumArgs() ; i++ ) //start at 3, because we burnt through the first 2 already
	{
		nodeEvent->AddToken( ev->GetToken( i ) );
	}
	
	node->ProcessEvent( nodeEvent );
}

//
// Class Declaration and Event Registration
//
CLASS_DECLARATION( Listener, HelperNode, "info_helpernode" )
{
	// Core Events
	{ &EV_SetTargetName,						&HelperNode::SetTargetName      },
	{ &EV_SetTarget,							&HelperNode::SetTarget          },
	{ &EV_SetAngle,								&HelperNode::SetAngle           },
	{ &EV_SetOrigin,							&HelperNode::SetOrigin          },
	{ &EV_SpawnFlags,							&HelperNode::SetFlags           },
	{ &EV_HelperNode_SetNodeAnim,				&HelperNode::SetAnim            },
	{ &EV_HelperNode_SetNodeEntryThread,		&HelperNode::SetEntryThread     },
	{ &EV_HelperNode_SetNodeExitThread,			&HelperNode::SetExitThread      },
	{ &EV_HelperNode_SetAnimCount,				&HelperNode::SetAnimCount       },
	{ &EV_HelperNode_SetAnimTarget,				&HelperNode::SetAnimTarget      },
	{ &EV_HelperNode_SetAnimActive,				&HelperNode::SetAnimActive      },
	{ &EV_HelperNode_SetCoverType,				&HelperNode::SetCoverType       },
	{ &EV_HelperNode_SetCoverDirection,			&HelperNode::SetCoverDir        },
	{ &EV_HelperNode_SetWait,					&HelperNode::SetWait            },
	{ &EV_HelperNode_SetWaitRandom,				&HelperNode::SetWaitRandom      },
	{ &EV_HelperNode_WaitForAnim,				&HelperNode::SetWaitForAnim     },
	{ &EV_HelperNode_SetKillTarget,				&HelperNode::SetKillTarget      },
	{ &EV_HelperNode_SetCustomType,				&HelperNode::SetCustomType      },
	{ &EV_HelperNode_SetMaxKills,				&HelperNode::SetMaxKills        },
	{ &EV_HelperNode_SetMinHealth,				&HelperNode::SetMinHealth       },
	{ &EV_HelperNode_SetMinEnemyRange,			&HelperNode::SetMinEnemyRange   },
	{ &EV_HelperNode_SetActivationRange,		&HelperNode::SetActivationRange },
	{ &EV_HelperNode_SetPriority,				&HelperNode::SetPriority        },
	{ &EV_HelperNode_SetNodeID,					&HelperNode::SetID              },
	{ &EV_HelperNode_SetCriticalChange,			&HelperNode::SetCriticalChange  },
	{ &EV_HelperNode_SetDescriptor,				&HelperNode::SetDescriptor      },
	{ &EV_HelperNode_SendCommand,				&HelperNode::SendCommand        },
	{ &EV_HelperNode_AddAnimation,				&HelperNode::AddAnimation       },
	{ NULL, NULL }
};


//
// Name:        HelperNode()
// Class:       HelperNode
//
// Description: Constructor
//
// Parameters:  None
//
// Returns:     None
//
HelperNode::HelperNode()
{
	if ( !LoadingSavegame )
	{
		AddHelperNodeToList( this );		
		_init();	
	}
}


//
// Name:        ~HelperNode()
// Class:        HelperNode
//
// Description:  Destructor
//
// Parameters:   None
//
// Returns:      None
//
HelperNode::~HelperNode()
{
	customAnimListEntry_t* checkEntry;
	
   	for ( int i = _customAnimList.NumObjects() ; i > 0 ; i-- )
	{
		checkEntry = _customAnimList.ObjectAt( i );
		delete checkEntry;
		checkEntry = NULL;
		_customAnimList.RemoveObjectAt( i );
	}
}


//
// Name:         _init()
// Class:        HelperNode
//
// Description:  Initializes default values
//
// Parameters:   None
//
// Returns:      None
//
void HelperNode::_init()
{
	_entryThread = "";
	_exitThread  = "";
	_anim        = "";
	_animtarget  = "";
	_killtarget  = "";
	
	// Default Value for _id means the node is open to everyone
	_id         = -1;
	
	// Default Value for _animcount means no maximum
	_animcount  = -1;
	
	// Default Value for _maxkills means no maximum
	_maxkills   = -1;
	
	
	_animactive     = true;
	_coveractive    = true;
	_waitrandom     = false;
	_waitforanim    = false;
	_criticalchange = false;
	
	_waittime        = 0.0f;
	_minhealth       = 0.0f;
	_minenemyrange   = 0.0f;
	_activationRange = 0.0f;
	_priority        = 0.0f;
	_lastUseTime		= 0.0f;
	
	_covertype  = COVER_TYPE_NONE;
	_coverdir   = COVER_DIRECTION_NONE;
	_descriptor = DESCRIPTOR_NONE;
	
	_reserved = false;   
	_nodeflags = 0;     
	
	_customAnimListIndex = 1;
	_usingCustomAnimList = false;
	_user = NULL;
	
}

//======================================
// Core Functionality
//======================================
void HelperNode::SetTarget( Event *ev )
{
	str name;
	name = ev->GetString( 1 );
	
	SetTarget(name);
}

void HelperNode::SetTargetName( Event *ev )
{
	str name;
	name = ev->GetString( 1 );
	
	SetTargetName(name);
}

void HelperNode::SetAngle( Event *ev )
{
	Vector movedir;
	
	movedir = G_GetMovedir( ev->GetFloat( 1 ) );
	SetAngle(movedir.toAngles());
}

void HelperNode::SetOrigin( Event *ev )
{
	Vector pos;
	pos = ev->GetVector( 1 );
	
	SetOrigin(pos);
}

void HelperNode::SetTarget( const str &name   )
{
	target = name;
}

void HelperNode::SetTargetName( const str &name   )
{
	targetname = name;
}

void HelperNode::SetAngle( const Vector &ang )
{
	angles = ang;   
}

void HelperNode::SetOrigin( const Vector &pos )
{
	origin = pos;
}

//======================================
// Helper Node Event Interface
//======================================

//
// Name:        SetThread()
// Class:       HelperNode
//
// Description: Calls SetEntryThread()
//
// Parameters:  Event *ev - Event containing the string
//
// Returns:     None
//
void HelperNode::SetEntryThread( Event *ev )
{
	SetEntryThread(ev->GetString( 1 ) );
}


//
// Name:        SetExitThread()
// Class:       HelperNode
//
// Description: Calls SetExitThread()
//
// Parameters:  Event *ev -- Event containing the string
//
// Returns:     None
//
void HelperNode::SetExitThread( Event *ev )
{
	SetExitThread(ev->GetString( 1 ) );   
}

//
// Name:        SetAnim()
// Class:       HelperNode
//
// Description: Calls SetAnim()
//
// Parameters:  Event *ev - Event containing the string
//
// Returns:     None
//
void HelperNode::SetAnim( Event *ev )
{
	SetAnim( ev->GetString( 1 ) );
}


//
// Name:        SetAnimTarget()
// Class:       HelperNode
//
// Description: Calls SetAnimTarget()
//
// Parameters:  Event *ev - Event containing the string
//
// Returns:     None
//
void HelperNode::SetAnimTarget( Event *ev )
{
	SetAnimTarget( ev->GetString( 1 ) );
}


//
// Name:        SetKillTarget()
// Class:       HelperNode
//
// Description: Calls SetKillTarget()
//
// Parameters:  Event *ev - Event contaning the string
//
// Returns:     None
//
void HelperNode::SetKillTarget( Event *ev )
{
	SetKillTarget( ev->GetString( 1 ) );
}


//
// Name:        SetCustomType()
// Class:       HelperNode
//
// Description: Calls SetCustomType()
//
// Parameters:  Event *ev - Event contaning the string
//
// Returns:     None
//
void HelperNode::SetCustomType( Event *ev )
{
	SetCustomType( ev->GetString( 1 ) );
}

//
// Name:        SetID()
// Class:       HelperNode
//
// Description: Calls SetID()
//
// Parameters:  Event *ev - Event containing the integer
//
// Returns:     None
//
void HelperNode::SetID( Event *ev )
{
	SetID(ev->GetInteger( 1 ) );
}


//
// Name:        SetAnimCount()
// Class:       HelperNode
//
// Description: Calls SetAnimCount()
//
// Parameters:  Event *ev - Event containing the integer
//
// Returns:     None
//
void HelperNode::SetAnimCount( Event *ev )
{
	SetAnimCount( ev->GetInteger( 1 ) );
}


//
// Name:        SetMaxKills()
// Class:       HelperNode
//
// Description: Calls SetMaxKills()
//
// Parameters:  Event *ev - Event containing the integer
//
// Returns:     None
//
void HelperNode::SetMaxKills( Event *ev )
{
	SetMaxKills( ev->GetInteger( 1 ) );
}


//
// Name:        SetWait()
// Class:       HelperNode
//
// Description: Calls SetWait()
//
// Parameters:  Event *ev - Event containing the float
//
// Returns:     None
//
void HelperNode::SetWait( Event *ev )
{
	SetWait( ev->GetFloat( 1 ) );
}


//
// Name:        SetWaitRandom()
// Class:       HelperNode
//
// Description: Calls SetWaitRandom()
//
// Parameters:  Event *ev - Event containing the float
//
// Returns:     None
//
void HelperNode::SetWaitRandom( Event *ev )
{
	SetWaitRandom( ev->GetFloat( 1 ) );
}


//
// Name:        SetWaitForAnim()
// Class:       HelperNode
//
// Description: Calls SetWaitForAnim()
//
// Parameters:  Event *ev
//
// Returns:     None
//
void HelperNode::SetWaitForAnim( Event *ev )
{
	SetWaitForAnim( ev->GetBoolean( 1 ) );
}


//
// Name:        SetCriticalChange()
// Class:       HelperNode
//
// Description: Calls SetCriticalChange()
//
// Parameters:  Event *ev
//
// Returns:     None
//
void HelperNode::SetCriticalChange( Event * )
{
	SetCriticalChange( true );
}


//
// Name:        SetDescriptor()
// Class:       HelperNode
//
// Description: Gets the enum for the string and calls
//              SetDescriptor()
//
// Parameters:  Event *ev -- The event containing the string
//
// Returns:     None
//
void HelperNode::SetDescriptor( Event *ev )
{
	str descriptor = ev->GetString( 1 );
	SetDescriptor( _DescriptorForString(descriptor) );
}


void HelperNode::SetPriority( Event *ev )
{
	float priority = ev->GetFloat( 1 );
	SetPriority( priority );
}

//
// Name:         SetMinHealth()
// Class:        HelperNode
//
// Description:  Calls SetMinHealth()
//
// Parameters:   Event *ev - Event containing the float
//
// Returns:      None
//
void HelperNode::SetMinHealth( Event *ev )
{
	SetMinHealth( ev->GetFloat( 1 ) );
}


//
// Name:        SetMinEnemyRange()
// Class:       HelperNode
//
// Description: Calls SetMinEnemyRange()
//
// Parameters:  Event *ev - Event containing the float
//
// Returns:     None
//
void HelperNode::SetMinEnemyRange( Event *ev )
{
	SetMinEnemyRange( ev->GetFloat( 1 ) );
}


void HelperNode::SetActivationRange( Event *ev )
{	
	SetActivationRange( ev->GetFloat( 1 ) );	
}

//
// Name:        SetFlags()
// Class:       HelperNode
//
// Description: Calls SetFlags()
//
// Parameters:  Event *ev - Event containing the float
//
// Returns:     None
//
void HelperNode::SetFlags( Event *ev )
{
	SetFlags(ev->GetInteger( 1 ) );
}


//
// Name:        SetAnimActive()
// Class:       HelperNode
//
// Description: Calls SetAnimActive()
//
// Parameters:  Event *ev - Event containing the boolean
//
// Returns:     None
//
void HelperNode::SetAnimActive( Event *ev )
{
	SetAnimActive(ev->GetBoolean( 1 ) );
}


//
// Name:        SetCoverActive()
// Class:       HelperNode
//
// Description: Calls SetCoverActive()
//
// Parameters:  Event *ev - Event containing the boolean
//
// Returns:     None
//
void HelperNode::SetCoverActive( Event *ev )
{
	SetCoverActive(ev->GetBoolean( 1 ) );
}

//
// Name:        SetCoverType()
// Class:       HelperNode
//
// Description: Calls SetCoverType()
//
// Parameters:  Event *ev - Event containing the string
//
// Returns:     None
//
void HelperNode::SetCoverType( Event *ev )
{
	CoverType_t coverType;
	coverType = _CoverTypeForString( ev->GetString( 1 ) );
	
	SetCoverType(coverType);
}


//
// Name:        SetCoverDir()
// Class:       HelperNode
//
// Description: Calls SetCoverDir()
//
// Parameters:  Event *ev - Event containing the string
//
// Returns:     None
//
void HelperNode::SetCoverDir( Event *ev )
{  
	CoverDirection_t coverDir;
	coverDir = _CoverDirectionForString( ev->GetString( 1 ) );
	
	SetCoverDir(coverDir);
}

//======================================
// Helper Node Mutators
//======================================

//
// Name:        SetEntryThread()
// Class:       HelperNode
//
// Description: Sets the _entryThread variable
//
// Parameters:  const str& thread - the thread to set
//
// Returns:     None
//
void HelperNode::SetEntryThread( const str& thread )
{
	_entryThread = thread;
}


//
// Name:        SetExitThread()
// Class:       HelperNode
//
// Description: Sets the _exitThread variable
//
// Parameters:  const str& thread - the thread to set
//
// Returns:     None
//

void HelperNode::SetExitThread( const str& thread )
{
	_exitThread = thread;
}

//
// Name:        SetAnim()
// Class:       HelperNode
//
// Description: Sets the _anim variable
//
// Parameters:  const str& anim
//
// Returns:     None
//
void HelperNode::SetAnim( const str& anim )
{
	_anim = anim;
}


//
// Name:        SetAnimTarget()
// Class:       HelperNode
//
// Description: Sets the _animtarget variable
//
// Parameters:  const str& animtarget
//
// Returns:     None
//
void HelperNode::SetAnimTarget( const str& animtarget )
{
	_animtarget = animtarget;
}


//
// Name:        SetKillTarget()
// Class:       HelperNode
//
// Description: Sets the _killtarget variable
//
// Parameters:  const str& killtarget
//
// Returns:     None
//
void HelperNode::SetKillTarget( const str& killtarget )
{
	_killtarget = killtarget;
}


//
// Name:        SetCustomType()
// Class:       HelperNode
//
// Description: Sets the _customType variable
//
// Parameters:  const str& customtype
//
// Returns:     None
//
void HelperNode::SetCustomType( const str& customtype )
{
	_customType = customtype;
}

//
// Name:        SetID()
// Class:       HelperNode
//
// Description: Sets the _id variable
//
// Parameters:  int ID - The id to set
//
// Returns:     None
//
void HelperNode::SetID( int ID )
{
	_id = ID;
}


//
// Name:        SetAnimCount()
// Class:       HelperNode
//
// Description: Sets the _animcount variable
//
// Parameters:  int animcount
//
// Returns:     None
//
void HelperNode::SetAnimCount( int animcount ) 
{
	_animcount = animcount;
}


//
// Name:        SetMaxKills()
// Class:       HelperNode
//
// Description: Sets the _maxkills variable
//
// Parameters:  int maxkills
//
// Returns:     None
//
void HelperNode::SetMaxKills( int maxkills ) 
{  
	_maxkills = maxkills;
}

//
// Name:        SetWait()
// Class:       HelperNode
//
// Description: Sets the _wait variable
//
// Parameters:  float wait
//
// Returns:     None
//
void HelperNode::SetWait( float wait )
{
	_waittime = wait;
}

//
// Name:        SetWaitRandom()
// Class:       HelperNode
//
// Description: Sets the _wait variable, and sets _waitRandom to true;
//
// Parameters:  float wait
//
// Returns:     None
//
void HelperNode::SetWaitRandom( float wait )
{
	_waittime = wait;
	_waitrandom = true;
}


//
// Name:        SetWaitForAnim()
// Class:       HelperNode
//
// Description: Sets the _waitforanim variable
//
// Parameters:  qboolean wait
//
// Returns:     None
//
void HelperNode::SetWaitForAnim(qboolean wait)
{
	_waitforanim = wait;
}

//
// Name:        SetMinHealth()
// Class:       HelperNode
//
// Description: Sets _minhealth variable
//
// Parameters:  float health - The health to set
//
// Returns:     None
//
void HelperNode::SetMinHealth( float health )
{
	_minhealth = health;
}


//
// Name:        SetMinEnemyRange()
// Class:       HelperNode
//
// Description: Sets _minenemyrange variable
//
// Parameters:  float range - The range to set
//
// Returns:     None
//
void HelperNode::SetMinEnemyRange( float range )
{
	_minenemyrange = range;
}

void HelperNode::SetActivationRange( float range )
{
	_activationRange = range;
}

//
// Name:        SetAnimActive()
// Class:       HelperNode
//
// Description: Sets the _animactive variable
//
// Parameters:  qboolean animactive
//
// Returns:     None
//
void HelperNode::SetAnimActive( qboolean animactive )
{
	_animactive = animactive;
}


//
// Name:        SetCoverActive()
// Class:       HelperNode
//
// Description: Sets the _coveractive variable
//
// Parameters:  qboolean coveractive
//
// Returns:     None
//
void HelperNode::SetCoverActive( qboolean coveractive )
{
	_coveractive = coveractive;
}

//
// Name:        SetCoverType()
// Class:       HelperNode
//
// Description: Sets the _covertype variable
//
// Parameters:  CoverType_t covertype
//
// Returns:     None
//
void HelperNode::SetCoverType( CoverType_t covertype )
{
	_covertype = covertype;
}


//
// Name:        SetCoverDir()
// Class:       HelperNode
//
// Description: Sets the _coverdir variable
//
// Parameters:  CoverDirection_t coverdir
//
// Returns:     None
//
void HelperNode::SetCoverDir( CoverDirection_t coverdir )
{
	_coverdir = coverdir;
}


//
// Name:        SetCriticalChange()
// Class:       HelperNode
//
// Description: Sets the _criticalchange variable
//
// Parameters:  qboolean change
//
// Returns:     None
//
void HelperNode::SetCriticalChange( qboolean change )
{
	_criticalchange = change;
}


//
// Name:        SetDescriptor()
// Class:       HelperNode
//
// Description: Sets the _descriptor variable
//
// Parameters:  NodeDescriptorType_t descriptor
//
// Returns:     None
//
void HelperNode::SetDescriptor( NodeDescriptorType_t descriptor )
{
	_descriptor = descriptor;
}

void HelperNode::SetPriority( float priority )
{
	_priority = priority;
}

//
// Name:        SetFlags()
// Class:       HelperNode
//
// Description: Sets the NodeFlags
//
// Parameters:  unsigned int flags - the integer mask of the flags
//
// Returns:     None
//
void HelperNode::SetFlags( unsigned int flags )
{
	_nodeflags = flags;
}


//======================================
// Helper Node Accessors
//======================================

//
// Name:        GetEntryThread()
// Class:       HelperNode
//
// Description: Returns the _thread variable
//
// Parameters:  None
//
// Returns:     _entryThread
//
const str& HelperNode::GetEntryThread()
{
	return _entryThread;
}


//
// Name:        GetExitThread()
// Class:       HelperNode
//
// Description: Returns the _exitThread
//
// Parameters:  None
//
// Returns:     _exitThread
//
const str& HelperNode::GetExitThread()
{
	return _exitThread;
}

//
// Name:        GetAnim()
// Class:       HelperNode
//
// Description: Returns the _anim variable
//
// Parameters:  None
//
// Returns:     _anim
//
const str& HelperNode::GetAnim()
{
	return _anim;
}


//
// Name:         GetAnimTarget()
// Class:        HelperNode
//
// Description:  Returns _animtarget variable
//
// Parameters:   None
//
// Returns:      _animtarget
//
const str& HelperNode::GetAnimTarget()
{
	return _animtarget;
}


//
// Name:        GetKillTarget()
// Class:       HelperNode
//
// Description: Returns _killtarget variable
//
// Parameters:  None
//
// Returns:     _killtarget
//
const str& HelperNode::GetKillTarget()
{
	return _killtarget;
}


//
// Name:        GetCustomType()
// Class:       HelperNode
//
// Description: Returns _customType variable
//
// Parameters:  None
//
// Returns:     _customType
//
const str& HelperNode::GetCustomType()
{
	return _customType;
}

//
// Name:        GetID()
// Class:       HelperNode
//
// Description: Returns _id variable
//
// Parameters:  None
//
// Returns:     _id
//
int HelperNode::GetID()
{
	return _id;
}

//
// Name:        GetAnimCount()
// Class:       HelperNode
//
// Description: Returns the _animcount variable
//
// Parameters:  None
//
// Returns:     _animcount
//
int HelperNode::GetAnimCount()
{
	return _animcount;
}

//
// Name:        GetMaxKills()
// Class:       HelperNode
//
// Description: Returns the _maxkills variable
//
// Parameters:  None
//
// Returns:     _maxkills
//
int HelperNode::GetMaxKills()
{
	return _maxkills;
}

//
// Name:        GetWaitTime()
// Class:       HelperNode
//
// Description: Returns _waittime
//
// Parameters:  None
//
// Returns:     _waittime
//
float HelperNode::GetWaitTime()
{
	return _waittime;
}


//
// Name:        GetMinHealth()
// Class:       HelperNode
//
// Description: Returns _minhealth variable
//
// Parameters:  None
//
// Returns:     _minhealth
//
float HelperNode::GetMinHealth()
{
	return _minhealth;
}


//
// Name:        GetMinEnemyRange()
// Class:       HelperNode
//
// Description: Returns _minenemyrange;
//
// Parameters:  None
//
// Returns:     _minenemyrange
//
float HelperNode::GetMinEnemyRange()
{
	return _minenemyrange;
}

float HelperNode::GetActivationRange()
{
	return _activationRange;
}

//
// Name:        GetCoverType()
// Class:       HelperNode
//
// Description: Returns _covertype
//
// Parameters:  None
//
// Returns:     _covertype
//
CoverType_t HelperNode::GetCoverType()
{  
	return _covertype;
}


//
// Name:        GetCoverDirection()
// Class:       HelperNode
//
// Description: Returns _coverdir
//
// Parameters:  None
//
// Returns:     _coverdir
//
CoverDirection_t HelperNode::GetCoverDirection()
{
	return _coverdir;
}


//
// Name:        GetDescriptor
// Class:       HelperNode
//
// Description: Returns _descriptor
//
// Parameters:  None
//
// Returns:     NodeDescriptorType_t
//
NodeDescriptorType_t HelperNode::GetDescriptor()
{
	return _descriptor;
}

float HelperNode::GetPriority()
{
	return _priority;
}

//======================================
// Helper Node Queries
//======================================


//
// Name:        isOfType()
// Class:       HelperNode
//
// Description: Checks if the nodeflags match the passed in mask
//
// Parameters:  unsigned int mask
//
// Returns:     true or false;
//
qboolean HelperNode::isOfType(unsigned int mask)
{
	return _nodeflags & ( mask );
}


//
// Name:        isAnimActive()
// Class:       HelperNode
//
// Description: Returns _animactive
//
// Parameters:  None
//
// Returns:     true or false
//
qboolean HelperNode::isAnimActive()
{
	return _animactive;
}


//
// Name:        isCoverActive()
// Class:       HelperNode
//
// Description: Returns _coveractive
//
// Parameters:  None
//
// Returns:     true or false
//
qboolean HelperNode::isCoverActive()
{
	return _coveractive;
}


//
// Name:        isWaitRandom()
// Class:       HelperNode
//
// Description: Returns _waitrandom
//
// Parameters:  None
//
// Returns:     true or false
//
qboolean HelperNode::isWaitRandom()
{
	return _waitrandom;
}


//
// Name:        isWaitForAnim()
// Class:       HelperNode
//
// Description: Returns _waitforanim
//
// Parameters:  None
//
// Returns:     true or false
//
qboolean HelperNode::isWaitForAnim()
{
	return _waitforanim;
}


//
// Name:        isChanged()
// Class:       HelperNode
//
// Description: Returns _criticalchange
//
// Parameters:  None
//
// Returns:     true or false
//
qboolean HelperNode::isChanged()
{
	return _criticalchange;
}

qboolean HelperNode::isReserved()
{
	return _reserved;
}

//======================================
// Helper Node Utility
//======================================


//
// Name:        RunEntryThread()
// Class:       HelperNode
//
// Description: Runs the entry thread
//
// Parameters:  None
//
// Returns:     None
//
void HelperNode::RunEntryThread()
{
	if ( _entryThread.length() <= 0 )
		return;
	
	CThread *thread;
	
	thread = Director.CreateThread( _entryThread );
	
	if ( thread )
		thread->DelayedStart( 0.0f );
}


//
// Name:        RunExitThread()
// Class:       HelperNode
//
// Description: Runs the exit thread
//
// Parameters:  None
//
// Returns:     None
//
void HelperNode::RunExitThread()
{
	if ( _exitThread.length() <= 0 )
		return;
	
	CThread *thread;
	
	thread = Director.CreateThread( _exitThread );
	
	if ( thread )
		thread->DelayedStart( 0.0f );
}


//======================================
// Helper Node Convience Functions
//======================================

//
// Name:        _CoverTypeForString()
// Class:        HelperNode
//
// Description:  Returns the enumerated covertype based on the string
//
// Parameters:   const str &covertype
//
// Returns:      CoverType_t coverType
//
CoverType_t HelperNode::_CoverTypeForString( const str &covertype )
{
	CoverType_t coverType;
	
	for ( int i = 0 ; i < COVER_TYPE_TOTAL_NUMBER ; i++ )
	{
		if ( !Q_stricmp(covertype.c_str() , CoverTypeStrings[i] ) )
		{
			coverType = (CoverType_t)i;
			return coverType;
		}
		
	}
	
	gi.WDPrintf( "Unknown CoverType - %s\n", covertype.c_str() );
	return COVER_TYPE_NONE;
}


//
// Name:        _CoverDirectionForString()
// Class:        HelperNode
//
// Description:  Returns the enumerated coverdir based on the string
//
// Parameters:   const str &coverdir
//
// Returns:      CoverType_t coverDir
//
CoverDirection_t HelperNode::_CoverDirectionForString( const str &coverdir )
{
	CoverDirection_t coverDir;
	
	for ( int i = 0 ; i < COVER_DIRECTION_TOTAL_NUMBER ; i++ )
	{
		if ( !Q_stricmp(coverdir.c_str() , CoverDirStrings[i] ) )
		{
			coverDir = (CoverDirection_t)i;
			return coverDir;
		}
		
	}
	
	gi.WDPrintf( "Unknown CoverDirection - %s\n", coverdir.c_str() );
	return COVER_DIRECTION_NONE;
}


//
// Name:        _CoverDirectionForString()
// Class:        HelperNode
//
// Description:  Returns the enumerated coverdir based on the string
//
// Parameters:   const str &coverdir
//
// Returns:      CoverType_t coverDir
//
NodeDescriptorType_t HelperNode::_DescriptorForString( const str &descriptor )
{
	NodeDescriptorType_t descriptorType;
	
	for ( int i = 0 ; i < DESCRIPTOR_TOTAL_NUMBER ; i++ )
	{
		if ( !Q_stricmp(descriptor.c_str() , DescriptorTypeStrings[i] ) )
		{
			descriptorType = (NodeDescriptorType_t)i;
			return descriptorType;
		}
		
	}
	
	gi.WDPrintf( "Unknown Descriptor - %s\n", descriptor.c_str() );
	return DESCRIPTOR_NONE;
}

//
// Name:        FindClosestHelperNode()
// Class:       None
//
// Description: Return nearest valid helpernode based on the criteria passed in
//
// Parameters:  Actor &self -- Actor looking for the node
//              int mask    -- Mask of the node
//
// Returns:     HelperNode* bestNode
//
HelperNode* HelperNode::FindClosestHelperNode( Actor &self , int mask , float maxDist , float minDistanceFromPlayer, bool unreserveCurrentNode )
{
	HelperNode      *node;
	HelperNode      *bestNode;      
	float            pathLen;
	float            bestDist;
	float            activationRange;
	FindMovementPath find;
	Path            *path;
	int              nodeID;
	
	
	// Set up our pathing heuristics
	find.heuristic.self = &self;
	find.heuristic.setSize( self.size );
	find.heuristic.entnum = self.entnum;
	
	// Initialize our nodes and distances
	bestNode   = NULL;
	bestDist   = 999999999.8f;
	pathLen    = bestDist - 1.0f;
	
	//We are going to loop through all the helper nodes in the level, find ones matching
	//our mask, and compare path lengths to each of them.  We can't do a plain ol'line of
	//sight check, because, it seems valid to me that the nearest helpernode might not be
	//visible.  We also can't just compare raw proximity, because we could have a node that
	//is technically "closer" but behind a wall, forcing us to path a very long way to get
	//to it.
	
	//Note this is a potientially VERY expensive function, and we need to keep an eye on it
	//to make sure its not called a lot, and keep in mind that it may be a candidate for
	//extensive optmization   
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		nodeID = node->GetID();
		
		if ( nodeID != -1 && nodeID != self.currentHelperNode.nodeID )
			continue;
		
		if ( node && node->isOfType(mask))
		{
			activationRange = node->GetActivationRange();
			
			
			if ( activationRange > 0 )
			{
				if ( !self.WithinDistance( node->origin, activationRange ) )
					continue;
			}
			else
			{
				if ( !self.WithinDistance( node->origin, maxDist ) )
					continue;
			}
			
			if ( node == self.ignoreHelperNode.node )
				continue;
			
			if ( node->isReserved() )
				continue;
			
			Player* player;
			player = GetPlayer(0);
			if ( !player ) return false;

			if ( player->WithinDistance( node->origin , minDistanceFromPlayer ) )
				continue;

			path = find.FindPath( self.origin, node->origin );
			
			if ( path )
				pathLen = path->Length();            
			
			delete path;
			path = NULL;
			
			if ( pathLen < bestDist )
            {  
				bestDist = pathLen;
				bestNode = node;
            }
			
		}
		
		
	}
	
    if ( self.currentHelperNode.node && unreserveCurrentNode )
		self.currentHelperNode.node->UnreserveNode();
	
	self.currentHelperNode.node = bestNode;
	self.currentHelperNode.mask = mask;
	
	if ( bestNode )
		bestNode->ReserveNode();
	
	return bestNode;
}


//
// Name:        FindClosestHelperNode()
// Class:       None
//
// Description: Return nearest valid helpernode based on the criteria passed in
//
// Parameters:  Actor &self -- Actor looking for the node
//              int mask    -- Mask of the node
//
// Returns:     HelperNode* bestNode
//
HelperNode* HelperNode::FindClosestHelperNode( Actor &self , int mask , NodeDescriptorType_t descriptor, float maxDist)
{
	HelperNode      *node;
	HelperNode      *bestNode;      
	float            pathLen;
	float            bestDist;
	float            activationRange;
	FindMovementPath find;
	Path            *path;
	int              nodeID;
	
	
	// Set up our pathing heuristics
	find.heuristic.self = &self;
	find.heuristic.setSize( self.size );
	find.heuristic.entnum = self.entnum;
	
	// Initialize our nodes and distances
	bestNode   = NULL;
	bestDist   = 999999999.8f;
	pathLen    = bestDist - 1.0f;
	
	//We are going to loop through all the helper nodes in the level, find ones matching
	//our mask, and compare path lengths to each of them.  We can't do a plain ol'line of
	//sight check, because, it seems valid to me that the nearest helpernode might not be
	//visible.  We also can't just compare raw proximity, because we could have a node that
	//is technically "closer" but behind a wall, forcing us to path a very long way to get
	//to it.
	
	//Note this is a potientially VERY expensive function, and we need to keep an eye on it
	//to make sure its not called a lot, and keep in mind that it may be a candidate for
	//extensive optmization   
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		nodeID = node->GetID();
		
		if ( nodeID != -1 && nodeID != self.currentHelperNode.nodeID )
			continue;
		
		if ( node && node->isOfType(mask) && node->GetDescriptor() == descriptor )
		{
			activationRange = node->GetActivationRange();
			
			if ( node->isReserved() )
				continue;
			
			if ( node == self.ignoreHelperNode.node )
				continue;
			
			
			if ( activationRange > 0 )
			{
				if ( !self.WithinDistance( node->origin, activationRange ) )
					continue;
			}
			else
			{
				if ( !self.WithinDistance( node->origin, maxDist ) )
					continue;
			}
			
			path = find.FindPath( self.origin, node->origin );
			
			if ( path )
				pathLen = path->Length();            
			
			delete path;
			path = NULL;
			
			if ( pathLen < bestDist )
            {  
				bestDist = pathLen;
				bestNode = node;
            }
			
		}
		
		
	}
	
	if ( self.currentHelperNode.node )
		self.currentHelperNode.node->UnreserveNode();
	
	self.currentHelperNode.node = bestNode;
	self.currentHelperNode.mask = mask;
	
	if ( bestNode )
		bestNode->ReserveNode();
	
	return bestNode;
}

HelperNode* HelperNode::FindClosestHelperNodeAtDistanceFrom( Actor &self , Entity *ent , int mask , NodeDescriptorType_t descriptor , float maxDistFromSelf , float minDistFromEnt )
{
	HelperNode      *node;
	HelperNode      *bestNode;      
	float            pathLen;
	float            bestDist;
	float            activationRange;
	FindMovementPath find;
	Path            *path;
	int              nodeID;
	
	
	// Set up our pathing heuristics
	find.heuristic.self = &self;
	find.heuristic.setSize( self.size );
	find.heuristic.entnum = self.entnum;
	
	// Initialize our nodes and distances
	bestNode   = NULL;
	bestDist   = 999999999.8f;
	pathLen    = bestDist - 1.0f;
	
	//We are going to loop through all the helper nodes in the level, find ones matching
	//our mask, and compare path lengths to each of them.  We can't do a plain ol'line of
	//sight check, because, it seems valid to me that the nearest helpernode might not be
	//visible.  We also can't just compare raw proximity, because we could have a node that
	//is technically "closer" but behind a wall, forcing us to path a very long way to get
	//to it.
	
	//Note this is a potientially VERY expensive function, and we need to keep an eye on it
	//to make sure its not called a lot, and keep in mind that it may be a candidate for
	//extensive optmization   
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		nodeID = node->GetID();
		
		if ( nodeID != -1 && nodeID != self.currentHelperNode.nodeID )
			continue;
		
		if ( node && node->isOfType(mask) && node->GetDescriptor() == descriptor )
		{
			activationRange = node->GetActivationRange();
			
			if ( node->isReserved() )
				continue;
			
			if ( activationRange > 0 )
			{
				if ( !self.WithinDistance( node->origin, activationRange ) )
					continue;
			}
			else
			{
				if ( !self.WithinDistance( node->origin, maxDistFromSelf ) )
					continue;
			}
			
			if ( ent->WithinDistance( node->origin , minDistFromEnt ) )
				continue;
			
			path = find.FindPath( self.origin, node->origin );
			
			if ( path )
				pathLen = path->Length();            
			
			delete path;
			path = NULL;
			
			if ( pathLen < bestDist )
			{  
				bestDist = pathLen;
				bestNode = node;
			}
		}
	}
	
	self.currentHelperNode.node = bestNode;
	self.currentHelperNode.mask = mask;
	return bestNode;
}

HelperNode* HelperNode::FindHelperNodeClosestToWithoutPathing( Actor &self , Entity *ent , int mask , float maxDist )
{
	HelperNode      *node;
	HelperNode      *bestNode;      
	float            pathLen;
	float            bestDist;
	float            activationRange;
	int              nodeID;
	
	
	if ( !ent )
		return NULL;
	
	// Initialize our nodes and distances
	bestNode   = NULL;
	bestDist   = 999999999.8f;
	pathLen    = bestDist - 1.0f;
	
	//We are going to loop through all the helper nodes in the level, find ones matching
	//our mask, and compare path lengths to each of them.  We can't do a plain ol'line of
	//sight check, because, it seems valid to me that the nearest helpernode might not be
	//visible.  We also can't just compare raw proximity, because we could have a node that
	//is technically "closer" but behind a wall, forcing us to path a very long way to get
	//to it.
	
	//Note this is a potientially VERY expensive function, and we need to keep an eye on it
	//to make sure its not called a lot, and keep in mind that it may be a candidate for
	//extensive optmization   
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		nodeID = node->GetID();
		
		if ( nodeID != -1 && nodeID != self.currentHelperNode.nodeID )
			continue;
		
		if ( node && node->isOfType(mask))
		{
			activationRange = node->GetActivationRange();
			
			if ( activationRange > 0 )
			{
				if ( !ent->WithinDistance( node->origin, activationRange ) )
					continue;
			}
			else
			{
				if ( !ent->WithinDistance( node->origin, maxDist ) )
					continue;
			}
			
			if ( node == self.ignoreHelperNode.node )
				continue;
			
			if ( node->isReserved() )
				continue;
			
			Vector pathToNode = ent->origin - node->origin;
			pathLen = pathToNode.length();
			
			if ( pathLen < bestDist )
            {  
				bestDist = pathLen;
				bestNode = node;
            }
			
		}
		
		
	}
	
	if ( self.currentHelperNode.node )
		self.currentHelperNode.node->UnreserveNode();
	
	self.currentHelperNode.node = bestNode;
	self.currentHelperNode.mask = mask;
	
	if ( bestNode )
		bestNode->ReserveNode();
	
	return bestNode;
}



HelperNode* HelperNode::FindHelperNodeClosestTo( Actor &self, Entity *ent , int mask , float maxDist )
{
	HelperNode      *node;
	HelperNode      *bestNode;      
	float            pathLen;
	float            bestDist;
	float            activationRange;
	FindMovementPath find;
	Path            *path;
	int              nodeID;
	
	
	if ( !ent )
		return NULL;
	
	// Set up our pathing heuristics
	find.heuristic.self = &self;
	find.heuristic.setSize( self.size );
	find.heuristic.entnum = self.entnum;
	
	// Initialize our nodes and distances
	bestNode   = NULL;
	bestDist   = 999999999.8f;
	pathLen    = bestDist - 1.0f;
	
	//We are going to loop through all the helper nodes in the level, find ones matching
	//our mask, and compare path lengths to each of them.  We can't do a plain ol'line of
	//sight check, because, it seems valid to me that the nearest helpernode might not be
	//visible.  We also can't just compare raw proximity, because we could have a node that
	//is technically "closer" but behind a wall, forcing us to path a very long way to get
	//to it.
	
	//Note this is a potientially VERY expensive function, and we need to keep an eye on it
	//to make sure its not called a lot, and keep in mind that it may be a candidate for
	//extensive optmization   
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		nodeID = node->GetID();
		
		if ( nodeID != -1 && nodeID != self.currentHelperNode.nodeID )
			continue;
		
		if ( node && node->isOfType(mask))
		{
			activationRange = node->GetActivationRange();
			
			if ( activationRange > 0 )
			{
				if ( !ent->WithinDistance( node->origin, activationRange ) )
					continue;
			}
			else
			{
				if ( !ent->WithinDistance( node->origin, maxDist ) )
					continue;
			}
			
			if ( node == self.ignoreHelperNode.node )
				continue;
			
			if ( node->isReserved() )
				continue;
			
			path = find.FindPath( ent->origin, node->origin );
			
			if ( path )
				pathLen = path->Length();            
			
			delete path;
			path = NULL;
			
			if ( pathLen < bestDist )
            {  
				bestDist = pathLen;
				bestNode = node;
            }
			
		}
		
		
	}
	
	if ( self.currentHelperNode.node )
		self.currentHelperNode.node->UnreserveNode();
	
	self.currentHelperNode.node = bestNode;
	self.currentHelperNode.mask = mask;
	
	if ( bestNode )
		bestNode->ReserveNode();
	
	return bestNode;
}

//
// Name:        FindClosestHelperNodeWithoutPathing()
// Class:       None
//
// Description: Return nearest valid helpernode based on the criteria passed in
//
// Parameters:  Actor &self -- Actor looking for the node
//              int mask    -- Mask of the node
//
// Returns:     HelperNode* bestNode
//
HelperNode* HelperNode::FindClosestHelperNodeWithoutPathing( Actor &self, float maxDist)
{
	// Set up our pathing heuristics
	FindMovementPath find;
	find.heuristic.self = &self;
	find.heuristic.setSize( self.size );
	find.heuristic.entnum = self.entnum;
	
	// Initialize our nodes and distances
	HelperNode *bestNode = NULL;
	float bestDistance = 999999999.8f;
	
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		HelperNode *node = NULL;
		node = HelperNodes.ObjectAt( i );
		int nodeID = node->GetID();
		
		if ( ( nodeID != -1 ) && ( nodeID != self.currentHelperNode.nodeID) )
			continue;
		
		float activationRange = node->GetActivationRange();
		
		if ( activationRange > 0 )
		{
			if ( !self.WithinDistance( node->origin, activationRange ) )
				continue;
		}
		else
		{
			if ( !self.WithinDistance( node->origin, maxDist ) )
				continue;
		}
		
		float currentDistance = Vector::Distance( self.origin, node->origin );
		
		if ( currentDistance < bestDistance )
        {  
			bestDistance = currentDistance;
			bestNode = node;
		}
	}
	
	self.currentHelperNode.node = bestNode;
	return bestNode;
}

HelperNode* HelperNode::FindClosestHelperNodeThatCannotSeeEntity( Actor &self , int mask , unsigned int clipMask , float maxDist , float minDist , Entity *ent , float minDistFromPlayer )
{
	HelperNode			*node;
	HelperNode			*bestNode;      
	float				pathLen;
	float				bestDist;
	float				activationRange;
	FindMovementPath	find;
	Path				*path;
	int					nodeID;
	trace_t				trace;
	
	Q_UNUSED(clipMask);
	
	// Set up our pathing heuristics
	find.heuristic.self = &self;
	find.heuristic.setSize( self.size );
	find.heuristic.entnum = self.entnum;
	
	// Initialize our nodes and distances
	bestNode	= NULL;
	bestDist	= 999999999.8f;
	pathLen	= bestDist - 1.0f;
	
	//We are going to loop through all the helper nodes in the level, find ones matching
	//our mask, and compare path lengths to each of them.  We can't do a plain ol'line of
	//sight check, because, it seems valid to me that the nearest helpernode might not be
	//visible.  We also can't just compare raw proximity, because we could have a node that
	//is technically "closer" but behind a wall, forcing us to path a very long way to get
	//to it.
	
	//Note this is a potientially VERY expensive function, and we need to keep an eye on it
	//to make sure its not called a lot, and keep in mind that it may be a candidate for
	//extensive optmization   
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		nodeID = node->GetID();
		
		if ( nodeID != -1 && nodeID != self.currentHelperNode.nodeID )
			continue;

		
		if ( node && node->isOfType(mask))
		{
			
			activationRange = node->GetActivationRange();
			
			if ( activationRange > 0 )
			{
				if ( !self.WithinDistance( node->origin, activationRange ) )
					continue;
			}
			else
			{
				if ( !self.WithinDistance( node->origin, maxDist ) )
					continue;
			}
			
			if ( node->isReserved() )
				continue;

			//Check if node is within the minimum distance of the player
			Player* player;
			player = GetPlayer(0);
			if ( !player ) return false;

			if ( player->WithinDistance( node->origin , minDistFromPlayer ) )
				continue;

			
			path = find.FindPath( self.origin, node->origin );
			
			if ( !path )
				continue;
			
			pathLen = path->Length();            
			
			delete path;
			path = NULL;
			
			if ( pathLen < bestDist && !ent->WithinDistance( node->origin , minDist ) )
			{  
				Vector endPos;
				endPos = ent->centroid;

				if ( ent->isSubclassOf(Actor) )
					{
					Actor *theActor;
					theActor = (Actor*)ent;
					endPos = theActor->EyePosition();
					}

				trace = G_Trace( node->origin , vec_zero , vec_zero , endPos , NULL , MASK_OPAQUE, false, "FindClosestHelperNode");
				if ( trace.fraction > .9 )
					continue;
				
				bestDist = pathLen;
				bestNode = node;			
			}  
		}
	}
	
	
	if ( bestNode && self.currentHelperNode.node )
		self.currentHelperNode.node->UnreserveNode();
	
	if ( bestNode )
	{
		self.currentHelperNode.node = bestNode;
		self.currentHelperNode.mask = mask;
	}
	
	if ( bestNode )
		bestNode->ReserveNode();
	
	return bestNode;
}


HelperNode* HelperNode::FindClosestHelperNode( Actor &self , const str& customType , float maxDist)
{
	HelperNode      *node;
	HelperNode      *bestNode;      
	float            pathLen;
	float            bestDist;
	FindMovementPath find;
	Path            *path;
	int              nodeID;
	
	
	// Set up our pathing heuristics
	find.heuristic.self = &self;
	find.heuristic.setSize( self.size );
	find.heuristic.entnum = self.entnum;
	
	// Initialize our nodes and distances
	bestNode   = NULL;
	bestDist   = 999999999.8f;
	pathLen    = bestDist - 1.0f;
	
	//We are going to loop through all the helper nodes in the level, find ones matching
	//our mask, and compare path lengths to each of them.  We can't do a plain ol'line of
	//sight check, because, it seems valid to me that the nearest helpernode might not be
	//visible.  We also can't just compare raw proximity, because we could have a node that
	//is technically "closer" but behind a wall, forcing us to path a very long way to get
	//to it.
	
	//Note this is a potientially VERY expensive function, and we need to keep an eye on it
	//to make sure its not called a lot, and keep in mind that it may be a candidate for
	//extensive optmization
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		nodeID = node->GetID();
		
		if ( nodeID != -1 && nodeID != self.currentHelperNode.nodeID )
			continue;
		
		if ( node == self.ignoreHelperNode.node )
			continue;
		
		if ( node->isReserved() )
			continue;
		
		if ( node && node->isOfType(NODETYPE_CUSTOM))
		{
			str type;
			type = node->GetCustomType();
			
			if ( !Q_stricmp(type.c_str() , customType.c_str() ) )
            {
				if ( !self.WithinDistance( node->origin , maxDist ) )
					continue;
				
				path = find.FindPath( self.origin, node->origin );
				
				if ( path )
					pathLen = path->Length();            
				
				delete path;
				path = NULL;
				
				if ( pathLen < bestDist )
				{  
					bestDist = pathLen;
					bestNode = node;
				}
            }
			
		}
		
		
	}
	
	
	if ( self.currentHelperNode.node )
		self.currentHelperNode.node->UnreserveNode();
	
	self.currentHelperNode.node = bestNode;
	self.currentHelperNode.mask = NODETYPE_CUSTOM;
	
	if ( bestNode )
		bestNode->ReserveNode();
	
	
	return bestNode;
}






HelperNode* HelperNode::FindHighestPriorityNode(  Actor& self , const str& customType  )
{
	HelperNode      *node;
	HelperNode      *bestNode;      
	float            priority;
	float            bestPriority;
	int              nodeID;
	
	
	// Initialize our nodes and priorities
	bestNode     = NULL;
	priority     = 0.0f;
	bestPriority = 0.0f;
	
	
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		nodeID = node->GetID();
		
		if ( nodeID != -1 && nodeID != self.currentHelperNode.nodeID )
			continue;
		
		if ( node->isReserved() )
			continue;
		
		if ( node->isOfType(NODETYPE_CUSTOM))
		{
			str type;
			type = node->GetCustomType();
			
			if ( !stricmp(type.c_str() , customType.c_str() ) )
			{
				priority = node->GetPriority();
				if ( priority > bestPriority )
				{
					bestPriority = priority;
					bestNode = node;
				}
			}
			
		}
		
		
	}
	
	self.currentHelperNode.node = bestNode;
	self.currentHelperNode.mask = NODETYPE_CUSTOM;
	return bestNode;
}

HelperNode* HelperNode::FindHighestPriorityNode(  Actor& self , const str& customType , const str &targetedTo  )
{
	HelperNode      *node;
	HelperNode      *bestNode;      
	float            priority;
	float            bestPriority;
	int              nodeID;
	
	
	// Initialize our nodes and priorities
	bestNode     = NULL;
	priority     = 0.0f;
	bestPriority = 999999.0f;
	
	
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		nodeID = node->GetID();
		
		if ( nodeID != -1 && nodeID != self.currentHelperNode.nodeID )
			continue;
		
		if ( node->isReserved() )
			continue;
		
		if ( node->isOfType(NODETYPE_CUSTOM))
		{
			str type;
			type = node->GetCustomType();
			
			if ( !stricmp(type.c_str() , customType.c_str() ) && !stricmp(node->target, targetedTo.c_str() ) )
			{
				priority = node->GetPriority();
				if ( priority < bestPriority )
				{
					bestPriority = priority;
					bestNode = node;
				}
			}
			
		}
		
		
	}
	
	self.currentHelperNode.node = bestNode;
	self.currentHelperNode.mask = NODETYPE_CUSTOM;
	return bestNode;
}

//
// Name:        FindClosestHelperNode()
// Class:       None
//
// Description: Return nearest valid helpernode based on the criteria passed in
//
// Parameters:  Actor &self -- Actor looking for the node
//              int mask    -- Mask of the node
//
// Returns:     HelperNode* bestNode
//
HelperNode* HelperNode::FindClosestHelperNode( Actor &self , const str& targetName )
{
	HelperNode      *node;
	HelperNode      *bestNode;      
	float            pathLen;
	float            bestDist;   
	FindMovementPath find;
	Path            *path;   
	
	// Set up our pathing heuristics
	find.heuristic.self = &self;
	find.heuristic.setSize( self.size );
	find.heuristic.entnum = self.entnum;
	
	// Initialize our nodes and distances
	bestNode   = NULL;
	bestDist   = 999999999.8f;
	pathLen    = bestDist - 1.0f;
	
	//We are going to loop through all the helper nodes in the level, find ones matching
	//our mask, and compare path lengths to each of them.  We can't do a plain ol'line of
	//sight check, because, it seems valid to me that the nearest helpernode might not be
	//visible.  We also can't just compare raw proximity, because we could have a node that
	//is technically "closer" but behind a wall, forcing us to path a very long way to get
	//to it.
	
	//Note this is a potientially VERY expensive function, and we need to keep an eye on it
	//to make sure its not called a lot, and keep in mind that it may be a candidate for
	//extensive optmization   
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{   
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		
		
		
		if ( node && node->targetname == targetName )
		{
			path = find.FindPath( self.origin, node->origin );
			
			if ( path )
				pathLen = path->Length();            
			
			delete path;
			path = NULL;
			
			if ( pathLen < bestDist )
            {  
				bestDist = pathLen;
				bestNode = node;
            }
			
		}
		
		
	}
	
	self.currentHelperNode.node = bestNode;
	return bestNode;
}

//--------------------------------------------------------------
// Name:		isHelperNodeInRange()
// Class:		HelperNode
//
// Description:	Checks if the helper node is within range
//
// Parameters:	Actor &self,
//				int mask,
//				float range,
//
// Returns:		true or false;
//--------------------------------------------------------------
bool HelperNode::isHelperNodeInRange( Actor &self , int mask , float range )
{
	HelperNode      *node;
	
	node = FindHelperNodeClosestTo( self, &self, mask , range );
	if ( node )
		return true;
	
	return false;
	
}

//
// Name:        GetTargetdHelperNode()
// Class:       None
//
// Description: Returns the helpernode with the passed in targetName
//
// Parameters: const str &targetName -- the targetName of the node to find
//
// Returns:    HelperNode* node
//
HelperNode* HelperNode::GetTargetedHelperNode( const str& targetName )
{
	HelperNode      *node;   
	
	for ( int i = 1 ; i <= HelperNodes.NumObjects() ; i++ )
	{
		node = NULL;
		node = HelperNodes.ObjectAt( i );
		
		if ( node && (!Q_stricmp( node->targetname , targetName.c_str() ) ) )
		{
			return node;
		}
	}
	
	return NULL;
}

//
// Name:        GetHelperNodeMask()
// Class:       None
//
// Description: Returns an integer bit mask based on the string passed in
//
// Parameters:  const str& type -- string of the node type
//
// Returns:     int mask
//
int HelperNode::GetHelperNodeMask( const str& type )
{
	int mask;
	
	mask = 0;
	// Create our mask
	if ( !Q_stricmp( type.c_str() , "flee" ) )
		mask |= NODETYPE_FLEE;
	else if ( !Q_stricmp( type.c_str() , "work" ) )
		mask |= NODETYPE_WORK;
	else if ( !Q_stricmp( type.c_str() , "anim" ) )
		mask |= NODETYPE_ANIM;
	else if ( !Q_stricmp( type.c_str() , "cover" ) )
		mask |= NODETYPE_COVER;
	else if ( !Q_stricmp( type.c_str() , "patrol" ) )
		mask |= NODETYPE_PATROL;
	else if ( !Q_stricmp( type.c_str() , "sniper" ) )
		mask |= NODETYPE_SNIPER;
	else if ( !Q_stricmp( type.c_str() , "custom" ) )
		mask |= NODETYPE_CUSTOM;
	else if ( !Q_stricmp( type.c_str() , "combat" ) )
		mask |= NODETYPE_COMBAT;
	
	return mask;
}


//
// Editor Setup
//

/*****************************************************************************/
/*QUAKED info_helpernode (1 0 1) (-16 -16 0) (16 16 16) FLEE WORK ANIM COVER PATROL SNIPER CUSTOM COMBAT

NodeType FLEE
Purpose:  Marks this spot as a safe destination for the AI when they flee

Additional Commands for FLEE type Nodes
None at this time


NodeType WORK
Purpose:  Marks this spot as a good place for the AI to work at. 
          The AI will turn to match the angles of helper node, so make sure to angle it appropriately.
          The AI will also run the animation you specify with the "setnodeanim" event and execute the
          thread you specify with the "setnodethread" event

Additional Commands for WORK type Nodes
setnodeanim        -- Sets the animation that the AI will run while on this node
setnodeentrythread -- Sets the thread that the AI will call when they arrive at this node
setnodeexitthread  -- Sets the thread that the AI will call when they leave the node
wait               -- Sets the time at this work node
waitrandom         -- Sets a random waittime from 1 to the time specified
waitforanim        -- Sets the actor to work until his animation is completed


NodeType ANIM
Purpose:  Allows you to specify a "special" animation to be played at this location.  For example, if
          you have a "dive through window" animation that you would like played as the AI moves through
          the node, you can specify it here

Additional Commands for ANIM type Nodes
setnodeanim   -- Sets the animation that the AI will run while on this node
animcount     -- Number of times the anim can be played
animtarget    -- TargetName for a specific entity you wish to be animated
animactive    -- Sets if the animation component is active
                 1 is true ( default )
                 0 is false
                 if animactive is 0, then actors will not change animations at the node

NodeType COVER
Purpose:  Marks this spot as a good place for the AI to use as cover.  The AI will turn to match the angles 
          of the helper node, so make sure to angle it appropriately.  The AI will also use the cover 
          according to the information you specify in the setcovertype and setcoverdir events.

Additional Commands for COVER type Nodes
setcovertype  -- Sets the type of cover at this spot. Valid values are ( crate , wall )                 
setcoverdir   -- Sets the direction that the AI can fire from ( Based Relative to the direction the node is facing )
                 valid values are ( left , right, all )
coveractive   -- Sets if the cover component is active
                 1 is true ( default )
                 0 is false
                 if coveractive is 0, then actors will not use it for cover.


NodeType PATROL
Purpose:  Marks a position in a patrol path.  To be of any value, though, patrol helper nodes must be chained together.
          If you want a full circuit, then you must connect all nodes together.  Any nodes CANNOT be cross linked together.
          I know, I know, it sucks, but unfortunately, due to limits with targetname, it is the way it must be.
          If you do not make a circuit, the AI will stand idle at the last node until it decides to do something else.

Additional Commands for PATROL type Nodes
wait         -- Makes the AI pause at this node for the specifed amount of time
waitrandom   -- Makes the AI pause for a random amount of time (up to a max of the amount specified )
waitforanim  -- Makes the AI pause long enough to complete his animation


NodeType SNIPER
Purpose:  Marks a position as a good sniping position.

Additional Commands for SNIPER type Nodes
wait         -- Specifies a maximum time the AI will sit at this Node
attacktarget -- Specifies a target to kill -- If not specified, will try and snipe any enemies in view
maxkills     -- Specifies the maximum number of kills at this Node


NodeType CUSTOM
Purpose: Marks the node as a custom helper node.  Must be used in conjunction with the customtype command

NodeType COMBAT
Purpose: Marks the node as a general place to do combat.




NodeType 
          
******************************************************************************/







//=============================================================================
//                           HelperNodeController
//=============================================================================
void HelperNodeController::Archive( Archiver &arc )
{
	HelperNode* checkNode;
	
	int num , i;
	
	if ( arc.Saving() )
	{
		num = HelperNodes.NumObjects();
		
		arc.ArchiveInteger( &num );
		
		for ( i = 1 ; i <= num ; i++ )
		{
			checkNode = HelperNodes.ObjectAt( i );
			arc.ArchiveObject( checkNode );
		}
	}
	else
	{
		arc.ArchiveInteger( &num );
		HelperNodes.ClearObjectList();
		HelperNodes.Resize( num );
		
		for ( i = 1 ; i<= num ; i++ )
		{
			checkNode = new HelperNode;	
			arc.ArchiveObject( checkNode );
			HelperNodes.AddObject( checkNode );
		}
		
	}
	
}

//--------------------------------------------------------------
// Name:		CleanupHelperNodeList()
// Class:		HelperNode
//
// Description:	Iterates through the helper node list and cleans
//				it out
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
void HelperNode::CleanupHelperNodeList()
{
	HelperNode* checkNode;
	
	for ( int i = HelperNodes.NumObjects() ; i > 0 ; i-- )
	{
		checkNode = HelperNodes.ObjectAt( i );
		if ( checkNode )
		{
			delete checkNode;
			checkNode = NULL;
			HelperNodes.RemoveObjectAt( i );
		}
	}
	
}


waitType_t HelperNode::GetWaitType( const str &waitType )
{
	if ( waitType == "anim" )
		return WAITTYPE_ANIM;
	else if ( waitType == "event" )
		return WAITTYPE_EVENT;
	else if ( waitType == "time" )
		return WAITTYPE_TIME;
	
	assert( "Unknown waitType" );
	return WAITTYPE_ERROR;
}


void HelperNode::SendCommand( Event *ev )
{
	Event *commandEvent;
	int numArgs;
	
	//First make sure we have a user to send 
	//the command event to
	if ( !_user )
		return;
	
	commandEvent = new Event ( EV_HelperNodeCommand );
	
	numArgs = ev->NumArgs();
	for ( int i = 1 ; i <= numArgs ; i++ )
	{
		commandEvent->AddToken( ev->GetToken( i ) );
	}
	
	_user->ProcessEvent( commandEvent );
}


customAnimListEntry_t* HelperNode::GetNextAnimEntryFromList()
{
	int animCount = _customAnimList.NumObjects();
	int index = _customAnimListIndex;
	
	if ( index < animCount  )
		index++;
	
	return _customAnimList.ObjectAt( index );
}

customAnimListEntry_t* HelperNode::GetCurrentAnimEntryFromList()
{
	int index = _customAnimListIndex;
	return _customAnimList.ObjectAt( index );
}

void HelperNode::AddAnimation( Event *ev )
{
	float time;
	if ( ev->NumArgs() > 2 )
		time = ev->GetFloat( 3 );
	else
		time = 0.0f;
	
	AddAnimation(ev->GetString( 1 ) , GetWaitType( ev->GetString( 2 ) ) , time );
}

void HelperNode::AddAnimation( const str &anim , waitType_t waitType , float time )
{
	customAnimListEntry_t* checkEntry;
	
	checkEntry = new customAnimListEntry_t;
	checkEntry->anim = anim;
	checkEntry->waitType = waitType;
	checkEntry->time = time;
	_customAnimList.AddObject( checkEntry );
	
	_usingCustomAnimList = true;
}

bool HelperNode::isAnimListFinished()
{
	return  _customAnimListIndex > _customAnimList.NumObjects();		
}

void HelperNode::NextAnim()
{
	_customAnimListIndex++;
}

//
// Global Functions
//
void AddHelperNodeToList( HelperNode* node )
{
	if ( node )
		HelperNodes.AddUniqueObject( node );
}

void RemoveHelperNodeFromList( HelperNode *node )
{
	HelperNode* checkNode;
	
	for ( int i = HelperNodes.NumObjects() ; i > 0 ; i-- )
	{
		checkNode = HelperNodes.ObjectAt( i );
		if ( checkNode == node )
		{
			delete checkNode;
			checkNode = NULL;
			HelperNodes.RemoveObjectAt( i );
			return;
		}
	}
}

