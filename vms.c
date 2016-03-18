/* Everything system specific goes in this file.  If you are looking to port
 * to some system the code currently doesn't work on, most if not all of your
 * problems should restricted to this file. */
/* some filenames changes: PR 950718 */
#include "defs.h"
#include "ext.h"

/* System specific defs */
#include "vms.h"

static char uname[30];
static unsigned long ef_write, ef_netout;
static int init_setterm = 1;

void
     checkex(unsigned long);
void
     net_ast(void);
void
     putch_ast(void);
void
     net_send_ast(void);
void
     getch_ast(void);
void
     child_ast(int);
int netflush();
unsigned long sys$assign();
unsigned long sys$waitfr();
unsigned long sys$clref();
unsigned long sys$trnlnm();
unsigned long sys$qiow();
unsigned long sys$qio();
unsigned long lib$disable_ctrl();
unsigned long lib$enable_ctrl();
unsigned long lib$signal();
unsigned long lib$get_ef();
unsigned long sys$getmsg();
unsigned long lib$getjpi();
unsigned long sys$cancel();
unsigned long sys$setef();
unsigned long lib$wait();
unsigned long lib$spawn();
unsigned long lib$getef();
unsigned long sys$cancel();
/* Wait for the next activity from either the user or the network.  The event
 * flag will be set when a QIO completes from either source. Return 1 for
 * user input pending, 2 for network input pending, 3 for both. */

int
waitnextevent()
{
    int result;

    /* Write out any pending output to screen. */
    fflush(NULL);

    /* Wait for completion of QIO. */
    checkex(sys$waitfr(ef));
    /* Reset flag. */
    sys$clref(ef);
    if (result = ((net_in != 0) << 1) | (car_in != 0))
	return (result);
    /* Should never reach here, but just making sure. */
    return (0);
}

/* Opens the temp file, sys$login:.bbstmp. If the BBSTMP logical name is
 * defined, it is used, otherwise the file is opened in sys$login, and is
 * called .bbstmp */

void
opentmpfile()
{
    $DESCRIPTOR(tab, "LNM$PROCESS_TABLE");
    $DESCRIPTOR(log, "YAWCBBSTMP");
    struct il {
	unsigned long start;
	struct LNM$ITEMLIST ilist;
	unsigned long end;
    }  list;
    int len;
    char setfile[80];

    list.start = 0;
    list.end = 0;
    list.ilist.lnm$item_buffer = tempfilename;
    list.ilist.lnm$item_buflen = 100;
    list.ilist.lnm$item_retlen = 0;
    list.ilist.lnm$item_flag = 0;
    list.ilist.lnm$item_code = LNM$_STRING;

    if (sys$trnlnm(0, &tab, &log, 0, 0) == 1)
	sys$trnlnm(0, &tab, &log, 0, &(list.ilist));
    else
	strcpy(tempfilename, "sys$login:.yawcbbstmp");
    if (!(tempfile = fopen(tempfilename, "a+")))
	fatalexit("\n[Error:  couldn't open temp file]\n");

    /* Set file version limit to 2. */
    fclose(tempfile);
    strcpy(setfile, "set file/ver=2/prot=(o:rwed,g,w) ");
    strcat(setfile, tempfilename);
    system(setfile);
    if (!(tempfile = fopen(tempfilename, "a+")))
	fatalexit("\n[Error:  couldn't open temp file]\n");

}


void
findhome()
{
    $DESCRIPTOR(userd, "");
    unsigned long item = JPI$_USERNAME;
    char *c;

    userd.dsc$a_pointer = uname;
    userd.dsc$w_length = 12;
    checkex(lib$getjpi(&item, 0, 0, 0, &userd, &userd.dsc$w_length));
    uname[userd.dsc$w_length] = 0;
    for (c = uname; *c != ' ' && *c != 0; c++);
    *c = 0;
    strcpy(user, uname);
}

/* Find .bbsrc file.  If a BBSRC logical exists it is used.  Otherwise we
 * look for .bbsrc in the sys$login directory. */
