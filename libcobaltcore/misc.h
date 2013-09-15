#ifndef LIBCOBALT_MISC_H
#define LIBCOBALT_MISC_H

#include "main.h"
#include "variant.h"

extern const int primes[500];

CoString ftoa (double num);
void simplify (int& numer, int& denom);

static const long long
	million = 1000000LL,
	billion = 1000000000LL;

// =============================================================================
// -----------------------------------------------------------------------------
template<class T> T exponent (T a, ulong b) {
	if (b == 0)
		return 1;
	
	long c = a;
	
	while (b-- > 1)
		c *= a;
	
	return c;
}

// =============================================================================
// -----------------------------------------------------------------------------
template<class T> void dataswap (T& a, T& b) {
	T* c = &a;
	a = b;
	b = *c;
}

// =============================================================================
// -----------------------------------------------------------------------------
template<class T> bool in (const T& needle, const initlist<T>& haystack) {
	for (const T& hay : haystack)
		if (needle == hay)
			return true;
	
	return false;
}

// =============================================================================
// -----------------------------------------------------------------------------
template<class T> str arrayRep (const T& array) {
	str text = "{ ";
	uint i = 0;
	
	for (const alias it : array) {
		if (i++)
			text += ", ";
		
		CoVariant arg = it;
		text += arg.stringRep();
	}
	
	if (i)
		text += " ";
	
	text += "}";
	return text;
}

// =============================================================================
// Plural expression
// -----------------------------------------------------------------------------
template<class T> static inline const char* plural (T n) {
	return (n != 1) ? "s" : "";
}

// =============================================================================
// Templated clamp
// -----------------------------------------------------------------------------
template<class T> static inline T& clamp (T& a, T &min, T& max) {
	return (a > max) ? max : (a < min) ? min : a;
}

// =============================================================================
// Templated minimum
// -----------------------------------------------------------------------------
template<class T> static inline T& min (T& a, T& b) {
	return (a < b) ? a : b;
}

// =============================================================================
// Templated maximum
// -----------------------------------------------------------------------------
template<class T> static inline T& max (T& a, T& b) {
	return (a > b) ? a : b;
}

// =============================================================================
// Templated absolute value
// -----------------------------------------------------------------------------
template<class T> static inline T abs (T a) {
	return (a >= 0) ? a : -a;
}

#endif // LIBCOBALT_MISC_H