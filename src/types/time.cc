#include <sys/time.h>
#include <time.h>
#include "main.h"
#include "time.h"
#include "format.h"
#include "variant.h"
#include "misc.h"

namespace cbl
{
	// -----------------------------------------------------------------------------
	//
	time::time( time_t val )
	{
		timespec a;
		a.tv_nsec = 0;
		a.tv_sec = val;
		set_spec( a );
	}

	// -----------------------------------------------------------------------------
	//
	time time::now()
	{
		timespec t;
		clock_gettime( CLOCK_REALTIME, &t );
		return time( t );
	}

	// -----------------------------------------------------------------------------
	//
	time time::diff( const time& other ) const
	{
		const timespec& a = spec(),
						b = other.spec();
		timespec c;

		c.tv_nsec = a.tv_nsec - b.tv_nsec;
		c.tv_sec = a.tv_sec - b.tv_sec;

		if( c.tv_nsec < 0 )
		{
			c.tv_nsec += cbl::g_billion;
			c.tv_sec--;
		}

		return time( c );
	}

	// -----------------------------------------------------------------------------
	//
	time_t time::seconds() const
	{
		return spec().tv_sec;
	}

	// -----------------------------------------------------------------------------
	//
	time time::operator+( const time& other ) const
	{
		const timespec& a = spec(),
						b = other.spec();
		timespec c;

		c.tv_nsec = a.tv_nsec + b.tv_nsec;
		c.tv_sec = a.tv_sec + b.tv_sec;

		if( c.tv_nsec >= g_billion )
		{
			c.tv_nsec -= g_billion;
			c.tv_sec++;
		}

		return time( c );
	}

	// -----------------------------------------------------------------------------
	//
	time time::operator-( const time& other ) const
	{
		return diff( other );
	}

	// -----------------------------------------------------------------------------
	//
	static void describe_helper( string& rep, time_t& n, time_t block, bool twodigit )
	{
		int numblocks = n / block;

		if( numblocks > 0 || twodigit )
		{
			if( rep.is_empty() == false )
				rep += ":";

			if( twodigit )
			{
				char digits[16];
				sprintf( digits, "%.2d", numblocks );
				rep += digits;
			}
			else
				rep += format( "%1", numblocks );

			n -= numblocks * block;
		}
	}

	// -----------------------------------------------------------------------------
	//
	string time::describe() const
	{
		time_t n = seconds();
		string rep;

		describe_helper( rep, n, day,		false );
		describe_helper( rep, n, hour,		false );
		describe_helper( rep, n, minute,	true );
		describe_helper( rep, n, 1,			true );

		const int msecs = milliseconds();

		if( msecs > 0 )
		{
			char x[16];
			sprintf( x, ".%.4d", msecs );
			rep += x;
		}

		return rep;
	}

	// -----------------------------------------------------------------------------
	//
	static void describe_full_helper( string& rep, time_t& secs,
									time_t block, const char* unit )
	{
		if( secs < block )
			return;

		int numblocks = secs / block;

		if( rep.is_empty() == false )
			rep += ", ";

		rep += format( "%1 %2%3", numblocks, unit, numblocks == 1 ? "" : "s" );
		secs -= numblocks * block;
	}

	// -----------------------------------------------------------------------------
	//
	string time::describe_full() const
	{
		string rep;
		time_t secs = seconds();

		describe_full_helper( rep, secs, year, "year" );
		describe_full_helper( rep, secs, day, "day" );
		describe_full_helper( rep, secs, hour, "hour" );
		describe_full_helper( rep, secs, minute, "minute" );
		describe_full_helper( rep, secs, 1, "second" );
		return rep;
	}

	// -----------------------------------------------------------------------------
	//
	long time::milliseconds() const
	{
		return( seconds() * 1000 ) + ( spec().tv_nsec / 1000000L );
	}

	// -----------------------------------------------------------------------------
	//
	void time::set_milliseconds( long val )
	{
		timespec v = spec();
		v.tv_nsec = val * 1000000L;
		set_spec( v );
	}

	// -----------------------------------------------------------------------------
	//
	int time::compare( const time& other ) const
	{
		if( seconds() != other.seconds() )
			return seconds() > other.seconds() ? 1 : -1;

		if( milliseconds() != other.milliseconds() )
			return milliseconds() > other.milliseconds() ? 1 : -1;

		return 0;
	}

	// -----------------------------------------------------------------------------
	//
	bool time::operator>( const time& other ) const
	{
		return compare( other ) > 0;
	}

	// -----------------------------------------------------------------------------
	//
	bool time::operator<( const time& other ) const
	{
		return compare( other ) < 0;
	}

	// -----------------------------------------------------------------------------
	//
	bool time::operator==( const time& other ) const
	{
		return compare( other ) == 0;
	}

	// -----------------------------------------------------------------------------
	//
	bool time::operator!=( const time& other ) const
	{
		return compare( other ) != 0;
	}

	// -----------------------------------------------------------------------------
	//
	bool time::operator>=( const time& other ) const
	{
		return compare( other ) >= 0;
	}

	// -----------------------------------------------------------------------------
	//
	bool time::operator<=( const time& other ) const
	{
		return compare( other ) <= 0;
	}
}
