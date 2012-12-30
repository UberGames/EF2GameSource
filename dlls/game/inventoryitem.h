//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/inventoryitem.h                               $
// $Revision:: 4                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Items that are visible in the player's inventory


#ifndef __INVITEM_H__
#define __INVITEM_H__

#include "item.h"

class InventoryItem : public Item
	{
	public:
      CLASS_PROTOTYPE( InventoryItem );

                     InventoryItem();
      virtual void   UseEvent( Event *ev );
	};

extern Event EV_InventoryItem_Use;

#endif /* inventoryitem.h */
