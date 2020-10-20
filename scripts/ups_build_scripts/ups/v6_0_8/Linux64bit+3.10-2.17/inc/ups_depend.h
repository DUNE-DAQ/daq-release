/************************************************************************
 *
 * FILE:
 *       ups_depend.h
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

#ifndef _UPS_DEPEND_H_
#define _UPS_DEPEND_H_

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
t_upslst_item *ups_depend( t_upsugo_command * const u_cmd, 
			   const char * const s_cmd,
			   const int e_cmd );
/*
 * Declaration of private globals.
 */

/*
 * Declarations of public variables.
 */


#endif /* _UPS_DEPEND_H_ */
