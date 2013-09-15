#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <cstdarg>
#include "main.h"

// =============================================================================
// Checks a value against multiple keys and returns the value based on the
// matched key. expr is the value to be checked against, defaultval is the value
// to be given if no given key matches. numvals is the amount of key/value pairs
// that are given. numvals * 2 additional arguments are expected, first the key
// of the first value and the value it corresponds to, followed by the next
// key/value pair and so on.
//
// Example: valueMap<const char*>( n, "No medal", 3, 1, "Gold", 2, "Silver", 3, "Bronze" )
//
// TODO: make this use vector<variant>
template <class T> T valueMap (int expr, T defaultval, int numvals, ...) {
	va_list va;
	va_start (va, numvals);
	
	while (numvals-- > 0) {
		int key = va_arg (va, int);
		T value = va_arg (va, T);
		
		if (key == expr) {
			va_end (va);
			return value;
		}
	}
	
	va_end (va);
	return defaultval;
}

str join (const CoStringList& args, str delim);

int posof (const str& a, int i);

// =============================================================================
// Checks a string against a mask.
bool mask (str string, str mask);

#endif // __UTILITY_H__