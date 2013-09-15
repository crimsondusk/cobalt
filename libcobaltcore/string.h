#ifndef LIBCOBALT_STRING_H
#define LIBCOBALT_STRING_H

#include <deque>
#include <string>
#include <stdarg.h>
#include "types.h"
#include "containers.h"

class CoString;
typedef CoString str; // TODO: move this out of the lib
typedef CoList<CoString> CoStringList;
typedef const CoString& CoStringRef;
typedef const CoStringList& CoStringListRef;

class CoString {
public:
	typedef typename ::std::string::iterator Iterator;
	typedef typename ::std::string::const_iterator ConstIterator;
	typedef std::string StringType;
	typedef const std::string& StringTypeRef;
	
	CoString() {}
	CoString (const char* data) : m_string (data) {}
	CoString (StringTypeRef data) : m_string (data) {}
	
	int          count (const char needle) const;
	void         dump() const;
	bool         endsWith (const CoString& other) const;
	int          first (CoStringRef c, int a = 0) const;
	int          last (CoStringRef c, int a = -1) const;
	CoString     lower() const;
	bool         numeric() const;
	int          posof (int n) const;
	void         replace (CoStringRef a, CoStringRef b);
	CoStringList split (CoString del) const;
	CoStringList split (char del) const;
	bool         startsWith (const CoString& other) const;
	CoString     strip (std::initializer_list<char> unwanted);
	CoString     substr (int a, int b = -1) const;
	double       toDouble (bool* ok = null) const;
	float        toFloat (bool* ok = null) const;
	long         toLong (bool* ok = null, int base = 10) const;
	void         trim (int n);
	CoString     upper() const;
	
	inline int           compare (CoStringRef other) const { return m_string.compare (other.stdstring()); }
	inline void          append (const char* data) { m_string.append (data); }
	inline void          append (const char data) { m_string.push_back (data); }
	inline void          append (CoStringRef data) { m_string.append (data.chars()); }
	inline Iterator      begin() { return m_string.begin(); }
	inline ConstIterator begin() const { return m_string.cbegin(); }
	inline int           capacity() const { return m_string.capacity(); }
	inline const char*   chars() const { return m_string.c_str(); }
	inline Iterator      end() { return m_string.end(); }
	inline ConstIterator end() const { return m_string.end(); }
	inline void          clear() { m_string.clear(); }
	inline bool          empty() const { return m_string.empty(); }
	inline void          erase (int pos) { m_string.erase (m_string.begin() + pos); }
	inline void          insert (int pos, char c) { m_string.insert (m_string.begin() + pos, c); }
	inline int           length() const { return m_string.length(); }
	inline int           maxSize() const { return m_string.max_size(); }
	inline void          resize (int n) { m_string.resize (n); }
	inline void          remove (int pos, int n) { m_string.replace (pos, n, ""); }
	inline void          shrinkToFit() { m_string.shrink_to_fit(); }
	inline StringTypeRef stdstring() const { return m_string; };
	inline CoString      strip (char unwanted) { return strip ({unwanted}); }
	inline CoString&     operator+= (const char data) { append (data); return *this; }
	inline CoString      operator+  () const { return upper(); }
	inline CoString      operator-  () const { return lower(); }
	inline CoString&     operator-= (int n) { trim (n); return *this; }
	
	CoString             operator+  (const CoString data) const;
	CoString             operator+  (const char* data) const;
	CoString&            operator+= (const CoString data);
	CoString&            operator+= (const char* data);
	CoString             operator-  (int n) const;
	inline bool          operator== (const CoString& other) const { return compare (other) == 0; }
	inline bool          operator== (const char* other) const { return compare (other) == 0; }
	inline bool          operator!= (const CoString& other) const { return compare (other) != 0; }
	inline bool          operator!= (const char* other) const { return compare (other) != 0; }
	inline bool          operator!  () const { return empty(); }
	
	static CoString      fromNumber (int i);
	static CoString      fromNumber (long i);
	static CoString      fromNumber (uint i);
	static CoString      fromNumber (ulong i);
	static CoString      fromNumber (float i);
	static CoString      fromNumber (double i);
	
	operator const char*() const { return chars(); }
	operator const std::string&() const { return stdstring(); }

private:
	::std::string m_string;
};

inline bool operator== (const char* a, const CoString& b) {
	return b == a;
}

inline CoString operator+ (const char* a, const CoString& b) {
	return str (a) + b;
}

#endif // LIBCOBALT_STRING_H