FILE *
findbbsrc()
{
    FILE *f;

    $DESCRIPTOR(tab, "LNM$PROCESS_TABLE");
    $DESCRIPTOR(log, "YAWCBBSRC");
    struct LNM$ITEMLIST *ilist;
    unsigned long items[2];
    char setfile[80];

    ilist = (struct LNM$ITEMLIST *)items;
    items[1] = 0;
    ilist->lnm$item_buffer = bbsrcname;
    ilist->lnm$item_buflen = 200;
    ilist->lnm$item_flag = 0;
    ilist->lnm$item_code = LNM$_STRING;
    if (sys$trnlnm(0, &tab, &log, 0, 0) == 1)
	sys$trnlnm(0, &tab, &log, 0, ilist);
    else
	strcpy(bbsrcname, "sys$login:.yawcbbsrc");

    f = openbbsrc();

    /* Set version limit to 2 on the bbsrc file. */
    if (f) {
	fclose(f);
	strcpy(setfile, "set file/ver=2/prot=(o:rwed,g,w) ");
	strcat(setfile, bbsrcname);
	system(setfile);
	f = openbbsrc();
    }
    return (f);
}

/* Open a connection to the bbs.  Defaults to BBSIPNUM port BBSPORT. but can
 * be overridden in the bbsrc file if/when the source to the ISCA BBS is
 * released and others start their own on different machines and/or ports.
 * NOTE: For VMS we only support IP numbers for now. */

void
connectbbs(how)
    int how;
{
    int status;
    unsigned long ip_num;

    $DESCRIPTOR(ipdev_mti, "INET0:");	/* Multinet IP device. */
    $DESCRIPTOR(ipdev_cmu, "IP:");	/* CMU IP device. */
    $DESCRIPTOR(ipdev_ucx, "BG0:");	/* UCX IP device. */
    $DESCRIPTOR(tab, "LNM$SYSTEM_TABLE");
    $DESCRIPTOR(log, "CMUIP_ROOT");
    $DESCRIPTOR(logm, "MULTINET_ROOT");
    $DESCRIPTOR(logw, "TWG$TCP");
    NetSB_Fields iosb;
    register struct hostent *host;
    struct sockaddr_in sa;
    int tcp = 0;
    long ucx_sock_def = (AF_INET << 24) + (SOCK_STREAM << 16);
    struct LNM$ITEMLIST rhost;

    /* Determine which TCP/IP we're running. */
    if (sys$assign(&ipdev_cmu, &ctlchan, 0, 0) == SS$_NORMAL)
	tcp = CMU;
    else if (sys$trnlnm(0, &tab, &logm, 0, 0) == 1 ||
	     sys$trnlnm(0, &tab, &logw, 0, 0) == 1)
	tcp = MULTINET;
    else if (sys$assign(&ipdev_ucx, &ctlchan, 0, 0) == SS$_NORMAL)
	tcp = UCX;

    if (tcp == 0) {
	printf("Error: could not identify network software.\n");
	printf("The client does not recognize your network software as one that\n");
	printf("it supports.\n");
	printf("This version of the client will not run on this system.\n");
	fflush(NULL);
	myexit();
    }
    /* Assign channel to network interface. */
    if (tcp == MULTINET)
	checkex(sys$assign(&ipdev_mti, &ctlchan, 0, 0));

    /* Get an event flag we can use to wait for input later. */
    checkex(lib$get_ef(&ef));
    sys$clref(ef);

    /* set default IP adres (Carl The Cub) */

    if (!*bbshost[0])
	strcpy(bbshost[0], BBSIPNUM);

    /* Convert the IP string to a number. */

    IPTOUL(bbshost[0], ip_num);

    if (tcp == MULTINET || tcp == UCX) {
	sa.sin_family = AF_INET;
	/* sa.sin_port = 0x1700;	/* Port 23; telnet */
	sa.sin_port = 0xB807;	/* Port 1976; YAWC's */
	sa.sin_addr.s_addr = ip_num;
    }
    /* Open the TCP connection. */
    if (tcp == MULTINET) {
	checkex(sys$qiow(0, ctlchan, IO$_SOCKET, &iosb, 0, 0,
			 AF_INET, SOCK_STREAM, 0, 0, 0, 0));
	checkex(sys$qiow(0, ctlchan, IO$_CONNECT, &iosb, 0, 0,
			 (struct sockaddr *)&sa, sizeof sa, 0, 0, 0, 0));
    }
    if (tcp == CMU) {
	checkex(sys$qiow(0, ctlchan, TCP$OPEN, &iosb, 0, 0,
			 &ip_num, BBSPORT, 0, 7, 0, 0));
    }
    if (tcp == UCX) {
	rhost.lnm$item_buflen = sizeof(sa);
	rhost.lnm$item_buffer = (char *)&sa;
	rhost.lnm$item_code = 0;
	rhost.lnm$item_flag = 0;
	checkex(sys$qiow(0, ctlchan, IO$_SETMODE, &iosb, 0, 0,
			 &ucx_sock_def, 0, 0, 0, 0, 0));
	checkex(sys$qiow(0, ctlchan, IO$_ACCESS, &iosb, 0, 0,
			 0, 0, &rhost, 0, 0, 0));
    }
    /* Check if connection was successful. */
    if (iosb.NSB$STATUS != SS$_NORMAL) {
	printf("Unable to open connection.\n");
	fflush(NULL);
	myexit();
    }
    printf("[Connected]\n");
    fflush(NULL);
}


