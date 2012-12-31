//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/gamecmds.cpp                              $
// $Revision:: 54                                                             $
//     $Date:: 9/26/03 2:36p                                                  $
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

#include "_pch_cpp.h"
#include "gamecmds.h"
#include "camera.h"
#include "viewthing.h"
#include "soundman.h"
#include "navigate.h"
#include "mp_manager.hpp"
#include "CinematicArmature.h"
#include <qcommon/gameplaymanager.h>
#include "botmenudef.h"

typedef struct
{
	const char  *command;
	qboolean    ( *func )( const gentity_t *ent );
	qboolean    allclients;
} consolecmd_t;

consolecmd_t G_ConsoleCmds[] =
{
	//   command name       function             available in multiplayer?
	{ "vtaunt",				G_VTaunt,				true },
//	{ "vsay_team",			G_VTaunt,				true },
//	{ "vosay_team",			G_VTaunt,				true },
//	{ "vtell",				G_VTaunt,				true },
	{ "vsay",				G_SayCmd,				true },
	{ "vosay",				G_SayCmd,				true },
	{ "tell",				G_TellCmd,				true },
	{ "vtell",				G_TellCmd,				true },
	{ "say",				G_SayCmd,				true },
	{ "taunt",				G_TauntCmd,				true },
	{ "vosay_team",			G_TeamSayCmd,			true },
	{ "vsay_team",			G_TeamSayCmd,			true },
	{ "tsay",				G_TeamSayCmd,			true },
	{ "say_team",			G_TeamSayCmd,			true }, // added for BOTLIB
	{ "eventlist",			G_EventListCmd,			false },
	{ "pendingevents",		G_PendingEventsCmd,		false },
	{ "eventhelp",			G_EventHelpCmd,			false },
	{ "dumpevents",			G_DumpEventsCmd,		false },
	{ "classevents",		G_ClassEventsCmd,		false },
	{ "dumpclassevents",	G_DumpClassEventsCmd,	false },
	{ "dumpallclasses",		G_DumpAllClassesCmd,	false },
	{ "classlist",			G_ClassListCmd,			false },
	{ "classtree",			G_ClassTreeCmd,			false },
	{ "cam",				G_CameraCmd,			false },
	{ "snd",				G_SoundCmd,				false },
	{ "cin",				G_CinematicCmd,			false },
//	{ "showvar",			G_ShowVarCmd,			false },
	{ "script",				G_ScriptCmd,			false },
	{ "clientrunthread",	G_ClientRunThreadCmd,	false },
	{ "clientsetvar",		G_ClientSetVarCmd,		false },
	{ "levelvars",			G_LevelVarsCmd,			false },
	{ "gamevars",			G_GameVarsCmd,			false },
	{ "loc",				G_LocCmd,				false },
	{ "warp",				G_WarpCmd,				false },
	{ "mask",				G_MaskCmd,				false },
	{ "setgameplayfloat",	G_SetGameplayFloatCmd,	false },
	{ "setgameplaystring",	G_SetGameplayStringCmd, false },
	{ "purchaseSkill",		G_PurchaseSkillCmd,		false },
	{ "swapItem",			G_SwapItemCmd,			false },
	{ "dropItem",			G_DropItemCmd,			false },
	{ "dialogrunthread",	G_DialogRunThread,		false },
	{ NULL,					NULL,					NULL }
};

void G_InitConsoleCommands( void )
{
	consolecmd_t *cmds;
	
	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	gi.AddCommand( "give" );
	gi.AddCommand( "god" );
	gi.AddCommand( "notarget" );
	gi.AddCommand( "noclip" );
	gi.AddCommand( "kill" );
	gi.AddCommand( "script" );
	
	for( cmds = G_ConsoleCmds; cmds->command != NULL; cmds++ )
	{
		gi.AddCommand( cmds->command );
	}
}

