/************************************************************************
 *
 * FILE:
 *       upserr.c
 * 
 * DESCRIPTION: 
 *       This module contains the error handling functions.
 *
 * AUTHORS:
 *       Eileen Berman
 *       David Fagan
 *       Lars Rasmussen
 *
 *       Fermilab Computing Division
 *       Batavia, Il 60510, U.S.A.
 *
 * MODIFICATIONS:
 *       15-Jul-1997, EB, first
 *       13-Aug-1997, LR, added UPS_LINE_TOO_LONG
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* ups specific include files */
#include "upserr.h"

/*
 * Definition of public variables.
 */

int UPS_ERROR = UPS_SUCCESS;    /* start out with no errors */
int UPS_VERBOSE = 0;            /* start out not verbose */
int UPS_VERIFY = 0;
int g_ups_line = 0;
char *g_ups_file = '\0';

/*
 * Declaration of private functions.
 */


/*
 * Definition of global variables.
 */
#define G_BUFSIZE 2000
#define G_ERROR_INIT -1
#define G_ERROR_BUF_MAX 100

static int g_buf_counter = G_ERROR_INIT;   /* pointer to current message */
static int g_buf_start = G_ERROR_INIT;     /* pointer to oldest message */

static char *g_error_buf[G_ERROR_BUF_MAX];

