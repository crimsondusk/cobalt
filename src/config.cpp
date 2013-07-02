#include <stdarg.h>
#include "common.h"
#include "config.h"
#include "scanner.h"
#include <util/vector.h>

vector<gameserverinfo> g_GameServers;
vector<configinfo> g_Config;
vector<str> g_AdminMasks;

typedef struct
{
	Name::Type name;
	const char* label;
	bool defined;
} configmeta;

#define CFG(X) {Name::X, #X, false},
#define iterate_meta for (uint i = 0; i < sizeof (g_ConfigMeta) / sizeof (*g_ConfigMeta); i++)

// Names to config
configmeta g_ConfigMeta[] =
{
	CFG( Nickname )
	CFG( Username )
	CFG( Realname )
	CFG( Channel )
	CFG( IRCServer )
	CFG( IRCPort )
	CFG( IRCCommandPrefix )
	CFG( OwnerSymbol )
	CFG( ProtectSymbol )
	CFG( OpSymbol )
	CFG( HalfOpSymbol )
	CFG( VoiceSymbol )
	CFG( UserFlags )
	CFG( TrackerAccount )
	CFG( TrackerPassword )
	CFG( TrackerURL )
	CFG( NickServ )
	CFG( IRCPassword )
};

static void parseError( str msg )
{
	fprint( stderr, "Couldn't parse configuration: %1\n", msg );
	abort();
}

/*
static void parseWarning( str msg )
{
	fprint( stderr, "Warning: %1\n", msg );
}
*/

void loadConfig()
{
	// If the config was already loaded, clear it.
	g_Config.clear();
	g_GameServers.clear();
	
	str configfile;
	FILE* fp = fopen( APPNAME ".cfg", "r" );
	
	if( !fp )
		parseError( "couldn't open " APPNAME ".cfg for reading" );
	
	// Determine file size
	fseek( fp, 0, SEEK_END );
	size_t fsize = ftell( fp );
	rewind( fp );
	
	char* buf = new char[fsize];
	assert( fread( buf, sizeof( char ), fsize, fp ) == fsize );
	
	Scanner r( buf );
	enum
	{
		ParsingProperties,
		ParsingServers,
		ParsingAdmins,
	} parserMode = ParsingProperties;
	
	for( ;; )
	{
		str next = r.PeekNext( true );
		
		if( +next == "SERVERS:" )
		{
			r.Next( true );
			parserMode = ParsingServers;
			continue;
		}
		
		if( +next == "ADMINS:" )
		{
			r.Next( true );
			parserMode = ParsingAdmins;
			continue;
		}
		
		// ===========================================
		if( !r.Next( parserMode != ParsingProperties ) )
			break;
		
		switch( parserMode )
		{
		case ParsingProperties:
			{
				Name::Type cfgName = Name::None;
				
				for( configmeta& meta : g_ConfigMeta )
				{
					if( +r.token == +str( meta.label ) )
					{
						cfgName = meta.name;
						meta.defined = true;
						break;
					}
				}
				
				if( cfgName == Name::None )
					parseError( fmt( "unknown token `%1`", r.token ));
				
				// Get the value.
				str cfgValue;
				r.MustNext( "=" );
				r.MustString();
				cfgValue = r.token;
				r.MustNext( ";" );
				
				configinfo cfg;
				cfg.key = cfgName;
				cfg.val = cfgValue;
				g_Config << cfg;
			}
			break;
		
		case ParsingServers:
			{
				// Check that the IP is well formed.
				bool valid = true;
				
				if( r.token.count( '.' ) != 3 || r.token.count( ':' ) != 1 ||
						r.token.last( "." ) > r.token.first( ":" ) )
				{
					parseError( fmt( "ill-formed IP: `%1`, was expecting of type `a.b.c.d:port`", r.token ));
				}
				
				str ipstring = r.token.substr( 0, r.token.first( ":" ) );
				vector<str> iparray = ipstring / ".";
				assert( iparray.size() == 4 );
				
				for( int i = 0; i < 4 && valid; i++ )
				{
					str valstr = iparray[i];
					
					if( !valstr.numeric() || atoi( valstr ) < 0 || atoi( valstr ) > 255 )
						parseError( fmt( "IP octet #%1 is invalid, got `%2`, expected a number 0-255",
							i + 1, valstr ));
				}
				
				str portstr = r.token.substr( r.token.first( ":" ) + 1, -1 );
				
				if( !portstr.numeric() || atoi( portstr ) < 0 || atoi( portstr ) >= 0x10000 )
					parseError( fmt( "bad port string, got `%1`, expected a number", portstr ));
				
				gameserverinfo info;
				info.node = ipstring;
				info.port = atoi( portstr );
				g_GameServers << info;
			}
			break;
		
		case ParsingAdmins:
			// Add the admin mask to list
			g_AdminMasks << r.token;
			break;
		
		default:
			assert( false );
		}
	}
	
	fclose( fp );
	
	// Make sure we have all configured
	for( configmeta& meta : g_ConfigMeta )
		if( !meta.defined )
			parseError( fmt( "%1 was not defined in configuration", meta.label ));
	
	print( "Configuration loaded.\n" );
}

str cfg( Name::Type cfgname )
{
	for( configinfo& info : g_Config )
		if( info.key == cfgname )
			return info.val;
	
	fprint( stderr, "WARNING: couldn't read config entry %1!\n", g_NameStrings[cfgname] );
	return "";
}
