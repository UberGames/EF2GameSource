//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/Shared/qcommon/qfiles.h                 $
// $Revision:: 58                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:42a                                                 $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#ifndef __QFILES_H__
#define __QFILES_H__

#pragma warning( disable : 4201 )
#include <game/q_shared.h>

//
// qfiles.h: quake file formats
// This file must be identical in the game and utils directories
//

// added for BOTLIB
#define	LAST_VISIBLE_CONTENTS	64

// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

// 3-5 are non-axial planes snapped to the nearest
#define	PLANE_ANYX		3
#define	PLANE_ANYY		4
#define	PLANE_ANYZ		5
// end BOTLIB additions

// surface geometry should not exceed these limits
#define	SHADER_MAX_VERTEXES	3000
#define	SHADER_MAX_INDEXES	(6*SHADER_MAX_VERTEXES)


// the maximum size of game reletive pathnames
#define	MAX_QPATH		64


/// Common macros used throughout engine and utils (no place else to put this stuff!)
#define BIT(x)			(1<<(x)) // Squirrel


//---------------------------------------------------------------------------
// Integral constants for lighting system (used by both tools and game)
//---------------------------------------------------------------------------
enum
{
	DYNAMIC_GROUP_NONE		= -1,

	MAX_LIGHT_EXCLUSIVE_NAMES = 4,
};


enum
{
	SUBINFO_BUFFER_ID_BSP			= 2,
	SUBINFO_BUFFER_ID_VIS			= 3,
	SUBINFO_BUFFER_ID_LIGHT			= 4,

	MAX_SUBINFO_BUFFERS				= 10, // Maximum number of subinfo buffer types
	MAX_SUBINFO_BUFFER_SIZE			= 2084, // Maximum number of bytes per subinfo buffer
};


/*
========================================================================

QVM files

========================================================================
*/

#define	VM_MAGIC	0x12721444
typedef struct {
	int		vmMagic;

	int		instructionCount;

	int		codeOffset;
	int		codeLength;

	int		dataOffset;
	int		dataLength;
	int		litLength;			// ( dataLength - litLength ) should be byteswapped on load
	int		bssLength;			// zero filled memory appended to datalength
} vmHeader_t;

/*
========================================================================

PCX files are used for 8 bit images

========================================================================
*/

typedef struct {
    char	manufacturer;
    char	version;
    char	encoding;
    char	bits_per_pixel;
    unsigned short	xmin,ymin,xmax,ymax;
    unsigned short	hres,vres;
    unsigned char	palette[48];
    char	reserved;
    char	color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    char	filler[58];
    unsigned char	data;			// unbounded
} pcx_t;


/*
========================================================================

TGA files are used for 24/32 bit images

========================================================================
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

/*
========================================================================

FTX files are pre mipmapped files

========================================================================
*/
#define FTX_EXTENSION ".ftx"

typedef struct ftx_s {
   int   width;
   int   height;
   int   has_alpha;
   // data follows
   } ftx_t;

/*
========================================================================

.MD3 triangle model file format

========================================================================
*/

#define MD3_IDENT			(('3'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD3_VERSION			15

// limits
#define MD3_MAX_LODS		3
#define	MD3_MAX_TRIANGLES	8192	// per surface
#define MD3_MAX_VERTS		4096	// per surface
#define MD3_MAX_SHADERS		256		// per surface
#define MD3_MAX_FRAMES		1024	// per model
#define	MD3_MAX_SURFACES	32		// per model
#define MD3_MAX_TAGS		16		// per frame

// vertex scales
#define	MD3_XYZ_SCALE		(1.0/64)

typedef struct md3Frame_s {
	vec3_t		bounds[2];
	vec3_t		localOrigin;
	float		radius;
	char		name[16];
} md3Frame_t;

typedef struct md3Tag_s {
	char		name[MAX_QPATH];	// tag name
	vec3_t		origin;
	vec3_t		axis[3];
} md3Tag_t;

/*
** md3Surface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numFrames
*/
typedef struct {
	int		ident;				//

	char	name[MAX_QPATH];	// polyset name

	int		flags;
	int		numFrames;			// all surfaces in a model should have the same

	int		numShaders;			// all surfaces in a model should have the same
	int		numVerts;

	int		numTriangles;
	int		ofsTriangles;

	int		ofsShaders;			// offset from start of md3Surface_t
	int		ofsSt;				// texture coords are common for all frames
	int		ofsXyzNormals;		// numVerts * numFrames

	int		ofsEnd;				// next surface follows
} md3Surface_t;

