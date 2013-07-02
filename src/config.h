#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "common.h"
#include "string.h"

typedef struct {
	str node;
	int port;
} gameserverinfo;

typedef struct {
	Name::Type key;
	str val;
} configinfo;

extern vector<gameserverinfo> g_GameServers;
extern vector<configinfo> g_Config;
extern vector<str> g_AdminMasks;

void loadConfig ();
str cfg (Name::Type cfgname);

#endif // __CONFIG_H__