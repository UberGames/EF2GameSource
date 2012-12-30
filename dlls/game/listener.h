//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/listener.h                                    $
// $Revision:: 26                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
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

//
// this has to be placed in front of the __LISTENER_H__
// if it is not, listener.cpp will not compile
//
#if defined( GAME_DLL )
//
// game dll specific defines
//
#include "g_local.h"

#endif

#ifndef __LISTENER_H__
#define __LISTENER_H__

#if defined( GAME_DLL )
//
// game dll specific defines
//
#define EVENT_DebugPrintf gi.DebugPrintf
#define EVENT_DPrintf gi.DPrintf
#define EVENT_Printf gi.Printf
#define EVENT_WDPrintf gi.WDPrintf
#define EVENT_WPrintf gi.WPrintf
#define EVENT_time   level.time
#define EVENT_realtime gi.Milliseconds()
#define EVENT_Error gi.Error

#define EVENT_FILENAME "events.txt"

class Entity;
class CThread;
class Archiver;

#elif defined ( CGAME_DLL )
//
// cgame dll specific defines
//
#include <cgame/cg_local.h>
#include "vector.h"
#include "str.h"
#include <qcommon/qcommon.h>

#define EVENT_DebugPrintf cgi.DebugPrintf
#define EVENT_DPrintf cgi.DPrintf
#define EVENT_Printf cgi.Printf
#define EVENT_WDPrintf cgi.WDPrintf
#define EVENT_WPrintf cgi.WPrintf
#define EVENT_time   ( ( ( float )cg.time / 1000.0f ) )
#define EVENT_realtime   cgi.Milliseconds()
#define EVENT_Error cgi.Error

#define EVENT_FILENAME "cg_events.txt"

#elif defined ( UI_LIB )

#include <game/q_shared.h>
#include "vector.h"
#include "str.h"
#include <qcommon/qcommon.h>
#include "ui_local.h"

#define EVENT_DebugPrintf Com_DebugPrintf
#define EVENT_DPrintf Com_DPrintf
#define EVENT_Printf Com_Printf
#define EVENT_WDPrintf Com_WDPrintf
#define EVENT_WPrintf Com_WPrintf
//#define EVENT_time   ( ( ( float )cls.realtime / 1000.0f ) )
//#define EVENT_realtime   Sys_Milliseconds()
#define EVENT_time   ( ( ( float )cls.unscaledTime / 1000.0f ) )
#define EVENT_realtime   ( cls.unscaledTime )
#define EVENT_Error Com_Error

#define EVENT_FILENAME "ui_events.txt"

#else

//
// client specific defines
//
#include <game/q_shared.h>
#include "vector.h"
#include "str.h"
#include <qcommon/qcommon.h>

#define EVENT_DebugPrintf Com_DebugPrintf
#define EVENT_DPrintf Com_DPrintf
#define EVENT_Printf Com_Printf
#define EVENT_WDPrintf Com_WDPrintf
#define EVENT_WPrintf Com_WPrintf

/**
 * EVENT_time looks like it should be an obsolute time. Possibly the time
 * from when the game started or when the level started. 
 * EVENT_time used to be set to cls.unscaledTime. The problem is that on a dedicated linux
 * server, cls is not defined nor is it needed. So Sys_Milliseconds replaces this, but I am not 
 * sure if this is the correct time to use since Sys_Milliseconds returns the real time, not the 
 * absolute time.
 */
#define EVENT_time   ( ( ( float )Sys_Milliseconds() / 1000.0f ) )
#define EVENT_realtime Sys_Milliseconds()
#define EVENT_Error Com_Error

#define EVENT_FILENAME "cl_events.txt"
#endif

#include "class.h"
#include "container.h"

typedef enum
	{
	EV_FROM_CODE,
	EV_FROM_CONSOLE,
	EV_FROM_SCRIPT,
   EV_FROM_ANIMATION
	} eventsource_t;

// Posted Event Flags
#define EVENT_LEGS_ANIM    (1<<0)      // this event is associated with an animation for the legs
#define EVENT_TORSO_ANIM   (1<<1)      // this event is associated with an animation for the torso
#define EVENT_DIALOG_ANIM  (1<<2)      // this event is associated with an animation for dialog lip syncing


