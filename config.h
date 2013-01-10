#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "common.h"

typedef struct {
	str node;
	int port;
} gameserverinfo;

typedef struct {
	name_e key;
	str val;
} configinfo;

extern array<gameserverinfo> g_GameServers;
extern array<configinfo> g_Config;
extern array<str> g_AdminMasks;

void loadConfig ();
void parseError (const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
void parseWarning (const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
str doGetConfig (name_e cfg);

#define getConfig(X) doGetConfig (name (X))

#endif // __CONFIG_H__