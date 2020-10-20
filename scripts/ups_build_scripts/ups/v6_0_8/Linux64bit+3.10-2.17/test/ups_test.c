/* ups main test routine

Authors:        Margaret Votava
e-mail:         "votava@fnal.gov"
 
Include files:-
===============
*/
#include "stdlib.h"
#include "upstst_cmdtable.h"
#include "ups.h"

/* Globals:-
   ========= */

extern t_upstyp_product *upstst_file;

/* Prototypes:-
   ============ */

void upstst_commandloop (char *prompt, upstst_cmd_table_t *);

/* function prototypes for test routines 
   ------------------------------------- */

int upstst_date(int, char **);		int upstst_echo(int, char **);
int upstst_debug_level(int, char **);
int upstst_err_output(int, char **);	int upstst_err_clear(int, char**);
int upstst_err_add(int, char**);
int upstst_fil_read_file(int, char**);  int upstst_fil_write_file(int, char**);
int upstst_ugo_env(int, char**);  	int upstst_ugo_next(int, char**);
int upstst_ugo_bldcmd(int, char**);
int upstst_mat_instance(int, char**); 	
int upstst_get_translation(int, char**);int upstst_get_allout(int, char**);
int upstst_act_print(int, char**);	int upstst_act_process_commands(int, char**); 
int upstst_list(int, char**);		int upstst_depend(int, char**);
int upstst_declare(int, char**);	int upstst_undeclare(int, char**);
int upstst_configure(int, char**);	int upstst_unconfigure(int, char**);
int upstst_tailor(int, char**);		int upstst_copy(int, char**);
int upstst_flavor(int, char**);
int upstst_start(int, char**);		int upstst_stop(int, char**);
int upstst_get(int, char**);		int upstst_modify(int, char**);
int upstst_setup(int, char**);		int upstst_unsetup(int, char**);
int upstst_touch(int, char**); 		int upstst_unk(int, char**);		

/*=============================================================================
Routine:
   	main program for ups_test. It's a pretty simply program. just call 
	the command loop with the prompt and list of valid commands.
==============================================================================*/

int main(void)
{

/* build the list of commands that we support and their corresponding
   function pointers. 
   ------------------------------------------------------------------ */
upstst_cmd_table_t upstst_my_cmds[] = {
	{ "ups_date",			upstst_date },
	{ "ups_echo",			upstst_echo },
	{ "ups_debug",			upstst_debug_level },
  	{ "upserr_output",		upstst_err_output },
  	{ "upserr_clear",		upstst_err_clear },
  	{ "upserr_add",			upstst_err_add },
	{ "upsfil_read_file",		upstst_fil_read_file },
	{ "upsfil_write_file",		upstst_fil_write_file },
	{ "upsugo_env",			upstst_ugo_env },
	{ "upsugo_next",		upstst_ugo_next },
	{ "upsugo_bldcmd",		upstst_ugo_bldcmd },
	{ "upsmat_instance",		upstst_mat_instance },
	{ "upsget_translation",		upstst_get_translation },
	{ "upsget_allout",		upstst_get_allout },
	{ "upsact_print",		upstst_act_print },
	{ "upsact_process_commands",	upstst_act_process_commands },
	{ "list",			upstst_list },
	{ "depend",			upstst_depend },
	{ "declare",			upstst_declare },
	{ "undeclare",			upstst_undeclare },
	{ "configure",			upstst_configure },
	{ "unconfigure",		upstst_unconfigure },
	{ "tailor",			upstst_tailor },
	{ "copy",			upstst_copy },
	{ "start",			upstst_start },
	{ "stop",			upstst_stop },
	{ "flavor",			upstst_flavor },
	{ "get",			upstst_get },
	{ "modify",			upstst_modify },
	{ "setup",			upstst_setup },
	{ "unsetup",			upstst_unsetup },
	{ "touch",			upstst_touch },
	{ "unk",			upstst_unk },
	{ NULL,				0} };

upstst_commandloop ("ups_test> ", upstst_my_cmds);
return(0);
}

