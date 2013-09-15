#ifndef LIBCOBALTCORE_FLAGS_H
#define LIBCOBALTCORE_FLAGS_H

#include <cstdio>
#include <cassert>

template<class T> class CoFlags {
public:
	typedef T EnumType;
	typedef CoFlags<T> Flags;
	
	CoFlags (int i = 0) : m_value (i) {}
	CoFlags (EnumType e) : m_value (e) {}
	
	inline const int& intValue() const {
		return m_value;
	}
	
	inline CoFlags& operator|= (Flags other) {
		m_value |= other.intValue();
		return *this;
	}
	
	inline CoFlags& operator&= (Flags other) {
		m_value &= other.intValue();
		return *this;
	}
	
	inline CoFlags& operator^= (Flags other) {
		m_value ^= other.intValue();
		return *this;
	}
	
	inline CoFlags operator| (Flags other) const {
		return intValue() | other.intValue();
	}
	
	inline CoFlags operator& (Flags other) const {
		return intValue() & other.intValue();
	}
	
	inline CoFlags operator^ (Flags other) const {
		return intValue() ^ other.intValue();
	}
	
	inline CoFlags operator~() const {
		return ~intValue();
	}
	
	inline bool operator== (Flags other) const {
		return m_value == other.intValue();
	}
	
	inline bool operator!= (Flags other) const {
		return !operator== (other);
	}
	
private:
	int m_value;
};

#endif // LIBCOBALTCORE_FLAGS_H