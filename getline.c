/* This code is partially GPL'ed, so I'll GPL the rest for y'all.
 * 
 * This code originally came out of Thanatos, so it should be most interesting
 * for all the other Sysops... ;~)
 * 
 * This is, as it was, code ripped from the actual BBS, but with subtle
 * additions so it can use the old nasty stdio functions instead of cprintf
 * at full power.
 * 
 * Actually, I'll probably bundle cprintf into the CLient, since it's GPL'd
 * anyway... (It's a heavily modified bit of Linux kernel code...)
 * 
 * Right. Things that this code does that the PR didn't:
 * 
 * We can do inline editing, like Thanatos can on a telnet connection. You can't
 * go up and down lines, but up takes you to the beginning of the line, and
 * down takes you to the end. Left and right are as normal. ^E and ^A and ^F
 * and ^B do nothing, yet. Perhaps they should.
 * 
 * -  Diamond White Dave.
 * 
 * (C)1500BC Serendipity (Or something) (C)1995 - 1996 MathFox (Or Peter
 * Roozemaal) (C)1996 - Diamond White (Or Dave Cridland) Some fixes by Flint
 * (Or Sven Winnecke =) */
#include "defs.h"
#include "ext.h"
#include <unistd.h>

#define MAXALIAS	20
#define MAXNAME		40

static void cputs_ins P((char *, int, char *, int));

/**********************************************************************
*
*  colour_so_far() - builds and returns a string to set the colours
*  mode: 0 = ignore string-so_far, just return the last colour string
*        1 = build a new colour string, set active_status = 1.
*	-1 = mark the routine as inactive, return "".		-Flint
*
***********************************************************************/
char *
colour_so_far(string_so_far, mode)
    char *string_so_far;
    int mode;
{
    static char colour_string[20];
    static int active_status = 0;
    int i, j, key;

    if (mode == -1) {
	active_status = 0;
	return ("");
    }
    if (mode == 0)
	if ((active_status == 1) && !TELNET)
	    return (colour_string);
	else
	    return ("");

    active_status = 1;

    strcpy(colour_string, "\1a\1 \1 \1 \1 \1 \1 ");
    for (i = 0; i < strlen(string_so_far) - 1; i++) {
	if (string_so_far[i] == 1) {
	    key = string_so_far[i + 1];
	    if (mystrchr("drgybpcw", key))
		colour_string[13] = key;
	    if (mystrchr("DRGYBPCW", key))
		colour_string[15] = key;
	    if (key == 'f')
		colour_string[3] = key;
	    if (key == 'e')
		colour_string[5] = key;
	    if (key == 'u')
		colour_string[7] = key;
	    if (key == 'i')
		colour_string[9] = key;
	    if (key == 'h')
		colour_string[11] = key;
	    if (key == 'a')
		for (j = 1; j < 8; j++)
		    colour_string[2 * j + 1] = ' ';
	}
    }
    return (colour_string);
}

/* get_name() ***** LJ/Animal replacement exists for ***** Usernames... see
 * express_multi.c And we'll be using that, too... But at a later date,
 * methinks. Once me and Martin and PeteR and Flint and... *insert your name
 * here* have finally figured out a way for CLients and Servers to talk to
 * each other. Used for getting names (usernames, roomnames, etc.)
 * Capitalizes first letter of word auto- magically. Does different things
 * depending on the value of quit_priv. The name is then returned to the
 * caller.
 * 
 * IF quit_priv = 1  ->  one can use CTRL-D to break the input...
 * 
 * quit_priv = 2  ->  get a 20 characters name (UserName)
 * 
 * quit_priv = 3  ->  get a 40 characters name (RoomName); changed so that you
 * can use *;:()and so on ...now also with TABfeature! ...Now without TAB
 * feature again... *laugh*
 * 
 * quit_priv = 4  ->  get a 20 characters name (UserName) with TABfeature!
 * 
 * quit_priv = 5  ->  get a Mail recipient name. TAB expansion on hostnames
 * proposed. Never did that. I did TAB expand through wholist as always,
 * instead. Much easier... :~)
 * 
 * This won't break CLient code... Why? Because quit_priv==5 is used for a
 * normal quit_priv==2 on both YAWC and ISCA CLients. quit_priv = 6  -> Same
 * as 2, but with comma support - for Multi-X/Multi-Mail */

