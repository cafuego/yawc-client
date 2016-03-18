/*
 * ,-------------------------------------------------------------------------. |
 * | |  THIS IS DEAD CODE. | | | | I left it in because "Diamond White" Dave
 * Cridland added it and maybe | | someday someone decides to re-include it
 * again. | | | | Basically cprintf() is just great. But I have found that
 * some stupid | | compilers just can't handle those variable headers, so I
 * had to take it | | out again to make the code compilable on as many
 * systems as possible.   | | Shame on me. | | Oh yeah, I moved ColourChar()
 * into utility btw. ;) | |
 * - Flint. | | |
 * `-------------------------------------------------------------------------'
 * */

/*-----------------------------
|    +-----------------+      |
|    | Y A W C - B B S |      |
|    +-----------------+      |
|       <IOSYSTEM.C>          |
|      Copyright (c)          |
| (See below, too...)         |
| GPL'd by inheritance.       |
| Kenneth Haglund 1994 - 1995 |
|    Dave Cridland   1995     |
-----------------------------*/

/*------------------------------------------------------------------------------

  ColourChar            - places a colour string into the output buffer
  cprintf               - handles placing data into the buffer
  number                - handles output of numbers

------------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "defs.h"
#include "ext.h"
/* Code for cprintf(), done properly, just like KH would really like. It been
 * made streamed, rather than forcing all the output to stdout. I don't
 * pretend to understand anything here, I've just adapted it, as needed,
 * from:
 * 
 * (CLient note: I changed it back to stdio, since there's little point, when
 * you've got the thing locally.) */

/* linux/kernel/vsprintf.c
 * 
 * Copyright (C) 1991, 1992  Linus Torvalds */

/* I imagine that Linus Torvalds would appreciate that being left here...
 * Have fun...  -  -  Diamond White Dave. */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/* Wirzenius wrote this portably, Torvalds fucked it up :-) */

/* I've left these here.... - Animal */

/* Cheers, DWD */

/* static unsigned long simple_strtoul(cp, endp, base) const char *cp; char
 * **endp; unsigned int base; { unsigned long result = 0,value;
 * 
 * if (!base) { base = 10; if (*cp == '0') { base = 8; cp++; if ((*cp == 'x') &&
 * isxdigit(cp[1])) { cp++; base = 16; } } } while (isxdigit(*cp) && (value =
 * isdigit(*cp) ? *cp-'0' : (islower(*cp) ? toupper(*cp) : *cp)-'A'+10) <
 * base) { result = result*base + value; cp++; } if (endp) endp = (char *)cp;
 * return result; } */

/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')

static int
skip_atoi(s)
    char **s;
{
    int i = 0;

    while (is_digit(**s))
	i = i * 10 + *((*s)++) - '0';
    return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */
/* #define do_div(n,base) ({ \ int __res; \ __res = ((unsigned long) n) %
 * (unsigned) base; \ n = ((unsigned long) n) / (unsigned) base; \ __res; }) */
static void
number(num, base, size, precision, type)
    long num;
    int base;
    int size;
    int precision;
    int type;
{
    char c, sign, tmp[36];
    char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    int i;
    int res;

    if (type & LARGE)
	digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type & LEFT)
	type &= ~ZEROPAD;
    if (base < 2 || base > 36)
	return;
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (type & SIGN) {
	if (num < 0) {
	    sign = '-';
	    num = -num;
	    size--;
	} else if (type & PLUS) {
	    sign = '+';
	    size--;
	} else if (type & SPACE) {
	    sign = ' ';
	    size--;
	}
    }
    if (type & SPECIAL) {
	if (base == 16)
	    size -= 2;
	else if (base == 8)
	    size--;
    }
    i = 0;
    if (num == 0)
	tmp[i++] = '0';
    else
	while (num != 0) {
	    res = ((unsigned long)num) % (unsigned)base;
	    num = ((unsigned long)num) / (unsigned)base;
	    tmp[i++] = digits[res];
	}
    if (i > precision)
	precision = i;
    size -= precision;
    if (!(type & (ZEROPAD + LEFT)))
	while (size-- > 0)
	    putchar(' ');
    if (sign)
	putchar(sign);
    if (type & SPECIAL)
	if (base == 8)
	    putchar('0');
	else if (base == 16) {
	    putchar('0');
	    putchar(digits[33]);
	}
    if (!(type & LEFT))
	while (size-- > 0)
	    putchar(c);
    while (i < precision--)
	putchar('0');
    while (i-- > 0)
	putchar(tmp[i]);
    while (size-- > 0)
	putchar(' ');
    return;
}

