//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/actor_combatsubsystem.cpp                  $
// $Revision:: 65                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:35p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//

#include "_pch_cpp.h"
#include "actor_combatsubsystem.h"
#include "player.h"
#include "object.h"
#include <qcommon/gameplaymanager.h>


//======================================
// CombatSubsystem Implementation
//======================================

//
// Name:        CombatSubsystem()
// Parameters:  None
// Description: Constructor()
//
CombatSubsystem::CombatSubsystem()
{
	// Should always use other constructor
	gi.Error( ERR_FATAL, "CombatSubsystem::CombatSubsystem -- Default Constructor Called" );	
}


//
// Name:        CombatSubsystem()
// Parameters:  Actor *actor
// Description: Constructor
//
CombatSubsystem::CombatSubsystem( Actor *actor )
{
	//Initialize our Actor
	if ( actor )
		act = actor;
	else 
		gi.Error( ERR_DROP, "MovementSubsystem::MovementSubsystem -- actor is NULL" );
	
	_init();
}


//
// Name:        ~CombatSubystem()
// Parameters:  None
// Description: Destructor
//
CombatSubsystem::~CombatSubsystem()
{
	
}


//
// Name:        UseActorWeapon()
// Parameters:  const str &weaponName
// Description: Sets the weapon to be active, and attaches it
//
void CombatSubsystem::UseActorWeapon(const str &weaponName , weaponhand_t hand )
{
	Weapon *weapon;
	
	//First see if we just want to put our current item away
	if ( !stricmp( weaponName.c_str() , "none" ) )
	{
		act->DeactivateWeapon(WEAPON_LEFT);
		act->DeactivateWeapon(WEAPON_RIGHT);
		act->DeactivateWeapon(WEAPON_DUAL);
		act->AddStateFlag( STATE_FLAG_CHANGED_WEAPON );
		act->ClearTorsoAnim();
		_activeWeapon.weapon = NULL;
		
		return;
	}
	
	weapon = ( Weapon * )act->FindItem( weaponName.c_str() );
	
	// Check to see if player has the weapon
	if ( !weapon )
	{
		act->warning( "CombatSubsystem::UseActorWeapon", "Actor does not have weapon %s", weaponName.c_str() );
		return;
	}
	
	// If we don't already have an active weapon, just go ahead and make this one active
	if ( !_activeWeapon.weapon )
	{
		_activeWeapon.weapon = weapon;
		_activeWeapon.hand = hand;
		_activeWeapon.weapon->SetOwner( act );
		act->ActivateWeapon(weapon, hand);
		act->AddStateFlag( STATE_FLAG_CHANGED_WEAPON );
		return;
	}
	
	// See if we are already using this weapon
	if ( weapon == _activeWeapon.weapon && hand == _activeWeapon.hand )
		return;
	
	// Well, we have a weapon in the same hand, put away the weapon thats there.
	Weapon *oldweapon = act->GetActiveWeapon(hand);
	if ( oldweapon )
		act->DeactivateWeapon(hand);
	
	// Activate this weapon
	_activeWeapon.weapon = weapon;
	_activeWeapon.hand = hand;
	_activeWeapon.weapon->SetOwner( act );
	act->ActivateWeapon(weapon, hand);
	act->AddStateFlag( STATE_FLAG_CHANGED_WEAPON );
}

//
// Name:        UsingWeaponNamed()
// Parameters:  None
// Description: Checks if our _activeWeapon has the same name
//
bool CombatSubsystem::UsingWeaponNamed( const str &weaponName )
{
	if ( !_activeWeapon.weapon )
		return false;
	
	
	if ( !Q_stricmp(_activeWeapon.weapon->getName().c_str() , weaponName.c_str() ) )
		return true;
	
	return false;
}

//
// Name:        HaveWeapon()
// Parameters:  None
// Description: Returns True or False depending on _activeWeapon
//
bool CombatSubsystem::HaveWeapon()
{
	if ( _activeWeapon.weapon )
		return true;
	
	return false;
}

