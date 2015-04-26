//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/listener.cpp                              $
// $Revision:: 29                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 4:00p                                                  $
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
// WARNING: This file is shared between game, cgame and possibly the user interface.
// It is instanced in each one of these directories because of the way that SourceSafe works.
//

#include "listener.h"
#include <qcommon/output.h>

#if defined( GAME_DLL )

#include "worldspawn.h"
#include "scriptmaster.h"
#include "mp_manager.hpp"

#elif defined( CGAME_DLL )

#elif defined( UI_LIB )

#else

//#include "client.h"

#endif


Event EV_Remove
(
	"immediateremove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener immediately."
);
Event EV_ScriptRemove
(
	"remove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener the next time events are processed."
);

extern "C"
{
	int				numEvents = 0;
	int				numFreeEvents = 0;
}

cvar_t *g_showevents;
cvar_t *g_eventlimit;
cvar_t *g_timeevents;
cvar_t *g_watch;
cvar_t *g_eventstats;

Event FreeEventHead;
Event *FreeEvents = &FreeEventHead;
Event EventQueueHead;
Event *EventQueue = &EventQueueHead;
Event ActiveEventHead;
Event *ActiveEvents = &ActiveEventHead;

Container<str *>  *Event::commandList = NULL;
Container<int>    *Event::flagList = NULL;
Container<int>    *Event::sortedList = NULL;
Container<Event *> *Event::eventDefList = NULL;
qboolean          Event::dirtylist = false;

int Event::numfinds;
int Event::numfirstsearch;
int Event::numcompares;
int Event::lastevent;
bool Event::EventSystemStarted;

int Event_totalmemallocated;

Event NullEvent;

void EV_Print( FILE *event_file, const char *fmt, ... )
{
	va_list	argptr;
	char		text[ 1024 ];
	
	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );
	
	if ( event_file )
		fprintf( event_file, text );
	else
		EVENT_Printf( text );
}

EventVar::EventVar( const char *text )
{
	assert( text );
	if ( !text )
	{
		text = "";
	}
	
	dirtyFlags  = DIRTY_ALL & ~DIRTY_STRING;
	stringValue = text;
	type        = IS_STRING;
	intValue    = 0;
	floatValue  = 0;
}

EventVar::EventVar( const str &text )
{
	dirtyFlags  = DIRTY_ALL & ~DIRTY_STRING;
	stringValue = text;
	type        = IS_STRING;
	intValue    = 0;
	floatValue  = 0;
}

#ifdef GAME_DLL

EventVar::EventVar( const Entity *ent )
{
	type        = IS_ENTITY;
	dirtyFlags  = DIRTY_ALL & ~DIRTY_INTEGER;
	if ( ent )
	{
		intValue = ent->entnum;
	}
	else
	{
		intValue = ENTITYNUM_NONE;
	}
	floatValue  = 0;
}

void EventVar::Archive( Archiver &arc )
{
	arc.ArchiveShort( &type );
	arc.ArchiveShort( &dirtyFlags );
	arc.ArchiveString( &stringValue );
	arc.ArchiveVector( &vectorValue );
	arc.ArchiveInteger( &intValue );
	arc.ArchiveFloat( &floatValue );
}

#endif

const char *EventVar::GetToken( Event & )
{
	if ( dirtyFlags & DIRTY_STRING )
	{
		switch( type )
		{
		case IS_VECTOR :
            stringValue = va( "(%f %f %f)", vectorValue.x, vectorValue.y, vectorValue.z );
            break;
			
		case IS_INTEGER :
            stringValue = va( "%d", intValue );
            break;
			
		case IS_FLOAT :
            stringValue = va( "%f", floatValue );
            break;
			
		case IS_ENTITY :
            stringValue = va( "*%d", intValue );
            break;
		}
		
		dirtyFlags &= ~DIRTY_STRING;
	}
	
	return stringValue.c_str();
}

const char *EventVar::GetString( Event & )
{
	if ( dirtyFlags & DIRTY_STRING )
	{
		switch( type )
		{
		case IS_VECTOR :
            stringValue = va( "(%f %f %f)", vectorValue.x, vectorValue.y, vectorValue.z );
            break;
			
		case IS_INTEGER :
            stringValue = va( "%d", intValue );
            break;
			
		case IS_FLOAT :
            stringValue = va( "%f", floatValue );
            break;
			
		case IS_ENTITY :
            stringValue = va( "*%d", intValue );
            break;
		}
		
		dirtyFlags &= ~DIRTY_STRING;
	}
	
	return stringValue.c_str();
}

int EventVar::GetInteger( Event & )
{
	if ( dirtyFlags & DIRTY_INTEGER )
	{
		switch( type )
		{
		case IS_STRING :
            intValue = atoi( stringValue.c_str() );
            break;
			
		case IS_VECTOR :
            intValue = 0;
            break;
			
		case IS_FLOAT :
            intValue = int( floatValue );
            break;
		}
		
		dirtyFlags &= ~DIRTY_INTEGER;
	}
	
	return intValue;
}

float EventVar::GetFloat( Event & )
{
	if ( dirtyFlags & DIRTY_FLOAT )
	{
		switch( type )
		{
		case IS_STRING :
            floatValue = (float)atof( stringValue.c_str() );
            break;
			
		case IS_VECTOR :
            floatValue = 0;
            break;
			
		case IS_ENTITY :
		case IS_INTEGER :
            floatValue = float( intValue );
            break;
		}
		
		dirtyFlags &= ~DIRTY_FLOAT;
	}
	
	return floatValue;
}

Vector EventVar::GetVector( Event & )
{
	if ( dirtyFlags & DIRTY_VECTOR )
	{
		switch( type )
		{
		case IS_STRING :
            const char *text;
			
            text = stringValue.c_str();
			if ( text[ 0 ] == '(' )
			{
				vectorValue = Vector(&text[ 1 ]);
			}
#ifdef GAME_DLL
            // is it an entity
			else if ( text[ 0 ] == '$' )
			{
				Entity * ent;
				
				// allow console users to not use '$'
				ent = G_FindTarget( NULL, &text[ 1 ] );
				
				if ( ent )
				{
					vectorValue = ent->origin;
				}
				else
				{
					vectorValue = vec_zero;
				}
			}
#endif
            else
			{
				vectorValue = Vector(text);
			}
            break;
			
		case IS_FLOAT :
		case IS_INTEGER :
            vectorValue = vec_zero;
            break;
		case IS_ENTITY :
#ifdef GAME_DLL
            {
				Entity * ent;
				
				ent = G_GetEntity( intValue );
				if ( ent )
				{
					vectorValue = ent->origin;
				}
				else
				{
					vectorValue = vec_zero;
				}
            }
#else
            vectorValue = vec_zero;
#endif
            break;
		}
		
		dirtyFlags &= ~DIRTY_VECTOR;
	}
	
	return vectorValue;
}

#ifdef GAME_DLL
Entity *EventVar::GetEntity( Event &ev )
{
	if ( dirtyFlags & DIRTY_INTEGER )
	{
		switch( type )
		{
		case IS_VECTOR :
            intValue = 0;
            break;
			
		case IS_FLOAT :
            intValue = int( floatValue );
            break;
			
		case IS_STRING :
			const char *name;
            int        t;
			
            t = 0;
            
			name = stringValue.c_str();
			if ( ev.GetSource() == EV_FROM_CONSOLE )
			{
				Entity * ent;
				
				// allow console users to not use '$'
				ent = G_FindTarget( NULL, name );
				intValue = ent->entnum;
				break;
			}
			
            if ( name[ 0 ] == '$' )
			{
				Entity * ent;
				
				ent = G_FindTarget( NULL, &name[ 1 ] );
				if ( !ent )
				{
					ev.Error( "Entity with targetname of '%s' not found", &name[ 1 ] );
					
					return NULL;
				}
				else
				{
					t = ent->entnum;
				}
			}
			else
			{
				if ( name[ 0 ] != '*' )
				{
					if ( ev.GetSource() == EV_FROM_CONSOLE )
					{
						ev.Error( "Entity with targetname of '%s' not found", name );
					}
					else
					{
						ev.Error( "Expecting a '*'-prefixed entity number but found '%s'.", name );
					}
					
					return NULL;
				}
				
				if ( !::IsNumeric( &name[ 1 ] ) )
				{
					ev.Error( "Expecting a numeric value but found '%s'.", &name[ 1 ] );
					
					return NULL;
				}
				else
				{
					t = atoi( &name[ 1 ] );
				}
			}
			
            intValue = t;
			
            break;
		}
		
		dirtyFlags &= ~DIRTY_INTEGER;
	}
	
	if ( ( intValue < 0 ) || ( intValue > game.maxentities ) )
	{
		ev.Error( "%d out of valid range for entity.", intValue );
		return NULL;
	}
	
	return G_GetEntity( intValue );
}

