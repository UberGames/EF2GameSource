//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/weapon.cpp                                    $
// $Revision:: 263                                                            $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:43a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Source file for Weapon class.  The weapon class is the base class for
// all weapons in the game.  Any entity created from a class derived from the weapon
// class will be usable by any Sentient (players and monsters) as a weapon.
//

#include "_pch_cpp.h"
#include "entity.h"
#include "item.h"
#include "weapon.h"
#include "scriptmaster.h"
#include "sentient.h"
#include "misc.h"
#include "specialfx.h"
#include "actor.h"
#include "weaputils.h"
#include "player.h"
#include "decals.h"
#include "mp_manager.hpp"
#include <qcommon/gameplaymanager.h>

// for bot code
#include "g_local.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_ea.h"
#include "be_ai_char.h"
#include "be_ai_chat.h"
#include "be_ai_gen.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"
#include "be_ai_weap.h"
#include "ai_main.h"
extern bot_state_t	*botstates[MAX_CLIENTS];

#define TargetIdleTime 0.2

Event EV_Weapon_Shoot
(
	"shoot",
	EV_DEFAULT,
	"S",
	"mode",
	"Shoot the weapon"
 );
Event EV_Weapon_DoneRaising
(
	"ready",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Signals the end of the ready animation so the weapon can be used"
 );
Event EV_Weapon_DoneAnimating
(
	"weapon_done_animate",
	EV_CODEONLY,
	NULL,
	NULL,
	"Signals the end of the ready animation so the weapon can be used"
);
Event EV_Weapon_DoneFiring
(
	"donefiring",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Signals the end of the fire animation"
);
Event EV_Weapon_Idle
(
	"idle",
	EV_DEFAULT,
	NULL,
	NULL,
	"Puts the weapon into an idle state"
);
Event	EV_Weapon_SecondaryUse
(
	"secondaryuse",
	EV_DEFAULT,
	NULL,
	NULL,
	"Puts the weapon into its secondary mode of operation"
);
Event EV_Weapon_DoneReloading
(
	"donereloading",
	EV_CODEONLY,
	NULL,
	NULL,
	"Signals the end of the reload animation"
);
Event EV_Weapon_DoneReloadingBurst
(
	"doneReloadingBurst",
	EV_CODEONLY,
	NULL,
	NULL,
	"Signals the end of the reload animation for a burst"
);
Event EV_Weapon_SetAmmoClipSize
(
	"clipsize",
	EV_TIKIONLY,
	"i",
	"ammoClipSize",
	"Set the amount of rounds a clip of the weapon holds"
);
Event EV_Weapon_SetAmmoInClip
(
	"ammo_in_clip",
	EV_TIKIONLY,
	"i",
	"ammoInClip",
	"Set the amount of ammo in the clip"
);
Event EV_Weapon_ProcessModelCommands
(
	"process_mdl_cmds",
	EV_CODEONLY,
	NULL,
	NULL,
	"Forces a processing of the init commands for the model"
);
Event EV_Weapon_SetMaxRange
(
	"maxrange",
	EV_TIKIONLY,
	"f",
	"maxRange",
	"Set the maximum range of a weapon so the AI knows how to use it"
);
Event EV_Weapon_SetMinRange
(
	"minrange",
	EV_TIKIONLY,
	"f",
	"minRange",
	"Set the minimum range of a weapon so the AI knows how to use it"
);
Event EV_Weapon_ActionIncrement
(
	"actionincrement",
	EV_TIKIONLY,
	"i",
	"actionLevelIncrement",
	"Set the action level increment of the weapon.\n"
	"When the weapon is fired, it will raise the action level by this amount"
);
Event EV_Weapon_NotDroppable
(
	"notdroppable",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Makes a weapon not droppable"
);
Event EV_Weapon_SetAimAnim
(
	"setaimanim",
	EV_TIKIONLY,
	"si",
	"aimAnimation aimFrame",
	"Set the aim animation and frame for when a weapon fires"
);
Event EV_Weapon_SetFireType
(
	"firetype",
	EV_TIKIONLY,
	"s",
	"firingType",
	"Set the firing type of the weapon (projectile or bullet)"
);
Event EV_Weapon_SetProjectile
(
	"projectile",
	EV_TIKIONLY,
	"s",
	"projectileModel",
	"Set the model of the projectile that this weapon fires"
);
Event EV_Weapon_SetBulletDamage
(
	"bulletdamage",
	EV_TIKIONLY,
	"f",
	"bulletDamage",
	"Set the damage that the bullet causes"
);
Event EV_Weapon_SetBulletKnockback
(
	"bulletknockback",
	EV_TIKIONLY,
	"f",
	"bulletKnockback",
	"Set the knockback that the bullet causes"
);
Event EV_Weapon_SetBulletCount
(
	"bulletcount",
	EV_TIKIONLY,
	"f",
	"bulletCount",
	"Set the number of bullets this weapon shoots when fired"
);
Event EV_Weapon_SetBulletRange
(
	"bulletrange",
	EV_TIKIONLY,
	"f",
	"bulletRange",
	"Set the range of the bullets"
);
Event EV_Weapon_SetBulletSpread
(
	"bulletspread",
	EV_TIKIONLY,
	"ffFFF",
	"bulletSpreadX bulletSpreadY endSpreadX endSpreadY spreadTime",
	"Set the max spread of the bullet in the x and y axis, optionally with a different end spread and time interval"
);
Event EV_Weapon_SetRange
(
	"range",
	EV_TIKIONLY,
	"f",
	"range",
	"Set the range of the weapon"
);
Event EV_Weapon_Hand
(
	"hand",
	EV_TIKIONLY,
	"s",
	"weaponHand",
	"Set the hand the weapon can be wielded in (lefthand, righthand, both, or any)"
);
Event EV_Weapon_Mode
(
	"modeset",
	EV_TIKIONLY,
	"SSSSSSSSS",
	"arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9",
	"Set a value for a mode by passing commands through"
);
Event EV_Weapon_AmmoType
(
	"ammotype",
	EV_TIKIONLY,
	"s",
	"name",
	"Set the type of ammo this weapon uses"
);
Event EV_Weapon_StartAmmo
(
	"startammo",
	EV_TIKIONLY,
	"i",
	"amount",
	"Set the starting ammo of this weapon"
);
Event EV_Weapon_AmmoBoost
(
	"ammoBoost",
	EV_TIKIONLY,
	"i",
	"amount",
	"Set the ammo given to a sentient that picks up this weapon but already had a weapon of this type."
);
Event EV_Weapon_AmmoRequired
(
	"ammorequired",
	EV_TIKIONLY,
	"i",
	"amount",
	"Set the amount of ammo this weapon requires to fire"
);
Event EV_Weapon_MaxChargeTime
(
	"maxchargetime",
	EV_TIKIONLY,
	"i",
	"time",
	"Set the maximum time the weapon may be charged up"
);
Event EV_Weapon_GiveStartingAmmo
(
	"startingammotoowner",
	EV_CODEONLY,
	NULL,
	NULL,
	"Internal event used to give ammo to the owner of the weapon"
);
Event EV_Weapon_GiveAmmoBoost
(
	"giveAmmoBoost",
	EV_CODEONLY,
	NULL,
	NULL,
	"Internal event used to give ammo boost to the player that just picked up this weapon."
);
Event EV_Weapon_AutoAim
(
	"autoaim",
	EV_TIKIONLY,
	"FF",
	"selection_angle lockon_angle",
	"Turn on auto aiming for the weapon"
);
Event EV_Weapon_Crosshair
(
	"crosshair",
	EV_TIKIONLY,
	"b",
	"bool",
	"Turn on/off the crosshair for this weapon"
);
Event EV_Weapon_TorsoAim
(
	"torsoaim",
	EV_DEFAULT,
	"b",
	"bool",
	"Turn on/off the torsoaim for this weapon"
);
Event EV_Weapon_SetQuiet
(
	"quiet",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Makes the weapon make no noise."
);
Event EV_Weapon_SetLoopFire
(
	"loopfire",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Makes the weapon fire by looping the fire animation."
);
/*
Event EV_Weapon_FullAnimFire
(
	"fullanimfire",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the weapon play the full fire animation even if key is released"
);
*/
Event EV_Weapon_LeftAttachToTag
(
	"leftattachtotag",
	EV_TIKIONLY,
	"s",
	"tagname",
	"Set the name of the tag to attach this to it's owner when wielded in the left hand."
);
Event EV_Weapon_RightAttachToTag
(
	"rightattachtotag",
	EV_TIKIONLY,
	"s",
	"tagname",
	"Set the name of the tag to attach this to it's owner when wielded in the right hand."
);
Event EV_Weapon_DualAttachToTag
(
	"dualattachtotag",
	EV_TIKIONLY,
	"s",
	"tagname",
	"Set the name of the tag to attach this to it's owner when wielded dual handed."
);
Event EV_Weapon_HolsterTagLeft
(
	"leftholstertag",
	EV_TIKIONLY,
	"s",
	"tagname",
	"Set the name of the tag to attach this to when the weapon is put away from the left hand."
);
Event EV_Weapon_HolsterTagRight
(
	"rightholstertag",
	EV_TIKIONLY,
	"s",
	"tagname",
	"Set the name of the tag to attach this to when the weapon is put away from the right hand."
);
Event EV_Weapon_HolsterTagDual
(
	"dualholstertag",
	EV_TIKIONLY,
	"s",
	"tagname",
	"Set the name of the tag to attach this to when the weapon is put away from dual handed"
);
Event EV_Weapon_LeftHolsterAngles
(
	"leftholsterangles",
	EV_TIKIONLY,
	"v",
	"angles",
	"Set the angles of this weapon when it's attached to the left holster"
);
Event EV_Weapon_RightHolsterAngles
(
	"rightholsterangles",
	EV_TIKIONLY,
	"v",
	"angles",
	"Set the angles of this weapon when it's attached to the right holster"
);
Event EV_Weapon_DualHolsterAngles
(
	"dualholsterangles",
	EV_TIKIONLY,
	"v",
	"angles",
	"Set the angles of this weapon when it's attached to the dual holster"
);
Event EV_Weapon_HolsterScale
(
	"holsterscale",
	EV_TIKIONLY,
	"f",
	"scale",
	"Set the scale of the weapon when it's attached to the holster"
);
Event EV_Weapon_WeildedScale
(
	"weildedScale",
	EV_TIKIONLY,
	"f",
	"scale",
	"Sets the scale of the weapon when it's weilded"
);
Event EV_Weapon_AutoPutaway
(
	"autoputaway",
	EV_TIKIONLY,
	"b",
	"bool",
	"Set the weapon to be automatically put away when out of ammo"
);
Event EV_Weapon_UseNoAmmo
(
	"usenoammo",
	EV_TIKIONLY,
	"b",
	"bool",
	"Set the weapon to be able to be used when it's out of ammo"
);
Event EV_Weapon_SetMeansOfDeath
(
	"meansofdeath",
	EV_TIKIONLY,
	"s",
	"meansOfDeath",
	"Set the meansOfDeath of the weapon."
);
Event EV_Weapon_SetWorldHitSpawn
(
	"worldhitspawn",
	EV_TIKIONLY,
	"s",
	"modelname",
	"Set a model to be spawned when the weapon strikes the world."
);
Event EV_Weapon_MakeNoise
(
	"makenoise",
	EV_TIKIONLY,
	"FB",
	"noise_radius force",
	"Makes the weapon make noise that actors can hear."
);
Event EV_Weapon_SetViewModel
(
	"weaponviewmodel",
	EV_TIKIONLY,
	"s",
	"filename",
	"Set the view model name"
);
Event EV_Weapon_DonePutaway
(
	"doneputaway",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Called when the weapon is done with it's putaway anim"
);
Event EV_Weapon_SetRegenAmmo
(
	"regenammo",
	EV_TIKIONLY,
	"ii",
	"regenamount regentime",
	"Specifics that a weapon regenerates ammo over time"
);
Event EV_Weapon_SetRegenOnlyWhenIdle
(
	"regenonlywhenidle",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Specifics that the weapon only regenerates ammo when idle, must still use regen ammo to set how much."
);
Event EV_Weapon_ChangeIdle
(
	"changeidle",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Changes the idle animation"
);
Event EV_Weapon_DrawBowStrain
(
	"drawbowstrain",
	EV_DEFAULT,
	NULL,
	NULL,
	"Starts the bow draw strain animation"
);
Event EV_Weapon_AltDrawBowStrain
(
	"altdrawbowstrain",
	EV_DEFAULT,
	NULL,
	NULL,
	"Starts the alternate bow draw strain animation"
);
Event EV_Weapon_SetAccuracy
(
	"setaccuracy",
	EV_TIKIONLY,
	"sfffff",
	"firemode stoppedac acchange walkac runac crouchac",
	"Sets the accuracy of the weapon"
);
Event EV_Weapon_SetReticuleTime
(
	"setreticuletime",
	EV_TIKIONLY,
	"f",
	"reticuletime",
	"Reticule time"
);
Event EV_Weapon_SetZoomFOV
(
	"setzoomfov",
	EV_TIKIONLY,
	"f",
	"zoomfov",
	"Zoom FOV"
);
Event EV_Weapon_IncrementZoomFOV
(
	"inczoomfov",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Increments the zoom fov"
);
Event EV_Weapon_SetZoomStage
(
	"setzoomstage",
	EV_TIKIONLY,
	"ff",
	"fov1 fov2",
	"Sets the fov stage"
);
Event EV_Weapon_SetStartZoom
(
	"setstartzoom",
	EV_TIKIONLY,
	"f",
	"startzoom",
	"Sets the start zoom fov"
);
Event EV_Weapon_SetEndZoom
(
	"setendzoom",
	EV_TIKIONLY,
	"f",
	"endzoom",
	"Sets the end zoom fov"
);
Event EV_Weapon_SetZoomTime
(
	"setzoomtime",
	EV_TIKIONLY,
	"f",
	"zoomtime",
	"Sets the time it takes to zoom from startzoom to endzoom"
);
Event EV_Weapon_SetAimType
(
	"setaim",
	EV_DEFAULT,
	"s",
	"aimtype",
	"Sets the aimtype of the weapon (changes according to player state)"
);
Event EV_Weapon_SetFireTimer
(
	"setfiretimer",
	EV_TIKIONLY,
	"sf",
	"mode mintime",
	"Specifies the minimum time between shots"
);
Event EV_Weapon_UseSameClip
(
	"usesameclip",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Specifies that ammo comes out of the same clip in all fire modes"
);
Event EV_Weapon_SetMaxModes
(
	"maxmode",
	EV_TIKIONLY,
	"s",
	"maxmode",
	"Overrides the maximum number of modes for this weapon (default is 2)"
);
Event EV_Weapon_SetSwitchMode
(
	"switchmode",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Specifies that this is a switch mode weapon (right button switches)"
);
Event	EV_Weapon_DoneSwitching
(
	"doneswitching",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Signals the end of the switching to mode animation"
);
Event	EV_Weapon_DoneSwitchToMiddle
(
	"doneswitchtomiddle",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Signals the end of the switching to 'neutral' animation"
);
Event	EV_Weapon_TargetIdle
(
	"targetidle",
	EV_CODEONLY,
	NULL,
	NULL,
	"This weapon have a specific on-target idle"
);
Event EV_Weapon_TargetIdleThink
(
	"targetidlethink",
	EV_DEFAULT,
	NULL,
	NULL,
	"Think event to check for target on target idle weapons"
);
Event EV_Weapon_SetBurstMode
(
	"burstmode",
	EV_TIKIONLY,
	"i",
	"burstcount",
	"Set this mode to be burst mode, that uses burstcount ammo"
);
Event EV_Weapon_StartFiring
(
	"startfiring",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Sets the time that the weapon starts firing (for spread computations)"
);
Event EV_Weapon_FinishedFiring
(
	"finishedfiring",
	EV_TIKIONLY,
	"i",
	"finished_firing",
	"Sets if the gun is finished firing"
);
Event EV_Weapon_Zoom
(
	"zoom",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Zooms in"
);
Event EV_Weapon_EndZoom
(
	"endZoom",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Ends the zoom"
);
Event EV_Weapon_Rezoom
(
	"rezoom",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Rezooms to the last zoom fov"
);
Event EV_Weapon_SetTargetingSkin
(
	"targetingskin",
	EV_TIKIONLY,
	"i",
	"skinNum",
	"Sets the skin number to use when targeting something"
);
Event EV_Weapon_SetShootingSkin
(
	"shootingskin",
	EV_TIKIONLY,
	"i",
	"skinNum",
	"Sets the skin number to use when shooting"
);
Event EV_Weapon_SetFullAmmoSkin
(
	"fullAmmoSkin",
	EV_TIKIONLY,
	"ii",
	"skinNum modeNum",
	"Sets the skin number to use when the weapon has full ammo"
);
Event EV_Weapon_SetMoveSpeedModifier
(
	"moveSpeedModifier",
	EV_DEFAULT,
	"f",
	"modifier",
	"Sets the move speed modifier for this weapon (when not shooting)."
);
Event EV_Weapon_SetShootingMoveSpeedModifier
(
	"shootingMoveSpeedModifier",
	EV_DEFAULT,
	"f",
	"modifier",
	"Sets the move speed modifier for this weapon while shooting."
);
Event EV_Weapon_UseActorAiming
(
	"useactoraiming",
	EV_DEFAULT,
	"B",
	"flag",
	"This weapon aims with it's tag_barrel.  Regardless of who is holding it."
);
Event EV_Weapon_ViewShake
(
	"viewShake",
	EV_DEFAULT,
	"fF",
	"viewShakeMagnitude viewShakeDuration",
	"Sets the magnitude & duration of the view shaking while firing this weapon.\n"
	"Duration defaults to .05 if not set"
);
Event EV_Weapon_AdvancedViewShake
(
	"advancedViewShake",
	EV_DEFAULT,
	"vvFB",
	"minViewShake maxViewShake viewShakeDuration override",
	"Sets the min shake vector, max shake vector, & duration of the view shaking while firing this weapon.\n"
	"Duration defaults to .05 if not set"
);
Event EV_Weapon_ClearViewShake
(
	"clearViewShake",
	EV_CODEONLY,
	NULL,
	NULL,
	"Clears the current view shake."
);
Event EV_Weapon_ReduceViewShake
(
	"reduceViewShake",
	EV_CODEONLY,
	NULL,
	NULL,
	"Reduces the current view shake."
);
Event EV_Weapon_SetPowerRating
(
	"powerrating",
	EV_DEFAULT,
	"f",
	"rating",
	"Specifies how much damage is done per second by the weapon"
);
Event EV_Weapon_SetProjectileSpeed
(
	"projectilespeed",
	EV_DEFAULT,
	"f",
	"proj_speed",
	"Specifies the speed of the projectile -- Used by AI"
);
Event EV_Weapon_SetProjectileDamage
(
	"projectiledamage",
	EV_DEFAULT,
	"f",
	"proj_damage",
	"Specifies the damage of the projectile"
);
Event EV_Weapon_SetBurstModeDelay
(
	"burstModeDelay",
	EV_DEFAULT,
	"f",
	"burstModeDelay",
	"Specifies the length of time to use between bursts (do not use this if you want to use the animation timing)"
);
Event EV_Weapon_SetArcProjectile
(
	"arcprojectile",
	EV_DEFAULT,
	"b",
	"arc_bool",
	"Specifies for actors if the projectile should arc"
);
Event EV_Weapon_SetLowArcRange
(
	"lowarcrange",
	EV_DEFAULT,
	"f",
	"range",
	"Specifies for actors the range at which to change from high trajectory to normal"
);

Event EV_Weapon_SetPlayMissSound
(
	"playmisssound",
	EV_DEFAULT,
	"b",
	"playsound",
	"Sets whether or not to play a miss sound( snd_ricochet ) at the point of impact"
);

