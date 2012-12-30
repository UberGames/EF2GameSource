//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/worldspawn.cpp                            $
// $Revision:: 44                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
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

#include "_pch_cpp.h"
#include "entity.h"
#include "scriptmaster.h"
#include "worldspawn.h"
#include "gravpath.h"
#include "earthquake.h"
#include "specialfx.h"
#include "soundman.h"
#include "mp_manager.hpp"
#include "groupcoordinator.hpp"

WorldPtr  world;

#define DEFAULT_ENTITY_FADE_DIST  3000

char worldPhysicsVarNames[ WORLD_PHYSICS_TOTAL_NUMBER ][ 32 ] =
{
	"maxSpeed",
	"airAccelerate",
	"gravity"
};

/*****************************************************************************/
/*QUAKED worldspawn (0 0 0) ? CINEMATIC

Only used for the world.

"soundtrack" the soundtrack to use on the map
"skipthread" thread that is activated to skip this level (if cinematic)
"nextmap"    map to goto when player exits
"message"	 text to print at user logon
"script"	    script to run on start of map
"watercolor" view color when underwater
"wateralpha" view alpha when underwater
"lavacolor"  view alpha when in lava
"lavaalpha"  view alpha when in lava
"farplane_color" color to fade to when the far clip plane is on
"farplane_cull" whether or not the far plane should cull, default is yes
"farplane_fog" whether or not to use fog with farplane
"farplane"   distance from the viewer that the far clip plane is
"ambientlight" ambient lighting to be applied to all entities
"ambient"    ambient lighting to be applied to all entities, use _color to specify color
"suncolor"   color of the sun in the level
"sunlight"   intensity of the sun in the level
"sundirection" direction of the sun in the level
"sunflare"   worldspace position of the sun flare
"sunflare_inportalsky" whether or not the flare is in the portal sky
"lightmapdensity" default lightmap density to be used for all surfaces
"skyalpha"   initial value of the sky's alpha, defaults to 1

******************************************************************************/

#define CINEMATIC 1

