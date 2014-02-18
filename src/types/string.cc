#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <climits>
#include "containers.h"
#include "string.h"
#include "variant.h"
#include "format.h"
#include "string_list.h"

namespace cbl
{
	// =============================================================================
	//
	void string::trim( int n )
	{
		if( n > 0 )
			m_string = get_substring( 0, length() - n - 1 ).std_string();
		else
			m_string = get_substring( n, -1 ).std_string();
	}

	// =============================================================================
	//
	string string::strip( ::std::initializer_list<char> unwanted )
	{
		string copy( m_string );

		for( char c : unwanted )
		{
			for( int i = length(); i >= 0; --i )
				if( copy[i] == c )
					copy.remove_at( i );
		}

		return copy;
	}

	// =============================================================================
	//
	string string::to_uppercase() const
	{
		string newstr = m_string;

		for( char& c : newstr )
			if( c >= 'a' && c <= 'z' )
				c -= 'a' - 'A';

		return newstr;
	}

	// =============================================================================
	//
	string string::to_lowercase() const
	{
		string newstr = m_string;

		for( char & c : newstr )
			if( c >= 'A' && c <= 'Z' )
				c += 'a' - 'A';

		return newstr;
	}

	// =============================================================================
	//
	string_list string::split( const string& del ) const
	{
		string_list res;
		long a = 0;

		// Find all separators and store the text left to them.
		for( ;; )
		{
			long b = first( del, a );

			if( b == -1 )
				break;

			string sub = get_substring( a, b - 1 );

			if( sub.length() > 0 )
				res.append( get_substring( a, b ));

			a = b + strlen( del );
		}

		// Add the string at the right of the last separator
		if( a < ( int ) length() )
			res.append( get_substring( a ));

		return res;
	}

	// =============================================================================
	//
	void string::replace( const string& a, const string& b )
	{
		long pos = 0;

		while(( pos = first( a, pos )) != -1 )
		{
			m_string = m_string.replace( pos, strlen( a ), b.std_string() );
			pos += strlen( b );
		}
	}

	// =============================================================================
	//
	int string::count( char needle ) const
	{
		int num = 0;

		for( char c : m_string )
			if( c == needle )
				num++;

		return num;
	}

	// =============================================================================
	//
	string string::get_substring( int a, int b ) const
	{
		if( b == -1 )
			b = length();

		if( b < a )
			return "";

		return m_string.substr( a, ( b - a ));
	}

	// =============================================================================
	//
	int string::get_word_position( int n ) const
	{
		int count = 0;

		for( int i = 0; i < length(); ++i )
		{
			if( m_string[i] != ' ' )
				continue;

			if( ++count < n )
				continue;

			return i;
		}

		return -1;
	}

	// =============================================================================
	//
	bool string::mask( string const& mask ) const
	{
		// Elevate to uppercase for case-insensitive matching
		const string string_to_check = to_uppercase();
		const string mask_to_check = mask.to_uppercase();
		const char* maskstring = mask_to_check.c_str();
		const char* mptr = &maskstring[0];

		for( const char* sptr = string_to_check.c_str(); *sptr != '\0'; sptr++ )
		{
			if( *mptr == '?' )
			{
				if( *( sptr + 1 ) == '\0' )
				{
					// ? demands that there's a character here and there wasn't.
					// Therefore, mask matching fails
					return false;
				}
			}
			else if( *mptr == '*' )
			{
				char end = *( ++mptr );

				// If '*' is the final character of the message, all of the remaining
				// string matches against the '*'. We don't need to bother checking
				// the string any further.
				if( end == '\0' )
					return true;

				// Skip to the end character
				while( *sptr != end && *sptr != '\0' )
					sptr++;

				// String ended while the mask still had stuff
				if( *sptr == '\0' )
					return false;
			} else if( *sptr != *mptr )
				return false;

			mptr++;
		}

		return true;
	}

	// =============================================================================
	//
	int string::first( const string& c, int a ) const
	{
		for( ; a < length(); a++ )
			if( m_string[a] == c[0] && strncmp( m_string.c_str() + a, c.c_str(), c.length() ) == 0 )
				return a;

		return -1;
	}

