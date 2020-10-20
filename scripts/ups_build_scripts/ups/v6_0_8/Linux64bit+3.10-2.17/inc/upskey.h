/***********************************************************************
 *
 * FILE:
 *       upskey.h
 * 
 * DESCRIPTION: 
 *       Translations for ups keys
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
 *       25-aug-1997, LR, first
 *
 ***********************************************************************/

#ifndef _UPSKEY_H_
#define _UPSKEY_H_

/*
 * Standard include files, if needed for .h file
 */

/*
 * ups specific include files, if needed for .h file
 */
#include "upstyp.h"

/*
 * Constants.
 */

/* enum of known keys (changes here should be reflected in g_ups_keys) */
enum e_upskey_key {
  e_key_file = 0,
  e_key_product,
  e_key_version,
  e_key_chain,
  e_key_ups_db_version,

  /* it's importend that the above keys are all part of a 
     file descriptor, and the keys below are not */

  e_key_flavor,
  e_key_qualifiers,

  /* it's improtend that flavor and qualifier is the first instance
     keys, which is not part of the file descriptor */

  e_key_declarer,
  e_key_declared,
  e_key_modifier,
  e_key_modified,
  e_key_origin,
  e_key_prod_dir,
  e_key_ups_dir,
  e_key_table_dir,
  e_key_table_file,
  e_key_archive_file,
  e_key_authorized_nodes,
  e_key_description,
  e_key_statistics,
  e_key_compile_dir,
  e_key_compile_file,
  e_key_catman_source_dir,
  e_key_html_source_dir,
  e_key_info_source_dir,
  e_key_man_source_dir,
  e_key_news_source_dir,

  e_key_db_dir,
  e_key_action,
  e_key_user_defined,

  e_key_prod_dir_prefix,
  e_key_man_target_dir,
  e_key_catman_target_dir,
  e_key_info_target_dir,
  e_key_news_target_dir,
  e_key_html_target_dir,
  e_key_upd_usercode_dir,
  e_key_setups_dir,
  e_key_version_subdir,
  e_key_strict_deps,
  
  /* it's importend that the keys below are not part of
     any ups product or instance or like */

  e_key_group,
  e_key_common,
  e_key_end,

  e_key_osalias,

  e_key_unknown,
  e_key_count
};

/*
 * Types.
 */

typedef struct upskey_map {
  int           ikey;    /* enum of key (corresponding to e_upskey_key) */
  char          *key;    /* string of key */
  int           p_index; /* index into product structure (t_upstyp_product) */
  int           i_index; /* index into instance structure (t_upstyp_instance) */
  int           c_index; /* index into config structure (t_upstyp_config) */
  unsigned int  flag;    /* flags, see ups_keys.c */
} t_upskey_map;

typedef union upskey_product_u {
  t_upstyp_product	prod;
  char   		*arr[e_key_count];
} t_upskey_product_u;

typedef union upskey_instance_u {
  t_upstyp_instance 	inst;
  char           	*arr[e_key_count];
} t_upskey_instance_u;

typedef union upskey_config_u {
  t_upstyp_config   	conf;
  char          	*arr[e_key_count];
} t_upskey_config_u;

/* some convenient macros */

#define UPSKEY_PROD2ARR( prod ) (((t_upskey_product_u *)prod)->arr)
#define UPSKEY_INST2ARR( inst ) (((t_upskey_instance_u *)inst)->arr)
#define UPSKEY_CONF2ARR( conf ) (((t_upskey_config_u *)conf)->arr)

#define UPSKEY_ISIN_VERSION( flag ) (flag&0x00000001 ? 1 : 0)
#define UPSKEY_ISIN_TABLE( flag )   (flag&0x00000010 ? 1 : 0)
#define UPSKEY_ISIN_CHAIN( flag )   (flag&0x00000100 ? 1 : 0)
#define UPSKEY_ISIN_CONFIG( flag )  (flag&0x00001000 ? 1 : 0)
#define UPSKEY_TRY_TRANSLATE( flag )  (flag&0x00010000 ? 1 : 0)

/*
 * Declaration of public functions.
 */
t_upskey_map *upskey_get_info( const char * const skey );
char         *upskey_inst_getval( t_upstyp_instance * const inst,
				  const char * const skey );
char         *upskey_inst_setval( t_upstyp_instance * const inst,
				  const char * const skey,
				  const char * const sval );
t_upstyp_action *upskey_inst_getaction( t_upstyp_instance * const inst,
				     const char * const action_name );
char         *upskey_inst_getuserval(t_upstyp_instance * const inst,
				  const char * const skey );
int          *upskey_verhead_arr(void);
int          *upskey_verinst_arr(void);
int          *upskey_tblhead_arr(void);
int          *upskey_tblinst_arr(void);
int          *upskey_chnhead_arr(void);
int          *upskey_chninst_arr(void);
int          *upskey_inst_arr(void);
int          *upskey_prod_arr(void);
void         upskey_inst_print( const t_upstyp_instance * const prod );

char         *upskey_prod_getval( t_upstyp_product * const prod,
				  const char * const skey );
char         *upskey_prod_setval( t_upstyp_product * const prod,
				  const char * const skey,
				  const char * const sval );
void         upskey_prod_print( const t_upstyp_product * const prod );
void         upskey_conf_print( const t_upstyp_config * const conf );
     
/*
 * Declarations of public variables.
 */

#endif /* _UPSKEY_H_ */
