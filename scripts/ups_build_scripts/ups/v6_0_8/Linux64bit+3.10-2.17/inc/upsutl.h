/************************************************************************
 *
 * FILE:
 *       upsutl.h
 * 
 * DESCRIPTION: 
 *       Utility routine definitions
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
 *       28-Jul-1997, EB, first
 *       13-Aug-1997, LR, added string handling upsutl_str_*
 *       26-Aug-1997, LR, fixed bug in str_sort.
 *                        added option 'p' to str_create.
 *                        added function str_crecat.
 *       11-Sep-1997, LR, added function str_stricmp, case insensitive
 *                        string comparison. It's a copy of strcasecmp.
 *       24-Sep-1997, LR, added function str_strincmp, like str_stricmp,
 *                        except it will compare most n characters.
 *
 ***********************************************************************/

#ifndef _UPSUTL_H_
#define _UPSUTL_H_

/* standard include files, if needed for .h file */
#include <sys/types.h>

/* ups specific include files, if needed for .h file */
#include "upstyp.h"
#include "upslst.h"
#include "upsugo.h"

/*
 * Constans.
 */
#define DIVIDER  "#####################################################################"

#define STR_TRIM 't'
#define STR_TRIM_PACK 'p'
#define STR_TRIM_DEFAULT ' '
#define UPS_SEPARATOR ":"
#define MUST_EXIST 1
#define NOT_EXIST  0

/*
 * Types.
 */

/*
 * Declaration of public functions.
 */

void upsutl_unset_upsvars( const FILE * const a_stream,
			   const t_upsugo_command * const a_command_line,
			   const char * const a_prefix);
int upsutl_finish_up(const FILE * const a_stream, const int a_shell,
		      const int a_command_index, const int a_simulate_flag);
void upsutl_finish_temp_file( const FILE * const a_stream,
			      const t_upsugo_command * const a_command_line,
			      const char * const a_prefix);
int upsutl_is_authorized( const t_upstyp_matched_instance * const a_minst,
			  const t_upstyp_db * const a_db_info,
			  char ** const a_node);
char *upsutl_get_hostname( void );
t_upstyp_product *upsutl_get_config( const char * const a_db);
t_upstyp_config *upsutl_free_config(
				   const t_upstyp_config * const a_db_config);
t_upslst_item *upsutl_free_matched_product_list(
			   t_upslst_item ** const a_mproduct_list);
t_upslst_item *upsutl_free_matched_instance_list(
					   t_upslst_item ** const a_inst_list);
t_upslst_item *upsutl_free_inst_list( t_upslst_item ** const a_inst_list);
void upsutl_get_files(const char * const a_dir,
		      const char * const a_pattern,
		      t_upslst_item ** const a_file_list);
char *upsutl_get_prod_dir(const char * const a_db,
			  const char * const a_prod_name);
void upsutl_start_timing(void);
void upsutl_stop_timing(void);
void upsutl_statistics(t_upslst_item const * const a_mproduct,
		       char const * const a_command);
char *upsutl_time_date(const int a_flag);
char *upsutl_user(void);

int   upsutl_stricmp( const char *s1, const char *s2 );
int   upsutl_strincmp( const char *s1, const char *s2, const size_t n );
char  *upsutl_strstr( const char * const a_str, const char * const a_pattern);
char  *upsutl_upcase(const char * const a_str);
char  *upsutl_str_create( char * const str, const char copt );
char  *upsutl_str_crecat( char * const str1, char * const str2 );
int    upsutl_str_sort( char * const, const char );
size_t upsutl_str_remove( char * const str, const char * const ct );
size_t upsutl_str_remove_edges( char * const str, const char * const ct );
size_t upsutl_str_remove_end_quotes( char * str, char * const quotes, char * const wspaces );
int    upsutl_is_a_file(const char * const a_filename);
int    upsutl_is_a_dir(const char * const a_pathname);


/*
 * Declaration of private globals.
 */


/*
 * Declarations of public variables.
 */

#endif /* _UPSUTL_H_ */

