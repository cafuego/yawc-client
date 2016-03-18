/***************************************************************************
yawc_edit.c - YAWC inline editor
---------------------------------

The code is taken from the Jassyca BBS code so there might be some features
that aren't used in the CLient environment. This editor is not finished yet,
I'll just stick it in for experimental reasons. The whole thing will only
work an an ANSI (or on most vt100) compatible terminals incidentially.

Functions:
strrkhr			- string function, see function header below
alloc_line		- allocates space for a new line
expand_line		- allocates _more_ space for an existing line
line_forw		- sets the current line to the next line in buffer
line_back		- sets the current line to the previous line in buffer
trim_line		- removes trailing characters from a string
print_line		- prints a (colourized) line to the screen
reprint_colours		- restores colours up to a certain position in buffer
get_realpos		- gets cursor position in line considering colour codes
set_realpos		- sets cursor position in line considering colour codes
reprint_whole_line	- what it says
get_len			- get line length without trailing spaces
YAWC_editor		- the main routine
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "ext.h"

struct buffer_lines {		/* basic buffer line struct */
    int len;			/* currently allocated # of bytes */
    char *data;			/* the string itself 		  */
    struct buffer_lines *prev;	/* previous line		  */
    struct buffer_lines *next;	/* next line			  */
};

/***********************************************************
*
* strrkhr()
* Function: returns the pointer to the last character in the
* string s that is _not_ (int) c and not \0. Used to remove
* unnecessary <space>'s at the end of a string, in KH_editor
* for example. I think it's quite useful.       -KHaglund
*
***********************************************************/

char *
strrkhr(s, c)
    const char *s;
    int c;
{
    const char *p;

    p = s;
    for (; *s; ++s);
    for (; s >= p && (*s == (char)c || !*s); --s);
    return (char *)(s + 1);
}

/***********************************************************
*
* alloc_line()
* allocates a new buffer (10 safety bytes) line and filles it with spaces
*
************************************************************/
struct buffer_lines *
alloc_line(length)
    int length;
{
    struct buffer_lines *tmp;

    tmp = (struct buffer_lines *)malloc(sizeof(struct buffer_lines));
    tmp->data = (char *)malloc(length + 11);
    memset(tmp->data, ' ', length + 10);
    tmp->data[length] = '\0';
    tmp->len = length;
    tmp->next = NULL;
    return (tmp);
}

/***********************************************************
*
* expand_line()
* makes space for 20 more characters on the current line
*
************************************************************/
void
expand_line(line)
    struct buffer_lines *line;
{
    line->data = (char *)realloc(line->data, line->len + 20);
    line->len += 20;
    memset(&line->data[line->len - 20], ' ', 20);
}

/***********************************************************
*
* line_forw()
* go to next line in virtual buffer, allocate one if necessary.
*
************************************************************/
struct buffer_lines *
line_forw(curr, length)
    struct buffer_lines *curr;
    int length;
{
    struct buffer_lines *tmp;

    if (curr->next == NULL) {
	tmp = alloc_line(length);
	curr->next = tmp;
	tmp->prev = curr;
	return (tmp);
    } else
	return (curr->next);
}

/***********************************************************
*
* line_back()
* go to previous line in virtual buffer.
*
************************************************************/
struct buffer_lines *
line_back(curr)
    struct buffer_lines *curr;
{
    if (curr->prev != NULL)
	return (curr->prev);
    else
	return (curr);
}

/***********************************************************
*
* trim_line()
* st will be truncated by cutting off any trailing spaces
*
************************************************************/
void
trim_line(st)
    char *st;
{
    char *t1;

    t1 = strrkhr(st, ' ');
    *t1 = '\0';
}

