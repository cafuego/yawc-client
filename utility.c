/* Various utility routines that didn't really belong elsewhere.  Yawn. */
#ifdef linux
#include <sys/socket.h>
#endif
#ifdef BSD
#include <sys/socket.h>
#endif
#include <unistd.h>

#include "defs.h"
#include "ext.h"


void
fatalperror(error)
    char *error;
{
    fflush(stdout);
    if (last_errno) {
        errno = last_errno;
        perror("YAWCclient");
    } else
        perror(error);
    fprintf(stderr, "\r");
    myexit();
}


void
fatalexit(message)
    char *message;
{
    fflush(stdout);
    fprintf(stderr, "%s\r\n", message);
    myexit();
}


/* KHAddition: we need to check for other open bbs-sessions in here, so that
 * <l>ogout <y>es doesn't quit all sessions from one session and so on. */

int
check_for_other_sessions()
{
    int i;


    shutdown(net[curr_bbs], 2);
    close(net[curr_bbs]);
    net[curr_bbs] = 0;

    for (i = 0; i <= MAXSYS; i++) {
        if (net[i]) {
            printf("\n[Switching back to #%d:%s]\n", i, (*bbsname[i]) ? bbsname[i] : bbshost[i]);
            curr_bbs = i;

            connect_fp();

            return (0);
        }
    }

    return (1);

}


void
connect_fp()
{
    if (!fps[curr_bbs][0]) {
        if (!(netifp = fdopen(net[curr_bbs], "r")))
            fatalperror("Local error: fdopen");
        fps[curr_bbs][0] = netifp;
    } else
        netifp = fps[curr_bbs][0];

    if (!fps[curr_bbs][1]) {
        if (!(netofp = fdopen(net[curr_bbs], "w")))
            fatalperror("Local error: fdopen");
        fps[curr_bbs][1] = netofp;
    } else
        netofp = fps[curr_bbs][1];
}


void
myexit()
{
    fflush(stdout);
    if (childpid) {
        /* Wait for child to terminate */
        sigoff();
        childpid = (-childpid);
        while (childpid)
            sigpause(0);
    }
    resetterm();
    if (lastsave)
        (void)freopen(tempfilename, "w+", tempfile);
    deinitialize();
    exit(0);
}


void
looper()
{
    register int c;
    int invalid = 0;

    for (;;) {
        if ((c = inkey()) < 0)
            return;
        /* Don't bother sending stuff to the bbs it won't use anyway : if ((c
         * >= 32 && c <= 127) ||
         * mystrchr("\1\3\4\5\6\b\n\r\016\022\27\30\32\033", c)) Hm - I
         * better remove this, so all CTRL codes work again. ;) -Flint */
        {
            invalid = 0;
            netput(c);
            if (byte[curr_bbs]) {
                /* print out what we save...
                 * 
                 * printf("[%c]", c); fflush(stdout); */

                save[curr_bbs][byte[curr_bbs]++ % sizeof save[curr_bbs]] = c;

            }
        }
        /* else if (invalid++) flush_input(invalid); */
    }
}


int
yesno()
{
    register int c;
    int invalid = 0;

    while (!mystrchr("nNyY", c = inkey()))
        if (invalid++)
            flush_input(invalid);
    if (c == 'y' || c == 'Y') {
        printf("Yes\r\n");
        return (1);
    } else {
        printf("No\r\n");
        return (0);
    }
}


/*************************************************
*
* yesnodefault()
*
* by KHaglund
*
*************************************************/

int
yesnodefault()
{
    register int c;
    int invalid = 0;


    while (!mystrchr("nNyY\n ", c = inkey()))
        if (invalid++)
            flush_input(invalid);

    if (c == 'y' || c == 'Y') {
        printf("Yes\r\n");
        return (1);
    } else if (c == 'n' || c == 'N') {
        printf("No\r\n");
        return (0);
    } else {
        printf("\r\n");
        return (9);
    }

}



