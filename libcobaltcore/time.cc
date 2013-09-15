#include <sys/time.h>
#include <time.h>
#include "main.h"
#include "time.h"
#include "format.h"
#include "variant.h"
#include "misc.h"

static const char* g_monthNames[] = {
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

static const char* g_weekDayNames[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

CoTime::CoTime (time_t val) {
	timespec a;
	a.tv_nsec = 0;
	a.tv_sec = val;
	setSpec (a);
}

CoTime CoTime::now() {
	timespec t;
	clock_gettime (CLOCK_REALTIME, &t);
	return CoTime (t);
}

CoTime CoTime::diff (const CoTime& other) const {
	const timespec& a = spec(),
		b = other.spec();
	timespec c;
	
	c.tv_nsec = a.tv_nsec - b.tv_nsec;
	c.tv_sec = a.tv_sec - b.tv_sec;
	
	if (c.tv_nsec < 0) {
		c.tv_nsec += billion;
		c.tv_sec--;
	}
	
	return CoTime (c);
}

time_t CoTime::seconds() const {
	return spec().tv_sec;
}

CoTime CoTime::operator+ (const CoTime& other) const {
	const timespec& a = spec(),
	                b = other.spec();
	timespec c;
	
	c.tv_nsec = a.tv_nsec + b.tv_nsec;
	c.tv_sec = a.tv_sec + b.tv_sec;
	
	if (c.tv_nsec >= (long long) billion) {
		c.tv_nsec -= billion;
		c.tv_sec++;
	}
	
	return CoTime (c);
}

CoTime CoTime::operator- (const CoTime& other) const {
	return diff (other);
}

static void stringRepPart (CoString& rep, time_t& n, time_t block, bool twodigit) {
	int numblocks = n / block;
	
	if (numblocks > 0 || twodigit) {
		if (rep.length())
			rep += ":";
		
		if (twodigit) {
			char digits[16];
			sprintf (digits, "%.2d", numblocks);
			rep += digits;
		} else
			rep += fmt ("%1", numblocks);
		
		n -= numblocks * block;
	}
}

CoString CoTime::stringRep() const {
	time_t n = seconds();
	CoString rep;
	
	stringRepPart (rep, n, day, false);
	stringRepPart (rep, n, hour, false);
	stringRepPart (rep, n, minute, true);
	stringRepPart (rep, n, 1, true);
	
	const int microsecs = nanoseconds() / 1000;
	
	if (microsecs > 0) {
		char x[16];
		sprintf (x, ".%.4d", microsecs);
		rep += x;
	}
	
	return rep;
}

static void fullStringRepPart (CoString& rep, time_t& secs, time_t block, const char* unit) {
	if (secs < block)
		return;
	
	int numblocks = secs / block;
	
	if (rep.length() > 0)
		rep += ", ";
	
	rep += fmt ("%1 %2%3", numblocks, unit, numblocks == 1 ? "" : "s");
	secs -= numblocks * block;
}

str CoTime::fullStringRep() const {
	str rep;
	time_t secs = seconds();

	fullStringRepPart (rep, secs, year, "year");
	fullStringRepPart (rep, secs, day, "day");
	fullStringRepPart (rep, secs, hour, "hour");
	fullStringRepPart (rep, secs, minute, "minute");
	fullStringRepPart (rep, secs, 1, "second");
	return rep;
}

long CoTime::nanoseconds() const {
	return spec().tv_nsec;
}

void CoTime::setNanoseconds (long val) {
	timespec v = spec();
	v.tv_nsec = val;
	setSpec (v);
}

int CoTime::compare (const CoTime& other) const {
	if (seconds() != other.seconds())
		return seconds() > other.seconds() ? 1 : -1;

	if (nanoseconds() != other.nanoseconds())
		return nanoseconds() > other.nanoseconds() ? 1 : -1;

	return 0;
}

bool CoTime::operator> (const CoTime& other) const {
	return compare (other) > 0;
}

bool CoTime::operator< (const CoTime& other) const {
	return compare (other) < 0;
}

bool CoTime::operator== (const CoTime& other) const {
	return compare (other) == 0;
}

bool CoTime::operator!= (const CoTime& other) const {
	return compare (other) != 0;
}

bool CoTime::operator>= (const CoTime& other) const {
	return compare (other) >= 0;
}

bool CoTime::operator<= (const CoTime& other) const {
	return compare (other) <= 0;
}

// =============================================================================
CoDate::CoDate() {
	time_t secs = CoTime().seconds();
	setInfo (localtime (&secs));
}

CoDate::CoDate (const CoTime& t) {
	time_t secs = t.seconds();
	setInfo (localtime (&secs));
}

int CoDate::year() const {
	return info()->tm_year + 1900;
}

CoDate::Month CoDate::month() const {
	int monum = info()->tm_mon;
	assert (monum >= 0 && monum <= 11);

	return (Month) (monum);
}

CoDate::Weekday CoDate::weekday() const {
	int daynum = info()->tm_wday;
	assert (daynum >= 0 && daynum <= 6);
	return (Weekday) (daynum);
}

int CoDate::day() const {
	return info()->tm_yday;
}

int CoDate::hour() const {
	return info()->tm_hour;
}

int CoDate::minute() const {
	return info()->tm_min;
}

int CoDate::second() const {
	return info()->tm_sec;
}

int CoDate::timezone() const {
	return info()->tm_gmtoff;
}

str CoDate::timezoneName() const {
	return str (info()->tm_zone);
}

bool CoDate::dst() const {
	return info()->tm_isdst;
}

void CoDate::setYear (int val) {
	m_info->tm_year = val - 1900;
}

void CoDate::setMonth (CoDate::Month val) {
	m_info->tm_mon = (int) val;
}

void CoDate::setDay (int val) {
	m_info->tm_mday = val;
}

void CoDate::setWeekday (CoDate::Weekday val) {
	m_info->tm_wday = (int) val;
}

void CoDate::setHour (int val) {
	m_info->tm_hour = val;
}

void CoDate::setMinute (int val) {
	m_info->tm_min = val;
}

void CoDate::setSecond (int val) {
	m_info->tm_sec = val;
}

void CoDate::setTimezone (int val) {
	m_info->tm_gmtoff = val;
}

void CoDate::setDST (bool val) {
	m_info->tm_isdst = val;
}

str CoDate::stringRep() const {
	str rep = asctime (info());
	
	// asctime is stupid and adds a new-line character
	// after the message. Remove it.
	if (rep[rep.length() - 1] == '\n')
		rep -= 1;
	
	return rep;
}

str CoDate::monthName (CoDate::Month val) {
	return g_monthNames[val];
}

str CoDate::monthName() const {
	return g_monthNames[month()];
}

str CoDate::weekdayName (CoDate::Weekday val) {
	return g_weekDayNames[val];
}

str CoDate::weekdayName() const {
	return g_weekDayNames[weekday()];
}

CoDate CoDate::GMT (CoTime t) {
	time_t now = t.seconds();
	struct tm* gmtnow = gmtime (&now);
	return CoDate (gmtnow);
}

// =============================================================================
CoStopwatch::CoStopwatch() {
	m_running = false;
}

void CoStopwatch::start() {
	clock_gettime (CLOCK_REALTIME, &m_start);
	m_running = true;
}

void CoStopwatch::stop() {
	if (m_running)
		clock_gettime (CLOCK_REALTIME, &m_now);
	
	m_running = false;
}

CoTime CoStopwatch::elapsed() const {
	if (m_running)
		clock_gettime (CLOCK_REALTIME, &m_now);
	
	return CoTime (m_now) - CoTime (m_start);
}