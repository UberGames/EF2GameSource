//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/entity.cpp           $
// $Revision:: 271                                                           $
//   $Author:: Steven                                                   $
//     $Date:: 10/13/03 9:43a                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Base class for all enities that are controlled by the game.  If you have any
// object that should be called on a periodic basis and it is not an entity,
// then you have to have an dummy entity that calls it.
//
// An entity in the game is any object that is not part of the world.  Any non-world
// object that is visible in the game is an entity, although it is not required that
// all entities be visible to the player.  Some objects are basically just virtual
// constructs that act as an instigator of certain actions, for example, some
// triggers are invisible and cannot be touched, but when activated by other
// objects can cause things to happen.
//
// All entities are capable of receiving messages from the game or from other entities.
// Messages received by an entity may be ignored, passed on to their superclass,
// or acted upon by the entity itself.  The programmer must decide on the proper
// action for the entity to take to any message.  There will be many messages
// that are completely irrelevant to an entity and should be ignored.  Some messages
// may require certain states to exist and if they are received by an entity when
// it these states don't exist may indicate a logic error on the part of the
// programmer or map designer and should be reported as warnings (if the problem is
// not severe enough for the game to be halted) or as errors (if the problem should
// not be ignored at any cost).
//

#include "_pch_cpp.h"
//#include "g_local.h"
#include "entity.h"
#include "worldspawn.h"
#include "scriptmaster.h"
#include "sentient.h"
#include "specialfx.h"
#include "misc.h"
#include "object.h"
#include "player.h"
#include "weaputils.h"
#include "soundman.h"
#include "earthquake.h"
#include <qcommon/qfiles.h>
#include "mp_manager.hpp"
#include <qcommon/gameplaymanager.h>


// Player events
Event EV_ClientMove
	(
	"client_move",
   EV_DEFAULT,
	NULL,
	NULL,
	"The movement packet from the client is processed by this event."
	);
Event EV_ClientEndFrame
	(
	"client_endframe",
	EV_DEFAULT,
	NULL,
	NULL,
	"Called at the end of each frame for each client."
	);

// Generic entity events
Event EV_Classname
	(
	"classname" ,
	EV_TIKIONLY,
	"s",
	"nameOfClass",
	"Determines what class to use for this entity,\n"
	"this is pre-processed from the BSP at the start\n"
	"of the level."
	);
Event EV_SpawnFlags
	(
	"spawnflags",
	EV_CODEONLY,
	"i",
	"flags",
	"spawnflags from the BSP, these are set inside the editor"
	);
Event EV_SetTeam
	(
	"team",
	EV_DEFAULT,
	"s",
	"moveTeam",
	"used to make multiple entities move together."
	);
Event EV_Trigger
	(
	"trigger",
	EV_SCRIPTONLY,
	"s",
	"name",
	"Trigger the specified target or entity."
	);
Event EV_Activate
	(
	"doActivate",
	EV_DEFAULT,
	"e",
	"activatingEntity",
	"General trigger event for all entities"
	);
Event EV_Use
	(
	"doUse",
	EV_DEFAULT,
	"e",
	"activatingEntity",
	"sent to entity when it is used by another entity"
	);

Event EV_FadeNoRemove
	(
	"fade",
	EV_DEFAULT,
	"F[0,]F[0,1]",
	"fadetime target_alpha",
	"Fade the entity's alpha, reducing it by 0.03\n"
	"every FRAMETIME, until it has faded out, does not remove the entity"
	);

Event EV_FadeOut
	(
	"_fadeout",
	EV_CODEONLY,
	NULL,
	NULL,
	"Fade the entity's alpha and scale out, reducing it by 0.03\n"
	"every FRAMETIME, until it has faded out, removes the entity\n"
   "Once the entity has been completely faded, the entity is removed."
	);

Event EV_Fade
	(
	"fadeout",
	EV_DEFAULT,
	"F[0,]F[0,1]",
	"fadetime target_alpha",
	"Fade the entity's alpha and scale out, reducing it by 0.03\n"
	"every FRAMETIME, until it has faded out. If fadetime or\n"
   "target_alpha are defined, they will override the defaults.\n"
   "Once the entity has been completely faded, the entity is removed."
	);
Event EV_FadeIn
	(
	"fadein",
	EV_DEFAULT,
   "F[0,]F[0,1]",
   "fadetime target_alpha",
	"Fade the entity's alpha and scale in, increasing it by 0.03\n"
	"every FRAMETIME, until it has faded completely in to 1.0.\n"
   "If fadetime or target_alpha are defined, they will override\n"
   "the default values."
	);
 Event EV_Killed
	(
	"killed",
	EV_DEFAULT,
	"efei",
	"attacker damage inflictor meansofdeath",
	"event which is sent to an entity once it as been killed"
	);
Event EV_GotKill
	(
	"gotkill" ,
	EV_DEFAULT,
	"eieib",
	"victim damage inflictor meansofdeath gib",
	"event sent to attacker when an entity dies"
	);
Event EV_Pain
	(
	"pain",
	EV_DEFAULT,
	"iei",
	"damage attacker meansofdeath",
	"used to inflict pain to an entity"
	);
Event EV_Damage
	(
	"_damage",
	EV_CODEONLY,
	"feevvviii",
	"damage inflictor attacker position direction normal knockback damageflags meansofdeath",
	"general damage event used by all entities"
	);
Event EV_Stun
	(
	"_stun",
	EV_CODEONLY,
	"f",
	"time",
	"Stun this entity for the specified time"
	);
Event EV_Kill
	(
	"kill",
	EV_CONSOLE,
	NULL,
	NULL,
	"console based command to kill yourself if stuck."
	);
Event EV_Gib
	(
	"gib",
	EV_DEFAULT,
	"iIFS",
	"number power scale gibmodel",
	"causes entity to spawn a number of gibs"
	);
Event EV_Hurt
	(
	"hurt",
	EV_DEFAULT,
	"fSV",
	"damage means_of_death direction",
	"Inflicts damage if the entity is damageable.  If the number of damage\n"
	"points specified in the command argument is greater or equal than the\n"
	"entity's current health, it will be killed or destroyed."
	);

Event EV_TakeDamage
	(
	"takedamage",
	EV_DEFAULT,
	NULL,
	NULL,
	"makes entity take damage."
	);
Event EV_NoDamage
	(
	"nodamage",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity does not take damage."
	);

Event EV_Stationary
	(
	"stationary",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity does not move, causes no physics to be run on it."
	);

// Physics events
Event EV_MoveDone
	(
	"movedone",
	EV_DEFAULT,
	"e",
	"finishedEntity",
	"Sent to commanding thread when done with move ."
	);
Event EV_Touch
	(
	"doTouch",
	EV_DEFAULT,
	"e",
	"touchingEntity",
	"sent to entity when touched for the first time."
	);
Event EV_Contact
	(
	"inContactWithEntity",
	EV_DEFAULT,
	"e",
	"entityWrapper",
	"sent to an entity that is in contact with specified entity."
	);
Event EV_LostContact
	(
	"lostContactWithEntity",
	EV_DEFAULT,
	"e",
	"entityNoLongerInContact",
	"sent to an entity no longer in contact with specified entity."
	);
Event EV_Blocked
	(
	"doBlocked",
	EV_DEFAULT,
	"e",
	"obstacle",
	"sent to entity when blocked."
	);
Event EV_UseBoundingBox
	(
	"usebbox",
	EV_DEFAULT,
	NULL,
	NULL,
	"do not perform perfect collision, use bounding box instead."
	);
Event EV_Gravity
	(
	"gravity",
	EV_DEFAULT,
	"f",
	"gravityValue",
	"Change the gravity on this entity"
	);
Event EV_Stop
	(
	"stopped",
	EV_DEFAULT,
	NULL,
	NULL,
	"sent when entity has stopped bouncing for MOVETYPE_TOSS."
	);
Event EV_SetFullTrace
	(
	"fulltrace",
	EV_DEFAULT,
	"b",
	"on_or_off",
	"Turns fulltrace physics movement on or off for this entity."
	);

Event EV_ProcessInitCommands
	(
	"processinit",
	EV_CODEONLY,
	"i",
	"modelIndex",
	"process the init section of this entity, this is an internal command,\n"
	"it is not meant to be called from script."
	);
Event EV_Attach
	(
	"attach",
	EV_DEFAULT,
	"esIVV",
	"parent tagname use_angles offset angles_offset",
	"attach this entity to the parent's legs tag called tagname"
	);
Event EV_AttachModel
	(
	"attachmodel",
	EV_DEFAULT,
	"ssFSBFFFFVV",
	"modelname tagname scale targetname detach_at_death removetime fadeintime fadeoutdelay fadetime offset angles_offset",
	"attach a entity with modelname to this entity to tag called tagname.\n"
	"scale           - scale of attached entities\n"
	"targetname      - targetname for attached entities\n"
	"detach_at_death - when entity dies, should this model be detached.\n"
	"removetime      - when the entity should be removed, if not specified, never.\n"
   "fadeintime      - time to fade the model in over.\n"
   "fadeoutdelay    - time to wait until we fade the attached model out\n"
   "fadeoutspeed    - time the model fades out over\n"
	"offset		     - vector offset for the model from the specified tag\n"
	"angles_offset   - angles offset for the model from the specified tag"
	);
Event EV_RemoveAttachedModel
	(
	"removeattachedmodel",
	EV_DEFAULT,
	"sFS",
	"tagname fadetime modelname",
	"Removes the model attached to this entity at the specified tag."
	);
Event EV_RemoveAttachedModelByTargetname
(
	"removeAttachedModelByTargetname",
	EV_DEFAULT,
	"s",
	"targetname",
	"Removes all models that are attached to this entity with the specified targetname."
);
Event EV_Detach
	(
	"detach",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"detach this entity from its parent."
	);

Event EV_IncreaseShotCount
   (
	"increaseshotcount",
	EV_DEFAULT,
	NULL,
	NULL,
	"boosts the shotsFired on this entity's parent if it is an actor"
	);

// script stuff
Event EV_Model
	(
	"model",
	EV_DEFAULT,
	"s", 
	"modelName",
	"set the model to modelName."
	);
Event EV_Hide
	(
	"hide",
	EV_DEFAULT,
	NULL,
	NULL,
	"hide the entity, opposite of show."
	);
Event EV_Show
	(
	"show",
	EV_DEFAULT,
	NULL,
	NULL,
	"show the entity, opposite of hide."
	);
Event EV_BecomeSolid
	(
	"solid",
	EV_DEFAULT,
	NULL,
	NULL,
	"make solid."
	);
Event EV_BecomeNonSolid
	(
	"notsolid",
	EV_DEFAULT,
	NULL,
	NULL,
	"make non-solid."
	);
Event EV_Ghost
	(
	"ghost",
	EV_DEFAULT,
	NULL,
	NULL,
	"make non-solid but still send to client regardless of hide status."
	);
Event EV_TouchTriggers
	(
	"touchtriggers",
	EV_DEFAULT,
   "B",
   "touch_triggers_bool",
   "Sets whether the entity should touch triggers or not"
	);
Event EV_Sound
	(
	"playsound",
	EV_DEFAULT,
	"sIFS",
	"soundName channel volume min_distance",
	"play a sound coming from this entity.\n"
	"default channel, CHAN_BODY."
	);
Event EV_StopSound
	(
	"stopsound",
	EV_DEFAULT,
	"I",
	"channel",
	"stop the current sound on the specified channel.\n"
	"default channel, CHAN_BODY."
	);
Event EV_Bind
	(
	"bind",
	EV_DEFAULT,
	"e",
	"parent",
	"bind this entity to the specified entity."
	);
Event EV_Unbind
	(
	"unbind",
	EV_DEFAULT,
	NULL,
	NULL,
	"unbind this entity."
	);
Event EV_JoinTeam
	(
	"joinTeam",
	EV_DEFAULT,
	"e",
	"teamMember",
	"join a bind team."
	);
Event EV_QuitTeam
	(
	"quitTeam",
	EV_DEFAULT,
	NULL,
	NULL,
	"quit the current bind team"
	);
Event EV_SetHealth
   (
   "health",
   EV_CHEAT | EV_TIKIONLY,
   "i",
   "newHealth",
   "set the health of the entity to newHealth"
   );
Event EV_GetHealth
   (
   "gethealth",
   EV_CHEAT | EV_SCRIPTONLY,
   "@f",
   "currentHealth",
   "Gets the health of the entity"
   );
Event EV_SetMaxHealth
	(
	"maxhealth",
	EV_CHEAT,
	"i",
	"newMaxHealth",
	"set the max_health of the entity to newMaxHealth"
	);
Event EV_SetScale
   (
   "scale",
   EV_DEFAULT,
   "f",
   "newScale",
   "set the scale of the entity"
   );
Event EV_SetRandomScale
   (
   "randomScale",
   EV_DEFAULT,
   "fd",
   "minScale maxScale",
   "Set the scale of the entity randomly between minScale and maxScale"
   );
Event EV_SetSize
   (
   "setsize",
   EV_DEFAULT,
   "vv",
   "mins maxs",
   "Set the bounding box of the entity to mins and maxs."
   );
Event EV_GetMins
	(
	"getmins",
	EV_SCRIPTONLY,
	"@v",
	"returnval",
	"Returns the minimum extent of the bounding box"
	);
Event EV_GetMaxs
	(
	"getmaxs",
	EV_SCRIPTONLY,
	"@v",
	"returnval",
	"Returns the maximum extent of the bounding box"
	);
Event EV_SetMins
   (
   "_setmins",
   EV_CODEONLY,
   "v",
   "mins",
   "Set the mins of the bounding box of the entity to mins."
   );
Event EV_SetMaxs
   (
   "_setmaxs",
   EV_CODEONLY,
   "v",
   "maxs",
   "Set the maxs of the bounding box of the entity to maxs."
   );
Event EV_SetAlpha
   (
   "alpha",
   EV_DEFAULT,
   "f",
   "newAlpha",
   "Set the alpha of the entity to alpha."
   );
Event EV_SetOrigin
   (
   "origin",
   EV_SCRIPTONLY,
   "v",
   "newOrigin",
   "Set the origin of the entity to newOrigin."
   );
Event EV_SetOriginEveryFrame
	(
	"setorigineveryframe",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the origin every frame ( for bound objects )"
	);

Event EV_GetOrigin
   (
   "getorigin",
   EV_CHEAT | EV_SCRIPTONLY,
   "@v",
   "returnval",
   "Gets the origin of the entity."
   );
Event EV_SetTargetName
   (
   "targetname",
   EV_SCRIPTONLY,
   "s",
   "targetName",
   "set the targetname of the entity to targetName."
   );
Event EV_GetTargetName
   (
   "getTargetName",
   EV_SCRIPTONLY,
   "@s",
   "targetName",
   "Gets the targetname of the entity with the leading $."
   );
Event EV_GetRawTargetName
   (
   "getRawTargetName",
   EV_SCRIPTONLY,
   "@s",
   "rawTargetName",
   "Gets the targetname of the entity without the leading $."
   );
Event EV_SetTarget
   (
   "target",
   EV_DEFAULT,
   "s",
   "targetname_to_target",
   "target another entity with targetname_to_target."
   );
Event EV_GetTarget
   (
   "getTarget",
   EV_SCRIPTONLY,
   "@sB",
   "name_of_current_target wantsPrefix",
   "Returns the name of the current target."
   );

Event EV_GetTargetEntity
	(
	"gettargetentity",
	EV_SCRIPTONLY,
	"@e",
	"NULL",
	"Returns the current target entity"
	);

Event EV_SetKillTarget
   (
   "killtarget",
   EV_DEFAULT,
   "s",
   "targetName",
   "when dying kill entities with this targetName."
   );
Event EV_GetModelName
	(
	"getmodelname",
	EV_SCRIPTONLY,
	"@s",
	"modelname",
	"Retrieves the model name of the entity"
	);
Event EV_SetAngles
   (
   "angles",
   EV_SCRIPTONLY,
   "v[0,360][0,360][0,360]",
   "newAngles",
   "set the angles of the entity to newAngles."
   );
Event EV_GetAngles
	(
	"getangles",
	EV_SCRIPTONLY,
	"@v",
	"angle_vector",
	"Retrieves the entity's angles"
	);
Event EV_SetAngle
   (
   "angle",
   EV_DEFAULT,
   "f",
   "newAngle",
   "set the angles of the entity using just one value.\n"
   "Sets the yaw of the entity or an up and down\n"
   "direction if newAngle is [0-359] or -1 or -2"
   );
Event EV_RegisterAlias
   (
   "alias",
   EV_CACHE,
   "ssSSSSSS",
   "alias realname parameter1 parameter2 parameter3 parameter4 parameter5 paramater6",
   "create an alias for the given realname\n"
   "Valid parameters are as follows:\n"
   "global - all instances act as one\n"
   "stop - once used, don't use again\n"
   "timeout [seconds] - once used wait [seconds] until using again\n"
   "maxuse [times] - can only be used [times] times.\n"
   "weight [weight] - random weighting"
   );
Event EV_RegisterAliasAndCache
   (
   "aliascache",
   EV_CACHE,
   "ssSSSSSS",
   "alias realname parameter1 parameter2 parameter3 parameter4 parameter5 paramater6",
   "create an alias for the given realname and cache the resource\n"
   "Valid parameters are as follows:\n"
   "global - all instances act as one\n"
   "stop - once used, don't use again\n"
   "timeout [seconds] - once used wait [seconds] until using again\n"
   "maxuse [times] - can only be used [times] times.\n"
   "weight [weight] - random weighting"
   );
Event EV_Cache
   (
   "cache",
   EV_CACHE,
   "s",
   "resourceName",
   "pre-cache the given resource."
   );
Event EV_AutoCache
   (
   "autocache",
   EV_CACHE | EV_TIKIONLY,
   "s",
   "resourceName",
   "pre-cache the given resource."
   );
Event EV_SetMass
   (
   "mass",
   EV_TIKIONLY,
   "i",
   "massAmount",
   "set the mass of this entity."
   );

Event EV_LoopSound
   (
   "loopsound",
   EV_DEFAULT,
   "sFS",
   "soundName volume minimum_distance",
   "play a looped-sound with a certain volume and minimum_distance\n"
   "which is attached to the current entity."
   );
Event EV_StopLoopSound
   (
   "stoploopsound",
   EV_DEFAULT,
   NULL,
   NULL,
   "Stop the looped-sound on this entity."
   );

Event EV_SurfaceModelEvent
   (
   "surface",
   EV_DEFAULT,
   "sSSSSSS",
   "surfaceName parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "change a legs surface parameter for the given surface.\n"
   "+ sets the flag, - clears the flag\n"
   "Valid surface commands are:\n"
   "skin1 - set the skin1 offset bit\n"
   "skin2 - set the skin2 offset bit\n"
   "nodraw - don't draw this surface"
   );


Event EV_SetAnimOnAttachedModel
   (
	"attachanim",
	EV_DEFAULT,
	"ss",
	"anim_name tag_name",
	"sets the anim on an attached entity at the specified tag"
	);

Event EV_SetCinematicAnim
	(
	"setcinematicanim",
	EV_DEFAULT,
	"s",
	"anim_name",
	"sets a cinematic anim on an entity"
	);

Event EV_CinematicAnimDone
	(
	"cinematicanimdone",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when an entity's cinematic animation is done, "
	"it is not meant to be called from script."
	);

Event EV_SetEntityExplosionModel
   (
   "setentityexplosionmodel",
	EV_DEFAULT,
	"s",
	"model_name",
	"sets this entitys explosion model which is used in selfDetonate"
	);

// AI sound events
Event EV_BroadcastSound
   (
   "soundevent",
   EV_DEFAULT,
   "FS",
   "soundRadius soundtype",
   "Let the AI know that this entity made a sound,\n"
   "radius determines how far the sound reaches."
   );
Event EV_HeardSound
   (
   "heardsound",
   EV_DEFAULT,
   "evi",
   "noisyEntity noisyLocation soundtype",
   "sent to entities when another makes a sound, not for script use\n"
   );

// Conditionals
Event EV_IfSkill
   (
   "ifskill",
   EV_DEFAULT,
   "isSSSSS",
   "skillLevel command argument1 argument2 argument3 argument4 argument5",
   "if the current skill level is skillLevel than execute command"
   );

// Lighting
Event EV_SetLight
   (
   "light",
   EV_DEFAULT,
   "ffff",
   "color_red color_green color_blue radius",
   "Create a dynmaic light on this entity."
   );

Event EV_LightOn
   (
   "lightOn",
   EV_DEFAULT,
   NULL,
   NULL,
   "Turn the configured dynmaic light on this entity on."
   );
Event EV_LightOff
   (
   "lightOff",
   EV_DEFAULT,
   NULL,
   NULL,
   "Turn the configured dynamic light on this entity off."
   );
Event EV_LightStyle
   (
   "lightStyle",
   EV_DEFAULT,
   "i",
   "lightStyleIndex",
   "What light style to use for this dynamic light on this entity."
   );
Event EV_LightRed
   (
   "lightRed",
   EV_DEFAULT,
   "f",
   "red",
   "Set the red component of the dynmaic light on this entity."
   );
Event EV_LightGreen
   (
   "lightGreen",
   EV_DEFAULT,
   "f",
   "green",
   "Set the green component of the dynmaic light on this entity."
   );
Event EV_LightBlue
   (
   "lightBlue",
   EV_DEFAULT,
   "f",
   "blue",
   "Set the blue component of the dynmaic light on this entity."
   );
Event EV_LightRadius
   (
   "lightRadius",
   EV_DEFAULT,
   "f",
   "radius",
   "Set the radius of the dynmaic light on this entity."
   );

// Entity flag specific
Event EV_EntityFlags
   (
   "flags",
   EV_DEFAULT,
   "SSSSSS",
   "parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "Change the current entity flags.\n"
   "Valid flags are as follows:\n"
   "+ sets a flag, - clears a flag\n"
   "blood - should it bleed\n"
   "explode - should it explode when dead\n"
   "die_gibs - should it spawn gibs when dead\n"
   "god - makes the entity invincible\n"
	"notarget - makes the entity notarget\n"
   );
Event EV_EntityRenderEffects
   (
   "rendereffects",
   EV_DEFAULT,
   "SSSSSS",
   "parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "Change the current render effects flags.\n"
   "Valid flags are as follows:\n"
   "+ sets a flag, - clears a flag\n"
   "dontdraw - send the entity to the client, but don't draw\n"
   "betterlighting - do sphere based vertex lighting on the entity\n"
   "lensflare - add a lens glow to the entity at its origin\n"
   "viewlensflare - add a view dependent lens glow to the entity at its origin\n"
   "lightoffset - use the dynamic color values as a light offset to the model\n"
   "skyorigin - this entity is the portal sky origin\n"
   "minlight - this entity always has some lighting on it\n"
   "fullbright - this entity is always fully lit\n"
   "additivedynamiclight - the dynamic light should have an additive effect\n"
   "lightstyledynamiclight - the dynamic light uses a light style, use the\n"
   "depthhack - this entity is drawn with depth hack on\n"
   "'lightstyle' command to set the index of the light style to be used"

   );
Event EV_EntityEffects
   (
   "effects",
   EV_DEFAULT,
   "SSSSSS",
   "parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "Change the current entity effects flags.\n"
   "Valid flags are as follows:\n"
   "+ sets a flag, - clears a flag\n"
   "everyframe - process commands every time entity is rendered"
   );
Event EV_EntitySVFlags
   (
   "svflags",
   EV_DEFAULT,
   "SSSSSS",
   "parameter1 parameter2 parameter3 parameter4 parameter5 parameter6",
   "Change the current server flags.\n"
   "Valid flags are as follows:\n"
   "+ sets a flag, - clears a flag\n"
   "broadcast - always send this entity to the client"
   );

// Special Effects
Event EV_Censor
   (
   "censor",
   EV_TIKIONLY,
   NULL,
   NULL,
   "used to ban certain contact when in parentmode\n"
   );
Event EV_Explosion
   (
   "explosionattack",
   EV_DEFAULT,
   "sS",
   "explosionModel tagName",
   "Spawn an explosion optionally from a specific tag"
   );
Event EV_DoRadiusDamage
	(
	"doradiusdamage",
	EV_DEFAULT,
	"fsffBF",
	"damage meansofdeath radius knockback constant_damage repeat_time",
	"calls RadiusDamage() using the entity information and owner"
	);
Event EV_SelfDetonate
   (
	"selfdetonate",
	EV_DEFAULT,
	NULL,
	NULL,
	"spawns and explosion and removes this entity"
	);

Event EV_ShaderEvent
   (
   "shader",
   EV_DEFAULT,
   "sfF",
   "shaderCommand argument1 argument2",
   "change a specific shader parameter for the entity.\n"
   "Valid shader commands are:\n"
   "translation [trans_x] [trans_y] - change the texture translation\n"
   "offset [offset_x] [offset_y] - change the texture offset\n"
   "rotation [rot_speed] - change the texture rotation speed\n"
   "frame [frame_num] - change the animated texture frame\n"
   "wavebase [base] - change the base parameter of the wave function\n"
   "waveamp [amp] - change the amp parameter of the wave function\n"
   "wavebase [phase] - change the phase parameter of the wave function\n"
   "wavefreq [freq] - change the frequency parameter of the wave function\n"
   );

Event EV_ScriptShaderEvent
   (
   "scriptshader",
   EV_DEFAULT,
   "sfF",
   "shaderCommand argument1 argument2",
   "alias for shader command, change a specific shader parameter for the entity.\n"
   "Valid shader commands are:\n"
   "translation [trans_x] [trans_y] - change the texture translation\n"
   "offset [offset_x] [offset_y] - change the texture offset\n"
   "rotation [rot_speed] - change the texture rotation speed\n"
   "frame [frame_num] - change the animated texture frame\n"
   "wavebase [base] - change the base parameter of the wave function\n"
   "waveamp [amp] - change the amp parameter of the wave function\n"
   "wavebase [phase] - change the phase parameter of the wave function\n"
   "wavefreq [freq] - change the frequency parameter of the wave function\n"
   );

Event EV_KillAttach
   (
   "killattach",
   EV_DEFAULT,
   NULL,
   NULL,
   "kill all the attached entities."
   );
Event EV_DropToFloor
	(
	"droptofloor",
	EV_CODEONLY,
	"F",
	"maxRange",
	"drops the entity to the ground, if maxRange is not specified 8192 is used."
	);
Event EV_AddToSoundManager
	(
	"_addtosoundmanager",
	EV_CODEONLY,
	NULL,
	NULL,
	"adds the current entity to the sound manager."
	);
Event EV_SetControllerAngles
	(
	"setcontrollerangles",
	EV_CODEONLY,
	"iv",
	"num angles",
	"Sets the control angles for the specified bone."
	);
Event EV_DeathSinkStart
	(
	"deathsinkstart",
	EV_CODEONLY,
	NULL,
	NULL,
	"Makes the entity sink into the ground and then get removed (this starts it)."
	);
Event EV_DeathSink
	(
	"deathsinkeachframe",
	EV_CODEONLY,
	NULL,
	NULL,
	"Makes the entity sink into the ground and then get removed (this gets called each frame)."
	);
Event EV_DamageType
	(
	"damage_type",
	EV_DEFAULT,
	"s",
   "meansofdeathstring",
	"Set the type of damage that this entity can take"
	);
Event EV_LookAtMe
	(
	"lookatme",
	EV_DEFAULT,
	NULL,
   NULL,
	"Makes the player look at this object if close."
	);
Event EV_ProjectilesCanStickToMe
	(
	"projectilecansticktome",
	EV_DEFAULT,
	"b",
	"can_stick",
	"Bool that determines whether projectiles stick in this entity."
	);
Event EV_DetachAllChildren
	(
	"detachallchildren",
	EV_SCRIPTONLY,
	NULL,
   NULL,
	"Detach all the children from the entity."
	);
Event EV_Morph
	(
	"morph",
	EV_DEFAULT,
	"sFFB",
   "morph_target_name final_percent morph_time return_to_zero",
	"Morphs to the specified morph target"
	);
Event EV_Unmorph
	(
	"unmorph",
	EV_DEFAULT,
	"s",
   "morph_target_name",
	"Unmorphs the specified morph target"
	);
Event EV_MorphControl
	(
	"morphcontrol",
	EV_CODEONLY,
	NULL,
   NULL,
	"Does all of the morph work each frame"
	);

Event EV_SetObjectProgram
   (
   "setobjectprogram",
	EV_DEFAULT,
	"s",
	"program",
	"sets this objects program"
	);

Event EV_ExecuteObjectProgram
   (
   "executeobjectprogram",
	EV_DEFAULT,
	"f",
	"time",
	"executes this objects program at the specified time"
	);

Event EV_ProjectileAtk
	(
	"projectileattack",
	EV_DEFAULT,
	"sS",
	"projectile_name tag_name",
	"Launches a projectile"
	);
Event EV_ProjectileAttackPoint
	(
	"projectileattackpoint",
	EV_DEFAULT,
	"svFF",
	"projectileName targetPosition trajectoryAngle lifespan",
	"Launches a projectile at the named entity"
	);
Event EV_ProjectileAttackEntity
	(
	"projectileattackentity",
	EV_DEFAULT,
	"ssFF",
	"projectileName entityName trajectoryAngle lifespan",
	"Launches a projectile at the named entity"
	);
Event EV_ProjectileAttackFromTag
	(
	"projectileattackfromtag",
	EV_DEFAULT,
	"ssFF",
	"projectileName tagName speed lifespan",
	"Launches a projectile from the named tag"
	);
Event EV_ProjectileAttackFromPoint
	(
	"projectileattackfrompoint",
	EV_DEFAULT,
	"svvFF",
	"projectileName position direction speed lifespan",
	"Launches a projectile from the desired location"
	);
Event EV_TraceAtk
   (
   "traceattack",
	EV_DEFAULT,
	"ffSFS",
	"damage range means_of_death knockback tag_name",
	"Does a trace attack"
	);

Event EV_Contents
(
	"contents",
	EV_DEFAULT,
	"sSSSSS",
	"ct1 ct2 ct3 ct4 ct5 ct6",
	"The content type of the entity"
);
Event EV_Mask
(
	"mask",
	EV_DEFAULT,
	"sSSSSS",
	"mask1 mask2 mask3 mask4 mask5 mask6",
	"Sets the mask of the entity.  Masks can either be real masks or contents.  A +before the"
	"mask adds it, a - removes it, and nothing sets it"
);

Event EV_DisplayEffect
	(
	"displayeffect",
	EV_DEFAULT,
	"sS",
	"effect_type effect_name",
	"Displays the named effect."
	);

Event EV_ForceAlpha
	(
	"forcealpha",
	EV_DEFAULT,
	"B",
	"bool",
	"Sets whether or not alpha is forced"
	);

Event EV_SpawnEffect
	(
	"spawneffect",
	EV_DEFAULT,
	"ssF",
	"modelName tagName removeTime",
	"Spawns the effect at the specified tag"
	);

Event EV_CreateEarthquake
   (
   "earthquake",
   EV_DEFAULT,
   "ffF",
   "magnitude duration distance",
   "Creates an earthquake"
   );