void
tempfileerror()
{
    fflush(stdout);
    perror("\r\n[Warning, local error writing to tempfile");
    fprintf(stderr, "\r");
}



int
more(line, pct)
    int *line;
    int pct;
{
    register int c;
    int invalid = 0;

    if (pct >= 0)
        printf("--MORE--(%d%%)", pct);
    else
        printf("--MORE--");
    for (;;) {
        c = inkey();
        if (c == ' ' || c == 'y' || c == 'Y')
            *line = 1;
        else if (c == '\n')
            --* line;
        else if (mystrchr("nNqsS", c))
            *line = -1;
        else if (invalid++) {
            flush_input(invalid);
            continue;
        }
        printf("\r              \r");
        break;
    }
    return (*line < 0 ? -1 : 0);
}



/* Not all systems have strstr(), so I roll my own... */
char *
mystrstr(str, substr)
    char *str;
    char *substr;
{
    register char *s;

    for (s = str; *s; s++)
        if (*s == *substr && !strncmp(s, substr, strlen(substr)))
            break;
    if (!*s)
        return ((char *)NULL);
    else
        return (s);
}



/* Not all systems have strchr() either (they usually have index() instead,
 * but I don't want to count on that or check for it) */
char *
mystrchr(str, ch)
    char *str;
    int ch;
{
    while (*str && ch != *str)
        str++;
    if (*str)
        return (str);
    else
        return ((char *)NULL);
}

char *
mystrrchr(str, ch)
    char *str;
    int ch;
{
    char *st;

    st = str;
    while (*st)
        st++;
    while (st != str && ch != *st)
        st--;
    if (st != str)
        return (st);
    else
        return ((char *)NULL);
}

char *
mybzero(str, n)
    char *str;
    int n;
{
    int i;
    char *p;

    p = str;
    for (i = 0; i < n; i++)
        *(p++) = 0;

    return (str);
}

/*************************************************
*
* color()                        -KHaglund
* SymLinked to ColourChar().
*
*************************************************/

void
color(ch)
    int ch;
{
    ColourChar(ch);
}



/*************************************************
 *
 * colorize()
 *
 * DWD says - stuff this for a lark... YAWC was hell before cprintf.
 * We had to use this stuff all the time. Ugh!
 * So I wrote cprintf, or rather, adjusted a bit of code ripped from the
 * Linux kernel. The code in the Kernel has changed, since, with is rather
 * alarming... But heck. It (mostly) works.
 *
 * Flint says - I'm sorry, but cprintf confused a lot of compilers - back to
 * the roots then, for the sake of compatibility. *sighs*
 *
 *************************************************/

void
colorize(string)
    char *string;
{
    char *p;

    if (strlen(string) < 1)
        return;

    p = string;
    for (p = string; *p; p++) {
        if (*p == '\1') {        /* <Ctrl-A> */
            p++;
            if (*p > 31 && *p < 127)
                color(*p);
        } else
            putchar(*p);
    }
}



/*************************************************
*
* back()
*
*************************************************/

void
back(spaces)
    int spaces;
{
    int a;

    for (a = 0; a < spaces; a++) {
        putchar(8);
        putchar(32);
        putchar(8);
    }
}

/**************************************************************************
*
* new_passwd - gets a verified new password with at least minlen chars
*
* returns: 0 = no errors, new password written into newpw (max. 19 chars)
*         -1 = aborted (user hit <Return> on a blank line)
*          1 = user abort after mismatch
*
***************************************************************************/
int
new_password(new_pwd, prompt, minlen)
    char *new_pwd;
    char *prompt;
    int minlen;
{
    char *tmp, *tmp2;

    tmp = (char *)malloc(20);
    tmp2 = (char *)malloc(20);

    for (;;) {
        printf("%s  -> ", prompt);
        get_string(-19, tmp, -999, 0);
        if (!*tmp) {
            new_pwd[0] = 0;
            free(tmp);
            free(tmp2);
            return (-1);
        }
        if (strlen(tmp) < minlen) {
            printf("The password has to contain at least %d characters; try again.\n"
                   ,minlen);
            continue;
        }
        printf("Again for verification -> ");
        get_string(-19, tmp2, -999, 0);
        if (strcmp(tmp, tmp2) == 0) {
            strncpy(new_pwd, tmp, 19);
            free(tmp);
            free(tmp2);
            return (0);
        } else {
            printf("The passwords don't match. Try again (y/n)? ");
            if (yesno() == NO) {
                new_pwd[0] = 0;
                free(tmp);
                free(tmp2);
                return (1);
            }
        }
    }
}

