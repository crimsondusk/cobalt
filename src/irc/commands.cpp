#include <netdb.h>
#include "libcobaltcore/time.h"
#include "libcobaltcore/ip.h"
#include <libcobaltcore/xml.h>
#include "../main.h"
#include "../mantisbt.h"
#include "../zanserver.h"
#include "irc.h"
#include "channel.h"
#include "user.h"
#include "commands.h"
#include "connection.h"

#define CALL_COMMAND(A) \
	IRCCommand_##A (conn, invoker, channel, message, parms);

EXTERN_CONFIG (String, tracker_url)
EXTERN_CONFIG (String, irc_commandprefix)

IRCCommandInfo g_IRCCommands[256];
int            g_numIRCCommands = 0;

IRCCommandAdder::IRCCommandAdder (
	const char* namestring, IRCCommandType func,
	IRCChannel::Status status, bool adminOnly
)
{	g_IRCCommands[g_numIRCCommands++] = {namestring, func, status, adminOnly};
}

str IRCReplyTarget (IRCUser* sender, IRCChannel* chan)
{	if (chan)
		return chan->name();

	return sender->nick();
}

// =============================================================================
ADMIN_COMMAND (quit)
{	IRC_REPLY ("Bye.")
	conn->write ( { "QUIT :Leaving" });
}

// =============================================================================
ADMIN_COMMAND (raw)
{	int space = message.first (" ");
	str raw = message.substr (space + 1, -1);
	conn->write (Format ("%1\n", raw));
}

IRC_COMMAND (ban, HalfOp, false)
{	if (channel == null || conn->me()->chanStatus (channel) < IRCChannel::HalfOp)
		return;

	if (parms.size() < 2)
	{	IRC_REPLY ("Need a mask")
		return;
	}

	conn->write (Format ("MODE %1 +b %2\n", REPLY_TARGET, parms[1]));

	for (IRCUser* user : conn->users)
		if (mask (user->userhost(), parms[1]))
			conn->write (Format ("KICK %1 %2 :Banned", REPLY_TARGET, user->nick()));
}

ADMIN_COMMAND (testfatal)
{	if (invoker->isAdmin() == false)
		return;

	parms.erase (0);
	fatal ("%1", join (parms, " "));
}

ADMIN_COMMAND (whois)
{	if (parms.size() < 2)
	{	IRC_REPLY ("Who is... who?")
		return;
	}

	IRCUser* user = conn->findUser (parms[1]);

	if (user == null)
	{	IRC_REPLY ("I don't know who that is.")
		return;
	}

	CoStringList channames;

	for (IRCChannel* chan : user->channels())
		channames << chan->name();

	IRC_REPLY ("I think %1 is %2. Real name: %3", user->nick(), user->userhost(), user->name())
	IRC_REPLY ("I see %1 on %2", user->nick(), join (channames, ", "))
}

IRC_COMMAND (ticket, Normal, false)
{	if (parms.size() < 2)
	{	conn->privmsg (REPLY_TARGET, "Which ticket?");
		return;
	}

	if (atoi (parms[1]) <= 0)
		return;

	str idstr = CoString::fromNumber (parms[1].toLong());
	str msg;
	bool success = ticketinfo (idstr, msg);

	if (success)
	{	IRC_REPLY (msg)
		IRC_REPLY ("Link: %1view.php?id=%2", tracker_url, idstr)
	}
	else
		IRC_REPLY (msg)
	}

ADMIN_COMMAND (fullticketinfo)
{	if (parms.size() < 2)
	{	IRC_REPLY ("Which ticket?")
		return;
	}

	if (atoi (parms[1]) <= 0)
		return;

	str idstr = str::fromNumber (parms[1].toLong());
	str msg = fullticketinfo (idstr);

	for (const str& line : msg.split ("\n"))
		conn->privmsg (REPLY_TARGET, line);
}

