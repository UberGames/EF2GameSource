//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/q_shared.h                                    $
// $Revision:: 167                                                            $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:42a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// included first by ALL program modules.  A user mod should never modify
// this file

#ifndef __Q_SHARED_H__
#define __Q_SHARED_H__

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file


#include <gamedefs.h>

#define	DEVELOPER_NAME			"Ritual Entertainment"

#define	QDECL	
// __cdecl		// added for bot code

#ifdef _WIN32

#pragma warning(disable : 4018)     // signed/unsigned mismatch
//#pragma warning(disable : 4032)     // formal parameter 'number' has different type when promoted
#pragma warning(disable : 4051)     // type conversion, possible loss of data
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)     // 'type' : named type definition in parentheses
//#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
//#pragma warning(disable : 4131)     // 'function' : uses old-style declarator
#pragma warning(disable : 4136)     // conversion between different floating-point types
#pragma warning(disable : 4201)     // nonstandard extension used : nameless struct/union
//#pragma warning(disable : 4214)     // nonstandard extension used : bit field types other than int
//#pragma warning(disable : 4220)		// varargs matches remaining parameters
//#pragma warning(disable : 4239)     // nonstandard extension used, conversion from class b to class & b
#pragma warning(disable : 4244)     // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4305)		// truncation from const double to float
//#pragma warning(disable : 4310)		// cast truncates constant value
#pragma warning(disable : 4389)
#pragma warning(disable : 4512)     // 'Class' : assignment operator could not be generated
#pragma warning(disable : 4514)     // unreferenced inline/local function has been removed
//#pragma warning(disable : 4611)     // interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4710)     // did not inline this function
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4996)


// shut up warnings with Intel Compiler
/*ARGSUSED*/
/*NOTREACHED*/
/*VARARGS*/
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32

//#pragma intrinsic( memset, memcpy )

#endif


// this is the define for determining if we have an asm version of a C function
#if (defined _M_IX86 || defined __i386__) && !defined C_ONLY && !defined __sun__
#define id386	1
#else
#define id386	0
#endif

//======================= WIN32 DEFINES =================================

#ifdef WIN32

#define	MAC_STATIC

// buildstring will be incorporated into the version string
#ifdef NDEBUG
#ifdef _M_IX86
#define	CPUSTRING	"win-x86"
#elif defined _M_ALPHA
#define	CPUSTRING	"win-AXP"
#endif
#else
#ifdef _M_IX86
#define	CPUSTRING	"win-x86-debug"
#elif defined _M_ALPHA
#define	CPUSTRING	"win-AXP-debug"
#endif
#endif


#define	PATH_SEP '\\'

#endif

//======================= MAC OS X SERVER DEFINES =====================

#if defined(__MACH__) && defined(__APPLE__)

#define MAC_STATIC

#ifdef __ppc__
#define CPUSTRING	"MacOSXS-ppc"
#elif defined __i386__
#define CPUSTRING	"MacOSXS-i386"
#else
#define CPUSTRING	"MacOSXS-other"
#endif

#define	PATH_SEP	'/'

#define	GAME_HARD_LINKED
#define	CGAME_HARD_LINKED
#define	UI_HARD_LINKED
#define	BOTLIB_HARD_LINKED

#endif

//======================= MAC DEFINES =================================

#ifdef __MACOS__

#define	MAC_STATIC	static

#define	CPUSTRING	"MacOS-PPC"

#define	PATH_SEP ':'

#define	GAME_HARD_LINKED
#define	CGAME_HARD_LINKED
#define	UI_HARD_LINKED
#define	BOTLIB_HARD_LINKED

void Sys_PumpEvents( void );

#endif

//======================= LINUX DEFINES =================================

// the mac compiler can't handle >32k of locals, so we
// just waste space and make big arrays static...
#ifdef __linux__

#define	MAC_STATIC

#ifdef __i386__
#define	CPUSTRING	"linux-i386"
#elif defined __axp__
#define	CPUSTRING	"linux-alpha"
#else
#define	CPUSTRING	"linux-other"
#endif

#define	PATH_SEP '/'

#endif

//=============================================================

#define DATATYPE_SCHAR_MIN		-128
#define DATATYPE_SCHAR_MAX		 127

#define DATATYPE_UCHAR_MIN		 0
#define DATATYPE_UCHAR_MAX		 255

#define DATATYPE_SSHORT_MIN	-32768
#define DATATYPE_SSHORT_MAX	 32767

#define DATATYPE_USHORT_MIN	 0
#define DATATYPE_USHORT_MAX	 65535

#define DATATYPE_SINT_MIN		-2147483648
#define DATATYPE_SINT_MAX		 2147483647

#define DATATYPE_UINT_MIN		 0
#define DATATYPE_UINT_MAX		 4294967295

#define DATATYPE_SLONG_MIN		-2147483648
#define DATATYPE_SLONG_MAX		 2147483647

#define DATATYPE_ULONG_MIN		 0
#define DATATYPE_ULONG_MAX		 4294967295

//======================= C++ DEFINES =================================

#ifdef __cplusplus
extern "C"
   {
#endif

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#define bound(a,minval,maxval)  ( ((a) > (minval)) ? ( ((a) < (maxval)) ? (a) : (maxval) ) : (minval) )

typedef unsigned char 		byte;

#ifdef __cplusplus
typedef int qboolean;
#define qfalse (0)
#define qtrue (!qfalse)
#else
typedef enum {qfalse, qtrue}	qboolean;
#endif

typedef int		qhandle_t;
typedef int		sfxHandle_t;
typedef int		fileHandle_t;
typedef int		clipHandle_t;


#ifndef NULL
#define NULL ((void *)0)
#endif

#define	MAX_QINT			0x7fffffff
#define	MIN_QINT			(-MAX_QINT-1)


// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

// Axis vector indexes
#define AXIS_FORWARD_VECTOR	0
#define AXIS_RIGHT_VECTOR		1
#define AXIS_UP_VECTOR			2

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	256		// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token
#define MAX_ARGUMENTS		512		// max number of arguments for commands

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		8192
#define	BIG_INFO_VALUE		8192

#define	MAX_QPATH			64		// max length of a pathname
#define	MAX_OSPATH			256		// max length of a filesystem pathname

#define	MAX_NAME_LENGTH		32		// max length of a client name

#define	MAX_LIGHTING_GROUPS		32
#define  MAX_STATIC_LOD_MODLES	256

#define MAX_EXTRA_ENTITIES_FROM_GAME  100

typedef enum {
	CENTERPRINT_IMPORTANCE_NORMAL,
	CENTERPRINT_IMPORTANCE_HIGH,
	CENTERPRINT_IMPORTANCE_CRITICAL
} CenterPrintImportance;


// server browser sources
#define AS_LOCAL			0
#define AS_GLOBAL			1
#define AS_FAVORITES		2

// paramters for command buffer stuffing
typedef enum {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
					   	// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
} cbufExec_t;


//
// these aren't needed by any of the VMs.  put in another header?
//
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility


// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
	PRINT_ALL,
	PRINT_DEVELOPER,		// only print when "developer 1"
	PRINT_DEVELOPER_2,		// only print when "developer 2"
	PRINT_WARNING,
	PRINT_ERROR
} printParm_t;


// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_DISCONNECT,				// don't kill server
	ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;

#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16

/*
==============================================================

MATHLIB

==============================================================
*/


typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef struct
{
	float	radius;
	float	height;
	vec3_t	center;
}cylinder_t;

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define NUMVERTEXNORMALS	162
extern	vec3_t	bytedirs[NUMVERTEXNORMALS];

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480

#define TINYCHAR_WIDTH		(SMALLCHAR_WIDTH)
#define TINYCHAR_HEIGHT		(SMALLCHAR_HEIGHT/2)

#define SMALLCHAR_WIDTH		8
#define SMALLCHAR_HEIGHT	16

#define BIGCHAR_WIDTH		16
#define BIGCHAR_HEIGHT		16

#define	GIANTCHAR_WIDTH		32
#define	GIANTCHAR_HEIGHT	48

