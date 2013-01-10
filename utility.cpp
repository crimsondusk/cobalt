#include <time.h>
#include "common.h"

// =============================================================================
// Checks a value against multiple keys and returns the value based on the key
template <class T> T Map (int expr, T defaultval, int numvals, ...) {
	va_list va;
	va_start (va, numvals);
	while (numvals-- > 0) {
		int key = va_arg (va, int);
		T value = va_arg (va, T);
		
		if (key == expr)
			return value;
	}
	
	return defaultval;
}

// =============================================================================
str GetDate () {
	time_t now;
	time (&now);
	tm* timeinfo = localtime (&now);
	
	const char* weekday = Map<const char*> (timeinfo->tm_wday, "Sunday", 6,
		1, "Monday",
		2, "Tuesday",
		3, "Wednesday",
		4, "Thursday",
		5, "Friday",
		6, "Saturday"
	);
	
	const char* month = Map<const char*> (timeinfo->tm_mon, "January", 11,
		1, "February",
		2, "March",
		3, "April",
		4, "May",
		5, "June",
		6, "July",
		7, "August",
		8, "September",
		9, "October",
		10, "November",
		11, "December"
	);
	
	const char* daysuffix = Map<const char*> (timeinfo->tm_mday % 10, "th", 3,
		1, "st",
		2, "nd",
		3, "rd"
	);
	
	str tzstring;
	int timezone = timeinfo->tm_hour - gmtime (&now)->tm_hour;
	if (timezone == 0)
		tzstring = "GMT";
	else
		tzstring.format ("GMT%s%d", (timezone > 0) ? "+" : "-", abs (timezone));
	
	str date;
	date.format ("%s %s %d%s %.2d:%.2d:%.2d %s", weekday, month, timeinfo->tm_mday,
		daysuffix, timeinfo->tm_hour + timezone, timeinfo->tm_min, timeinfo->tm_sec,
		tzstring.chars());
	return date;
}

// =============================================================================
// Checks a string against a mask
bool Mask (str string, str mask) {
	char* maskstring = mask.chars();
	char* mptr = maskstring;
	for (char* sptr = string.chars(); *sptr != '\0'; sptr++) {
		if (*mptr == '?') {
			if (*(sptr + 1) == '\0') {
				// ? demands that there's a character here and there wasn't.
				// Therefore, mask matching fails
				return false;
			}
			
		} else if (*mptr == '*') {
			char end = *(++mptr);
			
			// * is the final character of the message, so if we get
			// here, all of the remaining string matches against the *.
			if (end == '\0')
				return true;
			
			// Skip to the end character
			while (*sptr != end && *sptr != '\0')
				sptr++;
			
			if (*sptr == '\0') {
				// String ended while the mask still had stuff
				return false;
			}
		} else if (*sptr != *mptr)
			return false;
		
		mptr++;
	}
	
	return true;
}