bool CombatSubsystem::CanAttackEnemy()
{
	Entity *currentEnemy = NULL;
	
	currentEnemy = act->enemyManager->GetCurrentEnemy();
	if ( !currentEnemy )
		return false;

	//Did this so I could check the f'ng return value
	bool attack = CanAttackTarget( currentEnemy );
	return attack;
	
	
}

bool CombatSubsystem::WeaponIsFireType( firetype_t fire_type )
{
	firetype_t weapon_fire_type;
	
	if ( !_activeWeapon.weapon )
		return false;
	
	weapon_fire_type = _activeWeapon.weapon->GetFireType( FIRE_MODE1 );
	
	if ( weapon_fire_type == fire_type )
		return true;
	
	
	return false;
}

//
// Name:        CanShootTarget()
// Parameters:  Entity *target
// Description: Checks if the Actor can shoot the target from its currentPosition
//
bool CombatSubsystem::CanAttackTarget( Entity *target )
{
	Vector startPos;
	
	if ( !target )
		return false;
	
	if ( act->CurrentAnim( legs ) < 0 ) 
		return false;

	if ( _activeWeapon.weapon && ( FT_MELEE != _activeWeapon.weapon->GetFireType( FIRE_MODE1 ) ))
		GetGunPositionData(&startPos);
	else
		startPos = act->centroid;
	
	bool attack = CanAttackTargetFrom( target , startPos );
	return attack;
	
}


//
// Name:        CanShootTargetFrom()
// Parameters:  Entity *target
//              Vector &startPos
// Description: Checks if the Actor can shoot the target from startPos
//
bool CombatSubsystem::CanAttackTargetFrom( Entity *target , const Vector &startPos )
{
	if ( (_activeWeapon.weapon && !IsTargetInWeaponRange(target)) || !target )
		return false;
	
	//Make sure target isn't too far "behind" us.
	Vector startToTarget = target->centroid - startPos;
	startToTarget.normalize();
	
	if ( _activeWeapon.weapon )
		{
		float dot;
		dot = DotProduct(startToTarget , act->movementSubsystem->getAnimDir() );

		if ( dot <= -.10 )
			return false;
		}


	//We don't want to check constantly
	if ( level.time < _nextTimeTracedToTarget )
		return _canShootTarget;
	
	_canShootTarget = _traceHitTarget( target , startPos );
	_nextTimeTracedToTarget = level.time + _traceInterval + G_Random();
	//_nextTimeTracedToTarget = 0.0f;
	
	return _canShootTarget;
}

//
// Name:        IsTargetInWeaponRange()
// Parameters:  Entity *target
// Description: Checks if the target is within the range of _activeWeapon
//
bool CombatSubsystem::IsTargetInWeaponRange( Entity *target )
{
	if ( !_activeWeapon.weapon )
		return false;
	
	Vector distance;
	distance = target->origin - act->origin;
	
	if ( distance.length() <= _activeWeapon.weapon->GetMaxRange() )
		return true;
	
	return false;
}


//
// Name:        SetTraceInterval()
// Parameters:  float interval
// Description: Sets _traceInterval
//
void CombatSubsystem::SetTraceInterval( const float interval )
{
	_traceInterval = interval;
}


//
// Name:        DoArchive()
// Parameters:  Archiver &arc
//              Actor *actor
// Description: Sets the Actor Pointer and Calls Archive()
//
void CombatSubsystem::DoArchive( Archiver &arc , Actor *actor )
{
	Archive( arc );
	if ( actor )
		act = actor;
	else
		gi.Error( ERR_FATAL, "MovementSubsystem::DoArchive -- actor is NULL" );	
	
}

