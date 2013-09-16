#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
#include "main.h"

#define CONFIG(T, NAME, DEFAULT) CoConfig::T NAME = DEFAULT; \
	CoConfig::ConfigAdder zz_ConfigAdder_##NAME (&NAME, CoConfig::T##Type, \
		#NAME, CoConfig::T (DEFAULT));

#define EXTERN_CONFIG(T, NAME) extern CoConfig::T NAME;
#define COBALT_MAX_CONFIG 512

class CoXMLDocument;

// =========================================================
namespace CoConfig {
	enum Type {
		IntType,
		StringType,
		FloatType,
		BoolType,
		IntListType,
		StringListType,
		StringMapType,
	};
	
	struct ConfigData {
		void* ptr;
		Type type;
		const char* name;
	};
	
	extern ConfigData g_configData[COBALT_MAX_CONFIG];
	extern int g_cfgDataCursor;
	
	// Type-definitions for the above enum list
	typedef int Int;
	typedef CoString String;
	typedef float Float;
	typedef bool Bool;
	typedef CoList<int> IntList;
	typedef CoStringList StringList;
	typedef CoMap<CoString, CoString> StringMap;
	
	// ------------------------------------------
	bool           load (CoStringRef fname);
	bool           save (CoStringRef fname);
	CoXMLDocument* xml();
	
	class ConfigAdder {
	public:
		// =============================================================================
		// We cannot just add config objects to a list or vector because that would rely
		// on the CoList's c-tor being called before the configs' c-tors. With global
		// variables we cannot assume that!! Therefore we need to use a C-style array here.
		// -----------------------------------------------------------------------------
		template<class T> ConfigAdder (void* ptr, Type type, const char* name, const T& def) {
			if (g_cfgDataCursor == 0)
				memset (g_configData, 0, sizeof g_configData);
			
			assert (g_cfgDataCursor < COBALT_MAX_CONFIG);
			alias i = g_configData[g_cfgDataCursor++];
			i.ptr = ptr;
			i.type = type;
			i.name = name;
			
			*(reinterpret_cast<T*> (ptr)) = def;
		}
	};
};

#endif // CONFIG_H