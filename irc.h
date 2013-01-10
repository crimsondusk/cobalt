#ifndef __TCP_H__
#define __TCP_H__

#include <netdb.h>
#include "common.h"

#define NICK "nullbot"
#define USER "test"
#define NAME "test"
#define CHANNEL "#null-object"

#define BOLDCHAR		"\x02"
#define COLORCHAR		"\x03"

enum {
	RPL_WELCOME			= 1,
	RPL_YOURHOST		= 2,
	RPL_CREATED			= 3,
	RPL_MYINFO			= 4,
	RPL_BOUNCE			= 5,
	RPL_AWAY			= 301,
	RPL_USERHOST		= 302,
	RPL_ISON			= 303,
	RPL_UNAWAY			= 305,
	RPL_NOWAWAY			= 306,
	RPL_WHOISUSER		= 311,
	RPL_WHOISSERVER		= 312,
	RPL_WHOISOPERATOR	= 313,
	RPL_WHOWASUSER		= 314,
	RPL_ENDOFWHO		= 315,
	RPL_WHOISIDLE		= 317,
	RPL_ENDOFWHOIS		= 318,
	RPL_WHOISCHANNELS	= 319,
	RPL_ENDOFWHOWAS		= 369,
	RPL_LISTSTART		= 321,
	RPL_LIST			= 322,
	RPL_LISTEND			= 323,
	RPL_CHANNELMODEIS	= 324,
	RPL_UNIQOPIS		= 325,
	RPL_NOTOPIC			= 331,
	RPL_TOPIC			= 332,
	RPL_INVITING		= 341,
	RPL_SUMMONING		= 342,
	RPL_INVITELIST		= 346,
	RPL_ENDOFINVITELIST	= 347,
	RPL_EXCEPTLIST		= 348,
	RPL_ENDOFEXCEPTLIST	= 349,
	RPL_VERSION			= 351,
	RPL_WHOREPLY		= 352,
	RPL_NAMEREPLY		= 353,
	RPL_LINKS			= 364,
	RPL_ENDOFLINKS		= 365,
	RPL_ENDOFNAMES		= 366,
	RPL_BANLIST			= 367,
	RPL_ENDOFBANLIST	= 368,
	RPL_INFO			= 371,
	RPL_MOTD			= 372,
	RPL_ENDOFINFO		= 374,
	RPL_MOTDSTART		= 375,
	RPL_ENDOFMOTD		= 376,
	RPL_YOUREOPER		= 381,
	RPL_REHASHING		= 382,
	RPL_YOURESERVICE	= 383,
	RPL_TIME			= 391,
	ERR_NEEDMOREPARAMS	= 461,
};

enum userstatus_e {
	normal = 0,
	voice,
	halfop,
	op,
	ircop,
};

#define UF_Operator		(1 << 0) // is an operator on the channel
#define UF_HalfOperator	(1 << 1) // is a half-operator
#define UF_Voiced		(1 << 2) // is voiced on the channel
#define UF_IRCOp		(1 << 3) // is an IRC operator
#define UF_Away			(1 << 4) // is /AWAY
#define UF_Admin		(1 << 5) // is a configured administrator of this bot

class IRCUser {
public:
	str nick, user, host;
	str server;
	long flags;
	bool admin;
	
	IRCUser () {
		flags = 0;
	}
	
	userstatus_e status () {
		if (flags & UF_IRCOp)
			return ircop;
		if (flags & UF_Operator)
			return op;
		if (flags & UF_HalfOperator)
			return halfop;
		if (flags & UF_Voiced)
			return voice;
		return normal;
	}
	
	void RemoveChannelStatus () {
		flags &= ~(UF_Operator | UF_HalfOperator | UF_Voiced);
	}
	
	void CheckAdmin ();
};

typedef struct addrinfo addrinfo_t;

class IRCConnection {
public:
	int sock;
	str token;
	bool authed;
	bool connected;
	bool namesdone;
	
	// List of known users
	array<IRCUser> userlist;
	
	// Our user meta
	IRCUser* me;
	
	IRCConnection (const char* node, unsigned int port);
	ssize_t writef (const char* fmt, ...) __attribute__ ((format (printf,2,3)));
	void privmsgf (const char* target, const char* fmt, ...) __attribute__ ((format (printf,3,4)));
	void parseToken ();
	IRCUser* FindUserMeta (str nick);
	IRCUser* FetchUserMeta (str nick);
	void RemoveUser (str nick);
};

#endif // __TCP_H__