Event EV_SetFloatVar
	( 
	"setfloatvar",
	EV_DEFAULT,
	"sf",
	"variable_name float_value",
	"Sets a level, game, or entity variable."
	);
Event EV_SetVectorVar
	( 
	"setvectorvar",
	EV_DEFAULT,
	"sv",
	"variable_name vector_value",
	"Sets a level, game, or entity variable."
	);
Event EV_SetStringVar
	( 
	"setstringvar",
	EV_DEFAULT,
	"ss",
	"variable_name string_value",
	"Sets a level, game, or entity variable."
	);
Event EV_DoesVarExist
	( 
	"doesVarExist",
	EV_SCRIPTONLY,
	"@fs",
	"float_value variable_name",
	"Returns whether or not a variable exists."
	);
Event EV_GetFloatVar
	( 
	"getfloatvar",
	EV_SCRIPTONLY,
	"@fs",
	"float_value variable_name",
	"Gets a level, game, or entity variable."
	);
Event EV_RemoveVariable
	(
	"removevar",
	EV_SCRIPTONLY,
	"s",
	"variable_name",
	"Removes a level, game, or entity variable and frees any memory used by it"
	);
Event EV_GetVectorVar
	( 
	"getvectorvar",
	EV_SCRIPTONLY,
	"@vs",
	"vector_value variable_name",
	"Gets a level, game, or entity variable."
	);
Event EV_GetStringVar
	( 
	"getstringvar",
	EV_SCRIPTONLY,
	"@ss",
	"string_value variable_name",
	"Gets a level, game, or entity variable."
	);

Event EV_SetUserVar1
	( 
	"uservar1",
	EV_DEFAULT,
	"s",
	"string_value",
	"Sets an entity variable."
	);
Event EV_SetUserVar2
	( 
	"uservar2",
	EV_DEFAULT,
	"s",
	"string_value",
	"Sets an entity variable."
	);
Event EV_SetUserVar3
	( 
	"uservar3",
	EV_DEFAULT,
	"s",
	"string_value",
	"Sets an entity variable."
	);
Event EV_SetUserVar4
	( 
	"uservar4",
	EV_DEFAULT,
	"s",
	"string_value",
	"Sets an entity variable."
	);

Event EV_AffectingViewMode
	( 
	"viewmode",
	EV_DEFAULT,
	"s",
	"viewModeName",
	"Specifies that this entity uses the specified view mode."
	);

// These are events that may come from tiki files
Event EV_TikiNote
	( 
	"note",
	EV_DEFAULT,
	"s",
	"note",
	"This is a comment"
	);
Event EV_TikiTodo
	( 
	"todo",
	EV_DEFAULT,
	"s",
	"todo",
	"This is an item that needs to be done"
	);

Event EV_SetGroupID
	(
	"setgroupid",
	EV_DEFAULT,
	"i",
	"groupID",
	"Sets the groupID of this entity"
	);

Event EV_Multiplayer
	(
	"multiplayer",
	EV_DEFAULT,
	"sSSSSS",
	"realEventName parm1 parm2 parm3 parm4 parm5",
	"Sends off a real event only if this is a multiplayer game"
	);

Event EV_DamageModifier
	(
	"damagemodifier",
	EV_DEFAULT,
	"ssfFFF",
	"modifiertype value multiplier chance minpain maxpain",
	"Add a damage modifier to this entities list\n"
	"  modifiertypes are:\n"
	"  tikiname, name, group, actortype, targetname, damagetype"
	);

Event EV_SetMoveType
	(
	"setmovetype",
	EV_DEFAULT,
	"s",
	"movetype",
	"Sets the move type of this entity\n"
	" Valid types are: \n"
	" none , stationary , noclip , push , stop\n"
	" walk , step , fly , toss , flymissile\n"
	" bounce, slider, rope , gib , vehicle"
	);

Event EV_HelperNodeCommand
	(
	"helpernodecommand",
	EV_CODEONLY,
	"s",
	"commandtype",
	"Command from a helper node"
	);

Event EV_UseDataAnim
	(
	"useanim",
	EV_DEFAULT,
	"s",
	"animname",
	"Animation for the player to play when this entity is used."
	);

Event EV_UseDataType
	(
	"usetype",
	EV_DEFAULT,
	"s",
	"usetype",
	"Use type (widget name) for the use icon"
	);

Event EV_UseDataThread
	(
	"usethread",
	EV_DEFAULT,
	"s",
	"threadname",
	"Thread to call when this entity is used."
	);

Event EV_UseData
	(
	"usedata",
	EV_DEFAULT,
	"sss",
	"animname usetype threadname",
	"Sets data for this usuable entity."
	);

Event EV_UseMaxDist
	(
	"usemaxdist",
	EV_DEFAULT,
	"f",
	"maxdist",
	"Sets maximum distance this entity can be used."
	);

Event EV_UseCount
	(
	"usecount",
	EV_DEFAULT,
	"i",
	"count",
	"Sets the number of times this entity can be used."
	);

Event EV_SetArchetype
	(
	"archetype",
	EV_DEFAULT,
	"s",
	"archetype",
	"Sets the archetype name for this entity"
	);

Event EV_SetMissionObjective
	(
	"missionobjective",
	EV_DEFAULT,
	"b",
	"missionobjective",
	"Sets the mission objective flag"
	);

Event EV_SetGameplayHealth
	(
	"gdb_sethealth",
	EV_DEFAULT,
	"s",
	"healthstr",
	"Sets the gameplay version of health with keywords."
	);

Event EV_SetGameplayDamage
	(
	"gdb_setdamage",
	EV_DEFAULT,
	"s",
	"damagestr",
	"Sets the gameplay version of damage with keywords."
	);

Event EV_ProcessGameplayData
	(
	"processgameplaydata",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Causes any subclass of entity to process any specific gameplay related data."
	);

Event EV_GetVelocity
	(
	"getvelocity",
	EV_SCRIPTONLY,
	"@v",
	"NULL",
	"Returns the Velocity"
	);

Event EV_SetVelocity
	(
	"setvelocity",
	EV_SCRIPTONLY,
	"v",
	"velocity",
	"Sets the Velocity"
	);
Event EV_WatchOffset
	(
	"watchoffset",
	EV_DEFAULT,
	"v",
	"offset",
	"Sets the entity's watch offset."
	);

Event EV_StartStasis
(
	"startStasis",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the entity go into stasis mode"
);
Event EV_StopStasis
(
	"stopStasis",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the entity stop its stasis mode"
);

Event EV_SetTargetPos
(
	"settargetposition",
	EV_DEFAULT,
	"s",
	"targetbone",
	"Sets the Target Position Bone"
);

Event EV_AddHealthOverTime
	(
	"addHealthOverTime",
	EV_DEFAULT,
	"ff",
	"healthToAdd timeToAdd",
	"Specifies how much and howlong to add health."
	);
Event EV_SimplePlayDialog
	(
	"simplePlayDialog",
	EV_DEFAULT,
	"sFF",
	"sound_file volume min_dist",
	"Plays a dialog without all the special features the actors' have."
	);
Event EV_Warp
(
	"warp",
	EV_CHEAT,
	"v",
	"position",
	"Warps the entity to the specified position."
);
Event EV_TraceHitsEntity
(
	"traceHitsEntity",
	EV_SCRIPTONLY,
	"sfe",
	"tagName length entityToCheck",
	"Does a trace to check to see if it hits this entity\n."
	"Use this very rarely or a programmer will kill you!"
);

Event EV_SetCustomShader
(
	"setcustomshader",
	EV_DEFAULT,
	"s",
	"shader_name",
	"Sets Custom Shader FX on Entity"
);
Event EV_ClearCustomShader
(
	"clearcustomshader",
	EV_DEFAULT,
	"s",
	"shader_name",
	"Clears Custom Shader FX on Entity"
);
Event EV_SetCustomEmitter
(
	"setCustomEmitter",
	EV_DEFAULT,
	"s",
	"emitterName",
	"Sets up a custom emitter for this entity."
);
Event EV_ClearCustomEmitter
(
	"clearCustomEmitter",
	EV_DEFAULT,
	"s",
	"emitterName",
	"Clears the custom emitter for this Entity"
);

Event EV_IsWithinDistanceOf
(
	"iswithindistanceof",
	EV_DEFAULT,
	"@fef",
	"returnvalue targetEntity distance",
	"returns 1.0 if this entity is within the specified distance of the target entity return 0.0 if it is not"
);
Event EV_NetworkDetail
(
	"networkDetail",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Sets this entity as detail that doesn't get sent across the network of set as low bandwidth by the client"
);

CLASS_DECLARATION( Listener, Entity, NULL )
	{
		{ &EV_DamageModifier,				&Entity::AddDamageModifier },
		{ &EV_TikiNote,				        &Entity::TikiNote },
		{ &EV_TikiTodo,				        &Entity::TikiTodo },
		{ &EV_Damage,				        &Entity::DamageEvent },
		{ &EV_DamageType,			        &Entity::DamageType },
		{ &EV_Kill,							&Entity::Kill },
		{ &EV_FadeNoRemove,  				&Entity::FadeNoRemove },
		{ &EV_FadeOut,				        &Entity::FadeOut },
		{ &EV_FadeIn,  						&Entity::FadeIn },
		{ &EV_Fade,				            &Entity::Fade },
		{ &EV_Hide,					        &Entity::EventHideModel },
		{ &EV_Show,					        &Entity::EventShowModel },
		{ &EV_BecomeSolid,					&Entity::BecomeSolid },
		{ &EV_BecomeNonSolid,				&Entity::BecomeNonSolid },
		{ &EV_Ghost,						&Entity::Ghost },
		{ &EV_TouchTriggers,	            &Entity::TouchTriggersEvent },
		{ &EV_Sound,				        &Entity::Sound },
		{ &EV_StopSound,			        &Entity::StopSound },
		{ &EV_SetHealth,			        &Entity::SetHealth },
		{ &EV_GetHealth,			        &Entity::GetHealth },
		{ &EV_SetMaxHealth,					&Entity::SetMaxHealth	},
		{ &EV_SetSize,				        &Entity::SetSize },
		{ &EV_SetMins,				        &Entity::SetMins },
		{ &EV_SetMaxs,				        &Entity::SetMaxs },
		{ &EV_GetMins,						&Entity::GetMins },
		{ &EV_GetMaxs,						&Entity::GetMaxs },
		{ &EV_SetScale,						&Entity::SetScale },
		{ &EV_SetRandomScale,				&Entity::setRandomScale },
		{ &EV_SetAlpha,						&Entity::SetAlpha },
		{ &EV_SetOrigin,			        &Entity::SetOrigin },
		{ &EV_GetOrigin,			        &Entity::GetOrigin },
		{ &EV_SetTargetName,		        &Entity::SetTargetName },
		{ &EV_GetTargetName,		        &Entity::GetTargetName },
		{ &EV_GetRawTargetName,				&Entity::GetRawTargetName },
		{ &EV_SetTarget,			        &Entity::SetTarget },
		{ &EV_GetTarget,					&Entity::getTarget },
		{ &EV_GetTargetEntity,				&Entity::GetTargetEntity	},
		{ &EV_SetKillTarget,		        &Entity::SetKillTarget },
		{ &EV_GetModelName,					&Entity::GetModelName	},
		{ &EV_SetAngles,			        &Entity::SetAngles },
		{&EV_GetAngles,						&Entity::GetAngles },
		{ &EV_SetAngle,						&Entity::SetAngleEvent },
		{ &EV_SetMass,						&Entity::SetMassEvent },
		{ &EV_SetFullTrace,					&Entity::SetFullTraceEvent },
		{ &EV_RegisterAlias,	            &Entity::RegisterAlias },
		{ &EV_RegisterAliasAndCache,		&Entity::RegisterAliasAndCache },
		{ &EV_Cache,						&Entity::Cache },
		{ &EV_AutoCache,					&Entity::Cache },
		{ &EV_LoopSound,					&Entity::LoopSound },
		{ &EV_StopLoopSound,				&Entity::StopLoopSound },
		{ &EV_Model,				        &Entity::SetModelEvent },
		{ &EV_SetLight,						&Entity::SetLight },
		{ &EV_LightOn,				        &Entity::LightOn },
		{ &EV_LightOff,						&Entity::LightOff },
		{ &EV_LightRed,						&Entity::LightRed },
		{ &EV_LightGreen,		            &Entity::LightGreen },
		{ &EV_LightBlue,		            &Entity::LightBlue },
		{ &EV_LightRadius,					&Entity::LightRadius },
		{ &EV_LightStyle,					&Entity::LightStyle },
		{ &EV_EntityFlags,					&Entity::Flags },
		{ &EV_EntityEffects,				&Entity::Effects },
		{ &EV_EntitySVFlags,	            &Entity::SVFlags },
		{ &EV_EntityRenderEffects,			&Entity::RenderEffects },
		{ &EV_BroadcastSound,				&Entity::BroadcastSound },
		{ &EV_SurfaceModelEvent,			&Entity::SurfaceModelEvent },
		{ &EV_ProcessInitCommands,			&Entity::ProcessInitCommandsEvent },
		{ &EV_Attach,						&Entity::AttachEvent },
		{ &EV_AttachModel,					&Entity::AttachModelEvent },
		{ &EV_RemoveAttachedModel,			&Entity::RemoveAttachedModelEvent },
		{ &EV_RemoveAttachedModelByTargetname,			&Entity::removeAttachedModelByTargetname },
		{ &EV_Detach,						&Entity::DetachEvent },
		{ &EV_IncreaseShotCount,			&Entity::IncreaseShotCount		},
		{ &EV_TakeDamage,					&Entity::TakeDamageEvent },
		{ &EV_NoDamage,						&Entity::NoDamageEvent },
		{ &EV_Gravity,						&Entity::Gravity },
		{ &EV_UseBoundingBox,				&Entity::UseBoundingBoxEvent },
		{ &EV_Hurt,							&Entity::HurtEvent },
		{ &EV_IfSkill,						&Entity::IfSkillEvent },
		{ &EV_Classname,					&Entity::ClassnameEvent },
		{ &EV_SpawnFlags,					&Entity::SpawnFlagsEvent },
		{ &EV_SetTeam,						&Entity::SetTeamEvent },
		{ &EV_Trigger,						&Entity::TriggerEvent },
		{ &EV_Censor,						&Entity::Censor },
		{ &EV_Stationary,					&Entity::StationaryEvent },
		{ &EV_Explosion,					&Entity::Explosion },
		{ &EV_ShaderEvent,					&Entity::Shader },
		{ &EV_ScriptShaderEvent,			&Entity::Shader },
		{ &EV_KillAttach,					&Entity::KillAttach },
		{ &EV_DropToFloor,					&Entity::DropToFloorEvent },
		{ &EV_Bind,				         	&Entity::BindEvent },
		{ &EV_Unbind,				        &Entity::EventUnbind },
		{ &EV_JoinTeam,         			&Entity::JoinTeam },
		{ &EV_QuitTeam,						&Entity::EventQuitTeam },
		{ &EV_AddToSoundManager,			&Entity::AddToSoundManager },
		{ &EV_SetControllerAngles,			&Entity::SetControllerAngles },
		{ &EV_DeathSinkStart,				&Entity::DeathSinkStart },
		{ &EV_DeathSink,					&Entity::DeathSink },
		{ &EV_LookAtMe,						&Entity::LookAtMe },
		{ &EV_ProjectilesCanStickToMe,		&Entity::ProjectilesCanStickToMe },

		{ &EV_DetachAllChildren,			&Entity::DetachAllChildren },
		{ &EV_Morph,						&Entity::MorphEvent },
		{ &EV_Unmorph,						&Entity::UnmorphEvent },
		{ &EV_MorphControl,					&Entity::MorphControl },
		{ &EV_SetCinematicAnim,				&Entity::SetCinematicAnim },
		{ &EV_CinematicAnimDone,			&Entity::CinematicAnimDone },
		{ &EV_SetAnimOnAttachedModel,		&Entity::SetAnimOnAttachedModel		},
		{ &EV_SetEntityExplosionModel,		&Entity::SetEntityExplosionModel		},
		{ &EV_SetObjectProgram,				&Entity::SetObjectProgram		},		
		{ &EV_ExecuteObjectProgram,			&Entity::ExecuteProgram },
		{ &EV_DoRadiusDamage,				&Entity::DoRadiusDamage },
		{ &EV_SelfDetonate,					&Entity::SelfDetonate		},

		{ &EV_Anim,							&Entity::PassToAnimate },
		{ &EV_SetFrame,						&Entity::PassToAnimate },
		{ &EV_StopAnimating,				&Entity::PassToAnimate },
		{ &EV_Torso_StopAnimating,			&Entity::PassToAnimate },
		{ &EV_NewAnim,						&Entity::PassToAnimate },

		{ &EV_ProjectileAtk,				&Entity::ProjectileAtk },
		{ &EV_ProjectileAttackPoint,		&Entity::ProjectileAttackPoint },
		{ &EV_ProjectileAttackEntity,		&Entity::ProjectileAttackEntity },
		{ &EV_ProjectileAttackFromTag,		&Entity::ProjectileAttackFromTag },
		{ &EV_ProjectileAttackFromPoint,	&Entity::ProjectileAttackFromPoint },

		{ &EV_TraceAtk,						&Entity::TraceAtk },
		{ &EV_Contents,						&Entity::Contents },
		{ &EV_Mask,							&Entity::setMask },

		{ &EV_DisplayEffect,				&Entity::DisplayEffect },

		{ &EV_ForceAlpha,					&Entity::ForceAlpha },
		{ &EV_SpawnEffect,					&Entity::SpawnEffect },

		{ &EV_CreateEarthquake,				&Entity::CreateEarthquake },

		{ &EV_SetFloatVar,					&Entity::SetFloatVar },
		{ &EV_SetVectorVar,					&Entity::SetVectorVar },
		{ &EV_SetStringVar,					&Entity::SetStringVar },

		{ &EV_DoesVarExist,					&Entity::doesVarExist },
		{ &EV_RemoveVariable,				&Entity::RemoveVariable },

		{ &EV_GetFloatVar,					&Entity::GetFloatVar },
		{ &EV_GetVectorVar,					&Entity::GetVectorVar },
		{ &EV_GetStringVar,					&Entity::GetStringVar },

		{ &EV_SetUserVar1,					&Entity::SetUserVar1 },
		{ &EV_SetUserVar2,					&Entity::SetUserVar2 },
		{ &EV_SetUserVar3,					&Entity::SetUserVar3 },
		{ &EV_SetUserVar4,					&Entity::SetUserVar4 },

		{ &EV_AffectingViewMode,			&Entity::affectingViewMode },
		{ &EV_SetGroupID,                   &Entity::SetGroupID	},

		{ &EV_Multiplayer,                  &Entity::MultiplayerEvent	},
		{ &EV_SetMoveType,					&Entity::setMoveType			},

		{ &EV_UseDataAnim,					&Entity::useDataAnim			},
		{ &EV_UseDataType,					&Entity::useDataType			},
		{ &EV_UseDataThread,				&Entity::useDataThread		},
		{ &EV_UseData,						&Entity::useDataEvent		},
		{ &EV_UseMaxDist,					&Entity::useDataMaxDist		},
		{ &EV_UseCount,						&Entity::useDataCount		},

		{ &EV_SetArchetype,					&Entity::setArchetype		},
		{ &EV_SetMissionObjective,			&Entity::setMissionObjective },
		{ &EV_SetGameplayHealth,			&Entity::setGameplayHealth   },
		{ &EV_GetVelocity,					&Entity::GetVelocity		},
		{ &EV_SetVelocity,					&Entity::SetVelocity		},
		{ &EV_WatchOffset,					&Entity::SetWatchOffset	},

		{ &EV_StartStasis,					&Entity::startStasis },
		{ &EV_StopStasis,					&Entity::stopStasis },

		{ &EV_SetTargetPos,					&Entity::setTargetPos	},

		{ &EV_AddHealthOverTime,			&Entity::addHealthOverTime },
		{ &EV_SimplePlayDialog,				&Entity::simplePlayDialog },

		{ &EV_Warp,							&Entity::warp },
		{ &EV_TraceHitsEntity,				&Entity::traceHitsEntity },
		{ &EV_SetOriginEveryFrame,			&Entity::setOriginEveryFrame },		

		{ &EV_SetCustomShader,				&Entity::setCustomShader	},
		{ &EV_ClearCustomShader,			&Entity::clearCustomShader },

		{ &EV_SetCustomEmitter,				&Entity::setCustomEmitter },
		{ &EV_ClearCustomEmitter,			&Entity::clearCustomEmitter },
		{ &EV_IsWithinDistanceOf,			&Entity::isWithinDistanceOf },

		{ &EV_NetworkDetail,				&Entity::setNetworkDetail },
	
		{ NULL, NULL }
	};

Entity::Entity()
	{
	Setup();
	}

Entity::Entity( int create_flag )
	{
	Setup();

	if ( create_flag & ENTITY_CREATE_FLAG_ANIMATE )
		animate = new Animate( this );

	if ( create_flag & ENTITY_CREATE_FLAG_MOVER )
		mover = new Mover( this );
	}

void Entity::Setup()
	{
	// Pluggable modules

	animate    = NULL;
	mover      = NULL;
	bind_info  = NULL;
	morph_info = NULL;

   edict = level.AllocEdict( this );
	client = edict->client;
	entnum = edict->s.number;
	edict->s.clientNum = ENTITYNUM_NONE ;

   // spawning variables
	spawnflags = level.spawnflags;
   level.spawnflags = 0;

	// rendering variables
	setAlpha( 1.0f );
	setScale( 1.0f );

	// physics variables
   total_delta             = Vector(0, 0, 0);
	mass							= 0;
	gravity						= 1.0;
	groundentity				= NULL;
   groundcontents          = 0;
	velocity						= vec_zero;
	avelocity					= vec_zero;
   edict->clipmask         = MASK_SOLID;

   // bind variables
	edict->s.bindparent = ENTITYNUM_NONE;

   // this is an generic entity
   edict->s.eType = ET_GENERAL;

   setContents( 0 );

   edict->s.parent      = ENTITYNUM_NONE;
   edict->s.pos.trType  = TR_LERP;
   edict->ownerNum      = ENTITYNUM_NONE;

	setOrigin( vec_zero );
   origin.copyTo( edict->s.origin2 );

   setAngles( vec_zero );

	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_NOT );

	// Character state
	health		= 0;
	max_health	= 0;
	deadflag		= DEAD_NO;
	flags			= 0;

	// underwater variables
	watertype		= 0;
	waterlevel		= 0;

	// Pain and damage variables
	takedamage				= DAMAGE_NO;
   damage_type          = -1;

   // Surface variables
   numsurfaces    = 0;

   // Light variables
   lightRadius    = 0;

	look_at_me = false;
	projectilesCanStickToMe = true;

	explosionModel = "";

	_affectingViewModes = 0;

	edict->s.infoIcon = 0;

	addAffectingViewModes( gi.GetViewModeClassMask( "entity" ) );
	_groupID = 0;

	damageModSystem = 0;

	_fulltrace = false ;
	useData = 0;
	setTargetPos( "" );

	ObjectProgram = NULL;

	_missionObjective = false;

	_networkDetail = false;
	}

Entity::~Entity()
{
	Container<Entity *> bindlist;
	Entity *ent;
	int num;
	int i;
	
	if ( bind_info )
	{
		// unbind any entities that are bound to me
		// can't unbind within this loop, so make an array
		// and unbind them outside of it.
		num = 0;
		
		for( ent = bind_info->teamchain; ent; ent = ent->bind_info->teamchain )
		{
			if ( ent->bind_info->bindmaster == this )
			{
				bindlist.AddObject( ent );
			}
		}
		
		num = bindlist.NumObjects();
		for( i = 1; i <= num; i++ )
		{
			bindlist.ObjectAt( i )->unbind();
		}
		
		bindlist.FreeObjectList();
		
		unbind();
		quitTeam();
		
		detach();
		
		//
		// go through and set our children
		//
		num = bind_info->numchildren;
		for( i = 0; ( i < MAX_MODEL_CHILDREN ) && num; i++ )
		{
			if ( bind_info->children[ i ] == ENTITYNUM_NONE )
			{
				continue;
			}
			ent = G_GetEntity( bind_info->children[ i ] );
			if ( ent )
			{
				ent->PostEvent( EV_Remove, 0.0f );
			}
			num--;
		}
	}
	
	if ( targetname.length() && world )
	{
		world->RemoveTargetEntity( targetname, this );
	}
	
	level.FreeEdict( edict );
	
	// Pluggable modules
	
	if ( animate )
		delete animate;
	
	if ( mover )
		delete mover;
	
	if ( bind_info )
		delete bind_info;
	
	if ( morph_info )
		delete morph_info;
	
	entityVars.ClearList();
	
	if ( damageModSystem )
		delete damageModSystem;
	
	if ( useData )
	{
		delete useData;
		useData = NULL;
	}
	
	// Note, the ObjectProgram deletion is handled elsewhere

	// Make sure to remove our selves from the extra list if we are a mission objective

	if ( edict->s.missionObjective )
	{
		G_RemoveEntityFromExtraList(entnum);
	}
}

Entity * Entity::FindEntityByName( const str &entityName )
{
	for ( int i = 0; i < MAX_GENTITIES; i++ )
	{
		if ( g_entities[i].inuse && g_entities[i].entity && ( entityName == g_entities[ i ].entname ) )
		{
			return g_entities[i].entity;
		}
	}
	
	return NULL;
}

void Entity::SetEntNum
   (
   int num
   )

   {
   if ( edict )
      {
      level.FreeEdict( edict );
      }

   level.spawn_entnum = num;
   level.AllocEdict( this );
	client = edict->client;
	entnum = edict->s.number;
   }

void Entity::ClassnameEvent
	(
	Event *ev
   )

   {
	strncpy( edict->entname, ev->GetString( 1 ), sizeof( edict->entname ) - 1 );
	}

void Entity::SpawnFlagsEvent
	(
	Event *ev
   )

   {
	// spawning variables
	spawnflags = ev->GetInteger( 1 );
   if ( spawnflags & SPAWNFLAG_DETAIL )
		{
      edict->s.renderfx |= RF_DETAIL;
		}
	}

void Entity::SetTarget
	(
	const char *text
	)

	{
   if ( text )
		{
      target = text;
		}
	else
		{
		target = "";
		}
	}

void Entity::SetTargetName
	(
	const char *text
	)

	{
   if ( targetname.length() && world )
      {
      world->RemoveTargetEntity( targetname, this );
      }

   if ( text )
		{
      if ( text[ 0 ] == '$' )
         text++;
      targetname = text;
		}
	else
		{
		targetname = "";
		}

   if ( targetname.length() && world )
		{
      //
      // make sure we don't re-targetname the world entity
      //
      if (
            ( this != world ) ||
            ( targetname == str( "world" ) )
         )
         {
         world->AddTargetEntity( targetname, this );
         }
      else
         {
			error( "SetTargetName", "World was re-targeted with targetname %s\n", targetname.c_str() );

         // this is bad
         //assert( 0 );
         //gi.WDPrintf( "world was re-targeted with targetname %s\n", targetname.c_str() );
         //targetname = "world";
         }

		}
	}

void Entity::SetKillTarget
	(
	const char *text
	)

	{
   if ( text )
		{
      killtarget = text;
		}
	else
		{
		killtarget = "";
		}
	}

void Entity::setModel( const char *mdl )
{
	str temp;
	
	if ( LoadingSavegame && ( this == world ) )
	{
		// don't set model on the world
		return;
	}
	
	if ( !mdl )
	{
		mdl = "";
	}
	
	// Prepend 'models/' to make things easier
	temp = "";
	if ( ( strlen( mdl ) > 0 ) && !strchr( mdl, '*' ) && strnicmp( "models/", mdl, 7 ) && !strstr( mdl, ".spr" ) )
	{
		temp = "models/";
	}
	temp += mdl;
	
	// we use a temp string so that if model was passed into here, we don't
	// accidentally free up the string that we're using in the process.
	model = temp;
	
	gi.setmodel( edict, model.c_str() );
	
	if ( gi.IsModel( edict->s.modelindex ) )
	{
		Event *ev;
		
		numsurfaces = gi.NumSurfaces( edict->s.modelindex );
		
		if ( !LoadingSavegame )
		{
			CancelEventsOfType( EV_ProcessInitCommands );
			
			ev = new Event( EV_ProcessInitCommands );
			ev->AddInteger( edict->s.modelindex );
			PostEvent( ev, EV_PROCESS_INIT );
		}
		else
		{
			ProcessInitCommands( edict->s.modelindex, true );
		}
	}
	else if ( strstr( mdl, ".spr" ) )
	{
		edict->s.eType = ET_SPRITE;
	}
	
	// Sanity check to see if we're expecting a B-Model
	assert( !( ( edict->solid == SOLID_BSP ) && !edict->s.modelindex ) );
	if ( ( edict->solid == SOLID_BSP ) && !edict->s.modelindex )
	{
		const char *name;
		
		name = getClassID();
		if ( !name )
		{
			name = getClassname();
		}
		gi.WDPrintf( "%s with SOLID_BSP and no model - '%s'(%d)\n", name, targetname.c_str(), entnum );
		
		// Make it non-solid so that the collision code doesn't kick us out.
		setSolidType( SOLID_NOT );
	}
	
	mins = edict->mins;
	maxs = edict->maxs;
	size = maxs - mins;
	edict->radius = size.length() * 0.5f;
	edict->radius2 = edict->radius * edict->radius;
	
	//
	// see if we have a mins and maxs set for this model
	//
	
	if ( gi.IsModel( edict->s.modelindex ) && !mins.length() && !maxs.length() )
	{
		vec3_t tempmins, tempmaxs;
		int animNum;

		animNum = gi.Anim_NumForName( edict->s.modelindex, "idle" );

		if ( animNum >= 0 )
		{
			gi.Frame_Bounds( edict->s.modelindex, animNum, 0, edict->s.scale, tempmins, tempmaxs );
			setSize( tempmins, tempmaxs );
		}

		//vec3_t tempmins, tempmaxs;
		//gi.CalculateBounds( edict->s.modelindex, edict->s.scale, tempmins, tempmaxs );
		//setSize( tempmins, tempmaxs );
	}

	if ( this->isSubclassOf(Player) )
		{
		//If we're a player, we need to reset the statemachine
		Player* player = (Player*)this;

		if ( player )
			{
			player->SetAnim( "stand_idle", legs, true );
			player->SetAnim( "stand_idle", torso, true );
			player->LoadStateTable();
			}
		}
}