Event EV_Weapon_NoAmmoMode
(
	"noAmmoMode",
	EV_DEFAULT,
	NULL,
	NULL,
	"Specifies that the current mode also has a no ammo mode."
);
Event EV_Weapon_NoDelay
(
	"noDelay",
	EV_DEFAULT,
	NULL,
	NULL,
	"Specifies that this mode has no delay."
);
Event EV_Weapon_PauseRegen
(
	"pauseRegen",
	EV_DEFAULT,
	NULL,
	NULL,
	"Pauses the regen of ammo for a little bit."
);
Event EV_Weapon_SetChargedModels
(
	"chargedModels",
	EV_DEFAULT,
	"i",
	"numChargedModels",
	"Sets the number of charged models there are to use."
);
Event EV_Weapon_SetControllingProjectile
(
	"controllingProjectile",
	EV_DEFAULT,
	"B",
	"bool",
	"Sets whether or not the weapon is currently controlling a projectile or not."
);
Event EV_Weapon_SetCanInterruptFiringState
(
	"canInterruptFiringState",
	EV_DEFAULT,
	"B",
	"bool",
	"Sets whether or not the weapon can be interrupted during its firing state."
);
Event EV_Weapon_StartViewShake
(
	"startViewShake",
	EV_CODEONLY,
	NULL,
	NULL,
	"Starts the weapons view shake."
);
Event EV_Weapon_SpreadAnimData
(
	"spreadAnimData",
	EV_DEFAULT,
	"if",
	"numAnims totalTime",
	"Sets up the info for spread over time using different fire anims."
);
Event EV_Weapon_MaxViewShakeChange
(
	"maxViewShakeChange",
	EV_DEFAULT,
	"f",
	"maxViewChange",
	"Sets the max view change for this weapon."
);
Event EV_Weapon_ControlParms
(
	"controlProjParms",
	EV_DEFAULT,
	"ss",
	"emitterName soundName",
	"Sets the paramaters to use when controlling a projectile."
);
Event EV_Weapon_ProjectileControlHidden
(
	"controlProjHidden",
	EV_DEFAULT,
	"b",
	"hiddenBool",
	"Sets whether or not the projectile control is currently hidden."
);

Event EV_Weapon_MeleeParms
(
	"meleeParms",
	EV_DEFAULT,
	"fff",
	"width height length",
	"Sets the parms of the melee attack."
);
Event EV_Weapon_FireOffset
(
	"fireOffset",
	EV_DEFAULT,
	"v",
	"fireOffset",
	"Sets the offset where the weapon will fire from."
);
Event EV_Weapon_AutoReload
(
	"autoReload",
	EV_DEFAULT,
	"b",
	"autoReload",
	"Specifies whether or not the weapon automatically reloads."
);
Event EV_Weapon_AllowAutoSwitch
(
	"allowAutoSwitch",
	EV_DEFAULT,
	"b",
	"allowAutoSwitch",
	"Specifies whether or not the weapon automatically switches to another weapon when it's out of ammo."
);
Event EV_Weapon_ForceReload
(
	"forceReload",
	EV_DEFAULT,
	NULL,
	NULL,
	"Forces a reload to occur."
);
Event EV_Weapon_NextSwitchTime
(
	"nextSwitchTime",
	EV_DEFAULT,
	"f",
	"time",
	"Sets the next switch time."
);

CLASS_DECLARATION( Item, Weapon, NULL )
{
	{ &EV_Item_Pickup,							&Weapon::PickupWeapon },
	{ &EV_Weapon_DoneRaising,					&Weapon::DoneRaising },
	{ &EV_Weapon_DoneFiring,					&Weapon::DoneFiring },
	{ &EV_Weapon_DoneAnimating,					&Weapon::DoneAnimating },
	{ &EV_Weapon_Idle,							&Weapon::Idle },
	{ &EV_BroadcastSound,						&Weapon::WeaponSound },
	{ &EV_Weapon_DoneReloading,					&Weapon::DoneReloading },
	{ &EV_Weapon_DoneReloadingBurst,			&Weapon::DoneReloadingBurst },
	{ &EV_Weapon_SetAmmoClipSize,				&Weapon::SetAmmoClipSize },
	{ &EV_Weapon_SetAmmoInClip,					&Weapon::SetAmmoInClip },
	{ &EV_Weapon_ProcessModelCommands,			&Weapon::ProcessWeaponCommandsEvent },
	{ &EV_Weapon_SetMaxRange,					&Weapon::SetMaxRangeEvent },
	{ &EV_Weapon_SetMinRange,					&Weapon::SetMinRangeEvent },
	{ &EV_Weapon_ActionIncrement,				&Weapon::SetActionLevelIncrement },
	{ &EV_Weapon_NotDroppable,					&Weapon::NotDroppableEvent },
	{ &EV_Weapon_SetAimAnim,					&Weapon::SetAimAnim },
	{ &EV_Weapon_Shoot,							&Weapon::Shoot },
	{ &EV_Weapon_SetFireType,					&Weapon::SetFireType },
	{ &EV_Weapon_DonePutaway,					&Weapon::DonePutaway },
	{ &EV_Weapon_SetProjectile,					&Weapon::SetProjectile },
	{ &EV_Weapon_SetBulletDamage,				&Weapon::SetBulletDamage },
	{ &EV_Weapon_SetBulletCount,				&Weapon::SetBulletCount },
	{ &EV_Weapon_SetBulletKnockback,			&Weapon::SetBulletKnockback },
	{ &EV_Weapon_SetBulletRange,				&Weapon::SetBulletRange },
	{ &EV_Weapon_SetRange,						&Weapon::SetRange },
	{ &EV_Weapon_SetBulletSpread,				&Weapon::SetBulletSpread },
	{ &EV_Weapon_Hand,							&Weapon::SetHand },
	{ &EV_Weapon_Mode,							&Weapon::ModeSet },
	{ &EV_Weapon_AmmoType,						&Weapon::SetAmmoType },
	{ &EV_Weapon_StartAmmo,						&Weapon::SetStartAmmo },
	{ &EV_Weapon_AmmoBoost,						&Weapon::setAmmoBoost },
	{ &EV_Weapon_AmmoRequired,					&Weapon::SetAmmoRequired },
	{ &EV_Weapon_MaxChargeTime,					&Weapon::SetMaxChargeTime },
	{ &EV_Weapon_GiveStartingAmmo,				&Weapon::GiveStartingAmmo },
	{ &EV_Weapon_GiveAmmoBoost,					&Weapon::giveAmmoBoost },
	{ &EV_Weapon_AutoAim,						&Weapon::AutoAim },
	{ &EV_Weapon_Crosshair,						&Weapon::Crosshair },
	{ &EV_Weapon_TorsoAim,						&Weapon::TorsoAim },
	{ &EV_Weapon_LeftAttachToTag,				&Weapon::LeftAttachToTag },
	{ &EV_Weapon_RightAttachToTag,				&Weapon::RightAttachToTag },
	{ &EV_Weapon_DualAttachToTag,				&Weapon::DualAttachToTag },
	{ &EV_Weapon_HolsterTagLeft,				&Weapon::LeftHolsterAttachToTag },
	{ &EV_Weapon_HolsterTagRight,				&Weapon::RightHolsterAttachToTag },
	{ &EV_Weapon_HolsterTagDual,				&Weapon::DualHolsterAttachToTag },
	{ &EV_Weapon_RightHolsterAngles,			&Weapon::SetRightHolsterAngles },
	{ &EV_Weapon_LeftHolsterAngles,				&Weapon::SetLeftHolsterAngles },
	{ &EV_Weapon_DualHolsterAngles,				&Weapon::SetDualHolsterAngles },
	{ &EV_Weapon_HolsterScale,					&Weapon::SetHolsterScale },
	{ &EV_Weapon_WeildedScale,					&Weapon::setWeildedScale },
	{ &EV_Weapon_SetQuiet,						&Weapon::SetQuiet },
	{ &EV_Weapon_SetLoopFire,					&Weapon::SetLoopFire },
	//	{ &EV_Weapon_FullAnimFire,             SetFullAnimFire		},
	{ &EV_Weapon_AutoPutaway,					&Weapon::SetAutoPutaway },
	{ &EV_Weapon_UseNoAmmo,						&Weapon::SetUseNoAmmo },
	{ &EV_Weapon_SetMeansOfDeath,				&Weapon::SetMeansOfDeath },
	{ &EV_Weapon_SetWorldHitSpawn,				&Weapon::SetWorldHitSpawn },
	{ &EV_Weapon_MakeNoise,						&Weapon::MakeNoise },
	{ &EV_Weapon_SetViewModel,					&Weapon::SetViewModel },
	{ &EV_Weapon_SetRegenAmmo,					&Weapon::SetRegenAmmo },
	{ &EV_Weapon_SetRegenOnlyWhenIdle,			&Weapon::SetRegenOnlyWhenIdle },
	{ &EV_Weapon_ChangeIdle,					&Weapon::ChangeIdle },
	{ &EV_Weapon_DrawBowStrain,					&Weapon::DrawBowStrain },
	{ &EV_Weapon_AltDrawBowStrain,				&Weapon::AltDrawBowStrain },
	{ &EV_Weapon_SetAccuracy,					&Weapon::SetAccuracy },
	{ &EV_Weapon_SetReticuleTime,				&Weapon::SetReticuleTime },
	{ &EV_Weapon_SetZoomFOV,					&Weapon::SetZoomFOV },
	{ &EV_Weapon_IncrementZoomFOV,				&Weapon::IncrementZoom	},
	{ &EV_Weapon_SetZoomStage,					&Weapon::setZoomStage	},
	{ &EV_Weapon_SetStartZoom,					&Weapon::SetStartZoom },
	{ &EV_Weapon_SetEndZoom,					&Weapon::SetEndZoom },
	{ &EV_Weapon_SetZoomTime,					&Weapon::SetZoomTime },
	{ &EV_Weapon_SetAimType,					&Weapon::SetAimType },
	{ &EV_Weapon_SetFireTimer,					&Weapon::SetFireTimer },
	{ &EV_Weapon_UseSameClip,					&Weapon::UseSameClip },
	{ &EV_Weapon_SetMaxModes,					&Weapon::SetMaxModes },
	{ &EV_Weapon_SetSwitchMode,					&Weapon::SetSwitchMode },
	{ &EV_Weapon_DoneSwitchToMiddle,			&Weapon::DoneSwitchToMiddle },
	{ &EV_Weapon_DoneSwitching,					&Weapon::DoneSwitching },
	{ &EV_Weapon_TargetIdle,					&Weapon::TargetIdle },
	{ &EV_Weapon_TargetIdleThink,				&Weapon::TargetIdleThink },
	{ &EV_Weapon_SetBurstMode,					&Weapon::SetBurstMode },
	{ &EV_Weapon_FinishedFiring,				&Weapon::FinishedFiring		},
	{ &EV_Weapon_StartFiring,					&Weapon::StartFiring		},
	{ &EV_Weapon_Zoom,							&Weapon::Zoom },
	{ &EV_Weapon_EndZoom,						&Weapon::endZoom },
	{ &EV_Weapon_Rezoom,						&Weapon::rezoom },
	{ &EV_Weapon_SetTargetingSkin,				&Weapon::SetTargetingSkin		},
	{ &EV_Weapon_SetShootingSkin,				&Weapon::SetShootingSkin		},
	{ &EV_Weapon_SetFullAmmoSkin,				&Weapon::setFullAmmoSkin },
	{ &EV_Weapon_SetMoveSpeedModifier,			&Weapon::setMoveSpeedModifier },
	{ &EV_Weapon_SetShootingMoveSpeedModifier,	&Weapon::setShootingMoveSpeedModifier },
	{ &EV_Weapon_UseActorAiming,				&Weapon::UseActorAiming },
	{ &EV_Weapon_SetPowerRating,				&Weapon::SetPowerRating },
	{ &EV_Weapon_SetProjectileSpeed,			&Weapon::SetProjectileSpeed },
	{ &EV_Weapon_SetProjectileDamage,			&Weapon::SetProjectileDamage  },
	{ &EV_Weapon_ViewShake,						&Weapon::setViewShakeInfo },
	{ &EV_Weapon_AdvancedViewShake,				&Weapon::setAdvancedViewShakeInfo },
	{ &EV_Weapon_ReduceViewShake,				&Weapon::reduceViewShake },
	{ &EV_Weapon_ClearViewShake,				&Weapon::clearViewShake },
	{ &EV_ProcessGameplayData,					&Weapon::processGameplayData },

	{ &EV_Weapon_SetArcProjectile,				&Weapon::SetArcProjectile	},
	{ &EV_Weapon_SetLowArcRange,				&Weapon::SetLowArcRange		},

	{ &EV_Weapon_NoAmmoMode,					&Weapon::noAmmoMode },
	{ &EV_Weapon_NoDelay,						&Weapon::setNoDelay },
	{ &EV_Weapon_PauseRegen,					&Weapon::pauseRegen },

	{ &EV_Weapon_SetBurstModeDelay,				&Weapon::setBurstModeDelay },
	{ &EV_Weapon_SetChargedModels,				&Weapon::setChargedModels },
	{ &EV_Weapon_SetControllingProjectile,		&Weapon::setControllingProjectile },

	{ &EV_Weapon_SetCanInterruptFiringState,	&Weapon::setCanInterruptFiringState },

	{ &EV_Weapon_StartViewShake,				&Weapon::startViewShake },

	{ &EV_Weapon_SpreadAnimData,				&Weapon::setSpreadAnimData },

	{ &EV_Weapon_MaxViewShakeChange,			&Weapon::setMaxViewShakeChange },
	{ &EV_Weapon_ControlParms,					&Weapon::setControlParms },
	{ &EV_Weapon_ProjectileControlHidden,		&Weapon::setProjectileControlHidden },

	{ &EV_Weapon_MeleeParms,					&Weapon::setMeleeParms },
	{ &EV_Weapon_FireOffset,					&Weapon::setFireOffset },

	{ &EV_Weapon_AutoReload,					&Weapon::setAutoReload },
	{ &EV_Weapon_AllowAutoSwitch,				&Weapon::setAllowAutoSwitch },
	{ &EV_Weapon_ForceReload,					&Weapon::forceReload },
	{ &EV_Weapon_SetPlayMissSound,				&Weapon::SetPlayMissSound },

	{ &EV_Weapon_NextSwitchTime,				&Weapon::setNextSwitchTime },

	// These anim events are overridden from Entity
	{ &EV_Anim,									&Weapon::PassToAnimate },
	{ &EV_NewAnim,								&Weapon::PassToAnimate },

	{ NULL, NULL }
};

//======================
//Weapon::Weapon
//======================
Weapon::Weapon()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	// Owner of the weapon
	owner					      = NULL;
	
	// Starting rank of the weapon
	rank					      = 0;
	
	// Amount of ammo required for weapon
	INITIALIZE_WEAPONMODE_VAR( ammorequired, 0 );
	
	// Starting ammo of the weapon
	INITIALIZE_WEAPONMODE_VAR( startammo, 0 );

	INITIALIZE_WEAPONMODE_VAR( _ammoBoost, 0 );
	
	// Amount of ammo the clip can hold
	INITIALIZE_WEAPONMODE_VAR( ammo_clip_size, 0 );
	
	// Amount of ammo in clip
	INITIALIZE_WEAPONMODE_VAR( ammo_in_clip, 0 );
	
	// Amount of time to pass before broadcasting a weapon sound again
	nextweaponsoundtime     = 0;
	
	// The initial state of the weapon
	weaponstate			      = WEAPON_HOLSTERED;
	
	// Is the weapon droppable when the owner is killed
	notdroppable            = false;
	
	// Aim animation for behavior of monsters
	aimanim                 = -1;
	aimframe                = 0;
	
	// start off unattached
	attached = false;
	
	// maximum effective firing distance (for autoaim)
	maxrange = 1000;
	
	// minimum safe firing distance (for AI)
	minrange = 0;
	
	// speed of the projectile (0 == infinite speed)
	memset( projectilespeed, 0, sizeof( projectilespeed ) );
	
	// default action_level_increment
	INITIALIZE_WEAPONMODE_VAR( action_level_increment, 2 );
	
	// Weapons don't move
	setMoveType( MOVETYPE_NONE );
	
	// What type of ammo this weapon fires
	INITIALIZE_WEAPONMODE_VAR( firetype, (firetype_t)0 );
	
	// Init the bullet specs
	INITIALIZE_WEAPONMODE_VAR( bulletdamage,    0 );
	INITIALIZE_WEAPONMODE_VAR( bulletcount,     1 );
	INITIALIZE_WEAPONMODE_VAR( bulletrange,     1024 );
	INITIALIZE_WEAPONMODE_VAR( bulletknockback, 0 );
	INITIALIZE_WEAPONMODE_VAR( ammo_type,       "" );
	INITIALIZE_WEAPONMODE_VAR( loopfire,        false );
	//INITIALIZE_WEAPONMODE_VAR( fullanimfire,    false );
	INITIALIZE_WEAPONMODE_VAR( burstmode,		  false);
	burstcount = 0;
	burstcountmax = 0;
	
	// Init the max amount of time a weapon may be charged (5 seconds)
	INITIALIZE_WEAPONMODE_VAR( max_charge_time,   5 );
	charge_fraction = 1.0f;
	
	// Tag to attach this weapon to on its owner when used in the left hand and in the right hand
	left_attachToTag  = "tag_lhand";
	right_attachToTag = "tag_rhand";
	dual_attachToTag  = "tag_weapon_dual";
	
	// putaway is flagged true when the weapon should be put away by state machine
	putaway = false;
	
	// Default to being able to use the weapon in any hand
	hand    = WEAPON_ANY;
	
	// This is used for setting mode functionality when initializing stuff
	firemodeindex = FIRE_MODE1;
	
	// Name and index
	setName( "Unnamed Weapon" );
	
	// do better lighting on all weapons
	edict->s.renderfx |= RF_EXTRALIGHT;
	
	// make all weapons RF_WEAPONMODELs
	edict->s.renderfx |= RF_WEAPONMODEL;
	
	// Weapons do not auto aim automatically
	autoAimTargetSelectionAngle = 0;
	autoAimLockonAngle = 0;
	// No crosshair visible
	crosshair = false;
	
	// Don't torsoaim
	torsoaim = false;
	
	// Weapons default to making noise
	quiet = false;
	next_noise_time = 0;
	next_noammo_time = 0;
	
	// Used to keep track of last angles and scale before holstering
	lastValid = false;
	lastScale = 1.0f;
	holsterScale = 1.0f;
	_weildedScale = 0.5f;
	
	// Weapon will not be putaway by default when out of ammo
	auto_putaway = false;
	
	// Weapon will be able to be used when it has no ammo
	use_no_ammo  = true;
	
	// Default accuracy value
	for ( int i=0; i<MAX_FIREMODES; i++ )
	{
		for ( int j=0; j<MAX_ACCURACYTYPES; j++ )
		{
			accuracy[i][j] = 0.0; // Defaults to perfect accuracy
		}
	}
		
	aimtype = ACCURACY_STOPPED;
	
	// Reticule and zoom fov defaults
	reticuletime = 0.0;
	zoomfov = 90.0;
	_lastZoomFov = 90.0f;
	
	usesameclip = false;
	
	curmode = FIRE_MODE1;
	maxmode = FIRE_MODE2;
	
	chargetime = 0.0;
	
	switchmode = false;
	targetidle = false;
	targetidleflag = false;
	
	chx = 0;
	chy = 0;
	
	donefiring = false;
	
	zoomed = false;
	useActorAiming = false;
	
	INITIALIZE_WEAPONMODE_VAR(next_fire_time, 0.0f);
	INITIALIZE_WEAPONMODE_VAR(fire_timer, 0.0f);
	
	INITIALIZE_WEAPONMODE_VAR( meansofdeath, MOD_SWORD );
	
	targetingSkin = 0;
	shootingSkin = 0;
	_fullAmmoSkin = 0;
	_fullAmmoMode = FIRE_MODE1;
	
	_powerRating	  = 0.0f;
	_projectileDamage = 0.0f;
	_projectileSpeed  = 0.0f;
	_arcProjectile = false;
	_lowArcRange = 256.0f;
	_playMissSound = false;
	
	defaultMoveSpeedModifier = 1.0f;
	INITIALIZE_WEAPONMODE_VAR( shootingMoveSpeedModifier, 1.0f );
	
	INITIALIZE_WEAPONMODE_VAR( _viewShakeMagnitude, 0.0f );
	INITIALIZE_WEAPONMODE_VAR( _viewShakeDuration, 0.0f );

	_maxViewShakeChange = 10.0f;
	
	INITIALIZE_WEAPONMODE_VAR( _burstModeDelay, 0.0f );
	
	INITIALIZE_WEAPONMODE_VAR( _noAmmoMode, false );
	
	INITIALIZE_WEAPONMODE_VAR( _noDelay, false );

	INITIALIZE_WEAPONMODE_VAR( _chargedModels, 0 );
	
	PostEvent( EV_Weapon_Idle, 0.0f );

	_mpItemType = MP_ITEM_TYPE_WEAPON;

	_controllingProjectile = false;
	_controllingProjectileHidden = false;

	startfiretime = 0.0f;

	INITIALIZE_WEAPONMODE_VAR( _spreadAnims, 0 );
	INITIALIZE_WEAPONMODE_VAR( _spreadTime, 0.0f );

	INITIALIZE_WEAPONMODE_VAR( _meleeWidth,   15.0f );
	INITIALIZE_WEAPONMODE_VAR( _meleeHeight , 45.0f );
	INITIALIZE_WEAPONMODE_VAR( _meleeLength,  96.0f );

	// Regen stuff

	INITIALIZE_WEAPONMODE_VAR( _regenAmount,  0 );
	INITIALIZE_WEAPONMODE_VAR( _regenTime,  0.0f );
	INITIALIZE_WEAPONMODE_VAR( _regenOnlyWhenIdle,  false );
	INITIALIZE_WEAPONMODE_VAR( _nextRegenTime,  0.0f );

	_zoomStage = ZOOM_STAGE_1;

	setRespawnTime( 10 );

	_autoReload = true;

	_allowAutoSwitch = true;

	edict->s.renderfx |= RF_CHILDREN_DONT_INHERIT_ALPHA;

	_nextSwitchTime = 0.0f;
}


