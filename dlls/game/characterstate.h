//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/characterstate.h                              $
// $Revision:: 15                                                             $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 1999 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
//

//===========================
// Forward Declarations
//===========================

#ifndef __CHARACTERSTATE_H__
#define __CHARACTERSTATE_H__

#include "g_local.h"
#include "script.h"

enum testcondition_t
   {
   TC_ISTRUE,     // no prefix
   TC_ISFALSE,    // !
   TC_EDGETRUE,   // +
   TC_EDGEFALSE   // -
   };

enum movecontrol_t
   {
   MOVECONTROL_USER,       // Quake style
   MOVECONTROL_LEGS,       // Quake style, legs state system active
   MOVECONTROL_ANIM,       // move based on animation, with full collision testing
   MOVECONTROL_ABSOLUTE,   // move based on animation, with full collision testing but no turning
   MOVECONTROL_HANGING,    // move based on animation, with full collision testing, hanging
   MOVECONTROL_WALLHUG,    // move based on animation, with full collision testing, hanging
   MOVECONTROL_MONKEYBARS, // move based on animation, with full collision testing, monkey bars
   MOVECONTROL_PIPECRAWL,  // move based on animation, with full collision testing, crawling on pipe
   MOVECONTROL_PIPEHANG,   // move based on animation, with full collision testing, hanging from pipe
   MOVECONTROL_STEPUP,
   MOVECONTROL_ROPE_GRAB,
   MOVECONTROL_ROPE_RELEASE,
   MOVECONTROL_ROPE_MOVE,
   MOVECONTROL_PICKUPENEMY,
   MOVECONTROL_PUSH,
   MOVECONTROL_CLIMBWALL,
   MOVECONTROL_USEANIM,
   MOVECONTROL_CROUCH,
   MOVECONTROL_LOOPUSEANIM,
   MOVECONTROL_USEOBJECT,
   MOVECONTROL_COOLOBJECT,
   MOVECONTROL_FAKEPLAYER
   };

enum cameratype_t
   {
   CAMERA_TOPDOWN,
   CAMERA_BEHIND,
   CAMERA_FRONT,
   CAMERA_SIDE,
   CAMERA_BEHIND_FIXED,
   CAMERA_SIDE_LEFT,
   CAMERA_SIDE_RIGHT,
   CAMERA_BEHIND_NOPITCH
   };

#define DEFAULT_MOVETYPE   MOVECONTROL_LEGS
#define DEFAULT_CAMERA     CAMERA_BEHIND

class Conditional;

template< class Type >
struct Condition
	{
	const char     *name;
   qboolean       ( Type::*func )( Conditional &condition );
   };

class Conditional : public Class
   {
	private :
		qboolean                   result;
		qboolean                   previous_result;
		bool                       checked;

	public :
		Condition<Class>        condition;
		Container<str>			parmList;

		bool                    getResult( testcondition_t test, Entity &ent );
		const char              *getName( void );

								Conditional( void );
								Conditional( const Condition<Class> &condition );

		const char				*getParm( int number );
		void                    addParm( const str &parm );
		int						numParms( void );
		void					clearCheck( void );
   };

inline void Conditional::addParm
   (
   const str &parm
   )

   {
   parmList.AddObject( parm );
   }

inline const char *Conditional::getParm
   (
   int number
   )

   {
   if ( ( number < 1 ) || ( number > parmList.NumObjects() ) )
      {
      gi.Error( ERR_DROP, "Parm #%d out of range on %s condition\n", number, condition.name );
      }
   return parmList.ObjectAt( number ).c_str();
   }

inline int Conditional::numParms
   (
   void
   )

   {
   return parmList.NumObjects();
   }

inline void Conditional::clearCheck
   (
   void
   )

   {
   checked = false;
   }

inline const char *Conditional::getName
   (
   void
   )

   {
   return condition.name;
   }

inline bool Conditional::getResult
   (
   testcondition_t test,
   Entity &ent
   )

   {

   if ( condition.func && !checked )
      {
      checked = true;
		previous_result = result;

		result = ( ent.*condition.func )( (Conditional&)*this );
      }

   switch( test )
      {
      case TC_ISFALSE :
         return !result;
         break;

      case TC_EDGETRUE :
         return result && !previous_result;
         break;

      case TC_EDGEFALSE :
         return !result && previous_result;
         break;

      case TC_ISTRUE :
      default :
         return result != false;
      }
   }

class State;
class StateMap;
class FuzzyVar;

