#ifndef COMMON_H
#define COMMON_H

#include <util/string.h>
#include <util/types.h>
#include <util/vector.h>
#include <util/variant.h>
#include <util/format.h>
#include "names.h"
#include "config.h"
#include "utility.h"

// Application name and version
#define APPNAME "cobalt"
#define VERSION_MAJOR 0
#define VERSION_MINOR 999

void FatalError (const char* file, ulong line, const char* func, vector<var> s);
void DoWarn (const char* file, ulong line, const char* func, vector<var> s);

#ifndef IN_IDE_PARSER
# define fatal(...) FatalError (__FILE__, __LINE__, __func__, {__VA_ARGS__})
# define warn(...) DoWarn (__FILE__, __LINE__, __func__, {__VA_ARGS__})
#else
void fatal (const char* fmtstr, ...);
void warn (const char* msg, ...);
#endif // IN_IDE_PARSER

#endif // COMMON_H