extern	vec4_t		colorBlack;
extern	vec4_t		colorRed;
extern	vec4_t		colorGreen;
extern	vec4_t		colorBlue;
extern	vec4_t		colorYellow;
extern	vec4_t		colorMagenta;
extern	vec4_t		colorCyan;
extern	vec4_t		colorWhite;
extern	vec4_t		colorLtGrey;
extern	vec4_t		colorMdGrey;
extern	vec4_t		colorDkGrey;

#define Q_COLOR_ESCAPE	'^'
#define Q_IsColorString(p)	( p && ( *(p) == Q_COLOR_ESCAPE ) && ( *((p)+1) ) && ( *((p)+1) != Q_COLOR_ESCAPE ) )

#define COLOR_BLACK		'0'
#define COLOR_RED		'1'
#define COLOR_GREEN		'2'
#define COLOR_YELLOW	'3'
#define COLOR_BLUE		'4'
#define COLOR_CYAN		'5'
#define COLOR_MAGENTA	'6'
#define COLOR_WHITE		'7'
#define COLOR_NONE		'8'
#define ColorIndex(c)	( ( (c) - '0' ) & 7 )

#define S_COLOR_BLACK	"^0"
#define S_COLOR_RED		"^1"
#define S_COLOR_GREEN	"^2"
#define S_COLOR_YELLOW	"^3"
#define S_COLOR_BLUE	"^4"
#define S_COLOR_CYAN	"^5"
#define S_COLOR_MAGENTA	"^6"
#define S_COLOR_WHITE	"^7"

extern vec4_t	g_color_table[8];

#define	MAKERGB( v, r, g, b ) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA( v, r, g, b, a ) v[0]=r;v[1]=g;v[2]=b;v[3]=a

#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )

struct cplane_s;

extern	vec3_t	vec3_origin;
extern	vec3_t	axisDefault[3];

#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

#ifndef __Q_FABS__
#define __Q_FABS__
#endif

float Q_fabs( float f );
float Q_rsqrt( float f );		// reciprocal square root

#define SQRTFAST( x ) ( 1.0f / Q_rsqrt( x ) )

signed char ClampChar( int i );
signed short ClampShort( int i );

double		dEpsilon( void );
double 		dIdentity( void );
double		dSign( const double number );
double		dClamp( const double value, const double min, const double max );
double		dDistance (const double value1, const double value2 );
qboolean	dCloseEnough( const double value1, const double value2, const double epsilon );
qboolean	dSmallEnough( const double value, const double epsilon );

float		fEpsilon( void );
float 		fIdentity( void );
float		fSign( const float number );
float		fClamp( const float value, const float min, const float max );
float		fDistance (const float value1, const float value2 );
qboolean	fCloseEnough( const float value1, const float value2, const float epsilon );
qboolean	fSmallEnough( const float value, const float epsilon );

int			iSign( const int number);
int			iClamp( const int value, const int min, const int max);


// this isn't a real cheap function to call!
int DirToByte( const vec3_t dir );
void ByteToDir( int b, vec3_t dir );

#if	1

#define DotProduct(x,y)			( ((x)[0]*(y)[0]) + ((x)[1]*(y)[1]) + ((x)[2]*(y)[2]) )
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	((o)[0]= (v)[0] + ((b)[0]*(s)), (o)[1] = (v)[1] + ((b)[1]*(s)), (o)[2] = (v)[2] + ((b)[2]*(s)))
#define Round(a)				(int)(a + 0.5f)
#else

#define DotProduct(x,y)			_DotProduct(x,y)

#ifdef MSVC_BUILD
//intel change to accomodate manual CPU dispatch.  if using intel compiler, this header
//show up in q_math.c instead of here.  
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
#endif

#define VectorCopy(a,b)			_VectorCopy(a,b)
#define	VectorScale(v, s, o)	_VectorScale(v,s,o)
#define	VectorMA(v, s, b, o)	_VectorMA(v,s,b,o)

#endif

#ifdef __LCC__
#ifdef VectorCopy
#undef VectorCopy
// this is a little hack to get more efficient copies
typedef struct {
	float	v[3];
} vec3struct_t;
#define VectorCopy(a,b)	*(vec3struct_t *)b=*(vec3struct_t *)a;
#endif
#endif

#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define Vector4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define OrientClear( orient )   \
   ( VectorSet( orient.origin, 0, 0, 0 ), \
   VectorSet( orient.axis[ 0 ], 1, 0, 0 ), \
   VectorSet( orient.axis[ 1 ], 0, 1, 0 ), \
   VectorSet( orient.axis[ 2 ], 0, 0, 1 ) )
#define OrientCopy( a, b )   \
   ( VectorCopy( (a).origin, (b).origin ), \
   VectorCopy( (a).axis[ 0 ], (b).axis[ 0 ] ), \
   VectorCopy( (a).axis[ 1 ], (b).axis[ 1 ] ), \
   VectorCopy( (a).axis[ 2 ], (b).axis[ 2 ] ) )

#define QuatSet( q, x, y, z, w ) ((q)[0]=(x),(q)[1]=(y),(q)[2]=(z),(q)[3]=(w))
#define QuatCopy( a,b ) ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define uint_cast(X) ( *(unsigned int *) &(X) )
#define int_cast(X) ( *(int *) &(X) )
#define IsNegative(X) ( uint_cast(X) >> 31 )
#define USES_CLAMP_ZERO const unsigned s_clamp0table[2] = { 0xFFFFFFFF, 0x00000000 }
#define ClampZero(X) ( uint_cast(X) &= s_clamp0table[IsNegative(X)] )
#define ClampNormalFloat255Byte(dst,X) { \
   float _f_ = X; \
   unsigned mask = ~ ( (int) ( uint_cast(_f_) - 1 ) >> 31 ); \
   \
   uint_cast(_f_) += (unsigned) ( 0x04000000 - 0x437F0000 ); \
   uint_cast(_f_) &= ( (int) uint_cast(_f_) ) >> 31; \
   uint_cast(_f_) += (unsigned) 0x437F0000; \
   uint_cast(_f_) &= mask; \
   dst = (unsigned char) _f_; \
}

#define SetHighest(type, dst,upperbound) { \
   type _ff_=dst-upperbound;\
   ClampZero ( _ff_ );\
   dst = dst - _ff_;\
}

#define SetLowest(type, dst,lowerbound) { \
   type _ff_= dst - lowerbound;\
   ClampZero ( _ff_ );\
   dst = lowerbound + _ff_;\
}

#define SetLowestFloat(dst,l) ( dst = ( ( dst < l ) ? l : dst ) )
#define SetHighestFloat(dst,l) ( dst = ( ( dst > l ) ? l : dst ) )
//#define SetLowestFloat(dst,l) SetLowest ( float, dst, l )
//#define SetHighestFloat(dst,l) SetHighest ( float, dst, l )
#define SetLowestInt(dst,l) SetLowest ( int, dst, l )
#define SetHighestInt(dst,l) SetHighest ( int, dst, l )

#ifdef __cplusplus
inline bool VectorFromString ( const char *ss, vec3_t v ) {
   return sscanf ( ss, "%f %f %f", &v[0], &v[1], &v[2] ) == 3;
}
#else
#define VectorFromString(ss,v)   ( sscanf ( (ss), "%f %f %f", &(v)[0], &(v)[1], &(v)[2] ) == 3 )
#endif

#define	SnapVector(v) {v[0]=(int)v[0];v[1]=(int)v[1];v[2]=(int)v[2];}

// just in case you do't want to use the macros
vec_t _DotProduct( const vec3_t v1, const vec3_t v2 );
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorCopy( const vec3_t in, vec3_t out );
void _VectorScale( const vec3_t in, float scale, vec3_t out );
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc );

unsigned ColorBytes3 (float r, float g, float b);
unsigned ColorBytes4 (float r, float g, float b, float a);

float NormalizeColor( const vec3_t in, vec3_t out );

float RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void ClearBounds( vec3_t mins, vec3_t maxs );
qboolean BoundsClear( const vec3_t mins, const vec3_t maxs );
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );
int VectorCompare( const vec3_t v1, const vec3_t v2 );
vec_t VectorLength( const vec3_t v );
vec_t VectorLengthSquared( const vec3_t v );
vec_t Distance( const vec3_t p1, const vec3_t p2 );
vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 );
void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );
vec_t VectorNormalize (vec3_t v);		// returns vector length
void VectorNormalizeFast(vec3_t v);		// does NOT return vector length, uses rsqrt approximation
vec_t VectorNormalize2( const vec3_t v, vec3_t out );
void VectorInverse (vec3_t v);
void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out );
void VectorRotate( const vec3_t in, const vec3_t matrix[3], vec3_t out );
int Q_log2(int val);
unsigned short NormalToLatLong( const vec3_t normal );


int		Q_rand( int *seed );
float	Q_random( int *seed );
float	Q_crandom( int *seed );

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0f * (random() - 0.5f))

float randomrange( float min, float max );
float crandomrange( float min, float max );
float grandom( float average, float deviation );
float erandom( float mean );

void AccumulateTransform( vec3_t dstLocation, vec3_t dstAxes[3],
						  const vec3_t childLocation, const vec3_t childAxes[3],
						  const vec3_t parentLocation, const vec3_t parentAxes[3] );
void AccumulatePosition( vec3_t dstOrigin, const vec3_t childOrigin, const vec3_t parentOrigin, const vec3_t parentAxes[3] );

void vectoangles( const vec3_t value1, vec3_t angles);
float vectoyaw( const vec3_t vec );

//FIXME
// get rid of all references to AnglesToMat
#define AnglesToMat AnglesToAxis

void AxisClear( vec3_t axis[3] );
void AxisCopy( const vec3_t in[3], vec3_t out[3] );

void SetPlaneSignbits( struct cplane_s *out );
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);

float	AngleMod(float a);
float	LerpAngle (float from, float to, float frac);
float LerpAngleFromCurrent (float from, float to, float current, float frac);
float	AngleSubtract( float a1, float a2 );
void	AnglesSubtract( const vec3_t v1, const vec3_t v2, vec3_t v3 );

float AngleNormalize360 ( float angle );
float AngleNormalize180 ( float angle );
float AngleNormalizeArbitrary ( const float angle, const float minimumAngle );
float AngleDelta ( float angle1, float angle2 );

qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );
void RotateAroundDirection( vec3_t axis[3], float yaw );
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up );

float noise(float vec[], int len);
float noise1(float arg);
float noise2(const float arg[2]);
float noise3(const float arg[3]);
// perpendicular vector could be replaced by this

void  R_ConcatRotations ( const float in1[3][3], const float in2[3][3], float out[3][3] );
void  R_ConcatTransforms ( const float in1[3][4], const float in2[3][4], float out[3][4] );
float	anglemod(float a);
float angledist( float ang );
int   BoxOnPlaneSide2 ( const vec3_t emins, const vec3_t emaxs, const struct cplane_s *p );
void  CalculateRotatedBounds( const vec3_t angles, vec3_t mins, vec3_t maxs );
void  CalculateRotatedBounds2( float trans[3][3], vec3_t mins, vec3_t maxs );
int   BoundingBoxToInteger( const vec3_t mins, const vec3_t maxs );
void  IntegerToBoundingBox( int num, vec3_t mins, vec3_t maxs );
void  MatrixTransformVector( const vec3_t in, const float mat[ 3 ][ 3 ], vec3_t out );
void  Matrix4TransformVector( const vec3_t in, const float mat[ 4 ][ 4 ], vec3_t out );
//void  MatrixToEulerAngles( float mat[ 3 ][ 3 ], vec3_t ang );
void  TransposeMatrix( const float in[ 3 ][ 3 ], float out[ 3 ][ 3 ] );
void  OrthoNormalize( float mat[3][3] );
float NormalizeQuat( float q[ 4 ] );
//void  MatToQuat( float srcMatrix[ 3 ][ 3 ], float destQuat[ 4 ] );
void  RotateAxis( const float axis[ 3 ], float angle, float q[ 4 ] );
void  MultQuat( const float q1[ 4 ], const float q2[ 4 ], float out[ 4 ] );
//void  QuatToMat( float q[ 4 ], float m[ 3 ][ 3 ] );
void  SlerpQuaternion( const float from[ 4 ], const float to[ 4 ], float t, float res[ 4 ] );
//void  EulerToQuat( float ang[ 3 ], float q[ 4 ] );
int	PlaneTypeForNormal ( const vec3_t normal );



//=============================================

float Com_Clamp( float min, float max, float value );

void COM_FilePath (const char *in, char *out);
void COM_FileBase (const char *in, char *out);
void COM_FileName (const char *in, char *out);

const char	*COM_SkipPath( const char *pathname );
void	      COM_StripExtension( const char *in, char *out );
void	      COM_DefaultExtension( char *path, int maxSize, const char *extension );
void        Com_BackslashToSlash( char *str );

void	      COM_BeginParseSession( void );
int         COM_GetCurrentParseLine( void );
const char	*COM_Parse( const char **data_p );
char	      *COM_ParseExt( char **data_p, qboolean allowLineBreak );
const char  *COM_GetToken(const char **data_p, qboolean crossline);
int			COM_GetParseLineNumber(void);

// data is an in/out parm, returns a parsed out token

void	COM_MatchToken( char**buf_p, char *match );

qboolean SkipBracedSection (char **program);
void SkipRestOfLine ( char **data );

void Parse1DMatrix (char **buf_p, int x, float *m);
void Parse2DMatrix (char **buf_p, int y, int x, float *m);
void Parse3DMatrix (char **buf_p, int z, int y, int x, float *m);

void	Com_sprintf (char *dest, int size, const char *fmt, ...);


// mode parm for FS_FOpenFile
typedef enum {
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPEND_SYNC
} fsMode_t;

typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

//=============================================

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );

// portable case insensitive compare
int	Q_stricmp (const char *s1, const char *s2);
int	Q_strncmp (const char *s1, const char *s2, int n);
int	Q_stricmpn (const char *s1, const char *s2, int n);
char	*Q_strlwr( char *s1 );
char	*Q_strupr( char *s1 );
char	*Q_strrchr( const char* string, int c );

// buffer size safe library replacements
void	Q_strncpyz( char *dest, const char *src, int destsize );
void	Q_strcat( char *dest, int size, const char *src );
char *Q_CleanStr( char *string );

//=============================================

extern qboolean bigendian;

short	BigShort(short l);
short	LittleShort(short l);
int	BigLong (int l);
int	LittleLong (int l);
float	BigFloat (float l);
float	LittleFloat (float l);
unsigned short	LittleUnsignedShort(unsigned short l);
unsigned short	BigUnsignedShort(unsigned short l);

void	Swap_Init (void);
const char *va(const char *format, ...);

//=============================================

int MusicMood_NameToNum( const char * name );
const char * MusicMood_NumToName( int num );
int EAXMode_NameToNum( const char * name );
const char * EAXMode_NumToName( int num );

unsigned int GenerateHashForName( const char* name, qboolean caseSensitive, unsigned int maxHash );

int PlayerStat_NameToNum( const char * name );
const char * PlayerStat_NumToName( int num );

//
// key / value info strings
//
const char *Info_ValueForKey( const char *s, const char *key );
void        Info_RemoveKey( char *s, const char *key );
void        Info_SetValueForKey( char *s, const char *key, const char *value );
qboolean    Info_Validate( const char *s );
void        Info_NextPair( const char **s, char key[MAX_INFO_KEY], char value[MAX_INFO_VALUE] );

void ParseMapName( const char *fullName, char *mapName, char *spawnposName, char *movieName );

// this is only here so the functions in q_shared.c and bg_*.c can link
void	Com_Error( int level, const char *error, ... );
void	Com_Printf( const char *msg, ... );
void	Com_WPrintf( const char *msg, ... );

void	Com_WidgetPrintf ( const char *widgetName, const char *fmt, ...	);

/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define	CVAR_ARCHIVE		1	// set to cause it to be saved to vars.rc
								      // used for system variables, not for player
								      // specific configurations
