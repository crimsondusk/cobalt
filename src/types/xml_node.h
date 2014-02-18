#pragma once
#include "main.h"

namespace cbl
{
	// =============================================================================
	//
	class xml_node
	{
		public:
			using attribute_map = map<string, string>;

		PROPERTY( public,		string,				contents,	set_contents,	STOCK_WRITE )
		PROPERTY( public,		string,				name,		set_name,		STOCK_WRITE )
		PROPERTY( protected,	list<xml_node*>,	subnodes,	set_subnodes,	STOCK_WRITE )
		PROPERTY( protected,	attribute_map,		attributes,	set_attributes,	STOCK_WRITE )
		PROPERTY( protected,	bool,				is_cdata,	set_cdata,		STOCK_WRITE )
		PROPERTY( protected,	xml_node*,			parent,		set_parent,		STOCK_WRITE )

		public:
			xml_node( string const& name, xml_node* parent );
			~xml_node();

			xml_node*			add_subnode( string const& name, string const& cont );
			string	find_attribute( const cbl::string& name ) const;
			void				dropNode( xml_node* node );
			xml_node*			find_subnode( string const& fname, bool recursive = false );
			list<xml_node*>		get_nodes_by_attribute( const string& attrname, const string& attrvalue ) const;
			xml_node*			get_one_node_by_attribute( const string& attrname, const string& attrvalue ) const;
			list<xml_node*>		get_nodes_by_name( const string& name ) const;
			bool				has_attribute( string const& name ) const;
			bool				is_empty() const;
			void				set_attribute( string const& name, string const& data );

		protected:
			friend class xml_document;
	};
}
