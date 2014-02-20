#include <cstdio>
#include "main.h"
#include "xml_document.h"
#include "format.h"
#include "xml_scanner.h"
#include "misc.h"
#include "string_list.h"

namespace cbl
{
	// -----------------------------------------------------------------------------
	//
	static string				g_error_string;
	static vector<xml_node*>	g_stack;
	static int					g_save_stack;

	static const pair<string, string> gEncodingConversions[] =
	{
		{"&",  "&amp;"},
		{"<",  "&lt;"},
		{">",  "&gt;"},
		{"\"", "&quot;"}
	};

	// -----------------------------------------------------------------------------
	//
	static xml_node* topStackNode()
	{
		if( g_stack.size() == 0 )
			return null;

		return g_stack.last();
	}

	// -----------------------------------------------------------------------------
	//
	xml_document::xml_document( xml_node* root ) :
		m_root( root )
	{
		m_header["version"] = "1.0";
		m_header["encoding"] = "UTF-8";
	}

	// -----------------------------------------------------------------------------
	//
	xml_document::~xml_document()
	{
		delete m_root;
	}

	// -----------------------------------------------------------------------------
	//
	xml_document* xml_document::new_document( string root_name )
	{
		return new xml_document( new xml_node( root_name, null ));
	}

	// -----------------------------------------------------------------------------
	//
	xml_document* xml_document::load_from_file( string fname )
	{
		FILE*			fp = null;
		long			fsize;
		char*			buf = null;
		xml_node*		root = null;
		header_type		header;

		try
		{
			if( ( fp = fopen( fname.c_str(), "r" )) == null )
				throw format( "couldn't open %1 for reading: %2", fname, strerror( errno ));

			fseek( fp, 0l, SEEK_END );
			fsize = ftell( fp );
			rewind( fp );
			buf = new char[fsize + 1];

			if( ( long ) fread( buf, 1, fsize, fp ) < fsize )
				throw format( "I/O error while opening %1", fname );

			buf[fsize] = '\0';
			fclose( fp );
			fp = null;
			xml_scanner scan( buf );
			scan.must_scan_next( xml_scanner::tk_header_start );

			while( scan.scan_next_token( xml_scanner::tk_symbol ))
			{
				string attrname = scan.token_string();
				scan.must_scan_next( xml_scanner::tk_equals );
				scan.must_scan_next( xml_scanner::tk_string );
				header[attrname] = scan.token_string();
			}

			scan.must_scan_next( xml_scanner::tk_header_end );

			if( header.find( "version" ) == header.end() )
				throw string( "No version defined in header!" );

			while( scan.scan_next_token() )
			{
				switch( scan.token_type() )
				{
					case xml_scanner::tk_tag_start:
					{
						scan.must_scan_next( xml_scanner::tk_symbol );
						xml_node* node = new xml_node( scan.token_string(), topStackNode() );

						if( g_stack.size() == 0 )
						{
							if( root != null )
							{
								// XML forbids having multiple roots
								delete node;
								throw string( "Multiple root nodes" );
							}

							root = node;
						}

						g_stack << node;

						while( scan.scan_next_token( xml_scanner::tk_symbol ))
						{
							string attrname = scan.token_string();
							scan.must_scan_next( xml_scanner::tk_equals );
							scan.must_scan_next( xml_scanner::tk_string );
							node->set_attribute( attrname, scan.token_string() );
							assert( node->has_attribute( attrname ));
						}

						if( scan.scan_next_token( xml_scanner::tk_tag_self_closer ))
						{
							xml_node* popee;
							assert( g_stack.pop( popee ) && popee == node );
						}
						else
							scan.must_scan_next( xml_scanner::tk_tag_end );

						break;
					}

					case xml_scanner::tk_tag_closer:
					{
						scan.must_scan_next( xml_scanner::tk_symbol );
						xml_node* popee;

						if( g_stack.pop( popee ) == false || popee->name() != scan.token_string() )
							throw string( "Misplaced closing tag" );

						scan.must_scan_next( xml_scanner::tk_tag_end );
						break;
					}

					case xml_scanner::tk_cdata:
					case xml_scanner::tk_symbol:
					{
						if( g_stack.size() == 0 )
							throw string( "Misplaced CDATA/symbol" );

						xml_node* node = g_stack[g_stack.size() - 1];

						node->set_cdata( scan.token_type() == xml_scanner::tk_cdata );
						node->set_contents( node->is_cdata() ? scan.token_string() : decode_string( scan.token_string() ));
						break;
					}

					case xml_scanner::tk_string:
					case xml_scanner::tk_header_start:
					case xml_scanner::tk_header_end:
					case xml_scanner::tk_equals:
					case xml_scanner::tk_tag_self_closer:
					case xml_scanner::tk_tag_end:
					{
						throw format( "Unexpected token '%1'", scan.token_string() );
						break;
					}
				}
			}
		}
		catch( string& e )
		{
			g_error_string = e;
			delete[] buf;
			delete root;

			if( fp != null )
				fclose( fp );

			return null;
		}

		delete[] buf;
		xml_document* doc = new xml_document( root );
		doc->set_header( header );
		return doc;
	}

