//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/weaputils.cpp                             $
// $Revision:: 136                                                            $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// General Weapon Utility Functions

#include "_pch_cpp.h"
//#include "g_local.h"
#include "weaputils.h"
#include "specialfx.h"
#include "sentient.h"
#include "actor.h"
#include "decals.h"
#include "weapon.h"
#include "player.h"
#include "mp_manager.hpp"
#include <qcommon/gameplaymanager.h>

void FlashPlayers
(
	const Vector   &org,
	float    r,
	float    g,
	float    b,
	float    a,
	float    minradius,
	float    radius,
	float	minTime,
	float    time,
	int      type
);

qboolean MeleeAttack
(
	const Vector &pos,
	const Vector &original_end,
	float damage,
	Entity *attacker,
	meansOfDeath_t means_of_death,
	float attack_width,
	float attack_min_height,
	float attack_max_height,
	float knockback,
	qboolean hit_dead,
	Container<EntityPtr>*victimlist,
	Weapon *weapon,
	bool	critical
)
{
	trace_t trace;
	Entity *victim;
	Vector dir;
	float world_dist;
	Vector new_pos;
	Entity *skip_ent;
	qboolean hit_something = false;
	Vector mins;
	Vector maxs;
	Container<Entity *> potential_victimlist;
	int i;
	int num_traces;
	Vector end;
	
	end = original_end;
	
	/* if ( attack_width == 0 )
	attack_width = 15;
	if ( attack_min_height == 0 )
	attack_min_height = -15;
	if ( attack_max_height == 0 )
	attack_max_height = 15; */

	if ( attacker && attacker->isSubclassOf( Player ) && ( means_of_death != MOD_FIRE ) && ( means_of_death != MOD_ON_FIRE ) )
	{
		Player *player = (Player *)attacker;
		player->shotFired();
	}
	
	// See how far the world is away
	
	dir = end - pos;
	world_dist = dir.length();
	
	new_pos = pos;
	
	skip_ent = attacker;
	
	num_traces = 0;
	
	while( new_pos != end )
	{
		trace = G_Trace( pos, vec_zero, vec_zero, end, skip_ent, MASK_SOLID, false, "MeleeAttack - World test" );
		
		num_traces++;
		
		if ( trace.fraction < 1.0f )
		{
			if ( attacker->isSubclassOf( Actor ) )
			{
				Actor *act = (Actor*)attacker;
				
				if ( trace.entityNum == ENTITYNUM_WORLD )
					act->SetActorFlag ( "meleehitworld" , true );
				else
					act->SetActorFlag ( "meleehitworld" , false );
			}			
			
			if ( ( trace.entityNum == ENTITYNUM_WORLD ) || ( trace.ent && trace.ent->entity && !trace.ent->entity->takedamage ) )
			{
				dir = trace.endpos - pos;
				world_dist = dir.length();
				break;
			}
			else
			{
				// Make sure we don't go backwards any in our trace
				if ( ( Vector( new_pos - pos ).length() + 0.001f ) >= Vector( trace.endpos - pos ).length() )
					break;
				
				if ( num_traces > 10 )
				{
					// We have done too many traces, stop here
					dir = trace.endpos - pos;
					world_dist = dir.length();
					break;
				}
				
				new_pos = trace.endpos;
				
				if ( trace.ent )
					skip_ent = trace.ent->entity;
			}
		}
		else
		{
			break;
		}
	}
	
	// Find things hit
	
	dir = end - pos;
	dir.normalize();
	end = pos + ( dir * world_dist );
	
	victim = NULL;
	
	mins = Vector( -attack_width, -attack_width, attack_min_height );
	maxs = Vector( attack_width, attack_width, attack_max_height );
	
	if ( g_showbullettrace->integer )
		{
		G_DebugBBox(pos, mins, maxs, 1.0f, 0.0f, 0.0f, 1.0f );
		}

	G_TraceEntities( pos, mins, maxs, end, &potential_victimlist, MASK_MELEE );
	
	for( i = 1 ; i <= potential_victimlist.NumObjects() ; i++ )
	{
		victim = potential_victimlist.ObjectAt( i );
		
		if ( victimlist && victimlist->ObjectInList(victim) )
			continue;
		
		if ( victim && victim->takedamage && ( victim != attacker ) )
		{
			dir = end - pos;
			dir.normalize();
			
			if ( dir == vec_zero )
			{
				dir = victim->centroid - pos;
				dir.normalize();
			}
			
			if ( victim->isSubclassOf( Sentient ) && ( victim->health > 0.0f ) )
				hit_something = true;
			
			
			if ( ( victim->health > 0 ) || hit_dead )
			{
				if ( victimlist && victim->isSubclassOf( Sentient ) && ( victim->health > 0 ) )
					victimlist->AddObject( victim );
				
				// Override damage if we're using the GameplayManager system
				//	 Comes from From --
				//		Actor::MeleeAttack
				//		Player::AdvancedMeleeAttack
				//		
				GameplayManager *gpm = GameplayManager::getTheGameplayManager();
				if ( weapon && gpm->hasObject(weapon->getArchetype()) )
				{
					str attacktype = "";
					if ( attacker->isSubclassOf( Player ) )
					{
						Player *player = (Player*)attacker;
						attacktype = player->getAttackType();
					}
					GameplayFormulaData fd(attacker, 0, weapon, attacktype);
					switch ( weapon->GetCurMode() )
					{
					case FIRE_MODE1: 
						if ( gpm->hasFormula("DamageMode1") )
							damage = gpm->calculate("DamageMode1", fd);
						break;
					case FIRE_MODE2:
						if ( gpm->hasFormula("DamageMode2") )
							damage = gpm->calculate("DamageMode2", fd);
						break;
					}
					if ( critical && !victim->deadflag )
					{
						damage *= 2.0f ;
						str snd = gpm->getStringValue( "Criticals.Use", "wav");
						if ( snd.length() )
						{
							int channel = CHAN_BODY;
							float volume = -1.0f;
							float mindist = -1.0f;
							if ( gpm->hasProperty( "Criticals.Use","channel") )
								channel = (int)gpm->getFloatValue( "Criticals.Use", "channel");
							if ( gpm->hasProperty( "Criticals.Use","volume") )
								volume = (int)gpm->getFloatValue( "Criticals.Use", "volume");
							if ( gpm->hasProperty( "Criticals.Use","mindist") )
								mindist = (int)gpm->getFloatValue( "Criticals.Use", "mindist");
							weapon->Sound(snd, channel, volume, mindist);
						}
						
						str effect = gpm->getStringValue("Criticals.Use", "tikifx");
						if ( effect.length() )
						{
							Vector pos, pos2, fxpos;
							weapon->GetActorMuzzlePosition(&pos, NULL, NULL, NULL, "tag_swipe1");
							weapon->GetActorMuzzlePosition(&pos2, NULL, NULL, NULL, "tag_swipe2");
							fxpos = (pos + pos2) / 2.0f; // Spark is halfway between the two points
							weapon->SpawnEffect(effect, fxpos, Vector(0.0f,0.0f,0.0f), 2.0f); 
						}
					}
					
					if ( gpm->hasFormula("Knockback") )
						knockback = gpm->calculate("Knockback", fd);
				}
				
				Vector painDir;

				painDir = victim->centroid - pos;
				painDir.normalize();
				
				victim->Damage( attacker, attacker, damage, pos, painDir, vec_zero, knockback, 0, means_of_death, -1, -1, weapon );
			}
		}
	}
	
	if ( hit_something && attacker && attacker->isSubclassOf( Player ) && ( means_of_death != MOD_FIRE ) && ( means_of_death != MOD_ON_FIRE ) )
	{
		Player *player = (Player *)attacker;
		player->shotHit();
	}

	return hit_something;
}

