#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <util/tcp.h>
#include <util/format.h>
#include <util/time.h>
#include "irc.h"
#include "irc_connection.h"
#include "irc_user.h"
#include "irc_channel.h"
#include "../config.h"
#include "../utility.h"
#include "../mantisbt.h"

#define MAX_BUF 2048

IRCConnection::IRCConnection( str node, uint16 port )  :
	AbstractTCPSocket( node, port ),
	m_loggedIn( false ) {}

void IRCConnection::privmsg( str target, str msg )
{
	// If the target blocks color codes, we need to strip those now.
	IRCChannel* chan = findChannel( target );
	if( chan && chan->getMode( ChanMode_BlockColors ))
	{
		msg = msg.strip({ '\002', '\x0F' });
		
		// Strip the color codes. This assumes the colors are always 5 characters, though..
		long code;
		while(( code = msg.first( "\003" )) != -1 )
			msg = msg.substr( 0, code ) + msg.substr( code + 6, -1 );
	}
	
	write({ fmt( "PRIVMSG %1 :%2", target, msg )});
}

void IRCConnection::incoming( str data )
{
	// If we haven't authed yet, do so.
	if( !authed )
	{
		write( {"USER %1 0 * :%2", cfg( Name::Username ), cfg( Name::Realname )} );
		write( {"NICK %1", cfg( Name::Nickname )} );
		
		str ircpass = cfg( Name::IRCPassword );
		if( ircpass != "" )
			write({ "PASS %1", ircpass });
		
		setCurrentNickname( cfg( Name::Nickname ));
		authed = true;
	}
	
	// Remove crap off the message.
	data = data.strip( {17, '\r'} );
	
	// Deliminate it
	vector<str> tokens = data.split( " " );
	
	if( tokens.size() == 0 )
		return;
	
	// If the server is pinging us, reply with a pong.
	if( +tokens[0] == "PING" )
	{
		write({ "PONG %1", tokens[1] });
		return;
	}
	
	// =========================================================================
	// NUMERIC CODES
	switch( atoi( tokens[1] ) )
	{
	case RPL_WELCOME:
		// Server accepted us, tell it that we're joining
		write({ "JOIN %1", cfg( Name::Channel ) });
		write({ "MODE %1 %2", currentNickname(), cfg( Name::UserFlags ) });
		
		setLoggedIn( true );
		break;
	
	case ERR_NEEDMOREPARAMS:
	case ERR_ERRONEUSNICKNAME:
		// Well shit
		if( !loggedIn())
			fatal( "Server rejected the user credentials." );
		
		break;
	
	case RPL_NAMEREPLY:
		namesResponse( tokens );
		break;
	
	case RPL_ENDOFNAMES:
		// Request information about this channel's users
		write({ "WHO %1", tokens[3] });
		namesdone = true;
		break;
	
	case RPL_WHOREPLY:
		whoReply( data, tokens );
		break;
	
	case ERR_NICKNAMEINUSE:
		setCurrentNickname( currentNickname() + "_" );
		write({ "NICK :%1", currentNickname() });
		break;
	
	case RPL_CHANNELMODEIS:
		{
			IRCChannel* chan = findChannel( tokens[3] );
			if( !chan )
				break;
			
			chan->applyModeString( data.substr( data.posof( 4 ) + 1, - 1 ));
		}
		break;
	
	case RPL_WHOISACCOUNT:
		if( tokens.size() >= 4 )
		{
			IRCUser* user = findUser( tokens[3] );
			str acc = tokens[4];
			
			if( user )
				user->setAccount( acc );
		}
		break;
	
	case 0:
		nonNumericResponse( data, tokens );
		break;
	}
}

IRCChannel* IRCConnection::findChannel( str name )
{
	for( IRCChannel * chan : m_channels )
		if( chan->name() == name )
			return chan;
	
	return null;
}

