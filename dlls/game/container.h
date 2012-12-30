//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/container.h                                   $
// $Revision:: 12                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Base class for a dynamic array.  Allows adding, removing, index of,
// and finding of entries with specified value.  NOTE: indices in container
// are 1 based, not 0.  This means that loops must check from 1 up to and including
// NumObjects()  (ei. for( i = 1; i <= list.NumObjects(); i++ ) ).
//
// FIXME: Someday make this 0 based and update all code to match.
//
// WARNING: This file is shared between game, cgame and possibly the user interface.
// It is instanced in each one of these directories because of the way that SourceSafe works.
//

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <qcommon/platform.h>

#if defined( GAME_DLL )
//
// game dll specific defines
//
#include "g_local.h"

#define CONTAINER_Error gi.Error
#define CONTAINER_DPrintf gi.DPrintf
#define CONTAINER_WDPrintf gi.WDPrintf

#elif defined ( CGAME_DLL )
//
// cgame dll specific defines
//
#define CONTAINER_Error cgi.Error
#define CONTAINER_DPrintf cgi.DPrintf
#define CONTAINER_WDPrintf cgi.WDPrintf

#else

//
// client specific defines
//
#define CONTAINER_Error Com_Error
#define CONTAINER_DPrintf Com_DPrintf
#define CONTAINER_WDPrintf Com_WDPrintf
#endif

#include <stdlib.h>

class Archiver;
template< class Type >
class Container
	{
	private:
		Type	*objlist;
		int	numobjects;
		int	maxobjects;

	public:
					Container();
					~Container<Type>();

		void		FreeObjectList( void );
		void		ClearObjectList( void );
		int			NumObjects( void ) const;
		void		Resize( int maxelements );
		void		SetObjectAt( int index, const Type& obj );
		int			AddObject( const Type& obj );
		int			AddUniqueObject( const Type& obj );
		void  		AddObjectAt( int index, const Type& obj );
		int			IndexOfObject( const Type& obj );
		int			IndexOfObject( const Type& obj ) const;
		qboolean	ObjectInList( const Type& obj );
		Type&		ObjectAt( const int index ) const;
		Type		*AddressOfObjectAt( int index );
		void		InsertObjectAt( int index, const Type& obj );
		void		RemoveObjectAt( int index );
		void		RemoveObject( const Type& obj );
		void		Sort( int ( __cdecl *compare )( const void *elem1, const void *elem2 ) );
		Type&		operator[]( const int index ) const;
#if defined( GAME_DLL )
      void     Archive( Archiver &arc );
#endif
	};

template< class Type >
Container<Type>::Container()
	{
   objlist = NULL;
	FreeObjectList();
	}

template< class Type >
Container<Type>::~Container<Type>()
	{
	FreeObjectList();
	}

template< class Type >
void Container<Type>::FreeObjectList
	(
	void
	)

	{
	if ( objlist )
		{
		delete[] objlist;
		}
	objlist = NULL;
	numobjects = 0;
	maxobjects = 0;
	}

template< class Type >
void Container<Type>::ClearObjectList
	(
	void
	)

	{
	// only delete the list if we have objects in it
	if ( objlist && numobjects )
		{
		delete[] objlist;

		if ( maxobjects == 0 )
			{
			objlist = NULL;
			return;
			}

		objlist = new Type[ maxobjects ];
		numobjects = 0;
		}
	}

template< class Type >
int Container<Type>::NumObjects
	(
	void
	) const

	{
	return numobjects;
	}

template< class Type >
void Container<Type>::Resize
	(
	int maxelements
	)

	{
	Type *temp;
	int i;

   assert( maxelements >= 0 );

   if ( maxelements <= 0 )
      {
      FreeObjectList();
      return;
      }

	if ( !objlist )
		{
		maxobjects = maxelements;
		objlist = new Type[ maxobjects ];
		}
	else
		{
		temp = objlist;
		maxobjects = maxelements;
		if ( maxobjects < numobjects )
			{
			maxobjects = numobjects;
			}

		objlist = new Type[ maxobjects ];
		for( i = 0; i < numobjects; i++ )
			{
			objlist[ i ] = temp[ i ];
			}
		delete[] temp;
		}
	}

