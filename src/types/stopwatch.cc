#include "stopwatch.h"

namespace cbl
{
	// -----------------------------------------------------------------------------
	//
	stopwatch::stopwatch()
	{
		m_running = false;
	}

	// -----------------------------------------------------------------------------
	//
	void stopwatch::start()
	{
		clock_gettime( CLOCK_REALTIME, &m_start );
		m_running = true;
	}

	// -----------------------------------------------------------------------------
	//
	void stopwatch::stop()
	{
		if( m_running )
			clock_gettime( CLOCK_REALTIME, &m_now );

		m_running = false;
	}

	// -----------------------------------------------------------------------------
	//
	time stopwatch::elapsed() const
	{
		if( m_running )
			clock_gettime( CLOCK_REALTIME, &m_now );

		return time( m_now ) - time( m_start );
	}

	// -----------------------------------------------------------------------------
	//
	time stopwatch::reset()
	{
		time retv = elapsed();
		start();
		return retv;
	}
}