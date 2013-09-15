#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netdb.h>
#include "tcp.h"
#include "variant.h"
#include "format.h"

// =============================================================================
// -----------------------------------------------------------------------------
CoTCPSocket::CoTCPSocket (str node, uint port) {
	print ("Attempting connection to %1:%2\n", node, port);
	setVerbose (false);
	
	struct addrinfo hints;
	struct addrinfo* result;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	
	str portstring = fmt ("%1", port);
	
	int r = getaddrinfo (node, portstring, &hints, &result);
	if (r) {
		fprint (stderr, "Name lookup error: %1\n", gai_strerror (r));
		abort();
	}
	
	struct addrinfo* i;
	for (i = result; i; i = i->ai_next) {
		sock = socket (i->ai_family, i->ai_socktype, i->ai_protocol);
		if (sock == -1)
			continue;
		
		if (connect (sock, i->ai_addr, i->ai_addrlen) != -1) {
			printf ("Connection successful.\n");
			break;
		}
		
		close (sock);
	}
	
	if (!i) {
		fprint (stderr, "Connection failed (%1).\n", strerror (errno));
		abort();
	}
	
	int flags = fcntl (sock, F_GETFL, 0);
	if (flags < 0) {
		fprint (stderr, "AbstractTCPSocket: got invalid file control flags while setting socket as non-blocking\n");
		abort();
	}
	
	if (fcntl (sock, F_SETFL, flags | O_NONBLOCK) != 0) {
		fprint (stderr, "unable to set socket as non-blocking\n");
		abort();
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoTCPSocket::tick() {
	char buf[5120];
	
	{
		// Wait 0.05 seconds for the data
		struct timeval wait;
		fd_set set;
		
		FD_ZERO (&set);
		FD_SET (sock, &set);
		
		wait.tv_sec = 0;
		wait.tv_usec = 50000;
		if (select (sock, &set, null, null, &wait) == -1)
			return;
	}
	
	long readsize = read (sock, buf, sizeof buf);
	if (readsize == -1) {
		if (errno != EWOULDBLOCK)
			perror ("read failure");
		
		return;
	} else if (readsize > 0) {
		// Tokenize what we got into lines
		for (char* i = buf; i < buf + readsize; i++) {
			if (*i == '\n') {
				if (verbose())
					print ("-> %1\n", token);
				
				incoming (token);
				token = "";
			} else {
				token += *i;
			}
		}
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
long CoTCPSocket::write (CoString msg) {
	msg += '\n';
	
	if (verbose())
		print ("<- %1", msg);
	
	return ::write (sock, msg.chars(), msg.length());
}