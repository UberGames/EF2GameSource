//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/Shared/qcommon/gameplaydatabase.cpp                 $
// $Revision:: 17                                                             $
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
// GameplayDatabase.cpp: implementation of the GameplayDatabase class.
//
//////////////////////////////////////////////////////////////////////

#include "gameplaydatabase.h"



////////////////////////////////////////////////////////////////
//
//		GameplayProperty CLASS
//
////////////////////////////////////////////////////////////////
CLASS_DECLARATION( Class, GameplayProperty, NULL )
{
	{NULL,NULL}
};

//===============================================================
// Name:		setName
// Class:		GameplayProperty
//
// Description: Sets the name for this property
// 
// Parameters:	const str& -- the name for the property
//
// Returns:		None
// 
//===============================================================
void GameplayProperty::setName( const str &name )
{
	_name = name;
}


//===============================================================
// Name:		setFloatValue
// Class:		GameplayProperty
//
// Description: Sets the float value for this property.  Only
//				sets the value if the value has changed.  Returns
//				a boolean specifying whether or not the value changed.
// 
// Parameters:	float -- value
//
// Returns:		bool -- true if the value actually changed.
// 
//===============================================================
bool GameplayProperty::setFloatValue(float valuefloat)
{
	if ( _valuefloat == valuefloat ) return false ;

	_valuefloat = valuefloat ;
	return true ;
}

//===============================================================
// Name:		setVectorValue
// Class:		GameplayProperty
//
// Description: Sets the vector value for this property.  Only
//				sets the value if the value has changed.  Returns
//				a boolean specifying whether or not the value changed.
// 
// Parameters:	const Vector& vector -- the vector
//
// Returns:		bool -- true if the value actually changed.
// 
//===============================================================
bool GameplayProperty::setVectorValue(const Vector& vector)
{
	if ( _valuevector == vector ) return false ;

	_valuevector = vector ;
	return true ;
}

//===============================================================
// Name:		setStringValue
// Class:		GameplayProperty
//
// Description: Sets the string value for this property.  Only
//				sets the value if the value has changed.  Returns
//				a boolean specifying whether or not the value changed.
// 
// Parameters:	const str& -- value
//
// Returns:		bool -- true if the value actually changed.
// 
//===============================================================
bool GameplayProperty::setStringValue(const str& valuestr)
{
	if ( _valuestr == valuestr ) return false ;

	_valuestr = valuestr ;
	return true ;
}


//--------------------------------------------------------------
//
// Name:			parseProperty
// Class:			GameplayProperty
//
// Description:		Parses the gameplay property and returns
//
// Parameters:		gameplayFile -- Pointer to the script object (file)
//					const str& name -- name of the GameplayProperty
//		
// Returns:			bool -- sucessful parse or not
//
//--------------------------------------------------------------
bool GameplayProperty::parseProperty(Parser &gameplayFile, const str& type)
{
	const char *token;
	float val1, val2, val3;
	int readcount;
	
	if ( type == "vector" || type == "string" || type == "float" )
	{
		token = gameplayFile.GetToken(false);
		if ( !token )
			return false;
		setName(token);
	}
	else // Old-style file
		setName(type);

	token = gameplayFile.GetToken(false);
	if ( !token )
		return false;

	readcount = sscanf(token,"%f %f %f",&val1, &val2, &val3);

	if ( readcount == 1 ) // Single Float
	{
		setFloatValue(val1);
		setType(VALUE_FLOAT);
	}
	else if ( readcount == 3 ) // Vector
	{	
		setVectorValue(Vector(val1, val2, val3));
		setType(VALUE_VECTOR);
	}
	else // String
	{
		setStringValue(token);
		setType(VALUE_STRING);
	}

	return true;
}


//--------------------------------------------------------------
//
// Name:			getFloatValueStr
// Class:			GameplayProperty
//
// Description:		Gets the float value as a string
//
// Parameters:		None
//
// Returns:			const str -- string version
//
//--------------------------------------------------------------
const str GameplayProperty::getFloatValueStr()
{
	char tmpstr[16];
	sprintf(tmpstr, "%g", _valuefloat);
	return tmpstr;
}


#ifdef GAME_DLL
//--------------------------------------------------------------
//
// Name:			Archive
// Class:			GameplayProperty
//
// Description:		Archive function.
//
// Parameters:		Archiver &arc -- Archive reference
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayProperty::Archive(Archiver &arc)
{
	if ( arc.Loading() )
	{
		return ;
	}

	if ( arc.Saving() && isModified() )
	{
		ArchiveEnum( _type, GameplayValueType );
		arc.ArchiveString( &_name );
		switch ( _type )
		{
		case VALUE_FLOAT:
			arc.ArchiveFloat( &_valuefloat );
			break ;
		case VALUE_STRING:
			arc.ArchiveString( &_valuestr );
			break ;
		case VALUE_VECTOR:
			arc.ArchiveVector( &_valuevector );
			break ;
		}
	}
}

