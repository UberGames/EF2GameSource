//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/viewthing.cpp                            $
// $Revision:: 46                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Actor code for the Viewthing.
//

#include "_pch_cpp.h"
#include "entity.h"
#include "viewthing.h"
#include "player.h"
#include "q_shared.h"
#include <qcommon/cm_public.h>

// These command vars are used with the viewset and viewlist commands
cvar_t         *g_vt_droptoggle;      // treated as a boolean.  If true, we drop newly spawned things
cvar_t         *g_vt_setFilename ;    // contains the filename of the current .lst file (if any)
cvar_t         *g_vt_modelIndex ;     // Index specifying which model in the .lst file we are viewing
cvar_t         *g_vt_numModels ;      // Number of total models in the .lst file
cvar_t         *g_vt_height ;         // Z Height off 0 that we position spawned things
cvar_t         *g_vt_scale ;          // Scale of spawned things
cvar_t         *g_vt_yaw ;            // Current yaw value for spawned things
cvar_t         *g_vt_roll ;           // Current roll value for spawned things
cvar_t         *g_vt_pitch ;          // Current pitch value for spawned things
cvar_t         *g_vt_xtrans ;         // Current x translation for current viewthing
cvar_t         *g_vt_ytrans ;         // Current y translation for current viewthing
cvar_t         *g_vt_ztrans ;         // Current z translation for current viewthing
cvar_t         *g_vt_makestatic ;     // Toggle for controlling if spawned objects are "static" (static lighting) or not.
cvar_t         *g_vt_usebox ;         // When shooting objects, objects pull away from wall by their bounding box.  Defaults true.
cvar_t         *g_vt_viewcount ;      // Holds number of spawned viewthings
cvar_t         *g_vt_viewnumber ;     // Holds position of current_viewthing amongst all spawned ones
cvar_t         *g_vt_viewrandomize ;  // If true (default is false) spawned things are affected by randomization settings (below)
cvar_t         *g_vt_viewsrandomize;  // If true randomize scale (default true)
cvar_t         *g_vt_viewzrandomize;  // If true randomize rotations about z axis (yaw) (default false)
cvar_t         *g_vt_viewyrandomize;  // If true randomize rotations about y axis (pitch) (default false)
cvar_t         *g_vt_viewxrandomize;  // If true randomize rotations about x axis (roll) (default false)
cvar_t         *g_vt_viewminrandomize;// Min value for randomizing scale (default is 50%)
cvar_t         *g_vt_viewmaxrandomize;// Max value for randomizing scale (default is 150%)
cvar_t         *g_vt_viewbellrandomize;// Set to true if you want scale randomizatins to be on a bell curve (default true)


Event EV_ViewThing_Think
(
	"viewthing_think",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called every frame to process the view thing."
);
Event EV_ViewThing_ToggleAnimate
(
	"viewanimate",
	EV_CHEAT,
	NULL,
	NULL,
	"Cycle through the animations modes of the current viewthing\n"
	"No Animation\n"
	"Animation with no motion\n"
	"Animation with looping motion\n"
	"Animation with motion\n"
);
Event EV_ViewThing_SetModel
(
	"viewmodel",
	EV_CHEAT,
	"s",
	"viewthingModel",
	"Set the model of the current viewthing"
);
Event EV_ViewThing_NextFrame
(
	"viewnext",
	EV_CHEAT,
	NULL,
	NULL,
	"Advance to the next frame of animation of the current viewthing"
);
Event EV_ViewThing_PrevFrame
(
	"viewprev",
	EV_CHEAT,
	NULL,
	NULL,
	"Advance to the previous frame of animation of the current viewthing"
);
Event EV_ViewThing_NextAnim
(
	"viewnextanim",
	EV_CHEAT,
	NULL,
	NULL,
	"Advance to the next animation of the current viewthing"
);
Event EV_ViewThing_PrevAnim
(
	"viewprevanim",
	EV_CHEAT,
	NULL,
	NULL,
	"Advance to the previous animation of the current viewthing"
);
Event EV_ViewThing_SetFrame
(
	"viewsetframe",
	EV_CHEAT,
	NULL,
	NULL,
	"Set the animation to a specific frame number."
);
Event EV_ViewThing_ScaleUp
(
	"viewscaleup",
	EV_CHEAT,
	NULL,
	NULL,
	"Increase the scale of the current viewthing"
);
Event EV_ViewThing_ScaleDown
(
	"viewscaledown",
	EV_CHEAT,
	NULL,
	NULL,
	"Decrease the scale of the current viewthing"
);
Event EV_ViewThing_SetScale
(
	"viewscale",
	EV_CHEAT,
	"f",
	"scale",
	"Set the scale of the current viewthing"
);
Event EV_ViewThing_SetYaw
(
	"viewyaw",
	EV_CHEAT,
	"f",
	"yaw",
	"Set the yaw of the current viewthing"
);
Event EV_ViewThing_SetPitch
(
	"viewpitch",
	EV_CHEAT,
	"f",
	"pitch",
	"Set the pitch of the current viewthing"
);
Event EV_ViewThing_SetRoll
(
	"viewroll",
	EV_CHEAT,
	"f",
	"roll",
	"Set the roll of the current viewthing"
);
Event EV_ViewThing_SetAngles
(
	"viewangles",
	EV_CHEAT,
	"f[0,360]f[0,360]f[0,360]",
	"pitch yaw roll",
	"Set the angles of the current viewthing"
);
Event EV_ViewThing_TranslateX
(
	"viewxtranslate",
	EV_CHEAT,
	"f",
	"xtrans",
	"Translate current viewthing in x direction"
);
Event EV_ViewThing_TranslateY
(
	"viewytranslate",
	EV_CHEAT,
	"f",
	"ytrans",
	"Translate current viewthing in y direction"
);
Event EV_ViewThing_TranslateZ
(
	"viewztranslate",
	EV_CHEAT,
	"f",
	"ztrans",
	"Translate current viewthing in z direction"
);
Event EV_ViewThing_Pull
(
	"viewthingpull",
	EV_CHEAT,
	NULL,
	NULL,
	"Pulls the current viewthing to the camera"
);
Event EV_ViewThing_Spawn
(
	"viewspawn",
	EV_CHEAT,
	"s",
	"model",
	"Create a viewthing with the specified model"
);
Event EV_ViewThing_SpawnFromTS
(
	"viewspawnfromts",
	EV_CHEAT,
	"sS",
	"reloadmodel spawnmodel",
	"Create a viewthing from the model specified by the ToolServer"
);
Event EV_ViewThing_Next
(
	"viewthingnext",
	EV_CHEAT,
	NULL,
	NULL,
	"Change the active viewthing to the next viewthing"
);
Event EV_ViewThing_Prev
(
	"viewthingprev",
	EV_CHEAT,
	NULL,
	NULL,
	"Change the active viewthing to the previous viewthing"
);
Event EV_ViewThing_Attach
(
	"viewattach",
	EV_CHEAT,
	"ss",
	"tagname model",
	"Attach a model the the specified tagname"
);
Event EV_ViewThing_Detach
(
	"viewdetach",
	EV_CHEAT,
	NULL,
	NULL,
	"Detach the current viewthing from its parent"
);
Event EV_ViewThing_DetachAll
(
	"viewdetachall",
	EV_CHEAT,
	NULL,
	NULL,
	"Detach all the models attached to the current viewthing"
);
Event EV_ViewThing_Delete
(
	"viewdelete",
	EV_CHEAT,
	NULL,
	NULL,
	"Delete the current viewthing"
);
Event EV_ViewThing_SetStatic
(
	"viewsetstatic",
	EV_CHEAT,
	"i",
	"boolean",
	"Makes the current viewthing a static (not dynamically lit) object"
);
Event EV_ViewThing_SetOrigin
(
	"vieworigin",
	EV_CHEAT,
	"fff",
	"x y z",
	"Set the origin of the current viewthing"
);
Event EV_ViewThing_DeleteAll
(
	"viewdeleteall",
	EV_CHEAT,
	NULL,
	NULL,
	"Delete all viewthings"
);
Event EV_ViewThing_LastFrame
(
	"viewlastframe",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the view things last animation frame is displayed."
);
Event EV_ViewThing_SaveOffSurfaces
(
	"viewsavesurfaces",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called after the model is spawned to save off the models original surfaces."
);
Event EV_ViewThing_PrintTime
(
	"_viewthing_printtime",
	EV_CODEONLY,
	NULL,
	NULL,
	"Prints out the current level.time."
);
Event EV_ViewThing_SetAnim
(
	"viewsetanim",
	EV_CHEAT,
	"f",
	"animNum",
	"Set the animation absolutely based off a floating point value"
);
Event EV_ViewThing_NextMorph
(
	"viewnextmorph",
	EV_CHEAT,
	NULL,
	NULL,
	"Goes to the next morph for this model"
);
Event EV_ViewThing_PrevMorph
(
	"viewprevmorph",
	EV_CHEAT,
	NULL,
	NULL,
	"Goes to the previous morph for this model"
);
Event EV_ViewThing_Morph
(
	"viewmorph",
	EV_CHEAT,
	NULL,
	NULL,
	"Morphs the current morph"
);
Event EV_ViewThing_Unmorph
(
	"viewunmorph",
	EV_CHEAT,
	NULL,
	NULL,
	"Unmorphs the current morph"
);
Event EV_ViewSet_Load
(
	"viewsetload",
	EV_CHEAT,
	"s",
	"filename",
	"Loads the specified viewset"
);
Event EV_ViewSet_Save
(
	"viewsetsave",
	EV_CHEAT,
	NULL,
	NULL,
	"Saves names and positions of currently spawned things data to disk"
);
Event EV_ViewThing_Copy
(
	"viewcopy",
	EV_CHEAT,
	NULL,
	NULL,
	"Copies the current viewthing"
);
Event EV_ViewThing_Shoot
(
	"viewshoot",
	EV_CHEAT,
	NULL,
	NULL,
	"Shoots current viewthing ahead and sticks it on the wall"
);
Event EV_ViewList_List
(
	"viewlistlist",
	EV_CHEAT,
	NULL,
	NULL,
	"List all TIKIs in the current .lst set"
);
Event EV_ViewList_NextInSet
(
	"viewlistnext",
	EV_CHEAT,
	NULL,
	NULL,
	"Delete current viewthing and load next view thing in view list"
);
Event EV_ViewList_PrevInSet
(
	"viewlistprev",
	EV_CHEAT,
	NULL,
	NULL,
	"Delete current viewthing and load prev view thing in view list"
);
Event EV_ViewList_LiftModel
(
	"viewlistliftmodel",
	EV_CHEAT,
	"f",
	"z",
	"Raise the origin (z) by specified amount"
);
Event EV_ViewList_ToggleDrop
(
	"viewlisttoggledrop",
	EV_CHEAT,
	NULL,
	NULL,
	"Toggle whether objects are dropped upon spawning"
);
Event EV_ViewList_Jump
(
	"viewlistjump",
	EV_CHEAT,
	"i",
	"modelindex",
	"Jumps to a specific model in a view set (.lst file) of models"
);
Event EV_ViewThing_ResetOrigin
(
	"viewresetorigin",
	EV_CHEAT,
	NULL,
	NULL,
	"Sets the origin of current viewspawn to its current origin"
);
Event EV_ViewThing_XTrans
(
	"viewtransx",
	EV_CHEAT,
	"i",
	"offsetamount",
	"translate current viewspawn in x direction by specified amount"
);
Event EV_ViewThing_YTrans
(
	"viewtransy",
	EV_CHEAT,
	"i",
	"offsetamount",
	"translate current viewspawn in y direction by specified amount"
);
Event EV_ViewThing_ZTrans
(
	"viewtransz",
	EV_CHEAT,
	"i",
	"offsetamount",
	"translate current viewspawn in z direction by specified amount"
);
Event EV_ViewThing_Offset
(
	"viewoffset",
	EV_CHEAT,
	"fff",
	"x y z",
	"Offsets the origin of the current viewthing"
);
Event EV_ViewThing_ChangeRoll
(
	"viewchangeroll",
	EV_CHEAT,
	"i",
	"offsetamount",
	"rotate current viewspawn around x axis by specified amount"
);
Event EV_ViewThing_ChangePitch
(
	"viewchangepitch",
	EV_CHEAT,
	"i",
	"offsetamount",
	"rotate current viewspawn around y axis by specified amount"
);
Event EV_ViewThing_ChangeYaw
(
	"viewchangeyaw",
	EV_CHEAT,
	"i",
	"offsetamount",
	"rotate current viewspawn around z axis by specified amount"
);
Event EV_ViewThing_Flash
(
	"viewflash",
	EV_CHEAT,
	NULL,
	NULL,
	"Flashes the currently select viewthing"
);

