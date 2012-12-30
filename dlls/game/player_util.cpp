//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/player_util.cpp                               $
// $Revision:: 15                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/05/02 9:18p                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// This file is used to hold the utility functions that are issued by the
// player at the console.  Most of these are developer commands

#include "_pch_cpp.h"
#include "player.h"
#include "object.h"

//====================
//Player::ActorInfo
//====================
void Player::ActorInfo( Event *ev )
{
	int num;
	Entity *ent;
	
	if ( ev->NumArgs() != 1 )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Usage: actorinfo <entity number>\n\"" );
		return;
	}
	
	num = ev->GetInteger( 1 );
	if ( ( num < 0 ) || ( num >= globals.max_entities ) )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Value out of range.  Possible values range from 0 to %d.\n\"", globals.max_entities );
		return;
	}
	
	ent = G_GetEntity( num );
	if ( !ent || !ent->isSubclassOf( Actor ) )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Entity not an Actor.\n\"" );
	}
	else
	{
		( ( Actor * )ent )->ShowInfo();
	}
}

//====================
//Player::WhatIs
//====================
void Player::WhatIs( Event *ev )
{
	int num;
	Entity *ent;
	
	if ( ev->NumArgs() != 1 )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Usage: whatis <entity number>\n\"" );
		return;
	}
	
	num = ev->GetInteger( 1 );
	if ( ( num < 0 ) || ( num >= globals.max_entities ) )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Value out of range.  Possible values range from 0 to %d.\n\"", globals.max_entities );
		return;
	}
	
	ent = G_GetEntity( num );
	if ( !ent )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Entity not in use.\n\"", globals.max_entities );
	}
	else
	{
		const char * animname;
		
		animname = NULL;
		if ( gi.IsModel( ent->edict->s.modelindex ) )
		{
			animname = gi.Anim_NameForNum( ent->edict->s.modelindex, ent->edict->s.anim & ANIM_MASK );
		}
		
		if ( !animname )
		{
			animname = "( N/A )";
		}
		
		gi.SendServerCommand( edict-g_entities, "print \""
			"Entity #   : %d\n"
			"Class ID   : %s\n"
			"Classname  : %s\n"
			"Targetname : %s\n"
			"Modelname  : %s\n"
			"Animname   : %s\n"
			"Origin     : ( %f, %f, %f )\n"
			"Angles     : ( %f, %f, %f )\n"
			"Bounds     : Mins( %.2f, %.2f, %.2f ) Maxs( %.2f, %.2f, %.2f )\n"
			"Velocity   : ( %f, %f, %f )\n"
			"SVFLAGS    : %x\n"
			"Movetype   : %i\n"
			"Solidtype  : %i\n"
			"Contents   : %x\n"
			"Areanum    : %i\n"
			"Parent     : %i\n"
			"Health     : %.1f\n"
			"Max Health : %.1f\n"
			"Edict Owner: %i\n\"",
			num,
			ent->getClassID(),
			ent->getClassname(),
			ent->TargetName(),
			ent->model.c_str(),
			animname,
			ent->origin.x, ent->origin.y, ent->origin.z,
			ent->angles.x, ent->angles.y, ent->angles.z,
			ent->mins.x, ent->mins.y, ent->mins.z, ent->maxs.x, ent->maxs.y, ent->maxs.z,
			ent->velocity.x, ent->velocity.y, ent->velocity.z,
			ent->edict->svflags,
			ent->movetype,
			ent->edict->solid,
			ent->edict->contents,
			ent->edict->areanum,
			ent->edict->s.parent,
			ent->health,
			ent->max_health,
			ent->edict->ownerNum
			);
	}
}

//====================
//Player::KillEnt
//====================
void Player::KillEnt( Event * ev )
{
	int num;
	Entity *ent;
	
	if ( ev->NumArgs() != 1 )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Usage: killent <entity number>\n\"" );
		return;
	}
	
	num = ev->GetInteger( 1 );
	if ( ( num < 0 ) || ( num >= globals.max_entities ) )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Value out of range.  Possible values range from 0 to %d.\n\"", globals.max_entities );
		return;
	}
	
	ent = G_GetEntity( num );
	ent->Damage( world, world, ent->max_health + 25.0f, origin, vec_zero, vec_zero, 0, 0, 0 );
}

//====================
//Player::RemoveEnt
//====================
void Player::RemoveEnt( Event * ev )
{
	int num;
	Entity *ent;
	
	if ( ev->NumArgs() != 1 )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Usage: removeent <entity number>\n\"" );
		return;
	}
	
	num = ev->GetInteger( 1 );
	if ( ( num < 0 ) || ( num >= globals.max_entities ) )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Value out of range.  Possible values range from 0 to %d.\n\"", globals.max_entities );
		return;
	}
	
	ent = G_GetEntity( num );
	ent->PostEvent( Event( EV_Remove ), 0.0f );
}