qboolean	G_ConsoleCommand( void )
{
	gentity_t *ent;
	qboolean result;
	
	result = false;
	try
	{
		if ( dedicated->integer )
		{
			const char *cmd;

			cmd = gi.argv( 0 );

			if ( stricmp( cmd, "say" ) == 0 )
			{
				G_Say( NULL, false, false );
				result = true;
			}
		}

		if ( !result )
		{
			ent = &g_entities[ 0 ];
			result = G_ProcessClientCommand( ent );
		}
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
	
	return result;
}

void G_ClientCommand( gentity_t *ent )
{
	try
	{
		if ( ent && !G_ProcessClientCommand( ent ) )
		{
			// anything that doesn't match a command will be a chat
			G_Say( ent, false, true );
		}
	}
	
	catch( const char *error )
	{
		G_ExitWithError( error );
	}
}

qboolean G_ProcessClientCommand( gentity_t *ent )
{
	const char	   *cmd;
	consolecmd_t   *cmds;
	int			   i;
	int            n;
	Event			   *ev;
	
	cmd = gi.argv( 0 );

	if ( !ent || !ent->client || !ent->entity || !ent->inuse )
	{
		// not fully in game yet
		return false;
	}
	
	for( cmds = G_ConsoleCmds; cmds->command != NULL; cmds++ )
	{
		// if we have multiple clients and this command isn't allowed by multiple clients, skip it
		if ( ( game.maxclients > 1 ) && ( !cmds->allclients ) && !sv_cheats->integer )
		{
			continue;
		}
		
		if ( !Q_stricmp( cmd, cmds->command ) )
		{
			return cmds->func( ent );
		}
	}
	
	if ( Event::Exists( cmd ) )
	{
		ev = new Event( cmd );
		ev->SetSource( EV_FROM_CONSOLE );
		ev->SetConsoleEdict( ent );
		
		n = gi.argc();
		for( i = 1; i < n; i++ )
		{
			ev->AddToken( gi.argv( i ) );
		}
		
		if ( !Q_stricmpn( cmd, "ai_", 2 ) )
		{
			return thePathManager.ProcessEvent( ev );
		}
		else if ( !Q_stricmpn( cmd, "view", 4 ) )
		{
			return Viewmodel.ProcessEvent( ev );
		}
		else
		{
			if( ent && ent->entity )
				return ent->entity->ProcessEvent( ev );
		}
	}
	
	return false;
}

/*
==================
Cmd_Say_f
==================
*/
void G_Say( const gentity_t *ent, bool team, qboolean arg0 )
{
	str text;
	const char	*p;
	
	if ( gi.argc() < 2 && !arg0 )
	{
		return;
	}
	
	if ( arg0 )
	{
		text = gi.argv( 0 );
		text += " ";
		text += gi.args();
	}
	else
	{
		p = gi.args();
		
		if ( *p == '"' )
		{
			p++;
			text = p;
			text[ text.length() - 1 ] = 0;
		}
		else
		{
			text = p;
		}
	}
	
	if ( ent && ent->entity && ent->entity->isSubclassOf( Player ) )
	{
		multiplayerManager.say( (Player *)ent->entity, text, team );
	}
	else
	{
		multiplayerManager.say( NULL, text, false );
	}
}

qboolean G_CameraCmd( const gentity_t *ent )
{
	Event *ev;
	const char *cmd;
	int   i;
	int   n;

	Q_UNUSED(ent);
	
	n = gi.argc();
	if ( !n )
	{
		gi.WPrintf( "Usage: cam [command] [arg 1]...[arg n]\n" );
		return true;
	}
	
	cmd = gi.argv( 1 );
	if ( Event::Exists( cmd ) )
	{
		ev = new Event( cmd );
		ev->SetSource( EV_FROM_CONSOLE );
		ev->SetConsoleEdict( NULL );
		
		for( i = 2; i < n; i++ )
		{
			ev->AddToken( gi.argv( i ) );
		}
		
		CameraMan.ProcessEvent( ev );
	}
	else
	{
		gi.WPrintf( "Unknown camera command '%s'.\n", cmd );
	}
	
	return true;
}

//===============================================================
// Name:		G_CinematicCmd
// Class:		
//
// Description: Takes a cinematic command from the console, rips
//				off the first argument and looks up the second
//				as a cinematic armature event.  If found, sends
//				it off to the armature along with the rest of the
//				event tokens.
// 
// Parameters:	gentity_t* -- entity sending the event
//
// Returns:		qboolean -- true if processed.
// 
//===============================================================
qboolean G_CinematicCmd( const gentity_t *ent )
{
	Event		*ev ;
	const char	*cmd ;
	int			 n		= gi.argc();
	
	Q_UNUSED(ent);

	if ( !n )
	{
		gi.WPrintf( "Usage: cin <command> <arg1> ... <argn>\n");
		return true ;
	}
	
	cmd = gi.argv( 1 );
	if ( Event::Exists( cmd ) )
	{
		ev = new Event( cmd );
		ev->SetSource( EV_FROM_CONSOLE );
		ev->SetConsoleEdict( NULL );
		
		for (int i = 2; i < n; i++ )
		{
			ev->AddToken( gi.argv( i ) );
		}
		
		theCinematicArmature.ProcessEvent( ev );
	}
	else
	{
		gi.WPrintf( "Unknown cinematic armature command '%s'.\n", cmd );
	}
	
	return true ;
}

qboolean G_SoundCmd( const gentity_t *ent )
{
	Event *ev;
	const char *cmd;
	int   i;
	int   n;

	Q_UNUSED(ent);
	
	n = gi.argc();
	if ( !n )
	{
		gi.WPrintf( "Usage: snd [command] [arg 1]...[arg n]\n" );
		return true;
	}
	
	cmd = gi.argv( 1 );
	if ( Event::Exists( cmd ) )
	{
		ev = new Event( cmd );
		ev->SetSource( EV_FROM_CONSOLE );
		ev->SetConsoleEdict( NULL );
		
		for( i = 2; i < n; i++ )
		{
			ev->AddToken( gi.argv( i ) );
		}
		
		SoundMan.ProcessEvent( ev );
	}
	else
	{
		gi.WPrintf( "Unknown sound command '%s'.\n", cmd );
	}
	
	return true;
}

char *ClientName(int client, char *name, int size);
qboolean G_VTaunt( const gentity_t *ent )
{
	Q_UNUSED(ent);
// NOTE: vtant, vsay, vosay are q3's specific hotkey chat mechanisms.  we don't have these so this is a stub
// function right now.
/*
	gentity_t *who;
	int i;

	if (!ent->client) {
		return qfalse;
	}


	Player *player = (Player *)ent->entity;
	Player *enemy = multiplayerManager.getLastKilledByPlayer(player);

	// insult someone who just killed you
	if (ent->enemy && ent->enemy->client && ent->enemy->client->lastkilled_client == ent->s.number) {
		// i am a dead corpse
		if (!(ent->enemy->r.svFlags & SVF_BOT)) {
			G_Voice( ent, ent->enemy, SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		if (!(ent->r.svFlags & SVF_BOT)) {
			G_Voice( ent, ent,        SAY_TELL, VOICECHAT_DEATHINSULT, qfalse );
		}
		ent->enemy = NULL;
		return;
	}
	// insult someone you just killed
	if (ent->client->lastkilled_client >= 0 && ent->client->lastkilled_client != ent->s.number) {
		who = g_entities + ent->client->lastkilled_client;
		if (who->client) {
			// who is the person I just killed
			if (who->client->lasthurt_mod == MOD_GAUNTLET) {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );	// and I killed them with a gauntlet
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse );
				}
			} else {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice( ent, who, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );	// and I killed them with something else
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice( ent, ent, SAY_TELL, VOICECHAT_KILLINSULT, qfalse );
				}
			}
			ent->client->lastkilled_client = -1;
			return;
		}
	}

	if (gametype >= GT_TEAM) {
		// praise a team mate who just got a reward
		for(i = 0; i < MAX_CLIENTS; i++) {
			who = g_entities + i;
			if (who->client && who != ent && who->client->sess.sessionTeam == ent->client->sess.sessionTeam) {
				if (who->client->rewardTime > level.time) {
					if (!(who->r.svFlags & SVF_BOT)) {
						G_Voice( ent, who, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					if (!(ent->r.svFlags & SVF_BOT)) {
						G_Voice( ent, ent, SAY_TELL, VOICECHAT_PRAISE, qfalse );
					}
					return;
				}
			}
		}
	}
*/
	// just say something
/*
	cmd = "vchat";
	}

	trap_SendServerCommand( other-g_entities, va("%s %d %d %d %s", cmd, voiceonly, ent->s.number, color, id));
*/

//	trap_SendServerCommand( other-g_entities, va("%s %d %d %d %s", cmd, voiceonly, ent->s.number, color, id));
/*
	GameplayManager
	gi.SendServerCommand(NULL,va("%s %d %d %d %s", "taunt", voiceonly, ent->s.number, color, id));
	//	G_Say( ent,false, va("%s","taunt") ); // VOICECHAT_TAUNT)); //  NULL, SAY_ALL, VOICECHAT_TAUNT, qfalse );
	char bogoname[1024];

	gi.SendServerCommand(ent->s.clientNum,va("hudsay \"%s: %s \"",ClientName(ent->s.clientNum,bogoname,1024),"taunt")); // ClientName(ent->s.clientNum,bogoname,1024),
*/

	return true;
}

qboolean G_TellCmd( const gentity_t *ent )
{
	str text;
	const char	*p;
	int i;
	int entnum;
	
	if ( gi.argc() < 3 )
	{
		return true;
	}

	entnum = atoi( gi.argv( 1 ) );
	
	for ( i = 2 ; i < gi.argc() ; i++ )
	{
		p = gi.argv( i );
	
		if ( *p == '"' )
		{
			p++;
			text += p;
			text[ text.length() - 1 ] = 0;
		}
		else
		{
			text += p;
		}

	}
	
	if ( ent->entity && ent->entity->isSubclassOf( Player ) )
	{
		multiplayerManager.tell( (Player *)ent->entity, text, entnum );
	}

	return true;
}

qboolean G_SayCmd( const gentity_t *ent )
{
	G_Say( ent, false, false );

	return true;
}

qboolean G_TeamSayCmd( const gentity_t *ent )
{
	G_Say( ent, true, false );

	return true;
}

qboolean G_TauntCmd( const gentity_t *ent )
{
	str tauntName;
	
	if ( gi.argc() < 2 )
	{
		return true;
	}
	
	tauntName = "taunt";
	tauntName += gi.argv( 1 );
	
	if ( ent->entity && ent->entity->isSubclassOf( Player ) )
	{
		Player *player = (Player *)ent->entity;

		if ( multiplayerManager.inMultiplayer() && !multiplayerManager.isPlayerSpectator( player ) )
		{
			//ent->entity->Sound( tauntName, CHAN_TAUNT, DEFAULT_VOL, LEVEL_WIDE_MIN_DIST );
			ent->entity->Sound( tauntName, CHAN_TAUNT, DEFAULT_VOL, 250.0f );
		}
	}

	return true;
}

qboolean G_LocCmd( const gentity_t *ent )
{
	if ( ent )
	{
		gi.Printf( "Origin = ( %f, %f, %f )   Angles = ( %f, %f, %f )\n", 
			ent->currentOrigin[0], ent->currentOrigin[1], ent->currentOrigin[2],
			ent->currentAngles[0], ent->currentAngles[1], ent->currentAngles[0] );
	}
	
	return true;
}

qboolean G_WarpCmd( const gentity_t *ent )
{
	Vector pos;

	if ( sv_cheats->integer == 0 )
	{
		return true;
	}
	
	if ( ent )
	{
		Entity *entity;
		
		if ( ent->entity )
		{
			// Get the new position
			
			if ( gi.argc() == 2 )
			{
				pos = Vector(gi.argv( 1 ));
			}
			else if ( gi.argc() == 4 )
			{
				pos[ 0 ] = atof( gi.argv( 1 ) );
				pos[ 1 ] = atof( gi.argv( 2 ) );
				pos[ 2 ] = atof( gi.argv( 3 ) );
			}
			else
			{
				gi.Printf( "Incorrect parms\n" );
				return false;
			}
			
			// Move the entity to the new position
			
			entity = ent->entity;
			entity->setOrigin( pos );
		}
	}
	
	return true;
}

qboolean G_MaskCmd( const gentity_t *ent )
{
	Vector pos;
	
	// Check parms
	
	if ( gi.argc() != 2 )
	{
		gi.Printf( "Incorrect parms\n" );
		return true;
	}
	
	if ( ent )
	{
		Entity *entity;
		
		if ( ent->entity )
		{
			entity = ent->entity;
			
			// Get and set new mask
			
			if ( stricmp( gi.argv( 1 ), "monster" ) == 0 )
			{
				entity->edict->clipmask	 = MASK_MONSTERSOLID;
			}
			else if ( stricmp( gi.argv( 1 ), "player" ) == 0 )
			{
				entity->edict->clipmask	 = MASK_PLAYERSOLID;
			}
			else
			{
				gi.Printf( "Unknown mask name - %s\n", gi.argv( 1 ) );
			}
		}
	}
	
	return true;
}

qboolean G_EventListCmd( const gentity_t *ent )
{
	const char *mask;
	
	Q_UNUSED(ent);

	mask = NULL;
	if ( gi.argc() > 1 )
	{
		mask = gi.argv( 1 );
	}
	
	Event::ListCommands( mask );
	
	return true;
}

qboolean G_PendingEventsCmd( const gentity_t *ent )
{
	const char *mask;

	Q_UNUSED(ent);
	
	mask = NULL;
	if ( gi.argc() > 1 )
	{
		mask = gi.argv( 1 );
	}
	
	Event::PendingEvents( mask );
	
	return true;
}

qboolean G_EventHelpCmd( const gentity_t *ent )
{
	const char *mask;
	
	Q_UNUSED(ent);

	mask = NULL;
	if ( gi.argc() > 1 )
	{
		mask = gi.argv( 1 );
	}
	
	Event::ListDocumentation( mask, false );
	
	return true;
}

qboolean G_DumpEventsCmd( const gentity_t *ent )
{
	const char *mask;
	
	Q_UNUSED(ent);

	mask = NULL;
	if ( gi.argc() > 1 )
	{
		mask = gi.argv( 1 );
	}
	
	Event::ListDocumentation( mask, true );
	
	return true;
}

qboolean G_ClassEventsCmd( const gentity_t *ent )
{
	const char *className;

	Q_UNUSED(ent);
	
	className = NULL;
	if ( gi.argc() < 2 )
	{
		gi.WPrintf( "Usage: classevents [className]\n" );
		className = gi.argv( 1 );
	}
	else
	{
		className = gi.argv( 1 );
		ClassEvents( className );
	}
	return true;
}

qboolean G_DumpClassEventsCmd( const gentity_t *ent )
{
	const char *className;
	
	Q_UNUSED(ent);

	className = NULL;
	if ( gi.argc() < 2 )
	{
		gi.WPrintf( "Usage: dumpclassevents [className]\n" );
		className = gi.argv( 1 );
	}
	else
	{
		className = gi.argv( 1 );
		ClassEvents( className, true );
	}
	return true;
}

qboolean G_DumpAllClassesCmd( const gentity_t *ent )
{
	const char *tmpstr = NULL;
	const char *filename = NULL;
	int typeFlag = EVENT_ALL;
	int outputFlag = OUTPUT_ALL;

	Q_UNUSED(ent);
	
	if ( gi.argc() > 1 )
	{   
		tmpstr = gi.argv( 1 );
		if ( !strcmp("tiki", tmpstr ) )
			typeFlag = EVENT_TIKI_ONLY;
		if ( !strcmp("script", tmpstr ) )
			typeFlag = EVENT_SCRIPT_ONLY;
	}
	if ( gi.argc() > 2 )
	{
		tmpstr = gi.argv( 2 );
		if ( !strcmp("html", tmpstr ) )
			outputFlag = OUTPUT_HTML;
		if ( !strcmp("cmdmap", tmpstr ) )
			outputFlag = OUTPUT_CMD;
	}
	if ( gi.argc() > 3 )
	{
		filename = gi.argv( 3 );
	}
	
	DumpAllClasses(typeFlag, outputFlag, filename);
	
	return true;
}

qboolean G_ClassListCmd( const gentity_t *ent )
{
	Q_UNUSED(ent);

	listAllClasses();
	
	return true;
}

qboolean G_ClassTreeCmd( const gentity_t *ent )
{
	if ( gi.argc() > 1 )
	{
		listInheritanceOrder( gi.argv( 1 ) );
	}
	else
	{
		gi.SendServerCommand( ent - g_entities, "print \"Syntax: classtree [classname].\n\"" );
	}
	
	return true;
}

/*
qboolean G_ShowVarCmd( gentity_t *ent )
{
	ScriptVariable *var;

	var = Director.GetExistingVariable( gi.argv( 1 ) );
	if ( var )
	{
		gi.SendServerCommand( ent - g_entities, "print \"%s = '%s'\n\"", gi.argv( 1 ), var->stringValue() );
	}
	else
	{
		gi.SendServerCommand( ent - g_entities, "print \"Variable '%s' does not exist.\"", gi.argv( 1 ) );
	}
  
	return true;
} */

qboolean G_ScriptCmd( const gentity_t *ent )
{
	int i, argc;
	const char *argv[ 32 ];
	char args[ 32 ][ 64 ];

	Q_UNUSED(ent);

	argc = 0;
	for( i = 1; i < gi.argc(); i++ )
	{
		if ( argc < 32 )
		{
			strncpy( args[ argc ], gi.argv( i ), 64 );
			argv[ argc ] = args[ argc ];
			argc++;
		}
	}
	if ( argc > 0 )
	{
		level.consoleThread->ProcessCommand( argc, argv );
	}
	
	return true;
}

qboolean G_ClientRunThreadCmd( const gentity_t *ent )
{
	str		threadName;
	CThread	*thread;
	
	Q_UNUSED(ent);

	// Get the thread name
	
	if ( !gi.argc() )
		return true;
	
	threadName = gi.argv( 1 );
	
	
	// Check to make sure player is allowed to run this thread
	
	// Need to do this part
	
	// Run the thread
	
	if ( !threadName.length() )
		return true;
	
	thread = Director.CreateThread( threadName );
	
	if ( thread )
		thread->DelayedStart( 0.0f );
	
	return true;
}

qboolean G_ClientSetVarCmd( const gentity_t *ent )
{
	str		varName;
	str		value;
	
	Q_UNUSED(ent);
	
	if ( gi.argc() != 3 )
		return true;
	
	// Get the variable name
	
	varName = gi.argv( 1 );
	
	// Get the variable value
	
	value = gi.argv( 2 );
	
	// Check to make sure player is allowed to set this variable
	
	// Need to do this part
	
	// Set the variable
	
	if ( varName.length() && value.length() )
	{
		levelVars.SetVariable( varName, value );
	}
	
	return true;
}

//===============================================================
// Name:		G_SendCommandToAllPlayers
// Class:		None
//
// Description: Sends the specified command to all connected
//				clients (players).
// 
// Parameters:	const char* -- the command to send
//
// Returns:		qboolean -- qtrue if successfully sent.
// 
//===============================================================
qboolean G_SendCommandToAllPlayers( const char *command )
{
	bool retVal = true ;

	for( int clientIdx = 0; clientIdx < maxclients->integer; ++clientIdx )
	{
		gentity_t *ent = g_entities + clientIdx ;
		if ( !ent->inuse || !ent->client || !ent->entity ) continue;

		if ( !G_SendCommandToPlayer( ent, command ) ) retVal = false ;
	}

	return retVal ;
}


//===============================================================
// Name:		G_SendCommandToPlayer
// Class:		
//
// Description: Sends the specified command to the specified player.
// 
// Parameters:	Player* -- the player to send it to
//				str		-- the command
//
// Returns:		None
// 
//===============================================================
qboolean G_SendCommandToPlayer( const gentity_t *ent, const char *command )
{
	assert( ent );
	Entity *entity = ent->entity ;
	
	assert( entity );
	assert( entity->isSubclassOf( Player ) );
	Player *player = ( Player* )entity ;
	
	str		builtCommand("stufftext \"");
	builtCommand += command ;
	builtCommand += "\"\n";
	gi.SendServerCommand( player->edict - g_entities, builtCommand.c_str() );
	
	return true ;
}


//===============================================================
// Name:		G_EnableWidgetOfPlayer
// Class:		
//
// Description: Enables (or disables) the specified widget of 
//				the specified player.
// 
// Parameters:	const gentity_t*	-- the player gentity_t to send this command to.
//				const char*			-- the name of the widget to enable/disable
//				bool				-- true means enable
//
// Returns:		None
// 
//===============================================================
qboolean G_EnableWidgetOfPlayer( const gentity_t *ent, const char *widgetName, qboolean enableFlag )
{
	assert( widgetName );

	str command("globalwidgetcommand ");
	command += widgetName ;
	command += ( enableFlag ) ? " enable" : " disable" ;

	return G_SendCommandToPlayer( ent, command.c_str());
}


//===============================================================
// Name:		G_SetWidgetTextOfPlayer
// Class:		
//
// Description: Sets the widget text of the widget for the speicifed player
// 
// Parameters:	const gentity_t*	-- the player gentity_t to send this command to.
//				const char*			-- the name of the widget to set the text on
//				const char*			-- text the put on the widget
//
// Returns:		None
// 
//===============================================================
qboolean G_SetWidgetTextOfPlayer( const gentity_t *ent, const char *widgetName, const char *widgetText )
{
	assert( widgetName );
	char tmpstr[4096];
	if ( strlen(widgetText) > 4095 )
		assert(0);
	
	strcpy(tmpstr, widgetText);
	
	for ( unsigned i = 0; i < strlen(widgetText); i++ )
	{
		if ( tmpstr[i] == '\n' )
			tmpstr[i] = '~';
		if ( tmpstr[i] == ' ' )
			tmpstr[i] = '^';
	}
	
	str command("globalwidgetcommand ");
	command += widgetName ;
	command += " labeltext " ;
	command += tmpstr ;
	
	return G_SendCommandToPlayer( ent, command.c_str());
}

void PrintVariableList( ScriptVariableList * list )
{
	ScriptVariable *var;
	int i;
	
	for( i = 1; i <= list->NumVariables(); i++ )
	{
		var = list->GetVariable( i );
		gi.Printf( "%s = %s\n", var->getName(), var->stringValue() );
	}
	gi.Printf( "%d variables\n", list->NumVariables() );
}

qboolean G_LevelVarsCmd( const gentity_t *ent )
{
	Q_UNUSED(ent);

	gi.Printf( "Level Variables\n" );
	PrintVariableList( &levelVars );
	
	return true;
}

qboolean G_GameVarsCmd( const gentity_t *ent )
{
	Q_UNUSED(ent);

	gi.Printf( "Game Variables\n" );
	PrintVariableList( &gameVars );
	
	return true;
}

//--------------------------------------------------------------
//
// Name:			G_SetGameplayFloatCmd
// Class:			None
//
// Description:		Sets properties on the gameplay database.
//
// Parameters:		const gentity_t *ent -- Entity, not used
//
// Returns:			qboolean
//
//--------------------------------------------------------------
qboolean G_SetGameplayFloatCmd( const gentity_t *ent )
{
	str objname;
	str propname;
	str create = "0";
	float value = 1.0f;

	Q_UNUSED(ent);

	// Check for not enough args
	if ( gi.argc() < 4 )
		return qfalse;

	objname = gi.argv( 1 );
	propname = gi.argv( 2 );
	value = (double)atof(gi.argv( 3 ));
	if ( gi.argc() > 4 )
		create = gi.argv( 4 );

	if ( create == "0" )
		GameplayManager::getTheGameplayManager()->setFloatValue(objname, propname, value);
	else
		GameplayManager::getTheGameplayManager()->setFloatValue(objname, propname, value, true);

	gi.Printf("Gameplay Modified -- %s's %s changed to %g\n", objname.c_str(), propname.c_str(), value);

	return qtrue;
}


//--------------------------------------------------------------
//
// Name:			G_SetGameplayStringCmd
// Class:			None
//
// Description:		Sets properties on the gameplay database.
//
// Parameters:		const gentity_t *ent -- Entity, not used
//
// Returns:			qboolean
//
//--------------------------------------------------------------
qboolean G_SetGameplayStringCmd( const gentity_t *ent )
{
	str objname;
	str propname;
	str valuestr;
	str create = "0";

	Q_UNUSED(ent);

	// Check for not enough args
	if ( gi.argc() < 4 )
		return qfalse;

	objname = gi.argv( 1 );
	propname = gi.argv( 2 );
	valuestr = gi.argv( 3 );
	if ( gi.argc() > 4 )
		create = gi.argv( 4 );

	if ( create == "0" )
		GameplayManager::getTheGameplayManager()->setStringValue(objname, propname, valuestr);
	else
		GameplayManager::getTheGameplayManager()->setStringValue(objname, propname, valuestr, true);

	gi.Printf("Gameplay Modified -- %s's %s changed to %s\n", objname.c_str(), propname.c_str(), valuestr.c_str());

	return qtrue;
}


//===============================================================
// Name:		G_PurchaseSkillCmd
// Class:		None
//
// Description: Purchases a skill for the current player.  This
//				goes into the gameplay database and looks for a
//				CurrentPlayer object who has a skillPoints property.
//
//				If its found, we attempt to increment the value
//				of the skill specified in the command's first argument.
//				We also decrement by one the number of available skillPoints.
// 
// Parameters:	gentity_t -- the entity issuing the command. Not used.
//
// Returns:		qboolean -- true if the command was executed.
// 
//===============================================================
qboolean G_PurchaseSkillCmd( const gentity_t *ent )
{
	str propname ;

	if ( gi.argc() < 1 ) return qfalse ;

	propname = gi.argv( 1 );

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm ) return false ;

	float skillPoints = gpm->getFloatValue( "CurrentPlayer", "SkillPoints" );
	if ( skillPoints > 0.0f )
	{
		str objName("CurrentPlayer.");
		objName += propname ;
		float skillValue = gpm->getFloatValue( objName, "value" );
		if ( skillValue < gpm->getFloatValue( objName, "max" ) )
		{
			gpm->setFloatValue( objName, "value", skillValue + 1.0f );
			gpm->setFloatValue( "CurrentPlayer", "SkillPoints", skillPoints - 1.0f );

			if ( ent->entity->isSubclassOf( Player ) )
			{
				Player *player = (Player*)ent->entity;
				player->skillChanged( objName );
			}
		}
	}

	return qtrue ;
}