// Added to set the weapon view model
void Entity::setViewModel
	(
	const char *mdl
	)

	{
   str temp;

   if ( LoadingSavegame && ( this == world ) )
      {
      // don't set model on the world
      return;
      }

	if ( !mdl )
		{
		mdl = "";
		}

   // Prepend 'models/' to make things easier
   temp = "";
	if ( ( strlen( mdl ) > 0 ) && !strchr( mdl, '*' ) && strnicmp( "models/", mdl, 7 ) && !strstr( mdl, ".spr" ) )
		{
		temp = "models/";
		}
   temp += mdl;

   // we use a temp string so that if model was passed into here, we don't
   // accidentally free up the string that we're using in the process.
   model = temp;

	gi.setviewmodel( edict, model.c_str() );

   if ( gi.IsModel( edict->s.viewmodelindex ) )
      {
  	   Event *ev;

      numsurfaces = gi.NumSurfaces( edict->s.viewmodelindex );

      if ( !LoadingSavegame )
         {
		   CancelEventsOfType( EV_ProcessInitCommands );

         ev = new Event( EV_ProcessInitCommands );
         ev->AddInteger( edict->s.viewmodelindex );
         PostEvent( ev, EV_PROCESS_INIT );
         }
      else
         {
         ProcessInitCommands( edict->s.viewmodelindex, true );
         }
      }
   else if ( strstr( mdl, ".spr" ) )
      {
      edict->s.eType = ET_SPRITE;
      }

	// Sanity check to see if we're expecting a B-Model
	assert( !( ( edict->solid == SOLID_BSP ) && !edict->s.viewmodelindex ) );
	if ( ( edict->solid == SOLID_BSP ) && !edict->s.viewmodelindex )
		{
      const char *name;

      name = getClassID();
      if ( !name )
         {
         name = getClassname();
         }
      gi.WDPrintf( "%s with SOLID_BSP and no model - '%s'(%d)\n", name, targetname.c_str(), entnum );

      // Make it non-solid so that the collision code doesn't kick us out.
      setSolidType( SOLID_NOT );
      }

	mins = edict->mins;
	maxs = edict->maxs;
   size = maxs - mins;
   edict->radius = size.length() * 0.5f;
	edict->radius2 = edict->radius * edict->radius;

   //
   // see if we have a mins and maxs set for this model
   //
   //FIXME
   //We only did this on startup, but with the spawnargs as events it would have to
   //be here.  Do we still need this?  It may cause strange effects.
   /* if ( gi.IsModel( edict->s.viewmodelindex ) && !mins.length() && !maxs.length() )
      {
      vec3_t tempmins, tempmaxs;
      gi.CalculateBounds( edict->s.viewmodelindex, edict->s.scale, tempmins, tempmaxs );
      setSize( tempmins, tempmaxs );
      } */
	}

void Entity::ProcessInitCommands
	(
	int index,
   qboolean cache
	)

	{
   tiki_cmd_t cmds;

   if ( LoadingSavegame && !cache )
      {
      // Don't process init commands when loading a savegame since
      // it will cause items to be added to inventories unnecessarily.
      // All variables affected by the init commands will be set
      // by the unarchive functions.
      //
      // we do want to process the cache commands though regardless
      return;
      }

   if ( gi.InitCommands( index, &cmds ) )
      {
      int i, j, savedindex;
   	Event		*event;

      // because the model has not necessarily been spawned yet, we need to set
      // this entity to have this index so that precaches go where they are supposed
      // to, this should have no bad effects, since we are only doing it in the
      // cache phase of spawning
      if ( index != edict->s.modelindex )
         {
         savedindex = edict->s.modelindex;
         edict->s.modelindex = index;
         }
      else
         {
         savedindex = -1;
         }
      for( i = 0; i < cmds.num_cmds; i++ )
         {
      	event = new Event( cmds.cmds[ i ].args[ 0 ] );
         if ( !cache || ( event->GetFlags() & EV_CACHE ) )
            {
            for( j = 1; j < cmds.cmds[ i ].num_args; j++ )
               {
      		   event->AddToken( cmds.cmds[ i ].args[ j ] );
               }
            ProcessEvent( event );
            }
         else
            {
            delete event;
            }
         }
      // restore the modelindex, see above
      if ( savedindex != -1 )
         {
         edict->s.modelindex = savedindex;
         }
      }
	}

void Entity::ProcessInitCommandsEvent
	(
	Event *ev
	)

	{
   int index;

   index = ev->GetInteger( 1 );
   ProcessInitCommands( index, false );
	}

void Entity::EventHideModel
	(
	Event *ev
	)

	{
	hideModel();
	}

void Entity::EventShowModel
	(
	Event *ev
	)

	{
	showModel();
	}

void Entity::SetTeamEvent
	(
	Event *ev
	)

	{
	if ( !bind_info )
		bind_info = CreateBindInfo();

   bind_info->moveteam = ev->GetString( 1 );
	}

void Entity::TriggerEvent
	(
	Event *ev
	)

	{
	const char	*name;
	Event		   *event;
	Entity	   *ent;
   TargetList  *tlist;
   int         i;
   int         num;

	name = ev->GetString( 1 );

	if ( !name )
		return;

	// Check for object commands
	if ( name[ 0 ] == '$' )
		{
      tlist = world->GetTargetList( str( name + 1 ) );
      num = tlist->list.NumObjects();
      for ( i = 1; i <= num; i++ )
         {
         ent = tlist->list.ObjectAt( i );

		   assert( ent );

         event = new Event( EV_Activate );
			event->SetSource( ev->GetSource() );
			event->SetThread( ev->GetThread() );
			event->SetLineNumber( ev->GetLineNumber() );
         event->AddEntity( this );
		   ent->ProcessEvent( event );
         }
      }
	else if ( name[ 0 ] == '*' )   // Check for entnum commands
      {
		if ( !IsNumeric( &name[ 1 ] ) )
			{
			ev->Error( "Expecting numeric value for * command, but found '%s'\n", &name[ 1 ] );
			}
		else
         {
         ent = G_GetEntity( atoi( &name[ 1 ] ) );
         if ( ent )
            {
            event = new Event( EV_Activate );
			   event->SetSource( ev->GetSource() );
			   event->SetThread( ev->GetThread() );
			   event->SetLineNumber( ev->GetLineNumber() );
            event->AddEntity( this );
            ent->ProcessEvent( event );
            }
         else
            {
            ev->Error( "Entity not found for * command\n" );
            }
         }
      return;
      }
	else
		{
		ev->Error( "Invalid entity reference '%s'.\n", name );
		}
	}

void Entity::setAlpha
	(
	float alpha
	)

	{
	if ( alpha > 1.0f )
		{
		alpha = 1.0f;
		}
	if ( alpha < 0.0f )
		{
		alpha = 0.0f;
		}
   edict->s.alpha = alpha;
	}

void Entity::setScale
	(
	float scale
	)

	{
   edict->s.scale = scale;
	}

void Entity::setSolidType
	(
	solid_t type
	)

	{
	if (
         ( !LoadingSavegame ) &&
         ( type == SOLID_BSP ) &&
         ( this != world ) &&
         (
            !model.length() ||
            (
               ( model[ 0 ] != '*' ) &&
               ( !strstr( model.c_str(), ".bsp" ) )
            )
         )
      )
		{
      error( "setSolidType", "SOLID_BSP entity at x%.2f y%.2f z%.2f with no BSP model", origin[ 0 ], origin[ 1 ], origin[ 2 ] );
		}
	edict->solid = type;

   //
   // set the appropriate contents type
   if ( edict->solid == SOLID_BBOX )
      {
      if ( !getContents() )
   	   setContents( CONTENTS_SOLID );
      }
   else if ( edict->solid == SOLID_NOT )
      {
      if ( getContents() == CONTENTS_SOLID )
   	   setContents( 0 );
      }
   else if ( edict->solid == SOLID_BSP )
      {
      if ( !getContents() )
   	   setContents( CONTENTS_SOLID );
      }

	link();

	edict->svflags &= ~SVF_NOCLIENT;
	if ( hidden() )
		{
		edict->svflags |= SVF_NOCLIENT;
		}
	}

void Entity::setSize
	(
	Vector min,
	Vector max
	)

	{
   Vector delta;

   if ( flags & FL_ROTATEDBOUNDS )
      {
      vec3_t tempmins, tempmaxs;

      //
      // rotate the mins and maxs for the model
      //
	   min.copyTo( tempmins );
	   max.copyTo( tempmaxs );

      CalculateRotatedBounds2( edict->s.mat, tempmins, tempmaxs );

      mins = Vector( tempmins );
      maxs = Vector( tempmaxs );
	   size = max - min;

	   mins.copyTo( edict->mins );
	   maxs.copyTo( edict->maxs );
      edict->radius = size.length() * 0.5;
		edict->radius2 = edict->radius * edict->radius;
      }
   else
      {
      if ( ( min == edict->mins ) && ( max == edict->maxs ) )
         {
         return;
         }

	   mins = min;
	   maxs = max;
	   size = max - min;

	   mins.copyTo( edict->mins );
	   maxs.copyTo( edict->maxs );

      //
      // get the full mins and maxs for this model
      //
      /* if ( gi.IsModel( edict->s.modelindex ) )
         {
         vec3_t fullmins, fullmaxs;
         Vector delta;

         gi.CalculateBounds( edict->s.modelindex, edict->s.scale, fullmins, fullmaxs );

         delta = Vector( fullmaxs ) - Vector( fullmins );
         edict->radius = delta.length() * 0.5f;
			edict->radius2 = edict->radius * edict->radius;
         }
      else */
         {
         edict->radius = size.length() * 0.5;
			edict->radius2 = edict->radius * edict->radius;
         }
      }

	link();
	}

Vector Entity::getLocalVector
	(
	const Vector &vec
	)

	{
	Vector pos;

	pos[ 0 ] = vec * orientation[ 0 ];
	pos[ 1 ] = vec * orientation[ 1 ];
	pos[ 2 ] = vec * orientation[ 2 ];

	return pos;
	}

void Entity::link
	(
	void
	)

	{
	if ( !level._cleanup )
		gi.linkentity( edict );

	absmin = edict->absmin;
	absmax = edict->absmax;
	centroid = ( absmin + absmax ) * 0.5f;
	centroid.copyTo( edict->centroid );

   // If this has a parent, then set the areanum the same
   // as the parent's
   if ( edict->s.parent != ENTITYNUM_NONE )
      {
      edict->areanum = g_entities[ edict->s.parent ].areanum;
      }
	}

void Entity::addOrigin
	(
	const Vector &add
	)

   {
   setOrigin( GetLocalOrigin() + add );
   }

void Entity::setOrigin
	(
   void
	)

   {
   setOrigin( GetLocalOrigin() );
   }

void Entity::setOrigin
	(
	const Vector &org
	)

	{
   Entity * ent;
   int i,num;

	if ( bind_info && bind_info->bindmaster )
		{
      SetLocalOrigin( org );

		if ( bind_info->bind_use_my_angles )
			MatrixTransformVector( GetLocalOrigin(), orientation, origin );
		else
			MatrixTransformVector( GetLocalOrigin(), bind_info->bindmaster->orientation, origin );

      origin += bind_info->bindmaster->origin;
      origin.copyTo( edict->s.netorigin );
		}
   // If entity has a parent, then set the origin as the
   // centroid of the parent, and set edict->s.netorigin
   // as the local origin of the entity which will be used
   // to position this entity on the client.
   else if ( edict->s.parent != ENTITYNUM_NONE )
      {
		orientation_t orient;

      VectorClear( edict->s.netorigin );
      ent = ( Entity * )G_GetEntity( edict->s.parent );

      //ent->GetTag(( edict->s.tag_num & TAG_MASK, &origin );
		ent->GetTag( edict->s.tag_num & TAG_MASK, &orient );

		MatrixTransformVector( edict->s.attach_offset, orient.axis, origin );

		//origin += edict->s.attach_offset;
		origin += orient.origin;

      SetLocalOrigin( vec_zero );
      }
	else
		{
      origin = org;
      SetLocalOrigin( org );
      origin.copyTo( edict->s.netorigin );
		}

   origin.copyTo( edict->s.origin );
   origin.copyTo( edict->currentOrigin );

	link();

#if 0
   if ( this->isClient() )
      {
      i = CurrentAnim();
      j = CurrentFrame();

      G_DrawCoordSystem( origin, orientation[0], orientation[1], orientation[2], 30 );
      gi.Printf( "%s:legs anim:%s frame %i\n", this->getClassname(), gi.Anim_NameForNum( edict->s.modelindex, i ), j );
      }
#endif

	if ( bind_info )
		{
		// Go through and set our children

		num = bind_info->numchildren;
		for( i = 0; ( i < MAX_MODEL_CHILDREN ) && num; i++ )
			{
			if ( bind_info->children[ i ] == ENTITYNUM_NONE )
				{
				continue;
				}
			ent = ( Entity * )G_GetEntity( bind_info->children[ i ] );
			if ( ent )
			{
				ent->setOrigin();
			}
			num--;
			}


		/* for( ent = bind_info->teamchain; ent != NULL; ent = ent->bind_info->teamchain )
			{
			if ( ent->bind_info->teammaster == this )
				ent->setOrigin();
			} */
		}
	}

void Entity::GetRawTag
	(
   int    tagnum,
   orientation_t * orient,
	bodypart_t part
	)

	{
   int anim;
   int frame;

   anim = CurrentAnim(part);
   frame = CurrentFrame(part);

   //If we don't have a valid animation, we can't get a tag
   if ( anim < 0 ) return;

	*orient = gi.Tag_OrientationEx( edict->s.modelindex, CurrentAnim( legs ), CurrentFrame( legs ), tagnum & TAG_MASK, 
			edict->s.scale, edict->s.bone_tag, edict->s.bone_quat, 0, 0, 1.0f, ( edict->s.anim & ANIM_BLEND ) != 0, 
			( edict->s.torso_anim & ANIM_BLEND ) != 0, CurrentAnim( torso ), CurrentFrame( torso ), 0, 0, 1.0f );
   }

qboolean Entity::GetRawTag
	(
	const char *name,
   orientation_t * orient,
	bodypart_t part
	)
	{
   int      tagnum;

   tagnum = gi.Tag_NumForName( edict->s.modelindex, name );

   if ( tagnum < 0 )
      return false;

   GetRawTag( tagnum, orient, part );
   return true;
   }

void Entity::GetTag
	(
   int    tagnum,
   orientation_t * orient
	)

	{
   orientation_t  orn;
   int            i;

   GetRawTag( tagnum, &orn );

   VectorCopy( origin, orient->origin );

   for ( i = 0 ; i < 3 ; i++ )
      {
		VectorMA( orient->origin, orn.origin[i], orientation[i], orient->origin );
	   }
   MatrixMultiply( orn.axis, orientation, orient->axis );
   }

qboolean Entity::GetTag
	(
	const char *name,
   orientation_t * orient
	)
	{
   int      tagnum;

   tagnum = gi.Tag_NumForName( edict->s.modelindex, name );

   if ( tagnum < 0 )
      return false;

   GetTag( tagnum, orient );
   return true;
   }

void Entity::GetTag
	(
   int    tagnum,
	Vector *pos,
	Vector *forward,
	Vector *left,
	Vector *up
	)

	{
   orientation_t orn;

   GetTag( tagnum, &orn);

	if ( pos )
		{
		*pos = Vector( orn.origin );
		}
	if ( forward )
		{
		*forward = Vector( orn.axis[ 0 ] );
		}
	if ( left )
		{
		*left = Vector( orn.axis[ 1 ] );
		}
	if ( up )
		{
		*up = Vector( orn.axis[ 2 ] );
		}
   }

qboolean Entity::GetTag
	(
	const char *name,
	Vector *pos,
	Vector *forward,
	Vector *left,
	Vector *up
	)

	{
   int      tagnum;

   tagnum = gi.Tag_NumForName( edict->s.modelindex, name );

   if ( tagnum < 0 )
      return false;

   GetTag( tagnum, pos, forward, left, up );
   return true;
   }

void Entity::addAngles
   (
   const Vector &add
   )

   {
   if ( bind_info && bind_info->bindmaster )
      {
      setAngles( localangles + add );
      }
   else
      {
      setAngles( angles + add );
      }
   }

void Entity::setAngles
	(
   void
   )

   {
   if ( bind_info && bind_info->bindmaster )
      {
      setAngles( localangles );
      }
   else
      {
      setAngles( angles );
      }
   }


void Entity::setAngles
	(
	const Vector &ang
	)

	{
   Entity * ent;
		
   int num,i;

	angles[ 0 ] = AngleMod( ang[ 0 ] );
	angles[ 1 ] = AngleMod( ang[ 1 ] );
	angles[ 2 ] = AngleMod( ang[ 2 ] );

   localangles = angles;
   if ( bind_info && bind_info->bindmaster )
      {
      float	mat[3][3];
		AnglesToAxis( localangles, mat );
		R_ConcatRotations( mat, bind_info->bindmaster->orientation, orientation );
      MatrixToEulerAngles( orientation, angles );
      }
   else if ( edict->s.parent != ENTITYNUM_NONE  )
		{
		Entity *parent;		
		Vector tagPos;
		Vector tagForward;
		Vector forwardAngles;

		vec3_t tempAxis[3];
		vec3_t tempAxis2[3];
		vec3_t finalAxis[3];
		vec3_t tempForward;
		vec3_t finalForward;		


		parent = ( Entity * )G_GetEntity( edict->s.parent );
		parent->GetTag( edict->s.tag_num , &tagPos , &tagForward );

		
		forwardAngles = tagForward.toAngles();
		forwardAngles.copyTo( tempForward );
		
		AnglesToAxis( tempForward, tempAxis );
		AnglesToAxis( edict->s.attach_angles_offset , tempAxis2 );
		MatrixMultiply( tempAxis2, tempAxis, finalAxis );
		AxisToAngles( finalAxis, finalForward );
		angles = finalForward;		
		
		AnglesToAxis( angles, orientation );

		}
	else
      {
      AnglesToAxis( angles, orientation );
      }

   angles.copyTo( edict->s.netangles );
   angles.copyTo( edict->s.angles );
   angles.copyTo( edict->currentAngles );
	// Fill the edicts matrix
	VectorCopy( orientation[ 0 ], edict->s.mat[ 0 ] );
	VectorCopy( orientation[ 1 ], edict->s.mat[ 1 ] );
	VectorCopy( orientation[ 2 ], edict->s.mat[ 2 ] );

	if (this->isSubclassOf( Player ) )
	{
		Player *player = (Player*)this ;
		player->GetVAngles().copyTo( edict->s.viewangles );
	}
	else
	{
		edict->s.viewangles[0] = 0.0f ;
		edict->s.viewangles[1] = 0.0f ;
		edict->s.viewangles[2] = 0.0f ;
	}

	if ( bind_info )
		{
		// Go through and set our children

		num = bind_info->numchildren;

		for( i = 0 ; (i < MAX_MODEL_CHILDREN) && num ; i++ )
			{
			if ( bind_info->children[i] == ENTITYNUM_NONE )
				continue;
			ent = ( Entity * )G_GetEntity( bind_info->children[i] );
			if ( ent )
			{
				ent->setAngles();
				VectorClear( ent->edict->s.netangles );
			}
			num--;
			}

		/* for( ent = bind_info->teamchain; ent != NULL; ent = ent->bind_info->teamchain )
			{
			if ( ent->bind_info->teammaster == this )
				ent->setAngles();
			} */
		}
	}

qboolean Entity::droptofloor
	(
	float maxfall
	)

	{
	trace_t	trace;
	Vector	end;
   Vector   start;

   //start = origin + Vector( "0 0 1" );
	start = origin;
   end = origin;
	end[ 2 ]-= maxfall;

	trace = G_Trace( start, mins, maxs, end, this, edict->clipmask, false, "Entity::droptofloor" );
	if ( ( trace.fraction == 1.0f ) || trace.startsolid || trace.allsolid || !trace.ent )
		{
		groundentity = world->edict;
		return false;
		}

	setOrigin( trace.endpos );

	groundentity = trace.ent;

	return true;
	}

void Entity::DamageType
   (
   Event *ev
   )

   {
   str damage;
   damage = ev->GetString( 1 );
   if ( damage == "all" )
      {
      damage_type = -1;
      }
   else
      {
      damage_type = MOD_NameToNum( damage );
      }
   }

void Entity::Damage
   (
   Entity *inflictor,
   Entity *attacker,
   float damage,
   const Vector &position,
   const Vector &direction,
   const Vector &normal,
   int knockback,
   int dflags,
   int meansofdeath,
	int surface_number,
	int bone_number,
	Entity *weapon
   )

   {
	Event	*ev;

   // if our damage types do not match, return
   if ( !MOD_matches( meansofdeath, damage_type ) )
      {
      return;
      }

	if ( !attacker )
		{
		attacker = world;
		}
	if ( !inflictor )
		{
		inflictor = world;
		}

	ev = new Event( EV_Damage );
	ev->AddFloat( damage );
	ev->AddEntity ( inflictor );
	ev->AddEntity ( attacker );
   ev->AddVector ( position );
   ev->AddVector ( direction );
   ev->AddVector ( normal );
   ev->AddInteger( knockback );
   ev->AddInteger( dflags );
   ev->AddInteger( meansofdeath );
	ev->AddInteger( surface_number );
	ev->AddInteger( bone_number );
	ev->AddEntity( weapon );
   ProcessEvent  ( ev );
   }

void Entity::DamageEvent
	(
	Event *ev
	)

	{
	Entity	*inflictor;
	Entity	*attacker;
	float		damage;
	Vector	dir;
	Vector	momentum;
	Event		*event;
	float		m;

	if ( ( takedamage == DAMAGE_NO ) || ( movetype == MOVETYPE_NOCLIP ) )
		{
		return;
		}

	damage		= ev->GetFloat( 1 );
	inflictor	= ev->GetEntity( 2 );
	attacker		= ev->GetEntity( 3 );

	if ( !inflictor || !attacker )
		return;

	// figure momentum add
	if ( ( inflictor != world ) &&
		( movetype != MOVETYPE_NONE ) &&
		( movetype != MOVETYPE_STATIONARY ) &&
		( movetype != MOVETYPE_BOUNCE ) &&
		( movetype != MOVETYPE_PUSH ) &&
		( movetype != MOVETYPE_STOP ) )
		{
      dir = origin - ( inflictor->origin + ( inflictor->mins + inflictor->maxs ) * 0.5f );
		dir.normalize();

		if ( mass < 50 )
			{
			m = 50;
			}
		else
			{
			m = mass;
			}

		momentum = dir * damage * ( 1700.0f / (float)m );
		velocity += momentum;
		}

	// check for godmode or invincibility
	if ( flags & FL_GODMODE )
		{
		return;
		}

	// team play damage avoidance
	//if ( ( global->teamplay == 1 ) && ( edict->team > 0 ) && ( edict->team == attacker->edict->team ) )
	//	{
	//	return;
	//	}

   if ( !multiplayerManager.inMultiplayer() && isSubclassOf( Player ) )
      {
      damage *= 0.15f;
      }

   if ( deadflag )
      {
      // Check for gib.
      if ( inflictor->isSubclassOf( Projectile ) )
         {
         Event *gibEv;

         health -= damage;

         gibEv = new Event( EV_Gib );
         gibEv->AddEntity( this );
         gibEv->AddFloat( health );
         ProcessEvent( gibEv );
         }
      return;
      }

	// do the damage
	health -= damage;
	if ( health <= 0.0f )
		{
      if ( attacker )
         {
		   event = new Event( EV_GotKill );
		   event->AddEntity( this );
		   event->AddFloat( damage );
		   event->AddEntity( inflictor );
         event->AddInteger( ev->GetInteger( 9 ) );
         event->AddInteger( 0 );
		   attacker->ProcessEvent( event );
         }

		event = new Event( EV_Killed );
		event->AddEntity( attacker );
		event->AddFloat( damage );
		event->AddEntity( inflictor );
		ProcessEvent( event );
		return;
		}

	event = new Event( EV_Pain );
	event->AddFloat( damage );
	event->AddEntity( attacker );
	ProcessEvent( event );
	}

void Entity::Stun
   (
   float time
   )

   {
   Event *ev = new Event( EV_Stun );
   ev->AddFloat( time );
   ProcessEvent( ev );
   }

/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean Entity::CanDamage
	(
	const Entity *target,
	const Entity *skip_ent
	)

	{
	trace_t	trace;
	Vector	pos;
	const Entity *skip_entity;
	//int maskToUse = MASK_SHOT;
	int maskToUse = CONTENTS_SOLID;

	if ( skip_ent )
		skip_entity = skip_ent;
	else
		skip_entity = this;

   trace = G_Trace( origin, vec_origin, vec_origin, target->centroid, skip_entity, maskToUse, false, "Entity::CanDamage 1" );
	if ( ( trace.fraction == 1.0f ) || ( trace.ent == target->edict ) )
		{
		return true;
		}
	pos = target->centroid + Vector( 15.0f, 15.0f, 0.0f );
   trace = G_Trace( origin, vec_origin, vec_origin, pos, skip_entity, maskToUse, false, "Entity::CanDamage 3" );
	if ( ( trace.fraction == 1.0f ) || ( trace.ent == target->edict ) )
		{
		return true;
		}
	pos = target->centroid + Vector( -15.0f, 15.0f, 0.0f );
   trace = G_Trace( origin, vec_zero, vec_zero, pos, skip_entity, maskToUse, false, "Entity::CanDamage 4" );
	if ( ( trace.fraction == 1.0f ) || ( trace.ent == target->edict ) )
		{
		return true;
		}
	pos = target->centroid + Vector( 15.0f, -15.0f, 0.0f );
   trace = G_Trace( origin, vec_zero, vec_zero, pos, skip_entity, maskToUse, false, "Entity::CanDamage 5" );
	if ( ( trace.fraction == 1.0f ) || ( trace.ent == target->edict ) )
		{
		return true;
		}
  	pos = target->centroid + Vector( -15.0f, -15.0f, 0.0f );
   trace = G_Trace( origin, vec_zero, vec_zero, pos, skip_entity, maskToUse, false, "Entity::CanDamage 6" );
	if ( ( trace.fraction == 1.0f ) || ( trace.ent == target->edict ) )
		{
		return true;
		}

	return false;
	}

qboolean Entity::IsTouching
	(
	const Entity *e1
	)

	{
	if ( e1->absmin.x > absmax.x )
		{
		return false;
		}
	if ( e1->absmin.y > absmax.y )
		{
		return false;
		}
	if ( e1->absmin.z > absmax.z )
		{
		return false;
		}
	if ( e1->absmax.x < absmin.x )
		{
		return false;
		}
	if ( e1->absmax.y < absmin.y )
		{
		return false;
		}
	if ( e1->absmax.z < absmin.z )
		{
		return false;
		}

	return true;
	}

void Entity::FadeNoRemove
	(
	Event *ev
	)

	{
   float rate;
   float target;
   float myalpha;

   if ( ev->NumArgs() > 1 )
      {
      target = ev->GetFloat( 2 );
      }
   else
      {
      target = 0;
      }

   if ( ev->NumArgs() > 0 )
      {
      rate = ev->GetFloat( 1 );
      assert( rate );
      if ( rate > 0.0f )
         rate = FRAMETIME / rate;
      }
   else
      {
      rate = 0.03f;
      }

   myalpha = edict->s.alpha;
   myalpha -= rate;

   if ( myalpha < target )
      myalpha = target;

   setAlpha( myalpha );

   if ( myalpha > target )
      {
      PostEvent( *ev, FRAMETIME );
      }

   G_SetConstantLight( &edict->s.constantLight, &myalpha, &myalpha, &myalpha, 0 );
	}

void Entity::FadeOut
	(
	Event *ev
	)

	{
   float myscale;
   float myalpha;

   myscale = edict->s.scale;
   myscale -= 0.03f;
   myalpha = edict->s.alpha;
   myalpha -= 0.03f;
   if ( myscale < 0.0f )
      myscale = 0.0f;
   if ( myalpha < 0.0f )
      myalpha = 0.0f;

	if ( ( myscale <= 0.0f ) && ( myalpha <= 0.0f ) )
		{
	   PostEvent( EV_Remove, 0.0f );
		}
   else
      {
	   PostEvent( *ev, FRAMETIME );
      }

   setScale( myscale );
   setAlpha( myalpha );
	}

void Entity::FadeIn
	(
	Event *ev
	)

	{
   float rate;
   float target;
   float myalpha;

   if ( ev->NumArgs() > 1 )
      {
      target = ev->GetFloat( 2 );
      }
   else
      {
      target = 1;
      }

   if ( ev->NumArgs() > 0 )
      {
      rate = ev->GetFloat( 1 );
      assert( rate );
      if ( rate > 0.0f )
         rate = FRAMETIME / rate;
      }
   else
      {
      rate = 0.03f;
      }

   myalpha = edict->s.alpha;
   myalpha += rate;

   if ( myalpha > target )
      myalpha = target;

   if ( myalpha < target )
		{
   	PostEvent( *ev, FRAMETIME );
		}
   setAlpha( myalpha );
	}

void Entity::Fade
	(
	Event *ev
	)

	{
   float rate;
   float target;
   float myalpha;

   if ( ev->NumArgs() > 1 )
      {
      target = ev->GetFloat( 2 );
      }
   else
      {
      target = 0;
      }

   if ( ev->NumArgs() > 0 )
      {
      rate = ev->GetFloat( 1 );
      assert( rate );
      if ( rate > 0.0f )
         rate = FRAMETIME / rate;
      }
   else
      {
      rate = 0.03f;
      }

   myalpha = edict->s.alpha;
   myalpha -= rate;

   if ( myalpha <= 0.0f )
		{
	   PostEvent( EV_Remove, 0.0f );
      return;
		}

   if ( myalpha < target )
      myalpha = target;

   if ( myalpha > target )
      {
      PostEvent( *ev, FRAMETIME );
      }

   setAlpha( myalpha );
   G_SetConstantLight( &edict->s.constantLight, &myalpha, &myalpha, &myalpha, 0 );
	}

void Entity::SetMassEvent
	(
	Event *ev
	)

	{
   mass = ev->GetInteger( 1 );
	}

void Entity::CheckGround
	(
   void
	)

	{
	Vector	point;
	trace_t	trace;

	if ( flags & ( FL_SWIM | FL_FLY ) )
		{
		return;
		}

	if ( velocity.z > 100.0f )
		{
		groundentity = NULL;
		return;
		}

	// if the hull point one-quarter unit down is solid the entity is on ground
	point = origin;
	point.z -= sv_groundtracelength->value;
	
	trace = G_Trace( origin, mins, maxs, point, this, edict->clipmask, false, "Entity::CheckGround" );

	if (!(trace.surfaceFlags & SURF_TERRAIN))
	{
		point.z = origin.z - 0.25f; // put groundtrace dist back to 0.25f for non-terrain check
		trace = G_Trace( origin, mins, maxs, point, this, edict->clipmask, false, "Entity::CheckGround" );
	}

	// check steepness
	if ( ( trace.plane.normal[ 2 ] <= 0.7f ) && !trace.startsolid )
		{
		groundentity = NULL;
		return;
		}

	groundentity = trace.ent;
	groundplane = trace.plane;
	groundcontents = trace.contents;

	if ( !trace.startsolid && !trace.allsolid )
		{
		setOrigin( trace.endpos );
		velocity.z = 0;
		}
	}

void Entity::BecomeSolid
	(
	Event *ev
	)

	{
	if ( ( model.length() ) && ( ( model[ 0 ] == '*' ) || ( strstr( model.c_str(), ".bsp" ) ) ) )
		{
		setSolidType( SOLID_BSP );
		}
	else
		{
		setSolidType( SOLID_BBOX );
		}
	}