//====================
//Player::KillClass
//====================
void Player::KillClass( Event * ev )
{
	int except;
	str classname;
	gentity_t * from;
	Entity *ent;
	
	if ( ev->NumArgs() < 1 )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Usage: killclass <classname> [except entity number]\n\"" );
		return;
	}
	
	classname = ev->GetString( 1 );
	
	except = 0;
	if ( ev->NumArgs() == 2 )
	{
		except = ev->GetInteger( 1 );
	}
	
	for ( from = this->edict + 1; from < &g_entities[ globals.num_entities ]; from++ )
	{
		if ( !from->inuse )
		{
			continue;
		}
		
		assert( from->entity );
		
		ent = from->entity;
		
		if ( ent->entnum == except )
		{
			continue;
		}
		
		if ( ent->inheritsFrom( classname.c_str() ) )
		{
			ent->Damage( world, world, ent->max_health + 25.0f, origin, vec_zero, vec_zero, 0, 0, 0 );
		}
	}
}

//====================
//Player::RemoveClass
//====================
void Player::RemoveClass( Event * ev )
{
	int except;
	str classname;
	gentity_t * from;
	Entity *ent;
	
	if ( ev->NumArgs() < 1 )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Usage: removeclass <classname> [except entity number]\n\"" );
		return;
	}
	
	classname = ev->GetString( 1 );
	
	except = 0;
	if ( ev->NumArgs() == 2 )
	{
		except = ev->GetInteger( 1 );
	}
	
	for ( from = this->edict + 1; from < &g_entities[ globals.num_entities ]; from++ )
	{
		if ( !from->inuse )
		{
			continue;
		}
		
		assert( from->entity );
		
		ent = from->entity;
		
		if ( ent->entnum == except )
			continue;
		
		if ( ent->inheritsFrom( classname.c_str() ) )
		{
			ent->PostEvent( Event( EV_Remove ), 0.0f );
		}
	}
}

//====================
//Player::TestThread
//====================
void Player::TestThread( Event *ev )
{
	const char *label;
	CThread *thread;
	
	if ( ev->NumArgs() < 1 )
	{
		gi.SendServerCommand( edict-g_entities, "print \"Syntax: testthread label.\n\"" );
		return;
	}
	
	label = ev->GetString( 1 );
	thread = Director.CreateThread( label );
	if ( thread )
	{
		// start right away
		thread->Start();
	}
}

//====================
//Player::SpawnEntity
//====================
void Player::SpawnEntity( Event *ev )
{
	Entity		*ent;
	str         name;
	ClassDef		*cls;
	str         text;
	Vector		forward;
	Vector		up;
	Vector		delta;
	Vector		v;
	int			n;
	int			i;
	Event       *e;
	
	if ( ev->NumArgs() < 1 )
	{
		ev->Error( "Usage: spawn entityname [keyname] [value]..." );
		return;
	}
	
	name = ev->GetString( 1 );
	if ( !name.length() )
	{
		ev->Error( "Must specify an entity name" );
		return;
	}
	
	// create a new entity
	SpawnArgs args;
	
	args.setArg( "classname", name.c_str() );
	args.setArg( "model", name.c_str() );
	
	cls = args.getClassDef();
	if ( !cls )
	{
		cls = &Entity::ClassInfo;
	}
	
	if ( !checkInheritance( &Entity::ClassInfo, cls ) )
	{
		ev->Error( "%s is not a valid Entity", name.c_str() );
		return;
	}
	
	ent = ( Entity * )cls->newInstance();
	
	e = new Event( EV_Model );
	e->AddString( name.c_str() );
	ent->PostEvent( e, EV_SPAWNARG );
	
	angles.AngleVectors( &forward, NULL, &up );
	v = origin + ( ( forward + up ) * 128.0f );
	
	e = new Event( EV_SetOrigin );
	e->AddVector( v );
	ent->PostEvent( e, EV_SPAWNARG );
	
	delta = origin - v;
	v.x = 0;
	v.y = delta.toYaw();
	v.z = 0;
	
	e = new Event( EV_SetAngles );
	e->AddVector( v );
	ent->PostEvent( e, EV_SPAWNARG );
	
	if ( ev->NumArgs() > 2 )
	{
		n = ev->NumArgs();
		for( i = 2; i <= n; i += 2 )
		{
			e = new Event( ev->GetString( i ) );
			e->AddToken( ev->GetString( i + 1 ) );
			ent->PostEvent( e, EV_SPAWNARG );
		}
	}
	
	e = new Event( EV_Anim );
	e->AddString( "idle" );
	ent->PostEvent( e, EV_SPAWNARG );
}

