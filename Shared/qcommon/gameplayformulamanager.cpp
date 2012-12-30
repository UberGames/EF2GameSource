//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/Shared/qcommon/gameplayformulamanager.cpp           $
// $Revision:: 4                                                              $
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
// GameplayFormulaManager.cpp: implementation of the GameplayFormulaManager class.
//
//////////////////////////////////////////////////////////////////////

#ifdef GAME_DLL

#include "gameplayformulamanager.h"
#include "gameplaymanager.h"


////////////////////////////////////////////////////////////////
//
//		GameplayFormulaData CLASS
//
////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
//
// Name:			GameplayFormulaData
// Class:			GameplayFormulaData
//
// Description:		Constructor
//
// Parameters:		Entity *primary -- Default 0
//					Entity *secondary -- Default 0
//					Entity *weapon -- Default 0
//					const str& attackType -- Default ""
//
// Returns:			
//
//--------------------------------------------------------------
GameplayFormulaData::GameplayFormulaData( Entity *pPrimary, Entity *pSecondary, Entity *pWeapon, const str& pAttackType )
{
	primary = pPrimary;
	secondary = pSecondary;
	weapon = pWeapon;
	attackType = pAttackType;
}






////////////////////////////////////////////////////////////////
//
//		GameplayFormulaVariable CLASS
//
////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
//
// Name:			GameplayFormulaVariable
// Class:			GameplayFormulaVariable
//
// Description:		Constructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayFormulaVariable::GameplayFormulaVariable()
{
	_categoryList.ClearObjectList();
}

//--------------------------------------------------------------
//
// Name:			~GameplayFormulaVariable
// Class:			GameplayFormulaVariable
//
// Description:		Destructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayFormulaVariable::~GameplayFormulaVariable()
{

}





////////////////////////////////////////////////////////////////
//
//		GameplayFormulaOperand CLASS
//
////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
//
// Name:			getResult
// Class:			GameplayFormulaOperand
//
// Description:		Gets the result of the operand given the data
//
// Parameters:		const GameplayFormulaData& formulaData -- Data to use
//
// Returns:			float -- The result, or 1.0 if there's a problem
//
//--------------------------------------------------------------
float GameplayFormulaOperand::getResult(const GameplayFormulaData& formulaData)
{
	float finalResult = _constant; // Constant is factored in here.
	str name;
	GameplayManager *gpm = GameplayManager::getTheGameplayManager();

	char firstobj[255], *sptr;
	strcpy(firstobj, _object.c_str());
	sptr = strchr(firstobj, '.');
	sptr = 0;

	// Defines are a special case
	if ( !strcmp(firstobj,"Defines") )
	{
		finalResult *= gpm->getFloatValue(_property, "value");
		return finalResult;
	}

	if ( !strcmp(firstobj, "Primary") && formulaData.primary )
		name = formulaData.primary->getArchetype();
	if ( !strcmp(firstobj, "Secondary") && formulaData.secondary )
		name = formulaData.secondary->getArchetype();
	if ( !strcmp(firstobj, "Weapon") && formulaData.weapon )
		name = formulaData.weapon->getArchetype();
	if ( !strcmp(firstobj, "AttackType") )
		name = formulaData.attackType;
	
	// The object was not one of the code-keywords, check the database
	// itself for the object and property.
	if ( !name.length() )
	{
		finalResult *= gpm->getFloatValue(_object, _property);
		return finalResult;
	}

	if ( _inverse )
		finalResult *= (1.0f / gpm->getFloatValue(name, _property));
	else
		finalResult *= gpm->getFloatValue(name, _property);
	
	return finalResult;
}

//--------------------------------------------------------------
//
// Name:			paraseOperand
// Class:			GameplayFormulaOperand
//
// Description:		Parses the operand
//
// Parameters:		Script &formulaFile -- The file to parse
//					const str& token -- The first token
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayFormulaOperand::parseOperand(Script &formulaFile, const str& constant)
{
	const char *token;
	_constant = (float)atof(constant);

	if ( !formulaFile.TokenAvailable(false) )
		return false;
	token = formulaFile.GetToken(false);
	if ( token[0] == '/' )
		_inverse = true;

	if ( !formulaFile.TokenAvailable(false) )
		return false;
	token = formulaFile.GetToken(false);
	
	char *sptr, tmpstr[255];
	strcpy(tmpstr, token);
	sptr = strrchr(tmpstr,'.');
	if ( sptr )
	{
		*sptr = 0;
		sptr++;
	}

	_object = tmpstr;
	_property = sptr;

	return true;
}

////////////////////////////////////////////////////////////////
//
//		GameplayFormula CLASS
//
////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
//
// Name:			GameplayFormula
// Class:			GameplayFormula
//
// Description:		Constructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayFormula::GameplayFormula()
{
	_operandList.ClearObjectList();
}

//--------------------------------------------------------------
//
// Name:			~GameplayFormula
// Class:			GameplayFormula
//
// Description:		Destructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayFormula::~GameplayFormula()
{
	int i;
	for ( i=1; i<=_operandList.NumObjects(); i++ )
	{
		GameplayFormulaOperand *gpfo = _operandList.ObjectAt(i);
		delete gpfo;
	}

	_operandList.ClearObjectList();
}


