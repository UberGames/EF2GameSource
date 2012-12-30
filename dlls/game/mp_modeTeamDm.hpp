//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/teamArena.h                                    $
// $Revision:: 20                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 7/17/02 4:00p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
// 
// 
// Description:
//

#ifndef __MP_MODETEAMDM_HPP__
#define __MP_MODETEAMDM_HPP__

#include "mp_modeTeamBase.hpp" // for base class MultiplayerModeBase and dependent headers

class ModeTeamDeathmatch : public ModeTeamBase
	{
	private:
		Team*                               _redTeam ;
		Team*                               _blueTeam ;

	protected:

	public:
		CLASS_PROTOTYPE( ModeTeamDeathmatch );
											ModeTeamDeathmatch();
											~ModeTeamDeathmatch();

		/* virtual */ void					init( int maxPlayers );

		void								playerKilled( Player *killedPlayer, Player *attackingPlayer, Entity *inflictor, int meansOfDeath );
		bool								isEndOfMatch( void );
		/* virtual */ int					getTeamPoints( Player *player );

		/* virtual */ void					setupMultiplayerUI( Player *player );

		/* virtual */ void					score( const Player *player );

		/* virtual */ bool					checkGameType( const char *rule );
	};

#endif // __MP_MODETEAMDM_HPP__
