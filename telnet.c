/*
 * This handles the information flowing into the client from the BBS.  It
 * basically understands a very limited subset of the telnet protocol (enough
 * to "fake it" with the BBS) with some extensions to allow the extra features
 * the client provides.  The telnet stuff is unimportant, but it'll confuse the
 * BBS if you alter it. 
 *
 *  / KH says: umm, yeah, at Isca perhaps, but YAWC doesn't work like that: \
 * <  YAWC has the CLient-connection on a different port (1976), which lets  >
 *  \ us get rid of everything telnet'ish.                                  /
 *
 * The client tells the BBS it is a client rather a telnet program when it first
 * connects, after that the BBS acts differently, letting the client know when
 * something in particular is being done (entering an X message, posting, etc.)
 * to allow the client to handle that however it wants, as well as providing
 * extra protocol for the start/end of messages/X's and the special who list. 
 *
 * KH says: read this following carefully! it's important...
 * --------
 *
 * This is made more complex by the fact that the client doesn't know it should
 * handle (for example) an X message differently until it receives word from
 * the BBS that an X message should be entered -- but by this time the client
 * may have already sent some of the X message over the network instead of
 * gathering it up locally.  So when the BBS tells the client to go into the
 * local X message mode it also tells the client how many bytes have been
 * passed to it (they count them in the same manner) and throws away the excess
 * on its side.  The client has buffered this excess on its side and therefore
 * make it available locally instead of having it lost forever.  Boy, that was
 * a pisser when I realized I'd have to do that! 
 *
 *
 * KHThoughts: xmsgflag and xmsgnow seems to do the exact same job... ?
 *
 */
#include <stdio.h>

#include "defs.h"
#include "ext.h"
#include "telnet.h"

/*
   Flint: I had to exclude parts from telrcv() to (a) shorten the routine so
   _all_ compilers could optimize the function, and (b) to have the DOC-style
   wholist handling back if in DOC mode. That's why these variables have been
   made global for this module. 
*/

    static int state = TS_DATA;	/* Current state of telnet state machine */
    static unsigned char buf[80];	/* Generic buffer		*/
    static int bufp = 0;	/* Pointer into generic buffer	*/
    static char postbuf[160];	/* Buffer for post header (for kill files) */
    static int postbufp = 0;	/* Pointer for post header buffer	*/
    static int postflag = 0;	/* True if post header being received	*/
    static char xmsgbuf[160];	/* Buffer for X message header		*/
    static int xmsgbufp = 0;	/* Pointer for X message header buffer	*/
    static int xmsgflag = 0;	/* True if X message header being received */
    static int postnow = 0;	/* True while we are receiving a post */
    static int postwas = 0;	/* True for a line after a post received */
    static int xmsgnow = 0;	/* True while we are receiving an X message */
    static int moreflag = 0;	/* True while we are inside a MORE prompt */
    static int numposts = 0;	/* Count of # of posts we've received so far */
    static int colorflag = 0;	/* if set, next character will be color()ed */
    static int fileflag = 0;	/* True if file is being received	*/
    static int updateflag = 0;	/* True if update-chars are received.	*/
    static int wholistflag = 0;	/* True if wholist is being received.	*/
    static char wholistbuf[140];/* Buffer for one wholist-line	*/
    static int current_sb = 0;	/* which subnegotiation so we have right now? */
    int i;
    char    junk[160];
    char   *sp;
    char * temp;
    char Testname[41];
    /* DOC extensions */
    static unsigned char who[21];   /* Buffer for current name in who list */
    static int whop = 0;            /* Pointer into who array */
    static int col = 0;           /* Current column (for formatting who list */
    char    work[80];
    unsigned char saveinfo[60][60]; /* added saved info array */
    int savewhop;              /* pointer to end of saved who list */
    int escape_seq_state = 0;
    char escape_seq[20] = "";


/*************************************************************************
*
* decolorize()
*
* remove control codes (optionally including colors) for capturing
*
* buf: string to be processes (will be changed)
* how: 0 = remove YAWC style colors as well, 1 = only remove control codes
*
**************************************************************************/

void decolorize(buf, how)
char *buf;
int how;
{
  char tmpbuf[160];
  int i,j=0;
  for (i=0; i<strlen(buf); i++)
    if(!how && buf[i]==1)
      i++;
    else
      if((buf[i] >= 32 || buf[i] == TAB || buf[i] == 1 
         || buf[i] == '\n' || buf[i] == '\r') && (buf[i] < DEL))
      tmpbuf[j++]=buf[i];
  tmpbuf[j]=0;
  strcpy(buf, tmpbuf);
}

/**************************************************************************
*
* process_YAWC_wholist()
*
* handles YAWC-BBS style wholist lines
*
**************************************************************************/