//======================
//Weapon::Weapon
//======================
Weapon::Weapon( const char *file )
{
	Q_UNUSED(file);
	// The tik file holds all the information available for a weapon
	Weapon();
}

//======================
//Weapon::~Weapon
//======================
Weapon::~Weapon()
{
	DetachGun();
}

//======================
//Weapon::GetRank
//======================
int Weapon::GetRank( void )
{
	return rank;
}

//======================
//Weapon::GetOrder
//======================
int Weapon::GetOrder( void )
{
	return order;
}

//======================
//Weapon::SetRank
//======================
void Weapon::SetRank( int order, int rank )
{
	this->order = order;
	this->rank = rank;
}

//======================
//Weapon::SetAutoPutaway
//======================
void Weapon::SetAutoPutaway( Event *ev )
{
	auto_putaway = ev->GetBoolean( 1 );
}

//======================
//Weapon::SetUseNoAmmo
//======================
void Weapon::SetUseNoAmmo( Event *ev )
{
	use_no_ammo = ev->GetBoolean( 1 );
}

//======================
//Weapon::SetStartAmmo
//======================
void Weapon::SetStartAmmo( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	startammo[firemodeindex] = ev->GetInteger( 1 );
}

void Weapon::setAmmoBoost( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );

	_ammoBoost[ firemodeindex ] = ev->GetInteger( 1 );
}

//======================
//Weapon::SetMaxChargeTime
//======================
void Weapon::SetMaxChargeTime( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	max_charge_time[firemodeindex] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetAmmoRequired
//======================
void Weapon::SetAmmoRequired( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	ammorequired[firemodeindex] = ev->GetInteger( 1 );
}

//======================
//Weapon::GetStartAmmo
//======================
int Weapon::GetStartAmmo( firemode_t mode )
{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) )
		return startammo[mode];
	else
	{
		warning( "Weapon::GetStartAmmo", "Invalid mode %d\n", mode );
		return 0;
	}
}

int Weapon::getAmmoBoost( firemode_t mode )
{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) )
	{
		return _ammoBoost[ mode ];
	}
	else
	{
		warning( "Weapon::getAmmoBoost", "Invalid mode %d\n", mode );
		return 0;
	}
}

//======================
//Weapon::GetAmmoType
//======================
str Weapon::GetAmmoType( firemode_t mode )
{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) )
		return ammo_type[mode];
	else
	{
		warning( "Weapon::GetAmmoType", "Invalid mode %d\n", mode );
		return "UnknownAmmo";
	}
}

//======================
//Weapon::SetAmmoType
//======================
void Weapon::SetAmmoType( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	
	if ( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) )
		ammo_type[firemodeindex] = ev->GetString( 1 );
	else
	{
		warning( "Weapon::SetAmmoType", "Invalid mode %d\n", firemodeindex );
		return;
	}
}

//======================
//Weapon::SetAmmoAmount
//======================
void Weapon::SetAmmoAmount( int amount, firemode_t mode )
{
	firemode_t clipToUse;
	
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( usesameclip )
		clipToUse = FIRE_MODE1;
	else
		clipToUse = mode;
	
	// If the clip can hold ammo, then set the amount in the clip to the specified amount
	if ( ( clipToUse >= 0 ) && ( clipToUse < MAX_FIREMODES ) )
	{
		if ( ammo_clip_size[clipToUse] )
			ammo_in_clip[clipToUse] = amount;
	}
	else
	{
		warning( "Weapon::SetAmmoAmount", "Invalid mode %d\n", clipToUse );
		return;
	}
}


//-----------------------------------------------------
//
// Name:		GetRequiredAmmo	
// Class:		Weapon
//
// Description:	Retrieves the required amount of ammo to fire one round
//
// Parameters:	mode - the firing mode.
//
// Returns:		None
//-----------------------------------------------------
int Weapon::GetRequiredAmmo( firemode_t mode )
{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	return ammorequired[mode];
}


//======================
//Weapon::GetClipSize
//======================
int Weapon::GetClipSize( firemode_t mode )
{
	firemode_t clipToUse;
	
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( usesameclip )
		clipToUse = FIRE_MODE1;
	else
		clipToUse = mode;
	
	if ( ( clipToUse >= 0 ) && ( clipToUse < MAX_FIREMODES ) )
		return ammo_clip_size[clipToUse];
	else
	{
		warning( "Weapon::GetClipSize", "Invalid mode %d\n", clipToUse );
		return 0;
	}
}

