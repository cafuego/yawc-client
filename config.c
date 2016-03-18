/*
 * This file handles configuration of the bbsrc file.  Its somewhat sloppy but
 * it should do the job.  Someone else can put a nicer interface on it. 
 *
 *** PR: I changed the save-format for macro's somewhat:
 ***     put an | at the end to allow spaces at the end of macro's
 ***     to be saved too!
 */
#include "defs.h"
#include "ext.h"


/*
 * Changes settings in bbsrc file and saves it. 
 */
void
configbbsrc()
{
char    tmp[80], tmp2[80];
register int c;
register int i;
register int j;
register int k;
int     invalid;
int     lines;
int	bbsgrp;

  configflag = 1;
  if (bbsrcro)
    printf("\r\nConfiguration file is read-only, cannot save configuration for next session.\r\n");
  else if (!bbsrc)
    printf("\r\nNo configuration file, cannot save configuration for next session.\r\n");
  for (;;)
  {
    colorize("\r\n\1a\1fw<rEw>gnemy list w<rFw>griend list w<rKw>geys w<rLw>gocal w<rMw>gacros w<rOw>gptions w-> ");
    for (invalid = 0;;)
    {
      c = inkey();
      if (!mystrchr("EeFfKkLlMmOoQqSs \n", c))
      {
	if (invalid++)
	  flush_input(invalid);
	continue;
      }
      break;
    }
    switch (c)
    {
      case 'l':
      case 'L':

	printf("Local Config.\r\n");
	for (; c != 'q';)
	{
	  colorize("\r\nw<rBw>gBS's w<rEw>gditor \1w<\1rM\1w>\1gaster password \1w<\1rA\1w>\1gway message w-> ");
	  for (invalid = 0;;)
	  {
	    c = inkey();
	    if (!mystrchr("AaBbEeMmQq \n", c))
	    {
	      if (invalid++)
		flush_input(invalid);
	      continue;
	    }
	    break;
	  }
	  switch (c)
	  {
	    case 'a':
	    case 'A':
	      putchar('\n');
	      enter_awaymessage();
	      break;

	    case 'b':
	    case 'B':

	      bbsgrp=0;
	      for(;;)
	        {
		  putchar('\n');
		  j=0; k=0;
	          for(i=0+12*bbsgrp; i<12+12*bbsgrp; i++)
		    if(strlen(bbsname[i])>j)
		      j=strlen(bbsname[i]);

		    sprintf(tmp,"  <\1r%%d\1w> ->  \1y%%-%d.%ds \1w- \1c%%s\1w\n", j,j);
		    sprintf(tmp2,"  <\1r%%c\1w> ->  \1y%%-%d.%ds \1w- \1c%%s\1w\n", j,j);
	          for(i=0+12*bbsgrp; i<12+12*bbsgrp; i++)
		    {
		      if(*bbshost[i])
			if(i>9)
			{
		          sprintf(colst, tmp2, i+55, bbsname[i], bbshost[i]);
			  colorize(colst);
			}
			else
			{
		          sprintf(colst, tmp, i, bbsname[i], bbshost[i]);
			  colorize(colst);
			}
		      else
			if(i>9)
			{
		          sprintf(colst, "  <\1r%c\1w> ->  \1r----- \1yUnused \1r-----\1w\n", i+55);
			  colorize(colst);
			}
			else
			{
		          sprintf(colst, "  <\1r%d\1w> ->  \1r----- \1yUnused \1r-----\1w\n", i);
			  colorize(colst);
			}
		    }

	          printf("\nEnter <slot> to change, <+>/<-> to see next/previous slots, <return> to quit.\n");
	          printf("Choice: ");

	          for(invalid = 0;;)
	            {
		      c = inkey();
		      if(!mystrchr("1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+- \n", c))
		        {
		          if(invalid++)
		            flush_input(invalid);
	                  continue;
			}
		      break;
		    }

		  if(mystrchr(" \n", c))
		    break;

		  if(c == '+')
		  {
		    if(bbsgrp<2)
		      bbsgrp++;
		    continue;
		  }

		  if(c == '-')
		  {
		    if(bbsgrp>0)
		      bbsgrp--;
		    continue;
		  }

		  if(mystrchr("1234567890", c))
	            i = c - 48;
		  if(mystrchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", c))
	            i = c - 55;
		  if(mystrchr("abcdefghijklmnopqrstuvwxyz", c))
	            i = c - 87;
		  if(*bbshost[i])
	            printf("\nBBS host address [%s]  -> ", bbshost[i]);
		  else
	            printf("\nBBS host address  -> ");
	          get_string(39, tmp, -999, 0);
		  if(strlen(tmp) < 2)
		  {
		    sprintf(colst, "Delete slot #%d (y/n)? ", i);
		    colorize(colst);
		    if(yesno() == YES)
	  	    {
		      tmp[0]=0;
		      bbshost[i][0]=0;
		      bbsname[i][0]=0;
		      bbsuser[i][0]=0;
		      bbspass[i][0]=0;
		      bbspw_e[i][0]=0;
		      printf("Host #%d deleted.\n", i);
		      continue;
		    }
		    else
		    {
		      printf("Abort setting up slot #%d (y/n)? ",i);
		      if(yesno() == YES)
			continue;
		      else
		      {
		        sprintf (colst, "Leaving entry '\1y%s\1w' unchanged.\n\n", bbshost[i]);
			colorize(colst);
		      }
		    }
		  }
		  else
		    strcpy(bbshost[i], tmp);

	          printf("Name of the BBS (don't append 'BBS'!) -> ");
	          get_string(24, tmp, -999, 0);
	          if (*tmp)
		  {
	            strcpy(bbsname[i], tmp);
		    strcat (bbsname[i], " BBS");
		  }
		  else
		  {
		    sprintf (colst, "Leaving entry '\1y%s\1w' unchanged.\n", bbsname[i]);
		    colorize(colst);
		  }

 		   printf("\nThis CLient supports auto login. You can provide a BBS username and password.\n");
		   colorize("\1r>>>> Note that it is insecure to store even encrypted passwords in a file. <<<<\1w\n");
 		   printf("If you want to do it, at least protect the data with a master password.\n");
 		   printf("Just hit <Return> if you don't want to store name or password.\n\n");
		  bbsuser[i][0]=0;
		  bbspass[i][0]=0;
	          printf("Your username of the BBS  -> ");
	          strcpy (tmp, get_name(2));
	          if (*tmp)
		  {
		    strcpy(bbsuser[i], tmp);
		    if(new_password(tmp, "Your BBS password", 1) == 0)
		    {
		      if(*master_password)
		      {
		        strcpy(bbspass[i], tmp);
		        strcpy(bbspw_e[i], do_des(master_password, tmp, ENCRYPT));
		      }
		      else
			if(*master_pw_enc)
			  printf("No correct master password given, won't change BBS password.\n");
			else
		          strcpy(bbspass[i], tmp);
		    }
		    else
		      printf("No password saved.\n");
		  }
		  else
		    printf("No username and password saved.\n");
		}
	      break;

	    case 'e': /* Zavvy suggested this fix; was 'l', 'L' */
	    case 'E':
	      printf("\r\nEnter name of local editor to use (%s) -> ", editor);
 	      get_string(72, tmp, -999, 0);
	      if (*tmp)
	        strcpy(editor, tmp);
	      break;

	    case 'm':
	    case 'M':
	      putchar('\n');
	      if(get_old_pw("Enter CURRENT password", master_pw_enc, 
                 master_password, 0, 0) == 1)
	      {
		printf("Nope, that wasn't correct. Won't change the master password.\n");
		break;
	      }
	      switch(new_password(tmp, "Your CLient master password", 4))
	      {
		case 0:
		  strcpy(master_password, tmp);
		  strcpy(master_pw_enc, my_crypt(master_password));
		  printf("Master password changed.\n");
		  break;

		case -1:
		  printf("\nDo you really want to delete the CLient master password (y/n)? ");
		  if(yesno() == NO)
		  {
		    printf("CLient master password unchanged.\n");
		    break;
		  }
		  mybzero(master_password, 20);
		  mybzero(master_pw_enc, 20);
		  printf("Master password deleted, BBS passwords won't be saved encrypted!\n");
		  colorize("\1rNote that leaving BBS passwords unprotected in the rc file is not very wise.\1w\n");
		  for(i=0; i<MAXSYS; i++)
		    *bbspw_e[i]=0;
		  break;

		case 1:
		  printf("Master password not changed.\n");
		  break;
	      }
	      break;

	    case 'q':
	    case 'Q':
	    case ' ':
	    case '\n':
	      printf("Quit\r\n");
	      c = 'q';
	      break;
	  }
	}
	break;


      case 'k':
      case 'K':
	printf("Keys\r\n\n");
	printf("Enter key to indicate start of macro (%s) -> ", strctrl(macrokey));
	for (;;)
	{
	  printf("%s\r\n", strctrl(macrokey = newkey(macrokey)));
	  if (macrokey < ' ')
	    break;
	  printf("You must use a control character for your macro key, try again -> ");
	}
	printf("Enter key to quit client (%s) -> ", strctrl(quitkey));
	printf("%s\r\n", strctrl(quitkey = newkey(quitkey)));
	printf("Enter key to suspend client (%s) -> ", strctrl(suspkey));
	printf("%s\r\n", strctrl(suspkey = newkey(suspkey)));
	printf("Enter key to start a new shell (%s) -> ", strctrl(shellkey));
	printf("%s\r\n", strctrl(shellkey = newkey(shellkey)));
	printf("Enter key to toggle capture mode (%s) -> ", strctrl(capturekey));
	printf("%s\r\n", strctrl(capturekey = newkey(capturekey)));
	printf("Enter key to get to the connect prompt (%s) -> ", strctrl(connectkey));
	printf("%s\r\n", strctrl(connectkey = newkey(connectkey)));
	break;

      case 'f':
      case 'F':
	printf("Friend list\r\n");
	editusers(friend, &friendp, sizeof friend / 20, "friend");
	break;

      case 'e':
      case 'E':
	printf("Enemy list\r\n");
	editusers(enemy, &enemyp, sizeof enemy / 20, "enemy");
	break;

      case 'm':
      case 'M':
	printf("Macros\r\n");
	for (; c != 'q';)
	{
	  colorize("\r\nw<rEw>gdit w<rLw>gist w-> ");
	  for (invalid = 0;;)
	  {
	    c = inkey();
	    if (!mystrchr("EeLlQq \n", c))
	    {
	      if (invalid++)
		flush_input(invalid);
	      continue;
	    }
	    break;
	  }
	  switch (c)
	  {
	    case 'e':
	    case 'E':
	      printf("Edit\r\n");
	      for (;;)
	      {
		printf("\r\nMacro to edit (%s to end) -> ", strctrl(macrokey));
		c = newkey(-1);
		if (c == macrokey || (c > '0' && c < '9'))
		  break;
		printf("%s\r\n", strctrl(c));
		newmacro(c);
	      }
	      printf("Done\r\n");
	      break;

	    case 'l':
	    case 'L':
	      printf("List\r\n\n");
	      for (i = 0, lines = 1; i < 128; i++)
		if (*macro[i])
		{
		  printf("'%s': \"", strctrl(i));
		  for (j = 0; macro[i][j]; j++)
		    printf("%s", strctrl(macro[i][j]));
		  printf("\"\r\n");
		  if (++lines == rows - 1 && more(&lines, -1) < 0)
		    break;
		}
	      break;

	    case 'q':
	    case 'Q':
	    case ' ':
	    case '\n':
	      printf("Quit\r\n");
	      c = 'q';
	      break;
	  }
	}
	break;

	/*
	 * KHAddition
	 */

      case 'o':
      case 'O':
        printf(" Options.\n");
	for (;;)
	{
	  putchar ('\n');
	  sprintf(colst, "  \1w(\1r%c\1w) \1y1.\1g Suppress logout warnings\n", 
	          (userflags & US_NOWARNING)?'*':' ');
	  colorize(colst);
	  sprintf(colst, "  \1w(\1r%c\1w) \1y2.\1g Have ANSI colours switched on at startup\n", 
	          (userflags & US_ANSIDEFAULT)?'*':' ');
	  colorize(colst);
	  sprintf(colst, "  \1w(\1r%c\1w) \1y3.\1g Use new edit routine (NOT FINISHED YET!)\n", 
	          (userflags & US_NEW_EDIT)?'*':' ');
	  colorize(colst);
	  colorize("\nOption \1w->  ");
	  do
	    c = inkey();
	  while (mystrchr("123\r\nQ ", c) == NULL);
	  if(mystrchr("\r\nQ ", c) != NULL)	/* quit options */
	    break;
	  sprintf (colst, "Toggling option \1y#%c\n", c);
	  colorize(colst);
	  switch (c)
	  {
	    case '1':
	      userflags = (userflags ^ US_NOWARNING);
	      break;
	    case '2':
	      userflags = (userflags ^ US_ANSIDEFAULT);
	      break;
	    case '3':
	      userflags = (userflags ^ US_NEW_EDIT);
	      break;
	  }
	}
	break;

      case 'Q':
	printf("Abort\nLeaving config unchanged.\n");
	configflag = 0;
        return;

      case 'q':
	printf("Quit\nSave changes (y/n)? ");
	if(yesno() == NO)
	{
	  printf("\nLeaving config unchanged.\n");
	  configflag = 0;
	  return;
	}
      /* else fall through */

      case 'S':
      case 's':
      case ' ':
      case '\n':
	if(c != 'q')
	  printf("Quit");
	printf("\nSaving configuration.\r\n");
	configflag = 0;
	if (bbsrcro || !bbsrc)
	  return;

	save_bbsrc();
	return;

      default:
	break;
    }
  }
}



/*
 * Gets a new hotkey value or returns the old value if the default is taken. If
 * the old value is specified as -1, no checking is done to see if the new
 * value doesn't conflict with other hotkeys.  Calls getkey() instead of
 * inkey() to avoid the character translation (since the hotkey values are
 * checked within inkey() instead of getkey()) 
 */
int
newkey(oldkey)
  int     oldkey;
{
int     c;

  for (;;)
  {
    c = getkey();
    if (((c == '\n' || c == '\r') && oldkey >= 0) || c == oldkey)
      return (oldkey);
    if (oldkey >= 0 && (c == macrokey || c == suspkey || c == quitkey || c == shellkey || c == capturekey || c == connectkey))
      printf("\r\nThat key is already in use for another hotkey, try again -> ");
    else if (c == ' ')
      printf("\r\nYou cannot use space, try again -> ");
    else
      return (c);
  }
}



/*
 * Gets a new value for macro 'which'. 
 */
void
newmacro(which)
  int     which;
{
register int i;
register int c;

  if (*macro[which])
  {
    printf("\r\nCurrent macro for '%s' is: \"", strctrl(which));
    for (i = 0; macro[which][i]; i++)
      printf("%s", strctrl(macro[which][i]));
    printf("\"\r\nDo you wish to change this? (Y/N) -> ");
  }
  else
    printf("\r\nNo current macro for '%s'.\r\nDo you want to make one? (Y/N) -> ", strctrl(which));
  if (!yesno())
    return;
  printf("\r\nEnter new macro (use %s to end)\r\n -> ", strctrl(macrokey));
  for (i = 0;; i++)
  {
    c = inkey();
    if (c == '\b')
    {
      if (i)
      {
	if (macro[which][i - 1] < ' ')
	  printf("\b \b");
	i--;
	printf("\b \b");
      }
      i--;
      continue;
    }
    if (c == macrokey)
    {
      macro[which][i] = 0;
      printf("\r\n");
      return;
    }
    else if (i == 70)
    {
      i--;
      continue;
    }
    printf("%s", strctrl(macro[which][i] = c));
  }
}



/*
 * Returns a string representation of c suitable for printing.  If c is a
 * regular character it will be printed normally, if it is a control character
 * it is printed as in the Unix ctlecho mode (i.e. ctrl-A is printed as ^A) 
 */
char   *
strctrl(c)
  int     c;
{
static char ret[3];

  if (c <= 31 || c == DEL)
  {
    ret[0] = '^';
    ret[1] = c == 10 ? 'M' : c ^ 0x40;
  }
  else
  {
    ret[0] = c;
    ret[1] = 0;
  }
  ret[2] = 0;
  return (ret);
}



/*
 * Does the editing of the friend and enemy lists. 
 */
void
editusers(list, total, size, name)
  char    list[][20];
  int    *total;
  int     size;
  char   *name;
{
register int c;
register int i;
register int j;
int     invalid = 0;
int     lines;
char   *sp;

  for (;;)
  {
    colorize("\r\nw<rAw>gdd w<rDw>gelete w<rLw>gist w-> ");
    c = inkey();
    switch (c)
    {
      case 'a':
      case 'A':
	printf("Add\r\n");
	if (*total == size)
	  printf("\r\nSorry, your %s list is full!\r\n\n", name);
	else
	{
	  printf("\r\nUser to add to your %s list -> ", name);
/*
	  sp = get_name(-999);
*/
	  sp = get_name(2);
	  if (*sp)
	  {
	    for (i = 0; i < *total; i++)
	      if (!strcmp(list[i], sp))
	      {
		printf("\r\n%s is already on your %s list.\r\n", sp, name);
		i = -1;
		break;
	      }
	    if (i < 0)
	      break;
	    strcpy(list[i], sp);
	    (*total)++;
	    printf("\r\n%s was added to your %s list.\r\n", sp, name);
	  }
	}
	break;

      case 'd':
      case 'D':
	printf("Delete\r\n\nUser to delete from your %s list -> ", name);
	sp = get_name(-999);
	if (*sp)
	{
	  for (i = 0; i < *total; i++)
	    if (!strcmp(list[i], sp))
	    {
	      for (j = i; j < *total - 1; j++)
		strcpy(list[j], list[j + 1]);
	      (*total)--;
	      printf("\r\n%s was deleted from your %s list.\r\n", sp, name);
	      i = -1;
	      break;
	    }
	  if (i < 0)
	    break;
	  printf("\r\n%s is not in your %s list.\r\n", sp, name);
	}
	break;

      case 'l':
      case 'L':
	printf("List\r\n\n");
	for (i = 0, lines = 1; i < *total; i++)
	{
	  printf("%-19s%s", list[i], (i % 4) == 3 ? "\r\n" : " ");
	  if ((i % 4) == 3)
	    lines++;
	  if (lines == rows - 1 && more(&lines, -1) < 0)
	    break;
	}
	if (i % 4)
	  printf("\r\n");
	break;

      case 'q':
      case 'Q':
      case '\n':
      case ' ':
	printf("Quit\r\n");
	return;

      default:
	if (invalid++)
	  flush_input(invalid);
	continue;
    }
    invalid = 0;
  }
}