//===============================================================
// Name:		G_SwapItemCmd
// Class:		None
//
// Description: Swaps an inventory item with the currently held item
//				of that type.  This is fairly specific at this point
//				to use the Gameplay Database in a particular way.
//
//				The command takes a single string, which defines an
//				inventory slot object.  We retrieve the name of the
//				item in that inventory slot.  If it isn't found, there
//				isn't an item there and nothing happens.
//
//				If it is found, we determine if the player already has
//				an item of that type in their local inventory (hands).
//				This is done by again checking the database.  If they
//				do have a weapon of that type, we swap this one with
//				that.  Otherwise we just give him this weapon.
// 
// Parameters:	gentity_t -- the entity issuing the command.  This
//							 is the player who issued the command.
//
// Returns:		qboolean -- true if the command was executed.
// 
//===============================================================
qboolean G_SwapItemCmd( const gentity_t *ent )
{
	if ( gi.argc() < 1 ) return qfalse ;
	if ( !ent->entity->isSubclassOf( Player ) ) return qfalse ;
	
	Player		*player			= (Player*)ent->entity ;
	str			 objectName		= gi.argv( 1 );
	str			 propertyName	= gi.argv( 2 );
	str			 heldItem ;
	weaponhand_t hand			= WEAPON_RIGHT;
	
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm ) return false ;
	
	str itemName	= gpm->getStringValue( objectName, propertyName );
	str tikiName	= gpm->getStringValue( itemName, "model" );
	str itemType	= gpm->getStringValue( itemName, "class" );
	str	playerItem("CurrentPlayer." + itemType);
	
	if ( !gpm->hasObject(playerItem) )
	{
		gi.WPrintf( "Warning: Unknown item type %s for item %s", itemType.c_str(), itemName.c_str() );
		return qfalse ;
	}
	
	// Get the currently held item, and replace it with the item we clicked on
	heldItem = gpm->getStringValue( playerItem, "name" );
	gpm->setStringValue( playerItem, "name", itemName );
	
	// Ranged weapons are left hand
	if ( itemType == "Ranged" )
		hand = WEAPON_LEFT ;
	
	// Remove the currently held item from the player's inventory
	player->takeItem( heldItem.c_str() );
	
	// Give the player the new item we clicked on
	player->giveItem( tikiName );
	
	// Use the weapon we clicked on if we picked a weapon of the same type
	// that we have currently equipped.  Otherwise, we just swap out
	// the slots
	Weapon *weap = 0;
	weap = player->GetActiveWeapon(WEAPON_RIGHT);
	if ( !weap )
		weap = player->GetActiveWeapon(WEAPON_LEFT);
	if ( weap )
	{
		str tmpstr = gpm->getStringValue(weap->getArchetype(), "class");
		if ( tmpstr == itemType )
			player->useWeapon( itemType, hand );
	}
	
	// Put the held item in inventory slot the clicked item was in.
	gpm->setStringValue( objectName, propertyName, heldItem );
	
	return qtrue ;
}

