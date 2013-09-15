#ifndef LIBCOBALT_IP_H
#define LIBCOBALT_IP_H

#include "main.h"
#include "types.h"
#include "string.h"

struct sockaddr;
class CoIPAddress;
class CoIPRange;

// =============================================================================
// Abstract class to contain methods common to both IP addresses and IP ranges.
class CoAbstractIP {
	PROPERTY (uint32, address, setAddress)
	
public:
	uint8 octet (int n) const;
	void setOctet (int n, uint8 oct);
	uint8 operator[] (int n) const {
		return octet (n);
	}
};

// =============================================================================
// IP address, with port
class CoIPAddress : public CoAbstractIP {
	PROPERTY (uint16, port, setPort)
	
public:
	enum {
		WithPort = false,
		WithoutPort = true,
		
		NoError = 0
	};
	
	static const unsigned short AnyPort = 0;
	
	CoIPAddress();
	CoIPAddress (uint32 addr, uint16 port);
	CoIPAddress (const CoIPAddress& other);
	CoIPAddress (str ipstring);
	
	static bool fromString (str input, CoIPAddress& value);
	static int resolve (str node, CoIPAddress& val);
	str toString() const;
	bool matches (CoIPRange range) const;
	
	bool compare (const CoIPAddress& other) const;
	bool operator== (const CoIPAddress& other) const { return compare (other); }
	bool operator!= (const CoIPAddress& other) const { return !operator== (other); }
	bool operator< (const CoIPAddress& other) const;
	struct sockaddr_in toSockAddr() const;
};

// =============================================================================
// CoIPRange
//
// IP range, with wildcard and expiration support.
// Wildcards are stored in 4 bits: if bit N is set, octet N is a wildcard.
// =============================================================================
class CoIPRange : public CoAbstractIP {
	PROPERTY (uint8, wildcards, setWildcards)
	PROPERTY (time_t, expireTime, setExpireTime)
	PROPERTY (str, comment, setComment)

public:
	CoIPRange() {}
	CoIPRange (uint32 addr, uint8 wildcards);
	CoIPRange (str input);
	
	bool fromString (str input);
	str fullString() const;
	void setWildcarded (short i);
	str toString() const;
	bool wildcarded (short i) const;
};

extern CoIPAddress localhost;

#endif // LIBCOBALT_IP_H