#define	CVAR_USERINFO		2	// sent to server on connect or change
#define	CVAR_SERVERINFO	4	// sent in response to front end requests
#define	CVAR_SYSTEMINFO	8	// these cvars will be duplicated on all clients
#define	CVAR_INIT			16	// don't allow change from console at all,
								      // but can be set from the command line
#define	CVAR_LATCH			32	// will only change when C code next does
								      // a Cvar_Get(), so it can't be changed
								      // without proper initialization.  modified
								      // will be set, even though the value hasn't
								      // changed yet
#define	CVAR_ROM			   64	// display only, cannot be set by user at all
#define	CVAR_USER_CREATED	128	// created by a set command
#define	CVAR_TEMP			256	// can be set even when cheats are disabled, but is not archived
#define  CVAR_CHEAT			512	// can not be changed if cheats are disabled
#define  CVAR_NORESTART		1024	// do not clear when a cvar_restart is issued
#define  CVAR_RESETSTRING  2048  // force the cvar's reset string to be set
#define	CVAR_SOUND_LATCH	4096	// specifically for sound will only change
#define	CVAR_UNDEFINED		8192
                                 // when C code next does a Cvar_Get(), so it
                                 // can't be changed without proper initialization.
                                 // modified will be set, even though the value hasn't
								         // changed yet

#define CVAR_UNDEFINED_STRING "UNDEFINED"

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s {
	char		*name;
	char		*string;
	char		*resetString;		// cvar_restart will reset to this value
	char		*latchedString;		// for CVAR_LATCH vars
	int			flags;
	qboolean	modified;			// set each time the cvar is changed
	int			modificationCount;	// incremented each time the cvar is changed
	float		value;				// atof( string )
	int			integer;			// atoi( string )
	struct cvar_s *next;
	qboolean	defaultSet;			// set when setd creates the default value
} cvar_t;

#define	MAX_CVAR_VALUE_STRING	256

typedef int	cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct {
	cvarHandle_t	handle;
	int			modificationCount;
	float		value;
	int			integer;
	char		string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "surfaceflags.h"			// shared with the q3map utility

// plane types are used to speed some tests
// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
#define	PLANE_NON_AXIAL	3


// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s {
	vec3_t	normal;
	float	dist;
	byte	type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
	byte	signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
	byte	pad[2];
} cplane_t;

typedef struct
   {
   qboolean valid;
   int      surface;
	int		bone;
   vec3_t   position;
   vec3_t   normal;
   float    damage_multiplier;
   } tikimdl_intersection_t;

// a trace is returned when a box is swept through the world
typedef struct {
	qboolean	         allsolid;	   // if true, plane is not valid
	qboolean	         startsolid;	   // if true, the initial point was in a solid area
	float		         fraction;	   // time completed, 1.0 = didn't hit anything
	vec3_t	         endpos;		   // final position
	cplane_t	         plane;		   // surface normal at impact, transformed to world space
	int		         surfaceFlags;	// surface hit
	int		         contents;	   // contents on other side of surface hit
	int		         entityNum;	   // entity the contacted surface is a part of
   struct gentity_s	*ent;		      // Pointer to entity hit
	tikimdl_intersection_t intersect; // set if the trace hit a specific polygon
} trace_t;

typedef struct {
	vec3_t		boxmins, boxmaxs;// enclose the test object along entire move
	const float	*mins;
	const float *maxs;	// size of the moving object
	const float	*start;
	vec3_t		end;
	trace_t		trace;
	int			passEntityNum;
	int			contentmask;
   qboolean    cylinder;
	qboolean		fulltrace;
} moveclip_t;

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD


// markfragments are returned by CM_MarkFragments()
typedef struct {
	int		firstPoint;
	int		numPoints;

	vec3_t	normal;
	float		dist;
} markFragment_t;

typedef struct {
	vec3_t		origin;
	vec3_t		axis[3];
} orientation_t;

typedef struct {
   int      time;                   // real time of day
   int      serverTime;             // time in the game
   int		totalGameTime;			// total time playing the game
   char		comment[ MAX_QPATH ];   // user comment
   char     mapName[ MAX_QPATH ];   // name of map
   char     saveName[ MAX_QPATH ];  // local name of savegame
} savegamestruct_t;

//=====================================================================

// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE	1
#define	KEYCATCH_UI			2
#define	KEYCATCH_MESSAGE	4
#define KEYCATCH_HUD		8


// sound channels
// channel 0 never willingly overrides
// other channels will allways override a playing sound on that channel
typedef enum {
	CHAN_AUTO,
	CHAN_LOCAL,
	CHAN_WEAPON,
	CHAN_VOICE,
	CHAN_ITEM,
	CHAN_BODY,
	CHAN_DIALOG,
	CHAN_DIALOG_SECONDARY,
	CHAN_WEAPONIDLE,
	CHAN_MENU,
	CHAN_CINEMATIC,
	CHAN_MUSIC,
	CHAN_TAUNT,
	CHAN_MENU2,
	CHAN_COMBAT1 = 20,
	CHAN_COMBAT2 = 21,
	CHAN_COMBAT3 = 22,
	CHAN_COMBAT4 = 23
} soundChannel_t;

#define DEFAULT_MIN_DIST  -1.0f
#define DEFAULT_VOL  -1.0f

#define CONTEXT_WIDE_MIN_DIST 1024
#define LEVEL_WIDE_MIN_DIST_CUTOFF  10000
#define LEVEL_WIDE_MIN_DIST  1000000	// full volume the entire level
#define LEVEL_WIDE_STRING "levelwide"

#define  SOUND_SYNCH             0x1
#define  SOUND_SYNCH_FADE        0x2
#define  SOUND_RANDOM_PITCH_20   0x4
#define  SOUND_RANDOM_PITCH_40   0x8
#define  SOUND_LOCAL_DIALOG      0x10

typedef enum
   {
   mood_none,
   mood_normal,
   mood_action,
   mood_suspense,
   mood_mystery,
   mood_success,
   mood_failure,
   mood_surprise,
   mood_special,
   mood_aux1,
   mood_aux2,
   mood_aux3,
   mood_aux4,
   mood_aux5,
   mood_aux6,
   mood_aux7,
   mood_totalnumber
   } music_mood_t;

typedef enum
   {
   eax_generic,
   eax_paddedcell,
   eax_room,
   eax_bathroom,
   eax_livingroom,
   eax_stoneroom,
   eax_auditorium,
   eax_concerthall,
   eax_cave,
   eax_arena,
   eax_hangar,
   eax_carpetedhallway,
   eax_hallway,
   eax_stonecorridor,
   eax_alley,
   eax_forest,
   eax_city,
   eax_mountains,
   eax_quarry,
   eax_plain,
   eax_parkinglot,
   eax_sewerpipe,
   eax_underwater,
   eax_drugged,
   eax_dizzy,
   eax_psychotic,
   eax_totalnumber
   } eax_mode_t;

#define LIP_SYNC_HZ  20.0f

typedef enum {
	MORPH_CHAN_NONE,
	MORPH_CHAN_MOUTH,
	MORPH_CHAN_BROW,
   MORPH_CHAN_LEFT_BROW,
	MORPH_CHAN_RIGHT_BROW,
	MORPH_CHAN_EYES,
	MORPH_CHAN_LEFT_LID,
	MORPH_CHAN_RIGHT_LID
} morphChannel_t;

/* #define MAX_EXPRESSION_NAME_LENGTH 32

typedef struct
{
   int         morph_index;
   float       percent;
} dtikimorphtarget_t;

typedef struct
{
	char			alias[ MAX_EXPRESSION_NAME_LENGTH ];
   int         num_morph_targets;
   int         ofs_morph_targets;
} dtikiexpression_t; */

/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define	SNAPFLAG_RATE_DELAYED	1
#define	SNAPFLAG_NOT_ACTIVE		2	// snapshot used during connection and for zombies
#define  SNAPFLAG_SERVERCOUNT	   4	// toggled every map_restart so transitions can be detected

//
// per-level limits
//
#define	MAX_CLIENTS			128		// absolute limit
#define MAX_LOCATIONS		64

#define	GENTITYNUM_BITS		10		// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values thatare going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE		(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD		(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)

