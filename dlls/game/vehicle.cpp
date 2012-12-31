//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/vehicle.cpp                               $
// $Revision:: 34                                                             $
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
// Script controlled Vehicles.
//

#include "_pch_cpp.h"
#include "scriptslave.h"
#include "vehicle.h"
#include "player.h"
#include "specialfx.h"
#include "explosion.h"
#include "earthquake.h"
#include "gibs.h"
#include "player.h"

Event EV_Vehicle_Start
(
	"start",
	EV_DEFAULT,
	NULL,
	NULL,
	"Initialize the vehicle."
);
Event EV_Vehicle_Enter
(
	"enter",
	EV_CODEONLY,
	"eS",
	"vehicle driver_anim",
	"Called when someone gets into a vehicle."
);
Event EV_Vehicle_Exit
(
	"exit",
	EV_DEFAULT,
	"e",
	"vehicle",
	"Called when driver gets out of the vehicle."
);
Event EV_Vehicle_Drivable
(
	"drivable",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make the vehicle drivable"
);
Event EV_Vehicle_UnDrivable
(
	"undriveable",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make the vehicle undrivable"
);
Event EV_Vehicle_Jumpable
(
	"canjump",
	EV_DEFAULT,
	"b",
	"jumpable",
	"Sets whether or not the vehicle can jump"
);
Event EV_Vehicle_Lock
(
	"lock",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the vehicle to be locked"
);
Event EV_Vehicle_UnLock
(
	"unlock",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the vehicle to be unlocked"
);
Event EV_Vehicle_SeatAnglesOffset
(
	"seatanglesoffset",
	EV_DEFAULT,
	"v",
	"angles",
	"Set the angles offset of the seat"
);
Event EV_Vehicle_SeatOffset
(
	"seatoffset",
	EV_DEFAULT,
	"v",
	"offset",
	"Set the offset of the seat"
);
Event EV_Vehicle_DriverAnimation
(
	"driveranim",
	EV_DEFAULT,
	"s",
	"animation",
	"Set the animation of the driver to use when the driver is in the vehicle"
);
Event EV_Vehicle_SetWeapon
(
	"setweapon",
	EV_DEFAULT,
	"s",
	"weaponname",
	"Set the weapon for the vehicle"
);
Event EV_Vehicle_SetSpeed
(
	"vehiclespeed",
	EV_DEFAULT,
	"f",
	"speed",
	"Set the speed of the vehicle"
);
Event EV_Vehicle_SetTurnRate
(
	"turnrate",
	EV_DEFAULT,
	"f",
	"rate",
	"Set the turning rate of the vehicle"
);
Event EV_Vehicle_SteerInPlace
(
	"steerinplace",
	EV_DEFAULT,
	NULL,
	NULL,
	"Set the vehicle to turn in place"
);
Event EV_Vehicle_ShowWeapon
(
	"showweapon",
	EV_DEFAULT,
	NULL,
	NULL,
	"Set the weapon to be show in the view"
);
Event EV_Vehicle_RestrictPitch
(
	"restrictpitch",
	EV_DEFAULT,
	"f",
	"pitchDelta",
	"The max and minimum pitch of the driver"
);
Event EV_Vehicle_RestrictRotation
(
	"restrictrotation",
	EV_DEFAULT,
	"f",
	"rotateDelta",
	"The max and min rotation of the driver"
);
Event EV_Vehicle_NoPrediction
(
	"noprediction",
	EV_DEFAULT,
	"b",
	"bool",
	"Turns no prediction on or off"
);

Event EV_Vehicle_DisableInventory
(
	"disableinventory",
	EV_DEFAULT,
	NULL,
	NULL,
	"Disables the inventory when the player uses this vehicle"
);

extern Event EV_Player_DisableUseWeapon;
extern Event EV_Player_PutawayWeapon;

CLASS_DECLARATION( ScriptModel, VehicleBase, NULL )
{
	{ NULL, NULL }
};

VehicleBase::VehicleBase()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	takedamage = DAMAGE_NO;
	showModel();
	setSolidType( SOLID_NOT );
	setMoveType( MOVETYPE_NONE );
	setOrigin( GetLocalOrigin() + Vector( "0 0 30") );
	
	//
	// we want the bounds of this model auto-rotated
	//
	flags |= FL_ROTATEDBOUNDS;
	
	//
	// rotate the mins and maxs for the model
	//
	setSize( mins, maxs );
	
	vlink = NULL;
	offset = Vector(0, 0, 0);
	
	PostEvent( EV_BecomeNonSolid, EV_POSTSPAWN );
}

CLASS_DECLARATION( VehicleBase, BackWheels, "script_wheelsback" )
{
	{ NULL, NULL }
};

CLASS_DECLARATION( VehicleBase, FrontWheels, "script_wheelsfront" )
{
	{ NULL, NULL }
};

CLASS_DECLARATION( VehicleBase, Vehicle, "script_vehicle" )
{
	{ &EV_Blocked,					    &Vehicle::VehicleBlocked	},
	{ &EV_Touch,					    &Vehicle::VehicleTouched	},
	{ &EV_Use,						    &Vehicle::DriverUse		},
	{ &EV_Vehicle_Start,			    &Vehicle::VehicleStart	},
	{ &EV_Vehicle_Drivable,		        &Vehicle::Drivable		},
	{ &EV_Vehicle_UnDrivable,	        &Vehicle::UnDrivable		},
	{ &EV_Vehicle_Jumpable,	            &Vehicle::Jumpable		},
	{ &EV_Vehicle_SeatAnglesOffset,     &Vehicle::SeatAnglesOffset},
	{ &EV_Vehicle_SeatOffset,           &Vehicle::SeatOffset		},
	{ &EV_Vehicle_Lock,		            &Vehicle::Lock			},
	{ &EV_Vehicle_UnLock,	            &Vehicle::UnLock			},
	{ &EV_Vehicle_SetWeapon,            &Vehicle::SetWeapon		},
	{ &EV_Vehicle_DriverAnimation,      &Vehicle::DriverAnimation },
	{ &EV_Vehicle_SetSpeed,             &Vehicle::SetSpeed		},
	{ &EV_Vehicle_SetTurnRate,          &Vehicle::SetTurnRate		},
	{ &EV_Vehicle_SteerInPlace,         &Vehicle::SteerInPlace	},
	{ &EV_Vehicle_ShowWeapon,           &Vehicle::ShowWeaponEvent },
	{ &EV_Vehicle_RestrictPitch,		&Vehicle::RestrictPitch	},
	{ &EV_Vehicle_RestrictRotation,		&Vehicle::RestrictRotation},
	{ &EV_Vehicle_NoPrediction,			&Vehicle::SetNoPrediction },
	{ &EV_Vehicle_DisableInventory,		&Vehicle::DisableInventory},
	{ NULL, NULL }
};

Vehicle::Vehicle()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	takedamage = DAMAGE_YES;
	seatangles = vec_zero;
	driveroffset = vec_zero;
	seatoffset = vec_zero;
	driver = 0;
	lastdriver = 0;
	currentspeed = 0;
	turnangle = 0;
	turnimpulse = 0;
	moveimpulse = 0;
	jumpimpulse = 0;
	conesize = 75;
	hasweapon = false;
	locked = false;
	steerinplace = false;
	drivable = true;
	jumpable = false;
	showweapon = false;
	flags |= FL_DIE_EXPLODE;
	// touch triggers by default
	flags |= FL_TOUCH_TRIGGERS;
	gravity = 1;
	mass = size.length() * 10.0f;
	
	health      = 1000;
	speed       = 1200;
	maxturnrate = 40.0f;
	
	usetime     = 0.0f;
	
	maxtracedist = 0;
	
	
	_restrictPitch = false;
	_restrictYaw = false;
	_noPrediction = false;
	
	_disableInventory = false;

	PostEvent( EV_Vehicle_Start, FRAMETIME );
}

