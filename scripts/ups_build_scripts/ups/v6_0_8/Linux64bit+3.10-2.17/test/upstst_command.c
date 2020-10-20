/*

Authors:        Margaret Votava
e-mail:         "votava@fnal.gov"

Include files:-
===============
*/

#include <unistd.h>
extern char *g_temp_file_name;

/* ups specific include files */
#include "ups.h"
#include "upstst_parse.h"
#include "upstst_macros.h"

/* ==========================================================================

    upstst_commands - tests ups all the commands with the generic interface

   ==========================================================================*/


int upstst_command(int argc, char ** const argv, const void * myfunc(),const char * const funcname, const int calledby)
{
static char	*f_stdout;			/* filename to output */
static char     *f_stdout_diff;			/* file to diff */
static char	*f_action;			/* filename to output actions */
static char     *f_action_diff;			/* file to diff */
static char	*estatus_str;                   /*expected status str */
int		estatus = UPS_SUCCESS;          /* expected status */
int		status;				/* function status */
FILE		*ofd;				/* f_stdout file descriptor */
FILE		*afd;				/* action file descriptor */
FILE		*fufd = (FILE *)1;		/* finish up file descriptor */
upstst_argt     argt[] = {{"-out",     UPSTST_ARGV_STRING,NULL,&f_stdout},
                          {"-diff",    UPSTST_ARGV_STRING,NULL,&f_stdout_diff},
                          {"-actout",  UPSTST_ARGV_STRING,NULL,&f_action},
                          {"-actdiff", UPSTST_ARGV_STRING,NULL,&f_action_diff},
                          {"-status",  UPSTST_ARGV_STRING,NULL,&estatus_str},
                          {NULL,       UPSTST_ARGV_END,   NULL,NULL}};
t_upsugo_command	*uc =0;			/* ups command */
char            diffcmd[132];                   /* diff command */
int		stdout_dup;			/* dup of stdout */
int		had_error;

/* parse command line
   ------------------ */

f_action = f_action_diff = f_stdout = f_stdout_diff = estatus_str = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_EXACTMATCH);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);
if (f_stdout) 					/* don't use stdout */
   {
   if (!(ofd = fopen(f_stdout,"w")))		/* open file */
      { perror("f_stdout"); return (1); }
   }
else
   {ofd = stdout;}

if (!f_action)
   {
   f_action = "/dev/null";
   fufd = NULL;
   }

if (!(afd = fopen(f_action,"w")))
      { perror("f_action"); return (1); }
if (fufd) fufd = afd;
g_temp_file_name = f_action; 


stdout_dup = dup(STDOUT_FILENO);		/* dup stdout */
(void) fflush(stdout);					/* clear output buffer */
(void) dup2(fileno(ofd),STDOUT_FILENO);		/* reset it to output file */

/* call the real routine
   --------------------- */

UPS_ERROR = UPS_SUCCESS;
while ((uc = upsugo_next(argc,argv,UPSTST_ALLOPTS)) != 0) /* for all commands */
   {
   if (UPS_ERROR != UPS_SUCCESS)
      {
      (void) fprintf(stderr,"upsugo_next failed: %s\n", g_error_ascii[UPS_ERROR]);
      upserr_output(); upserr_clear();
      return (0);
      }
   if (calledby == e_list)
      (void) (*myfunc)(uc,0);
   else
      (void) (*myfunc)(uc,afd,calledby);
   had_error = UPS_ERROR;
   if (UPS_ERROR != estatus)
   { (void) fprintf(stderr,"function: %s\n",funcname);       
     (void) fprintf(stderr,"%s: %s, %s: %s\n","actual status",
             g_error_ascii[UPS_ERROR],"expected status",    
             g_error_ascii[estatus]);                       
     /* if (UPS_ERROR) upserr_output(); DjF */ 
   } else { /* it's expected so clear it DjF */
     upserr_clear();
   }
/* last command COULD have been sucessful but still have errors DjF */
   upserr_output(); 
   upserr_clear();
   if (had_error) continue;
   (void) upsutl_finish_up(fufd,uc->ugo_shell,calledby,FALSE);
   UPSTST_CHECK_UPS_ERROR(estatus);		/* check UPS_ERROR */
   }