Event EV_Projectile_Speed
(
	"speed",
	EV_DEFAULT,
	"f",
	"projectileSpeed",
	"set the speed of the projectile"
);
Event EV_Projectile_MinSpeed
(
	"minspeed",
	EV_TIKIONLY,
	"f",
	"minspeed",
	"set the minimum speed of the projectile (this is for charge up weapons)"
);
Event EV_Projectile_ChargeSpeed
(
	"chargespeed",
	EV_TIKIONLY,
	NULL,
	NULL,
	"set the projectile's speed to be determined by the charge time"
);
Event EV_Projectile_Damage
(
	"hitdamage",
	EV_TIKIONLY,
	"f",
	"projectileHitDamage",
	"set the damage a projectile does when it hits something"
);
Event EV_Projectile_Life
(
	"life",
	EV_DEFAULT,
	"f",
	"projectileLife",
	"set the life of the projectile"
);
Event EV_Projectile_MinLife
(
	"minlife",
	EV_TIKIONLY,
	"f",
	"minProjectileLife",
	"set the minimum life of the projectile (this is for charge up weapons)"
);
Event EV_Projectile_ChargeLife
(
	"chargelife",
	EV_TIKIONLY,
	NULL,
	NULL,
	"set the projectile's life to be determined by the charge time"
);
Event EV_Projectile_Knockback
(
	"knockback",
	EV_TIKIONLY,
	"f",
	"projectileKnockback",
	"set the knockback of the projectile when it hits something"
);
Event EV_Projectile_DLight
(
	"dlight",
	EV_TIKIONLY,
	"ffff",
	"color_red color_green color_blue intensity",
	"set the color and intensity of the dynamic light on the projectile"
);
Event EV_Projectile_Avelocity
(
	"avelocity",
	EV_TIKIONLY,
	"SFSFSF",
	"[random|crandom] yaw [random|crandom] pitch [random|crandom] roll",
	"set the angular velocity of the projectile"
);
Event EV_Projectile_MeansOfDeath
(
	"meansofdeath",
	EV_TIKIONLY,
	"s",
	"meansOfDeath",
	"set the meansOfDeath of the projectile"
);
Event EV_Projectile_BeamCommand
(
	"beam",
	EV_TIKIONLY,
	"sSSSSSS",
	"command arg1 arg2 arg3 arg4 arg5 arg6",
	"send a command to the beam of this projectile"
);
Event EV_Projectile_UpdateBeam
(
	"updatebeam",
	EV_CODEONLY,
	NULL,
	NULL,
	"Update the attached beam"
);
Event EV_Projectile_SetQuietExpire
(
	"setquietexpire",
	EV_TIKIONLY,
	NULL,
	NULL,
	"If projectile times out without dying, make it not play explosion tiki"
);
Event EV_Projectile_BounceFactor
(
	"bouncefactor",
	EV_TIKIONLY,
	"f",
	"bounceFactor",
	"set the amount of velocity retained when a projectile bounces"
);
Event EV_Projectile_BounceTouch
(
	"bouncetouch",
	EV_TIKIONLY,
	"s",
	"bouncetype",
	"Make the projectile bounce when it hits a non-damageable solid"
);
Event EV_Projectile_BounceSound
(
	"bouncesound",
	EV_DEFAULT,
	NULL,
	NULL,
	"Set the name of the sound that is played when the projectile bounces"
);
Event EV_Projectile_Explode
(
	"explode",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make the projectile explode"
);
Event EV_Projectile_ImpactMarkShader
(
	"impactmarkshader",
	EV_TIKIONLY,
	"s",
	"shader",
	"Set the impact mark of the shader"
);
Event EV_Projectile_ImpactMarkRadius
(
	"impactmarkradius",
	EV_TIKIONLY,
	"f",
	"radius",
	"Set the radius of the impact mark"
);
Event EV_Projectile_ImpactMarkOrientation
(
	"impactmarkorientation",
	EV_TIKIONLY,
	"f",
	"degrees",
	"Set the orientation of the impact mark"
);
Event EV_Projectile_SetExplosionModel
(
	"explosionmodel",
	EV_TIKIONLY,
	"s",
	"modelname",
	"Set the modelname of the explosion to be spawned"
);
Event EV_Projectile_SetAddVelocity
(
	"addvelocity",
	EV_TIKIONLY,
	"fff",
	"velocity_x velocity_y velocity_z",
	"Set a velocity to be added to the projectile when it is created"
);
Event EV_Projectile_AddOwnerVelocity
(
	"addownervelocity",
	EV_TIKIONLY,
	"b",
	"bool",
	"Set whether or not the owner's velocity is added to the projectile's velocity"
);
Event EV_Projectile_HeatSeek
(
	"heatseek",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Make the projectile heat seek"
);
Event EV_Projectile_Drunk
(
	"drunk",
	EV_TIKIONLY,
	"BF",
	"drunkFlag angleModifier",
	"Make the projectile drunk"
);
Event EV_Projectile_SetCanHitOwner
(
	"canhitowner",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Make the projectile be able to hit its owner"
);
Event EV_Projectile_ClearOwner
(
	"clearowner",
	EV_CODEONLY,
	NULL,
	NULL,
	"Make the projectile be able to hit its owner now"
);
Event EV_Projectile_RemoveWhenStopped
(
	"removewhenstopped",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Make the projectile get removed when it stops"
);
Event EV_Projectile_SetOwnerControl
(
	"ownercontrol",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Make the projectile's angles controlled by the player"
);
Event EV_Projectile_SetOwnerControlLaser
(
	"ownerControlLaser",
	EV_TIKIONLY,
	"B",
	"bool",
	"Make's the projectile's angles controlled by the player (laser method)"
);
Event EV_Projectile_SetControlTurnSpeed
(
	"controlTurnSpeed",
	EV_TIKIONLY,
	"f",
	"controlTurnSpeed",
	"Sets the max turn speed of a controlled projectile"
);
Event EV_Projectile_StickOnTouch
(
	"stickontouch",
	EV_TIKIONLY,
	"B",
	"stick_on_touch",
	"sets whether or not the projectile sticks on contact" 
);
Event EV_Projectile_TriggerDetonate
(
	"triggerdetonate",
	EV_TIKIONLY,
	"B",
	"bool",
	"Make the projectile explode when triggered."
);
Event EV_Projectile_AngleThink
(
	"anglethink",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Make the projectile update it's angles in flight"
);
Event EV_Projectile_ScaleByCharge
(
	"scaleByCharge",
	EV_TIKIONLY,
	"ff",
	"minScale maxScale",
	"Makes the projectile size scaled by the charge."
);
Event EV_Projectile_ScaleExplosion
(
	"scaleExplosion",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Make the explosion scale with the projectile."
);

Event EV_Projectile_NotifyActors
(
	"notifyactors",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Notify's active actors when close"
);
Event EV_Projectile_NotShootable
(
	"proj_NotShootable",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Specifies that this projectile is not shootable and can't hit shootable only stuff."
);
Event EV_Projectile_HitsProjectiles
(
	"proj_HitsProjectiles",
	EV_TIKIONLY,
	"b",
	"hitsProjectilesBool",
	"Specifies whether or not this projectile will hit other projectiles."
);
Event EV_Projectile_MinOnGroundTime
(
	"minOnGroundTime",
	EV_TIKIONLY,
	"f",
	"minOnGroundTime",
	"Specifies how long the projectile has to stay on the ground until it changes it's animation."
);

CLASS_DECLARATION( Entity, Projectile, NULL )
{
	{ &EV_Touch,								&Projectile::Touch						},
	{ &EV_Projectile_Speed,						&Projectile::SetSpeed					},
	{ &EV_Projectile_MinSpeed,					&Projectile::SetMinSpeed					},
	{ &EV_Projectile_ChargeSpeed,				&Projectile::SetChargeSpeed				},
	{ &EV_Projectile_Damage,					&Projectile::SetDamage					},
	{ &EV_Projectile_Life,						&Projectile::SetLife						},
	{ &EV_Projectile_MinLife,					&Projectile::SetMinLife					},
	{ &EV_Projectile_ChargeLife,				&Projectile::SetChargeLife				},
	{ &EV_Projectile_Knockback,					&Projectile::SetKnockback				},
	{ &EV_Projectile_DLight,					&Projectile::SetDLight					},
	{ &EV_Projectile_Avelocity,					&Projectile::SetAvelocity				},
	{ &EV_Projectile_MeansOfDeath,				&Projectile::SetMeansOfDeath				},
	{ &EV_Projectile_SetQuietExpire,			&Projectile::SetQuietExpire				},
	{ &EV_Projectile_BounceFactor,				&Projectile::SetBounceFactor				},
	{ &EV_Projectile_BounceTouch,				&Projectile::SetBounceTouch				},
	{ &EV_Projectile_BounceSound,				&Projectile::SetBounceSound				},
	{ &EV_Projectile_BeamCommand,				&Projectile::BeamCommand					},
	{ &EV_Projectile_UpdateBeam,				&Projectile::UpdateBeam					},
	{ &EV_Projectile_Explode,					&Projectile::Explode						},
	{ &EV_Projectile_ImpactMarkShader,			&Projectile::SetImpactMarkShader			},
	{ &EV_Projectile_ImpactMarkRadius,			&Projectile::SetImpactMarkRadius			},
	{ &EV_Projectile_ImpactMarkOrientation,		&Projectile::SetImpactMarkOrientation	},
	{ &EV_Projectile_SetExplosionModel,			&Projectile::SetExplosionModel			},
	{ &EV_Projectile_SetAddVelocity,			&Projectile::SetAddVelocity				},
	{ &EV_Projectile_AddOwnerVelocity,			&Projectile::AddOwnerVelocity			},
	{ &EV_Projectile_HeatSeek,					&Projectile::HeatSeek					},
	{ &EV_Projectile_Drunk,						&Projectile::Drunk						},
	{ &EV_Projectile_SetCanHitOwner,			&Projectile::SetCanHitOwner				},
	{ &EV_Projectile_ClearOwner,				&Projectile::ClearOwner					},
	{ &EV_Projectile_RemoveWhenStopped,			&Projectile::RemoveWhenStopped			},
	{ &EV_Killed,								&Projectile::destroyed					},
	{ &EV_Stop,									&Projectile::Stopped						},
	{ &EV_Projectile_SetOwnerControl,			&Projectile::SetOwnerControl				},
	{ &EV_Projectile_SetOwnerControlLaser,		&Projectile::setOwnerControlLaser		},
	{ &EV_Projectile_SetControlTurnSpeed,		&Projectile::setControlTurnSpeed			},
	{ &EV_Projectile_StickOnTouch,				&Projectile::StickOnTouch				},
	{ &EV_Projectile_TriggerDetonate,			&Projectile::TriggerDetonate				},
	{ &EV_Projectile_AngleThink,				&Projectile::AngleThink					},
	{ &EV_Projectile_ScaleByCharge,				&Projectile::setScaleByCharge			},
	{ &EV_Projectile_ScaleExplosion,			&Projectile::setScaleExplosion			},
	{ &EV_Projectile_NotifyActors,				&Projectile::setNotifyActors				},
	{ &EV_Projectile_NotShootable,				&Projectile::setNotShootable				},
	{ &EV_Projectile_HitsProjectiles,			&Projectile::setHitsProjectiles			},
	{ &EV_Projectile_MinOnGroundTime,			&Projectile::setMinOnGroundTime			},
	{ NULL, NULL }
};

