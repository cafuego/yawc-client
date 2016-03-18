/* This is just a hacked-up version of the editor from the BBS...ugly, isn't
 * it?  You don't need to mess with this unless you have a lot of time to
 * waste on a lost cause.  Use a real editor, that is what '.edit' is for! */
/* PR: I fully agree with previous author; ditched his "edit" code and now
 * use get_string to provide line-edit capabilities. Shortened the code by
 * pages. */
#include "defs.h"
#include "ext.h"

#define COLORCHAR 1


void
old_getpost(khflag, fp, tmpstr)
    int *khflag;
    FILE *fp;
    char *tmpstr;
{
    int lnlngth;		/* line length				 */
    char thisline[81];		/* array to save current line		 */
    for (;;) {

	*khflag = 0;

	while (1) {
	    lnlngth = 48689 - ftell(fp);
	    if (lnlngth > 78)
		lnlngth = 78;
	    if (lnlngth < 1)
		break;
	    get_string(lnlngth, thisline, *khflag, 1);

	    if (*thisline) {
		tmpstr = (char *)realloc(tmpstr, strlen(tmpstr) + strlen(thisline) + 5);
		strcat(tmpstr, thisline);
		colour_so_far(tmpstr, 1);
	    } else
		break;

	    if (fputs(thisline, fp) < 0)
		tempfileerror();
	    if (putc('\n', fp) < 0)
		tempfileerror();
	    *khflag = 1;
	}

	if (fflush(fp) < 0)
	    tempfileerror();
	if (prompt(fp, khflag, '\n') < 0) {
	    free(tmpstr);
	    colour_so_far("", -1);
	    return;
	}
    }
}

void
new_getpost(khflag, fp, tmpstr)
    int *khflag;
    FILE *fp;
    char *tmpstr;
{
    char *textbuf;

    for (;;) {

	*khflag = 0;

	textbuf = (char *)malloc(48700);

	YAWC_editor(textbuf, 600, 78, E_2ENTEREND + E_COLORS, "");

	if (fputs(textbuf, fp) < 0)
	    tempfileerror();
	*khflag = 1;

	if (fflush(fp) < 0)
	    tempfileerror();
	if (prompt(fp, khflag, '\n') < 0) {
	    free(tmpstr);
	    colour_so_far("", -1);
	    return;
	}
    }
}

/*************************************************
*
* upload:	1 -> normal
*		2 -> upload (end with ^D)
*		3 -> editor directly [NOTMADE]
*
*************************************************/

void
makemessage(upload)
    int upload;
{
    int chr, colourflag = 0;	/* colourflag for ^N uploading 		 */
    FILE *fp;
    int old = '\n';
    int khflag = -1;		/* used for <E>diting directly.		 */
    char *tmpstr;		/* to store the colour coming from the
				 * previous line  -Flint */

    fp = tempfile;
    tmpstr = (char *)malloc(20);
    strcpy(tmpstr, curr_col);
    colour_so_far(tmpstr, 1);

    posting = 1;
    if (capture) {
	printf("\r\n[Capture to temp file turned OFF]\r\n");
	capture = 0;
	fflush(tempfile);
    }
    rewind(fp);
    if (lastsave) {
	(void)freopen(tempfilename, "w+", tempfile);
	lastsave = 0;
    }
    if (getc(fp) >= 0) {
	rewind(fp);
	printf("There is text in your edit file.  Do you wish to erase it? (Y/N) -> ");
	if (yesno())
	    (void)freopen(tempfilename, "w+", tempfile);
	else {
	    (void)checkfile(fp);
	    old = -1;
	}
    }
    if (upload == 3)		/* KHCHANGE: Editor directly	 */
	for (;;) {
	    khflag = -1;

	    if (prompt(fp, &khflag, 'e') < 0)
		return;

	    khflag = 0;

	    if (prompt(fp, &khflag, ' ') < 0)
		return;
	}

    if (upload == 2)		/* PR: simple ^D loop */
	for (;;) {
	    while ((chr = inkey()) != CTRL_D)
		if (ftell(fp) > 48700) {
		    printf("\r\nMessage too long, must Abort or Save\r\n\n");
		    fflush(stdout);
		    mysleep(1);
		    flush_input(0);
		} else if (putc(chr, fp) < 0)
		    tempfileerror();
		else {
		    if (chr == 1)
			colourflag = 1;
		    else {
			if (colourflag) {
			    sprintf(colst, "\1%c", chr);
			    colorize(colst);
			} else
			    putchar(chr);
			colourflag = 0;
		    }
		}
	    khflag = 0;
	    if (fflush(fp) < 0)
		tempfileerror();
	    if (prompt(fp, &khflag, chr) < 0)
		return;
	}

    if ((upload == 1) || (upload == 0)) {	/* line editing via
						 * get_string */
	if (userflags & US_NEW_EDIT)
	    new_getpost(&khflag, fp, tmpstr);
	else
	    old_getpost(&khflag, fp, tmpstr);
	return;
    }
    free(tmpstr);		/* should never be reached though... */
    colour_so_far("", -1);
}



