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

    upstst_ugo_env - tests upsugo_env

   ==========================================================================*/

int upstst_ugo_env (int argc, char ** const argv)
{
static char     *funcname = "upsugo_env";
int             status;                         /* status of parse */
int             estatus = UPS_SUCCESS;          /* expected status */
static char     *estatus_str;                   /* expected status string */
static char     *product;			/* product name */
static char     *options;			/* options */
static char	*outfile;			/* filename to output */
static char     *difffile;			/* file to diff */
FILE		*ofd;				/* outfile file descriptor */
int		stdout_dup;			/* dup of stdout */
upstst_argt     argt[] = {{"<product>", UPSTST_ARGV_STRING,NULL,&product},
			  {"-options",  UPSTST_ARGV_STRING,NULL,&options},
                          {"-status",   UPSTST_ARGV_STRING,NULL,&estatus_str},
                          {"-out",    UPSTST_ARGV_STRING,NULL,&outfile},
                          {"-diff",   UPSTST_ARGV_STRING,NULL,&difffile},
                          {NULL,        UPSTST_ARGV_END,   NULL,NULL}};
t_upsugo_command *uc;				/* ups command */
char            diffcmd[132];                   /* diff command */


/* parse command line
   ------------------ */

estatus_str = NULL; options = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);
if (!options) options = UPSTST_ALLOPTS;

/* let's get our output file descriptor setup
   ------------------------------------------ */

if (outfile) 					/* don't use stdout */
   {
   if (!(ofd = fopen(outfile,"w")))
      { perror(outfile); return (1); }
   }
else
   {ofd = stdout;}
stdout_dup = dup(STDOUT_FILENO);                /* dup stdout */
(void) fflush(stdout);                          /* clear output buffer */
status = dup2(fileno(ofd),STDOUT_FILENO);	/* reset it to output file */
 
/* call the real routine
   --------------------- */

uc = upsugo_env(product,options);
UPSTST_CHECK_CALL(UPSTST_NONZEROSUCCESS,uc,estatus);
(void) upsugo_dump(uc,FALSE);			/* display */

/* dump the output to specified file and compare
   --------------------------------------------- */

(void) fflush(stdout);				/* flush buffer */
(void) dup2(stdout_dup,STDOUT_FILENO);		/* reset stdout */
(void) close(stdout_dup);			/* close files*/
if(fileno(ofd) != STDOUT_FILENO) (void) fclose(ofd);	

if (difffile && outfile)
   {
   (void) sprintf (diffcmd,"diff %s %s",difffile,outfile);
   if (system(diffcmd)) (void) printf("files %s %s differ\n",difffile,outfile);
   }
return (0);
}

/* ==========================================================================

    upstst_ugo_next - tests upsugo_next

   ==========================================================================*/
int upstst_ugo_next (int argc, char ** const argv)
{
static char     *funcname = "upsugo_next";
int             status;                         /* status of parse */
int             estatus = UPS_SUCCESS;          /* expected status */
static char     *estatus_str;                   /* expected status string */
static char     *options;			/* options */
static int	encmds;				/* expected count */
static char	*outfile;			/* filename to output */
static char     *difffile;			/* file to diff */
FILE		*ofd;				/* outfile file descriptor */
int		stdout_dup;			/* dup of stdout */
upstst_argt     argt[] = {{"-options",UPSTST_ARGV_STRING,NULL,&options},
			  {"-encmds", UPSTST_ARGV_INT,   NULL,&encmds},
                          {"-status", UPSTST_ARGV_STRING,NULL,&estatus_str},
                          {"-out",    UPSTST_ARGV_STRING,NULL,&outfile},
                          {"-diff",   UPSTST_ARGV_STRING,NULL,&difffile},
                          {NULL,      UPSTST_ARGV_END,   NULL,NULL}};
t_upsugo_command *uc =0;			/* ups command */
int		ncmds = 0;
char            diffcmd[132];                   /* diff command */


/* parse command line
   ------------------ */

estatus_str = NULL; encmds = 1; outfile = NULL; difffile = NULL;
options = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_EXACTMATCH);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);
if (!options) options = UPSTST_ALLOPTS;

/* let's get our output file descriptor setup
   ------------------------------------------ */

