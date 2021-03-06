/* <des header files> Copyright (C) 1995 Eric Young (eay@mincom.oz.au) All
 * rights reserved.
 * 
 * This file is part of an SSL implementation written by Eric Young
 * (eay@mincom.oz.au). The implementation was written so as to conform with
 * Netscapes SSL specification.  This library and applications are FREE FOR
 * COMMERCIAL AND NON-COMMERCIAL USE as long as the following conditions are
 * aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in the code
 * are not to be removed.  If this code is used in a product, Eric Young
 * should be given attribution as the author of the parts used. This can be
 * in the form of a textual message at program startup or in documentation
 * (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: 1. Redistributions of source code must retain the copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions
 * in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. All advertising materials
 * mentioning features or use of this software must display the following
 * acknowledgement: This product includes software developed by Eric Young
 * (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply
 * be copied and put under another distribution licence [including the GNU
 * Public Licence.] */

/* Flint: The des* files could really do with some clean-ups... *sighs* */

#ifndef HEADER_DES_H
#define HEADER_DES_H

#include <stdio.h>

typedef unsigned char des_cblock[8];
typedef struct des_ks_struct {
    union {
	des_cblock _;
	/* make sure things are correct size on machines with 8 byte longs */
	unsigned long pad[2];
    }     ks;
#undef _
#define _	ks._
}             des_key_schedule[16];

#define DES_KEY_SZ 	(sizeof(des_cblock))
#define DES_SCHEDULE_SZ (sizeof(des_key_schedule))

#define DES_ENCRYPT	1
#define DES_DECRYPT	0

#define DES_CBC_MODE	0
#define DES_PCBC_MODE	1

#define C_Block des_cblock
#define Key_schedule des_key_schedule
#define KEY_SZ DES_KEY_SZ
#define string_to_key des_string_to_key
#define read_pw_string des_read_pw_string
#define set_key des_set_key
#define key_sched des_key_sched
#define ecb_encrypt des_ecb_encrypt
#define cbc_cksum des_cbc_cksum

/* For compatibility with the MIT lib - eay 20/05/92 */
typedef struct des_key_schedule bit_64;
#define des_fixup_key_parity des_set_odd_parity
#define des_check_key_parity check_parity

extern int des_check_key;	/* defaults to false */
extern int des_rw_mode;		/* defaults to DES_PCBC_MODE */

/* The next line is used to disable full ANSI prototypes, if your compiler
 * has problems with the prototypes, make sure this line always evaluates to
 * true :-) */
#if defined(MSDOS) || defined(__STDC__)
#undef PROTO
#define PROTO
#endif
#ifdef PROTO
unsigned long
des_cbc_cksum(des_cblock * input, des_cblock * output,
	      long length, des_key_schedule schedule, des_cblock * ivec);
void
des_ecb_encrypt(des_cblock * input, des_cblock * output,
		des_key_schedule ks, int enc);
void des_encrypt(unsigned long *data, des_key_schedule ks, int enc);
void des_encrypt2(unsigned long *data, des_key_schedule ks, int enc);

void des_random_seed(des_cblock key);
void des_random_key(des_cblock ret);
void des_set_odd_parity(des_cblock * key);
int des_is_weak_key(des_cblock * key);
int des_set_key(des_cblock * key, des_key_schedule schedule);
int des_key_sched(des_cblock * key, des_key_schedule schedule);
void des_string_to_key(char *str, des_cblock * key);

/* The following functions are not in the normal unix build or the SSLeay
 * build.  When using the SSLeay build, use RAND_seed() and RAND_bytes()
 * instead. */
int des_new_random_key(des_cblock * key);
/* void des_init_random_number_generator(des_cblock *key); */
void des_set_random_generator_seed(des_cblock * key);
void des_set_sequence_number(des_cblock new_sequence_number);
void des_generate_random_block(des_cblock * block);

#else

unsigned long des_cbc_cksum();
void des_ecb_encrypt();
void des_encrypt();
void des_encrypt2();
void des_random_seed();
void des_random_key();
void des_set_odd_parity();
int des_is_weak_key();
int des_set_key();
int des_key_sched();
void des_string_to_key();

/* The following functions are not in the normal unix build or the SSLeay
 * build.  When using the SSLeay build, use RAND_seed() and RAND_bytes()
 * instead. */
int des_new_random_key();
void des_init_random_number_generator();
void des_set_random_generator_seed();
void des_set_sequence_number();
void des_generate_random_block();

#endif
#endif

/*----------------------------  des_locl.h ---------------------------------*/

#ifndef HEADER_DES_LOCL_H
#define HEADER_DES_LOCL_H
#include <stdlib.h>
#ifndef MSDOS
#include <unistd.h>
#endif

/* the following is tweaked from a config script, that is why it is a
 * protected undef/define */
