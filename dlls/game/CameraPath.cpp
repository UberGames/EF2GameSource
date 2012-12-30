//-----------------------------------------------------------------------------
// CameraPath.cpp
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
//	Implementation file for the following camera-related classes:
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

#include "_pch_cpp.h"
#include "CameraPath.h"




/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		CameraKeyFrame
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLASS_DECLARATION( Class, CameraKeyFrame, NULL )
{
	{ NULL, NULL }
};

//-----------------------------------------------------------------------------------------------
// Name:		CameraKeyFrame
// Class:		CameraKeyFrame
//
// Description: Default Constructor
// 
// Parameters:	None
//
// Returns:		N/a
// 
//-----------------------------------------------------------------------------------------------
CameraKeyFrame::CameraKeyFrame()
:
_position( Vector( 0.0f, 0.0f, 0.0f ) ),
_orientation( Quaternion( 0.0f, 0.0f, 0.0f, 1.0f ) ),
_fovDegrees( 90.0f ),
_seconds( 0.0f )
{
}


//-----------------------------------------------------------------------------------------------
// Name:		CameraKeyFrame
// Class:		CameraKeyFrame
//				
// Description:	Constructor( Vector, Quaternion, float )
//				
// Parameters:	position		XYZ coordinates of camera
//				orientation		camera orientation as a quaternion
//				fovDegrees		horizontal fov of camera, in degrees
//				
// Returns:		n/a
//				
//-----------------------------------------------------------------------------------------------
CameraKeyFrame::CameraKeyFrame( const Vector& position, const Quaternion& orientation, const float fovDegrees, const float seconds )
:
_position( position ),
_orientation( orientation ),
_fovDegrees( fovDegrees ),
_seconds( seconds )
{
}


//-----------------------------------------------------------------------------------------------
// Name:		ParseFrameInfo
// Class:		CameraKeyFrame
//				
// Description:	Starting with the keyword "Frame", parses one single frame entry from a .KFC
//				buffer (using a Script object) in its entirety.
//				
// Parameters:	int frameNumber, Script& cameraPathFile
//				
// Returns:		bool - true upon success
//				
//-----------------------------------------------------------------------------------------------
bool CameraKeyFrame::ParseFrameInfo( int frameNumber, Script& cameraPathFile, float& totalPathTime )
{
	const char* token;

	/// Read in the Frame number
	token = cameraPathFile.GetToken( true );
	if( !stricmp( token, "Frame" ) )
	{
		/// Make sure the frame number is the same as <frameNumber>, as expected
		int actualFrame = cameraPathFile.GetInteger( false );
		if( actualFrame != frameNumber )
		{
			cameraPathFile.error( "CameraKeyFrame::ParseFrameInfo", "Incorrect Frame number (found %d, expecting %d)\n", frameNumber, actualFrame );
			return( false );
		}
	}
	else // bad identifier
	{
		gi.Printf( "Unexpected token %s in camera path file (expecting \"Frame\").\n", token );
		cameraPathFile.error( "CameraKeyFrame::ParseFrameInfo", "Unexpected token %s in camera path file (expecting \"Frame\").\n", token );
		return( false );
	}

	return( ParseFrameInfoBlock( cameraPathFile, totalPathTime ) );
}


