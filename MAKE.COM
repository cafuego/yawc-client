!
! This procedure simulates a simple "make"
! It will compile any out of date module, and link if necessary.
! It is NOT meant to be complete and/or foolproof.
!
! Specify a gcc parameter for GCC.  Default is VAXC
!
$if P1 .eqs. "" then P1 = "VAXC"
$if P1 .eqs. "GCC" then mycc = "gcc/opt=2"
$if P1 .eqs. "VAXC" then mycc = "cc/def=__STDC__"
$if P1 .eqs. "ALPHA" then mycc = "cc/nowarn"
$date_ext = f$cvtime(f$file_attributes("ext.h","RDT"))
$date_def = f$cvtime(f$file_attributes("defs.h","RDT"))
$max_obj = "0"
$on control_y then goto abort
$on error then continue
$set mess/nofac/nosev/notex/noid
$define sys$command sys$input
$set mess/fac/sev/tex/id
$!
$!
$!     Add any new source files in here
$!
$create cfiles.dat
bbsrc
main
global
utility
vms
config
telnet
getline
edit
inkey
des_enc
$open/read files cfiles.dat
$loop:
$read/end=end files file
$obj = file+".obj"
$c = file+".c"
$if f$search(obj) .eqs. ""
$then
$ write sys$output "Compiling "+c
$mycc 'file'
$endif
$date_src = f$cvtime(f$file_attributes(c,"RDT"))
$date_obj = f$cvtime(f$file_attributes(obj,"RDT"))
$if date_src .gts. date_obj .or. date_ext .gts. date_obj .or. date_def .gts. date_obj
$then
$ write sys$output "Compiling "+c
$mycc 'file'
$endif
$date_obj = f$cvtime(f$file_attributes(obj,"RDT"))
$if date_obj .gts. max_obj then max_obj = date_obj
$got loop
$end:
!
$!Check for linking
!
$li=0
$if f$search("bbs.exe") .eqs. "" 
$then 
$li=1
$else
$date_exe = f$cvtime(f$file_attributes("bbs.exe","RDT"))
$if max_obj .gts. date_exe then li=1
$endif
$if li .eq. 1
$then
$write sys$output "Linking..."
$@lin 'P1'
$endif
$abort:
$write sys$output "Done"
$delete cfiles.dat.*
$close files
$set on
$set nover
$exit