class Expression : public Class
   {
   private :
      struct condition_t
         {
         testcondition_t      test;
         int			         condition_index;
         };

      str                     value;
      Container<condition_t>  conditions;
		
	// For Fuzzy Vars
	float                   points;

   public :
                              Expression();
                              Expression( const Expression &exp );
                              Expression( Script &script, State &state );
							  Expression( Script &script, FuzzyVar &fuzzyVar );

      void		               operator=( const Expression &exp );

		bool                    getResult( const State &state, Entity &ent, const Container<Conditional *> *sent_conditionals );		
		bool                    getResult( const FuzzyVar &fVar, Entity &ent, const Container<Conditional *> *sent_conditionals );		
      const char              *getValue( void );

		// For Fuzzy Vars
		float                   getPoints();
   };

inline void Expression::operator=
	(
   const Expression &exp
   )

   {
	int i;
	Expression *temp_exp = (Expression *)&exp;		// This is here to get around some const issues

   value = exp.value;
	points = exp.points;

   conditions.FreeObjectList();
   for( i = 1; i <= temp_exp->conditions.NumObjects(); i++ )
      {
      conditions.AddObject( temp_exp->conditions.ObjectAt( i ) );
      }
   }

inline const char *Expression::getValue
   (
   void
   )

   {
   return value.c_str();
   }

inline float Expression::getPoints()
	{
	return points;
	}

class State : public Class
   {
   private :
      Container<int>					condition_indexes;

      StateMap                   &statemap;

      str                        name;

      str                        nextState;
      movecontrol_t              movetype;
      cameratype_t               cameratype;

		str								behaviorName;
		Container<str>					behaviorParmList;

		str								headBehaviorName;
		Container<str>					headBehaviorParmList;

		str                        eyeBehaviorName;
		Container<str>             eyeBehaviorParmList;

		str								torsoBehaviorName;
		Container<str>             torsoBehaviorParmList;

		float								minTime;
		float								maxTime;

      Container<Expression>      legAnims;
      Container<Expression>      torsoAnims;

      Container<Expression>      states;
      Container<str>             entryCommands;
      Container<str>             exitCommands;
		
		qboolean                   ignoreGlobalStates;

      void                       readNextState( Script &script );
      void                       readMoveType( Script &script );
      void                       readCamera( Script &script );
      void                       readLegs( Script &script );
      void                       readTorso( Script &script );
		void                       readBehavior( Script &script );
		void								readHeadBehavior( Script &script );
		void                       readEyeBehavior( Script &script );
		void                       readTorsoBehavior( Script &script );
		void                       readTime( Script &script );
      void                       readStates( Script &script );		
      void                       readCommands( Script &script, Container<str> &container );

      void                       ParseAndProcessCommand( const str &command, Entity *target );

   public :
                                 State( const char *name, Script &script, StateMap &map );

      State                      *Evaluate( Entity &ent, Container<Conditional *> *ent_conditionals );
      int								addCondition( const char *name, Script &script );
      void                       CheckStates( void );

      const char                 *getName( void );

      const char                 *getLegAnim( Entity &ent, Container<Conditional *> *sent_conditionals );
      const char                 *getTorsoAnim( Entity &ent, Container<Conditional *> *sent_conditionals );
		const char                 *getBehaviorName( void );
		const char						*getHeadBehaviorName( void );
		const char                 *getEyeBehaviorName( void );
		const char                 *getTorsoBehaviorName( void );
      State                      *getNextState( void );
      movecontrol_t              getMoveType( void );
      cameratype_t               getCameraType( void );
      qboolean                   setCameraType( const str &ctype );

		const char						*getBehaviorParm( int number=1 );
		const char						*getHeadBehaviorParm( int number=1 );
		const char                 *getEyeBehaviorParm( int number=1 );
		const char                 *getTorsoBehaviorParm( int number=1 );
      void                       addBehaviorParm( const str &parm );
		void								addHeadBehaviorParm( const str &parm );
		void                       addEyeBehaviorParm( const str &parm );
		void                       addTorsoBehaviorParm( const str &parm );
		int								numBehaviorParms( void );
		int								numHeadBehaviorParms( void );
		int                        numEyeBehaviorParms( void );
		int                        numTorsoBehaviorParms( void );

		float								getMinTime( void );
		float								getMaxTime( void );
      void                       ProcessEntryCommands( Entity *target );
      void                       ProcessExitCommands( Entity *target );
      void                       GetLegAnims( Container<const char *> *c );
      void                       GetTorsoAnims( Container<const char *> *c );
		qboolean                   IgnoreGlobalStates();
   };

inline void State::addBehaviorParm
   (
   const str &parm
   )

   {
   behaviorParmList.AddObject( parm );
   }

inline void State::addHeadBehaviorParm
	(
	const str &parm
	)

	{
	headBehaviorParmList.AddObject( parm );
	}

inline void State::addEyeBehaviorParm
   (
	const str &parm
	)

	{
   eyeBehaviorParmList.AddObject( parm );
	}