void Entity::BecomeNonSolid
	(
	Event *ev
	)

	{
	setSolidType( SOLID_NOT );
	}

void Entity::Ghost
	(
	Event *ev
	)

	{
   // Make not solid, but send still send over whether it is hidden or not
	setSolidType( SOLID_NOT );
   edict->svflags &= ~SVF_NOCLIENT;
	}

void Entity::LoopSound
	(
	Event *ev
	)

	{
	str sound_name;
	float volume   = DEFAULT_VOL;
	float min_dist = DEFAULT_MIN_DIST;
	str min_dist_string;


	if (ev->NumArgs() < 1)
		return;

	// Get parameters

	sound_name = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		volume = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		{
		min_dist_string = ev->GetString( 3 );

		if ( min_dist_string == LEVEL_WIDE_STRING )
			min_dist = LEVEL_WIDE_MIN_DIST;
		else
			min_dist = ev->GetFloat( 3 );

		if ( min_dist >= LEVEL_WIDE_MIN_DIST_CUTOFF )
			min_dist = LEVEL_WIDE_MIN_DIST;
		}

	// Add this sound to loop

	LoopSound( sound_name, volume, min_dist );
	}

void Entity::LoopSound( const str &sound_name, float volume, float min_dist )
	{
	const char *name = NULL;
	str random_alias;


	// Get the real sound to be played

	if ( sound_name.length() > 0 )
		{
		// Get the real sound to play

		name = gi.GlobalAlias_FindRandom( sound_name.c_str() );

		if ( !name )
			{
			random_alias = GetRandomAlias( sound_name ).c_str();

			if ( random_alias.length() > 0 )
				name = random_alias.c_str();
			}

		if ( !name )
			name = sound_name.c_str();

		// Add the looping sound to the entity

      edict->s.loopSound        = gi.soundindex( name );
		edict->s.loopSoundVolume  = volume;
		edict->s.loopSoundMinDist = min_dist;
		}
	}

void Entity::StopLoopSound( Event *ev )
	{
   StopLoopSound();
	}

void Entity::StopLoopSound( void )
	{
   edict->s.loopSound = 0;
	}

void Entity::Sound( Event *ev )
	{
	str sound_name;
	float volume;
   int channel;
   float min_dist;
   int i;
	str min_dist_string;


   // Set defaults

   volume   = DEFAULT_VOL;
	min_dist = DEFAULT_MIN_DIST;
   channel  = CHAN_BODY;

	// Get sound parameters

   for ( i = 1 ; i <= ev->NumArgs() ; i++ )
      {
      switch (i-1)
         {
         case 0:
            sound_name = ev->GetString( i );
            break;
         case 1:
            channel = ev->GetInteger( i );
            break;
			case 2:
            volume = ev->GetFloat( i );
            break;
         case 3:
				min_dist_string = ev->GetString( i );

				if ( min_dist_string == LEVEL_WIDE_STRING )
					min_dist = LEVEL_WIDE_MIN_DIST;
				else
					min_dist = ev->GetFloat( i );

				if ( min_dist >= LEVEL_WIDE_MIN_DIST_CUTOFF )
					min_dist = LEVEL_WIDE_MIN_DIST;
            break;
         default:
            break;
         }
      }

	Sound( sound_name, channel, volume, min_dist, NULL );
	}

void Entity::StopSound
	(
	Event *ev
	)

	{
   if (ev->NumArgs() < 1)
      StopSound( CHAN_BODY );
   else
      StopSound( ev->GetInteger( 1 ) );
	}

void Entity::StopSound
	(
	int channel
	)

	{
	gi.StopSound( entnum, channel );
	}

void Entity::SetLight
	(
	Event *ev
	)

	{
   float r, g, b;

   if ( ev->NumArgs() == 1 )
      {
      Vector tmp;

      tmp = ev->GetVector( 1 );
      r = tmp.x;
      g = tmp.y;
      b = tmp.z;
      }
   else
      {
      r = ev->GetFloat( 1 );
      g = ev->GetFloat( 2 );
      b = ev->GetFloat( 3 );
      lightRadius  = ev->GetFloat( 4 );
      }

   G_SetConstantLight( &edict->s.constantLight, &r, &g, &b, &lightRadius );
   }

void Entity::LightOn
	(
	Event *ev
	)

	{
   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, NULL, &lightRadius );
   }

void Entity::LightOff
	(
	Event *ev
	)

	{
   float radius = 0;

   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, NULL, &radius );
   }

void Entity::LightRed
	(
	Event *ev
	)

	{
   float r;

   r = ev->GetFloat( 1 );
   G_SetConstantLight( &edict->s.constantLight, &r, NULL, NULL, NULL );
   }

void Entity::LightGreen
	(
	Event *ev
	)

	{
   float g;

   g = ev->GetFloat( 1 );
   G_SetConstantLight( &edict->s.constantLight, NULL, &g, NULL, NULL );
   }

void Entity::LightBlue
	(
	Event *ev
	)

	{
   float b;

   b = ev->GetFloat( 1 );
   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, &b, NULL );
   }

void Entity::LightRadius
	(
	Event *ev
	)

	{
   lightRadius = ev->GetFloat( 1 );
   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, NULL, &lightRadius );
   }

void Entity::LightStyle
	(
	Event *ev
	)

	{
   int style;

   style = ev->GetInteger( 1 );
   G_SetConstantLight( &edict->s.constantLight, NULL, NULL, NULL, NULL, &style );
   }

void Entity::SetHealth
	(
	Event *ev
	)

	{
	health = ev->GetFloat( 1 );
   if( max_health < health )
		{
		max_health = health;
		}
	
	}


//--------------------------------------------------------------
//
// Name:			setGameplayHealth
// Class:			Entity
//
// Description:		This function acts as a filter to the real function.
//					It gets data from the database, and then passes it
//					along to the original event.  This is here as an attempt
//					to sway people into using the database standard instead of
//					hardcoded numbers.
//
// Parameters:		Event *ev
//						str -- The value keyword from the database (low, medium, high, etc).
//
// Returns:			None
//
//--------------------------------------------------------------
void Entity::setGameplayHealth( Event *ev )
{
	if ( ev->NumArgs() < 1 )
		return;

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();	
	if ( !gpm->hasFormula("Health") )
		return;

	str healthstr = ev->GetString( 1 );
	float healthmod = 1.0f;
	if ( gpm->getDefine(healthstr) != "" )
		healthmod = (float)atof(gpm->getDefine(healthstr));
	GameplayFormulaData fd(this);
	float finalhealth = gpm->calculate("Health", fd, healthmod);
	Event *newev = new Event(EV_SetHealth);
	newev->AddFloat(finalhealth);
	ProcessEvent(newev);
}

//----------------------------------------------------------------
// Name:			addHealth
// Class:			Entity
//
// Description:		Adds health to the entity up to the max health of the entity or the
//					max health specified (if its not 0) 
//
// Parameters:		float healthToAdd				- health we are going to add to the entity
//					float maxHealth					- if not 0, the max health the entity can have
//
// Returns:			none
//----------------------------------------------------------------

void Entity::addHealth( float healthToAdd, float maxHealth )
{
	float tempMaxHealth;
	float newHealth;

	// Get the max health

	if ( maxHealth )
		tempMaxHealth = maxHealth;
	else
		tempMaxHealth = max_health;

	// Calculate the new health

	newHealth = health + healthToAdd;

	// Set the new health
	
	if ( newHealth > tempMaxHealth )
		setHealth( tempMaxHealth );
	else
		setHealth( newHealth );
}

void Entity::GetHealth
	(
	Event *ev
	)

	{
	ev->ReturnFloat( health );
	}

void Entity::SetMaxHealth 
	(
	Event *ev
	)
	{
	max_health = ev->GetFloat( 1 );
	}

void Entity::SetSize
	(
	Event *ev
	)

	{
	Vector min, max;

   min = ev->GetVector( 1 );
   max = ev->GetVector( 2 );
   setSize( min, max );
   }

void Entity::SetMins
	(
	Event *ev
	)

	{
	Vector min;

   min = ev->GetVector( 1 );
   setSize( min, maxs );
   }

void Entity::SetMaxs
	(
	Event *ev
	)

	{
	Vector max;

   max = ev->GetVector( 1 );
   setSize( mins, max );
   }

void Entity::GetMins
	(
	Event *ev
	)

	{
	ev->ReturnVector( mins );
	}
	
void Entity::GetMaxs
	(
	Event *ev
	)

	{
	ev->ReturnVector( maxs );
	}


void Entity::SetScale
	(
	Event *ev
	)

	{
	setScale( ev->GetFloat( 1 ) );
   }

void Entity::setRandomScale( Event *ev )
{
	float minScale;
	float maxScale;

	minScale = ev->GetFloat( 1 );
	maxScale = ev->GetFloat( 2 );

	setScale( G_Random( maxScale - minScale ) + minScale );
}

void Entity::SetAlpha
	(
	Event *ev
	)

	{
	setAlpha( ev->GetFloat( 1 ) );
   }

void Entity::SetOrigin
	(
	Event *ev
	)

	{
	setOrigin( ev->GetVector( 1 ) );
	}

void Entity::GetOrigin
	(
	Event *ev
	)

	{
	ev->ReturnVector( origin );
	}

void Entity::SetTargetName
	(
	Event *ev
	)

	{
	SetTargetName( ev->GetString( 1 ) );
	}

//----------------------------------------------------------------
// Name:			GetRawTargetName
// Class:			Entity
//
// Description:		Gets the targetname of the entity without the leading $
//
// Parameters:		None
//
// Returns:			str targetname (through ev)		- the targetname of the entity
//----------------------------------------------------------------

void Entity::GetRawTargetName
	(
	Event *ev
	)

	{
	ev->ReturnString( targetname );
	}

//----------------------------------------------------------------
// Name:			GetTargetName
// Class:			Entity
//
// Description:		Gets the targetname of the entity with the leading $
//
// Parameters:		None
//
// Returns:			str targetname (through ev)		- the targetname of the entity
//----------------------------------------------------------------

void Entity::GetTargetName
	(
	Event *ev
	)

	{
	str nameToReturn;

	nameToReturn = "$";
	nameToReturn += targetname;

	ev->ReturnString( nameToReturn );
	}

void Entity::SetTarget
	(
	Event *ev
	)

	{
	SetTarget( ev->GetString( 1 ) );
	}

void Entity::getTarget( Event *ev )
{
	str nameToReturn;
	bool wantsPrefix;

	if ( ev->NumArgs() > 0 )
		wantsPrefix = ev->GetBoolean( 1 );
	else
		wantsPrefix = false;

	if ( wantsPrefix )
	{
		nameToReturn = "$";
	}

	nameToReturn += target;

	ev->ReturnString( nameToReturn );
}

//--------------------------------------------------------------
// Name:		GetTargetEntity()
// Class:		Entity
//	
// Description:	Returns the Entity of the target 
//
// Parameters:	Event *ev
//
// Returns:		None ( Entity, though, through the event )
//--------------------------------------------------------------
void Entity::GetTargetEntity( Event *ev )
{
	TargetList *tlist	 = world->GetTargetList( target, false );
   
	if ( tlist )
		{
		 ev->ReturnEntity( tlist->GetNextEntity( NULL ) );
		}
	else
		{
		ev->ReturnEntity( NULL );
		}	
}


void Entity::SetKillTarget
	(
	Event *ev
	)

	{
	SetKillTarget( ev->GetString( 1 ) );
	}

//-----------------------------------------------------
//
// Name:		GetModelName
// Class:		Entity
//
// Description:	Retrieves the model name
//
// Parameters:	event - the event to request the model name
//
// Returns:		None
//-----------------------------------------------------
void Entity::GetModelName(Event* ev)
{
	ev->ReturnString(model.c_str());
}

void Entity::SetAngles
	(
	Event *ev
	)

	{
	setAngles( ev->GetVector( 1 ) );
	}

//-----------------------------------------------------
//
// Name:		GetAngles
// Class:		Entity
//
// Description:	Retrieves the entity's angles
//
// Parameters:	event - event to request the entity's angles
//
// Returns:		None
//-----------------------------------------------------
void Entity::GetAngles(Event* ev)
{
	ev->ReturnVector(angles);
}


Vector Entity::GetControllerAngles
	(
	int num
	)

	{
	Vector controller_angles;

   assert( ( num >= 0 ) && ( num < NUM_BONE_CONTROLLERS ) );

   if ( ( num < 0 ) || ( num >= NUM_BONE_CONTROLLERS ) )
      {
      error( "GetControllerAngles", "Bone controller index out of range (%d)\n", num );
      return vec_zero;
      }

	controller_angles = edict->s.bone_angles[ num ];

   return controller_angles;
   }

void Entity::SetControllerAngles
	(
	int num,
   vec3_t angles
	)

	{
   assert( ( num >= 0 ) && ( num < NUM_BONE_CONTROLLERS ) );

   if ( ( num < 0 ) || ( num >= NUM_BONE_CONTROLLERS ) )
      {
      error( "SetControllerAngles", "Bone controller index out of range (%d)\n", num );
      return;
      }

   VectorCopy( angles, edict->s.bone_angles[ num ] );
   EulerToQuat( edict->s.bone_angles[ num ], edict->s.bone_quat[ num ] );
   }

void Entity::SetControllerAngles
	(
	Event *ev
	)

	{
	int num;
   Vector angles;

	if ( ev->NumArgs() < 2 )
		return;

	num = ev->GetInteger( 1 );
	angles = ev->GetVector( 2 );

	SetControllerAngles( num, angles );
   }

void Entity::SetControllerTag
	(
	int num,
   int tag_num
	)

	{
   assert( ( num >= 0 ) && ( num < NUM_BONE_CONTROLLERS ) );

   if ( ( num < 0 ) || ( num >= NUM_BONE_CONTROLLERS ) )
      {
      error( "SetControllerTag", "Bone controller index out of range (%d)\n", num );
      return;
      }

   edict->s.bone_tag[ num ] = tag_num;
   }



//===============================================================
// Name:		SetFullTraceEvent
// Class:		Entity
//
// Description: Sets the fulltrace flag.  This flag is used by
//				G_PushMove to determine if the object needs a
//				fulltrace or not.  Default for flag is false.
// 
// Parameters:	Event* -- first argument is boolean
//
// Returns:		None
// 
//===============================================================
inline void Entity::SetFullTraceEvent
(
	Event *ev
)
{
	_fulltrace = ev->GetBoolean( 1 );
}


void Entity::RegisterAlias
	(
	Event *ev
	)

	{
	char parameters[100];
	int i;

	// Get the parameters for this alias command

	parameters[0] = 0;

	for( i = 3 ; i <= ev->NumArgs() ; i++ )
	{
		strcat( parameters, ev->GetString( i ) );
		strcat( parameters, " ");
	}

   gi.Alias_Add( edict->s.modelindex, ev->GetString( 1 ), ev->GetString( 2 ),  parameters );
	}

void Entity::Cache
	(
	Event *ev
	)

	{
   CacheResource( ev->GetString( 1 ), this );
   }

void Entity::RegisterAliasAndCache
	(
	Event *ev
	)

	{
	RegisterAlias(ev);

   CacheResource( ev->GetString( 2 ), this );
	}

void Entity::Sound
	(
	const str &sound_name,
   int   channel,
	float volume,
   float min_dist,
	Vector *sound_origin,
	float pitch_modifier,
	qboolean onlySendToThisEntity
	)
	{
	const char *name = NULL;
	vec3_t org;
	str random_alias;


	if ( sound_name.length() > 0 )
		{
		// Get the real sound to play

		name = gi.GlobalAlias_FindRandom( sound_name.c_str() );

		if ( !name )
			{
			random_alias = GetRandomAlias( sound_name ).c_str();

			if ( random_alias.length() > 0 )
				name = random_alias.c_str();
			}

		if ( !name )
			name = sound_name.c_str();

		// Play the sound

		if ( name != NULL )
			{
			if ( sound_origin != NULL)
				{
				sound_origin->copyTo( org );
				entnum = ENTITYNUM_NONE;
				}
			else
				{
				VectorCopy( edict->s.origin, org );
				}

			gi.Sound( &org, entnum, channel, name, volume, min_dist, pitch_modifier, onlySendToThisEntity );
			}
		}
	else
		{
		warning( "Sound", "Null sample pointer" );
		}
	}

qboolean Entity::attach
	(
   int parent_entity_num,
   int tag_num,
	qboolean use_angles,
	Vector offset,
	Vector angles_offset
	)

	{
   int i;
   Entity * parent;

   if ( entnum == parent_entity_num )
      {
      warning("attach","Trying to attach to oneself." );
      return false;
      }

   if ( edict->s.parent != ENTITYNUM_NONE )
      detach();

   //
   // make sure this is a modelanim entity so that the attach works properly
   //
   if ( edict->s.eType == ET_GENERAL )
      {
      edict->s.eType = ET_MODELANIM;
      }

   //
   // get the parent
   //
   parent = ( Entity * )G_GetEntity( parent_entity_num );

	if ( !parent->bind_info )
		parent->bind_info = CreateBindInfo();

   if (parent->bind_info->numchildren < MAX_MODEL_CHILDREN)
      {
      //
      // find a free spot in the parent
      //
      for ( i=0; i < MAX_MODEL_CHILDREN; i++ )
         if ( parent->bind_info->children[i] == ENTITYNUM_NONE )
            {
            break;
            }
      edict->s.parent = parent_entity_num;
      setSolidType( SOLID_NOT );
      parent->bind_info->children[i] = entnum;
      parent->bind_info->numchildren++;
	  if ( tag_num >= 0 )
	  {
		edict->s.tag_num = tag_num;
	  }
		edict->s.attach_use_angles = use_angles;
		offset.copyTo( edict->s.attach_offset );
		angles_offset.copyTo( edict->s.attach_angles_offset );
      setOrigin();
      return true;
      }
   return false;
	}

void Entity::KillAttach
	(
	Event *ev
	)

	{
   int i;
	Entity *child = NULL;


	if ( bind_info )
		{
		// Kill all of this entities children

		for ( i = 0 ; i < MAX_MODEL_CHILDREN; i++ )
			{
			if ( bind_info->children[i] != ENTITYNUM_NONE )
				{
				// Remove child
				child = ( Entity * )G_GetEntity( bind_info->children[i] );

				if ( child )
					child->ProcessEvent( EV_Remove );

				// Remove child from this entity
				bind_info->children[i] = ENTITYNUM_NONE;
				}
			}

		bind_info->numchildren = 0;
		}
	}

void Entity::detach
	(
	void
	)

	{
   int i;
   int num;
   Entity * parent;

   if ( edict->s.parent == ENTITYNUM_NONE )
      return;

   parent = ( Entity * )G_GetEntity( edict->s.parent );

   if (!parent)
      return;

	if ( parent->bind_info)
		{
		for ( i=0,num = parent->bind_info->numchildren; i < MAX_MODEL_CHILDREN; i++ )
			{
			if ( parent->bind_info->children[i] == ENTITYNUM_NONE )
				{
				continue;
				}
			if (parent->bind_info->children[i] == entnum)
				{
				parent->bind_info->children[i] = ENTITYNUM_NONE;
				parent->bind_info->numchildren--;
				break;
				}
			num--;
			if (!num)
				break;
			}
		}

   edict->s.parent = ENTITYNUM_NONE;
   setOrigin( origin );
	}

void Entity::Flags( Event *ev )
   {
   const char *flag;
   int mask;
   int action;
   int i;

   for ( i = 1; i <= ev->NumArgs(); i++ )
      {
      action = FLAG_IGNORE;
      flag = ev->GetString( i );
      switch( flag[0] )
         {
         case '+':
            action = FLAG_ADD;
            flag++;
            break;
         case '-':
            action = FLAG_CLEAR;
            flag++;
            break;
         default:
            ev->Error( "Entity::Flags", "First character is not '+' or '-', assuming '+'\n" );
            action = FLAG_ADD;
            break;
         }

      //
      // WARNING: please change the Event decleration,
      // to match this function, if flags are added or
      // deleted the event must be updated.
      //
      if ( !stricmp( flag, "blood" ) )
         mask = FL_BLOOD;
      else if ( !stricmp( flag, "explode" ) )
         mask = FL_DIE_EXPLODE;
      else if ( !stricmp( flag, "die_gibs" ) )
         mask = FL_DIE_GIBS;
      else if ( !stricmp( flag, "autoaim" ) )
         mask = FL_AUTOAIM;
      else if ( !stricmp( flag, "god" ) )
         mask = FL_GODMODE;
		else if ( !stricmp( flag, "notarget" ) )
			mask = FL_NOTARGET;
      else
         {
         mask = 0;
         action = FLAG_IGNORE;
         ev->Error( "Unknown flag '%s'", flag );
         }
      switch (action)
         {
         case FLAG_ADD:
            flags |= mask;
            break;
         case FLAG_CLEAR:
            flags &= ~mask;
            break;
         case FLAG_IGNORE:
            break;
         }
      }
   if ( !com_blood->integer )
      {
      if ( flags & (FL_BLOOD|FL_DIE_GIBS) )
         {
         flags &= ~FL_BLOOD;
         flags &= ~FL_DIE_GIBS;
         }
      }
   }


void Entity::Effects( Event *ev )
   {
   const char *flag;
   int mask=0;
   int action;
   int i;

   for ( i = 1; i <= ev->NumArgs(); i++ )
      {
      action = 0;
      flag = ev->GetString( i );
      switch( flag[0] )
         {
         case '+':
            action = FLAG_ADD;
            flag++;
            break;
         case '-':
            action = FLAG_CLEAR;
            flag++;
            break;
         default:
            ev->Error( "Entity::Effects", "First character is not '+' or '-', assuming '+'\n" );
            action = FLAG_ADD;
            break;
         }

      //
      // WARNING: please change the Event decleration,
      // to match this function, if flags are added or
      // deleted the event must be updated.
      //
      if ( !stricmp( flag, "everyframe" ) )
         mask = EF_EVERYFRAME;
      else
         {
         action = FLAG_IGNORE;
         ev->Error( "Unknown token %s.", flag );
         }

      switch (action)
         {
         case FLAG_ADD:
            edict->s.eFlags |= mask;
            break;
         case FLAG_CLEAR:
            edict->s.eFlags &= ~mask;
            break;
         case FLAG_IGNORE:
            break;
         }
      }
   }

void Entity::RenderEffects( Event *ev )
   {
   const char *flag;
   int mask=0;
   int action;
   int i;

   for ( i = 1; i <= ev->NumArgs(); i++ )
      {
      action = 0;
      flag = ev->GetString( i );
      switch( flag[0] )
         {
         case '+':
            action = FLAG_ADD;
            flag++;
            break;
         case '-':
            action = FLAG_CLEAR;
            flag++;
            break;
         default:
            ev->Error( "Entity::RenderEffects", "First character is not '+' or '-', assuming '+'\n" );
            action = FLAG_ADD;
            break;
         }

      //
      // WARNING: please change the Event decleration,
      // to match this function, if flags are added or
      // deleted the event must be updated.
      //
      if ( !stricmp( flag, "dontdraw" ) )
         mask = RF_DONTDRAW;
      else if ( !stricmp( flag, "betterlighting" ) )
         mask = RF_EXTRALIGHT;
      else if ( !stricmp ( flag, "lensflare" ) )
         mask = RF_LENSFLARE;
      else if ( !stricmp ( flag, "viewlensflare" ) )
         mask = RF_VIEWLENSFLARE;
      else if ( !stricmp ( flag, "lightoffset" ) )
         mask = RF_LIGHTOFFSET;
      else if ( !stricmp( flag, "skyorigin" ) )
         mask = RF_SKYORIGIN;
      else if ( !stricmp( flag, "fullbright" ) )
         mask = RF_FULLBRIGHT;
      else if ( !stricmp( flag, "minlight" ) )
         mask = RF_MINLIGHT;
      else if ( !stricmp( flag, "additivedynamiclight" ) )
         mask = RF_ADDITIVE_DLIGHT;
      else if ( !stricmp( flag, "lightstyledynamiclight" ) )
         mask = RF_LIGHTSTYLE_DLIGHT;
      else if ( !stricmp( flag, "shadow" ) )
         mask = RF_SHADOW;
	  else if ( !stricmp( flag, "shadowFromBip01" ) )
         mask = RF_SHADOW_FROM_BIP01;
      else if ( !stricmp( flag, "preciseshadow" ) )
         mask = RF_SHADOW_PRECISE;
	  else if ( !stricmp( flag, "dontInheritAlpha" ) )
         mask = RF_CHILDREN_DONT_INHERIT_ALPHA;
      else if ( !stricmp( flag, "invisible" ) )
         mask = RF_INVISIBLE;
	  else if ( !stricmp( flag, "depthhack" ) )
         mask = RF_DEPTHHACK;
      else
         {
         action = FLAG_IGNORE;
         ev->Error( "Unknown token %s.", flag );
         }

      switch (action)
         {
         case FLAG_ADD:
            edict->s.renderfx |= mask;
            break;
         case FLAG_CLEAR:
            edict->s.renderfx &= ~mask;
            break;
         case FLAG_IGNORE:
            break;
         }
      }
   }

void Entity::SVFlags
   (
   Event *ev
   )

   {
   const char *flag;
   int mask=0;
   int action;
   int i;

   for ( i = 1; i <= ev->NumArgs(); i++ )
      {
      action = 0;
      flag = ev->GetString( i );
      switch( flag[0] )
         {
         case '+':
            action = FLAG_ADD;
            flag++;
            break;
         case '-':
            action = FLAG_CLEAR;
            flag++;
            break;
         default:
            ev->Error( "Entity::SVFlags", "First character is not '+' or '-', assuming '+'\n" );
            action = FLAG_ADD;
            break;
         }

      //
      // WARNING: please change the Event decleration,
      // to match this function, if flags are added or
      // deleted the event must be updated.
      //
      if ( !stricmp( flag, "broadcast" ) )
         mask = SVF_BROADCAST;
      else if ( !stricmp( flag, "sendonce" ) )
         mask = SVF_SENDONCE;
      else
         {
         action = FLAG_IGNORE;
         ev->Error( "Unknown token %s.", flag );
         }

      switch (action)
         {
         case FLAG_ADD:
            edict->svflags |= mask;
            break;
         case FLAG_CLEAR:
            edict->svflags &= ~mask;
            break;
         case FLAG_IGNORE:
            break;
         }
      }

   if ( edict->svflags & SVF_SENDONCE )
      {
      // Turn this entity into an event if the SENDONCE flag is sent
      edict->s.eType = ET_EVENTS;
		edict->svflags &= ~SVF_SENT;
      }
   }

void Entity::BroadcastSound
	(
	float rad,
	int soundType	//Defaults to SOUNDTYPE_GENERAL
	)

	{
	if ( !( this->flags & FL_NOTARGET ) )
		{
      G_BroadcastSound( this, centroid, rad, soundType );
      }
	}

void Entity::BroadcastSound
	(
	Event *ev
	)

	{
   float rad		  = SOUND_RADIUS;
	int soundTypeIdx = SOUNDTYPE_GENERAL;
	str soundTypeStr = "";
		
	if ( !( this->flags & FL_NOTARGET ) )
		{
		
		if (ev->NumArgs() > 0 )	//<-- At least 1 Parameter
			{
			rad = ev->GetFloat( 1 );

			if(ev->NumArgs() > 1 )	//<-- At least 2 Parameters
				{
				soundTypeStr = ev->GetString( 2 );			
				soundTypeIdx = Soundtype_string_to_int( soundTypeStr );	
				}
			}
		
		if (soundTypeIdx == SOUNDTYPE_FOOTSTEPS_RUN || soundTypeIdx == SOUNDTYPE_FOOTSTEPS_WALK )
			rad = ModifyFootstepSoundRadius( rad , soundTypeIdx );


		BroadcastSound( rad, soundTypeIdx );
      }
	}

float Entity::ModifyFootstepSoundRadius
   (
	float radius,
	int soundTypeIdx
	)

	{
	trace_t	trace;
	Vector	end;
   Vector   start;

   start = origin; 
   end = origin;
	end[2]-= 1000.0f;
	int surftype;

	trace = G_Trace( start, mins, maxs, end, this, edict->clipmask, false, "Entity::ModifyFootstepsRadius" );
	surftype = trace.surfaceFlags & MASK_SURF_TYPE;
	
	switch ( surftype )
		{
		case SURF_TYPE_DIRT:
			radius *= .5f;
			break;
		case SURF_TYPE_ROCK:
			break;
		case SURF_TYPE_METAL:
			radius *= 1.5;
			break;
		case SURF_TYPE_GRILL:
			radius *= 1.25f;
			break;
		case SURF_TYPE_ORGANIC:
			radius *= .5f;
			break;
		case SURF_TYPE_SQUISHY:
			radius *= .5f;
			break;
		case SURF_TYPE_SAND:
			radius *= .5f;
			break;
		case SURF_TYPE_SNOW:
			radius *= .5f;
			break;
		case SURF_TYPE_METAL_DUCT:
			radius *= 1.25f;
			break;
		case SURF_TYPE_METAL_HOLLOW:
			radius *= 1.25f;
			break;
		case SURF_TYPE_CARPET:
			radius *= .5f;
			break;
      }
	
	//Tone Radius Down for Walking
	if (soundTypeIdx == SOUNDTYPE_FOOTSTEPS_WALK )
		radius *= .75f;

	return radius;
	}

void Entity::Think
	(
	void
	)

	{
	}

void Entity::SetWaterType
   (
   void
   )

   {
   qboolean isinwater;

   watertype = gi.pointcontents( origin, 0 );
	isinwater = watertype & MASK_WATER;

	if ( isinwater )
		{
		waterlevel = 1;
		}
	else
		{
		waterlevel = 0;
		}
   }

void Entity::DamageSkin
   (
   trace_t * trace,
   float damage
   )

   {
   /* FIXME : Do we need damage skins?
   int surface;

   // FIXME handle different bodyparts
   surface = trace->intersect.surface;
   if ( !edict->s.surfaces[ surface ] )
		{
      edict->s.surfaces[ surface ]++;
		}
   */
   }

void Entity::Kill
	(
	Event *ev
	)

	{
	health = 0.0f;
   Damage( this, this, 10.0f, origin, vec_zero, vec_zero, 0, 0, MOD_SUICIDE );
	}


