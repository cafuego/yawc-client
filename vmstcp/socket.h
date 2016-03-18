#define SOCK_STREAM	1		/* stream socket */
#define AF_INET		2		/* internetwork: UDP, TCP, etc. */

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
	u_short sa_family;		/* address family */
	char	sa_data[14];		/* up to 14 bytes of direct address */
};

