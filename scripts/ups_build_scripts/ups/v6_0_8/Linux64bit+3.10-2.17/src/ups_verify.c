/************************************************************************
 *
 * FILE:
 *       ups_verify.c
 * 
 * DESCRIPTION: 
 *       Verify information in the ups database
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

/* standard include files */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ups specific include files */
#include "upserr.h"
#include "upsugo.h"
#include "upsutl.h"
#include "upsmat.h"
#include "upsact.h"
#include "upsget.h"
#include "upsmem.h"
#include "upstyp.h"
#include "ups_main.h"
#include "ups_verify.h"

/*
 * Definition of public variables.
 */
extern t_cmd_map g_func_info[];

/*
 * Declaration of private functions.
 */

#define VERIFY_INST_PARAMS  \
      const t_upstyp_instance *const a_inst,            \
      const t_upstyp_db * const a_db,                   \
      const t_upstyp_matched_instance * const a_minst,  \
      const t_upsugo_command * const a_command_line

static void ups_verify_chain_instance(VERIFY_INST_PARAMS);
static void ups_verify_version_instance(VERIFY_INST_PARAMS);
static void ups_verify_table_instance(VERIFY_INST_PARAMS);
static void ups_verify_generic_instance(VERIFY_INST_PARAMS,
					const char * const a_product_name);

static void shutup(VERIFY_INST_PARAMS);		/* pretend to use all of the parameters we've defined */
#define SHUTUP \
  if ((&bit_bucket + bit_bucket_offset == 0) && 0) shutup (a_inst, a_db, a_minst, a_command_line);

/*
 * Definition of global variables.
 */
static char *g_dollarsign = "$";
static long bit_bucket = 0;
static long bit_bucket_offset = 0;

#define VERIFY_DIR_SPEC(dir, tran)   \
    if (dir && (dir[0] != '\0')) {                                          \
      char *trans_dir;                                                      \
      struct stat file_stat;                                                \
      t_upstyp_matched_instance *tran_tmp;                                  \
      tran_tmp = (t_upstyp_matched_instance *) tran;                        \
      if ( tran_tmp )                                                       \
	trans_dir = upsget_translation(a_minst, a_db, a_command_line, dir); \
      else                                                                  \
	trans_dir = dir;                                                    \
      if (strstr(trans_dir, g_dollarsign)) {                                \
        upserr_add(UPS_VERIFY_ENV_VAR, UPS_WARNING, dir);                   \
      }                                                                     \
      if (! stat(trans_dir, &file_stat)) {                                  \
        if (! S_ISDIR(file_stat.st_mode)) {                                 \
	  upserr_add(UPS_NOT_A_DIR, UPS_WARNING, trans_dir);                \
        }                                                                   \
      } else {                                                              \
        upserr_add(UPS_NO_FILE, UPS_WARNING, trans_dir);                    \
      }                                                                     \
    }

#define VERIFY_FILE_SPEC(file)   \
    if (file) {                                                              \
      char *trans_file;                                                      \
      trans_file = upsget_translation(a_minst, a_db, a_command_line, file);  \
      if (strstr(trans_file, g_dollarsign)) {                                \
        upserr_add(UPS_VERIFY_ENV_VAR, UPS_WARNING, file);                   \
      }                                                                      \
      if (upsutl_is_a_file(trans_file) == UPS_NO_FILE) {                     \
        upserr_add(UPS_NO_FILE, UPS_WARNING, trans_file);                    \
      }                                                                      \
    }

#define g_COMMA ","
#define CHECK_FOR_COMMA(keyword)     \
    if (keyword && strstr(keyword, g_COMMA)) {                   \
      upserr_add(UPS_INVALID_SEPARATOR, UPS_WARNING, g_COMMA, keyword);   \
    }

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * ups_verify_dbconfig
 *
 * Verify the information read in from the dbconfig file.
 *
 * Input : information from the dbconfig file, a matched instance and info
 *          from the command line
 * Output: none
 * Return: none
 */
void ups_verify_dbconfig(const t_upstyp_db * const a_db,
			 const t_upstyp_matched_instance * const a_minst,
			 const t_upsugo_command * const a_command_line)
{
  /* verify that each keyword whose value points to a directory, points to a
     valid directory spec */
  if (a_db && a_db->config) {
    /* if we are checking syntax only, do not look for external files */
    if ( a_command_line && ! a_command_line->ugo_S) {
      VERIFY_DIR_SPEC(a_db->config->prod_dir_prefix, a_minst);
      VERIFY_DIR_SPEC(a_db->config->man_target_dir, a_minst);
      VERIFY_DIR_SPEC(a_db->config->catman_target_dir, a_minst);
      VERIFY_DIR_SPEC(a_db->config->info_target_dir, a_minst);
      VERIFY_DIR_SPEC(a_db->config->html_target_dir, a_minst);
      VERIFY_DIR_SPEC(a_db->config->news_target_dir, a_minst);
      VERIFY_DIR_SPEC(a_db->config->upd_usercode_dir, a_minst);
      VERIFY_DIR_SPEC(a_db->config->setups_dir, a_minst);
    }
    /* make sure that these keywords do not contain a comma separated list */
    CHECK_FOR_COMMA(a_db->config->authorized_nodes);
    CHECK_FOR_COMMA(a_db->config->statistics);
  }
}