#ifndef DES_USE_PTR
#undef DES_USE_PTR
#endif

#ifdef MSDOS			/* Visual C++ 2.1 (Windows NT/95) */
#include <stdlib.h>
#include <time.h>
#include <io.h>
#define RAND
#undef PROTO
#define PROTO
#endif

#if defined(__STDC__) || defined(VMS) || defined(M_XENIX) || defined(MSDOS)
#include <string.h>
#endif

#ifndef RAND
#define RAND
#endif

#ifdef MSDOS
#define getpid() 2
extern int errno;
#define RAND
#undef PROTO
#define PROTO
#endif

#if defined(NOCONST)
#define const
#endif

#ifdef __STDC__
#undef PROTO
#define PROTO
#endif

#ifdef RAND
#define srandom(s) srand(s)
#define random rand
#endif

#define ITERATIONS 16
#define HALF_ITERATIONS 8

/* used in des_read and des_write */
#define MAXWRITE	(1024*16)
#define BSIZE		(MAXWRITE+4)

#define c2l(c,l)	(l =((unsigned long)(*((c)++)))    , \
			 l|=((unsigned long)(*((c)++)))<< 8L, \
			 l|=((unsigned long)(*((c)++)))<<16L, \
			 l|=((unsigned long)(*((c)++)))<<24L)

/* NOTE - c is not incremented as per c2l */
#define c2ln(c,l1,l2,n)	{ \
			c+=n; \
			l1=l2=0; \
			switch (n) { \
			case 8: l2 =((unsigned long)(*(--(c))))<<24L; \
			case 7: l2|=((unsigned long)(*(--(c))))<<16L; \
			case 6: l2|=((unsigned long)(*(--(c))))<< 8L; \
			case 5: l2|=((unsigned long)(*(--(c))));     \
			case 4: l1 =((unsigned long)(*(--(c))))<<24L; \
			case 3: l1|=((unsigned long)(*(--(c))))<<16L; \
			case 2: l1|=((unsigned long)(*(--(c))))<< 8L; \
			case 1: l1|=((unsigned long)(*(--(c))));     \
				} \
			}

#define l2c(l,c)	(*((c)++)=(unsigned char)(((l)     )&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8L)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16L)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>24L)&0xff))

/* replacements for htonl and ntohl since I have no idea what to do when
 * faced with machines with 8 byte longs. */
#define HDRSIZE 4

#define n2l(c,l)	(l =((unsigned long)(*((c)++)))<<24L, \
			 l|=((unsigned long)(*((c)++)))<<16L, \
			 l|=((unsigned long)(*((c)++)))<< 8L, \
			 l|=((unsigned long)(*((c)++))))

#define l2n(l,c)	(*((c)++)=(unsigned char)(((l)>>24L)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16L)&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8L)&0xff), \
			 *((c)++)=(unsigned char)(((l)     )&0xff))

/* NOTE - c is not incremented as per l2c */
#define l2cn(l1,l2,c,n)	{ \
			c+=n; \
			switch (n) { \
			case 8: *(--(c))=(unsigned char)(((l2)>>24L)&0xff); \
			case 7: *(--(c))=(unsigned char)(((l2)>>16L)&0xff); \
			case 6: *(--(c))=(unsigned char)(((l2)>> 8L)&0xff); \
			case 5: *(--(c))=(unsigned char)(((l2)     )&0xff); \
			case 4: *(--(c))=(unsigned char)(((l1)>>24L)&0xff); \
			case 3: *(--(c))=(unsigned char)(((l1)>>16L)&0xff); \
			case 2: *(--(c))=(unsigned char)(((l1)>> 8L)&0xff); \
			case 1: *(--(c))=(unsigned char)(((l1)     )&0xff); \
				} \
			}

/* The changes to this macro may help or hinder, depending on the compiler
 * and the achitecture.  gcc2 always seems to do well :-). Inspired by Dana
 * How <how@isl.stanford.edu> DO NOT use the alternative version on machines
 * with 8 byte longs. */