typedef struct {
	char			name[MAX_QPATH];
	int				shaderIndex;	// for in-game use
} md3Shader_t;

typedef struct {
	int			indexes[3];
} md3Triangle_t;

typedef struct {
	float		st[2];
} md3St_t;

typedef struct {
	short		xyz[3];
	short		normal;
} md3XyzNormal_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	int			flags;

	int			numFrames;
	int			numTags;
	int			numSurfaces;

	int			numSkins;

	int			ofsFrames;			// offset for first frame
	int			ofsTags;			// numFrames * numTags
	int			ofsSurfaces;		// first surface, others follow

	int			ofsEnd;				// end of file
} md3Header_t;


/*
==============================================================================

MD4 file format

==============================================================================
*/

#define MD4_IDENT			(('4'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD4_VERSION			1
#define	MD4_MAX_BONES		128

typedef struct {
	int			boneIndex;		// these are indexes into the boneReferences,
	float		   boneWeight;		// not the global per-frame bone list
	vec3_t		offset;
} md4Weight_t;

typedef struct {
   vec3_t      vertex;
	vec3_t		normal;
	vec2_t		texCoords;
	int			numWeights;
	md4Weight_t	weights[1];		// variable sized
} md4Vertex_t;

typedef struct {
	int			indexes[3];
} md4Triangle_t;

typedef struct {
	int			ident;

	char		name[MAX_QPATH];	// polyset name
	char		shader[MAX_QPATH];
	int			shaderIndex;		// for in-game use

	int			ofsHeader;			// this will be a negative number

	int			numVerts;
	int			ofsVerts;

	int			numTriangles;
	int			ofsTriangles;

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int			numBoneReferences;
	int			ofsBoneReferences;

	int			ofsEnd;				// next surface follows
} md4Surface_t;

typedef struct {
	float		matrix[3][4];
} md4Bone_t;

typedef struct {
	vec3_t		bounds[2];			// bounds of all surfaces of all LOD's for this frame
	vec3_t		localOrigin;		// midpoint of bounds, used for sphere cull
	float		radius;				// dist from localOrigin to corner
	char		name[16];
	md4Bone_t	bones[1];			// [numBones]
} md4Frame_t;

typedef struct {
	int			numSurfaces;
	int			ofsSurfaces;		// first surface, others follow
	int			ofsEnd;				// next lod follows
} md4LOD_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	// frames and bones are shared by all levels of detail
	int			numFrames;
	int			numBones;
	int			ofsFrames;			// md4Frame_t[numFrames]

	// each level of detail has completely separate sets of surfaces
	int			numLODs;
	int			ofsLODs;

	int			ofsEnd;				// end of file
} md4Header_t;


/*
==============================================================================

  TIKI model file format

==============================================================================
*/

#define TIKI_IDENT         (('I'<<24)+('K'<<16)+('I'<<8)+'T')
#define TIKI_ANIM_IDENT    ((' '<<24)+('N'<<16)+('A'<<8)+'T')
#define TIKI_ANIM_VERSION  2

// limits
#define TIKI_MAX_LODS      4
#define TIKI_MAX_TRIANGLES 4096  // per surface
#define TIKI_MAX_VERTS				4000
#define TIKI_MAX_SURFACE_VERTS	1200  // per surface
#define TIKI_MAX_SHADERS   256   // per surface
#define TIKI_MAX_FRAMES			1024  // per model (vertex models)
#define TIKI_MAX_SKEL_FRAMES    8192  // per model (skel models)
#define TIKI_MAX_SURFACES  32    // per model
#define TIKI_MAX_TAGS      16    // per frame

#define MAX_SHADERNAME     64

typedef struct
   {
   unsigned short xyz[3];
   short    normal;
   } tikiXyzNormal_t;

typedef struct tikiFrame_s
   {
   vec3_t      bounds[2];
   vec3_t      scale; // multiply by this
   vec3_t      offset; // and add by this
   vec3_t      delta;
   float       radius;
   float       frametime;
   } tikiFrame_t;

