#include <netdb.h>
#include "libcobaltcore/time.h"
#include "libcobaltcore/ip.h"
#include "../main.h"
#include "../mantisbt.h"
#include "../zanserver.h"
#include "irc.h"
#include "irc_channel.h"
#include "irc_user.h"
#include "irc_connection.h"

#define REPLY(...) conn->privmsg (TARGET, fmt (__VA_ARGS__));

EXTERN_CONFIG (String, tracker_url)

static CoList<IRCCommandInfo> G_IRCCommands;

IRCCommandAdder::IRCCommandAdder (const char* namestring, IRCCommandType func) {
	const IRCCommandInfo info = {namestring, func};
	G_IRCCommands << info;
}

str IRCTarget (IRCUser* sender, IRCChannel* chan) {
	if (chan)
		return chan->name();
	
	return sender->nick();
}

#define TARGET IRCTarget (invoker, channel)

// =============================================================================
IRC_COMMAND (sayhi) {
	if (parms.size() < 2)
		conn->privmsg (TARGET, "Hi!");
	else
		conn->privmsg (TARGET, fmt ("Hi, %2!", parms[1]));
}

// =============================================================================
IRC_COMMAND (quit) {
	if (!invoker->isAdmin()) {
		REPLY ("%1: Who are you to tell me to GTFO?", invoker->nick())
		return;
	}
	
	REPLY ("Bye.")
	conn->write ({ "QUIT :Leaving" });
}

// =============================================================================
IRC_COMMAND (raw) {
	if (!invoker->isAdmin()) {
		REPLY ("Who are you to tell me what to do?")
		return;
	}
	
	int space = message.first (" ");
	str raw = message.substr (space + 1, -1);
	conn->write ({ "%1\n", raw });
}

IRC_COMMAND (ban) {
	if (channel == null ||
		invoker->chanStatus (channel) < IRCChannel::HalfOp ||
		conn->me()->chanStatus (channel) < IRCChannel::HalfOp
	)
		return;
	
	if (parms.size() < 2) {
		REPLY ("Need a mask")
		return;
	}
	
	conn->write ({ "MODE %1 +b %2\n", TARGET, parms[1] });
	
	for (IRCUser* user : conn->users)
		if (mask (user->userhost(), parms[1]))
			conn->write ({ "KICK %1 %2 :Banned", TARGET, user->nick() });
}

IRC_COMMAND (testfatal) {
	if (invoker->isAdmin() == false)
		return;

	parms.erase (0);
	fatal ("%1", join (parms, " "));
}

IRC_COMMAND (time) {
	conn->privmsg (TARGET, fmt ("Now is %1", CoDate (CoTime::now())));
}

CoStopwatch g_stopwatch;
IRC_COMMAND (sw_start) {
	g_stopwatch.start();
	REPLY ("Stopwatch started.")
}

IRC_COMMAND (sw_stop) {
	g_stopwatch.stop();
	REPLY ("Stopwatch stopped, lapsed time: %1", g_stopwatch.elapsed())
}

IRC_COMMAND (whois) {
	if (parms.size() < 2) {
		REPLY ("Who is... who?")
		return;
	}
	
	IRCUser* user = conn->findUser (parms[1]);
	
	if (user == null) {
		REPLY ("I don't know who that is.")
		return;
	}
	
	CoStringList channames;
	for (IRCChannel * chan : user->channels())
		channames << chan->name();
	
	REPLY ("I think %1 is %2. Real name: %3", user->nick(), user->userhost(), user->name())
	REPLY ("I see %1 on %2", user->nick(), join (channames, ", "))
}

IRC_COMMAND (ticket) {
	if (parms.size() < 2) {
		conn->privmsg (TARGET, "Which ticket?");
		return;
	}
	
	if (atoi (parms[1]) <= 0)
		return;
	
	str idstr = CoString::fromNumber (parms[1].toLong());
	str msg;
	bool success = ticketinfo (idstr, msg);
	
	if (success) {
		REPLY (msg)
		REPLY ("Link: %1view.php?id=%2", tracker_url, idstr)
	} else
		REPLY (msg)
}

IRC_COMMAND (fullticketinfo) {
	if (parms.size() < 2) {
		REPLY ("Which ticket?")
		return;
	}
	
	if (atoi (parms[1]) <= 0)
		return;
	
	str idstr = str::fromNumber (parms[1].toLong());
	str msg = fullticketinfo (idstr);
	
	for (const str& line : msg.split ("\n"))
		conn->privmsg (TARGET, line);
}

IRC_COMMAND (commands) {
	CoStringList cmdnames;
	
	for (const IRCCommandInfo& info : G_IRCCommands)
		cmdnames << info.namestring;
	
	cmdnames.sort();
	conn->privmsg (TARGET, fmt ("Available commands: %1", join (cmdnames, " ")));
}

IRC_COMMAND (msg) {
	if (!invoker->isAdmin() || parms.size() < 3)
		return;

	CoStringList msgparms = parms;
	msgparms.erase (0);
	msgparms.erase (0);
	str msg = join (msgparms, " ");
	conn->privmsg (parms[1], msg);
}

#if 0
IRC_COMMAND (md5) {
	int space = message.first (" ");
	str raw = (space != -1) ? message.substr (space + 1, -1) : "";
	conn->privmsg (TARGET, fmt ("Digest of '%1' is: %2", raw, md5 (raw)));
}
#endif // 0

IRC_COMMAND (query) {
	IPAddress addr;
	
	if (parms.size() < 2) {
		conn->privmsg (TARGET, "What shall I query?");
		return;
	}
	
	if (!CoIPAddress::fromString (parms[1], addr)) {
		REPLY ("Bad IP address!")
		return;
	}
	
	addServerQuery (addr, TARGET);
}

IRC_COMMAND (resolve) {
	if (parms.size() < 2)
		return;
	
	IPAddress addr;

	REPLY ("Resolving %1...", parms[1]);
	int r = CoIPAddress::resolve (parms[1], addr);

	if (r != 0)
		REPLY ("Failed to resolve: %1", CoString (gai_strerror (r)))
	else
		REPLY ("address: %1", addr)
}

IRC_COMMAND (masshighlight) {
	if (!channel)
		return;
	
	if (invoker->chanStatus (channel) < IRCChannel::Voiced) {
		REPLY ("Must be half-operator or higher to use this");
		return;
	}
	
	str nicklist;
	for (IRCChannel::Entry& e : *channel) {
		if (nicklist.length() > 0)
			nicklist += ", ";
		
		nicklist += e.userinfo()->nick();
		
		if (nicklist.length() > 300) {
			REPLY ("%1", nicklist);
			nicklist = "";
		}
	}
	
	if (nicklist.length() > 0)
		REPLY ("%1", nicklist);
}

IRC_COMMAND (mhl) {
	IRCCommand_masshighlight (conn, invoker, channel, message, parms);
}