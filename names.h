#ifndef __NAMES_H__
#define __NAMES_H__

#define name(X) NAME_ ## X

enum name_e {
#define _N(X) NAME_ ## X
	_N(None) = 0,
#include "namelist.h"
#undef _N
};

// Name-to-string array, definition in main.cpp
extern const char* g_NameStrings[];

#endif // __NAMES_H__