Event EV_World_SetSoundtrack
(
	"soundtrack",
	EV_SCRIPTONLY,
	"s",
	"MusicFile",
	"Set music soundtrack for this level."
);
Event EV_World_SetSkipThread
(
	"skipthread",
	EV_SCRIPTONLY,
	"s",
	"skipThread",
	"Set the thread for skipping cinematics"
);
Event EV_World_SetNextMap
(
	"nextmap",
	EV_SCRIPTONLY,
	"s",
	"nextMap",
	"Set the next map to change to"
);
Event EV_World_SetMessage
(
	"message",
	EV_SCRIPTONLY,
	"s",
	"worldMessage",
	"Set a message for the world"
);
Event EV_World_SetScript
(
	"script",
	EV_SCRIPTONLY,
	"s",
	"worldScript",
	"Set the script for this map"
);
Event EV_World_SetWaterColor
(
	"watercolor",
	EV_SCRIPTONLY,
	"v",
	"color_water",
	"Set the watercolor screen blend"
);
Event EV_World_SetWaterAlpha
(
	"wateralpha",
	EV_SCRIPTONLY,
	"f",
	"waterAlpha",
	"Set the alpha of the water screen blend"
);
Event EV_World_SetSlimeColor
(
	"slimecolor",
	EV_SCRIPTONLY,
	"v",
	"color_slime",
	"Set the slimecolor screen blend"
);
Event EV_World_SetSlimeAlpha
(
	"slimealpha",
	EV_SCRIPTONLY,
	"f",
	"slimeAlpha",
	"Set the alpha of the slime screen blend"
);
Event EV_World_SetLavaColor
(
	"lavacolor",
	EV_SCRIPTONLY,
	"v",
	"color_lava",
	"Set the color of lava screen blend"
);
Event EV_World_SetLavaAlpha
(
	"lavaalpha",
	EV_SCRIPTONLY,
	"f",
	"lavaAlpha",
	"Set the alpha of lava screen blend"
);
Event EV_World_SetFarPlane_Color
(
	"farplane_color",
	EV_SCRIPTONLY,
	"v",
	"color_farplane",
	"Set the color of the far clipping plane fog"
);
Event EV_World_SetFarPlane_Cull
(
	"farplane_cull",
	EV_SCRIPTONLY,
	"b",
	"farplaneCull",
	"Whether or not the far clipping plane should cull things out of the world"
);
Event EV_World_SetFarPlane_Fog
(
	"farplane_fog",
	EV_SCRIPTONLY,
	"b",
	"farplaneFog",
	"Whether or not to use fog with far plane"
);
Event EV_World_SetFarPlane
(
	"farplane",
	EV_SCRIPTONLY,
	"f",
	"farplaneDistance",
	"Set the distance of the far clipping plane"
);
Event EV_World_SetTerrainGlobal
(
	"terrainglobal",
	EV_SCRIPTONLY,
	"b",
	"terrain_global_bool",
	"Turns the terrain drawing as global on or off"
);
Event EV_World_SetTerrainGlobalMin
(
	"terrainglobalmin",
	EV_SCRIPTONLY,
	"f",
	"terrain_global_min",
	"Turns the terrain drawing as global on and sets the minimum"
);
Event EV_World_SetEntityFadeDist
(
	"entity_fade_dist",
	EV_SCRIPTONLY,
	"f",
	"entity_fade_dist",
	"Sets the default distance to fade out entities"
);
Event EV_World_SetLightIntensity
(
	"light_intensity",
	EV_SCRIPTONLY,
	"sf",
	"light_group_name light_intensity",
	"Sets the intensity of a dynamic light"
);
Event EV_World_SetLightDefaultIntensity
(
   "light_default_intensity",
   EV_SCRIPTONLY,
   "sf",
   "light_group_name light_intensity",
   "Sets the default intensity of a dynamic light.  This is used only when r_dynamiclightmaps is 0.\n"
   "A value below 0 specifies that this light has no default value and will always be dynamic"
);
Event EV_World_SetLightFade
(
	"light_fade",
	EV_SCRIPTONLY,
	"sff",
	"light_group_name end_light_intensity fade_time",
	"Sets a up a dynamic light to fade"
);
Event EV_World_SetLightLightstyle
(
	"light_lightstyle",
	EV_SCRIPTONLY,
	"ssB",
	"light_group_name light_style stop_at_end",
	"Sets the lightstyle of a dynamic light"
);
Event EV_World_GetLightIntensity
(
	"getlightintensity",
	EV_SCRIPTONLY,
	"@is",
	"light_intensity light_group_name",
	"Gets the intensity of a dynamic light"
);
Event EV_World_SetWindDirection
(
	"wind_direction",
	EV_SCRIPTONLY,
	"v",
	"wind_direction",
	"Sets the direction of the wind"
);
Event EV_World_SetWindIntensity
(
	"wind_intensity",
	EV_SCRIPTONLY,
	"f",
	"wind_intensity",
	"Sets the intensity of the wind"
);
Event EV_World_SetWindGust
(
	"wind_gust",
	EV_SCRIPTONLY,
	"fff",
	"wind_intensity fade_time gust_time",
	"Creates a gust of wind"
);
Event EV_World_SetWindFade
(
	"wind_fade",
	EV_SCRIPTONLY,
	"ff",
	"end_wind_intensity fade_time",
	"Fades the wind intensity to the new value"
);
Event EV_World_SetWindMinMax
(
	"wind_minmax",
	EV_SCRIPTONLY,
	"ffF",
	"min_wind_intensity min_wind_intensity max_change_per_frame",
	"Sets the min & max intensity of the wind"
);
Event EV_World_SetWeather
(
	"weather",
	EV_SCRIPTONLY,
	"sf",
	"type intensity",
	"Sets the current weather"
);
Event EV_World_SetPhysicsVar
(
	"physicsVar",
	EV_SCRIPTONLY,
	"sf",
	"physicsVarName physicsVerValue",
	"Sets one of the world's physics variables"
);
Event EV_World_SetTimeScale
(
	"time_scale",
	EV_SCRIPTONLY,
	"f",
	"time_scale",
	"Sets the time scale for the world"
);
Event EV_World_SetAmbientLight
(
	"ambientlight",
	EV_SCRIPTONLY,
	"b",
	"ambientLight",
	"Set whether or not ambient light should be used"
);
Event EV_World_SetAmbientIntensity
(
	"ambient",
	EV_SCRIPTONLY,
	"f",
	"ambientIntensity",
	"Set the intensity of the ambient light"
);
Event EV_World_SetSunColor
(
	"suncolor",
	EV_SCRIPTONLY,
	"v",
	"sunColor",
	"Set the color of the sun"
);
Event EV_World_SetSunLight
(
	"sunlight",
	EV_SCRIPTONLY,
	"b",
	"sunlight",
	"Set whether or not there should be sunlight"
);
Event EV_World_SetSunDirection
(
	"sundirection",
	EV_SCRIPTONLY,
	"v",
	"sunlightDirection",
	"Set the direction of the sunlight"
);
Event EV_World_LightmapDensity
(
	"lightmapdensity",
	EV_SCRIPTONLY,
	"f",
	"density",
	"Set the default lightmap density for all world surfaces"
);
Event EV_World_SunFlare
(
	"sunflare",
	EV_SCRIPTONLY,
	"v",
	"position_of_sun",
	"Set the position of the sun for the purposes of the sunflare"
);
Event EV_World_SunFlareInPortalSky
(
	"sunflare_inportalsky",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Let the renderer know that the sun is in the portal sky"
);
Event EV_World_SetSkyAlpha
(
	"skyalpha",
	EV_SCRIPTONLY,
	"f",
	"newAlphaForPortalSky",
	"Set the alpha on the sky"
);
Event EV_World_SetSkyPortal
(
	"skyportal",
	EV_SCRIPTONLY,
	"b",
	"newSkyPortalState",
	"Whether or not to use the sky portal at all"
);
Event EV_World_AddBroken
(
	"addbroken",
	EV_SCRIPTONLY,
	"s",
	"brokenThingName",
	"Adds this thing to the broken list"
);
Event EV_World_RemoveBroken
(
	"removebroken",
	EV_SCRIPTONLY,
	"s",
	"brokenThingName",
	"Removes this thing to the broken list"
);
Event EV_World_AddAvailableViewMode
(
	"addAvailableViewMode",
	EV_SCRIPTONLY,
	"s",
	"modeName",
	"Adds this view mode as being available"
);
Event EV_World_RemoveAvailableViewMode
(
	"removeAvailableViewMode",
	EV_SCRIPTONLY,
	"s",
	"modeName",
	"Removes this view mode as being available"
);
Event EV_World_ClearAvailableViewModes
(
	"clearAvailableViewModes",
	EV_SCRIPTONLY,
	NULL,
	NULL,
	"Clears all available modes"
);
Event EV_World_CanShakeCamera
(
	"canShakeCamera",
	EV_SCRIPTONLY,
	"b",
	"canShakeCamera",
	"Specifies whether or not cinematic camera's can shake from earthquakes."
);

