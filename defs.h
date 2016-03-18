/*
 * This is where I've put all the #include files, to keep them separate in a
 * single location.  Pure C stuff goes here, the system-specific stuff is kept
 * over in unix.h. 
 */

#if (defined(__STDC__) && !defined(__HIGHC__) && !(defined(i386) && defined(MACH))) || (defined(sgi) && defined(__EXTENSIONS__))
#include <stdlib.h>
#define sig_return_t static void
#else
#define sig_return_t static int
#endif
#ifdef VAXC
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#include <errno.h>
extern int errno;

/* Boo hiss!  Amiga sucks!  Why do they have to be such idiots? */
#ifdef AMIX
#define INPUT_LEFT(__fp)	((__fp)->_cnt > 0)
#define NET_INPUT_LEFT()	(INPUT_LEFT(netifp))
#define netget()		(getc(netifp))
#define netput(__c)		(putc(__c, netofp))
#define netflush()		(fflush(netofp))
#else
#include "sysio.h"
#endif
#include "proto.h"

#define CTRL_D		4
#define CTRL_K		11
#define CTRL_N		14
#define CTRL_R		18
#define CTRL_U		21
#define CTRL_W		23
#define CTRL_X		24
#define TAB		9
#define CR		13
#define DEL		127

/* for yesno(): */
#define YES		1
#define NO		0

#define BBSHOST		"global.uibk.ac.at"
#define BBSIPNUM	"138.232.92.39"
#define BBSPORT		1976		/* don't change this one!	*/

/*
 * Theoretically,, you could aim it at Thanatos only on port 23. Never tried 
 * it though. Not a lot of point. But I thought I'd idly make a note of it.
 */

/*
 * here comes some of the options-flags (unsigned userflags)
 */

#define US_ANSICOLORS	1
#define US_NOFLASH	2
#define US_NOBOLDCOLORS 4
#define US_NOWARNING	8   /* don't get the kickout warning messages -Flint */
#define US_NEW_EDIT	16  /* use new edit function                  -Flint */

#define COLORCAPTURE	16  /* capture colours as well 			     */
#define LOGIN_ASAP	32  /* do auto-login as soon as possible 	     */
#define LOGGING_IN	128 /* auto login now in progress 		     */
#define NO_AUTO_LOGIN	256 /* no auto login even if username/pw known 	     */
#define US_ANSIDEFAULT  512 /* colours switched on/off by default at startup?*/

/* session flags */
#define S_INCOMING_DATA	1   /* is there unprinted data waiting?              */
#define S_LOGOUTW_RCV  	2   /* has a BBS logout warning been received?       */
#define S_LOGOUTW_WILL 	4   /* will the BBS send logout warnings?            */
#define S_TELNET       	8   /* Telnet mode                                   */
#define S_DOCCLIENT     16  /* ISCA (DOC client) mode                        */

/* editor flags */
#define E_EDIT          1    /* text is passed into the editor 		     */
#define E_2ENTEREND     2    /* user can end the text using <return> 2 times */
#define E_COLORS        4    /* YAWC colours allowed     		     */
#define E_INCLUDE_PROMPT 8   /* print a prompt on each line		     */

#define DEBUGFILE "yawc.debug"	/* guess what it is ;)  hint: see -d switch  */

#define MAXTRIES 2       /* re-try this number of times if connect() fails   */

#define ENCRYPT 1	/* for DES routines */
#define DECRYPT 0

#define TELNET (sessionflags[curr_bbs] & S_TELNET)
#define DOC (sessionflags[curr_bbs] & S_DOCCLIENT)

#define MAXSYS 37	/* max. amount of BBSs - [A-Z,0-9,!] */
#define MAXFRIENDS 400	/* max. friendlist length */
#define MAXENEMIES 80	/* max. friendlist length */
#define MAXWHO 1600	/* max. wholist entries (1200 users + 400 friends) */
