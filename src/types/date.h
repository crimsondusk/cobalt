#pragma once
#include "main.h"

namespace cbl
{

class date
{
	PROPERTY( private,	struct tm*,	tm, set_tm,	STOCK_WRITE )

	public:
		date();
		date( const time& t );
		date( struct tm* tminfo ) :
			m_tm( tminfo ) {}

		enum e_month
		{
			january,
			february,
			march,
			april,
			may,
			june,
			july,
			august,
			september,
			october,
			november,
			december
		};

		enum e_weekday
		{
			sunday,
			monday,
			tuesday,
			wednesday,
			thursday,
			friday,
			saturday
		};

		int				day() const;
		string			describe() const;
		int				hour() const;
		bool			is_dst() const;
		int				minute() const;
		e_month			month() const;
		string			month_name() const;
		int				second() const;
		void			set_year( int val );
		void			set_month( e_month val );
		void			set_day( int val );
		void			set_weekday( e_weekday a );
		void			set_hour( int a );
		void			set_minute( int a );
		void			set_second( int a );
		void			set_timezone( int a );
		void			set_dst( bool a );
		string			timezone_name() const;
		int				timezone() const;
		e_weekday		weekday() const;
		string			weekday_name() const;
		int				year() const;

		static string	month_name( e_month val );
		static string	weekday_name( e_weekday val );
		static date		gmt( time t );
};

}