// Event flags
#define EV_CONSOLE	   (1<<0)		// Allow entry from console
#define EV_CHEAT		   (1<<1)		// Only allow entry from console if cheats are enabled
#define EV_CODEONLY	   (1<<2)		// Hide from eventlist
#define EV_CACHE		   (1<<3)		// This event is used to cache data in
#define EV_TIKIONLY		(1<<4)		// This command only applies to TIKI files
#define EV_SCRIPTONLY	(1<<5)		// This command only applies to SCRIPT files

#define EV_DEFAULT	   -1    		// default flags

#define INUSE_BITS      2
#define MAX_EVENT_USE	( ( 1 << INUSE_BITS ) - 1 )

typedef enum
   {
   IS_STRING,
   IS_VECTOR,
   IS_BOOLEAN,
   IS_INTEGER,
   IS_FLOAT,
   IS_ENTITY
   } vartype;

#define DIRTY_STRING    ( 1 << 0 )
#define DIRTY_VECTOR    ( 1 << 1 )
#define DIRTY_INTEGER   ( 1 << 2 )
#define DIRTY_FLOAT     ( 1 << 3 )

#define DIRTY_ALL       ( 0x7fff )

class EventVar : public Class
   {
   private:
      short                   type;
      short                   dirtyFlags;
      str                     stringValue;
      Vector                  vectorValue;
      int                     intValue;
      float                   floatValue;

   public:

								EventVar()
                           {
                           type        = IS_INTEGER;
                           dirtyFlags  = DIRTY_ALL;
                           intValue    = 0;
                           floatValue  = 0;
                           };

                        EventVar( const EventVar &ev )
                           {
                           type        = ev.type;
                           dirtyFlags  = ev.dirtyFlags;
                           intValue    = ev.intValue;
                           floatValue  = ev.floatValue;
                           };

                        EventVar( const char *text );
		                  EventVar( const str &text );

		                  EventVar( int val )
                           {
                           type        = IS_INTEGER;
                           dirtyFlags  = DIRTY_ALL & ~DIRTY_INTEGER;
                           intValue    = val;
                           floatValue  = 0;
                           };

		                  EventVar( float val )
                           {
                           type        = IS_FLOAT;
                           dirtyFlags  = DIRTY_ALL & ~DIRTY_FLOAT;
									intValue    = 0;
                           floatValue  = val;
                           };

		                  EventVar( const Vector &vec )
                           {
                           type        = IS_VECTOR;
                           dirtyFlags  = DIRTY_ALL & ~DIRTY_VECTOR;
                           vectorValue = vec;
									intValue    = 0;
                           floatValue  = 0;
                           };

#ifdef GAME_DLL
		                  EventVar( const Entity *ent );
#endif

		const char			*GetToken( Event &ev );
		const char			*GetString( Event &ev );
		qboolean			GetBoolean( Event &ev );
		int					GetInteger( Event &ev );
		float				GetFloat( Event &ev );
		Vector				GetVector( Event &ev );
#ifdef GAME_DLL
		Entity				*GetEntity( Event &ev );
		qboolean			 IsEntity( Event &ev );
#endif

		void				SetString( const char *text );

      qboolean          IsVector( Event &ev );
      qboolean          IsNumeric( Event &ev );

#ifdef GAME_DLL
      void              Archive( Archiver &arc );
#endif
   };


class EventArgDef : public Class
   {
   private:
      int                     type;
      str                     name;
      float                   minRange[ 3 ];
      qboolean                minRangeDefault[ 3 ];
      float                   maxRange[ 3 ];
      qboolean                maxRangeDefault[ 3 ];
      qboolean                optional;
   public:

		EventArgDef()
         {
         type        = IS_INTEGER;
         //name        = "undefined";
         optional    = false;
         };
		void              Setup( const char * eventName, const char *argName, const char *argType, const char *argRange );
		void              PrintArgument( FILE *event_file = NULL );
      void              PrintRange( FILE *event_file = NULL );
      int               getType( void );
      const char        *getName( void );
      qboolean          isOptional( void );
		
		float GetMinRange(int index)
			{
			if ( index < 3 )
				return minRange[index];
			return 0.0;
			}

		qboolean GetMinRangeDefault(int index)
			{
			if ( index < 3 )
				return minRangeDefault[index];
			return false;
			}

		float GetMaxRange(int index)
			{
			if ( index < 3 )
				return maxRange[index];
			return 0.0;
			}

		qboolean GetMaxRangeDefault(int index)
			{
			if ( index < 3 )
				return maxRangeDefault[index];
			return false;
			}

#ifdef GAME_DLL
      void              Archive( Archiver &arc );
#endif
   };