char *
get_name(quit_priv)
    int quit_priv;

#define VALIDCHARS "1234567890;:.,_-+=*!&()/'?@$"
/* these are useable when quit_priv == 3 */
#define MAILVALIDCHARS "0123456789-@._"
/* Extra ones for email names and stuff. */
{
    static char pbuf[81];
    register char *p;
    register char c;
    int upflag;
    int fflag;
    int pblen = 0;
    int invalid = 0;
    int comma = 0;
    int a /* , b */ ;
    /* struct wholist *pointer; */

    for (a = 0; a < 40; a++)
	pbuf[a] = 0;

    /* for comma support 6 is like 2 so.. move to 2 ! */
    if (quit_priv == 6) {
	comma = 1;
	quit_priv = 2;
    }
    for (;;) {
	upflag = fflag = 1;
	a = 0;

	p = pbuf;

	for (;;) {
	    c = inkey();

	    if (c == '\n')
		break;

	    if (comma && c == ',') {
		break;
	    }
	    if ((c == CTRL_D || c == 3) && quit_priv == 1) {
		pbuf[0] = CTRL_D;
		pbuf[1] = 0;
		return (pbuf);
	    }
	    if (c == TAB && (quit_priv == 2 || quit_priv == 5)) {
		for (; wholist[a][0] != 0 && a < MAXWHO; a++) {
		    if (strncmp(pbuf, wholist[a], pblen) == 0) {
			if (strcmp(pbuf, wholist[a]) != 0) {
			    back(strlen(pbuf));

			    p = strcpy(pbuf, wholist[a]) + strlen(wholist[a]);

			    printf(pbuf);
			    break;
			}
		    }
		}

		if (!(*wholist[a]))
		    a = 0;	/* start from the beginning in the wholist
				 * again */

		continue;	/* read a new inkey... */
	    }
	    if (c == CTRL_N && (quit_priv == 2 || quit_priv == 5)) {
		if (!*wholist[a])
		    if (!*wholist[0])
			continue;
		    else
			a = 0;

		back(strlen(pbuf));
		p = strcpy(pbuf, wholist[a]) + strlen(wholist[a]);
		printf(pbuf);
		a++;
	    }
	    if (c == CTRL_K && (quit_priv == 2 || quit_priv == 5)) {
		if (a < 0)
		    for (a = 199; a >= 0; a--)
			if (*wholist[a])
			    break;

		if (a < 0)
		    continue;

		back(strlen(pbuf));
		p = strcpy(pbuf, wholist[a]) + strlen(wholist[a]);
		printf(pbuf);
		a--;
	    }
	    if ((c == '_') && (quit_priv != 5))
		c = ' ';

	    if ((c == ' ') && (fflag || upflag))
		continue;

	    if (c == '\b' || c == CTRL_X || c == ' ' || isalpha(c) ||
		(mystrchr(VALIDCHARS, c) && (quit_priv == 3))
		|| (mystrchr(MAILVALIDCHARS, c) && (quit_priv == 5)))
		invalid = 0;
	    else {
		invalid++;
		/* if (invalid++)  flush_input(invalid); */
		continue;
	    }
	    do
		if ((c == '\b' || c == CTRL_X) && p > pbuf) {
		    back(1);
		    --p;
		    pblen--;
		    upflag = (p == pbuf || *(p - 1) == ' ');
		    if (p == pbuf)
			fflag = 1;

		    pbuf[strlen(pbuf)] = 0;
		    *p = 0;
		} else if ((p < &pbuf[!quit_priv || quit_priv == 3 ? MAXNAME : (quit_priv == 5 ? 80 : MAXALIAS)]
		      && (isalpha(c) || c == ' ' || (mystrchr(VALIDCHARS, c)
						     && quit_priv == 3) || (mystrchr(MAILVALIDCHARS, c) && quit_priv == 5)))) {
		    fflag = 0;

		    if (upflag && islower(c))
			c -= 32;

		    upflag = 0;	/* hack to make space working after '*' f.e. */

		    if (c == ' ')
			upflag = 1;
		    *p++ = c;
		    putchar(c);
		}
	    while ((c == CTRL_X || c == CTRL_W) && p > pbuf);

	    pblen = strlen(pbuf);
	}
	*p = 0;
	break;
    }

    if (c != ',' && (p > pbuf || quit_priv >= 2))
	printf("\r\n");

    if (p > pbuf && p[-1] == ' ')
	p[-1] = 0;

    if (comma && c == ',')
	strcat(pbuf, ",");

    return (pbuf);
}

