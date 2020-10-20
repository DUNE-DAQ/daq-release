/************************************************************************
 *
 * FILE:
 *       ups_verify.h
 * 
 * DESCRIPTION: 
 *       Prototypes etc., for verifying
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
 *       25-Mar-1998, EB, first
 *
 ***********************************************************************/

#ifndef _UPS_VERIFY_H_
#define _UPS_VERIFY_H_

/* standard include files, if needed for .h file */

/* ups specific include files, if needed for .h file */
#include "upsugo.h"
#include "upstyp.h"

/*
 * Constans.
 */

/*
 * Types.
 */

/*
 * Declaration of public functions.
 */
void ups_verify_matched_instance(const t_upstyp_db * const a_db,
			      const t_upstyp_matched_instance * const a_minst,
			      const t_upsugo_command * const a_command_line,
			      const char * const a_product_name);
void ups_verify_dbconfig(const t_upstyp_db * const a_db,
			 const t_upstyp_matched_instance * const a_minst,
			 const t_upsugo_command * const a_command_line);

/*
 * Declaration of private globals.
 */


/*
 * Declarations of public variables.
 */


#endif /* _UPS_VERIFY_H_ */
