/************************************************************************
 *
 * FILE:
 *       ups_modify.h
 * 
 * DESCRIPTION: 
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
 *       Tue feb 20, Djf, First
 ***********************************************************************/

#ifndef _UPS_MODIFY_H_
#define _UPS_MODIFY_H_

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

t_upslst_item *ups_modify(t_upsugo_command * const a_command_line,
			  const FILE * const a_temp_file,
			  const int a_ups_command);


#endif /* _UPS_MODIFY_H_ */