void CombatSubsystem::FireWeapon()
{
	firemode_t  mode=FIRE_MODE1;
	mode = WeaponModeNameToNum("primary");
	
	if ( _activeWeapon.weapon )
		_activeWeapon.weapon->Fire( mode );
	
}

void CombatSubsystem::StopFireWeapon()
{
	
	if ( _activeWeapon.weapon )
	{
		if ( _activeWeapon.weapon->animate->HasAnim("endfire") )
			_activeWeapon.weapon->SetAnim( "endfire" );
		else
			_activeWeapon.weapon->ForceIdle();
	}
}

void CombatSubsystem::AimWeaponTag(const Vector &targetPos )
{
	Vector controllerAngles;
	Vector aimAngles;
	Vector barrelToEnemy;
	
	Vector gunPos, gunForward, gunRight, gunUp;
	
	//currentEnemy = act->enemyManager->GetCurrentEnemy();
	
	if ( !_activeWeapon.weapon )
		return;
	
	_activeWeapon.weapon->setOrigin();
	_activeWeapon.weapon->setAngles();
	
	_activeWeapon.weapon->SetControllerAngles( WEAPONBONE_BARREL_TAG, vec_zero );
	
	GetGunPositionData( &gunPos, &gunForward, &gunRight, &gunUp );
	

		
	//G_DebugLine( gunPos, targetPos, 1.0f, 0.0f, 1.0f, 1.0f );
	//G_DebugLine( gunPos, newTargetPos, 1.0f, 1.0f, 1.0f, 1.0f );

	
	Vector mypos, myforward, myleft, myup;
	_activeWeapon.weapon->GetTag( WEAPONBONE_BARREL_TAG, &mypos, &myforward, &myleft, &myup );
	
	Vector tagforwardPos = gunPos + (100.0f * myforward);
	Vector tagleftPos = gunPos + (100.0f * myleft);
	Vector tagupPos = gunPos + (100.0f * myup);
	//G_DebugLine( gunPos, tagforwardPos,	0.9f, 0.0f, 0.0f, 1.0f );
	//G_DebugLine( gunPos, tagleftPos,		0.0f, 0.9f, 0.0f, 1.0f );
	//G_DebugLine( gunPos, tagupPos,		0.0f, 0.0f, 0.9f, 1.0f );
	
	float gfToWorld[ 3 ][ 3 ];
	float worldToGf[ 3 ][ 3 ];
	gunForward.copyTo( gfToWorld[ 0 ] );
	gunRight.copyTo( gfToWorld[ 1 ] );
	gunUp.copyTo( gfToWorld[ 2 ] );
	
	TransposeMatrix( gfToWorld, worldToGf );
	
	Vector barrelToEnemyVector = targetPos - gunPos;
	vec3_t barrelToEnemyVec3_t;
	barrelToEnemyVector.copyTo( barrelToEnemyVec3_t );
	
	vec3_t barrelToEnemyTransformedVec3_t;
	MatrixTransformVector( barrelToEnemyVec3_t, worldToGf, barrelToEnemyTransformedVec3_t );
	Vector barrelToEnemyTransformed( barrelToEnemyTransformedVec3_t );
	barrelToEnemyTransformed.normalize();
	barrelToEnemyTransformed = barrelToEnemyTransformed.toAngles();
	barrelToEnemyTransformed.EulerNormalize();
	aimAngles = -barrelToEnemyTransformed;
	
	Vector aimUp, aimLeft, aimForward;
	float spreadX, spreadY;
	aimAngles.AngleVectors( &aimForward, &aimLeft, &aimUp );
	spreadX = GetDataForMyWeapon( "spreadx" );
	spreadY = GetDataForMyWeapon( "spready" );
	
	// figure the new projected impact point based upon computed spread
	if ( _activeWeapon.weapon->GetFireType(FIRE_MODE1) == FT_PROJECTILE ) 
		{
		// Apply Spread
		aimForward = ( aimForward * _activeWeapon.weapon->GetRange(FIRE_MODE1) ) + 
			( aimLeft * G_CRandom(spreadX) ) +
			( aimUp * G_CRandom(spreadY) );
		}
	
	// after figuring spread location, re-normalize vectors
	aimForward.normalize();
	//aimLeft = Vector::Cross(&aimForward,&aimUp);
	//aimUp = Vector::Cross(*aimLeft,*aimUp);	
	
	aimAngles = aimForward.toAngles();
	
	
	
	//   aimAngles = vec_zero;
	
	/*
	barrelToEnemy = targetPos - gunPos;
	barrelToEnemy.normalize();
	barrelToEnemy = barrelToEnemy.toAngles();
	barrelToEnemy.EulerNormalize();
	gunForward = gunForward.toAngles();
	gunForward.EulerNormalize();
	
	aimAngles = barrelToEnemy - gunForward;
	//aimAngles = gunForward - barrelToEnemy;
	  
	aimAngles.EulerNormalize();
	aimAngles = vec_zero;
	Vector oldAngles = _activeWeapon.weapon->GetControllerAngles( WEAPONBONE_BARREL_TAG );
	*/
	_activeWeapon.weapon->SetControllerTag( WEAPONBONE_BARREL_TAG, gi.Tag_NumForName( _activeWeapon.weapon->edict->s.modelindex, "tag_barrel" ) );
	_activeWeapon.weapon->SetControllerAngles( WEAPONBONE_BARREL_TAG, aimAngles );   
	
	//GetGunPositionData( &gunPos, &gunForward, &gunRight, &gunUp );
	
	//Draw Trace
	if ( g_showbullettrace->integer )
	{
		Vector test;
		GetGunPositionData( &gunPos , &gunForward );
		test = gunForward * 1000 + gunPos;
		
		G_DebugLine( gunPos, test, 1.0f, 0.0f, 0.0f, 1.0f );
		
		Vector barrelForward;
		Vector barrelStart;
		aimAngles.AngleVectors ( &barrelForward );
		barrelStart = barrelForward * 1000 + gunPos;
		//G_DebugLine( gunPos, barrelStart, 1.0f, 0.0f, 0.0f, 1.0f );
		
		//G_DebugLine( gunPos, targetPos, 1.0f, 1.0f, 0.0f, 1.0f );
	}		
}

