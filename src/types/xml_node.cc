#include "xml_node.h"
#include "xml_document.h"

namespace cbl
{
	// =============================================================================
	//
	xml_node::xml_node( string const& name, xml_node* parent ) :
		m_name( name ),
		m_is_cdata( false ),
		m_parent( parent )
	{
		if( parent )
			parent->m_subnodes << this;
	}

	// =============================================================================
	//
	xml_node::~xml_node()
	{
		for( xml_node* node : subnodes() )
			delete node;

		if( parent() )
			parent()->dropNode( this );
	}

	// =============================================================================
	//
	string xml_node::find_attribute( string const& name ) const
	{
		if( has_attribute( name ))
			return attributes().find( name )->second;

		return string();
	}

	// =============================================================================
	//
	void xml_node::dropNode( xml_node* node )
	{
		m_subnodes.remove( node );
	}

	// =============================================================================
	//
	bool xml_node::has_attribute( string const& name ) const
	{
		return attributes().find( name ) != attributes().end();
	}

	// =============================================================================
	//
	void xml_node::set_attribute( string const& name, string const& data )
	{
		m_attributes[name] = data;
	}

	// =============================================================================
	//
	xml_node* xml_node::find_subnode( string const& fname, bool recursive )
	{
		for( xml_node* node : subnodes() )
		{
			if( node->name() == fname )
				return node;

			xml_node* target;

			if( recursive && ( target = node->find_subnode( fname )) != null )
				return target;
		}

		return null;
	}

	// =============================================================================
	//
	list<xml_node*> xml_node::get_nodes_by_name( string const& name ) const
	{
		list<xml_node*> matches;

		for( xml_node * node : subnodes() )
			if( node->name() == name )
				matches << node;

		return matches;
	}

	// =============================================================================
	//
	bool xml_node::is_empty() const
	{
		return contents().is_empty() && subnodes().is_empty();
	}

	// =============================================================================
	//
	xml_node* xml_node::add_subnode( string const& name, string const& cont )
	{
		xml_node* node = new xml_node( name, this );

		if( cont.length() > 0 )
			node->set_contents( cont );

		return node;
	}

	// =============================================================================
	//
	list<xml_node*> xml_node::get_nodes_by_attribute( string const& attrname, string const& attrvalue ) const
	{
		list<xml_node*> matches;

		for( xml_node* node : subnodes() )
			if( node->find_attribute( attrname ) == attrvalue )
				matches << node;

		return matches;
	}

	// =============================================================================
	//
	xml_node* xml_node::get_one_node_by_attribute( string const& attrname, string const& attrvalue ) const
	{
		for( xml_node * node : subnodes() )
			if( node->find_attribute( attrname ) == attrvalue )
				return node;

		return null;
	}
}
