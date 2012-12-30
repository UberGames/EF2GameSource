//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/bit_vector.h                                  $
// $Revision:: 4                                                              $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 8:53a                                                 $
//
// Copyright (C) 2001 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// This class is to used to manage arrays of bits.


//=======================================
//Forward Declarations
//=======================================

class BitVector;
class BitReference;

#ifndef __BIT_VECTOR_H__
#define __BIT_VECTOR_H__

//================================================================
//
// Class:			BitVector
//
// Description:	The BitVector class is basically a class made to simplify 
//						working with an array/vector of bits.
//
//================================================================

class BitVector 
   {
	public:

		// Constructors / destructors

							BitVector( unsigned int numBits, bool value = false );
							BitVector( const BitVector &old );
							~BitVector();

		// Bit manipulation

		void				SetBit( unsigned int index, bool value = true );
		void				ClearBit( unsigned int index );
		void				FlipBit( unsigned int index );
		bool				GetBit( unsigned int index ) const;

		// Bit array manipulation

		void				Set( void );
		void				Clear( void );
		void				Flip( void );

		// Bit range manipulation

		void				SetRange( unsigned int first_index, unsigned int last_index, bool value = true );
		void				ClearRange( unsigned int first_index, unsigned int last_index );

		// Info

		unsigned int	BitSize( void ) const;
		unsigned int	ByteSize( void ) const;
		unsigned int	Count( void ) const;
		bool				Any( void ) const;
		bool				None( void ) const;

		// Operators

		bool				operator[]( unsigned int index ) const;
		BitReference	operator[]( unsigned int index );
		bool				operator== ( const BitVector &other );
		bool				operator!= ( const BitVector &other );
		void				operator|= ( const BitVector &other );
		void				operator&= ( const BitVector &other );
		BitVector		operator~ ( void );

	private:

		// Data

		unsigned char	*_bytes;
		unsigned int	_numBytes;
		unsigned int	_numBits;

		// Friends

		friend BitVector operator| ( const BitVector &left, const BitVector &right );
		friend BitVector operator& ( const BitVector &left, const BitVector &right );

		friend BitReference;
	};

//================================================================
//
// Class:			BitReference
//
// Description:	The BitReference class is a go between for the BitVector
//						class to allow the operator[] to work exactly as you
//						would expect it to for an array.  This makes it possible
//						to do the following:
//						bits[ 2 ] = true;
//						bit = bits[ 2 ];
//						bits[ 2 ] = other_bits[ 5 ];
//
//================================================================

class BitReference 
	{
	public:

		// Operators

		BitReference&	operator= ( bool b );
		BitReference&	operator= ( const BitReference &other );
							operator bool() const;

	private:

		// Data

		BitVector		*_vector;
		unsigned int	_index;

		// Setup only called by BitVector class

		void				Setup( BitVector *vector, unsigned int index );

		// Friends

		friend BitVector;
   };

//================================================================
// Name:				BitVector
// Class:			BitVector
//
// Description:	Constructor
//
// Parameters:		unsigned int numBits			- number of bits in the array
//						bool value						- value to default all bits to (defaults to false)
//
// Returns:			None
//================================================================

BitVector::BitVector( unsigned int numBits, bool value )
   {
	// Setup data

	_numBits = numBits;
	_numBytes = ( _numBits + 7 ) >> 3;
	_bytes = new unsigned char[ _numBytes ];

	// Initialize all bits

	if ( value )
		Set();
	else
		Clear();
	}

//================================================================
// Name:				BitVector
// Class:			BitVector
//
// Description:	Constructor (copy)
//
// Parameters:		const BitVector &old			- bit array to copy
//
// Returns:			None
//================================================================

BitVector::BitVector( const BitVector &old )
   {
	int i;

	// Setup data

	_numBits = old._numBits;
	_numBytes = old._numBytes;
	_bytes = new unsigned char[ _numBytes ];

	// Copy bits

	for ( i = 0 ; i < _numBytes ; i++ )
		_bytes[ i ] = old._bytes[ i ];
	}

