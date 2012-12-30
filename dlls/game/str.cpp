//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/str.cpp                                   $
// $Revision:: 9                                                              $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 4:08p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Simple, DLL portable string class
//
// WARNING: This file is shared between game, cgame and possibly the user interface.
// It is instanced in each one of these directories because of the way that SourceSafe works.
//

#include "str.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <game/q_shared.h>

#ifdef _WIN32
#pragma warning(disable : 4244)     // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4710)     // function 'blah' not inlined
#endif

#if defined( GAME_DLL )

#include "g_local.h"
#define STR_Error gi.Error

#elif defined ( CGAME_DLL )

#include "cg_local.h"
#define STR_Error cgi.Error

#else

//#include "client.h"
#define STR_Error Com_Error

#endif

static const int STR_ALLOC_GRAN = 20;

char *str::tolower( char *s1 )
{
	char *s;
	
	s = s1;
	while( *s )
	{
		*s = ::tolower( *s );
		s++;
	   }
	
	return s1;
}

char *str::toupper( char *s1 )
{
	char *s;
	
	s = s1;
	while( *s )
	{
		*s = ::toupper( *s );
		s++;
	   }
	
	return s1;
}

int str::icmpn( const char *s1, const char *s2, int n )
{
	int c1;
	int c2;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;
		
		if ( !n-- )
		{
			// strings are equal until end point
			return 0;
		}
		
		if ( c1 != c2 )
		{
			if ( c1 >= 'a' && c1 <= 'z' )
            {
				c1 -= ( 'a' - 'A' );
			}
			
			if ( c2 >= 'a' && c2 <= 'z' )
            {
				c2 -= ( 'a' - 'A' );
			}
			
			if ( c1 < c2 )
            {
				// strings less than
				return -1;
			}
			else if ( c1 > c2 )
            {
				// strings greater than
				return 1;
            }
		}
	   }
	while( c1 );
	
	// strings are equal
	return 0;
}

int str::icmp( const char *s1, const char *s2 )
{
	int c1;
	int c2;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;
		
		if ( c1 != c2 )
		{
			if ( c1 >= 'a' && c1 <= 'z' )
            {
				c1 -= ( 'a' - 'A' );
			}
			
			if ( c2 >= 'a' && c2 <= 'z' )
            {
				c2 -= ( 'a' - 'A' );
			}
			
			if ( c1 < c2 )
            {
				// strings less than
				return -1;
			}
			else if ( c1 > c2 )
            {
				// strings greater than
				return 1;
            }
		}
	   }
	while( c1 );
	
	// strings are equal
	return 0;
}

int str::cmpn( const char *s1, const char *s2, int n )
{
	int c1;
	int c2;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;
		
		if ( !n-- )
		{
			// strings are equal until end point
			return 0;
		}
		
		if ( c1 < c2 )
		{
			// strings less than
			return -1;
		}
		else if ( c1 > c2 )
		{
			// strings greater than
			return 1;
		}
	   }
	while( c1 );
	
	// strings are equal
	return 0;
}

int str::cmp( const char *s1, const char *s2 )
{
	int c1;
	int c2;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;
		
		if ( c1 < c2 )
		{
			// strings less than
			return -1;
		}
		else if ( c1 > c2 )
		{
			// strings greater than
			return 1;
		}
	   }
	while( c1 );
	
	// strings are equal
	return 0;
}

