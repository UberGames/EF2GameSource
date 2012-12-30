//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/health.h                                      $
// $Revision:: 7                                                              $
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
// Health powerup
//

#ifndef __HEALTH_H__
#define __HEALTH_H__

#include "g_local.h"
#include "item.h"
#include "sentient.h"
#include "item.h"

class Health : public Item
	{
	public:
      CLASS_PROTOTYPE( Health );

								Health();
		virtual void		PickupHealth( Event *ev );
	};

class HealthInventoryItem : public Item
{
	public:
		CLASS_PROTOTYPE( HealthInventoryItem );
	
		virtual void		processGameplayData( Event *ev );
		void				Use( Event *ev );

};

#endif /* health.h */
