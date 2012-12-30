//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/portal.h                                      $
// $Revision:: 2                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

#ifndef __PORTAL_H__
#define __PORTAL_H__

#include "g_local.h"

class PortalSurface : public Entity
   {
   public:
      CLASS_PROTOTYPE( PortalSurface );
      PortalSurface();

      void LocateCamera( Event *ev );
   };

class PortalCamera : public Entity
   {
   public:
      CLASS_PROTOTYPE( PortalCamera );
      PortalCamera();
      void Roll( Event *ev );
   };

#endif // __PORTAL_H__
