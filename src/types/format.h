#pragma once
#include "containers.h"
#include "variant.h"
#include "tuple.h"
#include "ip.h"
#include "time.h"
#include "date.h"

namespace cbl
{
	// -------------------------------------------------------------------------
	//
	class format_arg
	{
	public:
		format_arg( const string& a ) : m_text( a ) {}
		format_arg( const char* a ) : m_text( a ) {}
		format_arg( int a ) : m_text( string::from_number( a )) {}
		format_arg( long a ) : m_text( string::from_number( a )) {}
		format_arg( ulong a ) : m_text( string::from_number( a )) {}
		format_arg( float a ) : m_text( string::from_number( a )) {}
		format_arg( double a ) : m_text( string::from_number( a )) {}
		format_arg( const ip_address& a ) : m_text( a.to_string() ) {}
		format_arg( const ip_range& a ) : m_text( a.to_string() ) {}
		format_arg( const time& a ) : m_text( a.describe() ) {}
		format_arg( const date& a ) : m_text( a.describe() ) {}

		template<typename T, typename R>
		explicit format_arg( const generic_container<T, R>& a ) :
			m_text( a.describe() ) {}

		template<typename... T>
		explicit format_arg( tuple<T...>& a ) :
			m_text( a.describe() ) {}

		inline const string& text() const
		{
			return m_text;
		}

	private:
		string m_text;
	};

	// -------------------------------------------------------------------------
	//
	//    Expands args into a vector of strings
	//
	template<typename T, typename... argtypes>
	void expand_format_args( vector<string>& data, T arg, const argtypes&... rest )
	{
		data << format_arg( arg ).text();
		expand_format_args( data, rest... );
	}

	static void expand_format_args( vector<string>& data ) CBL_UNUSED;
	static void expand_format_args( vector<string>& data ) {}

	// -------------------------------------------------------------------------
	//
	//    Formats the given @fmtstr with @args and returns the formatted string
	//
	template<typename... argtypes>
	string format( string fmtstr, const argtypes&... raw_args )
	{
		vector<string> args;
		expand_format_args( args, raw_args... );
		assert( args.size() == sizeof...( raw_args ));
		return format_args( fmtstr, args );
	}

	static string format( string fmtstr ) CBL_UNUSED;
	static string format( string fmtstr )
	{
		return fmtstr;
	}

	// -------------------------------------------------------------------------
	//
	//    Prints @fmtstr formatted with @args to @fp
	//
	template<typename... argtypes>
	void print_to( FILE* fp, string fmtstr, const argtypes&... args )
	{
		fprintf( fp, "%s", format( fmtstr, args... ).c_str() );
	}

	// -------------------------------------------------------------------------
	//
	//    Prints @fmtstr formatted with @args to stdout
	//
	template<typename... argtypes>
	void print( string fmtstr, const argtypes&... args )
	{
		print_to( stdout, fmtstr, args... );
	}

	// -------------------------------------------------------------------------
	//
	//    Formats the given @fmtstr with the given @args.
	//
	string format_args( const string& fmtstr, const vector<string>& args );
};