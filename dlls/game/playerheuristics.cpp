//-----------------------------------------------------------------------------
//
//  $Logfile:: /EF2/Code/DLLs/game/playerheuristics.cpp                      $
// $Revision:: 14                                                             $
//   $Author:: Singlis                                                        $
//     $Date:: 9/26/03 2:36p                                                  $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:

#include "_pch_cpp.h"
#include "playerheuristics.h"

static fileHandle_t heuristicFile=NULL;
static str heuristicFileName=NULL;

PlayerHeuristics::PlayerHeuristics()
{
	//Single Player Stats
	shotsFired        = 0;
	shotsHit          = 0;
	numberOfDeaths    = 0;
	enemiesKilled     = 0;
	timeOnLevel       = 0;
	levelStartTime    = 0;
	levelEndTime      = 0;
	playerHealth      = 0.0f;
	lastLevel         = "";
	currentLevel      = "";
	
	shotsFiredInLevel		 = 0;
	shotsHitInLevel			 = 0;
	enemiesKilledInLevel	 = 0;
	
	
	//Multiplayer Stats
	ping              = 0;
	itemsPickedUp     = 0;
	specialEvents     = 0;
	
	
	//Skill Level
	skillLevel        = 0;
	
	//Mission Objectives
	numObjectives				= 0;
	numCompleteObjectives	= 0;
	numFailedObjectives		= 0;
	
}

PlayerHeuristics::PlayerHeuristics( const PlayerHeuristics &pHeuristic )
{
	//Single Player Stats
	shotsFired				= pHeuristic.shotsFired;
	shotsHit				= pHeuristic.shotsHit;
	numberOfDeaths			= pHeuristic.numberOfDeaths;
	enemiesKilled			= pHeuristic.enemiesKilled; 
	timeOnLevel				= pHeuristic.timeOnLevel;
	levelStartTime			= pHeuristic.levelStartTime;
	levelEndTime			= pHeuristic.levelEndTime;
	playerHealth			= pHeuristic.playerHealth;
	lastLevel				= pHeuristic.lastLevel;
	currentLevel			= pHeuristic.currentLevel;
	
	
	shotsFiredInLevel		= pHeuristic.shotsFiredInLevel;
	shotsHitInLevel			= pHeuristic.shotsHitInLevel;
	enemiesKilledInLevel	= pHeuristic.enemiesKilledInLevel;
	teammatesKilledInLevel	= pHeuristic.teammatesKilledInLevel;
	
	//Multiplayer Stats
	ping					= pHeuristic.ping;
	itemsPickedUp			= pHeuristic.itemsPickedUp;
	specialEvents			= pHeuristic.specialEvents;
	
	//Skill Level
	skillLevel				= pHeuristic.skillLevel;
	
	//Mission Objectives
	numObjectives				= pHeuristic.numObjectives;
	numCompleteObjectives	= pHeuristic.numCompleteObjectives;
	numFailedObjectives		= pHeuristic.numFailedObjectives;
	
}

PlayerHeuristics::~PlayerHeuristics()
{
	
}

void PlayerHeuristics::ShowHeuristics( const Player *player )
{
	//Show Player Information in the console
	
	levelEndTime = level.time;
	timeOnLevel+= levelEndTime;
	playerHealth = player->health;
	
	ping = player->client->ping;
	
	float accuracy = 0;
	
	if ( shotsFired != 0 )
	{
		accuracy = (float)shotsHit / (float)shotsFired;
		accuracy *= 100.0f;
	}
	
	gi.Printf( "\nPlayer Heuristics:\n");
	gi.Printf( "Shots Fired:        %d\n", shotsFired );
	gi.Printf( "Shots Hit:	        %d\n", shotsHit );
	gi.Printf( "Accuracy:           %%%.2f\n", accuracy );
	gi.Printf( "Number of Deaths:   %d\n", numberOfDeaths );
	gi.Printf( "Time On Level:      %f\n", timeOnLevel );
	gi.Printf( "Player Health:      %d\n", playerHealth );
	gi.Printf( "Ping:               %d\n", ping );
	gi.Printf( "Items Picked Up:    %d\n", itemsPickedUp );
	gi.Printf( "Special Events:     %d\n", specialEvents );
	gi.Printf( "Enemies Killed      %d\n", enemiesKilled );
	gi.Printf( "\n" );
	gi.Printf( "Skill Level:        %f\n", CalculateSkillLevel() );
	
	gi.Printf( "\n" );
	
}

