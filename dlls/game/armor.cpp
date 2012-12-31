//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/armor.cpp                                 $
// $Revision:: 63                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:35p                                                  $
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

#include "_pch_cpp.h"
#include "armor.h"
#include "player.h"
#include "mp_manager.hpp"

extern cvar_t *g_armoradaptionlimit;
extern Event EV_SetOriginEveryFrame;

//-------------------------------------------------------------------------
// Events
//-------------------------------------------------------------------------
Event EV_Armor_SetAdaptionFX
(
	"setadaptionfx",
	EV_DEFAULT,
	"s",
	"adaption_fx",
	"set the adaption fx model"
);
Event EV_Armor_SetFXLife
(
	"setFXlife",
	EV_DEFAULT,
	"ff",
	"Normal_Time Explosive_Time",
	"Sets the FX Time"
);
Event EV_Armor_SetActiveStatus
(
	"setarmoractivestatus",
	EV_DEFAULT,
	"b",
	"active",
	"sets the active status"
);
Event EV_Armor_SetMultiplier
(
	"setarmormultiplier",
	EV_DEFAULT,
	"f",
	"multiplier",
	"sets the damage multiplier for the armor"
);
Event EV_Armor_AddToNoProtectionList
(
	"noprotection",
	EV_DEFAULT,
	"s",
	"MOD",
	"Adds a MOD to the No Protection List"
);
Event EV_Armor_UseArmorDirection
(
	"usearmordirection",
	EV_DEFAULT,
	"ff",
	"directionAngleMin directionAngleMax",
	"Only allows armor to work if attack is within the specifed yaw range"
);

Event EV_Armor_LoadAdaptionDataFromGameVars
(
	"loadadaptiondata",
	EV_DEFAULT,
	NULL,
	NULL,
	"Used to control timing on adaption information loads"
);


//-------------------------------------------------------------------------
// Armor Implementation
//-------------------------------------------------------------------------
CLASS_DECLARATION( Item, Armor, NULL )
{
	{ NULL, NULL }
};


//
// Name:        Armor()
// Class:       Armor
//
// Description: Constructor
//
// Parameters:  None
//
// Returns:     None
//
Armor::Armor()
{
	// all data will be setup by the archive function
	if ( LoadingSavegame )
		return;
	
	if ( multiplayerManager.checkFlag( MP_FLAG_NO_ARMOR ) )
	{
		PostEvent( EV_Remove, EV_REMOVE );
		return;
	}
	
	setName( "UnknownArmor" );
	amount  = 0;	
	SetMax ( 200 );

	_mpItemType = MP_ITEM_TYPE_ARMOR;
}



//
// Name:        ~Armor()
// Class:        Armor
//
// Description:  Destructor
//
// Parameters:   None
//
// Returns:      None
//
Armor::~Armor()
{
	
}

//
// Name:        ResolveDamage() 
// Class:       Armor
//
// Description: Resolves the damage.
//
// Parameters:  float damage -- The initial damage.
//
// Returns:     float newDamage -- The new damage after armor factor is taken into account
//
float Armor::ResolveDamage( float damage , int meansOfDeath , const Vector &direction , const Vector &position , Entity *attacker )
{
	Q_UNUSED(attacker); Q_UNUSED(position); Q_UNUSED(direction); Q_UNUSED(meansOfDeath); Q_UNUSED(damage);

	gi.Error( ERR_FATAL, "Armor::ResolveDamage -- Do Not Use Armor Base Class -- You MUST use a subclassed" );
	return 0.0f;
}

//--------------------------------------------------------------
// Name:		CanBeDamagedBy()
// Class:		Armor
//
// Description:	Checks if this armor will aborb 100% of the damage
//				for the specified Means Of Death
//
// Parameters:	int meansOfDeath
//
// Returns:		true or false
//--------------------------------------------------------------
bool Armor::CanBeDamagedBy( int meansOfDeath )
{
	Q_UNUSED(meansOfDeath);

	gi.Error( ERR_FATAL, "Armor::CanBeDamagedBy -- Do Not Use Armor Base Class -- You MUST use a subclassed" );
	return true;
}

