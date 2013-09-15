#ifndef COBALT_IRC_CONNECTION_H
#define COBALT_IRC_CONNECTION_H

#include <libcobaltcore/tcp.h>
#include "irc.h"

class IRCConnection : public CoTCPSocket {
	PROPERTY (str, currentNickname, setCurrentNickname)
	READ_PROPERTY (bool, loggedIn, setLoggedIn)
	READ_PROPERTY (IRCUser*, me, setMe)
	
public:
	IRCConnection (str node, uint16 port);
	void privmsg (str target, str msg);
	void incoming (str data);
	IRCUser* findUser (str nick);
	IRCUser* getUser (str nick);
	void delUser (str nick);
	IRCChannel* findChannel (str name);
	IRCChannel* getChannel (str name);
	
	// TODO: make private, prefix with m_
	bool authed;
	bool namesdone;
	CoList<IRCUser*> users;

private:
	void namesResponse (CoStringListRef tokens);
	void whoReply (str data, const CoStringListRef tokens);
	void nonNumericResponse (const str& data, CoStringListRef tokens);
	IRCUser* newUser (str nick);
	
	CoList<IRCChannel*> m_channels;
};

#endif // COBALT_IRC_CONNECTION_H
