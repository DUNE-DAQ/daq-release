/***********************************************************************
 *
 * FILE:
 *       upskey.c
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
/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* ups specific include files */
#include "upstyp.h"
#include "upsmem.h"
#include "upsutl.h"
#include "upskey.h"

/*
 * Definition of public variables
 */

/*
 * Declaration of private functions
 */

/*
 * Definition of global variables
 */
#define NO INVALID_INDEX

/* this array is used for returning a valid list of key enums */
static int g_ikeys[ e_key_count+1 ];

/*
 * g_key_info.
 *
 * It goes like: 'enum key', 'string key', 'index to product structure',
 *               'index to instance structure', 'index into config structure',
 *               'flag'.
 * where flag:
 * <byte>: <description>
 *   0   : key can be in a version file
 *   1   : key can be in a table file
 *   2   : key can be in a chain file
 *   3   : key can be in a config file
 *   4   : upsfil will translate env. variables in the key value
 *   5   : spare
 *   6   : spare
 *   7   : spare
 *
 *
 * How to add a new keyword/field:
 *   1) add the new keyword to the enums in upskey.h
 *   2) add a corresponding line to the map below.
 *   3) add the new field to t_upstyp_product, _instance ... in upstyp.h
 *   4) free the field in ups_free_product, _instance ...
 *   5) only if the new keyword affect the file description (the head of a ups 
 *      file, like FILE and PRODUCT) be sure the file header writing control 
 *      routines are doing the right thing: upskey_*head_arr.
 *
 *
 * Note: The location of the keys from FILE to QUALIFIERS and from GROUP: to END:
 *       is kind of importend for the reading and writing functions.
 *
 */
t_upskey_map g_key_info[] =
{
  { e_key_file,             "FILE",             0,    NO,   NO, 0x00001111 },
  { e_key_product,          "PRODUCT",          1,     0,   NO, 0x00000111 },
  { e_key_version,          "VERSION",          2,     1,   NO, 0x00000111 }, 
  { e_key_chain,            "CHAIN",            3,     4,   NO, 0x00000110 },
  { e_key_ups_db_version,   "UPS_DB_VERSION",   4,    NO,    0, 0x00001111 },

  { e_key_flavor,           "FLAVOR",           NO,    2,   NO, 0x00000111 },
  { e_key_qualifiers,       "QUALIFIERS",       NO,    3,   NO, 0x00000111 },
  { e_key_declarer,         "DECLARER",         NO,    5,   NO, 0x00000101 },
  { e_key_declared,         "DECLARED",         NO,    6,   NO, 0x00000101 },
  { e_key_modifier,         "MODIFIER",         NO,    7,   NO, 0x00000101 },
  { e_key_modified,         "MODIFIED",         NO,    8,   NO, 0x00000101 },
  { e_key_origin,           "ORIGIN",           NO,    9,   NO, 0x00000001 },
  { e_key_prod_dir,         "PROD_DIR",         NO,   10,   NO, 0x00010001 },
  { e_key_ups_dir,          "UPS_DIR",          NO,   11,   NO, 0x00010001 },
  { e_key_table_dir,        "TABLE_DIR",        NO,   12,   NO, 0x00010001 },
  { e_key_table_file,       "TABLE_FILE",       NO,   13,   NO, 0x00000001 },
  { e_key_archive_file,     "ARCHIVE_FILE",     NO,   14,   NO, 0x00010001 },
  { e_key_authorized_nodes, "AUTHORIZED_NODES", NO,   15,    2, 0x00001001 },
  { e_key_description,      "DESCRIPTION",       5,   16,   NO, 0x00000111 },
  { e_key_statistics,       "STATISTICS",       NO,   17,    3, 0x00001001 },
  { e_key_compile_dir,      "COMPILE_DIR",      NO,   18,   NO, 0x00010001 },
  { e_key_compile_file,     "COMPILE_FILE",     NO,   19,   NO, 0x00000001 },
  { e_key_catman_source_dir,"CATMAN_SOURCE_DIR",NO,   20,   NO, 0x00000010 },
  { e_key_html_source_dir,  "HTML_SOURCE_DIR",  NO,   21,   NO, 0x00000010 },
  { e_key_info_source_dir,  "INFO_SOURCE_DIR",  NO,   22,   NO, 0x00000010 },
  { e_key_man_source_dir,   "MAN_SOURCE_DIR",   NO,   23,   NO, 0x00000010 },
  { e_key_news_source_dir,  "NEWS_SOURCE_DIR",  NO,   24,   NO, 0x00000010 },

  { e_key_db_dir,           "DB_DIR",           NO,   25,   NO, 0x00010000 },
  { e_key_action,           "ACTION",           NO,   26,   NO, 0x00000010 },
  { e_key_user_defined,     "USER",              6,   27,   NO, 0x00001111 },

  { e_key_prod_dir_prefix,  "PROD_DIR_PREFIX",  NO,   NO,    1, 0x00011000 },
  { e_key_man_target_dir,   "MAN_TARGET_DIR",   NO,   NO,    4, 0x00011000 },
  { e_key_catman_target_dir,"CATMAN_TARGET_DIR",NO,   NO,    5, 0x00011000 },
  { e_key_info_target_dir,  "INFO_TARGET_DIR",  NO,   NO,    6, 0x00011000 },
  { e_key_html_target_dir,  "HTML_TARGET_DIR",  NO,   NO,    7, 0x00011000 },
  { e_key_news_target_dir,  "NEWS_TARGET_DIR",  NO,   NO,    8, 0x00011000 },
  { e_key_upd_usercode_dir, "UPD_USERCODE_DIR", NO,   NO,    9, 0x00011000 },
  { e_key_setups_dir,       "SETUPS_DIR",       NO,   NO,   10, 0x00011000 },
  { e_key_version_subdir,   "VERSION_SUBDIR",   NO,   NO,   11, 0x00011000 },
  { e_key_strict_deps,      "STRICT_DEPS",      NO,   NO,   12, 0x00011000 },

  { e_key_group,            "GROUP:",           NO,   NO,   NO, 0x00000010 },
  { e_key_common,           "COMMON:",          NO,   NO,   NO, 0x00000010 },
  { e_key_end,              "END:",             NO,   NO,   NO, 0x00000010 },

  { e_key_unknown,0,0,0,0 },
  { e_key_count,0,0,0,0 },
};