void process_YAWC_wholist(c)
  register int c;
{
  wholistbuf[wholistbufp++] = '\n';
  wholistbuf[wholistbufp] = 0;	/* end the string there */
  wholistbufp = 0;
  
  /*
   * here's a KHaglund'ish hack: a ^D (Ctrl-D) is sent right
   * before the name so that we know where it is. (the name
   * that is)
   * 
   * Ew! No! We do things the Monolith-Thanatos-Jassyca way, now.
   * Flint, MLPT and I reckoned this was a much better way of
   * marking names out.
   * 
   * Trouble is, it's a little confusing to do in the wholist,
   * so Thanatos doesn't.
   */
  
  if(NULL!=(sp=mystrstr(wholistbuf, "\1n")))
    {
       char * temp;
       char Tempname[21];
       
       sp+=2;
       if(NULL!=(temp=mystrstr(sp, "\1N")))
	 {
	    strncpy(Tempname, sp, temp-sp);
	    strcpy(wholist[(wholistflag-1)], sp);
	 }
    }
  else
    {
       /*
	* If we can't find the correct tags, we try doing it
	* the old YAWC way.
	*/
       sp = (char *)mystrchr(wholistbuf, '\4');
       if(sp == NULL)
	 {
	    /*
	     * Still didn't work.
	     * So we try the worst kludge method yet,
	     * which is ridiculous.
	     */
	    sp = (char *)mystrchr(wholistbuf, '\r');
	 }
       if(sp != NULL)
	 {
	    sp = strncpy(wholist[(wholistflag-1)], sp+1, 20);
	    
	    /*
	     * we have now got a 20 character-string with spaces after
	     * the real name, so we'll remove 'em:
	     */
	    
	    while((sp = (char *)mystrchr(sp, ' ')) != NULL)
	      {
		 sp++;
		 if(*sp == ' ')
		   {
		      *(sp-1) = 0;
		      break;
		   }
	      }
	    
	    wholistflag++;	/* advance		*/
	 }
    }
  /* now filter out the name marking codes */
  if((sp = (char *)mystrchr(wholistbuf, '\4')) != NULL)
    for (; *sp; sp++)
      *sp=*(sp+1);
  if((sp = (char *)mystrchr(wholistbuf, '\r')) != NULL)
    for (; *sp; sp++)
      *sp=*(sp+1);

  colorize(wholistbuf);
  if (capture)	/* Flint-addition */
  {
    decolorize(wholistbuf, userflags&COLORCAPTURE);
    fprintf(tempfile, "%s", wholistbuf);
  }
}

/**************************************************************************
*
* friend_search()
*
* finds a name in the DOC wholist; entry on success; -1 on failure
* used by process_DOC_wholist().
*
**************************************************************************/

int friend_search(nm)
char *nm;
{
  for(i=0; i<MAXFRIENDS && friend[i]; i++)
    if(strcmp(friend[i], nm) == 0)
      return (i);
  return (-1);
}

/**************************************************************************
*
* process_DOC_wholist()
*
* handles DOC-BBS style wholist data
* base code taken from IO ERROR's ISCA client.
*
**************************************************************************/

void process_DOC_wholist(c)
register int c;
{
  /* handle processing of who list */
  if (!(who[whop++] = c))
    /* end of who list transfer */
    if (whop == 1)
    {
      if (wholistflag == 1)
      {
        if (savewhop)
  	  printf("Your friends online (old)\r\n\n");
        for (; col++ < savewhop;)
        {
  	  strcpy(junk, (char *)savewho[col - 1] + 1);
	  sprintf(work, /* "@Y%c%-20s@R%d:%02d@G - @C%s\r\n", */
/*
	  "%c%c%-19s%d:%02d - %s\r\n",
	  *junk & 0x80 ? '*' : '.', *junk & 0x7f,
	  junk + 1, *savewho[col - 1] / 60, *savewho[col - 1] % 60,
	  saveinfo[col-1]);
*/
	  "%c%c%-19s%d:%02d\n",
	  *junk & 0x80 ? '*' : '.', *junk & 0x7f,
	  junk + 1, *savewho[col - 1] / 60, *savewho[col - 1] % 60);
	  colorize(work);
        }
        col--;
      }
      /* wholist[wholistflag+1][0] = 0; */
      wholistflag = whop = col = 0;

    }
    else
    {
      if (wholistflag++ == 1)
      {
        savewhop = 0;
	wholistflag = friends2tab()+1;
      }
      /* output name and info if user is on our 'friend' list */
      strcpy(junk, (char *)who + 1);
      *junk &= 0x7f;
      if ((c = friend_search(junk)) != -1)
      {
	if (!col++)
	  printf("Your friends online (new)\r\n\n");
	--*who;
	if (col <= 60)
	{
/*
	  sprintf(work, (userflags & US_ANSICOLORS) ? "@Y%c%-20s@R%d:%02d@G - @C%s\r\n" :
	      "%c%-20s%d:%02d - %s\r\n", who[1] & 0x80 ? '*' : '.', junk,
	      *who / 60, *who % 60, info[c]);
*/
	  sprintf(work, (userflags & US_ANSICOLORS) ? "\1y%c%-20s\1r%d:%02d\1g\r\n" :
	      "%c%-20s%d:%02d\n", who[1] & 0x80 ? '*' : '.', junk,
	      *who / 60, *who % 60);
	  colorize(work);
	  *savewho[savewhop] = *who;
	  strcpy((char *)savewho[savewhop] + 1, (char *)who + 1);
/*
	  strcpy((char *)saveinfo[savewhop], info[c]);
*/
	  savewhop++;
	}
      }
      whop = 0;
      strcpy(wholist[wholistflag], junk); /* fill TAB table */
    }
}

