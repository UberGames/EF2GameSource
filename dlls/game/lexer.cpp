//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/lexer.cpp                                      $
// $Revision:: 7                                                              $
//     $Date:: 10/08/02 7:35a                                                 $
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

#include "_pch_cpp.h"
#include "lexer.h"

// longer symbols must be before a shorter partial match
static const char *pr_punctuation[] =
{
	"+=", "-=",  "*=",  "/=", "&=", "|=", "++", "--",
	"&&", "||",  "<=",  ">=", "==", "!=", ";",  ",", "!",
	"*",  "/",   "(",   ")",  "-",  "+",  "=",  "[", "]",
	"{",  "}",   "...", ".",  "<",  ">" , "#" , "&", "|",
	NULL
};

// simple types.  function types are dynamically allocated
type_t	type_void( ev_void,      &def_void );
type_t	type_string( ev_string,    &def_string );
type_t	type_float( ev_float,     &def_float );
type_t	type_vector( ev_vector,    &def_vector );
type_t	type_entity( ev_entity,    &def_entity );
type_t	type_function( ev_function,  &def_function, NULL, &type_void );

// type_function is a void() function used for state defs

int		type_size[6]      = { 1, 1, 1, 3, 1, 1 };

/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#if 0

def_t	   def_void          = { &type_void,      "temp" };
def_t	   def_string        = { &type_string,    "temp" };
def_t	   def_float         = { &type_float,     "temp" };
def_t	   def_vector        = { &type_vector,    "temp" };
def_t	   def_entity        = { &type_entity,    "temp" };
def_t	   def_function      = { &type_function,  "temp" };

#else

****************************************************************************/


def_t	   def_void;
def_t	   def_string;
def_t	   def_float;
def_t	   def_vector;
def_t	   def_entity;
def_t	   def_function;

/****************************************************************************
Squirrel : #if 0 / 1 block demoted to comment

#endif

****************************************************************************/


Lexer::Lexer( Program &prg ) 
	: program( prg )
{
	source_line = 0;
	pr_file_p = NULL;
	pr_line_start = NULL;
	pr_bracelevel = 0;
	pr_immediate_type = NULL;
}

/*
==============
NewLine

Call at start of file and when *pr_file_p == '\n'
==============
*/
void Lexer::NewLine( void )
{
	bool m;
	
	if ( *pr_file_p == '\n' )
	   {
		pr_file_p++;
		m = true;
	   }
	else
	{
		m = false;
	}
	
	source_line++;
	pr_line_start = pr_file_p;
	
	if ( m )
	{
		pr_file_p--;
	}
}

/*
==============
LexString

Parses a quoted string
==============
*/
void Lexer::LexString( void )
{
	int   c;
	int	len;
	
	len = 0;
	pr_file_p++;
	do
	{
		c = *pr_file_p++;
		if ( !c )
		{
			ParseError( "EOF inside quote" );
		}
		
		if ( c == '\n' )
		{
			ParseError( "newline inside quote" );
		}
		
		if ( c=='\\' )
		{
			// escape char
			c = *pr_file_p++;
			if ( !c )
			{
				ParseError( "EOF inside quote" );
			}
			
			if ( c == 'n' )
			{
				c = '\n';
			}
			else if ( c == '"' )
			{
				c = '"';
			}
			else
			{
				ParseError( "Unknown escape char" );
			}
		}
		else if ( c=='\"' )
		{
			pr_token[ len ] = 0;
			pr_token_type = tt_immediate;
			pr_immediate_type = &type_string;
			strcpy( pr_immediate_string, pr_token );
			
			return;
		}
		
		pr_token[ len ] = c;
		len++;
	}
	while( 1 );
}

/*
==============
LexNumber
==============
*/
float Lexer::LexNumber( void )
{
	int c;
	int len;
	
	len = 0;
	c = *pr_file_p;
	do
	{
		pr_token[ len ] = c;
		len++;
		pr_file_p++;
		c = *pr_file_p;
	}
	while( ( ( c >= '0' ) && ( c <= '9' ) ) || ( c == '.' ) );
	   
	pr_token[ len ] = 0;
	   
	return ( float )atof( pr_token );
}

/*
==============
LexVector

Parses a single quoted vector
==============
*/
void Lexer::LexVector( void )
{
	int i;
	
	pr_file_p++;
	pr_token_type = tt_immediate;
	pr_immediate_type = &type_vector;
	for( i = 0; i < 3; i++ )
	{
		pr_immediate.vector[ i ] = LexNumber();
		LexWhitespace();
	}
	
	if ( *pr_file_p != '\'' )
	{
		ParseError( "Bad vector" );
	}
	
	pr_file_p++;
}

