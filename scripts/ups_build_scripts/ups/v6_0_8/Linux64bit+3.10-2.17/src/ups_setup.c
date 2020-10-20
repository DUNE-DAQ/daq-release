/************************************************************************
 *
 * FILE:
 *       ups_setup.c
 * 
 * DESCRIPTION: 
 *       Setup the product
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
 *       24-Oct-1997, EB, first
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
#include "ups_main.h"
/*
 * Definition of public variables.
 */


/*
 * Declaration of private functions.
 */

/*
 * Definition of global variables.
 */

extern t_cmd_info g_cmd_info[];
/*
 * Definition of public functions.
 */
extern int check_setup_clash(const t_upsugo_command*);

/*-----------------------------------------------------------------------
 * ups_setup
 *
 * Cycle through the actions for unsetup possibly and setup, translate them,
 * and write them to the temp file.  This is done for all UPS product
 * requirements too.
 *
 * Input : information from the command line, a stream.
 * Output: <output>
 * Return: <return>
 */
t_upslst_item *ups_setup(const t_upsugo_command * const a_command_line,
			 const FILE * const a_temp_file,
			 const int a_ups_command, const int a_exist_cmd)
{
  t_upstyp_matched_product *mproduct = NULL;
  t_upstyp_matched_instance *minst = NULL;
  t_upslst_item *cmd_list = NULL, *mproduct_list = NULL;
  t_upsugo_command* prod_only_ugo;
  char *dummy = NULL;
  int need_unique = 1, top_unsetup = 0;


  if (a_command_line->ugo_B ) {
      check_setup_clash(a_command_line);
  
      if (UPS_ERROR != UPS_SUCCESS) {	  
         return 0;
      }
  }

  /* get the requested instance */
  mproduct_list = upsmat_instance((t_upsugo_command *)a_command_line,
				  NULL, need_unique);
  if (mproduct_list && (UPS_ERROR == UPS_SUCCESS)) {
    /* get the product to be set up */
    mproduct = (t_upstyp_matched_product *)mproduct_list->data;

    /* make sure an instance was matched before proceeding */
    if (mproduct->minst_list) {
      minst = (t_upstyp_matched_instance *)(mproduct->minst_list->data);

      /* check if this product is authorized to be setup on this node */
      if (upsutl_is_authorized(minst, mproduct->db_info, &dummy)) {
	/* Check if we need to unsetup this product first.  */
	if (a_command_line->ugo_k == 0) {
	  /* get all the unsetup commands.  we must pass a command structure
	     that only has a product name in it.  then upsact will look for
	     the appropriate setup_prod environment variables to do the
	     unsetup. Remember to add the j and the database options  */
             prod_only_ugo =(struct ups_command *)
                            upsmem_malloc( sizeof(struct ups_command));
/*	  prod_only_ugo = upsmem_malloc( sizeof( t_upsugo_command ) ); */
	  (void) memset(prod_only_ugo, 0, sizeof(t_upsugo_command));
	  prod_only_ugo->ugo_product = a_command_line->ugo_product;
	  if (a_command_line->ugo_j) {
	    prod_only_ugo->ugo_j = a_command_line->ugo_j;
	  }
	  if ( a_command_line->ugo_z ) {
	    prod_only_ugo->ugo_z = a_command_line->ugo_z;
	    prod_only_ugo->ugo_db = a_command_line->ugo_db;
	  }
	  cmd_list = upsact_get_cmd(prod_only_ugo, mproduct,
				    g_cmd_info[e_unsetup].cmd, e_unsetup);
	  /* the above routine will return all of the commands associated with
	     the matched instance.  including commands associated with
	     dependencies that may not be setup.  so we need to weed those
	     commands out.  we do not want to unsetup an instance that has not
	     been setup.  e.g. - if A depends on B and you do a 
	                     setup A
			     unsetup B
			     unsetup A
             you do not want to unsetup B again, you only want to unsetup A. */
	  if ((UPS_ERROR == UPS_SUCCESS) && cmd_list) {
	    cmd_list = upsact_trim_unsetup(cmd_list, &top_unsetup);
	  }
	  if ((UPS_ERROR == UPS_SUCCESS) && cmd_list && !a_command_line->ugo_B) {
	    upsact_process_commands(cmd_list, a_temp_file);
	  }
	  /* now clean up the memory that we used */
	  upsact_cleanup(cmd_list);

	  if (UPS_ERROR != UPS_SUCCESS) {	  
	    upserr_add(UPS_UNSETUP_FAILED, UPS_WARNING, mproduct->product);
	  }
	  upsmem_free( prod_only_ugo );
	}
	/* Now process the setup actions */
	cmd_list = upsact_get_cmd((t_upsugo_command *)a_command_line,
				  mproduct, g_cmd_info[a_ups_command].cmd, a_ups_command);
	if (UPS_ERROR == UPS_SUCCESS) {
	  upsact_process_commands(cmd_list, a_temp_file);
	}
	/* now clean up the memory that we used */
	upsact_cleanup(cmd_list);

      } else {
	upserr_add(UPS_NOT_AUTH, UPS_FATAL, mproduct->product);
      }
    }
  } else {
    /* do not add any errors to the stack if we were called because of the
       ups exist command */
    if (! a_exist_cmd) {
      if (a_command_line->ugo_version && 
	  (NOT_EQUAL_ANY_MATCH(a_command_line->ugo_version))) {
	upserr_add(UPS_NO_INSTANCE, UPS_FATAL,
		   a_command_line->ugo_product,
		   (char *)a_command_line->ugo_qualifiers->data,
		   a_command_line->ugo_version, "version",
		   "(or may not exist)");
      } else if (a_command_line->ugo_chain) {
	upserr_add(UPS_NO_INSTANCE, UPS_FATAL,
		   a_command_line->ugo_product,
		   (char *)a_command_line->ugo_qualifiers->data,
		   (char *)a_command_line->ugo_chain->data, "chain",
		   "(or may not exist)");
      }
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

