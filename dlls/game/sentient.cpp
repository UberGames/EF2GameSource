//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/sentient.cpp                              $
// $Revision:: 197                                                            $
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
// Base class of entity that can carry other entities, and use weapons.
//

#include "_pch_cpp.h"
#include "entity.h"
#include "sentient.h"
#include "weapon.h"
#include "WeaponDualWield.h"
#include "weaputils.h"
#include "scriptmaster.h"
#include "ammo.h"
#include "armor.h"
#include "misc.h"
#include "inventoryitem.h"
#include "player.h"
#include "actor.h"
#include "mp_manager.hpp"
#include <qcommon/gameplaymanager.h>

#include "decals.h"

Event EV_Sentient_BeginAttack
(
	"beginattack",
	EV_DEFAULT,
	NULL,
	NULL,
	"Called before attack animation begins"
);
Event EV_Sentient_EndAttack
(
	"endattack",
	EV_DEFAULT,
	NULL,
	NULL,
	"Called after attack animation ends"
);
Event EV_Sentient_Attack
(
	"fire",
	EV_DEFAULT,
	"SS",
	"hand mode",
	"Fires the weapon in the specified hand. With the specified mode (primary, alternate)"
);
Event EV_Sentient_StopFire
(
	"stopfire",
	EV_TIKIONLY,
	"s",
	"hand",
	"Stops the firing of the weapon in the specified hand."
);
Event EV_Sentient_StartChargeFire
(
	"startcharge",
	EV_DEFAULT,
	"s",
	"firemode",
	"Draws back the bow string"
);
Event EV_Sentient_ReleaseAttack
(
	"releasefire",
	EV_TIKIONLY,
	"f",
	"fireholdtime",
	"Releases the attack in the time specified."
);
Event EV_Sentient_GiveWeapon
(
	"weapon",
	EV_DEFAULT,
	"s",
	"weapon_modelname",
	"Gives the sentient the weapon specified."
);
Event EV_Sentient_Take
(
	"take",
	EV_DEFAULT,
	"s",
	"item_name",
	"Takes away the specified item from the sentient."
);
Event EV_Sentient_GiveAmmo
(
	"ammo",
	EV_DEFAULT,
	"siI",
	"type amount max_amount",
	"Gives the sentient some ammo."
);
Event EV_Sentient_GiveAmmoOverTime
(
	"giveAmmoOverTime",
	EV_DEFAULT,
	"sif",
	"type amount time",
	"Gives the sentient some ammo over the specified period of time."
);
Event EV_Sentient_GiveArmor
(
	"armor",
	EV_DEFAULT,
	"sFB",
	"type amount pickedup",
	"Gives the sentient some armor."
);
Event EV_Sentient_GiveItem
(
	"item",
	EV_DEFAULT,
	"si",
	"type amount",
	"Gives the sentient the specified amount of the specified item."
);
Event EV_Sentient_GiveTargetname
(
	"give",
	EV_CHEAT | EV_TIKIONLY | EV_SCRIPTONLY,
	"s",
	"name",
	"Gives the sentient the targeted item."
);
Event EV_Sentient_GiveHealth
(
	"health",
	EV_CHEAT | EV_TIKIONLY,
	"f",
	"health",
	"Gives the sentient the specified amount health."
);
Event EV_Sentient_SetBloodModel
(
	"bloodmodel",
	EV_TIKIONLY,
	"s",
	"bloodModel",
	"set the model to be used when showing blood"
);
Event EV_Sentient_TurnOffShadow
(
	"noshadow",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Turns off the shadow for this sentient."
);
Event EV_Sentient_TurnOnShadow
(
	"shadow",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Turns on the shadow for this sentient."
);
Event EV_Sentient_AddImmunity
(
	"immune",
	EV_DEFAULT,
	"sSSSSS",
	"immune_string1 immune_string2 immune_string3 immune_string4 immune_string5 immune_string6",
	"Adds to the immunity list for this sentient."
);
Event EV_Sentient_AddResistance
(
	"resistance",
	EV_DEFAULT,
	"si",
	"resistance_string resistance_amount",
	"Adds to the resistance list for this sentient."
);
Event EV_Sentient_RemoveImmunity
(
	"removeimmune",
	EV_DEFAULT,
	"sSSSSS",
	"immune_string1 immune_string2 immune_string3 immune_string4 immune_string5 immune_string6",
	"Removes from the immunity list for this sentient."
);
Event EV_Sentient_RemoveResistance
(
	"removeresistance",
	EV_DEFAULT,
	"s",
	"resistance_string",
	"Removes from the resistance list for this sentient."
);
Event EV_Sentient_UpdateOffsetColor
(
	"updateoffsetcolor",
	EV_CODEONLY,
	NULL,
	NULL,
	"Updates the offset color."
);
Event EV_Sentient_JumpXY
(
	"jumpxy",
	EV_DEFAULT,
	"fff",
	"forwardmove sidemove speed",
	"Makes the sentient jump."
);
Event EV_Sentient_MeleeAttackStart
(
	"meleeattackstart",
	EV_TIKIONLY,
	"S",
	"hand",
	"Is the start of the sentient's melee attack."
);
Event EV_Sentient_MeleeAttackEnd
(
	"meleeattackend",
	EV_TIKIONLY,
	"S",
	"hand",
	"Is the end of the sentient's melee attack."
);
Event EV_Sentient_RangedAttackStart
(
	"rangedattackstart",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Is the start of the sentient's hitscan attack."
);
Event EV_Sentient_RangedAttackEnd
(
	"rangedattackend",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Is the end of the sentient's hitscan attack."
);
Event EV_Sentient_BlockStart
(
	"blockstart",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Is the start of the sentient's block."
);
Event EV_Sentient_BlockEnd
(
	"blockend",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Is the end of the sentient's block."
);
Event EV_Sentient_StunStart
(
	"stunstart",
	EV_CODEONLY,
	NULL,
	NULL,
	"Is the start of the sentient's stun."
);
Event EV_Sentient_StunEnd
(
	"stunend",
	EV_CODEONLY,
	NULL,
	NULL,
	"Is the end of the sentient's stun."
);
Event EV_Sentient_SetMouthAngle
(
	"mouthangle",
	EV_DEFAULT,
	"f",
	"mouth_angle",
	"Sets the mouth angle of the sentient."
);
Event EV_Sentient_SetMaxMouthAngle
(
	"maxmouthangle",
	EV_TIKIONLY,
	"f",
	"max_mouth_angle",
	"Sets the max mouth angle."
);
Event EV_Sentient_OnFire
(
	"onfire",
	EV_CODEONLY,
	NULL,
	NULL,
	"Called every frame when the sentient is on fire."
);
Event EV_Sentient_StopOnFire
(
	"stoponfire",
	EV_CODEONLY,
	NULL,
	NULL,
	"Stops the sentient from being on fire."
);
Event EV_Sentient_SpawnBloodyGibs
(
	"spawnbloodygibs",
	EV_DEFAULT,
	"IF",
	"number_of_gibs scale",
	"Spawns some bloody generic gibs."
);
Event EV_Sentient_SetMaxGibs
(
	"maxgibs",
	EV_TIKIONLY,
	"i",
	"max_number_of_gibs",
	"Sets the maximum amount of generic gibs this sentient will spawn when hit."
);
Event EV_Sentient_CheckAnimations
(
	"checkanims",
	EV_CONSOLE,
	NULL,
	NULL,
	"Check the animations in the .tik file versus the statefile"
);
Event EV_Sentient_SetStateFile
(
	"setstatefile",
	EV_DEFAULT,
	"s",
	"state_file",
	"Change the state file associated with this character"
);

Event EV_Sentient_AddHealth
(
	"addhealth",
	EV_SCRIPTONLY,
	"fF",
	"health_to_add maxhealth",
	"Adds health to the sentient."
);
Event EV_Sentient_SetViewMode
(
	"setviewmode",
	EV_SCRIPTONLY,
	"sB",
	"viewModeName override",
	"Puts this sentient into the specified view mode.\n"
	"Override defaults to true."
);
Event EV_Sentient_GetActiveWeaponName
(
	"getActiveWeaponName",
	EV_SCRIPTONLY,
	"@sS",
	"weaponName hand",
	"Gets the name of the weapon in the specified hand (left, right, or dual).\n"
	"If no hand is specified it will return the first it finds"
);
Event EV_Sentient_CatchOnFire
(
	"catchonfire",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Catches the actor on fire."
);
Event EV_Sentient_AddMeleeAttacker
(
	"addmeleeattacker",
	EV_CODEONLY,
	"e",
	"attack_ent",
	"Adds the entity to the melee attacker list."
);
Event EV_Sentient_SwipeOn
(
	"swipeon",
	EV_TIKIONLY,
	"s",
	"hand",
	"Turn on the sword swiping for the weapon in the specified hand"
);
Event EV_Sentient_SwipeOff
(
	"swipeoff",
	EV_TIKIONLY,
	"s",
	"hand",
	"Turn off the sword swiping for the weapon in the specified hand"
);
Event EV_Sentient_WeaponAnim
(
	"weaponanimon",
	EV_TIKIONLY,
	"ss",
	"animname hand",
	"Put the weapon in hand in the animation specified"
);
Event EV_Sentient_HealOverTime
(
	"healovertime",
	EV_DEFAULT,
	"ffff",
	"add_immediately add_at_interval interval max_percentage",
	"Will add specified amount of health immediatly, then add more at each specified time interval\n"
	"until health reaches the maxPercentage"
);
Event EV_Sentient_HealAtInterval
(
	"healatinterval",
	EV_CODEONLY,
	"fff",
	"percentageToAdd interval maxPercentage",
	"Will add the specified amount to health, then, if necessary , will generate a new event of its\n"
	"own type to continue the regen process"
);
Event EV_Sentient_GroupMemberInjured
(
	"groupmemberinjured",
	EV_DEFAULT,
	"b",
	"injured",
	"Informs us that a group member is injured or not"
);
Event EV_Sentient_SetCriticalHealthPercentage
(
	"setcriticalhealthpercentage",
	EV_DEFAULT,
	"f",
	"percentage",
	"Sets the percentage of health that qualifies as critical -- Note, values must be entered as\n"
	"floating point numbers... .10 for example is 10 percent"
);
Event EV_Sentient_HeadWatchAllowed
(
	"headwatchallowed",
	EV_DEFAULT,
	"B",
	"flag",
	"Sets whether to headwatch or not, default is true."
);
Event EV_Sentient_SetDamageThreshold
(
	"setdamagethreshold",
	EV_DEFAULT,
	"ff",
	"maxDamage duration",
	"Sets up the damage threshold"
);
Event EV_Sentient_DisplayFireEffect
(
	"displayfireeffect",
	EV_DEFAULT,
	"B",
	"flag",
	"Sets whether or not this sentient displays the fire effect when on fire."
);
Event EV_Sentient_ClearDamageThreshold
(
	"cleardamagethreshold",
	EV_DEFAULT,
	NULL,
	NULL,
	"Clears out and resets the damage threshold"
);
Event EV_Sentient_DropItem
(
	"dropitem",
	EV_DEFAULT,
	"s",
	"itemName",
	"Drops the item by the specified name."
);
Event EV_Sentient_SetArmorActiveStatus
(
	"setmyarmorstatus",
	EV_DEFAULT,
	"b",
	"flag",
	"Sets The Active Status on Armor"
);

Event EV_Sentient_SetArmorMultiplier
(
	"setmyarmormultiplier",
	EV_DEFAULT,
	"f",
	"multiplier",
	"Sets the mulitplier of the armor"
);

Event EV_Sentient_AddToMyArmor
(
	"addtomyarmor",
	EV_DEFAULT,
	"f",
	"amountToAdd",
	"Adds the amount to the current Armor"
);

Event EV_Sentient_SetMyArmorAmount
(
	"setmyarmoramount",
	EV_DEFAULT,
	"f",
	"amount",
	"Sets the amount of armor to the specifed number"
);

Event EV_Sentient_FreeInventory
(
	"freeInventory",
	EV_DEFAULT,
	NULL,
	NULL,
	"Frees the sentient's inventory"
);

Event EV_Sentient_ArmorCommand
(
	"armorcommand",
	EV_CODEONLY,
	"sSSSSSSS",
	"arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
	"Pass the args to the active armor"
);

Event EV_Sentient_SetHateModifier
(
	"sethatemodifier",
	EV_DEFAULT,
	"f",
	"hate_modifier",
	"Sets the hate modifier"
);

Event EV_Sentient_CacheStateMachineAnims
(
	"cacheStateMachineAnims",
	EV_CACHE,
	"s",
	"stateMachineName",
	"Caches all of the anims needed by the statemachine"
);


CLASS_DECLARATION( Entity, Sentient, NULL )
{
	{ &EV_Sentient_BeginAttack,					&Sentient::BeginAttack					},
	{ &EV_Sentient_EndAttack,					&Sentient::EndAttack					},
	{ &EV_Sentient_Attack,						&Sentient::FireWeapon					},
	{ &EV_Sentient_StopFire,					&Sentient::StopFireWeapon				},
	{ &EV_Sentient_StartChargeFire,				&Sentient::StartChargeFire				},
	{ &EV_Sentient_ReleaseAttack,				&Sentient::ReleaseFireWeapon			},
	{ &EV_Sentient_GiveAmmo,					&Sentient::EventGiveAmmo				},
	{ &EV_Sentient_GiveAmmoOverTime,			&Sentient::giveAmmoOverTime				},
	{ &EV_Sentient_GiveWeapon,					&Sentient::EventGiveItem				},
	{ &EV_Sentient_GiveArmor,					&Sentient::EventGiveArmor				},
	{ &EV_Sentient_GiveItem,					&Sentient::EventGiveItem				},
	{ &EV_Sentient_GiveHealth,					&Sentient::EventGiveHealth				},
	{ &EV_Sentient_Take,						&Sentient::EventTake					},
	{ &EV_Sentient_SetBloodModel,				&Sentient::SetBloodModel				},
	{ &EV_Sentient_GiveTargetname,				&Sentient::EventGiveTargetname			},
	{ &EV_Damage,								&Sentient::ArmorDamage					},
	{ &EV_Sentient_TurnOffShadow,				&Sentient::TurnOffShadow				},
	{ &EV_Sentient_TurnOnShadow,				&Sentient::TurnOnShadow					},
	{ &EV_Sentient_AddImmunity,					&Sentient::AddImmunity					},
	{ &EV_Sentient_RemoveImmunity,				&Sentient::RemoveImmunity				},
	{ &EV_Sentient_AddResistance,				&Sentient::AddResistance				},
	{ &EV_Sentient_RemoveResistance,			&Sentient::RemoveResistance				},
	{ &EV_Sentient_UpdateOffsetColor,			&Sentient::UpdateOffsetColor			},
	{ &EV_Sentient_JumpXY,						&Sentient::JumpXY						},
	{ &EV_Sentient_MeleeAttackStart,			&Sentient::MeleeAttackStart				},
	{ &EV_Sentient_MeleeAttackEnd,				&Sentient::MeleeAttackEnd				},
	{ &EV_Sentient_RangedAttackStart,			&Sentient::RangedAttackStart			},
	{ &EV_Sentient_RangedAttackEnd,				&Sentient::RangedAttackEnd				},
	{ &EV_Sentient_BlockStart,					&Sentient::BlockStart					},
	{ &EV_Sentient_BlockEnd,					&Sentient::BlockEnd						},
	{ &EV_Sentient_StunStart,					&Sentient::StunStart					},
	{ &EV_Sentient_StunEnd,						&Sentient::StunEnd						},
	{ &EV_Sentient_SetMaxMouthAngle,			&Sentient::SetMaxMouthAngle				},
	{ &EV_Sentient_OnFire,						&Sentient::OnFire						},
	{ &EV_Sentient_StopOnFire,					&Sentient::StopOnFire					},
	{ &EV_Sentient_SpawnBloodyGibs,				&Sentient::SpawnBloodyGibs				},
	{ &EV_Sentient_SetMaxGibs,					&Sentient::SetMaxGibs					},
	{ &EV_Sentient_CheckAnimations,				&Sentient::CheckAnimations				},
	{ &EV_Sentient_SetStateFile,				&Sentient::SetStateFile					},
	{ &EV_Sentient_AddHealth,					&Sentient::AddHealth					},
	{ &EV_Sentient_SetViewMode,					&Sentient::setViewMode					},
	{ &EV_Sentient_GetActiveWeaponName,			&Sentient::getActiveWeaponName			},
	{ &EV_Sentient_CatchOnFire,					&Sentient::CatchOnFire					},
	{ &EV_Sentient_SwipeOn,						&Sentient::SwipeOn						},
	{ &EV_Sentient_SwipeOff,					&Sentient::SwipeOff						},
	{ &EV_Sentient_HealOverTime,				&Sentient::AddHealthOverTime			},
	{ &EV_Sentient_HealAtInterval,				&Sentient::AddHealthAtInterval			},
	{ &EV_Sentient_SetCriticalHealthPercentage,	&Sentient::SetCriticalHealthPercentage	},
	{ &EV_Sentient_HeadWatchAllowed,			&Sentient::HeadWatchAllowed				},
	{ &EV_Sentient_WeaponAnim,					&Sentient::SetWeaponAnim				},
	{ &EV_Sentient_SetDamageThreshold,			&Sentient::SetDamageThreshold			},
	{ &EV_Sentient_DisplayFireEffect,			&Sentient::DisplayFireEffect			},
	{ &EV_Sentient_ClearDamageThreshold,		&Sentient::ClearDamageThreshold			},
	{ &EV_Sentient_DropItem,					&Sentient::DropItemEvent				},
	{ &EV_Sentient_SetArmorActiveStatus,		&Sentient::SetArmorActiveStatus			},
	{ &EV_Sentient_SetArmorMultiplier,			&Sentient::SetArmorMultiplier			},
	{ &EV_Sentient_AddToMyArmor,				&Sentient::AddToMyArmor					},
	{ &EV_Sentient_SetMyArmorAmount,			&Sentient::SetMyArmorAmount				},
	{ &EV_Sentient_ArmorCommand,				&Sentient::ArmorEvent					},
	{ &EV_Sentient_SetHateModifier,				&Sentient::SetHateModifier				},

	{ &EV_Sentient_FreeInventory,				&Sentient::FreeInventory				},
	{ &EV_Sentient_CacheStateMachineAnims,		&Sentient::cacheStateMachineAnims		},
	{ NULL, NULL }
};