//===============================================================
// Name:		IsEntity -- Game DLL only
// Class:		EventVar
//
// Description: Determines if this event parameter describes an
//				entity.
// 
// Parameters:	Event& -- the event
//
// Returns:		qboolean -- qtrue if it is an entity.
// 
//===============================================================
qboolean EventVar::IsEntity( Event &ev )
{
	Entity *ent = 0 ;
	const char *name;

	if ( dirtyFlags & DIRTY_INTEGER )
	{
		switch( type )
		{
		case IS_VECTOR:						
			break ;

		case IS_FLOAT:
			break ;

		case IS_INTEGER:
			if ( ( intValue < 0 ) || ( intValue > game.maxentities ) )
			{
				break ;
			}
			ent = G_GetEntity( intValue );	
			break ;

        case IS_STRING :
         	name = stringValue.c_str();
			if ( ev.GetSource() == EV_FROM_CONSOLE )
			{
				ent = G_FindTarget( NULL, name );
			}
            else if ( name[ 0 ] == '$' )
			{
				ent = G_FindTarget( NULL, &name[ 1 ] );
			}
			else if (   (name[0]=='*')   &&   ::IsNumeric(&name[1])   )
			{
				ent = G_GetEntity( atoi(&name[1]) );
			}
            break;
		case IS_ENTITY :
			if ( ( intValue < 0 ) || ( intValue > game.maxentities ) )
			{
				break ;
			}
			ent = G_GetEntity( intValue );	
			break ;
		}
	}
	else
	{
		if ( ( intValue >= 0 ) && ( intValue < game.maxentities ) )
		{
			ent = G_GetEntity( intValue );	
		}
	}

	if ( ent )
	{
		intValue = ent->entnum;
		dirtyFlags &= ~DIRTY_INTEGER;
		return true;
	}
	else
	{
		return false;
	}
}

#else

qboolean IsNumeric( const char *str )
{
	int len;
	int i;
	qboolean dot;
	
	if ( *str == '-' )
	{
		str++;
	}
	
	dot = false;
	len = strlen( str );
	for( i = 0; i < len; i++ )
	{
		if ( !isdigit( str[ i ] ) )
		{
			if ( ( str[ i ] == '.' ) && !dot )
			{
				dot = true;
				continue;
			}
			return false;
		}
	}
	
	return true;
}

#endif

qboolean EventVar::IsVector( Event & )
{
	switch( type )
	{
	case IS_VECTOR :
		return true;
		break;
		
	case IS_STRING :
		if ( stringValue.c_str()[ 0 ] == '(' )
		      {
			return true;
		      }
		break;
	}
	
	return false;
}

qboolean EventVar::IsNumeric( Event & )
{
	switch( type )
	{
	case IS_STRING :
		return ::IsNumeric( stringValue.c_str() );
		break;
	case IS_FLOAT :
	case IS_INTEGER :
		return true;
		break;
	}
	
	return false;
}

//===========================================================================
// EventArgDef
//===========================================================================

void EventArgDef::Setup( const char *eventName, const char *argName, const char *argType, const char *argRange )
{
	char        scratch[ 256 ];
	const char  *ptr;
	char        *tokptr;
	const char  *endptr;
	int         index;
	
	// set name
	name = argName;
	
	// set optionality
	if ( isupper( argType[ 0 ] ) )
	{
		optional = true;
	}
	else
	{
		optional = false;
	}
	
	// grab the ranges
	index = 0;
	memset( minRangeDefault, true, sizeof( minRangeDefault ) );
	memset( minRange, 0, sizeof( minRange ) );
	memset( maxRangeDefault, true, sizeof( maxRangeDefault ) );
	memset( maxRange, 0, sizeof( maxRange ) );
	
	if ( argRange && argRange[ 0 ] )
	{
		ptr = argRange;
		while( 1 )
		{
			// find opening '['
			tokptr = const_cast<char*>(strchr( ptr, '[' ));
			if ( !tokptr )
            {
				break;
            }
			// find closing ']'
			endptr = strchr( tokptr, ']' );
			if ( !endptr )
            {
				assert( 0 );
				EVENT_WDPrintf( "Argument defintion %s, no matching ']' found for range spec in event %s.\n", name.c_str(), eventName );
				break;
            }
			// point to the next range
			ptr = endptr;
			// skip the '['
			tokptr++;
			// copy off the range spec
			// skip the ']'
			strncpy( scratch, tokptr, endptr - tokptr );
			// terminate the range
			scratch[ endptr - tokptr ] = 0;
			// see if there is one or two parameters here
			tokptr = strchr( scratch, ',' );
			if ( !tokptr )
            {
				// just one parameter
				minRange[ index >> 1 ] = (float)atof( scratch );
				minRangeDefault[ index >> 1 ] = false;
				index++;
				// skip the second parameter
				index++;
            }
			else if ( tokptr == scratch )
            {
				// just second parameter
				// skip the first paremeter
				index++;
				tokptr++;
				maxRange[ index >> 1 ] = (float)atof( scratch );
				maxRangeDefault[ index >> 1 ] = false;
				index++;
            }
			else
            {
				qboolean second;
				// one or two parameters
				// see if there is anything behind the ','
				if ( strlen( scratch ) > static_cast<size_t>(tokptr - scratch + 1) )
					second = true;
				else
					second = false;
				// zero out the ','
				*tokptr = 0;
				minRange[ index >> 1 ] = (float)atof( scratch );
				minRangeDefault[ index >> 1 ] = false;
				index++;
				// skip over the nul character
				tokptr++;
				if ( second )
				{
					maxRange[ index >> 1 ] = (float)atof( tokptr );
					maxRangeDefault[ index >> 1 ] = false;
				}
				index++;
            }
		}
	}
	
	// figure out the type of variable it is
	switch( tolower( argType[ 0 ] ) )
	{
	case 'e':
		type = IS_ENTITY;
		break;
	case 'v':
		type = IS_VECTOR;
		break;
	case 'i':
		type = IS_INTEGER;
		break;
	case 'f':
		type = IS_FLOAT;
		break;
	case 's':
		type = IS_STRING;
		break;
	case 'b':
		type = IS_BOOLEAN;
		break;
	}
}

void EventArgDef::PrintRange( FILE *event_file )
{
	qboolean integer;
	qboolean single;
	int numRanges;
	int i;
	
	single = false;
	integer = true;
	numRanges = 1;
	switch( type )
	{
	case IS_VECTOR:
		integer = false;
		numRanges = 3;
		break;
	case IS_FLOAT:
		integer = false;
		break;
	case IS_STRING:
		single = true;
		break;
	}
	for( i = 0; i < numRanges; i++ )
	{
		if ( single )
		{
			if ( !minRangeDefault[ i ] )
            {
				if ( integer )
				{
					EV_Print( event_file, "<%d>", ( int )minRange[ i ] );
				}
				else
				{
					EV_Print( event_file, "<%.2f>", minRange[ i ] );
				}
            }
		}
		else
		{
			// both non-default
			if ( !minRangeDefault[ i ] && !maxRangeDefault[ i ] )
            {
				if ( integer )
				{
					EV_Print( event_file, "<%d...%d>", ( int )minRange[ i ], ( int )maxRange[ i ] );
				}
				else
				{
					EV_Print( event_file, "<%.2f...%.2f>", minRange[ i ], maxRange[ i ] );
				}
            }
			// max default
			else if ( !minRangeDefault[ i ] && maxRangeDefault[ i ] )
            {
				if ( integer )
				{
					EV_Print( event_file, "<%d...max_integer>", ( int )minRange[ i ] );
				}
				else
				{
					EV_Print( event_file, "<%.2f...max_float>", minRange[ i ] );
				}
            }
			// min default
			else if ( minRangeDefault[ i ] && !maxRangeDefault[ i ] )
            {
				if ( integer )
				{
					EV_Print( event_file, "<min_integer...%d>", ( int )maxRange[ i ] );
				}
				else
				{
					EV_Print( event_file, "<min_float...%.2f>", maxRange[ i ] );
				}
            }
		}
	}
}