/* And now the error messages */
static char *g_error_messages[UPS_NERR] = {
/* 00 */  "%s: Success.\n",
/* 01 */  "%s: Unable to open file %s.\n",
/* 02 */  "%s: Unable to read file %s.\n",
/* 03 */  "%s: Invalid keyword - %s\n   in %s found.\n",
/* 04 */  "%s: No database specified on command line or in $PRODUCTS.\n",
/* 05 */  "%s: CPU time used - %f, Wall clock time used %f.\n",
/* 06 */  "%s: File name and path too long, must be less than %d bytes.\n",
/* 07 */  "%s: No statistics directory specified.\n",
/* 08 */  "%s: Unable to write file %s.\n",
/* 09 */  "%s: Invalid argument specified \"%s\"\n",
/* 10 */  "%s: No instance matches were made between the chain file (%s) and the version file (%s.version)\n",
/* 11 */  "%s: The passed filename was longer than the allowed system maximum (%d)\n",
/* 12 */  "%s: No instance matches were made between the \nversion file (%s) and the \ntable file (%s) for flavor (%s) and qualifiers (%s)\n",
/* 13 */  "%s: File not found - %s\n",
/* 14 */  "%s: Could not malloc %d bytes\n",
/* 15 */  "%s: MAX_LINE_LEN exceeded in \"%s\"\n",
/* 16 */  "%s: Unknown file type \"%s\"\n",
/* 17 */  "%s: Invalid value \"%s\" for argument \"%s\"\n",
/* 18 */  "%s: Invalid action - %s\n",
/* 19 */  "%s: Invalid argument to action - %s\n",
/* 20 */  "%s: Too many arguments to action - %s\n",
/* 21 */  "%s: Invalid shell - %s\n",
/* 22 */  "%s: Need unique instance but multiple \"%s\" found \n",
/* 23 */  "%s: Error in call to %s: %s\n",
/* 24 */  "%s: \"%s\" is not authorized for use on this node\n",
/* 25 */  "%s: No destination was specified in the UPS Database Configuration file for where to copy the %s files\n",
/* 26 */  "%s: Error when writing to temp file while processing %s action\n",
/* 27 */  "%s: Invalid number of parameters for action %s, must be between %i and %i (found %i)\n",
/* 28 */  "%s: UPS_SHELL not set using SHELL or default, value = %s\n",
/* 29 */  "%s: Unsetup of %s failed, continuing with setup\n",
/* 30 */  "%s: Cannot create file %s, it already exists\n",
/* 31 */  "%s: No table file name was specified on the command line\n",
/* 32 */  "%s: Version specified already exists\n",
/* 33 */  "%s: Version/Chain and Table file both cannot be specified on the command line \n",
/* 34 */  "%s: Cannot do unsetup, SETUP_%s is not defined\n",
/* 35 */  "%s: Cannot do %s, no product specified on command line\n",
/* 36 */  "%s: Action parsing failed on \"%s\"\n",
/* 37 */  "%s: Mismatch in unsetup and SETUP_ env. variable for %s\n",
/* 38 */  "%s: There is no ACTION=%s section in this table file.\n",
/* 39 */  "%s: No instance to copy to specified on command line.\n",
/* 40 */  "%s: Invalid Specification for %s -\n\t%s\n",
/* 41 */  "%s: No instance found for product: \'%s\'\n",
/* 42 */  "%s: Duplicate instance in %s file\n   %s \n   key = \"%s\" \"%s\" \"%s\" \"%s\" hash(%d)\n",
/* 43 */  "%s: Specified %s file %s\n   does not exist in specified or default locations\n", /* file not found not good enough */
/* 44 */  "%s: Missing match in %s file \n   File: %s\n",
/* 45 */  "%s: Product \'%s\' (with qualifiers \'%s\'), has no %s %s %s\n",
/* 46 */  "%s: Found no match for product \'%s\'\n",
/* 47 */  "%s: Unexpected key word \'%s\' in \'%s\', line %d\n",
/* 48 */  "%s: File cache is corrupt\n",
/* 49 */  "%s: No qualifiers found - in \'%s\', line %d\n",
/* 50 */  "%s: No \'PROD_DIR\' keyword for product \'%s\'\n",
/* 51 */  "%s: Chain keyword \'%s\' must match name of file it is in \'%s\'\n",
/* 52 */  "%s: Version keyword \'%s\' must match name of file it is in \'%s\'\n",
/* 53 */  "%s: Verifying product \'%s\'\n",
/* 54 */  "%s: Environment Variables must be of the form ${var} - not as in  \'%s\'\n",
/* 55 */  "%s:  \'%s\' is not a directory\n",
/* 56 */  "%s:  \'%s\'is not an allowable separator in \'%s\'\n",
/* 57 */  "%s: TABLE_FILE keyword not specified when TABLE_DIR was, in version \'%s\'\n",
/* 58 */  "%s: COMPILE_FILE keyword was not specified when COMPILE_DIR was, in version \'%s\'\n",
/* 59 */  "%s: Product keyword \'%s\' must match name of product it is in \'%s\'\n",
/* 60 */  "%s: Unable to translate or evaluate \'%s\'\n",
/* 61 */  "%s: Name of created temp file is %s\n",
/* 62 */  "%s: File %s must have %s keyword equal to %s in it\n",
/* 63 */  "%s: Action function \'%s\' is not supported in %s\n",
/* 64 */  "%s: The \'execute\' function must have \'UPS_ENV\' or \'NO_UPS_ENV\' as a second parameter, not \'%s\'\n",
/* 65 */  "%s: Flavor=ANY is not allowed in a %s file\n",
/* 66 */  "%s: Qualifiers=ANY is not allowed in a %s file\n",
/* 67 */  "%s: Ups command \'%s\' failed when processing temp file.\n",
/* 68 */  "%s: Possible UPS database (%s) corruption in product \'%s\'.\n",
/* 69 */  "%s: Unable to write statistics for product \'%s\'.\n",
/* 70 */  "%s: Error occurred for product \'%s\' \'%s\' \'%s\' \'%s\'.\n",
/* 71 */  "%s: \'VERSION\' keyword not allowed in Table file for product \'%s\'\n",
/* 72 */  "%s: The following product has a \'%s\' chain instance but no matching version.\n",
/* 73 */  "%s: Unable to remove file %s.\n",
/* 74 */  "%s: Mismatched EndIf condition '%s' does not match If condition '%s'.\n",
/* 75 */  "%s: Version conflict -- different branches of dependency tree require conflicting versions of product %s: %s %s vs %s\n",
/* 76 */  "%s: Version conflict -- dependency tree requires versions conflicting with current setup of product %s: %s %s vs %s\n",
};

