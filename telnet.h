/* Generic telnet protocol #defines, along with some special protocol that is
 * spoken between the ISCA BBS and a connected client.  You don't want to
 * change this, it depends on this being the same on both ends, and you can't
 * change what the BBS end does. */

#define	IAC	(unsigned char)255	/* interpret as command:	 */
#define	DONT	(unsigned char)254	/* you are not to use option	 */
#define	DO	(unsigned char)253	/* please, you use option	 */
#define	WONT	(unsigned char)252	/* I won't use option		 */
#define	WILL	(unsigned char)251	/* I will use option		 */
#define	SB	(unsigned char)250	/* interpret as subnegotiation	 */
#define	SE	(unsigned char)240	/* end sub negotiation		 */

/* BBS client additions */
#define CLIENT	(unsigned char)0xa0	/* Is a client			 */
#define BLOCK	(unsigned char)0xa1	/* Start block mode transfer	 */
#define G_STR	(unsigned char)0xa2	/* Get string (arg = length)	 */
#define G_NAME	(unsigned char)0xa3	/* Get name (arg = type)	 */
#define G_LINES	(unsigned char)0xa4	/* Get 5 lines of text (Xmsg, info)	 */
#define G_POST	(unsigned char)0xa5	/* Get post (arg: 0=normal, 1=upload) */
#define G_STRWR	(unsigned char)0xa6	/* Get string, wordwrapped.	 */
#define S_WHO  	(unsigned char)0xa6	/* DOC defined this for wholist
					 * start! */
#define XMSG_S	(unsigned char)0xa7	/* Start X message transfer	 */
#define XMSG_E	(unsigned char)0xa8	/* End   X message transfer	 */
#define POST_S	(unsigned char)0xa9	/* Start post transfer		 */
#define POST_E	(unsigned char)0xaa	/* End   post transfer		 */
#define MORE_M  (unsigned char)0xab	/* Mark for MORE prompt	 */
#define START	(unsigned char)0xac	/* Synchronize count, pass version
					 * number */
#define START2	(unsigned char)0xad	/* client version (1.3) which does
					 * config */
#define CONFIG	(unsigned char)0xae	/* Tell client to do configuration */
#define START3	(unsigned char)0xaf	/* "Final" stable version of client
					 * (1.5) */
#define CLIENT2	(unsigned char)0xb0	/* Client code for version 1.5	 */
#define POST_K	(unsigned char)0xb1	/* Kill post			 */
#define UNNUSED	(unsigned char)0xb2	/* unused (was QUIT before)	 */
#define FILE_S  (unsigned char)0xb3	/* Start file transfer		 */
#define FILE_E  (unsigned char)0xb4	/* End file transfer		 */
#define EDIT_S  (unsigned char)0xb5	/* Edit specfile		 */
#define UPDATE  (unsigned char)0xb6	/* Update-command		 */
#define WHO_S	(unsigned char)0xb7	/* Start wholist transfer	 */
#define WHO_E	(unsigned char)0xb8	/* End   wholist transfer	 */

#define TELOPT_ECHO	(unsigned char)1	/* echo	RFC 857 */
#define	TELOPT_SGA	(unsigned char)3	/* suppress go ahead RFC 858 */
#define	TELOPT_NAWS	(unsigned char)31	/* window size RFC 1073      */
#define TELOPT_TERMINAL_TYPE (unsigned char)24	/* Terminal type RFC 1091    */
#define TELOPT_ENVIRON	(unsigned char)36	/* Environment variables RFC
						 * 1408 */
#define TELOPT_NEW_ENVIRON (unsigned char)39	/* Environment variables RFC
						 * 1572 */
#define TELOPT_LOGOUT   (unsigned char)18	/* Logout warning/request RFC
						 * 727 */
#define TELOPT_BBSPOST   (unsigned char)254	/* Post notifications
						 * <<<NON-STD>> */
#define TELOPT_TIMING_MARK (unsigned char)6	/* Timing marks, RFC 860 */

/* defines for environment SB negotiations */
#define IS		0
#define SEND		1
#define VALUE		1	/* no mistake, it's really 1 as well */
#define USERVAR		3

/* defines for CLient SB negotiations */
#define ERROR         	0
#define OK            	1
#define OFF           	2
#define ON            	3
#define CLIENT_OPTIONS  76
#define POST_MARK     	1

/* Various states for telnet state machine */
#define TS_DATA 0
#define TS_IAC  1
#define TS_WILL 2
#define TS_WONT 3
#define TS_DO   4
#define TS_DONT 5
#define TS_CR   6		/* Unused... */
#define TS_SB   7
#define TS_GET  8		/* CLient specific. */
#define TS_VOID 9		/* Client specific. */
/* These two aren't normal... Along with various other bits of this crap
 * state engine. This engine does now understand WILL/WONT/DO/DONT and also
 * does correctly respond to them (Flint). Former versions of this only
 * responded (DWD): for IAC WILL LOGOUT, which is a replacement for the IAC
 * CLIENT used for CLient logout warnings, IAC DO NAWS - Actually use the
 * NAWS Windows Size stuff, IAC DO TIMING-MARK - Used so that the BBS can
 * "ping" the host for timing reasons, IAC DO BBSPOST - A non-RFC, but still
 * vaguely congruous, way of negotiating the use of POST_S/POST_E/POST_K,
 * used by the BBS and CLient for killing posts. More of these will appear as
 * I (DWD) try to persuade the CLient to use more a protocol which is a
 * closer fit to the standard TELNET protocol. It'll stay backwards
 * compatible, of course, but let's face it, the TELNET protocol when left
 * alone could make a pretty good CLient, so long as a specialist telnetd was
 * doing it. */
/* state machine extensions */
#define TS_SB_CLIENTOPTIONS 10	/* IAC SB CLIENT_OPTIONS received */
#define TS_SB_CLIENTOPTIONS_POSTMARK 11	/* IAC SB CLIENT_OPTIONS POST_MARK */
#define TS_SB_CLIENTOPTIONS_TIMEOUT  12	/* IAC SB CLIENT_OPTIONS
					 * TELOPT_LOGOUT */
#define TS_IGNORE_SB 		13	/* ignore incoming SB data 	 */
#define TS_IGNORE_SB_WAS_IAC 	14	/* about to end ignore mode 	 */
