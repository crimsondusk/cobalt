#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "common.h"
#include "irc/irc.h"
#include "irc/irc_connection.h"
#include "config.h"
#include "mantisbt.h"

IRCConnection* g_IRCConnection = null;

void sig( int );
void term();

int main()
{
	print( "%1 version %2.%3 starting\n", APPNAME, VERSION_MAJOR, VERSION_MINOR );
	loadConfig();
	
	atexit( term );
	
	struct sigaction sighandler;
	sighandler.sa_handler = &sig;
	sighandler.sa_flags = 0;
	sigemptyset( &sighandler.sa_mask );
	sigaction( SIGINT, &sighandler, null );
	sigaction( SIGSEGV, &sighandler, null );
	
	// Create the IRC connection.
	g_IRCConnection = new IRCConnection( cfg( Name::IRCServer ), atoi( cfg( Name::IRCPort ) ) );
	g_IRCConnection->setVerbose( true );
	
	for( ;; )
		g_IRCConnection->tick();
	
	return 0;
}

void term()
{
	print( APPNAME " exiting\n" );
	delete g_IRCConnection;
}

void sig( int signum )
{
	switch( signum )
	{
	case SIGINT:
		if( g_IRCConnection )
			g_IRCConnection->write({ "QUIT :Interrupted\n" });
		
		exit( EXIT_FAILURE );
	
	case SIGSEGV:
		if( g_IRCConnection )
			g_IRCConnection->write({ "QUIT :Segmentation fault\n" });
		
		exit( EXIT_FAILURE );
	}
}

void FatalError( const char* file, ulong line, const char* func, vector<var> s )
{
	str body = DoFormat( s );
	
	str msg = fmt( "fatal() called: %1:%2: %3: ", file, line, func );
	msg += body;
	msg += ", aborting";
	
	fprint( stderr, "%1\n", msg );
	
	// Quit with the error message if we're connected
	if( g_IRCConnection && g_IRCConnection->loggedIn() )
		g_IRCConnection->write( {"QUIT :%1\n", msg} );
	
	abort();
}

void DoWarn( const char* file, ulong line, const char* func, vector<var> s )
{
	str body = DoFormat( s );

	str msg = fmt( "warning: %1:%2: %3: ", file, line, func );
	msg += body;

	fprintf( stderr, "%s\n", msg.chars() );

	if( g_IRCConnection && g_IRCConnection->loggedIn() )
		g_IRCConnection->privmsg( cfg( Name::Channel ), msg );
}