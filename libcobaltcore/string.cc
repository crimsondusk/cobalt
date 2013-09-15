#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <climits>
#include "containers.h"
#include "string.h"
#include "variant.h"
#include "format.h"

void CoString::trim (int n) {
	if (n > 0)
		m_string = substr (0, length() - n - 1).stdstring();
	else
		m_string = substr (n, -1).stdstring();
}

CoString CoString::strip (::std::initializer_list<char> unwanted) {
	CoString copy (m_string);
	
	for (char c : unwanted) {
		for (long i = length(); i >= 0; --i)
			if (copy[i] == c)
				copy.erase (i);
	}
	
	return copy;
}

CoString CoString::upper() const {
	CoString newstr = m_string;
	
	for (char& c : newstr)
		if (c >= 'a' && c <= 'z')
			c -= 'a' - 'A';
	
	return newstr;
}

CoString CoString::lower() const {
	CoString newstr = m_string;
	
	for (char& c : newstr)
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
	
	return newstr;
}

CoStringList CoString::split (char del) const {
	CoString delimstr;
	delimstr += del;
	return split (delimstr);
}

CoStringList CoString::split (CoString del) const {
	CoStringList res;
	long a = 0;
	
	// Find all separators and store the text left to them.
	while (1) {
		long b = first (del, a);
		
		if (b == -1)
			break;
		
		CoString sub = substr (a, b);
		
		if (sub.length() > 0)
			res.push_back (substr (a, b));
		
		a = b + strlen (del);
	}
	
	// Add the string at the right of the last separator
	if (a < (int) length())
		res.push_back (substr (a, length()));
	
	return res;
}

void CoString::replace (CoStringRef a, CoStringRef b) {
	long pos = 0;
	
	while ((pos = first (a, pos)) != -1) {
		m_string = m_string.replace (pos, strlen (a), b.stdstring());
		pos += strlen (b);
	}
}

int CoString::count (const char needle) const {
	int num = 0;
	
	for (const char& c : m_string)
		if (c == needle)
			num++;
	
	return num;
}

CoString CoString::substr (int a, int b) const {
	if (b == -1)
		b = length();
	
	if (b < a)
		return "";
	
	return m_string.substr (a, (b - a) + 1);
}

int CoString::posof (int n) const {
	int count = 0;
	
	for (int i = 0; i < length(); ++i) {
		if (m_string[i] != ' ')
			continue;
		
		if (++count < n)
			continue;
		
		return i;
	}
	
	return -1;
}

// ============================================================================
#if 0
int CoString::first (CoStringRef c, int a) const {
	size_t pos;
	
	while ((pos = m_string.find_first_of (c.stdstring(), a)) != std::string::npos) {
		// For some reason std::string::find_first_of returns false positives
		// so we need to check that it's really a match.
		if (strncmp (chars() + pos, c, strlen (c)) == 0)
			return pos;
		
		a = pos + strlen (c);
	}
	
	return -1;
}

// ============================================================================
int CoString::last (CoStringRef c, int a) const {
	size_t pos = m_string.find_last_of (c.stdstring(), a);
	if (pos == std::string::npos)
		return -1;
	
	return pos;
}
#endif // 0

// ============================================================================
int CoString::first (CoStringRef c, int a) const {
	unsigned int r = 0;
	unsigned int index = 0;
	for (; a < length(); a++) {
		if (m_string[a] == c[r]) {
			if (r == 0)
				index = a;
			
			r++;
			if (r == strlen (c))
				return index;
		} else {
			if (r != 0) {
				// If the string sequence broke at this point, we need to
				// check this character again, for a new sequence just
				// might start right here.
				a--;
			}
			
			r = 0;
		}
	}
	
	return -1;
}

// ============================================================================
int CoString::last (CoStringRef c, int a) const {
	if (a == -1)
		a = length();
	
	int max = strlen (c) - 1;
	
	int r = max;
	for (; a >= 0; a--) {
		if (m_string[a] == c[r]) {
			r--;
			if (r == -1)
				return a;
		} else {
			if (r != max)
				a++;
			
			r = max;
		}
	}
	
	return -1;
}

void CoString::dump() const {
	print ("`%1`:\n", *this);
	int i = 0;
	
	for (const char& u : m_string)
		print ("\t%1. [%2] `%3`\n", i++, (int) u, u);
}

long CoString::toLong (bool* ok, int base) const {
	errno = 0;
	char* endptr;
	long i = strtol (m_string.c_str(), &endptr, base);
	
	if (ok)
		*ok = (errno == 0 && *endptr == '\0');
	
	return i;
}

float CoString::toFloat (bool* ok) const {
	errno = 0;
	char* endptr;
	float i = strtof (m_string.c_str(), &endptr);
	
	if (ok)
		*ok = (errno == 0 && *endptr == '\0');
	
	return i;
}

double CoString::toDouble (bool* ok) const {
	errno = 0;
	char* endptr;
	double i = strtod (m_string.c_str(), &endptr);
	
	if (ok)
		*ok = (errno == 0 && *endptr == '\0');
	
	return i;
}

CoString CoString::operator+ (const CoString data) const {
	CoString newString = *this;
	newString += data;
	return newString;
}

CoString CoString::operator+ (const char* data) const {
	CoString newString = *this;
	newString += data;
	return newString;
}

CoString& CoString::operator+= (const CoString data) {
	append (data);
	return *this;
}

CoString& CoString::operator+= (const char* data) {
	append (data);
	return *this;
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoString::numeric() const {
	strtof (m_string.c_str(), NULL);
	return (errno == 0);
}

// =============================================================================
// -----------------------------------------------------------------------------
template<class T> static CoString fromNumberConversion (T i, const char* fmtstr) {
	char buf[64];
	sprintf (buf, fmtstr, i);
	return CoString (buf);
}

// =============================================================================
// -----------------------------------------------------------------------------
template<class T> static CoString fromFloatConversion (T i, const char* fmtstr) {
	CoString rep;
	
	// Disable the locale first so that the decimal point will not
	// turn into anything weird (like commas)
	setlocale (LC_NUMERIC, "C");
	
	char x[64];
	sprintf (x, fmtstr, i);
	rep += x;
	
	// Remove trailing zeroes
	while (rep.length() > 0 && rep[rep.length() - 1] == '0')
		rep -= 1;
	
	// If there were only zeroes in the decimal place, remove
	// the decimal point now.
	if (rep[rep.length() - 1] == '.')
		rep -= 1;
	
	return rep;
}

CoString CoString::fromNumber (int i) {
	return fromNumberConversion (i, "%d");
}

CoString CoString::fromNumber (long i) {
	return fromNumberConversion (i, "%ld");
}

CoString CoString::fromNumber (uint i) {
	return fromNumberConversion (i, "%u");
}

CoString CoString::fromNumber (ulong i) {
	return fromNumberConversion (i, "%lu");
}

CoString CoString::fromNumber (float i) {
	return fromFloatConversion (i, "%f");
}

CoString CoString::fromNumber (double i) {
	return fromFloatConversion (i, "%g");
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoString::startsWith (const CoString& other) const {
	if (length() < other.length())
		return false;
	
	return (strncmp (chars(), other.chars(), other.length()) == 0);
}

bool CoString::endsWith (const CoString& other) const {
	if (length() < other.length())
		return false;
	
	int ofs = length() - other.length();
	return (strncmp (chars() + ofs, other.chars(), other.length()) == 0);
}

CoString CoString::operator- (int n) const {
	CoString newString = m_string;
	newString -= n;
	return newString;
}