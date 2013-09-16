#if (defined IN_IDE_PARSER || defined DEFINE_GAME_ENUMS)
# define BEGIN_GAMES_ENUM     enum GameType {
# define GAME(N)                  N,
# define END_GAMES_ENUM       };
#endif

#ifndef BEGIN_GAMES_ENUM
# error gameenums.h incorrectly included, did you mean zanserver.h?
#endif

BEGIN_GAMES_ENUM
GAME (Cooperative)
GAME (Survival)
GAME (Invasion)
GAME (Deathmatch)
GAME (TeamDM)
GAME (Duel)
GAME (Terminator)
GAME (LMS)
GAME (TeamLMS)
GAME (Possession)
GAME (TeamPossession)
GAME (Teamgame)
GAME (CTF)
GAME (OneFlagCTF)
GAME (Skulltag)
GAME (Domination)
GAME (NumGames)
END_GAMES_ENUM

#undef BEGIN_GAMES_ENUM
#undef GAME
#undef END_GAMES_ENUM