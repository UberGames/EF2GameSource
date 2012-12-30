//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/Shared/qcommon/gameplaymanager.cpp                  $
// $Revision:: 14                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// GameplayManager.cpp: implementation of the GameplayManager class.
//
//////////////////////////////////////////////////////////////////////

#include "gameplaymanager.h"
#include <assert.h>
//#include <qcommon/qcommon.h>

// The Singleton
GameplayManager *GameplayManager::_theGameplayManager = 0; 

//--------------------------------------------------------------
//
// Name:			GameplayManager
// Class:			GameplayManager
//
// Description:		Constructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayManager::GameplayManager()
{

}

//--------------------------------------------------------------
//
// Name:			~GameplayManager
// Class:			GameplayManager
//
// Description:		Destructor.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayManager::~GameplayManager()
{

}

//--------------------------------------------------------------
// Name:			Shutdown (static)
// Class:			GameplayManager
//
// Description:		Destroys the GPM variable
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayManager::shutdown()
{
	if ( _theGameplayManager )
	{
		delete _theGameplayManager;
		_theGameplayManager = 0;
	}
}

//--------------------------------------------------------------
// Name:			IsReady (static)
// Class:			GameplayManager
//
// Description:		Determines if the GPM variable has been created yet
//
// Parameters:		None
//
// Returns:			bool -- Is ready or not 
//
//--------------------------------------------------------------
bool GameplayManager::isReady()
{
	if ( _theGameplayManager )
		return true;

	return false;
}

//--------------------------------------------------------------
// Name:			getTheGameplayManager (static)
// Class:			GameplayManager
//
// Description:		Interface function to the GameplayManager singleton
//
// Parameters:		None
//
// Returns:			Pointer to the GameplayManager singleton
//
//--------------------------------------------------------------
GameplayManager* GameplayManager::getTheGameplayManager()
{
	if ( _theGameplayManager )
		return _theGameplayManager;

	assert(0); // Something called this function before create();

	return 0;
}

//--------------------------------------------------------------
// Name:			Create (static)
// Class:			GameplayManager
//
// Description:		Creates the GPM singleton.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayManager::create()
{
	if ( _theGameplayManager )
		shutdown();

	_theGameplayManager = new GameplayManager;
	if (!_theGameplayManager)
	{
		// This probably won't happen unless we're out of memory
		assert(0);
	}

	if ( !_theGameplayManager->_gameplayDatabase.parseFile("global/gameplay.gdb") )
	{
		// Parsing the database file failed!
		assert(0);
		Com_Error (ERR_DROP, "GameplayDatabase: Parsing failed.  Contact programmer regarding this error.");
	}
#ifdef GAME_DLL
	if ( !_theGameplayManager->_gameplayFormulaManager.parseFile("global/gameplay.gpf") )
	{
		// Parsing the formula file failed!
		assert(0);
		gi.Error(ERR_DROP, "GameplayFormulas: Parsing failed.  Contact programmer regarding this error.");
	}
#endif // GAME_DLL
}


//--------------------------------------------------------------
//
// Name:			hasObject
// Class:			GameplayManager
//
// Description:		Checks to see if an object is in the database
//
// Parameters:		const str& objname -- Name of the object
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayManager::hasObject(const str& objname)
{
	if ( !objname.length() )
		return false;
	return _gameplayDatabase.hasObject(objname);
}


//--------------------------------------------------------------
//
// Name:			isDefined
// Class:			GameplayManager
//
// Description:		Checks to see if the specified property 
//					exists as a define in the database.
//
// Parameters:		const str& propname -- Property to find
//
// Returns:			bool -- True if exists
//
//--------------------------------------------------------------
bool GameplayManager::isDefined(const str& propname)
{
	GameplayObject *gpobject = _gameplayDatabase.getObject(propname);
	if ( !gpobject )
		return false;
	
	if ( gpobject->hasProperty("value") )
		return true;

	return false;
}

