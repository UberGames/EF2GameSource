//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/g_spawn.cpp                                   $
// $Revision:: 8                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:43a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#include "_pch_cpp.h"
#include "class.h"
#include "entity.h"
#include "g_spawn.h"
#include "navigate.h"
#include "player.h"
#include "gravpath.h"
#include "object.h"

CLASS_DECLARATION( Class, SpawnArgs, NULL )
{
	{ NULL, NULL }
};

SpawnArgs::SpawnArgs()
{
}

SpawnArgs::SpawnArgs( SpawnArgs &otherlist )
{
	int num;
	int i;
	
	num = otherlist.NumArgs();
	keyList.Resize( num );
	valueList.Resize( num );
	for( i = 1; i <= num; i++ )
	{
		keyList.AddObject( otherlist.keyList.ObjectAt( i ) );
		valueList.AddObject( otherlist.valueList.ObjectAt( i ) );
	}
}

void SpawnArgs::Clear( void )
{
	keyList.FreeObjectList();
	valueList.FreeObjectList();
}

/*
====================
Parse

Parses spawnflags out of the given string, returning the new position.
Clears out any previous args.
====================
*/
const char *SpawnArgs::Parse( const char *data )
{
	str         keyname;
	const char	*com_token;
	
	Clear();
	
	// parse the opening brace
	com_token = COM_Parse( &data );
	if ( !data )
	{
		return NULL;
	}
	
	if ( com_token[ 0 ] != '{' )
	{
		gi.Error( ERR_DROP, "SpawnArgs::Parse : found %s when expecting {", com_token );
	}
	
	// go through all the dictionary pairs
	while( 1 )
	{
		// parse key
		com_token = COM_Parse( &data );
		if ( com_token[ 0 ] == '}' )
		{
			break;
		}
		
		if ( !data )
		{
			gi.Error( ERR_DROP, "SpawnArgs::Parse : EOF without closing brace" );
		}
		
		keyname = com_token;
		
		// parse value
		com_token = COM_Parse( &data );
		if ( !data )
		{
			gi.Error( ERR_DROP, "SpawnArgs::Parse : EOF without closing brace" );
		}
		
		if ( com_token[ 0 ] == '}' )
		{
			gi.Error( ERR_DROP, "SpawnArgs::Parse : closing brace without data" );
		}
		
		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by the game
		if ( keyname[ 0 ] == '_' )
		{
			continue;
		}
		
		setArg( keyname.c_str(), com_token );
	}
	
	return data;
}

const char *SpawnArgs::getArg( const char *key, const char *defaultValue )
{
	int i;
	int num;
	
	num = keyList.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		if ( keyList.ObjectAt( i ) == key )
		{
			return valueList.ObjectAt( i );
		}
	}
	
	return defaultValue;
}

void SpawnArgs::setArg( const char *key, const char *value )
{
	int i;
	int num;
	
	num = keyList.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		if ( keyList.ObjectAt( i ) == key )
		{
			valueList.ObjectAt( i ) = value;
			return;
		}
	}
	
	keyList.AddObject( str( key ) );
	valueList.AddObject( str( value ) );
}

void SpawnArgs::operator=( SpawnArgs &otherlist )
{
	int num;
	int i;
	
	Clear();
	
	num = otherlist.NumArgs();
	keyList.Resize( num );
	valueList.Resize( num );
	
	for( i = 1; i <= num; i++ )
	{
		keyList.AddObject( otherlist.keyList.ObjectAt( i ) );
		valueList.AddObject( otherlist.valueList.ObjectAt( i ) );
	}
}

int SpawnArgs::NumArgs( void )
{
	return keyList.NumObjects();
}

const char *SpawnArgs::getKey( int index )
{
	return keyList.ObjectAt( index + 1 );
}

const char *SpawnArgs::getValue( int index )
{
	return valueList.ObjectAt( index + 1 );
}

void SpawnArgs::Archive( Archiver &arc )
{
	Class::Archive( arc );
	
	keyList.Archive( arc );
	valueList.Archive( arc );
}

/*
===============
getClass

Finds the spawn function for the entity and returns ClassDef *
===============
*/