void PlayerHeuristics::SaveHeuristics( const Player *player )
{
	str token;
	str levelName;
	Script script;
	qboolean currentHeuristicsWritten;
	PlayerHeuristics *pHeuristic;
	
	//Precalculation for stats
	levelEndTime = level.time;
	timeOnLevel+= levelEndTime;
	playerHealth = player->health;
	ping = player->client->ping;
	skillLevel = CalculateSkillLevel();
	
	script.LoadFile( heuristicFileName );
	
	OpenPlayerHeuristicFile();
	if ( !heuristicFile )
	{
		script.Close();
		return;
	}
	
	//Save off a copy of our current stats
	pHeuristic = new PlayerHeuristics( *this );
	
	currentHeuristicsWritten = false;
	while( script.TokenAvailable( true ) )
	{
		token = script.GetToken(false);
		
		//The reason we saved off our old stats, and are reading through
		//is that in order to keep all the stats for all the other levels
		//we need to read them all in and write them back out, just replacing
		//the information we need for our current level
		if ( !Q_stricmp( token.c_str(), "Level:" ) )
		{
			levelName = script.GetToken(false);
			if ( Q_stricmp( levelName.c_str(), level.mapname.c_str() ) ) //Not Equal
			{
				script.UnGetToken();
				ReadInHeuristicData(script, levelName);
			}
			else
			{
				
				//Restore Values
				shotsFired					= pHeuristic->shotsFired;
				shotsHit					= pHeuristic->shotsHit;
				numberOfDeaths				= pHeuristic->numberOfDeaths;
				enemiesKilled				= pHeuristic->enemiesKilled;
				timeOnLevel					= pHeuristic->timeOnLevel;
				levelStartTime				= pHeuristic->levelStartTime;
				levelEndTime				= pHeuristic->levelEndTime;
				playerHealth				= pHeuristic->playerHealth;
				lastLevel					= pHeuristic->lastLevel;
				currentLevel				= pHeuristic->currentLevel;
				
				//Multiplayer Stats
				ping						= pHeuristic->ping;
				itemsPickedUp				= pHeuristic->itemsPickedUp;
				specialEvents				= pHeuristic->specialEvents;
				
				
				//Skill Level
				skillLevel					= pHeuristic->skillLevel;
				
				teammatesKilledInLevel		= pHeuristic->teammatesKilledInLevel;
				shotsFiredInLevel			= pHeuristic->shotsFiredInLevel;
				shotsHitInLevel				= pHeuristic->shotsFiredInLevel;
				
				numObjectives				= pHeuristic->numObjectives;
				numCompleteObjectives		= pHeuristic->numCompleteObjectives;
				numFailedObjectives			= pHeuristic->numFailedObjectives;
				
				
				
				currentHeuristicsWritten = true;
			}
			
			WriteOutHeuristicData(levelName);
		}
	}
	
	if (!currentHeuristicsWritten)
	{
		currentHeuristicsWritten = true;
		WriteOutHeuristicData(level.mapname);
	}
	
	ClosePlayerHeuristicFile();
}

void PlayerHeuristics::LoadHeuristics()
{
	str token;
	Script script;
	
	CheckForHeuristicFile();
	
	//Clear Heuristic Data Structure;
	shotsFired = 0;
	shotsHit = 0;
	numberOfDeaths = 0;
	enemiesKilled = 0;
	timeOnLevel = 0;
	levelStartTime = 0;
	levelEndTime = 0;
	playerHealth = 0;
	
	ping = 0;
	itemsPickedUp = 0;
	specialEvents = 0;
	
	//skillLevel = 0;
	
	script.LoadFile( heuristicFileName );
	
	ReadInHeuristicData(script, level.mapname);
	
	//level.setSkill(skillLevel);
	
	script.Close();
	ClosePlayerHeuristicFile();
}