Container<Sentient *> SentientList;

Sentient::Sentient()
{
	animate = new Animate( this );
	SentientList.AddObject( ( Sentient * )this );

	setContents( CONTENTS_BODY );
	inventory.ClearObjectList();
	
	newWeapon         = NULL;
	currentBaseArmor  = NULL;
	eyeposition       = Vector(0, 0, 64);
	shotsFiredThisVolley = 0;
	//firing_frame      = -1;
	//firing_anim       = -1;
	knock_start_time = 0;
	// do better lighting on all sentients
	edict->s.renderfx |= RF_EXTRALIGHT;
	edict->s.renderfx |= RF_SHADOW;
	// sentients have precise shadows
	edict->s.renderfx |= RF_SHADOW_PRECISE;

	in_melee_attack = false;
	in_ranged_attack = false;
	in_block = false;
	in_stun = false;
	attack_blocked = false;
	max_mouth_angle = 10;
	// touch triggers by default
	flags |= FL_TOUCH_TRIGGERS;
	on_fire = false;
	max_gibs = 0;
	next_bleed_time = 0;
	
	last_surface_hit = -1;
	last_bone_hit = -1;
	_canSendInjuredEvent = true;
	_headWatchAllowed = true;
	_hateModifier = 1.0;
	
	addAffectingViewModes( gi.GetViewModeClassMask( "sentient" ) );
	SetCriticalHealthPercentage( .5 );
	
	//Initial Max Value for _damageThreshold is set to -1 so that
	//we won't do a lot of processing unless we specifically set
	//our max value
	_damageThreshold.maxDamage = -1.0f;
	_displayFireEffect = true;

	on_fire_tagnums[ 0 ] = -1;
	on_fire_tagnums[ 1 ] = -1;
	on_fire_tagnums[ 2 ] = -1;
}

Sentient::~Sentient()
{
	Sentient *sentient;
	
	for ( int i = 1; i <= resistances.NumObjects(); i++ )
	{
		if (resistances.ObjectAt( i ) )
		{
			delete resistances.ObjectAt( i );
			resistances.ObjectAt( i ) = 0;
		}
	}
	
	sentient = this;
	
	SentientList.RemoveObject( sentient );
	FreeInventory();
}


// HACK HACK HACK
void Sentient::UpdateOffsetColor( Event *ev )
{
	G_SetConstantLight( &edict->s.constantLight, &offset_color[ 0 ], &offset_color[ 1 ], &offset_color[ 2 ], NULL );
	offset_color -= offset_delta;
	offset_time -= FRAMETIME;
	if ( offset_time > 0.0f )
	{
		PostEvent( EV_Sentient_UpdateOffsetColor, FRAMETIME );
	}
	else
	{
		CancelEventsOfType( EV_Sentient_UpdateOffsetColor );
		edict->s.renderfx &= ~RF_LIGHTOFFSET;
		offset_color[ 0 ] = offset_color[ 1 ] = offset_color[ 2 ] = 0;
		G_SetConstantLight( &edict->s.constantLight, &offset_color[ 0 ], &offset_color[ 1 ], &offset_color[ 2 ], NULL );
	}
}

void Sentient::SetOffsetColor( float r, float g, float b, float time )
{
	// kill all pending events
	CancelEventsOfType( EV_Sentient_UpdateOffsetColor );
	
	offset_color[ 0 ] = r;
	offset_color[ 1 ] = g;
	offset_color[ 2 ] = b;
	
	G_SetConstantLight( &edict->s.constantLight, &offset_color[ 0 ], &offset_color[ 1 ], &offset_color[ 2 ], NULL );
	
	// delta is a little less so we don't go below zero
	offset_delta = offset_color * ( FRAMETIME / ( time + ( 0.5f * FRAMETIME ) ) );
	offset_time = time;
	
	edict->s.renderfx |= RF_LIGHTOFFSET;
	
	PostEvent( EV_Sentient_UpdateOffsetColor, FRAMETIME );
}

Vector Sentient::EyePosition( void )
{
	int tagNum;

	tagNum = gi.Tag_NumForName( edict->s.modelindex, "tag_eyes" );

	if ( tagNum >= 0 )
	{
		Vector tag_pos;

		GetTag( tagNum, &tag_pos );

		return tag_pos;
	}
	else
	{
		return origin + eyeposition;
	}
}

Vector Sentient::GunPosition( void )
{
	return origin;
}

void Sentient::EventGiveHealth( Event *ev )
{
	health = ev->GetFloat( 1 );
}

void Sentient::AddHealth( Event *ev )
{
	float health_to_add;
	float maxhealth;


	health_to_add = ev->GetFloat( 1 );

	if ( ev->NumArgs() > 1 )
		maxhealth = ev->GetFloat( 2 );
	else
		maxhealth = max_health;

	AddHealth( health_to_add, maxhealth );
}

void Sentient::AddHealth( float healthToAdd, float maxHealth )
{
	float tempMaxHealth;
	float newHealth;

	if ( maxHealth )
		tempMaxHealth = maxHealth;
	else
		tempMaxHealth = max_health;

	newHealth = health + healthToAdd;

	// See if the health is already above the max (don't do anything if it is)

	if ( health >= tempMaxHealth )
		return;
	
	if ( newHealth > tempMaxHealth )
		SetHealth(tempMaxHealth);
	else
		SetHealth(newHealth);
	
}

void Sentient::SetBloodModel( Event *ev )
{
	str name;
	str cache_name;
	str models_dir = "models/";
	
	if ( ev->NumArgs() < 1 )
		return;
	
	blood_model = ev->GetString( 1 );
	cache_name = models_dir + blood_model;
	CacheResource( cache_name.c_str(), this );
	
	name = GetBloodSpurtName();
	if ( name.length() )
	{
		cache_name = models_dir + name;
		CacheResource( cache_name.c_str(), this );
	}
	
	name = GetBloodSplatName();
	if ( name.length() )
		CacheResource( name.c_str(), this );
	
	name = GetGibName();
	if ( name.length() )
	{
		cache_name = models_dir + name;
		CacheResource( cache_name.c_str(), this );
	}
}

void Sentient::StartChargeFire( Event *ev )
{
	Weapon * activeWeapon;
	firemode_t     mode=FIRE_MODE1;
	weaponhand_t   hand=WEAPON_RIGHT;
	
	
	hand = WeaponHandNameToNum( "dualhand" );
	
	if ( ev->NumArgs() > 0 )
	{
		mode = WeaponModeNameToNum( ev->GetString( 1 ) );
		
		if ( mode < 0 )
			return;
	}
	
	
	if ( hand > MAX_ACTIVE_WEAPONS )
	{
		warning( "Sentient::StartChargeFire", "Weapon hand number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", hand, MAX_ACTIVE_WEAPONS );
		return;
	}
	
	// start charging the active weapon
	activeWeapon = activeWeaponList[ (int)hand ];
	
    // Save off firing animation and frame
	/*firing_anim  = ev->GetAnimationNumber();
	firing_frame = ev->GetAnimationFrame();*/
	
	knock_start_time = level.time;

	/* if ( ( activeWeapon ) && activeWeapon->ReadyToFire( mode ) )
	{
		knock_start_time = level.time;
		
		if ( mode == FIRE_MODE1 )
			activeWeapon->SetAnim( "charge", EV_Weapon_DrawBowStrain);			
		else if ( mode == FIRE_MODE2 )
			activeWeapon->SetAnim( "alternatecharge", EV_Weapon_AltDrawBowStrain );
	} */
}


void Sentient::BeginAttack( Event *ev )
{
	if ( !inheritsFrom( "Player" ) )
	{
		return;
	}
	Weapon *weapon = GetActiveWeapon( WEAPON_RIGHT );
	if ( !weapon )
	{
		weapon = GetActiveWeapon( WEAPON_LEFT );
	}

	if ( !weapon )
	{
		return;
	}

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	
	float animationRate = 1.0f;
	if (gpm->hasProperty( weapon->getArchetype(), "animationRate" ) )
	{
		animationRate = gpm->getFloatValue( weapon->getArchetype(), "animationRate" );
	}
	
	animate->SetAnimationRate( animationRate );
}

void Sentient::EndAttack( Event *ev )
{
	animate->RestoreAnimationRate();
}

void Sentient::FireWeapon( Event *ev )
{
	Weapon      *activeWeapon;
	firemode_t  mode=FIRE_MODE1;
	int         number=0;
	str         modestring, side;
	
	if ( ev->NumArgs() > 0 )
	{
		side = ev->GetString( 1 );
		
		if ( !stricmp( side, "righthand" ) )
			number = WEAPON_RIGHT;
		else if ( !stricmp( side, "lefthand" ) )
			number = WEAPON_LEFT;
		else if ( !stricmp( side, "dualhand" ) )
		{
			number = WEAPON_DUAL;
			
			if ( ev->NumArgs() == 2 )
            {
				modestring = ev->GetString( 2 );
				mode = WeaponModeNameToNum(modestring);
				//if ( !modestring.icmp( "primary" ) )
				//   mode = FIRE_MODE1;
				//else if ( !modestring.icmp( "alternate" ) )
				//   mode = FIRE_MODE2;
				//else
				//   warning( "Sentient::FireWeapon", "Invalid fire mode %s\n", modestring );
            }
		}
		else
			number = atoi( side.c_str() );
	}
	
	if ( ( number > MAX_ACTIVE_WEAPONS ) || ( number < 0 ) )
	{
		warning( "Sentient::FireWeapon", "Weapon number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", number, MAX_ACTIVE_WEAPONS );
		return;
	}
	
	// Save off firing animation and frame
	//firing_anim  = ev->GetAnimationNumber();
	//firing_frame = ev->GetAnimationFrame();
	
	activeWeapon = activeWeaponList[ number ];
	
	if ( ( activeWeapon ) && activeWeapon->ReadyToFire( mode ) )
	{
		activeWeapon->Fire( mode );
	}
	else
	{
		if ( !activeWeapon )
			gi.WDPrintf( "No active weapon in slot #: \"%i\"\n", number );
	}
}

void Sentient::StopFireWeapon( Event *ev )
{
	Weapon      *activeWeapon;
	int         number=0;
	str         side;
	
	if ( ev->NumArgs() > 0 )
	{
		side = ev->GetString( 1 );
		
		if ( !stricmp( side, "righthand" ) )
			number = WEAPON_RIGHT;
		else if ( !stricmp( side, "lefthand" ) )
			number = WEAPON_LEFT;
		else if ( !stricmp( side, "dualhand" ) )
			number = WEAPON_DUAL;
		else
			number = atoi( side.c_str() );
	}
	
	if ( ( number > MAX_ACTIVE_WEAPONS ) || ( number < 0 ) )
	{
		warning( "Sentient::StopFireWeapon", "Weapon number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", number, MAX_ACTIVE_WEAPONS );
		return;
	}
	
	activeWeapon = activeWeaponList[ number ];
	
	if ( activeWeapon )
	{
		if ( activeWeapon->animate->HasAnim("fire_stop") )
			activeWeapon->SetAnim("fire_stop");
		else
			activeWeapon->ForceIdle();
	}
	else
	{
		gi.WDPrintf( "No active weapon in slot #: \"%i\"\n", number );
	}
}

void Sentient::ReleaseFireWeapon( Event *ev )
{
	Weapon      *activeWeapon;
	float       charge_time=0;
	firemode_t  mode=FIRE_MODE1;
	int         number=0;
	str         modestring, side;
	
	charge_time = level.time - knock_start_time;
	
	// Reset the down timer
	knock_start_time  = 0;
	
	if ( ev->NumArgs() > 0 )
	{
		side = ev->GetString( 1 );
		
		if ( !stricmp( side, "righthand" ) )
			number = WEAPON_RIGHT;
		else if ( !stricmp( side, "lefthand" ) )
			number = WEAPON_LEFT;
		else if ( !stricmp( side, "dualhand" ) )
		{
			number = WEAPON_DUAL;
			if ( ev->NumArgs() == 2 )
            {
				modestring = ev->GetString( 2 );
				mode = WeaponModeNameToNum(modestring);
				//if ( !modestring.icmp( "primary" ) )
				//   mode = FIRE_MODE1;
				//else if ( !modestring.icmp( "alternate" ) )
				//   mode = FIRE_MODE2;
				//else
				//   warning( "Sentient::FireWeapon", "Invalid fire mode %s\n", modestring );
            }
		}
		else
			number = atoi( side.c_str() );
	}
	
	if ( ( number > MAX_ACTIVE_WEAPONS ) || ( number < 0 ) )
	{
		warning( "Sentient::FireWeapon", "Weapon number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", number, MAX_ACTIVE_WEAPONS );
		return;
	}
	
	// Save off firing animation and frame
	//firing_anim  = ev->GetAnimationNumber();
	//firing_frame = ev->GetAnimationFrame();
	
	activeWeapon = activeWeaponList[ number ];
	
	if ( activeWeapon )
	{
		activeWeapon->ReleaseFire( mode, charge_time );
	}
}

void Sentient::AddItem( const Item *object )
{
	inventory.AddObject( object->entnum );
}

void Sentient::RemoveItem( Item *object )
{
	int i;

	inventory.RemoveObject( object->entnum );
	
	if ( object->isSubclassOf( Weapon ) )
		DeactivateWeapon( (Weapon *)object );
	
	for ( i=0; i<MAX_ACTIVE_WEAPONS; i++ )
	{
		Weapon *weap = activeWeaponList[i];
		
		if ( object == weap )
        {
			activeWeaponList[i] = NULL;
        }
	}

	//
	// let the sent know about it
	//
	RemovedItem( object );
}

Item *Sentient::FindBaseArmor( void )
{
	int	num;
	int	i;
	Item	*item;
	
	num = inventory.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		assert( item );
		if ( item->isSubclassOf( Armor ) )
		{
			return item;
		}
	}
	
	return NULL;
}

//--------------------------------------------------------------
//
// Name:			FindItemByExternalName
// Class:			Sentient
//
// Description:		Finds the item by its name
//
// Parameters:		const char *itemname -- Name to find
//					Item *current (default, 0) -- Item to start searching from
//
// Returns:			Item * or NULL
//
//--------------------------------------------------------------
Item *Sentient::FindItemByExternalName( const char *itemname, Item *current )
{
	int	num, i;
	Item *item;
	bool seeking;
	
	if ( current )
		seeking = true;
	else
		seeking = false;
	
	num = inventory.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		assert( item );
		if ( item == current )
		{
			seeking = false;
			continue; // Start compares with the next item
		}
		
		if ( seeking )
			continue;
		
		if ( !Q_stricmp( item->getName(), itemname ) )
			return item;
	}
	
	return NULL;
}


//--------------------------------------------------------------
//
// Name:			FindItemByModelname
// Class:			Sentient
//
// Description:		Finds the item by its name
//
// Parameters:		const char *mdl -- Model name to find
//					Item *current (default, 0) -- Item to start searching from
//
// Returns:			Item * or NULL
//
//--------------------------------------------------------------
Item *Sentient::FindItemByModelname( const char *mdl, Item *current )
{
	int i, num;
	bool seeking;
	Item *item;
	str tmpmdl;
	
	if ( current )
		seeking = true;
	else
		seeking = false;
	
	if ( strnicmp( "models/", mdl, 7 ) )
		tmpmdl = "models/";
	
	tmpmdl += mdl;
	
	num = inventory.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		assert( item );
		if ( item == current )
		{
			seeking = false;
			continue; // Start compares with the next item
		}
		
		if ( seeking )
			continue;
		
		if ( !Q_stricmp( item->model, tmpmdl ) )
			return item;
	}
	
	return NULL;
}


