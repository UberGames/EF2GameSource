//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/worldspawn.h                                  $
// $Revision:: 28                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 3/02/03 4:13p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Base class for worldspawn objects.  This should be subclassed whenever
// a DLL has new game behaviour that needs to be initialized before any other
// entities are created, or before any entity thinks each frame.  Also controls
// spawning of clients.
//

class TargetList;
class DynamicLightInfo;
class WindInfo;
class World;

#ifndef __WORLDSPAWN_H__
#define __WORLDSPAWN_H__

#include "entity.h"
#include <qcommon/qfiles.h>

typedef enum 
{
	WORLD_PHYSICS_MAXSPEED,
	WORLD_PHYSICS_AIRACCELERATE,
	WORLD_PHYSICS_GRAVITY,

	WORLD_PHYSICS_TOTAL_NUMBER
} WorldPhysicsVarTypes;

class TargetList : public Class
{
public:
	CLASS_PROTOTYPE( TargetList );
	
	Container<Entity *>		list;
	str						targetname;
	int						index;
	
						TargetList();
						TargetList( const str &tname );
						~TargetList();
	
	void				AddEntity( Entity * ent );
	void				RemoveEntity( Entity * ent );
	Entity				*GetNextEntity( Entity * ent );
	int					GetNumTargets()						{ return list.NumObjects() ; }
	
	virtual void		Archive( Archiver &arc );
};

class DynamicLightInfo
{
public:
	float		defaultIntensity;
	
	float		intensity;
	
	float		start_intensity;
	float		end_intensity;
	
	float		start_fade_time;
	float		fade_time;
	
	str			lightstyle;
	int			current_lightstyle_position;
	qboolean	stop_at_end;
	
					DynamicLightInfo();
	virtual void	Archive( Archiver &arc );
};

inline void DynamicLightInfo::Archive( Archiver &arc )
{
	arc.ArchiveFloat( &defaultIntensity );

	arc.ArchiveFloat( &intensity );
	arc.ArchiveFloat( &start_intensity );
	arc.ArchiveFloat( &end_intensity );

	arc.ArchiveFloat( &start_fade_time );
	arc.ArchiveFloat( &fade_time );

	arc.ArchiveString( &lightstyle );
	arc.ArchiveInteger( &current_lightstyle_position );
	arc.ArchiveBoolean( &stop_at_end );
}

class WindInfo
{
public:
	Vector		direction;
	float		intensity;
	
	float		start_intensity;
	float		end_intensity;
	
	float		min_intensity;
	float		max_intensity;
	
	float		start_fade_time;
	float		fade_time;
	
	float		gust_time;
	
	float		max_change;
	
					WindInfo();
	virtual void	Archive( Archiver &arc );
};

inline void WindInfo::Archive( Archiver &arc )
{
	arc.ArchiveVector( &direction );
	arc.ArchiveFloat( &intensity );
	arc.ArchiveFloat( &start_intensity );
	arc.ArchiveFloat( &end_intensity );

	arc.ArchiveFloat( &min_intensity );
	arc.ArchiveFloat( &max_intensity );

	arc.ArchiveFloat( &start_fade_time );
	arc.ArchiveFloat( &fade_time );

	arc.ArchiveFloat( &gust_time );

	arc.ArchiveFloat( &max_change );
}

class WeatherInfo
{
public:
	weather_t	type;
	int			intensity;
	
					WeatherInfo();
	virtual void	Archive( Archiver &arc );
};

inline void WeatherInfo::Archive( Archiver &arc )
{
	ArchiveEnum( type, weather_t );
	arc.ArchiveInteger( &intensity );
}

class World : public Entity
{
private:
	Container<TargetList *>		targetList;
	Container<str>				_brokenThings;
	Container<str>				_availableViewModes;
	qboolean					world_dying;
	
public:
	CLASS_PROTOTYPE( World );
	
	str							skipthread;
	float						farplane_distance;
	Vector						farplane_color;
	qboolean					farplane_cull;
	qboolean					farplane_fog;
	
	qboolean					terrain_global;
	float						terrain_global_min;
	
	float						entity_fade_dist;
	
	DynamicLightInfo			dynamic_lights[ MAX_LIGHTING_GROUPS ];
	
	WindInfo					wind;
	
	WeatherInfo					weather;
	float						time_scale;
	
	float						sky_alpha;
	qboolean					sky_portal;
	
