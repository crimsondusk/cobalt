#include "string_list.h"

namespace cbl
{
	// -----------------------------------------------------------------------------
	//
	string_list::string_list() {}

	// -----------------------------------------------------------------------------
	//
	string_list::string_list( std::initializer_list<string> vals ) :
		list<string>( vals ) {}

	// -----------------------------------------------------------------------------
	//
	string string_list::join( string delim )
	{
		string rep = "";

		for( string const& i : get_wrapped() )
		{
			if( rep.is_empty() == false )
				rep += delim;

			rep += i;
		}

		return rep;
	}
}