//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/debuglines.h                                  $
// $Revision:: 5                                                              $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#ifndef __DEBUGLINES_H__
#define __DEBUGLINES_H__


#if defined(__cplusplus)
#include "g_local.h"

void G_InitDebugLines( void );
void G_DebugLine( const Vector &start, const Vector &end, float r, float g, float b, float alpha );
void G_LineStipple( int factor, unsigned short pattern );
void G_LineWidth( float width );
void G_Color3f( float r, float g, float b	);
void G_Color3v( const Vector &color );
void G_Color4f( float r, float g,	float b,	float alpha	);
void G_Color3vf( const Vector &color, float alpha );
void G_BeginLine( void );
void G_Vertex( const Vector &v );
void G_EndLine( void );
void G_DebugBBox( const Vector &org, const Vector &mins, const Vector &maxs, float r, float g, float b, float alpha );
void G_DrawDebugNumber( const Vector &org, float number, float scale, float r, float g, float b, int precision = 0 );
void G_DebugCircle( const Vector &org, float radius, float r, float g, float b, float alpha, qboolean horizontal = false );
void G_DebugOrientedCircle( const Vector &org, float radius, float r, float g, float b, float alpha, Vector angles );
void G_DebugPyramid( const Vector &org, float radius, float r, float g, float b, float alpha );
void G_DrawCoordSystem( const Vector &pos, const Vector &f, const Vector &r, const Vector &u, int len );
void G_DebugArrow( const Vector &org, const Vector &dir, float length, float r, float g, float b, float alpha );
void G_DrawCSystem( void );

typedef enum
   {
   facet_north,
   facet_south,
   facet_east,
   facet_west,
   facet_up,
   facet_down
   } facet_t;

void G_DebugHighlightFacet( const Vector &org, const Vector &mins, const Vector &maxs, facet_t facet, float r, float g, float b, float alpha );
#endif // defined(__cplusplus)

#if defined(__cplusplus)
extern "C" {
#endif
	 void G_DebugLineC(const vec3_t start,const vec3_t end,float r,float g,float b,float alpha);
#if defined(__cplusplus)
}
#endif

#endif /* !__DEBUGLINES_H__ */