CLASS_DECLARATION( Listener, ViewMaster, NULL )
{
	{ &EV_ViewThing_Spawn,				&ViewMaster::Spawn },
	{ &EV_ViewThing_SpawnFromTS,		&ViewMaster::SpawnFromTS },
	{ &EV_ViewThing_Next,				&ViewMaster::Next },
	{ &EV_ViewThing_Prev,				&ViewMaster::Prev },
	{ &EV_ViewThing_SetModel,			&ViewMaster::SetModelEvent },
	{ &EV_ViewThing_DeleteAll,			&ViewMaster::DeleteAll },
	{ &EV_ViewThing_ToggleAnimate,		&ViewMaster::PassEvent },
	{ &EV_ViewThing_NextFrame,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_PrevFrame,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_NextAnim,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_PrevAnim,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_ScaleUp,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_ScaleDown,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_SetScale,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_SetYaw,				&ViewMaster::PassEvent },
	{ &EV_ViewThing_SetPitch,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_SetRoll,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_SetAngles,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_Attach,				&ViewMaster::PassEvent },
	{ &EV_ViewThing_Detach,				&ViewMaster::PassEvent },
	{ &EV_ViewThing_DetachAll,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_SetStatic,          &ViewMaster::PassEvent },
	{ &EV_ViewThing_SetOrigin,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_SetAnim,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_NextMorph,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_PrevMorph,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_Morph,				&ViewMaster::PassEvent },
	{ &EV_ViewThing_Unmorph,			&ViewMaster::PassEvent },
	{ &EV_ViewThing_ChangeRoll,         &ViewMaster::PassEvent },
	{ &EV_ViewThing_ChangePitch,        &ViewMaster::PassEvent },
	{ &EV_ViewThing_ChangeYaw,          &ViewMaster::PassEvent },
	{ &EV_ViewThing_Flash,              &ViewMaster::PassEvent },
	{ &EV_ViewThing_SetFrame,           &ViewMaster::PassEvent },
	{ &EV_ViewSet_Load,                 &ViewMaster::LoadSet   },
	{ &EV_ViewSet_Save,                 &ViewMaster::Save      },
	{ &EV_ViewThing_Copy,               &ViewMaster::Copy      },
	{ &EV_ViewThing_Shoot,              &ViewMaster::Shoot     },
	{ &EV_ViewThing_Delete,             &ViewMaster::Delete    },
	{ &EV_ViewThing_TranslateX,         &ViewMaster::SetXTranslation },
	{ &EV_ViewThing_TranslateY,         &ViewMaster::SetYTranslation },
	{ &EV_ViewThing_TranslateZ,         &ViewMaster::SetZTranslation },
	{ &EV_ViewList_PrevInSet,           &ViewMaster::PrevModelInSet },
	{ &EV_ViewList_NextInSet,           &ViewMaster::NextModelInSet },
	{ &EV_ViewList_LiftModel,           &ViewMaster::SetZTranslation },
	{ &EV_ViewList_ToggleDrop,          &ViewMaster::ToggleDrop },
	{ &EV_ViewList_Jump,                &ViewMaster::JumpToModel },
	{ &EV_ViewThing_Pull,               &ViewMaster::PullToCamera },
	{ &EV_ViewThing_ResetOrigin,        &ViewMaster::ResetOrigin },
	{ &EV_ViewThing_XTrans,             &ViewMaster::XTranslate },
	{ &EV_ViewThing_YTrans,             &ViewMaster::YTranslate },
	{ &EV_ViewThing_ZTrans,             &ViewMaster::ZTranslate },
	{ &EV_ViewThing_Offset,             &ViewMaster::Offset },
	{ NULL, NULL }
};

ViewMaster Viewmodel;

ViewMaster::ViewMaster()
{
	current_viewthing       = NULL;
	_spawnAtLastItemsOrigin = false ;
	_currentSetIdx          = 0 ;
	_numberOfSets           = 0 ;
	_numberOfModelsInSet    = 0 ;
}

ViewMaster::~ViewMaster()
{
	_setNamesArray.FreeObjectList();
	_modelNamesArray.FreeObjectList();
}

void ViewMaster::Init( void )
{
	gi.AddCommand( "viewanimate" );
	gi.AddCommand( "viewmodel" );
	gi.AddCommand( "viewnext" );
	gi.AddCommand( "viewprev" );
	gi.AddCommand( "viewnextanim" );
	gi.AddCommand( "viewprevanim" );
	gi.AddCommand( "viewscaleup" );
	gi.AddCommand( "viewscaledown" );
	gi.AddCommand( "viewscale" );
	gi.AddCommand( "viewyaw" );
	gi.AddCommand( "viewpitch" );
	gi.AddCommand( "viewroll" );
	gi.AddCommand( "viewangles" );
	gi.AddCommand( "viewxtranslate" );
	gi.AddCommand( "viewytranslate" );
	gi.AddCommand( "viewztranslate" );
	gi.AddCommand( "viewoffset" );
	gi.AddCommand( "viewspawn" );
	gi.AddCommand( "viewspawnfromts");
	gi.AddCommand( "viewthingnext" );
	gi.AddCommand( "viewthingprev" );
	gi.AddCommand( "viewthingpull" );
	gi.AddCommand( "viewattach" );
	gi.AddCommand( "viewdetach" );
	gi.AddCommand( "viewdetachall" );
	gi.AddCommand( "viewdelete" );
	gi.AddCommand( "viewsetstatic"  );
	gi.AddCommand( "vieworiginlift" );
	gi.AddCommand( "vieworigin" );
	gi.AddCommand( "viewdeleteall" );
	gi.AddCommand( "viewsetanim"   );
	gi.AddCommand( "viewsetload"   );
	gi.AddCommand( "viewsetnext"   );
	gi.AddCommand( "viewsetprev"   );
	gi.AddCommand( "viewsetsave"   );
	gi.AddCommand( "viewcopy"      );
	gi.AddCommand( "viewlistlist"  );
	gi.AddCommand( "viewlistnext"  );
	gi.AddCommand( "viewlistprev"  );
	gi.AddCommand( "viewlistliftmodel" );
	gi.AddCommand( "viewlisttoggledrop" );
	gi.AddCommand( "viewlistjump" );
	gi.AddCommand( "viewresetorigin" );
	gi.AddCommand( "viewtransx" );
	gi.AddCommand( "viewtransy" );
	gi.AddCommand( "viewtransz" );
	gi.AddCommand( "viewflash" );
	gi.AddCommand( "viewsetframe" );
	
	g_vt_droptoggle         = gi.cvar( "g_vt_droptoggle",       "1",    0 );
	g_vt_setFilename        = gi.cvar( "g_vt_setFilename",      "",     0 );
	g_vt_modelIndex         = gi.cvar( "g_vt_modelIndex",       "0",    0 );
	g_vt_height             = gi.cvar( "g_vt_height",           "0",    0 );
	g_vt_makestatic         = gi.cvar( "g_vt_makestatic",       "1",    0 );
	g_vt_scale              = gi.cvar( "g_vt_scale",            "1.0",  0 );
	g_vt_numModels          = gi.cvar( "g_vt_numModels",        "0",    0 );
	g_vt_yaw                = gi.cvar( "g_vt_yaw",              "0.0",  0 );
	g_vt_pitch              = gi.cvar( "g_vt_pitch",            "0.0",  0 );
	g_vt_roll               = gi.cvar( "g_vt_roll",             "0.0",  0 );
	g_vt_xtrans             = gi.cvar( "g_vt_xtrans",           "0.0",  0 );
	g_vt_ytrans             = gi.cvar( "g_vt_ytrans",           "0.0",  0 );
	g_vt_ztrans             = gi.cvar( "g_vt_ztrans",           "0.0",  0 );  
	g_vt_usebox             = gi.cvar( "g_vt_usebox",           "1",    0 );
	g_vt_viewnumber         = gi.cvar( "g_vt_viewnumber",       "0",    0 );
	g_vt_viewcount          = gi.cvar( "g_vt_viewcount",        "0",    0 );
	g_vt_viewrandomize      = gi.cvar( "g_vt_viewrandomize",    "0",    0 );
	g_vt_viewsrandomize     = gi.cvar( "g_vt_viewsrandomize",   "1",    0 );
	g_vt_viewzrandomize     = gi.cvar( "g_vt_viewzrandomize",   "1",    0 );
	g_vt_viewyrandomize     = gi.cvar( "g_vt_viewyrandomize",   "0",    0 );
	g_vt_viewxrandomize     = gi.cvar( "g_vt_viewxrandomize",   "0",    0 );
	g_vt_viewminrandomize   = gi.cvar( "g_vt_viewminrandomize", "25",   0 );
	g_vt_viewmaxrandomize   = gi.cvar( "g_vt_viewmaxrandomize", "175",  0 );
	g_vt_viewbellrandomize  = gi.cvar( "g_vt_viewminrandomize", "1",    0 );
}

