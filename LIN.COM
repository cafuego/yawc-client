!
! Link command file for the VMS client
!
! Note:  The cluster specification is required to force the linker to
! use our versions of printf, fprintf, etc...
!
! If we're using GCC, use the GCC object library
! If we're on an Alpha, the shareable library has a different file name
!
$if (P1 .eqs. "VAXC") .or. (P1 .eqs. "")
$then
!
! VAXC
!
$link/exe=bbs sys$input:/opt
cluster=clus1,,,vms
main,config,utility,bbsrc,global,telnet,getline,edit,inkey,des_enc
sys$share:vaxcrtl/shar
$endif
$if P1 .eqs. "GCC"
$then
!
! GCC (assumes gcclib is in the GCC directory.  on some systems it might
!          be in sys$library)
!
$link/exe=bbs sys$input:/opt
cluster=clus1,,,vms
main,utility,config,bbsrc,global,telnet,getline,edit,inkey,des_enc
gnu_cc:[000000]gcclib/lib
sys$share:vaxcrtl/shar
$endif
$if P1 .eqs. "ALPHA"
$then
!
! ALPHA
!
$link/exe=bbs sys$input:/opt
cluster=clus1,,,vms
main,config,utility,bbsrc,global,telnet,getline,edit,inkey,des_enc
sys$share:decc$shr/shar
$endif
$exit
