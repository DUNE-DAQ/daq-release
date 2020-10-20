/*

Authors:        Margaret Votava
e-mail:         "votava@fnal.gov"
 
Revision history:-
=================
10-Oct-1995 MEV	created - stolen from ftcl_ParseArgv which was heavily 
		inspired by the Tk command line parser.

upstst_parse()  - Parse command line options
upstst_print_usage()- Get the usage string for a command
upstst_split()	- split command line into argv and arc like stuff

Include files:-
===============
*/
 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "upstst_parse.h"

/* Global variables:-
   ================== */

/* This variable is a pointer to a string of all options/parameters that 
   appear on the command line.  
   --------------------------------------------------------------------- */

#define ARGSIZ  (BUFSIZ/2)
static char g_Specified_args[ARGSIZ];

/* Forward declarations
   ==================== */

static int 	upstst_pass1 (int * const, char ** const, upstst_argt * const, 
                              const unsigned int options);
static int  	upstst_pass2 (int * const, char **, upstst_argt * const);
static int   	assign_defaults(upstst_argt * const);

/* define names of tcl array where ups_ParseArgv values will be stored 
 * and define the array element names for each value
 */
#define UPSTST_INDENT		"     "


/*==============================================================================
ROUTINE: upstst_parse

Process an argv array according to a table of expected command-line options.

 (int ) upstst_parse(int *argcPtr, char **argv, shArgvInfo *argTable, int options)
          argcPtr  - Pointer to the argument count address
          argv     - Command line argument array
          argTable - Table of expected command-line options
	  options  - options
  Note: parameters marked by # will be changed as side effects.

DESCRIPTION:
  Processes an argv array according to a table of expected command
  line options. See the man page for more details (man page should
  be somewhere under $FTCL_DIR)

RETURNS:
   UPSTST_SUCCESS 	: if command-line options parsed successfully
   UPSTST_BADSYNTAX 	: on error. 
*/
int upstst_parse(int * const argcPtr, char ** const argv, 
   upstst_argt * const argTable, const unsigned int options)
{
int 			retValue, length;
register upstst_argt 	*ptr;

/* Populate g_Specified_args, the buffer that holds all recognized command
   line options and  parameters. We want to make sure that we do not
   overflow it. So lets count all characters in the options/parameters
   specified in the argument table. Their count should be <= ARGSIZ - 1 
   (since g_Specified_args is dimensioned for ARGSIZ)
   ------------------------------------------------------------------------  */

for (length = 0, ptr = argTable; ptr->type != UPSTST_ARGV_END; ptr++)
   {      
   if (ptr->key) length += (int) (strlen(ptr->key) + 1);
   }


if (length >= ARGSIZ)  
   {
   (void) fprintf(stderr,"Internal buffer overflow in ups test program(). Please notify "
              "ups maintainer!\n");
   abort();
   }

g_Specified_args[0] = 0;

retValue = upstst_pass1(argcPtr, argv, argTable, options);/* take out switch */
if (retValue) return (retValue);			/* return error */

retValue = upstst_pass2(argcPtr, argv, argTable);	/* take out param */
if (retValue) return (retValue);			/* return error */

if (*argcPtr > 1 && options & UPSTST_PARSE_NOLEFTOVERS)	/* no leftovers! */
   {         
   (void) fprintf (stderr,"Syntax Error: unprocessed command line parameters remain ");
   (void) fprintf (stderr,"\n    (check for extra parameters or invalid options)\n");
   return UPSTST_BADSYNTAX;
   }

return UPSTST_SUCCESS;
}

