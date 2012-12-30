//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/shrapnelbomb.cpp                          $
// $Revision:: 11                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:

#include "_pch_cpp.h"
#include "shrapnelbomb.h"

Event EV_ShrapnelBomb_ShrapnelModel
(
	"setshrapnelmodel",
	EV_TIKIONLY,
	"s",
	"modelname",
	"The model of the shrapnel pieces that are spawned when the main bomb explodes"
);
Event EV_ShrapnelBomb_ShrapnelCount
(
	"setshrapnelcount",
	EV_TIKIONLY,
	"i",
	"count",
	"The number of shrapnel pieces to spawn"
);

CLASS_DECLARATION( Projectile, ShrapnelBomb, NULL )
{
	{ &EV_ShrapnelBomb_ShrapnelCount,				&ShrapnelBomb::SetShrapnelCount },
	{ &EV_ShrapnelBomb_ShrapnelModel,				&ShrapnelBomb::SetShrapnelModel },


	{ NULL, NULL	}
};

ShrapnelBomb::ShrapnelBomb()   
{
	if ( LoadingSavegame ) return;

	shrapnelCount = 5;

	turnThinkOn();

	_splitOnDescent = true;

	_randomSpread = true;
}

void ShrapnelBomb::Think( void )
{
	if ( _splitOnDescent && ( level.time >= startTime + minlife ) )
	{
		if ( velocity.z < 0.0f )
		{
			Explode();
		}
	}
}

void ShrapnelBomb::Explode( Event *ev )
{
	Explode();
}

void ShrapnelBomb::Explode( void )
{
	Vector dir;
	int i;
	Vector angles;
	Vector left;
	
	//Spawn shrapnel
	for ( i = 0 ; i < shrapnelCount ; i++ )
	{
		dir = velocity;
		dir.normalize();
		
		angles = dir.toAngles();
		
		angles.AngleVectors( NULL, &left, NULL );
		
		if ( _randomSpread )
		{
			dir += left * G_CRandom( .5f );
		}
		else
		{
			//dir += left * G_CRandom( .5f );
			dir += left * ( ( i * 0.1f ) - ( ( shrapnelCount - 1 ) * 0.1f / 2.0f ) );
			dir += left * 0.05;
		}
		
		dir.normalize();
		
		/* dir = origin;
		dir.normalize();
		
		  //Adjust Vector
		  dir += Vector ( G_CRandom( .5f ), G_CRandom( .5f ), G_CRandom( .5f ) );
		  
			//Get a Random Yaw
			float yaw = G_Random( 360.0f );
			Vector angles = dir.toAngles();
			
			  //Set the new Yaw
			  angles[YAW] = yaw;
			  
				//Set the new dir vector for the spawned shrapnel
				Vector newDir;
				angles.AngleVectors( &newDir );
				
				  
		ProjectileAttack( this->origin, newDir, this, shrapnelModel, 1.0f, ( velocity.length() / 2.0f ) ); */
		ProjectileAttack( this->origin, dir, this, shrapnelModel, 1.0f, ( velocity.length() / 2.0f ) );
	}
	
	//velocity = Vector ( 0.0f, 0.0f, 0.0f );
	//setMoveType( MOVETYPE_NONE );
	this->animate->RandomAnimate("explode");	
	
	//PostEvent( EV_Remove, 1.0f );
	PostEvent( EV_Remove, 0.0f );
}

void ShrapnelBomb::SetShrapnelCount( Event *ev )
{
	shrapnelCount = ev->GetInteger( 1 );
}

void ShrapnelBomb::SetShrapnelModel( Event *ev )
{
	shrapnelModel = ev->GetString( 1 );
}