//================================================================
// Name:				~BitVector
// Class:			BitVector
//
// Description:	Deconstructor
//
// Parameters:		None
//
// Returns:			None
//================================================================

BitVector::~BitVector()
   {
	// Free the bit array

	delete[] _bytes;
	}

//================================================================
// Name:				SetBit
// Class:			BitVector
//
// Description:	Sets the specified bit with the value 
//
// Parameters:		unsigned int index			- index of bit
//						bool value						- value of bit to set (defaults to true)
//
// Returns:			None
//================================================================

inline void BitVector::SetBit( unsigned int index, bool value )
   {
	if ( index < _numBits )
		{
		if ( value )
			_bytes[ index >> 3 ] |= ( 1 << ( index & 7 ) );				// Set the bit
		else
			_bytes[ index >> 3 ] &= ~( 1 << ( index & 7 ) );			// Clear the bit
		}
	}

//================================================================
// Name:				ClearBit
// Class:			BitVector
//
// Description:	Clears the specified bit (sets it to 0)
//
// Parameters:		unsigned int index			- index of bit
//
// Returns:			None
//================================================================

inline void BitVector::ClearBit( unsigned int index )
	{
	SetBit( index, false );
	}

//================================================================
// Name:				FlipBit
// Class:			BitVector
//
// Description:	Flips the specified bit
//
// Parameters:		unsigned int index			- index of bit
//
// Returns:			None
//================================================================

inline void BitVector::FlipBit( unsigned int index )
	{
	bool value;

	value = GetBit( index );
	SetBit( index, !value );
	}

//================================================================
// Name:				GetBit
// Class:			BitVector
//
// Description:	Returns the specified bit
//
// Parameters:		unsigned int index			- index of bit
//
// Returns:			None
//================================================================

inline bool BitVector::GetBit( unsigned int index ) const 
	{
	if ( index < _numBits )
		return ( ( _bytes[ index >> 3 ] & ( 1 << ( index & 7 ) ) ) != 0 );
	else
		return false;
	}

//================================================================
// Name:				Set
// Class:			BitVector
//
// Description:	Sets every bit in bit array to 1
//
// Parameters:		None
//
// Returns:			None
//================================================================

inline void BitVector::Set( void )
	{
	unsigned int i;

	for ( i = 0 ; i < _numBytes ; i++ )
		_bytes[ i ] = (unsigned char)0xFF;
	}

//================================================================
// Name:				Clear
// Class:			BitVector
//
// Description:	Clears every bit in array (sets to 0)
//
// Parameters:		None
//
// Returns:			None
//================================================================

inline void BitVector::Clear( void )
	{
	unsigned int i;

	for ( i = 0 ; i < _numBytes ; i++ )
		_bytes[ i ] = 0;
	}

//================================================================
// Name:				Flip
// Class:			BitVector
//
// Description:	Flips every bit in array
//
// Parameters:		None
//
// Returns:			None
//================================================================

inline void BitVector::Flip( void )
	{
	unsigned int i;

	for ( i = 0 ; i < _numBytes ; i++ )
		{
		_bytes[ i ] = ~_bytes[ i ];
		}
	}

//================================================================
// Name:				SetRange
// Class:			BitVector
//
// Description:	Sets the specified range of bits to the value
//
// Parameters:		unsigned int first_index	- index of first bit
//						unsigned int last_index		- index of last bit
//						bool value						- value to set bits to (defaults to true)
//
// Returns:			None
//================================================================

inline void BitVector::SetRange( unsigned int first_index, unsigned int last_index, bool value )
	{
	unsigned int i;

	for ( i = first_index ; i < last_index ; i++ )
		SetBit( i, value );
	}

//================================================================
// Name:				ClearRange
// Class:			BitVector
//
// Description:	Clears a range of bits (sets to 0)
//
// Parameters:		unsigned int first_index	- index of first bit
//						unsigned int last_index		- index of last bit
//
// Returns:			None
//================================================================

