//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/body.h                                        $
// $Revision:: 4                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 2000 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Dead bodies

#ifndef __BODY_H__
#define __BODY_H__

//=============================================================
//class Body
//=============================================================
class Body : public Entity
   {
   private:
      void Damage( Event *ev );

   public:
      CLASS_PROTOTYPE( Body );
      Body();
   };

#endif //__BODY_H__
