#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "str.h"

#define ITERATE_SUBSCRIPTS(link) \
	for (link = data; link; link = link->next)

#define foreach_counter(NAME) \
	(int)(reinterpret_cast<int> (foreach_counter_##NAME) / sizeof (void*))
#define foreach(T,NAME,ARRAY) \
	if (ARRAY.size() > 0) for (T NAME = &ARRAY[0], *foreach_counter_##NAME = 0; \
		foreach_counter(NAME) < (signed)(ARRAY.size()); \
		NAME = &ARRAY[reinterpret_cast<int> (++foreach_counter_##NAME) / sizeof (foreach_counter_##NAME)])

// Single element of an array
template <class T> class arrayElement {
public:
	T value;
	arrayElement<T>* next;
	
	arrayElement () {
		next = NULL;
	}
};

// Dynamic array
template <class T> class array {
public:
	array () {
		data = NULL;
	}
	
	array (T* stuff, unsigned int c) {
		data = NULL;
		
		for (unsigned int i = 0; i < c; i++)
			push (stuff[c]);
	}
	
	~array () {
		clear ();
	}
	
	// =====================================================
	// Creates a clone of this array. Use this if you are
	// passing an array as a function parameter, as by just
	// assigning this to another array causes both arrays to
	// have the same data pointers! This ensures that both
	// arrays are truly unique and do not conflict.
	array<T> clone () {
		array<T> other;
		
		for (arrayElement<T>* e = data; e->next; e = e->next)
			other << e->value;
		
		return other;
	}
	
	// =====================================================
	// Empty an array of all of its data.
	void clear () {
		if (data)
			deleteElement (data);
	}
	
	// =====================================================
	// Adds a new element to the array.
	T& push (T stuff) {
		arrayElement<T>* e = new arrayElement<T>;
		e->value = stuff;
		e->next = NULL;
		
		if (!data) {
			data = e;
			return data->value;
		}
		
		arrayElement<T>* link;
		for (link = data; link && link->next; link = link->next);
		link->next = e;
		return e->value;
	}
	
	// =====================================================
	// Pops the last element out of the array and returns it.
	T pop () {
		int pos = size() - 1;
		assert (pos != -1);
		T res = subscript (pos);
		remove (pos);
		return res;
	}
	
	// =====================================================
	// Removes an element from the array by an index.
	void remove (unsigned int pos) {
		assert (data != NULL);
		
		if (pos == 0) {
			// special case for first element
			arrayElement<T>* first = data;
			data = data->next;
			delete first;
			return;
		}
		
		arrayElement<T>* link = data;
		unsigned int x = 0;
		while (link->next) {
			if (x == pos - 1)
				break;
			link = link->next;
			x++;
		}
		
		assert (link != NULL);
		
		arrayElement<T>* nextlink = link->next->next;
		delete link->next;
		link->next = nextlink;
	}
	
	// =====================================================
	// Measures the size of the array.
	unsigned int size () {
		unsigned int x = 0;
		arrayElement<T>* link;
		ITERATE_SUBSCRIPTS(link)
			x++;
		return x;
	}
	
	// =====================================================
	// Returns the given subscript of the array without
	// popping it.
	T& subscript (unsigned int i) {
		arrayElement<T>* link;
		unsigned int x = 0;
		ITERATE_SUBSCRIPTS(link) {
			if (x == i)
				return link->value;
			x++;
		}
		
		return data->value;
	}
	
	// =====================================================
	// Returns the array as a dynamic C-style array.
	T* out () {
		int s = size();
		T* out = new T[s];
		unsigned int x = 0;
		
		arrayElement<T>* link;
		ITERATE_SUBSCRIPTS(link) {
			out[x] = link->value;
			x++;
		}
		
		return out;
	}
	
	T& operator [] (const unsigned int i) {
		return subscript (i);
	}
	
	T& operator << (const T stuff) {
		return (push (stuff));
	}
	
	void operator >> (T& stuff) {
		stuff = pop ();
	}
	
private:
	void deleteElement (arrayElement<T>* e) {
		if (e->next)
			deleteElement (e->next);
		delete e;
	}
	
	arrayElement<T>* data;
};

#endif // __ARRAY_H__