void CombatSubsystem::AimWeaponTag(Entity *target)
{
	str targetBone = target->getTargetPos();
	Vector targetPos = target->centroid;
	
	if ( targetBone.length() )
	{
		if ( gi.Tag_NumForName( target->edict->s.modelindex , targetBone ) > 0 )
		{
			target->GetTag( targetBone.c_str() , &targetPos , NULL , NULL , NULL );
		}		
	}
	
	Vector newTargetPos = targetPos;

	if ( !_activeWeapon.weapon ) return;

	if ( _activeWeapon.weapon->GetFireType(FIRE_MODE1) == FT_PROJECTILE )
		 newTargetPos = GetLeadingTargetPos(_activeWeapon.weapon->GetProjectileSpeed() , targetPos , target );

	AimWeaponTag(newTargetPos);
}

void CombatSubsystem::ClearAim()
{
	_activeWeapon.weapon->SetControllerTag( WEAPONBONE_BARREL_TAG, gi.Tag_NumForName( _activeWeapon.weapon->edict->s.modelindex, "tag_barrel" ) );
	_activeWeapon.weapon->SetControllerAngles( WEAPONBONE_BARREL_TAG, vec_zero );   
}

//
// Name:        Archive()
// Parameters:  Archiver &arc
// Description: Archives Class Data
//
void CombatSubsystem::Archive(Archiver &arc )
{
	_activeWeapon.Archive( arc );
	arc.ArchiveFloat      ( &_nextTimeTracedToTarget );
	arc.ArchiveFloat      ( &_traceInterval          );
	arc.ArchiveBool       ( &_canShootTarget         );

	arc.ArchiveFloat( &_yawDiff );
}


