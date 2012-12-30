//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/specialfx.cpp                                 $
// $Revision:: 4                                                              $
//   $Author:: Squirrel                                                       $
//     $Date:: 5/13/02 2:49p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /Code/DLLs/game/specialfx.cpp                                      $

#ifndef STATIONARY_VEHICLE_HPP
#define STATIONARY_VEHICLE_HPP

#include "vehicle.h"


//-------------------------- CLASS ----------------------------------
// 
// Name:			StationaryVehicle
// Base Class:		Vehicle
// 
// Description:		This class is used for all stationary type vehicles. Stationary type vehicles do not
//					move and can steer in place. An example of this would be a turret
//
// Method Of Use:	
//
//-------------------------------------------------------------------
class StationaryVehicle : public Vehicle
{
	public:
		CLASS_PROTOTYPE( StationaryVehicle );
		StationaryVehicle();
		virtual ~StationaryVehicle();

		/*virtual*/ void		Postthink();
		/*virtual*/ qboolean	Drive( usercmd_t *ucmd );
		void					PositionVehicleAndDriver(void);

		void					Killed(Event* event);

		virtual void			Archive(	Archiver &arc );


	private:
		float					_yawDeltaDegrees;
		float					_pitchDeltaDegrees;
};

inline void StationaryVehicle::Archive( Archiver &arc )
{
	Vehicle::Archive( arc );

	arc.ArchiveFloat( &_yawDeltaDegrees );
	arc.ArchiveFloat( &_pitchDeltaDegrees );
}

#endif //STATIONARY_VEHICLE_HPP