//--------------------------------------------------------------
//
// Name:			getDefine
// Class:			GameplayManager
//
// Description:		Returns the define as a string
//
// Parameters:		const str& propname -- Property to find
//
// Returns:			const str& -- the define as a string
//
//--------------------------------------------------------------
const str GameplayManager::getDefine(const str& propname)
{
	GameplayObject *gpobject = _gameplayDatabase.getObject(propname);
	if ( !gpobject )
		return ""; // Should never happen since we go through isDefined first
	
	str value = gpobject->getPropertyStringValue("value");
	if ( value == "" )
	{
		float floatval;
		char tmpstr[16];
		floatval = gpobject->getPropertyFloatValue("value");
		sprintf(tmpstr, "%g", floatval);
		value = tmpstr;
	}

	return value;
}




//--------------------------------------------------------------
//
// Name:			setFloatValue
// Class:			GameplayManager
//
// Description:		Sets float value of a property.
//
// Parameters:		const str& objname -- Object name
//					const str& propname -- Property name
//					float value -- value
//					bool create -- whether or not to create properties that are not found
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayManager::setFloatValue(const str& objname, const str& propname, float value, bool create)
{
#ifdef GAME_DLL
	bool valueChanged = _gameplayDatabase.setFloatValue(objname, propname, value, create);
	if ( valueChanged )
	{
		char message[256] ;
		sprintf( message, "gdb_setfloatproperty %s %s %g\n", objname.c_str(), propname.c_str(), value );
		G_SendCommandToAllPlayers( message );
	}
#else
	_gameplayDatabase.setFloatValue(objname, propname, value, create);
#endif // GAME_DLL
}


//--------------------------------------------------------------
//
// Name:			setStringValue
// Class:			GameplayManager
//
// Description:		Sets float value of a property.
//
// Parameters:		const str& objname -- Object name
//					const str& propname -- Property name
//					const str& value -- string value
//					bool create -- whether or not to create properties that are not found
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayManager::setStringValue(const str& objname, const str& propname, const str& valuestr, bool create)
{
#ifdef GAME_DLL
	bool valueChanged = _gameplayDatabase.setStringValue(objname, propname, valuestr, create);

	if ( valueChanged )
	{
		char message[256] ;
		sprintf( message, "gdb_setstringproperty %s %s %s\n", objname.c_str(), propname.c_str(), valuestr.c_str()	);
		G_SendCommandToAllPlayers( message );
	}
#else
	_gameplayDatabase.setStringValue(objname, propname, valuestr, create);
#endif // GAME_DLL
}


//--------------------------------------------------------------
//
// Name:			getFloatValue
// Class:			GameplayManager
//
// Description:		Gets the float value of the property.
//
// Parameters:		const str& objname -- Object to retrieve the property from, supports scoping
//					via the . symbol, example: "Object1.SubObject"
//
// Returns:			float -- Float value of the property or 1.0
//
//--------------------------------------------------------------
float GameplayManager::getFloatValue(const str& objname, const str& propname)
{
	if ( !hasObject(objname) )
		return 1.0f;

	GameplayObject *gpobject = 0;
	gpobject = _gameplayDatabase.getObject(objname);

	return gpobject->getPropertyFloatValue(propname);
}


//--------------------------------------------------------------
//
// Name:			getStringValue
// Class:			GameplayManager
//
// Description:		Gets the string value of the property.
//
// Parameters:		const str& objname -- Object to retrieve the property from, supports scoping
//					via the . symbol, example: "Object1.SubObject"
//
// Returns:			const str -- String value of the property or ""
//
//--------------------------------------------------------------
const str GameplayManager::getStringValue(const str& objname, const str& propname)
{
	if ( !hasObject(objname) )
		return "";

	GameplayObject *gpobject = 0;
	gpobject = _gameplayDatabase.getObject(objname);

	return gpobject->getPropertyStringValue(propname);
}





//--------------------------------------------------------------
//
// Name:			hasProperty
// Class:			GameplayManager
//
// Description:		Checks to see if the property exists in the object
//
// Parameters:		const str& objname -- Object to find
//					const str& propname -- Property name to check for
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayManager::hasProperty(const str& objname, const str& propname)
{
	if ( !hasObject(objname) )
		return false;

	GameplayObject *gpobject = _gameplayDatabase.getObject(objname);
	return gpobject->hasProperty(propname);
}