/***********************************************************
*
* print_line()
* puts the line to the screen.
*   mode: 0 = just print it,
*	  1 = print and restore cursor position afterwards, appends 1 space
*	  2 = print and restore cursor position afterwards, no appending
*
* Function highly depends on the ANSI sequences <ESC><7> and <ESC><8>.
*
************************************************************/
void
print_line(line, mode)
    char *line;
    int mode;
{
    char *tmp, *tmp2;

    tmp = (char *)malloc(strlen(line) + 2);
    tmp2 = (char *)malloc(strlen(tmp) + 1);
    strcpy(tmp, line);
    if (strlen(tmp) > 0)
	trim_line(tmp);
    if ((strlen(tmp) > 0) || (mode == 1))
	switch (mode) {
			case 0:
			    colorize(tmp);
			    break;
			case 1:
			    sprintf(tmp2, "7%s 8", tmp);
			    colorize(tmp2);
			    break;
			case 2:
			    sprintf(tmp2, "7%s8", tmp);
			    colorize(tmp2);
			    break;
	}
    free(tmp);
    free(tmp2);
}

/***********************************************************
*
* reprint_colours()
* Reprints all colour codes up to pos p_x in curr->data to
* set it correctly.
* should be changed to collect the codes into one string and
* print that one.
* I wanted to avoid any global variable, hence the number of
* arguments... :')
*
************************************************************/
void
reprint_colours(anchor, curr, p_x, flags, pc)
    struct buffer_lines *anchor;
    struct buffer_lines *curr;
    int p_x;
    int flags;
    char *pc;
{
    struct buffer_lines *tmp;
    char *p, tmp2[3];
    int i;

    colorize(pc);

    tmp = anchor;
    while (tmp != curr) {
	for (i = 0; i < strlen(tmp->data); i++)
	    if (tmp->data[i] == 1) {
		sprintf(tmp2, "\1%c", tmp->data[i + 1]);
		colorize(tmp2);
	    }
	tmp = tmp->next;
    }

    for (i = 0; i < p_x; i++)
	if (tmp->data[i] == 1) {
	    sprintf(tmp2, "\1%c", tmp->data[i + 1]);
	    colorize(tmp2);
	}
    p = (char *)malloc(5);	/* fix to set the cursor colour on some
				 * xterms */
    sprintf(p, "%c\b", tmp->data[p_x]);
    colorize(p);
    free(p);
}

/***********************************************************
*
* get_realpos()
* determining the real line position by skipping the colour codes
*
************************************************************/
int
get_realpos(curr, p_x)
    struct buffer_lines *curr;
    int p_x;
{
    int i, j = -1;
    for (i = 0; i <= p_x; i++)
	if (curr->data[i] != 1)
	    j++;
	else
	    i++;
    return (j);
}

/***********************************************************
*
* set_realpos()
* returns p_x for the real line position p (without colorus)
*
************************************************************/
int
set_realpos(curr, p)
    struct buffer_lines *curr;
    int p;
{
    int i, j = -1;

    for (i = 0; i <= p; i++) {
	j++;
	while (curr->data[j] == 1)
	    j += 2;
    }
    return (j);
}

/***********************************************************
*
* reprint_whole_line()
* reprints the whole current line (in case it was off the
* screen until now
*   mode: 0 = cursor moved down, 1 = cursor moved up, 2 = don't move
*
************************************************************/
void
reprint_whole_line(prompt, curr, anchor, top_visual, bottom_visual, p_y, flags, mode, pc)
    char *prompt;
    struct buffer_lines *curr;
    struct buffer_lines *anchor;
    int *top_visual;
    int *bottom_visual;
    int p_y;
    int flags;
    int mode;
    char *pc;
{
    char *p;

    if (mode == 0) {
	*top_visual += (p_y - *bottom_visual);
	*bottom_visual = p_y;
    }
    if (mode == 1) {
	*bottom_visual -= (*top_visual - p_y);
	*top_visual = p_y;
    }
    printf("7\r");
    colorize(prompt);
    p = (char *)malloc(strlen(curr->data + 1));
    strcpy(p, curr->data);
    trim_line(p);
    if (strlen(p) > 0) {
	reprint_colours(anchor, curr, get_realpos(curr, 0), flags, pc);
	colorize(p);
	printf("8");
    } else
	printf("8");
    free(p);
}