/*
** tikiSurface_t
**
** CHUNK       SIZE
** header         sizeof( md3Surface_t )
** triangles[0]   sizeof( md3Triangle_t ) * numTriangles
** st             sizeof( md3St_t ) * numVerts
** XyzNormals     sizeof( md3XyzNormal_t ) * numVerts * numFrames
*/
typedef struct
   {
   int      ident;            //

   char     name[MAX_QPATH];  // polyset name

   int      numFrames;        // all surfaces in a model should have the same
   int      numVerts;
   int      numModelVerts;

   int      numTriangles;
   int      ofsTriangles;

   int      ofsCollapseMap;   // numVerts * int

   int      ofsSt;            // texture coords are common for all frames
   int      ofsXyzNormals;    // numVerts * numFrames

   int      ofsEnd;           // next surface follows
   } tikiSurface_t;

typedef struct
   {
   vec3_t      origin;
   vec3_t      axis[3];
   } tikiTagData_t;

typedef struct
   {
   char     name[MAX_QPATH];  // tag name
   } tikiTag_t;

typedef struct
   {
   int      ident;
   int      version;

   char     name[MAX_QPATH];  // model name

   int      numFrames;
   int      numTags;
   int      numSurfaces;
   float    totaltime;
   vec3_t   totaldelta;

   int      ofsFrames;        // offset for first frame
   int      ofsSurfaces;      // first surface, others follow
   int      ofsTags[ TIKI_MAX_TAGS ]; // tikiTag_t + numFrames * tikiTagData_t

   int      ofsEnd;           // end of file
   } tikiHeader_t;

/*
==============================================================================

  TIKI Skeleton model file format

==============================================================================
*/
#define TIKI_SKEL_IDENT       ((' '<<24)+('L'<<16)+('K'<<8)+'S')
#define TIKI_SKEL_ANIM_IDENT  (('N'<<24)+('A'<<16)+('K'<<8)+'S')
#define TIKI_SKEL_VERSION     4
#define TIKI_SKEL_MAXBONES    100
#define MAX_SKEL_MODELS       80
#define TIKI_BONE_CACHE       ( TIKI_SKEL_MAXBONES * MAX_SKEL_MODELS )

#define TIKI_MORPH_IDENT       (('H'<<24)+('P'<<16)+('R'<<8)+'M')
#define TIKI_MORPH_VERSION     2

#define TIKI_SKEL_PARENTBONE  -1

#define TIKI_BONEFLAG_LEG     1

#define TIKI_ANIM_NORMAL      0
#define TIKI_ANIM_NO_OFFSETS  1

#define TIKI_BONE_OFFSET_MANTISSA_BITS ( 9 )
#define TIKI_BONE_OFFSET_MAX_SIGNED_VALUE ( ( 1 << TIKI_BONE_OFFSET_MANTISSA_BITS ) - 1 )
#define TIKI_BONE_OFFSET_SIGNED_SHIFT ( 15 - ( TIKI_BONE_OFFSET_MANTISSA_BITS ) )
#define TIKI_BONE_OFFSET_MULTIPLIER ( ( 1 << ( TIKI_BONE_OFFSET_SIGNED_SHIFT ) ) - 1 )
#define TIKI_BONE_OFFSET_MULTIPLIER_RECIPROCAL ( ( 1.0f ) / ( TIKI_BONE_OFFSET_MULTIPLIER ) )

#define TIKI_BONE_QUAT_FRACTIONAL_BITS ( 15 )
#define TIKI_BONE_QUAT_MULTIPLIER ( ( 1 << ( TIKI_BONE_QUAT_FRACTIONAL_BITS ) ) - 1 )
#define TIKI_BONE_QUAT_MULTIPLIER_RECIPROCAL ( ( 1.0f ) / ( TIKI_BONE_QUAT_MULTIPLIER ) )

typedef struct
   {
	int			         boneIndex;
	float		            boneWeight;
   vec3_t               offset;
   } skelWeight_t;

typedef struct
   {
	vec3_t		         normal;
	vec2_t		         texCoords;
	int			         numWeights;
	skelWeight_t	      weights[ 1 ];		// variable sized
   } skelVertex_t;

typedef struct
   {
   short                shortQuat[ 4 ];
   short                shortOffset[ 3 ];
   short                padding_do_not_use;
   } skelBone_t;

typedef struct
   {
   float                quat[ 4 ];
   float                offset[ 3 ];
   float                matrix[ 3 ][ 3 ];
   } skelBoneCache_t;

typedef struct
   {
   vec3_t		         bounds[ 2 ];
   float		            radius;				// dist to corner
   vec3_t               delta;
   skelBone_t           bones[ 1 ];       // variable sized
   } skelFrame_t;

typedef struct
   {
	int			         indexes[ 3 ];
   } skelTriangle_t;

typedef struct
	{
	int						vertex_index;
	vec3_t					delta;
	} skelMorphVertex_t;

