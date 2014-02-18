#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include "udp.h"
#include "bytestream.h"
#include "format.h"
#include "flood_throttle.h"

namespace cbl
{
	// -----------------------------------------------------------------------------
	//
	udp_socket::udp_socket() :
		m_blocking( false ) {}

	// -----------------------------------------------------------------------------
	//
	udp_socket::~udp_socket() {}

	// -----------------------------------------------------------------------------
	//
	//    Initializes the socket with the given port
	//
	bool udp_socket::init( uint16 port )
	{
		m_error = "";
		m_socket = socket( AF_INET, SOCK_DGRAM, 0 );

		if( blocking() == false )
		{
			int flags = fcntl( m_socket, F_GETFL, 0 );

			if( flags < 0 || fcntl( m_socket, F_SETFL, flags | O_NONBLOCK ) != 0 )
			{
				m_error = "Unable to set socket as non-blocking";
				return false;
			}
		}

		return true;
	}

	// -----------------------------------------------------------------------------
	//
	//    Binds this socket with the given port for hosting.
	//
	bool udp_socket::bind( uint16 port )
	{
		struct sockaddr_in svaddr;
		memset( &svaddr, 0, sizeof svaddr );
		svaddr.sin_family = AF_INET;
		svaddr.sin_port = htons( port );
		svaddr.sin_addr.s_addr = htonl( INADDR_ANY );

		if( ::bind( m_socket, reinterpret_cast<struct sockaddr*>( &svaddr ), sizeof svaddr ) == -1 )
		{
			m_error = string( "Couldn't bind to port " ) + (int) port;
			return false;
		}

		return true;
	}

	// -----------------------------------------------------------------------------
	//
	// Ticks this socket. This must be called repeadetly for the socket to function.
	//
	bool udp_socket::tick()
	{
		struct sockaddr_in claddr;
		socklen_t socklen = sizeof claddr;

		char packet[5120];
		int n = recvfrom( m_socket, packet, sizeof packet, 0,
			reinterpret_cast<struct sockaddr*>( &claddr ), &socklen );

		if( n == -1 )
		{
			// We got an error, though EWOULDBLOCK is silent as it means no packets recieved.
			if( errno != EWOULDBLOCK )
				perror( "CoUDPSocket: recvfrom error:" );

			return;
		}

		ip_address addr( ntohl( claddr.sin_addr.s_addr ), ntohs( claddr.sin_port ) );

		// Ignore this request if the sender is throttled
		if( m_throttle.is_host_throttled( addr ))
		{
			if( is_verbose() )
				print( "-- Ignoring UDP packet from throttled host %1\n", addr );

			return;
		}

		bytestream in( packet, n );
		incoming( in, addr );
	}

	// -----------------------------------------------------------------------------
	//
	//    Launches the given bytestream packet to the specified IP address.
	//
	bool udp_socket::launch( const bytestream& data, const ip_address& addr )
	{
		struct sockaddr_in claddr = addr.to_sockaddr_in();

		int res = sendto( m_socket, data.data(), data.length(), 0,
						  reinterpret_cast<struct sockaddr*>( &claddr ), sizeof claddr );

		if( res == -1 )
		{
			if( is_verbose() )
				perror( "Unable to launch packet" );

			return false;
		}

		return true;
	}

	// -----------------------------------------------------------------------------
	//
	//    Adds the given IP address to the flood queue for the given amount of
	//    seconds. All messages from throttled addresses are ignored.
	//
	void udp_socket::throttle( CoIPAddress addr, int secs )
	{
		m_throttle.add_host( addr, time::now() + secs );
	}
}
