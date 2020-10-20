/* 

Authors:        Margaret Votava
e-mail:         "votava@fnal.gov"
 
Revision history:-
=================
17-Oct-1995 MEV created 
 
*/

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
 
#include "upstst_cmdline.h"
#include "upstst_parse.h"
#include "upstst_cmdtable.h"

/*=============================================================================
ROUTINE:

upstst_cmdproc

   process command line

RETURNS:

   -1 if time to exit
    0 success
    1 command failed, but still continue processing next command

==============================================================================*/

int upstst_cmdproc	(char * const cmdbuf, upstst_cmd_table_t * const cmdlist)
{
int			myargc;			/* argc */
char			**myargv;		/* argv */
upstst_cmd_table_t	*cmd;			/* command and function ptr */
upstst_cmd_table_t	*match = NULL;		/* command and function ptr */
char 			*usrcmd;		/* command name */
unsigned int		cmd_length;		/* command length */

if ( (!strcmp(cmdbuf, "exit")) |
     (!strcmp(cmdbuf, "quit")) |
     (!strcmp(cmdbuf, "logout")) )
     return -1;						/* time to exit */

if (cmdbuf[0] == '#')  return 0;			/* comment, return */
   
if (upstst_split (cmdbuf, &myargc, &myargv))		/* split command */ 
   {
   (void) fprintf (stderr, "could not split line \n");
   return (1);
   }

/* find matching command 
   --------------------- */

usrcmd = myargv[0];					/* cmd name */
if (!usrcmd) return 0;					/* no cmd */

cmd_length = strlen(usrcmd);				/* length of cmd */
for (cmd = cmdlist; cmd->cmdname != NULL; cmd++)	/* look thru all cmds */
   {
   if (!strncmp(usrcmd,cmd->cmdname,cmd_length))	/* found it? */
      {
      if (match) 					/* already found one */
         {
         (void) fprintf (stderr, "command %s is not unique \n",
	     usrcmd);					/* print error */
	 return (1);					/* return */
	 }
      match = cmd;					/* remember */
      if (cmd->cmdname[cmd_length] == 0) break;   	/* exact match */
      }
   }

if (!match)
   {
   (void) fprintf (stderr, "command %s not found \n",usrcmd);	/* print error */
   return (1);						/* return */
   } 

return (match->func(myargc,myargv));
}

