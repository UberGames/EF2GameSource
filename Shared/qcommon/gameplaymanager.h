// GameplayManager.h: interface for the GameplayManager class.
//
//////////////////////////////////////////////////////////////////////
class GameplayManager;

#ifndef __GAMEPLAYMANAGER_H__
#define __GAMEPLAYMANAGER_H__

#ifdef GAME_DLL
#include <game/g_local.h>
#include <game/actor.h>
#include <game/weapon.h>
#include <game/player.h>
#include <game/gamecmds.h>
#include "gameplayformulamanager.h"
#endif // GAME_DLL

#ifdef CGAME_DLL
#include "cg_local.h"
#endif // CGAME_DLL


#include "gameplaydatabase.h"

//------------------------- CLASS ------------------------------
//
// Name:          GameplayManager
// Base Class:    Listener
//
// Description:   Singlton class that handles gameplay elements
//
// Method of Use: Use in game code when things need to be resolved
//
//--------------------------------------------------------------
class GameplayManager : public Listener
{
	//-----------------------------------------------------------
	//           D A T A B A S E   S T U F F
	//-----------------------------------------------------------
	public:
		// Static Member functions
		static			GameplayManager* getTheGameplayManager();
		static void		shutdown();
		static bool		isReady();
		static void		create();

		// Queries
		bool			hasProperty(const str& objname, const str& propname);
		bool			hasObject(const str& objname);
		bool			hasSubObject(const str& objname, const str& subobject);
		bool			isDefined(const str& propname);

		// Accessors
		const str		getDefine(const str& propname);
		float			getFloatValue(const str& objname, const str& propname);
		const str		getStringValue(const str& objname, const str& propname);

		// Mutators		NOTE: These functions can only set values on root level objects!
		void			setFloatValue(const str& objname, const str& propname, float value, bool create = false);
		void			setStringValue(const str& objname, const str& propname, const str& valuestr, bool create = false);
		bool			setBase(const str& objname, const str& baseobj);
		bool			clearPropertyOverrides(const str& objname);

	protected:
						GameplayManager();
		virtual		   ~GameplayManager();

	private:
		static GameplayManager *_theGameplayManager;		// singleton
		GameplayDatabase		_gameplayDatabase;

#ifdef GAME_DLL
	//------------------------------------------------------------
	//            F O R M U L A   S T U F F
	//------------------------------------------------------------
	public:
		bool			hasFormula(const str& formulaName);
		float			calculate(const str& formulaName, const GameplayFormulaData& formulaData, float multiplier = 1.0f);

		void			processPendingMessages();
		void			Archive(Archiver &arc);
	private:
		GameplayFormulaManager _gameplayFormulaManager;
#endif // GAME_DLL
};


#endif
