#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "str.h"
#include "array.h"
#include "names.h"
#include "config.h"
#include "commands.h"
#include "utility.h"

#ifndef unix
typedef unsigned int uint;
typedef unsigned short int ushort;
typedef unsigned long int ulong;
#endif
typedef long long int llong;
typedef unsigned long long int ullong;
typedef long double ldouble;

// Application name and version
#define APPNAME "cobalt"
#define VERSION_MAJOR 0
#define VERSION_MINOR 999

#define PERFORM_FORMAT(in, out) \
	va_list v; \
	va_start (v, in); \
	char* out = vdynformat (in, v, 256); \
	va_end (v);

#define PLURAL(n) (n != 1) ? "s" : ""
#define twice for (int repeat_token = 0; repeat_token < 2; repeat_token++)

#ifndef __GNUC__
#define __attribute__(X)
#endif
#define deprecated __attribute__ ((deprecated))

// Byte datatype
typedef int32_t word;
typedef unsigned char byte;

#define typestruct typedef struct

#endif // __COMMON_H__