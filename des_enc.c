/* <des routines> Copyright (C) 1995 Eric Young (eay@mincom.oz.au) All rights
 * reserved.
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

#include <string.h>
#include "des.h"

char *DES_version = "libdes v 3.21 - 95/11/21 - eay";

#ifdef PROTO
static int check_parity(des_cblock(*key));
#else
static int check_parity();
#endif

int des_check_key = 0;

/*---------------------------- str2key.c ------------------------------------*/

unsigned long
des_cbc_cksum(input, output, length, schedule, ivec)
des_cblock(*input);
des_cblock(*output);
    long length;
    des_key_schedule schedule;
des_cblock(*ivec);
{
    register unsigned long tout0, tout1, tin0, tin1;
    register long l = length;
    unsigned long tin[2];
    unsigned char *in, *out, *iv;

    in = (unsigned char *)input;
    out = (unsigned char *)output;
    iv = (unsigned char *)ivec;

    c2l(iv, tout0);
    c2l(iv, tout1);
    for (; l > 0; l -= 8) {
	if (l >= 8) {
	    c2l(in, tin0);
	    c2l(in, tin1);
	} else
	    c2ln(in, tin0, tin1, l);

	tin0 ^= tout0;
	tin[0] = tin0;
	tin1 ^= tout1;
	tin[1] = tin1;
	des_encrypt((unsigned long *)tin, schedule, DES_ENCRYPT);
	/* fix 15/10/91 eay - thanks to keithr@sco.COM */
	tout0 = tin[0];
	tout1 = tin[1];
    }
    if (out != NULL) {
	l2c(tout0, out);
	l2c(tout1, out);
    }
    tout0 = tin0 = tin1 = tin[0] = tin[1] = 0;
    return (tout1);
}

/*---------------------------- str2key.c ------------------------------------*/

void
des_string_to_key(str, key)
    char *str;
des_cblock(*key);
{
    des_key_schedule ks;
    int i, length;
    register unsigned char j;

    memset(key, 0, 8);
    length = strlen(str);
#ifdef OLD_STR_TO_KEY
    for (i = 0; i < length; i++)
	(*key)[i % 8] ^= (str[i] << 1);
#else				/* MIT COMPATIBLE */
    for (i = 0; i < length; i++) {
	j = str[i];
	if ((i % 16) < 8)
	    (*key)[i % 8] ^= (j << 1);
	else {
	    /* Reverse the bit order 05/05/92 eay */
	    j = ((j << 4) & 0xf0) | ((j >> 4) & 0x0f);
	    j = ((j << 2) & 0xcc) | ((j >> 2) & 0x33);
	    j = ((j << 1) & 0xaa) | ((j >> 1) & 0x55);
	    (*key)[7 - (i % 8)] ^= j;
	}
    }
#endif
    des_set_odd_parity((des_cblock *) key);
    i = des_check_key;
    des_check_key = 0;
    des_set_key((des_cblock *) key, ks);
    des_check_key = i;
    des_cbc_cksum((des_cblock *) str, (des_cblock *) key, (long)length, ks,
		  (des_cblock *) key);
    memset(ks, 0, sizeof(ks));
    des_set_odd_parity((des_cblock *) key);
}

void
des_string_to_2keys(str, key1, key2)
    char *str;
