//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/g_public.h                                    $
// $Revision:: 83                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:43a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Game module information visible to server


#ifndef __G_PUBLIC_H__
#define __G_PUBLIC_H__

#include <qcommon/qfiles.h>

#define	GAME_API_VERSION	4

#define	FRAMETIME   		( level.fixedframetime )

// entity->svFlags
// the server does not know how to interpret most of the values
// in entityStates (level eType), so the game must explicitly flag
// special server behaviors
#define	SVF_NOCLIENT			      (1<<0)	// don't send entity to clients, even if it has effects
#define  SVF_BOT					      (1<<1)
#define	SVF_BROADCAST			      (1<<2)	// send to all connected clients
#define	SVF_PORTAL				      (1<<3)	// merge a second pvs at origin2 into snapshots
#define	SVF_SENDPVS				      (1<<4)	// even though it doesn't have a sound or modelindex, still run it through the pvs
#define	SVF_USE_CURRENT_ORIGIN	   (1<<5)	// entity->currentOrigin instead of entity->s.origin
  									                  // for link position (missiles and movers)
#define	SVF_DEADMONSTER			   (1<<6)	// treat as CONTENTS_DEADMONSTER for collision
#define	SVF_MONSTER					   (1<<7)	// treat as CONTENTS_MONSTER for collision
#define	SVF_USEBBOX 				   (1<<9)	// do not perform perfect collision use the bbox instead
#define	SVF_ONLYPARENT				   (1<<10)	// only send this entity to its parent
#define	SVF_HIDEOWNER				   (1<<11)	// hide the owner of the client
#define	SVF_MONSTERCLIP            (1<<12)	// treat as CONTENTS_MONSTERCLIP for collision
#define	SVF_PLAYERCLIP             (1<<13)	// treat as CONTENTS_PLAYERCLIP for collision
#define  SVF_SENDONCE               (1<<14)  // Send this entity over the network at least one time
#define  SVF_SENT                   (1<<15)  // This flag is set when the entity has been sent over at least one time

typedef enum
	{
	SOLID_NOT,			// no interaction with other objects
	SOLID_TRIGGER,		// only touch when inside, after moving
	SOLID_BBOX,			// touch on edge
	SOLID_BSP			// bsp clip, touch on edge
	} solid_t;

//===============================================================

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

#ifndef GAME_INCLUDE

// the server needs to know enough information to handle collision and snapshot generation