void EventArgDef::PrintArgument( FILE *event_file )
{
	if ( optional )
	{
		EV_Print( event_file, "[ " );
	}
	switch( type )
	{
	case IS_ENTITY:
		EV_Print( event_file, "Entity " );
		break;
	case IS_VECTOR:
		EV_Print( event_file, "Vector " );
		break;
	case IS_INTEGER:
		EV_Print( event_file, "Integer " );
		break;
	case IS_FLOAT:
		EV_Print( event_file, "Float " );
		break;
	case IS_STRING:
		EV_Print( event_file, "String " );
		break;
	case IS_BOOLEAN:
		EV_Print( event_file, "Boolean " );
		break;
	}
	EV_Print( event_file, "%s", name.c_str() );
	
	// print the range of the argument
	PrintRange( event_file );
	
	if ( optional )
	{
		EV_Print( event_file, " ]" );
	}
}

#ifdef GAME_DLL
void EventArgDef::Archive( Archiver &arc )
{
	int i;
	
	arc.ArchiveInteger( &type );
	arc.ArchiveString( &name );
	
	for ( i = 0; i < 3; i++ )
		arc.ArchiveFloat( &minRange[i] );
	
	for ( i = 0; i < 3; i++ )
		arc.ArchiveBoolean( &minRangeDefault[i] );
	
	for ( i = 0; i < 3; i++ )
		arc.ArchiveFloat( &maxRange[i] );
	
	for ( i = 0; i < 3; i++ )
		arc.ArchiveBoolean( &maxRangeDefault[i] );
	
	arc.ArchiveBoolean( &optional );
}
#endif

//===========================================================================
// EventCode
//===========================================================================

CLASS_DECLARATION( Class, Event, NULL )
{
	{ NULL, NULL }
};

// Free Event Management routines
static Event *RealAllocateEvent( void )
{
	Event *event;
	
	event = ( Event * )::new char[ sizeof( Event ) ];
	Event_totalmemallocated += sizeof( Event );
	
	return event;
}

static void RealDeallocateEvent( Event * event )
{
	::delete[]( ( void * )event );
	Event_totalmemallocated -= sizeof( Event );
}

void * Event::operator new( size_t s )
{
	Event * newevent;
	
	assert( sizeof( Event ) == s );
	
	// if the list is empty create a new one
	if ( LL_Empty( FreeEvents, next, prev ) )
	{
		// this is here to let us know that it is happening
		assert( 0 );
		newevent = RealAllocateEvent();
	}
	else
	{
		newevent = FreeEvents->next;
		LL_Remove( newevent, next, prev );
		numFreeEvents--;
	}
	
	// add it to the active list of events
	LL_Add( ActiveEvents, newevent, next, prev );
	
	newevent->time = EVENT_time;
	newevent->flags = 0;
	
	return newevent;
}

void Event::operator delete( void *ptr )
{
	Event * event;
	
	event = ( Event * )ptr;
	
	// clear out any safe pointers we have setup
	event->ClearSafePointers();
	
	LL_Remove( event, next, prev );
	LL_Add( FreeEvents, event, next, prev );
	numFreeEvents++;
}

#if defined( GAME_DLL )
#define INITIALLY_ALLOCATED_EVENTS 6000
#elif defined ( CGAME_DLL )
#define INITIALLY_ALLOCATED_EVENTS 512
#elif defined ( UI_LIB )
#define INITIALLY_ALLOCATED_EVENTS 192
#else
#define INITIALLY_ALLOCATED_EVENTS 192
#endif

void Event::InitializeEventLists( void )
{
	Event * newevent;
	int i;
	
	numEvents = 0;
	numFreeEvents = 0;
	//
	// initialize lists
	//
	LL_Reset( FreeEvents, next, prev );
	LL_Reset( EventQueue, next, prev );
	LL_Reset( ActiveEvents, next, prev );
	
	//
	// allocate the initial allottment of events
	//
	for( i = 0; i < INITIALLY_ALLOCATED_EVENTS; i++ )
	{
		newevent = RealAllocateEvent();
		LL_Add( FreeEvents, newevent, next, prev );
		numFreeEvents++;
	}
}

void Event::ShutdownEventLists( void )
{
	Event *event, *next;
	
	// free queued events
	for( event = EventQueue->next; event != EventQueue; event = next )
	{
		next = event->next;
		delete event;
	}
	// free active events
	for( event = ActiveEvents->next; event != ActiveEvents; event = next )
	{
		next = event->next;
		delete event;
	}
	// free free events
	for( event = FreeEvents->next; event != FreeEvents; event = next )
	{
		next = event->next;
		RealDeallocateEvent( event );
	}
	
	assert( Event_totalmemallocated == 0 );
	
	numEvents = 0;
	numFreeEvents = 0;
	
	//
	// initialize lists
	//
	LL_Reset( FreeEvents, next, prev );
	LL_Reset( EventQueue, next, prev );
	LL_Reset( ActiveEvents, next, prev );
}

int Event::NumEventCommands( void )
{
	if ( commandList )
	{
		// Add 1 since container gives the inclusive number of objects
		return commandList->NumObjects() + 1;
	}
	
	return 0;
}

int Event::compareEvents( const void *arg1, const void *arg2 )
{
	int ev1;
	int ev2;
	
	ev1 = *( int * )arg1;
	ev2 = *( int * )arg2;
	
	return stricmp( commandList->ObjectAt( ev1 )->c_str(), commandList->ObjectAt( ev2 )->c_str() );
}

void Event::SortEventList( void )
{
	dirtylist = false;
	
	if ( sortedList && commandList )
	{
		qsort( ( void * )sortedList->AddressOfObjectAt( 1 ),
			( size_t )sortedList->NumObjects(),
			sizeof( int ), compareEvents );
	}
}

int Event::FindEvent( const char *name )
{
	int eventnum;
	int index;
	int l;
	int r;
	int diff;
	
	assert( name );
	if ( !name )
	{
		return 0;
	}
	
	if ( !commandList )
	{
		return 0;
	}
	
	if ( dirtylist )
	{
		SortEventList();
	}
	
	numfinds++;
	numcompares++;
	if ( lastevent && !stricmp( name, commandList->ObjectAt( lastevent )->c_str() ) )
	{
		numfirstsearch++;
		return lastevent;
	}
	
	l = 1;
	r = sortedList->NumObjects();
	while( r >= l )
	{
		index = ( l + r ) >> 1;
		eventnum = sortedList->ObjectAt( index );
		diff = stricmp( name, commandList->ObjectAt( eventnum )->c_str() );
		numcompares++;
		if ( diff < 0 )
		{
			r = index - 1;
		}
		else if ( diff > 0 )
		{
			l = index + 1;
		}
		else
		{
			lastevent = eventnum;
			return eventnum;
		}
	}
	
	return 0;
}

int Event::MapSortedEventIndex( int index )
{
	return sortedList->ObjectAt( index );
}

int Event::FindEvent( const str &name )
{
	return FindEvent( name.c_str() );
}

void Event::ListCommands( const char *mask )
{
	str name;
	int flags;
	int eventnum;
	int num;
	int i;
	int n;
	int l;
	int p;
	int hidden;
	str text;
	
	if ( !commandList )
	{
		EVENT_WDPrintf( "No events.\n" );
		return;
	}
	
	if ( dirtylist )
	{
		SortEventList();
	}
	
	l = 0;
	if ( mask )
	{
		l = strlen( mask );
	}
	
	hidden = 0;
	num = 0;
	n = sortedList->NumObjects();
	for( i = 1; i <= n; i++ )
	{
		eventnum = sortedList->ObjectAt( i );
		name = commandList->ObjectAt( eventnum )->c_str();
		flags = flagList->ObjectAt( eventnum );
		
		if ( flags & EV_CODEONLY )
		{
			hidden++;
			continue;
		}
		
		if ( mask && Q_stricmpn( name.c_str(), mask, l ) )
		{
			continue;
		}
		
		num++;
		
		text = "   ";
		p = 0;
		if ( flags & EV_CONSOLE )
		{
			text[ p++ ] = '*';
		}
		if ( flags & EV_CHEAT )
		{
			text[ p++ ] = 'C';
		}
		if ( flags & EV_CACHE )
		{
			text[ p++ ] = '%';
		}
		
		EVENT_Printf( "%4d : %s%s\n", eventnum, text.c_str(), name.c_str() );
	}
	
	EVENT_Printf( "\n* = console command.\nC = cheat command.\n% = cache command.\n\n"
		"Printed %d of %d total commands.\n", num, n - hidden );
	
	if ( developer->integer && hidden )
	{
		EVENT_Printf( "Suppressed %d commands.\n", hidden );
	}
}

#ifdef GAME_DLL

