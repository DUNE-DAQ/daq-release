/************************************************************************
 *
 * FILE:
 *       ups_xyz.h
 * 
 * DESCRIPTION: 
 *       The following has never seen a compiler.
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
 *       dd-mmm-yyyy, NN, first
 *
 ***********************************************************************/

#ifndef _UPS_XYZ_H_
#define _UPS_XYZ_H_

/* standard include files, if needed for .h file */
#include <...>

/* ups specific include files, if needed for .h file */
#include "..."

/*
 * Constans.
 */

#define UPS_MAX_FILES 1024

/*
 * Types.
 */
typedef struct t_uxyz_product
{
  ...
} t_uxyz_product;


/*
 * Declaration of public functions.
 */

<type> uxyz_<name>( <type> <name> ... );

/*
 * Declaration of private globals.
 */

<type> p_<name>( <type> <name> ... );

/*
 * Declarations of public variables.
 */

extern <type> uxyz_<name>;


#endif /* _UPS_XYZ_H_ */
