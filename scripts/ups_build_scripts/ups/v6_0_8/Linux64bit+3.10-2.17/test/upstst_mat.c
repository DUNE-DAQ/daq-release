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

static void upstst_match_dump (const t_upslst_item * const );
/* ==========================================================================

    upstst_mat_instance - tests upsmat_instace

   ==========================================================================*/

int upstst_mat_instance (int argc, char ** const argv)
{
static char     	*funcname = "upsmat_instance";
int             	status;                         /* status of parse */
int             	estatus = UPS_SUCCESS;          /* expected status */
t_upsugo_command	*uc = 0;			/* ups command */
t_upslst_item		*mp = 0;			/* match product */
char			diffcmd[132];			/* diff command */

static char     	*estatus_str;                   /*expected status str */
static int		unique;				/* unique flag name */
static char		*outfile;			/* filename to output */
static char		*difffile;			/* file to diff */
static char		*database;			/* ups database */
FILE			*ofd;				/* outfile fd */
int			stdout_dup;			/* dup of stdout */
t_upslst_item		*dblist = NULL;
t_upstyp_db		*dbaddr = NULL;

upstst_argt     	argt[] = 
   {{"-unique",	UPSTST_ARGV_CONSTANT,(char *)TRUE,	&unique},
    {"-out",	UPSTST_ARGV_STRING,NULL,		&outfile},
    {"-diff",   UPSTST_ARGV_STRING,NULL,		&difffile},
    {"-db", 	UPSTST_ARGV_STRING,NULL,		&database},
    {"-status", UPSTST_ARGV_STRING,NULL,   		&estatus_str},
    {NULL,      UPSTST_ARGV_END,NULL,			NULL}};


/* parse command line
   ------------------ */

estatus_str = NULL; 
unique = 0; outfile = NULL; difffile = NULL; database = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_EXACTMATCH);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);

if (database) 
   {
   dbaddr = upsmem_malloc(sizeof(t_upstyp_db));
   dbaddr->name = upsutl_str_create(database, STR_TRIM_DEFAULT);
   dbaddr->config= NULL;
   dblist = upslst_new(dbaddr);
   }
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
(void) fflush(stdout);                          /* clear output buffer */
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
   mp = upsmat_instance(uc,dblist,unique);
   if (UPS_ERROR != estatus)                    	/* error? */
       {
       (void) fprintf(stderr,"%s: %s, %s: %s\n","actual status",
          g_error_ascii[UPS_ERROR],"expected status", g_error_ascii[estatus]);
       if (UPS_ERROR) { upserr_output(); upserr_clear(); }
       }
   upstst_match_dump(mp);
/*   if (mp) upsutl_free_matched_instance_list(mp); */
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

if (database) upsmem_free(dbaddr);

return (0);

}

/* ==========================================================================

    upstst_match_dump - static function to display contents of match buffer

   ==========================================================================*/

static void upstst_match_dump (const t_upslst_item * const mp)
{
t_upslst_item 			*prod_ptr;		/* product ptr */
t_upslst_item 			*inst_ptr;		/* instance ptr */
t_upstyp_matched_product 	*prod;			/* product match */
t_upstyp_matched_instance	*inst;			/* instance match */
t_upstyp_db			*db;			/* database */


if(!mp) return;
for (prod_ptr = (t_upslst_item *)mp; prod_ptr; prod_ptr = prod_ptr->next)
   {
   prod = (t_upstyp_matched_product *) prod_ptr->data;
   db = (t_upstyp_db *) prod->db_info;
   if (db) (void) printf("D:NAME=%s\n", db->name);
   for (inst_ptr = prod->minst_list; inst_ptr; inst_ptr = inst_ptr->next)
      {
      inst = (t_upstyp_matched_instance *) inst_ptr->data;
      if (inst->chain)
         {
         (void) printf("C:PRODUCT=%15s, VERSION=%10s, ", inst->chain->product,
             inst->chain->version);
         (void) printf("FLAVOR=%12s, QUALIFIERS=%s, CHAIN=%s\n", inst->chain->flavor,
             inst->chain->qualifiers, inst->chain->chain);
         if (inst->xtra_chains)
	    {
	    t_upstyp_instance 	*cinst;
	    t_upslst_item 	*clist;
 	    for (clist = inst->xtra_chains; clist; clist=clist->next)
		{
		cinst = (t_upstyp_instance *)clist->data; 
                (void) printf("C:PRODUCT=%15s, VERSION=%10s, ", cinst->product,
                    cinst->version);
                (void) printf("FLAVOR=%12s, QUALIFIERS=%s, CHAIN=%s\n", 
		    cinst->flavor, cinst->qualifiers, cinst->chain);
		}
	    }
         }
      if (inst->version)
         {
         (void) printf("V:PRODUCT=%15s, VERSION=%10s, ", 
		inst->version->product ? inst->version->product : "(null)" ,
		inst->version->version ? inst->version->version : "(null)" );
         (void) printf("FLAVOR=%12s, QUALIFIERS=%s\n", inst->version->flavor,
             inst->version->qualifiers);
         }
      if (inst->table)
         {
         (void) printf("T:PRODUCT=%15s, VERSION=%10s, ", 
		inst->table->product ? inst->table->product : "(null)",
		inst->table->version ? inst->table->version : "(null)");
         (void) printf("FLAVOR=%12s, QUALIFIERS=%s\n", inst->table->flavor,
             inst->table->qualifiers);
         }
      }
   }
}