//
// Name:        ItemPickup()
// Class:       Armor
//
// Description: Handles Sentients Picking up armor... In this case, however,
//              Only Actors are allowed to actually pick up armor
//
// Parameters:  Entity *other - The entity doing the picking up
//              qboolean add_to_inventory - Here to make parameters match
//
// Returns:     NULL -- This function doesn't actually return the armor, instead
//              it has the entity picking it up process and armor event
//
Item *Armor::ItemPickup( Entity *other, qboolean add_to_inventory, qboolean )
{
	Sentient *sent;
	str      realname;

	Q_UNUSED(add_to_inventory);
	
	// For right now, we only want players to pick up armor
	if ( !other->isSubclassOf( Player ) )
		return NULL;

	if ( !Pickupable( other ) )
		return NULL;
	
	sent = (Sentient*)other;
	
	// Check if we NEED to pick up the armor, if the armor we are about to pick up is
	// less than the armor we have, we don't want to pick it up
	if ( !_needThisArmor(sent) )
		return NULL;
	
	// Handle the actual "Picking Up"
	_pickupArmor(sent);
	
	// Give the Sentient the Armor
	Event *armorEvent;
	armorEvent = new Event(EV_Sentient_GiveArmor);
	armorEvent->AddString( item_name.c_str() );
	armorEvent->AddFloat( amount );
	armorEvent->AddInteger( true );
	
	sent->ProcessEvent(armorEvent);	
	
	return NULL; // This doesn't create any items
}



//
// Name:        _needThisArmor()
// Class:       Armor
//
// Description: Checks if the sentient needs to pick up the
//              armor or not
//
// Parameters:  Sentient *sentient -- The sentient to check
//
// Returns:     true or false
//
qboolean Armor::_needThisArmor( Sentient *sentient )
{
	int currentArmorValue = sentient->GetArmorValue();
	
	if( currentArmorValue >= amount )
		return false;
	else
		return true;
}


//
// Name:        _pickupArmor()
// Class:       Armor
//
// Description: Takes care of "Picking Up" the armor, by playing the
//              appropriate sounds, removing the model...
//
// Parameters:  None
// 
// Returns:     None
//
void Armor::_pickupArmor( Sentient *sentient)
{
	str      realname;
	
	// Play pickup sound
	realname = GetRandomAlias( "snd_pickup" );
	if ( realname.length() > 1 )
		sentient->Sound( realname, CHAN_ITEM );
	
	// Cancel some events
	CancelEventsOfType( EV_Item_DropToFloor );
	CancelEventsOfType( EV_Item_Respawn );
	CancelEventsOfType( EV_FadeOut );
	
	// Hide the model
	setSolidType( SOLID_NOT );

	if ( _missingSkin )
	{
		ChangeSkin( _missingSkin, true );
	}
	else
	{
		hideModel();
	}
	
	// Respawn?
	if ( !Respawnable() )
		PostEvent( EV_Remove, FRAMETIME );
	else
		PostEvent( EV_Item_Respawn, RespawnTime() );
	
	// fire off any pickup_thread's
	if ( pickup_thread.length() )
		ExecuteThread( pickup_thread );	
}


//-------------------------------------------------------------------------
// AdaptiveArmor Implementation
//-------------------------------------------------------------------------
CLASS_DECLARATION( Armor, AdaptiveArmor, NULL )
{
	{ &EV_Armor_SetAdaptionFX,           		&AdaptiveArmor::SetAdaptionFX 		},
	{ &EV_Armor_LoadAdaptionDataFromGameVars, 	&AdaptiveArmor::LoadDataFromGameVars},
	{ &EV_Armor_SetFXLife,				 		&AdaptiveArmor::SetFXLife			},
	{ NULL, NULL }
};


//
// Adaptive Armor is Adaption And Cannot Adapt is Global
//
Container<MODHitCounter*> AdaptiveArmor::_AdaptionList;
Container<int> AdaptiveArmor::_CannotAdaptToList;
Container<str> AdaptiveArmor::_fxList;

