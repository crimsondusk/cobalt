// A file full of green text!
#ifndef COBALT_MACROS_H
#define COBALT_MACROS_H

#ifndef __GNUC__
# define __attribute__(N)
#endif // __GNUC__

#define elif else if
#define alias auto&
#define until(A) while (!(A))
#define attr(N) __attribute__ ((N))

#define PROP_NAME(GET) m_##GET
#define READ_ACCESSOR(T, GET) T const& GET() const
#define SET_ACCESSOR(T, SET) void SET (T val)

// Read-only, private property with a get accessor
#define DECLARE_READ_PROPERTY(T, GET, SET) \
private: \
	T PROP_NAME (GET); \
	SET_ACCESSOR (T, SET); \
public: \
	READ_ACCESSOR (T, GET);

// Read/write private property with get and set accessors
#define DECLARE_PROPERTY(T, GET, SET) \
private: \
	T PROP_NAME (GET); \
public: \
	READ_ACCESSOR (T, GET); \
	SET_ACCESSOR (T, SET);

#define DEFINE_PROPERTY(T, CLASS, GET, SET) \
	READ_ACCESSOR (T, CLASS::GET) { return PROP_NAME (GET); } \
	SET_ACCESSOR (T, CLASS::SET) { PROP_NAME (GET) = val; }

// Shortcuts
#define PROPERTY(T, GET, SET) \
private: \
	T PROP_NAME (GET); \
public: \
	READ_ACCESSOR (T, GET) { return PROP_NAME (GET); } \
	SET_ACCESSOR (T, SET) { PROP_NAME (GET) = val; }

#define READ_PROPERTY(T, GET, SET) \
private: \
	T PROP_NAME (GET); \
	SET_ACCESSOR (T, SET) { PROP_NAME (GET) = val; } \
public: \
	READ_ACCESSOR (T, GET) { return PROP_NAME (GET); }

#endif // LIBCOBALT_MACROS_H