void Entity::SurfaceCommand
	(
   const char * surf_name,
   const char * token
	)

	{
	const char * current_surface_name;
   int surface_num;
   int mask;
   int action;
   qboolean do_all = false;
	qboolean mult = false;


	if ( surf_name[ strlen( surf_name ) - 1 ] == '*' )
		{
		mult = true;
		surface_num = 0;
		}
	else if ( str( surf_name ) != str( "all" ) )
      {
      surface_num = gi.Surface_NameToNum( edict->s.modelindex, surf_name );

      if (surface_num < 0)
         {
		   warning( "SurfaceCommand", "group %s not found for entity %s (%d), model %s.\n", surf_name, targetname.c_str(), entnum, model.c_str() );
         return;
         }
      }
   else
      {
      surface_num = 0;
      do_all = true;
      }

   action = 0;
   switch( token[0] )
      {
      case '+':
         action = FLAG_ADD;
         token++;
         break;
      case '-':
         action = FLAG_CLEAR;
         token++;
         break;
      default:
         warning( "Entity::SurfaceModelEvent", "First character is not '+' or '-', assuming '+' for entity %s (%d), model %s\n", targetname.c_str(), entnum, model.c_str() );
         action = FLAG_ADD;
         break;
      }
   //
   // WARNING: please change the Event decleration,
   // to match this function, if flags are added or
   // deleted the event must be updated.
   //
   if (!stricmp( token, "skin1"))
      {
      mask = MDL_SURFACE_SKINOFFSET_BIT0;
      }
   else if (!strcmpi (token, "skin2"))
      {
      mask = MDL_SURFACE_SKINOFFSET_BIT1;
      }
   else if (!strcmpi (token, "nodraw"))
      {
      mask = MDL_SURFACE_NODRAW;
      }
   else if (!strcmpi (token, "crossfade"))
      {
      mask = MDL_SURFACE_CROSSFADE_SKINS;
      }
   else
      {
      mask = 0;
      warning( "SurfaceCommand", "Unknown token %s. for entity %s (%d), model %s", token, targetname.c_str(), entnum, model.c_str() );
      action = FLAG_IGNORE;
      }
   for( ; surface_num < numsurfaces ; surface_num++ )
      {
		if ( mult )
			{
			current_surface_name = gi.Surface_NumToName( edict->s.modelindex, surface_num );

			if ( Q_stricmpn( current_surface_name, surf_name, strlen( surf_name ) - 1) != 0 )
				continue;
			}

      switch (action)
         {
         case FLAG_ADD:
            edict->s.surfaces[ surface_num ] |= mask;
            break;
         case FLAG_CLEAR:
            edict->s.surfaces[ surface_num ] &= ~mask;
            break;
         case FLAG_IGNORE:
            break;
         }

      if ( !do_all && !mult )
         break;
      }
	}

void Entity::SurfaceModelEvent
	(
	Event *ev
	)

	{
   const char * surf_name;
   const char * token;
   int i;

   surf_name = ev->GetString( 1 );

   for ( i = 2; i <= ev->NumArgs() ; i++ )
      {
      token = ev->GetString( i );
      SurfaceCommand( surf_name, token );
      }
	}

void Entity::AttachEvent
	(
	Event * ev
	)
	{
   Entity * parent;
   const char * bone;
   int tagnum;
	qboolean use_angles = true;
	Vector offset;
	Vector angles_offset;

	parent = ev->GetEntity( 1 );
   bone = ev->GetString( 2 );

	if ( ev->NumArgs() > 2 )
		use_angles = ev->GetInteger( 3 );

	if ( ev->NumArgs() > 3 )
		offset = ev->GetVector( 4 );

	if ( ev->NumArgs() > 4 )
		angles_offset = ev->GetVector( 5 );

   if ( !parent )
      return;

   tagnum = gi.Tag_NumForName( parent->edict->s.modelindex, bone );
   if ( tagnum >= 0 )
      {
      attach( parent->entnum, tagnum, use_angles, offset, angles_offset );
      }
   else
      {
      warning( "AttachEvent", "Tag %s not found", bone );
      }
   }

void Entity::AttachModelEvent
	(
	Event * ev
	)
	{
   Entity * obj;
   const char * bone;
   str modelname;
   int tagnum;
	float remove_time,fade_time,fade_delay;
	Vector offset;
	Vector angles_offset;
	qboolean use_angles = false;

   obj = new Entity( ENTITY_CREATE_FLAG_ANIMATE );

	obj->bind_info = CreateBindInfo();

	modelname = ev->GetString( 1 );
   bone = ev->GetString( 2 );
   if ( ev->NumArgs() > 2 )
		{
      obj->setScale( ev->GetFloat( 3 ) );
		}
   if ( ev->NumArgs() > 3 )
		{
      obj->SetTargetName( ev->GetString( 4 ) );
		}

	if ( ev->NumArgs() > 4 )
      obj->bind_info->detach_at_death = ev->GetInteger( 5 );

	if ( ev->NumArgs() > 5 )
		{
		remove_time = ev->GetFloat( 6 );

		if ( remove_time > 0.0f )
			{
			Event *remove_event = new Event( EV_Remove );
			obj->PostEvent( remove_event, remove_time );
			}
		}

	if ( ev->NumArgs() > 6 )
		{
		Event *fade_event;

      fade_time = ev->GetFloat( 7 );

      if ( fade_time > 0.0f )
         {
   		obj->setAlpha( 0.0f );

	   	fade_event = new Event( EV_FadeIn );
		   fade_event->AddFloat( fade_time );
		   obj->PostEvent( fade_event, 0.0f );
         }
		}

 	if ( ev->NumArgs() > 7 )
		{
		Event *fade_event;

      fade_delay = ev->GetFloat( 8 );

      if ( fade_delay != -1.0f )
         {
         if ( ev->NumArgs() > 8 )
            fade_time = ev->GetFloat( 9 );
         else
            fade_time = 0.0f;

	   	fade_event = new Event( EV_Fade );

         if ( fade_time > 0.0f )
            fade_event->AddFloat( fade_time );

		   obj->PostEvent( fade_event, fade_delay );
         }
		}

	if ( ev->NumArgs() > 9 )
		offset = ev->GetVector( 10 );

	if ( ev->NumArgs() > 10 )
		{
		angles_offset = ev->GetVector( 11 );
		use_angles = false;
		}

   obj->setModel( modelname );
	
	if ( !obj->animate )
		{
		Animate *newAnimate = 0;
		newAnimate = new Animate;
		if ( newAnimate )
			obj->animate = newAnimate;
		}
				
	int anim_num = gi.Anim_Random ( obj->edict->s.modelindex, "idle" );
	if ( anim_num != -1 && obj->animate )
		{
		obj->animate->NewAnim( anim_num );
		}

   tagnum = gi.Tag_NumForName( edict->s.modelindex, bone );
   if ( tagnum >= 0 )
      {
      if ( !obj->attach( this->entnum, tagnum, use_angles, offset, angles_offset ) )
         {
         //warning( "AttachModelEvent", "Could not attach model %s", modelname.c_str() );
         delete obj;
         return;
         }
      }
   else
      {
      warning( "AttachModelEvent", "Tag %s not found", bone );
      }
   }

void Entity::RemoveAttachedModelEvent
	(
	Event *ev
	)
	{
   const char *tag_name;
   int tag_num;
	int num;
	int i;
	Entity *ent;
	float fade_rate = 0;
	Event *fade_event;
	str model_name;

	if ( bind_info )
		{
		tag_name = ev->GetString( 1 );
		tag_num = gi.Tag_NumForName( edict->s.modelindex, tag_name );

		if ( ev->NumArgs() > 1 )
			fade_rate = ev->GetFloat( 2 );

		if ( ev->NumArgs() > 2 )
			model_name = ev->GetString( 3 );

		if ( tag_num >= 0 )
			{
			num = bind_info->numchildren;

			for ( i = 0 ; (i < MAX_MODEL_CHILDREN) && num ; i++ )
				{
				if ( bind_info->children[i] == ENTITYNUM_NONE )
					continue;

				ent = ( Entity * )G_GetEntity( bind_info->children[i] );

				if ( ent && ent->edict->s.tag_num == tag_num )
					{
					if ( !model_name.length() || ( model_name == ent->model ) )
						{
						if ( fade_rate )
							{
							fade_event = new Event( EV_Fade );
							fade_event->AddFloat( fade_rate );
							fade_event->AddFloat( 0.0f );
							ent->PostEvent( fade_event, 0.0f );
							}

						ent->PostEvent( EV_Remove, fade_rate );
						}
					}

				num--;
				}
			}
		}
   }

void Entity::removeAttachedModelByTargetname( Event *ev )
{
	str targetNameToRemove;

	targetNameToRemove = ev->GetString( 1 );

	removeAttachedModelByTargetname( targetNameToRemove );
}

void Entity::removeAttachedModelByTargetname( const str &targetNameToRemove )
{
	int num;
	int i;
	Entity *ent;

	if ( bind_info )
	{	
		num = bind_info->numchildren;
		
		for ( i = 0 ; (i < MAX_MODEL_CHILDREN) && num ; i++ )
		{
			if ( bind_info->children[i] == ENTITYNUM_NONE )
				continue;
			
			ent = ( Entity * )G_GetEntity( bind_info->children[i] );
			
			if ( ent && stricmp( ent->targetname, targetNameToRemove.c_str() ) == 0 )
			{
				ent->PostEvent( EV_Remove, 0.0f );
			}
			
			num--;
		}
	}
}

void Entity::DetachEvent
	(
	Event * ev
	)

	{
   if ( edict->s.parent == ENTITYNUM_NONE  )
		{
      return;
		}
   detach();
   }

void Entity::TakeDamageEvent
	(
	Event * ev
	)
	{
   takedamage = DAMAGE_YES;
   }

void Entity::NoDamageEvent
	(
	Event * ev
	)
	{
   takedamage = DAMAGE_NO;
   }

void Entity::Gravity
   (
   Event *ev
   )

   {
   gravity = ev->GetFloat( 1 );
   }

void Entity::UseBoundingBoxEvent
   (
   Event *ev
   )
   {
	edict->svflags |= SVF_USEBBOX;
   }

void Entity::HurtEvent
   (
   Event *ev
   )
   {
   Vector normal;
   float dmg;
	int means_of_death;
	Vector direction;

   if ( ev->NumArgs() < 1 )
      {
      dmg = 50.0f;
      }
   else
      {
      dmg = ev->GetFloat( 1 );
      }

	if ( ev->NumArgs() > 1 )
		means_of_death = MOD_NameToNum( ev->GetString( 2 ) );
	else
		means_of_death = MOD_CRUSH;

	if ( ev->NumArgs() > 2 )
		{
		direction = ev->GetVector( 3 );
		direction.normalize();
		}
	else
		{
		direction = vec_zero;
		}

   normal = Vector( orientation[ 0 ] );
   Damage( world, world, dmg, centroid, direction, normal, (int)dmg, 0, means_of_death );
   }

void Entity::IfSkillEvent
	(
	Event *ev
	)

	{
   float skilllevel;

	skilllevel = ev->GetFloat( 1 );

   if ( skill->value == skilllevel )
      {
	   int			argc;
	   int			numargs;
      Event       *event;
	   int			i;

	   numargs = ev->NumArgs();
	   argc = numargs - 2 + 1;

      event = new Event( ev->GetToken( 2 ) );

	   for( i = 1; i < argc; i++ )
		   {
         event->AddToken( ev->GetToken( 2 + i ) );
		   }
      ProcessEvent( event );
		}
	}

void Entity::Censor
	(
	Event *ev
	)

	{
   Vector delta;
   float oldsize;
   float newsize;

   if ( com_blood->integer )
      return;

   oldsize = size.length();
	setSolidType( SOLID_NOT );
   setModel( "censored.tik" );
   gi.CalculateBounds( edict->s.modelindex, 1.0f, mins, maxs );
   delta = maxs - mins;
   newsize = delta.length();
   edict->s.scale = oldsize / newsize;
   mins *= edict->s.scale;
   maxs *= edict->s.scale;
   setSize( mins, maxs );
   setOrigin();
	}

void Entity::StationaryEvent
   (
   Event *ev
   )

	{
	setMoveType( MOVETYPE_STATIONARY );
	}

void Entity::Explosion
   (
   Event *ev
   )

   {
   str expmodel;
	str tag_name;
	//orientation_t orient;
	Vector explosion_origin;

	expmodel = ev->GetString( 1 );
	explosion_origin = origin;

	if ( ev->NumArgs() > 1 )
		{
		tag_name = ev->GetString( 2 );

		//if ( GetRawTag( tag_name.c_str(), &orient, legs ) )
		//	VectorAdd( orient.origin, origin, explosion_origin );

		GetTag( tag_name.c_str(), &explosion_origin );
		}

   ExplosionAttack( explosion_origin, this, expmodel );
   }

//----------------------------------------------------------------
// Name:			DoRadiusDamage
// Class:			Entity
//
// Description:		Does radius damage from entity origin,
//					optionally re-posting itself to some future time
//
// Parameters:		Event *ev
// Event params:	(1) float damage
//					(2) const char * means of death string
//					(3) float radius
//					(4) float knockback
//					(5) (optional) bool constant damage over distance
//					(6) (optional) float forward re-posting time
//
// Returns:			NULL
//----------------------------------------------------------------

void Entity::DoRadiusDamage( Event *ev )
{
	Entity *owner = this;
	float damage = ev->GetFloat(1);
	const char *modstring = ev->GetString(2);
	float radius = ev->GetFloat(3);
	float knockback = ev->GetFloat(4);
	bool constant_damage = false;
	float damageDone;

	if (ev->NumArgs() >= 5) // constant damage info is supplied
		constant_damage = ev->GetBoolean(5);

	if ( this->isSubclassOf( Projectile ) )
	{
		Projectile *projectile = (Projectile *)this;

		owner = projectile->getOwner();
	}

	if ( owner && owner->isSubclassOf( Player ) )
	{
		Player *player = (Player *)owner;
		
		damage = player->getDamageDone( damage, MOD_NameToNum( modstring ), false );
	}

	damageDone = RadiusDamage(	this,
					owner,
					damage,
					this,
					MOD_NameToNum(modstring),
					radius,
					knockback,
					constant_damage
					);

	if ( damageDone && this->isSubclassOf( Projectile ) )
	{
		Projectile *projectile = (Projectile *)this;

		projectile->didDamage();
	}

	if (ev->NumArgs() == 6)
	{
		// if repost time is set, re-post this event
		float postTime = ev->GetFloat(6);
		if (postTime >= FRAMETIME) 
		{
			Event *ev2 = new Event(ev);
			PostEvent(ev2,postTime);
		}	
	}
}

void Entity::SelfDetonate
   (
	Event *ev
	)
	{
	if ( explosionModel.length() == 0 )
		explosionModel = "fx/fx-sml-exp.tik";

	ExplosionAttack( origin , this , explosionModel );	
	}

void Entity::Shader
	(
	Event *ev
	)

	{
   const char * token;

   if ( gi.IsModel( edict->s.modelindex ) )
      {
      ev->Error( "shader event being called on TIKI model\n" );
      }
   //
   // get sub shader command
   //
   token = ev->GetString( 1 );

   //
   // WARNING: please change the Event decleration,
   // to match this function, if flags are added or
   // deleted the event must be updated.
   //
   if (!strcmpi( token, "translation"))
      {
      float x, y;

      x = ev->GetFloat( 2 );
      y = ev->GetFloat( 3 );
      TRANSLATION_TO_PKT( (int)x, edict->s.tag_num );
      TRANSLATION_TO_PKT( (int)y, edict->s.skinNum );
      }
   else if (!strcmpi( token, "offset"))
      {
      float x, y;

      x = ev->GetFloat( 2 );
      y = ev->GetFloat( 3 );
      OFFSET_TO_PKT( x, edict->s.tag_num );
      OFFSET_TO_PKT( y, edict->s.skinNum );
      }
   else if (!strcmpi (token, "rotation"))
      {
      float rot;

      rot = ev->GetFloat( 2 );
      ROTATE_TO_PKT( rot, edict->s.tag_num );
      }
   else if (!strcmpi (token, "frame"))
      {
      edict->s.frame = ev->GetInteger( 2 );
      }
   else if (!strcmpi (token, "wavebase"))
      {
      float base;

      base = ev->GetFloat( 2 );
      BASE_TO_PKT( base, edict->s.surfaces[ 0 ] );
      }
   else if (!strcmpi (token, "waveamp"))
      {
      float amp;

      amp = ev->GetFloat( 2 );
      AMPLITUDE_TO_PKT( amp, edict->s.surfaces[ 1 ] );
      }
   else if (!strcmpi (token, "wavephase"))
      {
      float phase;

      phase = ev->GetFloat( 2 );
      PHASE_TO_PKT( phase, edict->s.surfaces[ 2 ] );
      }
   else if (!strcmpi (token, "wavefreq"))
      {
      float freq;

      freq = ev->GetFloat( 2 );
      FREQUENCY_TO_PKT( freq, edict->s.surfaces[ 3 ] );
      }
	}

void Entity::DropToFloorEvent
   (
   Event *ev
   )

   {
   float range;

	if ( ev->NumArgs() > 0 )
      {
      range = ev->GetFloat( 1 );
      }
   else
      {
      range = WORLD_SIZE;
      }
   if ( !droptofloor( range ) )
      {
      }
	}


//*************************************************************************
//
// BIND code
//
//*************************************************************************

qboolean Entity::isBoundTo
   (
   const Entity *master
   )

   {
   Entity *ent;

	if ( bind_info )
		{
		for( ent = bind_info->bindmaster; ent != NULL; ent = ent->bind_info->bindmaster )
			{
			if ( ent == master )
				{
				return true;
				}
			}
		}

   return false;
   }

void Entity::bind
	(
	Entity *master,
	qboolean use_my_angles
	)

	{
	float  mat[ 3 ][ 3 ];
	float  local[ 3 ][ 3 ];
	Vector ang;

	assert( master );
	if ( !master )
		{
		warning( "bind", "Null master entity" );
		return;
		}

   if ( master == this )
      {
      warning( "bind", "Trying to bind to oneself." );
      return;
      }

	if ( !bind_info )
		bind_info = CreateBindInfo();

   // unbind myself from my master
   unbind();

	bind_info->bindmaster = master;
	edict->s.bindparent = master->entnum;
	bind_info->bind_use_my_angles = use_my_angles;

   // We are now separated from our previous team and are either
   // an individual, or have a team of our own.  Now we can join
   // the new bindmaster's team.  Bindmaster must be set before
   // joining the team, or we will be placed in the wrong position
   // on the team.
   joinTeam( master );

	// calculate local angles
	TransposeMatrix( bind_info->bindmaster->orientation, mat );
	R_ConcatRotations( mat, orientation, local );
	MatrixToEulerAngles( local, ang );
	setAngles( ang );

   setOrigin( getParentVector( GetLocalOrigin() - bind_info->bindmaster->origin ) );

   return;
   }

void Entity::unbind
	(
	void
	)

	{
   Entity *prev;
   Entity *next;
   Entity *last;
   Entity *ent;


	if ( !bind_info || !bind_info->bindmaster )
		return;

	//bindmaster = NULL;

   // Check this GAMEFIX - should it be origin?
	SetLocalOrigin( edict->s.origin );
	localangles = Vector( edict->s.angles );

   if ( !bind_info->teammaster )
      {
      bind_info->bindmaster = NULL;
		edict->s.bindparent = ENTITYNUM_NONE;
      //Teammaster already has been freed
      return;
      }

   // We're still part of a team, so that means I have to extricate myself
   // and any entities that are bound to me from the old team.
   // Find the node previous to me in the team
   prev = bind_info->teammaster;

	for( ent = bind_info->teammaster->bind_info->teamchain; ent && ( ent != this ); ent = ent->bind_info->teamchain )
		{
      prev = ent;
		}

   // If ent is not pointing to me, then something is very wrong.
   assert( ent );
   if ( !ent )
      {
      error( "unbind", "corrupt team chain\n" );
      }

   // Find the last node in my team that is bound to me.
   // Also find the first node not bound to me, if one exists.
   last = this;
   for( next = bind_info->teamchain; next != NULL; next = next->bind_info->teamchain )
      {
      if ( !next->isBoundTo( this ) )
         {
         break;
         }

      // Tell them I'm now the teammaster
      next->bind_info->teammaster = this;
      last = next;
		}

   // disconnect the last member of our team from the old team
   last->bind_info->teamchain = NULL;

   // connect up the previous member of the old team to the node that
   // follow the last node bound to me (if one exists).
   if ( bind_info->teammaster != this )
      {
      prev->bind_info->teamchain = next;
      if ( !next && ( bind_info->teammaster == prev ) )
         {
         prev->bind_info->teammaster = NULL;
         }
      }
   else if ( next )
      {
      // If we were the teammaster, then the nodes that were not bound to me are now
      // a disconnected chain.  Make them into their own team.
		for( ent = next; ent->bind_info->teamchain != NULL; ent = ent->bind_info->teamchain )
         {
         ent->bind_info->teammaster = next;
			}
      next->bind_info->teammaster = next;
	   next->flags &= ~FL_TEAMSLAVE;
      }

   // If we don't have anyone on our team, then clear the team variables.
   if ( bind_info->teamchain )
      {
      // make myself my own team
      bind_info->teammaster = this;
      }
   else
      {
      // no longer a team
      bind_info->teammaster = NULL;
      }

	flags &= ~FL_TEAMSLAVE;
   bind_info->bindmaster = NULL;
	edict->s.bindparent = ENTITYNUM_NONE;
	}

void Entity::EventUnbind
	(
	Event *ev
	)

	{
	unbind();
	}

void Entity::BindEvent
	(
	Event *ev
	)

	{
	Entity *ent;

	ent = ev->GetEntity( 1 );
	if ( ent )
		{
		bind( ent );
		}
	}


Vector Entity::getParentVector
	(
	const Vector &vec
	)

	{
	Vector pos;

	if ( !bind_info || !bind_info->bindmaster )
		{
		return vec;
		}

	pos[ 0 ] = vec * bind_info->bindmaster->orientation[ 0 ];
	pos[ 1 ] = vec * bind_info->bindmaster->orientation[ 1 ];
	pos[ 2 ] = vec * bind_info->bindmaster->orientation[ 2 ];

	return pos;
	}

//
// Team methods
//

void Entity::joinTeam
	(
	Entity *teammember
	)

	{
	Entity *ent;
   Entity *master;
   Entity *prev;
   Entity *next;

	if ( !bind_info )
		bind_info = CreateBindInfo();

	if ( bind_info->teammaster && ( bind_info->teammaster != this ) )
		{
		quitTeam();
		}

	assert( teammember );
	if ( !teammember )
		{
		warning( "joinTeam", "Null entity" );
		return;
		}

	if ( !teammember->bind_info )
		teammember->bind_info = CreateBindInfo();

	master = teammember->bind_info->teammaster;
	if ( !master )
		{
		master = teammember;
		teammember->bind_info->teammaster = teammember;
      teammember->bind_info->teamchain = this;

      // make anyone who's bound to me part of the new team
      for( ent = bind_info->teamchain; ent != NULL; ent = ent->bind_info->teamchain )
         {
         ent->bind_info->teammaster = master;
         }
      }
   else
      {
      // skip past the chain members bound to the entity we're teaming up with
      prev = teammember;
	   next = teammember->bind_info->teamchain;
      if ( bind_info->bindmaster )
         {
         // if we have a bindmaster, joing after any entities bound to the entity
         // we're joining
	      while( next && (( Entity *)next)->isBoundTo( teammember ) )
		      {
            prev = next;
		      next = next->bind_info->teamchain;
		      }
         }
      else
         {
         // if we're not bound to someone, then put us at the end of the team
	      while( next )
		      {
            prev = next;
		      next = next->bind_info->teamchain;
		      }
         }

      // make anyone who's bound to me part of the new team and
      // also find the last member of my team
      for( ent = this; ent->bind_info->teamchain != NULL; ent = ent->bind_info->teamchain )
         {
         ent->bind_info->teamchain->bind_info->teammaster = master;
         }

    	prev->bind_info->teamchain = this;
      ent->bind_info->teamchain = next;
      }

   bind_info->teammaster = master;
	flags |= FL_TEAMSLAVE;
	}

void Entity::quitTeam
	(
	void
	)

	{
	Entity *ent;

	if ( !bind_info || !bind_info->teammaster )
		{
		return;
		}

	if ( bind_info->teammaster == this )
		{
      if ( !bind_info->teamchain->bind_info->teamchain )
         {
         bind_info->teamchain->bind_info->teammaster = NULL;
         }
      else
         {
		   // make next teammate the teammaster
		   for( ent = bind_info->teamchain; ent; ent = ent->bind_info->teamchain )
			   {
			   ent->bind_info->teammaster = bind_info->teamchain;
			   }
         }

      bind_info->teamchain->flags &= ~FL_TEAMSLAVE;
		}
	else
		{
		assert( flags & FL_TEAMSLAVE );
		assert( bind_info->teammaster->bind_info->teamchain );

		ent = bind_info->teammaster;
		while( ent->bind_info->teamchain != this )
			{
			// this should never happen
			assert( ent->bind_info->teamchain );

			ent = ent->bind_info->teamchain;
			}

		ent->bind_info->teamchain = bind_info->teamchain;

		if ( !bind_info->teammaster->bind_info->teamchain )
			{
			bind_info->teammaster->bind_info->teammaster = NULL;
			}
		}

	bind_info->teammaster = NULL;
	bind_info->teamchain = NULL;
	flags &= ~FL_TEAMSLAVE;
	}

void Entity::EventQuitTeam
	(
	Event *ev
	)

	{
	quitTeam();
	}


void Entity::JoinTeam
	(
	Event *ev
	)

	{
	Entity *ent;

	ent = ev->GetEntity( 1 );
	if ( ent )
		{
		joinTeam( ent );
		}
	}

void Entity::AddToSoundManager
	(
	Event *ev
	)

	{
   SoundMan.AddEntity( this );
	}

inline qboolean Entity::HitSky
	(
	const trace_t *trace
	)

	{
	assert( trace );
   if ( trace->surfaceFlags & SURF_SKY )
		{
		return true;
		}
	return false;
	}

qboolean Entity::HitSky
	(
	void
	)

	{
	return HitSky( &level.impact_trace );
	}

void Entity::SetAngleEvent
	(
	Event *ev
	)
	{
   Vector movedir;

   movedir = G_GetMovedir( ev->GetFloat( 1 ) );
	setAngles( movedir.toAngles() );
   }

void Entity::NoLerpThisFrame( void )
{
	gentity_t *checkEdict;

	edict->s.eFlags ^= EF_TELEPORT_BIT;

	// Make sure no one is standing on us

	for( checkEdict = active_edicts.next ; checkEdict != &active_edicts ; checkEdict = checkEdict->next )
	{
		assert( checkEdict );
		assert( checkEdict->inuse );
		
		if ( checkEdict->entity && checkEdict->entity->groundentity == edict )
		{
			checkEdict->entity->groundentity = NULL;
		}
	}
}

void Entity::Postthink
	(
	void
	)

	{
	}


//-----------------------------------------------------
//
// Name:		TouchTriggersEvent
// Class:		Entity
//
// Description:	Specifies the entity can touch triggers by setting
//				the touch triggers flag.
//
// Parameters:	ev - the event that specifies whether to turn
//				touch triggers on or off. If no param is specified, 
//				touchtriggers is true.
//
// Returns:		None
//-----------------------------------------------------
void Entity::TouchTriggersEvent(Event *ev)
{
	if ( ( ev->NumArgs() == 0 ) || ( ev->GetBoolean( 1 ) == true ) )
	{
		flags |= FL_TOUCH_TRIGGERS;
		turnThinkOn();
	}
	else
	{
		flags &= ~FL_TOUCH_TRIGGERS;
	}

}

void Entity::IncreaseShotCount 
   (
	Event *ev
	)
	{
	int parent_ent_num = edict->s.parent;
	Entity *parent = ( Entity * )G_GetEntity( parent_ent_num );

	if ( !parent->isSubclassOf(Actor) )
		return;
   
	Actor* act;
	act = (Actor*)parent;

	act->shotsFired++;	
	}

void Entity::DeathSinkStart
	(
	Event *ev
	)
	{
	float time;

	// Stop the sink when we can't be seen anymore

	if ( ( maxs[2] >= 0.0f ) && ( maxs[2] < 200.0f ) )
		time = maxs[2] / 20.0f;
	else
		time = 1.0f;

	PostEvent( EV_Remove, time );

	// Start the sinking

	ProcessEvent( EV_DeathSink );
	}

void Entity::DeathSink
	(
	Event *ev
	)
	{
	// Sink just a little

	origin[2] -= 1.0f;
	setOrigin( origin );

	// Make sure the sink happens again next frame

	PostEvent( EV_DeathSink, FRAMETIME );
	}

void Entity::LookAtMe
	(
	Event *ev
	)
	{
	if ( ev->NumArgs() > 0 )
		look_at_me = ev->GetBoolean( 1 );
	else
		look_at_me = true;
	}

void Entity::ProjectilesCanStickToMe( Event *ev )
{
	projectilesCanStickToMe = ev->GetBoolean( 1 );
}

void Entity::VelocityModified
	(
   void
	)
	{
	}

//--------------------------------------------------------------
//
// Name:			DetachAllChildren
// Class:			Entity
//
// Description:		Detaches all attached models to this entity
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Entity::DetachAllChildren(Event *ev)
	{
	if ( !bind_info ) // Abort if bind_info is NULL for some reason
		return;

	for ( int i=0; i<MAX_MODEL_CHILDREN; i++)
		{
		if ( bind_info->children[i] == ENTITYNUM_NONE )
			continue;

		Entity *ent = ( Entity * )G_GetEntity( bind_info->children[i] );
		if ( ent )
			ent->PostEvent( EV_Remove, 0.0f );
		}
	}

