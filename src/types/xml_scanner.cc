#include "xml_scanner.h"
#include "xml_document.h"

static const char* g_XMLTokens[] =
{
	"<?xml",			// HeaderStart
	"?>",				// HeaderEnd
	"</",				// TagCloser
	"/>",				// TagSelfCloser
	"<",				// TagStart
	">",				// TagEnd
	// ----------------------------------
	"CData",			// tk_cdata,
	"Equals sign",		// EEquals,
	"a symbol",			// ESymbol
	"a string",			// EString
};

// Amount of tokens which are processed from tokens above
static const int	gNumNamedTokens = 6;
static const char*	gCDataStart   = "<![CDATA[";
static const char*	gCDataEnd     = "]]>";
static const char*	gCommentStart = "<!--";
static const char*	gCommentEnd   = "-->";

namespace cbl
{
	// =============================================================================
	//
	xml_scanner::xml_scanner( const char* data ) :
		m_data( data ),
		m_position( &m_data[0] ),
		m_is_inside_tag( false ),
		m_line_number( 0 ) {}

	// =============================================================================
	//
	bool xml_scanner::check_string( const char* c, bool peek )
	{
		const bool r = strncmp( position(), c, strlen( c )) == 0;

		if( r && !peek )
			set_position( position() + strlen( c ));

		return r;
	}

	// =============================================================================
	//
	bool xml_scanner::scan_next_token()
	{
		set_token_string( "" );

		while( isspace( *position() ))
		{
			if( *position() == '\n' )
				set_line_number( line_number() + 1 );

			increase_position();
		}

		if( *position() == '\0' )
			return false;

		// Skip any comments
		while( check_string( gCommentStart ))
			while( !check_string( gCommentEnd ))
				increase_position();

		// Check and parse CDATA
		if( check_string( gCDataStart ))
		{
			while( !check_string( gCDataEnd ))
			{
				set_token_string( token_string() + *position() );
				increase_position();
			}

			set_token_type( tk_cdata );
			return true;
		}

		// Check "<", ">", "/>", ...
		for( int i = 0; i < gNumNamedTokens; ++i )
		{
			if( check_string( g_XMLTokens[i] ))
			{
				set_token_string( g_XMLTokens[i] );
				set_token_type(( token ) i );

				// We need to keep track of when we're inside node tags so we can
				// stop on '=' signs for attributes when inside tags where '=' has
				// special meaning but not outside tags where it's just a glyph.
				if( i == tk_tag_start || i == tk_header_start )
					set_inside_tag( true );
				else if( i == tk_tag_end || tk_tag_self_closer || i == tk_header_end )
					set_inside_tag( false );

				return true;
			}
		}

		// Check and parse string
		if( *position() == '\"' )
		{
			increase_position();

			while( *position() != '\"' )
			{
				if( !*position() )
					return false;

				if( check_string( "\\\"" ))
				{
					set_token_string( token_string() + "\"" );
					continue;
				}

				set_token_string( token_string() + *position() );
				increase_position();
			}

			set_token_type( tk_string );
			increase_position(); // skip the final quote
			return true;
		}

		set_token_type( tk_symbol );

		while( *position() != '\0' )
		{
			if( is_inside_tag() && isspace( *position() ))
				break;

			// Stop at '=' if inside tag
			if( is_inside_tag() && *position() == '=' )
			{
				if( token_string().length() > 0 )
					break;
				else
				{
					set_token_type( tk_equals );
					set_token_string( "=" );
					increase_position();
					return true;
				}
			}

			bool stopHere = false;

			for( int i = 0; i < ( signed )( sizeof g_XMLTokens / sizeof * g_XMLTokens ); ++i )
			{
				if( check_string( g_XMLTokens[i], true ))
				{
					stopHere = true;
					break;
				}
			}

			if( stopHere )
				break;

			set_token_string( token_string() + *position() );
			increase_position();
		}

		return true;
	}

	// =============================================================================
	//
	bool xml_scanner::scan_next_token( token tok )
	{
		const char* old_position = position();

		if( !scan_next_token() )
			return false;

		if( token_type() != tok )
		{
			set_position( old_position );
			return false;
		}

		return true;
	}

	// =============================================================================
	//
	void xml_scanner::must_scan_next( token tok )
	{
		if( !scan_next_token( tok ))
			throw format( "Expected '%1', got '%2' instead", g_XMLTokens[tok], token_string() );
	}
}
