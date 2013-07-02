#ifndef COBALT_IRC_USER_H
#define COBALT_IRC_USER_H

#include "irc.h"
#include "irc_channel.h"

class IRCUser
{
	PROPERTY( str, nick, setNick )
	PROPERTY( str, user, setUser )
	PROPERTY( str, host, setHost )
	PROPERTY( str, name, setName )
	PROPERTY( str, server, setServer )
	PROPERTY( str, account, setAccount )
	PROPERTY( long, flags, setFlags )
	READ_PROPERTY( vector<IRCChannel*>, channels, setChannels )
	
public:
	enum Flags
	{
		Away     = ( 1 << 0 ), // is /AWAY
		Admin    = ( 1 << 1 ), // is a configured administrator of this bot
		IRCOp    = ( 1 << 2 ), // is an IRC Op
	};
	
	IRCUser() : m_flags( 0 ) {}
	
	IRCChannel::Status chanStatus( IRCChannel* chan );
	void checkAdmin();
	str userhost() const;
	void addKnownChannel( IRCChannel* chan );
	void delKnownChannel( IRCChannel* chan );
	bool isAdmin() const;
	
	long operator|( long f ) const;
	long operator&( long f ) const;
	long operator^( long f ) const;
	IRCUser operator|=( long f );
	IRCUser operator&=( long f );
	IRCUser operator^=( long f );
	operator variant();
};

#endif // COBALT_IRC_USER_H