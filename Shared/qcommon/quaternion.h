//-----------------------------------------------------------------------------
// Quaternion.h
// 
// Author:  Squirrel Eiserloh
// 
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
// 
// DESCRIPTION:
//	Header for (and inline implementation of) a basic Quaternion class.
//	This should ultimately be part of a yet-to-be-written general-purpose
//	Rotation class at some point, time-permitting.
// 
#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#include <math.h>
#include "vector.h"
//#include "Matrix.h"

#define UNUSED_ARG (void)



//---------------------------------------------------------------------------
// Quaternion
// 
// ...in the form (x, y, z, w) such that  q = xi + yj + zk + w,  where:
// 
// i x i = -1			i x j = k			j x i = -k
// j x j = -1	and		j x k = i	and		k x j = -i
// k = k = -1			k x i = j			i x k = -j
// 
// Note that i, j, and k are each imaginary numbers of different "flavors"
//	representing mutually perpendicular unit vectors defining 3 of the
//	4 axes in quaternion 4-space.  (The fourth axis is the real unit, 1.)
// 
// In vector form, the quaternion would look like (s, v), where s is a
//	scalar (equal to w) and v is a vector (x, y, z) in quaternion 4-space
//	(giving position along the base unit axis vectors i, j, and k,
//	respectively).
// 
// The most useful quaternions are those that are of unit length
//	(|q| = 1, or x*x + y*y + z*z + w*w = 1).  This defines a set
//	of points which make up a 4-dimensional "unit hypersphere",
//	across the surface of which we will be interpolating.
// 
// Note that Quaternion multiplication involves the vector cross
//	product (v1 x v2), so it is NOT COMMUTATIVE.  This means that
//	q1 x q2 != q2 x q1.  (Then again, matrix multiplication isn't
//	commutative either, so suck it down.)
// 
// "lhs" and "rhs" mean "left hand side" and "right hand side" for
//	operator arguments, respectively.
//---------------------------------------------------------------------------
class Quaternion
{
	//---------------------------------------------------------------------------
	// Member variables
	//---------------------------------------------------------------------------
private:
	float _x; // coefficient for the i imaginary term
	float _y; // coefficient for the j imaginary term
	float _z; // coefficient for the k imaginary term
	float _w; // coefficient for the real term

	//---------------------------------------------------------------------------
	// Accessors / Mutators
	//---------------------------------------------------------------------------
protected:

	//---------------------------------------------------------------------------
	// Implementation Methods
	//---------------------------------------------------------------------------
private:

	//---------------------------------------------------------------------------
	// Construction / Destruction
	//---------------------------------------------------------------------------
public:
	~Quaternion();
	explicit Quaternion(); // default constructor
	Quaternion( const Quaternion& rhs ); // copy constructor
	explicit Quaternion( const float x, const float y, const float z, const float w );
	explicit Quaternion( const float w, const Vector& vec );
	explicit Quaternion( const Vector& eulerAngles );
//	explicit Quaternion( const Matrix3x3& rotationMatrix );
//	explicit Quaternion( const Matrix4x4& transformMatrix );

	//---------------------------------------------------------------------------
	// Interface Methods
	//---------------------------------------------------------------------------
public:
	float CalcLength( void ) const;
	float CalcLengthSquared( void ) const;
	float Normalize( void );

	void SetFromSV( const float w, const Vector& vec );
	void SetFromXYZW( const float x, const float y, const float z, const float w );
	void SetFromEuler( const Vector& eulerAngles );
//	void SetFromMatrix3x3( const Matrix3x3& rotationMatrix );
//	void SetFromMatrix4x4( const Matrix4x4& transformMatrix );

	void GetToSV( float& w, Vector& vec ) const;
	void GetToXYZW( float& x, float& y, float& z, float& w ) const;
	void GetToEuler( Vector& eulerAngles ) const;
//	void GetToMatrix3x3( Matrix3x3& rotationMatrix ) const;
//	void GetToMatrix4x4( Matrix4x4& transformMatrix ) const;