//--------------------------------------------------------------
//
// Name:			getResult
// Class:			GameplayFormula
//
// Description:		Gets the result of a formula given it's the data
//
// Parameters:		const GameplayFormulaData& formulaData -- Data to use
//
// Returns:			float -- The result, or 1.0 if there's a problem
//
//--------------------------------------------------------------
float GameplayFormula::getResult(const GameplayFormulaData& formulaData)
{
	float finalResult = 1.0f;

	int i;
	for ( i=1; i<=_operandList.NumObjects(); i++ )
	{
		GameplayFormulaOperand *gpfo = _operandList.ObjectAt(i);
		finalResult *= gpfo->getResult(formulaData);
	}

	return finalResult;
}

//--------------------------------------------------------------
//
// Name:			paraseFormula
// Class:			GameplayFormula
//
// Description:		Parses the formula
//
// Parameters:		Script &formulaFile -- The file to parse
//					const str& name -- The token
//
// Returns:			
//
//--------------------------------------------------------------
bool GameplayFormula::parseFormula(Script &formulaFile, const str& name)
{
	const char *token;
	GameplayFormulaOperand *gpfo;
	
	if ( name != "FORMULA" )
		return false;

	if ( !formulaFile.TokenAvailable(false) )
		return false;
	
	token = formulaFile.GetToken(false);
	setName(token);

	// Get the open brace
	token = formulaFile.GetToken(true);
	if ( token[0] != '{' )
		assert(0);

	while (formulaFile.TokenAvailable(true))
	{
		token = formulaFile.GetToken(true);

		// If we have a close brace, we're done.
		if ( token[0] == '}' )
			return true;

		gpfo = new GameplayFormulaOperand();
		if ( gpfo->parseOperand(formulaFile, token) )
			_operandList.AddObject(gpfo);
		else
		{
			delete gpfo;
			return false;
		}
	}

	// Premature end of file, missing close brace.
	return false;
}


////////////////////////////////////////////////////////////////
//
//		GameplayFormulaManager CLASS
//
////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
//
// Name:			GameplayFormulaManager
// Class:			GameplayFormulaManager
//
// Description:		Constructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayFormulaManager::GameplayFormulaManager()
{
	_variableList.ClearObjectList();
	_formulaList.ClearObjectList();
}


//--------------------------------------------------------------
//
// Name:			~GameplayFormulaManager
// Class:			GameplayFormulaManager
//
// Description:		Destructor
//
// Parameters:		None
//
// Returns:			None
//
//--------------------------------------------------------------
GameplayFormulaManager::~GameplayFormulaManager()
{
	int i;
	for ( i=1; i<=_variableList.NumObjects(); i++ )
	{
		GameplayFormulaVariable *gpfv = _variableList.ObjectAt(i);
		delete gpfv;
	}

	for ( i=1; i<=_formulaList.NumObjects(); i++ )
	{
		GameplayFormula *gpf = _formulaList.ObjectAt(i);
		delete gpf;
	}

	_variableList.FreeObjectList();
	_formulaList.FreeObjectList();
}


//--------------------------------------------------------------
//
// Name:			getFormulaResult
// Class:			GameplayFormulaManager
//
// Description:		Gets the result of a formula given it's name and the data
//
// Parameters:		const str& formulaName -- Name of the formula to use
//					const GameplayFormulaData& formulaData -- Data to use
//
// Returns:			float -- The result, or 1.0 if there's a problem
//
//--------------------------------------------------------------
float GameplayFormulaManager::getFormulaResult(const str& formulaName, const GameplayFormulaData& formulaData)
{
	float finalResult = 1.0f;

	int i;
	for ( i=1; i<=_formulaList.NumObjects(); i++ )
	{
		GameplayFormula *gpf = _formulaList.ObjectAt(i);
		if ( gpf->getName() == formulaName )
		{
			// Found the matching formula, get the result.
			finalResult = gpf->getResult(formulaData);
			return finalResult;
		}
	}

	return finalResult;
}


//--------------------------------------------------------------
//
// Name:			parseFile
// Class:			GameplayFormulaManager
//
// Description:		Reads and parses the formula file
//
// Parameters:		const str& filename -- Name of the file
//
// Returns:			bool -- sucessful parse or not
//
//--------------------------------------------------------------
bool GameplayFormulaManager::parseFile(const str& filename)
{
	Script			formulaFile;
	const char		*token;
	GameplayFormula	*gpformula;

	if ( !gi.FS_Exists(filename.c_str()) )
		return false;

	formulaFile.LoadFile(filename.c_str());

	while (formulaFile.TokenAvailable(true))
	{
		token = formulaFile.GetToken(false);
		
		// If the first token isn't a formula, there's a problem
		if ( strcmp(token, "FORMULA") )
			return false;

		gpformula = new GameplayFormula();
		if ( gpformula->parseFormula(formulaFile, token) )
			_formulaList.AddObject(gpformula);
		else
		{
			delete gpformula;
			return false;
		}
	}

	return true;
}


//--------------------------------------------------------------
//
// Name:			hasFormula
// Class:			GameplayFormulaManager
//
// Description:		Checks to see if the formula exists
//
// Parameters:		const str& formulaName
//
// Returns:			bool
//
//--------------------------------------------------------------
bool GameplayFormulaManager::hasFormula(const str& formulaName)
{
	int i;
	for ( i=1; i<=_formulaList.NumObjects(); i++ )
	{
		GameplayFormula *gpf = _formulaList.ObjectAt(i);
		if ( gpf->getName() == formulaName )
			return true;
	}

	return false;
}

#endif // GAME_DLL