void ViewMaster::Save( Event * )
{
	// saves currently spawned things to disk
	str         buf;
	str         filename;
	int         count = 1 ;
	
	// get the name of the sound file from the world
	filename = "maps/";
	filename += level.mapname;
	for(int i = filename.length() - 1; i >= 0; i-- )
	{
		if ( filename[ i ] == '.' )
		{
			filename[ i ] = 0;
			break;
		}
	}
	
	filename += "_ents.map";
	gi.cvar_set("g_vt_savefile", filename.c_str());
	gi.Printf( "Saving spawned things to '%s'...\n", filename.c_str() );
	
	buf = "";
	buf += va( "//\n" );
	buf += va( "// Viewspawned entities file \"%s\".\n", filename.c_str());
	buf += va( "//\n" );
	
	Viewthing *viewthingPtr = 0 ;
	Entity    *entityPtr    = 0 ;
	entityPtr = G_FindClass( entityPtr, "viewthing" );
	
	while (entityPtr != 0)
	{
		viewthingPtr = (Viewthing*)entityPtr;
		
		str   modelName     = viewthingPtr->model ;
		char* strPtr        = (char*)modelName.c_str();
		char* endPtr        = strPtr + (strlen(strPtr) - 1) ;
		bool  found         = false ;
		
		for (int idx=strlen(strPtr); idx > 0; idx++)
		{
			if (*endPtr == '/')
			{
				endPtr++ ;
				found = true ;
				break ;
			}
			endPtr-- ;
		}			
		
		if (found)
		{
			strPtr = endPtr ;
		}
		
		buf += va( "// Entity %d\n", count);
		buf += va( "{\n");
		buf += va( "\"make_static\" \"%d\"\n",				viewthingPtr->_static);
		buf += va( "\"classname\"   \"FROM_TIKI\"\n" );
		buf += va( "\"model\"       \"%s\"\n",				strPtr);			
		buf += va( "\"scale\"       \"%f\"\n",				viewthingPtr->edict->s.scale);
		buf += va( "\"origin\"      \"%.2f %.2f %.2f\"\n",	viewthingPtr->origin.x, viewthingPtr->origin.y, viewthingPtr->origin.z );
		buf += va( "\"angles\"      \"%.2f %.2f %.2f\"\n",	viewthingPtr->angles.x, viewthingPtr->angles.y, viewthingPtr->angles.z );
		buf += va( "\"spawnflags\"  \"1\"\n");
		buf += va( "}\n" );
		
		entityPtr = G_FindClass( entityPtr, "viewthing");
		count++ ;
	}
	gi.FS_WriteFile( filename.c_str(), buf.c_str(), buf.length() + 1 );
	gi.Printf( "Done.\n" );		
}

void ViewMaster::LoadSet( Event *ev )
{
	// If we have stuff loaded, delete it all
	if (_numberOfModelsInSet || _numberOfSets)
	{
		gi.cvar_set( "viewsetname",       va( " ") );
		gi.cvar_set( "viewlistmodelname", va( " ") );
		gi.cvar_set( "g_vt_modelIndex",   "0");
		gi.cvar_set( "g_vt_setFilename",  "");
		
		_currentSetIdx       = 0 ;
		_numberOfSets        = 0 ;
		_numberOfModelsInSet = 0 ;
		_setNamesArray.FreeObjectList();
		_modelNamesArray.FreeObjectList();
	}
	
	str filename = ev->GetString(1);
	_setNamesArray.AddObject(filename);
	_numberOfSets++ ;
	
	// If we loaded any tiki filenames, display the first one
	if (_numberOfSets)
	{
		_currentSetIdx = 1 ;
		//str setFilename = _setNamesArray.ObjectAt(_currentSetIdx);
		LoadModelsInSet(_setNamesArray.ObjectAt(_currentSetIdx));
	}
}

		
void ViewMaster::LoadModelsInSet( const str &setFileName )
{	
	char  *buffer;
	char  *buf;
	char   com_token[MAX_STRING_CHARS];
	
	if (_numberOfModelsInSet)
	{
		gi.cvar_set( "viewlistmodelname", va( " ") );
		gi.cvar_set( "g_vt_modelIndex",   "0");
		_modelNamesArray.FreeObjectList();
		_numberOfModelsInSet = 0 ;
	}
	
	// Read the viewlist.txt file.  This file contains newline seperated list of .tik models
	if ( gi.FS_ReadFile( setFileName, ( void ** )&buf, true ) != -1 )
	{
		buffer = buf;
		while ( 1 )
		{
			// Pull out a token
			strcpy( com_token, COM_ParseExt( &buffer, true ) );
			
			// Quit when no more tokens
			if (!com_token[0])
				break;
			
			// Check for single comment lines
			if ((com_token[0] != '#')) {  // # is a single line comment
				const char *relative_pathname = NULL;
				
				str modelName  = "models/" ;
				
				relative_pathname = strstr( com_token, "models/" );
				
				if ( !relative_pathname )
					relative_pathname = strstr( com_token, "models\\" );
				
				if ( relative_pathname )
				{
					relative_pathname += strlen( "models/" );
					modelName += relative_pathname;
				}
				else
				{
					modelName += com_token;
				}
				
				_modelNamesArray.AddObject(modelName);
				_numberOfModelsInSet++ ;
			}
			
			// Read in the rest of the line
			while( 1 )
			{
				strcpy( com_token, COM_ParseExt( &buffer, false ) );
				if (!com_token[0])
					break;
			}
		}
		gi.FS_FreeFile( buf );
		
		// If we loaded any tiki filenames, display the first one
		if (_numberOfModelsInSet)
		{
			gi.cvar_set( "g_vt_modelIndex",  "1");
			DisplayCurrentModelInSet(false) ;
		}
		
		str setName = _setNamesArray.ObjectAt(_currentSetIdx);
		gi.cvar_set( "viewsetname",    va( "%s", setName.c_str()) );
		gi.cvar_set( "g_vt_numModels", va( "%d", _numberOfModelsInSet) );
	}
}	

void ViewMaster::DisplayCurrentModelInSet( bool spawnAtLastOrigin )
{
	if ( (g_vt_modelIndex->integer <= 0) || (g_vt_modelIndex->integer > _numberOfModelsInSet) )
	{
		gi.cvar_set( "viewlistmodelname", " " );
		gi.Printf( " No models in set or position out of range\n");
		return ; 
	}
	
	
	// Get the string name of the view item
	str modelName = _modelNamesArray.ObjectAt(g_vt_modelIndex->integer);
	
	// Create a new spawn event with item name to send to Spawn function
	Event* spawnEvent = new Event( EV_ViewThing_Spawn);
	spawnEvent->AddString(modelName);
	_spawnAtLastItemsOrigin = spawnAtLastOrigin ;
	Spawn(spawnEvent);
	_spawnAtLastItemsOrigin = false ;
	delete spawnEvent ;
	spawnEvent = 0 ;
	
	gi.cvar_set( "viewlistnumitems",  va( "%d", _numberOfModelsInSet ) );
	gi.cvar_set( "viewlistmodelname", modelName.c_str() );
	gi.cvar_set( "g_vt_scale",  "1.0");
	gi.cvar_set( "g_vt_height", "0"  );
}

//================================================================
// Name:        Copy
// Class:       ViewMaster
//              
// Description: Event function to spawn a duplicate of the current
//              viewthing at the spot occupied by the player.
//         
//              The object should be a true copy, including rotation
//              and scaling.  But currently isn't.  FIXME
//     
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::Copy( Event * )
{
	DisplayCurrentModelInSet(false);
	_updateViewthingCounts();
}