CLASS_DECLARATION( Entity, World, "worldspawn" )
{
	{ &EV_World_SetSoundtrack,				&World::SetSoundtrack },
	{ &EV_World_SetSkipThread,				&World::SetSkipThread },
	{ &EV_World_SetNextMap,					&World::SetNextMap },
	{ &EV_World_SetMessage,					&World::SetMessage },
	{ &EV_World_SetScript,					&World::SetScript },
	{ &EV_World_SetWaterColor,				&World::SetWaterColor },
	{ &EV_World_SetWaterAlpha,				&World::SetWaterAlpha },
	{ &EV_World_SetSlimeColor,				&World::SetSlimeColor },
	{ &EV_World_SetSlimeAlpha,				&World::SetSlimeAlpha },
	{ &EV_World_SetLavaColor,				&World::SetLavaColor },
	{ &EV_World_SetLavaAlpha,				&World::SetLavaAlpha },
	{ &EV_World_SetFarPlane_Color,			&World::SetFarPlane_Color },
	{ &EV_World_SetFarPlane_Cull,			&World::SetFarPlane_Cull },
	{ &EV_World_SetFarPlane_Fog,			&World::SetFarPlane_Fog },
	{ &EV_World_SetFarPlane,				&World::SetFarPlane },
	{ &EV_World_SetTerrainGlobal,			&World::SetTerrainGlobal },
	{ &EV_World_SetTerrainGlobalMin,		&World::SetTerrainGlobalMin },
	{ &EV_World_SetEntityFadeDist,			&World::SetEntityFadeDist },
	{ &EV_World_SetLightIntensity,			&World::SetLightIntensity },
	{ &EV_World_SetLightDefaultIntensity,	&World::SetLightDefaultIntensity },
	{ &EV_World_SetLightFade,				&World::SetLightFade },
	{ &EV_World_SetLightLightstyle,			&World::SetLightLightstyle },
	{ &EV_World_GetLightIntensity,			&World::GetLightIntensity },
	{ &EV_World_SetWindDirection,			&World::SetWindDirection },
	{ &EV_World_SetWindIntensity,			&World::SetWindIntensity },
	{ &EV_World_SetWindGust,				&World::SetWindGust },
	{ &EV_World_SetWindFade,				&World::SetWindFade },
	{ &EV_World_SetWindMinMax,				&World::SetWindMinMax },
	{ &EV_World_SetWeather,					&World::SetWeather },
	{ &EV_World_SetPhysicsVar,				&World::setPhysicsVar },
	{ &EV_World_SetTimeScale,				&World::SetTimeScale },
	{ &EV_World_SetSkyAlpha,				&World::SetSkyAlpha },
	{ &EV_World_SetSkyPortal,				&World::SetSkyPortal },
	{ &EV_World_SetAmbientLight,			NULL },
	{ &EV_World_SetAmbientIntensity,		NULL },
	{ &EV_World_SetSunColor,				NULL },
	{ &EV_World_SetSunLight,				NULL },
	{ &EV_World_SetSunDirection,			NULL },
	{ &EV_World_LightmapDensity,			NULL },
	{ &EV_World_SunFlare,					NULL },
	{ &EV_World_SunFlareInPortalSky,		NULL },
	{ &EV_World_AddBroken,					&World::addBrokenThing },
	{ &EV_World_RemoveBroken,				&World::removeBrokenThing },
	{ &EV_World_AddAvailableViewMode,		&World::addAvailableViewMode },
	{ &EV_World_RemoveAvailableViewMode,	&World::removeAvailableViewMode },
	{ &EV_World_ClearAvailableViewModes,	&World::clearAvailableViewModes },
	{ &EV_World_CanShakeCamera,				&World::setCanShakeCamera },

	{ NULL, NULL }
};

World::World()
{
	const char  *text;
	str         mapname;
	int		   i;

	assert( this->entnum == ENTITYNUM_WORLD );

	world = this;
	world_dying = false;

	setMoveType( MOVETYPE_NONE );
	setSolidType( SOLID_BSP );

	// world model is always index 1
	edict->s.modelindex = 1;
	model = "*1";

	turnThinkOn();

	UpdateConfigStrings();

	groupcoordinator = NULL;

	// Anything that modifies configstrings, or spawns things is ignored when loading savegames
	if ( LoadingSavegame )
	{
		return;
	}

	// clear out the soundtrack from the last level
	ChangeSoundtrack( "" );

	// set the default farplane parameters
	farplane_distance = 0;
	farplane_color = Vector(0, 0, 0);
	farplane_cull = true;
	farplane_fog = true;
	UpdateFog();

	terrain_global = false;
	terrain_global_min = MIN_WORLD_COORD;
	UpdateTerrain();

	entity_fade_dist = DEFAULT_ENTITY_FADE_DIST;
	UpdateEntityFadeDist();

	UpdateDynamicLights();

	UpdateWeather();

	time_scale = 1.0f;
	sky_alpha = 1.0f;
	sky_portal = true;
	UpdateSky();

	//
	// see if this is a cinematic level
	//
	level.cinematic = ( spawnflags & CINEMATIC ) ? true : false;

	if ( level.cinematic )
		gi.cvar_set( "sv_cinematic", "1" );
	else
		gi.cvar_set( "sv_cinematic", "0" );

	level.nextmap = "";
	level.level_name = level.mapname;

	// Set up the mapname as the default script
	mapname = "maps/";
	mapname += level.mapname;
	for( i = mapname.length() - 1; i >= 0; i-- )
	{
		if ( mapname[ i ] == '.' )
		{
			mapname[ i ] = 0;
			break;
		}
	}

	mapname += ".scr";
	text = &mapname[ 5 ];

	// If there isn't a script with the same name as the map, then don't try to load script
	if ( gi.FS_ReadFile( mapname.c_str(), NULL, true ) != -1 )
	{
		gi.DPrintf( "Adding script: '%s'\n", text );

		// just set the script, we will start it in G_Spawn
		level.SetGameScript( mapname.c_str() );
	}
	else
	{
		level.SetGameScript( "" );
	}

	level.consoleThread = Director.CreateThread();

	SoundMan.Init();
	SoundMan.Load();

	// Set the color for the blends.
	level.water_color       = Vector( 0.0f, 0.0f, 0.5f );
	level.water_alpha       = 0.4f;

	level.slime_color       = Vector( 0.2f, 0.4f, 0.2f );
	level.slime_alpha       = 0.6f;

	level.lava_color        = Vector( 0.5f, 0.15f, 0.0f );
	level.lava_alpha        = 0.6f;

	//
	// set the targetname of the world
	//
	SetTargetName( "world" );

	groupcoordinator = new GroupCoordinator;

	// Initialize movement info

	for ( i = 0 ; i < WORLD_PHYSICS_TOTAL_NUMBER ; i++ )
	{
		_physicsInfo[ i ] = -1.0f;
	}

	_canShakeCamera = false;
}