#endif // GAME_DLL






////////////////////////////////////////////////////////////////
//
//		GameplayObject CLASS
//
////////////////////////////////////////////////////////////////


CLASS_DECLARATION( Class, GameplayObject, NULL )
{
	{NULL,NULL}
};
//--------------------------------------------------------------
//
// Name:			GameplayObject
// Class:			GameplayObject
//
// Description:		Constructor.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayObject::GameplayObject() : 
	_name(""),
	_category(""),
	_fullScopeName(""),
	_depth(0),
	_baseObject(0),
	_nextHashObject(0),
	_modified( false )
{
	_propertyList.ClearObjectList();
	_subObjectList.ClearObjectList();
}

//--------------------------------------------------------------
//
// Name:			GameplayObject
// Class:			GameplayObject
//
// Description:		Constructor -- Overload with depth
//
// Parameters:		int depth
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayObject::GameplayObject(int depth) : 
	_name(""),
	_category(""),
	_fullScopeName(""),
	_depth(depth),
	_baseObject(0),
	_nextHashObject(0),
	_modified( false )
{
	_propertyList.ClearObjectList();
	_subObjectList.ClearObjectList();
}

//--------------------------------------------------------------
//
// Name:			~GameplayObject
// Class:			GameplayObject
//
// Description:		Destructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayObject::~GameplayObject()
{
	int i;
	for ( i=1; i<=_propertyList.NumObjects(); i++ )
	{
		GameplayProperty *gp = _propertyList.ObjectAt(i);
		delete gp;
	}

	for ( i=1; i<=_subObjectList.NumObjects(); i++ )
	{
		GameplayObject *go = _subObjectList.ObjectAt(i);
		delete go;
	}

	_baseObject = 0; // Note: This pointer will get deleted by the database.

	_subObjectList.FreeObjectList();
	_propertyList.FreeObjectList();
}

//--------------------------------------------------------------
//
// Name:			parseObject
// Class:			GameplayObject
//
// Description:		Parses the gameplay object and returns
//
// Parameters:		gameplayFile -- Pointer to the script object (file)
//					const str& name -- name of the GameplayObject
//		
// Returns:			bool -- sucessful parse or not
//
//--------------------------------------------------------------
bool GameplayObject::parseObject(Parser &gameplayFile, const str& name)
{
	const char *token;
	GameplayProperty *gpproperty;
	
	if ( name != "OBJECT" )
		return false;

	if ( !gameplayFile.TokenAvailable(false) )
		return false;
	
	token = gameplayFile.GetToken(false);
	setName(token);
	_fullScopeName += _name;

	if ( gameplayFile.TokenAvailable(false) )
	{
		token = gameplayFile.GetToken(false);
		setCategory(token);
	}

	// Get the open brace
	token = gameplayFile.GetToken(true);
	if ( token[0] != '{' )
		assert(0);

	while (gameplayFile.TokenAvailable(true))
	{
		token = gameplayFile.GetToken(true);

		// If we have a close brace, we're done.
		if ( token[0] == '}' )
			return true;

		// We have a suboject
		if ( !strcmp(token, "OBJECT") )
		{
			GameplayObject *gpobject;
			gpobject = new GameplayObject(_depth+1);
			gpobject->setFullScopeName(_fullScopeName + ".");
			if ( gpobject->parseObject(gameplayFile, token) )
			{
				_subObjectList.AddObject(gpobject);
			}
			else
			{
				delete gpobject;
				return false;
			}
		}
		else
		{
			gpproperty = new GameplayProperty();
			if ( gpproperty->parseProperty(gameplayFile, token) )
				_propertyList.AddObject(gpproperty);
			else
			{
				delete gpproperty;
				return false;
			}
		}
	}

	// Premature end of file, missing close brace.
	return false;
}

//--------------------------------------------------------------
//
// Name:			getProperty
// Class:			GameplayObject
//
// Description:		Gets the property by name, if it exists.
//
// Parameters:		const str& propname -- Property name to look up
//
// Returns:			Pointer to the property or 0 if not found
//
//--------------------------------------------------------------
GameplayProperty* GameplayObject::getProperty(const str& propname)
{
	int i;
	GameplayProperty *gpprop;

	if ( propname == "" )
		return 0;

	for ( i=1; i<=_propertyList.NumObjects(); i++ )
	{
		gpprop = _propertyList.ObjectAt(i);
		if ( gpprop->getName() == propname )
			return gpprop;
	}

	return 0;
}