struct gentity_s
   {
	entityState_t	         s;				      // communicated by server to clients
	struct playerState_s	   *client;
	qboolean	               inuse;
	qboolean	               linked;				// qfalse if not in any good cluster
	int			            linkcount;

	int			            svFlags;			   // SVF_NOCLIENT, SVF_BROADCAST, etc

	qboolean	               bmodel;				// if false, assume an explicit mins / maxs bounding box
									                  // only set by gi.SetBrushModel
	vec3_t		            mins, maxs;
	int			            contents;			// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									                  // a non-solid entity should set to 0

	vec3_t		            absmin, absmax;	// derived from mins/maxs and origin + rotation

   float                   radius;           // radius of object
   vec3_t                  centroid;         // centroid, to be used with radius
   int                     areanum;          // areanum needs to be seen inside the game as well

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		            currentOrigin;
	vec3_t		            currentAngles;

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->s.ownerNum = passEntityNum	(don't interact with your own missiles)
	// entity[ent->s.ownerNum].ownerNum = passEntityNum	(don't interact with other missiles from owner)
	int			            ownerNum;
	//gentity_t               *owner;		// objects never interact with their owners, to
									            // prevent player missiles from immediately
									            // colliding with their owner

   solid_t                 solid;
	// the game dll can add anything it wants after
	// this point in the structure
   };

#endif		// GAME_INCLUDE

//===============================================================

//
// functions provided by the main engine
//

// added for BOTLIB
#include "botlib.h"
//typedef struct bot_input_s bot_input_t;
//typedef struct bot_entitystate_s bot_entitystate_t;
//typedef struct client_s client_t;
//typedef struct usercmd_s usercmd_t;
// end BOTLIB additions

typedef struct
   {
	//============== general services ==================

	// print message on the local console
	void	         (*Printf)( const char *fmt, ... );
   void           (*DPrintf)( const char *fmt, ... );
	void	         (*WPrintf)( const char *fmt, ... );
   void           (*WDPrintf)( const char *fmt, ... );
   void           (*DebugPrintf)( const char *fmt, ... );


   void			  (*LocalizeFilePath)(const char* path, char* localizedPath);

	// abort the game
	void	         (*Error)( int errorLevel, const char *fmt, ... );

	// get current time for profiling reasons this should NOT be used for any game related tasks,
	// because it is not journaled
	int	         (*Milliseconds)( void );

	// managed memory allocation
	void	         *(*Malloc)( int size );
	void	         (*Free)( void *block );

	// console variable interaction
	cvar_t	      *(*cvar)( const char *var_name, const char *value, int flags );
	cvar_t		  *(*cvar_get)( const char *var_name);
	void           (*cvar_set)( const char *var_name, const char *value );
	// this section added for botlib compatibility
	void			(*Cvar_VariableStringBuffer)( const char *var_name, char *buffer, int bufsize );
	void			(*Cvar_Register)( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
	float			(*Cvar_VariableValue)( const char *var_name );
	void			(*Cvar_Update)( vmCvar_t *vmCvar );
	int				(*Cvar_VariableIntegerValue)( const char *var_name );
	// end cvar botlib additions

	// ClientCommand and ServerCommand parameter access
	int	         (*argc)( void );
	char	         *(*argv)( int n );
	const char	   *(*args)(void);
   void           (*AddCommand)( const char *cmd );

	// the returned buffer may be part of a larger pak file, or a discrete file from anywhere in the search path
	// a -1 return means the file does not exist NULL can be passed for buf to just determine existance
	int	         (*FS_ReadFile)( const char *name, void **buf, qboolean quiet );
	qboolean	 (*FS_Exists)( const char *filename );
	void	         (*FS_FreeFile)( void *buf );
	void	         (*FS_WriteFile)( const char *qpath, const void *buffer, int size );
   fileHandle_t   (*FS_FOpenFileWrite)( const char *qpath );
   fileHandle_t   (*FS_FOpenFileAppend)( const char *filename );
	char**        (*FS_ListFiles)( const char *path, const char *extension, int *numfiles );

   char *         (*FS_PrepFileWrite)( const char *filename );
   int	         (*FS_Write)( const void *buffer, int len, fileHandle_t f );
   int	         (*FS_Read)( void *buffer, int len, fileHandle_t f );
   void	         (*FS_FCloseFile)( fileHandle_t f );
   int		      (*FS_FTell)( fileHandle_t f );
   int		      (*FS_FSeek)( fileHandle_t f, long offset, int origin );
	//int				(*FS_filelength)( fileHandle_t f);
   void           (*FS_Flush)( fileHandle_t f );
	void				(*FS_DeleteFile)(const char *filename);
	int					(*FS_GetFileList)(  const char *path, const char *extension, char *listbuf, int bufsize ); // added for botlib
	
   const char *   (*GetArchiveFileName)( const char *gameName, const char *filename, const char *extension, qboolean tempFile );
	// add commands to the console as if they were typed in for map changing, etc
	void	         (*SendConsoleCommand)( const char *text );
	void	         (*DebugGraph)( float value, int color );

	//=========== server specific functionality =============

	// SendServerCommand reliably sends a command string to be interpreted by the given
	// client.  If ent is NULL, it will be sent to all clients
	void	         (*SendServerCommand)( int clientnum, const char *fmt, ... );
	
	int				(*GetNumFreeReliableServerCommands) ( int clientNum );

	// config strings hold all the index strings, the lightstyles, and misc data like the cdtrack.
	// All of the current configstrings are sent to clients when they connect, and
   // changes are sent to all connected clients.
   void           (*setConfigstring)( int index, const char *val );
   char *         (*getConfigstring)( int index );

   void           (*setUserinfo)( int index, const char *val );
   void           (*getUserinfo)( int index, char *buffer, int bufferSize );

	// sets mins and maxs based on the brushmodel name
	void	         (*SetBrushModel)( gentity_t *ent, const char *name );

	// collision detection
	void           (*trace)( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, qboolean cylinder );
	void           (*fulltrace)( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, qboolean cylinder );
	int		      (*pointcontents)( const vec3_t point, int passEntityNum );
	int		      (*pointbrushnum)( const vec3_t point, int passEntityNum );
	qboolean	      (*inPVS)( const vec3_t p1, const vec3_t p2 );
	qboolean	      (*inPVSIgnorePortals)( const vec3_t p1, const vec3_t p2 );
	void		      (*AdjustAreaPortalState)( gentity_t *ent, qboolean open );
	qboolean	      (*AreasConnected)( int area1, int area2 );

	int				(*GetLightingGroup)( const char *group_name );

	void				(*SetDynamicLight)( int dynamic_light, float intensity );
	void				(*SetDynamicLightDefault)( int dynamic_light, float intensity );

	void				(*SetWindDirection)( vec3_t direction );
	void				(*SetWindIntensity)( float wind_intensity );

	void				(*SetWeatherInfo)( weather_t type, int intensity );

	void				(*SetTimeScale)( float time_scale );
	// an entity will never be sent to a client or used for collision
	// if it is not passed to linkentity.  If the size, position, or
	// solidity changes, it must be relinked.
	void	         (*linkentity)( gentity_t *ent );
	void	         (*unlinkentity)( gentity_t *ent );		// call before removing an interactive entity

	// EntitiesInBox will perform exact checking to bmodels, as well as bounding box
	// intersection tests to other models
   int            (*AreaEntities)( const vec3_t mins, const vec3_t maxs, int *list, int maxcount, qboolean fullTrace );
	void				(*ClipToEntity)( trace_t *trace, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int entityNum, int contentmask );

	int			 (*objectivenameindex)( const char* name );
	int			 (*archetypeindex)(const char* name);
	int	         (*imageindex)( const char *name );
	int			 (*failedcondition)( const char *name );
	int	         (*itemindex)( const char *name );
	int	         (*soundindex)( const char *name );
	int	         (*modelindex)( const char *name );

  	void	         (*SetLightStyle)( int i, const char *data );

   const char	   *(*GameDir)( void );

   //
   // MODEL UTILITY FUNCTIONS
   //
   qboolean       (*IsModel)( int index );
	void		      (*setmodel)( gentity_t *ent, const char *name );
	void		      (*setviewmodel)( gentity_t *ent, const char *name );

   // DEF SPECIFIC STUFF
   int				(*NumAnims)( int modelindex );
   int				(*NumSkins)( int modelindex );
   int				(*NumSurfaces)( int modelindex );
   int				(*NumTags)( int modelindex );
	int				(*NumMorphs)( int modelindex );
   qboolean       (*InitCommands)( int modelindex, tiki_cmd_t * tiki_cmd );
   void				(*CalculateBounds)( int modelindex, float scale, vec3_t mins, vec3_t maxs );

   void				(*TIKI_CacheAnim)( const char * path );

   // ANIM SPECIFIC STUFF
   const char *   (*Anim_NameForNum)( int modelindex, int animnum );
   int				(*Anim_NumForName)( int modelindex, const char * name );
   int				(*Anim_Random)( int modelindex, const char * name );
   int				(*Anim_NumFrames)( int modelindex, int animnum );
   float				(*Anim_Time)( int modelindex, int animnum );
   void				(*Anim_Delta)( int modelindex, int animnum, vec3_t delta );
   void				(*Anim_AbsoluteDelta)( int modelindex, int animnum, vec3_t delta );
   int				(*Anim_Flags)( int modelindex, int animnum );
   qboolean			(*Anim_HasCommands) ( int modelindex, int animnum );

   // FRAME SPECIFIC STUFF
   qboolean       (*Frame_Commands)( int modelindex, int animnum, int framenum, tiki_cmd_t * tiki_cmd );
   void				(*Frame_Delta)( int modelindex, int animnum, int framenum, vec3_t delta );
   float				(*Frame_Time)( int modelindex, int animnum, int framenum );
   void				(*Frame_Bounds)( int modelindex, int animnum, int framenum, float scale, vec3_t mins, vec3_t maxs );

   // SURFACE SPECIFIC STUFF
   int				(*Surface_NameToNum)( int modelindex, const char * name );
   const char *   (*Surface_NumToName)( int modelindex, int num );
   int				(*Surface_Flags)( int modelindex, int num );
   int				(*Surface_NumSkins)( int modelindex, int num );

	// Morph specific stuff
	int				(*Morph_NumForName)( int modelindex, const char * name );
	const char *   (*Morph_NameForNum)( int modelindex, int num );

	dtikimorphtarget_t *(*GetExpression)( int modelindex, const char *expression_name, int *number_of_morph_targets );

   // TAG SPECIFIC STUFF
	int				(*Tag_NumForName)( int modelindex, const char * name );
   const char *   (*Tag_NameForNum)( int modelindex, int num );
   orientation_t  (*Tag_Orientation)( int modelindex, int anim, int frame, int num, float scale, int *bone_tag, vec4_t *bone_quat );
   orientation_t  (*Tag_OrientationEx)( int modelindex, int anim, int frame, int num, float scale, int *bone_tag, vec4_t *bone_quat,
      int crossblend_anim, int crossblend_frame, float crossblend_lerp, qboolean uselegs, qboolean usetorso, int torso_anim, int torso_frame,
      int torso_crossblend_anim, int torso_crossblend_frame, float torso_crossblend_lerp );
	int			   (*Bone_GetParentNum)( int modelindex, int bonenum );

   qboolean			(*Alias_Add)( int modelindex, const char * alias, const char * name, const char *parameters );
   const char *   (*Alias_FindRandom)( int modelindex, const char * alias );
   const char *   (*Alias_Find)( int modelindex, const char * alias );
   void				(*Alias_Dump)( int modelindex );
   void				(*Alias_Clear)( int modelindex );
	const char *   (*Alias_FindDialog)( int modelindex, const char * alias, int random, int entity_number );
	const char*    (*Alias_FindSpecificAnim)(int modelindex, const char *name );
	qboolean       (*Alias_CheckLoopAnim)(int modelindex, const char *name );
	void				*(*Alias_GetList)( int modelindex );
	void				(*Alias_UpdateDialog)( int model_index, const char *alias, int number_of_times_played, byte been_played_this_loop, int last_time_played );
	void				(*Alias_AddActorDialog)( int model_index, const char *alias, int actor_number, int number_of_times_played, byte been_played_this_loop, int last_time_played );


	const char *	(*NameForNum)( int modelindex );

   // GLOBAL ALIAS SYSTEM
   qboolean			(*GlobalAlias_Add)( const char * alias, const char * name, const char *parameters );
   const char *   (*GlobalAlias_FindRandom)( const char * alias );
   const char *   (*GlobalAlias_Find)( const char * alias );
   void				(*GlobalAlias_Dump)( void );
   void			   (*GlobalAlias_Clear)( void );

	qboolean			(*isClientActive)( const gentity_t *ent );

	void		      (*centerprintf)( const gentity_t *ent, CenterPrintImportance importance, const char *fmt, ...);
	void		      (*locationprintf)( const gentity_t *ent, int x, int y, const char *fmt, ...);

   // Sound
	void				(*Sound)( vec3_t *org, int entnum, int channel, const char *sound_name, float volume, float attenuation, float pitch_modifier, qboolean onlySendToSameEntity );
	void				(*StopSound)( int entnum, int channel );
 	float 		   (*SoundLength) ( const char *path );
	char	         *(*GetNextMorphTarget)( const char *name, int *index, int *number_of_amplitudes, float *amplitude );


   unsigned short (*CalcCRC)(const unsigned char *start, int count);

   debugline_t		**DebugLines;
	int				*numDebugLines;

   void           (*LocateGameData)( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
                                     playerState_t *clients, int sizeofGameClient );

   void           (*SetFarPlane)( int farplane, qboolean farplane_cull );
	void				(*TikiReload)( const char * modelstr );
	void				(*TikiLoadFromTS)( const char * path, const char * tikidata);
	void*				(*ToolServerGetData)( );
   void           (*SetSkyPortal)( qboolean skyportal );

	void				(*WidgetPrintf)( const char *widgetName, const char *fmt, ... );

	void				(*ProcessLoadingScreen)( const char *loadingStatus );

   // MISSION OBJECTIVE SPECIFIC STUFF
   
   const char*  (*MObjective_GetDescription)( const char* objectivename );
   void         (*MObjective_SetDescription)( const char* objectivename , const char* desc );

   qboolean     (*MObjective_GetShowObjective)( const char* objectivename );
   void         (*MObjective_SetShowObjective)( const char* objectivename , qboolean show );

   qboolean     (*MObjective_GetObjectiveComplete)( const char* objectivename );
   void         (*MObjective_SetObjectiveComplete)( const char* objectivename , qboolean complete );

   qboolean     (*MObjective_GetObjectiveFailed)( const char* objectivename );
   void         (*MObjective_SetObjectiveFailed)( const char* objectivename , qboolean failed );

   const char*  (*MObjective_GetNameFromIndex)( unsigned int index );
   int          (*MObjective_GetIndexFromName) (const char* name );
   void         (*MObjective_NewObjective)( const char* name );
   void         (*MObjective_ClearObjectiveList) (void);
   void         (*MObjective_ParseObjectiveFile) (const char* levelname );
   void			(*MObjective_Update)( const char* objectivesName );
	int			 (*MObjective_GetNumObjectives)(void);
	int			 (*MObjective_GetNumActiveObjectives)(void);
	int			 (*MObjective_GetNumCompleteObjectives)(void);
	int			 (*MObjective_GetNumFailedObjectives)(void);
	int			 (*MObjective_GetNumIncompleteObjectives)(void);



   // MISSION INFORMATION SPECIFIC STUFF
   const char*  (*MI_GetShader)( const char* informationname );
   void         (*MI_SetShader)( const char* informationname , const char* shader );

   const char*  (*MI_GetInformationData)( const char* informationname );
   void         (*MI_SetInformationData)( const char* informationname , const char* data );

   const char*  (*MI_GetNameFromIndex)( unsigned int index );
   int          (*MI_GetIndexFromName)( const char* name );
   void         (*MI_NewInformation)( const char* name );
   void         (*MI_ClearInformationList)( void );   

   void         (*MI_SetShowInformation) (const char* informationname , qboolean show );
   qboolean     (*MI_GetShowInformation) (const char* informationname );

	void		(*SR_InitializeStringResource)( void );
	void		(*SR_UninitializeStringResource)( void );
	void		(*SR_LoadLevelStrings)(const char* levelName);

	// View mode stuff
	unsigned int	(*GetViewModeMask)( const char *viewModeName );
	unsigned int	(*GetViewModeClassMask)( const char *className );
	qboolean			(*GetViewModeSendInMode)( unsigned int viewModeMask );
	qboolean			(*GetViewModeSendNotInMode)( unsigned int viewModeMask );
	qboolean			(*GetViewModeScreenBlend)( unsigned int viewModeMask, vec3_t color, float *alpha, qboolean *additive );

	void				(*GetLevelDefs)( const char *name, const char **environmentName, const char **levelName, const char **sublevelName );
	qboolean			(*areSublevels)( const char *oldMapName, const char *newMapName );

	const char *		(*SurfaceTypeToName)( unsigned int surfaceType );


	////////
	// AAS STUFF
	////////
	// swiped from aas_export_t
	//-----------------------------------
	// be_aas_entity.h
	//-----------------------------------
	void		(*AAS_EntityInfo)(int entnum, struct aas_entityinfo_s *info);
	//-----------------------------------
	// be_aas_main.h
	//-----------------------------------
	int			(*AAS_Initialized)(void);
	void		(*AAS_PresenceTypeBoundingBox)(int presencetype, vec3_t mins, vec3_t maxs);
	float		(*AAS_Time)(void);
	//--------------------------------------------
	// be_aas_sample.c
	//--------------------------------------------
	int			(*AAS_PointAreaNum)(vec3_t point);
	int			(*AAS_PointReachabilityAreaIndex)( vec3_t point );
	int			(*AAS_TraceAreas)(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas);
	int			(*AAS_BBoxAreas)(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas);
	int			(*AAS_AreaInfo)( int areanum, struct aas_areainfo_s *info );
	//--------------------------------------------
	// be_aas_bspq3.c
	//--------------------------------------------
	int			(*AAS_PointContents)(vec3_t point);
	int			(*AAS_NextBSPEntity)(int ent);
	int			(*AAS_ValueForBSPEpairKey)(int ent, char *key, char *value, int size);
	int			(*AAS_VectorForBSPEpairKey)(int ent, char *key, vec3_t v);
	int			(*AAS_FloatForBSPEpairKey)(int ent, char *key, float *value);
	int			(*AAS_IntForBSPEpairKey)(int ent, char *key, int *value);
	//--------------------------------------------
	// be_aas_reach.c
	//--------------------------------------------
	int			(*AAS_AreaReachability)(int areanum);
	//--------------------------------------------
	// be_aas_route.c
	//--------------------------------------------
	int			(*AAS_AreaTravelTimeToGoalArea)(int areanum, vec3_t origin, int goalareanum, int travelflags);
	int			(*AAS_EnableRoutingArea)(int areanum, int enable);
	int			(*AAS_PredictRoute)(struct aas_predictroute_s *route, int areanum, vec3_t origin,
							int goalareanum, int travelflags, int maxareas, int maxtime,
							int stopevent, int stopcontents, int stoptfl, int stopareanum);
	//--------------------------------------------
	// be_aas_altroute.c
	//--------------------------------------------
	int			(*AAS_AlternativeRouteGoals)(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags,
										struct aas_altroutegoal_s *altroutegoals, int maxaltroutegoals,
										int type);
	//--------------------------------------------
	// be_aas_move.c
	//--------------------------------------------
	int			(*AAS_Swimming)(vec3_t origin);
	int			(*AAS_PredictClientMovement)(struct aas_clientmove_s *move,
											int entnum, vec3_t origin,
											int presencetype, int onground,
											vec3_t velocity, vec3_t cmdmove,
											int cmdframes,
											int maxframes, float frametime,
											int stopevent, int stopareanum, int visualize);

	////////
	// BOTLIB client Elementary Actions (EA)
	////////
	//ClientCommand elementary actions
	void	(*EA_Command)(int client, const char *command );
	void	(*EA_Say)(int client, char *str);
	void	(*EA_SayTeam)(int client, char *str);
	//
	void	(*EA_Action)(int client, int action);
	void	(*EA_Gesture)(int client);
	void	(*EA_Talk)(int client);
	void	(*EA_ToggleFireState)(int client);
	void	(*EA_Attack)(int client, int primarydangerous, int altdangerous);
	void	(*EA_Use)(int client);
	void	(*EA_Respawn)(int client);
	void	(*EA_MoveUp)(int client);
	void	(*EA_MoveDown)(int client);
	void	(*EA_MoveForward)(int client);
	void	(*EA_MoveBack)(int client);
	void	(*EA_MoveLeft)(int client);
	void	(*EA_MoveRight)(int client);
	void	(*EA_Crouch)(int client);

	void	(*EA_SelectWeapon)(int client, int weapon);
	void	(*EA_Jump)(int client);
	void	(*EA_DelayedJump)(int client);
	void	(*EA_Move)(int client, vec3_t dir, float speed);
	void	(*EA_View)(int client, vec3_t viewangles);
	//send regular input to the server
	void	(*EA_EndRegular)(int client, float thinktime);
	void	(*EA_GetInput)(int client, float thinktime, bot_input_t *input);
	void	(*EA_ResetInput)(int client);

	////////
	// BOTLIB AI stuff
	////////
	//-----------------------------------
	// be_ai_char.h
	//-----------------------------------
	int		(*BotLoadCharacter)(char *charfile, float skill);
	void	(*BotFreeCharacter)(int character);
	float	(*Characteristic_Float)(int character, int index);
	float	(*Characteristic_BFloat)(int character, int index, float min, float max);
	int		(*Characteristic_Integer)(int character, int index);
	int		(*Characteristic_BInteger)(int character, int index, int min, int max);
	void	(*Characteristic_String)(int character, int index, char *buf, int size);
	//-----------------------------------
	// be_ai_chat.h
	//-----------------------------------
	int		(*BotAllocChatState)(void);
	void	(*BotFreeChatState)(int handle);
	void	(*BotQueueConsoleMessage)(int chatstate, int type, char *message);
	void	(*BotRemoveConsoleMessage)(int chatstate, int handle);
	int		(*BotNextConsoleMessage)(int chatstate, struct bot_consolemessage_s *cm);
	int		(*BotNumConsoleMessages)(int chatstate);
	void	(*BotInitialChat)(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7);
	int		(*BotNumInitialChats)(int chatstate, char *type);
	int		(*BotReplyChat)(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7);
	int		(*BotChatLength)(int chatstate);
	void	(*BotEnterChat)(int chatstate, int client, int sendto);
	void	(*BotGetChatMessage)(int chatstate, char *buf, int size);
	int		(*StringContains)(char *str1, char *str2, int casesensitive);
	int		(*BotFindMatch)(char *str, struct bot_match_s *match, unsigned long int context);
	void	(*BotMatchVariable)(struct bot_match_s *match, int variable, char *buf, int size);
	void	(*UnifyWhiteSpaces)(char *string);
	void	(*BotReplaceSynonyms)(char *string, unsigned long int context);
	int		(*BotLoadChatFile)(int chatstate, char *chatfile, char *chatname);
	void	(*BotSetChatGender)(int chatstate, int gender);
	void	(*BotSetChatName)(int chatstate, char *name, int client);
	//-----------------------------------
	// be_ai_goal.h
	//-----------------------------------
	void	(*BotResetGoalState)(int goalstate);
	void	(*BotResetAvoidGoals)(int goalstate);
	void	(*BotRemoveFromAvoidGoals)(int goalstate, int number);
	void	(*BotPushGoal)(int goalstate, struct bot_goal_s *goal);
	void	(*BotPopGoal)(int goalstate);
	void	(*BotEmptyGoalStack)(int goalstate);
	void	(*BotDumpAvoidGoals)(int goalstate);
	void	(*BotDumpGoalStack)(int goalstate);
	void	(*BotGoalName)(int number, char *name, int size);
	int		(*BotGetTopGoal)(int goalstate, struct bot_goal_s *goal);
	int		(*BotGetSecondGoal)(int goalstate, struct bot_goal_s *goal);
	int		(*BotChooseLTGItem)(int goalstate, vec3_t origin, int *inventory, int travelflags);
	int		(*BotChooseNBGItem)(int goalstate, vec3_t origin, int *inventory, int travelflags,
								struct bot_goal_s *ltg, float maxtime);
	int		(*BotTouchingGoal)(vec3_t origin, struct bot_goal_s *goal);
	int		(*BotItemGoalInVisButNotVisible)(int viewer, vec3_t eye, vec3_t viewangles, struct bot_goal_s *goal);
	int		(*BotGetLevelItemGoal)(int index, char *classname, struct bot_goal_s *goal);
	int		(*BotGetNextCampSpotGoal)(int num, struct bot_goal_s *goal);
	int		(*BotGetMapLocationGoal)(char *name, struct bot_goal_s *goal);
	float	(*BotAvoidGoalTime)(int goalstate, int number);
	void	(*BotSetAvoidGoalTime)(int goalstate, int number, float avoidtime);
	void	(*BotInitLevelItems)(void);
	void	(*BotUpdateEntityItems)(void);
	int		(*BotLoadItemWeights)(int goalstate, char *filename);
	void	(*BotFreeItemWeights)(int goalstate);
	void	(*BotInterbreedGoalFuzzyLogic)(int parent1, int parent2, int child);
	void	(*BotSaveGoalFuzzyLogic)(int goalstate, char *filename);
	void	(*BotMutateGoalFuzzyLogic)(int goalstate, float range);
	int		(*BotAllocGoalState)(int client);
	void	(*BotFreeGoalState)(int handle);
	//-----------------------------------
	// be_ai_move.h
	//-----------------------------------
	void	(*BotResetMoveState)(int movestate);
	void	(*BotMoveToGoal)(struct bot_moveresult_s *result, int movestate, struct bot_goal_s *goal, int travelflags);
	int		(*BotMoveInDirection)(int movestate, vec3_t dir, float speed, int type);
	void	(*BotResetAvoidReach)(int movestate);
	void	(*BotResetLastAvoidReach)(int movestate);
	int		(*BotReachabilityArea)(vec3_t origin, int testground);
	int		(*BotMovementViewTarget)(int movestate, struct bot_goal_s *goal, int travelflags, float lookahead, vec3_t target);
	int		(*BotPredictVisiblePosition)(vec3_t origin, int areanum, struct bot_goal_s *goal, int travelflags, vec3_t target);
	int		(*BotAllocMoveState)(void);
	void	(*BotFreeMoveState)(int handle);
	void	(*BotInitMoveState)(int handle, struct bot_initmove_s *initmove);
	void	(*BotAddAvoidSpot)(int movestate, vec3_t origin, float radius, int type);
	//-----------------------------------
	// be_ai_weap.h
	//-----------------------------------
	int		(*BotChooseBestFightWeapon)(int weaponstate, int *inventory);
	void	(*BotGetWeaponInfo)(int weaponstate, int weapon, struct weaponinfo_s *weaponinfo);
	int		(*BotLoadWeaponWeights)(int weaponstate, char *filename);
	int		(*BotAllocWeaponState)(void);
	void	(*BotFreeWeaponState)(int weaponstate);
	void	(*BotResetWeaponState)(int weaponstate);
	//-----------------------------------
	// be_ai_gen.h
	//-----------------------------------
	int		(*GeneticParentsAndChildSelection)(int numranks, float *ranks, int *parent1, int *parent2, int *child);


	////////
	// BOTLIB STUFF
	////////
	// swiped from botlib_import_t, but cleaned up to make sense
	//print messages from the bot library
	void		(*Print)(int type, char *fmt, ...);
//	void	    (*Printf)( const char *fmt, ... );

	//trace a bbox through the world
//	void		(*Trace)(bsp_trace_t *trace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passent, int contentmask);
	//trace a bbox against a specific entity
//	void		(*EntityTrace)(bsp_trace_t *trace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int entnum, int contentmask);
	//retrieve the contents at the given point
	int			(*PointContents)(vec3_t point);
	//check if the point is in potential visible sight
//	int			(*inPVS)(vec3_t p1, vec3_t p2);
	//retrieve the BSP entity data lump
	char		*(*BSPEntityData)(void);
	//
	void		(*BSPModelMinsMaxsOrigin)(int modelnum, vec3_t angles, vec3_t mins, vec3_t maxs, vec3_t origin);
	//send a bot client command
	void		(*BotClientCommand)(int client, const char *command); // was char *, changed for c++ compilation
	//memory allocation
//	void		*(*GetMemory)(int size);		// allocate from Zone
//	void		(*FreeMemory)(void *ptr);		// free memory from Zone
	int			(*AvailableMemory)(void);		// available Zone memory
	void		*(*HunkAlloc)(int size);		// allocate from hunk
	//file system access
	int			(*FS_FOpenFile)( const char *qpath, fileHandle_t *file, fsMode_t mode );
//	int			(*FS_Read)( void *buffer, int len, fileHandle_t f );
//	int			(*FS_Write)( const void *buffer, int len, fileHandle_t f );
//	void		(*FS_FCloseFile)( fileHandle_t f );
	int			(*FS_Seek)( fileHandle_t f, long offset, int origin );
	//debug visualisation stuff
	int			(*DebugLineCreate)(void);
	void		(*DebugLineDelete)(int line);
	void		(*DebugLineShow)(int line, vec3_t start, vec3_t end, int color);
	//
	int			(*DebugPolygonCreate)(int color, int numPoints, vec3_t *points);
	void		(*DebugPolygonDelete)(int id);

// added for botlib import
	void		(*DropClient)( int clientNum, const char *reason ); // maps to SV_GameDropClient

	void		(*SV_GetServerinfo)( char *buffer, int bufferSize ) ;
	int			(*BotAllocateClient)(void); // maps to SV_BotAllocateClient
	int			(*BotGetSnapshotEntity)( int client, int ent ); // maps to SV_BotGetSnapshotEntity
	int			(*BotGetConsoleMessage)( int client, char *buf, int size ); // maps to SV_BotGetConsoleMessage

	//Area Awareness System functions
//	aas_export_t aas;
	//Elementary Action functions
//	ea_export_t ea;
	//AI functions
//	ai_export_t ai;
	//setup the bot library, returns BLERR_
	int (*BotLibSetup)(void);
	//shutdown the bot library, returns BLERR_
	int (*BotLibShutdown)(void);
	//sets a library variable returns BLERR_
	int (*BotLibVarSet)(char *var_name, char *value);
	//gets a library variable returns BLERR_
	int (*BotLibVarGet)(char *var_name, char *value, int size);

	//sets a C-like define returns BLERR_
	int (*PC_AddGlobalDefine)(char *string);
	int (*PC_LoadSourceHandle)(const char *filename);
	int (*PC_FreeSourceHandle)(int handle);
//	int (*PC_ReadTokenHandle)(int handle, pc_token_t *pc_token);
	int (*PC_SourceFileAndLine)(int handle, char *filename, int *line);

	//start a frame in the bot library
	int (*BotLibStartFrame)(float time);
	//load a new map in the bot library
	int (*BotLibLoadMap)(const char *mapname);
	//entity updates
	int (*BotLibUpdateEntity)(int ent, bot_entitystate_t *state);
	//just for testing
	int (*Test)(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);
	void (*BotUserCommand)(int cl, usercmd_t *cmd); // maps to SV_ClientThink(), gonna need a wrapper to go from clientnum to *client_t on exec side
	
// end botlib additions
   } game_import_t;

//
// functions exported by the game subsystem
//
typedef struct {
	int			apiversion;

	// the init function will only be called when a game starts,
	// not each time a level is loaded.  Persistant data for clients
	// and the server can be allocated in init
	void		   (*Init) ( int startTime, int randomSeed);
	void		   (*Shutdown) (void);
	void		   (*Cleanup) ( qboolean restart );

	// each new level entered will cause a call to SpawnEntities
	void		   (*SpawnEntities) (const char *mapname, const char *entstring, int levelTime);
	void		   (*PostLoad)( void );
	void		   (*PostSublevelLoad)( const char *spawnPosName );

	// return NULL if the client is allowed to connect, otherwise return
	// a text string with the reason for denial
	const char *(*ClientConnect)( int clientNum, qboolean firstTime, qboolean isBot, qboolean checkPassword );

	void		   (*ClientBegin)( gentity_t *ent, const usercmd_t *cmd );
	void		   (*ClientUserinfoChanged)( gentity_t *ent, const char *userinfo );
	void		   (*ClientDisconnect)( gentity_t *ent );
	void		   (*ClientCommand)( gentity_t *ent );
	void		   (*ClientThink)( gentity_t *ent, usercmd_t *cmd );

   void		   (*PrepFrame)( void );
	void		   (*RunFrame)( int levelTime, int frameTime );
	qboolean		(*SendEntity)( gentity_t *clientEntity, gentity_t *entityToSend );
	void			(*UpdateEntityStateForClient)( gentity_t *clientEntity, entityState_t *state );
	void			(*UpdatePlayerStateForClient)( gentity_t *clientEntity, playerState_t *state );
	void			(*ExtraEntitiesToSend)( gentity_t *clientEntity, int *numExtraEntities, int *extraEntities );
	int			(*GetEntityCurrentAnimFrame)( int entityNum, int bodyPart );

	// ConsoleCommand will be called when a command has been issued
	// that is not recognized as a builtin function.
	// The game can issue gi.argc() / gi.argv() commands to get the command
	// and parameters.  Return qfalse if the game doesn't recognize it as a command.
	qboolean	   (*ConsoleCommand)( void );

   // Read/Write Persistant is for storing persistant cross level information
	// about the world state and the clients.
	// WriteGame is called every time a level is exited.
	// ReadGame is called every time a level is entered.
	void		   (*WritePersistant)( const char *filename, qboolean sublevelTransition );
	qboolean    (*ReadPersistant)( const char *filename, qboolean sublevelTransition );

 	// ReadLevel is called after the default map information has been
	// loaded with SpawnEntities, so any stored client spawn spots will
	// be used when the clients reconnect.
	void		   (*WriteLevel)( const char *filename, qboolean autosave );
	qboolean	   (*ReadLevel)( const char *filename );
	qboolean	   (*LevelArchiveValid)( const char *filename );

	qboolean	   (*inMultiplayerGame)( void );
	qboolean	   (*isDefined)(const char *propname);
	const char *   (*getDefine)(const char *propname);

	////////
	//
	// BOTLIB unfortunate botlib graffiti, the following functions are required
	// for the executable to start a bot
	//
	////////
	int			(*BotAIStartFrame)(int time);	// executable hook to bot AI code loop
	void		(*AddBot_f)(void);				// used for sv_addbot_f


	
	int		(*GetTotalGameFrames)(void);
	//
	// global variables shared between game and server
	//

	// The gentities array is allocated in the game dll so it
	// can vary in size from one game to another.
	//
	// The size will be fixed when ge->Init() is called
	// the server can't just use pointer arithmetic on gentities, because the
	// server's sizeof(struct gentity_s) doesn't equal gentitySize
	struct gentity_s	*gentities;
	int			      gentitySize;
	int			      num_entities;		// current number, <= max_entities
	int			      max_entities;

   const char        *error_message;
} game_export_t;

game_export_t *GetGameApi (game_import_t *import);

int BotAISetup( int restart );
int BotAIShutdown( int restart );
int BotAILoadMap( int restart );
void G_InitBots(qboolean restart);
int BotAIShutdownClient(int client, qboolean restart);

#endif // __G_PUBLIC_H__
