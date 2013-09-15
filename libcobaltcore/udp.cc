#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include "udp.h"
#include "bytestream.h"
#include "format.h"

/* =============================================================================
 * -----------------------------------------------------------------------------
 * [[CONSTRUCTOR]] */
CoUDPSocket::CoUDPSocket() {
	setBlocking (true);
}

/* =============================================================================
 * -----------------------------------------------------------------------------
 * Initializes the socket with the given IP address */
bool CoUDPSocket::init (uint16 port) {
	m_port = port;
	error = "";
	sock = socket (AF_INET, SOCK_DGRAM, 0);
	
	if (!blocking()) {
		int flags = fcntl (sock, F_GETFL, 0);
		
		if (flags < 0) {
			error = "!! Invalid file control flags";
			return false;
		}
		
		if (fcntl (sock, F_SETFL, flags | O_NONBLOCK) != 0) {
			error = "!! Unable to set socket as non-blocking!";
			return false;
		}
	}
	
	return true;
}

/* =============================================================================
 * -----------------------------------------------------------------------------
 * Binds this socket to the port it was initialized with for hosting. */
bool CoUDPSocket::bind() {
	struct sockaddr_in svaddr;
	memset (&svaddr, 0, sizeof svaddr);
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons (m_port);
	svaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	
	if (::bind (sock, reinterpret_cast<struct sockaddr*> (&svaddr), sizeof svaddr) == -1) {
		error = fmt ("!! Couldn't bind to address %u!", m_port);
		return false;
	}
	
	return true;
}

// =============================================================================
// Ticks this socket. This must be called repeadetly for the socket to function.
// -----------------------------------------------------------------------------
void CoUDPSocket::tick() {
	struct sockaddr_in claddr;
	socklen_t socklen = sizeof claddr;
	
	char packet[5120];
	int n = recvfrom (sock, packet, sizeof packet, 0,
		reinterpret_cast<struct sockaddr*> (&claddr), &socklen);
	
	if (n == -1) {
		// We got an error, though EWOULDBLOCK is silent as it means no packets recieved.
		if (errno != EWOULDBLOCK)
			perror ("CoUDPSocket: recvfrom error:");
		
		return;
	}
	
	CoIPAddress addr (ntohl (claddr.sin_addr.s_addr), ntohs (claddr.sin_port));
	
	// Ignore this request if the sender is throttled
	if (m_throttle.isThrottled (addr)) {
		print ("-- Ignoring throttled packet from %1\n", addr);
		return;
	}
	
	CoBytestream in (packet, n);
	incoming (in, addr);
}

// =============================================================================
// Launches the given bytestream packet to the specified IP address.
// -----------------------------------------------------------------------------
void CoUDPSocket::launch (const CoBytestream& data, CoIPAddress addr) {
	struct sockaddr_in claddr = addr.toSockAddr();
	
	int res = sendto (sock, data.data(), data.length(), 0,
		reinterpret_cast<struct sockaddr*> (&claddr), sizeof claddr);
	
	if (res == -1)
		perror ("CoUDPSocket: Unable to launch packet");
}

// =============================================================================
// Adds the given IP address to the flood queue for the given amount of seconds.
// All messages from throttled addresses are ignored.
// -----------------------------------------------------------------------------
void CoUDPSocket::throttle (CoIPAddress addr, ulong secs) {
	m_throttle.addIP (addr, secs);
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoFloodThrottle::addIP (CoIPAddress addr, ulong seconds) {
	m_data[addr] = CoTime::now() + CoTime (seconds);
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoFloodThrottle::delIP (CoIPAddress addr) {
	m_data.erase (addr);
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoFloodThrottle::isThrottled (CoIPAddress addr) {
	CoTime now = CoTime::now();
	CoVector<CoIPAddress> expired;
	bool throttled = false;
	
	for (const pair & val : m_data) {
		if (val.second < now) {
			// expired
			expired << val.first;
			continue;
		}
	
		if (val.first == addr) {
			// found in throttle list
			throttled = true;
		}
	}
	
	for (const CoIPAddress & addr : expired)
		delIP (addr);
	
	return throttled;
}
