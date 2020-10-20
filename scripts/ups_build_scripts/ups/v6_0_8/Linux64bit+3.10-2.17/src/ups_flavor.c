/************************************************************************
 *
 * FILE:
 *       ups_flavor.c
 * 
 * DESCRIPTION: 
 *       This is the 'ups flavor' command.
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
 *       Tue Jan 20, DjF, Starting...
 *
 ***********************************************************************/

/* standard include files */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ups specific include files */
#include "ups.h"

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
 * ups_flavor
 *
 * This is the main line for the 'ups flavor' command.
 *
 * Input : 
 * Output: 
 * Return: 
 */
void ups_flavor( t_upsugo_command * const uc )
{ t_upslst_item *l_ptr=0;
  int count=0;
  l_ptr=uc->ugo_flavor;
  if(!uc->ugo_l)
  { (void) printf("%s\n",(char *)l_ptr->data);
  } else {
    for ( l_ptr = upslst_first(uc->ugo_flavor); l_ptr; 
          l_ptr = l_ptr->next, count++ )
    { (void) printf("%s\n",(char *)l_ptr->data);
} } }
