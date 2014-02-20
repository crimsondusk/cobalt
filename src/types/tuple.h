#pragma once
#include <tuple>
#include "string.h"
#include "variant.h"

namespace cbl
{
	template<typename... T>
	class tuple;

	/**
	 *	@class cbl::tuple_describe_helper
	 *	@brief A private, internal class to assist in describing tuples. Do not use.
	 */
	template<int N, typename... T>
	class tuple_describe_helper
	{
		public:
			tuple_describe_helper(){}
			void work( tuple<T...>& tup, string& description );
	};

	template<typename... T>
	class tuple_describe_helper<0, T...>
	{
		public:
			tuple_describe_helper(){}
			void work( tuple<T...>& tup, string& description );
	};

	/**
	 *	@class cbl::tuple
	 *	@brief A wrapper around std::tuple.
	 *
	 *	The cbl::tuple class provides a tuple as defined by STL.
	 */
	template<typename... T>
	class tuple
	{
	private:
		std::tuple<T...>	m_tuple;

	public:
		template<size_t N>
		using type_index = decltype( std::get<N>( m_tuple ));

		/**
		 *	Constructs an empty tuple.
		 */
		tuple() {}

		/**
		 *	Constructs a tuple with the given elements.
		 *	@param elements the elements to build the tuple with.
		 */
		tuple( const T&... elements ) :
			m_tuple( elements... ) {}

		/**
		 *	Yields a single value from the tuple.
		 *	@param N the index of the element to get.
		 *	@return the value of the respective element
		 */
		template<size_t N>
		type_index<N> get()
		{
			return std::get<N>( m_tuple );
		}

		/**
		 *	@return The underlying std::tuple.
		 */
		const std::tuple<T...>& std_tuple() const
		{
			return m_tuple;
		}

		/**
		 *	@return The amount of elements in the tuple.
		 */
		constexpr size_t size() const
		{
			return sizeof...( T );
		}

		/**
		 *	Yields a printable description of the tuple.
		 *	@return a string representing the tuple.
		 */
		string describe()
		{
			string a;
			tuple_describe_helper<sizeof...(T) - 1, T...>().work( *this, a );
			return "{" + a + "}";
		}
	};

	template<int N, typename... T>
	void tuple_describe_helper<N, T...>::work( tuple<T...>& tup, string& description )
	{
		tuple_describe_helper<N - 1, T...>().work( tup, description );

		if( description.is_empty() == false )
			description += ", ";

		description += variant( std::get<N>( tup.std_tuple() )).describe();
	}

	template<typename... T>
	void tuple_describe_helper<0, T...>::work( tuple<T...>& tup, string& description )
	{
		if( description.is_empty() == false )
			description += ", ";

		description += variant( std::get<0>( tup.std_tuple() )).describe();
	}
}