#pragma once
#include <deque>
#include <string>
#include <stdarg.h>
#include "types.h"
#include "containers.h"

namespace cbl
{
	class string_list;

	class string
	{
		public:
			using iterator			= typename ::std::string::iterator;
			using const_iterator	= typename ::std::string::const_iterator;

			inline string() {}

			inline string( const char* data ) :
				m_string( data ) {}

			inline string( const std::string& data ) :
				m_string( data ) {}

			int					count( char needle ) const;
			void				dump() const;
			bool				ends_with( const string& other ) const;
			int					first( const string& c, int a = 0 ) const;
			int					last( const string& c, int a = -1 ) const;
			string				to_lowercase() const;
			bool				is_numeric() const;
			int					get_word_position( int n ) const;
			bool				mask( const cbl::string& mask ) const;
			void				replace( const string& a, const string& b );
			string_list			split( const string& del ) const;
			bool				starts_with( const string& other ) const;
			string				strip( std::initializer_list<char> unwanted );
			string				get_substring( int a, int b = -1 ) const;
			double				to_double( bool* ok = null ) const;
			float				to_float( bool* ok = null ) const;
			long				to_long( bool* ok = null, int base = 10 ) const;
			void				trim( int n );
			string				to_uppercase() const;

			string				operator+ ( const string data ) const;
			string				operator+ ( const char* data ) const;
			string&				operator+= ( const string data );
			string&				operator+= ( const char* data );
			string				operator- ( int n ) const;

			static string		from_number( int i );
			static string		from_number( long i );
			static string		from_number( uint i );
			static string		from_number( ulong i );
			static string		from_number( float i );
			static string		from_number( double i );

			inline int compare( const string& other ) const
			{
				return m_string.compare( other.std_string() );
			}

			inline void append( const char* data )
			{
				m_string.append( data );
			}

			inline void append( char data )
			{
				m_string.push_back( data );
			}

			inline iterator begin()
			{
				return m_string.begin();
			}

			inline const_iterator begin() const
			{
				return m_string.cbegin();
			}

			inline int capacity() const
			{
				return m_string.capacity();
			}

			inline const char* c_str() const
			{
				return m_string.c_str();
			}

			inline iterator end()
			{
				return m_string.end();
			}

			inline const_iterator end() const
			{
				return m_string.end();
			}

			inline void clear()
			{
				m_string.clear();
			}

			inline bool is_empty() const
			{
				return m_string.empty();
			}

			inline void remove_at( int pos )
			{
				m_string.erase( m_string.begin() + pos );
			}

			inline void insert( int pos, char c )
			{
				m_string.insert( m_string.begin() + pos, c );
			}

			inline int length() const
			{
				return m_string.length();
			}

			inline int max_size() const
			{
				return m_string.max_size();
			}

			inline void resize( int n )
			{
				m_string.resize( n );
			}

			inline void remove_range( int pos, int n )
			{
				m_string.replace( pos, n, "" );
			}

			inline void shrink_to_fit()
			{
				m_string.shrink_to_fit();
			}

			inline const std::string& std_string() const
			{
				return m_string;
			}

			inline string strip( char unwanted )
			{
				return strip({ unwanted });
			}

			inline string& operator+= ( const char data )
			{
				append( data );
				return *this;
			}

			inline string operator+() const
			{
				return to_uppercase();
			}

			inline string operator-() const
			{
				return to_lowercase();
			}

			inline string& operator-= ( int n )
			{
				trim( n );
				return *this;
			}

			inline bool operator== ( const string& other ) const
			{
				return compare( other ) == 0;
			}

			inline bool operator== ( const char* other ) const
			{
				return compare( other ) == 0;
			}

			inline bool operator!= ( const string& other ) const
			{
				return compare( other ) != 0;
			}

			inline bool operator!= ( const char* other ) const
			{
				return compare( other ) != 0;
			}

			inline bool operator!() const
			{
				return is_empty();
			}

			operator const char*() const
			{
				return c_str();
			}

			operator const std::string&() const
			{
				return std_string();
			}

		private:
			std::string m_string;
	};

	inline bool operator==( const char* a, const string& b )
	{
		return b == a;
	}

	inline string operator+( const char* a, const string& b )
	{
		return string( a ) + b;
	}
}