//--------------------------------------------------------------
// Name:		Projectile()
// Class:		Projectile
//
// Description:	Constructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
Projectile::Projectile()
{
	animate = new Animate( this );
	
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	m_beam						= NULL;
	speed						= 0;
	minspeed					= 0;
	bouncefactor				= 0;
	damage						= 0;
	life						= 5;
	knockback					= 0;
	dlight_radius				= 0;
	dlight_color				= Vector( 1.0f, 1.0f, 1.0f );
	avelocity					= Vector( 0.0f, 0.0f, 0.0f );
	mins						= Vector( -1.0f, -1.0f, -1.0f );
	maxs						= Vector( 1.0f, 1.0f, 1.0f );
	meansofdeath				= (meansOfDeath_t )0;
	projFlags					= 0;
	gravity						= 0;
	impactmarkradius			= 10;
	charge_fraction				= 1.0f;
	target						= NULL;
	addownervelocity			= true;
	drunk						= false;
	can_hit_owner				= false;
	remove_when_stopped			= false;
	stick_on_touch				= false;
	takedamage					= DAMAGE_NO;
	ownercontrol				= false;
	_ownerControlLaser			= false;
	_ownerControlUsed			= false;
	_controlTurnSpeed			= 20.0f;
	firstTimeOwnerControl		= true;
	triggerdetonate				= false;
	drunkAngleModifier			= 1.0;
	_notifyActors				= false;
	
	_scaleByCharge = false;
	_scaleExplosion = false;
	
	_minScaleFromCharge = 1.0f;
	_maxScaleFromCharge = 1.0f;
	
	// make this shootable but non-solid on the client
	setContents( CONTENTS_SHOOTABLE_ONLY );
	setFullTrace( true );
	//
	// touch triggers by default
	//
	flags |= FL_TOUCH_TRIGGERS;
	
	startTime = level.time;

	_notShootable = false;
	_hitsProjectiles = true;

	_minOnGroundTime = 0.0f;
	_onGround = false;
	_startOnGroundTime = 0.0f;

	minlife = 0.0f;

	_heatSeek = false;

	_damagedSomething = false;
}

//--------------------------------------------------------------
// Name:		~Projectile()
// Class:		Projectile
//
// Description:	Destructor
//
// Parameters:	None
//
// Returns:		None
//--------------------------------------------------------------
Projectile::~Projectile()
{
	detach();
}

float Projectile::ResolveMinimumDistance( Entity *potential_target, float currmin )
{
	float		currdist;
	float		dot;
	Vector	angle;
	Vector	delta;
	Vector	norm;
	float		sine = 0.4f;
	
	delta = potential_target->centroid - this->origin;
	
	norm = delta;
	norm.normalize();
	
	// Test if the target is in front of the missile
	dot = norm * orientation[ 0 ];
	if ( dot < 0.0f )
	{
		return currmin;
	}
	
	// Test if we're within the rocket's viewcone (45 degree cone)
	dot = norm * orientation[ 1 ];
	if ( fabs( dot ) > sine )
	{
		return currmin;
	}
	
	dot = norm * orientation[ 2 ];
	if ( fabs( dot ) > sine )
	{
		return currmin;
	}
	
	currdist = delta.length();
	if ( currdist < currmin )
	{
		currmin = currdist;
		target = potential_target;
	}
	
	return currmin;
}

float Projectile::AdjustAngle( float maxadjust, float currangle, float targetangle )
{
	float dangle;
	float magangle;
	
	dangle = currangle - targetangle;
	
	if ( dangle )
	{
		magangle = ( float )fabs( dangle );
		
		while( magangle >= 360.0f )
		{
			magangle -= 360.0f;
		}
		
		if ( magangle < maxadjust )
		{
			currangle = targetangle;
		}
		else
		{
			if ( magangle > 180.0f  )
			{
				maxadjust = -maxadjust;
			}
			if ( dangle > 0 )
			{
				maxadjust = -maxadjust;
			}
			currangle += maxadjust;
		}
	}
	
	while( currangle >= 360.0f )
	{
		currangle -= 360.0f;
	}
	
	while( currangle < 0.0f )
	{
		currangle += 360.0f;
	}
	
	return currangle;
}

void Projectile::Drunk( Event *ev )
{
	if ( ev->NumArgs() > 0 )
		drunk = ev->GetBoolean( 1 );
	else
		drunk = true;

	if ( ev->NumArgs() > 1 )
		drunkAngleModifier = ev->GetFloat( 2 );
	
	if ( drunk )
	{
		turnThinkOn();
	}
}

void Projectile::TriggerDetonate( Event *ev )
{
	if( ev->NumArgs() > 0 )
		triggerdetonate = ev->GetBoolean(1);
	else
		triggerdetonate = true;
	
	if ( triggerdetonate )
	{
		turnThinkOn();
	}
}

void Projectile::SetOwnerControl( Event *ev )
{
	ownercontrol = ev->GetBoolean( 1 );
	
	if ( ownercontrol )
	{
		turnThinkOn();
	}
}

void Projectile::setOwnerControlLaser( Event *ev )
{
	if ( ev->NumArgs() > 0 )
	{
		_ownerControlLaser = ev->GetBoolean( 1 );
	}
	else
	{
		_ownerControlLaser = true;
	}
}

void Projectile::setControlTurnSpeed( Event *ev )
{
	_controlTurnSpeed = ev->GetFloat( 1 );
}

void Projectile::HeatSeek( Event *ev )
{
	float		mindist;
	Entity	*ent;
	trace_t  trace;
	Vector	delta;
	Vector	v;
	int      n;
	int      i;

	_heatSeek = true;
	
	if ( ( !target ) || ( target == world ) )
	{
		mindist = 8192.0f;
		
		n = SentientList.NumObjects();
		for( i = 1; i <= n; i++ )
		{
			ent = SentientList.ObjectAt( i );
			if ( ent->entnum == owner )
			{
				continue;
			}
			
			if ( ( ( ent->takedamage != DAMAGE_AIM ) || ( ent->health <= 0.0f ) ) && !( edict->svflags & SVF_MONSTER ) )
			{
				continue;
			}
			
			trace = G_Trace( this->origin, vec_zero, vec_zero, ent->centroid, this, MASK_SHOT, false, "DrunkMissile::HeatSeek" );
			if ( ( trace.fraction != 1.0f ) && ( trace.ent != ent->edict ) )
			{
				continue;
			}
			
			mindist = ResolveMinimumDistance( ent, mindist );
		}
	}
	else
	{
		float angspeed;
		
		delta = target->centroid - this->origin;
		v = delta.toAngles();
		
		angspeed = 5.0f;
		angles.x = AdjustAngle( angspeed, angles.x, v.x );
		angles.y = AdjustAngle( angspeed, angles.y, v.y );
		angles.z = AdjustAngle( angspeed, angles.z, v.z );

		setAngles( angles );
		velocity = Vector( orientation[ 0 ] ) * speed;
	}
	PostEvent( EV_Projectile_HeatSeek, 0.1f );
	//turnThinkOn();
}

void Projectile::AngleThink( Event *ev )
{
	turnThinkOn();
}

void Projectile::NotifyActors()
{
	Actor *act;
	Vector selfToActor;
	float distanceToActor;
	Event *projCloseEvent;
   //First loop through the awake list
	for( int i = 1; i <= ActiveList.NumObjects(); i++ )
		{
		act = ActiveList.ObjectAt( i );
		selfToActor = act->origin - origin;
		distanceToActor = selfToActor.length();
		if ( distanceToActor < 192 )
			{
			projCloseEvent = new Event(EV_Actor_ProjectileClose);
			projCloseEvent->AddEntity( G_GetEntity(owner) );
			act->ProcessEvent(projCloseEvent);
			}
		}

}