/* Suspend the ISCA session.  For VMS this spawns a subprocess. */
void
suspend(void)
{
    int status;
    float delay = 0.5;
    unsigned long flags = SPAWN$WAIT;

    /* Spawn a DCL subprocess. */
    fflush(NULL);
    if (tbusy)
	sys$waitfr(ef_write);

    /* Interrupt input sequences. */
    checkex(sys$cancel(key_chan));
    tbusy = 0;
    resetterm();
    sys$clref(ef_child);
    checkex(lib$spawn(0, 0, 0, &flags, 0, 0, 0, 0, child_ast, 0));
}

/* All this signal stuff does very little for us in VMS; but it seems
 * harmless. */

/* Quits gracefully when we are given a HUP or STOP signal. */
sig_return_t
bye(void)
{
    myexit();
}


/* Initialize necessary signals */
void
siginit()
{
    oldrows = -1;

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
#endif
    /* signal(SIGHUP, bye); signal(SIGTERM, bye); */
#ifdef SIGWINCH
    signal(SIGWINCH, naws);
#endif
    signal(SIGALRM, sleeping);
    alarm(60);
}


/* Turn off signals now that we are ready to terminate */
void
sigoff()
{
#if 0
    signal(SIGALRM, SIG_IGN);
#ifdef SIGWINCH
    signal(SIGWINCH, SIG_IGN);
#endif
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
#endif
}


void
mysleep(sec)
    int sec;
{
    float f;

    f = (float)sec;
    lib$wait(&f);
}


/* Run the editor. */
void
run(char *comm, char *argu)
{
    char command[250];
    char stat[32];
    unsigned long flags = SPAWN$NOWAIT;

    $DESCRIPTOR(comm_line, "");

    /* We must close the temp file before attempting to edit it. */
    fclose(tempfile);
    if (argu)
	sprintf(command, "%s %s", comm, argu);
    else
	strcpy(command, comm);

    /* Pending input QIO would prevent terminal output. */
    fflush(NULL);
    if (tbusy)
	sys$waitfr(ef_write);
    checkex(sys$cancel(key_chan));
    tbusy = 0;
    comm_line.dsc$a_pointer = command;
    comm_line.dsc$w_length = strlen(command);

    /* Flush screen output. */
    sys$clref(ef_child);
    checkex(lib$spawn(&comm_line, 0, 0, &flags, 0, 0, 0, 0, child_ast, 1));
    childpid = 1;

    /* Fake completion of character IO so we don't lose a character. */
    char_wait = inkey();
    char_wait_flag = 1;
}


/* AST called when a subprocess (editor or shell) terminates. */
void
child_ast(int openflag)
{
    /* Restart terminal input QIO sequence. */
    checkex(sys$cancel(key_chan));
    tbusy = 0;
    setterm();
    childpid = 0;
    car_in = 1;
    sys$setef(ef);
    sys$setef(ef_child);

    /* Reopen tempfile if necessary. */
    if (openflag) {
	if (!(tempfile = fopen(tempfilename, "a+")))
	    fatalexit("\n[Error:  couldn't open temp file]\n");
	in_ch = 1;
    } else {
	printf("\n[Continue]\n");
	fflush(NULL);
	in_ch = 1;
	/* Send new window size if necessary */
	if (oldrows != getwindowsize() && oldrows != -1)
	    sendnaws();
    }
}


