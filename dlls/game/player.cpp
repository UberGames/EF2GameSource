//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/player.cpp                                $
// $Revision:: 590                                                            $
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
// Class definition of the player.
//
#include "_pch_cpp.h"
#include "entity.h"
#include "player.h"
#include "worldspawn.h"
#include "weapon.h"
#include "trigger.h"
#include "scriptmaster.h"
#include "path.h"
#include "navigate.h"
#include "misc.h"
#include "gravpath.h"
#include "armor.h"
#include "inventoryitem.h"
#include "gibs.h"
#include "actor.h"
#include "object.h"
#include "characterstate.h"
#include "weaputils.h"
#include "mp_manager.hpp"
#include "body.h"
#include "viewthing.h"
#include "equipment.h"
#include "powerups.h"
#include "gamecmds.h"
#include "teammateroster.hpp"
#include <qcommon/gameplaymanager.h>

//Forward
//Back
//TurnRight
//TurnLeft
//Moveleft (strafe)
//Moveright (strafe)
//Moveup (Jump)
//Movedown (Duck)
//Action (Use)
//Sneak (Toggle or Momentary)
//Speed/Walk (Toggle or Momentary)
//Fire Left hand
//Fire Right hand

#define SLOPE_45_MIN    0.7071f
#define SLOPE_45_MAX    0.831f
#define SLOPE_22_MIN    SLOPE_45_MAX
#define SLOPE_22_MAX    0.95f

#define PUSH_OBJECT_DISTANCE  16.0f

#define R_ARM_NAME "Bip01 R UpperArm"
#define L_ARM_NAME "Bip01 L UpperArm"

#define MELEE_ATTACK_LEFT	(1<<0)
#define MELEE_ATTACK_RIGHT	(1<<1)

const float	standardTorsoMult = 0.75f;

const Vector power_color( 0.0f, 1.0f, 0.0f );
const Vector acolor( 1.0f, 1.0f, 1.0f );
const Vector bcolor( 0.5f, 0.0f, 0.0f );

const Vector damageNormalColor( 0.5f, 0.0f, 0.0f );
const Vector damageFireColor( 0.5f, 0.25f, 0.0f );

qboolean TryPush( int entnum, vec3_t move_origin, vec3_t move_end );

Event EV_Player_DumpState
(
	"state",
	EV_CHEAT,
	NULL,
	NULL,
	"Dumps the player's state to the console."
);
Event EV_Player_ForceTorsoState
(
	"forcetorsostate",
	EV_DEFAULT,
	"s",
	"torsostate",
	"Force the player's torso to a certain state"
);
Event EV_Player_GiveAllCheat
(
	"wuss",
	EV_CHEAT,
	NULL,
	NULL,
	"Gives player all weapons."
);
Event EV_Player_EndLevel
(
	"endlevel",
	EV_DEFAULT,
	NULL,
	NULL,
	"Called when the player gets to the end of the level."
);
Event EV_Player_DevGodCheat
(
	"god",
	EV_CHEAT,
	"I",
	"god_mode",
	"Sets the god mode cheat or toggles it."
);
Event EV_Player_DevNoTargetCheat
(
	"notarget",
	EV_CHEAT,
	NULL,
	NULL,
	"Toggles the notarget cheat."
);
Event EV_Player_DevNoClipCheat
(
	"noclip",
	EV_CHEAT,
	NULL,
	NULL,
	"Toggles the noclip cheat."
);
Event EV_Player_PrevItem
(
	"invprev",
	EV_CONSOLE,
	NULL,
	NULL,
	"Cycle to player's previous item."
);
Event EV_Player_NextItem
(
	"invnext",
	EV_CONSOLE,
	NULL,
	NULL,
	"Cycle to player's next item."
);
Event EV_Player_GiveCheat
(
	"give",
	EV_CHEAT | EV_TIKIONLY | EV_SCRIPTONLY,
	"sI",
	"name amount",
	"Gives the player the specified thing (weapon, ammo, item, etc.) and optionally the amount."
);
Event EV_Player_GiveWeaponCheat
(
	"giveweapon",
	EV_CHEAT,
	"s",
	"weapon_name",
	"Gives the player the specified weapon."
);
Event EV_Player_UseItem
(
	"use",
	EV_CONSOLE,
	"sI",
	"name weapon_hand",
	"Use the specified weapon in the hand choosen (optional)."
);
Event EV_Player_GameVersion
(
	"gameversion",
	EV_CONSOLE,
	NULL,
	NULL,
	"Prints the game version."
);
Event EV_Player_Fov
(
	"fov",
	EV_CONSOLE,
	"F",
	"fov",
	"Sets the fov."
);
Event EV_Player_Dead
(
	"dead",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the player is dead."
);
Event EV_Player_SpawnEntity
(
	"spawn",
	EV_CHEAT | EV_SCRIPTONLY,
	"sSSSSSSSS",
	"entityname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
	"Spawns an entity."
);
Event EV_Player_SpawnActor
(
	"actor",
	EV_CHEAT,
	"sSSSSSSSS",
	"modelname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
	"Spawns an actor."
);
Event EV_Player_Respawn
(
	"respawn",
	EV_CODEONLY,
	NULL,
	NULL,
	"Respawns the player."
);
Event EV_Player_TestThread
(
	"testthread",
	EV_CHEAT,
	"sS",
	"scriptfile label",
	"Starts the named thread at label if provided."
);
Event EV_Player_ResetState
(
	"resetstate",
	EV_CHEAT,
	NULL,
	NULL,
	"Reset the player's state table."
);
Event EV_Player_WhatIs
(
	"whatis",
	EV_CHEAT,
	"i",
	"entity_number",
	"Prints info on the specified entity."
);
Event EV_Player_ActorInfo
(
	"actorinfo",
	EV_CHEAT,
	"i",
	"actor_number",
	"Prints info on the specified actor."
);
Event EV_Player_ShowHeuristics
(
	"showheuristics",
	EV_CHEAT,
	NULL,
	NULL,
	"Shows the current heuristic numbers"
);
Event EV_Player_KillEnt
(
	"killent",
	EV_CHEAT  | EV_SCRIPTONLY,
	"i",
	"entity_number",
	"Kills the specified entity."
);
Event EV_Player_KillClass
(
	"killclass",
	EV_CHEAT  | EV_SCRIPTONLY,
	"sI",
	"classname except_entity_number",
	"Kills all of the entities in the specified class."
);
Event EV_Player_RemoveEnt
(
	"removeent",
	EV_CHEAT  | EV_SCRIPTONLY,
	"i",
	"entity_number",
	"Removes the specified entity."
);
Event EV_Player_RemoveClass
(
	"removeclass",
	EV_CHEAT  | EV_SCRIPTONLY,
	"sI",
	"classname except_entity_number",
	"Removes all of the entities in the specified class."
);
Event EV_Player_ActivateNewWeapon
(
	"activatenewweapon",
	EV_DEFAULT,
	NULL,
	NULL,
	"Active the new weapon specified by useWeapon."
);
Event EV_Player_DeactivateWeapon
(
	"deactivateweapon",
	EV_DEFAULT,
	"s",
	"side",
	"Deactivate the weapon in the specified hand."
);
Event EV_Player_Jump
(
	"jump",
	EV_TIKIONLY,
	"f",
	"height",
	"Makes the player jump."
);
Event EV_Player_AnimLoop_Torso
(
	"animloop_torso",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the torso animation has finished."
);
Event EV_Player_AnimLoop_Legs
(
	"animloop_legs",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called when the legs animation has finished."
);
Event EV_Player_DoUse
(
	"usestuff",
	EV_DEFAULT,
	"E",
	"usingEntity",
	"Makes the player try to use whatever is in front of her."
);
Event EV_Player_SetHeadTarget
(
	"headtarget",
	EV_CONSOLE,
	NULL,
	NULL,
	"Sets the Makes the player try to use whatever is in front of her."
);
Event EV_Player_ListInventory
(
	"listinventory",
	EV_CONSOLE,
	NULL,
	NULL,
	"List of the player's inventory."
);
Event EV_Player_ClearTarget
(
	"cleartarget",
	EV_DEFAULT,
	NULL,
	NULL,
	"Clears the target of the player"
);
Event EV_Player_ActivateShield
(
	"activateshield",
	EV_DEFAULT,
	NULL,
	NULL,
	"Activates the player's shield"
);
Event EV_Player_DeactivateShield
(
	"deactivateshield",
	EV_DEFAULT,
	NULL,
	NULL,
	"Deactivates the player's shield"
);
Event EV_Player_AdjustTorso
(
	"adjust_torso",
	EV_DEFAULT,
	"b",
	"boolean",
	"Turn or off the torso adjustment"
);
Event EV_Player_DualWield
(
	"dualwield",
	EV_CONSOLE,
	"ss",
	"weaponleft weaponright",
	"Dual wield the specified weapons"
);
Event EV_Player_UseDualWield
(
	"usedualwield",
	EV_CONSOLE,
	NULL,
	NULL,
	"Use the weapons that are on the dual wield list"
);
Event EV_Player_EvaluateTorsoAnim
(
	"evaluatetorsoanim",
	EV_CONSOLE,
	NULL,
	NULL,
	"Evaluate the torso anim for possible changes"
);
Event EV_Player_Turn
(
	"turn",
	EV_SCRIPTONLY,
	"fF",
	"yawangle time",
	"Causes player to turn the specified amount."
);
Event EV_Player_TurnTowardsEntity
(
	"turnTowardsEntity",
	EV_SCRIPTONLY,
	"e",
	"entity",
	"Causes the player to turn towards the specified entity."
);
Event EV_Player_TurnUpdate
(
	"turnupdate",
	EV_SCRIPTONLY,
	"ff",
	"yaw timeleft",
	"Causes player to turn the specified amount."
);
Event EV_Player_TurnLegs
(
	"turnlegs",
	EV_SCRIPTONLY,
	"f",
	"yawangle",
	"Turns the players legs instantly by the specified amount."
);
Event EV_Player_DontTurnLegs
(
	"dontturnlegs",
	EV_CODEONLY,
	"b",
	"flag",
	"Specifies whether or not to turn the legs while strafing."
);
Event EV_Player_NextPainTime
(
	"nextpaintime",
	EV_CODEONLY,
	"f",
	"seconds",
	"Set the next time the player experiences pain (Current time + seconds specified)."
);
Event EV_Player_FinishUseAnim
(
	"finishuseanim",
	EV_DEFAULT,
	NULL,
	NULL,
	"Fires off all targets associated with a particular useanim."
);
Event EV_Player_Nightvision
(
	"nightvision",
	EV_CODEONLY | EV_CONSOLE,
	NULL,
	NULL,
	"Toggles player nightvision mode"
);
Event EV_Player_Holster
(
	"holster",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Holsters all wielded weapons, or unholsters previously put away weapons"
);
Event EV_Player_SafeHolster
(
	"safeholster",
	EV_SCRIPTONLY,
	"b",
	"putaway",
	"Holsters all wielded weapons, or unholsters previously put away weapons\n"
	"preserves state, so it will not holster or unholster unless necessary"
);
Event EV_Player_StartUseObject
(
	"startuseobject",
	EV_DEFAULT,
	NULL,
	NULL,
	"starts up the useobject's animations."
);
Event EV_Player_FinishUseObject
(
	"finishuseobject",
	EV_DEFAULT,
	NULL,
	NULL,
	"Fires off all targets associated with a particular useobject."
);
Event EV_Player_WatchActor
(
	"watchactor",
	EV_SCRIPTONLY,
	"eFFB",
	"entity_to_watch time angle watchEntireDuration",
	"Makes the player's camera watch the specified entity."
);
Event EV_Player_WatchEntity
(
	"watchentity",
	EV_SCRIPTONLY,
	"eFFB",
	"entityToWatch time angle watchEntireDuration",
	"Makes the player's camera watch the specified entity."
);
Event EV_Player_StopWatchingEntity
(
	"stopwatchingactor",
	EV_SCRIPTONLY,
	"e",
	"entity_to_stop_watching",
	"Makes the player's camera stop watching the specified entity."
);
Event EV_Player_PutawayWeapon
(
	"putawayweapon",
	EV_DEFAULT,
	"s",
	"whichHand",
	"Put away or deactivate the current weapon, whichHand can be left, right or dual."
);
Event EV_Player_Weapon
(
	"weaponcommand",
	EV_CODEONLY,
	"sSSSSSSS",
	"hand arg1 arg2 arg3 arg4 arg5 arg6 arg7",
	"Pass the args to the active weapon in the specified hand"
);
Event EV_Player_Done
(
	"playerdone",
	EV_CODEONLY,
	NULL,
	NULL,
	"Clears the waitForPlayer script command, allowing threads to run"
);
Event EV_Player_ActivateDualWeapons
(
	"activatedualweapons",
	EV_DEFAULT,
	NULL,
	NULL,
	"Activates 2 weapons at once"
);
Event EV_Player_StartCoolItem
(
	"startcoolitem",
	EV_CODEONLY,
	NULL,
	NULL,
	"Player is starting to show off the cool item"
);
Event EV_Player_StopCoolItem
(
	"stopcoolitem",
	EV_CODEONLY,
	NULL,
	NULL,
	"Player is starting to show off the cool item"
);
Event EV_Player_ShowCoolItem
(
	"showcoolitem",
	EV_CODEONLY,
	NULL,
	NULL,
	"Player is showing the cool item, actually display it"
);
Event EV_Player_HideCoolItem
(
	"hidecoolitem",
	EV_CODEONLY,
	NULL,
	NULL,
	"Player is finished showing the cool item, now hide it"
);
Event EV_Player_SetDamageMultiplier
(
	"damage_multiplier",
	EV_SCRIPTONLY,
	"f",
	"damage_multiplier",
	"Sets the current damage multiplier"
);
Event EV_Player_WaitForState
(
	"waitForState",
	EV_SCRIPTONLY,
	"s",
	"stateToWaitFor",
	"When set, the player will clear waitforplayer when this state is hit\n"
	"in the legs or torso."
);
Event EV_Player_LogStats
(
	"logstats",
	EV_CHEAT,
	"b",
	"state",
	"Turn on/off the debugging playlog"
);
Event EV_Player_TakePain
(
	"takepain",
	EV_DEFAULT,
	"b",
	"bool",
	"Set whether or not to take pain"
);
Event EV_Player_SkipCinematic
(
	"skipcinematic",
	EV_CONSOLE,
	NULL,
	NULL,
	"Skip the current cinematic"
);
Event EV_Player_ResetHaveItem
(
	"resethaveitem",
	EV_CONSOLE,
	"s",
	"weapon_name",
	"Resets the game var that keeps track that we have gotten this weapon"
);
Event EV_Player_Score
(
	"score",
	EV_CONSOLE,
	NULL,
	NULL,
	"Show the score for the current deathmatch game"
);
Event EV_Player_CallVote
(
	"callvote",
	EV_CONSOLE,
	"sS",
	"arg1 arg2",
	"Player calls a vote"
);
Event EV_Player_Vote
(
	"vote",
	EV_CONSOLE,
	"s",
	"arg1",
	"Player votes either yes or no"
);
Event EV_Player_JoinTeam
(
	"joinmpteam",
	EV_CONSOLE,
	"s",
	"teamName",
	"Makes the player join the specified team."
);
Event EV_Player_MultiplayerCommand
(
	"mpCmd",
	EV_CONSOLE,
	"ss",
	"command parm",
	"Sends a command from the player to the multiplayer system."
);
Event EV_Player_DeadBody
(
	"deadbody",
	EV_CODEONLY,
	NULL,
	NULL,
	"Spawn a dead body"
);
Event EV_Player_SetAimType
(
	"setaim",
	EV_DEFAULT,
	"s",
	"aimtype",
	"Sets the accuracy modifiers for the player"
);
Event EV_Player_ReloadWeapon
(
	"forcereload",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Tells the weapon to reload"
);
Event EV_Player_AnimateWeapon
(
	"animateweapon",
	EV_DEFAULT,
	"sSB",
	"anim hand animatingFlag",
	"Animates the weapon, optionally, in the specific hand (defaults to DUAL)"
	"If the animatingFlag is set to false, it will not set the WEAPON_ANIMATING"
	"as the status for this animation."
);
Event EV_Player_SwitchWeaponMode
(
	"switchmode",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Tells the weapon to switch modes of fire"
);
Event EV_Player_ReloadTiki
(
	"reloadviewtiki",
	EV_CHEAT,
	"s",
	"tikiname",
	"Reloads the specified TIKI file from disk"
);
Event EV_Player_SetViewAngles
(
	"playerviewangles",
	EV_CHEAT,
	"v[0,360][0,360][0,360]",
	"newAngles",
	"set the angles of the player to newAngles."
 );
Event EV_Player_ProjDetonate
(
	"projdetonate",
	EV_CODEONLY,
	"b",
	"detonate",
	"Event that sets the whether or not to detonate a trigger projectile (grenade launcher)"
);
Event EV_Player_UseEntity
(
	"useentity",
	EV_DEFAULT,
	"e",
	"entity",
	"makes player use the passed in entity"
);
Event EV_Player_SetupDialog
(
	"setupdialog",
	EV_CODEONLY,
	"is",
	"entnum soundname",
	"Sets up the dialog for the player"
);
Event EV_Player_ClearDialog
(
	"cleardialog",
	EV_CODEONLY,
	NULL,
	NULL,
	"Clears the dialog for the player"
);

Event EV_Player_ClearTextDialog
(
	"cleartextdialog",
	EV_CODEONLY,
	NULL,
	NULL,
	"Clears the text dialog of the player"
);

//
// Objective Events
//
Event EV_Player_LoadObjectives
(
	"loadobjectives",
	EV_SCRIPTONLY,
	"s",
	"name",
	"Loads the a set of objectives"
);
Event EV_Player_SetObjectiveShow
(
	"setobjectiveshow",
	EV_SCRIPTONLY,
	"sb",
	"name show",
	"Sets whether or not a specified objective is shown"
);
Event EV_Player_SetObjectiveComplete
(
	"setobjectivecomplete",
	EV_SCRIPTONLY,
	"sb",
	"name complete",
	"Sets whether or not a specified objective is complete"
);
Event EV_Player_SetObjectiveFailed
(
	"setobjectivefailed",
	EV_SCRIPTONLY,
	"sb",
	"name failed",
	"Sets whether or not a specified objective is failed"
);

//
// Information Events
//
Event EV_Player_SetInformationShow
(
	"setinformationshow",
	EV_SCRIPTONLY,
	"sb",
	"name show",
	"Sets whether or not a specified information is shown"
);
Event EV_Player_MissionFailed
(
	"missionfailed",
	EV_SCRIPTONLY,
	"S",
	"reason",
	"Displays the mission failed screen on the client side"
);
Event EV_Player_SetStat
(
	"setstat",
	EV_SCRIPTONLY,
	"si",
	"stat_name stat_value",
	"Sets a stat value"
);
Event EV_Player_SpecialMoveChargeStart
(
	"specialmovestart",
	EV_CODEONLY,
	NULL,
	NULL,
	"Starts charging up for a special move"
);
Event EV_Player_SpecialMoveChargeEnd
(
	"specialmoveend",
	EV_CODEONLY,
	NULL,
	NULL,
	"Ends charging up for a special move"
);
Event EV_Player_SpecialMoveChargeTime
(
	"specialmovetime",
	EV_CODEONLY,
	"f",
	"time",
	"Sets how long to wait before triggering the special move."
);
Event EV_Player_ChangeChar
(
	"changechar",
	EV_SCRIPTONLY,
	"ssss",
	"statefile model replacemodelName spawnNPC",
	"Changes the character state and model."
);
Event EV_Player_SetPlayerCharacter
(
	"setplayerchar",
	EV_SCRIPTONLY,
	"ss",
	"statefile model",
	"Sets the player character."
);
Event EV_Player_PlayerKnockback
(
	"playerknockback",
	EV_CODEONLY,
	"f",
	"knockback",
	"Sets knockback for the player"
);
Event EV_Player_KnockbackMultiplier
(
	"knockbackmultiplier",
	EV_CODEONLY,
	"i",
	"knockbackmult",
	"Sets knockback multiplierfor the player"
);
Event EV_Player_Melee
(
	"melee",
	EV_DEFAULT,
	"FSF",
	"damage means_of_death knockback",
	"Makes the player do a weapon-less melee attack. "
);
Event EV_Player_ChangeStance
(
	"stance",
	EV_CONSOLE,
	"i",
	"stanceNumber",
	"Makes the player change to the specified stance"
);
Event EV_Player_ClearStanceTorso
(
	"clearstancetorso",
	EV_CODEONLY,
	NULL,
	NULL,
	"Clears internal torso stance data."
);
Event EV_Player_ClearStanceLegs
(
	"clearstancelegs",
	EV_CODEONLY,
	NULL,
	NULL,
	"Clears internal legs stance data."
);
Event EV_Player_GivePoints
(
	"givepoints",
	EV_SCRIPTONLY,
	"i",
	"points",
	"Gives the player points (added to current points)"
);
Event EV_Player_SetPoints
(
	"setpoints",
	EV_SCRIPTONLY,
	"i",
	"points",
	"Sets the players number of points."
);
Event EV_Player_ChangeCharFadeIn
(
	"changecharfadein",
	EV_CODEONLY,
	NULL,
	NULL,
	"Begins the fade in and swaps character."
);
Event EV_Player_ClearIncomingMelee
(
	"clearincomingmelee",
	EV_CODEONLY,
	NULL,
	NULL,
	"Clears the incoming melee flag."
);
Event EV_Player_BendTorso
(
	"bendtorso",
	EV_DEFAULT,
	"F",
	"multiplier",
	"Sets multiplier for the torso bend angles, defaults to 0.75"
);
Event EV_Player_RemovePowerup
(
	"removepowerup",
	EV_CODEONLY,
	NULL,
	NULL,
	"Removes the current powerup."
);
Event EV_Player_DropRune
(
	"droprune",
	EV_CODEONLY,
	NULL,
	NULL,
	"Drops the player's current rune."
);
Event EV_Player_SetCanTransferEnergy
(
	"canTransferEnergy",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the player able to transfer energy from ammo to armor."
);
Event EV_Player_SetDoDamageScreenFlash
(
	"doDamageScreenFlash",
	EV_DEFAULT,
	"B",
	"bool",
	"Makes the screen flash when the player is damaged (or turns it off)."
);
Event EV_Player_MeleeDamageStart
(
	"meleedamagestart",
	EV_DEFAULT,
	"S",
	"hand",
	"Start doing melee damage with the weapon in the specified hand."
);
Event EV_Player_MeleeDamageEnd
(
	"meleedamageend",
	EV_DEFAULT,
	"S",
	"hand",
	"Stop doing melee damage with the weapon in the specified hand."
);
Event EV_Player_PointofView
(
	"pointofview",
	EV_DEFAULT,
	NULL,
	NULL,
	"Changes the viewpoint of the player, alternates between 1st and 3rd person"
);
Event EV_Player_FinishingMove
(
	"finishingmove",
	EV_TIKIONLY,
	"ssFFFF",
	"state direction coneangle distance enemyyaw chance",
	"Changes the viewpoint of the player, alternates between 1st and 3rd person"
);
Event EV_Player_ClearFinishingMove
(
	"clearfinishingmovelist",
	EV_DEFAULT,
	NULL,
	NULL,
	"Clears the finishing move list."
);
Event EV_Player_DoFinishingMove
(
	"dofinishingmove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Fires off the finishing move."
);
Event EV_Player_ForceTimeScale
(
	"forcetimescale",
	EV_DEFAULT,
	"F",
	"timescale",
	"Sets the timescale for the game"
);
Event EV_Player_Freeze
(
	"freeze",
	EV_DEFAULT,
	"B",
	"freeze",
	"Freezes the player until the freeze 0 is called"
);
Event EV_Player_Immobilize
(
	"immobilize",
	EV_DEFAULT,
	"B",
	"immobilize",
	"Immobilizes the player until the immobilize 0 is called (can only look around)"
);
Event EV_Player_DoUseEntity
(
	"douseentity",
	EV_DEFAULT,
	NULL,
	NULL,
	"Called from the tiki on a frame to use an entity."
);
Event EV_Player_DoneUseEntity
(
	"doneuseentity",
	EV_DEFAULT,
	NULL,
	NULL,
	"When the useentity animation is complete"
);
Event EV_Player_SetAttackType
(
	"attacktype",
	EV_DEFAULT,
	"s",
	"attacktype",
	"Sets the attack type of the attack"
);
Event EV_Player_NextGameplayAnim
(
	"nextgameplayanim",
	EV_CODEONLY,
	"s",
	"objname",
	"Increments the gameplay animation index."
);
Event EV_Player_SetGameplayAnim
(
	"setgameplayanim",
	EV_CODEONLY,
	"I",
	"index",
	"Sets the gameplay animation index directly."
);
Event EV_Player_DisableUseWeapon
(
	"disableuseweapon",
	EV_DEFAULT,
	"B",
	"disable",
	"Disables the weapon use"
);
Event EV_Player_DisableInventory
(
	"disableinventory",
	EV_DEFAULT,
	"f",
	"disable",
	"Disables the player's inventory"
);

Event EV_Player_EquipItems
(
	"equipitems",
	EV_DEFAULT,
	NULL,
	NULL,
	"Equips active items from the database."
);
Event EV_Player_AddRosterTeammate1
(
	"addrosterteammate1",
	EV_DEFAULT,
	"e",
	"entity",
	"Sets the teammate to roster position 1"
);
Event EV_Player_AddRosterTeammate2
(
	"addrosterteammate2",
	EV_DEFAULT,
	"e",
	"entity",
	"Sets the teammate to roster position 2"
);
Event EV_Player_AddRosterTeammate3
(
	"addrosterteammate3",
	EV_DEFAULT,
	"e",
	"entity",
	"Sets the teammate to roster position 3"
);
Event EV_Player_AddRosterTeammate4
(
	"addrosterteammate4",
	EV_DEFAULT,
	"e",
	"entity",
	"Sets the actor to roster position 4"
);
Event EV_Player_RemoveRosterTeammate1
(
	"removerosterteammate1",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removed the teammate from roster position 1"
);
Event EV_Player_RemoveRosterTeammate2
(
	"removerosterteammate2",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removed the teammate from roster position 2"
);
Event EV_Player_RemoveRosterTeammate3
(
	"removerosterteammate3",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removed the teammate from roster position 3"
);
Event EV_Player_RemoveRosterTeammate4
(
	"removerosterteammate4",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removed the teammate from roster position 4"
);

Event EV_Player_HudPrint
(
	"hudPrint",
	EV_DEFAULT,
	"s",
	"string",
	"Prints to the hud."
);
Event EV_Player_ClearItemText
(
	"clearItemText",
	EV_CODEONLY,
	NULL,
	NULL,
	"Clears the item text."
);
Event EV_Player_ValidPlayerModel
(
	"validPlayerModel",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Specifies that this model is valid for the player."
);
Event EV_Player_AddHud
(
	"addHud",
	EV_SCRIPTONLY,
	"s",
	"hudName",
	"Tells the player to add a hud to his display."
);
Event EV_Player_RemoveHud
(
	"removeHud",
	EV_SCRIPTONLY,
	"s",
	"hudName",
	"Tells the player to remove a hud from his display."
);

Event EV_Player_KillAllDialog
(
	"killalldialog",
	EV_DEFAULT,
	NULL,
	NULL,
	"Calls stopdialog on all the actors in the level"
);
Event EV_Player_ForceMoveType
(
	"forceMoveType",
	EV_DEFAULT,
	"s",
	"moveTypeName",
	"Forces the player to use the named move type"
);
Event EV_Player_IsOnGround
(
	"isplayeronground",
	EV_DEFAULT,
	"@f",
	"onground",
	"Checks if the player is on the ground -- returns 1 if true 0 if false"
);
Event EV_Player_BackpackAttachOffset
(
	"backpackAttachOffset",
	EV_DEFAULT,
	"v",
	"offset",
	"Sets the attachment offset for backpacks"
);
Event EV_Player_BackpackAttachAngles
(
	"backpackAttachAngles",
	EV_DEFAULT,
	"v",
	"anglesOffset",
	"Sets the attachment angles offset for backpacks"
);
Event EV_Player_FlagAttachOffset
(
	"flagAttachOffset",
	EV_DEFAULT,
	"v",
	"offset",
	"Sets the attachment offset for flags"
);
Event EV_Player_FlagAttachAngles
(
	"flagAttachAngles",
	EV_DEFAULT,
	"v",
	"anglesOffset",
	"Sets the attachment angles offset for flags"
);
Event EV_Player_BackupModel
(
	"backupModel",
	EV_DEFAULT,
	"s",
	"backupModelName",
	"Sets the name of the model to use as backup if the client doesn't have this one"
);

/*
==============================================================================

PLAYER

==============================================================================
*/

CLASS_DECLARATION( Sentient, Player, "player" )
{
	{ &EV_ClientMove,										&Player::ClientThink },
	{ &EV_ClientEndFrame,									&Player::EndFrame },
	{ &EV_Vehicle_Enter,									&Player::EnterVehicle },
	{ &EV_Vehicle_Exit,										&Player::ExitVehicle },
	{ &EV_Player_EndLevel,									&Player::EndLevel },
	{ &EV_Player_UseItem,									&Player::EventUseItem },
	{ &EV_Player_GiveCheat,									&Player::GiveCheat },
	{ &EV_Player_GiveWeaponCheat,							&Player::GiveWeaponCheat },
	{ &EV_Player_GiveAllCheat,								&Player::GiveAllCheat },
	{ &EV_Player_DevGodCheat,								&Player::GodCheat },
	{ &EV_Player_DevNoTargetCheat,							&Player::NoTargetCheat },
	{ &EV_Player_DevNoClipCheat,							&Player::NoclipCheat },
	{ &EV_Player_GameVersion,								&Player::GameVersion },
	{ &EV_Player_DumpState,	    							&Player::DumpState },
	{ &EV_Player_ForceTorsoState,							&Player::ForceTorsoState },
	{ &EV_Player_Fov,										&Player::Fov },
	{ &EV_Kill,												&Player::Kill },
	{ &EV_Player_Dead,										&Player::Dead },
	{ &EV_Player_SpawnEntity,								&Player::SpawnEntity },
	{ &EV_Player_SpawnActor,								&Player::SpawnActor },
	{ &EV_Player_Respawn,									&Player::Respawn },
	{ &EV_Player_DoUse,										&Player::DoUse },
	{ &EV_Pain,												&Player::Pain },
	{ &EV_Killed,											&Player::Killed },
	{ &EV_Gib,												&Player::GibEvent },
	{ &EV_GotKill,											&Player::GotKill },
	{ &EV_Player_TestThread,								&Player::TestThread },
	{ &EV_Player_ResetState,								&Player::ResetState },
	{ &EV_Player_WhatIs,									&Player::WhatIs },
	{ &EV_Player_ActorInfo,									&Player::ActorInfo },
	{ &EV_Player_ShowHeuristics,							&Player::ShowHeuristics	},
	{ &EV_Player_KillEnt,									&Player::KillEnt },
	{ &EV_Player_RemoveEnt,									&Player::RemoveEnt },
	{ &EV_Player_KillClass,									&Player::KillClass },
	{ &EV_Player_RemoveClass,								&Player::RemoveClass },
	{ &EV_Player_AnimLoop_Legs,								&Player::EndAnim_Legs },
	{ &EV_Player_AnimLoop_Torso,							&Player::EndAnim_Torso },
	{ &EV_Player_Jump,               						&Player::Jump },
	{ &EV_Sentient_JumpXY,									&Player::JumpXY },
	{ &EV_Player_ActivateNewWeapon,							&Player::ActivateNewWeapon },
	{ &EV_Player_DeactivateWeapon,							&Player::DeactivateWeapon },
	{ &EV_Player_SetHeadTarget,								&Player::SetHeadTarget },
	{ &EV_Player_ListInventory,								&Player::ListInventoryEvent },
	{ &EV_Player_ClearTarget,								&Player::ClearTarget },
	{ &EV_Player_ActivateShield,							&Player::ActivateShield },
	{ &EV_Player_DeactivateShield,							&Player::DeactivateShield },
	{ &EV_Player_AdjustTorso,								&Player::AdjustTorso },
	{ &EV_Player_DualWield,									&Player::DualWield },
	{ &EV_Player_UseDualWield,								&Player::UseDualWield },
	{ &EV_Player_EvaluateTorsoAnim,							&Player::EvaluateTorsoAnim },
	{ &EV_Player_NextPainTime,								&Player::NextPainTime },
	{ &EV_Player_Turn,										&Player::Turn },
	{ &EV_Player_TurnTowardsEntity,							&Player::turnTowardsEntity },
	{ &EV_Player_TurnUpdate,								&Player::TurnUpdate },
	{ &EV_Player_TurnLegs,									&Player::TurnLegs },
	{ &EV_Player_DontTurnLegs,								&Player::DontTurnLegs },
	{ &EV_Player_FinishUseAnim,								&Player::FinishUseAnim },
	{ &EV_Sentient_SetMouthAngle,							&Player::SetMouthAngle },
	{ &EV_Player_Holster,									&Player::HolsterToggle },
	{ &EV_Player_Nightvision,								&Player::NightvisionToggle },
	{ &EV_Player_SafeHolster,								&Player::Holster },
	{ &EV_Player_StartUseObject,							&Player::StartUseObject },
	{ &EV_Player_FinishUseObject,							&Player::FinishUseObject },
	{ &EV_Player_WatchActor,								&Player::WatchEntity },
	{ &EV_Player_WatchEntity,								&Player::WatchEntity },
	{ &EV_Player_StopWatchingEntity,						&Player::StopWatchingEntity },
	{ &EV_Player_PutawayWeapon,								&Player::PutawayWeapon },
	{ &EV_Player_Weapon,									&Player::WeaponCommand },
	{ &EV_Player_Done,										&Player::PlayerDone },
	{ &EV_Player_ActivateDualWeapons,						&Player::ActivateDualWeapons },
	{ &EV_Player_StartCoolItem,								&Player::StartCoolItem },
	{ &EV_Player_StopCoolItem,								&Player::StopCoolItem },
	{ &EV_Player_ShowCoolItem,								&Player::ShowCoolItem },
	{ &EV_Player_HideCoolItem,								&Player::HideCoolItem },
	{ &EV_Player_SetDamageMultiplier,						&Player::SetDamageMultiplier },
	{ &EV_Player_WaitForState,								&Player::WaitForState },
	{ &EV_Player_LogStats,									&Player::LogStats },
	{ &EV_Player_TakePain,									&Player::SetTakePain },
	{ &EV_Player_SkipCinematic,								&Player::SkipCinematic },
	{ &EV_Player_ResetHaveItem,								&Player::ResetHaveItem },
	{ &EV_Show,												&Player::PlayerShowModel },
	{ &EV_Player_Score,										&Player::Score },
	{ &EV_Player_CallVote,									&Player::CallVote },
	{ &EV_Player_Vote,										&Player::Vote },
	{ &EV_Player_JoinTeam,									&Player::joinTeam },
	{ &EV_Player_MultiplayerCommand,						&Player::multiplayerCommand },
	{ &EV_Player_DeadBody,									&Player::DeadBody },
	{ &EV_Player_SetAimType,								&Player::SetAimType },
	{ &EV_Player_ReloadWeapon,								&Player::ReloadWeapon },
	{ &EV_Player_AnimateWeapon,								&Player::AnimateWeapon },	
	{ &EV_Player_SwitchWeaponMode,							&Player::SwitchWeaponMode },
	{ &EV_Player_ReloadTiki,								&Player::ReloadTiki },
	{ &EV_Player_SetViewAngles,								&Player::SetViewAnglesEvent },
	{ &EV_Player_ProjDetonate,								&Player::ProjDetonate },
	{ &EV_Player_UseEntity,									&Player::UseSpecifiedEntity },
	{ &EV_Player_SetupDialog,								&Player::SetupDialog },
	{ &EV_Player_ClearDialog,								&Player::ClearDialog },
	{ &EV_Player_ClearTextDialog,							&Player::ClearTextDialog },
	{ &EV_Player_LoadObjectives,							&Player::LoadObjectives },
	{ &EV_Player_SetObjectiveShow,							&Player::SetObjectiveShow },
	{ &EV_Player_SetObjectiveComplete,						&Player::SetObjectiveComplete },
	{ &EV_Player_SetObjectiveFailed,						&Player::SetObjectiveFailed },
	{ &EV_Player_SetInformationShow,						&Player::SetInformationShow },
	{ &EV_Player_MissionFailed,								&Player::MissionFailed },
	   
	{ &EV_Player_SetStat,									&Player::SetStat },
	{ &EV_Player_SpecialMoveChargeStart,					&Player::SpecialMoveChargeStart },
	{ &EV_Player_SpecialMoveChargeEnd,						&Player::SpecialMoveChargeEnd },
	{ &EV_Player_SpecialMoveChargeTime,						&Player::SpecialMoveChargeTime },
	{ &EV_Player_ChangeChar,								&Player::ChangeChar },
	{ &EV_Player_SetPlayerCharacter,						&Player::SetPlayerChar },
	{ &EV_Player_PlayerKnockback,							&Player::PlayerKnockback },
	{ &EV_Player_KnockbackMultiplier,						&Player::KnockbackMultiplier },
	{ &EV_Player_Melee,										&Player::MeleeEvent },
	{ &EV_Player_ChangeStance,								&Player::ChangeStance },
	{ &EV_Player_ClearStanceTorso,							&Player::ClearStanceTorso },
	{ &EV_Player_ClearStanceLegs,							&Player::ClearStanceLegs },
	   
	{ &EV_Player_MeleeDamageStart,							&Player::MeleeDamageStart },
	{ &EV_Player_MeleeDamageEnd,							&Player::MeleeDamageEnd },
	{ &EV_Player_GivePoints,								&Player::GivePointsEvent },
	{ &EV_Player_SetPoints,									&Player::SetPointsEvent },
	{ &EV_Player_ChangeCharFadeIn,							&Player::ChangeCharFadeIn },
	{ &EV_Player_ClearIncomingMelee,						&Player::ClearIncomingMelee },
	{ &EV_Player_BendTorso,									&Player::SetBendTorso },
	{ &EV_Sentient_HeadWatchAllowed,						&Player::HeadWatchAllowed },
	   
	{ &EV_Player_RemovePowerup,								&Player::removePowerupEvent },
	{ &EV_Player_DropRune,									&Player::dropRune },
	{ &EV_Sentient_AddMeleeAttacker,						&Player::AddMeleeAttacker },
	   
	{ &EV_Player_SetCanTransferEnergy,						&Player::setCanTransferEnergy },
	{ &EV_Player_SetDoDamageScreenFlash,					&Player::setDoDamageScreenFlash },
	{ &EV_Player_PointofView,								&Player::pointOfView },
	{ &EV_Player_FinishingMove,								&Player::addFinishingMove },
	{ &EV_Player_ClearFinishingMove,						&Player::clearFinishingMove },
	{ &EV_Player_DoFinishingMove,							&Player::doFinishingMove },
	{ &EV_Player_ForceTimeScale,							&Player::forceTimeScale },
	{ &EV_Player_Freeze,									&Player::freezePlayer },
	{ &EV_Player_Immobilize,								&Player::immobilizePlayer },
	{ &EV_Player_DoUseEntity,								&Player::doUseEntity },
	{ &EV_Player_DoneUseEntity,								&Player::doneUseEntity },
	{ &EV_Player_SetAttackType,								&Player::setAttackType },
	{ &EV_Player_NextGameplayAnim,							&Player::nextGameplayAnim },
	{ &EV_Player_SetGameplayAnim,							&Player::setGameplayAnim },
	{ &EV_Player_DisableUseWeapon,							&Player::setDisableUseWeapon },
	{ &EV_Player_DisableInventory,							&Player::setDisableInventory },
	{ &EV_Player_EquipItems,								&Player::equipItems },
	   
	{ &EV_Use,												&Player::usePlayer },
	   
	{ &EV_Player_AddRosterTeammate1,						&Player::addRosterTeammate1 },
	{ &EV_Player_AddRosterTeammate2,						&Player::addRosterTeammate2 },
	{ &EV_Player_AddRosterTeammate3,						&Player::addRosterTeammate3 },
	{ &EV_Player_AddRosterTeammate4,						&Player::addRosterTeammate4 },
	   
	{ &EV_Player_RemoveRosterTeammate1,						&Player::removeRosterTeammate1 },
	{ &EV_Player_RemoveRosterTeammate2,						&Player::removeRosterTeammate2 },
	{ &EV_Player_RemoveRosterTeammate3,						&Player::removeRosterTeammate3 },
	{ &EV_Player_RemoveRosterTeammate4,						&Player::removeRosterTeammate4 },

	{ &EV_Player_HudPrint,									&Player::hudPrint },

	{ &EV_Player_ClearItemText,								&Player::clearItemText },

	{ &EV_Player_ValidPlayerModel,							&Player::setValidPlayerModel },

	{ &EV_Player_AddHud,									&Player::addHud },
	{ &EV_Player_RemoveHud,									&Player::removeHud },
		   
	//Vehicle Event Handoff
	{ &EV_Driver_AnimDone,									&Player::PassEventToVehicle },

	{ &EV_Warp,												&Player::warp },
	{ &EV_Player_KillAllDialog,								&Player::killAllDialog	},

	{ &EV_Player_ForceMoveType,								&Player::forceMoveType },
	{ &EV_Player_IsOnGround,								&Player::isPlayerOnGround },

	{ &EV_Player_BackpackAttachOffset,						&Player::setBackpackAttachOffset },
	{ &EV_Player_BackpackAttachAngles,						&Player::setBackpackAttachAngles },

	{ &EV_Player_FlagAttachOffset,							&Player::setFlagAttachOffset },
	{ &EV_Player_FlagAttachAngles,							&Player::setFlagAttachAngles },
	
	{ &EV_Player_BackupModel,								&Player::setBackupModel },
	{ NULL, NULL }
};

qboolean Player::checkturnleft( Conditional & )
{
	float yaw;
	
	yaw = SHORT2ANGLE( last_ucmd.angles[ YAW ] + client->ps.delta_angles[ YAW ] );
	
	return ( angledist( old_v_angle[ YAW ] - yaw ) < -8.0f );
}

qboolean Player::checkturnright( Conditional & )
{
	float yaw;
	
	yaw = SHORT2ANGLE( last_ucmd.angles[ YAW ] + client->ps.delta_angles[ YAW ] );
	
	return ( angledist( old_v_angle[ YAW ] - yaw ) > 8.0f );
}

qboolean Player::checkforward( Conditional & )
{
	return last_ucmd.forwardmove > 0;
}

qboolean Player::checkbackward( Conditional & )
{
	return last_ucmd.forwardmove < 0;
}

qboolean Player::checkstrafeleft( Conditional & )
{
	return last_ucmd.rightmove < 0;
}

qboolean Player::checkstraferight( Conditional & )
{
	return last_ucmd.rightmove > 0;
}

qboolean Player::checkleanleft(Conditional &)
{
	if( client->ps.leanDelta > 0 )
	{
		return qtrue;
	}
	
	return qfalse;
}

qboolean Player::checkleanright( Conditional & )
{
	
	if(client->ps.leanDelta < 0)
	{
		return qtrue;
	}
	
	return qfalse;
}

qboolean Player::checkduck( Conditional & )
{
	if ( client->ps.pm_flags & PMF_DUCKED )
		return qtrue;
	else
		return qfalse;
}

qboolean Player::checkrise( Conditional & )
{
	if ( ( do_rise ) && ( velocity.z > 32.0f ) )
	{
		return qtrue;
	}
	do_rise = qfalse;
	
	return qfalse;
}

qboolean Player::checkjump( Conditional & )
{
	if ( sv_instantjump->integer )
		return client->ps.jumped;
	else
		return last_ucmd.upmove > 0;
}

qboolean Player::checkcrouch( Conditional & )
{
	if ( last_ucmd.upmove < 0 ) // check for downward movement
		return qtrue;
	
	return qfalse;
}

qboolean Player::checkjumpflip( Conditional & )
{
	return velocity.z < ( sv_currentGravity->value * 0.5f );
}

qboolean Player::checkanimdone_legs( Conditional & )
{
	if ( ( edict->s.anim & ANIM_BLEND ) == 0 )
		return true;
	
	return animdone_Legs;
}

qboolean Player::checkanimdone_torso( Conditional & )
{
	if ( ( edict->s.torso_anim & ANIM_BLEND ) == 0 )
		return true;

	return animdone_Torso;
}

qboolean Player::checkattackleft( Conditional & )
{
	if ( level.playerfrozen || ( flags & FL_IMMOBILE ) )
	{
		return qfalse;
	}
	
	if ( multiplayerManager.inMultiplayer() && ( !multiplayerManager.isFightingAllowed() || multiplayerManager.isPlayerSpectator( this ) ) )
	{
		return qfalse;
	}
	
	// Make sure we aren't leaning
	
	if ( client->ps.leanDelta != 0 )
		return false;
	
	if ( last_ucmd.buttons & BUTTON_ATTACKLEFT )
	{
		Weapon *weapon;
		
		last_attack_button = BUTTON_ATTACKLEFT;
		
		weapon = GetActiveWeapon( WEAPON_LEFT );
		if ( weapon )
		{
			weapon->CheckReload( FIRE_MODE1 );
			return qtrue;
		}
		
		weapon = GetActiveWeapon( WEAPON_RIGHT );
		if ( weapon )
		{
			weapon->CheckReload( FIRE_MODE1 );
			return qtrue;
		}
		
		weapon = GetActiveWeapon( WEAPON_DUAL );
		if ( weapon )
		{
			weapon->CheckReload( FIRE_MODE1 );
			return qtrue;
		}
		
		// No ammo
		return qfalse;
	}
	else
	{
		return qfalse;
	}
}

qboolean Player::checkattackbuttonleft( Conditional & )
{
	if ( level.playerfrozen || ( flags & FL_IMMOBILE ) )
	{
		return qfalse;
	}
	
	if ( multiplayerManager.inMultiplayer() && ( !multiplayerManager.isFightingAllowed() || multiplayerManager.isPlayerSpectator( this ) ) )
	{
		return qfalse;
	}
	else
	{
		return ( last_ucmd.buttons & BUTTON_ATTACKLEFT );
	}
}

qboolean Player::checkattackright( Conditional & )
{
	Weapon *weapon;
	
	if ( level.playerfrozen || ( flags & FL_IMMOBILE ) )
	{
		return qfalse;
	}
	
	if ( multiplayerManager.inMultiplayer() && ( !multiplayerManager.isFightingAllowed() || multiplayerManager.isPlayerSpectator( this ) ) )
	{
		return qfalse;
	}
	
	// Make sure we aren't leaning
	
	if ( client->ps.leanDelta != 0 )
		return false;
	
	// If we're in the middle of a reload, we're not done attacking
	/*	weapon = GetActiveWeapon( WEAPON_RIGHT );
	if (weapon )
	{
		if ( weapon->weaponstate == WEAPON_RELOADING )
			return true;
	}*/
	
	if ( last_ucmd.buttons & BUTTON_ATTACKRIGHT )
	{
		last_attack_button = BUTTON_ATTACKRIGHT;
		
		weapon = GetActiveWeapon( WEAPON_RIGHT );
		if ( weapon )
		{
			weapon->CheckReload( FIRE_MODE2 );
			return qtrue;
		}
		
		weapon = GetActiveWeapon( WEAPON_LEFT );
		if ( weapon )
		{
			weapon->CheckReload( FIRE_MODE2 );
			return qtrue;
		}
		
		weapon = GetActiveWeapon( WEAPON_DUAL );
		if ( weapon )
		{
			weapon->CheckReload( FIRE_MODE2 );
			return qtrue;
		}
		
		// No ammo
		return qfalse;
	}
	else
	{
		return qfalse;
	}
}

qboolean Player::checkattackbuttonright( Conditional & )
{
	if ( level.playerfrozen || ( flags & FL_IMMOBILE ) )
	{
		return qfalse;
	}
	
	if ( multiplayerManager.inMultiplayer() && ( !multiplayerManager.isFightingAllowed() || multiplayerManager.isPlayerSpectator( this ) ) )
	{
		return qfalse;
	}
	else
	{
		return ( last_ucmd.buttons & BUTTON_ATTACKRIGHT );
	}
}

qboolean Player::checksneak( Conditional & )
{
	return qfalse; //( last_ucmd.buttons & BUTTON_SNEAK ) != 0;
}

qboolean Player::checkrun( Conditional & )
{
	return ( last_ucmd.buttons & BUTTON_RUN ) != 0;
}

qboolean Player::checkwasrunning( Conditional & )
{
	return ( pm_lastruntime > MINIMUM_RUNNING_TIME );
}

qboolean Player::checkholsterweapon( Conditional & )
{
	if(client->ps.pm_flags & PMF_DISABLE_INVENTORY || _disableUseWeapon)
	{
		return qfalse;
	}

	return ( last_ucmd.buttons & BUTTON_HOLSTERWEAPON ) != 0;
}

qboolean Player::checkuse( Conditional & )
{
	return ( last_ucmd.buttons & BUTTON_USE ) != 0;
}

qboolean Player::checkcanturn( Conditional &condition )
{
	float yaw;
	Vector oldang( v_angle );
	qboolean result;
	
	yaw = (float)atof( condition.getParm( 1 ) );
	
	v_angle[ YAW ] = (float)( ( int )( anglemod( v_angle[ YAW ] + yaw ) / 22.5f ) * 22.5f );
	SetViewAngles( v_angle );
	
	result = qtrue; //CheckMove( vec_zero );
	
	SetViewAngles( oldang );
	
	return result;
}

qboolean Player::checkblocked( Conditional &condition )
{
	int test_moveresult;
	
	test_moveresult = moveresult;
	
	if ( flags & FL_IMMOBILE )
		test_moveresult = MOVERESULT_BLOCKED;
	
	if ( condition.numParms() )
	{
		return test_moveresult >= atoi( condition.getParm( 1 ) );
	}
	
	return test_moveresult >= MOVERESULT_BLOCKED;
}

qboolean Player::checkonground( Conditional & )
{
	return client->ps.walking;
}

qboolean Player::check22degreeslope( Conditional & )
{
	if ( client->ps.walking && client->ps.groundPlane && ( client->ps.groundTrace.plane.normal[ 2 ] < SLOPE_22_MAX ) &&
		( client->ps.groundTrace.plane.normal[ 2 ] >= SLOPE_22_MIN ) )
	{
		return qtrue;
	}
	
	return qfalse;
}

qboolean Player::check45degreeslope( Conditional & )
{
	if ( client->ps.walking && client->ps.groundPlane && ( client->ps.groundTrace.plane.normal[ 2 ] < SLOPE_45_MAX ) &&
		( client->ps.groundTrace.plane.normal[ 2 ] >= SLOPE_45_MIN ) )
	{
		return qtrue;
	}
	
	return qfalse;
}

qboolean Player::checkrightleghigh( Conditional & )
{
	float groundyaw;
	float yawdelta;
	int which;
	
	groundyaw = ( int )vectoyaw( client->ps.groundTrace.plane.normal );
	yawdelta = anglemod( v_angle.y - groundyaw );
	which = ( ( int )yawdelta + 45 ) / 90;
	
	return ( which == 3 );
}

qboolean Player::checkleftleghigh( Conditional & )
{
	float groundyaw;
	float yawdelta;
	int which;
	
	groundyaw = ( int )vectoyaw( client->ps.groundTrace.plane.normal );
	yawdelta = anglemod( v_angle.y - groundyaw );
	which = ( ( int )yawdelta + 45 ) / 90;
	
	return ( which == 1 );
}

qboolean Player::checkfacingupslope( Conditional & )
{
	float groundyaw;
	float yawdelta;
	int which;
	
	groundyaw = ( int )vectoyaw( client->ps.groundTrace.plane.normal );
	yawdelta = anglemod( v_angle.y - groundyaw );
	which = ( ( int )yawdelta + 45 ) / 90;
	
	return ( which == 2 );
}

qboolean Player::checkfacingdownslope( Conditional & )
{
	float groundyaw;
	float yawdelta;
	int which;
	
	groundyaw = ( int )vectoyaw( client->ps.groundTrace.plane.normal );
	yawdelta = anglemod( v_angle.y - groundyaw );
	which = ( ( int )yawdelta + 45 ) / 90;
	
	return ( ( which == 0 ) || ( which == 4 ) );
}

qboolean Player::checkfalling( Conditional & )
{
	return falling;
}

qboolean Player::checkgroundentity( Conditional & )
{
	return ( groundentity != NULL );
}

qboolean Player::checkhardimpact( Conditional & )
{
	return hardimpact;
}

qboolean Player::checkcanfall( Conditional & )
{
	return canfall;
}

qboolean Player::checkatdoor( Conditional & )
{
	// Check if the player is at a door
	return ( atobject && atobject->isSubclassOf( Door ) );
}

qboolean Player::checkatuseanim( Conditional & )
{
	// Check if the player is at a useanim
	if ( atobject && atobject->isSubclassOf( UseAnim ) )
	{
		return ( ( UseAnim * )( Entity * )atobject )->canBeUsed( this );
	}
	
	return qfalse;
}

qboolean Player::checktouchuseanim( Conditional & )
{
	if ( toucheduseanim )
	{
		return ( ( UseAnim * )( Entity * )toucheduseanim )->canBeUsed( this );
	}
	
	return qfalse;
}

qboolean Player::checkuseanimfinished( Conditional & )
{
	return ( useanim_numloops <= 0 );
}

qboolean Player::checkatuseobject( Conditional & )
{
	// Check if the player is at a useanim
	if ( atobject && atobject->isSubclassOf( UseObject ) )
	{
		return ( ( UseObject * )( Entity * )atobject )->canBeUsed( origin, yaw_forward );
	}
	
	return qfalse;
}

qboolean Player::checkloopuseobject( Conditional & )
{
	// Check if the player is at a useanim
	if ( useitem_in_use && useitem_in_use->isSubclassOf( UseObject ) )
	{
		return ( ( UseObject * )( Entity * )useitem_in_use )->Loop();
	}
	
	return qfalse;
}

qboolean Player::checkdead( Conditional & )
{
	return ( deadflag );
}

qboolean Player::checkhealth( Conditional &condition )
{
	return health < atoi( condition.getParm( 1 ) );
}

qboolean Player::checkpain( Conditional & )
{
	return ( ( pain != 0 ) || ( knockdown != 0 ) );
}

qboolean Player::checkknockdown( Conditional & )
{
	if ( knockdown )
	{
		knockdown = false;
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

qboolean Player::checkpaintype( Conditional &condition )
{
	if ( pain_type == MOD_NameToNum( condition.getParm( 1 ) ) )
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

qboolean Player::checkpaindirection( Conditional &condition )
{
	if ( pain_dir == Pain_string_to_int( condition.getParm( 1 ) ) )
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

qboolean Player::checkaccumulatedpain( Conditional &condition )
{
	float threshold = (float)atof( condition.getParm( 1 ) );
	
	if ( accumulated_pain >= threshold )
	{
		accumulated_pain = 0; // zero out accumulation
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

qboolean Player::checkpainthreshold( Conditional &condition )
{
	float threshold = (float)atof( condition.getParm( 1 ) );
	
	if ( (  pain >= threshold ) && ( level.time > nextpaintime ) )
	{
		accumulated_pain = 0; // zero out accumulation since we are going into a pain anim right now
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

qboolean Player::checklegsstate( Conditional &condition )
{
	if ( currentState_Legs )
	{
		str current = currentState_Legs->getName();
		str compare = condition.getParm( 1 );
		
		if ( current == compare )
		{
			return qtrue;
		}
	}
	
	return qfalse;
}

qboolean Player::checktorsostate( Conditional &condition )
{
	if ( currentState_Torso )
	{
		str current = currentState_Torso->getName();
		str compare = condition.getParm( 1 );
		
		if ( current == compare )
		{
			return qtrue;
		}
	}
	
	return qfalse;
}

qboolean Player::checkhasweapon( Conditional &condition )
{
	if ( condition.numParms() > 0 )
	{
		weaponhand_t hand = WeaponHandNameToNum( condition.getParm( 1 ) );
		if ( GetActiveWeapon( hand ) )
			return true;
		else
			return false;
	}
	
	return WeaponsOut();
}

qboolean Player::checkhasdualweapon( Conditional & )
{
	return IsDualWeaponActive();
}

qboolean Player::checknewweapon( Conditional & )
{
	Weapon * weapon;
	
	weapon = GetNewActiveWeapon();
	
	if ( weapon && _disableUseWeapon == false)
		return qtrue;
	else
		return qfalse;
}

// Check to see if a weapon has been raised
qboolean Player::checkuseweapon( Conditional &condition )
{
	const char     *weaponName;
	const char     *parm;
	
	weaponhand_t   hand;
	Weapon         *weap;
	
	weap = GetNewActiveWeapon();
	parm = condition.getParm( 1 );
	
	if ( !str::icmp( parm, "ERROR" ) )
	{
		if ( weap )
			warning( "Player::checkuseweapon", "%s does not have a valid RAISE_WEAPON state\n", weap->item_name.c_str() );
		else
			warning( "Player::checkuseweapon", "New Active weapon does not exist\n" );
		
		ClearNewActiveWeapon();
		return qtrue;
	}
	
	hand = WeaponHandNameToNum( parm );
	
	if ( hand == WEAPON_ERROR )
		return qfalse;
	
	weaponName = condition.getParm( 2 );
	
	if (
        ( weap != NULL ) &&
        ( GetNewActiveWeaponHand() == hand ) &&
        ( !stricmp( weap->item_name, weaponName ) )
		)
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

// Checks to see if any weapon is active in the specified hand
qboolean Player::checkanyweaponactive( Conditional &condition )
{
	weaponhand_t   hand;
	Weapon         *weap;
	
	hand = WeaponHandNameToNum( condition.getParm( 1 ) );
	
	if ( hand == WEAPON_ERROR )
		return qfalse;
	
	weap = GetActiveWeapon( hand );
	return ( weap != NULL );
}

// Checks to see if any weapon is active in the specified hand
qboolean Player::checkweaponhasammo( Conditional &condition )
{
	weaponhand_t   hand;
	Weapon         *weap;
	firemode_t     mode = FIRE_MODE1;
	int numShots = 1;
	
	hand = WeaponHandNameToNum( condition.getParm( 1 ) );
	
	if ( condition.numParms() > 1 )
		mode = WeaponModeNameToNum( condition.getParm( 2 ) );
	
	if ( condition.numParms() > 2 )
		numShots = atoi( condition.getParm( 3 ) );
	
	if ( hand == WEAPON_ERROR )
		return qfalse;
	
	weap = GetActiveWeapon( hand );
	
	if ( !weap )
		return qfalse;
	else
		return ( weap->HasAmmo( mode, numShots ) );
}

qboolean Player::checkweaponhasfullammo( Conditional &condition )
{
	str ammoName;

	ammoName = condition.getParm( 1 );

	if ( AmmoCount( ammoName ) == MaxAmmoCount( ammoName ) )
		return true;
	else
		return false;
}

// Checks to see if any weapon is active in the specified hand
qboolean Player::checkweaponhasinvammo( Conditional &condition )
{
	weaponhand_t   hand;
	Weapon         *weap;
	firemode_t     mode = FIRE_MODE1;
	
	hand = WeaponHandNameToNum( condition.getParm( 1 ) );
	
	if ( condition.numParms() > 1 )
		mode = WeaponModeNameToNum( condition.getParm( 2 ) );
	
	if ( hand == WEAPON_ERROR )
		return qfalse;
	
	weap = GetActiveWeapon( hand );
	
	if ( !weap )
		return qfalse;
	else
		return ( weap->HasInvAmmo( mode ) );
}

// Checks to see if weapon is active
qboolean Player::checkweaponactive( Conditional &condition )
{
	const char *weaponName;
	weaponhand_t hand;
	
	weaponName = condition.getParm( 2 );
	hand = WeaponHandNameToNum( condition.getParm( 1 ) );
	
	if ( hand == WEAPON_ERROR )
		return qfalse;
	
	Weapon *weapon = GetActiveWeapon( hand );
	
	return ( weapon && !strcmp( weaponName, weapon->item_name ) );
}

qboolean Player::checkweaponreload( Conditional & )
{
	Weapon *weapon = GetActiveWeapon( WEAPON_DUAL );
	
	if ( weapon )
		if ( weapon->weaponstate == WEAPON_RELOADING )
			return qtrue;
		
		return qfalse;
}

qboolean Player::checkweaponswitchmode( Conditional & )
{
	Weapon *weapon = GetActiveWeapon( WEAPON_DUAL );
	
	if ( weapon )
		if ( weapon->weaponstate == WEAPON_SWITCHINGMODE )
			return qtrue;
		
		return qfalse;
}

qboolean Player::checkweaponinmode( Conditional &condition )
{
	Weapon *weapon = GetActiveWeapon( WEAPON_DUAL );
	str mode;
	
	if ( weapon )
	{
		if ( condition.numParms() > 0 )
			mode = condition.getParm( 1 );
		if ( weapon->GetCurMode() == WeaponModeNameToNum( mode ) )
			return qtrue;
		else
			return qfalse;
	}
	
	return qfalse;
}

qboolean Player::checkweapondonefiring( Conditional & )
{
	Weapon      *weapon = GetActiveWeapon( WEAPON_DUAL );
	
	// Weapon there check
	if ( !weapon )
		return qtrue;
	
	return weapon->IsDoneFiring();
}

// Checks to see if weapon is active and ready to fire
qboolean Player::checkweaponreadytofire( Conditional &condition )
{
	firemode_t     mode = FIRE_MODE1;
	str            weaponName="None";
	weaponhand_t   hand;
	qboolean    ready;
	
	if ( level.playerfrozen || ( flags & FL_IMMOBILE ) )
	{
		return qfalse;
	}
	
	hand = WeaponHandNameToNum( condition.getParm( 1 ) );
	
	if ( hand == WEAPON_DUAL )
	{
		gi.DPrintf( "This check should only be used for single handed weapons\n" );
		return qfalse;
	}
	
	if ( condition.numParms() > 1 )
		weaponName = condition.getParm( 2 );
	
	if ( hand == WEAPON_ERROR )
		return qfalse;
	
	Weapon *weapon = GetActiveWeapon( hand );
	
	// Weapon there check
	if ( !weapon )
		return qfalse;
	
	// Name check
	if ( condition.numParms() > 1 )
	{
		if ( strcmp( weaponName, weapon->item_name ) )
		{
			return qfalse;
		}
	}
	
	// Ammo check
	ready = weapon->ReadyToFire( mode );
	return( ready );
}

qboolean Player::checkdualweaponreadytofire( Conditional &condition )
{
	firemode_t  mode      = FIRE_MODE1;
	str         weaponName="None";
	Weapon      *weapon = GetActiveWeapon( WEAPON_DUAL );
	qboolean    ready;


	mode = WeaponModeNameToNum( condition.getParm( 1 ) );

	if ( condition.numParms() > 1 )
		weaponName = condition.getParm( 2 );

	// Make sure we aren't leaning

	if ( client->ps.leanDelta != 0 )
		return false;

	// Weapon there check
	if ( !weapon )
		return qfalse;

	// Name check
	if ( condition.numParms() > 1 )
	{
		if ( strcmp( weaponName, weapon->item_name ) )
		{
			return qfalse;
		}
	}

	// Ammo check
	ready = weapon->ReadyToFire( mode );

	// Fire timer checks

	if ( !weapon->isModeNoDelay( mode ) )
	{
		if ( weapon->next_fire_time[mode] > level.time )
			return qfalse;

		if ( weapon->next_fire_time[FIRE_MODE1] > level.time )
			return qfalse;

		if ( weapon->next_fire_time[FIRE_MODE2] > level.time )
			return qfalse;
	}

	return( ready );
}

qboolean Player::checkweaponlowered( Conditional & )
{
	Weapon *weapon = GetActiveWeapon( WEAPON_DUAL );
	
	if ( !weapon )
		return true;
	else
		return ( weapon->weaponstate == WEAPON_LOWERING );
}

qboolean Player::checkweaponfiretimer( Conditional &condition )
{
	firemode_t mode = FIRE_MODE1;
	Weapon *weapon = GetActiveWeapon( WEAPON_DUAL );
	
	if ( weapon->GetSwitchMode() )
		mode = weapon->GetCurMode();
	else 
		mode = WeaponModeNameToNum( condition.getParm( 1 ) );
	
	if ( weapon->GetFireTime(mode) > level.time )
		return qfalse;
	
	return qtrue;
}

qboolean Player::checkweaponfullclip( Conditional & )
{
	Weapon *weapon = GetActiveWeapon( WEAPON_DUAL );
	
	if ( !weapon )
		return qtrue;
	
	return weapon->HasFullClip();
}

qboolean Player::checkweaponforcereload( Conditional & )
{
	return ( last_ucmd.buttons & BUTTON_RELOAD ) != 0;
}

qboolean Player::checkweaponcanreload( Conditional & )
{
	Weapon *weapon = GetActiveWeapon( WEAPON_DUAL );
	
	if ( !weapon )
		return qfalse;

	return weapon->canReload();
}

// Check to see if any of the active weapons need to be put away
qboolean Player::checkputawayleft( Conditional & )
{
	Weapon *weapon = GetActiveWeapon( WEAPON_LEFT );
	
	return weapon && weapon->GetPutaway();
}

qboolean Player::checkputawayright( Conditional & )
{
	Weapon *weapon = GetActiveWeapon( WEAPON_RIGHT );
	
	return weapon && weapon->GetPutaway();
}

qboolean Player::checkputawayboth( Conditional & )
{
	Weapon *weapon = GetActiveWeapon( WEAPON_DUAL );
	
	return weapon && weapon->GetPutaway();
}

qboolean Player::checktargetacquired( Conditional & )
{
	return (targetEnemy != NULL);
}

qboolean Player::returntrue( Conditional & )
{
	return qtrue;
}

qboolean Player::checkstatename( Conditional &condition )
{
	str part      = condition.getParm( 1 );
	str statename = condition.getParm( 2 );
	
	if ( currentState_Legs && !part.icmp( "legs" ) )
	{
		return ( !statename.icmpn( currentState_Legs->getName(), statename.length() ) );
	}
	else if ( !part.icmp( "torso" ) )
	{
		return ( !statename.icmpn( currentState_Torso->getName(), statename.length() ) );
	}
	
	return qfalse;
}

qboolean Player::checkattackblocked( Conditional & )
{
	if ( attack_blocked )
	{
		attack_blocked = false;
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

qboolean Player::checkblockdelay( Conditional &condition )
{
	float t = (float)atof ( condition.getParm( 1 ) );
	return ( level.time > ( attack_blocked_time + t ) );
}

qboolean Player::checkpush( Conditional & )
{
	// Check if the player is at a pushobject
	if ( atobject && atobject->isSubclassOf( PushObject ) && ( atobject_dist < ( PUSH_OBJECT_DISTANCE + 15.0f ) ) )
	{
		Vector dir;
		
		dir = atobject_dir * 8.0f;
		return ( ( PushObject * )( Entity * )atobject )->canPush( dir );
	}
	
	return qfalse;
}

qboolean Player::checkpull( Conditional & )
{
	// Check if the player is at a pushobject
	if ( atobject && atobject->isSubclassOf( PushObject ) && ( atobject_dist < ( PUSH_OBJECT_DISTANCE + 15.0f ) ) )
	{
		Vector dir;
		
		dir = atobject_dir * -64.0f;
		return ( ( PushObject * )( Entity * )atobject )->canPush( dir );
	}
	
	return qfalse;
}

qboolean Player::checkcharavailable( Conditional &condition )
{
	str character = condition.getParm( 1 );
	Actor *act = Actor::FindActorByName(character.c_str());
	if ( act )
		return true;
	
	return false;
}

qboolean Player::checkOnLadder( Conditional & )
{
	return _onLadder;
}

qboolean Player::checkcanstand( Conditional & )
{
	Vector newmins( mins );
	Vector newmaxs( maxs );
	Vector tmporg( origin );
	trace_t trace;
	
	// Start the trace alittle higher than the origin, for some
	// reason crouching causes the trace to start allsolid if you're 
	// on a slope
	tmporg[ 2 ] += 2.0f;
	newmins[ 2 ] = MINS_Z;
	newmaxs[ 2 ] = MAXS_Z;
	
	trace = G_Trace( tmporg, newmins, newmaxs, tmporg, this, edict->clipmask, true, "checkcanstand" );
	if ( trace.startsolid )
	{
		return qfalse;
	}
	
	return qtrue;
}

qboolean Player::checkdualwield( Conditional & )
{
	// Only start the dual wield state if dual wield is set and the hands have no weapons
	
	return ( dual_wield_active );
}

qboolean Player::checkdualweapons( Conditional &condition )
{
	int i,j;
	
	for ( i=1; i<=condition.numParms(); i++ )
	{
		str weaponName;
		weaponName = condition.getParm( i );
		
		for ( j=1; j<=dual_wield_weaponlist.NumObjects(); j++ )
		{
			WeaponSetItem *dw;
			dw = dual_wield_weaponlist.ObjectAt( j );
			
			if ( dw->name == weaponName )
            {
				goto out;
            }
		}
		return qfalse;
out:
		;
	}
	
	return qtrue;
}

qboolean Player::checkchance( Conditional &condition )
{
	float percent_chance;
	
	percent_chance = (float)atof( condition.getParm( 1 ) );
	
	return ( G_Random() < percent_chance );
}

qboolean Player::checkturnedleft( Conditional & )
{
	return yawing_left;
}

qboolean Player::checkturnedright( Conditional & )
{
	return yawing_right;
}

//-----------------------------------------------------
//
// Name:		checkinvehicle
// Class:		Player
//
// Description:	Checks if the vehicle is in specific type of vehicle
//
// Parameters:	condition - the condition that contains the vehicle type
//
// Returns:		
//-----------------------------------------------------
qboolean Player::checkinvehicle( Conditional &condition )
{
	
	if(vehicle == NULL || vehicle->getArchetype() != condition.getParm(1))
		return qfalse;
	
	return qtrue;
	
}

qboolean Player::checksolidforward( Conditional &condition )
{
	// Trace out forward to see if there is a solid ahead
	float dist = (float)atof( condition.getParm( 1 ) );
	Vector end( centroid + ( yaw_forward * dist ) );
	
	trace_t trace = G_Trace( centroid, Vector( mins.x, mins.y, -8.0f ), Vector( maxs.x, maxs.y, 8.0f ),
		end, this, MASK_SOLID, true, "Player::checksolidforward" );
	
	return ( trace.fraction < 0.7f );
}

qboolean Player::checkweaponsholstered( Conditional & )
{
	
	if (
		( holsteredWeapons[WEAPON_DUAL] ) ||
		( holsteredWeapons[WEAPON_LEFT] ) ||
		( holsteredWeapons[WEAPON_RIGHT] )
		)
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

qboolean Player::checkfakeplayeractive( Conditional & )
{
	return fakePlayer_active;
}

qboolean Player::checkspecialmovecharge( Conditional & )
{
	if ( specialMoveCharge >= specialMoveEndTime )
		return true;
	return false;
}

qboolean Player::checkstancechangedtorso( Conditional & )
{
	return changedStanceTorso;
}

qboolean Player::checkstancechangedlegs( Conditional & )
{
	return changedStanceLegs;
}

qboolean Player::checkstance( Conditional &condition )
{
	int stancenum;
	stancenum = (int)atoi( condition.getParm( 1 ) );
	
	if ( stancenum == stanceNumber )
		return true;
	
	return false;
}

qboolean Player::checkpoints( Conditional &condition )
{
	int pointnum;
	pointnum = (int)atoi( condition.getParm( 1 ) );
	
	if ( points >= pointnum )
		return true;
	
	return false;
}

qboolean Player::checkincomingmeleeattack( Conditional & )
{
	return incomingMeleeAttack;
}


qboolean Player::checkfinishingmove( Conditional & )
{
	if ( _finishActor )
		return true;
	return false;
}

//--------------------------------------------------------------
//
// Name:			checkatuseentity
// Class:			Player
//
// Description:		Conditional to see if the player is standing at a usable entity
//
// Parameters:		Conditional &condition
//
// Returns:			qboolean
//
//--------------------------------------------------------------
qboolean Player::checkatuseentity(Conditional &)
{
	// Check if the player is at a useentity
	if ( atobject )
	   {
	   Entity *ent = (Entity*)atobject;
	   if ( ent->hasUseData() )
		   return qtrue;
	   }

	return qfalse;
}

//--------------------------------------------------------------
//
// Name:			checkusingentity
// Class:			Player
//
// Description:		Conditional to see if the player is in the middle
//					of using an entity.
//
// Parameters:		Conditional &condition
//
// Returns:			qboolean
//
//--------------------------------------------------------------
qboolean Player::checkusingentity(Conditional &)
{
	return _usingEntity;
}

//--------------------------------------------------------------
//
// Name:			checkthirdperson
// Class:			Player
//
// Description:		Conditional to see if the player is in third person
//
// Parameters:		Conditional &condition
//
// Returns:			qboolean
//
//--------------------------------------------------------------
qboolean Player::checkthirdperson(Conditional &)
{
	return _isThirdPerson;
}


//--------------------------------------------------------------
//
// Name:			checkPropChance
// Class:			Player
//
// Description:		Check the chance based on the property passed.
//					This property should be located in the Actor's
//					StateData object in the gameplay database.
//
// Parameters:		Conditional &conditional
//
// Returns:			qboolean
//
//--------------------------------------------------------------
qboolean Player::checkPropChance( Conditional &condition )
{
	str propname;
	str objname = condition.getParm( 1 );
	if ( condition.numParms() > 1 )
		propname = condition.getParm( 2 );
	
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	str scopestr = getArchetype() + "." + objname;
	if ( !gpm->hasObject(scopestr) )
		return false;
	
	float chance;
	if ( propname.length() )
		chance = gpm->getFloatValue(scopestr, propname);
	else
		chance = gpm->getFloatValue(scopestr, "value");
	
	return ( G_Random() <= chance );
}

//--------------------------------------------------------------
//
// Name:			checkPropExists
// Class:			Player
//
// Description:		Check to see if the property exists
//					The Actor's StateData object will be asked
//					if it has the property.
//
// Parameters:		Conditional &conditional
//
// Returns:			qboolean
//
//--------------------------------------------------------------
qboolean Player::checkPropExists( Conditional &condition )
{
	str objname = condition.getParm( 1 );
	str propname = condition.getParm( 2 );
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	str scopestr = getArchetype() + "." + objname;
	if ( !gpm->hasProperty(scopestr, propname) )
		return false;
	
	return true;
}

//--------------------------------------------------------------
//
// Name:			checkEndAnimChain
// Class:			Player
//
// Description:		Check to see if the current animation chain is on
//					the last anim.
//
// Parameters:		Conditional &conditional
//
// Returns:			qboolean
//
//--------------------------------------------------------------
qboolean Player::checkEndAnimChain( Conditional &condition )
{
	if ( condition.numParms() < 1 )
		return false;
	
	str objname = condition.getParm( 1 );
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	str scopestr = "CurrentPlayer.ChainedMoves";
	if ( !gpm->hasObject(scopestr) )
		return false;
	
	int max = (int)gpm->getFloatValue(scopestr, "value");
	max++; // Increment to compensate for 0 based skill system, we always have to play the first anim
	if ( _gameplayAnimIdx > max )
		return false; // Loop anim
	
	if ( _gameplayAnimIdx == max )
		return true;
	
	return false;
}

qboolean Player::checkWeaponType( Conditional &condition )
{
	if ( condition.numParms() < 1 )
		return false;
	
	str type = condition.getParm( 1 );
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	Weapon* weap = GetActiveWeapon(WEAPON_RIGHT);
	if ( !weap )
		weap = GetActiveWeapon(WEAPON_LEFT);
	if ( !weap )
		return false;
	
	if ( !gpm->hasObject(weap->getArchetype()) )
		return false;
	
	str weaptype = gpm->getStringValue(weap->getArchetype(), "class");
	if ( weaptype == type )
		return true;
	
	return false;
}

qboolean Player::checkHasAnim( Conditional &condition )
{
	str animName;

	if ( condition.numParms() < 1 )
		return false;
	
	animName = condition.getParm( 1 );

	if ( gi.Anim_NumForName( edict->s.modelindex, animName.c_str() ) < 0 )
		return false;
	else
		return true;
}

qboolean Player::checkIsWeaponControllingProjectile( Conditional & )
{
	Weapon *weapon;

	// Only support dual handed weapon for now
		
	weapon = GetActiveWeapon( WEAPON_DUAL );

	if ( weapon )
	{
		if ( weapon->getControllingProjectile() )
			return true;
		else
			return false;
	}

	return false;
}


Condition<Player> Player::Conditions[] =
{
	{ "default",							&Player::returntrue },
	{ "SNEAK",								&Player::checksneak },
	{ "RUN",								&Player::checkrun },
	{ "WAS_RUNNING",						&Player::checkwasrunning },
	{ "HOLSTERWEAPON",						&Player::checkholsterweapon },
	{ "USE",								&Player::checkuse },
	{ "LEFT",								&Player::checkturnleft },
	{ "RIGHT",								&Player::checkturnright },
	{ "FORWARD",							&Player::checkforward },
	{ "BACKWARD",							&Player::checkbackward },
	{ "STRAFE_LEFT",						&Player::checkstrafeleft },
	{ "STRAFE_RIGHT",						&Player::checkstraferight },
	{ "LEAN_LEFT",							&Player::checkleanleft },
	{ "LEAN_RIGHT",							&Player::checkleanright},
	{ "DUCK",								&Player::checkduck },
	{ "JUMP",								&Player::checkjump },
	{ "RISE",								&Player::checkrise },
	{ "CROUCH",								&Player::checkcrouch },
	{ "ANIMDONE_LEGS",						&Player::checkanimdone_legs },
	{ "ANIMDONE_TORSO",						&Player::checkanimdone_torso },
	{ "CAN_TURN",							&Player::checkcanturn },
	{ "BLOCKED",							&Player::checkblocked },
	{ "ONGROUND",							&Player::checkonground },
	{ "CAN_FALL",							&Player::checkcanfall },
	{ "AT_DOOR",							&Player::checkatdoor },
	{ "FALLING",							&Player::checkfalling },
	{ "HARD_IMPACT",						&Player::checkhardimpact },
	{ "KILLED",								&Player::checkdead },
	{ "HEALTH",								&Player::checkhealth },
	{ "PAIN",								&Player::checkpain },
	{ "PAIN_TYPE",							&Player::checkpaintype },
	{ "PAIN_DIRECTION",						&Player::checkpaindirection },
	{ "PAIN_THRESHOLD",						&Player::checkpainthreshold },
	{ "PAIN_ACCUMULATED",					&Player::checkaccumulatedpain },
	{ "KNOCKDOWN",							&Player::checkknockdown },
	{ "LEGS",								&Player::checklegsstate },
	{ "TORSO",								&Player::checktorsostate },
	{ "AT_USEANIM",							&Player::checkatuseanim },
	{ "TOUCHEDUSEANIM",						&Player::checktouchuseanim },
	{ "FINISHEDUSEANIM",					&Player::checkuseanimfinished },
	{ "AT_USEOBJECT",						&Player::checkatuseobject },
	{ "AT_USEENTITY",						&Player::checkatuseentity },
	{ "LOOP_USEOBJECT",						&Player::checkloopuseobject },
	{ "CAN_PUSH",							&Player::checkpush },
	{ "CAN_PULL",							&Player::checkpull },
	{ "ON_LADDER",							&Player::checkOnLadder },
	{ "CAN_STAND",							&Player::checkcanstand },
	{ "CHANCE",								&Player::checkchance },
	{ "TURNED_LEFT",						&Player::checkturnedleft },
	{ "TURNED_RIGHT",						&Player::checkturnedright },
	{ "IN_VEHICLE",							&Player::checkinvehicle },
	//      { "WATER_LEVEL",				checkwaterlevel },
	{ "SOLID_FORWARD",						&Player::checksolidforward },
	{ "GROUNDENTITY",						&Player::checkgroundentity },
	{ "FAKEPLAYERACTIVE",					&Player::checkfakeplayeractive },
	{ "SLOPE_22",							&Player::check22degreeslope },
	{ "SLOPE_45",							&Player::check45degreeslope },
	{ "RIGHT_LEG_HIGH",						&Player::checkrightleghigh },
	{ "LEFT_LEG_HIGH",						&Player::checkleftleghigh },
	{ "FACING_UP_SLOPE",					&Player::checkfacingupslope },
	{ "FACING_DOWN_SLOPE",					&Player::checkfacingdownslope },
	
	// Weapon conditions
	{ "ATTACKLEFT",							&Player::checkattackleft },  // Checks to see if there is an active weapon as well as the button being pressed
	{ "ATTACKRIGHT",						&Player::checkattackright }, // Checks to see if there is an active weapon as well as the button being pressed
	{ "ATTACKLEFTBUTTON",					&Player::checkattackbuttonleft }, // Checks to see if the left attack button is pressed
	{ "ATTACKRIGHTBUTTON",					&Player::checkattackbuttonright },// Checks to see if the right attack button is pressed
	{ "HAS_DUAL_WEAPON",					&Player::checkhasdualweapon},
	{ "HAS_WEAPON",							&Player::checkhasweapon },
	{ "NEW_WEAPON",							&Player::checknewweapon },
	{ "IS_NEW_WEAPON",						&Player::checkuseweapon },
	{ "IS_WEAPON_ACTIVE",					&Player::checkweaponactive },
	{ "IS_WEAPON_RELOADING",				&Player::checkweaponreload },
	{ "IS_IN_MODE",							&Player::checkweaponinmode },
	{ "IS_WEAPON_SWITCHINGMODE",			&Player::checkweaponswitchmode },
	{ "IS_WEAPON_READY_TO_FIRE",			&Player::checkweaponreadytofire },
	{ "IS_DUALWEAPON_READY_TO_FIRE",		&Player::checkdualweaponreadytofire },
	{ "IS_WEAPON_FINISHED_FIRING",			&Player::checkweapondonefiring		},
	{ "PUTAWAYBOTH",						&Player::checkputawayboth },
	{ "PUTAWAYLEFT",						&Player::checkputawayleft },
	{ "PUTAWAYRIGHT",						&Player::checkputawayright },
	{ "TARGET_ACQUIRED",					&Player::checktargetacquired },
	{ "ANY_WEAPON_ACTIVE",					&Player::checkanyweaponactive },
	{ "ATTACK_BLOCKED",						&Player::checkattackblocked },
	{ "STATE_ACTIVE",						&Player::checkstatename },
	{ "BLOCK_DELAY",						&Player::checkblockdelay },
	{ "DUALWIELD",							&Player::checkdualwield },
	{ "DUALWIELDWEAPONS",					&Player::checkdualweapons },
	{ "HAS_AMMO",							&Player::checkweaponhasammo },
	{ "HAS_FULLAMMO",						&Player::checkweaponhasfullammo },
	{ "HAS_INVAMMO",						&Player::checkweaponhasinvammo },
	{ "WEAPONS_HOLSTERED",					&Player::checkweaponsholstered },
	{ "WEAPON_LOWERED",						&Player::checkweaponlowered }, // See if we're done with our putaway anim
	{ "WEAPON_FIRETIMER",					&Player::checkweaponfiretimer },
	{ "WEAPON_FORCERELOAD",					&Player::checkweaponforcereload },
	{ "WEAPON_CAN_RELOAD",					&Player::checkweaponcanreload },
	{ "WEAPON_FULLCLIP",					&Player::checkweaponfullclip },
	{ "IS_SPECIALMOVE_CHARGED",				&Player::checkspecialmovecharge },
	{ "IS_CHAR_AVAILABLE",					&Player::checkcharavailable },
	
	{ "STANCE_CHANGEDTORSO",				&Player::checkstancechangedtorso },
	{ "STANCE_CHANGEDLEGS",					&Player::checkstancechangedlegs },
	{ "STANCE",								&Player::checkstance },
	{ "POINTS",								&Player::checkpoints },
	{ "INCOMING_MELEE",						&Player::checkincomingmeleeattack },
	{ "FINISHINGMOVE",						&Player::checkfinishingmove },
	{ "USINGENTITY",						&Player::checkusingentity },
	{ "IS_THIRDPERSON",						&Player::checkthirdperson },
	{ "PROP_CHANCE",						&Player::checkPropChance },
	{ "PROP_EXISTS",						&Player::checkPropExists	},
	{ "ANIMCHAIN_END",						&Player::checkEndAnimChain },
	{ "WEAPON_TYPE",						&Player::checkWeaponType },
	{ "HAS_ANIM",							&Player::checkHasAnim },
	{ "IS_WEAPON_CONTROLLING_PROJECTILE",	&Player::checkIsWeaponControllingProjectile },
	
	{ NULL,									NULL },
};

movecontrolfunc_t Player::MoveStartFuncs[] =
{
	NULL,					// MOVECONTROL_USER,       // Quake style
	NULL,					// MOVECONTROL_LEGS,       // Quake style, legs state system active
	NULL,					// MOVECONTROL_ANIM,       // move based on animation, with full collision testing
	NULL,					// MOVECONTROL_ABSOLUTE,   // move based on animation, with full collision testing but no turning
	NULL,					// MOVECONTROL_HANGING
	NULL,					// MOVECONTROL_WALLHUG
	NULL,					// MOVECONTROL_MONKEYBARS
	NULL,					// MOVECONTROL_PIPECRAWL
	NULL,					// MOVECONTROL_PIPEHANG
	NULL,					// MOVECONTROL_STEPUP
	NULL,					// MOVECONTROL_ROPE_GRAB
	NULL,					// MOVECONTROL_ROPE_RELEASE
	NULL,					// MOVECONTROL_ROPE_MOVE
	NULL,					// MOVECONTORL_PICKUPENEMY
	&Player::StartPush,			// MOVECONTROL_PUSH
	NULL,					// MOVECONTORL_CLIMBWALL
	&Player::StartUseAnim,		// MOVECONTROL_USEANIM
	NULL,					// MOVECONTROL_CROUCH
	&Player::StartLoopUseAnim,	// MOVECONTROL_LOOPUSEANIM
	&Player::SetupUseObject,	// MOVECONTROL_USEOBJECT
	&Player::StartCoolItemAnim,	// MOVECONTROL_COOLOBJECT
	&Player::StartFakePlayer	// MOVECONTROL_FAKEPLAYER
};

Player::Player()
{
	//
	// set the entity type
	//
	edict->s.eType       = ET_PLAYER;
	respawn_time         = -1;
	statemap_Legs        = NULL;
	statemap_Torso       = NULL;
	camera               = NULL;
	atobject             = NULL;
	atobject_dist        = 0;
	toucheduseanim       = NULL;
	useitem_in_use       = NULL;
	damage_blood         = 0;
	damage_count         = 0;
	damage_from          = vec_zero;
	damage_alpha         = 0;
	last_attack_button   = 0;
	attack_blocked       = false;
	shield_active        = false;
	canfall              = false;
	moveresult           = MOVERESULT_NONE;
	animspeed            = 0;
	airspeed             = 350;
	vehicle              = NULL;
	action_level         = 0;
	adjust_torso         = false;
	dual_wield_active    = false;
	cool_item            = NULL;
	weapons_holstered_by_code = false;
	actor_camera         = NULL;
	cool_camera          = NULL;

	_started = false;
	
	StopWatchingEntity();
	playerCameraMode = PLAYER_CAMERA_MODE_NORMAL;
	
	damage_multiplier    = 1;
	take_pain            = true;
	look_at_time			= 0;
	fakePlayer_active    = false;
	projdetonate			= false;
	
	_flashMaxTime = 0.0f;
	_flashMinTime = 0.0f;
	
	dont_turn_legs			= false;
	
	specialMoveChargeTime	= 3.0f;
	specialMoveCharge		= 0.0f;
	specialMoveEndTime		= 0.0f;
	playerKnockback			= 0.0f;
	knockbackMultiplier		= 1.0f;
	changedStanceTorso		= false;
	changedStanceLegs		= false;
	incomingMeleeAttack		= false;
	stanceNumber			= 1;
	points					= 0;
	bendTorsoMult			= standardTorsoMult;
	meleeAttackFlags		= 0;
	changingChar			= false;
	_isThirdPerson			= true; // This will get set later
	_finishActor			= 0;
	_finishState			= "";
	_doingFinishingMove		= false;
	_usingEntity			= false;
	_attackType				= "";
	_gameplayAnimIdx		= 1;
	_disableUseWeapon		= false;
	_infoHudOn				= false;
	_nextRegenTime			= 0;
	_useEntityStartTimer	= 0.0f;
	
#ifdef DEDICATED
	p_heuristics = 0;
#else
	p_heuristics = new PlayerHeuristics;
#endif
	
	currentCallVolume = "";
	
	//Add the player to the teammates list 
	//Probably should be a better way to do this.
	TeamMateList.AddObject( this );
	
	// make sure that we are not overflowing the stats for players
	assert( STAT_LAST_STAT <= MAX_STATS );
	
	/* fov = (float)atof( Info_ValueForKey( client->pers.userinfo, "fov" ) );
	if ( fov < 1.0f )
	{
		fov = sv_defaultFov->value;
	}
	else if ( fov > 160.0f )
	{
		fov = 160.0f;
	} */

	fov = sv_defaultFov->value;
	//_userFovChoice = fov;
	
	if ( atoi( Info_ValueForKey( client->pers.userinfo, "mp_autoSwitchWeapons" ) ) )
		_autoSwitchWeapons = true;
	else
		_autoSwitchWeapons = false;
	
	//
	// set targetnameplayer
	//
	if ( !LoadingSavegame )
		SetTargetName( "player" );
	
	_powerup = NULL;
	_rune = NULL;
	_holdableItem = NULL;
	
	if ( !LoadingSavegame )
	{
		GameplayManager *gpm = GameplayManager::getTheGameplayManager();
		
		const str selectedHighlight( "target_selected_highlight" );
		if ( gpm->isDefined( selectedHighlight ) )
		{
			const str targetSelectedHighlightModelName( gpm->getDefine( selectedHighlight ) );
			_targetSelectedHighlight = new Entity;
			_targetSelectedHighlight->setModel( targetSelectedHighlightModelName );
			_targetSelectedHighlight->setSolidType( SOLID_NOT );
			_targetSelectedHighlight->setMoveType( MOVETYPE_NONE );
			_targetSelectedHighlight->takedamage = DAMAGE_NO;
			_targetSelectedHighlight->hideModel();
		}
		
		const str lockedHighlight( "target_locked_highlight" );
		if ( gpm->isDefined( lockedHighlight ) )
		{
			const str targetLockedHighlightModelName( gpm->getDefine( lockedHighlight ) );
			_targetLockedHighlight = new Entity;
			_targetLockedHighlight->setModel( targetLockedHighlightModelName );
			_targetLockedHighlight->setSolidType( SOLID_NOT );
			_targetLockedHighlight->setMoveType( MOVETYPE_NONE );
			_targetLockedHighlight->takedamage = DAMAGE_NO;
			_targetLockedHighlight->hideModel();
		}
	}
	
	Init();
	
	_viewMode = 0;
	
	addAffectingViewModes( gi.GetViewModeClassMask( "player" ) );
	
	_canTransferEnergy = false;
	
	_doDamageScreenFlash = false;
	
	Vector maxs(MAXS_X,MAXS_Y,MAXS_Z),mins(MINS_X,MINS_Y,MINS_Z);

	setSize( mins, maxs );
	
	// Setup ladder stuff
	
	_onLadder = false;
	_nextLadderTime = 0.0f;
	_ladderTop = 0.0f;
	
	_objectiveStates	= 0;
	_informationStates	= 0;
	_objectiveNameIndex = 0;

	_lastDamagedTimeFront = 0.0f;
	_lastDamagedTimeBack  = 0.0f;
	_lastDamagedTimeLeft  = 0.0f;
	_lastDamagedTimeRight = 0.0f;

	_totalGameFrames = 0;
	_updateGameFrames = true;

	_nextPainShaderTime = 0.0f;
	_lastPainShaderMod = MOD_NONE;

	_validPlayerModel = false;

	_secretsFound = 0;

	_allowActionMusic = true;

	setSkill( skill->integer );

	_needToSendBranchDialog = false;
	_branchDialogActor = NULL;
}

Player::~Player()
{	
	if ( p_heuristics )
	{
		p_heuristics->SaveHeuristics( this );	
		delete p_heuristics;
	}
	
	Sentient *player;
	player = this;
	
	if ( TeamMateList.ObjectInList( player ) )
		TeamMateList.RemoveObject( player );
	
	if ( edict->svflags & SVF_BOT )
	{
		BotAIShutdownClient( entnum, qfalse );
	}
	
	edict->s.modelindex = 0;
	
	freeConditionals( legs_conditionals );
	freeConditionals( torso_conditionals );
	
	removePowerup();
	removeRune();
	removeHoldableItem();
	
	clearFinishingMove(NULL);
	
	if ( multiplayerManager.inMultiplayer() )
	{
		multiplayerManager.removePlayer( this );
	}
}

static qboolean logfile_started = false;

void Player::Init( void )
{

	InitClient();
	InitPhysics();
	InitWorldEffects();
	InitSound();
	InitView();
	InitState();
	InitEdict();
	InitModel();
	InitWeapons();
	InitInventory();
	InitHealth();
	//InitArmorValue();
	InitStats();
	
	_dialogEntnum = ENTITYNUM_NONE;
	_dialogSoundIndex = -1;
	_dialogTextSoundIndex = -1;

	client->ps.objectiveNameIndex = -1;
	
	edict->s.clientNum = client->ps.clientNum ;
	
	LoadStateTable();
	
	logfile_started = false;
	
	removePowerup();
	removeRune();
	removeHoldableItem();

	_nextEnergyTransferTime = 0.0f;
	_nextPainShaderTime = 0.0f;
	
	edict->s.renderfx &= ~RF_FORCE_ALPHA;
	
	setAlpha( 1.0f );
	
	if ( multiplayerManager.inMultiplayer() )
	{
		multiplayerManager.addPlayer( this );
	}
	else
	{
		if ( !LoadingSavegame )
		{
			ChooseSpawnPoint();
		}
	}
	
	clearAllHuds();


	_backpackAttachOffset = Vector( -12, -9, 0 );
	_backpackAttachAngles = Vector( 0, 90, 90 );

	_flagAttachOffset = Vector( -96, -16, 0 );
	_flagAttachAngles = Vector( 0, 90, 90 );

	_cameraCutThisFrame = false;

	//Clear out deaththread
	

	// Make sure we put the player back into the world
	
	link();
}

void Player::InitEdict( void )
{
	// entity state stuff
	setSolidType( SOLID_BBOX );
	setMoveType( MOVETYPE_WALK );
	setSize( Vector( MINS_X, MINS_Y, 0.0f ), Vector( MAXS_X, MAXS_Y, MAXS_Z ) );
	edict->clipmask	 = MASK_PLAYERSOLID;
	edict->svflags		&= ~SVF_DEADMONSTER;
	edict->svflags    &= ~SVF_HIDEOWNER;
	edict->ownerNum    = ENTITYNUM_NONE;
	
	// clear entity state values
	edict->s.eFlags		= 0;
	edict->s.frame	      = 0;
	
	// players have precise shadows
	edict->s.renderfx |= RF_SHADOW_PRECISE;
}

void Player::InitSound( void )
{
	//
	// reset the music
	//
	client->ps.current_music_mood  = mood_normal;
	client->ps.fallback_music_mood = mood_normal;
	ChangeMusic( "normal", "normal", false );
	
	client->ps.music_volume           = 1.0f;
	client->ps.music_volume_fade_time = 0.0f;
	
	_allowMusicDucking					= true;
	client->ps.allowMusicDucking		= true;
	
	ChangeMusicVolume( 1.0f, 0.0f );
	
	music_forced = false;
	
	// Reset the reverb stuff
	
	client->ps.reverb_type  = eax_generic;
	client->ps.reverb_level = 0;
	SetReverb( client->ps.reverb_type, client->ps.reverb_level );
}

void Player::InitClient( void )
{
	// deathmatch wipes most client data every spawn
	if ( multiplayerManager.inMultiplayer() )
	{
		float savedTime;
		char userinfo[ MAX_INFO_STRING ];
		//char savedTeamName[ 16 ];
		
		savedTime = client->pers.enterTime;
		//strcpy( savedTeamName, client->pers.lastTeam );
		
		memcpy( userinfo, client->pers.userinfo, sizeof( userinfo ) );
		G_InitClientPersistant( client );
		G_ClientUserinfoChanged( edict, userinfo );
		
		//strcpy( client->pers.lastTeam, savedTeamName );
		client->pers.enterTime = savedTime;
	}
	
	// save things that should be saved before memset-ing the client
	// during loadgame, ps.stats is loaded before this point, so don't trample that data
	int savedstats[ sizeof( client->ps.stats ) ]; // only a few hundred bytes
	assert( sizeof( savedstats ) < 2048 ); // should be allocated if it gets too big

	memcpy( savedstats, client->ps.stats, sizeof( client->ps.stats ) );
	client_persistant_t	savedpers = client->pers;

	memset( client, 0, sizeof( *client ) );

	client->pers = savedpers;
	memcpy( client->ps.stats, savedstats, sizeof( client->ps.stats ) );
	
	client->ps.clientNum = client - game.clients;
	client->ps.vehicleoffset[0] = 0;
	client->ps.vehicleoffset[1] = 0;
	client->ps.vehicleoffset[2] = 0;
	client->ps.in_vehicle = false;

	client->ps.viewheight = (int) sv_defaultviewheight->value;
	client->ps.pm_defaultviewheight = (int) sv_defaultviewheight->value;
}

void Player::InitState( void )
{
	gibbed             = false;
	pain               = 0;
	accumulated_pain   = 0;
	nextpaintime       = 0.0f;
	knockdown          = false;
	pain_dir           = PAIN_NONE;
	pain_type          = MOD_NONE;
	takedamage			 = DAMAGE_AIM;
	deadflag				 = DEAD_NO;
	flags					&= ~FL_NO_KNOCKBACK;
	flags					|= ( FL_BLOOD | FL_DIE_GIBS );
	
	if ( !com_blood->integer )
	{
		flags &= ~FL_BLOOD;
		flags &= ~FL_DIE_GIBS;
	}
}

/*
void Player::InitArmorValue
   (
	void
	)

	{
   //ArmorValue = 0;
	}
*/

void Player::InitHealth( void )
{
	// Don't do anything if we're loading a server game.
	// This is either a loadgame or a restart
	if ( LoadingSavegame )
	{
		return;
	}
	
	// reset the health values
	health	  = 100;
	max_health = 100;
}

void Player::InitModel( const char *modelName )
{
	orientation_t orient;
	int anim;
	int tagnum;
	int i;
	
	
	// Reset all surface to visible
	
	for ( i = 0 ; i < MAX_MODEL_SURFACES ; i++ )
	{
		edict->s.surfaces[ i ] &= ~MDL_SURFACE_NODRAW;
	}
	
	if ( modelName )
		setModel( modelName );
	else
		setModel( str( g_playermodel->string ) + ".tik" );
	
	SetControllerTag( HEAD_TAG,   gi.Tag_NumForName( edict->s.modelindex, "Bip01 Head" ) );
	SetControllerTag( TORSO_TAG,  gi.Tag_NumForName( edict->s.modelindex, "Bip01 Spine1" ) );
	SetControllerTag( R_ARM_TAG,  gi.Tag_NumForName( edict->s.modelindex, R_ARM_NAME ) );
	SetControllerTag( L_ARM_TAG,  gi.Tag_NumForName( edict->s.modelindex, L_ARM_NAME ) );
	SetControllerTag( MOUTH_TAG,   gi.Tag_NumForName( edict->s.modelindex, "tag_mouth" ) );
	
	anim = gi.Anim_NumForName( edict->s.modelindex, "stand_idle" );
	
	tagnum = gi.Tag_NumForName( edict->s.modelindex, "Bip01 R Foot" ) & TAG_MASK;
	orient = gi.Tag_Orientation( edict->s.modelindex, anim, 0, tagnum, 1.0f, NULL, NULL );
	base_rightfoot_pos = orient.origin;
	base_rightfoot_pos.z = 0;
	
	tagnum = gi.Tag_NumForName( edict->s.modelindex, "Bip01 L Foot" ) & TAG_MASK;
	orient = gi.Tag_Orientation( edict->s.modelindex, anim, 0, tagnum, 1.0f, NULL, NULL );
	base_leftfoot_pos = orient.origin;
	base_leftfoot_pos.z = 0;
	
	showModel();
	
	yawing_left = false;
	yawing_right = false;
}

void Player::InitPhysics( void )
{
	// Physics stuff
	oldvelocity = vec_zero;
	velocity    = vec_zero;
	old_v_angle = v_angle;
	gravity     = 1.0;
	falling     = false;
	hardimpact  = false;
	mass			= 200;
	setContents( CONTENTS_BODY );
	memset( &last_ucmd, 0, sizeof( last_ucmd ) );

	_forcedMoveType = PM_NONE;
}

void Player::InitWorldEffects( void )
{
	// world effects
	next_drown_time    = 0;
	next_painsound_time    = 0;
	air_finished       = level.time + 20.0f;
	old_waterlevel     = 0;
	drown_damage       = 0.0f;
}

void Player::InitWeapons( void )
{
	// Don't do anything if we're loading a server game.
	// This is either a loadgame or a restart
	if ( LoadingSavegame )
	{
		return;
	}

	ClearNewActiveWeapon();
}

void Player::InitInventory( void )
{
}

void Player::InitView( void )
{
	// view stuff
	camera         = NULL;
	v_angle			= vec_zero;
	SetViewAngles( v_angle );
	viewheight	   = STAND_EYE_HEIGHT;
	
	head_target       = NULL;
	targetEnemy  = NULL;
	targetEnemyLocked = false;
	_targetedEntity		= NULL;
	
	// blend stuff
	damage_blend	= vec_zero;
}

void Player::InitStats( void )
{
	if ( p_heuristics )
	{
		p_heuristics->LoadHeuristics();
		p_heuristics->ClearLevelStatistics();
	}
}

void Player::ChooseSpawnPoint( void )
{
	str thread;
	// set up the player's spawn location
	SelectSpawnPoint( origin, angles, thread );
	setOrigin( origin	+ Vector( "0 0 1" ) );
	origin.copyTo( edict->s.origin2 );
	edict->s.renderfx |= RF_FRAMELERP;
	
	KillBox( this );
	
	setAngles( angles );
	SetViewAngles( angles );
	
	if ( thread.length() )
	{
		ExecuteThread( thread );
	}
}

void Player::EndLevel( Event * )
{
	// this happens here to avoid the last frame (displayed while loading) from being fullbright or whatever
	setViewMode( "none" );

	if ( health > max_health )
	{
		health = max_health;
	}
	
	if ( health < 1.0f )
	{
		health = 1.0f;
	}

	_updateGameFrames = false;
}

void Player::LevelCleanup( void )
{
	// Do any re-initialization things that affect visuals that aren't supposed to be visible to player
	// reset stats that aren't supposed to carry across missions, i.e. only between sublevels
	// should only be called in single player
	
	if( !gi.areSublevels( level.mapname, level.nextmap ) && client )
	{
		client->ps.stats[ STAT_ENEMIES_KILLED ] = 0;
		client->ps.stats[ STAT_TEAMMATES_KILLED ] = 0;
		client->ps.stats[ STAT_SHOTS_FIRED ] = 0;
		client->ps.stats[ STAT_SHOTS_HIT ] = 0;
		client->ps.stats[ STAT_MISSION_DURATION ] =0;
	}

	client->ps.pm_flags &= ~PMF_NIGHTVISION;
}

void Player::Respawn( Event * )
{
	if ( multiplayerManager.inMultiplayer() )
	{
		assert ( deadflag == DEAD_DEAD );
		
		respawn_time = level.time;
		
		Init();
		
		// hold in place briefly
		client->ps.pm_time = 50;
		client->ps.pm_flags |= PMF_TIME_TELEPORT;
		
		return;
	}
	else
	{
#ifdef PRE_RELEASE_DEMO
		gi.SendConsoleCommand( "forcemenu demomain; forcemenu loadsave\n" );
#else
//		gi.SendConsoleCommand( "forcemenu main; forcemenu loadsave\n" );
#endif
		logfile_started = false;
	}
}

void Player::SetDeltaAngles( void )
{
	int i;
	
	// Use v_angle since we may be in a camera
	for( i = 0; i < 3; i++ )
	{
		client->ps.delta_angles[ i ] = ANGLE2SHORT( v_angle[ i ] );
	}
}

void Player::Dead( Event * )
{
	
	CancelEventsOfType( EV_Player_Dead );
	
	animate->StopAnimatingAtEnd();
	
	if ( edict->s.torso_anim & ANIM_BLEND )
		animate->StopAnimatingAtEnd(torso);
	
	//if ( ( pain_type == MOD_VAPORIZE || pain_type == MOD_VAPORIZE_COMP || pain_type == MOD_VAPORIZE_DISRUPTOR ) && ( edict->s.eFlags | EF_EFFECT_PHASER ) && ( edict->s.alpha > 0.0f ) )
	if ( ( ( pain_type == MOD_VAPORIZE ) || ( pain_type == MOD_VAPORIZE_COMP ) || 
		( pain_type == MOD_VAPORIZE_DISRUPTOR ) || ( pain_type == MOD_VAPORIZE_PHOTON ) || ( pain_type == MOD_SNIPER ) ) && 
		( edict->s.alpha > 0.0f ) && ( level.time < respawn_time ) )
	{
		PostEvent( EV_Player_Dead, FRAMETIME );
		return;
	}
	
	deadflag = DEAD_DEAD;
	
	// stop animating
	//animate->StopAnimating( legs );
	
	// increase player's death count
	if ( p_heuristics )
		p_heuristics->IncrementNumberOfDeaths();	
	++client->ps.stats[ STAT_DEATHS ];
	
	// Heuristics Stay Persistant Regardless of deaths.
	if ( p_heuristics )
		p_heuristics->SaveHeuristics( this );
	
	if ( multiplayerManager.inMultiplayer() )
	{
		multiplayerManager.playerDead( this );
	}
	else
	{
		//pick random player killed string.
		int i = (rand() % 10) + 1;
		str playerKilled = "PlayerKilled";
		playerKilled += i;
		G_MissionFailed(playerKilled);
	}
	
	_updateGameFrames = false;
}

void Player::Killed( Event *ev )
{
	Entity   *attacker;
	Entity   *inflictor;
	int      meansofdeath;
	Vector direction;
	
	attacker     = ev->GetEntity( 1 );
	inflictor    = ev->GetEntity( 3 );
	meansofdeath = ev->GetInteger( 4 );
	direction    = ev->GetVector( 7 );
	
	pain_type = (meansOfDeath_t)meansofdeath;

	damage_from = direction;
	
	if ( multiplayerManager.inMultiplayer() )
	{
		if ( attacker && attacker->isSubclassOf( Player ) )
			multiplayerManager.playerKilled( this, (Player *)attacker, inflictor, meansofdeath );
		else
			multiplayerManager.playerKilled( this, NULL, inflictor, meansofdeath );
	}

	SpawnDamageEffect( (meansOfDeath_t)meansofdeath );

	animate->ClearTorsoAnim();
	animate->ClearLegsAnim();
	
	deadflag	= DEAD_DYING;
	
	respawn_time = level.time + 1.0f;
	
	edict->clipmask = MASK_DEADSOLID;
	edict->svflags |= SVF_DEADMONSTER;
	
	setContents( CONTENTS_CORPSE );
	
	setMoveType( MOVETYPE_NONE );
	
	angles.x = 0;
	angles.z = 0;
	setAngles( angles );
	
	//
	// change music
	//
	ChangeMusic( "failure", "normal", true );
	
	health = 0;
	
	// Stop targeting monsters
	if( _targetedEntity != NULL)
	{
		_targetedEntity->edict->s.eFlags &= ~EF_DISPLAY_INFO;
		_targetedEntity = NULL;
	}
	
	// Post a dead event just in case
	PostEvent( EV_Player_Dead, 5.0f );
	
	dropRune();
	dropPowerup();
	removeHoldableItem();
	
	// In Deathmatch drop your weapon
	
	if ( multiplayerManager.inMultiplayer()	&& !multiplayerManager.checkFlag( MP_FLAG_NO_DROP_WEAPONS ) &&
		 multiplayerManager.checkRule( "dropWeapons", true ) )
	{
		int i;
		for ( i=0; i<MAX_ACTIVE_WEAPONS; i++ )
		{
			Weapon *weap = activeWeaponList[i];
			
			if ( weap )
            {
				weap->Drop();
				activeWeaponList[i] = NULL;
            }
		}
		
	}

	ClearNewActiveWeapon();
	
	// Now take away the rest of the weapons
	FreeInventory();
	

	_updateGameFrames = false;
}

void Player::Pain( Event *ev )
{
	float		damage,yawdiff;
	Entity	*attacker;
	int      meansofdeath;
	Vector   dir,pos,attack_angle;
	
	damage	      = ev->GetFloat( 1 );
	attacker       = ev->GetEntity( 2 );
	meansofdeath   = ev->GetInteger( 3 );
	pos            = ev->GetVector( 4 );
	dir            = ev->GetVector( 5 );
	
	if ( !damage && !knockdown )
		return;
	
	client->ps.stats[ STAT_LAST_PAIN ] = (int) damage;
	
	// Determine direction
	attack_angle = dir.toAngles();
	yawdiff = torsoAngles[YAW] - attack_angle[YAW] + 180.0f;
	yawdiff = AngleNormalize180( yawdiff );
	
	if ( ( yawdiff > -45.0f ) && ( yawdiff < 45.0f ) )
		pain_dir = PAIN_FRONT;
	else if ( ( yawdiff < -45.0f ) && ( yawdiff > -135.0f ) )
		pain_dir = PAIN_LEFT;
	else if ( ( yawdiff > 45.0f ) && ( yawdiff < 135.0f ) )
		pain_dir = PAIN_RIGHT;
	else
		pain_dir = PAIN_REAR;
	
	// accumulate pain for animation purposes
	if ( take_pain )
	{
		accumulated_pain   += damage;
	}
	
	// Spawn off any damage effect if we get hit with a certain type of damage
	SpawnDamageEffect( (meansOfDeath_t)meansofdeath );
	
	pain_type          = (meansOfDeath_t)meansofdeath;
	
	// Only set the regular pain level if enough time since last pain has passed
	if ( ( level.time > nextpaintime ) && take_pain )
	{
		pain = damage;
	}

	if ( ( level.time > next_painsound_time ) && ( health > 0.0f ) )
	{
		if ( G_GetDatabaseFloat( "MOD", MOD_NumToName( meansofdeath ), "DoPainSound" ) )
		{
			next_painsound_time = level.time + 0.25f + G_Random( 0.25f );
			Sound( "snd_pain", CHAN_VOICE, DEFAULT_VOL, 300.0f );
		}
	}
	
	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame

	damage_blood += damage;

	if ( deadflag == DEAD_NO )
	{
		if ( meansofdeath == MOD_DEATH_QUAD )
			damage_from = vec_zero;
		else
			damage_from = ev->GetVector( 5 ) * damage;
	}


	if ( meansofdeath == MOD_FIRE || meansofdeath == MOD_ON_FIRE )
		damage_blend = damageFireColor;
	else
		damage_blend = damageNormalColor;

	// Determine which side was hit

	if ( damage_from == vec_zero )
	{
		_lastDamagedTimeFront = level.time;
		_lastDamagedTimeLeft  = level.time;
		_lastDamagedTimeRight = level.time;
		_lastDamagedTimeBack  = level.time;
	}
	else
	{
		Vector damageAngles;
		float yaw;

		damageAngles = damage_from * -1.0f;
		damageAngles = damageAngles.toAngles();

		yaw = AngleNormalize180( AngleNormalize180( damageAngles[ YAW ] ) - AngleNormalize180( v_angle[ YAW ] ) );

		if ( ( yaw >= -45.0f ) && ( yaw <= 45.0f ) )
			_lastDamagedTimeFront = level.time;
		else if ( ( yaw > 45.0f ) && ( yaw < 135.0f ) )
			_lastDamagedTimeLeft = level.time;
		else if ( ( yaw < -45.0f ) && ( yaw > -135.0f ) )
			_lastDamagedTimeRight = level.time;
		else
			_lastDamagedTimeBack = level.time;
	}
}

void Player::DoUse( Event *ev )
{
	int		   i;
	int		   num;
	int         touch[ MAX_GENTITIES ];
	gentity_t   *hit;
	Event		   *event;
	Vector	   min;
	Vector	   max;
	Vector	   offset;
	trace_t	   trace;
	Vector	   start;
	Vector	   end;
	float		   t;
	Entity		*usingEntity;
	bool entityActuallyUsed;
	Entity *bestActor;
	

	if ( multiplayerManager.inMultiplayer() && multiplayerManager.isPlayerSpectator( this ) )
		return;

	if ( ev->NumArgs() > 0 )
		usingEntity = ev->GetEntity( 1 );
	else
		usingEntity = this;
	
	// if we are in a vehicle, we want to use the vehicle always
	if ( vehicle )
	{
		event = new Event( EV_Use );
		event->AddEntity( this );		
		vehicle->ProcessEvent( event );
		return;
	}
	
	start = origin;
	start.z += client->ps.viewheight;
	end = start + ( yaw_forward * 64.0f );
	
	trace = G_Trace( start, vec_zero, vec_zero, end, this, MASK_USABLE, true, "Player::DoUse" );
	
	t = 64.0f * trace.fraction - maxs[ 0 ];
	if ( t < 0.0f )
	{
		t = 0.0f;
	}
	
	offset = yaw_forward * t;
	
	min = start + offset + Vector( "-31 -31 -31" );
	max = start + offset + Vector( "31 31 31" );
	
	num = gi.AreaEntities( min, max, touch, MAX_GENTITIES, qfalse );
	
	entityActuallyUsed = false;
	
	bestActor = getBestActorToUse( touch, num );

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for( i = 0; i < num; i++ )
	{
		hit = &g_entities[ touch[ i ] ];
		if ( !hit->inuse )
		{
			continue;
		}
		
		if ( usingEntity == hit->entity )
			continue;

		if ( hit->entity->isSubclassOf( Actor ) && ( hit->entity != bestActor ) )
			continue;
		
		assert( hit->entity );
		
		event = new Event( EV_Use );
		event->AddEntity( usingEntity );
		hit->entity->ProcessEvent( event );
		
		if ( hit->entity->isSubclassOf( Weapon ) )
			continue;

		if ( hit->entity->isSubclassOf( Projectile ) )
			continue;

		if ( hit->entity->getSolidType() == SOLID_NOT )
			continue;

		entityActuallyUsed = true;
	}
	
	// If we didn't use any object in the world, use our holdable item (if any)
	
	if ( !entityActuallyUsed )
		useHoldableItem();
}

Actor *Player::getBestActorToUse( int *entityList, int count )
{
	int i;
	float bestYawDiff = 1000.0f;
	float yawDiff;
	Actor *bestActor = NULL;
	Actor *currentActor;
	gentity_t *edict;
	Vector dir;
	Vector angles;

	for ( i = 0 ; i < count ; i++ )
	{
		edict = &g_entities[ entityList[ i ] ];

		if ( !edict->inuse || !edict->entity || !edict->entity->isSubclassOf( Actor ) )
			continue;

		currentActor = (Actor *)edict->entity;

		dir = currentActor->centroid - centroid;
		dir.normalize();
		angles = dir.toAngles();

		yawDiff = AngleNormalize180( AngleNormalize180( v_angle[ YAW ] ) - AngleNormalize180( angles[ YAW ] ) );
		yawDiff = abs( (int)yawDiff );

		if ( yawDiff < bestYawDiff )
		{
			bestYawDiff = yawDiff;
			bestActor = currentActor;
		}
	}

	return bestActor;
}

void Player::TouchStuff( const pmove_t *pm )
{
	gentity_t  *other;
	Event		*event;
	int		i;
	int		j;
	
	//
	// clear out any conditionals that are controlled by touching
	//
	toucheduseanim = NULL;
	
	if ( GetMovePlayerMoveType() != PM_NOCLIP )
	{
		// See if we should touch all triggers or just teleporters

		if ( multiplayerManager.inMultiplayer() && multiplayerManager.isPlayerSpectator( this ) )
			G_TouchTeleporters( this );
		else
			G_TouchTriggers( this );
	}
	
	// touch other objects
	for( i = 0; i < pm->numtouch; i++ )
	{
		other = &g_entities[ pm->touchents[ i ] ];
		
		for( j = 0; j < i ; j++ )
		{
			gentity_t *ge = &g_entities[ j ];
			
			if ( ge == other )
				break;
		}
		
		if ( j != i )
		{
			// duplicated
			continue;
		}
		
		// Don't bother touching the world
		if ( ( !other->entity ) || ( other->entity == world ) )
		{
			continue;
		}
		
		event = new Event( EV_Touch );
		event->AddEntity( this );
		other->entity->ProcessEvent( event );
		
		event = new Event( EV_Touch );
		event->AddEntity( other->entity );
		ProcessEvent( event );
	}
}

//-----------------------------------------------------
//
// Name:		disableInventory
// Class:		Player
//
// Description:	Disables the player's inventory
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void Player::disableInventory( void )
{
	client->ps.pm_flags |= PMF_DISABLE_INVENTORY;
}


//-----------------------------------------------------
//
// Name:		enableInventory
// Class:		Player
//
// Description:	Enables the players inventory
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void Player::enableInventory( void )
{
	client->ps.pm_flags &= ~PMF_DISABLE_INVENTORY;
}

usercmd_t Player::GetLastUcmd(void) 
{
	return last_ucmd;
}

void Player::GetMoveInfo( pmove_t *pm )
{
	moveresult = pm->moveresult;
	
	if ( !deadflag || ( multiplayerManager.inMultiplayer() && multiplayerManager.isPlayerSpectator( this ) ) )
	{
		v_angle[ 0 ] = pm->ps->viewangles[ 0 ];
		v_angle[ 1 ] = pm->ps->viewangles[ 1 ];
		v_angle[ 2 ] = pm->ps->viewangles[ 2 ];
		
		if ( moveresult == MOVERESULT_TURNED )
		{
			angles.y = v_angle[ 1 ];
			setAngles( angles );
			SetViewAngles( angles );
		}
	}
	
	setOrigin( Vector( pm->ps->origin[ 0 ], pm->ps->origin[ 1 ], pm->ps->origin[ 2 ] ) );
	velocity = Vector( pm->ps->velocity[ 0 ], pm->ps->velocity[ 1 ], pm->ps->velocity[ 2 ] );
	
	if ( ( client->ps.pm_flags & PMF_FROZEN ) || ( client->ps.pm_flags & PMF_NO_MOVE ) )
	{
		velocity = vec_zero;
	}
	else
	{
		if ( !vehicle )
			setSize( pm->mins, pm->maxs );
		else
			setSize( vehicle->_DriverBBoxMins , vehicle->_DriverBBoxMaxs );
	}
	
	// water type and level is set in the predicted code
	waterlevel = pm->waterlevel;
	watertype = pm->watertype;
	
	// Set the ground entity
	groundentity = NULL;
	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
	{
		groundentity = &g_entities[ pm->ps->groundEntityNum ];
		airspeed = 350;
	}
}

void Player::SetMoveInfo( pmove_t *pm, const usercmd_t *ucmd )
{
	Vector move;
	
	//float test;
	//test = velocity[0];
	//test = velocity[1];
	//test = velocity[2];
	// set up for pmove
	memset( pm, 0, sizeof( pmove_t ) );
	
	velocity.copyTo( client->ps.velocity );
	
	pm->ps             = &client->ps;
	
	if ( ucmd )
	{
		pm->cmd            = *ucmd;
	}
	
	//pm->tracemask      = MASK_PLAYERSOLID;
	pm->tracemask      = edict->clipmask;
	pm->trace          = gi.trace;
	pm->pointcontents  = gi.pointcontents;
	pm->trypush        = TryPush;
	
	pm->ps->origin[ 0 ] = origin.x;
	pm->ps->origin[ 1 ] = origin.y;
	pm->ps->origin[ 2 ] = origin.z;
	
	pm->ps->velocity[ 0 ] = velocity.x;
	pm->ps->velocity[ 1 ] = velocity.y;
	pm->ps->velocity[ 2 ] = velocity.z;
	
	// save off pm_runtime
	if ( pm->ps->pm_runtime )
		pm_lastruntime = pm->ps->pm_runtime;
}

pmtype_t Player::GetMovePlayerMoveType( void )
{
	if ( multiplayerManager.isPlayerSpectator( this, SPECTATOR_TYPE_FOLLOW ) )
	{
		return PM_SPECTATOR_FOLLOW;
	}
	else if ( multiplayerManager.isPlayerSpectator( this, SPECTATOR_TYPE_FREEFORM ) )
	{
		return PM_NOCLIP;
	}
	else if ( multiplayerManager.isPlayerSpectator( this ) )
	{
		return PM_SPECTATOR;
	}
	else if ( getMoveType() == MOVETYPE_NOCLIP )
	{
		return PM_NOCLIP;
	}
	else if ( deadflag )
	{
		return PM_DEAD;
	}
	else if ( _forcedMoveType != PM_NONE )
	{
		return _forcedMoveType;
	}
	else
	{
		return PM_NORMAL;
	}
}

void Player::CheckGround( void )
{
	pmove_t pm;
	
	SetMoveInfo( &pm, current_ucmd );
	Pmove_GroundTrace( &pm );
	GetMoveInfo( &pm );
}

qboolean Player::AnimMove( const Vector &move, Vector *endpos )
{
	trace_t trace;
	int     mask;
	Vector  start( origin );
	Vector  end( origin + move );
	
	mask = MASK_PLAYERSOLID;
	
	// test the player position if they were a stepheight higher
	trace = G_Trace( start, mins, maxs, end, this, mask, true, "AnimMove" );
	if ( trace.fraction < 1 )
	{
		return TestMove( move, endpos );
	}
	else
	{
		if ( endpos )
		{
			*endpos = trace.endpos;
		}
		
		return true;
	}
}

qboolean Player::TestMove( const Vector &move, Vector *endpos )
{
	trace_t trace;
	Vector pos( origin + move );
	
	trace = G_Trace( origin, mins, maxs, pos, this, edict->clipmask, true, "TestMove" );
	if ( trace.allsolid )
	{
		// player is completely trapped in another solid
		if ( endpos )
		{
			*endpos = origin;
		}
		return false;
	}
	
	if ( trace.fraction < 1.0f )
	{
		Vector up( origin );
		up.z += STEPSIZE;
		
		trace = G_Trace( origin, mins, maxs, up, this, edict->clipmask, true, "TestMove" );
		if ( trace.fraction == 0.0f )
		{
			if ( endpos )
            {
				*endpos = origin;
            }
			return false;
		}
		
		Vector temp( trace.endpos );
		Vector end( temp + move );
		
		trace = G_Trace( temp, mins, maxs, end, this, edict->clipmask, true, "TestMove" );
		if ( trace.fraction == 0.0f )
		{
			if ( endpos )
            {
				*endpos = origin;
            }
			return false;
		}
		
		temp = trace.endpos;
		
		Vector down( trace.endpos );
		down.z = origin.z;
		
		trace = G_Trace( temp, mins, maxs, down, this, edict->clipmask, true, "TestMove" );
	}
	
	if ( endpos )
	{
		*endpos = trace.endpos;
	}
	
	return true;
}

//--------------------------------------------------------------
//
// Name:			showObjectInfo
// Class:			Player
//
// Description:		This build the string to be displayed when
//					the sv_showinfo cvar is set.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::showObjectInfo()
{
	str desc;
	char addstr[512];
	
	Entity *ent = (Entity*)atobject;
	desc = "tiki  : " + ent->model + "\n";
	sprintf(addstr,"entnum: %d\n",ent->entnum);
	desc += addstr;
	sprintf(addstr,"origin: (%.2f, %.2f, %.2f)\n",ent->origin.x, ent->origin.y, ent->origin.z);
	desc += addstr;
	sprintf(addstr,"angles: (%.2f, %.2f, %.2f)\n",ent->angles.x, ent->angles.y, ent->angles.z);
	desc += addstr;
	sprintf(addstr,"bounds Mins: ( %.2f, %.2f, %.2f )\n", ent->mins.x, ent->mins.y, ent->mins.z);
	desc += addstr;
	sprintf(addstr,"bounds Maxs: ( %.2f, %.2f, %.2f )\n", ent->maxs.x, ent->maxs.y, ent->maxs.z );
	desc += addstr;
	sprintf(addstr,"size: ( %.2f, %.2f, %.2f )\n", ent->size.x, ent->size.y, ent->size.z );
	desc += addstr;
	sprintf(addstr,"velocity: ( %.2f, %.2f, %.2f )\n\n", ent->velocity.x, ent->velocity.y, ent->velocity.z );
	desc += addstr;
	
	if ( ent->isSubclassOf(Actor) )
	{
		Actor *act = (Actor *)ent;
		desc += "--------- SUBCLASS OF ACTOR ------------\n";
		sprintf(addstr, "Class ID: %s\n", act->getClassID() );
		desc += addstr;
		sprintf(addstr, "Classname: %s\n", act->getClassname() );
		desc += addstr;
		sprintf(addstr, "Name: %s\n", act->name.c_str() );
		desc += addstr;
		if ( act->part_name.length() > 0 )
		{
			sprintf(addstr, "Part name: %s\n", act->part_name.c_str() );
			desc += addstr;
		}
		sprintf(addstr, "Targetname: %s\n", act->TargetName() );
		desc += addstr;
		
		if ( act->behavior )
			sprintf(addstr, "Behavior: %s\n", act->behavior->getClassname() );
		else
			sprintf(addstr, "Behavior: NULL -- was '%s'\n", act->currentBehavior.c_str() );
		desc += addstr;
		
		if ( act->headBehavior )
			sprintf(addstr, "Head Behavior: %s\n", act->headBehavior->getClassname() );
		else
			sprintf(addstr, "Head Behavior: NULL -- was %s\n", act->currentHeadBehavior.c_str() );
		desc += addstr;
		
		if ( act->eyeBehavior )
			sprintf(addstr, "Eye Behavior: %s\n", act->eyeBehavior->getClassname() );
		else
			sprintf(addstr, "Eye Behavior: NULL -- was %s\n", act->currentEyeBehavior.c_str() );
		desc += addstr;
		
		if ( act->torsoBehavior )
			sprintf(addstr, "Torso Behavior: %s\n", act->torsoBehavior->getClassname() );
		else
			sprintf(addstr, "Torso Behavior: NULL -- was %s\n", act->currentTorsoBehavior.c_str() );
		desc += addstr;
		
		if ( act->currentState )
			sprintf(addstr, "State: %s\n", act->currentState->getName() );
		else
			sprintf(addstr, "State: NONE\n" );
		desc += addstr;
		
		if ( act->GetActorFlag( ACTOR_FLAG_AI_ON ) )
			sprintf(addstr, "AI is ON\n" );
		else
			sprintf(addstr, "AI is OFF\n" );
		desc += addstr;
		
		if ( act->isThinkOn() )
			sprintf(addstr, "Think is ON\n" );
		else
			sprintf(addstr, "Think is OFF\n" );
		desc += addstr;
		
		if ( act->mode == ACTOR_MODE_IDLE )
			sprintf(addstr, "Mode: IDLE\n" );
		else if ( act->mode == ACTOR_MODE_AI )
			sprintf(addstr, "Mode: AI\n" );
		else if ( act->mode == ACTOR_MODE_SCRIPT )
			sprintf(addstr, "Mode: SCRIPT\n" );
		else if ( act->mode == ACTOR_MODE_TALK )
			sprintf(addstr, "Mode: TALK\n" );
		desc += addstr;
		
		sprintf(addstr, "Actortype: %d\n", act->actortype );
		desc += addstr;
		sprintf(addstr, "Anim: %s\n", act->animname.c_str() );
		desc += addstr;
		sprintf(addstr, "Health: %f\n", act->health );
		desc += addstr;
		
		sprintf(addstr, "CurrentEnemy: " );
		desc += addstr;
		
		// Get our current enemy
		Entity *currentEnemy;
		currentEnemy = act->enemyManager->GetCurrentEnemy();
		if ( currentEnemy )
			sprintf(addstr, "%d : '%s'\n", currentEnemy->entnum, currentEnemy->targetname.c_str() );
		else
			sprintf(addstr, "None\n" );
		desc += addstr;
		
		switch( act->deadflag )
		{
		case DEAD_NO :
			sprintf(addstr, "deadflag: NO\n" );
			break;
		case DEAD_DYING :
			sprintf(addstr, "deadflag: DYING\n" );
			break;
		case DEAD_DEAD :
			sprintf(addstr, "deadflag: DEAD\n" );
			break;
		case DEAD_RESPAWNABLE :
			sprintf(addstr, "deadflag: RESPAWNABLE\n" );
			break;
		}
		desc += addstr;
		
	}
		
	G_SendCommandToPlayer( edict, "ui_addhud showinfo");
	G_EnableWidgetOfPlayer( edict, "InfoHUD", true );
	G_SetWidgetTextOfPlayer( edict, "InfoHUD", desc);		
}

//--------------------------------------------------------------
//
// Name:			clearActionType
// Class:			Player
//
// Description:		Clears the action icon on the hud and NULL's
//					out use entity.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::clearActionType()
{
	if ( lastActionType.length() )
	{
		G_EnableWidgetOfPlayer( edict, lastActionType.c_str(), false );
		//G_EnableWidgetOfPlayer( edict, "ActionTextArea", false );
		lastActionType = "";
	}
	
	atobject = NULL;
}

//--------------------------------------------------------------
//
// Name:			handleUseEntity
// Class:			Player
//
// Description:		Called from CheckMoveFlags to handle the 
//					case that our trace hit an entity.
//					We test to see if this is a usable entity
//
// Parameters:		Entity *ent -- Entity to test
//					float real_dist -- The real distance away from this entity,
//										ignoring pitch.
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::handleUseEntity(Entity *ent, float real_dist)
{
	if ( ent->hasUseData() )
	{
		// See if this entity is within distance to use.
		if ( real_dist > ent->useData->getUseMaxDist() )
		{
			clearActionType();
			return;
		}
		
		// Check to see if this entity is usable (due to count)
		if ( ent->useData->getUseCount() == 0)
		{
			// Our count is 0, we can't use this entity anymore,
			// so delete the useData for him.
			delete ent->useData;
			ent->useData = NULL;
			clearActionType();
			return;
		}
		
		// If he has a usetype show the HUD icon
		if ( ent->useData->getUseType().length() > 0 && !lastActionType.length() )
		{
			G_EnableWidgetOfPlayer( edict, ent->useData->getUseType().c_str(), true );
			//G_EnableWidgetOfPlayer( edict, "ActionTextArea", true );
			//G_SetWidgetTextOfPlayer( edict, "ActionTextArea", ent->getArchetype().c_str());
			lastActionType = ent->useData->getUseType();
		}
	}
	else
		clearActionType();
}

//--------------------------------------------------------------
//
// Name:			handleUseObject
// Class:			Player
//
// Description:		Called from CheckMoveFlags to handle the 
//					case that our trace hit a useobject
//
// Parameters:		UseObject *uo
//
// Returns:			None		
//
//--------------------------------------------------------------
void Player::handleUseObject(UseObject *uo)
{
	if ( uo->canBeUsed( origin, yaw_forward ) )
	{
		if ( uo->action_type.length() > 0 && !lastActionType.length() )
		{
			G_EnableWidgetOfPlayer( edict, uo->action_type.c_str(), true );
			lastActionType = uo->action_type;
		}
	}
	else // Not usable, clear HUD icon and data
		clearActionType();
}

void Player::CheckMoveFlags()
{
	trace_t trace;
	Vector start, end, fwd, yfwd;
	float oldsp, tracelen, real_dist;
	int content;
	Vector olddir( oldvelocity.x, oldvelocity.y, 0.0f );
	
	//MatrixTransformVector( base_righthand_pos, orientation, righthand_pos );
	//MatrixTransformVector( base_lefthand_pos, orientation, lefthand_pos );
	MatrixTransformVector( base_rightfoot_pos, orientation, rightfoot_pos );
	MatrixTransformVector( base_leftfoot_pos, orientation, leftfoot_pos );
	//righthand_pos += origin;
	//lefthand_pos += origin;
	rightfoot_pos += origin;
	leftfoot_pos += origin;
	
	//
	// Check if moving forward will cause the player to fall
	//
	start = origin + ( yaw_forward * 52.0f );
	end	= start;
	end.z -= STEPSIZE * 2.0f;
	
	trace = G_Trace( start, mins, maxs, end, this, edict->clipmask, true, "CheckMoveFlags" );
	canfall = !( trace.fraction < 1.0f );
	
	CheckGround();
	if ( !groundentity && ( velocity.z < -250.0f ) )
	{
		falling = true;
		hardimpact = false;
	}
	else
	{
		hardimpact = ( oldvelocity.z < -1000.0f );
		falling = false;
	}
	
	// check for running into walls
	oldsp = VectorNormalize( olddir );
	if ( ( oldsp > 250.0f ) && ( velocity * olddir < 5.0f ) )
		moveresult = MOVERESULT_HITWALL;
	
	//
	// Check if the useobject or useentity
	//
	start = origin;
	start[ 2 ] += viewheight;	
	GetVAngles().AngleVectors(&fwd);
	yfwd = yaw_forward;
	yfwd.normalize();
	
	// Normal test trace
	tracelen = 96.0f;
	content = MASK_USABLE;
	
	// If we have sv_showinfo on, use a better, longer trace.
	if ( sv_showinfo->integer )
	{
		_infoHudOn = true;
		tracelen = sv_showinfodist->value;
		
		if ( sv_showinfo->integer == 1 )
			content = MASK_ALL;
		else
			content = CONTENTS_BODY | CONTENTS_CORPSE;
	}
	
	// See if the cvar has been turned off, if so, clear the hud.
	if ( !sv_showinfo->integer && _infoHudOn )
	{
		G_EnableWidgetOfPlayer( edict, "InfoHUD", false );
		_infoHudOn = false;
	}
	
	float tracedist = tracelen / (Vector::Dot(fwd, yfwd)); // lengthen the trace to compensate for pitch
	end	= start + ( fwd * tracedist );
	trace = G_Trace( start, Vector(-5,-5,5), Vector(-5,-5,5), end, this, content, true, "CheckMoveFlags -- Checking for UseEntities" );
	if ( trace.ent && trace.ent->entity && ( trace.ent->entity != world ) )
	{
		atobject = trace.ent->entity;
		if ( trace.startsolid )
			trace.fraction = 0.0;
		atobject_dist = trace.fraction * tracedist;
		real_dist = (atobject_dist / tracedist) * tracelen; // real distance ignoring pitch
		atobject_dir.setXYZ( -trace.plane.normal[ 0 ], -trace.plane.normal[ 1 ], -trace.plane.normal[ 2 ] );
		
		if ( sv_showinfo->integer )
			showObjectInfo();
		
		if ( atobject->isSubclassOf( UseObject ) )
		{
			UseObject *uo = (UseObject *)(Entity *)atobject;
			handleUseObject(uo);
		}
		else 
		{
			Entity *ent = (Entity *)atobject;
			handleUseEntity(ent, real_dist); // see if this entity is usable
		}
	}
	else // Didn't hit anything
	{
		if ( sv_showinfo->integer )
		{
			G_EnableWidgetOfPlayer( edict, "InfoHUD", false );
		}
		
		clearActionType();
	}
	
	/*
	//
	// get the distances the player can move left, right, forward, and back
	//
	if ( ( movecontrol == MOVECONTROL_USER ) || ( movecontrol == MOVECONTROL_LEGS ) )
	{
		move_left_dist     = TestMoveDist( yaw_left * 128.0f );
		move_right_dist    = TestMoveDist( yaw_left * -128.0f );
		move_backward_dist = TestMoveDist( yaw_forward * -128.0f );
		move_forward_dist  = TestMoveDist( yaw_forward * 128.0f );
	}
	else
	{
		move_left_dist     = CheckMoveDist( yaw_left * 2.0f );
		move_right_dist    = CheckMoveDist( yaw_left * -2.0f );
		move_backward_dist = CheckMoveDist( yaw_forward * -2.0f );
		move_forward_dist  = CheckMoveDist( yaw_forward * 2.0f );
	}
	*/
}

qboolean Player::CheckMove( const Vector &move, Vector *endpos )
{
	return AnimMove( move, endpos );
}

void Player::ApplyPowerupEffects(int &moveSpeed)
{
	if ( _powerup )
		moveSpeed *= (int)_powerup->getMoveMultiplier();

	if ( _rune )
		moveSpeed *= (int)_rune->getMoveMultiplier();

	/* if ( poweruptype == POWERUP_SPEED )
		moveSpeed *= 2;
		
	if ( poweruptype == POWERUP_STEALTH )
	{
		if ( !(flags & FL_NOTARGET) )
			flags ^= FL_NOTARGET;
	}
		
	if ( poweruptype == POWERUP_PROTECTION )
	{
		if ( !(flags & FL_GODMODE) )
			flags ^= FL_GODMODE;
	}

	if ( poweruptype == POWERUP_FLIGHT )
	{
		client->ps.pm_flags |= PMF_FLIGHT;
	}	
	
	if ( poweruptype == POWERUP_ACCURACY )
	{
		// Tell the weapon we have the accuracy powerup
		Weapon *weap;
		weap = GetActiveWeapon( WEAPON_DUAL );
		if ( weap )
			weap->SetAccuracyPowerup( true );
	} */
}

//----------------------------------------------------------------
// Name:			applyWeaponSpeedModifiers
// Class:			Player
//
// Description:		Applies any weapon speed modifiers
//
// Parameters:		int *moveSpeed					- the move speed to modify
//
// Returns:			none
//----------------------------------------------------------------

void Player::applyWeaponSpeedModifiers( int *moveSpeed )
{
	Weapon *weap;

	weap = GetActiveWeapon( WEAPON_DUAL );

	if ( weap )
	{
		*moveSpeed *= (int) weap->getMoveSpeedModifier();
	}

}

void Player::ClientMoveLadder( usercmd_t *ucmd )
{	
	if ( _onLadder )
	{		
		// Get off the ladder if the player jumps, the player touches the ground while moving down, or the player
		//   is no longer on the ladder physically

		if ( ucmd->upmove > 0 )
		{
			velocity = _ladderNormal * sv_jumpvelocity->value;
			_nextLadderTime = level.time + 0.20;
		}
		else
		{
			Vector climbDir;
			Vector climbAngles;
			Vector playerForward;
			Vector playerLeft;
			Vector forward;
			Vector left;
			Vector climbForward;
			Vector climbLeft;
			Vector climbUp;
			bool goingUp;
			float upAmount;
			float rightAmount;
			float forwardAmount;
			trace_t trace;
			bool pushForward;
			
			// Change the forward velocity to directly up or down depending on how the player is facing and whether
			//   he is trying to go forwards or backwards

			v_angle.AngleVectors( &playerForward, &playerLeft );
			
			if ( playerForward.z > 0.0f )
				goingUp = true;
			else
				goingUp = false;

			if ( ucmd->forwardmove < 0 )
				goingUp = !goingUp;

			// Get the ladder climbing angles

			climbDir = -_ladderNormal;

			climbAngles = climbDir.toAngles();
			climbAngles.AngleVectors( &climbForward, &climbLeft, &climbUp );

			// Determine if we want to push against the ladder
			
			trace = G_Trace( origin, mins, maxs, origin + climbForward * 4, this, edict->clipmask, true, "ClientMoveLadder" );

			if ( trace.fraction < 1.0f )
				pushForward = false;
			else
				pushForward = true;

			// Get the forward/backwards movement of the player

			if ( ucmd->forwardmove < 0 )
				playerForward *= -1;

			// Get the up/down amount from the forward movement 

			upAmount = playerForward * climbUp;

			if ( ( upAmount >= -0.1f ) && ( upAmount <= 0.1f ) )
				upAmount = 0.1f;

			forwardAmount =  abs( (int)(playerForward * climbForward) );

			if ( upAmount >= 0.0f )
				upAmount += forwardAmount;
			else
				upAmount -= forwardAmount;

			// Get the left/right amount from the forward movement 

			rightAmount = playerForward * climbLeft;

			if ( ( rightAmount >= -0.1f ) && ( rightAmount <= 0.1f ) )
				rightAmount = 0.0f;

			// Calculate the forward movement

			forward = ( climbUp * upAmount + climbLeft * rightAmount ) * 0.5f;
			forward.normalize();

			if ( pushForward )
			{
				forward = ( climbForward + forward ) * 0.5f;
			}

			// Get the left/right movement of the player

			if ( ucmd->rightmove > 0 )
				playerLeft *= -1;

			// Get the up/down amount from the strafe movement 

			upAmount = playerLeft * climbUp;

			forwardAmount = playerLeft * climbForward;

			upAmount += forwardAmount;

			// Get the left/right amount from the strafe movement 

			rightAmount = playerLeft * climbLeft;

			if ( ( rightAmount >= -0.1f ) && ( rightAmount <= 0.1f ) )
				rightAmount = 0.0f;

			// Calculate the strafe movement

			left = ( climbUp * upAmount + climbLeft * rightAmount ) * 0.5f;
			left.normalize();

			if ( pushForward )
			{
				left = ( climbForward + left ) * 0.5f;
			}

			// Change our velocity based on the forward/strafe movements calculated

			forward.normalize();

			forward *= abs( ucmd->forwardmove ) / 127.0f * sv_maxspeed->value * 0.75f;

			left *= abs( ucmd->rightmove ) / 127.0f * sv_maxspeed->value * 0.75f;
			
			velocity = forward + left;
			
			ucmd->forwardmove = 0;
			
			client->ps.pm_flags |= PMF_NO_GRAVITY;

			// If we have a groundentity and we are going down get off the ladder

			if ( groundentity && !goingUp )
			{
				_nextLadderTime = level.time + 0.20;
			}
		}

		_onLadder = false;
	}
}

void Player::ClientMoveDuck( usercmd_t *ucmd )
{
	if ( client->ps.pm_flags & PMF_DUCKED )
	{
		// See if we can stand where we are, otherwise we have to stay ducked.
		Vector newmins( mins );
		Vector newmaxs( maxs );
		Vector tmporg( origin );
		trace_t trace;
		tmporg[ 2 ] += 2.0f;
		newmins[ 2 ] = MINS_Z;
		newmaxs[ 2 ] = MAXS_Z;
		trace = G_Trace( tmporg, newmins, newmaxs, tmporg, this, edict->clipmask, true, "checkcanstand" );
		if ( trace.startsolid )
		{
			client->ps.pm_flags |= PMF_DUCKED;
		}
		else
		{
			client->ps.pm_flags &= ~PMF_DUCKED;
		}
	}
	
	if ( ucmd->upmove < 0 && sv_cancrouch->integer )
		client->ps.pm_flags |= PMF_DUCKED;
}

//-----------------------------------------------
//	Name:			ClientMoveLean
//
//	Class:			Player
//
//	Description:	Leans the player
//
//	Parameters:		ucmd - the structure of user commands from the client
//
//	Returns:		None
//-----------------------------------------------
void Player::ClientMoveLean( usercmd_t *ucmd )
{
	Q_UNUSED(ucmd);
/*
	client->ps.pm_flags &= ~(PMF_LEAN_RIGHT | PMF_LEAN_LEFT);
	
	if(ucmd->lean > 0)
	{
		client->ps.pm_flags |= PMF_LEAN_RIGHT;
	}
	
	if(ucmd->lean < 0)
	{
		client->ps.pm_flags |= PMF_LEAN_LEFT;
	}
	*/
}

void Player::ClientMoveFlagsAndSpeeds( int moveSpeed, int noclipSpeed, int crouchSpeed, int airSpeed )
{
	if ( level.playerfrozen || ( flags & FL_STUNNED ) )
	{
		client->ps.pm_flags |= PMF_FROZEN;
	}
	
	if ( ( flags & FL_IMMOBILE ) || ( flags & FL_PARTIAL_IMMOBILE ) )
	{
		client->ps.pm_flags |= PMF_NO_MOVE;
		//client->ps.pm_flags |= PMF_NO_PREDICTION;
	}
	
	if ( movecontrol == MOVECONTROL_ANIM )
		client->ps.pm_flags |= PMF_NO_PREDICTION;
	
	if ( !groundentity )
	{
		client->ps.speed = airSpeed;
	}
	else if ( getMoveType() == MOVETYPE_NOCLIP || ( waterlevel > 1 ) )
	{
		// No clip underwater speeds
		if ( last_ucmd.buttons & BUTTON_RUN )
			client->ps.speed = noclipSpeed;
		else
			client->ps.speed = noclipSpeed/2;
	}
	else
	{
		// Duck speed
		if ( client->ps.pm_flags & PMF_DUCKED )
		{
			client->ps.speed = crouchSpeed;	
		}
		else
		{
			// Normal run/walk speed
			if ( sv_useanimmovespeed->integer || movecontrol == MOVECONTROL_ANIM )
			{
				client->ps.speed = (int) animspeed;
			}
			else
			{
				if ( last_ucmd.buttons & BUTTON_RUN )
					client->ps.speed = moveSpeed;
				else
					client->ps.speed = moveSpeed/2;
			}
		}
	}
	
	if ( getMoveType() == MOVETYPE_NOCLIP )
	{
		// Normal noclip speed (run/walk)
		if ( last_ucmd.buttons & BUTTON_RUN )
			client->ps.speed = noclipSpeed;
		else
			client->ps.speed = noclipSpeed/2;
	}
	
	if ( sv_instantjump->integer )
		client->ps.instantJump = true;
	else
		client->ps.instantJump = false;

	if ( sv_strafeJumpingAllowed->integer )
		client->ps.strafeJumpingAllowed = true;
	else
		client->ps.strafeJumpingAllowed = false;
}

void Player::ClientMoveMisc( usercmd_t *ucmd )
{
	// We're falling, can't move around in the air
	if ( client->ps.feetfalling && ( waterlevel < 2 ) && !( client->ps.pm_time ) )
	{
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
	}
	
	// Fake player, cancel any movement input
	if ( fakePlayer_active || movecontrol == MOVECONTROL_ANIM )
	{
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
	}
}

void Player::ClientMove( usercmd_t *ucmd )
{
	pmove_t pm;
	Vector move;
	
	int moveSpeed;
	int noclipSpeed = (int) sv_noclipspeed->value;
	int crouchSpeed;
	int airSpeed;
	
	if ( world->getPhysicsVar( WORLD_PHYSICS_MAXSPEED ) != -1.0f )
		moveSpeed = (int) world->getPhysicsVar( WORLD_PHYSICS_MAXSPEED );
	else
		moveSpeed = (int) sv_maxspeed->value;
	
	if ( sv_crouchspeed->value > 0.0f )
		crouchSpeed = (int) sv_crouchspeed->value;
	else
		crouchSpeed = (int)((float)moveSpeed * 0.5);
	
	if ( sv_airmaxspeed->value > 0.0f )
		airSpeed = (int) sv_airmaxspeed->value;
	else
		airSpeed = moveSpeed;
	
	// Save off current origin
	oldorigin = origin;
	
	client->ps.pm_type = GetMovePlayerMoveType();
	
	// Clear movement flags
	client->ps.pm_flags &= ~( PMF_FLIGHT | PMF_FROZEN | PMF_NO_PREDICTION | PMF_NO_MOVE | PMF_HAVETARGET | PMF_NO_GRAVITY );
	
	// Modify speeds 
	
	ApplyPowerupEffects( moveSpeed );
	applyWeaponSpeedModifiers( &moveSpeed );
	
	if ( multiplayerManager.inMultiplayer() )
		multiplayerManager.applySpeedModifiers( this, &moveSpeed );
	
	ApplyPowerupEffects( crouchSpeed );
	applyWeaponSpeedModifiers( &crouchSpeed );
	
	if ( multiplayerManager.inMultiplayer() )
		multiplayerManager.applySpeedModifiers( this, &crouchSpeed );
	
	ApplyPowerupEffects( airSpeed );
	applyWeaponSpeedModifiers( &airSpeed );
	
	if ( multiplayerManager.inMultiplayer() )
		multiplayerManager.applySpeedModifiers( this, &airSpeed );
	
	// Handle ducking
	ClientMoveDuck(ucmd);
	
	//Handle leaning
	ClientMoveLean(ucmd);
	
	// Movement speed and mobility flags
	ClientMoveFlagsAndSpeeds(moveSpeed, noclipSpeed, crouchSpeed, airSpeed);
	
	// Misc movement stuff, like falling, and fake player control
	ClientMoveMisc(ucmd);
	
	// Handle ladder situations
	ClientMoveLadder(ucmd);
	
	// Gravity modifier
	client->ps.gravity = (int)(sv_currentGravity->value * gravity);
	
	// Perform the move
	CheckGround();
	SetMoveInfo( &pm, ucmd );
	Pmove( &pm );
	GetMoveInfo( &pm );
	ProcessPmoveEvents( pm.pmoveEvent );
	
	// Animation driven stuff happens here
	if ( ( getMoveType() == MOVETYPE_NOCLIP ) || !( client->ps.pm_flags & PMF_NO_PREDICTION ) )
	{
		total_delta = vec_zero;
	}
	else
	{
		if ( movecontrol == MOVECONTROL_ABSOLUTE )
		{
			velocity = vec_zero;
		}
		
		if ( total_delta != vec_zero )
		{
			// Animation driven move
			MatrixTransformVector( total_delta, orientation, move );
			CheckMove( move, &origin );
			setOrigin( origin );
			CheckGround();
		}
	}
	
	total_delta = vec_zero;
	
	TouchStuff( &pm );
	
	// Debug output
	if ( ( whereami->integer ) && ( origin != oldorigin ) )
		gi.DPrintf( "x %8.2f y%8.2f z %8.2f area %2d\n", origin[ 0 ], origin[ 1 ], origin[ 2 ], edict->areanum );
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void Player::ClientThink( Event * )
{
	// sanity check the command time to prevent speedup cheating
	if ( current_ucmd->serverTime > level.inttime )
	{
		//
		// we don't want any future commands, these could be from the previous game
		//
		return;
	   }
	
	if ( current_ucmd->serverTime < ( level.inttime - 1000 ) )
	{
		current_ucmd->serverTime = level.inttime - 1000;
	   }
	
	if ( ( current_ucmd->serverTime - client->ps.commandTime ) < 1 )
	{
		return;
	}
	
	_started = true;

	last_ucmd   = *current_ucmd;
	new_buttons	= current_ucmd->buttons & ~buttons;
	buttons		= current_ucmd->buttons;
	
	// Handle Heath Regen if it's in the gameplay database
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	str scopestr = "CurrentPlayer.HPRegen";
	if ( gpm->hasObject(scopestr) )
	{
		int value = (int)gpm->getFloatValue(scopestr, "value");
		if ( level.inttime > _nextRegenTime && value > 0 )
		{
			_nextRegenTime = level.inttime + ((6-value) * 1000);
			AddHealth(1.0f);
		}
	}
	
	if ( _useEntityStartTimer != 0.0f )
	{
		if ( level.time > _useEntityStartTimer )
		{
			_usingEntity = false;
			_useEntityStartTimer = 0.0f;
		}
	}
		
	if ( current_ucmd->thirdperson )
		_isThirdPerson = true;
	else
		_isThirdPerson = false;
	
	// Set cvar values
	if ( world->getPhysicsVar( WORLD_PHYSICS_AIRACCELERATE ) != -1.0f )
		client->ps.pm_airaccelerate = (int)  world->getPhysicsVar( WORLD_PHYSICS_AIRACCELERATE );
	else
		client->ps.pm_airaccelerate = (int) sv_airaccelerate->value;
	
	if ( multiplayerManager.inMultiplayer() )
		multiplayerManager.applyAirAccelerationModifiers( this, &client->ps.pm_airaccelerate );
	
	client->ps.pm_wateraccelerate = (int) sv_wateraccelerate->value;
	client->ps.pm_stopspeed = (int) sv_stopspeed->value;
	client->ps.pm_friction = (int) sv_friction->value;
	client->ps.pm_waterfriction = (int) sv_waterfriction->value;
	client->ps.jumpvelocity = (int) sv_jumpvelocity->value;
	
	if ( multiplayerManager.inMultiplayer() )
		multiplayerManager.applyJumpModifiers( this, &client->ps.jumpvelocity );
	
	client->ps.crouchjumpvelocity = (int) sv_crouchjumpvelocity->value;
	
	client->ps.pm_accelerate = (int) sv_accelerate->value;
	client->ps.pm_defaultviewheight = (int) sv_defaultviewheight->value;
	
	viewheight = client->ps.pm_defaultviewheight;
	
	
	
	if ( level.intermissiontime )
	{
		client->ps.pm_flags |= PMF_FROZEN;
		
		if (g_endintermission->integer > 0 )
		{
			g_endintermission->integer = 0;
			level.exitintermission = true;
		}
		// can exit intermission after 10 seconds (default)
		if ( (( level.time - level.intermissiontime ) > level.intermission_advancetime)
			&& (level.intermission_advancetime != 0))
		{
			if ( multiplayerManager.inMultiplayer() )
			{
				//if ( new_buttons & BUTTON_ANY )
				if ( ( new_buttons & BUTTON_ATTACKRIGHT ) || ( new_buttons & BUTTON_ATTACKLEFT ) )
				{
					level.exitintermission = true;
				}
			}
			else
			{
				level.exitintermission = true;
				
			}
		}
		
		// Save cmd angles so that we can get delta angle movements next frame
		client->cmd_angles[ 0 ] = SHORT2ANGLE( current_ucmd->angles[ 0 ] );
		client->cmd_angles[ 1 ] = SHORT2ANGLE( current_ucmd->angles[ 1 ] );
		client->cmd_angles[ 2 ] = SHORT2ANGLE( current_ucmd->angles[ 2 ] );
		
		return;
	}
	
	moveresult = MOVERESULT_NONE;
	
	if ( !vehicle || !vehicle->Drive( current_ucmd ) )
	{
		ClientMove( current_ucmd );
	}
	
	if ( vehicle )
	{
		pmove_t pm;
		
		SetMoveInfo( &pm, current_ucmd );
		Pmove( &pm );
	}
	

	if(current_ucmd)
	{
		_crossHairXOffset		= current_ucmd->choffset[0];
		_crossHairYOffset		= current_ucmd->choffset[1];
	}
	

	client->ps.pm_flags &= ~(PMF_RADAR_MODE | PMF_SCANNER );
	
	CheckForTargetedEntity();
	ProcessTargetedEntity();
	
	Weapon *weap;
	weap = GetActiveWeapon( WEAPON_DUAL );
	if ( weap )
	{
		if(weap->GetPutaway())
		{
			//			weap->SetTargetedEntity(0);
		}
		else
		{
			//			weap->SetRealViewOrigin();
			//			weap->SetThirdPerson();
			//			weap->SetCHOffset( current_ucmd->choffset[0],  current_ucmd->choffset[1]);
	
			weap->ProcessTargetedEntity(GetTargetedEntity());
		}

		if ( ( ( weap->GetFireType( FIRE_MODE1 ) == FT_CONTROL_PROJECTILE ) && ( new_buttons & BUTTON_ATTACKLEFT ) ) ||
			 ( ( weap->GetFireType( FIRE_MODE2 ) == FT_CONTROL_PROJECTILE ) && ( new_buttons & BUTTON_ATTACKRIGHT ) ) )
		{
			weap->toggleProjectileControl();
		}

		if ( ( ( weap->GetFireType( FIRE_MODE1 ) == FT_CONTROL_ZOOM ) && ( new_buttons & BUTTON_ATTACKLEFT ) ) ||
			 ( ( weap->GetFireType( FIRE_MODE2 ) == FT_CONTROL_ZOOM ) && ( new_buttons & BUTTON_ATTACKRIGHT ) ) )
		{
			weap->changeZoomStage( 40.0f, 10.0f );
		}
	}
	
	// If we're charging up for a special move, set the current time
	if ( specialMoveCharge > 0.0f )
	{
		specialMoveCharge = level.time;
		//gi.Printf("Charging Up... Time: %.2f\n",specialMoveEndTime - level.time);
	}
	
	
	// only evaluate the state when not noclipping
	if ( getMoveType() == MOVETYPE_NOCLIP )
	{
		// force the stand animation if were in noclip
		SetAnim( "idle", all );
	}
	else
	{
		// set flags for state machine
		CheckMoveFlags();
		EvaluateState();
		
		if ( groundentity && groundentity->entity && groundentity->entity->isSubclassOf( Actor ) )
		{
			Event *event = new Event( EV_Actor_Push );
			event->AddVector( Vector( 0.0f, 0.0f, -10.0f ) );
			groundentity->entity->PostEvent( event, 0.0f );
		}
	}
	
	oldvelocity = velocity;
	old_v_angle  = v_angle;
	
	// If we're dying, check for respawn
	if ( ( deadflag == DEAD_DEAD && ( level.time > respawn_time ) ) )
	{
		// wait for any button just going down
		//if ( new_buttons || ( multiplayerManager.inMultiplayer() && multiplayerManager.checkFlag( MP_FLAG_FORCE_RESPAWN ) ) )
		if ( ( ( new_buttons & BUTTON_ATTACKRIGHT ) || ( new_buttons & BUTTON_ATTACKLEFT ) ) || 
			   ( multiplayerManager.inMultiplayer() && multiplayerManager.checkFlag( MP_FLAG_FORCE_RESPAWN ) ) )
		{
			if ( multiplayerManager.inMultiplayer() )
			{
				//respawn_time = level.time ;
				multiplayerManager.respawnPlayer( this, false );
				//				ProcessEvent( EV_Player_Respawn );
			}
			
			else
			{
				G_RestartLevelWithDelay( 1.0f );
			}
		}
	}
	
	// Save cmd angles so that we can get delta angle movements next frame
	client->cmd_angles[ 0 ] = SHORT2ANGLE( current_ucmd->angles[ 0 ] );
	client->cmd_angles[ 1 ] = SHORT2ANGLE( current_ucmd->angles[ 1 ] );
	client->cmd_angles[ 2 ] = SHORT2ANGLE( current_ucmd->angles[ 2 ] );

	if ( client->ps.pm_flags & PMF_CAMERA_VIEW )
	{
		VectorCopy( current_ucmd->realvieworigin, client->ps.camera_origin );
		VectorCopy( current_ucmd->realviewangles, client->ps.camera_angles );
	}
	
	if ( g_logstats->integer )
	{
		if ( !logfile_started )
		{
			ProcessEvent( EV_Player_LogStats );
			logfile_started = true;
		}
	}
	
	if ( ( last_ucmd.buttons & BUTTON_USE ) != 0 )
		ProcessEvent( EV_Player_DoUse );
	
	if ( ( last_ucmd.buttons & BUTTON_DROP_RUNE ) != 0 )
		dropRune();
	
	if ( ( last_ucmd.buttons & BUTTON_TRANSFER_ENERGY ) != 0 )
		transferEnergy();

	if ( multiplayerManager.inMultiplayer() )
		multiplayerManager.playerInput( this, new_buttons );
	
	// Check for incoming melee attacks and set the incomingMeleeAttack flag
	incomingMeleeAttack = false;
	if ( meleeAttackerList.NumObjects() )
	{
		Entity *attacker;
		int i;
		for( i = meleeAttackerList.NumObjects() ; i >= 1 ; i-- )
		{
			attacker = (Entity*)meleeAttackerList.ObjectAt(i);
			if ( !attacker )
				continue;
			
			if ( attacker->isSubclassOf(Actor) )
			{
				Actor *act = (Actor *)attacker;
				if ( act->in_melee_attack )
					incomingMeleeAttack = true;
				else
					meleeAttackerList.RemoveObjectAt( i );
			}
		}
		
		// No one is attacking us, make sure the list is clear
		if ( !incomingMeleeAttack )
			meleeAttackerList.ClearObjectList();
	}
}

// This function returns the endpoint of where the crosshair is located on the
// screen.  Regardless of whether we're in camera, first or 3rd person view.
void Player::GetViewTrace( trace_t& trace,  int contents, float maxDistance )
{
//	trace_t trace;
	Vector f,r,u,p,s;
	Vector pos, forward, endpoint, vorg, dir;
    vec3_t viewAngles;
	

	if (client->ps.pm_flags & PMF_CAMERA_VIEW ) // 3rd person automatic camera
	{
		vorg = client->ps.camera_origin;
		VectorCopy(client->ps.camera_angles, viewAngles);
	}
	else if ( !_isThirdPerson ) // First person
	{
		vorg = origin;
		vorg.z += client->ps.viewheight;
		VectorCopy(GetVAngles(), viewAngles);
	}
	else // Third person
	{
		//vorg = current_ucmd->realviewangles;
		VectorCopy(GetVAngles(), viewAngles);
		
	}
	
	vec3_t left;
	//Adjust the view end point if we are leaning.
	if( client->ps.leanDelta != 0)
	{
		viewAngles[2] -= client->ps.leanDelta / 2.0f;

		AngleVectors( viewAngles, NULL, left, NULL );
		VectorMA( vorg, client->ps.leanDelta, left, vorg );
	}

	// Add the damage angles
	VectorSubtract( viewAngles, client->ps.damage_angles, viewAngles );
	
	AngleVectors(viewAngles, f,r,u);
	
	float xmax,ymax, fov_x, fov_y, x;
	
    fov_x = client->ps.fov;
	x = 640.0f / (float)tan( fov_x / 360.0 * M_PI );
	fov_y = (float)atan2( 480.0f, x );
	fov_y *= 360.0f / M_PI;
	
	ymax = 4.0f * (float)tan( fov_y * M_PI / 360.0 );
	xmax = 4.0f * (float)tan( fov_x * M_PI / 360.0 );
	
	p.x = (float) _crossHairXOffset * (-xmax / 320.0f);
	p.y = (float) _crossHairYOffset * (-ymax / 240.0f);
	s = ( 4.0f * f ) + ( p.x * r ) + ( p.y * u );
	s.normalize();
	endpoint = vorg + (s * maxDistance );
	
	if ( !multiplayerManager.inMultiplayer() || multiplayerManager.fullCollision() )
		trace = G_FullTrace( vorg, vec_zero, vec_zero, endpoint, this, contents, true, "Player::GetViewEndPoint" );
	else
		trace = G_Trace( vorg, vec_zero, vec_zero, endpoint, this, contents, true, "Player::GetViewEndPoint" );
	
	//endpoint = trace.endpos;
	
	//G_DebugLine( vorg, endpoint, 1,0,1,1 );
	
	//return endpoint;
}
//-----------------------------------------------------
//
// Name:		CheckForTargetedEntity
// Class:		Player
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::CheckForTargetedEntity( void )
{
	trace_t viewTrace;
		
	memset(&viewTrace,0,sizeof(trace_t));
	GetViewTrace(viewTrace, MASK_SHOT | CONTENTS_TARGETABLE);
	
	if ( viewTrace.ent && ( viewTrace.entityNum != ENTITYNUM_WORLD ))
	{
		if(viewTrace.ent->entity->isSubclassOf(Actor) && viewTrace.ent->entity->getHealth() <= 0)
		{
			SetTargetedEntity(0);
		}
		else
		{
			SetTargetedEntity(viewTrace.ent->entity);
		}
		
		return;
	}
	
	SetTargetedEntity(0);
}


//-----------------------------------------------------
//
// Name:		SetTargetedEntity
// Class:		Player
//
// Description:	Sets the targeted entity.
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::SetTargetedEntity(EntityPtr entity)
{
	//remove the EF_DISPLAY_INFO from the previous entity.
	if(_targetedEntity != 0)
		_targetedEntity ->edict->s.eFlags &= ~(EF_DISPLAY_INFO|EF_DISPLAY_DESC1 | EF_DISPLAY_DESC2 | EF_DISPLAY_DESC3);
	

	_targetedEntity  = entity;
	if(_targetedEntity  != 0)
		_targetedEntity->edict->s.eFlags |= EF_DISPLAY_INFO;
}



//-----------------------------------------------------
//
// Name:		ProcessTargetedEntity
// Class:		Player
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::ProcessTargetedEntity( void )
{
	if(_targetedEntity == 0)
		return;	
	
	_targetedEntity->edict->s.eFlags |= EF_DISPLAY_DESC1;
}


//--------------------------------------------------------------
//
// Name:			SetState
// Class:			Player
//
// Description:		Sets the state machine to the specific legs and torso state specified
//
// Parameters:		const str& legsstate -- Legs state name
//					const str& torsostate -- Torso state name
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::SetState(const str& legsstate, const str& torsostate)
{
	animdone_Legs = false;
	animdone_Torso = false;
	
	movecontrol = MOVECONTROL_USER;
	
	currentState_Legs = statemap_Legs->FindState( legsstate.c_str() );
	currentState_Torso = statemap_Torso->FindState( torsostate.c_str() );
	
	str legsAnim( currentState_Legs->getLegAnim( *this, &legs_conditionals ) );
	if ( !animate->HasAnim(legsAnim) )
		legsAnim = getGameplayAnim(legsAnim);
	if ( legsAnim == "" )
	{
		partAnim[ legs ] = "";
		animate->ClearLegsAnim();
	}
	else if ( legsAnim != "none" )
	{
		SetAnim( legsAnim, legs );
	}
	
	str torsoAnim( currentState_Torso->getTorsoAnim( *this, &torso_conditionals ) );
	if ( !animate->HasAnim(torsoAnim) )
		torsoAnim = getGameplayAnim(torsoAnim);
	if ( torsoAnim == "" )
	{
		partAnim[ torso ] = "";
		animate->ClearTorsoAnim();
	}
	else if ( torsoAnim != "none" )
	{
		SetAnim( torsoAnim.c_str(), torso );
	}
	
	movecontrol = currentState_Legs->getMoveType();
	if ( ( movecontrol < ( sizeof( MoveStartFuncs ) / sizeof( MoveStartFuncs[ 0 ] ) ) ) && ( MoveStartFuncs[ movecontrol ] ) )
	{
		( this->*MoveStartFuncs[ movecontrol ] )();
	}
	
	// This seems breaks the secret move mode because it's called at a bad time and causes the client/server angles
	// to get out of sync because delta_angles is not updated correctly.
	// I don't think it's necessary to call it anyway, so I'm removing it.
	//SetViewAngles( v_angle );
}


//--------------------------------------------------------------
//
// Name:			LoadStateTable
// Class:			Player
//
// Description:		Loads the state table for the player.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::LoadStateTable()
{
	statemap_Legs = NULL;
	statemap_Torso = NULL;

	freeConditionals( legs_conditionals );
	freeConditionals( torso_conditionals );

	statemap_Legs = GetStatemap( str( g_statefile->string ) + "_legs.st", ( Condition<Class> * )Conditions, &legs_conditionals, false );
	statemap_Torso = GetStatemap( str( g_statefile->string ) + "_torso.st", ( Condition<Class> * )Conditions, &torso_conditionals, false );

	SetState("STAND", "START");
}

void Player::ResetState( Event * )
{
	movecontrol = MOVECONTROL_USER;
	LoadStateTable();
}

void Player::StartPush( void )
{
	trace_t trace;
	Vector  end( origin + ( yaw_forward * 64.0f ) );
	
	trace = G_Trace( origin, mins, maxs, end, this, MASK_SOLID, true, "StartPush" );
	if ( trace.fraction == 1.0f )
	{
		return;
	}
	v_angle.y = vectoyaw( trace.plane.normal ) - 180.0f;
	SetViewAngles( v_angle );
	
	setOrigin( trace.endpos - ( yaw_forward * 0.4f ) );
}

void Player::StartUseAnim( void )
{
	UseAnim  *ua;
	Vector   neworg;
	Vector   newangles;
	str      newanim;
	str      state;
	str      camera;
	trace_t  trace;
	
	if ( toucheduseanim )
	{
		ua = ( UseAnim * )( Entity * )toucheduseanim;
	}
	else if ( atobject )
	{
		ua = ( UseAnim * )( Entity * )atobject;
	}
	else
	{
		return;
	}
	
	useitem_in_use = ua;
	toucheduseanim = NULL;
	atobject = NULL;
	
	if ( ua->GetInformation( this, &neworg, &newangles, &newanim, &useanim_numloops, &state, &camera ) )
	{
		trace = G_Trace( origin, mins, maxs, neworg, this, edict->clipmask, true, "StartUseAnim" );
		if ( trace.startsolid || ( trace.fraction < 1.0f ) )
		{
			gi.WDPrintf( "Move to UseAnim was blocked.\n" );
		}
		
		if ( !trace.startsolid )
		{
			setOrigin( trace.endpos );
		}
		
		setAngles( newangles );
		v_angle.y = newangles.y;
		SetViewAngles( v_angle );
		
		movecontrol = MOVECONTROL_ABSOLUTE;
		
		if ( state.length() )
		{
			State * newState;
			
			newState = statemap_Torso->FindState( state );
			if ( newState )
            {
				EvaluateState( newState );
            }
			else
            {
				gi.WDPrintf( "Could not find state %s on UseAnim\n", state.c_str() );
            }
		}
		else
		{
			if ( currentState_Torso )
            {
				if ( camera.length() )
				{
					currentState_Torso->setCameraType( camera );
				}
				else
				{
					currentState_Torso->setCameraType( "behind" );
				}
            }
			SetAnim( newanim, legs );
		}
	}
}

void Player::StartLoopUseAnim( void )
{
	useanim_numloops--;
}

void Player::FinishUseAnim( Event * )
{
	UseAnim  *ua;
	
	if ( !useitem_in_use )
		return;
	
	ua = ( UseAnim * )( Entity * )useitem_in_use;
	ua->TriggerTargets( this );
	useitem_in_use = NULL;
}

void Player::SetupUseObject()
{
	UseObject  *uo;
	Vector   neworg;
	Vector   newangles;
	str      state;
	trace_t  trace;
	
	if ( atobject )
		uo = ( UseObject * )( Entity * )atobject;
	else
		return;
	
	useitem_in_use = uo;
	
	uo->Setup( this, &neworg, &newangles, &state );
	if ( uo->movetheplayer )
	{
		trace = G_Trace( neworg, mins, maxs, neworg, this, edict->clipmask, true, "SetupUseObject - 1" );
		if ( trace.startsolid || trace.allsolid )
		{
			trace = G_Trace( origin, mins, maxs, neworg, this, edict->clipmask, true, "SetupUseObject - 2" );
			if ( trace.startsolid || ( trace.fraction < 1.0f ) )
			{
				gi.WDPrintf( "Move to UseObject was blocked.\n" );
			}
		}
		
		if ( !trace.startsolid )
		{
			setOrigin( trace.endpos );
		}
		
		setAngles( newangles );
		v_angle.y = newangles.y;
		SetViewAngles( v_angle );
	}
	
	movecontrol = MOVECONTROL_ABSOLUTE;
	if ( state.length() )
	{
		State * newState;
		
		newState = statemap_Torso->FindState( state );
		if ( newState )
			EvaluateState( newState );
		else
			gi.WDPrintf( "Could not find state %s on UseObject\n", state.c_str() );
	}
	else
	{
		uo->Start();
	}
}

void Player::StartUseObject( Event * )
{
	UseObject  *uo;
	
	if ( !useitem_in_use )
		return;
	
	uo = ( UseObject * )( Entity * )useitem_in_use;
	uo->Start();
}

void Player::FinishUseObject( Event * )
{
	UseObject  *uo;
	
	if ( !useitem_in_use )
		return;
	
	uo = ( UseObject * )( Entity * )useitem_in_use;
	uo->Stop( this );
	useitem_in_use = NULL;
}

void Player::turnTowardsEntity( Event *ev )
{
	Entity *entity;
	Vector dir;
	Vector angles;
	Event *turnEvent;
	float currentYaw;
	float yawDiff;

	entity = ev->GetEntity( 1 );

	if ( !entity )
		return;

	// Get the angles to the entity

	dir = entity->origin - origin;
	dir.normalize();

	angles = dir.toAngles();

	// Get the yaw difference

	currentYaw = anglemod( v_angle[ YAW ] );

	yawDiff = angles[ YAW ] - currentYaw;

	yawDiff = AngleNormalize180( yawDiff );

	// Actually turn the appropriate amount
	
	turnEvent = new Event( EV_Player_Turn );
	turnEvent->AddFloat( yawDiff );
	ProcessEvent( turnEvent );
}

void Player::Turn( Event *ev )
{
	float yaw;
	float time;
	int numFrames;
	
	yaw = ev->GetFloat( 1 );

	if ( ev->NumArgs() > 1 )
		time = ev->GetFloat( 2 );
	else
		time = 0.5;

	numFrames = (int)(time * ( 1 / level.frametime ));
	time = numFrames * level.frametime;
	
	CancelEventsOfType( EV_Player_TurnUpdate );
	
	if ( time > 0 )
	{
		ev = new Event( EV_Player_TurnUpdate );
		ev->AddFloat( yaw / numFrames );
		ev->AddFloat( time );
		ProcessEvent( ev );
	}
	else
	{
		v_angle[ YAW ] += yaw;
		SetViewAngles( v_angle );
	}
}

void Player::TurnUpdate( Event *ev )
{
	float yaw;
	float timeleft;
	
	yaw = ev->GetFloat( 1 );
	timeleft = ev->GetFloat( 2 );

	// Remove some time

	timeleft -= 0.05f;

	// Turn the specified amount

	v_angle[ YAW ] += yaw;
	SetViewAngles( v_angle );

	// Repost the turn event if we have time remaining

	if ( timeleft > 0.0f )
	{
		ev = new Event( EV_Player_TurnUpdate );
		ev->AddFloat( yaw );
		ev->AddFloat( timeleft );
		PostEvent( ev, 0.05f );
	}
}

void Player::TurnLegs( Event *ev )
{
	float yaw;
	
	yaw = ev->GetFloat( 1 );
	
	angles[ YAW ] += yaw;
	setAngles( angles );
}

void Player::DontTurnLegs( Event *ev )
{
	dont_turn_legs = ev->GetBoolean( 1 );
}

void Player::EvaluateState( State *forceTorso, State *forceLegs )
{
	int count;
	State *laststate_Legs;
	State *laststate_Torso;
	State *startstate_Legs;
	State *startstate_Torso;
	movecontrol_t move;
	
	if ( getMoveType() == MOVETYPE_NOCLIP )
	{
		return;
	}
	
	// Evaluate the current state.
	// When the state changes, we reevaluate the state so that if the
	// conditions aren't met in the new state, we don't play one frame of
	// the animation for that state before going to the next state.
	startstate_Torso = laststate_Torso = currentState_Torso;
	count = 0;
	do
	{
		// since we could get into an infinite loop here, do a check
		// to make sure we don't.
		count++;
		if ( count > 10 )
		{
			gi.WDPrintf( "Possible infinite loop in state '%s'\n", currentState_Torso->getName() );
			assert( 0 );
			if ( count > 20 )
            {
				gi.Error( ERR_DROP, "Stopping due to possible infinite state loop\n" );
				break;
            }
		}
		
		laststate_Torso = currentState_Torso;
		
		if ( forceTorso )
			currentState_Torso = forceTorso;
		else
			currentState_Torso = currentState_Torso->Evaluate( *this, &torso_conditionals );
		
		animdone_Torso = false;
		if ( movecontrol != MOVECONTROL_LEGS )
		{
			animdone_Legs = false;
		}
		if ( laststate_Torso != currentState_Torso )
		{
			// Process exit commands of the last state
			laststate_Torso->ProcessExitCommands( this );
			
			// Process entry commands of the new state
			currentState_Torso->ProcessEntryCommands( this );
			
			if ( waitForState.length() && ( !waitForState.icmpn( currentState_Torso->getName(), waitForState.length() ) ) )
            {
				waitForState = "";
				PlayerDone( NULL );
            }
			
			move = currentState_Torso->getMoveType();
			
			str legsAnim( currentState_Torso->getLegAnim( *this, &torso_conditionals ) );
			if ( !animate->HasAnim(legsAnim) )
				legsAnim = getGameplayAnim(legsAnim);
			str torsoAnim( currentState_Torso->getTorsoAnim( *this, &torso_conditionals ) );
			if ( !animate->HasAnim(torsoAnim) )
				torsoAnim = getGameplayAnim(torsoAnim);
			
			if ( legsAnim != "" )
			{
				animdone_Legs = false;
				if ( !vehicle )
					SetAnim( legsAnim, legs );
			}
			else if ( move == MOVECONTROL_LEGS )
			{
				if ( !currentState_Legs )
				{
					animdone_Legs = false;
					currentState_Legs = statemap_Legs->FindState( "STAND" );
					legsAnim = currentState_Legs->getLegAnim( *this, &legs_conditionals );
					if ( !animate->HasAnim(legsAnim) )
						legsAnim = getGameplayAnim(legsAnim);
					if ( legsAnim == "" )
					{
						partAnim[ legs ] = "";
						animate->ClearLegsAnim();
					}
					else if ( legsAnim != "none" )
					{
						if ( !vehicle )
							SetAnim( legsAnim, legs );
					}
				}
			}
			else
			{
				partAnim[ legs ] = "";
				animate->ClearLegsAnim();
			}
			
			if ( torsoAnim == "" )
            {
				partAnim[ torso ] = "";
				animate->ClearTorsoAnim();
            }
			else if ( torsoAnim != "none" )
            {
				SetAnim( torsoAnim.c_str(), torso );
            }
			
			if ( movecontrol != move )
            {
				movecontrol = move;
				if ( ( move < ( sizeof( MoveStartFuncs ) / sizeof( MoveStartFuncs[ 0 ] ) ) ) && ( MoveStartFuncs[ move ] ) )
				{
					( this->*MoveStartFuncs[ move ] )();
				}
            }
			
			// This seems breaks the secret move mode because it's called at a bad time and causes the client/server angles
			// to get out of sync because delta_angles is not updated correctly.
			// I don't think it's necessary to call it anyway, so I'm removing it.
			//SetViewAngles( v_angle );
		}
	}
	while( laststate_Torso != currentState_Torso );
	  
	// Evaluate the current state.
	// When the state changes, we reevaluate the state so that if the
	// conditions aren't met in the new state, we don't play one frame of
	// the animation for that state before going to the next state.
	startstate_Legs = laststate_Legs = currentState_Legs;
	if ( movecontrol == MOVECONTROL_LEGS )
	{
		count = 0;
		do
		{
			// since we could get into an infinite loop here, do a check
			// to make sure we don't.
			count++;
			if ( count > 10 )
			{
				gi.WDPrintf( "Possible infinite loop in state '%s'\n", currentState_Legs->getName() );
				assert( 0 );
				if ( count > 20 )
				{
					gi.Error( ERR_DROP, "Stopping due to possible infinite state loop\n" );
					break;
				}
			}
			
			if ( !currentState_Legs )
			{
				currentState_Legs = statemap_Legs->FindState( "STAND" );
			}
			
			laststate_Legs = currentState_Legs;
			
			if ( forceLegs )
				currentState_Legs = forceLegs;
			else
				currentState_Legs = currentState_Legs->Evaluate( *this, &legs_conditionals );
			
			animdone_Legs = false;
			if ( laststate_Legs != currentState_Legs )
			{
				// Process exit commands of the last state
				laststate_Legs->ProcessExitCommands( this );
				
				// Process entry commands of the new state
				currentState_Legs->ProcessEntryCommands( this );
				
				if ( waitForState.length() && ( !waitForState.icmpn( currentState_Legs->getName(), waitForState.length() ) ) )
				{
					waitForState = "";
					PlayerDone( NULL );
				}
				
				str legsAnim( currentState_Legs->getLegAnim( *this, &legs_conditionals ) );
				if ( !animate->HasAnim(legsAnim) )
					legsAnim = getGameplayAnim(legsAnim);
				if ( legsAnim == "" )
				{
					partAnim[ legs ] = "";
					animate->ClearLegsAnim();
				}
				else if ( legsAnim != "none" )
				{
					if ( !vehicle )
						SetAnim( legsAnim, legs );
				}
			}
		}
		while( laststate_Legs != currentState_Legs );
	}
	else
	{
		currentState_Legs = NULL;
	}
	
	if ( g_showplayeranim->integer )
	{
		if ( last_leg_anim_name != animate->AnimName( legs ) )
		{
			gi.DPrintf( "Legs change from %s to %s\n", last_leg_anim_name.c_str(), animate->AnimName( legs ) );
			last_leg_anim_name = animate->AnimName( legs );
		}
		
		if ( last_torso_anim_name != animate->AnimName( torso ) )
		{
			gi.DPrintf( "Torso change from %s to %s\n", last_torso_anim_name.c_str(), animate->AnimName( torso ) );
			last_torso_anim_name = animate->AnimName( torso );
		}
	}
	
	if ( g_showplayerstate->integer )
	{
		if ( startstate_Legs != currentState_Legs )
		{
			gi.DPrintf( "Legs change from %s to %s\n",
				startstate_Legs ? startstate_Legs->getName() : "NULL",
				currentState_Legs ? currentState_Legs->getName() : "NULL" );
		}
		
		if ( startstate_Torso != currentState_Torso )
		{
			gi.DPrintf( "Torso change from %s to %s\n",
				startstate_Torso ? startstate_Torso->getName() : "NULL",
				currentState_Torso ? currentState_Torso->getName() : "NULL" );
		}
	}
	
	if ( g_showplayerweapon->integer )
	{
		Weapon *weapon;
		
		weapon = GetActiveWeapon( WEAPON_DUAL );
		
		if ( !weapon )
			weapon = GetActiveWeapon( WEAPON_RIGHT );
		
		if ( !weapon )
			weapon = GetActiveWeapon( WEAPON_LEFT );
		
		if ( weapon )
		{
			gi.DPrintf( "Weapon - anim %s, frame %d\n", weapon->animate->GetName().c_str(), weapon->animate->CurrentFrame() );
		}
	}
	
	// This is so we don't remember pain when we change to a state that has a PAIN condition
	pain = 0;
	// Every second drop accumulated pain by 1
	if ( ( float )( int )( level.time ) == level.time )
	{
		accumulated_pain -= 1.0f;
		if ( accumulated_pain < 0.0f )
			accumulated_pain = 0.0f;
	}
}

void Player::EventUseItem( Event *ev )
{
	const char  *name;
	weaponhand_t hand = WEAPON_DUAL;
	
	if ( deadflag || _disableUseWeapon )
	{
		return;
	}

	if ( multiplayerManager.inMultiplayer() && ( !multiplayerManager.isFightingAllowed() || multiplayerManager.isPlayerSpectator( this ) ) )
	{
		return;
	}
	
	name = ev->GetString( 1 );
	
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	
	str objectName("CurrentPlayer.");
	objectName += name ;
	if ( gpm->hasProperty( objectName, "name" ) )
		name = gpm->getStringValue( objectName, "name" );
	else if ( gpm->hasProperty(name, "consumable" ) )
	{
		str slotname = gpm->getStringValue(name, "invslot");
		if ( !gpm->hasProperty(slotname, "quantity") )
			return;
		
		float quantity = gpm->getFloatValue(slotname, "quantity");
		if ( quantity < 1.0f )
			return;
		
		// Give the model, decrement the quantity
		str model = gpm->getStringValue(name, "model");
		giveItem(model);
		gpm->setFloatValue(slotname, "quantity", quantity-1.0f);
	}
	
	Item *item = ( Item * )FindItem( name );
	
	if ( item && item->isSubclassOf( InventoryItem ) )
	{
		InventoryItem *ii = ( InventoryItem * )item;
		Event *ev1;
		
		ev1 = new Event( EV_InventoryItem_Use );
		ev1->AddEntity( this );
		ii->ProcessEvent( ev1 );
		return;
	}
	else if ( ev->NumArgs() > 1 )
	{
		hand = WeaponHandNameToNum( ev->GetString( 2 ) );
	}
	
	useWeapon( name, hand );	
}

void Player::GiveWeaponCheat( Event *ev )
{
	giveItem( ev->GetString( 1 ) );
}

void Player::GiveCheat( Event *ev )
{
	str name;
	
	if ( deadflag )
	{
		return;
	}
	
	name = ev->GetString( 1 );
	
	if ( !name.icmp( "all" ) )
	{
		GiveAllCheat( ev );
		return;
	}
	EventGiveItem( ev );
}

void Player::GiveAllCheat( Event *ev )
{
	char  *buffer;
	char  *buf;
	char	com_token[MAX_STRING_CHARS];
	int numEvents;
	float postTime;


	if ( deadflag )
	{
		return;
	}

	if ( !gi.isClientActive( this->edict ) )
	{
		PostEvent( *ev, FRAMETIME );
		return;
	}

	numEvents = 0;
	postTime = 0.0f;

	if ( gi.FS_ReadFile( "global/giveall.scr", ( void ** )&buf, true ) != -1 )
	{
		buffer = buf;

		while ( 1 )
		{
			strcpy( com_token, COM_ParseExt( &buffer, true ) );

			if (!com_token[0])
				break;

			// Create the event
			ev = new Event( com_token );

			// get the rest of the line
			while( 1 )
			{
				strcpy( com_token, COM_ParseExt( &buffer, false ) );
				if (!com_token[0])
					break;

				ev->AddToken( com_token );
			}

			if ( numEvents >= 5 )
			{
				numEvents = 0;
				//postTime += level.frametime * 4;
				postTime += 1.0f;
			}

			PostEvent( ev, postTime );

			numEvents++;
		}

		gi.FS_FreeFile( buf );
	}
}

void Player::GodCheat( Event *ev )
{
	const char *msg;
	
	if ( ev->NumArgs() > 0 )
	{
		if ( ev->GetInteger( 1 ) )
		{
			flags |= FL_GODMODE;
		}
		else
		{
			flags &= ~FL_GODMODE;
		}
	}
	else
	{
		flags ^= FL_GODMODE;
	}
	
	if ( ev->GetSource() == EV_FROM_CONSOLE )
	{
		if ( !( flags & FL_GODMODE ) )
		{
			msg = "godmode OFF\n";
		}
		else
		{
			msg = "godmode ON\n";
		}
		
		gi.SendServerCommand( edict-g_entities, "print \"%s\"", msg );
	}
}

void Player::Kill( Event * )
{
	if ( ( level.time - respawn_time ) < 5.0f )
	{
		return;
	}
	
	flags &= ~FL_GODMODE;
	health = 1.0f;
	Damage( this, this, 10.0f, origin, vec_zero, vec_zero, 0, DAMAGE_NO_PROTECTION, MOD_SUICIDE );
}

void Player::NoTargetCheat( Event * )
{
	const char *msg;
	
	flags ^= FL_NOTARGET;
	if ( !( flags & FL_NOTARGET ) )
	{
		msg = "notarget OFF\n";
	}
	else
	{
		msg = "notarget ON\n";
	}
	
	gi.SendServerCommand( edict-g_entities, "print \"%s\"", msg );
}

//--------------------------------------------------------------
//
// Name:			NoclipCheat
// Class:			Player
//
// Description:		This is the event that puts the player in noclip mode
//
// Parameters:		Event *ev - No params
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::NoclipCheat( Event *	)
{
	const char *msg;
	
	if ( vehicle )
		msg = "Must exit vehicle first\n";
	else if ( getMoveType() == MOVETYPE_NOCLIP )
	{
		setMoveType( MOVETYPE_WALK );
		msg = "noclip OFF\n";
		
		// reset the state machine so that her animations are correct
		SetState("STAND", "STAND");
	}
	else
	{
		client->ps.feetfalling = false;
		movecontrol = MOVECONTROL_USER;
		
		setMoveType( MOVETYPE_NOCLIP );
		msg = "noclip ON\n";
	}
	
	gi.SendServerCommand( edict-g_entities, "print \"%s\"", msg );
}

void Player::GameVersion( Event * )
{
	gi.SendServerCommand( edict-g_entities, "print \"%s : %s\n\"", GAME_NAME, __DATE__ );
}

void Player::SetFov( float newFov, bool forced )
{
	fov = newFov;
	
	if ( !forced && ( fov != sv_defaultFov->value ) && multiplayerManager.checkFlag( MP_FLAG_FIXED_FOV ) )
	{
		fov = sv_defaultFov->value;
		return;
	}

	//if ( !forced )
	//{
	//	_userFovChoice = newFov;
	//}
	
	if ( fov < 1.0f )
	{
		fov = sv_defaultFov->value;
	}
	else if ( fov > 160.0f )
	{
		fov = 160.0f;
	}

	str tempString( fov );
	gi.cvar_set( "r_fov", tempString.c_str() );
}

float Player::getDefaultFov( void )
{
	if ( multiplayerManager.inMultiplayer() && multiplayerManager.checkFlag( MP_FLAG_FIXED_FOV ) )
		return sv_defaultFov->value;
	else
		//return _userFovChoice;
		return userFov;
}

void Player::Fov( Event *ev )
{
	CancelEventsOfType(EV_Player_Fov);
	if ( ev->NumArgs() > 1 )
	{
		Event *event = new Event( EV_Player_Fov );
		event->AddFloat( ev->GetFloat( 1 ) );
		PostEvent ( event, ev->GetFloat( 2 ) );
		return;
	}
	
	if ( ev->NumArgs() < 1 )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Fov = %d\n\"", fov );
		return;
	}
	
	SetFov( ev->GetFloat( 1 ) );
}

/*
===============
CalcRoll

===============
*/
float Player::CalcRoll( void )
{
	float	sign;
	float	side;
	float	value;
	Vector l;
	
	angles.AngleVectors( NULL, &l, NULL );
	side = velocity * l;
	sign = side < 0.0f ? 4.0f : -4.0f;
	side = fabs( side );
	
	value = sv_rollangle->value;
	
	if ( side < sv_rollspeed->value )
	{
		side = side * value / sv_rollspeed->value;
	}
	else
	{
		side = value;
	}
	
	return side * sign;
}

void Player::GravityNodes( void )
{
	Vector grav;
	Vector gravnorm;
	Vector velnorm;
	float  dot;
	qboolean force;
	float max_speed;
	Vector new_velocity;
	
	//
	// Check for gravity pulling points
	//
	
	// no pull during waterjumps
	if ( client->ps.pm_flags & PMF_TIME_WATERJUMP )
	{
		return;
	}
	
	grav = gravPathManager.CalculateGravityPull( *this, origin, &force, &max_speed );
	
	// Check for unfightable gravity.
	if ( force && ( grav != vec_zero ) )
	{
		velnorm = velocity;
		velnorm.normalize();
		
		gravnorm = grav;
		gravnorm.normalize();
		
		dot = ( gravnorm.x * velnorm.x ) + ( gravnorm.y * velnorm.y ) + ( gravnorm.z * velnorm.z );
		
		// This prevents the player from trying to swim upstream
		if ( dot < 0.0f )
		{
			float tempdot;
			Vector tempvec;
			
			tempdot = 0.2f - dot;
			tempvec = velocity * tempdot;
			velocity = velocity - tempvec;
		}
	}
	
	if ( grav != vec_zero )
	{
		new_velocity = velocity + grav;
		
		if ( new_velocity.length() < velocity.length() )
		{
			// Is slowing down, defintely need to apply grav
			velocity = new_velocity;
		}
		else if ( velocity.length() < max_speed )
		{
			// Applay grav
			
			velocity = new_velocity;
			
			// Make sure we aren't making the player go too fast
			
			if ( velocity.length() > max_speed )
			{
				velocity.normalize();
				velocity *= max_speed;
			}
		}
		else
		{
			// Going too fast but still want to pull the player up if any z velocity in grav
			
			grav.x = 0;
			grav.y = 0;
			
			velocity = velocity + grav;
		}
	}
}

//
// PMove Events
//
void Player::ProcessPmoveEvents( int event )
{
	float damage;
	
	switch( event )
	{
	case EV_NONE:
		break;
	case EV_FALL_VERY_SHORT:
	case EV_FALL_SHORT:
	case EV_FALL_MEDIUM:
	case EV_FALL_FAR:
	case EV_FALL_VERY_FAR:
	case EV_FALL_FATAL:
		damage = 0.0f;
		if ( event == EV_FALL_VERY_SHORT )
			damage = 5.0f;
		else if ( event == EV_FALL_SHORT )
			damage = 10.0f;
		else if ( event == EV_FALL_MEDIUM )
			damage = 25.0f;
		else if ( event == EV_FALL_FAR )
			damage = 50.0f;
		else if ( event == EV_FALL_VERY_FAR )
			damage = 100.0f;
		else if ( event == EV_FALL_FATAL )
			damage = 1000.0f;
		
		if ( !multiplayerManager.checkFlag( MP_FLAG_NO_FALLING ) )
		{
            Damage( world, world, damage, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_FALLING );
		}
		break;
	case EV_TERMINAL_VELOCITY:
		Sound( "snd_fall", CHAN_VOICE );
		break;
		
	case EV_WATER_TOUCH:   // foot touches
		if ( watertype & CONTENTS_LAVA )
		{
			Sound( "snd_burn", CHAN_LOCAL );
		}
		else
		{
			Entity *water;
			trace_t trace;
			Vector start;
			float scale;
			
			Sound( "impact_playersplash", CHAN_AUTO );
			
			// Find the correct place to put the splash
			
			start = origin + Vector( 0.0f, 0.0f, 90.0f );
			trace = G_Trace( start, vec_zero, vec_zero, origin, NULL, MASK_WATER, false, "ProcessPmoveEvents" );
			
			// Figure out a good scale for the splash
			
			scale = 1.0f + ( velocity[2] + 400.0f ) / -1500.0f;
			
			if ( scale < 1.0f )
				scale = 1.0f;
			else if ( scale > 1.5f )
				scale = 1.5f;
			
			// Spawn in a water splash
			
			water = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
			
			water->setOrigin( trace.endpos );
			water->setModel( "fx_splashsmall.tik" );
			water->setScale( scale );
			water->animate->RandomAnimate( "idle" );
			water->PostEvent( EV_Remove, 5.0f );
			
		}
		break;
	case EV_WATER_LEAVE:   // foot leaves
		Sound( "impact_playerleavewater", CHAN_AUTO );
		break;
	case EV_WATER_UNDER:   // head touches
		Sound( "impact_playersubmerge", CHAN_AUTO );
		break;
	case EV_WATER_CLEAR:   // head leaves
		Sound( "snd_gasp", CHAN_LOCAL );
		break;
	}
}

/*
=============
WorldEffects
=============
*/
void Player::WorldEffects( void )
{
	if ( deadflag == DEAD_DEAD )
	{
		// if we are dead, no world effects
		return;
	}
	
	if ( movetype == MOVETYPE_NOCLIP )
	{
		// don't need air
		air_finished = level.time + 20.0f;
		return;
	}
	
	//
	// check for on fire
	//
	if ( on_fire )
	{
		if ( next_painsound_time < level.time )
		{
			next_painsound_time = level.time + 4.0f;
			Sound( "snd_onfire", CHAN_LOCAL );
		}
	}
	
	//
	// check for lava
	//
	if ( watertype & CONTENTS_LAVA )
	{
		if ( next_drown_time < level.time )
		{
			next_drown_time = level.time + 0.2f;
			Damage( world, world, 10.0f * waterlevel, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_LAVA );
		}
		if ( next_painsound_time < level.time )
		{
			next_painsound_time = level.time + 3.0f;
			Sound( "snd_burned", CHAN_LOCAL );
		}
	}
	
	//
	// check for slime
	//
	if ( watertype & CONTENTS_SLIME )
	{
		if ( next_drown_time < level.time )
		{
			next_drown_time = level.time + 0.4f;
			Damage( world, world, 7.0f * waterlevel, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_SLIME );
		}
		if ( next_painsound_time < level.time )
		{
			next_painsound_time = level.time + 5.0f;
			Sound( "snd_burned", CHAN_LOCAL );
		}
	}
	
	//
	// check for drowning
	//
	if ( waterlevel == 3 )
	{
		// if out of air, start drowning
		if ( ( air_finished < level.time ) && !( flags & FL_GODMODE ) )
		{
			// drown!
			if ( ( next_drown_time < level.time ) && ( health > 0 ) )
			{
				next_drown_time = level.time + 1.0f;
				
				// take more damage the longer underwater
				drown_damage += 2.0f;
				if ( drown_damage  > 15.0f )
				{
					drown_damage = 15.0f;
				}
				
				// play a gurp sound instead of a normal pain sound
				if ( health <= drown_damage )
				{
					Sound( "snd_drown", CHAN_LOCAL );
					BroadcastSound();
				}
				else if ( rand() & 1 )
				{
					Sound( "snd_choke", CHAN_LOCAL );
					BroadcastSound();
				}
				else
				{
					Sound( "snd_choke", CHAN_LOCAL );
					BroadcastSound();
				}
				
				Damage( world, world, drown_damage, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_DROWN );
			}
		}
	}
	else
	{
		air_finished = level.time + 20.0f;
		drown_damage = 2.0f;
	}
	
	GravityNodes();
	
	old_waterlevel = waterlevel;
}

/*
=============
AddBlend
=============
*/
void Player::AddBlend( float r, float g, float b, float a )
{
	float	a2;
	float a3;
	
	if ( a <= 0.0f )
	{
		return;
	}
	
	// new total alpha
	a2 = blend[ 3 ] + ( 1.0f - blend[ 3 ] ) * a;
	
	// fraction of color from old
	a3 = blend[ 3 ] / a2;
	
	blend[ 0 ] = ( blend[ 0 ] * a3 ) + ( r * ( 1.0f - a3 ) );
	blend[ 1 ] = ( blend[ 1 ] * a3 ) + ( g * ( 1.0f - a3 ) );
	blend[ 2 ] = ( blend[ 2 ] * a3 ) + ( b * ( 1.0f - a3 ) );
	blend[ 3 ] = a2;
}

/*
=============
SetBlend
=============
*/
void Player::SetBlend( float r, float g, float b, float a, int additive )
{
	client->ps.blend[ 0 ] = r;
	client->ps.blend[ 1 ] = g;
	client->ps.blend[ 2 ] = b;
	client->ps.blend[ 3 ] = a;
	
	client->ps.stats[ STAT_ADDFADE ] = additive;
}

void Player::SetBlend( int additive )
{
	SetBlend( blend[ 0 ], blend[ 1 ], blend[ 2 ], blend[ 3], additive );
}

/*
=============
CalcBlend
=============
*/
void Player::CalcBlend( void )
{
	int		contents;
	Vector	vieworg;
	Vector	viewModeColor;
	float		viewModeAlpha;
	qboolean	viewModeAdditive;
	
	client->ps.stats[STAT_ADDFADE] = 0;
	blend[ 0 ] = blend[ 1 ] = blend[ 2 ] = blend[ 3 ] = 0;
	SetBlend( false );
	
	// Set to view mode blend if necessary
	
	if ( gi.GetViewModeScreenBlend( getViewMode(), viewModeColor, &viewModeAlpha, &viewModeAdditive ) )
	{
		AddBlend( viewModeColor[ 0 ], viewModeColor[ 1 ], viewModeColor[ 2 ], viewModeAlpha );
		SetBlend( viewModeAdditive );
		
		return;
	}
	
	// add for contents
	vieworg = origin;
	vieworg[ 2 ] += viewheight;	
	
	contents = gi.pointcontents( vieworg, 0 );
	
	if ( flags & FL_STUNNED )
	{
		AddBlend( 0.0f, 0.0f, 0.5f, 0.1f );
		SetBlend( true );
	}
	else if ( contents & CONTENTS_SOLID )
	{
		// Outside of world
		//AddBlend( 0.8, 0.5, 0.0, 0.2 );
	}
	else if ( contents & CONTENTS_LAVA )
	{
		AddBlend( level.lava_color[0], level.lava_color[1], level.lava_color[2], level.lava_alpha );
		SetBlend( true );
	}
	else if ( contents & CONTENTS_WATER )
	{
		AddBlend( level.water_color[0], level.water_color[1], level.water_color[2], level.water_alpha );
		SetBlend( true );
	}
	else if ( contents & CONTENTS_SLIME )
	{
		AddBlend( level.slime_color[0], level.slime_color[1], level.slime_color[2], level.slime_alpha );
		SetBlend( true );
	}
	
	// add for damage
	if ( _doDamageScreenFlash && ( damage_alpha > 0 ) )
	{
		AddBlend( damage_blend[ 0 ], damage_blend[ 1 ], damage_blend[ 2 ], damage_alpha );
		SetBlend( false );
		
		// drop the damage value
		/* damage_alpha -= 0.06;
		
		if ( damage_alpha < 0 )
		{
			damage_alpha = 0;
		} */
	}
	
	// Add screen flash
	
	if ( level.time < _flashMaxTime )
	{
		float currentFlashAlpha;
		
		currentFlashAlpha = _flashAlpha;
		
		if ( level.time > _flashMinTime )
		{
			currentFlashAlpha = _flashAlpha * ( _flashMaxTime - level.time ) / ( _flashMaxTime - _flashMinTime );
		}
		
		AddBlend( _flashBlend[ 0 ], _flashBlend[ 1 ], _flashBlend[ 2 ], currentFlashAlpha );
		SetBlend( false );
	}
	
	
	// Do the cinematic fading
	float alpha=1;
	
	level.m_fade_time -= level.frametime;
	
	// Return if we are completely faded in
	if ( ( level.m_fade_time <= 0.0f ) && ( level.m_fade_type == fadein ) )
	{
		//client->ps.blend[3] = 0 + damage_alpha;
		return;
	}
	
	// If we are faded out, and another fade out is coming in, then don't bother
	/* if ( ( level.m_fade_time_start > 0 ) && ( level.m_fade_type == fadeout ) )
	{
	if ( client->ps.blend[3] >= 1 )
	return;
} */
	
	if ( level.m_fade_time_start > 0.0f )
		alpha = level.m_fade_time / level.m_fade_time_start;
	
	if ( level.m_fade_type == fadeout )
		alpha = 1.0f - alpha;
	
	if ( alpha < 0.0f )
		alpha = 0.0f;
	
	if ( alpha > 1.0f )
		alpha = 1.0f;
	
	if ( level.m_fade_style == additive )
	{
		if ( client->ps.stats[STAT_ADDFADE] == 1 )
		{
			AddBlend( level.m_fade_color[0] * level.m_fade_alpha * alpha, level.m_fade_color[1] * level.m_fade_alpha * alpha, 
				level.m_fade_color[2] * level.m_fade_alpha * alpha, level.m_fade_alpha * alpha );
			SetBlend( true );
		}
		else
		{
			SetBlend( level.m_fade_color[0] * level.m_fade_alpha * alpha, level.m_fade_color[1] * level.m_fade_alpha * alpha, 
				level.m_fade_color[2] * level.m_fade_alpha * alpha, level.m_fade_alpha * alpha, true );
		}
	}
	else
	{
		SetBlend( level.m_fade_color[0], level.m_fade_color[1], level.m_fade_color[2], level.m_fade_alpha * alpha, false );
	}
}

void Player::setFlash( const Vector &color, float alpha, float minTime, float maxTime )
{
	_flashBlend = color;
	_flashAlpha = alpha;
	_flashMinTime = level.time + minTime;
	_flashMaxTime = level.time + maxTime;
}

/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/

#define DAMAGE_MAX_PITCH_SCALE 0.3f
#define DAMAGE_MAX_YAW_SCALE 0.3f

void Player::DamageFeedback( void )
{
	if ( _lastDamagedTimeFront + 0.25f >= level.time )
		client->ps.pm_flags |= PMF_DAMAGE_FRONT;
	else
		client->ps.pm_flags &= ~PMF_DAMAGE_FRONT;

	if ( _lastDamagedTimeBack + 0.25f >= level.time )
		client->ps.pm_flags |= PMF_DAMAGE_BACK;
	else
		client->ps.pm_flags &= ~PMF_DAMAGE_BACK;

	if ( _lastDamagedTimeLeft + 0.25f >= level.time )
		client->ps.pm_flags |= PMF_DAMAGE_LEFT;
	else
		client->ps.pm_flags &= ~PMF_DAMAGE_LEFT;

	if ( _lastDamagedTimeRight + 0.25f >= level.time )
		client->ps.pm_flags |= PMF_DAMAGE_RIGHT;
	else
		client->ps.pm_flags &= ~PMF_DAMAGE_RIGHT;

	// if we are dead, don't setup any feedback

	if ( health <= 0.0f )
	{
		damage_count = 0;
		damage_blood = 0;
		damage_alpha = 0;
		VectorClear( damage_angles );
		return;
	}
	
	if ( damage_blood > damage_count )
	{
		Vector delta;
		Vector damageDir;
		
		//damageDir = damage_from * -1.0f;
		damageDir = damage_from;

		delta = damageDir.toAngles();

		delta[ PITCH ] = AngleDelta( delta[ PITCH ], v_angle[ PITCH ] );
		delta[ YAW ]   = AngleDelta( delta[ YAW ], v_angle[ YAW ] );
		delta[ ROLL ]  = AngleDelta( delta[ ROLL ], v_angle[ ROLL ] );

		// Dim it down by a factor of a hundred so we can multiple it by our built up damage later

		delta *= 0.01f;
		
		damage_angles = delta;
	}
	
	if ( damage_count > 0 )
	{
		// decay damage_count over time

		if ( damage_count * 0.1f < 0.25f )
		{
			damage_count -= 0.25f;
		}
		else
		{
			damage_count *= 0.9f;
		}

		if ( damage_count < 0.1f )
		{
			damage_count = 0;
		}
	}

	damage_count += damage_blood;

	if ( damage_count <= 0.0f )
	{
		damage_alpha = 0.0f;
		return;
	}

	// the total alpha of the blend is always proportional to count
	
	damage_alpha = damage_count / 25.0f;

	// Make sure the alpha is within a reasonable range

	if ( damage_alpha < 0.1f )
	{
		damage_alpha = 0.1f;
	}
	else if ( damage_alpha > 0.6f )
	{
		damage_alpha = 0.6f;
	}

	//damage_blend = bcolor;
	
	// clear totals

	damage_blood = 0;
}

void Player::GetPlayerView( Vector *pos, Vector *angle )
{
	if ( pos )
	{
		*pos = origin;
		pos->z += viewheight;
	}
	
	if ( angle )
	{
		*angle = Vector( client->ps.viewangles );
	}
}

#define EARTHQUAKE_SCREENSHAKE_PITCH 2
#define EARTHQUAKE_SCREENSHAKE_YAW 2
#define EARTHQUAKE_SCREENSHAKE_ROLL 3

void Player::SetClientViewAngles( const Vector &position, const float cameraoffset, const Vector &ang, const Vector &vel, const float camerafov ) const
{
	if ( client->ps.useCameraFocalPoint )
	{
		client->ps.cameraFocalPoint[ 0 ] = position[ 0 ];
		client->ps.cameraFocalPoint[ 1 ] = position[ 1 ];
		client->ps.cameraFocalPoint[ 2 ] = position[ 2 ] + cameraoffset + 18.0f;
	}

	client->ps.origin[ 0 ] = position[ 0 ];
	client->ps.origin[ 1 ] = position[ 1 ];
	client->ps.origin[ 2 ] = position[ 2 ];
	
	client->ps.viewheight = (int) cameraoffset;

	client->ps.viewangles[ 0 ] = ang[ 0 ];
	client->ps.viewangles[ 1 ] = ang[ 1 ];
	client->ps.viewangles[ 2 ] = ang[ 2 ];
	
	client->ps.velocity[ 0 ] = vel[ 0 ];
	client->ps.velocity[ 1 ] = vel[ 1 ];
	client->ps.velocity[ 2 ] = vel[ 2 ];
	
	client->ps.fov = camerafov;
}

void Player::ShakeCamera( void )
{
	// Shake the player's view
	
	VectorClear( client->ps.damage_angles );

	// Don't shake camera if in cinematic and not allowed to

	if ( ( playerCameraMode == PLAYER_CAMERA_MODE_CINEMATIC ) && !world->canShakeCamera() )
		return;
	
	// Add earthquake shaking
	
	float earthquakeMagnitude = level.getEarthquakeMagnitudeAtPosition( origin );

	if ( earthquakeMagnitude > 0.0f )
	{
		client->ps.damage_angles[ PITCH ] += G_CRandom() * earthquakeMagnitude * EARTHQUAKE_SCREENSHAKE_PITCH;
		client->ps.damage_angles[ YAW ]   += G_CRandom() * earthquakeMagnitude * EARTHQUAKE_SCREENSHAKE_YAW;
		client->ps.damage_angles[ ROLL ]  += G_CRandom() * earthquakeMagnitude * EARTHQUAKE_SCREENSHAKE_ROLL;
	}

	// Don't do anything else to the camera if in a cinematic

	if ( playerCameraMode == PLAYER_CAMERA_MODE_CINEMATIC )
		return;

	VectorAdd( client->ps.damage_angles, getWeaponViewShake(), client->ps.damage_angles );
	
	// Add damage shaking
	
	if ( damage_count && sv_showdamageshake->integer )
	{
		if ( health <= 0.0f )
		{
			damage_angles = vec_zero;
		}
		else if ( multiplayerManager.inMultiplayer() )
		{
			damage_angles[ YAW ]   = G_CRandom( 0.05f );
			damage_angles[ PITCH ] = G_CRandom( 0.05f );
			damage_angles[ ROLL ]  = G_CRandom( 0.05f );
		}
		else
		{
			damage_angles[ YAW ]   = G_CRandom( 0.2f );
			damage_angles[ PITCH ] = G_CRandom( 0.2f );
			damage_angles[ ROLL ]  = G_CRandom( 0.2f );
		}

		client->ps.damage_angles[ YAW ]   += damage_angles[ YAW ]   * damage_count;
		client->ps.damage_angles[ PITCH ] += damage_angles[ PITCH ] * damage_count;
		client->ps.damage_angles[ ROLL ]  += damage_angles[ ROLL ]  * damage_count;
	}

	if ( client->pers.mp_lowBandwidth )
	{
		client->ps.damage_angles[ YAW ]   = 0.0f;
		client->ps.damage_angles[ PITCH ] = 0.0f;
		client->ps.damage_angles[ ROLL ]  = 0.0f;
	}
}


void Player::SetPlayerViewUsingNoClipController( void )
{
	client->ps.pm_flags &= ~PMF_CAMERA_VIEW;
	
	// Force camera to behind in NOCLIP
	client->ps.camera_flags = client->ps.camera_flags & CF_CAMERA_CUT_BIT;
}

void Player::SetPlayerViewUsingCinematicController( Camera *camera )
{
	client->ps.camera_angles[ 0 ] = camera->angles[ 0 ];
	client->ps.camera_angles[ 1 ] = camera->angles[ 1 ];
	client->ps.camera_angles[ 2 ] = camera->angles[ 2 ];
	
	client->ps.camera_origin[ 0 ] = camera->origin[ 0 ];
	client->ps.camera_origin[ 1 ] = camera->origin[ 1 ];
	client->ps.camera_origin[ 2 ] = camera->origin[ 2 ];
	client->ps.pm_flags |= PMF_CAMERA_VIEW;
	
	//
	// clear out the flags, but preserve the CF_CAMERA_CUT_BIT
	//
	client->ps.camera_flags = client->ps.camera_flags & CF_CAMERA_CUT_BIT;

	ShakeCamera();
}


void Player::SetPlayerViewUsingActorController( Camera *camera )
{
	if ( actor_camera )
	{
		// Find the focal point ( either the actor's watch offset or top of the bounding box)
		
		// Go a little above the view height
		
		actor_camera->origin = origin;
		actor_camera->origin[2] += client->ps.pm_defaultviewheight + 10.f;
		
		// Shift the camera back just a little
		
		Vector forward;
		Vector left;
		angles.AngleVectors( &forward, &left );
		actor_camera->origin -= forward * 15.0f;
		
		// Shift the camera a little to the left or right
		actor_camera->origin -= left * 15.0f;
		
		// Set the camera's position
		
		actor_camera->setOrigin( actor_camera->origin );
		
		// Set the camera's angles
		
		actor_camera->setAngles( angles );
		
		// Set this as our camera
		
		SetCamera( actor_camera, 0.5f );

	}
	else if ( ( level.automatic_cameras.NumObjects() > 0 ) && ( !camera || camera->IsAutomatic() ) )
	{
		// if we currently are not in a camera or the camera we are looking through is automatic, evaluate our camera choices
		AutomaticallySelectedNewCamera();
	}

	client->ps.camera_angles[ 0 ] = camera->angles[ 0 ];
	client->ps.camera_angles[ 1 ] = camera->angles[ 1 ];
	client->ps.camera_angles[ 2 ] = camera->angles[ 2 ];
	
	client->ps.camera_origin[ 0 ] = camera->origin[ 0 ];
	client->ps.camera_origin[ 1 ] = camera->origin[ 1 ];
	client->ps.camera_origin[ 2 ] = camera->origin[ 2 ];
	client->ps.pm_flags |= PMF_CAMERA_VIEW;
	
	//
	// clear out the flags, but preserve the CF_CAMERA_CUT_BIT
	//
	client->ps.camera_flags = client->ps.camera_flags & CF_CAMERA_CUT_BIT;
}

void Player::SetPlayerViewUsingEntityWatchingController( void )
{

	SetPlayerViewNormal();

	// Find the focal point ( either the actor's watch offset or top of the bounding box)
	
	Vector focal_point;
	if ( entity_to_watch->watch_offset != vec_zero )
	{
		MatrixTransformVector( entity_to_watch->watch_offset, entity_to_watch->orientation, focal_point );
		focal_point += entity_to_watch->origin;
		focal_point[2] += client->ps.pm_defaultviewheight + 18.0f;
	}
	else
	{
		focal_point = entity_to_watch->origin;
		focal_point[2] += client->ps.pm_defaultviewheight + 18.0f;
	}

	if ( client->ps.useCameraFocalPoint )
	{
		client->ps.cameraFocalPoint[ 0 ] = focal_point[ 0 ];
		client->ps.cameraFocalPoint[ 1 ] = focal_point[ 1 ];
		client->ps.cameraFocalPoint[ 2 ] = focal_point[ 2 ];
	}
	
}

void Player::SetPlayerViewNormal( void )
{
	client->ps.pm_flags &= ~PMF_CAMERA_VIEW;
	
	//
	// make sure the third person camera is setup correctly.
	//
	
	int camera_type = currentState_Torso->getCameraType();
	if ( last_camera_type != camera_type )
    {
		//
		// clear out the flags, but preserve the CF_CAMERA_CUT_BIT
		//
		client->ps.camera_flags = client->ps.camera_flags & CF_CAMERA_CUT_BIT;
		bool do_cut = false;
		switch( camera_type )
		{
		case CAMERA_TOPDOWN:
			client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
			client->ps.camera_offset[ PITCH ] = -75;
			client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
			break;
		case CAMERA_FRONT:
			client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
			client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
			client->ps.camera_offset[ YAW ] = 180;
			client->ps.camera_offset[ PITCH ] = 0;
			do_cut = true;
			break;
		case CAMERA_SIDE:
			client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
			client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
			// randomly invert the YAW
			if ( G_Random( 1.0f ) > 0.5f )
			{
				client->ps.camera_offset[ YAW ] = -90;
			}
			else
			{
				client->ps.camera_offset[ YAW ] = 90;
			}
			client->ps.camera_offset[ PITCH ] = 0;
			break;
		case CAMERA_SIDE_LEFT:
			client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
			client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
			client->ps.camera_offset[ YAW ] = 90;
			client->ps.camera_offset[ PITCH ] = 0;
			break;
		case CAMERA_SIDE_RIGHT:
			client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
			client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
			client->ps.camera_offset[ YAW ] = -90;
			client->ps.camera_offset[ PITCH ] = 0;
			break;
		case CAMERA_BEHIND_FIXED:
			client->ps.camera_offset[ YAW ] = 0;
			client->ps.camera_offset[ PITCH ] = 0;
			client->ps.camera_flags |= CF_CAMERA_ANGLES_ALLOWOFFSET;
			break;
		case CAMERA_BEHIND_NOPITCH:
			client->ps.camera_flags |= CF_CAMERA_ANGLES_IGNORE_PITCH;
			client->ps.camera_offset[ YAW ] = 0;
			client->ps.camera_offset[ PITCH ] = 0;
			break;
		case CAMERA_BEHIND:
			client->ps.camera_offset[ YAW ] = 0;
			client->ps.camera_offset[ PITCH ] = 0;
			break;
		default:
			client->ps.camera_offset[ YAW ] = 0;
			client->ps.camera_offset[ PITCH ] = 0;
			break;
		}
		last_camera_type = camera_type;
		if ( do_cut )
			CameraCut();
	}
	
	// Add weapon shaking
	ShakeCamera();

}



void Player::SetupView(	void )
{
	// Check for change of state
	// These switch statements are indicative that we need a polymorphic set of classes based off a base class called PlayerCameraController
	switch( playerCameraMode )
	{
	case PLAYER_CAMERA_MODE_NORMAL:
	case PLAYER_CAMERA_MODE_CINEMATIC:
		break;

	case PLAYER_CAMERA_MODE_ACTOR:
		{
			if ( !entity_to_watch  || entity_to_watch->deadflag )
			{
				DestroyActorCamera();
			}
			else
			{
				trace_t trace = G_Trace( actor_camera->origin, Vector(-5, -5, -5), Vector(5, 5, 5), actor_camera->origin, actor_camera, MASK_DEADSOLID, false, "SetupView" );
				if ( trace.startsolid )
				{
					DestroyActorCamera();
				}
			}
		}
		break;
	
	case PLAYER_CAMERA_MODE_ENTITY_WATCHING:
		// See if we still want to watch this actor
		{
			if ( entity_to_watch )
			{
				Vector cameraPosition;
				Vector cameraAngles;
				GetPlayerView( &cameraPosition, &cameraAngles );
				Vector directionToWatchedEntity( entity_to_watch->origin - cameraPosition );
				Vector cameraForward;
				cameraAngles.AngleVectors( &cameraForward );
				const float angleToWatchedEntity = RAD2DEG( Vector::AngleBetween( directionToWatchedEntity, cameraForward ) );
				if ( angleToWatchedEntity > maximumAngleToWatchedEntity )
				{
					if ( watchEntityForEntireDuration )
					{
						client->ps.useCameraFocalPoint = false;
					}
					else
					{
						StopWatchingEntity();
					}
				}
				else
				{
					client->ps.useCameraFocalPoint = true;
				}

			}
		}
		break;
	
	case PLAYER_CAMERA_MODE_NO_CLIP:
		break;
	
	default:
		assert( false ); // default case is not valid
	}

	
	
	// Select the camera mode for the player (btw I hate that the player knows anything about cameras) -BillS
	if ( actor_camera)
	{
		playerCameraMode = PLAYER_CAMERA_MODE_ACTOR;
		if ( !camera )
		{
			AutomaticallySelectedNewCamera();
		}
	}
	else if ( entity_to_watch )
	{
		playerCameraMode = PLAYER_CAMERA_MODE_ENTITY_WATCHING;
	}
	else if ( camera )
	{
		playerCameraMode = PLAYER_CAMERA_MODE_CINEMATIC;
	}
	else if ( getMoveType() == MOVETYPE_NOCLIP )
	{
		playerCameraMode = PLAYER_CAMERA_MODE_NO_CLIP;
	}
	else
	{
		playerCameraMode = PLAYER_CAMERA_MODE_NORMAL;
	}

	
	// Update the camera position using the appropriate controller 
	switch( playerCameraMode )
	{
	case PLAYER_CAMERA_MODE_NORMAL:
		{
			SetClientViewAngles( origin, client->ps.viewheight, v_angle, velocity, fov );
			SetPlayerViewNormal();		
		}
		break;

	case PLAYER_CAMERA_MODE_ACTOR:
		{
			// Create the camera if we don't have one yet
			if ( !actor_camera )
			{
				actor_camera = new Camera();
			}
			if ( !camera )
			{
				SetCamera( actor_camera, 0.5f );
			}
			SetClientViewAngles( origin, viewheight, v_angle, velocity, camera->Fov() );
			SetPlayerViewUsingActorController( camera );
		}
		break;
	
	case PLAYER_CAMERA_MODE_ENTITY_WATCHING:
		{
			SetClientViewAngles( origin, client->ps.viewheight, v_angle, velocity, fov);
			SetPlayerViewUsingEntityWatchingController();
		}
		break;
	
	case PLAYER_CAMERA_MODE_NO_CLIP:
		{
			SetClientViewAngles( origin, client->ps.viewheight, v_angle, velocity, fov );
			SetPlayerViewUsingNoClipController();
		}
		break;

	case PLAYER_CAMERA_MODE_CINEMATIC:
		{
			SetClientViewAngles( origin, viewheight, v_angle, velocity, camera->Fov() );
			SetPlayerViewUsingCinematicController( camera );
		}
		break;
	
	default:
		assert( false ); // default case is not valid
	}
}


void Player::AutomaticallySelectedNewCamera( void )
{
	float bestScore = 999;
	Camera *bestCamera = NULL;

	for( int i = 1; i <= level.automatic_cameras.NumObjects(); i++ )
	{
		Camera *cam = level.automatic_cameras.ObjectAt( i );
		float score = cam->CalculateScore( this, currentState_Torso->getName() );
		
		// if this is our current camera, scale down the score a bit to favor it.
		if ( cam == camera )
		{
			score *= 0.9f;
		}
		
		if ( score < bestScore )
		{
			bestScore = score;
			bestCamera = cam;
		}
	}
	if ( bestScore <= 1.0f )
	{
		// we have a camera to switch to
		if ( bestCamera != camera )
		{
			if ( camera )
			{
				camera->AutomaticStop( this );
			}
			SetCamera( bestCamera, bestCamera->AutomaticStart( this ) );
		}
	}
	else
	{
		// we don't have a camera to switch to
		if ( camera )
		{
			SetCamera( NULL, camera->AutomaticStop( this ) );
		}
	}
}

//----------------------------------------------------------------
// Name:			getWeaponViewShake
// Class:			Player
//
// Description:		Gets the view shake from the current weapon(s)
//
// Parameters:		none
//
// Returns:			Vector					- angles to shake the view by
//----------------------------------------------------------------

Vector Player::getWeaponViewShake( void )
{
	Vector viewShake;
	Weapon *weapon;

	// Try dual hand 

	weapon = GetActiveWeapon( WEAPON_DUAL );

	if ( weapon )
	{
		viewShake = weapon->getViewShake();
	}
	else
	{
		// Try left hand 

		weapon = GetActiveWeapon( WEAPON_LEFT );

		if ( weapon )
		{
			viewShake = weapon->getViewShake();
		}

		// Try right hand 

		weapon = GetActiveWeapon( WEAPON_RIGHT );

		if ( weapon )
		{
			viewShake += weapon->getViewShake();
		}
	}

	return viewShake;
}

void Player::StopWatchingEntity( void )
{
	entity_to_watch = NULL;
	maximumAngleToWatchedEntity = 0.0f;
	client->ps.useCameraFocalPoint = false;
	client->ps.pm_flags &= ~PMF_CAMERA_VIEW;
}

void Player::DestroyActorCamera( void )
{
	if ( actor_camera )
	{
		delete actor_camera;
		actor_camera = NULL;
		SetCamera( NULL, .5f );
	}
}
/*
==================
SwingAngles
==================
*/
void Player::SwingAngles( float destination, float swingTolerance, float clampTolerance, float speed, float *angle,
		qboolean *swinging )
{
	float	swing;
	float	move;
	float	scale;
	
	if ( !*swinging )
	{
		// see if a swing should be started
		swing = AngleSubtract( *angle, destination );
		if ( ( swing > swingTolerance ) || ( swing < -swingTolerance ) )
		{
			*swinging = true;
			// we intentionally return so that we can start the animation before turning
			return;
		}
	}
	
	if ( !*swinging )
	{
		return;
	}
	
	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract( destination, *angle );
	scale = fabs( swing );
	
/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment
	
#if 0
	if ( scale < swingTolerance * 0.5 )
	{
		scale = 0.5;
	}
	else if ( scale < swingTolerance )
	{
		scale = 1.0;
	}
	else
	{
		scale = 2.0;
	}
#else
	  
****************************************************************************/
	
	scale = 1.0f;
/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment
	
  #endif
	  
****************************************************************************/
	
	
	// swing towards the destination angle
	if ( swing >= 0.0f )
	{
		move = level.frametime * scale * speed;
		if ( move >= swing )
		{
			move = swing;
			*swinging = false;
		}
		
		*angle = AngleMod( *angle + move );
	}
	else if ( swing < 0.0f )
	{
		move = level.frametime * scale * -speed;
		if ( move <= swing )
		{
			move = swing;
			*swinging = false;
		}
		*angle = AngleMod( *angle + move );
	}
	
	// clamp to no more than tolerance
	swing = AngleSubtract( destination, *angle );
	if ( swing > clampTolerance )
	{
		*angle = AngleMod( destination - ( clampTolerance - 1.0f ) );
	}
	else if ( swing < -clampTolerance )
	{
		*angle = AngleMod( destination + ( clampTolerance - 1.0f ) );
	}
}

void Player::SetHeadTarget
   (
   Event *ev
   )

   {
   str ht = ev->GetString( 1 );

   head_target = G_FindTarget( 0, ht );
   }

qboolean Player::GetTagPositionAndOrientation( int tagnum, orientation_t *new_or )
{
	int            i;
	orientation_t  tag_or;
	vec3_t         axis[3];
	
	tag_or = gi.Tag_OrientationEx( edict->s.modelindex,
			CurrentAnim( legs ),
			CurrentFrame( legs ),
			tagnum & TAG_MASK,
			edict->s.scale,
			edict->s.bone_tag,
			edict->s.bone_quat,
			0,
			0,
			1.0f,
			( edict->s.anim & ANIM_BLEND ) != 0,
			( edict->s.torso_anim & ANIM_BLEND ) != 0,
			CurrentAnim( torso ),
			CurrentFrame( torso ),
			0,
			0,
			1.0f
			);
	
	AnglesToAxis( angles, axis );
	VectorCopy( origin, new_or->origin );
	
	for ( i=0;  i<3; i++ )
		VectorMA( new_or->origin, tag_or.origin[i], axis[i], new_or->origin );
	
	MatrixMultiply( tag_or.axis, axis, new_or->axis );
	return true;
}

qboolean Player::GetTagPositionAndOrientation( const str &tagname, orientation_t *new_or )
{
	int            tagnum;
	
	tagnum = gi.Tag_NumForName( edict->s.modelindex, tagname );
	
	if ( tagnum < 0 )
	{
		warning( "Player::GetTagPositionAndOrientation", "Could not find tag \"%s\"", tagname.c_str() );
		return false;
	}
	
	return GetTagPositionAndOrientation( tagnum, new_or );
}

Vector Player::GetAngleToTarget( const Entity *ent, const str &tag, float yawclamp, float pitchclamp, 
		const Vector &baseangles )
{
	assert( ent );
	
	if ( ent )
	{
		Vector         delta,angs;
		orientation_t  tag_or;
		
		int tagnum = gi.Tag_NumForName( edict->s.modelindex, tag.c_str() );
		
		if ( tagnum < 0 )
			return Vector( 0.0f, 0.0f, 0.0f );
		
		GetTagPositionAndOrientation( tagnum, &tag_or );
		
		delta = ent->centroid - tag_or.origin;
		delta.normalize();
		
		angs = delta.toAngles();
		
		//AnglesSubtract( angs, baseangles, angs );
		angs -= baseangles;
		
		angs[PITCH] = AngleNormalize180( angs[PITCH] );
		angs[YAW]   = AngleNormalize180( angs[YAW] );
		
		if ( angs[PITCH] > pitchclamp )
			angs[PITCH] = pitchclamp;
		else if ( angs[PITCH] < -pitchclamp )
			angs[PITCH] = -pitchclamp;
		
		if ( angs[YAW] > yawclamp )
			angs[YAW] = yawclamp;
		else if ( angs[YAW] < -yawclamp )
			angs[YAW] = -yawclamp;
		
		return angs;
	}
	else
	{
		return Vector( 0.0f, 0.0f, 0.0f );
	}
}

void Player::DebugWeaponTags( int controller_tag, Weapon *weapon, const str &weapon_tagname )
{
	int i;
	orientation_t  bone_or, tag_weapon_or, barrel_or, final_barrel_or;
	
	GetTagPositionAndOrientation( edict->s.bone_tag[controller_tag], &bone_or );
	//G_DrawCoordSystem( Vector( bone_or.origin ), Vector( bone_or.axis[0] ), Vector( bone_or.axis[1] ), Vector( bone_or.axis[2] ), 20 );
	
	GetTagPositionAndOrientation( gi.Tag_NumForName( edict->s.modelindex, weapon_tagname ), &tag_weapon_or );
	//G_DrawCoordSystem( Vector( tag_weapon_or.origin ), Vector( tag_weapon_or.axis[0] ), Vector( tag_weapon_or.axis[1] ), Vector( tag_weapon_or.axis[2] ), 40 );
	
	weapon->GetRawTag( "tag_barrel", &barrel_or );
	VectorCopy( tag_weapon_or.origin, final_barrel_or.origin );
	
	for ( i = 0 ; i < 3 ; i++ )
		VectorMA( final_barrel_or.origin, barrel_or.origin[i], tag_weapon_or.axis[i], final_barrel_or.origin );
	
	MatrixMultiply( barrel_or.axis, tag_weapon_or.axis, final_barrel_or.axis );
	//G_DrawCoordSystem( Vector( final_barrel_or.origin ), Vector( final_barrel_or.axis[0] ), Vector( final_barrel_or.axis[1] ), Vector( final_barrel_or.axis[2] ), 80 );
}

Entity const * Player::GetTarget( void ) const
{
	return targetEnemy;
}

bool Player::GetProjectileLaunchAngles( Vector &launchAngles, const Vector &launchPoint, const float initialSpeed, const float gravity ) const
{
	Entity const *target = GetTarget();
	if ( target && targetEnemyLocked)
	{
		const Vector targetPoint( target->centroid );
		
		Trajectory projectileTrajectory( launchPoint, targetPoint, initialSpeed, gravity * -sv_currentGravity->value );
		if ( projectileTrajectory.GetTravelTime() > 0.0f )
		{
			launchAngles.setPitch( projectileTrajectory.GetLaunchAngle() );

			Vector direction( targetPoint - launchPoint );
			direction.z = 0.0f;
			direction.normalize();

			launchAngles.setYaw( direction.toYaw() );
			launchAngles.setRoll( 0.0f );
			return true;
		}

	}
	return false;
}

void Player::AcquireTarget( void )
{
	targetEnemyLocked = false;
	
	if ( GetTarget() )
	{
		GetTarget()->edict->s.eFlags &= ~PMF_ENEMY_TARGETED;
	}
	
	// Find a right hand target ( might be the same one )
	
	Weapon *weapon = GetActiveWeapon( WEAPON_RIGHT );
	if ( ! (weapon && weapon->autoAimTargetSelectionAngle) )
	{
		weapon = GetActiveWeapon( WEAPON_LEFT );
	}
	if ( weapon && weapon->autoAimTargetSelectionAngle )
	{
		Entity *newTarget = FindClosestEntityInRadius( weapon->autoAimTargetSelectionAngle, 170.0f, weapon->GetMaxRange() );
		targetEnemy = newTarget;
		if ( newTarget )
		{
			
			if ( FindClosestEntityInRadius( weapon->autoAimLockonAngle, 170.0f, weapon->GetMaxRange() ) )
			{
				targetEnemyLocked = true;
				targetEnemy->edict->s.eFlags |= PMF_ENEMY_TARGETED;
			}
			return;
		}
	}

}

void Player::AutoAim( void )
{
	if ( deadflag )
		return;
	
	// Check for targets in the FOV
	AcquireTarget();

	// Update Crosshair
	if ( _targetSelectedHighlight )
	{
		_targetSelectedHighlight->hideModel();
	}

	if ( _targetLockedHighlight )
	{
		_targetLockedHighlight->hideModel();
	}

	EntityPtr currentHighlight = _targetSelectedHighlight;
	if ( targetEnemyLocked )
	{
		currentHighlight = _targetLockedHighlight;
	}

	if ( currentHighlight && !level.cinematic)
	{
		if ( !GetTarget() )
		{
			return;
		}
		Weapon *weapon = GetActiveWeapon( WEAPON_RIGHT );
		if ( !(weapon && weapon->crosshair) )
		{
			weapon = GetActiveWeapon( WEAPON_LEFT );
		}
		if ( weapon && weapon->crosshair )
        {
			Vector newCrosshairLocation( GetTarget()->origin );
			currentHighlight->setOrigin( newCrosshairLocation );
			currentHighlight->showModel();
		}

	}
}

   
/*
===============
PlayerAngles
===============
*/
void Player::PlayerAngles( void )
{
	float    deltayaw;
	Vector   moveangles;
	float		speed;
	float    yawAngle;
	float    speedscale;
	vec3_t   temp;
	Vector   dir;
	Vector   newAimAngles;
	
	if ( gi.Anim_Flags( edict->s.modelindex, CurrentAnim( legs ) ) & MDL_ANIM_DEFAULT_ANGLES )
	{
		//SetControllerAngles( HEAD_TAG, vec_zero );
		SetControllerAngles( TORSO_TAG, vec_zero );
		setAngles( Vector( 0.0f, v_angle.y, 0.0f ) );
		return;
	}
	
	// set the head and torso directly
	headAngles.setXYZ( v_angle.x, AngleMod( v_angle.y ), v_angle.z );
	torsoAngles.setXYZ( v_angle.x, AngleMod( v_angle.y ), v_angle.z );
	
	dir = Vector( velocity.x, velocity.y, 0.0f );
	speed = VectorNormalize( dir );
	
	// If moving, angle the legs toward the direction we are moving
	if ( ( speed > 32.0f ) && groundentity && ( last_ucmd.forwardmove || last_ucmd.rightmove ) )
	{
		speedscale = 3;
		yawing   = true; // always center
		deltayaw = AngleSubtract( dir.toYaw(), headAngles[ YAW ] );
	}
	else
	{
		speedscale = 1;
		deltayaw = 0;
	}
	
	// --------- yaw -------------
	// Clamp to g_legclampangle
	if ( fabs( deltayaw ) > 90.0f )
		deltayaw = AngleSubtract( deltayaw, 180.0f );
	
	if ( deltayaw > g_legclampangle->value )
		deltayaw = g_legclampangle->value;
	else if ( deltayaw < -g_legclampangle->value )
		deltayaw = -g_legclampangle->value;
	
	yawAngle = headAngles[ YAW ] + deltayaw;
	
	yawing_left = false;
	yawing_right = false;
	
	if ( client->ps.walking && client->ps.groundPlane && ( movecontrol == MOVECONTROL_LEGS ) &&
		!last_ucmd.forwardmove && !last_ucmd.rightmove && ( client->ps.groundTrace.plane.normal[ 2 ] < SLOPE_22_MAX ) )
	{
		float groundyaw;
		float yawdelta;
		
		groundyaw = ( int )vectoyaw( client->ps.groundTrace.plane.normal );
		yawdelta = anglemod( v_angle.y - groundyaw );
		yawdelta = (float)( ( ( ( int )yawdelta + 45 ) / 90 ) * 90.0f );
		angles.y = groundyaw + yawdelta;
	}
	else
	{
		// if purely strafing, don't swing the legs
		if ( dont_turn_legs )
		{
			setAngles( Vector( 0, v_angle.y, 0 ) );
		}
		else
		{
			SwingAngles( yawAngle, g_legtolerance->value, g_legclamptolerance->value, g_legswingspeed->value * speedscale, &angles[ YAW ], &yawing );
			/*if ( yawing )
            {
            float swing;
			swing = AngleSubtract( yawAngle, angles[ YAW ] );
			if ( swing > 0.0f )
			{
			yawing_left = true;
			}
            else
			{
			yawing_right = true;
			}
		}*/
			
			if(last_ucmd.deltaAngles[ YAW ] > 0.0f)
				yawing_left = true;
			if(last_ucmd.deltaAngles[ YAW] < 0.0f)
				yawing_right = true;
		}
	}
	
	if ( ( deadflag == DEAD_DYING ) || ( deadflag == DEAD_DEAD ) )
		headAngles[PITCH] = 0.0f;
	
	// --------- pitch -------------
	headAngles[PITCH] *= 0.65f; // Nerf head angles alittle... head pitch 90 degrees is not reasonable.
	
	// only show a fraction of the pitch angle in the torso
	if ( headAngles[ PITCH ] > 180.0f )
	{
		torsoAngles[PITCH] = ( -360.0f + headAngles[ PITCH ] ) * bendTorsoMult;
	}
	else
	{
		torsoAngles[PITCH] = headAngles[ PITCH ] * bendTorsoMult;
	}
	
	if ( _headWatchAllowed )
		AcquireHeadTarget();
	else
		head_target = NULL;
	
	// Adjust head and torso angles for the target if needed
	if ( head_target )
	{
		//newAimAngles   = GetAngleToTarget( head_target, "Bip01 Head", 60, 45, torsoAngles );
		newAimAngles   = GetAngleToTarget( head_target, "Bip01 Head", 40.0f, 30.0f, torsoAngles );
		
		headAimAngles[PITCH]  = LerpAngle( headAimAngles[PITCH], newAimAngles[PITCH], 0.25f );
		headAimAngles[YAW]    = LerpAngle( headAimAngles[YAW],   newAimAngles[YAW],   0.25f );
		
		torsoAimAngles[PITCH] = LerpAngle( torsoAimAngles[PITCH], 0.0f, 0.5f );
		torsoAimAngles[YAW]   = LerpAngle( torsoAimAngles[YAW],   0.0f, 0.5f );
	}
	else // Otherwise return to them to 0
	{
		headAimAngles[PITCH]  = LerpAngle( headAimAngles[PITCH],  0.0f, 0.5f );
		headAimAngles[YAW]    = LerpAngle( headAimAngles[YAW],    0.0f, 0.1f );
		
		torsoAimAngles[PITCH] = LerpAngle( torsoAimAngles[PITCH], 0.0f, 0.5f );
		torsoAimAngles[YAW]   = LerpAngle( torsoAimAngles[YAW],   0.0f, 0.5f );
	}

	//if ( !multiplayerManager.inMultiplayer() )
	{
		// pull the head angles back out of the hierarchial chain
		AnglesSubtract( headAngles, torsoAngles, temp );
		
		// Add in the aim angles for the head
		VectorAdd( temp, headAimAngles, temp );
		
		// Update the head controller
		SetControllerAngles( HEAD_TAG, temp );
		
		// pull the torso angles back out of the hierarchial chain
		AnglesSubtract( torsoAngles, angles, temp );
		
		// Add in the aim angles for the torso
		VectorAdd( temp, torsoAimAngles, temp );
		
		// Update the torso controller
		SetControllerAngles( TORSO_TAG, temp );
	}
	
	// Set the rest (legs)
	setAngles( angles );
}

void Player::FinishMove( void )
{
	//
	// If the origin or velocity have changed since ClientThink(),
	// update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	//
	// If it wasn't updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	//
	if ( !( client->ps.pm_flags & PMF_FROZEN ) && !( client->ps.pm_flags & PMF_NO_MOVE ))
	{
		origin.copyTo( client->ps.origin );
		velocity.copyTo( client->ps.velocity );
	}
	
	if ( !( client->ps.pm_flags & PMF_FROZEN ) )
	{
		PlayerAngles();
	}
	
	// burn from lava, etc
	WorldEffects();
	
	// determine the view offsets
	DamageFeedback();
	CalcBlend();
}

void Player::UpdateStats( void )
{
	int i,count;
	float healthToDisplay;
	float armorToDisplay;
	
	// Deathmatch stats for arena mode
	if ( multiplayerManager.inMultiplayer() )
	{
		/*
		// Arena name configstring index
		if ( current_arena )
		{
			client->ps.stats[STAT_ARENA] = CS_ARENA_INFO + current_arena->getID();
		}
		else
		{
			// CS_ARENA_INFO index holds the "No Arena" string
			client->ps.stats[STAT_ARENA] = CS_ARENA_INFO;
		}
		
		if ( current_team )
		{
			client->ps.stats[STAT_TEAM] = current_team->getIndex();
		}
		else
		{
			// CS_TEAM_INFO index holds the "No Team" string
			client->ps.stats[STAT_TEAM] = CS_TEAM_INFO;
		}
		*/

		//client->ps.stats[STAT_KILLS]		= multiplayerManager.getPoints( this );
		//client->ps.stats[STAT_DEATHS]		= multiplayerManager.getDeaths( this );
		//client->ps.stats[STAT_ARENA]		= multiplayerManager.getTeamPoints( this );

		client->ps.stats[STAT_RED_TEAM_SCORE]  = multiplayerManager.getTeamPoints( "Red" );
		client->ps.stats[STAT_BLUE_TEAM_SCORE] = multiplayerManager.getTeamPoints( "Blue" );

		client->ps.stats[STAT_SCORE]  = multiplayerManager.getPoints( this );
		client->ps.stats[STAT_KILLS]  = multiplayerManager.getKills( this );
		client->ps.stats[STAT_DEATHS] = multiplayerManager.getDeaths( this );
		
		client->ps.stats[STAT_MP_GENERIC1] = multiplayerManager.getStat( this, STAT_MP_GENERIC1 );
		client->ps.stats[STAT_MP_GENERIC2] = multiplayerManager.getStat( this, STAT_MP_GENERIC2 );
		client->ps.stats[STAT_MP_GENERIC3] = multiplayerManager.getStat( this, STAT_MP_GENERIC3 );
		client->ps.stats[STAT_MP_GENERIC4] = multiplayerManager.getStat( this, STAT_MP_GENERIC4 );
		client->ps.stats[STAT_MP_GENERIC5] = multiplayerManager.getStat( this, STAT_MP_GENERIC5 );
		client->ps.stats[STAT_MP_GENERIC6] = multiplayerManager.getStat( this, STAT_MP_GENERIC6 );
		client->ps.stats[STAT_MP_GENERIC7] = multiplayerManager.getStat( this, STAT_MP_GENERIC7 );
		client->ps.stats[STAT_MP_GENERIC8] = multiplayerManager.getStat( this, STAT_MP_GENERIC8 );

		client->ps.stats[STAT_MP_SPECTATING_ENTNUM] = multiplayerManager.getStat( this, STAT_MP_SPECTATING_ENTNUM );
		
		client->ps.stats[STAT_MP_MODE_ICON]			= multiplayerManager.getIcon( this, STAT_MP_MODE_ICON );
		client->ps.stats[STAT_MP_TEAM_ICON]			= multiplayerManager.getIcon( this, STAT_MP_TEAM_ICON );
		client->ps.stats[STAT_MP_TEAMHUD_ICON]		= multiplayerManager.getIcon( this, STAT_MP_TEAMHUD_ICON );
		client->ps.stats[STAT_MP_SPECIALTY_ICON]	= multiplayerManager.getIcon( this, STAT_MP_SPECIALTY_ICON );

		client->ps.stats[STAT_MP_OTHERTEAM_ICON]	= multiplayerManager.getIcon( this, STAT_MP_OTHERTEAM_ICON );

		if ( multiplayerManager.inMultiplayer() )
			edict->s.infoIcon = multiplayerManager.getInfoIcon( this, last_ucmd.buttons );
		else
			edict->s.infoIcon = 0;

		client->ps.stats[STAT_MP_AWARD_ICON]  = multiplayerManager.getIcon( this, STAT_MP_AWARD_ICON );
		client->ps.stats[STAT_MP_AWARD_COUNT] = multiplayerManager.getStat( this, STAT_MP_AWARD_COUNT );

		client->ps.stats[STAT_MP_STATE] = multiplayerManager.getStat( this, STAT_MP_STATE );
		
		if ( _holdableItem )
			client->ps.stats[STAT_MP_HOLDABLEITEM_ICON]	= _holdableItem->getIcon();
		else
			client->ps.stats[STAT_MP_HOLDABLEITEM_ICON]	= -1;
		
		if ( _rune )
			client->ps.stats[STAT_MP_RUNE_ICON]			= _rune->getIcon();
		else
			client->ps.stats[STAT_MP_RUNE_ICON]			= -1;
		
		if ( _powerup )
			client->ps.stats[STAT_MP_POWERUP_ICON]		= _powerup->getIcon();
		else
			client->ps.stats[STAT_MP_POWERUP_ICON]		= -1;
		
		/* client->ps.stats[STAT_WON_MATCHES]     = num_won_matches;
		client->ps.stats[STAT_LOST_MATCHES]    = num_lost_matches; */
		
		// Get queue position
		//client->ps.stats[STAT_QUEUE_PLACE] = 0;
		/*
		if ( current_arena )
		{
			client->ps.stats[STAT_QUEUE_PLACE] = current_arena->GetLinePosition( this );
		}
		*/
//		multiplayerManager.getStat( this, STAT_TIMELEFT_MINUTES );
		client->ps.stats[STAT_TIMELEFT_SECONDS] = multiplayerManager.getStat( this, STAT_TIMELEFT_SECONDS );
	}
	
	//
	// Health
	//

	// Use my health/armor as the default to display

	healthToDisplay = health;
	armorToDisplay = GetArmorValue();

	if ( multiplayerManager.inMultiplayer() && multiplayerManager.isPlayerSpectator( this, SPECTATOR_TYPE_FOLLOW ) )
	{
		Player *playerSpectating;

		playerSpectating = multiplayerManager.getPlayerSpectating( this );

		if ( playerSpectating )
		{
			// Display the health/armor of the player we are spectating instead of ours

			healthToDisplay = playerSpectating->getHealth();
			armorToDisplay = playerSpectating->GetArmorValue();
		}
	}

	// Round health a little so that it doesn't mislead the player when it gets changed to an int

	if ( ( healthToDisplay < 1.0f ) && ( healthToDisplay > 0.0f ) )
	{
		client->ps.stats[ STAT_HEALTH ] = 1;
	}
	else
	{
		client->ps.stats[ STAT_HEALTH ] = (int)healthToDisplay;
	}


	if ( health <= 0.0f )
	{
		Vector damageAngles = damage_from * -1;
		damageAngles = damageAngles.toAngles();
		client->ps.stats[ STAT_DEAD_YAW ] = (int)  damageAngles[ YAW ];
	}
	else
	{
		client->ps.stats[ STAT_DEAD_YAW ] = 0;
	}
	
	//ArmorValue Stat Update
	client->ps.stats[ STAT_ARMOR_LEVEL ] = (int) armorToDisplay;
	
	Weapon *leftweapon  = GetActiveWeapon( WEAPON_LEFT );
	Weapon *rightweapon = GetActiveWeapon( WEAPON_RIGHT );
	Weapon *dualweapon  = GetActiveWeapon( WEAPON_DUAL );
	
	client->ps.stats[STAT_AMMO_LEFT]                 = 0;
	client->ps.stats[STAT_AMMO_RIGHT]                = 0;
	client->ps.stats[STAT_CLIPAMMO_LEFT]             = 0;
	client->ps.stats[STAT_CLIPAMMO_RIGHT]            = 0;
	client->ps.stats[STAT_NUM_SHOTS_LEFT]			 = 0;
	client->ps.stats[STAT_MAX_NUM_SHOTS_LEFT]		 = 0;
	client->ps.stats[STAT_NUM_SHOTS_RIGHT]			 = 0;
	client->ps.stats[STAT_MAXAMMO_LEFT]              = 0;
	client->ps.stats[STAT_MAXAMMO_RIGHT]             = 0;
	client->ps.stats[STAT_MAXCLIPAMMO_LEFT]          = 0;
	client->ps.stats[STAT_MAXCLIPAMMO_RIGHT]         = 0;

	client->ps.stats[STAT_AMMO_TYPE1]				 = 0;
	client->ps.stats[STAT_AMMO_TYPE2]				 = 0;
	client->ps.stats[STAT_AMMO_TYPE3]				 = 0;
	client->ps.stats[STAT_AMMO_TYPE4]				 = 0;

	if ( _powerup )
		client->ps.stats[ STAT_POWERUPTIME ] = static_cast<int>( _powerup->getTimeLeft() );
	else
		client->ps.stats[ STAT_POWERUPTIME ] = 0;
	
	client->ps.stats[ STAT_ACCUMULATED_PAIN]         = (int) accumulated_pain;
	
	client->ps.activeItems[ ITEM_NAME_AMMO_LEFT		]		= -1;
	client->ps.activeItems[ ITEM_NAME_AMMO_RIGHT	]		= -1;
	client->ps.activeItems[ ITEM_NAME_WEAPON_LEFT	]		= -1;
	client->ps.activeItems[ ITEM_NAME_WEAPON_RIGHT	]		= -1;
	client->ps.activeItems[ ITEM_NAME_WEAPON_DUAL	]		= -1;
#ifndef DEDICATED	
	//
	// mission objectives
	//
	client->ps.stats[STAT_NUM_OBJECTIVES]			= gi.MObjective_GetNumActiveObjectives();
	client->ps.stats[STAT_COMPLETE_OBJECTIVES]		= gi.MObjective_GetNumCompleteObjectives();
	client->ps.stats[STAT_FAILED_OBJECTIVES]		= gi.MObjective_GetNumFailedObjectives();
	client->ps.stats[STAT_INCOMPLETE_OBJECTIVES]	= gi.MObjective_GetNumIncompleteObjectives();
#endif

	// misc stats
	if ( client->ps.stats[ STAT_SHOTS_FIRED ] > 0 )
		client->ps.stats[STAT_ACCURACY] = (int)((float)( (float)client->ps.stats[ STAT_SHOTS_HIT ] / (float)client->ps.stats[ STAT_SHOTS_FIRED ] ) * 100.0f);
	else
		client->ps.stats[STAT_ACCURACY] = 100;
		
	client->ps.stats[STAT_MISSION_DURATION] = (int) level.timeInLevel;
	
	client->ps.stats[ STAT_WEAPON_GENERIC1 ] = 0;
	client->ps.stats[ STAT_WEAPON_GENERIC2 ] = 0;
	
	if ( dualweapon )
	{
		// Left is PRIMARY
		if ( dualweapon->GetClipSize( FIRE_MODE1 ) > 0 )
		{
			//set the max ammo to contain the number of clips and the max number of clips.
			client->ps.stats[STAT_AMMO_LEFT]        = AmmoCount( dualweapon->GetAmmoType( FIRE_MODE1 ) ) / dualweapon->GetClipSize( FIRE_MODE1 );
			client->ps.stats[STAT_MAXAMMO_LEFT]     = MaxAmmoCount( dualweapon->GetAmmoType( FIRE_MODE1 ) ) / dualweapon->GetClipSize( FIRE_MODE1 );

			client->ps.stats[STAT_CLIPAMMO_LEFT]    = dualweapon->ClipAmmo( FIRE_MODE1 );
			client->ps.stats[STAT_MAXCLIPAMMO_LEFT] = dualweapon->GetClipSize( FIRE_MODE1 );

			if( dualweapon->ammorequired[FIRE_MODE1] != 0)
			{
				client->ps.stats[STAT_NUM_SHOTS_LEFT]		= dualweapon->ClipAmmo( FIRE_MODE1 ) / dualweapon->ammorequired[FIRE_MODE1]; 
				client->ps.stats[STAT_MAX_NUM_SHOTS_LEFT]	= dualweapon->GetClipSize( FIRE_MODE1 ) / dualweapon->ammorequired[FIRE_MODE1];
			}
		}
		else
		{
			client->ps.stats[STAT_AMMO_LEFT]        = AmmoCount( dualweapon->GetAmmoType( FIRE_MODE1 ) );
			client->ps.stats[STAT_MAXAMMO_LEFT]     = MaxAmmoCount( dualweapon->GetAmmoType( FIRE_MODE1 ) );

			client->ps.stats[STAT_CLIPAMMO_LEFT]    = AmmoCount( dualweapon->GetAmmoType( FIRE_MODE1 ) );
			client->ps.stats[STAT_MAXCLIPAMMO_LEFT] = MaxAmmoCount( dualweapon->GetAmmoType( FIRE_MODE1 ) );

			if( dualweapon->ammorequired[FIRE_MODE1] != 0)
			{
				client->ps.stats[STAT_NUM_SHOTS_LEFT]		= AmmoCount( dualweapon->GetAmmoType( FIRE_MODE1 ) ) / dualweapon->ammorequired[FIRE_MODE1]; 
				client->ps.stats[STAT_MAX_NUM_SHOTS_LEFT]	= MaxAmmoCount( dualweapon->GetAmmoType( FIRE_MODE1 ) ) / dualweapon->ammorequired[FIRE_MODE1];
			}
			

		}


		
		client->ps.activeItems[ITEM_NAME_AMMO_LEFT]     = AmmoIndex( dualweapon->GetAmmoType( FIRE_MODE1 ) );
		
		// Right is AlTERNATE
		client->ps.stats[STAT_AMMO_RIGHT]               = AmmoCount( dualweapon->GetAmmoType( FIRE_MODE2 ) );
		client->ps.stats[STAT_MAXAMMO_RIGHT]            = MaxAmmoCount( dualweapon->GetAmmoType( FIRE_MODE2 ) );
		
		if ( dualweapon->GetClipSize( FIRE_MODE2 ) > 0 )
		{
			client->ps.stats[STAT_CLIPAMMO_RIGHT]    = dualweapon->ClipAmmo( FIRE_MODE2 );
			client->ps.stats[STAT_MAXCLIPAMMO_RIGHT] = dualweapon->GetClipSize( FIRE_MODE2 );

			if(dualweapon->ammorequired[FIRE_MODE2] != 0)
				client->ps.stats[STAT_NUM_SHOTS_RIGHT]	 = dualweapon->ClipAmmo( FIRE_MODE2 ) / dualweapon->ammorequired[FIRE_MODE2]; 
		}
		else
		{
			client->ps.stats[STAT_CLIPAMMO_RIGHT]    = AmmoCount( dualweapon->GetAmmoType( FIRE_MODE2 ) );
			client->ps.stats[STAT_MAXCLIPAMMO_RIGHT] = MaxAmmoCount( dualweapon->GetAmmoType( FIRE_MODE2 ) );

			if(dualweapon->ammorequired[FIRE_MODE2] != 0)
				client->ps.stats[STAT_NUM_SHOTS_RIGHT]	 = AmmoCount( dualweapon->GetAmmoType( FIRE_MODE2 ) ) / dualweapon->ammorequired[FIRE_MODE2]; 

		}
		


		
		client->ps.activeItems[ITEM_NAME_AMMO_RIGHT]    = AmmoIndex( dualweapon->GetAmmoType( FIRE_MODE2 ) );
		
		client->ps.activeItems[ITEM_NAME_WEAPON_DUAL] = dualweapon->getIndex();

		//Send the # for all ammo groups
		//This should be fixed since its a poor implementation.

		for( int i = 1; i <= ammo_inventory.NumObjects(); i++ )
		{
			if(ammo_inventory.ObjectAt(i)->getName() == "Plasma")
			{
				client->ps.stats[STAT_AMMO_TYPE1] = ammo_inventory.ObjectAt(i)->getAmount();
			}
			else if(ammo_inventory.ObjectAt(i)->getName() == "Fed")
			{
				client->ps.stats[STAT_AMMO_TYPE2] = ammo_inventory.ObjectAt(i)->getAmount();
			}
			else if(ammo_inventory.ObjectAt(i)->getName() == "Idryll")
			{
				client->ps.stats[STAT_AMMO_TYPE3] = ammo_inventory.ObjectAt(i)->getAmount();
			}
			else if( ammo_inventory.ObjectAt(i)->getName() == "Phaser" )
			{
				client->ps.stats[STAT_AMMO_TYPE4] = ammo_inventory.ObjectAt(i)->getAmount();
			}
		}

		client->ps.stats[ STAT_WEAPON_GENERIC1 ] = dualweapon->getStat( STAT_WEAPON_GENERIC1 );
		client->ps.stats[ STAT_WEAPON_GENERIC2 ] = dualweapon->getStat( STAT_WEAPON_GENERIC2 );
	}
	else
	{
		if ( leftweapon )
		{
			client->ps.stats[STAT_AMMO_LEFT]                = AmmoCount( leftweapon->GetAmmoType( FIRE_MODE1 ) );
			client->ps.stats[STAT_MAXAMMO_LEFT]             = MaxAmmoCount( leftweapon->GetAmmoType( FIRE_MODE1 ) );
			client->ps.stats[STAT_CLIPAMMO_LEFT]            = leftweapon->ClipAmmo( FIRE_MODE1 );
			client->ps.stats[STAT_MAXCLIPAMMO_LEFT]         = leftweapon->GetClipSize( FIRE_MODE1 );
			client->ps.activeItems[ITEM_NAME_AMMO_LEFT]     = AmmoIndex( leftweapon->GetAmmoType( FIRE_MODE1 ) );
			client->ps.activeItems[ITEM_NAME_WEAPON_LEFT]   = leftweapon->getIndex();
			
		}
		
		if ( rightweapon )
		{
			client->ps.stats[STAT_AMMO_RIGHT]               = AmmoCount( rightweapon->GetAmmoType( FIRE_MODE1 ) );
			client->ps.stats[STAT_MAXAMMO_RIGHT]            = MaxAmmoCount( rightweapon->GetAmmoType( FIRE_MODE1 ) );
			client->ps.stats[STAT_CLIPAMMO_RIGHT]           = rightweapon->ClipAmmo( FIRE_MODE1 );
			client->ps.stats[STAT_MAXCLIPAMMO_RIGHT]        = rightweapon->GetClipSize( FIRE_MODE1 );
			client->ps.activeItems[ITEM_NAME_AMMO_RIGHT]    = AmmoIndex( rightweapon->GetAmmoType( FIRE_MODE1 ) );
			client->ps.activeItems[ITEM_NAME_WEAPON_RIGHT]  = rightweapon->getIndex();
			
		}
	}
	
	
	//
	// set boss health and name
	//
	
	if ( bosshealth->value > 0.0f )
	{
		client->ps.stats[ STAT_BOSSHEALTH ] = (int)(bosshealth->value * 100.0f);
		
		if ( ( ( bosshealth->value * 100.0f ) > 0 ) && ( client->ps.stats[ STAT_BOSSHEALTH ] == 0 ) )
		{
			client->ps.stats[ STAT_BOSSHEALTH ] = 1;
		}
		
		client->ps.stats[ STAT_BOSSNAME_CONFIGINDEX ] = G_FindConfigstringIndex( bossname->string, CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true ) + CS_GENERAL_STRINGS;
		//client->ps.stats[ STAT_BOSSNAME_CONFIGINDEX ] = gi.soundindex( bossname->string ) + CS_SOUNDS;
	}
	else
	{
		client->ps.stats[ STAT_BOSSHEALTH ] = 0;
	}

	if ( _itemText.length() > 0  )
	{
		client->ps.stats[ STAT_ITEMICON ] = _itemIcon;
		
		client->ps.stats[ STAT_ITEMTEXT ] = G_FindConfigstringIndex( _itemText.c_str(), CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true ) + CS_GENERAL_STRINGS;
		//client->ps.stats[ STAT_BOSSNAME_CONFIGINDEX ] = gi.soundindex( bossname->string ) + CS_SOUNDS;
	}
	else
	{
		client->ps.stats[ STAT_ITEMTEXT ] = -1;
	}

	if ( _voteText.length() > 0  )
	{	
		client->ps.stats[ STAT_VOTETEXT ] = G_FindConfigstringIndex( _voteText.c_str(), CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true ) + CS_GENERAL_STRINGS;
	}
	else
	{
		client->ps.stats[ STAT_VOTETEXT ] = -1;
	}
	
	if ( specialMoveEndTime > 0.0f )
	{
		float beginTime = specialMoveEndTime - specialMoveChargeTime;
		float timeElapsed = specialMoveCharge - beginTime;
		float percentElapsed = timeElapsed / specialMoveChargeTime;
		float finalValue = percentElapsed * 100.0f;
		if ( finalValue > 100.0f )
			finalValue = 100.0f;
		client->ps.stats[ STAT_SPECIALMOVETIMER ] = (int) finalValue;
	}	
	else
		client->ps.stats[ STAT_SPECIALMOVETIMER ] = 0;
	
	client->ps.stats[ STAT_POINTS ] = points;

	client->ps.stats[ STAT_SECRETS_TOTAL ] = level.total_secrets;
	client->ps.stats[ STAT_SECRETS_FOUND ] = level.found_secrets;
	client->ps.stats[ STAT_ITEMS_TOTAL ] = level.total_specialItems;
	client->ps.stats[ STAT_ITEMS_FOUND ] = level.found_specialItems;
	
	
	// Set cinematic stuff
	
	client->ps.stats[ STAT_CINEMATIC ] = 0;
	
	if ( level.cinematic )
		client->ps.stats[ STAT_CINEMATIC ] = (1<<0);
	
	if ( actor_camera )
		client->ps.stats[ STAT_CINEMATIC ] += (1<<1);
	
	// Go through all the player's weapons and send over the indexes of the names
	memset( client->ps.inventory_name_index, 0, sizeof( client->ps.inventory_name_index ) );
	memset( client->ps.ammo_in_clip, 0, sizeof(client->ps.ammo_in_clip));
	
	count = inventory.NumObjects();
	
	if ( count > MAX_INVENTORY )
	{
		count = MAX_INVENTORY;
		warning( "Player::UpdateStats", "Max inventory exceeded\n" );
	}
	
	Weapon* weapon;
	Ammo*	ammo;
	int		ammoRequired;
	for ( i=1; i<=count; i++ )
	{
		int entnum = inventory.ObjectAt( i );
		Item *item = ( Item * )G_GetEntity( entnum );

		if ( item )
		{
			client->ps.inventory_name_index[i-1] = item->getIndex();
			if( item->isSubclassOf(Weapon) )
			{
				weapon = (Weapon*) item;
				ammo = FindAmmoByName(weapon->GetAmmoType(FIRE_MODE1));
				if( ammo == 0)
					continue;
				client->ps.inventory_weapon_ammo_index[ i - 1 ]		= ammo->getIndex();
				
				ammoRequired = weapon->GetRequiredAmmo(FIRE_MODE1);
				client->ps.inventory_weapon_required_ammo[i - 1]	= ammoRequired;
				
				client->ps.ammo_in_clip[ i - 1 ] = weapon->getAmmoInClip(FIRE_MODE1);
			}
		}
	}
	
	// Go through all the player's ammo and send over the names/amounts
	memset( client->ps.ammo_amount, 0, sizeof( client->ps.ammo_amount ) );
	count = ammo_inventory.NumObjects();
	
	assert( count < MAX_AMMO );
	if ( count > MAX_AMMO )
	{
		gi.Error( ERR_DROP, "Player::UpdateStats : Exceeded MAX_AMMO\n" );
	}
	
	for ( i=1; i<=count; i++ )
	{
		Ammo *ammo = ammo_inventory.ObjectAt( i );
		
		if ( ammo )
		{
			client->ps.ammo_amount[i-1]     = ammo->getAmount();
			client->ps.max_ammo_amount[i-1] = ammo->getMaxAmount();
			client->ps.ammo_name_index[i-1] = ammo->getIndex();
		}
	}
	
	// Do letterbox
	
	// Check for letterbox fully out
	if ( ( level.m_letterbox_time <= 0.0f ) && ( level.m_letterbox_dir == letterbox_in ) )
	{
		client->ps.stats[STAT_LETTERBOX] = (int)(level.m_letterbox_fraction * MAX_LETTERBOX_SIZE);
		return;
	}
	else if ( ( level.m_letterbox_time <= 0.0f ) && ( level.m_letterbox_dir == letterbox_out ) )
	{
		client->ps.stats[STAT_LETTERBOX] = 0;
		return;
	}
	
	float frac;
	
	level.m_letterbox_time -= level.frametime;
	
	frac = level.m_letterbox_time / level.m_letterbox_time_start;
	
	if ( frac > 1.0f )
		frac = 1.0f;
	if ( frac < 0.0f )
		frac = 0.0f;
	
	if ( level.m_letterbox_dir == letterbox_in )
		frac = 1.0f - frac;
	
	client->ps.stats[STAT_LETTERBOX] = (int)((float)( frac * level.m_letterbox_fraction ) * MAX_LETTERBOX_SIZE);
	
	// Stats for Mission Status
}

//-----------------------------------------------------
//
// Name:		UpdateObjectiveStatus	
// Class:		Player	
//
// Description:	Updates the player state of the mission objectives.
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void Player::UpdateObjectiveStatus( void )
{
	client->ps.objectiveStates = _objectiveStates;
	client->ps.informationStates = _informationStates;
	client->ps.objectiveNameIndex = _objectiveNameIndex;
}

void Player::UpdateMusic( void )
{
	
	if ( !g_allowActionMusic->integer || !_allowActionMusic )
	{
		action_level = 0.0f;
	}
	
	if ( music_forced )
	{
		client->ps.current_music_mood = music_current_mood;
		client->ps.fallback_music_mood = music_fallback_mood;
	}
	else if ( action_level > 30.0f )
	{
		client->ps.current_music_mood = mood_action;
		client->ps.fallback_music_mood = mood_normal;
	}
	else if ( action_level < 15.0f )
	{
		client->ps.current_music_mood = music_current_mood;
		client->ps.fallback_music_mood = music_fallback_mood;
	}
	
	if ( action_level > 0.0f )
	{
		if ( multiplayerManager.inMultiplayer() )
		{
			action_level -= level.fixedframetime * 2.0f;
		}
		else
		{
			action_level -= level.fixedframetime * 15.0f;
		}

		if ( action_level > 80.0f )
		{
			action_level = 80.0f;
		}
	}
	else
	{
		action_level = 0.0f;
	}
	
	//
	// set the music
	// naturally decay the action level
	//
	if ( s_debugmusic->integer )
	{
		gi.DPrintf( "%s's action_level = %4.2f\n", client->pers.netname, action_level );
	}
	
	// Copy music volume and fade time to player state
	client->ps.music_volume				= music_current_volume;
	client->ps.music_volume_fade_time	= music_volume_fade_time;
	client->ps.allowMusicDucking		= _allowMusicDucking;
}

void Player::SetReverb( int type, float level )
{
	reverb_type  = type;
	reverb_level = level;
}

void Player::SetReverb( const str &type, float level )
{
	reverb_type  = EAXMode_NameToNum( type );
	reverb_level = level;
}

void Player::SetReverb( Event *ev )
{
	if ( ev->NumArgs() < 2 )
		return;
	
	SetReverb( ev->GetInteger( 1 ), ev->GetFloat( 2 ) );
}

void Player::UpdateReverb( void )
{
	client->ps.reverb_type  = reverb_type;
	client->ps.reverb_level = reverb_level;
}

void Player::EndAnim_Legs( Event * )
{
	if ( vehicle )
		return;
	
	animdone_Legs = true;
	animate->SetAnimDoneEvent( EV_Player_AnimLoop_Legs, legs );
	EvaluateState();
}

void Player::EndAnim_Torso( Event * )
{
	if ( vehicle )
		return;
	
	animdone_Torso = true;
	animate->SetAnimDoneEvent( EV_Player_AnimLoop_Torso, torso );
	EvaluateState();
}

void Player::SetAnim( const char *anim, bodypart_t part, bool force )
{
	assert( anim );
	
	if ( !force && ( part != all ) && ( partAnim[ part ] == anim ) )
	{
		return;
	}
	
	if ( !force && ( part == all ) && ( partAnim[ legs ] == anim ) && ( partAnim[ torso ] == anim ) )
	{
		return;
	}
	
	if ( getMoveType() == MOVETYPE_NOCLIP )
	{
		anim = "idle";
	}
	
	if ( part != all )
	{
		partAnim[ part ] = anim;
	}
	else
	{
		partAnim[ legs ] = anim;
		partAnim[ torso ] = anim;
	}
	
	switch( part )
	{
	default :
	case all :
		animate->RandomAnimate( anim, EV_Player_AnimLoop_Legs, part );
		break;
		
	case legs :
		animate->RandomAnimate( anim, EV_Player_AnimLoop_Legs, part );
		break;
		
	case torso :
		animate->RandomAnimate( anim, EV_Player_AnimLoop_Torso, part );
		break;
	}
	
	if ( ( part == legs ) || ( part == all ) )
	{
		Vector animmove;
		float time;
		float len;
		
		time = gi.Anim_Time( edict->s.modelindex, CurrentAnim() );
		gi.Anim_Delta( edict->s.modelindex, CurrentAnim(), animmove );
		len = animmove.length();
		if ( ( len == 0.0f ) || ( time == 0.0f ) )
		{
			animspeed = 0;
		}
		else
		{
			animspeed = len / time;
		}
	}
}

void Player::CheckReloadWeapons( void )
{
	Weapon *weap;
	
	weap = GetActiveWeapon( WEAPON_DUAL );

	if ( weap )
	{
		weap->CheckReload();
	}
	else
	{
		weap = GetActiveWeapon( WEAPON_LEFT );
		if ( weap )
		{
			weap->CheckReload();
		}
		
		weap = GetActiveWeapon( WEAPON_RIGHT );
		if ( weap )
		{
			weap->CheckReload();
		}
	}
}

void Player::UpdateMisc( void )
{
	//
	// clear out the level exit flag
	//
	client->ps.pm_flags &= ~PMF_LEVELEXIT;
	
	//
	// see if our camera is the level exit camera
	//
	if ( camera && camera->IsLevelExit() )
	{
		client->ps.pm_flags |= PMF_LEVELEXIT;
	}
	else if ( level.near_exit )
	{
		client->ps.pm_flags |= PMF_LEVELEXIT;
	}
	//
	// do anything special for level exits
	//
	if ( client->ps.pm_flags & PMF_LEVELEXIT )
	{
		//
		// change music
		//
		if ( music_current_mood != mood_success )
		{
			ChangeMusic( "success", "normal", false );
		}
	}

	if(_updateGameFrames == true)
		_totalGameFrames++;

	//Update the dialog information.
	client->ps.dialogEntnum			= _dialogEntnum;
	client->ps.dialogSoundIndex		= _dialogSoundIndex;
	client->ps.dialogTextSoundIndex = _dialogTextSoundIndex;
}

/*
=================
EndFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void Player::EndFrame( Event * )
{
	AutoAim();
	
	if ( meleeAttackFlags & MELEE_ATTACK_LEFT )
		AdvancedMeleeAttack(WEAPON_LEFT);
	
	if ( meleeAttackFlags & MELEE_ATTACK_RIGHT )
		AdvancedMeleeAttack(WEAPON_RIGHT);
	
	if ( finishableList.NumObjects() > 0 )
		HandleFinishableList();
	else
	{
		//G_EnableWidgetOfPlayer( edict, "ActionIcon_Kick", false ); // Temporary
		_doingFinishingMove = false;
		_finishActor = NULL;
		_finishState = "";
	}
	
	FinishMove();
	CheckReloadWeapons();
	UpdateStats();
	UpdateMusic();
	UpdateReverb();
	UpdateMisc();
	SetupView();
	UpdateObjectiveStatus();
	
	if ( _powerup )
		_powerup->update( level.frametime );
	
	if ( _rune )
		_rune->update( level.frametime );
	
	if ( _holdableItem )
		_holdableItem->update( level.frametime );

	if ( multiplayerManager.inMultiplayer() && ( _lastPainShaderMod != MOD_NONE ) && ( _nextPainShaderTime < level.time ) )
	{
		str painShader = getPainShader( _lastPainShaderMod, false );

		clearCustomShader( "ArmorDeflection" );

		if ( painShader.length() > 0 )
		{
			clearCustomShader( painShader );

			_lastPainShaderMod = MOD_NONE;
		}
	}

	if ( needToSendAllHudsToClient() )
	{
		sendAllHudsToClient();
	}

	if( _needToSendBranchDialog == true && _started )
	{
		_needToSendBranchDialog = false;
		if(_branchDialogActor)
			_branchDialogActor->setBranchDialog();
	}

	_cameraCutThisFrame = false;
}


void Player::GibEvent( Event *ev )
{
	qboolean hidemodel;
	
	hidemodel = !ev->GetInteger( 1 );
	
	if ( com_blood->integer )
	{
		if ( hidemodel )
		{
			gibbed = true;
			takedamage = DAMAGE_NO;
			setSolidType( SOLID_NOT );
			hideModel();
		}
		
		CreateGibs( this, health, 0.75f, 3 );
	}
}

void Player::GotKill( Event * )
{
}

/* void Player::SetPowerupTimer
   (
   Event *ev
   )

   {
   Event *event;

   poweruptimer = ev->GetInteger( 1 );
   poweruptype  = (powerup_t)ev->GetInteger( 2 );
   event = new Event( EV_Player_UpdatePowerupTimer );
   PostEvent ( event, 1.0f );

	if ( p_heuristics )
		p_heuristics->IncrementItemsPickedUp();	
   } */

/* void Player::UpdatePowerupTimer
   (
   Event *ev
   )

   {
   poweruptimer -= 1;
   if ( poweruptimer > 0 )
      {
      Event *event = new Event( EV_Player_UpdatePowerupTimer );
		PostEvent ( event, 1.0f );
      }
   else
      {
		// Reset any flags as necessary, powerup is done
		if ( poweruptype == POWERUP_STEALTH && (flags & FL_NOTARGET) )
			flags ^= FL_NOTARGET;
		if ( poweruptype == POWERUP_PROTECTION && (flags & FL_GODMODE) )
			flags ^= FL_GODMODE;

		if ( poweruptype == POWERUP_ACCURACY )
			{
			Weapon *weap;
			weap = GetActiveWeapon( WEAPON_DUAL );
			// Tell the weapon we lost the accuracy powerup
			if ( weap )
				weap->SetAccuracyPowerup( false );
			}

      poweruptype = POWERUP_NONE;
      }
   } */

void Player::ChangeMusic( const char * current, const char * fallback, qboolean force )
{
	int current_mood_num;
	int fallback_mood_num;
	
	music_forced = force;
	if ( str( current ) == "normal" )
	{
		music_forced = false;
	}

	// We no longer let any music be forced
	//music_forced = false;
	
	// zero out action_level so that we do get a change
	//
	action_level = 0;
	
	if ( current )
	{
		current_mood_num = MusicMood_NameToNum( current );
		if ( current_mood_num < 0 )
		{
			gi.DPrintf( "current music mood %s not found", current );
		}
		else
		{
			music_current_mood = current_mood_num;
		}
	}
	
	if ( fallback )
	{
		fallback_mood_num = MusicMood_NameToNum( fallback );
		if ( fallback_mood_num < 0 )
		{
			gi.DPrintf( "fallback music mood %s not found", fallback );
			fallback = NULL;
		}
		else
		{
			music_fallback_mood = fallback_mood_num;
		}
	}
}

void Player::ChangeMusicVolume( float volume, float fade_time )
{
	music_volume_fade_time = fade_time;
	music_saved_volume     = music_current_volume;
	music_current_volume   = volume;
}

void Player::RestoreMusicVolume( float fade_time )
{
	music_volume_fade_time = fade_time;
	music_current_volume   = music_saved_volume;
	music_saved_volume     = -1.0f;
}

//----------------------------------------------------------------
// Name:			allowMusicDucking
// Class:			Player
//
// Description:		Specifies whether or not music ducking is allowed
//
// Parameters:		bool allowMusicDucking					- whether or not music ducking is allowed
//
// Returns:			none
//----------------------------------------------------------------

void Player::allowMusicDucking( bool allowMusicDucking )
{
	_allowMusicDucking = allowMusicDucking;
}

//----------------------------------------------------------------
// Name:			allowActionMusic
// Class:			Player
//
// Description:		Specifies whether or not action music is allowed
//
// Parameters:		bool allowActionMusic					- whether or not action music is allowed
//
// Returns:			none
//----------------------------------------------------------------

void Player::allowActionMusic( bool allowActionMusic )
{
	_allowActionMusic = allowActionMusic;
}

void Player::GiveOxygen( float time )
{
	air_finished = level.time + time;
}

void Player::Jump( Event *ev )
{
	float maxheight;
	
	maxheight = ev->GetFloat( 1 );
	
	if ( maxheight > 16.0f )
	{
		// v^2 = 2ad
		velocity[ 2 ] += sqrt( 2.0f * sv_currentGravity->integer * maxheight );
		
		// make sure the player leaves the ground
		client->ps.walking = false;
	}
}

void Player::JumpXY( Event *ev )
{
	float forwardmove;
	float sidemove;
	float distance;
	float time;
	float speed;
	
	forwardmove = ev->GetFloat( 1 );
	sidemove    = ev->GetFloat( 2 );
	speed       = ev->GetFloat( 3 );
	
	velocity = ( yaw_forward * forwardmove ) - ( yaw_left * sidemove );
	distance = velocity.length();
	velocity *= speed / distance;
	time = distance / speed;
	velocity[ 2 ] = sv_currentGravity->integer * time * 0.5f;
	
	airspeed = distance;
	
	// make sure the player leaves the ground
	client->ps.walking = false;
}

void Player::StartFakePlayer( void )
{
	Actor * fake;
	
	//
	// if we don't have a fakePlayer active, no need to check
	//
	if ( !fakePlayer_active )
	{
		return;
	}
	
	fakePlayer_active = false;
	
	fakePlayer = new Actor;
	
	if ( !fakePlayer )
		return;
	
	fake = fakePlayer;
	
	CloneEntity( fake, this );
	
	fake->SetTargetName( "fakeplayer" );
	fake->ProcessEvent( EV_Actor_AIOff );
	
	// make sure it thinks so that it can fall when necessary
	fake->turnThinkOn();
	
	// Make the fake player a stepsize shorter to prevent some collision issues
	
	fake->maxs[2] -= STEPSIZE;
	fake->setSize( mins, maxs );
	
	fake->takedamage = DAMAGE_NO;
	
	// hide the player
	this->hideModel();
	this->ProcessEvent( EV_Sentient_TurnOffShadow );
	//
	// immobolize the player
	//
	this->flags |= FL_IMMOBILE;
	// make the player not solid
	setSolidType( SOLID_NOT );
	
	// let the scripts now we are ready
	PostEvent( EV_Player_Done, 0.0f );
}

void Player::FakePlayer( qboolean holster )
{
	//
	// make sure we don't have one already
	//
	if ( fakePlayer )
	{
		return;
	}
	
	fakePlayer_active = true;
	
	// if we are in the holster state, wait until next frame
	// if we aren't process immediately
	if ( !holster )
	{
		StartFakePlayer();
	}
	else
	{
		if ( WeaponsOut() )
		{
			SafeHolster( true );
		}
	}
}

void Player::RemoveFakePlayer( void )
{
	Actor * fake;
	
	//
	// make sure we have one
	//
	if ( !fakePlayer )
	{
		return;
	}
	fake = fakePlayer;
	
	//
	// warp the real player to the fakeplayer location
	//
	this->setOrigin( fake->origin );
	this->setAngles( fake->angles );
	this->SetViewAngles( fake->angles );
	// show the player
	this->showModel();
	this->ProcessEvent( EV_Sentient_TurnOnShadow );
	// allow the player to move
	this->flags &= ~FL_IMMOBILE;
	// make the player solid
	setSolidType( SOLID_BBOX );
	// remove the fake
	fake->PostEvent ( EV_Remove, 0.f );
	// null out the fake player
	fakePlayer = NULL;
	SafeHolster( false );
}

void Player::SetViewAngles( Vector newViewangles )
{
	// set the delta angle
	client->ps.delta_angles[0] = ANGLE2SHORT( newViewangles.x - client->cmd_angles[0] );
	client->ps.delta_angles[1] = ANGLE2SHORT( newViewangles.y - client->cmd_angles[1] );
	client->ps.delta_angles[2] = ANGLE2SHORT( newViewangles.z - client->cmd_angles[2] );
	
	v_angle = newViewangles;
	
	// get the pitch and roll from our leg angles
	newViewangles.x = angles.x;
	newViewangles.z = angles.z;
	AnglesToMat( newViewangles, orientation );
	yaw_forward = orientation[ 0 ];
	yaw_left = orientation[ 1 ];
	
	//MatrixTransformVector( base_righthand_pos, orientation, righthand_pos );
	//MatrixTransformVector( base_lefthand_pos, orientation, lefthand_pos );
	MatrixTransformVector( base_rightfoot_pos, orientation, rightfoot_pos );
	MatrixTransformVector( base_leftfoot_pos, orientation, leftfoot_pos );
	//righthand_pos += origin;
	//lefthand_pos += origin;
	rightfoot_pos += origin;
	leftfoot_pos += origin;
}

void Player::DumpState( Event * )
{
	gi.DPrintf( "Legs: %s Torso: %s\n", currentState_Legs ? currentState_Legs->getName() : "NULL", currentState_Torso->getName() );
}

void Player::ForceTorsoState( Event *ev )
{
	State *ts = statemap_Torso->FindState( ev->GetString( 1 ) );
	EvaluateState( ts );
}

void Player::TouchedUseAnim( Entity * ent )
{
	toucheduseanim = ent;
}

void Player::ClearTarget( Event * )
{
	targetEnemy = NULL;
}

void Player::AdjustTorso( Event *ev )
{
	adjust_torso = ev->GetBoolean( 1 );
}

void Player::UseDualWield( Event * )
{
	// This is triggered by the state machine.
	// If there is a weapon in the dual wield list, use it, then remove it from the list.
	if ( dual_wield_weaponlist.NumObjects() )
	{
		WeaponSetItem *dw;
		
		dw = dual_wield_weaponlist.ObjectAt( 1 );
		
		useWeapon( dw->name, dw->hand );
		dual_wield_weaponlist.RemoveObjectAt( 1 );
		delete dw;
	}
	else
	{
		dual_wield_active = false;  // We are done wielding all the weapons
	}
}

void Player::DualWield( Event *ev )
{
	str      leftweap, rightweap;
	Weapon   *leftactweap, *rightactweap, *dualactweap;
	
	leftweap  = ev->GetString( 1 );
	rightweap = ev->GetString( 2 );
	
	// Set the putaway flags on any active weapons
	leftactweap  = GetActiveWeapon( WEAPON_LEFT );
	rightactweap = GetActiveWeapon( WEAPON_RIGHT );
	dualactweap  = GetActiveWeapon( WEAPON_DUAL );
	
	// Check for any dual handed weapon being out, and mark it for putaway
	if ( dualactweap )
	{
		dualactweap->SetPutAway( true );
	}
	
	// if the left and right weapons are already out, then holster them both
	if (
        ( leftactweap && !leftweap.icmp( leftactweap->getName() ) ) &&
        ( rightactweap && !rightweap.icmp( rightactweap->getName() ) )
		)
	{
		leftactweap->SetPutAway( true );
		rightactweap->SetPutAway( true );
		return;
	}
	
	WeaponSetItem *dualweap;
	
	// Putaway the old weapons, and add the new ones to the dual_wield list
	if ( !leftactweap )
	{
		dualweap = new WeaponSetItem;
		dualweap->name = leftweap;
		dualweap->hand = WEAPON_LEFT;
		dual_wield_weaponlist.AddObject( dualweap );
	}
	else if ( leftweap.icmp( leftactweap->getName() ) )
	{
		leftactweap->SetPutAway( true );
		
		dualweap = new WeaponSetItem;
		dualweap->name = leftweap;
		dualweap->hand = WEAPON_LEFT;
		dual_wield_weaponlist.AddObject( dualweap );
	}
	
	if ( !rightactweap )
	{
		dualweap = new WeaponSetItem;
		dualweap->name = rightweap;
		dualweap->hand = WEAPON_RIGHT;
		dual_wield_weaponlist.AddObject( dualweap );
	}
	else if ( rightweap.icmp( rightactweap->getName() ) )
	{
		rightactweap->SetPutAway( true );
		
		dualweap = new WeaponSetItem;
		dualweap->name = rightweap;
		dualweap->hand = WEAPON_RIGHT;
		dual_wield_weaponlist.AddObject( dualweap );
	}
	
	dual_wield_active = true;
}

void Player::EvaluateTorsoAnim( Event * )
{
	str torsoAnim( currentState_Torso->getTorsoAnim( *this, &torso_conditionals ) );
	if ( !animate->HasAnim(torsoAnim) )
		torsoAnim = getGameplayAnim(torsoAnim);
	
	if ( torsoAnim == "" )
	{
		partAnim[ torso ] = "";
		animate->ClearTorsoAnim();
	}
	else if ( torsoAnim != "none" )
	{
		SetAnim( torsoAnim.c_str(), torso );
	}
}

void Player::NextPainTime( Event *ev )
{
	nextpaintime = level.time + ev->GetFloat( 1 );
	pain_type    = MOD_NONE;
	pain         = 0;
}

void Player::SetMouthAngle( Event *ev )
{
	int tag_num;
	float angle_percent;
	Vector mouth_angles;
	
	
	angle_percent = ev->GetFloat( 1 );
	
	if ( angle_percent < 0.0f )
		angle_percent = 0.0f;
	
	if ( angle_percent > 1.0f )
		angle_percent = 1.0f;
	
	tag_num = gi.Tag_NumForName( edict->s.modelindex, "tag_mouth" );
	
	if ( tag_num != -1 )
	{
		SetControllerTag( MOUTH_TAG, tag_num );
		
		mouth_angles = vec_zero;
		mouth_angles[PITCH] = max_mouth_angle * angle_percent;
		
		SetControllerAngles( MOUTH_TAG, mouth_angles );
	}
}

void Player::EnterVehicle( Event *ev )
{
	Entity *ent;
	
	ent = ev->GetEntity( 1 );
	if ( ent && ent->isSubclassOf( Vehicle ) )
	{
		flags |= FL_PARTIAL_IMMOBILE;
		viewheight = STAND_EYE_HEIGHT;
		velocity = vec_zero;
		vehicle = ( Vehicle * )ent;
		if ( vehicle->IsDrivable() )
			setMoveType( MOVETYPE_VEHICLE );
		else
			setMoveType( MOVETYPE_NOCLIP );
	}
}

void Player::ExitVehicle( Event * )
{
	flags &= ~FL_PARTIAL_IMMOBILE;
	setMoveType( MOVETYPE_WALK );
	vehicle = NULL;
}

qboolean Player::WeaponsOut( void )
{
	return ( GetActiveWeapon( WEAPON_LEFT ) || GetActiveWeapon( WEAPON_RIGHT ) || GetActiveWeapon( WEAPON_DUAL ) );
}

qboolean Player::IsDualWeaponActive( void )
{
	if(GetActiveWeapon( WEAPON_LEFT ) || GetActiveWeapon( WEAPON_RIGHT ))
	{
		return qfalse;
	}
	
	return qtrue;
}


void Player::Holster( qboolean putaway )
{
//	if(client->ps.pm_flags & PMF_DISABLE_INVENTORY)
//		return;

	Weapon   *leftWeap, *rightWeap, *dualWeap;
	
	leftWeap  = GetActiveWeapon( WEAPON_LEFT );
	rightWeap = GetActiveWeapon( WEAPON_RIGHT );
	dualWeap  = GetActiveWeapon( WEAPON_DUAL );
	
	// Holster
	if ( leftWeap || rightWeap || dualWeap )
	{
		if ( putaway )
		{
			if ( leftWeap )
            {
				leftWeap->SetPutAway( true );
				holsteredWeapons[WEAPON_LEFT] = leftWeap;
            }
			if ( rightWeap )
            {
				rightWeap->SetPutAway( true );
				holsteredWeapons[WEAPON_RIGHT] = rightWeap;
            }
			if ( dualWeap )
            {
				dualWeap->SetPutAway( true );
				holsteredWeapons[WEAPON_DUAL] = dualWeap;
            }
			
			// Set a level var so the script can know if the player is going to holster
			levelVars.SetVariable( "holster_active", 1 );
		}
	}
	else
	{
		if ( !putaway )
		{
			// Unholster
			if ( holsteredWeapons[WEAPON_DUAL] )
            {
				useWeapon( holsteredWeapons[WEAPON_DUAL], WEAPON_DUAL );
            }
			else if ( holsteredWeapons[WEAPON_LEFT] && holsteredWeapons[WEAPON_RIGHT] )
            {
				Event *ev1;
				
				ev1 = new Event( EV_Player_DualWield );
				ev1->AddString( holsteredWeapons[WEAPON_LEFT]->getName() );
				ev1->AddString( holsteredWeapons[WEAPON_RIGHT]->getName() );
				ProcessEvent( ev1 );
            }
			else if ( holsteredWeapons[WEAPON_RIGHT] )
            {
				useWeapon( holsteredWeapons[WEAPON_RIGHT], WEAPON_RIGHT );
            }
			else if ( holsteredWeapons[WEAPON_LEFT] )
            {
				useWeapon( holsteredWeapons[WEAPON_LEFT], WEAPON_LEFT );
            }
			
			holsteredWeapons[WEAPON_LEFT]  = NULL;
			holsteredWeapons[WEAPON_RIGHT] = NULL;
			holsteredWeapons[WEAPON_DUAL]  = NULL;
			// Set a level var to let the script know there is no holstering
			levelVars.SetVariable( "holster_active", 0 );
		}
	}
}

void Player::SafeHolster( qboolean putaway )
{
	if ( WeaponsOut() )
	{
		if ( putaway )
		{
			weapons_holstered_by_code = true;
			Holster( true );
		}
	}
	else
	{
		if ( putaway )
		{
			if ( !fakePlayer_active )
            {
				WeaponsHolstered();
            }
		}
		else
		{
			if ( weapons_holstered_by_code )
            {
				weapons_holstered_by_code = false;
				Holster( false );
            }
		}
	}
}

void Player::WeaponsNotHolstered( void )
{
}

void Player::WeaponsHolstered( void )
{
}

void Player::setTargeted( bool targeted )
{
	if ( targeted )
		{
		gi.SendServerCommand( entnum, "stufftext \"ui_addhud targetedhud\"\n");
		}
	else
		{
		gi.SendServerCommand( entnum, "stufftext \"ui_removehud targetedhud\"\n");
		}
}

void Player::NightvisionToggle( Event * )
{
	if ( level.cinematic )
		return;

	if ( ( multiplayerManager.isPlayerSpectator( this ) ) && ( client->ps.pm_flags ^ PMF_NIGHTVISION ) )
		return;

	client->ps.pm_flags ^= PMF_NIGHTVISION;

	Event *newEvent;
	newEvent = new Event( EV_Sentient_SetViewMode );
	if (client->ps.pm_flags & PMF_NIGHTVISION) 
		newEvent->AddString("nightvision");
	else
		newEvent->AddString("normal");
	ProcessEvent( newEvent );
}

void Player::HolsterToggle( Event * )
{
	if ( WeaponsOut() )
	{
		Holster( true );
	}
	else
	{
		Holster( false );
	}
}

void Player::Holster( Event *ev )
{

	SafeHolster( ev->GetBoolean( 1 ) );
}

void Player::IncreaseActionLevel( float action_level_increase )
{
	action_level += action_level_increase;
}

void Player::WatchEntity( Event *ev )
{
	if ( camera || ( currentState_Torso->getCameraType() != CAMERA_BEHIND ) )
		return;

	watchEntityForEntireDuration = false;

	if ( ev->NumArgs() > 1 )
	{
		Event *stopWatchingEvent = new Event( EV_Player_StopWatchingEntity );
		stopWatchingEvent->AddEntity( ev->GetEntity( 1 ) );
		const float timeToWatch = ev->GetFloat( 2 );
		PostEvent( stopWatchingEvent, timeToWatch );

		maximumAngleToWatchedEntity = ev->GetFloat( 3 );

		if ( ev->NumArgs() > 3 )
		{
			watchEntityForEntireDuration = ev->GetBoolean( 4 );
		}

	}

	entity_to_watch = ev->GetEntity( 1 );
}

void Player::StopWatchingEntity( Event * )
{
	StopWatchingEntity();
}

void Player::setAngles( const Vector &ang )
{
	Vector new_ang;
	
	new_ang = ang;
	
	// set the angles normally
	Entity::setAngles( new_ang );
	
	// set the orientation based off of the current view, also update our yaw_forward and yaw_left
	new_ang[ YAW ] = v_angle[ YAW ];
	AnglesToMat( new_ang, orientation );
	yaw_forward = orientation[ 0 ];
	yaw_left = orientation[ 1 ];
}

void Player::WeaponCommand( Event *ev )
{
	weaponhand_t   hand;
	Weapon         *weap;
	int            i;
	
	if ( ev->NumArgs() < 2 )
		return;
	
	hand = WeaponHandNameToNum( ev->GetString( 1 ) );
	weap = GetActiveWeapon( hand );
	
	if ( !weap )
		return;
	
	Event *e;
	e = new Event( ev->GetToken( 2 ) );
	
	for( i=3; i<=ev->NumArgs(); i++ )
		e->AddToken( ev->GetToken( i ) );
	
	weap->ProcessEvent( e );
}

qboolean TryPush( int entnum, vec3_t move_origin, vec3_t move_end )
{
	Actor *act;
	Vector dir;
	Vector dir2;
	Entity *ent;
	
	if ( entnum == ENTITYNUM_NONE )
		return false;
	
	ent = G_GetEntity( entnum );
	
	if ( ent->isSubclassOf( Actor ) )
	{
		act = (Actor *) ent;
		
		dir = act->origin - move_origin;
		dir.z = 0.0f;
		dir.normalize();
		
		dir2 = move_end;
		dir2 -= move_origin;
		
		if ( act->flags & FL_FLY )
		{
			dir *= dir2.length() / 2.0f;
			
			if ( act->movementSubsystem->Push( dir ) )
				return true;
		}
		else
		{
			dir *= dir2.length();
			
			Event *event = new Event( EV_Actor_Push );
			event->AddVector( dir );
			act->PostEvent( event, 0.0f );
		}
	}
	
	return false;
}

void Player::PlayerDone( Event * )
{
	// This is used to let scripts know that the player is done doing something
	
	// let any threads waiting on us know they can go ahead
	Director.PlayerSpawned();
}

painDirection_t Player::Pain_string_to_int( const str &pain )
{
	if ( !pain.icmp( pain, "Front" ) )
		return PAIN_FRONT;
	else if ( !pain.icmp( pain, "Left" ) )
		return PAIN_LEFT;
	else if ( !pain.icmp( pain, "Right" ) )
		return PAIN_RIGHT;
	else if ( !pain.icmp( pain, "Rear" ) )
		return PAIN_REAR;
	else
		return PAIN_NONE;
}

void Player::ArchivePersistantData( Archiver &arc, qboolean sublevelTransition )
{
	int i;
	str model_name;
	bool anglesArchived;
	
	Sentient::ArchivePersistantData( arc, sublevelTransition );
	
	model_name = g_playermodel->string;
	
	arc.ArchiveString( &model_name );
	
	if ( arc.Loading() )
	{
		// set the cvar
		gi.cvar_set( "g_playermodel", model_name.c_str() );
		
		model_name += ".tik";
		setModel( model_name.c_str() );
	}
	
	for( i = 0; i < MAX_ACTIVE_WEAPONS; i++ )
	{
		str name;
		if ( arc.Saving() )
		{
			if ( holsteredWeapons[ i ] )
            {
				name = holsteredWeapons[ i ]->getName();
            }
			else
            {
				name = "none";
            }
		}
		arc.ArchiveString( &name );
		if ( arc.Loading() )
		{
			if ( name != "none" )
            {
				holsteredWeapons[ i ] = ( Weapon * )FindItem( name );
            }
		}
	}
	
	if ( arc.Saving() )
	{
		if ( sublevelTransition && level._saveOrientation )
		{
			anglesArchived = true;
			arc.ArchiveBool( &anglesArchived );
			arc.ArchiveVector( &angles );
			arc.ArchiveVector( &v_angle );
		}
		else
		{
			anglesArchived = false;
			arc.ArchiveBool( &anglesArchived );
		}
	}
	else
	{
		arc.ArchiveBool( &anglesArchived );
		
		if ( anglesArchived )
		{
			arc.ArchiveVector( &angles );
			arc.ArchiveVector( &v_angle );
			
			setAngles( angles );
			SetViewAngles( v_angle );
		}
	}
	
	
	// Force a re-evaluation of the player's state
	LoadStateTable();
	
	arc.ArchiveInteger(&_totalGameFrames);
	_updateGameFrames = true;

	arc.ArchiveInteger( &_secretsFound );

	arc.ArchiveInteger( &_skillLevel );

	arc.ArchiveRaw( client->ps.stats, sizeof( client->ps.stats ) );
}

const str Player::getPainShader( meansOfDeath_t mod, bool takeArmorIntoAccount )
{
	if ( ( takeArmorIntoAccount ) && ( GetArmorValue() >= 100 ) )
		return "ArmorDeflection";
	else
		return getPainShader( MOD_NumToName( mod )  );
}

const str Player::getPainShader( const char *MODName )
{
	GameplayManager *gpm;
	str modName;
	const char *defaultName = "default";

	gpm = GameplayManager::getTheGameplayManager();

	if ( !gpm )
		return str("");

	modName = "MOD";
	modName += MODName;

	if ( !gpm->hasObject( modName ) )
	{
		if ( stricmp( MODName, defaultName ) == 0 )
			return "";
		else
			return getPainShader( defaultName );
	}

	return gpm->getStringValue( modName, "PainShaderName" );

	/* switch ( mod )
	{
	case MOD_NONE:
		return "none";
		break;
	default:
		return "electriclines";
		break;
	} */
}

void Player::SpawnDamageEffect( meansOfDeath_t mod )
{
	/* switch ( mod )
	{
	case MOD_ELECTRIC:
		// lint -fallthrough
	case MOD_ELECTRICWATER:
		//SpawnEffect( "fx_elecstrike.tik", origin );
		//Sound( "sound/weapons/sword/electric/hitmix2.wav", 0 );
		
		break;
		
	default:
		break;
	} */

	if ( multiplayerManager.inMultiplayer() )
	{
		if ( ( mod != MOD_NONE ) && ( mod != MOD_DEATH_QUAD ) && ( _nextPainShaderTime < level.time ) && !hasCustomShader() )
		{
			str painShader = getPainShader( mod, true );

			if ( painShader.length() > 0 )
			{
				setCustomShader( painShader );

				_lastPainShaderMod  = mod;
				_nextPainShaderTime = level.time + 0.25f;
			}
		}
	}
}

void Player::ActivateDualWeapons( Event * )
{
	int i;
	
	Weapon *weapon = 0;
	for ( i=dual_wield_weaponlist.NumObjects(); i>=1; i-- )
	{
		WeaponSetItem   *dw;
		
		dw = dual_wield_weaponlist.ObjectAt( i );
		
		weapon = ( Weapon * )FindItem( dw->name, weapon );
		
		// Check to see if player has the weapon
		if ( !weapon )
		{
			warning( "Player::ActivateDualWeapons", "Player does not have weapon %s", dw->name.c_str() );
			return;
		}
		
		ChangeWeapon( weapon, dw->hand );
		dual_wield_weaponlist.RemoveObjectAt( i );
		delete dw;
	}
	
	// Clear out the newActiveWeapon
	ClearNewActiveWeapon();
	
	// Clear out the holstered weapons
	holsteredWeapons[WEAPON_LEFT]  = NULL;
	holsteredWeapons[WEAPON_RIGHT] = NULL;
	holsteredWeapons[WEAPON_DUAL]  = NULL;
	
	// let the player know that our weapons are not holstered
	WeaponsNotHolstered();
}

void Player::VelocityModified( void )
{
	if ( velocity.z > 32.0f )
	{
		do_rise = true;
	}
}

int Player::GetKnockback( int original_knockback, qboolean blocked )
{
	int new_knockback;
	
	new_knockback = original_knockback;
	
	// If blocked, absorb some of the knockback
	
	if ( blocked )
	{
		if ( LargeShieldActive() )
			new_knockback -= 150;
		else
			new_knockback -= 50;
	}
	
	// See if we still have enough knockback to knock the player down
	
	if ( ( new_knockback >= 200.0f ) && take_pain )
	{
		knockdown = true;
		
		if ( blocked )
		{
			float damage;
			
			damage = new_knockback / 50;
			
			if ( damage > 10.0f )
				damage = 10.0f;
			
			Damage( world, world, damage, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_CRUSH );
		}
	}
	
	// Make sure knockback is still at least 0
	
	if ( new_knockback < 0 )
		new_knockback = 0;
	
	return new_knockback;
}

void Player::ResetHaveItem( Event *ev )
{
	str fullname;
	ScriptVariable * var;
	
	fullname = str( "playeritem_" ) + ev->GetString( 1 );
	
	var = gameVars.GetVariable( fullname.c_str() );
	
	if ( var )
		var->setIntValue( 0 );
}

void Player::ReceivedItem( Item * item )
{
	qboolean forced;
	qboolean first;
	str fullname;
	str dialog;
	str anim;
	ScriptVariable * var;
	
	//
	// set our global game variables
	//
	
	if ( item->isSubclassOf( Weapon ) )
	{
		setItemText( item->getIcon(), va( "$$PickedUpThe$$ $$Weapon-%s$$\n", item->getName().c_str() ) );
		//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$PickedUpThe$$ $$%s$$\n", item->getName() );
	}
	else if ( item->getAmount() > 1 )
	{
		if ( item->getName() == "health" )  // I know this is horrible :(
			setItemText( item->getIcon(), va( "$$PickedUp$$ %d $$Item-%s$$\n", (int)item->getAmount(), item->getName().c_str() ) );
			//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$PickedUp$$ %d %s\n", (int)item->getAmount(), item->getName() );
		else
			setItemText( item->getIcon(), va( "$$PickedUp$$ %d $$Item-%s$$s\n", (int)item->getAmount(), item->getName().c_str() ) );
			//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$PickedUp$$ %d %ss\n", (int)item->getAmount(), item->getName() );
	}
	else
	{
		setItemText( item->getIcon(), va( "$$PickedUpThe$$ $$Item-%s$$\n", item->getName().c_str() ) );
		//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$PickedUpThe$$ %s\n", item->getName() );
	}
	
	fullname = str( "playeritem_" ) + item->getName();
	
	first = true;
	
	var = gameVars.GetVariable( fullname.c_str() );
	if ( !var )
	{
		gameVars.SetVariable( fullname.c_str(), 1 );
	}
	else
	{
		int amount;
		
		amount = var->intValue() + 1;
		var->setIntValue( amount );
		//
		// if we just received it, let the player know
		//
		if ( amount > 1 )
		{
			first = false;
		}
	}
	
	var = levelVars.GetVariable( fullname.c_str() );
	if ( !var )
	{
		levelVars.SetVariable( fullname.c_str(), 1 );
	}
	else
	{
		int amount;
		
		amount = var->intValue() + 1;
		var->setIntValue( amount );
	}
	
	if ( item->IsItemCool( &dialog, &anim, &forced ) )
	{
		if ( first || forced )
		{
			cool_item = item;
			cool_dialog = dialog;
			cool_anim = anim;
		}
	}
}

void Player::RemovedItem( Item * item )
{
	str fullname;
	ScriptVariable * var;
	
	//
	// set our global game variables if client
	//
	
	fullname = str( "playeritem_" ) + item->getName();
	
	var = levelVars.GetVariable( fullname.c_str() );
	if ( var )
	{
		int amount;
		
		amount = var->intValue() - 1;
		if ( amount < 0 )
			amount = 0;
		var->setIntValue( amount );
	}
	
	var = gameVars.GetVariable( fullname.c_str() );
	if ( var )
	{
		int amount;
		
		amount = var->intValue() - 1;
		if ( amount < 0 )
			amount = 0;
		var->setIntValue( amount );
	}
}

void Player::AmmoAmountChanged( Ammo * ammo, int ammo_in_clip )
{
	str fullname;
	ScriptVariable * var;
	
	//
	// set our level variables
	//
	fullname = str( "playerammo_" ) + ammo->getName();
	
	var = levelVars.GetVariable( fullname.c_str() );
	if ( !var )
	{
		levelVars.SetVariable( fullname.c_str(), ammo->getAmount() + ammo_in_clip );
	}
	else
	{
		var->setIntValue( ammo->getAmount() + ammo_in_clip );
	}
}

void Player::StartCoolItem( Event * )
{
	// turn off ai off during the cinematic
	level.ai_on = false;
	// make sure we don't take damage during this time
	takedamage = DAMAGE_NO;
	// freeze the player
	level.playerfrozen = true;
	// turn on cinematic mode
	G_StartCinematic();
	
	assert( ( Camera * )cool_camera == NULL );
	// start playing the success music
	if ( music_current_mood != mood_success )
	{
		ChangeMusic( "success", MusicMood_NumToName( music_current_mood ), false );
	}
	
	// create an orbit cam
	cool_camera = new Camera();
	cool_camera->SetOrbitHeight( 150.0f );
	cool_camera->Orbit( this, 200.0f, this, -90.0f );
	cool_camera->Cut( NULL );
	SetCamera( cool_camera, 1.0f );
}

void Player::ShowCoolItem( Event * )
{
	Entity *fx;
	Vector org;
	
	org = origin;
	org.z += 128.0f;
	
	fx = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
	
	fx->setOrigin( org );
	fx->setModel( "fx_coolitem.tik" );
	fx->animate->RandomAnimate( "idle" );
	fx->PostEvent( EV_Remove, 1.0f );
	
	if ( cool_item )
	{
		cool_item->setOrigin( org );
		cool_item->PostEvent( EV_Show, 0.1f );
		// place a lens flare on the object
		cool_item->edict->s.renderfx |= RF_VIEWLENSFLARE;
		
		if ( cool_dialog.length() )
		{
			Sound( cool_dialog, CHAN_DIALOG );
		}
	}
}

void Player::HideCoolItem( Event * )
{
	Entity *fx;
	Vector org;
	
	org = origin;
	org.z += 128.0f;
	
	fx = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
	
	fx->setOrigin( org );
	fx->setModel( "fx_coolitem_reverse.tik" );
	fx->animate->RandomAnimate( "idle" );
	fx->PostEvent( EV_Remove, 1.0f );
	
	if ( cool_item )
	{
		Event * event;
		
		cool_item->PostEvent( EV_Hide, 1.0f );
		
		event = new Event( EV_SetOrigin );
		event->AddVector( vec_zero );
		cool_item->PostEvent( event, 1.0f );
		// remove the lens flare on the object
		cool_item->edict->s.renderfx &= ~RF_VIEWLENSFLARE;
	}
}

void Player::StartCoolItemAnim( void )
{
	movecontrol = MOVECONTROL_ABSOLUTE;
	
	if ( cool_item && cool_anim.length() )
	{
		SetAnim( cool_anim, legs );
		// clear out anim till next time
		cool_anim = "";
	}
}

void Player::StopCoolItem( Event * )
{
	if ( cool_item && cool_anim.length() )
	{
		State * newState;
		
		newState = statemap_Torso->FindState( "DO_COOL_ITEM_ANIM" );
		if ( newState )
		{
			currentState_Torso = newState;
			return;
		}
	}
	
	// turn ai back on
	level.ai_on = true;
	
	// turn damage back on
	takedamage = DAMAGE_AIM;
	
	// unfreeze the player
	level.playerfrozen = false;
	
	// turn off cinematic mode
	G_StopCinematic();
	
	cool_item = NULL;
	
	// delete our camera
	if ( cool_camera )
	{
		SetCamera( NULL, 1.0f );
		delete cool_camera;
		cool_camera = NULL;
	}
}

void Player::WaitForState( Event *ev )
{
	waitForState = ev->GetString( 1 );
}


void Player::SetDamageMultiplier( Event *ev )
{
	damage_multiplier = ev->GetFloat( 1 );
}

void Player::SetTakePain( Event *ev )
{
	take_pain = ev->GetBoolean( 1 );
}

void Player::Loaded( void )
{
}

void Player::PlayerShowModel( Event * )
{
	Entity::showModel();
}

void Player::showModel( void )
{
	Entity::showModel();
}

void Player::WarpToPoint( const Entity *spawnpoint )
{
	if ( !spawnpoint )
		return;
	
	NoLerpThisFrame();
	setOrigin( spawnpoint->origin + Vector( "0 0 1" ) );
	origin.copyTo( edict->s.origin2 );
	setAngles( spawnpoint->angles );
	SetViewAngles( angles );
	CameraCut();

	oldvelocity = vec_zero;
	velocity    = vec_zero;
}

void Player::Gib( void )
{
	/* str gib_name;
	int number_of_gibs;
	float scale;
	Entity *ent;
	str real_gib_name;
	
	if ( !com_blood->integer )
		return;
	
	gib_name       = "fx_rgib";
	number_of_gibs = 5;
	scale          = 1.3;
	
	// Spawn the gibs
	real_gib_name = gib_name;
	real_gib_name += number_of_gibs;
	real_gib_name += ".tik";
	
	ent = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
	ent->setModel( real_gib_name.c_str() );
	ent->setScale( scale );
	ent->setOrigin( centroid );
	ent->animate->RandomAnimate( "idle" );
	ent->PostEvent( EV_Remove, 1.0f );
	
	this->hideModel();
	Sound( "snd_decap", CHAN_BODY, 1.0f, 300.0f );
	gibbed = true; */
}

void Player::ArmorDamage( Event *ev )
{
	float oldHealth;

	::Damage damage(ev);
	
	// Protect the player from errant damage before fighting
	if ( multiplayerManager.inMultiplayer() && !multiplayerManager.isFightingAllowed() )
		return;

	if ( multiplayerManager.inMultiplayer() && multiplayerManager.isPlayerSpectator( this ) )
		return;
	
	// Quick dirty hack to do no damage when you have the shield up.
	if ( shield_active )
	{
		// Code to be implemented soon.
		Weapon *weapon = GetActiveWeapon(WEAPON_RIGHT);
		if ( !weapon )
			return;
		
		Vector attack_angle;
		float yaw_diff;
		attack_angle = damage.attacker->angles;
		yaw_diff = angles[YAW] - attack_angle[YAW] + 180.0f;
		yaw_diff = AngleNormalize180( yaw_diff );
		if ( ( yaw_diff > -45.0f ) && ( yaw_diff < 45.0f ) )
		{
			int tagnum = 0;
			tagnum = gi.Tag_NumForName( weapon->edict->s.modelindex, "tag_swipe1" );
			if ( tagnum >= 0)
			{
				Vector pos, pos2, sparkpos;
				weapon->GetActorMuzzlePosition(&pos, NULL, NULL, NULL, "tag_swipe1");
				weapon->GetActorMuzzlePosition(&pos2, NULL, NULL, NULL, "tag_swipe2");
				sparkpos = (pos + pos2) / 2.0f; // Spark is halfway between the two points
				WeaponEffectsAndSound( weapon, "Parry", sparkpos );
			}
			
			if ( damage.attacker->isSubclassOf( Sentient ) )
			{
				Sentient *sent = ( Sentient * )damage.attacker;
				sent->SetAttackBlocked( true );
			}
			
			//attack_blocked = true;
			return;
		}
	}
	
	if ( _powerup )
		damage.damage = _powerup->getDamageTaken( damage.attacker, damage.damage, damage.meansofdeath );
	
	if ( _rune )
		damage.damage = _rune->getDamageTaken( damage.attacker, damage.damage, damage.meansofdeath );
	
	if ( multiplayerManager.inMultiplayer() && damage.attacker->isSubclassOf( Player ) )
	{
		damage.damage = multiplayerManager.playerDamaged( this, (Player *)damage.attacker, damage.damage, damage.meansofdeath );
		
		damage.knockback = (int)multiplayerManager.getModifiedKnockback( this, (Player *)damage.attacker, damage.knockback );
	}
	
	if ( !multiplayerManager.inMultiplayer() && ( damage.meansofdeath != MOD_FALLING ) )
	{
		GameplayManager *gpm;
		float damageMultiplier;
		int skillLevel;
		
		skillLevel = getSkill();

		gpm = GameplayManager::getTheGameplayManager();
		
		if ( gpm->hasObject( "SkillLevel-PlayerDamage" ) )
		{
			if ( skillLevel == 0 )
				damageMultiplier = gpm->getFloatValue( "SkillLevel-PlayerDamage", "Easy" );
			else if ( skillLevel == 1 )
				damageMultiplier = gpm->getFloatValue( "SkillLevel-PlayerDamage", "Normal" );
			else if ( skillLevel == 2 )
				damageMultiplier = gpm->getFloatValue( "SkillLevel-PlayerDamage", "Hard" );
			else
				damageMultiplier = gpm->getFloatValue( "SkillLevel-PlayerDamage", "VeryHard" );
			
			
			damage.damage *= damageMultiplier;
		}
	}

	oldHealth = health;
	
	Sentient::ArmorDamage(damage);

	if ( multiplayerManager.inMultiplayer() )
	{
		float damageTaken;

		damageTaken = oldHealth - health;

		if ( damageTaken > 0.0f )
		{
			// Increase victim's action level

			if ( damage.meansofdeath > MOD_LAST_SELF_INFLICTED )
			{
				IncreaseActionLevel( damageTaken );
			}

			if ( damage.attacker->isSubclassOf( Player ) )
			{
				Player *attackingPlayer = (Player *)damage.attacker;

				// Tell the multiplayer system that the player took damage

				multiplayerManager.playerTookDamage( this, attackingPlayer, damageTaken, damage.meansofdeath );

				// Increase attacker's action level

				if ( attackingPlayer != this )
				{
					attackingPlayer->IncreaseActionLevel( damageTaken );
				}
			}
		}
	}
	
	// If we're dead, go ahead and gib completely on Projectiles
	if ( ( multiplayerManager.inMultiplayer() ) && ( health <= 0.0f ) && damage.inflictor->isSubclassOf( Projectile ) )
	{
		Gib();
	}
}

void Player::DeadBody( Event * )
{
	// Spawn a dead body at the spot
	Body *body;
	int surfaceNum;
	
	if ( ( pain_type == MOD_VAPORIZE ) || ( pain_type == MOD_VAPORIZE_COMP ) || 
		( pain_type == MOD_VAPORIZE_DISRUPTOR ) || ( pain_type == MOD_VAPORIZE_PHOTON ) || ( pain_type == MOD_SNIPER ))
		return;
	
	body = new Body;
	
	if ( gibbed )
		return;
	
	body->setModel( this->model );
	body->ProcessInitCommands( body->edict->s.modelindex );
	body->edict->s.anim        = this->edict->s.anim;
	body->edict->s.frame       = this->edict->s.frame;
	
	//body->edict->s.torso_anim  = this->edict->s.anim;
	//body->edict->s.torso_frame = this->edict->s.frame;
	
	body->edict->s.torso_anim  = this->edict->s.torso_anim;
	body->edict->s.torso_frame = this->edict->s.torso_frame;
	body->edict->s.scale       = this->edict->s.scale;
	
	body->setOrigin( this->origin );
	body->setAngles( this->angles );

	// Copy over all of the surface data from the player to the body

	for( surfaceNum = 0 ; surfaceNum < numsurfaces ; surfaceNum++ )
	{
		body->edict->s.surfaces[ surfaceNum ] = edict->s.surfaces[ surfaceNum ];
	}
}

void Player::ShowHeuristics( Event * )
{
	
	if ( p_heuristics )
		p_heuristics->ShowHeuristics( this );
}

void Player::FireWeapon( Event *ev )
{	
	Sentient::FireWeapon(ev);
}

void Player::ReleaseFireWeapon( Event *ev )
{
	Sentient::ReleaseFireWeapon(ev);
}

void Player::SetAimType( Event *ev )
{
	Weapon* weapon = 0;
	weapon = GetActiveWeapon( WEAPON_DUAL );
	if (!weapon )
		return;
	
	// Forward the event to the weapon itself
	weapon->SetAimType( ev );
}

void Player::ReloadWeapon( Event * )
{
	Weapon* weapon = 0;
	weapon = GetActiveWeapon( WEAPON_DUAL );
	if (!weapon )
		return;
	
	// Reload
	weapon->ForceReload();
}

void Player::AnimateWeapon( Event *ev )
{
	Weapon* weapon = 0;
	weaponhand_t hand = WEAPON_DUAL;
	bool animatingFlag = true;
	
	if ( ev->NumArgs() > 1 )
		hand = WeaponHandNameToNum(ev->GetString( 2 ));
	
	if ( ev->NumArgs() > 2 )
		animatingFlag = ev->GetBoolean( 3 );
	
	weapon = GetActiveWeapon( hand );
	if ( !weapon )
		return;
	
	weapon->playAnim( ev->GetString( 1 ), animatingFlag );
}

void Player::SwitchWeaponMode( Event * )
{
	Weapon* weapon = 0;
	weapon = GetActiveWeapon( WEAPON_DUAL );
	if (!weapon )
		return;
	
	// Switch Modes
	weapon->SwitchMode();
}

qboolean Player::GetCrouch( void )
{
	if ( last_ucmd.upmove < 0 ) // check for downward movement
		return true;
	return false;
}
	
void Player::ReloadTiki( Event *ev )
{
	if ( ev->NumArgs() < 1 )
		return;
	
	int frame, anim, animstate;
	Viewthing *viewthing;
	
	viewthing = ( Viewthing * )( ( Entity * )Viewmodel.current_viewthing );
	if ( !viewthing )
		return;
	
	// Save off info about the current viewspawn
	frame = viewthing->frame;
	anim = viewthing->CurrentAnim();
	Vector vieworigin(viewthing->origin.x, viewthing->origin.y, viewthing->origin.z);
	Vector viewangles(viewthing->angles.x, viewthing->angles.y, viewthing->angles.z);
	
	// We're actually going to call ToggleAnimationState, so we
	// set the animstate here one less than the real state
	animstate = viewthing->animstate-1;
	if ( animstate < 0 ) 
		animstate = -1;
	
	// Process the event that deletes the old viewmodel and spawns the new one
	Event *ev2 = new Event(EV_ViewThing_SpawnFromTS);
	ev2->AddString(ev->GetString(1));
	ev2->AddString(viewthing->model);
	Viewmodel.ProcessEvent(ev2);
	
	// Re-get the new viewthing pointer
	viewthing = ( Viewthing * )( ( Entity * )Viewmodel.current_viewthing );
	
	// Update all its info to be the same to the one we deleted
	ev2 = new Event;
	ev2->AddFloat(vieworigin.x);
	ev2->AddFloat(vieworigin.y);
	ev2->AddFloat(vieworigin.z);		
	viewthing->ChangeOrigin(ev2);
	if ( ev2 ) delete ev2;
	
	ev2 = new Event;
	ev2->AddFloat(viewangles.x);
	ev2->AddFloat(viewangles.y);
	ev2->AddFloat(viewangles.z);		
	viewthing->SetAnglesEvent(ev2);
	
	viewthing->frame = frame;
	viewthing->SetAnim(anim);
	viewthing->animstate = animstate;
	if ( ev2 ) delete ev2;
	ev2 = new Event;
	viewthing->ToggleAnimateEvent(ev2);
}

void Player::SetViewAnglesEvent( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		SetViewAngles(ev->GetVector(1));
}

void Player::ProjDetonate(Event *ev)
{
	if ( ev->NumArgs() > 0 )
		projdetonate = ev->GetBoolean(1);
}

void Player::SetProjDetonate(qboolean value)
{
	projdetonate = value;
}

qboolean Player::GetProjDetonate()
{
	Weapon *weapon;

	if ( projdetonate )
		return true;

	weapon = GetActiveWeapon( WEAPON_DUAL );

	if ( !weapon )
		return false;

	if ( ( weapon->GetFireType( FIRE_MODE1 ) == FT_TRIGGER_PROJECTILE ) && ( isButtonDown( BUTTON_ATTACKLEFT ) ) )
		return true;

	if ( ( weapon->GetFireType( FIRE_MODE2 ) == FT_TRIGGER_PROJECTILE ) && ( isButtonDown( BUTTON_ATTACKRIGHT ) ) )
		return true;

	return false;
}

void Player::PassEventToVehicle( Event *ev )
{
	if ( vehicle )
		vehicle->HandleEvent( ev );
}

void Player::UseSpecifiedEntity( Event *ev )
{
	Event  *event = NULL;
	Entity *ent = NULL;
	
	// Event with a param is old functionality
	if ( ev->NumArgs() > 0 )
	{
		ent = ev->GetEntity( 1 );
		if ( ent == 0 )
		{
			Com_Error ( ERR_DROP , "UseSpecifiedEntity(): NULL entity referenced\n" );
			return;
		}

		event = new Event( EV_Use );
		event->AddEntity( this );
		ent->ProcessEvent( event );
		return;
	}
	else // No params, new functionality
	{
		if ( !atobject )
			return;
		ent = (Entity *)atobject;
		if ( !ent->hasUseData() )
			return;
		
		ent->useData->useMe();
		
		if ( ent->useData->getUseAnim().length() > 0 && animate->HasAnim(ent->useData->getUseAnim()) )
		{
			// We are assuming we have a valid anim that
			// will manually trigger the use on a specific frame
			// using the "douseentity" call	
			
			SetAnim( ent->useData->getUseAnim(), legs );
			movecontrol = MOVECONTROL_ABSOLUTE;
			_usingEntity = true;
			_useEntityStartTimer = level.time + 2.5f;
		}
		else
		{
			// No animation, just call the thread and notify the entity
			// he's been used.
			if ( ent->useData->getUseThread().length() > 0 )
				ExecuteThread(ent->useData->getUseThread().c_str(), true, ent);
			
			_usingEntity = false; // No anim, so we can just leave the state immediately.
			
			// If it's an item, we're going to pick it up, so no need to call
			// it's use event.  Check the gameplay manager to make sure this item
			// cannot be auto-picked up.
			GameplayManager *gpm = GameplayManager::getTheGameplayManager();
			if ( ent->isSubclassOf(Item) && gpm->hasProperty(ent->getArchetype(), "noautopickup"))
			{
				Item *item = (Item *)ent;
				handlePickupItem(item);
				return;
			}
			
			event = new Event( EV_Use );
			event->AddEntity( this );
			ent->ProcessEvent( event );
		}
	}
}

//--------------------------------------------------------------
//
// Name:			handlePickupItem
// Class:			Player
//
// Description:		Called when the user has clicked on a useentity that
//					happens to be an item. The default behavior of this
//					action is to put the item in the inventory and remove
//					it from the world.
//
// Parameters:		Item *item -- The item to pick up
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::handlePickupItem( Item *item )
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	str type = item->getArchetype();
	str invslot = gpm->getStringValue(type, "invslot");
	if ( invslot.length() )
	{
		float quantity = gpm->getFloatValue(invslot, "quantity");
		quantity += 1.0f;
		gpm->setFloatValue(invslot, "quantity", quantity);
		gpm->setStringValue(invslot, "name", type);
	}
	else
	{
		str slotName = getFreeInventorySlot();
		if ( !slotName.length() )
			return;
		
		gpm->setStringValue(slotName, "name", type);
	}
	
	str snd = gpm->getStringValue(type + ".Pickup", "wav");
	if ( snd.length() )
	{
		int channel = CHAN_BODY;
		float volume = -1.0f;
		float mindist = -1.0f;
		if ( gpm->hasProperty(type + ".Pickup","channel") )
			channel = (int)gpm->getFloatValue(type + ".Pickup", "channel");
		if ( gpm->hasProperty(getArchetype() + ".Pickup","volume") )
			volume = (int)gpm->getFloatValue(getArchetype() + ".Pickup", "volume");
		if ( gpm->hasProperty(getArchetype() + ".Pickup","mindist") )
			mindist = (int)gpm->getFloatValue(getArchetype() + ".Pickup", "mindist");
		item->Sound(snd, channel, volume, mindist);
	}
	
	// Remove the item from the world
	item->ProcessEvent(EV_Remove);
}

//--------------------------------------------------------------
//
// Name:			doUseEntity
// Class:			Player
//
// Description:		Called from the tiki to do the actual useentity
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::doUseEntity( Event * )
{
	Entity *ent = NULL;
	Event *event = NULL;
	
	if ( !atobject )
		return;
	
	ent = (Entity *)atobject;
	if ( !ent->hasUseData() )
		return;
	
	if ( ent->useData->getUseThread().length() > 0 )
		ExecuteThread(ent->useData->getUseThread().c_str(), true, ent);
	
	// If it's an item, we're going to pick it up, so no need to call
	// it's use event.  Check the gameplay manager to make sure this item
	// cannot be auto-picked up.
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( ent->isSubclassOf(Item) && gpm->hasProperty(ent->getArchetype(), "noautopickup"))
	{
		Item *item = (Item *)ent;
		handlePickupItem(item);
		return;
	}
	
	event = new Event( EV_Use );
	event->AddEntity( this );
	ent->ProcessEvent( event );
}

//--------------------------------------------------------------
//
// Name:			doneUseEntity
// Class:			Player
//
// Description:		Called when the useentity animation is done
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::doneUseEntity( Event * )
{
	_usingEntity = false;
	level.playerfrozen = false;
	_useEntityStartTimer = 0.0f;
	SetState("STAND", "STAND");
}

void Player::SetupDialog( Event *ev )
{
	Entity *entity;
	str soundName;
	
	entity = ev->GetEntity( 1 );
	soundName = ev->GetString( 2 );
	
	SetupDialog( entity, soundName );
}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::SetupDialog( Entity *entity, const str &soundName )
{
	if( gi.SoundLength( soundName.c_str() ) <= 0 )
		return;
	
	//If we have an entity, then we are dealing with Dialog events.
	if ( entity )
	{
		handleDialogSetup(entity, soundName);
	}
	else
	{
		handleTextDialogSetup( soundName );
	}
}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::handleDialogSetup( Entity* entity, const str& soundName )
{
	// Set the correct info and post the clear event when done
	// Make sure all current clears are canceled
	CancelEventsOfType( EV_Player_ClearDialog );
	
	// Make sure current stuff is cleared properly
	
	ClearDialog();

	if ( gi.SoundLength( soundName.c_str() ) >= 0 )
	{
		_dialogEntnum = entity->entnum;
		_dialogSoundIndex = gi.soundindex( soundName.c_str() );

		PostEvent( EV_Player_ClearDialog, gi.SoundLength( soundName.c_str() ) );
	}
	else
	{
		ProcessEvent( EV_Player_ClearDialog );
	}
}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::handleTextDialogSetup( const str& soundName )
{
	// Set the correct info and post the clear event when done
	// Make sure all current clears are canceled
	CancelEventsOfType( EV_Player_ClearTextDialog );
	
	// Make sure current stuff is cleared properly
	ClearTextDialog();
	
	if ( gi.SoundLength( soundName.c_str() ) >= 0 )
	{
		_dialogTextSoundIndex = gi.soundindex( soundName.c_str() );
		PostEvent( EV_Player_ClearTextDialog, gi.SoundLength( soundName.c_str() ) );
	}
	else
	{
		ProcessEvent( EV_Player_ClearTextDialog );
	}
}


void Player::ClearDialog( Event * )
{
	ClearDialog();
}

void Player::ClearDialog( void )
{
	// Clear the dialog info
	_dialogEntnum = ENTITYNUM_NONE;
	_dialogSoundIndex = -1;
}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::ClearTextDialog( Event*  )
{
	ClearTextDialog();
}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::ClearTextDialog( void )
{
	_dialogTextSoundIndex = -1;
}


//
// Objective Functions
//
void Player::SetObjectiveComplete( Event* ev )
{
	int ObjIndex;
	str ObjName;
	qboolean Complete;
	
	ObjName = ev->GetString( 1 );
	Complete = ev->GetBoolean( 2 );
	
	ObjIndex = gi.MObjective_GetIndexFromName( ObjName.c_str() );   
	if ( ObjIndex == 0 )
		return;
	
	switch ( ObjIndex )
	{
	case OBJECTIVE1:
		if ( Complete )
            _objectiveStates |= OBJECTIVE1_COMPLETE;
		else
            _objectiveStates &= ~(OBJECTIVE1_COMPLETE);
		break;
		
	case OBJECTIVE2:
		if ( Complete )
            _objectiveStates |= OBJECTIVE2_COMPLETE;
		else
            _objectiveStates &= ~(OBJECTIVE2_COMPLETE);
		break;
		
	case OBJECTIVE3:
		if ( Complete )
            _objectiveStates |= OBJECTIVE3_COMPLETE;
		else
            _objectiveStates &= ~(OBJECTIVE3_COMPLETE);
		
		break;
		
	case OBJECTIVE4:
		if ( Complete )
            _objectiveStates |= OBJECTIVE4_COMPLETE;
		else
            _objectiveStates &= ~(OBJECTIVE4_COMPLETE);
		
		break;
		
	case OBJECTIVE5:
		if ( Complete )
            _objectiveStates |= OBJECTIVE5_COMPLETE;
		else
            _objectiveStates &= ~(OBJECTIVE5_COMPLETE);
		
		break;
		
	case OBJECTIVE6:
		if ( Complete )
            _objectiveStates |= OBJECTIVE6_COMPLETE;
		else
            _objectiveStates &= ~(OBJECTIVE6_COMPLETE);
		
		break;
		
	case OBJECTIVE7:
		if ( Complete )
            _objectiveStates |= OBJECTIVE7_COMPLETE;
		else
            _objectiveStates &= ~(OBJECTIVE7_COMPLETE);
		
		break;
		
	case OBJECTIVE8:
		if ( Complete )
            _objectiveStates |= OBJECTIVE8_COMPLETE;
		else
            _objectiveStates &= ~(OBJECTIVE8_COMPLETE);
		
		break;
		
	default:
		break;
	}
	
	
	//   gi.MObjective_SetObjectiveComplete( ObjName.c_str() , Complete );         
}



void Player::SetObjectiveFailed( Event *ev )
{
	int ObjIndex;
	str ObjName;
	qboolean Failed;
	
	ObjName = ev->GetString( 1 );
	Failed = ev->GetBoolean( 2 );
	
	ObjIndex = gi.MObjective_GetIndexFromName( ObjName.c_str() );   
	if ( ObjIndex == 0 )
		return;
	
	switch ( ObjIndex )
	{
	case OBJECTIVE1:
		if ( Failed )
            _objectiveStates |= OBJECTIVE1_FAILED;
		else
            _objectiveStates &= ~(OBJECTIVE1_FAILED);
		break;
		
	case OBJECTIVE2:
		if ( Failed )
            _objectiveStates |= OBJECTIVE2_FAILED;
		else
            _objectiveStates &= ~(OBJECTIVE2_FAILED);
		
		break;
		
	case OBJECTIVE3:
		if ( Failed )
            _objectiveStates |= OBJECTIVE3_FAILED;
		else
			_objectiveStates &= ~(OBJECTIVE3_FAILED);
		
		break;
		
	case OBJECTIVE4:
		if ( Failed )
            _objectiveStates |= OBJECTIVE4_FAILED;
		else
            _objectiveStates &= ~(OBJECTIVE4_FAILED);
		
		break;
		
	case OBJECTIVE5:
		if ( Failed )
            _objectiveStates |= OBJECTIVE5_FAILED;
		else
            _objectiveStates &= ~(OBJECTIVE5_FAILED);
		
		break;
		
	case OBJECTIVE6:
		if ( Failed )
            _objectiveStates |= OBJECTIVE6_FAILED;
		else
            _objectiveStates &= ~(OBJECTIVE6_FAILED);
		
		break;
		
	case OBJECTIVE7:
		if ( Failed )
            _objectiveStates |= OBJECTIVE7_FAILED;
		else
            _objectiveStates &= ~(OBJECTIVE7_FAILED);
		
		
	case OBJECTIVE8:
		if ( Failed )
            _objectiveStates |= OBJECTIVE8_FAILED;
		else
            _objectiveStates &= ~(OBJECTIVE8_FAILED);
		
		break;
		
	default:
		break;
	}
	
	// gi.MObjective_SetObjectiveFailed( ObjName.c_str() , Failed );   
}


//-----------------------------------------------------
//
// Name:		LoadObjectives
// Class:		Player
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::LoadObjectives(Event* ev)
{
	loadObjectives(ev->GetString(1));
}


//-----------------------------------------------------
//
// Name:		loadObjectives
// Class:		Player
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::loadObjectives( const str& objectiveName )
{
	_objectiveStates   = 0;
	_informationStates = 0;
	gi.MObjective_Update(objectiveName);

	_objectiveNameIndex = gi.objectivenameindex(objectiveName);
}


//-----------------------------------------------------
//
// Name:		SetObjectiveShow
// Class:		Player
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::SetObjectiveShow( Event* ev )
{
	int ObjIndex;
	str ObjName;
	qboolean Show;
	bool playSound;

	playSound = false;
	
	ObjName = ev->GetString( 1 );
	Show = ev->GetBoolean( 2 );
	
	
	ObjIndex = gi.MObjective_GetIndexFromName( ObjName.c_str() );
	if ( ObjIndex == 0 )
		return;

	//Update the objective show flag.
	gi.MObjective_SetShowObjective(ObjName.c_str(), Show);


	//Set the appropriate bit on the flag passed to the client.
	unsigned int bitToChange;

	switch ( ObjIndex )
	{
	case OBJECTIVE1:
		bitToChange = OBJECTIVE1_SHOW;
		break;
	case OBJECTIVE2:
		bitToChange = OBJECTIVE2_SHOW;
		break;
	case OBJECTIVE3:
		bitToChange = OBJECTIVE3_SHOW;
		break;
	case OBJECTIVE4:
		bitToChange = OBJECTIVE4_SHOW;
		break;
	case OBJECTIVE5:
		bitToChange = OBJECTIVE5_SHOW;
		break;
	case OBJECTIVE6:
		bitToChange = OBJECTIVE6_SHOW;
		break;
	case OBJECTIVE7:
		bitToChange = OBJECTIVE7_SHOW;
		break;
	case OBJECTIVE8:
		bitToChange = OBJECTIVE8_SHOW;
		break;
	default:
		bitToChange = 0;
		break;
	}

	if ( Show && !(_objectiveStates & bitToChange) )
	{			
		_objectiveStates |= bitToChange;		
		playSound = true;
	}

	// Else is removed because if we call setobjectiveshow on the same objective twice, we will
	// actually hide it
	//else
	//{
    //    _objectiveStates &= ~bitToChange;
	//}

	if ( playSound )
		Sound( "snd_objectivechanged", CHAN_LOCAL );
}

void Player::SpecialMoveChargeStart( Event* )
{
	specialMoveCharge = level.time;
	specialMoveEndTime = level.time + specialMoveChargeTime;
}

void Player::SpecialMoveChargeEnd( Event* )
{
	specialMoveCharge = 0.0f;
	specialMoveChargeTime = 0.0f;
	specialMoveEndTime = 0.0f;
}

void Player::SpecialMoveChargeTime( Event* ev )
{
	if ( ev->NumArgs() > 0 )
		specialMoveChargeTime = ev->GetFloat( 1 );
}

void Player::SetInformationShow( Event* ev )
{
	int InfoIndex;
	str InfoName;
	qboolean Show;
	
	InfoName = ev->GetString( 1 );
	Show = ev->GetBoolean( 2 );
	
	InfoIndex = gi.MI_GetIndexFromName( InfoName.c_str() );
	if ( InfoIndex == 0 )
		return;
	
	
	switch ( InfoIndex )
	{
	case INFORMATION1:
		if ( Show )
            _informationStates |= INFORMATION1_SHOW;
		else
            _informationStates &= ~(INFORMATION1_SHOW);
		break;
		
	case INFORMATION2:
		if ( Show )
            _informationStates |= INFORMATION2_SHOW;
		else
            _informationStates &= ~(INFORMATION2_SHOW);
		
		break;
		
	case INFORMATION3:
		if ( Show )
            _informationStates |= INFORMATION3_SHOW;
		else
            _informationStates &= ~(INFORMATION3_SHOW);
		
		break;
		
	case INFORMATION4:
		if ( Show )
            _informationStates |= INFORMATION4_SHOW;
		else
            _informationStates &= ~(INFORMATION4_SHOW);
		
		break;
		
	case INFORMATION5:
		if ( Show )
            _informationStates |= INFORMATION5_SHOW;
		else
            _informationStates &= ~(INFORMATION5_SHOW);
		
		break;
		
	case INFORMATION6:
		if ( Show )
            _informationStates |= INFORMATION6_SHOW;
		else
            _informationStates &= ~(INFORMATION6_SHOW);
		
		break;
		
	case INFORMATION7:
		if ( Show )
            _informationStates |= INFORMATION7_SHOW;
		else
            _informationStates &= ~(INFORMATION7_SHOW);
		
		
	case INFORMATION8:
		if ( Show )
            _informationStates |= INFORMATION8_SHOW;
		else
            _informationStates &= ~(INFORMATION8_SHOW);
		
		break;
		
	default:
		break;
	}
}

void Player::MissionFailed( Event* ev )
{
	str reason = "DefaultFailure";
	if ( ev->NumArgs() > 0 )
		reason = ev->GetString( 1 );

	G_MissionFailed(reason);
}

void Player::setMissionFailed( void )
{
	client->ps.missionStatus |= MISSION_FAILED;
}

void Player::SetStat( Event *ev )
{
	int stat_index;
	int stat_value;
	
	stat_index = PlayerStat_NameToNum( ev->GetString( 1 ) );
	
	if ( stat_index < 0 )
	{
		gi.Printf( "Couldn't find player stat %s\n", ev->GetString( 1 ) );
		return;
	}
	
	stat_value = ev->GetInteger( 2 );
	
	client->ps.stats[ stat_index ] = stat_value;
}

void Player::SetStateFile( Event *ev )
{
	str stateFileName(ev->GetString(1));
	gi.cvar_set( "g_statefile", stateFileName );
	LoadStateTable();
}

//----------------------------------------------------------------
// Name:			ShouldSendToClient
// Class:			Player
//
// Description:		Decides whether or not we should send this entity to the client
//
// Parameters:		Entity *entityToSend						- entity that we deciding about
//
// Returns:			qboolean										- whether or not we should send this entity
//----------------------------------------------------------------

qboolean Player::ShouldSendToClient( Entity *entityToSend )
{
	// For now, early out if we don't have care about any view modes

	if ( multiplayerManager.inMultiplayer() && client->pers.mp_lowBandwidth && entityToSend->isNetworkDetail() )
		return false;

	if ( !entityToSend->_affectingViewModes )
		return true;


	// Check to see if we should send this entity based on the player's current view mode
	
	if ( entityToSend->_affectingViewModes & _viewMode )
		return gi.GetViewModeSendInMode( entityToSend->_affectingViewModes & _viewMode );
	else
		return gi.GetViewModeSendNotInMode( entityToSend->_affectingViewModes & (~_viewMode) );
}

void Player::UpdateEntityStateForClient( entityState_t *state )
{
	int i;

	if ( !state )
		return;

	// Only update entity in multiplayer

	if ( !multiplayerManager.inMultiplayer() )
		return;

	// Only update entity in low bandwidth mode

	if ( !client->pers.mp_lowBandwidth )
		return;

	// Clear net angles if a client

	if ( state->clientNum != ENTITYNUM_NONE )
	{
		VectorClear( state->netangles );
	}

	// Clear bone angles

	for ( i = 0 ; i < NUM_BONE_CONTROLLERS ; i++ )
	{
		VectorClear( state->bone_angles[ i ] );
	}
}

void Player::UpdatePlayerStateForClient( playerState_t *state )
{
	if ( !state )
		return;

	// Only update playerstate in multiplayer

	if ( !multiplayerManager.inMultiplayer() )
		return;

	if ( !multiplayerManager.isPlayerSpectator( this, SPECTATOR_TYPE_FOLLOW ) && !mp_savingDemo )
	{
		VectorClear( state->viewangles );
	}
}

//----------------------------------------------------------------
// Name:			ExtraEntitiesToSendToClient
// Class:			Player
//
// Description:		Adds extra entities to send over to the client
//
// Parameters:		int *numExtraEntities					- the number of extra entities we added
//					int *extraEntities						- the list of entity numbers that we added
//
// Returns:			None
//----------------------------------------------------------------

void Player::ExtraEntitiesToSendToClient( int *numExtraEntities, int *extraEntities )
{
	*numExtraEntities = 0;
	
	// Add in our current dialog person (if any)
	if ( _dialogEntnum != ENTITYNUM_NONE )
	{
		extraEntities[ *numExtraEntities ] = _dialogEntnum;
		(*numExtraEntities)++;
		
		// Make sure we haven't added too many entities to the list
		
		if ( *numExtraEntities == MAX_EXTRA_ENTITIES_FROM_GAME )
			return;
	}
}

//----------------------------------------------------------------
// Name:			setViewMode
// Class:			Player
//
// Description:		Sets the players current view mode
//
// Parameters:		const str &viewModeName					- the name of the view mode to go to
//
// Returns:			None
//----------------------------------------------------------------

void Player::setViewMode( const str &viewModeName )
{
	Sentient::setViewMode( viewModeName );
	client->ps.viewMode = getViewMode();
}

//----------------------------------------------------------------
// Name:			AwardPoints
// Class:			Player
//
// Description:		Gives the player points
//
// Parameters:		int numPoints -- the number of points to give
//
// Returns:			int -- The new total number of points
//----------------------------------------------------------------
int Player::AwardPoints(int numPoints)
{
	points += numPoints;
	
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm ) return points ;
	
	float maxPoints		= 0.0f ;
	float experience	= 0.0f ;
	
	if ( gpm->hasProperty( "CurrentPlayer", "maxPoints" ) )
	{
		maxPoints  = gpm->getFloatValue( "CurrentPlayer", "maxPoints" );
		experience = 100.0f * (points / maxPoints) ;
		gpm->setFloatValue( "CurrentPlayer", "experience", experience );
		
		if ( experience >= 100.0f )
		{
			float level			= gpm->getFloatValue( "CurrentPlayer", "level" );
			float skillPoints	= gpm->getFloatValue( "CurrentPlayer", "SkillPoints" );
			
			level		+= 1.0 ;
			skillPoints += level ;
			maxPoints	 = level * level * 100.0f ;
			experience	 = 0.0f ;
			
			gpm->setFloatValue( "CurrentPlayer", "level",		level );
			gpm->setFloatValue( "CurrentPlayer", "SkillPoints",	skillPoints );
			gpm->setFloatValue( "CurrentPlayer", "maxPoints",	maxPoints );
			gpm->setFloatValue( "CurrentPlayer", "experience",	experience );
			points = 0 ;
			
			G_EnableWidgetOfPlayer( edict, "level_up", true );
		}
	}
	
	return points;
}

//----------------------------------------------------------------
// Name:			TakePoints
// Class:			Player
//
// Description:		Takes points away (never below 0)
//
// Parameters:		int numPoints -- the number of points to take
//
// Returns:			int -- The new total number of points
//----------------------------------------------------------------
int Player::TakePoints(int numPoints)
{
	points -= numPoints;
	if ( points < 0 )
		points = 0;
	
	return points;
}

//----------------------------------------------------------------
// Name:			GivePointsEvent
// Class:			Player
//
// Description:		Gives the player points (from script)
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::GivePointsEvent( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		AwardPoints(ev->GetInteger( 1 ));
}

//----------------------------------------------------------------
// Name:			SetPointsEvent
// Class:			Player
//
// Description:		Sets the players total number of points
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::SetPointsEvent( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		points = ev->GetInteger( 1 );
}

//----------------------------------------------------------------
// Name:			ChangeChar
// Class:			Player
//
// Description:		Starts the screen fading out.  When it fades to black
//					we switch characters.
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::ChangeChar( Event *ev )
{
	if ( changingChar ) // In the process of changing don't allow this event AGAIN until we're done.
		return;
	
	// Don't switch characters if we're already that character.
	str curChar = gi.cvar("g_playermodel","",0)->string;
	str curStateFile = gi.cvar("g_statefile","",0)->string;
	if ((ev->GetString( 1 ) == curStateFile) && (ev->GetString( 2 ) == curChar))
		return;
	
	Actor *act = Actor::FindActorByName(ev->GetString( 3 ));
	if ( act )
	{
		changingChar = true;
		G_FadeOut(1.0f);
		Event *ev2 = new Event(EV_Player_ChangeCharFadeIn);
		ev2->AddString(ev->GetString( 1 ));
		ev2->AddString(ev->GetString( 2 ));
		ev2->AddString(ev->GetString( 3 ));
		ev2->AddString(ev->GetString( 4 ));
		ev2->AddEntity(act);
		PostEvent(ev2, 1.0f);
	}
}

//----------------------------------------------------------------
// Name:			ChangeCharFadeIn
// Class:			Player
//
// Description:		Changes to the specified state machine and model
//					as the screen fades in.
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::ChangeCharFadeIn( Event *ev )
{
	str statemachine = ev->GetString( 1 );
	str newmodel = ev->GetString( 2 );
	str replacemodel = ev->GetString( 3 );
	str spawnNPC = ev->GetString( 4 );
	Actor *act = (Actor*)ev->GetEntity( 5 );

	ProcessEvent(EV_DetachAllChildren);

	gi.cvar_set( "g_statefile", statemachine.c_str() );
	gi.cvar_set( "g_playermodel", newmodel.c_str() );
	InitModel();
	LoadStateTable();

	Event *e = new Event( EV_Player_SpawnActor );
	e->AddString( spawnNPC.c_str() );
	e->AddString( "origin" );
	e->AddVector(origin);
	e->AddString( "angles" );
	e->AddVector(angles);
	// Spawn the actor where the player is currently positioned
	SpawnActor(e);

	// Set the player angles and origin to the new location
	origin = act->origin;
	SetViewAngles(act->angles);
	act->ProcessEvent(  EV_Remove );

	G_AutoFadeIn();
	changingChar = false;
}

//----------------------------------------------------------------
// Name:			SetPlayerChar
// Class:			Player
//
// Description:		Sets the player character
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::SetPlayerChar( Event *ev )
{
	str statemachine = ev->GetString( 1 );
	str newmodel = ev->GetString( 2 );
	
	// Don't switch characters if we're already that character.
	str curChar = gi.cvar("g_playermodel","",0)->string;
	str curStateFile = gi.cvar("g_statefile","",0)->string;
	if ((statemachine == curStateFile) && (newmodel == curChar))
		return;
	
	gi.cvar_set( "g_statefile", statemachine.c_str() );
	gi.cvar_set( "g_playermodel", newmodel.c_str() );
	ProcessEvent(EV_DetachAllChildren);
	InitModel();
	LoadStateTable();
}

//----------------------------------------------------------------
// Name:			PlayerKnockback
// Class:			Player
//
// Description:		Sets the player knockback value.
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::PlayerKnockback(Event *ev)
{
	playerKnockback = ev->GetInteger( 1 );
}

//----------------------------------------------------------------
// Name:			KnockbackMultiplier
// Class:			Player
//
// Description:		Sets the player knockback multiplier
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::KnockbackMultiplier(Event *ev)
{
	knockbackMultiplier = ev->GetInteger( 1 );
}

//----------------------------------------------------------------
// Name:			MeleeEvent
// Class:			Player
//
// Description:		Performs a weaponless melee attack.
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::MeleeEvent( Event *ev )
{
	Vector melee_pos;
	Vector melee_end;
	float damage = 20;
	str means_of_death_string;
	meansOfDeath_t attack_means_of_death;
	float knockback;
	
	// Get all of the parameters
	
	if ( ev->NumArgs() > 0 )
		damage = ev->GetFloat( 1 );
	
	if ( ev->NumArgs() > 1 )
		means_of_death_string = ev->GetString( 2 );
	
	if ( ev->NumArgs() > 2 )
		knockback = ev->GetFloat( 3 );
	else
		knockback = damage * 8.0f;
	
	melee_pos = centroid;
	melee_end = centroid + Vector( orientation[0] ) * 96.0f;
	
	if ( means_of_death_string.length() > 0 )
		attack_means_of_death = (meansOfDeath_t)MOD_NameToNum( means_of_death_string );
	else
		attack_means_of_death = MOD_CRUSH;
	
	// Do the actual attack
	MeleeAttack( melee_pos, melee_end, damage, this, attack_means_of_death, 15.0f, -45.0f, 45.0f, knockback );
}

//----------------------------------------------------------------
// Name:			MeleeDamageStart
// Class:			Player
//
// Description:		Sets flags to perform melee damage with the weapon
//					in the hand specified (defaults to right hand)
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::MeleeDamageStart( Event *ev )
{
	Weapon *weapon;
	weaponhand_t hand = WEAPON_RIGHT;
	
	if ( ev->NumArgs() > 0 )
		hand = WeaponHandNameToNum(ev->GetString( 1 ));
	
	weapon = GetActiveWeapon( hand );
	if ( !weapon )
		return;
	
	// Make sure the tags are there on the weapon, or it will crash.
	int tag_num = 0;
	tag_num += gi.Tag_NumForName( weapon->edict->s.modelindex, "tag_swipe1" );
	tag_num += gi.Tag_NumForName( weapon->edict->s.modelindex, "tag_swipe2" );
	if ( tag_num < 0 )
		return;
	
	weapon->ClearMeleeVictims();
	if ( hand == WEAPON_RIGHT )
		meleeAttackFlags |= MELEE_ATTACK_RIGHT;
	if ( hand == WEAPON_LEFT )
		meleeAttackFlags |= MELEE_ATTACK_LEFT;
}

//----------------------------------------------------------------
// Name:			MeleeDamageEnd
// Class:			Player
//
// Description:		Stops doing damage with the melee weapon in the hand
//					specified.
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::MeleeDamageEnd( Event *ev )
{	
	weaponhand_t hand = WEAPON_RIGHT;
	
	if ( ev->NumArgs() > 0 )
		hand = WeaponHandNameToNum(ev->GetString( 1 ));
	
	if ( hand == WEAPON_RIGHT )
		meleeAttackFlags &= ~MELEE_ATTACK_RIGHT;
	if ( hand == WEAPON_LEFT )
		meleeAttackFlags &= ~MELEE_ATTACK_LEFT;
}

//----------------------------------------------------------------
// Name:			ChangeStance
// Class:			Player
//
// Description:		Changes to the stance specified
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::ChangeStance(Event *ev)
{
	if ( ev->NumArgs() < 1 )
		return;
	
	changedStanceTorso = true;
	changedStanceLegs = true;
	stanceNumber = ev->GetInteger( 1 );
}

//----------------------------------------------------------------
// Name:			ClearStanceTorso
// Class:			Player
//
// Description:		Clears internal torso stance data
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::ClearStanceTorso(Event *)
{
	changedStanceTorso = false;
}

//----------------------------------------------------------------
// Name:			ClearStanceLegs
// Class:			Player
//
// Description:		Clears internal legs stance data
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::ClearStanceLegs(Event *)
{
	changedStanceLegs = false;
}

//----------------------------------------------------------------
// Name:			ClearIncomingMelee
// Class:			Player
//
// Description:		Clears the incoming melee flag
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::ClearIncomingMelee(Event *)
{
	incomingMeleeAttack = false;
}

//----------------------------------------------------------------
// Name:			AddMeleeAttacker
// Class:			Player
//
// Description:		Adds the entity to the melee attacker container
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::AddMeleeAttacker( Event *ev)
{
	Entity *ent = ev->GetEntity( 1 );
	meleeAttackerList.AddUniqueObject( ent );
}

//----------------------------------------------------------------
// Name:			SetBendTorso
// Class:			Player
//
// Description:		Sets the torso bending for the player
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::SetBendTorso(Event *ev)
{
	if ( ev->NumArgs() > 0 )
		bendTorsoMult = ev->GetFloat( 1 );
	else
		bendTorsoMult = standardTorsoMult;
}

//----------------------------------------------------------------
// Name:			HeadWatchAllowed
// Class:			Player
//
// Description:		Sets whether to headwatch or not, defaults to true
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Player::HeadWatchAllowed(Event *ev)
{
	Sentient::HeadWatchAllowed( ev );
}

void Player::SetCurrentCallVolume( const str &volumeName )	
{
    currentCallVolume = volumeName;
}
	
str	Player::GetCurrentCallVolume()
{
	return currentCallVolume;
}

//
// Multiplayer stuff
//

void Player::Score( Event * )
{
	multiplayerManager.score( this );
}

void Player::joinTeam( Event *ev )
{
	str teamName;

	teamName = ev->GetString( 1 );

	multiplayerManager.joinTeam( this, teamName );
}

void Player::multiplayerCommand( Event *ev )
{
	str command;
	str parm;

	command = ev->GetString( 1 );
	parm    = ev->GetString( 2 );

	multiplayerManager.playerCommand( this, command, parm );
}

void Player::Disconnect( void )
{
	if ( multiplayerManager.inMultiplayer() )
	{
		multiplayerManager.removePlayer( this );
	}
}

void Player::CallVote( Event *ev )
{
	str command;
	str arg;

	if ( multiplayerManager.inMultiplayer() )
	{
		command = ev->GetString( 1 );

		if ( ev->NumArgs() > 1 )
			arg = ev->GetString( 2 );

		multiplayerManager.callVote( this, command, arg );
	}
}

void Player::Vote( Event *ev )
{
	str vote;

	if ( multiplayerManager.inMultiplayer() )
	{
		if ( ev->NumArgs() != 1 )
		{
			multiplayerManager.HUDPrint( entnum, "$$Usage$$: vote <1|0|y|n>" );
			return;
		}

		vote = ev->GetString( 1 );

		multiplayerManager.vote( this, vote );
	}
}

//
// Powerup stuff
//

void Player::addPowerupEffect( PowerupBase *powerup )
{
	str modelName;
	str tagName;
	float modelRemoveTime;
	str shaderName;

	if ( !powerup )
		return;

	// Attach a model to the player if necessary

	powerup->getModelToAttachOnUse( modelName, tagName, modelRemoveTime );

	if ( modelName.length() > 0 )
	{
		Event *attachModel;

		attachModel = new Event( EV_AttachModel );

		attachModel->AddString( modelName );
		attachModel->AddString( tagName );
		attachModel->AddFloat( 1.0f );
		attachModel->AddString( "" );
		attachModel->AddInteger( 0 );
		attachModel->AddFloat( modelRemoveTime );

		ProcessEvent( attachModel );
	}
}

void Player::removePowerupEffect( PowerupBase *powerup )
{
	str modelName;
	str tagName;
	float modelRemoveTime;
	str shaderName;

	if ( !powerup )
		return;

	// Remove the attached model from the player if attached before

	powerup->getModelToAttachOnUse( modelName, tagName, modelRemoveTime );

	if ( modelName.length() > 0 )
	{
		Event *removeAttachedModel;

		removeAttachedModel = new Event( EV_RemoveAttachedModel );
		
		removeAttachedModel->AddString( tagName );
		removeAttachedModel->AddFloat( 0.0f );
		removeAttachedModel->AddString( modelName );

		ProcessEvent( removeAttachedModel );
	}

	// Remove the custom shader from the player if set before

	powerup->getShaderToDisplayOnUse( shaderName );

	if ( shaderName.length() > 0 )
	{
		clearCustomShader( shaderName.c_str() );
	}
}

void Player::setPowerup( Powerup *powerup )
{
	// If this is the same powerup type, stack them

	if ( _powerup && powerup && ( _powerup->getName() == powerup->getName() ) && _powerup->canStack() && powerup->canStack() )
	{
		float newTimeLeft;
		newTimeLeft = _powerup->getTimeLeft() + powerup->getTimeLeft();
		_powerup->setTimeLeft( newTimeLeft );
	}
	else
	{
		removePowerup();

		CancelEventsOfType( EV_Player_RemovePowerup );

		_powerup = powerup;

		if ( !_powerup )
			return;

		addPowerupEffect( _powerup );
	}

	setItemText( _powerup->getIcon(), va( "$$Using$$ $$Item-%s$$", _powerup->getName().c_str() ) );
	//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$Using$$ %s", _powerup->getRealName() );
}

void Player::removePowerup( void )
{
	if ( _powerup )
	{
		removePowerupEffect( _powerup );

		delete _powerup;
		_powerup = NULL;
	}
}

void Player::removePowerupEvent( Event * )
{
	removePowerup();
}

void Player::dropPowerup( void )
{
	if ( _powerup )
	{
		if ( _powerup->canDrop() )
		{
			_powerup->spawn( centroid );
		}

		removePowerup();
	}
}

//
// Rune stuff
//

bool Player::hasRune( void )
{
	if ( _rune )
		return true;
	else
		return false;
}

void Player::setRune( Rune *rune )
{
	removeRune();

	_rune = rune;

	if ( !_rune )
		return;

	addPowerupEffect( _rune );

	// Tell the player they are now using this rune

	setItemText( rune->getIcon(), va( "$$Using$$ $$Item-%s$$", _rune->getName().c_str() ) );
	//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$Using$$ %s", _rune->getRealName() );
}

void Player::removeRune( void )
{
	if ( _rune )
	{
		removePowerupEffect( _rune );

		delete _rune;
		_rune = NULL;
	}
}

//
// Holdable item stuff
//

void Player::setHoldableItem( HoldableItem *holdableItem )
{
	removeHoldableItem();

	_holdableItem = holdableItem;
}

void Player::removeHoldableItem( void )
{
	if ( _holdableItem )
	{
		_holdableItem->PostEvent( EV_Remove, 0.0f );

		_holdableItem = NULL;
	}
}

void Player::useHoldableItem( void )
{
	if ( _holdableItem )
	{
		if ( multiplayerManager.inMultiplayer() )
		{
			multiplayerManager.playerEventNotification( "use-HoldableItem", _holdableItem->getName(), this );
		}

		if ( _holdableItem->use() )
		{
			// Must check again if holdable item exists, because use might have caused the holdable item to be destroyed

			if ( _holdableItem )
			{
				addPowerupEffect( _holdableItem );

				setItemText( _holdableItem->getIcon(), va( "$$Used$$ $$Item-%s$$", _holdableItem->getName().c_str() ) );
				//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$Using$$ $$Item-%s$$", _holdableItem->getName() );
				removeHoldableItem();
			}
		}
	}
}

HoldableItem* Player::getHoldableItem( void )
{
	return _holdableItem;
}

float Player::getDamageDone( float damage, int meansOfDeath, bool inMelee )
{
	float damageDone;


	damageDone = damage;

	if ( inMelee )
		damageDone *= damage_multiplier;

	if ( _powerup )
		damageDone = _powerup->getDamageDone( damageDone, meansOfDeath );

	if ( _rune )
		damageDone = _rune->getDamageDone( damageDone, meansOfDeath );
	
	return damageDone;
}

meansOfDeath_t Player::changetMeansOfDeath( meansOfDeath_t meansOfDeath )
{
	meansOfDeath_t realMeansOfDeath;

	realMeansOfDeath = meansOfDeath;

	if ( _powerup )
		realMeansOfDeath = _powerup->changetMeansOfDeath( realMeansOfDeath );

	if ( _rune )
		realMeansOfDeath = _rune->changetMeansOfDeath( realMeansOfDeath );

	if ( _finishActor && _doingFinishingMove )
		realMeansOfDeath = MOD_REDEMPTION; // Finishing move MOD... oh sweet irony

	return realMeansOfDeath;
}

void Player::dropRune( Event * )
{
	if ( _rune )
	{
		dropRune();
	}
}

void Player::dropRune( void )
{
	if ( _rune )
	{
		setItemText( _rune->getIcon(), va( "$$Dropping$$ $$Item-%s$$", _rune->getName().c_str() ) );
		//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$Dropping$$ $$Item-%s$$", _rune->getName() );

		_rune->spawn( centroid );

		removeRune();
	}
	else
	{
		multiplayerManager.playerCommand( this, "dropItem", "" );
	}
}

//----------------------------------------------------------------
// Name:			setCanTransferEnergy
// Class:			Player
//
// Description:		Makes the player allowed to transfer energy from ammo to armor
//
// Parameters:		Event *
//
// Returns:			none
//----------------------------------------------------------------

void Player::setCanTransferEnergy( Event * )
{
	_canTransferEnergy = true;
}

//----------------------------------------------------------------
// Name:			transferEnergy
// Class:			Player
//
// Description:		Transfers some energy from ammo to the armor
//
// Parameters:		none
//
// Returns:			none
//----------------------------------------------------------------

void Player::transferEnergy( void )
{
	Event *armorEvent;

	// Make sure we are allowed to transfer energy

	if ( !_canTransferEnergy )
		return;

	// Make sure enough time has gone by to transfer more energy

	if ( _nextEnergyTransferTime > level.time )
		return;

	_nextEnergyTransferTime = level.time + level.frametime;

	// Make sure we have enough energy

	if ( AmmoCount( "Plasma" ) < 1 )
		return;

	// Make sure we still need more armor

	if ( GetArmorValue() >= 100 )
		return;

	// Give ourselves a little bit of armor

	armorEvent = new Event( EV_Sentient_GiveArmor );

	armorEvent->AddString( "BasicArmor" );
	armorEvent->AddFloat( 1.0f );

	ProcessEvent( armorEvent );	

	// Use up some energy

	UseAmmo( "Plasma", 1 );
}

//--------------------------------------------------------------
//
// Name:			AdvancedMeleeAttack
// Class:			Player
//
// Description:		Gets the waepon in the hand specified and calls
//					it's AdvancedMeleeAttack function.
//
// Parameters:		weaponhand_t hand -- weapon hand
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::AdvancedMeleeAttack(weaponhand_t hand)
{
	Weapon *weapon;
	bool	critical = false ;

	if ( hand == WEAPON_ERROR )
		return;

	weapon = GetActiveWeapon( hand );
	if ( !weapon )
		return;

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( gpm->hasProperty( "CurrentPlayer.Criticals", "value" ) )
	{
		float criticalSkill			= gpm->getFloatValue( "CurrentPlayer.Criticals", "value" );
		float criticalBaseChance	= gpm->getFloatValue( "Criticals", "baseChance" );
		float criticalChance		= criticalSkill * criticalBaseChance ;
		if ( G_Random() < criticalChance )
		{
			critical = true ;
		}
	}

	weapon->AdvancedMeleeAttack("tag_swipe1", "tag_swipe2", critical );
}

//--------------------------------------------------------------
// Name:			setDoDamageScreenFlash
// Class:			Player
//
// Description:		Sets whether or not we want to flash the player's screen when he is damaged
//
// Parameters:		Event *ev					- optionally contains a bool specifying on or off
//
// Returns:			None
//--------------------------------------------------------------

void Player::setDoDamageScreenFlash( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		_doDamageScreenFlash = ev->GetBoolean( 1 );
	else
		_doDamageScreenFlash = true;
}

//--------------------------------------------------------------
// Name:			pointOfView
// Class:			Player
//
// Description:		Forces the player into a different point of view
//
// Parameters:		Event *ev - 1 = 1st person, 0 = 3rd person
//
// Returns:			None
//--------------------------------------------------------------

void Player::pointOfView( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		// More than one argument, we are attempting to SET
		// the point of view specifically.
		bool pov = ev->GetBoolean( 1 );
		if ( pov == _isThirdPerson )
			return;
	}
	
	// Toggle the point of view
	if ( _isThirdPerson )
	{
		G_SendCommandToPlayer(edict, "cg_3rd_person 0");
		_isThirdPerson = false;
	}
	else
	{
		G_SendCommandToPlayer(edict, "cg_3rd_person 1");
		_isThirdPerson = true;
	}
}

void Player::HandleFinishableList()
{
	int i,j;
	for ( i=1; i<=finishableList.NumObjects(); i++ )
	{
		Actor *act = finishableList.ObjectAt(i);
		for ( j=1; j<=finishingMoveList.NumObjects(); j++ )
		{
			FinishingMove *fm = finishingMoveList.ObjectAt(j);
			float fovdot = cos( DEG2RAD( fm->coneangle * 0.5f) );
			Vector delta = ( act->origin ) - origin;
			if ( delta.length() > fm->distance )
				continue; 
			
			// Enemy Angles not yet used...
			// Chance not yet used...
			
			delta.z = 0.0f;
			delta.normalize();
			float dot = 0.0f;
			switch ( fm->direction )
			{
			case 0 : dot = DotProduct( yaw_forward, delta ); break;
			case 1 : dot = DotProduct( yaw_forward*-1.0f, delta ); break;
			case 2 : dot = DotProduct( yaw_left, delta ); break;
			case 3 : dot = DotProduct( yaw_left*-1.0f, delta ); break;
			}
			
			if ( dot > fovdot )
			{
				//				G_EnableWidgetOfPlayer( edict, "ActionIcon_Kick", true );
				_finishActor = act;
				_finishState = fm->statename;
				return;
			}
		}
	}
	
	//	G_EnableWidgetOfPlayer( edict, "ActionIcon_Kick", false );
	_finishActor = NULL;
	_finishState = "";
}

//--------------------------------------------------------------
//
// Name:			addFinishingMove
// Class:			Player
//
// Description:		Add a finishing move to the list
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::addFinishingMove( Event *ev )
{
	str state, dirstr;
	float coneangle = 45.0f;
	float dist = 64.0f;
	float eyaw = 0.0f;
	float chance = 1.0f;
	
	state = ev->GetString( 1 );
	dirstr = ev->GetString( 2 );
	
	if ( ev->NumArgs() > 2 )
		coneangle = ev->GetFloat( 3 );
	if ( ev->NumArgs() > 3 )
		dist = ev->GetFloat( 4 );
	if ( ev->NumArgs() > 4 )
		eyaw = ev->GetFloat( 5 );
	if ( ev->NumArgs() > 5 )
		chance = ev->GetFloat( 6 );

	FinishingMove *fm = new FinishingMove();
	fm->statename = state;
	if ( dirstr == "front" )
		fm->direction = 0;
	else if ( dirstr == "behind" )
		fm->direction = 1;
	else if ( dirstr == "left" )
		fm->direction = 2;
	else if ( dirstr = "right" )
		fm->direction = 3;
	else 
		fm->direction = 0; // Front default
	fm->coneangle = coneangle;
	fm->distance = dist;
	fm->enemyyaw = eyaw;
	fm->chance = chance;

	finishingMoveList.AddObject(fm);
}

//--------------------------------------------------------------
//
// Name:			clearFinishingMove
// Class:			Player
//
// Description:		Clears the finishing move list.
//
// Parameters:		Event *ev -- no params
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::clearFinishingMove( Event * )
{
	int i;
	for ( i=1; i<=finishingMoveList.NumObjects(); i++ )
	{
		FinishingMove *fm = finishingMoveList.ObjectAt(i);
		delete fm;
	}
	finishingMoveList.FreeObjectList();
}

//--------------------------------------------------------------
//
// Name:			doFinishingMove
// Class:			Player
//
// Description:		Fires off the finishing move that's currently prepared
//
// Parameters:		Event *ev -- no params
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::doFinishingMove( Event * )
{
	movecontrol = MOVECONTROL_ABSOLUTE;
	
	if ( _finishState.length() )
	{
		State* newState = statemap_Torso->FindState( _finishState );
		if ( newState )
		{
			EvaluateState( newState );
			_doingFinishingMove = true;
			_finishActor->SetState("FINISH_ME");
		}
		else
			gi.WDPrintf( "Could not find state %s on doFinishingMove\n", _finishState.c_str() );
	}
}

//--------------------------------------------------------------
//
// Name:			forceTimeScale
// Class:			Player
//
// Description:		Forces timescale for the game
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::forceTimeScale( Event *ev )
{
	float timescale = 1.0f;
	if ( ev->NumArgs() > 0 )
		timescale = ev->GetFloat( 1 );

	char tmp[15];
	sprintf(tmp, "timescale %f", timescale);

	G_SendCommandToPlayer(edict, tmp);
}

//--------------------------------------------------------------
//
// Name:			freezePlayer
// Class:			Player
//
// Description:		Freezes the player
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::freezePlayer( Event *ev )
{
	bool freeze = true;
	if ( ev->NumArgs() > 0 )
		freeze = ev->GetBoolean( 1 );

	level.playerfrozen = freeze;
}

//--------------------------------------------------------------
//
// Name:			immobilizePlayer
// Class:			Player
//
// Description:		Immobilizes the player (can only look around)
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::immobilizePlayer( Event *ev )
{
	bool freeze = true;

	if ( ev->NumArgs() > 0 )
		freeze = ev->GetBoolean( 1 );

	if ( freeze )
		flags |= FL_IMMOBILE;
	else
		flags &= ~FL_IMMOBILE;

}

//--------------------------------------------------------------
//
// Name:			setAttackType
// Class:			Player
//
// Description:		Sets the attack type of the attack
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::setAttackType( Event *ev )
{
	_attackType = ev->GetString( 1 );
}


//--------------------------------------------------------------
//
// Name:			getGameplayAnim
// Class:			Player
//
// Description:		Gets the anim name from the gameplay database
//					based on the current gameplayAnimIdx
//
// Parameters:		const str& objname -- The object
//
// Returns:			const str
//
//--------------------------------------------------------------
const str Player::getGameplayAnim(const str& objname)
{
	if ( !objname.length() || !getArchetype().length() )
		return "";

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	str scopestr = getArchetype() + "." + objname;
	if ( !gpm->hasObject(scopestr) )
		return "";

	char tmpstr[6];
	sprintf(tmpstr, "%d", _gameplayAnimIdx);
	return gpm->getStringValue(scopestr, tmpstr);
}


//--------------------------------------------------------------
//
// Name:			nextGameplayAnim
// Class:			Player
//
// Description:		Increment the the gameplay animation up to 
//					the maxchain value in the database.  Wrap
//					around afterwards.
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::nextGameplayAnim( Event *ev )
{
	if ( ev->NumArgs() < 1 )
		return;

	str objname = ev->GetString( 1 );
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	str scopestr = "CurrentPlayer.ChainedMoves";
	if ( !gpm->hasObject(scopestr) )
		return;

	int max = (int)gpm->getFloatValue(scopestr, "value");
	max++; // Increment to compensate for 0 based skill system, we always have to play the first anim
	if ( _gameplayAnimIdx < max )
		_gameplayAnimIdx++;
	else
		_gameplayAnimIdx = 1;

	// Max of 4 chains deep.
	if ( _gameplayAnimIdx > 4 )
		_gameplayAnimIdx = 1;
}

//--------------------------------------------------------------
//
// Name:			setGameplayAnim
// Class:			Player
//
// Description:		Sets the gameplayAnimIdx to the number specified
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::setGameplayAnim( Event *ev )
{
	int val = 1;
	if ( ev->NumArgs() > 0)
		val = ev->GetInteger( 1 );

	_gameplayAnimIdx = val;
}


//-----------------------------------------------------
//
// Name:		SetDisableUseWeapon
// Class:		Player
//
// Description:	Can disable the use of a new weapon
//
// Parameters:	ev - the event that sets the disableUseWeapon
//
// Returns:		None
//-----------------------------------------------------
void Player::setDisableUseWeapon( Event* ev)
{
	if(ev->NumArgs() > 0)
	{
		_disableUseWeapon = ev->GetBoolean(1);
	}
	else
	{
		_disableUseWeapon = true;
	}


	Com_Printf("DisableUseWeapon %d", _disableUseWeapon);

}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::setDisableInventory( Event* ev )
{
	if(ev->NumArgs() > 0)
	{
		disableInventory();
		return;
	}
	else if( ev->GetFloat(1))
	{
		enableInventory();
		return;
	}

	disableInventory();
}

//--------------------------------------------------------------
//
// Name:			skillChanged
// Class:			Player
//
// Description:		Handles skills when they change
//
// Parameters:		const str& objname -- Object in the database that has changed
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::skillChanged(const str& objname)
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(objname) )
		return;

	float value = gpm->getFloatValue(objname, "value");
	if ( objname == "CurrentPlayer.BonusHP" && value > 0.0f )
	{
		if ( gpm->hasFormula("BonusHP") )
		{
			GameplayFormulaData fd(this);
			float hp = gpm->calculate("BonusHP", fd);
			float maxhp = max_health + ( max_health * hp );
			setMaxHealth(maxhp);
		}
	}
}


//--------------------------------------------------------------
//
// Name:			EquipItems
// Class:			Player
//
// Description:		Equips items from the database
//
// Parameters:		Event *ev -- not used
//
// Returns:			None
//
//--------------------------------------------------------------
void Player::equipItems( Event * )
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();

	str itemName, modelName;
	
	itemName	= gpm->getStringValue("CurrentPlayer.DualWield", "name");
	modelName	= gpm->getStringValue(itemName, "model");
	if ( modelName.length() )
		giveItem(modelName);

	itemName	= gpm->getStringValue("CurrentPlayer.TwoHanded", "name");
	modelName	= gpm->getStringValue(itemName, "model");
	if ( modelName.length() )
		giveItem(modelName);

	itemName	= gpm->getStringValue("CurrentPlayer.Ranged", "name");
	modelName	= gpm->getStringValue(itemName, "model");
	if ( modelName.length() )
		giveItem(modelName);

	itemName	= gpm->getStringValue("CurrentPlayer.Ring", "name");
	modelName	= gpm->getStringValue(itemName, "model");
	if ( modelName.length() )
		giveItem(modelName);

	itemName	= gpm->getStringValue("CurrentPlayer.Amulet", "name");
	modelName	= gpm->getStringValue(itemName, "model");
	if ( modelName.length() )
		giveItem(modelName);

	useWeapon("DualWield", WEAPON_RIGHT);
}


//--------------------------------------------------------------
//
// Name:			getFreeInventorySlot
// Class:			Player
//
// Description:		Finds a free inventory slot from the database
//
// Parameters:		None
//
// Returns:			Object name of the free slot, or "" if the inventory
//					is full.
//
//--------------------------------------------------------------
const str Player::getFreeInventorySlot()
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();

	int i;
	char tmpstr[128];
	for ( i=1; i<15; i++ )
	{
		sprintf(tmpstr,"PartyInventory.Slot%d",i);
		str invitem = gpm->getStringValue(tmpstr, "name");
		if ( invitem == "Empty" )
			return tmpstr;
	}
	
	return "";
}

void Player::usePlayer( Event *ev )
{
	Player *usingPlayer = NULL;
	Equipment *equipment = NULL;
	Entity *owner;

	if ( multiplayerManager.inMultiplayer() )
	{
		Entity *entity;

		entity = ev->GetEntity( 1 );

		if ( entity->isSubclassOf( Player ) )
		{
			usingPlayer = (Player *)entity;
		}
		else if ( entity->isSubclassOf( Equipment ) )
		{
			equipment = (Equipment *)entity;

			owner = equipment->GetOwner();

			if ( owner && owner->isSubclassOf( Player ) )
			{
				usingPlayer = (Player *)owner;
			}
		}

		multiplayerManager.playerUsed( this, usingPlayer, equipment );
	}
}


//-----------------------------------------------------
//
// Name:		addRosterTeammate1
// Class:		Player
//
// Description:	Sets the third teammate in the roster.
//
// Parameters:	ev
//
// Returns:		None
//-----------------------------------------------------
void Player::addRosterTeammate1( Event*  )
{
}


//-----------------------------------------------------
//
// Name:		addRosterTeammate2
// Class:		Player
//
// Description:	Adds the second teammate from the roster.
//
// Parameters:	ev
//
// Returns:		None
//-----------------------------------------------------
void Player::addRosterTeammate2( Event*  )
{
}


//-----------------------------------------------------
//
// Name:		addRosterTeammate3
// Class:		Player
//
// Description:	Adds the third teammate from the roster.
//
// Parameters:	ev
//
// Returns:		None
//-----------------------------------------------------
void Player::addRosterTeammate3( Event*  )
{

}


//-----------------------------------------------------
//
// Name:		addRosterTeammate4
// Class:		Player
//
// Description:	Adds the fourth teammate from the roster.
//
// Parameters:	ev
//
// Returns:		None
//-----------------------------------------------------
void Player::addRosterTeammate4( Event*  )
{

}


//-----------------------------------------------------
//
// Name:		removeRosterTeammate1
// Class:		Player
//
// Description:	Removes the first teammate from the roster.
//
// Parameters:	ev
//
// Returns:		None
//-----------------------------------------------------
void Player::removeRosterTeammate1( Event*  )
{
}


//-----------------------------------------------------
//
// Name:		removeRosterTeammate2
// Class:		Player
//
// Description:	Removes the second teammate from the roster.
//
// Parameters:	ev
//
// Returns:		None
//-----------------------------------------------------
void Player::removeRosterTeammate2( Event*  )
{
}


//-----------------------------------------------------
//
// Name:		removeRosterTeammate3
// Class:		Player
//
// Description:	Removes the third teammate from the roster.
//
// Parameters:	ev
//
// Returns:		None
//-----------------------------------------------------
void Player::removeRosterTeammate3( Event*  )
{
}


//-----------------------------------------------------
//
// Name:		removeRosterTeammate4
// Class:		Player
//
// Description:	Removes the fourth teammate from the roster.
//
// Parameters:	ev
//
// Returns:		None
//-----------------------------------------------------
void Player::removeRosterTeammate4( Event*  )
{
}


bool Player::isButtonDown( int button )
{
	if ( last_ucmd.buttons & button )
		return true;
	else
		return false;
}

void Player::notifyPlayerOfMultiplayerEvent( const char *eventName, const char *eventItemName, Player *eventPlayer )
{
	Q_UNUSED(eventName);
	Q_UNUSED(eventItemName);
	Q_UNUSED(eventPlayer);
	// Put stuff here :)
}

void Player::touchingLadder( Trigger *ladder, const Vector &normal, float top )
{
	Q_UNUSED(ladder);

	// Make sure we are allowed to get on the ladder time wise

	if ( level.time < _nextLadderTime )
		return;

	// If we are near the top of the ladder and we have a groundentity don't get on the ladder

	if ( groundentity && origin.z > top - 48 )
		return;

	// Save off all necessary info

	_onLadder = true;
	_ladderNormal = normal;
	_ladderTop = top;
}

void Player::warp( Event *ev )
{
	setOrigin( ev->GetVector( 1 ) );
	NoLerpThisFrame();
	client->ps.pm_flags |= PMF_TIME_TELEPORT;
	CameraCut();
}

void Player::hudPrint( Event *ev )
{
	hudPrint( ev->GetString( 1 ) );
}

void Player::hudPrint( const str &string )
{
	str command;

	// Build the hud print command to send to the client

	command = "hudprint \"";
	command += string;
	command += "\"\n";

	// Send the HUD print command

	gi.SendServerCommand( edict - g_entities, command.c_str() );
}

void Player::setItemText( int itemIcon, const str &itemText )
{
	CancelEventsOfType( EV_Player_ClearItemText );

	_itemIcon = itemIcon;
	_itemText = itemText;

	PostEvent( EV_Player_ClearItemText, 2.0f );
}

void Player::clearItemText( void )
{
	_itemIcon = 0;
	_itemText = "";

	CancelEventsOfType( EV_Player_ClearItemText );
}

void Player::clearItemText( Event * )
{
	clearItemText();
}

void Player::shotFired( void )
{
	if ( p_heuristics )
	{
		p_heuristics->IncrementShotsFired();
	}
	
	++client->ps.stats[ STAT_SHOTS_FIRED ];

}

void Player::shotHit( void )
{
	if ( p_heuristics )
	{
		p_heuristics->IncrementShotsHit();
	}

	++client->ps.stats[ STAT_SHOTS_HIT ];
}


void Player::cinematicStarted( void )
{

	// Turn off any viewmodes

	setViewMode( "normal" );

	// Kill the zoom if any

	Weapon *weapon = GetActiveWeapon( WEAPON_DUAL );

	if ( weapon )
	{
		weapon->ProcessEvent( "endZoom" );

		// Force the weapon to idle

		weapon->ForceIdle();
	}
}

void Player::cinematicStopped( void )
{
	Weapon *weapon;
	Equipment *equipment;

	weapon = GetActiveWeapon( WEAPON_DUAL );

	if ( weapon && weapon->isSubclassOf( Equipment ) )
	{
		equipment = (Equipment *)weapon;

		equipment->updateMode();
	}

	// Force the player's state back to idle

	SetAnim( "stand_idle", legs, true );
	SetAnim( "stand_idle", torso, true );
	LoadStateTable();
}

void Player::loadUseItem( const str &item )
{
	Event *event;
	str itemName;

	itemName = item;

	if ( stricmp( item.c_str(), "tricorder" ) == 0 )
	{
		if ( HasItem( "Tricorder-stx" ) )
			itemName = "Tricorder-stx";
		else if ( HasItem( "Tricorder-rom" ) )
			itemName = "Tricorder-rom";
	}

	if ( itemName.length() )
	{
		event = new Event( EV_Player_UseItem );
		event->AddString( itemName );
		ProcessEvent( event );
	}
}

void Player::setValidPlayerModel( Event * )
{
	_validPlayerModel = true;
}

void Player::setVoteText( const str &voteText )
{
	_voteText = voteText;
}

void Player::clearVoteText( void )
{
	_voteText = "";
}

void Player::incrementSecretsFound( void )
{
	_secretsFound++;

	// Update the cvar if we are greater

	if ( _secretsFound > g_secretCount->integer )
	{
		gi.cvar_set( "g_secretCount", va( "%d", _secretsFound ) );
	}
}

void Player::addHud( Event *ev )
{
	addHud( ev->GetString( 1 ) );
}

void Player::addHud( const str &hudName )
{
	if ( !_hudList.ObjectInList( hudName ) )
	{
		addHudToClient( hudName );

		_hudList.AddObject( hudName );
	}
}

void Player::removeHud( Event *ev )
{
	removeHud( ev->GetString( 1 ) );
}

void Player::removeHud( const str &hudName )
{
	if ( _hudList.ObjectInList( hudName ) )
	{
		removeHudFromClient( hudName );

		_hudList.RemoveObject( hudName );
	}
}

bool Player::needToSendAllHudsToClient( void )
{
	if ( _needToSendHuds && _started )
		return true;
	else
		return false;
}

void Player::sendAllHudsToClient( void )
{
	int i;
	str commandString;
	str hudName;

	if ( _needToSendHuds )
	{
		for( i = 1 ; i <= _hudList.NumObjects() ; i++ )
		{
			hudName = _hudList.ObjectAt( i );

			addHudToClient( hudName );
		}
	}

	_needToSendHuds = false;
}

void Player::clearAllHuds( void )
{
	int i;
	str commandString;
	str hudName;

	for( i = 1 ; i <= _hudList.NumObjects() ; i++ )
	{
		hudName = _hudList.ObjectAt( i );

		removeHudFromClient( hudName );
	}

	_hudList.ClearObjectList();
}

void Player::addHudToClient( const str &hudName )
{
	str commandString;

	commandString = "stufftext \"ui_addhud ";
	commandString += hudName;
	commandString += "\"\n";

	gi.SendServerCommand( entnum, commandString.c_str() );
}

void Player::removeHudFromClient( const str &hudName )
{
	str commandString;

	commandString = "stufftext \"ui_removehud ";
	commandString += hudName;
	commandString += "\"\n";

	gi.SendServerCommand( entnum, commandString.c_str() );
}

void Player::killAllDialog( Event * )
{
	killAllDialog();
}

void Player::killAllDialog()
{
	Actor *theActor;
	int i;

	for ( i = 1; i <= SleepList.NumObjects(); i++ )
		{
		theActor = (Actor*)SleepList.ObjectAt( i );
		theActor->StopDialog();
		}		

	for ( i = 1; i <= ActiveList.NumObjects(); i++ )
		{
		theActor = (Actor*)ActiveList.ObjectAt( i );
		theActor->StopDialog();
		}
}

void Player::clearTempAttachments( void )
{
	int i;
	Entity *child;

	if ( bind_info )
	{
		for( i = 0; i < MAX_MODEL_CHILDREN ; i++ )
		{
			if ( bind_info->children[ i ] == ENTITYNUM_NONE )
				continue;

			child = ( Entity * )G_GetEntity( bind_info->children[ i ] );

			if ( child )
			{
				if ( child->isSubclassOf( Projectile ) || child->CancelEventsOfType( EV_Remove ) )
				{
					child->PostEvent( EV_Remove, 0.0f );			
				}
			}
		}
	}
}

void Player::setSkill( int skill )
{
	_skillLevel = skill;

	if ( _skillLevel < 0 )
		_skillLevel = 0;
	else if ( _skillLevel > 3 )
		_skillLevel = 3;


}

int Player::getSkill( void )
{
	return _skillLevel;
}

void Player::forceMoveType( Event *ev )
{
	str moveTypeName;

	moveTypeName = ev->GetString( 1 );

	if ( stricmp( moveTypeName, "secret" ) == 0 )
		_forcedMoveType = PM_SECRET_MOVE_MODE;
	else if ( stricmp( moveTypeName, "3rdPerson" ) == 0 )
		_forcedMoveType = PM_3RD_PERSON;
	else if ( stricmp( moveTypeName, "none" ) == 0 )
		_forcedMoveType = PM_NONE;
}

void Player::isPlayerOnGround( Event *ev )
{
	float onGround;

	if ( client->ps.walking ) 
		{
		onGround = 1.0f;
		}
	else
		{
		onGround = 0.0f;
		}


	ev->ReturnFloat( onGround );
}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::setBranchDialogActor( const Actor* actor)
{
	if( _branchDialogActor != 0)
		_branchDialogActor->clearBranchDialog();

	_branchDialogActor = (Actor*)actor;
}


//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Player::clearBranchDialogActor( void )
{
	if( _branchDialogActor != 0)
		_branchDialogActor->clearBranchDialog( );

	_branchDialogActor = 0;
}

void Player::setBackpackAttachOffset( Event *ev )
{
	_backpackAttachOffset = ev->GetVector( 1 );
}

void Player::setBackpackAttachAngles( Event *ev )
{
	_backpackAttachAngles = ev->GetVector( 1 );
}

Vector Player::getBackpackAttachOffset( void )
{
	return _backpackAttachOffset;
}

Vector Player::getBackpackAttachAngles( void )
{
	return _backpackAttachAngles;
}

void Player::setFlagAttachOffset( Event *ev )
{
	_flagAttachOffset = ev->GetVector( 1 );
}

void Player::setFlagAttachAngles( Event *ev )
{
	_flagAttachAngles = ev->GetVector( 1 );
}

Vector Player::getFlagAttachOffset( void )
{
	return _flagAttachOffset;
}

Vector Player::getFlagAttachAngles( void )
{
	return _flagAttachAngles;
}

bool Player::canRegenerate( void )
{
	if ( _powerup && !_powerup->canOwnerRegenerate() )
		return false;

	if ( _rune && !_rune->canOwnerRegenerate() )
		return false;

	return true;
}

void Player::modelChanged( void )
{
	if ( _powerup )
	{
		removePowerupEffect( _powerup );
		addPowerupEffect( _powerup );
	}

	if ( _rune )
	{
		removePowerupEffect( _rune );
		addPowerupEffect( _rune );
	}
}

void Player::setBackupModel( Event *ev )
{
	setBackupModel( ev->GetString( 1 ) );
}

void Player::setBackupModel( const str &modelName )
{
	gi.setviewmodel( edict, modelName );
}
