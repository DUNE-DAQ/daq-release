/************************************************************************
 *
 * FILE:
 *       upsmat.h
 * 
 * DESCRIPTION: 
 *       Prototypes etc., for instance matching
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
 *       30-Jul-1997, EB, first
 *
 ***********************************************************************/

#ifndef _UPSMAT_H_
#define _UPSMAT_H_

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
t_upslst_item *upsmat_instance(t_upsugo_command * const a_command_line,
			       const t_upslst_item * const a_db_info,
			       const int a_need_unique);
t_upstyp_instance *upsmat_version(t_upstyp_instance * const a_inst,
				  const t_upstyp_db * const a_db_info);
t_upslst_item *upsmat_match_with_instance(
				    const t_upstyp_instance * const a_instance,
				    const t_upstyp_product * const a_product);

/*
 * Declaration of private globals.
 */


/*
 * Declarations of public variables.
 */


#endif /* _UPSMAT_H_ */
