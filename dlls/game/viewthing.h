//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/viewthing.h                                  $
// $Revision:: 19                                                             $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:54a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Actor code for the viewthing.
//

#ifndef __VIEWTHING_H__
#define __VIEWTHING_H__

#include "animate.h"

// forward declaration
class Viewthing ;

class ViewMaster : public Listener
	{
	public:
      CLASS_PROTOTYPE( ViewMaster );

		EntityPtr current_viewthing;
		Container<str>  _modelNamesArray ;
		Container<str>  _setNamesArray ;
		Vector          _lastBaseOrigin ;
		bool            _spawnAtLastItemsOrigin ;
		int             _currentSetIdx ;
		int             _numberOfModelsInSet ;
		int             _numberOfSets ;

		ViewMaster();
		~ViewMaster();
		
		void LoadSet ( Event *ev );
		void NextModelInSet ( Event *ev );
		void PrevModelInSet ( Event *ev );
		void JumpToModel    ( Event *ev );
		void PullToCamera   ( Event *ev );
		void SetXTranslation( Event *ev );
		void SetYTranslation( Event *ev );
		void SetZTranslation( Event *ev );
		void XTranslate     ( Event *ev );
		void YTranslate     ( Event *ev );
		void ZTranslate     ( Event *ev );
		void Offset		     ( Event *ev );
		void Copy           ( Event *ev );
		void Save           ( Event *ev );
		void Shoot          ( Event *ev );
		void ResetOrigin    ( Event *ev );
		void Delete         ( Event *ev );

		void LoadModelsInSet ( const str& setFilename);  // Loads up list of tikis in .lst files
		void spawnAtPosition(const str& modelName, const Vector& pos);
		void DeleteCurrentViewthing();
		void DisplayCurrentModelInSet(bool spawnAtLastOrigin=true);
		void ToggleDrop( Event *ev );
		void Spawn( Event *ev );
		void SpawnFromTS( Event *ev );

		// These iterate through models already spawned
		void Next( Event *ev );
		void Prev( Event *ev );

		// Additional functions
		void Init( void );
		void DeleteAll( Event *ev );
		void SetModelEvent( Event *ev );
		void PassEvent( Event *ev );
		virtual void Archive( Archiver &arc );

	protected:
		void _resetOrigin();
		void _setToPosition( const Vector& pos );
		void _selectPrevious();
		void _selectNext();
		void _updateViewthingCounts(int countAdjustment=0); // supports manipulating final count--see notes in .cpp
		void _randomizeViewthing(Viewthing* viewthingPtr);  // randomizes scale and/or rotation of this viewthing
	};

inline void ViewMaster::Archive
	(
	Archiver &arc
	)

   {
   Listener::Archive( arc );

   arc.ArchiveSafePointer( &current_viewthing );
   }


// The DLL Global ViewMaster singleton
extern ViewMaster Viewmodel;



//-----------------------------------------------------------
// Class Viewthing
//      A viewspawned tiki model in the world. This class
//      provides a shell for viewing any tiki model in the
//      game.  The model can be animated and manipulated, but
//      does not act like a normally spawned object (it doesn't
//      execute scripts or AI.
//-----------------------------------------------------------
class Viewthing : public Entity
	{
	public:
      CLASS_PROTOTYPE( Viewthing );

		int      animstate;
		int      frame;
		int      lastframe;
		Vector   baseorigin;
		byte     origSurfaces[MAX_MODEL_SURFACES];
		int		 current_morph;
		qboolean _static ;
		qboolean _selected ;
		int      _pulseCount ;
						Viewthing();
		void			UpdateCvars( qboolean quiet = false );
		void            SetSelected( qboolean state = false );
		void			SetAnim           ( int num );
		void            Delete();

		void			PrintTime         ( Event *ev );
		void			ThinkEvent        ( Event *ev );
		void			LastFrameEvent    ( Event *ev );
		void			ToggleAnimateEvent( Event *ev );
		void			SetModelEvent     ( Event *ev );
		void			NextFrameEvent    ( Event *ev );
		void			PrevFrameEvent    ( Event *ev );
		void			NextAnimEvent     ( Event *ev );
		void			PrevAnimEvent     ( Event *ev );
		void			ScaleUpEvent      ( Event *ev );
		void			ScaleDownEvent    ( Event *ev );
		void			SetScaleEvent     ( Event *ev );
		void			SetYawEvent       ( Event *ev );
		void			SetPitchEvent     ( Event *ev );
		void			SetRollEvent      ( Event *ev );
		void			SetAnglesEvent    ( Event *ev );
		void            SetStatic         ( Event *ev );
		void			AttachModel       ( Event *ev );
		void			Delete            ( Event *ev );
		void			DetachAll         ( Event *ev );
		void			ChangeOrigin      ( Event *ev );
		void			SaveSurfaces      ( Event *ev );
		void			SetAnim           ( Event *ev );
		void            ChangeRollEvent   ( Event *ev );
		void            ChangePitchEvent  ( Event *ev );
		void            ChangeYawEvent    ( Event *ev );
		void			NextMorph         ( Event *ev );
		void			PrevMorph         ( Event *ev );
		void			Morph             ( Event *ev );
		void			Unmorph           ( Event *ev );
		void            Flash             ( Event *ev );
		void			SetFrame		  ( Event *ev );

		virtual void	Archive( Archiver &arc );
	};

inline void Viewthing::Archive
	(
	Archiver &arc
	)

   {
   Entity::Archive( arc );

   arc.ArchiveInteger( &animstate );
   arc.ArchiveInteger( &frame );
   arc.ArchiveInteger( &lastframe );
   arc.ArchiveVector( &baseorigin );
   arc.ArchiveRaw( origSurfaces, sizeof( origSurfaces ) );
	arc.ArchiveInteger( &current_morph );
   }

#endif /* viewthing.h */