void PlayerHeuristics::ReadInHeuristicData( Script& script, str& levelName )
{
	str token;
	str precedingToken ;
	
	//Fill Structure from File
	while( script.TokenAvailable( true ) )
	{
		str heuristic;
		str endSection;
		token = script.GetToken( false );
		
		if ( !Q_stricmp( token.c_str(), levelName.c_str() )  && ( precedingToken == "Level:" ) )
		{
			endSection = levelName + "_end";
			
			while ( Q_stricmp( token.c_str(), endSection.c_str() ) )
			{
				token = script.GetToken( false );
				if ( !Q_stricmp( token.c_str(), "ShotsFired:" ) )
				{
					heuristic = script.GetToken(false);
					shotsFired = atol(heuristic.c_str());
				}
				else if ( !Q_stricmp( token.c_str(), "ShotsHit:" ) )
				{
					heuristic = script.GetToken(false);
					shotsHit = atol(heuristic.c_str());
				}
				else if ( !Q_stricmp( token.c_str(), "NumberOfDeaths:" ) )
				{
					heuristic = script.GetToken(false);
					numberOfDeaths = atol(heuristic.c_str());
				}
				else if ( !Q_stricmp( token.c_str(), "EnemiesKilled:" ) )
				{
					heuristic = script.GetToken(false);
					enemiesKilled = atol(heuristic.c_str());
				}
				else if ( !Q_stricmp( token.c_str(), "TimeOnLevel:" ) )
				{
					heuristic = script.GetToken(false);
					timeOnLevel = (float)atof(heuristic.c_str());
				}
				else if ( !Q_stricmp( token.c_str(), "PlayerHealth:" ) )
				{
					heuristic = script.GetToken(false);
					playerHealth = atoi(heuristic.c_str());					
				}
				else if ( !Q_stricmp( token.c_str(), "Ping:" ) )
				{
					heuristic = script.GetToken(false);
					ping = atoi(heuristic.c_str());
				}
				else if ( !Q_stricmp( token.c_str(), "ItemsPickedUp:" ) )
				{
					heuristic = script.GetToken(false);
					itemsPickedUp = atoi(heuristic.c_str());
				}
				else if ( !Q_stricmp( token.c_str(), "SpecialEvents:" ) )
				{
					heuristic = script.GetToken(false);
					specialEvents = atoi(heuristic.c_str());
				}
				else if ( !Q_stricmp( token.c_str(), "SkillLevel:" ) )
				{
					heuristic = script.GetToken(false);
					skillLevel = atof(heuristic.c_str());
				}
			}
			return;
		}
		precedingToken = token ;  
	}
}

void PlayerHeuristics::WriteOutHeuristicData( const str &levelName )
{
	str s;
	str stat;
	
	//Build File String
	s = "Level: " + levelName + "\n";
	
	stat = va("%l", shotsFired);
	s+= "ShotsFired: "		 + stat + "\n";
	
	stat = va("%l", shotsHit);
	s+= "ShotsHit: "			 + stat + "\n";
	
	stat = va("%l", numberOfDeaths);
	s+= "NumberOfDeaths: "   + stat + "\n";
	
	stat = va("%l", enemiesKilled);	
	s+= "EnemiesKilled: "  + stat + "\n";
	
	stat = timeOnLevel;
	s+= "TimeOnLevel: "		 + stat + "\n";
	
	stat = playerHealth;
	s+= "PlayerHealth: "		 + stat + "\n";
	
	stat = ping;
	s+= "Ping: "				 + stat + "\n";
	
	stat = itemsPickedUp;
	s+= "ItemsPickedUp: "	 + stat + "\n";
	
	stat = specialEvents;
	s+= "SpecialEvents: "	 + stat + "\n";
	
	stat = skillLevel;
	s+= "SkillLevel: "	    + stat + "\n";
	
	s+= levelName + "_end";
	s+= "\n\n";
	
	gi.FS_Write( s, s.length(), heuristicFile );
	gi.FS_Flush( heuristicFile );
	
}

void PlayerHeuristics::CreateInitialHeuristicFile()
{
	//Clear Heuristic Data Structure;
	shotsFired = 0;
	shotsHit = 0;
	numberOfDeaths = 0;
	enemiesKilled = 0;
	timeOnLevel = 0;
	levelStartTime = 0;
	levelEndTime = 0;
	playerHealth = 0;
	
	ping = 0;
	itemsPickedUp = 0;
	specialEvents = 0;
	
	skillLevel = 0;
	
	OpenPlayerHeuristicFile();
	if ( !heuristicFile )
	{
		return;
	}
	
	WriteOutHeuristicData(level.mapname);
	ClosePlayerHeuristicFile();
}