//======================
//Weapon::UseAmmo
//======================
void Weapon::UseAmmo( int amount, firemode_t mode )
{
	firemode_t clipToUse;

	if ( UnlimitedAmmo( mode ) )
		return;

	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	if ( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
		warning( "Weapon::UseAmmo", "Invalid mode %d\n", mode );

	if ( !HasAmmo( mode ) && hasNoAmmoMode( mode ) )
	{
		return;
	}

	// Determine which clip to use

	if ( usesameclip )
		clipToUse = FIRE_MODE1;
	else
		clipToUse = mode;

	// Remove ammo from the clip if it's available

	if ( ammo_clip_size[clipToUse] )
	{
		ammo_in_clip[clipToUse] -= amount;
		if (ammo_in_clip[clipToUse] < 0)
		{
			warning("UseAmmo","Used more ammo than in clip.\n");
			ammo_in_clip[clipToUse] = 0;
		}
		owner->AmmoAmountInClipChanged( ammo_type[mode], ammo_in_clip[clipToUse] );
	}
	else
	{
		assert( owner );
		if ( owner && owner->isClient() && !UnlimitedAmmo( mode ) )
		{
			// Remove ammo from the player's inventory
			owner->UseAmmo( ammo_type[mode], ammorequired[mode] );
		}
	}

	if ( burstmode[mode] ) 
	{
		burstcount--;
	}
}

void Weapon::GetActorMuzzlePosition( Vector *position, Vector *forward, Vector *right, Vector *up, const char* tagname )
{
	int tag_num;
	
	// Using tag_name for convience since it's referenced several times
	str tag_name;
	if ( !tagname )
		tag_name = "tag_barrel";
	else
		tag_name = tagname;
	
	tag_num = gi.Tag_NumForName( edict->s.modelindex, tag_name );
	
	if ( tag_num < 0 )
		gi.Error( ERR_FATAL, "Weapon::GetActorMuzzlePosition, invalid tag name." );
	
	//Test Stuff
	setOrigin();
	setAngles();
	
	//GetTag( tag_num, &pos, &forward, &right, &up );
	
	orientation_t  weap_or, barrel_or, orn;
	Vector         pos;
	vec3_t         mat[3]={0,0,0,0,0,0,0,0,0};
	vec3_t         orient[3];
	int            i, mi, tagnum;
	Sentient *owner;   
	owner = this->owner;
	
	// Get the owner's weapon orientation ( this is custom code and doesn't use the GetTag function
	// because we need to use the saved off fire_frame and fire_animation indexes from the owner	
	mi = owner->edict->s.modelindex;
	
	tagnum = gi.Tag_NumForName( mi, current_attachToTag.c_str() );
	
	
	// Get the orientation based on the frame and anim stored off in the owner.
	// This is to prevent weird timing with getting orientations on different frames of firing
	// animations and the orientations will not be consistent.
	
	AnglesToAxis( owner->angles, owner->orientation );

	orn	= gi.Tag_OrientationEx( mi,
								owner->CurrentAnim( legs ),
								owner->CurrentFrame( legs ),
								tagnum & TAG_MASK,
								owner->edict->s.scale,
								owner->edict->s.bone_tag,
								owner->edict->s.bone_quat,
								0,
								0,
								1.0f,
								( owner->edict->s.anim & ANIM_BLEND ) != 0,
								( owner->edict->s.torso_anim & ANIM_BLEND ) != 0,
								owner->CurrentAnim( torso ),
								owner->CurrentFrame( torso ),
								0,
								0,
								1.0f
								);

	// Transform the weapon's orientation through the owner's orientation
	// Player orientation is normally based on the player's view, but we need
	// it to be based on the model's orientation, so we calculate it here.
	AnglesToAxis( owner->angles, orient );
	VectorCopy( owner->origin, weap_or.origin );
	for ( i=0;  i<3; i++ )
	{
		VectorMA( weap_or.origin, orn.origin[i], orient[i], weap_or.origin );
	}
	
	MatrixMultiply( orn.axis, orient, weap_or.axis );
	
	if ( !this->GetRawTag( tag_name, &barrel_or ) )
	{
		pos = owner->centroid;
		AnglesToAxis( owner->angles, mat );
		return;
	}
	
	// Translate the barrel's orientation through the weapon's orientation
	VectorCopy( weap_or.origin, pos );
	
	for ( i = 0 ; i < 3 ; i++ )
	{
		VectorMA( pos, barrel_or.origin[i], weap_or.axis[i], pos );
	}
	
	MatrixMultiply( barrel_or.axis, weap_or.axis, mat );
	
	if ( position )
		*position = pos;
	
	if ( forward )
		*forward = mat[0];
	
	if ( right )
		*right = mat[1];
	
	if ( up )
		*up = mat[2];
}

//======================
//Weapon::GetMuzzlePosition
//======================
void Weapon::GetMuzzlePosition( Vector *position, Vector *forward, Vector *right, Vector *up )
{
	Vector endpoint, vorg;
	orientation_t barrel_or;
	Vector f, r, u, viewWeapOrg, pview, pos;
	Sentient *owner;
	Player *player;
	int      tagnum;
	
	owner = this->owner;
	assert( owner );
	
	if ( owner->isSubclassOf( Player ) )
		player = ( Player * )owner;
	else
	{
		warning("GetMuzzlePosition called from Weapon class for a non-player character.",NULL);
		return;
	}
	
	// Assign player based variables

	vorg = player->origin;
	// Offset by the view since the origin is at the feet
	vorg.z += player->client->ps.viewheight;

	pview = player->GetVAngles();
	
/*	
	vec3_t new_vieworg;
	vec3_t left;

	if( player->client->ps.leanDelta != 0)
	{
		pview[2] -= player->client->ps.leanDelta / 2.0f;

		AngleVectors( pview, NULL, left, NULL );
		VectorMA( vorg, player->client->ps.leanDelta, left, new_vieworg );
		VectorCopy( new_vieworg, vorg );
	}
*/	
	// Assign player based variables
	
	// Get our f, r, u vectors and set an endpoint
	AngleVectors( pview, f, r, u );
	
	// Create our point in worldspace.  Lots of hardcoded crap here.  
	// This will only work correctly with the default FOV (since we don't have access any real numbers)
	// We also don't have access to the znear value.
	trace_t viewTrace;
	player->GetViewTrace(viewTrace, MASK_SHOT);
	endpoint = viewTrace.endpos;
	Vector dir = endpoint - vorg;
	float len = dir.length();
	dir.normalize();
	endpoint = vorg + dir * (len + 5.0f);
	/*Vector p,s;
	p.x = (float)chx * (-4.0f / 320.0f);
	p.y = (float)chy * (-3.0f / 240.0f);
	s = 4.0f * f + p.x * r + p.y * u;
	s.normalize();
	endpoint = vorg + (s * 3000);*/
	
	// Now we get the origin of the weapon that the CLIENT knows (first person)
	viewWeapOrg = vorg;
	//setScale(0.5f); // Client scales the weapon to half-size
	//VectorMA( viewWeapOrg, -9.0f, r, viewWeapOrg );
	//VectorMA( viewWeapOrg, -7.5f, u, viewWeapOrg );
	
	// Weapon is about at viewWeapOrg position, neglecting viewbob/pullback
	
	// Do a trace to get our endpoint
	
	trace_t trace;
	
	if ( !multiplayerManager.inMultiplayer() || multiplayerManager.fullCollision() )
		trace = G_FullTrace( vorg, vec_zero, vec_zero, endpoint, owner, MASK_SHOT, true, "Weapon::GetMuzzlePosition" );
	else
		trace = G_Trace( vorg, vec_zero, vec_zero, endpoint, owner, MASK_SHOT, true, "Weapon::GetMuzzlePosition" );
	
	// Get the barrel tag from the viewmodel
	
	tagnum = gi.Tag_NumForName( edict->s.viewmodelindex, "tag_barrel" );
	
	if ( tagnum >= 0 )
	{
		barrel_or = gi.Tag_Orientation( edict->s.viewmodelindex, 0, 0, tagnum & TAG_MASK, 1.0f, edict->s.bone_tag, edict->s.bone_quat );
	}
	else
	{
		VectorClear( barrel_or.origin );
	}
	
	// Get the tag_barrel of the gun, otherwise use the owners centroid
	/* if ( !this->GetRawTag( "tag_barrel", &barrel_or ) )
	{
	warning( "Weapon::GetMuzzlePosition", "Could not find tag \"%s\"", current_attachToTag.c_str() );
	*position = owner->centroid;		
	} 
	else
	*position = barrel_or.origin; */
	
	// Translate the barrel's orientation through the weapon's orientation

	pos = viewWeapOrg;

	// Offset the fire position if the weapon tells us to

	if ( _fireOffset[ curmode ] != vec_zero )
	{
		float viewOrientation[3][3];

		AnglesToAxis( pview, viewOrientation );
		MatrixTransformVector( _fireOffset[ curmode ], viewOrientation, pos );
		pos += viewWeapOrg;
	}
	
	// For now lets shoot from the viewpoint not the weapon
	//VectorMA( pos, barrel_or.origin[0], f, pos );
	//VectorMA( pos, barrel_or.origin[1], r, pos );
	//VectorMA( pos, barrel_or.origin[2] - 1.0f, u, pos );
	
	// Final barrel position
	*position = pos;
	
	// Now set a NEW f,r,u vector based on changed needed to hit the center of the screen
	Vector f2 = trace.endpos - *position;
	AngleVectors( f2.toAngles(), f, r, u );
	
	if ( forward ) 
		*forward = f;
	if ( right )
		*right = r;
	if ( up )
		*up = u;
	
	//G_DebugLine( *position, trace.endpos, 1,0,0,1 );
}


//======================
//Weapon::SetAmmoClipSize
//======================
void Weapon::SetAmmoClipSize( Event * ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	ammo_clip_size[firemodeindex] = ev->GetInteger( 1 );
}

//======================
//Weapon::SetAmmoInClip
//======================
void Weapon::SetAmmoInClip( Event * ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	ammo_in_clip[firemodeindex] = ev->GetInteger( 1 );
}

//======================
//Weapon::Shoot
//======================
// This function is called from the weapon tiki file
// With a loopfire weapon, this function may be called 
// multiple times without having called "Fire", so we need
// to also call UseAmmo here for loopfire weapons
void Weapon::Shoot( Event *ev )
{
	Vector      pos, forward, right, up, delta;
	firemode_t  mode = FIRE_MODE1;
	firemode_t  realmode = FIRE_MODE1;
	firemode_t  regenMode;
	
	if ( ev->NumArgs() > 0 )
	{
		mode = WeaponModeNameToNum( ev->GetString( 1 ) );
		realmode = mode;
		if ( mode == FIRE_ERROR )
			return;
	}
	
	// Override the firemode if we're a switch weapon
	if ( switchmode ) 
		mode = curmode;
	else 
		curmode = mode;
	
	/* if ( next_fire_time[mode] > level.time )
		return; */
	
	// Set the fire timer if we have one
	if ( fire_timer[mode] > 0.0 )
		next_fire_time[mode] = fire_timer[mode] + level.time;
	
	// Set accuracy spread
	//bulletspread[mode].x = 50.0f * accuracy[mode][aimtype];
	//bulletspread[mode].y = 50.0f * accuracy[mode][aimtype];
	
	//if ( usesameclip )
	//	mode = FIRE_MODE1;
	
	// If we are in loopfire, we need to keep checking ammo and using it up
	if ( loopfire[mode] )
	{
		if ( HasAmmo( mode ) || hasNoAmmoMode( mode ) ||UnlimitedAmmo( mode ) )
		{
			// Use up the appropriate amount of ammo, it's already been checked that we have enough
			UseAmmo( ammorequired[mode], mode );
			/* if ( mode != realmode )
			{
				if ( burstmode[realmode] )
					burstcount -= ammorequired[realmode];
			} */
			if ( switchmode ) 
				mode = curmode;
		}
		else
		{
			if ( switchmode ) 
				mode = curmode;
			ForceIdle();
			return;
		}
	}
	
	mode = realmode;
	
	if ( ( usesameclip ) || ( ammo_clip_size[ mode ] == 0 ) )
		regenMode = FIRE_MODE1;
	else
		regenMode = mode;

	if ( ( _regenOnlyWhenIdle[ regenMode ] ) && ( _regenAmount[ regenMode ] > 0 ) )
	{
		_nextRegenTime[ regenMode ] = level.time + _regenTime[ regenMode ] + 0.5f;
	}
	
	Sentient *owner;   
	owner = this->owner;
	
	// If I am owned by a player, I need to get the muzzle position, otherwise, I just
	// care about my barrel tag
	if ( useActorAiming || !owner->isSubclassOf( Player ) )
	{
		/*			
		Actor *actorOwner;
		actorOwner = (Actor*)owner;

		Vector gunPos, gunForward, gunRight, gunUp;
		actorOwner->combatSubsystem->AimWeaponTag(actorOwner->enemyManager->GetCurrentEnemy() );
		actorOwner->combatSubsystem->GetGunPositionData( &gunPos, &gunForward, &gunRight, &gunUp );
		forward = gunForward;
		*/
		owner->shotsFiredThisVolley++;
		GetActorMuzzlePosition( &pos , &forward, &right, &up );
	}
	else if ( owner->isSubclassOf( Player ) )
	{
		Player *player = (Player *)owner;

		GetMuzzlePosition( &pos, &forward, &right, &up );
		if (g_aimviewangles->integer || ( player->client->ps.pm_type == PM_SECRET_MOVE_MODE ) ) // use viewangles to aim instead of player muzzle orientation thing
		{
			vec3_t fwd;
			AngleVectors(owner->client->ps.viewangles,fwd,NULL,NULL);
			forward = fwd;
		}
		// Apply spread
	
		if ( firetype[mode] != FT_BULLET )
		{
			applySpread( &forward, &right, &up );
		}
	}
	
	PostEvent( EV_Weapon_StartViewShake, level.frametime * 2 );
	
	Vector testForward;
	pos.AngleVectors( &testForward );
	
	if ( firetype[mode] == FT_PROJECTILE )
	{
		// Temporary 
		// This is bad bad bad
		//
		/*if ( owner->isSubclassOf(Actor) )
		{
			Entity *target = NULL;
			Actor *actor = NULL;
			Vector actorToTarget;
			
			actor = (Actor*)owner;
			
			if ( actor )
			{
				target = actor->enemyManager->GetCurrentEnemy();
				
				if ( target )
				{
					actorToTarget = target->centroid - actor->centroid;
					actorToTarget.normalize();
					forward = actorToTarget;
				}
			}			
			
			applySpread( pos, &forward, &right, &up );
		}*/

		str projectileModelName;
		
		// Figure out the projectile name to shoot

		if ( _chargedModels[ mode ] )
		{
			int modelIndexToUse;

			// Build the projectile name - projectileModel + # + .tik

			projectileModelName = projectileModel[mode];

			modelIndexToUse = ( _chargedModels[ mode ] * charge_fraction ) + 1;

			if ( modelIndexToUse > _chargedModels[ mode ] )
				modelIndexToUse = _chargedModels[ mode ];

			projectileModelName.CapLength( projectileModelName.length() - 4 );
			projectileModelName += modelIndexToUse;
			projectileModelName += ".tik";
		}
		else
		{
			projectileModelName = projectileModel[mode];
		}

		ProjectileAttack( pos,
			forward,
			owner,
			projectileModelName,
			charge_fraction
			);
	}
	else if ( firetype[mode] == FT_BULLET )
	{
		Vector spread;
		float range;

		if ( owner->isSubclassOf( Player ) )
		{
			// Move the position to shoot from back a little ( 1 1/2 feet )
			
			//pos = pos - forward * 24.0f;

			spread = getSpread();
			//range = bulletrange[mode] + 24.0f;
			range = bulletrange[mode];
		}
		else
		{
			spread = bulletspread[ mode ];
			range = bulletrange[mode];
		}
		
		BulletAttack( pos,
			forward,
			right,
			up,
			range,
			bulletdamage[mode],
			bulletknockback[mode],
			0,
			GetMeansOfDeath( mode ),
			spread,
			bulletcount[mode],
			owner,
			this
			);
	}
	else if ( firetype[mode] == FT_EXPLOSION )
	{
		if ( owner && owner->isSubclassOf( Player ) )
		{
			Player *player = (Player *)owner;

			player->shotFired();
		}

		ExplosionAttack( pos, owner, projectileModel[mode], forward, owner );
	}
	// Apparently the special_projectile stuff is no longer used
	/* else if ( firetype[mode] == FT_SPECIAL_PROJECTILE )
	{
		this->SpecialFireProjectile( pos,
			forward,
			right,
			up,
			owner,
			projectileModel[mode],
			charge_fraction
			);
	} */
	else if ( firetype[mode] == FT_MELEE ) // this is a weapon that fires like a sword
	{
		Vector melee_pos, melee_end;
		Vector dir;
		float damage;
		meansOfDeath_t meansofdeath;
		float knockback;
		
		if ( owner->isSubclassOf( Player ) )
		{
			Vector forward;

			Player *player = (Player *)owner;
			melee_pos = player->origin + Vector( 0.0f, 0.0f, player->client->ps.viewheight );

			player->GetVAngles().AngleVectors( &forward );

			melee_end = melee_pos + forward * _meleeLength[ mode ];
		}
		else
		{
			melee_pos = owner->centroid;
			melee_end = owner->centroid + Vector( owner->orientation[0] ) * _meleeLength[ mode ];
		}
		
		damage = bulletdamage[mode];
		knockback = bulletknockback[mode];
		
		meansofdeath = GetMeansOfDeath( mode );
		
		if ( owner->isSubclassOf( Player ) )
		{
			Player *player = (Player *)(Sentient *)owner;
			
			meansofdeath = player->changetMeansOfDeath( meansofdeath );
			damage = player->getDamageDone( damage, meansofdeath, true );
			
			knockback = (knockback + player->GetPlayerKnockback()) * player->GetKnockbackMultiplier();
		}
		
		Container<EntityPtr>victimlist;
		
		if ( MeleeAttack( melee_pos, melee_end, damage, owner, meansofdeath, _meleeWidth[ mode ], -_meleeHeight[ mode ], 
			 _meleeHeight[ mode ], knockback, true, &victimlist ) )
		{
			// Hit something

			Sound( "impact_flesh", CHAN_VOICE );
		}
		else
		{
			// Didn't hit anything that took damage

			// Try to hit the world since we didn't do any damage to anything
			trace_t trace = G_Trace( melee_pos, Vector( -8.0f, -8.0f, -8.0f ), Vector( 8.0f, 8.0f, 8.0f ), melee_end, owner, MASK_MELEE, false, "Weapon::Shoot" );
			
			Entity *victim = G_GetEntity( trace.entityNum );
			
			if ( victim && ( ( victim == world ) || ( victim->takedamage == DAMAGE_NO ) ) )
			{
				vec3_t  newangles;
				vectoangles( trace.plane.normal, newangles );
				WorldHitSpawn( mode, trace.endpos, newangles, 0.1f );
				
				str realname = this->GetRandomAlias( "impact_world" );
				if ( realname.length() > 1 )
					this->Sound( realname, CHAN_VOICE );
			}
		}
	}
	
	if ( !quiet )
	{
		if ( next_noise_time <= level.time )
		{
			BroadcastSound();
			next_noise_time = level.time + 1.0f;
		}
	}
}

//======================
//Weapon::SetAimAnim
//======================
void Weapon::SetAimAnim( Event *ev )
{
	str anim;
	
	anim     = ev->GetString( 1 );
	aimanim  = gi.Anim_NumForName( edict->s.modelindex, anim.c_str() );
	aimframe = ev->GetInteger( 2 );
}

//======================
//Weapon::SetOwner
//======================
void Weapon::SetOwner( Sentient *ent )
{
	assert( ent );
	if ( !ent )
	{
		// return to avoid any buggy behaviour
		return;
	}
	
	Item::SetOwner( ent );
	
	setOrigin( vec_zero );
	setAngles( vec_zero );
}

//======================
//Weapon::AmmoAvailable
//======================
int Weapon::AmmoAvailable( firemode_t mode )
{
	// Returns the amount of ammo the owner has that is available for use
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
	{
		warning( "Weapon::AmmoAvailable", "Invalid mode %d\n", mode );
		return 0;
	}
	
	// Make sure there is an owner before querying the amount of ammo
	if ( owner )
	{
		return owner->AmmoCount( ammo_type[mode] );
	}
	else
	{
		warning( "Weapon::AmmoAvailable", "Weapon does not have an owner.\n" );
		return 0;
	}
}

//======================
//Weapon::UnlimitedAmmo
//======================
qboolean Weapon::UnlimitedAmmo( firemode_t mode )
{
	if ( !owner )
	{
		return false;
	}
	
	if ( !ammorequired[mode] )
		return true;
	
	if ( !owner->isClient() || ( owner->flags & FL_GODMODE ) || multiplayerManager.checkFlag( MP_FLAG_INFINITE_AMMO ) )
	{
		return true;
	}
	else if ( !stricmp( ammo_type[mode], "None" ) )
	{
		return true;
	}
	
	return false;
}

//======================
//Weapon::HasInvAmmo
//======================
// Checks to see if we have any ammo in general
qboolean Weapon::HasInvAmmo( firemode_t mode )
{
	if ( !ammorequired[mode] )
		return false;
	
	return ( AmmoAvailable( mode ) >= ammorequired[mode] );
}

//======================
//Weapon::HasAmmo
//======================
qboolean Weapon::HasAmmo( firemode_t mode, int numShots )
{
	firemode_t clipToUse;

	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
	{
		warning( "Weapon::HasAmmo", "Invalid mode %d\n", mode );
		return false;
	}
	
	if ( !owner )
	{
		return false;
	}
	
	if ( UnlimitedAmmo( mode ) )
	{
		return true;
	}
	
	if ( burstmode[mode] && burstcount <= 0 ) 
	{
		return false;
	}

	if ( usesameclip )
	{
		clipToUse = FIRE_MODE1;
	}
	else
	{
		clipToUse = mode;
	}

	// If the weapon uses a clip, check for ammo in the right clip
	if ( ammo_clip_size[clipToUse] )
	{
		return HasAmmoInClip( mode, numShots );
	}
	else // Otherwise check if ammo is available in general
	{
		if ( !ammorequired[mode] )
			return false;
		
		return ( AmmoAvailable( mode ) >= ammorequired[mode] * numShots );
	}
}

//======================
//Weapon::HasAmmoInClip
//======================
qboolean Weapon::HasAmmoInClip( firemode_t mode, int numShots )
{
	firemode_t clipToUse;

	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
		warning( "Weapon::HasAmmoInClip", "Invalid mode %d\n", mode );

	if ( usesameclip )
	{
		clipToUse = FIRE_MODE1;
	}
	else
	{
		clipToUse = mode;
	}
	
	if ( ammo_clip_size[clipToUse] )
	{
		if ( ammo_in_clip[clipToUse] >= ammorequired[mode] * numShots )
		{
			return true;
		}
	}
	return false;
}

//======================
//Weapon::ForceState
//======================
void Weapon::ForceState( weaponstate_t state )
{
	weaponstate = state;
}

//======================
//Weapon::ReadyToFire
//======================
qboolean Weapon::ReadyToFire( firemode_t mode, qboolean playsound )
{
	// Make sure the weapon is in a good state

	if ( _canInterruptFiringState )
	{
		if ( weaponstate != WEAPON_READY && weaponstate != WEAPON_FIRING )
			return false;
	}
	else
	{
		if ( weaponstate != WEAPON_READY )
			return false;
	}

	// If the weapon doesn't take ammo, we're always ready.
	str ammotype = GetAmmoType( ( firemode_t )mode );
	if ( ammotype == "None" )
		return true;
	
	//if ( usesameclip )
	//	mode = FIRE_MODE1;
	
	if ( HasAmmo( mode ) )
	{
		return true;
	}
	
	if ( hasNoAmmoMode( mode ) )
	{
		return true;
	}
	
	if ( playsound && ( level.time > next_noammo_time ) && ( AmmoAvailable( mode ) < ammorequired[mode] ) )
	{
		Sound( "snd_noammo" );
		next_noammo_time = level.time + 0.25f;

		// If this is a bot, wipe out the rest of his useless ammo in the clip to prevent confusion

		if ( owner && owner->isSubclassOf( Player ) )
		{
			/* if ( player->edict->svflags & SVF_BOT )
			{
				ammo_in_clip[ mode ] = 0;
			}
			else */
			{
				// Auto switch to phaser

				if ( shouldAutoSwitch( mode ) )
				{
					autoSwitch();
				}

			}
		}
	}

	return false;
}

//======================
//Weapon::PutAway
//======================
void Weapon::PutAway( void )
{
	//	int i;
	//	int original_ammo_count;
	//	int new_ammo_count;
	
	if ( owner && owner->isSubclassOf( Actor ) )
	{
		Event *newEvent = new Event( EV_DisplayEffect );
		newEvent->AddString( "TransportOut" );
		newEvent->AddString( "FederationWeapons" );
		ProcessEvent( newEvent );
	}

	if ( _controllingProjectile )
	{
		toggleProjectileControl();
	}
	
	// set the putaway flag to true, so the state machine know to put this weapon away
	putaway = true;
	
	if ( !owner )
		return;

	Uninitialize();
	
	// Give ammo back to owner
	/*
	for( i = 0 ; i < MAX_FIREMODES ; i++ )
	{
		if ( ammo_in_clip[ i ] )
		{
			// Add ammo to owner
			original_ammo_count = owner->AmmoCount( ammo_type[ i ] );
			owner->GiveAmmo( ammo_type[ i ], ammo_in_clip[ i ] );
			new_ammo_count = owner->AmmoCount( ammo_type[ i ] );
	
			// Subtract ammo from weapon
	  
			ammo_in_clip[ i ] = ammo_in_clip[ i ] - (new_ammo_count - original_ammo_count);
		
			owner->AmmoAmountInClipChanged( ammo_type[ i ], ammo_in_clip[ i ] );
		}
	}*/
}

//======================
//Weapon::DetachFromOwner
//======================
void Weapon::DetachFromOwner( void )
{
	if ( edict->s.renderfx | RF_DONTDRAW )
		animate->StopAnimatingAtEnd();
	else
		animate->StopAnimating();

	DetachGun();
	weaponstate = WEAPON_HOLSTERED;
	current_attachToTag = "";

	Uninitialize();
}

//======================
//Weapon::AttachToOwner
//======================
void Weapon::AttachToOwner( weaponhand_t hand )
{
	AttachGun( hand , false );
	
	if ( targetidle )
		PostEvent(EV_Weapon_TargetIdleThink, TargetIdleTime);

	ForceIdle();
}

//======================
//Weapon::AttachToHolster
//======================
void Weapon::AttachToHolster( weaponhand_t hand )
{
	AttachGun( hand, true );
	animate->RandomAnimate( "holster", EV_Weapon_Idle );
}

//======================
//Weapon::Drop
//======================
qboolean Weapon::Drop( void )
{
	float radius;
	Vector temp;
	int i;
	
	if ( !owner )
	{
		return false;
	}
	
	if ( !IsDroppable() )
	{
		return false;
	}

	if ( multiplayerManager.inMultiplayer() && !multiplayerManager.checkRule( "weapon-candrop", true, NULL ) )
	{
		return false;
	}
	
	DetachGun();

	// Get rid of special effect stuff

	CancelEventsOfType( EV_DisplayEffect );
	CancelEventsOfType( EV_FadeIn );
	clearCustomShader();
	setAlpha( 1.0f );
	edict->s.renderfx &= ~RF_FORCE_ALPHA_EFFECTS;
	edict->s.renderfx &= ~RF_FORCE_ALPHA;

	if ( _controllingProjectile )
	{
		toggleProjectileControl();
	}

	temp[ 1 ] = 64.0;
	temp[ 2 ] = 48.0;
	if ( owner )
	{
		setOrigin( owner->origin + temp );
	}
	else
	{
		setOrigin( origin + temp );
	}
	
	// hack to fix the bounds when the gun is dropped
	// once dropped reset the rotated bounds
	flags |= FL_ROTATEDBOUNDS;
	
	if ( ( mins == vec_zero ) && ( maxs == vec_zero ) )
	{
		gi.CalculateBounds( edict->s.modelindex, edict->s.scale, mins, maxs );
		radius = ( mins - maxs ).length() * 2.0f;
		mins.x = mins.y = -radius;
		maxs.x = maxs.y = radius;
		setSize( mins, maxs );
	}
	
	// stop animating
	ForceIdle();
	animate->SetFrame( 0 );
	
	// drop the weapon
	PlaceItem();
	if ( owner )
	{
		temp[ 0 ] = G_CRandom( 50.0f );
		temp[ 1 ] = G_CRandom( 50.0f );
		temp[ 2 ] = 250.0f;
		velocity = owner->velocity * 0.5f + temp;
		setAngles( owner->angles );
	}
	
	avelocity = Vector( 0.0f, G_CRandom( 360.0f ), 0.0f );

	// Get rid of all of the ammo in the weapon

	for ( i = 0 ; i < MAX_FIREMODES ; i++ )
	{
		ammo_in_clip[ i ] = 0;
	}
	
	// FIXME - Make this work right if we need it
	/*
	if ( owner && owner->isClient() )
	{
		spawnflags |= DROPPED_PLAYER_ITEM;
		if ( ammo_clip_size )
			startammo = ammo_in_clip;
		else
			startammo = 0;
	
		// If owner is dead, put all his ammo of that type in the gun.
		if ( owner->deadflag )
		{
			startammo = AmmoAvailable();
		}
	}
	else
	{
		spawnflags |= DROPPED_ITEM;
		if ( ammo_clip_size && ammo_in_clip )
			startammo = ammo_in_clip;
		else
			startammo >>= 2;
	  
		if ( startammo == 0 )
		{
			startammo = 1;
		}
	}
	*/
	
	// Wait some time before the last owner can pickup this weapon
	last_owner = owner;
	last_owner_trigger_time = level.time + 2.5f;
	
	// Cancel reloading events
	CancelEventsOfType( EV_Weapon_DoneReloading );
	CancelEventsOfType( EV_Weapon_DoneReloadingBurst );
	
	// Remove this from the owner's item list
	if ( owner )
	{
		owner->RemoveItem( this );
	}
	
	owner = NULL;

	if ( multiplayerManager.inMultiplayer() && gi.Anim_NumForName( edict->s.modelindex, "idle_onground" ) >= 0 )
	{
		animate->RandomAnimate( "idle_onground" );
		edict->s.eType = ET_MODELANIM;
	}

	if ( multiplayerManager.inMultiplayer() )
	{
		edict->s.renderfx |= RF_FULLBRIGHT;
	}

	// Fade out dropped weapons, to keep down the clutter
	PostEvent( EV_FadeOut, 30.0f );

	Uninitialize();
	return true;
}

//======================
//Weapon::Charge
//======================
void Weapon::Charge( firemode_t mode )
{
	Q_UNUSED(mode);
}

//======================
//Weapon::ReleaseFire
//======================
void Weapon::ReleaseFire( firemode_t mode, float charge_time )
{
	// Calculate and store off the charge fraction to use when the weapon actually shoots
	
	// Clamp to max_charge_time
	if ( charge_time > max_charge_time[mode] )
		charge_fraction = 1.0f;
	else
		charge_fraction = charge_time / max_charge_time[mode];
	
	// Call regular fire function
	Fire( mode );
}

//======================
//Weapon::Fire
//======================
// This function is called from the state machine
// to tell the tiki which weapon animation to play
// The tiki file will most likely post the "shoot" event
// shortly after this function is called
void Weapon::Fire( firemode_t mode )
{
	Event *done_event=NULL;
	firemode_t realmode;
	Vector pos;
	//char anim[128];
	str fireAnimName;
	
	// Sanity check the mode
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
		warning( "Weapon::Fire", "Invalid mode %d\n", mode );
	
	realmode = mode;
	
	// Override the firemode if we're a switch weapon
	if ( switchmode )
		mode = curmode;
	else
		curmode = mode;
	
	if ( next_fire_time[mode] > level.time )
		return;
	
	// Set the fire timer if we have one
	if ( fire_timer[mode] > 0.0f )
		next_fire_time[mode] = fire_timer[mode] + level.time;
	
	// If we are in loopfire mode, then we don't pass a DoneFiring event
	if ( !loopfire[mode] )
	{
		// The DoneFiring event requires to know the firing mode so save that off in the event
		done_event = new Event( EV_Weapon_DoneFiring );
		done_event->AddInteger( mode );
	}
	
	/*
	if ( fullanimfire[mode] )
	{
		donefiring = false;
		done_event = new Event ( EV_Weapon_DoneFiring );
		done_event->AddInteger( mode );
	}
	*/
	
	// Set the state of the weapon to FIRING
	weaponstate = WEAPON_FIRING;
	
	if ( shootingSkin )
		ChangeSkin( shootingSkin, true );
	
	// Cancel any old done firing events
	CancelEventsOfType( EV_Weapon_DoneFiring );
	
	// Play the correct animation
	if ( !switchmode )
	{
		if ( mode == FIRE_MODE1 )
			fireAnimName = "fire";
		//animate->RandomAnimate( "fire", done_event );
		else if ( mode == FIRE_MODE2 )
			fireAnimName = "alternatefire";
		//animate->RandomAnimate( "alternatefire", done_event );
	}
	else
	{
		//sprintf(anim,"fire_mode%d",(int)mode+1);
		fireAnimName = "fire_mode";
		fireAnimName += (int)mode+1;
		//if ( animate->HasAnim( anim ) )
		//	animate->RandomAnimate( anim, done_event );
	}
	
	if ( world && world->isThingBroken( item_name.c_str() ) )
	{
		fireAnimName += "_broken";
		animate->RandomAnimate( fireAnimName.c_str(), done_event );
		return;
	}
	else if ( !HasAmmo( mode ) && hasNoAmmoMode( mode ) )
	{
		fireAnimName += "_noammo";
	}
	else if ( _spreadTime[ mode ] && _spreadAnims[ mode ] )
	{
		float firingTime;
		int animNum;

		firingTime = level.time - startfiretime;

		animNum = 1 + ( firingTime / _spreadTime[ mode ] ) * ( _spreadAnims[ mode ] + 1 );

		if ( animNum > _spreadAnims[ mode ] )
			animNum = _spreadAnims[ mode ];

		fireAnimName += "_spread";
		fireAnimName += animNum;
	}
	
	animate->RandomAnimate( fireAnimName.c_str(), done_event );
	
	// Use up the appropriate amount of ammo, it's already been checked that we have enough
	//if ( usesameclip )
	//	mode = FIRE_MODE1;
	
	if ( !loopfire[mode] )
	{
		if ( ammorequired[mode] != ammorequired[realmode] )
			UseAmmo(ammorequired[realmode], mode);
		else
			UseAmmo( ammorequired[mode], mode );
		if ( mode != realmode )
		{
			if ( burstmode[realmode] )
				burstcount -= ammorequired[realmode];
		}
	}
}

//======================
//Weapon::DetachGun
//======================
void Weapon::DetachGun( void )
{
	if ( attached )
	{
		StopSound( CHAN_WEAPONIDLE );
		attached = false;
		detach();
		hideModel();

		clearDisplayEffects();

		if ( lastValid )
		{
			// Restore the last scale

			setScale( lastScale );
			lastValid = false;
		}
	}
}

//======================
//Weapon::AttachGun
//======================
void Weapon::AttachGun( weaponhand_t hand, qboolean holstering )
{
	int tag_num = 0;
	
	if ( !owner )
	{
		current_attachToTag = "";
		return;
	}
	
	if ( attached )
	{
		DetachGun();
	}
	
	if ( holstering )
	{
		// Save off these values if we are holstering the weapon.  We will restore them when
		// the users raises the weapons again.
		lastAngles = this->angles;
		lastScale  = this->edict->s.scale;
		lastValid  = true;
	}
	else
	{
		lastScale  = this->edict->s.scale;
		lastValid  = true;

		if ( owner->isSubclassOf( Player ) )
		{
			if ( mp_bigGunMode->integer )
				setScale( _weildedScale * 2.0f );
			else
				setScale( _weildedScale );
		}
	}
	/* else if ( lastValid )
	{

		// Restore the last
		setScale( lastScale );
		setAngles( lastAngles );
		lastValid = false;
	} */

	
	switch( hand )
	{
	case WEAPON_LEFT:
		if ( holstering )
		{
			current_attachToTag = leftholster_attachToTag;
			setAngles( leftHolsterAngles );
			setScale( holsterScale );
		}
		else
		{
			current_attachToTag = left_attachToTag;
		}
		break;
	case WEAPON_RIGHT:
	case WEAPON_DUAL:
		if ( holstering )
		{
			current_attachToTag = rightholster_attachToTag;
			setAngles( rightHolsterAngles );
			setScale( holsterScale );
		}
		else
		{
			current_attachToTag = right_attachToTag;
		}
		break;
		
	default:
		warning( "Weapon::AttachGun", "Invalid hand for attachment of weapon specified" );
		break;
	}
	
	if ( !current_attachToTag.length() )
		return;
	
	tag_num = gi.Tag_NumForName( owner->edict->s.modelindex, this->current_attachToTag.c_str() );
	
	NoLerpThisFrame();
	if	( tag_num >= 0 )
	   {
		attached = true;
		
		attach( owner->entnum, tag_num, false );
		
		showModel();
		setOrigin();

		if ( owner && owner->isSubclassOf( Actor ) )
		{
			Event *newEvent = new Event( EV_DisplayEffect );
			newEvent->AddString( "TransportIn" );
			newEvent->AddString( "FederationWeapons" );
			ProcessEvent( newEvent );
		}
	}
	else
	{
		warning( "Weapon::AttachGun", "Attachment of weapon to tag \"%s\": Tag Not Found\n", this->current_attachToTag.c_str() );
	}
}

//======================
//Weapon::GiveStartingAmmo
//======================
void Weapon::GiveStartingAmmo( Event *ev )
{
	str   ammotype;
	int   mode;
	Entity *entityToGiveAmmo;
	Sentient *sentient;
	
	if ( ev->NumArgs() > 0 )
		entityToGiveAmmo = ev->GetEntity( 1 );
	else
		entityToGiveAmmo = owner;
	
	if ( !entityToGiveAmmo || !entityToGiveAmmo->isSubclassOf( Sentient ) )
	{
		assert( entityToGiveAmmo );
		
		warning( "Weapon::GiveStartingAmmo", "Could not give ammo\n" );
		return;
	}
	
	sentient = reinterpret_cast<Sentient *>(entityToGiveAmmo);
	
	// Give the player the starting ammo
	
	for ( mode=FIRE_MODE1; mode<MAX_FIREMODES; mode++ )
	{
		ammotype = GetAmmoType( ( firemode_t )mode );
		
		if ( ammotype.length() )
		{
			sentient->GiveAmmo( ammotype, this->GetStartAmmo( ( firemode_t )mode ), false );
		}
	}
}

void Weapon::giveAmmoBoost( Event *ev )
{
	str   ammotype;
	int   mode;
	Entity *entityToGiveAmmo;
	Sentient *sentient;
	
	
	// Get the sentient to give the ammo boost to

	entityToGiveAmmo = ev->GetEntity( 1 );
	
	if ( !entityToGiveAmmo || !entityToGiveAmmo->isSubclassOf( Sentient ) )
	{
		assert( entityToGiveAmmo );
		
		warning( "Weapon::GiveAmmoBoost", "Could not give ammo\n" );
		return;
	}
	
	sentient = reinterpret_cast<Sentient *>(entityToGiveAmmo);
	
	// Give the sentient the ammo boost
	
	for ( mode = FIRE_MODE1 ; mode < MAX_FIREMODES ; mode++ )
	{
		ammotype = GetAmmoType( ( firemode_t )mode );
		
		if ( ammotype.length() )
		{
			sentient->GiveAmmo( ammotype, this->getAmmoBoost( ( firemode_t )mode ), true );
		}
	}
}

//======================
//Weapon::PickupWeapon
//======================
void Weapon::PickupWeapon( Event *ev )
{
	Sentient       *sen;
	Entity         *other;
	Weapon         *weapon;
	qboolean       hasweapon;
	//qboolean       giveammo[MAX_FIREMODES];
	//int            mode;
	
	other = ev->GetEntity( 1 );
	
	if ( !other )
		return;
	
	if ( !other->isSubclassOf( Sentient ) )
	{
		return;
	}
	
	sen = ( Sentient * )other;

	if ( !Pickupable( other ) )
		return;
	
	// If this is the last owner, check to see if he can pick it up
	if ( ( sen == last_owner ) && ( level.time < last_owner_trigger_time ) )
	{
		return;
	}
	
	hasweapon = sen->HasItem( item_name );
	
	
	if ( hasweapon && !multiplayerManager.checkFlag( MP_FLAG_WEAPONS_STAY ) )
	{
		Event *event;
		
		event = new Event( EV_Weapon_GiveAmmoBoost );
		event->AddEntity( other );
		ProcessEvent( event );
		
		ItemPickup( other, false );
	}
	else
	{
		weapon = ( Weapon * )ItemPickup( other );
		
		if ( !weapon )
		{
			// Item Pickup failed, so don't give ammo either.
			return;
		}
	}
	/*   else
	{
		for ( mode = FIRE_MODE1; mode < MAX_FIREMODES; mode++ )
		{
			giveammo[mode] = ( sen->isClient() && ammo_type[mode].length() && startammo[mode] );
	
			if ( !giveammo[mode] )
			{
				return;
			}
	  
			sen->GiveAmmo( ammo_type[mode].c_str(), startammo[mode] );
		}
	} */
}

//======================
//Weapon::ForceIdle
//======================
void Weapon::ForceIdle( void )
{
	// Force the weapon to the idle animation
	weaponstate = WEAPON_READY;
	
	// Clear our fire timer
	//INITIALIZE_WEAPONMODE_VAR(next_fire_time, 0.0);
	
	float r = G_Random(100.0f);
	char anim[128];
	
	
	int animNum = gi.Anim_NumForName( edict->s.modelindex, "idle_onground" );
	if ( multiplayerManager.inMultiplayer() && animNum >= 0	&& animate->CurrentAnim() == animNum )
	{
		return;
	}
	
	// If this is a switchmode weapon, we have a very different idle system
	if ( switchmode )
	{
		if ( r < 5.0f )
		{
			sprintf(anim,"idle_rare_mode%d",(int)curmode+1);
			if ( animate->HasAnim( anim ) )
			{
				animate->RandomAnimate( anim );
				return;
			}
		}
		
		if ( r < 15.0f )
		{
			sprintf(anim,"idle_uncommon_mode%d",(int)curmode+1);
			if ( animate->HasAnim( anim ) )
			{
				animate->RandomAnimate( anim );
				return;
			}
		}
		
		sprintf(anim,"idle_common_mode%d",(int)curmode+1);
		if ( animate->HasAnim( anim ) )
		{
			animate->RandomAnimate( anim );
			return;
		}
		
		// We shouldn't get here if the anims are set up correctly
		// for the switch mode weapon
		animate->RandomAnimate("idle");
		return;
	}
	
	if ( r < 5.0f && animate->HasAnim("idle_rare") )
	{
		animate->RandomAnimate( "idle_rare" );
		return;
	}
	
	if ( r < 15.0 && animate->HasAnim("idle_uncommon") )
	{
		animate->RandomAnimate("idle_uncommon");
		return;
	}
	
	if ( animate->HasAnim("idle_common") )
	{
		animate->RandomAnimate( "idle_common" );
		return;
	}
	
	// We shouldn't get here if the anims are set up correctly
	animate->RandomAnimate("idle");
}

//======================
//Weapon::DoneRaising
//======================
void Weapon::DoneRaising( Event * )
{
	Event *event;
	
	weaponstate = WEAPON_READY;
	ForceIdle();
	
	if ( !owner )
	{
		PostEvent( EV_Remove, 0.0f );
		return;
	}
	
	event = new Event( EV_Weapon_DoneReloading );
	event->AddInteger( FIRE_MODE1 );
	ProcessEvent( event );
	
	event = new Event( EV_Weapon_DoneReloading );
	event->AddInteger( FIRE_MODE2 );
	ProcessEvent( event );
	
	if ( targetidle )
		PostEvent(EV_Weapon_TargetIdleThink, TargetIdleTime);
}

//----------------------------------------------------------------
// Name:			playAnim
// Class:			Weapon
//
// Description:		Plays the specified animation and marks its weapon state as animating
//
// Parameters:		const str &animName			- name of the animation to play
//					bool animatingFlag			- defaults to true
//
// Returns:			none
//----------------------------------------------------------------
void Weapon::playAnim( const str &animName, bool animatingFlag )
{ 
	if ( animate->HasAnim( animName.c_str() ) )
	{
		if ( animatingFlag )
			weaponstate = WEAPON_ANIMATING;

		SetAnim( animName, EV_Weapon_DoneAnimating );
	}
}



//===============================================================
// Name:		animName
// Class:		Weapon
//
// Description: Sets the weapons animation.
// 
// Parameters:	const str& -- 
//
// Returns:		None
// 
//===============================================================
void Weapon::SetAnim( const str	&animName, Event *endevent, bodypart_t bodypart )
{
	assert( animate );
	animate->RandomAnimate( animName.c_str(), endevent, bodypart );
	AddEffectsAnims();
}


//===============================================================
// Name:		animName
// Class:		Weapon
//
// Description: Sets the weapons animation.
// 
// Parameters:	const str& -- 
//
// Returns:		None
// 
//===============================================================
void Weapon::SetAnim( const str	&animName, const Event &endevent, bodypart_t bodypart )
{
	assert( animate );
	animate->RandomAnimate( animName.c_str(), endevent, bodypart );
	AddEffectsAnims();
}

//===============================================================
// Name:		PassToAnimate
// Class:		Weapon
//
// Description: Passes the event to the animation object, and gives
//				the weapon a chance to add additional special effects.
//				If the animation object doesn't exist, creates it.
// 
// Parameters:	Event* -- an animation related event.
//
// Returns:		None
// 
//===============================================================
void Weapon::PassToAnimate( Event *ev )
{
	if ( !animate )
	{
		animate = new Animate( this );
	}
	Event *new_event = new Event( ev );
	animate->ProcessEvent( new_event );
	AddEffectsAnims();
}

//===============================================================
// Name:		AddEffectsAnims
// Class:		Weapon
//
// Description: Adds the effect anims for a weapon.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void Weapon::AddEffectsAnims( void )
{
}

//======================
//Weapon::DoneAnimating
//======================
void Weapon::DoneAnimating( Event * )
{
	weaponstate = WEAPON_READY;
	
	if ( !owner )
		PostEvent( EV_Remove, 0.0f );
	
	return;
}

//======================
//Weapon::ClientFireDone
//======================
void Weapon::ClientFireDone( void )
{
	// This is called when the client's firing animation is done
}

//======================
//Weapon::DoneFiring
//======================
void Weapon::DoneFiring( Event * )
{
	/*
	firemode_t mode = (firemode_t)ev->GetInteger( 1 );

  
	if ( fullanimfire[mode] )
	{
		donefiring = true;	
		return;
	}
	*/
	
	if ( shootingSkin )
		ChangeSkin( shootingSkin, false );
	
	// This is called when the weapon's firing animation is done 
	// If this weapon has a fire_stop animation, play it (tiki will force us back to idle)
	if ( animate->HasAnim("fire_stop") )
		animate->RandomAnimate("fire_stop");
	else
		ForceIdle();
	
	// Check to see if the auto_putaway flag is set, and the weapon is out of ammo.  If so, then putaway the
	// weapon.
	if (
		( !HasAmmo( FIRE_MODE1 )   || ( !stricmp( ammo_type[ FIRE_MODE1 ], "None" ) ) ) &&
		( !HasAmmo( FIRE_MODE2 ) || ( !stricmp( ammo_type[ FIRE_MODE2 ], "None" ) ) ) &&
		auto_putaway
		)
	{
		PutAway();
	}
}

//======================
//Weapon::DoneReloading
//======================
void Weapon::DoneReloading( Event *ev )
{
	int         amount;
	int         amount_used;
	firemode_t  mode;
	firemode_t  clipToUse;
	
	// Get the mode from the passed in event
	mode = (firemode_t)ev->GetInteger( 1 );
	
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
		warning( "Weapon::DoneReloading", "Invalid mode %d\n", mode );
	
	/* if ( regenAmount > 0 )
	{
		int tmpamount = 0;
		Ammo *ammo;
		ammo = owner->FindAmmoByName( ammo_type[mode] );
		if ( ammo ) 
		{
			tmpamount = (ammo->getMaxAmount() / 2) - owner->AmmoCount( ammo_type[mode] );
			owner->GiveAmmo( ammo_type[ mode ],  tmpamount);
			ForceIdle();
			return;
		}
	} */
	
	// If we need to do a REAL reload (not a burst reload) we skip the test
	// altogether.
	/* firemode_t realmode = mode;
	if ( usesameclip )
		mode = FIRE_MODE1;
	if ( !(ammo_clip_size[mode] && !ammo_in_clip[mode] && AmmoAvailable(mode)) )
	{
		int tmp = 0;
		if ( switchmode )
			tmp = burstmode[curmode];
		else
			tmp = burstmode[realmode];
	
		if ( tmp )
		{
			int tmpamount = 0;
			Ammo *ammo;
			ammo = owner->FindAmmoByName( ammo_type[mode] );
			if ( ammo ) 
			{
				tmpamount = owner->AmmoCount( ammo_type[mode] );
	  
				// Try the clip ammo if we have none in our main inventory
				if ( tmpamount == 0 ) 
					tmpamount = ammo_in_clip[mode];
		
				if ( tmpamount > burstcountmax )
					burstcount = burstcountmax;
				else 
					burstcount = tmpamount;
		  
				// We're out of ammo
				if ( burstcount == 0 )
					burstcount = -1;
			
				ForceIdle();
				return;
			}
		}
	} */
	
	if ( usesameclip )
		clipToUse = FIRE_MODE1;
	else
		clipToUse = mode;
	
	// Calc the amount the clip should get
	amount = ammo_clip_size[clipToUse] - ammo_in_clip[clipToUse];
	
	assert( owner );
	if ( owner && owner->isClient() && !UnlimitedAmmo( mode ) )
	{
		// use up the ammo from the player
		amount_used = owner->UseAmmo( ammo_type[mode], amount );
		
		// Stick it in the clip
		if ( ammo_clip_size[clipToUse] )
			ammo_in_clip[clipToUse] = amount_used + ammo_in_clip[clipToUse];
		
		assert( ammo_in_clip[clipToUse] <= ammo_clip_size[clipToUse] );
		if ( ammo_in_clip[clipToUse] > ammo_clip_size[clipToUse] )
			ammo_in_clip[clipToUse] = ammo_clip_size[clipToUse];
		
		if ( burstmode[curmode] && ( ammo_in_clip[clipToUse] >= burstcountmax ) )
			burstcount = burstcountmax;
	}
	owner->AmmoAmountInClipChanged( ammo_type[mode], ammo_in_clip[clipToUse] );
	
	ForceIdle();
}

void Weapon::DoneReloadingBurst( Event *ev )
{
	//int         amount;
	//int         amount_used;
	firemode_t  mode;
	int		inBurstMode;
	firemode_t clipToUse; 

	// Get the mode from the passed in event
	mode = (firemode_t)ev->GetInteger( 1 );

	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	if ( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
		warning( "Weapon::DoneReloading", "Invalid mode %d\n", mode );

	// If we need to do a REAL reload (not a burst reload) we skip the test
	// altogether.
	firemode_t realmode = mode;
	//if ( usesameclip )
	//	mode = FIRE_MODE1;

	// Make sure we are actually in burst mode

	inBurstMode = false;

	if ( switchmode )
		inBurstMode = burstmode[curmode];
	else
		inBurstMode = burstmode[realmode];

	if ( !inBurstMode )
		return;

	// Add to the burst count

	int tmpamount = 0;
	Ammo *ammo;

	ammo = owner->FindAmmoByName( ammo_type[mode] );

	if ( ammo ) 
	{
		tmpamount = owner->AmmoCount( ammo_type[mode] );
		
		// Try the clip ammo if we have none in our main inventory

		if ( usesameclip )
			clipToUse = FIRE_MODE1;
		else
			clipToUse = mode;

		if ( tmpamount == 0 ) 
			tmpamount = ammo_in_clip[clipToUse];

		if ( tmpamount > burstcountmax )
			burstcount = burstcountmax;
		else 
			burstcount = tmpamount;
		
		// We're out of ammo

		if ( burstcount == 0 )
			burstcount = -1;

		ForceIdle();
		return;
	}
}

qboolean Weapon::CheckReload( void )
{
	return CheckReload( curmode );
}

//======================
//Weapon::CheckReload - Checks to see if we NEED to reload the weapon
//======================
qboolean Weapon::CheckReload( firemode_t mode )
{
	int inBurstMode;
	firemode_t clipToUse;

	// Check to see if the weapon needs to be reloaded
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	if ( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
	{
		warning( "Weapon::CheckReload", "Invalid mode %d\n", mode );
		return false;
	}

	if ( !_autoReload )
	{
		return false;
	}

	if ( UnlimitedAmmo( mode ) )
	{
		return false;
	}

	if ( putaway )
	{
		return false;
	}

	if ( weaponstate != WEAPON_READY )
		return false;
	
	// If we're a burst mode weapon, see if we need to "reload"

	inBurstMode = false;

	if ( switchmode )
		inBurstMode = burstmode[curmode];
	else
		inBurstMode = burstmode[mode];

	firemode_t oldmode = mode;

	if ( inBurstMode && ( weaponstate != WEAPON_RELOADING ) && ( burstcount <= 0 ) )
	{
		Event *doneReloadingEvent;
		doneReloadingEvent = new Event( EV_Weapon_DoneReloadingBurst );
		doneReloadingEvent->AddInteger( oldmode );

		if ( _burstModeDelay[ mode ] > 0.0f )
		{
			animate->RandomAnimate( "reload_burst" );
			PostEvent( doneReloadingEvent, _burstModeDelay[ mode ] );
		}
		else
		{
			animate->RandomAnimate( "reload_burst", doneReloadingEvent );
		}

		weaponstate = WEAPON_RELOADING;
		return true;
	}

	mode = oldmode;

	if ( usesameclip )
		clipToUse = FIRE_MODE1;
	else
		clipToUse = mode;

	if ( ammo_clip_size[clipToUse] && ( ammo_in_clip[clipToUse] < ammorequired[mode] ) && ( AmmoAvailable( mode ) >= ammorequired[mode] ) )
	{
		Event *ev1;

		ev1 = new Event( EV_Weapon_DoneReloading );
		ev1->AddInteger( mode );

		if ( animate->HasAnim( "reload_normal" ) && !multiplayerManager.skipWeaponReloads() )
		{
			weaponstate = WEAPON_RELOADING;
			animate->RandomAnimate( "reload_normal", ev1 );
		}
		else
		{
			ProcessEvent( ev1 );
		}
		return true;
	}

	// We're a regenerating weapon out of ammo
	/* if ( !AmmoAvailable(mode) && ( regenAmount > 0 ) )
	{
		Event *ev1;

		ev1 = new Event( EV_Weapon_DoneReloading );
		ev1->AddInteger( mode );

		if ( animate->HasAnim( "idle_special" ) )
		{
			weaponstate = WEAPON_RELOADING;
			animate->RandomAnimate( "idle_special", ev1 );
		}
		else
		{
			ProcessEvent( ev1 );
		}
		return true;
	} */

	return false;
}

//======================
//Weapon::ForceReload - Reload the clip if it's not full already
//======================
qboolean Weapon::ForceReload( void )
{
	// Only primary mode can reload
	firemode_t mode = FIRE_MODE1;
	firemode_t clipToUse;
	
	// Weapon has to be in the ready state

	if ( !canReload() )
		return false;
	
	// Make sure it's not put away
	if ( putaway )
		return false;
	
	// We can't reload regenerating weapons
	if ( _regenAmount[ FIRE_MODE1 ] > 0 )
		return false;
	
	if ( usesameclip )
		clipToUse = FIRE_MODE1;
	else
		clipToUse = mode;
	
	// If our clip is not full and we have ammo available...
	// HACK: targetidle is currently only used on the TOW weapon,
	// so instead of making an entirely new event JUST for this, we use it here.
	if ( ( ( ammo_clip_size[clipToUse] != ammo_in_clip[clipToUse] ) && AmmoAvailable( mode ) ) || targetidle)
	{
		Event *ev1;
		
		ev1 = new Event( EV_Weapon_DoneReloading );
		ev1->AddInteger( mode );
		
		if ( animate->HasAnim( "reload_normal" ) )
		{
			weaponstate = WEAPON_RELOADING;
			animate->RandomAnimate( "reload_normal", ev1 );
		}
		else
		{
			ProcessEvent( ev1 );
		}
		return true;
	}
	
	return false;
}

//======================
//Weapon::Idle
//======================
void Weapon::Idle( Event * )
{
	ForceIdle();
}

//======================
//Weapon::GetMaxRange
//======================
float	Weapon::GetMaxRange( void )
{
	return maxrange;
}

//======================
//Weapon::GetMinRange
//======================
float	Weapon::GetMinRange( void )
{
	return minrange;
}

//======================
//Weapon::SetMaxRangeEvent
//======================
void Weapon::SetMaxRangeEvent( Event *ev )
{
	maxrange = ev->GetFloat( 1 );
}

//======================
//Weapon::SetMinRangeEvent
//======================
void Weapon::SetMinRangeEvent( Event *ev )
{
	minrange = ev->GetFloat( 1 );
}

//======================
//Weapon::NotDroppableEvent
//======================
void Weapon::NotDroppableEvent( Event * )
{
	notdroppable = true;
}

//======================
//Weapon::SetMaxRange
//======================
void Weapon::SetMaxRange( float val )
{
	maxrange = val;
}

//======================
//Weapon::SetMinRange
//======================
void Weapon::SetMinRange( float val )
{
	minrange = val;
}

//======================
//Weapon::WeaponSound
//======================
void Weapon::WeaponSound( Event *ev )
{
	Event *e;
	
	// Broadcasting a sound can be time consuming.  Only do it once in a while on really fast guns.
	if ( nextweaponsoundtime > level.time )
	{
		if ( owner )
		{
			owner->BroadcastSound(SOUND_RADIUS, SOUNDTYPE_WEAPONFIRE);;
		}
		else
		{
			BroadcastSound(SOUND_RADIUS, SOUNDTYPE_WEAPONFIRE);;
		}
		return;
	}
	
	if ( owner )
	{
		e = new Event( ev );
		owner->ProcessEvent( e );
	}
	else
	{
		Item::BroadcastSound();
	}
	
	// give us some breathing room
	nextweaponsoundtime = level.time + 0.4f;
}

//======================
//Weapon::Removable
//======================
qboolean Weapon::Removable( void )
{
	if ( multiplayerManager.checkFlag( MP_FLAG_WEAPONS_STAY ) && Respawnable() )
		return false;
	else
		return true;
}

//======================
//Weapon::Pickupable
//======================
qboolean Weapon::Pickupable( Entity *other )
{
	Sentient *sen;
	
	if ( !other->isSubclassOf( Sentient ) )
	{
		return false;
	}
	else if ( !other->isClient() )
	{
		return false;
	}

	if ( getSolidType() == SOLID_NOT )
	{
		return NULL;
	}

	if ( multiplayerManager.inMultiplayer() && other->isSubclassOf( Player ) )
	{
		if ( !multiplayerManager.canPickup( (Player *)other, getMultiplayerItemType(), getName().c_str() ) )
			return false;
	}
	
	sen = ( Sentient * )other;
	
	//FIXME
	// This should be in player
	
	// If we have the weapon and weapons stay, then don't pick it up
	if ( ( multiplayerManager.checkFlag( MP_FLAG_WEAPONS_STAY ) ) && Respawnable() )
	{
		Weapon   *weapon;
		
		weapon = ( Weapon * )sen->FindItem( getName() );
		
		if ( weapon )
			return false;
	}
	
	return true;
}

//======================
//Weapon::AutoChange
//======================
qboolean Weapon::AutoChange( void )
{
	return true;
}

//======================
//Weapon::ClipAmmo
//======================
int Weapon::ClipAmmo( firemode_t mode )
{
	firemode_t clipToUse;
	
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
		warning( "Weapon::ClipAmmo", "Invalid mode %d\n", mode );
	
	if ( usesameclip )
		clipToUse = FIRE_MODE1;
	else
		clipToUse = mode;
	
	if (ammo_clip_size[clipToUse])
		return ammo_in_clip[clipToUse];
	else
		return -1;
}

//======================
//Weapon::ProcessWeaponCommandsEvent
//======================
void Weapon::ProcessWeaponCommandsEvent( Event *ev )
{
	int index;
	
	index = ev->GetInteger( 1 );
	ProcessInitCommands( index );
}

//======================
//Weapon::SetActionLevelIncrement
//======================
void Weapon::SetActionLevelIncrement( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	action_level_increment[firemodeindex] = ev->GetInteger( 1 );
}

//======================
//Weapon::ActionLevelIncrement
//======================
int Weapon::ActionLevelIncrement( firemode_t mode )
{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) )
		return action_level_increment[mode];
	else
	{
		warning( "Weapon::ActionLevelIncrement", "Invalid mode %d\n", mode );
		return 0;
	}
}

//======================
//Weapon::IsDroppable
//======================
qboolean Weapon::IsDroppable( void )
{
	if ( notdroppable )
	{
		return false;
	}
	else
	{
		return true;
	}
}

//======================
//Weapon::SetFireType
//======================
void Weapon::SetFireType( Event *ev )
{
	str ftype;
	
	ftype = ev->GetString( 1 );
	
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	
	if ( !ftype.icmp( "projectile" ) )
		firetype[firemodeindex] = FT_PROJECTILE;
	else if ( !ftype.icmp( "bullet" ) )
		firetype[firemodeindex] = FT_BULLET;
	else if ( !ftype.icmp( "melee" ) )
		firetype[firemodeindex] = FT_MELEE;
	else if ( !ftype.icmp( "special_projectile" ) )
		firetype[firemodeindex] = FT_SPECIAL_PROJECTILE;
	else if ( !ftype.icmp( "none" ) )
		firetype[firemodeindex] = FT_NONE;
	else if ( !ftype.icmp( "explosion" ) )
		firetype[firemodeindex] = FT_EXPLOSION;
	else if ( !ftype.icmp( "triggerProjectile" ) )
		firetype[firemodeindex] = FT_TRIGGER_PROJECTILE;
	else if ( !ftype.icmp( "controlProjectile" ) )
		firetype[firemodeindex] = FT_CONTROL_PROJECTILE;
	else if ( !ftype.icmp( "controlZoom" ) )
		firetype[firemodeindex] = FT_CONTROL_ZOOM;
	else
		warning( "Weapon::SetFireType", "unknown firetype: %s\n", ftype.c_str() );
}

//======================
//Weapon::GetFireType
//======================
firetype_t Weapon::GetFireType( firemode_t mode )
{
	return firetype[mode];
}

//======================
//Weapon::SetProjectile
//======================
void Weapon::SetProjectile( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	projectileModel[firemodeindex] = ev->GetString( 1 );
	CacheResource( projectileModel[firemodeindex].c_str(), this );
}

//======================
//Weapon::SetBulletDamage
//======================
void Weapon::SetBulletDamage( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletdamage[firemodeindex] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetBulletKnockback
//======================
void Weapon::SetBulletKnockback( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletknockback[firemodeindex] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetBulletRange
//======================
void Weapon::SetBulletRange( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletrange[firemodeindex] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetRange
//======================
void Weapon::SetRange( Event *ev )
{
	SetBulletRange( ev );
}

//======================
//Weapon::SetBulletCount
//======================
void Weapon::SetBulletCount( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletcount[firemodeindex] = ev->GetFloat( 1 );
}

void Weapon::SetBulletSpread( float spreadX , float spreadY )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletspread[firemodeindex].x = spreadX;
	bulletspread[firemodeindex].y = spreadY;
}

//======================
//Weapon::SetBulletSpread
//======================
void Weapon::SetBulletSpread( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletspread[firemodeindex].x = ev->GetFloat( 1 );
	if (ev->NumArgs() < 2) // this shouldn't happen, but there are assets that do it
		bulletspread[firemodeindex].y = bulletspread[firemodeindex].x;
	else
		bulletspread[firemodeindex].y = ev->GetFloat( 2 );
	if (ev->NumArgs() == 5) // expanding/shrinking spread
	   {
		endbulletspread[firemodeindex].x = ev->GetFloat(3);
		endbulletspread[firemodeindex].y = ev->GetFloat(4);
		endbulletspread[firemodeindex].z = ev->GetFloat(5); // time interval
	}
	else
	   {
		endbulletspread[firemodeindex].z = 0; // make sure spread is constant if it's not variable (duh)
	   }
}

//======================
//Weapon::SetHand
//======================
void Weapon::SetHand( Event *ev )
{
	str side;
	
	side = ev->GetString( 1 );
	
	if ( !stricmp( side.c_str(), "righthand" ) || !stricmp( side.c_str(), "right" ) )
		hand = WEAPON_RIGHT;
	else if ( !stricmp( side.c_str(), "lefthand" ) || !stricmp( side.c_str(), "left" ) )
		hand = WEAPON_LEFT;
	else if ( !stricmp( side.c_str(), "dualhand" ) || !stricmp( side.c_str(), "dual" ) )
		hand = WEAPON_DUAL;
	else if ( !stricmp( side.c_str(), "any" ) )
		hand = WEAPON_ANY;
	else
	{
		warning( "Weapon::SetHand", "Unknown side %s\n", side.c_str() );
		assert( 0 );
	}
}

//======================
//Weapon::ModeSet
//======================
void Weapon::ModeSet( Event *ev )
{
	int i;
	str modestr;
	
	Event *modeev = new Event( ev->GetToken( 2 ) );
	
	modestr = ev->GetToken( 1 );
	
	firemodeindex = WeaponModeNameToNum(modestr);
	
	for( i=3; i<=ev->NumArgs(); i++ )
	{
		modeev->AddToken( ev->GetToken( i ) );
	}
	
	ProcessEvent( modeev );
	firemodeindex = FIRE_MODE1;
}

//====================
//Weapon::AutoAim
//====================
void Weapon::AutoAim( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		autoAimTargetSelectionAngle = ev->GetInteger( 1 );
		autoAimLockonAngle = ev->GetInteger( 2 );
	}
	else
	{
		autoAimTargetSelectionAngle = 60;
		autoAimLockonAngle = 15;
	}
}

//====================
//Weapon::Crosshair
//====================
void Weapon::Crosshair( Event *ev )
{
	crosshair = ev->GetBoolean( 1 );
}

//====================
//Weapon::TorsoAim
//====================
void Weapon::TorsoAim( Event *ev )
{
	torsoaim = ev->GetBoolean( 1 );
}

//====================
//Weapon::LeftAttachToTag
//====================
void Weapon::LeftAttachToTag( Event *ev )
{
	left_attachToTag = ev->GetString( 1 );
}

//====================
//Weapon::RightAttachToTag
//====================
void Weapon::RightAttachToTag( Event *ev )
{
	right_attachToTag = ev->GetString( 1 );
}

//====================
//Weapon::DualAttachToTag
//====================
void Weapon::DualAttachToTag( Event *ev )
{
	dual_attachToTag = ev->GetString( 1 );
}

//====================
//Weapon::LeftHolsterAttachToTag
//====================
void Weapon::LeftHolsterAttachToTag( Event *ev )
{
	leftholster_attachToTag = ev->GetString( 1 );
}

//====================
//Weapon::RightHolsterAttachToTag
//====================
void Weapon::RightHolsterAttachToTag( Event *ev )
{
	rightholster_attachToTag = ev->GetString( 1 );
}

//====================
//Weapon::DualHolsterAttachToTag
//====================
void Weapon::DualHolsterAttachToTag( Event *ev )
{
	dualholster_attachToTag = ev->GetString( 1 );
}

//====================
//Weapon::SetLeftHolsterAngles
//====================
void Weapon::SetLeftHolsterAngles( Event *ev )
{
	leftHolsterAngles = ev->GetVector( 1 );
}

//====================
//Weapon::SetRightHolsterAngles
//====================
void Weapon::SetRightHolsterAngles( Event *ev )
{
	rightHolsterAngles = ev->GetVector( 1 );
}

//====================
//Weapon::SetDualHolsterAngles
//====================
void Weapon::SetDualHolsterAngles( Event *ev )
{
	dualHolsterAngles = ev->GetVector( 1 );
}

//====================
//Weapon::SetHolsterScale
//====================
void Weapon::SetHolsterScale( Event *ev )
{
	holsterScale = ev->GetFloat( 1 );
}

void Weapon::setWeildedScale( Event *ev )
{
	_weildedScale = ev->GetFloat( 1 );
}

//====================
//Weapon::SetQuiet
//====================
void Weapon::SetQuiet( Event * )
{
	quiet = true;
}

//====================
//Weapon::SetLoopFire
//====================
void Weapon::SetLoopFire( Event * )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	loopfire[firemodeindex] = true;
}
/*
void Weapon::SetFullAnimFire( Event *ev )
	{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
   fullanimfire[firemodeindex] = true;
	}
*/

//======================
//Weapon::SetMeansOfDeath
//======================
void Weapon::SetMeansOfDeath( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	meansofdeath[firemodeindex] = (meansOfDeath_t )MOD_NameToNum( ev->GetString( 1 ) );
}

//======================
//Weapon::GetMeansOfDeath
//======================
meansOfDeath_t Weapon::GetMeansOfDeath( firemode_t mode )
{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) )
		return meansofdeath[mode];
	else
	{
		warning( "Weapon::GetMeansOfDeath", "Invalid mode %d\n", mode );
		return MOD_NONE;
	}
}

