//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/armor.h                                       $
// $Revision:: 38                                                             $
//   $Author:: Sketcher                                                       $
//     $Date:: 2/21/03 6:01p                                                  $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Standard armor that prevents a percentage of damage per hit.
//

#ifndef __ARMOR_H__
#define __ARMOR_H__

#include "item.h"


//
// Defines
//
#define ADAPTION_LIMIT      200
#define FX_CHANGE_DELTA     25

//
// Stucture Name: MODHitCounter
//
// Description: Holds mapping of MODs and counts of hits
//
typedef struct
	{
	int MODIndex;
	float damage;
	bool adapted;
	} MODHitCounter;


//
// Class Name: ArmorEntity
// Base Class: Item
//
// Description: Base Class for all Armor Items
//
class Armor : public Item
	{
	public:
		CLASS_PROTOTYPE( Armor );

			Armor();
			~Armor();
		virtual Item	*ItemPickup( Entity *other, qboolean add_to_inventory, qboolean );
		virtual float   ResolveDamage( float damage , int meansOfDeath , const Vector &direction , const Vector &position , Entity *attacker );
		virtual void    Archive( Archiver &arc );
		virtual bool    CanBeDamagedBy( int meansOfDeath );

	protected:
		virtual qboolean  _needThisArmor( Sentient *sentient );
		virtual void      _pickupArmor( Sentient *sentient );
	
	};

inline void Armor::Archive( Archiver &arc )
{
	Item::Archive( arc );
}

//
// Class Name: AdaptiveArmor
// Base Class: ArmorEntity
//
// Description: Armor that adapts to MOD's
//
class AdaptiveArmor : public Armor
	{
	public:
		CLASS_PROTOTYPE( AdaptiveArmor );

			AdaptiveArmor();
			~AdaptiveArmor();

			float  ResolveDamage          ( float damage , int meansOfDeath , const Vector &direction , const Vector &position , Entity *attacker );
			void   SetAdaptionFX          ( Event *ev         );
			void   SetAdaptionFX          ( const str &fxName );  
			bool   CanBeDamagedBy		  ( int meansOfDeath  );
			void   Archive                ( Archiver &arc     );

			void   SetFXLife			  ( Event *ev		  );
						
			static void ClearAdaptionList ();

			//Game Var Support Functions			
			void AddGameVar( const str &name , float value );
			void UpdateGameVar( const str &name , float value );			
			void LoadAdaptionData( const str &name );
			void LoadDataFromGameVars( Event *ev );
			void LoadDataFromGameVars();



			

	protected:
			void	 _AddAdaption             ( int MeansOfDeath , float damage                  );
			void     _UpdateAdaption          ( int MeansOfDeath , float damage                  );
			qboolean _AdaptionInList          ( int MeansOfDeath                                 );
			float    _GetDamageTotal          ( int MeansOfDeath                                 );
			void     _PlayAdaptionFX          ( const Vector &direction , const Vector &position );
			void     _AddMODToCannotAdaptList ( int MOD                                          );
			qboolean _CanAdaptTo              ( int MOD                                          );
			void     _changeFX();


	private:
		static Container<MODHitCounter*> _AdaptionList;
		static Container<int>            _CannotAdaptToList;
		static Container<str>            _fxList;
		int								 _currentFXIndex;             
		str								 _AdaptionFX;             
		Vector							 _currentFXPosition;
		float							 _fxTime;
		float							 _fxTimeNormal;
		float							 _fxTimeExplosive;
		float							 _adaptionLimit;
             

	};

