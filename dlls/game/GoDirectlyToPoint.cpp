//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/GoDirectlyToPoint.cpp                      $
// $Revision:: 6                                                              $
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

#include "_pch_cpp.h"
#include "actor.h"
#include "GoDirectlyToPoint.h"


//------------------------- CLASS ------------------------------
//
// Name:		GoDirectlyToPoint
// Base Class:	Class
//
// Description:	Simple Steering class that moves directly to the 
// desired point without any regard for obstacle or world 
// avoidance. 
//
// Method of Use:	This is an appropiate steering method iff
// some guarentee is made that the Actor will not collide with
// anything while travelling to the target point. 
//
//--------------------------------------------------------------
CLASS_DECLARATION( Steering, GoDirectlyToPoint, NULL )
{
	{ NULL, NULL }
};

//----------------------------------------------------------------
// Name:			GoDirectlyToPoint
// Class:			GoDirectlyToPoint
//
// Description:		default constructor
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
GoDirectlyToPoint::GoDirectlyToPoint()
{
}

//----------------------------------------------------------------
// Name:			Begin
// Class:			GoDirectlyToPoint
//
// Description:		Initializes variables necessary to Evaluate
//
// Parameters:		
//					Actor self - the actor moving to a point
//
// Returns:			None
//----------------------------------------------------------------
void GoDirectlyToPoint::Begin(Actor &self)
{
}

//----------------------------------------------------------------
// Name:			Evaluate
// Class:			GoDirectlyToPoint
//
// Description:		attempts to move the Actor to the goal position
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			Steering::ReturnValue returns reason for failure
//----------------------------------------------------------------
const Steering::ReturnValue GoDirectlyToPoint::Evaluate( Actor &self )
{
	ResetForces();

	if (AtDestination( self ))
	{
		return Steering::SUCCESS;
	}
	
	Vector newSteeringForce = self.movementSubsystem->SteerTowardsPoint
											( 
											_destination,			
											vec_zero,											
											self.movementSubsystem->getMoveDir(),		
											self.movementSubsystem->getMoveSpeed()		
											);
	self.movementSubsystem->Accelerate( newSteeringForce );
	
	return Steering::EVALUATING ;
}

//----------------------------------------------------------------
// Name:			ShowInfo
// Class:			GoDirectlyToPoint
//
// Description:		prints out useful debug info for the class
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			None
//----------------------------------------------------------------
void GoDirectlyToPoint::ShowInfo( Actor &self )
{
	Steering::ShowInfo( self );
	
	gi.Printf( "\n destination : ( %f, %f, %f ) \n", _destination.x, _destination.y, _destination.z );
}



//----------------------------------------------------------------
// Name:			AtDestination
// Class:			GoDirectlyToPoint
//
// Description:		test that determines if the Actor is close to
//					the goal
//
// Parameters:		
//					Actor self - the actor following the path
//
// Returns:			bool that is true if the Actor is at the goal
//----------------------------------------------------------------
const bool GoDirectlyToPoint::AtDestination(const Actor &self) const
{
	//Check if we are in range 
	if ((Vector::DistanceXY( _destination, self.origin ) ) <= _radius )
	{
		return true;
	}

	return false;
}