/*
==============
LexEntity

Parses an identifier
==============
*/
void Lexer::LexEntity( void )
{
	int c;
	int len;
	TargetList *list;
	
	pr_file_p++;
	len = 0;
	c = *pr_file_p;
	do
	{
		pr_token[ len ] = c;
		len++;
		pr_file_p++;
		c = *pr_file_p;
	}
	while( ( ( c >= 'a' ) && ( c <= 'z' ) ) || ( ( c >= 'A' ) && ( c <= 'Z' ) ) || ( c == '_' ) || ( ( c >= '0' ) && ( c <= '9' ) ) );
	   
	pr_token[ len ] = 0;
	pr_token_type = tt_immediate;
	pr_immediate_type = &type_entity;
	   
	list= world->GetTargetList( str( pr_token ), true );
	if ( list )
	{
		pr_immediate.entity = -list->index;
	}
	else
	{
		pr_immediate.entity = 0;
	}
}

/*
==============
LexName

Parses an identifier
==============
*/
void Lexer::LexName( void )
{
	int c;
	int len;
	
	len = 0;
	c = *pr_file_p;
	do
	{
		pr_token[ len ] = c;
		len++;
		pr_file_p++;
		c = *pr_file_p;
	}
	while( ( ( c >= 'a' ) && ( c <= 'z' ) ) || ( ( c >= 'A' ) && ( c <= 'Z' ) ) || ( c == '_' ) || ( ( c >= '0' ) && ( c <= '9' ) ) );
	   
	pr_token[ len ] = 0;
	pr_token_type = tt_name;
}

/*
==============
LexPunctuation
==============
*/
void Lexer::LexPunctuation( void )
{
	int	len;
	const char	**ptr;
	const char  *p;
	
	pr_token_type = tt_punct;
	
	for( ptr = pr_punctuation; *ptr != NULL; ptr++ )
	{
		p = *ptr;
		len = strlen( p );
		if ( !strncmp( p, pr_file_p, len ) )
		{
			strcpy( pr_token, p );
			if ( p[ 0 ] == '{' )
            {
				pr_bracelevel++;
            }
			else if ( p[ 0 ] == '}' )
            {
				pr_bracelevel--;
            }
			
			pr_file_p += len;
			return;
		}
	}
	
	ParseError( "Unknown punctuation" );
}

/*
==============
LexWhitespace
==============
*/
void Lexer::LexWhitespace( void )
{
	int c;
	
	while( 1 )
	{
		// skip whitespace
		while( ( c = *pr_file_p ) <= ' ' )
		{
			if ( c=='\n' )
            {
				NewLine();
            }
			
			if ( c == 0 )
            {
				// end of file
				return;
            }
			
			pr_file_p++;
		}
		
		// skip // comments
		if ( ( c == '/' ) && ( pr_file_p[ 1 ] == '/' ) )
		{
			while( *pr_file_p && ( *pr_file_p != '\n' ) )
            {
				pr_file_p++;
            }
			
			NewLine();
			pr_file_p++;
			
			continue;
		}
		
		// skip /* */ comments
		if ( ( c == '/' ) && ( pr_file_p[ 1 ] == '*' ) )
		{
			do
			{
				pr_file_p++;
				if ( pr_file_p[ 0 ]=='\n' )
				{
					NewLine();
				}
				
				if ( pr_file_p[ 1 ] == 0 )
				{
					return;
				}
			} 
			while( ( pr_file_p[ -1 ] != '*' ) || ( pr_file_p[ 0 ] != '/' ) );
			
			pr_file_p++;
			continue;
		}
		
		// a real character has been found
		break;
	}
}

void Lexer::LexPreprocessor( void )
{
	int c;
	int len;
	
	len = 0;
	c = *pr_file_p;
	do
	{
		pr_token[ len ] = c;
		len++;
		pr_file_p++;
		c = *pr_file_p;
	}
	while( ( ( c >= 'a' ) && ( c <= 'z' ) ) || ( ( c >= 'A' ) && ( c <= 'Z' ) ) || ( c == '_' ) || ( ( c >= '0' ) && ( c <= '9' ) ) );

	pr_token[ len ] = 0;
	pr_token_type = tt_preprocessor;
}

//============================================================================