//
// Some of these are sent over the net with a fixed number of bits so they cannot be blindly increased
//

#define  MAX_MODEL_BITS			9
#define	 MAX_MODELS				(1 << MAX_MODEL_BITS) // 512		

#define  MAX_IMAGES           128
#define  IMAGE_BITS			  7	

#define  MAX_ACTIVE_ITEMS     8
#define  MAX_INVENTORY        32
#define  MAX_AMMO             16
#define  MAX_ITEMS            (MAX_INVENTORY + MAX_AMMO + 16)
#define  MAX_AMMOCOUNT        16
#define  MAX_SOUNDS_BITS      9 //Was 8
#define	 MAX_SOUNDS			   (1<<MAX_SOUNDS_BITS)
#define  MAX_LIGHTSTYLES      32
#define  MAX_ARENA_INFO       32
#define  MAX_TEAM_INFO        64
#define  MAX_ARCHETYPES		  383
#define  MAX_OBJECTIVE_NAMES  5
#define  MAX_GENERAL_STRINGS  256

#define	MAX_CONFIGSTRINGS	2300 //Was 1800

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)
#define	CS_NAME     		2		// A descriptive name of the current level

#define	MAX_GAMESTATE_CHARS	36000
typedef struct {
	int			stringOffsets[MAX_CONFIGSTRINGS];
	char		stringData[MAX_GAMESTATE_CHARS];
	int			dataCount;
} gameState_t;

#define Square(x) ((x)*(x))   // added for bot code

typedef enum {
   GT_SINGLE_PLAYER,
   GT_MULTIPLAYER,
   GT_BOT_SINGLE_PLAYER,

   // BOTLIB =================   added to get bot code up, these are set by querying multiplayermanager when bot first loads
	GT_FFA,				// free for all
	GT_TOURNAMENT,		// one on one tournament

	//-- team games go after this --

	GT_TEAM,			// team deathmatch
	GT_CTF,				// capture the flag
#ifdef MISSIONPACK
	GT_1FCTF,			// tikitech oneflag
	GT_OBELISK,			// tikitech destruction
	GT_HARVESTER,
#endif
// ============================

	GT_MAX_GAME_TYPE
} gametype_t;


//=========================================================

typedef enum
   {
	STAT_HEALTH,
	STAT_DEAD_YAW,				// cleared each frame
	STAT_AMMO_LEFT,				// ammo in current weapon in left hand or primary ammo in 2 handed weapons
	STAT_CLIPAMMO_LEFT,			// ammo in left weapon clip
	STAT_NUM_SHOTS_LEFT,		// amount of shots for the left hand weapon or primary fire.
	STAT_MAX_NUM_SHOTS_LEFT,	// max amount of shots for the left hand weapon or primary fire.
	STAT_AMMO_RIGHT,			// ammo in current weapon in right hand or alternate ammo in 2 handed weapons
	STAT_CLIPAMMO_RIGHT,		// ammo in right weapon clip
	STAT_NUM_SHOTS_RIGHT,		// amount of shots for the right hand or alternate fire
	STAT_ARMOR_LEVEL,			// amount of armor the player has
	STAT_MAXAMMO_LEFT,			// maxammo for left weapon
	STAT_MAXAMMO_RIGHT,			// maxammo for right weapon
	STAT_MAXCLIPAMMO_LEFT,		// maxammo in left weapon clip
	STAT_MAXCLIPAMMO_RIGHT,		// maxammo in right weapon clip
	STAT_AMMO_TYPE1,			// ammount of ammo type 1
	STAT_AMMO_TYPE2,			// ammount of ammo type 2
	STAT_AMMO_TYPE3,			// ammount of ammo type 3
	STAT_AMMO_TYPE4,			// ammount of ammo type 4
	STAT_LAST_PAIN,				// Last amount of damage the player took
	STAT_ACCUMULATED_PAIN,		// Accumulated damage
	STAT_BOSSHEALTH,			// if we are fighting a boss, how much health he currently has
	STAT_BOSSNAME_CONFIGINDEX,	// this is the index of the config string that has the boss name
	STAT_CINEMATIC,				// This is set when we go into cinematics
	STAT_ADDFADE,				// This is set when we need to do an addblend for the fade
	STAT_LETTERBOX,				// This is set to the fraction of the letterbox
	STAT_POWERUPTIME,			// The time the powerup has remaining

	STAT_WEAPON_GENERIC1,
	STAT_WEAPON_GENERIC2,

	STAT_ITEMICON,
	STAT_ITEMTEXT,

	STAT_VOTETEXT,

	// Stats for the Mission Status Screen
	STAT_ENEMIES_KILLED,		// Number of enemies the player has killed on this level
	STAT_TEAMMATES_KILLED,		// Number of teammates the player has killed on this level.
	STAT_SHOTS_FIRED,			// Number of shots the player has fired on this level
	STAT_SHOTS_HIT,				// Number of shots that have hit an actor
	STAT_ACCURACY,				// Accuracy
	STAT_MISSION_DURATION,		// How much time has been spent on this level

	STAT_GENERIC,				// Stat for generic use

	STAT_NUM_OBJECTIVES,		// The number objectives for the level
	STAT_COMPLETE_OBJECTIVES,	//The number of completed objectives for the level
	STAT_FAILED_OBJECTIVES,		// The number of failed objectives for the level.
	STAT_INCOMPLETE_OBJECTIVES, // The number of incomplete objectives for the level.

	STAT_SPECIALMOVETIMER,		// Time for the special move timer
	STAT_POINTS,				// Number of points the player has

	STAT_SECRETS_TOTAL,			// Total Number of Secrets in the Level
	STAT_SECRETS_FOUND,			// Number of secrets uncovered by the player
	STAT_ITEMS_TOTAL,			// Total Number of Items in the Level
	STAT_ITEMS_FOUND,			// Number of Items found by the player

	STAT_RED_TEAM_SCORE,
	STAT_BLUE_TEAM_SCORE,
	//STAT_ARENA,					// Current arena id number the player is in
	STAT_TEAM,					// Current team number
	//STAT_QUEUE_PLACE,			// Place in the Queue for Deathmatch
	STAT_SCORE,					// Number of kills
	STAT_KILLS,				// Number of deaths
	STAT_DEATHS,				// Number of deaths
//	STAT_TIMELEFT_MINUTES,		// Timeleft in minutes
	STAT_TIMELEFT_SECONDS,		// Timeleft in seconds
	STAT_WON_MATCHES,			// Num matches won (for arena mode)
	STAT_LOST_MATCHES,			// Num matches lost (for arena mode)

	STAT_MP_GENERIC1,
	STAT_MP_GENERIC2,
	STAT_MP_GENERIC3,
	STAT_MP_GENERIC4,
	STAT_MP_GENERIC5,
	STAT_MP_GENERIC6,
	STAT_MP_GENERIC7,
	STAT_MP_GENERIC8,

	STAT_MP_SPECTATING_ENTNUM,

	STAT_MP_MODE_ICON,
	STAT_MP_TEAM_ICON,
	STAT_MP_TEAMHUD_ICON,
	STAT_MP_OTHERTEAM_ICON,
	STAT_MP_SPECIALTY_ICON,
	STAT_MP_HOLDABLEITEM_ICON,
	STAT_MP_RUNE_ICON,
	STAT_MP_POWERUP_ICON,

	STAT_MP_AWARD_ICON,
	STAT_MP_AWARD_COUNT,

	STAT_MP_STATE,

	STAT_LAST_STAT				// Last stat
		   
   } playerstat_t;

typedef enum
{
	SPECTATOR_TYPE_NONE,
	SPECTATOR_TYPE_NORMAL,
	SPECTATOR_TYPE_FREEFORM,
	SPECTATOR_TYPE_FOLLOW,
	SPECTATOR_TYPE_ANY
} SpectatorTypes;

// bit field limits

// If MAX_STATS is raised the transmitting of stats across the networking layer needs to be fixed
#define	MAX_STATS				96
#define	MAX_PERSISTANT			16
#define	MAX_POWERUPS			16
#define	MAX_WEAPONS				16

#define	MAX_PS_EVENTS			2