/*==============================================================================
ROUTINE
  upstst_pass1 - Do a first pass through argv[], weeding out all known 
               switches specified in argTable[].

CALL: 
  (static int) upstst_pass1 (int *argcPtr,
                          char **argv, upstst_argt *argTable)
               argcPtr  - Pointer to the address of argument count
               argv     - Command line argument array
               argTable - Table of known arguments

DESCRIPTION:
  upstst_pass1() performs a first pass through the command line arguments
  specified in argv[]. As it goes thru it's motions, it strips out all
  known arguments. Known arguments are specified in argTable[]. As an
  argument is recognized and processed, it is removed from argv[]. 
  *argcPtr will be decremented to account for this. Only switches
  are weeded out in this pass. Positional parameters are searched for 
  in the next pass.

  Example - Consider the following command line argument:

     myCommand reg1 20 -debugLevel 3 -useMalloc -delay 10 -o

  Also consider that argTable[] knows about the following arguments
  <reg1>       - Positional parameter
  [<numCols>]  - Optional positional parameter
  -debugLevel  - Switch that takes an argument
  -useMalloc   - Switch that takes no argument

  After a call to this function, argv[] will look like:
             argv[0] ---> myCommand
             argv[1] ---> reg1
             argv[2] ---> 20
             argv[3] ---> -delay
             argv[4] ---> 10
             argv[5] ---> -o
  argcPtr will have been modified to contain 6.

RETURNS:
   UPSTST_SUCCESS   : if command-line options parsed successfully
   UPSTST_BADSYNTAX : on error. Reason for error is put in interp->result
============================================================================= */
static int upstst_pass1(int * const argcPtr, char ** const argv, upstst_argt * const argTable, const unsigned int options)
{
upstst_argt *infoPtr;
			/* Pointer to the current entry in the
			 * table of argument descriptions. */
upstst_argt *matchPtr;	/* Descriptor that matches current argument. */
char *curArg;		/* Current argument */
int srcIndex;		/* Location from which to read next argument from argv*/
int dstIndex;		/* Index into argv to which next unused
				 * argument should be copied (never greater
				 * than srcIndex). */
size_t length;		/* Number of characters in current argument. */
char tmpKeyBuf[30];

#define upstst_missing_arg(myarg) {					\
   (void) fprintf(stderr,"Syntax Error: %s option requires an additional argument\n",\
    myarg);								\
   return UPSTST_BADSYNTAX;						\
   }

if (assign_defaults(argTable) == 0) return UPSTST_BADSYNTAX;

    /*
     * Do the real parsing, skipping over first arg (ie command)
     */