des_cblock(*key1);
des_cblock(*key2);
{
    des_key_schedule ks;
    int i, length;
    register unsigned char j;

    memset(key1, 0, 8);
    memset(key2, 0, 8);
    length = strlen(str);
#ifdef OLD_STR_TO_KEY
    if (length <= 8) {
	for (i = 0; i < length; i++) {
	    (*key2)[i] = (*key1)[i] = (str[i] << 1);
	}
    } else {
	for (i = 0; i < length; i++) {
	    if ((i / 8) & 1)
		(*key2)[i % 8] ^= (str[i] << 1);
	    else
		(*key1)[i % 8] ^= (str[i] << 1);
	}
    }
#else				/* MIT COMPATIBLE */
    for (i = 0; i < length; i++) {
	j = str[i];
	if ((i % 32) < 16) {
	    if ((i % 16) < 8)
		(*key1)[i % 8] ^= (j << 1);
	    else
		(*key2)[i % 8] ^= (j << 1);
	} else {
	    j = ((j << 4) & 0xf0) | ((j >> 4) & 0x0f);
	    j = ((j << 2) & 0xcc) | ((j >> 2) & 0x33);
	    j = ((j << 1) & 0xaa) | ((j >> 1) & 0x55);
	    if ((i % 16) < 8)
		(*key1)[7 - (i % 8)] ^= j;
	    else
		(*key2)[7 - (i % 8)] ^= j;
	}
    }
    if (length <= 8)
	memcpy(key2, key1, 8);
#endif
    des_set_odd_parity((des_cblock *) key1);
    des_set_odd_parity((des_cblock *) key2);
    i = des_check_key;
    des_check_key = 0;
    des_set_key((des_cblock *) key1, ks);
    des_cbc_cksum((des_cblock *) str, (des_cblock *) key1, (long)length, ks,
		  (des_cblock *) key1);
    des_set_key((des_cblock *) key2, ks);
    des_cbc_cksum((des_cblock *) str, (des_cblock *) key2, (long)length, ks,
		  (des_cblock *) key2);
    des_check_key = i;
    memset(ks, 0, sizeof(ks));
    des_set_odd_parity(key1);
    des_set_odd_parity(key2);
}

/*---------------------------- set_key.c ------------------------------------*/

void
des_set_odd_parity(key)
des_cblock(*key);
{
    int i;

    for (i = 0; i < DES_KEY_SZ; i++)
	(*key)[i] = odd_parity[(*key)[i]];
}

static int
check_parity(key)
des_cblock(*key);
{
    int i;

    for (i = 0; i < DES_KEY_SZ; i++) {
	if ((*key)[i] != odd_parity[(*key)[i]])
	    return (0);
    }
    return (1);
}

/* Weak and semi week keys as take from %A D.W. Davies %A W.L. Price %T
 * Security for Computer Networks %I John Wiley & Sons %D 1984 Many thanks to
 * smb@ulysses.att.com (Steven Bellovin) for the reference (and actual cblock
 * values). */
#define NUM_WEAK_KEY	16
static des_cblock weak_keys[NUM_WEAK_KEY] = {
    /* weak keys */
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE},
    {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    {0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0},
    /* semi-weak keys */
    {0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE},
    {0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01},
    {0x1F, 0xE0, 0x1F, 0xE0, 0x0E, 0xF1, 0x0E, 0xF1},
    {0xE0, 0x1F, 0xE0, 0x1F, 0xF1, 0x0E, 0xF1, 0x0E},
    {0x01, 0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1},
    {0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1, 0x01},
    {0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E, 0xFE},
    {0xFE, 0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E},
    {0x01, 0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E},
    {0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E, 0x01},
    {0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE},
{0xFE, 0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1}};

int
des_is_weak_key(key)
des_cblock(*key);
{
    int i;

    for (i = 0; i < NUM_WEAK_KEY; i++)
	/* Added == 0 to comparision, I obviously don't run this section very
	 * often :-(, thanks to engineering@MorningStar.Com for the fix eay
	 * 93/06/29 */
	if (memcmp(weak_keys[i], key, sizeof(key)) == 0)
	    return (1);
    return (0);
}

/* NOW DEFINED IN des_local.h See ecb_encrypt.c for a pseudo description of
 * these macros. #define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
 * (b)^=(t),\ (a)=((a)^((t)<<(n)))) */

#define HPERM_OP(a,t,n,m) ((t)=((((a)<<(16-(n)))^(a))&(m)),\
	(a)=(a)^(t)^(t>>(16-(n))))

static int shifts2[16] = {0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0};