inline void Entity::Archive( Archiver &arc )
{
	int tempInt;
	qboolean is_archived;
	qboolean true_bool = true;
	qboolean false_bool = false;


	Listener::Archive( arc );

	arc.ArchiveVector( &_localOrigin );

	_lastTouchedList.Archive( arc );

	arc.ArchiveBool( &_fulltrace );
	arc.ArchiveInteger( &_groupID );

	// Don't archive entnum, it will be set elsewhere
	//int					entnum;

	G_ArchiveEdict( arc, edict );

	arc.ArchiveString(&_archetype);
	if(arc.Loading())
	{
		edict->s.archeTypeIndex = gi.archetypeindex(_archetype);
	}

	arc.ArchiveBool( &_missionObjective );
	arc.ArchiveString( &_targetPos );
	arc.ArchiveBool( &_networkDetail );

	if(arc.Loading())
	{
		edict->s.missionObjective = _missionObjective;
		if(_missionObjective == true)
			G_AddEntityToExtraList(entnum);
	}

	// Don't archive client
	//gclient_t			*client;

	arc.ArchiveInteger( &spawnflags );

	arc.ArchiveString( &model );

	if ( arc.Loading() && model.length() )
      setModel( model.c_str() );

	arc.ArchiveVector( &total_delta );
	arc.ArchiveVector( &mins );
	arc.ArchiveVector( &maxs );
	arc.ArchiveVector( &absmin );
	arc.ArchiveVector( &absmax );
	arc.ArchiveVector( &centroid );
	arc.ArchiveVector( &velocity );
	arc.ArchiveVector( &avelocity );
	arc.ArchiveVector( &origin );
	arc.ArchiveVector( &angles );
	arc.ArchiveVector( &size );
	arc.ArchiveInteger( &movetype );
	arc.ArchiveInteger( &mass );
	arc.ArchiveFloat( &gravity );
	arc.ArchiveRaw( orientation, sizeof( orientation ) );

	arc.ArchiveVector( &localangles );

	if ( arc.Saving() )
	{
		if ( groundentity )
		{
			tempInt = groundentity - g_entities;
		}
		else
		{
			tempInt = -1;
		}
	}

	arc.ArchiveInteger( &tempInt );

	if ( arc.Loading() )
	{
		if ( tempInt == -1 )
		{
			groundentity = NULL;
		}
		else
		{
		groundentity = &g_entities[ tempInt ];
		}
	}

	arc.ArchiveRaw( &groundplane, sizeof( groundplane ) );
	arc.ArchiveInteger( &groundcontents );

	arc.ArchiveInteger( &numsurfaces );

	arc.ArchiveFloat( &lightRadius );

	arc.ArchiveString( &target );
	arc.ArchiveString( &targetname );
	arc.ArchiveString( &killtarget );

	if ( arc.Loading() )
	{
		// Don't set this here, they are handled in the world
		//SetTargetName( targetname.c_str() );

		// Not needed
		//SetTarget( target.c_str() );
	}

	arc.ArchiveFloat( &health );
	arc.ArchiveFloat( &max_health );
	arc.ArchiveInteger( &deadflag );
	arc.ArchiveInteger( &flags );

	arc.ArchiveInteger( &watertype );
	arc.ArchiveInteger( &waterlevel );

	ArchiveEnum( takedamage, damage_t );
	arc.ArchiveInteger( &damage_type );

	arc.ArchiveBoolean( &look_at_me );
	arc.ArchiveBool( &projectilesCanStickToMe );

	arc.ArchiveString( &explosionModel );

	entityVars.Archive( arc );

	arc.ArchiveUnsigned( &_affectingViewModes );

	arc.ArchiveVector( &watch_offset );

	//  Pluggable modules

	if ( arc.Loading() )
	{
		arc.ArchiveBoolean( &is_archived );

		if ( is_archived )
		{
			animate = new Animate( this );
			arc.ArchiveObject( animate );
		}

		arc.ArchiveBoolean( &is_archived );

		if ( is_archived )
		{
			mover = new Mover( this );
			arc.ArchiveObject( mover );
		}

		arc.ArchiveBoolean( &is_archived );

		if ( is_archived )
		{
			bind_info = CreateBindInfo();
			bind_info->Archive( arc );
		}

		arc.ArchiveBoolean( &is_archived );

		if ( is_archived )
		{
			morph_info = CreateMorphInfo();
			morph_info->Archive( arc );
		}
	}
	else
	{
		if ( animate )
		{
			arc.ArchiveBoolean( &true_bool );
			arc.ArchiveObject( animate );
		}
		else
		{
			arc.ArchiveBoolean( &false_bool );
		}

		if ( mover )
		{
			arc.ArchiveBoolean( &true_bool );
			arc.ArchiveObject( mover );
		}
		else
		{
			arc.ArchiveBoolean( &false_bool );
		}

		if ( bind_info )
		{
			arc.ArchiveBoolean( &true_bool );
			bind_info->Archive( arc );
		}
		else
		{
			arc.ArchiveBoolean( &false_bool );
		}

		if ( morph_info )
		{
			arc.ArchiveBoolean( &true_bool );
			morph_info->Archive( arc );
		}
		else
		{
			arc.ArchiveBoolean( &false_bool );
		}
	}

	if ( arc.Saving() )
	{
		if ( ObjectProgram )
		{
			arc.ArchiveBoolean( &true_bool );
			ObjectProgram->Archive( arc );
		}
		else
		{
			arc.ArchiveBoolean( &false_bool );
		}
	}
	else
	{
		arc.ArchiveBoolean( &is_archived );

		if ( is_archived )
			ObjectProgram->Archive( arc );
		else
			ObjectProgram = NULL;
	}

	// Archiving of the Damage Modification System
	if ( arc.Saving() )
		{
		bool exists;
		int type;
		if ( damageModSystem )
			{
			exists = true;
			arc.ArchiveBool( &exists );

			int numobj = damageModSystem->getModifierList().NumObjects();
			arc.ArchiveInteger( &numobj );
			for ( tempInt = 1; tempInt <= numobj; tempInt++ )
				{
				DamageModifier *dmod = damageModSystem->getModifierList().ObjectAt ( tempInt );
				if ( dmod )
					{
					type = (int)dmod->getType();
					arc.ArchiveInteger( &type );
					dmod->Archive(arc);
					}
				else
					{
					// How did a NULL get in the container??
					assert( 0 );
					}
				}
			}
		else
			{
			exists = false;
			arc.ArchiveBool( &exists );
			}
		}
	else // Loading
	{
		bool exists;
		int numobj, type;
		arc.ArchiveBool( &exists );
		if ( exists )
		{
			damageModSystem = new DamageModificationSystem();
			arc.ArchiveInteger( &numobj );
			for ( tempInt = 1; tempInt <= numobj; tempInt++ )
			{
				arc.ArchiveInteger( &type );
				DamageModifier *newMod = 0;
				switch ( type )
				{
					case TIKI_NAME:
						newMod = new DamageModifierTikiName();
						break;
					case NAME:
						newMod = new DamageModifierName();
						break;
					case GROUP:
						newMod = new DamageModifierGroup();
						break;
					case ACTOR_TYPE:
						newMod = new DamageModifierActorType();
						break;
					case TARGETNAME:
						newMod = new DamageModifierTargetName();
						break;
					case DAMAGE_TYPE:
						newMod = new DamageModifierDamageType();
						break;
				}

				if ( newMod )
				{
					newMod->Archive(arc);

					damageModSystem->addDamageModifier( newMod );
				}
			}
		}
	}

	// Archive the useData member if it exists
	if ( arc.Saving() )
		{
		bool exists = false;
		if ( useData )
			{
			exists = true;
			arc.ArchiveBool( &exists );
			useData->Archive(arc);
			}
		else
			arc.ArchiveBool( &exists );
		}
	else // Loading
		{
		bool exists;
		arc.ArchiveBool( &exists );
		if ( exists )
			{
			useData = new UseData();
			useData->Archive(arc);
			}
		}


	
}

// Animate interface

inline int Entity::CurrentFrame
	(
   bodypart_t part
	)

	{
	if ( animate )
		return animate->CurrentFrame( part );
	else
		return 0;
	}

inline int Entity::CurrentAnim
	(
   bodypart_t part
	)

	{
	if ( animate )
		return animate->CurrentAnim( part );
	else
		return 0;
	}

void Entity::PassToAnimate
	(
	Event *ev
	)

	{
	Event *new_event;

	if ( !animate )
		animate = new Animate( this );

	new_event = new Event( ev );
	animate->ProcessEvent( new_event );
	}

void Entity::SetObjectProgram
   (
	Event *ev
	)
	{
	ObjectProgram = new Program;
	
	if ( !ObjectProgram )
		return;

	ObjectProgram->Load( ev->GetString( 1 )  );


	//CThread *gamescript = 0;

	//gamescript = Director.CreateThread( "obj_main" , ObjectProgram );
	//gamescript->DelayedStart( 0 );

	}

void Entity::SetWatchOffset( Event *ev )
{
	watch_offset = ev->GetVector( 1 );
}


void Entity::ExecuteProgram
   (
	Event *ev
	)
	{
	float exeTime = 0;
	
	if ( !ObjectProgram )
		return;

	if ( ev->NumArgs() > 0 )
		exeTime = ev->GetFloat( 1 );

	CThread *gamescript = 0;
	gamescript = Director.CreateThread( "obj_main" , ObjectProgram );
	gamescript->DelayedStart( exeTime );
	}


// BindInfo interface

inline BindInfo *CreateBindInfo( void )
	{
	BindInfo *new_bind_info;

	new_bind_info = new BindInfo;

	if ( !new_bind_info )
		gi.Error( ERR_DROP, "Couldn't alloc BindInfo" );

	return new_bind_info;
	}

// MorphInfo interface

void Entity::MorphEvent
	(
	Event *ev
	)
	{
	str	morph_target_name;
	int	morph_index;
	float	final_percent = 100;
	float	morph_time = 0.5;
	qboolean return_to_zero = false;
	int i;
	qboolean override = true;
	int morph_channel = MORPH_CHAN_NONE;
	qboolean channel_being_used;
	qboolean override_all = false;
	qboolean matching_channel;
	qboolean unmorph = false;

	// Get parms

	morph_target_name = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		final_percent = ev->GetFloat( 2 );
	if ( ev->NumArgs() > 2 )
		morph_time = ev->GetFloat( 3 );
	if ( ev->NumArgs() > 3 )
		return_to_zero = ev->GetBoolean( 4 );
	if ( ev->NumArgs() > 4 )
		override = ev->GetBoolean( 5 );
	if ( ev->NumArgs() > 5 )
		morph_channel = ev->GetInteger( 6 );

	// See if this is an expression

	if ( strnicmp( morph_target_name.c_str(), "exp_", 4 ) == 0 )
		{
		// Process this expression

		dtikimorphtarget_t *morph_targets;
		Event *new_event;
		const char *morph_name;
		int number_of_morph_targets;

		morph_targets = gi.GetExpression( edict->s.modelindex, morph_target_name.c_str(), &number_of_morph_targets );

		if ( morph_targets )
			{
			for ( i = 0 ; i < number_of_morph_targets ; i++ )
				{
				morph_name = gi.Morph_NameForNum( edict->s.modelindex, morph_targets[ i ].morph_index );

				if ( morph_name )
					{
					new_event = new Event( EV_Morph );
					new_event->AddString( morph_name );
					new_event->AddFloat( morph_targets[ i ].percent );
					new_event->AddFloat( morph_time );
					new_event->AddInteger( return_to_zero );
					new_event->AddInteger( override );
					new_event->AddInteger( morph_channel );
					ProcessEvent( new_event );
					}
				}

			return;
			}
		}

	// Find this morph target

	morph_index = gi.Morph_NumForName( edict->s.modelindex, morph_target_name.c_str() );

	// Check unmorphing stuff

	if ( ( stricmp( morph_target_name.c_str(), "morph_base" ) == 0 ) || 
		  ( stricmp( morph_target_name.c_str(), "morph_mouth_base" ) == 0 ) || 
		  ( stricmp( morph_target_name.c_str(), "morph_brows_base" ) == 0 ) || 
		  ( stricmp( morph_target_name.c_str(), "morph_eyes_base" ) == 0 ) )
		unmorph = true;

	if ( morph_index == -1 && !unmorph )
		return;

	if ( morph_channel == MORPH_CHAN_NONE )
		morph_channel = GetMorphChannel( morph_target_name.c_str() );

	// Make sure we have a morph controller block

	if ( !morph_info )
		morph_info = CreateMorphInfo();

	// Deal with current morphs

	channel_being_used = false;

	if ( unmorph && ( morph_channel == MORPH_CHAN_NONE ) )
		override_all = true;

	for( i = 0 ; i < NUM_MORPH_CONTROLLERS ; i++ )
		{
		if ( morph_info->controllers[ i ].index != -1 )
			{
			// See if this is a matching channel

			if ( MorphChannelMatches( morph_channel, morph_info->controllers[ i ].channel ) )
				matching_channel = true;
			else 
				matching_channel = false;

			if ( override_all || ( matching_channel && override ) )
				{
				if ( morph_info->controllers[ i ].final_percent != 0 )
				{
					// Override this morph

					morph_info->controllers[ i ].speed = ( morph_info->controllers[ i ].current_percent * FRAMETIME ) / morph_time;
					morph_info->controllers[ i ].final_percent = 0;

					StartMorphController();
				}
				}
			else if ( matching_channel )
				{
				channel_being_used = true;
				}
			}
		}

	// If just unmorphing stuff, stop here

	if ( unmorph )
		return;

	// If not overriding and the channel is already being used, stop here

	if ( !override && channel_being_used )
		return;

	// Find a free morph controller & add this morph

	for( i = 0 ; i < NUM_MORPH_CONTROLLERS ; i++ )
		{
		if ( morph_info->controllers[ i ].index == -1 )
			{
			morph_info->controllers[ i ].index           = morph_index;
			morph_info->controllers[ i ].current_percent = 0;
			morph_info->controllers[ i ].final_percent   = final_percent;
			morph_info->controllers[ i ].speed           = (final_percent * FRAMETIME ) / morph_time;
			morph_info->controllers[ i ].return_to_zero  = return_to_zero;
			morph_info->controllers[ i ].channel         = morph_channel;

			StartMorphController();

			break;
			}
		}
	}

void Entity::UnmorphEvent
	(
	Event *ev
	)
	{
	str	morph_target_name;
	int	morph_index;
	int	i;
	float morph_time = 0.5;

	morph_target_name = ev->GetString( 1 );

	if ( ev->NumArgs() > 2 )
		morph_time = ev->GetFloat( 3 );

	// See if this is an expression

	if ( strnicmp( morph_target_name.c_str(), "exp_", 4 ) == 0 )
		{
		// Process this expression

		dtikimorphtarget_t *morph_targets;
		Event *new_event;
		const char *morph_name;
		int number_of_morph_targets;

		morph_targets = gi.GetExpression( edict->s.modelindex, morph_target_name.c_str(), &number_of_morph_targets );

		if ( morph_targets )
			{
			for ( i = 0 ; i < number_of_morph_targets ; i++ )
				{
				morph_name = gi.Morph_NameForNum( edict->s.modelindex, morph_targets[ i ].morph_index );

				if ( morph_name )
					{
					new_event = new Event( EV_Unmorph );
					new_event->AddString( morph_name );
					new_event->AddFloat( morph_time );
					ProcessEvent( new_event );
					}
				}

			return;
			}
		}

	morph_index = gi.Morph_NumForName( edict->s.modelindex, morph_target_name.c_str() );

	if ( morph_index == -1 || !morph_info )
		return;

	// Find this morph controller

	for( i = 0 ; i < NUM_MORPH_CONTROLLERS ; i++ )
		{
		if ( ( morph_info->controllers[ i ].index == morph_index ) && ( morph_info->controllers[ i ].final_percent != 0 ) )
			{
			morph_info->controllers[ i ].speed = (morph_info->controllers[ i ].final_percent * FRAMETIME ) / morph_time;
			morph_info->controllers[ i ].final_percent = 0.0f;

			StartMorphController();

			break;
			}
		}
	}

void Entity::MorphControl
	(
	Event *ev
	)
	{
	int i;
	qboolean process_next_frame = false;
	morph_t *controller;

	for( i = 0 ; i < NUM_MORPH_CONTROLLERS ; i++ )
		{
		controller = &morph_info->controllers[ i ];

		if ( controller->index != -1 )
			{
			// Lerp the morph percent

			if ( controller->current_percent != controller->final_percent )
				{
				if ( controller->current_percent < controller->final_percent )
					{
					controller->current_percent += controller->speed;

					if ( controller->current_percent > controller->final_percent )
						controller->current_percent = controller->final_percent;
					}
				else
					{
					controller->current_percent -= controller->speed;

					if ( controller->current_percent < controller->final_percent )
						controller->current_percent = controller->final_percent;
					}
				}

			if ( controller->current_percent == 0.0f )
				{
				controller->index = -1;
				controller->current_percent = 0.0f;
				}

			// Return to zero if necessary

			if ( controller->current_percent == controller->final_percent && controller->return_to_zero )
				controller->final_percent = 0.0f;

			if ( controller->current_percent != controller->final_percent )
				process_next_frame = true;
			}

		// Copy to edict

		edict->s.morph_controllers[ i ].index   = controller->index;
		edict->s.morph_controllers[ i ].percent = controller->current_percent / 100.0f;
		}

	if ( process_next_frame )
		PostEvent( EV_MorphControl, FRAMETIME );
	else
		morph_info->controller_on = false;
	}

