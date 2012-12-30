//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/interpreter.h                                  $
// $Revision:: 6                                                              $
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

#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include "program.h"

#define	MAX_STACK_DEPTH		32
#define	LOCALSTACK_SIZE		2048

typedef struct
   {
	int				s;
	dfunction_t		*f;
   int            stackbase;
   } prstack_t;

class Interpreter : public Listener
   {
   private:
      prstack_t	         pr_stack[ MAX_STACK_DEPTH ];
      int			         pr_depth;

      byte                 localstack_type[ LOCALSTACK_SIZE ];
      int			         localstack[ LOCALSTACK_SIZE ];
      int			         localstack_used;
      int			         localstack_base;

      bool                 pr_trace;
      dfunction_t	         *pr_xfunction;
      int			         pr_xstatement;

   public:

      Program              *program;
      int                  instruction_pointer;
      bool                 doneProcessing;
      bool                 threadDying;

      Container<int>			updateList;

      int						threadNum;
		str						threadName;

      CLASS_PROTOTYPE( Interpreter );

                           Interpreter();
                           ~Interpreter();

      void                 PrintStatement( const dstatement_t *s );
      void                 StackTrace( void );
      void                 Profile( void );
      void                 RunError( const char *error, ... );
	  inline void			NullEntityError( const dstatement_t* statement ); //should be const but can't be
      void                 ThreadCall( const Interpreter *source, dfunction_t	*newf, int args );
      int                  EnterFunction( dfunction_t *func );
      int                  LeaveFunction( void );
      Event                *EventForFunction( const dfunction_t *func, int args );
      void                 Execute( Event *ev );
      void                 Reset( void );
      void                 Wait( float time );
                           
      const char           *GlobalString( int ofs );
      const char           *GlobalStringNoContents( int ofs );

      const char           *Filename( void );

		void						SetThreadNum( int num );
		int						ThreadNum( void );
		const char				*ThreadName( void );

      virtual void         DoMove( void );

      qboolean             labelExists( const char *label );
      qboolean             Goto( const char *label );
      qboolean             Setup( int num, const char *label );

      void						Archive( Archiver &arc );
		void						CleanupStack( int localstack_used, int oldstacktop );
   };

inline void Interpreter::SetThreadNum
	(
   int num
	)

	{
   threadNum = num;
	}

inline int Interpreter::ThreadNum
	(
	void
	)

	{
	return threadNum;
	}

inline const char *Interpreter::ThreadName
	(
	void
	)

	{
	return threadName.c_str();
	}

inline const char *Interpreter::Filename( void )
{
	if ( instruction_pointer < 0 )
		return "";
	else
		return program->GetFilename( program->statements[ instruction_pointer ].file );
}

#endif