/*
 * Definition of public functions
 */

/*-----------------------------------------------------------------------
 * upskey_get_info
 *
 * Will return a map (t_upskey_map) for passed key.
 *
 * Input : char *, string of key.
 * Output: none.
 * Return: t_upskey_map *, corresponding map, or 0.
 */
t_upskey_map *upskey_get_info( const char * const str )
{
  t_upskey_map *keys;

  /* for now, just a linear search */
  
  for ( keys = g_key_info; keys->key; keys++ ) {
    if ( !upsutl_stricmp( keys->key, str ) )
      return keys;
  }

  return 0;
}

/*-----------------------------------------------------------------------
 * upskey_inst_getval
 *
 * Will return an instance value corresponding to passed key
 *
 * Input : t_upstyp_instance *, an instance.
 *         char *, string of key.
 * Output: none.
 * Return: char *, an instance value, or 0.
 */
char *upskey_inst_getval( t_upstyp_instance * const inst, const char * const skey )
{
  t_upskey_map *key = upskey_get_info( skey );
  if ( key && key->i_index != NO )
    return UPSKEY_INST2ARR( inst )[key->i_index];
  return 0;  
}

/*-----------------------------------------------------------------------
 * upskey_inst_setval
 *
 * Will set an instance value corresponding to passed key.
 *
 * Input : t_upstyp_instance *, an instance.
 *         char *, string of key.
 *         char *, value.
 * Output: none.
 * Return: char *, return value, or 0.
 */
char *upskey_inst_setval( t_upstyp_instance * const inst,
			  const char * const skey, const char * const sval )
{
  t_upskey_map *key = upskey_get_info( skey );
  if ( key && key->i_index != NO ) {
    char *new_val = (char *)upsmem_malloc( (int)strlen( sval ) + 1 );
    (void) strcpy( new_val, sval );
    return ( UPSKEY_INST2ARR( inst )[key->i_index] = new_val );
  }
  return 0;  
}

/*-----------------------------------------------------------------------
 * upskey_inst_getaction
 *
 * Will return from an instance, the action with the passed name
 *
 * Input : t_upstyp_instance *, an instance.
 *         char *, action name
 * Output: none.
 * Return: t_upstyp_action *, a pointer to an action or 0.
 */
