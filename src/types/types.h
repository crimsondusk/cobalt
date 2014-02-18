#pragma once
#include <map>
#include <tuple>
#include <stdint.h>
#include "macros.h"

namespace cbl
{
	// Null pointer
	static const std::nullptr_t null = nullptr;

	using uchar = unsigned char;
	using uint = unsigned int;
	using ushort = unsigned short;
	using ulong = unsigned long;
	using longlong = long long;
	using ulonglong = unsigned long long;
	using ldouble = long double;
	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;
	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;
	using size_t = std::size_t;

	template<class T>
	using initializer_list = std::initializer_list<T>;

	template<class T, class R>
	using map = std::map<T, R>;

	template<class T, class R>
	using pair = std::pair<T, R>;

	template<class... args>
	using tuple = std::tuple<args...>;

	static const long long g_billion = 1000000000000LL;
}