	// =============================================================================
	//
	int string::last( const string& c, int a ) const
	{
		if( a == -1 || a >= length() )
			a = length() - 1;

		for( ; a > 0; a -- )
			if( m_string[a] == c[0] && strncmp( m_string.c_str() + a, c.c_str(), c.length() ) == 0 )
				return a;

		return -1;
	}

	// =============================================================================
	//
	void string::dump() const
	{
		print( "`%1`:\n", *this );
		int i = 0;

		for( char u : m_string )
			print( "\t%1. [%2] `%3`\n", i++, u, string( &u ));
	}

	// =============================================================================
	//
	long string::to_long( bool* ok, int base ) const
	{
		errno = 0;
		char* endptr;
		long i = strtol( m_string.c_str(), &endptr, base );

		if( ok )
			*ok = ( errno == 0 && *endptr == '\0' );

		return i;
	}

	// =============================================================================
	//
	float string::to_float( bool* ok ) const
	{
		errno = 0;
		char* endptr;
		float i = strtof( m_string.c_str(), &endptr );

		if( ok )
			*ok = ( errno == 0 && *endptr == '\0' );

		return i;
	}

	// =============================================================================
	//
	double string::to_double( bool* ok ) const
	{
		errno = 0;
		char* endptr;
		double i = strtod( m_string.c_str(), &endptr );

		if( ok )
			*ok = ( errno == 0 && *endptr == '\0' );

		return i;
	}

	// =============================================================================
	//
	string string::operator+ ( const string data ) const
	{
		string newString = *this;
		newString += data;
		return newString;
	}

	// =============================================================================
	//
	string string::operator+ ( const char* data ) const
	{
		string newString = *this;
		newString += data;
		return newString;
	}

	// =============================================================================
	//
	string& string::operator+= ( const string data )
	{
		append( data );
		return *this;
	}

	// =============================================================================
	//
	string& string::operator+= ( const char* data )
	{
		append( data );
		return *this;
	}

	// =============================================================================
	//
	bool string::is_numeric() const
	{
		strtof( m_string.c_str(), NULL );
		return ( errno == 0 );
	}

	// =============================================================================
	//
	template<class T>
	static string convert_from_number( T i, const char* fmtstr )
	{
		char buf[64];
		sprintf( buf, fmtstr, i );
		return string( buf );
	}

	// =============================================================================
	//
	template<class T>
	static string convert_from_float( T i, const char* fmtstr )
	{
		string rep;

		// Disable the locale first so that the decimal point will not
		// turn into anything weird (like commas)
		setlocale( LC_NUMERIC, "C" );

		char x[64];
		sprintf( x, fmtstr, i );
		rep += x;

		// Remove trailing zeroes
		while( rep.length() > 0 && rep[rep.length() - 1] == '0' )
			rep -= 1;

		// If there were only zeroes in the decimal place, remove
		// the decimal point now.
		if( rep[rep.length() - 1] == '.' )
			rep -= 1;

		return rep;
	}

	// =============================================================================
	//
	string string::from_number( int i )
	{
		return convert_from_number( i, "%d" );
	}

	// =============================================================================
	//
	string string::from_number( long i )
	{
		return convert_from_number( i, "%ld" );
	}

	// =============================================================================
	//
	string string::from_number( uint i )
	{
		return convert_from_number( i, "%u" );
	}

	// =============================================================================
	//
	string string::from_number( ulong i )
	{
		return convert_from_number( i, "%lu" );
	}

	// =============================================================================
	//
	string string::from_number( float i )
	{
		return convert_from_float( i, "%f" );
	}

	// =============================================================================
	//
	string string::from_number( double i )
	{
		return convert_from_float( i, "%g" );
	}

	// =============================================================================
	//
	bool string::starts_with( const string& other ) const
	{
		if( length() < other.length() )
			return false;

		return ( strncmp( c_str(), other.c_str(), other.length() ) == 0 );
	}

	// =============================================================================
	//
	bool string::ends_with( const string& other ) const
	{
		if( length() < other.length() )
			return false;

		int ofs = length() - other.length();
		return ( strncmp( c_str() + ofs, other.c_str(), other.length() ) == 0 );
	}

	// =============================================================================
	//
	string string::operator-( int n ) const
	{
		string newString = m_string;
		newString -= n;
		return newString;
	}
}
