#include "variant.h"
#include "format.h"
#include "time.h"
#include "ip.h"

#define DEFINE_VARIANT_TYPE(T, N)                                           \
	CoVariant::CoVariant (const T& val) {                                   \
		buildFrom##N (val);                                                 \
	}                                                                       \
	                                                                        \
	T CoVariant::as##N() const {                                            \
		assert (valueType() == N##Type);                                    \
		return *(reinterpret_cast<T*> (m_value));                           \
	}                                                                       \
	                                                                        \
	void CoVariant::buildFrom##N (const T& val) {                           \
		T* valptr = new T (val);                                            \
		                                                                    \
		/* Init the variant */                                              \
		m_type = N##Type;                                                   \
		m_typestring = #T;                                                  \
		m_allocsize = sizeof (T);                                           \
		m_value = valptr;                                                   \
		                                                                    \
		/* Write down the functions we'll clone or deinit this with */      \
		m_deinitfunc = CoVariant::deleteAs##N;                              \
		m_clonefunc = CoVariant::cloneAs##N;                                \
	}                                                                       \
	                                                                        \
	void* CoVariant::cloneAs##N (const CoVariant& var) {                    \
		return new T (var.as##N());                                         \
	}                                                                       \
	                                                                        \
	void CoVariant::deleteAs##N (const void* val) {                         \
		const T* c_ptr = reinterpret_cast<const T*> (val);                  \
		delete const_cast<T*> (c_ptr);                                      \
	}

DEFINE_VARIANT_TYPE (int, Int)
DEFINE_VARIANT_TYPE (short, Short)
DEFINE_VARIANT_TYPE (long, Long)
DEFINE_VARIANT_TYPE (long long, LongLong)
DEFINE_VARIANT_TYPE (uint, UInt)
DEFINE_VARIANT_TYPE (ushort, UShort)
DEFINE_VARIANT_TYPE (ulong, ULong)
DEFINE_VARIANT_TYPE (unsigned long long, ULongLong)
DEFINE_VARIANT_TYPE (char, Char)
DEFINE_VARIANT_TYPE (uchar, UChar)
DEFINE_VARIANT_TYPE (float, Float)
DEFINE_VARIANT_TYPE (double, Double)
DEFINE_VARIANT_TYPE (long double, LongDouble)
DEFINE_VARIANT_TYPE (str, String)
DEFINE_VARIANT_TYPE (CoTime, Time)
DEFINE_VARIANT_TYPE (CoDate, Date)
DEFINE_VARIANT_TYPE (CoIPAddress, IPAddress)
DEFINE_VARIANT_TYPE (CoIPRange, IPRange)
DEFINE_VARIANT_TYPE (std::nullptr_t, Null)

CoVariant::CoVariant (void* val) {
	buildAsPointer (val);
}

CoVariant::CoVariant (const char* val) {
	buildFromString (str (val));
}

CoVariant::CoVariant (const CoVariant& val) {
	m_type = val.valueType();
	m_typestring = val.typestring();
	m_deinitfunc = val.deinitFunc();
	m_clonefunc = val.cloneFunc();
	
	if (val.valueType() != PointerType) {
		m_value = (*val.cloneFunc()) (val);
	} else {
		m_value = const_cast<void*> (val.asPointer());
		m_allocsize = 0;
	}
}

void CoVariant::buildAsPointer (void* val, const char* typestring) {
	m_type = PointerType;
	m_value = val;
	m_typestring = typestring;
	m_allocsize = 0;
}

int CoVariant::allocsize() const {
	return m_allocsize;
}

const void* CoVariant::asPointer() const {
	return m_value;
}

CoVariant::~CoVariant() {
	(*m_deinitfunc) (asPointer());
}

const char* CoVariant::typestring() const {
	return m_typestring;
}

CoVariant::Type CoVariant::valueType() const {
	return m_type;
}

int CoVariant::size() const {
	return sizeof m_value;
}

CoString CoVariant::stringRep (StringFlags flags) const {
	switch (m_type) {
#define NUMERIC_STRINGREP(N, C)                              \
	case N##Type:                                            \
		{                                                    \
			char valstr[64];                                 \
			if (flags & Hex)                                 \
				sprintf (valstr, "0x%lX", (ulong) as##N());  \
			else                                             \
				sprintf (valstr, "%" #C, as##N());           \
			return valstr;                                   \
		}
	
	NUMERIC_STRINGREP (Char,        c)
	NUMERIC_STRINGREP (Int,         d)
	NUMERIC_STRINGREP (Short,       d)
	NUMERIC_STRINGREP (Long,       ld)
	NUMERIC_STRINGREP (LongLong,  lld)
	NUMERIC_STRINGREP (UChar,       c)
	NUMERIC_STRINGREP (UInt,        u)
	NUMERIC_STRINGREP (UShort,      u)
	NUMERIC_STRINGREP (ULong,      lu)
	NUMERIC_STRINGREP (ULongLong, llu)
	NUMERIC_STRINGREP (Float,       f)
	NUMERIC_STRINGREP (Double,      g)
	NUMERIC_STRINGREP (LongDouble, Lg)
	
	case PointerType:
		char valstr[64];
		sprintf (valstr, "%p", asPointer());
		return valstr;
	
	case StringType:
		return asString();
	
	case TimeType:
		return asTime().stringRep();
	
	case DateType:
		return asDate().stringRep();
	
	case IPAddressType:
		return asIPAddress().toString();
	
	case IPRangeType:
		return asIPRange().toString();
	
	default:
		return "<" + CoString (typestring()) + ">";
	}
	
	return "";
}