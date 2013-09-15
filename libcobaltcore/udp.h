#ifndef LIBCOBALT_UDP_H
#define LIBCOBALT_UDP_H

#include <map>
#include "main.h"
#include "ip.h"
#include "time.h"

class CoBytestream;
class CoUDPSocket;
class CoFloodThrottle;

typedef void (*udpcallback) (CoUDPSocket*, CoIPAddress, CoBytestream);

class CoFloodThrottle {
public:
	typedef std::pair<CoIPAddress, CoTime> pair;
	
	void addIP (CoIPAddress addr, ulong seconds);
	void delIP (CoIPAddress addr);
	bool isThrottled (CoIPAddress addr);
	
private:
	std::map<CoIPAddress, CoTime> m_data;
};

class CoUDPSocket {
public:
	PROPERTY (bool, blocking, setBlocking)
	
public:
	CoIPAddress m_addr;
	uint16 m_port;
	str error;
	int sock;
	
	CoUDPSocket();
	bool init (uint16 port);
	bool bind();
	void tick();
	void launch (const CoBytestream& data, CoIPAddress addr);
	void throttle (CoIPAddress addr, ulong secs);
	
	virtual ~CoUDPSocket() {}
	virtual void incoming (const CoBytestream& data, CoIPAddress addr) = 0;

private:
	CoFloodThrottle m_throttle;
};

#endif // LIBCOBALT_UDP_H