int
vcprintf(fmt, args)
    char *fmt;
    va_list args;
{
    int len, DynamicColourFlag = 0, ColourCount;
    unsigned long num;
    int i, base;
    char *s;

    int flags;			/* flags to number() */

    int field_width;		/* width of output field */
    int precision;		/* min. # of digits for integers; max number
				 * of chars for from string */
    int qualifier;		/* 'h', 'l', or 'L' for integer fields */
    static int was_escape = 0;	/* was the last char an ESC code? */

    for (; *fmt; ++fmt) {
	ColourCount = 0;
	if (*fmt != '%') {
	    if (*fmt == 1) {
		if (*(fmt + 1) == '%') {
		    DynamicColourFlag = 1;	/* Tricky stuff... */
		} else {
		    DynamicColourFlag = 0;
		    ColourChar(*(++fmt));
		}
	    } else {
		if (was_escape == 1) {
		    if (*fmt == '7')
			strcpy(stored_col, curr_col);
		    if (*fmt == '8')
			strcpy(curr_col, stored_col);
		}
		if (*fmt == 0x33)
		    was_escape = 1;
		else
		    was_escape = 0;
		putchar(*fmt);
	    }
	    continue;
	}
	/* process flags */
	flags = 0;
repeat:
	++fmt;			/* this also skips first '%' */
	switch (*fmt) {
			case '-':
			    flags |= LEFT;
			    goto repeat;
			case '+':
			    flags |= PLUS;
			    goto repeat;
			case ' ':
			    flags |= SPACE;
			    goto repeat;
			case '#':
			    flags |= SPECIAL;
			    goto repeat;
			case '0':
			    flags |= ZEROPAD;
			    goto repeat;
	}

	/* get field width */
	field_width = -1;
	if (is_digit(*fmt))
	    field_width = skip_atoi(&fmt);
	else if (*fmt == '*') {
	    ++fmt;
	    /* it's the next argument */
	    field_width = va_arg(args, int);
	    if (field_width < 0) {
		field_width = -field_width;
		flags |= LEFT;
	    }
	}
	/* get the precision */
	precision = -1;
	if (*fmt == '.') {
	    ++fmt;
	    if (is_digit(*fmt))
		precision = skip_atoi(&fmt);
	    else if (*fmt == '*') {
		++fmt;
		/* it's the next argument */
		precision = va_arg(args, int);
	    }
	    if (precision < 0)
		precision = 0;
	}
	/* get the conversion qualifier */
	qualifier = -1;
	if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
	    qualifier = *fmt;
	    ++fmt;
	}
	/* default base */
	base = 10;

	switch (*fmt) {
			case 'c':
			    if (!(flags & LEFT))
				while (--field_width > 0)
				    putchar(' ');
			    if (DynamicColourFlag) {
				DynamicColourFlag = 0;
				ColourChar((unsigned char)va_arg(args, int));
			    } else
				putchar((unsigned char)va_arg(args, char));
			    while (--field_width > 0)
				putchar(' ');
			    continue;

			case 's':
			case 'S':	/* For '*' instead of the real
					 * string, but still colourised, etc.  */
			    s = va_arg(args, char *);
			    if (!s)
				s = "\nAUGH, he cried in true KH style.\nYell, right now, and tell us what you pressed.\n";
			    len = strlen(s);
			    if (precision < 0)
				precision = len;
			    else if (len > precision)
				len = precision;

			    if (!(flags & LEFT))
				while (len < field_width--)
				    putchar(' ');
			    for (i = 0; i < len; ++i) {
				if (!*s) {
				    i = len;
				    break;
				}
				if (*s == 1) {
				    ColourCount++;	/* Count the colour
							 * codes, for
							 * formatting. */
				    i--;	/* Rewind... We (or the
						 * CLient) have not printed
						 * anything... */
				    s++;	/* Skip over ^A */
				    ColourChar(*s);	/* Convert the Colour
							 * character */
				    s++;	/* Move on. */
				} else {
				    if (*fmt == 's')
					putchar(*s++);	/* Output and move on. */
				    else {
					s++;
					putchar('*');	/* If we want *'s
							 * instead, then print
							 * them. */
				    }
				}
			    }
			    while (len < (field_width-- + (ColourCount * 2)))
				putchar(' ');
			    continue;

			case 'p':
			    if (field_width == -1) {
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			    }
			    number((unsigned long)va_arg(args, void *), 16,
				   field_width, precision, flags);
			    continue;


			    /* case 'n': if (qualifier == 'l') { long * ip =
			     * va_arg(args, long *); ip = (str - buf); } else
			     * { int * ip = va_arg(args, int *); ip = (str -
			     * buf); } continue; I don't support %n: I cannot
			     * actually see the point of it. Feel free to put
			     * one in. Just count up however many output()'s
			     * vcprintf and number use, and return that. It
			     * would require a static global variable for
			     * this module, though. */
			    /* integer number formats - set up the flags and
			     * "break" */
			case 'o':
			    base = 8;
			    break;

			case 'X':
			    flags |= LARGE;
			case 'x':
			    base = 16;
			    break;

			case 'd':
			case 'i':
			    flags |= SIGN;
			case 'u':
			    break;

			default:
			    if (*fmt != '%')
				putchar('%');
			    if (*fmt)
				putchar(*fmt);
			    else
				--fmt;
			    continue;
	}
	if (qualifier == 'l')
	    num = va_arg(args, unsigned long);
	else if (qualifier == 'h')
	    if (flags & SIGN)
		num = va_arg(args, short);
	    else
		num = va_arg(args, unsigned short);
	else if (flags & SIGN)
	    num = va_arg(args, int);
	else
	    num = va_arg(args, unsigned int);
	number(num, base, field_width, precision, flags);
    }
    return (0);
}