//================================================================
// Name:        Shoot
// Class:       ViewMaster
//              
// Description: Event function to spawn a duplicate of the current
//              viewthing at the spot aimed by the player.  The new
//              viewthing is not copied in scale or rotation.
//              
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::Shoot( Event * )
{
	int idx = g_vt_modelIndex->integer ;
	if ( (idx <= 0) || (idx > _modelNamesArray.NumObjects()) ) return ;
	
	Vector  forward ;
	Vector  right ;
	Vector  up ;
	Vector  view ;
	Vector  start;
	Vector	end;
	Vector  mins = vec_zero ;
	Vector  maxs = vec_zero ;
	trace_t	trace;
	
	// Check if we have a client
	Player* player = (Player*)g_entities[ 0 ].entity;
	if ( !player ) return;
	
	Viewthing* viewthing = 0 ;
	if (current_viewthing && g_vt_usebox->integer)
	{
		viewthing = (Viewthing*) ( (Entity*)current_viewthing);
		mins = viewthing->mins ;
		maxs = viewthing->maxs ;
	}
	
	view    = player->GetVAngles();
	AngleVectors( view, forward, right, up );
	start    = player->origin ;
	start.z += player->viewheight ;
	end      = start + ( forward * 10000.0f ) ;
	
	trace = G_FullTrace( start, mins, maxs, end, player, MASK_SHOT, false, "ViewMaster::shoot" );
	if ( ( trace.fraction == 1.0f ) || trace.allsolid || !trace.ent )
	{
		return ;
	}
	
	if ( g_showbullettrace->integer )
	{
		G_DebugLine( start, end, 1.0f, 1.0f, 1.0f, 1.0f );
	}
	
	Vector pos = Vector( trace.endpos ) + ( Vector( trace.plane.normal ) * 0.2f );
	
	str    modelName(_modelNamesArray.ObjectAt(g_vt_modelIndex->integer));
	spawnAtPosition(modelName, pos);
	
	if (current_viewthing)
	{
		Event* yawEvent = new Event(EV_ViewThing_SetYaw);
		viewthing = (Viewthing*) ( (Entity*)current_viewthing);
		Vector n(trace.plane.normal);
		float yaw = n.toYaw();
		yawEvent->AddFloat(yaw);
		viewthing->SetYawEvent(yawEvent);
		delete yawEvent ;
		yawEvent = 0 ;
		
		if (g_vt_viewrandomize->integer)
		{
			_randomizeViewthing(viewthing);
		}
	}
}

//================================================================
// Name:        NextModelInSet
// Class:       ViewMaster
//              
// Description: Event function to switch current_viewthing to the
//              next model in the set (if any).  This will switch
//              the current tiki model to be the next tiki model in
//              the loaded .lst file.  It does not change the number
//              of total viewthings in the world.  Wraps when at
//              end of list.
//              
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::NextModelInSet( Event * )
{
	if (!_numberOfModelsInSet)
	{
		gi.Printf(" No current set\n");
		return ;
	}
	
	// Increment on to next model
	int nextIndex = ( g_vt_modelIndex->integer >= _numberOfModelsInSet) ? 1 : (g_vt_modelIndex->integer + 1 );
	gi.cvar_set( "g_vt_modelIndex",  va( "%d", nextIndex ) );
	DeleteCurrentViewthing();
	DisplayCurrentModelInSet();
}

//================================================================
// Name:        PrevModelInSet
// Class:       ViewMaster
//              
// Description: Event function to switch current_viewthing to the
//              prev model in the set (if any).  This will switch
//              the current tiki model to be the prev tiki model in
//              the loaded .lst file.  It does not change the number
//              of total viewthings in the world.  Wraps when at 
//              start of list.
//              
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::PrevModelInSet( Event * )
{
	if (!_numberOfModelsInSet)
	{
		gi.Printf(" No models in current set\n");
		return ;
	}
	
	// Increment on to next model
	int nextIndex = ( g_vt_modelIndex->integer <= 1) ? _numberOfModelsInSet : ( g_vt_modelIndex->integer - 1 );
	gi.cvar_set( "g_vt_modelIndex",  va( "%d", nextIndex ) );
	DeleteCurrentViewthing();
	DisplayCurrentModelInSet() ;
}

//================================================================
// Name:        JumpToModel
// Class:       ViewMaster
//              
// Description: Explicity function to jump to the tiki specified
//              by number out of the total list of tiki's in the
//              load .lst file.  This enables random access thru
//              the .lst instead of one at a time.
//              
// Parameters:  Event* -- first argument is position to jump to
//              
// Returns:     None
//              
//================================================================
void ViewMaster::JumpToModel( Event *ev )
{
	int nextIndex = ev->GetInteger(1);
	if ((nextIndex < 1) || (nextIndex > _numberOfModelsInSet))
	{
		gi.Printf(" Illegal index for model in viewset list\n");
		return ;
	}
	
	gi.cvar_set( "g_vt_modelIndex",  va( "%d", nextIndex ) );
	DeleteCurrentViewthing();
	DisplayCurrentModelInSet();
}

void ViewMaster::SetXTranslation( Event *ev )
{
	if (!current_viewthing) return ;
	
	Viewthing *viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	Event *originEvent   = new Event(EV_ViewThing_SetOrigin);
	
	float y = viewthing->origin.y ;
	float z = viewthing->origin.z ;
	originEvent->AddFloat(_lastBaseOrigin.x + ev->GetFloat(1));
	originEvent->AddFloat(y);
	originEvent->AddFloat(z);
	viewthing->ChangeOrigin(originEvent);
	delete originEvent ;
	originEvent = 0 ;
	
	gi.cvar_set( "g_vt_xtrans", va("%0.2f", ev->GetFloat(1)) );
}

void ViewMaster::XTranslate( Event *ev )
{
	if (!current_viewthing) return ;
	
	Viewthing *viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	Event *originEvent   = new Event(EV_ViewThing_SetOrigin);
	
	originEvent->AddFloat(viewthing->origin.x + ev->GetFloat(1));
	originEvent->AddFloat(viewthing->origin.y);
	originEvent->AddFloat(viewthing->origin.z);
	
	viewthing->ChangeOrigin(originEvent);
	delete originEvent ;
	originEvent = 0 ;
	
	_resetOrigin();
}

void ViewMaster::SetYTranslation( Event *ev )
{
	if (!current_viewthing) return ;
	
	Viewthing *viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	Event *originEvent = new Event(EV_ViewThing_SetOrigin);
	
	float x = viewthing->origin.x ;
	float z = viewthing->origin.z ;
	originEvent->AddFloat(x);
	originEvent->AddFloat(_lastBaseOrigin.y + ev->GetFloat(1));
	originEvent->AddFloat(z);
	viewthing->ChangeOrigin(originEvent);
	delete originEvent ;
	originEvent = 0 ;
	
	gi.cvar_set( "g_vt_ytrans", va("%0.2f", ev->GetFloat(1)) );
}

void ViewMaster::YTranslate( Event *ev )
{
	if (!current_viewthing) return ;
	
	Viewthing *viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	Event *originEvent   = new Event(EV_ViewThing_SetOrigin);
	
	originEvent->AddFloat(viewthing->origin.x);
	originEvent->AddFloat(viewthing->origin.y + ev->GetFloat(1));
	originEvent->AddFloat(viewthing->origin.z);
	
	viewthing->ChangeOrigin(originEvent);
	delete originEvent ;
	originEvent = 0 ;
	
	_resetOrigin();
}

void ViewMaster::SetZTranslation( Event *ev )
{
	if (!current_viewthing) return ;
	
	Viewthing *viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	Event *originEvent = new Event(EV_ViewThing_SetOrigin);
	
	float x = viewthing->origin.x ;
	float y = viewthing->origin.y ;
	originEvent->AddFloat(x);
	originEvent->AddFloat(y);
	originEvent->AddFloat(_lastBaseOrigin.z + ev->GetFloat(1));
	viewthing->ChangeOrigin(originEvent);
	delete originEvent ;
	originEvent = 0 ;
	
	gi.cvar_set( "g_vt_ztrans", va("%0.2f", ev->GetFloat(1)) );
}

void ViewMaster::ZTranslate( Event *ev )
{
	if (!current_viewthing) return ;
	
	Viewthing *viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	Event *originEvent   = new Event(EV_ViewThing_SetOrigin);
	
	originEvent->AddFloat(viewthing->origin.x);
	originEvent->AddFloat(viewthing->origin.y);
	originEvent->AddFloat(viewthing->origin.z + ev->GetFloat(1));
	
	viewthing->ChangeOrigin(originEvent);
	delete originEvent ;
	originEvent = 0 ;
	
	_resetOrigin();
}

void ViewMaster::Offset( Event *ev )
{
	Viewthing *viewthing;
	Event *new_event;
	
	if ( !current_viewthing ) 
		return;
	
	viewthing = ( Viewthing * )( Entity * )current_viewthing;
	
	new_event = new Event( EV_ViewThing_SetOrigin );
	
	new_event->AddFloat( viewthing->origin.x + ev->GetFloat( 1 ) );
	new_event->AddFloat( viewthing->origin.y + ev->GetFloat( 2 ) );
	new_event->AddFloat( viewthing->origin.z + ev->GetFloat( 3 ) );
	
	viewthing->ProcessEvent( new_event );
	
	_resetOrigin();
}

//================================================================
// Name:        ResetOrigin
// Class:       ViewMaster
//              
// Description: Forces the _lastBaseOrigin to be the origin of the
//              current_viewthing (if any).  Calls _resetOrigin to
//              do the work.
//
//              This is useful because models can be translated off
//              of _lastBaseOrigin and reset to it.  This enables
//              the _lastBaseOrigin to move to where ever the viewthing
//              has been translated too so further relative movements are 
//              relative to this new position.
//              
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::ResetOrigin( Event* )
{
	_resetOrigin();
}

//================================================================
// Name:        PullToCamera
// Class:       ViewMaster
//              
// Description: Moves the current_viewthing (if any) to the current
//              player's (client[0]) position.  Does not drop the
//              object but instead explicitly calls _setToPosition
//              to force the origin of the current_viewthing to be
//              at player's positionn.
//              
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::PullToCamera( Event* )
{
	if (!current_viewthing) return ;
	
	// Check if we have a client
	Entity* clientEntity = g_entities[ 0 ].entity;
	if (!clientEntity) return ;
	
	// Store this new camera position as being where items should spawn here on out
	_lastBaseOrigin = clientEntity->origin ;
	_setToPosition(clientEntity->origin);
}

//================================================================
// Name:        ToggleDrop
// Class:       ViewMaster
//              
// Description: Toggles the g_vt_droptoggle cvar. When the
//              cvar is true, objects are dropped upon spawning.
//              
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::ToggleDrop( Event * )
{
	if (g_vt_droptoggle->integer)
	{
		gi.cvar_set("g_vt_droptoggle", "0");
	}
	else
	{
		gi.cvar_set("g_vt_droptoggle", "1");
	}
}