typedef struct
	{
	char						name[ MAX_QPATH ];

	int						numverts;
	int						ofsMorphVerts;
	} skelMorph_t;

typedef struct {
	int			         ident;
	char		            name[ MAX_QPATH ];	// polyset name
	int			         numTriangles;
	int			         numVerts;
   int                  numModelVerts;
   int			         ofsTriangles;
   int			         ofsVerts;
   int                  ofsCollapse;
   int			         ofsEnd;				   // next surface follows
   } skelSurface_t;

typedef struct
   {
   int                  parent;
   int                  flags;
	char		            name[ MAX_QPATH ];	// bone name
   } skelBoneName_t;

#define MAX_BONE_REFERENCE_NAME_LENGTH  32

typedef struct
   {
	float						length;
	char						name[ MAX_BONE_REFERENCE_NAME_LENGTH ];
   } skelBoneReference_t;

typedef struct          
   {                    
	int                  ident;
	int                  version;
	char		            name[ MAX_QPATH ];	// model name

   int                  numsurfaces;
   int                  numbones;

   int                  ofsBones;
   int                  ofsSurfaces;
	int                  ofsBoneBaseFrame;
	int                  ofsEnd;
   } skelHeader_t;

typedef struct
   {
	int                  ident;
	int                  version;
   char		            name[ MAX_QPATH ];   // anim name

   int                  type;
	int                  numFrames;
	int                  numbones;
   float                totaltime;
   float                frametime;
   vec3_t               totaldelta;
	int                  ofsBoneRefs;
	int                  ofsFrames;
   } skelAnimHeader_t;

typedef struct
	{
	int						ident;
	int						version;
	char		            name[ MAX_QPATH ];	// model name

	int						numverts;
	int						nummorphs;

	int						ofsVertexId;
	int						ofsMorphTargets;
	} skelMorphHeader_t;

//====================================
// TIKI DEF STUFF
//====================================


// 
// frame cmd flags
//
#define TIKI_FRAME_CMD_EVERY_FRAME  -1
#define TIKI_FRAME_CMD_EXIT         -2
#define TIKI_FRAME_CMD_ENTRY        -3
#define TIKI_FRAME_CMD_LAST_SPECIAL -4
#define TIKI_FRAME_CMD_END			-5

#define TIKI_FRAME_CMD_MAXFRAMERATE ( 1000 / 50 )

#define MAX_ANIMDEFNAME   48
#define MAX_ARGS_PER_CMD  32

#define MAX_ANIMFULLNAME   128
#define MAX_DEFNAME        128
#define MAX_SKELNAME       128

typedef struct
{
   int         frame_num;
   int         num_args;
   int         ofs_args[MAX_ARGS_PER_CMD];
} dtikicmd_t;

#define MAX_EXPRESSION_NAME_LENGTH 32

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
} dtikiexpression_t;

#define  MAX_TIKI_SKINS    4
typedef struct
   {
   char        name[MAX_QPATH];  // polyset name
   char        shader[MAX_TIKI_SKINS][MAX_SHADERNAME];// shader name
#ifdef UTILS
   int         shaderIndex[MAX_TIKI_SKINS];      // for in-game use
#else
   qhandle_t   hShader[MAX_TIKI_SKINS];      // for in-game use
#endif
   int         numskins;         // number of skins for this surface
   int         flags;
   float       damage_multiplier;
   } dtikisurface_t;

typedef struct
   {
	int			skelIndex;
	int			surfaceIndex;
	int			boneMappingIndex;
	int			morphMappingIndex;
	char		skelName[ MAX_SKELNAME ];
	char		morphName[ MAX_SKELNAME ];
   } dtikiReplacedSurface_t;

typedef struct
{
   int         bone;
   int         surface;
   float       radius;
	float			extra_length;
} dtikibonemapping_t;

//
// Animation flags
//
#define  MDL_ANIM_DELTA_DRIVEN   ( 1 << 0 )
#define  MDL_ANIM_DEFAULT_ANGLES ( 1 << 3 )

typedef struct
{
   char        alias[MAX_ANIMDEFNAME]; // anim name from grabbing
   char        fullname[MAX_ANIMFULLNAME]; // the full path name of the animation
   float       weight;
   int         blendtime;
   int         handle;
   int         flags;
   qboolean		_needsSetup;
   int         num_client_cmds;
   int         ofs_client_cmds;
   int         num_server_cmds;
   int         ofs_server_cmds;
} dtikianimdef_t;