//--------------------------------------------------------------
//
// Name:			hasSubObject
// Class:			GameplayManager
//
// Description:		Checks to see if the specified object has the subobject
//
// Parameters:		const str& objname -- Object name
//					const str& subobject -- Subobject to look for
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayManager::hasSubObject(const str& objname, const str& subobject)
{
	if ( !objname.length() || !subobject.length())
		return false;

	GameplayObject *gpobject = 0;
	gpobject = _gameplayDatabase.getObject(objname);
	if ( !gpobject )
		return false;

	if ( !gpobject->getSubObject(subobject) )
		return false;

	return true;
}

#ifdef GAME_DLL
//--------------------------------------------------------------
//               F O R M U L A   S T U F F
//--------------------------------------------------------------

//--------------------------------------------------------------
//
// Name:			hasFormula
// Class:			GameplayManager
//
// Description:		Checks to see if the formula is in the database
//
// Parameters:		const str& formulaName -- Formula to check for
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayManager::hasFormula(const str& formulaName)
{
	return _gameplayFormulaManager.hasFormula(formulaName);
}


//--------------------------------------------------------------
//
// Name:			calculate
// Class:			GameplayManager
//
// Description:		Queries the FormulaManager for the result given the formula
//					name and some data.
//
// Parameters:		const str& formulaName -- Name of the formula
//					const GameplayFormulaData& formulaData -- Formula Data
//					float multiplier -- optional multiplier that defaults to 1.0
//
// Returns:			flaot 
//
//--------------------------------------------------------------
float GameplayManager::calculate(const str& formulaName, const GameplayFormulaData& formulaData, float multiplier)
{
	float result = _gameplayFormulaManager.getFormulaResult(formulaName, formulaData) * multiplier;
	return result;	
}


//--------------------------------------------------------------
//
// Name:			setBase
// Class:			GameplayManager
//
// Description:		Sets the base object of objname to be baseobj.
//
// Parameters:		const str& objname -- Object on which to set the base
//					const str& baseobj -- Base object to reference
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayManager::setBase(const str& objname, const str& baseobj)
{
	return _gameplayDatabase.setBase(objname, baseobj);
}


//--------------------------------------------------------------
//
// Name:			clearPropertyOverrides
// Class:			GameplayManager
//
// Description:		Clears all properties in this object that override
//					properties in the base object.
//
// Parameters:		const str& objname -- Object to clear the properties for
//
// Returns:			bool - successful or not
//
//--------------------------------------------------------------
bool GameplayManager::clearPropertyOverrides(const str& objname)
{
	return _gameplayDatabase.clearPropertyOverrides(objname);
}

//===============================================================
// Name:		processPendingMessages
// Class:		GameplayManager
//
// Description: Sends all messages that haven't been sent about
//				deltas in the database to the client.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
void GameplayManager::processPendingMessages( void )
{
	char message[256] ;

	Container<PendingDelta>	&pendingDeltaList = _gameplayDatabase.getPendingDeltaList();
	for ( int pendingDeltaIdx = 1; pendingDeltaIdx <= pendingDeltaList.NumObjects(); pendingDeltaIdx++ )
	{
		PendingDelta	pendingDelta	= pendingDeltaList.ObjectAt( pendingDeltaIdx );
		Vector			vectorValue ;
		float			floatValue		= 0.0f ;
		str				objName			= pendingDelta.getObjectName();
		str				propName		= pendingDelta.getPropertyName();
		str				stringValue ;

		switch ( pendingDelta.getGameplayValueType() )
		{
			case VALUE_FLOAT:
				floatValue	= pendingDelta.getFloatValue();
				sprintf( message, "gdb_setfloatproperty %s %s %g\n", objName.c_str(), propName.c_str(), floatValue );
				break ;
			case VALUE_STRING:
				stringValue	= pendingDelta.getStringValue();
				sprintf( message, "gdb_setstringproperty %s %s %s\n", objName.c_str(), propName.c_str(), stringValue.c_str() );
				break ;
			case VALUE_VECTOR:
				vectorValue = pendingDelta.getVectorValue();
				sprintf( message, "gdb_setvectorproperty %s %s %g %g %g\n", objName.c_str(), propName.c_str(), vectorValue.x, vectorValue.y, vectorValue.z );
				break ;
			default:
				break ;
		}
		G_SendCommandToAllPlayers( message );
	}

	_gameplayDatabase.clearPendingDeltaList();
}