void Projectile::Think( void )
{
	Vector end;
	Vector f, r, u;
	Vector delta;
	Entity *own;
	Player *player;


	if ( _notifyActors )
	{
		NotifyActors();
	}
	
	if ( drunk )
	{
		angles += Vector( G_CRandom( drunkAngleModifier ), G_CRandom( drunkAngleModifier ), 0.0f );
		velocity = Vector( orientation[ 0 ] ) * speed;
	}

	if ( !_onGround )
	{
		setAngles( velocity.toAngles() );
	}

	// Determine if the projectile is on the ground (if needed)

	if ( ( _minOnGroundTime > 0.0f ) && !_onGround )
	{
		trace_t trace = G_Trace( origin, vec_zero, vec_zero, origin - Vector( 0, 0, maxs[2] - mins[2] ) , this, MASK_PROJECTILE, false, "Projectile::Think" );
			
		if ( trace.ent && trace.ent->entity && trace.ent->entity == world )
		{
			// Projectile is on the ground

			if ( _startOnGroundTime == 0.0f )
			{
				_startOnGroundTime = level.time;
			}

			if ( _startOnGroundTime + _minOnGroundTime < level.time )
			{
				_onGround = true;

				if ( animate && animate->HasAnim( "onground" ) )
				{
					animate->RandomAnimate( "onground" );
				}

				velocity = vec_zero;
			}
		}
		else
		{
			_startOnGroundTime = 0.0f;
		}
	}
	
	if ( this->owner != ENTITYNUM_NONE )
	{
		own = G_GetEntity(owner);
		if ( own && own->isSubclassOf( Player ) )
		{
			player = (Player *)own;
			
			if ( ownercontrol )
			{
				if ( firstTimeOwnerControl && !_ownerControlLaser )
				{
					setAngles( player->GetVAngles() );
					prevPlayerAngles = player->GetVAngles();
					firstTimeOwnerControl = false;
					//towcam = new Camera;
					//towcam->setOrigin( origin );
					//towcam->setAngles( player->GetVAngles() );
					//towcam->FollowEntity((Entity *)this, 25.0, MASK_SOLID, (Entity *)this);
					//player->SetCamera(towcam, 0);
				}
				else
				{
					if ( _ownerControlLaser )
					{
						Vector pos;
						Vector dir;
						Vector forward;
						Weapon *weapon;
						Vector goalAngles;
						
						weapon = player->GetActiveWeapon( WEAPON_DUAL );
						
						if ( weapon && weapon->getControllingProjectile() )
						{
							if ( !_ownerControlUsed )
							{
								_ownerControlUsed = true;

								if ( animate && animate->HasAnim( "controlled" ) )
								{
									animate->RandomAnimate( "controlled" );
								}
							}

							weapon->GetMuzzlePosition( &pos, &forward, NULL, NULL );
							
							trace_t trace = G_Trace( pos, vec_zero, vec_zero, pos + 2000 * forward, NULL, MASK_SOLID, false, "Projectile::Think" );
							
							dir = trace.endpos - origin;
							goalAngles = dir.toAngles();
							
							delta = goalAngles - angles;
							
							delta.EulerNormalize();

							// Don't let the projectile try to turn all of the way around

							if ( ( delta[ YAW ]   < -135.0f ) || ( delta[ YAW ]   > 135.0f ) ||
								 ( delta[ PITCH ] < -135.0f ) || ( delta[ PITCH ] > 135.0f ) )
							{
								delta[ YAW ] = 0.0f;
								delta[ PITCH ] = 0.0f;
							}

							delta.EulerNormalize();

							if ( delta[ YAW ] < -_controlTurnSpeed )
								delta[ YAW ] = -_controlTurnSpeed;
							if ( delta[ YAW ] > _controlTurnSpeed )
								delta[ YAW ] = _controlTurnSpeed;
							if ( delta[ PITCH ] < -_controlTurnSpeed )
								delta[ PITCH ] = -_controlTurnSpeed;
							if ( delta[ PITCH ] > _controlTurnSpeed )
								delta[ PITCH ] = _controlTurnSpeed;
							
							setAngles( angles + delta );
						}
					}
					else
					{
						delta = (player->GetVAngles() - prevPlayerAngles);
						prevPlayerAngles = player->GetVAngles();
						
						if ( player->isButtonDown( BUTTON_ATTACKRIGHT ) )
						{
							setAngles( angles + delta );
						}
					}
				}
				AngleVectors( angles, f, r, u );
				orientation[0][0] = f.x; orientation[0][1] = f.y; orientation[0][2] = f.z;
				orientation[1][0] = r.x; orientation[1][1] = r.y; orientation[1][2] = r.z;
				orientation[2][0] = u.x; orientation[2][1] = u.y; orientation[2][2] = u.z;
			}
			
			if ( triggerdetonate )
			{
				if ( ( player->getHealth() <= 0.0f ) || ( player->deadflag != DEAD_NO ) )
				{
					PostEvent( EV_Remove, 0.0f );
				}
				else if ( player->GetProjDetonate() )
				{
					Event *newEvent = new Event( EV_Player_Weapon );
					newEvent->AddString( "dual" );
					newEvent->AddString( "nextSwitchTime" );
					newEvent->AddFloat( 0.0f );
					player->ProcessEvent( newEvent );

					if ( level.time > startTime + minlife )
						ProcessEvent( EV_Projectile_Explode );
					else
						PostEvent( EV_Projectile_Explode, startTime + minlife - level.time );
				}
				
			}
		}
	}
	else
		setAngles( angles );
	
	if ( ownercontrol )
	{
		velocity = Vector( orientation[ 0 ] ) * speed;
	}
}

void Projectile::AddOwnerVelocity( Event *ev )
{
	addownervelocity = ev->GetBoolean( 1 );
}

void Projectile::SetAddVelocity( Event *ev )
{
	addvelocity.x = ev->GetFloat( 1 );
	addvelocity.y = ev->GetFloat( 2 );
	addvelocity.z = ev->GetFloat( 3 );
}

void Projectile::SetExplosionModel( Event *ev )
{
	explosionmodel = ev->GetString( 1 );

	CacheResource( explosionmodel, this );
}

void Projectile::SetImpactMarkShader( Event *ev )
{
	impactmarkshader = ev->GetString( 1 );
}

void Projectile::SetImpactMarkRadius( Event *ev )
{
	impactmarkradius = ev->GetFloat( 1 );
}

void Projectile::SetImpactMarkOrientation( Event *ev )
{
	impactmarkorientation = ev->GetString( 1 );
}

void Projectile::Explode( Event *ev )
{
	Entity *owner;
	Entity *ignoreEnt=NULL;
	
	if ( ev->NumArgs() == 1 )
		ignoreEnt = ev->GetEntity( 1 );
	
	// Get the owner of this projectile
	owner = G_GetEntity( this->owner );
	
	// If the owner's not here, make the world the owner
	if ( !owner )
		owner = world;

	takedamage = DAMAGE_NO;
	
	if (! ((level.time - startTime >= life-FRAMETIME) && (projFlags & P_QUIET_EXPIRE)) ) // if projectile didn't expire by timeout
	{											// or isn't set to quiet_expire (FRAMETIME required due to float roundoff errors)
		// Spawn an explosion model
		if ( explosionmodel.length() )
		{
			// Move the projectile back off the surface a bit so we can see
			// explosion effects.
			Vector dir, v;
			float scale;
			
			v = velocity;
			v.normalize();
			dir = v;
			//v = origin - ( v * 36 );
			
			if ( _scaleExplosion )
				scale = edict->s.scale;
			else
				scale = 1.0f;
			
			//ExplosionAttack( v, owner, explosionModleToUse, dir, ignoreEnt, scale );
			ExplosionAttack( centroid, owner, explosionmodel, dir, ignoreEnt, scale );
		}
	}
	
	CancelEventsOfType( EV_Projectile_UpdateBeam );
	
	// Kill the beam
	if ( m_beam )
	{
		m_beam->ProcessEvent( EV_Remove );
		m_beam = NULL;
	}
	
	// Remove the projectile
	
	PostEvent( EV_Remove, 0.0f );
}

//----------------------------------------------------------------
// Name:			SetQuietExpire
// Class:			Projectile
//
// Description:		sets projectile to not spawn explosionmodel if it times out
//
// Parameters:		None
//
// Returns:			void
//----------------------------------------------------------------
void Projectile::SetQuietExpire( Event *ev )
{
	projFlags |= P_QUIET_EXPIRE; // set projectile to not spawn explosionmodel if it times out
}

void Projectile::SetBounceFactor( Event *ev )
{
	bouncefactor = ev->GetFloat(1);
}

void Projectile::SetBounceTouch( Event *ev )
{
	// if arg1 is "all", make projectile bounce off *everything*
	if (ev->NumArgs() > 0)
	{
		str t = ev->GetString( 1 );
		// add more tests here if we want them (for p_bounce_actor, p_bounce_player, etc)
		if (!t.icmp("all"))
		{
			projFlags |= P_BOUNCE_ALL;
		}
	} // "bouncetouch all" should fall through and set P_BOUNCE_TOUCH and movetype anyway
	
	projFlags |= P_BOUNCE_TOUCH;   
	setMoveType( MOVETYPE_BOUNCE );
}

void Projectile::BeamCommand( Event *ev )
{
	int i;
	
	if ( !m_beam )
	{
		m_beam = new FuncBeam;
		
		m_beam->setOrigin( this->origin );
		m_beam->Ghost( NULL );
	}
	
	Event *beamev = new Event( ev->GetToken( 1 ) );
	
	for( i=2; i<=ev->NumArgs(); i++ )
	{
		beamev->AddToken( ev->GetToken( i ) );
	}
	
	m_beam->ProcessEvent( beamev );
	PostEvent( EV_Projectile_UpdateBeam, level.frametime );
}

void Projectile::UpdateBeam( Event *ev )
{
	if ( m_beam )
	{
		m_beam->setOrigin( this->origin );
		PostEvent( EV_Projectile_UpdateBeam, level.frametime );
	}
}

void Projectile::SetBounceSound( Event *ev )
{
	bouncesound = ev->GetString( 1 );
}

void Projectile::SetChargeLife( Event *ev )
{
	projFlags   |= P_CHARGE_LIFE;
}

void Projectile::SetMinLife( Event *ev )
{
	minlife  = ev->GetFloat( 1 );
	projFlags   |= P_CHARGE_LIFE;
}

void Projectile::SetLife( Event *ev )
{
	life = ev->GetFloat( 1 );
}

void Projectile::SetSpeed( Event *ev )
{
	speed = ev->GetFloat( 1 );
}

void Projectile::SetMinSpeed( Event *ev )
{
	minspeed = ev->GetFloat( 1 );
	projFlags   |= P_CHARGE_SPEED;
}

void Projectile::SetChargeSpeed( Event *ev )
{
	projFlags   |= P_CHARGE_SPEED;
}

void Projectile::SetAvelocity( Event *ev )
{
	int i=1;
	int j=0;
	str vel;
	
	if ( ev->NumArgs() < 3 )
	{
		warning( "ClientGameCommandManager::SetAngularVelocity", "Expecting at least 3 args for command randvel" );
	}
	
	while ( j < 3 )
	{
		vel = ev->GetString( i++ );
		if ( vel == "crandom" )
		{
			avelocity[j++] = ev->GetFloat( i++ ) * crandom();
		}
		else if ( vel == "random" )
		{
			avelocity[j++] = ev->GetFloat( i++ ) * random();
		}
		else
		{
			avelocity[j++] = (float)atof( vel.c_str() );
		}
	}
}