void Vehicle::VehicleStart( Event * )
{
	Entity *ent;
	VehicleBase *last;
	Vector   drivemins, drivemaxs;
	float    max;
	float    width,height;
	orientation_t orn;
	
	// become solid
	setSolidType( SOLID_BBOX );
	
	last = this;
	
	for( ent = G_NextEntity( NULL ); ent != NULL; ent = G_NextEntity( ent ) )
	{
		if ( ( ent != this ) && ( ent->isSubclassOf( VehicleBase ) ) )
		{
			if ( ( ent->absmax.x >= absmin.x ) && ( ent->absmax.y >= absmin.y ) && ( ent->absmax.z >= absmin.z ) &&
				( ent->absmin.x <= absmax.x ) && ( ent->absmin.y <= absmax.y ) && ( ent->absmin.z <= absmax.z ) )
			{
				last->vlink = ( VehicleBase * )ent;
				last = ( VehicleBase * )ent;
				last->offset = last->origin - origin;
				last->offset = getLocalVector( last->offset );
				last->edict->s.scale *= edict->s.scale;
			}
		}
	}
	
	last->vlink = NULL;
	
	//
	// get the seat offset
	//
	if ( GetRawTag( "tag_rider", &orn ) )
	{
		driveroffset = Vector( orn.origin );
	}
	driveroffset += seatoffset * edict->s.scale;
	
	SetDriverAngles( angles + seatangles );
	
	
	
	max_health = health;
	
	//
	// calculate drive mins and maxs
	//
	max = 0;
	if ( fabs( mins[ 0 ] ) > max )
		max = fabs( mins[ 0 ] );
	if ( fabs( maxs[ 0 ] ) > max )
		max = fabs( maxs[ 0 ] );
	if ( fabs( mins[ 1 ] ) > max )
		max = fabs( mins[ 1 ] );
	if ( fabs( maxs[ 1 ] ) > max )
		max = fabs( maxs[ 1 ] );
	drivemins = Vector( -max, -max, mins[ 2 ] ) * edict->s.scale;
	drivemaxs = Vector( max, max, maxs[ 2 ] ) * edict->s.scale;
	
	width = maxs[ 1 ] - mins[ 1 ];
	height = maxs[ 0 ] - mins[ 0 ];
	
	maxtracedist = height;
	
	Corners[ 0 ][ 0 ] = -( width / 4.0f );
	Corners[ 0 ][ 1 ] = ( height / 4.0f );
	Corners[ 0 ][ 2 ] = 0.0f;
	
	Corners[ 1 ][ 0 ] = ( width / 4.0f );
	Corners[ 1 ][ 1 ] = ( height / 4.0f );
	Corners[ 1 ][ 2 ] = 0.0f;
	
	Corners[ 2 ][ 0 ] = -( width / 4.0f );
	Corners[ 2 ][ 1 ] = -( height / 4.0f );
	Corners[ 2 ][ 2 ] = 0.0f;
	
	Corners[ 3 ][ 0 ] = ( width / 4.0f );
	Corners[ 3 ][ 1 ] = -( height / 4.0f );
	Corners[ 3 ][ 2 ] = 0.0f;
	if ( drivable )
	{
		// drop everything back to the floor
		//droptofloor( 250.0f );
		Postthink();
	}
	last_origin = origin;
	setSize( drivemins, drivemaxs );
}

void Vehicle::Drivable( Event * )
{
	setMoveType( MOVETYPE_NONE );
	drivable = true;
}

void Vehicle::UnDrivable( Event * )
{
	setMoveType( MOVETYPE_PUSH );
	drivable = false;
}

void Vehicle::Jumpable( Event * )
{
	jumpable = true;
}

void Vehicle::Lock( Event * )
{
	locked = true;
}

void Vehicle::UnLock( Event * )
{
	locked = false;
}

void Vehicle::SteerInPlace( Event * )
{
	steerinplace = true;
}

void Vehicle::SeatAnglesOffset( Event *ev )
{
	seatangles = ev->GetVector( 1 );
}

void Vehicle::SeatOffset( Event *ev )
{
	seatoffset = ev->GetVector( 1 );
}

void Vehicle::SetWeapon( Event *ev )
{
	showweapon = true;
	hasweapon = true;
	weaponName = ev->GetString( 1 );
}

void Vehicle::ShowWeaponEvent( Event * )
{
	showweapon = true;
}

//-----------------------------------------------------
//
// Name:		RestrictPitch
// Class:		Vehicle
//
// Description:	Restricts the drivers pitch while in the vehicle. This calculates the pitchSeam. 
//				A seam is the location where the normalized angle range boundary. For example
//				on a normalized range of -180, 180, the seam is located at -180/180. To solve the seam
//				problem, the seam is always positioned directly (-180 degrees) behind the starting pitch.
//
// Parameters:	event - event containing the pitch restrictions
//
// Returns:		None
//-----------------------------------------------------
void Vehicle::RestrictPitch(Event* event)
{
	_startPitch = angles[PITCH];
	_pitchSeam = _startPitch - 180.0f;

	_minimumPitch = AngleNormalizeArbitrary( _startPitch - event->GetFloat(1), _pitchSeam);
	_maximumPitch = AngleNormalizeArbitrary( _startPitch + event->GetFloat(1), _pitchSeam);

	_restrictPitch = true;
}


//-----------------------------------------------------
//
// Name:		RestrictRotation
// Class:		Vehicle
//
// Description:	Restricts the drivers rotation while in the vehicle.This calculates the yawSeam. 
//				A seam is the location where the normalized angle range boundary. For example
//				on a normalized range of -180, 180, the seam is located at -180/180. To solve the seam
//				problem, the seam is always positioned directly (-180 degrees) behind the starting yaw.
//
// Parameters:	event - the event containing the rotation restrictions
//
// Returns:		None
//-----------------------------------------------------
void Vehicle::RestrictRotation(Event* event)
{
	_startYaw = angles[YAW];
	_yawSeam = _startYaw - 180.0f;
	
	_minimumYaw = AngleNormalizeArbitrary( _startYaw - event->GetFloat(1), _yawSeam);
	_maximumYaw = AngleNormalizeArbitrary( _startYaw + event->GetFloat(1), _yawSeam);

	_restrictYaw = true;
}


//-----------------------------------------------------
//
// Name:		SetNoPrediction
// Class:		Vehicle
//
// Description:	Turns the no prediction flag
//
// Parameters:	event - the event that turns no prediction on or off
//
// Returns:		None
//-----------------------------------------------------
void Vehicle::SetNoPrediction(Event* event)
{
	_noPrediction = event->GetBoolean(1);
}


void Vehicle::DisableInventory(Event* )
{
	_disableInventory = true;
}


void Vehicle::DriverAnimation( Event *ev )
{
	driveranim = ev->GetString( 1 );
}

qboolean Vehicle::HasWeapon( void )
{
	return hasweapon;
}

qboolean Vehicle::ShowWeapon( void )
{
	return showweapon;
}

void Vehicle::SetDriverAngles( const Vector &angles )
{
	int i;
	
	if ( !driver )
		return;
	
	for( i = 0; i < 3; i++ )
	{
		driver->client->ps.delta_angles[ i ] = ANGLE2SHORT( angles[ i ] - driver->client->cmd_angles[ i ] );
	}
}

void Vehicle::HandleEvent( Event * )
{

}

/*
=============
CheckWater
=============
*/
void Vehicle::CheckWater( void )
{
	Vector  point;
	int	  cont;
	int	  sample1;
	int	  sample2;
	VehicleBase *v;
	
	unlink();
	v = this;
	while( v->vlink )
	{
		v = v->vlink;
		v->unlink();
	}
	
	if ( driver )
	{
		driver->unlink();
	}
	
	//
	// get waterlevel
	//
	waterlevel = 0;
	watertype = 0;
	
	sample2 = maxs[ 2 ] - mins[ 2 ];
	sample1 = sample2 / 2;
	
	point = origin;
	point[ 2 ] += mins[ 2 ];
	cont = gi.pointcontents( point, 0 );
	
	if ( cont & MASK_WATER )
	{
		watertype = cont;
		waterlevel = 1;
		point[ 2 ] = origin[ 2 ] + mins[ 2 ] + sample1;
		cont = gi.pointcontents( point, 0 );
		if ( cont & MASK_WATER )
		{
			waterlevel = 2;
			point[ 2 ] = origin[ 2 ] + mins[ 2 ] + sample2;
			cont = gi.pointcontents( point, 0 );
			if ( cont & MASK_WATER )
            {
				waterlevel = 3;
            }
		}
	}
	
	link();
	v = this;
	while( v->vlink )
	{
		v = v->vlink;
		v->link();
	}
	
	if ( driver )
	{
		driver->link();
		driver->waterlevel = waterlevel;
		driver->watertype = watertype;
	}
}

