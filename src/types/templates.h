#pragma once

namespace cbl
{
	template<typename T>
	bool less( const T& a, const T& b )
	{
		return a < b;
	}

	// -----------------------------------------------------------------------------
	//
	// Templated clamp
	//
	template<typename T>
	static inline T& clamp( T& a, T& min, T& max )
	{
		return ( a > max ) ? max : ( a < min ) ? min : a;
	}

	// -----------------------------------------------------------------------------
	//
	// Templated minimum
	//
	template<typename T>
	static inline T& min( T& a, T& b )
	{
		return ( a < b ) ? a : b;
	}

	// -----------------------------------------------------------------------------
	//
	template<typename T>
	static inline T const& min( T const& a, T const& b )
	{
		return ( a < b ) ? a : b;
	}

	// -----------------------------------------------------------------------------
	//
	// Templated maximum
	//
	//
	template<typename T>
	static inline T& max( T& a, T& b )
	{
		return ( a > b ) ? a : b;
	}

	// -----------------------------------------------------------------------------
	//
	template<typename T>
	static inline T const& max( T const& a, T const& b )
	{
		return ( a > b ) ? a : b;
	}

	// -----------------------------------------------------------------------------
	//
	// Templated absolute value
	//
	template<typename T>
	static inline T& abs( T& a )
	{
		return ( a >= 0 ) ? a : -a;
	}

	// -----------------------------------------------------------------------------
	//
	template<typename T>
	static inline T const& abs( T const& a )
	{
		return ( a >= 0 ) ? a : -a;
	}
}