//
// Name:        AdaptiveArmor()
// Class:       AdaptiveArmor
//
// Description: Constructor
//
// Parameters:  None
//
// Returns:     None
//
AdaptiveArmor::AdaptiveArmor()
{
	amount  = 0;	
	SetMax ( 200 );	
	
	// Add Adaptions to list
	_fxList.AddObject( "fx/fx-borg-sheild-01.tik" );
	_fxList.AddObject( "fx/fx-borg-sheild-02.tik" );
	_fxList.AddObject( "fx/fx-borg-sheild-03.tik" );
	
	// Set our current Adaption
	_AdaptionFX = "fx/fx-borg-sheild-01.tik";
	
	// Set our index
	_currentFXIndex = 1;
	_fxTimeNormal = 0.15;
	_fxTimeExplosive = 1.0;
	
	_AddMODToCannotAdaptList( MOD_IMOD_PRIMARY);   
	_AddMODToCannotAdaptList( MOD_IMOD_SECONDARY);
	_AddMODToCannotAdaptList( MOD_SUICIDE );

	if ( !LoadingSavegame )
	{
		//Try and pull data from the gameVars
		Event *loadEvent = new Event ( EV_Armor_LoadAdaptionDataFromGameVars );

		PostEvent( loadEvent, 3.0 );
		//LoadDataFromGameVars();
	}
}

//
// Name:        ~AdaptiveArmor()
// Class:       AdaptiveArmor
//
// Description: Destructor
//
// Parameters:  None
//
// Returns:     None
//
AdaptiveArmor::~AdaptiveArmor()
{
	
	
}


//
// Name:        ResolveDamage()
// Class:       AdaptiveArmor
//
// Description: Resolves the damage
//
// Parameters:  float damage -- The initial damage.
//
// Returns:     float newDamage -- The new damage after armor factor is taken into account
//
float AdaptiveArmor::ResolveDamage( float damage , int meansOfDeath , const Vector &direction , const Vector &position , Entity *attacker )
{
	int DamageTotal;
	
	if (!_CanAdaptTo( meansOfDeath ) )
		return damage;
	
	if ( attacker->isSubclassOf( Player ) )
	{
		if ( !_AdaptionInList( meansOfDeath ) )
			_AddAdaption( meansOfDeath , damage );
		else
			_UpdateAdaption( meansOfDeath , damage );
	}
	
	DamageTotal = (int)_GetDamageTotal( meansOfDeath );
	
	_adaptionLimit = g_armoradaptionlimit->value;
	if ( DamageTotal > _adaptionLimit )
	{		
		if ( meansOfDeath == MOD_EXPLOSION || meansOfDeath == MOD_VAPORIZE_COMP )
			_fxTime = _fxTimeExplosive;
		else
			_fxTime = _fxTimeNormal;
		
		_PlayAdaptionFX( direction , position );
		return 0.0f;
	}
		
	return damage;
}


void AdaptiveArmor::SetAdaptionFX( Event *ev )
{
	SetAdaptionFX(ev->GetString( 1 ) );
}

void AdaptiveArmor::SetAdaptionFX( const str &fxName )
{
	_AdaptionFX = fxName;
}

//
// Name:        _AddAdaption()
// Class:       AdaptiveArmor
//
// Description: Adds and adaption to the adaption list
//
// Parameters:  int MOD - The Means Of Death index
//
// Returns:     None
//
void AdaptiveArmor::_AddAdaption( int MeansOfDeath , float damage)
{
	MODHitCounter *adaption = NULL;
	adaption = new MODHitCounter;
	
	//We don't want to be able to adapt to all MOD
	if ( _AdaptionList.NumObjects() > 2 )
		return;

	//Double Check we're not adding the same MOD 
	if ( _AdaptionInList( MeansOfDeath ) )
		return;

	if ( adaption )
	{
		adaption->MODIndex = MeansOfDeath;
		adaption->damage = damage + .1; // Have to add just a bit incase damage = 0
		adaption->adapted = false;

		str MODName = MOD_NumToName( MeansOfDeath );
		AddGameVar ( MODName , adaption->damage );
		_AdaptionList.AddObject( adaption );
	}		
}