inline void BitVector::ClearRange( unsigned int first_index, unsigned int last_index )
	{
	SetRange( first_index, last_index, false );
	}

//================================================================
// Name:				BitSize
// Class:			BitVector
//
// Description:	Returns the number of bits
//
// Parameters:		None
//
// Returns:			unsigned int					- number of bits
//================================================================

inline unsigned int BitVector::BitSize( void ) const
	{
	return _numBits;
	}

//================================================================
// Name:				ByteSize
// Class:			BitVector
//
// Description:	Returns the number of bytes in the array
//
// Parameters:		None
//
// Returns:			unsigned int					- number of bytes in array
//================================================================

inline unsigned int BitVector::ByteSize( void ) const
	{
	return _numBytes;
	}

//================================================================
// Name:				Count
// Class:			BitVector
//
// Description:	Returns the number of bits set to 1
//
// Parameters:		None
//
// Returns:			unsigned int					- number of bits set to 1
//================================================================

inline unsigned int BitVector::Count( void ) const
	{
	unsigned int i;
	unsigned count;

	count = 0;

	for ( i = 0 ; i < _numBits ; i++ )
		{
		if ( GetBit( i ) )
			count++;
		}

	return count;
	}

//================================================================
// Name:				Any
// Class:			BitVector
//
// Description:	Returns whether or not any bits are set to 1
//
// Parameters:		None
//
// Returns:			bool								- if any bits are set to 1
//================================================================

inline bool BitVector::Any( void ) const
	{
	unsigned int i;

	for ( i = 0 ; i < _numBits ; i++ )
		{
		if ( GetBit( i ) )
			return true;
		}

	return false;
	}

//================================================================
// Name:				None
// Class:			BitVector
//
// Description:	Returns whether or not none of the bits are set to 1
//
// Parameters:		None
//
// Returns:			bool								- if none of the bits are set to 1
//================================================================

inline bool BitVector::None( void ) const
	{
	return !Any();
	}

//================================================================
// Name:				operator[]
// Class:			BitVector
//
// Description:	Returns the referenced bit
//
// Parameters:		unsigned int index			- index of bit
//
// Returns:			bool								- bit
//================================================================

inline bool BitVector::operator[]( unsigned int index ) const 
	{
	return GetBit( index );
	}

//================================================================
// Name:				operator[]
// Class:			BitVector
//
// Description:	Returns a reference to the bit in question
//
// Parameters:		unsigned int index			- index of bit
//
// Returns:			BitReference					- reference of bit
//================================================================

inline BitReference BitVector::operator[]( unsigned int index )
	{
	BitReference bitRef;

	bitRef.Setup( this, index );
	return bitRef;
	}

//================================================================
// Name:				operator==
// Class:			BitVector
//
// Description:	Returns whether or not this bitvector equals another
//
// Parameters:		const BitVector &other		- the other BitVector to compare to
//
// Returns:			bool								- if equal
//================================================================

inline bool BitVector::operator== ( const BitVector &other )
	{
	unsigned int i;

	if ( BitSize() != other.BitSize() )
		return false;

	for ( i = 0 ; i < _numBytes ; i++ )
		{
		if ( _bytes[ i ] != other._bytes[ i ] )
			return false;
		}

	return true;
	}

//================================================================
// Name:				operator!=
// Class:			BitVector
//
// Description:	Returns whether or not this bitvector doesn't equal another
//
// Parameters:		const BitVector &other		- the other BitVector to compare to
//
// Returns:			bool								- if not equal
//================================================================

inline bool BitVector::operator!= ( const BitVector &other )
	{
	unsigned int i;

	if ( BitSize() != other.BitSize() )
		return true;

	for ( i = 0 ; i < _numBytes ; i++ )
		{
		if ( _bytes[ i ] != other._bytes[ i ] )
			return true;
		}

	return false;
	}

//================================================================
// Name:				operator|=
// Class:			BitVector
//
// Description:	Ors together this BitVector with the specified BitVector (bit or)
//
// Parameters:		const BitVector &other		- the other BitVector to or with
//
// Returns:			None
//================================================================

