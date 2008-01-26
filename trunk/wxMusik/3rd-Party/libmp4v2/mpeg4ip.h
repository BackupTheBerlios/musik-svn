/* This file is a compatibility file that acts like mpeg4ip.h in the
   mpeg4ip distribution.

   This file is in the pubic domain.

   Noa Resare (noa@resare.com)
*/

#ifndef __MPEG4IP_INCLUDED__
#define __MPEG4IP_INCLUDED__

typedef void (*lib_message_func_t)(int loglevel,
								   const char *lib,
								   const char *fmt,
								   ...);

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef FALSE
#define FALSE 0
#endif



#ifdef WIN32
#include "mpeg4ip_win32.h"
#else /* UNIX */
#include "config.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/syslog.h>


#define UINT64_TO_DOUBLE(a) ((double)(a))

#if SIZEOF_LONG == 8
#define U64F  "lu"
#define X64F "lx"
#define D64F  "ld"
#define TO_U64(a) (a##LU)
#else
#define U64F  "llu"
#define X64F "llx"
#define D64F  "lld"
#define TO_U64(a) (a##LLU)
#endif


#ifdef HAVE_FPOS_T___POS
#define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)((fpos).__pos)
#define VAR_TO_FPOS(fpos, var) (fpos).__pos = (var)
#else
#define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)(fpos)
#define VAR_TO_FPOS(fpos, var) (fpos) = (var)
#endif
#endif /* else WIN32 */

#define CHECK_AND_FREE(a) if ((a) != NULL) { free((void *)(a)); (a) = NULL;}
#define NUM_ELEMENTS_IN_ARRAY(name) ((sizeof((name))) / (sizeof(*(name))))

#define U64  "%"U64F
#define X64 "%"X64F
#define D64  "%"D64F

#endif /* __MPEG4IP_INCLUDED__ */
