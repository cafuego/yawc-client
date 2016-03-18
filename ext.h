/*
 * This is the home of all global variables (see global.c) 
 */

extern int systemtype;		/* 0=Unix, 1=VMS, ...			*/
extern char bbsrcname[100];	/* bbsrc filename (usually ~/.yawcbbsrc) */
extern FILE *bbsrc;		/* file descriptor of .yawcbbsrc file	*/
extern int bbsrcro;		/* set if .yawcbbsrc file is read-only	*/
extern char tempfilename[100];	/* bbstmp filename (usually ~/.yawcbbstmp) */
extern FILE *tempfile;		/* file pointer to above		*/
extern char specfilename[100];	/* KHAddition				*/
extern FILE *specfile;		/* KHAddition				*/
extern char user[16];		/* username				*/	
extern char editor[80];		/* name of editor to invoke		*/
extern char myeditor[80];	/* name of user's preferred editor	*/

extern int childpid;		/* process id of child process		*/
extern jmp_buf jmpenv;		/* Yuck!  I have to use longjmp! Gag!	*/

extern char bbshost[MAXSYS+1][40];	/* name of bbs host			*/
extern char bbsuser[MAXSYS+1][25];	/* user's name there			*/
extern char bbspass[MAXSYS+1][20];	/* user's pw there			*/
extern char bbspw_e[MAXSYS+1][20];	/* user's pw there (encrypted)		*/
extern char bbsname[MAXSYS+1][30];	/* name of the BBS (comment)		*/

extern char friend[MAXFRIENDS][20]; /* array for 'friend' list		*/
extern int friendp;		/* pointer to end of friend list	*/
extern unsigned char savewho[60][21];	/* array for saved who list	*/
extern int wholistbufp;		/* pointer to end of saved who list	*/
extern unsigned char wholist[MAXWHO][21]; /* KH: real wholist.		*/

extern char enemy[MAXENEMIES][20]; /* array for 'enemy' list		*/
extern int enemyp;		/* pointer to end of enemy list		*/
/* extern char saveheader[160];	  for saving our message header, unused	*/

extern char macro[128][72];	/* array for macros			*/
extern int macrokey;		/* hotkey for signalling a macro follows */
extern int quitkey;		/* hotkey to quit (macrokey quitkey)	*/
extern int suspkey;		/* hotkey for suspending (" suspkey)	*/
extern int connectkey;		/* hotkey for connect prompt (" connect)*/
extern int capturekey;		/* Toggle text capture key (" capturekey) */
extern int capture;		/* Capture status			*/
extern int shellkey;		/* hotkey for shelling out (" shellkey) */
extern char shell[80];		/* User's preferred shell		*/

extern int macron;
extern int macrop;

extern int curr_bbs;		/* from 0 to MAXSYS-1			*/
extern char * cmd_line_hostname; /* command line hostname		*/
extern short int cmd_line_port;	/* command line port number		*/
extern int net[MAXSYS+1];	/* file descriptor of network socket	*/
extern FILE *fps[MAXSYS+1][2];	/* file pointers                    	*/
extern FILE *netifp;		/* file pointer for input from net	*/
extern FILE *netofp;		/* file pointer for output to net	*/

extern int rows;		/* number of rows on user's screen	*/
extern int oldrows;		/* previous value of rows		*/
extern int posting;		/* true if user is currently posting	*/
extern int inputmode;		/* true if user is in local mode -> AWAY
				   message cannot be sent in that case  */
extern int lastsave;		/* true if last time user edited they saved */
extern int check;		/* true if waiting to check BBS for X's */
extern int configflag;		/* true if we are in bbsrc config funcs */
extern unsigned int userflags;	

extern long byte[MAXSYS+1];		/* current byte (remotely synched with bbs) */
extern long targetbyte[MAXSYS+1];	/* where the client wants to get	*/
extern long bytep[MAXSYS+1];		/* where the client is			*/
extern unsigned char save[MAXSYS+1][1000];/* buffer to save past user bytes	*/
extern int idletime;
/* Flint additions */
extern int port[MAXSYS+1];		/* BBS port (mainly telnet/non-telnet   */
extern int sessionflags[MAXSYS+1];	/* the state the session is in  */
extern char curr_col[20];	/* the current colour as per ColourChar */
extern char stored_col[20];	/* colour stored via ^[7		*/
extern int debugging;           /* debug file write on/off              */
extern char master_password[20];/* what it says ;)			*/
extern char master_pw_enc[20];  /* the encrypted form			*/
extern FILE *debugfile;         /* debug file handle			*/
extern char away_message[200];	/* away macro				*/
extern char alt_away[200];	/* alternative away macro		*/
extern char colst[200];		/* temp string needed for cprint -> colorize */
extern int away;		/* away state				*/
extern int last_errno;		/* last error code			*/