//======================
//Weapon::SetAimTarget
//======================
void Weapon::SetAimTarget( Entity *ent )
{
	aim_target = ent;
}

//======================
//Weapon::WorldHitSpawn
//======================
void Weapon::WorldHitSpawn( firemode_t mode, const Vector &origin, const Vector &angles, float life )
{
	if ( !worldhitspawn[mode].length() )
		return;
	
	Entity::SpawnEffect(worldhitspawn[mode], origin, angles, life);
}

//======================
//Weapon::SetWorldHitSpawn
//======================
void Weapon::SetWorldHitSpawn( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	worldhitspawn[firemodeindex] = ev->GetString( 1 );
}

void Weapon::MakeNoise( Event *ev )
{
	float radius = 500.0f;
	qboolean force = false;
	
	if ( ev->NumArgs() > 0 )
		radius = ev->GetFloat( 1 );
	
	if ( ev->NumArgs() > 1 )
		force = ev->GetBoolean( 2 );
	
	if ( attached && ( next_noise_time <= level.time || force ) )
	{
		BroadcastSound( radius , SOUNDTYPE_WEAPONFIRE );
		next_noise_time = level.time + 1.0f;
	}
}

void Weapon::SetViewModel( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		viewmodel = ev->GetString( 1 );
		
		gi.setviewmodel( edict, viewmodel );
		//setViewModel( viewmodel );
	}
}

