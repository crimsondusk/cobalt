#ifndef __NAMES_H__
#define __NAMES_H__

#ifndef IN_IDE_PARSER
namespace Name {
	enum Type {
#define _N(X) X,
	None = 0,
#include "namelist.h"
#undef _N
	};
}
#else
#include "namelist.h"
#endif // IN_IDE_PARSER

// Name-to-string array, definition in main.cpp
extern const char* g_NameStrings[];

#endif // __NAMES_H__
