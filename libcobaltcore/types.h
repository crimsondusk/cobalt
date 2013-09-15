#ifndef LIBCOBALT_TYPES_H
#define LIBCOBALT_TYPES_H

#include <map>
#include <stdint.h>

// Null pointer
static const std::nullptr_t null = nullptr;

typedef unsigned char      uchar;
typedef unsigned int       uint;
typedef unsigned short     ushort;
typedef unsigned long      ulong;
typedef long long          longlong;
typedef unsigned long long ulonglong;
typedef long double        ldouble;
typedef int8_t             int8;
typedef int16_t            int16;
typedef int32_t            int32;
typedef int64_t            int64;
typedef uint8_t            uint8;
typedef uint16_t           uint16;
typedef uint32_t           uint32;
typedef uint64_t           uint64;

template<class T> using initlist = std::initializer_list<T>;
template<class T, class R> using CoMap = std::map<T, R>;
template<class T, class R> using CoPair = std::pair<T, R>;
typedef std::size_t size_t;

// Generic templated structs for multiple-value return types
template<class T, class R = T> struct CoDuo {
	T a;
	R b;
};

template<class T, class R = T, class Q = R> struct CoTrio {
	T a;
	R b;
	Q c;
};

template<class T, class R = T, class Q = R, class S = Q> struct CoQuad {
	T a;
	R b;
	Q c;
	S d;
};

#endif // LIBCOBALT_TYPES_H