for (srcIndex = dstIndex = 1; srcIndex < *argcPtr; )
   {
   curArg = argv[srcIndex];
   srcIndex++;
   length = strlen(curArg);
   if (length == 0) 
      {
	  /*
	   * Someone passed a NULL string.  Just move on.
	   */
      argv[dstIndex] = curArg;
      dstIndex++;
      continue;
      }

	/*
	 * Loop throught the argument descriptors searching for one with
	 * the matching key string.  If found, leave a pointer to it in
	 * matchPtr.
	 */

   if (!strcmp(curArg,"-usage")) return UPSTST_USAGE;
   matchPtr = NULL;
   for (infoPtr = argTable; infoPtr->type != UPSTST_ARGV_END; infoPtr++)

      {
      if (infoPtr->key == NULL)  continue;
      if (options && UPSTST_PARSE_EXACTMATCH)
         {
         if (strcmp(infoPtr->key, curArg)) continue;
         }
      else
         {
         if (strncmp(infoPtr->key, curArg, length)) continue;
         }
      if (infoPtr->key[length] == 0) 
	 {
	 matchPtr = infoPtr;
	 goto gotMatch;
	 }
      if (matchPtr != NULL) 
	 {
	 (void) fprintf (stderr, "Syntax Error: ambiguous option %s ", curArg); 
	 return UPSTST_BADSYNTAX;
	 }
      matchPtr = infoPtr;
      }

   if (matchPtr == NULL) 
      {
	    /*
	     * Unrecognized argument.  Just copy it down
	     */
      argv[dstIndex] = curArg;
      dstIndex++;
      continue;
      }

	/*
	 * Take the appropriate action based on the option type
	 */

gotMatch:
   infoPtr = matchPtr;
   switch (infoPtr->type) {

      case UPSTST_ARGV_CONSTANT:
 	 *((int *) infoPtr->dst) = (int) infoPtr->src;
         (void) sprintf(tmpKeyBuf, "%s,", infoPtr->key);
         (void) strcat(g_Specified_args, tmpKeyBuf);
	 break;

      case UPSTST_ARGV_INT:
         {
	 char *endPtr;
	 if (srcIndex == *argcPtr) upstst_missing_arg(curArg);

	 *((int *) infoPtr->dst) = (int) strtoul(argv[srcIndex], &endPtr, 0);
	 if ((endPtr == argv[srcIndex]) || (*endPtr != 0)) 
	    {
	    (void) fprintf (stderr,"Syntax Error: expected integer argument "
	 	"for %s but got %s \n", infoPtr->key,argv[srcIndex]);
	    return UPSTST_BADSYNTAX;
	    }
	 srcIndex++;
         (void) sprintf(tmpKeyBuf, "%s,", infoPtr->key);
         (void) strcat(g_Specified_args, tmpKeyBuf);
	 break;
	 }

      case UPSTST_ARGV_STRING:

	 if (srcIndex == *argcPtr) upstst_missing_arg(curArg);
	 *((char **)infoPtr->dst) = argv[srcIndex];
	 srcIndex++;
         (void) sprintf(tmpKeyBuf, "%s,", infoPtr->key);
         (void) strcat(g_Specified_args, tmpKeyBuf);
	 break;

      case UPSTST_ARGV_DOUBLE :

	 {
	 char *endPtr;
	 if (srcIndex == *argcPtr) upstst_missing_arg(curArg);
	 *((double *) infoPtr->dst) = strtod(argv[srcIndex], &endPtr);
	 if ((endPtr == argv[srcIndex]) || (*endPtr != 0)) 
	    {
	    (void) fprintf(stderr,"Syntax Error: expected floating-point argument ");
	    (void) fprintf(stderr," for %s but got %s\n",infoPtr->key, argv[srcIndex]);
	    return UPSTST_BADSYNTAX;
	    }
	 srcIndex++;
         (void) sprintf(tmpKeyBuf, "%s,", infoPtr->key);
         (void) strcat(g_Specified_args, tmpKeyBuf);
	 break;
	 }

      default:
	 
	 (void) fprintf(stderr,"Programmer Error: bad argument type %d in arg table",
	    infoPtr->type);
	  return UPSTST_BADSYNTAX;
      }
   }

    /*
     * Copy the remaining arguments down.
     */
for (;srcIndex < *argcPtr; argv[dstIndex++] = argv[srcIndex++]);
argv[dstIndex] = (char *) NULL;
*argcPtr = dstIndex;

return UPSTST_SUCCESS;

}

