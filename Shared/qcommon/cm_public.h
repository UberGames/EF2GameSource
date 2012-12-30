//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/Shared/qcommon/cm_public.h                           $
// $Revision:: 16                                                             $
//     $Date:: 10/13/03 8:54a                                                 $
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

#ifndef __CM_PUBLIC_H__
#define __CM_PUBLIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "qfiles.h"


void		CM_LoadMap( const char *name, qboolean clientload, int *checksum);
clipHandle_t CM_InlineModel( int index );		// 0 = world, 1 + are bmodels
clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int contents );

void		CM_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );

int			CM_NumClusters (void);
int			CM_NumInlineModels( void );
char		*CM_EntityString (void);

// returns an ORed contents mask
int			CM_PointContents( const vec3_t p, clipHandle_t model );
int			CM_PointBrushNum( const vec3_t p, clipHandle_t model );
int			CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );

void		CM_BoxTrace ( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask, qboolean cylinder);
void		CM_BoxTraceEx ( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask, qboolean cylinder, const unsigned int traceExFlags );
void		CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const vec3_t origin, const vec3_t angles, qboolean cylinder, qboolean force_rotation );

byte		*CM_ClusterPVS (int cluster);

int			CM_PointLeafnum( const vec3_t p );

void		CM_Clear( void );

// only returns non-solid leafs
// overflow if return listsize and if *lastLeaf != list[listsize-1]
int			CM_BoxLeafnums( const vec3_t mins, const vec3_t maxs, int *list,
		 					int listsize, int *lastLeaf );

int			CM_LeafCluster (int leafnum);
int			CM_LeafArea (int leafnum);

void		CM_AdjustAreaPortalState( int area1, int area2, qboolean open );
qboolean	CM_AreasConnected( int area1, int area2 );
void     CM_ResetAreaPortals( void );
void     CM_WritePortalState( fileHandle_t );
void	   CM_ReadPortalState( fileHandle_t );

int			CM_WriteAreaBits( byte *buffer, int area );
byte        *CM_VisibilityPointer( void );

int		CM_GetLightingGroup( const char *group_name );

// cm_tag.c
void		CM_LerpTag( orientation_t *tag,  clipHandle_t model, int startFrame, int endFrame, 
					 float frac, const char *tagName );


// cm_marks.c
int	CM_MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection,
				   int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float max_dist, qboolean test_normal );

// cm_patch.c
void CM_DrawDebugSurface( void (*drawPoly)(int color, int numPoints, float *points) );
void CM_DrawDebugTerrain( void (*drawPoly)(unsigned int color, int numPoints, float *points) );

// ToolServer wrapper functions
void  ToolServerInit( void );
void  ToolServerShutdown( void );
void  ToolServerProcessCommands( void );
void* ToolServerGetData( void );
unsigned int ToolServerGetNumClients();

// GameplayManager functions
void	CreateGameplayManager( void );
void	ShutdownGameplayManager( void );


// LoadSaveGameManager functions
void CreateLoadSaveGameManager( void );
void DeleteLoadSaveGameManager( void );

//Arena Controller functions
void CreateArenaController( void );
void DeleteArenaController( void );

#ifdef __cplusplus
	}
#endif

#endif
