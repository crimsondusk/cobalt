#include "../common.h"
#include "../mantisbt.h"
#include "irc.h"
#include "irc_channel.h"
#include "irc_user.h"
#include "irc_connection.h"
#include <util/time.h>

vector<IRCCommandInfo> g_IRCCommands;

IRCCommandAdder::IRCCommandAdder( const char* namestring, IRCCommandType func )
{
	const IRCCommandInfo info = {namestring, func};
	g_IRCCommands << info;
}

str IRCTarget( IRCUser* sender, IRCChannel* chan )
{
	if( chan )
		return chan->name();

	return sender->nick();
}

#define TARGET IRCTarget (invoker, channel)

// =============================================================================
IRC_COMMAND( sayhi )
{
	if( parms.size() < 2 )
		conn->privmsg( TARGET, "Hi!" );
	else
		conn->privmsg( TARGET, fmt( "Hi, %2!", parms[1] ));
}

// =============================================================================
IRC_COMMAND( quit )
{
	if( !invoker->isAdmin() )
	{
		conn->privmsg( TARGET, fmt( "%1: Who are you to tell me to GTFO?", invoker->nick() ));
		return;
	}
	
	conn->privmsg( TARGET, "Bye." );
	conn->write({ "QUIT :Leaving" });
}

// =============================================================================
IRC_COMMAND( userlist )
{
	if( invoker->isAdmin() == false || parms.size() < 2 )
		return;
	
	IRCChannel* chan = conn->findChannel( parms[1] );
	
	if( chan == null )
	{
		conn->privmsg( TARGET, "What channel is that?" );
		return;
	}
	
	for( IRCChannel::Entry& e : *chan )
	{
		IRCUser* info = e.userinfo();
		
		conn->privmsg( invoker->nick(), fmt( "%1: %2%3%4%5%6",
			*info,
			chan->statusname( e.status() ),
			( *info & IRCUser::IRCOp ) ? ", IRC Operator" : "",
			( *info & IRCUser::Away ) ? ", AFK" : "",
			( *info & IRCUser::Admin ) ? ", " APPNAME " admin" : "",
			( info == conn->me() ) ? " <<---- me!" : ""
			));
	}
	
	conn->privmsg( invoker->nick(), "====================================================" );
	conn->privmsg( invoker->nick(), fmt( "%1 users listed", chan->numUsers() ));
}

// =============================================================================
IRC_COMMAND( raw )
{
	if( !invoker->isAdmin() )
	{
		conn->write( {"PRIVMSG %1 :%2: Who are you to tell me what to do?", TARGET, invoker->nick()} );
		return;
	}
	
	int space = message.first( " " );
	str raw = message.substr( space + 1, -1 );
	conn->write( {"%1\n", raw} );
}

// =============================================================================
IRC_COMMAND( trymask )
{
	if( parms.size() < 2 )
	{
		conn->write( {"PRIVMSG %1 :I need a mask to check against!", TARGET} );
		return;
	}
	
	for( IRCUser* user : conn->users )
		if( mask( user->userhost(), parms[1] ))
			conn->privmsg( TARGET, fmt( "%1 matches the mask", *user ));
}

IRC_COMMAND( ban )
{
	if( channel == null || invoker->chanStatus( channel ) < IRCChannel::Op )
		return;
	
	if( parms.size() < 2 )
	{
		conn->write({ "PRIVMSG %1 :need a mask", TARGET });
		return;
	}
	
	conn->write({ "MODE %1 +b %2\n", TARGET, parms[1] });
	
	for( uint i = 0; i < conn->users.size(); i++ )
	{
		IRCUser* user = conn->users[i];
		
		if( mask( user->userhost(), parms[1] ))
			conn->write({ "KICK %1 %2 :Banned", TARGET, user->nick() });
	}
}

IRC_COMMAND( testfatal )
{
	if( invoker->isAdmin() == false )
		return;
	
	parms.erase( 0 );
	fatal( "%1", join( parms, " " ) );
}

/*
IRC_COMMAND( crash )
{
	if( !invoker->isAdmin() )
		return;
	
	*(( int* ) null ) = 5;
}
*/

IRC_COMMAND( time )
{
	conn->privmsg( TARGET, fmt( "Now is %1", Date( Time::now() )));
}

Stopwatch g_stopwatch;
IRC_COMMAND( sw_start )
{
	g_stopwatch.start( );
	conn->privmsg( TARGET, "Stopwatch started." );
}

IRC_COMMAND( sw_stop )
{
	g_stopwatch.stop( );
	conn->privmsg( TARGET, fmt( "Stopwatch stopped, lapsed time: %1", g_stopwatch.elapsed() ));
}

IRC_COMMAND( whois )
{
	if( parms.size() < 2 )
	{
		conn->privmsg( TARGET, "Who is... who?" );
		return;
	}
	
	IRCUser* user = conn->findUser( parms[1] );
	
	if( user == null )
	{
		conn->privmsg( TARGET, "I don't know who that is." );
		return;
	}
	
	vector<str> channames;
	for( IRCChannel* chan : user->channels())
		channames << chan->name();
	
	conn->privmsg( TARGET, fmt( "%1 is %2, real name: %3", user->nick(), user->userhost(), user->name() ));
	conn->privmsg( TARGET, fmt( "I see %1 on %2", user->nick(), join( channames, ", " )));
	conn->privmsg( TARGET, fmt( "NickServ account for %1 is %2\n", user->nick(), user->account() ));
}

IRC_COMMAND( ticket )
{
	if( parms.size() < 2 )
	{
		conn->privmsg( TARGET, "Which ticket?" );
		return;
	}
	
	if( atoi( parms[1] ) <= 0 )
		return;
	
	str idstr = variant( atoi( parms[1] )).stringRep();
	str msg;
	bool success = ticketinfo( idstr, msg );
	
	if( success )
	{
		conn->privmsg( TARGET, msg );
		conn->privmsg( TARGET, fmt( "Link: %1view.php?id=%2", cfg( Name::TrackerURL ), idstr ));
	}
	else
	{
		conn->privmsg( TARGET, msg );
	}
}

IRC_COMMAND( fullticketinfo )
{
	if( parms.size() < 2 )
	{
		conn->privmsg( TARGET, "Which ticket?" );
		return;
	}
	
	if( atoi( parms[1] ) <= 0 )
		return;
	
	str idstr = variant( atoi( parms[1] )).stringRep();
	str msg = fullticketinfo( idstr );
	
	for( const str& line : msg / "\n" )
		conn->privmsg( TARGET, line );
}

IRC_COMMAND( chanmode )
{
	if( !channel )
		return;
	
	conn->privmsg( TARGET, channel->modeString() );
}

IRC_COMMAND( list_commands )
{
	vector<str> cmdnames;
	for( const IRCCommandInfo& info : g_IRCCommands )
		cmdnames << info.namestring;
	
	cmdnames.sort();
	conn->privmsg( TARGET, fmt( "Available commands: %1", join( cmdnames, " " )));
}

IRC_COMMAND( msg )
{
	if( !invoker->isAdmin() || parms.size() < 3 )
		return;
	
	vector<str> msgparms = parms;
	msgparms.erase( 0 );
	msgparms.erase( 0 );
	str msg = join( msgparms, " " );
	conn->privmsg( parms[1], msg );
}