//-----------------------------------------------------------------------------------------------
// Name:		ParseFrameInfoBlock
// Class:		CameraKeyFrame
//				
// Description:	Parse in a single camera key frame from a .KFC file, from '{' to '}'
//				
// Parameters:	Script& cameraPathFile - parse object for .KFC file
//				
// Returns:		bool - true upon success
//				
//-----------------------------------------------------------------------------------------------
bool CameraKeyFrame::ParseFrameInfoBlock( Script& cameraPathFile, float& totalPathTime )
{
	const char* token;
	float frameTimeInSeconds = DEFAULT_KEY_FRAME_LENGTH_SECONDS;

	/// Read the opening bracket
	token = cameraPathFile.GetToken( true );
	if( *token != '{' )
	{
		cameraPathFile.error( "CameraKeyFrame::ParseFrameInfoBlock", "Expected '{', found \"%s\"\n", token );
		return( false );
	}

	/// Read each entry in the Path info block
	token = cameraPathFile.GetToken( true );
	while( *token != '}' )
	{
		if( !stricmp( token, "fov" ) )
		{
			/// Read in the horizontal field of view (fov) for this camera key frame
			_fovDegrees = cameraPathFile.GetFloat( false );
		}
		else if( !stricmp( token, "position" ) )
		{
			/// Read in the position vector for this camera key frame
			_position = cameraPathFile.GetVector( false );
		}
		else if( !stricmp( token, "quaternion" ) )
		{
			/// Read in the orientation of the camera as a quaternion
			float x = cameraPathFile.GetFloat( false );
			float y = cameraPathFile.GetFloat( false );
			float z = cameraPathFile.GetFloat( false );
			float w = cameraPathFile.GetFloat( false );
			_orientation.SetFromXYZW( x, y, z, w );
		}
		else // unknown token
		{
			gi.Printf( "Unexpected token %s in camera path file.\n", token );
			cameraPathFile.error( "CameraKeyFrame::ParseFrameInfoBlock", "Unexpected token %s in camera path file.\n", token );
			return( false );
		}

		token = cameraPathFile.GetToken( true );
	}

	_seconds = totalPathTime;
	totalPathTime += frameTimeInSeconds;
	return( true );
}



//-----------------------------------------------------------------------------------------------
// Name:		Interpolate
// Class:		CameraKeyFrame
//				
// Description:	Sets the position, orientation, and fov values in the current CameraKeyFrame
//				object (this) to be a weighted average of two other key frames: <before> and
//				<after>, where <fraction> represents the weighting of the <after> frame (and
//				1 - fraction is the weighting of the <before> frame).
//				
// Parameters:	const CameraKeyFrame& before - key frame being approached as fraction -> 0
//				const CameraKeyFrame& after - key frame being approached as fraction -> 1
//				const float fraction - in the range [0,1]; the fraction of <after> to be used
//				
// Returns:		const CameraKeyFrame& 
//				
//-----------------------------------------------------------------------------------------------
const CameraKeyFrame& CameraKeyFrame::Interpolate( const CameraKeyFrame& before, const CameraKeyFrame& after, const float fraction, const bool normalizeQuaternion )
{
	/// Set up some convenience references
	const Vector& pos1 = before.GetPosition();
	const Vector& pos2 = after.GetPosition();
	const Quaternion& quat1 = before.GetOrientation();
	const Quaternion& quat2 = after.GetOrientation();
	float fov1 = before.GetFOV();
	float fov2 = after.GetFOV();

	/// Calculate interpolated position, orientation, and fov values for this new frame

	_position = pos1 + ( pos2 - pos1 ) * fraction; // linear componentwise interpolation

	_orientation = CalcSlerp( quat1, quat2, (double) fraction ); // (hyper)spherical linear interpolation
	_fovDegrees = fov1 + (fraction * (fov2 - fov1) ); // linear float interpolation

	/// Check if the caller has requested the quaternion to be renormalized after interpolation (a good idea, but costly)
	if( normalizeQuaternion )
		_orientation.Normalize();

	return( *this );
}


//-----------------------------------------------------------------------------------------------
// Name:		GetEulerAngles
// Class:		CameraKeyFrame
//				
// Description:	Returns the orientation of the key frame in Euler angles (pitch, yaw, roll).
//				Note that orientations are internally stored as quaternions, and this function
//				is mainly provided as a interface useful to the surrounding Euler-centric
//				Quake / Fakk / Tiki code.
//				
// Parameters:	
//				
// Returns:		const Vector
//				
//-----------------------------------------------------------------------------------------------
const Vector CameraKeyFrame::GetEulerAngles( void ) const
{
	Vector eulerAngles;
	_orientation.GetToEuler( eulerAngles );
	return eulerAngles;
}




