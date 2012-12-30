//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/vehicle.h                                     $
// $Revision:: 22                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Script controlled vehicles.
//

class Vehicle;
class HorseVehicle;

#ifndef __VEHICLE_H__
#define __VEHICLE_H__

#include "g_local.h"
#include "entity.h"
#include "sentient.h"
#include "scriptslave.h"
#include "waypoints.h"

extern Event EV_Vehicle_Enter;
extern Event EV_Vehicle_Exit;
extern Event EV_Vehicle_Drivable;
extern Event EV_Vehicle_UnDrivable;
extern Event EV_Vehicle_Lock;
extern Event EV_Vehicle_UnLock;
extern Event EV_Vehicle_SeatAnglesOffset;
extern Event EV_Vehicle_SeatOffset;
extern Event EV_Vehicle_DriverAnimation;
extern Event EV_Vehicle_SetWeapon;
extern Event EV_Vehicle_ShowWeapon;
extern Event EV_Vehicle_SetSpeed;
extern Event EV_Vehicle_SetTurnRate;

extern Event EV_FollowPath_SetWayPointName;

typedef enum
	{
	CROSSHAIR_MODE_STRAIGHT,
	CROSSHAIR_MODE_RIGHT,
	CROSSHAIR_MODE_LEFT
	} crosshairMode_t;

typedef enum
	{
	JUMPMODE_START,
	JUMPMODE_HOLD,
	JUMPMODE_LAND,
	JUMPMODE_DONE
	} jumpMode_t;

typedef enum
	{
	DUCKMODE_START,
	DUCKMODE_HOLD,
	DUCKMODE_FINISH,
	DUCKMODE_DONE
	} duckMode_t;


//================================================
// VehicleMoveMode Classes
//
// The move mode classes provide a way for all new
// vehicles to easily change they're movement 
// strategy
//=================================================
class VehicleMoveMode : public Listener
	{
	public:
			CLASS_PROTOTYPE( VehicleMoveMode );
			VehicleMoveMode ();

		virtual void Move( Vehicle *vehicle );	
		virtual void HandleEvent( Event *ev );
	};

class HVMoveMode_Standard : public VehicleMoveMode
	{
	public:
			  HVMoveMode_Standard ();
		void Move( Vehicle *vehicle );
	};

class HVMoveMode_Strafe  : public VehicleMoveMode
	{
	public:
			  HVMoveMode_Strafe ();
		void Move( Vehicle *vehicle );
	};

class HVMoveMode_Locked : public VehicleMoveMode
	{
	public:
			  HVMoveMode_Locked ();
		void Move( Vehicle *vehicle );

	};

class HVMoveMode_FollowPath : public VehicleMoveMode
	{
	public:
		CLASS_PROTOTYPE( HVMoveMode_FollowPath );

			  HVMoveMode_FollowPath ();
		void Move( Vehicle *vehicle );
	   void SetWaypointName ( Event *ev );
		void HandleEvent ( Event *ev );

	private:
		void     _SetWayPoint( const str& name );
		void     _RunThread( const str &thread_name );

	private:
		WayPointNode*	  _currentWaypoint;
		str				  _currentWaypointName;
		qboolean			  _pathcompleted;	

	};


//================================================
// Old Vehicle Base class -- Needs to be changed
//================================================
class VehicleBase : public ScriptModel
	{
	public:
		VehicleBase *vlink;
		Vector		 offset;

      CLASS_PROTOTYPE( VehicleBase );

		VehicleBase();
      virtual void Archive(	Archiver &arc );
	};

inline void VehicleBase::Archive
	(
	Archiver &arc
	)
   {
   ScriptModel::Archive( arc );

   arc.ArchiveObjectPointer( ( Class ** )&vlink );
   arc.ArchiveVector( &offset );
   }


/*QUAKED script_wheelsback (0 .5 .8) ?
*/
class BackWheels : public VehicleBase
	{
	public:
      CLASS_PROTOTYPE( BackWheels );
	};

/*QUAKED script_wheelsfront (0 .5 .8) ?
*/
class FrontWheels : public VehicleBase
	{
	public:
      CLASS_PROTOTYPE( FrontWheels );
	};