inline void State::addTorsoBehaviorParm
   (
	const str &parm
	)

	{
   torsoBehaviorParmList.AddObject( parm );
	}

inline const char *State::getBehaviorParm
   (
   int number
   )

   {
   return behaviorParmList.ObjectAt( number ).c_str();
   }

inline const char *State::getHeadBehaviorParm
	(
	int number
	)

	{
	return headBehaviorParmList.ObjectAt( number ).c_str();
	}

inline const char *State::getEyeBehaviorParm
   (
	int number
	)

	{
   return eyeBehaviorParmList.ObjectAt( number ).c_str();
	}

inline const char *State::getTorsoBehaviorParm
   (
	int number
	)

	{
   return torsoBehaviorParmList.ObjectAt( number ).c_str();
	}

inline int State::numBehaviorParms
   (
   void
   )

   {
   return behaviorParmList.NumObjects();
   }

inline int State::numHeadBehaviorParms 
	(
	void
	)

	{
	return headBehaviorParmList.NumObjects();
	}

inline int State::numEyeBehaviorParms
   (
	void
	)

	{
   return eyeBehaviorParmList.NumObjects();
	}

inline int State::numTorsoBehaviorParms
   (
	void
	)

	{
   return torsoBehaviorParmList.NumObjects();
	}

class StateMap : public Class
	{
	private :
		Container<State *>         stateList;
		Container<State *>		   globalStateList;
		Condition<Class>           *current_conditions;
		Container<Conditional *>   *current_conditionals;
		str						   filename;				

	public :
		StateMap();
		StateMap( const char *filename, Condition<Class> *conditions, Container<Conditional *> *conditionals );
		~StateMap();

		Condition<Class>		*getCondition( const char *name );
		int						findConditional( Conditional *condition );
		int						addConditional( Conditional *condition );
		Conditional				*getConditional( const char *name );
		void					GetAllAnims( Container<const char *> *c );
		State					*FindState( const char *name );
		State					*FindGlobalState( const char *name );
		const char				*Filename();
		void					ReadStates( Script *script );
		State					*GotoGlobalState( State *CurrentState, State *newState );
	};

inline const char *StateMap::Filename
   (
   void
   )

   {
   return filename.c_str();
   }

inline const char *State::getName
   (
   void
   )

   {
   return name.c_str();
   }

inline State *State::getNextState
   (
   void
   )

   {
   return statemap.FindState( nextState.c_str() );
   }

inline movecontrol_t State::getMoveType
   (
   void
   )

   {
   return movetype;
   }

inline cameratype_t State::getCameraType
   (
   void
   )

   {
   return cameratype;
   }




//================================================
// Class Name: Fuzzy Engine
//
// Description:
//
//================================================
class FuzzyEngine : public Class
   {
   public :
                                 FuzzyEngine( const char *filename, Condition<Class> *conditions, Container<Conditional *> *conditionals );
                                 
      Condition<Class>           *getCondition( const char *name );
      int								findConditional( Conditional *condition );
      int	                     addConditional( Conditional *condition );
      Conditional                *getConditional( const char *name );
      FuzzyVar                   *FindFuzzyVar( const char *name );
		const char						*Filename();
		void								ReadFuzzyVars( const char *filename );
		

   private :
      Container<FuzzyVar *>       _varList;
      Condition<Class>           *_current_conditions;
      Container<Conditional *>   *_current_conditionals;
		str								 _filename;

   };

//================================================
// Class Name: Fuzzy Engine
//
// Description:
//
//================================================
class FuzzyVar : public Class
	{
	public:
                                 FuzzyVar( const char *name, Script &script, FuzzyEngine &f_engine );

      float                      Evaluate( Entity &ent, Container<Conditional *> *ent_conditionals );
      int								addCondition( const char *name, Script &script );
		void                       readEvaluations( Script &script );
      
      const char                 *getName( void );

   private :
      Container<int>					_condition_indexes;
      Container<Expression>      _evaluations;

		FuzzyEngine                &_fuzzyEngine;

      str                        _name;
      Container<float>           _points;
	};

void ClearCachedStatemaps( void );
StateMap *GetStatemap( const str &filename, Condition<Class> *conditions,  Container<Conditional *> *conditionals, qboolean reload, qboolean cache_only = false );
void CacheStatemap( const str &filename, Condition<Class> *conditions	);

void ClearCachedFuzzyEngines( void );
FuzzyEngine *GetFuzzyEngine( const str &filename, Condition<Class> *conditions,  Container<Conditional *> *conditionals, qboolean reload, qboolean cache_only = false );
void CacheFuzzyEngine( const str &filename, Condition<Class> *conditions	);

#endif /* !__CHARACTERSTATE_H__ */
