#pragma once
#include "containers.h"
#include "string.h"

namespace cbl
{
	class string_list : public list<string>
	{
		public:
			string_list();
			string_list( std::initializer_list<string> vals );
			string join( string delim = " " );
	};
}