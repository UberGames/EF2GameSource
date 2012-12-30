//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/inventoryitem.cpp                         $
// $Revision:: 10                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Inventory items

#include "_pch_cpp.h"
#include "inventoryitem.h"
#include "mp_manager.hpp"

Event EV_InventoryItem_Use
(
	"useinvitem",
	EV_CODEONLY,
	NULL,
	NULL,
	"Use this inventory item."
);

CLASS_DECLARATION( Item, InventoryItem, NULL )
{
	{ &EV_InventoryItem_Use,         &InventoryItem::UseEvent },

	{ NULL, NULL }
};

InventoryItem::InventoryItem()
{
	if ( LoadingSavegame )
	{
		return;
	}
	
	// All powerups are inventory items
	/* if ( multiplayerManager.checkFlag( MP_FLAG_NO_POWERUPS ) )
	{
		PostEvent( EV_Remove, EV_REMOVE );
		return;
	} */
}

void InventoryItem::UseEvent( Event *ev )
{
}

