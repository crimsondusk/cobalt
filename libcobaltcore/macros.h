// A file full of green text!
#ifndef LIBCOBALT_MACROS_H
#define LIBCOBALT_MACROS_H

#ifndef __GNUC__
# define __attribute__(N)
#endif // ! __GNUC__

#ifndef NO_LIBCOBALT_KEYWORDS
# define elif(A) else if (A)
# define alias auto&
# define until(A) while (!(A))
# define forever for (;;)
# define attr(N) __attribute__ ((N))
#endif // ! NO_LIBCOBALT_KEYWORDS

#define CO_PROP(GET) m_##GET
#define READ_ACCESSOR(T, GET) T const& GET() const
#define SET_ACCESSOR(T, SET) void SET (T val)

// Read-only, private property with a get accessor
#define DECLARE_READ_PROPERTY(T, GET, SET) \
private: \
	T CO_PROP (GET); \
	SET_ACCESSOR (T, SET); \
public: \
	READ_ACCESSOR (T, GET);

// Read/write private property with get and set accessors
#define DECLARE_PROPERTY(T, GET, SET) \
private: \
	T CO_PROP (GET); \
public: \
	READ_ACCESSOR (T, GET); \
	SET_ACCESSOR (T, SET);

#define DEFINE_PROPERTY(T, CLASS, GET, SET) \
	READ_ACCESSOR (T, CLASS::GET) { return CO_PROP (GET); } \
	SET_ACCESSOR (T, CLASS::SET) { CO_PROP (GET) = val; }

// Shortcuts
#define PROPERTY(T, GET, SET) \
private: \
	T CO_PROP (GET); \
public: \
	READ_ACCESSOR (T, GET) { return CO_PROP (GET); } \
	SET_ACCESSOR (T, SET) { CO_PROP (GET) = val; }

#define READ_PROPERTY(T, GET, SET) \
private: \
	T CO_PROP (GET); \
	SET_ACCESSOR (T, SET) { CO_PROP (GET) = val; } \
public: \
	READ_ACCESSOR (T, GET) { return CO_PROP (GET); }

#endif // LIBCOBALT_MACROS_H