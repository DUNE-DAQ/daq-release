/*


Authors:        Margaret Votava
e-mail:         "votava@fnal.gov"
 
Include files:-
===============
*/

#include <time.h>
#include "upstst_parse.h"
#include "upstst_macros.h"


/* ============================================================================

ROUTINE:
	upstst_date
 	
	print date to stderr
==============================================================================*/
int	upstst_date		(int argc, char ** const argv)
{

time_t	stime;			/* time */
int 		status;
upstst_argt	argt[] = {{NULL, UPSTST_ARGV_END, NULL, NULL}};

/* parse command line
   ------------------ */

status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE(status,argt,argv[0]);

/* echo date
   --------- */

stime = time(NULL);					/* display time */
(void) printf (asctime(localtime(&stime)));
return 0;						/* success */
}


/* ============================================================================

ROUTINE:
	upstst_echo
 	
	print string to stderr
==============================================================================*/
int	upstst_echo		(int argc, char ** const argv)
{
int 		status;				/* status */
static char	*mystring;			/* string */
upstst_argt	argt[] = {
	{"<mystring>",	UPSTST_ARGV_STRING,	NULL,		&mystring},
	{NULL,		UPSTST_ARGV_END,		NULL,		NULL}};

/* parse command line
   ------------------ */

mystring = NULL;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE (status, argt, argv[0]);

/* echo string to stderr
   --------------------- */

(void) printf ("%s\n",mystring);
return 0;
}

/* ============================================================================

ROUTINE:
	upstst_debug_level
 	
	set/display the debug level

==============================================================================*/
int	upstst_debug_level	(int argc, char ** const argv)
{
int 		status;			/* status */
static int	testflag;		/* test flag */
static int	level;			/* string */
upstst_argt	argt[] = {
	{"[level]",	UPSTST_ARGV_INT,		NULL,		&level},
	{"-test",	UPSTST_ARGV_CONSTANT,	(char *)TRUE,	&testflag},
	{NULL,		UPSTST_ARGV_END,		NULL,		NULL}};

/* parse command line
   ------------------ */

level = -1; testflag = FALSE;
status = upstst_parse (&argc, argv, argt, UPSTST_PARSE_NOLEFTOVERS);
UPSTST_CHECK_PARSE (status, argt, argv[0]);

/* either set or show debug level
   ------------------------------ */

if (level == -1)					/* show current level */
   {
   (void) printf ("Current test debug level is: %d\n",upstst_debug);
#if 0
   (void) printf ("Current ups debug level is: %d\n",ups_debug);
#endif
   }
else							/* change level */
   {
   if (testflag)
      upstst_debug = level;
#if 0
   else
      ups_debug = level;
#endif
   }

return 0;
}