//--------------------------------------------------------------
//
// Name:			getPropertyFloatValue
// Class:			GameplayObject
//
// Description:		Gets the float value of the property specified
//					Also checks the baseObject if the property doesn't exists
//					in this object.
//
// Parameters:		const str& propname -- Property name to look up
//
// Returns:			float -- value or 1.0
//
//--------------------------------------------------------------
float GameplayObject::getPropertyFloatValue(const str& propname)
{
	GameplayProperty *gpprop;
	
	gpprop = getProperty(propname);
	if ( !gpprop )
	{
		if ( _baseObject )
		{
			gpprop = _baseObject->getProperty(propname);
			if ( !gpprop )
				return 1.0f;
		}	
		else
			return 1.0f;
	}

	return gpprop->getFloatValue();
}


//--------------------------------------------------------------
//
// Name:			getPropertyStringValue
// Class:			GameplayObject
//
// Description:		Gets the string value of the property specified
//					Also checks the baseObject if the property doesn't exists
//					in this object.
//
// Parameters:		const str& propname -- Property name to look up
//
// Returns:			const str -- value or ""
//
//--------------------------------------------------------------
const str GameplayObject::getPropertyStringValue(const str& propname)
{
	GameplayProperty *gpprop;
	
	gpprop = getProperty(propname);
	if ( !gpprop )
	{
		if ( _baseObject )
		{
			gpprop = _baseObject->getProperty(propname);
			if ( !gpprop )
				return "";
		}	
		else
			return "";
	}

	return gpprop->getStringValue();
}

//--------------------------------------------------------------
//
// Name:			setFloatValue
// Class:			GameplayObject
//
// Description:		Sets float value of a property.
//
// Parameters:		const str& propname -- Property name
//					float value -- value
//					bool create -- whether or not to create properties that are not found
//
// Returns:			bool -- true if the value was changed or created.
//
//--------------------------------------------------------------
bool GameplayObject::setFloatValue(const str& propname, float value, bool create)
{
	GameplayProperty *gpprop = getProperty(propname);
	if ( !gpprop )
	{
		if ( !create ) return false ;


		gpprop = new GameplayProperty();
		gpprop->setName(propname);
		_propertyList.AddObject(gpprop);
	}

	bool valueChanged = gpprop->setFloatValue(value);
	if ( valueChanged )
	{
		gpprop->setModified( true );
		setModified( true );
	}
	
	return valueChanged ;
}


//--------------------------------------------------------------
//
// Name:			setStringValue
// Class:			GameplayObject
//
// Description:		Sets float value of a property.
//
// Parameters:		const str& propname -- Property name
//					const str& value -- string value
//					bool create -- whether or not to create properties that are not found
//
// Returns:			bool -- true if the value was changed or created.
//
//--------------------------------------------------------------
bool GameplayObject::setStringValue(const str& propname, const str& valuestr, bool create)
{
	GameplayProperty *gpprop = getProperty(propname);
	if ( !gpprop )
	{
		if ( !create ) return false;

		gpprop = new GameplayProperty();
		gpprop->setName(propname);
		_propertyList.AddObject(gpprop);
	}

	bool valueChanged = gpprop->setStringValue(valuestr);
	if ( valueChanged )
	{
		gpprop->setModified( true );
		setModified( true );
	}

	return valueChanged ;
}

//--------------------------------------------------------------
//
// Name:			setVectorValue
// Class:			GameplayObject
//
// Description:		Sets vector value of a property.
//
// Parameters:		const str& propname -- Property name
//					Vector value -- value
//					bool create -- whether or not to create properties that are not found
//
// Returns:			bool -- true if the value was changed or created.
//
//--------------------------------------------------------------
bool GameplayObject::setVectorValue(const str& propname, const Vector& value, bool create)
{
	GameplayProperty *gpprop = getProperty(propname);
	if ( !gpprop )
	{
		if ( !create ) return false ;


		gpprop = new GameplayProperty();
		gpprop->setName(propname);
		_propertyList.AddObject(gpprop);
	}

	bool valueChanged = gpprop->setVectorValue(value);
	if ( valueChanged )
	{
		gpprop->setModified( true );
		setModified( true );
	}
	
	return valueChanged ;
}

