#pragma once
#include <ctime>
#include "main.h"
#include "string.h"

namespace cbl
{
	static const long
		minute = 60l * 1l,
		hour   = 60l * minute,
		day    = 24l * hour,
		week   = 7l * day,
		month  = 30l * day,
		year   = 365l * day;

	class time
	{
		PROPERTY( private, timespec, spec, set_spec, STOCK_WRITE )

		public:
			time( time_t val = 0 );
			time( timespec val ) : m_spec( val ) {}

			time	diff( const time& other ) const;
			string	describe() const;
			string	describe_full() const;
			time_t	seconds() const;
			long	milliseconds() const;
			void	set_milliseconds( long val );
			int		compare( const time& other ) const;

			static time now();

			time operator+ ( const time& other ) const;
			time operator- ( const time& other ) const;
			bool operator> ( const time& other ) const;
			bool operator< ( const time& other ) const;
			bool operator>= ( const time& other ) const;
			bool operator<= ( const time& other ) const;
			bool operator== ( const time& other ) const;
			bool operator!= ( const time& other ) const;
	};
}
