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
static CoList<LauncherRequest*> g_serverRequests;

// =============================================================================
// -----------------------------------------------------------------------------
LauncherRequest::LauncherRequest() {}

uchar g_huffmanBuffer[131072];

void LauncherRequest::query() {
	CoBytestream s;
	s.writeLong (ZandronumProtocol::LauncherServerChallenge);
	s.writeLong (Name | MapName | MaxClients | NumPlayers | GameType | ForcePassword | IWAD | PWADs);
	s.writeLong (1234);
	
	int len;
	HUFFMAN_Encode (const_cast<uchar*> (reinterpret_cast<const uchar*> (s.data())),
		g_huffmanBuffer, s.length(), &len);
	CoBytestream encoded (reinterpret_cast<char*> (g_huffmanBuffer), len);
	
	launch (encoded, address());
	setNextRequest (CoTime::now() + 1);
}

// =============================================================================
// -----------------------------------------------------------------------------
void LauncherRequest::update() {
	CoTime now = CoTime::now();

	if (now > deadline()) {
		m_info.state = TimedOut;
		setDone (true);
		return;
	}

	if (now > nextRequest())
		query();

	tick();
}

// =============================================================================
// -----------------------------------------------------------------------------
void LauncherRequest::incoming (const CoBytestream& encoded, CoIPAddress addr) {
	int len;
	int32 header, flags;
	str sval;
	int32 sink;
	
	HUFFMAN_Decode (const_cast<uchar*> (reinterpret_cast<const uchar*> (encoded.data())),
		g_huffmanBuffer, encoded.length(), &len);
	
	CoBytestream s (reinterpret_cast<char*> (g_huffmanBuffer), len);
	m_info.maxclients = m_info.players = m_info.gametype = 0;
	setDone (true);
	
	if (!s.readLong (header) || !s.readLong (sink))
		return;
	
	switch (header) {
	case ZandronumProtocol::ServerQueryData:
		if (!s.readString (m_info.version) || !s.readLong (flags))
			return;
		
		if (flags & Name)
			s.readString (m_info.name);
		
		if (flags & MapName)
			s.readString (m_info.mapname);
		
		if (flags & MaxClients)
			s.readByte (m_info.maxclients);
		
		if (flags & PWADs) {
			uint8 numwads;
			s.readByte (numwads);
			
			while (numwads--) {
				str wad;
				s.readString (wad);
				m_info.pwads << wad;
			}
		}
		
		if (flags & GameType) {
			s.readByte (m_info.gametype);
			s.readByte (m_info.instagib);
			s.readByte (m_info.buckshot);
		}
		
		if (flags & IWAD)
			s.readString (m_info.iwad);
		
		if (flags & NumPlayers)
			s.readByte (m_info.players);
		
		if (m_info.gametype >= NumGames)
			m_info.gametype = Cooperative;
		
		m_info.state = Known;
		break;
	
	case ZandronumProtocol::ServerQueryIgnoring:
		m_info.state = Throttled;
		break;
	
	case ZandronumProtocol::ServerQueryBanned:
		m_info.state = Banned;
		break;
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
void tickServerRequests() {
	for (LauncherRequest * req : g_serverRequests) {
		req->update();
		
		if (req->done()) {
			CoString target = req->payloadTarget();
			LauncherRequest::InfoStruct& info = req->m_info;
			CoStringList replies;
			
			switch (req->m_info.state) {
			case LauncherRequest::TimedOut:
				replies << fmt ("Query to %1 timed out.", req->address());
				break;
			
			case LauncherRequest::Banned:
				replies << fmt ("I'm banned from %1!", req->address());
				break;
			
			case LauncherRequest::Throttled:
				replies << fmt ("I'm throttled from %1, please try again in 10 seconds.", req->address());
				break;
			
			case LauncherRequest::Known:
				print ("%1\n", info.gametype);
				replies << fmt ("Version %1 server", info.version);
				replies << fmt ("Name: %1", info.name);
				replies << fmt ("Players: %1 / %2", (int) info.players, (int) info.maxclients);
				replies << fmt ("Gamemode: %1, map: %2", gamestrings[info.gametype], info.mapname);
				replies << fmt ("IWAD: %1, PWADs: %2", info.iwad, arrayRep (info.pwads));
				break;
			
			case LauncherRequest::Unknown:
				replies << fmt ("Server %1 replied with garbage.", req->address());
				break;
			}
			
			for (const str& reply : replies)
				g_IRCConnection->privmsg (target, reply);
			
			delete req;
			g_serverRequests.remove (req);
		}
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
void addServerQuery (CoIPAddress addr, str target) {
	LauncherRequest* req = new LauncherRequest;
	req->setBlocking (false);
	req->init (0);
	req->setDone (false);
	req->setAddress (addr);
	req->setPayloadTarget (target);
	req->setNextRequest (CoTime::now());
	req->setDeadline (CoTime::now() + 5);
	g_serverRequests << req;
	
	g_IRCConnection->privmsg (target, fmt ("Querying %1...", addr));
}
