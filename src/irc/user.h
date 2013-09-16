#ifndef COBALT_IRC_USER_H
#define COBALT_IRC_USER_H

#include "irc.h"
#include "channel.h"
#include <libcobaltcore/flags.h>

class IRCUser {
public:
	enum Flag {
		Away     = (1 << 0),    // is /AWAY
		Admin    = (1 << 1),    // is a configured administrator of this bot
		IRCOp    = (1 << 2),    // is an IRC Op
	};
	
	typedef CoFlags<Flag> Flags;
	
	PROPERTY (str, nick, setNick)
	PROPERTY (str, user, setUser)
	PROPERTY (str, host, setHost)
	PROPERTY (str, name, setName)
	PROPERTY (str, server, setServer)
	PROPERTY (str, account, setAccount)
	PROPERTY (Flags, flags, setFlags)
	READ_PROPERTY (CoList<IRCChannel*>, channels, setChannels)
	
public:
	IRCUser() :
		m_flags (0) {}
	
	IRCChannel::Status chanStatus (IRCChannel* chan);
	void checkAdmin();
	str userhost() const;
	void addKnownChannel (IRCChannel* chan);
	void delKnownChannel (IRCChannel* chan);
	bool isAdmin() const;
	str asString() const;
	
	Flags operator| (Flags f) const;
	Flags operator& (Flags f) const;
	Flags operator^ (Flags f) const;
	IRCUser operator|= (Flags f);
	IRCUser operator&= (Flags f);
	IRCUser operator^= (Flags f);
};

#endif // COBALT_IRC_USER_H