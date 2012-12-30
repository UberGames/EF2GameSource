//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/UseData.h                 $
// $Revision:: 5                                    $
//   $Author:: Singlis                                    $
//     $Date:: 9/26/03 2:36p                                       $
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



// UseData.h: interface for the UseData class.
//
//////////////////////////////////////////////////////////////////////

class UseData;

#ifndef __USEDATA_H__
#define __USEDATA_H__

#include "g_local.h"


//------------------------- CLASS ------------------------------
//
// Name:			UseData
// Base Class:		Class
//
// Description:		Stores information about how to use this entity
//					
// Method of Use:	There is a pointer to this class in Entity,
//					NULL by default.  If this class is allocated,
//					it is assumed that this entity can be "used"
//					When used, it plays _useAnim on the player,
//					calls _useThread in the script.  Displays
//					_useType icon in the HUD.
//--------------------------------------------------------------
class UseData : public Class
{
	private:
		str _useAnim;
		str _useThread;
		str _useType;

		float _useMaxDist;
		int _useCount;

	public:
		UseData::UseData()
			:_useAnim(""),
			 _useType(""),
			 _useThread(""),
			 _useMaxDist(64.0f),
			 _useCount(-1)
		{ }

		~UseData() { }

		const str& getUseAnim()		{ return _useAnim; }
		const str& getUseThread()	{ return _useThread; }
		const str& getUseType()		{ return _useType; }
		float getUseMaxDist()		{ return _useMaxDist; }		
		int getUseCount()			{ return _useCount; }	

		void setUseAnim(const str& newanim)			{ _useAnim = newanim; }
		void setUseThread(const str& newthread)		{ _useThread = newthread; }
		void setUseType(const str& newtype)			{ _useType = newtype; }
		void setUseMaxDist(float newdist)			{ _useMaxDist = newdist; }
		void setUseCount(float newcount)			{ _useCount = (int)newcount; }

		void useMe();

		void Archive(Archiver &arc);
};

inline void UseData::Archive(Archiver &arc)
{
	Class::Archive( arc );

	arc.ArchiveString( &_useAnim );
	arc.ArchiveString( &_useThread );
	arc.ArchiveString( &_useType );
	arc.ArchiveFloat( &_useMaxDist );
	arc.ArchiveInteger( &_useCount );
}

#endif // __USEDATA_H__
