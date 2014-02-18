#pragma once
#include "macros.h"
#include "version.h"
#include "types/templates.h"
#include "types/property.h"
#include "types/types.h"
#include "types/containers.h"
#include "types/string.h"
#include "types/variant.h"
#include "types/format.h"
#include "types/config.h"

#ifndef IN_IDE_PARSER
# define fatal( ... ) FatalError( __FILE__, __LINE__, __func__, { __VA_ARGS__ })
# define warn( ... ) DoWarn( __FILE__, __LINE__, __func__, { __VA_ARGS__ })
#else
void fatal (const char* fmtstr, ...);
void warn (const char* msg, ...);
#endif // IN_IDE_PARSER

void fatal_error ( const char* file, ulong line, const char* func, const cbl::list< cbl::format_arg >& s );
void warn_args ( const char* file, ulong line, const char* func, const cbl::list< cbl::format_arg >& s );
bool save_configuration();
const cbl::string& get_config_file_name();