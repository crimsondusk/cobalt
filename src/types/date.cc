#include <sys/time.h>
#include <time.h>
#include "date.h"
#include "time.h"

namespace cbl
{
	// -----------------------------------------------------------------------------
	//
	static const char* g_month_names[] =
	{
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};

	// -----------------------------------------------------------------------------
	//
	static const char* g_week_day_names[] =
	{
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday"
	};

	// =============================================================================
	date::date()
	{
		time_t secs = time().seconds();
		set_tm( localtime( &secs ));
	}

	date::date( const time& t )
	{
		time_t secs = t.seconds();
		set_tm( localtime( &secs ));
	}

	int date::year() const
	{
		return tm()->tm_year + 1900;
	}

	date::e_month date::month() const
	{
		int monum = tm()->tm_mon;
		assert( monum >= 0 && monum <= 11 );

		return ( e_month )( monum );
	}

	date::e_weekday date::weekday() const
	{
		int daynum = tm()->tm_wday;
		assert( daynum >= 0 && daynum <= 6 );
		return ( e_weekday ) daynum;
	}

	int date::day() const
	{
		return tm()->tm_yday;
	}

	int date::hour() const
	{
		return tm()->tm_hour;
	}

	int date::minute() const
	{
		return tm()->tm_min;
	}

	int date::second() const
	{
		return tm()->tm_sec;
	}

	int date::timezone() const
	{
		return tm()->tm_gmtoff;
	}

	string date::timezone_name() const
	{
		return string( tm()->tm_zone );
	}

	bool date::is_dst() const
	{
		return tm()->tm_isdst;
	}

	void date::set_year( int val )
	{
		m_tm->tm_year = val - 1900;
	}

	void date::set_month( date::e_month val )
	{
		m_tm->tm_mon = ( int ) val;
	}

	void date::set_day( int val )
	{
		m_tm->tm_mday = val;
	}

	void date::set_weekday( e_weekday a )
	{
		m_tm->tm_wday = ( int ) a;
	}

	void date::set_hour( int a )
	{
		m_tm->tm_hour = a;
	}

	void date::set_minute( int a )
	{
		m_tm->tm_min = a;
	}

	void date::set_second( int a )
	{
		m_tm->tm_sec = a;
	}

	void date::set_timezone( int a )
	{
		m_tm->tm_gmtoff = a;
	}

	void date::set_dst( bool a )
	{
		m_tm->tm_isdst = a;
	}

	string date::describe() const
	{
		string rep = asctime( tm() );

		// asctime is stupid and adds a new-line character
		// after the message. Remove it.
		if( rep[rep.length() - 1] == '\n' )
			rep -= 1;

		return rep;
	}

	string date::month_name( date::e_month val )
	{
		return g_month_names[val];
	}

	string date::month_name() const
	{
		return g_month_names[month()];
	}

	string date::weekday_name( date::e_weekday val )
	{
		return g_week_day_names[val];
	}

	string date::weekday_name() const
	{
		return g_week_day_names[weekday()];
	}

	date date::gmt( time t )
	{
		time_t now = t.seconds();
		struct tm* gmtnow = gmtime( &now );
		return date( gmtnow );
	}
}
