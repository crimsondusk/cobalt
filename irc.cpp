#include <unistd.h>
#include "common.h"
#include "irc.h"
#include "config.h"
#include "commands.h"
#include "array.h"
#include "utility.h"

#define MAX_BUF 2048

IRCConnection::IRCConnection (const char* node, unsigned int port) {
	printf ("Attempting connection to %s:%u\n", node, port);
	
	authed = false;
	connected = false;
	namesdone = false;
	
	addrinfo_t hints;
	addrinfo_t* result;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	
	str sport;
	sport.appendformat ("%u", port);
	
	int r = getaddrinfo (node, (char*)sport, &hints, &result);
	if (r) {
		fprintf (stderr, "Name lookup error: %s\n", gai_strerror (r));
		exit (EXIT_FAILURE);
	}
	
	addrinfo_t* i;
	for (i = result; i; i = i->ai_next) {
		sock = socket (i->ai_family, i->ai_socktype, i->ai_protocol);
		if (sock == -1)
			continue;
		
		if (connect (sock, i->ai_addr, i->ai_addrlen) != -1) {
			printf ("Connection successful.\n");
			break;
		}
		
		close (sock);
	}
	
	if (!i) {
		fprintf (stderr, "Connection failed.\n");
		exit (EXIT_FAILURE);
	}
	
	char buf[MAX_BUF];
	
	ssize_t readsize;
	
	while ((readsize = read (sock, buf, sizeof buf)) > 0) {
		if (readsize < MAX_BUF)
			buf[readsize] = '\0';
		
		// Tokenize what we got into lines
		for (char* i = buf; i < buf + readsize; i++) {
			if (*i == '\n') {
				parseToken ();
				token = "";
			} else {
				token += *i;
			}
		}
	}
}

ssize_t IRCConnection::writef (const char* fmt, ...) {
	PERFORM_FORMAT (fmt, buf);
	buf[strlen (buf)] = '\0';
	ssize_t r = write (sock, buf, strlen (buf));
	printf ("<- %s", buf);
	
	delete[] buf;
	return r;
}

