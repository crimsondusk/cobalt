#ifndef __STR_H__
#define __STR_H__

#include <string.h>
#include <initializer_list>

template<class T> class array;

char* vdynformat (const char* csFormat, va_list vArgs, long int lSize);

#define SCCF_NUMBER	1<<0
#define SCCF_WORD	1<<1

// Dynamic string object, allocates memory when needed and
// features a good bunch of manipulation methods
class str {
private:
	// The actual message
	char* text;
	
	// Where will append() place new characters?
	unsigned int curs;
	
	// Allocated length
	unsigned int alloclen;
	
	// Resize the text buffer to len characters
	void resize (unsigned int len);
	
public:
	// ======================================================================
	str ();
	str (const char* c);
	str (char c);
	~str ();
	
	// ======================================================================
	// METHODS
	
	// Empty the string
	void clear ();
	
	// Length of the string
	size_t len () {
		return strlen (text);
	}
	
	// The char* form of the string
	char* chars ();
	
	// Dumps the character table of the string
	void dump ();
	
	// Appends text to the string
	void append (char c);
	void append (const char* c);
	void append (str c);
	
	// Appends formatted text to the string.
	void format (const char* fmt, ...);
	
	// Appends formatted text to the string.
	void appendformat (const char* c, ...);
	
	// Returns the first occurrence of c in the string, optionally starting
	// from a certain position rather than the start.
	int first (const char* c, unsigned int a = 0);
	
	// Returns the last occurrence of c in the string, optionally starting
	// from a certain position rather than the end.
	int last (const char* c, int a = -1);
	
	// Returns a substring of the string, from a to b.
	str substr (unsigned int a, unsigned int b);
	
	// Replace a substring with another substring.
	void replace (const char* o, const char* n, unsigned int a = 0);
	
	// Removes a given index from the string, optionally more characters than just 1.
	void remove (unsigned int idx, unsigned int dellen=1);
	
	str trim (int dellen);
	
	// Inserts a substring into a certain position.
	void insert (char* c, unsigned int pos);
	
	// Reverses the string.
	str reverse ();
	
	// Repeats the string a given amount of times.
	str repeat (int n);
	
	// Is the string a number?
	bool isnumber ();
	
	// Is the string a word, i.e consists only of alphabetic letters?
	bool isword ();
	
	// Convert string to lower case
	str tolower ();
	
	// Convert string to upper case
	str toupper ();
	
	// Compare this string with another
	int compare (const char* c);
	int compare (str c);
	int icompare (str c);
	int icompare (const char* c);
	
	// Counts the amount of substrings in the string
	unsigned int count (char* s);
	
	// Counts where the given substring is seen for the nth time
	int instanceof (const char* s, unsigned n);
	
	char subscript (uint pos) {
		return operator[] (pos);
	}
	
	array<str> split (str del);
	
	void strip (char c);
	void strip (std::initializer_list<char> unwanted);
	
	// ======================================================================
	// OPERATORS
	str operator+ (str& c) {
		append (c);
		return *this;
	}
	
	str& operator+= (char c) {
		append (c);
		return *this;
	}
	
	str& operator+= (const char* c) {
		append (c);
		return *this;
	}
	
	str& operator+= (const str c) {
		append (c);
		return *this;
	}
	
	str operator* (const int repcount) {
		repeat (repcount);
		return *this;
	}
	
	str& operator*= (const int repcount) {
		str other = repeat (repcount);
		clear ();
		append (other);
		return *this;
	}
	
	str operator- (const int trimcount) {
		trim (trimcount);
		return *this;
	}
	
	str& operator-= (const int trimcount) {
		str other = trim (trimcount);
		clear ();
		append (other);
		return *this;
	}
	
	array<str> operator/ (str splitstring);
	array<str> operator/ (char* splitstring);
	array<str> operator/ (const char* splitstring);
	
	int operator% (str splitstring) {return count (splitstring.chars());}
	int operator% (char* splitstring) {return count (splitstring);}
	int operator% (const char* splitstring) {
		return count (str (splitstring).chars());
	}
	
	str operator+ () {
		return toupper ();
	}
	
	str operator- () {
		return tolower ();
	}
	
	str operator! () {
		return reverse ();
	}
	
#define DEFINE_OPERATOR_TYPE(OPER, TYPE) \
	bool operator OPER (TYPE other) {return compare(other) OPER 0;}
#define DEFINE_OPERATOR(OPER) \
	DEFINE_OPERATOR_TYPE (OPER, str) \
	DEFINE_OPERATOR_TYPE (OPER, char*) \
	DEFINE_OPERATOR_TYPE (OPER, const char*)
	
	DEFINE_OPERATOR (==)
	DEFINE_OPERATOR (!=)
	DEFINE_OPERATOR (>)
	DEFINE_OPERATOR (<)
	DEFINE_OPERATOR (>=)
	DEFINE_OPERATOR (<=)
	
	char& operator[] (int pos) {
		return text[pos];
	}
	
	operator char* () const {
		return text;
	}
	
	operator int () const {return atoi(text);}
	operator unsigned int () const {return atoi(text);}
};

#endif // __STR_H__