//====================
//Player::SpawnActor
//====================
void Player::SpawnActor( Event *ev )
{
	Entity   *ent;
	str      name;
	str      text;
	Vector	forward;
	Vector	up;
	Vector	delta;
	Vector	v;
	int		n;
	int		i;
	ClassDef	*cls;
	Event    *e;
	
	if ( ev->NumArgs() < 1 )
	{
		ev->Error( "Usage: actor [modelname] [keyname] [value]..." );
		return;
	}
	
	name = ev->GetString( 1 );
	if ( !name[ 0 ] )
	{
		ev->Error( "Must specify a model name" );
		return;
	}
	
	if ( !strstr( name.c_str(), ".tik" ) )
	{
		name += ".tik";
	}
	
	// create a new entity
	SpawnArgs args;
	
	args.setArg( "model", name.c_str() );
	
	cls = args.getClassDef();
	
	if ( cls == &Object::ClassInfo )
	{
		cls = &Actor::ClassInfo;
	}
	
	if ( !cls || !checkInheritance( &Actor::ClassInfo, cls ) )
	{
		ev->Error( "%s is not a valid Actor", name.c_str() );
		return;
	}
	
	ent = ( Entity * )cls->newInstance();
	e = new Event( EV_Model );
	e->AddString( name.c_str() );
	ent->PostEvent( e, EV_SPAWNARG );
	
	angles.AngleVectors( &forward, NULL, &up );
	v = origin + ( ( forward + up ) * 40.0f );
	
	e = new Event( EV_SetOrigin );
	e->AddVector( v );
	ent->PostEvent( e, EV_SPAWNARG );
	
	delta = origin - v;
	v = delta.toAngles();
	
	e = new Event( EV_SetAngle );
	e->AddFloat( v[ 1 ] );
	ent->PostEvent( e, EV_SPAWNARG );
	
	if ( ev->NumArgs() > 2 )
	{
		n = ev->NumArgs();
		for( i = 2; i <= n; i += 2 )
		{
			e = new Event( ev->GetString( i ) );
			e->AddToken( ev->GetString( i + 1 ) );
			ent->PostEvent( e, EV_SPAWNARG );
		}
	}
}

void Player::ListInventoryEvent( Event *ev )
{
	ListInventory();
}

void Player::GetStateAnims( Container<const char *> *c )
{
	statemap_Legs->GetAllAnims( c );
	statemap_Torso->GetAllAnims( c );
}

static fileHandle_t logfile=NULL;

static void OpenPlayerLogFile( void )
{
	str s,filename;
	
	s = "playlog_";
	s += level.mapname;
	filename = gi.GetArchiveFileName( NULL, s, "log", qtrue );
	
	logfile = gi.FS_FOpenFileWrite( filename.c_str() );	
}

void Player::LogStats( Event *ev )
{
	str s;
	
	if ( !logfile )
	{
		OpenPlayerLogFile();
	}
	
	if ( !logfile )
	{
		return;
	}
	
	int b =  AmmoCount( "Bullet" );
	int p =  AmmoCount( "Plasma" );
	int g =  AmmoCount( "Gas" );
	int r =  AmmoCount( "Rocket" );
	int f =  AmmoCount( "Flashbangs" );
	int m =  AmmoCount( "Meteor" );
	int gp = AmmoCount( "Gas Pod" );
	
	s = va(  "%.2f\t", level.time );
	s += va( "(%.2f %.2f %.2f)\t", origin.x, origin.y, origin.z );
	s += va( "%.2f\t", health );
	s += va( "%d\t%d\t%d\t%d\t%d\t%d\t%d\n", b,p,g,r,f,m,gp );
	
	gi.FS_Write( s, s.length(), logfile );
	gi.FS_Flush( logfile );
	
	Event *ev1 = new Event( ev );
	PostEvent( ev1, 1.0f );
}

void ClosePlayerLogFile( void )
{
	if ( logfile )
	{
		gi.FS_FCloseFile( logfile );
		logfile = NULL;
	}
}

void Player::SkipCinematic( Event *ev )
{
	if ( level.cinematic && ( world->skipthread.length() > 1 ) )
	{
		str skipthread;
		G_ClearFade();
		
		skipthread = world->skipthread;
		// now that we have executed it, lets kill it so we don't call it again
		world->skipthread = "";
		ExecuteThread( skipthread );
		// reset the roll on our view just in case
		v_angle.z = 0;
		SetViewAngles( v_angle );
	}
}
