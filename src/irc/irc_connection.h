#ifndef COBALT_IRC_CONNECTION_H
#define COBALT_IRC_CONNECTION_H

#include "irc.h"

class IRCConnection : public AbstractTCPSocket
{
	PROPERTY( str, currentNickname, setCurrentNickname )
	READ_PROPERTY( bool, loggedIn, setLoggedIn )
	READ_PROPERTY( IRCUser*, me, setMe )
	
public:
	IRCConnection( str node, uint16 port );
	void privmsg( str target, str msg );
	void incoming( str data );
	IRCUser* findUser( str nick );
	IRCUser* getUser( str nick );
	void delUser( str nick );
	IRCChannel* findChannel( str name );
	IRCChannel* getChannel( str name );
	
	bool authed;
	bool namesdone;
	vector<IRCUser*> users;

private:
	void namesResponse( const vector<str>& tokens );
	void whoReply( str data, const vector< str >& tokens );
	void nonNumericResponse( const str& data, const vector<str>& tokens );
	IRCUser* newUser( str nick );
	
	vector<IRCChannel*> m_channels;
};

#endif // COBALT_IRC_CONNECTION_H
