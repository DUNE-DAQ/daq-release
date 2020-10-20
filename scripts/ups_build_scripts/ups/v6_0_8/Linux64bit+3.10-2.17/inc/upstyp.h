/***********************************************************************
 *
 * FILE:
 *       upstyp.h
 * 
 * DESCRIPTION: 
 *       Common types for ups
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
 *       22-Jul-1997, LR, first, very preliminary
 *       25-Jul-1997, DjF, added command line inputs
 *       29-Jul-1997, LR, New 'unified' t_upstyp_instance structure.
 *                        Added function declarations for creating
 *                        and destroying common types.
 *       30-Jul-1997, LR, Added 'char *db_dir' to instance structure.
 *       13-Aug-1997, LR, Added MAX_LINE_LENGTH.
 *       28-Aug-1997, DjF, added upsugo_env - Build ups_command structure
 *                         from the product environment variable given
 *                         product name.
 *       08-Sep-1997, LR, Added constant INVALID_INDEX.
 *       28-Oct-1997, EB, Add t_upstyp_db
 *
 ***********************************************************************/

#ifndef _UPSTYP_H_
#define _UPSTYP_H_

/*
 * Standard include files, if needed for .h file
 */

/*
 * ups specific include files, if needed for .h file
 */
#include "upslst.h"

/*
 * Public typdef's
 */

/* a db config file */
typedef struct upstyp_config {
  char             *ups_db_version;
  char             *prod_dir_prefix;
  char             *authorized_nodes;
  char             *statistics;
  char             *man_target_dir;
  char             *catman_target_dir;
  char             *info_target_dir;
  char             *html_target_dir;
  char             *news_target_dir;
  char             *upd_usercode_dir;
  char             *setups_dir;
  char             *version_subdir;
  char		   *strict_deps;
} t_upstyp_config;

/* database information.  this includes the name (location) of the database
   as returned by the upsugo routines and a place for the read in configuration
   information (filled in by upsmat) */
typedef struct upstyp_db
{
  char             *name;
  t_upstyp_config  *config;
} t_upstyp_db;

/* a matched ups product. a list of all the instances that could pertain to
   the product/ */
typedef struct upstyp_matched_product
{
  t_upstyp_db      *db_info;
  char             *product;
  
  t_upslst_item    *minst_list;
} t_upstyp_matched_product;

/* a product instance */
typedef struct upstyp_instance
{
  char             *product;
  char             *version;
  char             *flavor;
  char             *qualifiers;
  
  char             *chain;
  char             *declarer;
  char             *declared;
  char             *modifier;
  char             *modified;
  char             *origin;
  char             *prod_dir;
  char             *ups_dir;
  char             *table_dir;
  char             *table_file;
  char             *archive_file;
  char             *authorized_nodes;
  char             *description;  
  char             *statistics;
  char             *compile_dir;
  char             *compile_file;
  char             *catman_source_dir;
  char             *html_source_dir;
  char             *info_source_dir;
  char             *man_source_dir;
  char             *news_source_dir;
  
  char             *db_dir;  
  
  t_upslst_item    *action_list;

  t_upslst_item    *user_list;

  struct upstyp_instance *sav_inst;
  int              flag;       /* used by verify in upsmat */

} t_upstyp_instance;

/* a matched instance */
typedef struct upstyp_matched_instance
{
  t_upstyp_instance *chain;
  t_upstyp_instance *version;
  t_upstyp_instance *table;

  t_upslst_item *xtra_chains;

  int invalid_instance;
  int orig_slot;
} t_upstyp_matched_instance;

/* an action */
typedef struct upstyp_action
{
  char             *action;
  t_upslst_item    *command_list;
} t_upstyp_action;

/* any ups file */
typedef struct upstyp_product
{
  char             *file;
  char             *product;
  char             *version;
  char             *chain;
  char             *ups_db_version;
  char             *description;  
  t_upslst_item    *user_list;
  
  t_upslst_item    *instance_list;
  t_upslst_item    *comment_list;
  t_upstyp_config  *config;

  int              journal;
} t_upstyp_product;

/* 
 * Public variables
 */

/* check if the first character matches ANY_MATCH. NOTE: the value of ANY_MATCH
   and the value subtracted in NOT_EQUAL_ANY_MATCH must be the same */
#define ANY_MATCH "*"
#define NOT_EQUAL_ANY_MATCH(string) (*(string) - '*')

#define ANY_FLAVOR "ANY"
#define MAX_LINE_LEN 1024
#define INVALID_INDEX -1

#define JOURNAL 1
#define NOJOURNAL 0

#define UPSRELATIVE(dir) ((dir && (dir[0] && (dir[0] != '/'))))

/*
 * Declaration of public functions.
 */
t_upstyp_matched_product *ups_new_matched_product(
				     const t_upstyp_db * const a_db_info,
			  	     const char * const a_prod_name,
			             const t_upslst_item * const a_minst_list);
t_upstyp_matched_product *ups_free_matched_product(
				 t_upstyp_matched_product * const a_mproduct);
t_upstyp_matched_instance *ups_new_matched_instance( void );
t_upstyp_matched_instance *ups_free_matched_instance(
                                 t_upstyp_matched_instance * const minst_ptr);
t_upstyp_matched_instance *ups_free_matched_instance_structure(
                                 t_upstyp_matched_instance * const minst_ptr);
t_upstyp_product  *ups_new_product( void );
int               ups_free_product( t_upstyp_product * const prod_ptr );
t_upstyp_instance *ups_new_instance( void );
int               ups_free_instance( t_upstyp_instance * const inst_ptr );
t_upstyp_action   *ups_new_action( void );
int               ups_free_action( t_upstyp_action * const act_ptr );
t_upstyp_config   *ups_new_config( void );
int               ups_free_config( t_upstyp_config * const conf_ptr );


#endif /* _UPSTYP_H_ */
