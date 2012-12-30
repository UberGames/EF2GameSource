//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/light.h                                       $
// $Revision:: 3                                                              $
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
// Classes for creating and controlling lights.
//

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "g_local.h"
#include "entity.h"

class Light : public Entity
	{
	public:
      CLASS_PROTOTYPE( Light );

								Light();
	};

#endif /* light.h */
