#include "flood_throttle.h"
#include "time.h"

namespace cbl
{
	// =============================================================================
	// -----------------------------------------------------------------------------
	void flood_throttle::add_host( const ip_address& addr, const time& duration )
	{
		m_data[addr] = time::now() + duration;
	}

	// =============================================================================
	// -----------------------------------------------------------------------------
	void flood_throttle::remove_host( const ip_address& addr )
	{
		m_data.erase( m_data.find( addr ));
	}

	// =============================================================================
	// -----------------------------------------------------------------------------
	bool flood_throttle::is_host_throttled( const ip_address& addr )
	{
		const time			now = time::now();
		vector<ip_address>	expired;
		bool				throttled = false;

		for( const auto& val : m_data )
		{
			if( val.second < now )
			{
				// This throttle expired
				expired << val.first;
				continue;
			}

			if( val.first == addr )
			{
				// Address given has been found in throttle list
				throttled = true;
			}
		}

		for( const ip_address& addr : expired )
			remove_host( addr );

		return throttled;
	}
}