//
// Name:        GetGunPosition()
// Parameters:  None
// Description: Gets the GunBarrel Position
//
void CombatSubsystem::GetGunPositionData(Vector *pos , Vector *forward , Vector *right , Vector *up)
{
	int tag_num;
	
	if ( !_activeWeapon.weapon )
		gi.Error( ERR_FATAL, "Actor has no activeweapon" );
	
	tag_num = gi.Tag_NumForName( _activeWeapon.weapon->edict->s.modelindex, "tag_barrel" );
	if ( tag_num < 0 )
		gi.Error( ERR_FATAL, "Weapon has no tag_barrel" );
	
	_activeWeapon.weapon->GetActorMuzzlePosition( pos, forward, right, up );
	
}

float CombatSubsystem::GetAimGunYaw( const Vector &target )
{  
	Vector GunPos, GunForward, GunRight, GunUp;
	Vector GunPosToOrigin , OriginToGunPos, OriginToTarget;      
	Vector TagForwardAngles, GunToOriginAngles, OriginToGunAngles, OriginToTargetAngles;
	
	float AngleDiff;
	float yaw;   
	
	// Vars for calculating the yaw... 
	float a , r , d;
	
	
	// Get our Gun Data
	GetGunPositionData( &GunPos, &GunForward, &GunRight, &GunUp );
	
	
	// Get Vectors to Gun and To Target;
	GunPosToOrigin = act->origin - GunPos;
	OriginToGunPos = GunPos - act->origin;
	OriginToTarget = target - act->origin;
	
	// Zero out the Z
	GunPosToOrigin.z = 0.0f;
	OriginToTarget.z = 0.0f;
	OriginToGunPos.z = 0.0f;
	
	// Get Lengths
	a = GunPosToOrigin.length();
	d = OriginToTarget.length();
	
	// Get Angle Difference
	TagForwardAngles = GunForward.toAngles();
	GunToOriginAngles = GunPosToOrigin.toAngles();
	OriginToGunAngles = OriginToGunPos.toAngles();
	OriginToTargetAngles = OriginToTarget.toAngles();
	
	AngleDiff = AngleNormalize180( GunToOriginAngles[YAW] - TagForwardAngles[YAW] );
	AngleDiff = 180.0f - AngleDiff;
	r = a * sin( DEG2RAD(AngleDiff) );
	
	float angleA;
	angleA = RAD2DEG(acos ( r / d ));
	
	float angleB;
	angleB = RAD2DEG(acos ( r / a ));
	
	float angleC;
	angleC = AngleNormalize180( OriginToTargetAngles[YAW] - OriginToGunAngles[YAW] );
	
	float totalAngle;
	totalAngle = angleB + angleC;
	
	float finalAngle;
	finalAngle = totalAngle - angleA;
	
	yaw = AngleNormalize180(finalAngle);
	
	return yaw;
	
}


float CombatSubsystem::GetAimGunPitch( const Vector &target )
{  
	Vector GunPos, GunForward, GunRight, GunUp , TagToTarget;
	Vector TagForwardAngles, TagToTargetAngles;
	
	// Get our Gun Data
	GetGunPositionData( &GunPos, &GunForward, &GunRight, &GunUp );
	TagToTarget = target - GunPos;
	
	
	// Get Angle Difference
	TagForwardAngles = GunForward.toAngles();
	TagToTargetAngles = TagToTarget.toAngles();
	
	return AngleNormalize180(TagToTargetAngles[PITCH] - TagForwardAngles[PITCH]);
	
}

