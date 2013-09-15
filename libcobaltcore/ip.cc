#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include "ip.h"
#include "format.h"

CoIPAddress localhost (0x7F000001, 0);

// =============================================================================
CoIPAddress::CoIPAddress() {
	memset (this, 0, sizeof * this);
}

// =============================================================================
// -----------------------------------------------------------------------------
CoIPAddress::CoIPAddress (uint32 addr, uint16 port) {
	setAddress (addr);
	setPort (port);
}

// =============================================================================
// -----------------------------------------------------------------------------
CoIPAddress::CoIPAddress (const CoIPAddress& other) {
	memcpy (this, &other, sizeof other);
}

// =============================================================================
// -----------------------------------------------------------------------------
CoIPAddress::CoIPAddress (str ipstring) {
	CoIPAddress::fromString (ipstring, *this);
}

// =============================================================================
str CoIPAddress::toString() const {
	str val;
	
	for (int i = 0; i < 4; ++i) {
		if (i > 0)
			val += '.';
		
		val += CoVariant ((uint) operator[] (i)).stringRep();
	}
	
	if (port() != 0)
		val += fmt (":%1", port());
	
	return val;
}

// =============================================================================
// -----------------------------------------------------------------------------
int CoIPAddress::resolve (str node, CoIPAddress& val) {
	struct addrinfo   hints;
	struct addrinfo*  result;
	int               r;
	
	memset (&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	
	if ((r = getaddrinfo (node, null, &hints, &result)) != 0)
		return r;
	
	for (struct addrinfo* it = &result[0]; it; it = it->ai_next) {
		struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*> (it->ai_addr);
		val.setAddress (ntohl (addr->sin_addr.s_addr));
		val.setPort (ntohs (addr->sin_port));
		break;
	}
	
	freeaddrinfo (result);
	return NoError;
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoIPAddress::fromString (str input, CoIPAddress& value) {
	uint parts[4];
	const int colonpos = input.first (":");
	const str addressString = colonpos == -1 ? input : input.substr (0, colonpos);
	
	value.setAddress (0);
	value.setPort (0);
	
	// Try scanf the IPv4 address first. If it's not an IP string, it could
	// be a hostname; thus try resolve it.
	if (sscanf (addressString, "%u.%u.%u.%u", &parts[0], &parts[1], &parts[2], &parts[3])) {
		for (short i = 0; i < 4; ++i)
			value.setOctet (i, parts[i]);
	} elif (CoIPAddress::resolve (addressString, value) != NoError)
		return false; // resolution failed
	
	// Success! Deal with the possible port now.
	if (colonpos != -1)
		value.setPort (atoi (input.substr (colonpos + 1, -1)));
	
	return true;
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoIPAddress::matches (CoIPRange range) const {
	if (address() == range.address())
		return true; // full match
	
	for (short i = 0; i < 4; ++i) {
		if (range.wildcarded (i))
			continue; // wildcarded
		
		if (octet (i) != range[i])
			return false; // matching failed
	}
	
	return true;
}

// =============================================================================
// -----------------------------------------------------------------------------
uint8 CoAbstractIP::octet (int n) const {
	return (address() >> ((3 - n) * 8)) & 0xFF;
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoAbstractIP::setOctet (int n, uint8 oct) {
	// TODO: make a big-endian version
	setAddress ((address() & ~ (0xFF << (3 - n) * 8)) | (oct << (3 - n) * 8)); // :)
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoIPAddress::compare (const CoIPAddress& other) const {
	for (short i = 0; i < 4; ++i)
		if (octet (i) != other[i])
			return false;
	
	if (port() != AnyPort && other.port() != AnyPort && port() != other.port())
		return false;
	
	return true;
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoIPAddress::operator< (const CoIPAddress& other) const {
	for (int i = 0; i < 4; ++i) {
		if (octet (i) < other[i])
			return true;
		
		if (octet (i) > other[i])
			return false;
	}
	
	return port() < other.port();
}

// =============================================================================
// -----------------------------------------------------------------------------
CoIPRange::CoIPRange (uint32 addr, uint8 wildcards) {
	setAddress (addr);
	setWildcards (wildcards);
}

// =============================================================================
// -----------------------------------------------------------------------------
CoIPRange::CoIPRange (str input) {
	fromString (input);
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoIPRange::fromString (str input) {
	if (input.count ('.') != 3)
		return false;
	
	setWildcards (0);
	setExpireTime (0);
	
	str token;
	const char* c = &input[0];
	CoIPAddress part;
	
	while (*c == '*' || *c == '.' || (*c >= '0' && *c <= '9'))
		token += *c++;
	
	if (!CoIPAddress::fromString (token, part))
		return false;
	
	setAddress (part.address());
	
	// Check to see if there was any wildcards
	{
		assert (token.count ('.') == 3);
		ulong i = 0;
		
		for (str& part : token.split (".")) {
			if (part == "*")
				setWildcarded (i);
			
			++i;
		}
	}
	
	if (*c == '<') {
		c++;
		token = "";
		
		while (*c && *c != '>')
			token += *c++;
		
		c++;
		uint month, day, year, hour, minute;
		
		if (sscanf (token, "%u/%u/%u %u:%u", &month, &day, &year, &hour, &minute) ||
				sscanf (token, "%u/%u/%u", &month, &day, &year)) {
			time_t rawtime = time (null);
			
			struct tm* info = localtime (&rawtime);
			info->tm_year = year - 1900;
			info->tm_mon = month - 1;
			info->tm_mday = day;
			info->tm_sec = 0;
			
			if (token.first (" ") != -1) {
				info->tm_hour = hour;
				info->tm_min = minute;
			} else
				info->tm_hour = info->tm_min = 0;
			
			setExpireTime (mktime (info));
		}
	}
	
	if (*c == ':')
		setComment (++c);
	
	return true;
}

// =============================================================================
// -----------------------------------------------------------------------------
str CoIPRange::toString() const {
	str val;
	
	for (int i = 0; i < 4; ++i) {
		if (i > 0)
			val += '.';
		
		if (wildcarded (i))
			val += '*';
		else
			val += CoVariant ((uint) octet (i)).stringRep();
	}
	
	return val;
}

// =============================================================================
// -----------------------------------------------------------------------------
str CoIPRange::fullString() const {
	str val = toString();
	
	if (expireTime() != 0) {
		time_t expr = expireTime();
		struct tm* info = localtime (&expr);
		
		val += fmt ("<%.2d/%.2d/%.4d %.2d:%.2d>", info->tm_mon + 1, info->tm_mday,
			info->tm_year + 1900, info->tm_hour, info->tm_min);
	}

	if (comment().length() > 0) {
		val += ":";
		val += comment();
	}

	return val;
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoIPRange::wildcarded (short i) const {
	return wildcards() & (1 << i);
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoIPRange::setWildcarded (short i) {
	setWildcards (wildcards() | (1 << i));
}

// =============================================================================
// -----------------------------------------------------------------------------
struct sockaddr_in CoIPAddress::toSockAddr() const {
	struct sockaddr_in claddr;
	memset (&claddr, 0, sizeof claddr);
	claddr.sin_addr.s_addr = htonl (address());
	claddr.sin_port = htons (port());
	claddr.sin_family = AF_INET;
	return claddr;
}