void Weapon::DonePutaway( Event * )
{
	//int i;

	// Tell the state machine we're lowering the weapon
	// so it will continue to do it's normal deactivate code
	weaponstate = WEAPON_LOWERING;

	if ( zoomed )
	{
		CancelEventsOfType( EV_Weapon_Zoom );	
		ProcessEvent( EV_Weapon_Zoom );	
	}

	if ( _controllingProjectile )
	{
		toggleProjectileControl();
	}

	// No longer targeting anyone

	//SetTargetedEntity( NULL );

	if ( targetidle )
		CancelEventsOfType( EV_Weapon_TargetIdleThink );

	// Move ammo from weapon back to owner if possible

	/* if ( owner )
	{
		for ( i = 0 ; i < MAX_FIREMODES ; i++ )
		{
			if ( ammo_in_clip[ i ] )
			{
				int maxAmount;
				int amount;

				maxAmount = owner->MaxAmmoCount( ammo_type[ i ] ) - owner->AmmoCount( ammo_type[ i ] );

				if ( maxAmount > ammo_in_clip[ i ] )
					amount = ammo_in_clip[ i ];
				else
					amount = maxAmount;

				owner->GiveAmmo( ammo_type[ i ], amount, false );
				ammo_in_clip[ i ] -= amount;
				owner->AmmoAmountInClipChanged( ammo_type[ i ], ammo_in_clip[ i ] );
			}
		}
	} */
}