//
// surface flags for models
//
//

//
// Surface flags sent over the net when changed
//
#define  MDL_SURFACE_SKINOFFSET_BIT0  ( 1 << 0 )
#define  MDL_SURFACE_SKINOFFSET_BIT1  ( 1 << 1 )
#define  MDL_SURFACE_NODRAW           ( 1 << 2 )
#define  MDL_SURFACE_SURFACETYPE_BIT0 ( 1 << 3 )
#define  MDL_SURFACE_SURFACETYPE_BIT1 ( 1 << 4 )
#define  MDL_SURFACE_SURFACETYPE_BIT2 ( 1 << 5 )
#define  MDL_SURFACE_CROSSFADE_SKINS  ( 1 << 6 )
#define  MDL_SURFACE_SKIN_NO_DAMAGE   ( 1 << 7 )

// 
// Surface flags which are static (not sent over net)
//
#define  MDL_SURFACE_NOMIPMAPS        ( 1 << 8 )
#define  MDL_SURFACE_NOPICMIP         ( 1 << 9 )
#define  MDL_SURFACE_NOCUSTOMSHADER   ( 1 << 10 )

typedef struct dtiki_s
{
	char           name[MAX_DEFNAME];
	int            num_surfaces;
	int            num_tags;
	int            num_anims;
	int            num_bone_mappings;
	int            ofs_bone_mappings;
	int            ofsAnimBoneMappings;
	int            num_expressions;
	int            ofs_expressions;
	void           *alias_list;
	int            num_client_initcmds;
	int            ofs_client_initcmds;
	int            num_server_initcmds;
	int            ofs_server_initcmds;
	int            ofs_surfaces;
	int            ofs_replacedSurfaces;
	float			load_scale;
	float			lod_scale;
	float			lod_bias;
	float			lod_start_distance;
	float			lod_start_percent_verts;
	float			lod_stop_distance;
	float			lod_stop_percent_verts;
	float			fade_dist_mod;
	vec3_t			boundingVolumeMin;
	vec3_t			boundingVolumeMax;
	vec3_t			light_offset;
	vec3_t			head_offset;
	qboolean		_loadAllAnims;
	vec3_t			load_origin;
	vec3_t			mins;
	vec3_t			maxs;
	float			radius;
	int				skelIndex;
	char			skelName[ MAX_SKELNAME ];
	char			morphName[ MAX_SKELNAME ];
	unsigned int	exclusive_light_name_hash;
	int				ofs_animdefs[ 1 ];
} dtiki_t;

/*
==============================================================================

  .BSP file format

==============================================================================
*/


#define BSP_HEADER  (('!'<<24)+('2'<<16)+('F'<<8)+'E')
		// little-endian "EF2!"
#define BSP_VERSION			20


// there shouldn't be any problem with increasing these values at the
// expense of more memory allocation in the utilities
#define	MAX_MAP_MODELS		      0x400
#define	MAX_MAP_BRUSHES		   0x8000
#define	MAX_MAP_ENTITIES	      0x1000
#define	MAX_MAP_ENTSTRING	      0x400000 // 0x80000
#define	MAX_MAP_SHADERS		   0x400

#define  MAX_MAP_SHADERSTRING    0x4000
#define  MAX_MAP_NUM_SHADERS     4096

#define	MAX_MAP_AREAS		      0x100	// MAX_MAP_AREA_BYTES in q_shared must match!
#define	MAX_MAP_FOGS		      0x100
#define	MAX_MAP_PLANES		      0x20000
#define	MAX_MAP_NODES		      0x20000
#define	MAX_MAP_BRUSHSIDES	   0x40000
#define	MAX_MAP_LEAFS		      0x20000
#define	MAX_MAP_LEAFFACES	      0x20000
#define	MAX_MAP_LEAFBRUSHES     0x40000
#define	MAX_MAP_PORTALS		   0x20000
#define	MAX_MAP_LIGHTING	      0x1000000
#define	MAX_MAP_LIGHTGRID	      0x800000
#define	MAX_MAP_VISIBILITY	   0x200000
#define  MAX_MAP_SPHERE_LIGHTS   0x400

#define	MAX_MAP_DRAW_SURFS	   0x20000
#define	MAX_MAP_DRAW_VERTS	   0x80000
#define	MAX_MAP_DRAW_INDEXES	   0x80000

#define	MAX_MAP_LIGHTING_VERTS		0x80000
#define	MAX_MAP_LIGHTING_SURFS		0x20000
#define	MAX_MAP_LIGHTING_VERTSURFS 0x20000