char *g_error_ascii[] = {
   /* UPS_SUCCESS             0  */ "UPS_SUCCESS",
   /* UPS_OPEN_FILE           1  */ "UPS_OPEN_FILE",
   /* UPS_READ_FILE           2  */ "UPS_READ_FILE",
   /* UPS_INVALID_KEYWORD     3  */ "UPS_INVALID_KEYWORD",
   /* UPS_NO_DATABASE         4  */ "UPS_NO_DATABASE",
   /* UPS_TIME                5  */ "UPS_TIME",
   /* UPS_NAME_TOO_LONG       6  */ "UPS_NAME_TOO_LONG",
   /* UPS_NO_STAT_DIR         7  */ "UPS_NO_STAT_DIR",
   /* UPS_WRITE_FILE          8  */ "UPS_WRITE_FAIL",
   /* UPS_INVALID_ARGUMENT    9  */ "UPS_INVALID_ARGUMENT",
   /* UPS_NO_VERSION_MATCH    10 */ "UPS_NO_VERSION_MATCH",
   /* UPS_FILENAME_TOO_LONG   11 */ "UPS_FILENAME_TOO_LONG",
   /* UPS_NO_TABLE_MATCH      12 */ "UPS_NO_TABLE_MATCH",
   /* UPS_NO_FILE             13 */ "UPS_NO_FILE",
   /* UPS_NO_MEMORY           14 */ "UPS_NO_MEMORY",
   /* UPS_LINE_TOO_LONG       15 */ "UPS_LINE_TOO_LONG",
   /* UPS_UNKNOWN_FILETYPE    16 */ "UPS_UNKNOWN_FILETYPE",
   /* UPS_NOVALUE_ARGUMENT    17 */ "UPS_NOVALUE_ARGUMENT",
   /* UPS_INVALID_ACTION      18 */ "UPS_INVALID_ACTION",
   /* UPS_INVALID_ACTION_ARG  19 */ "UPS_INVALID_ACTION_ARG",
   /* UPS_TOO_MANY_ACTION_ARG 20 */ "UPS_TOO_MANY_ACTION_ARG",
   /* UPS_INVALID_SHELL       21 */ "UPS_INVALID_SHELL",
   /* UPS_NEED_UNIQUE         22 */ "UPS_NEED_UNIQUE",
   /* UPS_SYSTEM_ERROR        23 */ "UPS_SYSTEM_ERROR",
   /* UPS_NOT_AUTH            24 */ "UPS_NOT_AUTH",
   /* UPS_NO_DESTINATION      25 */ "UPS_NO_DESTINATION",		    
   /* UPS_ACTION_WRITE_ERROR  26 */ "UPS_ACTION_WRITE_ERROR",
   /* UPS_INVALID_ACTION_PARAMS 27 */ "UPS_INVALID_ACTION_PARAMS",
   /* UPS_NOSHELL             28 */ "UPS_NOSHELL",
   /* UPS_UNSETUP_FAILED      29 */ "UPS_UNSETUP_FAILED",
   /* UPS_FILE_EXISTS         30 */ "UPS_FILE_EXISTS",
   /* UPS_NO_TABLE_FILE       31 */ "UPS_NO_TABLE_FILE",
   /* UPS_VERSION_EXISTS      32 */ "UPS_NO_TABLE_FILE",
   /* UPS_TABLEFILE_AND_VERSION 33 */ "UPS_TABLEFILE_AND_VERSION",
   /* UPS_NO_SETUP_ENV        34 */ "UPS_NO_SETUP_ENV",
   /* UPS_NO_PRODUCT          35 */ "UPS_NO_PRODUCT",
   /* UPS_ACTION_PARSE        36 */ "UPS_ACTION_PARSE",
   /* UPS_UNSETUP_CLASH       37 */ "UPS_UNSETUP_CLASH",		    
   /* UPS_NO_ACTION           38 */ "UPS_NO_ACTION",
   /* UPS_NO_NEW_INSTANCE     39 */ "UPS_NO_NEW_INSTANCE",
   /* UPS_INVALID_SPECIFICATION 40 */ "UPS_INVALID_SPECIFICATION",
   /* UPS_NO_PRODUCT_FOUND    41 */ "UPS_NO_PRODUCT_FOUND",
   /* UPS_DUPLICATE_INSTANCE  42 */ "UPS_DUPLICATE_INSTANCE",
   /* UPS_FILE_NOT_FOUND      43 */ "UPS_FILE_NOT_FOUND",
   /* UPS_MISSING_MATCH       44 */ "UPS_MISSING_MATCH",
   /* UPS_NO_INSTANCE         45 */ "UPS_NO_INSTANCE",
   /* UPS_NO_MATCH            46 */ "UPS_NO_MATCH",
   /* UPS_UNEXPECTED_KEYWORD  47 */ "UPS_UNEXPECTED_KEYWORD",
   /* UPS_FILEMAP_CORRUPT     48 */ "UPS_FILEMAP_CORRUPT",
   /* UPS_NO_QUALIFIER        49 */ "UPS_NO_QUALIFIER",
   /* UPS_NO_PROD_DIR         50 */ "UPS_NO_PROD_DIR",
   /* UPS_MISMATCH_CHAIN      51 */ "UPS_MISMATCH_CHAIN",
   /* UPS_MISMATCH_VERSION    52 */ "UPS_MISMATCH_VERSION",
   /* UPS_VERIFY_PRODUCT      53 */ "UPS_VERIFY_PRODUCT",
   /* UPS_VERIFY_ENV_VAR      54 */ "UPS_VERIFY_ENV_VAR",
   /* UPS_NOT_A_DIR           55 */ "UPS_NOT_A_DIR",
   /* UPS_INVALID_SEPARATOR   56 */ "UPS_INVALID_SEPARATOR",
   /* UPS_VERIFY_TABLE_DIR    57 */ "UPS_VERIFY_TABLE_DIR",
   /* UPS_VERIFY_COMPILE_DIR  58 */ "UPS_VERIFY_COMPILE_DIR",
   /* UPS_MISMATCH_PROD_NAME  59 */ "UPS_MISMATCH_PROD_NAME",
   /* UPS_NO_TRANSLATION      60 */ "UPS_NO_TRANSLATION",
   /* UPS_TEMP_FILE           61 */ "UPS_TEMP_FILE",
   /* UPS_NO_KEYWORD          62 */ "UPS_NO_KEYWORD",
   /* UPS_ACTION_PARSE_INVALID 63 */ "UPS_ACTION_PARSE_INVALID",
   /* UPS_EXECUTE_ARG2        64 */ "UPS_EXECUTE_ARG2",
   /* UPS_INVALID_ANY_FLAVOR  65 */ "UPS_INVALID_ANY_FLAVOR",
   /* UPS_INVALID_ANY_QUALS   66 */ "UPS_INVALID_ANY_QUALS",
   /* UPS_COMMAND_FAILED      67 */ "UPS_COMMAND_FAILED",
   /* UPS_DB_CORRUPTION       68 */ "UPS_DB_CORRUPTION",
   /* UPS_STATISTICS          69 */ "UPS_STATISTICS",
   /* UPS_PRODUCT_INFO        70 */ "UPS_PRODUCT_INFO",
   /* UPS_NO_VERSION          71 */ "UPS_NO_VERSION",
   /* UPS_DANGLING_CHAIN      72 */ "UPS_DANGLING_CHAIN",
   /* UPS_REMOVE_FILE         73 */ "UPS_REMOVE_FILE", 
   /* UPS_MISMATCHED_ENDIF    74 */ "UPS_MISMATCHED_ENDIF",
   /* UPS_DEP_CONFLICT        75 */ "UPS_DEP_CONFLICT",
   /* UPS_SETUP_CONFLICT      76 */ "UPS_SETUP_CONFLICT",
   /* UPS_NERR                77 */ "UPS_NERR",
   0 };

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * upserr_add
 *
 * Add the requested message to the error buf.  If the error buf is too
 * big, overwrite the oldest error message.
 *
 * Input : error message number, error severity and associated information
 * Output: none
 * Return: none
 */