void IRCConnection::namesResponse( const vector<str>& tokens )
{
	IRCChannel* chan = findChannel( tokens[4] );
	
	if( !chan )
		return;
	
	for( uint i = 5; i < tokens.size(); i++ )
	{
		str nick = tokens[i];
		
		if( !nick.len() )
			continue;
		
		// The first entry is prefixed with a `:`
		if( i == 5 )
			nick = nick.substr( 1, -1 );
		
		// Determine status
		long statusflags = IRCChannel::Normal;
		
		uint j;
		for( j = 0; j < nick.len(); j++ )
		{
			long flag = valueMap<long>( nick[j], IRCChannel::Normal, 5,
				cfg( Name::OpSymbol )[0],      IRCChannel::Op,
				cfg( Name::HalfOpSymbol )[0],  IRCChannel::HalfOp,
				cfg( Name::VoiceSymbol )[0],   IRCChannel::Voiced,
				cfg( Name::ProtectSymbol )[0], IRCChannel::Admin,
				cfg( Name::OwnerSymbol )[0],   IRCChannel::Owner );
			
			if( flag )
				statusflags |= flag;
			else
				break;
		}
		
		// Remove the status symbol from the name now that
		// we have identified it.
		nick = nick.substr( j, -1 );
		
		// If we don't already have him listed, create an entry.
		IRCUser* info = getUser( nick );
		
		IRCChannel::Entry* e = chan->findUser( nick );
		if( e == null )
			e = chan->addUser( info );
		
		e->setStatus( statusflags );
	}
	
	namesdone = false;
}

void IRCConnection::whoReply( str data, const vector<str>& tokens )
{
	// We recieve information about a user.
	str user = tokens[4];
	str host = tokens[5];
	str server = tokens[6];
	str nick = tokens[7];
	str flags = tokens[8];
	str name = data.substr( data.posof( 10 ) + 1, -1 );
	
	// Get the meta object
	IRCUser* info = getUser( nick );
	
	if( !info )
		warn( "Unknown user '%1'", nick );
	
	info->setUser( user );
	info->setHost( host );
	info->setName( name );
	info->setServer( server );
	
	// Scan through the flags field for stuff
	for( uint i = 0; i < flags.len(); i++ )
	{
		switch( flags[i] )
		{
		case 'H': // Here
			*info &= ~IRCUser::Away;
			break;
		
		case 'G': // Gone
			*info |= IRCUser::Away;
			break;
		
		case '*': // IRC operator
			*info |= IRCUser::IRCOp;
			break;
		}
	}
}