IRC_COMMAND (commands, Normal, false)
{	CoStringList cmdnames;

	for (int i = 0; i < g_numIRCCommands; ++i)
	{	const IRCCommandInfo& cmd = g_IRCCommands[i];

		if (!invoker->canCallCommand (cmd, channel))
			continue;

		cmdnames << cmd.namestring;
	}

	cmdnames.sort();
	conn->privmsg (REPLY_TARGET, Format ("Available commands: %1", join (cmdnames, " ")));
}

ADMIN_COMMAND (msg)
{	if (parms.size() < 3)
		return;

	CoStringList msgparms = parms;
	msgparms.erase (0);
	msgparms.erase (0);
	str msg = join (msgparms, " ");
	conn->privmsg (parms[1], msg);
}

IRC_COMMAND (query, Normal, 0)
{	CoIPAddress addr;

	if (parms.size() < 2)
	{	conn->privmsg (REPLY_TARGET, "What shall I query?");
		return;
	}

	if (!CoIPAddress::fromString (parms[1], addr))
	{	IRC_REPLY ("Bad IP address!")
		return;
	}

	addServerQuery (addr, REPLY_TARGET);
}

IRC_COMMAND (resolve, Normal, 0)
{	if (parms.size() < 2)
		return;

	CoIPAddress addr;

	IRC_REPLY ("Resolving %1...", parms[1]);
	int r = CoIPAddress::resolve (parms[1], addr);

	if (r != 0)
		IRC_REPLY ("Failed to resolve: %1", CoString (gai_strerror (r)))
	else
		IRC_REPLY ("address: %1", addr)
}

IRC_COMMAND (masshighlight, HalfOp, false)
{	if (!channel)
		return;

	str nicklist;

	for (const IRCChannel::Entry & e : channel->userlist())
	{	if (nicklist.length() > 0)
			nicklist += ", ";

		nicklist += e.userinfo()->nick();

		if (nicklist.length() > 300)
		{	IRC_REPLY ("%1", nicklist);
			nicklist = "";
		}
	}

	if (nicklist.length() > 0)
		IRC_REPLY ("%1", nicklist);
}

IRC_COMMAND (mhl, HalfOp, false)
{	CALL_COMMAND (masshighlight)
}

ADMIN_COMMAND (add_autojoin)
{	if (parms.size() < 2)
		return;

	CoXMLNode* parent = CoConfig::xml()->navigateTo ( {"irc", "channels"}, true);

	// Check if we already have this in our auto-joins
	bool isNew = true;
	CoXMLNode* chanNode = null;

	for (CoXMLNode * subnode : parent->nodes())
	{	if (subnode->name() == "channel" && subnode->contents() == parms[1])
		{	chanNode = subnode;
			isNew = false;
			break;
		}
	}

	if (chanNode == null)
		chanNode = new CoXMLNode ("channel", parent);

	chanNode->setAttribute ("name", parms[1]);

	IRCChannel* chan = conn->findChannel (parms[1]);
	IRCChannel::Mode* mode_k = chan ? chan->getMode (ChanMode_Locked) : null;
	CoString passwordBlurb = "";

	if (mode_k != null)
	{	chanNode->setAttribute ("password", mode_k->arg);
		passwordBlurb = Format (" (password: %1)", mode_k->arg);
	}

	IRC_REPLY ("%1 auto-join channel %2%3", (isNew ? "Added" : "Updated"),  parms[1], passwordBlurb);
	save_configuration();
}

ADMIN_COMMAND (rehash)
{	if (!invoker->isAdmin())
		return;

	if (!CoConfig::load (get_config_file_name()))
		IRC_REPLY ("Error rehashing: %1", CoXMLDocument::parseError())
	else
		IRC_REPLY ("Rehash successful")
}

ADMIN_COMMAND (writecfg)
{	if (!invoker->isAdmin())
		return;

	if (!CoConfig::save (get_config_file_name()))
		IRC_REPLY ("Error saving: %1", strerror (errno))
	else
		IRC_REPLY ("Saved successfully")
}
