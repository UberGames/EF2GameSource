//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/queue.h                                       $
// $Revision:: 4                                                              $
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
// Generic Queue object
//

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "class.h"

class QueueNode : public Class
	{
	public:
		void		 *data;
		QueueNode *next;

		QueueNode();
	};

inline QueueNode::QueueNode()
	{
	data = NULL;
	next = NULL;
	}

class Queue : public Class
	{
	private:
		QueueNode *head;
		QueueNode *tail;
      int count;

	public:
					Queue();
					~Queue();
		void		Clear( void	);
		qboolean Empty( void );
		void		Enqueue( void *data );
		void		*Dequeue( void );
      void     Remove( const void *data );
      qboolean Inqueue( const void *data );
      int      IndexOfObject( const void *data );
      int      GetCount();
	};

inline int Queue::GetCount
   (
   )
   {
   return count;
   }

inline qboolean Queue::Empty
	(
	void
	)

	{
	if ( head == NULL )
		{
		assert( !tail );
		return true;
		}

	assert( tail );
	return false;
	}

inline void Queue::Enqueue
	(
	void *data
	)

	{
	QueueNode *tmp;

	tmp = new QueueNode;
	if ( !tmp )
		{
		assert( NULL );
		gi.Error( ERR_DROP, "Queue::Enqueue : Out of memory" );
		}

	tmp->data = data;

	assert( !tmp->next );
	if ( !head )
		{
		assert( !tail );
		head = tmp;
		}
	else
		{
		assert( tail );
		tail->next = tmp;
		}
	tail = tmp;
   count++;
	}

inline void *Queue::Dequeue
	(
	void
	)

	{
	void *ptr;
	QueueNode *node;

	if ( !head )
		{
		assert( !tail );
		return NULL;
		}

	node = head;
	ptr = node->data;

	head = node->next;
	if ( head == NULL )
		{
		assert( tail == node );
		tail = NULL;
		}

	delete node;

   count--;
	return ptr;
	}

inline void Queue::Clear
	(
	void
	)

	{
	while( !Empty() )
		{
		Dequeue();
		}
   count = 0;
	}

inline Queue::Queue()
	{
	head = NULL;
	tail = NULL;
   count = 0;
	}

inline Queue::~Queue()
	{
	Clear();
	}

inline void Queue::Remove
   (
   const void *data
   )

   {
	QueueNode *node;
	QueueNode *prev;

	if ( !head )
		{
		assert( !tail );

      gi.WDPrintf( "Queue::Remove : Data not found in queue\n" );
		return;
		}

   for( prev = NULL, node = head; node != NULL; prev = node, node = node->next )
      {
      if ( node->data == data )
         {
         break;
         }
      }

   if ( !node )
      {
      gi.WDPrintf( "Queue::Remove : Data not found in queue\n" );
      }
   else
      {
      if ( !prev )
         {
         // at head
         assert( node == head );
         head = node->next;
	      if ( head == NULL )
		      {
		      assert( tail == node );
		      tail = NULL;
		      }
         }
      else
         {
         prev->next = node->next;
         if ( prev->next == NULL )
            {
            // at tail
            assert( tail == node );
            tail = prev;
            }
         }

	   delete node;
      count--;
      }
   }

inline qboolean Queue::Inqueue
   (
   const void *data
   )

   {
	QueueNode *node;

   for( node = head; node != NULL; node = node->next )
      {
      if ( node->data == data )
         {
         return true;
         }
      }

   return false;
   }

inline qboolean Queue::IndexOfObject
   (
   const void *data
   )

   {
   int         count=1;
	QueueNode   *node;

   for( node = head; node != NULL; node = node->next )
      {
      if ( node->data == data )
         {
         return count;
         }
      count++;
      }

   return 0;
   }

#endif /* queue.h */
