#pragma once
#include <tuple>
#include "string.h"
#include "variant.h"

namespace cbl
{
	template<typename... T>
	class tuple;

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

	template<typename... T>
	class tuple
	{
	private:
		std::tuple<T...>	m_tuple;

	public:
		template<size_t N>
		using type_index = decltype( std::get<N>( m_tuple ));

		tuple() {}
		tuple( const T&... elements ) :
			m_tuple( elements... ) {}

		template<size_t N>
		type_index<N> get()
		{
			return std::get<N>( m_tuple );
		}

		const std::tuple<T...>& std_tuple() const
		{
			return m_tuple;
		}

		constexpr size_t size() const
		{
			return sizeof...( T );
		}

	public:
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