/**************************************************************************
*
* handle_IAC()
*
* handles IAC (Is A Command) codes
* excluded from telrcv() for some optimizer's sake. *sighs*
*
**************************************************************************/

void handle_IAC(c)
register int c;
{
  switch (c)
  {

    /*
     * This is sent/received when the BBS thinks the client is
     * 'inactive' as per the normal 10 minute limit in the BBS -- the
     * actual inactive time limit for a client is an hour, this is
     * done to make sure the client is still alive on this end so dead
     * connections can be timed out and not be ghosted for an hour. 
     */

   case CLIENT:
     if(debugging)
       printf("[BBS: IAC CLIENT]\n");
     state = TS_DATA;
     if(idletime<6 || DOC)
       {
	  netput(IAC);
	  netput(CLIENT);
	  /*
	   * Old style.
	   * Not sure if it's even sent, actually.
	   */
       }
     break;

    case WHO_S:
      state = TS_DATA;
      wholistflag = friends2tab() + 2;
      break;

    case WHO_E:
      state = TS_DATA;
      wholist[wholistflag-1][0] = 0;
      wholistflag = 0;
      break;

    case G_POST:                /* get post			*/
    case G_LINES:               /* get five lines (X or profile) */
    case G_NAME:                /* get name			*/
    case G_STR:                 /* get string			*/
    case G_STRWR:               /* get string, wordwrapped	*/
    case CONFIG:                /* do configuration		*/
      if (DOC && (c == S_WHO))  /* DOC wholist start */
      {
	state = TS_DATA;
	wholistflag = 1;
      }
      else
      {
	state = TS_GET;
	buf[bufp++] = c;
      }
      break;

    /*
     * This code is used by the bbs to signal the client to synchronize
     * its count of the current byte we are on.  We then send back a
     * START to the bbs so it can synchronize with us.  NOTE:  If it
     * becomes necessary in the future to introduce incompatible
     * changes in the BBS and the client, this will be how the BBS can
     * differentiate between new clients that understand the new stuff,
     * and old clients which the BBS will probably still want to be
     * compatible with. Instead of START a new code can be sent to let
     * the BBS know we understand the new stuff and will operate
     * accordingly.  If there are multiple different versions of the
     * BBS to worry about as well (gag!) then more logic would be
     * needed, I refuse to worry about this case, if it comes about
     * it'll be after I ever have to worry about or maintain that BBS
     * code! 
     */
     
    case START:
      state = TS_DATA;

      targetbyte[curr_bbs] = byte[curr_bbs];

      byte[curr_bbs] = bytep[curr_bbs] = 1;
      numposts = 1;
      bufp = 0;

      /*
       * netput(IAC);
       * netput(START);
       *
       * KHaglund says: currently not necessary.
       */

      if (sessionflags[curr_bbs] & S_DOCCLIENT)
      {
	netput(IAC);
	netput(START3);
      }
      break;

    case POST_S:                /* Start of post transfer	*/
      state = TS_DATA;
      postflag = postnow = 1;
      break;

    case POST_E:                /* End of post transfer		*/
      state = TS_DATA;
      numposts++;
      postflag = postbufp = postnow = 0;
      postwas = 1;
      break;

    case MORE_M:                /* More prompt marker		*/
      state = TS_DATA;
      moreflag ^= 1;
      break;

    case EDIT_S:		/* Edit specfilename		*/
      run(editor, specfilename);
      sendblock();
      rewind(specfile);
      for(;;)
	{
	  if((i = getc(specfile))<0)
	    break;
	  putc(i, netofp);
	  byte[curr_bbs]++;
	}
      putc('\0', netofp);
      byte[curr_bbs]++;
      break;

    case FILE_S:		/* Start of file transfer	*/
      state = TS_DATA;
      freopen(specfilename, "w+", specfile);
      fileflag = 1;
      break;

    case FILE_E:		/* End of file transfer		*/
      state = TS_DATA;
      fflush(specfile);
      fileflag = 0;
      break;

    case UPDATE:		/* KH-addition			*/
      state = TS_DATA;
      updateflag = 1;
      break;

    case XMSG_S:                /* Start of X message transfer	*/
      state = TS_DATA;
      xmsgflag = xmsgnow = 1;
      break;

    case XMSG_E:                /* End of X message transfer	*/
      state = TS_DATA;
      if((away & 1) && (mystrstr(xmsgbuf, "This message was automatically generated; please do not reply.") == NULL))
      {
	strncpy(&alt_away[2], Testname, strlen(Testname));
	macron = -1;
	macrop = 1;
      }
      xmsgflag = xmsgbufp = xmsgnow = 0;
      break;

      /* telnet DO/DONT/WILL/WONT option negotiation commands (ignored) */
    case DO:
      state=TS_DO;
      break;
     
    case DONT:
     state=TS_DONT;
     break;
     
    case WILL:
     state=TS_WILL;
     break;
     
    case WONT:
      state = TS_WONT;
      break;
     
   case SB:
     state=TS_SB;
     break;
      
    case TELOPT_NAWS:	       /* Another ikky CLient kludge... */
      oldrows=0; 	       /* Force NAWS. */
      sendnaws();
      if(debugging)
	printf("[BBS: IAC TELOPT_NAWS]\n");
      break;

    default:
      state = TS_DATA;
      break;
  }
}