if (outfile) 					/* don't use stdout */
   {
   if (!(ofd = fopen(outfile,"w")))
      { perror(outfile); return (1); }
   }
else
   {ofd = stdout;}
stdout_dup = dup(STDOUT_FILENO);                /* dup stdout */
(void) fflush(stdout);                          /* clear output buffer */
status = dup2(fileno(ofd),STDOUT_FILENO);	/* reset it to output file */


/* call the real routine
   --------------------- */

UPS_ERROR = UPS_SUCCESS;
while ((uc = upsugo_next(argc,argv,options)) != 0)	/* for all commands */
   {
   UPSTST_CHECK_UPS_ERROR(estatus);		/* check UPS_ERROR */
   (void) upsugo_dump(uc,FALSE);		/* display */
   ncmds++;					/* increment ncommands */
   }

if (ncmds != encmds)
   (void) fprintf(stderr,"%s: %d, %s: %d\n","actual ncmds",ncmds, 
      "expected ncmds", encmds);    

/* dump the output to specified file and compare
   --------------------------------------------- */

(void) fflush(stdout);				/* flush buffer */
(void) dup2(stdout_dup,STDOUT_FILENO);		/* reset stdout */
(void) close(stdout_dup);			/* close files*/
if(fileno(ofd) != STDOUT_FILENO) (void) fclose(ofd);	

if (difffile && outfile)
   {
   (void) sprintf (diffcmd,"diff %s %s",difffile,outfile);
   if (system(diffcmd)) (void) printf("files %s %s differ\n",difffile,outfile);
   }

return (0);
}

/* ==========================================================================

    upstst_ugo_bldcmd - tests upsugo_bldcmd

   ==========================================================================*/

int upstst_ugo_bldcmd (int argc, char ** const argv)
{
static char     *funcname = "upsugo_bldcmd";
int             status;                         /* status of parse */
int             estatus = UPS_SUCCESS;          /* expected status */
static char     *estatus_str;                   /* expected status string */
static char     *command;			/* command string name */
static char     *options;			/* options */
static char	*outfile;			/* filename to output */
static char     *difffile;			/* file to diff */
FILE		*ofd;				/* outfile file descriptor */
int		stdout_dup;			/* dup of stdout */
upstst_argt     argt[] = {{"<command>", UPSTST_ARGV_STRING,NULL,&command},
			  {"-options",  UPSTST_ARGV_STRING,NULL,&options},
                          {"-status",   UPSTST_ARGV_STRING,NULL,&estatus_str},
                          {"-out",    UPSTST_ARGV_STRING,NULL,&outfile},
                          {"-diff",   UPSTST_ARGV_STRING,NULL,&difffile},
                          {NULL,        UPSTST_ARGV_END,   NULL,NULL}};
t_upsugo_command *uc;				/* ups command */
char            diffcmd[132];                   /* diff command */


/* parse command line
   ------------------ */

estatus_str = NULL; options = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);
if (!options) options = UPSTST_ALLOPTS;

/* let's get our output file descriptor setup
   ------------------------------------------ */

if (outfile) 					/* don't use stdout */
   {
   if (!(ofd = fopen(outfile,"w")))
      { perror(outfile); return (1); }
   }
else
   {ofd = stdout;}
stdout_dup = dup(STDOUT_FILENO);                /* dup stdout */
(void) fflush(stdout);                          /* clear output buffer */
status = dup2(fileno(ofd),STDOUT_FILENO);	/* reset it to output file */
 
/* call the real routine
   --------------------- */

uc = upsugo_bldcmd(command,options);
UPSTST_CHECK_CALL(UPSTST_NONZEROSUCCESS,uc,estatus);
(void) upsugo_dump(uc,FALSE);			/* display */

/* dump the output to specified file and compare
   --------------------------------------------- */

(void) fflush(stdout);				/* flush buffer */
(void) dup2(stdout_dup,STDOUT_FILENO);		/* reset stdout */
(void) close(stdout_dup);			/* close files*/
if(fileno(ofd) != STDOUT_FILENO) (void) fclose(ofd);	

if (difffile && outfile)
   {
   (void) sprintf (diffcmd,"diff %s %s",difffile,outfile);
   if (system(diffcmd)) (void) printf("files %s %s differ\n",difffile,outfile);
   }
return (0);
}