	// Movement info
	float						_physicsInfo[ WORLD_PHYSICS_TOTAL_NUMBER ];

	bool						_canShakeCamera;
	
	World();
	~World();
	
	void			FreeTargetList( void );
	//TargetList     *GetTargetList( const str &targetname );
	TargetList		*GetTargetList( const str &targetname, qboolean createnew = true );
	TargetList		*GetTargetList( int index );
	void			AddTargetEntity( const str &targetname, Entity * ent );
	void			RemoveTargetEntity( const str &targetname, Entity * ent );
	Entity			*GetNextEntity( const str &targetname, Entity * ent );
	void			SetSoundtrack( Event *ev );
	void			SetSkipThread( Event *ev );
	void			SetNextMap( Event *ev );
	void			SetMessage( Event *ev );
	void			SetScript( Event *ev );
	void			SetWaterColor( Event *ev );
	void			SetWaterAlpha( Event *ev );
	void			SetSlimeColor( Event *ev );
	void			SetSlimeAlpha( Event *ev );
	void			SetLavaColor( Event *ev );
	void			SetLavaAlpha( Event *ev );
	void			SetFarPlane_Color( Event *ev );
	void			SetFarPlane_Cull( Event *ev );
	void			SetFarPlane_Fog( Event *ev );
	void			SetFarPlane( Event *ev );
	void			SetTerrainGlobal( Event *ev );
	void			SetTerrainGlobalMin( Event *ev );
	void			SetEntityFadeDist( Event *ev );
	void			SetSkyAlpha( Event *ev );
	void			SetSkyPortal( Event *ev );
	void			UpdateConfigStrings( void );
	void			UpdateFog( void );
	void			UpdateTerrain( void );
	void			UpdateEntityFadeDist( void );
	void			UpdateDynamicLights( void );
	void			UpdateWindDirection( void );
	void			UpdateWindIntensity( void );
	void			UpdateWeather( void );
	void			UpdateTimeScale( void );
	void			UpdateSky( void );
	
	void			SetLightIntensity( Event *ev );
	void			SetLightDefaultIntensity( Event *ev );
	void			SetLightFade( Event *ev );
	void			SetLightLightstyle( Event *ev );
	void			GetLightIntensity( Event *ev );
	
	void			SetWindDensity( Event *ev );
	void			SetWindDirection( Event *ev );
	void			SetWindIntensity( Event *ev );
	void			SetWindGust( Event *ev );
	void			SetWindFade( Event *ev );
	void			SetWindMinMax( Event *ev );
	void			SetWeather( Event *ev );
	
	int				worldPhysicsVarNameToIndex( const char *varName );
	void			setPhysicsVar( Event *ev );
	void			setPhysicsVar( const char *varName, float varValue );
	float			getPhysicsVar( const char *varName );
	float			getPhysicsVar( int index );
	
	void			SetTimeScale( Event *ev );
	
	int				findBrokenThing( const char *name );
	void			addBrokenThing( const char *name );
	void			removeBrokenThing( const char *name );
	bool			isThingBroken( const char *name );
	void			addBrokenThing( Event *ev );
	void			removeBrokenThing( Event *ev );
	void			freeAllBrokenThings( void );
	
	void			addAvailableViewMode( Event *ev );
	void			removeAvailableViewMode( Event *ev );
	void			clearAvailableViewModes( Event *ev );
	bool			isAnyViewModeAvailable( void );
	bool			isViewModeAvailable( const char *name );

	void			setCanShakeCamera( Event *ev );
	bool			canShakeCamera( void );
	
	void			Think( void );
	
	virtual void	Archive( Archiver &arc );
};

inline void TargetList::Archive( Archiver &arc )
{
	int num, i;

	Class::Archive( arc );

	if ( arc.Saving() )
		num = list.NumObjects();

	arc.ArchiveInteger( &num );

	if ( arc.Loading() ) 
	{
		list.ClearObjectList();
		list.Resize( num );
	}

	for ( i = 1; i <= num; i++ )
	{
		arc.ArchiveObjectPointer( ( Class ** )list.AddressOfObjectAt( i ) );
	}

	arc.ArchiveString( &targetname );
	arc.ArchiveInteger( &index );
}

inline TargetList *World::GetTargetList( int index )
{
	return targetList.ObjectAt( index );
}

typedef SafePtr<World> WorldPtr;
extern WorldPtr world;

#endif /* worldspawn.h */
