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
};

#define UF_IRCOp	(1 << 0)
#define UF_Away		(1 << 1)

class IRCUser {
public:
	str nick, user, host;
	userstatus_e status;
	str server;
	long flags;
	
	IRCUser () {
		status = normal;
		flags = 0;
	}
};

typedef struct addrinfo addrinfo_t;

class IRCConnection {
public:
	int sock;
	str token;
	bool authed;
	bool connected;
	bool namesdone;
	array<IRCUser> userlist;
	IRCUser* me;
	
	IRCConnection (const char* node, unsigned int port);
	ssize_t writef (const char* fmt, ...) __attribute__ ((format (printf,2,3)));
	void parseToken ();
	IRCUser* findIRCUser (str nick);
	IRCUser* FetchUserMeta (str nick);
};

#endif // __TCP_H__