#include "common.h"
#include "irc.h"
#include "config.h"
#include "wad.h"

IRCConnection* conn = NULL;

void term ();

int main () {
	printf ("%s version %d.%d\n", APPNAME, VERSION_MINOR, VERSION_MAJOR);
	
	// Load the configuration
	loadConfig ();
	
	// On exit, call term().
	atexit (term);
	
	// Create the IRC connection.
	conn = new IRCConnection (getConfig (IRCServer), atoi (getConfig (IRCPort)));
}

void term () {
	printf ("%s exiting\n", APPNAME);
}

// Name-to-string array.
const char* g_NameStrings[] = {
#define _N(X) "NAME_" # X
	_N(None),
#include "namelist.h"
#undef _N
};