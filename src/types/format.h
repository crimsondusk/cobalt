#pragma once
#include "containers.h"
#include "variant.h"
#include "tuple.h"

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
		// format_arg( const variant& a ) : m_text( a.describe() ) {}

		template<class T, class R>
		explicit format_arg( const generic_container<T, R>& a )
		{
			string contents;

			for( const auto& it : a )
			{
				if( contents.is_empty() == false )
					contents += ", ";

				contents += variant( it ).describe();
			}

			m_text = "{" + contents + "}";
		}

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

	string format_args( const string& fmtstr, const vector<string>& args );

	// -------------------------------------------------------------------------
	//
	//    Formats the given @fmtstr with @args and returns the formatted string
	//
	template<typename T>
	string describe_array( const T& array );

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
};