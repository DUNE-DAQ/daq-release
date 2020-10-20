/************************************************************************
 *
 * FILE:
 *       ups_unsetup.c
 * 
 * DESCRIPTION: 
 *       Unsetup the product
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

/* ups specific include files */
#include "upserr.h"
#include "upsugo.h"
#include "upsutl.h"
#include "upsmat.h"
#include "upsact.h"
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

/*-----------------------------------------------------------------------
 * ups_unsetup
 *
 * Cycle through the actions for unsetup, translate them,
 * and write them to the temp file.  This is done for all UPS product
 * requirements too.
 *
 * Input : information from the command line, a stream.
 * Output: <output>
 * Return: <return>
 */
t_upslst_item *ups_unsetup( const t_upsugo_command * const a_command_line,
			    const FILE * const a_temp_file,
			    const int a_ups_command)
{
  t_upstyp_matched_product *mproduct = NULL;
  t_upstyp_matched_instance *minst = NULL;
  t_upslst_item *cmd_list, *mproduct_list = NULL;
  t_upsugo_command *new_command_line;
  char *dummy = NULL;
  int need_unique = 1, got_new_cmd_line = 0;
  int top_unsetup = 0;

  /* if no product name was given, this is an error */
  if (a_command_line->ugo_product) {
    /* If enough information was passed on the command line that allows us to
       identify an instance then use that information.  otherwise, try to get
       the SETUP_<prod> environment variable and get the information from
       that. */
    if (TOO_MUCH_FOR_UNSETUP(a_command_line)) {
      new_command_line = (t_upsugo_command *)a_command_line;
    } else {
      /* translate SETUP_<prodname> to get instance information */
      new_command_line = upsugo_env(a_command_line->ugo_product,
                                    g_cmd_info[e_setup].valid_opts);
      got_new_cmd_line = 1;
      if (new_command_line)      /* Use the command line's -j setting */
        new_command_line->ugo_j = a_command_line->ugo_j;
    }

    /* silently ignore -B if we are a command-line unsetup */
    if (new_command_line) new_command_line->ugo_B = 0;
    
    if (new_command_line) {
      /* get all the requested instances */
      mproduct_list = upsmat_instance((t_upsugo_command *)new_command_line,
				      NULL, need_unique);

      if (mproduct_list && (UPS_ERROR == UPS_SUCCESS)) {
	/* get the product to be set up */
	mproduct = (t_upstyp_matched_product *)mproduct_list->data;
      
	/* make sure an instance was matched before proceeding */
	if (mproduct->minst_list) {
	  minst = (t_upstyp_matched_instance *)(mproduct->minst_list->data);
	
	  /* check if this product is authorized to be unsetup on this node */
	  if (upsutl_is_authorized(minst, mproduct->db_info, &dummy)) {
	    if (UPS_ERROR == UPS_SUCCESS) {	  
	      /* Now process the unsetup actions */
	      cmd_list = upsact_get_cmd((t_upsugo_command *)new_command_line,
					mproduct, 
					g_cmd_info[a_ups_command].cmd,
					a_ups_command);
	      /* the above routine will return all the commands associated with
		 the matched instance.  including commands associated with
		 dependencies that may not be setup.  so we need to weed those
		 commands out.  we do not want to unsetup an instance that has
		 not been setup.  e.g. - if A depends on B and you do a 
		         setup A
			 unsetup B
			 unsetup A
		 you do not want to unsetup B again, you only want to
		 unsetup A. */
	      if ((UPS_ERROR == UPS_SUCCESS) && cmd_list) {
		cmd_list = upsact_trim_unsetup(cmd_list, &top_unsetup);
	      }
	      if ((UPS_ERROR == UPS_SUCCESS) && top_unsetup && cmd_list) {
		/* top product is set up (top_unsetup = 1), so do a unsetup */
		upsact_process_commands(cmd_list, a_temp_file);
	      }
	      /* now clean up the memory that we used */
	      upsact_cleanup(cmd_list);
	    }
	  } else {
	    upserr_add(UPS_NOT_AUTH, UPS_FATAL, mproduct->product);
	  }
	}
      }
      /* free this if we got a new one */
      if (got_new_cmd_line) {
        (void )upsugo_free(new_command_line);
      }
    } else {
      upserr_add(UPS_NO_SETUP_ENV, UPS_FATAL,
		 upsutl_upcase(a_command_line->ugo_product));
    }
  } else {
    upserr_add(UPS_NO_PRODUCT, UPS_FATAL, g_cmd_info[a_ups_command].cmd);
  }

  return(mproduct_list);
}

/*
 * Definition of private globals.
 */

/*
 * Definition of private functions.
 */