//
// Name:        _traceHitTarget()
// Parameters:  Entity *target
//              const Vector &startPos
// Description: Checks of the a trace hits the target
//
bool CombatSubsystem::_traceHitTarget(Entity *target , const Vector &startPos )
{
	Entity *t;
	int mask;
	trace_t trace;
	Vector targetPos;
	str targetBone;
	
	mask = MASK_SHOT;
	
	targetBone = target->getTargetPos();
	
	targetPos = target->centroid;
	
	if ( targetBone.length() )
	{
		if ( gi.Tag_NumForName( target->edict->s.modelindex , targetBone ) > 0 )
		{
			target->GetTag( targetBone.c_str() , &targetPos , NULL , NULL , NULL );
		}		
	}
	
	Vector attackDir = targetPos - startPos;
	float attackDirLength = attackDir.length();
	attackDir.normalize();
	attackDir *= attackDirLength + ( 32.0f );
	attackDir += startPos;
	targetPos = attackDir;
	
	trace = G_FullTrace( startPos, vec_zero, vec_zero, targetPos, act, mask, false, "CombatSubsystem::_traceHitTarget" );
	//G_DebugLine( startPos, targetPos, .5f, 1.0f, .5f, 1.0f );
	
	if ( trace.startsolid )
	{
		if ( trace.ent == target->edict )
			return true;

		return false;
	}
		

	/*if ( trace.startsolid )
	{
		// If we hit the guy we wanted, then shoot
		if ( trace.ent == target->edict )
			return true;
		
		trace = G_FullTrace( act->centroid, vec_zero, vec_zero, targetPos, act, mask, false, "CombatSubsystem::_traceHitTarget" );
		
		if ( trace.startsolid )
			return false;
	}*/
	
	
	// see if we hit anything at all
	if ( !trace.ent )
		return false;
	
	
	// If we hit the guy we wanted, then shoot
	if ( trace.ent == target->edict )
		return true;
	
	
	// If we hit someone else we don't like, then shoot
	t = trace.ent->entity;
	if ( act->enemyManager->IsValidEnemy( t ) )
	{
		act->enemyManager->SetCurrentEnemy( trace.ent->entity );		
		// We want to return false though, so we don't add
		// the attempted enemy to the hatelist
		return false;
	}
	
	
	
	// if we hit something breakable, check if shooting it will
	// let us shoot someone.
	if ( t->isSubclassOf( Object ) || t->isSubclassOf( ScriptModel ) )
	{
		trace = G_Trace( Vector( trace.endpos ), vec_zero, vec_zero, target->centroid, t, mask, false, "CombatSubsystem::_traceHitTarget 2" );
		if ( trace.startsolid )
			return false;
		
		// see if we hit anything at all
		if ( !trace.ent )
			return false;
		
		// If we hit the guy we wanted, then shoot
		if ( trace.ent == target->edict )
			return true;
		
		// If we hit someone else we don't like, then shoot
		if ( act->enemyManager->IsValidEnemy( trace.ent->entity ) )
			return true;
		
		// Forget it then
		return false;
	}
	
	return false;
	
}

//
// Name:        _init()
// Parameters:  None
// Description: Initializes Class Data
//
void CombatSubsystem::_init()
{
	_activeWeapon.weapon = NULL;
	_nextTimeTracedToTarget = 0;
	_traceInterval          = DEFAULT_TRACE_INTERVAL;
	_canShootTarget         = false;
	_yawDiff = 0.0f;
}