	/// Self-modifying operators
	const Quaternion& operator = ( const Quaternion& rhs );
	const Quaternion& operator += ( const Quaternion& rhs );
	const Quaternion& operator -= ( const Quaternion& rhs );
	const Quaternion& operator *= ( const float scale );
	const Quaternion& operator *= ( const Quaternion& rhs );
	const Quaternion& operator /= ( const float invScale );
	const Quaternion operator - () const;

	/// Construction operators
	const Quaternion operator + ( const Quaternion& rhs ) const;
	const Quaternion operator - ( const Quaternion& rhs ) const;

	bool operator == ( const Quaternion& rhs ) const;
	bool operator != ( const Quaternion& rhs ) const;

private:
	const Quaternion operator * ( const float scale ) const; // multiply-by-right-scalar forbidden; use (float, Quaternion&) version instead
};
//---------------------------------------------------------------------------
// External Operators & Functions
//---------------------------------------------------------------------------
const Quaternion operator * ( const Quaternion& lhs, const Quaternion& rhs );
const Quaternion operator * ( const float scale, const Quaternion& rhs );
const Quaternion operator / ( const Quaternion& lhs, const float invScale );
const Quaternion CalcSlerp( const Quaternion& q1, const Quaternion& q2, const float fraction );
const Quaternion CalcLerp( const Quaternion& q1, const Quaternion& q2, const float q2Fraction );
const Quaternion CalcNoLerp( const Quaternion& q1, const Quaternion& q2, const float q2Fraction );
float QuaternionDotProduct( const Quaternion& q1, const Quaternion& q2 );



//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
inline Quaternion::~Quaternion()
{
}


//---------------------------------------------------------------------------
// Default constructor
//---------------------------------------------------------------------------
inline Quaternion::Quaternion()
{
	// Do nothing; this should be used only by static array declarations
}


//---------------------------------------------------------------------------
// Copy Constructor( Quaternion )
//---------------------------------------------------------------------------
inline Quaternion::Quaternion( const Quaternion& rhs )
{
	SetFromXYZW( rhs._x, rhs._y, rhs._z, rhs._w );
}


//---------------------------------------------------------------------------
// Constructor( float, float, float, float )
//---------------------------------------------------------------------------
inline Quaternion::Quaternion( const float x, const float y, const float z, const float w )
{
	SetFromXYZW( x, y, z, w );
}


//---------------------------------------------------------------------------
// Constructor( float, Vector )
//---------------------------------------------------------------------------
inline Quaternion::Quaternion( const float w, const Vector& vec )
{
	SetFromSV( w, vec );
}


//---------------------------------------------------------------------------
// Constructor( Vector )
//---------------------------------------------------------------------------
inline Quaternion::Quaternion( const Vector& eulerAngles )
{
	SetFromEuler( eulerAngles );
}


//---------------------------------------------------------------------------
// Constructor( Matrix3x3 )
//---------------------------------------------------------------------------
/*
inline Quaternion::Quaternion( const Matrix3x3& rotationMatrix )
{
	SetFromMatrix3x3( rotationMatrix );
}
*/

//---------------------------------------------------------------------------
// Constructor( Matrix4x4 )
//---------------------------------------------------------------------------
/*
inline Quaternion::Quaternion( const Matrix4x4& transformMatrix )
{
	SetFromMatrix4x4( transformMatrix );
}
*/

//---------------------------------------------------------------------------
// CalcLength()
//---------------------------------------------------------------------------
inline float Quaternion::CalcLength( void ) const
{
	float length = (float) sqrt( CalcLengthSquared() );
	return( length );
}


//---------------------------------------------------------------------------
// CalcLengthSquared()
//---------------------------------------------------------------------------
inline float Quaternion::CalcLengthSquared( void ) const
{
	float lengthSquared;
	lengthSquared = (_x * _x) + (_y * _y) + (_z * _z) + (_w * _w);
	return( lengthSquared );
}


//---------------------------------------------------------------------------
// Normalize
//---------------------------------------------------------------------------
inline float Quaternion::Normalize( void )
{
	/// Get the length of the quaternion 4d vector
	float length = CalcLength();
	if( !length )
		return( 0.0f );

	/// Divide each component by <length>
	*this /= length;
	return( length );
}


