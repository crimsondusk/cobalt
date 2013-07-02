#include <time.h>
#include <util/time.h>
#include "common.h"

// Name-to-string array.
const char* g_NameStrings[] =
{
#define _N(X) "NAME_" # X
	_N( None ),
#include "namelist.h"
#undef _N
};

Date GMTDate( Time t )
{
	time_t now = t.seconds();
	struct tm* gmtnow = gmtime( &now );
	return Date( gmtnow );
}

// =============================================================================
// Checks a string against a mask
bool mask( str string, str mask )
{
	// Elevate to uppercase for case-insensitive matching
	string = +string;
	mask = +mask;

	const char* maskstring = mask.chars();
	const char* mptr = maskstring;

	for( const char* sptr = string.chars(); *sptr != '\0'; sptr++ )
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

			// * is the final character of the message, so if we get
			// here, all of the remaining string matches against the *.
			if( end == '\0' )
				return true;

			// Skip to the end character
			while( *sptr != end && *sptr != '\0' )
				sptr++;

			if( *sptr == '\0' )
			{
				// String ended while the mask still had stuff
				return false;
			}
		}
		else if( *sptr != *mptr )
			return false;

		mptr++;
	}

	return true;
}

str join( const vector<str>& args, str delim )
{
	str rep = "";
	
	for( const str& i : args )
	{
		if (!!rep)
			rep += delim;
		
		rep += i;
	}
	
	return rep;
}