#pragma once
#include "main.h"
#include "variant.h"
#include "containers.h"

namespace cbl
{
	extern const int g_primes[500];

	void simplify( int& numer, int& denom );

	// =============================================================================
	// -----------------------------------------------------------------------------
	template<typename T>
	string describe_array( const T& array )
	{
		if( array.is_empty() )
			return "{}";

		string text;
		uint i = 0;

		for( const auto& it : array )
		{
			if( text.is_empty() )
				text += ", ";

			variant arg = it;
			text += arg.describe();
		}

		return "{" + text + "}";
	}

	// =============================================================================
	// -----------------------------------------------------------------------------
	template<typename T>
	T exponent( T a, int b )
	{
		if( b == 0 )
			return 1;

		T c = a;

		while( b-- > 1 )
			c *= a;

		return c;
	}

	// =============================================================================
	// -----------------------------------------------------------------------------
	template<typename T>
	bool in( const T& needle, const list<T>& haystack )
	{
		for( const T & hay : haystack )
			if( needle == hay )
				return true;

		return false;
	}

	// =============================================================================
	//
	// Checks a value against multiple keys and returns the value based on the
	// matched key. expr is the value to be checked against, defaultval is the value
	// to be given if no given key matches. numvals is the amount of key/value pairs
	// that are given. numvals * 2 additional arguments are expected, first the key
	// of the first value and the value it corresponds to, followed by the next
	// key/value pair and so on.
	//
	// Example: cbl::value_map<const char*>( n, "No medal", { 1, "Gold", 2, "Silver", 3, "Bronze" })
	//
	template <typename T>
	T value_map( int expr, T defaultval, list<tuple<int, T>> values )
	{
		for( auto it = values.begin(); it != values.end(); ++it )
			if( std::get<0>( *it ) == expr )
				return std::get<1>( *it );

		return defaultval;
	}
}

