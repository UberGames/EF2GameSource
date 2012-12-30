//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/q_shared.c                                $
// $Revision:: 36                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// A mix of functions used by every part of the code

#include "q_shared.h"

#ifdef _WIN32
#pragma optimize( "", off )
#endif

static char musicmoods[ mood_totalnumber ][ 16 ] =
{
	"none",
	"normal",
	"action",
	"suspense",
	"mystery",
	"success",
	"failure",
	"surprise",
	"special",
	"aux1",
	"aux2",
	"aux3",
	"aux4",
	"aux5",
	"aux6",
	"aux7"
};

static char eaxmodes[ eax_totalnumber ][ 16 ] =
{
	"generic",
	"paddedcell",
	"room",
	"bathroom",
	"livingroom",
	"stoneroom",
	"auditorium",
	"concerthall",
	"cave",
	"arena",
	"hangar",
	"carpetedhallway",
	"hallway",
	"stonecorridor",
	"alley",
	"forest",
	"city",
	"mountains",
	"quarry",
	"plain",
	"parkinglot",
	"sewerpipe",
	"underwater",
	"drugged",
	"dizzy",
	"psychotic"
};

/*
=================
MusicMood_NameToNum
=================
*/
int MusicMood_NameToNum( const char * name )
{
	int i;
	
	if ( !name )
		return -1;
	
	for ( i = 0; i < mood_totalnumber; i++ )
	{
		if ( !strcmpi( name, musicmoods[ i ] ) )
		{
			return i;
		}
	}
	return -1;
}

/*
=================
MusicMood_NumToName
=================
*/
const char * MusicMood_NumToName( int num )
{
	if ( ( num < 0 ) || ( num >= mood_totalnumber ) )
		return "";
	else
		return musicmoods[ num ];
}

/*
=================
EAXMode_NameToNum
=================
*/
int EAXMode_NameToNum( const char * name )
{
	int i;
	
	if ( !name )
		return -1;
	
	for ( i = 0; i < eax_totalnumber; i++ )
	{
		if ( !strcmpi( name, eaxmodes[ i ] ) )
		{
			return i;
		}
	}
	return -1;
}

/*
=================
EAXMode_NumToName
=================
*/
const char * EAXMode_NumToName( int num )
{
	if ( ( num < 0 ) || ( num >= eax_totalnumber ) )
		return "";
	else
		return eaxmodes[ num ];
}

static char playerStatNames[ STAT_LAST_STAT ][ 32 ] =
{
	"health",
	"dead_yaw",
	"ammo_left",
	"clipammo_left",
	"num_shots_left",
	"max_num_shots_left",
	"ammo_right",
	"clipammo_right",
	"num_shots_right",
	"armor_level",
	"maxammo_left",
	"maxammo_right",
	"maxclipammo_left",
	"maxclipammo_right",
	"ammo_type1",
	"ammo_type2",
	"ammo_type3",
	"ammo_type4",
	"last_pain",
	"accumulated_pain",
	"bosshealth",
	"bossnameIndex",
	"cinematic",
	"addfade",
	"letterbox",
	"poweruptime",

	"weapon_generic1",
	"weapon_generic2",

	"itemicon",
	"itemtext",

	"votetext",

	"enemies_killed",
	"teammates_killed",
	"shots_fired",
	"shots_hit",
	"accuracy",
	"mission_duration",

	"generic",

	"num_objectives",
	"complete_objectives",
	"failed_objectives",
	"incomplete_objectives",

	"specialmovetimer",
	"points",

	"secretstotal",
	"secretsfound",
	"itemstotal",
	"itemsfound",

	"redTeamScore",
	"blueTeamScore",
	//"arena",
	"team",
	//"queue_place",
	"score",
	"kills",
	"deaths",
//	"timeleft_minutes",
	"timeleft_seconds",
	"won_matches",
	"lost_matches",

	"mp_generic1",
	"mp_generic2",
	"mp_generic3",
	"mp_generic4",
	"mp_generic5",
	"mp_generic6",
	"mp_generic7",
	"mp_generic8",

	"mp_spectating_entnum",

	"mp_mode_icon",
	"mp_team_icon",
	"mp_teamhud_icon",
	"mp_otherteam_icon",
	"mp_specialty_icon",
	"mp_holdableitem_icon",
	"mp_rune_icon",
	"mp_powerup_icon",

	"mp_award_icon",
	"mp_award_count",

	"mp_state"
};


