/************************************************************************
 *
 * FILE:
 *       ups_copy.h
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
 *       2-Mar-1998, EB, first
 *
 ***********************************************************************/

#ifndef _UPS_COPY_H_
#define _UPS_COPY_H_

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
t_upslst_item *ups_copy( const t_upsugo_command * const a_command_line, 
			 const FILE * const a_temp_file, 
			 const int a_ups_command);

/*
 * Declaration of private globals.
 */

/*
 * Declarations of public variables.
 */


#endif /* _UPS_COPY_H_ */