void World::UpdateConfigStrings( void )
{
	//
	// make some data visible to connecting client
	//
	gi.setConfigstring( CS_GAME_VERSION, GAME_VERSION );
	gi.setConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

	// make some data visible to the server
	gi.setConfigstring( CS_NAME, level.level_name.c_str() );
}

void World::UpdateFog( void )
{
	gi.SetFarPlane( farplane_distance, farplane_cull );
	gi.setConfigstring( CS_FOGINFO, va( "%d %d %.0f %.4f %.4f %.4f", farplane_cull, farplane_fog, farplane_distance, farplane_color.x, farplane_color.y, farplane_color.z ) );
}

void World::UpdateTerrain( void )
{
	//gi.SetFarPlane( farplane_distance );
	gi.setConfigstring( CS_TERRAININFO, va( "%d %.0f", terrain_global, terrain_global_min ) );
}

void World::UpdateEntityFadeDist( void )
{
	gi.setConfigstring( CS_ENTITYFADEINFO, va( "%f", entity_fade_dist ) );
}

void World::UpdateDynamicLights( void )
{
	int i;

	for ( i = 0 ; i < MAX_LIGHTING_GROUPS ; i++ )
	{
		gi.SetDynamicLight( i, dynamic_lights[ i ].intensity );
		gi.SetDynamicLightDefault( i, dynamic_lights[ i ].defaultIntensity );
	}
}

void World::UpdateWindDirection( void )
{
	gi.SetWindDirection( wind.direction );
}

void World::UpdateWindIntensity( void )
{
	gi.SetWindIntensity( wind.intensity );
}

void World::UpdateTimeScale( void )
{
	gi.SetTimeScale( time_scale );
}

void World::UpdateWeather( void )
{
	gi.SetWeatherInfo( weather.type, weather.intensity );
}

void World::UpdateSky( void )
{
	gi.SetSkyPortal( sky_portal );
	gi.setConfigstring( CS_SKYINFO, va( "%.4f %d", sky_alpha, sky_portal ) );
}


void World::SetSoundtrack( Event *ev )
{
	const char *text;

	text = ev->GetString( 1 );
	ChangeSoundtrack( text );
}

void World::SetFarPlane( Event *ev )
{
	farplane_distance = ev->GetFloat( 1 );
	UpdateFog();
}

void World::SetFarPlane_Color( Event *ev )
{
	farplane_color = ev->GetVector( 1 );
	UpdateFog();
}

void World::SetFarPlane_Cull( Event *ev )
{
	farplane_cull = ev->GetBoolean( 1 );
	UpdateFog();
}

void World::SetFarPlane_Fog( Event *ev )
{
	farplane_fog = ev->GetBoolean( 1 );
	UpdateFog();
}

void World::SetTerrainGlobal( Event *ev )
{
	terrain_global = ev->GetBoolean( 1 );
	terrain_global_min = MIN_WORLD_COORD;
	UpdateTerrain();
}

void World::SetTerrainGlobalMin( Event *ev )
{
	terrain_global = true;
	terrain_global_min = ev->GetFloat( 1 );
	UpdateTerrain();
}

void World::SetEntityFadeDist( Event *ev )
{
	entity_fade_dist = ev->GetFloat( 1 );
	UpdateEntityFadeDist();
}

void World::SetLightIntensity( Event *ev )
{
	str light_group_name;
	int light_group_num;
	float light_intensity;

	// Get the light group

	light_group_name = ev->GetString( 1 );
	light_intensity  = ev->GetFloat( 2 );

	light_group_num = gi.GetLightingGroup( light_group_name );

	// Make sure this is a valid light group

	if ( light_group_num == -1 )
	{
		gi.WDPrintf( "Unknown light group %s\n", light_group_name.c_str() );
		return;
	}

	// Set the light info

	dynamic_lights[ light_group_num ].intensity = light_intensity;
	dynamic_lights[ light_group_num ].fade_time = 0;
	dynamic_lights[ light_group_num ].lightstyle = "";
}

//----------------------------------------------------------------
// Name:			SetLightDefaultIntensity
// Class:			World
//
// Description:		Sets the default light intensity for a dynamic light
//
// Parameters:		Event *ev					- contains the following
//												  str light_group_name 
//												  float light_intensity
//
// Returns:			
//----------------------------------------------------------------