#define ESCAPE_CODE 27


/* getline()
 * 
 * Now, hopefully, does inline editing. - DWD
 * 
 * string - Pointer to string buffer lim - Maximum length; if negative, no-show
 * nocol - flag: 1 -> no colors allowed.
 * 
 */

int
new_getline(prompt, string, lim, nocol, line, LastLine, chatmode)
    char *prompt;
    char *string;
    int lim;
    int nocol;
    int line;
    int LastLine;
    int chatmode;
{
    static char Wrap[200];	/* For holding the wrapped bits. */
    unsigned char a;		/* This variable does something. */
    int b = 0;			/* So does this one, I think... ;~) Okay. I
				 * am using 'b' for what I *think* KH was
				 * using it for... a pointer to the insert
				 * point. */
    char ins[3];
    char flag = 0, colorflag = 0, TerminalCode = 0, StickToColour = 1,	/* for inserting
									 * character left of a
									 * colour code */
         Escape = 0;
    int noc, i = 0, j, k;	/* # of colours for ^U and ^W, and some more
				 * ints =) */
    char *Colours = "afeuirgbcpywdDRGYBPCW";
    char *tmpstr;

    if (lim < 0) {
	lim = (0 - lim);
	flag = 1;
    }
    if (lim > 128) {
	lim = (256 - lim);
	flag = 1;
    }
    strcpy(string, "");
    colorize(prompt);

    /* if we have a wrap, and we had previous lines */
    if (strcmp(Wrap, "") && (line > 0) && (line <= LastLine)) {
	strcat(string, Wrap);
	b = strlen(string);
	strcpy(Wrap, "");
	colorize(string);
    }
    if (line == 0) {
	strcpy(Wrap, "");
    }
    ins[0] = 0;
    ins[1] = 0;
    ins[2] = 0;

    for (;;) {
	a = (inkey() /* &127 */ );	/* Trim off eighth bit. -Why? -Flint */

	if (TerminalCode) {
	    StickToColour = 0;
	    switch ((int)a) {
				/* up */
			    case 'A':
				{
				    int Temp;
				    for (Temp = b; Temp != 0; Temp--)
					if (string[Temp] != 1 && string[Temp > 0 ? Temp - 1 : Temp] != 1)
					    putchar(8);
				}
				b = 0;
				break;

				/* down */
			    case 'B':
				{
				    int Temp;
				    for (Temp = b; Temp != strlen(string); Temp++)
					if (string[Temp] != 1 && string[Temp > 0 ? Temp - 1 : Temp] != 1)
					    printf("\033[C");
				}
				b = strlen(string);
				break;

				/* right */
			    case 'C':
				if (b < strlen(string)) {	/* We don't want to go
				     * past the end *//* of the string */
				    b++;
				    /* Flint: now skip any colour codes */
				    while (string[b] == 1)
					b += 2;
				    printf("\033[C");
				}
				break;


				/* left */
			    case 'D':
				if (b > 0) {
				    b--;
				    /* Flint: again skip colour codes if any */
				    while ((string[b - 1] == 1) && (b > 2))
					b -= 2;
				    printf("\033[D");
				}
				break;	/* ... or before the beginning. */

	    }
	    TerminalCode = 0;
	    continue;
	}
	if (Escape) {
	    if (a == '[')
		TerminalCode = 1;
	    Escape = 0;
	    continue;
	}
	if ((a == '\1') && (!colorflag) && (nocol != 1)) {
	    colorflag = 1;	/* So we know to intercept the colour flag
				 * when we get it. */
	    continue;
	}
	if (a == '\1')
	    continue;

	if (colorflag == 1) {
	    if (memchr(Colours, a, strlen(Colours))) {
		ins[0] = 1;
		ins[1] = a;
		ins[2] = 0;
		cputs_ins(string, b, ins, flag);
		b += 2;
	    }
	    colorflag = 0;	/* Yeah, like I forgot this the first few
				 * times around... */
	    StickToColour = 1;
	    continue;
	}
	if (a == ESCAPE_CODE) {
	    Escape = 1;
	    continue;
	}
	if ((a < 32) && (a != 1) && (a != 8) && (a != CTRL_X) && (a != CTRL_W) && (a != '\n'))
	    a = 32;

	if ((a == 8) && ((strlen(string) == 0) || (b == 0)))	/* backspace at
								 * beginning of line */
	    continue;

	if ((a == 21) && (strlen(string) == 0))	/* ^U at beginning of line */
	    continue;

	if (a == CTRL_X) {	/* ^U someplace else on line */
	    noc = 0;
	    for (i = 0; i < strlen(string); i++)
		if (string[i] == 1)
		    noc += 2;

	    for (i = b; i != strlen(string); i++)
		if (string[i] != 1 && string[i > 0 ? i - 1 : i] != 1)
		    printf("\033[C");

	    back(strlen(string) - noc);
	    b = 0;
	    colorize(colour_so_far("", 0));
	    strcpy(string, "");	/* clear the string buffer */
	    StickToColour = 1;
	    Wrap[0] = 0;
	    continue;
	}
	if (a == CTRL_W) {	/* delete word, unix-style (bash-style anyway
				 * ;) -Flint */
	    if ((strlen(string) == 0) || (b == 0))	/* no string left of the
							 * cursor */
		continue;

	    while ((string[b - 2] == 1) && (b > 1))
		b -= 2;
	    i = b;
	    if ((string[i] != ' ') && (string[i - 1] == ' '))
		i--;
	    while ((string[i] == ' ') && (i >= 0))	/* read over current
							 * spacing */
		i--;
	    if (i == -1)	/* there are only spaces left of cursor */
		continue;
	    while ((string[i] != ' ') && (i >= 0))
		i--;		/* find the beginning of the word */
	    i++;
	    while (string[i] == 1)
		i += 2;

	    for (j = i; j < b; j++)	/* now set the new cursor position */
		if (string[j] == 1)
		    j++;
		else
		    putchar('\b');

	    colorize(colour_so_far("", 0));	/* set colours */
	    for (j = 0; j < i; j++)
		if (string[j] == 1) {
		    sprintf(colst, "\1%c", string[j + 1]);
		    colorize(colst);
		}
	    colorize(&string[b]);	/* overwrite the rest of the line */
	    for (j = i; j < b; j++)
		if (string[j] == 1)
		    j++;
		else
		    putchar(' ');

	    for (j = i; j < strlen(string); j++)	/* now back to the new
							 * position */
		if (string[j] == 1)
		    j++;
		else
		    putchar('\b');

	    k = strlen(string) - b;	/* # of bytes at the rest of the line */
	    for (j = 0; j <= k; j++)
		string[i + j] = string[b + j];	/* move rest of line over
						 * deleted word */
	    if ((b >= lim - 1) && (i <= 0))	/* Wrap fix */
		Wrap[0] = 0;
	    b = i;		/* set buffer pointer */

	    while (string[b] == 1)
		b += 2;
	    StickToColour = 0;
	    continue;
	}
	if ((a == ' ') && (strlen(string) >= lim))	/* <space> at EOL hack
							 * -Flint */
	    a = '\n';

	if ((a != '\n') && (a != 8) && (strlen(string) >= lim))
	    /* we have reached the end of the line */
	{
	    if (line < LastLine || LastLine == -1) {
		char *lastword;

		lastword = string;
		while (*lastword)
		    lastword++;
		while (lastword != string && !mystrchr(" -,", *lastword))
		    lastword--;
		if (lastword == string) {
		    Wrap[0] = a;
		    Wrap[1] = 0;
		} else {
		    int Temp;

		    strcpy(Wrap, (mystrchr(" -.,", *lastword)) ? lastword + 1 : lastword);
		    Temp = strlen(Wrap);
		    Wrap[Temp++] = a;
		    Wrap[Temp] = 0;

		    for (Temp = b; Temp != strlen(string); Temp++)
			if (string[Temp] != 1 && string[Temp > 0 ? Temp - 1 : Temp] != 1)
			    printf("\033[C");

		    lastword++;
		    back(strlen(lastword));	/* Kill the bit at the end. */
		    *lastword = 0;
		    printf(" \n");
		    return (0);
		}
	    }
	    putc('\n', stdout);
	    return (0);
	    continue;
	}
	if ((a == 8) && (string[0] != 0) && (b != 0)) {	/* Backspace (delete
							 * char to left.) */
	    int i, j, col_remove;

	    col_remove = 0;
	    i = strlen(string);

	    if (string[b - 2] == 1)
		col_remove = 1;

	    if (b == i) {	/* backspace on the last position */
		if (col_remove) {
		    b -= 2;
		    string[b] = 0;
		    tmpstr = (char *)malloc(21 + i);	/* correct the current
							 * colour */
		    sprintf(tmpstr, "%s%s", colour_so_far("", 0), string);
		    colorize(colour_so_far(tmpstr, 1));
		    tmpstr[14] = 0;
		    colour_so_far(tmpstr, 1);
		    continue;
		} else {
		    string[--b] = 0;
		    back(1);
		    continue;
		}
	    } else {
		for (j = (b - 1 - col_remove); j != i; j++)
		    string[j] = string[j + 1 + col_remove];

		if (!(col_remove))
		    putchar('\b');
		else
		    b--;
		cputs_ins(string, --b, "", flag);
		continue;
	    }
	}
	if ((a == '\n') || (a == 10)) {
	    putc('\n', stdout);
	    return (0);
	}
	if ((a == '/') && (chatmode) && (!strlen(string))) {
	    colorize("\1r\1fCommand \1w(? for help): ");
	    a = inkey();
	    back(22);

	    return (a);
	}
	k = 0;
	if (!StickToColour) {	/* special insert-before-colour hack */
	    while ((string[b - 2] == 1) && (b > 1)) {
		b -= 2;
		k += 2;
	    }
	}
	ins[0] = a;
	ins[1] = 0;
	cputs_ins(string, b++, ins, flag);
	b += k;
    }
}

