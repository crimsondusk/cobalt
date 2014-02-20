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
	// ---------------------------------------------------------------------========
	// generic_container
	//
	// Class that wraps around ::std::vector, ::std::deque and ::std::list
	// T is the complete container type, R is the type of the container element.
	// For instance, cobalt::vector<int> -> T = ::std::vector<int>, R = int
	// ---------------------------------------------------------------------========
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
			using sort_comparetype			= bool(*)( const value_type&, const value_type& );

			generic_container() {}

			generic_container( std::initializer_list<value_type> vals ) :
				m_data( vals ) {}

			// ---------------------------------------------------------------------
			//
			iterator begin()
			{
				return m_data.begin();
			}

			// ---------------------------------------------------------------------
			//
			const_iterator begin() const
			{
				return m_data.cbegin();
			}

			// ---------------------------------------------------------------------
			//
			iterator end()
			{
				return m_data.end();
			}

			// ---------------------------------------------------------------------
			//
			const_iterator end() const
			{
				return m_data.cend();
			}

			// ---------------------------------------------------------------------
			//
			reverse_iterator rbegin()
			{
				return m_data.rbegin();
			}

			// ---------------------------------------------------------------------
			//
			const_reverse_iterator crbegin() const
			{
				return m_data.crbegin();
			}

			// ---------------------------------------------------------------------
			//
			reverse_iterator rend()
			{
				return m_data.rend();
			}

			// ---------------------------------------------------------------------
			//
			const_reverse_iterator crend() const
			{
				return m_data.crend();
			}

			// ---------------------------------------------------------------------
			//
			inline void remove_at( int pos )
			{
				assert( pos < size() );
				m_data.erase( m_data.begin() + pos );
			}

			// ---------------------------------------------------------------------
			//
			inline void erase( iterator const& pos )
			{
				m_data.erase( pos );
			}

			// ---------------------------------------------------------------------
			//
			value_type& prepend( const value_type& value )
			{
				m_data.push_front( value );
				return m_data[0];
			}

			// ---------------------------------------------------------------------
			//
			value_type& append( const value_type& value )
			{
				m_data.push_back( value );
				return m_data[m_data.size() - 1];
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
			inline void clear()
			{
				m_data.clear();
			}

			// ---------------------------------------------------------------------
			//
			inline void insert( int pos, const value_type& value )
			{
				m_data.insert( m_data.begin() + pos, value );
			}

			// ---------------------------------------------------------------------
			//
			inline void remove_duplicates()
			{
				sort();
				resize( std::distance( begin(), std::unique( begin(), end() )));
			}

			// ---------------------------------------------------------------------
			//
			inline int size() const
			{
				return m_data.size();
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

			// ---------------------------------------------------------------------
			//
			inline void resize( std::ptrdiff_t size )
			{
				m_data.resize( size );
			}

			// ---------------------------------------------------------------------
			//
			inline void sort()
			{
				std::sort( begin(), end() );
			}

			// ---------------------------------------------------------------------
			//
			inline value_type const& first() const
			{
				return *begin();
			}

			// ---------------------------------------------------------------------
			//
			inline value_type& first()
			{
				return *begin();
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
			inline bool is_empty() const
			{
				return size() == 0;
			}

			// ---------------------------------------------------------------------
			//
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
			inline const wrapped_container& get_wrapped() const
			{
				return m_data;
			}

			int find_first( const value_type& needle );
			void remove( const value_type& it );

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

	// ---------------------------------------------------------------------========
	//
	template<typename T, typename R>
	int generic_container<T, R>::find_first( const value_type& needle )
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

	// ---------------------------------------------------------------------========
	//
	template<typename T, typename R>
	void generic_container<T, R>::remove( const value_type& a )
	{
		int idx = find_first( a );

		if( idx != -1 )
			remove_at( idx );
	}

	template<typename T>
	using list = generic_container<std::deque<T>, T>;

	template<typename T>
	using vector = generic_container<std::vector<T>, T>;

	template<typename T>
	using linked_list = generic_container<std::list<T>, T>;
}