/* This command is used to send data over the network. */
void
net_send(unsigned char c)
{
    /* Check for full buffer. */
    if (current_out_net - outbuf_net == NET$BUFSIZ) {
	/* We must wait for QIO completion. */
	if (netbusy)
	    sys$waitfr(ef_netout);
	netflush();
	if (netbusy)
	    sys$waitfr(ef_netout);
	if (current_out_net != outbuf_net)
	    exit(0);
    }
    /* Store character in buffer. */
    *current_out_net++ = c;
    return;
}

/* AST called after an output to the network completes. */
void
net_send_ast(void)
{
    int status;
    unsigned char *old_c;

    /* Check IOSB for abnormal status. */
    status = iosb_send.NSB$STATUS;
    if (status != SS$_NORMAL) {
	/* Connection is dead. */
	printf("Network error (send).\n");
	fflush(NULL);
	if (tbusy)
	    sys$waitfr(ef_write);
	myexit();
    }
    netbusy = 0;

    /* If a request was issued while we were busy, do another QIO. */
    if (net_request && current_out_net != outbuf_net) {
	netbusy = 1;
	net_request = 0;
	old_c = current_out_net;
	current_out_net = outbuf_net;
	sys$clref(ef_netout);
	checkex(sys$qio(ef_netout, ctlchan, TCP$SEND,
			&iosb_send, net_send_ast, 0, outbuf_net,
			(old_c - outbuf_net), 0, 0, 0, 0));
    }
    net_request = 0;
}

/* Get a character from the network. */
int
net_rcv(void)
{
    static unsigned char chars[512];
    int status;
    unsigned char ch;
    static unsigned char *current = chars;

    /* Return next available character. */
    if (net_in > 1) {
	net_in--;
	return (*current++);
    }
    /* No more data; issue a new read QIO. */
    ch = *current;
    current = chars;
    net_in = 0;
    checkex(sys$qio(ef, ctlchan, TCP$RECEIVE, &iosb_rcv, net_ast, 0,
		    chars, 512, 0, 0, 0, 0));
    return (ch);
}


/* AST called when a keyboard input completes. */
void
getch_ast(void)
{
    /* Character typed.  Set the flag. */
    car_in = 1;
}

/* AST called when network input arrives. */
void
net_ast(void)
{
    int status;

    net_in = iosb_rcv.NSB$BYTE_COUNT;	/* # bytes received */
    status = iosb_rcv.NSB$STATUS;

    /* Check status.  0 bytes indicates a closed connection. */
    if (status != SS$_NORMAL || net_in == 0) {
	if (tbusy)
	    sys$waitfr(ef_write);
	sys$cancel(key_chan);
	printf("[Disconnected]\n");
	tbusy = 0;
	fflush(NULL);
	if (tbusy)
	    sys$waitfr(ef_write);
	myexit();
    }
}

/* Put character to terminal; */
void
myputchar(char c)
{
    if (c == 10)
	myputchar(13);		/* seems to be needed */

    /* Check for full buffer. */
    if (current_out - outbuf == TERM$BUFSIZ) {
	/* We must wait for QIO completion. */
	if (tbusy)
	    sys$waitfr(ef_write);
	fflush(NULL);
	if (tbusy)
	    sys$waitfr(ef_write);
	if (current_out != outbuf)
	    exit(0);
    }
    /* Store character in buffer. */
    *current_out++ = c;
    return;
}

/* Flush the network output buffer. */
int
netflush(void)
{
    int status;
    unsigned char *old_c;

    /* If a QIO is in progress, return and note that a request was made. */
    if (netbusy == 1) {
	net_request = 1;
	return;
    }
    /* Issue a QIO if buffer not empty. */
    if (current_out_net != outbuf_net) {
	netbusy = 1;
	old_c = current_out_net;
	current_out_net = outbuf_net;
	sys$clref(ef_netout);
	checkex(sys$qio(ef_netout, ctlchan, TCP$SEND, &iosb_send, net_send_ast, 0,
			outbuf_net, (old_c - outbuf_net), 0, 0, 0, 0));
    }
}