/**************************************************************************
*
* my_crypt - returns an encrypted password
*
**************************************************************************/
char *
my_crypt(pw_clear)
    char *pw_clear;
{
    char *s1, *s2, *p;
    int i;
    s1 = (char *)calloc(1, 12);
    s2 = (char *)calloc(1, 12);
    i = strlen(pw_clear) / 2;
    strncpy(s1, pw_clear, i);
    s1[i] = 0;
    strncpy(s2, &pw_clear[i], 12);
    p = do_des(s1, s2, ENCRYPT);
    return (p);
}

/**************************************************************************
*
* get_old_pw - verifies password
*
* pw_enc   : encrypted password to check for
* pw_clear : space for new password
* how      : 0 = use get_string(),
*             1 = use getpass() - get_string() doesn't work before the CLient
*                 is connected - why?
*            -1 = only one try, beep on error (for keyboard lock)
*
* returns  : 0 = no errors, password written into pw_clear (max. 19 chars)
*                 or no pw_enc given, pw_clear will then be cleared.
*           -1 = aborted (user hit <Return> on a blank line)
*            1 = aborted because of too many mismatches (3)
*
***************************************************************************/
int
get_old_pw(prompt, pw_enc, pw_clear, how, first)
    char *prompt;
    char *pw_enc;
    char *pw_clear;
    int how;
    int first;
{
    int i = 3;
    char *tmp, *tmp2, *p;

    if (strlen(pw_enc) < 1) {
        pw_clear[0] = 0;
        return (0);
    }
    tmp = (char *)malloc(20);
    tmp2 = (char *)malloc(21);
    do {
        printf("%s  -> ", prompt);
        fflush(stdout);
        if (how == 1) {
            p = (char *)getpass("");
            strncpy(tmp, p, 20);
            tmp[19] = 0;
        } else
            get_string(-19, tmp, -999, 0);

        if ((strlen(tmp) < 1) || mystrchr("\3\4", tmp[0])) {
            printf("Aborting.\n");
            free(tmp);
            return (-1);
        }
        if (first)
            sprintf(tmp2, "%c%s", first, tmp);
        else
            sprintf(tmp2, "%s", tmp);

        if (strlen(tmp) < 2) {        /* too short */
            printf("Wrong password.\n");
            if ((i < 0) || (how == -1)) {
                if (how == -1) {
                    putchar(7);
                    fflush(stdout);
                    sleep(1);
                }
                free(tmp);
                return (1);
            } else
                continue;
        }
        if ((strcmp(my_crypt(tmp), pw_enc) == 0)
            || (strcmp(my_crypt(tmp2), pw_enc) == 0)) {
            strncpy(pw_clear, tmp, 19);
            free(tmp);
            return (0);
        }
        /* else */
        printf("Wrong password.\n");
    }
    while ((--i > 0) && (how != -1));

    if (how == -1) {
        putchar(7);
        fflush(stdout);
        sleep(1);
    }
    free(tmp);
    return (1);
}

