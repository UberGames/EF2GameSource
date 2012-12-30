//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/teammateroster.cpp                            $
// $Revision:: 10                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/10/02 1:13p                                                 $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:

#include "teammateroster.hpp"
#include "GameplayManager.h"

TeammateRoster* TeammateRoster::_instance = 0;

//-----------------------------------------------------
//
// Name:		TeammateRoster
// Class:		TeammateRoster
//
// Description:	Initializes data members
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
TeammateRoster::TeammateRoster()
{

	GameplayManager* gpm = GameplayManager::getTheGameplayManager();
	if(gpm == 0)
		return;
		
	_healthyShader		= gpm->getStringValue("TeammateData", "HealthyShader");
	_injuredShader		= gpm->getStringValue("TeammateData", "InjuredShader");
	_criticalShader		= gpm->getStringValue("TeammateData", "CriticalShader");
	_defaultShader		= gpm->getStringValue("TeammateData", "DefaultShader");

	_healthyAnimation	= gpm->getStringValue("TeammateData", "HealthyAnimation");
	_injuredAnimation	= gpm->getStringValue("TeammateData", "InjuredAnimation");
	_criticalAnimation	= gpm->getStringValue("TeammateData", "CriticalAnimation");

	for(int i = 0; i < MAX_TEAMMATES; i++)
	{
		_teammateList[i].active = false;
		_teammateList[i].entNum = ENTITYNUM_NONE;
	}
}


//-----------------------------------------------------
//
// Name:		~TeammateRoster
// Class:		TeammateRoster
//
// Description:	Uninitializes data members
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
TeammateRoster::~TeammateRoster()
{
	for(int i = 0; i < MAX_TEAMMATES; i++)
	{
		removeTeammate(i);
	}
}


//-----------------------------------------------------
//
// Name:		createInstance
// Class:		TeammateRoster
//
// Description:	Creates an instance of the teammate roster system
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void TeammateRoster::createInstance(void)
{
	if(_instance == 0)
		_instance = new TeammateRoster;
}


//-----------------------------------------------------
//
// Name:		deleteInstance
// Class:		TeammateRoster
//
// Description:	Deletes the instance of the teammate roster.
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void TeammateRoster::deleteInstance(void)
{
	delete _instance;
	_instance = 0;
}


//-----------------------------------------------------
//
// Name:		addTeammate
// Class:		TeammateRoster
//
// Description:	Adds a teammate to the roster. If a teammate already exists
//				in that current position, the new teammate replaces the old
//				teammate.
//
// Parameters:	entity - the entity to add as a teammate.
//				index  - the position on the roster to add the teammate to. 
//
// Returns:		
//-----------------------------------------------------
void TeammateRoster::addTeammate( const Entity* entity, int index)
{
	if(index > MAX_TEAMMATES - 1 || index < 0 )
	{
		return;
	}

	if(entity == 0)
		return;


	TeammateData& teammateData = _teammateList[index];

	teammateData.entNum				 = entity->edict->s.number;
	teammateData.injuredHealthLevel  = entity->max_health - (entity->max_health  * 0.33f);	//injured health level is below 2/3 of their health
	teammateData.criticalHealthLevel = entity->max_health - (entity->max_health  * 0.66f);	//critical health level is below 1/3 of their health

	//get the archetype
	teammateData.archeType = entity->getArchetype();
	teammateData.active = true;
}


//-----------------------------------------------------
//
// Name:		removeTeammate
// Class:		TeammateRoster
//
// Description:	Removes a teammate from the list.
//
// Parameters:	index - the index of the teammate data.
//
// Returns:		None
//-----------------------------------------------------
void TeammateRoster::removeTeammate(int index)
{
	if(index > MAX_TEAMMATES - 1 || index < 0 )
		return;

	TeammateData& teammateData = _teammateList[index];

	teammateData.entNum					= ENTITYNUM_NONE;
	teammateData.criticalHealthLevel	= 0;
	teammateData.injuredHealthLevel		= 0;
	teammateData.active					= false;

	removeTeammateDataFromDatabase(index);
}