/*
==============
Lex

Sets pr_token, pr_token_type, and possibly pr_immediate and pr_immediate_type
==============
*/
void Lexer::Lex( void )
{
	int c;
	
	pr_token[ 0 ] = 0;
	
	if ( !pr_file_p )
	   {
		pr_token_type = tt_eof;
		return;
	   }
	
	LexWhitespace();
	
	c = *pr_file_p;
	if ( !c )
	   {
		pr_token_type = tt_eof;
		return;
	   }
	
	// handle quoted strings as a unit
	if ( c == '\"' )
	   {
		LexString ();
		return;
	   }
	
	// handle quoted vectors as a unit
	if ( c == '\'' )
	   {
		LexVector ();
		return;
	   }
	
	// if the first character is a valid identifier, parse until a non-id
	// character is reached
	if ( ( ( c >= '0' ) && ( c <= '9' ) ) || ( ( c == '-' ) && ( pr_file_p[ 1 ] >= '0' ) && ( pr_file_p[ 1 ] <= '9' ) ) || 
		( ( c == '.' ) && ( pr_file_p[ 1 ] >= '0' ) && ( pr_file_p[ 1 ] <= '9' ) ) )
	   {
		pr_token_type = tt_immediate;
		pr_immediate_type = &type_float;
		pr_immediate._float = LexNumber();
		return;
   	}
	
	// entity names
	if ( c == '$' )
	   {
		LexEntity();
		return;
	   }
	
	if ( c == '#' )
	{
		LexPreprocessor();
		return;
	}
	
	if ( ( ( c >= 'a' ) && ( c <= 'z' ) ) || ( ( c >= 'A' ) && ( c <= 'Z' ) ) || ( c == '_' ) )
	   {
		LexName();
		return;
	   }
	
	// parse symbol strings until a non-symbol is found
	LexPunctuation();
}

//=============================================================================

/*
============
ParseError

Aborts the current file load
============
*/
void Lexer::ParseError( const char *error, ... )
{
	va_list	argptr;
	char		string[ 1024 ];
	
	va_start( argptr, error );
	vsprintf( string, error, argptr );
	va_end( argptr );
	
	gi.Printf( "%s(%i) : %s\n", program.s_file.c_str(), source_line, string );
	
	throw "Error";
}

/*
=============
Expect

Issues an error if the current token isn't equal to string
Gets the next token
=============
*/
void Lexer::Expect( char *string )
{
	if ( strcmp( string, pr_token ) )
	{
		ParseError( "expected %s, found %s",string, pr_token );
	}
	
	Lex();
}

/*
=============
Check

Returns true and gets the next token if the current token equals string
Returns false and does nothing otherwise
=============
*/
bool Lexer::Check( const char *string )
{
	if ( strcmp( string, pr_token ) )
	{
		return false;
	}
	
	Lex();
	
	return true;
}

/*
============
ParseName

Checks to see if the current token is a valid name
============
*/
char *Lexer::ParseName( void )
{
	static char	ident[ MAX_NAME ];
	
	if ( pr_token_type != tt_name )
	{
		ParseError( "not a name" );
	}
	
	if ( strlen( pr_token ) >= ( MAX_NAME - 1 ) )
	{
		ParseError( "name too long" );
	}
	
	strcpy( ident, pr_token );
	Lex();
	
	return ident;
}

/*
============
SkipOutOfFunction

For error recovery, pops out of nested braces
============
*/
void Lexer::SkipOutOfFunction( void )
{
	do
	{
		if ( !pr_bracelevel )
		{
			return;
		}
		   
		Lex();
	}
	while( pr_token[ 0 ] ); // eof will return a null token
}

/*
============
SkipToSemicolon

For error recovery
============
*/
void Lexer::SkipToSemicolon( void )
{
	do
	{
		if ( Check( ";" ) )
		{
			return;
		}
		   
		Lex();
	}
	while( pr_token[ 0 ] ); // eof will return a null token
}

/*
============
CheckType

Parses a variable type, including functions types
============
*/
type_t *Lexer::CheckType( void )
{
	type_t *type;
	
	if ( !strcmp( pr_token, "float" ) )
	{
		type = &type_float;
	}
	else if ( !strcmp( pr_token, "vector" ) )
	{
		type = &type_vector;
	}
	else if ( !strcmp( pr_token, "entity" ) )
	{
		type = &type_entity;
	}
	else if ( !strcmp( pr_token, "string" ) )
	{
		type = &type_string;
	}
	else if ( !strcmp( pr_token, "void" ) )
	{
		type = &type_void;
	}
	else
	{
		type = NULL;
	}
	
	return type;
}

/*
============
ParseType

Parses a variable type, including functions types
============
*/
type_t *Lexer::ParseType( void )
{
	type_t *type;
	
	type = CheckType();
	if ( !type )
	{
		ParseError( "\"%s\" is not a type", pr_token );
	}
	
	Lex();
	
	return type;
}