/* dump the output to specified file and compare
   --------------------------------------------- */

(void) fflush(stdout);                                 /* flush buffer */
(void) dup2(stdout_dup,STDOUT_FILENO);                 /* reset stdout */
(void) close(stdout_dup);                              /* close files*/
if(fileno(ofd) != STDOUT_FILENO) (void) fclose(ofd);

if (f_stdout_diff && f_stdout)
   {
   (void) sprintf (diffcmd,"diff %s %s",f_stdout_diff,f_stdout);
   if (system(diffcmd)) (void) printf("files %s %s differ\n",f_stdout_diff,f_stdout);
   }

if (f_action_diff && f_action)
   {
   (void) sprintf (diffcmd,"diff %s %s",f_action_diff,f_action);
   if (system(diffcmd)) (void) printf("files %s %s differ\n",f_action_diff,f_action);
   }
return (0);
}

int upstst_declare(int argc, char ** const argv)
{ return(upstst_command(argc,argv, 
          (const void * (*)(void)) &ups_declare,"ups_declare",e_declare)); }

int upstst_undeclare(int argc, char ** const argv)
{ return(upstst_command(argc,argv, 
          (const void * (*)(void)) &ups_undeclare,"ups_undeclare",e_undeclare)); }

int upstst_configure(int argc, char ** const argv)
{ return(upstst_command(argc,argv, 
          (const void * (*)(void)) &ups_configure,"ups_configure",e_configure)); }

int upstst_unconfigure(int argc, char ** const argv)
{ return(upstst_command(argc,argv, 
          (const void * (*)(void)) &ups_unconfigure,"ups_unconfigure",e_unconfigure)); }

int upstst_tailor(int argc, char ** const argv)
{ return(upstst_command(argc,argv,(const void * (*)(void)) &ups_tailor,"ups_tailor",e_tailor)); }

int upstst_copy(int argc, char ** const argv)
{ return(upstst_command(argc,argv,(const void * (*)(void)) &ups_copy,"ups_copy",e_copy)); }

int upstst_start(int argc, char ** const argv)
{ return(upstst_command(argc,argv,(const void * (*)(void)) &ups_start,"ups_start",e_start)); }

int upstst_stop(int argc, char ** const argv)
{ return(upstst_command(argc,argv, (const void * (*)(void)) &ups_stop,"ups_stop",e_stop)); }

int upstst_flavor(int argc, char ** const argv)
{ return(upstst_command(argc,argv,(const void * (*)(void)) &ups_flavor,"ups_flavor",e_flavor)); }

int upstst_get(int argc, char ** const argv)
{ return (upstst_command(argc,argv,(const void * (*)(void)) &ups_get,"ups_get",e_get)); }

int upstst_modify(int argc, char ** const argv)
{ return(upstst_command(argc,argv,(const void * (*)(void)) &ups_modify,"ups_modify",e_modify)); }

int upstst_setup(int argc, char ** const argv)
{ return(upstst_command(argc,argv,(const void * (*)(void)) &ups_setup,"ups_setup",e_setup));}

int upstst_unsetup(int argc, char ** const argv)
{ return(upstst_command(argc,argv, 
          (const void * (*)(void)) &ups_unsetup,"ups_unsetup",e_unsetup)); }

int upstst_unk(int argc, char ** const argv)
{ return (upstst_command(argc,argv, (const void * (*)(void)) &ups_unk,"ups_unk",e_unk)); }

int upstst_depend(int argc, char ** const argv)
{ return(upstst_command(argc,argv,(const void * (*)(void)) &ups_depend,"ups_depend",e_depend)); }

int upstst_touch(int argc, char ** const argv)
{ return(upstst_command(argc,argv,(const void * (*)(void)) &ups_touch,"ups_touch",e_touch)); }

int upstst_list(int argc, char ** const argv)
{ return(upstst_command(argc,argv,(const void * (*)(void)) &ups_list,"ups_list",e_list)); }
