// Make KDevelop understand these
#ifdef IN_IDE_PARSER
#define _N(X) X,
namespace Name {
	enum Type {
		None = 0,
#endif

_N(Nickname)
_N(Username)
_N(Realname)
_N(Channel)
_N(IRCServer)
_N(IRCPort)
_N(IRCCommandPrefix)
_N(OwnerSymbol)
_N(ProtectSymbol)
_N(OpSymbol)
_N(HalfOpSymbol)
_N(VoiceSymbol)
_N(UserFlags)
_N(TrackerAccount)
_N(TrackerPassword)
_N(TrackerURL)
_N(NickServ)
_N(IRCPassword)

#ifdef IN_IDE_PARSER
	};
}
#endif