//--------------------------------------------------------------
//
// Name:			getModified
// Class:			GameplayObject
//
// Description:		Gets the modified flag on the given property name
//					Also checks the baseObject if the property doesn't exists
//					in this object.
//
// Parameters:		const std::string& propname -- property name
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayObject::getModified(const str& propname)
{
	GameplayProperty* gpprop = 0;
	gpprop = getProperty(propname);
	if ( !gpprop )
	{
		if ( _baseObject )
		{
			gpprop = _baseObject->getProperty(propname);
			if ( !gpprop )
				return false;
		}	
		else
			return false;
	}

	return gpprop->getModified();
}


//--------------------------------------------------------------
//
// Name:			getSubObject
// Class:			GameplayObject
//
// Description:		Gets the subobject by the specified name.
//
// Parameters:		const str& subobjname -- Name to find
//
// Returns:			GameplayObject * -- Object or 0 if not found.
//
//--------------------------------------------------------------
GameplayObject* GameplayObject::getSubObject(const str& subobjname)
{
	int i;
	for ( i=1; i<=_subObjectList.NumObjects(); i++ )
	{
		GameplayObject *go = _subObjectList.ObjectAt(i);
		if ( go->getName() == subobjname )
			return go;
	}

	return 0;
}


//--------------------------------------------------------------
//
// Name:			hasProperty
// Class:			GameplayObject
//
// Description:		Checks to see if the property exists
//
// Parameters:		const str& propname -- Property name to look up
//					bool localonly (default false) -- Specifies whether
//						or not to check local properties only.
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayObject::hasProperty(const str& propname, bool localonly)
{
	int i;
	GameplayProperty *gpprop;

	if ( propname == "" )
		return false;

	for ( i=1; i<=_propertyList.NumObjects(); i++ )
	{
		gpprop = _propertyList.ObjectAt(i);
		if ( gpprop->getName() == propname )
			return true;
	}

	if ( _baseObject && !localonly && _baseObject->hasProperty(propname) )
		return true;

	return false;
}


//--------------------------------------------------------------
//
// Name:			removeProperty
// Class:			GameplayObject
//
// Description:		Removes the property from this object
//
// Parameters:		const str& propname -- Property to remove
//
// Returns:			bool - success or not
//
//--------------------------------------------------------------
bool GameplayObject::removeProperty(const str& propname)
{
	int i;
	GameplayProperty *gpprop;

	if ( propname == "" )
		return false;

	for ( i=1; i<=_propertyList.NumObjects(); i++ )
	{
		gpprop = _propertyList.ObjectAt(i);
		if ( gpprop->getName() == propname )
		{
			_propertyList.RemoveObjectAt(i);
			delete gpprop;
			return true;
		}
	}	

	// Not found
	return false;
}


//===============================================================
// Name:		getNumberOfModifiedProperties
// Class:		GameplayObject
//
// Description: Retrieves the number of properties that have been
//				modified from their initial value loaded from disk.
//				This includes properties created at runtime.
// 
// Parameters:	None
//
// Returns:		int -- number of properties that have been modified.
// 
//===============================================================
int GameplayObject::getNumberOfModifiedProperties
(
	void
)
{
	int numModifiedProperties	= 0 ;
	int numProperties			= _propertyList.NumObjects();

	for ( int propertyIdx = 1; propertyIdx <= numProperties; ++propertyIdx )
	{
		GameplayProperty *property = _propertyList.ObjectAt( propertyIdx );
		if ( property->isModified() )
		{
			numModifiedProperties++ ;
		}
	}

	return numModifiedProperties ;
}



#ifdef GAME_DLL
//--------------------------------------------------------------
//
// Name:			Archive
// Class:			GameplayObject
//
// Description:		Archive function.  We only save out the properties
//					that have been modified.
//
// Parameters:		Archiver &arc -- Archive reference
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayObject::Archive(Archiver &arc)
{
	if ( arc.Loading() )
	{
		return ;
	}

	if ( arc.Saving() && isModified() )
	{
		// Get the number of modified properties and total properties
		int numModifiedProperties	= getNumberOfModifiedProperties();
		int numProperties			= _propertyList.NumObjects();

		// Save our name and number of modified properties
		arc.ArchiveString( &_fullScopeName );
		arc.ArchiveInteger( &numModifiedProperties );
		for ( int modifiedIdx = 1; modifiedIdx <= numProperties; ++modifiedIdx )
		{
			GameplayProperty *property = _propertyList.ObjectAt( modifiedIdx );
			if ( property->isModified() )
			{
				property->Archive( arc );
			}
		}
	}
}

#endif // GAME_DLL

////////////////////////////////////////////////////////////////
//
//		GameplayDatabase CLASS
//
////////////////////////////////////////////////////////////////