inline int EventArgDef::getType
   (
   void
   )

   {
   return type;
   }

inline const char *EventArgDef::getName
   (
   void
   )

   {
   return name.c_str();
   }

inline qboolean EventArgDef::isOptional
   (
   void
   )

   {
   return optional;
   }

#ifndef GAME_DLL
extern "C"
   {
   // interface functions
   void	        L_ProcessPendingEvents( void );
   void          L_ClearEventList( void );
   void          L_InitEvents( void );
   void          L_ShutdownEvents( void );
   }
#endif


class	Listener;
typedef SafePtr<Listener> ListenerPtr;


class Event : public Class
	{
	private:
      friend class	      Listener;

		typedef struct EventInfo
			{
			unsigned			   inuse			: INUSE_BITS;	// must change MAX_EVENT_USE to reflect maximum number stored here
			unsigned			   source		: 2;
			unsigned			   flags			: 9;
			unsigned			   linenumber	: 19;		      // linenumber does double duty in the case of the console commands
         } EventInfo_t;

		int					   eventnum;
		EventInfo			   info;
      const char           *name;
      const char           *formatspec;
      const char           *argument_names;
      const char           *documentation;
		Container<EventVar>  *data;
		Container<EventArgDef> *definition;
		EventArgDef          *returntype;
		int					   threadnum;
      short                anim_number;
      short                anim_frame;
      SafePtr<Listener>    m_sourceobject;

	   //Listener             *obj;
	   float		            time;
      int                  flags;

		friend class			Level;

		static void			   initCommandList( void );
      static void          InitializeEventLists( void );
      static void          ShutdownEventLists( void );
      static void          InitializeDocumentation( void );
      static void          ShutdownDocumentation( void );

      static int           numfinds;
      static int           numfirstsearch;
      static int           numcompares;
      static int           lastevent;
      static bool          EventSystemStarted;

		friend void	         L_ProcessPendingEvents( void );
		friend void          L_ClearEventList( void );
		friend void          L_InitEvents( void );
		friend void          L_ShutdownEvents( void );

#ifdef GAME_DLL
      friend void          L_ArchiveEvents( Archiver &arc );
      friend void          L_UnarchiveEvents( Archiver &arc );

#endif

		static Container<str *>	*commandList;
		static Container<int>	*flagList;
		static Container<int>	*sortedList;
		static Container<Event *> *eventDefList;
		static qboolean			dirtylist;

		static int			   compareEvents( const void *arg1, const void *arg2 );
		static void			   SortEventList( void );
		static void			   PrintEvent( Event *ev );

	public:
		CLASS_PROTOTYPE( Event );

      Event             *next;               // next event in the list, used for event recycling
      Event             *prev;               // previous event int the list, used for event recycling

		void * operator	new( size_t );
		void operator		delete( void * );

		static int			   FindEvent( const char *name );
		static int			   FindEvent( const str &name );

		static int			NumEventCommands( void );
		static void       ListCommands( const char *mask = NULL );
      static void       ListDocumentation( const char *mask, qboolean print_to_file = false );
		static void       PendingEvents( const char *mask = NULL );
		static Event      *GetEventDef( int i );
      static int        NumEventDefs( void );
      static void       PrintEventDocumentation( Event * event, FILE *event_file = NULL, qboolean html = false, int typeFlag = 0 );
      static int        MapSortedEventIndex( int index );

								Event();
								Event( const Event &ev );
								Event( Event *ev );
								Event( int num );
								Event
                           (
                           const char *command,
                           int flags,
                           const char *formatspec,
                           const char *argument_names,
                           const char *documentation
                           );
								Event( const char *command );
								Event( const str &command );
								~Event();

      void              SetupDocumentation( void );
      void              DeleteDocumentation( void );
      void              PrintDocumentation( FILE *event_file = NULL, qboolean html = false );

		int               getNumArgDefs( void );
      EventArgDef       *getArgDef( int num );
      EventArgDef       *getReturnType( void );

		const char			*getName( void );

		void					SetSource( eventsource_t source );
		void					SetLineNumber( unsigned int linenumber );
      void              SetSourceObject( Listener *source );
      Listener          *GetSourceObject( void );
      SafePtr<Listener> *GetSourceObjectPointer( void );

		eventsource_t		GetSource( void );
		int					GetLineNumber( void );
      int               GetAnimationNumber( void );
      int               GetAnimationFrame( void );
      void              SetAnimationNumber( int anim );
      void              SetAnimationFrame( int frame );

		int         		GetFlags( void );

		void					Error( const char *fmt, ... );

		static Event		Find( const char *command );
		static qboolean	Exists( const char *command );
		static Event		Find( const str &command );

		Event&				printInfo(void);

      friend bool       operator==( const Event &a, const Event &b );
      friend bool       operator!=( const Event &a, const Event &b );
		void		         operator=( const Event &ev );

		operator				int();
		operator				const char *();

		int					NumArgs( void );

		qboolean				IsVectorAt( int pos );
		qboolean				IsEntityAt( int pos );
		qboolean				IsNumericAt( int pos );

		const char			*GetToken( int pos );
		const char			*GetString( int pos );
		int					GetInteger( int pos );
		float					GetFloat( int pos );
		Vector				GetVector( int pos );
      bool              GetBoolean( int pos );

		void					SetToken( int pos,		const char *text );
		void					SetString( int pos,		const str &text );
		void					SetInteger( int pos,	int value );
		void					SetFloat( int pos,		float value );
		void					SetVector( int pos,		const Vector &vector );
		void					SetBoolean( int pos,	bool value );

		void					AddToken( const char *text );
		void					AddTokens( int argc, const char **argv );
		void					AddString( const char *text );
		void					AddString( const str &text );
		void					AddInteger( int val );
		void					AddFloat( float val );
		void					AddVector( const Vector &vec );

		const char			*GetDocumentation() { return documentation; }

#ifdef GAME_DLL
		void              ReturnString( const char *text );
      void              ReturnFloat( float value );
      void              ReturnInteger( int value );
      void              ReturnVector( const Vector &vec );
      void              ReturnEntity( Entity *ent );

		void					AddEntity( Entity *ent );
		Entity				*GetEntity( int pos );
		
		void					SetThread( CThread *thread );
		CThread				*GetThread( void );
      void              SetConsoleEdict( const gentity_t *consoleedict );
      gentity_t         *GetConsoleEdict( void );

      virtual void      Archive( Archiver &arc );
#endif
	};

