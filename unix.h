/* This is where all the system-specific #include files go, and all the
 * #ifdefs for portability to different Unix systems belong here and in
 * unix.c. */
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/ioctl.h>
#ifdef USE_TERM
#include <client.h>
#endif

#if defined(sun) && defined(unix) && !defined(FIONREAD) && !defined(__svr4__)
#define __svr4__
#endif

#if defined(hpux) || defined(_AIX) || defined(linux) || defined(sgi) || defined(__DGUX__) || defined(_AUX_SOURCE) || defined(AMIX) || defined(__svr4__) || defined(M_XENIX) || defined(_SEQUENT_)
#define USE_TERMIO
#ifdef M_XENIX
#define _IBCS2
#endif
#endif

#ifdef USE_TERMIO
#include <termio.h>
#else
#include <sgtty.h>
#endif

#ifdef _AIX
#include <sys/select.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

#if defined(AMIX) || defined(__svr4__)
#include <sys/filio.h>
#endif