CLASS_DECLARATION( Listener, GameplayDatabase, NULL )
{
	{NULL,NULL}
};
//--------------------------------------------------------------
//
// Name:			GameplayDatabase
// Class:			GameplayDatabase
//
// Description:		Constructor.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayDatabase::GameplayDatabase() :	
	_lastObj(0),
	_lastObjName("")
{
	for ( int i=0; i<HASH_SIZE; i++ )
		_hashTable[i] = 0;

	_objectList.ClearObjectList();
}

//--------------------------------------------------------------
//
// Name:			~GameplayDatabase
// Class:			GameplayDatabase
//
// Description:		Destructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayDatabase::~GameplayDatabase()
{
	int i;

	// Empty the hash table
	for ( i=0; i<HASH_SIZE; i++ )
		_hashTable[i] = 0;

	// Delete the objects
	for ( i=1; i<=_objectList.NumObjects(); i++ )
	{
		GameplayObject *go = _objectList.ObjectAt(i);
		delete go;
	}
	_objectList.FreeObjectList();
}


//--------------------------------------------------------------
//
// Name:			parseFile
// Class:			GameplayDatabase
//
// Description:		Reads and parses the database file
//
// Parameters:		const str& filename -- Name of the file
//
// Returns:			bool -- sucessful parse or not
//
//--------------------------------------------------------------
bool GameplayDatabase::parseFile(const str& filename)
{
	Parser			gameplayFile;
	const char		*token;
	GameplayObject	*gpobject;

#ifdef GAME_DLL
	if ( !gi.FS_Exists(filename.c_str()) )
		return false;
#else
	if ( !FS_Exists(filename.c_str()) )
		return false;
#endif // GAME_DLL

	gameplayFile.LoadFile(filename.c_str());

	while (gameplayFile.TokenAvailable(true))
	{
		token = gameplayFile.GetToken(false);
		
		// If the first token isn't an object, there's a problem
		if ( strcmp(token, "OBJECT") )
			return false;

		gpobject = new GameplayObject();
		if ( gpobject->parseObject(gameplayFile, token) )
		{
			_objectList.AddObject(gpobject);
		}
		else
		{
			delete gpobject;
			return false;
		}
	}

	// Build hash table and link subobjects to their base
	_buildHashTable();
	_linkSubObjectsToBase();

	return true;
}


//--------------------------------------------------------------
//
// Name:			getRootObject
// Class:			GameplayDatabase
//
// Description:		Does not support the scope operation.
//					This function requests an object at the root level
//					of the database only.
//					Use this function ONLY if you want strictly rool level
//					objects, otherwise, use getObject.
//
// Parameters:		const str& objname -- Object to look.
//
// Returns:			Point to the object or 0 if not found
//
//--------------------------------------------------------------
GameplayObject* GameplayDatabase::getRootObject(const str& objname)
{
	GameplayObject *gpobject;
	unsigned int loc = GenerateHashForName(objname.c_str(), false, HASH_SIZE);
	gpobject = _hashTable[loc];
	while ( gpobject && gpobject->getName() != objname )
		gpobject = gpobject->getNextObject();

	return gpobject;
}


//--------------------------------------------------------------
//
// Name:			getObject
// Class:			GameplayDatabase
//
// Description:		Gets the object by the specified name.  Also
//					supports the . scope operation.
//
// Parameters:		const str& objname -- Object to look.
//
// Returns:			Point to the object or 0 if not found
//
//--------------------------------------------------------------
GameplayObject* GameplayDatabase::getObject(const str& objname)
{
	// For speed reasons, we save off the last object we asked for
	// if subsequent requests come in for the same object, we don't
	// have to look through the hash table again.
	if ( objname == _lastObjName )
		return _lastObj;

	GameplayObject *gpobject;
	unsigned int loc = GenerateHashForName(objname.c_str(), false, HASH_SIZE);
	gpobject = _hashTable[loc];
	while ( gpobject && gpobject->getFullScopeName() != objname )
		gpobject = gpobject->getNextObject();

	_lastObj = gpobject;
	_lastObjName = objname;

	return gpobject;
}


//--------------------------------------------------------------
//
// Name:			getSubObject
// Class:			GameplayDatabase
//
// Description:		Gets the subobject by the specified name
//
// Parameters:		const str& objname -- Root Object name
//					const str& subobjname -- Subobject name
//
// Returns:			Point to the object or 0 if not found
//
//--------------------------------------------------------------
GameplayObject* GameplayDatabase::getSubObject(const str& objname, const str& subobjname)
{
	int i;
	GameplayObject *gpobject;

	for ( i=1; i<=_objectList.NumObjects(); i++ )
	{
		gpobject = _objectList.ObjectAt(i);
		if ( gpobject->getName() == objname )
			return gpobject->getSubObject(subobjname);
	}

	return 0;
}


