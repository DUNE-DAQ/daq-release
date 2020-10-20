/************************************************************************
 *
 * FILE:
 *       ups_unk.h
 * 
 * DESCRIPTION: 
 *       This is the unknown command handler
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
 *       16-Oct-1997, EFB, first
 *
 ***********************************************************************/

#ifndef _UPS_UNK_H_
#define _UPS_UNK_H_

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

t_upslst_item * ups_unk(const t_upsugo_command * const a_command_line,
			const FILE * const a_temp_file,
			const char * const a_unk_cmd);

/*
 * Declaration of private globals.
 */

/*
 * Declarations of public variables.
 */

#endif /* _UPS_UNK_H_ */