/*=============================================================================
ROUTINE
  upstst_pass2 - Do a second pass through argv[], weeding out all known
               positional parameters specified in argTable[].

CALL:
  (static int) upstst_pass2(int *argcPtr, char **argv, upstst_argt *argTable)
               argcPtr  - Pointer to the address of argument count
               argv     - Command line argument array
               argTable - Table of known arguments

DESCRIPTION:
  upstst_pass2() performs a second pass through the command line arguments
  specified in argv[]. Note that this argv[] has been (probably) modified
  by upstst_pass1(). Thus all known switches have been stripped out of argv.
  As pass2 goes thru it's motions, it strips out all positional
  parameters. Positional parameters are specified in argTable[]. As an
  parameter is recognized and processed, it is removed from argv[].
  *argcPtr will be decremented to account for this. 

  Example - Working on the command line argument from the comments in
            ups_Pass1(), let's assume argv[] is now the following:

          myCommand reg1 20 -delay 10 -o

  i.e. all known switches have been removed from it.

  Also consider that argTable[] knows about the following arguments
  <reg1>       - Positional parameter
  [<numCols>]  - Optional positional parameter
  -debugLevel  - Switch that takes an argument   # removed in ups_Pass1()
  -useMalloc   - Switch that takes no argument   # removed in ups_Pass1()

  After a call to this function, argv[] will look like:
             argv[0] ---> myCommand
             argv[3] ---> -delay
             argv[4] ---> 10
             argv[5] ---> -o
  argcPtr will have been modified to contain 6.

RETURNS:
  UPSTST_SUCCESS 	: on success
  UPSTST_BADSYNTAX  	: otherwise
============================================================================= */
static int upstst_pass2(int * const argcPtr, char **argv, 
   upstst_argt * const argTable)
{
upstst_argt 	*infoPtr;
int      	reqPosParams,	/* Count of required positional params */
		optPosParams,	/* Count of optional positional params */
		i;
short    	flag;		/* General purpose flag */
char     	*sp;		/* Utility pointer used in various places */
 
reqPosParams = optPosParams = 0;
flag = 0;
   /*
    * Do some housekeeping first: see how many positional parameters
    * are needed.
    */
for (infoPtr = argTable; infoPtr->type != UPSTST_ARGV_END; infoPtr++)  
   {
   if (infoPtr->key == NULL)  
      {
      flag = 1;
      break;
      }

   if (*infoPtr->key == '-') continue;

   if (*infoPtr->key == '<')
            reqPosParams++;
   else if (*infoPtr->key == '[')
            optPosParams++;
   else  
      {
      flag = 1;
      break;
      }
   }

if (flag)  
   {
   char *p;

   p = (infoPtr->key == NULL ? "NULL" : infoPtr->key);

   (void) fprintf (stderr, "Programmer Error: %s argument table key must be -null",p);
   (void) fprintf (stderr, " non-null and begin with\n");
   (void) fprintf (stderr, "either '<', '[', or '-' only\n");
   return UPSTST_BADSYNTAX;
   }
 
if (reqPosParams == 0 && optPosParams == 0) /* No sense sticking around */
   return UPSTST_SUCCESS;               /* if pos. params. not needed */
 
if (reqPosParams && *argcPtr <= 1)  
   {
   (void) fprintf (stderr, "Syntax Error: expected at least one positional parameter\n");
   return UPSTST_BADSYNTAX;
   }

if (*argcPtr <= 1)     /* Boundary case: if by this pass only argv[0]  */
   return UPSTST_SUCCESS;                   /* remains, return now */

for (infoPtr = argTable; infoPtr->type != UPSTST_ARGV_END; infoPtr++)  
   {
   if (infoPtr->key == NULL) continue;

   if (*infoPtr->key == '-')   /* Skip all switches */
      continue;
        /*
         * Now set argv[] to the first non-switch argument. A switch
         * argument is anything preceeding a '-'. While searching for 
         * the first non-switch argument, check the first character
         * beyond the '-'. If that character is an alpha, we have
         * a bonafide switch. If that character is a digit or it is a '.' we 
         * will recognize it as a positional parameter (a negative number).
         * Note that when we start searching argv[], we start from
         * argv[1], effectively skipping argv[0]
         */
   flag = 0;
   for (i = 1; argv[i] != NULL; i++)  
      {
      if (*argv[i] == '-')  
	 {
         if (isdigit((int)argv[i][1]) || argv[i][1] == '.')  
	    {
                     /*
                      * argv[i] could be specified as -0.99 or -.99
                      */
            flag = 1;
            break; 
	    }
         }
      else
         break;
      }
      
        /*
         * Boundary condition: see if argv[i] == NULL. If it is, error
         * out. argv[i] should not be NULL if more positional parameters
         * are expected.
         */
        
   if (*infoPtr->key == '[')  
      {
      if (argv[i] == NULL)
      break;
      }
   else if (*infoPtr->key == '<')  
      {
      if (argv[i] == NULL)  
         {
         (void) fprintf (stderr,"Syntax Error: expected %d more", reqPosParams); 
         (void) fprintf (stderr," required positional parameters\n");
         return UPSTST_BADSYNTAX;
         }
      reqPosParams--;
      }

        /*
         * Now make the address passed in the table for this positional 
         * parameter point to argv[i]
         */
   switch (infoPtr->type)  {
      char keyBuf[50], *pSp;

      case UPSTST_ARGV_INT :

         *((int *) infoPtr->dst) = (int) strtoul(argv[i], &sp, 0);
         if (sp == argv[i] || *sp != 0)  
	    {
	    (void) fprintf(stderr,"Syntax Error: expected integer positional\n"); 
	    (void) fprintf(stderr,"parameter for %s but got %s", infoPtr->key,argv[i]);
            return UPSTST_BADSYNTAX;
	    }
         (void) sprintf(keyBuf, "%s,", infoPtr->key);
         (void) strcat(g_Specified_args, keyBuf);
         break;

      case UPSTST_ARGV_DOUBLE :

         *((double *) infoPtr->dst) = strtod(argv[i], &sp);
         if (sp == argv[i] || *sp != 0)  
	    {
	    (void) fprintf(stderr,"Syntax Error: expected floating point positional\n"); 
	    (void) fprintf(stderr,"parameter for %s but got %s", infoPtr->key,argv[i]);
            return UPSTST_BADSYNTAX;
            }
         (void) sprintf(keyBuf, "%s,", infoPtr->key);
         (void) strcat(g_Specified_args, keyBuf);
         break;

      case UPSTST_ARGV_STRING :

         *((char **) infoPtr->dst) = argv[i];
         (void) sprintf(keyBuf, "%s,", infoPtr->key);
         (void) strcat(g_Specified_args, keyBuf);
         break;

      case UPSTST_ARGV_CONSTANT :
                 /*
                  * None of these types can have positional parameters.
                  * Get rid of positional parameter delimiters '<...>' or
                  * '[...]' from infoPtr->key. Store the result in keyBuf[].
                  */
         (void) sprintf(keyBuf, "%s", &infoPtr->key[1]);
         for (pSp = keyBuf; *pSp != 0; pSp++)
            if (*pSp == '>' || *pSp == ']') *pSp = 0;

         (void) fprintf(stderr,"Programmer Error: %s is not a ", infoPtr->key);
         (void) fprintf(stderr,"supported object for positional\n parameters. ");
	 (void) fprintf(stderr,"Positional parameters must be of type UPSTST_ARGV_INT");
	 (void) fprintf(stderr,",\nUPSTST_ARGV_DOUBLE, or UPSTST_ARGV_STRING only.");
	 (void) fprintf(stderr," Please modify your argument\ntable and declare %s",
	    infoPtr->key);
         (void) fprintf(stderr," as %s instead\n",keyBuf);
         return UPSTST_BADSYNTAX;
         /* break; */ /* NOTREACHED */
      }
      
        /* 
         * Now comes the fun part: shuffle the argv[] array to account
         * for the positional parameter just handled above. Example:
         * assume that the argv[] array contained:
         *       a.out -X 10 -name Region 20 -test
         * and that 20 is the positional parameter. Once the code above
         * has been executed and 20 has been recognized, argv[] should
         * be compacted so that it looks like:
         *       a.out -X 10 -name Region -test
         */
   for (; argv[i] != NULL; i++) argv[i] = argv[i+1];
   argv[i] = NULL;
         
   }

for (i = 0; argv[i] != NULL; i++)   /* Set *argcPtr to contain the number */
        ;                              /* of elements in argv[] */
*argcPtr = i;
 
return UPSTST_SUCCESS;
}