/**************************************************************************
*
* enter_awaymessage() - gets new away message
*
* returns: 0 = no away message set, 1 = away message present
*
**************************************************************************/
int
enter_awaymessage()
{
    char tmp[80], *p;

    if (*away_message) {
        p = mystrchr(away_message, ':') + 2;
        sprintf(colst, "This is the old message:\1c\n%s\1wDo you want to change it (y/n)? ", p);
        colorize(colst);
        if (yesno() == NO)
            return (1);
    }
    colorize("Enter new away message. You have 70 characters.\n\1c");
    get_string(68, tmp, -999, 1);
    colorize("\1w");

    if (!*tmp) {
        *away_message = 0;
        return (0);
    }
    /* else */
    sprintf(away_message, " v\1f\n\1y=== This is an \1rautomatic\1y CLient message. \1rDO NOT RESPOND\1y ===\1c\nThe user is presently away from keyboard.\nRemark: %s\n\n", tmp);
    sprintf(alt_away, " x                    \nThis message was automatically generated; please do not reply.\nThe user is presently away from keyboard.\nRemark: %s\n\n", tmp);
    return (1);
}

/************************************************************************
*
* strucmp() - case-insensitive strcmp, ignores string length differences
*             as long as no string is void
*
*************************************************************************/

int
strucmp(st1, st2)
    char st1[], st2[];
{
    int a, b, c, d;

    a = 0;
    b = 0;
    while (st1[a] && st2[a] && !b) {
        c = st1[a];
        d = st2[a];
        if ((c >= 'a') && (c <= 'z'))
            c -= 32;
        if ((d >= 'a') && (d <= 'z'))
            d -= 32;

        if (c < d)
            return (-1);
        if (c > d)
            return (1);
        a++;
    }

    if (!a && ((st1[a] && !st2[a]) || (!st1[a] && st2[a])))
        return (1);

    return (b);
}

/*************************************************
*
* ColourChar() - by DWD. (spelt correctly ;)
* Modelled on color() by KH. Internal use only.
*
* Moved from iosystem.c to here.  -Flint.
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
                            break;        /* dark textcolor                 */
                        case 'r':
                            s = "\033[31m";
                            break;        /* red textcolor                 */
                        case 'g':
                            s = "\033[32m";
                            break;        /* green textcolor                 */
                        case 'y':
                            s = "\033[33m";
                            break;        /* yellow textcolor                 */
                        case 'b':
                            s = "\033[34m";
                            break;        /* blue textcolor                 */
                        case 'p':
                            s = "\033[35m";
                            break;        /* purple textcolor                 */
                        case 'c':
                            s = "\033[36m";
                            break;        /* cyan textcolor                 */
                        case 'w':
                            s = "\033[37m";
                            break;        /* white textcolor                 */
                        case 'D':
                            s = "\033[40m";
                            break;        /* dark backgroundcolor         */
                        case 'R':
                            s = "\033[41m";
                            break;        /* red backgroundcolor         */
                        case 'G':
                            s = "\033[42m";
                            break;        /* green backgroundcolor         */
                        case 'Y':
                            s = "\033[43m";
                            break;        /* yellow backgroundcolor         */
                        case 'B':
                            s = "\033[44m";
                            break;        /* blue backgroundcolor         */
                        case 'P':
                            s = "\033[45m";
                            break;        /* purple backgroundcolor         */
                        case 'C':
                            s = "\033[46m";
                            break;        /* cyan backgroundcolor         */
                        case 'W':
                            s = "\033[47m";
                            break;        /* white backgroundcolor         */
                        case 'a':
                            s = "\033[0m";
                            break;        /* 'reset'-attribute                 */
                        case 'f':
                            s = ((userflags & US_NOBOLDCOLORS) ? "" : "\033[1m");
                            break;        /* 'bold'-attribute                 */
                        case 'u':
                            s = "\033[4m";
                            break;        /* 'underlined'-attribute         */
                        case 'e':
                            s = ((userflags & US_NOFLASH) ? "" : "\033[5m");
                            break;        /* 'flashing'-attribute        (mmmmm
                                         * Nice...) */
                        case 'i':
                            s = "\033[7m";
                            break;        /* 'inverse'-attribute         */
                        case 'h':
                            s = "\033[8m";
                            break;        /* 'hidden'-attribute         */
                        default:
                            s = "";
        }
        printf("%s", s);
    }
}

