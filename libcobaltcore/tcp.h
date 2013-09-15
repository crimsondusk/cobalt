#ifndef LIBCOBALT_TCP_H
#define LIBCOBALT_TCP_H

#include "main.h"
#include "variant.h"

class CoTCPSocket {
	PROPERTY (bool, blocking, setBlocking)
	PROPERTY (bool, verbose, setVerbose)
	
public:
	CoTCPSocket (str node, uint port);
	bool init (uint16 port);
	long write (std::initializer_list<CoVariant> args);
	void privmsgf (const char* target, const char* fmt, ...);
	void tick();
	
	virtual ~CoTCPSocket() {}
	virtual void incoming (str token) = 0;
	
private:
	int sock;
	str token;
};

#endif // LIBCOBALT_TCP_H