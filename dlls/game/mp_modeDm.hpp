//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/multiplayerArena.h                             $
// $Revision:: 38                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 7/23/02 3:55p                                                  $
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

#ifndef __MP_MODEDM_HPP__
#define __MP_MODEDM_HPP__

#include "mp_modeBase.hpp"

//-----------------------------------------------------------------------
// ModeDeathmatch --  Implements plain-old deathmatch (PODM).
//-----------------------------------------------------------------------
//CLASS_ARENA( ModeDeathmatch );
class ModeDeathmatch : public MultiplayerModeBase
	{
	public:
		CLASS_PROTOTYPE( ModeDeathmatch );
											ModeDeathmatch();
		virtual								~ModeDeathmatch();
		
		/* virtual */ void					AddPlayer( Player *player );

		/* virtual */ void					respawnPlayer( Player *player );

		/* virtual */ void					init( int maxPlayers );

		/* virtual */ bool					checkGameType( const char *rule );

		/* virtual */ void					update( float frameTime );

		/* virtual */ bool					checkRule( const char *rule, bool defaultValue, Player *player = NULL );

	private:

		int									getHighestPoints( int entnum );
	};

#endif // __MP_MODEDM_HPP__