/* parse ANSI sequences into curr_col string */
void
new_color(s)
    char *s;
{
    int i;

    /* printf("m<%s>\n", s); */
    while (*s) {
        switch (s[0]) {
                        case '0':
                            if ((s[1] == 'm') || (s[1] == ';'))
                                for (i = 1; i < 8; i++)
                                    curr_col[2 * i + 1] = ' ';
                            break;

                        case '1':
                            if (s[1] == 'm' || s[1] == ';')
                                curr_col[3] = 'f';
                            break;

                        case '3':
                            if (s[2] != 'm' || s[2] == ';')
                                break;
                            switch (s[1]) {
                                            case '0':
                                                curr_col[13] = 'd';
                                                break;
                                            case '1':
                                                curr_col[13] = 'r';
                                                break;
                                            case '2':
                                                curr_col[13] = 'g';
                                                break;
                                            case '3':
                                                curr_col[13] = 'y';
                                                break;
                                            case '4':
                                                curr_col[13] = 'b';
                                                break;
                                            case '5':
                                                curr_col[13] = 'p';
                                                break;
                                            case '6':
                                                curr_col[13] = 'c';
                                                break;
                                            case '7':
                                                curr_col[13] = 'w';
                                                break;
                                            default:
                                                break;
                            }

                        case '4':
                            if (s[1] == 'm' || s[1] == ';') {
                                curr_col[7] = 'u';
                                break;
                            }
                            if (s[2] != 'm' || s[2] == ';')
                                break;
                            switch (s[1]) {
                                            case '0':
                                                curr_col[13] = 'D';
                                                break;
                                            case '1':
                                                curr_col[13] = 'R';
                                                break;
                                            case '2':
                                                curr_col[13] = 'G';
                                                break;
                                            case '3':
                                                curr_col[13] = 'Y';
                                                break;
                                            case '4':
                                                curr_col[13] = 'B';
                                                break;
                                            case '5':
                                                curr_col[13] = 'P';
                                                break;
                                            case '6':
                                                curr_col[13] = 'C';
                                                break;
                                            case '7':
                                                curr_col[13] = 'W';
                                                break;
                                            default:
                                                break;
                            }

                        case '5':
                            if (s[1] == 'm' || s[1] == ';')
                                curr_col[5] = 'e';
                            break;

                        case '7':
                            if (s[1] == 'm' || s[1] == ';')
                                curr_col[9] = 'i';
                            break;

                        case '8':
                            if (s[1] == 'm' || s[1] == ';')
                                curr_col[11] = 'h';
                            break;

                        default:
                            break;
        }
        if (s[1] == 'm' || s[1] == ';')
            s = &s[2];
        else {
            if (s[2] == 'm' || s[2] == ';')
                s = &s[3];
            else
                return;                /* should never happen... */
        }
    }
}

int
friends2tab()
{
    int i;

    for (i = 0; (i < MAXFRIENDS) && *friend[i]; i++)
        strncpy((char *)wholist[i], friend[i], 20);

    return (i - 1);
}

