//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/GoDirectlyToPoint.h                            $
// $Revision:: 4                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Simple Steering class that moves directly to the desired point 
// without any regard for obstacle or world avoidance
//

#ifndef __GO_DIRECTLY_TO_POINT__
#define __GO_DIRECTLY_TO_POINT__

#include "g_local.h"
#include "steering.h"

class Actor;

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

class GoDirectlyToPoint : public Steering
{
public:
								CLASS_PROTOTYPE( GoDirectlyToPoint );

								GoDirectlyToPoint();
	virtual						~GoDirectlyToPoint() { }
	virtual void				Begin( Actor &self );
	virtual const ReturnValue	Evaluate( Actor &self );
	virtual void				ShowInfo( Actor &self );

	void						SetDestination( const Vector &destination ) { _destination = destination; }
	void						SetRadius( const float radius ) { _radius = radius; }
	virtual void				Archive( Archiver &arc );
	const bool					AtDestination( const Actor &self ) const;

private:
	Vector						_destination;
	float						_radius;
};

inline void GoDirectlyToPoint::Archive( Archiver &arc )
	
{
	Steering::Archive( arc );
	arc.ArchiveVector( &_destination );
	arc.ArchiveFloat( &_radius );
	
}

#endif // GoDirectlyToPoint