int
cprintf(fmt)
    char *fmt;
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vcprintf(fmt, args);
    va_end(args);
    return i;
}

/*************************************************
*
* ColourChar() - by DWD. (spelt correctly ;)
* Modelled on color() by KH. Internal use only.
*
*************************************************/

void
ColourChar(key)
    int key;
{
    char *s;
    int i;

    if (userflags & US_ANSICOLORS) {
	if (mystrchr("drgybpcw", key))
	    curr_col[13] = key;
	if (mystrchr("DRGYBPCW", key))
	    curr_col[15] = key;
	if (key == 'f')
	    curr_col[3] = key;
	if (key == 'e')
	    curr_col[5] = key;
	if (key == 'u')
	    curr_col[7] = key;
	if (key == 'i')
	    curr_col[9] = key;
	if (key == 'h')
	    curr_col[11] = key;
	if (key == 'a')
	    for (i = 1; i < 8; i++)
		curr_col[2 * i + 1] = ' ';

	switch (key) {
			case 'd':
			    s = "\033[30m";
			    break;	/* dark textcolor		 */
			case 'r':
			    s = "\033[31m";
			    break;	/* red textcolor		 */
			case 'g':
			    s = "\033[32m";
			    break;	/* green textcolor		 */
			case 'y':
			    s = "\033[33m";
			    break;	/* yellow textcolor		 */
			case 'b':
			    s = "\033[34m";
			    break;	/* blue textcolor		 */
			case 'p':
			    s = "\033[35m";
			    break;	/* purple textcolor		 */
			case 'c':
			    s = "\033[36m";
			    break;	/* cyan textcolor		 */
			case 'w':
			    s = "\033[37m";
			    break;	/* white textcolor		 */
			case 'D':
			    s = "\033[40m";
			    break;	/* dark backgroundcolor	 */
			case 'R':
			    s = "\033[41m";
			    break;	/* red backgroundcolor	 */
			case 'G':
			    s = "\033[42m";
			    break;	/* green backgroundcolor	 */
			case 'Y':
			    s = "\033[43m";
			    break;	/* yellow backgroundcolor	 */
			case 'B':
			    s = "\033[44m";
			    break;	/* blue backgroundcolor	 */
			case 'P':
			    s = "\033[45m";
			    break;	/* purple backgroundcolor	 */
			case 'C':
			    s = "\033[46m";
			    break;	/* cyan backgroundcolor	 */
			case 'W':
			    s = "\033[47m";
			    break;	/* white backgroundcolor	 */
			case 'a':
			    s = "\033[0m";
			    break;	/* 'reset'-attribute		 */
			case 'f':
			    s = ((userflags & US_NOBOLDCOLORS) ? "" : "\033[1m");
			    break;	/* 'bold'-attribute		 */
			case 'u':
			    s = "\033[4m";
			    break;	/* 'underlined'-attribute	 */
			case 'e':
			    s = ((userflags & US_NOFLASH) ? "" : "\033[5m");
			    break;	/* 'flashing'-attribute	(mmmmm
					 * Nice...) */
			case 'i':
			    s = "\033[7m";
			    break;	/* 'hidden'-attribute	 */
			default:
			    s = "";
	}
	cprintf(s);
    }
}