//--------------------------------------------------------------
//
// Name:			getFloatValue
// Class:			GameplayDatabase
//
// Description:		Gets the float value associated with the property
//					of the object.
//
// Parameters:		const str& objname -- Object name to look up.
//					const str& propnme -- Property name to look up
//
// Returns:			The value, or 1.0
//
//--------------------------------------------------------------
float GameplayDatabase::getFloatValue(const str& objname, const str& propname)
{
	GameplayObject *gpobject = 0;
	gpobject = getObject(objname);
	if ( !gpobject )
		return 1.0f;

	return gpobject->getPropertyFloatValue(propname);
}

//--------------------------------------------------------------
//
// Name:			getStringValue
// Class:			GameplayDatabase
//
// Description:		Gets the float value associated with the property
//					of the object.
//
// Parameters:		const str& objname -- Object name to look up.
//					const str& propnme -- Property name to look up
//
// Returns:			The value, or ""
//
//--------------------------------------------------------------
const str GameplayDatabase::getStringValue(const str& objname, const str& propname)
{
	GameplayObject *gpobject = 0;
	gpobject = getObject(objname);
	if ( !gpobject )
		return "";

	return gpobject->getPropertyStringValue(propname);
}

//--------------------------------------------------------------
//
// Name:			setFloatValue
// Class:			GameplayDatabase
//
// Description:		Sets float value of a property.
//
// Parameters:		const str& objname -- Object name
//					const str& propname -- Property name
//					float value -- value
//					bool create -- whether or not to create properties that are not found
//
// Returns:			bool -- true if the value changed or created.
//
//--------------------------------------------------------------
bool GameplayDatabase::setFloatValue(const str& objname, const str& propname, float value, bool create)
{
	GameplayObject *gpobject = getObject(objname);
	if ( !gpobject )
	{
		if ( !create ) return false;

		gpobject = _createFromScope(objname);
		gpobject->setCategory("Misc"); // Category doesn't really matter.
		return true ;
	}
	
	return gpobject->setFloatValue(propname, value, create);
}

//--------------------------------------------------------------
//
// Name:			setStringValue
// Class:			GameplayDatabase
//
// Description:		Sets float value of a property.
//
// Parameters:		const str& objname -- Object name
//					const str& propname -- Property name
//					const str& value -- string value
//					bool create -- whether or not to create properties that are not found
//
// Returns:			bool -- true if the value was changed or created.
//
//--------------------------------------------------------------
bool GameplayDatabase::setStringValue(const str& objname, const str& propname, const str& valuestr, bool create)
{
	GameplayObject *gpobject = getObject(objname);
	if ( !gpobject )
	{
		if ( !create ) return false;

		gpobject = _createFromScope(objname);
		gpobject->setCategory("Misc"); // Category doesn't really matter.
		return true ;
	}

	return gpobject->setStringValue(propname, valuestr, create);
}

//--------------------------------------------------------------
//
// Name:			setVectorValue
// Class:			GameplayDatabase
//
// Description:		Sets vector value of a property.
//
// Parameters:		const str& objname -- Object name
//					const str& propname -- Property name
//					Vector value -- value
//					bool create -- whether or not to create properties that are not found
//
// Returns:			bool -- true if the value changed or created.
//
//--------------------------------------------------------------
bool GameplayDatabase::setVectorValue(const str& objname, const str& propname, const Vector& value, bool create)
{
	GameplayObject *gpobject = getObject(objname);
	if ( !gpobject )
	{
		if ( !create ) return false;

		gpobject = _createFromScope(objname);
		gpobject->setCategory("Misc"); // Category doesn't really matter.
		return true ;
	}
	
	return gpobject->setVectorValue(propname, value, create);
}


//--------------------------------------------------------------
//
// Name:			_linkSubObjectsToBase
// Class:			GameplayDatabase
//
// Description:		Iterates through all the objects in the database
//					and calls _linkToBase to link each one's subobjects
//					up.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayDatabase::_linkSubObjectsToBase()
{
	int i;
	for ( i=1; i<=_objectList.NumObjects(); i++ )
	{
		GameplayObject *go = _objectList.ObjectAt(i);
		_linkToBase(go);
	}
}

