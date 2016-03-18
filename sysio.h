/*
 * This is the file for I/O defines, different systems do certain things
 * differently, such system-specific stuff should be put here.
 */

/*
 * Sure would be nice if there was a standard interface to this kind of
 * information, it is very useful.
 */
#ifdef vms
# define  INPUT_LEFT(__fp)   ((__fp) == stdin ? car_in_func() : net_in_func())
#else
# ifdef linux
#  ifdef _IO_file_flags
#   define INPUT_LEFT(__fp)   ((__fp)->_IO_read_ptr < (__fp)->_IO_read_end)
#  else
#   define INPUT_LEFT(__fp)   ((__fp)->_gptr < (__fp)->_egptr)
#  endif
# else
#  if ((BSD >= 44) || defined(bsdi)) && !defined(__osf__)
#   define INPUT_LEFT(__fp)  ((__fp)->_r > 0)
#  else
#   define INPUT_LEFT(__fp)   ((__fp)->_cnt > 0)
#  endif
# endif
#endif


/*
 * Defines for network I/O, standard I/O can't always be used on network
 * connections in the non-Unix world.
 */
#ifndef vms
# define NET_INPUT_LEFT() (INPUT_LEFT(netifp))
# define netget()         (getc(netifp))
# define netput(__c)      (putc(__c, netofp))
# define netflush()	  (fflush(netofp))
#else
# define NET_INPUT_LEFT() (INPUT_LEFT(netifp))
# define netget()	  (net_rcv())
# define netput(__c)	  (net_send(__c), 0)
# ifdef __ALPHA
#  define getchar()       (fgetc(stdin))
#  define putchar(__c)    (fputc(__c,stdout))
# endif
#endif
