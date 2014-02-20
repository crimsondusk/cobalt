#pragma once
#include "main.h"
#include "variant.h"

namespace cbl
{
	class ip_address;

	class tcp_socket
	{
		PROPERTY (public, bool,	is_blocking,	set_blocking,	STOCK_WRITE)
		PROPERTY (public, bool,	is_verbose,		set_verbose,	STOCK_WRITE)

	public:
		tcp_socket();
		virtual ~tcp_socket();

		bool connect_to_host( const ip_address& addr );
		bool connect_to_host( const cbl::string& node, cbl::uint16 port );
		ssize_t write( const cbl::string& msg );
		bool tick();

		virtual void incoming (const string& token) = 0;

	private:
		int		m_socket;
		string	m_token;
		string	m_error;
	};
}