void Weapon::SetRegenAmmo( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		_regenAmount[ firemodeindex ] = ev->GetInteger( 1 );	

		_regenTime[ firemodeindex ] = 1.0; // default time
	}

	if ( ev->NumArgs() > 1 )
	{
		_regenTime[ firemodeindex ] = ev->GetFloat( 2 );
	}

	turnThinkOn();
}

//----------------------------------------------------------------
// Name:			SetRegenOnlyWhenIdle
// Class:			Weapon
//
// Description:		Specifies that this weapon only regenerates ammo when it is idle (not shooting)
//
// Parameters:		Event *ev
//
// Returns:			none
//----------------------------------------------------------------

void Weapon::SetRegenOnlyWhenIdle( Event * )
{
	_regenOnlyWhenIdle[ firemodeindex ] = true;
}

void Weapon::ChangeIdle( Event * )
{
	ForceIdle();
	
	if ( !owner )
	{
		return;
	}
}

void Weapon::DrawBowStrain( Event * )
{
	if ( animate->HasAnim( "draw_strain" ) )
		animate->RandomAnimate( "draw_strain" );
}

void Weapon::AltDrawBowStrain( Event * )
{
	if ( animate->HasAnim( "alternate_draw_strain" ) )
		animate->RandomAnimate( "alternate_draw_strain" );
}

void Weapon::SetAccuracy( Event *ev )
{
	if ( ev->NumArgs() < 6 )
	{
		gi.DPrintf("SetAccuracy: Too few parameters\n");
		return;
	}
	firemode_t mode = WeaponModeNameToNum( ev->GetToken( 1 ) );
	accuracy[mode][ACCURACY_STOPPED] = ev->GetFloat( 2 );
	accuracy[mode][ACCURACY_CHANGE] = ev->GetFloat( 3 );
	accuracy[mode][ACCURACY_WALK] = ev->GetFloat( 4 );
	accuracy[mode][ACCURACY_RUN] = ev->GetFloat( 5 );
	accuracy[mode][ACCURACY_CROUCH] = ev->GetFloat( 6 );
}

void Weapon::SetZoomFOV( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		zoomfov = ev->GetFloat( 1 );
	}
}

void Weapon::SetReticuleTime( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		reticuletime = ev->GetFloat( 1 );
}

void Weapon::SetStartZoom( Event* ev )
{
	if(ev == 0)
		return;
	
	startzoom = ev->GetFloat(1);
}

void Weapon::SetEndZoom( Event* ev )
{
	if(ev == 0)
		return;
	
	endzoom = ev->GetFloat(1);
}

void Weapon::SetZoomTime( Event* ev )
{
	if(ev == 0)
		return;
	
	zoomtime = ev->GetFloat(1);
}

void Weapon::SetAimType( Event *ev )
{
	
	str aimstr;
	
	Sentient *owner;
	Player *player = NULL;
	owner = this->owner;
	assert( owner );
	if ( owner->isSubclassOf( Player ) )
		player = ( Player * )owner;
	
	if ( ev->NumArgs() > 0 )
		aimstr = ev->GetString( 1 );
	
	// Crouching is a special case... you'll always have the "crouch" accuracy
	// rating in your crouched, regardless of whether you're walking or not.
	if ( player && player->GetCrouch() || ( aimstr == "crouch" ) )
		aimtype = ACCURACY_CROUCH;
	else 
	{
		if ( aimstr == "stopped" )
			aimtype = ACCURACY_STOPPED;	
		if ( aimstr == "walk" )
			aimtype = ACCURACY_WALK;
		if ( aimstr == "run" )
			aimtype = ACCURACY_RUN;
		if ( aimstr == "change" )
			aimtype = ACCURACY_CHANGE;
	}
	
	if ( ( aimstr != "stopped" ) && ( aimstr != "walk" ) && ( aimstr != "run" ) && ( aimstr != "crouch" ) && ( aimstr != "change" ) )
	{
		warning("SetAimType: Invalid aimtype, %s, defaulting to stopped",aimstr);
		aimtype = ACCURACY_STOPPED;
	}
	
	if ( g_showaccuracymod->integer )
		gi.DPrintf("Accuracy Mod: %s = %f\n", aimstr.c_str(), accuracy[curmode][aimtype] );
}

void Weapon::SetFireTimer( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	
	fire_timer[firemodeindex] = ev->GetFloat( 1 );
}

void Weapon::UseSameClip( Event * )
{
	usesameclip = true;
}

qboolean Weapon::HasFullClip( void )
{
	firemode_t mode = FIRE_MODE1;
	firemode_t clipToUse;
	
	if ( usesameclip )
		clipToUse = FIRE_MODE1;
	else
		clipToUse = mode;
	
	if ( ammo_in_clip[clipToUse] == ammo_clip_size[clipToUse] )
		return true;
	
	return false;
}

void Weapon::SetMaxModes( Event *ev )
{
	if ( ev->NumArgs() < 1 )
	{
		maxmode = FIRE_MODE2;
		return;
	}
	
	maxmode = WeaponModeNameToNum( ev->GetToken( 1 ) );
}

void Weapon::SetSwitchMode( Event * )
{
	switchmode = true;
}

// Target Idle Weapon functions
void Weapon::TargetIdle( Event * )
{
	targetidle = true;
}

void Weapon::TargetIdleThink( Event * )
{
	CancelEventsOfType( EV_Weapon_TargetIdleThink );
	
	// HACK sortof: If the weapon is playing it's put away animation,
	// we don't do this event.  For some reason a CancelEventsOfType
	// call in the Putaway function failed to cancel this event.
	if  ( animate->GetName() == "putaway" )
		return;
	
	PostEvent( EV_Weapon_TargetIdleThink, TargetIdleTime );
	
	Sentient *owner;
	Player *player;
	owner = this->owner;
	assert( owner );
	
	if ( owner->isSubclassOf( Player ) )
	{
		player = ( Player * )owner;
		Vector pos, forward, right, up, endpoint, vorg;   
		GetMuzzlePosition( &pos, &forward, &right, &up );
		vorg = player->origin;
		vorg.z += player->viewheight;
		
		endpoint = vorg + (forward * 4000.0f);
		trace_t trace = G_Trace( pos, vec_zero, vec_zero, endpoint, player, MASK_SHOT, true, "Weapon::TargetIdleThink" );
		if ( trace.ent && ( trace.entityNum != ENTITYNUM_WORLD ) && trace.ent->entity->isSubclassOf( Sentient ))
		{
			if ( !trace.ent->entity->deadflag )
			{
				if ( !strstr(animate->GetName(), "idle_target") )
					animate->RandomAnimate( "idle_target" );
				targetidleflag = true;
			}
			return;
		}
		else 
		{
			if ( targetidleflag )
			{
				targetidleflag = false;
				ForceIdle();
			}
		}
			
	}
}
// ****************************

// Weapon Mode switching functions
void Weapon::SwitchMode( void )
{
	char anim[128];
	
	Event *ev1;
	ev1 = new Event( EV_Weapon_DoneSwitchToMiddle );
	
	strcpy(anim,"");
	sprintf(anim,"switch_mode%d_neutral",(int)curmode+1);
	
	if ( animate->HasAnim( anim ) )
	{
		weaponstate = WEAPON_SWITCHINGMODE;
		animate->RandomAnimate( anim, ev1 );
	}
	else
	{
		ProcessEvent( ev1 );
	}
}

void Weapon::DoneSwitchToMiddle( Event * )
{
	int mode;
	char anim[128];
	
	Event *ev1;
	ev1 = new Event( EV_Weapon_DoneSwitching );
	
	if ( curmode == maxmode )
		mode = FIRE_MODE1;
	else
		mode = (int)curmode + 1;
	
	strcpy(anim,"");
	sprintf(anim,"switch_mode%d",mode);
	
	if ( animate->HasAnim( anim ) )
	{
		weaponstate = WEAPON_SWITCHINGMODE;
		animate->RandomAnimate( anim, ev1 );
	}
	else
	{
		ProcessEvent( ev1 );
	}
}

void Weapon::DoneSwitching( Event * )
{
	if ( curmode == maxmode )
		curmode = FIRE_MODE1;
	else
		curmode = (firemode_t)((int)curmode + 1);
	
	weaponstate = WEAPON_READY;
	
	// Clear our fire timer
	INITIALIZE_WEAPONMODE_VAR(next_fire_time, 0.0);
	
	ForceIdle();
}
// *************

void Weapon::SetBurstMode( Event *ev )
{
	burstmode[firemodeindex] = true;
	if ( ev->NumArgs() > 0 )
		burstcountmax = ev->GetInteger( 1 );
	else
		burstcountmax = 10; // Default to 10
	
	burstcount = burstcountmax;
}

void Weapon::setBurstModeDelay( Event *ev )
{
	_burstModeDelay[ firemodeindex ] = ev->GetFloat( 1 );
}

void Weapon::SetCHOffset(int chx, int chy)
{
	this->chx = chx;
	this->chy = chy;
}



// GetTargetedEntity
//
// Returns the entity that is targeted by the weapon.
// If there is not an entity, 0 is returned.
void Weapon::CheckForTargetedEntity( void )
{

}

void Weapon::SetRealViewOrigin( const Vector &rv )
{
	realvieworg = rv;
}

void Weapon::SetThirdPerson( qboolean tp )
{
	thirdperson = tp;
}

qboolean Weapon::IsDoneFiring()
{
	return donefiring;
}

//----------------------------------------------------------------
// Name:			StartFiring
// Class:			Weapon
//
// Description:		Sets the start time used for bullet spread (for repeating weapons, this should be set in a prefire state)
//
// Parameters:		Event *ev
//
// Returns:			none
//----------------------------------------------------------------
void Weapon::StartFiring( Event * )
{
	startfiretime = level.time;
}

void Weapon::FinishedFiring( Event *ev )
{
	int oldDoneFiring;

	oldDoneFiring = donefiring;

	donefiring = ev->GetInteger( 1 );

	if ( shootingSkin )
		ChangeSkin( shootingSkin, false );

	if ( donefiring && !oldDoneFiring )
	{
		CheckReload();
	}
}

void Weapon::Zoom( Event * )
{
	Player *player = NULL;
	
	if ( owner && owner->isSubclassOf( Player ) )
		player = (Player *)(Sentient *)owner;
	
	if ( !player )
		return;
	
	if ( zoomed )
	{
		endZoom();
	}
	else
	{
		zoomfov = startzoom;
		startzoomtime = level.time;
		zoomed = true;
		
		player->SetFov( zoomfov, true );
		player->client->ps.pm_flags |= PMF_ZOOM;

	}
}

void Weapon::endZoom( Event * )
{
	if ( zoomed )
		_lastZoomFov = zoomfov;
	else
		_lastZoomFov = sv_defaultFov->value;

	endZoom();
}

void Weapon::rezoom( Event * )
{
	Player *player = NULL;
	
	if ( owner && owner->isSubclassOf( Player ) )
		player = (Player *)(Sentient *)owner;
	
	if ( !player )
		return;

	if ( _lastZoomFov < sv_defaultFov->value )
	{
		zoomed = true;
		zoomfov = _lastZoomFov;
			
		player->SetFov( zoomfov, true );
		player->client->ps.pm_flags |= PMF_ZOOM;
	}
}

void Weapon::endZoom( void )
{
	Player *player = NULL;
	float defaultFov;
	
	if ( owner && owner->isSubclassOf( Player ) )
		player = (Player *)(Sentient *)owner;
	
	if ( !player )
		return;

	defaultFov = player->getDefaultFov();

	player->SetFov( defaultFov, true );
		
	//remove the player client zoom flag
	player->client->ps.pm_flags &= ~PMF_ZOOM;

	//gi.SendServerCommand(player->entnum, "stufftext \"ui_removehud zoomhud\"\n");
	player->removeHud( "zoomhud" );

	_zoomStage = ZOOM_NORMAL_FOV;
	zoomed = false;
}

void Weapon::IncrementZoom( Event*  )
{
	
	if(!zoomed)
		return;
	
	Player *player = NULL;
	
	if ( owner && owner->isSubclassOf( Player ) )
		player = (Player *)(Sentient *)owner;
	
	float zoomPercentage = ((float)level.time - startzoomtime) / zoomtime;
	zoomfov = ((endzoom - startzoom) * zoomPercentage) + startzoom;
	if(zoomfov < endzoom)
		zoomfov = endzoom;
	
	player->SetFov( zoomfov, true );
}


//-----------------------------------------------------
//
// Name:		zoomFov
// Class:		Weapon
//
// Description:	Changes the zoom fov from Normal FOV to FOV 1 to FOV 2 then back to normal fov
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void Weapon::setZoomStage( Event* ev )
{
	changeZoomStage( ev->GetFloat( 1 ), ev->GetFloat( 2 ) );
}

void Weapon::changeZoomStage( float firstZoom, float secondZoom )
{
	Player* player = NULL;

	if ( owner && owner->isSubclassOf( Player ) )
		player = (Player *)(Sentient *)owner;

	if(player == NULL)
		return;

	// Move to next zoom stage

	switch( _zoomStage )
	{
		case ZOOM_NORMAL_FOV:
			_zoomStage = ZOOM_STAGE_1;
			player->SetFov( firstZoom, true );
			player->addHud( "zoomhud" );
			break;

		case ZOOM_STAGE_1:
			_zoomStage = ZOOM_STAGE_2;
			player->SetFov( secondZoom, true );
			player->addHud( "zoomhud" );
			break;

		case ZOOM_STAGE_2:
			_zoomStage = ZOOM_NORMAL_FOV;
			endZoom();
			return;
	}

	player->client->ps.pm_flags |= PMF_ZOOM;
}


void Weapon::SetTargetingSkin( Event *ev )
{
	targetingSkin = ev->GetInteger( 1 );
}

void Weapon::SetShootingSkin( Event *ev )
{
	shootingSkin = ev->GetInteger( 1 );
}

void Weapon::setFullAmmoSkin( Event *ev )
{
	_fullAmmoSkin = ev->GetInteger( 1 );

	_fullAmmoMode = WeaponModeNameToNum( ev->GetString( 2 ) );

	turnThinkOn();
}


void Weapon::ProcessTargetedEntity( EntityPtr entity )
{
	
	Player* player;
	assert(owner);

	if ( !owner )
		return;

	player =  (Player*) (Sentient*)owner;
	if(player == 0)
		return;
	
	if ( targetingSkin )
	{
		ChangeSkin( targetingSkin, false );
		
		if ( entity && entity->isSubclassOf( Actor ) )
		{
			
			Actor *actor = (Actor *)entity.Pointer();
			
			if ( !actor->deadflag && ( actor->actortype == IS_ENEMY ) )
				ChangeSkin( targetingSkin, true );
				
		}
	}

	if(entity != 0)
	{
		entity->edict->s.eFlags |= EF_DISPLAY_DESC1;
	}

}

//----------------------------------------------------------------
// Name:			setMoveSpeedModifier
// Class:			Weapon
//
// Description:		Sets the move speed modifier (when not firing)
//
// Parameters:		Event *ev			- float, specifies modifier
//
// Returns:			none
//----------------------------------------------------------------
void Weapon::Uninitialize(void)
{
	if( _zoomStage == ZOOM_STAGE_1 || _zoomStage == ZOOM_STAGE_2 )
	{
		endZoom();
	}
}



//----------------------------------------------------------------
// Name:			setMoveSpeedModifier
// Class:			Weapon
//
// Description:		Sets the move speed modifier (when not firing)
//
// Parameters:		Event *ev			- float, specifies modifier
//
// Returns:			none
//----------------------------------------------------------------
void Weapon::setMoveSpeedModifier( Event *ev )
{
	defaultMoveSpeedModifier = ev->GetFloat( 1 );
}

//----------------------------------------------------------------
// Name:			setShootingMoveSpeedModifier
// Class:			Weapon
//
// Description:		Sets the move speed modified for a particular mode
//
// Parameters:		Event *ev			- float, specifies modifier
//
// Returns:			none
//----------------------------------------------------------------
void Weapon::setShootingMoveSpeedModifier( Event *ev )
{
	shootingMoveSpeedModifier[ firemodeindex ] = ev->GetFloat( 1 );
}

//----------------------------------------------------------------
// Name:			getMoveSpeedModifier
// Class:			Weapon
//
// Description:		Gets the current move speed modifier
//
// Parameters:		none
//
// Returns:			float				- the move speed modifier
//----------------------------------------------------------------
float Weapon::getMoveSpeedModifier( void )
{
	if ( weaponstate == WEAPON_FIRING )
		return shootingMoveSpeedModifier[ curmode ];
	else
		return defaultMoveSpeedModifier;
}

//----------------------------------------------------------------
// Name:			UseActorAiming
// Class:			Weapon
//
// Description:		Tells the weapon to use the tag_barrel for aiming, no matter who is holding it.
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Weapon::UseActorAiming( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		useActorAiming = ev->GetBoolean( 1 );
	else
		useActorAiming = true;
}

//----------------------------------------------------------------
// Name:			updateViewShake
// Class:			Weapon
//
// Description:		Updates the view shake angles
//
// Parameters:		none
//
// Returns:			none
//----------------------------------------------------------------
void Weapon::updateViewShake( void )
{
	if ( _viewShakeMagnitude[ curmode ] > 0.0f )
	{
		// Apply simple view shake

		viewShake[ PITCH ] = G_CRandom() * _viewShakeMagnitude[ curmode ];
		viewShake[ YAW ]   = G_CRandom() * _viewShakeMagnitude[ curmode ];
		viewShake[ ROLL ]  = G_CRandom() * _viewShakeMagnitude[ curmode ] * 1.5;
	}
	else if ( ( _viewMinShake[ curmode ] != vec_zero ) || ( _viewMaxShake[ curmode ] != vec_zero ) )
	{
		Vector minShake;
		Vector maxShake;

		// Apply advanced view shake

		minShake = _viewMinShake[ curmode ];
		maxShake = _viewMaxShake[ curmode ];

		if ( _viewShakeOverride[ curmode ] )
		{
			viewShake[ PITCH ] = G_Random() * ( maxShake[ PITCH ] - minShake[ PITCH ] ) + minShake[ PITCH ];
			viewShake[ YAW   ] = G_Random() * ( maxShake[ YAW   ] - minShake[ YAW   ] ) + minShake[ YAW   ];
			viewShake[ ROLL  ] = G_Random() * ( maxShake[ ROLL  ] - minShake[ ROLL  ] ) + minShake[ ROLL  ];
		}
		else
		{
			viewShake[ PITCH ] += G_Random() * ( maxShake[ PITCH ] - minShake[ PITCH ] ) + minShake[ PITCH ];
			viewShake[ YAW   ] += G_Random() * ( maxShake[ YAW   ] - minShake[ YAW   ] ) + minShake[ YAW   ];
			viewShake[ ROLL  ] += G_Random() * ( maxShake[ ROLL  ] - minShake[ ROLL  ] ) + minShake[ ROLL  ];
		}

		for ( int i = 0 ; i < 3 ; i++ )
		{
			if ( viewShake[ i ] > _maxViewShakeChange )
				viewShake[ i ] = _maxViewShakeChange;
			else if ( viewShake[ i ] < -_maxViewShakeChange )
				viewShake[ i ] = -_maxViewShakeChange;

		}
	}
}

void Weapon::startViewShake( Event * )
{
	startViewShake();
}
void Weapon::startViewShake( void )
{
	if ( ( _viewShakeMagnitude[ curmode ] > 0.0f ) || ( _viewMinShake[ curmode ] != vec_zero ) || ( _viewMaxShake[ curmode ] != vec_zero ) )
	{
		//CancelEventsOfType( EV_Weapon_ClearViewShake );

		updateViewShake();

		//if ( _viewShakeDuration[ curmode ] > FRAMETIME )
		{
			CancelEventsOfType( EV_Weapon_ReduceViewShake );
			PostEvent( EV_Weapon_ReduceViewShake, FRAMETIME );
			//PostEvent( EV_Weapon_ReduceViewShake, 0.25 );
		}

		//PostEvent( EV_Weapon_ClearViewShake, _viewShakeDuration[ curmode ] );
	}
}