void World::SetLightDefaultIntensity( Event *ev )
{
	str light_group_name;
	int light_group_num;
	float light_intensity;

	// Get the light group

	light_group_name = ev->GetString( 1 );
	light_intensity  = ev->GetFloat( 2 );

	light_group_num = gi.GetLightingGroup( light_group_name );

	// Make sure this is a valid light group

	if ( light_group_num == -1 )
	{
		gi.WDPrintf( "Unknown light group %s\n", light_group_name.c_str() );
		return;
	}

	// Set the light info

	dynamic_lights[ light_group_num ].defaultIntensity = light_intensity;
}

void World::SetLightFade( Event *ev )
{
	str light_group_name;
	int light_group_num;
	float light_end_intensity;
	float fade_time;

	// Get the light group

	light_group_name = ev->GetString( 1 );
	light_end_intensity  = ev->GetFloat( 2 );
	fade_time  = ev->GetFloat( 3 );

	light_group_num = gi.GetLightingGroup( light_group_name );

	// Make sure this is a valid light group

	if ( light_group_num == -1 )
	{
		gi.WDPrintf( "Unknown light group %s\n", light_group_name.c_str() );
		return;
	}

	// Set the light info

	dynamic_lights[ light_group_num ].start_intensity = dynamic_lights[ light_group_num ].intensity;
	dynamic_lights[ light_group_num ].end_intensity = light_end_intensity;

	dynamic_lights[ light_group_num ].start_fade_time = level.time;
	dynamic_lights[ light_group_num ].fade_time = fade_time;
}

void World::SetLightLightstyle( Event *ev )
{
	str light_group_name;
	int light_group_num;
	str lightstyle;
	qboolean stop_at_end;

	// Get the light group

	light_group_name = ev->GetString( 1 );
	lightstyle  = ev->GetString( 2 );

	if ( ev->NumArgs() > 2 )
		stop_at_end = ev->GetBoolean( 3 );
	else
		stop_at_end = false;

	light_group_num = gi.GetLightingGroup( light_group_name );

	// Make sure this is a valid light group

	if ( light_group_num == -1 )
	{
		gi.WDPrintf( "Unknown light group %s\n", light_group_name.c_str() );
		return;
	}

	// Set the light info

	dynamic_lights[ light_group_num ].lightstyle = lightstyle;
	dynamic_lights[ light_group_num ].current_lightstyle_position = -1;
	dynamic_lights[ light_group_num ].stop_at_end = stop_at_end;
}

void World::GetLightIntensity( Event *ev )
{
	str light_group_name;
	int light_group_num;

	// Get the light group

	light_group_name = ev->GetString( 1 );

	light_group_num = gi.GetLightingGroup( light_group_name );

	// Make sure this is a valid light group

	if ( light_group_num == -1 )
	{
		gi.WDPrintf( "Unknown light group %s\n", light_group_name.c_str() );
		return;
	}

	ev->ReturnFloat( dynamic_lights[ light_group_num ].intensity );
}

void World::SetWindDirection( Event *ev )
{
	wind.direction = ev->GetVector( 1 );
}

void World::SetWindIntensity( Event *ev )
{
	wind.intensity = ev->GetFloat( 1 );

	wind.gust_time = 0;
	wind.fade_time = 0;
	wind.min_intensity = -1;
	wind.max_intensity = -1;
}

void World::SetWindGust( Event *ev )
{
	wind.start_intensity = wind.intensity;
	wind.end_intensity = ev->GetFloat( 1 );

	wind.start_fade_time = level.time;
	wind.fade_time = ev->GetFloat( 2 );

	wind.gust_time = ev->GetFloat( 3 );

	wind.min_intensity = -1;
	wind.max_intensity = -1;
}

void World::SetWindFade( Event *ev )
{
	wind.start_intensity = wind.intensity;
	wind.end_intensity = ev->GetFloat( 1 );

	wind.start_fade_time = level.time;
	wind.fade_time = ev->GetFloat( 2 );

	wind.gust_time = 0;

	wind.min_intensity = -1;
	wind.max_intensity = -1;
}

void World::SetWindMinMax( Event *ev )
{
	wind.fade_time = 0;

	wind.gust_time = 0;

	wind.min_intensity = ev->GetFloat( 1 );
	wind.max_intensity = ev->GetFloat( 2 );

	if ( ev->NumArgs() > 2 )
		wind.max_change = ev->GetFloat( 3 );
	else
		wind.max_change = ( wind.max_intensity - wind.min_intensity ) * 0.05f;
}

void World::SetWeather( Event *ev )
{
	str weather_name;

	weather_name = ev->GetString( 1 );

	if ( weather_name == "rain" )
		weather.type = WEATHER_RAIN;
	else if ( weather_name == "rain_plain" )
		weather.type = WEATHER_RAIN_PLAIN;
	else if ( weather_name == "snow" )
		weather.type = WEATHER_SNOW;
	else
		weather.type = WEATHER_NONE;

	weather.intensity = ev->GetFloat( 2 );

	UpdateWeather();
}

void World::SetTimeScale( Event *ev )
{
	time_scale = ev->GetFloat( 1 );
}

void World::SetSkyAlpha( Event *ev )
{
	sky_alpha = ev->GetFloat( 1 );
	UpdateSky();
}

void World::SetSkyPortal( Event *ev )
{
	sky_portal = ev->GetBoolean( 1 );
	UpdateSky();
}


void World::SetSkipThread( Event *ev )
{
	str label;

	label = ev->GetString( 1 );
	if ( label.length() && label.icmp( "null" ) )
	{
		skipthread = label;
	}
	else
	{
		skipthread = "";
	}
}

void World::SetNextMap( Event *ev )
{
	level.nextmap = ev->GetString( 1 );
}