class Vehicle : public VehicleBase
	{
	public:
		float					turnimpulse;
		float					pitchimpulse;
		float					moveimpulse;
		float					jumpimpulse;
		float					currentspeed;

		Vector           _DriverBBoxMaxs;
		Vector           _DriverBBoxMins;
		Vector           _originalBBoxMaxs;
		Vector           _originalBBoxMins;

		void			 CalculateOrientation();		

	protected:
		SentientPtr			driver;
		SentientPtr			lastdriver;
		float					maxturnrate;
		
		float					turnangle;
		float					pitchangle;
		
		float					speed;
      float             conesize;
      float             maxtracedist;
      str               weaponName;
      str               driveranim;
      Vector            last_origin;
      Vector            seatangles;
      Vector            seatoffset;
      Vector            driveroffset;
      Vector            Corners[4];

      qboolean          drivable;
      qboolean          locked;
      qboolean          hasweapon;
      qboolean          showweapon;
      qboolean          steerinplace;
      qboolean          jumpable;

	  qboolean			_restrictPitch;
	  float				_maximumPitch;
	  float				_minimumPitch;

	  qboolean			_restrictYaw;
	  float				_maximumYaw;
	  float				_minimumYaw;

	  float				_startYaw;
	  float				_startPitch;
	  float				_yawSeam;
	  float				_pitchSeam;

	  qboolean			_noPrediction;
	  
	  vec3_t			_oldOrigin;
	  bool				_oldWeaponName;

	  bool				_disableInventory;

		//Test stuff
		float             usetime;

		virtual void      WorldEffects( void );
		virtual void      CheckWater( void );
		virtual void		DriverUse( Event *ev );
		virtual void		VehicleStart( Event *ev );
		virtual void		VehicleTouched( Event *ev );
		virtual void		VehicleBlocked( Event *ev );
		virtual void		Postthink( void );
		virtual void		Drivable( Event *ev );
		virtual void		UnDrivable( Event *ev );
		virtual void		Jumpable( Event *ev );
		virtual void		SeatAnglesOffset( Event *ev );
		virtual void		SeatOffset( Event *ev );
		virtual void      SetDriverAngles( const Vector &angles );
		virtual void		Lock( Event *ev );
		virtual void		UnLock( Event *ev );
		virtual void		SetWeapon( Event *ev );
		virtual void		ShowWeaponEvent( Event *ev );
		virtual void		DriverAnimation( Event *ev );
		virtual void		SetSpeed( Event *ev );
		virtual void		SetTurnRate( Event *ev );
		virtual void		SteerInPlace( Event *ev );
	

	public:

      CLASS_PROTOTYPE( Vehicle );

								Vehicle();

		virtual qboolean	Drive( usercmd_t *ucmd );
		virtual qboolean	HasWeapon( void );
		virtual qboolean	ShowWeapon( void );
		Sentient				*Driver( void );
		virtual qboolean	IsDrivable( void );
		void				disableDriverInventory( void );
		void				enableDriverInventory( void );
		
		virtual void      Archive(	Archiver &arc );
		virtual void      HandleEvent( Event *ev );
		void				RestrictPitch(Event* event);
		void				RestrictRotation(Event* event);
		void				SetNoPrediction(Event* event);
		void				DisableInventory(Event* event);
	};