//--------------------------------------------------------------
//
// Name:			Archive
// Class:			GameplayDatabase
//
// Description:		Archive function.
//
// Parameters:		Archiver &arc -- Archive reference
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayManager::Archive(Archiver &arc)
{
	_gameplayDatabase.Archive(arc);
}


#else



//----------------------------------------------------------------
//             E X T E R N A L   "C"   A P I
//----------------------------------------------------------------

//-----------------------------------------------------
//
// Name:		HasProperty
// Class:		None
//
// Description:	Checks if a property exists. 
//
// Parameters:	objectName		- The object name
//				propertyName	- The property name
//
// Returns:		If found, true is returned, otherwise false.
//-----------------------------------------------------
extern "C" qboolean HasGameplayProperty(const char* objectName, const char* propertyName)
{
	GameplayManager* gpm = GameplayManager::getTheGameplayManager();
	if(gpm == 0)
		return 0;

	if(gpm->hasProperty(objectName, propertyName))
		return qtrue;

	return qfalse;
}

//-----------------------------------------------------
//
// Name:		GetGameplayStringProperty
// Class:		None
//
// Description:	Gets the string property in the gameplay database	
//
// Parameters:	objectName		- The object name
//				propertyName	- The property name
//
// Returns:		If found, the property string is returned, otherwise 0
//-----------------------------------------------------
extern "C" void GetGameplayStringProperty(const char* objectName, const char* propertyName, char* buffer, int length)
{
	if(buffer == 0 || length <= 0)
		return;

	GameplayManager* gpm = GameplayManager::getTheGameplayManager();
	if(gpm == 0)
		return;

	str stringValue = gpm->getStringValue(objectName, propertyName);

	strncpy(buffer, stringValue.c_str(), length);
}

//-----------------------------------------------------
//
// Name:		SetGameplayStringProperty
// Class:		None
//
// Description:	Sets the string property in the gameplay database	
//
// Parameters:	objname		- The object name
//				propname	- The property name
//				valuestr	- The value of the property.
//				create		- If true, it creates the property if the property does not exist
//
// Returns:		None
//-----------------------------------------------------
extern "C" void SetGameplayStringProperty(const char* objectName, const char* propertyName, const char* stringValue, qboolean create)
{
	GameplayManager* gpm = GameplayManager::getTheGameplayManager();
	if(gpm == 0)
		return;

	//Convert the qboolean to boolean
	bool createProperty = false;

	if(create)
		createProperty = true;

	gpm->setStringValue(objectName, propertyName, stringValue, createProperty);
}



//-----------------------------------------------------
//
// Name:		GetGameplayFloatProperty
// Class:		None
//
// Description:	Gets a float property from the gameplay datbase
//
// Parameters:	objectName - the object name
//				propertyName - the property name
//
// Returns:		The found value.
//-----------------------------------------------------
extern "C" float GetGameplayFloatProperty(const char* objectName, const char* propertyName)
{
	GameplayManager* gpm = GameplayManager::getTheGameplayManager();
	if(gpm == 0)
		return 1.0f;

	return gpm->getFloatValue(objectName, propertyName);
}


//-----------------------------------------------------
//
// Name:		GetGameplayFloatPropertyCmd
// Class:		None
//
// Description:	Gets a float property from the gameplay datbase
//				and prints it to the console.
//
// Parameters:	
//
// Returns:		The found value.
//-----------------------------------------------------
void GetGameplayFloatPropertyCmd( void )
{
	GameplayManager* gpm = GameplayManager::getTheGameplayManager();

	if ( gpm )
	{
		const char *objectName		= Cmd_Argv( 1 );
		const char *propertyName	= Cmd_Argv( 2 );
		float		floatValue		= gpm->getFloatValue( objectName, propertyName );
		Com_Printf( "%s.%s: %g\n", objectName, propertyName, floatValue );
	}
}