//--------------------------------------------------------------
//
// Name:			FindItemByClassName
// Class:			Sentient
//
// Description:		Finds the item by its name
//
// Parameters:		const char *classname -- Classname to find
//					Item *current (default, 0) -- Item to start searching from
//
// Returns:			Item * or NULL
//
//--------------------------------------------------------------
Item *Sentient::FindItemByClassName( const char *classname, Item *current )
{
	int	num, i;
	Item *item;
	bool seeking;
	
	if ( current )
		seeking = true;
	else
		seeking = false;
	
	num = inventory.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		assert( item );
		
		if ( item == current )
		{
			seeking = false;
			continue; // Start compares with the next item
		}
		
		if ( seeking )
			continue;
		
		if ( !Q_stricmp( item->edict->entname, classname ) )
			return item;
	}
	
	return NULL;
}


//--------------------------------------------------------------
//
// Name:			FindItem
// Class:			Sentient
//
// Description:		Finds the item by its name
//
// Parameters:		const char *classname -- Classname to find
//					Item *current (default, 0) -- Item to start searching from
//
// Returns:			Item * or NULL
//
//--------------------------------------------------------------
Item *Sentient::FindItem( const char *itemname, Item *current )
{
	Item *item = NULL;
	
	if ( !itemname )
		return NULL;
	
	item = FindItemByExternalName( itemname, current );
	if ( !item )
	{
		item = FindItemByModelname( itemname, current );
		if ( !item )
		{
			item = FindItemByClassName( itemname, current );
		}
	}
	
	return item;
}

void Sentient::AttachAllActiveWeapons( void )
{
	int i;
	
	for ( i=0; i<MAX_ACTIVE_WEAPONS; i++ )
	{
		Weapon *weap = activeWeaponList[i];
		
		if ( weap )
			weap->AttachToOwner( (weaponhand_t )i );
	}
}

void Sentient::DetachAllActiveWeapons( void )
{
	int i;
	
	for ( i=0; i<MAX_ACTIVE_WEAPONS; i++ )
	{
		Weapon *weap = activeWeaponList[i];
		
		if ( weap )
			weap->DetachFromOwner();
	}
}

void Sentient::FreeInventory( Event *ev )
{
	FreeInventory();
}

void Sentient::FreeInventory( void )
{
	int	num;
	int	i;
	Item	*item;
	Ammo  *ammo;
	
	// Detach all Weapons
	DetachAllActiveWeapons();
	
	// Delete all inventory items ( this includes weapons )

	num = inventory.NumObjects();
	for( i = num; i > 0; i-- )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		delete item;
	}
	inventory.ClearObjectList();
	
	// Remove all ammo
	num = ammo_inventory.NumObjects();
	for( i = num; i > 0; i-- )
	{
		ammo = ( Ammo * )ammo_inventory.ObjectAt( i );
		delete ammo;
	}

	ammo_inventory.ClearObjectList();

	for( i = 0 ; i < MAX_ACTIVE_WEAPONS ; i++ )
	{
		activeWeaponList[i] = NULL;
	}

}


qboolean Sentient::HasItem( const char *itemname )
{
	return ( FindItem( itemname ) != NULL );
}

int Sentient::NumWeapons( void )
{
	int	num;
	int	i;
	Item	*item;
	int   numweaps;
	
	numweaps = 0;
	
	num = inventory.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		if ( checkInheritance( &Weapon::ClassInfo, item->getClassname() ) )
		{
			numweaps++;
		}
	}
	
	return numweaps;
}

bool Sentient::ChangeWeapon( Weapon *weapon, weaponhand_t hand )
{
	if ( ( hand > MAX_ACTIVE_WEAPONS ) )
	{
		warning( "Sentient::ChangeWeapon", "Weapon hand number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", hand, MAX_ACTIVE_WEAPONS );
		return false;
	}

	// Check if weapon is already active in the slot
	if ( weapon == activeWeaponList[hand] )
		return false;
	
	ActivateWeapon( weapon, hand );
	return true;
}

void Sentient::DeactivateWeapon( weaponhand_t hand )
{
	int i;
	
	if ( !activeWeaponList[hand] )
	{
		warning( "Sentient::DeactivateWeapon", "Tried to deactivate a non-active weapon in hand %d\n", hand );
		return;
	}
	
	activeWeaponList[hand]->AttachToHolster( hand );
	activeWeaponList[hand]->SetPutAway( false );
	activeWeaponList[hand]->animate->RandomAnimate( "putaway" );
	
	// Check the player's inventory and detach any weapons that are already attached to that spot
	for ( i=1; i<=inventory.NumObjects(); i++ )
	{
		Item *item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		
		if ( item->isSubclassOf( Weapon ) )
		{
			Weapon *weap = ( Weapon * )item;
			
			if (
				( weap != activeWeaponList[hand] ) &&
				( !str::cmp( weap->GetCurrentAttachToTag(), activeWeaponList[hand]->GetCurrentAttachToTag() ) )
				)
            {
				weap->DetachFromOwner();
            }
		}
	}
	activeWeaponList[hand] = NULL;
}

void Sentient::DeactivateWeapon( Weapon *weapon )
{
	int i;
	
	for ( i=0; i<MAX_ACTIVE_WEAPONS; i++ )
	{
		if ( activeWeaponList[i] == weapon )
		{
			activeWeaponList[i]->DetachFromOwner();
			activeWeaponList[i]->SetPutAway( false );
			activeWeaponList[i] = NULL;
		}
	}
}

void Sentient::ActivateWeapon( Weapon *weapon, weaponhand_t hand )
{
	int i;
	
	if ( weapon->isSubclassOf(WeaponDualWield) )
	   {
		WeaponDualWield *dw = (WeaponDualWield*)weapon;
		activeWeaponList[WEAPON_LEFT] = dw->getLeftWeapon();
		activeWeaponList[WEAPON_RIGHT] = dw->getRightWeapon();
	   }
	else
 		activeWeaponList[hand] = weapon;
	
	str holsterTag;
	
	switch( hand )
	{
	case WEAPON_LEFT:
		holsterTag = weapon->GetLeftHolsterTag();
		break;
	case WEAPON_RIGHT:
		holsterTag = weapon->GetRightHolsterTag();
		break;
	case WEAPON_DUAL:
		holsterTag = weapon->GetDualHolsterTag();
		break;
	default:
		holsterTag = "";
		break;
	}
	
	// Check the player's inventory and detach any weapons that are currently attached to that tag.
	for ( i=1; i<=inventory.NumObjects(); i++ )
	{
		Item *item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		
		if ( item->isSubclassOf( Weapon ) )
		{
			Weapon *weap = ( Weapon * )item;
			
			if (
				( !str::cmp( holsterTag, weap->GetCurrentAttachToTag() ) )
				)
            {
				weap->DetachFromOwner();
            }
		}
	}
	weapon->AttachToOwner( hand );
	
	weapon->playAnim( "raise", true );
}

Weapon *Sentient::BestWeapon( Weapon *ignore )
{
	Item		*next;
	int		n;
	int		j;
	int		bestrank;
	Weapon	*bestweapon;
	
	n = inventory.NumObjects();
	
	// Search forewards until we find a weapon
	bestweapon = NULL;
	bestrank = -999999;
	for( j = 1; j <= n; j++ )
	{
		next = ( Item * )G_GetEntity( inventory.ObjectAt( j ) );
		
		assert( next );
		
		if ( ( next != ignore ) && next->isSubclassOf( Weapon ) && ( ( ( Weapon * )next )->GetRank() > bestrank ) &&
			( ( ( Weapon * )next )->HasAmmo( FIRE_MODE1 ) )	)
		{
			bestweapon = ( Weapon * )next;
			bestrank = bestweapon->GetRank();
		}
	}
	
	return bestweapon;
}

Weapon *Sentient::NextWeapon( Weapon *weapon )
{
	Item		*item;
	int		i;
	int		n;
	int		weaponorder;
	Weapon	*choice;
	int		choiceorder;
	Weapon	*bestchoice;
	int		bestorder;
	Weapon	*worstchoice;
	int		worstorder;
	
	if ( !inventory.ObjectInList( weapon->entnum ) )
	{
		error( "NextWeapon", "Weapon not in list" );
	}
	
	weaponorder = weapon->GetOrder();
	bestchoice = weapon;
	bestorder = 65535;
	worstchoice = weapon;
	worstorder = weaponorder;
	
	n = inventory.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		
		assert( item );
		
		if ( item->isSubclassOf( Weapon ) )
		{
			choice = ( Weapon * )item;
			if ( !choice->HasAmmo( FIRE_MODE1 ) || !choice->AutoChange() )
			{
				continue;
			}
			
			choiceorder = choice->GetOrder();
			if ( ( choiceorder > weaponorder ) && ( choiceorder < bestorder ) )
			{
				bestorder = choiceorder;
				bestchoice = choice;
			}
			
			if ( choiceorder < worstorder )
			{
				worstorder = choiceorder;
				worstchoice = choice;
			}
		}
	}
	
	if ( bestchoice == weapon )
	{
		return worstchoice;
	}
	
	return bestchoice;
}

Weapon *Sentient::PreviousWeapon( Weapon *weapon )
{
	Item		*item;
	int		i;
	int		n;
	int		weaponorder;
	Weapon	*choice;
	int		choiceorder;
	Weapon	*bestchoice;
	int		bestorder;
	Weapon	*worstchoice;
	int		worstorder;
	
	if ( !inventory.ObjectInList( weapon->entnum ) )
	{
		error( "PreviousWeapon", "Weapon not in list" );
	}
	
	weaponorder = weapon->GetOrder();
	bestchoice = weapon;
	bestorder = -65535;
	worstchoice = weapon;
	worstorder = weaponorder;
	
	n = inventory.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		
		assert( item );
		
		if ( item->isSubclassOf( Weapon ) )
		{
			choice = ( Weapon * )item;
			if ( !choice->HasAmmo( FIRE_MODE1 ) || !choice->AutoChange() )
			{
				continue;
			}
			
			choiceorder = choice->GetOrder();
			if ( ( choiceorder < weaponorder ) && ( choiceorder > bestorder ) )
			{
				bestorder = choiceorder;
				bestchoice = choice;
			}
			
			if ( choiceorder > worstorder )
			{
				worstorder = choiceorder;
				worstchoice = choice;
			}
		}
	}
	
	if ( bestchoice == weapon )
	{
		return worstchoice;
	}
	
	return bestchoice;
}

Weapon *Sentient::GetActiveWeapon( weaponhand_t hand )
{
	if ( ( hand > MAX_ACTIVE_WEAPONS ) || ( hand < 0 ) )
	{
		warning( "Sentient::GetActiveWeapon", "Weapon hand number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", hand, MAX_ACTIVE_WEAPONS );
		return NULL;
	}
	else if ( hand == WEAPON_ANY )
	{
		if ( activeWeaponList[ WEAPON_LEFT ] )
			return activeWeaponList[ WEAPON_LEFT ];
		else if ( activeWeaponList[ WEAPON_RIGHT ] )
			return activeWeaponList[ WEAPON_RIGHT ];
		else if ( activeWeaponList[ WEAPON_DUAL ] )
			return activeWeaponList[ WEAPON_DUAL ];
		else
			return NULL;
	}
	else
	{
		return activeWeaponList[hand];
	}
}

qboolean Sentient::IsActiveWeapon( const Weapon *weapon )
{
	int i;
	
	for( i=0; i<MAX_ACTIVE_WEAPONS; i++ )
	{
		Weapon *activeWeap = activeWeaponList[i];
		
		if ( activeWeap == weapon )
			return true;
	}
	
	return false;
}

void Sentient::EventGiveTargetname( Event *ev )
{
	int         i;
	TargetList  *tlist;
	str         name;
	const char *ptr;
	qboolean    found;
	
	name = ev->GetString( 1 );
	
	ptr = name.c_str();
	
	// skip over the $
	ptr++;
	
	found = false;
	
	tlist = world->GetTargetList( str( ptr ) );
	for ( i = 1; i <= tlist->list.NumObjects(); i++ )
	{
		Entity * ent;
		
		ent = tlist->list.ObjectAt( i );
		assert( ent );
		
		if ( ent->isSubclassOf( Item ) )
		{
			Item *item;
			
			item = ( Item * )ent;
			item->SetOwner( this );
			item->ProcessPendingEvents();
			AddItem( item );
			found = true;
		}
	}
	
	if ( !found )
	{
		ev->Error( "Could not give item with targetname %s to this sentient.\n", name.c_str() );
	}
}

Item *Sentient::giveItem( const str &itemname, int amount, bool pickedUp, float skillLevel )
{
	ClassDef	*cls;
	Item     *item;
	
	item = FindItem( itemname );
	if ( item )
	{
		item->Add( amount );
		item->SetSkillLevel( skillLevel );
		return item;
	}
	else
	{
		qboolean set_the_model = false;
		
		// we don't have it, so lets try to resolve the item name
		// first lets see if it is a registered class name
		cls = getClass( itemname );
		if ( !cls )
		{
			SpawnArgs args;
			
			// if that didn't work lets try to resolve it as a model
			args.setArg( "model", itemname );
			
			cls = args.getClassDef();
			if ( !cls )
            {
				gi.WDPrintf( "No item called '%s'\n", itemname.c_str() );
				return NULL;
			}
			set_the_model = true;
		}
		assert( cls );
		item = ( Item * )cls->newInstance();
		
		if ( !item )
		{
			gi.WDPrintf( "Could not spawn an item called '%s'\n", itemname.c_str() );
			return NULL;
		}
		
		if ( !item->isSubclassOf( Item ) )
		{
			gi.WDPrintf( "Could not spawn an item called '%s'\n", itemname.c_str() );
			delete item;
			return NULL;
		}
		
		if ( set_the_model )
		{
			// Set the model
			item->setModel( itemname );
		}
		
		item->SetOwner( this );
		item->ProcessPendingEvents();
		if ( amount )
			item->setAmount( amount );
		item->hideModel();
		
		item->SetSkillLevel( skillLevel );
		AddItem( item );
		
		if ( item->isSubclassOf( Weapon ) )
		{
			// Post an event to give the ammo to the sentient
			Event *ev1;
			
			// Remove the fullbright from the weapon
			
			item->edict->s.renderfx &= ~RF_FULLBRIGHT;
			
			ev1 = new Event( EV_Weapon_GiveStartingAmmo );
			ev1->AddEntity( this );
			item->ProcessEvent( ev1 );
			
			if ( this->isSubclassOf( Player ) )
			{
				Player *player = (Player *)this;
				Weapon *weapon = (Weapon *)item;
				Weapon *currentWeapon;
				
				if ( pickedUp && player->getAutoSwitchWeapons() )
				{
					currentWeapon = GetActiveWeapon( WEAPON_DUAL );

					if ( ( !currentWeapon ) || ( weapon->getWeaponPriority() < currentWeapon->getWeaponPriority() ) )
					{
						ev1 = new Event( EV_Player_UseItem );
						ev1->AddString( item->getName() );
						PostEvent( ev1, FRAMETIME );
					}
				}
			}
		}
		
		return item;
	}
}

void Sentient::takeItem( const char *name )
{
	Item * item;
	
	item = FindItem( name );
	if ( item )
	{
		gi.DPrintf( "Taking item %s away from player\n", item->getName().c_str() );
		
		item->PostEvent( EV_Remove, 0.0f );
		return;
	}
	
	Ammo *ammo;
	ammo = FindAmmoByName( name );
	if ( ammo )
	{
		gi.DPrintf( "Taking ammo %s away from player\n", name );
		
		ammo->setAmount( 0 );
	}
}

bool Sentient::useWeapon( const char *weaponname, weaponhand_t hand )
{
	Weapon *weapon;
	
	assert( weaponname );
	
	if ( !weaponname )
	{
		warning( "Sentient::useWeapon", "weaponname is NULL\n" );
		return false;
	}

	if ( deadflag || ( multiplayerManager.inMultiplayer() && this->isSubclassOf( Player ) && multiplayerManager.isPlayerSpectator( (Player *)this ) ) )
		return false;
	
	// Find the item in the sentient's inventory
	weapon = ( Weapon * )FindItem( weaponname );
	
	// If it exists, then make the change to the slot number specified
	if ( weapon )
	{
		return ChangeWeapon( weapon, hand );
	}
	return false;
}

