// Copyright (C) 1999-2000 Id Software, Inc.
//

/*****************************************************************************
 * name:		ai_cmd.h
 *
 * desc:		Quake3 bot AI
 *
 * $Archive: /Code/DLLs/game/ai_cmd.h $
 * $Author: Jwaters $ 
 * $Revision: 1 $
 * $Modtime: 7/25/02 11:48a $
 * $Date: 7/30/02 1:10p $
 *
 *****************************************************************************/

extern int notleader[MAX_CLIENTS];

int BotMatchMessage(bot_state_t *bs, char *message);
void BotPrintTeamGoal(bot_state_t *bs);

