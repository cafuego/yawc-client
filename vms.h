/* This is where all the system-specific #include files go, and all the
 * #ifdefs for portability to different systems belong here and in system.c. */

#define SPAWN$WAIT   0
#define SPAWN$NOWAIT 1
#define TERM$BUFSIZ  1000
#define NET$BUFSIZ  512
#define u_long  unsigned long
#define u_short unsigned short
#define caddr_t char *
#define CMU      1
#define MULTINET 2		/* Note: Wollongong is compatible */
#define UCX      3

/* Macro to convert an IP string to a 32 bit unsigned long. */
#define SWBI(I) (3-I)
#define IPTOUL( IPCHARPTR, IPLONG)\
  {\
  unsigned char *bits8by4 = (unsigned char *) (&IPLONG);\
  int tmpbits[4];\
  int tmpint;\
\
    if ( 4 == sscanf( IPCHARPTR, "%d.%d.%d.%d", &tmpbits[SWBI(3)], \
    &tmpbits[SWBI(2)], &tmpbits[SWBI(1)], &tmpbits[SWBI(0)]))\
      for ( tmpint=0; tmpint<4; tmpint++)\
        bits8by4[tmpint] = (unsigned char) tmpbits[tmpint];\
    else\
      IPLONG= 0;\
  }

/* CMU IP defs */
#include "vmstcp/tcpdefs.h"

/* Multinet definitions */
#include "vmstcp/in.h"
#include "vmstcp/socket.h"
#include "vmstcp/inetiodef.h"

/* Various VMS header files. */
#include <jpidef.h>
#include <ttdef.h>
#include <iodef.h>
#include <descrip.h>
#include <errno.h>
#include <ssdef.h>
#include <stdarg.h>


/* VMS specific variables */
unsigned short ctlchan;		/* Network IO channel  */
unsigned short key_chan;	/* Terminal IO channel */
unsigned long ef;		/* Event flag set when input arrives       */
unsigned long ef_child;		/* Event flag set when subprocess finishes */
NetSB_Fields iosb_send, iosb_rcv, iosb_getc;	/* IO status blocks */
FILE *netofp;			/* Dummy declaration for compatibility */
int in_ch;			/* Last character read from terminal */
int car_in;			/* True if a character is waiting */
int char_wait;			/* Fake character pushed in input */
int char_wait_flag;		/* True if fake input is present. */
int net_cara;			/* Input character from network   */
int net_in;			/* True if network input is available */

unsigned char outbuf[TERM$BUFSIZ];	/* Terminal output buffer. */
unsigned char outbuf_net[NET$BUFSIZ];	/* Terminal output buffer. */
unsigned char *current_out;	/* Pointer into output buffer */
unsigned char *current_out_net;	/* Pointer into output buffer */
int tbusy;			/* True if an output QIO is pending */
int netbusy;			/* True if an output QIO is pending */
int net_request;
int request;			/* True if we have received a flush request
				 * while a QIO was pending. */

/* Structure for searching logical names. */
#define LNM$_STRING 2		/* Translation string  */
struct LNM$ITEMLIST {
    unsigned short lnm$item_buflen;
    unsigned short lnm$item_code;
    char *lnm$item_buffer;
    char *lnm$item_retlen;
    int lnm$item_flag;
};