void Sentient::EventTake( Event *ev )
{
	takeItem( ev->GetString( 1 ) );
}

void Sentient::EventGiveAmmo( Event *ev )
{
	int			amount,maxamount=-1;
	const char	*type;
	
	type        = ev->GetString( 1 );
	amount      = ev->GetInteger( 2 );
	
	if ( ev->NumArgs() == 3 )
		maxamount   = ev->GetInteger( 3 );
	
	GiveAmmo( type, amount, false, maxamount );
}

void Sentient::giveAmmoOverTime( Event *ev )
{
	Event *event;
	float ammoToAdd;
	float ammoToAddThisFrame;
	float timeLeft;
	int numFrames;
	const char	*ammoType;

	ammoType  = ev->GetString( 1 );
	ammoToAdd = ev->GetInteger( 2 );
	timeLeft  = ev->GetFloat( 3 );

	// Figure out how much ammo to add this frame

	if ( timeLeft < level.frametime )
		numFrames = 1;
	else
		numFrames = timeLeft / level.frametime;

	ammoToAddThisFrame = ammoToAdd / numFrames;

	// Actually add the ammo to the entity

	GiveAmmo( ammoType, ammoToAddThisFrame, false, -1 );

	// Post the event for the next frame

	ammoToAdd -= ammoToAddThisFrame;
	timeLeft -= level.frametime;

	//CancelEventsOfType( EV_Sentient_GiveAmmoOverTime );

	if ( timeLeft > 0.0f )
	{
		event = new Event( EV_Sentient_GiveAmmoOverTime );
		event->AddString( ammoType );
		event->AddInteger( ammoToAdd );
		event->AddFloat( timeLeft );
		PostEvent( event, level.frametime );
	}
}

void Sentient::EventGiveItem( Event *ev )
{
	const char	*type;
	float       amount;
	
	type     = ev->GetString( 1 );
	if ( ev->NumArgs() > 1 )
		amount   = ev->GetInteger( 2 );
	else
		amount = 1;
	
	giveItem( type, amount );
}

void Sentient::EventGiveArmor ( Event *ev )
{
	const char *type;
	Item* currentArmor;
	float amount;
	bool pickedup;
	
	currentArmor = NULL;
	currentArmor = FindBaseArmor();
	
	//We can only have 1 base armor
	//if ( currentArmor )
	//	RemoveItem( currentArmor );
	
	// Do the giveItem to put the armor in inventory
	// and to get it instantiated
	type = ev->GetString( 1 );
	
	if ( stricmp(type , "none" ) == 0 )
	{
		currentArmor = NULL;
		currentBaseArmor = NULL;
		return;
	}
	
	if ( ev->NumArgs() > 1 )
		amount = ev->GetFloat( 2 );
	else
		amount = 0;
	
	if ( ev->NumArgs() > 2 )
		pickedup = ev->GetBoolean( 3 );
	else
		pickedup = false;
	
	giveItem( type , amount );
	
	currentArmor = NULL;
	currentArmor = FindBaseArmor();

	if ( pickedup && this->isSubclassOf( Player ) && currentArmor )
	{
		int iconIndex;

		iconIndex = gi.imageindex( "sysimg/icons/items/armor" );
		
		((Player *)this)->setItemText( iconIndex, va( "$$PickedUp$$ %d $$Item-Armor$$", (int)amount ) );
		//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$PickedUp$$ %d armor", (int)amount );
	}
	
	// Cast is safe because FindBaseArmor will only return
	// items of type armor
	if ( currentArmor )
		currentBaseArmor = (Armor*)currentArmor;
}

qboolean Sentient::DoGib( int meansofdeath )
{
	if ( !com_blood->integer )
	{
		return false;
	}
	
	if (
		( meansofdeath == MOD_TELEFRAG ) ||
		( meansofdeath == MOD_LAVA )
		)
	{
		return true;
	}
	
	if ( health > -75.0f )
	{
		return false;
	}
	
	// Impact and Crush < -75 health
	if ( ( meansofdeath == MOD_IMPACT ) || ( meansofdeath == MOD_CRUSH ) )
	{
		return true;
	}
	
	return false;
}

//#define WATER_CONVERSION_FACTOR 1.0f

void Sentient::ArmorDamage( ::Damage &damage )
{
	CheckDamageThreshold( damage.damage );

	if ( damageModSystem )
		damageModSystem->resolveDamage(damage);

	ArmorDamage( 
		damage.damage, 
		damage.inflictor, 
		damage.attacker,
		damage.position, 
		damage.direction, 
		damage.normal, 
		damage.knockback, 
		damage.dflags, 
		damage.meansofdeath, 
		damage.surfaceNumber,
		damage.boneNumber,
		damage.weapon,
		damage.showPain
		);
}

void Sentient::ArmorDamage( Event *ev )
{
	float		damage;
	Entity		*inflictor;
	Entity		*attacker;
	Entity		*weapon = 0;
	Vector		position;
	Vector		direction;
	Vector		normal;
	int			knockback;
	int			dflags;
	int			meansofdeath;
	int			surfaceNumber = -1;
	int			boneNumber = -1;

	damage		   = ev->GetFloat  ( 1 );
	inflictor	   = ev->GetEntity ( 2 );
	attacker	   = ev->GetEntity ( 3 );
	position       = ev->GetVector ( 4 );
	direction      = ev->GetVector ( 5 );
	normal         = ev->GetVector ( 6 );
	knockback      = ev->GetInteger( 7 );
	dflags         = ev->GetInteger( 8 );
	meansofdeath   = ev->GetInteger( 9 );

	if ( ev->NumArgs() > 9 )
		surfaceNumber = ev->GetInteger( 10 );

	if ( ev->NumArgs() > 10 )
		boneNumber = ev->GetInteger( 11 );

	if ( ev->NumArgs() > 11 )
		weapon = ev->GetEntity( 12 );

	ArmorDamage( damage, inflictor, attacker, position, direction, normal, knockback, dflags, meansofdeath, surfaceNumber,
			boneNumber, weapon , false );
}

void Sentient::ArmorDamage( float damage, Entity *inflictor, Entity *attacker, const Vector &position, const Vector &direction, 
		const Vector &normal, int knockback, int dflags, int meansofdeath, int surfaceNumber, int boneNumber, Entity *weapon , bool showPain )
{
	Vector			momentum;
	Event			*event;
	Event			*injuredEvent;
	float			damage_red;
	float			damage_green;
	float			damage_time;
	float			resistance_modifier;
	qboolean		blocked = false;
	qboolean		set_means_of_death;
	Vector			normalizedDirection;
	

	if ( ( takedamage == DAMAGE_NO ) || ( movetype == MOVETYPE_NOCLIP ) )
	   {
		return;
		}
	
	// See if we should set means of death
	
	set_means_of_death = true;
	
	if ( this->isSubclassOf( Actor ) )
	{
		Actor *act = (Actor *)this;
		
		if ( ( act->state_flags & STATE_FLAG_SMALL_PAIN ) && ( meansofdeath == MOD_ON_FIRE || Immune( meansofdeath ) ) )
			set_means_of_death = false;
	}
	
	// See if sentient is immune to this type of damage
	
	if ( Immune ( meansofdeath ) )
	{		
		if ( set_means_of_death )
			means_of_death = meansofdeath;
		
		// Send pain event
		event = new Event( EV_Pain );
		event->AddFloat( 0.0f );
		event->AddEntity( attacker );
		event->AddInteger( meansofdeath );
		event->AddVector( position );
		event->AddVector( direction );
		ProcessEvent( event );
		return;
	}
	
	// Apply any resistances if any
	resistance_modifier = GetResistanceModifier( meansofdeath );
	
	if( resistance_modifier )
	{
		resistance_modifier /= 100.0f;
		float damagemod = resistance_modifier * damage;
		damage -= damagemod;	
	}
	
	// See if the damage is melee and high enough on actor
	
	if ( this->isSubclassOf( Actor ) )
	{
		Actor *act = ( Actor * )this;
		
		// Check the attack if it is melee
		
		if ( ( meansofdeath == MOD_SWORD ) || ( meansofdeath == MOD_CHAINSWORD ) || ( meansofdeath == MOD_AXE ) || 
			( meansofdeath == MOD_FIRESWORD ) || ( meansofdeath == MOD_ELECTRICSWORD ) || ( meansofdeath == MOD_LIGHTSWORD ) )
		{
			// Make sure attack is high enough
			
			if ( position.z - origin.z < act->minimum_melee_height )
			{
				Entity::SpawnEffect("fx/fx-sparkblock.tik", position, vec_zero, 2.0f);
				return;
			}
			
			// Make sure attack is within the damage angles
			
			if ( act->damage_angles )
			{
				Vector attack_angle;
				float yaw_diff;
				
				attack_angle = direction.toAngles();
				
				yaw_diff = angles[YAW] - attack_angle[YAW] + 180.0f;
				
				yaw_diff = AngleNormalize180( yaw_diff );
				
				if ( ( yaw_diff < -act->damage_angles ) || ( yaw_diff > act->damage_angles ) )
				{
					Entity::SpawnEffect("fx_sparkblock.tik", position, vec_zero, 2.0f);
					return;
				}
			}
		}
	}
	
	if ( deadflag )
	{
		// Spawn a blood spurt if this model has one
		if ( ShouldBleed( meansofdeath, true ) )
		{
			AddBloodSpurt( position, direction, boneNumber );
			
			if ( ShouldGib( meansofdeath, damage ) )
				ProcessEvent( EV_Sentient_SpawnBloodyGibs );
		}
		
		if ( set_means_of_death )
			means_of_death = meansofdeath;
		
		if ( ( meansofdeath == MOD_FIRE ) || ( meansofdeath == MOD_FIRESWORD ) || ( meansofdeath == MOD_FIRE_BLOCKABLE ) )
		{
			if ( GetResistanceModifier(meansofdeath) < 100 )
				TryLightOnFire( meansofdeath, attacker );
		}
		
		
		if ( damage > 0.0f )
		{
			// Send pain event
			event = new Event( EV_Pain );
			event->AddFloat( damage );
			event->AddEntity( attacker );
			event->AddInteger( meansofdeath );
			event->AddVector( position );
			event->AddVector( direction );
			event->AddInteger( showPain );
			ProcessEvent( event );
		}
		
		return;
	}
	
	if ( flags & FL_GODMODE )
	{
		return;
	}
	
	if ( currentBaseArmor )
	{
		if ( this->isSubclassOf(Actor) )
		{
			Actor *act;
			qboolean resolveDamage;
			
			act = (Actor*)this;
			
			if (  act->GetActorFlag( ACTOR_FLAG_DISABLED ) || act->GetActorFlag( ACTOR_FLAG_CRIPPLED ) )
				resolveDamage = false;
			else
				resolveDamage = true;
			
			if ( resolveDamage )
				damage = currentBaseArmor->ResolveDamage( damage , meansofdeath , direction , position, attacker );	
		}
		else
			damage = currentBaseArmor->ResolveDamage( damage , meansofdeath , direction , position , attacker );	
	}
	
	if ( this->isSubclassOf( Player ) )
	{
		Vector attack_angle;
		Vector player_angle;
		float yaw_diff;
		Player *player;		
		
		player = ( Player * )this;
		
		
		player_angle = player->GetTorsoAngles();
		attack_angle = direction.toAngles();
		yaw_diff     = player_angle[YAW] - attack_angle[YAW] + 180.0f;
		yaw_diff     = AngleNormalize180( yaw_diff );
		
		// Get real knockback value
		knockback = player->GetKnockback( knockback, blocked );

		//If we're not in multiplayer AND we don't have a ground entity
		//cut our knockback in half
		if ( !multiplayerManager.inMultiplayer() && !player->groundentity)
			knockback*=0.5;
		
	}
	
	// Do the kick
	if (!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if ((knockback) &&
			(movetype != MOVETYPE_NONE) &&
			(movetype != MOVETYPE_STATIONARY) &&
			(movetype != MOVETYPE_BOUNCE) &&
			(movetype != MOVETYPE_PUSH) &&
			(movetype != MOVETYPE_STOP))
		{
			float	m;
			Event *immunity_event;
			Event *resistance_event;
			
			if (mass < 50)
				m = 50;
			else
				m = mass;
			
			normalizedDirection = direction;
			normalizedDirection.normalize();
			if ( isClient() && ( attacker == this ) && multiplayerManager.inMultiplayer() )
				momentum = normalizedDirection * ( 1000.0f * ( float )knockback / m ); // the rocket jump hack...
			else
				momentum = normalizedDirection * ( 500.0f * ( float )knockback / m );
			
			if ( dflags & DAMAGE_BULLET )
            {
				// Clip the z velocity for bullet weapons
				if ( momentum.z > 75.0f )
					momentum.z = 75.0f;
            }
			velocity += momentum;
			
			// Make this sentient vulnerable to falling damage now
			
			if ( Immune( MOD_FALLING ) )
			{
				immunity_event = new Event( EV_Sentient_RemoveImmunity );
				immunity_event->AddString( "falling" );
				ProcessEvent( immunity_event );
			}
			
			if ( Resistant( MOD_FALLING ) )
			{
				resistance_event = new Event( EV_Sentient_RemoveResistance );
				resistance_event->AddString( "falling" );
				ProcessEvent( resistance_event );
			}
			
			
		}
	}
	
	Vector attack_angle;
	float yaw_diff;
	attack_angle = attacker->angles;
	yaw_diff = angles[YAW] - attack_angle[YAW] + 180.0f;
	yaw_diff = AngleNormalize180( yaw_diff );
	
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	float clashchance = 0.0f;
	if ( gpm->isDefined("clash_chance") )
		clashchance = atof(gpm->getDefine("clash_chance"));
	
	// Consider attacker == inflictor, so we don't clash vs. projectiles.
	if ( in_melee_attack && G_Random() < clashchance && attacker == inflictor)
	{
		if ( ( yaw_diff > -45.0f ) && ( yaw_diff < 45.0f ) )
		{
			if ( attacker->isSubclassOf( Sentient ) )
			{
				Sentient *sent = ( Sentient * )attacker;
				sent->SetAttackBlocked( true );
			}
			
			SetAttackBlocked( true );
			WeaponEffectsAndSound( weapon, "Clash", position );
			means_of_death = meansofdeath;
			return;
		}
	}
	
	// Blocking of the attack by an enemy.
	if ( in_block && ( meansofdeath != MOD_ON_FIRE ) && ( meansofdeath != MOD_EXPLOSION ) )
	{
		if ( ( yaw_diff > -45.0f ) && ( yaw_diff < 45.0f ) )
		{
			if ( ( meansofdeath != MOD_FIRE ) && ( meansofdeath != MOD_LIFEDRAIN ) )
			{
				if ( attacker->isSubclassOf( Sentient ) )
				{
					Sentient *sent = ( Sentient * )attacker;
					sent->SetAttackBlocked( true );
				}
				if ( this->isSubclassOf( Actor ) )
				{
					Actor *act = ( Actor * )this;
					act->AddStateFlag( STATE_FLAG_BLOCKED_HIT );
				}
				
				WeaponEffectsAndSound( weapon, "Blocked", position );
			}
			
			means_of_death = meansofdeath;
			return;
		}
		
		in_block = false;
	}
	
	if ( g_debugdamage->integer )
		G_DebugDamage( damage, this, attacker, inflictor );
	
	// Handle statis damage
	
	if ( meansofdeath == MOD_STASIS )
	{

		if ( damage > 0.0f )
		{
			//Stupid last minute hack to make sure we can't stasis people who will break
			if ( this->isSubclassOf(Actor) )
				{
				Actor *me;
				me = (Actor*)this;
				if ( me->GetActorFlag(ACTOR_FLAG_CANNOT_FREEZE ) )
					{
					return;
					}
				}

			startStasis();
			PostEvent( EV_StopStasis, damage );
		}
		
		return;
	}
	
	if ( meansofdeath != MOD_LIFEDRAIN )
	{
		if ( damage < 0 )
		{
			//Because a resistance of over 100 will Add to health, need to make
			//sure that the health doesn't go over the max
			if( health < max_health ) 
			{
				health -= damage;
				
				if ( health > max_health ) 
					health = max_health;
			}
		}
		else
		{
			health -= damage;
		}
	}
	
	if ( ( damage > 0.0f ) && ( surfaceNumber != -1 ) )
	{
		if ( this->isSubclassOf( Actor ) )
		{
			Actor* theActor = (Actor*)this;
			if( theActor->GetActorFlag(ACTOR_FLAG_USE_DAMAGESKINS) )
			{
				if ( !edict->s.surfaces[ surfaceNumber ] )
					edict->s.surfaces[ surfaceNumber ]++;
				
			}
		}
	}
	
	// Do all of the regional damage stuff
	
	SetRegionalDamage( surfaceNumber, boneNumber, direction );
	
	if ( ( ( meansofdeath == MOD_FIRE ) || ( meansofdeath == MOD_FIRESWORD ) || ( meansofdeath == MOD_FIRE_BLOCKABLE ) ) && !blocked )
	{
		if ( GetResistanceModifier(meansofdeath) < 100 )
			TryLightOnFire( meansofdeath, attacker );
	}
	
	
	// Set means of death
	
	if ( set_means_of_death )
		means_of_death = meansofdeath;
	
	// Spawn a blood spurt if this model has one
	if ( ( damage > 0.0f ) && ShouldBleed( meansofdeath, false ) && !blocked )
	{
		AddBloodSpurt( position, direction, boneNumber );
		
		if ( ( this->isSubclassOf( Actor ) || ( damage > 10.0f ) ) && ShouldGib( meansofdeath, damage ) )
			ProcessEvent( EV_Sentient_SpawnBloodyGibs );
	}
	
	if ( health < 0.1f )
	{
		// See if we can kill this actor or not
		
		if ( this->isSubclassOf( Actor ) )
		{
			Actor *act = ( Actor * )this;
			
			if ( act->IsImmortal() )
				health = 1;
		}
	}
	
	
	if ( health < 0.1f )
	{
		// Recalculate damage so that it is only damage needed to kill
		
		damage += health;
		
		// Make sure health is now 0
		
		health = 0;
		
		if ( ( meansofdeath == MOD_VAPORIZE ) || ( meansofdeath == MOD_VAPORIZE_COMP ) || 
			 ( meansofdeath == MOD_VAPORIZE_DISRUPTOR ) || ( meansofdeath == MOD_VAPORIZE_PHOTON ) || 
			 ( meansofdeath == MOD_SNIPER ) )
		{
			bool canDisintegrate = true;

			//YAY!!! ANOTHER SPECIAL CASE!! WOOPEE!! YEE-HAW!!! MY EXCITEMENT KNOWS NO BOUNDS!!!
			Actor *stupidSpecialCaseActor;
			if ( isSubclassOf( Actor ) )
				{
				//YAY!!!! I get to cast myself!!! YAY!!!!
				stupidSpecialCaseActor = ( Actor* )this;
				if(stupidSpecialCaseActor->GetActorFlag(ACTOR_FLAG_CANNOT_DISINTEGRATE) )
					canDisintegrate = false;
				}

			if ( canDisintegrate )
				{
				event = new Event( EV_DisplayEffect );
				
				event->AddString( "FadeOut" );
				
				if (  ( meansofdeath == MOD_VAPORIZE_DISRUPTOR ) )
					event->AddString( "Disruptor" );
				else if (  ( meansofdeath == MOD_VAPORIZE_PHOTON ) )
					event->AddString( "Photon" );
				else if (  ( meansofdeath == MOD_SNIPER ) )
					event->AddString( "Sniper" );
				else
					event->AddString( "Phaser" );
				
				ProcessEvent( event );

				edict->s.renderfx |= RF_CHILDREN_DONT_INHERIT_ALPHA;
							
				if ( !isSubclassOf( Player ) )
					PostEvent( EV_Remove, 5.0f );
				}
		}
		
		event = new Event( EV_Killed );
		event->AddEntity( attacker );
		event->AddFloat( damage );
		event->AddEntity( inflictor );
		event->AddInteger( meansofdeath );
		event->AddInteger( false );
		event->AddEntity( weapon );
		event->AddVector( direction );
		ProcessEvent( event );
	}
	
	if (
		( meansofdeath == MOD_GAS ) ||
		( meansofdeath == MOD_GAS_BLOCKABLE ) ||
		( meansofdeath == MOD_SLIME ) ||
		( meansofdeath == MOD_POISON )
		)
	{
		damage_green = damage / 50.0f;
		if ( damage_green > 1.0f )
			damage_green = 1.0f;
		if ( ( damage_green < 0.2f ) && ( damage_green > 0.0f ) )
			damage_green = 0.2f;
		damage_red = 0.0f;
	}
	else
	{
		damage_red = damage / 50.0f;
		if ( damage_red > 0.5f )
			damage_red = 0.5f;
		if ( ( damage_red < 0.2f ) && ( damage_red > 0.0f ) )
			damage_red = 0.2f;
		damage_green = 0.0f;
	}
	
	damage_time = damage / 50.0f;
	
	if ( damage_time > 0.5f )
		damage_time = 0.5f;
	
	if ( sv_showdamagecolors->integer && !this->isSubclassOf( Player ) )
	{
		SetOffsetColor( damage_red, damage_green, 0.0f, damage_time );
	}
	
	if ( health > 0.0f && damage > 0.0f )
	{
		// Send pain event
		event = new Event( EV_Pain );
		event->AddFloat( damage );
		event->AddEntity( attacker );
		event->AddInteger( meansofdeath );
		event->AddVector( position );
		event->AddVector( direction );
		event->AddInteger( showPain );
		ProcessEvent( event );
	}
	
	// Send the injured event to our group if we are at or below our critical health and
	// we can send the event;
	if ( _canSendInjuredEvent && ( health / max_health ) < GetCriticalHealthPercentage() )
	{
		injuredEvent = new Event (EV_Sentient_GroupMemberInjured);
		injuredEvent->AddInteger( 1 );
		groupcoordinator->SendEventToGroup(injuredEvent , GetGroupID());
		_canSendInjuredEvent = false;
	}
	
	
	return;
}