//================================================================
// Name:        Next
// Class:       ViewMaster
//              
// Description: Takes an event and moves to next spawned viewthing
//              if there is one.  Does not change current_viewthing
//              if there isn't a next viewthing.
//              
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::Next( Event * )
{
	_selectNext();
}

//================================================================
// Name:        Prev
// Class:       ViewMaster
//              
// Description: Responds to an event and moves to previous spawned
//              viewthing if there is one.  Does not change current_viewthing
//              if there isn't a previous viewthing.
//              
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::Prev( Event * )
{
	_selectPrevious();
}

//================================================================
// Name:        DeleteCurrentViewthing
// Class:       ViewMaster
//              
// Description: Support function that simply sends a EV_Remove event
//              to the current viewthing if there is one.  This will
//              delete the current viewthing.
//              
//              Does not modify current_viewthing nor the count of
//              viewthings because it is expected that the caller 
//              handles this.
//
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void ViewMaster::DeleteCurrentViewthing()
{
	if (current_viewthing)
	{
		// delete all existing models
		Viewthing *viewthing = (Viewthing*)( (Entity*) current_viewthing);
		viewthing->PostEvent( EV_Remove, 0.0f );		
	}
}

//================================================================
// Name:        Delete
// Class:       ViewMaster
//              
// Description: Deletes the current_viewthing if there is one.  Attempts
//              to set the current_viewthing to the previous one in the
//              list of viewthings.  If it cannot do that, it sets it
//              to the next one.  If it cannot do that, it sets current_viewthing
//              to 0 (NULL).
//              
// Parameters:  Event* -- Not used
//              
// Returns:     None
//              
//================================================================
void ViewMaster::Delete( Event * )
{
	if (!current_viewthing) return ;
	
	// Save off the one to delete
	Viewthing *viewthing = (Viewthing*)( (Entity*) current_viewthing);
	
	_selectPrevious();                  // Try and select previous
	
	if (current_viewthing == viewthing) // If same as one we are deleting, there wasn't a previous
	{
		_selectNext();                  // Try and select next
	}
	
	if (current_viewthing == viewthing) // If same as one we are deleting, there wasn't a next
	{
		current_viewthing = 0 ;         // set it to 0
	}
	
	// Delete the one we saved off
	viewthing->PostEvent( EV_Remove, 0.0f );
	_updateViewthingCounts(-1);         // we pass -1 here because there will be one less viewthing but it hasn't actually been deleted yet
}

//================================================================
// Name:        DeleteAll
// Class:       ViewMaster
//              
// Description: Deletes all viewspawn things currently in memory.
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void ViewMaster::DeleteAll( Event * )
{
	Entity *next;
	
	for( next = G_FindClass( NULL, "viewthing" ); next != NULL; next = G_FindClass( next, "viewthing" ) )
	{
		next->PostEvent( EV_Remove, 0.0f );
	}
	
	current_viewthing = NULL;
}

//================================================================
// Name:        spawnAtPosition
// Class:       ViewMaster
//              
// Description: Spawns the specified tiki model at the specified
//              position.  The current_viewthing will point to this
//              new viewthing if the spawn succeeds, 0 if it doesn't.
//              
// Parameters:  const str&    -- name of tiki model ("models/eliza.tik")
//              const Vector& -- Position to spawn model at
//
// Returns:     None
//              
//================================================================
void ViewMaster::spawnAtPosition( const str& modelName, const Vector& pos )
{
	// create a new viewthing
	Viewthing *viewthing  = new Viewthing;
	current_viewthing     = viewthing;
	viewthing->baseorigin = pos ;
	viewthing->setOrigin( viewthing->baseorigin );
	_lastBaseOrigin = viewthing->baseorigin ;
	
	if (g_vt_droptoggle->integer)
	{
		viewthing->droptofloor( 10000.0f );
	}
	
	viewthing->setAngles( vec_zero );
	
	Event* event = new Event( EV_ViewThing_SetModel );
	event->AddString( modelName );
	viewthing->ProcessEvent( event );
	
	if ( !gi.IsModel( viewthing->edict->s.modelindex ) )
	{
		delete viewthing;
		current_viewthing = 0 ;
		return;
	}
	_updateViewthingCounts();
}

//================================================================
// Name:			SpawnFromTS
// Class:			ViewMaster
//
// Description:		Called by the ToolServer to viewspawn a model
//					the ToolServer sends over dynamically.
//
// Parameters:		Event *ev -- Point to event whose first string argument
//                        is name of tiki model to reload, the second string
//						  is the tiki to respawn.  If the second string isn't
//						  specified, it respawns the first string
//
// Returns:			None
//
//================================================================
void ViewMaster::SpawnFromTS( Event *ev )
{
	char *data = (char*)gi.ToolServerGetData();
	
	if (!data) // No data? Just abort.
		return;
	
	DeleteAll(NULL); // Overkill?
	
	char spawnmodel[256], reloadmodel[256];
	
	strcpy(reloadmodel, "models/");
	strcat(reloadmodel, ev->GetString(1));
	
	if ( ev->NumArgs() > 1 )
	{
		// Already has models/ on it
		strcpy(spawnmodel, ev->GetString(2));
	}
	else
	{
		strcpy(spawnmodel,"models/");
		strcat(spawnmodel,  ev->GetString(1));
	}
	
	gi.DPrintf("Reloading Model: %s...\n",reloadmodel);
	gi.TikiLoadFromTS(reloadmodel, data);
	// Setting this config string causes the client to reload the surfaces for this tiki
	gi.setConfigstring( CS_DEBUGINFO, va( "reload %s %d", spawnmodel, gi.modelindex(reloadmodel) ) );
	
	// Create a new spawn event with item name to send to Spawn function
	Event* spawnEvent = new Event( EV_ViewThing_Spawn);
	spawnEvent->AddString(spawnmodel);
	Spawn(spawnEvent);
	delete spawnEvent;
	spawnEvent = 0 ;
	
	gi.setConfigstring( CS_DEBUGINFO, "" );
}

//================================================================
// Name:        Spawn
// Class:       ViewMaster
//              
// Description: Spawns the tiki model specified in the Event's
//              first string argument.  The current_viewthing points
//              to the spawned object if the spawn succeeds, 0
//              if it doesn't.
// 
//              The spawned object is dropped straight down if
//              the cvar g_vt_droptoggle is set to true (default).
//              This will cause objects to spawn on the ground.
//              
// Parameters:  Event* -- Point to event whose first string argument
//                        is name of tiki model to spawn ("model/eliza.tik")
//              
// Returns:     None
//              
//================================================================
void ViewMaster::Spawn( Event *ev )
{
	const char	*mdl;
	Viewthing	*viewthing;
	Entity		*ent;
	Event		*event;
	Vector		 forward;
	Vector		 up;
	Vector		 delta;
	
	mdl = ev->GetString( 1 );
	if ( !mdl || !mdl[ 0 ] )
	{
		ev->Error( "Must specify a model name" );
		return;
	}
	
	// Check if we have a client
	ent = g_entities[ 0 ].entity;
	assert( ent );
	
	//-----------------------------------------
	// This if block attempts to keep the current
	// model index of a set up to date when an explicit
	// viewspawn command is issued.  If the model that's
	// being spawn is within the set, the index in the
	// set will be updated accordingly.
	// 
	// If it isn't found in the set, we simply continue.
	//-----------------------------------------
	if (_numberOfModelsInSet)
	{
		str modelName(mdl);
		int pos = g_vt_modelIndex->integer ;
		if ((pos > 1) && (pos < _modelNamesArray.NumObjects()) && (_modelNamesArray.ObjectAt(pos) != modelName) )
		{
			int pos = _modelNamesArray.IndexOfObject(modelName);
			if (pos)
			{
				gi.cvar_set( "g_vt_modelIndex", va("%d", pos) );
				DeleteCurrentViewthing();
				DisplayCurrentModelInSet();
				return ;
			}
		}
	}
	
	// create a new viewthin and set current_viewthing to it.
	viewthing         = new Viewthing;
	current_viewthing = viewthing;
	
	//FIXME FIXME
	ent->angles.AngleVectors( &forward, NULL, &up );
	
	// If _spawnAtLastItemsOrigin is set, we will spawn on
	if (_spawnAtLastItemsOrigin)
	{
		float upOffset = (g_vt_droptoggle->integer) ? 48 : 0 ;
		viewthing->baseorigin = _lastBaseOrigin ;
		viewthing->baseorigin += forward ;
		viewthing->baseorigin += (up * upOffset);
	}
	else {
		viewthing->baseorigin = ent->origin;
		viewthing->baseorigin += ( forward * 48.0f );
		viewthing->baseorigin += ( up * 48.0f );
	}
	
	viewthing->setOrigin( viewthing->baseorigin );
	
	// Added this so that guns and such can be prevented from falling into the floor.
	// Controlled by viewmodeltoggledrop command.  Default is true, so same as old functionality
	if (g_vt_droptoggle->integer)
	{
		viewthing->droptofloor( 10000.0f );
	}

	viewthing->baseorigin = viewthing->origin;
	_lastBaseOrigin = viewthing->baseorigin ;

	delta = ent->origin - viewthing->origin;
	viewthing->setAngles( vec_zero );

	event = new Event( EV_ViewThing_SetModel );
	event->AddString( mdl );
	viewthing->ProcessEvent( event );

	if ( !gi.IsModel( viewthing->edict->s.modelindex ) )
	{
		ev->Error( "model %s not found, viewmodel not spawned.", mdl );
		delete viewthing;
		current_viewthing = NULL;
		return;
	}

	_updateViewthingCounts();
}

