#ifndef LIBCOBALT_CMDLINEPARSER_H
#define LIBCOBALT_CMDLINEPARSER_H

#include "string.h"
#include "typedpointer.h"

// =============================================================================
// -----------------------------------------------------------------------------
class CoCmdLineParser {
public:
	class SchemaEntry {
	public:
		enum Type {
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
		DECLARE_SCHEMA_CTOR (CoString)
		
#undef DECLARE_SCHEMA_CTOR
		
	protected:
		char             m_shortName;
		const char*      m_longName;
		Type             m_type;
		CoTypedPointer   m_ptr;
		friend class CoCmdLineParser;
	};
	
	typedef CoList<SchemaEntry> Schema;
	
	CoCmdLineParser (int argc, char** argv, Schema& schema);
	
	SchemaEntry* findSchemaEntry (char shortName);
	SchemaEntry* findSchemaEntry (const CoString& longName);
	void parse (CoStringList& args);
	void parseOptionValue (SchemaEntry* opt, const CoString& value);
	
private:
	int         m_argc;
	char**      m_argv;
	Schema*     m_schema;
};

#endif // LIBCOBALT_CMDLINEPARSER_H