void Sentient::SetRegionalDamage( int surface_number, int bone_number, Vector direction )
{
	Vector attack_angle;
	float yaw_diff;
	
	// Save the last surface that was hit
	
	last_surface_hit = surface_number;
	
	// Save the last bone that was hit
	
	last_bone_hit = bone_number;
	
	// Save the last general region that was hit
	
	last_region_hit = 0;
	
	attack_angle = direction.toAngles();
	
	yaw_diff = angles[YAW] - attack_angle[YAW] + 180.0f;
	
	yaw_diff = AngleNormalize180( yaw_diff );
	
	if ( ( yaw_diff < -90.0f ) || ( yaw_diff > 90.0f ) )
		last_region_hit |= REGIONAL_DAMAGE_BACK;
	else
		last_region_hit |= REGIONAL_DAMAGE_FRONT;
}

qboolean Sentient::CanBlock( int meansofdeath, qboolean full_block )
{
	
	// Check to see what a full block can't even block
	
	switch ( meansofdeath )
	{
	case MOD_DROWN :
	case MOD_TELEFRAG :
	case MOD_SLIME :
	case MOD_LAVA :
	case MOD_FALLING :
	case MOD_RADIATION :
	case MOD_IMPALE :
	case MOD_ON_FIRE :
	case MOD_ELECTRICWATER :
	case MOD_EAT :
		return false;
	}
	
	// Full blocks block everything else
	
	if ( full_block )
		return true;
	
	// Check to see what a small block can't block
	
	switch ( meansofdeath )
	{
	case MOD_FIRE :
	case MOD_GAS :
	case MOD_CRUSH_EVERY_FRAME :
		return false;
	}
	
	// Everything else is blocked
	
	return true;
}

void Sentient::AddBloodSpurt( const Vector &position, const Vector &direction, int bone_number )
{
	Entity *blood;
	Vector dir;
	Event *event;
	str blood_splat_name;
	float blood_splat_size;
	float length;
	trace_t trace;
	float scale;
	
	
	if ( !com_blood->integer )
		return;
	
	next_bleed_time = level.time + .5f;
	
	// Calculate a good scale for the blood
	
	if ( mass < 50 )
		scale = .5f;
	else if ( mass > 300 )
		scale = 1.5f;
	else if ( mass >= 200 )
		scale = mass / 200.0;
	else
		scale = .5f + ( (float)mass - 50.0f ) / 300.0f;
	
	// Add blood spurt
	
	blood = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
	blood->setModel( blood_model );

	if ( !isSubclassOf( Player ) )
	{
		// Network optimization

		dir[0] = -direction[0];
		dir[1] = -direction[1];
		dir[2] = -direction[2];
		blood->angles = dir.toAngles();

		blood->setAngles( blood->angles );
	}
	
	// Make the blood come from the centroid of the bone if possible
	
	if ( bone_number >= 0 )
	{
		int parent_bone_number;
		Vector bone_pos;
		Vector parent_bone_pos;
		Vector bone_centroid;
		
		parent_bone_number = gi.Bone_GetParentNum( edict->s.modelindex, bone_number );
		
		if ( parent_bone_number >= 0 )
		{
			GetTag( bone_number, &bone_pos );
			GetTag( parent_bone_number, &parent_bone_pos );
			
			bone_centroid = (bone_pos + parent_bone_pos ) * 0.5f;
			
			blood->setOrigin( bone_centroid );
		}
	}
	else
	{
		// Default the blood spurt to the position where the trace stopped
	
		blood->setOrigin( position );
	}

	
	//blood->origin.copyTo( blood->edict->s.origin2 );
	blood->setSolidType( SOLID_NOT );
	blood->setScale( scale );
	
	event = new Event( EV_Remove );
	blood->PostEvent( event, 1.0f );
	
	// Add blood splats near feet
	
	blood_splat_name = GetBloodSplatName();
	blood_splat_size = GetBloodSplatSize();
	
	if ( blood_splat_name.length() && G_Random() < 0.5f )
	{
		dir = origin - centroid;
		dir.z -= 50.0f;
		dir.x += G_CRandom( 20.0f );
		dir.y += G_CRandom( 20.0f );
		
		length = dir.length();
		
		dir.normalize();
		
		dir = dir * ( length + 10.0f );
		
		trace = G_Trace( centroid, vec_zero, vec_zero, centroid + dir, NULL, MASK_DEADSOLID, false, "AddBloodSpurt" );
		
		if ( trace.fraction < 1.0f )
		{
			Decal *decal = new Decal;
			decal->setShader( blood_splat_name );
			decal->setOrigin( Vector( trace.endpos ) + ( Vector( trace.plane.normal ) * 0.2f ) );
			decal->setDirection( trace.plane.normal );
			decal->setOrientation( "random" );
			decal->setRadius( blood_splat_size + G_Random( blood_splat_size ) );
		}
	}
}

qboolean Sentient::ShouldBleed( int meansofdeath, qboolean dead	)
{
	// Make sure we have a blood model
	
	if ( !blood_model.length() )
		return false;
	
	// Don't let the player bleed in single player

	if ( !multiplayerManager.inMultiplayer() && isSubclassOf( Player ) )
		return false;

	// Make sure if we are dead we are allowed to bleed after death
	
	if ( dead && this->isSubclassOf( Actor ) )
	{
		Actor *act = (Actor *)this;
		
		if ( !act->GetActorFlag( ACTOR_FLAG_BLEED_AFTER_DEATH ) )
			return false;
	}
	
	// See if we can bleed now based on means of death
	
	switch ( meansofdeath )
	{
		// Sometimes bleed (based on time)
		
	case MOD_BULLET :
	case MOD_STING :
	case MOD_STING2 :
	case MOD_VORTEX :
	case MOD_CHAINSWORD :
	case MOD_PLASMABEAM :
	case MOD_CRUSH_EVERY_FRAME :
	case MOD_POISON :
	case MOD_ELECTRICWATER :
	case MOD_EAT :
	case MOD_CIRCLEOFPROTECTION :
		
		if ( next_bleed_time > level.time )
			return false;
		
		break;
		
		// Sometimes bleed (based on chance)
		
	case MOD_PLASMASHOTGUN :
		
		if ( next_bleed_time < level.time )
			return true;

		if ( G_Random() > 0.1f )
			return false;
		
		break;
		
		// Never bleed
		
	case MOD_LIFEDRAIN :
	case MOD_SLIME :
	case MOD_LAVA :
	case MOD_GAS :
	case MOD_GAS_BLOCKABLE :
	case MOD_FIRE :
	case MOD_FIRE_BLOCKABLE :
	case MOD_SLING :
	case MOD_DROWN :
	case MOD_FLASHBANG :
	case MOD_ON_FIRE :
	case MOD_FALLING :
	case MOD_RADIATION :
	case MOD_DEATH_QUAD :
		return false;
	}
	
	// Always bleed by default
	
	return true;
}

// ShouldGib assumes that ShouldBleed has already been called

qboolean Sentient::ShouldGib( int meansofdeath, float damage )
{
	// See if we can gib based on means of death
	
	switch ( meansofdeath )
	{
		// Always gib
		
	case MOD_CHAINSWORD :
	case MOD_CRUSH_EVERY_FRAME :
	case MOD_PLASMASHOTGUN :
	case MOD_EAT :
	case MOD_EXPLOSION :
		return true;
		
		break;
		
		// Sometimes gib
		
	case MOD_BULLET :
	case MOD_STING :
	case MOD_STING2 :
		
		if ( G_Random( 100.0f ) < damage * 10.0f )
			return true;
		
		break;
		
	case MOD_PLASMABEAM :
		
		if ( G_Random( 100.0f ) < damage * 5.0f )
			return true;
		
		break;
		
		// Never gib
		
	case MOD_LIFEDRAIN :
	case MOD_SLIME :
	case MOD_LAVA :
	case MOD_GAS :
	case MOD_GAS_BLOCKABLE :
	case MOD_POISON :
	case MOD_FIRE :
	case MOD_FIRE_BLOCKABLE :
	case MOD_SLING :
	case MOD_DROWN :
	case MOD_FLASHBANG :
	case MOD_ON_FIRE :
	case MOD_FALLING :
	case MOD_RADIATION :
	case MOD_VORTEX :
	case MOD_ELECTRICWATER :		
		return false;
	}
	
	// Default is random based on how much damage done
	
	if ( G_Random( 100.0f ) < damage * 2.0f )
		return true;
	
	return false;
}

str Sentient::GetBloodSpurtName( void )
{
	str blood_spurt_name;
	
	if ( blood_model == "fx/fx_bspurt.tik" )
		blood_spurt_name = "fx/fx_bspurt2.tik";
	else if ( blood_model == "fx/fx_gspurt.tik" )
		blood_spurt_name = "fx/fx_gspurt2.tik";
	else if ( blood_model == "fx/fx_bspurt_blue.tik" )
		blood_spurt_name = "fx/fx_bspurt2_blue.tik";
	
	return blood_spurt_name;
}

str Sentient::GetBloodSplatName( void )
{
	str blood_splat_name;
	
	if ( blood_model == "fx/fx_bspurt.tik" )
		blood_splat_name = "bloodsplat.spr";
	else if ( blood_model == "fx/fx_gspurt.tik" )
		blood_splat_name = "greensplat.spr";
	else if ( blood_model == "fx/fx_bspurt_blue.tik" )
		blood_splat_name = "bluesplat.spr";
	
	return blood_splat_name;
}

float Sentient::GetBloodSplatSize( void )
{
	float m;
	
	m = (int)mass;
	
	if ( m < 50.0f )
		m = 50.0f;
	else if ( m > 250.0f )
		m = 250.0f;
	
	return ( 10.0f + ( m - 50.0f ) / 200.0f * 6.0f );
}

str Sentient::GetGibName( void )
{
	str gib_name;
	
	if ( blood_model == "fx/fx_bspurt.tik" )
		gib_name = "fx/fx_rgib";
	else if ( blood_model == "fx/fx_gspurt.tik" )
		gib_name = "fx/fx_ggib";
	else if ( blood_model == "fx/fx_bspurt_green.tik" )
		gib_name = "fx/fx_ggib";
	
	return gib_name;
}

int Sentient::NumInventoryItems( void )
{
	return inventory.NumObjects();
}

Item *Sentient::NextItem( Item *item )
{
	Item		*next_item;
	int		i;
	int		n;
	qboolean item_found = false;
	
	if ( !item )
	{
		item_found = true;
	}
	else if ( !inventory.ObjectInList( item->entnum ) )
	{
		error( "NextItem", "Item not in list" );
	}
	
	n = inventory.NumObjects();
	
	for( i = 1; i <= n; i++ )
	{
		next_item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		assert( next_item );
		
		if ( ( next_item->isSubclassOf( InventoryItem ) || ( next_item->isSubclassOf( Weapon ) ) )&& item_found )
			return next_item;
		
		if ( next_item == item )
			item_found = true;
	}
	
	return NULL;
}

Item *Sentient::PrevItem( Item *item )
{
	Item		*prev_item;
	int		i;
	int		n;
	qboolean item_found = false;
	
	if ( !item )
	{
		item_found = true;
	}
	else if ( !inventory.ObjectInList( item->entnum ) )
	{
		error( "NextItem", "Item not in list" );
	}
	
	n = inventory.NumObjects();
	
	for( i = n; i >= 1; i-- )
	{
		prev_item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		assert( prev_item );
		
		if ( prev_item->isSubclassOf( InventoryItem ) && item_found)
			return prev_item;
		
		if ( prev_item == item )
			item_found = true;
	}
	
	return NULL;
}

