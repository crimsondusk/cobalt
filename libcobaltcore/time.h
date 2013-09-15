#ifndef LIBCOBALTCORE_TIME_H
#define LIBCOBALTCORE_TIME_H

#include <ctime>
#include "main.h"
#include "string.h"

static const long
	minute = 60l * 1l,
	hour   = 60l * minute,
	day    = 24l * hour,
	week   = 7l * day,
	month  = 30l * day,
	year   = 365l * day;

class CoTime {
	READ_PROPERTY (timespec, spec, setSpec)

public:
	CoTime (time_t val = 0);
	CoTime (timespec val) : m_spec (val) {}
	
	CoTime   diff (const CoTime& other) const;
	CoString stringRep() const;
	CoString fullStringRep() const;
	time_t   seconds() const;
	long     nanoseconds() const;
	void     setNanoseconds (long val);
	int      compare (const CoTime& other) const;
	
	static CoTime now();
	
	CoTime operator+ (const CoTime& other) const;
	CoTime operator- (const CoTime& other) const;
	bool operator> (const CoTime& other) const;
	bool operator< (const CoTime& other) const;
	bool operator>= (const CoTime& other) const;
	bool operator<= (const CoTime& other) const;
	bool operator== (const CoTime& other) const;
	bool operator!= (const CoTime& other) const;
};

class CoDate {
	READ_PROPERTY (struct tm*, info, setInfo)

public:
	CoDate();
	CoDate (const CoTime& t);
	CoDate (struct tm* tminfo) : m_info (tminfo) {}
	
	enum Month {
		January,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};
	
	enum Weekday {
		Sunday,
		Monday,
		Tuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday
	};
	
	int year() const;
	Month month() const;
	int day() const;
	Weekday weekday() const;
	int hour() const;
	int minute() const;
	int second() const;
	CoString timezoneName() const;
	int timezone() const;
	bool dst() const;
	
	void setYear (int val);
	void setMonth (Month val);
	void setDay (int val);
	void setWeekday (Weekday val);
	void setHour (int val);
	void setMinute (int val);
	void setSecond (int val);
	void setTimezone (int val);
	void setDST (bool val);
	
	static CoString monthName (Month val);
	static CoString weekdayName (Weekday val);
	static CoDate GMT (CoTime t);
	CoString monthName() const;
	CoString weekdayName() const;
	
	CoString stringRep() const;
};

class CoStopwatch {
public:
	CoStopwatch();
	void start();
	void stop();
	CoTime elapsed() const;
	
private:
	bool m_running;
	timespec m_start;
	mutable timespec m_now;
};

#endif // LIBCOBALTCORE_TIME_H