/*
 * NAME: assign_defaults()
 *
 * CALL:
 *   (int) assign_defaults(Tcl_Interp *a_interp, upstst_argt *argTable)
 *         a_interp - TCL Interpreter
 *         argTable - Argument table
 *
 * DESCRIPTION:
 *   assign_defaults() goes through the argument table assigning default 
 *   values from the src field to the dst field. The src field contains a
 *   string representation of the default value. It may be NULL. If the src
 *   field is not NULL, the value in the field is converted to the appropriate 
 *   representation of dst and saved there (in dst).
 *
 *   If the user later specifies a different value for a command line 
 *   argument, the value of dst will be over-written with the new one.
 *
 * RETURNS: 
 *   1 : on success
 *   0 : on failure. TCL Interp will contain the result of the failure.
 */
static int assign_defaults(upstst_argt * const argTable)
{
upstst_argt *infoPtr;

   /*
    * Go thru the table assigning default values if specified. Default
    * values are specified in the src field. So, if the src field is not
    * NULL, convert the src field to the proper representation of the dst
    * field, and save that value in the dst field. 
    *
    * This way, if the user specifies switch value on the command line, the
    * default values can always be overwritten later. If the user did not
    * specify a command line value, it will be properly defaulted.
    */
for (infoPtr = argTable; infoPtr->type != UPSTST_ARGV_END; infoPtr++)  
   {
   char   *pSrc, *pEnd;

   pSrc = (char *) infoPtr->src;

   switch (infoPtr->type)  {
      case UPSTST_ARGV_CONSTANT :
         break;
      case UPSTST_ARGV_INT      :
         if (pSrc != NULL)  
	    {
            *((int *) infoPtr->dst) = (int) strtoul(pSrc, &pEnd, 0);
            if (pEnd == pSrc || *pEnd != 0)  
	       {
	       (void) fprintf (stderr,"Parsing error: cannot convert default value " );
	       (void) fprintf (stderr,"%s to an integer",pSrc);
               return FALSE;
               }
            }
         break;
      case UPSTST_ARGV_DOUBLE   :
         if (pSrc != NULL)  
	    {
            *((double *)infoPtr->dst) = strtod(pSrc, &pEnd);
            if (pEnd == pSrc || *pEnd != 0)  
	       {
	       (void) fprintf (stderr,"Parsing error: cannot convert default value " );
	       (void) fprintf (stderr,"%s to an integer",pSrc);
               return FALSE;
               }
            }
         break;
      case UPSTST_ARGV_STRING  :
         if (pSrc != NULL) *((char **)infoPtr->dst) = pSrc;
         break;
      }
   }

return TRUE;
}