/*-----------------------------------------------------------------------
 * ups_verify_matched_instance
 *
 * Verify the information in a matched instance.  the following information
 * is not verified here -
 *	           version
 *                 chain
 *                 declarer
 *	           declared
 *	           modifier
 *	           modified
 *	           origin
 *	           prod_dir
 *                 ups_dir
 *                 db_dir
 *                 user_list
 *
 * Input : information from the dbconfig file, a matched instance and info
 *          from the command line and a product name
 * Output: none
 * Return: none
 */
void ups_verify_matched_instance(const t_upstyp_db * const a_db,
			      const t_upstyp_matched_instance * const a_minst,
			      const t_upsugo_command * const a_command_line,
			      const char * const a_product_name)
{
  if (a_minst) {
    if (a_minst->chain) {
      ups_verify_chain_instance(a_minst->chain, a_db, a_minst, 
				a_command_line);
    }
    if (a_minst->version) {
      ups_verify_generic_instance(a_minst->version, a_db, a_minst,
				  a_command_line, a_product_name);
      ups_verify_version_instance(a_minst->version, a_db, a_minst, 
				  a_command_line);
    }
    if (a_minst->table) {
      ups_verify_generic_instance(a_minst->table, a_db, a_minst,
				  a_command_line, a_product_name);
      ups_verify_table_instance(a_minst->table, a_db, a_minst, a_command_line);
    }
  }
}
/*
 * Definition of private globals.
 */

/*
 * Definition of private functions.
 */

/*-----------------------------------------------------------------------
 * ups_verify_chain_instance
 *
 * Verify the information in a chain instance
 *
 * Input : the instance to verify, information from the dbconfig file,
 *          a matched instance and info
 *          from the command line and a product name
 * Output: none
 * Return: none
 */
static void ups_verify_chain_instance(VERIFY_INST_PARAMS)
{
  /* the following elements of an instance structure are verified here -
	       flavor
	       qualifiers
	       version
   */
  
  /* make sure that the value of flavor is not any */
  if (! strcmp(a_inst->flavor, ANY_FLAVOR)) {
    upserr_add(UPS_INVALID_ANY_FLAVOR, UPS_WARNING, "CHAIN");
  }
  /* make sure that the value of qualifiers is not any */
  if (! strcmp(a_inst->qualifiers, ANY_FLAVOR)) {
    upserr_add(UPS_INVALID_ANY_QUALS, UPS_INFORMATIONAL, "CHAIN");
  }
  SHUTUP;
}
/*-----------------------------------------------------------------------
 * ups_verify_version_instance
 *
 * Verify the information in a version instance
 *
 * Input : the instance to verify, information from the dbconfig file,
 *          a matched instance and info
 *          from the command line and a product name
 * Output: none
 * Return: none
 */
static void ups_verify_version_instance(VERIFY_INST_PARAMS)
{
  char *bufPtr, *noftp_archive_file;

  /* the following elements of an instance structure are verified here -
	       table_dir
	       table_file
	       archive_file
	       compile_dir
	       compile_file	       
   */
  
  /* make sure that we do not have a directory for things and not a filename */
  if (a_inst->table_dir && (!a_inst->table_file)) {
    upserr_add(UPS_VERIFY_TABLE_DIR, UPS_WARNING, a_inst->version);
  }
  if (a_inst->compile_dir && (!a_inst->compile_file)) {
    upserr_add(UPS_VERIFY_COMPILE_DIR, UPS_WARNING, a_inst->version);
  }
  /* make sure that the value of flavor is not any */
  if (! strcmp(a_inst->flavor, ANY_FLAVOR)) {
    upserr_add(UPS_INVALID_ANY_FLAVOR, UPS_WARNING, "VERSION");
  }
  /* make sure that the value of qualifiers is not any */
  if (! strcmp(a_inst->qualifiers, ANY_FLAVOR)) {
    upserr_add(UPS_INVALID_ANY_QUALS, UPS_INFORMATIONAL, "VERSION");
  }

  /* if we are checking syntax only, do not look for external files */
  if (! a_command_line->ugo_S) {
    /* Make sure the following files exist */
    noftp_archive_file = upsget_archive_file(a_db, a_minst, a_command_line, 1);
      
    if (noftp_archive_file[0] != '\0') {
      if (upsutl_is_a_file(noftp_archive_file) == UPS_NO_FILE) {
	upserr_add(UPS_NO_FILE, UPS_WARNING, noftp_archive_file);
      }
      upsmem_free(noftp_archive_file);
    }
    VERIFY_DIR_SPEC(a_inst->compile_dir, a_minst);
    bufPtr = upsget_prod_dir(a_db, a_minst, a_command_line);
    VERIFY_DIR_SPEC(bufPtr, (0));
    bufPtr = upsget_ups_dir(a_db, a_minst, a_command_line);
    VERIFY_DIR_SPEC(bufPtr, (0));
  }
}
/*-----------------------------------------------------------------------
 * ups_verify_table_instance
 *
 * Verify the information in a table instance
 *
 * Input : the instance to verify, information from the dbconfig file,
 *          a matched instance and info
 *          from the command line and a product name
 * Output: none
 * Return: none
 */