void World::SetMessage( Event *ev )
{
	const char *text;

	text = ev->GetString( 1 );
	level.level_name = text;
	gi.setConfigstring( CS_NAME, text );
}

void World::SetScript( Event *ev )
{
	str text;
	str scriptname;

	text = ev->GetString( 1 );

	gi.DPrintf( "Adding script: '%s'\n", text.c_str() );
	scriptname = "maps/" + text;

	// just set the script, we will start it in G_Spawn
	level.SetGameScript( scriptname.c_str() );
}

void World::SetWaterColor( Event *ev )
{
	level.water_color = ev->GetVector( 1 );
}

void World::SetWaterAlpha( Event *ev )
{
	level.water_alpha = ev->GetFloat( 1 );
}

void World::SetSlimeColor( Event *ev )
{
	level.slime_color = ev->GetVector( 1 );
}

void World::SetSlimeAlpha( Event *ev )
{
	level.slime_alpha = ev->GetFloat( 1 );
}

void World::SetLavaColor( Event *ev )
{
	level.lava_color = ev->GetVector( 1 );
}

void World::SetLavaAlpha( Event *ev )
{
	level.lava_alpha = ev->GetFloat( 1 );
}

TargetList * World::GetTargetList( const str &targetname, qboolean createnew )
{
	TargetList * targetlist;
	int i;

	for( i = 1; i <= targetList.NumObjects(); i++ )
	{
		targetlist = targetList.ObjectAt( i );
		if ( targetname == targetlist->targetname)
		{
			return targetlist;
		}
	}

	if ( !createnew )
	{
		return NULL;
	}

	targetlist = new TargetList( targetname );
	targetlist->index = targetList.AddObject( targetlist );

	return targetlist;
}

void World::AddTargetEntity( const str &targetname, Entity * ent )
{
	TargetList * targetlist;

	targetlist = GetTargetList( targetname );
	targetlist->AddEntity( ent );
}

void World::RemoveTargetEntity( const str &targetname, Entity * ent )
{
	TargetList * targetlist;

	if ( world_dying )
		return;

	targetlist = GetTargetList( targetname );
	targetlist->RemoveEntity( ent );
}

Entity * World::GetNextEntity( const str &targetname, Entity * ent )
{
	TargetList * targetlist;

	targetlist = GetTargetList( targetname );
	return targetlist->GetNextEntity( ent );
}

World::~World()
{
	world_dying = true;
	FreeTargetList();

	if ( groupcoordinator )
		delete groupcoordinator;

	groupcoordinator = NULL;

	freeAllBrokenThings();
}

void World::FreeTargetList( void )
{
	int i;
	int num;

	num = targetList.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		delete targetList.ObjectAt( i );
	}

	targetList.FreeObjectList();
}

void World::Think( void )
{
	int i;
	DynamicLightInfo *dlight;

	// Do dynamic light stuff

	for ( i = 0 ; i < MAX_LIGHTING_GROUPS ; i++ )
	{
		dlight = &dynamic_lights[ i ];

		if ( dlight->fade_time )
		{
			if ( level.time > ( dlight->start_fade_time + dlight->fade_time ) )
			{
				dlight->intensity = dlight->end_intensity;
				dlight->fade_time = 0;
			}
			else
			{
				dlight->intensity = dlight->start_intensity + ( ( dlight->end_intensity - dlight->start_intensity ) *
						( ( level.time - dlight->start_fade_time ) / dlight->fade_time ) );
			}
		}
		else if ( dlight->lightstyle.length() )
		{
			dlight->current_lightstyle_position++;

			if ( dlight->current_lightstyle_position >= dlight->lightstyle.length() )
			{
				if ( dlight->stop_at_end )
				{
					dlight->lightstyle = "";
					dlight->current_lightstyle_position = -1;
				}
				else
				{
					dlight->current_lightstyle_position = 0;
				}
			}

			if ( dlight->current_lightstyle_position >= 0 )
				dlight->intensity = ( dlight->lightstyle[ dlight->current_lightstyle_position ] - 'a' ) / 25.0f;
		}
	}

	UpdateDynamicLights();

	// Do wind stuff

	if ( wind.gust_time )
	{
		if ( level.time < ( wind.start_fade_time + wind.fade_time ) )
		{
			wind.intensity = wind.start_intensity + ( ( wind.end_intensity - wind.start_intensity ) *
					( ( level.time - wind.start_fade_time ) / wind.fade_time ) );
		}
		else if ( level.time < ( wind.start_fade_time + wind.fade_time + wind.gust_time ) )
		{
			wind.intensity = wind.end_intensity;
		}
		else
		{
			wind.intensity = wind.end_intensity + ( ( wind.start_intensity - wind.end_intensity ) *
					( ( level.time - wind.start_fade_time - wind.fade_time - wind.gust_time ) / wind.fade_time ) );

			if ( level.time > ( wind.start_fade_time + wind.fade_time + wind.gust_time + wind.fade_time ) )
			{
				wind.intensity = wind.start_intensity;
				wind.fade_time = 0;
				wind.gust_time = 0;
			}
		}
	}
	else if ( wind.fade_time )
	{
		if ( level.time > ( wind.start_fade_time + wind.fade_time ) )
		{
			wind.intensity = wind.end_intensity;
			wind.fade_time = 0;
		}
		else
		{
			wind.intensity = wind.start_intensity + ( ( wind.end_intensity - wind.start_intensity ) *
					( ( level.time - wind.start_fade_time ) / wind.fade_time ) );
		}
	}
	else if ( ( wind.min_intensity > 0.0f ) && ( wind.max_intensity > 0.0f ) )
	{
		wind.intensity += G_CRandom( wind.max_change );

		if ( wind.intensity > wind.max_intensity )
			wind.intensity = wind.max_intensity;

		if ( wind.intensity < wind.min_intensity )
			wind.intensity = wind.min_intensity;
	}

	UpdateWindDirection();
	UpdateWindIntensity();
	UpdateTimeScale();

	// Do gravity stuff

	float gravity;

	// Try to use the world's specified gravity

	gravity = getPhysicsVar( WORLD_PHYSICS_GRAVITY );

	// If not set, use the normal cvar for the gravity

	if ( gravity == -1 )
	{
		gravity = sv_gravity->value;
	}

	// If gravity has changed change the cvar

	if ( sv_currentGravity->value != gravity )
	{
		gi.cvar_set( "sv_currentGravity", va( "%f", gravity ) );
	}
}

