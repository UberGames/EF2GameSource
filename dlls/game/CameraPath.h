//-----------------------------------------------------------------------------
// CameraPath.h
// 
// Author:  Squirrel Eiserloh
// 
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
// 
// DESCRIPTION:
//	Header file for the following camera-related classes:
// 
//	CameraPath			Describes a single camera path through time -
//						including camera orientation, position,
//						and field-of-view - using either many simple
//						reference keyframes or a few bspline nodes.
//						Also owns the coordinate system through which
//						all positions and orientations are transformed
//						(for scripts with relative playback locales).
//	
//	CameraKeyFramePath	The innards of a CameraPath object implemented
//						using keyframes (as opposed to bspline nodes).
//						Does not know about relative locales.
// 
//	CameraKeyFrame		A single key frame item in a CameraKeyFramePath.
//						Stores a time index / frame number, the camera'a
//						position (as a Vector), orientation (as a
//						Quaternion), and field-of-fiew (as a Scalar).
//
#ifndef _CAMERA_PATH_H_
#define _CAMERA_PATH_H_


/// Included headers
#include "class.h"
#include <qcommon/quaternion.h>

/// forward class declarations
class CameraPath;
class CameraKeyFramePath;
class CameraKeyFrame;

const float DEFAULT_KEY_FRAME_LENGTH_SECONDS = 0.05f;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		CameraKeyFrame
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CameraKeyFrame : public Class
{
	/// Member variables
private:
	Vector						_position;		// the camera's position in xyz coordinates
	Quaternion					_orientation;	// the camera's orientation, as a quaternion
	float						_fovDegrees;	// the camera's horizontal field of view
	float						_seconds;		// time (path start = 0) at which the key frame occurs

	/// Accessors / Mutators
public:
	inline const Vector&		GetPosition( void ) const { return _position; }
	inline const Quaternion&	GetOrientation( void ) const { return _orientation; }
	inline const float			GetFOV( void ) const { return _fovDegrees; }
	inline const float			GetSeconds( void ) const { return _seconds; }

	/// Construction / Destruction
public:
	CLASS_PROTOTYPE( CameraKeyFrame );
								CameraKeyFrame(); // default constructor
								CameraKeyFrame( const Vector& position, const Quaternion& orientation, const float fovDegrees, const float seconds );
	/// Interface methods
public:
	bool						ParseFrameInfo( int frameNumber, Script& cameraPathFile, float& frameLengthInSeconds );
	const CameraKeyFrame&		Interpolate( const CameraKeyFrame& before, const CameraKeyFrame& after, const float fraction, const bool normalizeQuaternion );
	const Vector				GetEulerAngles( void ) const;
	const CameraKeyFrame&		TransformToPlaybackOffsets( const float yawOffsetDegrees, const Vector& originOffset );

	void						Archive( Archiver &arc );

	/// Implementation methods
private:
	bool						ParseFrameInfoBlock( Script& cameraPathFile, float& frameLengthInSeconds );
};