void PlayerHeuristics::OpenPlayerHeuristicFile()
{
	if ( !heuristicFile )
	{
		heuristicFile = gi.FS_FOpenFileWrite( heuristicFileName.c_str() );
	}      
}

void PlayerHeuristics::ClosePlayerHeuristicFile()
{
	if ( heuristicFile )
	{
		gi.FS_FCloseFile( heuristicFile );
		heuristicFile = NULL;
	}
}

void PlayerHeuristics::CheckForHeuristicFile()
{
	str s,filename;
	int filesize;
	
	s = "heuristics";
	filename = gi.GetArchiveFileName( NULL, s, "log", qtrue );
	heuristicFileName = filename.c_str();
	
	filesize = gi.FS_ReadFile( heuristicFileName.c_str(), NULL, true );
	if ( filesize <= 0 ) //File Does Not Exsist
		CreateInitialHeuristicFile();
}

float PlayerHeuristics::CalculateSkillLevel()
{
	// Current Skill Level Formula
	// (((Accuracy * 100) * .75 ) + (1 - (Deaths * .25))) / 10
	// Returns a floating point number between 0 and 10
	
	// I expect this function to change a lot during the course of development
	// That is why it is written so verbosely.
	
	float Accuracy;
	float Deaths;
	float SkillLevel;
	
	Accuracy = (float)shotsHit / (float)shotsFired;
	Deaths = numberOfDeaths;
	
	SkillLevel = (((Accuracy * 100.0f) * .75f ) + (1.0f - (Deaths * .25f))) / 10.0f;
	if (SkillLevel < 0.0f )
	{
		SkillLevel = 0.0f;
	}
	
	return SkillLevel;
	
}



//-----------------------------------------------------
//
// Name:		ClearLevelStatistics
// Class:		PlayerHeuristics
//
// Description:	Clears the variables that are level specific.
//
// Parameters:	None
//
// Returns:		None
//-----------------------------------------------------
void PlayerHeuristics::ClearLevelStatistics(void)
{
	shotsFiredInLevel		= 0;		
	shotsHitInLevel			= 0;
	enemiesKilledInLevel	= 0;	
	numCompleteObjectives	= 0;
	numObjectives			= 0;
	numFailedObjectives		= 0;
	teammatesKilledInLevel	= 0;
}


void PlayerHeuristics::SetShotsFired( long int shots )
{
	shotsFired = shots;
}

void PlayerHeuristics::IncrementShotsFired( void )
{
	shotsFired++;
	shotsFiredInLevel++;
}

void PlayerHeuristics::SetShotsHit( long int shots )
{
	shotsHit = shots;   
}

void PlayerHeuristics::IncrementShotsHit()
{
	shotsHit++;
	shotsHitInLevel++;
}

void PlayerHeuristics::SetNumberOfDeaths( long int deaths )
{
	numberOfDeaths = deaths;
}

void PlayerHeuristics::IncrementNumberOfDeaths()
{
	numberOfDeaths++;
}

void PlayerHeuristics::SetTimeOnLevel( float levelTime )
{
	timeOnLevel = levelTime;
}

void PlayerHeuristics::SetPlayerHealth( int health )
{
	playerHealth = health;
}

void PlayerHeuristics::SetPing( int ping_value )
{
	ping = ping_value;
}

void PlayerHeuristics::SetItemsPickedUp( int items )
{
	itemsPickedUp = items;
}

void PlayerHeuristics::IncrementItemsPickedUp()
{
	itemsPickedUp++;
}

void PlayerHeuristics::SetSpecialEvents( int specialevent )
{
	specialEvents = specialevent;
}

void PlayerHeuristics::IncrementSpecialEvents()
{
	specialEvents++;
}

void PlayerHeuristics::UpdateShotsFired( void )
{
	IncrementShotsFired();	
}


void PlayerHeuristics::SetObjectives(int numMissionObjectives)
{
	numObjectives = numMissionObjectives;
}

void PlayerHeuristics::IncrementCompleteObjectives()
{
	numCompleteObjectives++;
}

void PlayerHeuristics::IncrementFailedObjectives()
{
	numFailedObjectives++;
}

void PlayerHeuristics::IncrementEnemiesKilled(void)
{
	enemiesKilled++;
	enemiesKilledInLevel++;
}

void PlayerHeuristics::IncrementTeammatesKilled(void)
{
	teammatesKilledInLevel++;
}