//================================================================
// Name:        SetModelEvent
// Class:       ViewMaster
//              
// Description: Sets the model for the current viewthing.  Passes
//              a SetModelEvent down to the current viewthing. The
//              Event must contain the model name as its first string
//              argument.
//              
// Parameters:  Event* -- First string argument contains model name
//              
// Returns:     None
//              
//================================================================
void ViewMaster::SetModelEvent( Event *ev )
{
	const char	*mdl;
	char			str[ 128 ];
	Event			*event;
	Viewthing	*viewthing;
	
	mdl = ev->GetString( 1 );
	if ( !mdl || !mdl[ 0 ] )
	{
		ev->Error( "Must specify a model name" );
		return;
	}
	
	if ( !current_viewthing )
	{
		// try to find one on the map
		current_viewthing = G_FindClass( NULL, "viewthing" );
		if ( !current_viewthing )
		{
			ev->Error( "No viewmodel" );
			return;
		}
	}
	
	viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	
	// Prepend 'models/' to make things easier
	str[ 0 ] = 0;
	if ( ( mdl[ 1 ] != ':' ) && strnicmp( mdl, "models", 6 ) )
	{
		strcpy( str, "models/" );
	}
	strcat( str, mdl );
	
	event = new Event( EV_ViewThing_SetModel );
	event->AddString( str );
	viewthing->ProcessEvent( event );
	viewthing->UpdateCvars();
}

//================================================================
// Name:        PassEvent
// Class:       ViewMaster
//              
// Description: Passes an event down to the current_viewthing if
//              it is set.
//              
// Parameters:  Event* -- Event to pass
//              
// Returns:     None
//              
//================================================================
void ViewMaster::PassEvent( Event *ev )
{
	Viewthing *viewthing;
	Event *event;
	
	if ( !current_viewthing )
	{
		ev->Error( "No viewmodel" );
		return;
	}
	
	viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	if ( viewthing )
	{
		event = new Event( ev );
		viewthing->ProcessEvent( event );
	}
}


//---------------------------------------------------------------------------
//              P R O T E C T E D   M E T H O D S 
//---------------------------------------------------------------------------


//================================================================
// Name:        _resetOrigin
// Class:       ViewMaster
//              
// Description: Sets the origin for the current viewthing to be wherever
//              it is at.  This also sets the _lastBaseOrigin, which
//              means further relative translations are based from here.
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void ViewMaster::_resetOrigin()
{
	if (!current_viewthing) return ;
	
	Viewthing *viewthing = ( Viewthing*) ( (Entity*) current_viewthing);
	_lastBaseOrigin = viewthing->origin ;
	_setToPosition(_lastBaseOrigin); // just ensure we update cvars
}

//================================================================
// Name:        _setToPosition
// Class:       ViewMaster
//              
// Description: Sets the Position of the current viewthing.
//              
// Parameters:  Vector& pos -- vector of position to set origin to
//              
// Returns:     None
//              
//================================================================
void ViewMaster::_setToPosition( const Vector& pos )
{
	// Create a new event to pass to viewthing with new origin
	Event* originEvent = new Event(EV_ViewThing_SetOrigin);
	
	originEvent->AddFloat(pos.x);
	originEvent->AddFloat(pos.y);
	originEvent->AddFloat(pos.z);
	
	// Pass to the viewthing and clean up
	Viewthing *viewthing = (Viewthing*)( (Entity*)current_viewthing );
	viewthing->ChangeOrigin(originEvent);
	delete originEvent ;
	originEvent = 0 ;
	
	gi.cvar_set( "g_vt_xtrans", "0" );
	gi.cvar_set( "g_vt_ytrans", "0" );
	gi.cvar_set( "g_vt_ztrans", "0" );
}

//================================================================
// Name:        _selectNext
// Class:       ViewMaster
//              
// Description: Selects the next viewthing in the set of viewthings
//              Selects none if there are no more viewthings.
//
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void ViewMaster::_selectNext()
{
	Viewthing *viewthing;
	Entity * ent;
	
	ent = G_FindClass( current_viewthing, "viewthing" );
	if ( ent )
	{
		if (current_viewthing) 
		{
			viewthing = ( Viewthing * )( ( Entity * )current_viewthing );				
			viewthing->SetSelected( false );
		}
		
		current_viewthing = ent;
		viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
		_lastBaseOrigin = viewthing->origin ;
		viewthing->UpdateCvars();
		viewthing->SetSelected( true );
		gi.Printf( "current viewthing model %s.\n", viewthing->model.c_str() );
		_updateViewthingCounts();
	}
}

//================================================================
// Name:        _selectPrev
// Class:       ViewMaster
//              
// Description: Selects the previous viewthing in the set of viewthings.
//              Selects none if there are no previous viewthings.
//              
// Parameters:  None
//              
// Returns:     None
//              
//================================================================
void ViewMaster::_selectPrevious()
{
	Viewthing   *viewthing;
	Entity      *prev;
	Entity      *next;
	
	next = NULL;
	do
	{
		prev = next;
		next = G_FindClass( prev, "viewthing" );
	}
	while( next != current_viewthing );
	
	if ( prev )
	{
		if (current_viewthing) 
		{
			viewthing = ( Viewthing * )( ( Entity * )current_viewthing );				
			viewthing->SetSelected( false );
		}
		
		current_viewthing = prev;
		viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
		_lastBaseOrigin = viewthing->origin ;
		viewthing->UpdateCvars();
		viewthing->SetSelected(true);
		gi.Printf( "current viewthing model %s.\n", viewthing->model.c_str() );
		_updateViewthingCounts();
	}
}		

//================================================================
// Name:        _updateViewthingCounts
// Class:       ViewMaster
//              
// Description: Sets the cvars g_vt_viewnumber and g_vt_viewcount
//              appropriately after any change to the number of 
//              spawned viewthings in the world.  When the loop
//              finishes, g_vt_viewcount is the total number of 
//              spawned viewthings and g_vt_viewnumber is position
//              in this list of the current_viewthing.
//              
//              The passed in parameter can be used for manipulating
//              the final count.  Because deletion of viewthings is
//              event driven, when we know there is going to be a
//              deletion the deletion hasn't actually occured yet.
//              So it would show up in the count.  Hence, we can pass
//              in a -1 to offset the final count appropriately.
//              Hack.
//
// Parameters:  int countAdjustment -- offset final count (default is 0).
//              
// Returns:     None
//              
//================================================================
void ViewMaster::_updateViewthingCounts(int countAdjustment)
{
	Entity* next = G_FindClass( NULL, "viewthing");
	int     numberOfViewthings    = 0 ;
	int     currentViewthingIndex = 0 ;
	
	while (next)
	{
		numberOfViewthings++ ;
		if (next == current_viewthing)
		{
			currentViewthingIndex = numberOfViewthings ;
		}
		next = G_FindClass( next, "viewthing");
	}
	
	gi.cvar_set( "g_vt_viewnumber", va("%d", currentViewthingIndex) );
	gi.cvar_set( "g_vt_viewcount",  va("%d", numberOfViewthings + countAdjustment) );
}

//================================================================
// Name:        _randomizeViewthing
// Class:       ViewMaster
//              
// Description: Randomizes the scaling and rotation of the specified
//              viewthing, based on various cvar settings.  These
//              randomizations can be turned on and off individually,
//              but they are only applied if g_vt_viewrandomize is true.
//              
//              The bell curve randomization is done in the scaling section.
//              It works like this:
//                o Add two random values between 0 and 100 and divide total by 100.  (Bell curve value--try with two dice)
//                o Get half distance between desired low and high ((high-low) divided by 2)
//                o Scale this half-distance by the bell curve value computed in step 1 (yields # from 0 to 2*half, but tends towards halfway)
//                o Add scaled half to low value, giving bell curve between desired low and high
//
// Parameters:  Viewthing* -- viewthing to randomize
//              
// Returns:     None
//              
//================================================================
void ViewMaster::_randomizeViewthing( Viewthing* viewthingPtr )
{
	if (!viewthingPtr)
	{
		gi.Printf( "No viewthing to randomize\n");
		return ;
	}
	
	if (g_vt_viewsrandomize->integer)
	{
		// Randomly scale
		float die1        = G_Random( 100.0f );
		float die2        = G_Random( 100.0f );
		float scaleValue  = (die1 + die2) / 100.0f ;
		
		float midpoint    = ( (float)g_vt_viewmaxrandomize->integer - (float)g_vt_viewminrandomize->integer) / 2.0f ;
		float adjustment  = (float)g_vt_viewminrandomize->integer + (midpoint * scaleValue);
		float scaleFactor = (adjustment / 100.0f);
		
		Event* scaleEvent = new Event(EV_ViewThing_SetScale);
		scaleEvent->AddFloat(scaleFactor);
		viewthingPtr->SetScaleEvent(scaleEvent);
		delete scaleEvent ;
		scaleEvent = 0 ;
	}
	
	if (g_vt_viewzrandomize->integer)
	{
		// Randomly rotate around z-axis
		Event* rotateEvent = new Event(EV_ViewThing_SetYaw);
		rotateEvent->AddFloat(G_Random(360.0f));
		viewthingPtr->SetYawEvent(rotateEvent);
		delete rotateEvent ;
		rotateEvent = 0 ;
	}
	
	if (g_vt_viewyrandomize->integer)
	{
		// Randomly rotate around z-axis
		Event* rotateEvent = new Event(EV_ViewThing_SetPitch);
		rotateEvent->AddFloat(G_Random(360.0f));
		viewthingPtr->SetPitchEvent(rotateEvent);
		delete rotateEvent ;
		rotateEvent = 0 ;
	}
	
	if (g_vt_viewxrandomize->integer)
	{
		// Randomly rotate around z-axis
		Event* rotateEvent = new Event(EV_ViewThing_SetRoll);
		rotateEvent->AddFloat(G_Random(360.0f));
		viewthingPtr->SetRollEvent(rotateEvent);
		delete rotateEvent ;
		rotateEvent = 0 ;
	}
}

//------------------------------------------------------------------------
//
//                          VIEWTHING
//
//------------------------------------------------------------------------