//========================================================================
//
// Flags for Objective States
//
//========================================================================
#define OBJECTIVE1_SHOW     ( 1<<0 )
#define OBJECTIVE1_COMPLETE ( 1<<1 )
#define OBJECTIVE1_FAILED   ( 1<<2 )

#define OBJECTIVE2_SHOW     ( 1<<3 )
#define OBJECTIVE2_COMPLETE ( 1<<4 )
#define OBJECTIVE2_FAILED   ( 1<<5 )

#define OBJECTIVE3_SHOW     ( 1<<6 )
#define OBJECTIVE3_COMPLETE ( 1<<7 )
#define OBJECTIVE3_FAILED   ( 1<<8 )

#define OBJECTIVE4_SHOW     ( 1<<9 )
#define OBJECTIVE4_COMPLETE ( 1<<10 )
#define OBJECTIVE4_FAILED   ( 1<<11 )

#define OBJECTIVE5_SHOW     ( 1<<12 )
#define OBJECTIVE5_COMPLETE ( 1<<13 )
#define OBJECTIVE5_FAILED   ( 1<<14 )

#define OBJECTIVE6_SHOW     ( 1<<15 )
#define OBJECTIVE6_COMPLETE ( 1<<16 )
#define OBJECTIVE6_FAILED   ( 1<<17 )

#define OBJECTIVE7_SHOW     ( 1<<18 )
#define OBJECTIVE7_COMPLETE ( 1<<19 )
#define OBJECTIVE7_FAILED   ( 1<<20 )

#define OBJECTIVE8_SHOW     ( 1<<21 )
#define OBJECTIVE8_COMPLETE ( 1<<22 )
#define OBJECTIVE8_FAILED   ( 1<<23 )

#define OBJECTIVE1 1
#define OBJECTIVE2 2
#define OBJECTIVE3 3
#define OBJECTIVE4 4
#define OBJECTIVE5 5
#define OBJECTIVE6 6
#define OBJECTIVE7 7
#define OBJECTIVE8 8


//========================================================================
//
// Flags for Information States
//
//========================================================================
#define INFORMATION1_SHOW ( 1<<0 )
#define INFORMATION2_SHOW ( 1<<1 )
#define INFORMATION3_SHOW ( 1<<2 )
#define INFORMATION4_SHOW ( 1<<3 )
#define INFORMATION5_SHOW ( 1<<4 )
#define INFORMATION6_SHOW ( 1<<5 )
#define INFORMATION7_SHOW ( 1<<6 )
#define INFORMATION8_SHOW ( 1<<7 )

#define INFORMATION1 1
#define INFORMATION2 2
#define INFORMATION3 3
#define INFORMATION4 4
#define INFORMATION5 5
#define INFORMATION6 6
#define INFORMATION7 7
#define INFORMATION8 8


#define MISSION_FAILED  1
#define MISSION_SUCCESS 2
//========================================================================


// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
typedef struct playerState_s {
	int			commandTime;	// cmd->serverTime of last executed command
	int			pm_type;
	int			bobCycle;		// for view bobbing and footstep generation
	int			pm_flags;		// ducked, jump_held, etc
	int			pm_time;
   int         pm_runtime;    // used to keep track of how long player has been running
	/*	jhefty/jwaters -- another strafe-jump fix
   	int			pm_landtime;	// used to record land time, to prevent strafe jumping proper-like
    */
	vec3_t		origin;
	vec3_t		velocity;
	int			gravity;
	int			speed;
	int			jumpvelocity;
	int			crouchjumpvelocity;
	qboolean	crouchjumpset;
	int			pm_stopspeed;
	int			pm_airaccelerate;
	int			pm_wateraccelerate;
	int			pm_friction;
	int			pm_waterfriction;
	int			pm_accelerate;

	int			pm_defaultviewheight;

	int			delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters
	//lean amount
	float		leanDelta;

	int			groundEntityNum;// ENTITYNUM_NONE = in air
	qboolean	   walking;
	qboolean	   jumped;
	qboolean	   groundPlane;
	qboolean		instantJump;
	qboolean		strafeJumpingAllowed;
   int         feetfalling;
   vec3_t      falldir;
	trace_t		groundTrace;

	int			clientNum;		// ranges from 0 to MAX_CLIENTS-1

	vec3_t		viewangles;		// for fixed views
	int			viewheight;

	int			stats[MAX_STATS];
   int         activeItems[MAX_ACTIVE_ITEMS];
   int         inventory_name_index[MAX_INVENTORY];
   int			inventory_weapon_ammo_index[MAX_INVENTORY];
   int			inventory_weapon_required_ammo[MAX_INVENTORY];
   int			ammo_in_clip[MAX_INVENTORY];
   int         ammo_name_index[MAX_AMMO];
   int         ammo_amount[MAX_AMMOCOUNT];
   int         max_ammo_amount[MAX_AMMOCOUNT];

   int         current_music_mood;
   int         fallback_music_mood;
	float		   music_volume;
	float		   music_volume_fade_time;
	qboolean	   allowMusicDucking;

	int			reverb_type;
	float			reverb_level;

  	float		   blend[4];		   // rgba full screen effect
   float       fov;              // fov of the player

	vec3_t		camera_origin;    // origin for camera view
	vec3_t		cameraFocalPoint;
	qboolean	useCameraFocalPoint;
   vec3_t      camera_angles;    // angles for camera view
   float       camera_time;      // time to switch between camera and normal view

   vec3_t      camera_offset;    // angular offset for camera
   vec3_t      damage_angles;    // these angles are added directly to the view, without lerping
   int         camera_flags;     // third-person camera flags
	vec3_t      vehicleoffset;
	qboolean    in_vehicle;

	int			dialogEntnum;
	int			dialogSoundIndex;
	int			dialogTextSoundIndex;

	int				objectiveNameIndex;
	unsigned int	objectiveStates;
	unsigned int	informationStates;

	unsigned int  missionStatus;

	unsigned int	viewMode;

	// not communicated over the net at all
	int			ping;			// server to game info for scoreboard
} playerState_t;

typedef enum {
	WEATHER_NONE,
	WEATHER_RAIN,
	WEATHER_RAIN_PLAIN,
	WEATHER_SNOW
} weather_t;

#define DYNAMIC_LIGHT_NO_DEFAULT		255
#define DYNAMIC_LIGHT_MAX_INTENSITY		254

typedef struct worldState_s {
	byte				dynamic_light_intensities[ MAX_LIGHTING_GROUPS ];

	byte				dynamic_light_default_intensities[ MAX_LIGHTING_GROUPS ];

	vec3_t				wind_direction;
	float				wind_intensity;

	weather_t		weather_type;
	int				weather_intensity;

	float				time_scale;
} worldState_t;

#define MAX_SERVER_SOUNDS        32
#define MAX_SERVER_SOUNDS_BITS   6

typedef struct
{
	vec3_t origin;
	int entity_number;
	int channel;
	short sound_index;
	float volume;
	float min_dist;
	qboolean stop_flag;
	float pitch_modifier;
} server_sound_t;

//====================================================================


//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game-only definitions
//
#define	BUTTON_ATTACKRIGHT_BITINDEX 	0
#define	BUTTON_ATTACKLEFT_BITINDEX  	1
//#define  BUTTON_SNEAK_BITINDEX         7
#define	BUTTON_RELOAD_BITINDEX			2			// weapon reload key
#define  BUTTON_RUN_BITINDEX           3
#define  BUTTON_HOLSTERWEAPON_BITINDEX 4
#define  BUTTON_USE_BITINDEX           5
#define	BUTTON_TALK_BITINDEX			   6			// displays talk balloon and disables actions
#define	BUTTON_ANY_BITINDEX			   7		   // any key whatsoever
#define	BUTTON_DROP_RUNE_BITINDEX	   8
#define	BUTTON_TRANSFER_ENERGY_BITINDEX	   8

// This should always be at least 1 larger than the highest above
// Also if this number needs to be larger the buttons var in usercmd_t might need to be larger
#define	USERCMD_BUTTON_MAX	16