void upserr_add (const int a_error_index, ...)
{
  va_list args;
  char buf[G_BUFSIZE];
  char *tmpBufPtr;

  /* Initialize */
  buf[0] = '\0';

  if ( (a_error_index < UPS_NERR) && (a_error_index > UPS_INVALID)) {
    /* format the error and put it in the error buf */
    va_start(args, a_error_index);
    (void) vsprintf(buf, g_error_messages[a_error_index], args);
    va_end(args);

    /* figure out if it was an informational message or not.  if so, do
       not reset UPS_ERROR */
    if (strncmp(buf, UPS_INFORMATIONAL, UPS_INFORMATIONAL_LEN)) {
      UPS_ERROR = a_error_index;
    }
  }
  else {
    /* This was an invalid error message request */
    (void) sprintf(buf, "ERROR: Invalid error message number %d.\n", a_error_index);
    UPS_ERROR = UPS_INVALID;
  }


  /* Check if we need to add error location information to output too */
  if (UPS_VERBOSE && g_ups_line) {
    (void) sprintf(buf, "%s (line number %d in file %s)\n", buf, g_ups_line,
	    g_ups_file);
    g_ups_line = 0;          /* reset so next time do not give false info */
  }

  /* Malloc space for the message so we can save it and copy it in. */
  tmpBufPtr = (char *)malloc(strlen(buf) + 1);  /* leave room for the \0 too */
  (void) strcpy(tmpBufPtr, buf);

  /* Add the error message to the error buf.  If we are at the bottom of
     the buf, go back to the top and overwrite the oldest message */
  if (++g_buf_counter == G_ERROR_BUF_MAX) {
    /* we have reached the end of the buf, go to the start */
    g_buf_counter = 0;
  }

  if (g_buf_counter == g_buf_start) {
    /* the buf is full, we must delete the oldest message to make room */
    free(g_error_buf[g_buf_start++]);
    if (g_buf_start == G_ERROR_BUF_MAX) {
      /* we have reached the end of the buf, go to the start */
      g_buf_start = 0;
    }
  }

  /* check if this is our first message*/
  if (g_buf_start == G_ERROR_INIT) {
    /* yes, move to start of buffer */
    g_buf_start = 0;
  }

  g_error_buf[g_buf_counter] = tmpBufPtr;
}