int Entity::GetMorphChannel
	(
	const char *morph_name
	)
	{
	int morph_channel;

	if ( stricmp( morph_name, "morph_a-i" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_c-t" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_e" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_f-v" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_l-th" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_m-b-p" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_o" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_q-w" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_u" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_frown" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_sneer-l" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_sneer-r" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;
	else if ( stricmp( morph_name, "morph_mouth_base" ) == 0 )
		morph_channel = MORPH_CHAN_MOUTH;

	else if ( stricmp( morph_name, "morph_brows-up" ) == 0 )
		morph_channel = MORPH_CHAN_BROW;
	else if ( stricmp( morph_name, "morph_brows_base" ) == 0 )
		morph_channel = MORPH_CHAN_BROW;

	else if ( stricmp( morph_name, "morph_brow-ldn" ) == 0 )
		morph_channel = MORPH_CHAN_LEFT_BROW;

	else if ( stricmp( morph_name, "morph_brow-rdn" ) == 0 )
		morph_channel = MORPH_CHAN_RIGHT_BROW;

	else if ( stricmp( morph_name, "morph_lid-lshut" ) == 0 )
		morph_channel = MORPH_CHAN_LEFT_LID;

	else if ( stricmp( morph_name, "morph_lid-rshut" ) == 0 )
		morph_channel = MORPH_CHAN_RIGHT_LID;

	else if ( stricmp( morph_name, "morph_eyeshut" ) == 0 )
		morph_channel = MORPH_CHAN_EYES;
	else if ( stricmp( morph_name, "morph_eyes_base" ) == 0 )
		morph_channel = MORPH_CHAN_EYES;

	else 
		morph_channel = MORPH_CHAN_NONE;

	return morph_channel;
	}

void Entity::StartMorphController
	(
	void
	)
	{
	if ( !morph_info->controller_on )
		{
		morph_info->controller_on = true;
		CancelEventsOfType( EV_MorphControl );
		PostEvent( EV_MorphControl, FRAMETIME );
		}
	}


void Entity::SetAnimOnAttachedModel
   (
	const str &AnimName,
	const str &TagName
	)

	{
	int tag_num = gi.Tag_NumForName( this->edict->s.modelindex , TagName.c_str() );

	if ( bind_info )
		{
		Entity *attachment = 0;

		for ( int i = 0; i < MAX_MODEL_CHILDREN; i++ )
			{
			
			//Check for valid entities
         if( bind_info->children[i] == ENTITYNUM_NONE )
				continue;

			attachment = ( Entity * )G_GetEntity( bind_info->children[i] );
			
			if ( attachment->edict->s.tag_num == tag_num )
				{
				if ( !attachment->animate )
					{
					attachment->animate = new Animate;
					}
				
				int anim_num = gi.Anim_Random ( attachment->edict->s.modelindex, AnimName.c_str() );
				if ( anim_num != -1 )
					{
					attachment->animate->NewAnim( anim_num );
					}
				}
			}
		}
	}

void Entity::SetAnimOnAttachedModel
   (
	Event *ev
	)
	{
	str attachmentAnim = ev->GetString( 1 );
	str TagName = ev->GetString( 2 );

	SetAnimOnAttachedModel( attachmentAnim , TagName );	
	}


void Entity::SetCinematicAnim
	(
	const str &AnimName
	)
	{
	if (!animate)
		{
		animate = new Animate( this );
		}
				
	int anim_num = gi.Anim_Random ( edict->s.modelindex, AnimName.c_str() );
	if ( anim_num != -1 )
		{
		animate->NewAnim( anim_num );
		animate->SetAnimDoneEvent( new Event (EV_CinematicAnimDone) );
		}

	gravity = 0.0f;
	edict->contents = CONTENTS_SETCLIP ;
	edict->clipmask = MASK_SETCLIP ;
	}


void Entity::SetCinematicAnim
   (
	Event *ev
	)
	{
	str animName = ev->GetString( 1 );
	SetCinematicAnim( animName);	
	}


void Entity::CinematicAnimDone
	( 
	void 
	)
	{
	gravity = 1.0f ;
	edict->contents &= ~CONTENTS_SETCLIP ;
	edict->clipmask = MASK_SOLID ;
	}

void Entity::CinematicAnimDone
	(
	Event *ev
	)
	{
	CinematicAnimDone();
	}


void Entity::SetEntityExplosionModel
   (
	Event *ev
	)
	{
	explosionModel = ev->GetString( 1 );
	}

//----------------------------------------------------------------
// Name:			ProjectileAtk
// Class:			Entity
//
// Description:		launches a projectile either from the player 
//					or from a tag
//
// Parameters:		event ev contains projectile name and an
//					optional tag name
//
// Returns:			None
//----------------------------------------------------------------
void Entity::ProjectileAtk( Event *ev )
{
	str projectileName = ev->GetString( 1 );
	
	Vector position;
	Vector direction;
	
	if ( ev->NumArgs() > 1 )
	{
		// Projectile name, tagName
		str tagName( ev->GetString( 2 ) );
		GetTag( tagName.c_str(), &position, &direction );
	}
	else
	{
		// Projectile name, Player is the target
		position = origin;
		gentity_t *ed;
		Entity* enemy = NULL;
		for( int i = 0 ; i < game.maxclients; i++ )
		{
			ed = &g_entities[ i ];
			
			if ( !ed->inuse || !ed->entity )
				continue;
			
			enemy = ed->entity;
		}
		if ( enemy )
		{
			direction = enemy->origin - origin;
			direction.normalize();
		}
		else
		{
			angles.AngleVectors( &direction, NULL, NULL );
		}
	}
	
	ProjectileAttack( position, direction, this, projectileName.c_str(), 1.0f, 0.0f );
}

//----------------------------------------------------------------
// Name:			ProjectileAttackPoint
// Class:			Entity
//
// Description:		launches a projectile towards the given point
//
// Parameters:		event ev contains projectile name and entity name
//
// Returns:			None
//----------------------------------------------------------------
void Entity::ProjectileAttackPoint( Event *ev )
{
	const str projectileName( ev->GetString( 1 ) );
	const Vector targetPosition( ev->GetVector( 2 ) );
	
	Vector direction( targetPosition - origin );
	direction.normalize();
	Projectile *projectile = ProjectileAttack( origin, direction, this, projectileName.c_str(), 1.0f, 0.0f );
	if ( ev->NumArgs() > 2 )
	{
		Angle launchAngle( ev->GetFloat( 3 ) );
		Trajectory trajectory( origin, targetPosition, launchAngle, projectile->gravity * sv_currentGravity->value );

		projectile->velocity = trajectory.GetInitialVelocity();
		Vector launchDirection( projectile->velocity );
		launchDirection.normalize();
		projectile->angles = launchDirection.toAngles();
		
		projectile->CancelEventsOfType( EV_Projectile_Explode );
		Event *event = new Event( EV_Projectile_Explode );
		if ( ev->NumArgs() > 3 )
		{
			projectile->PostEvent( event, ev->GetFloat( 4 ) );
		}
		else
		{
			projectile->PostEvent( event, trajectory.GetTravelTime() );
		}
	}
}

//----------------------------------------------------------------
// Name:			ProjectileAttackEntity
// Class:			Entity
//
// Description:		launches a projectile either from the named entity
//
// Parameters:		event ev contains projectile name and entity name
//
// Returns:			None
//----------------------------------------------------------------
void Entity::ProjectileAttackEntity( Event *ev )
{
	const str projectileName( ev->GetString( 1 ) );
	
	Entity *target = ev->GetEntity( 2 );
	if ( target )
	{
		Vector direction( target->origin - origin );
		direction.normalize();
		Projectile *projectile = ProjectileAttack( origin, direction, this, projectileName.c_str(), 1.0f, 0.0f );

		if ( !projectile )
			return;

		if ( ev->NumArgs() > 2 )
		{
			Angle launchAngle( ev->GetFloat( 3 ) );
			Trajectory trajectory( origin, target->centroid, launchAngle, projectile->gravity * -sv_currentGravity->value );

			projectile->velocity = trajectory.GetInitialVelocity();
			Vector launchDirection( projectile->velocity );
			launchDirection.normalize();
			projectile->angles = launchDirection.toAngles();
			
			projectile->CancelEventsOfType( EV_Projectile_Explode );
			Event *event = new Event( EV_Projectile_Explode );
			if ( ev->NumArgs() > 3 )
			{
				projectile->PostEvent( event, ev->GetFloat( 4 ) );
			}
			else
			{
				projectile->PostEvent( event, trajectory.GetTravelTime() );
			}
		}
	}
}

//----------------------------------------------------------------
// Name:			ProjectileAttackFromTag
// Class:			Entity
//
// Description:		launches a projectile either from the named tag
//
// Parameters:		event ev contains projectile name and tag name
//
// Returns:			None
//----------------------------------------------------------------
void Entity::ProjectileAttackFromTag( Event *ev )
{
	str projectileName( ev->GetString( 1 ) );
	str tagName( ev->GetString( 2 ) );
	
	Vector position;
	Vector direction;
	GetTag( tagName.c_str(), &position, &direction );
	
	float speed = 0.0f;
	if ( ev->NumArgs() > 3 )
	{
		speed = ev->GetFloat( 4 );
	}

	Projectile *projectile = ProjectileAttack( position, direction, this, projectileName.c_str(), 1.0f, speed );
	if ( ev->NumArgs() > 4 )
	{
		projectile->CancelEventsOfType( EV_Projectile_Explode );
		Event *event = new Event( EV_Projectile_Explode );
		projectile->PostEvent( event, ev->GetFloat( 5 ) );
	}
}

//----------------------------------------------------------------
// Name:			ProjectileAttackFromPoint
// Class:			Entity
//
// Description:		launches a projectile either from the desired
//					location facing the desired direction
//
// Parameters:		event ev contains projectile name, the 
//					position and direction for the new projectile
//
// Returns:			None
//----------------------------------------------------------------
void Entity::ProjectileAttackFromPoint( Event *ev )
{
	str projectileName( ev->GetString( 1 ) );

	Vector position( ev->GetVector( 2 ) );
	Vector direction( ev->GetVector( 3 ) );

	float speed = 0.0f;
	if ( ev->NumArgs() > 3 )
	{
		speed = ev->GetFloat( 4 );
	}

	Vector forward;
	direction.AngleVectors( &forward );
	Projectile *projectile = ProjectileAttack( origin + position, forward, this, projectileName.c_str(), 1.0f, speed );
	if ( ev->NumArgs() > 4 )
	{
		projectile->CancelEventsOfType( EV_Projectile_Explode );
		Event *event = new Event( EV_Projectile_Explode );
		projectile->PostEvent( event, ev->GetFloat( 5 ) );
	}
}

void Entity::TraceAtk
   (
	Event *ev
	)
	{
	Vector position;
	Vector forward;
	Vector right;
	Vector up;
	float range;
	float damage;
	float knockback = 0.0;
	str means_of_death_string = "bullet";
	int means_of_death;
	//int offsetPitch = 0;

	damage = ev->GetFloat( 1 );
	range  = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		means_of_death_string = ev->GetString( 3 );

	if ( ev->NumArgs() > 3 )
		knockback = ev->GetFloat( 4 );

	if ( ev->NumArgs() > 4 )
		{
		str tag_name;

		tag_name = ev->GetString( 5 );

		GetTag( tag_name.c_str(), &position, &forward, &right, &up );
		}
	else
		{
		position = origin;
		angles.AngleVectors( &forward, &right, &up );
		}

	means_of_death = MOD_NameToNum( means_of_death_string );
	
	/*
	if ( ev->NumArgs() > 5 )
		offsetPitch = ev->GetInteger( 6 );

	if ( offsetPitch )
		{
		Vector ForAngles;
		ForAngles = forward.toAngles();

		ForAngles[YAW] = AngleNormalize180( ForAngles[YAW] );
		ForAngles[PITCH] = AngleNormalize180( ForAngles[PITCH] );
		ForAngles[ROLL] = AngleNormalize180( ForAngles[ROLL] );
		offsetPitch = AngleNormalize180(offsetPitch);

		ForAngles[PITCH] += offsetPitch;


		ForAngles[YAW] = AngleNormalize360( ForAngles[YAW] );
		ForAngles[PITCH] = AngleNormalize360( ForAngles[PITCH] );
		ForAngles[ROLL] = AngleNormalize360( ForAngles[ROLL] );

		ForAngles.AngleVectors( &forward );
		}
   */

	BulletAttack( position, forward, right, up, range, damage, knockback, 0, means_of_death, vec_zero, 1, this );
	}

qboolean Entity::MorphChannelMatches
	(
	int morph_channel1,
	int morph_channel2
	)
	{
	// Nothing matches with non

	if ( morph_channel1 == MORPH_CHAN_NONE )
		return false;

	// See if they match exactly
		
	if ( morph_channel1 == morph_channel2  )
		return true;

	// Check special cases

	if ( ( morph_channel1 == MORPH_CHAN_BROW ) && ( ( morph_channel2 == MORPH_CHAN_LEFT_BROW ) || ( morph_channel2 == MORPH_CHAN_RIGHT_BROW ) ) )
		return true;

	if ( ( morph_channel2 == MORPH_CHAN_BROW ) && ( ( morph_channel1 == MORPH_CHAN_LEFT_BROW ) || ( morph_channel1 == MORPH_CHAN_RIGHT_BROW ) ) )
		return true;

	if ( ( morph_channel1 == MORPH_CHAN_EYES ) && ( ( morph_channel2 == MORPH_CHAN_LEFT_LID ) || ( morph_channel2 == MORPH_CHAN_RIGHT_LID ) ) )
		return true;

	if ( ( morph_channel2 == MORPH_CHAN_EYES ) && ( ( morph_channel1 == MORPH_CHAN_LEFT_LID ) || ( morph_channel1 == MORPH_CHAN_RIGHT_LID ) ) )
		return true;

	return false;
	}

inline MorphInfo *CreateMorphInfo( void )
	{
	MorphInfo *new_morph_info;

	new_morph_info = new MorphInfo;

	if ( !new_morph_info )
		gi.Error( ERR_DROP, "Couldn't alloc MorphInfo" );

	return new_morph_info;
	}

#define CONTENTS_OPERATION_ADD			0
#define CONTENTS_OPERATION_SUBTRACT		1
#define CONTENTS_OPERATION_SET			2

void Entity::Contents
	(
	Event* ev
	)
	{
	if(ev == 0)
		return;

	int operation;
	const char* contentType;
	unsigned long contents = 0;
	unsigned long newFlags = 0;

	contents = getContents();

	for(int i = 1; i <= ev->NumArgs(); i++)
		{
		contentType = ev->GetString(i);

		if( ( contentType == 0 ) || ( strlen( contentType ) == 0 ) )
			continue;

		// Get operation type

		if(contentType[0] == '+')
			{
			operation = CONTENTS_OPERATION_ADD;
			contentType++;
			}
		else if(contentType[0] == '-')
			{
			operation = CONTENTS_OPERATION_SUBTRACT;
			contentType++;
			}
		else 
			operation = CONTENTS_OPERATION_SET;

		// Get contents type

		if(stricmp(contentType, "shootable") == 0)
			newFlags = CONTENTS_SHOOTABLE_ONLY;
		else if(stricmp(contentType, "targetable") == 0)
		{
			newFlags = CONTENTS_TARGETABLE;
			setSolidType(SOLID_BBOX);
		}
		else if(stricmp(contentType, "body") == 0)
			newFlags = CONTENTS_BODY;
		else if(stricmp(contentType, "solid") == 0)
			newFlags = CONTENTS_SOLID;
		else if(stricmp(contentType, "usable") == 0)
			newFlags = CONTENTS_USABLE;
		else if(stricmp(contentType, "setclip") == 0)
			newFlags = CONTENTS_SETCLIP;
		else if(stricmp(contentType, "playerclip") == 0)
			newFlags = CONTENTS_PLAYERCLIP;
		else if(stricmp(contentType, "monsterclip") == 0)
			newFlags = CONTENTS_MONSTERCLIP;
		else if(stricmp(contentType, "cameraclip") == 0)
			newFlags = CONTENTS_CAMERACLIP;
		else if(stricmp(contentType, "weaponclip") == 0)
			newFlags = CONTENTS_WEAPONCLIP;
		else if(stricmp(contentType, "corpse") == 0)
			newFlags = CONTENTS_CORPSE;
		else if(stricmp(contentType, "all") == 0)
			newFlags = 0xFFFFFFFF;

		// Change contents appropriatly

		if ( operation == CONTENTS_OPERATION_ADD )
			contents |= newFlags;
		else if ( operation == CONTENTS_OPERATION_SUBTRACT )
			contents &= ~newFlags;
		else if ( operation == CONTENTS_OPERATION_SET )
			contents = newFlags;
		}

		setContents(contents);
		link();
	}

void Entity::setMask( Event* ev )
{
	int i;
	int operation;
	const char* maskType;
	unsigned long currentMask;
	unsigned long newMask = 0;


	// Get the current mask

	currentMask = edict->clipmask;

	// Loop through all of the parms and change the mask appropriately

	for( i = 1 ; i <= ev->NumArgs() ; i++ )
	{
		maskType = ev->GetString( i );

		if( ( !maskType ) || ( strlen( maskType ) == 0 ) )
			continue;

		// Get operation type

		if ( maskType[0] == '+' )
		{
			operation = CONTENTS_OPERATION_ADD;
			maskType++;
		}
		else if ( maskType[0] == '-' )
		{
			operation = CONTENTS_OPERATION_SUBTRACT;
			maskType++;
		}
		else 
		{
			operation = CONTENTS_OPERATION_SET;
		}

		// Get contents type

		// Masks

		if ( stricmp( maskType, "solid" ) == 0 )
			newMask = MASK_SOLID;
		else if ( stricmp( maskType, "usable" ) == 0 )
			newMask = MASK_USABLE;
		else if ( stricmp( maskType, "playersolid" ) == 0 )
			newMask = MASK_PLAYERSOLID;
		else if ( stricmp( maskType, "deadsolid" ) == 0 )
			newMask = MASK_DEADSOLID;
		else if ( stricmp( maskType, "monstersolid" ) == 0 )
			newMask = MASK_MONSTERSOLID;
		else if ( stricmp( maskType, "water" ) == 0 )
			newMask = MASK_WATER;
		else if ( stricmp( maskType, "opaque" ) == 0 )
			newMask = MASK_OPAQUE;
		else if ( stricmp( maskType, "shot" ) == 0 )
			newMask = MASK_SHOT;
		else if ( stricmp( maskType, "projectile" ) == 0 )
			newMask = MASK_PROJECTILE;
		else if ( stricmp( maskType, "melee" ) == 0 )
			newMask = MASK_MELEE;
		else if ( stricmp( maskType, "pathsolid" ) == 0 )
			newMask = MASK_PATHSOLID;
		else if ( stricmp( maskType, "camerasolid" ) == 0 )
			newMask = MASK_CAMERASOLID;
		else if ( stricmp( maskType, "setclip" ) == 0 )
			newMask = MASK_SETCLIP;

		// Contents

		else if ( stricmp( maskType, "contents_solid" ) == 0 )
			newMask = CONTENTS_SOLID;
		else if ( stricmp( maskType, "contents_usable" ) == 0 )
			newMask = CONTENTS_USABLE;
		else if ( stricmp( maskType, "contents_setclip" ) == 0 )
			newMask = CONTENTS_SETCLIP;
		else if ( stricmp( maskType, "contents_targetable" ) == 0 )
			newMask = CONTENTS_TARGETABLE;
		else if ( stricmp( maskType, "contents_playerclip" ) == 0 )
			newMask = CONTENTS_PLAYERCLIP;
		else if ( stricmp( maskType, "contents_monsterclip" ) == 0 )
			newMask = CONTENTS_MONSTERCLIP;
		else if ( stricmp( maskType, "contents_cameraclip" ) == 0 )
			newMask = CONTENTS_CAMERACLIP;
		else if ( stricmp( maskType, "contents_weaponclip" ) == 0 )
			newMask = CONTENTS_WEAPONCLIP;
		else if ( stricmp( maskType, "contents_shootable" ) == 0 )
			newMask = CONTENTS_SHOOTABLE_ONLY;
		else if ( stricmp( maskType, "contents_body" ) == 0 )
			newMask = CONTENTS_BODY;
		else if ( stricmp( maskType, "contents_corpse" ) == 0 )
			newMask = CONTENTS_CORPSE;

		// All

		else if ( stricmp( maskType, "all" ) == 0 )
			newMask = 0xFFFFFFFF;

		// Change mask appropriately

		if ( operation == CONTENTS_OPERATION_ADD )
			currentMask |= newMask;
		else if ( operation == CONTENTS_OPERATION_SUBTRACT )
			currentMask &= ~newMask;
		else if ( operation == CONTENTS_OPERATION_SET )
			currentMask = newMask;
	}

	// Save the new mask

	edict->clipmask = currentMask;
}

void Entity::getCustomShaderInfo( const str &customShader, str &shaderName, str &soundName )
{
	// Setup the defaults

	shaderName = customShader;
	soundName = "";

	// See if we need to get info from the database

	if ( ( customShader.length() > 4 ) && ( stricmp( customShader.c_str() + customShader.length() - 4, ".gdb" ) == 0 ) )
	{
		str gameplayObjectName = customShader;
		gameplayObjectName.CapLength( customShader.length() - 4 );

		shaderName = G_GetDatabaseString( "CustomShader", gameplayObjectName, "ShaderName" );
		soundName  = G_GetDatabaseString( "CustomShader", gameplayObjectName, "SoundName" );
	}
}

//----------------------------------------------------------------
// Name:			setCustomShader
// Class:			Entity
//
// Description:		Sets the custom shader for this entity
//
// Parameters:		const char *customShader		- name of the shader
//
// Returns:			none
//----------------------------------------------------------------

void Entity::setCustomShader( const char *customShader )
{
	str shaderName;
	str soundName;

	if ( !customShader || ( strlen( customShader ) == 0 ) )
		return;

	getCustomShaderInfo( customShader, shaderName, soundName );

	// Apply the custom shader

	edict->s.customShader = gi.imageindex( shaderName );
	edict->s.eFlags |= EF_EFFECT_CUSTOM;

	// Start the loop sound associated with the customshader

	if ( soundName.length() )
	{
		LoopSound( soundName );
	}
}

//----------------------------------------------------------------
// Name:			setCustomShader
// Class:			Entity
//
// Description:		Sets the custom shader for this entity
//
// Parameters:		Event *ev						- event that contains the name of the shader
//
// Returns:			none
//----------------------------------------------------------------

void Entity::setCustomShader( Event *ev )
{
	setCustomShader( ev->GetString( 1 ) );
}

//----------------------------------------------------------------
// Name:			clearCustomShader
// Class:			Entity
//
// Description:		Clears the custom shader for this entity if it matches the shader name passed in
//
// Parameters:		const char *customShader		- name of the shader to clear, if NULL will always clear
//
// Returns:			none
//----------------------------------------------------------------

void Entity::clearCustomShader( const char *customShader )
{
	int tempImageIndex;
	str shaderName;
	str soundName;

	if ( customShader && edict->s.customShader )
	{
		getCustomShaderInfo( customShader, shaderName, soundName );

		// Only clear the custom shader if it matches the one passed in

		tempImageIndex = gi.imageindex( shaderName );

		if ( edict->s.customShader == tempImageIndex )
		{
			edict->s,customShader = 0;
			edict->s.eFlags &= ~EF_EFFECT_CUSTOM;
		}

		// Get rid of the loopsound associated with this custom shader

		if ( soundName.length() )
		{
			StopLoopSound();
		}
	}
	else
	{
		edict->s,customShader = 0;
		edict->s.eFlags &= ~EF_EFFECT_CUSTOM;
	}
}

//----------------------------------------------------------------
// Name:			clearCustomShader
// Class:			Entity
//
// Description:		Clears the custom shader for this entity if it matches the shader name passed in
//
// Parameters:		Event *ev				- event that optionally contains the name of the shader to clear
//
// Returns:			none
//----------------------------------------------------------------

void Entity::clearCustomShader( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		clearCustomShader( ev->GetString( 1 ) );
	else
		clearCustomShader();
}

bool Entity::hasCustomShader( const char *customShader )
{
	// See if any custom shader is currently being used

	if ( !( edict->s.eFlags & EF_EFFECT_CUSTOM ) )
		return false;

	if ( customShader && edict->s.customShader )
	{
		int tempImageIndex;

		// Check to see if the shader matches the one passed in

		tempImageIndex = gi.imageindex( customShader );

		if ( edict->s.customShader != tempImageIndex )
		{
			// Not the correct shader

			return false;
		}
	}

	// Everything matches

	return true;
}

void Entity::setCustomEmitter( const char *customEmitter )
{
	if ( !customEmitter )
		return;

	edict->s.customEmitter = gi.imageindex( customEmitter );
	edict->s.eFlags |= EF_EMITTER_CUSTOM;
}

void Entity::setCustomEmitter( Event *ev )
{
	setCustomEmitter( ev->GetString( 1 ) );
}

void Entity::clearCustomEmitter( const char *customEmitter )
{
	int tempImageIndex;

	if ( customEmitter && edict->s.customEmitter )
	{
		// Only clear the custom shader if it matches the one passed in

		tempImageIndex = gi.imageindex( customEmitter );

		if ( edict->s.customEmitter == tempImageIndex )
		{
			edict->s,customEmitter = 0;
			edict->s.eFlags &= ~EF_EMITTER_CUSTOM;
		}

	}
	else
	{
		edict->s,customEmitter = 0;
		edict->s.eFlags &= ~EF_EMITTER_CUSTOM;
	}
}

void Entity::clearCustomEmitter( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		clearCustomEmitter( ev->GetString( 1 ) );
	else
		clearCustomEmitter();
}

void Entity::hideFeaturesForFade( void )
{
	Event *event;

	// Shut the eyes

	event = new Event( EV_Morph );
	event->AddString( "morph_lid-lshut" );
	event->AddFloat( 100.0f );
	event->AddFloat( 0.05f );
	ProcessEvent ( event );

	event = new Event( EV_Morph );
	event->AddString( "morph_lid-rshut" );
	event->AddFloat( 100.0f );
	event->AddFloat( 0.05f );
	ProcessEvent ( event );

	// Hide the eyes and mouth surfaces

	event = new Event( EV_SurfaceModelEvent );
	event->AddString( "material3" );
	event->AddString( "+nodraw" );
	ProcessEvent ( event );

	// Don't let the entity blink

	event = new Event( EV_Actor_Blink );
	event->AddInteger( 0 );
	ProcessEvent ( event );
}

void Entity::showFeaturesForFade( void )
{
	Event *event;

	// Open the eyes

	event = new Event( EV_Unmorph );
	event->AddString( "morph_lid-lshut" );
	event->AddFloat( 100.0f );
	event->AddFloat( 0.5f );
	ProcessEvent ( event );

	event = new Event( EV_Unmorph );
	event->AddString( "morph_lid-rshut" );
	event->AddFloat( 100.0f );
	event->AddFloat( 0.5f );
	ProcessEvent ( event );

	// Show the eyes and mouth surfaces

	event = new Event( EV_SurfaceModelEvent );
	event->AddString( "material3" );
	event->AddString( "-nodraw" );
	ProcessEvent ( event );

	// Allow the entity to blink again

	event = new Event( EV_Actor_Blink );
	event->AddInteger( 1 );
	ProcessEvent ( event );
}

void Entity::DisplayEffect ( Event *ev )
{
	str effectType;
	str effectName;
	Event *event;
	int i;
	Entity *ent;
	bool passEvent = false;
	bool cancelEvents = false;
	Vector effectPosition;
	GameplayManager *gpm;
	str gameplayObjectName;


	// Get the parms

	effectType = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
	{
		effectName = ev->GetString( 2 );
	}
	else
	{
		const char *delimiterPtr;

		// There is only 1 parm, so lets see if the type and name are both crammed into the 1 parm

		delimiterPtr = strstr( effectType.c_str(), "-" );

		if ( delimiterPtr )
		{
			// Effect name is everything after the -
			effectName = delimiterPtr + 1;

			// Effect type is everything before the -
			effectType.CapLength( delimiterPtr - effectType );
		}
	}

	// Get a pointer to the gameplay manager

	gpm = GameplayManager::getTheGameplayManager();

	// See if there is a gameplay object (if necessary)

	if ( ( strnicmp( effectType.c_str(), "TransportOut", strlen( "TransportOut" ) ) == 0 ) ||
		 ( strnicmp( effectType.c_str(), "TransportIn", strlen( "TransportIn" ) ) == 0 ) ||
		 ( strnicmp( effectType.c_str(), "FadeOut", strlen( "FadeOut" ) ) == 0 ) ||
		 ( strnicmp( effectType.c_str(), "FadeIn", strlen( "FadeIn" ) ) == 0 ) )
	{
		if ( ( strnicmp( effectType.c_str(), "TransportOut", strlen( "TransportOut" ) ) == 0 ) )
		{
			gameplayObjectName = "TransportOut";
		}
		else if ( ( strnicmp( effectType.c_str(), "TransportIn", strlen( "TransportIn" ) ) == 0 ) )
		{
			gameplayObjectName = "TransportIn";
		}
		else if ( ( strnicmp( effectType.c_str(), "FadeOut", strlen( "FadeOut" ) ) == 0 ) )
		{
			gameplayObjectName = "FadeOut";
		}
		else if ( ( strnicmp( effectType.c_str(), "FadeIn", strlen( "FadeIn" ) ) == 0 ) )
		{
			gameplayObjectName = "FadeIn";
		}

		gameplayObjectName += effectName;

		if ( !gpm->hasObject( gameplayObjectName ) )
		{
			gi.WDPrintf( "Can't find object called %s to use in DisplayEffect\n", gameplayObjectName.c_str() );
			return;
		}
	}

	if ( effectType == "TransportOut" )
	{
		str shaderName;
		str soundName;
		str effectTikiName;
		str animName;
		str effectPos;
		str currentEffectPos;
		float effectTime;
		float transportTime;
		int numEffects;
		int immobilize;
		int hideFeatures;
		int attach;


		// Get all of the info from the gameplay database

		shaderName		= gpm->getStringValue( gameplayObjectName, "ShaderName" );
		soundName		= gpm->getStringValue( gameplayObjectName, "SoundName" );
		effectTikiName	= gpm->getStringValue( gameplayObjectName, "EffectName" );
		attach			= (int) gpm->getFloatValue( gameplayObjectName, "AttachEffect" );
		animName		= gpm->getStringValue( gameplayObjectName, "AnimName" );

		effectPos		= gpm->getStringValue( gameplayObjectName, "EffectPosName" );
		numEffects		= (int) gpm->getFloatValue( gameplayObjectName, "NumEffects" );

		effectTime		= gpm->getFloatValue( gameplayObjectName, "EffectTime" );
		transportTime	= gpm->getFloatValue( gameplayObjectName, "TransportTime" );

		immobilize		= (int) gpm->getFloatValue( gameplayObjectName, "Immobilize" );
		hideFeatures	= (int) gpm->getFloatValue( gameplayObjectName, "HideFeatures" );

		setCustomShader( shaderName );
		setAlpha( 0.0f );

		edict->s.renderfx |= RF_FORCE_ALPHA_EFFECTS;
		edict->s.renderfx &= ~RF_FORCE_ALPHA;

		event = new Event( EV_FadeIn );
		event->AddFloat( effectTime );
		ProcessEvent( event );

		event = new Event( EV_DisplayEffect );
		event->AddString( "TransportOut2" );
		event->AddString( effectName );
		PostEvent( event, effectTime );

		if ( hideFeatures )
			hideFeaturesForFade();

		if ( edict->s.parent == ENTITYNUM_NONE )
		{
			if ( soundName.length() )
			{
				Sound( soundName );
			}

			if ( effectTikiName.length() > 4 )
			{
				Vector pos;

				currentEffectPos = effectPos;

				for ( i = 0 ; i < numEffects ; i++ )
				{
					if ( !effectPos.length() || effectPos == "centroid" )
						pos = centroid;
					else if ( effectPos == "origin" )
						pos = origin;
					else if ( effectPos == "randombone" )
					{
						int tagNum;

						tagNum = (int) G_Random( gi.NumTags( edict->s.modelindex ) );
						currentEffectPos = gi.Tag_NameForNum( edict->s.modelindex, tagNum );
						
						GetTag( tagNum, &pos, NULL );
					}
					else if ( gi.Tag_NumForName( edict->s.modelindex, effectPos.c_str() ) >= 0 )
						GetTag( effectPos, &pos, NULL );
					else
						pos = centroid;

					if ( attach )
						attachEffect( effectTikiName, currentEffectPos, effectTime + transportTime );
					else
						SpawnEffect( effectTikiName, pos, angles, effectTime + transportTime );
				}
			}
		}

		if ( animate && animName.length() && animate->HasAnim( animName.c_str() ) )
		{
			animate->RandomAnimate( animName.c_str() );
		}

		if ( immobilize )
		{
			takedamage = DAMAGE_NO;
			flags |= FL_IMMOBILE;
		}

		passEvent = true;
	}
	else if ( effectType == "TransportOut2" )
	{
		str shaderName;
		float transportTime;

		// Get all of the info from the gameplay database

		shaderName    = gpm->getStringValue( gameplayObjectName, "ShaderName" );
		transportTime = gpm->getFloatValue( gameplayObjectName, "TransportTime" );

		CancelEventsOfType( EV_FadeIn );

		edict->s.renderfx &= ~RF_FORCE_ALPHA_EFFECTS;

		setCustomShader( shaderName.c_str() );
		setAlpha( 1.0f );
		edict->s.renderfx |= RF_FORCE_ALPHA;

		event = new Event( EV_FadeNoRemove );
		event->AddFloat( transportTime );
		event->AddFloat( 0.0f );
		ProcessEvent( event );

		event = new Event( EV_DisplayEffect );
		event->AddString( "TransportOut3" );
		event->AddString( effectName );
		PostEvent( event, transportTime );
	}
	else if ( effectType == "TransportOut3" )
	{
		str shaderName;
		int hideFeatures;

		// Get all of the info from the gameplay database

		shaderName    = gpm->getStringValue( gameplayObjectName, "ShaderName" );

		hideFeatures = (int) gpm->getFloatValue( gameplayObjectName, "HideFeatures" );

		if ( hideFeatures )
			showFeaturesForFade();

		if ( shaderName.length() )
		{
			clearCustomShader( shaderName );
		}

		// Finish up the transport out

		event = new Event( EV_EntityRenderEffects );
		event->AddString( "-shadow" );
		ProcessEvent( event );

		takedamage = DAMAGE_YES;
		flags &= ~FL_IMMOBILE;
	}
	else if ( effectType == "TransportIn" )
	{
		str shaderName;
		str soundName;
		str effectPos;
		str currentEffectPos;
		str effectTikiName;
		float effectTime;
		float transportTime;
		int numEffects;
		int immobilize;
		int attach;
		int hideFeatures;

		// Cancel other display events so we don't get any overlap (can't do this because it breaks animations that
		//   have both since animate posts all frame commands un front

		//CancelEventsOfType( EV_DisplayEffect ); 
		clearCustomShader();

		// Get all of the info from the gameplay database

		shaderName = gpm->getStringValue( gameplayObjectName, "ShaderName" );
		soundName = gpm->getStringValue( gameplayObjectName, "SoundName" );
		effectTikiName	= gpm->getStringValue( gameplayObjectName, "EffectName" );
		attach			= (int) gpm->getFloatValue( gameplayObjectName, "AttachEffect" );

		effectPos		= gpm->getStringValue( gameplayObjectName, "EffectPosName" );
		numEffects		= (int) gpm->getFloatValue( gameplayObjectName, "NumEffects" );

		effectTime		= gpm->getFloatValue( gameplayObjectName, "EffectTime" );
		transportTime   = gpm->getFloatValue( gameplayObjectName, "TransportTime" );

		immobilize		= (int) gpm->getFloatValue( gameplayObjectName, "Immobilize" );
		hideFeatures	= (int) gpm->getFloatValue( gameplayObjectName, "HideFeatures" );

		edict->s.renderfx &= ~RF_FORCE_ALPHA_EFFECTS;

		if ( shaderName.length() )
		{
			setCustomShader( shaderName );
		}

		setAlpha( 0.0f );
		edict->s.renderfx |= RF_FORCE_ALPHA;

		event = new Event( EV_FadeIn );
		event->AddFloat( transportTime );
		ProcessEvent( event );

		event = new Event( EV_DisplayEffect );
		event->AddString( "TransportIn2" );
		event->AddString( effectName );
		PostEvent( event, transportTime );

		if ( hideFeatures )
			hideFeaturesForFade();

		if ( edict->s.parent == ENTITYNUM_NONE )
		{
			if ( soundName.length() )
			{
				Sound( soundName );
			}

			if ( effectTikiName.length() > 4 )
			{
				Vector pos;

				currentEffectPos = effectPos;

				for ( i = 0 ; i < numEffects ; i++ )
				{
					if ( !effectPos.length() || effectPos == "centroid" )
						pos = centroid;
					else if ( effectPos == "origin" )
						pos = origin;
					else if ( effectPos == "randombone" )
					{
						int tagNum;

						tagNum = (int) G_Random( gi.NumTags( edict->s.modelindex ) );
						currentEffectPos = gi.Tag_NameForNum( edict->s.modelindex, tagNum );
						
						GetTag( tagNum, &pos, NULL );
					}
					else if ( gi.Tag_NumForName( edict->s.modelindex, effectPos.c_str() ) >= 0 )
						GetTag( effectPos, &pos, NULL );
					else
						pos = centroid;

					if ( attach )
						attachEffect( effectTikiName, currentEffectPos, effectTime + transportTime );
					else
						SpawnEffect( effectTikiName, pos, angles, effectTime + transportTime );
				}
			}
		}

		if ( immobilize )
		{
			takedamage = DAMAGE_NO;
			flags |= FL_IMMOBILE;
		}

		passEvent = true;
		cancelEvents = true;
	}
	else if ( effectType == "TransportIn2" )
	{
		str shaderName;
		float effectTime;

		// Get all of the info from the gameplay database

		shaderName     = gpm->getStringValue( gameplayObjectName, "ShaderName" );
		effectTime    = gpm->getFloatValue( gameplayObjectName, "EffectTime" );

		CancelEventsOfType( EV_FadeIn );
		edict->s.renderfx &= ~RF_FORCE_ALPHA;

		if ( shaderName.length() )
		{
			setCustomShader( shaderName );
		}

		setAlpha( 1.0f );

		if ( shaderName.length() )
		{
			edict->s.renderfx |= RF_FORCE_ALPHA_EFFECTS;

			event = new Event( EV_FadeNoRemove );
			event->AddFloat( effectTime );
			event->AddFloat( 0.0f );
			ProcessEvent( event );
		}

		event = new Event( EV_DisplayEffect );
		event->AddString( "TransportIn3" );
		event->AddString( effectName );
		PostEvent( event, effectTime  );
	}
	else if ( effectType == "TransportIn3" )
	{
		str shaderName;
		str animName;
		int hideFeatures;

		// Get all of the info from the gameplay database

		shaderName		= gpm->getStringValue( gameplayObjectName, "ShaderName" );
		animName		= gpm->getStringValue( gameplayObjectName, "AnimName" );

		hideFeatures	= (int) gpm->getFloatValue( gameplayObjectName, "HideFeatures" );

		if ( shaderName.length() )
		{
			clearCustomShader( shaderName );
			edict->s.renderfx &= ~RF_FORCE_ALPHA_EFFECTS;
		}

		if ( animate && animName.length() && animate->HasAnim( animName.c_str() ) )
		{
			animate->RandomAnimate( animName.c_str() );
		}

		CancelEventsOfType( EV_FadeNoRemove );

		if ( hideFeatures )
			showFeaturesForFade();

		setAlpha( 1.0f );

		event = new Event( EV_EntityRenderEffects );
		event->AddString( "+shadow" );
		ProcessEvent( event );

		takedamage = DAMAGE_YES;
		flags &= ~FL_IMMOBILE;
	}
	else if ( effectType == "FadeOut" )
	{
		str shaderName;
		str soundName;
		str effectTikiName;
		str effectPos;
		str currentEffectPos;
		float effectTime;
		float fadeTime;
		int numEffects;
		int attach;
		int hideFeatures;

		// Get all of the info from the gameplay database

		shaderName		= gpm->getStringValue( gameplayObjectName, "ShaderName" );
		soundName		= gpm->getStringValue( gameplayObjectName, "SoundName" );
		effectTikiName	= gpm->getStringValue( gameplayObjectName, "EffectName" );
		attach			= (int) gpm->getFloatValue( gameplayObjectName, "AttachEffect" );

		effectPos		= gpm->getStringValue( gameplayObjectName, "EffectPosName" );
		numEffects		= (int) gpm->getFloatValue( gameplayObjectName, "NumEffects" );

		effectTime		= gpm->getFloatValue( gameplayObjectName, "EffectTime" );
		fadeTime		= gpm->getFloatValue( gameplayObjectName, "FadeTime" );
		hideFeatures	= (int) gpm->getFloatValue( gameplayObjectName, "HideFeatures" );

		setCustomShader( shaderName );

		setAlpha( 0.0f );

		edict->s.renderfx |= RF_FORCE_ALPHA_EFFECTS;
		edict->s.renderfx &= ~RF_FORCE_ALPHA;

		if ( effectTime > 0.0f )
		{
			event = new Event( EV_FadeIn );
			event->AddFloat( effectTime );
			ProcessEvent( event );
		}

		event = new Event( EV_DisplayEffect );
		event->AddString( "FadeOut2" );
		event->AddString( effectName );
		PostEvent( event, effectTime  );

		if ( hideFeatures )
			hideFeaturesForFade();

		if ( edict->s.parent == ENTITYNUM_NONE )
		{
			// Play sound

			if ( soundName.length() )
			{
				Sound( soundName );
			}

			// Spawn an effect 

			if ( effectTikiName.length() > 4 )
			{
				Vector pos;

				currentEffectPos = effectPos;

				for ( i = 0 ; i < numEffects ; i++ )
				{
					if ( !effectPos.length() || effectPos == "centroid" )
						pos = centroid;
					else if ( effectPos == "origin" )
						pos = origin;
					else if ( effectPos == "randombone" )
					{
						int tagNum;

						tagNum = (int) G_Random( gi.NumTags( edict->s.modelindex ) );
						currentEffectPos = gi.Tag_NameForNum( edict->s.modelindex, tagNum );
						
						GetTag( tagNum, &pos, NULL );
					}
					else if ( gi.Tag_NumForName( edict->s.modelindex, effectPos.c_str() ) >= 0 )
						GetTag( effectPos, &pos, NULL );
					else
						pos = centroid;

					if ( attach )
						attachEffect( effectTikiName, currentEffectPos, effectTime + fadeTime );
					else
						SpawnEffect( effectTikiName, pos, angles, effectTime + fadeTime );
				}
			}
		}

		passEvent = true;
	}
	else if ( effectType == "FadeOut2" )
	{
		str shaderName;
		float fadeTime;

		// Get all of the info from the gameplay database

		shaderName = gpm->getStringValue( gameplayObjectName, "ShaderName" );
		fadeTime   = gpm->getFloatValue( gameplayObjectName, "FadeTime" );

		CancelEventsOfType( EV_FadeIn );
		edict->s.renderfx &= ~RF_FORCE_ALPHA_EFFECTS;

		setCustomShader( shaderName );
		setAlpha( 1.0f );
		edict->s.renderfx |= RF_FORCE_ALPHA;

		event = new Event( EV_FadeNoRemove );
		event->AddFloat( fadeTime );
		event->AddFloat( 0.0f );
		ProcessEvent( event );

		event = new Event( EV_DisplayEffect );
		event->AddString( "FadeOut3" );
		event->AddString( effectName );
		PostEvent( event, fadeTime  );
	}
	else if ( effectType == "FadeOut3" )
	{
		/* int hideFeatures;

		hideFeatures = gpm->getFloatValue( gameplayObjectName, "HideFeatures" );

		if ( hideFeatures )
			showFeaturesForFade(); */
	}
	else if ( effectType == "FadeIn" )
	{
		str shaderName;
		str soundName;
		str effectTikiName;
		str effectPos;
		str currentEffectPos;
		//float effectTime;
		float fadeTime;
		int numEffects;
		int attach;
		int hideFeatures;

		// Get all of the info from the gameplay database

		shaderName		= gpm->getStringValue( gameplayObjectName, "ShaderName" );
		soundName		= gpm->getStringValue( gameplayObjectName, "SoundName" );
		effectTikiName	= gpm->getStringValue( gameplayObjectName, "EffectName" );
		attach			= (int) gpm->getFloatValue( gameplayObjectName, "AttachEffect" );

		effectPos		= gpm->getStringValue( gameplayObjectName, "EffectPosName" );
		numEffects		= (int) gpm->getFloatValue( gameplayObjectName, "NumEffects" );

		//effectTime   = gpm->getFloatValue( gameplayObjectName, "EffectTime" );
		fadeTime		= gpm->getFloatValue( gameplayObjectName, "FadeTime" );
		hideFeatures	= (int) gpm->getFloatValue( gameplayObjectName, "HideFeatures" );

		setCustomShader( shaderName );

		setAlpha( 0.0f );

		edict->s.renderfx &= ~RF_FORCE_ALPHA_EFFECTS;
		edict->s.renderfx |= RF_FORCE_ALPHA;

		event = new Event( EV_FadeIn );
		event->AddFloat( fadeTime );
		ProcessEvent( event );

		event = new Event( EV_DisplayEffect );
		event->AddString( "FadeIn2" );
		event->AddString( effectName );
		PostEvent( event, fadeTime  );

		if ( hideFeatures )
			hideFeaturesForFade();

		if ( edict->s.parent == ENTITYNUM_NONE )
		{
			// Play sound

			if ( soundName.length() )
			{
				Sound( soundName );
			}

			// Spawn an effect 

			if ( effectTikiName.length() > 4 )
			{
				Vector pos;

				currentEffectPos = effectPos;

				for ( i = 0 ; i < numEffects ; i++ )
				{
					if ( !effectPos.length() || effectPos == "centroid" )
						pos = centroid;
					else if ( effectPos == "origin" )
						pos = origin;
					else if ( effectPos == "randombone" )
					{
						int tagNum;

						tagNum = (int) G_Random( gi.NumTags( edict->s.modelindex ) );
						currentEffectPos = gi.Tag_NameForNum( edict->s.modelindex, tagNum );
						
						GetTag( tagNum, &pos, NULL );
					}
					else if ( gi.Tag_NumForName( edict->s.modelindex, effectPos.c_str() ) >= 0 )
						GetTag( effectPos, &pos, NULL );
					else
						pos = centroid;

					if ( attach )
						attachEffect( effectTikiName, currentEffectPos, fadeTime + fadeTime );
					else
						SpawnEffect( effectTikiName, pos, angles, fadeTime + fadeTime );
				}
			}
		}

		passEvent = true;
	}
	else if ( effectType == "FadeIn2" )
	{
		str shaderName;
		float effectTime;

		// Get all of the info from the gameplay database

		shaderName = gpm->getStringValue( gameplayObjectName, "ShaderName" );
		effectTime   = gpm->getFloatValue( gameplayObjectName, "EffectTime" );

		CancelEventsOfType( EV_FadeIn );

		setAlpha( 1.0f );

		if ( shaderName.length() > 0 )
		{
			setCustomShader( shaderName );

			edict->s.renderfx |= RF_FORCE_ALPHA_EFFECTS;
			edict->s.renderfx |= RF_FORCE_ALPHA;

			event = new Event( EV_FadeNoRemove );
			event->AddFloat( effectTime );
			event->AddFloat( 0.0f );
			ProcessEvent( event );
		}

		event = new Event( EV_DisplayEffect );
		event->AddString( "FadeIn3" );
		event->AddString( effectName );
		PostEvent( event, effectTime  );
	}
	else if ( effectType == "FadeIn3" )
	{
		str shaderName;
		int hideFeatures;

		// Get all of the info from the gameplay database

		shaderName = gpm->getStringValue( gameplayObjectName, "ShaderName" );
		hideFeatures = (int) gpm->getFloatValue( gameplayObjectName, "HideFeatures" );

		CancelEventsOfType( EV_FadeNoRemove );

		edict->s.renderfx &= ~RF_FORCE_ALPHA_EFFECTS;
		edict->s.renderfx &= ~RF_FORCE_ALPHA;

		if ( hideFeatures )
			showFeaturesForFade();

		clearCustomShader( shaderName );
		setAlpha( 1.0f );
	}
	else if ( effectType == "start_invisibility" )
	{
		edict->s.eFlags |= EF_EFFECT_ELECTRIC;
		setAlpha( 0.1f );

		edict->s.renderfx |= RF_FORCE_ALPHA;

		addAffectingViewModes( gi.GetViewModeMask( "forcevisible" ) );

		passEvent = true;
	}
	else if ( effectType == "stop_invisibility" )
	{
		edict->s.eFlags &= ~EF_EFFECT_ELECTRIC;
		setAlpha( 1.0f );

		edict->s.renderfx &= ~RF_FORCE_ALPHA;

		removeAffectingViewModes( gi.GetViewModeMask( "forcevisible" ) );

		passEvent = true;
	}
	else if ( effectType == "electric" )
	{
		edict->s.eFlags |= EF_EFFECT_ELECTRIC;
		passEvent = true;
	}
	else if ( effectType == "noelectric" )
	{
		edict->s.eFlags &= ~EF_EFFECT_ELECTRIC;
		passEvent = true;
	}
	else if ( effectType == "failure" )
	{
		edict->s.eFlags |= EF_BEHAVIOR_FAILURE;
		passEvent = true;
	}
	else if ( effectType == "nofailure" )
	{
		edict->s.eFlags &= ~EF_BEHAVIOR_FAILURE;
		passEvent = true;
	}
	else if ( ( effectType == "transport_out" ) || ( effectType == "transport_in" ) || ( effectType == "borg_transport_out" ) ||
			  ( effectType == "borg_transport_in" ) )
	{
		gi.WDPrintf( "Support for %s will soon be removed please use the new way\n", effectType.c_str() );

		event = new Event( EV_DisplayEffect );

		if ( effectType == "transport_out" )
		{
			event->AddString( "TransportOut" );
			event->AddString( "Federation" );
		}
		else if ( effectType == "transport_in" )
		{
			event->AddString( "TransportIn" );
			event->AddString( "Federation" );
		}
		else if ( effectType == "borg_transport_out" )
		{
			event->AddString( "TransportOut" );
			event->AddString( "Borg" );
		}
		else if ( effectType == "borg_transport_in" )
		{
			event->AddString( "TransportIn" );
			event->AddString( "Borg" );
		}

		ProcessEvent( event );
		return;
	}

	if ( passEvent && bind_info && bind_info->numchildren )
	{
		for( i = 0 ; i < MAX_MODEL_CHILDREN ; i++ )
		{
			if ( bind_info->children[ i ] == ENTITYNUM_NONE )
				continue;

			ent = G_GetEntity( bind_info->children[ i ] );

			if ( ent )
			{
				/* if ( cancelEvents )
				{
					// Cancel other display events so we don't get any overlap

					ent->CancelEventsOfType( EV_DisplayEffect ); 
				} */

				ent->ProcessEvent( *ev );
			}
		}
	}
}

//----------------------------------------------------------------
// Name:			clearDisplayEffects
// Class:			Entity
//
// Description:		Clears all of the display type of effects and sets the alpha back to normal
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------

void Entity::clearDisplayEffects( void ) 
{
	if ( edict->s.eFlags | EF_EFFECTS )
	{
		edict->s.eFlags &= ~EF_EFFECTS;
		setAlpha( 1.0f );

		edict->s.renderfx &= ~RF_FORCE_ALPHA_EFFECTS;
		edict->s.renderfx &= ~RF_FORCE_ALPHA;
	}
}

void Entity::SpawnEffect( Event *ev ) 
	{
	//const str &name, const Vector &origin, const Vector &angles, float removeTime )
	str tagName;
	str modelName;
	Vector tagPos;
	Vector tagForward;
	Vector tagAngles;
	float removeTime;


	modelName = ev->GetString( 1 );

	tagName = ev->GetString( 2 );
	GetTag( tagName, &tagPos, &tagForward );

	tagAngles = tagForward.toAngles();

	if ( ev->NumArgs() > 2 )
		removeTime = ev->GetFloat( 3 );
	else
		removeTime = 0.0f;

	SpawnEffect( modelName, tagPos, tagAngles, removeTime );
	}

Entity *Entity::SpawnEffect( const str &name, const Vector &origin, const Vector &angles, float removeTime )
{
	Entity *newEntity;
	str modelName;
	int nameLength;

	nameLength = name.length();

	if ( stricmp( name.c_str() + nameLength - 4, ".gdb" ) == 0 )
	{
		str gameplayObjectName;
		GameplayManager *gpm;

		gpm = GameplayManager::getTheGameplayManager();

		gameplayObjectName = name;
		gameplayObjectName.CapLength( nameLength - 4 );

		if ( gpm->hasObject( gameplayObjectName ) )
		{
			modelName = gpm->getStringValue( gameplayObjectName, "ModelName" );
		}
		else
		{
			const char *dash;

			dash = strstr( gameplayObjectName.c_str(), "-" );

			if ( dash )
			{
				gameplayObjectName.CapLength( dash - gameplayObjectName.c_str() );
				gameplayObjectName += "-default";

				if ( gpm->hasObject( gameplayObjectName ) )
				{
					modelName = gpm->getStringValue( gameplayObjectName, "ModelName" );
				}
				else
				{
					gi.WDPrintf( "%s not find in the gameplay database\n", name.c_str() );
					return NULL;
				}
			}
			else
			{
				gi.WDPrintf( "%s not find in the gameplay database\n", name.c_str() );
				return NULL;
			}
		}
	}
	else
	{
		modelName = name;
	}

	newEntity = new Entity( ENTITY_CREATE_FLAG_ANIMATE );

	newEntity->setModel( modelName );

	newEntity->angles = angles;
	newEntity->setAngles();

	newEntity->setOrigin( origin );

	newEntity->setSolidType( SOLID_NOT );

	if ( removeTime > 0.0f )
		newEntity->PostEvent( EV_Remove, removeTime );

	newEntity->CancelEventsOfType( EV_ProcessInitCommands );
	newEntity->ProcessInitCommands( newEntity->edict->s.modelindex );

	newEntity->animate->RandomAnimate( "idle" );

	return newEntity;
}

Entity* Entity::SpawnSound( const str &sound, const Vector &pos, float volume, float removeTime )
{
	Entity *newEntity;
	Vector soundOrigin;
	newEntity = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
	newEntity->setOrigin( pos );
	newEntity->setSolidType( SOLID_NOT );

	if ( removeTime > 0.0f )
		newEntity->PostEvent( EV_Remove, removeTime );

	soundOrigin = pos;
	newEntity->CancelEventsOfType( EV_ProcessInitCommands );
	newEntity->ProcessInitCommands( newEntity->edict->s.modelindex );
	newEntity->Sound( sound , CHAN_BODY, volume, -1.0f, &soundOrigin );
	return newEntity;
}

void Entity::attachEffect( Event *ev ) 
	{
	str tagName;
	str modelName;
	//Vector tagPos;
	//Vector tagForward;
	//Vector tagAngles;
	float removeTime;


	modelName = ev->GetString( 1 );

	tagName = ev->GetString( 2 );
	//GetTag( tagName, &tagPos, &tagForward );

	//tagAngles = tagForward.toAngles();

	if ( ev->NumArgs() > 2 )
		removeTime = ev->GetFloat( 3 );
	else
		removeTime = 0.0f;

	attachEffect( modelName, tagName, removeTime );
	}

void Entity::attachEffect( const str &modelName, const str &tagName, float removeTime )
{
	Event *newEvent = new Event( EV_AttachModel );
	newEvent->AddString( modelName );
	newEvent->AddString( tagName );
	newEvent->AddFloat( 1.0f );
	newEvent->AddString( "" );
	newEvent->AddInteger( 0 );
	newEvent->AddFloat( removeTime );
	ProcessEvent( newEvent );
}

void Entity::ForceAlpha( Event *ev )
	{
	if ( ev->NumArgs() == 0 )
		edict->s.renderfx |= RF_FORCE_ALPHA;
	else if ( ev->GetBoolean( 1 ) )
		edict->s.renderfx |= RF_FORCE_ALPHA;
	else
		edict->s.renderfx &= ~RF_FORCE_ALPHA;
	}

void Entity::CreateEarthquake( Event *ev )
   {
	Earthquake *earthquake;
	float duration;
	float magnitude;
	float distance = 0;
	Event *newEvent;


	if ( origin == vec_zero )
	{
		gi.WDPrintf( "Earthquake being started when origin hasn't been set yet in model %s\n", model.c_str() ); 
		return;
	}

	magnitude = ev->GetFloat( 1 );
	duration  = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		distance = ev->GetFloat( 3 );

	earthquake = new Earthquake;

	newEvent = new Event( EV_SetOrigin );
	newEvent->AddVector( origin );
	earthquake->ProcessEvent( newEvent );

	newEvent = new Event( EV_Earthquake_SetMagnitude );
	newEvent->AddFloat( magnitude );
	earthquake->ProcessEvent( newEvent );

	newEvent = new Event( EV_Earthquake_SetDuration );
	newEvent->AddFloat( duration );
	earthquake->ProcessEvent( newEvent );

	if ( distance )
		{
		newEvent = new Event( EV_Earthquake_SetDistance );
		newEvent->AddFloat( distance );
		earthquake->ProcessEvent( newEvent );
		}

	earthquake->ProcessEvent( EV_Trigger_Effect );

	earthquake->PostEvent( EV_Remove, duration );
   }

void Entity::SetFloatVar
	(
   Event *ev
	)

	{
	str var_name;
	float value;

	var_name = ev->GetString( 1 );
	value    = ev->GetFloat( 2 );

	entityVars.SetVariable( var_name, value );
	}

void Entity::SetVectorVar
	(
   Event *ev
	)

	{
	str var_name;
	Vector value;

	var_name = ev->GetString( 1 );
	value    = ev->GetVector( 2 );

	entityVars.SetVariable( var_name, value );
	}

void Entity::SetStringVar
	(
   Event *ev
	)

	{
	str var_name;
	str value;

	var_name = ev->GetString( 1 );
	value    = ev->GetString( 2 );

	entityVars.SetVariable( var_name, value );
	}

void Entity::doesVarExist( Event *ev )
{
	ScriptVariable *var = NULL;


	var = entityVars.GetVariable( ev->GetString( 1 ) );

	if ( var )
		ev->ReturnFloat( 1.0f );
	else
		ev->ReturnFloat( 0.0f );
}

void Entity::GetFloatVar
	(
   Event *ev
	)

	{
	str var_name;
	ScriptVariable *var = NULL;

	var_name = ev->GetString( 1 );

	var = entityVars.GetVariable( var_name );

	if ( var )
		ev->ReturnFloat( var->floatValue() );
	else
		{
		gi.WDPrintf( "%s variable not found\n", var_name.c_str() );
		ev->ReturnFloat( 0.0f );
		}
	}

void Entity::RemoveVariable( Event* ev )
{
	str var_name;
	if( ev->NumArgs() > 0 )	
	{
		var_name = ev->GetString( 1 );
		if( var_name.length() > 0 ) 
		{
			entityVars.RemoveVariable( var_name );
		}
	}
}

void Entity::GetVectorVar
	(
   Event *ev
	)

	{
	str var_name;
	ScriptVariable *var = NULL;

	var_name = ev->GetString( 1 );

	var = entityVars.GetVariable( var_name );

	if ( var )
		ev->ReturnVector( var->vectorValue() );
	else
		{
		gi.WDPrintf( "%s variable not found\n", var_name.c_str() );
		ev->ReturnVector( Vector(0, 0, 0) );
		}
	}

void Entity::GetStringVar
	(
   Event *ev
	)

	{
	str var_name;
	ScriptVariable *var = NULL;

	var_name = ev->GetString( 1 );

	var = entityVars.GetVariable( var_name );

	if ( var )
		ev->ReturnString( var->stringValue() );
	else
		{
		gi.WDPrintf( "%s variable not found\n", var_name.c_str() );
		ev->ReturnString( "" );
		}
	}

void Entity::SetUserVar1
	(
   Event *ev
	)

	{
	entityVars.SetVariable( "uservar1", ev->GetString( 1 ) );
	}

void Entity::SetUserVar2
	(
   Event *ev
	)

	{
	entityVars.SetVariable( "uservar2", ev->GetString( 1 ) );
	}

void Entity::SetUserVar3
	(
   Event *ev
	)

	{
	entityVars.SetVariable( "uservar3", ev->GetString( 1 ) );
	}

void Entity::SetUserVar4
	(
   Event *ev
	)

	{
	entityVars.SetVariable( "uservar4", ev->GetString( 1 ) );
	}

Vector Entity::GetClosestCorner( const Vector &position )
   {
   Vector corner;  

   Vector closestCorner;
   Vector compare;
   float distance;
   float length;

   distance = 999999999.9f;

   for ( int i = 0 ; i < 4 ; i++ )
      {
      // corner's based on a top down view of the bounding box -- I am returning
      // the vector for the corner on a plane with the origin only.
      switch ( i )
         {
         // Upper Left
         case 0:
            corner.x = origin.x + mins.x;
            corner.y = origin.y + maxs.y;
            corner.z = origin.z;
         break;

         // Upper Right
         case 1:
            corner.x = origin.x + maxs.x;
            corner.y = origin.y + maxs.y;
            corner.z = origin.z;
         break;

         // Lower Left
         case 2:
            corner.x = origin.x + mins.x;
            corner.y = origin.y + mins.y;
            corner.z = origin.z;
         break;

         case 3:
            corner.x = origin.x + maxs.x;
            corner.y = origin.y + mins.y;
            corner.z = origin.z;
         break;
         }

      compare = corner - position;
      length = compare.length();

      if ( length < distance )
         {
         closestCorner = corner;
         distance = length;
         }
      }

   return closestCorner;
   
   }

//----------------------------------------------------------------
// Name:			affectingViewMode
// Class:			Entity
//
// Description:		Adds a new viewmode  to the entities applicable viewmodes
//
// Parameters:		Event *ev								- event (name of the view mode)
//
// Returns:			None
//----------------------------------------------------------------

void Entity::affectingViewMode( Event *ev )
	{
	addAffectingViewModes( gi.GetViewModeMask( ev->GetString( 1 ) ) );
	}

//----------------------------------------------------------------
// Name:			addAffectingViewMode
// Class:			Entity
//
// Description:		Adds the specified viewmode bits to the entities applicable viewmodes
//
// Parameters:		unsigned int mask						- the bit mask of the relevant viewmodes
//
// Returns:			None
//----------------------------------------------------------------

void Entity::addAffectingViewModes( unsigned int mask )
{
	_affectingViewModes |= mask;
	edict->s.affectingViewModes = _affectingViewModes;
}

void Entity::removeAffectingViewModes( unsigned int mask )
{
	_affectingViewModes &= ~mask;
	edict->s.affectingViewModes = _affectingViewModes;
}


//--------------------------------------------------------------
// Name:		SetGroupID()
// Class:		Entity
//
// Description:	Grabs the ID from the event, and sends it to the
//				group coordinator for registration.  In the future
//				we need to migrate this so that all group registration
//				is done throught he group coordinator alone.
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::SetGroupID( Event *ev )
{
	AddToGroup( ev->GetInteger( 1 ) );
}

void Entity::AddToGroup( int ID )
{
	groupcoordinator->AddEntityToGroup( this , ID );	
}

//----------------------------------------------------------------
// Name:			TikiTodo and TikiNote
// Class:			Entity
//
// Description:		These commands may come from tiki files (via TikiMaster).
//					They are here so the commands are not considered errors.
//					In the future we might want cvars to print todo items.
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Entity::TikiTodo( Event *ev )
	{
	}
void Entity::TikiNote( Event *ev )
	{
	}

//----------------------------------------------------------------
// Name:			MultiplayerEvent
// Class:			Entity
//
// Description:		This is a passthrough event.  It only allows the contained event to be called
//					if we are in a multiplayer game
//
// Parameters:		Event *ev					- contains the real event to process
//
// Returns:			None
//----------------------------------------------------------------

void Entity::MultiplayerEvent( Event *ev )
{
	// Make sure we are in a multiplayer game

	if ( multiplayerManager.inMultiplayer() )
	{
		Event *event;
		str eventName;
		str token;
		int i;

		// Get the event name

		eventName = ev->GetString( 1 );

		event = new Event( eventName );

		// Get all of the event parms

		for ( i = 2 ; i <= ev->NumArgs() ; i++ )
		{
			event->AddToken( ev->GetToken( i ) );
		}

		// Process the event

		ProcessEvent( event );
	}
}


//--------------------------------------------------------------
//
// Name:			AddDamageModifier
// Class:			Entity
//
// Description:		Adds a new damage modifier to this entities list
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Entity::AddDamageModifier( Event *ev )
{
	str damagemodtype, value;
	float multiplier, chance = 1.0f, painBaseLine = 50.0f;

	damagemodtype = ev->GetString( 1 );
	value = ev->GetString( 2 );
	multiplier = ev->GetFloat( 3 );
	if ( ev->NumArgs() > 3 )
		chance = ev->GetFloat( 4 );
	if ( ev->NumArgs() > 4 )
		painBaseLine = ev->GetFloat( 5 );

	if ( !damageModSystem )
		damageModSystem = new DamageModificationSystem;

	damageModSystem->addDamageModifier(damagemodtype, value, multiplier, chance, painBaseLine );
}

//--------------------------------------------------------------
//
// Name:			ResolveDamage
// Class:			Entity
//
// Description:		Calls the DamageModificationSystem to resolve damage
//
// Parameters:		Damage &damage -- Reference to damage to modifiy
//
// Returns:			
//
//--------------------------------------------------------------
void Entity::ResolveDamage( ::Damage &damage )
{
	if ( damageModSystem )
		damageModSystem->resolveDamage(damage);
}

//--------------------------------------------------------------
// Name:		setMoveType()
// Class:		Entity
//
// Description:	Will convert a string to the appropriate moveType
//				and then call setMoveType( type )
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::setMoveType( Event *ev )
{
	str type;
	type = ev->GetString( 1 );

	if ( !stricmp( type.c_str() , "none"			) )
		{
		velocity = vec_zero;
		setMoveType( MOVETYPE_NONE );
		}
	else if ( !stricmp( type.c_str() , "stationary" ) )
		setMoveType( MOVETYPE_STATIONARY );
	else if ( !stricmp( type.c_str() , "noclip"		) )
		setMoveType( MOVETYPE_NOCLIP );
	else if ( !stricmp( type.c_str() , "push"		) )
		setMoveType( MOVETYPE_PUSH );
	else if ( !stricmp( type.c_str() , "stop"		) )
		setMoveType( MOVETYPE_STOP );
	else if ( !stricmp( type.c_str() , "walk"		) )
		setMoveType( MOVETYPE_WALK );
	else if ( !stricmp( type.c_str() , "step"		) )
		setMoveType( MOVETYPE_STEP );
	else if ( !stricmp( type.c_str() , "fly"		) )
		setMoveType( MOVETYPE_FLY );
	else if ( !stricmp( type.c_str() , "toss"		) )
		setMoveType( MOVETYPE_TOSS );
	else if ( !stricmp( type.c_str() , "flymissile"	) )
		setMoveType( MOVETYPE_FLYMISSILE );
	else if ( !stricmp( type.c_str() , "bounce"		) )
		setMoveType( MOVETYPE_BOUNCE );
	else if ( !stricmp( type.c_str() , "slide"		) )
		setMoveType( MOVETYPE_SLIDE );
	else if ( !stricmp( type.c_str() , "rope"		) )
		setMoveType( MOVETYPE_ROPE );
	else if ( !stricmp( type.c_str() , "gib"		) )
		setMoveType( MOVETYPE_GIB );
	else if ( !stricmp( type.c_str() , "vehicle"	) )
		setMoveType( MOVETYPE_VEHICLE );
}


//===============================================================
// Name:		buildUseData
// Class:		Entity
//
// Description: Creates a usedata structure if need be.  Also
//				ensures the contents type of a usable object is
//				set to something that can be detected by the
//				use trace (so it can indeed be used).
//
//				Testing this idea out.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Entity::buildUseData
(
	void
)
{
	if ( useData ) return ;

	useData = new UseData();
}


//--------------------------------------------------------------
// Name:		useDataAnim
// Class:		Entity
//
// Description:	Sets the useData anim member
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::useDataAnim( Event *ev )
{
	buildUseData();	
	useData->setUseAnim(ev->GetString( 1 ));
}

//--------------------------------------------------------------
// Name:		useDataType
// Class:		Entity
//
// Description:	Sets the useData type member
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::useDataType( Event *ev )
{
	buildUseData();	
	useData->setUseType(ev->GetString( 1 ));
}

//--------------------------------------------------------------
// Name:		useDataThread
// Class:		Entity
//
// Description:	Sets the useData thread member
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::useDataThread( Event *ev )
{
	buildUseData();	
	useData->setUseThread(ev->GetString( 1 ));
}

//--------------------------------------------------------------
// Name:		useDataEvent
// Class:		Entity
//
// Description:	Sets the useData variables
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::useDataEvent( Event *ev )
{
	buildUseData();	

	useData->setUseAnim(ev->GetString( 1 ));
	useData->setUseType(ev->GetString( 2 ));
	useData->setUseThread(ev->GetString( 3 ));
}

//--------------------------------------------------------------
// Name:		useDataMaxDist
// Class:		Entity
//
// Description:	Sets the maximum distance this entity can be used.
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::useDataMaxDist( Event *ev )
{
	buildUseData();	
	useData->setUseMaxDist(ev->GetFloat( 1 ));
}

//--------------------------------------------------------------
// Name:		useDataCount
// Class:		Entity
//
// Description:	Sets the number of times this entity can be used.
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::useDataCount( Event *ev )
{
	buildUseData();	
	useData->setUseCount(ev->GetInteger( 1 ));
}


//--------------------------------------------------------------
// Name:		setArchetype
// Class:		Entity
//
// Description:	Sets the archetype name for this entity
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::setArchetype( Event *ev )
{
	_archetype = ev->GetString(1);

	//Sets the archetype index
	edict->s.archeTypeIndex = gi.archetypeindex(_archetype);
}


//--------------------------------------------------------------
// Name:		getArchetype
// Class:		Entity
//
// Description:	Gets the archetype name for this entity,
//
// Parameters:	None
//
// Returns:		const str
//--------------------------------------------------------------
const str Entity::getArchetype() const
{
	if ( _archetype.length() == 0 )
		return getName();

	return _archetype;
}

//-----------------------------------------------------
//
// Name:		setMissionObjective
// Class:		Entity
//
// Description:	Sets the entity to be a mission objective or not.
//
// Parameters:	ev 
//
// Returns:		None
//-----------------------------------------------------
void Entity::setMissionObjective(Event* ev)
{
	_missionObjective = ev->GetBoolean(1);
	edict->s.missionObjective = ev->GetBoolean(1);
	if( edict->s.missionObjective )
	{
		G_AddEntityToExtraList(entnum);
	}
	else
	{
		G_RemoveEntityFromExtraList(entnum);
	}
}


//--------------------------------------------------------------
// Name:		GetVelocity
// Class:		Entity
//
// Description:	Gets the Velocity of the Entity
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::GetVelocity( Event *ev )
{
	ev->ReturnVector( velocity );
}

//--------------------------------------------------------------
// Name:		SetVelocity
// Class:		Entity
//
// Description: Sets the Velocity of the Entity
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Entity::SetVelocity( Event *ev )
{
	velocity = ev->GetVector( 1 );
}

void Entity::startStasis( Event *ev )
{
	startStasis();
}

void Entity::stopStasis( Event *ev )
{
	stopStasis();
}

void Entity::startStasis( void )
{
	// Immobilize the character

	flags |= FL_IMMOBILE;
	flags |= FL_STUNNED;

	// Show an effect

	setCustomShader( "stasis" );

	if ( animate )
	{
		animate->StopAnimating();

		if ( edict->s.torso_anim & ANIM_BLEND )
			animate->StopAnimating( torso );
	}
}

void Entity::stopStasis( void )
{
	// Unimmobilize the character

	flags &= ~FL_IMMOBILE;
	flags &= ~FL_STUNNED;

	// Stop the effect

	clearCustomShader( "stasis" );

	if ( this->isSubclassOf( Actor ) )
	{
		Actor *actor = (Actor *)this;

		actor->resetStateMachine();
	}
	else if ( this->isSubclassOf( Player ) )
	{
		Player *player = (Player *)this;

		player->SetState("STAND", "START");
	}

}

void Entity::setTargetPos( Event *ev )
{
	setTargetPos( ev->GetString( 1 ) );
}

void Entity::setTargetPos( const str &targetPos )
{
	_targetPos = targetPos;
}

str	Entity::getTargetPos()
{
	return _targetPos;
}

void Entity::addHealthOverTime( Event *ev )
{
	Event *event;
	float healthToAdd;
	float healthToAddThisFrame;
	float timeLeft;
	int numFrames;

	healthToAdd = ev->GetFloat( 1 );
	timeLeft = ev->GetFloat( 2 );

	// Figure out how much health to add this frame

	if ( timeLeft < level.frametime )
		numFrames = 1;
	else
		numFrames = (int) (timeLeft / level.frametime);

	healthToAddThisFrame = healthToAdd / numFrames;

	// Actually add the health to the entity

	addHealth( healthToAddThisFrame );

	// Post the event for the next frame

	healthToAdd -= healthToAddThisFrame;
	timeLeft -= level.frametime;

	//CancelEventsOfType( EV_AddHealthOverTime );

	if ( timeLeft > 0.0f )
	{
		event = new Event( EV_AddHealthOverTime );
		event->AddFloat( healthToAdd );
		event->AddFloat( timeLeft );
		PostEvent( event, level.frametime );
	}
}

void Entity::simplePlayDialog( Event *ev )
{
	str dialogName;
	float volume   = DEFAULT_VOL;
	float min_dist = DEFAULT_MIN_DIST;
	char localizedDialogName[MAX_QPATH];
	Player *player;


	// Get all of the parms

	dialogName = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		volume = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
	{
		str minDistString;

		min_dist = ev->GetFloat( 3 );

		minDistString = ev->GetString( 3 );

		if ( stricmp( minDistString.c_str(), LEVEL_WIDE_STRING ) == 0 )
			min_dist = LEVEL_WIDE_MIN_DIST;
		else
			min_dist = ev->GetFloat( 3 );

		if ( min_dist >= LEVEL_WIDE_MIN_DIST_CUTOFF )
			min_dist = LEVEL_WIDE_MIN_DIST;
	}

	// Get the localized name of the dialog

	gi.LocalizeFilePath( dialogName.c_str(), localizedDialogName );

	// Play the sound

	Sound( localizedDialogName, CHAN_DIALOG, volume, min_dist );

	// Tell the player about the dialog

	player = GetPlayer( 0 );
	
	if ( player )
	{
		player->SetupDialog( NULL, localizedDialogName );
	}
}

void Entity::warp( Event *ev )
{
	Entity *ent;
	int i;

	if ( ev->NumArgs() > 0 )
		setOrigin( ev->GetVector( 1 ) );
	else
		setOrigin();

	setAngles();

	NoLerpThisFrame();

	if ( bind_info )
	{
		// Make sure everyone bound to us doesn't lerp

		for( ent = bind_info->teamchain; ent != NULL; ent = ent->bind_info->teamchain )
		{
			if ( ent->bind_info->teammaster == this )
			{
				ent->ProcessEvent( EV_Warp );
			}
		}

		// Make sure everyone attached bound to us doesn't lerp

		for( i = 0; i < MAX_MODEL_CHILDREN ; i++ )
		{
			if ( bind_info->children[ i ] == ENTITYNUM_NONE )
			{
				continue;
			}
			
			ent = G_GetEntity( bind_info->children[ i ] );

			if ( ent )
			{
				ent->ProcessEvent( EV_Warp );
			}
		}
	}
}

void Entity::traceHitsEntity( Event *ev )
{
	trace_t	trace;
	Vector end;
	Vector start;
	Vector dir;
	str tagName;
	float length;
	Entity *entityToCheck;


	// Get the event info

	tagName = ev->GetString( 1 );
	length = ev->GetFloat( 2 );
	entityToCheck = ev->GetEntity( 3 );

	// Determine the start and end point of the trace

	GetTag( tagName, &start, &dir );

	end = start + dir * length;

	// Do the actual trace

	trace = G_Trace( start, vec_zero, vec_zero, end, NULL, MASK_SHOT, false, "traceHitsEntity" );

	// Determine if we hit this entity

	if ( trace.ent && entityToCheck && trace.ent->entity == entityToCheck )
		ev->ReturnFloat( 1.0f );
	else
		ev->ReturnFloat( 0.0f );
}

void Entity::setOriginEveryFrame( Event *ev )
{
	Event *repost;
	repost = new Event(EV_SetOriginEveryFrame);

	setOrigin();

	PostEvent( repost , FRAMETIME );
	
}

void Entity::isWithinDistanceOf( Event *ev )
{
	Entity *destination;
	float distance;
	float returnValue;

	destination = ev->GetEntity( 1 );
	distance = ev->GetFloat( 2 );
	returnValue = 0.0f;

	if ( WithinDistance( destination , distance ) )
		returnValue = 1.0;

	ev->ReturnFloat( returnValue );
}

void Entity::setNetworkDetail( Event *ev )
{
	_networkDetail = true;
}

bool Entity::isNetworkDetail( void )
{
	return _networkDetail;
}