/* cputs_ins()
 * 
 * puts a string, inserting a string insert at b. string must have a byte free
 * at the end, incidentally. The cursor is put after the string inserted. If
 * Secret is set, then we print '*' instead.
 * 
 */


static void
cputs_ins(string, b, insert, Secret)
    char *string;
    int b;
    char *insert;
    int Secret;
{
    char buf[100];
    char buf2[100];
    char *p;
    int ColNum, Temp;

    /* Special case for Lisa :~) */

    if (b == strlen(string) && strlen(insert)) {
	strcat(string, insert);
	if (Secret) {
	    sprintf(colst, "%s", insert);
	    for (p = colst; *p; p++)
		if ((*p != 1) && (*p != 0))
		    *p = '*';
		else if (*(p + 1) != 0)
		    p++;
	    colorize(colst);
	} else
	    colorize(insert);
	return;
    }
    if (b != 0) {
	strcpy(buf, string);
	buf[b] = 0;
	for (Temp = 0, ColNum = 0; Temp != strlen(buf); Temp++) {
	    if (buf[Temp] == 1) {
		Temp++;
		ColNum++;
	    }
	}
	for (Temp = 0; Temp != (strlen(buf) - (ColNum * 2)); Temp++)
	    putchar('\b');
    } else
	strcpy(buf, "");
    strcpy(buf2, string + b);
    strcat(buf, insert);
    strcat(buf, buf2);
    colorize(colour_so_far("", 0));
    if (Secret) {
	sprintf(colst, "%s", insert);
	for (p = colst; *p; p++)
	    if ((*p != 1) && (*p != 0))
		*p = '*';
	    else if (*(p + 1) != 0)
		p++;
	colorize(colst);
    } else
	colorize(buf);

    printf(" \b");

    for (Temp = 0, ColNum = 0; Temp != strlen(buf2); Temp++) {
	if (buf2[Temp] == 1) {
	    Temp++;
	    ColNum++;
	}
    }
    for (Temp = 0; Temp != (strlen(buf2) - (ColNum * 2)); Temp++)
	putchar('\b');
    strcpy(string, buf);
}

