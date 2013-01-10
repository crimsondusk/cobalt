#ifndef __UTILITY_H__
#define __UTILITY_H__

// =============================================================================
// Checks a value against multiple keys and returns the value based on the
// matched key. expr is the value to be checked against, defaultval is the value
// to be given if no given key matches. numvals is the amount of key/value pairs
// that are given. numvals * 2 additional arguments are expected, first the key
// of the first value and the value it corresponds to, followed by the next
// key/value pair and so on.
//
// Example: Map<const char*> (n, "No medal", 3, 1, "Gold", 2, "Silver", 3, "Bronze")
template <class T> T Map (int expr, T defaultval, int numvals, ...);

// =============================================================================
// Returns the current date as a formatted string.
str GetDate ();

// =============================================================================
// Checks a string against a mask.
bool CheckMask (str string, str mask);

#endif // __UTILITY_H__