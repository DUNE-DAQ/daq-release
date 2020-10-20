/************************************************************************
 *
 * FILE:
 *       ups_setup.h
 * 
 * DESCRIPTION: 
 *       Define all necessary command prototypes etc.
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
 *       14-Apr-1998, DjF, first
 *
 ***********************************************************************/

#ifndef _UPSCPY_H_
#define _UPSCPY_H_

/* standard include files, if needed for .h file */

/* ups specific include files, if needed for .h file */

/*
 * Constans.
 */

/*
 * Types.
 */

/*
 * Declaration of public functions.
 */
#define UPSCPY_PARAMS \
  const t_upstyp_matched_instance * const a_minst, \
  const t_upstyp_db * const a_db_info,             \
  const t_upsugo_command * const a_command_line,   \
  const FILE * const a_stream

void upscpy_html(UPSCPY_PARAMS);
void upscpy_info(UPSCPY_PARAMS);
void upscpy_man(UPSCPY_PARAMS);
void upscpy_rmman(UPSCPY_PARAMS);
void upscpy_catman(UPSCPY_PARAMS);
void upscpy_rmcatman(UPSCPY_PARAMS);
void upscpy_news(UPSCPY_PARAMS);

/*
 * Declaration of private globals.
 */

/*
 * Declarations of public variables.
 */


#endif /* _UPSCPY_H_ */