/* Used for getting X's and profiles.  'which' tells which of those two we
 * are wanting, to allow the special commands for X's, like PING and ABORT.
 * When we've got what we need, we send it immediately over the net.
 * 
 * which: 1 -> X	(7 "lines") 2 -> Profile	(5 "lines")
 * 
 * let me (KHaglund) explain the lines-deal now: if you're entering an X (which
 * == 1), this function will NOT prompt for 7 lines (7 is the default
 * X-lines-value on the YAWC's), it will let you type in 7 lines times 80
 * characters. Actually I don't know why nobody has made it like this before
 * me, since it's ridiculous to have it the old, bad way.
 * 
 * There is, however, a limit for each of these: you can only type in 20 lines
 * for an X, and 10 lines for a profile.
 * 
 * KHReminder: should probably use which as ANDed flags, like it was before...
 * 
 * Yummy. Nice protocol mangler. So what about Thanatos, which attempts to use 7
 * line protocols??? *sighs* Have to change them to infinite length, then. -
 * DWD */

void
get_some_lines(which)
    int which;
{
    register int i;
    register int j;
    register int k;
    char send_string[20][80];
    char *tmpstr;
    int total_sofar = 0;
    int total_allowed;
    int howmanytoget = 0;

    if (debugging)
	printf("<which = %d>\n", which);

    total_allowed = (((which & 1) == 1 && !DOC) ? 7 : 5) * 79;

    tmpstr = (char *)malloc(20);

    strcpy(tmpstr, curr_col);
    colour_so_far(tmpstr, 1);

    for (i = 0; i < ((DOC) ? 5 : (((which & 1) == 1) ? 20 : 10)) && (!i || *send_string[i - 1]); i++) {
	putchar('>');
	colorize(colour_so_far("", 0));

	howmanytoget = (total_allowed - total_sofar >= 78) ? 78 : (total_allowed -
							   total_sofar - 2);

	if (howmanytoget < 1)
	    break;

	get_string(howmanytoget, send_string[i], i, 1);	/* PR 0 -> 1: kleur */
	/* He means "Colour". He's Dutch, though. We can excuse him the
	 * spelling. Unlike the Americans... ;~) DWD */
	total_sofar += strlen(send_string[i]) + 1;

	if (*send_string[i]) {
	    tmpstr = (char *)realloc(tmpstr, strlen(tmpstr) + strlen(send_string[i]) + 5);
	    strcat(tmpstr, send_string[i]);
	    colour_so_far(tmpstr, 1);
	}
	if (((which & 1) == 1 && ((strcmp(send_string[i], "ABORT") == 0) ||
				  (strcmp(send_string[i], "PING") == 0)))) {
	    if (strcmp(send_string[i], "ABORT") == 0) {
		strcpy(send_string[0], "ABORT");
		i = 1;
	    } else {		/* PING */
		send_string[0][0] = 0;
		i = 1;
	    }
	    break;
	}
    }
    sendblock();
    for (j = 0; j < i; j++) {
	for (k = 0; send_string[j][k]; k++)
	    netput(send_string[j][k]);

	netput('\n');
	byte[curr_bbs] += k + 1;
    }

    if (!TELNET && !DOC && (strcmp(send_string[0], "ABORT") != 0)) {
	netput('\0');
	byte[curr_bbs]++;
    }
    colour_so_far(tmpstr, -1);	/* delete old colour settings */

    free(tmpstr);
}


