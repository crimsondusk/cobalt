#pragma once
#include "main.h"
#include "xml_node.h"

namespace cbl
{
	// =============================================================================
	//
	class xml_document
	{
		public:
			typedef map<string, string> header_type;

			PROPERTY( protected,	header_type,	header,	set_header,	STOCK_WRITE )
			PROPERTY( private,		xml_node*,		root,	set_root,	STOCK_WRITE )

		public:
			xml_document( xml_node* root = null );
			~xml_document();

			xml_node* find_node_by_name( string name ) const;
			xml_node* navigate( string_list const& path, bool create = false );
			xml_node* navigate( string_list const& path ) const;
			bool save_to_file( string fname ) const;

			static string			encode_string( string in );
			static string			decode_string( string in );
			static string			get_parse_error();
			static xml_document*	load_from_file( string fname );
			static xml_document*	new_document( string root_name );

		private:
			void write_node_to_file( FILE* fp, const xml_node* node ) const;
	};

}