extern Event NullEvent;
extern Event EV_Remove;

class Listener;

class Listener : public Class
	{
	private:
		void							ScriptRemove( Event *e );

	protected:
		qboolean						CheckEventFlags( Event *event );

	public:
		CLASS_PROTOTYPE( Listener );

										~Listener();
		void							Remove( Event *e );
      qboolean						ValidEvent( int ev );
		qboolean						ValidEvent( Event &e );
		qboolean						ValidEvent( const char *name );
		qboolean						EventPending( Event &ev );
		qboolean						ProcessEvent( Event *event );
		qboolean						ProcessEvent( const Event &event );
		void							PostEvent( Event *event, float time, int flags = 0 );
		void							PostEvent( const Event &event, float time, int flags = 0 );
		qboolean						PostponeEvent( Event &event, float time );
		qboolean						PostponeEvent( Event *event, float time );
		bool							CancelEventsOfType( Event *event );
		bool							CancelEventsOfType( Event &event );
		void							CancelFlaggedEvents( int flags );
		void							CancelPendingEvents( void );
		qboolean						ProcessPendingEvents( void );
	};

inline void Event::SetSourceObject
   (
   Listener *source
   )

   {
   m_sourceobject = source;
   }

inline Listener *Event::GetSourceObject
   (
   void
   )

   {
   return m_sourceobject;
   }

