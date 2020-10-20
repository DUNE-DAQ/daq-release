/*

Authors:        Margaret Votava
e-mail:         "votava@fnal.gov"
 
Include files:-
===============
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "upstst_parse.h"
#include "upstst_macros.h"


/* ============================================================================

ROUTINE:
	upstst_err_output
 	
	print error buffer
==============================================================================*/
int	upstst_err_output (int argc, char ** const argv)
{

int 		status;
upstst_argt	argt[] = {{NULL, UPSTST_ARGV_END, NULL, NULL}};

/* parse command line
   ------------------ */

status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE(status,argt,argv[0]);

upserr_output();					/* output error */
return 0;						/* success */
}


/* ============================================================================

ROUTINE:
	upstst_err_clear
 	
	clear error 
==============================================================================*/
int	upstst_err_clear (int argc, char ** const argv)
{

int 		status;
upstst_argt	argt[] = {{NULL, UPSTST_ARGV_END, NULL, NULL}};

/* parse command line
   ------------------ */

status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE(status,argt,argv[0]);

upserr_clear();						/* clear buffer */
return 0;						/* success */
}


/* ============================================================================

ROUTINE:
	upstst_err_add
 	
	add error 
	number, severity, associated info)
==============================================================================*/
int	upstst_err_add (int argc, char ** const argv)
{

int 		status;
static char	*funcname = "upserr_add";
static char	*severity;
static char	*error_str;
static char	*extra_str;
int		error = 0;
upstst_argt	argt[] = {{"<error>",  UPSTST_ARGV_STRING,NULL,&error_str},
			  {"-severity",UPSTST_ARGV_STRING,NULL,&severity},
			  {"[<extra>]",  UPSTST_ARGV_STRING,NULL,&extra_str},
			  {NULL,       UPSTST_ARGV_END,   NULL,NULL}};

/* parse command line
   ------------------ */

severity = UPS_INFORMATIONAL; error_str = NULL; extra_str = UPS_UNKNOWN_TEXT;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE(status,argt,argv[0]);

UPSTST_CHECK_ESTATUS (error_str,error);			/* get error */
upserr_add(error,severity,extra_str);			/* clear buffer */
return 0;						/* success */
}