ClassDef *SpawnArgs::getClassDef( qboolean *tikiWasStatic )
{
	const char	*classname;
	ClassDef		*cls = NULL;
	
	classname = getArg( "classname" );
	
	if ( tikiWasStatic )
	{
		*tikiWasStatic = false;
	}
	
	//
	// check normal spawn functions
	// see if the class name is stored within the model
	//
	if ( classname )
	{
		//
		// explicitly inhibit lights
		//
		if ( !strcmpi( classname, "light" ) )
		{
			//
			// HACK HACK HACK
			// hack to suppress a warning message
			//
			if ( tikiWasStatic )
            {
				*tikiWasStatic = true;
            }
			return NULL;
		}
		
		cls = getClassForID( classname );
		if ( !cls )
		{
			cls = getClass( classname );
		}
	}
	
	if ( !cls )
	{
		const char *model;
		
		//
		// get Object in case we cannot find an alternative
		//
		cls = &Object::ClassInfo;
		model = getArg( "model" );
		if ( model )
		{
			tiki_cmd_t cmds;
			int modelindex;
			int i;
			
			//
			// get handle to def file
			//
			if ( ( strlen( model ) >= 3 ) && ( !strcmpi( &model[ strlen(model) - 3 ], "tik" ) ) )
            {
				modelindex = modelIndex( model );
				
				if ( modelindex == -1 )
					return NULL;
				
				if ( gi.IsModel( modelindex ) )
				{
					const char * s;
					
					s = getArg( "make_static" );
					if ( s && atoi( s ) )
					{
						//
						// if make_static then we don't want to spawn
						//
						if ( tikiWasStatic )
						{
							*tikiWasStatic = true;
						}
						
						return NULL;
					}
					
					if ( gi.InitCommands( modelindex, &cmds ) )
					{
						for( i = 0; i < cmds.num_cmds; i++ )
						{
							if ( !strcmpi( cmds.cmds[ i ].args[ 0 ], "classname" ) )
							{
								cls = getClass( cmds.cmds[ i ].args[ 1 ] );
								break;
							}
						}
						if ( i == cmds.num_cmds )
						{
							if ( developer->integer == 2 )
								gi.WDPrintf( "Classname %s used, but 'classname' was not found in Initialization commands, using Object.\n", classname );
						}
					}
					else
						gi.WDPrintf( "Classname %s used, but TIKI had no Initialization commands, using Object.\n", classname );
				}
				else
					gi.WDPrintf( "Classname %s used, but TIKI was not valid, using Object.\n", classname );
            }
			else
				gi.WDPrintf( "Classname %s used, but model was not a TIKI, using Object.\n", classname );
		}
		else
		{
			gi.WDPrintf( "Classname %s' used, but no model was set, using Object.\n", classname );
		}
	}
	
	return cls;
}

/*
===============
Spawn

Finds the spawn function for the entity and calls it.
Returns pointer to Entity
===============
*/

Entity *SpawnArgs::Spawn( void )
{
	str      classname;
	ClassDef	*cls;
	Entity	*obj;
	Event    *ev;
	int      i;
	qboolean tikiWasStatic; // used to determine if entity was intentionally suppressed
	
	classname = getArg( "classname", "Unspecified" );
	cls = getClassDef( &tikiWasStatic );
	if ( !cls )
	{
		if ( !tikiWasStatic )
		{
			gi.DPrintf( "%s doesn't have a spawn function\n", classname.c_str() );
		}
		
		return NULL;
	}
	
	obj = ( Entity * )cls->newInstance();
	
	// post spawnarg events
	for( i = 0; i < NumArgs(); i++ )
	{
		// if it is the "script" key, execute the script commands individually
		if ( !Q_stricmp( getKey( i ), "script" ) )
		{
			char *ptr;
			char * token;
			
			ptr = const_cast< char * >( getValue( i ) );
			while ( 1 )
            {
				token = COM_ParseExt( &ptr, true );
				if ( !token[ 0 ] )
					break;
				if ( strchr( token, ':' ) )
				{
					gi.WDPrintf( "Label %s imbedded inside editor script for %s.\n", token, classname.c_str() );
				}
				else
				{
					ev = new Event( token );
					while ( 1 )
					{
						token = COM_ParseExt( &ptr, false );
						if ( !token[ 0 ] )
							break;
						ev->AddToken( token );
					}
					
					obj->PostEvent( ev, EV_SPAWNARG );
				}
            }
		}
		else
		{
			ev = new Event( getKey( i ) );
			ev->AddToken( getValue( i ) );
			
			if ( Q_stricmp( getKey( i ), "model" ) == 0 )
				obj->PostEvent( ev, EV_PRIORITY_SPAWNARG );
			else
				obj->PostEvent( ev, EV_SPAWNARG );
		}
	}
	
	if ( !obj )
	{
		gi.WDPrintf( "%s failed on newInstance\n", classname.c_str() );
		return NULL;
	}
	
	return obj;
}

/*
==============
G_InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void G_InitClientPersistant( gclient_t *client )
{
	memset( &client->pers, 0, sizeof( client->pers ) );
}

ClassDef *FindClass( const char *name, qboolean *isModel )
{
	ClassDef *cls;
	
	*isModel = false;
	
	// first lets see if it is a registered class name
	cls = getClass( name );
	if ( !cls )
	{
		SpawnArgs args;
		
		// if that didn't work lets try to resolve it as a model
		args.setArg( "model", name );
		
		cls = args.getClassDef();
		if ( cls )
		{
			*isModel = true;
		}
	}
	return cls;
}