//
// Name:        _AdaptionInList()
// Class:       AdaptiveArmor
//
// Description: Checks if an adaption for the MOD is in the list
//
// Parameters:  int MOD -- the Means Of Death index
//
// Returns:     true or false
//
qboolean AdaptiveArmor::_AdaptionInList( int MeansOfDeath )
{
	int i;
	MODHitCounter *adaption;
	
	for( i=_AdaptionList.NumObjects(); i>0; i-- )
	{
		adaption = _AdaptionList.ObjectAt( i );
		if ( adaption->MODIndex == MeansOfDeath )
			return true;
	}
	
	return false;
}


//
// Name:        _UpdateAdaption()
// Class:       AdaptiveArmor
//
// Description: Updates a particular adaption in the adaption list
//
// Parameters:  int MOD -- the Means Of Death index
//              int count -- the number to add
//
// Returns:     None
//
void AdaptiveArmor::_UpdateAdaption( int MeansOfDeath , float damage )
{
	int i;
	MODHitCounter *adaption;
		
	for( i=_AdaptionList.NumObjects(); i>0; i-- )
	{
		adaption = _AdaptionList.ObjectAt( i );
		if ( adaption->MODIndex == MeansOfDeath )
		{
			adaption->damage += damage;

			str MODName = MOD_NumToName( MeansOfDeath );
			UpdateGameVar ( MODName , adaption->damage );

			if ( (adaption->damage >= g_armoradaptionlimit->value) && !adaption->adapted )
			{
				adaption->adapted = true;				

				Actor *act;
				for ( int i=1; i<=ActiveList.NumObjects(); i++ )
				{
					act = ActiveList.ObjectAt(i);
					if ( !act )
						continue;
					
					act->InContext( "enemyadapted" , 0 );					
				}
				
			}
			return;
		}
	}		
}


//
// Name:         _GetDamageTotal()
// Class:        AdaptiveArmor
//
// Description:  Retrieves the number of times shot for a 
//               particular Means Of Death
//
// Parameters:   int MOD -- Means Of Death Index
//
// Returns:      int count
//
float AdaptiveArmor::_GetDamageTotal( int MeansOfDeath )
{
	int i;
	MODHitCounter *adaption;
	
	for( i=_AdaptionList.NumObjects(); i>0; i-- )
	{
		adaption = _AdaptionList.ObjectAt( i );
		if ( adaption->MODIndex == MeansOfDeath )
		{
			if ( adaption->damage > 0 )
				return adaption->damage;				
		}
		
	}		
	
	return 0.0f;
}

void AdaptiveArmor::_PlayAdaptionFX(const Vector &direction , const Vector &position )
{
	Vector dir;
	Vector newDir;
	Vector fxOrigin;
	Vector PosDelta;
	Entity *theFX;
	
	
	dir = direction;
	dir *= -1;
	dir.normalize();
	
	fxOrigin  = dir * 15;
	fxOrigin += position;
	
	dir = dir.toAngles();
	
	PosDelta = _currentFXPosition - fxOrigin;
	
	if ( PosDelta.length() > FX_CHANGE_DELTA )
	{
		_changeFX();
		_currentFXPosition = fxOrigin;
	}
		
	theFX = SpawnEffect(_AdaptionFX, fxOrigin , dir , _fxTime );	
	theFX->bind( owner );

	theFX->PostEvent( EV_SetOriginEveryFrame , FRAMETIME );	
}

qboolean AdaptiveArmor::_CanAdaptTo( int MOD )
{
	int checkMOD;
	int i;
	
	for( i=_CannotAdaptToList.NumObjects(); i>0; i-- )
	{
		checkMOD = _CannotAdaptToList.ObjectAt( i );
		if ( checkMOD == MOD )
			return false;
	}
	
	return true;
}

void AdaptiveArmor::_AddMODToCannotAdaptList( int MOD )
{
	_CannotAdaptToList.AddObject( MOD );
}

void AdaptiveArmor::_changeFX()
{
	int MaxNumber;
	MaxNumber = _fxList.NumObjects();
	
	if ( MaxNumber == 0 )
		return;

	_currentFXIndex++;
	
	if ( _currentFXIndex > MaxNumber )
		_currentFXIndex = 1;
	
	_AdaptionFX = _fxList.ObjectAt( _currentFXIndex );
	
}

