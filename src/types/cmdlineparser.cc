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
	mShortName (shortName), \
	mLongName  (longName), \
	mType      (N), \
	mPtr       (ptr) {}

DEFINE_SCHEMA_CTOR (int, Int)
DEFINE_SCHEMA_CTOR (char, Char)
DEFINE_SCHEMA_CTOR (bool, Bool)
DEFINE_SCHEMA_CTOR (float, Float)
DEFINE_SCHEMA_CTOR (String, String)

// =============================================================================
// -----------------------------------------------------------------------------
CoCmdLineParser::CoCmdLineParser (int argc, char** argv, Schema& schema) :
	mArgc (argc),
	mArgv (argv),
	mSchema (&schema) {}

void CoCmdLineParser::parse (StringList& args)
{
	SchemaEntry* lastOpt = null;
	bool endOfOpts = false;

	for (int i = 1; i < mArgc; ++i)
	{
		String arg (mArgv[i]);

		if (arg[0] == '-' && !endOfOpts && !lastOpt)
		{
			if (arg == "--")
			{
				endOfOpts = true;
				continue;
			}

			if (arg[1] == '-')
			{
				// Long form name: --arg=value
				int equals = arg.FirstIndexOf ("=");
				String longname = arg.GetSubstring (2, equals),
						 value = arg.GetSubstring (equals + 1, -1);

				SchemaEntry* entry = findSchemaEntry (longname);

				if (!entry)
				{
					PrintTo (stderr, "error: unknown argument %1\n", longname);
					exit (EXIT_FAILURE);
				}

				parseOptionValue (entry, value);
			}
			else
			{
				// Short form name: -a
				if (arg.Length() != 2)
				{
					PrintTo (stderr, "error: expected argument of form -a or --arg, got %1\n", arg);
					exit (EXIT_FAILURE);
				}

				SchemaEntry* entry = findSchemaEntry (arg[1]);

				if (!entry)
				{
					PrintTo (stderr, "error: unknown option `%1`\n", arg);
					exit (EXIT_FAILURE);
				}

				if (entry->mType == SchemaEntry::Toggle)
					entry->mPtr.value<bool>() = true;

				lastOpt = entry;
			}
		} elif (lastOpt != null)

		{
			parseOptionValue (lastOpt, arg);
			lastOpt = null;
		} else
		{
			args << arg;
		}
	}

	if (lastOpt != null)
	{
		PrintTo (stderr, "error: option --%1 requires a value\n", lastOpt->mLongName);
		exit (EXIT_FAILURE);
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoCmdLineParser::parseOptionValue (SchemaEntry* opt, const String& value)
{
	bool ok;
	int base;

	switch (opt->mType)
	{
		case SchemaEntry::Int:
		{
			base = (value.GetSubstring (0, 2).ToLowercase() == "0x") ? 16 : 10;
			opt->mPtr.value<int>() = value.ToLong (&ok, base);
		} break;

		case SchemaEntry::Float:
		{
			opt->mPtr.value<float>() = value.ToFloat (&ok);
		} break;

		case SchemaEntry::Bool:
		{
			bool& val = opt->mPtr.value<bool>();
			String lowCoString = value.ToLowercase();
			ok = true;

			if (lowCoString == "true" || lowCoString == "on" || lowCoString == "1")
				val = true;

			elif (lowCoString == "false" || lowCoString == "off" || lowCoString == "0")
			val = false;
			else
				ok = false;
		} break;

		case SchemaEntry::Char:
		{
			ok = true;

			if (value.Length() == 1)
				opt->mPtr.value<char>() = value[0];
			else
				ok = false;
		} break;

		case SchemaEntry::String:
		{
			ok = (value.Length() > 0);
			opt->mPtr.value<String>() = value;
		} break;

		default:
			break;
	}

	if (!ok)
	{
		PrintTo (stderr, "error: invalid argument passed to option --%1, got `%2`\n",
				opt->mLongName, value);
		exit (EXIT_FAILURE);
	}
}

// =============================================================================
// -----------------------------------------------------------------------------
CoCmdLineParser::SchemaEntry* CoCmdLineParser::findSchemaEntry (char shortName)
{
	SchemaEntry* entry = null;

	for (SchemaEntry & e : *mSchema)
	{
		if (e.mShortName == shortName)
		{
			entry = &e;
			break;
		}
	}

	return entry;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoCmdLineParser::SchemaEntry* CoCmdLineParser::findSchemaEntry (const String& longName)
{
	SchemaEntry* entry = null;

	for (SchemaEntry & e : *mSchema)
	{
		if (e.mLongName == longName)
		{
			entry = &e;
			break;
		}
	}

	return entry;
}
