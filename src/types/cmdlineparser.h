#ifndef LIBCOBALT_CMDLINEPARSER_H
#define LIBCOBALT_CMDLINEPARSER_H

#include "string.h"
#include "typedpointer.h"

// =============================================================================
// -----------------------------------------------------------------------------
class CoCmdLineParser
{
	public:
		class SchemaEntry
		{
			public:
				enum Type
				{
					Int,
					Bool,
					Float,
					Char,
					String,
					Toggle,
				};

#define DECLARE_SCHEMA_CTOR(T) \
		SchemaEntry (char shortName, const char* longName, T* ptr);
				DECLARE_SCHEMA_CTOR (int)
				DECLARE_SCHEMA_CTOR (char)
				DECLARE_SCHEMA_CTOR (bool)
				DECLARE_SCHEMA_CTOR (float)
				DECLARE_SCHEMA_CTOR (String)
#undef DECLARE_SCHEMA_CTOR

			protected:
				char             mShortName;
				const char*      mLongName;
				Type             mType;
				CoTypedPointer   mPtr;
				friend class CoCmdLineParser;
		};

		typedef List<SchemaEntry> Schema;

		CoCmdLineParser (int argc, char** argv, Schema& schema);

		SchemaEntry* findSchemaEntry (char shortName);
		SchemaEntry* findSchemaEntry (const String& longName);
		void parse (StringList& args);
		void parseOptionValue (SchemaEntry* opt, const String& value);

	private:
		int         mArgc;
		char**      mArgv;
		Schema*     mSchema;
};

#endif // LIBCOBALT_CMDLINEPARSER_H
