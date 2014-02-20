#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netdb.h>
#include "tcp.h"
#include "variant.h"
#include "format.h"
#include "ip.h"

namespace cbl
{
	// -----------------------------------------------------------------------------
	//
	tcp_socket::tcp_socket() :
		m_is_verbose( false ) {}

	// -----------------------------------------------------------------------------
	//
	tcp_socket::~tcp_socket() {}

	// -----------------------------------------------------------------------------
	//
	//    Overload for using a @cbl::ip_address as the connect_to_host parameter
	//
	bool tcp_socket::connect_to_host( const ip_address& addr )
	{
		return connect_to_host( addr.to_string( false ), addr.port() );
	}

	// -----------------------------------------------------------------------------
	//
	//    Attempts a TCP connection to node:host. Returns @true if connection was
	//    successful, @false if not. If connection failed, @error_string() will
	//    return a meaningful error string.
	//
	bool tcp_socket::connect_to_host( const string& node, uint16 port )
	{
		print( "Attempting connection to %1:%2\n", node, port );

		struct addrinfo hints;
		struct addrinfo* result;
		struct addrinfo* it;
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;
		hints.ai_protocol = 0;

		string portstring = string::from_number( port );
		int errorcode = getaddrinfo( node, portstring, &hints, &result );

		if( errorcode != 0 )
		{
			m_error = string( "Name lookup error: " ) + gai_strerror( errorcode );
			return false;
		}

		for( it = result; it != null; it = it->ai_next )
		{
			m_socket = socket( it->ai_family, it->ai_socktype, it->ai_protocol );

			if( m_socket == -1 )
				continue;

			if( connect( m_socket, it->ai_addr, it->ai_addrlen ) != -1 )
				break;

			close( m_socket );
		}

		if( it == null )
		{
			m_error = string( "Connection failed (" ) + strerror( errno ) + ")";
			abort();
		}

		int flags = fcntl( m_socket, F_GETFL, 0 );

		if( flags < 0 )
		{
			m_error = "Got invalid file control flags while setting socket as non-blocking";
			close( m_socket );
			return false;
		}

		if( fcntl( m_socket, F_SETFL, flags | O_NONBLOCK ) != 0 )
		{
			m_error = "Unable to set socket as non-blocking";
			close( m_socket );
			return false;
		}

		return true;
	}

	// -----------------------------------------------------------------------------
	//
	//    Waits up to 50 msecs for data. If any data was recieved, incoming() is
	//    called.
	//
	//    Returns @false if an error occurred. Note: it will return @true even if
	//    no data is recieved, as the tick is still considered successful.
	//
	//    If @false is returned, @error_string() will return the appropriate
	//    error string.
	//
	bool tcp_socket::tick()
	{
		char buf[5120];

		{
			// Wait 0.05 seconds for the data
			struct timeval wait;
			fd_set set;
			FD_ZERO( &set );
			FD_SET( m_socket, &set );
			wait.tv_sec = 0;
			wait.tv_usec = 50000;

			if( select( m_socket, &set, null, null, &wait ) == -1 )
				return true;
		}

		long readsize = ::read( m_socket, buf, sizeof buf );

		if( readsize == -1 )
		{
			if( errno != EWOULDBLOCK )
			{
				m_error = string( "read failure: " ) + strerror( errno );
				return false;
			}

			return true;
		}

		if( readsize > 0 )
		{
			// Tokenize what we got into lines
			for( char* i = buf; i < buf + readsize; i++ )
			{
				if( *i == '\n' )
				{
					if( is_verbose() )
						print( "-> %1\n", m_token );

					incoming( m_token );
					m_token = "";
				}
				else
					m_token += *i;
			}
		}

		return true;
	}

	// -----------------------------------------------------------------------------
	//
	//    Writes the given message to the socket. Returns the amount of bytes
	//    written or -1 if there was an error.
	//
	ssize_t tcp_socket::write( const cbl::string& msg )
	{
		string msg_to_send = msg + '\n';

		if( is_verbose() )
			print( "<- %1", msg_to_send );

		return ::write( m_socket, msg_to_send.c_str(), msg_to_send.length() );
	}
}