t_upstyp_action *upskey_inst_getaction( t_upstyp_instance * const inst,
				     const char * const action_name )
{
  t_upstyp_action *act_ptr = 0;
  t_upslst_item *act_list = 0;

  if ( !action_name || !inst || !inst->action_list )
    return 0;

  act_list = upslst_first( inst->action_list );
  for ( ; act_list; act_list = act_list->next ) {
    t_upstyp_action *act = (t_upstyp_action *)act_list->data;
    if ( !upsutl_stricmp( action_name, act->action ) ) {
      act_ptr = act;
      break;
    }
  }

  return act_ptr;
}

/*-----------------------------------------------------------------------
 * upskey_inst_getuserval
 *
 * Will return from an instance, the value defined by the passed user key.
 * If user key is defined but have no value, it will return a pointer
 * to a static zero length string.
 * If user key is not defined, it will return 0.
 *
 * Input : t_upstyp_instance *, an instance.
 *         char *, action name
 * Output: none.
 * Return: char *, a pointer to a string or 0.
 */
char *upskey_inst_getuserval( t_upstyp_instance * const inst,
			      const char * const skey )
{
  static char *sany = "";
  t_upslst_item *usr_l = 0;
  char *sp_d = 0;
  char *sp_e = 0;
  size_t len = 0;

  if ( !inst || !inst->user_list || !skey ) 
    return 0;

  len = strlen( skey );
  usr_l = upslst_first( inst->user_list );
  for ( ; usr_l; usr_l = usr_l->next ) {
    sp_d = (char *)usr_l->data;

    /* we can be strict with the format, since we are passing an
       already passed and rewritten user key/val */
    
    if ( (sp_e = strstr( sp_d, " = " )) ) {
      if ( (len == (size_t )(sp_e - sp_d)) && 
	   !upsutl_strincmp( sp_d, skey, len ) )
	return (sp_e + 3);
    }
    else if ( (len == strlen( sp_d )) && 
	      !upsutl_strincmp( sp_d, skey, len ) ) {
      return sany;	      
    }
  }

  return 0;
}

void upskey_inst_print( const t_upstyp_instance * const inst )
{
  t_upskey_map *keys;
  int ix;
  
  if ( !inst ) return;
  
  for ( keys = g_key_info; keys->key; keys++ ) {
    if ( (ix = keys->i_index) != NO && UPSKEY_INST2ARR( inst )[ix] )
      (void) printf( "%s = %s\n", keys->key, UPSKEY_INST2ARR( inst )[ix] );    
  }
}

/*-----------------------------------------------------------------------
 * upskey_prod_getval
 *
 * Will return a product value corresponding to passed key
 *
 * Input : t_upstyp_product *, a product.
 *         char *, string of key.
 * Output: none.
 * Return: char *, a product value, or 0.
 */
char *upskey_prod_getval( t_upstyp_product * const prod, const char * const skey )
{
  t_upskey_map *key = upskey_get_info( skey );
  if ( key && key->p_index != NO )
    return UPSKEY_PROD2ARR( prod )[key->p_index];
  return 0;  
}

/*-----------------------------------------------------------------------
 * upskey_prod_setval
 *
 * Will set a product value corresponding to passed key.
 *
 * Input : t_upstyp_product *, a product.
 *         char *, string of key.
 *         char *, value.
 * Output: none.
 * Return: char *, return value, or 0.
 */
char *upskey_prod_setval( t_upstyp_product * const prod,
			  const char * const skey, const char * const sval )
{
  t_upskey_map *key = upskey_get_info( skey );
  if ( key && key->p_index != NO ) {
    char *new_val = (char *)upsmem_malloc( (int)strlen( sval ) + 1 );
    (void) strcpy( new_val, sval );
    return ( UPSKEY_PROD2ARR( prod )[key->p_index] = new_val );
  }
  return 0;  
}

/*
 * The following six or seven routines will return an array of key enums 
 * for valid keys as a file descriptor (head) or as an instance in ups files 
 * (version, table, chain). Mainly used by the upsfil writing routines.
 */

