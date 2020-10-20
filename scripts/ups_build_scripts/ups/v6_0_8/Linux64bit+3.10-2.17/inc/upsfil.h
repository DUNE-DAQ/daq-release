/************************************************************************
 *
 * FILE:
 *       upsfil.h
 * 
 * DESCRIPTION: 
 *       Will read an ups file, and fill corresponding data structures.
 *
 * AUTHORS:
 *       Eileen Berman
 *       David Fagan
 *       Lars Rasmussen
 *
 *       Fermilab Computing Division
 *       Batavia, Il 60510, U.S.A.                                              *
 * MODIFICATIONS:
 *       07-jul-1997, LR, first
 *
 ***********************************************************************/

#ifndef _UPSFIL_H_
#define _UPSFIL_H_

/* standard include files, if needed for .h file */
#include <stdio.h>

/* ups specific include files, if needed for .h file */
#include "upstyp.h"
#include "upstbl.h"

/* public typdef's */

/*
 * Declaration of public functions
 */

const char        *upsfil_last_file( void );
t_upstyp_product  *upsfil_read_file( const char * const ups_file );
int               upsfil_write_file( t_upstyp_product * const prod_ptr,
				     const char * const ups_file,
				     const char copt,
				     const int journal );
int               upsfil_write_journal_files( void );
int               upsfil_clear_journal_files( void );
int               upsfil_exist( const char * const ups_file );
void              upsfil_flush( void );
void              upsfil_stat( const int iopt );
t_upstyp_product  *upsfil_is_in_cache( const char * const ups_file );
t_upstbl          *g_file_cache( void );
void              g_print_product( t_upstyp_product * const prod_ptr );

/*
 * Declarations of public variables
 */

#define VERSION_SUFFIX  ".version"
#define CHAIN_SUFFIX    ".chain"
#define UPS_FILES       ".upsfiles"
#define UPS_FILES_LEN   9
#define CONFIG_FILE     "dbconfig"
#define CONFIG_FILE_LEN 8
#define CONFIG_SIZE     UPS_FILES_LEN + CONFIG_FILE_LEN + 1

/* enum of known file types (changes here should be reflected in cfilei) */
enum e_ups_file {
  e_file_version = 0,
  e_file_table,
  e_file_chain,
  e_file_dbconfig,
  e_file_unknown,
  e_file_count
};


#endif /* _UPSFIL_H_ */







