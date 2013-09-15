#ifndef LIBCOBALTCORE_TYPEDPOINTER_H
#define LIBCOBALTCORE_TYPEDPOINTER_H

#include <cassert>
#include <typeinfo>

// =============================================================================
// Essentially a void* pointer which remembers its type. Check type with ::type()
// before using castTo because if the types mismatch you net yourself an assertion
// failure!
// -----------------------------------------------------------------------------
class CoTypedPointer {
public:
	template<class T> CoTypedPointer (T* ptr) :
		m_ptr (ptr),
		m_type (&typeid (*ptr)) {}
	
	CoTypedPointer (std::nullptr_t) :
		m_ptr (nullptr),
		m_type (&typeid (std::nullptr_t)) {}
	
	// Get this pointer as T*.
	template<class T> T* castTo() const {
		if (m_ptr == nullptr)
			return nullptr;
		
		assert (typeid (T) == *m_type);
		return reinterpret_cast<T*> (m_ptr);
	}
	
	template<class T> inline T& value() {
		return *castTo<T>();
	}
	
	inline void* ptr() const {
		return m_ptr;
	}
	
	inline const std::type_info& type() const {
		return *m_type;
	}
	
private:
	void*                      m_ptr;
	const std::type_info*      m_type;
};

#endif // LIBCOBALTCORE_TYPEDPOINTER_H