/*==============================================================================
ROUTINE:
  void upstst_print_usage

      routine to print a brief usage statement from argTable info.

CALL:
   ups_print_usage (upstst_argt *argTable,  char *cmd_name);

          argTable      - Table of known command-line options
          cmd_name      - name of command giving usage for

==============================================================================*/
void upstst_print_usage(const upstst_argt * const argTable, const char *cmd_name)
{
register const upstst_argt 	*infoPtr; 	/* ptr to entry in argTable */

if (NULL == cmd_name) cmd_name = "";		/* blank command */

(void) fprintf (stderr,"%s Usage: %s",UPSTST_INDENT,
   cmd_name);					/* main usage line */

for (infoPtr = argTable; infoPtr->type != UPSTST_ARGV_END; infoPtr++) 
   {
   if (infoPtr->key == NULL) continue;	/* no key */
   (void) fprintf (stderr," %s",infoPtr->key);	/* print key */
   }

(void) fprintf (stderr,"\n");				/* end with new line */
}


/* ============================================================================
ROUTINE:
   upstst_split
 
splits the input string into argc and argv like information based on
whitespace as the delimiter. things within "" are considered as one
argument.

Note that it scribbles on the original string (putting
in NULL's). and uses a static array of character pointers, so if you
call it again its previous return value points to the new data...

============================================================================= */

int upstst_split(char *input, int * const argc, char *** const argv)
{
static char *tmp_argv[255];

*argc = 0;
#define getquote() {				\
  input++;					\
  tmp_argv[(*argc)++] = input;			\
  while (*input && *input != '"') input++;	\
  if (*input)					\
     *input = 0;				\
  else						\
      return -1;				\
  }
	
while( *input ) 
   {
   if (*input == '"') 
      { getquote(); }
   else if (!(isspace((int)*input)))		/* start of word */
      {
      tmp_argv[(*argc)++] = input;		/* take full arg as is */
      while (*input && !(isspace((int)*input))) 
         {
         input++;				/* copy full word */
	 if (*input == '"')
            { *input = 0; getquote(); input++; }
         }
      if (*input == 0) goto split_end;		/* all done */
      *input = 0;				/* null terminate */
      }

   input++;
   }

split_end:

tmp_argv[*argc] = 0;
*argv = tmp_argv; 
return 0;
}