/* return 0 if key parity is odd (correct), return -1 if key parity error,
 * return -2 if illegal weak key. */
int
des_set_key(key, schedule)
des_cblock(*key);
    des_key_schedule schedule;
{
    register unsigned long c, d, t, s;
    register unsigned char *in;
    register unsigned long *k;
    register int i;

    if (des_check_key) {
	if (!check_parity(key))
	    return (-1);

	if (des_is_weak_key(key))
	    return (-2);
    }
    k = (unsigned long *)schedule;
    in = (unsigned char *)key;

    c2l(in, c);
    c2l(in, d);

    /* do PC1 in 60 simple operations */
    /* PERM_OP(d,c,t,4,0x0f0f0f0fL); HPERM_OP(c,t,-2, 0xcccc0000L);
     * HPERM_OP(c,t,-1, 0xaaaa0000L); HPERM_OP(c,t, 8, 0x00ff0000L);
     * HPERM_OP(c,t,-1, 0xaaaa0000L); HPERM_OP(d,t,-8, 0xff000000L);
     * HPERM_OP(d,t, 8, 0x00ff0000L); HPERM_OP(d,t, 2, 0x33330000L);
     * d=((d&0x00aa00aaL)<<7L)|((d&0x55005500L)>>7L)|(d&0xaa55aa55L);
     * d=(d>>8)|((c&0xf0000000L)>>4); c&=0x0fffffffL; */

    /* I now do it in 47 simple operations :-) Thanks to John Fletcher
     * (john_fletcher@lccmail.ocf.llnl.gov) for the inspiration. :-) */
    PERM_OP(d, c, t, 4, 0x0f0f0f0fL);
    HPERM_OP(c, t, -2, 0xcccc0000L);
    HPERM_OP(d, t, -2, 0xcccc0000L);
    PERM_OP(d, c, t, 1, 0x55555555L);
    PERM_OP(c, d, t, 8, 0x00ff00ffL);
    PERM_OP(d, c, t, 1, 0x55555555L);
    d = (((d & 0x000000ffL) << 16L) | (d & 0x0000ff00L) |
	 ((d & 0x00ff0000L) >> 16L) | ((c & 0xf0000000L) >> 4L));
    c &= 0x0fffffffL;

    for (i = 0; i < ITERATIONS; i++) {
	if (shifts2[i]) {
	    c = ((c >> 2L) | (c << 26L));
	    d = ((d >> 2L) | (d << 26L));
	} else {
	    c = ((c >> 1L) | (c << 27L));
	    d = ((d >> 1L) | (d << 27L));
	}
	c &= 0x0fffffffL;
	d &= 0x0fffffffL;
	/* could be a few less shifts but I am to lazy at this point in time
	 * to investigate */
	s = des_skb[0][(c) & 0x3f] |
	    des_skb[1][((c >> 6) & 0x03) | ((c >> 7L) & 0x3c)] |
	    des_skb[2][((c >> 13) & 0x0f) | ((c >> 14L) & 0x30)] |
	    des_skb[3][((c >> 20) & 0x01) | ((c >> 21L) & 0x06) |
		       ((c >> 22L) & 0x38)];
	t = des_skb[4][(d) & 0x3f] |
	    des_skb[5][((d >> 7L) & 0x03) | ((d >> 8L) & 0x3c)] |
	    des_skb[6][(d >> 15L) & 0x3f] |
	    des_skb[7][((d >> 21L) & 0x0f) | ((d >> 22L) & 0x30)];

	/* table contained 0213 4657 */
	*(k++) = ((t << 16L) | (s & 0x0000ffffL)) & 0xffffffffL;
	s = ((s >> 16L) | (t & 0xffff0000L));

	s = (s << 4L) | (s >> 28L);
	*(k++) = s & 0xffffffffL;
    }
    return (0);
}

int
des_key_sched(key, schedule)
des_cblock(*key);
    des_key_schedule schedule;
{
    return (des_set_key(key, schedule));
}