//-----------------------------------------------------
//
// Name:		removeTeammateDataFromDatabase
// Class:		TeammateRoster
//
// Description:	Removes the teammate data from the database
//
// Parameters:	index - the index of the teammate data
//
// Returns:		None
//-----------------------------------------------------
void TeammateRoster::removeTeammateDataFromDatabase(int index)
{
	GameplayManager* gpm = GameplayManager::getTheGameplayManager();
	if(gpm == 0)
		return;

	str objectName = "Teammate";
	objectName += (index + 1);

	gpm->setStringValue(objectName,"Archetype","EMPTY");
	gpm->setStringValue(objectName, "StatusShader", _defaultShader);
}


//-----------------------------------------------------
//
// Name:		getTeammateStatus
// Class:		TeammateRoster
//
// Description:	Retrieves the teammates status based upon their current health.
//
// Parameters:	index - index into the list for the teammate data.
//
// Returns:		TeammateStatus
//-----------------------------------------------------
TeammateStatus TeammateRoster::getTeammateStatus(int index)
{
	if(index > MAX_TEAMMATES - 1 || index < 0 )
		return HEALTHY_STATUS;

	TeammateData& teammateData = _teammateList[index];

	// Make sure we are referencing an actual entity

	if ( teammateData.entNum == ENTITYNUM_NONE )
		return HEALTHY_STATUS;

	gentity_t* gentity = &g_entities[teammateData.entNum];

	if(gentity == 0 || gentity->entity == 0)
		return HEALTHY_STATUS;

	if(gentity->entity->health > teammateData.injuredHealthLevel)
	{
		return HEALTHY_STATUS;
	}
	else if(gentity->entity->health <= teammateData.injuredHealthLevel &&
			gentity->entity->health >  teammateData.criticalHealthLevel)
	{
		return INJURED_STATUS;
	}
	else if(gentity->entity->health <= teammateData.criticalHealthLevel)
	{
		return CRITICAL_STATUS;
	}

	return HEALTHY_STATUS;
}

//-----------------------------------------------------
//
// Name:		update
// Class:		TeammateRoster
//
// Description:	Updates the teammates status via the database.
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void TeammateRoster::update(void)
{
	GameplayManager*	gpm;
	str					objectName;
	TeammateStatus		status;
	str					shader;
	str					animation;


	gpm = GameplayManager::getTheGameplayManager();
	if(gpm == 0)
		return;

	for(int i = 0; i < MAX_TEAMMATES; i++)
	{

		if(_teammateList[i].active == false)
			continue;

		//update the teammates status.
		objectName = "Teammate";
		objectName += (i + 1);

		status = getTeammateStatus(i);

		switch (status)
		{
			case HEALTHY_STATUS:
				shader		= _healthyShader;
				animation	= _healthyAnimation;
				break;

			case INJURED_STATUS:
				shader		= _injuredShader;
				animation	= _injuredAnimation;
				break;

			case CRITICAL_STATUS:
				shader		= _criticalShader;
				animation	= _criticalAnimation;
				break;
		}

	
		gpm->setStringValue(objectName, "Archetype", _teammateList[i].archeType, true);

		str objectRenderName = _teammateList[i].archeType;
		objectRenderName += ".ModelRendering";
		gpm->setStringValue(objectRenderName, "animation", animation, true);
		
		gpm->setStringValue(objectName, "StatusShader", shader, true);
	}
}


//-----------------------------------------------------
//
// Name:		clearTeammates
// Class:		TeammateRoster
//
// Description:	Clears the teammates from the roster.
//
// Parameters:	
//
// Returns:		
//-----------------------------------------------------
void TeammateRoster::clearTeammates( void )
{
	for( int i = 0; i < MAX_TEAMMATES; i++)
	{
		removeTeammate(i);
	}
}