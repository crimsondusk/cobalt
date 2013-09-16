#include <netdb.h>
#include "libcobaltcore/time.h"
#include "libcobaltcore/ip.h"
#include <libcobaltcore/xml.h>
#include "../main.h"
#include "../mantisbt.h"
#include "../zanserver.h"
#include "irc.h"
#include "irc_channel.h"
#include "irc_user.h"
#include "irc_connection.h"

#define REPLY(...) conn->privmsg (TARGET, fmt (__VA_ARGS__));

EXTERN_CONFIG (String, tracker_url)
EXTERN_CONFIG (String, irc_commandprefix)

CoList<IRCCommandInfo> g_IRCCommands;
static CoXMLDocument*  G_FactsXML = null;

IRCCommandAdder::IRCCommandAdder (const char* namestring, IRCCommandType func) {
	const IRCCommandInfo info = {namestring, func};
	g_IRCCommands << info;
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
	conn->write (fmt ("%1\n", raw));
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
	
	conn->write (fmt ("MODE %1 +b %2\n", TARGET, parms[1]));
	
	for (IRCUser* user : conn->users)
		if (mask (user->userhost(), parms[1]))
			conn->write (fmt ("KICK %1 %2 :Banned", TARGET, user->nick()));
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
	
	for (const IRCCommandInfo& info : g_IRCCommands)
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
	CoIPAddress addr;
	
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
	
	CoIPAddress addr;

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
	for (const IRCChannel::Entry& e : channel->userlist()) {
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

IRC_COMMAND (add_autojoin) {
	if (!invoker->isAdmin() || parms.size() < 2)
		return;
	
	CoXMLNode* parent = CoConfig::xml()->navigateTo ({"irc", "channels"}, true);
	
	// Check if we already have this in our auto-joins
	bool isNew = true;
	CoXMLNode* chanNode = null;
	for (CoXMLNode* subnode : parent->nodes()) {
		if (subnode->name() == "channel" && subnode->contents() == parms[1]) {
			chanNode = subnode;
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
	
	if (mode_k != null) {
		chanNode->setAttribute ("password", mode_k->arg);
		passwordBlurb = fmt (" (password: %1)", mode_k->arg);
	}
	
	REPLY ("%1 auto-join channel %2%3", (isNew ? "Added" : "Updated"),  parms[1], passwordBlurb);
	saveConfig();
}

IRC_COMMAND (rehash) {
	if (!invoker->isAdmin())
		return;
	
	if (!CoConfig::load (configFileName()))
		REPLY ("Error rehashing: %1", CoXMLDocument::parseError())
	else
		REPLY ("Rehash successful")
}

IRC_COMMAND (writecfg) {
	if (!invoker->isAdmin())
		return;
	
	if (!CoConfig::save (configFileName()))
		REPLY ("Error saving: %1", strerror (errno))
	else
		REPLY ("Saved successfully")
}

static bool initFactsXML() {
	if (G_FactsXML == null) {
		G_FactsXML = CoXMLDocument::load ("facts.xml");
		
		if (errno)
			G_FactsXML = CoXMLDocument::newDocumentWithRoot ("facts");
	}
	
	return (G_FactsXML != null);
}

static CoXMLNode* getFactoid (CoStringRef subject) {
	assert (G_FactsXML != null);
	return G_FactsXML->root()->getOneNodeByAttribute ("subject", subject);
}

IRC_COMMAND (learn) {
	if (parms.size() < 3) {
		REPLY ("I should learn what?");
		REPLY ("Usage: %1 <subject> <:text>", parms[0]);
		return;
	}
	
	CoString subject = parms[1];
	CoString text = message.substr (message.posof (2) + 1);
	
	if (subject.length() > 20) {
		REPLY ("That subject is too long.")
		return;
	}
	
	for (char c : subject) {
		if ((c < 'a' || c > 'z') &&
			(c < 'A' || c > 'Z') &&
			(c < '0' || c > '9') &&
			c != '_')
		{
			REPLY ("Subject must only contain alpha-numeric characters or underscores")
			return;
		}
	}
	
	if (!initFactsXML())
		REPLY ("Couldn't open facts.xml: %1", CoXMLDocument::parseError());
	
	// If there already is a factoid by this name, delete it now
	CoXMLNode* node;
	while ((node = G_FactsXML->root()->findSubNode (subject)) != null)
		delete node;
	
	node = new CoXMLNode ("fact", G_FactsXML->root());
	node->setAttribute ("subject", subject);
	node->addSubNode ("author", invoker->userhost());
	node->addSubNode ("time", CoString::fromNumber (CoTime::now().seconds()));
	CoXMLNode* textNode = node->addSubNode ("text", text);
	textNode->setCDATA (true);
	
	if (!G_FactsXML->save ("facts.xml"))
		REPLY ("WARNING: Failed to save facts.xml with the new data: %1\n", strerror (errno));
	
	REPLY ("I now know about '%1'", subject);
}

IRC_COMMAND (about) {
	if (parms.size() < 2) {
		REPLY ("Usage: %1 <subject>", parms[0]);
		return;
	}
	
	if (!initFactsXML())
		REPLY ("Couldn't open facts.xml: %1", CoXMLDocument::parseError());
	
	CoString subject = parms[1];
	CoXMLNode* node = getFactoid (subject);
	CoXMLNode* textNode = node ? node->findSubNode ("text") : null;
	
	if (!textNode) {
		REPLY ("I don't know anything about %1. You can teach me that with %2teach.",
			subject, irc_commandprefix);
		return;
	}
	
	REPLY ("%1 is %2", subject, textNode->contents());
}

IRC_COMMAND (factoid_meta) {
	if (parms.size() < 2) {
		REPLY ("Usage: %1 <subject>", parms[0]);
		return;
	}
	
	if (!initFactsXML())
		REPLY ("Couldn't open facts.xml: %1", CoXMLDocument::parseError());
	
	CoString subject = parms[1];
	CoXMLNode* node = getFactoid (subject);
	
	if (!node) {
		REPLY ("No such factoid");
		return;
	}
	
	CoXMLNode* timeNode = node->findSubNode ("time"),
		*authorNode = node->findSubNode ("author");
	
	if (!timeNode || !authorNode) {
		REPLY ("Bad factoid: no time and/or author nodes");
		return;
	}
	
	CoTime ts = CoTime (timeNode->contents().toLong());
	REPLY ("Factoid '%1' was set by %2 on %3", subject, authorNode->contents(), CoDate (ts));
}

IRC_COMMAND (factoid_del) {
	if (parms.size() < 2 || !invoker->isAdmin())
		return;
	
	initFactsXML();
	if (!G_FactsXML)
		return;
	
	CoString subject = parms[1];
	CoXMLNode* node = getFactoid (subject);
	if (node) {
		delete node;
		G_FactsXML->save ("facts.xml");
		REPLY ("Factoid '%1' deleted", subject);
	} else
		REPLY ("No such factoid exists")
}