inline void BitVector::operator|= ( const BitVector &other )
	{
	unsigned int i;

	if ( BitSize() != other.BitSize() )
		return;

	for ( i = 0 ; i < _numBytes ; i++ )
		{
		_bytes[ i ] |= other._bytes[ i ];
		}
	}

//================================================================
// Name:				operator&=
// Class:			BitVector
//
// Description:	Ands together this BitVector with the specified BitVector (bit and)
//
// Parameters:		const BitVector &other		- the other BitVector to and with
//
// Returns:			None
//================================================================

inline void BitVector::operator&= ( const BitVector &other )
	{
	unsigned int i;

	if ( BitSize() != other.BitSize() )
		return;

	for ( i = 0 ; i < _numBytes ; i++ )
		{
		_bytes[ i ] &= other._bytes[ i ];
		}
	}

//================================================================
// Name:				operator~
// Class:			BitVector
//
// Description:	Returns a not'ed version of the bitarray
//
// Parameters:		None
//
// Returns:			BitVector						- bitwise xor'ed version
//================================================================

inline BitVector BitVector::operator~ ( void )
	{
	BitVector temp_bits( _numBits );
	unsigned int i;

	for ( i = 0 ; i < _numBytes ; i++ )
		{
		temp_bits._bytes[ i ] = ~_bytes[ i ];
		}

	return temp_bits;
	}

//================================================================
// Name:				operator|
// Class:			
//
// Description:	Ors together 2 BitVectors
//
// Parameters:		const BitVector &left		- BitVector on left of |
//						const BitVector &right		- BitVector on right of |
//
// Returns:			None
//================================================================

inline BitVector operator| ( const BitVector &left, const BitVector &right )
	{
	BitVector temp_bits( left );

	temp_bits |= right;
	return temp_bits;
	}

//================================================================
// Name:				operator&
// Class:			
//
// Description:	Ands together 2 BitVectors
//
// Parameters:		const BitVector &left		- BitVector on left of &
//						const BitVector &right		- BitVector on right of &
//
// Returns:			None
//================================================================

inline BitVector operator& ( const BitVector &left, const BitVector &right )
	{
	BitVector temp_bits( left );

	temp_bits &= right;
	return temp_bits;
	}

//================================================================
// Name:				operator=
// Class:			BitReference
//
// Description:	Sets the saved indexed bit to the value
//
// Parameters:		bool b							- value to set bit to
//
// Returns:			BitReference&					- the reference of this bit
//================================================================

inline BitReference& BitReference::operator= ( bool b )
	{
	if ( _vector )
		_vector->SetBit( _index, b );

	return *this;
	}

//================================================================
// Name:				operator=
// Class:			BitReference
//
// Description:	Sets the saved indexed bit to the saved index bit in the passed in BitReference
//
// Parameters:		const BitReference &other	- reference of the bit/BitVector to get bit from
//
// Returns:			BitReference&					- the reference of this bit
//================================================================

inline BitReference& BitReference::operator= ( const BitReference &other )
	{
	if ( _vector )
		_vector->SetBit( _index, other._vector->GetBit( other._index ) );

	return *this;
	}

//================================================================
// Name:				operator bool
// Class:			BitVector
//
// Description:	Returns bit that is referenced
//
// Parameters:		None
//
// Returns:			bool								- referenced bit
//================================================================

inline BitReference::operator bool() const
	{
	if ( _vector )
		return _vector->GetBit( _index );
	else
		return false;
	}

//================================================================
// Name:				Setup
// Class:			BitVector
//
// Description:	Sets up the BitReference
//
// Parameters:		BitVector *vector				- vector to reference
//						unsigned int index			- index of the bit to reference
//
// Returns:			None
//================================================================

inline void BitReference::Setup( BitVector *vector, unsigned int index )
	{
	_vector = vector;
	_index = index;
	}

#endif // __BIT_VECTOR_H__
