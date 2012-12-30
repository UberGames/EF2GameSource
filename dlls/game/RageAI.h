//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/RageAI.h                              $
// $Revision:: 30                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// The Rage AI System, will house several components that will make the AI function
// such as the Strategos, Personality, etc...
//

//============================
// Forward Declarations
//============================
class Strategos;
class DefaultStrategos;
class PackageManager;
class Personality;

#ifndef __RAGE_AI_H__
#define __RAGE_AI_H__

#include "actor.h"
#include "actorincludes.h"
#include "characterstate.h"
#include "script.h"

//============================
// Global Functions
//============================
void FillBehaviorPackageList( void );
void ClearBehaviorPackageList( void );

//============================
// Class Strategos 
//============================
//
// Handles all strategic thinking for the actor
// This will get more complicated as the AI gets more
// sophisticated
//
class Strategos 
   {
	public:
			 Strategos()             { }
			 Strategos( Actor *act ) { }
		virtual ~Strategos()             { }
		
		virtual void Evaluate() {	}
		virtual void NotifySightStatusChanged ( Entity *enemy , qboolean canSee ) {	}		
		virtual void NotifyDamageChanged( Entity *enemy , float damage ) { }
		
		virtual void Attack ( Entity *enemy ) {	}
		virtual void DoArchive( Archiver &arc , Actor *act );
		/* virtual */ void Archive( Archiver &arc );
		virtual void SetBehaviorPackage( const str &packageName )    {  }

      virtual float GetCheckYawMin() { assert( 0 ); return 0; }
      virtual float GetCheckYawMax() { assert( 0 ); return 0; }
	  virtual float GetCheckInConeDistMax() { assert( 0 ); return 0; }
	  virtual void  SetCheckInConeDistMax( float distance ) { assert( 0 ); }
				
		//Accessors and Mutators
		void  SetEvaluateInterval( float interval ) { _evaluateInterval = interval;	}
		float GetEvaluateInterval()                 { return _evaluateInterval;       }
		
		void  SetNextEvaluateTime( float time )     { _nextEvaluateTime = time;    	}
		float GetNextEvaluateTime()                 { return _nextEvaluateTime;    	}

		void  SetSightBasedHate( float hate )       { _sightBasedHate = hate;         }
		float GetSightBasedHate()                   { return _sightBasedHate;      	}


	private:
		float    _sightBasedHate;
		float    _nextEvaluateTime;
		float    _evaluateInterval;      
      
      


	};


//============================
// Class DefaultStrategos 
//============================
//
// The Strategos supplied by default.  All actors get this
// in construction.  As additional subclasses are added
// events will be put into place in Actor that will allow
// the specification of the strategos to use from the TIKI file
//
class DefaultStrategos : public Strategos
{
public:

	DefaultStrategos();
	DefaultStrategos( Actor *act );
	~DefaultStrategos();

	void Evaluate();		
	void NotifySightStatusChanged ( Entity *enemy , qboolean canSee );		
	void NotifyDamageChanged( Entity *enemy, float damage );
	void SetBehaviorPackage( const str &packageName );

	float GetCheckYawMin();
	float GetCheckYawMax();
	float GetCheckInConeDistMax();
	
	void SetCheckInConeDistMax( float distance );
  
	void Attack ( Entity *enemy );		
	void DoArchive ( Archiver &arc, Actor *actor );
	/* virtual */ void Archive( Archiver &arc );
			 
private: // Functions
	
	void  _EvaluateEnemies();
	void  _EvaluatePackages();
	void  _EvaluateWorld();
	void  _CheckForInTheWay();
	void  _CheckForInConeOfFire();
	
private: // Member Variables

	Actor *act;
	float _checkYawMin;
	float _checkYawMax;
	float _checkInConeDistMax;
	
	
	
};



//============================
// PackageManager
//============================
//
// Handles behavior packages.  It should be noted that, if an
// actor has a statemap, but no fuzzyengine, then it will just execute
// the specified statemap.  A fuzzyengine MUST be supplied if the actor
// is going to take advantage of the PackageManager.
//

class PackageManager 
	{
	public:
		PackageManager();
		PackageManager( Actor *actor );
	   ~PackageManager();
	
		void RegisterPackage( const str &packageName );
		void UnregisterPackage( const str &packageName );

		void  EvaluatePackages( FuzzyEngine *fEngine );
		int   GetHighestScoringPackage();
		int   GetCurrentFVarIndex();
		float GetCurrentFVarLastExecuteTime();
		void  SetLastExecutionTime(int packageIndex);
		void  UpdateCurrentPackageIndex( int packageIndex );		
		int	  GetPackageIndex( const str &packageName );
		str	  GetCurrentPackageName();

		void DoArchive( Archiver &arc , Actor *actor );				

	private: // Member Variables
		Actor *act;
		Container<BehaviorPackageEntry_t> _BehaviorPackages;

		int						_currentFVarIndex;
		float					_currentFVarLastExecuteTime;
		int						_currentPackageIndex;
	};


//============================
// Class Personality
//============================
//
// Stores Personality Measures.  Now, you maybe asking why am I making
// a full blown class if all I'm doing is storing data.  Well, I'm setting
// this up for future expansion, so when I realize I need this class do something
// to the data its storing, it won't be a huge pain to make that happen
//

class Personality 
   {
   public: 
		      Personality();
				Personality( Actor *actor );
			  ~Personality();

      void  SetBehaviorTendency( const str& packageName , float tendency );
      void  SetTendency ( const str& tendencyName , float tendencyValue );

      qboolean WantsToExecuteCurrentPackage(float interval);
      qboolean ExecutedPackageInLastTimeFrame(float interval);

		void  SetAggressiveness( float aggressiveness );
      float GetAggressiveness();

      void  SetTalkiness( float talkiness );
      float GetTalkiness();

      float GetTendency( const str& tendencyName );

		virtual void Archive( Archiver &arc );
		void  DoArchive ( Archiver &arc, Actor *actor );

	protected: // Member Functions
		float _clampValue( float value );
      qboolean _wantsToExecutePackage(PackageTendency_t *tendency);
      

	private: // Emotions and Tendencies
      
      float _aggressiveness;
      float _talkiness;

      float _anger;
      float _fear;
      
      
      // Package Tendencies
      Container<PackageTendency_t> _PackageTendencies;
      Container<Tendency_t> _Tendencies;
		
	private: // Member Variables			
		Actor *act;

	};


#endif /* __RAGE_AI_H__ */