void Event::PrintEvent( Event * event )
{
	int i;
	int n;
	Listener * obj;
	str text;
	
	obj = event->GetSourceObject();
	
	text = va( "%6.1f:%1d: %s", event->time, event->flags, obj->getClassname() );
	if ( obj->isSubclassOf( Entity ) )
	{
		text += va( " (*%d) ", ( ( Entity * )obj )->entnum );
		if ( ( ( Entity * )obj )->Targeted() )
		{
			text += va( "'%s'", ( ( Entity * )obj )->TargetName() );
		}
	}
	else if ( obj->isSubclassOf( CThread ) )
	{
		text += va( " #%d:'%s'", ( ( CThread * )obj )->ThreadNum(), ( ( CThread * )obj )->ThreadName() );
	}
	
	switch( event->GetSource() )
	{
	default :
	case EV_FROM_CODE :
		text += " : Code :";
		break;
		
	case EV_FROM_SCRIPT :
		assert( event->GetThread() );
		if ( event->GetThread() )
		{
			text += va( " : %s(%d) :", event->GetThread()->Filename(), event->info.linenumber );
		}
		else
		{
			text += " : NULL :";
		}
		break;
		
	case EV_FROM_CONSOLE :
		text += " : Console :";
		break;
	}
	
	text += event->getName();
	n = event->NumArgs();
	for( i = 1; i <= n; i++ )
	{
		text += va( " %s", event->GetToken( i ) );
	}
	
	text += "\n";
	
	if ( !g_watch->integer || ( obj->isSubclassOf( Entity ) && ( g_watch->integer == ( ( Entity * )obj )->entnum ) ) )
	{
		if ( g_showevents->integer == 2 )
		{
			EVENT_DebugPrintf( text.c_str() );
		}
		else
		{
			EVENT_DPrintf( "%s", text.c_str() );
		}
	}
}

#else

void Event::PrintEvent( Event * event )
{
	int i;
	int n;
	str text;
	
	
	text = va( "%6.1f:%1d ", event->time, event->flags );
	
	switch( event->GetSource() )
	{
	default :
	case EV_FROM_CODE :
		text += " : Code :";
		break;
		
	case EV_FROM_CONSOLE :
		text += " : Console :";
		break;
	}
	
	text += event->getName();
	n = event->NumArgs();
	for( i = 1; i <= n; i++ )
	{
		text += va( " %s", event->GetToken( i ) );
	}
	
	text += "\n";
	
	if ( g_showevents->integer == 2 )
	{
		EVENT_DebugPrintf( text.c_str() );
	}
	else
	{
		EVENT_DPrintf( text.c_str() );
	}
}

#endif

void Event::PendingEvents( const char *mask )
{
	Event *event;
	int l, num;
	
	l = 0;
	if ( mask )
	{
		l = strlen( mask );
	}
	
	assert( EventQueue );
	assert( EventQueue->next );
	assert( EventQueue->prev );
	
	num = 0;
	event = EventQueue->next;
	while( event != EventQueue )
	{
		assert( event );
		assert( event->m_sourceobject );
		
		if ( !mask || !Q_stricmpn( event->getName(), mask, l ) )
		{
			num++;
			Event::PrintEvent( event );
		}
		
		event = event->next;
	}
	EVENT_Printf( "%d pending events as of %.2f\n", num, EVENT_time );
}

Event *Event::GetEventDef( int num )
{
	if ( !eventDefList )
	{
		return NULL;
	}
	
	assert( ( num > 0 ) && ( num <= eventDefList->NumObjects() ) );
	
	if ( ( num <= 0 ) || ( num > eventDefList->NumObjects() ) )
	{
		return NULL;
	}
	
	return eventDefList->ObjectAt( num );
}

int Event::NumEventDefs( void )
{
	if ( !eventDefList )
	{
		return 0;
	}
	
	return eventDefList->NumObjects();
}

void Event::PrintDocumentation( FILE *event_file, qboolean html )
{
	int i;
	int p;
	str text;
	
	if ( !html )
	{
		text = "   ";
		p = 0;
		
		if ( flags & EV_CONSOLE )
		{
			text[ p++ ] = '*';
		}
		if ( flags & EV_CHEAT )
		{
			text[ p++ ] = 'C';
		}
		if ( flags & EV_CACHE )
		{
			text[ p++ ] = '%';
		}
	}
	
	if ( html )
	{
		EV_Print( event_file, "\n<P><tt><B>%s</B>", name );
	}
	else
	{
		if ( text[ 0 ] != ' ' )
		{
			EV_Print( event_file, "\n%s:%s", text.c_str(), name );
		}
		else
		{
			EV_Print( event_file, "\n%s %s", text.c_str(), name );
		}
	}
	
	if ( definition )
	{
		if ( html )
		{
			EV_Print( event_file, "( <i>" );
		}
		else
		{
			EV_Print( event_file, "( " );
		}
		for( i = 1; i <= definition->NumObjects(); i++ )
		{
			definition->ObjectAt( i ).PrintArgument( event_file );
			if ( i < definition->NumObjects() )
            {
				EV_Print( event_file, ", " );
            }
		}
		if ( html )
		{
			EV_Print( event_file, " </i>)</tt><BR>\n" );
		}
		else
		{
			EV_Print( event_file, " )\n" );
		}
	}
	else
	{
		if ( html )
		{
			EV_Print( event_file, "</tt><BR>\n" );
		}
		else
		{
			EV_Print( event_file, "\n" );
		}
	}
	if ( documentation )
	{
		char new_doc[1024];
		int new_index = 0;
		
		for ( unsigned old_index = 0 ; old_index < strlen ( documentation ) ; old_index++ )
		{
			if ( documentation[old_index] == '\n' )
			{
				if ( html )
				{
					new_doc[new_index    ] = '<';
					new_doc[new_index + 1] = 'B';
					new_doc[new_index + 2] = 'R';
					new_doc[new_index + 3] = '>';
					new_doc[new_index + 4] = '\n';
					new_index += 5;
				}
				else
				{
					new_doc[new_index    ] = '\n';
					new_doc[new_index + 1] = '\t';
					new_doc[new_index + 2] = '\t';
					new_index += 3;
				}
			}
			else
			{
				new_doc[new_index] = documentation[old_index];
				new_index++;
			}
			
		}
		
		new_doc[new_index] = 0;
		
		if ( html )
		{
			//         EV_Print( event_file, "<BLOCKQUOTE>%s<BR></BLOCKQUOTE>\n", new_doc );
			EV_Print( event_file, "<ul>%s</ul>\n", new_doc );
		}
		else
		{
			EV_Print( event_file, "\t\t- %s\n", new_doc );
		}
	}
}

void Event::PrintEventDocumentation( Event * ePtr, FILE *event_file, qboolean html, int typeFlag )
{
	int flags;
	
	flags = ePtr->GetFlags();
	
	if ( flags & EV_CODEONLY && ( typeFlag == EVENT_SCRIPT_ONLY || typeFlag == EVENT_TIKI_ONLY ) )
	{
		return;
	}
	
	if ( (flags & EV_TIKIONLY) && typeFlag == EVENT_SCRIPT_ONLY )
		return;
	
	if ( (flags & EV_SCRIPTONLY) && typeFlag == EVENT_TIKI_ONLY )
		return;
	
	if ( (flags & EV_TIKIONLY) && typeFlag == EVENT_SCRIPT_ONLY )
		return;
	
	if ( (flags & EV_SCRIPTONLY) && typeFlag == EVENT_TIKI_ONLY )
		return;
	
	// purposely suppressed
	if ( ePtr->name[ 0 ] == '_' )
	{
		return;
	}
	
	ePtr->PrintDocumentation( event_file, html );
}

void Event::ListDocumentation( const char *mask, qboolean print_to_disk	)
{
	Event * ePtr;
	int num;
	int i;
	int n;
	int l;
	int flags;
	int hidden;
	str name;
	str text;
	FILE *event_file = NULL;
	str event_filename;
	
	if ( !eventDefList )
	{
		EVENT_DPrintf( "No events.\n" );
		return;
	}
	
	if ( print_to_disk )
	{
		if ( !mask || !mask[0] )
			event_filename = EVENT_FILENAME;
		else
			event_filename = str ( mask ) + ".txt";
		
		event_file = fopen( event_filename.c_str(), "w" );
		
		if ( event_file == NULL )
			return;
	}
	
	l = 0;
	if ( mask )
	{
		l = strlen( mask );
	}
	
	
	EV_Print( event_file, "\nCommand Documentation\n" );
	EV_Print( event_file, "=====================\n" );
	
	hidden = 0;
	num = 0;
	n = eventDefList->NumObjects();
	for( i = 1; i <= n; i++ )
	{
		ePtr = eventDefList->ObjectAt( i );
		flags = ePtr->GetFlags();
		name = ePtr->getName();
		
		if ( flags & EV_CODEONLY )
		{
			hidden++;
			continue;
		}
		
		if ( mask && Q_stricmpn( name, mask, l ) )
		{
			continue;
		}
		
		num++;
		
		ePtr->PrintDocumentation( event_file );
	}
	
	
	EV_Print( event_file, "\n* = console command.\nC = cheat command.\n% = cache command.\n\n"
		"Printed %d of %d total commands.\n", num, n - hidden );
	
	if ( developer->integer && hidden )
	{
		EV_Print( event_file, "Suppressed %d commands.\n", hidden );
	}
	
	if ( event_file != NULL )
	{
		EVENT_Printf( "Printed event info to file %s\n", event_filename.c_str() );
		fclose( event_file );
	}
}

