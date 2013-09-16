#ifndef COMMON_H
#define COMMON_H

#include <libcobaltcore/main.h>
#include <libcobaltcore/format.h>
#include <libcobaltcore/config.h>
#include "version.h"
#include "utility.h"

typedef CoString str;

#ifndef IN_IDE_PARSER
# define fatal( ... ) FatalError( __FILE__, __LINE__, __func__, { __VA_ARGS__ })
# define warn( ... ) DoWarn( __FILE__, __LINE__, __func__, { __VA_ARGS__ })
#else
void fatal (const char* fmtstr, ...);
void warn (const char* msg, ...);
#endif // IN_IDE_PARSER

void FatalError (const char* file, ulong line, const char* func, initlist<CoVariant> s);
void DoWarn (const char* file, ulong line, const char* func, std::initializer_list<CoVariant> s);
bool saveConfig();
CoStringRef configFileName();

#endif // COMMON_H