void Projectile::SetDamage( Event *ev )
{
	damage = ev->GetFloat( 1 );
}

void Projectile::SetKnockback( Event *ev )
{
	knockback = ev->GetFloat( 1 );
}

void Projectile::SetDLight( Event *ev )
{
	dlight_color[0] = ev->GetFloat( 1 );
	dlight_color[1] = ev->GetFloat( 2 );
	dlight_color[2] = ev->GetFloat( 3 );
	dlight_radius   = ev->GetFloat( 4 );
}

void Projectile::SetMeansOfDeath( Event *ev )
{
	meansofdeath = (meansOfDeath_t )MOD_NameToNum( ev->GetString( 1 ) );
}

void Projectile::DoDecal( void )
{
	if ( impactmarkshader.length() )
	{
		Decal *decal = new Decal;
		decal->setShader( impactmarkshader );
		decal->setOrigin( level.impact_trace.endpos );
		decal->setDirection( level.impact_trace.plane.normal );
		decal->setOrientation( impactmarkorientation );
		decal->setRadius( impactmarkradius );
	}
}

void Projectile::Touch( Event *ev )
{
	// Other is what got hit
	Entity   *other = ev->GetEntity( 1 );
	assert( other );
	
	// *really* bouncy projectile
	if ( projFlags & P_BOUNCE_ALL )
	{
		if (bouncefactor)
			velocity *= bouncefactor;
		return;
	}
	
	// Don't touch teleporters
	if ( other->isSubclassOf( Teleporter ) )
	{
		return;
	}
	
	// Can't hit yourself with a projectile
	if ( other->entnum == edict->ownerNum )
	{
		return;
	}

	// Remove it if we hit the sky
	if ( HitSky() )
	{
		PostEvent( EV_Remove, 0.0f );
		return;
	}

	if ( can_hit_owner )
	{
		ProcessEvent( EV_Projectile_ClearOwner );
	}
	
	// Bouncy Projectile
	if ( ( projFlags & P_BOUNCE_TOUCH ) && !other->takedamage )
		//( !other->takedamage || other->health <= 0 ) )
	{
		Actor *act = (Actor *)other;

		if ( other->isSubclassOf(Actor) )
			{
			if ( act->GetActorFlag( ACTOR_FLAG_BOUNCE_OFF ) && (act->Immune( meansofdeath ) || act->takedamage == DAMAGE_NO ))
				{
				// Remove the owner so it can be hit after the bounce
				//this->owner     = ENTITYNUM_NONE;
				//edict->ownerNum = ENTITYNUM_NONE;
				edict->ownerNum = other->entnum;
				if ( bouncesound.length() )
					Sound( bouncesound, CHAN_BODY );
				
				//Bounce The Projectile
				velocity *= act->bounce_off_velocity;

				// Tell the actor something just bounced off of it
				
				Event *event = new Event( EV_Actor_BounceOff );
				event->AddVector( origin );
				act->PostEvent( event, 0.0f );
				return;
				}
			}

		if (bouncefactor)
			velocity *= bouncefactor;

		// Play a bouncy sound
		if ( !_onGround && bouncesound.length() )
		{
			Sound( bouncesound, CHAN_BODY );
		}
		BroadcastSound();
		return;
	}
	
	if ( stick_on_touch && other->projectilesCanStickToMe )
	{
		turnThinkOff();
		Vector forward;
		angles.AngleVectors(&forward);	   
		forward.normalize();
		
		Vector endOfTrace = ( forward * 1000.0f ) + origin;
		if ( other->entnum == ENTITYNUM_WORLD )
		{
			if ( animate && animate->HasAnim( "stuck" ) )
			{
				animate->RandomAnimate( "stuck" );
			}

			trace_t trace = G_Trace( origin, vec_zero, vec_zero, endOfTrace, NULL, MASK_PROJECTILE, false, "ProjectileTouch" );
			
			if( trace.ent && trace.ent->entity && trace.ent->entity == world )
			{
				origin.x = trace.endpos[0];
				origin.y = trace.endpos[1];
				origin.z = trace.endpos[2];
				setOrigin(origin);
			}
			
			setMoveType( MOVETYPE_NONE );
			
			CancelEventsOfType( EV_Projectile_AngleThink );
			velocity = Vector(0, 0, 0);
			return;
		}
		else
		{
			if ( animate && animate->HasAnim( "stuck_body" ) )
			{
				animate->RandomAnimate( "stuck_body" );
			}

			const trace_t trace = G_FullTrace( origin, vec_zero, vec_zero, endOfTrace, this, MASK_PROJECTILE, false, "ProjectileTouch" );
			
			if( trace.ent && trace.ent->entity )
			{
				const Vector targetDimensions( Vector( trace.ent->maxs ) - Vector( trace.ent->mins ) );
				
				const Vector translation( 
					( G_Random( 0.24f ) - 0.12f ) * targetDimensions.x, // Lots of magic numbers that generally makes the  
					( G_Random( 0.24f ) - 0.12f ) * targetDimensions.y, // projectile stick in the target's torso
					( G_Random( 0.12f ) + 0.12f ) * targetDimensions.z
					); 
				
				const float yawOffset = angles.yaw() - trace.ent->entity->angles.yaw();
				const float pitchOffset = angles.pitch() - trace.ent->entity->angles.pitch();
				const Vector rotation( pitchOffset, yawOffset, 0 );
				
				if ( !attach( trace.ent->entity->entnum, -1, false, translation, rotation ) )
				{
					PostEvent( EV_Remove, 0.0f );
					
				}
			}
			
			setMoveType( MOVETYPE_NONE );
			CancelEventsOfType( EV_Projectile_AngleThink );
			velocity = Vector(0, 0, 0);
		}
	}
	else
	{
		// See if we should bounce off object
		if ( other->isSubclassOf( Actor ) )
		{
			Actor *act = (Actor *)other;
			
			if ( act->GetActorFlag( ACTOR_FLAG_BOUNCE_OFF ) && (act->Immune( meansofdeath ) || act->takedamage == DAMAGE_NO ))
			{
				// Remove the owner so it can be hit after the bounce
				//this->owner     = ENTITYNUM_NONE;
				//edict->ownerNum = ENTITYNUM_NONE;
				edict->ownerNum = other->entnum;
				if ( bouncesound.length() )
					Sound( bouncesound, CHAN_BODY );
				
				//Bounce The Projectile
				velocity *= act->bounce_off_velocity;

				// Tell the actor something just bounced off of it
				
				Event *event = new Event( EV_Actor_BounceOff );
				event->AddVector( origin );
				act->PostEvent( event, 0.0f );
				return;
			}
		}		

		// Remove the projectile
		PostEvent( EV_Remove, 0.0f );
		// Call the explosion event
		Event *explEv;
		explEv = new Event( EV_Projectile_Explode );
		explEv->AddEntity( other );
		ProcessEvent( explEv );
	}
	
	
	// Make the projectile not solid
	setSolidType( SOLID_NOT );
	setMoveType( MOVETYPE_NONE );
	//hideModel();
	
	// Do a decal
	DoDecal();
	
	// Get the owner of this projectile
	Entity   *myOwner = G_GetEntity( owner );
	
	// If the owner's not here, make the world the owner
	if ( !myOwner )
		myOwner = world;
	
	
	str realname = GetRandomAlias( "impact_world" );
	if ( other->isSubclassOf( Sentient ) )
		realname = GetRandomAlias( "impact_flesh" );
	
	if ( realname.length() > 1 )
		Sound( realname, CHAN_VOICE );
	
	
	// 
	// Override damage if we're using the GameplayManager system
	//
	
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( gpm->hasObject(getArchetype()) )
	{
		str attacktype = "";
		if ( myOwner->isSubclassOf( Player ) )
		{
			Player *player = (Player*)myOwner;
			attacktype = player->getAttackType();
		}
		GameplayFormulaData fd(myOwner, 0, this, attacktype);
		if ( gpm->hasFormula("OffensiveDamage") )
			damage = GameplayManager::getTheGameplayManager()->calculate("OffensiveDamage", fd);
		if ( gpm->hasFormula("Knockback") )
			knockback = GameplayManager::getTheGameplayManager()->calculate("Knockback", fd);
	}
	
	
	// Damage the thing that got hit
	if (other->takedamage)
	{
		if ( myOwner && myOwner->isSubclassOf( Player ) )
		{
			Player *player = (Player *)myOwner;
			
			damage = player->getDamageDone( damage, meansofdeath, false );
		}

		other->Damage( this,
			myOwner,
			damage,
			origin,
			velocity,
			level.impact_trace.plane.normal,
			knockback,
			0,
			meansofdeath,
			level.impact_trace.intersect.surface,
			level.impact_trace.intersect.bone
			);
	}
	
	BroadcastSound();
}

void Projectile::SetCanHitOwner( Event *ev )
{
	can_hit_owner = true;
}

void Projectile::ClearOwner( Event *ev )
{
	//this->owner     = ENTITYNUM_NONE;
	edict->ownerNum = ENTITYNUM_NONE;
}

void Projectile::RemoveWhenStopped( Event *ev )
{
	remove_when_stopped = true;
}

void Projectile::StickOnTouch( Event *ev )
{
	if(ev->NumArgs() > 0 )
		stick_on_touch = ev->GetBoolean(1);
	else
		stick_on_touch = true;
}