void IRCConnection::nonNumericResponse( const str& data, const vector<str>& tokens )
{
	str usermask = tokens[0].substr( 1, tokens[0].len( ));
	str codestring = +tokens[1];
	str nick, user, host;
	int excl = usermask.first( "!" ),
		atsign = usermask.first( "@" );
	
	if( ~excl && ~atsign )
	{
		nick = usermask.substr( 0, excl );
		user = usermask.substr( excl + 1, atsign );
		host = usermask.substr( atsign + 1, -1 );
	}
	
	// Try find meta for this user
	IRCUser* info = findUser( nick );
	
	// Check whether the user is an admin now.
	if( info )
		info->checkAdmin();
	
	if( codestring == "JOIN" )
	{
		if( !info )
			info = getUser( nick );
		
		IRCChannel* chan = getChannel( tokens[2].substr( 1, -1 ));
		chan->addUser( info );
		
		// Fill in the user and host
		if( info )
		{
			info->setUser( user );
			info->setHost( host );
		}
		
		// If it was us who joined, mark down the pointer so we know who we are.
		// Otherwise, send a WHO request to get more information.
		if( nick == currentNickname() )
		{
			setMe( info );
			print( "Joined channel at %1\n", Time::now().seconds() );
		}
		else
			write({ "WHO %1", nick });
	}
	else if( codestring == "PRIVMSG" )
	{
		str target = tokens[2];
		IRCChannel* chan = findChannel( tokens[2] ); // if null, this is PM
		str message = data.substr( data.posof( 3 ) + 2, -1 );
		
		if( chan && Time::now().seconds() <= chan->joinTime().seconds() )
			return;
		
		// Rid the initial :
		if( message[0] == ':' )
			message -= -1;
		
		vector<str> msgargs = message / " ";
		IRCUser* ircuser = getUser( nick );
		
		// ========================
		// CTCP support
		if( message[0] == '\001' )
		{
			str ctcpcode = +( msgargs[0].substr( 1, -1 ) );
			
			if( ctcpcode[ctcpcode.len() - 1] == 1 )
				ctcpcode -= 1;
			
			// Let's assume that the \001 is the last character in the message,
			// and that there's no \001's before the actual CTCP data.
			// Unless some IRC client goes nuts this should cause no problems.
			int ctcpargidx = message.first( " ", message.first( "\1" ) ) + 1;
			str ctcpargs = message.substr( ctcpargidx, message.last( "\1" ) );
			
			if( ctcpcode == "VERSION" )
				write({ "NOTICE %1 :\001VERSION %2 %3.%4\001",
					nick, APPNAME, VERSION_MAJOR, VERSION_MINOR });
			else if( ctcpcode == "PING" )
				write({ "NOTICE %1 :\001PING %2\001", nick, ctcpargs });
			else if( ctcpcode == "TIME" )
				write({ "NOTICE %1 :\001TIME %2\001", nick, Date( Time::now() ) });
			
			return;
		}
		
		// Try see if the message contained a Zandronum tracker link
		str url = cfg( Name::TrackerURL );
		for( const str& tok : msgargs )
		{
			str pagepart = "view.php?id=";
			if( tok.substr( 0, ~url ) == url && tok.substr( ~url, ~url + ~pagepart ) == pagepart )
			{
				// Twist the ID string a bit to flush out unwanted characters (punctuation, etc)
				str idstr = tok.substr( ~url + ~pagepart, -1 );
				int id = atoi( idstr );
				str idstr2 = variant( id ).stringRep ();
				
				str val;
				ticketinfo( idstr, val );
				privmsg( target, val );
			}
		}
		
		if( message[0] == cfg( Name::IRCCommandPrefix )[0] )
		{
			str cmdname = msgargs[0].substr( 1, -1 );
			
			// Paranoia
			if( ircuser == null )
			{
				write( {"PRIVMSG %1 :Who are you, %2?", target, nick} );
				return;
			}
			
			// This was a command, try parse it as such
			for( const IRCCommandInfo & info : g_IRCCommands )
			{
				if( -cmdname == info.namestring )
				{
					// Found the command, call it and pass it the arguments
					( *info.func )( this, ircuser, chan, message, msgargs );
					return;
				}
			}
			
			write( {"PRIVMSG %1 :Unknown command `%2%3`!",
					target, cfg( Name::IRCCommandPrefix )[0], cmdname
				   } );
			return;
		}
	}
	else if( codestring == "PART" )
	{
		// User left the channel, mark him out of the user list.
		str channame = tokens[2];
		
		if( channame[0] == ':' )
			channame -= -1;
		
		IRCChannel* chan = findChannel( channame );
		if( chan && info )
			chan->delUser( info );
	}
	else if( codestring == "QUIT" )
	{
		// User quit IRC so he goes out of the user list globally.
		delUser( nick );
		
		// If it was our nickname, reclaim it now
		if( +nick == +cfg( Name::Nickname ))
		{
			write({ "NICK :%1", cfg( Name::Nickname ) });
			setCurrentNickname( cfg( Name::Nickname ));
		}
	}
	else if( codestring == "MODE" )
	{
		IRCChannel* chan = findChannel( tokens[2] );
		if( chan )
		{
			chan->applyModeString( data.substr( data.posof( 3 ) + 1, -1 ));
		}
	}
	else if( codestring == "NICK" )
	{
		// User changed his nickname, update his meta.
		str newnick = tokens[2];
		
		IRCUser* info = findUser( nick );
		
		if( info )
		{
			info->setNick( newnick );
			info->checkAdmin();
		}
		else
			warn( "Unknown user %1!", nick );
	}
}

IRCChannel* IRCConnection::getChannel( str name )
{
	IRCChannel* chan = findChannel( name );
	
	if( chan == null )
	{
		chan = new IRCChannel( name );
		m_channels << chan;
		
		// Request mode string
		write({ "MODE %1", name });
	}
	
	return chan;
}

IRCUser* IRCConnection::findUser( str nick )
{
	for( uint i = 0; i < users.size(); i++ )
		if( +users[ i ]->nick() == +nick )
			return users[ i ];

	return null;
}

IRCUser* IRCConnection::getUser( str nick )
{
	if( !nick.len( ) )
		return null;
	
	IRCUser* info = findUser( nick );
	
	if( info != null )
		return info;
	
	// Didn't find him. Create a default info set and return it.
	info = newUser( nick );
	
	return users << info;
}

IRCUser* IRCConnection::newUser( str nick )
{
	IRCUser* info = new IRCUser;
	info->setNick( nick );
	
	// Request a /WHOIS on this user so we get the NickServ account name.
	// write({ "WHOIS %1", nick });
	
	return info;
}

void IRCConnection::delUser( str nick )
{
	IRCUser* user = findUser( nick );
	
	// Remove this user from all channels
	for( IRCChannel* chan : m_channels )
		chan->delUser( user );
	
	for( uint i = 0; i < users.size(); i++ )
	{
		if( users[i] == user )
		{
			delete users[i];
			users.erase( i );
			break;
		}
	}
}