#define	BUTTON_ATTACKRIGHT 	( 1 << BUTTON_ATTACKRIGHT_BITINDEX )
#define	BUTTON_ATTACKLEFT  	( 1 << BUTTON_ATTACKLEFT_BITINDEX )
//#define  BUTTON_SNEAK         ( 1 << BUTTON_SNEAK_BITINDEX )
#define  BUTTON_RUN           ( 1 << BUTTON_RUN_BITINDEX )
#define  BUTTON_HOLSTERWEAPON ( 1 << BUTTON_HOLSTERWEAPON_BITINDEX )
#define  BUTTON_USE           ( 1 << BUTTON_USE_BITINDEX )
#define	BUTTON_TALK			   ( 1 << BUTTON_TALK_BITINDEX )			// displays talk balloon and disables actions
#define	BUTTON_RELOAD			( 1 << BUTTON_RELOAD_BITINDEX )		// weapon reload key
#define	BUTTON_ANY			   ( 1 << BUTTON_ANY_BITINDEX )		   // any key whatsoever
#define	BUTTON_DROP_RUNE	   ( 1 << BUTTON_DROP_RUNE_BITINDEX )
#define	BUTTON_TRANSFER_ENERGY ( 1 << BUTTON_TRANSFER_ENERGY_BITINDEX )



// usercmd_t is sent to the server each client frame
typedef struct usercmd_s {
	int	serverTime;
   byte	msec;
	short	buttons;
	byte	weapon;
	short	angles[3];
	short	deltaAngles[3];	//the absolute number of angles the player has moved.
	short choffset[2];
	vec3_t realvieworigin;
	vec3_t realviewangles;
	qboolean thirdperson;
	signed char	forwardmove, rightmove, upmove, lean;
	
} usercmd_t;

//===================================================================

//
// Animation flags
//
#define  MDL_ANIM_DELTA_DRIVEN   ( 1 << 0 )
#define  MDL_ANIM_DEFAULT_ANGLES ( 1 << 3 )
#define  MDL_ANIM_NO_TIMECHECK   ( 1 << 4 )

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define	SOLID_BMODEL	0xffffff

#define	RF_THIRD_PERSON	   (1<<0)   	// don't draw through eyes, only mirrors (player bodies, chat sprites)
#define	RF_FIRST_PERSON	   (1<<1)		// only draw through eyes (view weapon, damage blood blob)
#define  RF_DEPTHHACK         (1<<2)      // hack the z-depth so that view weapons do not clip into walls
#define  RF_VIEWLENSFLARE     (1<<3)      // View dependent lensflare
#define	RF_FRAMELERP		   (1<<4)      // interpolate between current and next state
#define	RF_BEAM				   (1<<5)      // draw a beam between origin and origin2
#define  RF_SHADOW_FROM_BIP01   (1<<6)
#define  RF_DONTDRAW			   (1<<7)      // don't draw this entity but send it over
#define  RF_LENSFLARE         (1<<8)      // add a lens flare to this
#define  RF_EXTRALIGHT        (1<<9)      // use good lighting on this entity
#define  RF_DETAIL            (1<<10)     // Culls a model based on the distance away from you
#define  RF_SHADOW            (1<<11)     // whether or not to draw a shadow
#define	RF_PORTALSURFACE	   (1<<12)   	// don't draw, but use to set portal views
#define	RF_SKYORIGIN   	   (1<<13)   	// don't draw, but use to set sky portal origin and coordinate system
#define	RF_SKYENTITY   	   (1<<14)   	// this entity is only visible through a skyportal
#define	RF_LIGHTOFFSET   	   (1<<15)   	// this entity has a light offset
#define	RF_CUSTOMSHADERPASS  (1<<16)     // draw the custom shader on top of the base geometry
#define	RF_MINLIGHT			   (1<<17)		// allways have some light (viewmodel, some items)
#define	RF_FULLBRIGHT		   (1<<18)		// allways have full lighting
#define  RF_LIGHTING_ORIGIN	(1<<19)		// use refEntity->lightingOrigin instead of refEntity->origin
									               // for lighting.  This allows entities to sink into the floor
									               // with their origin going solid, and allows all parts of a
									               // player to get the same lighting
#define	RF_SHADOW_PLANE      (1<<20)		// use refEntity->shadowPlane
#define	RF_WRAP_FRAMES		   (1<<21)		// mod the model frames by the maxframes to allow continuous
									               // animation without needing to know the frame count
#define  RF_PORTALENTITY      (1<<22)     // this entity should only be drawn from a portal
#define  RF_DUALENTITY        (1<<23)     // this entity is drawn both in the portal and outside it.
#define  RF_ADDITIVE_DLIGHT   (1<<24)     // this entity has an additive dynamic light
#define  RF_LIGHTSTYLE_DLIGHT (1<<25)     // this entity has a dynamic light that uses a light style
#define  RF_SHADOW_PRECISE    (1<<26)     // this entity can have a precise shadow applied to it
#define  RF_INVISIBLE         (1<<27)     // This entity is invisible, and only negative lights will light it up
#define  RF_WEAPONMODEL       (1<<28)     // This entity is a weapon model which is attached to players
#define  RF_FORCE_ALPHA			(1<<29)		// Force alpha on model
#define  RF_FORCE_ALPHA_EFFECTS	(1<<30)	// Force alpha on effects
#define  RF_CHILDREN_DONT_INHERIT_ALPHA	(1<<31)

//
// use this mask when propagating renderfx from one entity to another
//
#define  RF_FLAGS_NOT_INHERITED ( RF_LENSFLARE | RF_VIEWLENSFLARE | RF_BEAM | RF_EXTRALIGHT | RF_SKYORIGIN | RF_SHADOW | RF_SHADOW_PRECISE | RF_SHADOW_PLANE | RF_LIGHTOFFSET | RF_CHILDREN_DONT_INHERIT_ALPHA )

//
// the following flag is used by the server and is also defined in bg_public.h
//
#define  PMF_CAMERA_VIEW      ( 1<<8 )    // use camera view instead of ps view

#define BEAM_LIGHTNING_EFFECT   (1<<0)
#define BEAM_USEMODEL           (1<<1)
#define BEAM_PERSIST_EFFECT     (1<<2)
#define BEAM_SPHERE_EFFECT      (1<<3)
#define BEAM_RANDOM_DELAY       (1<<4)
#define BEAM_TOGGLE             (1<<5)
#define BEAM_RANDOM_TOGGLEDELAY (1<<6)
#define BEAM_WAVE_EFFECT        (1<<7)
#define BEAM_USE_NOISE          (1<<8)
#define BEAM_PARENT             (1<<9)
#define BEAM_TILESHADER         (1<<10)
#define BEAM_OFFSET_ENDPOINTS   (1<<11)
#define BEAM_WAVE_EFFECT2       (1<<12)
#define BEAM_FULLWAVE_EFFECT    (1<<13)


typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_SINE,					      // value = base + sin( time / duration ) * delta
	TR_GRAVITY,
   TR_LERP                    // Lerp between current origin and last origin
} trType_t;

typedef struct {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
	vec3_t	trBase;
	vec3_t	trDelta;			   // velocity, etc
} trajectory_t;

#define MAX_MODEL_SURFACES   32 // this needs to be the same in qfiles.h for TIKI_MAX_SURFACES

#define  MDL_SURFACE_SKINOFFSET_BIT0  ( 1 << 0 )
#define  MDL_SURFACE_SKINOFFSET_BIT1  ( 1 << 1 )
#define  MDL_SURFACE_NODRAW           ( 1 << 2 )
#define  MDL_SURFACE_SURFACETYPE_BIT0 ( 1 << 3 )
#define  MDL_SURFACE_SURFACETYPE_BIT1 ( 1 << 4 )
#define  MDL_SURFACE_SURFACETYPE_BIT2 ( 1 << 5 )
#define  MDL_SURFACE_CROSSFADE_SKINS  ( 1 << 6 )
#define  MDL_SURFACE_SKIN_NO_DAMAGE   ( 1 << 7 )

#define CROUCH_HEIGHT		36
#define CROUCH_EYE_HEIGHT	30
#define STAND_HEIGHT			72
#define STAND_EYE_HEIGHT	66

