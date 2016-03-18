/* This file handles parsing of the bbsrc file, and setting of all the
 * options it allows.  It is just generic C code, easily extensible to allow
 * the addition of features in the future.
 * 
 * PR: do the trick with the | over here! */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "defs.h"
#include "ext.h"


/* Given a pointer to a string, this function evaluates it to a control
 * character, translating '^z', '^Z', or an actual ctrl-Z to all be ctrl-Z,
 * If the character is not a control character, it is simply returned as is. */
static int
ctrl(s)
    char *s;
{
    int c = *s;

    if (c == '^')
	if (((c = *++s) >= '@' && c <= '_') || c == '?')
	    c ^= 0x40;
	else if (c >= 'a' && c <= 'z')
	    c ^= 0x60;
    if (c == '\r')
	c = '\n';
    return (c);
}


/* Parses the bbsrc file, setting necessary globals depending on the content
 * of the bbsrc, or returning an error if the bbsrc couldn't be properly
 * parsed. */
void
readbbsrc(argv, argc)
    char *argv[];
    int argc;
{
    char tmp[82];
    int c;
    char *s, *m;
    int l = 0;
    int insecure = 0;
    int i;
    int reads = 0;


    macrokey = quitkey = suspkey = capturekey = shellkey = connectkey = -1;

    friendp = enemyp = 0;
    for (c = 0; c <= 127; c++)
	*macro[c] = 0;
    *editor = 0;

    master_password[0] = 0;
    master_pw_enc[0] = 0;

    for (i = 0; i < MAXSYS; i++) {
	*bbshost[i] = 0;
	*bbsname[i] = 0;
	*bbsuser[i] = 0;
	*bbspass[i] = 0;
	*bbspw_e[i] = 0;
	sessionflags[i] = 0;
    }
    bbsrc = findbbsrc();

    while (bbsrc && fgets(tmp, 82, bbsrc)) {
	reads++;
	l++;
	if ((int)strlen(tmp) >= 81) {
	    printf("Line %d in .yawcbbsrc too long, ignored.\n", l);
	    while ((int)strlen(tmp) >= 81 && tmp[80] != '\n')
		fgets(tmp, 82, bbsrc);
	    continue;
	}
	for (c = strlen(tmp) - 1; c >= 0; c--)
	    if (tmp[c] == ' ' || tmp[c] == '\t' || tmp[c] == '\n' || tmp[c] == '\r')
		tmp[c] = 0;
	    else
		break;

	/* Just ignore these for now, they'll be quietly erased... */
	if (!strncmp(tmp, "reread ", 7));
	else if (!strncmp(tmp, "xwrap ", 6));

	else if (!strncmp(tmp, "editor ", 7))
	    if (*editor)
		printf("Multiple definition of 'editor' ignored.\n");
	    else
		strncpy(editor, tmp + 7, 72);

	else if (!strncmp(tmp, "site", 4)) {
	    i = tmp[4] - 48;
	    if ((i < 0) || (i > 9))
		i = tmp[4] - 55;

	    if ((s = mystrchr(tmp + 6, ' ')) != NULL) {
		strncpy(bbsname[i], s + 1, 20);
		*s = '\0';
	    }
	    strncpy(bbshost[i], tmp + 6, 40);

	    if (strlen(bbshost[i]) < 1) {
		printf("Skipping illegal hostname in site #%d.\n", i);
		*bbshost[i] = 0;
		*bbsname[i] = 0;
	    }
	} else if (!strncmp(tmp, "masterpw", 8)) {
	    strncpy(master_pw_enc, tmp + 9, 20);
	    master_pw_enc[19] = 0;

	} else if (!strncmp(tmp, "username", 8)) {
	    i = tmp[8] - 48;
	    if ((i < 0) || (i > 9))
		i = tmp[8] - 55;

	    if ((i < 0) || (i > MAXSYS - 1)) {
		printf("Skipping illegal username for site #%d.\n", i);
		*bbsuser[i] = 0;
	    } else {
		strncpy(bbsuser[i], tmp + 10, 25);
		bbsuser[i][24] = 0;
	    }
	} else if (!strncmp(tmp, "password", 8)) {
	    i = tmp[8] - 48;
	    if ((i < 0) || (i > 9))
		i = tmp[8] - 55;
	    if ((i < 0) || (i > MAXSYS - 1)) {
		printf("Skipping illegal password for site #%d.\n", i);
		*bbspass[i] = 0;
	    } else {
		strncpy(bbspass[i], tmp + 10, 20);
		bbspass[i][19] = 0;
		if (bbspass[i][strlen(bbspass[i]) - 1] != '|')
		    printf("WARNING: missing '|' at the end of password %d; trailing spaces might got lost.\n", i);
		else
		    bbspass[i][strlen(bbspass[i]) - 1] = 0;
		insecure++;
	    }
	} else if (!strncmp(tmp, "pwd_encr", 8)) {
	    i = tmp[8] - 48;
	    if ((i < 0) || (i > 9))
		i = tmp[8] - 55;
	    if ((i < 0) || (i > MAXSYS - 1)) {
		printf("Skipping illegal encrypted password for site #%d.\n", i);
		*bbspw_e[i] = 0;
	    } else {
		strncpy(bbspw_e[i], tmp + 10, 16);
		bbspw_e[i][16] = 0;
	    }
	} else if (!strncmp(tmp, "friend ", 7))
	    if (friendp < sizeof friend / 20)
		if (strlen(tmp) == 7)
		    printf("Empty username in 'friend'.\n");
		else {
		    for (i = 0; i < friendp; i++)
			if (!strcmp(tmp + 7, friend[i])) {
			    printf("Duplicate username in 'friend'.\n");
			    break;
			}
		    if (i == friendp)
			strncpy(friend[friendp++], tmp + 7, 19);
		}
	    else
		printf("Too many entries given for 'friend'.\n");

	else if (!strncmp(tmp, "enemy ", 6))
	    if (enemyp < sizeof enemy / 20)
		if (strlen(tmp) == 6)
		    printf("Empty username in 'enemy'.\n");
		else {
		    for (i = 0; i < enemyp; i++)
			if (!strcmp(tmp + 6, enemy[i])) {
			    printf("Duplicate username in 'enemy'.\n");
			    break;
			}
		    if (i == enemyp)
			strncpy(enemy[enemyp++], tmp + 6, 19);
		}
	    else
		printf("Too many entries for 'enemy'.\n");

	else if (!strncmp(tmp, "macrokey ", 9)) {
	    if (macrokey >= 0)
		printf("Additional definition for 'macrokey' ignored.\n");
	    else {
		macrokey = ctrl(tmp + 9);
		if (mystrchr("\0x01\0x03\0x04\0x05\b\n\r\0x11\0x13\0x15\0x17\0x18\0x19\0x1a\0x7f", macrokey) || macrokey >= ' ') {
		    printf("Illegal value for 'macrokey', using default of '^B'.\n");
		    macrokey = 0x02;
		}
	    }
	} else if (!strncmp(tmp, "quit ", 5)) {
	    if (quitkey >= 0)
		printf("Additional definition for 'quit' ignored.\n");
	    else
		quitkey = ctrl(tmp + 5);
	} else if (!strncmp(tmp, "susp ", 5)) {
	    if (suspkey >= 0)
		printf("Additional definition for 'susp' ignored.\n");
	    suspkey = ctrl(tmp + 5);
	} else if (!strncmp(tmp, "capture ", 8)) {
	    if (capturekey >= 0)
		printf("Additional definition for 'capture' ignored.\n");
	    else
		capturekey = ctrl(tmp + 8);
	} else if (!strncmp(tmp, "connect ", 8)) {
	    if (connectkey >= 0)
		printf("Additional definition for 'connect' ignored.\n");
	    else
		connectkey = ctrl(tmp + 8);
	} else if (!strncmp(tmp, "macro ", 6)) {
	    c = ctrl(tmp + 6);
	    s = tmp + 7 + (tmp[6] == '^');
	    if (*s++ == ' ') {
		if (*macro[c])
		    printf("Additional definition of same 'macro' value ignored.\n");
		else {
		    m = macro[c];
		    /*** PR hack follows: */
		    if (s[strlen(s) - 1] != '|') {
			printf("WARNING: old style macro %s; trailing spaces might got lost.\n",
			       strctrl(c));
		    } else {
			s[strlen(s) - 1] = 0;
		    }
		    /*** PR: <EOH> */
		    while ((c = *s++)) {
			if (c == '^' && *s != '^')
			    c = ctrl(s++ - 1);
			if (c == '\r')
			    c = '\n';
			*m++ = c;
		    }
		}
	    } else
		printf("Syntax error in 'macro', ignored.\n");
	} else if (!strncmp(tmp, "shell ", 6)) {
	    if (shellkey >= 0)
		printf("Additional definition for 'shell' ignored.\n");
	    else
		shellkey = ctrl(tmp + 6);
	} else if (!strncmp(tmp, "new_edit ", 9)) {
	    if (tmp[9] == '1')
		userflags |= US_NEW_EDIT;
	    else
		userflags &= ~US_NEW_EDIT;
	} else if (!strncmp(tmp, "nowarnings ", 11)) {
	    if (tmp[11] == '1')
		userflags |= US_NOWARNING;
	    else
		userflags &= ~US_NOWARNING;
	} else if (!strncmp(tmp, "colour ", 7)) {
	    if (tmp[7] == '1') {
		userflags |= US_ANSIDEFAULT;
		userflags |= US_ANSICOLORS;
	    } else {
		userflags &= ~US_ANSIDEFAULT;
		userflags &= ~US_ANSICOLORS;
	    }
	} else if (*tmp != '#' && *tmp)
	    printf("Syntax error in .yawcbbsrc file in line %d.\n", l);
    }

    if (!bbsrc || !reads || (macrokey == -1))
	macrokey = 0x02;
    if (!bbsrc || !reads || (quitkey == -1))
	quitkey = 0x06;
    if (!bbsrc || !reads || (suspkey == -1))
	suspkey = 0x1a;
    if (!bbsrc || !reads || (capturekey == -1))
	capturekey = 0x19;
    if (!bbsrc || !reads || (connectkey == -1))
	connectkey = 0x03;
    if (!bbsrc || !reads || (shellkey == -1))
	shellkey = '!';

    if (quitkey >= 0 && *macro[quitkey])
	printf("Warning: duplicate definition of 'macro' and 'quit'");
    if (suspkey >= 0 && *macro[suspkey])
	printf("Warning: duplicate definition of 'macro' and 'susp'");
    if (capturekey >= 0 && *macro[capturekey])
	printf("Warning: duplicate definition of 'macro' and 'capture'");
    if (shellkey >= 0 && *macro[capturekey])
	printf("Warning: duplicate definition of 'macro' and 'shell'");
    if (quitkey >= 0 && quitkey == suspkey)
	printf("Warning: duplicate definition of 'quit' and 'susp'");
    if (quitkey >= 0 && quitkey == capturekey)
	printf("Warning: duplicate definition of 'quit' and 'capture'");
    if (quitkey >= 0 && quitkey == shellkey)
	printf("Warning: duplicate definition of 'quit' and 'shell'");
    if (suspkey >= 0 && suspkey == capturekey)
	printf("Warning: duplicate definition of 'susp' and 'capture'");
    if (suspkey >= 0 && suspkey == shellkey)
	printf("Warning: duplicate definition of 'susp' and 'shell'");
    if (capturekey >= 0 && capturekey == shellkey)
	printf("Warning: duplicate definition of 'capture' and 'shell'");

    if (!*bbshost[0])
	strcpy(bbshost[0], BBSHOST);

    if (!*editor)
	strcpy(editor, myeditor);
    if (bbsrc && systemtype == 1)
	fclose(bbsrc);


    for (i = 1; i <= argc; i++)
	if (argv[i] != NULL)
	    if ((strcmp(argv[i], "-?") == 0) || (strcmp(argv[i], "-l") == 0)
	      || (strcmp(argv[i], "?") == 0) || (strcmp(argv[i], "-P") == 0)
		|| (strncmp(argv[i], "-F", 2) == 0))
		return;

    if (insecure) {
	if (*master_pw_enc)
	    fprintf(stderr, "There are unencrypted passwords in your config file. Hit <c><c><space>\nto encrypt them once you're connected to a BBS.\n\n");
	else
	    fprintf(stderr, "There are unprotected passwords in your config file. You should set a\nmaster password (<c><c><l><m>) once you're connected to a BBS to encrypt them.\n\n");
    }
    if (*master_pw_enc) {
	i = get_old_pw("CLient master password", master_pw_enc, master_password, 1, 0);
	if (i) {
	    if (i == 1)
		printf("Incorrect master password.");
	    else
		printf("No master password given.");
	    printf(" Won't decrypt/change encrypted BBS passwords.\n");
	    mybzero(master_password, 20);
	} else {
	    for (i = 0; i < MAXSYS; i++)
		if (*bbspw_e[i]) {
		    sprintf(tmp, "%d%s", i, master_password);
		    strcpy(bbspass[i], do_des(tmp, bbspw_e[i], DECRYPT));
		    insecure = 0;
		    for (l = 0; l < strlen(bbspass[i]); l++)
			if (bbspass[i][l] & 128)
			    insecure = 1;
		    if (insecure) {
			printf("Warning: Password error for site #%d! Won't use the password.\n", i);
			*bbspass[i] = 0;
		    }
		}
	}
    }
    fflush(stdout);

}



