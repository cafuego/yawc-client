
/* Define TCP QIO function codes. */
/* Use predefined codes which are meaningful only to TCP. */

#define TCP$OPEN     IO$_CREATE
#define TCP$CLOSE    IO$_DELETE
#define TCP$SEND     IO$_WRITEVBLK
#define TCP$RECEIVE  IO$_READVBLK
#define TCP$ABORT    IO$_DEACCESS
#define TCP$STATUS   IO$_ACPCONTROL
#define TCP$INFO     IO$_MODIFY

#define GTHST        IO$_SKIPFILE
#define GTH_LCLHST  0                      /* Local host information */
#define GTH_NAMADR  1                 /* Name to address translation */
#define GTH_ADRNAM  2                 /* Address to name translation */

/* OPEN modes */
#define Active       1                     /* TCP - Connection is ACTIVE */
#define Passive      0                    /* TCP - Connection is PASSIVE */
#define UDPData      1         /* UDP - UDP data only (must match above) */
#define UDPAddr      0   /* UDP - IP address supplied (must match above) */
#define WILD         0                           /* Wild port specifier. */
#define Asynch       1                                         /* $QIO's */
#define Synch        0                                        /* $QIOW's */

/* Open flag bit positions ** Must match STRUCTURE and IPDRIVER ** */

#define OPF$Mode     1
#define OPF$Nowait   2
#define OPF$Addrflag 4

/* Name length and address count literals */

#define Host_Name_Max_Size  128
#define MAX_HNAME           Host_Name_Max_Size
#define MAX_HADDRS          20

/* TCB Connection States */

#define CS$Closed         0                        /* (not a real state) */
#define CS$Listen         1                           /* Waiting for SYN */
#define CS$SYN_Sent       2             /* SYN sent, waiting for SYN-ACK */
#define CS$SYN_RECV       3      /* SYN received & sent, waiting for ACK */
#define CS$Established    4     /* Connection ready to send/receive data */
#define CS$FIN_Wait_1     5     /* CLOSE done, FIN sent, waiting for ACK */
#define CS$FIN_Wait_2     6      /* ACK of FIN received, waiting for FIN */
#define CS$Time_Wait      7                /* FINs sent, received, ACKed */
#define CS$Close_Wait     8      /* FIN received, waiting for user CLOSE */
#define CS$Closing        9 /* FINs sent and received, waiting for ACK * */
#define CS$Last_ACK       10/* FINs sent and received, waiting for ACK** */
#define CS$Reset          11                        /* (not a TCP state) */
#define CS$Inactive       12   /* (not a TCP state) Connection is closed */
#define CS$NameLook       13/* (not a TCP state) Waiting for name lookup */

/* * State only reached via FIN-WAIT-1 (local initiation of close) */
/* ** State only reached via CLOSE-WAIT (remote initiation of close) */

/* I/O Status block (Quadword) field definitions.  These are basically the
   same as defined in VMS with the exception of the 2nd longword.
   The 2nd longword contains the ACP error code if the VMS Return code is
   SS$_ABORT.  For SS$_NORMAL requests, second longword contains flags and
   extra status information. */

/* !===============================================================! */
/* +        Bytes Transfered        |       VMS Return Code        + */
/* !---------------------------------------------------------------! */
/* +             Unused             |   Bit Flags   |   ICMP code  + */
/* !===============================================================! */

/* Type BitBool = [BIT(1),HIDDEN] Boolean; */
typedef unsigned char UByte;
typedef unsigned short UWord;
typedef unsigned long ULong;
typedef struct {
           UWord NSB$STATUS;
           UWord NSB$BYTE_COUNT;
           union { ULong NSB$XSTATUS;
                   struct { UByte NSB$ICMP_CODE; 
                            UByte NSB$FLAGS;
                   } NSB$2BYTES;
                   struct { unsigned NSB$FILL1  :8; 
                            unsigned NSB$F_URG  :1; unsigned NSB$F_PUSH :1;
                            unsigned NSB$F_EOF  :1; unsigned NSB$F_ICMP :1;
                   } NSB$FLAGBITS;
                   ULong NSB$XERROR;
           } NSB$UNION2;
         } NetSB_Fields;

typedef struct {
           ULong SOURCE_ADDR;
           ULong DEST_ADDR;
           UWord SOURCE_PORT;
           UWord DEST_PORT;
         } UDP_HostInfo;

/* Define protocol codes for open */

#define U$TCP_Protocol  0                      /* TCP protocol (default) */
#define U$UDP_Protocol  1                                /* UDP protocol */
