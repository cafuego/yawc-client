#
# Unix Makefile for ISCA BBS client, now used for the YAWC CLient.
#


# USER CONFIGURATION SECTION


# Change these only if you want to use a different compiler or different flags
# to the compiler. $CC is the compiler to try first, if it fails use the other.
CC=gcc
CC2=cc
CFLAGS=-O2
CFLAGS2=-O
LDFLAGS=-s

# To use Michael O'Reilly's term package to connect to the BBS, first, read
# README.term and make sure you actually have the term package.
#
# Then, edit the following lines to point to your client.h and libclient.a,
# respectively, and "make term".
CLIENT_H_HOME=-I$(HOME)/include
CLIENT_A_HOME=-L$(HOME)/lib
TERM_CFLAGS=$(CLIENT_H_HOME) -DUSE_TERM
TERM_LDFLAGS=$(CLIENT_A_HOME) -lclient


# To use the SOCKS package for crossing firewalls to connect to the BBS, first,
# read README.socks and make sure you have the SOCKS package and it is set up
# correctly.
#
# Then, edit the following lines to point to your libsocks.a library and
# "make socks".
SOCKS_LIB_HOME=-L/usr/local/lib
SOCKS_LDFLAGS=$(SOCKS_LIB_HOME) -lsocks


# END OF USER CONFIGURATION SECTION



SRCS=global.c utility.c inkey.c telnet.c bbsrc.c edit.c getline.c\
	config.c unix.c des_enc.c yawc_edit.c main.c
OBJS=global.o utility.o inkey.o telnet.o bbsrc.o edit.o getline.o\
	config.o unix.o des_enc.o yawc_edit.o main.o
HDRS=defs.h proto.h ext.h telnet.h unix.h sysio.h

.c.o:
	@echo "Compiling '$*.c'."
	@$(CC) $(CFLAGS) -c $*.c || $(CC2) $(CFLAGS2) -c $*.c

yawc:	$(OBJS)
	@echo "Linking BBS client..."
	@$(CC) $(LDFLAGS) -o yawc $(OBJS) ||\
	$(CC) $(LDFLAGS) -o yawc $(OBJS) -lsocket -lnsl ||\
	$(CC) $(LDFLAGS) -o yawc $(OBJS) -lsocket ||\
	$(CC) $(LDFLAGS) -o yawc $(OBJS) -lsocket -lseq -linet -lnsl ||\
	$(CC2) $(LDFLAGS) -o yawc $(OBJS) ||\
	$(CC2) $(LDFLAGS) -o yawc $(OBJS) -lsocket -lnsl ||\
	$(CC2) $(LDFLAGS) -o yawc $(OBJS) -lsocket ||\
	$(CC2) $(LDFLAGS) -o yawc $(OBJS) -lsocket -lseq -linet -lnsl
	@echo "BBS client successfully compiled. YOU CAN SAFELY IGNORE ANY PREVIOUS ERRORS."

unix.o:
	@echo "Compiling 'unix.c'."
	@$(CC) $(CFLAGS) -c unix.c ||\
	$(CC) $(CFLAGS) -c unix.c -DUSE_TERMIO ||\
	$(CC2) $(CFLAGS2) -c unix.c ||\
	$(CC2) $(CFLAGS2) -c unix.c -DUSE_TERMIO

# For reasons unknown the gcc compiler on our ULTRIX system didn't generate a
# correct binary. On the other hand cc didn't understand all of the code. So I
# had to compile it the mixed way.
# Be sure to have $(CC) set to gcc !!
#
ultrix: yawc
	@echo "Building Ultrix version..."
	@cc $(CFLAGS) -c main.c
	@echo "Linking BBS client..."
	@cc $(LDFLAGS) -o yawc $(OBJS)
	@echo "Ultrix version of the BBS client successfully compiled."

install: yawc
	@chmod 755 yawc
	@if test -d $(HOME)/yawc; then\
	echo;\
	echo "Cannot install BBS client as $(HOME)/yawc.";\
	echo "$(HOME)/yawc already exists and is a directory.";\
	echo "Remove it first, then type 'make install' again.";\
	echo;\
	else \
	mv -i yawc $(HOME);\
	echo;\
	echo "BBS client installed as '$(HOME)/yawc'.";\
	fi;\
	if test ! -f $(HOME)/.yawcbbsrc; then\
	cp yawcbbsrc $(HOME)/.yawcbbsrc;\
	fi;\
	echo "Type $(HOME)/yawc (or maybe just 'yawc') to use it to connect to the BBS.";\
	echo

clean:
	rm -f yawc core $(OBJS)

$(OBJS): defs.h proto.h ext.h sysio.h
telnet.o: telnet.h
unix.o: unix.c unix.h
des_enc.o: des_enc.c des.h