//----------------------------------------------------------------
// Name:			GenerateHashForName (Squirrel)
// Class:			
//
// Description:		Generates an unsigned 32-bit integer hash
//					value for a given char* string.  The resulting
//					hash value is guaranteed to be in the range
//					of [0,maxHash).  If no maximum hash value is
//					desired, pass 0 for <maxHash>; this allows
//					<hash> to be any 32-bit unsigned int value.
// Parameters:		
//	const char* name		The string for which a hash will be generated
//	qboolean caseSensitive	If false, hash is generated from all lower-case
//	unsigned int maxHash	The non-inclusive maximum hash value desired
// 
// Returns:		unsigned int	32-bit unsigned hash value for <name>
//----------------------------------------------------------------
unsigned int GenerateHashForName( const char* name, qboolean caseSensitive, unsigned int maxHash )
{
	const char* scan;
	unsigned int hash = 0;
	unsigned int ch;

	/// Add each individual character to the hash
	for( scan = name; *scan; scan ++ )
	{
		/// Get character (or its lowercase version, if we're case-insensitive)
		ch = *scan;
		if( !caseSensitive )
			ch = tolower( ch );

		/// Multiply the existing hash by a magic number and add the new character
		hash &= 0x07ffffff; // clear the top 5 bits to make room for multiply
		hash *= 31;
		hash += ch;
	}

	/// Mod-clamp the hash to the range [0,maxHash) IF <maxHash> was specified
	if( maxHash )
		hash %= maxHash;

	return( hash );
}


//----------------------------------------------------------------
// Name:			PlayerStat_NameToNum
// Class:			
//
// Description:		Converts a player stat name to the corresponding index
//
// Parameters:		const char *name					- player stat name
//
// Returns:			int									- index of the player stat
//----------------------------------------------------------------