//--------------------------------------------------------------
//
// Name:			G_DropItemCmd
// Class:			None
//
// Description:		Drops an item that is clicked on from the inventory
//
// Parameters:		gentity_t -- the entity issuing the command.  This
//							 is the player who issued the command.
//
// Returns:			qboolean -- true if the command was executed.	
//
//--------------------------------------------------------------
qboolean G_DropItemCmd( const gentity_t *ent )
{
	if ( gi.argc() < 1 ) return false ;
	if ( !ent->entity->isSubclassOf( Player ) ) return false ;
	
	Player		*player			= (Player*)ent->entity ;
	str			 objectName		= gi.argv( 1 );
	str			 propertyName	= gi.argv( 2 );
	
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm ) return false ;
	
	str itemName	= gpm->getStringValue( objectName, propertyName );
	if ( itemName == "Empty" )
		return false;
	
	str tikiName	= gpm->getStringValue( itemName, "model" );
	str itemType	= gpm->getStringValue( itemName, "class" );
	str	playerItem("CurrentPlayer." + itemType);
	
	if ( !gpm->hasObject(playerItem) )
	{
		gi.WPrintf( "Warning: Unknown item type %s for item %s", itemType.c_str(), itemName.c_str() );
		return false ;
	}
	
	// Empty the slot
	gpm->setStringValue( objectName, propertyName, "Empty" );
	
	// Give the player the item to drop
	//Item *givenItem = player->giveItem(tikiName);
	
	Weapon *dropweap;
	ClassDef *cls;
	cls = getClass( tikiName.c_str() );
	if ( !cls )
	{
		SpawnArgs args;
		args.setArg( "model", tikiName.c_str() );
		cls = args.getClassDef();
		if ( !cls )
			return false;
	}
	dropweap = ( Weapon * )cls->newInstance();
	dropweap->setModel( tikiName.c_str() );
	dropweap->ProcessPendingEvents();
	dropweap->SetOwner(player);
	dropweap->hideModel();
	dropweap->setAttached(true);
	dropweap->Drop();
	
	return true ;
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
qboolean G_DialogRunThread( const gentity_t *ent )
{
	str		threadName;
	
	// clear out the current dialog actor
	if(ent->entity->isSubclassOf(Player))
	{
		Player* player = (Player*)ent->entity;
		player->clearBranchDialogActor();
	}

	return G_ClientRunThreadCmd( ent );
}