//-----------------------------------------------------
//
// Name:		GetGameplayStringPropertyCmd
// Class:		None
//
// Description:	Gets a string property from the gameplay datbase
//				and prints it to the console.
//
// Parameters:	
//
// Returns:		The found value.
//-----------------------------------------------------
void GetGameplayStringPropertyCmd( void )
{
	GameplayManager* gpm = GameplayManager::getTheGameplayManager();
	if( gpm )
	{
		const char *objectName		= Cmd_Argv( 1 );
		const char *propertyName	= Cmd_Argv( 2 );
		str			stringValue		= gpm->getStringValue( objectName, propertyName );
		Com_Printf( "%s.%s: %s\n", objectName, propertyName, stringValue.c_str() );
	}
}


//-----------------------------------------------------
//
// Name:		SetGameplayFloatProperty
// Class:		None
//
// Description:	Sets a float property in the gameplay database
//
// Parameters:	objectName	 - the object name
//				propertyName - the property name
//				value		 - the new float value
//				create		 - If true, the value is created, otherwise the value is set.
//
// Returns:		None
//-----------------------------------------------------
extern "C" void SetGameplayFloatProperty(const char* objectName, const char* propertyName, float value, qboolean create)
{
	GameplayManager* gpm = GameplayManager::getTheGameplayManager();
	if(gpm == 0)
		return;

	//Convert the qboolean to boolean.
	bool createProperty = false;

	if(create)
		createProperty = true;

	gpm->setFloatValue(objectName, propertyName, value, createProperty);
}


//===============================================================
// Name:		SetGameplayStringProperty
// Class:		None
//
// Description: Sets a string property in the gameplay database.
// 
// Parameters:	None	
//
// Returns:		None
// 
//===============================================================
void SetGameplayStringProperty( void )
{
	if ( Cmd_Argc() != 4 )
	{
		Com_Printf( "Syntax: gdb_setstringproperty <objname> <propname> <stringvalue>\n" );
		return ;
	}

	const char *objectName		= Cmd_Argv( 1 );
	const char *propertyName	= Cmd_Argv( 2 );
	const char *stringValue		= Cmd_Argv( 3 );

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm ) return ;

	gpm->setStringValue( objectName, propertyName, stringValue, true );
}

//===============================================================
// Name:		SetGameplayFloatProperty
// Class:		None
//
// Description: Sets a float property in the gameplay database.
// 
// Parameters:	None	
//
// Returns:		None
// 
//===============================================================
void SetGameplayFloatProperty( void )
{
	if ( Cmd_Argc() != 4 )
	{
		Com_Printf( "Syntax: gdb_setfloatproperty <objname> <propname> <floatvalue>\n" );
		return ;
	}

	const char *objectName			= Cmd_Argv( 1 );
	const char *propertyName		= Cmd_Argv( 2 );
	const char *floatStringValue	= Cmd_Argv( 3 );

	GameplayManager *gpm = GameplayManager::getTheGameplayManager();
	if ( !gpm ) return ;

	float floatValue = atof( floatStringValue );
	gpm->setFloatValue( objectName, propertyName, floatValue, true );
}


//===============================================================
// Name:		CreateGameplayManager
// Class:		None
//
// Description: Creates the GameplayManager.  This loads the
//				Gameplay Database from disk.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
extern "C" void CreateGameplayManager( void )
{
	GameplayManager::create();
	Cmd_AddCommand( "gdb_setfloatproperty",		SetGameplayFloatProperty );
	Cmd_AddCommand( "gdb_setstringproperty",	SetGameplayStringProperty );
	Cmd_AddCommand( "gdb_getfloatproperty",		GetGameplayFloatPropertyCmd );
	Cmd_AddCommand( "gdb_getstringproperty",	GetGameplayStringPropertyCmd );
}

//===============================================================
// Name:		ShutdownGameplayManager
// Class:		None
//
// Description: Shuts down (destroys) the GameplayManager.
// 
// Parameters:	None
//
// Returns:		None
// 
//===============================================================
extern "C" void ShutdownGameplayManager( void )
{
	Cmd_RemoveCommand( "gdb_setfloatproperty" );
	Cmd_RemoveCommand( "gdb_setstringproperty" );
	Cmd_RemoveCommand( "gdb_getfloatproperty" );
	Cmd_RemoveCommand( "gdb_getstringproperty" );
	GameplayManager::shutdown();
}

#endif // GAME_DLL
