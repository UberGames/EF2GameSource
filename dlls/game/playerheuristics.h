
//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/playerheuristics.h                            $
// $Revision:: 10                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:11a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// PlayerHeuristics Class Definition.
//

class PlayerHeuristics;

// PlayerHeuristics class
//
// This class and file are in a state of transition, and currently, is messy.
// I have, at least, managed to pull most of the heuristic stuff out of player.cpp and place it here,
// for now.
//
// Future Plans:
// Where this mess is headed -- Eventually, I would like to create a new system where we have a 
// Heuristics manager that is seperate from anything in the game ( like Player, or Weapon ) and 
// Thus could be used by anyone to track their own stats.  
//
// PlayerHeuristics would still be around, though its role would change to that of an intermediary
// between the new Heuristics Manager and Player.  It would control what stats it's tracking and 
// manage any _PLAYER_ specific heuristic data.
//

#ifndef __PLAYER_HEURISTICS_H__
#define __PLAYER_HEURISTICS_H__

#include "player.h"

class PlayerHeuristics : public Class
{
	public:
		
				 PlayerHeuristics();
				 PlayerHeuristics( const PlayerHeuristics &pHeuristic );
				~PlayerHeuristics();
		
		void	ShowHeuristics( const Player *player );
		void	SaveHeuristics( const Player *player );
		void	LoadHeuristics();
		void	ReadInHeuristicData(Script& script, str& levelName);
		void	WriteOutHeuristicData( const str& levelName );
		void	CreateInitialHeuristicFile();
		void	OpenPlayerHeuristicFile();
		void	ClosePlayerHeuristicFile();
		void	CheckForHeuristicFile();
		float	CalculateSkillLevel();
		void	ClearLevelStatistics(void);
		
		void	SetShotsFired( long int shots);	
		void	IncrementShotsFired( void );
		void	SetShotsHit( long int shots);
		void	IncrementShotsHit();
		void	SetNumberOfDeaths( long int deaths);  
		void	IncrementNumberOfDeaths();
		void	SetTimeOnLevel( float levelTime );
		void	SetPlayerHealth( int health );
		void	SetPing (int ping_value );
		void	SetItemsPickedUp ( int items );
		void	IncrementItemsPickedUp ();

		void	IncrementTeammatesKilled(void);

		void	SetSpecialEvents( int specialevent );
		void	IncrementSpecialEvents();
		void	UpdateShotsFired( void );
		void	SetEnemiesKilled( long int enemies );
		void	IncrementEnemiesKilled(void);

		//Mission objectives
		void	SetObjectives(int numMissionObjectives);
		void	IncrementCompleteObjectives();
		void	IncrementFailedObjectives();


		
		//Single Player Stats Persistent
		long int	shotsFired;
		long int	shotsHit;
		long int	numberOfDeaths;
		long int	enemiesKilled;

		//level properties...
		//these are the statistics for that instance of the level
		long int	shotsFiredInLevel;		
		long int	shotsHitInLevel;
		long int	enemiesKilledInLevel;
		long int	teammatesKilledInLevel;

		int			numObjectives;
		int			numCompleteObjectives;
		int			numFailedObjectives;
		

		float		timeOnLevel;
		float		levelStartTime;
		float		levelEndTime;
		float		playerHealth;
		str			lastLevel;
		str			currentLevel;

 		//Multiplayer Stats
		int			ping;
		int			itemsPickedUp;
		int			specialEvents;
		
		//Weapon Stats


		//Skill Level
		float		skillLevel;

};




#endif /* __PLAYER_HEURISTICS_H__ */
