#pragma once
#include <map>
#include "main.h"
#include "ip.h"
#include "time.h"

namespace cbl
{
	class udp_socket
	{
		PROPERTY( public, bool, blocking, set_blocking, STOCK_WRITE )

		public:
			udp_socket();
			virtual ~udp_socket();

			bool init( uint16 port );
			bool bind( cbl::uint16 port );
			bool tick();
			void launch( const bytestream& data, const cbl::ip_address& addr );
			void throttle( CoIPAddress addr, int secs );

			virtual void incoming( const Bytestream& data, CoIPAddress addr ) = 0;

		private:
			flood_throttle	m_throttle;
			ip_address		m_addr;
			string			m_error;
			int				m_socket;
	};
}