/* This function used to be part of edit(), it was broken out because stupid
 * DEC optimizers found edit() too long to optimize without a warning, and
 * that warning made people think something went wrong in the compilation.
 * This also might even make this stuff easier for others to understand, but
 * I doubt it. */
int
prompt(fp, old, cmd)
    FILE *fp;
    int *old;
    int cmd;
{
    FILE *cp;
    int i;
    int chr = cmd;
    int lnlngth;
    int invalid = 0;
    int size;
    int lines;
    int last_was_ctrl_a = 0;	/* Flint: the name says it all =); used for
				 * 'p' */
    char thisline[80];
    char lastchar;

    for (i = 0;;) {
	if (*old != -1) {
	    if (i != 1) {

		/* The DOC'ish way was to send a
		 * <c>ontinue-character-"command" from right here. I have no
		 * idea why they wanted to make it like that, but i do know I
		 * won't have it like that. I was quite confused for some
		 * time... (c: */

		colorize("\1a\1f\1w<\1rA\1w>\1gbort \1w<\1rC\1w>\1gontinue \1w<\1rP\1w>\1grint \1w<\1rS\1w>\1gave \1w<\1rX\1w>\1gpress \1w<\1rE\1w>\1gdit \1w-> ");
		fflush(stdout);
	    }
	    i = 0;
	    while (!mystrchr("aAcCeEpPsSQtTx", chr = inkey())) {
		if (invalid++)
		    flush_input(invalid);
	    }

	    invalid = 0;
	}
	switch (chr) {

			case ' ':
			case '\n':
			    if (!i++)
				continue;
			    flush_input(i);
			    printf("\r\n");
			    continue;

			case 'a':
			case 'A':
			    printf("Abort: are you sure? ");
			    if (yesno()) {
				sendblock();
				netput(CTRL_D);
				if (TELNET && !DOC) {	/* DOC hack */
				    netput('\n');
				    netput('\n');
				}
				netput('a');
				if (TELNET && !DOC)	/* DOC hack */
				    netput('y');
				byte[curr_bbs] += 2;
				posting = 0;
				return (-1);
			    }
			    continue;

			case 'c':
			case 'C':
			    printf("Continue...\r\n");
			    colorize(colour_so_far("", 0));	/* set the previous
								 * colour */
			    break;

			case 'p':
			case 'P':
			    if (*old == -1)
				*old = '\n';
			    else
				/* cprintf("Print formatted\r\n\n%s\1a\1c",
				 * saveheader); */
				colorize("Print formatted\r\n\n\1a\1c");
			    /* Flint: added ^^^^^^-this to initialize colours */
			    fseek(fp, 0L, SEEK_END);
			    size = ftell(fp);
			    rewind(fp);
			    lines = 2;
			    lnlngth = i = 0;
			    while ((chr = getc(fp)) > 0) {
				i++;
				if (chr == COLORCHAR) {	/* Flint: added this to
							 * set last_was_ctrl_a */
				    last_was_ctrl_a = 1;
				    continue;
				}
				if (chr == TAB) {
				    do
					putchar(' ');
				    while (++lnlngth & 7);
				} else {
				    if (chr == '\n')
					putchar('\r');
				    if (last_was_ctrl_a) {	/* Flint: colour code?
								 * Then colourize! =) */
					ColourChar(chr);
					last_was_ctrl_a = 0;
				    } else
					putchar(chr);
				    lnlngth++;
				}
				if (chr == '\n' && !(lnlngth = 0) && ++lines == rows && more(&lines, i * 100 / size) < 0)
				    break;
			    }
			    fseek(fp, 0L, SEEK_END);
			    break;

			case 's':
			case 'S':
			    printf("Save message\r\n");
			    if (checkfile(fp))
				continue;
			    rewind(fp);
			    sendblock();
			    lastchar = 0;
			    while ((chr = getc(fp)) > 0) {
				if ((chr == 10) && (lastchar == 10)) {
				    netput(' ');
				    byte[curr_bbs]++;
				}
				netput(chr);
				lastchar = chr;
				byte[curr_bbs]++;
			    }
			    netput(CTRL_D);
			    if (TELNET && !DOC) {	/* DOC telnet hack */
				netput('\n');
				netput('\n');
			    }
			    netput('s');
			    byte[curr_bbs] += 2;
			    lastsave = 1;
			    posting = 0;
			    return (-1);

			    /* KHCHANGE - nothing to receive these at the
			     * BBS-end.
			     * 
			     * case 'Q': case 't': case 'T': case 'x': case '?':
			     * case '/': sendblock(); netput(CTRL_D);
			     * netput(chr); byte[curr_bbs] += 2; looper();
			     * netput('c'); continue; */

			case 'e':
			case 'E':
			    printf("Edit\r\n");
			    if (!*editor)
				printf("[Error:  no editor available]\r\n");
			    else {
				if (chr == 'E') {
				    fseek(fp, 0L, SEEK_END);
				    if (ftell(fp)) {
					printf("\r\nThere is text in your edit file.  Do you wish to erase it? (Y/N) -> ");
					if (yesno())
					    (void)freopen(tempfilename, "w+", tempfile);
					else
					    continue;
				    }
				    printf("\r\nFilename -> ");
				    get_string(67, thisline, -999, 0);
				    if (!*thisline)
					continue;
				    if (!(cp = fopen(thisline, "r"))) {
					printf("\r\n[Error:  named file does not exist]\r\n\n");
					continue;
				    } else {
					while ((i = getc(cp)) >= 0)
					    if (putc(i, fp) < 0) {
						tempfileerror();
						break;
					    }
					if (feof(cp) && fflush(fp) < 0)
					    tempfileerror();
					fclose(cp);
				    }
				}
				run(editor, tempfilename);
				printf("[Editing complete]\r\n");
				(void)freopen(tempfilename, "r+", tempfile);
				if (checkfile(fp)) {
				    fflush(stdout);
				    mysleep(1);
				}
				/* KH-Addition */

				if (*old == -1)
				    *old = 0;

			    }
			    continue;
	}
	return (0);
    }
}