/*
=============
WorldEffects
=============
*/
void Vehicle::WorldEffects( void )
{
	//
	// Check for earthquakes
	//
	/* if ( groundentity && ( level.earthquake > level.time ) )
	{
		velocity += Vector
			(
			level.earthquake_magnitude * EARTHQUAKE_STRENGTH * G_CRandom(),
			level.earthquake_magnitude * EARTHQUAKE_STRENGTH * G_CRandom(),
			level.earthquake_magnitude * 1.5f * G_Random()
			);
	} */
	
	//
	// check for lava
	//
	if ( watertype & CONTENTS_LAVA )
	{
		Damage( world, world, 20.0f * waterlevel, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_LAVA );
	}
}

void Vehicle::DriverUse( Event *ev )
{
	Event *event;
	Entity *other;
	
	other = ev->GetEntity( 1 );
	if ( level.time < ( usetime + 1.0f ) )
		return;
	else
		usetime = level.time;
	
	if ( !other || !other->isSubclassOf( Sentient ) )
	{
		return;
	}
	
	if ( driver )
	{
//		int height;
//		int ang;
		Vector angles;
		Vector forward;
		Vector pos;
//		float ofs;
//		trace_t trace;
		
		if ( other != driver )
		{
			return;
		}
		
		if ( locked )
			return;
		/*
		//
		// place the driver on the ground
		//
		ofs = size.length() * 0.5f;
		for ( height = 0; height < 100; height += 16 )
		{
			for ( ang = 0; ang < 360; ang += 30 )
            {
				angles[ 1 ] = driver->angles[ 1 ] + ang + 90.0f;
				angles.AngleVectors( &forward, NULL, NULL );
				pos = origin + (forward * ofs);
				pos[2] += height;
				trace = G_Trace( pos, driver->mins, driver->maxs, pos, NULL, MASK_PLAYERSOLID, false, "Vehicle::DriverUse 1" );
				if ( !trace.startsolid && !trace.allsolid )
				{
					Vector end;
					
					end = pos;
					end[ 2 ] -= 128.0f;
					trace = G_Trace( pos, driver->mins, driver->maxs, end, NULL, MASK_PLAYERSOLID, false, "Vehicle::DriverUse 2" );
					if ( trace.fraction < 1.0f )
					{
						driver->setOrigin( pos );
						goto foundpos;
					}
				}
            }
		}
		*/

//		return;
		
//foundpos:
		
		turnimpulse = 0;
		moveimpulse = 0;
		jumpimpulse = 0;
		
		event = new Event( EV_Vehicle_Exit );
		event->AddEntity( this );
		driver->ProcessEvent( event );

		if ( drivable )
		{
			StopLoopSound();
			Sound( "snd_dooropen", CHAN_BODY );
			Sound( "snd_stop", CHAN_VOICE );
			driver->setSolidType( SOLID_BBOX );
		}

		driver->setOrigin(_oldOrigin);

		enableDriverInventory();

		if( hasweapon )
		{
			driver->DeactivateWeapon( WEAPON_DUAL );
			driver->takeItem( weaponName.c_str() );
		}

		driver = NULL;

	}
	else
	{
		driver = ( Sentient * )other;
		VectorCopy(driver->edict->client->ps.origin, _oldOrigin);
		
		lastdriver = driver;
		
		
		if ( drivable )
			setMoveType( MOVETYPE_VEHICLE );

		if ( hasweapon )
		{
			driver->giveItem( weaponName.c_str() );
			driver->useWeapon(weaponName.c_str(), WEAPON_DUAL);
		}

		disableDriverInventory();
		
		if ( drivable )
		{
			Sound( "snd_doorclose", CHAN_BODY );
			Sound( "snd_start", CHAN_VOICE );
			driver->setSolidType( SOLID_NOT );
		}
		
		event = new Event( EV_Vehicle_Enter );
		event->AddEntity( this );
		if ( driveranim.length() )
			event->AddString( driveranim );
		driver->ProcessEvent( event );
		
		offset = other->origin - origin;
		
		flags	|= FL_POSTTHINK;
		SetDriverAngles( angles + seatangles );
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
void Vehicle::disableDriverInventory( void )
{
	if( _disableInventory == false )
		return;

	if(driver != 0 && driver->isSubclassOf(Player))
	{
		((Player*)driver.Pointer())->disableInventory();
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
void Vehicle::enableDriverInventory( void )
{
	if( driver != 0 && driver->isSubclassOf(Player))
	{
		((Player*)driver.Pointer())->enableInventory();
	}
}

qboolean Vehicle::Drive( usercmd_t *ucmd )
{
	Vector i, j, k;
	
	if ( !driver || !driver->isClient() )
	{
		return false;
	}
	
	if ( !drivable )
	{
		driver->client->ps.pm_flags |= PMF_FROZEN;
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		return false;
	}
	
	if(_noPrediction )
	{
		driver->client->ps.pm_flags |= PMF_NO_PREDICTION;
	}
	
	moveimpulse = ( ( float )ucmd->forwardmove ) * 3.0f;
	//turnimpulse = ( ( float )-ucmd->rightmove ) * 3.0f;
	jumpimpulse = ( ( float )ucmd->upmove * gravity ) / 350.0f;

	ucmd->lean = 0;
	
	if ( ( jumpimpulse < 0.0f ) || ( !jumpable ) )
		jumpimpulse = 0.0f;
	
	turnimpulse = 10.0f * angledist( SHORT2ANGLE( ucmd->angles[ 1 ] ) - driver->client->cmd_angles[ 1 ] );
	
	return true;
}

void Vehicle::Postthink( void )
{
	float			turn;
	Vector		i, j, k;
	Vector      normalsum;
	Vector      temp;
	Vector      pitch;
	Vector      roll;
	VehicleBase *v;
	VehicleBase *last;
	float       drivespeed;
	

	if ( drivable )
	{
		currentspeed = moveimpulse / 4.0f;
		
		turnangle = turnangle * 0.25f + turnimpulse;
		if ( turnangle > maxturnrate )
		{
			turnangle = maxturnrate;
		}
		else if ( turnangle < -maxturnrate )
		{
			turnangle = -maxturnrate;
		}
		else if ( fabs( turnangle ) < 2.0f )
		{
			turnangle = 0;
		}
		
		CalculateOrientation();		
		
		turn = turnangle * ( 1.0f / 200.0f );
		
		if ( groundentity )
		{
			float dot;
			Vector newvel;
			Vector flatvel;
			
			velocity[ 0 ] *= 0.925f;
			velocity[ 1 ] *= 0.925f;
			flatvel = Vector( orientation[ 0 ] );
			velocity += flatvel * currentspeed;
			flatvel[ 2 ] = 0;
			dot = velocity * flatvel;
			if ( dot > speed )
			{
				dot = speed;
			}
			else if ( dot < -speed )
			{
				dot = -speed;
			}
			else if ( fabs( dot ) < 20.0f )
			{
				dot = 0;
			}
			newvel = flatvel * dot;
			velocity[ 0 ] = newvel[ 0 ];
			velocity[ 1 ] = newvel[ 1 ];
			velocity[ 2 ] += dot * jumpimpulse;
			
			avelocity *= 0.05f;
			if ( steerinplace )
            {
				if ( dot < 350.0f )
					dot = 350.0f;
				avelocity.y += turn * dot;
            }
			else
            {
				avelocity.y += turn * dot;
            }
		}
		else
		{
			avelocity *= 0.1f;
		}
		angles += avelocity * level.frametime;
		setAngles( angles );
	}
	
	drivespeed = velocity * Vector( orientation[ 0 ] );
	
	if ( drivable && driver )
	{
		str sound_name;
		
		if ( currentspeed > 0.0f )
			sound_name = "snd_forward";
		else if ( currentspeed < 0.0f )
			sound_name = "snd_backward";
		else
			sound_name = "snd_idle";
		
		LoopSound( sound_name.c_str() );
	}
	
	i = Vector( orientation[ 0 ] );
	j = Vector( orientation[ 1 ] );
	k = Vector( orientation[ 2 ] );
	
	if ( driver )
	{
		Player * player;
		
		if ( driver->isSubclassOf ( Player ) )
		{
			player = ( Player * )( Sentient * )driver;
			player->setOrigin( origin + ( i * driveroffset[0] ) + ( j * driveroffset[1] ) + ( k * driveroffset[2] ) );
			if ( drivable )
			{
				player->velocity = vec_zero;
				Vector playerAngles = player->v_angle;
				Vector viewAngles = angles;
				
				if( _restrictPitch )
				{
					if(viewAngles[PITCH] > _maximumPitch)	
						viewAngles[PITCH] = _maximumPitch;
					else if(viewAngles[PITCH] < _minimumPitch)	
						viewAngles[PITCH] = _minimumPitch;
				}
				
				if( _restrictYaw )
				{
					if(viewAngles[YAW] > _maximumYaw)	
						viewAngles[YAW] = _maximumYaw;
					else if(viewAngles[YAW] <= _minimumYaw)	
						viewAngles[YAW] = _minimumYaw;
				}
				
				angles = viewAngles;
				playerAngles.y = angles.y;
				playerAngles.z = angles.z;
				player->SetViewAngles(playerAngles);
			}
		}
	}
	
	last = this;
	while( last->vlink )
	{
		v = last->vlink;
		v->setOrigin( origin + ( i * v->offset.x ) + ( j * v->offset.y ) + ( k * v->offset.z ) );
		v->avelocity = avelocity;
		v->velocity = velocity;
		v->angles[ ROLL ] = angles[ ROLL ];
		v->angles[ YAW ] = angles[ YAW ];
		v->angles[ PITCH ] = (float)( (int)( v->angles[ PITCH ] + (drivespeed/4) ) % 360 );
		
		if ( v->isSubclassOf( FrontWheels ) )
		{
			v->angles += Vector( 0.0f, turnangle, 0.0f );
		}
		v->setAngles( v->angles );
		
		last = v;
	}
	
	//CheckWater();
	WorldEffects();
	
	// save off last origin
	last_origin = origin;
	
	if ( !driver && !velocity.length() && groundentity && !( watertype & CONTENTS_LAVA ) )
	{
		flags &= ~FL_POSTTHINK;
		if ( drivable )
			setMoveType( MOVETYPE_NONE );
	}
}

/////////////////////////////////////////////////////////
//
// Name:  CalculateOrientation()
//
// Description: Calculates the orientation of the vehicle
// by getting the normals of all the poly's underneath 
// each corner of the vehicle's bounding box, this
// allows the vehicle to pitch appropriately
//
/////////////////////////////////////////////////////////

void Vehicle::CalculateOrientation( void )
{
	Vector      temp, pitch, normalsum;
	Vector      i, j, k;
	int         numnormals;
	trace_t     trace;
	
	temp[ PITCH ] = 0;
	temp[ YAW ] = angles[ YAW ];
	temp[ ROLL ] = 0;
	temp.AngleVectors( &i, &j, &k );
	j = vec_zero - j;
	
	//
	// figure out what our orientation is
	//
	
	numnormals = 0;
	for ( int index = 0; index < 4; index++ )
	{
		Vector start, end;
		Vector boxoffset;
		
		boxoffset = Corners[ index ];
		start = origin + ( i * boxoffset[0] ) + ( j * boxoffset[1] ) + ( k * boxoffset[2] );
		end = start;
		end[ 2 ] -= maxtracedist;
		trace = G_Trace( start, vec_zero, vec_zero, end, NULL, MASK_SOLID, false, "Vehicle::PostThink Corners" );
		
		if ( ( trace.fraction > 0.0f ) && ( trace.fraction != 1.0f ) && !trace.startsolid )
		{
			normalsum += Vector( trace.plane.normal );
			numnormals++;
		}
	}
	
	if ( numnormals > 1 )
	{
		temp = normalsum * ( 1.0f/ numnormals );
		temp.normalize();
		i = temp.CrossProduct( temp, j );
		pitch = i;
		
		// determine pitch
		angles[ 0 ] = -(pitch.toPitch());
	}
}

void Vehicle::VehicleTouched( Event *ev )
{
	Entity	*other;
	float		speed;
	Vector   delta;
	Vector	dir;
	
	other = ev->GetEntity( 1 );
	if ( other == driver )
	{
		return;
	}
	
	if ( other == world )
	{
		return;
	}
	
	if ( drivable && !driver )
	{
		return;
	}
	
	delta = origin - last_origin;
	speed = delta.length();
	if ( speed > 2 )
	{
		Sound( "vehicle_crash", true );
		dir = delta * ( 1.0f / speed );
		other->Damage( this, lastdriver, speed * 8.0f, origin, dir, vec_zero, speed * 15, 0, MOD_VEHICLE );
	}
	
}

void Vehicle::VehicleBlocked( Event * )
{
	return;
	/*
	Entity	*other;
	float		speed;
	float    damage;
	Vector   delta;
	Vector   newvel;
	Vector	dir;
	
	if ( !velocity[0] && !velocity[1] )
		return;
	  
	other = ev->GetEntity( 1 );
	if ( other == driver )
	{
		return;
	}
	if ( other->isSubclassOf( VehicleBase ) )
	{
		delta = other->origin - origin;
		delta.normalize();
		
		newvel = vec_zero - ( velocity) + ( other->velocity * 0.25 );
		if ( newvel * delta < 0 )
		{
			velocity = newvel;
			delta = velocity - other->velocity;
			damage = delta.length()/4;
		}
		else
		{
			return;
		}
	}
	else if ( ( velocity.length() < 350 ) )
	{
		other->velocity += velocity*1.25f;
		other->velocity[ 2 ] += 100;
		damage = velocity.length()/4;
	}
	else
	{
		damage = other->health + 1000;
	}
		  
	// Gib 'em outright
	speed = fabs( velocity.length() );
	dir = velocity * ( 1 / speed );
	other->Damage( this, lastdriver, damage, origin, dir, vec_zero, speed, 0, MOD_VEHICLE, -1, -1, 1.0f );
	*/
}

Sentient *Vehicle::Driver( void )
{
	return driver;
}

qboolean Vehicle::IsDrivable( void )
{
	return drivable;
}

void Vehicle::SetSpeed( Event *ev )
{
	speed = ev->GetFloat( 1 );
}

void Vehicle::SetTurnRate( Event *ev )
{
	maxturnrate = ev->GetFloat( 1 );
}


CLASS_DECLARATION( Vehicle, DrivableVehicle, "script_drivablevehicle" )
{
	{ &EV_Damage,				   &Entity::DamageEvent },
	{ &EV_Killed,				   &DrivableVehicle::Killed },
	{ NULL, NULL }
};

DrivableVehicle::DrivableVehicle()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	drivable = true;
	flags |= FL_DIE_EXPLODE;
}

void DrivableVehicle::Killed(Event *ev)
{
	Entity * ent;
	Entity * attacker;
	Vector dir;
	Event * event;
	const char * name;
	VehicleBase *last;
	
	takedamage = DAMAGE_NO;
	setSolidType( SOLID_NOT );
	hideModel();
	
	attacker		= ev->GetEntity( 1 );
	
	//
	// kill the driver
	//
	if ( driver )
	{
		Vector dir;
		SentientPtr sent;
		Event * event;
		
		velocity = vec_zero;
		sent = driver;
		event = new Event( EV_Use );
		event->AddEntity( sent );
		ProcessEvent( event );
		dir = sent->origin - origin;
		dir[ 2 ] += 64.0f;
		dir.normalize();
		sent->Damage( this, this, sent->health * 2.0f, origin, dir, vec_zero, 50, 0, MOD_VEHICLE  );
	}
	
	if (flags & FL_DIE_EXPLODE)
	{
		CreateExplosion( origin, 150.0f * edict->s.scale, this, this, this );
	}
	
	if (flags & FL_DIE_GIBS)
	{
		setSolidType( SOLID_NOT );
		hideModel();
		
		CreateGibs( this, -150.0f, edict->s.scale, 3 );
	}
	//
	// kill all my wheels
	//
	last = this;
	while( last->vlink )
	{
		last->vlink->PostEvent( EV_Remove, 0.0f );
		last = last->vlink;
	}
	
	
	//
	// kill the killtargets
	//
	name = KillTarget();
	if ( name && strcmp( name, "" ) )
	{
		ent = NULL;
		do
		{
			ent = G_FindTarget( ent, name );
			if ( !ent )
			{
				break;
			}
			ent->PostEvent( EV_Remove, 0.0f );
		}
		while ( 1 );
	}
	
	//
	// fire targets
	//
	name = Target();
	if ( name && strcmp( name, "" ) )
	{
		ent = NULL;
		do
		{
			ent = G_FindTarget( ent, name );
			if ( !ent )
			{
				break;
			}
			
			event = new Event( EV_Activate );
			event->AddEntity( attacker );
			ent->ProcessEvent( event );
		}
		while ( 1 );
	}
	
	PostEvent( EV_Remove, 0.0f );
}


//===============================================================
// Horse Vehicle Class
// New style of vehicle -- Allows the player to drive the vehicle
// but still mouselook and fire ( turret style )
// different control configurations ( ie Strafe Only ) etc can 
// be subclassed from movemode and plugged in easily
// 
//===============================================================

Event EV_HorseVehicle_SetSpeed
(
	"sethorsespeed",
	EV_SCRIPTONLY,
	"f", 
	"speed",
	"Set the speed of the horse"
);
Event EV_HorseVehicle_SetMoveMode
(
	"setmovemode",
	EV_SCRIPTONLY,
	"sF",
	"mode angles",
	"Mode ( standard , strafe, or locked ) and a Vector describing the view Angle )"
);
Event EV_HorseVehicle_SetForcedForward
(
	"forceforwardspeed",
	EV_SCRIPTONLY,
	"f",
	"speed",
	"Forces the vehicle to move forward at the specified speed"
);
Event EV_Vehicle_AnimDone
(
	"animdone",
	EV_CODEONLY,
	NULL,
	NULL,
	"animdoneevent"
);
Event EV_Driver_AnimDone
(
	"driveranimdone",
	EV_CODEONLY,
	NULL,
	NULL,
	"driveranimdoneevent"
);

CLASS_DECLARATION( Vehicle, HorseVehicle, "script_horsevehicle" )
{
	{ &EV_HorseVehicle_SetSpeed ,			&HorseVehicle::SetSpeed	},
	{ &EV_HorseVehicle_SetMoveMode ,		&HorseVehicle::SetVehicleMoveMode		},
	{ &EV_HorseVehicle_SetForcedForward,	&HorseVehicle::SetForcedForwardSpeed		},
	{ &EV_Vehicle_AnimDone,                 &HorseVehicle::AnimDone	},
	{ &EV_Driver_AnimDone,                  &HorseVehicle::DriverAnimDone	},


	//Move Mode Events
	{ &EV_FollowPath_SetWayPointName,		&HorseVehicle::PassToMoveMode		},
	{ NULL, NULL }
};


/////////////////////////////////////////////////////////
//
// Name:  HorseVehicle()
//
// Description: Constructor
// 
/////////////////////////////////////////////////////////

HorseVehicle::HorseVehicle()
{
	if ( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}
	
	drivable = true;
	jumpable = true;
	driver = NULL;
	currentspeed = 300;
	
	_baseYaw = 0;
	_driverYaw = 0;
	_lastYaw = 0;
	
	_driverPitch = 0;
	_lastPitch = 0;
	
	_forcedForwardSpeed = 0;
	_jumpflag = false;
	_jumped = false;
	_jumpSpeed = 0;
	
	_ducked = false;
	
	_minYawThreshold = 90;
	_maxYawThreshold = 90;
	
	_minPitchThreshold = 90;
	_maxPitchThreshold = 90;
	
	_moveMode = NULL;
	_SetMoveMode( "standard" );
	
	_currentCrosshairMode = CROSSHAIR_MODE_STRAIGHT;
	_newCrosshairMode = CROSSHAIR_MODE_STRAIGHT;
	_jumpmode = JUMPMODE_DONE;
	_duckmode = DUCKMODE_DONE;
	_animDone = false;
	_driverAnimDone = false;
	
	_ducked = false;
	_duckheld = false;
	
	_DriverBBoxMaxs   = vec_zero;
	_DriverBBoxMins   = vec_zero;
	_originalBBoxMaxs = vec_zero;
	_originalBBoxMins = vec_zero;
	
	//Create the Animate Object so that we can set the
	//animation on the vehicle
	if ( !animate )
		animate = new Animate;
	
}

HorseVehicle::~HorseVehicle()
{
	if ( _moveMode )
	{
		delete _moveMode;
		_moveMode = 0;
		
	}
}

/////////////////////////////////////////////////////////
//
// Name:  PassToMoveMode()
//
// Description: Hands off the event to the move mode for
// it to handle -- allows the vehicle to remain dumb in 
// regards to movement specific events
// 
/////////////////////////////////////////////////////////

void HorseVehicle::PassToMoveMode( Event *ev )
{
	
	if ( _moveMode )
		_moveMode->HandleEvent( ev );
	
}

/////////////////////////////////////////////////////////
//
// Name:  Drive()
//
// Description: Called from Player->ClientThink
// Player passes it a ucmd pointer, which points to the
// latest input data from the user.  Drive() uses this
// data to set up the movement impulses that help control
// how the vehicle moves
// 
/////////////////////////////////////////////////////////

qboolean HorseVehicle::Drive(	usercmd_t *ucmd )
{
	Vector i, j, k;
	
	if ( !driver || !driver->isClient() )
		return false;
	
	if ( !drivable )
	{
		driver->client->ps.pm_flags |= PMF_FROZEN;
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		return false;
	}
	
	//Turn off Client Prediction
	//driver->client->ps.pm_flags |= PMF_NO_PREDICTION;
	driver->client->ps.in_vehicle = true;
	driver->client->ps.vehicleoffset[0] = origin[0];
	driver->client->ps.vehicleoffset[1] = origin[1];
	driver->client->ps.vehicleoffset[2] = origin[2] + 160.0f;
	
	driver->velocity = velocity;
	
	//jumpimpulse is used to calculate a jump value
	//moveimpulse is used to calculate forward and back movement
	//turnimpulse is used to calculate strafe movement
	jumpimpulse = ( ( float )ucmd->upmove * gravity ) / 350.0f;
	moveimpulse = ucmd->forwardmove;
	turnimpulse = ucmd->rightmove;
	
	/*
	if ( ( jumpimpulse < 0 ) || ( !jumpable ) )
	jumpimpulse = 0;
	*/
	
	return true;
}

/////////////////////////////////////////////////////////
//
// Name:  Postthink()
//
// Description: Think Function ( called every frame )
// Postthink is the master function for the vehicle.
// it delegates out movement, animation, and FX to 
// subfunctions.
/////////////////////////////////////////////////////////

void HorseVehicle::Postthink ( void	)
{
	
	if ( drivable )
	{
		CalculateOrientation();
		
		_moveMode->Move( this );
		
		if (driver)
		{
			_PlayMovementSound();
			
			if ( !_jumpflag)
				_AnimateVehicle("run");			
			
			if ( !_duckflag )
				_PositionDriverModel();			
			
			if ( groundentity && ( jumpimpulse > 0 ) && ( _jumpmode == JUMPMODE_DONE ) && ( _duckmode == DUCKMODE_DONE ) )
				_InitializeJump();		
			
			if ( _jumpflag )
				_HandleJump();
			
			if ( groundentity && ( jumpimpulse < 0 ) && ( _duckmode == DUCKMODE_DONE ) && ( _jumpmode == JUMPMODE_DONE ) && !_duckflag )
				_InitializeDuck();
			
			//Released the duck key
			if ( _duckflag && ( jumpimpulse >= 0 ) )
				_ducked = false;
			
			if ( _duckflag )
				_HandleDuck();
			
		}
		
	}
	
	//Do World Effects ( for earthquakes and the like )
	WorldEffects();
	
	Vector flatvel;
	if ( _forcedForwardSpeed )
	{
		flatvel = Vector( orientation[ 0 ] );
		velocity += flatvel * _forcedForwardSpeed;
	}
	
	
	// Turn off think if we aren't being used
	if ( !driver && !velocity.length() && groundentity && !( watertype & CONTENTS_LAVA ) )
	{
		flags &= ~FL_POSTTHINK;
		if ( drivable )
			setMoveType( MOVETYPE_NONE );
	}
}

/////////////////////////////////////////////////////////
//
// Name:  DriverUse()
//
// Description: Function is called when the player "uses"
// the vehicle.  It handles putting the driver in and 
// taking the driver out of the vehicle.
//
// Some sort of use-delay mechanism must be in place or
// it will be very difficult for the player to get on and
// off the vehicle, because the use events are stepping
// on each other
// 
/////////////////////////////////////////////////////////

void HorseVehicle::DriverUse( Event *ev )
{
	
	Event *event;
	Entity *other;
	
	other = ev->GetEntity( 1 );
	if ( level.time < usetime + 1.0f )
		return;
	else
		usetime = level.time;
	
	if ( !other || !other->isSubclassOf( Sentient ) )
	{
		return;
	}
	
	if ( driver )
	{
		int height;
		int ang;
		Vector angles;
		Vector forward;
		Vector pos;
		float ofs;
		trace_t trace;
		
		if ( other != driver )
		{
			return;
		}
		
		if ( locked )
			return;
		
		//
		// place the driver on the ground
		//
		driver->detach();
		_AnimateDriver( "idle" );
		_AnimateVehicle( "idle" );
		driver->client->ps.in_vehicle = false;
		ofs = size.length() * 0.5f;
		for ( height = 0; height < 100; height += 16 )
		{
			for ( ang = 0; ang < 360; ang += 30 )
            {
				angles[ 1 ] = driver->angles[ 1 ] + ang + 90.0f;
				angles.AngleVectors( &forward, NULL, NULL );
				pos = origin + (forward * ofs);
				pos[2] += height;
				trace = G_Trace( pos, driver->mins, driver->maxs, pos, NULL, MASK_PLAYERSOLID, false, "Vehicle::DriverUse 1" );
				if ( !trace.startsolid && !trace.allsolid )
				{
					Vector end;
					
					end = pos;
					end[ 2 ] -= 128.0f;
					trace = G_Trace( pos, driver->mins, driver->maxs, end, NULL, MASK_PLAYERSOLID, false, "Vehicle::DriverUse 2" );
					if ( trace.fraction < 1.0f )
					{
						driver->setOrigin( pos );
						goto foundpos;
					}
				}
            }
		}
		return;
		
foundpos:
		
		turnimpulse = 0;
		moveimpulse = 0;
		jumpimpulse = 0;
		
		event = new Event( EV_Vehicle_Exit );
		event->AddEntity( this );
		driver->ProcessEvent( event );
		if ( hasweapon )
		{
			driver->takeItem( weaponName.c_str() );
		}
		if ( drivable )
		{
			StopLoopSound();
			Sound( "snd_dooropen", CHAN_BODY );
			Sound( "snd_stop", CHAN_VOICE );
			driver->setSolidType( SOLID_BBOX );
		}
		
		driver = NULL;
	}
	else
	{
		driver = ( Sentient * )other;
		
		lastdriver = driver;
		
		
		if ( drivable )
			setMoveType( MOVETYPE_VEHICLE );
		
		if ( drivable )
		{
			Sound( "snd_doorclose", CHAN_BODY );
			Sound( "snd_start", CHAN_VOICE );
			driver->setSolidType( SOLID_NOT );
		}
		
		event = new Event( EV_Vehicle_Enter );
		event->AddEntity( this );
		if ( driveranim.length() )
			event->AddString( driveranim );
		driver->ProcessEvent( event );
		
		offset = other->origin - origin;
		
		flags	|= FL_POSTTHINK;
		SetDriverAngles( angles + seatangles );
		
		int tagnum = gi.Tag_NumForName( this->edict->s.modelindex, "tag_rider" );
		
		
		driver->attach(this->entnum , tagnum , false , seatoffset );
		
		if ( driver->isSubclassOf( Player ) )
		{
			Player* player;
			player = ( Player * )( Sentient * )driver;
			player->v_angle = angles;
			
			_lastYaw = player->client->cmd_angles[YAW];
			_DriverBBoxMaxs = player->maxs;
			_DriverBBoxMins = player->mins;
		}
		
		_driverYaw = angles[YAW];
		_driverPitch = angles[PITCH];		
		
		_originalBBoxMaxs = _DriverBBoxMaxs;
		_originalBBoxMins = _DriverBBoxMins;
	}
}

/////////////////////////////////////////////////////////
//
// Name:  _PlayMovementSound()
//
// Description: Plays a sound based on movement
//
/////////////////////////////////////////////////////////

void HorseVehicle::_PlayMovementSound()
{
	str sound_name;
	
	if ( currentspeed > 0.0f )
		sound_name = "snd_forward";
	else if ( currentspeed < 0.0f )
		sound_name = "snd_backward";
	else
		sound_name = "snd_idle";
	
	LoopSound( sound_name.c_str() );	
}

/////////////////////////////////////////////////////////
//
// Name:  _AnimateVehicle()
//
// Description: Sets the animation on the vehicle
//
/////////////////////////////////////////////////////////

void HorseVehicle::_AnimateVehicle(const str &anim , qboolean useEvent )
{
	if ( animate )
	{
		int anim_num = gi.Anim_Random ( this->edict->s.modelindex, anim.c_str() );
		if ( anim_num != -1 )
		{
			if ( animate->CurrentAnim() != anim_num )
			{
				animate->ClearLegsAnim();
				animate->ClearTorsoAnim();
				animate->NewAnim( anim_num );	
				
				if ( useEvent )
				{
					Event *ev = new Event( EV_Vehicle_AnimDone );
					animate->SetAnimDoneEvent( ev );
					_animDone = false;
				}
			}
			
		}
	}	
}

/////////////////////////////////////////////////////////
//
// Name:  _AnimateDriver()
//
// Description: Sets the animation on the vehicle
//
/////////////////////////////////////////////////////////

void HorseVehicle::_AnimateDriver(const str &anim , qboolean useEvent )
{
	if ( !driver )
		return;
	
	Player *player;
	player = ( Player * )( Sentient * )driver;
	
	
	if ( player->animate )
	{
		int anim_num = gi.Anim_Random ( player->edict->s.modelindex, anim.c_str() );
		if ( anim_num != -1 )
		{
			if ( player->animate->CurrentAnim() != anim_num )
			{
				player->animate->ClearLegsAnim();
				player->animate->ClearTorsoAnim();
				player->animate->NewAnim( anim_num );	
				
				if ( useEvent )
				{
					Event *ev = new Event( EV_Driver_AnimDone );
					player->animate->SetAnimDoneEvent( ev );
					_driverAnimDone = false;
				}
			}
			
		}
	}	
}


/////////////////////////////////////////////////////////
//
// Name:  _PositionDriverModel()
//
// Description: Places the driver model in the correct position
//
/////////////////////////////////////////////////////////

void HorseVehicle::_PositionDriverModel()
{
	Vector		i, j, k;
	
	i = Vector( orientation[ 0 ] );
	j = Vector( orientation[ 1 ] );
	k = Vector( orientation[ 2 ] );
	
	if ( driver )
	{
		Player *player;
		player = ( Player * )( Sentient * )driver;
		
		if ( drivable )
		{
			player->velocity = vec_zero;
			player->setAngles( angles );
			
			float delta;
			delta = AngleDelta(_lastYaw , player->client->cmd_angles[YAW]  );
			
			
			_driverYaw += ( delta * -1.0f );
			_lastYaw = player->client->cmd_angles[YAW];
			_newCrosshairMode = CROSSHAIR_MODE_STRAIGHT;
			
			if ( _driverYaw >= ( angles[YAW] + _maxYawThreshold ) )
			{
				_driverYaw = angles[YAW] + _maxYawThreshold;
				_newCrosshairMode = CROSSHAIR_MODE_RIGHT;
				
			}
			
			if ( _driverYaw <= ( angles[YAW] - _minYawThreshold ) )
			{
				_driverYaw = angles[YAW] - _minYawThreshold;
				_newCrosshairMode = CROSSHAIR_MODE_LEFT;
			}
			
			delta = AngleDelta(_lastPitch , player->client->cmd_angles[PITCH] );
			
			_driverPitch += ( delta * -1.0f );
			_lastPitch = player->client->cmd_angles[PITCH];
			
			if ( _driverPitch > ( angles[PITCH] + _maxPitchThreshold ) )
				_driverPitch = angles[PITCH] + _maxPitchThreshold;
			
			if ( _driverPitch < ( angles[PITCH] - _minPitchThreshold ) )
				_driverPitch = angles[PITCH] - _minPitchThreshold;
			
			_SetCrossHairMode();
			player->v_angle = player->client->cmd_angles;
			player->v_angle[YAW] = _driverYaw;
			player->v_angle[PITCH] = _driverPitch;
			
			if ( !_duckflag )
				player->SetAnim( "ride" , legs );
			
		}
	}
}

/////////////////////////////////////////////////////////
//
// Name:  _SetSpeed()
//
// Description: Specifies the speed that the vehicle will move
//
/////////////////////////////////////////////////////////

void HorseVehicle::_SetSpeed ( Event *ev )
{
	currentspeed = ev->GetFloat( 1 );
}
   
/////////////////////////////////////////////////////////
//
// Name:  SetVehicleMoveMode()
//
// Description: Script Interface to set the moveMode object
//
/////////////////////////////////////////////////////////

void HorseVehicle::SetVehicleMoveMode( Event *ev )
{
	str modeName = ev->GetString( 1 );
	_SetMoveMode( modeName );
}

/////////////////////////////////////////////////////////
//
// Name:  SetForcedForwardSpeed()
//
// Description: Script interface for setting the 
//              _forcedForwardSpeed
//
/////////////////////////////////////////////////////////

void HorseVehicle::SetForcedForwardSpeed( Event *ev )
{
	_forcedForwardSpeed = ev->GetFloat( 1 );
}

/////////////////////////////////////////////////////////
//
// Name:  _HandleJump()
//
// Description: Takes care of jumping
//
/////////////////////////////////////////////////////////

void HorseVehicle::_HandleJump()
{
	
	if ( groundentity && _jumped && ( _jumpmode == JUMPMODE_DONE ) )
	{
		velocity[2] = 0;
		_jumpflag = false;		
		_jumped = false;
		_jumpSpeed = 0;
		return;
		
	}
	
	switch ( _jumpmode )
	{
	case JUMPMODE_START:
		flags |= FL_FLY;
		_jumpSpeed += 100.0f;
		if ( _jumpSpeed > 600.0f )
			_jumpSpeed = 600.0f;		
		
		velocity[2] = _jumpSpeed;
		_AnimateVehicle ( "jump_to_rise" , true );
		
		if ( _animDone )
		{
			_jumpSpeed = 0;
			_animDone = false;
			_jumpmode = JUMPMODE_HOLD;
		}
		
		break;
		
	case JUMPMODE_HOLD:
		if ( level.time < _holdtime )
		{
			_jumpSpeed += 10.0f;
			velocity[2] = _jumpSpeed;
			_AnimateVehicle("rise" , true);
		}
		else
		{
			_jumpSpeed = 0;
			_animDone = false;
			_jumpmode = JUMPMODE_LAND;		
		}
		
		break;
		
	case JUMPMODE_LAND:
		_jumpSpeed -= 100.0f;
		if ( _jumpSpeed < -600.0f )
			_jumpSpeed = -600.0f;
		
		velocity[2] = _jumpSpeed;
		_AnimateVehicle( "rise_to_land" , true );				
		
		if ( _animDone )
		{
			flags &= ~FL_FLY;
			_jumpmode = JUMPMODE_DONE;
			_AnimateVehicle( "run" );					
		}
		
		break;
		
		
	default:
		//Should never get here, but you know... It's code.
		break;
		
	}
	
	_jumped = true;
	
}

/////////////////////////////////////////////////////////
//
// Name:  _InitializeDuck()
//
// Description: Starts Jump
//
/////////////////////////////////////////////////////////
void HorseVehicle::_InitializeDuck()
{
	if ( !driver )
		return;
	
	
	_duckflag = true;
	_duckmode = DUCKMODE_START;
	_driverAnimDone = false;
	_ducked = true;
	
	//Player *player;
	//player = ( Player * )( Sentient * )driver;
	
	_DriverBBoxMaxs[2] -= 64.0f;
}

/////////////////////////////////////////////////////////
//
// Name:  _HandleDuck()
//
// Description: Starts Jump
//
/////////////////////////////////////////////////////////
void HorseVehicle::_HandleDuck()
{
	//Going to position torso Forward
	//_driverYaw = 0;
	//_lastYaw = 0;
	
	//_driverPitch = 0;
	//_lastPitch = 0;
	
	if ( !driver )
		return;
	
	Player *player;
	player = ( Player * )( Sentient * )driver;
	player->v_angle = angles;
	player->setAngles( angles );
	
	_driverYaw = angles[YAW];
	_lastYaw = player->client->cmd_angles[YAW];
	
	Vector tempMins;
	Vector tempMaxs;
	
	switch ( _duckmode )
	{
	case DUCKMODE_START:
		_AnimateDriver("ride_to_duck" , true );
		
		if ( _driverAnimDone )
			_duckmode = DUCKMODE_HOLD;		
		break;
		
	case DUCKMODE_HOLD:
		if ( _ducked )
			_AnimateDriver("ride_duck_hold" , true );
		else
			_duckmode = DUCKMODE_FINISH;
		break;
		
	case DUCKMODE_FINISH:
		_AnimateDriver("duck_to_ride" , true );
		if ( _driverAnimDone )
			_duckmode = DUCKMODE_DONE;				
		break;
		
	case DUCKMODE_DONE:	
		_AnimateDriver( "ride" );
		
		tempMaxs[2] +=64.0f;
		
		player->setSize(_DriverBBoxMaxs, _DriverBBoxMaxs );
		_DriverBBoxMaxs = _originalBBoxMaxs;
		_DriverBBoxMins = _originalBBoxMins;
		_duckflag = false;
		break;
	}
}

/////////////////////////////////////////////////////////
//
// Name:  _InitializeJump()
//
// Description: Starts Jump
//
/////////////////////////////////////////////////////////

void HorseVehicle::_InitializeJump()
{
	_jumptime = level.time + .25f;
	_holdtime = _jumptime + .15f;
	
	_jumpflag = true;
	_jumpmode = JUMPMODE_START;
	_animDone = false;
	
}

/////////////////////////////////////////////////////////
//
// Name:  _SetMoveMode()
//
// Description: _moveMode Factory
//
/////////////////////////////////////////////////////////

void HorseVehicle::_SetMoveMode( const str &modeName )
{
	if ( _moveMode )
	{
		delete _moveMode;
		_moveMode = 0;
	}
	
	if ( !Q_stricmp(modeName.c_str() , "standard" ) )
		_moveMode = new HVMoveMode_Standard;
	
	else if ( !Q_stricmp(modeName.c_str() , "strafe" ) )
		_moveMode = new HVMoveMode_Strafe;
	
	else if ( !Q_stricmp(modeName.c_str() , "locked" ) )
		_moveMode = new HVMoveMode_Locked;
	
	else if ( !Q_stricmp(modeName.c_str() , "waypoint" ) )
		_moveMode = new HVMoveMode_FollowPath;
}

void HorseVehicle::_SetCrossHairMode()
{
	if ( _newCrosshairMode == _currentCrosshairMode )
		return;
	
	switch ( _newCrosshairMode )
	{
	case CROSSHAIR_MODE_STRAIGHT:
		gi.cvar_set("cl_chright", "100" );
		gi.cvar_set("cl_chleft" , "-100" );
		_currentCrosshairMode = CROSSHAIR_MODE_STRAIGHT;
		break;
		
	case CROSSHAIR_MODE_LEFT:
		gi.cvar_set("cl_chright", "310" );
		gi.cvar_set("cl_chleft" , "-100" );
		_currentCrosshairMode = CROSSHAIR_MODE_LEFT;
		break;
		
	case CROSSHAIR_MODE_RIGHT:
		gi.cvar_set("cl_chright", "100" );
		gi.cvar_set("cl_chleft" , "-310" );
		_currentCrosshairMode = CROSSHAIR_MODE_RIGHT;
		break;
		
	default:
		return;
	}
	
}

void HorseVehicle::AnimDone( Event * )
{
	_animDone = true;
}

void HorseVehicle::DriverAnimDone( Event * )
{
	_driverAnimDone = true;
}

void HorseVehicle::HandleEvent( Event *ev )
{
	Event *new_event;
	new_event = new Event( ev );
	ProcessEvent(new_event);
}

void HorseVehicle::_SetBaseYaw()
{
	if ( !driver )
		return;
	
	
	Player *player;
	player = ( Player * )( Sentient * )driver;
	_baseYaw = player->client->cmd_angles[YAW];
	_lastYaw = _baseYaw;
	
}

//===============================================================================
//
// Vehicle Move Mode Classes:
// These classes act as movement handling strategies for vehicles.  This way
// we can specify multiple different ways a single vehicle reacts, based on
// the context of the game.
//
//===============================================================================
CLASS_DECLARATION( Listener, VehicleMoveMode, "MoveModeBaseClass" )
{
	{ NULL, NULL }
};

VehicleMoveMode::VehicleMoveMode()
{

}

void VehicleMoveMode::Move( Vehicle *vehicle )
{
	Q_UNUSED(vehicle);
}

void VehicleMoveMode::HandleEvent( Event * )
{

}

//==============================================================================
//
// HVMoveMode_Standard
//
// Standard Move Strategy for the Horse Vehicle.  This is the strategy that is
// instaniated by default
//
//===============================================================================
HVMoveMode_Standard::HVMoveMode_Standard()
{

}

void HVMoveMode_Standard::Move( Vehicle *base_vehicle )
{
	HorseVehicle *vehicle;
	vehicle = (HorseVehicle*)base_vehicle;
	
	Vector flatvel;
	
	//Zero Out our Velocity		
	vehicle->velocity = vec_zero;
	
	if ( vehicle->moveimpulse > 0.0f )
	{
		if ( vehicle->currentspeed < 0.0f )
			vehicle->currentspeed *= -1.0f;
		
		flatvel = Vector( vehicle->orientation[ 0 ] );
		vehicle->velocity += flatvel * vehicle->currentspeed;
	}
	
	if ( vehicle->moveimpulse < 0.0f )
	{
		if ( vehicle->currentspeed > 0.0f )
			vehicle->currentspeed *= -1.0f;
		
		flatvel = Vector( vehicle->orientation[ 0 ] );
		vehicle->velocity += flatvel * vehicle->currentspeed;
	}
	
	if ( vehicle->turnimpulse > 0.0f )
	{
		if ( vehicle->currentspeed > 0.0f )
			vehicle->currentspeed *= -1.0f;
		
		flatvel = Vector( vehicle->orientation[ 1 ] );
		vehicle->velocity += flatvel * vehicle->currentspeed;
	}
	
	if ( vehicle->turnimpulse < 0.0f )
	{
		if ( vehicle->currentspeed < 0.0f )
			vehicle->currentspeed *= -1.0f;
		
		flatvel = Vector( vehicle->orientation[ 1 ] );
		vehicle->velocity += flatvel * vehicle->currentspeed;
	}
}

//==============================================================================
//
// HVMoveMode_Strafe
//
// The only movement allowed by the horse is strafing left and right
//
//===============================================================================
HVMoveMode_Strafe::HVMoveMode_Strafe()
{
	
}

void HVMoveMode_Strafe::Move( Vehicle *base_vehicle )
{
	HorseVehicle *vehicle;
	vehicle = (HorseVehicle*)base_vehicle;
	
	Vector flatvel;
	
	//Zero Out our Velocity	
	vehicle->velocity = vec_zero;
	
	if ( vehicle->turnimpulse > 0)
	{
		if ( vehicle->currentspeed > 0 )
			vehicle->currentspeed*=-1;
		
		flatvel = Vector( vehicle->orientation[ 1 ] );
		vehicle->velocity += flatvel * vehicle->currentspeed;
	}
	
	if ( vehicle->turnimpulse < 0)
	{
		if ( vehicle->currentspeed < 0 )
			vehicle->currentspeed*=-1;
		
		flatvel = Vector( vehicle->orientation[ 1 ] );
		vehicle->velocity += flatvel * vehicle->currentspeed;
	}	
}

//==============================================================================
//
// HVMoveMode_Locked
//
// A bit strange... Allows No Movement at all
//
//===============================================================================
HVMoveMode_Locked::HVMoveMode_Locked()
{
	
}

void HVMoveMode_Locked::Move( Vehicle *base_vehicle )
{
	Q_UNUSED(base_vehicle);
}

//==============================================================================
//
// HVMoveMode_FollowPath
//
// Makes the Vehicle Follow WayPointNodes -- There is currently NO collision
// avoidance here
//
//===============================================================================
Event EV_FollowPath_SetWayPointName
(
	"waypointname",
	EV_SCRIPTONLY,
	"s",
	"waypoint_target_name",
	"Set the waypoint node name to go to first"
);

CLASS_DECLARATION( VehicleMoveMode, HVMoveMode_FollowPath, "FollowPath_MoveModeStrategy" )
{
	{ &EV_FollowPath_SetWayPointName,  &HVMoveMode_FollowPath::SetWaypointName },
	{ NULL, NULL }
};

HVMoveMode_FollowPath::HVMoveMode_FollowPath()
{
	_pathcompleted = false;
	_currentWaypoint = NULL;
}

void HVMoveMode_FollowPath::Move( Vehicle *base_vehicle )
{
	HorseVehicle *vehicle;
	vehicle = (HorseVehicle*)base_vehicle;
	
	
	//Zero Out our Velocity	
	vehicle->velocity = vec_zero;
	if ( vehicle->currentspeed < 0.0f )
		vehicle->currentspeed *= -1.0f;
	
	//First Check if we are at a way point;
	Vector dest;
	Vector check;
	
	if ( _pathcompleted )
		return;
	
	if (!_currentWaypoint)
	{
		_SetWayPoint( _currentWaypointName );
		if (!_currentWaypoint)
			return;
	}
	
	//Paranoia check, as it keeps crashing one particular box
	if ( !_currentWaypoint )
		return;
	
	dest = _currentWaypoint->origin;
	check = dest - vehicle->origin;
	
	// Take care of Z - Differences
	check[2] = 0;
	
	// Check if we're close enough
	if ( check.length() < 25.0f )
	{
		// Run Our Thread
		str waypointThread;
		waypointThread = _currentWaypoint->GetThread();
		
		if (waypointThread.length() )
			_RunThread( waypointThread );
		
		// See if we have another point to go to
		if ( _currentWaypoint->target.length() == 0 )
		{
			vehicle->velocity = vec_zero;
			_currentWaypoint = NULL;
			_pathcompleted = true;
			return;			
		}
		
		// Go To the Next Point
		_currentWaypointName = _currentWaypoint->target;
		_SetWayPoint( _currentWaypointName );		
		
		if (!_currentWaypoint)
			return;	
	}
	
	if ( _currentWaypoint )
	{
		vehicle->velocity = check;	
		vehicle->velocity.normalize();
		vehicle->velocity *= vehicle->currentspeed;
	}	
}

void HVMoveMode_FollowPath::SetWaypointName( Event *ev )
{
	_currentWaypointName = ev->GetString( 1 );
	_pathcompleted = false;
}

void HVMoveMode_FollowPath::_SetWayPoint( const str& name )
{
	Entity* ent_in_range;
	gentity_t *ed;
	
	for ( int i = 0; i < MAX_GENTITIES; i++ )
	{
		ed = &g_entities[i];
		
		if ( !ed->inuse || !ed->entity )
		{
			continue;
		}
		
		
		ent_in_range = g_entities[i].entity;
		
		if( ent_in_range->isSubclassOf( WayPointNode ) )
		{
			if (!Q_stricmp(ent_in_range->targetname.c_str() , name.c_str() ))
			{
				_currentWaypoint = (WayPointNode*)ent_in_range;
				return;
			}
		}
	}
	
	_currentWaypoint =  NULL;
}

void HVMoveMode_FollowPath::_RunThread( const str &thread_name	)
{
	if ( thread_name.length() <= 0 )
		return;
	
	CThread *thread;
	
	thread = Director.CreateThread( thread_name );
	
	if ( thread )
		thread->DelayedStart( 0.0f );
}
                            
void HVMoveMode_FollowPath::HandleEvent( Event *ev )
{
	Event *new_event;
	new_event = new Event( ev );
	ProcessEvent(new_event);
}
