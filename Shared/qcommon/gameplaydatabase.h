// GameplayDatabase.h: interface for the GameplayDatabase class.
//
//////////////////////////////////////////////////////////////////////

class GameplayDatabase;
class GameplayObject;
class GameplayProperty;

#ifndef __GAMEPLAYDATABASE_H__
#define __GAMEPLAYDATABASE_H__

#ifdef GAME_DLL

#include <game/g_local.h>
#include <game/script.h>

#endif // GAME_DLL


// Select a parser to use.  We wouldn't have to do this part
// if the system didn't have 10000 parsers.
#ifdef GAME_DLL
#define Parser Script
#else
#define Parser TikiScript
#endif // GAME_DLL

#define HASH_SIZE 256

#include <game/class.h>
#include <game/listener.h>
#include <game/str.h>
#include <game/container.h>
#include <qcommon/tiki_script.h>

typedef enum GameplayValueType
{
	VALUE_FLOAT,
	VALUE_STRING,
	VALUE_VECTOR,
	GAMEPLAY_VALUE_UNSPECIFIED
};

//------------------------ CLASS -------------------------------
//
// Name:			PendingDelta
// Base Class:		None
//
// Description:		This class stores a database delta.  When
//					a game is saved, all the changes to the database
//					are stored in the save game file.  When this
//					file is loaded, these changes are made back to
//					the gameplay database.
//
//					However, the changes are made before the client
//					has had a chance to initialize.  We store off
//					all these changes until then.  When the server
//					learns the player is fully initialized, the 
//					GameplayManager will ask for this list of
//					pending deltas.  It will then send them all
//					to the client.
//-------------------------------------------------------------
class PendingDelta
{
private:
	str					_objName ;
	str					_propName ;
	str					_stringValue ;
	float				_floatValue ;
	Vector				_vectorValue ;
	GameplayValueType	_type ;
	
public:
	PendingDelta() : _type( GAMEPLAY_VALUE_UNSPECIFIED ) { }
	PendingDelta( const str &objName, const str &propName, float floatValue )		
		: _objName( objName ), _propName( propName ), _floatValue( floatValue ), _type( VALUE_FLOAT ) { }
	PendingDelta( const str &objName, const str &propName, const str &stringValue ) 
		: _objName( objName ), _propName( propName ), _stringValue( stringValue ), _type( VALUE_STRING ) { }
	PendingDelta( const str &objName, const str &propName, const Vector &vectorValue ) 
		: _objName( objName ), _propName( propName ), _vectorValue( vectorValue ), _type( VALUE_VECTOR ) { }

	float				getFloatValue()			{ return _floatValue ; }
	const str&			getStringValue()		{ return _stringValue ; }
	const Vector&		getVectorValue()		{ return _vectorValue ; }
	const str&			getObjectName()			{ return _objName ; }
	const str&			getPropertyName()		{ return _propName ; }
	GameplayValueType	getGameplayValueType()	{ return _type ; }
};


//------------------------- CLASS ------------------------------
//
// Name:			GameplayProperty
// Base Class:		Class
//
// Description:		Object that has a key and a value which can be
//					a string or a float
//
// Method of Use:	Used in GameplayObject's 
//
//--------------------------------------------------------------
class GameplayProperty : public Class
{
private:
	str					_name;
	str					_valuestr;
	float				_valuefloat;
	Vector				_valuevector;
	bool				_modified;
	GameplayValueType	_type;

public:

	CLASS_PROTOTYPE( GameplayProperty );

	GameplayProperty()
		:	_name(""),
			_valuestr(""),
			_valuefloat(1.0f),
			_modified(false),
			_type(VALUE_FLOAT),
			_valuevector(vec_zero)
		{}

	virtual ~GameplayProperty() {}

	// Parsing
	bool parseProperty(Parser &gameplayFile, const str& type);
	bool isModified( void )			{ return _modified ; }

	// Accessors -- Gets
	const str& getName()			{ return _name; }
	const str& getStringValue()		{ return _valuestr; }
	float getFloatValue()			{ return _valuefloat; }
	const Vector& getVectorValue()	{ return _valuevector; }
	bool getModified()				{ return _modified; }
	GameplayValueType getType()		{ return _type; }
	const str getFloatValueStr();

	// Accessors -- Sets
	void setName(const str& name);
	void setModified(bool modified)				{ _modified = modified; }
	void setType(GameplayValueType type)		{ _type = type; }
	bool setStringValue(const str& valuestr);
	bool setFloatValue(float valuefloat);
	bool setVectorValue(const Vector& vector);

#ifdef GAME_DLL
public:
	void Archive(Archiver &arc);
#endif // GAME_DLL
};


