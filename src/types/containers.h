#pragma once
#include <assert.h>
#include <algorithm>
#include <vector>
#include <list>
#include <deque>
#include <initializer_list>
#include "macros.h"
#include "templates.h"
#include "variant.h"
#include "string.h"

namespace cbl
{
	/**
	 *	@class cbl::generic_container
	 *	@brief A generic container which wraps around one of many container types.
	 *
	 * The generic container is a class that wraps around std::vector,
	 * std::deque or std::list. T is the complete container type, R is the type
	 * of the container element.
	 *
	 * generic_container should not be used directly. Instead, three type aliases
	 * are provided for convenience:
	 * - cbl::list<T>: a generic_container which wraps around std::deque<T>.
	 * - cbl::vector<T>: a generic_container which wraps around std::vector<T>.
	 * - cbl::linked_list<T>: a generic_container which wraps around std::list<T>.
	 */
	template<typename T, typename R>
	class generic_container
	{
		public:
			using iterator					= typename T::iterator;
			using const_iterator			= typename T::const_iterator;
			using reverse_iterator			= typename T::reverse_iterator;
			using const_reverse_iterator	= typename T::const_reverse_iterator;
			using wrapped_container			= T;
			using value_type				= R;
			using self						= generic_container<T, R>;

			/**
			 * Constructs an empty container.
			 */
			generic_container() {}

			/**
			 * Constructs a container with the given values.
			 * @param values the values to construct the container with.
			 */
			generic_container( const wrapped_container& values ) :
				m_data( values ) {}

			/**
			 * Constructs a container with the given values.
			 * @param values the values to construct the container with.
			 */
			generic_container( const self& values ) :
				m_data( values.get_wrapped() ) {}

			/**
			 * Constructs a container with the given values.
			 * @param values the values to construct the container with.
			 */
			generic_container( const std::initializer_list<value_type>& values ) :
				m_data( values ) {}

			/**
			 * @return an iterator to the beginning of the container.
			 */
			inline iterator begin()
			{
				return m_data.begin();
			}

			/**
			 * @return a const iterator to the beginning of the container.
			 */
			inline const_iterator begin() const
			{
				return m_data.cbegin();
			}

			/**
			 * @return an iterator to the end of the container.
			 */
			inline iterator end()
			{
				return m_data.end();
			}

			/**
			 * @return a const iterator to the end of the container.
			 */
			inline const_iterator end() const
			{
				return m_data.cend();
			}

			/**
			 * @return a reverse iterator to the beginning of the container.
			 */
			inline reverse_iterator rbegin()
			{
				return m_data.rbegin();
			}

			/**
			 * @return a const reverse iterator to the beginning of the container.
			 */
			inline const_reverse_iterator crbegin() const
			{
				return m_data.crbegin();
			}

			/**
			 * @return a reverse iterator to the end of the container.
			 */
			inline reverse_iterator rend()
			{
				return m_data.rend();
			}

			/**
			 * @return a const reverse iterator to the end of the container.
			 */
			inline const_reverse_iterator crend() const
			{
				return m_data.crend();
			}

			/**
			 * Appends a copy of the given value into the list. This function
			 * calls the STL method push_back.
			 *
			 * @param value the element to add to the list
			 *
			 * Example:
			 * @code
			 *    cbl::list<int> a({ 10, 20, 30, 40, });
			 *    a.append( 50 );
			 *    cbl::print( "%1\n", a ); // { 10, 20, 30, 40, 50 }
			 * @endcode
			 */
			inline void append( const value_type& value )
			{
				m_data.push_back( value );
			}

			/**
			 * Clears the container, removing all elements.
			 */
			inline void clear()
			{
				m_data.clear();
			}