#define  MAX_MAP_LIGHTDEFS       MAX_MAP_DRAW_SURFS

#define	MAX_LIGHTING_GROUPS		32 // NOTE: do NOT change this... it is apparently used as "the max number of bitfields stored in an int" (grumble grumble).

#define	MAX_STATIC_LOD_MODELS	0x1000

//#define  MAX_MAP_BOUNDS          8192
//#define  MIN_MAP_BOUNDS          ( -MAX_MAP_BOUNDS )
//#define  MAP_SIZE                ( MAX_MAP_BOUNDS - MIN_MAP_BOUNDS )

// key / value pair sizes in the entities lump
#define	MAX_KEY				      32
#define	MAX_VALUE			      1024

// the editor uses these predefined yaw angles to orient entities up or down
#define	ANGLE_UP			               -1
#define	ANGLE_DOWN			            -2

#define  DEFAULT_CURVE_SUBDIVISIONS    4
//#define  DEFAULT_LIGHTMAP_RESOLUTION   32
#define  DEFAULT_LIGHTMAP_RESOLUTION   16
#define  DEFAULT_TERRAIN_LIGHTMAP_RESOLUTION   32
#define  MIN_LIGHTMAP_RESOLUTION       4
#define  MAX_LIGHTMAP_RESOLUTION       128

#define	LIGHTMAP_WIDTH		            128
#define	LIGHTMAP_HEIGHT		         128

#define	LIGHTMAP_SIZE			         ( LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT * 3 )

#define MIN_WORLD_COORD ( -65536 )
#define MAX_WORLD_COORD	( 65536 )

#define WORLD_SIZE		( MAX_WORLD_COORD - MIN_WORLD_COORD )


//=============================================================================


// in game version of lump
typedef struct {
   void     *buffer;
   int      length;
} gamelump_t;

typedef struct {
	int		fileofs, filelen;
} lump_t;

#define  LUMP_SHADERS				0
#define  LUMP_PLANES				1
#define  LUMP_LIGHTMAPS				2
#define  LUMP_BASELIGHTMAPS			3
#define  LUMP_CONTLIGHTMAPS			4
#define  LUMP_SURFACES				5
#define  LUMP_DRAWVERTS				6
#define  LUMP_DRAWINDEXES			7
#define  LUMP_LEAFBRUSHES			8
#define  LUMP_LEAFSURFACES			9
#define  LUMP_LEAFS					10
#define  LUMP_NODES					11
#define  LUMP_BRUSHSIDES			12
#define  LUMP_BRUSHES				13
#define  LUMP_FOGS					14
#define  LUMP_MODELS				15
#define  LUMP_ENTITIES				16
#define  LUMP_VISIBILITY			17
#define  LUMP_LIGHTGRID				18
#define  LUMP_ENTLIGHTS				19
#define  LUMP_ENTLIGHTSVIS			20
#define  LUMP_LIGHTDEFS				21
#define  LUMP_BASELIGHTINGVERTS		22
#define  LUMP_CONTLIGHTINGVERTS		23
#define  LUMP_BASELIGHTINGSURFS		24
#define  LUMP_LIGHTINGSURFS			25
#define  LUMP_LIGHTINGVERTSURFS		26
#define  LUMP_LIGHTINGGROUPS		27
#define  LUMP_STATIC_LOD_MODELS		28
#define  LUMP_BSPINFO				29
#define  HEADER_LUMPS				30


