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
 *       11-Nov-1997, DjF, First
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* ups specific include files */
#include "upserr.h"
#include "upsver.h"
/*
 * Definition of public variables.
 */


/*
 * Declaration of private functions.
 */


/*
 * Definition of global variables.
 */

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * upsver_mes
 *
 * Input : verbose level to output on, format, and variables
 * Output: none
 * Return: none
 */
void upsver_mes (const int vl, char * const format, ...)
{
  va_list args;

  if ( UPS_VERBOSE >= vl )
  { va_start(args,format);
    (void) vfprintf(stderr,format, args);
    va_end(args);
  }
}