void AdaptiveArmor::ClearAdaptionList()
{
	
	int i;
	MODHitCounter *adaption;
	
	for( i=_AdaptionList.NumObjects(); i>0; i-- )
	{
		adaption = _AdaptionList.ObjectAt( i );
		delete adaption;
		adaption = NULL;
	}
	
    _AdaptionList.FreeObjectList();	
	
	
}

//--------------------------------------------------------------
// Name:		CanBeDamagedBy()
// Class:		AdaptiveArmor
//
// Description:	Checks if this armor will aborb 100% of the damage
//				for the specified Means Of Death
//
// Parameters:	int meansOfDeath
//
// Returns:		true or false
//--------------------------------------------------------------
bool AdaptiveArmor::CanBeDamagedBy( int meansOfDeath )
{
	float DamageTotal;
	
	if ( !_AdaptionInList( meansOfDeath ) )
		return true;
	
	DamageTotal = _GetDamageTotal( meansOfDeath );
	if ( DamageTotal > ADAPTION_LIMIT )
		return false;
	
	return true;
}



void AdaptiveArmor::AddGameVar( const str &name , float value )
{
	ScriptVariable *script_var;
	if ( gameVars.VariableExists( name.c_str() ) )
		return;

	script_var = new ScriptVariable;
	script_var->setName( name.c_str() );
	script_var->setFloatValue( value );
	gameVars.AddVariable( script_var );	
}

void AdaptiveArmor::UpdateGameVar( const str &name , float value )
{
	ScriptVariable *script_var;
	float currentValue;

	if ( !gameVars.VariableExists( name.c_str() ) )
		return;

	script_var = gameVars.GetVariable( name.c_str() );
	currentValue = script_var->floatValue();
	currentValue = currentValue + value;
	script_var->setFloatValue( currentValue );	
}

void AdaptiveArmor::LoadAdaptionData( const str &name )
{
	ScriptVariable *script_var;
	float lastValue;
	int i;
	unsigned int theMOD;
	MODHitCounter *adaption;

	theMOD = MOD_NameToNum( name );

	if ( gameVars.VariableExists( name.c_str() ) )
		{
		script_var = gameVars.GetVariable( name.c_str() );
		lastValue = script_var->floatValue();

		_AddAdaption( theMOD , lastValue );


		if ( lastValue >= g_armoradaptionlimit->value )
			{
			for( i=_AdaptionList.NumObjects(); i>0; i-- )
			{
			adaption = _AdaptionList.ObjectAt( i );
			if ( adaption->MODIndex == theMOD )
				{
				adaption->adapted = true;
				}
			}

		}	

	}		
}

void AdaptiveArmor::LoadDataFromGameVars( Event * )
{
	LoadDataFromGameVars();
}

void AdaptiveArmor::LoadDataFromGameVars()
{
	//Sigh, I HATE having to do this, but since we evidently HAVE to have
	//the adaptions carry across levels, here we go... 
	LoadAdaptionData( "phaser"			);
	LoadAdaptionData( "vaporize"		);
	LoadAdaptionData( "comp_rifle"		);
	LoadAdaptionData( "vaporize_comp"	);
}

void AdaptiveArmor::SetFXLife( Event *ev )
{
	_fxTimeNormal = ev->GetFloat( 1 );
	_fxTimeExplosive = ev->GetFloat( 2 );
}

//-------------------------------------------------------------------------
// BasicArmor Implementation
//-------------------------------------------------------------------------

const float	BasicArmor::_maxAmount = 200.0f;
const float	BasicArmor::_normalMaxAmount = 100.0f;
const float	BasicArmor::_lossRate = 1.0f;

const float	BasicArmor::_highRangeCutoffSinglePlayer = 66.0f;
const float	BasicArmor::_midRangeCutoffSinglePlayer = 33.0f;

const float	BasicArmor::_highRangeCutoffMultiplayer = 100.0f;
const float	BasicArmor::_midRangeCutoffMultiplayer = 50.0f;

const float	BasicArmor::_highRangePercent = 0.75f;
const float	BasicArmor::_midRangePercent = 0.50f;
const float	BasicArmor::_lowRangePercent = 0.25f;