//------------------------- CLASS ------------------------------
//
// Name:			GameplayObject
// Base Class:		Class
//
// Description:		Object that has a name, and a container of 
//					GameplayProperty's.
//
// Method of Use:	Used in GameplayDatabase
//
//--------------------------------------------------------------
class GameplayObject : public Class
{
private:
	str				_name;
	str				_category;
	str				_fullScopeName;
	int				_depth;
	bool			_modified ;
	GameplayObject* _baseObject;
	GameplayObject* _nextHashObject;

	Container<GameplayProperty *> _propertyList;
	Container<GameplayObject *> _subObjectList;

public:

	CLASS_PROTOTYPE( GameplayObject );

	GameplayObject();
	GameplayObject(int depth);
	virtual ~GameplayObject();

	// Parsing
	bool parseObject(Parser &gameplayFile, const str& name);

	//Queries
	bool hasProperty(const str& propname, bool localonly = false);
	bool isModified( void )									{ return _modified ; }

	// Accessors -- Gets
	const str& getName()									{ return _name; }
	const str& getCategory()								{ return _category; }
	const str& getFullScopeName()							{ return _fullScopeName; }
	const Container<GameplayObject *>& getSubObjectList()	{ return _subObjectList; };
	const Container<GameplayProperty *>& getPropertyList()	{ return _propertyList; };
	GameplayObject* getBaseObject()							{ return _baseObject; }
	GameplayObject* getNextObject()							{ return _nextHashObject; }
	GameplayObject* getSubObject(const str& subobjname);
	GameplayProperty* getProperty(const str& propname);
	float getPropertyFloatValue(const str& propname);
	const str getPropertyStringValue(const str& propname);
	bool getModified(const str& propname);
	int	getNumberOfModifiedProperties( void );

	// Accessors -- Sets
	void setModified( bool modified )					{ _modified = modified ; }
	void setName(const str& name)						{ _name = name; }
	void setCategory(const str& category)				{ _category = category; }
	void setFullScopeName(const str& fullscope)			{ _fullScopeName = fullscope; }
	void setBaseObject(GameplayObject* baseObject)		{ _baseObject = baseObject; }
	void setNextObject(GameplayObject *nextObject)		{ _nextHashObject = nextObject; }
	bool setFloatValue(const str& propname, float value, bool create = false);
	bool setStringValue(const str& propname, const str& valuestr, bool create = false);
	bool setVectorValue( const str& propname, const Vector& valuevector, bool create = false );

	// Deletion
	bool removeProperty(const str& propname);

#ifdef GAME_DLL
public:
	void Archive(Archiver &arc);
#endif // GAME_DLL
};


//------------------------- CLASS ------------------------------
//
// Name:			GameplayDatabase
// Base Class:		Listener
//
// Description:		Database of GameplayObjects.  Queries are made
//					to this database to retrieve the data
//
// Method of Use:	Used by the GameplayManager
//
//--------------------------------------------------------------
class GameplayDatabase  : public Listener
{
private:
	Container<PendingDelta>	_pendingDeltaList ;
	Container<GameplayObject *> _objectList;
	GameplayObject* _hashTable[HASH_SIZE];

	// Saved off last object reference
	GameplayObject *_lastObj;
	str				_lastObjName;

	// Private Functions
	void			_linkSubObjectsToBase();
	void			_linkToBase(GameplayObject *object);
	GameplayObject* _createFromScope(const str& scope);
	bool			_addToHashTable(GameplayObject *object);
	void			_buildHashTable();

public:

	CLASS_PROTOTYPE( GameplayDatabase );

	GameplayDatabase();
	virtual ~GameplayDatabase();

	// Parsing
	bool			parseFile(const str& filename);

	// Queries
	bool			hasObject(const str& objname);

	// Accessors -- Gets
	GameplayObject* getObject(const str& objname);
	GameplayObject* getRootObject(const str& objname);
	GameplayObject* getSubObject(const str& objname, const str& subobjname);
	int				getNumberOfModifiedObjects( void );
	float			getFloatValue(const str& objname, const str& propname);
	const str		getStringValue(const str& objname, const str& propname);
	Container<PendingDelta>&	getPendingDeltaList()							{ return _pendingDeltaList ; }

	// Accessors -- Sets
	bool			setFloatValue(const str& objname, const str& propname, float value, bool create = false);
	bool			setStringValue(const str& objname, const str& propname, const str& valuestr, bool create = false);
	bool			setVectorValue( const str& objname, const str& propname, const Vector& valuevalue, bool create = false );
	bool			setBase(const str& objname, const str& baseobj);
	bool			clearPropertyOverrides(const str& objname);
	void			clearPendingDeltaList()										{ _pendingDeltaList.FreeObjectList(); }

#ifdef GAME_DLL
public:
	void			Archive(Archiver &arc);
#endif // GAME_DLL
};

#endif
