#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "common.h"
#include "irc.h"

typedef struct {
	IRCUser* user;
	str channel;
	str message;
} sIRCCommandMeta;

#define CMD_ARGS IRCConnection*, sIRCCommandMeta, array<str>
#define COMMANDTYPE(N) void (*N) (CMD_ARGS)

#define command(X) void COMMAND_##X (CMD_ARGS); \
IRCCommandAdder CMDAdder_##X (#X, &COMMAND_##X); \
void COMMAND_##X (IRCConnection* conn, sIRCCommandMeta meta, array<str> parms)

class IRCCommandAdder {
public:
	IRCCommandAdder (const char* namestring, COMMANDTYPE (func));
};

typedef struct {
	const char* namestring;
	COMMANDTYPE (func);
} FunctionMeta;

// =============================================================================
// List of all commands we have defined + pointers to them
extern array<FunctionMeta> g_CommandMeta;

#endif // __COMMANDS_H__