//--------------------------------------------------------------
//
// Name:			_linkToBase
// Class:			GameplayDatabase
//
// Description:		Links the subojects in each object to the parent
//					(inherited) object.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayDatabase::_linkToBase(GameplayObject *object)
{
	int j;

	// Go through all the sub objects
	const Container<GameplayObject *>& subObjectList = object->getSubObjectList();
	for ( j=1; j<=subObjectList.NumObjects(); j++ )
	{
		GameplayObject *subobj = subObjectList.ObjectAt(j);
		GameplayObject *baseObj = getObject(subobj->getName());
		if ( !baseObj )
			assert(0); // Suboject found with no base
		else
			subobj->setBaseObject(baseObj);

		_linkToBase(subobj);
	}
}

//--------------------------------------------------------------
//
// Name:			hasObject
// Class:			GameplayDatabase
//
// Description:		Checks to see if the object exists.  Includes
//					scope support.
//
// Parameters:		const str& objname -- Object to look for
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayDatabase::hasObject(const str& objname)
{
	if ( getObject(objname) )
		return true;

	return false;
}


//--------------------------------------------------------------
//
// Name:			_createFromScope
// Class:			GameplayDatabase
//
// Description:		Creates an object given the scope... it will return
//					the created object, even if it had to create
//					other objects to get to the scope level required on the
//					way.
//
// Parameters:		const str& scope -- Scope of the object to create
//
// Returns:			GameplayObject * -- The new object
//
//--------------------------------------------------------------
GameplayObject* GameplayDatabase::_createFromScope(const str& scope)
{
	str scopename;
	char tmpstr[255], *sptr;
	//GameplayObject *newObj = 0;
	strcpy(tmpstr, scope.c_str());
	sptr = strtok(tmpstr,".\n");
	scopename += sptr;
	GameplayObject *gpobject = 0;
	gpobject = getRootObject(sptr);
	if ( !gpobject )
	{
		gpobject = new GameplayObject;
		gpobject->setName(sptr);
		gpobject->setFullScopeName(scopename);
		_addToHashTable(gpobject);
		_objectList.AddObject(gpobject);
	}
	
	sptr = strtok(NULL,".\n");
	while ( sptr )
	{
		scopename += ".";
		scopename += sptr;
		gpobject = gpobject->getSubObject(sptr);
		if ( !gpobject )
		{
			gpobject = new GameplayObject;
			gpobject->setName(sptr);
			gpobject->setFullScopeName(scopename);
			_addToHashTable(gpobject);
			_objectList.AddObject(gpobject);
		}

		sptr = strtok(NULL,".\n");
	}

	//if ( newObj )
	//	return newObj;
	//else
		return gpobject;
}


//--------------------------------------------------------------
//
// Name:			_addToHashTable
// Class:			GameplayDatabase
//
// Description:		Adds the object to the hash table
//
// Parameters:		GameplayObject *object -- Object to add
//
// Returns:			bool -- Collision flag. The object or one if it's
//					children collided while being put in the hash table
//
//--------------------------------------------------------------
bool GameplayDatabase::_addToHashTable(GameplayObject *object)
{
	if ( !object )
		false;

	bool collisionFlag = false;

	// Go through and hash all the object's children (since order doesn't matter
	// to the hash table).
	const Container<GameplayObject *>& subObjectList = object->getSubObjectList();
	for ( int j=1; j<=subObjectList.NumObjects(); j++ )
	{
		GameplayObject *subobj = subObjectList.ObjectAt(j);
		if ( _addToHashTable(subobj) )
			collisionFlag = true;
	}

	unsigned int loc = GenerateHashForName(object->getFullScopeName().c_str(), false, HASH_SIZE);
	GameplayObject *obj = 0;
	obj = _hashTable[loc];
	if ( !obj )
	{
		_hashTable[loc] = object;
		return collisionFlag;
	}

	// There's a collision because there's already
	// an object in the slot. Get to the last object
	// in the list.
	GameplayObject *prev = 0;
	collisionFlag = true;
	while ( obj )
	{
		prev = obj;
		obj = obj->getNextObject();
	}
	
	// Attach the object onto the end of the list
	prev->setNextObject(object);

	return collisionFlag;
}


//--------------------------------------------------------------
//
// Name:			_buildHashTable
// Class:			GameplayDatabase
//
// Description:		Builds the hash table.
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
void GameplayDatabase::_buildHashTable()
{
	int i;
	for ( i=1; i<=_objectList.NumObjects(); i++ )
	{
		GameplayObject *go = _objectList.ObjectAt(i);
		_addToHashTable(go);
	}
}


