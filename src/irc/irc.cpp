#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <libcobaltcore/xml.h>
#include "../main.h"
#include "../utility.h"
#include "../mantisbt.h"
#include "irc.h"
#include "connection.h"
#include "user.h"
#include "channel.h"
#include "commands.h"

CONFIG (String,    irc_usermodes, "+iw")
CONFIG (String,    irc_nickname, APPNAME)
CONFIG (String,    irc_username, APPNAME)
CONFIG (String,    irc_realname, APPNAME)
CONFIG (String,    irc_password, "")
CONFIG (String,    irc_chan_ownersymbol, "~")
CONFIG (String,    irc_chan_adminsymbol, "&")
CONFIG (String,    irc_chan_opsymbol, "@")
CONFIG (String,    irc_chan_halfopsymbol, "%")
CONFIG (String,    irc_chan_voicesymbol, "+")
CONFIG (String,    irc_commandprefix, "&")
EXTERN_CONFIG (String, tracker_url)

// =============================================================================
// -----------------------------------------------------------------------------
IRCConnection::IRCConnection (str node, uint16 port) :
	CoTCPSocket (node, port),
	m_loggedIn (false)
{	setBlocking (false);
	setVerbose (true);
}

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::privmsg (str target, str msg)
{	// If the target blocks color codes, we need to strip those now.
	IRCChannel* chan = findChannel (target);

	if (chan && chan->getMode (ChanMode_BlockColors))
	{	msg.replace ("\002", "");
		msg.replace ("\x0F", "");

		// Strip the color codes. This assumes the colors are always 5 characters, though..
		/*
		long code;

		while ((code = msg.first ("\003")) != -1)
			msg = msg.substr (0, code) + msg.substr (code + 5, -1);
		*/
	}

	// Encode \r and \n for the sake of security.
	msg.replace ("\n", "\\n");
	msg.replace ("\r", "\\r");

	write (Format ("PRIVMSG %1 :%2", target, msg));
}

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::incoming (str data)
{	// If we haven't authed yet, do so.
	if (!authed)
	{	write (Format ("USER %1 0 * :%2", irc_username, irc_realname));
		write (Format ("NICK cobalt", irc_nickname));

		if (irc_password.length() > 0)
			write (Format ("PASS %1", irc_password));

		setCurrentNickname (irc_nickname);
		authed = true;
	}

	// Remove crap off the message.
	data.replace ("\x11", "");
	data.replace ("\r", "");

	// Deliminate it
	CoStringList tokens = data.split (" ");

	if (tokens.size() == 0)
		return;

	// If the server is pinging us, reply with a pong.
	if (+tokens[0] == "PING")
	{	write (Format ("PONG %1", tokens[1]));
		return;
	}

	// =========================================================================
	// NUMERIC CODES
	switch (tokens[1].toLong())
	{
		case RPL_WELCOME:
		{	// Server accepted us, tell it that we're joining
			CoXMLNode* parent;

			// Join any channels defined in our xml
			if ( (parent = CoConfig::xml()->navigateTo ( {"irc", "channels"})) != null)
			{	CoList<CoXMLNode*> nodes = parent->getNodesByName ("channel");

			for (CoXMLNode * node : nodes)
				{	CoString joincmd = Format ("JOIN %1", node->attribute ("name"));
					CoString passwd;

					if ( (passwd = node->attribute ("password")).length() > 0)
						joincmd += Format (" %1", passwd);

					write (joincmd);
				}
			}

			write (Format ("MODE %1 %2", currentNickname(), irc_usermodes));
			setLoggedIn (true);
		} break;

		case ERR_NEEDMOREPARAMS:
		case ERR_ERRONEUSNICKNAME:
		{	// Well shit
			if (!loggedIn())
				fatal ("Server rejected the user credentials.");
		} break;

		case RPL_NAMEREPLY:
		{	namesResponse (tokens);
		} break;

		case RPL_ENDOFNAMES:
		{	// Request information about this channel's users
			write (Format ("WHO %1", tokens[3]));
			namesdone = true;
		} break;

		case RPL_WHOREPLY:
		{	whoReply (data, tokens);
		} break;

		case ERR_NICKNAMEINUSE:
		{	setCurrentNickname (currentNickname() + "_");
			write (Format ("NICK :%1", currentNickname()));
		} break;

		case RPL_CHANNELMODEIS:
		{	IRCChannel* chan = findChannel (tokens[3]);

			if (!chan)
				break;

			CoStringList modelist;

			for (int i = 4; i < tokens.size(); ++i)
				modelist << tokens[i];

			chan->applyModeString (join (modelist, " "));
		} break;

		case RPL_WHOISACCOUNT:
		{	if (tokens.size() >= 4)
			{	IRCUser* user = findUser (tokens[3]);
				str acc = tokens[4];

				if (user)
					user->setAccount (acc);
			}
		} break;

		case 0:
		{	nonNumericResponse (data, tokens);
		} break;
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
IRCChannel* IRCConnection::findChannel (str name)
{	for (IRCChannel* chan : m_channels)
		if (chan->name() == name)
			return chan;

	return null;
}

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::namesResponse (CoStringListRef tokens)
{	IRCChannel* chan = findChannel (tokens[4]);

	if (!chan)
		return;

	for (int i = 5; i < tokens.size(); i++)
	{	str nick = tokens[i];

		if (!nick.length())
			continue;

		// The first entry is prefixed with a `:`
		if (i == 5)
			nick.remove (0, 1);

		// Determine status
		long statusflags = IRCChannel::Normal;

		int j;

		for (j = 0; j < nick.length(); j++)
		{	long flag = valueMap<long> (nick[j], IRCChannel::Normal, 5,
				irc_chan_opsymbol[0],      IRCChannel::Op,
				irc_chan_halfopsymbol[0],  IRCChannel::HalfOp,
				irc_chan_voicesymbol[0],   IRCChannel::Voiced,
				irc_chan_adminsymbol[0],   IRCChannel::Admin,
				irc_chan_ownersymbol[0],   IRCChannel::Owner);

			if (flag)
				statusflags |= flag;
			else
				break;
		}

		// Remove the status symbol from the name now that
		// we have identified it.
		nick = nick.substr (j, -1);

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
void IRCConnection::whoReply (str data, const list<str>& tokens)
{	// We recieve information about a user.
	str user = tokens[4];
	str host = tokens[5];
	str server = tokens[6];
	str nick = tokens[7];
	str flags = tokens[8];
	str name = data.substr (posof (data, 10) + 1, -1);

	// Get the meta object
	IRCUser* info = getUser (nick);

	if (!info)
		warn ("Unknown user '%1'", nick);

	info->setUser (user);
	info->setHost (host);
	info->setName (name);
	info->setServer (server);

	// Scan through the flags field for stuff
	for (int i = 0; i < flags.length(); i++)
	{	switch (flags[i])
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

// =============================================================================
// -----------------------------------------------------------------------------
void IRCConnection::nonNumericResponse (CoStringRef data, CoStringListRef tokens)
{	str usermask = tokens[0].substr (1, tokens[0].length());
	str codestring = +tokens[1];
	str nick, user, host;
	int excl = usermask.first ("!"),
		atsign = usermask.first ("@");

	if (~excl && ~atsign)
		nick = usermask.substr (0, excl);

	user = usermask.substr (excl + 1, atsign);
	host = usermask.substr (atsign + 1);

	// Try find meta for this user
	IRCUser* info = findUser (nick);

	// Check whether the user is an admin now.
	if (info)
		info->checkAdmin();

	if (codestring == "JOIN")
	{	if (!info)
			info = getUser (nick);

		IRCChannel* chan = getChannel (tokens[2].substr (1));
		chan->addUser (info);

		// Fill in the user and host
		if (info)
		{	info->setUser (user);
			info->setHost (host);
		}

		// If it was us who joined, mark down the pointer so we know who we are.
		// Otherwise, send a WHO request to get more information.
		if (nick == currentNickname())
			setMe (info);
		else
			write (Format ("WHO %1", nick));
	} elif (codestring == "PRIVMSG")

	{	str target = tokens[2];
		IRCChannel* chan = findChannel (tokens[2]);  // if null, this is PM
		str message = data.substr (posof (data, 3) + 2);

		// [TODO] Is this still needed?
		/*
		if (chan && chan->joinTime().secsTo (QTime::currentTime()) < 1)
			return;
		*/

		// Rid the initial :
		if (message[0] == ':')
			message.remove (0, 1);

		CoStringList msgargs = message.split (" ");
		IRCUser* ircuser = getUser (nick);

		// ========================
		// CTCP support
		if (message[0] == '\001')
		{	str ctcpcode = + (msgargs[0].substr (1, -1));

			if (ctcpcode[ctcpcode.length() - 1] == 1)
				ctcpcode -= 1;

			// Let's assume that the \001 is the last character in the message,
			// and that there's no \001's before the actual CTCP data.
			// Unless some IRC client goes nuts this should cause no problems.
			int ctcpargidx = message.first (" ", message.first ("\1")) + 1;
			str ctcpargs = message.substr (ctcpargidx, message.last ("\1"));

			if (ctcpcode == "VERSION")
				write (Format ("NOTICE %1 :\001VERSION %2 %3.%4\001", nick, APPNAME, VERSION_MAJOR, VERSION_MINOR));
			elif (ctcpcode == "PING")
				write (Format ("NOTICE %1 :\001PING %2\001", nick, ctcpargs));
			elif (ctcpcode == "TIME")
				write (Format ("NOTICE %1 :\001TIME %2\001", nick, CoDate (CoTime::now())));

			return;
		}

		// Try see if the message contained a Zandronum tracker link
		for (CoStringRef tok : msgargs)
		{	if (
				(tok.startsWith ("http://" + tracker_url) || tok.startsWith ("https://" + tracker_url)) &&
				tok.first ("view.php?id=") != -1
			)
			{	// Twist the ID string a bit to flush out unwanted characters (punctuation, etc)
				str idstr = tok.substr (tok.first ("?id=") + 4);
				str idstr2 = str::fromNumber (idstr.toLong());

				str val;

				if (ticketinfo (idstr2, val))
					privmsg (target, val);
			}
		}

		if (message[0] == irc_commandprefix[0])
		{	str cmdname = msgargs[0].substr (1);

			// Paranoia
			if (ircuser == null)
			{	write (Format ("PRIVMSG %1 :Who are you, %2?", target, nick));
				return;
			}

			// Try find this in our list of commands, check and launch it.
			for (int i = 0; i < g_numIRCCommands; ++i)
			{	const IRCCommandInfo& cmdinfo = g_IRCCommands[i];

				if (-cmdname == cmdinfo.namestring)
				{	if (!ircuser->canCallCommand (cmdinfo, chan))
					{	privmsg (target, "You may not call that");
						return;
					}

					(*cmdinfo.func) (this, ircuser, chan, message, msgargs);
					return;
				}
			}

			return;
		}
	} elif (codestring == "PART")
	{	// User left the channel, mark him out of the user list.
		str channame = tokens[2];

		if (channame[0] == ':')
			channame -= -1;

		IRCChannel* chan = findChannel (channame);

		if (chan && info)
			chan->delUser (info);
	} elif (codestring == "QUIT")
	{	// User quit IRC so he goes out of the user list globally.
		delUser (nick);

		// If it was our nickname, reclaim it now
		if (+nick == +irc_nickname)
		{	write (Format ("NICK :%1", irc_nickname));
			setCurrentNickname (irc_nickname);
		}
	} elif (codestring == "MODE")
	{	IRCChannel* chan = findChannel (tokens[2]);

		if (chan)
			chan->applyModeString (data.substr (posof (data, 3) + 1));
	} elif (codestring == "NICK")
	{	// User changed his nickname, update his meta.
		str newnick = tokens[2];
		IRCUser* info = findUser (nick);

		if (info)
		{	info->setNick (newnick);
			info->checkAdmin();
		}
		else
			warn ("Unknown user %1!", nick);
	}
}

IRCChannel* IRCConnection::getChannel (str name)
{	IRCChannel* chan = findChannel (name);

	if (chan == null)
	{	chan = new IRCChannel (name);
		m_channels << chan;

		// Request mode string
		write (Format ("MODE %1", name));
	}

	return chan;
}

IRCUser* IRCConnection::findUser (str nick)
{	for (int i = 0; i < users.size(); i++)
		if (+users[i]->nick() == +nick)
			return users[i];

	return null;
}

IRCUser* IRCConnection::getUser (str nick)
{	if (!nick.length())
		return null;

	IRCUser* info = findUser (nick);

	if (info != null)
		return info;

	// Didn't find him. Create a default info set and return it.
	info = newUser (nick);

	return users << info;
}

IRCUser* IRCConnection::newUser (str nick)
{	IRCUser* info = new IRCUser;
	info->setNick (nick);

	// Request a /WHOIS on this user so we get the NickServ account name.
	// write({ "WHOIS %1", nick });

	return info;
}

void IRCConnection::delUser (str nick)
{	IRCUser* user = findUser (nick);

	// Remove this user from all channels
	for (IRCChannel * chan : m_channels)
		chan->delUser (user);

	for (int i = 0; i < users.size(); i++)
	{	if (users[i] == user)
		{	delete users[i];
			users.erase (i);
			break;
		}
	}
}

str IRC::getColor (IRC::Color fg)
{	char fgcode[3];
	sprintf (fgcode, "%.2d", fg);
	return str (COLORCHAR) + fgcode;
}

str IRC::getColor (IRC::Color fg, IRC::Color bg)
{	char bgcode[3];
	sprintf (bgcode, "%.2d", bg);
	return getColor (fg) + "," + bgcode;
}
