/*
 * Everything Unix/system specific goes in this file.  If you are looking to
 * port to some system the code currently doesn't work on, most if not all of
 * your problems should restricted to this file. 
 */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "defs.h"
#include "ext.h"
#include "unix.h"

#ifdef _AIX
#  define HANDLER void(*)(int)
#else
#  define HANDLER void(*)
#endif

static struct passwd *pw;
static int abortlogin = 0;
/*
 * Wait for the next activity from either the user or the network -- we ignore
 * the user while we have a child process running.  Returns 1 for user input
 * pending, 2 for network input pending, 3 for both. 
 */
int
waitnextevent()
{
fd_set  fdr;
int     result;
struct timeval timeout;
int	i;

  for (;;)
  {
    for (i=0; i<MAXSYS; i++)	/* scanning for movements in other sessions */
    {
      if(net[i] && !(sessionflags[i] & S_INCOMING_DATA) && (i != curr_bbs))
      {
        FD_ZERO(&fdr);
        FD_SET(net[i], &fdr);
	timeout.tv_sec = 0;
	timeout.tv_usec = 1;
	if (select(net[i] + 1, &fdr, 0, 0, &timeout) < 0)
	  if (errno == EINTR)
	    continue;
          else
	    fatalperror("\r\nLocal error: select");
	if(FD_ISSET(net[i], &fdr))
	{
	  printf("\7\n[Incoming data for session #%d received.]\n", i);
	  fflush(stdout);
	  sessionflags[i] |= S_INCOMING_DATA;
	}
      }
    }

    FD_ZERO(&fdr);
    if (!childpid && !check)
      FD_SET(0, &fdr);
    FD_SET(net[curr_bbs], &fdr);

    /*
     * KHTEST: this here might cause trouble...
     */

    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;

    if (select(net[curr_bbs] + 1, &fdr, 0, 0, &timeout) < 0)
      if (errno == EINTR)
	continue;
      else
	fatalperror("\r\nLocal error: select");

    if ((result = ((FD_ISSET(net[curr_bbs], &fdr) != 0) << 1 | (FD_ISSET(0, &fdr) != 0))))
      return (result);
  }
}


/*
 * Find the user's home directory (needed for .yawcbbsrc and .yawcbbstmp) 
 */
void
findhome()
{
  if ((pw = getpwuid(getuid())))
    strcpy(user, pw->pw_name);
  else if (getenv("USER"))
    strcpy(user, (char *)getenv("USER"));
  else
    fatalexit("Local error: can't find user name.");
}


/*
 * Locate the bbsrc file.  Usually is ~/.yawcbbsrc, can be overriden by
 * the BBSRC environment variable if it is set.
 * Returns a pointer to the file via openbbsrc(). 
 */
FILE   *
findbbsrc()
{
FILE *f;

  if (getenv("BBSRC"))
    strcpy(bbsrcname, (char *)getenv("BBSRC"));
  else if (pw)
    sprintf(bbsrcname, "%s/.yawcbbsrc", pw->pw_dir);
  else if (getenv("HOME"))
    sprintf(bbsrcname, "%s/.yawcbbsrc", (char *)getenv("HOME"));
  else
    fatalexit("Local error: can't locate home directory.");
  if ((f = fopen(bbsrcname, "r")) && chmod(bbsrcname, 0600) < 0)
    fprintf(stderr, "Warning: can't set access on bbsrc file.\n");
  if (f)
    fclose(f);
  return (openbbsrc());
}



/*
 * Truncates bbsrc file to the specified length. 
 */
void
truncbbsrc(len)
  int     len;
{
  /* Anyone know how to do this in SCO/Xenix?  If so, please let me know! */
#ifndef M_XENIX
  if (ftruncate(fileno(bbsrc), len) < 0)
    fatalexit("Local error: ftruncate");
#endif
}



/*
 * Opens the temp file, ~/.yawcbbstmp.  If the BBSTMP environment variable is
 * set, that file is used instead. 
 *
 * Now opens the specfile too, which is used for editing remote files locally.
 */
 
