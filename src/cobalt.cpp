#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "main.h"
#include "irc/irc.h"
#include "irc/irc_connection.h"
#include "mantisbt.h"
#include "zanserver.h"
#include "3rdparty/huffman.h"

IRCConnection* g_IRCConnection = null;

CONFIG (String, irc_server, "irc.zandronum.com")
CONFIG (Int,    irc_port,   6667)

static CoString G_ConfigFile = "cobalt.xml";

void sig (int);
void term();

int main (int argc, char* argv[]) {
	print ("%1 version %2.%3 starting\n", APPNAME, VERSION_MAJOR, VERSION_MINOR);
	if (!CoConfig::load (G_ConfigFile)) {
		fprint (stderr, "error: Couldn't open %1: %2\n", G_ConfigFile, strerror (errno));
		return 1;
	}
	
	atexit (term);
	HUFFMAN_Construct();
	
	int i = 5;
	float f;
	
	struct sigaction sighandler;
	sighandler.sa_handler = &sig;
	sighandler.sa_flags = 0;
	sigemptyset (&sighandler.sa_mask);
	sigaction (SIGINT, &sighandler, null);
	sigaction (SIGSEGV, &sighandler, null);
	
	// Create the IRC connection.
	g_IRCConnection = new IRCConnection (irc_server, irc_port);
	
	forever {
		g_IRCConnection->tick();
		tickServerRequests();
	}
	
	return 0;
}

void term() {
	print (APPNAME " exiting\n");
	delete g_IRCConnection;
}

void sig (int signum) {
	switch (signum) {
	case SIGINT:
		if (g_IRCConnection)
			g_IRCConnection->write ("QUIT :Interrupted\n");
		
		exit (EXIT_FAILURE);
		
	case SIGSEGV:
		if (g_IRCConnection)
			g_IRCConnection->write ("QUIT :Segmentation fault\n");
		
		exit (EXIT_FAILURE);
	}
}

void FatalError (const char* file, ulong line, const char* func, initlist<var> s) {
	str body = DoFormat (s);
	str msg = fmt ("fatal() called: %1:%2: %3: ", file, line, func);
	msg += body;
	msg += ", aborting";
	
	fprint (stderr, "%1\n", msg);
	
	// Quit with the error message if we're connected
	if (g_IRCConnection && g_IRCConnection->loggedIn())
		g_IRCConnection->write ({ "QUIT :%1\n", msg });
	
	abort();
}

void DoWarn (const char* file, ulong line, const char* func, initlist<var> s) {
	str body = DoFormat (s);
	str msg = fmt ("warning: %1:%2: %3: ", file, line, func);
	msg += body;
	
	fprintf (stderr, "%s\n", msg.toStdString().c_str());
	
	if (g_IRCConnection && g_IRCConnection->loggedIn())
		g_IRCConnection->privmsg (cfg (Name::Channel), msg);
}