//--------------------------------------------------------------
// Name:		GetBestAvailableWeapon()
// Class:		CombatSubsystem
//
// Description:	Iterates through the actor's inventory list and
//				checks the power rating ( modified by range and armor )
//				of each weapon, returning the best one.
//
// Parameters:	Entity *target -- The target we are trying to shoot
//
// Returns:		None
//--------------------------------------------------------------
WeaponPtr CombatSubsystem::GetBestAvailableWeapon( Entity *target )
{
	Item   *item;
	Weapon *weapon;
	Weapon *bestWeapon;
	float bestPowerRating;
	float powerRating;
	
	item			= NULL;
	weapon			= NULL;
	bestWeapon		= NULL;
	bestPowerRating = 0.0f;
	
	// Try and grab the first item.  If you pass a null into the
	// NextItem() it will attempt to grab the first item in the 
	// inventory list
	item = act->NextItem(item);
	
	// Loop as long as we have inventory items to check
	while ( item )
	{
		if ( item->isSubclassOf(Weapon) )
		{
			weapon = (Weapon*)(Item*)item;
			if ( weapon )
			{
				powerRating = getModifiedPowerRating( target, weapon );
				if ( powerRating > bestPowerRating )
				{
					bestPowerRating = powerRating;
					bestWeapon = weapon;
				}			
			}
		}
		
		item = act->NextItem(item);
	}
	
	return bestWeapon;
}


//--------------------------------------------------------------
// Name:		getModifiedPowerRating()
// Class:		CombatSubsystem
//
// Description:	Returns the powerRating of the weapon modified by
//				such conditions as Armor, Range, Spread, and speed
//              of the projectile
//
// Parameters:	Entity *target -- Target we're trying to shoot
//				Weapon *weapon -- Weapon we're trying to shoot with
//
// Returns:		weaponPowerRating
//--------------------------------------------------------------
float CombatSubsystem::getModifiedPowerRating( Entity *target, Weapon *weapon )
{
	float rangeToTarget;
	float weaponPowerRating;
	float weaponRange;
	float weaponSpread;
	float hitPercentage;
	float skillLevel;
	Actor *actTarget;
	Vector selfToTarget;
	
	// If we don't have a target we can't shoot it
	if ( !target )
	{
		//Assert to let us know this occurred
		assert ( target != NULL );
		return 0.0f;
	}
	
	// Set our default values
	weaponPowerRating = weapon->GetPowerRating();
	weaponRange       = weapon->GetRange( FIRE_MODE1 );
	
	// Calculate our distance to the target
	selfToTarget = target->origin - act->origin;
	rangeToTarget = selfToTarget.length();
	
	
	// If we are out of range, we can't shoot target
	if ( rangeToTarget > weaponRange )
		return 0.0f;
	
	// Check if we can even hurt the target with this weapon
	if ( target->isSubclassOf(Actor) )
	{
		actTarget = (Actor*)target;			
		if ( !actTarget->canBeDamagedBy( weapon->GetMeansOfDeath(FIRE_MODE1) ) )
			return 0.0f;
	}
	
	//Calculate our hit %
	hitPercentage = 0.0;	
	if ( (weapon->flags |= FT_BULLET) || (weapon->flags |= FT_MELEE) )
	{
		weaponSpread = weapon->GetBulletSpreadX(FIRE_MODE1);
		if ( weaponSpread < 1.0 )
			weaponSpread = 1.0f;
		
		hitPercentage = rangeToTarget/weaponRange * weaponSpread;
	}
	else
	{
		hitPercentage = weapon->GetProjectileSpeed() / rangeToTarget;
	}
	
	// Get our skill level with this weapon -- Should be a number between 0.0 and 1.0
	// all skill levels default to 1.0 so it won't affect anything directly
	skillLevel = weapon->GetSkillLevel();
	
	return hitPercentage * weaponPowerRating * skillLevel;
}

//--------------------------------------------------------------
// Name:		GetActiveWeaponPowerRating()
// Class:		CombatSubsystem
//
// Description:	Returns the power rating of the ActiveWeapon
//
// Parameters:	Entity *target
//
// Returns:		float
//--------------------------------------------------------------
float CombatSubsystem::GetActiveWeaponPowerRating( Entity *target )
{
	if ( _activeWeapon.weapon )
		return getModifiedPowerRating( target, _activeWeapon.weapon );

	return 0.0f;
}

str CombatSubsystem::GetActiveWeaponName()
{
	str name;

	if ( _activeWeapon.weapon )
		name =  _activeWeapon.weapon->getName();

	return name;
}

