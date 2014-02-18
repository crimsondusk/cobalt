#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "main.h"
#include "types/variant.h"
#include "types/tuple.h"
#include "types/ip.h"
#include <typeinfo>
//#include "irc/irc.h"
//#include "irc/connection.h"

//IRCConnection* g_IRCConnection = cbl::null;

CBL_CONFIG( string,	irc_server,	"" )
CBL_CONFIG( int,	irc_port,	6667 )

static const cbl::string g_config_file = APPNAME ".xml";

void crash_handler( int );
void on_terminate();

// -----------------------------------------------------------------------------
//
int main( int argc, char* argv[] )
{
	try
	{
		cbl::tuple<int, float, cbl::string, cbl::ip_address> argh( 5, 3.14f, "argh", cbl::localhost );
		int blargh = 1;
		cbl::print( "tuple: %1\ntest: %2 blargh%s2\n", argh, blargh );
		// print( "%1 version %2.%3 starting\n", APPNAME, VERSION_MAJOR, VERSION_MINOR );

		/*
		if( cbl::load_config( g_config_file ) == false )
		{
			print_to( stderr, "error: Couldn't open %1: %2\n", g_config_file, xml_document::parseError() );

			if( errno )
			{
				print_to( stderr, "Creating default config...\n" );
				cbl::save_config( g_config_file );
			}

			return 1;
		}

		atexit( on_terminate );

		struct sigaction sighandler;
		sighandler.sa_handler = &crash_handler;
		sighandler.sa_flags = 0;
		sigemptyset( &sighandler.sa_mask );
		sigaction( SIGINT, &sighandler, cbl::null );
		sigaction( SIGSEGV, &sighandler, cbl::null );

		// Create the IRC connection.
		g_IRCConnection = new IRCConnection( cfg::irc_server, cfg::irc_port );

		for( ;; )
		{
			g_IRCConnection->Tick();
			tickServerRequests();
		}
		*/
	}
	catch( std::exception& e )
	{
		fprintf( stderr, "error: %s\n", e.what() );
		return 1;
	}

	return 0;
}

void on_terminate()
{
	cbl::print( APPNAME " exiting\n" );
	// delete g_IRCConnection;
}

void crash_handler( int signum )
{
	/*
	switch( signum )
	{
		case SIGINT:
		{
			if( g_IRCConnection )
				g_IRCConnection->write( "QUIT :Interrupted\n" );

			exit( EXIT_FAILURE );
		} break;

		case SIGSEGV:
		{
			if( g_IRCConnection )
				g_IRCConnection->write( "QUIT :Segmentation fault\n" );

			exit( EXIT_FAILURE );
		} break;
	}
	*/
}

/*
void fatal_error( const char* file, ulong line, const char* func, cbl::list<cbl::format_arg> const& s )
{
	cbl::string body = cbl::format_args( s );
	cbl::string msg = format( "fatal() called: %1:%2: %3: ", file, line, func );
	msg += body;
	msg += ", aborting";

	print_to( stderr, "%1\n", msg );

	// Quit with the error message if we're connected
	//if( g_IRCConnection && g_IRCConnection->loggedIn() )
	//	g_IRCConnection->write( format( "QUIT :%1\n", msg ) );

	abort();
}

void warn_args( const char* file, ulong line, const char* func, cbl::list<cbl::format_arg> const& s )
{
	cbl::string body = cbl::format_args( s );
	cbl::string msg = format( "warning: %1:%2: %3: ", file, line, func );
	msg += body;

	print_to( stderr, msg + "\n" );

#if 0
	if( g_IRCConnection && g_IRCConnection->loggedIn() )
		g_IRCConnection->privmsg( irc_channel, msg );
#endif
}
*/

bool save_configuration()
{
	return cbl::save_config( g_config_file );
}

const cbl::string& get_config_file_name()
{
	return g_config_file;
}
