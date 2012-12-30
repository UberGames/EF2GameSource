//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/lexer.h                                        $
// $Revision:: 4                                                              $
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

#ifndef __LEXER_H__
#define __LEXER_H__

#include "program.h"

#define	MAX_FRAMES	256

extern	int		type_size[ 6 ];

extern	type_t	type_void;
extern	type_t	type_string;
extern	type_t	type_float;
extern	type_t	type_vector;
extern	type_t	type_entity;
extern	type_t	type_function;

extern	def_t	   def_void;
extern	def_t	   def_string;
extern	def_t	   def_float;
extern	def_t	   def_vector;
extern	def_t	   def_entity;
extern	def_t	   def_function;

extern	def_t	   def_ret;
extern	def_t	   junkdef;

typedef enum
   {
   tt_eof,			// end of file reached
   tt_name, 		// an alphanumeric name token
   tt_punct, 		// code punctuation
   tt_immediate,	// string, float, vector
   tt_preprocessor  // #-prefixed command
   } token_type_t;

class Lexer
   {
   private:
      int			   source_line;
      
      const char		*pr_file_p;
      const char		*pr_line_start;		// start of current source line
      
      int			   pr_bracelevel;

      char		      pr_token[ 2048 ];

   public:
   
      token_type_t	pr_token_type;

      type_t		   *pr_immediate_type;
      eval_t		   pr_immediate;
      char	         pr_immediate_string[ 2048 ];

      Program        &program;

                     Lexer( Program &prg );

      void           NewLine( void );
      void           LexString( void );
      float          LexNumber( void );
      void           LexVector( void );
      void           LexEntity( void );
      void           LexName( void );
      void           LexPunctuation( void );
      void           LexWhitespace( void );
      void           LexPreprocessor( void );
      void           Lex( void );
      void           ParseError( const char *error, ... );
      void           Expect( char *string );
      bool           Check( const char *string );
      char           *ParseName( void );
      void           SkipOutOfFunction( void );
      void           SkipToSemicolon( void );
      type_t         *CheckType( void );
      type_t         *ParseType( void );

      int            SourceLine( void );
      void           SetSource( const char *text );
   };

inline int Lexer::SourceLine
   (
   void
   )

   {
   return source_line;
   }

inline void Lexer::SetSource
   (
   const char *text
   )

   {
   pr_file_p = text;
	source_line = 0;
   NewLine();
   }

#endif
