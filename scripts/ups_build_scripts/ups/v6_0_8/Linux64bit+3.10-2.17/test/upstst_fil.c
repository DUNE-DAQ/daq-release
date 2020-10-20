/*

Authors:        Margaret Votava
e-mail:         "votava@fnal.gov"

Include files:-
===============
*/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* ups specific include files */
#include "ups.h"
#include "upstst_parse.h"
#include "upstst_macros.h"

/*
 * Definition of global variables
 */

t_upstyp_product *upstst_file = NULL;

int upstst_fil_read_file (int argc, char ** const argv)
{
static char     *funcname = "upsfil_read_file";
int             status;				/* status of parse */
int             estatus = UPS_SUCCESS; 		/* expected status */
static char     *estatus_str;			/* expected status string */
static char     *filename;			/* filename to read */
upstst_argt     argt[] = {{"<filename>",UPSTST_ARGV_STRING,NULL,&filename},
			  {"-status",   UPSTST_ARGV_STRING,NULL,&estatus_str},
                          {NULL,        UPSTST_ARGV_END,   NULL,NULL}};

/* parse command line
   ------------------ */

estatus_str = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);

upstst_file = upsfil_read_file(filename);
UPSTST_CHECK_UPS_ERROR(estatus);
return (0);
}

int upstst_fil_write_file (int argc, char ** const argv)
{
static char     *funcname = "upsfil_write_file";
int             status;				/* status of parse */
int             estatus = UPS_SUCCESS; 		/* expected status */
static char     *estatus_str;			/* expected status string */
static char     *filename;			/* filename to read */
upstst_argt     argt[] = {{"<filename>",UPSTST_ARGV_STRING,NULL,&filename},
			  {"-status",   UPSTST_ARGV_STRING,NULL,&estatus_str},
                          {NULL,        UPSTST_ARGV_END,   NULL,NULL}};

/* parse command line
   ------------------ */

estatus_str = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE(status,argt,argv[0]);
UPSTST_CHECK_ESTATUS (estatus_str, estatus);

status = upsfil_write_file(upstst_file,filename,' ',NOJOURNAL);
UPSTST_CHECK_CALL(UPSTST_ZEROSUCCESS,status,estatus);
return (0);
}

