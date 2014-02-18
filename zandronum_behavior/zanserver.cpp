#include "zanserver.h"
#include "libcobaltcore/bytestream.h"
#include "libcobaltcore/misc.h"
#include "irc/irc.h"
#include "irc/connection.h"

#define BEGIN_GAMES_ENUM     const CoStringList gamestrings ({
#define GAME(N)                  #N,
#define END_GAMES_ENUM       });

#include "gameenums.h"
#include "3rdparty/huffman.h"

extern IRCConnection* g_IRCConnection;
static List<LauncherRequest*> g_serverRequests;

// =============================================================================
// -----------------------------------------------------------------------------
LauncherRequest::LauncherRequest() {}

uchar g_huffmanBuffer[131072];

void LauncherRequest::query()
{
	Bytestream s;
	s.writeLong (ZandronumProtocol::LauncherServerChallenge);
	s.writeLong (Name | MapName | MaxClients | NumPlayers | GameType | ForcePassword | IWAD | PWADs);
	s.writeLong (1234);

	int len;
	HUFFMAN_Encode (const_cast<uchar*> (reinterpret_cast<const uchar*> (s.data())),
					g_huffmanBuffer, s.length(), &len);
	Bytestream encoded (reinterpret_cast<char*> (g_huffmanBuffer), len);

	launch (encoded, address());
	setNextRequest (CoTime::now() + 1);
}

// =============================================================================
// -----------------------------------------------------------------------------
void LauncherRequest::update()
{
	CoTime now = CoTime::now();

	if (now > deadline())
	{
		mInfo.state = TimedOut;
		setDone (true);
		return;
	}

	if (now > nextRequest())
		query();

	tick();
}

// =============================================================================
// -----------------------------------------------------------------------------
void LauncherRequest::incoming (const Bytestream& encoded, CoIPAddress addr)
{
	int		len;
	int32	header,
			flags;
	str		sval;
	int32	sink;

	HUFFMAN_Decode (const_cast<uchar*> (reinterpret_cast<const uchar*> (encoded.data())),
					g_huffmanBuffer, encoded.length(), &len);

	Bytestream s (reinterpret_cast<char*> (g_huffmanBuffer), len);
	mInfo.maxclients = mInfo.players = mInfo.gametype = 0;
	setDone (true);

	if (!s.readLong (header) || !s.readLong (sink))
		return;

	switch (header)
	{
		case ZandronumProtocol::ServerQueryData:
		{
			if (!s.readString (mInfo.version) || !s.readLong (flags))
				return;

			if (flags & Name)
				s.readString (mInfo.name);

			if (flags & MapName)
				s.readString (mInfo.mapname);

			if (flags & MaxClients)
				s.readByte (mInfo.maxclients);

			if (flags & PWADs)
			{
				uint8 numwads;
				s.readByte (numwads);

				while (numwads--)
				{
					str wad;
					s.readString (wad);
					mInfo.pwads << wad;
				}
			}

			if (flags & GameType)
			{
				s.readByte (mInfo.gametype);
				s.readByte (mInfo.instagib);
				s.readByte (mInfo.buckshot);
			}

			if (flags & IWAD)
				s.readString (mInfo.iwad);

			if (flags & NumPlayers)
				s.readByte (mInfo.players);

			if (mInfo.gametype >= NumGames)
				mInfo.gametype = Cooperative;

			mInfo.state = Known;
		} break;

		case ZandronumProtocol::ServerQueryIgnoring:
		{
			mInfo.state = Throttled;
		} break;

		case ZandronumProtocol::ServerQueryBanned:
		{
			mInfo.state = Banned;
		} break;
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
void tickServerRequests()
{
	for (LauncherRequest * req : g_serverRequests)
	{
		req->update();

		if (req->done())
		{
			String target = req->payloadTarget();
			LauncherRequest::InfoStruct& info = req->mInfo;
			StringList replies;

			switch (req->mInfo.state)
			{
			case LauncherRequest::TimedOut:
			{
				replies << Format ("Query to %1 timed out.", req->address());
			} break;

			case LauncherRequest::Banned:
			{
				replies << Format ("I'm banned from %1!", req->address());
			} break;

			case LauncherRequest::Throttled:
			{
				replies << Format ("I'm throttled from %1, please try again in 10 seconds.", req->address());
			} break;

			case LauncherRequest::Known:
			{
				Print ("%1\n", info.gametype);
				replies << Format ("Version %1 server", info.version);
				replies << Format ("Name: %1", info.name);
				replies << Format ("Players: %1 / %2", (int) info.players, (int) info.maxclients);
				replies << Format ("Gamemode: %1, map: %2", gamestrings[info.gametype], info.mapname);
				replies << Format ("IWAD: %1, PWADs: %2", info.iwad, arrayRep (info.pwads));
			} break;

			case LauncherRequest::Unknown:
			{
				replies << Format ("Server %1 replied with garbage.", req->address());
			} break;
			}

			for (const str & reply : replies)
				g_IRCConnection->privmsg (target, reply);

			delete req;
			g_serverRequests.remove (req);
		}
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
void addServerQuery (CoIPAddress addr, str target)
{
	LauncherRequest* req = new LauncherRequest;
	req->setBlocking (false);
	req->init (0);
	req->setDone (false);
	req->setAddress (addr);
	req->setPayloadTarget (target);
	req->setNextRequest (CoTime::now());
	req->setDeadline (CoTime::now() + 5);
	g_serverRequests << req;

	g_IRCConnection->privmsg (target, Format ("Querying %1...", addr));
}