void Event::initCommandList( void )
{
	int flags;
	str *n;
	
	flags = 0;
	commandList = new Container<str *>;
	
	n = new str( "NULL" );
	NullEvent.eventnum = commandList->AddObject( n );
	
	flagList = new Container<int>;
	flagList->AddObject( flags );
	
	sortedList = new Container<int>;
	sortedList->AddObject( NullEvent.eventnum );
	
	eventDefList = new Container<Event *>;
	
	dirtylist = false;
	
	NullEvent.data = NULL;
	NullEvent.info.inuse = 0;
	NullEvent.info.source = EV_FROM_CODE;
	NullEvent.info.flags = 0;
	NullEvent.info.linenumber = 0;
}

Event::Event()
{
	info.inuse = 0;
	info.source = EV_FROM_CODE;
	info.flags = 0;
	info.linenumber = 0;
	threadnum = -1;
	eventnum = 0;
	data = NULL;
	definition = NULL;
	name = NULL;
	returntype = NULL;
	documentation = NULL;
}

Event::Event( int num )
{
	if ( !commandList )
	{
		initCommandList();
	}
	
	assert( ( num > 0 ) && ( num <= commandList->NumObjects() ) );
	
	if ( ( num <= 0 ) || ( num > commandList->NumObjects() ) )
	{
		EVENT_WDPrintf( "Event %d out of range.\n", num );
		num = 0;
		name = NULL;
		info.flags = 0;
	}
	else
	{
		name = commandList->ObjectAt( num )->c_str();
		info.flags = flagList->ObjectAt( num );
	}
	
	eventnum = num;
	data = NULL;
	definition = NULL;
	returntype = NULL;
	documentation = NULL;
	info.inuse = 0;
	info.source = EV_FROM_CODE;
	info.linenumber = 0;
	threadnum = -1;
}

Event::Event( const Event &ev )
{
	int num;
	int i;
	
	//eventnum = ( int )ev;
	eventnum = ev.eventnum;
	assert( ( eventnum > 0 ) && ( eventnum <= commandList->NumObjects() ) );
	data = NULL;
	definition = NULL;
	returntype = NULL;
	documentation = NULL;
	
	name = commandList->ObjectAt( eventnum )->c_str();
	info.inuse = 0;
	info.source = ev.info.source;
	info.flags = ev.info.flags;
	info.linenumber = ev.info.linenumber;
	threadnum = ev.threadnum;
	
	if ( ev.data )
	{
		num = ev.data->NumObjects();
		
		data = new Container<EventVar>;
		data->Resize( num );
		
		for( i = 1; i <= num; i++ )
		{
			data->AddObject( ev.data->ObjectAt( i ) );
		}
	}
}

Event::Event( Event *ev )
{
	int num;
	int i;
	
	assert( ev );
	if ( !ev )
	{
		EVENT_Error( ERR_DROP, "NULL Event\n" );
	}
	
	//eventnum = ( int )*ev;
	eventnum = ev->eventnum;
	assert( ( eventnum > 0 ) && ( eventnum <= commandList->NumObjects() ) );
	data = NULL;
	definition = NULL;
	returntype = NULL;
	documentation = NULL;
	name = commandList->ObjectAt( eventnum )->c_str();
	info.inuse = 0;
	info.source = ev->info.source;
	info.flags = ev->info.flags;
	info.linenumber = ev->info.linenumber;
	threadnum = ev->threadnum;
	if ( ev->data )
	{
		num = ev->data->NumObjects();
		
		data = new Container<EventVar>;
		data->Resize( num );
		
		for( i = 1; i <= num; i++ )
		{
			data->AddObject( ev->data->ObjectAt( i ) );
		}
	}
}

Event::Event( const char *command, int flags, const char *theFormatspec, const char *theArgument_names, 
		const char *theDocumentation )
{
	str *t;
	
	if ( !commandList )
	{
		initCommandList();
	}
	
	eventnum = FindEvent( command );
	if ( !eventnum )
	{
		t = new str( command );
		eventnum = commandList->AddObject( t );
		// check for default flags
		if ( flags == EV_DEFAULT )
		{
			flags = 0;
		}
		flagList->AddObject( ( int )flags );
		sortedList->AddObject( eventnum );
		dirtylist = true;
	}
	
	// Use the name stored in the command list in case the string passed in
	// is not in static memory.
	name = commandList->ObjectAt( eventnum )->c_str();
	
	data = NULL;
	info.inuse = 0;
	info.source = EV_FROM_CODE;
	info.linenumber = 0;
	threadnum = -1;
	formatspec = theFormatspec;
	argument_names = theArgument_names;
	returntype = NULL;
	documentation = theDocumentation;
	definition = NULL;
	
	// If flags have changed, let the user know.  It's probably a development bug.
	int &flagobj = flagList->ObjectAt( eventnum );
	
	// check for default flags
	if ( flags == EV_DEFAULT )
	{
		flags = flagobj;
	}
	
	//assert( flags == flagobj );
	if ( flags != flagobj )
	{
		// Flags not equal.  Use combined value.
		flagobj |= flags;
	}
	
	info.flags = flagobj;
	
	// add it to the list for Documentation
	// suppress it if it starts with '_'
	if ( documentation && ( command[ 0 ] != '_' ) )
	{
		eventDefList->AddObject( ( Event * )this );
	}
	else
	{
		// purposely suppressed
		if ( command[ 0 ] != '_' )
		{
			// empty documentation!
			assert( 0 );
		}
	}
}

Event::Event( const char *command )
{
	if ( !commandList )
	{
		initCommandList();
	}
	
	eventnum = FindEvent( command );
	if ( !eventnum )
	{
		EVENT_WDPrintf( "Event '%s' does not exist.\n", command );
		name = NULL;
		info.flags = 0;
	}
	else
	{
		name = commandList->ObjectAt( eventnum )->c_str();
		info.flags = flagList->ObjectAt( eventnum );
	}
	
	data = NULL;
	definition = NULL;
	returntype = NULL;
	documentation = NULL;
	info.inuse = 0;
	info.source = EV_FROM_CODE;
	info.linenumber = 0;
	threadnum = -1;
}

Event::Event( const str &command )
{
	if ( !commandList )
	{
		initCommandList();
	}
	
	eventnum = FindEvent( command );
	if ( !eventnum )
	{
		EVENT_WDPrintf( "Event '%s' does not exist.\n", command.c_str() );
		name = NULL;
		info.flags = 0;
	}
	else
	{
		name = commandList->ObjectAt( eventnum )->c_str();
		info.flags = flagList->ObjectAt( eventnum );
	}
	
	data = NULL;
	definition = NULL;
	returntype = NULL;
	documentation = NULL;
	info.inuse = 0;
	info.source = EV_FROM_CODE;
	info.linenumber = 0;
	threadnum = -1;
}

Event::~Event()
{
	if ( data )
	{
		delete data;
		data = NULL;
	}
	if ( definition )
	{
		delete definition;
		definition = NULL;
	}
	
	if ( returntype )
	{
		delete returntype;
		returntype = NULL;
	}
}

