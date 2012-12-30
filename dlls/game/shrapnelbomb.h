//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/shrapnelbomb.h                                $
// $Revision:: 7                                                              $
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

#ifndef __SHRAPNELBOMB_H__
#define __SHRAPNELBOMB_H__

#include "weapon.h"
#include "weaputils.h"

class ShrapnelBomb : public Projectile
   {
	private:
		str			shrapnelModel;
		int			shrapnelCount;
		bool		_splitOnDescent;
		bool		_randomSpread;

	public:
		CLASS_PROTOTYPE( ShrapnelBomb );

		      ShrapnelBomb();
		void  SetShrapnelModel ( Event *ev );
		void  SetShrapnelCount ( Event *ev );

		//Override
		void  Explode			  ( Event *ev );

		void						Explode( void );

		/* virtual */ void			Think( void );

		virtual void Archive ( Archiver &arc );

	};

inline void ShrapnelBomb::Archive ( Archiver &arc )
   {
	Projectile::Archive( arc );

	arc.ArchiveString( &shrapnelModel );
	arc.ArchiveInteger( &shrapnelCount );
	arc.ArchiveBool( &_splitOnDescent );
	arc.ArchiveBool( &_randomSpread );
	}
        

#endif //__SHRAPNELBOMB_H__
