/*
 * This is the main procedure, the details are elsewhere.  Since it is so
 * small, it is the only function in this whole program I can confidently say
 * is maybe probably sort of a bit slightly bug-free. 
 *
 *
 * ISCA BBS client v1.5    02/15/94 (des)
 *
 * Revision history:
 *
 * v1.0    06/20/93 (des)  Initial release of Unix client.
 * v1.01   07/06/93 (des)  (Hopefully) made it more portable, added automatic X
 *                         message wrapping from Marc Dionne (Marco Polo)
 * v1.02   07/12/93 (des)  Portable for more machines, fixed X wrapping bugs.
 * v1.03   07/26/93 (des)  Fixed another X wrap problem, added VMS porting code
 *                         from Marc Dionne (Marco Polo), added code for
 *                         dropping invalid characters as done in the BBS.
 * v1.1    07/27/93 (des)  Added NeXT window titles from Dave Lacey (Doctor
 *                         Dave).  Added 'shell' key to shell out of client.
 *                         Initial public release of VMS client.
 * v1.11   08/11/93 (des)  Several fairly minor bug fixes for Unix and VMS. VMS
 *                         version should now work on Wollongong's TCP.
 * v1.2    10/12/93 (des)  A number of small bug fixes, ctrl-W lists will now
 *                         save the previous listing to avoid seeing nothing
 *                         when hitting ctrl-W, and capture now captures forum
 *                         info and message number.  .bbsrc now not required.
 * v1.21   10/14/93 (des)  Small bug fixes, added ability to use BBSRC and
 *                         BBSTMP environment variables in the Unix version,
 *                         which had been added at 1.2 for VMS by Marc Dionne.
 * v1.3    10/24/93 (des)  Small bug fixes (as always!)  Added bbsrc config
 *                         stuff, bbsrc is now self-generating and no longer
 *                         required to be installed when first setting up the
 *                         client, making things hopefully much easier for the
 *                         less than clueful hordes. 
 * v1.31   11/01/93 (des)  Minor bug fixes.  Unix version now truncates file to
 *                         avoid extra blank lines.  Unreleased test version.
 * v1.4    11/08/93 (des)  Usual minor bug fixes.  Marco Polo comes through in
 *                         the crunch with UCX compatability for the VMS
 *                         version!  Capture after posting doesn't ask if you
 *                         want to clear the temp file.  BBSRC and BBSTMP files
 *                         chmod'ed to 0600 in the Unix version by default.
 *                         Gave in and let people use any key they want for
 *                         their hotkeys as long as it isn't a return and does
 *                         not conflict with any other hotkey.
 * v1.41   12/26/93 (des)  Minor bug fixes, added in ctrl-W line erase, plus
 *                         the character translation and stupid-term-program
 *                         protection of the real BBS (see inkey.c)  Finally
 *                         nabbed that real X message line wrapping bug, plus a
 *                         few others.  This release is more for testing these
 *                         fixes while I continue to look at the feasibility
 *                         usefulness of adding compression for the data coming
 *                         to the client from the BBS.  Made .bbsrc parsing
 *                         more forgiving.
 * v1.42   12/31/93 (des)  Bug fixes, added compability for a few more
 *                         machines, including Amigas running Unix, BSDI, and
 *                         SCO/Xenix.  Added "term" compatibility for Michael
 *                         O'Reilly's term package courtesy of Ozy.
 * v1.5    02/15/94 (des)  Fixed nasty character dropping bug, made enemy list
 *                         work for > 1 screen posts properly, plus it now
 *                         works correctly when in ANSI mode.  A couple of VMS
 *                         specific bugfixes.  Added ctrl-R ala Unix reprint to
 *                         match change in BBS code, changed 'bad character
 *                         eating' code used to keep safe from runaway pastes
 *                         and line noise to make it less obtrusive and work
 *                         better.  The character translation for \r\n -> \n
 *                         was updated to work well on older BSD systems as
 *                         well as VMS and termio-based systems (this was the
 *                         source of the nasty character dropping bug)
 *
 *                         This is likely to be pretty much it as far as my
 *                         [Serendipity's] contribution to the client, other
 *                         than fixing bugs present in 1.5, or fixes to bring
 *                         the client in line with any changes in how the BBS
 *                         itself does things.  Any new functionality will be
 *                         added by others...
 */