//===============================================================
// Name:		Archive
// Class:		CameraKeyFrame
//
// Description: Archives the data of a single camera key frame.
// 
// Parameters:	Archiver& -- reference to archive object for storing data.
//
// Returns:		None
// 
//===============================================================
inline void CameraKeyFrame::Archive
(
	Archiver &arc
)
{
	Class::Archive( arc );

	float x = 0.0f ;
	float y = 0.0f ;
	float z = 0.0f ;
	float w = 0.0f ;


	arc.ArchiveVector( &_position );

	if ( arc.Saving() )
	{
		_orientation.GetToXYZW( x, y, z, w );
	}

	arc.ArchiveFloat( &x );
	arc.ArchiveFloat( &y );
	arc.ArchiveFloat( &z );
	arc.ArchiveFloat( &w );

	if ( arc.Loading() )
	{
		_orientation.SetFromXYZW( x, y, z, w );
	}

	arc.ArchiveFloat( &_fovDegrees );
	arc.ArchiveFloat( &_seconds );
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		CameraKeyFramePath
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CameraKeyFramePath : public Class
{
	/// Member variables
private:
	bool						_isLoaded;
	int							_numKeyFrames;
	CameraKeyFrame*				_keyFrameArray;
	float						_totalSeconds;

	/// Accessors / Mutators
public:
	inline const bool			IsLoaded( void ) const { return _isLoaded; }
private:
	inline const int			GetNumKeyFrames( void ) const { return _numKeyFrames; }

	/// Construction / Destruction
public:
	CLASS_PROTOTYPE( CameraKeyFramePath );
								CameraKeyFramePath(); // default constructor

	/// Interface methods
public:
	void						Unload( void );
	bool						ParsePathInfo( Script& cameraPathFile );
	const CameraKeyFrame		CreateInterpolatedFrameForTime( const float seconds );
	float						GetPathLengthInSeconds( void );

	void						Archive( Archiver &arc );

	/// Implementation methods
private:
	void						GetClosestFramesForTime( CameraKeyFrame*& before, CameraKeyFrame*& after, const float seconds );
	bool						CreateFrames( const int numberOfFrames );
	const CameraKeyFrame		CreateInterpolatedFrame( const CameraKeyFrame& before, const CameraKeyFrame& after, const float seconds );
};



//===============================================================
// Name:		Archive
// Class:		CameraKeyFramePath
//
// Description: Archives the data of a archive key frame path.
// 
// Parameters:	Archiver& -- reference to the archive object
//
// Returns:		None
// 
//===============================================================
inline void CameraKeyFramePath::Archive
(
	Archiver &arc
)
{
	Class::Archive( arc );

	arc.ArchiveBool( &_isLoaded );
	arc.ArchiveInteger( &_numKeyFrames );

	if ( arc.Loading() )
	{
		_keyFrameArray = new CameraKeyFrame[ _numKeyFrames ];
	}

	for ( int frameIdx = 0; frameIdx < _numKeyFrames; ++frameIdx )
	{
		arc.ArchiveObject( &_keyFrameArray[ frameIdx ] );
	}

	arc.ArchiveFloat( &_totalSeconds );
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		CameraPath
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CameraPath : public Class
{
	/// Member variables
private:
	bool						_isLoaded;
	str							_fileName;
	CameraKeyFramePath*			_keyFramePath;
	float						_yawPlaybackOffsetDegrees;
	Vector						_originPlaybackOffset;

	/// Accessors / Mutators
public:
	inline const bool			IsLoaded( void ) const { return _isLoaded; }

	/// Construction / Destruction
public:
	CLASS_PROTOTYPE( CameraPath );
								CameraPath( const str& fileName );
								CameraPath()						: _isLoaded( false ), _keyFramePath( 0 ), _yawPlaybackOffsetDegrees( 0 ) { };

	/// Interface methods
public:
	void						Unload( void );
	bool						Reload( void );
	const CameraKeyFrame		GetInterpolatedFrameForTime( const float seconds );
	void						SetPlaybackOffsets( const float yawOffsetDegrees, const Vector& originOffset );
	float						GetPathLengthInSeconds( void );

	void						Archive( Archiver &arc );

	/// Implementation methods
private:
	bool						LoadKeyFramedCameraFile( const str& fileName );
	bool						ParsePathInfo( Script& cameraPathFile );
};


//===============================================================
// Name:		Archive
// Class:		CameraPath
//
// Description: Archives the data of the camera path.
// 
// Parameters:	Archiver& -- reference to archiver storing data.
//
// Returns:		None
// 
//===============================================================
inline void CameraPath::Archive
(
	Archiver &arc
)
{
	Class::Archive( arc );

	arc.ArchiveBool( &_isLoaded );
	arc.ArchiveString( &_fileName );

	//arc.ArchiveObjectPointer( (Class**)&_keyFramePath );

	if ( arc.Loading() )
	{
		_keyFramePath = ( CameraKeyFramePath*)arc.ReadObject();
	}
	else
	{
		arc.ArchiveObject( _keyFramePath );
	}

	arc.ArchiveFloat( &_yawPlaybackOffsetDegrees );
	arc.ArchiveVector( &_originPlaybackOffset );
}


#endif // _CAMERA_PATH_H_
