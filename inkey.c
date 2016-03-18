/* Return next letter the user typed.  This function can be indirectly
 * recursive -- inkey may call telrcv(), which calls other functions which
 * can call inkey()...
 * 
 * Basically, if there is a keypress waiting in the stdio buffer, it is returned
 * immediately, if not the socket's stdio buffer is checked (this is where
 * the recursion can take place) and if that is also empty, the output
 * buffers are flushed, then the program blocks in select() until either the
 * user types something or something is sent from the remote side. */
#include <unistd.h>

#include "defs.h"
#include "ext.h"
#include "telnet.h"

static int lastcr = 0;


/* The functionality of the former inkey() has been renamed to getkey(), so
 * that inkey() could strip a \n after a \r (common terminal program problem
 * with user misconfiguration) and translate certain keypresses into common
 * Unix equivalents (i.e. \r -> \n, DEL -> BS, ctrl-U -> ctrl-X) */
int
inkey()
{
    register int c;

    for (;;) {
        c = getkey();
        if (!lastcr || c != '\n')
            break;
        lastcr = 0;
    }
    lastcr = 0;
    if (!TELNET || inputmode) {
        if (c == '\r') {
            c = '\n';
            lastcr = 1;
        } else if (c == 127)
            c = '\b';
        else if (c == CTRL_U)
            c = CTRL_X;
    } else if (c == '\n')
        c = '\r';

    idletime = 0;
    /* WARNING! Don't change this. It's really cool ATM, and although simply
     * fucking about with the idle time would certainly solve many of the
     * problems of being idled out, even though the BBS does give up after a
     * while, and assume that you've been a naughty boy, (or girl), it's only
     * wasteful of the BBS's resources, and the guys who run the BBSes are
     * way cooler than to have an idletimeout for no reason. It's there so
     * you can be nice to others, basically. And if you're not nice to
     * others, then we (The BBS admins) will personally hate you lots. */
    if (sessionflags[curr_bbs] & S_LOGOUTW_RCV) {
        netput(IAC);
        netput(DONT);
        netput(TELOPT_LOGOUT);
        if (!inputmode)
            byte[curr_bbs] += 2;
        netflush();
        sessionflags[curr_bbs] &= ~S_LOGOUTW_RCV;
    }
    return (c);
}