void IRCConnection::parseToken () {
	printf ("-> %s\n", (char*)token);
	
	// If we haven't authed yet, do so.
	if (!authed) {
		writef ("USER %s 0 * :%s\n", getConfig (Username).chars(), getConfig (Realname).chars());
		writef ("NICK %s\n", getConfig (Nickname).chars());
		authed = true;
	}
	
	// Remove crap off the message.
	token.strip ({17, '\r'});
	
	// Deliminate it
	array<str> msg = token / " ";
	
	// If the server is pinging us, reply with a pong.
	if (+msg[0] == "PING") {
		writef ("PONG %s\n", (char*)msg[1]);
		return;
	}
	
	// =========================================================================
	// NUMERIC CODES
	switch (atoi (msg[1])) {
	case RPL_WELCOME:
		// Server accepted us, tell it that we're joining
		writef ("JOIN %s\n", getConfig (Channel).chars());
		writef ("MODE %s %s\n", getConfig (Nickname).chars(), getConfig (UserFlags).chars());
		
		connected = true;
		break;
	case ERR_NEEDMOREPARAMS:
		if (!connected) {
			// Well shit.
			fprintf (stderr, "Server rejected the user credentials.\n");
			exit (1);
		}
		
		break;
	
	// =========================================================================
	// /NAMES response
	case RPL_NAMEREPLY:
		/*
		if (namesdone)
			userlist.clear ();
		*/
		
		if (msg[4] != getConfig (Channel)) {
			printf ("got /NAMES of %s, my channel is %s!\n", (char*)msg[3], (char*)getConfig (Channel));
			return;
		}
		
		for (uint i = 5; i < msg.size(); i++) {
			str nick = msg[i];
			
			if (!nick.len())
				continue;
			
			// The first entry is prefixed with a `:`
			if (i == 5)
				nick -= -1;
			
			// Determine status
			userstatus_e status = normal;
			if (nick[0] == getConfig (OpSymbol)[0])
				status = op;
			else if (nick[0] == getConfig (HalfOpSymbol)[0])
				status = halfop;
			else if (nick[0] == getConfig (VoiceSymbol)[0])
				status = voice;
			
			// Remove the status symbol from the name now that
			// we have identified it.
			if (status != normal)
				nick -= -1;
			
			// If we already have him listed, just update his status and get out.
			// /NAMES won't give us anything else.
			if (IRCUser* meta = findIRCUser (nick)) {
				meta->status = status;
				continue;
			}
			
			IRCUser info;
			info.nick = nick;
			info.host = info.user = "";
			info.status = status;
			userlist << info;
			
			printf ("User list: added %s: (%d)\n",
				info.nick.chars(), (int)(info.status));
		}
		
		namesdone = false;
		break;
	case RPL_ENDOFNAMES:
		// Request information about this channel's users
		writef ("WHO %s\n", (char*)getConfig (Channel));
		
		namesdone = true;
		break;
	
	// =========================================================================
	// /WHO response
	case RPL_WHOREPLY: {
		// We recieve information about a user.
		str user = msg[4];
		str host = msg[5];
		str server = msg[6];
		str nick = msg[7];
		str flags = msg[8];
		
		// Get the meta object
		IRCUser* meta = FetchUserMeta (nick);
		
		printf ("Setting meta for %s\n", (char*)meta->nick);
		
		meta->user = user;
		meta->host = host;
		meta->server = server;
		meta->status = normal;
		
		printf ("%s is %s!%s@%s\n", (char*)meta->nick, (char*)meta->nick,
			(char*)meta->user, (char*)meta->host);
		
		// Scan through the flags field for stuff
		for (uint i = 0; i < flags.len(); i++) {
			switch (flags[i]) {
			case 'H': // Here
				printf ("%s is here\n", (char*)meta->nick);
				meta->flags &= ~UF_Away;
				break;
			case 'G': // Gone
				printf ("%s is afk\n", (char*)meta->nick);
				meta->flags |= UF_Away;
				break;
			case '+': // Voiced user
				printf ("%s is voiced\n", (char*)meta->nick);
				meta->status = voice;
				break;
			case '%': // Half-operator
				printf ("%s is halfop\n", (char*)meta->nick);
				meta->status = halfop;
				break;
			case '@': // Operator
				printf ("%s is op\n", (char*)meta->nick);
				meta->status = op;
				break;
			case '*': // IRC operator
				printf ("%s is irc op\n", (char*)meta->nick);
				meta->flags |= UF_IRCOp;
				break;
			}
		}
		
		break;
	}
	
	// =========================================================================
	// NON-NUMERIC CODES
	case 0: {
		str codestring = +msg[1];
		
		str usermask = msg[0].substr (1, msg[0].len());
		str nick, user, host;
		int excl = usermask.first ("!"),
			atsign = usermask.first ("@");
		
		if (~excl * ~atsign) {
			nick = usermask.substr (0, excl);
			user = usermask.substr (excl + 1, atsign);
			host = usermask.substr (atsign + 1, -1);
		}
		
		if (codestring == "JOIN") {
			// Try find meta for this user
			IRCUser* usermeta = findIRCUser (nick);
			if (!usermeta) {
				// Not in the userlist, add him there.
				IRCUser info;
				info.nick = nick;
				info.user = user;
				info.host = host;
				info.status = normal;
				usermeta = &(userlist << info);
				
				printf ("Added %s to userlist\n", (char*)nick);
			} else {
				// Update their meta. Nick is unneeded since the user is found by
				// it anyway.
				usermeta->user = user;
				usermeta->host = host;
				
				// Users who join have normal user status by default.
				usermeta->status = normal;
			}
			
			if ((char*)nick == getConfig(Nickname)) {
				// It was us who joined! Mark our meta.
				me = usermeta;
			} else {
				// Was not us who joined, request additional information about him
				writef ("WHO %s\n", (char*)nick);
			}
		} else if (codestring == "PRIVMSG") {
			str target = msg[2];
			
			str message;
			for (uint i = 3; i < msg.size(); i++) {
				message += msg[i];
				if (i < msg.size() - 1)
					message += " ";
			}
			
			// Rid the initial :
			if (message[0] == ':')
				message -= -1;
			
			array<str> msgargs = message / " ";
			
			printf ("%s said to %s: `%s`\n", (char*)nick, (char*)target, (char*)message);
			IRCUser* ircuser = findIRCUser (nick);
			
			// ========================
			// CTCP support
			if (message[0] == 1) {
				str ctcpcode = +(msgargs[0].substr (1, -1));
				if (ctcpcode[ctcpcode.len() - 1] == 1)
					ctcpcode -= 1;
				printf ("ctcp code: %s\n", (char*)ctcpcode);
				
				// Let's assume that the \001 is the last character in the message,
				// and that there's no \001's before the actual CTCP data.
				// Unless the IRCD goes nuts this should cause no problems.
				int ctcpargidx = message.first (" ", message.first ("\1")) + 1;
				str ctcpargs = message.substr (ctcpargidx, message.last ("\1"));
				printf ("ctcp args: %s\n", (char*)ctcpargs);
				ctcpargs.dump();
				
				if (ctcpcode == "VERSION") {
					writef ("NOTICE %s :\001VERSION %s %d.%d\001\n",
						(char*)nick, APPNAME, VERSION_MAJOR, VERSION_MINOR);
				} else if (ctcpcode == "PING") {
					writef ("NOTICE %s :\001PING %s\001\n", (char*)nick, (char*)ctcpargs);
				} else if (ctcpcode == "TIME") {
					writef ("NOTICE %s :\001TIME %s\001\n", (char*)nick, GetDate().chars());
				}
				return;
			}
			
			if (message[0] == getConfig (IRCCommandPrefix)[0]) {
				str cmdname = msgargs[0].substr (1, -1);
				
				cmdname.dump ();
				
				// If this was called by someone who we don't have in our books,
				// we know nothing about them and they may not call any commands.
				// Though, anyone who's in the channel should be in the books..
				if (ircuser == NULL) {
					writef ("PRIVMSG %s :I don't know who you are, %s.\n",
						target.chars(), nick.chars());
					return;
				}
				
				// This was a command, try parse it as such
				for (uint i = 0; i < g_CommandMeta.size (); i++) {
					if (-cmdname == g_CommandMeta[i].namestring) {
						// Found the command, call it and pass it the arguments
						sIRCCommandMeta info;
						info.channel = target;
						info.user = ircuser;
						info.message = message;
						(*g_CommandMeta[i].func) (this, info, msgargs.clone());
						return;
					}
				}
				
				writef ("PRIVMSG %s :Unknown command `%c%s`!\n",
					target.chars(), getConfig (IRCCommandPrefix)[0], cmdname.chars());
				return;
			}
		}
		
		break;
	}
	}
}

IRCUser* IRCConnection::findIRCUser (str nick) {
	for (uint i = 0; i < userlist.size(); i++)
		if (+userlist[i].nick == +nick)
			return &userlist[i];
	return NULL;
}

IRCUser* IRCConnection::FetchUserMeta (str nick) {
	IRCUser* metaptr;
	if ((metaptr = findIRCUser (nick)) != NULL)
		return metaptr;
	
	// Didn't find him. Create an empty meta and return it.
	IRCUser meta;
	meta.nick = nick;
	return &(userlist << meta);
}