void Projectile::Stopped( Event *ev )
{
	if ( remove_when_stopped )
		PostEvent( EV_Remove, 0.0f );
}

void Projectile::setScaleByCharge( Event *ev )
{
	_scaleByCharge = true;

	_minScaleFromCharge = ev->GetFloat( 1 );
	_maxScaleFromCharge = ev->GetFloat( 2 );
}

void Projectile::setScaleExplosion( Event *ev )
{
	_scaleExplosion = true;
}

void Projectile::setNotifyActors( Event *ev )
{
	_notifyActors = ev->GetBoolean( 1 );

	if ( _notifyActors )
	{
		turnThinkOn();
	}
}

void Projectile::setNotShootable( Event *ev )
{
	_notShootable = true;
}

void Projectile::setHitsProjectiles( Event *ev )
{
	_hitsProjectiles = ev->GetBoolean( 1 );
}

Entity *Projectile::getOwner( void )
{
	return G_GetEntity( owner );
}

void Projectile::setMinOnGroundTime( Event *ev )
{
	_minOnGroundTime = ev->GetFloat( 1 );

	turnThinkOn();
}

void Projectile::didDamage( void )
{
	if ( !_damagedSomething )
	{
		Entity *owner = getOwner();

		_damagedSomething = true;

		if ( owner && owner->isSubclassOf( Player ) )
		{
			Player *player = (Player *)owner;
			
			player->shotHit();
		}
	}
}

void Projectile::destroyed( Event *ev )
{
	Entity *shooter;

	shooter = ev->GetEntity( 1 );

	if ( shooter )
	{
		owner           = shooter->entnum;
		edict->ownerNum = shooter->entnum;
	}

	ProcessEvent( EV_Projectile_Explode );
}

Event EV_Explosion_Radius
(
	"radius",
	EV_DEFAULT,
	"f",
	"projectileRadius",
	"set the radius for the explosion"
);
Event EV_Explosion_ConstantDamage
(
	"constantdamage",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Makes the explosion do constant damage over the radius"
);
Event EV_Explosion_DamageEveryFrame
(
	"damageeveryframe",
	EV_TIKIONLY,
	NULL,
	NULL,
	"Makes the explosion damage every frame"
);
Event EV_Explosion_DamageAgain
(
	"damageagain",
	EV_CODEONLY,
	NULL,
	NULL,
	"This event is generated each frame if explosion is set to damage each frame"
);
Event EV_Explosion_Flash
(
	"flash",
	EV_TIKIONLY,
	"fffffff",
	"mintime time color_red color_green color_blue minradius radius",
	"Flash player screens"
);
Event EV_Explosion_RadiusDamage
(
	"radiusdamage",
	EV_TIKIONLY,
	"f",
	"radiusDamage",
	"set the radius damage an explosion does"
);

CLASS_DECLARATION( Projectile, Explosion, NULL )
{
	{ &EV_Explosion_Radius,						&Explosion::SetRadius },
	{ &EV_Explosion_ConstantDamage,				&Explosion::SetConstantDamage },
	{ &EV_Explosion_DamageEveryFrame,			&Explosion::SetDamageEveryFrame },
	{ &EV_Explosion_DamageAgain,				&Explosion::DamageAgain },
	{ &EV_Explosion_Flash,			 	      	&Explosion::SetFlash },
	{ &EV_Explosion_RadiusDamage,		      	&Explosion::SetRadiusDamage },
	{ &EV_Explosion_RadiusDamage,		      	&Explosion::SetRadiusDamage },

	{ NULL, NULL }
};

Explosion::Explosion()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	radius               = 0;
	radius_damage			= 0.0f;
	constant_damage      = false;
	damage_every_frame   = false;
	flash_mintime           = 0;
	flash_time           = 0;
	flash_type           = 0;

	mins = Vector( -1.0f, -1.0f, -1.0f );
	maxs = Vector( 1.0f, 1.0f, 1.0f );
}

void Explosion::SetFlash( Event *ev )
{
	flash_mintime  = ev->GetFloat( 1 );
	flash_time     = ev->GetFloat( 2 );
	flash_r        = ev->GetFloat( 3 );
	flash_g        = ev->GetFloat( 4 );
	flash_b        = ev->GetFloat( 5 );
	flash_a        = ev->GetFloat( 6 );
	flash_minradius = ev->GetFloat( 7 );
	flash_radius	= ev->GetFloat( 8 );
	flash_type		= 0;
	
	if ( ev->NumArgs() > 8 )
	{
		str t = ev->GetString( 9 );
		
		if ( !t.icmp( "addblend" ) )
			flash_type = 1;
		else if ( !t.icmp( "alphablend" ) )
			flash_type = 0;
	}
}

void Explosion::SetRadius( Event *ev )
{
	radius = ev->GetFloat( 1 );
}

void Explosion::SetRadiusDamage( Event *ev )
{
	radius_damage = ev->GetFloat( 1 );
}

void Explosion::SetConstantDamage( Event *ev )
{
	constant_damage = true;
}

void Explosion::SetDamageEveryFrame( Event *ev )
{
	damage_every_frame = true;
}

void Explosion::DamageAgain( Event *ev )
{
	Entity *owner_ent;
	
	owner_ent = G_GetEntity( owner );
	RadiusDamage( this, owner_ent, radius_damage, NULL, meansofdeath, radius, knockback, constant_damage );
	
	PostEvent( EV_Explosion_DamageAgain, FRAMETIME );
}

Projectile *ProjectileAttack( const Vector &start, Vector &dir, Entity *owner, const str &projectileModel, float fraction,
		float real_speed )
{
	Event          *ev;
	Projectile     *proj=NULL;
	float          newspeed,newlife;
	SpawnArgs      args;
	Entity         *obj;
	float          dot=0;
	
	if ( !projectileModel.length() )
	{
		gi.WDPrintf( "ProjectileAttack : No model specified for ProjectileAttack" );
		return NULL;
	}
	
	args.setArg( "model", projectileModel );
	obj = args.Spawn();
	
	if ( !obj )
	{
		gi.WDPrintf( "projectile model '%s' not found\n", projectileModel.c_str() );
		return NULL;
	}
	
	if ( obj->isSubclassOf( Projectile ) )
		proj = ( Projectile * )obj;
	else
		gi.WDPrintf( "%s is not of class projectile\n", projectileModel.c_str() );
	
	if ( !proj )
		return NULL;

	if ( owner && owner->isSubclassOf( Player ) )
	{
		Player *player = (Player *)owner;
		player->shotFired();
	}
	
	// Create a new projectile entity and set it off
	proj->setModel( projectileModel );
	proj->setMoveType( MOVETYPE_BOUNCE );
	proj->CancelEventsOfType( EV_ProcessInitCommands );
	proj->ProcessInitCommands( proj->edict->s.modelindex );
	proj->owner             = owner->entnum;
	proj->edict->ownerNum   = owner->entnum;
	
	proj->angles            = dir.toAngles();
	proj->charge_fraction   = fraction;
	
	if ( proj->_scaleByCharge )
	{
		proj->edict->s.scale = proj->_minScaleFromCharge + ( proj->_maxScaleFromCharge - proj->_minScaleFromCharge ) * fraction;
		
		proj->damage *= proj->edict->s.scale;
	}
	
	if ( proj->projFlags & P_CHARGE_SPEED )
	{
		newspeed = (proj->speed - proj->minspeed ) * fraction + proj->minspeed;
	}
	else
	{
		newspeed = proj->speed;
	}
	
	if ( real_speed )
		newspeed = real_speed;
	
	if ( proj->addownervelocity )
	{
		dot = DotProduct( owner->velocity, dir );
		if ( dot < 0.0f )
			dot = 0;
	}
	
	Player *player = NULL;
	Vector newAngles = proj->angles;
	Vector newVelocity = dir * ( newspeed + dot );
	newVelocity += proj->addvelocity;
	
	if ( owner->isSubclassOf( Player ) )
	{
		player = static_cast<Player *>(owner);
		if ( player->GetTarget() && player->GetTargetEnemyLocked() )
		{
			Vector anglesToTarget;
			if ( player->GetProjectileLaunchAngles( anglesToTarget, start, newspeed, proj->gravity ) )
			{
				newAngles = anglesToTarget;
				anglesToTarget.AngleVectors( &newVelocity );
				newVelocity *= newspeed;
			}
			else
			{
				anglesToTarget = Vector::Identity();
			}
		}
		
		if ( multiplayerManager.inMultiplayer() )
			multiplayerManager.playerFired( player );
	}
	
	
	Actor *actor = NULL;
	if ( owner->isSubclassOf( Actor ) )
	{
		actor = static_cast<Actor *>(owner);
		
		if ( actor->combatSubsystem->shouldArcProjectile() )
		{
			Vector anglesToTarget;
			Entity *enemy = actor->enemyManager->GetCurrentEnemy();
			
			if ( enemy )
			{
				Vector ActorToTarget = actor->centroid - enemy->centroid;
				float distToTarget = ActorToTarget.length();
				
				if ( distToTarget > actor->combatSubsystem->GetLowArcRange() )
				{
					if ( actor->combatSubsystem->GetProjectileLaunchAngles( anglesToTarget, start, newspeed, proj->gravity , true ) )
					{
						newAngles = anglesToTarget;
						anglesToTarget.AngleVectors( &newVelocity );
						newVelocity *= newspeed;
					}
					else
					{
						anglesToTarget = Vector::Identity();
					}
				}
				else
				{
					if ( actor->combatSubsystem->GetProjectileLaunchAngles( anglesToTarget, start, newspeed, proj->gravity , false ) )
					{
						newAngles = anglesToTarget;
						anglesToTarget.AngleVectors( &newVelocity );
						newVelocity *= newspeed;
					}
					else
					{
						anglesToTarget = Vector::Identity();
					}
				}
			}			
		}
	}
	
	proj->velocity = newVelocity;
	proj->setAngles( newAngles );

	if ( proj->_notShootable )
	{
		proj->setSolidType( SOLID_BBOX );
		proj->setContents( 0 );
		proj->edict->clipmask = MASK_PROJECTILE_NOTSHOOTABLE;
	}
	else
	{
		proj->setSolidType( SOLID_BBOX );

		if ( proj->_hitsProjectiles )
			proj->edict->clipmask = MASK_PROJECTILE;
		else
			proj->edict->clipmask = MASK_PROJECTILE_NOTSHOOTABLE;
	}
	
	proj->setSize( proj->mins, proj->maxs );
	proj->setOrigin( start );
	//proj->origin.copyTo( proj->edict->s.origin2 );
	
	if ( proj->m_beam )
	{
		proj->m_beam->setOrigin( start );
		proj->m_beam->origin.copyTo( proj->m_beam->edict->s.origin2 );
	}
	
	if ( proj->dlight_radius )
	{
		G_SetConstantLight( &proj->edict->s.constantLight,
			&proj->dlight_color[0],
			&proj->dlight_color[1],
			&proj->dlight_color[2],
			&proj->dlight_radius
			);
	}
	
	// Calc the life of the projectile
	if ( proj->projFlags & P_CHARGE_LIFE )
	{
		newlife = proj->life * fraction;
		
		if ( newlife < proj->minlife )
			newlife = proj->minlife;
	}
	else
	{
		newlife = proj->life;
	}
	
	// Remove the projectile after it's life expires
	ev = new Event( EV_Projectile_Explode );
	proj->PostEvent( ev, newlife );
	
	proj->animate->RandomAnimate( "idle", EV_StopAnimating );
	
	// If can hit owner clear the owner of this projectile in a second
	
	if ( proj->can_hit_owner )
		proj->PostEvent( EV_Projectile_ClearOwner, 1.0f );
	
	// Notify any actor that is in way of projectile
	Vector end_pos;
	trace_t trace;
	Event *event;
	Entity *victim;
	
	end_pos = ( dir * 1000.0f ) + start;
	trace = G_Trace( start, vec_zero, vec_zero, end_pos, owner, MASK_PROJECTILE, false, "ProjectileAttack" );
	
	if ( trace.entityNum != ENTITYNUM_NONE )
	{
		event = new Event( EV_ActorIncomingProjectile );
		event->AddEntity( proj );
		victim = G_GetEntity( trace.entityNum );
		victim->PostEvent( event, 0.0f );
	}

	if ( owner && proj->triggerdetonate )
	{
		Event *newEvent = new Event( EV_Player_Weapon );
		newEvent->AddString( "dual" );
		newEvent->AddString( "nextSwitchTime" );
		newEvent->AddFloat( newlife );
		owner->ProcessEvent( newEvent );
	}
	
	return proj;
}