int PlayerStat_NameToNum( const char *name )
{
	int i;

	if ( !name )
		return -1;

	for ( i = 0 ; i < STAT_LAST_STAT ; i++ )
	{
		if ( strcmpi( name, playerStatNames[ i ] ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

//----------------------------------------------------------------
// Name:			PlayerStat_NumToName
// Class:			
//
// Description:		Converts a player stat index to the corresponding name
//
// Parameters:		int									- player stat index
//
// Returns:			const char *						- name of the player stat
//----------------------------------------------------------------

const char *PlayerStat_NumToName( int num )
{
	if ( ( num < 0 ) || ( num >= STAT_LAST_STAT ) )
		return "";
	else
		return playerStatNames[ num ];
}

//====================================================================================

/*
============
COM_SkipPath
============
*/
const char *COM_SkipPath (const char *pathname)
{
	const char	*last;

	last = pathname;
	while (*pathname)
	{
		if (*pathname=='/')
			last = pathname+1;
		pathname++;
	}
	return last;
}

/*
============
COM_ParseHex
============
*/
int COM_ParseHex (const char *hex)
{
	const char    *str;
	int    num;

	num = 0;
	str = hex;

	while (*str)
	{
		num <<= 4;
		if ( ( *str >= '0' ) && ( *str <= '9' ) )
			num += *str - '0';
		else if ( ( *str >= 'a' ) && ( *str <= 'f' ) )
			num += 10 + *str - 'a';
		else if ( ( *str >= 'A' ) && ( *str <= 'F' ) )
			num += 10 + *str - 'A';
		else
			Com_WPrintf("Bad hex number: %s",hex);
		str++;
	}

	return num;
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension (const char *in, char *out)
   {
	while ( *in && ( *in != '.' ) )
		*out++ = *in++;
	*out = 0;
   }

/*
============
COM_FileExtension
============
*/
const char *COM_FileExtension (const char *in)
{
	static char exten[8];
	int		i;

	while ( *in && ( *in != '.' ) )
		in++;
	if (!*in)
		return "";
	in++;
	for (i=0 ; i<7 && *in ; i++,in++)
		exten[i] = *in;
	exten[i] = 0;
	return exten;
}

/*
============
COM_FileBase
============
*/
void COM_FileBase (const char *in, char *out)
{
	const char *s;
	const char *s2;

	s = in + strlen(in) - 1;

	while ( ( s != in ) && ( *s != '.' ) )
		s--;

	for ( s2 = s ; ( s2 != in ) && ( *s2 != '/' ) ; s2-- )
		;

	if (s-s2 < 2)
		out[0] = 0;
	else
	{
		s--;
		strncpy (out,s2+1, s-s2);
		out[s-s2] = 0;
	}
}

/*
============
COM_FileName

Returns the filename, without being picky like COM_FileBase
============
*/
void COM_FileName (const char *in, char *out)
{
	const char *start, *end, *s;

	start = NULL;
	end = NULL;

	for (s = in; *s; s++)
	{
		if(*s == '.')
		{
			end = s;
		}
		else if((*s == '/') || (*s == '\\') || (*s == ':'))
		{
			start = s+1;
		}
	}

	if(end == NULL)
		end = s;

	if(start == NULL)
		start = in;

	for (s=start; s<end; s++)
	{
		*(out++) = *s;
	}
	*out = '\0';
}

/*
============
COM_FilePath

Returns the path up to, but not including the last /
============
*/
void COM_FilePath (const char *in, char *out)
{
	const char *s;

	s = in + strlen(in) - 1;

	while ( ( s != in ) && ( *s != '/' ) )
		s--;

	strncpy (out,in, s-in);
	out[s-in] = 0;
}

/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension (char *path, int maxSize, const char *extension ) {
	char	oldPath[MAX_QPATH];
	char    *src;

//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while ( ( *src != '/' ) && ( src != path ) ) {
		if ( *src == '.' ) {
			return;                 // it has an extension
		}
		src--;
	}

   // catch any errors we may encounter, while debugging
   assert( strlen( path ) <= sizeof( oldPath ) );

	Q_strncpyz( oldPath, path, sizeof( oldPath ) );
	Com_sprintf( path, maxSize, "%s%s", oldPath, extension );
}

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

qboolean	bigendian;

// can't just use function pointers, or dll linkage can
// mess up when qcommon is included in multiple places
short	(*_BigShort) (short l);
short	(*_LittleShort) (short l);
int		(*_BigLong) (int l);
int		(*_LittleLong) (int l);
float	(*_BigFloat) (float l);
float	(*_LittleFloat) (float l);
unsigned short	(*_BigUnsignedShort) (unsigned short l);
unsigned short	(*_LittleUnsignedShort) (unsigned short l);

short	BigShort(short l){return _BigShort(l);}
short	LittleShort(short l) {return _LittleShort(l);}
int		BigLong (int l) {return _BigLong(l);}
int		LittleLong (int l) {return _LittleLong(l);}
float	BigFloat (float l) {return _BigFloat(l);}
float	LittleFloat (float l) {return _LittleFloat(l);}
unsigned short	BigUnsignedShort(unsigned short l){return _BigUnsignedShort(l);}
unsigned short	LittleUnsignedShort(unsigned short l) {return _LittleUnsignedShort(l);}

short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short	ShortNoSwap (short l)
{
	return l;
}


unsigned short   UnsignedShortSwap (unsigned short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

unsigned short	UnsignedShortNoSwap (unsigned short l)
{
	return l;
}


int    LongSwap (int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int	LongNoSwap (int l)
{
	return l;
}

float FloatSwap (float f)
{
	union
	{
		float	f;
		byte	b[4];
	} dat1, dat2;


	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float FloatNoSwap (float f)
{
	return f;
}

/*
================
Swap_Init
================
*/
void Swap_Init (void)
{
	byte	swaptest[2] = {1,0};
	
	// set the byte swapping variables in a portable manner
	if ( *(short *)swaptest == 1)
	{
		bigendian = qfalse;
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigUnsignedShort = UnsignedShortSwap;
		_LittleUnsignedShort = UnsignedShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	}
	else
	{
		bigendian = qtrue;
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigUnsignedShort = UnsignedShortNoSwap;
		_LittleUnsignedShort = UnsignedShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}
}



/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
const char *va( const char *format, ... )
{
	va_list		argptr;
	static char		string[2][16384];	// in case va is called by nested functions
	static int		index = 0;
	char	*buf;

	buf = string[index & 1];
	index++;

	va_start (argptr, format);
	vsprintf (buf, format,argptr);
	va_end (argptr);

	return buf;
}


char	com_token[MAX_STRING_CHARS];

/*
==============
COM_GetToken

Parse a token out of a string
==============
*/
const char *COM_GetToken(const char **data_p, qboolean crossline)
{
	unsigned int		c;
	int					len;
	const unsigned char *data;

	data = (const unsigned char*)*data_p;
	len = 0;
	com_token[0] = 0;

	if (!data)
	{
		*data_p = NULL;
		return "";
	}

// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
      if (c == '\n' && !crossline)
         {
         *data_p = (const char*) data;
         return "";
         }
		if ( !c )
		{
			*data_p = NULL;
			return "";
		}
		data++;
	}

// skip // comments
	if ( ( c == '/' ) && ( data[1] == '/' ) )
	{
		while ( *data && ( *data != '\n' ) )
			data++;
		goto skipwhite;
	}

// skip /* comments
	if ( ( c == '/' ) && ( data[1] == '*' ) )
	   {
      data++;
      while (*data)
         {
         if ( (*(data-1)=='*') && (*data == '/') )
            break;
         data++;
         }
		while ( *data && ( *data != '\n' ) )
			data++;
		goto skipwhite;
	   }


// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
         if ( ( c == '\\' ) && ( *data == '\"' ) )
            {
			   if (len < MAX_STRING_CHARS)
			      {
				   com_token[len] = '\"';
				   len++;
			      }
            data++;
            }
			else if (c=='\"' || !c)
			{
				com_token[len] = 0;
				*data_p = (const char*)data;
				return com_token;
			}
         else if (len < MAX_STRING_CHARS)
			{
            if ( ( c == '\\' ) && ( *data == 'n' ) )
               {
               com_token[len] = '\n';
               data++;
               }
		      else
               {
               com_token[len] = c;
               }
				len++;
//            com_token[len] = c;
//				len++;
			}
		}
	}

// parse a regular word
	do
	{
		if (len < MAX_STRING_CHARS)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
	} while (c>32);

	if (len == MAX_STRING_CHARS)
	{
//		Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
		len = 0;
	}
	com_token[len] = 0;

	*data_p = (const char*)data;
	return com_token;
}


static	int		com_lines = 0;

/*
=================
SkipRestOfLine
=================
*/
void SkipRestOfLine ( char **data ) {
	char	*p;
	int		c;

	p = *data;
	while ( (c = *p++) != 0 ) {
		if ( c == '\n' ) {
			com_lines++;
			break;
		}
	}

	*data = p;
}

/*
=================
SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
qboolean SkipBracedSection (char **program) {
	char			*token;
	int				depth;

	depth = 0;
	do {
		token = COM_ParseExt( program, qtrue );
		if( token[1] == 0 ) {
			if( token[0] == '{' ) {
				depth++;
			}
			else if( token[0] == '}' ) {
				depth--;
			}
		}
	} while( depth && *program );

	return !depth;
}

static char *SkipWhitespace( char *data, qboolean *hasNewLines )
{
	int c;

	while ( (c = *data) <= ' ')
	{
		if ( !c )
		{
			return NULL;
		}
		if ( c == '\n' )
		{
			com_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

char *COM_ParseExt( char **data_p, qboolean allowLineBreaks )
{
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	char *data;

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	// make sure incoming data is valid
	if ( !data )
	{
		*data_p = NULL;
		return com_token;
	}

	while ( 1 )
	{
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data )
		{
			*data_p = NULL;
			return com_token;
		}
		if ( hasNewLines && !allowLineBreaks )
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if ( ( c == '/' ) && ( data[1] == '/' ) )
		{
			while ( *data && ( *data != '\n' ) )
				data++;
		}
		// skip /* */ comments
		else if ( ( c == '/' ) && ( data[1] == '*' ) )
		{
			while ( *data && ( ( *data != '*' ) || ( data[1] != '/' ) ) )
			{
				data++;
			}
			if ( *data )
			{
				data += 2;
			}
		}
		else
		{
			break;
		}
	}

	// handle quoted strings
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				*data_p = ( char * ) data;
				return com_token;
			}
			if (len < MAX_TOKEN_CHARS)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
      if ( ( len > 0 ) && ( ( c == '{' ) || ( c == '}' ) ) )
      {
         break;
      }

		data++;
		if (len < MAX_TOKEN_CHARS)
		{
         // handle '\n' correctly
         if ( ( c == '\\' ) && ( *data == 'n' ) )
            {
            com_token[len] = '\n';
            data++;
            }
         else
            {
            com_token[len] = c;
            }
			len++;

         if ( ( len == 1 ) && ( ( c == '{' ) || ( c == '}' ) ) )
         {
            break;
         }

		}
		c = *data;
		if ( c == '\n' )
			com_lines++;
	} while (c>32);

	if (len == MAX_TOKEN_CHARS)
	{
//		Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}
	com_token[len] = 0;

	*data_p = ( char * ) data;
	return com_token;
}

//-----------------------------------------------------
//
// Name:		
// Class:		
//
// Description:	
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
int COM_GetParseLineNumber( void )
{
	return com_lines;
}


/*
==============
COM_Parse

Parse a token out of a string
==============
*/
const char *COM_Parse (const char **data_p)
{
   return COM_GetToken( data_p, 1 );
}

/*
===============
Com_PageInMemory

===============
*/
int	paged_total;

void Com_PageInMemory ( const byte *buffer, int size )
{
	int		i;

	for (i=size-1 ; i>0 ; i-=4096)
		paged_total += buffer[i];
}



/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/
// never goes past bounds or leaves without a terminating 0
void Q_strcat( char *dest, int size, const char *src ) {
	int		l1;

	l1 = strlen( dest );
	if ( l1 >= size ) {
		Com_Error( ERR_FATAL, "Q_strcat: already overflowed" );
	}
	strncpy( dest + l1, src, size - 1 - l1 );
	dest[ size - 1 ] = 0;
}


char *Q_strlwr( char *s1 ) {
	char	*s;

	s = s1;
	while ( *s ) {
		*s = tolower(*s);
		s++;
	}
	return s1;
}

void Q_strncpyz( char *dest, const char *src, int destsize ) {
	if ( !src ) {
		Com_Error( ERR_FATAL, "Q_strncpyz: NULL src" );
	}
	if ( destsize < 1 ) {
		Com_Error( ERR_FATAL,"Q_strncpyz: destsize < 1" );
	}

	strncpy( dest, src, destsize-1 );
    dest[destsize-1] = 0;
}

int Q_stricmpn (const char *s1, const char *s2, int n) {
	int		c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}

		if (c1 != c2) {
			if ( ( c1 >= 'a' ) && ( c1 <= 'z' ) ) {
				c1 -= ('a' - 'A');
			}
			if ( ( c2 >= 'a' ) && ( c2 <= 'z' ) ) {
				c2 -= ('a' - 'A');
			}
			if (c1 < c2) {
				return -1;		// strings less than
			}
         else if ( c1 > c2 ) {
            return 1;      // strings greater than
         }
		}
	} while (c1);

	return 0;		// strings are equal
}