template< class Type >
void Container<Type>::SetObjectAt
	(
	int index,
	const Type& obj
	)

	{
	if ( !objlist )
		return;

	if ( ( index <= 0 ) || ( index > numobjects ) )
		{
		CONTAINER_Error( ERR_DROP, "Container::SetObjectAt : index out of range" );
		}

	objlist[ index - 1 ] = obj;
	}

template< class Type >
int Container<Type>::AddObject
	(
	const Type& obj
	)

	{
	if ( !objlist )
		{
		Resize( 10 );
		}

	if ( numobjects == maxobjects )
		{
		Resize( maxobjects * 2 );
		}

	objlist[ numobjects ] = obj;
	numobjects++;

	return numobjects;
	}

template< class Type >
int Container<Type>::AddUniqueObject
	(
	const Type& obj
	)

	{
   int index;

   index = IndexOfObject( obj );
   if ( !index )
      index = AddObject( obj );
   return index;
	}

template< class Type >
void Container<Type>::AddObjectAt
	(
	int index,
	const Type& obj
	)

	{
   //
   // this should only be used when reconstructing a list that has to be identical to the original
   //
   if ( index > maxobjects )
      {
      Resize( index );
      }
   if ( index > numobjects )
      {
      numobjects = index;
      }
   SetObjectAt( index, obj );
	}

template< class Type >
int Container<Type>::IndexOfObject
	(
	const Type& obj
	)

	{
	int i;

	if ( !objlist )
		return 0;

	for( i = 0; i < numobjects; i++ )
		{
		if ( objlist[ i ] == obj )
			{
			return i + 1;
			}
		}

	return 0;
	}

template< class Type >
int Container<Type>::IndexOfObject
	(
	const Type& obj
	) const

	{
	int i;

	if ( !objlist )
		return 0;

	for( i = 0; i < numobjects; i++ )
		{
		if ( objlist[ i ] == obj )
			{
			return i + 1;
			}
		}

	return 0;
	}

template< class Type >
qboolean Container<Type>::ObjectInList
	(
	const Type& obj
	)

	{
	if ( !IndexOfObject( obj ) )
		{
		return false;
		}

	return true;
	}

template< class Type >
Type& Container<Type>::ObjectAt
	(
	int index
	) const

	{
	if ( ( index <= 0 ) || ( index > numobjects ) )
		{
		CONTAINER_Error( ERR_DROP, "Container::ObjectAt : index out of range" );
		}

	return objlist[ index - 1 ];
	}

template< class Type >
Type * Container<Type>::AddressOfObjectAt
	(
	int index
	)

	{
   //
   // this should only be used when reconstructing a list that has to be identical to the original
   //
   if ( index > maxobjects )
      {
      CONTAINER_Error( ERR_DROP, "Container::AddressOfObjectAt : index is greater than maxobjects" );
      }
   if ( index > numobjects )
      {
      numobjects = index;
      }
	return &objlist[ index - 1 ];
	}