//---------------------------------------------------------------------------
// SetFromSV
//---------------------------------------------------------------------------
inline void Quaternion::SetFromSV( const float w, const Vector& vec )
{
	SetFromXYZW( vec.x, vec.y, vec.z, w );
}


//---------------------------------------------------------------------------
// SetFromXYZW
//---------------------------------------------------------------------------
inline void Quaternion::SetFromXYZW( const float x, const float y, const float z, const float w )
{
	_x = x;
	_y = y;
	_z = z;
	_w = w;
}


//---------------------------------------------------------------------------
// SetFromEuler
//---------------------------------------------------------------------------
inline void Quaternion::SetFromEuler( const Vector& eulerAngles )
{
	// FIXME: THIS IS TEMPORARY HACKED STUFF FOR PROOF OF CONCEPT ONLY!!!
	float quat[ 4 ];
	vec3_t eulerAng;

	eulerAngles.copyTo( eulerAng );
	EulerToQuat( eulerAng, quat );

	_x = quat[ 0 ];
	_y = quat[ 1 ];
	_z = quat[ 2 ];
	_w = quat[ 3 ];
}


//---------------------------------------------------------------------------
// SetFromMatrix3x3
//---------------------------------------------------------------------------
/*
inline void Quaternion::SetFromMatrix3x3( const Matrix3x3& rotationMatrix )
{
	// FIXME: stub
	UNUSED_ARG rotationMatrix;
}
*/

//---------------------------------------------------------------------------
// SetFromMatrix4x4
//---------------------------------------------------------------------------
/*
inline void Quaternion::SetFromMatrix4x4( const Matrix4x4& transformMatrix )
{
	// FIXME: stub
	UNUSED_ARG transformMatrix;
}
*/

//---------------------------------------------------------------------------
// GetToSV
//---------------------------------------------------------------------------
inline void Quaternion::GetToSV( float& w, Vector& vec ) const
{
	GetToXYZW( vec.x, vec.y, vec.z, w );
}


//---------------------------------------------------------------------------
// GetToXYZW
//---------------------------------------------------------------------------
inline void Quaternion::GetToXYZW( float& x, float& y, float& z, float& w ) const
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}


//---------------------------------------------------------------------------
// GetToEuler
//---------------------------------------------------------------------------
inline void Quaternion::GetToEuler( Vector& eulerAngles ) const
{
	// FIXME: THIS IS TEMPORARY HACKED STUFF FOR PROOF OF CONCEPT ONLY!!!
	float matrix[ 3 ][ 3 ];
	float quat[ 4 ];
	vec3_t eulerAng;

	quat[ 0 ] = _x;
	quat[ 1 ] = _y;
	quat[ 2 ] = _z;
	quat[ 3 ] = _w;

	QuatToMat( quat, matrix );
	MatrixToEulerAngles( matrix, eulerAng );
	eulerAngles.setXYZ( eulerAng[0], eulerAng[1], eulerAng[2] );
}


//---------------------------------------------------------------------------
// GetToMatrix3x3
//---------------------------------------------------------------------------
/*
inline void Quaternion::GetToMatrix3x3( Matrix3x3& rotationMatrix ) const
{
	// FIXME: stub
	UNUSED_ARG rotationMatrix;
}
*/

//---------------------------------------------------------------------------
// GetToMatrix4x4
//---------------------------------------------------------------------------
/*
inline void Quaternion::GetToMatrix4x4( Matrix4x4& transformMatrix ) const
{
	// FIXME: stub
	UNUSED_ARG transformMatrix;
}
*/

//---------------------------------------------------------------------------
// operator = (Quaternion)
//---------------------------------------------------------------------------
inline const Quaternion& Quaternion::operator = ( const Quaternion& rhs )
{
	if( this == &rhs )
		return *this;

	_x = rhs._x;
	_y = rhs._y;
	_z = rhs._z;
	_w = rhs._w;
	return *this;
}


//---------------------------------------------------------------------------
// operator += (Quaternion)
//---------------------------------------------------------------------------
inline const Quaternion& Quaternion::operator += ( const Quaternion& rhs )
{
	_x += rhs._x;
	_y += rhs._y;
	_z += rhs._z;
	_w += rhs._w;
	return *this;
}