void Sentient::DropInventoryItems( void )
{
	int   num;
	int   i;
	Item  *item;
	
	// Drop any inventory items
	num = inventory.NumObjects();
	for( i = num; i >= 1; i-- )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		if ( item->isSubclassOf( InventoryItem ) )
		{
			item->Drop();
		}
	}
}

void Sentient::setModel( const char *mdl )
{
	// Rebind all active weapons
	
	DetachAllActiveWeapons();
	Entity::setModel( mdl );
	AttachAllActiveWeapons();
}

void Sentient::TurnOffShadow( Event *ev )
{
	edict->s.renderfx &= ~RF_SHADOW;
}

void Sentient::TurnOnShadow( Event *ev )
{
	edict->s.renderfx |= RF_SHADOW;
}

void Sentient::Archive( Archiver &arc )
{
	int i;
	int num;
	Resistance *resistance;


	Entity::Archive( arc );

	arc.ArchiveFloat( &_criticalHealthPercentage );

	// Archive Damage Threshold;
	arc.ArchiveFloat( &_damageThreshold.duration	  );
	arc.ArchiveFloat( &_damageThreshold.startTime	  );
	arc.ArchiveFloat( &_damageThreshold.maxDamage	  );
	arc.ArchiveFloat( &_damageThreshold.currentDamage );
	arc.ArchiveFloat( &shotsFiredThisVolley );

	arc.ArchiveFloat( &_hateModifier );

	inventory.Archive( arc );

	if ( arc.Saving() )
	{
		num = ammo_inventory.NumObjects();
	}
	else
	{
		ammo_inventory.ClearObjectList();
	}

	arc.ArchiveInteger( &num );

	for( i = 1; i <= num; i++ )
	{
		Ammo * ptr;

		if ( arc.Loading() )
		{
			ptr = new Ammo;
			ammo_inventory.AddObject( ptr );
		}
		else
		{
			ptr = ammo_inventory.ObjectAt( i );
		}

		arc.ArchiveObject( ptr );
	}

	arc.ArchiveSafePointer( &newWeapon );
	arc.ArchiveSafePointer( &currentBaseArmor );

	//arc.ArchiveInteger( &firing_frame );
	//arc.ArchiveInteger( &firing_anim );

	arc.ArchiveVector( &offset_color );
	arc.ArchiveVector( &offset_delta );
	arc.ArchiveFloat( &offset_time );

	arc.ArchiveFloat( &knock_start_time );

	arc.ArchiveString( &blood_model );

	arc.ArchiveInteger( &last_surface_hit );
	arc.ArchiveInteger( &last_bone_hit );
	arc.ArchiveUnsigned( &last_region_hit );

	immunities.Archive( arc );

	if ( arc.Saving() )
	{
		num = resistances.NumObjects();

		arc.ArchiveInteger( &num );

		for( i = 1 ; i <= num ; i++ )
		{
			resistance = resistances.ObjectAt( i );

			arc.ArchiveInteger( &resistance->MODIndex );
			arc.ArchiveInteger( &resistance->ResistanceAmount );
		}
	}
	else
	{
		arc.ArchiveInteger( &num );

		for( i = 1 ; i <= num ; i++ )
		{
			resistance = new Resistance;
			resistances.AddObject( resistance );

			arc.ArchiveInteger( &resistance->MODIndex );
			arc.ArchiveInteger( &resistance->ResistanceAmount );
		}
	}

	for( i = 0; i < MAX_ACTIVE_WEAPONS; i++ )
	{
		arc.ArchiveSafePointer( &activeWeaponList[ i ] );
	}

	arc.ArchiveUnsigned( &_viewMode );

	arc.ArchiveBool( &_canSendInjuredEvent );
	arc.ArchiveBool( &_headWatchAllowed );
	arc.ArchiveBool( &_displayFireEffect );

	arc.ArchiveVector( &gunoffset );
	arc.ArchiveVector( &eyeposition );
	arc.ArchiveInteger( &viewheight );
	arc.ArchiveInteger( &means_of_death );
	arc.ArchiveBoolean( &in_melee_attack );
	arc.ArchiveBoolean( &in_ranged_attack );
	arc.ArchiveBoolean( &in_block );
	arc.ArchiveBoolean( &in_stun );

	arc.ArchiveBoolean( &on_fire );
	arc.ArchiveFloat( &on_fire_stop_time );
	arc.ArchiveFloat( &next_catch_on_fire_time );
	arc.ArchiveInteger( &on_fire_tagnums[ 0 ] );
	arc.ArchiveInteger( &on_fire_tagnums[ 1 ] );
	arc.ArchiveInteger( &on_fire_tagnums[ 2 ] );
	arc.ArchiveSafePointer( &fire_owner );

	arc.ArchiveBoolean( &attack_blocked );
	arc.ArchiveFloat( &attack_blocked_time );

	arc.ArchiveFloat( &max_mouth_angle );
	arc.ArchiveInteger( &max_gibs );

	arc.ArchiveFloat( &next_bleed_time );
}

void Sentient::ArchivePersistantData( Archiver &arc, qboolean sublevelTransition )
{
	int i;
	int num;
	
	
	if ( arc.Loading() )
		FreeInventory();
	
	// archive the inventory
	if ( arc.Saving() )
	{
		// count up the total number
		num = inventory.NumObjects();
	}
	else
	{
		inventory.ClearObjectList();
	}
	// archive the number
	arc.ArchiveInteger( &num );
	// archive each item
	for( i = 1; i <= num; i++ )
	{
		str name;
		int amount;
		Item * item = NULL;
		
		if ( arc.Saving() )
		{
			Entity * ent;
			
			ent = G_GetEntity( inventory.ObjectAt( i ) );

			if ( ent && ent->isSubclassOf( Armor ) )
			{
				item = ( Item * )ent;
				name = item->getName();
				amount = item->getAmount();
			}
			else if ( ent && ent->isSubclassOf( Item ) )
            {
				item = ( Item * )ent;
				name = item->model;
				amount = item->getAmount();
            }
			else
            {
				error( "ArchivePersistantData", "Non Item in inventory\n" );
            }
		}
		arc.ArchiveString( &name );
		arc.ArchiveInteger( &amount );
		if ( arc.Loading() )
		{
			item = giveItem( name, amount );
			item->CancelEventsOfType( EV_Weapon_GiveStartingAmmo );
			item->ProcessPendingEvents();
		}

		if ( item )
		{
			item->ArchivePersistantData( arc );
		}
	}
	
	// archive the ammo inventory
	if ( arc.Saving() )
	{
		// count up the total number
		num = ammo_inventory.NumObjects();
	}
	else
	{
		ammo_inventory.ClearObjectList();
	}
	// archive the number
	arc.ArchiveInteger( &num );
	// archive each item
	for( i = 1; i <= num; i++ )
	{
		str name;
		int amount;
		int maxamount;
		Ammo * ptr;
		
		if ( arc.Saving() )
		{
			ptr = ammo_inventory.ObjectAt( i );
			name = ptr->getName();
			amount = ptr->getAmount();
			maxamount = ptr->getMaxAmount();
		}
		arc.ArchiveString( &name );
		arc.ArchiveInteger( &amount );
		arc.ArchiveInteger( &maxamount );
		if ( arc.Loading() )
		{
			GiveAmmo( name, amount, false, maxamount );
		}
	}
	
	for( i = 0; i < MAX_ACTIVE_WEAPONS; i++ )
	{
		str name;
		if ( arc.Saving() )
		{
			if ( activeWeaponList[ i ] )
            {
				name = activeWeaponList[ i ]->getName();
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
				Weapon * weapon;
				weapon = ( Weapon * )FindItem( name );
				if ( weapon )
				{
					ChangeWeapon( weapon, ( weaponhand_t )i );
				}
            }
		}
	}
	
	arc.ArchiveFloat( &health );
	arc.ArchiveFloat( &max_health );

	if ( arc.Loading() )
	{
		currentBaseArmor = (Armor *)FindBaseArmor();
	}
}

void Sentient::WeaponKnockedFromHands( void )
{
	str realname;
	
	realname = GetRandomAlias( "snd_lostweapon" );
	if ( realname.length() > 1 )
	{
		Sound( realname.c_str() , CHAN_VOICE );
	}
	else
	{
		Sound( "snd_pain", CHAN_VOICE );
	}
}

void Sentient::AddImmunity( Event *ev )
{
	str immune_string;
	int new_immunity;
	int number_of_immunities;
	int i;
	
	// No Longer using Immunities.  Now using Resistances
	// A resistance of 100 is immune.  However due to a large 
	// amount of legacy code, the immunity functions will still
	// be inplace, however, they will instead be used as wrappers
	// for the resistance functions
	/*
	number_of_immunities = ev->NumArgs();
	
	for ( i = 1 ; i <= number_of_immunities ; i++ )
	{
		immune_string = ev->GetString( i );

		new_immunity = MOD_NameToNum( immune_string );

		if ( new_immunity != -1 )
			immunities.AddUniqueObject( new_immunity );
	}
	*/
	
	Resistance *new_resistance;
	number_of_immunities = ev->NumArgs();
	
	for ( i = 1 ; i <= number_of_immunities ; i ++ )
	{
		immune_string = ev->GetString( i );
		
		new_immunity = MOD_NameToNum( immune_string );
		
		if ( new_immunity != -1 )
		{
			new_resistance = new Resistance;
			new_resistance->MODIndex = new_immunity;
			new_resistance->ResistanceAmount = 100;
			resistances.AddUniqueObject( new_resistance );
		}
		
	}
}

void Sentient::AddResistance( Event *ev )
{
	Resistance *new_resistance = new Resistance;
	
	if(new_resistance)
	{
		new_resistance->MODIndex = MOD_NameToNum(ev->GetString(1));
		new_resistance->ResistanceAmount = ev->GetInteger(2);
		resistances.AddUniqueObject( new_resistance );
	}
}

void Sentient::RemoveImmunity( Event *ev )
{
	str immune_string;
	int old_immunity;
	int number_of_immunities;
	int i;
	
	Resistance *old_resistance;
	// No Longer using Immunities.  Now using Resistances
	// A resistance of 100 is immune.  However due to a large 
	// amount of legacy code, the immunity functions will still
	// be inplace, however, they will instead be used as wrappers
	// for the resistance functions
	
	/*
	number_of_immunities = ev->NumArgs();
	
	for ( i = 1 ; i <= number_of_immunities ; i++ )
	{
		immune_string = ev->GetString( i );
	  
		old_immunity = MOD_NameToNum( immune_string );
		
		if ( old_immunity != -1 && immunities.ObjectInList( old_immunity ) )
			immunities.RemoveObject( old_immunity );
	}*/
	
	number_of_immunities = ev->NumArgs();
	
	for ( i = 1 ; i <= number_of_immunities ; i++ )
	{
		immune_string = ev->GetString( i );
		
		old_immunity = MOD_NameToNum( immune_string );
		
		for ( int x = resistances.NumObjects(); x > 0 ; x-- )
		{
			old_resistance = resistances.ObjectAt(i);
			if( old_resistance->MODIndex == old_immunity )
			{
				resistances.RemoveObjectAt( x );
				delete old_resistance;
				old_resistance = 0;
			}
		}
	}
}

void Sentient::RemoveResistance( Event *ev )
{
	Resistance *old_resistance;
	int resistance_idx;
	
	resistance_idx = MOD_NameToNum( ev->GetString(1) );
	
	for ( int i = resistances.NumObjects(); i > 0 ; i-- )
	{
		old_resistance = resistances.ObjectAt(i);
		if( old_resistance->MODIndex == resistance_idx )
		{
			resistances.RemoveObjectAt( i );
			delete old_resistance;
			old_resistance = 0;
		}
	}
	
}

qboolean Sentient::Immune( int meansofdeath )
{
	int number_of_immunities, i;
	
	number_of_immunities = immunities.NumObjects();
	
	for( i = 1 ; i <= number_of_immunities ; i++ )
	{
		if ( meansofdeath == immunities.ObjectAt( i ) )
			return true;
	}
	
	return false;
}

qboolean Sentient::Resistant( int meansofdeath )
{
	int number_of_resistances = resistances.NumObjects();
	Resistance *checkResistance;
	
	for( int i = 1; i <= number_of_resistances; i++ )
	{
		checkResistance = resistances.ObjectAt(i);
		
		if( meansofdeath == checkResistance->MODIndex )
			return true;
	}
	
	return false;
	
}

int Sentient::GetResistanceModifier( int meansofdeath )
{
	int number_of_resistances = resistances.NumObjects();
	Resistance *checkResistance;
	
	for( int i = 1; i <= number_of_resistances; i++ )
	{
		checkResistance = resistances.ObjectAt(i);
		
		if( meansofdeath == checkResistance->MODIndex )
			return checkResistance->ResistanceAmount;
	}
	
	return 0;
	
}

Ammo *Sentient::FindAmmoByName( const str &name )
{
	int count, i;
	
	count = ammo_inventory.NumObjects();
	
	for ( i=1; i<=count; i++ )
	{
		Ammo *ammo = ammo_inventory.ObjectAt( i );
		if ( name == ammo->getName() )
		{
			return ammo;
		}
	}
	return NULL;
}

int Sentient::AmmoIndex( const str &type )
{
	Ammo *ammo;
	
	ammo = FindAmmoByName( type );
	
	if ( ammo )
		return ammo->getIndex();
	else
		return 0;
}

int Sentient::AmmoCount( const str &type )
{
	Ammo *ammo;
	
	if ( !type.length() )
		return 0;
	
	ammo = FindAmmoByName( type );
	
	if ( ammo )
		return ammo->getAmount();
	else
		return 0;
}

int Sentient::MaxAmmoCount( const str &type )
{
	Ammo *ammo;
	
	ammo = FindAmmoByName( type );
	
	if ( ammo )
		return ammo->getMaxAmount();
	else
		return 0;
}

int Sentient::GiveAmmo( const str &type, int amount, bool pickedUp, int maxamount )
{
	Ammo *ammo;
	int startAmount = 0;
	int totalAmount = 0;
	int amountLeft;
	
	ammo = FindAmmoByName( type );
	
	if ( ammo )
	{
		// Add amount to current amount
		startAmount = ammo->getAmount();
		
		if ( maxamount >= 0 )
			ammo->setMaxAmount( maxamount );

		ammo->setAmount( ammo->getAmount() + amount );
	}
	else
	{
		// Create a new inventory entry with this name
		ammo = new Ammo;
		
		if ( maxamount >= 0 )
			ammo->setMaxAmount( maxamount );
		
		ammo->setAmount( amount );
		
		ammo->setName( type );
		ammo_inventory.AddObject( ammo );
	}
	
	totalAmount = ammo->getAmount() - startAmount;

	amountLeft = amount - totalAmount;

	if ( amountLeft > 0 )
	{
		int entnum;
		Item *item;
		Weapon *weapon;
		int roomLeftInClip;
		int amountToAdd;
		int i;

		// Try to give directly to weapons now
	
		for ( i = 1 ; i <= inventory.NumObjects() ; i++ )
		{
			entnum = inventory.ObjectAt( i );

			item = ( Item * )G_GetEntity( entnum );

			if ( item->isSubclassOf( Weapon ) )
			{
				weapon = (Weapon *)item;

				if ( weapon->HasFullClip() )
					continue;

				if ( stricmp( weapon->GetAmmoType( FIRE_MODE1 ).c_str(), type.c_str() ) != 0 )
					continue;

				roomLeftInClip = weapon->GetClipSize( FIRE_MODE1 ) - weapon->getAmmoInClip( FIRE_MODE1 );

				if ( roomLeftInClip )
				{
					if ( amountLeft > roomLeftInClip )
						amountToAdd = roomLeftInClip;
					else
						amountToAdd = amountLeft;

					amountLeft -= amountToAdd;

					weapon->SetAmmoAmount( weapon->getAmmoInClip( FIRE_MODE1 ) + amountToAdd, FIRE_MODE1 );
				}

				if ( amountLeft <= 0 )
				{
					break;
				}
			}
		}

	}
	
	if ( pickedUp && this->isSubclassOf( Player ) && ( amount - amountLeft > 0 ) )
	{
		int iconIndex;
		str imageName;

		imageName = "sysimg/icons/items/ammo_";
		imageName += type;

		iconIndex = gi.imageindex( imageName );

		((Player *)this)->setItemText( iconIndex, va( "$$PickedUp$$ %d $$Ammo-%s$$\n", amount, type.c_str() ) );
		//((Player *)this)->setItemText( iconIndex, va( "$$PickedUp$$ %d $$Ammo-%s$$\n", amount - amountLeft, type.c_str() ) );
		//gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$PickedUp$$ %d %s\n", totalAmount, type.c_str() );
	}
	
	AmmoAmountChanged( ammo );

	return amount - amountLeft;
}

