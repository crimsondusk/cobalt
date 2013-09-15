#ifndef __TCP_H__
#define __TCP_H__

#include "../main.h"

#define BOLDCHAR        "\x02"
#define COLORCHAR       "\003"
#define NORMALCHAR      "\x0F"

class QStringList;
class IRCChannel;
class IRCConnection;
class IRCUser;

namespace IRC {
	enum Color {
		White,
		Black,
		Blue,
		Green,
		Red,
		DarkRed,
		Purple,
		Orange,
		Yellow,
		LightGreen,
		LightBlue,
		SkyBlue,
		Cyan,
		LightPurple,
		DarkGray,
		LightGray
	};
	
	str getColor (Color fg);
	str getColor (Color fg, Color bg);
}

// =============================================================================
// IRC command macros
#define IRC_COMMAND_PARMS \
	IRCConnection* conn, \
	IRCUser* invoker, \
	IRCChannel* channel, \
	str message, \
	QStringList parms

#define IRC_COMMAND(X) \
void IRCCommand_##X (IRC_COMMAND_PARMS); \
IRCCommandAdder CMDAdder_##X (#X, &IRCCommand_##X); \
void IRCCommand_##X (IRC_COMMAND_PARMS)

typedef void (*IRCCommandType) (IRC_COMMAND_PARMS);

struct IRCCommandInfo {
	const char* namestring;
	IRCCommandType func;
};

// List of all commands we have defined + pointers to them
extern QList<IRCCommandInfo> g_IRCCommands;

// This class adds an IRC command to g_IRCCommands
class IRCCommandAdder {
public:
	IRCCommandAdder (const char* namestring, IRCCommandType func);
};

// =============================================================================
// IRC server reply codes.. not a comprehensive list
enum ReplyCode {
	RPL_WELCOME          = 001,
	RPL_YOURHOST         = 002,
	RPL_CREATED          = 003,
	RPL_MYINFO           = 004,
	RPL_BOUNCE           = 005,
	RPL_AWAY             = 301,
	RPL_USERHOST         = 302,
	RPL_ISON             = 303,
	RPL_UNAWAY           = 305,
	RPL_NOWAWAY          = 306,
	RPL_WHOISUSER        = 311,
	RPL_WHOISSERVER      = 312,
	RPL_WHOISOPERATOR    = 313,
	RPL_WHOWASUSER       = 314,
	RPL_ENDOFWHO         = 315,
	RPL_WHOISIDLE        = 317,
	RPL_ENDOFWHOIS       = 318,
	RPL_WHOISCHANNELS    = 319,
	RPL_ENDOFWHOWAS      = 369,
	RPL_LISTSTART        = 321,
	RPL_LIST             = 322,
	RPL_LISTEND          = 323,
	RPL_CHANNELMODEIS    = 324,
	RPL_UNIQOPIS         = 325,
	RPL_WHOISACCOUNT     = 330,
	RPL_NOTOPIC          = 331,
	RPL_TOPIC            = 332,
	RPL_INVITING         = 341,
	RPL_SUMMONING        = 342,
	RPL_INVITELIST       = 346,
	RPL_ENDOFINVITELIST  = 347,
	RPL_EXCEPTLIST       = 348,
	RPL_ENDOFEXCEPTLIST  = 349,
	RPL_VERSION          = 351,
	RPL_WHOREPLY         = 352,
	RPL_NAMEREPLY        = 353,
	RPL_LINKS            = 364,
	RPL_ENDOFLINKS       = 365,
	RPL_ENDOFNAMES       = 366,
	RPL_BANLIST          = 367,
	RPL_ENDOFBANLIST     = 368,
	RPL_INFO             = 371,
	RPL_MOTD             = 372,
	RPL_ENDOFINFO        = 374,
	RPL_MOTDSTART        = 375,
	RPL_ENDOFMOTD        = 376,
	RPL_YOUREOPER        = 381,
	RPL_REHASHING        = 382,
	RPL_YOURESERVICE     = 383,
	RPL_TIME             = 391,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKNAMEINUSE    = 433,
	ERR_NEEDMOREPARAMS   = 461,
};

extern IRCConnection* conn;

#endif // __TCP_H__