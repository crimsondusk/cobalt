#ifndef COBALT_IRC_CHANNEL_H
#define COBALT_IRC_CHANNEL_H

#include <libcobaltcore/time.h>
#include "irc.h"

class IRCUser;
enum ChanMode
{	ChanMode_AllInvite,
	ChanMode_Ban,
	ChanMode_BlockCaps,
	ChanMode_BlockColors,
	ChanMode_BlockCTCP,
	ChanMode_DelayedJoin,
	ChanMode_BanExempt,
	ChanMode_FloodKick,
	ChanMode_NickFlood,
	ChanMode_WordCensor,
	ChanMode_NetworkCensor,
	ChanMode_InviteOnly,
	ChanMode_KickNoRejoin,
	ChanMode_Locked,
	ChanMode_NoKnock,
	ChanMode_UserLimit,
	ChanMode_Redirect,
	ChanMode_Moderated,
	ChanMode_RegisterSpeak,
	ChanMode_NoExtMessages,
	ChanMode_NoNickChanges,
	ChanMode_ChanOp,
	ChanMode_OpersOnly,
	ChanMode_Private,
	ChanMode_Permanent,
	ChanMode_ChanOwner,
	ChanMode_NoKick,
	ChanMode_RegisterJoin,
	ChanMode_Secret,
	ChanMode_StripColors,
	ChanMode_TopicLock,
	ChanMode_BlockNotice,
	ChanMode_Auditorium,
	ChanMode_UserVoice,
	ChanMode_IRCOpInChannel,
	ChanMode_Secure,
	ChanMode_ChanHalfOp,
	ChanMode_History,
	ChanMode_JoinFlood,
	ChanMode_InviteExcept,
	ChanMode_DelayedMessage,
	ChanMode_AutoOp,
	ChanMode_NamedMode,
	ChanMode_IRCOpOJoin,
	ChanMode_GenericRestrict,
};

struct ChannelModeInfo
{	const char c;
	const ChanMode mode;
	const str name;
	const bool hasArg;
};

class IRCChannel
{	public:
		// =========================================================================
		// -------------------------------------------------------------------------
		enum Status
		{	Normal = (0),
			Voiced = (1 << 0),
			HalfOp = (1 << 1),
			Op     = (1 << 2),
			Admin  = (1 << 3),
			Owner  = (1 << 4),
		};

		// =========================================================================
		// -------------------------------------------------------------------------
		struct Mode
		{	const ChannelModeInfo* info;
			str arg;
		};

		// =========================================================================
		// -------------------------------------------------------------------------
		class Entry
		{		PROPERTY (IRCUser*, userinfo, setUserinfo)
				PROPERTY (long, status, setStatus)

			public:
				Entry (IRCUser* user, Status stat) :
					m_userinfo (user),
					m_status (stat) {}

				bool operator== (const Entry& other) const;
		};

		// =========================================================================
		// -------------------------------------------------------------------------
		PROPERTY (str, name, setName)
		PROPERTY (str, topic, setTopic)
		READ_PROPERTY (CoTime, joinTime, setJoinTime)

	public:
		typedef ChannelModeInfo modeinfo;

		IRCChannel (str name);

		Entry*        addUser (IRCUser* info);
		void          applyModeString (CoStringRef text);
		void          delUser (IRCUser* info);
		Entry*        findUser (str name);
		Entry*        findUser (IRCUser* info);
		str           modeString() const;
		ulong         numUsers() const;
		long          statusof (IRCUser* info);
		Status        effectiveStatus (IRCUser* info);
		Mode*         getMode (ChanMode modenum);

		static Status effectiveStatus (long mode);
		static long   getStatusFlag (char c);
		static str    statusname (long mode);

		inline const CoList<Entry>& userlist() const
		{	return m_userlist;
		}

	private:
		CoList<Entry> m_userlist;
		CoList<Mode>  m_modes;
		CoList<str>   m_banlist,
			   m_whitelist,
			   m_invitelist;
};

#endif // COBALT_IRC_CHANNEL_H
