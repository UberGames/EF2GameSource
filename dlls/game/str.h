//----------------------------------------------------------------------------- 
//
//  $Logfile:: /Code/DLLs/game/str.h                                         $
// $Revision:: 16                                                             $
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
// Simple, DLL portable string class
//
// WARNING: This file is shared between game, cgame and possibly the user interface.
// It is instanced in each one of these directories because of the way that SourceSafe works.
//

#ifndef __STR_H__
#define __STR_H__

#include <qcommon/platform.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#pragma warning(disable : 4710)     // function 'blah' not inlined
#pragma warning(disable : 4996)
#endif

#define STRING_PREALLOC_SIZE 16

void TestStringClass( void );

#if 1
class __declspec( dllexport ) str
#else
class str : public Class
#endif
	{
	protected:

		char *data;
		char buffer[ STRING_PREALLOC_SIZE ];
      int alloced;
      int len;

      void              EnsureAlloced ( int, bool keepold = true );

  	public:
								~str();
								str();
								str( const char *text );
								str( const str& text );
								str( const str text, int start, int end );
								str( const char ch );
                        str( const int num );
								str( const float num );
								str( const unsigned num );


					int		length( void ) const;
		inline const		char *	c_str( void ) const;

					void		append( const char *text );
					void		append( const str& text );

					char		operator[]( int index ) const;
					char&		operator[]( int index );

					str&		operator=( const str& text );
					str&		operator=( const char *text );

		friend	str		operator+( const str& a, const str& b );
		friend	str		operator+( const str& a, const char *b );
		friend	str		operator+( const char *a, const str& b );

      friend	str		operator+( const str& a, const float b );
      friend	str		operator+( const str& a, const int b );
      friend	str		operator+( const str& a, const unsigned b );
      friend	str		operator+( const str& a, const bool b );
      friend	str		operator+( const str& a, const char b );

					str&		operator+=( const str& a );
					str&		operator+=( const char *a );
					str&		operator+=( const float a );
					str&		operator+=( const char a );
					str&		operator+=( const int a );
					str&		operator+=( const unsigned a );
					str&		operator+=( const bool a );

		friend	bool		operator==(	const str& a, const str& b );
		friend	bool		operator==(	const str& a, const char *b );
		friend	bool		operator==(	const char *a, const str& b );

		friend	bool		operator!=(	const str& a, const str& b );
		friend	bool		operator!=(	const str& a, const char *b );
		friend	bool		operator!=(	const char *a, const str& b );

                        operator const char * () const;

               int      icmpn( const char *text, int n ) const;
               int      icmpn( const str& text, int n ) const;
               int      icmp( const char *text ) const;
               int      icmp( const str& text ) const;
               int      cmpn( const char *text, int n ) const;
               int      cmpn( const str& text, int n ) const;
					int      cmp( const char *text ) const;
               int      cmp( const str& text ) const;

               char		*tolower( void );
               char		*toupper( void );

      static   char     *tolower( char *s1 );
      static   char     *toupper( char *s1 );

      static   int      icmpn( const char *s1, const char *s2, int n );
      static   int      icmp( const char *s1, const char *s2 );
      static   int      cmpn( const char *s1, const char *s2, int n );
      static   int      cmp( const char *s1, const char *s2 );

      static   void     snprintf ( char *dst, int size, const char *fmt, ... );

      static   bool	   isNumeric( const char *str );
               bool	   isNumeric( void ) const;

               void     CapLength ( int );

               void     BackSlashesToSlashes ();
   };


inline char str::operator[]( int index ) const
	{
   assert ( data );

   if ( !data )
      return 0;

	// don't include the '/0' in the test, because technically, it's out of bounds
	assert( ( index >= 0 ) && ( index < len ) );

	// In release mode, give them a null character
	// don't include the '/0' in the test, because technically, it's out of bounds
	if ( ( index < 0 ) || ( index >= len ) )
		{
		return 0;
		}

	return data[ index ];
	}

inline int str::length( void ) const
	{
   return len;
	}

inline str::~str()
	{
	if ( data != buffer )
		delete [] data;
	}

inline str::str()
	{
	data = buffer;
	alloced = STRING_PREALLOC_SIZE;
	data[ 0 ] = 0;
	len = 0;
	}

inline str::str
	(
	const char *text
   )

	{
   int stringLength;

	assert( text );

	data = buffer;
	alloced = STRING_PREALLOC_SIZE;
	data[ 0 ] = 0;
	len = 0;

	if ( text )
		{
      stringLength = strlen( text );
		EnsureAlloced ( stringLength + 1 );
		strcpy( data, text );
      len = stringLength;
		}
	}

