//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/program.h                                      $
// $Revision:: 11                                                             $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// 
//
// DESCRIPTION:
// 

#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "g_local.h"

class Lexer;

#define	MAX_STRINGS		4096
//#define	MAX_STATEMENTS	65536
#define	MAX_STATEMENTS	16384
#define	MAX_FUNCTIONS	2560

#define	MAX_PARMS	   16
#define	MAX_NAME		   64		// chars long
#define	MAX_REGS		   16384

#define	OFS_NULL		   0
#define	OFS_RETURN		1
#define	RESERVED_OFS	4
#define	OFS_CAM        4
#define  OFS_END        6

extern const str complextypestring;
extern const str emptystring;
extern const str resultstring;
extern const str immediatestring;

// offsets are always multiplied by 4 before using
typedef int	gofs_t;				// offset in global data block

typedef enum 
   {
   ev_error = -1, ev_void, ev_string, ev_float, ev_vector, ev_entity, ev_function
   } etype_t;

typedef int	func_t;

typedef union eval_s
   {
	int				string;
	float				_float;
	float				vector[ 3 ];
	func_t			function;
	int				_int;
   int            entity;
   } eval_t;

class def_t;

class type_t : public Class
{
public:
	type_t( etype_t t = ev_error, def_t *d = NULL, type_t *n = NULL, type_t *aux = NULL ) :
		type( t ), def( d ), next( n ), aux_type( aux ) { num_parms = 0; min_parms = 0; };
	CLASS_PROTOTYPE( type_t );

	virtual void	Archive( Archiver &arc );

	etype_t			type;
	def_t          *def;		                  // a def that points to this type
	type_t			*next;

   // function types are more complex
	type_t			*aux_type;	               // return type
	int				num_parms;	               // -1 = variable args
	int				min_parms;	               // for variable arg events
	type_t			*parm_types[ MAX_PARMS ];	// only [num_parms] allocated
};

class def_t : public Class
   {
   public:
	   type_t		   *type;
	   str            name;
      def_t          *next;
	   gofs_t		   ofs;
      gofs_t		   localofs;      // equal to ofs for globals, negative for locals
      def_t          *scope;		   // function the var was defined in, or NULL
	   int			   initialized;	// 1 when a declaration included "= immediate"

		bool				caseSensitive;

		bool				_onDefList;

                     def_t();

							CLASS_PROTOTYPE( def_t );

		virtual void	Archive( Archiver &arc );
   };

typedef struct
   {
	unsigned short	type;
	unsigned short	ofs;
   signed   short localofs;
	unsigned short s_name;
   } ddef_t;

typedef struct statement_s
   {
	unsigned short	op;
	short	         a;
   short	         b;
   short	         c;
   unsigned short linenumber;
   unsigned short file;
   } dstatement_t;

class dfunction_t : public Class
   {
public:
	dfunction_t()	{ };
	CLASS_PROTOTYPE( dfunction_t );

	virtual void	Archive( Archiver &arc );

   int      eventnum;
	int		first_statement;	// negative numbers are builtins
	int		parm_start;
   int		parm_total;
	int		locals;				// total ints of parms + locals
	
	int		profile;		      // runtime
	
	str      s_name;
	str      s_file;			   // source file defined in

	int		numparms;
   int      minparms;
	byte	   parm_size[ MAX_PARMS ];
   byte	   parm_type[ MAX_PARMS ];
   };

class localstr_t
{
public:
	localstr_t() : inuse( false ), s() { };
	bool		inuse;
	str		s;
};