int Sentient::UseAmmo( const str &type, int amount )
{
	int count, i;
	
	count = ammo_inventory.NumObjects();
	
	for ( i=1; i<=count; i++ )
	{
		Ammo *ammo = ammo_inventory.ObjectAt( i );
		if ( type == ammo->getName() )
		{
			int ammo_amount = ammo->getAmount();
			
			// Less ammo than what we specified to use
			if ( ammo_amount < amount )
            {
				ammo->setAmount( 0 );
				AmmoAmountChanged( ammo );
				return ammo_amount;
            }
			else
            {
				ammo->setAmount( ammo->getAmount() - amount );
				AmmoAmountChanged( ammo );
				return amount;
            }
		}
	}
	return 0;
}

void Sentient::AmmoAmountInClipChanged( const str &type, int amount_in_clip )
{
	int count, i;
	
	count = ammo_inventory.NumObjects();
	
	for ( i=1; i<=count; i++ )
	{
		Ammo *ammo = ammo_inventory.ObjectAt( i );
		if ( type == ammo->getName() )
		{
			AmmoAmountChanged( ammo, amount_in_clip );
		}
	}
}

void Sentient::JumpXY( Event *ev )
{
	float forwardmove;
	float sidemove;
	float distance;
	float time;
	float speed;
	Vector yaw_forward;
	Vector yaw_left;
	
	forwardmove = ev->GetFloat( 1 );
	sidemove    = ev->GetFloat( 2 );
	speed       = ev->GetFloat( 3 );
	
	Vector( 0.0f, angles.y, 0.0f ).AngleVectors( &yaw_forward, &yaw_left );
	
	velocity = ( yaw_forward * forwardmove ) - ( yaw_left * sidemove );
	distance = velocity.length();
	velocity *= speed / distance;
	time = distance / speed;
	velocity[ 2 ] = sv_currentGravity->integer * time * 0.5f;
}

void Sentient::MeleeAttackStart( Event *ev )
{
	int i;
	Entity *victim;
	Vector mins, maxs, pos, end;
	Container<Entity *> potential_victimlist;
	
	mins = Vector( -8, -8, -8 );
	maxs = Vector( 8, 8, 8 );
	pos = centroid;
	end = centroid + Vector( orientation[0] ) * 96.0f;
	
	G_TraceEntities( pos, mins, maxs, end, &potential_victimlist, MASK_MELEE );
	
	for( i = 1 ; i <= potential_victimlist.NumObjects() ; i++ )
	{
		victim = potential_victimlist.ObjectAt( i );
		Event *event = new Event(EV_Sentient_AddMeleeAttacker);
		event->AddEntity(this);
		victim->ProcessEvent(event);
	}
	
	SetMeleeAttack(true);
}

void Sentient::MeleeAttackEnd( Event *ev )
{
	SetMeleeAttack(false);
}

void Sentient::SetMeleeAttack(bool value)
{
	in_melee_attack = value;
}

void Sentient::RangedAttackStart( Event *ev )
{
	in_ranged_attack = true;
}

void Sentient::RangedAttackEnd( Event *ev )
{
	in_ranged_attack = false;
}

void Sentient::BlockStart( Event *ev )
{
	in_block = true;
}

void Sentient::BlockEnd( Event *ev )
{
	in_block = false;
}

void Sentient::StunStart( Event *ev )
{
	in_stun = true;
}

void Sentient::StunEnd( Event *ev )
{
	in_stun = false;
}

void Sentient::ListInventory( void )
{
	int i,count;
	
	// Display normal inventory
	count = inventory.NumObjects();
	
	gi.Printf( "'Name' : 'Amount'\n" );
	
	for ( i=1; i<=count; i++ )
	{
		int entnum = inventory.ObjectAt( i );
		Item *item = ( Item * )G_GetEntity( entnum );
		gi.Printf( "'%s' : '%d'\n", item->getName().c_str(), item->getAmount() );
	}
	
	// Display ammo inventory
	count = ammo_inventory.NumObjects();
	
	for ( i=1; i<=count; i++ )
	{
		Ammo *ammo = ammo_inventory.ObjectAt( i );
		gi.Printf( "'%s' : '%d'\n", ammo->getName().c_str(), ammo->getAmount() );
	}
}

void Sentient::SetAttackBlocked( qboolean blocked )
{
	attack_blocked      = blocked;
	attack_blocked_time = level.time;
}

void Sentient::SetMaxMouthAngle( Event *ev )
{
	max_mouth_angle = ev->GetFloat( 1 );
}


//----------------------------------------------------------------
// Name:			CatchOnFire
// Class:			Sentient
//
// Description:		Sets the character on fire
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Sentient::CatchOnFire( Event *ev )
{
	TryLightOnFire(MOD_FIRE, NULL);
}

void Sentient::TryLightOnFire( int meansofdeath, Entity *attacker )
{
	float chance;
	float min_time;
	float add_time;
	
	
	if ( !com_blood->integer )
		return;
	
	// Get the base chance of catching on fire
	
	if ( meansofdeath == MOD_FIRESWORD )
		chance = .5f;
	else
		chance = .05f;
	
	// Get some values based on whether or not the sentient is a player or an actor
	
	if ( this->isSubclassOf( Player ) )
	{
		chance = chance / 2.0f;
		min_time = 4.0f;
		add_time = 2.0f;
	}
	else
	{
		min_time = 8.0f;
		add_time = 4.0f;
		
		// Going to use MOD_FIRE to force fire on Actors - not players
		if ( meansofdeath == MOD_FIRE )
			chance = 1.0f;
	}
	
	// Make sure not immune to on_fire
	
	if ( Immune( MOD_ON_FIRE ) )
		return;
	
	// See if we should catch the victim on fire
	
	if ( G_Random() < chance )
	{
		Event *event;
		int number_of_tags;
		int i;
		const char *tag_name;
		int number_of_fires_to_add;
		float scale;
		float chanceToChange;
		
		if ( !on_fire )
		{
			on_fire_tagnums[0] = -1;
			on_fire_tagnums[1] = -1;
			on_fire_tagnums[2] = -1;
		}
		
		if ( mass >= 200 )
			number_of_fires_to_add = 3;
		else if ( mass >= 100 )
			number_of_fires_to_add = 2;
		else
			number_of_fires_to_add = 1;
		
		if ( mass > 200 )
			scale = 1.25f;
		else if ( mass >= 300 )
			scale = 1.75f;
		else
			scale = 1.00f;
		
		chanceToChange = 0.05f;
		
		number_of_tags = gi.NumTags( edict->s.modelindex );
		
		if ( number_of_tags )
		{
			for ( i = 0 ; i < number_of_fires_to_add ; i++ )
			{
				if ( !on_fire || ( G_Random() < chanceToChange ) )
				{
					if ( on_fire && ( on_fire_tagnums[i] >= 0 ) )
					{
						tag_name = gi.Tag_NameForNum( edict->s.modelindex, on_fire_tagnums[i] );
						
						if ( _displayFireEffect )
						{
							event = new Event( EV_RemoveAttachedModel );
							event->AddString( tag_name );
							event->AddFloat( 0.0f );
							event->AddString( "models/fx_catchfire.tik" );
							ProcessEvent( event );
						}
					}
					
					on_fire_tagnums[i] = G_Random( number_of_tags );
					tag_name = gi.Tag_NameForNum( edict->s.modelindex, on_fire_tagnums[i] );
					
					if ( _displayFireEffect )
					{
						event = new Event( EV_AttachModel );
						event->AddString( "fx_catchfire.tik" );
						event->AddString( tag_name );
						event->AddFloat( scale );
						ProcessEvent( event );
					}
				}
			}
			
			if ( !on_fire )
			{
				on_fire = true;
				fire_owner = attacker;
				PostEvent( EV_Sentient_OnFire, FRAMETIME );
				next_catch_on_fire_time = level.time + .15f + G_Random( .2f );
				on_fire_stop_time = level.time + min_time + G_Random( add_time );
			}
		}
	}
}

void Sentient::OnFire( Event *ev )
{
	float damage;
	
	// See if we should stop being on fire
	
	if ( !on_fire || ( on_fire_stop_time <= level.time ) || ( gi.pointcontents( origin, 0 ) & MASK_WATER ) )
	{
		// Stop the fire
		
		ProcessEvent( EV_Sentient_StopOnFire );
		return;
	}
	
	// Cause a little bit of damage
	
	if ( this->isSubclassOf( Player ) )
		damage = 0.05;
	else
		damage = 0.5;

	if ( fire_owner && fire_owner->isSubclassOf( Player ) )
	{
		Player *player = (Player *)(Entity *)fire_owner;
		
		damage = player->getDamageDone( damage, MOD_ON_FIRE, false );
	}
	
	Damage( fire_owner, fire_owner, damage, vec_zero, vec_zero, vec_zero, 0, 0, MOD_ON_FIRE );
	
	// Try to set other sentients on fire
	
	if ( next_catch_on_fire_time <= level.time )
	{
		MeleeAttack( centroid, centroid, 0.05f, fire_owner, MOD_FIRE, maxs[0] + 10, -maxs[2] / 2, maxs[2] / 2, 0.0f, false );
		next_catch_on_fire_time = level.time + .15f + G_Random( .2f );
	}
	
	// Call this event again next frame
	
	PostEvent( EV_Sentient_OnFire, FRAMETIME );
}

void Sentient::StopOnFire( Event *ev )
{
	int i;
	const char *tag_name;
	Event *event;
	
	
	for( i = 0 ; i < 3 ; i++ )
	{
		if ( ( on_fire_tagnums[i] >= 0 ) && ( on_fire_tagnums[i] < gi.NumTags( edict->s.modelindex ) ) )
		{
			tag_name = gi.Tag_NameForNum( edict->s.modelindex, on_fire_tagnums[i] );
	
			if ( _displayFireEffect )
			{
				event = new Event( EV_RemoveAttachedModel );
				event->AddString( tag_name );
				event->AddFloat( 0.0f );
				event->AddString( "models/fx_catchfire.tik" );
				ProcessEvent( event );
			}
		}
	}

	on_fire_tagnums[ 0 ] = -1;
	on_fire_tagnums[ 1 ] = -1;
	on_fire_tagnums[ 2 ] = -1;
	
	on_fire = false;

	CancelEventsOfType( EV_Sentient_OnFire );
}

void Sentient::SpawnBloodyGibs( Event *ev )
{
	str gib_name;
	int number_of_gibs;
	float scale;
	Entity *ent;
	str real_gib_name;
	
	if ( !com_blood->integer )
		return;
	
	//if ( GetActorFlag( ACTOR_FLAG_FADING_OUT ) )
	//	return;
	
	gib_name = GetGibName();
	
	if ( !gib_name.length() )
		return;
	
	// Determine the number of gibs to spawn
	
	if ( ev->NumArgs() > 0 )
	{
		number_of_gibs = ev->GetInteger( 1 );
	}
	else
	{
		if ( max_gibs == 0 )
			return;
		
		if ( deadflag )
			//number_of_gibs = G_Random( max_gibs / 2 ) + 1;
			number_of_gibs = max_gibs;
		else
			number_of_gibs = (int)G_Random( (float)max_gibs ) + 1;
	}
	
	// Make sure we don't have too few or too many gibs
	
	if ( ( number_of_gibs <= 0 ) || ( number_of_gibs > 9 ) )
		return;
	
	if ( ev->NumArgs() > 1 )
	{
		scale = ev->GetFloat( 2 );
	}
	else
	{
		// Calculate a good scale value
		
		if ( mass <= 50 )
			scale = 1.0;
		else if ( mass <= 100 )
			scale = 1.1;
		else if ( mass <= 250 )
			scale = 1.2;
		else
			scale = 1.3;
	}
	
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
	
	Sound( "snd_decap", CHAN_BODY, 1.0f, 300.0f );
}

void Sentient::SetMaxGibs( Event *ev )
{
	max_gibs = ev->GetInteger( 1 );
}

void Sentient::GetStateAnims( Container<const char *> *c )
{
}

void Sentient::CheckAnimations( Event *ev )
{
	int i,j;
	Container<const char *>co;
	const char *cs;
	
	GetStateAnims( &co );
	
	gi.DPrintf( "Unused Animations in TIKI\n" );
	gi.DPrintf( "-------------------------\n" );
	for( i=0; i<animate->NumAnims(); i++ )
	{
		const char *c;
		
		c = gi.Anim_NameForNum( edict->s.modelindex, i );
		
		for ( j=1; j<=co.NumObjects(); j++ )
		{
			cs = co.ObjectAt( j );
			
			if ( !stricmp( c, cs ) )
            {
				goto out;
            }
			else if ( !strnicmp( c, cs, strlen( cs ) ) ) // partial match
            {
				int state_len = strlen( cs );
				
				// Animation in tik file is longer than the state machine's anim
				if ( (int)strlen( c ) > state_len )
				{
					if ( c[state_len] != '_' ) // If next character is an '_' then no match
					{
						goto out;
					}
				}
				else
				{
					goto out;
				}
            }
		}
		// No match made
		gi.DPrintf( "%s used in TIK file but not statefile\n", c );
out:
		;
	}
	
	gi.DPrintf( "Unknown Animations in Statefile\n" );
	gi.DPrintf( "-------------------------------\n" );
	for ( j=1; j<=co.NumObjects(); j++ )
	{
		if ( !animate->HasAnim( co.ObjectAt( j ) ) )
		{
			gi.DPrintf( "%s in statefile is not in TIKI\n", co.ObjectAt( j ) );
		}
	}
}

void Sentient::SetStateFile( Event *ev )
{
	Event *temp=ev;
	if (ev != NULL)
	{
		temp=NULL;
	}
	
	// This function is not defined for a Sentient
	// If Sentient was an abstract class then this would be pure virtual
}

void Sentient::ReceivedItem( Item * item )
{
}

void Sentient::RemovedItem( Item * item )
{
}

void Sentient::AmmoAmountChanged( Ammo * ammo, int ammo_in_clip )
{
}

//
// Armor Stuff
//
void Sentient::SetArmorValue(int armorVal)
{
	if ( currentBaseArmor )
		currentBaseArmor->setAmount( armorVal );
	
}

int Sentient::GetArmorValue()
{
	if ( currentBaseArmor )
		return currentBaseArmor->getAmount();
	
	return 0;
}

//----------------------------------------------------------------
// Name:			setViewMode
// Class:			Sentient
//
// Description:		Sets the players current view mode
//
// Parameters:		Event *ev									- event (the name of the view mode to go to)
//
// Returns:			None
//----------------------------------------------------------------

void Sentient::setViewMode( Event *ev )
{
	str viewMode;
	bool override;

	viewMode = ev->GetString( 1 );

	if ( ev->NumArgs() > 1 )
		override = ev->GetBoolean( 2 );
	else
		override = true;

	if ( _viewMode && !override )
		return;

	setViewMode( ev->GetString( 1 ) );
}

//----------------------------------------------------------------
// Name:			setViewMode
// Class:			Sentient
//
// Description:		Sets the players current view mode
//
// Parameters:		const str &viewModeName					- the name of the view mode to go to
//
// Returns:			None
//----------------------------------------------------------------

void Sentient::setViewMode( const str &viewModeName )
{
	unsigned int newViewMode;

	newViewMode = gi.GetViewModeMask( viewModeName.c_str() );

	if ( _viewMode != newViewMode )
	{
		_viewMode = newViewMode;
	
		if ( this->isSubclassOf( Player ) )
		{
			gi.centerprintf ( edict, CENTERPRINT_IMPORTANCE_NORMAL, "$$SwitchedTo$$ $$ViewMode-%s$$ $$Viewmode$$", viewModeName.c_str() );
		}
	}
}

//----------------------------------------------------------------
// Name:			getViewMode
// Class:			Sentient
//
// Description:		Gets the current view mode of this sentient
//
// Parameters:		None
//
// Returns:			unsigned int								- the current view mode bit(s)
//----------------------------------------------------------------

unsigned int Sentient::getViewMode( void )
{
	return _viewMode;
}

//----------------------------------------------------------------
// Name:			getActiveWeaponName
// Class:			Sentient
//
// Description:		Gets the name of the active weapon
//
// Parameters:		Event *ev						- contains name of the hand (left, right, or dual)
//
// Returns:			str (through the event)			- name of the active weapon
//----------------------------------------------------------------

void Sentient::getActiveWeaponName( Event *ev )
{
	weaponhand_t	hand = WEAPON_ANY;
	str				weaponName;

	// See which hand we are concerned about

	if ( ev->NumArgs() > 0 )
	{
		hand = WeaponHandNameToNum( ev->GetString( 1 ) );
	}
	else
	{
		hand = WEAPON_ANY;
	}

	getActiveWeaponName(hand, weaponName);

	ev->ReturnString(weaponName);
}

