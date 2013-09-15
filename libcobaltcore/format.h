#ifndef LIBCOBALT_FORMAT_H
#define LIBCOBALT_FORMAT_H

#include <initializer_list>
#include "variant.h"

// Formatter function
CoString DoFormat (std::initializer_list<CoVariant> args);
void DoPrint (FILE* fp, std::initializer_list<CoVariant> args);

#ifndef IN_IDE_PARSER
#define fmt(...) DoFormat ({__VA_ARGS__})
#define print(...) DoPrint (stdout, {__VA_ARGS__})
#define fprint(F, ...) DoPrint (F, {__VA_ARGS__})
#else
CoString fmt (const char* fmtstr, ...);
void print (const char* fmtstr, ...);
void fprint (FILE* fp, const char* fmtstr, ...);
#endif

#endif // LIBCOBALT_FORMAT_H