/**************************************************************************
*
* handle_lines()
*
* handles complete received lines.
* excluded from telrcv() for some optimizer's sake. *sighs*
*
**************************************************************************/

void handle_lines(c)
register int c;
{
  /*
   * we are receiving an X message header; check if an enemy
   * is the author, if so, kill the X.
   */
   
  if (xmsgflag > 0)
    /* ignore the initial linefeed (\r\n pair) */
  {
    if (xmsgbufp < 10)
    {
      xmsgbuf[xmsgbufp++] = c;
    }
    else
    {
      int i;
      /* end of header line reached -- do kill file processing */
      Testname[0]=0;
      xmsgbuf[xmsgbufp] = 0;
      xmsgbufp = 0;
/* printf("<<%s>>\n", xmsgbuf); */
      if((sp = mystrstr(xmsgbuf, "\1n")))
      {
        sp+=2;
      }
      if(sp!=NULL)
      {
	if (!(temp=mystrstr(sp, "\1N")))
	   sp=NULL;
	strncpy(Testname, sp, temp-sp);
	Testname[temp-sp]=0;
      }
      if(strlen(Testname) == 0)
      {
	if((sp = mystrstr(xmsgbuf, " from ")))
	{
	  sp += 6;
	  while (*sp == 1) /* YAWC color codes */
	    sp +=2;
	  strncpy(Testname, sp, 20);
	  Testname[20]=0;
	  if((sp = mystrstr(Testname, " at ")))
	  {
	    while (*(sp-2) == 1)
	      sp-=2;
	    *sp=0;
	  }
	}
      }

      for(i=0; i<MAXWHO && sp; i++)
      {
        if(!strcmp(wholist[i], Testname))
	  break;
	if(wholist[i][0]==0)
	{
	  strcpy(wholist[i], Testname);
	  break;
	 }
       }
     
       /*
        * it was (sp + 6) below here earlier, but the colorcodes
        * should make it (sp + 8) if I'm not mistaken.
        */
     
        for (c = 0; sp && c < enemyp; c++)
          if (!strcmp(Testname, enemy[c]))
          {
	    printf("\r\n[X message by %s killed]\r\n", enemy[c]);
	    xmsgflag = xmsgnow = -1;
	  }

	  if (xmsgflag > 0)
	  {
	    xmsgflag = 0;
	    colorize(xmsgbuf);
	    putchar('\n');
	    if (capture)
	    {
	      decolorize(xmsgbuf, userflags&COLORCAPTURE);
	      fprintf(tempfile, "\n%s\n", xmsgbuf + 1);
	    }
	  }
    }
  }
   
  /*
   * we have just received a post header -- do kill processing
   *
   * KHComment: not made working for YAWC yet.
   * 
   * DWD Comment: Trust me. It will. :~)
   * I've not handled it yet, but it's a matter of time.
   *
   * Flint Comment: Indeed - now it works. =)
   */
   
  else if (postflag > 0)
    /* ignore the (possible) initial linefeed (\r\n pair) */
    {
    if (postbufp < 50)
      {
        postbuf[postbufp++] = '\r';
        postbuf[postbufp++] = '\n';
      }
      else
      {
        Testname[0]=0;
	     
	postbuf[postbufp] = 0;
	strcpy(junk, postbuf);
/*
	strcpy(saveheader, postbuf);
	strcat(saveheader, "\r\n");
*/
	sp = mystrstr(postbuf, "\1n");
	if(sp!=NULL)
	{
	  temp=mystrstr(sp+2, "\1N");
	  strncpy(Testname, sp+2, temp-sp-2);
	  Testname[temp-sp-2]=0;
	}
	if(strlen(Testname) == 0)
	{
	  if((sp = mystrstr(postbuf, " from ")))
	  {
	    sp += 6;
	    while (*sp == 1) /* YAWC color codes */
	      sp +=2;
	    while (*sp == 27) /* ESC color codes */
	      sp +=5;
	    strncpy(Testname, sp, 40);
	    Testname[40]=0;

	    sp = Testname;
	    sp += (strlen(Testname)-1);
	    while (*(sp-5) == 27)
	      sp-=5;
	    *sp=0;

	  }
	}

	for (c = 0; sp && c < enemyp; c++)
	  if (!strcmp(Testname, enemy[c]))
	  {
	    printf("%s[Post by %s killed]\r\n", *postbuf == '\n' ? "\r\n" : "", enemy[c]);
	    postflag = postnow = -1;
	    if (DOC)
	    {
   	      netput(IAC);
	      netput(POST_K);
	      netput(numposts & 0xff);
	      netput(17);
	    }
	    netflush();
	  }
	  Testname[0]=0;

	  if (postflag > 0)
	  {
	    postflag = 0;
	    sprintf(colst, "%s\r\n", junk);
 	    colorize (colst);
	    if (capture)
	    {
	      decolorize(junk, userflags&COLORCAPTURE);
	      fprintf(tempfile, "\n%s", junk);
	    }
          }
	  postbufp = 0;
      }
    }
   
    /*
     * we're receiving the wholist line by line		-KHaglund
     */
   
    else if (wholistflag > 0)
      {
        if(!DOC)
 	  process_YAWC_wholist(c); /* the DOC routine works character-wise */
      }

    else if (!xmsgflag && !postflag && (c != -1))
      {
        putchar(c);
        if(capture) 
          if (putc(c, tempfile) < 0)
	  tempfileerror();
      }
}