void Event::SetupDocumentation( void )
{
	// setup documentation
	if ( formatspec )
	{
		if ( argument_names )
		{
			char        argumentNames[ 256 ];
			str         argSpec;
			str         rangeSpec;
			str         argName;
			EventArgDef argDef;
			const char  *namePtr;
			const char  *specPtr;
			int         specLength, index;
			Container<str> argNames;
			
			specLength = strlen( formatspec );
			specPtr = formatspec;
			//
			// store off all the names
			//
			strcpy( argumentNames, argument_names );
			namePtr = strtok( argumentNames, " " );
			while ( namePtr != NULL )
            {
				argNames.AddObject( str( namePtr ) );
				namePtr = strtok( NULL, " " );
            }
			
			index = 0;
			
			//
			// Create the return type, if any
			//
			if ( specLength && ( *specPtr == '@' ) )
            {
				if ( specLength < 2 )
				{
					assert( 0 );
					Error( "Return type indicated, but no return type specified in event %s\n", name );
				}
				
				// clear the rangeSpec
				rangeSpec = "";
				
				// get the argSpec
				argSpec = "";
				argSpec += specPtr[ 1 ];
				specPtr += 2;
				specLength -= 2;
				
				if ( index < argNames.NumObjects() )
				{
					argName = argNames.ObjectAt( index + 1 );
					
					returntype = new EventArgDef;
					returntype->Setup( name, argName, argSpec, rangeSpec );
				}
				else
				{
					assert( 0 );
					Error( "More format specifiers than argument names for event %s\n", name );
				}
				
				index++;
            }
			
			//
			// create the definition container
			//
			definition = new Container<EventArgDef>;
			definition->Resize( argNames.NumObjects() );
			
			// go throught he formatspec
			while( specLength )
            {
				// clear the rangeSpec
				rangeSpec = "";
				// get the argSpec
				argSpec = "";
				argSpec += *specPtr;
				specPtr++;
				specLength--;
				// see if there is a range specified
				while ( *specPtr == '[' )
				{
					// add in all the characters until NULL or ']'
					while( specLength && ( *specPtr != ']' ) )
					{
						rangeSpec += *specPtr;
						specPtr++;
						specLength--;
					}
					if ( specLength && ( *specPtr == ']' ) )
					{
						rangeSpec += *specPtr;
						specPtr++;
						specLength--;
					}
				}
				if ( index < argNames.NumObjects() )
				{
					argName = argNames.ObjectAt( index + 1 );
					argDef.Setup( name, argName, argSpec, rangeSpec );
					definition->AddObject( argDef );
				}
				else
				{
					assert( 0 );
					Error( "More format specifiers than argument names for event %s\n", name );
				}
				index++;
            }
			if ( index < argNames.NumObjects() )
            {
				assert( 0 );
				Error( "More argument names than format specifiers for event %s\n", name );
			}
		}
	}
}

void Event::DeleteDocumentation( void )
{
	// setup documentation
	if ( formatspec )
	{
		if ( argument_names )
		{
			definition->FreeObjectList();
			delete definition;
			definition = NULL;
		}
	}
	
	if ( returntype )
	{
		delete returntype;
		returntype = NULL;
	}
}

#ifdef GAME_DLL

void Event::SetThread( CThread *thread )
{
	if ( thread )
	{
		threadnum = thread->ThreadNum();
	}
	else
	{
		threadnum = -1;
	}
}

CThread *Event::GetThread( void )
{
	if ( threadnum != -1 )
	{
		return Director.GetThread( threadnum );
	}
	
	return NULL;
}

void Event::ReturnString( const char *text )
{
	CThread *thread;
	
	thread = GetThread();
	if ( thread )
	{
		thread->program->setString( OFS_RETURN, text );
	}
}

void Event::ReturnFloat( float value )
{
	CThread *thread;
	
	thread = GetThread();
	if ( thread )
	{
		thread->program->setFloat( OFS_RETURN, value );
	}
}

void Event::ReturnInteger( int value )
{
	CThread *thread;
	
	thread = GetThread();
	if ( thread )
	{
		thread->program->setInteger( OFS_RETURN, value );
	}
}

void Event::ReturnVector( const Vector &vec )
{
	CThread *thread;
	
	thread = GetThread();
	if ( thread )
	{
		thread->program->setVector( OFS_RETURN, vec );
	}
}

void Event::ReturnEntity( Entity *ent )
{
	CThread *thread;
	
	thread = GetThread();
	if ( thread )
	{
		thread->program->setEntity( OFS_RETURN, ent );
	}
}

#endif

void Event::Error( const char *fmt, ... )
{
	va_list			argptr;
	char				text[ 1024 ];
	
	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );
	
	switch( GetSource() )
	{
	default :
	case EV_FROM_CODE :
#if defined( GAME_DLL )
		EVENT_WDPrintf( "Game: '%s' : %s\n", getName(), text );
#elif defined( CGAME_DLL )
		EVENT_WDPrintf( "CGame: '%s' : %s\n", getName(), text );
#elif defined( UI_LIB )
		EVENT_WDPrintf( "UI_Lib: '%s' : %s\n", getName(), text );
#else
		EVENT_WDPrintf( "Client: '%s' : %s\n", getName(), text );
#endif
		break;
#ifdef GAME_DLL
	case EV_FROM_SCRIPT :
		{
			CThread	*thread = GetThread();
			const char *filename = "Dead script";
			if ( thread )
			{
				filename = thread->Filename();
			}
			EVENT_DPrintf( "%s(%d): '%s' :\n%s\n", filename, info.linenumber, getName(), text );
		}
		break;
		
	case EV_FROM_CONSOLE :
		gi.SendServerCommand( GetConsoleEdict()-g_entities, "print \"Console: '%s' : %s\n\"", getName(), text );
		break;
#endif
	}
}

qboolean Event::IsVectorAt( int pos )
{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
	{
		Error( "Index %d out of range.", pos );
		return false;
	}
	
	return data->ObjectAt( pos ).IsVector( *this );
}

#ifdef GAME_DLL
qboolean Event::IsEntityAt( int pos )
{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
	{
		Error( "Index %d out of range.", pos );
		return false;
	}
	
	return data->ObjectAt( pos ).IsEntity( *this );
}
#endif

qboolean Event::IsNumericAt( int pos )
{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
	{
		Error( "Index %d out of range.", pos );
		return false;
	}
	
	return data->ObjectAt( pos ).IsNumeric( *this );
}

#ifdef GAME_DLL
void Event::Archive( Archiver &arc )
{
	str name;
	int num;
	int i;
	EventVar var;
	EventArgDef def;
	
	if ( arc.Saving() )
	{
		name = getName();
	}
	arc.ArchiveString( &name );
	if ( arc.Loading() )
	{
		if ( data )
		{
			delete data;
			data = NULL;
		}
		*this = Event( name );
	}
	
	arc.ArchiveRaw( &info, sizeof( info ) );
	arc.ArchiveInteger( &threadnum );
	arc.ArchiveShort( &anim_number );
	arc.ArchiveShort( &anim_frame );
	arc.ArchiveSafePointer( &m_sourceobject );
	arc.ArchiveFloat( &time );
	arc.ArchiveInteger( &flags );
	
	// save data
	
	if ( arc.Saving() )
	{
		if ( !data )
		{
			num = 0;
		}
		else
		{
			num = data->NumObjects();
		}
	}
	arc.ArchiveInteger( &num );
	if ( arc.Loading() && num )
	{
		data = new Container<EventVar>;
		data->Resize( num );
	}
	
	for( i = 1; i <= num; i++ )
	{
		if ( arc.Saving() )
		{
			var = data->ObjectAt( i );
		}
		var.Archive( arc );
		if ( arc.Loading() )
		{
			data->AddObject( var );
		}
	}
	
	// save definition
	if ( arc.Saving() )
	{
		if ( !definition )
		{
			num = 0;
		}
		else
		{
			num = definition->NumObjects();
		}
	}
	arc.ArchiveInteger( &num );
	if ( arc.Loading() && num )
	{
		definition = new Container<EventArgDef>;
		definition->Resize( num );
	}
	
	for( i = 1; i <= num; i++ )
	{
		if ( arc.Saving() )
		{
			def = definition->ObjectAt( i );
		}
		def.Archive( arc );
		if ( arc.Loading() )
		{
			definition->AddObject( def );
		}
	}
	
	// save return type
	if ( arc.Saving() )
	{
		if ( returntype )
			num = 1;
		else
			num = 0;
	}
	
	arc.ArchiveInteger( &num );
	
	if ( num == 1 )
	{
		if ( arc.Loading() )
			returntype = new EventArgDef;
		
		arc.ArchiveObject( ( Class * )&returntype );
	}	
}

#endif

CLASS_DECLARATION( Class, Listener, NULL )
{
	{ &EV_Remove,							&Listener::Remove },
	{ &EV_ScriptRemove,						&Listener::ScriptRemove },

	{ NULL, NULL }
};

void Listener::Remove( Event * )
{
	delete this;
}

void Listener::ScriptRemove( Event * )
{
	// Forces the remove to be done at a safe time
	PostEvent( EV_Remove, 0.0f );
}