inline str::str
	(
	const str& text
   )

	{
	data = buffer;
	alloced = STRING_PREALLOC_SIZE;
	data[ 0 ] = 0;
	len = 0;

	EnsureAlloced ( text.length() + 1);
	strcpy( data, text.c_str() );
   len = text.length();
   }

inline str::str
	(
	const str text,
	int start,
	int end
   )

	{
	int i;
   int stringLength;

	data = buffer;
	alloced = STRING_PREALLOC_SIZE;
	data[ 0 ] = 0;
	len = 0;

	if ( end > text.length() )
		{
		end = text.length();
		}

	if ( start > text.length() )
		{
		start = text.length();
		}

	stringLength = end - start;
	if ( stringLength < 0 )
		{
		stringLength = 0;
		}

   EnsureAlloced ( stringLength + 1 );

	for( i = 0; i < stringLength; i++ )
		{
		data[ i ] = text[ start + i ];
		}

	data[ stringLength ] = 0;
   len = stringLength;
	}

inline str::str
   (
   const char ch
   )

   {
	data = buffer;
	alloced = STRING_PREALLOC_SIZE;

   EnsureAlloced ( 2 );

   data[ 0 ] = ch;
   data[ 1 ] = 0;
   len = 1;
   }

inline str::str
   (
   const float num
   )

   {
   char text[ 32 ];
   int stringLength;

	data = buffer;
	alloced = STRING_PREALLOC_SIZE;

   sprintf( text, "%.3f", num );
   stringLength = strlen( text );
   EnsureAlloced( stringLength + 1 );
   strcpy( data, text );
   len = stringLength;
   }

inline str::str
   (
   const int num
   )

   {
   char text[ 32 ];
   int stringLength;

	data = buffer;
	alloced = STRING_PREALLOC_SIZE;

   sprintf( text, "%d", num );
   stringLength = strlen( text );
   EnsureAlloced( stringLength + 1 );
   strcpy( data, text );
   len = stringLength;
   }

inline str::str
   (
   const unsigned num
   )

   {
   char text[ 32 ];
   int stringLength;

	data = buffer;
	alloced = STRING_PREALLOC_SIZE;

   sprintf( text, "%u", num );
   stringLength = strlen( text );
   EnsureAlloced( stringLength + 1 );
   strcpy( data, text );
   len = stringLength;
   }

inline const char *str::c_str( void ) const
	{
	//assert( data );

	return data;
	}

inline void str::append
	(
	const char *text
	)

	{
   int new_length;

	assert( text );

	if ( text )
		{
		new_length = length();
		new_length += strlen( text );
		EnsureAlloced( new_length + 1 );

      strcat( data, text );
      len = new_length;
		}
	}

inline void str::append
	(
	const str& text
	)

	{
   int new_length;

   new_length = length();
   new_length += text.length();
   EnsureAlloced ( new_length + 1 );

   strcat ( data, text.c_str () );
   len = new_length;
	}


inline char& str::operator[]
	(
	int index
	)

	{
	// Used for result for invalid indices
	static char dummy = 0;
   assert ( data );

   if ( !data )
      return dummy;

	// don't include the '/0' in the test, because technically, it's out of bounds
	assert( ( index >= 0 ) && ( index < len ) );

	// In release mode, let them change a safe variable
	// don't include the '/0' in the test, because technically, it's out of bounds
	if ( ( index < 0 ) || ( index >= len ) )
		{
		return dummy;
		}

	return data[ index ];
	}

inline str& str::operator=
	(
	const str& text
	)

	{
   EnsureAlloced ( text.length() + 1, false );
	strcpy( data, text.c_str() );
   len = text.length();
	return *this;
   }

inline str& str::operator=
	(
	const char *text
	)

	{
   int stringLength;

	assert( text );

	if ( !text )
		{
		// safe behaviour if NULL
		EnsureAlloced ( 1, false );
      data[0] = 0;
      len = 0;
      return *this;
		}

   if ( text == data )
      return *this; // Copying same thing.  Punt.

   // Now we need to check if we're aliasing..

   unsigned int dataStart = reinterpret_cast<unsigned int> (data);
   unsigned int dataEnd = reinterpret_cast<unsigned int> (data) + len;
   unsigned int textStart = reinterpret_cast<unsigned int> (text);
   if ( textStart >= dataStart && textStart <= dataEnd )
      {
      // Great, we're aliasing.  We're copying from inside ourselves.
      // This means that I don't have to ensure that anything is alloced,
      // though I'll assert just in case.
      int diff = text - data;
      int i;

      assert ( strlen ( text ) < (unsigned) len );

      for ( i = 0; text[i]; i++ )
         {
         data[i] = text[i];
         }

      data[i] = 0;

      len -= diff;

      return *this;
      }

	stringLength = strlen( text );
   EnsureAlloced ( stringLength + 1, false );
	strcpy( data, text );
   len = stringLength;
	return *this;
	}

