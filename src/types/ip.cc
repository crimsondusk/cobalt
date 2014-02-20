#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include "ip.h"
#include "format.h"
#include "string_list.h"
#include "time.h"

namespace cbl
{

const ip_address localhost( 0x7F000001, 0 );

// -----------------------------------------------------------------------------
//
ip_address::ip_address()
{
	memset( this, 0, sizeof *this );
}

// -----------------------------------------------------------------------------
//
ip_address::ip_address( uint32 addr, uint16 port )
{
	set_address( addr );
	set_port( port );
}

// -----------------------------------------------------------------------------
//
ip_address::ip_address( const ip_address& other )
{
	memcpy( this, &other, sizeof other );
}

// -----------------------------------------------------------------------------
//
ip_address::ip_address( const cbl::string& ipstring )
{
	ip_address::from_string( ipstring, *this );
}

// -----------------------------------------------------------------------------
//
string ip_address::to_string( bool withport ) const
{
	string val;

	for( int i = 0; i < 4; ++i )
	{
		if( i > 0 )
			val += '.';

		val += string::from_number( operator[]( i ));
	}

	if( port() != 0 )
		val += format( ":%1", port() );

	return val;
}

// -----------------------------------------------------------------------------
//
int ip_address::resolve( string node, ip_address& val )
{
	struct addrinfo   hints;
	struct addrinfo*  result;
	int               r;

	memset( &hints, 0, sizeof hints );
	hints.ai_family = AF_INET;

	if( ( r = getaddrinfo( node, null, &hints, &result ) ) != 0 )
		return r;

	for( struct addrinfo* it = &result[0]; it; it = it->ai_next )
	{
		struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>( it->ai_addr );
		val.set_address( ntohl( addr->sin_addr.s_addr ) );
		val.set_port( ntohs( addr->sin_port ) );
		break;
	}

	freeaddrinfo( result );
	return NoError;
}

// -----------------------------------------------------------------------------
//
bool ip_address::from_string( string input, ip_address& value )
{
	uint parts[4];
	const int colonpos = input.find_first( ":" );
	const string addressString = colonpos == -1 ? input : input.get_substring( 0, colonpos );

	value.set_address( 0 );
	value.set_port( 0 );

	// Try scanf the IPv4 address first. If it's not an IP string, it could
	// be a hostname; thus try resolve it.
	if( sscanf( addressString, "%u.%u.%u.%u", &parts[0], &parts[1], &parts[2], &parts[3] ))
	{
		for( short i = 0; i < 4; ++i )
			value.set_octet( i, parts[i] );
	}
	else if( ip_address::resolve( addressString, value ) != NoError )
	{
		return false; // resolution failed
	}

	// Success! Deal with the possible port now.
	if( colonpos != -1 )
		value.set_port( atoi( input.get_substring( colonpos + 1, -1 ) ) );

	return true;
}

// -----------------------------------------------------------------------------
//
bool ip_address::matches( ip_range range ) const
{
	if( address() == range.address() )
		return true; // full match

	for( short i = 0; i < 4; ++i )
	{
		if( range.is_octect_wildcarded( i ) )
			continue; // wildcarded

		if( get_octet( i ) != range[i] )
			return false; // matching failed
	}

	return true;
}

// -----------------------------------------------------------------------------
//
uint8 abstract_ip::get_octet( int n ) const
{
	return ( address() >> ( ( 3 - n ) * 8 ) ) & 0xFF;
}

// -----------------------------------------------------------------------------
//
void abstract_ip::set_octet( int n, uint8 oct )
{
	// TODO: make a big-endian version
	set_address( ( address() & ~( 0xFF << ( 3 - n ) * 8 ) ) | ( oct << ( 3 - n ) * 8 ) ); // :)
}

// -----------------------------------------------------------------------------
//
bool ip_address::compare( const ip_address& other ) const
{
	for( short i = 0; i < 4; ++i )
		if( get_octet( i ) != other[i] )
			return false;

	if( port() != any_port && other.port() != any_port && port() != other.port() )
		return false;

	return true;
}

// -----------------------------------------------------------------------------
//
bool ip_address::operator< ( const ip_address& other ) const
{
	for( int i = 0; i < 4; ++i )
	{
		if( get_octet( i ) < other[i] )
			return true;

		if( get_octet( i ) > other[i] )
			return false;
	}

	return port() < other.port();
}

// -----------------------------------------------------------------------------
//
ip_range::ip_range( uint32 addr, uint8 wildcards )
{
	set_address( addr );
	set_wildcards( wildcards );
}

// -----------------------------------------------------------------------------
//
ip_range::ip_range( const cbl::string& input )
{
	from_string( input );
}

// -----------------------------------------------------------------------------
//
bool ip_range::from_string( const cbl::string& input )
{
	if( input.count( '.' ) != 3 )
		return false;

	set_wildcards( 0 );
	set_expire_time( 0 );

	string token;
	const char* c = &input[0];
	ip_address part;

	while( *c == '*' || *c == '.' || ( *c >= '0' && *c <= '9' ))
		token += *c++;

	if( !ip_address::from_string( token, part ) )
		return false;

	set_address( part.address() );

	// Check to see if there was any wildcards
	string_list octets = token.split( "." );

	for( auto it = octets.begin(); it != octets.end(); ++it )
		if( *it == "*" )
			set_octect_wildcarded( it - octets.begin() );

	if( *c == '<' )
	{
		c++;
		token = "";

		while( *c && *c != '>' )
			token += *c++;

		c++;
		uint month, day, year, hour, minute;

		if( sscanf( token, "%u/%u/%u %u:%u", &month, &day, &year, &hour, &minute ) ||
				sscanf( token, "%u/%u/%u", &month, &day, &year ) )
		{
			// TODO: use cbl::date here
			time_t rawtime = ::time( null );
			struct tm* info = localtime( &rawtime );
			info->tm_year = year - 1900;
			info->tm_mon = month - 1;
			info->tm_mday = day;
			info->tm_sec = 0;

			if( token.find_first( " " ) != -1 )
			{
				info->tm_hour = hour;
				info->tm_min = minute;
			}
			else
				info->tm_hour = info->tm_min = 0;

			set_expire_time( mktime( info ) );
		}
	}

	if( *c == ':' )
		set_comment( ++c );

	return true;
}

// -----------------------------------------------------------------------------
//
string ip_range::to_string() const
{
	string val;

	for( int i = 0; i < 4; ++i )
	{
		if( i > 0 )
			val += '.';

		if( is_octect_wildcarded( i ) )
			val += '*';
		else
			val += string::from_number( get_octet( i ));
	}

	return val;
}

// -----------------------------------------------------------------------------
//
string ip_range::describe() const
{
	string val = to_string();

	if( expire_time() != 0 )
	{
		time_t expr = expire_time();
		struct tm* info = localtime( &expr );

		val += format( "<%.2d/%.2d/%.4d %.2d:%.2d>", info->tm_mon + 1, info->tm_mday,
			info->tm_year + 1900, info->tm_hour, info->tm_min );
	}

	if( comment().length() > 0 )
	{
		val += ":";
		val += comment();
	}

	return val;
}

// -----------------------------------------------------------------------------
//
bool ip_range::is_octect_wildcarded( short i ) const
{
	return wildcards() & ( 1 << i );
}

// -----------------------------------------------------------------------------
//
void ip_range::set_octect_wildcarded( short i )
{
	set_wildcards( wildcards() | ( 1 << i ) );
}

// -----------------------------------------------------------------------------
//
struct sockaddr_in ip_address::to_sockaddr_in() const
{
	struct sockaddr_in claddr;
	memset( &claddr, 0, sizeof claddr );
	claddr.sin_addr.s_addr = htonl( address() );
	claddr.sin_port = htons( port() );
	claddr.sin_family = AF_INET;
	return claddr;
}

}
