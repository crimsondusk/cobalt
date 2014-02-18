#include <stdio.h>
#include "variant.h"
#include "misc.h"
#include "format.h"

namespace cbl
{
	// -----------------------------------------------------------------------------
	//
	void print_args( FILE* fp, list<variant> const& args )
	{
		string msg = format_args( args );
		fprintf( fp, "%s", msg.c_str() );
	}

	// -----------------------------------------------------------------------------
	//
	string format_args( list<variant> const& args )
	{
		assert( args.size() >= 1 );
		assert( args.begin()->value_type() == variant::string_type );

		bool perc = false;
		int percnum = 0;
		int percdigits = 0;
		string fmtd;
		string percstr;
		string fmtstr = args.begin()->as_string();

		for( const char* c = &fmtstr[0];; c++ )
		{
			if( perc )
			{
				if( *c >= '0' && *c <= '9' )
				{
					// Argument digit
					percnum += ( *c - '0' ) * exponent( 10, percdigits );
					percdigits++;
					percstr += *c;
					continue;
				}
				else if( percdigits > 0 )
				{
					// End of argument, fill in the value
					if( args.size() < percnum + 1 )
					{
						print_to( stderr, "Too few arguments to format string `%1`\n", fmtstr );
						fmtd += percstr;
					}
					else
						fmtd += ( args.begin() + percnum )->describe();

					perc = false;
				} else
				{
					// '%' followed by a non-digit. If we got %%, add both percentage signs
					// and jump to the start of the loop to prevent the latter % from being
					// intepreted as an argument symbol.

					perc = false;
					fmtd += "%";

					if( *c == '%' )
					{
						fmtd += "%";
						continue;
					}
				}
			}

			if( *c == '\0' )
				break;

			if( *c == '%' )
			{
				if( perc )
				{
					// Another case of %%
					perc = false;
					fmtd += "%%";
				}
				else
				{
					// We got '%', begin intepreting argument
					perc = true;
					percdigits = 0;
					percnum = 0;
					percstr = "%";
				}

				continue;
			}

			fmtd += *c;
		}

		return fmtd;
	}
}