	// -----------------------------------------------------------------------------
	//
	bool xml_document::save_to_file( string fname ) const
	{
		FILE* fp;

		if(( fp = fopen( fname.c_str(), "w" )) == null )
			return false;

		print_to( fp, "<?xml" );

		for( auto it = header().begin(); it != header().end(); ++it )
			print_to( fp, " %1=\"%2\"", it->first, it->second );

		print_to( fp, " ?>\n" );
		g_save_stack = 0;
		write_node_to_file( fp, root() );
		fclose( fp );
		return true;
	}

	// =============================================================================
	//
	void xml_document::write_node_to_file( FILE* fp, const xml_node* node ) const
	{
		string indent;

		for( int i = 0; i < g_save_stack; ++i )
			indent += "\t";

		print_to( fp, "%1<%2", indent, node->name() );

		for( auto it = node->attributes().begin(); it != node->attributes().end(); ++it )
			print_to( fp, " %1=\"%2\"", encode_string( it->first ), encode_string( it->second ));

		if( node->is_empty() && g_save_stack > 0 )
		{
			print_to( fp, " />\n" );
			return;
		}

		print_to( fp, ">" );

		if( node->subnodes().size() > 0 )
		{
			// Write nodes
			print_to( fp, "\n" );

			for( const xml_node* subnode : node->subnodes() )
			{
				g_save_stack++;
				write_node_to_file( fp, subnode );
				g_save_stack--;
			}

			print_to( fp, indent );
		}
		else
		{
			// Write content
			if( node->is_cdata() )
				print_to( fp, "<![CDATA[%1]]>", node->contents() );
			else
				print_to( fp, encode_string( node->contents() ));
		}

		print_to( fp, "</%1>\n", node->name() );
	}

	// -----------------------------------------------------------------------------
	//
	string xml_document::encode_string( string in )
	{
		string out( in );

		for( auto i : gEncodingConversions )
			out.replace( i.first, i.second );

		return out;
	}

	// -----------------------------------------------------------------------------
	//
	string xml_document::decode_string( string in )
	{
		string out( in );

		for( auto i : gEncodingConversions )
			out.replace( i.second, i.first );

		return out;
	}

	// -----------------------------------------------------------------------------
	//
	xml_node* xml_document::find_node_by_name( string name ) const
	{
		return root()->find_subnode( name, true );
	}

	// -----------------------------------------------------------------------------
	//
	xml_node* xml_document::navigate( const cbl::string_list& path, bool create )
	{
		xml_node* node = root();

		for( string const& name : path )
		{
			xml_node* parent = node;
			node = parent->find_subnode( name );

			if( !node )
			{
				if( create )
					node = new xml_node( name, parent );
				else
					return null;
			}
		}

		return node;
	}

	// -----------------------------------------------------------------------------
	//
	xml_node* xml_document::navigate( const string_list& path ) const
	{
		return const_cast<xml_document*>( this )->navigate( path, false );
	}

	// -----------------------------------------------------------------------------
	//
	string xml_document::get_parse_error()
	{
		return g_error_string;
	}
}
