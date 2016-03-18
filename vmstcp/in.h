
/*
 * Internet address (a structure for historical reasons)
 */
struct in_addr {
	u_long s_addr;
};

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
	short	sin_family;
	u_short sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};

