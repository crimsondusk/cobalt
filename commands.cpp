#include "common.h"
#include "commands.h"
#include "irc.h"

array<FunctionMeta> g_CommandMeta;

IRCCommandAdder::IRCCommandAdder (const char* namestring, COMMANDTYPE (func)) {
	const FunctionMeta info = {namestring, func};
	g_CommandMeta << info;
}

// =============================================================================
command (sayhi) {
	if (parms.size() < 2)
		conn->privmsgf (meta.channel, "Hi!");
	else
		conn->privmsgf (meta.channel, "Hi, %s!", (char*)parms[1]);
}

// =============================================================================
command (quit) {
	if (meta.user->status() < op && !(meta.user->flags & UF_Admin)) {
		conn->privmsgf (meta.channel, "%s: Who are you to tell me to get out?",
			(char*)meta.user->nick);
		return;
	}
	
	conn->privmsgf (meta.channel, "Bye.");
	conn->writef ("QUIT :\n");
}

// =============================================================================
command (userlist) {
	for (uint i = 0; i < conn->userlist.size(); i++) {
		IRCUser* usermeta = &conn->userlist[i];
		conn->writef ("PRIVMSG %s :%u. %s (%s@%s), status: %s%s%s\n",
			(char*)meta.channel, i, (char*)usermeta->nick,
			(char*)usermeta->user, (char*)usermeta->host,
			(usermeta->flags & UF_Operator) ? "Operator" : 
			(usermeta->flags & UF_HalfOperator) ? "Half-operator" :
			(usermeta->flags & UF_Voiced) ? "Voiced user" : "Normal user",
			(usermeta->flags & UF_IRCOp) ? ", IRC Operator" : "",
			(usermeta->flags & UF_Away) ? ", AFK" : "");
	}
	
	conn->writef ("PRIVMSG %s :====================================================\n",
		(char*)meta.channel);
	conn->writef ("PRIVMSG %s :%u users listed\n", (char*)meta.channel, conn->userlist.size());
}

// =============================================================================
command (raw) {
	if (!(meta.user->flags & UF_Admin)) {
		conn->writef ("PRIVMSG %s :%s: Who are you to tell me what to do?\n",
			(char*)meta.channel, (char*)meta.user->nick);
		return;
	}
	
	int space = meta.message.first (" ");
	str raw = meta.message.substr (space + 1, -1);
	conn->writef ("%s\n", raw.chars());
}

// =============================================================================
command (trymask) {
	if (parms.size() < 2)
		conn->writef ("PRIVMSG %s :I need a mask to check against!\n",
			(char*)meta.channel);
	
	for (uint i = 0; i < conn->userlist.size(); i++) {
		IRCUser* usermeta = &conn->userlist[i];
		str userhost;
		userhost.format ("%s!%s@%s",
			(char*)usermeta->nick,
			(char*)usermeta->user,
			(char*)usermeta->host);
		
		if (Mask (userhost, parms[1])) {
			conn->writef ("PRIVMSG %s :%s matches the mask\n",
				(char*)meta.channel, (char*)usermeta->nick);
		}
	}
}