typedef struct {
	int			ident;
	int			version;
   int         checksum;

	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef struct {
	float		mins[3], maxs[3];
	int		firstSurface, numSurfaces;
	int		firstBrush, numBrushes;
} dmodel_t;

typedef struct {
	char		shader[MAX_QPATH];
	int		surfaceFlags;
	int		contentFlags;
   int      subdivisions;
} dshader_t;

// planes x^1 is allways the opposite of plane x

typedef struct {
	float		normal[3];
	float		dist;
} dplane_t;

typedef struct {
	int			planeNum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	int			mins[3];		// for frustom culling
	int			maxs[3];
} dnode_t;

typedef struct {
   int         cluster;       // -1 = opaque cluster (brushes but no surfaces)
	int			area;

	int			mins[3];			// for frustum culling
	int			maxs[3];

	int			numLeafSurfaces;
	int			firstLeafSurface;

	int			firstLeafBrush;
	int			numLeafBrushes;
} dleaf_t;

typedef struct {
	int			shaderNum;
	int			planeNum;			// positive plane side faces out of the leaf
} dbrushside_t;

typedef struct {
	int			numSides;
	int			firstSide;
	int			shaderNum;		// the shader that determines the contents flags
} dbrush_t;

typedef struct {
	char		   shader[MAX_QPATH];
	int			brushNum;
	int			visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
} dfog_t;

#ifndef QERADIANT

typedef struct {
	vec3_t		xyz;
	float		   st[2];
	vec3_t		normal;
	byte		   color[4];
	float			lodExtra;
	union
		{
		float		   lightmap[2];
		int         collapseMap;
		};
} drawVert_t;

typedef struct {
	byte		   color[3];
} lightingVert_t;

typedef struct {
	int			baseLightmapNum;
	int			baseLightmapX, baseLightmapY;
	int			baseLightmapWidth, baseLightmapHeight;

	int			lastUpdatedFrame;

	int			firstLightingSurface;
	int			numLightingSurfaces;

	int			baseLightingVerts;
	int			firstLightingVertSurface;
	int			numLightingVertSurfaces;
} baseLightingSurf_t;

typedef struct {
	int			lightmapNum;
	
	int			lightmapX, lightmapY;
	int			lightGroupNum;

	byte		   color[3];
} lightingSurf_t;

typedef struct {
	int			lightGroupNum;
	int			firstLightingVert;
	byte		   color[3];
} lightingVertSurf_t;

#define DYNAMIC_LIGHTMAP_BIT  ( 1 << 30 )

#define MAX_LIGHT_GROUP_NAME_LENGTH  64

typedef struct {
	char			name[ MAX_LIGHT_GROUP_NAME_LENGTH ];
} lightingGroup_t;

typedef struct {
	int	totalNumberOfVerts;
	float origin[3];
	float startDistance;
	float startPerCentVerts;
	float stopDistance;
	float stopPerCentVerts;
	float alphaFadeDistance;
	int	lodNumberOfVerts;
	int	frameCount;
} staticLodModel_t;

#endif

typedef enum {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE,
	MST_TERRAIN
} mapSurfaceType_t;

#define LIGHTMAP_NONE		-1
#define LIGHTMAP_NEEDED		-2
#define NUMBER_TERRAIN_VERTS 9

typedef struct {
	int			shaderNum;
	int			fogNum;
	int			surfaceType;

	int			firstVert;
	int			numVerts;

	int			firstIndex;
	int			numIndexes;

	int			lightmapNum;
	int			lightmapX, lightmapY;
	int			lightmapWidth, lightmapHeight;

	vec3_t		lightmapOrigin;
	vec3_t		lightmapVecs[3];	// for patches, [0] and [1] are lodbounds

	int			patchWidth;
	int			patchHeight;

   float       subdivisions;

	int			baseLightingSurface;
	// Terrain fields
	int			inverted;
	int			faceFlags[4];
} dsurface_t;

// the light grid may not contain the entire bounds of the world, to
// allow q3test2 like levels that float in the middle of a giant sky box
// to not waste huge amounts of time and space
typedef struct {
	vec3_t		origin;
	vec3_t		axis;
	int			bounds[3];
} dlightGrid_t;

typedef struct {
	int			lightIntensity;
	int			lightAngle;
	int			lightmapResolution;
	qboolean	twoSided;
	qboolean	lightLinear;
	vec3_t		lightColor;
	float		lightFalloff;
	float		backsplashFraction;
	float		backsplashDistance;
	float		lightSubdivide;
	qboolean	autosprite;
} dlightdef_t;


//---------------------------------------------------------------------------
// mapspherelext_s (Squirrel)
// 
// An extension of the <mapspherel_s> struct which adds new lighting
//	system characteristics to LUMP_ENTLIGHTS.
//---------------------------------------------------------------------------
typedef struct mapspherelext_ mapspherelext_s;
struct mapspherelext_
{
	vec3_t			targetNormal;
	float			dotProduct_hotspot;
	float			dotProduct_penumbra;
	float			falloff_start_dist;
	float			falloff_end_dist;
	float			curveOffset;
	float			spherical_ambient;
	float			brightness;
	int				exclusiveLightNameCount;
	unsigned int	exclusiveLightNameHashList[ MAX_LIGHT_EXCLUSIVE_NAMES ];
};


//---------------------------------------------------------------------------
// mapspherel_t (legacy)
//---------------------------------------------------------------------------
typedef struct mapspherel_s mapspherel_t;
struct mapspherel_s
{
	vec3_t		origin;
	vec3_t		color;
//	float		intensity;
//	float		min_intensity;
	int			leaf;
	qboolean	needs_trace;
	qboolean	lensflare;
	int			type;
//	vec3_t		spot_dir;
//	float		spot_radiusbydistance;
	int			group_num;

	mapspherelext_s		extras; // additional set of fields added to support entity lights in the new system (semi-hack)
};



//---------------------------------------------------------------------------
// bspSubInfoStruct_t
//---------------------------------------------------------------------------
typedef struct bspSubInfoStruct_s bspSubInfoStruct_t;
struct bspSubInfoStruct_s
{
	int		subInfoBufferID;
	char	subInfoBuffer[ MAX_SUBINFO_BUFFER_SIZE ];
};



//
// Q3RADIANT defines
//
#define MAX_BRUSH_SIZE WORLD_SIZE

/*
==============================================================================

  .WAL texture file format

==============================================================================
*/


#define  MIPLEVELS   4
typedef struct miptex_s
{
   char        name[32];
   unsigned    width, height;
   unsigned    offsets[MIPLEVELS];     // four mip maps stored
   char        animname[32];           // next frame in animation chain
   int         flags;
   int         contents;
   int         value;
} miptex_t;


/*
==============================================================================
LIGHTING DEFINITIONS
==============================================================================
*/
#define LIGHTING_GRIDSIZE_X 192
#define LIGHTING_GRIDSIZE_Y 192
#define LIGHTING_GRIDSIZE_Z 320
#define LIGHTING_GRIDSIZE { LIGHTING_GRIDSIZE_X, LIGHTING_GRIDSIZE_Y, LIGHTING_GRIDSIZE_Z }
#define LIGHTING_POINTSCALE 7500
#define LIGHTING_LINEARSCALE ( 1.0f / 8000.0f )
#define LIGHTING_SUNDIRECTION { 0.45,  0.3, 0.9 }
#define LIGHTING_SUNCOLOR     { 100, 100, 92 }
#define LIGHTING_DEFAULT_INTENSITY 300.0f
#define LIGHTING_AREASCALE 0.25
#define LIGHTING_FORMFACTORSCALE 3

#define LIGHTING_SPOTRADIUS      64
#define LIGHTING_SPOTDISTANCE    64
#define LIGHTING_RADIUSBYDISTANCE( rad, dist ) ( ( ( ( rad ) + 16 ) / ( dist ) ) )
#define LIGHTING_SUNLIGHT( dot, sunColor, dest ) \
   {                                               \
   float sunlight_scale = 2 * ( dot );             \
   if ( sunlight_scale > 1 ) sunlight_scale = 1;   \
	VectorMA( ( dest ), (sunlight_scale + (1-sunlight_scale)/4), ( sunColor ), ( dest ) ); \
   }
#define LIGHTING_LINEARPOINTLIGHT( dot, photons, linearscale, distance, falloff ) \
         ( ( dot ) * ( photons ) * ( linearscale ) - ( ( distance ) / ( falloff ) ) )
#define LIGHTING_POINTLIGHT( dot, photons, distance ) \
         ( ( dot ) * ( photons ) / ( ( distance ) * ( distance ) ) )

#define LIGHTING_DISTANCE_AT_POWER( lightintensity, power ) sqrt( LIGHTING_POINTSCALE * ( lightintensity ) / ( power )  )


//---------------------------------------------------------------------------
// lightFlags_e
//---------------------------------------------------------------------------
enum lightFlags_
{
	/// Spawnflags for light entities (Squirrel)
	SF_LIGHT_LINEAR			= BIT(  0 ), // 1
	SF_LIGHT_NO_ENTITIES	= BIT(  1 ), // 2
	SF_LIGHT_NEEDS_TRACE	= BIT(  2 ), // 4
	SF_LIGHT_SUN			= BIT(  3 ), // 8
	SF_LIGHT_DYNAMIC		= BIT(  4 ), // 16
	SF_LIGHT_LENSFLARE		= BIT(  5 ), // 32
	SF_LIGHT_NO_WORLD		= BIT(  6 ), // 64
};
typedef enum lightFlags_ lightFlags_e;

typedef enum
{
	emit_point,
	emit_area,
	emit_spotlight,
	emit_sun
} emittype_t;

unsigned int SurfaceNameToType( const char *surfaceName );
const char *SurfaceTypeToName( unsigned int surfaceType );

#endif