/**************************************************************************
*
* handle_GET()
*
* handles GET (local mode) codes.
* excluded from telrcv() for some optimizer's sake. *sighs*
*
**************************************************************************/

int handle_GET(c)
register int c;
{
        buf[bufp++] = c;

        if (bufp == 5)
          {
            targetbyte[curr_bbs] = byte[curr_bbs];

	    /* Decode the bbs' idea of what the current byte is */

            byte[curr_bbs] = bytep[curr_bbs] = ((long)buf[2] << 16) +
		((long)buf[3] << 8) + buf[4];
    
            /*
             * If we are more out of sync than our buffer size, we can't
	     * recover.  If we are out of sync but not so far out of sync we
	     * haven't overrun our buffers, we just go back into our buffer
	     * and find out what we erroneously sent over the network, and
	     * reuse it.
	     *
	     * (the (char) save can contain 1000 characters...)
	     */

	     /*
	      printf("\nclientbyte : %ld\nbbsbyte: %ld\n", targetbyte[curr_bbs]
		      , byte[curr_bbs]);
	     */
	
	    if( targetbyte[curr_bbs] < byte[curr_bbs] )
              {
	        if (debugging)
		  printf("\007*** BEEP: A problem has occured when the CLient communicated with the BBS! <<%ld>>***\n", targetbyte[curr_bbs]);
		targetbyte[curr_bbs] = byte[curr_bbs];
	      }
	  
            if (byte[curr_bbs] < targetbyte[curr_bbs] - (int)(sizeof save[curr_bbs]) - 1)
              printf("\r\n[Error:  characters lost during transmission]\r\n");


	    state = TS_DATA;
	    bufp = 0;

	    inputmode = 1;
	    switch (*buf)
              {
                case G_POST:                /* get post */
                  if (posting)
                    {
		      printf("Local error: already posting?!\n");
                      check = 0;
                      return (-1);
                    }
                  makemessage(buf[1]);
                  break;
      
                case G_LINES:               /* get some lines (Xmsg,profile) */
                  get_some_lines(buf[1]);
                  break;
      
                case G_NAME:		/* get name			*/
		  sp = get_name(buf[1]);
                  sendblock();

		  for (i = 0; sp[i]; i++)
		    netput(sp[i]);
		    
                  netput('\n');
                  byte[curr_bbs] += i + 1;
		  break;
      
                case G_STR:		/* get string			*/
                case G_STRWR:		/* get string, wordwrapped	*/

		  /*
		   * this here means you cannot get a string longer than
		   * 255 chars, since buf[1] is a character...
		   */

		  if(*buf == G_STRWR)
                    get_string(buf[1], (char *)buf, 1, 1);
		  else
		    get_string(buf[1], (char *)buf, 999/* -1 */, 1);

		  sendblock();

		  for(i = 0; buf[i]; i++)
		    netput(buf[i]);

		  netput('\n');
                  byte[curr_bbs] += i + 1;
                  break;
      
                case CONFIG:                /* do configuration */
                  sendblock();
                  configbbsrc();
                  netput('\n');
                  byte[curr_bbs]++;
                  break;
	      }
	  }
        inputmode = 0;
  return(0);
}

/**************************************************************************
*
* handle_ESC()
*
* sets new color value from received ANSi color sequences.
* excluded from telrcv() for some optimizer's sake. *sighs*
* escape_seq_state == 0: currently no ESC sequence received,
* escape_seq_state == 1: we're receiving the first char after the ESC
* escape_seq_state == 2: we're receiving any subsequent characters
*
**************************************************************************/