inline void AdaptiveArmor::Archive( Archiver &arc )
{
	int num;
	MODHitCounter* modHitCounter;
	int i;


	Armor::Archive( arc );

	if ( arc.Loading() )
	{
		ClearAdaptionList();
		_CannotAdaptToList.ClearObjectList();
		_fxList.ClearObjectList();
	}

	if ( arc.Saving() )
	{
		num = _AdaptionList.NumObjects();

		arc.ArchiveInteger( &num );

		for( i = 1 ; i <= num ; i++ )
		{
			modHitCounter = _AdaptionList.ObjectAt( i );

			arc.ArchiveInteger( &modHitCounter->MODIndex );
			arc.ArchiveFloat( &modHitCounter->damage );
			arc.ArchiveBool( &modHitCounter->adapted );
		}

	}
	else
	{
		arc.ArchiveInteger( &num );

		for( i = 1 ; i <= num ; i++ )
		{
			modHitCounter = new MODHitCounter;

			arc.ArchiveInteger( &modHitCounter->MODIndex );
			arc.ArchiveFloat( &modHitCounter->damage );
			arc.ArchiveBool( &modHitCounter->adapted );

			_AdaptionList.AddObject( modHitCounter );
		}
	}

	_CannotAdaptToList.Archive( arc );

	_fxList.Archive( arc );

	arc.ArchiveInteger (&_currentFXIndex   );
	arc.ArchiveString  (&_AdaptionFX       );
	arc.ArchiveVector  (&_currentFXPosition);
	arc.ArchiveFloat   (&_fxTime			);
	arc.ArchiveFloat   (&_fxTimeNormal     );
	arc.ArchiveFloat   (&_fxTimeExplosive  );
	arc.ArchiveFloat   (&_adaptionLimit    );
	}

//
// Class Name: BasicArmor
// Base Class: Item
//
// Description: Basic armor
//
class BasicArmor : public Armor
	{
	protected:
		static const float			_maxAmount;
		static const float			_normalMaxAmount;
		static const float			_lossRate;

		static const float			_highRangeCutoffSinglePlayer;
		static const float			_midRangeCutoffSinglePlayer;

		static const float			_highRangeCutoffMultiplayer;
		static const float			_midRangeCutoffMultiplayer;

		static const float			_highRangePercent;
		static const float			_midRangePercent;
		static const float			_lowRangePercent;

		float						_lastAddTime;

		virtual	qboolean		_needThisArmor( Sentient *sentient );
		/* virtual */ void		Think( void );

	public:
      CLASS_PROTOTYPE( BasicArmor );

								BasicArmor();
	                 
		virtual float			ResolveDamage( float damage , int meansOfDeath , const Vector &direction , const Vector &position , Entity *attacker );
		virtual bool			CanBeDamagedBy( int meansOfDeath );
		/* virtual */ void		Add( int num );
		/* virtual */ void		cacheStrings( void );

		void					Archive ( Archiver &arc );
	};

inline void BasicArmor::Archive( Archiver &arc )
{
	Armor::Archive( arc );	

	arc.ArchiveFloat( &_lastAddTime );
}

//
// Class Name: ShieldArmor
// Base Class: Item
//
// Description: Shield armor that can take a specified amount of damage
//
class ShieldArmor : public Armor
	{
	public:
		CLASS_PROTOTYPE ( ShieldArmor );

						ShieldArmor();

		void			SetActiveStatus( Event *ev );
		void			SetMultiplier( Event *ev );
		void			AddMODToNoProtectionList( Event *ev );
		void			AddMODToNoProtectionList( const str &MODName );
		void			SetShieldDirection( Event *ev );
		void			SetShieldDirection( float minAngle, float maxAngle );
		bool			CanBeUsed();
		virtual float	ResolveDamage( float damage, int meansOfDeath, const Vector &direction, const Vector &position , Entity *attacker );
		virtual bool	CanBeDamagedBy ( int meansOfDeath  );
		void			Archive ( Archiver &arc     );

	private:
		bool			_active;
		float			_multiplier;
		float			_directionMin;
		float			_directionMax;
		bool			_useDirection;
		Container<unsigned int> _noProtectionList;

	};

inline void ShieldArmor::Archive( Archiver &arc )
{
	Armor::Archive( arc );
	
	arc.ArchiveBool( &_active );
	arc.ArchiveFloat( &_multiplier );
	arc.ArchiveFloat( &_directionMin);
	arc.ArchiveFloat( &_directionMax );
	arc.ArchiveBool( &_useDirection );

	int num , i;
	
	if ( arc.Saving() )
	{
		num = _noProtectionList.NumObjects();

		arc.ArchiveInteger( &num );

		for( i = 1 ; i <= num ; i++ )
		{
			arc.ArchiveUnsigned ( &_noProtectionList.ObjectAt( i ) );			
		}

	}
	else
	{
		arc.ArchiveInteger( &num );

		for( i = 1 ; i <= num ; i++ )
		{
			unsigned int theMod;
			arc.ArchiveUnsigned( &theMod );
			_noProtectionList.AddObject( theMod );
		}
	}
	
}

typedef SafePtr<Armor> ArmorPtr;

#endif /* armor.h */