//---------------------------------------------------------------------------
// operator -= (Quaternion)
//---------------------------------------------------------------------------
inline const Quaternion& Quaternion::operator -= ( const Quaternion& rhs )
{
	_x -= rhs._x;
	_y -= rhs._y;
	_z -= rhs._z;
	_w -= rhs._w;
	return *this;
}


//---------------------------------------------------------------------------
// operator *= (float)
//---------------------------------------------------------------------------
inline const Quaternion& Quaternion::operator *= ( const float scale )
{
	_x *= scale;
	_y *= scale;
	_z *= scale;
	_w *= scale;
	return *this;
}


//---------------------------------------------------------------------------
// operator *= (Quaternion)
// 
// Quaternion multiplication is NOT COMMUTATIVE, and is defined as
//	follows, where "s" is the scalar component (w) and "v" is the vector
//	component (x,y,z) (and 'x' means "3d vector cross product", '|' means
//	"3d vector dot product"):
// 
// q1 * q2 = q3 = (s3, v3)     !=     q2 * q1
// 
// s3 = (s1 * s2) - (v1 | v2)
// v3 = (s1 * v2) + (s2 * v1) + (v1 x v2)
// 
// i.e.  Q(s1, v1)  *  Q(s2, v2)  =  Q(s3, v3)
//---------------------------------------------------------------------------
inline const Quaternion& Quaternion::operator *= ( const Quaternion& rhs )
{
	float s1, s2, s3; // see above comment
	Vector v1, v2, v3; // see above comment

	/// Get both quaternions into (s,v) form
	GetToSV( s1, v1 );
	rhs.GetToSV( s2, v2 );

	/// Calculate the new scalar term (s3)
	s3 = (s1 * s2) - Vector::Dot( v1, v2 );

	/// Calculate the new vector term (v3)
	Vector v1CrossV2; // temp variable for cross-product result, since our Vector class sucks
	v1CrossV2.CrossProduct( v1, v2 ); // our vector class sucks
	v3 = (s1 * v2) + (s2 * v1) + v1CrossV2;

	/// Set the new scalar and vector terms and return this
	SetFromSV( s3, v3 );
	return *this;
}


//---------------------------------------------------------------------------
// operator /= (float)
//---------------------------------------------------------------------------
inline const Quaternion& Quaternion::operator /= ( const float invScale )
{
	if( invScale )
	{
		_x /= invScale;
		_y /= invScale;
		_z /= invScale;
		_w /= invScale;
	}
	else
	{
		SetFromXYZW( 0.0f, 0.0f, 0.0f, 0.0f );
	}

	return *this;
}


//---------------------------------------------------------------------------
// operator - (Quaternion) : unary minus
//---------------------------------------------------------------------------
inline const Quaternion Quaternion::operator - () const
{
	return Quaternion( -_x, -_y, -_z, -_w );
}


//---------------------------------------------------------------------------
// operator == (Quaternion, Quaternion)
//---------------------------------------------------------------------------
//inline bool operator == ( const Quaternion& lhs, const Quaternion& rhs )
inline bool Quaternion::operator == ( const Quaternion& rhs ) const
{
	if( _x != rhs._x )
		return false;

	if( _y != rhs._y )
		return false;

	if( _z != rhs._z )
		return false;

	if( _w != rhs._w )
		return false;

	return true;
}


//---------------------------------------------------------------------------
// operator != (Quaternion, Quaternion)
//---------------------------------------------------------------------------
inline bool Quaternion::operator != ( const Quaternion& rhs ) const
{
	return( !(*this == rhs) );
}


//---------------------------------------------------------------------------
// operator - (Quaternion, Quaternion)
//---------------------------------------------------------------------------
inline const Quaternion Quaternion::operator - ( const Quaternion& rhs ) const
{
	Quaternion difference( *this );
	difference -= rhs;
	return( difference );
}

