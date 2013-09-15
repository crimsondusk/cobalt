#ifndef LIBCOBALT_CONTAINERS_H
#define LIBCOBALT_CONTAINERS_H

#include <assert.h>
#include <algorithm>
#include <vector>
#include <list>
#include <deque>
#include <initializer_list>

// =============================================================================
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// =============================================================================
// Container
//
// Class that wraps around ::std::vector, ::std::deque and ::std::list
// T is the complete container type, R is the type of the container element.
// For instance, Vector<int> -> T = ::std::vector<int>, R = int
// =============================================================================
template<class T, class R> class CoContainer {
public:
	typedef typename T::iterator Iterator;
	typedef typename T::const_iterator ConstIterator;
	typedef typename T::reverse_iterator ReverseIterator;
	typedef typename T::const_reverse_iterator ConstReverseIterator;
	typedef CoContainer<T, R> ContainerType;
	
	CoContainer() {}
	CoContainer (std::initializer_list<R> vals) {
		m_data = vals;
	}
	
	Iterator begin() {
		return m_data.begin();
	}
	
	ConstIterator begin() const {
		return m_data.cbegin();
	}
	
	Iterator end() {
		return m_data.end();
	}
	
	ConstIterator end() const {
		return m_data.cend();
	}
	
	ReverseIterator rbegin() {
		return m_data.rbegin();
	}
	
	ConstReverseIterator crbegin() const {
		return m_data.crbegin();
	}
	
	ReverseIterator rend() {
		return m_data.rend();
	}
	
	ConstReverseIterator crend() const {
		return m_data.crend();
	}
	
	void erase (int pos) {
		assert (pos < size());
		m_data.erase (m_data.begin() + pos);
	}
	
	R& push_front (const R& value) {
		m_data.push_front (value);
		return m_data[0];
	}
	
	R& push_back (const R& value) {
		m_data.push_back (value);
		return m_data[m_data.size() - 1];
	}
	
	void push_back (const ContainerType& vals) {
		for (const R& val : vals)
			push_back (val);
	}
	
	bool pop (R& val) {
		if (size() == 0)
			return false;
		
		val = m_data[size() - 1];
		erase (size() - 1);
		return true;
	}
	
	R& operator<<  (const R& value) {
		return push_back (value);
	}
	
	void operator<<  (const ContainerType& vals) {
		push_back (vals);
	}
	
	bool operator>>  (R& value) {
		return pop (value);
	}
	
	void clear() {
		m_data.clear();
	}
	
	void insert (int pos, const R& value) {
		m_data.insert (m_data.begin() + pos, value);
	}
	
	void makeUnique() {
		// Remove duplicate entries. For this to be effective, the vector must be
		// sorted first.
		sort();
		Iterator pos = std::unique (begin(), end());
		resize (std::distance (begin(), pos));
	}
	
	int size() const {
		return m_data.size();
	}
	
	R& operator[] (int n) {
		assert (n < size());
		return m_data[n];
	}
	
	const R& operator[] (int n) const {
		assert (n < size());
		return m_data[n];
	}
	
	void resize (std::ptrdiff_t size) {
		m_data.resize (size);
	}
	
	void sort() {
		std::sort (begin(), end());
	}
	
	int find (const R& needle) {
		int i = 0;
		
		for (const R& hay : *this) {
			if (hay == needle)
				return i;
			
			i++;
		}
		
		return -1;
	}
	
	void remove (const R& it) {
		int idx = find (it);
		
		if (idx != -1)
			erase (idx);
	}
	
private:
	T m_data;
};

// :D
template<class T, class R> CoContainer<T, R>& operator>> (const R& value, CoContainer<T, R>& haystack) {
	haystack.push_front (value);
	return haystack;
}

template<class T> using CoList = CoContainer< ::std::deque<T>, T>;
template<class T> using CoVector = CoContainer< ::std::vector<T>, T>;
template<class T> using CoLinkedList = CoContainer< ::std::list<T>, T>;
template<class T> using CoListRef = const CoList<T>&;
template<class T> using CoVectorRef = const CoVector<T>&;
template<class T> using CoLinkedListRef = const CoLinkedList<T>&;
template<class T> using list = CoList<T>;

#endif // LIBCOBALT_CONTAINERS_H