//----------------------------------------------------------------
// Name:			findBrokenThing
// Class:			World
//
// Description:		Tries to find the specified broken thing in the list
//
// Parameters:		const char *name			- name of the potential broken thing
//
// Returns:			int							- index of the broken thing if found, 0 if not found
//----------------------------------------------------------------

int World::findBrokenThing( const char *name )
{
	int i;
	str *brokenThingName;

	for( i = 1 ; i <= _brokenThings.NumObjects() ; i++ )
	{
		brokenThingName =  &_brokenThings.ObjectAt( i );

		if ( stricmp( brokenThingName->c_str(), name ) == 0 )
			return i;
	}

	return 0;
}

//----------------------------------------------------------------
// Name:			addBrokenThing
// Class:			World
//
// Description:		Adds a broken thing to the list
//
// Parameters:		Event *ev					- contains the name of the broken thing
//
// Returns:			none
//----------------------------------------------------------------

void World::addBrokenThing( Event *ev )
{
	addBrokenThing( ev->GetString( 1 ) );
}

//----------------------------------------------------------------
// Name:			removeBrokenThing
// Class:			World
//
// Description:		Removes a broken thing from the list
//
// Parameters:		Event *ev					- contains the name of the broken thing
//
// Returns:			none
//----------------------------------------------------------------

void World::removeBrokenThing( Event *ev )
{
	removeBrokenThing( ev->GetString( 1 ) );
}

//----------------------------------------------------------------
// Name:			addBrokenThing
// Class:			World
//
// Description:		Adds a broken thing to the list
//
// Parameters:		const char *name			- name of the broken thing
//
// Returns:			none
//----------------------------------------------------------------

void World::addBrokenThing( const char *name )
{
	if ( !findBrokenThing( name ) )
	{
		_brokenThings.AddObject( name );
	}
}

//----------------------------------------------------------------
// Name:			removeBrokenThing
// Class:			World
//
// Description:		Removes a broken thing from the list
//
// Parameters:		const char *name			- name of the broken thing
//
// Returns:			none
//----------------------------------------------------------------

void World::removeBrokenThing( const char *name )
{
	int index;

	index = findBrokenThing( name );

	if ( index )
	{
		_brokenThings.RemoveObjectAt( index );
	}
}

//----------------------------------------------------------------
// Name:			freeAllBrokenThings
// Class:			World
//
// Description:		Frees the broken thing list
//
// Parameters:		none
//
// Returns:			none
//----------------------------------------------------------------

void World::freeAllBrokenThings( void )
{
	_brokenThings.FreeObjectList();
}

//----------------------------------------------------------------
// Name:			addAvailableViewMode
// Class:			World
//
// Description:		Adds a mode to the list of view modes that are available
//
// Parameters:		Event *ev					- contains name of the mode
//
// Returns:			none
//----------------------------------------------------------------

void World::addAvailableViewMode( Event *ev )
{
	_availableViewModes.AddUniqueObject( ev->GetString( 1 ) );
}

//----------------------------------------------------------------
// Name:			removeAvailableViewMode
// Class:			World
//
// Description:		Removes the named mode from the list of view modes that are available
//
// Parameters:		Event *ev					- contains name of the mode
//
// Returns:			none
//----------------------------------------------------------------

void World::removeAvailableViewMode( Event *ev )
{
	if ( _availableViewModes.ObjectInList( ev->GetString( 1 ) ) )
	{
		_availableViewModes.RemoveObject( ev->GetString( 1 ) );
	}
}

//----------------------------------------------------------------
// Name:			clearAvailableViewModes
// Class:			World
//
// Description:		Clears all of the available view modes
//
// Parameters:		Event *ev					- not used
//
// Returns:			none
//----------------------------------------------------------------

void World::clearAvailableViewModes( Event *ev )
{
	_availableViewModes.ClearObjectList();
}

//----------------------------------------------------------------
// Name:			isAnyViewModeAvailable
// Class:			World
//
// Description:		Checks to see if any view modes are available
//
// Parameters:		none
//
// Returns:			bool				- whether or not any view modes are available
//----------------------------------------------------------------

bool World::isAnyViewModeAvailable( void )
{
	if ( _availableViewModes.NumObjects() )
		return true;
	else
		return false;
}

//----------------------------------------------------------------
// Name:			isViewModeAvailable
// Class:			World
//
// Description:		Checks to see if the specified view mode is available
//
// Parameters:		none
//
// Returns:			bool				- whether or not the specified view mode is available
//----------------------------------------------------------------

bool World::isViewModeAvailable( const char *name )
{
	if ( _availableViewModes.IndexOfObject( name ) )
		return true;
	else
		return false;
}

int World::worldPhysicsVarNameToIndex( const char *varName )
	{
	int i;

	for ( i = 0 ; i < WORLD_PHYSICS_TOTAL_NUMBER ; i++ )
		{
		if ( stricmp( varName, worldPhysicsVarNames[ i ] ) == 0 )
			return i;
		}

	gi.DPrintf( "Unknown physics variable %s\n", varName );
	return -1;
	}