/* Flush the screen output buffer. */
int
fflush(FILE * f)
{
    int func, status;
    unsigned char *old_c;

    /* If a QIO is in progress, return and note that a request was made. */
    if (tbusy == 1) {
	request = 1;
	return;
    }
    /* Issue a QIO if buffer not empty. */
    if (current_out != outbuf) {
	func = IO$_WRITEVBLK;
	tbusy = 1;
	old_c = current_out;
	current_out = outbuf;
	sys$clref(ef_write);
	checkex(sys$qio(ef_write, key_chan, func, 0, putch_ast, 0,
			outbuf, (old_c - outbuf), 0, 0, 0, 0));
    }
}

/* AST executed upon completion of an output QIO to the screen. */
void
putch_ast(void)
{
    unsigned char *old_c;

    tbusy = 0;

    /* If a request was issued while we were busy, do another QIO. */
    if (request && current_out != outbuf) {
	tbusy = 1;
	request = 0;
	old_c = current_out;
	current_out = outbuf;
	sys$clref(ef_write);
	checkex(sys$qio(ef_write, key_chan, IO$_WRITEVBLK, 0, putch_ast, 0, outbuf,
			(old_c - outbuf), 0, 0, 0, 0));
    }
    request = 0;
}


/* New definitions for printf and fprintf to intercept calls. We do our own
 * IO */
int
#ifdef __STDC__
fprintf(FILE * fp, const char *fmt,...)
#else
fprintf(fp, fmt, va_alist)
    FILE *fp;
    const char *fmt;

va_dcl
#endif
{
    va_list ap;
    char s[5000];
    int len, i;

#ifdef __STDC__
    va_start(ap, fmt);
#else
    va_start(ap);
#endif

    len = vsprintf(s, fmt, ap);
    for (i = 0; i < len; i++)
	fputc(s[i], fp);
    return (0);
}


int
#ifdef __STDC__
printf(const char *fmt,...)
#else
printf(fmt, va_alist)
    const char *fmt;

va_dcl
#endif
{
    va_list ap;
    char s[5000];
    char *p;

#ifdef __STDC__
    va_start(ap, fmt);
#else
    va_start(ap);
#endif
    vsprintf(s, fmt, ap);
    /* Send a string to the screen. */
    for (p = s; (*p) != '\0'; p++)
	myputchar(*p);
    return (0);
}


/* Routine to check return status of system calls. Note:  some calls return
 * different things. (ex: clref) */
void
checkex(unsigned long status)
{

    $DESCRIPTOR(message, "");
    char mess[256];
    unsigned long stat1;

    if (status == SS$_NORMAL)
	return;
    /* Will be removed when i figure which ones are ok. */
    return;
    message.dsc$a_pointer = mess;
    message.dsc$w_length = 256;

    stat1 = sys$getmsg(status, &message.dsc$w_length, &message, 0, 0);
    if (stat1 != SS$_NORMAL)
	lib$signal(stat1);
    mess[message.dsc$w_length] = 0;
    printf("ERROR in system call\n%s\n", mess);
    fflush(NULL);
    myexit();
}

/* Initialization stuff specific to VMS. */
void
initialize(char *version)
{
    unsigned long mask = 0x02100000;

    $DESCRIPTOR(term, "SYS$COMMAND:");

    systemtype = 1;
    outbuf[0] = 0;
    outbuf_net[0] = 0;
    current_out = outbuf;
    current_out_net = outbuf_net;
    tbusy = 0;
    netbusy = 0;
    request = 0;
    net_request = 0;
    netifp = NULL;
    netofp = NULL;
    char_wait_flag = 0;
    shell[0] = 0;
    strcpy(myeditor, "edit");

    /* Assign terminal channel. */
    checkex(sys$assign(&term, &key_chan, 0, 0));
    lib$get_ef(&ef_child);
    lib$get_ef(&ef_write);
    lib$get_ef(&ef_netout);
    sys$clref(ef_write);
    sys$clref(ef_netout);
    sys$clref(ef_child);

    /* Disable CTRL_Y interruptions, and CTRL-T. */
    lib$disable_ctrl(&mask, 0);

    printf("\n\nClientPro for YAWC, DOC and ABC %s (VMS)\n\n", version);
    fflush(NULL);
}

static unsigned char saveterm[12];

getwindowsize()
{
    checkex(sys$qiow(0, key_chan, IO$_SENSEMODE, 0, 0, 0,
		     saveterm, 12, 0, 0, 0, 0));
    rows = saveterm[7];
    if (rows < 5 || rows > 100)
	rows = 24;
    return (rows);
}