float BulletAttack(
	const Vector	&start,
	const Vector	&dir,
	const Vector   &right,
	const Vector   &up,
	float    range,
	float  	damage,
	float    knockback,
	int      dflags,
	int      meansofdeath,
	const Vector   &spread,
	int      count,
	Entity   *owner,
	Entity	*weap )
{
	Vector   end;
	int      i;
	trace_t  trace;
	Entity   *ent;
	Player   *player;
	float    damage_total = 0.0f;
	float		original_value;
	float		new_value;
	
	ent = NULL;
	
	if ( owner->isSubclassOf( Player ) )
	{
		player = (Player *)owner;
		
		meansofdeath = (int)player->changetMeansOfDeath( (meansOfDeath_t)meansofdeath );
		
		damage = player->getDamageDone( damage, meansofdeath, false );
		
		player->shotFired();
		
		if ( multiplayerManager.inMultiplayer() )
			multiplayerManager.playerFired( player );
	}
	
	for ( i=0; i<count; i++ )
	{
		end = start + ( dir * range ) +
			( right * G_CRandom() * spread.x ) +
            ( up    * G_CRandom() * spread.y );
		
		if ( !multiplayerManager.inMultiplayer() || multiplayerManager.fullCollision() )
			trace = G_FullTrace( start, vec_zero, vec_zero, end, owner, MASK_SHOT, false, "BulletAttack" );
		else
			trace = G_Trace( start, vec_zero, vec_zero, end, owner, MASK_SHOT, false, "BulletAttack" );
		
		// Do a bloodsplat for debugging
		/*Decal *decal = new Decal;
		decal->setShader( "bloodsplat.spr" );
		decal->setOrigin( Vector( trace.endpos ) + ( Vector( trace.plane.normal ) * 0.2 ) );
		decal->setDirection( trace.plane.normal );
		decal->setOrientation( "random" );
		decal->setRadius( 8 );*/
		
		if ( trace.ent )
			ent = trace.ent->entity;

		/*if ( trace.ent )
		{
			ent = trace.ent->entity;
			
			//Let Entity know a hitscan hit has been registered
			if ( ent->isSubclassOf( Actor ) )
			{
				Actor *act = (Actor *)ent;
				act->SetActorFlag( "incominghitscan" , true );
			}
			
			// See if bullet should bounce off of entity
			
			if ( ent->isSubclassOf( Actor ) )
			{
				Actor *act = (Actor *)ent;
				Vector	real_dir;
				Vector	bounce_start;
				Vector angles;
				Vector left;
				Vector up;
				
				if ( act->GetActorFlag( ACTOR_FLAG_BOUNCE_OFF ) && ( act->Immune( meansofdeath ) || act->takedamage == DAMAGE_NO ) )
				{
					// Play sound
					
					if ( owner )
						owner->Sound( "snd_ricochet", 0 );
					
					real_dir = trace.endpos - start;
					
					real_dir = real_dir * -1.0f;
					
					angles = real_dir.toAngles();
					
					angles.AngleVectors( NULL, &left, &up );
					
					real_dir.normalize();
					real_dir *= range;
					
					real_dir += ( left * G_CRandom() * 100.0f ) + ( up * G_CRandom() * 100.0f );
					
					bounce_start = trace.endpos;
					
					trace = G_Trace( bounce_start, vec_zero, vec_zero, bounce_start + real_dir, NULL, MASK_SHOT, false, "BulletAttack2" );
					
					// Add tracer
					
					//No tracer this time around...  Too many beam weapons

					//if ( G_Random( 1.0f ) < .2f )
					//{
					//	FuncBeam *beam = CreateBeam( NULL, "tracer", bounce_start, trace.endpos, 1, 1.0f, .1f, 0.f );
					//	beam->PostEvent( EV_Remove, 1.0f );
					//}
					
					if ( trace.ent )
					{
						ent = trace.ent->entity;
						
						if ( ent == world )
						{
							// Add bullet hole
							Decal *decal = new Decal;
							decal->setShader( "bullethole" );
							decal->setOrigin( trace.endpos );
							decal->setDirection( trace.plane.normal );
							decal->setOrientation( "random" );
							decal->setRadius( 2.0f );
							
							// Add smoke
							Entity *smoke = new Entity( ENTITY_CREATE_FLAG_ANIMATE );
							
							smoke->setModel( "fx_bulletsmoke.tik" );
							smoke->setOrigin( trace.endpos );
							smoke->animate->RandomAnimate( "idle" );
							smoke->PostEvent( EV_Remove, 2.0f );
						}
					}
					else
					{
						ent = NULL;
					}
				}
			}
		}
		else
		{
			ent = NULL;
		}
		*/
		if ( ent && ent->takedamage )
		{
			
			// Override damage if we're using the GameplayManager system
			//	 Comes from From --
			//		Weapon::Shoot
			//	
			/*
			GameplayManager *gpm = GameplayManager::getTheGameplayManager();
			float bulletDamage;
			
			  if ( weap && gpm->hasObject(weap->getArchetype()) )
			  {
			  str attacktype = "";
			  if ( owner->isSubclassOf( Player ) )
			  {
			  Player *player = (Player*)owner;
			  attacktype = player->getAttackType();
			  }
			  GameplayFormulaData fd(owner, 0, weap, attacktype);
			  Weapon *weapon = (Weapon*)weap;
			  switch ( weapon->GetCurMode() )
			  {
			  case FIRE_MODE1:
			  bulletDamage = gpm->getFloatValue(weapon->getArchetype() + ".Mode1", "bulletdamage");
			  if ( gpm->hasFormula("DamageMode1") )
			  damage = gpm->calculate("DamageMode1", fd, bulletDamage);
			  break;
			  case FIRE_MODE2:
			  bulletDamage = gpm->getFloatValue(weapon->getArchetype() + ".Mode2", "bulletdamage");
			  if ( gpm->hasFormula("DamageMode2") )
			  damage = gpm->calculate("DamageMode2", fd, bulletDamage);
			  break;
			  }
			  if ( gpm->hasFormula("Knockback") )
			  knockback = gpm->calculate("Knockback", fd);
			  }
			*/
			
			original_value = ent->health;
			ent->Damage( NULL,
				owner,
				damage,
				trace.endpos,
				dir,
				trace.plane.normal,
				knockback,
				dflags,
				meansofdeath,
				trace.intersect.surface,
				trace.intersect.bone,
				weap);
			
			new_value = ent->health;
			damage_total += original_value - new_value;
		}
		else 
		{
		Weapon *theWeapon;
		theWeapon = (Weapon*)weap;

		if ( theWeapon && theWeapon->shouldPlayMissSound() )
			{
			theWeapon->SpawnSound( "snd_ricochet" , trace.endpos , -1 , 1.0 );
			}
		}
		
		// Draw a debug trace line to show bullet fire
		if ( g_showbullettrace->integer )
			G_DebugLine( start, end, 1.0f, 1.0f, 1.0f, 1.0f );
	}
	  
	if ( damage_total > 0.0f )
	{
		if ( owner && owner->isSubclassOf( Player ) )
		{
			Player *player = (Player *)owner;
			player->shotHit();
		}
		
		return damage_total;
	}
	else
	{
		return 0;
	}
}