inline SafePtr<Listener> *Event::GetSourceObjectPointer
   (
   void
   )

   {
   return &m_sourceobject;
   }

inline qboolean Event::Exists
	(
	const char *command
	)

	{
	int num;
	str c;

	if ( !commandList )
		{
		initCommandList();
		}

	c = command;
	num = FindEvent( c );
	if ( num )
		{
      return true;
		}

	return false;
	}


inline Event Event::Find
	(
	const char *command
	)

	{
	int num;
	str c;

	if ( !commandList )
		{
		initCommandList();
		}

	c = command;
	num = FindEvent( c );
	if ( num )
		{
      Event ev( num );
		return ev;
		}

	return NullEvent;
	}

inline Event Event::Find
	(
	const str &command
	)

	{
	int num;

	if ( !commandList )
		{
		initCommandList();
		}

	num = FindEvent( command );
	if ( num )
		{
      Event ev( num );
		return ev;
		}

	return NullEvent;
	}

inline int Event::getNumArgDefs
   (
   void
   )

   {
   if ( definition )
      {
      return definition->NumObjects();
      }

   return 0;
   }

inline EventArgDef *Event::getArgDef
   (
   int num
   )

   {
   if ( !definition )
      {
      return NULL;
      }

   return &definition->ObjectAt( num );
   }

inline EventArgDef *Event::getReturnType
   (
   void
   )

   {
   return returntype;
   }

inline void Event::SetSource
	(
	eventsource_t source
	)

	{
	info.source = ( unsigned )source;
	}

inline void Event::SetLineNumber
	(
	unsigned int linenumber
	)

	{
	info.linenumber = linenumber;
	}

inline eventsource_t Event::GetSource
	(
	void
	)

	{
	return ( eventsource_t )info.source;
	}

inline int Event::GetAnimationNumber
   (
   void
   )

   {
   return anim_number;
   }

inline int Event::GetAnimationFrame
   (
   void
   )

   {
   return anim_frame;
   }

inline void Event::SetAnimationNumber
   (
   int anim
   )

   {
   anim_number = anim;
   }

inline void Event::SetAnimationFrame
   (
   int frame
   )

   {
   anim_frame = frame;
   }

inline int Event::GetLineNumber
	(
	void
	)

	{
	// linenumber does double duty in the case of the console commands
	if ( info.source == EV_FROM_SCRIPT )
		{
		return info.linenumber;
		}

	return 0;
	}

inline int Event::GetFlags
	(
	void
	)

	{
	return info.flags;
	}

inline const char *Event::getName
	(
	void
	)

	{
   assert( name || !eventnum );

   if ( !name )
		{
		return "NULL";
		}

   return name;
	}

inline Event& Event::printInfo
	(
	void
	)

	{
   EVENT_DPrintf( "event '%s' is number %d\n", getName(), eventnum );

	return *this;
	}

inline bool operator==
   (
   const Event &a,
   const Event &b
   )

   {
   return a.eventnum == b.eventnum;
   }

inline bool operator!=
   (
   const Event &a,
   const Event &b
   )

   {
   return a.eventnum != b.eventnum;
   }

inline void Event::operator=
   (
   const Event &ev
   )
   {
   eventnum = ev.eventnum;
   info = ev.info;
   if ( ev.data )
      {
      int i;

      data = new Container<EventVar>;
      data->Resize( ev.data->NumObjects() );
      for( i = 1; i < ev.data->NumObjects(); i++ )
         {
         data->AddObject( ev.data->ObjectAt( i ) );
         }
      }
   name = ev.name;
   definition = NULL;
	threadnum = ev.threadnum;
   anim_number = ev.anim_number;
   anim_frame = ev.anim_frame;
   m_sourceobject = ev.m_sourceobject;
   time = ev.time;
   flags = ev.flags;
   }


inline Event::operator int()
	{
	return eventnum;
	}

inline Event::operator const char *()
	{
	return getName();
	}

inline int Event::NumArgs
	(
	void
	)

	{
	if ( !data )
		{
		return 0;
		}

	return ( data->NumObjects() );
	}

