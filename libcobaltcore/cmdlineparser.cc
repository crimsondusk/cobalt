#include "cmdlineparser.h"
#include "containers.h"
#include "types.h"
#include "format.h"
#include "variant.h"
#include "main.h"

// =============================================================================
// -----------------------------------------------------------------------------
#define DEFINE_SCHEMA_CTOR(T, N) \
CoCmdLineParser::SchemaEntry::SchemaEntry (char shortName, const char* longName, T* ptr) : \
	m_shortName (shortName), \
	m_longName  (longName), \
	m_type      (N), \
	m_ptr       (ptr) {}

DEFINE_SCHEMA_CTOR (int, Int)
DEFINE_SCHEMA_CTOR (char, Char)
DEFINE_SCHEMA_CTOR (bool, Bool)
DEFINE_SCHEMA_CTOR (float, Float)
DEFINE_SCHEMA_CTOR (CoString, String)

// =============================================================================
// -----------------------------------------------------------------------------
CoCmdLineParser::CoCmdLineParser (int argc, char** argv, Schema& schema) :
	m_argc   (argc),
	m_argv   (argv),
	m_schema (&schema) {}

void CoCmdLineParser::parse (CoStringList& args) {
	SchemaEntry* lastOpt = null;
	bool endOfOpts = false;
	
	for (int i = 1; i < m_argc; ++i) {
		CoString arg (m_argv[i]);
		
		if (arg[0] == '-' && !endOfOpts && !lastOpt) {
			if (arg == "--") {
				endOfOpts = true;
				continue;
			}
			
			if (arg[1] == '-') {
				// Long form name: --arg=value
				int equals = arg.first ("=");
				CoString longname = arg.substr (2, equals),
					value = arg.substr (equals + 1, -1);
				
				SchemaEntry* entry = findSchemaEntry (longname);
				
				if (!entry) {
					fprint (stderr, "error: unknown argument %1\n", longname);
					exit (EXIT_FAILURE);
				}
				
				parseOptionValue (entry, value);
			} else {
				// Short form name: -a
				if (arg.length() != 2) {
					fprint (stderr, "error: expected argument of form -a or --arg, got %1\n", arg);
					exit (EXIT_FAILURE);
				}
				
				SchemaEntry* entry = findSchemaEntry (arg[1]);
				
				if (!entry) {
					fprint (stderr, "error: unknown option `%1`\n", arg);
					exit (EXIT_FAILURE);
				}
				
				if (entry->m_type == SchemaEntry::Toggle)
					entry->m_ptr.value<bool>() = true;
				
				lastOpt = entry;
			}
		} elif (lastOpt != null) {
			parseOptionValue (lastOpt, arg);
			lastOpt = null;
		} else {
			args << arg;
		}
	}
	
	if (lastOpt != null) {
		fprint (stderr, "error: option --%1 requires a value\n", lastOpt->m_longName);
		exit (EXIT_FAILURE);
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoCmdLineParser::parseOptionValue (SchemaEntry* opt, const CoString& value) {
	bool ok;
	int base;
	
	switch (opt->m_type) {
	case SchemaEntry::Int:
		base = (value.substr (0, 2).lower() == "0x") ? 16 : 10;
		opt->m_ptr.value<int>() = value.toLong (&ok, base);
		break;
	
	case SchemaEntry::Float:
		opt->m_ptr.value<float>() = value.toFloat (&ok);
		break;
	
	case SchemaEntry::Bool:
		{
			bool& val = opt->m_ptr.value<bool>();
			CoString lowCoString = value.lower();
			ok = true;
			
			if (lowCoString == "true" || lowCoString == "on" || lowCoString == "1")
				val = true;
			elif (lowCoString == "false" || lowCoString == "off" || lowCoString == "0")
				val = false;
			else
				ok = false;
		}
		break;
	
	case SchemaEntry::Char:
		ok = true;
		
		if (value.length() == 1)
			opt->m_ptr.value<char>() = value[0];
		else
			ok = false;
		break;
	
	case SchemaEntry::String:
		ok = (value.length() > 0);
		opt->m_ptr.value<CoString>() = value;
		break;
	
	default:
		break;
	}
	
	if (!ok) {
		fprint (stderr, "error: invalid argument passed to option --%1, got `%2`\n",
			opt->m_longName, value);
		exit (EXIT_FAILURE);
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
CoCmdLineParser::SchemaEntry* CoCmdLineParser::findSchemaEntry (char shortName) {
	SchemaEntry* entry = null;
	for (SchemaEntry& e : *m_schema) {
		if (e.m_shortName == shortName) {
			entry = &e;
			break;
		}
	}
	
	return entry;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoCmdLineParser::SchemaEntry* CoCmdLineParser::findSchemaEntry (const CoString& longName) {
	SchemaEntry* entry = null;
	for (SchemaEntry& e : *m_schema) {
		if (e.m_longName == longName) {
			entry = &e;
			break;
		}
	}
	
	return entry;
}