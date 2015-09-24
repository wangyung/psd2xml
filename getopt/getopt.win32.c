#define _INC_TCHAR /* prevent tchar.h to be included */

#include <stdlib.h> /* getenv() */

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(disable: 4996 4005) /* CRT insecure... */

#define _T(s) s
/* omit inconsistant dll linkage */
#ifndef getenv
#define getenv getenv
#endif

#include "getopt.c.h"
#include "getopt1.c.h"

/* map to wide char interfaces */
#define char wchar_t
#undef _T
#define _T(s) L ## s
#define strcmp	wcscmp
#define strncmp	wcsncmp
#define strlen	wcslen
#define fprintf	fwprintf
#undef getenv
#define getenv	_wgetenv
#define strchr wcschr
#define strcmp wcscmp
#define fprintf fwprintf
#define printf wprintf

/* public interfaces */
#define optarg woptarg
#define optind woptind
#define option woption
#define getopt wgetopt
#define getopt_long wgetopt_long
#define getopt_long_only wgetopt_long_only

/* getopt internals */
#define optopt woptopt
#define opterr wopterr
#define nextchar wnextchar
#define __getopt_initialized __wgetopt_initialized
#define my_index wmy_index
#define posixly_correct wposixly_correct
#define _getopt_initialize _wgetopt_initialize
#define exchange wexchange
#define ordering wordering
#define _getopt_data _wgetopt_data
#define getopt_data wgetopt_data
#define _getopt_internal _wgetopt_internal
#define _getopt_internal_r _wgetopt_internal_r
#define _getopt_long_r _wgetopt_long_r
#define _getopt_long_only_r _wgetopt_long_only_r

#define REQUIRE_ORDER _wREQUIRE_ORDER
#define PERMUTE _wPERMUTE
#define RETURN_IN_ORDER _wRETURN_IN_ORDER

#undef _GETOPT_H		/* force getopt.h included again */
#undef _GETOPT_INT_H	/* force getopt_int.h included again */
#include "getopt.c.h"
#include "getopt1.c.h"

#ifdef __cplusplus
}
#endif