int Q_stricmp (const char *s1, const char *s2) {
	return Q_stricmpn (s1, s2, 99999);
}

void Com_sprintf (char *dest, int size, const char *fmt, ...)
{
	char	bigbuffer[0x10000];
	int		len;
	va_list		argptr;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);
	if (len >= size)
		Com_WPrintf ("Com_sprintf: overflow of %i in %i\n", len, size);
	strncpy (dest, bigbuffer, size-1);
}

void Com_BackslashToSlash( char *str )
   {
	int i;
	int len;
	char *t;

   if ( str )
      {
	   t = str;
	   len = strlen( str );

	   for( i = 0; i < len; i++ )
		   {
		   if ( t[ i ] == '\\' )
			   {
			   t[ i ] = '/';
			   }
		   }
      }
   }

char *Q_CleanStr( char *string ) {
	char*	d;
	char*	s;
	int		c;

	s = string;
	d = string;
	while ((c = *s) != 0 ) {
		if ( Q_IsColorString( s ) ) {
			s++;
		}
		else if ( ( c >= 0x20 ) && ( c <= 0x7E ) ) {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
const char *Info_ValueForKey (const char *s, const char *key)
{
	char	pkey[512];
	static	char value[2][512];	// use two buffers so compares
								// work without stomping on each other
	static	int	valueindex = 0;
	char	*o;

	if ( !s || !key ) {
		return "";
	}

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring" );
	}

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
			return value[valueindex];

		if (!*s)
			return "";
		s++;
	}
}

void Info_RemoveKey (char *s, const char *key)
{
	char	*start;
	char	pkey[512];
	char	value[512];
	char	*o;

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_RemoveKey: oversize infostring" );
	}

	if (strstr (key, "\\"))
	{
		Com_WPrintf ("Can't use a key with a \\\n");
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate (const char *s)
{
	if (strstr (s, "\""))
		return qfalse;
	if (strstr (s, ";"))
		return qfalse;
	return qtrue;
}

void Info_SetValueForKey( char *s, const char *key, const char *value ) {
	char	newi[MAX_INFO_STRING];

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
	}

	if (strchr (key, '\\') || strchr (value, '\\'))
	{
		Com_WPrintf ("Can't use keys or values with a \\\n");
		return;
	}

	if (strchr (key, ';') || strchr (value, ';'))
	{
		Com_WPrintf ("Can't use keys or values with a semicolon\n");
		return;
	}

	if (strchr (key, '\"') || strchr (value, '\"'))
	{
		Com_WPrintf ("Can't use keys or values with a \"\n");
		return;
	}

	Info_RemoveKey (s, key);
	if (!value || !strlen(value))
		return;

	Com_sprintf (newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) > MAX_INFO_STRING)
	{
		Com_WPrintf ("Info string length exceeded\n");
		return;
	}

	strcat (s, newi);
}

