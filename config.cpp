#include <stdarg.h>
#include "common.h"
#include "config.h"
#include "scanner.h"
#include "array.h"

array<gameserverinfo> g_GameServers;
array<configinfo> g_Config;

typedef struct {
	name_e name;
	const char* label;
	bool defined;
} configmeta;

#define CFG(X) {name(X), #X, false},
#define iterate_meta for (uint i = 0; i < sizeof (g_ConfigMeta) / sizeof (*g_ConfigMeta); i++)

configmeta g_ConfigMeta[] = {
	CFG (Nickname)
	CFG (Username)
	CFG (Realname)
	CFG (Channel)
	CFG (IRCServer)
	CFG (IRCPort)
	CFG (IRCCommandPrefix)
	CFG (OpSymbol)
	CFG (HalfOpSymbol)
	CFG (VoiceSymbol)
	CFG (UserFlags)
};

void parseError (const char* fmt, ...) {
	va_list v;
	va_start (v, fmt);
	
	fprintf (stderr, "Couldn't parse configuration: ");
	vfprintf (stderr, fmt, v);
	fprintf (stderr, "\n");
	va_end (v);
	
	exit (1);
}

void parseWarning (const char* fmt, ...) {
	va_list v;
	va_start (v, fmt);
	
	fprintf (stderr, "Warning: ");
	vfprintf (stderr, fmt, v);
	fprintf (stderr, "\n");
	va_end (v);
}

#define BUFSIZE 256

void loadConfig () {
	// If the config was already loaded, clear it.
	g_Config.clear();
	g_GameServers.clear();
	
	str configfile;
	FILE* fp = fopen (APPNAME ".cfg", "r");
	if (!fp)
		parseError ("couldn't open " APPNAME ".cfg for reading");
	
	// Determine file size
	fseek (fp, 0, SEEK_END);
	size_t fsize = ftell (fp);
	rewind (fp);
	
	char* buf = new char[fsize];
	assert (fread (buf, sizeof (char), fsize, fp) == fsize);
	
	Scanner r (buf);
	enum {
		PARSERMODE_Properties,
		PARSERMODE_Servers,
	} parserMode = PARSERMODE_Properties;
	
	while (r.Next(parserMode == PARSERMODE_Servers)) {
		if (+(r.token) == "SERVERS") {
			r.MustNext (":");
			parserMode = PARSERMODE_Servers;
			continue;
		}
		
		if (parserMode == PARSERMODE_Properties) {
			name_e cfgName = name(None);
			
			iterate_meta {
				if (r.token.icompare (g_ConfigMeta[i].label) == 0) {
					cfgName = g_ConfigMeta[i].name;
					g_ConfigMeta[i].defined = true;
					break;
				}
			}
			
			if (cfgName == name(None))
				parseError ("unknown token `%s`", r.token.chars());
			
			// Get the value.
			str cfgValue;
			r.MustNext ("=");
			r.MustString ();
			cfgValue = r.token;
			r.MustNext (";");
			
			configinfo cfg;
			cfg.key = cfgName;
			cfg.val = cfgValue;
			g_Config << cfg;
		} else if (parserMode == PARSERMODE_Servers) {
			// Check that the IP is well formed.
			bool valid = true;
			
			if (r.token % "." != 3 || r.token % ":" != 1 ||
				r.token.last (".") > r.token.first (":"))
			{
				parseError ("ill-formed IP: `%s`, was expecting of type `a.b.c.d:port`", r.token.chars());
			}
			
			str ipstring = r.token.substr (0, r.token.first (":"));
			array<str> iparray = ipstring / ".";
			assert (iparray.size() == 4);
			for (int i = 0; i < 4 && valid; i++) {
				str valstr = iparray[i];
				if (!valstr.isnumber () || atoi (valstr) < 0 || atoi (valstr) > 255)
					parseError ("IP octet #%d is invalid, got `%s`, expected a number 0-255",
						i + 1, valstr.chars());
			}
			
			str port = r.token.substr (r.token.first (":") + 1, -1);
			if (!port.isnumber() || atoi (port) < 0 || atoi (port) >= 0x10000)
				parseError ("bad port string, got `%s`, expected a number", port.chars());
			
			gameserverinfo info;
			info.node = ipstring;
			info.port = atoi (port);
			g_GameServers << info;
		} else
			assert (false);
	}
	
	fclose (fp);
	
	// Make sure we have all configured
	iterate_meta {
		if (!g_ConfigMeta[i].defined) {
			parseError ("%s was not defined in configuration", g_ConfigMeta[i].label);
		}
	}
	
	// Check that everything is right in the config
	for (uint i = 0; i < g_Config.size(); i++) {
		if (g_Config[i].key == name(IRCCommandPrefix) &&
			g_Config[i].val.len() != 1)
		{
			parseWarning ("IRC command prefix `%s` not one character long, it will be truncated",
				(char*)g_Config[i].val);
			g_Config[i].val = g_Config[i].val[0];
		}
	}
}

str doGetConfig (name_e cfg) {
	for (uint i = 0; i < g_Config.size(); i++) {
		if (g_Config[i].key == cfg) {
			return g_Config[i].val;
		}
	}
	
	fprintf (stderr, "WARNING: couldn't read config entry %s!\n", g_NameStrings[cfg]);
	return "";
}