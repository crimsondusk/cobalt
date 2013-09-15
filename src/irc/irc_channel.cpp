#include "irc_channel.h"
#include "irc_user.h"
#include "../utility.h"
#include <QStringList>

IRCChannel::IRCChannel (str name) :
	m_name (name),
	m_joinTime (QTime::currentTime()) {}

IRCChannel::Entry* IRCChannel::addUser (IRCUser* info) {
	info->addKnownChannel (this);
	
	Entry e (info, Normal);
	return &(m_userlist << e);
}

void IRCChannel::delUser (IRCUser* info) {
	info->delKnownChannel (this);
	
	for (Entry& e : m_userlist) {
		if (e.userinfo() == info) {
			m_userlist.remove (e);
			return;
		}
	}
}

IRCChannel::Entry* IRCChannel::findUser (str name) {
	for (Entry& e : m_userlist)
		if (e.userinfo()->nick() == name)
			return &e;
	
	return null;
}

IRCChannel::Entry* IRCChannel::findUser (IRCUser* info) {
	for (Entry& e : m_userlist)
		if (e.userinfo() == info)
			return &e;
	
	return null;
}

bool IRCChannel::Entry::operator== (const IRCChannel::Entry& other) const {
	return (userinfo() == other.userinfo()) && (status() == other.status());
}

IRCChannel::it IRCChannel::begin() {
	return m_userlist.begin();
}

IRCChannel::c_it IRCChannel::begin() const {
	return m_userlist.begin();
}

IRCChannel::it IRCChannel::end() {
	return m_userlist.end();
}

IRCChannel::c_it IRCChannel::end() const {
	return m_userlist.end();
}

long IRCChannel::statusof (IRCUser* info) {
	Entry* e = findUser (info);
	
	if (!e)
		return Normal;
	
	return e->status();
}

IRCChannel::Status IRCChannel::effectiveStatus (IRCUser* info) {
	Entry* e = findUser (info);
	
	if (!e)
		return Normal;
	
	long mode = e->status();
	return effectiveStatus (mode);
}

IRCChannel::Status IRCChannel::effectiveStatus (long mode) {
	if (mode & Owner)  return Owner;
	if (mode & Admin)  return Admin;
	if (mode & Op)     return Op;
	if (mode & HalfOp) return HalfOp;
	if (mode & Voiced) return Voiced;
	
	return Normal;
}

str IRCChannel::statusname (long mode) {
	return valueMap<const char*> ((int) effectiveStatus (mode), "User", 5,
		Owner,  "Owner",
		Admin,  "Admin",
		Op,     "Operator",
		HalfOp, "Half-operator",
		Voiced, "Voiced");
}

ulong IRCChannel::numUsers() const {
	return m_userlist.size();
}

const ChannelModeInfo g_ChannelModeInfo[] = {
#define CHANMODE(C, N, HASARG) \
	{ C, ChanMode_##N, #N, HASARG },
	CHANMODE ('A', AllInvite,      false)
	CHANMODE ('b', Ban,            true)
	CHANMODE ('B', BlockCaps,      false)
	CHANMODE ('c', BlockColors,    false)
	CHANMODE ('C', BlockCTCP,      false)
	CHANMODE ('D', DelayedJoin,    false)
	CHANMODE ('d', DelayedMessage, true)
	CHANMODE ('e', BanExempt,      true)
	CHANMODE ('f', FloodKick,      true)
	CHANMODE ('F', NickFlood,      false)
	CHANMODE ('g', WordCensor,     true)
	CHANMODE ('G', NetworkCensor,  false)
	CHANMODE ('h', ChanHalfOp,     true)
	CHANMODE ('H', History,        true)
	CHANMODE ('i', InviteOnly,     false)
	CHANMODE ('I', InviteExcept,   true)
	CHANMODE ('j', JoinFlood,      true)
	CHANMODE ('J', KickNoRejoin,   true)
	CHANMODE ('k', Locked,         true)
	CHANMODE ('K', NoKnock,        false)
	CHANMODE ('l', UserLimit,      true)
	CHANMODE ('L', Redirect,       true)
	CHANMODE ('m', Moderated,      false)
	CHANMODE ('M', RegisterSpeak,  false)
	CHANMODE ('n', NoExtMessages,  false)
	CHANMODE ('N', NoNickChanges,  false)
	CHANMODE ('o', ChanOp,         true)
	CHANMODE ('O', OpersOnly,      false)
	CHANMODE ('p', Private,        false)
	CHANMODE ('P', Permanent,      false)
	CHANMODE ('q', ChanOwner,      true)
	CHANMODE ('Q', NoKick,         false)
	CHANMODE ('R', RegisterJoin,   false)
	CHANMODE ('s', Secret,         false)
	CHANMODE ('S', StripColors,    false)
	CHANMODE ('t', TopicLock,      false)
	CHANMODE ('T', BlockNotice,    false)
	CHANMODE ('u', Auditorium,     false)
	CHANMODE ('v', UserVoice,      false)
	CHANMODE ('w', AutoOp,         true)
	CHANMODE ('y', IRCOpInChannel, false)
	CHANMODE ('Y', IRCOpOJoin,     false)
	CHANMODE ('X', GenericRestrict, true)
	CHANMODE ('z', Secure,         false)
	CHANMODE ('Z', NamedMode,      true)
};

long IRCChannel::getStatusFlag (char c) {
	return valueMap<long> (c, 0, 5,
		'q', Owner,
		'a', Admin,
		'o', Op,
		'h', HalfOp,
		'v', Voiced);
}

void IRCChannel::applyModeString (str text) {
	bool neg = false;
	QStringList args = text.split (" ", QString::SkipEmptyParts);
	uint argidx = 0;
	str modestring = args[0];
	args.erase (0);
	
	for (const QChar& c : modestring) {
		if (c == '+') {
			// +abc
			neg = false;
			continue;
		} elif (c == '-') {
			// -abc
			neg = true;
			continue;
		}
		
		// Handle status modes. Who on earth thought it was a good
		// idea to have them as channel modes?
		if (c == 'o' || c == 'v' || c == 'h' || c == 'a' || c == 'q') {
			// All of these require an argument
			str arg;
			
			if (!(args >> arg))
				continue;
			
			Entry* e = findUser (arg);
			
			if (!e)
				continue;
			
			long status = e->status();
			long flag = getStatusFlag (c);
			
			if (!neg)
				status |= flag;
			else
				status &= ~flag;
			
			e->setStatus (status);
			continue;
		}
		
		for (const ChannelModeInfo& it : g_ChannelModeInfo) {
			if (it.c != c)
				continue;
			
			if (neg == false) {
				// New mode
				Mode mode;
				mode.info = &it;
				
				if (it.hasArg)
					args >> mode.arg;
				
				m_modes << mode;
			} else {
				// Remove existing mode
				for (int j = 0; j < m_modes.size(); ++j) {
					Mode mode = m_modes[j];
					
					if (mode.info != &it)
						continue;
					
					m_modes.erase (j);
					
					if (!!mode.arg)
						argidx++;
				}
			}
			
			break;
		}
	}
}

str IRCChannel::modeString() const {
	str modestring;
	QList<str> args;
	
	for (const Mode& mode : m_modes) {
		modestring += mode.info->c;
		
		if (mode.info->hasArg)
			args << mode.arg;
	}
	
	args.push_front (modestring);
	return join (args, " ");
}

IRCChannel::Mode* IRCChannel::getMode (ChanMode modenum) {
	for (Mode& mode : m_modes) {
		if (mode.info->mode != modenum)
			continue;
		
		return &mode;
	}
	
	return null;
}
