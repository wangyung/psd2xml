#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#include "getopt.h"
#include "getopt_int.h" /* for reentrant APIs */

#define char wchar_t
#define option woption
#define _getopt_data _wgetopt_data
#define _getopt_internal _wgetopt_internal
#define _getopt_internal_r _wgetopt_internal_r
#define _getopt_long_r _wgetopt_long_r
#define _getopt_long_only_r _wgetopt_long_only_r
#define REQUIRE_ORDER _wREQUIRE_ORDER
#define PERMUTE _wPERMUTE
#define RETURN_IN_ORDER _wRETURN_IN_ORDER
#define optarg woptarg
#define optind woptind
#define getopt wgetopt
#define getopt_long wgetopt_long
#define getopt_long_only wgetopt_long_only

#undef _GETOPT_H		/* force getopt.h included again */
#include "getopt.h"
#undef _GETOPT_INT_H	/* force getopt_int.h included again */
#include "getopt_int.h"

#undef char
#undef option
#undef _getopt_data
#undef _getopt_internal
#undef _getopt_internal_r
#undef _getopt_long_r
#undef _getopt_long_only_r
#undef REQUIRE_ORDER
#undef PERMUTE
#undef RETURN_IN_ORDER
#undef optarg
#undef optind
#undef opterr
#undef optopt
#undef getopt
#undef getopt_long

#endif // _WIN32

#if defined(_WIN32) && defined(_UNICODE)
#define _toptarg woptarg
#define _toptind woptind
#define _toption woption
#define _tgetopt wgetopt
#define _tgetopt_long wgetopt_long
#define _tgetopt_long_only wgetopt_long_only
#define _tgetopt_long_r _wgetopt_long_r
#define _tgetopt_long_only_r _wgetopt_long_only_r
#define _tgetopt_data _wgetopt_data
#else
#define _toptarg optarg
#define _toptind optind
#define _toption option
#define _tgetopt getopt
#define _tgetopt_long getopt_long
#define _tgetopt_long_only getopt_long_only
#define _tgetopt_long_r _getopt_long_r
#define _tgetopt_long_only_r _getopt_long_only_r
#define _tgetopt_data _getopt_data
#endif

#ifdef __cplusplus
}
#endif