void ExplosionAttack(
   const Vector   &pos,
   Entity   *owner,
   const str      &explosionModel,
   Vector   dir,
   Entity   *ignore,
   float    scale )
{
	Explosion   *explosion;
	Event			*ev;
	float damageMult = 1.0;
	float damage;
	int meansofdeath;
	float damageDone;
	
	
	if ( !owner )
		owner = world;
	
	if ( explosionModel.length() )
	{
		explosion = new Explosion;
		
		// Create a new explosion entity and set it off
		explosion->setModel( explosionModel );
		
		explosion->mins = Vector( -1.0f, -1.0f, -1.0f );
		explosion->maxs = Vector( 1.0f, 1.0f, 1.0f );

		explosion->setSolidType( SOLID_NOT );
		explosion->CancelEventsOfType( EV_ProcessInitCommands );
		
		// Process the INIT commands right away
		explosion->ProcessInitCommands( explosion->edict->s.modelindex );
		
		explosion->owner             = owner->entnum;
		
		explosion->edict->ownerNum   = owner->entnum;
		
		if ( explosion->can_hit_owner )
			explosion->PostEvent( EV_Projectile_ClearOwner, 0.1f );
		
		explosion->angles            = dir.toAngles();
		explosion->velocity          = dir * explosion->speed;
		explosion->edict->s.scale    = scale;
		explosion->setAngles( explosion->angles );
		
		//explosion->setMoveType( MOVETYPE_FLYMISSILE );

		explosion->setMoveType( MOVETYPE_NONE );

		explosion->edict->clipmask = MASK_PROJECTILE;
		explosion->setSize( explosion->mins, explosion->maxs );
		explosion->setOrigin( pos );
		explosion->origin.copyTo( explosion->edict->s.origin2 );
		
		if ( explosion->dlight_radius )
		{
			G_SetConstantLight( &explosion->edict->s.constantLight,
				&explosion->dlight_color[0],
				&explosion->dlight_color[1],
				&explosion->dlight_color[2],
				&explosion->dlight_radius
				);
		}
		
		explosion->BroadcastSound();
		explosion->animate->RandomAnimate( "idle", EV_StopAnimating );
		
		damage = explosion->radius_damage * scale * damageMult;
		meansofdeath = explosion->meansofdeath;
		
		if ( owner->isSubclassOf( Player ) )
		{
			Player *player = (Player *)owner;
			
			meansofdeath = (int)player->changetMeansOfDeath( (meansOfDeath_t)meansofdeath );
			
			damage = player->getDamageDone( damage, explosion->meansofdeath, false );
		}
		
		damageDone = RadiusDamage( explosion,
			owner,
			damage,
			ignore,
			meansofdeath,
			explosion->radius * scale,
			explosion->knockback,
				        explosion->constant_damage
						);

		if ( damageDone && owner && owner->isSubclassOf( Player ) )
		{
			Player *player = (Player *)owner;
			player->shotHit();
		}
		
		if ( explosion->flash_radius )
		{
			FlashPlayers( explosion->origin,
				explosion->flash_r,
				explosion->flash_g,
				explosion->flash_b,
				explosion->flash_a,
				explosion->flash_minradius * scale,
				explosion->flash_radius * scale,
				explosion->flash_mintime,
				explosion->flash_time,
				explosion->flash_type
				);
		}
		
		if ( explosion->damage_every_frame )
		{
			explosion->PostEvent( EV_Explosion_DamageAgain, FRAMETIME );
		}
		
		// Remove explosion after the life has expired
		
		if ( explosion->life )
		{
			if ( explosion->explosionmodel.length() > 0 )
			{
				ev = new Event( EV_Projectile_Explode );
				explosion->PostEvent( ev, explosion->life );
			}
			else
			{
				ev = new Event( EV_Remove );
				explosion->PostEvent( ev, explosion->life );
			}
		}
	}
}

void StunAttack(
   const Vector   &pos,
   Entity   *attacker,
   Entity   *inflictor,
   float    radius,
   float    time,
   Entity   *ignore )
{
	Entity *ent;
	
	ent = findradius( NULL, inflictor->origin, radius );
	
	while( ent )
	{
		if ( ( ent != ignore ) && ( ( ent->takedamage ) || ent->isSubclassOf( Actor ) ) )
		{
			if ( inflictor->CanDamage( ent, attacker ) )
			{
				// Fixme : Add in knockback
				ent->Stun( time );
			}
		}
		ent = findradius( ent, inflictor->origin, radius );
	}
}

float RadiusDamage(
	Entity   *inflictor,
	Entity   *attacker,
	float      damage,
	Entity   *ignore,
	int      mod,
	float		radius,
	float		knockback,
	qboolean	constant_damage)
{
	float		points;
	Entity	*ent;
	Vector	org;
	Vector	dir;
	float		dist;
	float		rad;
	float totalDamage;
	
	if ( radius )
		rad = radius;
	else
		rad = damage + 60.0f;

	totalDamage = 0.0f;
	
	ent = findradius( NULL, inflictor->origin, rad );
	
	while( ent )
	{
		if ( ( ent != ignore ) && ( ent->takedamage ) && ( ent->edict->solid != SOLID_NOT ) )
		{
			org = ent->centroid;
			dir = org - inflictor->origin;
			dist = dir.length();
			
			if ( dist > rad )
			{
				ent = findradius( ent, inflictor->origin, rad );
				continue;
			}
			
			if ( constant_damage )
			{
				points = damage;
				
			}
			else
			{
				points = damage - ( damage * ( dist / rad ) );
				
				knockback -= knockback * ( dist / rad );
				
				if ( points < 0.0f )
					points = 0.0f;
				
				if ( knockback < 0.0f )
					knockback = 0.0f;
			}
			
			if ( ent == attacker  )
			{
				points *= 0.5f;
			}
			
			if ( points > 0.0f )
			{
				// Add this in for deathmatch maybe
				
				// Only players can use cover
				//if ( !ent->isSubclassOf( Player ) || inflictor->CanDamage( ent, attacker ) )
				if ( inflictor->CanDamage( ent, attacker ) )
				{
					org = ent->centroid;
					dir = org - inflictor->origin;
					ent->Damage( inflictor,
						attacker,
						points,
						org,
						dir,
						vec_zero,
						knockback,
						DAMAGE_RADIUS,
						mod
						);

					totalDamage += points;
				}
			}
		}
		ent = findradius( ent, inflictor->origin, rad );
	}

	return totalDamage;
}


void FlashPlayers(
   const Vector   &org,
   float    r,
   float    g,
   float    b,
   float    a,
   float    minradius,
   float    radius,
   float	mintime,
   float    time,
   int      type )
{
	trace_t     trace;
	Vector      delta;
	float       length;
	Player      *player;
	gentity_t   *ed;
	int         i;
	Entity      *ent;
	float       newa = 1;
	Vector		dirToExplosion;
	Vector		playerViewDir;
	Vector		playerViewAngles;
	float		dot;
	
	for( i = 0; i < game.maxclients; i++ )
	{
		ed = &g_entities[ i ];
		if ( !ed->inuse || !ed->entity )
			continue;
		
		ent = ed->entity;
		if ( !ent->isSubclassOf( Player ) )
			continue;
		
		player = ( Player * )ent;
		
		if ( !player->WithinDistance( org, radius ) )
			continue;
		
		trace = G_Trace( org, vec_zero, vec_zero, player->origin, player, MASK_OPAQUE, false, "FlashPlayers" );
		if ( trace.fraction != 1.0f )
			continue;
		
		delta = org - trace.endpos;
		length = delta.length();
		
		dirToExplosion = org - player->origin;
		
		playerViewAngles = player->GetVAngles();
		playerViewAngles.AngleVectors( &playerViewDir );
		
		dot = dirToExplosion * playerViewDir;
		
		if ( dot < 0.0f )
			continue;
		
		// If alpha is specified, then modify it by the amount of distance away from the flash the player is
		if ( a != -1.0f )
		{
			if ( length < minradius )
				newa = a;
			else if ( length < radius )
				newa = a * ( 1.0f - ( length - minradius ) / ( radius - minradius ) );
			else
				newa = 0.0f;
		}
		
	/* if ( g_gametype->integer == GT_SINGLE_PLAYER ) // Single player uses the global level variables for flashes
	{
		level.m_fade_alpha      = newa;
		level.m_fade_color[0]   = r;
		level.m_fade_color[1]   = g;
		level.m_fade_color[2]   = b;
		level.m_fade_time       = time;
		level.m_fade_time_start = time;
		
		if ( type == 1 )
			level.m_fade_style = additive;
		else
			level.m_fade_style = alphablend;
	}
	else
	{
		player->AddBlend( r,g,b,newa );
	} */
		
		player->setFlash( Vector( r, g, b ), newa, mintime, time );
	}
}

