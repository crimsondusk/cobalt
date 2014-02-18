#include "main.h"
#include "config.h"
#include "xml.h"
#include "format.h"
#include "xml_node.h"
#include "xml_document.h"
#include "string_list.h"

// -----------------------------------------------------------------------------
//
namespace cbl
{
	static config_data*		g_config_data;
	static xml_document*	g_xml = null;
	static config_data*		g_first = null;

	config_adder::config_adder( void* ptr, data_type type, const char* name )
	{
		config_data* data = new config_data;
		data->ptr = ptr;
		data->type = type;
		data->name = name;
		data->next = g_first;
		g_first = data;
	}

	// -----------------------------------------------------------------------------
	//
	static void update_xml_node( string const& name, void* ptr, data_type type )
	{
		xml_node* node = g_xml->navigate( name.split( "_" ), true );

		if( !node )
			node = new xml_node( name, g_xml->root() );

		switch( type )
		{
			case cfgtype_int:
			{
				node->set_contents( string::from_number( *( reinterpret_cast<int*>( ptr ))));
				break;
			}

			case cfgtype_string:
			{
				node->set_contents( *( reinterpret_cast<string*>( ptr )));
				break;
			}

			case cfgtype_float:
			{
				node->set_contents( string::from_number( *( reinterpret_cast<float*>( ptr ))));
				break;
			}

			case cfgtype_bool:
			{
				node->set_contents( *( reinterpret_cast<bool*>( ptr ) ) ? "true" : "false" );
				break;
			}

			case cfgtype_stringlist:
			{
				assert( node->subnodes().is_empty() );

				for( string const& item : *reinterpret_cast<string_list*>( ptr ))
				{
					xml_node* subnode = new xml_node( "item", node );
					subnode->set_contents( item );
				}

				break;
			}

			case cfgtype_intlist:
			{
				for( int item : *reinterpret_cast<cfg_intlist*>( ptr ))
				{
					xml_node* subnode = new xml_node( "item", node );
					subnode->set_contents( string::from_number( item ));
				}

				break;
			}

			case cfgtype_stringmap:
			{
				for( auto pair : * reinterpret_cast<cfg_stringmap*>( ptr ))
				{
					xml_node* subnode = new xml_node( pair.first, node );
					subnode->set_contents( pair.second );
				}

				break;
			}
		}
	}

	// =============================================================================
	// -----------------------------------------------------------------------------
	static void set_config_value( void* ptr, data_type type, xml_node* node )
	{
		switch( type )
		{
			case cfgtype_int:
			{
				*reinterpret_cast<int*>( ptr ) = node->contents().to_long();
				break;
			}

			case cfgtype_string:
			{
				*reinterpret_cast<string*>( ptr ) = node->contents();
				break;
			}

			case cfgtype_float:
			{
				*reinterpret_cast<float*>( ptr ) = node->contents().to_float();
				break;
			}

			case cfgtype_bool:
			{
				string val = node->contents();
				bool& var = * ( reinterpret_cast<bool*>( ptr ) );

				if( val == "true" || val == "1" || val == "on" || val == "yes" )
					var = true;
				else
					var = false;

				break;
			}

			case cfgtype_stringlist:
			{
				string_list& var = *reinterpret_cast<string_list*>( ptr );

				for( const xml_node* subnode : node->subnodes() )
					var << subnode->contents();

				break;
			}

			case cfgtype_intlist:
			{
				cfg_intlist& var = *reinterpret_cast<cfg_intlist*>( ptr );

				for( const xml_node* subnode : node->subnodes() )
					var << subnode->contents().to_long();

				break;
			}

			case cfgtype_stringmap:
			{
				cfg_stringmap& var = *reinterpret_cast<cfg_stringmap*>( ptr );

				for( const xml_node * subnode : node->subnodes() )
					var[subnode->name()] = subnode->contents();

				break;
			}
		}
	}

	// -----------------------------------------------------------------------------
	//
	// Load the configuration from @fname
	//
	bool load( const string& fname )
	{
		print( "config::load: Loading configuration file from %1\n", fname );
		xml_document* doc = xml_document::load_from_file( fname );

		if( !doc )
			return false;

		for( config_data* i = g_config_data; i != null; i = i->next )
		{
			xml_node* node = doc->navigate( i->name.split( "_" ));

			if( node )
				set_config_value( i->ptr, i->type, node );
		}

		g_xml = doc;
		return true;
	}

	// -----------------------------------------------------------------------------
	//
	// Save the configuration to @fname
	//
	bool save_config( const string& fname )
	{
		if( g_xml == null )
			g_xml = xml_document::new_document( "config" );

		print( "Saving configuration to %1...\n", fname );

		for( config_data* i = g_config_data; i != null; i = i->next )
			update_xml_node( i->name, i->ptr, i->type );

		return g_xml->save_to_file( fname );
	}

	// =============================================================================
	// Reset configuration defaults.
	// -----------------------------------------------------------------------------
	/*
	void reset() {
		for (alias i : g_configData) {
			if (i.name == null)
				break;

			set_config_value (i.ptr, i.type, i.def);
		}
	}
	*/

	xml_document* xml()
	{
		return g_xml;
	}
}