/* If we receive a ctrl-x, use it. */
void
ctrlx_ast(void)
{
    /* Send a ctrl-x to client.  (VMS intercepts) */
    sys$setef(ef);
}


void
setterm(void)
{
    unsigned long maskctrl = 0x02100000;
    unsigned char tmpterm[12];
    unsigned char mask[8] = {0, 0, 0, 0, 0, 0, 0, 1};

    getwindowsize();
    memcpy(tmpterm, saveterm, 12);

    /* Set needed characteristics. (nothing right now) */
    tmpterm[4] |= 0;
    sys$qiow(0, key_chan, IO$_SETMODE, 0, 0, 0,
	     tmpterm, 12, 0, 0, 0, 0);

    /* Ask to be warned of ctrl-x */
    sys$qiow(0, key_chan, IO$_SETMODE | IO$M_OUTBAND | IO$M_INCLUDE, 0, 0, 0,
	     ctrlx_ast, mask, 0, 0, 0, 0);
    lib$disable_ctrl(&maskctrl, 0);
    if (init_setterm) {
	getchar();
	net_rcv();
	init_setterm = 0;
    }
}

void
term_sig(how)
    int how;
{
    /* to be done */
}

void
resetterm(void)
{
    unsigned long mask = 0x02100000;

    saveterm[4] &= 254;
    sys$qiow(0, key_chan, IO$_SETMODE, 0, 0, 0,
	     saveterm, 12, 0, 0, 0, 0);
    /* Reenable ctrl-y and ctrl-t */
    lib$enable_ctrl(&mask, 0);
}

/* Little functions that do little things. */
int
net_in_func(void)
{
    return (net_in);
}

int
car_in_func(void)
{
    return (car_in);
}

/* Our own version of fputc */
fputc(int c, FILE * f)
{
    char ch[2];

    if (f == stdout)
	myputchar(c);
    else {
	/* I know this is ugly. */
	ch[0] = c;
	ch[1] = 0;
	fputs(ch, f);
    }
}

#ifdef __ALPHA
#undef fgetc
#endif

int
fgetc(FILE * f)
{
    char *st;
    int ch;
    char temp;
    int status, func;

    if (f == stdin) {
	if (childpid)
	    return (0);		/* Shouldn't happen, but seems to happen. */
	/* Character is already waiting. */
	if (char_wait_flag == 1) {
	    char_wait_flag = 0;
	    sys$clref(ef);
	    return (char_wait);
	}
	ch = in_ch;
	car_in = 0;
	func = IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR;
	/* Issue a QIO to read a new character. */
	checkex(sys$qio(ef, key_chan, func, &iosb_getc, getch_ast, 0, &in_ch, 1, 0, 0, 0, 0));
	return (ch);
    } else {
	st = fgets((char *)&temp, 2, f);
	if (st == NULL)
	    /* End of file or error. */
	    return (-1);
	else
	    return ((int)temp);
    }
}

void
deinitialize(void)
{
    float del = 1.5;

    fflush(NULL);
    lib$wait(&del);
}

/* Purge the type-ahead buffer to prevent bad uploads to cause too much
 * damage. */
void
flush_input(int invalid)
{
    int func;
    unsigned char mask[8] = {0, 0, 0, 0, 0, 0, 0, 1};

    if (invalid / 2)
	mysleep(invalid / 2 < 3 ? invalid / 2 : 3);
    /* Cancel any pending QIOs, and issue a new one with the M_PURGE option */
    sys$cancel(key_chan);
    car_in = 0;
    func = IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR | IO$M_PURGE;
    checkex(sys$qio(ef, key_chan, func, &iosb_getc, getch_ast, 0, &in_ch, 1, 0, 0, 0, 0));

    /* Ask to be warned of ctrl-x */
    sys$qiow(0, key_chan, IO$_SETMODE | IO$M_OUTBAND | IO$M_INCLUDE, 0, 0, 0,
	     ctrlx_ast, mask, 0, 0, 0, 0);
    tbusy = 0;
    fflush(NULL);
}

int
sigpause(int i)
{
    unsigned long mask = 0x02100000;

    lib$enable_ctrl(&mask, 0);
    sys$waitfr(ef_child);
    childpid = 0;
    return (0);
}

void
truncbbsrc(int len)
{
    fclose(bbsrc);
}
