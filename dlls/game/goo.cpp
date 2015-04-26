//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/goo.cpp                                   $
// $Revision:: 7                                                              $
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
// Goo Gun Projectile

#include "_pch_cpp.h"
#include "goo.h"

Event EV_GooDebris_Prethink
(
	"_prethink",
	EV_CODEONLY,
	NULL,
	NULL,
	"Think function for the debris"
);

CLASS_DECLARATION( Projectile, GooDebris, NULL )
{
	{ &EV_GooDebris_Prethink,				&GooDebris::Prethink },
	{ &EV_Touch,							&GooDebris::Touch },

	{ NULL, NULL }
};

GooDebris::GooDebris()
{
	nexttouch = 0;
}

void GooDebris::Prethink( Event *ev )
{
	if ( ( velocity.length() < 30.0f ) && ( CurrentAnim() == gi.Anim_NumForName( edict->s.modelindex, "idle" ) ) )
	{
		velocity = Vector( 0.0f, 0.0f, 0.0f );
		animate->RandomAnimate( "splat" );
	}
	
	if ( ( level.time - this->edict->spawntime ) > 5.0f )
	{
		edict->s.scale *= 0.95f;
	}
	
	if ( edict->s.scale < 0.1f )
	{
		PostEvent( EV_Remove, 0.0f );
	}
	else
	{
		Event *ev1 = new Event( ev );
		PostEvent( ev1, level.frametime );
	}
}

void GooDebris::Touch( Event *ev )
{
	Entity *other;
	Entity *owner;
	Vector ang;
	
	other = ev->GetEntity( 1 );
	
	if ( other == world )
	{
		vectoangles( level.impact_trace.plane.normal, ang );
		setAngles( ang );
	}
	
	if ( level.time < nexttouch )
	{
		return;
	}
	
	nexttouch = level.time + 0.5f;
	
	if ( !other || !other->isSubclassOf( Sentient ) )
	{
		return;
	}
	
	owner = G_GetEntity( this->owner );
	
	if ( !owner )
	{
		owner = world;
	}
	
	if ( !other )
	{
		return;
	}
	
	other->Damage( this, owner, damage, origin, Vector( 0.0f, 0.0f, 0.0f ), Vector( 0.0f, 0.0f, 0.0f ), 0, 0, meansofdeath );
}

Event EV_Goo_DebrisModel
(
	"goodebrismodel",
	EV_TIKIONLY,
	"s",
	"modelname",
	"Model name for the debris that is spawned on explosion"
);
Event EV_Goo_DebrisCount
(
	"goodebriscount",
	EV_TIKIONLY,
	"i",
	"count",
	"Number of pieces of debris to spawn"
);

CLASS_DECLARATION( Projectile, GooProjectile, NULL )
{
	{ &EV_Goo_DebrisModel,					&GooProjectile::SetDebrisModel },
	{ &EV_Goo_DebrisCount,					&GooProjectile::SetDebrisCount },

	{ NULL, NULL }
};

GooProjectile::GooProjectile()
{
	m_debrismodel = "fx_goo_debris.tik";
	m_debriscount = 3;
}

void GooProjectile::SetDebrisModel( Event *ev )
{
	m_debrismodel = ev->GetString( 1 );
}

void GooProjectile::SetDebrisCount( Event *ev )
{
	m_debriscount = ev->GetInteger( 1 );
}

void GooProjectile::Explode( Event *ev )
{
	int i;
	Entity *owner;
	Entity *ignoreEnt=NULL;
	
	if ( ev->NumArgs() == 1 )
		ignoreEnt = ev->GetEntity( 1 );
	
	// Get the owner of this projectile
	owner = G_GetEntity( this->owner );
	
	// If the owner's not here, make the world the owner
	if ( !owner )
		owner = world;
	
	takedamage = DamageNo;
	
	// Spawn an explosion model
	if ( explosionmodel.length() )
	{
		// Move the projectile back off the surface a bit so we can see
		// explosion effects.
		Vector dir, v;
		v = velocity;
		v.normalize();
		dir = v;
		v = origin - v * 36.0f;
		setOrigin( v );
		
		ExplosionAttack( v, owner, explosionmodel, dir, ignoreEnt );
	}
	
	CancelEventsOfType( EV_Projectile_UpdateBeam );
	
	// Kill the beam
	if ( m_beam )
	   {
		m_beam->ProcessEvent( EV_Remove );
		m_beam = NULL;
	   }
	
	// When the goo hits something, spawn debris
	for ( i=0; i<m_debriscount; i++ )
	{
		GooDebris  *proj;
		Vector      dir;
		
		dir = Vector( level.impact_trace.plane.normal );
		dir += Vector( crandom() * 0.5, crandom() * 0.5f, 0.0f );
		
		proj = ( GooDebris * )ProjectileAttack( this->origin, dir, this, m_debrismodel, 1.0f );
		
		if ( !proj )
		{
			warning( "GooProjectile::Explode", "Could not create debris projectile" );
			return;
		}
		
		proj->owner = ENTITYNUM_WORLD;
		proj->setSolidType( SOLID_TRIGGER );
		proj->avelocity = Vector( G_CRandom( 360.0f ), G_CRandom( 360.0f ), G_CRandom( 360.0f ) );
		proj->setSize( Vector( -16.0f, -16.0f, 0.0f ) * proj->edict->s.scale, Vector( 16.0f ,16.0f ,32.0f ) * proj->edict->s.scale );
		proj->setMoveType( MOVETYPE_TOSS );
		proj->PostEvent( EV_GooDebris_Prethink, level.frametime );
	}
	
	// Change to the splat
	if ( level.impact_trace.ent && ( level.impact_trace.ent->solid == SOLID_BSP ) )
	{
		GooDebris *p;
		Vector ang;
		p = new GooDebris;
		vectoangles( level.impact_trace.plane.normal, ang );
		p->setAngles( ang );
		p->setModel( this->model );
		p->ProcessInitCommands( p->edict->s.modelindex );
		p->setSize( Vector( -16.0f, -16.0f, 0.0f ) * p->edict->s.scale, Vector( 16.0f, 16.0f, 32.0f ) * p->edict->s.scale );
		p->setOrigin( this->origin );
		p->velocity = Vector( 0.0f, 0.0f, 0.0f );
		p->setMoveType( MOVETYPE_FLY );
		p->setSolidType( SOLID_TRIGGER );
		p->animate->RandomAnimate( "splat" );
		p->owner = this->owner;
		p->PostEvent( EV_GooDebris_Prethink, level.frametime ); // shrink out
	}
}
