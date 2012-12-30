//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/Shared/qcommon/tiki_script.h                         $
// $Revision:: 10                                                             $
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
// C++ implementaion of tokenizing text interpretation.  Class accepts filename
// to load or pointer to preloaded text data.  Standard tokenizing operations
// such as skip white-space, get string, get integer, get float, get token,
// and skip line are implemented.  
//
// Note: all '//', '#', and ';' are treated as comments.  Probably should
// make this behaviour toggleable.
// 

#ifndef __TIKI_SCRIPT_H__
#define __TIKI_SCRIPT_H__

#if !defined( QERADIANT ) && !defined( UTILS )
#define	MAXTOKEN	256
#endif
#define  MAXMACROS 384 // was 256, increased for ef2 weapon lists

typedef struct
	{
	char	name[ MAXTOKEN ];
	char	macro[ MAXTOKEN ];
	} tiki_macro_t;

class TikiScript
	{
	public:
		~TikiScript();
		TikiScript();

		char*	buffer;
		int 	length;

		void			Close();
		const char*		Filename();
		int 			GetLineNumber();
		void			Reset();	
		qboolean		TokenAvailable( qboolean crossline );
		void			UnGetToken();
		const char*		GetToken( qboolean crossline );
		const char*		GetLine( qboolean crossline );
		const char*		GetRaw();
		const char*		GetString( qboolean crossline );
		qboolean		GetSpecific( const char *string );
		int 			GetInteger( qboolean crossline );
		double			GetDouble( qboolean crossline );
		float			GetFloat( qboolean crossline );
		void			GetVector( qboolean crossline, vec3_t vec );
		int 			LinesInFile();
		void			Parse( char *data, int length, const char *name );
		qboolean		LoadFile( const char *name, qboolean quiet = qfalse );
		qboolean		LoadFileFromTS( const char *name, const char * tikidata, qboolean quiet = qfalse );
		const char*		Token();
		float			EvaluateMacroMath(float value, float newval, char oper);
		char*			EvaluateMacroString(const char *theMacroString);
		const char*		GetMacroString(const char *theMacroName);

	protected:
		qboolean		error;
		qboolean		tokenready;
		TikiScript*		include;
		TikiScript*		parent;

		char			filename[ MAXTOKEN ];
		const char*		script_p;
		const char*		end_p;
		tiki_macro_t	macros[ MAXMACROS ];
		int 			nummacros;

		int 			line;		
		char			token[ MAXTOKEN ];

		qboolean		releaseBuffer;

		qboolean		AtComment();
		qboolean		AtExtendedComment();
		qboolean		AtCommand();
		qboolean		AtString( qboolean crossline );
		qboolean		ProcessCommand( qboolean crossline );
		qboolean		Completed();
		void			CheckOverflow();
		void			Uninclude();
		const char*		FindMacro( const char * macro );
		void			AddMacro( const char *macro, const char *expansion );

		qboolean		SkipToEOL();
		void			SkipWhiteSpace( qboolean crossline );
		void			SkipNonToken( qboolean crossline );
		qboolean		CommentAvailable( qboolean crossline );
		void			SkipExtendedComment();
	};


#endif