int
getkey()
{
    static int macronext = 0;        /* macro key was hit, macro is next */
    static int wasundef = 0;        /* to remove the blurb about undefined macro */
    register int c = -1, d;
    int result, a, b, i;        /* result returned from waitnextevent() */
    char tmp[90];


    /* Throughout this function, if we are currently running with a child
     * process we don't want to do anything with standard input, we are only
     * concerned with passing along anything that might be coming over the
     * net during this time (connection going down, broadcast message from
     * wizards, etc.)  That's the reason for all the references to
     * '!childpid' The same is true when 'check' is set, it is used when
     * entering the edit menu (abort, save, etc.) to check back with the BBS
     * for any X messages that may have arrived -- it is added purely to make
     * things compatible between the BBS and the client. */

    for (;;) {

        /* If we're busy going to our save buffers, targetbyte is non-zero.
         * We continue to take characters from our save buffers until bytep
         * catches up to targetbyte -- then we should be synced with the bbs
         * once again. */
        if (targetbyte[curr_bbs] && !childpid && !check) {
            if (bytep[curr_bbs] == targetbyte[curr_bbs]) {
                targetbyte[curr_bbs] = 0;
            } else {
                lastcr = 0;

                return (save[curr_bbs][bytep[curr_bbs]++ % sizeof save[curr_bbs]]);

            }
        }
        /* Main processing loop for processing a macro or characters in the
         * stdin buffers. */
        while ((macrop || INPUT_LEFT(stdin)) && !childpid && !check) {
            /* macrop != 0 when we are getting out input from a macro key */
            if (macrop > 0) {
                if (macron == -1) {
                    if (DOC)
                        c = alt_away[macrop++];
                    else
                        c = away_message[macrop++];
                } else
                    c = macro[macron][macrop++];

                if (c) {
                    lastcr = 0;
                    return (c);
                } else {        /* end of macro */
                    macrop = 0;
                    if (userflags & LOGGING_IN || macron == 1) {
                        userflags &= ~LOGGING_IN;
                        mybzero(macro[1], 72);        /* wipe auto login from macro
                                                 * list */
                    }
                    continue;
                }
            }
            if (!macrop)
                c = getchar() /* & 0x7f */ ;
            else
                macrop = 0;


            /* Did we hit macrokey last?  Then the next key hit is the macro */
            if (macronext) {
                printf("\b\b\b\b\b\b\b       \b\b\b\b\b\b\b");
                macronext = macrop = 0;

                if (c == quitkey) {

                    /* Ahaa, this has to be done differently!!! */

                    printf("\r\n[Quitting]\r\n");
                    netput(IAC);
                    netput(DO);
                    netput(TELOPT_LOGOUT);
                    byte[curr_bbs] += 2;
                    netflush();
                    if (check_for_other_sessions())
                        myexit();
                    continue;
                } else if (c == suspkey) {
                    printf("\r\n[Suspended]\r\n");
                    fflush(stdout);
                    suspend();
                    continue;
                } else if (c == shellkey) {
                    printf("\r\n[New shell]\r\n");
                    run(shell, 0);
                    printf("\r\n[Continue]\r\n");
                    continue;
                } else if (c == capturekey) {
                    if (capture < 0 || posting) {
                        printf("[Cannot capture!]");
                        wasundef = 1;
                        continue;
                    }
                    capture ^= 1;
                    printf("\r\n[Capture to temp file turned O%s]\r\n", capture ? "N" : "FF");
                    if (capture) {
                        rewind(tempfile);
                        printf("Capture colours as well (y/n)? ");
                        if (yesno() == YES)
                            userflags |= COLORCAPTURE;
                        else
                            userflags &= ~COLORCAPTURE;

                        if (lastsave) {
                            (void)freopen(tempfilename, "w+", tempfile);
                            lastsave = 0;
                        } else if (getc(tempfile) >= 0) {
                            printf("There is text in your edit file.  Do you wish to erase it? (Y/N) -> ");
                            capture = -1;
                            if (yesno())
                                (void)freopen(tempfilename, "w+", tempfile);
                            else
                                fseek(tempfile, 0L, SEEK_END);
                            capture = 1;
                        }
                        lastsave = 0;
                    } else
                        fflush(tempfile);
                    continue;
                } else if ((c >= '0' && c <= '9') || (c == '!') || (c == connectkey))
                    /* connect/switch to another bbs! */
                {
                    if (c == '!')
                        d = MAXSYS;        /* ! (command line session) */
                    else if (c >= '0' && c <= '9')
                        d = c - 48;        /* 0-9 */
                    else if ((d = connect_prompt()) == -1)        /* -1 = abort */
                        continue;

                    if (curr_bbs == d)
                        printf("\n[Session (%d:%s) is the current one]\n", curr_bbs, bbshost[curr_bbs]);
                    else {
                        sessionflags[d] &= ~S_INCOMING_DATA;
                        if (!(net[d])) {        /* not connected from before         */
                            curr_bbs = d;
                            printf("\n[Trying to connect to a new session (%d:%s [%s])]\n",
                                   curr_bbs, (*bbsname[curr_bbs]) ? bbsname[curr_bbs] : bbshost[curr_bbs], bbshost[curr_bbs]);
                            fflush(stdout);
                            connectbbs(1);
                            telinit();
                        } else {/* connected from before         */
                            curr_bbs = d;
                            printf("\n[Switching to session (%d:%s)]\n", curr_bbs, (*bbsname[curr_bbs]) ? bbsname[curr_bbs] : bbshost[curr_bbs]);
                            connect_fp();
                        }
                    }
                    continue;
                } else if ((c == '+') || (c == '-')) {        /* one BBS up/down */
                    d = curr_bbs;
                    do {
                        d = d + ((c == '+') ? 1 : -1);
                        if (d > MAXSYS)
                            d = 0;
                        if (d < 0)
                            d = MAXSYS;
                    }
                    while ((d != curr_bbs) && (!(net[d])));
                    if (curr_bbs == d)
                        printf("\n[No other sessions active]\n");
                    else {
                        sessionflags[d] &= ~S_INCOMING_DATA;
                        curr_bbs = d;
                        printf("\n[Switching to session (%d:%s)]\n", curr_bbs,
                               (*bbsname[curr_bbs]) ? bbsname[curr_bbs] : bbshost[curr_bbs]);
                        connect_fp();
                    }
                    continue;
                } else if (c == '?') {        /* open sessions & incoming status */
                    colorize("\n\1a\1f\1wOpen sessions:\n\n");
                    for (a = 0; a <= MAXSYS; a++)
                        if ((net[a])) {
                            printf("Session %c: %s (%s)%s", (a < 10) ? a + 48 : (a == MAXSYS) ? '!' :
                                   a + 55, bbsname[a], bbshost[a],
                                   (sessionflags[a] & S_INCOMING_DATA) ?
                                   ((a == curr_bbs) ?
                                    "  ---this session---\n" : "  ---data waiting---\n")
                                   : "\n");
                        }
                    printf("[continue]\n");
                    continue;
                } else if (c == '*') {        /* KH's TEST-function */

                    for (a = 0; *wholist[a]; a++)
                        printf("Name: (%s)\n", wholist[a]);
                    continue;
                } else if ((c == 'i') || (c == 'I')) {        /* away message */
                    if (c == 'I') {
                        if (!*master_password) {
                            printf("\n[Cannot activate keyboard lock without a master password, try <c><c><l><m>.]\n");
                            continue;
                        }
                        printf("\nLocking the keyboard; switching into auto reply mode (y/n)? ");
                        fflush(stdout);
                        do {        /* can't use yesno() here cause it calls
                                 * inkey() itself */
                            b = getchar();
                        }
                        while (!mystrchr("yYnN", b));

                        if (mystrchr("nN", b)) {
                            if (rows < 10 || rows > 70)
                                rows = 25;
                            for (i = 0; i < rows; i++)
                                printf("\n");
                            printf("No.\n[Keyboard now locked, enter password to unlock]\n");
                            away |= 2;
                            continue;
                        } else
                            printf("Yes.\n");
                        /* fall through to 'i' mode */
                    }
                    if (inputmode) {
                        printf("\n[Cannot switch into away mode while in editing mode.]\n");
                        continue;
                    }
                    if (!*away_message) {
                        printf("\n[Activating automatic response mode.]\n");
                        if (!enter_awaymessage()) {
                            printf("\n[Cannot switch into away mode - no away message.]\n");
                            if (c == 'I') {
                                printf("[Keyboard now locked, enter password to unlock]\n");
                                away |= 2;
                            }
                            continue;
                        }
                    }
                    if (c == 'i')
                        printf("\n[Now in away mode.]\n");
                    else {
                        if (rows < 10 || rows > 70)
                            rows = 25;
                        for (i = 0; i < rows; i++)
                            printf("\n");
                        printf("[Keyboard now locked, enter password to unlock]\n");
                        away |= 2;
                    }
                    away |= 1;
                    continue;
                }
                /* here follows a hack to make local editing work even on DOC
                 * BBSs that don't support an own client (e.g. KaraNet).
                 * <macrokey> + these keys will activate the corresponding
                 * function IF we're using the telnet port. I mainly put this
                 * in for my personal amusement. ;)   -Flint */

                else if (TELNET && (c == 'x')) {
                    if (inputmode) {
                        printf("[already in local mode!]\n");
                        continue;
                    }
                    putchar('\r');
                    inputmode = 1;
                    get_some_lines(1);        /* manual local X mode (DOC patch) */
                    inputmode = 0;
                    continue;
                } else if (TELNET && (c == 'n')) {
                    if (inputmode) {
                        printf("[already in local mode!]\n");
                        continue;
                    }
                    inputmode = 1;
                    strcpy(tmp, get_name(2));        /* manual local name input
                                                 * (DOC patch) */
                    inputmode = 0;
                    for (b = 0; b < strlen(tmp); b++)
                        netput(tmp[b]);
                    netput('\n');
                    continue;
                } else if (TELNET && (c == 's')) {
                    if (inputmode) {
                        printf("[already in local mode!]\n");
                        continue;
                    }
                    inputmode = 1;
                    get_string(80, tmp, 0, 0);        /* manual local string input
                                                 * (DOC patch) */
                    inputmode = 0;
                    for (b = 0; b < strlen(tmp); b++)
                        netput(tmp[b]);
                    netput('\n');
                    continue;
                } else if (TELNET && (c == 'p')) {
                    if (inputmode) {
                        printf("[already in local mode!]\n");
                        continue;
                    }
                    inputmode = 1;
                    get_string(-80, tmp, 0, 0);        /* manual local string input
                                                 * (DOC patch) */
                    inputmode = 0;
                    for (b = 0; b < strlen(tmp); b++)
                        netput(tmp[b]);
                    netput('\n');
                    continue;
                } else if (TELNET && (c == 'e')) {
                    if (inputmode) {
                        printf("[already in local mode!]\n");
                        continue;
                    }
                    inputmode = 1;
                    makemessage(1);        /* manual local post mode (DOC patch) */
                    inputmode = 0;
                    continue;
                } else if (TELNET && (c == 'c')) {
                    if (inputmode) {
                        printf("[already in local mode!]\n");
                        continue;
                    }
                    inputmode = 1;
                    configbbsrc();
                    inputmode = 0;
                    continue;
                }
                /* End of DOC hack */

                else if (c > 127 || !*macro[macron = c]) {
                    printf("[Undefined macro]");
                    wasundef = 1;
                    continue;
                } else
                    return (macro[macron][macrop++]);
            }
            /* If we just printed the undefined macro blurb, let's erase it */
            else if (wasundef) {
                wasundef = 0;
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b                 \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            }
            /* Did the user just hit the macro key? */
            if (c == macrokey && !configflag) {
                macronext = 1;
                printf("[Macro]");
            } else
                return (c);        /* Return the next character to the caller */
        }

        /* Handle any incoming net traffic in the network input buffer */
        if (NET_INPUT_LEFT()) {
            while (NET_INPUT_LEFT()) {
                c = netget();
                if ((c == G_NAME) && (userflags & LOGIN_ASAP)) {
                    userflags &= ~LOGIN_ASAP;
                    userflags |= LOGGING_IN;
                    macrop = macron = 1;
                }
                if (telrcv(c) < 0)
                    return (-1);
            }
            continue;
        }
        /* Flush out any output buffers */
        if (netflush() < 0)
            fatalperror("\r\nNetwork error: send");
        if (fflush(stdout) < 0)
            fatalperror("\r\nLocal error: write");

        /* Wait for the next event from either the network or the user, note
         * that if we are running with a child process, we'll be ignoring the
         * user input entirely -- we're only concerned with network traffic
         * until we no longer have a child process to swallow up our data for
         * us. */
        result = waitnextevent();


        /* The user has input waiting for us to process */
        if (result & 1) {
            c = getchar();

            if (away) {
                if (away == 1) {
                    printf("[Ah, you're back. =) Switching off away mode.]\n");
                    away = 0;
                } else {        /* keyboard lock */
                    b = away;
                    away = 0;        /* to enable entering the password */
                    if (get_old_pw("Password", master_pw_enc, tmp, -1, c)) {
                        away = b;
                        continue;
                    } else {
                        printf("[Keyboard unlocked.]\n");
                        continue;
                    }
                }
            }
            if (c < 0)
                fatalperror("\r\nLocal error: read");
            /* c &= 0x7f; */
            macrop = -1;
            continue;
        }
        /* The network has input waiting for us to process */
        if (result & 2) {
            errno = 0;
            if ((c = netget()) < 0) {
                if (errno) {
                    fatalperror("\r\nNetwork error: recv");
                } else {
                    if (childpid) {
                        colorize("\r\n\n\007\1a[DISCONNECTED]\r\n\n\007");
                    } else {
                        colorize("\r\n\1a[Disconnected]\r\n");
                    }
                    if (check_for_other_sessions()) {
                        myexit();
                    }
                }
            }
            /* Handle net traffic */
            if ((c == G_NAME) && (userflags & LOGIN_ASAP)) {
                userflags &= ~LOGIN_ASAP;
                macrop = macron = 1;
            }
            if (telrcv(c) < 0) {
                return (-1);
            }
        }
    }
}

/* Idletime thingy. See note in function inkey(), file inkey.c [This one,
 * dummy.] */

void
sleeping(i)
    int i;
{
    signal(SIGALRM, sleeping);
    alarm(60);
    idletime++;
    if ((idletime > 5) && ((userflags & US_NOWARNING) == 0)        /* with telnet hack */
    &&(sessionflags[curr_bbs] & S_LOGOUTW_RCV & S_LOGOUTW_WILL) && !TELNET) {
        colorize("\n\007\1a\1f\1w[\1rCLient\1y: \1gAre you awake?? \1y(\1bDo a \1c^L \1bto redraw on most systems if you're editing\1y)\1w]\n");
        fflush(stdout);
    }
}
