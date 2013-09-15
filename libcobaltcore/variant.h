#ifndef LIBCOBALTCORE_VARIANT_H
#define LIBCOBALTCORE_VARIANT_H

#include <string.h>
#include "types.h"
#include "string.h"
#include "main.h"

#define DECLARE_VARIANT_TYPE(T, N)                             \
public:                                                        \
	CoVariant (const T& val);                                  \
	T as##N () const;                                          \
	                                                           \
private:                                                       \
	void buildFrom##N (const T& val);                          \
	static void* cloneAs##N (const CoVariant& var);            \
	static void deleteAs##N (const void* val);

class CoTime;
class CoDate;
class CoIPRange;
class CoIPAddress;

class CoVariant {
public:
	typedef void (*DeinitFuncType) (const void*);
	typedef void* (*CloneFuncType) (const CoVariant&);
	
	enum StringFlags {
		Hex = (1 << 0),
		NoFlags = (0)
	};
	
	enum Type {
		IntType,
		ShortType,
		LongType,
		LongLongType,
		UIntType,
		UShortType,
		ULongType,
		ULongLongType,
		CharType,
		UCharType,
		FloatType,
		DoubleType,
		LongDoubleType,
		IPAddressType,
		IPRangeType,
		StringType,
		TimeType,
		DateType,
		PointerType,
		NullType,
	};
	
	DECLARE_VARIANT_TYPE (int, Int)
	DECLARE_VARIANT_TYPE (short, Short)
	DECLARE_VARIANT_TYPE (long, Long)
	DECLARE_VARIANT_TYPE (longlong, LongLong)
	DECLARE_VARIANT_TYPE (uint, UInt)
	DECLARE_VARIANT_TYPE (ushort, UShort)
	DECLARE_VARIANT_TYPE (ulong, ULong)
	DECLARE_VARIANT_TYPE (ulonglong, ULongLong)
	DECLARE_VARIANT_TYPE (char, Char)
	DECLARE_VARIANT_TYPE (uchar, UChar)
	DECLARE_VARIANT_TYPE (float, Float)
	DECLARE_VARIANT_TYPE (double, Double)
	DECLARE_VARIANT_TYPE (ldouble, LongDouble)
	DECLARE_VARIANT_TYPE (CoIPAddress, IPAddress)
	DECLARE_VARIANT_TYPE (CoIPRange, IPRange)
	DECLARE_VARIANT_TYPE (CoString, String)
	DECLARE_VARIANT_TYPE (CoTime, Time)
	DECLARE_VARIANT_TYPE (CoDate, Date)
	DECLARE_VARIANT_TYPE (std::nullptr_t, Null)
	
public:
	CoVariant (void* val);
	CoVariant (const char* val);
	CoVariant (const CoVariant& val);
	~CoVariant();
	
	void           buildAsPointer  (void* val, const char* typestring = "void*");
	CloneFuncType  cloneFunc       () const { return m_clonefunc; }
	DeinitFuncType deinitFunc      () const { return m_deinitfunc; }
	int            allocsize       () const;
	const void*    asPointer       () const;
	CoString       stringRep       (StringFlags flags = NoFlags) const;
	const char*    typestring      () const;
	Type           valueType       () const;
	int            size            () const;
	
private:
	int            m_allocsize;
	void*          m_value;
	const char*    m_typestring;
	Type           m_type;
	DeinitFuncType m_deinitfunc;
	CloneFuncType  m_clonefunc;
};

typedef const CoVariant& CoVariantRef;

#endif // LIBCOBALTCORE_VARIANT_H