#define NUM_BONE_CONTROLLERS 5

#define NUM_MORPH_CONTROLLERS 10

#define NUM_EFFECTS_ANIMS	4

typedef struct {
	int		index;
	float		percent;
} morph_controller_t;

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

typedef struct entityState_s {
	int		number;			// entity index
	int		instanceNumber;
	int		eType;			// entityType_t
	int		eFlags;

	trajectory_t	pos;	// for calculating position
	trajectory_t	apos;	// for calculating angles

   vec3_t   netorigin;    // these are the ones actually sent over
	vec3_t	origin;
	vec3_t	origin2;

   vec3_t   netangles;    // these are the ones actually sent over
	vec3_t	angles;

	vec3_t  viewangles ;  // player's view angles.

	unsigned int	constantLight;	   // r + (g<<8) + (b<<16) + (intensity<<24)

	int      loopSound;		   // constantly loop this sound
	float		loopSoundVolume;
	float		loopSoundMinDist;

   int      parent;           // if this entity is attached, this is non-zero
   int      tag_num;          // if attached, the tag number it is attached to on the parent
	qboolean attach_use_angles;
	vec3_t	attach_offset;
	vec3_t	attach_angles_offset;

	int      modelindex;
	int		viewmodelindex;
	int      skinNum;
	int		customShader;
	int		customEmitter;
	float	animationRate;
   int      anim;
   int      frame;
   int      crossblend_time;  // in milliseconds so it can be transmitted as a short

   int      torso_anim;
   int      torso_frame;
   int      torso_crossblend_time; // in milliseconds so it can be transmitted as a short

   int      bone_tag[ NUM_BONE_CONTROLLERS ];
   vec3_t   bone_angles[ NUM_BONE_CONTROLLERS ];
   vec4_t   bone_quat[ NUM_BONE_CONTROLLERS ];        // not sent over

	morph_controller_t morph_controllers[ NUM_MORPH_CONTROLLERS ];

   byte     surfaces[MAX_MODEL_SURFACES];

	int		clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses

   int		groundEntityNum;  // -1 = in air
	int		solid;			// for client side prediction, trap_linkentity sets this properly

   float    scale;
   float    alpha;

	int				renderfx;

	unsigned int	affectingViewModes;

	int				archeTypeIndex;
	int				missionObjective;

	int				infoIcon;

	// This enables additional effects to be combined from other anims
	int		effectsAnims[ NUM_EFFECTS_ANIMS ];

	int      bindparent;    // not sent over
  	float		quat[4];       // not sent over
	float		mat[3][3];     // not sent over
} entityState_t;

typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED, 	// not talking to a server
	CA_AUTHORIZING,	// not used any more, was checking cd key
	CA_CONNECTING,		// sending request packets to the server
	CA_CHALLENGING,	// sending challenge packets to the server
	CA_CONNECTED,		// netchan_t established, getting gamestate
	CA_LOADING,			// only during cg.CG_GameStateReceived, never during main loop
	CA_PRIMED,			// got gamestate, waiting for first frame
	CA_ACTIVE,			// game views should be displayed
	CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
} connstate_t;

/*
========================================================================

SYSTEM MATH

========================================================================
*/

void MatrixMultiply( const float in1[3][3], const float in2[3][3], float out[3][3] );
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
void AxisToAngles( vec3_t axis[3], vec3_t angles );
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t left, vec3_t up);
void PerpendicularVector( vec3_t dst, const vec3_t src );
void EulerToQuat( float ang[ 3 ], float q[ 4 ] );
void MatToQuat( const float srcMatrix[ 3 ][ 3 ], float destQuat[ 4 ] );
void QuatToMat( const float q[ 4 ], float m[ 3 ][ 3 ]	);
void MatrixToEulerAngles( const float mat[ 3 ][ 3 ], vec3_t ang );


/*
========================================================================

TIKI

========================================================================
*/

#define TIKI_CMD_MAX_CMDS 256
#define TIKI_CMD_MAX_ARGS 12

typedef struct
   {
   int num_args;
   char *args[TIKI_CMD_MAX_ARGS];
   } tiki_singlecmd_t;

typedef struct
   {
   int num_cmds;
   tiki_singlecmd_t cmds[ TIKI_CMD_MAX_CMDS ];
   } tiki_cmd_t;

typedef struct
	{
	vec3_t	start;
	vec3_t	end;
	vec3_t	color;
	float		alpha;
   float    width;
   unsigned short factor;
   unsigned short pattern;
	} debugline_t;

// Added for FAKK2
#define FLOAT_TO_INT( x, fracbits ) ( ( x ) * ( 1 << ( fracbits ) ) )

#define FLOAT_TO_PKT( x, dest, wholebits, fracbits )                            \
   {                                                                            \
   if ( ( x ) >= ( 1 << ( wholebits ) ) )                                       \
      {                                                                         \
      ( dest ) = FLOAT_TO_INT( ( 1 << ( wholebits ) ), ( fracbits ) ) - 1; \
      }                                                                         \
   else if ( ( x ) < 0 )                                                        \
      {                                                                         \
      ( dest ) = 0;                                                             \
      }                                                                         \
   else                                                                         \
      {                                                                         \
      ( dest ) = FLOAT_TO_INT( ( x ), ( fracbits ) );               \
      }                                                                         \
   }

#define SIGNED_FLOAT_TO_PKT( x, dest, wholebits, fracbits )                   \
   {                                                                          \
   float temp_x;                                                              \
   temp_x = ( x ) + ( 1 << ( wholebits ) );                                   \
   if ( temp_x >= ( 1 << ( ( wholebits ) + 1 ) ) )                            \
      ( dest ) = FLOAT_TO_INT( ( 1 << ( ( wholebits ) + 1 ) ), ( fracbits ) ) - 1;    \
   else if ( temp_x < 0 )                                                     \
      (dest) = 0;                                                             \
   else                                                                       \
      ( dest ) = FLOAT_TO_INT( temp_x, ( fracbits ) );                        \
   }

#define INT_TO_FLOAT( x, wholebits, fracbits ) ( ( float )( ( ( float )( x ) ) / ( float )( 1 << ( fracbits ) ) - ( float )( 1 << ( wholebits ) ) ) )
#define UINT_TO_FLOAT( x, fracbits ) ( ( float )( ( ( float )( x ) ) / ( float )( 1 << ( fracbits ) ) ) )

#define TRANSLATION_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 4, 11 )
#define PKT_TO_TRANSLATION( x ) UINT_TO_FLOAT( ( x ), 11 )

#define OFFSET_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 1, 14 )
#define PKT_TO_OFFSET( x ) UINT_TO_FLOAT( ( x ), 14 )

#define ROTATE_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 9, 6 )
#define PKT_TO_ROTATE( x ) UINT_TO_FLOAT( ( x ), 6 )

#define BASE_TO_PKT( x, dest ) SIGNED_FLOAT_TO_PKT( ( x ), ( dest ), 3, 4 )
#define PKT_TO_BASE( x ) INT_TO_FLOAT( ( x ), 3, 4 )

#define AMPLITUDE_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 4, 4 )
#define PKT_TO_AMPLITUDE( x ) UINT_TO_FLOAT( ( x ), 4 )

#define PHASE_TO_PKT( x, dest ) SIGNED_FLOAT_TO_PKT( ( x ), ( dest ), 3, 4 )
#define PKT_TO_PHASE( x ) INT_TO_FLOAT( ( x ), 3, 4 )

#define FREQUENCY_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 4, 4 )
#define PKT_TO_FREQUENCY( x ) UINT_TO_FLOAT( ( x ), 4 )

#define BEAM_PARM_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 4, 4 )
#define PKT_TO_BEAM_PARM( x ) UINT_TO_FLOAT( ( x ), 4 )

// cinematic states
typedef enum {
	FMV_IDLE,
	FMV_PLAY,		// play
	FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;

//CDKey defines
#define CDKEY_LEN	 16
#define CDCHKSUM_LEN 2


#ifdef __cplusplus
}
#endif

#endif	// __Q_SHARED_H__
