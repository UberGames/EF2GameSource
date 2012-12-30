//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/DamageModification.cpp   $
// $Revision:: 13                                             $
//     $Date:: 10/10/02 2:39p                              $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
//
// DamageModification.cpp: implementation of the DamageModification class.
//
//////////////////////////////////////////////////////////////////////

#include "_pch_cpp.h"
#include "DamageModification.hpp"
#include "actor.h"


// ------------ DamageModificationSystem -----------------------------

//--------------------------------------------------------------
//
// Name:			DamageModificationSystem		
// Class:			DamageModificationSystem
//
// Description:		Constructor / Destructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
DamageModificationSystem::DamageModificationSystem()
{
	damageModifiers.ClearObjectList();
}

DamageModificationSystem::~DamageModificationSystem()
{
	int i;
	for ( i=1; i<=damageModifiers.NumObjects(); i++ )
	{
		DamageModifier *damMod = damageModifiers.ObjectAt(i);
		delete damMod;
	}
	
	damageModifiers.FreeObjectList();
}

//--------------------------------------------------------------
//
// Name:			AddDamageModifier
// Class:			DamageModificationSystem
//
// Description:		Adds a new damage modifier to the internal
//					list.
//
// Parameters:		const str &damagemodtype -- String version of the DamageModifierType
//					const str &value -- String value to be determined by the type
//					float multiplier -- the multiplier to this damage mod.
//
// Returns:			None
//
//--------------------------------------------------------------
void DamageModificationSystem::addDamageModifier( const str &damagemodtype, const str &value, float multiplier,
		float chance, float painBaseLine )
{
	DamageModifier *newMod = 0;
	
	if ( damagemodtype == "tikiname" )
		newMod = new DamageModifierTikiName(TIKI_NAME, value, multiplier, chance, painBaseLine );
	
	else if ( damagemodtype == "name" )
		newMod = new DamageModifierName(NAME, value, multiplier, chance, painBaseLine );
	
	else if ( damagemodtype == "group" )
	{
		int groupVal = atoi(value);
		newMod = new DamageModifierGroup(GROUP, groupVal, multiplier, chance, painBaseLine );
	}
	
	else if ( damagemodtype == "actortype" )
	{
		int actortype = Actor::ActorTypeStringToInt(value);
		newMod = new DamageModifierActorType(ACTOR_TYPE, actortype, multiplier, chance, painBaseLine );
	}
	
	else if ( damagemodtype == "targetname" )
		newMod = new DamageModifierTargetName(TARGETNAME, value, multiplier, chance, painBaseLine );
	
	else if ( damagemodtype == "damagetype" )
	{
		int damagetypeVal = MOD_NameToNum(value);
		newMod = new DamageModifierDamageType(GROUP, damagetypeVal, multiplier, chance, painBaseLine );
	}
	
	damageModifiers.AddObject(newMod);
}


//--------------------------------------------------------------
//
// Name:			AddDamageModifier
// Class:			DamageModificationSystem
//
// Description:		Directly adds a premade DamageModifier object to the list
//					(this is currently only called from archive loading)
//
// Parameters:		DamageModifier *newModifier -- Modifier to add to the list
//
// Returns:			None
//
//--------------------------------------------------------------
void DamageModificationSystem::addDamageModifier(DamageModifier *newModifier)
{
	if ( newModifier )
		damageModifiers.AddObject(newModifier);
}

//--------------------------------------------------------------
//
// Name:			resolveDamage
// Class:			DamageModificationSystem
//
// Description:		The main damage resolving function
//
// Parameters:		Damage &damage -- The damage class to resolve
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModificationSystem::resolveDamage(Damage &damage)
{
	int i;
	for ( i=1; i<=damageModifiers.NumObjects(); i++ )
	{
		DamageModifier *damageMod = damageModifiers.ObjectAt( i );
		damageMod->resolveDamage(damage);
	}
	
	damage.showPain = false;
	for ( i=1; i<=damageModifiers.NumObjects(); i++ )
	{
		DamageModifier *damageMod = damageModifiers.ObjectAt( i );
		damageMod->resolvePain(damage);
	}
}


// ------------ Damage -----------------------------------------

//--------------------------------------------------------------
//
// Name:			Damage	
// Class:			Damage
//
// Description:		Constructor / Destructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
Damage::Damage()
{
	damage			= 0.0f;
	inflictor		= 0;
	attacker		= 0;
	position		= vec_zero;
	direction		= vec_zero;
	normal			= vec_zero;
	knockback		= 0;
	dflags			= 0;
	meansofdeath	= -1;
	surfaceNumber	= -1;
	boneNumber		= -1;
	showPain		= false;
	weapon			= 0;
}

// Constructor that takes the typical damage event as the initializer
Damage::Damage( Event *ev )
{
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
	else
		surfaceNumber = -1;

	if ( ev->NumArgs() > 10 )
		boneNumber = ev->GetInteger( 11 );
	else
		boneNumber = -1;

	if ( ev->NumArgs() > 11 )
		weapon = ev->GetEntity( 12 );
	else
		weapon = 0;
}

Damage::~Damage()
{

}


// ------------ DamageModifierTikiName -------------------------

//--------------------------------------------------------------
//
// Name:			resolveDamage
// Class:			DamageModifierTikiName
//
// Description:		Resolve damage for tiki name type modifiers
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierTikiName::resolveDamage(Damage &damage)
{
	if ( damage.attacker->model == _tikiName )
	{
		float rand = G_Random();
		if ( rand < getChance() )
			damage.damage *= getMultiplier();
		else
			damage.damage = 0.0f;
	}
}

