/************************************************************************
 *
 * FILE:
 *       ups_unk.c
 * 
 * DESCRIPTION: 
 *       This is the unknown command handler.
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
 *       16-Oct-1997, EFB, first
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>

/* ups specific include files */
#include "ups_unk.h"
#include "upslst.h"
#include "upstyp.h"
#include "upserr.h"
#include "upsutl.h"
#include "upsmat.h"
#include "upsact.h"

/*
 * Definition of public variables.
 */

/*
 * Declaration of private functions.
 */

/*
 * Definition of global variables.
 */

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * ups_unk
 *
 * Cycle through the actions for the command that was entered, translate them,
 * and write them to the temp file.
 *
 * Input : information from the command line, a stream.
 * Output: <output>
 * Return: <return>
 */
t_upslst_item *ups_unk(const t_upsugo_command * const a_command_line,
		       const FILE * const a_temp_file,
		       const char * const a_unk_cmd)
{
  t_upslst_item *mproduct_list = NULL;
  t_upstyp_matched_product *mproduct = NULL;
  t_upslst_item *cmd_list;
  int need_unique = 1;

  /* get all the requested instances */
  mproduct_list = upsmat_instance((t_upsugo_command *)a_command_line,
                                  NULL, need_unique);
  if (mproduct_list && (UPS_ERROR == UPS_SUCCESS)) {
    /* get the product  */
    mproduct = (t_upstyp_matched_product *)mproduct_list->data;

    /* make sure an instance was matched before proceeding */
    if (mproduct->minst_list) {

      /* Now process the actions */
      cmd_list = upsact_get_cmd((t_upsugo_command *)a_command_line,
				mproduct, a_unk_cmd, e_unk);
      if (cmd_list && UPS_ERROR == UPS_SUCCESS) {
	upsact_process_commands(cmd_list, a_temp_file);
      } else if (!cmd_list && UPS_ERROR == UPS_SUCCESS) {
	/* tell people that nothing was found in case they really meant
	   something else and just mistyped */
	upserr_add(UPS_NO_ACTION, UPS_INFORMATIONAL, a_unk_cmd);
      }
      /* now clean up the memory that we used */
      upsact_cleanup(cmd_list);
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