/*---------------------------- ecb_enc.c ------------------------------------*/

void
des_ecb_encrypt(input, output, ks, encrypt)
des_cblock(*input);
des_cblock(*output);
    des_key_schedule ks;
    int encrypt;
{
    register unsigned long l0, l1;
    register unsigned char *in, *out;
    unsigned long ll[2];

    in = (unsigned char *)input;
    out = (unsigned char *)output;
    c2l(in, l0);
    ll[0] = l0;
    c2l(in, l1);
    ll[1] = l1;
    des_encrypt(ll, ks, encrypt);
    l0 = ll[0];
    l2c(l0, out);
    l1 = ll[1];
    l2c(l1, out);
    l0 = l1 = ll[0] = ll[1] = 0;
}

void
des_encrypt(data, ks, encrypt)
    unsigned long *data;
    des_key_schedule ks;
    int encrypt;
{
    register unsigned long l, r, t, u;
#ifdef DES_USE_PTR
    register unsigned char *des_SP = (unsigned char *)des_SPtrans;
#endif
#ifdef MSDOS
    union fudge {
	unsigned long l;
	unsigned short s[2];
	unsigned char c[4];
    }     U, T;
#endif
    register int i;
    register unsigned long *s;

    u = data[0];
    r = data[1];

    IP(u, r);
    /* Things have been modified so that the initial rotate is done outside
     * the loop.  This required the des_SPtrans values in sp.h to be rotated
     * 1 bit to the right. One perl script later and things have a 5% speed
     * up on a sparc2. Thanks to Richard Outerbridge
     * <71755.204@CompuServe.COM> for pointing this out. */
    l = (r << 1) | (r >> 31);
    r = (u << 1) | (u >> 31);

    /* clear the top bits on machines with 8byte longs */
    l &= 0xffffffffL;
    r &= 0xffffffffL;

    s = (unsigned long *)ks;
    /* I don't know if it is worth the effort of loop unrolling the inner
     * loop */
    if (encrypt) {
	for (i = 0; i < 32; i += 4) {
	    D_ENCRYPT(l, r, i + 0);	/* 1 */
	    D_ENCRYPT(r, l, i + 2);	/* 2 */
	}
    } else {
	for (i = 30; i > 0; i -= 4) {
	    D_ENCRYPT(l, r, i - 0);	/* 16 */
	    D_ENCRYPT(r, l, i - 2);	/* 15 */
	}
    }
    l = (l >> 1) | (l << 31);
    r = (r >> 1) | (r << 31);
    /* clear the top bits on machines with 8byte longs */
    l &= 0xffffffffL;
    r &= 0xffffffffL;

    FP(r, l);
    data[0] = l;
    data[1] = r;
    l = r = t = u = 0;
}

void
des_encrypt2(data, ks, encrypt)
    unsigned long *data;
    des_key_schedule ks;
    int encrypt;
{
    register unsigned long l, r, t, u;
#ifdef DES_USE_PTR
    register unsigned char *des_SP = (unsigned char *)des_SPtrans;
#endif
#ifdef MSDOS
    union fudge {
	unsigned long l;
	unsigned short s[2];
	unsigned char c[4];
    }     U, T;
#endif
    register int i;
    register unsigned long *s;

    u = data[0];
    r = data[1];

    /* Things have been modified so that the initial rotate is done outside
     * the loop.  This required the des_SPtrans values in sp.h to be rotated
     * 1 bit to the right. One perl script later and things have a 5% speed
     * up on a sparc2. Thanks to Richard Outerbridge
     * <71755.204@CompuServe.COM> for pointing this out. */
    l = (r << 1) | (r >> 31);
    r = (u << 1) | (u >> 31);

    /* clear the top bits on machines with 8byte longs */
    l &= 0xffffffffL;
    r &= 0xffffffffL;

    s = (unsigned long *)ks;
    /* I don't know if it is worth the effort of loop unrolling the inner
     * loop */
    if (encrypt) {
	for (i = 0; i < 32; i += 4) {
	    D_ENCRYPT(l, r, i + 0);	/* 1 */
	    D_ENCRYPT(r, l, i + 2);	/* 2 */
	}
    } else {
	for (i = 30; i > 0; i -= 4) {
	    D_ENCRYPT(l, r, i - 0);	/* 16 */
	    D_ENCRYPT(r, l, i - 2);	/* 15 */
	}
    }
    l = (l >> 1) | (l << 31);
    r = (r >> 1) | (r << 31);
    /* clear the top bits on machines with 8byte longs */
    l &= 0xffffffffL;
    r &= 0xffffffffL;

    data[0] = l;
    data[1] = r;
    l = r = t = u = 0;
}

