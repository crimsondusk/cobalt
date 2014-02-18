#ifndef LIBCOBALT_IP_H
#define LIBCOBALT_IP_H

#include "main.h"
#include "types.h"
#include "string.h"

struct sockaddr;

namespace cbl
{
	class ip_address;
	class ip_range;

	// =============================================================================
	//
	// Abstract class to contain methods common to both IP addresses and IP ranges.
	//
	class abstract_ip
	{
		PROPERTY( public, uint32, address, set_address, STOCK_WRITE )

		public:
			uint8 get_octet( int n ) const;
			void set_octet( int n, uint8 oct );

			inline uint8 operator[]( int n ) const
			{
				return get_octet( n );
			}
	};

	// =============================================================================
	//
	// IP address, with port
	//
	class ip_address : public abstract_ip
	{
		PROPERTY( public, uint16, port, set_port, STOCK_WRITE )

		public:
			enum
			{
				WithPort = false,
				WithoutPort = true,

				NoError = 0
			};

			static const uint16 any_port = 0;

			ip_address();
			ip_address( uint32 addr, uint16 port );
			ip_address( const ip_address& other );
			ip_address( string const& ipstring );

			static bool from_string( string input, ip_address& value );
			static int resolve( string node, ip_address& val );
			string to_string( bool withport ) const;
			bool matches( ip_range range ) const;

			bool compare( const ip_address& other ) const;
			bool operator<( const ip_address& other ) const;
			struct sockaddr_in to_sockaddr_in() const;

			inline bool operator== ( const ip_address& other ) const
			{
				return compare( other );
			}

			inline bool operator!= ( const ip_address& other ) const
			{
				return !operator== ( other );
			}
	};

	// =============================================================================
	// CoIPRange
	//
	// IP range, with wildcard and expiration support.
	// Wildcards are stored in 4 bits: if bit N is set, octet N is a wildcard.
	// =============================================================================
	class ip_range : public abstract_ip
	{
		PROPERTY( public,	uint8,	wildcards,		set_wildcards,		STOCK_WRITE )
		PROPERTY( public,	time_t,	expire_time,	set_expire_time, 	STOCK_WRITE )
		PROPERTY( public,	string,	comment,		set_comment,		STOCK_WRITE )

		public:
			ip_range() {}
			ip_range( uint32 addr, uint8 wildcards );
			ip_range( string const& input );

			bool from_string( string const& input );
			string describe() const;
			void set_octect_wildcarded( short i );
			string to_string() const;
			bool is_octect_wildcarded( short i ) const;
	};

	extern const ip_address localhost;
}

#endif // LIBCOBALT_IP_H