inline void Vehicle::Archive( Archiver &arc )
{
	VehicleBase::Archive( arc );

	arc.ArchiveFloat( &turnimpulse );
	arc.ArchiveFloat( &moveimpulse );
	arc.ArchiveFloat( &moveimpulse );
	arc.ArchiveFloat( &jumpimpulse );
	arc.ArchiveFloat( &currentspeed );

	arc.ArchiveVector( &_DriverBBoxMaxs );
	arc.ArchiveVector( &_DriverBBoxMins );
	arc.ArchiveVector( &_originalBBoxMaxs );
	arc.ArchiveVector( &_originalBBoxMins );

	arc.ArchiveSafePointer( &driver );
	arc.ArchiveSafePointer( &lastdriver );

	arc.ArchiveFloat( &maxturnrate );
	arc.ArchiveFloat( &turnangle );
	arc.ArchiveFloat( &pitchangle );

	arc.ArchiveFloat( &speed );

	arc.ArchiveFloat( &conesize );
	arc.ArchiveFloat( &maxtracedist );
	arc.ArchiveString( &weaponName );
	arc.ArchiveString( &driveranim );
	arc.ArchiveVector( &last_origin );
	arc.ArchiveVector( &seatangles );
	arc.ArchiveVector( &seatoffset );
	arc.ArchiveVector( &driveroffset );

	arc.ArchiveVector( &Corners[ 0 ] );
	arc.ArchiveVector( &Corners[ 1 ] );
	arc.ArchiveVector( &Corners[ 2 ] );
	arc.ArchiveVector( &Corners[ 3 ] );

	arc.ArchiveBoolean( &drivable );
	arc.ArchiveBoolean( &locked );
	arc.ArchiveBoolean( &hasweapon );
	arc.ArchiveBoolean( &showweapon );
	arc.ArchiveBoolean( &steerinplace );
	arc.ArchiveBoolean( &jumpable );

	arc.ArchiveBoolean(&_restrictPitch);
	arc.ArchiveFloat(&_maximumPitch);
	arc.ArchiveFloat(&_minimumPitch);
   
	arc.ArchiveBoolean(&_restrictYaw);
	arc.ArchiveFloat(&_maximumYaw);
	arc.ArchiveFloat(&_minimumYaw);

	arc.ArchiveFloat( &_startYaw );
	arc.ArchiveFloat( &_startPitch );
	arc.ArchiveFloat( &_yawSeam );
	arc.ArchiveFloat( &_pitchSeam );

	arc.ArchiveBoolean(&_noPrediction);

	arc.ArchiveFloat( &usetime );
	
	arc.ArchiveBool(&_disableInventory);
}

class DrivableVehicle : public Vehicle
	{
	public:

      CLASS_PROTOTYPE( DrivableVehicle );

								DrivableVehicle();

		virtual void		Killed( Event *ev );
	};


//=================================================
// Horse Vehicle
// A lot of this functionality needs to be redone
// in a cleaner vehicle base class
//=================================================

class HorseVehicle : public DrivableVehicle
	{
   public:

		CLASS_PROTOTYPE( HorseVehicle );
   						  HorseVehicle();							 
							 ~HorseVehicle();
		//Virtual Functions
		qboolean	Drive( usercmd_t *ucmd );
		void		Postthink( void );
		void		DriverUse( Event *ev );
		
		//Other Functions
		void     SetVehicleMoveMode( Event *ev );
		void     SetForcedForwardSpeed ( Event *ev );
		void     SetWaypointName( Event *ev );
		void     SetMinYawThreshold( Event *ev );
		void     SetMaxYawThreshold( Event *ev );
		void     SetMinPitchThreshold( Event *ev );
		void     SetMaxPitchThreshold( Event *ev );
		void     PassToMoveMode( Event *ev );
		void     AnimDone( Event *ev );
		void     DriverAnimDone ( Event *ev );
		void     HandleEvent ( Event *ev );



	private:
		void     _SetSpeed( Event *ev );

		void     _PlayMovementSound();
		void     _AnimateVehicle(const str &anim, qboolean useEvent = false );
		void     _AnimateDriver( const str &anim, qboolean useEvent = false );
		void     _PositionDriverModel();
		void     _InitializeJump();
		void     _HandleJump();
		void     _InitializeDuck();		
		void     _HandleDuck();
		void     _SetMoveMode( const str &modeName );
		void     _SetCrossHairMode();
		void     _SetBaseYaw();
		
		
	
	//Member Vars
	private:		
		
		float            _baseYaw;

		float				  _driverYaw;
		float				  _lastYaw;
		float				  _minYawThreshold;
		float				  _maxYawThreshold;
		
		float				  _driverPitch;
		float				  _lastPitch;
		float				  _minPitchThreshold;
		float				  _maxPitchThreshold;

		float				  _forcedForwardSpeed;
		float				  _jumptime;
		
		qboolean			  _jumpflag;
		qboolean			  _jumped;
		float				  _jumpSpeed;

		qboolean         _animDone;
		qboolean         _driverAnimDone;

		float            _holdtime;
		jumpMode_t       _jumpmode;
		duckMode_t       _duckmode;
	
		qboolean         _duckflag;
		qboolean         _ducked;
		qboolean         _duckheld;

		crosshairMode_t  _currentCrosshairMode;
		crosshairMode_t  _newCrosshairMode;


		
		VehicleMoveMode* _moveMode;

	};



typedef SafePtr<Vehicle> VehiclePtr;

#endif /* vehicle.h */
