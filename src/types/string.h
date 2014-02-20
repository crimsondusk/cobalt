#pragma once
#include <deque>
#include <string>
#include <stdarg.h>
#include "types.h"
#include "containers.h"

namespace cbl
{
	class string_list;

	/**
	 *	\class cbl::string
	 *	\brief A thin wrapper around std::string.
	 *
	 *	The cbl::string class provdes a wrapper around std::string. It provides
	 *	string manipulation and convenience functions.
	 */
	class string
	{
		public:
			using iterator			= typename ::std::string::iterator;
			using const_iterator	= typename ::std::string::const_iterator;

			/*!
			 *	Constructs an empty string
			 */
			inline string() {}

			/*!
			 * Constructs a string with the given data.
			 * @param data the data to construct the string with.
			 */
			inline string( const char* data ) :
				m_string( data ) {}

			/*!
			 * Constructs a string with the given data.
			 * @param data the data to construct the string with.
			 */
			inline string( const std::string& data ) :
				m_string( data ) {}

			/*!
			 * Counts the amount of needles in the string.
			 * @param needle the needle to count the amount of.
			 * @return the amount of needles in the string.
			 */
			int					count( char needle ) const;
			void				dump() const;

			/*!
			 *	Checks whether the string ends with the given string.
			 *	@param other the string to check
			 *  @return true if the string ends with other.
			 */
			bool				ends_with( const string& other ) const;

			/*!
			 *	Finds the first found index of the given substring.
			 *	@param c the substring to try to find in the string.
			 *	@param a the position where to start looking for the string from.
			 *	@return the index of the character where c starts at. -1 if not found.
			 */
			int					find_first( const string& c, int a = 0 ) const;

			/*!
			 *	Finds the last found index of the given substring.
			 *	@param c the substring to try to find in the string.
			 *	@param a the position where to start looking from.
			 *	@return the index of the character where c starts at. -1 if not found.
			 */
			int					find_last( const string& c, int a = -1 ) const;

			/*!
			 *	Converts the string to lowercase.
			 *	@return the string in lowercase.
			 */
			string				to_lowercase() const;
			bool				is_numeric() const;
			int					get_word_position( int n ) const;
			bool				mask( const cbl::string& mask ) const;
			void				replace( const string& a, const string& b );
			string_list			split( const string& del ) const;
			bool				starts_with( const string& other ) const;
			string				strip( std::initializer_list<char> unwanted );
			string				get_substring( int a, int b = -1 ) const;

			/*!
			 *	Converts the string to a double-precision floating point number.
			 *	@param ok If not null, this is set to a truth value whether the conversion
			 *	was successful or not.
			 *	@param base The base to interpret the string in.
			 *	@returns The interpreted number.
			 */
			double				to_double( bool* ok = null ) const;

			/*!
			 *	Converts the string to a floating point number
			 *	@param ok If not null, this is set to a truth value whether the conversion
			 *	was successful or not.
			 *	@returns The interpreted number.
			 */
			float				to_float( bool* ok = null ) const;

			/*!
			 *	Converts the string to long integer
			 *	@param ok If not null, this is set to a truth value whether the conversion
			 *	was successful or not.
			 *	@param base The base to interpret the string in.
			 *	@returns The interpreted number.
			 */
			long				to_long( bool* ok = null, int base = 10 ) const;

			/*!
			 *	Removes characters at the end of the string.
			 *	@param n the amount of characters to remove
			 */
			void				trim( int n );

			/*!
			 *	Converts the string to uppercase.
			 *	@return the string in uppercase.
			 */
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

			/*!
			 *	@return the string's internal capacity.
			 */
			inline int capacity() const
			{
				return m_string.capacity();
			}

			/*!
			 *	@return the character array of the string
			 */
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

			/*!
			 *	Clears the string.
			 */
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

			inline void replace( int pos, int len, const string& replacement )
			{
				m_string.replace( m_string.begin() + pos, m_string.begin() + pos + len,
					replacement.c_str(), replacement.length() );
			}

			inline void insert( int pos, char c )
			{
				m_string.insert( m_string.begin() + pos, c );
			}

			inline void insert( int pos, const string& a )
			{
				m_string.replace( m_string.begin() + pos, m_string.begin() + pos, a.c_str(), a.length() );
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

			void sprintf( const char* fmtstr, ... )
			{
				char buf[512];
				va_list va;
				va_start( va, fmtstr );
				vsprintf( buf, fmtstr, va );
				va_end( va );

				m_string = buf;
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
