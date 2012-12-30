// GameplayDatabase.h: interface for the GameplayDatabase class.
//
//////////////////////////////////////////////////////////////////////

class GameplayDatabase;
class GameplayObject;
class GameplayProperty;

#ifndef __GAMEPLAYDATABASE_H__
#define __GAMEPLAYDATABASE_H__

#ifdef GAME_DLL
#include "g_local.h"
#endif // GAME_DLL
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
	str		_name;
	str		_valuestr;
	float	_valuefloat;
	bool	_modified;

public:
	GameplayProperty()
		:	_name(""),
			_valuestr(""),
			_valuefloat(1.0f),
			_modified(false)
		{}

	virtual ~GameplayProperty() {}

	// Parsing
	bool parseProperty(Script &gameplayFile, const str& name);

	// Accessors -- Gets
	const str& getName()			{ return _name; }
	const str& getStringValue()		{ return _valuestr; }
	float getFloatValue()			{ return _valuefloat; }
	bool getModified()				{ return _modified; }
	const str getFloatValueStr();

	// Accessors -- Sets
	void setName(const str& name)				{ _name = name; }
	void setModified(bool modified)				{ _modified = modified; }
	bool setStringValue(const str& valuestr);
	bool setFloatValue(float valuefloat);

	// Archiving
	void Archive(Archiver &arc);
};

inline void GameplayProperty::Archive(Archiver &arc)
{
	arc.ArchiveString(&_name);
	arc.ArchiveString(&_valuestr);
	arc.ArchiveFloat(&_valuefloat);
	arc.ArchiveBool(&_modified);
}



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
	str _name;
	str _category;
	int _depth;

	GameplayObject* _baseObject;

	Container<GameplayProperty *> _propertyList;
	Container<GameplayObject *> _subObjectList;

public:
	GameplayObject();
	GameplayObject(int depth);
	virtual ~GameplayObject();

	// Parsing
	bool parseObject(Script &gameplayFile, const str& name);

	//Queries
	bool hasProperty(const str& propname);

	// Accessors -- Gets
	const str& getName()									{ return _name; }
	const str& getCategory()								{ return _category; }
	const Container<GameplayObject *>& getSubObjectList()	{ return _subObjectList; };
	GameplayObject* getBaseObject()							{ return _baseObject; }
	GameplayObject* getSubObject(const str& subobjname);
	GameplayProperty* getProperty(const str& propname);
	float getPropertyFloatValue(const str& propname);
	const str getPropertyStringValue(const str& propname);
	bool getModified(const str& propname);

	// Accessors -- Sets
	void setName(const str& name)						{ _name = name; }
	void setCategory(const str& category)				{ _category = category; }
	void setBaseObject(GameplayObject* baseObject)		{ _baseObject = baseObject; }
	bool setFloatValue(const str& propname, float value, bool create = false);
	bool setStringValue(const str& propname, const str& valuestr, bool create = false);

	// Archiving
	void Archive(Archiver &arc);
};

inline void GameplayObject::Archive(Archiver &arc)
{
	// TODO: Archive the container of GameplayProperties
	// TODO: Archive the container of GameplayObjects
}



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
	Container<GameplayObject *> _objectList;
	GameplayObject *_lastObj;
	str	_lastObjName; // Full scope for quick compare

	// Private Functions
	void linkSubObjectsToBase();
	GameplayObject* createFromScope(const str& scope);

public:
	GameplayDatabase();
	virtual ~GameplayDatabase();

	// Parsing
	bool parseFile(const str& filename);

	// Queries
	bool hasObject(const str& objname);

	// Accessors -- Gets
	GameplayObject* getObject(const str& objname);
	GameplayObject* getRootObject(const str& objname);
	GameplayObject* getSubObject(const str& objname, const str& subobjname);
	float getFloatValue(const str& objname, const str& propname);
	const str getStringValue(const str& objname, const str& propname);

	// Accessors -- Sets
	bool setFloatValue(const str& objname, const str& propname, float value, bool create = false);
	bool setStringValue(const str& objname, const str& propname, const str& valuestr, bool create = false);

	// Archiving
	void Archive(Archiver &arc);
};

inline void GameplayDatabase::Archive(Archiver &arc)
{
	// TODO: Archive the container of GameplayObjects
}



#endif
