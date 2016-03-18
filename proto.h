/*
 *  This is the home of function prototypes for all the global functions.
 */

#ifdef  __STDC__
# define P(x) x
#else
# define P(x) ()
#endif

extern void
  sleeping P((int)),
  back P((int)),
  colorize P((char *)),
  color P((int)),
  configbbsrc P((void)),
  connect_fp P((void)),
  connectbbs P((int)),
  ColourChar P((int)),
  deinitialize P((void)),
  editusers P((char [][20], int *, int, char *)),
  fatalexit P((char *)),
  fatalperror P((char *)),
  findhome P((void)),
  flush_input P((int)),
  get_some_lines P((int)),
  get_string P((int, char *, int, int)),
  initialize P((char *)),
  looper P((void)),
  makemessage P((int)),
  myexit P((void)),
  mysleep P((int)),
  newmacro P((int)),
  new_color P((char *)),
  opentmpfile P((void)),
  readbbsrc P((char*[], int)),
  resetterm P((void)),
  run P((char *, char *)),
  save_bbsrc P((void)),
  sendblock P((void)),
  sendnaws P((void)),
  sendping P((void)),
  setterm P((void)),
  siginit P((void)),
  sigoff P((void)),
  suspend P((void)),
  telinit P((void)),
  tempfileerror P((void)),
  term_sig P((int)),
  truncbbsrc P((int));

extern int
  check_for_other_sessions P((void)),
  checkfile P((FILE *)),
  connect_prompt P((void)),
  enter_awaymessage P((void)),
  friends2tab P((void)),
  getkey P((void)),
  get_old_pw P((char *, char *, char *, int, int)),
  getwindowsize P((void)),
  inkey P((void)),
  more P((int *, int)),
  newkey P((int)),
  new_password P((char *, char *, int)),
  prompt P((FILE *, int *, int)),
  strucmp P((char *, char *)),
  telrcv P((int)),
  waitnextevent P((void)),
  cprintf P((char *, ...)),
  YAWC_editor P((char *, int, int, int, char *)),
  yesno P((void));

extern char
  *do_des P((char *, char *, int)),
  *get_name P((int)),
  *mybzero P((char *, int)),
  *mystrchr P((char *, int)),
  *mystrstr P((char *, char *)),
  *my_crypt P((char *)),
  *strctrl P((int)),
  *colour_so_far P((char *, int));


extern FILE
  *findbbsrc P((void)),
  *openbbsrc P((void));