void handle_ESC(c)
char c;
{
  if(((mystrchr("78[", c) == 0) && (escape_seq_state == 1))
   || ((mystrchr("1234567890;m",c) == 0) && (escape_seq_state == 2)))
  {
    escape_seq_state = 0;
    return;
  }
  if((escape_seq_state == 1) && ((c == '7') || (c == '8')))
  {					/* color storing / restoring */
    switch(c)
    {
      case '7':
	strcpy(stored_col, curr_col);
	escape_seq_state = 0;
	return;
      case '8':
	strcpy(curr_col, stored_col);
	escape_seq_state = 0;
	return;
    }
  }					/* ESC sequence start */
  if((c == '[') && (escape_seq_state == 1))
  {
    escape_seq[0]=0;
    escape_seq_state=2;
    return;
  }

  if(mystrchr("1234567890;m", c) && (escape_seq_state == 2))
    strncat(escape_seq, &c, 1);
 
  if((c == 'm') && (escape_seq_state == 2))
  {
    new_color(escape_seq);
    escape_seq[0]=0;
    escape_seq_state = 0;
  }
}

/**************************************************************************
*
* telrcv()
*
* main routine to process incoming net data
*
**************************************************************************/

int
telrcv(c)
  register int c;
{

    if (escape_seq_state)
    {
      handle_ESC(c);
    }

    if (c == 27)
      escape_seq_state = 1;

    if(debugging)
      fputc(c, debugfile);

    switch (state)
    {
       
       /*
	* the generic case
	*/
       
     case TS_DATA:
       
       /* telnet Is A Command (IAC) byte */
       
       if (c == IAC)
	 state = TS_IAC;
       
       /*
	* if a file is being sent over.
	*/
       
       else if (fileflag)
	 {
	    putc(c, specfile); 
	 }
       
       /*
	* if we're receiving (updating) the userflags from the BBS.
	*/
       
       else if(updateflag)
	 {
	    
	    /*
	     * after IAC UPDATE, these flags are sent:
	     * 1 US_ANSICOLORS,
	     * 2 US_NOFLASH,
	     * 3 US_NOBOLDCOLORS
	     */
	    
            updateflag++;
	    
            switch(updateflag)
	      {
	       case 2:
		 if(c == 0)
		   userflags &= ~US_ANSICOLORS;
		 else
		   userflags |= US_ANSICOLORS;
		 break;
		 
	       case 3:
		 if(c == 0)
		   userflags &= ~US_NOFLASH;
		 else
		   userflags |= US_NOFLASH;
		 break;
		 
	       case 4:
		 if(c == 0)
		   userflags &= ~US_NOBOLDCOLORS;
		 else
		   userflags |= US_NOBOLDCOLORS;
		 updateflag = 0;	/* this was the last flag sent	*/
		 break;
		 
	       default:
		 printf("UPDATE-problems... (don't worry though...)\n");
		 break;
	      }
	    
	 }
       
       /*
	* now here first comes a check if we got a newline-character, which
	* means we've got a line of something. this 'something' then has to
	* be handled appropriately.
	*/
       
       else if (c == '\n')	/* KHchange */
	 {
	    if (wholistflag && DOC)
	    {
	      process_DOC_wholist(c);
	      break;
	    }
	    else
	      handle_lines(c);
	  }
      
        else /* KHCHANGE if (!wholist || xmsgflag) */
          {
            if (DOC && !(userflags & US_ANSICOLORS) && c == '\033') 
	      userflags |= US_ANSICOLORS;

	    if (wholistflag && DOC)
	    {
	      process_DOC_wholist(c);
	      break;
	    }

            if (xmsgflag > 0)
	       {
		  xmsgbuf[xmsgbufp++] = c;
	       }

	    else if (postflag > 0)
              postbuf[postbufp++] = c;

	    else if (wholistflag > 0 && moreflag == 0)
	      wholistbuf[wholistbufp++] = c;

	    else if ((!xmsgflag && !postflag) || xmsgnow > 0)
              {

              if(userflags & COLORCAPTURE)
		if((capture) && (c < 0 || c >= 32 || c == TAB || c == 1 
		|| c == '\n' || c == '\r') && (c < DEL) && !moreflag)
                    if (putc(c, tempfile) < 0)
                      tempfileerror();

		/* KHCHANGE to enable ANSI-colors */

		if(c == '\1')
		  {
		    colorflag = 1;
		    break;
		  }

		else if(colorflag == 1)
		  {
		    colorflag = 0;
		    if(userflags & US_ANSICOLORS)
		      color(c);
		    break;
		  }

		if(c != -1)
		{
                  putchar(c);
	          if(!(userflags & COLORCAPTURE))
		  if((capture) && (c < 0 || c >= 32 || c == TAB || c == 1 
		  || c == '\n' || c == '\r') && (c < DEL) && !moreflag)
                      if (putc(c, tempfile) < 0)
                        tempfileerror();
		}

	      }
	  }
        break;
    
      /*
       * handle various telnet and client-specific IAC commands
       */
	   
      case TS_IAC:
        handle_IAC(c);
        break;
    
          /* Get local mode strings/lines/posts */
      case TS_GET:
	if (handle_GET(c) == -1)
	  return(-1);
        break;
    
          /* Ignore next byte
	   * (used for ignoring negotations we don't care about)
	   */
      case TS_VOID:
	
        default:
          state = TS_DATA;
          break;
       
     case TS_DO:
       switch(c)
	 {
	  case TELOPT_BBSPOST:     /* Yeah, I can handle them,
				      because I'm a big CLient. */
	  case TELOPT_TIMING_MARK: /* So that BBS's can ping CLients. */
	  case TELOPT_NAWS:
	  case CLIENT_OPTIONS:
	  case TELOPT_NEW_ENVIRON: 
	  case TELOPT_TERMINAL_TYPE: 
	    netput(IAC);
	    netput(WILL);
	    netput(c);
	    if(debugging)
	      printf("[BBS: IAC DO %d: WILL.]\n", c);
	    if(c==TELOPT_NAWS)
	      sendnaws();
	    break;

          default:
            netput(IAC);
              netput(WONT);
              netput(c);
	      if(debugging)
	        printf("[BBS: IAC DO %d: WONT.]\n", c);
            break;
	 }
       state=TS_DATA;
       break;
       
     case TS_DONT:
       state=TS_DATA;
       break;

     case TS_WILL:
       state=TS_DATA;
       switch(c)
	 {
	  case TELOPT_LOGOUT:
	    if(debugging)
	      printf("[BBS: IAC WILL TELOPT_LOGOUT]\n");
	    if(idletime<6)
	      {
		 /* Ah. No, piss off. You can't sodding log me out. I'm busy. */
		 netput(IAC);
		 netput(DONT);
		 netput(TELOPT_LOGOUT);
		 if(!inputmode)
		   byte[curr_bbs]+=2;
		 netflush();
	      }
	    else
	      {
		colorize("\007\n\1a\1f[\1rCLient\1y: \1gThe BBS has just warned you'll be thrown out in a minute.\1w]\n");
		fflush(stdout);
		if(sessionflags[curr_bbs] & S_LOGOUTW_WILL)
		  sessionflags[curr_bbs] |= S_LOGOUTW_RCV;
	      }
	    break;

	  case TELOPT_ECHO:
	  case TELOPT_SGA:
	    netput(IAC);	/* for telnet connections */
	    netput(DO);
	    netput(c);
	    if(debugging)
	      printf("[BBS: IAC WILL %d: DO.]\n", c);
	    break;

	  default:	/* !TS_TELOPT_LOGOUT */
	    netput(IAC);
	    netput(DONT);
	    netput(c);
	    if(debugging)
	      printf("[BBS: IAC WILL %d: DONT.]\n", c);
	    break;
	 }
       break;	/* TS_WILL */

     case TS_WONT:
       state=TS_DATA;
       break;

    case TS_SB:
      if(debugging)
        printf("[BBS: IAC SB %d]\n", c);
      switch(c)
      {
        case CLIENT_OPTIONS:
          state=TS_SB_CLIENTOPTIONS;
          break;
        case TELOPT_NEW_ENVIRON:
        case TELOPT_TERMINAL_TYPE:
	  state=TS_IGNORE_SB;
	  current_sb = c;	/* keep track on which SB we're in */
	  break;
        default:
          state=TS_IGNORE_SB; /* This should never be reached though. */
        break;
      }
      break;

    case TS_IGNORE_SB: /* wait for IAC SB, ignore the contents */
      if(c == IAC)
	state = TS_IGNORE_SB_WAS_IAC; /* else ignore */

      break;

    case TS_IGNORE_SB_WAS_IAC: /* wait for IAC SB, ignore the contents */
      state=TS_DATA;
      if(c == SE)
      {
	sp = (char *)malloc(80);
	switch(current_sb)
	{
          case TELOPT_NEW_ENVIRON:
          case TELOPT_TERMINAL_TYPE:
	    netput(IAC);
	    netput(SB);
	    netput(current_sb);
	    netput(IS);

	    if(current_sb != TELOPT_TERMINAL_TYPE)
	    {
	      netput(USERVAR);
	      sprintf(sp, "TERM");
	      for(i=0; i<strlen(sp); i++)
	        netput(sp[i]);
	      netput(VALUE);
	    }
	    sp = (char *) getenv("TERM");
	    if(sp == NULL)
	    {
	      sp = (char *)malloc(10);	/* should be freed again */
              sprintf(sp, "UNKNOWN");
	    }
	    for(i=0; i<strlen(sp); i++)
	      netput(sp[i]);

	    if(debugging)
	      printf("[BBS: IAC SB %d: (%s)]\n", c, sp);
	    netput(IAC);
	    netput(SE);
	    break;

	  default:
	    break;
	}
	current_sb = 0;	/* reset value */
      }
      else
	state = TS_IGNORE_SB;	/* not an IAC SE - back into ignore mode */

      break;

    case TS_SB_CLIENTOPTIONS:
      switch(c)
      {
         case POST_MARK:
           state=TS_SB_CLIENTOPTIONS_POSTMARK;
	   break;

	 case TELOPT_LOGOUT:
	   state=TS_SB_CLIENTOPTIONS_TIMEOUT;
           break;

         default:
           state=TS_DATA;      /* unexpected */
         break;
       }
       break;

     case TS_SB_CLIENTOPTIONS_POSTMARK:
       switch(c)
       {
         case ON:        /* CLient can handle both */
         case OFF:
           netput(IAC);
           netput(SB);
           netput(CLIENT_OPTIONS);
           netput(POST_MARK);
           netput(OK);
           state=TS_DATA;
	   if(debugging)
	     printf("[BBS: Post killing feature switched %s.]\n", (c==ON)?"on":"off");
         break;

         default:        /* not ON or OFF -> unknown */
           netput(IAC);
           netput(SB);
           netput(CLIENT_OPTIONS);
           netput(POST_MARK);
           netput(ERROR);
           state=TS_DATA;
	   if(debugging)
	     printf("[BBS: Post killing feature: unknown code (%d).]\n", c);
         break;
       }
       break;

     case TS_SB_CLIENTOPTIONS_TIMEOUT:
       switch(c)
       {
         case ON:        /* CLient can handle both */
         case OFF:
           netput(IAC);
           netput(SB);
           netput(CLIENT_OPTIONS);
           netput(TELOPT_LOGOUT);
           netput(OK);
           state=TS_DATA;
	   if(c==ON)
	   {
	     sessionflags[curr_bbs] |= S_LOGOUTW_WILL;
	     sessionflags[curr_bbs] &= ~S_LOGOUTW_RCV;
	   }
	   if(debugging)
	     printf("[BBS: Idle detection feature switched %s.]\n", (c==ON)?"on":"off");
         break;

         default:        /* not ON or OFF -> unknown */
           netput(IAC);
           netput(SB);
           netput(CLIENT_OPTIONS);
           netput(POST_MARK);
           netput(ERROR);
           state=TS_DATA;
	   if(debugging)
	     printf("[BBS: Idle detection feature: unknown code (%d).]\n", c);
         break;
       }
       break;
    }

    return (0);

}