template< class Type >
void Container<Type>::InsertObjectAt
	(
	int index,
	const Type& obj
	)

	{
	if ( ( index <= 0 ) || ( index > numobjects + 1 ) )
		{
		CONTAINER_Error( ERR_DROP, "Container::RemoveObjectAt : index out of range" );
		return;
		}
	
	numobjects++;
	int arrayIndex=index-1;

	if (numobjects > maxobjects)
		{
		maxobjects = numobjects;
		if ( !objlist )
			{
			objlist = new Type[ maxobjects ];
			assert(arrayIndex < maxobjects);
			assert(arrayIndex >= 0);
			objlist[ arrayIndex ] = obj;
			return;
			}
		else
			{
			Type *temp = objlist;
			if ( maxobjects < numobjects )
				{
				maxobjects = numobjects;
				}

			objlist = new Type[ maxobjects ];
			int i;
			for( i = arrayIndex - 1; i >= 0 ; i-- )
				{
				assert(i < maxobjects);
				assert(i >= 0);
				objlist[ i ] = temp[ i ];
				}

			assert(arrayIndex < maxobjects);
			assert(arrayIndex >= 0);
			objlist[ arrayIndex ] = obj;
			for( i = numobjects - 1; i > arrayIndex; i-- )
				{
				assert(i < maxobjects);
				assert(i >= 0);
				objlist[ i ] = temp[ i-1 ];
				}
			delete[] temp;
			}
		}
	else
		{
			for( int i = numobjects - 1; i > arrayIndex; i-- )
			{
			assert(i < maxobjects);
			assert(i >= 0);
			objlist[ i ] = objlist[ i - 1 ];
			}
		objlist[ arrayIndex ] = obj;
		}
	}

template< class Type >
void Container<Type>::RemoveObjectAt
	(
	int index
	)

	{
	int i;

	if ( !objlist )
		{
      CONTAINER_WDPrintf( "Container::RemoveObjectAt : Empty list\n" );
		return;
		}

	if ( ( index <= 0 ) || ( index > numobjects ) )
		{
		CONTAINER_Error( ERR_DROP, "Container::RemoveObjectAt : index out of range" );
		return;
		}

	i = index - 1;
	numobjects--;
	for( i = index - 1; i < numobjects; i++ )
		{
		objlist[ i ] = objlist[ i + 1 ];
		}
	}

template< class Type >
void Container<Type>::RemoveObject
	(
	const Type& obj
	)

	{
	int index;

	index = IndexOfObject( obj );
	if ( !index )
		{
      CONTAINER_WDPrintf( "Container::RemoveObject : Object not in list\n" );
		return;
		}

	RemoveObjectAt( index );
	}

template< class Type >
void Container<Type>::Sort
	(
	int ( __cdecl *compare )( const void *elem1, const void *elem2 )
	)

	{
	if ( !objlist )
		{
      CONTAINER_WDPrintf( "Container::Sort : Empty list\n" );
		return;
		}

	qsort( ( void * )objlist, ( size_t )numobjects, sizeof( Type ), compare );
	}

template< class Type >
Type& Container<Type>::operator[]( const int index ) const
{
	return ObjectAt( index + 1 );
}

#if 0
#if defined( GAME_DLL )

#include "str.h"
void Container<str>::Archive
	(
   Archiver &arc
	)
	{
   int i, num;

   if ( arc.Loading() )
      {
      ClearObjectList();
      arc.ArchiveInteger( &num );
      Resize( num );
      }
   else
      {
      num = numobjects;
      arc.ArchiveInteger( &num );
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveString( AddressOfObjectAt( i ) );
      }
	}

#include "vector.h"
void Container<Vector>::Archive
	(
   Archiver &arc
	)
	{
   int i, num;

   if ( arc.Loading() )
      {
      ClearObjectList();
      arc.ArchiveInteger( &num );
      Resize( num );
      }
   else
      {
      num = numobjects;
      arc.ArchiveInteger( &num );
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveVector( AddressOfObjectAt( i ) );
      }
	}

void Container<int>::Archive
	(
   Archiver &arc
	)
	{
   int i, num;

   if ( arc.Loading() )
      {
      ClearObjectList();
      arc.ArchiveInteger( &num );
      Resize( num );
      }
   else
      {
      num = numobjects;
      arc.ArchiveInteger( &num );
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveInteger( AddressOfObjectAt( i ) );
      }
	}

void Container<float>::Archive
	(
   Archiver &arc
	)
	{
   int i, num;

   if ( arc.Loading() )
      {
      ClearObjectList();
      arc.ArchiveInteger( &num );
      Resize( num );
      }
   else
      {
      num = numobjects;
      arc.ArchiveInteger( &num );
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveFloat( AddressOfObjectAt( i ) );
      }
	}

#endif
#endif

#endif /* container.h */