void Sentient::getActiveWeaponName( weaponhand_t	hand, str& weaponName)
{
	Weapon* weapon;

	weaponName = "None";
	// Get the name of the weapon
	if ( hand == WEAPON_ANY )
	{
		// Try the left hand

		weapon = GetActiveWeapon( WEAPON_LEFT );

		if ( weapon )
			weaponName = weapon->getName();

		// Try the right hand

		if ( !weapon )
		{
			weapon = GetActiveWeapon( WEAPON_RIGHT );

			if ( weapon )
				weaponName = weapon->getName();
		}

		// Try the both hands

		if ( !weapon )
		{
			weapon = GetActiveWeapon( WEAPON_DUAL );

			if ( weapon )
				weaponName = weapon->getName();
		}
	}
	else if ( ( hand == WEAPON_LEFT ) || ( hand == WEAPON_RIGHT ) || ( hand == WEAPON_DUAL ) )
	{
		weapon = GetActiveWeapon( hand );

		if ( weapon )
			weaponName = weapon->getName();
	}

}

//--------------------------------------------------------------
//
// Name:			SwipeOn
// Class:			Sentient
//
// Description:		Turn the weapon swipes on for this sentient's weapons
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Sentient::SwipeOn( Event *ev )
{
	Weapon         *weapon;
	weaponhand_t   hand;
	
	hand = WeaponHandNameToNum( ev->GetString( 1 ) );
	
	if ( hand == WEAPON_ERROR )
		return;
	
	weapon = GetActiveWeapon( hand );
	
	if ( weapon )
		weapon->SetAnim( "swipeon", NULL );
}

//--------------------------------------------------------------
//
// Name:			SwipeOff
// Class:			Sentient
//
// Description:		Turn the weapon swipes off for this sentient's weapons
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Sentient::SwipeOff( Event *ev )
{
	Weapon         *weapon;
	weaponhand_t   hand;
	
	hand = WeaponHandNameToNum( ev->GetString( 1 ) );
	
	if ( hand == WEAPON_ERROR )
		return;
	
	weapon = GetActiveWeapon( hand );
	
	if ( weapon )
		weapon->SetAnim( "swipeoff", NULL );
}

//--------------------------------------------------------------
// Name:		AddHealthOverTime()
// Class:		Sentient
//
// Description:	Adds the specified percentage of health right away,
//				then posts an event to AddHealthAtInterval to
//				kick start a health regeneration event chain
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Sentient::AddHealthOverTime( Event *ev )
{
	float percentageToAdd	= ev->GetFloat( 1 );
	float percentageToRegen = ev->GetFloat( 2 );
	float interval			= ev->GetFloat( 3 );
	float maxPercentage		= ev->GetFloat( 4 );
	float addHealth;
	float maxHealth;
	Event *regenEvent;
	
	//Translate our percentages into real numbers
	addHealth = percentageToAdd * max_health;
	maxHealth = maxPercentage * max_health;
	
	//Check if adding the initial health will put us
	//over our maximum allowed
	if ( (addHealth + health) >= maxHealth )
	{
		health = maxHealth;
		return;
	}
	else
		health+= addHealth;
	
	//Now Generate our AddHealthAtInterval Event
	//which will continue the regeneration process
	regenEvent = new Event(EV_Sentient_HealAtInterval);
	
	if ( !regenEvent)
	{
		assert(regenEvent);
		return;
	}
	
	regenEvent->AddFloat(percentageToRegen);
	regenEvent->AddFloat(interval);
	regenEvent->AddFloat(maxPercentage);
	
	PostEvent( regenEvent, interval );	
}

//--------------------------------------------------------------
// Name:		AddHealthAtInterval()
// Class:		Sentient
//
// Description:	Adds the regen percentage to the health
//				of the percentage... if the health is
//				still not the maxPercentage, then it
//				will generate a new event of its own
//				type
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Sentient::AddHealthAtInterval( Event *ev )
{
	float percentageToRegen = ev->GetFloat( 1 );
	float interval			= ev->GetFloat( 2 );
	float maxPercentage		= ev->GetFloat( 3 );
	float addHealth;
	float maxHealth;
	Event *regenEvent;
	
	//Translate our percentages into real numbers
	addHealth = percentageToRegen * max_health;
	maxHealth = maxPercentage * max_health;
	
	//Check if adding the initial health will put us
	//over our maximum allowed
	if ( (addHealth + health) >= maxHealth )
	{
		SetHealth(maxHealth);
		return;
	}
	else
		SetHealth(health + addHealth);
	
	//Now Generate a new AddHealthAtInterval Event
	//which will continue the regeneration process
	regenEvent = new Event(EV_Sentient_HealAtInterval);
	
	if ( !regenEvent)
	{
		assert(regenEvent);
		return;
	}
	
	regenEvent->AddFloat(percentageToRegen);
	regenEvent->AddFloat(interval);
	regenEvent->AddFloat(maxPercentage);
	
	PostEvent( regenEvent, interval );
}

void Sentient::SetHealth( float newHealth )
{
	health = newHealth;

	if ( ( health / max_health ) > GetCriticalHealthPercentage() )
		_canSendInjuredEvent = true;
}

//----------------------------------------------------------------
// Name:			HeadWatchAllowed
// Class:			Sentient
//
// Description:		Sets whether to headwatch or not, defaults to true
//
// Parameters:		Event *ev
//
// Returns:			None
//----------------------------------------------------------------
void Sentient::HeadWatchAllowed(Event *ev)
{
	if ( ev->NumArgs() > 0 )
		HeadWatchAllowed( ev->GetBoolean( 1 ) );
	else
		HeadWatchAllowed( true );
}

void Sentient::HeadWatchAllowed( bool allowed )
{
	_headWatchAllowed = allowed;
}

//--------------------------------------------------------------
//
// Name:			SetWeaponAnim
// Class:			Sentient
//
// Description:		Put the weapon in the specified animation
//
// Parameters:		Event *ev
//
// Returns:			None
//
//--------------------------------------------------------------
void Sentient::SetWeaponAnim( Event *ev )
{
	Weapon			*weapon;
	weaponhand_t	hand;
	str				animname;
	
	animname = ev->GetString( 1 );
	hand = WeaponHandNameToNum( ev->GetString( 2 ) );
	
	if ( hand == WEAPON_ERROR )
		return;
	
	weapon = GetActiveWeapon( hand );
	
	if ( weapon )
		weapon->playAnim( animname );
}

//--------------------------------------------------------------
// Name:		SetDamageThreshold()
// Class:		Sentient
//
// Description:	Sets Values on our damageThreshold structure
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Sentient::SetDamageThreshold( Event *ev )
{
	_damageThreshold.maxDamage = ev->GetFloat( 1 );
	_damageThreshold.duration = ev->GetFloat( 2 );
	_damageThreshold.startTime = 0;
}

//--------------------------------------------------------------
// Name:		CheckDamageThreshold()
// Class:		Sentient
//
// Description:	Checks if we have hit our damageThreshold
//				meaning, we have taken at least X damage 
//				in Y amount of time.
//
// Parameters:	float damageValue
//
// Returns:		None
//--------------------------------------------------------------
void Sentient::CheckDamageThreshold( float damageValue )
{
	//First Check if we even need to bother doing further 
	//checks
	float timeDiff;
	
	if ( _damageThreshold.maxDamage < 0 )
		return;
	
	if( _damageThreshold.startTime == 0 )
		_damageThreshold.startTime = level.time;
	
	timeDiff = level.time - _damageThreshold.startTime;
	
	//Now we check if our timeDiff is greater than our 
	//damageThreshold's duration.  If it is, then obviously
	//we didn't hit our threshold, so we need to update 
	//our starttime and currentDamage;
	if ( _damageThreshold.duration >= 0 && timeDiff > _damageThreshold.duration )
	{
		_damageThreshold.startTime = level.time;
		_damageThreshold.currentDamage = damageValue;
		return;
	}
	
	//Well, we're still within our timeframe to hit our
	//threshold, so we'll add the damageValue to our 
	//damageThreshold's currentDamage and check to see
	//if we hit the wall.
	_damageThreshold.currentDamage += damageValue;
	
	if ( _damageThreshold.currentDamage >= _damageThreshold.maxDamage )
	{		
		//Yay, more casting... I'm so excited about this
		if ( this->isSubclassOf(Actor) )
		{
			Actor *actor;
			actor = (Actor*)this;			
			actor->AddStateFlag(STATE_FLAG_DAMAGE_THRESHOLD_EXCEEDED);
		}		
		
		//Clear out our structure
		ClearDamageThreshold();
	}
}

//--------------------------------------------------------------
// Name:		DisplayFireEffect
// Class:		Sentient
//
// Description:	Sets the flag to display the fire effect while this entity
//				is on fire.  Note: this doesn't prevent the "on fire" state,
//				nor does it prevent the damage from the fire.  It's just
//				the fire effect itself that's not displayed.
//
// Parameters:	Event *ev
//
// Returns:		None
//--------------------------------------------------------------
void Sentient::DisplayFireEffect( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		_displayFireEffect = ev->GetBoolean( 1 );
	else
		_displayFireEffect = true;
}

void Sentient::ClearDamageThreshold( Event *ev )
{
	ClearDamageThreshold();
}

void Sentient::ClearDamageThreshold()
{
	_damageThreshold.startTime = level.time;
	_damageThreshold.currentDamage = 0.0f;
}


//--------------------------------------------------------------
//
// Name:			DropItemEvent
// Class:			Sentient
//
// Description:		Drops the specified item to the ground as a pickup
//
// Parameters:		Event *ev
//						-- str itenName
//
// Returns:			None
//
//--------------------------------------------------------------
void Sentient::DropItemEvent( Event *ev )
{
	int   num, i;
	Item  *item;

	str itemName = ev->GetString( 1 );
	num = inventory.NumObjects();
	for( i = num; i >= 1; i-- )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		if ( item->getName() == itemName )
		{
			DropItem(item);
			return;
		}
	}
}


//--------------------------------------------------------------
//
// Name:			DropItem
// Class:			Sentient
//
// Description:		Drops the specified item.  This function 
//					may grow.
//
// Parameters:		Item *itemToDrop -- Item pointer to drop
//
// Returns:			None
//
//--------------------------------------------------------------
void Sentient::DropItem( Item *itemToDrop )
{
	if ( !itemToDrop )
		return;

	itemToDrop->Drop();
}


//--------------------------------------------------------------
//
// Name:			DropItemsOnDeath
// Class:			Sentient
//
// Description:		Drops the item/weapon when the sentient dies
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void Sentient::DropItemsOnDeath()
{
	Weapon *weap = 0;
	weap = GetActiveWeapon(WEAPON_RIGHT);
	if ( !weap )
		weap = GetActiveWeapon(WEAPON_LEFT);
	if ( weap )
	{
		GameplayManager *gpm = GameplayManager::getTheGameplayManager();
		if ( gpm->hasProperty(weap->getArchetype(), "dropchance") )
		{
			float dropchance = gpm->getFloatValue(weap->getArchetype(), "dropchance");
			if ( G_Random() < dropchance )
				DropItem(weap);
		}

		if ( gpm->hasProperty(getArchetype(), "potionchance") )
		{
			str model;

			float potionchance = gpm->getFloatValue(getArchetype(), "potionchance");
			if ( G_Random() < potionchance )
			{
				model = gpm->getStringValue( "HealthPotion", "model");
			}
			else if ( G_Random() < potionchance * 2.0f )
			{
				model = gpm->getStringValue( "RedPotion", "model" );
			}

			if ( model.length() )
			{
				// Spawn the potion
				SpawnArgs args;
				Entity *ent;
				Item *item;
				args.setArg( "model", model );
				ent = args.Spawn();
				if ( !ent || !ent->isSubclassOf( Item ) )
					return;
				item = (Item *)ent;
				item->setOrigin( centroid );
				item->ProcessPendingEvents();
				item->PlaceItem();
				item->setOrigin( centroid );
				item->velocity = Vector( G_CRandom( 100.0f ), G_CRandom( 100.0f ), 200.0f + G_Random( 200.0f ) );
				item->edict->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
				item->targetname = targetname;
				item->targetname += "_item";
				item->SetTargetName( item->targetname );
			}
		}
	}
}

//--------------------------------------------------------------
//
// Name:			WeaponEffectsAndSound
// Class:			Sentient
//
// Description:		Plays a weapon specified tiki effect at pos.
//					Also plays a sound.  All this comes from the gameplay database
//
// Parameters:		Entity *weapon -- The weapon to use (only need Entity portion)
//					const str& objname -- Object name to reference
//					Vector pos -- location for the tiki effect
//
// Returns:			None
//
//--------------------------------------------------------------
void Sentient::WeaponEffectsAndSound( Entity *weapon, const str& objname, Vector pos )
{
	if ( !weapon )
		return;

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	str scopestr = weapon->getArchetype() + "." + objname;
	str effect = gpm->getStringValue(scopestr, "tikifx");
	if ( effect.length() )
		Entity::SpawnEffect(effect, pos, vec_zero, 2.0f);

	str snd = gpm->getStringValue(scopestr, "wav");
	if ( snd.length() )
	{
		float minpitch = gpm->getFloatValue(scopestr, "minpitch");
		float maxpitch = gpm->getFloatValue(scopestr, "maxpitch");
		float pitch = G_Random(maxpitch - minpitch) + minpitch;
		int channel = CHAN_BODY;
		float volume = -1.0f;
		float mindist = -1.0f;
		if ( gpm->hasProperty(scopestr,"channel") )
			channel = (int)gpm->getFloatValue(scopestr, "channel");
		if ( gpm->hasProperty(scopestr,"volume") )
			volume = (int)gpm->getFloatValue(scopestr, "volume");
		if ( gpm->hasProperty(scopestr,"mindist") )
			mindist = (int)gpm->getFloatValue(scopestr, "mindist");
		weapon->Sound(snd, channel, volume, mindist, NULL, pitch);
	}
}

void Sentient::SetArmorActiveStatus( Event *ev )
{
	bool status = ev->GetBoolean( 1 );

	SetArmorActiveStatus( status );
}

void Sentient::SetArmorActiveStatus(bool status)
{
	if ( currentBaseArmor )
	{
		Event* statusEvent;

		statusEvent = new Event ( EV_Armor_SetActiveStatus );
		statusEvent->AddInteger( status );
		currentBaseArmor->ProcessEvent ( statusEvent );
	}
}

void Sentient::SetArmorMultiplier( Event *ev )
{
	if ( currentBaseArmor )
	{
		float multiplier = ev->GetFloat( 1 );
		SetArmorMultiplier( multiplier );
	}
}

void Sentient::SetArmorMultiplier( float multiplier )
{
	Event* multiplierEvent;
	multiplierEvent = new Event ( EV_Armor_SetMultiplier );

	multiplierEvent->AddFloat( multiplier );

	if ( currentBaseArmor )
		currentBaseArmor->ProcessEvent ( multiplierEvent );
}


void Sentient::AddToMyArmor( Event *ev )
{
	AddToMyArmor( ev->GetFloat( 1 ) );
}

void Sentient::AddToMyArmor( float amountToAdd )
{
	if ( currentBaseArmor )
		currentBaseArmor->setAmount( currentBaseArmor->getAmount() + amountToAdd );
}

void Sentient::SetMyArmorAmount( Event *ev )
{
	SetMyArmorAmount( ev->GetFloat(1) );
}

void Sentient::SetMyArmorAmount( float amount )
{
	if ( currentBaseArmor )
		currentBaseArmor->setAmount( amount );

}

void Sentient::ArmorEvent( Event *ev )
{
	if ( !currentBaseArmor )
		return;
	
	Event *e;
	e = new Event( ev->GetToken( 1 ) );
	
	for( int i=2; i<=ev->NumArgs(); i++ )
		e->AddToken( ev->GetToken( i ) );
	
	currentBaseArmor->ProcessEvent( e );
}

void Sentient::SetHateModifier( Event *ev )
{
	float modifier;
	modifier = ev->GetFloat( 1 );
	SetHateModifier ( modifier );
}

void Sentient::SetHateModifier( float modifier )
{
	_hateModifier = modifier;
}

float Sentient::GetHateModifier()
{
	return _hateModifier;
}

void Sentient::cacheStateMachineAnims( Event *ev )
{
	G_CacheStateMachineAnims( this, ev->GetString( 1 ) );
}

void Sentient::freeConditionals( Container<Conditional *> &conditionalsToDelete )
{
	int i;
	Conditional *conditional;


	for ( i = conditionalsToDelete.NumObjects() ; i > 0 ; i-- )
	{
		conditional = conditionalsToDelete.ObjectAt( i );

		if ( conditional )
		{
			delete conditional;
		}
	}

	conditionalsToDelete.FreeObjectList();
}