			/**
			 * Returns a string description representative of the container. This
			 * function is used by cbl::format and friends to process containers
			 * as formatting arguments.
			 *
			 * Example:
			 * @code
			 *    cbl::list<int> a({ 10, 20, 30, 40 });
			 *    cbl::string b = a.describe(); // { 10, 20, 30, 40 }
			 * @endcode
			 */
			string describe() const
			{
				string contents;

				for( const auto& it : a )
				{
					if( contents.is_empty() == false )
						contents += ", ";

					contents += variant( it ).describe();
				}

				return "{" + contents + "}";
			}

			/**
			 * Removes an element by iterator.
			 * @param pos the iterator to remove
			 *
			 * Example:
			 * @code
			 *    cbl::list<int> a({ 10, 20, 30, 40, });
			 *    a.erase( a.begin() + 1 );
			 *    cbl::print( "%1\n", a ); // { 10, 30, 40 }
			 * @endcode
			 */
			inline void erase( iterator const& pos )
			{
				m_data.erase( pos );
			}

			/**
			 * Finds the index of the first element whose value is the same
			 * as the argument. The value type must provide an operator== for
			 * this method to work.
			 *
			 * @param needle the value to find in the list. -1 is returned if
			 * the value is not found.
			 *
			 * Example:
			 * @code
			 *    cbl::list<cbl::string> a({ "apple", "banana", "coconut" });
			 *    cbl::print( "%1\n", a.find_first( "banana" )); // 1
			 *    cbl::print( "%1\n", a.find_first( "diamond" )); // -1
			 * @endcode
			 */
			int find_first( const value_type& needle )
			{
				int i = 0;

				for( const value_type& hay : *this )
				{
					if( hay == needle )
						return i;

					i++;
				}

				return -1;
			}

			/**
			 * @return a const reference to the first value in the list.
			 *
			 * Example:
			 * @code
			 *    cbl::list<cbl::string> a({ "apple", "banana", "coconut" });
			 *    cbl::print( "%1\n", a.first() ); // apple
			 * @endcode
			 */
			inline value_type const& first() const
			{
				return *begin();
			}

			/**
			 * This is an override of @c first() const for non-const containers.
			 * @return a mutable reference to the first value in the list.
			 *
			 * Example:
			 * @code
			 *    cbl::list<cbl::string> a({ "apple", "banana", "coconut" });
			 *    a.first() = "ash";
			 *    cbl::print( "%1\n", a.first() ); // ash
			 * @endcode
			 */
			inline value_type& first()
			{
				return *begin();
			}

			// ---------------------------------------------------------------------
			//
			inline void insert( int pos, const value_type& value )
			{
				m_data.insert( m_data.begin() + pos, value );
			}

			// ---------------------------------------------------------------------
			//
			inline bool is_empty() const
			{
				return size() == 0;
			}

			// ---------------------------------------------------------------------
			//
			inline value_type const& last() const
			{
				return *( end() - 1 );
			}

			// ---------------------------------------------------------------------
			//
			inline value_type& last()
			{
				return *( end() - 1 );
			}

			// ---------------------------------------------------------------------
			//
			void merge( const self& vals )
			{
				// TODO
				for( const value_type& val : vals )
					append( val );
			}

			// ---------------------------------------------------------------------
			//
			bool pop( value_type& val )
			{
				if( size() == 0 )
					return false;

				val = m_data[size() - 1];
				remove_at( size() - 1 );
				return true;
			}

			/**
			 * Prepends an element to the container.
			 * @param value the value to copy to the container.
			 *
			 * Example:
			 * @code
			 *    cbl::list<int> a({ 10, 20, 30, 40 });
			 *    a.prepend( 5 );
			 *    cbl::print( a ); // { 5, 10, 20, 30, 40 }
			 * @endcode
			 *
			 * @warning This function is particularly slow with cbl::vector. If
			 * you intend to use this function you should use cbl::list instead.
			 */
			value_type& prepend( const value_type& value )
			{
				m_data.push_front( value );
				return m_data[0];
			}