#ifdef DES_USR_PTR
#define D_ENCRYPT(L,R,S) { \
	u=((R^s[S  ])<<2);	\
	t= R^s[S+1]; \
	t=((t>>2)+(t<<30)); \
	L^= \
	*(unsigned long *)(des_SP+0x0100+((t    )&0xfc))+ \
	*(unsigned long *)(des_SP+0x0300+((t>> 8)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0500+((t>>16)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0700+((t>>24)&0xfc))+ \
	*(unsigned long *)(des_SP+       ((u    )&0xfc))+ \
	*(unsigned long *)(des_SP+0x0200+((u>> 8)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0400+((u>>16)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0600+((u>>24)&0xfc)); }
#else				/* original version */
#ifdef MSDOS
#define D_ENCRYPT(L,R,S)	\
	U.l=R^s[S+1]; \
	T.s[0]=((U.s[0]>>4)|(U.s[1]<<12))&0x3f3f; \
	T.s[1]=((U.s[1]>>4)|(U.s[0]<<12))&0x3f3f; \
	U.l=(R^s[S  ])&0x3f3f3f3fL; \
	L^=	des_SPtrans[1][(T.c[0])]| \
		des_SPtrans[3][(T.c[1])]| \
		des_SPtrans[5][(T.c[2])]| \
		des_SPtrans[7][(T.c[3])]| \
		des_SPtrans[0][(U.c[0])]| \
		des_SPtrans[2][(U.c[1])]| \
		des_SPtrans[4][(U.c[2])]| \
		des_SPtrans[6][(U.c[3])];
#else
#define D_ENCRYPT(Q,R,S) {\
	u=(R^s[S  ]); \
	t=R^s[S+1]; \
	t=((t>>4L)+(t<<28L)); \
	Q^=	des_SPtrans[1][(t     )&0x3f]| \
		des_SPtrans[3][(t>> 8L)&0x3f]| \
		des_SPtrans[5][(t>>16L)&0x3f]| \
		des_SPtrans[7][(t>>24L)&0x3f]| \
		des_SPtrans[0][(u     )&0x3f]| \
		des_SPtrans[2][(u>> 8L)&0x3f]| \
		des_SPtrans[4][(u>>16L)&0x3f]| \
		des_SPtrans[6][(u>>24L)&0x3f]; }
#endif
#endif

/* IP and FP The problem is more of a geometric problem that random bit
 * fiddling. 0  1  2  3  4  5  6  7      62 54 46 38 30 22 14  6 8  9 10 11
 * 12 13 14 15      60 52 44 36 28 20 12  4 16 17 18 19 20 21 22 23      58
 * 50 42 34 26 18 10  2 24 25 26 27 28 29 30 31  to  56 48 40 32 24 16  8  0
 * 
 * 32 33 34 35 36 37 38 39      63 55 47 39 31 23 15  7 40 41 42 43 44 45 46 47
 * 61 53 45 37 29 21 13  5 48 49 50 51 52 53 54 55      59 51 43 35 27 19 11
 * 3 56 57 58 59 60 61 62 63      57 49 41 33 25 17  9  1
 * 
 * The output has been subject to swaps of the form 0 1 -> 3 1 but the odd and
 * even bits have been put into 2 3    2 0 different words.  The main trick
 * is to remember that t=((l>>size)^r)&(mask); r^=t; l^=(t<<size); can be
 * used to swap and move bits between words.
 * 
 * So l =  0  1  2  3  r = 16 17 18 19 4  5  6  7      20 21 22 23 8  9 10 11 24
 * 25 26 27 12 13 14 15      28 29 30 31 becomes (for size == 2 and mask ==
 * 0x3333) t =   2^16  3^17 -- --   l =  0  1 16 17  r =  2  3 18 19 6^20
 * 7^21 -- --        4  5 20 21       6  7 22 23 10^24 11^25 -- --        8 9
 * 24 25      10 11 24 25 14^28 15^29 -- --       12 13 28 29      14 15 28
 * 29
 * 
 * Thanks for hints from Richard Outerbridge - he told me IP&FP could be done in
 * 15 xor, 10 shifts and 5 ands. When I finally started to think of the
 * problem in 2D I first got ~42 operations without xors.  When I remembered
 * how to use xors :-) I got it to its final state. */
#define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
	(b)^=(t),\
	(a)^=((t)<<(n)))

#define IP(l,r) \
	{ \
	register unsigned long tt; \
	PERM_OP(r,l,tt, 4,0x0f0f0f0fL); \
	PERM_OP(l,r,tt,16,0x0000ffffL); \
	PERM_OP(r,l,tt, 2,0x33333333L); \
	PERM_OP(l,r,tt, 8,0x00ff00ffL); \
	PERM_OP(r,l,tt, 1,0x55555555L); \
	}

#define FP(l,r) \
	{ \
	register unsigned long tt; \
	PERM_OP(l,r,tt, 1,0x55555555L); \
	PERM_OP(r,l,tt, 8,0x00ff00ffL); \
	PERM_OP(l,r,tt, 2,0x33333333L); \
	PERM_OP(r,l,tt,16,0x0000ffffL); \
	PERM_OP(l,r,tt, 4,0x0f0f0f0fL); \
	}
#endif

/*-------------------------------  spr.h ------------------------------------*/

static unsigned long des_SPtrans[8][64] = {	/* removed 'const' */
    {
	/* nibble 0 */
	0x00820200L, 0x00020000L, 0x80800000L, 0x80820200L,
	0x00800000L, 0x80020200L, 0x80020000L, 0x80800000L,
	0x80020200L, 0x00820200L, 0x00820000L, 0x80000200L,
	0x80800200L, 0x00800000L, 0x00000000L, 0x80020000L,
	0x00020000L, 0x80000000L, 0x00800200L, 0x00020200L,
	0x80820200L, 0x00820000L, 0x80000200L, 0x00800200L,
	0x80000000L, 0x00000200L, 0x00020200L, 0x80820000L,
	0x00000200L, 0x80800200L, 0x80820000L, 0x00000000L,
	0x00000000L, 0x80820200L, 0x00800200L, 0x80020000L,
	0x00820200L, 0x00020000L, 0x80000200L, 0x00800200L,
	0x80820000L, 0x00000200L, 0x00020200L, 0x80800000L,
	0x80020200L, 0x80000000L, 0x80800000L, 0x00820000L,
	0x80820200L, 0x00020200L, 0x00820000L, 0x80800200L,
	0x00800000L, 0x80000200L, 0x80020000L, 0x00000000L,
	0x00020000L, 0x00800000L, 0x80800200L, 0x00820200L,
	0x80000000L, 0x80820000L, 0x00000200L, 0x80020200L,
    }, {
	/* nibble 1 */
	0x10042004L, 0x00000000L, 0x00042000L, 0x10040000L,
	0x10000004L, 0x00002004L, 0x10002000L, 0x00042000L,
	0x00002000L, 0x10040004L, 0x00000004L, 0x10002000L,
	0x00040004L, 0x10042000L, 0x10040000L, 0x00000004L,
	0x00040000L, 0x10002004L, 0x10040004L, 0x00002000L,
	0x00042004L, 0x10000000L, 0x00000000L, 0x00040004L,
	0x10002004L, 0x00042004L, 0x10042000L, 0x10000004L,
	0x10000000L, 0x00040000L, 0x00002004L, 0x10042004L,
	0x00040004L, 0x10042000L, 0x10002000L, 0x00042004L,
	0x10042004L, 0x00040004L, 0x10000004L, 0x00000000L,
	0x10000000L, 0x00002004L, 0x00040000L, 0x10040004L,
	0x00002000L, 0x10000000L, 0x00042004L, 0x10002004L,
	0x10042000L, 0x00002000L, 0x00000000L, 0x10000004L,
	0x00000004L, 0x10042004L, 0x00042000L, 0x10040000L,
	0x10040004L, 0x00040000L, 0x00002004L, 0x10002000L,
	0x10002004L, 0x00000004L, 0x10040000L, 0x00042000L,
    }, {
	/* nibble 2 */
	0x41000000L, 0x01010040L, 0x00000040L, 0x41000040L,
	0x40010000L, 0x01000000L, 0x41000040L, 0x00010040L,
	0x01000040L, 0x00010000L, 0x01010000L, 0x40000000L,
	0x41010040L, 0x40000040L, 0x40000000L, 0x41010000L,
	0x00000000L, 0x40010000L, 0x01010040L, 0x00000040L,
	0x40000040L, 0x41010040L, 0x00010000L, 0x41000000L,
	0x41010000L, 0x01000040L, 0x40010040L, 0x01010000L,
	0x00010040L, 0x00000000L, 0x01000000L, 0x40010040L,
	0x01010040L, 0x00000040L, 0x40000000L, 0x00010000L,
	0x40000040L, 0x40010000L, 0x01010000L, 0x41000040L,
	0x00000000L, 0x01010040L, 0x00010040L, 0x41010000L,
	0x40010000L, 0x01000000L, 0x41010040L, 0x40000000L,
	0x40010040L, 0x41000000L, 0x01000000L, 0x41010040L,
	0x00010000L, 0x01000040L, 0x41000040L, 0x00010040L,
	0x01000040L, 0x00000000L, 0x41010000L, 0x40000040L,
	0x41000000L, 0x40010040L, 0x00000040L, 0x01010000L,
    }, {
	/* nibble 3 */
	0x00100402L, 0x04000400L, 0x00000002L, 0x04100402L,
	0x00000000L, 0x04100000L, 0x04000402L, 0x00100002L,
	0x04100400L, 0x04000002L, 0x04000000L, 0x00000402L,
	0x04000002L, 0x00100402L, 0x00100000L, 0x04000000L,
	0x04100002L, 0x00100400L, 0x00000400L, 0x00000002L,
	0x00100400L, 0x04000402L, 0x04100000L, 0x00000400L,
	0x00000402L, 0x00000000L, 0x00100002L, 0x04100400L,
	0x04000400L, 0x04100002L, 0x04100402L, 0x00100000L,
	0x04100002L, 0x00000402L, 0x00100000L, 0x04000002L,
	0x00100400L, 0x04000400L, 0x00000002L, 0x04100000L,
	0x04000402L, 0x00000000L, 0x00000400L, 0x00100002L,
	0x00000000L, 0x04100002L, 0x04100400L, 0x00000400L,
	0x04000000L, 0x04100402L, 0x00100402L, 0x00100000L,
	0x04100402L, 0x00000002L, 0x04000400L, 0x00100402L,
	0x00100002L, 0x00100400L, 0x04100000L, 0x04000402L,
	0x00000402L, 0x04000000L, 0x04000002L, 0x04100400L,
    }, {
	/* nibble 4 */
	0x02000000L, 0x00004000L, 0x00000100L, 0x02004108L,
	0x02004008L, 0x02000100L, 0x00004108L, 0x02004000L,
	0x00004000L, 0x00000008L, 0x02000008L, 0x00004100L,
	0x02000108L, 0x02004008L, 0x02004100L, 0x00000000L,
	0x00004100L, 0x02000000L, 0x00004008L, 0x00000108L,
	0x02000100L, 0x00004108L, 0x00000000L, 0x02000008L,
	0x00000008L, 0x02000108L, 0x02004108L, 0x00004008L,
	0x02004000L, 0x00000100L, 0x00000108L, 0x02004100L,
	0x02004100L, 0x02000108L, 0x00004008L, 0x02004000L,
	0x00004000L, 0x00000008L, 0x02000008L, 0x02000100L,
	0x02000000L, 0x00004100L, 0x02004108L, 0x00000000L,
	0x00004108L, 0x02000000L, 0x00000100L, 0x00004008L,
	0x02000108L, 0x00000100L, 0x00000000L, 0x02004108L,
	0x02004008L, 0x02004100L, 0x00000108L, 0x00004000L,
	0x00004100L, 0x02004008L, 0x02000100L, 0x00000108L,
	0x00000008L, 0x00004108L, 0x02004000L, 0x02000008L,
    }, {
	/* nibble 5 */
	0x20000010L, 0x00080010L, 0x00000000L, 0x20080800L,
	0x00080010L, 0x00000800L, 0x20000810L, 0x00080000L,
	0x00000810L, 0x20080810L, 0x00080800L, 0x20000000L,
	0x20000800L, 0x20000010L, 0x20080000L, 0x00080810L,
	0x00080000L, 0x20000810L, 0x20080010L, 0x00000000L,
	0x00000800L, 0x00000010L, 0x20080800L, 0x20080010L,
	0x20080810L, 0x20080000L, 0x20000000L, 0x00000810L,
	0x00000010L, 0x00080800L, 0x00080810L, 0x20000800L,
	0x00000810L, 0x20000000L, 0x20000800L, 0x00080810L,
	0x20080800L, 0x00080010L, 0x00000000L, 0x20000800L,
	0x20000000L, 0x00000800L, 0x20080010L, 0x00080000L,
	0x00080010L, 0x20080810L, 0x00080800L, 0x00000010L,
	0x20080810L, 0x00080800L, 0x00080000L, 0x20000810L,
	0x20000010L, 0x20080000L, 0x00080810L, 0x00000000L,
	0x00000800L, 0x20000010L, 0x20000810L, 0x20080800L,
	0x20080000L, 0x00000810L, 0x00000010L, 0x20080010L,
    }, {
	/* nibble 6 */
	0x00001000L, 0x00000080L, 0x00400080L, 0x00400001L,
	0x00401081L, 0x00001001L, 0x00001080L, 0x00000000L,
	0x00400000L, 0x00400081L, 0x00000081L, 0x00401000L,
	0x00000001L, 0x00401080L, 0x00401000L, 0x00000081L,
	0x00400081L, 0x00001000L, 0x00001001L, 0x00401081L,
	0x00000000L, 0x00400080L, 0x00400001L, 0x00001080L,
	0x00401001L, 0x00001081L, 0x00401080L, 0x00000001L,
	0x00001081L, 0x00401001L, 0x00000080L, 0x00400000L,
	0x00001081L, 0x00401000L, 0x00401001L, 0x00000081L,
	0x00001000L, 0x00000080L, 0x00400000L, 0x00401001L,
	0x00400081L, 0x00001081L, 0x00001080L, 0x00000000L,
	0x00000080L, 0x00400001L, 0x00000001L, 0x00400080L,
	0x00000000L, 0x00400081L, 0x00400080L, 0x00001080L,
	0x00000081L, 0x00001000L, 0x00401081L, 0x00400000L,
	0x00401080L, 0x00000001L, 0x00001001L, 0x00401081L,
	0x00400001L, 0x00401080L, 0x00401000L, 0x00001001L,
    }, {
	/* nibble 7 */
	0x08200020L, 0x08208000L, 0x00008020L, 0x00000000L,
	0x08008000L, 0x00200020L, 0x08200000L, 0x08208020L,
	0x00000020L, 0x08000000L, 0x00208000L, 0x00008020L,
	0x00208020L, 0x08008020L, 0x08000020L, 0x08200000L,
	0x00008000L, 0x00208020L, 0x00200020L, 0x08008000L,
	0x08208020L, 0x08000020L, 0x00000000L, 0x00208000L,
	0x08000000L, 0x00200000L, 0x08008020L, 0x08200020L,
	0x00200000L, 0x00008000L, 0x08208000L, 0x00000020L,
	0x00200000L, 0x00008000L, 0x08000020L, 0x08208020L,
	0x00008020L, 0x08000000L, 0x00000000L, 0x00208000L,
	0x08200020L, 0x08008020L, 0x08008000L, 0x00200020L,
	0x08208000L, 0x00000020L, 0x00200020L, 0x08008000L,
	0x08208020L, 0x00200000L, 0x08200000L, 0x08000020L,
	0x00208000L, 0x00008020L, 0x08008020L, 0x08200000L,
	0x00000020L, 0x08208000L, 0x00208020L, 0x00000000L,
	0x08000000L, 0x08200020L, 0x00008000L, 0x00208020L,
}};

/*-------------------------------  sk.h ------------------------------------*/

static unsigned long des_skb[8][64] = {	/* removed 'const' */
    {
	/* for C bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
	0x00000000L, 0x00000010L, 0x20000000L, 0x20000010L,
	0x00010000L, 0x00010010L, 0x20010000L, 0x20010010L,
	0x00000800L, 0x00000810L, 0x20000800L, 0x20000810L,
	0x00010800L, 0x00010810L, 0x20010800L, 0x20010810L,
	0x00000020L, 0x00000030L, 0x20000020L, 0x20000030L,
	0x00010020L, 0x00010030L, 0x20010020L, 0x20010030L,
	0x00000820L, 0x00000830L, 0x20000820L, 0x20000830L,
	0x00010820L, 0x00010830L, 0x20010820L, 0x20010830L,
	0x00080000L, 0x00080010L, 0x20080000L, 0x20080010L,
	0x00090000L, 0x00090010L, 0x20090000L, 0x20090010L,
	0x00080800L, 0x00080810L, 0x20080800L, 0x20080810L,
	0x00090800L, 0x00090810L, 0x20090800L, 0x20090810L,
	0x00080020L, 0x00080030L, 0x20080020L, 0x20080030L,
	0x00090020L, 0x00090030L, 0x20090020L, 0x20090030L,
	0x00080820L, 0x00080830L, 0x20080820L, 0x20080830L,
	0x00090820L, 0x00090830L, 0x20090820L, 0x20090830L,
    }, {
	/* for C bits (numbered as per FIPS 46) 7 8 10 11 12 13 */
	0x00000000L, 0x02000000L, 0x00002000L, 0x02002000L,
	0x00200000L, 0x02200000L, 0x00202000L, 0x02202000L,
	0x00000004L, 0x02000004L, 0x00002004L, 0x02002004L,
	0x00200004L, 0x02200004L, 0x00202004L, 0x02202004L,
	0x00000400L, 0x02000400L, 0x00002400L, 0x02002400L,
	0x00200400L, 0x02200400L, 0x00202400L, 0x02202400L,
	0x00000404L, 0x02000404L, 0x00002404L, 0x02002404L,
	0x00200404L, 0x02200404L, 0x00202404L, 0x02202404L,
	0x10000000L, 0x12000000L, 0x10002000L, 0x12002000L,
	0x10200000L, 0x12200000L, 0x10202000L, 0x12202000L,
	0x10000004L, 0x12000004L, 0x10002004L, 0x12002004L,
	0x10200004L, 0x12200004L, 0x10202004L, 0x12202004L,
	0x10000400L, 0x12000400L, 0x10002400L, 0x12002400L,
	0x10200400L, 0x12200400L, 0x10202400L, 0x12202400L,
	0x10000404L, 0x12000404L, 0x10002404L, 0x12002404L,
	0x10200404L, 0x12200404L, 0x10202404L, 0x12202404L,
    }, {
	/* for C bits (numbered as per FIPS 46) 14 15 16 17 19 20 */
	0x00000000L, 0x00000001L, 0x00040000L, 0x00040001L,
	0x01000000L, 0x01000001L, 0x01040000L, 0x01040001L,
	0x00000002L, 0x00000003L, 0x00040002L, 0x00040003L,
	0x01000002L, 0x01000003L, 0x01040002L, 0x01040003L,
	0x00000200L, 0x00000201L, 0x00040200L, 0x00040201L,
	0x01000200L, 0x01000201L, 0x01040200L, 0x01040201L,
	0x00000202L, 0x00000203L, 0x00040202L, 0x00040203L,
	0x01000202L, 0x01000203L, 0x01040202L, 0x01040203L,
	0x08000000L, 0x08000001L, 0x08040000L, 0x08040001L,
	0x09000000L, 0x09000001L, 0x09040000L, 0x09040001L,
	0x08000002L, 0x08000003L, 0x08040002L, 0x08040003L,
	0x09000002L, 0x09000003L, 0x09040002L, 0x09040003L,
	0x08000200L, 0x08000201L, 0x08040200L, 0x08040201L,
	0x09000200L, 0x09000201L, 0x09040200L, 0x09040201L,
	0x08000202L, 0x08000203L, 0x08040202L, 0x08040203L,
	0x09000202L, 0x09000203L, 0x09040202L, 0x09040203L,
    }, {
	/* for C bits (numbered as per FIPS 46) 21 23 24 26 27 28 */
	0x00000000L, 0x00100000L, 0x00000100L, 0x00100100L,
	0x00000008L, 0x00100008L, 0x00000108L, 0x00100108L,
	0x00001000L, 0x00101000L, 0x00001100L, 0x00101100L,
	0x00001008L, 0x00101008L, 0x00001108L, 0x00101108L,
	0x04000000L, 0x04100000L, 0x04000100L, 0x04100100L,
	0x04000008L, 0x04100008L, 0x04000108L, 0x04100108L,
	0x04001000L, 0x04101000L, 0x04001100L, 0x04101100L,
	0x04001008L, 0x04101008L, 0x04001108L, 0x04101108L,
	0x00020000L, 0x00120000L, 0x00020100L, 0x00120100L,
	0x00020008L, 0x00120008L, 0x00020108L, 0x00120108L,
	0x00021000L, 0x00121000L, 0x00021100L, 0x00121100L,
	0x00021008L, 0x00121008L, 0x00021108L, 0x00121108L,
	0x04020000L, 0x04120000L, 0x04020100L, 0x04120100L,
	0x04020008L, 0x04120008L, 0x04020108L, 0x04120108L,
	0x04021000L, 0x04121000L, 0x04021100L, 0x04121100L,
	0x04021008L, 0x04121008L, 0x04021108L, 0x04121108L,
    }, {
	/* for D bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
	0x00000000L, 0x10000000L, 0x00010000L, 0x10010000L,
	0x00000004L, 0x10000004L, 0x00010004L, 0x10010004L,
	0x20000000L, 0x30000000L, 0x20010000L, 0x30010000L,
	0x20000004L, 0x30000004L, 0x20010004L, 0x30010004L,
	0x00100000L, 0x10100000L, 0x00110000L, 0x10110000L,
	0x00100004L, 0x10100004L, 0x00110004L, 0x10110004L,
	0x20100000L, 0x30100000L, 0x20110000L, 0x30110000L,
	0x20100004L, 0x30100004L, 0x20110004L, 0x30110004L,
	0x00001000L, 0x10001000L, 0x00011000L, 0x10011000L,
	0x00001004L, 0x10001004L, 0x00011004L, 0x10011004L,
	0x20001000L, 0x30001000L, 0x20011000L, 0x30011000L,
	0x20001004L, 0x30001004L, 0x20011004L, 0x30011004L,
	0x00101000L, 0x10101000L, 0x00111000L, 0x10111000L,
	0x00101004L, 0x10101004L, 0x00111004L, 0x10111004L,
	0x20101000L, 0x30101000L, 0x20111000L, 0x30111000L,
	0x20101004L, 0x30101004L, 0x20111004L, 0x30111004L,
    }, {
	/* for D bits (numbered as per FIPS 46) 8 9 11 12 13 14 */
	0x00000000L, 0x08000000L, 0x00000008L, 0x08000008L,
	0x00000400L, 0x08000400L, 0x00000408L, 0x08000408L,
	0x00020000L, 0x08020000L, 0x00020008L, 0x08020008L,
	0x00020400L, 0x08020400L, 0x00020408L, 0x08020408L,
	0x00000001L, 0x08000001L, 0x00000009L, 0x08000009L,
	0x00000401L, 0x08000401L, 0x00000409L, 0x08000409L,
	0x00020001L, 0x08020001L, 0x00020009L, 0x08020009L,
	0x00020401L, 0x08020401L, 0x00020409L, 0x08020409L,
	0x02000000L, 0x0A000000L, 0x02000008L, 0x0A000008L,
	0x02000400L, 0x0A000400L, 0x02000408L, 0x0A000408L,
	0x02020000L, 0x0A020000L, 0x02020008L, 0x0A020008L,
	0x02020400L, 0x0A020400L, 0x02020408L, 0x0A020408L,
	0x02000001L, 0x0A000001L, 0x02000009L, 0x0A000009L,
	0x02000401L, 0x0A000401L, 0x02000409L, 0x0A000409L,
	0x02020001L, 0x0A020001L, 0x02020009L, 0x0A020009L,
	0x02020401L, 0x0A020401L, 0x02020409L, 0x0A020409L,
    }, {
	/* for D bits (numbered as per FIPS 46) 16 17 18 19 20 21 */
	0x00000000L, 0x00000100L, 0x00080000L, 0x00080100L,
	0x01000000L, 0x01000100L, 0x01080000L, 0x01080100L,
	0x00000010L, 0x00000110L, 0x00080010L, 0x00080110L,
	0x01000010L, 0x01000110L, 0x01080010L, 0x01080110L,
	0x00200000L, 0x00200100L, 0x00280000L, 0x00280100L,
	0x01200000L, 0x01200100L, 0x01280000L, 0x01280100L,
	0x00200010L, 0x00200110L, 0x00280010L, 0x00280110L,
	0x01200010L, 0x01200110L, 0x01280010L, 0x01280110L,
	0x00000200L, 0x00000300L, 0x00080200L, 0x00080300L,
	0x01000200L, 0x01000300L, 0x01080200L, 0x01080300L,
	0x00000210L, 0x00000310L, 0x00080210L, 0x00080310L,
	0x01000210L, 0x01000310L, 0x01080210L, 0x01080310L,
	0x00200200L, 0x00200300L, 0x00280200L, 0x00280300L,
	0x01200200L, 0x01200300L, 0x01280200L, 0x01280300L,
	0x00200210L, 0x00200310L, 0x00280210L, 0x00280310L,
	0x01200210L, 0x01200310L, 0x01280210L, 0x01280310L,
    }, {
	/* for D bits (numbered as per FIPS 46) 22 23 24 25 27 28 */
	0x00000000L, 0x04000000L, 0x00040000L, 0x04040000L,
	0x00000002L, 0x04000002L, 0x00040002L, 0x04040002L,
	0x00002000L, 0x04002000L, 0x00042000L, 0x04042000L,
	0x00002002L, 0x04002002L, 0x00042002L, 0x04042002L,
	0x00000020L, 0x04000020L, 0x00040020L, 0x04040020L,
	0x00000022L, 0x04000022L, 0x00040022L, 0x04040022L,
	0x00002020L, 0x04002020L, 0x00042020L, 0x04042020L,
	0x00002022L, 0x04002022L, 0x00042022L, 0x04042022L,
	0x00000800L, 0x04000800L, 0x00040800L, 0x04040800L,
	0x00000802L, 0x04000802L, 0x00040802L, 0x04040802L,
	0x00002800L, 0x04002800L, 0x00042800L, 0x04042800L,
	0x00002802L, 0x04002802L, 0x00042802L, 0x04042802L,
	0x00000820L, 0x04000820L, 0x00040820L, 0x04040820L,
	0x00000822L, 0x04000822L, 0x00040822L, 0x04040822L,
	0x00002820L, 0x04002820L, 0x00042820L, 0x04042820L,
	0x00002822L, 0x04002822L, 0x00042822L, 0x04042822L,
}};

/*-------------------------------  sk.h ------------------------------------*/

static unsigned char odd_parity[256] = {	/* removed 'const' */
    1, 1, 2, 2, 4, 4, 7, 7, 8, 8, 11, 11, 13, 13, 14, 14,
    16, 16, 19, 19, 21, 21, 22, 22, 25, 25, 26, 26, 28, 28, 31, 31,
    32, 32, 35, 35, 37, 37, 38, 38, 41, 41, 42, 42, 44, 44, 47, 47,
    49, 49, 50, 50, 52, 52, 55, 55, 56, 56, 59, 59, 61, 61, 62, 62,
    64, 64, 67, 67, 69, 69, 70, 70, 73, 73, 74, 74, 76, 76, 79, 79,
    81, 81, 82, 82, 84, 84, 87, 87, 88, 88, 91, 91, 93, 93, 94, 94,
    97, 97, 98, 98, 100, 100, 103, 103, 104, 104, 107, 107, 109, 109, 110, 110,
    112, 112, 115, 115, 117, 117, 118, 118, 121, 121, 122, 122, 124, 124, 127, 127,
    128, 128, 131, 131, 133, 133, 134, 134, 137, 137, 138, 138, 140, 140, 143, 143,
    145, 145, 146, 146, 148, 148, 151, 151, 152, 152, 155, 155, 157, 157, 158, 158,
    161, 161, 162, 162, 164, 164, 167, 167, 168, 168, 171, 171, 173, 173, 174, 174,
    176, 176, 179, 179, 181, 181, 182, 182, 185, 185, 186, 186, 188, 188, 191, 191,
    193, 193, 194, 194, 196, 196, 199, 199, 200, 200, 203, 203, 205, 205, 206, 206,
    208, 208, 211, 211, 213, 213, 214, 214, 217, 217, 218, 218, 220, 220, 223, 223,
    224, 224, 227, 227, 229, 229, 230, 230, 233, 233, 234, 234, 236, 236, 239, 239,
241, 241, 242, 242, 244, 244, 247, 247, 248, 248, 251, 251, 253, 253, 254, 254};
