#pragma once

#define COBALT_NAMESPACE namespace cbl

#ifdef __GNUC__
# define CBL_UNUSED __attribute__(( unused ))
#else
# define CBL_UNUSED
#endif