int *upskey_prod_arr(void)
{
  t_upskey_map *keys = &g_key_info[ 0 ];
  int i = 0;

  for ( ; keys->key; keys++ )
    if ( keys->p_index != NO )
      g_ikeys[i++] = keys->ikey;

  g_ikeys[i] = -1;

  return g_ikeys;
}

int *upskey_inst_arr(void)
{
  t_upskey_map *keys = &g_key_info[ 0 ];
  int i = 0;

  for ( ; keys->key; keys++ )
    if ( keys->i_index != NO )
      g_ikeys[i++] = keys->ikey;

  g_ikeys[i] = -1;

  return g_ikeys;
}

int *upskey_verhead_arr(void)
{
  
  /* tsk, tsk */

  g_ikeys[0] = e_key_file;
  g_ikeys[1] = e_key_product;
  g_ikeys[2] = e_key_version;
  g_ikeys[3] = e_key_ups_db_version;
  g_ikeys[4] = e_key_description;
  g_ikeys[5] = e_key_user_defined;
  g_ikeys[6] = -1;

  return g_ikeys;
}

int *upskey_verinst_arr(void)
{
  t_upskey_map *keys = &g_key_info[ e_key_flavor ];
  int i = 0;

  for ( ; keys->key && keys->ikey < e_key_group; keys++ ) {
    if ( UPSKEY_ISIN_VERSION( keys->flag ) )
      g_ikeys[i++] = keys->ikey;
  }
  g_ikeys[i] = -1;

  return g_ikeys;
}

int *upskey_tblhead_arr(void)
{

  /* tsk, tsk */

  g_ikeys[0] = e_key_file;
  g_ikeys[1] = e_key_product;
  g_ikeys[2] = e_key_version;
  g_ikeys[3] = e_key_ups_db_version;
  g_ikeys[4] = e_key_description;
  g_ikeys[5] = e_key_user_defined;
  g_ikeys[6] = -1;

  return g_ikeys;
}

int *upskey_tblinst_arr(void)
{
  t_upskey_map *keys = &g_key_info[ e_key_flavor ];
  int i = 0;

  for ( ; keys->key && keys->ikey < e_key_group; keys++ ) {
    if ( UPSKEY_ISIN_TABLE( keys->flag ) )
      g_ikeys[i++] = keys->ikey;
  }
  g_ikeys[i] = -1;

  return g_ikeys;
}

int *upskey_chnhead_arr(void)
{

  /* tsk, tsk */

  g_ikeys[0] = e_key_file;
  g_ikeys[1] = e_key_product;
  g_ikeys[2] = e_key_chain;
  g_ikeys[3] = e_key_ups_db_version;
  g_ikeys[4] = e_key_description;
  g_ikeys[5] = e_key_user_defined;
  g_ikeys[6] = -1;

  return g_ikeys;
}

int *upskey_chninst_arr(void)
{
  t_upskey_map *keys = &g_key_info[ e_key_flavor + 1];
  int i = 0;

  g_ikeys[i++] = e_key_flavor;
  g_ikeys[i++] = e_key_version;

  
  for ( ; keys->key && keys->ikey < e_key_group; keys++ ) {
    if ( UPSKEY_ISIN_CHAIN( keys->flag ) )
      g_ikeys[i++] = keys->ikey;
  }
  g_ikeys[i] = -1;

  return g_ikeys;
}

/*
 * Some print routines
 */

void upskey_prod_print( const t_upstyp_product * const prod )
{
  t_upskey_map *keys;
  int ix;
   
  if ( !prod ) return;
  
  for ( (keys = g_key_info); keys->key; keys++ ) {
    if ( (ix = keys->p_index) != NO && UPSKEY_PROD2ARR( prod )[ix] ) 
      (void) printf( "%s = %s\n", keys->key, UPSKEY_PROD2ARR( prod )[ix] );
  }
}

void upskey_conf_print( const t_upstyp_config * const conf )
{
  t_upskey_map *keys;
  int ix;
  
  if ( !conf ) return;
  
  for ( keys = g_key_info; keys->key; keys++ ) {
    if ( (ix = keys->c_index) != NO && UPSKEY_CONF2ARR( conf )[ix] )
      (void) printf( "%s = %s\n", keys->key, UPSKEY_CONF2ARR( conf )[ix] );    
  }
}

/*
 * Definition of private functions
 */