#ifdef GAME_DLL
inline void Event::AddEntity
	(
	Entity *ent
	)

	{
	if ( !data )
		{
		data = new Container<EventVar>;
		data->Resize( 1 );
		}


   EventVar var( ent );
   data->AddObject( var );
	}
#endif

inline void Event::AddToken
	(
	const char *text
	)

	{
	if ( !data )
		{
		data = new Container<EventVar>;
		data->Resize( 1 );
		}

   EventVar var( text );
   data->AddObject( var );
	}

inline void Event::AddTokens
	(
	int argc,
	const char **argv
	)

	{
	int i;

	if ( !data )
		{
		data = new Container<EventVar>;
		data->Resize( argc );
		}

	for( i = 0; i < argc; i++ )
		{
		assert( argv[ i ] );
      EventVar var( argv[ i ] );
      data->AddObject( var );
		}
	}

inline void Event::AddString
	(
	const char *text
	)

	{
	if ( !data )
		{
		data = new Container<EventVar>;
		data->Resize( 1 );
		}

   EventVar var( text );
   data->AddObject( var );
	}

inline void Event::AddString
	(
	const str &text
	)

	{
	if ( !data )
		{
		data = new Container<EventVar>;
		data->Resize( 1 );
		}

   EventVar var( text );
   data->AddObject( var );
	}

inline void Event::AddInteger
	(
	int val
	)

	{
	if ( !data )
		{
		data = new Container<EventVar>;
		data->Resize( 1 );
		}

   EventVar var( val );
   data->AddObject( var );
	}

inline void Event::AddFloat
	(
	float val
	)

	{
	if ( !data )
		{
		data = new Container<EventVar>;
		data->Resize( 1 );
		}

   EventVar var( val );
   data->AddObject( var );
	}

inline void Event::AddVector
	(
	const Vector &vec
	)

	{
	if ( !data )
		{
		data = new Container<EventVar>;
		data->Resize( 1 );
		}

   EventVar var( vec );
   data->AddObject( var );
	}

inline const char *Event::GetToken
	(
	int pos
	)

	{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
		{
		Error( "Index %d out of range.", pos );
		return "";
		}

	return data->ObjectAt( pos ).GetToken( *this );
	}

inline const char *Event::GetString
	(
	int pos
	)

	{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
		{
		Error( "Index %d out of range.", pos );
		return "";
		}

   return data->ObjectAt( pos ).GetString( *this );
	}

inline int Event::GetInteger
	(
	int pos
	)

	{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
		{
		Error( "Index %d out of range.", pos );
		return 0;
		}

   return data->ObjectAt( pos ).GetInteger( *this );
	}

inline float Event::GetFloat
	(
	int pos
	)

	{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
		{
		Error( "Index %d out of range.", pos );
		return 0;
		}

   return data->ObjectAt( pos ).GetFloat( *this );
	}

inline Vector Event::GetVector
	(
	int pos
	)

	{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
		{
		Error( "Index %d out of range.", pos );
		return vec_zero;
		}

   return data->ObjectAt( pos ).GetVector( *this );
	}

inline bool Event::GetBoolean
   (
   int pos
   )

   {
   int val;

   val = this->GetInteger( pos );

   return ( val != 0 ) ? true : false;
   }

#ifdef GAME_DLL

inline Entity *Event::GetEntity
	(
	int pos
	)

	{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
		{
		Error( "Index %d out of range.", pos );
		return NULL;
		}

   return data->ObjectAt( pos ).GetEntity( *this );
	}

#endif


//===============================================================
// Name:		SetToken
// Class:		Event
//
// Description: Replaces the argument at the specified position
//				with a new string argument.
// 
// Parameters:	int			-- the position to replace ( 1-based )
//				const str&	-- the new string argument
//
// Returns:		None
// 
//===============================================================
inline void Event::SetToken
(
	int pos,
	const char *token
)
{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
	{
		Error( "Index %d out of range.", pos );
		return ;
	}

   EventVar var( token );
   data->SetObjectAt( pos, var );
}