CLASS_DECLARATION( Armor, BasicArmor, NULL )
{
	{ NULL, NULL }
};

BasicArmor::BasicArmor()
{
	maximum_amount = _maxAmount;
	setName( "BasicArmor" );
	turnThinkOn();

	_lastAddTime = 0.0f;
}

//
// Name:        ResolveDamage() 
// Class:       BasicAmor
//
// Description: Resolves the damage.
//
// Parameters:  float damage -- The initial damage.
//
// Returns:     float newDamage -- The new damage after armor factor is taken into account
//
float BasicArmor::ResolveDamage( float damage , int meansOfDeath , const Vector &direction , const Vector &position , Entity *attacker )
{
	float tempDamage;
	float damageAbsorbed;
	float percent;
	float highRangeCutoff;
	float midRangeCutoff;

	Q_UNUSED(attacker); Q_UNUSED(position); Q_UNUSED(direction);

	if ( ( meansOfDeath == MOD_ARMOR_PIERCING ) || ( meansOfDeath == MOD_IMOD_PRIMARY ) || 
		 ( meansOfDeath == MOD_IMOD_SECONDARY ) || ( meansOfDeath == MOD_DEATH_QUAD ) )
		return damage;

	// Determine which cutoff values to use

	if ( multiplayerManager.inMultiplayer() )
	{
		highRangeCutoff = _highRangeCutoffMultiplayer;
		midRangeCutoff  = _midRangeCutoffMultiplayer;
	}
	else
	{
		highRangeCutoff = _highRangeCutoffSinglePlayer;
		midRangeCutoff  = _midRangeCutoffSinglePlayer;
	}

	// Figure out amount to absorb
	
	if ( amount > highRangeCutoff )
		percent = _highRangePercent;
	else if ( amount > midRangeCutoff )
		percent = _midRangePercent;
	else
		percent = _lowRangePercent;

	tempDamage = damage * percent;
	
	if ( amount > tempDamage )
		damageAbsorbed = tempDamage;
	else
		damageAbsorbed = amount;
	
	// Damage the armor
	
	amount -= damageAbsorbed;
	if ( amount < 0.0 )
		amount = 0.0;
	
	// Return damage left
	
	return damage - damageAbsorbed;
}

void BasicArmor::Think( void )
{
	if ( ( level.time > _lastAddTime + 1.0f ) && ( amount > _normalMaxAmount ) )
	{
		amount -= level.frametime * _lossRate;

		if ( amount < _normalMaxAmount )
		{
			amount = _normalMaxAmount;
		}
	}
}

void BasicArmor::Add( int num )
{
	Armor::Add( num );

	_lastAddTime = level.time;
}

//--------------------------------------------------------------
// Name:		CanBeDamagedBy()
// Class:		BasicArmor
//
// Description:	Checks if this armor will aborb 100% of the damage
//				for the specified Means Of Death
//
// Parameters:	int meansOfDeath
//
// Returns:		true or false
//--------------------------------------------------------------
bool BasicArmor::CanBeDamagedBy( int meansOfDeath )
{
	Q_UNUSED(meansOfDeath);

	return true;
}

//--------------------------------------------------------------
// Name:		_needThisArmor
// Class:		BasicArmor
//
// Description:	Checks to see if we need more armor
//
// Parameters:	Sentient *sentient				- sentient that wants to know if it can have more armor
//
// Returns:		true or false
//--------------------------------------------------------------

qboolean BasicArmor::_needThisArmor( Sentient *sentient )
{
	int currentArmorValue = sentient->GetArmorValue();

  	if( currentArmorValue < _maxAmount )
		return true;
	else
		return false;
}

void BasicArmor::cacheStrings( void )
{
	G_FindConfigstringIndex( va( "$$PickedUp$$ %d $$Item-Armor$$", (int)amount ), CS_GENERAL_STRINGS, MAX_GENERAL_STRINGS, true );
}