/* Opens the bbsrc file, warning the user if it can't be opened or can't be
 * opened for write, returning the file pointer if it was opened
 * successfully. */
FILE *
openbbsrc()
{
    FILE *f;

    chmod(bbsrcname, 0600);
    f = fopen(bbsrcname, "r+");
    if (!f)
	f = fopen(bbsrcname, "w+");
    if (!f) {
	f = fopen(bbsrcname, "r");
	if (f) {
	    bbsrcro = 1;
	    printf("\n[Warning: configuration file is read-only]\n");
	} else {
	    f = fopen("/etc/yawcbbsrc", "r");
	    if (!f)
		printf("\n[Warning: configuration file not found, using defaults]\n");
	    else {
		printf("\n[Warning: Using system configuration file (/etc/yawcbbsrc)]\n");
		bbsrcro = 1;
	    }
	}
    }
    return (f);
}

/**************************************************************************
*
* save_bbsrc - moved here from configbbsrc() (config.c)
*
**************************************************************************/
void
save_bbsrc()
{
    int size, i, j, paragraph;
    char tmp[25];

    if (systemtype == 1)
	bbsrc = fopen(bbsrcname, "w");
    /* Calculate space requirements for bbsrc file */
    size = 8 + strlen(editor);	/* editor */

    if (*master_pw_enc)		/* encrypted master password */
	size += 9 + strlen(master_pw_enc);

    for (i = 0; i < MAXSYS; i++)
	if (strlen(bbshost[i]) > 0)
	    size += 8 + strlen(bbshost[i]);	/* site */

    for (i = 0; i < MAXSYS; i++)
	if (strlen(bbsname[i]) > 0)
	    size += 1 + strlen(bbsname[i]);	/* comment */

    for (i = 0; i < MAXSYS; i++)
	if (strlen(bbsuser[i]) > 0)
	    size += 10 + strlen(bbsuser[i]);	/* usernames */

    if (*master_password) {
	for (i = 0; i < MAXSYS; i++)
	    if (strlen(bbspass[i]) > 0)
		size += 25;	/* passwords */
    } else {
	for (i = 0; i < MAXSYS; i++)
	    if (strlen(bbspw_e[i]) > 0)
		size += 25;	/* passwords */
	    else if (strlen(bbspass[i]) > 0)
		size += 10 + strlen(bbspass[i]);
    }

    size += 3;			/* paragraphs */
    size += 11 + macrokey < 32;	/* macrokey */
    size += 7 + quitkey < 32;	/* quit */
    size += 7 + suspkey < 32;	/* susp */
    size += 7 + strlen(shell);	/* shell */
    size += 10 + capturekey < 32;	/* capture */
    size += 10 + connectkey < 32;	/* connect */
    size += 13;			/* nowarnings  -Flint */
    size += 9;			/* colour  -Flint */
    size += 9;			/* new_edit  -Flint */

    for (i = 0; i < friendp; i++)
	size += 8 + strlen(friend[i]);	/* friend */
    for (i = 0; i < enemyp; i++)
	size += 7 + strlen(enemy[i]);	/* enemy */
    for (i = 0; i < 128; i++)
	if ((i != 1) && *macro[i]) {
	    size += i < 32 ? 11 : 10;	/* macro *//*** PR: +1 */
	    for (j = 0; macro[i][j]; j++)
		size += macro[i][j] < 32 ? 2 : 1;
	}
    if (systemtype == 0)
	fseek(bbsrc, (long)size, SEEK_SET);
    if (systemtype == 0 && ftell(bbsrc) < size) {
	i = size - ftell(bbsrc);
	for (; i; i--)
	    if (putc('\n', bbsrc) < 0) {
		fflush(stdout);
		perror("\r\n[Local error saving new configuration]");
		fprintf(stderr, "\r\n");
	    }
	if (fflush(bbsrc) < 0) {
	    fflush(stdout);
	    perror("\r\n[Local error saving new configuration]");
	    fprintf(stderr, "\r\n");
	}
    }
    if (systemtype != 0 || ftell(bbsrc) >= size) {
	rewind(bbsrc);
	fprintf(bbsrc, "editor %s\n", editor);

	if (*master_pw_enc)	/* encrypted master password */
	    fprintf(bbsrc, "masterpw %s\n\n", master_pw_enc);

	paragraph = 0;
	for (i = 0; i < MAXSYS; i++)
	    if (strlen(bbshost[i]) > 0) {
		if (i < 10)
		    fprintf(bbsrc, "site%d %s", i, bbshost[i]);
		else
		    fprintf(bbsrc, "site%c %s", i + 55, bbshost[i]);
		if (strlen(bbsname[i]) > 0)
		    fprintf(bbsrc, " %s\n", bbsname[i]);
		else
		    fprintf(bbsrc, "\n");
		paragraph++;
	    }
	if (paragraph)
	    fprintf(bbsrc, "\n");

	paragraph = 0;
	for (i = 0; i < MAXSYS; i++)
	    if (strlen(bbsuser[i]) > 0) {
		if (i < 10)
		    fprintf(bbsrc, "username%d %s\n", i, bbsuser[i]);
		else
		    fprintf(bbsrc, "username%c %s\n", i + 55, bbsuser[i]);
		paragraph++;
	    }
	if (paragraph)
	    fprintf(bbsrc, "\n");

	paragraph = 0;

	if (*master_password) {
	    for (i = 0; i < MAXSYS; i++)
		if (strlen(bbspass[i]) > 0) {
		    sprintf(tmp, "%i%s", i, master_password);
		    if (i < 10)
			fprintf(bbsrc, "pwd_encr%d %s\n", i,
				do_des(tmp, bbspass[i], ENCRYPT));
		    else
			fprintf(bbsrc, "pwd_encr%c %s\n", i + 55,
				do_des(tmp, bbspass[i], ENCRYPT));
		} else if (strlen(bbspw_e[i]) > 0)
		    if (i < 10)
			fprintf(bbsrc, "pwd_encr%d %s\n", i, bbspw_e[i]);
		    else
			fprintf(bbsrc, "pwd_encr%c %s\n", i + 55, bbspw_e[i]);
	} else {
	    for (i = 0; i < MAXSYS; i++)
		if (strlen(bbspw_e[i]) > 0)
		    if (i < 10)
			fprintf(bbsrc, "pwd_encr%d %s\n", i, bbspw_e[i]);
		    else
			fprintf(bbsrc, "pwd_encr%c %s\n", i + 55, bbspw_e[i]);
		else if (strlen(bbspass[i]) > 0)
		    if (i < 10)
			fprintf(bbsrc, "password%d %s|\n", i, bbspass[i]);
		    else
			fprintf(bbsrc, "password%c %s|\n", i + 55, bbspass[i]);
	}

	for (i = 0; i < MAXSYS; i++)
	    if ((strlen(bbspw_e[i]) > 0) || (strlen(bbspass[i]) > 0))
		paragraph++;

	if (paragraph)
	    fprintf(bbsrc, "\n");

	fprintf(bbsrc, "macrokey %s\n", strctrl(macrokey));
	fprintf(bbsrc, "quit %s\n", strctrl(quitkey));
	fprintf(bbsrc, "susp %s\n", strctrl(suspkey));
	fprintf(bbsrc, "shell %s\n", strctrl(shellkey));
	fprintf(bbsrc, "capture %s\n", strctrl(capturekey));
	fprintf(bbsrc, "connect %s\n", strctrl(connectkey));
	fprintf(bbsrc, "nowarnings %d\n", (userflags & US_NOWARNING) ? 1 : 0);
	fprintf(bbsrc, "colour %d\n", (userflags & US_ANSIDEFAULT) ? 1 : 0);
	fprintf(bbsrc, "new_edit %d\n", (userflags & US_NEW_EDIT) ? 1 : 0);

	for (i = 0; i < friendp; i++)
	    fprintf(bbsrc, "friend %s\n", friend[i]);
	for (i = 0; i < enemyp; i++)
	    fprintf(bbsrc, "enemy %s\n", enemy[i]);
	for (i = 0; i < 128; i++) {
	    if ((i != 1) && *macro[i]) {
		fprintf(bbsrc, "macro %s ", strctrl(i));
		for (j = 0; macro[i][j]; j++)
		    fprintf(bbsrc, "%s", strctrl(macro[i][j]));
		fprintf(bbsrc, "|\n");	/*** PR: added "|" */
	    }
	}
	fflush(bbsrc);
	truncbbsrc(ftell(bbsrc));
    }
}