void
opentmpfile()
{
  if (getenv("BBSTMP"))
    {
      strcpy(tempfilename, (char *)getenv("BBSTMP"));
      sprintf(specfilename, "%s_edit", getenv("BBSTMP"));
    }
  else if (pw)
    {
      sprintf(tempfilename, "%s/.yawcbbstmp", pw->pw_dir);
      sprintf(specfilename, "%s/.yawcbbsedit", pw->pw_dir);
    }
  else if (getenv("HOME"))
    {
      sprintf(tempfilename, "%s/.yawcbbstmp", (char *)getenv("HOME"));
      sprintf(specfilename, "%s/.yawcbbsedit", (char *)getenv("HOME"));
    }
  else
    fatalexit("Local error: can't locate home directory.");

  if (!(tempfile = fopen(tempfilename, "a+")))
    fatalexit("Local error: couldn't open temp file.");
  if (chmod(tempfilename, 0600) < 0)
    fprintf(stderr, "Warning: can't set access on temp file.\n");

  if (!(specfile = fopen(specfilename, "a+")))
    fatalexit("Local error: couldn't open spec file.");
  if (chmod(specfilename, 0600) < 0)
    fprintf(stderr, "Warning: can't set access on spec file.\n");

}

void 
sigint_handler()
{
  abortlogin=1;
}

int real_connect(netFd, sad, szsa, how)
int netFd;
struct sockaddr * sad;
int szsa;
int how;
{
  int i=0;
  int j=-1;
  abortlogin = 0;
if(how)
  term_sig(1);
signal (SIGINT, (void *)sigint_handler);
ioctl(netFd,FIONBIO,1);
  while ((i++ < MAXTRIES) && (j<0))
  {
    do
    {
      j=connect(netFd, sad, szsa);
      last_errno = errno;
/*
      sleep(1);
*/
    }
    while ((j<0) && (last_errno == EWOULDBLOCK) && !abortlogin);

    if(abortlogin)
    {
      printf("[Aborting login attempt.]\n");
      return(j);
    }

    if(j<0)
    {
      if (debugging)
      {
	errno = last_errno;
        perror("YAWCclient: connect");
      }
      if(i<MAXTRIES)
        fprintf(stderr,"Failed to connect, trying again (%d more tr%s)...\n", MAXTRIES-i,
        ((MAXTRIES-i) == 1)?"y":"ies");
      netFd = socket(AF_INET, SOCK_STREAM, 0);
      if(netFd == -1)
      {
      j = netFd;
      i = MAXTRIES;
      }
    }
  }
ioctl(netFd,FIONBIO,0);
if(how)
  term_sig(0);
signal (SIGINT, SIG_IGN);
  return(j);
}

/*
 * Open a socket connection to the bbs.  Defaults to BBSHOST with port BBSPORT
 * but can be overridden
 * in the bbsrc file if/when the source to the ISCA BBS is released and others
 * start their own on different machines and/or ports. 
 */
void
connectbbs(how)
int how;
{
   register struct hostent *host;
   struct sockaddr_in sa;
   int j = 0;

   if(cmd_line_hostname==NULL)
     cmd_line_hostname=bbshost[curr_bbs];
   if(cmd_line_port==0)
     cmd_line_port=BBSPORT;
   
   if (!*cmd_line_hostname)
   {
     cmd_line_hostname = (char *)malloc(100);
     strcpy(cmd_line_hostname, BBSHOST);
   }
   fflush(stdout);

  /* is it ISCA, Gestalt or Shadow? if so use DOC mode instead of YAWC */
  if (cmd_line_port==1976 && (mystrstr("bbs.isca.uiowa.edu whip.isca.uiowa.edu 204.71.107.11 gestalt.isca.uiowa.edu gestalt.leepfrog.com 204.71.109.112 shadow.scc.iit.edu 198.87.165.39", cmd_line_hostname) != NULL))
  {
     cmd_line_port=23;
     sessionflags[curr_bbs] |= (S_TELNET + S_DOCCLIENT);
  }
  /* same for Constellation and their ABC code */
  if (cmd_line_port==1976 && (mystrstr("digitech.org lyra.digitech.org 199.44.69.52", cmd_line_hostname) != NULL))
  {
     cmd_line_port=4123;
     sessionflags[curr_bbs] |= (S_TELNET + S_DOCCLIENT);
  }

#ifdef USE_TERM
   net[curr_bbs] = connect_server(0);
   if (net[curr_bbs] < 0)
     fatalperror("Local error: connect_server");
   send_command(net[curr_bbs], C_PORT, 0, "%s:%d", cmd_line_hostname, cmd_line_port);
   send_command(net[curr_bbs], C_DUMB, 1, 0);
#else
   strncpy((char *)&sa, "", sizeof sa);
   sa.sin_family = AF_INET;
   sa.sin_port = htons(cmd_line_port);
   if (isdigit(*cmd_line_hostname))
     sa.sin_addr.s_addr = inet_addr(cmd_line_hostname);
   else if (!(host = gethostbyname(cmd_line_hostname)))
   {
     if(*cmd_line_hostname)
       fprintf(stderr, "\7Warning: Host '%s' not found.\nUsing default '%s'.\n\n",  cmd_line_hostname, BBSIPNUM);
      else
       fprintf(stderr, "\7Warning: Host not found. Using default '%s'.\n\n", BBSHOST);
     sleep(2);
     userflags |= NO_AUTO_LOGIN;
     sa.sin_addr.s_addr = inet_addr(BBSIPNUM);
   }
   else
     strncpy((char *)&sa.sin_addr, host->h_addr, sizeof sa.sin_addr);
   
   net[curr_bbs] = socket(AF_INET, SOCK_STREAM, 0);
   if (net[curr_bbs] < 0)
     fatalperror("Local error: socket");
#ifdef SOCKS
   if (Rconnect(net[curr_bbs], (struct sockaddr *) &sa, sizeof sa) < 0)
#else
     if ((j=real_connect(net[curr_bbs], (struct sockaddr *) &sa, sizeof sa, how)) < 0)
#endif
   {
     mybzero(macro[1], 72);
     if(check_for_other_sessions())
       if(abortlogin)
	 exit(1);
       else
         fatalperror("Network error: connect");
   }
#endif
   
   byte[curr_bbs] = bytep[curr_bbs] = targetbyte[curr_bbs] = 0;
 
   last_errno = 0;  
   if(j == 0)
     printf("[Connected]\n");
   fflush(stdout);
   
   /*
    * We let the stdio libraries handle buffering issues for us, that's what
    * they are provided for! 
    */
   
   connect_fp();
   cmd_line_hostname=NULL;
   cmd_line_port=0;
}


