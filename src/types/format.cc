#include <stdio.h>
#include "variant.h"
#include "misc.h"
#include "format.h"

namespace cbl
{
	// -------------------------------------------------------------------------
	//
	//    Does most of the formatting work
	//
	string format_args( const cbl::string& fmtstr, const vector< cbl::string >& args )
	{
		if( args.is_empty() )
			return fmtstr;

		string fmt = fmtstr;
		string out;
		int pos = 0;

		while(( pos = fmt.first( "%", pos ) ) != -1 )
		{
			if( fmt[pos + 1] == '%' )
			{
				fmt.replace( pos, 2, "%" );
				pos++;
				continue;
			}

			int ofs = 1;
			char mod = '\0';

			// handle modifiers
			if( fmt[pos + ofs] == 's' || fmt[pos + ofs] == 'x' || fmt[pos + ofs] == 'd' )
			{
				mod = fmt[pos + ofs];
				ofs++;
			}

			if( !isdigit( fmt[pos + ofs] ) )
				throw std::logic_error( "bad format string, expected digit with optional "
					"modifier after '%%'" );

			int i = fmt[pos + ofs]  - '0';

			if( i >= args.size() + 1 )
				throw std::logic_error( format( "format arg #%1 used but not defined, in format string \"%2\"",
					i, fmtstr ));

			string repl = args[i - 1];

			switch( mod )
			{
				case 's': repl = ( repl == "1" ) ? "" : "s";		break;
				case 'd': repl.sprintf( "%d", repl[0] );			break;
				case 'x': repl.sprintf( "0x%X", repl.to_long() );	break;
				default: break;
			}

			fmt.replace( pos, 1 + ofs, repl );
			pos += repl.length();
		}

		return fmt;
	}
}
