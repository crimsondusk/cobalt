#pragma once
#include <cstdio>
#include <cassert>
#include "macros.h"

namespace cbl
{
	template<typename enum_type>
	class flags
	{
		public:
			explicit flags( int a = 0 ) : m_value( a ) {}
			explicit flags( enum_type e ) : m_value( e ) {}
			flags( flags const& other ) :
				m_value( other.as_int() ) {}

			inline const int& as_int() const
			{
				return m_value;
			}

			inline flags& operator|= ( flags const& other )
			{
				m_value |= other.as_int();
				return *this;
			}

			inline flags& operator&= ( flags const& other )
			{
				m_value &= other.as_int();
				return *this;
			}

			inline flags& operator^= ( flags const& other )
			{
				m_value ^= other.as_int();
				return *this;
			}

			inline flags operator| ( flags const& other ) const
			{
				return as_int() | other.as_int();
			}

			inline flags operator& ( flags const& other ) const
			{
				return as_int() & other.as_int();
			}

			inline flags operator^ ( flags const& other ) const
			{
				return as_int() ^ other.as_int();
			}

			inline flags operator~() const
			{
				return ~as_int();
			}

			inline bool operator== ( flags const& other ) const
			{
				return m_value == other.as_int();
			}

			inline bool operator!= ( flags const& other ) const
			{
				return !operator== ( other );
			}

			inline operator bool() const
			{
				return m_value != 0;
			}

		private:
			int	m_value;
	};
}

#define COBALT_IMPLEMENT_FLAGS( ENUM, FLAGS )	\
using FLAGS = ::cbl::flags<ENUM>;

#define COBALT_IMPLEMENT_FLAG_OPERATORS( FLAGS )					\
FLAGS operator|( FLAGS::enum_type a_, FLAGS::enum_type b_ )			\
{																	\
	return test_flags( (int) a_ | (int) b_ );						\
}																	\
																	\
test_flags operator&( FLAGS::enum_type a_, FLAGS::enum_type b_ )	\
{																	\
	return test_flags( (int) a_ & (int) b_ );						\
}
