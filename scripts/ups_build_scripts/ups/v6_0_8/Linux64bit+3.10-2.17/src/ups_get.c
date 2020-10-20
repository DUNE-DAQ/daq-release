/************************************************************************
 *
 * FILE:
 *       ups_get.c
 * 
 * DESCRIPTION: 
 *       Get information about a product not readily obtainable from
 *       ups list.
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
 *       5-Jan-1998, EB, first
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>
#include <string.h>

/* ups specific include files */
#include "upserr.h"
#include "upsugo.h"
#include "upsutl.h"
#include "upsmat.h"
#include "upsact.h"
#include "upsmem.h"
#include "upsget.h"
#include "ups_main.h"
/*
 * Definition of public variables.
 */


/*
 * Declaration of private functions.
 */

static void get_files(const t_upstyp_matched_product * const a_mproduct,
		      const t_upsugo_command * const a_command_line);

/*
 * Definition of global variables.
 */

extern t_cmd_info g_cmd_info[];
/*
 * Definition of public functions.
 */

#define IS_IN_PROD_DIR()   \
   if (minst->version && minst->version->prod_dir) {  \
     if (! strstr(file, minst->version->prod_dir)) {  \
       /* no it is not, print it out */               \
       (void) printf("%s\n", (char *)file);           \
     }                                                \
   } else {                                           \
     /* cannot get prod dir, output all the files */  \
     (void) printf("%s\n", (char *)file);             \
   }

/*-----------------------------------------------------------------------
 * ups_get
 *
 * Cycle through the actions for get, translate them,
 * and write them to the temp file. Also output the requested additional
 * information.
 *
 * Input : information from the command line, a stream.
 * Output: <output>
 * Return: <return>
 */
t_upslst_item *ups_get(const t_upsugo_command * const a_command_line,
		       const FILE * const a_temp_file,
		       const int a_ups_command)
{
  t_upslst_item *mproduct_list = NULL;
  t_upstyp_matched_product *mproduct = NULL;
  t_upslst_item *cmd_list;
  int need_unique = 1;

  /* get all the requested instances */
  mproduct_list = upsmat_instance((t_upsugo_command *)a_command_line,
				  NULL, need_unique);
  if (mproduct_list && (UPS_ERROR == UPS_SUCCESS)) {
    /* get the product */
    mproduct = (t_upstyp_matched_product *)mproduct_list->data;

    /* make sure an instance was matched before proceeding */
    if (mproduct->minst_list) {
      /* Now process the get actions */
      cmd_list = upsact_get_cmd((t_upsugo_command *)a_command_line,
				mproduct, g_cmd_info[a_ups_command].cmd,
				a_ups_command);
      if (UPS_ERROR == UPS_SUCCESS) {
	upsact_process_commands(cmd_list, a_temp_file);
      }
      /* now clean up the memory that we used */
      upsact_cleanup(cmd_list);
    }

    /* now return to the user what was asked for */
    if (a_command_line->ugo_F) {
      /* user wants a list of all files used in an action line that are
	 not located under the product root directory. this means any action
	 line in this instance */
      get_files(mproduct, a_command_line);
    }
  } 

  return(mproduct_list);

}

/*
 * Definition of private globals.
 */

/*
 * Definition of private functions.
 */

/*-----------------------------------------------------------------------
 * get_files
 *
 * Return a list of files needed to use the product not located within the
 *         products' root directory. this list does not include the table file.
 *
 * Input : a matched product
 * Output: none
 * Return: list of files
 */
static void get_files(const t_upstyp_matched_product * const a_mproduct,
		      const t_upsugo_command * const a_command_line)
{
  t_upslst_item *all_file_list = NULL;
  t_upstyp_matched_instance *minst;
  t_upslst_item *act_list = NULL, *cmd_list = NULL;
  t_upstyp_action *action = NULL;
  char *cmd = NULL, *file = NULL;
  
  /* get the instance and make sure we have a table file and some actions */
  minst = (t_upstyp_matched_instance *)a_mproduct->minst_list->data;
  if (minst && minst->table) {
    /* for each action encountered in the table file, parse thru the 
       individual action commands and check if any of the parameters is a
       file or not. */
    for (act_list = minst->table->action_list ; act_list ;
	 act_list = act_list->next) {
      action = (t_upstyp_action *)act_list->data;
      for (cmd_list = action->command_list ; cmd_list ;
	   cmd_list = cmd_list->next) {
	cmd = (char *)cmd_list->data;
	/* now check the command to see if it contains references to any
	   files */
	all_file_list = upsact_check_files(a_mproduct, a_command_line, cmd);
	if (all_file_list) {
	  /* yes it did.  now we have to check the files to see if they
	     are located in the product root. they come back to us as a list.
	     we do this right now by comparing the PROD_DIR from the version
	     file and the file list.  if any of the files contain that string
	     in them we do not print them out. */
	  for ( ; all_file_list ; all_file_list = all_file_list->next) {
	    file = all_file_list->data;
	    
	    /* check if the file is in the product root directory */
	    IS_IN_PROD_DIR();
	  }

	  /* free up the list and the data elements too */
	  all_file_list = upslst_free(all_file_list, 'd');
	}
      }
    }

    /* now check the product description field, it may be a file name too */
    if (minst->table->description) {
      /* first translate any ups local environment variables */
      file = upsget_translation(
		     (t_upstyp_matched_instance *)a_mproduct->minst_list->data,
		     a_mproduct->db_info, a_command_line, 
		     minst->table->description);
      
      /* now see if the resulting string is a file */
      if(upsutl_is_a_file(file) == UPS_SUCCESS) {
	/* now see if it is under the product root directory or not */
	IS_IN_PROD_DIR();
      }
    }
  }
}