/* YAWC client history:
 *   Version 1.0 Kenneth Haglund.
 *   Version 1.1 MathFox
 *                           Better color support,
 *                           Bug fixes in several places,
 *                           Different save format for macro's
 *                           -# option from Zavvy for BBS selection
 *   Version 1.2 Diamond White
 *                           Added cprintf functionality to the code,
 *                           Added quit_priv==5 email support,
 *                           Changed all string input code,
 *                Added cursor support (Limited, anyone want to improve this?),
 *                Added ^An...^AN name recognition (X's and Wholists only),
 *                Added the nasty weird KH kludge handler for wholists.
 *
 *   1.2 Revision D Flint    Fixed some bugs in the new getline routine.
 *			     added unix-style ^U and ^W into getline
 *   1.2 Revision E Flint    Fixed the chatmode command and ^N bug.
 *
 *   Version 1.3 Flint	     Added auto login feature, password encryption
 *			     using DES software
 *			     Re-introduced BBS names (was: BBS comments)
 *			     Feature negotiation support,extended state machine
 *			     Improved DWD's state machine to be able now to use
 *			     the CLient as normal telnet client.
 *			     Added some telnet macro functions to activate
 *			     several local input modes manually
 *			     -d option for debugging, also # and -# are back
 *			     -l now lists all BBSs, [-]? prints options
 *			     Added auto reply and keyboard lock
 *			     ^N / ^K name rotation
 *			     Fixed capture function, added colour support to it
 *			     for further changes see file CHANGES.LOG.
 *                     	     removed cprintf() again - it gave too much trouble
 *			     on some stupid cc compilers. =(
 *
 *   further history in CHANGES file
 *
 */

#include "defs.h"
#include "ext.h"

static  char    *
get_arg(arg, next_arg, ac)
    char    *arg;
    char    *next_arg;
    int     *ac;
{
   if (*arg)
     return (arg);
   else
     {
        (*ac)++;
	if (next_arg)
	  return (next_arg);
	fprintf(stderr, "CLient: missing parameter\n");
	exit(1);
     }
}

void 
print_config(int which)
{
  int i;

  printf("Number    BBS Name             BBS address                   Username\n");
  printf("------------------------------------------------------------------------------\n");
  for (i=0; i<MAXSYS; i++)
    if((which < 0) || (which == i))
      if(*bbshost[i])
        printf("  %c    %-20.20s %-32.32s %-16.16s\n", (i<10)?i+'0':i-10+'A',
	        bbsname[i], bbshost[i], bbsuser[i]);
  printf("\n");
}

void print_help(name)
char *name;
{
  printf("Usage: %s [[-[s]] #] [[-h] host.address] [-p port#]|[-t] [-n] [-l] [-P] [-d]\n", name);
  printf("\nSwitches:\n");
  printf("  -s #  connects to the host specified as host # in your config file.\n");
  printf("  -h [IP# | alpha address]  connects to the specified host.\n");
  printf("  -p #  connects to port # instead of YAWC standard port 1976.\n");
  printf("  -t  Act as telnet client, connect to telnet port 23.\n");
  printf("  -i  Act as DOC BBS (ISCA) client.\n");
  printf("  -n  Don't do autologin even if usernames/passwords are stored in config file.\n");
  printf("  -l  List site configuration and exit.\n");
  printf("  -P  Ping specified host (see if it's alive) only and exit.\n");
  printf("  -F<User_Name> Finger specified host or user on host only and exit.\n");
  printf("  -d  debug mode, all incoming BBS data will be written to '%s'.\n\n", DEBUGFILE);
}