//-----------------------------------------------------------------------------------------------
// Name:		TransformToPlaybackOffsets
// Class:		CameraKeyFrame
//				
// Description:	Transforms a CameraKeyFrame object by (1) rotating it <yawOffsetDegrees> degrees
//				(including orientations and positions) and then (2) translating it by
//				<originOffset> world-units.
//				
// Parameters:	const float yawOffset - the amount of (yaw) rotation to apply
//				const Vector& originOffset - the amount of translation to apply
//				
// Returns:		const CameraKeyFrame& - returns *this, now transformed
//				
// NOTE: Because q_math.c is a C file and was written long ago, we use vec3_t and euler
//	angles instead of Vector and Quaternion.
//-----------------------------------------------------------------------------------------------
const CameraKeyFrame& CameraKeyFrame::TransformToPlaybackOffsets( const float yawOffsetDegrees, const Vector& originOffset )
{
	/// Rotate <_position> about the Z axis by <yawOffsetDegrees>
	vec3_t zNormal = { 0.0f, 0.0f, 1.0f };
	vec3_t point;
	vec3_t rotated;
	_position.copyTo( point ); // Don't blame me for this.  Our math library was written in C.
	RotatePointAroundVector( rotated, zNormal, point, yawOffsetDegrees );
	_position = rotated;

	/// Rotate <_orientation> in a like manner
	Vector eulerOrientation;
	_orientation.GetToEuler( eulerOrientation );
	eulerOrientation.y += yawOffsetDegrees;
	_orientation.SetFromEuler( eulerOrientation );
	_orientation.Normalize();

	/// Translate <_position> by <originOffset>
	_position += originOffset; // see how easy things can be in C++?

	return *this;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		CameraKeyFramePath
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLASS_DECLARATION( Class, CameraKeyFramePath, NULL )
{
	{ NULL, NULL }
};


//-----------------------------------------------------------------------------------------------
// Name:		CameraKeyFramePath
// Class:		CameraKeyFramePath
//				
// Description:	Default Constructor
//				
// Parameters:	None
//				
// Returns:		n/a
//				
//-----------------------------------------------------------------------------------------------
CameraKeyFramePath::CameraKeyFramePath()
:
_isLoaded( false ),
_numKeyFrames( 0 ),
_keyFrameArray( NULL ),
_totalSeconds( 0.0f )
{
}


//-----------------------------------------------------------------------------------------------
// Name:		Unload
// Class:		CameraKeyFramePath
//				
// Description:	Destroys the key-framed camera path data and
//				marks the object as dirty (_isLoaded = false).
//				
// Parameters:	void
//				
// Returns:		void 
//				
//-----------------------------------------------------------------------------------------------
void CameraKeyFramePath::Unload( void )
{
	if( IsLoaded() )
	{
		delete [] _keyFrameArray;
		_keyFrameArray = NULL;
	}

	_totalSeconds = 0.0f;
	_isLoaded = false;
}


//-----------------------------------------------------------------------------------------------
// Name:		CreateFrames
// Class:		CameraKeyFramePath
//				
// Description:	Allocates <numberOfFrames> CameraKeyFrame objects
//				in a dynamic array.  Previous data, if any, is
//				deleted.
//				
// Parameters:	numberOfFrames - The number of key frames to be allocated.
//				
// Returns:		bool - true upon success
//				
//-----------------------------------------------------------------------------------------------
bool CameraKeyFramePath::CreateFrames( const int numberOfFrames )
{
	/// Unload the previous data (if any)
	Unload();

	/// Allocte <numberOFFrames> CameraKeyFrame objects
	_keyFrameArray = new CameraKeyFrame[ numberOfFrames ];
	if( !_keyFrameArray )
		return( false );

	/// Set internal dependents
	_numKeyFrames = numberOfFrames;
	return( true );
}


//-----------------------------------------------------------------------------------------------
// Name:		ParsePathInfo
// Class:		CameraKeyFramePath
//				
// Description:	Parses a "Path" entry and all subsequent "Frame" entries for that path.
//				Parsing begins at the '{' following the "Path" keyword (already parsed).
//				
// Parameters:	Script& cameraPathFile - parsing object; keeps track of parse-offset, etc.
//				
// Returns:		bool - true upon success
//				
//-----------------------------------------------------------------------------------------------
bool CameraKeyFramePath::ParsePathInfo( Script& cameraPathFile )
{
	const char* token;

	/// Read the opening bracket
	token = cameraPathFile.GetToken( true );
	if( *token != '{' )
	{
		cameraPathFile.error( "CameraKeyFramePath::ParsePathInfo", "Expected '{', found \"%s\"\n", token );
		return( false );
	}

	/// Read each entry in the Path info block
	token = cameraPathFile.GetToken( true );
	while( *token != '}' )
	{
		if( !stricmp( token, "frameCount" ) )
		{
			/// Read in the number of frames in the path
			_numKeyFrames = cameraPathFile.GetInteger( false );
			assert( _numKeyFrames > 0 );
		}
		else // unknown token
		{
			gi.Printf( "Unexpected token %s in camera path file.\n", token );
			cameraPathFile.error( "CameraKeyFramePath::ParsePathInfo", "Unexpected token %s in camera path file.\n", token );
			return( false );
		}
		token = cameraPathFile.GetToken( true );
	}

	/// Allocate an array of CameraKeyFrame objects equal in number to what "frameCount" specified
	CreateFrames( _numKeyFrames );

	/// Loop through each key frame and let it parse itself
	for( int i = 0; i < _numKeyFrames; i ++ )
	{
		/// Tell each frame to parse itself
		CameraKeyFrame& keyFrame = _keyFrameArray[ i ];
		bool success = keyFrame.ParseFrameInfo( i, cameraPathFile, _totalSeconds );
		if( !success )
			return( false );
	}

	/// Inform the object that it has been successfully loaded and is ready for use
	_isLoaded = true;
	return( true );
}


//-----------------------------------------------------------------------------------------------
// Name:		GetClosestFramesForTime
// Class:		CameraKeyFramePath
//				
// Description:	Sets <before> and <after> pointers to the camera key frames closest in time
//				to <seconds>.  Both pointers are guaranteed to always be set, though they
//				may be identical (especially in cases where <seconds> matches a frame exactly,
//				or where <seconds> is out of the time bounds of the path).
//				
// Parameters:	CameraKeyFrame*& before - pointer (by reference) to be set to the closest
//					frame at or before <seconds>
// 
//				CameraKeyFrame*& after - pointer (by reference) to be set to the closest
//					frame at or after <seconds>
// 
//				const float seconds - the time offset, in seconds, from the beginning of the
//					camera path, around which the search for closest frames is centered
//				
// Returns:		void 
//				
//-----------------------------------------------------------------------------------------------
void CameraKeyFramePath::GetClosestFramesForTime( CameraKeyFrame*& before, CameraKeyFrame*& after, const float seconds )
{
	int i;

	/// Find the frame closest to - but no greater than - <seconds>
	before = &_keyFrameArray[ 0 ];
	for( i = 0; i < _numKeyFrames; i ++ )
	{
		CameraKeyFrame& frame = _keyFrameArray[ i ];
		if( frame.GetSeconds() > seconds )
			break;

		before = &frame;
	}

	/// Find the frame closest to - but no less than - <seconds>
	after = &_keyFrameArray[ _numKeyFrames - 1 ];
	for( i = _numKeyFrames - 1; i >= 0; i -- )
	{
		CameraKeyFrame& frame = _keyFrameArray[ i ];
		if( frame.GetSeconds() < seconds )
			break;

		after = &frame;
	}

}


//-----------------------------------------------------------------------------------------------
// Name:		CreateInterpolatedFrameForTime
// Class:		CameraKeyFramePath
//				
// Description:	Creates a new CameraKeyFrame object based on the best interpolated-approximation
//				of the CameraKeyFramePath when evaluated at <seconds> time.
//				
// Parameters:	const float seconds - time at which to evaluate the camera path
//				
// Returns:		const CameraKeyFrame - a new key frame, created by interpolating (if necessary)
//					between the key frames in the camera path closest to <seconds>.
//				
//-----------------------------------------------------------------------------------------------
const CameraKeyFrame CameraKeyFramePath::CreateInterpolatedFrameForTime( const float seconds )
{
	/// Check if <seconds> is out-of-bounds for this camera path
	if( seconds < 0.0f )
	{
		/// Return a copy of the first key frame
		return( _keyFrameArray[ 0 ] );
	}
	else if( seconds > _totalSeconds )
	{
		/// Return a copy of the last key frame
		return( _keyFrameArray[ _numKeyFrames - 1 ] );
	}

	/// Get the two closest frames in time to <seconds>; one just before and one just after
	CameraKeyFrame* closestFrameBefore = NULL;
	CameraKeyFrame* closestFrameAfter = NULL;
	GetClosestFramesForTime( closestFrameBefore, closestFrameAfter, seconds );
	assert( closestFrameBefore );
	assert( closestFrameAfter );

	/// Check if both frames are identical; if so, simply return a copy of it / them
	if( closestFrameBefore == closestFrameAfter )
		return( *closestFrameBefore );

	/// Generate an interpolated frame based on where <seconds> falls between those two frames
	return( CreateInterpolatedFrame( *closestFrameBefore, *closestFrameAfter, seconds ) );
}


//-----------------------------------------------------------------------------------------------
// Name:		CreateInterpolatedFrame
// Class:		CameraKeyFramePath
//				
// Description:	Creates a new CameraKeyFrame object based on the best interpolated-approximation
//				of the CameraKeyFramePath when evaluated at <seconds> time.  NOTE:  This method
//				is called by CreateInterpolatedFrameForTime(), above, and does all the real
//				interpolation work.
//				
// Parameters:	const CameraKeyFrame& before - closest frame shy of <seconds>
//				const CameraKeyFrame& after - closest frame past <seconds>
//				const float seconds - time index used to interpolate between <before> and <after>
//				
// Returns:		const CameraKeyFrame - new, interpolated key frame
//				
//-----------------------------------------------------------------------------------------------
const CameraKeyFrame CameraKeyFramePath::CreateInterpolatedFrame( const CameraKeyFrame& before, const CameraKeyFrame& after, const float seconds )
{
	/// Calculate the amount of time between the two reference frames
	const float startTime = before.GetSeconds();
	const float endTime = after.GetSeconds();
	const float timeSpan = endTime - startTime;
	assert( timeSpan >= 0.0f );
	assert( seconds >= startTime );
	assert( seconds <= endTime );

	/// Calculation the interpolation fraction
	float fraction;
	if( timeSpan )
	{
		fraction = (seconds - startTime) / timeSpan;
	}
	else
	{
		fraction = 0.0f;
	}

	/// Create a new frame with interpolated values
	CameraKeyFrame lerped;
	lerped.Interpolate( before, after, fraction, true );
	return( lerped );
}



//-----------------------------------------------------------------------------------------------
// Name:		GetPathLengthInSeconds
// Class:		CameraKeyFramePath
//				
// Description:	Returns the total length of the camera path, in seconds.
//				
// Parameters:	void
//				
// Returns:		float - the total length of the camera path, in seconds.
//				
//-----------------------------------------------------------------------------------------------
float CameraKeyFramePath::GetPathLengthInSeconds( void )
{
	return( _totalSeconds );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		CameraPath
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLASS_DECLARATION( Class, CameraPath, NULL )
{
	{ NULL, NULL }
};

//-----------------------------------------------------------------------------------------------
// Name:		CameraPath	
// Class:		CameraPath
//				
// Description:	Construct from .KFC file
//				
// Parameters:	fileName	Name of the .KFC file to parse on construction of this object.
//				
// Returns:		n/a
//				
//-----------------------------------------------------------------------------------------------
CameraPath::CameraPath( const str& fileName )
:
_fileName( str( "" ) ),
_isLoaded( false ),
_keyFramePath( NULL ),
_yawPlaybackOffsetDegrees( 0.0f ),
_originPlaybackOffset( Vector( 0.0f, 0.0f, 0.0f ) )
{
	LoadKeyFramedCameraFile( fileName );
}


//-----------------------------------------------------------------------------------------------
// Name:		LoadKeyFramedCameraFile
// Class:		CameraPath
//				
// Description:	Loads a .KFC file and builds a dynamic array of camera key frames.
//				Any previously loaded data in this object is destroyed.
//				
// Parameters:	fileName - Name of the .KFC file to load and parse.
//				
// Returns:		bool - true upon success 
//				
//-----------------------------------------------------------------------------------------------
bool CameraPath::LoadKeyFramedCameraFile( const str& fileName )
{
	Script			cameraPathFile;
	const char*		token;
	bool			success; // generic parsing return-value variable
	str				filePathName;

	/// Unload the previous data (if any)
	Unload();

	/// Store the filename in the object for duplicate-load later on
	_fileName = fileName;

	/// Build the new file path name
	filePathName = "cams/";
	filePathName += fileName;
	filePathName += ".kfc";

	/// Load the file into a buffer (in the Script object)
	cameraPathFile.LoadFile( filePathName.c_str() );

	/// Parse each token in turn until no more remain
	while( cameraPathFile.TokenAvailable( true ) )
	{
		/// Read the next token-word and take the appropriate action
		token = cameraPathFile.GetToken( true );
		if( !stricmp( token, "Path" ) )
		{
			/// Read in the path info and its subsequent key frame data
			success = ParsePathInfo( cameraPathFile );
			if( !success )
				return( false );
		}
		else // unknown token
		{
			gi.Printf( "Unexpected token %s in camera path file %s.\n", token, _fileName.c_str() );
			cameraPathFile.error( "CameraPath::LoadKeyFramedCameraFile", "Unexpected token %s in camera path file %s.\n", token, _fileName.c_str() );
			return( false );
		}
	}

	/// Inform the object that it has been successfully loaded and is ready for use
	_isLoaded = true;
	return( true );
}


//-----------------------------------------------------------------------------------------------
// Name:		Unload
// Class:		CameraPath
//				
// Description:	Destroys the camera path data and marks the object as dirty (_isLoaded = false).
//				
// Parameters:	void
//				
// Returns:		void 
//				
//-----------------------------------------------------------------------------------------------
void CameraPath::Unload( void )
{
	if( IsLoaded() )
	{
		_isLoaded = false;
		delete _keyFramePath;
		_keyFramePath = NULL;
	}
}


//-----------------------------------------------------------------------------------------------
// Name:		ParsePathInfo
// Class:		CameraPath
//				
// Description:	Parses the Path #, allocates a new path object, and passes parsing duties onto it.
//				
// Parameters:	Script& cameraPathFile - parsing object
//				
// Returns:		bool - true upon success
//				
//-----------------------------------------------------------------------------------------------
bool CameraPath::ParsePathInfo( Script& cameraPathFile )
{
	/// Read the path number
	int pathNumber = cameraPathFile.GetInteger( false );
	assert( pathNumber == 0 ); // FIXME: this is only temporary, until we allow more than one path per .kfc

	/// Create a new key frame camera path and pass parsing duties on to it
	_keyFramePath = new CameraKeyFramePath;
	bool success = _keyFramePath->ParsePathInfo( cameraPathFile );
	return( success );
}


//-----------------------------------------------------------------------------------------------
// Name:		GetInterpolatedFrameForTime
// Class:		CameraPath
//				
// Description:	Returns a newly created temporary object; the resulting camera key frame when
//				the CameraPath is evaluated at <seconds>.
//				
// Parameters:	const float seconds - time index at which to evaluate the camera path
//				
// Returns:		const CameraKeyFrame - key frame created as a result of interpolation / evaluation
//				
//-----------------------------------------------------------------------------------------------
const CameraKeyFrame CameraPath::GetInterpolatedFrameForTime( const float seconds )
{
	CameraKeyFrame transformedKeyFrame( _keyFramePath->CreateInterpolatedFrameForTime( seconds ) );
	transformedKeyFrame.TransformToPlaybackOffsets( _yawPlaybackOffsetDegrees, _originPlaybackOffset );
	return( transformedKeyFrame );
}


//-----------------------------------------------------------------------------------------------
// Name:		SetPlaybackOffsets
// Class:		CameraPath
//				
// Description:	Tells a CameraPath object what offsets to use when reporting interpolations /
//				evaluations.  <yawOffset> is applied first to all positions and orientations;
//				<originOffset> is applied last (to positions only).
//				
// Parameters:	const float yawOffsetDegrees - yaw rotation to apply to all positions & orientations
//				const Vector& originOffset - offset translation applied to all positions, post-rotation
//				
// Returns:		void 
//				
//-----------------------------------------------------------------------------------------------
void CameraPath::SetPlaybackOffsets( const float yawOffsetDegrees, const Vector& originOffset )
{
	_yawPlaybackOffsetDegrees = yawOffsetDegrees;
	_originPlaybackOffset = originOffset;
}



//-----------------------------------------------------------------------------------------------
// Name:		GetPathLengthInSeconds
// Class:		CameraPath
//				
// Description:	Returns the total length of the camera path, in seconds.
//				
// Parameters:	void
//				
// Returns:		float - the total length of the camera path, in seconds 
//				
//-----------------------------------------------------------------------------------------------
float CameraPath::GetPathLengthInSeconds( void )
{
	return( _keyFramePath->GetPathLengthInSeconds() );
}


