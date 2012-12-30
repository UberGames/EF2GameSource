// GameplayFormulaManager.h: interface for the GameplayFormulaManager class.
//
//////////////////////////////////////////////////////////////////////

class GameplayFormulaManager;
class GameplayFormula;
class GameplayFormulaVariable;
class GameplayFormulaData;
class GameplayFormulaOperand;

#ifndef __GAMEPLAY_FORMULA_MANAGER_H__
#define __GAMEPLAY_FORMULA_MANAGER_H__

#include "g_local.h"
#include "actor.h"
#include "weapon.h"
#include "entity.h"
#include "player.h"


//------------------------- CLASS ------------------------------
//
// Name:			GameplayFormulaData
// Base Class:		Class
//
// Description:		Utility class to the GameplayFormulaManager
//
// Method of Use:	The user will build on of these objects to pass
//					into the GameplayManager query functions.
//
//--------------------------------------------------------------
class GameplayFormulaData : public Class
{
public:
	GameplayFormulaData(	Entity *pPrimary = 0, 
							Entity *pSecondary = 0, 
							Entity *pWeapon = 0,
							const str& pAttackType = "");

	Entity *primary;
	Entity *secondary;
	Entity *weapon;
	str attackType;;
};





//------------------------- CLASS ------------------------------
//
// Name:			GameplayFormulaVariable
// Base Class:		Class
//
// Description:		Variable that contains the list of categories that
//					are specific to a variable type.
//
// Method of Use:	GameplayFormula uses this class
//
//--------------------------------------------------------------
class GameplayFormulaVariable : public Class
{
private:
	str				_name;
	Container<str>	_categoryList;

public:
	GameplayFormulaVariable();
	virtual ~GameplayFormulaVariable();

	// Accessors
	const str& getName()			{ return _name; }
	void setName(const str& name)	{ _name = name; }
};





//------------------------- CLASS ------------------------------
//
// Name:			GameplayFormulaOperand
// Base Class:		Class
//
// Description:		A operand in a formula
//
// Method of Use:	GameplayFormula has a list of these to multiply together
//
//--------------------------------------------------------------
class GameplayFormulaOperand : public Class
{
private:
	float	_constant;
	str		_object;
	str		_property;
	bool	_inverse;

public:
	GameplayFormulaOperand() 
		:	_constant(1.0f),
			_inverse(false)
	{}
	virtual ~GameplayFormulaOperand() {}

	// Accessors
	const str& getObjectName()					{ return _object; }
	void setObjectName(const str& object)		{ _object = object; }

	const str& getPropertyName()				{ return _property; }
	void setPropertyName(const str& property)	{ _property = property; }

	float getConstant()					{ return _constant; }
	void setConstant(float constant)	{ _constant = constant; }

	bool getInverseFlag()				{ return _inverse; }
	void setInverseFlag(bool inverse)	{ _inverse = inverse; }

	// Queries
	float getResult(const GameplayFormulaData& formulaData);

	// Parsing
	bool parseOperand(Script &formulaFile, const str& constant);
};





//------------------------- CLASS ------------------------------
//
// Name:			GameplayFormula
// Base Class:		Class
//
// Description:		A formula in the for the GameplayManager
//
// Method of Use:	GameplayFormulaManager requests data from this class
//
//--------------------------------------------------------------
class GameplayFormula : public Class
{
private:
	str	_name;
	Container<GameplayFormulaOperand *>	_operandList;

public:
	GameplayFormula();
	virtual ~GameplayFormula();

	// Accessors
	const str& getName()			{ return _name; }
	void setName(const str& name)	{ _name = name; }

	// Queries
	float getResult(const GameplayFormulaData& formulaData );

	// Parsing
	bool parseFormula(Script &formulaFile, const str& name);
};





//------------------------- CLASS ------------------------------
//
// Name:			GameplayFormulaManager
// Base Class:		Class
//
// Description:		The manager for all the formulas.  Accessed
//					by the GameplayManager
//
// Method of Use:	GameplayManager uses this class to access formulas
//
//--------------------------------------------------------------
class GameplayFormulaManager : public Class
{
private:
	Container<GameplayFormulaVariable *>	_variableList;
	Container<GameplayFormula *>			_formulaList;
public:
	GameplayFormulaManager();
	virtual ~GameplayFormulaManager();

	// Queries
	float getFormulaResult(const str& formulaName, const GameplayFormulaData& formulaData);
	bool hasFormula(const str& formulaName);

	// Parsing
	bool parseFile(const str& filename);
};

#endif

