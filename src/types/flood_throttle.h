#pragma once
#include "main.h"
#include "ip.h"

namespace cbl
{
	class flood_throttle
	{
		public:
			void add_host( const cbl::ip_address& addr, const cbl::time& duration );
			void remove_host( const cbl::ip_address& addr );
			bool is_host_throttled( const cbl::ip_address& addr );

		private:
			map<ip_address, time>	m_data;
	};
}