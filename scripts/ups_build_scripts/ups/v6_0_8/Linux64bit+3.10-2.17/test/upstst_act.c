/*

Authors:        Margaret Votava
e-mail:         "votava@fnal.gov"

Include files:-
===============
*/

#include <unistd.h>

/* ups specific include files */
#include "ups.h"
#include "upstst_parse.h"
#include "upstst_macros.h"

/* ==========================================================================

    upstst_act_print - tests ups list 

   ==========================================================================*/

int upstst_act_print (int argc, char ** const argv)
{
static char     *funcname = "upsact_print";
static char     *options;			/* options */
static char	*outfile;			/* filename to output */
static char     *difffile;			/* file to diff */
static char     *action;			/* file to diff */
int		status;				/* function status */
FILE		*ofd;				/* outfile file descriptor */
upstst_argt     argt[] = {{"-options",UPSTST_ARGV_STRING,NULL,&options},
                          {"-out",    UPSTST_ARGV_STRING,NULL,&outfile},
                          {"-diff",   UPSTST_ARGV_STRING,NULL,&difffile},
                          {"<action>", UPSTST_ARGV_STRING,NULL,&action},
                          {NULL,      UPSTST_ARGV_END,   NULL,NULL}};
t_upsugo_command	*uc =0;			/* ups command */
char            diffcmd[132];                   /* diff command */
int		stdout_dup;			/* dup of stdout */

/* parse command line
   ------------------ */

outfile = NULL; difffile = NULL; options = NULL; action = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_EXACTMATCH);
UPSTST_CHECK_PARSE(status,argt,argv[0]);

if (!options) options = "tal";
if (outfile) 					/* don't use stdout */
   {
   if (!(ofd = fopen(outfile,"w")))		/* open file */
      { perror(outfile); return (1); }
   }
else
   {ofd = stdout;}

stdout_dup = dup(STDOUT_FILENO);		/* dup stdout */
(void) fflush(stdout);					/* clear output buffer */
(void) dup2(fileno(ofd),STDOUT_FILENO);		/* reset it to output file */

/* call the real routine
   --------------------- */

UPS_ERROR = UPS_SUCCESS;
while ((uc = upsugo_next(argc,argv,UPSTST_ALLOPTS)) != 0)	/* for all commands */
   {
   UPSTST_CHECK_UPS_ERROR(UPS_SUCCESS);		/* check UPS_ERROR */
   (void) upsact_print(uc,NULL,action,upsact_action2enum(action),options);
   UPSTST_CHECK_UPS_ERROR(UPS_SUCCESS);		/* check UPS_ERROR */
   }

/* dump the output to specified file and compare
   --------------------------------------------- */

(void) fflush(stdout);                                 /* flush buffer */
(void) dup2(stdout_dup,STDOUT_FILENO);                 /* reset stdout */
(void) close(stdout_dup);                              /* close files*/
if(fileno(ofd) != STDOUT_FILENO) (void) fclose(ofd);

if (difffile && outfile)
   {
   (void) sprintf (diffcmd,"diff %s %s",difffile,outfile);
   if (system(diffcmd)) (void) printf("files %s %s differ\n",difffile,outfile);
   }

return (0);
}

/* ==========================================================================

    upstst_act_process_commands - tests ups list 

   ==========================================================================*/

int upstst_act_process_commands (int argc, char ** const argv)
{
static char     *funcname = "upsact_process_commands";
static char	*outfile;			/* filename to output */
static char     *difffile;			/* file to diff */
static char     *action;			/* file to diff */
static char	*estatus_str;                   /*expected status str */
int		status;				/* function status */
int 		estatus = UPS_SUCCESS;          /* expected status */
FILE		*ofd;				/* outfile file descriptor */
upstst_argt     argt[] = {{"-out",    UPSTST_ARGV_STRING,NULL,&outfile},
                          {"-diff",   UPSTST_ARGV_STRING,NULL,&difffile},
                          {"<action>",UPSTST_ARGV_STRING,NULL,&action},
    			  {"-status", UPSTST_ARGV_STRING,NULL,&estatus_str},

                          {NULL,      UPSTST_ARGV_END,   NULL,NULL}};
t_upsugo_command *uc =0;			/* ups command */
char            diffcmd[132];                   /* diff command */
int		stdout_dup;			/* dup of stdout */
t_upslst_item  *cmd_list;			/* list of commands */


/* parse command line
   ------------------ */

outfile = NULL; difffile = NULL; action = NULL; estatus_str = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_EXACTMATCH);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);

if (outfile) 					/* don't use stdout */
   {
   if (!(ofd = fopen(outfile,"w")))		/* open file */
      { perror(outfile); return (1); }
   }
else
   {ofd = stdout;}

stdout_dup = dup(STDOUT_FILENO);		/* dup stdout */
(void) fflush(stdout);					/* clear output buffer */
(void) dup2(fileno(ofd),STDOUT_FILENO);		/* reset it to output file */

/* call the real routine
   --------------------- */

UPS_ERROR = UPS_SUCCESS;
while ((uc = upsugo_next(argc,argv,UPSTST_ALLOPTS)) != 0)	/* for all commands */
   {
   UPSTST_CHECK_UPS_ERROR(UPS_SUCCESS);		/* check UPS_ERROR */
   cmd_list = upsact_get_cmd(uc,NULL,action,upsact_action2enum(action));
   if (UPS_ERROR != UPS_SUCCESS)		/* second one could be the error */
      UPSTST_CHECK_UPS_ERROR(estatus);		/* check UPS_ERROR */
   if (estatus != UPS_SUCCESS) continue;
   upsact_process_commands(cmd_list, ofd);
   UPSTST_CHECK_UPS_ERROR(estatus);		/* check UPS_ERROR */
   }

/* dump the output to specified file and compare
   --------------------------------------------- */

(void) fflush(stdout);                                 /* flush buffer */
(void) dup2(stdout_dup,STDOUT_FILENO);                 /* reset stdout */
(void) close(stdout_dup);                              /* close files*/
if(fileno(ofd) != STDOUT_FILENO) (void) fclose(ofd);

if (difffile && outfile)
   {
   (void) sprintf (diffcmd,"diff %s %s",difffile,outfile);
   if (system(diffcmd)) (void) printf("files %s %s differ\n",difffile,outfile);
   }

return (0);
}