/*
 * Suspend the client.  Restores terminal to previous state before suspending,
 * puts it back in proper mode when client restarts, and checks if the window
 * size was changed while we were away. 
 */
void
suspend()
{
  resetterm();
  kill(0, SIGSTOP);
  setterm();
  printf("\r\n[Continue]\r\n");
  if (oldrows != getwindowsize() && oldrows != -1)
    sendnaws();
}


/*
 * Quits gracefully when we are given a HUP or STOP signal. 
sig_return_t
 */
void
bye()
{
  myexit();
}


/*
 * Handles a WINCH signal given when the window is resized 
 */
sig_return_t
naws()
{
  if (oldrows != -1)
    sendnaws();
#ifdef SIGWINCH
  signal(SIGWINCH, (HANDLER)naws);
#endif
}


/*
 * Handles the death of the child by doing a longjmp back to the function that
 * forked it.  We get spurious signals when the child is stopped, and to avoid
 * confusion we don't allow the child to be stopped -- therefore we attempt to
 * send a continue signal to the child here.  If it fails, we assume the child
 * did in fact die, and longjmp back to the function that forked it.  If it
 * doesn't fail, the child is restarted and the user is forced to exit the
 * child cleanly to get back into the main client. 
 */
sig_return_t
reapchild()
{
  wait(0);
  if (kill(childpid, SIGCONT) < 0)
    longjmp(jmpenv, 1);
}


/*
 * Initialize necessary signals 
 */
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
  signal(SIGHUP, (HANDLER)bye);
  signal(SIGTERM, (HANDLER)bye);
#ifdef SIGWINCH
  signal(SIGWINCH, (HANDLER)naws);
#endif
   signal(SIGALRM, (HANDLER)sleeping);
   alarm(60);
}


/*
 * Turn off signals now that we are ready to terminate 
 */