//===============================================================
// Name:		SetString
// Class:		Event
//
// Description: Replaces the argument at the specified position
//				with a new string argument.
// 
// Parameters:	int			-- the position to replace ( 1-based )
//				const str&	-- the new string argument
//
// Returns:		None
// 
//===============================================================
inline void Event::SetString
(
	int pos,
	const str &text
)
{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
	{
		Error( "Index %d out of range.", pos );
		return ;
	}

   EventVar var( text );
   data->SetObjectAt( pos, var );
}

//===============================================================
// Name:		SetInteger
// Class:		Event
//
// Description: Replaces the argument at the specified position
//				with a new integer argument.
// 
// Parameters:	int	pos		-- the position to replace ( 1-based )
//				int	value	-- the new integer argument
//
// Returns:		None
// 
//===============================================================
inline void Event::SetInteger
(
	int pos,
	int value
)
{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
	{
		Error( "Index %d out of range.", pos );
		return ;
	}

   EventVar var( value );
   data->SetObjectAt( pos, var );
}


//===============================================================
// Name:		SetFloat
// Class:		Event
//
// Description: Replaces the argument at the specified position
//				with a new string argument.
// 
// Parameters:	int			-- the position to replace ( 1-based )
//				float		-- the new float argument
//
// Returns:		None
// 
//===============================================================
inline void Event::SetFloat
(
	int		pos,
	float	value
)
{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
	{
		Error( "Index %d out of range.", pos );
		return ;
	}

   EventVar var( value );
   data->SetObjectAt( pos, var );
}

//===============================================================
// Name:		SetVector
// Class:		Event
//
// Description: Replaces the argument at the specified position
//				with a new vector argument.
// 
// Parameters:	int				-- the position to replace ( 1-based )
//				const Vector&	-- the new vector argument
//
// Returns:		None
// 
//===============================================================
inline void Event::SetVector
(
	int pos,
	const Vector &vector
)
{
	if ( !data || ( pos < 1 ) || ( data->NumObjects() < pos ) )
	{
		Error( "Index %d out of range.", pos );
		return ;
	}

   EventVar var( vector );
   data->SetObjectAt( pos, var );
}


inline qboolean Listener::ProcessEvent
	(
	const Event &event
	)

	{
	Event *ev;

	ev = new Event( event );
	return ProcessEvent( ev );
	}

inline void Listener::PostEvent
	(
	const Event &event,
	float time,
   int flags
	)

	{
	Event *ev;

	ev = new Event( event );
	PostEvent( ev, time, flags );
	}

inline qboolean Listener::PostponeEvent
	(
	Event *event,
	float time
	)

	{
	return PostponeEvent( *event, time );
	}

inline bool Listener::CancelEventsOfType
	(
	Event &event
	)

	{
	return CancelEventsOfType( &event );
	}

inline qboolean Listener::ValidEvent
	(
	int ev
	)

	{
	ClassDef	*c;

	c = this->classinfo();
   assert( ( ev >= 0 ) && ( ev < c->numEvents ) );
	if ( ( ev < 0 ) || ( ev >= c->numEvents ) )
		{
		return false;
		}

	return ( c->responseLookup[ ev ] != NULL );
   }

inline qboolean Listener::ValidEvent
	(
	Event &e
	)

	{
	ClassDef	*c;
	int		ev;

	ev = ( int )e;

	c = this->classinfo();
   assert( ( ev >= 0 ) && ( ev < c->numEvents ) );
	if ( ( ev < 0 ) || ( ev >= c->numEvents ) )
		{
      warning( "ValidEvent", "Event '%s' out of response range for class '%s'.  "
         "Event probably invalid or allocated late.\n", e.getName(), getClassname() );
		return false;
		}

	return ( c->responseLookup[ ev ] != NULL );
   }

inline qboolean Listener::ValidEvent
	(
	const char *name
	)

	{
	ClassDef	*c;
	int		ev;

   ev = Event::FindEvent( name );

	c = this->classinfo();
   assert( ( ev >= 0 ) && ( ev < c->numEvents ) );
	if ( ( ev < 0 ) || ( ev >= c->numEvents ) )
		{
      warning( "ValidEvent", "Event '%s' out of response range for class '%s'.  "
         "Event probably invalid or allocated late.\n", name, getClassname() );
		return false;
		}

	return ( c->responseLookup[ ev ] != NULL );
	}

#endif
