//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/Shared/qcommon/alias.h                               $
// $Revision:: 5                                                              $
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
// Generic alias system for files.
// 

#ifndef __ALIAS_H__
#define __ALIAS_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// public implementation
//

const char *   Alias_Find( const char * alias );
qboolean Alias_Add( const char * alias, const char * name, const char * parameters );
qboolean Alias_Delete( const char * alias );
const char *   Alias_FindRandom( const char * alias );
void     Alias_Dump( void );
void     Alias_Clear( void );

//
// private implementation
//
#define MAX_ALIAS_NAME_LENGTH 32
#define MAX_REAL_NAME_LENGTH 128
#define MAX_ANIM_NAME_LENGTH 128
#define MAX_ALIASLIST_NAME_LENGTH MAX_QPATH

typedef struct AliasActorNode_s
	{
   int actor_number;

	int  number_of_times_played;
	byte been_played_this_loop;
	int  last_time_played;

   struct AliasActorNode_s * next;
   } AliasActorNode_t;

typedef struct AliasListNode_s
	{
   char alias_name[MAX_ALIAS_NAME_LENGTH];
   char real_name[MAX_REAL_NAME_LENGTH];
	char anim_name[MAX_ANIM_NAME_LENGTH];
   float weight;
	qboolean loop_anim;

	// Static alias info

	byte  global_flag;
	byte  stop_flag;
	float timeout;
	int   maximum_use;

	// Global alias info

	int  number_of_times_played;
	byte been_played_this_loop;
	int  last_time_played;

	// Actor infos

	AliasActorNode_t *actor_list;

   struct AliasListNode_s * next;
   } AliasListNode_t;

typedef struct AliasList_s
   {
   char name[ MAX_ALIASLIST_NAME_LENGTH ];
   qboolean    dirty;
   int         num_in_list;
   AliasListNode_t ** sorted_list;
   AliasListNode_t * data_list;
   } AliasList_t;

void Alias_ListClearActors( const AliasList_t * list );
AliasList_t * AliasList_New( const char * name );
const char *   Alias_ListFind( AliasList_t * list, const char * alias );
AliasListNode_t *Alias_ListFindNode( AliasList_t * list, const char * alias );
qboolean Alias_ListAdd( AliasList_t * list, const char * alias, const char * name, const char * parameters );
const char *   Alias_ListFindRandom( AliasList_t * list, const char * alias );
void     Alias_ListDump( AliasList_t * list );
void     Alias_ListClear( AliasList_t * list  );
void     Alias_ListDelete( AliasList_t * list );
void     Alias_ListSort( AliasList_t * list );
int Alias_IsGlobal( const AliasListNode_t *node, int actor_number );
AliasActorNode_t *Alias_FindActor( const AliasListNode_t *node, int actor_number );
void Alias_ListFindRandomRange( AliasList_t * list, const char * alias, int *min_index, int *max_index, float *total_weight );
const char * Alias_ListFindDialog( AliasList_t * list, const char * alias, int random, int actor_number);
const char* Alias_ListFindSpecificAnim( const AliasList_t *list, const char *name );
qboolean Alias_ListCheckLoopAnim( const AliasList_t *list, const char *name );
void Alias_ListUpdateDialog( AliasList_t * list, const char * alias, int number_of_times_played, byte been_played_this_loop, int last_time_played );
void Alias_ListAddActorDialog( AliasList_t * list, const char * alias, int actor_number, int number_of_times_played, byte been_played_this_loop, int last_time_played );
float randweight( void );

#ifdef __cplusplus
	}
#endif

#endif /* alias.h */