//--------------------------------------------------------------
//
// Name:			resolvePain
// Class:			DamageModifierTikiName
//
// Description:		Resolve pain for this damage
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierTikiName::resolvePain(Damage &damage)
{
	if ( damage.attacker->model == _tikiName )
	{
		if ( G_Random() <= (damage.damage / getPainBaseLine() ) )
			damage.showPain = true;			
	}
}


// ------------ DamageModifierName -------------------------

//--------------------------------------------------------------
//
// Name:			resolveDamage
// Class:			DamageModifierName
//
// Description:		Resolve damage for name type modifiers
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierName::resolveDamage(Damage &damage)
{
	if ( damage.attacker->isSubclassOf( Actor ) )
	{
		Actor *act = (Actor *)damage.attacker;
		if ( act->name == _name )
		{
			float rand = G_Random();
			if ( rand < getChance() )
				damage.damage *= getMultiplier();
			else
				damage.damage = 0.0f;
		}
	}
}

//--------------------------------------------------------------
//
// Name:			resolvePain
// Class:			DamageModifierName
//
// Description:		Resolve pain for this damage
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierName::resolvePain(Damage &damage)
{
	if ( damage.attacker->isSubclassOf( Actor ) )
	{
		Actor *act = (Actor *)damage.attacker;
		if ( act->name == _name )
		{
			if ( G_Random() <= (damage.damage / getPainBaseLine() ) )
				damage.showPain = true;			
		}
	}
}

// ------------ DamageModifierGroup -------------------------

//--------------------------------------------------------------
//
// Name:			resolveDamage
// Class:			DamageModifierGroup
//
// Description:		Resolve damage for group type modifiers
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierGroup::resolveDamage(Damage &damage)
{
	if ( damage.attacker->GetGroupID() == _group )
	{
		float rand = G_Random();
		if ( rand < getChance() )
			damage.damage *= getMultiplier();
		else
			damage.damage = 0.0f;
	}
}

//--------------------------------------------------------------
//
// Name:			resolvePain
// Class:			DamageModifierGroup
//
// Description:		Resolve pain for this damage
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierGroup::resolvePain(Damage &damage)
{
	if ( damage.attacker->GetGroupID() == _group )
	{
		if ( G_Random() <= (damage.damage / getPainBaseLine() ) )
			damage.showPain = true;	
	}
}



// ------------ DamageModifierActorType -------------------------

//--------------------------------------------------------------
//
// Name:			resolveDamage
// Class:			DamageModifierActorType
//
// Description:		Resolve damage for ActorType type modifiers
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierActorType::resolveDamage(Damage &damage)
{
	if ( damage.attacker->isSubclassOf( Actor ) )
	{
		Actor *act = (Actor *)damage.attacker;
		if ( act->actortype == _actortype )
		{
			float rand = G_Random();
			if ( rand < getChance() )
				damage.damage *= getMultiplier();
			else
				damage.damage = 0.0f;
		}
	}
}

//--------------------------------------------------------------
//
// Name:			resolvePain
// Class:			DamageModifierActorType
//
// Description:		Resolve pain for this damage
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierActorType::resolvePain(Damage &damage)
{
	if ( damage.attacker->isSubclassOf( Actor ) )
	{
		Actor *act = (Actor *)damage.attacker;
		if ( act->actortype == _actortype )
		{
			if ( G_Random() <= (damage.damage / getPainBaseLine() ) )
				damage.showPain = true;			
		}
	}
}


// ------------ DamageModifierTargetName -------------------------

//--------------------------------------------------------------
//
// Name:			resolveDamage
// Class:			DamageModifierTargetName
//
// Description:		Resolve damage for TargetName type modifiers
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierTargetName::resolveDamage(Damage &damage)
{
	if ( damage.attacker->targetname == _targetname )
	{
		float rand = G_Random();
		if ( rand < getChance() )
			damage.damage *= getMultiplier();
		else
			damage.damage = 0.0f;
	}
}

//--------------------------------------------------------------
//
// Name:			resolvePain
// Class:			DamageModifierTargetName
//
// Description:		Resolve pain for this damage
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierTargetName::resolvePain(Damage &damage)
{
	if ( damage.attacker->targetname == _targetname )
	{
		if ( G_Random() <= (damage.damage / getPainBaseLine() ) )
			damage.showPain = true;	
		
	}
}



// ------------ DamageModifierDamageType -------------------------

//--------------------------------------------------------------
//
// Name:			resolveDamage
// Class:			DamageModifierDamageType
//
// Description:		Resolve damage for DamageType type modifiers
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierDamageType::resolveDamage(Damage &damage)
{
	if ( damage.meansofdeath == _damagetype )
	{
		float rand = G_Random();
		if ( rand < getChance() )
			damage.damage *= getMultiplier();
		else
			damage.damage = 0.0f;
	}
	
}

//--------------------------------------------------------------
//
// Name:			resolvePain
// Class:			DamageModifierDamageType
//
// Description:		Resolve pain from this damage
//
// Parameters:		Damage &damage -- Damage reference to modify
//
// Returns:			None (reference above)
//
//--------------------------------------------------------------
void DamageModifierDamageType::resolvePain(Damage &damage)
{
	if ( damage.meansofdeath == _damagetype )
	{
		if ( G_Random() <= ( damage.damage / getPainBaseLine() ) )
			damage.showPain = true;			
		
	}
}
