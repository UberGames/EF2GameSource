//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/surfaceflags.h                                $
// $Revision:: 12                                                             $
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
// DESCRIPTION: Surface flag parameters for q3map and game
//

#ifndef __SURFACEFLAGS_H__
#define __SURFACEFLAGS_H__

// This file must be identical in the game and utils directories

// contents flags are seperate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!

#define	CONTENTS_SOLID			   (1<<0)		// an eye is never valid in a solid
#define	CONTENTS_WINDOW				(1<<1)		// added for BOTLIB
#define	CONTENTS_AUX				(1<<2)		// added for BOTLIB
#define	CONTENTS_LAVA			   (1<<3)
#define	CONTENTS_SLIME			   (1<<4)
#define	CONTENTS_WATER			   (1<<5)
#define	CONTENTS_FOG			   (1<<6)

// more BOTLIB additions FIXME 
#define CONTENTS_MIST				(1<<6)		// added for BOTLIB, same as CONTENTS_FOG
#define CONTENTS_LADDER				(1<<7) // I know this is on a surfaceparm
#define CONTENTS_NOTTEAM2			(1<<8) // FIXME these can't be good
#define CONTENTS_NOTTEAM1			(1<<9)
#define CONTENTS_JUMPPAD			(1<<10)
#define CONTENTS_TELEPORTER			(1<<11)
#define CONTENTS_MOVER				(1<<12)
// end BOTLIB additions

#define CONTENTS_USABLE				(1<<12)
#define CONTENTS_SETCLIP			(1<<13)	// All members of this content set noclip each other
#define  CONTENTS_TARGETABLE		(1<<14)
#define  CONTENTS_AREAPORTAL     (1<<15)
#define	CONTENTS_PLAYERCLIP		(1<<16)
#define	CONTENTS_MONSTERCLIP	   (1<<17)
#define	CONTENTS_CAMERACLIP		(1<<18)
#define	CONTENTS_WEAPONCLIP		(1<<19)  // blocks projectiles and weapon attacks as well
#define  CONTENTS_SHOOTABLE_ONLY (1<<20)  // player can walk through this but can shoot it as well
#define CONTENTS_CLUSTERPORTAL (1 << 21) // added for BOTLIB
#define CONTENTS_BOTCLIP		(1 << 22) // added for BOTLIB
#define CONTENTS_DONOTENTER		(1 << 23) // added for BOTLIB
#define	CONTENTS_ORIGIN			(1<<24) 	// removed before bsping an entity
#define	CONTENTS_BODY			   (1<<25)	// should never be on a brush, only in game
#define	CONTENTS_CORPSE			(1<<26)
#define	CONTENTS_DETAIL			(1<<27)	// brushes not used for the bsp
#define	CONTENTS_STRUCTURAL		(1<<28)	// brushes used for the bsp
#define	CONTENTS_TRANSLUCENT   (1<<29)	// don't consume surface fragments inside
#define CONTENTS_NOBOTCLIP		(1<<30)	// added for BOTLIB
#define	CONTENTS_NODROP			(1<<31)	// don't leave bodies or items (death fog, lava)

#define  CONTENTS_KEEP  (CONTENTS_DETAIL)

#define	SURF_NODAMAGE			( 1<<0 )	// never give falling damage
#define	SURF_SLICK				( 1<<1 )	// effects game physics
#define	SURF_SKY				( 1<<2 )	// lighting from environment map
#define	SURF_LADDER				( 1<<3 ) // ladder surface
#define	SURF_NOIMPACT			( 1<<4 ) // don't make missile explosions
#define	SURF_NOMARKS			( 1<<5 ) // don't leave missile marks
#define SURF_CASTSHADOW			( 1<<6 ) // used in conjunction with nodraw allows surface to be not drawn but still cast shadows
#define SURF_NODRAW				( 1<<7 ) // don't generate a drawsurface at all
#define SURF_SKIP				( 1<<8 ) // defined for BOTLIB
#define SURF_TYPE_SAND			( 1<<9 ) // sand surface
#define	SURF_NOLIGHTMAP			( 1<<10 )// surface doesn't need a lightmap
#define SURF_ALPHASHADOW		( 1<<11 )// do per-pixel shadow tests based on the texture
#define SURF_TYPE_SNOW			( 1<<12 )// snow surface
#define	SURF_NOSTEPS			( 1<<13 )// no footstep sounds
#define	SURF_NONSOLID			( 1<<14 )// don't collide against curves with this set
#define SURF_RICOCHET			( 1<<15 )// ricochet bullets

#define SURF_TYPE_METAL_DUCT	( 1<<16 )// metal duct surface
#define SURF_TYPE_METAL			( 1<<17 )// metal surface
#define SURF_TYPE_ROCK			( 1<<18 )// stone surface
#define SURF_TYPE_DIRT			( 1<<19 )// dirt surface
#define SURF_TYPE_GRILL			( 1<<20 )// metal grill surface
#define SURF_TYPE_ORGANIC		( 1<<21 )// oraganic (grass, loamy dirt)
#define SURF_TYPE_SQUISHY		( 1<<22 )// squishy (swamp dirt, flesh)

#define	SURF_NODLIGHT			( 1<<23 )// don't dlight even if solid (solid lava, skies)
#define	SURF_HINT   			( 1<<24 )// choose this plane as a partitioner
#define SURF_TYPE_DEFAULT		( 1<<25 )

#define SURF_TYPE_METAL_HOLLOW	( 1<<26 )// hollow metal surface
#define SURF_TYPE_CARPET		( 1<<27 )// carpet surface

#define SURF_PATCH				( 1<<29 )
#define SURF_TERRAIN			( 1<<30 )

#define SURF_KEEP		(SURF_PATCH | SURF_TERRAIN)

#define MASK_SURF_TYPE	(SURF_TYPE_SAND|SURF_TYPE_SNOW|SURF_TYPE_METAL_DUCT|SURF_TYPE_METAL|SURF_TYPE_ROCK|SURF_TYPE_DIRT|SURF_TYPE_GRILL|SURF_TYPE_ORGANIC|SURF_TYPE_SQUISHY|SURF_TYPE_METAL_HOLLOW|SURF_TYPE_CARPET)

enum
{
	TRACE_IGNORE_CURVE_PATCHES	=	( 1 << 0 ),
	TRACE_IGNORE_TERRAIN_BRUSHES =	( 1 << 1 ),
};

void ParseSurfaceParm( char *token, int * flags, int * contents );

#endif