/*-----------------------------------------------------------------------
 * upserr_backup
 *
 * Backup over the last error added to the buffer
 *
 * Input : none
 * Output: none
 * Return: none
 */
void upserr_backup (void)
{
  /* only do something if there are messages in the buffer */
  if (g_buf_start != G_ERROR_INIT) {
    /* free the current message */
    free(g_error_buf[g_buf_counter]);
    g_error_buf[g_buf_counter] = NULL;

    /* adjust the current message counter */
    --g_buf_counter;
    if (g_buf_counter < 0) {
      /* if the oldest msg is the first one, then that was the only one */
      if (g_buf_start == 0) {
	g_buf_start = G_ERROR_INIT;
	g_buf_counter = G_ERROR_INIT;
      } else {
	/* reset to the last entry in the buffer */
	g_buf_counter = G_ERROR_BUF_MAX - 1;
      }
    }
  }

  UPS_ERROR = UPS_SUCCESS;
  
}
/*-----------------------------------------------------------------------
 * upserr_clear
 *
 * Clear out the error buf.  All messages currently in the buf are lost.
 *
 * Input : none
 * Output: none
 * Return: none
 */
void upserr_clear (void)
{
  int i;

  /* only do something if there are messages in the buffer */
  if (g_buf_start != G_ERROR_INIT) {
    /* free all of the error message bufs */
    for (i = g_buf_start; i != g_buf_counter; ++i) {
      if (i < G_ERROR_BUF_MAX) {
	free(g_error_buf[i]);
      } else {
	i = G_ERROR_INIT;
      }
    }

    /* catch the last one we missed */
    free(g_error_buf[g_buf_counter]);

    /* Reset */
    g_buf_counter = G_ERROR_INIT;

    /* Reset */
    g_buf_start = G_ERROR_INIT;
  }

  UPS_ERROR = UPS_SUCCESS;
}

/*-----------------------------------------------------------------------
 * upserr_output
 *
 * Output the error buf to stderr if it is not empty.
 *
 * Input : none
 * Output: none
 * Return: none
 */
void upserr_output (void)
{
  int i;

  /* only do something if there are messages in the buffer */
  if (g_buf_start != G_ERROR_INIT) {
    for (i = g_buf_start; i != g_buf_counter; ++i) {
      if (i < G_ERROR_BUF_MAX) {
	if (g_error_buf[i]) {
	  (void) fputs(g_error_buf[i], stderr);
	}
      } else {
	i = G_ERROR_INIT;
      }
    }

    /* catch the last one we missed */
    if (g_error_buf[g_buf_counter]) {
      (void) fputs(g_error_buf[g_buf_counter], stderr);
    }
  }
}