/***********************************************************
*
* get_len()
* determining the string length without trailing spaces and
* without colour codes
*
************************************************************/
int
get_len(curr)
    struct buffer_lines *curr;
{
    char *p;
    int i, j = -1;

    p = (char *)malloc(strlen(curr->data));
    strcpy(p, curr->data);
    trim_line(p);

    for (i = 0; i <= strlen(p); i++)
	if (p[i] != 1)
	    j++;
	else
	    i++;

    free(p);
    return (j);
}

/**************************************************************************
*
* YAWC_editor() basics by KHaglund (10-03-1996) fully rewritten by Flint.
*   result: text buffer, must be allocated with lines*length bytes!
*   lines:  max. number of lines to be used,
*   length: max. length of each line
*   Flags:  E_EDIT	- result already contains text to be edited
*           E_2ENTEREND - user may end the input with double-<Return>
*			  (otherwise only <Ctrl-D> works, and does always
*           E_COLORS	- user may use colours
*   prompt: the beginning of each line (like '>' for X's), with colour
*
***************************************************************************/

int
YAWC_editor(result, lines, length, flags, prompt)
    char *result;
    int lines;
    int length;
    int flags;
    char *prompt;
{
    struct buffer_lines *anchor, *curr, *old;
    char *p = 0, *q = 0, *t = NULL, *wrap = 0;
    int p_x = 0, p_y = 0, a, lines_used = 1, col_dirty = 1, top_visual = 0, bottom_visual,
        insert_mode = 1, i;
    unsigned char c = 255, oldc = 0;
    char *pc = NULL, *pd = NULL;/* prompt colour and prompt data */
    char *tmp2;			/* for colourizing */

    bottom_visual = rows - 2;

    if (strlen(prompt) > 0) {	/* separate prompt colour and data */
	p = pc = (char *)malloc(strlen(prompt));
	pd = (char *)malloc(strlen(prompt));

	for (i = 0; i < strlen(prompt); i++)
	    if (prompt[i] == 1) {
		*p++ = 1;
		*p++ = prompt[++i];
	    }
	*p = 0;

	p = pd;
	for (i = 0; i < strlen(prompt); i++)
	    if (prompt[i] == 1)
		i++;
	    else
		*p++ = prompt[i];
	*p = 0;
    } else {
	pc = (char *)malloc(1);
	pd = (char *)malloc(1);

	*pc = 0;
	*pd = 0;
    }

    anchor = curr = alloc_line(length);	/* initial line */
    curr->prev = NULL;

    /* Have we been sent something to edit? If so, we will  print out the
     * lines from the start, of course. The cursor will be at the first
     * character of the last line. */

    if (*result && flags & E_EDIT) {
	lines_used = 1;
	p_x = 0;

	for (a = 0; a < strlen(result); a++) {
	    if (result[a] == '\n' || result[a] == 0) {	/* end of line? */
		if ((result[a - 1] == '\n') && (result[a] == 0))
		    break;	/* work already done */
		if (curr->prev == NULL)	/* first line? */
		    colorize(prompt);
		else
		    printf(pd);
		print_line(curr->data, 0);
		if ((result[a] != 0) && (result[a + 1] != 0)) {
		    curr = line_forw(curr, length);
		    putchar('\n');
		    lines_used++;
		    p_y++;
		    p_x = 0;
		} else {	/* last line */
		    putchar('\r');
		    for (i = 0; i < strlen(pd); i++)
			printf("[C");	/* move cursor behind prompt */
		    p_x = set_realpos(curr, 0);
		}
	    } else {		/* no end of line */
		if (strlen(curr->data) > (curr->len - 3))
		    expand_line(curr);
		curr->data[p_x++] = result[a];
	    }
	}

    }
    /* End E_EDIT */
    else
	colorize(prompt);

    for (;;) {

	oldc = c;
	c = inkey();

	if (c == '\n')
	    c = CR;

	if (c == 27) {		/* ESC                  */
	    c = inkey();
	    if (c == '[') {	/* [                    */
		c = inkey();
		switch (c) {
				case 'A':	/* up                   */
				    if (p_y > 0) {
					a = get_realpos(curr, p_x);
					p_y--;
					curr = line_back(curr);
					p_x = set_realpos(curr, a);
					printf("M");	/* go up and scroll if
							 * necessary */

					if (p_y < top_visual)	/* scrolled above top
								 * line? */
					    reprint_whole_line(prompt, curr, anchor,
							       &top_visual, &bottom_visual, p_y, flags, 1, pc);

					reprint_colours(anchor, curr, p_x, flags, pc);
				    } else
					putchar('\7');
				    break;

				case 'B':	/* down                 */
				    if (p_y < lines_used - 1) {
					a = get_realpos(curr, p_x);
					p_y++;
					curr = line_forw(curr, length);
					p_x = set_realpos(curr, a);
					printf("D\x0a");	/* go down, scroll if
								 * necessary */

					if (p_y >= col_dirty) {	/* re-print needed? */
					    printf("7\r");
					    colorize(prompt);
					    reprint_colours(anchor, curr, get_realpos(curr, 0),
							    flags, pc);
					    colorize(curr->data);
					    printf("8");
					    col_dirty = p_y + 1;
					}
					if (p_y > bottom_visual)	/* scrolled below the
									 * bottom? */
					    reprint_whole_line(prompt, curr, anchor,
							       &top_visual, &bottom_visual, p_y, flags, 0, pc);

					reprint_colours(anchor, curr, p_x, flags, pc);

					if (lines_used == p_y && lines_used < lines)
					    lines_used++;
				    } else
					putchar('\7');

				    break;

				case 'C':	/* right                */
				    if (get_realpos(curr, p_x) < length - 1) {
					p_x++;
					printf("[C");
					while (curr->data[p_x] == 1) {
					    tmp2 = (char *)malloc(3);
					    sprintf(tmp2, "\1%c", curr->data[++p_x]);
					    colorize(tmp2);
					    free(tmp2);
					    p_x++;
					    printf("%c\b", curr->data[p_x]);
					}
				    } else {
					if (p_y < lines_used - 1) {
					    curr = line_forw(curr, length);
					    p_y++;
					    p_x = set_realpos(curr, 0);
					    putchar('\n');
					    colorize(prompt);
					    if (p_y >= col_dirty) {
						reprint_colours(anchor, curr, get_realpos(curr, 0),
								flags, pc);
						print_line(curr->data, 2);
						col_dirty = p_y + 1;
					    }
					    if (p_y > bottom_visual)
						reprint_whole_line(prompt, curr, anchor, &top_visual,
								   &bottom_visual, p_y, flags, 0, pc);
					    reprint_colours(anchor, curr, p_x, flags, pc);
					} else
					    putchar('\7');
				    }
				    break;

				case 'D':	/* left                 */
				    if (get_realpos(curr, p_x) > 0) {
					p_x--;
					printf("[D");
					while (curr->data[p_x - 1] == 1) {
					    p_x -= 2;
					    reprint_colours(anchor, curr, p_x, flags, pc);
					}
				    } else {
					if (p_y > 0) {
					    curr = line_back(curr);
					    p_y--;
					    p_x = set_realpos(curr, get_len(curr));
					    tmp2 = (char *)malloc(strlen(prompt) + 20);
					    sprintf(tmp2, "\rM%s[%dC", prompt, get_len(curr));
					    colorize(tmp2);
					    free(tmp2);
					    reprint_colours(anchor, curr, p_x, flags, pc);
					    if (p_y >= col_dirty) {
						reprint_colours(anchor, curr, get_realpos(curr, 0),
								flags, pc);
						print_line(curr->data, 0);
					    }
					    if (p_y < top_visual)
						reprint_whole_line(prompt, curr, anchor, &top_visual,
								   &bottom_visual, p_y, flags, 0, pc);
					} else
					    putchar('\7');
				    }
				    break;

				case '2':
				    c = inkey();
				    if (c == '~')
					insert_mode = 1 - insert_mode;
				    else
					putchar('\7');

				    break;

				default:
				    putchar('\7');
				    break;
		}
	    } else
		putchar('\7');

	    continue;
	}
	if (c == CTRL_D)	/* temporary "finished"-button. */
	    break;

	if (c == CR) {		/* ends this line and moves to the next */

	    /* two enters in a row and allowed -> finished. */

	    if (flags & E_2ENTEREND && (c == oldc || oldc == 255))
		break;

	    if (p_y < lines - 1) {	/* is there a line to go to?    */
		curr = line_forw(curr, length);
		p_y++;
		p_x = set_realpos(curr, 0);

		putchar('\n');
		colorize(prompt);
		if (p_y >= col_dirty) {
		    reprint_colours(anchor, curr, get_realpos(curr, 0), flags, pc);
		    printf("7");
		    colorize(curr->data);
		    printf("8");
		    col_dirty = p_y + 1;
		}
		if (p_y > bottom_visual)
		    reprint_whole_line(prompt, curr, anchor, &top_visual,
				       &bottom_visual, p_y, flags, 0, pc);

		reprint_colours(anchor, curr, p_x, flags, pc);

		if (p_y == lines_used && lines_used < lines)
		    lines_used++;
	    } else
		break;

	    continue;

	}
	if (c == '\b' || c == CTRL_X || c == CTRL_W) {	/* deleting stuff */
	    if (p_x > 0) {
		do {
		    p = (char *)malloc(strlen(curr->data));
		    strcpy(p, (char *)&curr->data[p_x--]);

		    if (curr->data[p_x - 1] != 1) {
			putchar('\b');
			print_line(p, 1);
		    } else {
			reprint_colours(anchor, curr, --p_x, flags, pc);
			print_line(p, 2);
		    }
		    memcpy(&curr->data[p_x], p, strlen(p) + 1);
		    strcat(curr->data, " ");
		    free(p);
		} while (p_x > 0 && (c == CTRL_X ||
			      (c == CTRL_W && curr->data[p_x - 1] != ' ')));
	    } else if (p_y > 0) {	/* p_x == 0 */
		putchar('\7');	/* will work (anti-wrap) eventually */
	    } else		/* p_x == p_y == 0 */
		putchar('\7');

	    continue;
	}
	if (c == 1 && flags & E_COLORS) {
	    colorize("\1a\1u\1B\1c\1eA[D");	/* colour prompt */
	    fflush(stdout);
	    c = inkey();
	    reprint_colours(anchor, curr, p_x, flags, pc);
	    p = (char *)malloc(5);
	    sprintf(p, "%c\b", curr->data[p_x]);
	    colorize(p);
	    fflush(stdout);

	    if (c < 32 || c > 127) {
		putchar('\7');
		continue;
	    }
	    p = (char *)malloc(strlen(curr->data));
	    strcpy(p, (char *)&curr->data[p_x]);
	    tmp2 = (char *)malloc(3);
	    sprintf(tmp2, "\1%c", c);
	    colorize(tmp2);
	    free(tmp2);
	    print_line(p, 2);
	    if (strlen(curr->data) > (curr->len - 3))
		expand_line(curr);
	    curr->data[p_x++] = 1;
	    curr->data[p_x++] = c;
	    memcpy(&curr->data[p_x], p, strlen(p) + 1);
	    col_dirty = p_y + 1;
	    free(p);
	    continue;
	}
	if (c < 32 || c > 127) {
	    putchar('\7');
	    continue;
	}
	curr->data[p_x++] = c;
	putchar(c);

	while (curr->data[p_x] == 1) {
	    tmp2 = (char *)malloc(3);
	    sprintf(tmp2, "\1%c", curr->data[++p_x]);
	    colorize(tmp2);
	    free(tmp2);
	    p_x++;
	}

	if (get_len(curr) == length) {	/* will be full at next char    */
	    if (p_y < lines - 1) {	/* can go to the next line      */
		old = curr;
		wrap = (char *)malloc(strlen(curr->data));
		if (wrap == NULL)
		    printf("\nmalloc error - out of system memory?\n");
		wrap[0] = 0;
		p_x = get_len(curr);
		if ((c != ' ') && (p_y + 1 == lines_used)) {
		    for (q = &curr->data[p_x]; *q != ' ' && q > curr->data; q--);
		    if (q > curr->data) {
			strcpy(wrap, ++q);
			for (a = 0; a < strlen(wrap); a++) {
			    if (curr->data[(--p_x) - 1] != 1) {
				printf("\b \b");
				curr->data[p_x] = ' ';
			    } else {
				curr->data[p_x--] = ' ';
				curr->data[p_x] = ' ';
				a++;
			    }
			}	/* for wrap */
		    }		/* if q wrapping */
		}		/* wrapping action */
		curr = line_forw(curr, length);
		p_y++;
		putchar('\n');
		colorize(prompt);

		if (p_y >= col_dirty) {
		    reprint_colours(anchor, curr, get_realpos(curr, 0), flags, pc);
		    printf("7");
		    colorize(curr->data);
		    printf("8");
		    col_dirty = p_y + 1;
		}
		if (p_y > bottom_visual)
		    reprint_whole_line(prompt, curr, anchor, &top_visual,
				       &bottom_visual, p_y, flags, 0, pc);

		reprint_colours(anchor, curr, p_x, flags, pc);

		p_x = strlen(wrap);
		if (wrap[0] != 0) {
		    while (strlen(wrap) > (curr->len - 3))
			expand_line(curr);
		    memcpy(curr->data, wrap, strlen(wrap));
		    colorize(wrap);
		}
		free(wrap);
		if (p_y == lines_used && lines_used < lines) {
		    lines_used++;
		}
	    } else {		/* the end of the last line     */
		p_x--;
		printf("\b\007");
	    }
	}			/* wrapping main loop */
    }				/* main loop */

    /* Here, go down (lines_used - p_y) steps. Reason: If we were on the
     * topmost line and had several used lines below, we need to get below
     * those lines now that we're finished. We'll also stop the blinking
     * line-cursor. */

    for (a = p_y; a < lines_used; a++)
	putchar('\n');

    /* now remove the unnecessary <space>'s from the end of all of the lines,
     * starting from the last tmp_array. We do that so that we can
     * anti-include, whatever that's called in proper english ;), all of the
     * empty lines at the end.  -KHaglund
     * 
     * um - 'delete' maybe? ;)   -Flint */

    curr = anchor;
    while (curr->next != NULL)
	curr = curr->next;

    for (a = lines_used - 1; a >= 0; a--) {
	t = strrkhr(curr->data, ' ');
	*t = '\0';
	if (!*curr->data && a == lines_used - 1)
	    lines_used--;
	curr = curr->prev;
    }

    /* And last, but not least, lets put everything in (char *)result which
     * this function was called with. */

    curr = anchor;

    for (p = result, a = 0; a < lines_used; a++) {
	if (flags & E_INCLUDE_PROMPT)	/* include prompt into result? */
	    for (i = 0; i < strlen(pd); i++)
		*p++ = pd[i];

	t = (char *)memccpy(p, curr->data, '\0', curr->len + 1);
	curr = curr->next;
	if (t == NULL)
	    colorize("\1f\1r\nEdit problem. uh-oh...\n");
	else {
	    p = t;
	    *(p - 1) = '\n';
	}
    }

    *p = '\0';

    curr = anchor;
    while (curr != NULL) {
	old = curr;
	curr = curr->next;
	free(old);
    }

    return (0);
}
