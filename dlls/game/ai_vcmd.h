// Copyright (C) 1999-2000 Id Software, Inc.
//

/*****************************************************************************
 * name:		ai_vcmd.h
 *
 * desc:		Quake3 bot AI
 *
 * $Archive: /Code/DLLs/game/ai_vcmd.h $
 * $Author: Jwaters $ 
 * $Revision: 1 $
 * $Modtime: 7/25/02 11:48a $
 * $Date: 7/30/02 1:10p $
 *
 *****************************************************************************/

int BotVoiceChatCommand(bot_state_t *bs, int mode, char *voicechat);
void BotVoiceChat_Defend(bot_state_t *bs, int client, int mode);


