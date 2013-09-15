#include <QTcpSocket>
#include <QStringList>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "../main.h"
#include "../config.h"
#include "../utility.h"
#include "../mantisbt.h"
#include "irc.h"
#include "irc_connection.h"
#include "irc_user.h"
#include "irc_channel.h"

CONFIG (String, channel, "#null-object")
CONFIG (String, usermodes, "+iw")
CONFIG (String, nickname, "cobalt")
CONFIG (String, username, "cobalt")
CONFIG (String, realname, "cobalt")
CONFIG (String, password, "w3b0tt1ngn0w")
CONFIG (String, trackerurl, "zandronum.com/tracker")

// =============================================================================
// -----------------------------------------------------------------------------
IRCConnection::IRCConnection (str node, uint16 port) :
	m_loggedIn (false),
	m_conn (new QTcpSocket)
{
	m_conn->connectToHost (node, port);
}

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::privmsg (str target, str msg) {
	// If the target blocks color codes, we need to strip those now.
	IRCChannel* chan = findChannel (target);
	
	if (chan && chan->getMode (ChanMode_BlockColors)) {
		msg.remove ('\002');
		msg.remove ('\x0F');
		
		// Strip the color codes. This assumes the colors are always 5 characters, though..
		long code;
		
		while ((code = msg.indexOf ("\003")) != -1)
			msg.remove (code, 5);
	}
	
	// Encode \r and \n for the sake of security.
	msg.replace ("\n", "\\n");
	msg.replace ("\r", "\\r");
	
	write (fmt ("PRIVMSG %1 :%2", target, msg));
}

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::write (const str& msg) {
	m_conn->write (msg, msg.length());
}

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::incoming (str data) {
	// If we haven't authed yet, do so.
	if (!authed) {
		write (fmt ("USER %1 0 * :%2", cfg::username, cfg::realname));
		write (fmt ("NICK cobalt", cfg::nickname));
		
		str ircpass = cfg::password;
		
		if (ircpass != "")
			write (fmt ("PASS %1", ircpass));
		
		setCurrentNickname (cfg::nickname);
		authed = true;
	}
	
	// Remove crap off the message.
	data.remove (QChar (17));
	data.remove ('\r');
	
	// Deliminate it
	QStringList tokens = data.split (" ", QString::SkipEmptyParts);
	
	if (tokens.size() == 0)
		return;
	
	// If the server is pinging us, reply with a pong.
	if (+tokens[0] == "PING") {
		write (fmt ("PONG %1", tokens[1]));
		return;
	}
	
	// =========================================================================
	// NUMERIC CODES
	switch (tokens[1].toInt()) {
	case RPL_WELCOME:
		// Server accepted us, tell it that we're joining
		write (fmt ("JOIN %1", cfg::channel));
		write (fmt ("MODE %1 %2", currentNickname(), cfg::usermodes));
		
		setLoggedIn (true);
		break;
		
	case ERR_NEEDMOREPARAMS:
	case ERR_ERRONEUSNICKNAME:
		// Well shit
		if (!loggedIn())
			fatal ("Server rejected the user credentials.");
		
		break;
	
	case RPL_NAMEREPLY:
		namesResponse (tokens);
		break;
	
	case RPL_ENDOFNAMES:
		// Request information about this channel's users
		write ({ "WHO %1", tokens[3] });
		namesdone = true;
		break;
	
	case RPL_WHOREPLY:
		whoReply (data, tokens);
		break;
	
	case ERR_NICKNAMEINUSE:
		setCurrentNickname (currentNickname() + "_");
		write ({ "NICK :%1", currentNickname() });
		break;
	
	case RPL_CHANNELMODEIS:
		{
			IRCChannel* chan = findChannel (tokens[3]);
			
			if (!chan)
				break;
			
			QStringList modelist;
			for (int i = 4; i < tokens.size(); ++i)
				modelist += tokens[i];
			chan->applyModeString (modelist.join (" "));
		}
		break;
	
	case RPL_WHOISACCOUNT:
		if (tokens.size() >= 4) {
			IRCUser* user = findUser (tokens[3]);
			str acc = tokens[4];
			
			if (user)
				user->setAccount (acc);
		}
		
		break;

	case 0:
		nonNumericResponse (data, tokens);
		break;
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
IRCChannel* IRCConnection::findChannel (str name) {
	for (IRCChannel* chan : m_channels)
		if (chan->name() == name)
			return chan;
	
	return null;
}

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::namesResponse (const QStringList& tokens) {
	IRCChannel* chan = findChannel (tokens[4]);
	
	if (!chan)
		return;
	
	for (int i = 5; i < tokens.size(); i++) {
		str nick = tokens[i];
		
		if (!nick.length())
			continue;
		
		// The first entry is prefixed with a `:`
		if (i == 5)
			nick.remove (0, 1);
		
		// Determine status
		long statusflags = IRCChannel::Normal;
		
		int j;
		
		for (j = 0; j < nick.length(); j++) {
			long flag = valueMap<long> (nick[j], IRCChannel::Normal, 5,
				cfg (Name::OpSymbol) [0],      IRCChannel::Op,
				cfg (Name::HalfOpSymbol) [0],  IRCChannel::HalfOp,
				cfg (Name::VoiceSymbol) [0],   IRCChannel::Voiced,
				cfg (Name::ProtectSymbol) [0], IRCChannel::Admin,
				cfg (Name::OwnerSymbol) [0],   IRCChannel::Owner);
			
			if (flag)
				statusflags |= flag;
			else
				break;
		}
		
		// Remove the status symbol from the name now that
		// we have identified it.
		nick = nick.mid (j);
		
		// If we don't already have him listed, create an entry.
		IRCUser* info = getUser (nick);
		IRCChannel::Entry* e = chan->findUser (nick);
		
		if (e == null)
			e = chan->addUser (info);
		
		e->setStatus (statusflags);
	}
	
	namesdone = false;
}

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::whoReply (str data, const list<str>& tokens) {
	// We recieve information about a user.
	str user = tokens[4];
	str host = tokens[5];
	str server = tokens[6];
	str nick = tokens[7];
	str flags = tokens[8];
	str name = data.mid (posof (data, 10) + 1);

	// Get the meta object
	IRCUser* info = getUser (nick);

	if (!info)
		warn ("Unknown user '%1'", nick);

	info->setUser (user);
	info->setHost (host);
	info->setName (name);
	info->setServer (server);

	// Scan through the flags field for stuff
	for (int i = 0; i < flags.length(); i++) {
		switch (flags[i]) {
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

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::nonNumericResponse (const str& data, const list<str>& tokens) {
	str usermask = tokens[0].substr (1, tokens[0].len());
	str codestring = +tokens[1];
	str nick, user, host;
	int excl = usermask.indexOf ("!"),
		atsign = usermask.indexOf ("@");
	
	if (~excl && ~atsign)
		nick = usermask.left (excl);
	
	user = usermask.mid (excl + 1, usermask.length() - atsign);
	host = usermask.mid (atsign + 1);
	
	// Try find meta for this user
	IRCUser* info = findUser (nick);
	
	// Check whether the user is an admin now.
	if (info)
		info->checkAdmin();
	
	if (codestring == "JOIN") {
		if (!info)
			info = getUser (nick);
		
		IRCChannel* chan = getChannel (tokens[2].mid (1));
		chan->addUser (info);
		
		// Fill in the user and host
		if (info) {
			info->setUser (user);
			info->setHost (host);
		}
		
		// If it was us who joined, mark down the pointer so we know who we are.
		// Otherwise, send a WHO request to get more information.
		if (nick == currentNickname())
			setMe (info);
		else
			write ({ "WHO %1", nick });
	} elif (codestring == "PRIVMSG") {
		str target = tokens[2];
		IRCChannel* chan = findChannel (tokens[2]);  // if null, this is PM
		str message = data.mid (posof (data, 3) + 2);
		
		if (chan && chan->joinTime().secsTo (QTime::currentTime()) < 1)
			return;
		
		// Rid the initial :
		if (message[0] == ':')
			message -= -1;
		
		QStringList msgargs = message.split (" ");
		IRCUser* ircuser = getUser (nick);
		
		// ========================
		// CTCP support
		if (message[0] == '\001') {
			str ctcpcode = + (msgargs[0].substr (1, -1));
			
			if (ctcpcode[ctcpcode.len() - 1] == 1)
				ctcpcode -= 1;
			
			// Let's assume that the \001 is the last character in the message,
			// and that there's no \001's before the actual CTCP data.
			// Unless some IRC client goes nuts this should cause no problems.
			int ctcpargidx = message.first (" ", message.first ("\1")) + 1;
			str ctcpargs = message.substr (ctcpargidx, message.last ("\1"));
			
			if (ctcpcode == "VERSION")
				write ({ "NOTICE %1 :\001VERSION %2 %3.%4\001",
					 nick, APPNAME, VERSION_MAJOR, VERSION_MINOR});
			elif (ctcpcode == "PING")
			write ({ "NOTICE %1 :\001PING %2\001", nick, ctcpargs });
			elif (ctcpcode == "TIME")
			write ({ "NOTICE %1 :\001TIME %2\001", nick, Date (Time::now()) });

			return;
		}

		// Try see if the message contained a Zandronum tracker link
		str url = cfg::trackerurl;
		
		for (const str& tok : msgargs) {
			if (
				(tok.startsWith ("http://" + url) || tok.startsWith ("https://" + url)) &&
				tok.indexOf ("view.php?id=") != -1
			) {
				// Twist the ID string a bit to flush out unwanted characters (punctuation, etc)
				str idstr = tok.mid (tok.indexOf ("?id=") + 4, -1);
				print ("idstring: %1\n", idstr);
				str idstr2 = str::number (idstr.toInt());
				print ("idstring 2: %1\n", idstr2);
				
				str val;
				
				if (ticketinfo (idstr2, val))
					privmsg (target, val);
			}
		}
		
		if (message[0] == cfg (Name::IRCCommandPrefix) [0]) {
			str cmdname = msgargs[0].mid (1);
			
			// Paranoia
			if (ircuser == null) {
				write ( {"PRIVMSG %1 :Who are you, %2?", target, nick});
				return;
			}
			
			// This was a command, try parse it as such
			for (const IRCCommandInfo & info : g_IRCCommands) {
				if (-cmdname == info.namestring) {
					// Found the command, call it and pass it the arguments
					(*info.func) (this, ircuser, chan, message, msgargs);
					return;
				}
			}
			
			write ( { "PRIVMSG %1 :Unknown command `%2%3`!",
					  target, cfg (Name::IRCCommandPrefix) [0], cmdname
					});
			return;
		}
	} elif (codestring == "PART") {
		// User left the channel, mark him out of the user list.
		str channame = tokens[2];
		
		if (channame[0] == ':')
			channame -= -1;
		
		IRCChannel* chan = findChannel (channame);
		
		if (chan && info)
			chan->delUser (info);
	} elif (codestring == "QUIT") {
		// User quit IRC so he goes out of the user list globally.
		delUser (nick);
		
		// If it was our nickname, reclaim it now
		if (+nick == +cfg::nickname) {
			write (fmt ("NICK :%1", cfg::nickname));
			setCurrentNickname (cfg::nickname);
		}
	} elif (codestring == "MODE") {
		IRCChannel* chan = findChannel (tokens[2]);
		
		if (chan)
			chan->applyModeString (data.mid (posof (data, 3) + 1));
	} elif (codestring == "NICK") {
		// User changed his nickname, update his meta.
		str newnick = tokens[2];
		IRCUser* info = findUser (nick);
		
		if (info) {
			info->setNick (newnick);
			info->checkAdmin();
		} else
			warn ("Unknown user %1!", nick);
	}
}

IRCChannel* IRCConnection::getChannel (str name) {
	IRCChannel* chan = findChannel (name);
	
	if (chan == null) {
		chan = new IRCChannel (name);
		m_channels << chan;
		
		// Request mode string
		write (fmt ("MODE %1", name));
	}
	
	return chan;
}

IRCUser* IRCConnection::findUser (str nick) {
	for (int i = 0; i < users.size(); i++)
		if (+users[i]->nick() == +nick)
			return users[i];
	
	return null;
}

IRCUser* IRCConnection::getUser (str nick) {
	if (!nick.length())
		return null;
	
	IRCUser* info = findUser (nick);
	
	if (info != null)
		return info;
	
	// Didn't find him. Create a default info set and return it.
	info = newUser (nick);
	
	return users << info;
}

IRCUser* IRCConnection::newUser (str nick) {
	IRCUser* info = new IRCUser;
	info->setNick (nick);
	
	// Request a /WHOIS on this user so we get the NickServ account name.
	// write({ "WHOIS %1", nick });
	
	return info;
}

void IRCConnection::delUser (str nick) {
	IRCUser* user = findUser (nick);
	
	// Remove this user from all channels
	for (IRCChannel* chan : m_channels)
		chan->delUser (user);
	
	for (int i = 0; i < users.size(); i++) {
		if (users[i] == user) {
			delete users[i];
			users.erase (i);
			break;
		}
	}
}

str IRC::getColor (IRC::Color fg) {
	char fgcode[3];
	sprintf (fgcode, "%.2d", fg);
	return str (COLORCHAR) + fgcode;
}

str IRC::getColor (IRC::Color fg, IRC::Color bg) {
	char bgcode[3];
	sprintf (bgcode, "%.2d", bg);
	return getColor (fg) + "," + bgcode;
}
