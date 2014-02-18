#pragma once
#include "main.h"

namespace cbl
{
	class xml_scanner
	{
		public:
			enum token
			{
				tk_header_start,
				tk_header_end,
				tk_tag_closer,
				tk_tag_self_closer,
				tk_tag_start,
				tk_tag_end,
				tk_cdata,
				tk_equals,
				tk_symbol,
				tk_string
			};

			PROPERTY( private, const char*,	data,			set_data,			STOCK_WRITE )
			PROPERTY( private, const char*,	position,		set_position,		STOCK_WRITE )
			PROPERTY( private, string,		token_string,	set_token_string,	STOCK_WRITE )
			PROPERTY( private, token,		token_type,		set_token_type,		STOCK_WRITE )
			PROPERTY( private, bool,        is_inside_tag,	set_inside_tag,		STOCK_WRITE )
			PROPERTY( private, int,			line_number,	set_line_number,	STOCK_WRITE )

		public:
			xml_scanner( const char* data );

			bool scan_next_token();
			bool scan_next_token( token tok );
			void must_scan_next( token tok );

		private:
			bool check_string( const char* c, bool peek = false );

			inline void increase_position()
			{
				m_position++;
			}
	};
}