//-------------------------------------------------------------------------
// ShieldArmor Implementation
//-------------------------------------------------------------------------
CLASS_DECLARATION( Armor, ShieldArmor, NULL )
{
	{ &EV_Armor_SetActiveStatus,			&ShieldArmor::SetActiveStatus 			},
	{ &EV_Armor_SetMultiplier,			 	&ShieldArmor::SetMultiplier				},
	{ &EV_Armor_AddToNoProtectionList,	  	&ShieldArmor::AddMODToNoProtectionList	},
	{ &EV_Armor_UseArmorDirection,		   	&ShieldArmor::SetShieldDirection		},

	{ NULL, NULL }
};

ShieldArmor::ShieldArmor()
{
	maximum_amount = 100.0f;
	//amount = 100.0f;
	setName( "ShieldArmor" );
	_active = true;
	_multiplier = 0.0f;
	_directionMin = -90.0;
	_directionMax = 90.0;
	_useDirection = false;

}

void ShieldArmor::SetActiveStatus( Event *ev )
{
	_active = ev->GetBoolean( 1 );
}

void ShieldArmor::SetMultiplier( Event *ev )
{
	_multiplier = ev->GetFloat( 1 );
}

bool ShieldArmor::CanBeUsed()
{
	if ( amount > 0 )
		return true;

	return false;
}

//
// Name:        ResolveDamage() 
// Class:       BasicAmor
//
// Description: Resolves the damage.
//
// Parameters:  float damage -- The initial damage.
//
// Returns:     float newDamage -- The new damage after armor factor is taken into account
//
float ShieldArmor::ResolveDamage( float damage , int meansOfDeath , const Vector &direction , const Vector &position , Entity *attacker )
{
	Q_UNUSED(attacker);

	//Shield Armor basically acts as a "life meter" for a shield.  If the armor is actor 
	//and he have an armor value greater than 0, then we take 0 damage.  The as soon as the 
	//amount is less than 0, then we start taking full damage.
	if ( !_active ) return damage;
	
	int inNoProtectionList = 0;

	inNoProtectionList = _noProtectionList.IndexOfObject( meansOfDeath );

	if ( inNoProtectionList )
		return damage;

	if ( meansOfDeath == MOD_ARMOR_PIERCING )
		return damage;
	
	if ( amount <= 0 )
	{
		amount = 0.0f;
		return damage;
	}
	

	if ( _useDirection )
		{
		Vector directionAngle = direction.toAngles();
		float directionYaw = AngleNormalize180( directionAngle[YAW] );
		if ( (directionYaw > _directionMax) || (directionYaw < _directionMin) )
			{
			return damage;
			}
		}

	amount -= damage;
	
	if ( amount < 0 )
		amount = 0.0f;
	
	if ( amount > 0 )
	{
		// We blocked so spawn an effect
		
		str modelName;
		Vector effectAngles;
		
		effectAngles = direction;
		effectAngles.toAngles();
		
		modelName = "ReflectionModel-";
		modelName += MOD_NumToName( meansOfDeath );
		modelName += ".gdb";
		
		SpawnEffect( modelName, position, vec_zero, 1.0f );
		
		return (damage * _multiplier);
	}		
	
	return damage;	
}

//--------------------------------------------------------------
// Name:		CanBeDamagedBy()
// Class:		BasicArmor
//
// Description:	Checks if this armor will aborb 100% of the damage
//				for the specified Means Of Death
//
// Parameters:	int meansOfDeath
//
// Returns:		true or false
//--------------------------------------------------------------
bool ShieldArmor::CanBeDamagedBy( int meansOfDeath )
{
	Q_UNUSED(meansOfDeath);

	return true;
}


void ShieldArmor::AddMODToNoProtectionList( Event *ev )
{
	str MODName = ev->GetString( 1 );
	AddMODToNoProtectionList( MODName );
}

void ShieldArmor::AddMODToNoProtectionList(const str &MODName )
{
	unsigned int theMOD = MOD_NameToNum( MODName.c_str() );

	_noProtectionList.AddUniqueObject( theMOD );	
}

void ShieldArmor::SetShieldDirection( Event *ev )
{
	SetShieldDirection( ev->GetFloat(1) , ev->GetFloat(2) );
}

void ShieldArmor::SetShieldDirection( float minAngle, float maxAngle )
{
	_useDirection = true;
	_directionMin = minAngle;
	_directionMax = maxAngle;
}