/* Checks the file for lines longer than 79 characters, unprintable
 * characters, or the file itself being too long.  Returns 1 if the file has
 * problems and cannot be saved as is, 0 otherwise. */
int
checkfile(fp)
    FILE *fp;
{
    int i;
    int count = 0;
    int line = 1;
    int total = 0;

    rewind(fp);
    while (!feof(fp))
	if ((i = getc(fp)) != '\r' && i != '\n') {
	    if (i == COLORCHAR)
		count -= 2;
	    if ((i >= 0 && i < 32 && i != TAB && i != COLORCHAR) ||
		(i >= DEL && (mystrchr("äöüÄÖÜß", i) != NULL))) {
		printf("\r\n[Warning:  illegal character in line %d, edit file before saving]\n\n", line);
		return (1);
	    } else if ((count = i == TAB ? (count + 8) & 0xf8 : count + 1) > 79) {
		printf("\r\n[Warning:  line %d too long, edit file before saving]\r\n\n", line);
		return (1);
	    }
	} else {
	    total += count;
	    count = 0;
	    line++;
	}
    if (total > 48800) {
	printf("\r\n[Warning:  message too long, edit file before saving]\r\n\n");
	return (1);
    }
    if (total < 1) {
	printf("\r\n[Warning:  temp file empty, can't save it]\r\n\n");
	return (1);
    }
    return (0);
}