static void ups_verify_table_instance(VERIFY_INST_PARAMS)
{
  t_upslst_item *action_item, *command_item;
  t_upstyp_action *action;
  t_upsact_cmd *cmd_line;

  /* the following elements of an instance structure are verified here -
               version
               description
	       catman_source_dir
	       html_source_dir
	       info_source_dir
	       man_source_dir
	       news_source_dir
	       action_list
   */

  /* check that there is no version specified here */
  if (a_inst->version) {
    upserr_add(UPS_NO_VERSION, UPS_WARNING, 
	       (a_inst->product ? a_inst->product : " "));
  }
  /* make sure that the value of qualifiers is not any */
  if (! strcmp(a_inst->qualifiers, ANY_FLAVOR)) {
    upserr_add(UPS_INVALID_ANY_QUALS, UPS_INFORMATIONAL);
  }
  /* if we are checking syntax only, do not look for external files */
  if (! a_command_line->ugo_S) {
    /* check that if the product description is a file that it exists */
    VERIFY_FILE_SPEC(a_inst->description);

    /* make sure all of these directories exist */
    VERIFY_DIR_SPEC(a_inst->catman_source_dir, a_minst);
    VERIFY_DIR_SPEC(a_inst->html_source_dir, a_minst);
    VERIFY_DIR_SPEC(a_inst->info_source_dir, a_minst);
    VERIFY_DIR_SPEC(a_inst->man_source_dir, a_minst);
    VERIFY_DIR_SPEC(a_inst->news_source_dir, a_minst);
  }
  /* now verify the actions.  we will do this by parsing each action and
     checking for errors */
  for (action_item = a_inst->action_list ; action_item ;
       action_item = action_item->next) {
    action = (t_upstyp_action *)action_item->data;
    for (command_item = action->command_list ; command_item ; 
	 command_item = command_item->next) {
      if ((cmd_line = upsact_parse_cmd((char *)command_item->data))) {
	/* check that the number of parameters is legal */
	if ((cmd_line->argc < g_func_info[cmd_line->icmd].min_params) ||
	    (cmd_line->argc > g_func_info[cmd_line->icmd].max_params)) {
	  upserr_add(UPS_INVALID_ACTION_PARAMS, UPS_WARNING,
		     g_func_info[cmd_line->icmd].cmd, g_func_info[cmd_line->icmd].min_params,
		     g_func_info[cmd_line->icmd].max_params, cmd_line->argc);
	}
      } else {
	upserr_add(UPS_INVALID_ACTION, UPS_WARNING, action->action);
      }
    }
  }
}
/*-----------------------------------------------------------------------
 * ups_verify_generic_instance
 *
 * Verify the information in an instance that is common to all types
 *
 * Input : the instance to verify, information from the dbconfig file,
 *          a matched instance and info
 *          from the command line and a product name
 * Output: none
 * Return: none
 */
static void ups_verify_generic_instance(VERIFY_INST_PARAMS,
					const char * const a_product_name)
{
  /* the following elements of an instance structure are verified here -
               product
	       flavor
	       qualifiers
	       authorized_nodes
	       statistics 
  */
  /* make sure name in file matches name of product in database */
  if (a_inst->product && a_product_name &&
      strcmp(a_inst->product, a_product_name)) {
    upserr_add(UPS_MISMATCH_PROD_NAME, UPS_WARNING, a_inst->product, 
	       a_product_name);
  }
  /* make sure that these are not comma separated lists */
  CHECK_FOR_COMMA(a_inst->flavor);
  CHECK_FOR_COMMA(a_inst->qualifiers);
  CHECK_FOR_COMMA(a_inst->authorized_nodes);
  CHECK_FOR_COMMA(a_inst->statistics);
  SHUTUP;
}


static void shutup (VERIFY_INST_PARAMS)
{
  bit_bucket ^= (long) a_inst;
  bit_bucket ^= (long) a_db;
  bit_bucket ^= (long) a_minst;
  bit_bucket ^= (long) a_command_line;
}
