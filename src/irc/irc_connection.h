#ifndef COBALT_IRC_CONNECTION_H
#define COBALT_IRC_CONNECTION_H

#include "irc.h"

class QTcpSocket;

class IRCConnection : public QObject {
	Q_OBJECT
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
	void write (const str& msg);
	
	bool authed;
	bool namesdone;
	QList<IRCUser*> users;

private:
	void namesResponse (const QStringList& tokens);
	void whoReply (str data, const QList<str>& tokens);
	void nonNumericResponse (const str& data, const QList<str>& tokens);
	IRCUser* newUser (str nick);
	
	QTcpSocket*    m_conn;
	QList<IRCChannel*> m_channels;
};

#endif // COBALT_IRC_CONNECTION_H
