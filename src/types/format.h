#pragma once
#include "variant.h"

namespace cbl
{
	// Formatter function
	string format_args( list<variant> const& args );
	void print_args( FILE* fp, list<variant> const& args );
};

#define CBL_FORMAT( ... ) cbl::format_args({ __VA_ARGS__ })
#define CBL_PRINT( ... ) cbl::print_args( stdout, { __VA_ARGS__ })
#define CBL_PRINT_TO( F, ... ) cbl::print_args( F, { __VA_ARGS__ })

#ifndef COBALT_NO_KEYWORDS
# ifndef IN_IDE_PARSER
#  define format CBL_FORMAT
#  define print CBL_PRINT
#  define print_to CBL_PRINT_TO
# else
cbl::string format( const char* fmtstr, ... );
void print( const char* fmtstr, ... );
void print_to( FILE* fp, const char* fmtstr, ... );
# endif // IN_IDE_PARSER
#endif // COBALT_NO_KEYWORDS