CLASS_DECLARATION( Entity, Viewthing, "viewthing" )
{
	{ &EV_ViewThing_Think,				&Viewthing::ThinkEvent },
	{ &EV_ViewThing_LastFrame,     		&Viewthing::LastFrameEvent },
	{ &EV_ViewThing_ToggleAnimate,		&Viewthing::ToggleAnimateEvent },
	{ &EV_ViewThing_SetModel,			&Viewthing::SetModelEvent },
	{ &EV_ViewThing_NextFrame,			&Viewthing::NextFrameEvent },
	{ &EV_ViewThing_PrevFrame,			&Viewthing::PrevFrameEvent },
	{ &EV_ViewThing_NextAnim,			&Viewthing::NextAnimEvent },
	{ &EV_ViewThing_PrevAnim,			&Viewthing::PrevAnimEvent },
	{ &EV_ViewThing_ScaleUp,			&Viewthing::ScaleUpEvent },
	{ &EV_ViewThing_ScaleDown,			&Viewthing::ScaleDownEvent },
	{ &EV_ViewThing_SetScale,			&Viewthing::SetScaleEvent },
	{ &EV_ViewThing_SetYaw,				&Viewthing::SetYawEvent },
	{ &EV_ViewThing_SetPitch,			&Viewthing::SetPitchEvent },
	{ &EV_ViewThing_SetRoll,			&Viewthing::SetRollEvent },
	{ &EV_ViewThing_SetAngles,			&Viewthing::SetAnglesEvent },
	{ &EV_ViewThing_SetStatic,          &Viewthing::SetStatic },
	{ &EV_ViewThing_Attach,				&Viewthing::AttachModel },
	{ &EV_ViewThing_Detach,				&Viewthing::Delete },
	{ &EV_ViewThing_DetachAll, 			&Viewthing::DetachAll },
	{ &EV_ViewThing_Delete,				&Viewthing::Delete },
	{ &EV_ViewThing_SetOrigin,			&Viewthing::ChangeOrigin },
	{ &EV_ViewThing_SaveOffSurfaces,	&Viewthing::SaveSurfaces },
	{ &EV_ViewThing_PrintTime,			&Viewthing::PrintTime },
	{ &EV_ViewThing_SetAnim,			&Viewthing::SetAnim },
	{ &EV_ViewThing_NextMorph,			&Viewthing::NextMorph },
	{ &EV_ViewThing_PrevMorph,			&Viewthing::PrevMorph },
	{ &EV_ViewThing_Morph,				&Viewthing::Morph },
	{ &EV_ViewThing_Unmorph,			&Viewthing::Unmorph },
	{ &EV_ViewThing_ChangeRoll,         &Viewthing::ChangeRollEvent },
	{ &EV_ViewThing_ChangePitch,        &Viewthing::ChangePitchEvent },
	{ &EV_ViewThing_ChangeYaw,          &Viewthing::ChangeYawEvent },
	{ &EV_ViewThing_Flash,              &Viewthing::Flash },
	{ &EV_ViewThing_SetFrame,           &Viewthing::SetFrame },

	{ NULL, NULL }
};

Viewthing::Viewthing( void )
{
	animate    = new Animate( this );
	frame       = 0;
	animstate   = 0;
	_static     = (g_vt_makestatic->integer != 0) ? true : false ;
	_selected   = false ;
	_pulseCount = 0 ;
	
	setSolidType( SOLID_NOT );
	baseorigin = origin;
	Viewmodel.current_viewthing = this;
	edict->s.eType = ET_MODELANIM;
	edict->s.renderfx |= RF_SHADOW;
	edict->s.renderfx |= RF_SHADOW_PRECISE;
	edict->s.renderfx |= RF_EXTRALIGHT;
	
	current_morph = 0;
	
	gi.cvar_set( "g_vt_scale",  "1.0" );
	gi.cvar_set( "g_vt_roll",   "0.0" );
	gi.cvar_set( "g_vt_pitch",  "0.0" );
	gi.cvar_set( "g_vt_yaw",    "0.0" );
	gi.cvar_set( "g_vt_xtrans", "0.0" );
	gi.cvar_set( "g_vt_ytrans", "0.0" );
	gi.cvar_set( "g_vt_ztrans", "0.0" );
	
	
	// save off surfaces once the model is spawned
	PostEvent( EV_ViewThing_SaveOffSurfaces, FRAMETIME );
	
	PostEvent( EV_ViewThing_Think, FRAMETIME );
}


void Viewthing::SetSelected( qboolean state )
{
	_selected   = state ;
	_pulseCount = 0 ;
	
	if (this->hidden())
	{
		this->showModel();
	}
}

void Viewthing::Flash( Event * )
{
	SetSelected(true);
}

void Viewthing::PrintTime( Event *ev )
{
	gi.Printf( "ev current frame %d leveltime %.2f\n", ev->GetInteger( 1 ), level.time );
}

void Viewthing::UpdateCvars( qboolean quiet )
{
	
	gi.cvar_set( "viewmodelanim",      animate->AnimName() );
	gi.cvar_set( "viewmodeltotaltime", va( "%.3f", animate->AnimTime()) );
	gi.cvar_set( "viewmodelanimframes",va( "%d", animate->NumFrames()) );
	gi.cvar_set( "viewmodelframe",     va( "%d", CurrentFrame() ) );
	gi.cvar_set( "viewmodelname",      model.c_str() );
	gi.cvar_set( "viewmodelscale",     va( "%0.2f", edict->s.scale ) );
	gi.cvar_set( "currentviewthing",   model.c_str() );
	gi.cvar_set( "viewthingorigin",    va( "%0.2f,%0.2f,%0.2f", edict->s.origin[0],edict->s.origin[1],edict->s.origin[2] ) );
	gi.cvar_set( "viewmodelanimnum",   va( "%.3f", ( float )CurrentAnim() / ( float )animate->NumAnims() ) );
	gi.cvar_set( "g_vt_scale",         va( "%.2f", edict->s.scale) );
	gi.cvar_set( "g_vt_roll",          va( "%.2f", (angles.z <= 180.0f) ? angles.z : (angles.z - 360.0f)) );
	gi.cvar_set( "g_vt_pitch",         va( "%.2f", (angles.x <= 180.0f) ? angles.x : (angles.x - 360.0f)) );
	gi.cvar_set( "g_vt_yaw",           va( "%.2f", (angles.y <= 180.0f) ? angles.y : (angles.y - 360.0f)) );
	gi.cvar_set( "g_vt_xtrans",        va( "%.2f", (Viewmodel._lastBaseOrigin.x - baseorigin.x) ) );
	gi.cvar_set( "g_vt_ytrans",        va( "%.2f", (Viewmodel._lastBaseOrigin.y - baseorigin.y) ) );
	gi.cvar_set( "g_vt_ztrans",        va( "%.2f", (Viewmodel._lastBaseOrigin.z - baseorigin.z) ) );
	
	if ( gi.Morph_NameForNum( edict->s.modelindex, current_morph ) )
		gi.cvar_set( "viewmorphname", gi.Morph_NameForNum( edict->s.modelindex, current_morph ) );
	else
		gi.cvar_set( "viewmorphname", "" );
	
	if ( !quiet )
	{
		gi.Printf( "%s, frame %3d, scale %4.2f\n", animate->AnimName(), CurrentFrame(), edict->s.scale );
	}
}


void Viewthing::ThinkEvent( Event * )
{
	static float startTime = 0 ;
	
	if (_selected && (_pulseCount < 20)) 
	{
		if (startTime == 0.0f)
		{
			startTime = level.time ;
		}
		
		if ( (level.time - startTime) > 0.15f)
		{
			if (this->hidden())
			{
				this->showModel();
			}
			else 
			{
				this->hideModel();
			}
			startTime = 0.0f ;
			_pulseCount++ ;
		}
	}
	
	
	int f;
	if (animstate >= 2)
	{
		Vector   forward;
		Vector	left;
		Vector	up;
		Vector   realmove;
		
		angles.AngleVectors( &forward, &left, &up );
		realmove = ( left * total_delta[1] ) + ( up * total_delta[2] ) + ( forward * total_delta[0] );
		setOrigin( baseorigin + realmove );
		gi.cvar_set( "viewthingorigin", va( "%0.2f,%0.2f,%0.2f", edict->s.origin[0],edict->s.origin[1],edict->s.origin[2] ) );
	}
	PostEvent( EV_ViewThing_Think, FRAMETIME );
	if ( ( animstate > 0 ) && ( Viewmodel.current_viewthing == this ) )
	{
		f = CurrentFrame();
		if ( f != lastframe )
		{
			float time;
			lastframe = f;
			time = f * animate->AnimTime() / (float)animate->NumFrames();
			gi.Printf( "current frame %d time %.2f\n", f, time );
			gi.cvar_set( "viewmodeltime", va( "%.2f", time ) );
			gi.cvar_set( "viewmodelframe", va( "%d", f ) );
			gi.cvar_set( "viewmodelanim", animate->AnimName() );
		}
	}
}

void Viewthing::LastFrameEvent( Event * )
{
	if ( animstate != 3 )
	{
		total_delta = Vector(0, 0, 0);
	}
}

void Viewthing::ToggleAnimateEvent( Event * )
{
	animstate = ( animstate + 1 ) % 4;
	total_delta = Vector(0, 0, 0);
	setOrigin( baseorigin );
	
	// reset to a known state
	switch( animstate )
	{
	case 0:
		animate->SetFrame( frame );
		gi.Printf( "Animation stopped.\n" );
		gi.cvar_set( "viewmodelanimmode", "Stopped" );
		break;
	case 1:
		animate->NewAnim( CurrentAnim() );
		gi.Printf( "Animation no motion.\n" );
		gi.cvar_set( "viewmodelanimmode", "No Motion" );
		break;
	case 2:
		animate->NewAnim( CurrentAnim(), EV_ViewThing_LastFrame );
		gi.Printf( "Animation with motion and looping.\n" );
		gi.cvar_set( "viewmodelanimmode", "Motion and Looping" );
		break;
	case 3:
		animate->NewAnim( CurrentAnim(), EV_ViewThing_LastFrame );
		gi.Printf( "Animation with motion no looping.\n" );
		gi.cvar_set( "viewmodelanimmode", "Motion and No Looping" );
		break;
	}
	UpdateCvars( true );
}

void Viewthing::SetStatic( Event *ev )
{
	if (ev->NumArgs() == 1) 
	{
		_static = (ev->GetInteger(1)) ? true : false ;
	}
	else
	{
		_static = (g_vt_makestatic->integer != 0) ? true : false ;
	}
}

