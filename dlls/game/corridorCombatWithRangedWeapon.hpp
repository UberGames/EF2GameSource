//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/corridorCombatWithRangedWeapon.hpp             $
// $Revision:: 169                                                            $
//   $Author:: sketcher														  $
//     $Date:: 4/26/02 2:22p                                                  $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// CooridorCombatWithRangedWeapon Behavior Definition
//
//--------------------------------------------------------------------------------

//==============================
// Forward Declarations
//==============================
class CorridorCombatWithRangedWeapon;

#ifndef __CORRIDORCOMBAT_WITH_RANGEDWEAPON_HPP__
#define __CORRIDORCOMBAT_WITH_RANGEDWEAPON_HPP__

#include "behavior.h"
#include "behaviors_general.h"

//------------------------- CLASS ------------------------------
//
// Name:           CooridorCombatWithRangedWeapon
// Base Class:     Behavior
//
// Description:   
//
// Method of Use:  Called From State Machine
//--------------------------------------------------------------
class CorridorCombatWithRangedWeapon : public Behavior
	{
	//------------------------------------
	// States
	//------------------------------------
	public:  
		typedef enum
		{	
			CORRIDORCOMBAT_WRW_FINDNODE,
			CORRIDORCOMBAT_WRW_MOVETONODE,
			CORRIDORCOMBAT_WRW_BACKPEDAL,
			CORRIDORCOMBAT_WRW_FINDBETTERNODE,
			CORRIDORCOMBAT_WRW_MOVETOBETTERNODE,
			CORRIDORCOMBAT_WRW_CHANGEPOSTURE_DUCK,
			CORRIDORCOMBAT_WRW_CHANGEPOSTURE_STAND,
			CORRIDORCOMBAT_WRW_DUCKED,
			CORRIDORCOMBAT_WRW_DUCKED_FIRING,			
			CORRIDORCOMBAT_WRW_STAND,
			CORRIDORCOMBAT_WRW_STAND_FIRING,	
			CORRIDORCOMBAT_WRW_HOLD_POSITION,
			CORRIDORCOMBAT_WRW_SUCCESS,
			CORRIDORCOMBAT_WRW_FAILED
		} corridorCombatStates_t;

	//------------------------------------
	// Parameters
	//------------------------------------
	private:
		str		_movementAnim;
		str		_torsoAnim;
		str		_fireAnim;
		str		_preFireAnim;
		str		_postFireAnim;
		float	_postureChangeChance;
		float	_maxDistance;
		float	_retreatDistance;
		float	_threatDistance;
		float	_fireTimeMin;
		float	_fireTimeMax;
		float	_pauseTimeMin;
		float	_pauseTimeMax;		

	//-------------------------------------
	// Internal Functionality
	//-------------------------------------
	protected:
		void	transitionToState	( corridorCombatStates_t state );
		void	setInternalState	( corridorCombatStates_t state , const str &stateName );
		void	init				( Actor &self );
		void	think				();
		void	updateEnemy			();
		void	setTorsoAnim		();	

		bool	checkShouldDuck		();
		bool	checkShouldStand	();
		bool	checkShouldRetreat	();

		void							setupRotate();
		BehaviorReturnCode_t			evaluateRotate();

		void							setupStateFindNode				();
		BehaviorReturnCode_t			evaluateStateFindNode			();
		void							failureStateFindNode			( const str& failureReason );

		void							setupStateMoveToNode			();
		BehaviorReturnCode_t			evaluateStateMoveToNode			();
		void							failureStateMoveToNode			( const str& failureReason );

		void							setupStateBackPedal				();
		BehaviorReturnCode_t			evaluateStateBackPedal			();
		void							failureStateBackPedal			( const str& failureReason );

		void							setupStateFindBetterNode		();
		BehaviorReturnCode_t			evaluateStateFindBetterNode		();
		void							failureStateFindBetterNode		( const str& failureReason );

		void							setupStateMoveToBetterNode		();
		BehaviorReturnCode_t			evaluateStateMoveToBetterNode	();
		void							failureStateMoveToBetterNode	( const str& failureReason );

		void							setupStateChangePostureDuck		();
		BehaviorReturnCode_t			evaluateStateChangePostureDuck	();
		void							failureStateChangePostureDuck	( const str& failureReason );

		void							setupStateChangePostureStand	();
		BehaviorReturnCode_t			evaluateStateChangePostureStand	();
		void							failureStateChangePostureStand	( const str& failureReason );

		void							setupStateDucked				();
		BehaviorReturnCode_t			evaluateStateDucked				();
		void							failureStateDucked				( const str& failureReason );

		void							setupStateFireDucked			();
		BehaviorReturnCode_t			evaluateStateFireDucked			();
		void							failureStateFireDucked			( const str& failureReason );

		void							setupStateFirePauseDucked		();
		BehaviorReturnCode_t			evaluateStateFirePauseDucked	();
		void							failureStateFirePauseDucked		( const str& failureReason );

		void							setupStateStanding				();
		BehaviorReturnCode_t			evaluateStateStanding			();
		void							failureStateStanding			( const str& failureReason );

		void							setupStateFireStanding			();
		BehaviorReturnCode_t			evaluateStateFireStanding		();
		void							failureStateFireStanding		( const str& failureReason );

		void							setupStateFirePauseStanding		();
		BehaviorReturnCode_t			evaluateStateFirePauseStanding	();
		void							failureStateFirePauseStanding	( const str& failureReason );
	
		void							setupStateHoldPosition			();
		BehaviorReturnCode_t			evaluateStateHoldPosition		();
		void							failureStateHoldPosition		( const str& failureReason );


	//-------------------------------------
	// Public Interface
	//-------------------------------------
	public:
		CLASS_PROTOTYPE( CorridorCombatWithRangedWeapon );

										CorridorCombatWithRangedWeapon();
									   ~CorridorCombatWithRangedWeapon();

		void							SetArgs						( Event *ev );      
		void							AnimDone					( Event *ev );
		void							PostureDone					( Event *ev );

		void							Begin						( Actor &self );		
		BehaviorReturnCode_t			Evaluate					( Actor &self );
		void							End							( Actor &self );

		
		void							SetMovementAnim( const str& movementAnim )		{ _movementAnim = movementAnim;				}
		const str&						GetMovementAnim()								{ return _movementAnim;						}

		void							SetTorsoAnim( const str& torsoAnim )			{ _torsoAnim = torsoAnim;					}
		const str&						GetTorsoAnim()									{ return _torsoAnim;						}

		void							SetFireAnim( const str& fireAnim )				{ _fireAnim = fireAnim;						}
		const str&						GetFireAnim()									{ return _fireAnim;							}

		void							SetPreFireAnim( const str& preFireAnim )		{ _preFireAnim = preFireAnim;				}
		const str&						GetPreFireAnim()								{ return _preFireAnim;						}

		void							SetPostFireAnim( const str& postFireAnim )		{ _postFireAnim = postFireAnim;				}
		const str&						GetPostFireAnim()								{ return _postFireAnim;						}

		void							SetPostureChangeChance( float chance )			{ _postureChangeChance = chance;			}
		float							GetPostureChangeChance()						{ return _postureChangeChance;				}

		void							SetMaxDistance( float maxDistance )				{ _maxDistance = maxDistance;				}
		float							GetMaxDistance()								{ return _maxDistance;						}

		void							SetRetreatDistance( float retreatDistance )		{ _retreatDistance = retreatDistance;		}
		float							GetRetreatDistance()							{ return _retreatDistance;					}

		void							SetThreatDistance( float threatDistance )		{ _threatDistance = threatDistance;			}
		float							GetThreatDistance()								{ return _threatDistance;					}

		void							SetFireTimeMin( float fireTimeMin )				{ _fireTimeMin = fireTimeMin;				}
		float							GetFireTimeMin()								{ return _fireTimeMin;						}

		void							SetFireTimeMax( float fireTimeMax )				{ _fireTimeMax = fireTimeMax;				}
		float							GetFireTimeMax()								{ return _fireTimeMax;						}

		void							SetPauseTimeMin( float pauseTimeMin )			{ _pauseTimeMin = pauseTimeMin;				}
		float							GetPauseTimeMin()								{ return _pauseTimeMin;						}

		void							SetPauseTimeMax( float pauseTimeMax )			{ _pauseTimeMax = pauseTimeMax;				}
		float							GetPauseTimeMax()								{ return _pauseTimeMax;						}

		virtual void					Archive  ( Archiver &arc );

		static bool						CanExecute( float maxDistance , Actor &self );

	//-------------------------------------
	// Components
	//-------------------------------------
	private: 	
		GotoPoint						_gotoPoint;
		FireWeapon						_fireWeapon;
		MoveRandomDirection				_moveRandomDir;	
		RotateToEntity					_rotate;


	//-------------------------------------
	// Member Variables
	//-------------------------------------
	private: 
		corridorCombatStates_t			_state;
		HelperNodePtr					_node; 		
		Actor						   *_self;
		EntityPtr						_currentEnemy;
		bool							_finishedPostureTransition;		
		float							_nextFireTime;
		float							_nextPauseTime;				
		float							_holdPositionTime;
		float							_enemyUpdateTime;		

		static const float				NODE_RADIUS;


	};


