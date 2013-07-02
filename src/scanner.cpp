#include "common.h"
#include "scanner.h"
#include "config.h"

inline bool IsCharWhitespace( char c )
{
	return ( c <= 32 || c == 127 );
}

Scanner::Scanner( str input )
{
	cursor = 0;
	buffer = input;
}

bool Scanner::Next( bool raw )
{
	str work;
	token = "";
	
	for( ;; )
	{
		if( cursor >= buffer.len() )
			break;
		
		char c = buffer[cursor++];
		
		// Possible breakpoint
		if( !raw && (( c >= 33 && c <= 47 ) ||
					  ( c >= 58 && c <= 64 ) ||
					  ( c >= 91 && c <= 96 && c != '_' ) ||
					  ( c >= 123 && c <= 126 )))
		{
			// Slide past crap
			while( cursor < buffer.len() && IsCharWhitespace( c ) )
				cursor++;
			
			// Don't break out if we're at the beginning of the token.
			if( work.len() > 0 )
				cursor--;
			else
				work += c;
			
			break;
		}
		
		if( IsCharWhitespace( c ) )
		{
			// Don't break if we haven't gathered anything yet.
			if( work.len() )
				break;
		}
		else
		{
			work += c;
		}
	}
	
	token = work;
	return ( token.len() > 0 );
}

bool Scanner::MustNext( str stuff )
{
	str old_token = token;
	
	if( !Next() )
	{
		if( stuff.len() )
			fatal( "expected `%1`", stuff );
		else
			fatal( "unexpected end-of-command" );
		
		return false;
	}
	
	if( stuff.len() && token != stuff )
	{
		fatal( "expected `%1` after `%1`", stuff, old_token );
		return false;
	}
	
	return true;
}

bool Scanner::MustString()
{
	MustNext( "\"" );
	token = "";
	
	while( cursor < buffer.len() )
	{
		char c = buffer[cursor++];
		
		if( c == '\"' )
			return true;
		else
			token += c;
	}
	
	fatal( "unterminated string" );
	return false;
}

bool Scanner::MustLiteral()
{
	if( !Next() || token.numeric() )
		return false;
	
	return true;
}

str Scanner::PeekNext( bool raw )
{
	ulong oldcurs = cursor;
	str oldtoken = token;
	
	if( !Next( raw ) )
		return "";
	
	str peeked = token;
	cursor = oldcurs;
	token = oldtoken;
	
	return peeked;
}