/*
 * Send signal that block of data follows -- this is a signal to the bbs that
 * it should stop ignoring what we send it, since it begins ignoring and
 * throwing away everything it receives from the time it sends an IAC G_*
 * command until the time it receives an IAC BLOCK command. 
 */
void
sendblock()
{
  if (TELNET && !DOC)
    return;

  netput(IAC);
  netput(BLOCK);
}



/*
 * Send a NAWS command to the bbs to tell it what our window size is. 
 *
 * KH says: Hmm, currently this isn't used or necessary at all, but it could
 * be a good thing really... *nod* I'll make something to receive the data
 * at the BBS-side. and remove the usersupp->screenlength... ?
 */
void
sendnaws()
{
char    s[10];
register int i;

  if (oldrows != getwindowsize())
  {
    if (rows > 70 || rows < 10)
      rows = 24;
    else
      oldrows = rows;
    sprintf(s, "%c%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_NAWS, 0, 0, 0, rows, IAC, SE);
    for (i = 0; i < 9; i++)
      netput(s[i]);
  }
}


/*
 * Initialize telnet negotations with the bbs -- we don't really do the
 * negotations, we just tell the bbs what it needs to hear, since we don't need
 * to negotiate because we know the correct state to put the terminal in. The
 * BBS (the queue daemon actually) is kludged on its end as well by the IAC
 * CLIENT command. 
 */
void
telinit()
{
  char    s[39];
  register int i;

  if(DOC)
  {
    sprintf(s, "%c%c%c%c%c%c%cUSER%c%s%c%c"
             , IAC, CLIENT2, IAC, SB, TELOPT_ENVIRON, 0, 1, 0, user, IAC, SE);
    for (i = 0; i < (int)strlen(user) + 14; i++)
      netput(s[i]);
    sendnaws(); 
    netflush();
  }
  else if(!TELNET)
  {
    netput(IAC);
    netput(CLIENT);
    /*
    * this sendnaws() should not be here, since the yawcserver is just checking
    * for the CLIENT-signal above here, and skips the rest. it could perhaps be
    * in the START-case?
    */
    sendnaws(); 
    /*
    * And more to the point, we don't know if the BBS supports NAWS yet, either.
    * What a shite protocol.
    * Makes me ashamed to develop it.
    */
  }


  /* Flint hack: auto login macro */

  mybzero(macro[1], 72);	/* reset init macro */    
  if((strlen(bbsuser[curr_bbs]) > 0) && (!(userflags&NO_AUTO_LOGIN) ||
      (curr_bbs==MAXSYS)))
  {
    sprintf(macro[1]," %s\n",  bbsuser[curr_bbs]);
    if(strlen(bbspass[curr_bbs]) > 0)
    {
     strcat(macro[1], bbspass[curr_bbs]);
      strcat(macro[1], "\n");
    }
    userflags |= LOGIN_ASAP; /* activate macro after G_NAME network byte */
  }
}