inline void CorridorCombatWithRangedWeapon::Archive( Archiver &arc	)
{
	Behavior::Archive ( arc );	     

	//
	// Archive Parameters
	//
	arc.ArchiveString		( &_movementAnim			);
	arc.ArchiveString		( &_torsoAnim				);
	arc.ArchiveString		( &_fireAnim				);
	arc.ArchiveString		( &_preFireAnim				);
	arc.ArchiveString		( &_postFireAnim			);
	arc.ArchiveFloat		( &_postureChangeChance		);
	arc.ArchiveFloat		( &_maxDistance				);
	arc.ArchiveFloat		( &_retreatDistance			);
	arc.ArchiveFloat		( &_threatDistance			);
	arc.ArchiveFloat		( &_fireTimeMin				);
	arc.ArchiveFloat		( &_fireTimeMax				);	
	arc.ArchiveFloat		( &_pauseTimeMin			);
	arc.ArchiveFloat		( &_pauseTimeMax			);
	

	//
	// Archive Components
	//
	arc.ArchiveObject		( &_gotoPoint		);
	arc.ArchiveObject		( &_fireWeapon		);
	arc.ArchiveObject		( &_moveRandomDir	);
	arc.ArchiveObject		( &_rotate			);

	//
	// Archive Member Variables
	//
	ArchiveEnum				( _state, corridorCombatStates_t);
	arc.ArchiveSafePointer	( &_node						);
	arc.ArchiveObjectPointer( ( Class ** )&_self			);		
	arc.ArchiveSafePointer	( &_currentEnemy				);
	arc.ArchiveBool			( &_finishedPostureTransition	);	
	arc.ArchiveFloat		( &_nextFireTime				);
	arc.ArchiveFloat		( &_nextPauseTime				);
	arc.ArchiveFloat		( &_holdPositionTime			);
	arc.ArchiveFloat		( &_enemyUpdateTime				);
	
}


#endif /* __CORRIDORCOMBAT_WITH_RANGEDWEAPON_HPP__ */
