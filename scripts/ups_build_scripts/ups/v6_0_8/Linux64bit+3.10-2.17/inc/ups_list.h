/************************************************************************
 *
 * FILE:
 *       ups_list.h
 * 
 * DESCRIPTION: 
 *       Define all necessary command prototypes etc.
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
 *       18-Aug-1997, EB, first
 *
 ***********************************************************************/

#ifndef _UPS_LIST_H_
#define _UPS_LIST_H_

/* standard include files, if needed for .h file */

/* ups specific include files, if needed for .h file */
#include "upsugo.h"

/*
 * Constans.
 */

/*
 * Types.
 */

/*
 * Declaration of public functions.
 */
t_upslst_item *ups_list( t_upsugo_command * const a_command_line ,
			 const int verify );

/*
 * Declaration of private globals.
 */

/*
 * Declarations of public variables.
 */


#endif /* _UPS_LIST_H_ */