//--------------------------------------------------------------
//
// Name:			setBase
// Class:			GameplayDatabase
//
// Description:		Sets the base object of objname to be baseobj.
//
// Parameters:		const str& objname -- Object on which to set the base
//					const str& baseobj -- Base object to reference
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayDatabase::setBase(const str& objname, const str& baseobj)
{
	GameplayObject *obj = 0;
	obj = getObject(objname);
	if ( !obj )
		return false;

	GameplayObject *base = 0;
	base = getRootObject(baseobj);
	obj->setBaseObject(base);
	
	return true;
}


//--------------------------------------------------------------
//
// Name:			clearPropertyOverrides
// Class:			GameplayDatabase
//
// Description:		Clears all properties in this object that override
//					properties in the base object.
//
// Parameters:		const str& objname -- Object to clear the properties for
//
// Returns:			bool - successful or not
//
//--------------------------------------------------------------
bool GameplayDatabase::clearPropertyOverrides(const str& objname)
{
	int i = 0;

	// Get the object
	GameplayObject *obj = 0;
	obj = getObject(objname);
	if ( !obj )
		return false;
	
	// Get the base object
	GameplayObject *baseobj = 0;
	baseobj = obj->getBaseObject();
	if ( !baseobj )
		return true; // No base, return successful

	const Container<GameplayProperty *>& proplist = baseobj->getPropertyList();
	for ( i=1; i<=proplist.NumObjects(); i++ )
	{
		GameplayProperty *gp = proplist.ObjectAt(i);
		
		// Check for local properties only in the object.
		if ( obj->hasProperty(gp->getName(), true) )
			obj->removeProperty(gp->getName());
	}

	return true;
}


//===============================================================
// Name:		getNumberOfModifiedObjects
// Class:		GameplayDatabase
//
// Description: Retrieves the number of objects in the database
//				that have their modified flag set.  This is used
//				to determine how many objects in the database
//				we're going to have to save out.
// 
// Parameters:	None
//
// Returns:		int -- the number of modified objects.
// 
//===============================================================
int GameplayDatabase::getNumberOfModifiedObjects( void )
{
	int numberOfModifiedObjects	= 0 ;
	int numObjects				= _objectList.NumObjects();

	for ( int objectIdx = 1; objectIdx <= numObjects; ++objectIdx )
	{
		GameplayObject *object = _objectList.ObjectAt( objectIdx );
		if ( object->isModified() )
		{
			numberOfModifiedObjects++ ;
		}
	}

	return numberOfModifiedObjects ;
}


#ifdef GAME_DLL
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
void GameplayDatabase::Archive(Archiver &arc)
{
	GameplayObject	*object					= 0 ;
	int				 numModifiedObjects		= getNumberOfModifiedObjects();
	int				 numObjects				= _objectList.NumObjects();

	arc.ArchiveInteger( &numModifiedObjects );

	if ( arc.Saving() )
	{
		for ( int objectIdx = 1; objectIdx <= numObjects; ++objectIdx )
		{
			object					= _objectList.ObjectAt( objectIdx );
			object->Archive( arc );
		}
	}
	else
	{
		for ( int objectIdx = 1; objectIdx <= numModifiedObjects; ++objectIdx )
		{
			str	scopedObjectName ;
			int	numProperties ;

			arc.ArchiveString( &scopedObjectName );
			arc.ArchiveInteger( &numProperties );
			for ( int propertyIdx = 1; propertyIdx <= numProperties; ++propertyIdx )
			{
				GameplayValueType	propertyType	= VALUE_FLOAT ;
				Vector				vectorValue ;
				float				floatValue		= 0.0f ;
				str					propertyName ;
				str					stringValue ;

				ArchiveEnum( propertyType, GameplayValueType );
				arc.ArchiveString( &propertyName );
				switch( propertyType )
				{
					case VALUE_FLOAT:	
						arc.ArchiveFloat( &floatValue );	
						setFloatValue( scopedObjectName, propertyName, floatValue, true );
						_pendingDeltaList.AddObject( PendingDelta( scopedObjectName, propertyName, floatValue ) );
						break ;
					case VALUE_STRING:	
						arc.ArchiveString( &stringValue );
						setStringValue( scopedObjectName, propertyName, stringValue, true );
						_pendingDeltaList.AddObject( PendingDelta( scopedObjectName, propertyName, stringValue ) );
						break ;
					case VALUE_VECTOR:	
						arc.ArchiveVector( &vectorValue );
//						setVectorValue( scopedObjectName, propertyName, vectorValue, true );
						_pendingDeltaList.AddObject( PendingDelta( scopedObjectName, propertyName, vectorValue ) );
						break ;
				}
			}
		}		
	}
}

#endif // GAME_DLL