/*
============
IsNumeric

Checks a string to see if it contains only numerical values.
============
*/
bool str::isNumeric( const char *str )
{
	int len;
	int i;
	bool dot;
	
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

str operator+( const str& a, const float b )
{
	char text[ 20 ];
	
	str result( a );
	
	sprintf( text, "%f", b );
	result.append( text );
	
	return result;
}

str operator+( const str& a, const int b )
{
	char text[ 20 ];
	
	str result( a );
	
	sprintf( text, "%d", b );
	result.append( text );
	
	return result;
}

str operator+( const str& a, const unsigned b )
{
	char text[ 20 ];
	
	str result( a );
	
	sprintf( text, "%u", b );
	result.append( text );
	
	return result;
}

str& str::operator+=( const float a )
{
	char text[ 20 ];
	
	sprintf( text, "%f", a );
	append( text );
	
	return *this;
}

str& str::operator+=( const int a )
{
	char text[ 20 ];
	
	sprintf( text, "%d", a );
	append( text );
	
	return *this;
}

str& str::operator+=( const unsigned a )
{
	char text[ 20 ];
	
	sprintf( text, "%u", a );
	append( text );
	
	return *this;
}

void str::CapLength( int newlen )
{
	assert ( data );
	
	if ( length() <= newlen )
		return;
	
	data[newlen] = 0;
	len = newlen;
}

void str::EnsureAlloced( int amount, bool keepold )
{
	if ( !data )
		data = buffer;
	
	char *newbuffer;
	bool wasalloced = ( alloced != 0 );
	int old_size;
	
	// See if enough already allocated
	
	if ( amount <= alloced )
		return;
	
	old_size = alloced;
	
	assert ( amount );
	
	// Calculate new alloc size
	
	int newsize, mod;
	
	mod = amount % STR_ALLOC_GRAN;
	
	if ( !mod )
		newsize = amount;
	else
		newsize = amount + STR_ALLOC_GRAN - mod;
	
	alloced = newsize;
	
	// Allocate the new buffer
	
	newbuffer = new char[ alloced ];
	
	if ( !newbuffer )
		STR_Error( ERR_FATAL, "Str: failed on allocation of %i bytes", alloced );
	
	if ( wasalloced && keepold )
	{
		strncpy ( newbuffer, data, old_size );
	}
	
	// Delete old buffer if necessary
	
	if ( data != buffer )
	{
		delete [] data;
	}
	
	data = newbuffer;
}

void str::BackSlashesToSlashes( void )
{
	int i;
	
	for ( i = 0 ; i < len ; i++ )
	{
		if ( data[i] == '\\' )
			data[i] = '/';
	}
}

void str::snprintf( char *dst, int size, const char *fmt, ... )
{
	char buffer[0x10000];
	int		len;
	va_list		argptr;
	
	va_start (argptr,fmt);
	len = vsprintf (buffer,fmt,argptr);
	va_end (argptr);
	
	assert ( len < size );
	
	strncpy (dst, buffer, size-1);
}

#ifdef _WIN32
#pragma warning(disable : 4189)		// local variable is initialized but not referenced
#endif

/*
=================
TestStringClass

This is a fairly rigorous test of the str class's functionality.
Because of the fairly global and subtle ramifications of a bug occuring
in this class, it should be run after any changes to the class.
Add more tests as functionality is changed.  Tests should include
any possible bounds violation and NULL data tests.
=================
*/
void TestStringClass( void )
{
	char	ch;							// ch == ?
	str	*t;							// t == ?
	str	a;								// a.len == 0, a.data == "\0"
	str	b;								// b.len == 0, b.data == "\0"
	str	c( "test" );				// c.len == 4, c.data == "test\0"
	str	d( c );						// d.len == 4, d.data == "test\0"
	str	e( reinterpret_cast<const char *>(NULL) );
	// e.len == 0, e.data == "\0"					ASSERT!
	int	i;								// i == ?
	
	i = a.length();					// i == 0
	i = c.length();					// i == 4
	
	const char *s1 = a.c_str();	// s1 == "\0"
	const char *s2 = c.c_str();	// s2 == "test\0"
	
	t = new str();						// t->len == 0, t->data == "\0"
	delete t;							// t == ?
	
	b = "test";							// b.len == 4, b.data == "test\0"
	t = new str( "test" );			// t->len == 4, t->data == "test\0"
	delete t;							// t == ?
	
	a = c;								// a.len == 4, a.data == "test\0"
	//   a = "";
	a = NULL;							// a.len == 0, a.data == "\0"					ASSERT!
	a = c + d;							// a.len == 8, a.data == "testtest\0"
	a = c + "wow";						// a.len == 7, a.data == "testwow\0"
	a = c + reinterpret_cast<const char *>(NULL);
	// a.len == 4, a.data == "test\0"			ASSERT!
	a = "this" + d;					// a.len == 8, a.data == "thistest\0"
	a = reinterpret_cast<const char *>(NULL) + d;
	// a.len == 4, a.data == "test\0"			ASSERT!
	a += c;								// a.len == 8, a.data == "testtest\0"
	a += "wow";							// a.len == 11, a.data == "testtestwow\0"
	a += reinterpret_cast<const char *>(NULL);
	// a.len == 11, a.data == "testtestwow\0"	ASSERT!
	
	a = "test";							// a.len == 4, a.data == "test\0"
	ch = a[ 0 ];						// ch == 't'
	ch = a[ -1 ];						// ch == 0											ASSERT!
	ch = a[ 1000 ];					// ch == 0											ASSERT!
	ch = a[ 0 ];						// ch == 't'
	ch = a[ 1 ];						// ch == 'e'
	ch = a[ 2 ];						// ch == 's'
	ch = a[ 3 ];						// ch == 't'
	ch = a[ 4 ];						// ch == '\0'										ASSERT!
	ch = a[ 5 ];						// ch == '\0'										ASSERT!
	
	a[ 1 ] = 'b';						// a.len == 4, a.data == "tbst\0"
	a[ -1 ] = 'b';						// a.len == 4, a.data == "tbst\0"			ASSERT!
	a[ 0 ] = '0';						// a.len == 4, a.data == "0bst\0"
	a[ 1 ] = '1';						// a.len == 4, a.data == "01st\0"
	a[ 2 ] = '2';						// a.len == 4, a.data == "012t\0"
	a[ 3 ] = '3';						// a.len == 4, a.data == "0123\0"
	a[ 4 ] = '4';						// a.len == 4, a.data == "0123\0"			ASSERT!
	a[ 5 ] = '5';						// a.len == 4, a.data == "0123\0"			ASSERT!
	a[ 7 ] = '7';						// a.len == 4, a.data == "0123\0"			ASSERT!
	
	a = "test";							// a.len == 4, a.data == "test\0"
	b = "no";							// b.len == 2, b.data == "no\0"
	
	i = ( a == b );					// i == 0
	i = ( a == c );					// i == 1
	
	i = ( a == "blow" );				// i == 0
	i = ( a == "test" );				// i == 1
	i = ( a == NULL );				// i == 0											ASSERT!
	
	i = ( "test" == b );				// i == 0
	i = ( "test" == a );				// i == 1
	i = ( NULL == a );				// i == 0											ASSERT!
	
	i = ( a != b );					// i == 1
	i = ( a != c );					// i == 0
	
	i = ( a != "blow" );				// i == 1
	i = ( a != "test" );				// i == 0
	i = ( a != NULL );				// i == 1											ASSERT!
	
	i = ( "test" != b );				// i == 1
	i = ( "test" != a );				// i == 0
	i = ( NULL != a );				// i == 1											ASSERT!
	
	a = "test";                   // a.data == "test"
	b = a;                        // b.data == "test"
	
	a = "not";                   // a.data == "not", b.data == "test"
	
	a = b;                        // a.data == b.data == "test"
	
	a += b;                       // a.data == "testtest", b.data = "test"
	
	a = b;
	
	a[1] = '1';                   // a.data = "t1st", b.data = "test"
	
	a = "";								// a.data == "", a.len == 0
	
	for( i = 0 ; i < ( STRING_PREALLOC_SIZE + 5 ) ; i++ )
	{
		a += "a";
	}
	
	t = new str( "testtesttesttesttesttesttesttest" );			// t->len == 32
	delete t;							// t == ?
}

#ifdef _WIN32
#pragma warning(default : 4189)		// local variable is initialized but not referenced
#pragma warning(disable : 4514)     // unreferenced inline function has been removed
#endif