str CombatSubsystem::GetActiveWeaponArchetype()
{
	str atype;

	if ( _activeWeapon.weapon )
		atype = _activeWeapon.weapon->getArchetype();

	return atype;
}

bool CombatSubsystem::GetProjectileLaunchAngles( Vector &launchAngles, const Vector &launchPoint, const float initialSpeed, const float gravity , const bool useHighTrajectory ) const
{
	Entity const *target = act->enemyManager->GetCurrentEnemy();
	
	if ( target )
	{
		const Vector targetPoint( target->centroid );
		
		Trajectory projectileTrajectory( launchPoint, targetPoint, initialSpeed, gravity * -sv_currentGravity->value , useHighTrajectory );
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

bool CombatSubsystem::shouldArcProjectile()
{
	if ( _activeWeapon.weapon )
		return _activeWeapon.weapon->shouldArcProjectile();

	return false;
}

float CombatSubsystem::GetLowArcRange()
{
	if ( _activeWeapon.weapon )
		return _activeWeapon.weapon->GetLowArcRange();

	return 0.0;
}

const str CombatSubsystem::GetAnimForMyWeapon(const str &property )
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(act->getArchetype()) )
		return "";
		
	str objname = act->combatSubsystem->GetActiveWeaponArchetype();	
	objname = "Hold" + objname;

	if ( gpm->hasProperty(objname, property ) )
		return gpm->getStringValue( objname , property );
	
	return "";
}

float CombatSubsystem::GetDataForMyWeapon( const str &property )
{
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm->hasObject(act->getArchetype()) )
		return 0.0;
		
	str objname = act->combatSubsystem->GetActiveWeaponArchetype();	
	objname = "Hold" + objname;

	if ( gpm->hasProperty(objname, property ) )
		return gpm->getFloatValue( objname , property );
	
	return 0.0;
}

void CombatSubsystem::OverrideSpread( float spreadX , float spreadY )
{
	if ( !_activeWeapon.weapon ) return;

	_activeWeapon.weapon->SetBulletSpread( spreadX , spreadY );
}

Vector CombatSubsystem::GetLeadingTargetPos( float projSpeed , Vector originalTargetPos , Entity *target )
{
	Vector newTargetPos = originalTargetPos;

	if ( !target )
		return newTargetPos;

	if ( target->groundentity ) 
		{

		// Here we're going to try and lead the target;
		Vector targetVelocity = target->velocity;
		if ( projSpeed <= 0 )
			projSpeed = 1;

		Vector selfToTarget = originalTargetPos - act->centroid;
		float distToTarget = selfToTarget.length();
		float timeToTarget = distToTarget/projSpeed;
		Vector enemyMoveDir = targetVelocity;
		float targetSpeed = targetVelocity.length();

		enemyMoveDir.normalize();

		// Parameterize aim leading over the interval [minLeadFactor,maxLeadFactor]
		//	such that 0 is targetPos and 1 is newTargetPos.
		// 
		// Select a random parameter value in that range and interpolate
		//	between the two positions.
		// Essentially, the shot will aim at some random point between
		//	where the target is now and where it (presumably) will be
		//	given the lead distance.
		// 
		// When <leadFraction> is 0, shots aim at the current position
		//	of the target.  When <leadFracation> is 1, shots aim at
		//	the best-guess lead-position of the target.  When <leadFraction>
		//	is greater than 1, shots will over-lead the player.  Note
		//	that in the latter case shots may actually hit the player
		//	if he is circle-strafing or moving closer/farther in addition
		//	to his lateral movement.
		// 
		float leadFraction = act->minLeadFactor + G_Random( act->maxLeadFactor - act->minLeadFactor );
		Vector leadVector = enemyMoveDir * targetSpeed * timeToTarget;
		newTargetPos = originalTargetPos + (leadFraction * leadVector);
		}
	

	return newTargetPos;
}