class TargetList;
class Program : public Class
   {
   public:
		CLASS_PROTOTYPE( Program );

		virtual void	Archive( Archiver &arc );

      def_t		      *pr_global_defs[ MAX_REGS ];	// to find def for a global variable
      float		      pr_globals[ MAX_REGS ];
      int		      numpr_globals;

      int            locals_start;
      int			   locals_end;		// for tracking local variables vs temps

      localstr_t		strings[ MAX_STRINGS ];

      dstatement_t	statements[ MAX_STATEMENTS ];
      int			   numstatements;

      dfunction_t	   functions[ MAX_FUNCTIONS ];
      int			   numfunctions;

	   type_t		   *types;	
	   def_t		      def_head;		               // unused head of linked list
	   def_t		      *def_tail;		               // add new defs after this and move it

      int		      pr_error_count;

      Container<str> filenames;
      str            s_file;

      type_t         *FindType( const type_t *type );
      def_t          *GetDef( type_t *type, const char *name, def_t *scope, bool allocate, Lexer *lex );
      void           CreateDefForEvent( Event *ev );
      void           CreateEventDefs( void );

      int				CopyString( const char *str );

                     Program();
                     ~Program();

      void           InitData( void );
      void           BeginCompilation( void );
      bool           FinishCompilation( void );
      void           FreeData( void );

      const char     *GetFilename( int num );

      void           Compile( const char *filename );
      void           Load( const char *filename );

      float          getFloat( int offset );
      int            getInteger( int offset );
      float          *getVector( int offset );
      const char     *getString( int offset );
      Entity         *getEntity( int offset );
      gentity_t      *getEdict( int offset );
      TargetList     *getTargetList( int offset );
      func_t         getFunction( int offset );

      void           setFloat( int offset, float value );
      void           setInteger( int offset, int value );
      void           setVector( int offset, const Vector &vec );
      void           setString( int offset, const char *text );
      void           setFunction( int offset, func_t func );
      void           setEntity( int offset, const Entity *ent );
      void           setTargetList( int offset, const TargetList *list );

      func_t         findFunction( const char *name );

		int				AllocString();
		void				FreeString( int idx );
		int				CountUsedStrings();
   };
/*
inline float Program::getFloat
   (
   int offset
   )

   {
   return pr_globals[ offset ];
   }

inline int Program::getInteger
   (
   int offset
   )

   {
   return *( int * )&pr_globals[ offset ];
   }

inline float *Program::getVector
   (
   int offset
   )

   {
   return &pr_globals[ offset ];
   }

inline const char *Program::getString
   (
   int offset
   )

   {
   return strings[ *( int * )&pr_globals[ offset ] ].s.c_str();
//   return ( ( str * )&pr_globals[ offset ] ] ] )->c_str();
//   return ( ( str * )( &pr_globals[ offset ] ) )->getString();
   }

inline func_t Program::getFunction
   (
   int offset
   )

   {
   return *( func_t * )&pr_globals[ offset ];
   }

inline Entity *Program::getEntity
   (
   int offset
   )

   {
   Entity *ent;
   int entnum;

   entnum = *( int * )&pr_globals[ offset ];

   if ( entnum > 0 )
      {
      ent = G_GetEntity( entnum - 1 );
      return ent;
      }

   return NULL;
   }

inline TargetList *Program::getTargetList
   (
   int offset
   )

   {
   int entnum;

   entnum = *( int * )&pr_globals[ offset ];

   if ( entnum < 0 )
      {
      return world->GetTargetList( -entnum );
      }

   return NULL;
   }

inline gentity_t *Program::getEdict
   (
   int offset
   )

   {
   int entnum;
   Entity *ent;

   entnum = *( int * )&pr_globals[ offset ];

   if ( entnum > 0 )
      {
      ent = G_GetEntity( entnum - 1 );
      if ( ent )
         {
         return ent->edict;
         }
      }
   else if ( entnum < 0 )
      {
      }

   return NULL;
   }

inline void Program::setFunction
   (
   int offset,
   func_t func
   )

   {
   *( func_t * )&pr_globals[ offset ] = func;
   }

inline void Program::setEntity
   (
   int offset,
   Entity *ent
   )

   {
   if ( ent )
      {
      *( int * )&pr_globals[ offset ] = ent->entnum + 1;
      }
   else
      {
      *( int * )&pr_globals[ offset ] = 0;
      }
   }

inline void Program::setTargetList
   (
   int offset,
   TargetList *list
   )

   {
   if ( list )
      {
      *( int * )&pr_globals[ offset ] = -list->index;
      }
   else
      {
      *( int * )&pr_globals[ offset ] = 0;
      }
   }

inline void Program::setString
   (
   int offset,
   const char *text
   )

   {
   strings[ offset ].s = text;
   }

inline void Program::setFloat
   (
   int offset,
   float value
   )

   {
   pr_globals[ offset ] = value;
   }

inline void Program::setInteger
   (
   int offset,
   int value
   )

   {
   *( int * )&pr_globals[ offset ] = value;
   }

inline void Program::setVector
   (
   int offset,
   Vector &vec
   )

   {
   *( Vector * )&pr_globals[ offset ] = vec;
   }

inline const char *Program::GetFilename
   (
   int num
   )

   {
   return filenames.ObjectAt( num );
   }
*/
#endif