/*************************************************************************
*
* My CLient DES routine (Flint).
* Takes input from s1 and codes it to s2 using password pw and mode enc.
*
* pw:  password                          (only first 8 byte are considered)
* s1:  if ENCRYPT: text to be encrypted, ( "     "   "  "    "       "    )
*      if DECRYPT: _has to be_ a 16 byte hex number (=8 numbers, upper case,
*                  no spaces; e.g. "BAB6288A2CA96ED4") which represents the
*                  64 bits of the encrypted password.
* enc: if 1 (DO_ENCRYPT) encrypt s1 to s2, if 0 (DO_DECRYPT) decrypt s1 to s2.
*
* result: if ENCRYPT: 16 byte hex number representing the encrypted pw, see
*         above
*         if DECRYPT: the decrypted text (max. 8 characters)
*         must point to allocated area with at least 17 (encryption) or
*         9 (decryption) bytes.
* The result is returned in a static string, so watch out for possible
* overwriting!
*
***************************************************************************/


#include "defs.h"

char *
do_des(pw, s1, enc)
    char *pw;
    char *s1;
    int enc;
{
    int i;
    static char s2[20];
    char *s3, *s4;
    des_cblock *key;
    des_key_schedule schedule;

    key = (des_cblock *) malloc(8);
    s3 = (char *)malloc(10);
    s4 = (char *)malloc(10);
    des_string_to_key(pw, key);	/* generate key from password and set it */
    des_set_key(key, schedule);

    if (enc == DECRYPT)
	for (i = 0; i < 8; i++) {	/* decode 16 byte hex number into 64
					 * bit field */
	    strncpy(s4, s1 + (2 * i), 2);
	    s4[2] = 0;
	    s3[i] = strtol(s4, NULL, 16);
	}
    else {
	memcpy(s3, s1, 8);
	s3[8] = 0;		/* is this necessary? probably not... */
    }

    for (i = 0; i < 100; i++) {	/* suggestion by Ishtar - multiple encrypting */
	des_ecb_encrypt((des_cblock *) s3, (des_cblock *) s4, schedule, enc);
	if (i < 99)
	    memcpy(s3, s4, 8);
    }

    if (enc == ENCRYPT) {
	s2[0] = 0;		/* code the encrypted 64 bits into a hex
				 * number */
	for (i = 0; i < 8; i++)
	    sprintf(s2, "%s%02X", s2, s4[i] & 255);
    } else {
	memcpy(s2, s4, 8);
	s2[8] = 0;
    }

    free(key);
    return (s2);
}

/* standalone test routine for do_des function  void main() { char passw[20],
 * orig[40], encr[40], decr[40];
 * 
 * sprintf(passw,"P@ssW0rd"); sprintf(orig, "Cleartxt"); sprintf(encr,
 * "garbagegarbagegarbagegarbagegarbagegarb"); sprintf(decr,
 * "garbagegarbagegarbagegarbagegarbagegarb");
 * 
 * printf("Original: <%s>\n", orig); strcpy (encr, do_des (passw, orig,
 * ENCRYPT)); printf("encrypted: <%s>\n", encr); printf("decrypted: <%s>\n",
 * do_des (passw, encr, DECRYPT)); } */
