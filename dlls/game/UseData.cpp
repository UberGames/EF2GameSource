//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/UseData.cpp                   $
// $Revision:: 4                                    $
//   $Author:: Steven                                     $
//     $Date:: 5/20/02 3:53p                                       $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// UseDataObject
//

#include "_pch_cpp.h"
//#include "g_local.h"
#include "UseData.h"


//--------------------------------------------------------------
//
// Name:			useMe
// Class:			UseData
//
// Description:		Local notification that this entity has been
//					used.  Needed to decrement use counter.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void UseData::useMe()
{
	if ( _useCount == -1 )
		return;

	_useCount--;
}
