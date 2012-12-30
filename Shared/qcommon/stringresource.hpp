//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/uilib/console.h                                           $
// $Revision:: 2                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/10/01 10:29a                                                $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// String resource system

#ifndef STRING_RESOURCE_HPP
#define STRING_RESOURCE_HPP


#include <game/q_shared.h>
//#include "qcommon.h"
#include <game/container.h>
#include <game/str.h>

//#include <cgame/cg_local.h>
//#include <cgame/cg_commands.h>

//The map types of the string.
//Global - this is a global string and is used for every level
//Level  - this is a string specific for the level and therefore loaded when the level is loaded.
typedef enum StringType { GLOBAL, LEVEL };
#define STRING_HASH_SIZE 1024


//-------------------------- CLASS ----------------------------------
// 
// Name:			StringMapEntry
// Base Class:		None
// 
// Description:		This is the map entry that the String Resource contains.
//					The StringMapEntry stores the string, id and the type of string
//					that is used.
//
// Method Of Use:	This is used internally by the String Resource class.
//
//-------------------------------------------------------------------
class StringEntry
{
	public:
		StringEntry()									{	}
		~StringEntry()									{	}
		
		str&			getString(void)					{ return _string;	}
		str&			getStringId(void)				{ return _stringId;	}
		StringType		getType(void)					{ return _type;		}
		StringEntry*	getNext(void)					{ return _next;		}

		void			setString(const str& string)	{ _string	= string;		}
		void			setStringId(const str& stringId){ _stringId = stringId;		}	
		void			setType(StringType type)		{ _type		= type;			}
		void			setNext(StringEntry* next)		{ _next		= next;			}
		
	private:
		str				_string;
		str				_stringId;
		StringType		_type;
		StringEntry*	_next;
};


//-------------------------- CLASS ----------------------------------
// 
// Name:			StringResource
// Base Class:		None
// 
// Description:		The String Resource is a system that loads a list of strings. Each of these strings
//					are paired with an identifier. 
//
// Method Of Use:	This is used internally by the print statements, mainly center print.
//
//-------------------------------------------------------------------
class StringResource
{
	public:
		static void				createInstance(void);
		static void				deleteInstance(void);
		static StringResource*	getInstance(void)		{ if(_instance == 0) createInstance(); return _instance;	}

		str&					getStringResource(const str& stringId);

		void					convertStringResourceIds(str& string);
		void					parseStringId(str& stringId);
		void					loadGlobalStrings(void);
		void					loadLevelStrings(const str& levelName);
		void					clearAllStrings(void);
		void					clearLevelStrings(void);
		bool					addStringEntry(StringEntry* stringEntry);
		bool					loadStrings(const str& fileName, StringType stringType);
		void					parseStringValue(str& stringValue);

	protected:
		int						getHashValue(const str& stringId);

	private:
		StringResource();
		~StringResource();


		static StringResource*				_instance;
		StringEntry*						_stringHashTable[STRING_HASH_SIZE];
		str									_notFoundString;
		str									_loadedLevel;
		bool								_globalLoaded;
		bool								_overwrite;
		cvar_t*								_languageCvar;
};
#endif