			/**
			 * Removes the element at the given position.
			 * @param pos the index where to remove the element at.
			 *
			 * Example:
			 * @code
			 *    cbl::list<int> a({ 10, 20, 30, 40 });
			 *    a.remove_at( 1 );
			 *    cbl::print( a ); // { 10, 30, 40 }
			 * @endcode
			 */
			inline void remove_at( int pos )
			{
				assert( pos < size() );
				m_data.erase( m_data.begin() + pos );
			}

			/**
			 * Removes any duplicated entries from the list. The list is sorted
			 * in the process
			 *
			 * Example:
			 * @code
			 *    cbl::list<int> a({ 10, 30, 20, 10, 40 });
			 *    a.remove_duplicates();
			 *    cbl::print( a ); // { 10, 20, 30, 40 }
			 * @endcode
			 */
			void remove_duplicates()
			{
				sort();
				resize( std::distance( begin(), std::unique( begin(), end() )));
			}

			/**
			 * Removes the first found given element from the list. The value
			 * type must provide an operator== for this to work.
			 * @param unwanted the element to remove from the list.
			 *
			 * Example:
			 * @code
			 *    cbl::list<int> a({ 10, 20, 30, 40, 10 });
			 *    a.remove_one( 10 );
			 *    cbl::print( a ); // { 20, 30, 40, 10 }
			 * @endcode
			 */
			void remove_one( const value_type& unwanted )
			{
				int idx = find_first( unwanted );

				if( idx != -1 )
					remove_at( idx );
			}

			/**
			 * Resizes the container to hold a specific amount of values
			 * @param size the new size
			 *
			 * Example:
			 * @code
			 *    // copy a vector into a list
			 *    cbl::vector<float> a({ 1.01f, 2.02f, 3.14f });
			 *    cbl::list<float> b;
			 *    b.resize( a.size() );
			 *    std::copy( a.begin(), a.end(), b.begin() );
			 *    cbl::print( a ); // { 1.01, 2.02, 3.14 }
			 * @endcode
			 */
			inline void resize( std::ptrdiff_t size )
			{
				m_data.resize( size );
			}

			// ---------------------------------------------------------------------
			//
			inline int size() const
			{
				return m_data.size();
			}

			// ---------------------------------------------------------------------
			//
			self slice( int a, int b ) const
			{
				if( b < 0 )
					b = size() - b;

				self result;
				result.resize( b - a );
				std::copy_n( begin() + a, b - a, result.begin() );
				return result;
			}

			// ---------------------------------------------------------------------
			//
			inline void sort()
			{
				std::sort( begin(), end() );
			}

			// ---------------------------------------------------------------------
			//
			inline const wrapped_container& wrapped() const
			{
				return m_data;
			}

			// ---------------------------------------------------------------------
			//
			inline self& operator<<( const value_type& value )
			{
				append( value );
				return *this;
			}

			// ---------------------------------------------------------------------
			//
			inline self& operator<<( const self& vals )
			{
				append( vals );
				return *this;
			}

			// ---------------------------------------------------------------------
			//
			inline bool operator>>( value_type& value )
			{
				return pop( value );
			}

			// ---------------------------------------------------------------------
			//
			inline value_type& operator[]( int n )
			{
				assert( n < size() );
				return m_data[n];
			}

			// ---------------------------------------------------------------------
			//
			inline const value_type& operator[]( int n ) const
			{
				assert( n < size() );
				return m_data[n];
			}

		private:
			wrapped_container m_data;
	};

	// ---------------------------------------------------------------------========
	//
	// Operator for pushing front: List<int> vals; 0 >> vals;
	//
	template<typename T, typename R>
	generic_container<T, R>& operator>>( const R& value, generic_container<T, R>& haystack )
	{
		haystack.prepend( value );
		return haystack;
	}

	/**
	 * A generic_container with std::deque as the underlying class
	 */
	template<typename T>
	using list = generic_container<std::deque<T>, T>;

	template<typename T>
	using vector = generic_container<std::vector<T>, T>;

	template<typename T>
	using linked_list = generic_container<std::list<T>, T>;
}