inline str operator+
	(
	const str& a,
	const str& b
	)

	{
	str result( a );

	result.append( b );

	return result;
	}

inline str operator+
	(
	const str& a,
	const char *b
	)

	{
	str result( a );

	result.append( b );

	return result;
	}

inline str operator+
	(
	const char *a,
	const str& b
	)

	{
	str result( a );

	result.append( b );

	return result;
	}

inline str operator+
   (
   const str& a,
   const bool b
   )

   {
	str result( a );

   result.append( b ? "true" : "false" );

	return result;
   }

inline str operator+
	(
   const str& a,
	const char b
	)

	{
   char text[ 2 ];

   text[ 0 ] = b;
   text[ 1 ] = 0;

	return a + text;
	}

inline str& str::operator+=
	(
	const str& a
	)

	{
	append( a );
	return *this;
	}

inline str& str::operator+=
	(
	const char *a
	)

	{
	append( a );
	return *this;
	}

inline str& str::operator+=
	(
	const char a
	)

	{
   char text[ 2 ];

   text[ 0 ] = a;
   text[ 1 ] = 0;
	append( text );

   return *this;
	}

inline str& str::operator+=
	(
	const bool a
	)

	{
   append( a ? "true" : "false" );
	return *this;
	}

inline bool operator==
	(
	const str& a,
	const str& b
	)

	{
	// Check if lengths are equal
	if( a.length() != b.length() )
		return( false );

	return ( !strcmp( a.c_str(), b.c_str() ) );
	}

inline bool operator==
	(
	const str& a,
	const char *b
	)

	{
	assert( b );
	if ( !b )
		{
		return false;
		}
	return ( !strcmp( a.c_str(), b ) );
	}

inline bool operator==
	(
	const char *a,
	const str& b
	)

	{
	assert( a );
	if ( !a )
		{
		return false;
		}
	return ( !strcmp( a, b.c_str() ) );
	}

inline bool operator!=
	(
	const str& a,
	const str& b
	)

	{
	return !( a == b );
	}

inline bool operator!=
	(
	const str& a,
	const char *b
	)

	{
	return !( a == b );
	}

inline bool operator!=
	(
	const char *a,
	const str& b
	)

	{
	return !( a == b );
	}

inline int str::icmpn
   (
   const char *text,
   int n
   ) const

   {
	assert( data );
	assert( text );

   return str::icmpn( data, text, n );
   }

inline int str::icmpn
   (
   const str& text,
   int n
   ) const

   {
	assert( data );
	assert( text.data );

   return str::icmpn( data, text.data, n );
   }

inline int str::icmp
   (
   const char *text
   ) const

   {
	assert( data );
	assert( text );

   return str::icmp( data, text );
   }

inline int str::icmp
   (
   const str& text
   ) const

   {
	assert( c_str () );
	assert( text.c_str () );

   return str::icmp( c_str () , text.c_str () );
   }

inline int str::cmp
   (
   const char *text
   ) const

   {
	assert( data );
	assert( text );

   return str::cmp( data, text );
   }

inline int str::cmp
   (
   const str& text
   ) const

   {
	assert( c_str () );
	assert( text.c_str () );

   return str::cmp( c_str () , text.c_str () );
   }

inline int str::cmpn
   (
   const char *text,
   int n
   ) const

   {
	assert( c_str () );
	assert( text );

   return str::cmpn( c_str () , text, n );
   }

inline int str::cmpn
   (
   const str& text,
   int n
   ) const

   {
	assert( c_str () );
	assert( text.c_str ()  );

   return str::cmpn( c_str () , text.c_str () , n );
   }

inline char *str::tolower
   (
   void
   )

   {
   assert( data );

   return str::tolower( data );
   }

inline char *str::toupper
   (
   void
   )

   {
   assert( data );

   return str::toupper( data );
   }

inline bool str::isNumeric
   (
   void
   ) const

   {
   assert( data );
   return str::isNumeric( data );
   }

inline str::operator const char *
   (
   void
   ) const

   {
   return c_str ();
   }

#endif