/* Used for getting names (user names, room names, etc.)  Capitalizes first
 * letter of word automatically)  Does different things depending on the
 * value of quit_priv (that stuff should be left alone)  The name is then
 * returned to the caller.
 * 
 * quit_priv: 1 -> <Ctrl-D> possible to break 2 -> UserName (20 chars long) with
 * TABfeature. 3 -> RoomName (40 chars long) 5 -> [NOTMADE] like 2, but with
 * numbers allowed.
 * 
 */

/* Code removed. It basically does the same stuff as that code up there
 * anyway. */

/* Gets a generic string of length length, puts it in result and returns a a
 * pointer to result.  If the length given is negative, the string is echoed
 * with '.' instead of the character typed (used for passwords)
 * 
 * otherflag:	1 -> colors allowed. 9 -> ChatMode-extra-stuff */

void
get_string(length, result, line, otherflag)
    int length;
    char *result;
    int line;
    int otherflag;
{
    int a;

    a = new_getline("" /* Prompt. */ , result, length, (otherflag != 1), line, 20 /* LastLine. WTF? */ , (otherflag == 9));
    if (a != 0) {
	/* Flint: what was this intended to be? I'll replace it with an
	 * sprintf. */
	/* strcpy(result, "a"); result=(char)a; */
	sprintf(result, "/%c", a);
    }
}