int
connect_prompt()
{
    int i, j, k, done;
    char chost[40];
    char cport[7];
    int nport, sflags;

    printf("\r\n[Connect prompt]\r\n");
    do {
        done = 1;
        printf("\r\nSlot ([0..9],[a..z],[!], ? for help) -> ");
        i = getkey();
        if (i == '?') {
            printf("Help\nHit a number or letter key or '!' to specify the 'slot' the Client should use.\n");
            printf("So there are 37 slots available (10 numbers, 26 letters and a command line\n");
            printf("slots) that can be open at one time. Hit <return> to abort.\n");
            done = 0;
            continue;
        }
        if (mystrchr(
                     "0123456789abcdefghijklmnopqrstuvwxyz!ABCDEFGHIJKLMNOPQRSTUVWXYZ\n\r", i)
            == NULL) {
            printf("\7<invalid input>");
            done = 0;
        } else if ((i != '\n') && (i != '\r'))
            printf("'%c' = session slot #", i);
    }
    while (done == 0);
    if (mystrchr("0123456789", i) != NULL)
        i = i - '0';
    else if (mystrchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", i) != NULL)
        i = i - 'A' + 10;
    else if (mystrchr("abcdefghijklmnopqrstuvwxyz", i) != NULL)
        i = i - 'a' + 10;
    else if (i == '!')
        i = MAXSYS;
    else {
        printf("\n[No session opened.]\n");
        return (-1);
    }

    if (i != -1)
        printf("%d\n", i);

    if (!*bbshost[i])
        printf("No host defined yet for slot #%d. Please enter one or hit <return> to abort.\n", i);
    else
        printf("Default host for slot #%d is %s.\nHit <return> to accept or enter a new one below.\n", i, bbshost[i]);
    printf("Address of the BBS host -> ");
    *chost = 0;
    get_string(40, chost, 0, 0);
    if ((!*chost) && (!*bbshost[i])) {
        printf("\n[No session opened.]\n");
        return (-1);
    }
    do {
        done = 1;
        sflags = 0;
        nport = 0;
        printf("Port to connect (D, T, Y, number, <Return> for default, ? for help) -> ");
        get_string(6, cport, 0, 0);
        if (cport[0] == '?') {
            printf("Specify how the CLient should connect.\n");
            printf("Entering D will connect as DOC client (for ISCA & Gestalt, port 23),\n");
            printf("Entering T will connect as ordinary telnet client (port 23),\n");
            printf("Entering Y will connect as YAWC CLient (port 1976).\n");
            printf("If only a port number is entered the CLient will connect there in telnet mode.\n");
            printf("Just hitting <Return> defaults to YAWC CLient mode or DOC mode if ISCA's,\n");
            printf("or Gestalt's address is recognized.\n");
            printf("You can also combine both a letter and a number in case you need it.\n");
            printf("Examples (DON'T ENTER THE QUOTES!):\n");
            printf("'D'     will connect in DOC client mode,\n");
            printf("'3000'  will connect in telnet mode to port 3000,\n");
            printf("'Y23' will connect to port 23, using YAWC CLient mode.");
            printf("\n");
            done = 0;
            continue;
        }
        if (!*cport) {
            printf("Using default port.\n");
            cmd_line_hostname = NULL;
            cmd_line_port = 0;
            return (i);
        }
        if (mystrchr("DTYdty1234567890", cport[0]) == NULL) {
            printf("\nInvalid mode or port number\n");
            done = 0;
        }
        if (done == 0)
            continue;

        switch (cport[0]) {
                        case 'd':
                        case 'D':
                            nport = 23;
                            sflags = S_DOCCLIENT | S_TELNET;
                            break;

                        case 't':
                        case 'T':
                            nport = 23;
                            sflags = S_TELNET;
                            break;

                        case 'y':
                        case 'Y':
                            nport = 1976;
                            break;

                        default:
                            nport = 0;
        }

        k = -1;
        if (isdigit(cport[0]))
            k = 0;
        if (isdigit(cport[1]) && k < 0)
            k = 1;

        for (j = k; j < strlen(cport); j++)
            if (!isdigit(cport[j])) {
                printf("invalid port number!\n");
                done = 0;
                break;
            }
        if (done == 0)
            continue;

        if (k >= 0)
            nport = atoi(&cport[k]);
        if (nport <= 0 || nport > 65535) {
            printf("port number out of range!\n");
            done = 0;
        }
    }
    while (done == 0);

    if (strlen(chost) > 0) {
        strncpy(bbshost[i], chost, 40);
    }
    printf("Connecting to <%s>.\n", bbshost[i]);
    cmd_line_hostname = NULL;
    cmd_line_port = nport;
    return (i);
}
