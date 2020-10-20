/*

Authors:        David J. Fagan stolen from Margaret Votava
e-mail:         "votava@fnal.gov"

Include files:-
===============
*/

#include <unistd.h>

/* ups specific include files */
#include "ups.h"
#include "upstst_parse.h"
#include "upstst_macros.h"

static void upstst_trans_dump (const t_upslst_item * const , 
                               const t_upsugo_command * const);
/* ==========================================================================

    upstst_get_translation - tests upsget_translation

   ==========================================================================*/

int upstst_get_translation (int argc, char ** const argv)
{
static char     	*funcname = "upsget_translation";
int             	status;                         /* status of parse */
int             	estatus = UPS_SUCCESS;          /* expected status */
t_upsugo_command	*uc = 0;			/* ups command */
t_upslst_item		*mp = 0;			/* match product */
char			diffcmd[132];			/* diff command */

static char     	*estatus_str;                   /*expected status str */
static char		*outfile;			/* filename to output */
static char		*difffile;			/* file to diff */
FILE			*ofd;				/* outfile fd */
int			stdout_dup;			/* dup of stdout */
upstst_argt     	argt[] = 
    {{"-out",	  UPSTST_ARGV_STRING,NULL,		&outfile},
    {"-diff",     UPSTST_ARGV_STRING,NULL,		&difffile},
    {"-status",   UPSTST_ARGV_STRING,NULL,		&estatus_str},
    {NULL,        UPSTST_ARGV_END,NULL,			NULL}};



/* parse command line
   ------------------ */

estatus_str = NULL; outfile = NULL; difffile = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_EXACTMATCH);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);

/* let's get our output file descriptor setup
   ------------------------------------------ */

if (outfile)                                    /* don't use stdout */
   {
   if (!(ofd = fopen(outfile,"w")))
      { perror(outfile); return (1); }
   }
else
   {ofd = stdout;}
stdout_dup = dup(STDOUT_FILENO);                /* dup stdout */
(void) fflush(stdout);                                 /* clear output buffer */
status = dup2(fileno(ofd),STDOUT_FILENO);       /* reset it to output file */

/* call the real routine
   --------------------- */

UPS_ERROR = UPS_SUCCESS;
while ((uc = upsugo_next(argc,argv,UPSTST_ALLOPTS)) != 0)	/* for all commands */
   {
   if (UPS_ERROR != UPS_SUCCESS)			/* error on ugo_next */
       {
       (void) fprintf(stderr,"upsugo_next failed: %s\n", g_error_ascii[UPS_ERROR]);  
       upserr_output(); upserr_clear();
       return (0);
       }
   mp = upsmat_instance(uc,NULL,TRUE);
   UPSTST_CHECK_UPS_ERROR(estatus);
   upstst_trans_dump(mp,uc);
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

    upstst_trans_dump - static function to display contents of match buffer

   ==========================================================================*/

static void upstst_trans_dump (const t_upslst_item * const mp, 
   const t_upsugo_command * const uc)
{
t_upslst_item 			*prod_ptr;		/* product ptr */
t_upstyp_matched_product 	*prod;			/* product match */
static char 			*tostring;
/* I used lp since I knew the account at least existed on all systems 
   but location may be a problem.  root I guess could be used but that
   would effectively return nothing to tell if it really worked 
   your call marge...                                                   */
static char 			*string = 
	 "Name         = ${UPS_PROD_NAME}\
	\nTilde        = ~lp/dir/filename\
	\nVersion      = ${UPS_PROD_VERSION}\
	\nFlavor       = ${UPS_PROD_FLAVOR}\
	\nQualifiers   = ${UPS_PROD_QUALIFIERS}\
	\nProd dir     = ${UPS_PROD_DIR}\
	\nShell        = ${UPS_SHELL}\
	\nOptions      = ${UPS_OPTIONS}\
	\nVerbose      = ${UPS_VERBOSE}\
	\nExtended     = ${UPS_EXTENDED}\
	\nThis Db      = ${UPS_THIS_DB}\
	\nSetup        = ${UPS_SETUP}\
	\nOrigin       = ${UPS_ORIGIN}\
	\nCompile      = ${UPS_COMPILE}\
	\nUps_dir      = ${UPS_UPS_DIR}\
	\nPRODUCTS     = ${PRODUCTS}\
	\nJunk         = ${JUNK}\
	";

					

if(!mp) return;
for (prod_ptr = (t_upslst_item *)mp; prod_ptr; prod_ptr = prod_ptr->next)
   {
   prod = (t_upstyp_matched_product *) prod_ptr->data;
   tostring=upsget_translation(
			  (t_upstyp_matched_instance *)prod->minst_list->data,
			  prod->db_info,uc,string);
   if (tostring)
      (void) printf("%s\n",tostring); 
   else 
      (void) printf("String >%s< NOT Converted\n",string);
   }
}
/* ==========================================================================

    upstst_get_allout - tests upsget_allout

   ==========================================================================*/

int upstst_get_allout (int argc, char ** const argv)
{
static char     	*funcname = "upsget_allout";
int             	status;                         /* status of parse */
int             	estatus = UPS_SUCCESS;          /* expected status */
t_upsugo_command	*uc = 0;			/* ups command */
t_upslst_item		*mp = 0;			/* match product */
char			diffcmd[132];			/* diff command */
t_upstyp_matched_product        *prod;                  /* product match */
t_upstyp_matched_instance       *inst;                  /* instance match */

static char     	*estatus_str;                   /*expected status str */
static char		*outfile;			/* filename to output */
static char		*difffile;			/* file to diff */
FILE			*ofd;				/* outfile fd */
int			stdout_dup;			/* dup of stdout */
upstst_argt     	argt[] = 
    {{"-out",	  UPSTST_ARGV_STRING,NULL,		&outfile},
    {"-diff",     UPSTST_ARGV_STRING,NULL,		&difffile},
    {"-status",   UPSTST_ARGV_STRING,NULL,		&estatus_str},
    {NULL,        UPSTST_ARGV_END,NULL,			NULL}};



/* parse command line
   ------------------ */

estatus_str = NULL; outfile = NULL; difffile = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_EXACTMATCH);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);

/* let's get our output file descriptor setup
   ------------------------------------------ */

if (outfile)                                    /* don't use stdout */
   {
   if (!(ofd = fopen(outfile,"w")))
      { perror(outfile); return (1); }
   }
else
   {ofd = stdout;}
stdout_dup = dup(STDOUT_FILENO);                /* dup stdout */
(void) fflush(stdout);                                 /* clear output buffer */
status = dup2(fileno(ofd),STDOUT_FILENO);       /* reset it to output file */

/* call the real routine
   --------------------- */

UPS_ERROR = UPS_SUCCESS;
while ((uc = upsugo_next(argc,argv,UPSTST_ALLOPTS)) != 0)	/* for all commands */
   {
   if (UPS_ERROR != UPS_SUCCESS)			/* error on ugo_next */
       {
       (void) fprintf(stderr,"upsugo_next failed: %s\n", g_error_ascii[UPS_ERROR]);  
       upserr_output(); upserr_clear();
       return (0);
       }
   mp = upsmat_instance(uc,NULL,TRUE);
   UPSTST_CHECK_UPS_ERROR(estatus);
   prod = (t_upstyp_matched_product *) mp->data;
   inst = (t_upstyp_matched_instance *)prod->minst_list->data;
   upsget_allout(stdout, prod->db_info, inst, uc, "");
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
