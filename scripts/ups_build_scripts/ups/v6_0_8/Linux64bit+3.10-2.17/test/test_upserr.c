/************************************************************************
 *
 * FILE:
 *       test_ups_error.c
 * 
 * DESCRIPTION: 
 *       Test the error handling routines
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
 *       22-Jun-1997, EB, first
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>

/* ups specific include files */
#include "upserr.h"

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

int main(void)
{
  (void) fprintf(stderr, "\nOutputing with an empty stack - expect nothing.\n");
  upserr_output();
  (void) fprintf(stderr, "-------------------------------------------------\n");

  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE1");
  upserr_add(UPS_READ_FILE, UPS_FATAL, "FILE2");
  upserr_add(UPS_INVALID);

  (void) fprintf(stderr, "\nOutputing 3 messages.\n");
  upserr_output();
  (void) fprintf(stderr, "-------------------------------------------------\n");

  upserr_clear();

  UPS_VERBOSE = 1;
  upserr_vplace();
  upserr_add(UPS_INVALID_KEYWORD, UPS_WARNING, "BORING", "FILE1");
  upserr_vplace();
  upserr_add(UPS_NO_DATABASE, UPS_INFORMATIONAL);
  UPS_VERBOSE = 0;

  (void) fprintf(stderr, "\nOutputing 2 messages.\n");
  upserr_output();
  (void) fprintf(stderr, "-------------------------------------------------\n");

  upserr_vplace();
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE1");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE2");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE3");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE4");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE5");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE6");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE7");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE8");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE9");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE10");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE11");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE12");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE13");
  upserr_vplace();
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE14");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE15");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE16");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE17");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE18");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE19");
  upserr_add(UPS_OPEN_FILE, UPS_FATAL, "FILE20");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD1", "FILE21");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD2", "FILE22");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD3", "FILE23");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD4", "FILE24");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD5", "FILE25");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD6", "FILE26");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD7", "FILE27");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD8", "FILE28");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD9", "FILE29");
  upserr_vplace();
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD10", "FILE30");

  (void) fprintf(stderr, "\nOutputing 30 messages.\n");
  upserr_output();
  (void) fprintf(stderr, "-------------------------------------------------\n");

  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD11", "FILE31");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD12", "FILE32");

  (void) fprintf(stderr, "\nOutputing 30 messages. The oldest 2 from above have popped off\n");
  upserr_output();
  (void) fprintf(stderr, "-------------------------------------------------\n");

  upserr_clear();
  (void) fprintf(stderr, "\nOutputing with an empty stack - expect nothing.\n");
  upserr_output();
  (void) fprintf(stderr, "-------------------------------------------------\n");

  (void) fprintf(stderr, "\nOutputing 2 messages.\n");
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD8", "FILE28");
  UPS_VERBOSE = 1;
  upserr_vplace();
  upserr_add(UPS_INVALID_KEYWORD, UPS_FATAL, "KYWD9", "FILE29");
  upserr_output();
  (void) fprintf(stderr, "-------------------------------------------------\n");

  (void) fprintf(stderr, "\nAnd now backup over the last one.\n");
  upserr_backup();
  upserr_output();
  (void) fprintf(stderr, "-------------------------------------------------\n");

  return 0;
}