//---------------------------------------------------------------------------
// operator + (Quaternion, Quaternion)
//---------------------------------------------------------------------------
inline const Quaternion Quaternion::operator + ( const Quaternion& rhs ) const
//Quaternion Quaternion::operator + ( const Quaternion& rhs ) const
{
	Quaternion sum( *this );
	sum += rhs;
	return( sum );
}


//---------------------------------------------------------------------------
// general operator * (Quaternion, Quaternion)
//---------------------------------------------------------------------------
inline const Quaternion operator * ( const Quaternion& lhs, const Quaternion& rhs )
{
	Quaternion product( lhs );
	product *= rhs;
	return product;
}


//---------------------------------------------------------------------------
// general operator * (float, Quaternion)
//---------------------------------------------------------------------------
inline const Quaternion operator * ( const float scale, const Quaternion& rhs )
{
	Quaternion scaled( rhs );
	scaled *= scale;
	return scaled;
}


//---------------------------------------------------------------------------
// general operator / (Quaternion, float)
//---------------------------------------------------------------------------
inline const Quaternion operator / ( const Quaternion& lhs, const float invScale )
{
	Quaternion scaled( lhs );
	if( invScale )
	{
		scaled /= invScale;
	}
	else
	{
		scaled *= 0.0f;
	}
	return scaled;
}


//---------------------------------------------------------------------------
// general CalcLerp (Quaternion, Quaternion, float)
// 
// Performs a hypervector linear interpolation - or "lerp" - of two
//	Quaternions and returns the resulting (newly constructed) Quaternion.
// 
//	<q2Fraction> is a value in the range [0,1] representing how much of
//	<q2> to use in the interpolation; q1Fraction = 1-<q2Fraction> is the
//	weighting given to <q1>.
// 
// NOTE: This interpolation is faster, but less accurate, than CalcSlerp().
//	Use CalcSlerp() if the error incurred from CalcLerp() is noticeable.
// 
// For optimization purposes, CalcLerp() assumes <q1> and <q2> are
//	already normalized.
//---------------------------------------------------------------------------
inline const Quaternion CalcLerp( const Quaternion& q1, const Quaternion& q2, const float q2Fraction )
{
	const float q1Fraction = 1.0f - q2Fraction;

	/// Check if <q1> and <q2> are the same quaternion
	if( &q1 == &q2 )
		return Quaternion( q1 );

	/// Check if <q1> and <q2> are data-identical
	if( q1 == q2 )
		return Quaternion( q1 );

	/// Check if <q2Fraction> is at (or beyond) a boundary condition
	if( q2Fraction <= 0.0f )
		return Quaternion( q1 );
	if( q2Fraction >= 1.0f )
		return Quaternion( q2 );

	/// Create a new quaternion which represents the weighted average of <q1> and <q2>
	Quaternion lerped( (q1Fraction * q1) + (q2Fraction * q2) );
	float lerpedLength = lerped.Normalize();

	/// Check if the 4d vectors added up to 0.0 (degenerate case!)
	if( !lerpedLength )
	{
		/// Return whichever of <q1> or <q2> the parameter is currently closest to
		return CalcNoLerp( q1, q2, q2Fraction );
	}
		
	return lerped;
}


