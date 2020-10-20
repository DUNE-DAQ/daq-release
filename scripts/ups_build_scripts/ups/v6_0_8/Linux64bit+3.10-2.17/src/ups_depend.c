/************************************************************************
 *
 * FILE:
 *       ups_depend.c
 * 
 * DESCRIPTION: 
 *       To list product dependencies
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
 *       17-Dec-1997, LR, first
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>

/* ups specific include files */
#include "ups_depend.h"
#include "upserr.h"
#include "upsugo.h"
#include "upsact.h"
#include "ups_main.h"

/*
 * Definition of public variables.
 */

/*
 * Declaration of private functions.
 */

static void shutup (const char * const s_cmd);

#define SHUTUP \
  if ((&bit_bucket + bit_bucket_offset == 0) && 0) shutup (s_cmd);

/*
 * Definition of global variables.
 */

static long bit_bucket = 0;
static long bit_bucket_offset = 0;

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * ups_depend
 *
 * To list product dependencies
 *
 * Input : command line structure,
 *         ups command,
 *         enum of ups command,
 * Output: none
 * Return: none
 */
t_upslst_item *ups_depend( t_upsugo_command * const u_cmd, 
			   const char * const s_cmd,
			   const int e_cmd )
{
  if ( u_cmd->ugo_K ) {

    /* use the K options */

    (void) upsact_print( u_cmd, 0, "setup", e_cmd, "K" );

  }
  else if ( u_cmd->ugo_l > 0 )

    /* normal listing, long */

    (void) upsact_print( u_cmd, 0, "setup", e_cmd, "l" );

  else

    /* normal listing */

    (void) upsact_print( u_cmd, 0, "setup", e_cmd, "" );

  SHUTUP;

  return NULL;
}

static void shutup (const char * const s_cmd)
{
  bit_bucket ^= (long) s_cmd;
}