qboolean Listener::EventPending( Event &ev )
{
	Event *event;
	int eventnum;
	
	assert( EventQueue );
	assert( EventQueue->next );
	
	event = EventQueue->next;
	
	eventnum = ( int )ev;
	while( event != EventQueue )
	{
		if ( ( event->m_sourceobject.Pointer() == this ) && ( (int)*event == eventnum ) )
		{
			return true;
		}
		event = event->next;
	}
	
	return false;
}

inline qboolean Listener::CheckEventFlags( Event *event )
{
#ifdef GAME_DLL
	// Special handling of console events
	if ( event->GetSource() == EV_FROM_CONSOLE )
	{
		if ( !( event->info.flags & (EV_CONSOLE|EV_CHEAT) ) )
		{
			if ( isSubclassOf( Entity ) )
			{
				Entity *ent;
				
				ent = ( Entity * )this;
				gi.SendServerCommand( ent->edict-g_entities, "print \"Command not available from console.\n\"" );
			}
			
			// don't process
			return false;
		}
		
		// don't allow console cheats unless the server says it's ok.

		if (
			( event->info.flags & EV_CHEAT ) &&
			//( multiplayerManager.inMultiplayer() ) &&
			( !sv_cheats->integer )
			)
		{
			if ( isSubclassOf( Entity ) )
			{
				Entity *ent;
				
				ent = ( Entity * )this;
				gi.SendServerCommand( ent->edict-g_entities, "print \"You must run the server with '+set cheats 1' to enable this command.\n\"" );
			}
			
			// don't process
			return false;
		}
	}
#endif
	
	// ok to process
	return true;
}

qboolean Listener::ProcessEvent( Event *event )
{
	ClassDef *c;
	int		ev;
	
	if ( !Event::EventSystemStarted )
	{
		assert( 0 );
		return false;
	}
	
	if(event == NULL)
	{
		assert ( 0 );
		return false;
	}
	
	// make sure the event has only been used once
	if ( event->info.inuse )
	{
		EVENT_Error( ERR_DROP, "ProcessEvent : Event '%s' has already been posted.\n", event->getName() );
	}
	
	if ( g_showevents->integer )
	{
		Listener *obj;
		
		obj = event->GetSourceObject();
		if ( !obj )
		{
			event->SetSourceObject( this );
		}
		Event::PrintEvent( event );
	}
	
	ev = ( int )*event;
	c = this->classinfo();
	if ( ev >= c->numEvents	)
	{
		event->Error( "Event out of response range for class '%s'.  Event probably invalid or allocated late.\n", getClassname() );
		return false;
	}
	
	if ( c->responseLookup[ ev ] )
	{
		int start;
		int end;
		
		event->info.inuse++;
		
		if ( !g_timeevents->integer )
		{
			// only process the event if we allow it
			if ( CheckEventFlags( event ) )
			{
				( this->**c->responseLookup[ ev ] )( event );
				// Very dirty hack to dump event names to a text file
				// as they are posted
				//FILE *edata = fopen("edataclient.txt", "a");
				//fprintf(edata,"%s\n",event->getName());
				//fclose(edata);
			}
		}
		else
		{
			start = EVENT_realtime;
			
			// only process the event if we allow it
			if ( CheckEventFlags( event ) )
			{
				( this->**c->responseLookup[ ev ] )( event );
			}
			
			end = EVENT_realtime;
			
			if ( end - start >= g_timeevents->integer )
			{
#ifdef GAME_DLL
				if ( event != EV_Remove && this->isSubclassOf( Entity ) )
				{
					Entity *ent = (Entity *)this;
					
					EVENT_DPrintf( "(%d) ", ent->entnum );
				}
#endif
				
				EVENT_DebugPrintf( "'%s' : %d\n", event->getName(), end - start );
			}
		}
		
		// Prevent an event from being freed twice, in case it's been re-used
		event->info.inuse--;
		if ( !event->info.inuse )
		{
			delete event;
		}
		else
		{
			EVENT_Error( ERR_DROP, "ProcessEvent : Event '%s' is still in use after being processed.\n", event->getName() );
		}
		
		return true;
	}
	
	if ( !event->info.inuse )
	{
		delete event;
	}
	else
	{
		EVENT_Error( ERR_DROP, "ProcessEvent : Event '%s' is still in use after being processed.\n", event->getName() );
	}
	
	return false;
}

void Listener::PostEvent( Event *ev, float time, int flags )
{
	int evnum;
	ClassDef *c;
	Event *event;
	
#ifdef GAME_DLL
	if ( LoadingSavegame )
	{
		// while technically not bad, if we got here we have an event that is being posted while loading which is a bad thing
		assert( 0 );
		if ( !ev->info.inuse )
		{
			delete ev;
		}
		else
		{
			EVENT_Error( ERR_DROP, "PostEvent : Event '%s' is still in use during loading.\n", ev->getName() );
		}
		
		return;
	}
#endif
	
	if ( !Event::EventSystemStarted )
	{
		assert( 0 );
		return;
	}
	
	evnum = ( int )*ev;
	c = this->classinfo();
	if ( evnum >= c->numEvents	)
	{
		ev->Error( "Event out of response range for class '%s'.  Event probably invalid or allocated late.\n", getClassname() );
		return;
	}
	
	if ( !c->responseLookup[ evnum ] )
	{
		// we don't need it so lets delete it
		delete ev;
		return;
	}
	
	LL_Remove( ev, next, prev );
	
	ev->SetSourceObject( this );
	ev->time		= EVENT_time + time;
	ev->flags	= flags;
	
	event = EventQueue->next;
	while( ( event != EventQueue ) && ( ev->time >= event->time ) )
	{
		event = event->next;
	}
	
	LL_Add( event, ev, next, prev );
	numEvents++;
}

qboolean Listener::PostponeEvent( Event &ev, float time )
{
	Event *event;
	Event *node;
	int eventnum;
	
	assert( EventQueue );
	assert( EventQueue->next );
	
	eventnum = ( int )ev;
	
	event = EventQueue->next;
	while( event != EventQueue )
	{
		if ( ( event->GetSourceObject() == this ) && ( ( int )*event == eventnum ) )
		{
			event->time	+= time;
			
			node = event->next;
			while( ( node != EventQueue ) && ( event->time >= node->time ) )
			{
				node = node->next;
			}
			
			LL_Remove( event, next, prev );
			LL_Add( node, event, next, prev );
			
			return true;
		}
		event = event->next;
	}
	
	return false;
}

bool Listener::CancelEventsOfType( Event *ev )
{
	Event *event;
	Event *next;
	int eventnum;
	bool found = false;
	
	assert( EventQueue );
	assert( EventQueue->next );
	
	event = EventQueue->next;
	
	eventnum = (int)*ev;
	while( event != EventQueue )
	{
		next = event->next;
		if ( ( event->GetSourceObject() == this ) && ( (int)*event == eventnum ) )
		{
			LL_Remove( event, next, prev );
			numEvents--;
			delete event;
			found = true;
		}
		event = next;
	}

	return found;
}

void Listener::CancelFlaggedEvents( int flags )
{
	Event *event;
	Event *next;
	
	assert( EventQueue );
	assert( EventQueue->next );
	
	event = EventQueue->next;
	
	while( event != EventQueue )
	{
		next = event->next;
		if ( ( event->GetSourceObject() == this ) && ( event->flags & flags ) )
		{
			LL_Remove( event, next, prev );
			numEvents--;
			delete event;
		}
		event = next;
	}
}

void Listener::CancelPendingEvents( void )
{
	Event *event;
	Event *next;
	
	assert( EventQueue );
	assert( EventQueue->next );
	
	event = EventQueue->next;
	
	while( event != EventQueue )
	{
		next = event->next;
		if ( event->GetSourceObject() == this )
		{
			LL_Remove( event, next, prev );
			numEvents--;
			delete event;
		}
		event = next;
	}
}

qboolean Listener::ProcessPendingEvents( void )
{
	Event *event;
	qboolean processedEvents;
	float t;
	
	assert( EventQueue );
	assert( EventQueue->next );
	assert( EventQueue->prev );
	
	processedEvents = false;
	
	t = EVENT_time + 0.001f;
	
	event = EventQueue->next;
	while( event != EventQueue )
	{
		Listener * obj;
		
		assert( event );
		
		obj = event->GetSourceObject();
		
		if ( event->time > t )
		{
			break;
		}
		
		if ( obj != this )
		{
			// traverse normally
			event = event->next;
		}
		else
		{
			// the event is removed from its list and temporarily added to the active list
			LL_Remove( event, next, prev );
			numEvents--;
			LL_Add( ActiveEvents, event, next, prev );
			
			
			// ProcessEvent will dispose of this event when it is done
			obj->ProcessEvent( event );
			
			// start over, since can't guarantee that we didn't process any previous or following events
			event = EventQueue->next;
			
			processedEvents = true;
		}
	}
	
	return processedEvents;
}