void Viewthing::SetModelEvent( Event *ev )
{
	str modelname;
	
	modelname = ev->GetString( 1 );
	
	setModel( modelname );
	
	if ( gi.IsModel( edict->s.modelindex ) )
	{
		// Default the viewthing to an idle animation if the model has one
		
		if ( animate->HasAnim( "idle" ) )
			animate->RandomAnimate( "idle" );
		else
			animate->NewAnim( 0 );
		
		frame = 0;
		animate->SetFrame( 0 );
	}
	
	UpdateCvars();
}

void Viewthing::NextFrameEvent( Event * )
{
	int numframes;
	
	numframes = animate->NumFrames();
	if ( numframes )
	{
		frame = (frame+1)%numframes;
		animate->SetFrame( frame );
		animstate = 0;
		UpdateCvars();
	}
}

void Viewthing::PrevFrameEvent( Event * )
{
	int numframes;
	
	numframes = animate->NumFrames();
	if ( numframes )
	{
		frame = (frame-1)%numframes;
		animate->SetFrame( frame );
		animstate = 0;
		UpdateCvars();
	}
}

void Viewthing::SetAnim( Event *ev )
{
	int numanims, anim;
	
	numanims = animate->NumAnims();
	if ( numanims )
	{
		// restore original surfaces
		memcpy( edict->s.surfaces, origSurfaces, sizeof( origSurfaces ) );
		
		anim = ev->GetFloat( 1 ) * numanims;
		if ( anim >= numanims )
			anim = numanims - 1;
		animate->NewAnim( anim % numanims );
		frame = 0;
		animate->SetFrame( frame );
		animstate = 0;
		UpdateCvars();
	}
}

void Viewthing::SetAnim( int num )
{
	int numanims;
	
	numanims = animate->NumAnims();
	if ( numanims )
	{
		// restore original surfaces
		memcpy( edict->s.surfaces, origSurfaces, sizeof( origSurfaces ) );
		
		// if ( num >= numanims )
		//   num = numanims - 1;
		animate->NewAnim( num );
		animate->SetFrame( frame );
		//animstate = 0;
		UpdateCvars();
	}
}

void Viewthing::NextAnimEvent( Event * )
{
	int numanims;
	
	numanims = animate->NumAnims();
	if ( numanims )
	{
		// restore original surfaces
		memcpy( edict->s.surfaces, origSurfaces, sizeof( origSurfaces ) );
		
		animate->NewAnim( ( CurrentAnim() + 1 ) % numanims );
		frame = 0;
		animate->SetFrame( frame );
		animstate = 0;
		gi.Printf( "Animation stopped.\n" );
		gi.cvar_set( "viewmodelanimmode", "Stopped" );
		UpdateCvars();
	}
}

void Viewthing::PrevAnimEvent( Event * )
{
	int anim;
	int numanims;
	
	numanims = animate->NumAnims();
	if ( numanims )
	{
		// restore original surfaces
		memcpy( edict->s.surfaces, origSurfaces, sizeof( origSurfaces ) );
		
		anim = CurrentAnim() - 1;
		while( anim < 0 )
		{
			anim += numanims;
		}
		animate->NewAnim( anim );
		frame = 0;
		animate->SetFrame( frame );
		animstate = 0;
		gi.Printf( "Animation stopped.\n" );
		gi.cvar_set( "viewmodelanimmode", "Stopped" );
		UpdateCvars();
	}
}

void Viewthing::ScaleUpEvent( Event * )
{
	edict->s.scale += 0.05f;
	UpdateCvars();
}

void Viewthing::ScaleDownEvent( Event * )
{
	edict->s.scale -= 0.05f;
	UpdateCvars();
}

void Viewthing::SetScaleEvent( Event *ev )
{
	if ( ev->NumArgs() )
	{
		float s = ev->GetFloat( 1 );
		edict->s.scale = s;
		UpdateCvars();
	}
}

void Viewthing::SetYawEvent( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		angles.setYaw( ev->GetFloat( 1 ) );
		setAngles( angles );
		UpdateCvars();
	}
}

void Viewthing::SetPitchEvent( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		angles.setPitch( ev->GetFloat( 1 ) );
		setAngles( angles );
		UpdateCvars();
	}
}

void Viewthing::SetRollEvent( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		angles.setRoll( ev->GetFloat( 1 ) );
		setAngles( angles );
		UpdateCvars();
	}
}

void Viewthing::ChangeRollEvent( Event *ev )
{
	if ( ev->NumArgs() > 0)
	{
		angles.setRoll(angles.roll() + ev->GetFloat( 1 ) );
		setAngles(angles);
		UpdateCvars();
	}
}

void Viewthing::ChangePitchEvent( Event *ev )
{
	if ( ev->NumArgs() > 0)
	{
		angles.setPitch(angles.pitch() + ev->GetFloat( 1 ) );
		setAngles(angles);
		UpdateCvars();
	}
}

void Viewthing::ChangeYawEvent( Event *ev )
{
	if ( ev->NumArgs() > 0)
	{
		angles.setYaw(angles.yaw() + ev->GetFloat( 1 ) );
		setAngles(angles);
		UpdateCvars();
	}
}

void Viewthing::SetAnglesEvent( Event *ev )
{
	if ( ev->NumArgs() > 2 )
	{
		angles.x = ev->GetFloat( 1 );
		angles.y = ev->GetFloat( 2 );
		angles.z = ev->GetFloat( 3 );
		setAngles( angles );
	}
	gi.cvar_set( "g_vt_roll",  va ("%0.2f", angles.x) );
	gi.cvar_set( "g_vt_pitch", va ("%0.2f", angles.y) );
	gi.cvar_set( "g_vt_yaw",   va ("%0.2f", angles.z) );
	gi.Printf( "angles = %0.4f, %0.4f, %0.4f\n", angles.x, angles.y, angles.z );
}

//--------------------------------------------------------------
//
// Name:			AttachModel
// Class:			Viewthing
//
// Description:		Attaches a model to a viewspawned model
//
// Parameters:		Event *ev
//						string -- model name
//
// Returns:			None
//
//--------------------------------------------------------------
void Viewthing::AttachModel(Event *ev)
{
	Event *event = 0;
	Viewthing *child = 0;
	str mdl = ev->GetString(2);

	child = new Viewthing;
	child->setModel( mdl.c_str() );

	// Check to make sure it exists.
	if ( !gi.IsModel( child->edict->s.modelindex ) )
	{
		ev->Error( "attachmodel %s not found, attachmodel not spawned.", mdl.c_str() );
		delete child;
		Viewmodel.ProcessEvent(EV_ViewThing_Prev);
		return;
	}

	// Attach the child
	event = new Event( EV_Attach );
	event->AddEntity( this );
	event->AddString( ev->GetString( 1 ) );
	child->ProcessEvent( event );
}

void Viewthing::Delete( Event * )
{
	Viewmodel.current_viewthing = NULL;
	PostEvent( EV_Remove, 0.0f );
	Viewmodel.PostEvent( EV_ViewThing_Next, 0.0f );
}

void Viewthing::Delete()
{
	PostEvent( EV_Remove, 0.0f );
}

void Viewthing::DetachAll( Event * )
{
	int i;
	int num;
	Entity * ent;
	
	if ( bind_info )
	{
		num = bind_info->numchildren;
		
		for (i=0;i<MAX_MODEL_CHILDREN;i++)
		{
			if (!bind_info->children[i])
				continue;
			
			ent = ( Entity * )G_GetEntity( bind_info->children[i] );
			ent->PostEvent( EV_Remove, 0.0f );
			num--;
			
			if (!num)
				break;
		}
	}
}

void Viewthing::ChangeOrigin( Event *ev )
{
	if ( ev->NumArgs() )
	{
		origin.x = ev->GetFloat( 1 );
		origin.y = ev->GetFloat( 2 );
		origin.z = ev->GetFloat( 3 );
		setOrigin( origin );
		baseorigin = origin;
		gi.cvar_set( "g_vt_height", va ("%f", origin.z) );
		UpdateCvars();
	}
	gi.Printf( "vieworigin = x%0.4f y%0.4f z%0.4f\n", origin.x, origin.y, origin.z );
}

void Viewthing::SaveSurfaces( Event * )
{
	memcpy( origSurfaces, edict->s.surfaces, sizeof( origSurfaces ) );
}


void Viewthing::NextMorph( Event * )
{
	current_morph++;
	
	if ( current_morph >= gi.NumMorphs( edict->s.modelindex ) )
		current_morph = 0;
	
	if ( gi.Morph_NameForNum( edict->s.modelindex, current_morph ) )
		gi.Printf( "morph %s\n", gi.Morph_NameForNum( edict->s.modelindex, current_morph ) );
	
	UpdateCvars( true );
}

void Viewthing::PrevMorph( Event * )
{
	current_morph--;
	
	if ( current_morph < 0 )
		current_morph = gi.NumMorphs( edict->s.modelindex ) - 1;
	
	if ( gi.Morph_NameForNum( edict->s.modelindex, current_morph ) )
		gi.Printf( "morph %s\n", gi.Morph_NameForNum( edict->s.modelindex, current_morph ) );
	
	UpdateCvars( true );
}

void Viewthing::Morph( Event * )
{
	const char *morph_name;
	
	morph_name = gi.Morph_NameForNum( edict->s.modelindex, current_morph );
	
	if ( morph_name )
	{
		Event *new_event;
		
		new_event = new Event ( EV_Morph );
		new_event->AddString( morph_name );
		//new_event->AddFloat( 50 );
		ProcessEvent( new_event );
	}
}

void Viewthing::Unmorph( Event * )
{
	const char *morph_name;
	
	morph_name = gi.Morph_NameForNum( edict->s.modelindex, current_morph );
	
	if ( morph_name )
	{
		Event *new_event;
		
		new_event = new Event ( EV_Unmorph );
		new_event->AddString( morph_name );
		ProcessEvent( new_event );
	}
}

void Viewthing::SetFrame( Event *ev )
{
	int newframe;
	
	newframe = ev->GetInteger(1);
	if ( newframe < animate->NumFrames() )
	{
		animate->SetFrame( newframe );
		animstate = 0;
		UpdateCvars();
	}
}