//---------------------------------------------------------------------------
// general CalcSlerp (Quaternion, Quaternion, float)
// 
// Performs a (hyper)spherical linear interpolation - or "slerp" - of two
//	Quaternions and returns the resulting (newly constructed) Quaternion.
// 
//	<q2Fraction> is a value in the range [0,1] representing how much of
//	<q2> to use in the interpolation; q1Fraction = 1-<q2Fraction> is the
//	weighting given to <q1>.
// 
// For optimization purposes, CalcSlerp() assumes <q1> and <q2> are
//	already normalized.
//---------------------------------------------------------------------------
inline const Quaternion CalcSlerp( const Quaternion& q1, const Quaternion& q2, const float q2Fraction )
{
	static const float SIN_OMEGA_EPSILON = 0.00001f;
	const float q1Fraction = 1.0f - q2Fraction;

	/// Check if <q1> and <q2> are one and the same
	if( &q1 == &q2 )
		return Quaternion( q1 );

	/// Check if <q1> and <q2> are data-identical
	if( q1 == q2 )
		return Quaternion( q1 );

	/// Check if <q2Fraction> is at (or beyond) a boundary condition
	if( q2Fraction <= 0.0f )
		return Quaternion( q1 );
	if( q2Fraction >= 1.0f )
		return Quaternion( q2 );

	/// Calculate <cosOmega>, the dot product (cosine) of the angle between the two 4d hypervectors
	float cosOmega = QuaternionDotProduct( q1, q2 );

	/// Create a copy of q2 and invert it if <cosOmega> is negative (i.e. it's on the opposite side of the hypersphere)
	Quaternion q2copy( q2 );
	if( cosOmega < 0.0f )
	{
		/// Mirror the hypervector (and, therefore, the dot product) to be on the same side of the hypersphere as <q1>
		cosOmega = -cosOmega;
		q2copy *= -1.0f;
	}

	/// Check if either <q1> or <q2> was not normalized
	if( cosOmega > 1.0f )
	{
		/// FIXME: One or both of <q1> or <q2> were evidently not normalized; this should be an error condition!
		/// Return whichever of <q1> or <q2> the parameter is currently closest to
		return CalcNoLerp( q1, q2, q2Fraction );
	}

	/// Check if <q1> and <q2> are close enough to use linear interpolation instead
	if( (1.0f - cosOmega) > SIN_OMEGA_EPSILON )
	{
		return CalcLerp( q1, q2, q2Fraction );
	}

	/// Check if <q1> and <q2> are nearly opposite on the hypersphere
	if( (cosOmega + 1.0) < SIN_OMEGA_EPSILON )
	{
		// FIXME: how should this case be handled?
		// Watt & Watt does some voodoo-math which is clearly incorrect...
		return CalcNoLerp( q1, q2, q2Fraction );
	}

	/// Calculate <omega>, the angle between <q1> and <q2>, based on the dot product (cosine) between <q1> and <q2>
	const float omega = acos( cosOmega );
	const float sinOmega = sin( omega );

	/// Check if <sinOmega> is prohibitively small (since it will end up in the denominator later)
	if( (sinOmega > -SIN_OMEGA_EPSILON) && (sinOmega < SIN_OMEGA_EPSILON) )
	{
		/// Return whichever of <q1> or <q2> the parameter is currently closest to
		return CalcNoLerp( q1, q2, q2Fraction );
	}

	/// Build a new quaternion, <slerped>, which uses normal (hyper)spherical linear interpolation
	Quaternion slerped;
	float scale1 = sin( q1Fraction * omega ) / sinOmega;
	float scale2 = sin( q2Fraction * omega ) / sinOmega;
	slerped = (scale1 * q1) + (scale2 * q2);

	// FIXME: does <slerped> need to be renormalized at this point??
	// (yes, but only because of floating-point drift, and it's the caller's reponsibility to do this)

	return slerped;
}


//---------------------------------------------------------------------------
// general CalcNoLerp (Quaternion, Quaternion, float)
// 
// This is a fake / stub quaternion interpolation function; it simply
//	returns a copy of <q1> or <q2> based on which one <q2Fraction>
//	indicates it is closer to.
//---------------------------------------------------------------------------
inline const Quaternion CalcNoLerp( const Quaternion& q1, const Quaternion& q2, const float q2Fraction )
{
	if( q2Fraction < 0.5f )
	{
		return Quaternion( q1 );
	}
	else
	{
		return Quaternion( q2 );
	}
}


//---------------------------------------------------------------------------
// QuaternionDotProduct
// 
// Calculates the dot product of <q1> and <q2> where both quaternions are
//	taken as (probably unit) vectors in fourspace.
//---------------------------------------------------------------------------
inline float QuaternionDotProduct( const Quaternion& q1, const Quaternion& q2 )
{
	float x1, y1, z1, w1;
	float x2, y2, z2, w2;

	q1.GetToXYZW( x1, y1, z1, w1 );
	q2.GetToXYZW( x2, y2, z2, w2 );

	float dotProduct = (x1*x2) + (y1*y2) + (z1*z2) + (w1*w1);
	return dotProduct;
}



#endif // _QUATERNION_H_