int 
parsearg (argv, argc)
char *argv[];
int argc;
{
   int ac = 1;
   int GotBBSNum=0;
   int checkonly=0; /* ping or finger host only */
   int i;
   int cmdlineflags = 0;
   char *arg;
   char fingername[20];
   
   cmd_line_hostname=NULL;
   cmd_line_port=0;
   while ((arg = argv[ac++]) != (char *) NULL)
     {
        if ((*arg == '-' ) != '\0')
	  {
	     ++arg;
	     if(*arg)
	       {
		  switch (*(arg++))
		    {
		     case 's': /* slot number */
		       curr_bbs = atoi(get_arg(arg,argv[ac],&ac));
		       if(curr_bbs>MAXSYS)
			 {
                           fprintf(stderr, "%s: Invalid argument for switch -s '%d' (Must be 0 to 9).\n", argv[0], curr_bbs);
			   exit(1);
			 }
		       if(!*bbshost[curr_bbs])
			 {
			   fprintf(stderr, "%s: No BBS defined with number %d.\n", argv[0], curr_bbs);
			   exit(1);
			 }
                       if (GotBBSNum)
                         fprintf(stderr, "%s: Warning: Multiple slot numbers specified. Overriding previous value.\n", argv[0]);
		       GotBBSNum=1;
		       break;
		       
		     case 'h': /* host name */
		       cmd_line_hostname=get_arg(arg, argv[ac], &ac);
		       break;
		       
		     case 't': /* telnet mode */
		       cmd_line_port=23; /* just an idea - telnet! =) ... */
		       cmdlineflags |= S_TELNET;
		       break;

		     case 'i': /* ISCA (DOC) client mode */
		       cmd_line_port=23;
		       cmdlineflags |= S_DOCCLIENT | S_TELNET;
		       break;

		     case 'p': /* port number */
		       cmd_line_port=atoi(get_arg(arg, argv[ac], &ac));
		       break;

		     case 'P': /* ping */
		       checkonly = 1;
		       break;

		     case 'F': /* ping */
		       checkonly = 2;
		       strncpy(fingername, arg, 20);
		       fingername[19]=0;
		       break;

		     case 'l': /* list config */
		       print_config((GotBBSNum)?curr_bbs:-1);
		       exit(0);

		     case 'n': /* omit auto login */
		       userflags |= NO_AUTO_LOGIN;
		       break;

                     case 'd': /* debug mode */
                        debugging = 1;
                        debugfile = fopen(DEBUGFILE, "w");
                        if (debugfile == NULL)
                        {
                          printf("Couldn't open debugfile, ignoring debug option.\n");
                          debugging = 0;
                        }
                        break;

                     case '0': /* slot number */
                     case '1':
                     case '2':
                     case '3':
                     case '4':
                     case '5':
                     case '6':
                     case '7':
                     case '8':
                     case '9':
                       curr_bbs = atoi(&*(arg-1)); /* >10 BBSs prepared here */
                       if(curr_bbs>MAXSYS)
                       { 
                         fprintf(stderr, "%s: Invalid BBS number '%d' (Must be 0 to 9).\n", argv[0], curr_bbs);
                         exit(1);
                       }
                       if (GotBBSNum)
                         fprintf(stderr, "%s: Warning: Multiple slot numbers specified. Previous value ignored.\n", argv[0]);
                       if(!*bbshost[curr_bbs])
                       {
                         fprintf(stderr, "%s: No BBS defined with number %d.\n", argv[0], curr_bbs);
                         exit(1);
                       }
                       GotBBSNum=1;
		       break;

		     case '?':
			print_help(argv[0]);
			exit(0);
		    }
	       }
	  }

         else /* no option - hostname? */
	 if(strlen(argv[ac-1])>2)
	 {
	   if(!strchr(argv[ac-1], '.'))
	   {
	     for(i=0; i<MAXSYS; i++)
	       if(strucmp(argv[ac-1], bbsname[i]) == 0)
	       {
		 curr_bbs = i;
	         GotBBSNum=1;
	         break;
	       }
	       if(strucmp(argv[ac-1], bbsname[i]) == 0)
		 continue;
	   }
           cmd_line_hostname=get_arg(arg, argv[ac], &ac);
	 }
	 else
	 if(((argv[ac-1][0] >='0') && (argv[ac-1][0] <= '9'))
	    || ((argv[ac-1][0] >='A') && (argv[ac-1][0] <= 'Z') 
                && ((argv[ac-1][1]== 0) || (argv[ac-1][1]== ' ')))
	    || ((argv[ac-1][0] >='a') && (argv[ac-1][0] <= 'z') 
                && ((argv[ac-1][1]== 0) || (argv[ac-1][1]== ' '))))
         {
           if(mystrchr("1234567890", argv[ac-1][0]))
             curr_bbs = atoi(argv[ac-1]);
           if(mystrchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", argv[ac-1][0]))
             curr_bbs = argv[ac-1][0] - 55;
           if(mystrchr("abcdefghijklmnopqrstuvwxyz", argv[ac-1][0]))
             curr_bbs = argv[ac-1][0] - 87;

	   if(curr_bbs>MAXSYS)
	   {
             fprintf(stderr, "%s: Invalid BBS number '%d' (Must be 0 to 9).\n", argv[0], curr_bbs);
	     exit(1);
	   }
	   if (GotBBSNum)
	     fprintf(stderr, "%s: Warning: Multiple slot numbers specified. Previous value ignored.\n", argv[0]);
	   if(!*bbshost[curr_bbs])
	   {
     fprintf(stderr, "%s: No BBS defined with number %d.\n", argv[0], curr_bbs);
	    exit(1);
	   }
	   GotBBSNum=1;
        }
	else
	if(argv[ac-1][0] == '?')
	{
          print_help(argv[0]);
          exit(0);
	}
	else
	{
	  fprintf(stderr, "\n%s: Illegal option. Aborting.\n", argv[0]);
	  exit(1);
	}
     }
   if(cmd_line_hostname!=NULL)
   {
     if(!GotBBSNum)
       curr_bbs = MAXSYS;
     strcpy(bbshost[curr_bbs], cmd_line_hostname);
     mybzero(bbsuser[curr_bbs], 25);
     mybzero(bbspass[curr_bbs], 20);
   }
  
   if(cmd_line_port)
     port[curr_bbs]=cmd_line_port;

   if(checkonly)
   {
     printf("%s '%s', press ^C to abort.\n\n", (checkonly == 1)?"Pinging":
	    "Fingering", (GotBBSNum||(cmd_line_hostname == NULL))?
	     bbshost[curr_bbs]:cmd_line_hostname); 
     fflush(stdout);
     arg = (char *)malloc(80);
     sprintf(arg,"%s ", (checkonly == 1)?"ping ":"finger ");
     if(checkonly == 2)
     {
       if(fingername)
       {
         strcat(arg, fingername);
       }
       strcat(arg, "@");
     }
     strcat(arg, (GotBBSNum||(cmd_line_hostname == NULL))?bbshost[curr_bbs]:cmd_line_hostname);
     system(arg);
     free(arg);
     exit(1);
   }
   if(cmdlineflags & S_TELNET)
     sessionflags[curr_bbs] |= S_TELNET;
   if(cmdlineflags & S_DOCCLIENT)
     sessionflags[curr_bbs] |= S_DOCCLIENT;

/*
   if TELNET
     userflags &= ~US_ANSICOLORS;
*/
   return 0;
}

void
init_globals()
{
  int i;

  idletime=0;
  debugging=0;
  strcpy (curr_col, "\1a\1 \1 \1 \1 \1 \1 ");
  strcpy (stored_col, "");
  macron = 0;
  macrop = 0;
  *away_message = 0;
  away = 0;
  last_errno=0;
  for(i=0; i<MAXSYS; i++)
    port[i]=1976;
  userflags = US_ANSICOLORS | US_ANSIDEFAULT;
}

int
main(argc,argv,envp)
        int argc;
        char *argv[];
        char *envp[];
{
   init_globals();
   initialize("v2.0 revision C1");
   findhome();
   readbbsrc(argv,argc);
   opentmpfile();
   parsearg(argv,argc);
   printf("\n[Trying to connect to '%s']\n", (cmd_line_hostname)?
   cmd_line_hostname:(*bbsname[curr_bbs])?bbsname[curr_bbs]:
   (*bbshost[curr_bbs])?bbshost[curr_bbs]:BBSHOST);
   connectbbs(0);
   siginit();
   telinit();
   setterm();
   friends2tab();
   looper();
   exit(0);
   return(0);
}