Listener::~Listener()
{
	if ( Event::EventSystemStarted )
		CancelPendingEvents();
}

void L_ClearEventList( void )
{
	Event *event;
	
	// go through active and event queue lists
	while( !LL_Empty( EventQueue, next, prev ) )
	{
		event = EventQueue->next;
		numEvents--;
		LL_Remove( event, next, prev );
		delete event;
	}
	
	while( !LL_Empty( ActiveEvents, next, prev ) )
	{
		event = ActiveEvents->next;
		LL_Remove( event, next, prev );
		delete event;
	}
	
	numEvents = 0;
}

void L_ProcessPendingEvents( void )
{
	Event *event;
	float t;
	int num;
	int maxevents;
	
	assert( EventQueue );
	assert( EventQueue->next );
	assert( EventQueue->prev );
	
	maxevents = ( int )g_eventlimit->integer;
	
	num = 0;
	t = EVENT_time + 0.001f;
	while( !LL_Empty( EventQueue, next, prev ) )
	{
		Listener * obj;
		
		event = EventQueue->next;
		
		assert( event );
		
		obj = event->GetSourceObject();
		
		assert( obj );
		
		if ( event->time > t )
		{
			break;
		}
		
		// the event is removed from its list and temporarily added to the active list
		LL_Remove( event, next, prev );
		numEvents--;
		LL_Add( ActiveEvents, event, next, prev );
		
		
		// ProcessEvent will dispose of this event when it is done
		obj->ProcessEvent( event );
		
		// Don't allow ourselves to stay in here too long.  An abnormally high number
		// of events being processed is evidence of an infinite loop of events.
		num++;
		if ( num > maxevents )
		{
			EVENT_WPrintf( "Event overflow.  Possible infinite loop in script.  "
				"Enable g_showevents to trace.  Aborting frame.\n" );
			break;
		}
	}
	
	if ( g_eventstats->integer )
	{
		if ( g_eventstats->integer == 1 )
		{
			EVENT_Printf( "finds %d, searches %d, num first search %d, avg %f\n",
				Event::numfinds, Event::numcompares, Event::numfirstsearch,
				( float )Event::numcompares / ( float )Event::numfinds );
		}
		else if ( g_eventstats->integer == 2 )
		{
			EVENT_Printf( "pending %5d free %5d active+pending %4d\n",
				numEvents, numFreeEvents, numFreeEvents + numEvents );
		}
	}
}

#ifdef GAME_DLL

void Event::SetConsoleEdict( const gentity_t *consoleedict )
{
	// linenumber does double duty in the case of the console commands
	if ( consoleedict )
	{
		info.linenumber = consoleedict->s.number;
	}
	else
	{
		// default to player 1
		info.linenumber = 0;
	}
}

gentity_t *Event::GetConsoleEdict( void )
{
	// linenumber does double duty in the case of the console commands
	if ( ( info.source != EV_FROM_CONSOLE ) || ( game.maxclients && (info.linenumber >= (unsigned)game.maxclients )) )
	{
		// default to player 1 for release
		return &g_entities[ 0 ];
	}
	
	return &g_entities[ info.linenumber ];
}

void L_ArchiveEvents( Archiver &arc )
{
	Event *event;
	int num;
	
	assert( EventQueue );
	assert( EventQueue->next );
	assert( EventQueue->prev );
	
	num = 0;
	for( event = EventQueue->next; event != EventQueue; event = event->next )
	{
		Listener * obj;
		
		assert( event );
		
		obj = event->GetSourceObject();
		
		assert( obj );
		
		if ( obj->isSubclassOf( Entity ) &&
			( ( ( Entity * )obj )->flags & FlagDontsave ) )
		{
			continue;
		}
		
		num++;
	}
	
	arc.ArchiveInteger( &num );
	for( event = EventQueue->next; event != EventQueue; event = event->next )
	{
		Listener * obj;
		
		assert( event );
		
		obj = event->GetSourceObject();
		
		assert( obj );
		
		if ( obj->isSubclassOf( Entity ) &&
			( ( ( Entity * )obj )->flags & FlagDontsave ) )
		{
			continue;
		}
		
		arc.ArchiveEvent( event );
		arc.ArchiveFloat( &event->time );
		arc.ArchiveInteger( &event->flags );
	}
}

void L_UnarchiveEvents( Archiver &arc )
{
	Event *e;
	int i;
	
	// the FreeEvents list would already be allocated at this point
	// clear out any events that may exist
	L_ClearEventList();
	
	arc.ArchiveInteger( &numEvents );
	for( i = 0; i < numEvents; i++ )
	{
		e = new Event();
		LL_Remove( e, next, prev );
		arc.ArchiveEvent( e );
		arc.ArchiveFloat( &e->time );
		arc.ArchiveInteger( &e->flags );
		
		LL_Add( EventQueue, e, next, prev );
	}
}

#endif

void Event::InitializeDocumentation( void )
{
	Event * ePtr;
	int i;
	
	for( i = 1; i <= eventDefList->NumObjects(); i++ )
	{
		ePtr = eventDefList->ObjectAt( i );
		ePtr->SetupDocumentation();
	}
}

void Event::ShutdownDocumentation( void )
{
	Event * ePtr;
	int i;
	
	for( i = 1; i <= eventDefList->NumObjects(); i++ )
	{
		ePtr = eventDefList->ObjectAt( i );
		ePtr->DeleteDocumentation();
	}
}

void L_InitEvents( void )
{
	if ( Event::EventSystemStarted )
	{
		assert( 0 );
		L_ShutdownEvents();
	}
	
	Event::lastevent = 0;
	Event::numfinds = 0;
	Event::numcompares = 0;
	Event::numfirstsearch = 0;
	Event_totalmemallocated = 0;
	
#if defined( GAME_DLL )
	g_showevents = gi.cvar ( "g_showevents", "0", 0 );
	g_eventlimit = gi.cvar ( "g_eventlimit", "5000", 0 );
	g_timeevents = gi.cvar ( "g_timeevents", "0", 0 );
	g_watch      = gi.cvar ( "g_watch", "0", 0 );
	g_eventstats = gi.cvar ( "g_eventstats", "0", 0 );
#elif defined( CGAME_DLL )
	g_showevents = cgi.Cvar_Get ( "cg_showevents", "0", 0 );
	g_eventlimit = cgi.Cvar_Get ( "cg_eventlimit", "500", 0 );
	g_timeevents = cgi.Cvar_Get ( "cg_timeevents", "0", 0 );
	g_eventstats = cgi.Cvar_Get ( "cg_eventstats", "0", 0 );
#else
	g_showevents = Cvar_Get ( "cl_showevents", "0", 0 );
	g_eventlimit = Cvar_Get ( "cl_eventlimit", "500", 0 );
	g_timeevents = Cvar_Get ( "cl_timeevents", "0", 0 );
	g_eventstats = Cvar_Get ( "cl_eventstats", "0", 0 );
#endif
	
	BuildEventResponses();
	
	Event::InitializeEventLists();
	
	L_ClearEventList();
	
	// setup the documentation on all the events
	Event::InitializeDocumentation();
	
	// Sort the list before we go on since we won't be adding any more events
	Event::SortEventList();
	
	// the event system has started
	Event::EventSystemStarted = true;
}

void L_ShutdownEvents( void )
{
	int i;
	
	if ( !Event::EventSystemStarted )
		return;
	
	Event::ShutdownEventLists();
	
	// deletes all the documentation
	Event::ShutdownDocumentation();
	
	if ( Event::commandList )
	{
		for ( i=Event::commandList->NumObjects(); i>0; i-- )
		{
			str *s;
			
			s = Event::commandList->ObjectAt( i );
			assert( s );
			if ( s )
				delete s;
		}
		delete Event::commandList;
		Event::commandList = NULL;
	}
	
	if ( Event::eventDefList )
	{
		delete Event::eventDefList;
		Event::eventDefList = NULL;
	}
	
	if ( Event::flagList )
	{
		delete Event::flagList;
		Event::flagList = NULL;
	}
	
	if ( Event::sortedList )
	{
		delete Event::sortedList;
		Event::sortedList = NULL;
	}
	
	// say it is now shutdown
	Event::EventSystemStarted = false;
}