void
sigoff()
{
  signal(SIGALRM, SIG_IGN);
#ifdef SIGWINCH
  signal(SIGWINCH, SIG_IGN);
#endif
  signal(SIGHUP, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
}



static int savedterm = 0;

#ifdef USE_TERMIO
static struct termio saveterm;

#else
static struct sgttyb saveterm;
static struct tchars savetchars;
static struct ltchars saveltchars;
static int savelocalmode;

#endif


/*
 * Set terminal state to proper modes for running the client/bbs 
 */
void
setterm()
{
#ifdef USE_TERMIO
struct termio tmpterm;

#else
struct sgttyb tmpterm;
struct tchars tmptchars;
struct ltchars tmpltchars;
int tmplocalmode;

#endif

  getwindowsize();

#ifdef USE_TERMIO
  if (!savedterm)
    ioctl(0, TCGETA, &saveterm);
  tmpterm = saveterm;

  tmpterm.c_iflag &= ~(INLCR | IGNCR | ICRNL | ISTRIP);
  tmpterm.c_iflag |= IXOFF | IXON | IXANY;


/*
 * KHQUICKFIX
 *
 * tmpterm.c_oflag &= ~(ONLCR | OCRNL);
 *
 * that was how it was, the DOC'ish way. but I don't like those \r\n
 * combinations, so I'll try out my own output-thingymajig.
 *
 * ...and came to the conclusion that I won't do anything about the output-
 * flags at all. If you're on a strange computer/terminaltype, you might
 * have to AND or OR some of the c_oflags... *shrug* I doubt it though.
 */

  tmpterm.c_lflag &= ~(ISIG | ICANON | ECHO);

  tmpterm.c_cc[VMIN] = 1;
  tmpterm.c_cc[VTIME] = 0;
  ioctl(0, TCSETA, &tmpterm);
#else
  if (!savedterm)
    ioctl(0, TIOCGETP, (char *)&saveterm);
  tmpterm = saveterm;

/*
 * tmpterm.sg_flags &= ~(ECHO | CRMOD);
 */

  tmpterm.sg_flags &= ~ECHO;

  tmpterm.sg_flags |= CBREAK | TANDEM;
  ioctl(0, TIOCSETN, (char *)&tmpterm);
  if (!savedterm)
    ioctl(0, TIOCGETC, (char *)&savetchars);
  tmptchars = savetchars;
  tmptchars.t_intrc = tmptchars.t_quitc = tmptchars.t_eofc = tmptchars.t_brkc = -1;
  ioctl(0, TIOCSETC, (char *)&tmptchars);
  if (!savedterm)
    ioctl(0, TIOCGLTC, (char *)&saveltchars);
  tmpltchars = saveltchars;
  tmpltchars.t_suspc = tmpltchars.t_dsuspc = tmpltchars.t_rprntc = -1;
  ioctl(0, TIOCSLTC, (char *)&tmpltchars);
  if (!savedterm)
    ioctl(0, TIOCLGET, (char *)&savelocalmode);
  tmplocalmode = savelocalmode;
  tmplocalmode &= ~(LPRTERA | LCRTERA | LCRTKIL | LCTLECH | LPENDIN | LDECCTQ);
  tmplocalmode |= LCRTBS;
  ioctl(0, TIOCLSET, (char *)&tmplocalmode);
#endif
  savedterm = 1;
}

/*
 *  switch ^C on / off for connect() abortion
 */
void
term_sig(how)
int how;
{
#ifdef USE_TERMIO
  struct termio tmpterm;
#else
  struct sgttyb tmpterm;
  int tmplocalmode;
#endif

#ifdef USE_TERMIO
  ioctl(0, TCGETA, &tmpterm);
  if(how)
    tmpterm.c_lflag |= ISIG;
  else
    tmpterm.c_lflag &= ~ISIG;
  ioctl(0, TCSETA, &tmpterm);
#else
  ioctl(0, TIOCGETP, (char *)&tmpterm);
  if(how)
    tmpterm.sg_flags &= ~CBREAK;
  else
    tmpterm.sg_flags |= CBREAK;
  ioctl(0, TIOCSETN, (char *)&tmpterm);
#endif
}

/*
 * Reset the terminal to the previous state it was in when we started. 
 */
void
resetterm()
{
  if (!savedterm)
    return;
#ifdef USE_TERMIO
  ioctl(0, TCSETA, &saveterm);
#else
  ioctl(0, TIOCSETN, (char *)&saveterm);
  ioctl(0, TIOCSETC, (char *)&savetchars);
  ioctl(0, TIOCSLTC, (char *)&saveltchars);
  ioctl(0, TIOCLSET, (char *)&savelocalmode);
#endif
}


/*
 * Get the current window size. 
 */
int
getwindowsize()
{
#ifdef TIOCGWINSZ
struct winsize ws;

  if (ioctl(0, TIOCGWINSZ, (char *)&ws) < 0)
    return (rows = 24);
  else if ((rows = ws.ws_row) < 5 || rows > 100)
    return (rows = 24);
  else
    return (rows);
#else
  return (rows = 24);
#endif
}



void
mysleep(sec)
  int sec;
{
  sleep(sec);
}



/*
 * This function flushes the input buffer in the same manner as the BBS does.
 * By doing it on the client end we save the BBS the trouble of doing it, but
 * in general the same thing will happen on one end or the other, so you won't
 * speed things up at all by changing this, the sleep is there for your
 * protection to insure a cut and paste gone awry or line noise doesn't cause
 * you too much hassle of posting random garbage, changing your profile or
 * configuration or whatever. 
 */
void
flush_input(invalid)
  int invalid;
{
int     i;

  if (invalid / 2)
    mysleep(invalid / 2 < 3 ? invalid / 2 : 3);
#ifdef FIONREAD
  while (INPUT_LEFT(stdin) || (!ioctl(0, FIONREAD, &i) && i > 0))
#else
# ifdef TCFLSH
  i = 0;
  ioctl(0, TCFLSH, &i);
# endif
  while (INPUT_LEFT(stdin))
#endif
    (void)getchar();
}



/*
 * Run the command 'cmd' with argument 'arg'.  Used only for running the editor
 * right now.  In order to work properly with all the versions of Unix I've
 * tried to port this to so far without be overly complicated, I have to use a
 * setjmp to save the local stack context in this function, then longjmp back
 * here once I receive a signal from the child that it has terminated. So I
 * guess there actually IS a use for setjmp/longjmp after all! :-) 
 */
void
run(cmd, arg)
  char   *cmd;
  char   *arg;
{
  fflush(stdout);
  if (setjmp(jmpenv))
  {
    signal(SIGCHLD, SIG_DFL);
    if (childpid < 0)
    {
      childpid = 0;
      myexit();
    }
    else
    {
      setterm();
      childpid = 0;
    }
  }
  else
  {
    signal(SIGCHLD, (HANDLER)reapchild);
    resetterm();

    if (!(childpid = fork()))
    {
      execlp(cmd, cmd, arg, 0);
      perror("\nLocal error: exec");
      _exit(0);
    }
    else if (childpid > 0)
    {

      /*
       * Flush out anything in our stdio buffer -- it was copied to the child
       * process, we don't want it waiting for us when the child is done. 
       */
      flush_input(0);
      (void)inkey();
    }
    else
      fatalperror("Local error: fork");
  }
}



void
initialize(version)
  char   *version;
{
  int i;

  if (!isatty(0) || !isatty(1) || !isatty(2))
    exit(0);

  systemtype = 0;
#ifdef _IOFBF

  /*
   * I'm told pre revision 3 versions of SysV Unix may have the second and
   * third arguments to setvbuf() reversed.  Luckily for me, however, on almost
   * every system NULL == _IOFBF == 0!  If this causes problems (i.e. fflush()
   * causes a segmentation fault) an appropriate #ifdef will be needed for the
   * system in question. 
   */
  setvbuf(stdin, NULL, _IOFBF, 2048);
  setvbuf(stdout, NULL, _IOFBF, 4096);
#endif
#ifdef NeXT
  if (getenv("STUART"))
  {
struct winsize ws;

    ioctl(0, TIOCGWINSZ, (char *)&ws);
    printf("\033]1; BBS (%s)\033\\", mystrrchr((char *)ttyname(0), '/') + 1);
    printf("\033]2; BBS (%s) %dx%d\033\\", mystrrchr((char *)ttyname(0), '/') + 1, ws.ws_col, ws.ws_row);
    fflush(stdout);
  }
#endif
  printf("\nClientPro for YAWC, DOC and ABC %s (Unix)\n\n", version);
  fflush(stdout);
  if (getenv("SHELL"))
    strcpy(shell, (char *)getenv("SHELL"));
  else
    strcpy(shell, "/bin/sh");
  if (getenv("EDITOR"))
    strcpy(myeditor, (char *)getenv("EDITOR"));
  else
    strcpy(myeditor, "vi");

  curr_bbs = 0;
  netofp = NULL;

  for (i=0; i<MAXSYS; i++)
  {
    fps[i][0] = NULL;
    fps[i][1] = NULL;
  }
}



void
deinitialize()
{
char    tfile[100];

#ifdef NeXT
  if (getenv("STUART"))
  {
struct winsize ws;

    ioctl(0, TIOCGWINSZ, (char *)&ws);
    printf("\033]1; csh (%s)\033\\", mystrrchr((char *)ttyname(0), '/') + 1);
    printf("\033]2; (%s) %dx%d\033\\", mystrrchr((char *)ttyname(0), '/') + 1, ws.ws_col, ws.ws_row);
    fflush(stdout);
  }
#endif
  /* Get rid of ~ file emacs always leaves behind */
  strcpy(tfile, tempfilename);
  strcat(tfile, "~");
  unlink(tfile);
}