/*
===================
Info_NextPair

Used to itterate through all the key/value pairs in an info string
===================
*/
void Info_NextPair( const char **head, char key[MAX_INFO_KEY], char value[MAX_INFO_VALUE] ) {
	char	*o;
	const char	*s;

	s = *head;

	if ( *s == '\\' ) {
		s++;
	}
	key[0] = 0;
	value[0] = 0;

	o = key;
	while ( *s != '\\' ) {
		if ( !*s ) {
			*o = 0;
			*head = s;
			return;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	o = value;
	while ( *s != '\\' && *s ) {
		*o++ = *s++;
	}
	*o = 0;

	*head = s;
}

//----------------------------------------------------------------
// Name:			ParseMapName
// Class:			
//
// Description:		Parses the full mapname to get the mapname, spawn position, and movie name
//
// Parameters:		const char *fullname		- original full mapname
//					const char *mapname			- parsed map name
//					const char *spawnposName	- parsed spawn position name
//					const char *movieName		- parsed movie name
//
// Returns:			bool		- referenced bit
//----------------------------------------------------------------

void ParseMapName( const char *fullName, char *mapName, char *spawnposName, char *movieName )
{
	char fixedName[ MAX_QPATH ];
	char tempMapName[ MAX_QPATH ];
	char tempSpawnposName[ MAX_QPATH ];
	char tempMovieName[ MAX_QPATH ];
	char *spawnposNamePtr;
	char *movieNamePtr;


	// Clean up the map name

	strcpy( fixedName, fullName );

	Com_BackslashToSlash( fixedName );

	// Get the movie name

	movieNamePtr = strchr( fixedName, '#' );

	if ( movieNamePtr )
		strcpy( tempMovieName, movieNamePtr );
	else
		tempMovieName[ 0 ] = '/0';

	// Get the map name

	spawnposNamePtr = strchr( fixedName, '$' );

	if ( spawnposNamePtr )
	{
		Q_strncpyz( tempMapName, fixedName, spawnposNamePtr - fixedName + 1 );
	}
	else if ( movieNamePtr )
	{
		Q_strncpyz( tempMapName, fixedName, movieNamePtr - fixedName + 1 );
	}
	else
	{
		strcpy( tempMapName, fixedName );
	}

	// Get the spawn pos name

	if ( spawnposNamePtr )
	{
		if ( movieNamePtr )
		{
			Q_strncpyz( tempSpawnposName, spawnposNamePtr, movieNamePtr - spawnposNamePtr + 1 );
		}
		else
		{
			strcpy( tempSpawnposName, spawnposNamePtr );
		}
	}
	else
	{
		tempSpawnposName[ 0 ] = '/0';
	}

	// Update all parms

	if ( mapName )
		strcpy( mapName, tempMapName );

	if ( movieName )
		strcpy( movieName, tempMovieName );

	if ( spawnposName )
		strcpy( spawnposName, tempSpawnposName );
}