void Weapon::reduceViewShake( Event * )
{
	int i;
	bool repost;

	// Reduce the viewShake

	viewShake *= 0.75;

	// Determine if we should do reduce the viewShake next frame

	repost = false;

	for ( i = 0 ; i < 3 ; i++ )
	{
		if ( ( viewShake[ i ] > 0.1f ) || ( viewShake[ i ] < -0.1f ) )
		{
			repost = true;
		}
	}

	if ( repost )
	{
		PostEvent( EV_Weapon_ReduceViewShake, FRAMETIME );
	}
	else
	{
		viewShake = vec_zero;
	}
}

//----------------------------------------------------------------
// Name:			clearViewShake
// Class:			Weapon
//
// Description:		Clears the view shake angles
//
// Parameters:		none
//
// Returns:			none
//----------------------------------------------------------------
void Weapon::clearViewShake( Event * )
{
	viewShake = vec_zero;
	CancelEventsOfType( EV_Weapon_ReduceViewShake );
}

//----------------------------------------------------------------
// Name:			getViewShake
// Class:			Weapon
//
// Description:		Gets the angles to shake the view
//
// Parameters:		none
//
// Returns:			Vector					- angles to shake the view by
//----------------------------------------------------------------
Vector Weapon::getViewShake( void )
{
	return viewShake;
}

//----------------------------------------------------------------
// Name:			setViewShakeInfo
// Class:			Weapon
//
// Description:		Sets the magnitude and duration that the view should shake when firing the weapon
//
// Parameters:		Event *ev			- float, specifies the magnitude
//										  float, specifies the duration (optional)
//
// Returns:			none
//----------------------------------------------------------------
void Weapon::setViewShakeInfo( Event *ev )
{
	_viewShakeMagnitude[ firemodeindex ] = ev->GetFloat( 1 );

	if ( ev->NumArgs() > 1 )
	{
		_viewShakeDuration[ firemodeindex ] = ev->GetFloat( 2 );
	}
	else
	{
		_viewShakeDuration[ firemodeindex ] = FRAMETIME;
	}
}

void Weapon::setAdvancedViewShakeInfo( Event *ev )
{
	_viewMinShake[ firemodeindex ] = ev->GetVector( 1 );
	_viewMaxShake[ firemodeindex ] = ev->GetVector( 2 );

	if ( ev->NumArgs() > 2 )
	{
		_viewShakeDuration[ firemodeindex ] = ev->GetFloat( 3 );
	}
	else
	{
		_viewShakeDuration[ firemodeindex ] = FRAMETIME;
	}

	if ( ev->NumArgs() > 3 )
	{
		_viewShakeOverride[ firemodeindex ] = ev->GetBoolean( 4 );
	}
	else
	{
		_viewShakeOverride[ firemodeindex ] = false;
	}
}


//--------------------------------------------------------------
// Name:		SetPowerRating()
// Class:		Weapon
//
// Description:	Calls SetPowerRating()
//
// Parameters:	Event *ev -- Event with the rating
//
// Returns:		None
//--------------------------------------------------------------
void Weapon::SetPowerRating( Event *ev )
{
	SetPowerRating( ev->GetFloat( 1 ) );
}

//--------------------------------------------------------------
// Name:		SetProjectileDamage()
// Class:		Weapon
//
// Description:	Calls SetProjectileDamage
//
// Parameters:	Event *ev -- Event with the damage
//
// Returns:		None
//--------------------------------------------------------------
void Weapon::SetProjectileDamage( Event *ev )
{
	SetProjectileDamage(ev->GetFloat( 1 ) );
}

//--------------------------------------------------------------
// Name:		SetProjectileSpeed()
// Class:		Weapon
//
// Description:	Calls SetProjectileSpeed
//
// Parameters:	Event *ev -- Event with the speed of the projectile
//
// Returns:		None
//--------------------------------------------------------------
void Weapon::SetProjectileSpeed( Event *ev )
{
	SetProjectileSpeed(ev->GetFloat( 1 ) );
}

//--------------------------------------------------------------
// Name:		AdvancedMeleeAttack
// Class:		Weapon
//
// Description:	Does some literal melee damage based on the weapon itself.
//				Weapon angles (except Z) are taken into account, and we
//				utilize the meleestart and meleeend events to apply damage
//				properly to enemies in a single swipe.  The melee trace goes
//				between the two WEAPON tags passed in.
//
// Parameters:	const char* tag1 -- The first tag to do the melee trace from
//				const char* tag2 -- The second tag to do the melee trace from
//
// Returns:		None
//--------------------------------------------------------------
void Weapon::AdvancedMeleeAttack(const char* tag1, const char* tag2, bool criticalHit)
{
	Vector startpoint, endpoint;
	Vector tagpos, tagpos2, vect;
	float damage, knockback;
	meansOfDeath_t meansofdeath;
	
	// New Melee Combat code
	GetActorMuzzlePosition(&tagpos, NULL, NULL, NULL, tag1); // Get the position of the first tag
	GetActorMuzzlePosition(&tagpos2, NULL, NULL, NULL, tag2); // Get the position of the second tag
	startpoint = tagpos2;
	vect = tagpos - tagpos2;
	vect.z = 0;
	vect.normalize();
	endpoint = startpoint + (vect*48);
	
	// Debug line
	//G_DebugLine(startpoint, endpoint, 0.5, 1.0, 0.5, 1.0);
	
	damage = bulletdamage[FIRE_MODE1];
	knockback = bulletknockback[FIRE_MODE1];
	
	meansofdeath = GetMeansOfDeath( FIRE_MODE1 );
	
	if ( owner->isSubclassOf( Player ) )
	{
		Player *player = (Player *)(Sentient *)owner;
		
		meansofdeath = player->changetMeansOfDeath( meansofdeath );
		
		damage = player->getDamageDone( damage, meansofdeath, true );
		
		knockback = (knockback + player->GetPlayerKnockback()) * player->GetKnockbackMultiplier();
	}
	
	Sentient *owner;
	Player *player;
	owner = this->owner;
	assert( owner );
	if ( owner->isSubclassOf( Player ) )
	{
		player = ( Player * )owner;
		if ( !player->in_melee_attack )
			meleeVictims.ClearObjectList();
	}
	
	if ( !MeleeAttack( startpoint, endpoint, damage, owner, meansofdeath, 15.0f, -45.0f, 45.0f, knockback, true, &meleeVictims, this, criticalHit ) )
	{
		// Try to hit the world since we didn't do any damage to anything
		trace_t trace = G_Trace( startpoint, Vector( -8.0f, -8.0f, -8.0f ), Vector( 8.0f, 8.0f, 8.0f ), endpoint, owner, MASK_MELEE, false, "Weapon::Shoot" );
		
		Entity *victim = G_GetEntity( trace.entityNum );
		
		if ( victim && ( ( victim == world ) || ( victim->takedamage == DAMAGE_NO ) ) )
		{
			vec3_t  newangles;
			vectoangles( trace.plane.normal, newangles );
			WorldHitSpawn( FIRE_MODE1, trace.endpos, newangles, 0.1f );
			str realname = this->GetRandomAlias( "impact_world" );
			if ( realname.length() > 1 )
				this->Sound( realname, CHAN_VOICE );
		}
	}
	
	if ( !quiet )
	{
		if ( next_noise_time <= level.time )
		{
			BroadcastSound();
			next_noise_time = level.time + 1.0f;
		}
	}
}

//----------------------------------------------------------------
// Name:			applySpread
// Class:			Weapon
//
// Description:		Applies the weapons spread to the angles
//
// Parameters:		const Vector &pos				- position of the muzzle
//					Vector *forward					- forward vector of the muzzle
//					Vector *right					- right vector of the muzzle
//					Vector *up						- up vector of the muzzle
//
// Returns:			none
//----------------------------------------------------------------

void Weapon::applySpread( Vector *forward, Vector *right, Vector *up )
{
	Vector spread;


	if (( !forward ) || (!up) || (!right))
		return;

	spread = getSpread();
	
	// figure the new projected impact point based upon computed spread
	*forward = ( *forward * bulletrange[curmode]) + 
		( *right * G_CRandom( spread.x ) ) +
		( *up * G_CRandom( spread.y ) );
	
	// after figuring spread location, re-normalize vectors
	forward->normalize();
	*right = Vector::Cross(*forward,*up);
	*up = Vector::Cross(*right,*forward);	
}

Vector Weapon::getSpread( void )
{
	Vector spread;
	
	// compute the spread if it's time-variant

	if ( endbulletspread[ curmode ].z )
	{
		float timemult = (level.time - startfiretime) / endbulletspread[curmode].z;

		if (timemult > 1) // cap it so we don't get silly spread
			timemult = 1;

		spread.x = timemult * (endbulletspread[curmode].x - bulletspread[curmode].x) + bulletspread[curmode].x;
		spread.y = timemult * (endbulletspread[curmode].y - bulletspread[curmode].y) + bulletspread[curmode].y;
	}
	else
	{
		spread.x = bulletspread[curmode].x;
		spread.y = bulletspread[curmode].y;
	}

	return spread;
}


//--------------------------------------------------------------
//
// Name:			processGameplayData
// Class:			Weapon
//
// Description:		Called usually from the tiki file after all other
//					server side events are called.
//
// Parameters:		Event *ev -- not used
//
// Returns:			None
//
//--------------------------------------------------------------
void Weapon::processGameplayData( Event * )
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(getArchetype()) )
		return;

	str objname = getArchetype();
	str useanim, usetype, usethread;
	if ( gpm->hasProperty(objname, "useanim") )
		useanim = gpm->getStringValue(objname, "useanim");
	if ( gpm->hasProperty(objname, "usethread") )
		usethread = gpm->getStringValue(objname, "usethread");
	if ( gpm->hasProperty(objname + ".Pickup", "icon") )
		usetype = gpm->getStringValue(objname + ".Pickup", "icon");

	// If any of these strings were set, add to our UseData object.
	if ( useanim.length() || usethread.length() || usetype.length() )
	{
		if ( !useData )
			useData = new UseData();
		
		useData->setUseAnim(useanim);
		useData->setUseThread(usethread);
		useData->setUseType(usetype);
	}
}

float Weapon::RespawnTime( void )
{
	if ( multiplayerManager.inMultiplayer() )
		return respawntime * multiplayerManager.getWeaponRespawnMultiplayer();
	else
		return respawntime;
}

void Weapon::SetArcProjectile( Event *ev )
{
	_arcProjectile = ev->GetBoolean( 1 );
}

void Weapon::SetLowArcRange( Event *ev )
{
	_lowArcRange = ev->GetFloat( 1 );
}

void Weapon::SetPlayMissSound( Event *ev )
{
	_playMissSound = ev->GetBoolean( 1 );
}

void Weapon::noAmmoMode( Event * )
{
	_noAmmoMode[ firemodeindex ] = true;
}

bool Weapon::hasNoAmmoMode( firemode_t mode )
{
	return _noAmmoMode[ mode ];
}

void Weapon::setNoDelay( Event * )
{
	_noDelay[ firemodeindex ] = true;
}

bool Weapon::isModeNoDelay( firemode_t mode )
{
	return _noDelay[ mode ];
}

void Weapon::pauseRegen( Event * )
{
	int i;

	for( i = 0 ; i < MAX_FIREMODES ; i++ )
	{
		if ( ( _regenAmount[ i ] > 0 ) )
		{
			_nextRegenTime[ i ] = level.time + _regenTime[ i ] + 0.5f;
		}
	}
}

void Weapon::setChargedModels( Event *ev )
{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	_chargedModels[ firemodeindex ] = ev->GetInteger( 1 );
}

void Weapon::setControllingProjectile( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		_controllingProjectile = ev->GetBoolean( 1 );
	}
	else
	{
		_controllingProjectile = true;
	}
}

bool Weapon::getControllingProjectile( void )
{
	return _controllingProjectile;
}

void Weapon::setCanInterruptFiringState( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		_canInterruptFiringState = ev->GetBoolean( 1 );
	else
		_canInterruptFiringState = true;
}

void Weapon::setSpreadAnimData( Event *ev )
{
	_spreadAnims[ firemodeindex ] = ev->GetInteger( 1 );
	_spreadTime[ firemodeindex ]  = ev->GetFloat( 2 );
}

bool Weapon::canReload( void )
{
	if ( weaponstate != WEAPON_READY )
		return false;

	return true;
}

int Weapon::getAmmoInClip( firemode_t mode )
{
	firemode_t clipToUse;
	
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	
	if ( usesameclip )
		clipToUse = FIRE_MODE1;
	else
		clipToUse = mode;

	if ( ( clipToUse >= 0 ) && ( clipToUse < MAX_FIREMODES ) )
	{
		if ( ammo_clip_size[ clipToUse ] )
		{
			return ammo_in_clip[ clipToUse ];
		}
	}

	return 0;
}

void Weapon::setMaxViewShakeChange( Event *ev )
{
	_maxViewShakeChange = ev->GetFloat( 1 );
}

void Weapon::setControlParms( Event *ev )
{
	_controlEmitterName = ev->GetString( 1 );
	_controlSoundName   = ev->GetString( 2 );
}

void Weapon::toggleProjectileControl( void )
{
	if ( _controllingProjectile )
	{
		// No longer control projectile

		_controllingProjectile = false;

		// Turn off sound

		StopLoopSound();

		// Turn off emitter

		clearCustomEmitter( _controlEmitterName );
	}
	else
	{
		// Control projectile now

		_controllingProjectile = true;

		// Turn on sound

		LoopSound( _controlSoundName );

		if ( !_controllingProjectileHidden )
		{
			// Turn on emitter

			setCustomEmitter( _controlEmitterName );
		}
	}
}

void Weapon::setProjectileControlHidden( Event *ev )
{
	_controllingProjectileHidden = ev->GetBoolean( 1 );

	if ( _controllingProjectileHidden && _controllingProjectile )
	{
		// Turn off emitter

		clearCustomEmitter( _controlEmitterName );
	}
	else if ( !_controllingProjectileHidden && _controllingProjectile )
	{
		// Turn on emitter

		setCustomEmitter( _controlEmitterName );
	}
}

void Weapon::setMeleeParms( Event *ev )
{
	_meleeWidth[ firemodeindex ] = ev->GetFloat( 1 );
	_meleeHeight[ firemodeindex ] = ev->GetFloat( 2 );
	_meleeLength[ firemodeindex ] = ev->GetFloat( 3 );
}

void Weapon::setFireOffset( Event *ev )
{
	_fireOffset[ firemodeindex ] = ev->GetVector( 1 );
}

void Weapon::cacheStrings( void )
{
	G_FindConfigstringIndex( va( "$$PickedUpThe$$ $$Weapon-%s$$\n", getName().c_str() ), CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true );
}

void Weapon::setAutoReload( Event *ev )
{
	_autoReload = ev->GetBoolean( 1 );
}

void Weapon::setAllowAutoSwitch( Event *ev )
{
	_allowAutoSwitch = ev->GetBoolean( 1 );
}

void Weapon::forceReload( Event * )
{
	ForceReload();
}

void Weapon::Think( void )
{
	int i;

	for( i = 0 ; i < MAX_FIREMODES ; i++ )
	{
		if ( owner && ( _regenAmount[ i ] > 0 ) && ( _nextRegenTime[ i ] < level.time ) )
		//if ( ( attached ) && ( _regenAmount[ i ] > 0 ) && ( _nextRegenTime[ i ] < level.time ) )
		{
			owner->GiveAmmo( ammo_type[ i ], _regenAmount[ i ], false );

			_nextRegenTime[ i ] = level.time + _regenTime[ i ];
		}
	}

	if ( attached && _fullAmmoSkin )
	{
		bool full = false;
		firemode_t modeToCheck;

		if ( usesameclip )
			modeToCheck = FIRE_MODE1;
		else
			modeToCheck = _fullAmmoMode;

		if ( ammo_clip_size[ modeToCheck ] > 0 )
		{
			if ( ammo_in_clip[ modeToCheck ] == ammo_clip_size[ modeToCheck ] )
				full = true;
		}
		else
		{
			if ( ( owner ) && ( owner->AmmoCount( ammo_type[ modeToCheck ] ) == owner->MaxAmmoCount( ammo_type[ modeToCheck ] ) ) )
				full = true;
		}

		if ( full )
		{
			ChangeSkin( _fullAmmoSkin, true );
		}
		else
		{
			ChangeSkin( _fullAmmoSkin, false );
		}
	}
}

bool Weapon::shouldAutoSwitch( firemode_t mode )
{
	Player *player;
	int i;
	firemode_t modeToCheck;


	if ( !owner || !owner->isSubclassOf( Player ) )
		return false;

	player = (Player *)(Sentient *)owner;

	if ( HasAmmo( mode ) )
		return false;

	if ( !_allowAutoSwitch )
		return false;

	if ( player->edict->svflags & SVF_BOT )
		return true;

	if ( !player->getAutoSwitchWeapons() )
		return false;

	for ( i = 0 ; i < MAX_FIREMODES ; i++ )
	{
		if ( usesameclip )
			modeToCheck = FIRE_MODE1;
		else
			modeToCheck = (firemode_t)i;

		if ( HasAmmo( modeToCheck ) )
			return false;

		if ( firetype[ i ] == FT_TRIGGER_PROJECTILE )
		{
			if ( _nextSwitchTime > level.time )
			{
				return false;
			}
		}
	}

	return true;
}

int Weapon::getWeaponPriority( void )
{
	int priority;
	str currentWeaponName;

	for ( priority = 0 ; priority < 14 ; priority++ )
	{
		currentWeaponName = getWeaponByPriority( priority );

		if ( currentWeaponName == getName() )
			return priority;
	}

	return 100;
}

str Weapon::getWeaponByPriority( int priority )
{
	str weaponName;

	switch ( priority )
	{
	case 0 :
		weaponName = "PhotonBurst";
		break;
	case 1 :
		weaponName = "TetryonGatlingGun";
		break;
	case 2 :
		weaponName = "FederationSniperRifle";
		break;
	case 3 :
		weaponName = "RomulanRadGun";
		break;
	case 4 :
		weaponName = "AttrexianRifle";
		break;
	case 5 :
		weaponName = "BurstRifle";
		break;
	case 6 :
		weaponName = "GrenadeLauncher";
		break;
	case 7 :
		weaponName = "FieldAssaultRifle";
		break;
	case 8 :
		weaponName = "I-Mod";
		break;
	case 9 :
		weaponName = "CompressionRifle";
		break;
	case 10 :
		weaponName = "RomulanDisruptor";
		break;
	case 11 :
		weaponName = "DrullStaff";
		break;
	case 12 :
		weaponName = "Phaser-stx";
		break;
	case 13 :
		weaponName = "Phaser";
		break;
	}

	return weaponName;
}

void Weapon::autoSwitch()
{
	str weaponName;
	Weapon *weapon;
	Item *item;
	int i;
	Player *player;
	int ammoAvailable;

	if ( !owner || !owner->isSubclassOf( Player ) )
		return;

	player = (Player *)(Sentient *)owner;

	for ( i = 0 ; i < 14 ; i++ )
	{
		weaponName = getWeaponByPriority( i );

		if ( player->HasItem( weaponName ) )
		{
			item = player->FindItem( weaponName );

			if ( item && item->isSubclassOf( Weapon ) )
			{
				weapon = (Weapon *)item;

				ammoAvailable = weapon->getAmmoInClip( FIRE_MODE1 ) + weapon->AmmoAvailable( FIRE_MODE1 );

				if ( ammoAvailable > weapon->ammorequired[ FIRE_MODE1 ] )
				{
					Event *newEvent = new Event( EV_Player_UseItem );
					newEvent->AddString( weaponName );
					player->ProcessEvent( newEvent );

					return;
				}
			}
		}
	}
}

void Weapon::setNextSwitchTime( float time )
{
	_nextSwitchTime = level.time + time;
}

void Weapon::setNextSwitchTime( Event *ev )
{
	setNextSwitchTime( ev->GetFloat( 1 ) );
}
