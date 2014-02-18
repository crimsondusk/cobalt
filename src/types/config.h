#pragma once
#include "main.h"
#include "containers.h"

#define CBL_CONFIG( T, NAME, DEFAULT ) \
	namespace cfg \
	{ \
		cbl::cfg_##T NAME = DEFAULT; \
		static cbl::config_adder zz_config_adder_##NAME( &NAME, cbl::cfgtype_##T, #NAME ); \
	}

#define CBL_EXTERN_CONFIG(T, NAME) \
	namespace cfg \
	{ \
		extern cbl::cfg_##T NAME; \
	}

namespace cbl
{
	// =========================================================
	class xml_document;

	enum data_type
	{
		cfgtype_int,
		cfgtype_string,
		cfgtype_float,
		cfgtype_bool,
		cfgtype_intlist,
		cfgtype_stringlist,
		cfgtype_stringmap,
	};

	struct config_data
	{
		void*			ptr;
		data_type		type;
		string			name;
		config_data*	next;
	};

	// Type-definitions for the above enum list
	using cfg_int			= int;
	using cfg_string		= string;
	using cfg_float			= float;
	using cfg_bool			= float;
	using cfg_intlist		= list<int>;
	using cfg_stringlist	= string_list;
	using cfg_stringmap		= map<string, string>;

	// ------------------------------------------
	bool			load_config( const string& fname );
	bool			save_config( const string& fname );
	xml_document*	get_config_xml();

	class config_adder
	{
		public:
			config_adder( void* ptr, data_type type, const char* name );
	};
}