void World::setPhysicsVar( Event *ev )
{
	setPhysicsVar( ev->GetString( 1 ), ev->GetFloat( 2 ) );
}

void World::setPhysicsVar( const char *varName, float varValue )
{
	int index;

	index = worldPhysicsVarNameToIndex( varName );

	if ( index != -1 )
	{
		_physicsInfo[ index ] = varValue;
	}
}

float World::getPhysicsVar( int index )
{
	return _physicsInfo[ index ];
}

float World::getPhysicsVar( const char *varName )
{
	int index;

	index = worldPhysicsVarNameToIndex( varName );

	if ( index != -1 )
	{
		return getPhysicsVar( index );
	}
	else
	{
		return -1.0f;
	}
}


void World::Archive( Archiver &arc )
{
	int i;
	int num;
	TargetList *tempTargetList;

	Entity::Archive( arc );

	if ( arc.Loading() )
	{
		FreeTargetList();
	}

	if ( arc.Saving() )
		num = targetList.NumObjects();

	arc.ArchiveInteger( &num );

	for ( i = 1; i <= num; i++ ) 
	{
		if ( arc.Saving() )
		{
			tempTargetList = targetList.ObjectAt( i );
		}
		else 
		{
			tempTargetList = new TargetList;
			targetList.AddObject( tempTargetList );
		}

		arc.ArchiveObject( ( Class * )tempTargetList );
	}

	_brokenThings.Archive( arc );

	_availableViewModes.Archive( arc );

	arc.ArchiveBoolean( &world_dying );

	arc.ArchiveString( &skipthread );
	arc.ArchiveFloat( &farplane_distance );
	arc.ArchiveVector( &farplane_color );
	arc.ArchiveBoolean( &farplane_cull );
	arc.ArchiveBoolean( &farplane_fog );
	arc.ArchiveBoolean( &terrain_global );
	arc.ArchiveFloat( &terrain_global_min );
	arc.ArchiveFloat( &entity_fade_dist );

	for( i = 0 ; i < MAX_LIGHTING_GROUPS ; i++ )
		dynamic_lights[ i ].Archive( arc );

	wind.Archive( arc );
	weather.Archive( arc );

	arc.ArchiveFloat( &time_scale );

	arc.ArchiveFloat( &sky_alpha );
	arc.ArchiveBoolean( &sky_portal );

	for ( i = 0 ; i < WORLD_PHYSICS_TOTAL_NUMBER ; i++ )
	{
		arc.ArchiveFloat( &_physicsInfo[ i ] );
	}

	arc.ArchiveBool( &_canShakeCamera );

	if ( arc.Loading() )
	{
		UpdateConfigStrings();
		UpdateFog();
		UpdateTerrain();
		UpdateSky();
		UpdateDynamicLights();
		UpdateWindDirection();
		UpdateWindIntensity();
		UpdateWeather();
		UpdateTimeScale();
	}

	// Archive groupcoordinator (not part of world but this is a good place)

	if ( arc.Loading() )
	{
		if ( groupcoordinator )
			delete groupcoordinator;

		groupcoordinator = new GroupCoordinator;
	}

	groupcoordinator->Archive( arc );
}

void World::setCanShakeCamera( Event *ev )
{
	_canShakeCamera = ev->GetBoolean( 1 );
}

bool World::canShakeCamera( void )
{
	return _canShakeCamera;
}

//----------------------------------------------------------------
// Name:			isThingBroken
// Class:			World
//
// Description:		Checks to see if the specified thing is broken
//
// Parameters:		const char *name			- potential broken thing
//
// Returns:			none
//----------------------------------------------------------------

bool World::isThingBroken( const char *name )
{
	if ( findBrokenThing( name ) )
		return true;
	else
		return false;
}

DynamicLightInfo::DynamicLightInfo()
{
	defaultIntensity = 0.5f;

	intensity = 1.0f;
	fade_time = 0.0f;

	start_intensity = 0.0f;
	end_intensity   = 0.0f;

	start_fade_time = 0.0f;

	current_lightstyle_position = 0;
	stop_at_end = false;
}

WindInfo::WindInfo()
{
	intensity       = 0.0f;
	fade_time       = 0.0f;
	gust_time       = 0.0f;
	start_fade_time = 0.0f;
	max_change      = 0.0f;

	min_intensity = -1.0f;
	max_intensity = -1.0f;

	start_intensity = 0.0f;
	end_intensity   = 0.0f;
	direction = Vector( 1, 1, 0 );
}

WeatherInfo::WeatherInfo()
{
	type = WEATHER_NONE;
	intensity = 0;
}

//
// List stuff for targets
//

CLASS_DECLARATION( Class, TargetList, NULL )
{
	{ NULL, NULL }
};

TargetList::TargetList()
{
}

TargetList::TargetList( const str &tname )
{
	targetname = tname;
}

TargetList::~TargetList()
{
}

void TargetList::AddEntity( Entity * ent )
{
	if ( !list.ObjectInList( ent ) )
	{
		list.AddObject( ent );
	}
}

void TargetList::RemoveEntity( Entity * ent )
{
	if ( list.ObjectInList( ent ) )
	{
		list.RemoveObject( ent );
	}
}

Entity * TargetList::GetNextEntity( Entity * ent )
{
	int index;

	index = 0;
	if ( ent )
		index = list.IndexOfObject( ent );
	index++;
	if ( index > list.NumObjects() )
		return NULL;
	else
		return list.ObjectAt( index );
}
