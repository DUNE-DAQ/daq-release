/************************************************************************
 *
 * FILE:
 *       upsact.c
 * 
 * DESCRIPTION: 
 *       This file contains routines to manage ups action lines.
 *       *** It need to be reentrant ***
 *
 *       Steps in order to add a new action -
 *           1. add a f_<action> prototype at the top of the file
 *           2. add an e_<action> to the enum in upsact.h
 *           3. add a line to the g_func_infos structure
 *           4. add the code for the f_<action> function
 *           5. check if you have to add anything to upsact_check_files
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
 *       28-Aug-1997, EB, first
 *
 ***********************************************************************/

/* standard include files */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ups specific include files */
#include "upsact.h"
#include "upserr.h"
#include "upsver.h"
#include "upslst.h"
#include "upsutl.h"
#include "upsmem.h"
#include "upskey.h"
#include "upsugo.h"
#include "upsget.h"
#include "ups_main.h"

extern void list_K( const t_upstyp_matched_instance * const instance, 
		    const t_upsugo_command * const command, 
		    const t_upstyp_matched_product * const product,
		    int ignore_match_done );

/*
 * Definition of public variables.
 */
int g_LOCAL_VARS_DEF = 0;
int g_keep_temp_file = 0;
char *g_temp_file_name = NULL;
int g_COMPILE_FLAG = 0;
extern int g_UPS_SHELL;

static char *g_file_ext[e_MAX_SHELL] = {"sh", "csh"};

/*
 * Private constants
 */

#define DQUOTE '"'
#define COMMA ','
#define OPEN_PAREN '('
#define CLOSE_PAREN ')'
#define SLASH "/"
#define WSPACE " \t\n\r\f"
#define EXIT "EXIT"
#define CONTINUE "CONTINUE"
#define UPS_ENV "UPS_ENV"
#define NO_UPS_ENV "NO_UPS_ENV"
#define DO_CHECK 1
#define NO_CHECK 0
#define NO_EXIT 0
#define DO_EXIT 1
#define DO_NO_UPS_ENV 1
#define DO_UPS_ENV 0
#define DATE_FLAG "DATE"
#define OLD_FLAG "OLD"
#define CATMANPAGES "catman"
#define MANPAGES "man"
#define DROPIT upsact_dropit_buf

char upsact_dropit_buf[2048] = "$UPS_DIR/bin/dropit -e -n '' ";

/*
 * Private types
 */

/*
 * Declaration of private functions.
 */

static int parse_params( const char * const a_params,
			 char *argv[] );

static t_upslst_item *next_cmd( t_upslst_item * const top_list,
				t_upslst_item *dep_list,
				t_upsact_item * const p_act_itm,
				const char * const act_name,
				const char copt );
static t_upslst_item *next_top_prod( t_upslst_item * top_list,
				     t_upsact_item *const p_act_itm, 
				     const char *const act_name );
static t_upstyp_action *get_act( const t_upsugo_command * const ugo_cmd,
				 t_upstyp_matched_product * mat_prod,
				 const char * const act_name );
static t_upsact_item *get_top_item( t_upsugo_command * const ugo_cmd,
				    t_upstyp_matched_product *mat_prod,
				    const char * const act_name );
static t_upsugo_command *get_dependent( t_upsact_cmd * const p_cmd, 
				  int is_act_build,
				  int * const unsetup_flag );
static int is_prod_done( const char *const prod_name );
static int is_prod_clash( const t_upsugo_command *const initial);
static t_upsact_item *find_prod_name( t_upslst_item *const dep_list,
				      const char *const prod_name );
static t_upsact_item *new_act_item( t_upsugo_command * const ugo_cmd,
				    t_upstyp_matched_product *mat_prod,
				    const int level,
				    const int mode,
				    const char * const act_name );
static t_upsact_item *copy_act_item( const t_upsact_item * const act_itm );
static int cmp_ugo_db( const void * const d1, const void * const d2 );
static t_upslst_item* merge_ugo_db( t_upslst_item * const db_list1, 
				    t_upslst_item * const db_list2 );
static t_upstyp_action *new_default_action( t_upsact_item *const p_act_itm, 
					    const char * const act_name, 
					    const int iact );
static t_upslst_item *reverse_command_list( t_upsact_item *const p_act_itm,
					    t_upslst_item *const cmd_list );
static char *actitem2str( const t_upsact_item *const p_act_itm );
static t_upsugo_command *get_SETUP_prod( t_upsact_cmd * const p_cmd, 
					 /* const int i_act, */
					 const int i_build );
static int do_exit_action( const t_upsact_cmd * const a_cmd );
       int check_setup_clash(t_upsugo_command *new_ugo);
static int check_setup_match_clash(t_upstyp_matched_product * const mat_prod );

/* functions to handle specific action commands */

#define ACTION_PARAMS \
  const t_upstyp_matched_instance * const a_minst, \
  const t_upstyp_db * const a_db_info,             \
  t_upsugo_command * const a_command_line,   \
  const FILE * const a_stream,                     \
  const t_upsact_cmd * const a_cmd

static void f_copyhtml( ACTION_PARAMS);
static void f_copyinfo( ACTION_PARAMS);
static void f_copyman( ACTION_PARAMS);
static void f_uncopyman( ACTION_PARAMS);
static void f_copycatman( ACTION_PARAMS);
static void f_uncopycatman( ACTION_PARAMS);
static void f_copynews( ACTION_PARAMS);
static void f_envappend( ACTION_PARAMS);
static void f_envprepend( ACTION_PARAMS);
static void f_envremove( ACTION_PARAMS);
static void f_envset( ACTION_PARAMS);
static void f_envsetifnotset( ACTION_PARAMS);
static void f_envunset( ACTION_PARAMS);
static void f_exeaccess( ACTION_PARAMS);
static void f_execute( ACTION_PARAMS);
static void f_filetest( ACTION_PARAMS);
static void f_pathappend( ACTION_PARAMS);
static void f_pathprepend( ACTION_PARAMS);
static void f_pathremove( ACTION_PARAMS);
static void f_pathset( ACTION_PARAMS);
static void f_addalias( ACTION_PARAMS);
static void f_unalias( ACTION_PARAMS);
static void f_sourcerequired( ACTION_PARAMS);
static void f_sourceoptional( ACTION_PARAMS);
static void f_sourcereqcheck( ACTION_PARAMS);
static void f_sourceoptcheck( ACTION_PARAMS);
static void f_sourcecompilereq( ACTION_PARAMS);
static void f_sourcecompileopt( ACTION_PARAMS);
static void f_writecompilescript( ACTION_PARAMS);
static void f_setupenv( ACTION_PARAMS);
static void f_proddir( ACTION_PARAMS);
static void f_unsetupenv( ACTION_PARAMS);
static void f_unproddir( ACTION_PARAMS);
static void f_dodefaults( ACTION_PARAMS);
static void f_if( ACTION_PARAMS);
static void f_endif( ACTION_PARAMS);
static void f_else( ACTION_PARAMS);
static void f_unless( ACTION_PARAMS);

static void shutup( ACTION_PARAMS);
/* pretend to use all of the parameters we've defined */
#define SHUTUP \
  if ((&bit_bucket + bit_bucket_offset == 0) && 0) shutup (a_minst, a_db_info, a_command_line, a_stream, a_cmd);

#define OUTPUT_ACTION_INFO(severity, minst) \
    if (minst->version && minst->version->product &&                  \
        minst->version->version) {                                    \
      upserr_add(UPS_PRODUCT_INFO, severity, ACT_PREFIX,              \
                 minst->version->product, minst->version->version,    \
                 minst->version->flavor, minst->version->qualifiers); \
    }

#define CHECK_NUM_PARAM(action) \
    if ((a_cmd->argc < g_func_info[a_cmd->icmd].min_params) ||   \
        (a_cmd->argc > g_func_info[a_cmd->icmd].max_params)) {   \
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);                    \
      upserr_vplace();                                           \
      upserr_add(UPS_INVALID_ACTION_PARAMS, UPS_FATAL,           \
                 action, g_func_info[a_cmd->icmd].min_params,    \
                 g_func_info[a_cmd->icmd].max_params,            \
		 a_cmd->argc);                                   \
    }

#define ACT_PREFIX "UPSACT: "

#define OUTPUT_VERBOSE_MESSAGE(cmd)  \
    if (a_minst->version && a_minst->version->product) {                 \
      upsver_mes(1, "%sProcessing action \'%s\' for product \'%s\'\n",   \
                 ACT_PREFIX, cmd, a_minst->version->product);            \
    } else {                                                             \
      upsver_mes(1, "%sProcessing action \'%s\'\n", ACT_PREFIX, cmd);    \
    }

#define GET_DELIMITER() \
    if (a_cmd->argc == g_func_info[a_cmd->icmd].max_params) {           \
      /* remember arrays start at 0, so subtract one here */            \
      delimiter =                                                       \
	(a_cmd->argv[g_func_info[a_cmd->icmd].max_params-1]);           \
      /* trim delimiter for quotes */                                   \
      (void) upsutl_str_remove_end_quotes( delimiter, "\"\'", 0 );      \
      /* if the delimiter is 0 length, then use a space */              \
      if (*delimiter == '\0') {                                         \
	delimiter = g_space_delimiter;                                  \
      }                                                                 \
    } else {                                                            \
      /* use the default, nothing was entered */                        \
      delimiter = g_default_delimiter;                                  \
    }

#define CHECK_FOR_PATH(thePath, theDelimiter)  \
    /* if the variable is path then it must be of a certain case */  \
    if ( upsutl_stricmp(thePath,a_cmd->argv[0])) {                   \
      /* it was not equal to path so use the original value */       \
      pathPtr = a_cmd->argv[0];                                      \
    } else {                                                         \
      /* it was path, make sure we use a right one by using ours */  \
      pathPtr = thePath;                                             \
      /* and the delimiter must be set right too */                  \
      delimiter = theDelimiter;                                      \
    }

#define GET_ERR_MESSAGE(msg_ptr) \
    if (msg_ptr) {                                 \
      err_message = msg_ptr;                       \
    } else {                                       \
      err_message = "";                            \
    }

#define FPRINTF_ERROR() \
    upserr_vplace(); \
    upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "fprintf", strerror(errno));

#define GET_FLAGS() \
    if (a_cmd->argc > g_func_info[a_cmd->icmd].min_params) {                \
      int i;                                                               \
      /* we have more than the minimum number of params, must be flags */  \
      for (i = 2 ; i < a_cmd->argc ; ++i) {                                \
	if ((! upsutl_stricmp(a_cmd->argv[i], EXIT))) {                    \
	  exit_flag = DO_EXIT;                                             \
	  continue;                                                        \
	}                                                                  \
      }                                                                    \
    }

#define GET_UPS_ENV()  \
    /* determine if we are to set the ups local environment variables */  \
    if ((! upsutl_stricmp(a_cmd->argv[1], UPS_ENV))) {                    \
      no_ups_env_flag = DO_UPS_ENV;                                       \
    } else if (upsutl_stricmp(a_cmd->argv[1], NO_UPS_ENV)) {              \
      /* the second parameter was not UPS_ENV or NO_UPS_ENV, an error */  \
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);                             \
      upserr_vplace();                                                    \
      upserr_add(UPS_EXECUTE_ARG2, UPS_FATAL, a_cmd->argv[1]);            \
    }

#define SH_OUTPUT_LAST_PART_OPT() \
   if (fprintf((FILE *)a_stream, "fi\n#\n") < 0) {  \
     FPRINTF_ERROR();                               \
   }

#define CSH_OUTPUT_LAST_PART_OPT() \
   if (fprintf((FILE *)a_stream, "endif\n#\n") < 0) {  \
     FPRINTF_ERROR();                                  \
   }

#define SET_SETUP_PROD() \
   upsget_envout(a_stream, a_db_info, a_minst, a_command_line);

#define DO_SYSTEM_MOVE(move_flag)  \
   if (system(g_buff) != 0) {                                             \
     OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);                              \
     /* error from system call */                                         \
     upserr_vplace();                                                     \
     upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "system", strerror(errno));  \
   } else {                                                               \
     move_flag = 1;                                                       \
   }

#define SET_PARSE_ERROR( str ) \
    upserr_vplace(); \
    upserr_add( UPS_ACTION_PARSE, UPS_FATAL, str );

#define SET_NO_ACTION_ERROR( str ) \
    upserr_vplace(); \
    upserr_add( UPS_NO_ACTION, UPS_FATAL, str );

#define P_VERB_s( iver, str ) \
  if( (UPS_VERBOSE) ) upsver_mes( iver, "UPSACT: %s\n", \
				  (str != 0) ? str : "(null)" )

#define P_VERB_s_i( iver, str1, ival ) \
  if( (UPS_VERBOSE) ) upsver_mes( iver, "UPSACT: %s %d\n", \
				  (str1 != 0) ? str1 : "(null)", \
				  ival )
#define P_VERB_s_s( iver, str1, str2 ) \
  if( (UPS_VERBOSE) ) upsver_mes( iver, "UPSACT: %s %s\n", \
				  (str1 != 0) ? str1 : "(null)", \
				  (str2 != 0) ? str2 : "(null)" )

#define P_VERB_s_s_s( iver, str1, str2, str3 ) \
  if( (UPS_VERBOSE) ) upsver_mes( iver, "UPSACT: %s %s %s\n", \
				  (str1 != 0) ? str1 : "(null)", \
				  (str2 != 0) ? str2 : "(null)", \
				  (str3 != 0) ? str3 : "(null)" )

#define P_VERB_s_s_s_s( iver, str1, str2, str3, str4 ) \
  if( (UPS_VERBOSE) ) upsver_mes( iver, "UPSACT: %s %s %s %s\n", \
				  (str1 != 0) ? str1 : "(null)", \
				  (str2 != 0) ? str2 : "(null)", \
				  (str3 != 0) ? str3 : "(null)", \
				  (str4 != 0) ? str4 : "(null)" )
     
#define P_VERB_s_s_s_s_s( iver, str1, str2, str3, str4, str5 ) \
  if( (UPS_VERBOSE) ) upsver_mes( iver, "UPSACT: %s %s %s %s %s\n", \
				  (str1 != 0) ? str1 : "(null)", \
				  (str2 != 0) ? str2 : "(null)", \
				  (str3 != 0) ? str3 : "(null)", \
				  (str4 != 0) ? str4 : "(null)", \
				  (str5 != 0) ? str5 : "(null)" )

/*
 * Definition of global variables.
 */

static char *g_default_delimiter = ":";
static char *g_space_delimiter = " ";
static int g_ups_cmd = e_invalid_action;
static int g_top_unsetup = 0;
static char g_buff[MAX_LINE_LEN];
static char *g_shPath = "PATH";
static char *g_cshPath = "path";
static long bit_bucket = 0;
static long bit_bucket_offset = 0;

/* this one, is a pointer to the ugo_command from the command line */
static t_upsugo_command *g_ugo_cmd = 0;

/* this one, will hold the list of setuped products */
static t_upslst_item *g_prod_done = 0;

/* this one, is a pointer to the top level action item from the command line */
static t_upsact_item *g_act_itm;

/* 
 * Note: This array should in princip be in ups_main.c, but since
 * ups_main is not part of the ups library, it's here.
 *
 * The enum is defined in ups_main.h
 * where flags:
 * <byte>: <description>
 *   0   : specify if that action has a default set of commands
 *   1   : specify if the associated command needs a db entered from the
 *              command line (or a table file)
 */

t_cmd_info g_cmd_info[] = {
  {e_current,       "current", 0, 0x00000011, e_invalid_action},
  {e_development,   "development", 0, 0x00000010, e_invalid_action},
  {e_new,           "new", 0, 0x00000010, e_invalid_action},
  {e_old,           "old", 0, 0x00000010, e_invalid_action},
  {e_test,          "test", 0, 0x00000010, e_invalid_action},
  {e_chain,         "chain", 0, 0x00000010, e_invalid_action},
  {e_uncurrent,     "uncurrent", 0, 0x00000011, e_current},
  {e_undevelopment, "undevelopment", 0, 0x00000010, e_development},
  {e_unnew,         "unnew", 0, 0x00000010, e_new},
  {e_unold,         "unold", 0, 0x00000010, e_old},
  {e_untest,        "untest", 0, 0x00000010, e_test},
  {e_unchain,       "unchain", 0, 0x00000010, e_chain},
  {e_setup,       "setup",       "?Bcdef:g:H:jkm:M:noO:Pq:r:RstU:vVz:Z01234567567.", 0x00000011, e_invalid_action},
  {e_unsetup,     "unsetup",     "?cdef:g:H:jm:M:noO:Pq:r:stU:vVz:Z01234567.", 0x00000011, e_setup},
  {e_list,        "list",        "?acdf:g:H:K:lm:M:noq:r:tU:vz:Z01234567.", 0x00000010, e_invalid_action},
  {e_parent,        "parent",    "?acdf:g:H:K:lm:M:noq:r:tU:vz:Z01234567", 0x00000010, e_invalid_action},
  {e_configure,   "configure",   "?cdf:g:H:m:M:noO:Pq:r:stU:vVz:Z01234567.", 0x00000010, e_invalid_action},
  {e_copy,        "copy",        "?A:b:cCdD:f:g:G:H:m:M:noO:p:q:r:tT:u:U:vVWXz:Z01234567", 0x00000010, e_invalid_action},
  {e_declare,     "declare",     "?A:b:cCdD:f:g:H:Lm:M:noO:q:r:tT:u:U:vz:Z01234567", 0x00000010, e_declare},
  {e_depend,      "depend",      "?Bcdf:g:H:jK:lm:M:noOq:r:RtU:vVz:Z01234567", 0x00000010, e_invalid_action},
  {e_exist,       "exist",       "?Bcdef:g:H:jkm:M:noO:Pq:r:tU:vVz:Z01234567", 0x00000010, e_invalid_action},
/*  {e_modify,      "modify",      "?aA:E:f:H:m:M:Np:q:r:T:U:vVz:Z", 0x00000010, e_invalid_action},	*/
  {e_modify,      "modify",      "?Acd:E:f:H:m:M:nNop:q:r:tT:U:vVz:Z", 0x00000010, e_invalid_action},
  {e_start,       "start",       "?cdf:g:H:m:M:noO:Pq:r:stU:vVwz:Z01234567.", 0x00000010, e_invalid_action},
  {e_stop,        "stop",        "?cdf:g:H:m:M:noO:Pq:r:stU:vVz:Z01234567.", 0x00000010, e_invalid_action},
  {e_tailor,      "tailor",      "?cdf:g:H:K:m:M:noO:Pq:r:stU:vVz:Z01234567.", 0x00000010, e_invalid_action},
  {e_touch,       "touch",       "?cdf:g:H:noq:tvz:Z01234567", 0x00000010, e_invalid_action},
  {e_unconfigure, "unconfigure", "?cdf:g:H:m:M:noO:Pq:r:stU:vVz:Z01234567.", 0x00000010, e_configure},
  {e_undeclare,   "undeclare",   "?cCdf:g:H:m:M:noO:q:r:tU:vVyYz:Z01234567", 0x00000010, e_undeclare},
  {e_create,   "_",   "", 0x00000000, e_invalid_action}, /* no longer used */
  {e_get,         "get",         "?cdf:Fg:H:m:M:noq:r:tU:vz:Z", 0x00000010, e_invalid_action},
  {e_flavor,      "flavor",      "?Bf:H:lvZ01234567", 0x00000000, e_invalid_action},
  {e_help,        "help",        "?v", 0x00000000, e_invalid_action},
  {e_verify,      "verify",      "?Bacdf:g:H:m:M:noq:r:tU:vz:Z01234567", 0x00000010, e_invalid_action},
  {e_layout,      "layout",        "?az:", 0x00000010, e_invalid_action},
  /* the following one must always be at the end and contains all options */
  {e_unk,         NULL,          "?aA:b:B:cCdD:eE:f:Fg:G:H:jkK:lm:M:nNoO:Pp:q:r:sStRT:u:U:vVwW:XyYz:Z01234567.",
                                  0x00000010, e_invalid_action}
};

/* These action commands are listed in order of use.  Hopefully the more
 * used actions are at the front of the list. Also the ones most used by
 * setup and unsetup are at the front of the array.  The actions in this
 * array MUST appear in the same order as in the above enumeration. NOTE:
 * the 4th and 5th parameters are the minimum and maximum parameters expected
 * by the action. The six parameter is the corresponding 'undo' command.
 *
 * name, enum, command_handler, min argc, max argc, enum of opposite, flags
 *
 * where flags:
 * <byte>: <description>
 *   0   : specify if that action function is valid in a table file
 *
 */
t_cmd_map g_func_info[] = {
  { "setupoptional", e_setupoptional, NULL, 1, 1, e_unsetupoptional, 0x00000001 },
  { "setuprequired", e_setuprequired, NULL, 1, 1, e_unsetuprequired, 0x00000001 },
  { "unsetupoptional", e_unsetupoptional, NULL, 1, 1, e_setupoptional, 0x00000001 },
  { "unsetuprequired", e_unsetuprequired, NULL, 1, 1, e_setuprequired, 0x00000001 },
  { "exeactionoptional", e_exeactionoptional, NULL, 1, 1, e_rev_exeactionoptional, 0x00000001 },
  { "exeactionrequired", e_exeactionrequired, NULL, 1, 1, e_rev_exeactionrequired, 0x00000001 },
  { "rev_exeactionoptional", e_rev_exeactionoptional, NULL, 1, 1, e_invalid_cmd, 0x00000001 },
  { "rev_exeactionrequired", e_rev_exeactionrequired, NULL, 1, 1, e_invalid_cmd, 0x00000001 },
  { "sourcecompilereq", e_sourcecompilereq, f_sourcecompilereq, 1, 1, e_invalid_cmd, 0x00000001 },
  { "sourcecompileopt", e_sourcecompileopt, f_sourcecompileopt, 1, 1, e_invalid_cmd, 0x00000001 },
  { "envappend", e_envappend, f_envappend, 2, 3, e_envremove, 0x00000001 },
  { "envremove", e_envremove, f_envremove, 2, 3, e_invalid_cmd, 0x00000001 },
  { "envprepend", e_envprepend, f_envprepend, 2, 3, e_envremove, 0x00000001 },
  { "envset", e_envset, f_envset, 2, 2, e_envunset, 0x00000001 },
  { "envsetifnotset", e_envsetifnotset, f_envsetifnotset, 2, 2, e_invalid_cmd, 0x00000001 },
  { "envunset", e_envunset, f_envunset, 1, 1, e_invalid_cmd, 0x00000001 },
  { "pathappend", e_pathappend, f_pathappend, 2, 3, e_pathremove, 0x00000001 },
  { "pathremove", e_pathremove, f_pathremove, 2, 3, e_pathappend, 0x00000001 },
  { "pathprepend", e_pathprepend, f_pathprepend, 2, 3, e_pathremove, 0x00000001 },
  { "pathset", e_pathset, f_pathset, 2, 2, e_envunset, 0x00000001 },
  { "addalias", e_addalias, f_addalias, 2, 2, e_unalias, 0x00000001 },
  { "unalias", e_unalias, f_unalias, 1, 1, e_invalid_cmd, 0x00000001 },
  { "sourcerequired", e_sourcerequired, f_sourcerequired, 2, 3, e_sourceoptional, 0x00000001 },
  { "sourceoptional", e_sourceoptional, f_sourceoptional, 2, 3, e_sourceoptional, 0x00000001 },
  { "sourcereqcheck", e_sourcereqcheck, f_sourcereqcheck, 2, 3, e_sourceoptcheck, 0x00000001 },
  { "sourceoptcheck", e_sourceoptcheck, f_sourceoptcheck, 2, 3, e_sourceoptcheck, 0x00000001 },
  { "exeaccess", e_exeaccess, f_exeaccess, 1, 1, e_invalid_cmd, 0x00000001 },
  { "execute", e_execute, f_execute, 2, 3, e_invalid_cmd, 0x00000001 },
  { "filetest", e_filetest, f_filetest, 2, 3, e_invalid_cmd, 0x00000001 },
  { "copyhtml", e_copyhtml, f_copyhtml, 0, 0, e_invalid_cmd, 0x00000000 },
  { "copyinfo", e_copyinfo, f_copyinfo, 0, 0, e_invalid_cmd, 0x00000000 },
  { "copyman", e_copyman, f_copyman, 0, 0, e_uncopyman, 0x00000000 },
  { "uncopyman", e_uncopyman, f_uncopyman, 0, 0, e_copyman, 0x00000000 },
  { "copycatman", e_copycatman, f_copycatman, 0, 0, e_uncopycatman, 0x00000000 },
  { "uncopycatman", e_uncopycatman, f_uncopycatman, 0, 0, e_copycatman, 0x00000000 },
  { "copynews", e_copynews, f_copynews, 0, 0, e_invalid_cmd, 0x00000000 },
  { "writecompilescript", e_writecompilescript, f_writecompilescript, 2, 3, e_invalid_cmd, 0x00000001 },
  { "dodefaults", e_dodefaults, f_dodefaults, 0, 1, e_dodefaults, 0x00000001 },
  { "setupenv", e_setupenv, f_setupenv, 0, 0, e_unsetupenv, 0x00000001 },
  { "proddir", e_proddir, f_proddir, 0, 2, e_unproddir, 0x00000001 },
  { "unsetupenv", e_unsetupenv, f_unsetupenv, 0, 0, e_setupenv, 0x00000001 },
  { "unproddir", e_unproddir, f_unproddir, 0, 1, e_proddir, 0x00000001 },
  { "if", e_if, f_if, 1, 1, e_endunless, 0x00000001},
  { "endif", e_endif, f_endif, 1, 1, e_unless, 0x00000001 },
  { "else", e_else, f_else, 0, 0, e_else, 0x00000001 },
  { "unless", e_unless, f_unless, 1, 1, e_endif, 0x00000001 },
  { "endunless", e_endunless, f_endif, 1, 1, e_if, 0x00000001 },
  { 0,0,0,0,0, 0x00000000 }
};

/*=======================================================================
 *
 * Public functions
 *
 *=====================================================================*/

/*-----------------------------------------------------------------------
 * upsact_print
 *
 * will call ups_get_cmd to get list of dependencies of a product, and 
 * print it to stdout.
 *
 * Input : t_upsugo_command *, a ugo_command
 *         t_upstyp_matched_product *, a matched product (can be null).
 *         char *, name of action.
 *         int, enum of ups command.
 *         char *, options, 'l': it will print all action commands for all instances,
 *                               default is to only print instances.
 * Output: none
 * Return: t_upsact_tree *,
 */
int upsact_print( t_upsugo_command * const ugo_cmd,
		  t_upstyp_matched_product * const mat_prod_in,
		  const char * const act_name,
		  const int ups_cmd,
		  char * sopt )
{
  static char s_sopt[] = "";
  t_upslst_item *dep_list = 0;
  t_upslst_item *dep_l = 0;

  g_ups_cmd = ups_cmd;
  g_ugo_cmd = ugo_cmd;
  g_top_unsetup = 0;

  if ( !sopt )
    sopt = s_sopt;

  if ( !ugo_cmd || !act_name )
    return 0;

  /* get depency list */

  if ( strchr( sopt, 'l' ) )
    dep_list = upsact_get_cmd( ugo_cmd, mat_prod_in, act_name, ups_cmd );
  else
    dep_list = upsact_get_dep( ugo_cmd, mat_prod_in, act_name, ups_cmd );

  /* option l: print all functions for all dependencies */

  if ( strchr( sopt, 'l' ) ) {

    for ( dep_l = upslst_first( dep_list); dep_l; dep_l = dep_l->next )
      upsact_print_item( (t_upsact_item *)dep_l->data, sopt );

  }

  else {
    t_upstyp_matched_product *mat_prod = 0;
    t_upstyp_matched_instance *mat_inst = 0;
    t_upslst_item *l_mproduct = 0, *l_hlp;
    t_upslst_item *old_ugo_key;
    t_upsact_item dep_act_itm;
    int i;
    int double_line = 0;

    for ( dep_l = upslst_first( dep_list ); dep_l; dep_l = dep_l->next ) {

      t_upsact_item *act_ptr = (t_upsact_item *)dep_l->data;
      t_upsugo_command *dep_ugo = act_ptr->dep_ugo;      

      l_mproduct = upsmat_instance( act_ptr->dep_ugo, NULL, 1 );

      if ( !l_mproduct || !l_mproduct->data ) {
	(void) upsutl_free_matched_product_list( &l_mproduct );
	continue;	
      }

      mat_prod = (t_upstyp_matched_product *)l_mproduct->data;
      l_hlp = upslst_first( mat_prod->minst_list );
      mat_inst = l_hlp ? (t_upstyp_matched_instance *)(l_hlp->data) : 0;
      
      if ( strchr( sopt, 'K' ) ) {
	
	/* option K shoule be easy to parse */
	
	old_ugo_key = dep_ugo->ugo_key;
	dep_ugo->ugo_key = ugo_cmd->ugo_key;
	
	list_K( mat_inst, dep_ugo, mat_prod, 1 );
	
	dep_ugo->ugo_key = old_ugo_key;
      }
      else {
	static char s_indent[MAX_LINE_LEN];
	dep_act_itm.ugo = dep_ugo;
	dep_act_itm.mat = mat_prod;
	
	
	/* print indentations,
	   since we are trying to be pretty we have to decide when to 
	   print a '|   ' or a '    ' */
	
	s_indent[0] = '\0';
	
	for ( i=0; i<act_ptr->level - 1; i++ ) {
	  
	  t_upslst_item *l_h = dep_l->next;
	  char *sc = "   ";
	  
	  for ( ; l_h; l_h = l_h->next ) {
	    int lvl = ((t_upsact_item *)l_h->data)->level;
	    if ( lvl == i + 1 ) {
	      sc = "|  "; break;
	    }
	    else if ( lvl < i + 1 ) {
	      sc = "   "; break;
	    }
	  }
	  
	  (void) strcat( s_indent, sc );
	}
	
	if ( act_ptr->level > 0 ) {
	  if ( double_line ) {
	    (void) printf( "%s|  \n", s_indent );
	    (void) printf( "%s|__", s_indent );
	  }
	  else {
	    (void) printf( "%s|__", s_indent );
	  }
	}
	
	(void) printf( "%s\n", actitem2str( &dep_act_itm) );

      }

      (void) upsutl_free_matched_product_list( &l_mproduct );
    }
    
  }

  upsact_cleanup( upslst_first( dep_list ) );
  return 1;
}

/*-----------------------------------------------------------------------
 * upsact_get_dep
 *
 * Will get list of dependencies for a product.
 *
 * Input : t_upsugo_command *, a ugo_command.
 *         t_upstyp_matched_product *, a matched product (can be null).
 *         char *, action name.
 *         int, enum of ups command.
 * Output: none
 * Return: t_upslst_item *, a list of t_upsact_item's.
 */
t_upslst_item *upsact_get_dep( t_upsugo_command * const ugo_cmd,
			       t_upstyp_matched_product * const mat_prod,
			       const char * const act_name,
			       const int ups_cmd )
{
  t_upsact_item *act_itm;
  t_upslst_item *top_list = 0;
  t_upslst_item *dep_list = 0;

  g_ups_cmd = ups_cmd;
  g_ugo_cmd = ugo_cmd;
  g_top_unsetup = 0;
  g_prod_done = 0;

  if ( !ugo_cmd || !act_name )
    return 0;

  /* create a partial action structure for top product */

  if ( !(act_itm = get_top_item( ugo_cmd, mat_prod, act_name )) ) {
    upserr_vplace();
    upserr_add( UPS_NO_PRODUCT_FOUND, UPS_FATAL, ugo_cmd->ugo_product );
    return 0;
  }

  /* create a list of 1'st level dependecies,
     these instances have precedence over products at lower (higher ?) levels */

  g_act_itm = act_itm;
  top_list = next_top_prod( top_list, act_itm, act_name );
  
  /* add top product to dep list */

  act_itm->dep_ugo = ugo_cmd;
  dep_list = upslst_add( dep_list, act_itm );

  /* get the dependency list */

  dep_list = next_cmd( top_list, dep_list, act_itm, act_name, 'd' );

  /* clean up top list */

  upsact_cleanup( top_list );

  (void) upslst_free( g_prod_done, ' ' );

  return upslst_first( dep_list );
}

/*-----------------------------------------------------------------------
 * upsact_get_cmd
 *
 * Will get list of action commands for a product, following dependencies.
 *
 * Input : t_upsugo_command *, a ugo_command.
 *         t_upstyp_matched_product *, a matched product (can be null).
 *         char *, action name.
 *         int, enum of ups command.
 * Output: none
 * Return: t_upslst_item *, a list of t_upsact_item's.
 */
t_upslst_item *upsact_get_cmd( t_upsugo_command * const ugo_cmd,
			       t_upstyp_matched_product * const mat_prod,
			       const char * const act_name,
			       const int ups_cmd )
{
  t_upsact_item *act_itm;
  t_upslst_item *top_list = 0;
  t_upslst_item *dep_list = 0;

  g_ups_cmd = ups_cmd;
  g_ugo_cmd = ugo_cmd;
  g_top_unsetup = 0;
  g_prod_done = 0;

  if ( !ugo_cmd || !act_name )
    return 0;


  /* create a partial action structure for top product */

  if ( !(act_itm = get_top_item( ugo_cmd, mat_prod, act_name )) ) {
    upserr_vplace();
    upserr_add( UPS_NO_PRODUCT_FOUND, UPS_FATAL, ugo_cmd->ugo_product );
    return 0;
  }

  if (mat_prod && mat_prod->db_info && mat_prod->db_info && mat_prod->db_info->config &&
      mat_prod->db_info->config->strict_deps && mat_prod->db_info->config->strict_deps[0] == '1') {
          g_ugo_cmd->ugo_B = 1;
          P_VERB_s( 3, "Turning on -B because of strict_deps" );
  }

  if (g_ugo_cmd->ugo_B && check_setup_match_clash( mat_prod )) {
     return 0;
  }
  /* create a list of 1'st level dependecies,
     these instances have precedence over products at lower (higher ?) levels */

  g_act_itm = act_itm;
  top_list = next_top_prod( top_list, act_itm, act_name );
  
  /* get the dependency list */

  

  dep_list = next_cmd( top_list, dep_list, act_itm, act_name, 'c' );

  /* clean up top list */

  upsact_cleanup( top_list );

  (void) upslst_free( g_prod_done, ' ' );

  return upslst_first( dep_list );
}

/*-----------------------------------------------------------------------
 * upsact_trim_unsetup
 *
 * It will return a list of action items, which have to be executed for an
 * unsetup. It will just remove all items from an action list which has
 * not the unsetup flag set.
 *
 * Input : t_upslst_item *, a list of action items
 * Output: int *, will be zero if top product was not setup'ed
 *                note: that flag is only valid, if upsact_trim_unsetup
 *                is been called right after a upsact_get_cmd.
 * Return: t_upslst_item *, a list of action items
 */
t_upslst_item *upsact_trim_unsetup( t_upslst_item * const act_list,
				    int * const top_unsetup )
{
  t_upslst_item *l_s = 0, *l_i = upslst_first( act_list );
  
  while ( l_i ) {
    t_upsact_item *itm = (t_upsact_item *)l_i->data;
    if ( !itm->unsetup ) {
      l_i = upslst_delete_safe( l_i, itm, 'd' );
    }
    else {
      l_s = l_i;
      l_i = l_i->next;
    }
  }

  *top_unsetup = g_top_unsetup;

  return upslst_first( l_s );
}

/*-----------------------------------------------------------------------
 * upsact_parse_cmd
 *
 * It will translate a single action command line.
 * The returned t_upsact_cmd structure is allocated on the free store.
 *
 * Input : action line.
 * Output: none
 * Return: t_upsact_cmd *, a translated command line
 */
t_upsact_cmd *upsact_parse_cmd( const char * const cmd_str )
{
  static char trim_chars[] = " \t\n\r\f)";
  t_upsact_cmd *pcmd;
  char *act_s = (char *)cmd_str, *act_e = NULL, *act_p = NULL;
  int icmd = e_invalid_action;
  int i;
  int len;

  /* get rid of leading spaces */
  
  while ( act_s && *act_s && isspace( (unsigned long )(*(act_s)) ) ){ ++act_s; }

  if ( !act_s || !*act_s )
    return 0;

  /* create a new command structure, with space for the command string.
     the created argv array is just a list of pointers into that string. */
  
  pcmd = upsact_new_upsact_cmd( act_s );
  act_s = pcmd->pbuf;

  /* get name of action command */

  if ( (act_p = strchr( act_s, OPEN_PAREN )) != NULL ) {

    /* save pointer to parenthese and
       get rid of trailing spaces in name of action command */

    act_e = act_p - 1;
    while ( act_e && *act_e && isspace( (unsigned long )(*(act_e)) ) ){ --act_e; }    
    len = act_e - act_s + 1;

    /* look for action command in the supported action array */
    
    for ( i = 0; g_func_info[i].cmd; ++i ) {

	if ( len != (int )strlen(g_func_info[i].cmd) )
	continue;

      if ( !upsutl_strincmp( act_s, g_func_info[i].cmd, (size_t)len ) ) {

	/* check if function is valid for this file */

	if ( ! UPSACT_FUNC_ISIN_TABLE( g_func_info[i].flags ) ) {
	  upserr_vplace();
	  upserr_add( UPS_ACTION_PARSE_INVALID, UPS_FATAL, g_func_info[i].cmd, "table files" );
	  break;
	}

	/* create a pointer to a string with these parameters.
	   note - it does not include an open parenthesis */

	act_s = act_p + 1;
		 
	/* trim off whitespace & the ending ")", we will also get rid
           of ending ';' */

	(void) upsutl_str_remove_edges( act_s, " ;" );
	(void) upsutl_str_remove_edges( act_s, trim_chars );

        /* save the location in the array */

	icmd = g_func_info[i].icmd;
	
	break;
      }
    }
  }

  /* split parameter string into a list of arguments (fill argv) */
  
  if ( icmd != e_invalid_action ) {
    pcmd->icmd = icmd;
    pcmd->argc = parse_params( act_s, pcmd->argv );

    if ( UPS_VERBOSE ) {
      (void) strcpy( g_buff, g_func_info[icmd].cmd );
      (void) strcat( g_buff, "(" );
      for ( i = 0; i < pcmd->argc; i++ ) {
	(void) strcat( g_buff, pcmd->argv[i] );
	if ( i < pcmd->argc-1 )
	  (void) strcat( g_buff, ", " );
      }
      (void) strcat( g_buff, ")" );
      P_VERB_s_s( 3, "Parsed line:", g_buff );
    }

    return pcmd;
  }
  else {
    upsmem_free( pcmd );
    P_VERB_s( 3, "Parsed line: (null)" );
  }
  return 0;
}

void upsact_cleanup( t_upslst_item *dep_list )
{
  t_upslst_item * l_ptr = upslst_first( dep_list );

  if ( !l_ptr )
    return;

  while( l_ptr ) {
    t_upsact_item *act_ptr = l_ptr->data;
    l_ptr = upslst_delete( l_ptr, act_ptr, ' ' );
    upsact_free_act_item( act_ptr );
  }
}


/*-----------------------------------------------------------------------
 * upsact_print_item
 *
 * It will print a t_upsact_item to stdout.
 *
 * options (sopt):
 *    always : print correspoding product instance.
 *    'l'    : also print corresponding action commands
 *    't'    : also print indentions (corresponding to level)
 *
 * Input : t_upsact_item *, action item
 *         char *, sopt 
 * Output: none
 * Return: none
 */
void upsact_print_item( const t_upsact_item *const act_itm, 
			char * sopt )
{
  static char s_sopt[] = "";
  int i;
  if ( !sopt )
    sopt = s_sopt;

  if ( !act_itm )
    return;

  if ( strchr( sopt, 't' ) ) for ( i=0; i<act_itm->level; i++ ) { (void) printf( "   " ); }
  (void) printf( "%s", actitem2str( act_itm ) );
  if ( strchr( sopt, 'l' ) ) {
    (void) printf( g_default_delimiter );
    upsact_print_cmd( act_itm->cmd );
  }
  else 
    (void) printf( "\n" );
}

/*-----------------------------------------------------------------------
 * upsact_print_cmd
 *
 * It will print a t_upsact_cmd to stdout.
 *
 * Input : t_upsact_cmd *, and action command
 * Output: none
 * Return: none
 */
void upsact_print_cmd( const t_upsact_cmd * const cmd_cur )
{
  int i;
  int icmd;
  
  if ( !cmd_cur ) {
    (void) printf( "\n" ); 
    return;
  }

  icmd = cmd_cur->icmd;
  
  (void) printf( "%s(", g_func_info[icmd].cmd );
  for ( i = 0; i < cmd_cur->argc; i++ ) {
    if ( i == cmd_cur->argc - 1 ) 
      (void) printf( "%s", cmd_cur->argv[i] );
    else
      (void) printf( "%s, ", cmd_cur->argv[i] );
  }
  (void) printf( ")\n" ); 
}

/*-----------------------------------------------------------------------
 * upsact_new_upsact_cmd
 *
 * It will create a new upsact_cmd structure
 *
 * Input : char*, a string representing an action command
 * Output: none
 * Return: t_upsact_cmd *, a pointer to an action command structure
 */
t_upsact_cmd *upsact_new_upsact_cmd( const char * const act_str )
{
  t_upsact_cmd *cmd_ptr = 0;

  cmd_ptr = (t_upsact_cmd *)upsmem_malloc( (int )(sizeof( t_upsact_cmd ) + strlen( act_str ) + 1) );
  cmd_ptr->pbuf = (char *)(cmd_ptr + 1);
  
  (void) strcpy( cmd_ptr->pbuf, act_str );

  return cmd_ptr;
}

/*-----------------------------------------------------------------------
 * upsact_free_act_cmd
 *
 * It will free a single t_upsact_cmd
 *
 * Input : t_upsact_cmd *, a pointer to an action command structure
 * Output: none
 * Return: none
 */
void upsact_free_act_cmd( t_upsact_cmd * const act_cmd )
{
  if ( act_cmd )
    upsmem_free( act_cmd );
}

void upsact_free_act_item( t_upsact_item * const act_itm ) 
{

  if ( act_itm ) {

    upsmem_dec_refctr( act_itm );

    if ( upsmem_get_refctr( act_itm ) > 1 )
      return;
    
    if ( act_itm->ugo ) {
      upsmem_dec_refctr( act_itm->ugo );
      if ( upsmem_get_refctr( act_itm->ugo ) <= 0 )
	(void) upsugo_free( act_itm->ugo );
    }
       
    if ( act_itm->mat ) {      
      upsmem_dec_refctr( act_itm->mat );
      if ( upsmem_get_refctr( act_itm->mat ) <= 0 ) {
	/* (void) printf( "*** deleting a act_item %d, mat pointer %x\n",
		(unsigned long)act_itm, (unsigned long)act_itm->mat ); */
	(void) ups_free_matched_product( act_itm->mat );
      }
    }

    /* don't touch the t_upstyp_action */

    if ( act_itm->cmd ) {
      upsmem_dec_refctr( act_itm->cmd );
      upsact_free_act_cmd( act_itm->cmd );
    }

    upsmem_free( act_itm );
  }
}

int upsact_action2enum( const char * const act_name )
{
  /* it will map an action name to the corresponding enum */

  int iact = e_invalid_action, i = 0;

  if ( act_name ) {
    for ( i=0; g_cmd_info[i].cmd; i++ ) {
      if (! upsutl_stricmp(g_cmd_info[i].cmd, act_name)) {
	iact = g_cmd_info[i].cmd_index;
	break;
      }
    }
  }
  
  return iact;
}

/*-----------------------------------------------------------------------
 * upsact_check_files
 *
 * Check to see if the passed action command contains references to any
 * files.  
 *
 * Input : an action command
 * Output: none
 * Return: pointer to a list of file names
 */
t_upslst_item *upsact_check_files(
			    const t_upstyp_matched_product * const a_mproduct,
			    const t_upsugo_command * const a_command_line,
			    char *const a_cmd)
{
  t_upsact_cmd *parsed_cmd = NULL;
  t_upslst_item *file_list = NULL;
  char *new_string = NULL;
  char *trans_cmd = NULL;
  
  if (a_cmd) {
    /* translate any ${UPS...} variables */
    trans_cmd = upsget_translation(
                    (t_upstyp_matched_instance *)a_mproduct->minst_list->data,
		    a_mproduct->db_info, a_command_line, a_cmd);

    /* parse the command */
    parsed_cmd = upsact_parse_cmd(trans_cmd);
    if (parsed_cmd) {
      /* command was successfully parsed.  if there were no arguments, none 
	 of them can be files (;-) */
      if (parsed_cmd->argc > 0) {
	/* there are arguments, now depending on the command, one of these
	   arguments may or may not be a file. only look at the argument if
	   the action parsed is capable of having a file as an argument */
	switch (parsed_cmd->icmd) {
	  /* none of these actions have associated files with them */
	case e_invalid_cmd :
	case e_setupoptional:
	case e_setuprequired:
	case e_unsetupoptional:
	case e_unsetuprequired:
	case e_envappend:
	case e_envremove:
	case e_envprepend:
	case e_envset:
	case e_envsetifnotset:
	case e_envunset:
	case e_pathappend:
	case e_pathremove:
	case e_pathprepend:
	case e_pathset:
	case e_addalias:
	case e_unalias:
	case e_filetest:
	case e_dodefaults:
	case e_setupenv:
	case e_proddir:
	case e_unsetupenv:
	case e_unproddir:
	case e_exeaccess:
	case e_uncopyman:
	  break;
	  /* the following actions contain a file or directory path.  return
	     this value to the calling routine as a list element. */
	case e_writecompilescript:
	case e_sourcecompilereq:
	case e_sourcecompileopt:
	case e_sourcerequired:
	case e_sourceoptional:
	case e_sourcereqcheck:
	case e_sourceoptcheck:
	case e_copyhtml:
	case e_copyinfo:
	case e_copyman:
	case e_copynews:
	  if (parsed_cmd->argv[0]) {
	    new_string = upsutl_str_create(parsed_cmd->argv[0],
					   STR_TRIM_DEFAULT);
	    file_list = upslst_insert(file_list, new_string);
	  }
	  break;
	  /* if there is a path included when specifying the command to execute
	     then include this file, else we don't know the location as the
	     executable is assumed to be in the user's path. in the last case,
	     do not report any files mentioned here. */
	case e_execute:
	  if (parsed_cmd->argv[0] && 
	      (! strncmp(parsed_cmd->argv[0], SLASH, 1))) {
	    /* the first letter was a slash, so we assume that this string
	       contains a directory spec and a file name.  we return this to
	       the calling routine.  if the first letter was not a slash then
	       we cannot really tell where the binary is located as it is
	       either a relative path or assumed to be in the PATH environment
	       variable */
	    new_string = upsutl_str_create(parsed_cmd->argv[0],
					   STR_TRIM_DEFAULT);
	    file_list = upslst_insert(file_list, new_string);
	  }
	  break;
	}
      }
    }
  }

  return(file_list);
}

/*=======================================================================
 *
 * Private functions
 *
 *=====================================================================*/

/*-----------------------------------------------------------------------
 * next_top_prod
 *
 * Will fill the passed list of action item with translated actions.
 * It will not follow depencies. It's very close to 'next_cmd'.
 * It's recursive.
 *
 * Input : t_upsact_item *, action item
 *         char *, action name
 * Output: none
 * Return: t_upslst_item *, a list of top action items
 */
t_upslst_item *next_top_prod( t_upslst_item * top_list,
			      t_upsact_item *const p_act_itm, 
			      const char *const act_name )
{
  t_upslst_item *l_cmd = 0;
  t_upsact_cmd *p_cmd = 0;
  char *p_line = 0;
  int i_act = e_invalid_action;
  int i_cmd = e_invalid_cmd;

  P_VERB_s_s_s_s( 3, "Action top(", act_name, ") for:", actitem2str( p_act_itm ) );

  if ( p_act_itm && p_act_itm->act ) {
    i_act = upsact_action2enum( act_name );
    l_cmd = p_act_itm->act->command_list;
  }
  else {
    return top_list;
  }

  for ( ; l_cmd; l_cmd = l_cmd->next ) {

    P_VERB_s_s( 3, "Parsing line:", l_cmd->data );
  
    p_line = upsget_translation(
		 (t_upstyp_matched_instance *)p_act_itm->mat->minst_list->data,
		 p_act_itm->mat->db_info, p_act_itm->ugo,
		 (char *)l_cmd->data );

    p_cmd = upsact_parse_cmd( p_line );
    i_cmd = p_cmd ? p_cmd->icmd : e_invalid_cmd;

    if ( i_cmd == e_invalid_cmd ) {
      SET_PARSE_ERROR( p_line );
      continue;
    }
    else if ( i_cmd > e_exeactionrequired ) {

      /* STANDARD action commands to be added to the list
	 ================================================ */

      continue;
    }
    if ( i_cmd > e_unsetuprequired ) {

      /* EXECUTE another action
	 ====================== */

      char *action_name = (char *)act_name;
      t_upsact_item *new_act_itm = 0;
      
      /* quit if option P set and optional command */

      if ( p_act_itm->ugo->ugo_R && !(i_cmd & 1) )
	upserr_backup();
	continue;

      new_act_itm = new_act_item( p_act_itm->ugo, p_act_itm->mat,  
				  p_act_itm->level, i_cmd, p_cmd->argv[0] );

      /* ignore errors if optional exeaction */

      if ( !new_act_itm || !new_act_itm->act ) {
	if ( i_cmd & 1 ) {	  
	  SET_NO_ACTION_ERROR( p_cmd->argv[0] );
	  SET_PARSE_ERROR( p_line );
	}
	else {
	  upserr_backup();
	}
	upsact_free_act_item( new_act_itm );
	continue;
      }

      /* note: action name is parents action name */

      P_VERB_s_s( 3, "Execute action:", p_line );
      top_list = next_top_prod( top_list, new_act_itm,  action_name );
    }
    else if ( !(p_act_itm->ugo->ugo_j) ) {

      /* DEPENDENCIES
	 ============ */

      t_upsact_item *new_act_itm = 0;
      t_upsugo_command *new_ugo = 0;
      int unsetup_flag = 0;
      p_cmd->iact = i_act;

      /* quit if option P set and optional command */

      if ( p_act_itm->ugo->ugo_R && !(i_cmd & 1) )
	continue;

      /* get the ugo command */

      new_ugo = get_dependent( p_cmd, 0, &unsetup_flag ); 

      /* get the action item */

      if ( i_cmd & 2 ) 
	new_act_itm = new_act_item( new_ugo, 0, 1, i_cmd, "unsetup");
      else
	new_act_itm = new_act_item( new_ugo, 0, 1, i_cmd, "setup");

      if ( !new_act_itm ) {
	if ( i_cmd & 1 && ! i_cmd & 2 ) {
	  SET_PARSE_ERROR( p_line );
	}
	else {
	  upserr_backup();
	}
      }
      else {

	/* add a un/setup action item (product) */

	new_act_itm->unsetup = unsetup_flag;
	top_list = upslst_add( top_list, new_act_itm );
      }
    }
  }

  return upslst_first( top_list );
}

/*-----------------------------------------------------------------------
 * next_cmd
 *
 * Here is the work for creating a list of dependencies. 
 * It's recursive.
 *
 * Input : t_upslst_item *, list of 1'st level products (t_upsact_item's).
 *         t_upslst_item *, current list of dependent products (t_upsact_item's).
 *         t_upsact_item *, current t_upsact_item.
 *         char *, current name of action.
 *         char, option, 'd': will make a list of only depencies
 *               else : will make a list of commands
 *         
 * Output: none
 * Return: t_upsact_cmd *,
 */
t_upslst_item *next_cmd( t_upslst_item * const top_list,
			 t_upslst_item *dep_list,
			 t_upsact_item *const p_act_itm,
			 const char *const act_name,
			 const char copt)
{
  t_upslst_item *l_cmd = 0;
  t_upsact_cmd *p_cmd = 0;
  char *p_line = 0;
  int i_act = e_invalid_action;
  int i_cmd = e_invalid_cmd;
  int is_act_build = 0;

  if ( !p_act_itm )
    return dep_list;

  i_act = upsact_action2enum( act_name );

  /* add product to list of products we have done */

  g_prod_done = upslst_add( g_prod_done, p_act_itm->ugo );

  if ( !p_act_itm->act ) {
    if ( copt == 'd' ) {
      
      return dep_list;
    }
    else {
      
      /* if we don't have an action, take care of defaults cases, controlled by: 
	 g_cmd_info.flags, g_cmd_info.uncmd_index and g_func_info.icmd_undo */

      if ( !(p_act_itm->act = new_default_action( p_act_itm, act_name, i_act )) ) {
	P_VERB_s_s_s_s_s( 3, "Action(", act_name, ") for:", actitem2str( p_act_itm ),
			  "(null)" );
	return dep_list;
      }

      is_act_build = 1;
    }
  }
  
  P_VERB_s_s_s_s( 3, "Action(", act_name, ") for:", actitem2str( p_act_itm ) );

  /* process the command functions */

  l_cmd = p_act_itm->act->command_list;
  for ( ; l_cmd; l_cmd = l_cmd->next ) {

    P_VERB_s_s( 3, "Parsing line:", (char *)l_cmd->data );
  
    /* translate and parse command */

    p_line = upsget_translation(
		 (t_upstyp_matched_instance *)p_act_itm->mat->minst_list->data,
		 p_act_itm->mat->db_info, p_act_itm->ugo,
		 (char *)l_cmd->data );
    p_cmd = upsact_parse_cmd( p_line );

    if ( !p_cmd || p_cmd->icmd == e_invalid_cmd ) {
      SET_PARSE_ERROR( p_line );
      continue;
    }

    p_cmd->iact = i_act;
    i_cmd = p_cmd->icmd;

    if ( i_cmd > e_rev_exeactionrequired ) {

      /* STANDARD action commands to be added to the list
	 ================================================ */

      if ( copt != 'd' ) {

	t_upsact_item *new_act_itm = 0;

	/* SPECIAL case, if we are in compile mode, we should ignore
	   the source compile function */

	if ( g_COMPILE_FLAG && 
	     (i_cmd == e_sourcecompilereq || i_cmd == e_sourcecompileopt) ) {

	  /* if this is the very top product, we should remove all functions above
             the source compile  */

	  if ( p_act_itm->ugo == g_ugo_cmd ) {
	    upsact_cleanup( dep_list );
	    dep_list = 0;
	  }

	  continue;
	}

	new_act_itm = copy_act_item( p_act_itm );
	new_act_itm->cmd = p_cmd;
	dep_list = upslst_add( dep_list, new_act_itm );

	/* check if we should continue, we will not exit if we are
	   fetching dependencies */

	if ( do_exit_action( p_cmd ) && (g_ups_cmd != e_depend) )
	  break;
      }

    }
    else if ( i_cmd > e_unsetuprequired ) {

      /* EXECUTE another action
	 ====================== */

      char *action_name = (char *)act_name;
      t_upsact_item *new_act_itm = 0;
      
      /* quit if option P set and optional command */

      if ( p_act_itm->ugo->ugo_R && !(i_cmd & 1) )
	continue;

      /* note: level is not incremented */

      new_act_itm = new_act_item( p_act_itm->ugo, p_act_itm->mat,  
				  p_act_itm->level, i_cmd, p_cmd->argv[0] );
      new_act_itm->unsetup = p_act_itm->unsetup;

      /* ignore errors if optional exeaction */

      if ( !new_act_itm || !new_act_itm->act ) {
	if ( i_cmd & 1 ) {
	  SET_NO_ACTION_ERROR( p_cmd->argv[0] );
	  SET_PARSE_ERROR( p_line );
	}
	else {
	  upserr_backup();
	}
	upsact_free_act_item( new_act_itm );
	continue;
      }

      /* SPECIAL, if ghost commands e_rev_exeactionoptional or e_rev_exeactionrequired 
         then reverse commands */

      if ( i_cmd == e_rev_exeactionoptional || 
	   i_cmd == e_rev_exeactionrequired ) {

	t_upstyp_action *old_act = new_act_itm->act;
	t_upstyp_action *new_act = 0;

	new_act = (t_upstyp_action *)malloc( sizeof( t_upstyp_action ) );
	new_act->action = (char *)malloc( strlen( old_act->action ) + 1 );
	(void) strcpy( new_act->action, old_act->action );
	new_act->command_list = reverse_command_list( new_act_itm, old_act->command_list );

	new_act_itm->act = new_act;
      }

      /* note: action name is parents action name */

      dep_list = next_cmd( top_list, dep_list, new_act_itm,  action_name, copt );
      P_VERB_s_s( 3, "Execute action:", p_line );
    }
    else {

      /* DEPENDENCIES
	 ============ */

      char *action_name = (char *)act_name;
      t_upsugo_command *new_ugo = 0;
      t_upsact_item *set_act_itm = 0;
      t_upsact_item *new_act_itm = 0;
      int unsetup_flag = 0;

      /* quit if option R set and optional command */

      if ( p_act_itm->ugo->ugo_R && !(i_cmd & 1) )
	continue;
      
      /* get the ugo command */

      new_ugo = get_dependent( p_cmd, is_act_build, &unsetup_flag ); 

      /* new_ugo can be null if doing unsetup
	 not any more !!!
      if ( ! new_ugo && (i_cmd & 2) )
	continue;
      */



      if ( new_ugo ) {
	 P_VERB_s( 3, "new_ugo set");
 
         /* if -B do extra dependency check stuff */

	 if ( g_ugo_cmd->ugo_B || g_command_ugoB ) {

	    int flag1 = is_prod_clash( new_ugo );
	    int flag2 = 0;
            P_VERB_s( 3, "Doing -B check...");
            if ('c' == copt) {
               flag2 = check_setup_clash( new_ugo );
               P_VERB_s_i( 3, "Saw check_setup_clash returns", flag2);
            }
	    if ((flag1 || flag2 == 1 ) &&  'c' == copt) {
                 P_VERB_s( 3, "Saw setup conflict...");
                 UPS_ERROR = UPS_SETUP_CONFLICT;
                 return 0; 
	    }
            if (flag2 == 2) {
               /* already setup, and matches, so skip it... */
               UPS_ERROR = UPS_SUCCESS;
               continue;
            }
	 } else {
            P_VERB_s( 3, "No -B flag...");
         }

         /* if product is already in our setup list, go to next product */

      }

      /* if product is at the top level always use that instance */
      /* -- unless we're doing -B, otherwise the tree won't show the */
      /*    clashing versions ... */
      
      if ( new_ugo && ! g_ugo_cmd->ugo_B ) {
	if ( (new_act_itm = 
	      find_prod_name( top_list, new_ugo->ugo_product ) ) ) {
	  /*	  (void) upsugo_free( new_ugo );
		  new_ugo = new_act_itm->ugo */
	}
      }

      if ( (!g_ugo_cmd->ugo_B || 'c' == copt) && new_ugo && is_prod_done( new_ugo->ugo_product ) ) {
        continue;
      }

      /*
      if ( new_ugo && copt == 'd' ) {
	if ( find_prod_dep_name( dep_list, new_ugo->ugo_product ) ) { 
	  continue;
	}
      }
      else if ( new_ugo ) {
	if ( find_prod_name( dep_list, new_ugo->ugo_product ) ) { 
	  continue;
	}
      }
      */
	
      /* add a dependency item */

      if ( copt == 'd' ) {
	set_act_itm = copy_act_item( p_act_itm );
	set_act_itm->cmd = p_cmd;
	if ( new_act_itm ) {
	  new_ugo = new_act_itm->ugo;
	  upsmem_inc_refctr( new_ugo );
	}

	set_act_itm->level = p_act_itm->level + 1;
	set_act_itm->dep_ugo = new_ugo;
	dep_list = upslst_add( dep_list, set_act_itm );
      }

      /* don't follow link if opt j set */

      if ( p_act_itm->ugo->ugo_j )
	continue;

      /* get action item (new_act_itm is only set if product found at the 
	 top level) */

      if ( !new_act_itm ) {

	if ( i_cmd & 2 ) 
	  action_name = g_cmd_info[e_unsetup].cmd;
	else
	  action_name = g_cmd_info[e_setup].cmd;

	new_act_itm = new_act_item( new_ugo, 0, 0, i_cmd, action_name );

	/* ignore errors if optional (un)setup */

	if ( !new_act_itm ) {
	  if ( i_cmd & 1 ) {
	    SET_PARSE_ERROR( p_line );
	  }
	  else {
	    upserr_backup();
	  }
	  continue;
	}
      }

      /* new action, increment dependency level and parse it */

      new_act_itm->unsetup = unsetup_flag;
      new_act_itm->level = p_act_itm->level + 1;
      P_VERB_s_s( 3, "Adding dependency:", p_line );
      dep_list = next_cmd( top_list, dep_list, new_act_itm, action_name, copt );

      /* write out statistics info */

      if ((copt != 'd') && (new_act_itm->unsetup || (!(i_cmd & 2))))
      {
        char setup_string [256];
        t_upslst_item mproduct;
        int i;
        char *p = setup_string;

        mproduct.next = mproduct.prev = 0;
        mproduct.data = new_act_itm->mat;
        strcpy (setup_string, actitem2str (g_act_itm));
        for (i = 0; i < 2; ++i)
        {
          p += strspn (p, " ");
          p += strcspn (p, " ");
        }
        *p = '\0';

        (void) sprintf (setup_string, "%ssetup%s %s",
                        ((i_cmd & 2) ? "un" : ""),
                        ((i_cmd & 1) ? "required" : "optional"),
                        setup_string);
        upsutl_statistics (&mproduct, setup_string);
      }
    }
  }

  return dep_list;
}

/*-----------------------------------------------------------------------
 * parse_params
 *
 * Split an action's parameter string into into separate parameters and
 * return an array of these.The parameters in the string are separated 
 * by commas, but ignore commas within quotes.
 *
 * The routine writes into passed parameter string !!!.
 *
 * Input : char *, parameter string
 *         char **, pointer to array of arguments
 * Output: char **, pointer to array of arguments
 * Return: number of arguments found
 */
int parse_params( const char * const a_params, char **argv )
{
  char *ptr = (char *)a_params, *saved_ptr = NULL;
  char *new_ptr;
  int count = 0;

  while ( ptr && *ptr ) {

    if ( count >= UPS_MAX_ARGC ) {
      upserr_vplace();
      upserr_add( UPS_TOO_MANY_ACTION_ARG, UPS_FATAL, a_params );
      return 0;
    }
    
    switch ( *ptr ) {
      
    case DQUOTE:    /* found a double quote */
      
      /* this may be the beginning of the line, saved_ptr is not set yet
	 so do it now. */
      
      if ( !saved_ptr )
	saved_ptr = ptr;           /* the beginning of a new parameter */
      
      /* found a double quote, skip to next double quote */
      
      if ( (new_ptr = strchr( ++ptr, (int)DQUOTE) ) == NULL ) {
	
	/* did not find another quote  - take the end of the line as end of
	   string and end of param list */
	
	saved_ptr = ptr;         /* no longer valid, we got the param */
	ptr = NULL;              /* all done */
      }
      else {
	
	/* point string just past double quote */
	
	ptr = ++new_ptr;
      }
      break;
      
    case COMMA:     /* found a comma */       

      if ( saved_ptr ) {
	
	/* we have a param, create a new pointer to the string */
        /* and add it to the list */
	
	*ptr = '\0';
	(void) upsutl_str_remove_end_quotes( saved_ptr, "\"\'", WSPACE );
	argv[count++] = saved_ptr;
	
      }
      ++ptr;                       /* go past the comma */
      saved_ptr = ptr;             /* start of param */
      break;
      
    default:       /* go to next char */
      
      if ( !saved_ptr ) {
	saved_ptr = ptr;           /* the beginning of a new parameter */
      }
      ++ptr;                       /* go to the next character */
    }
  }

  if ( saved_ptr ) {
    
    /* Get the last one too */

    (void) upsutl_str_remove_end_quotes( saved_ptr, "\"\'", WSPACE );
    argv[count++] = saved_ptr;
  }
  
  return count;
}

/*-----------------------------------------------------------------------
 * get_SETUP_prod
 *
 * Will (maybe) create an ugo command from the env. variable SETUP_prodname.
 *
 * If product defined on the action line have onw of the switches: version, 
 * q, f, z, H, r, m or M, then SETUP_prodname will _not_ be used. 
 *
 * Input : t_upsact_cmd *, action command line
 *         int, enum of main line action
 * Output: none
 * Return: t_upsugo_command *, a ugo command
 */
t_upsugo_command *get_SETUP_prod( t_upsact_cmd * const p_cmd, 
				  /* const int i_act, */
				  const int i_build )
{
  static char s_pname[256];
  char *pname = 0;
  int use_cmd = 0;
  char *cmd_line = 0;
  t_upsugo_command *a_cmd_ugo = 0;

  if ( !p_cmd )
    return 0;

  cmd_line = p_cmd->argv[0];

  /* fetch product name,
     if it's not a simple command, let ugo do it. */

  if ( ! strchr( cmd_line, ' ' ) ) {

    (void) strcpy( s_pname, cmd_line );
  }
  else {

    a_cmd_ugo = upsugo_bldcmd( cmd_line, g_cmd_info[e_unsetup].valid_opts );
    (void) strcpy( s_pname, a_cmd_ugo->ugo_product );

    /* check if instance can differ from SETUP_prod, if it can we will use 
       that instance, except if the instance is created from a setup action */

    if ( !i_build )
      use_cmd = 0;
    /*      use_cmd = TOO_MUCH_FOR_UNSETUP( a_cmd_ugo ); */

  }

  if ( !use_cmd ) {
    (void) upsugo_free( a_cmd_ugo );
    pname = upsutl_upcase( s_pname );

    /* fetch, from the environment the product defined in $SETUP_prod */

    return upsugo_env( pname, g_cmd_info[e_unsetup].valid_opts );
  }
  return a_cmd_ugo;
}

/*-----------------------------------------------------------------------
 * get_act
 *
 * It will from a ugo command or a macthed product fetch the corresponding
 * action (t_upstyp_action). The passed ugo command or macthed product is
 * expected to be an unique instance.
 *
 * Input : t_upsugo_command *, a ugo command
 *         t_upstyp_matched_product *, a matched product (can be null)
 *         char *, name of action
 * Output: none
 * Return: t_upstyp_action *, an action (as read from a table file)
 */
t_upstyp_action *get_act( const t_upsugo_command * const ugo_cmd,
			  t_upstyp_matched_product * mat_prod,
			  const char * const act_name )
{
  t_upstyp_matched_instance *mat_inst;

  if ( !ugo_cmd || !act_name )
    return 0;

  if ( !mat_prod ) {
    t_upslst_item *l_mproduct = upsmat_instance( (t_upsugo_command *)ugo_cmd, NULL, 1 );
    if ( !l_mproduct || !l_mproduct->data )
      return 0;

    mat_prod = (t_upstyp_matched_product *)l_mproduct->data;
    
    /*    (void) printf( "*** get_act: creating and inc a mat pointer %x\n",
	    (unsigned long)mat_prod ); */
    
    /* we keep the matched product */
       
    upsmem_inc_refctr( mat_prod );
    
    (void) upsutl_free_matched_product_list( &l_mproduct );
  }

  /* we are expecting one match, else the above should fail */

  if ( mat_prod->minst_list ) { 
    mat_inst = (t_upstyp_matched_instance *)(upslst_first( mat_prod->minst_list ))->data;
    if ( mat_inst && mat_inst->table ) 
      return upskey_inst_getaction( mat_inst->table, act_name );
  }

  return 0;
}

/* it will, for un/setup functions build the corresponding
   ugo command structure, using upsugo_bldcmd */

t_upsugo_command *get_dependent( t_upsact_cmd *const p_cmd,
			   int is_act_build,
			   int * const unsetup_flag )
{

  t_upslst_item *l_itm = 0;
  t_upsugo_command *a_ugo = 0;
  int i_cmd = p_cmd->icmd;

  *unsetup_flag = 0;

  /* handle unsetup ... that's special, we will compare command line to the
     product actually setup and get the instance from $SETUP_<prodname>.
     currently only 'ups depend' will not use the current environment. */

  if ( (i_cmd & 2) && (g_ups_cmd != e_depend) && !g_COMPILE_FLAG ) {

    if ( (a_ugo = get_SETUP_prod( p_cmd, is_act_build )) )
    {

  /* Go ahead and parse the unSetupReq/Opt buffer for the few extra options
     we could possibly care about.  Include them all in case we've gotten
     here from an automatic reversal of a setup action */

      t_upsugo_command *temp_ugo = 0;
      if ((temp_ugo = upsugo_bldcmd( p_cmd->argv[0], g_cmd_info[e_setup].valid_opts)) != 0)
      {
	if (temp_ugo->ugo_e)
	  a_ugo->ugo_e = temp_ugo->ugo_e;
	if (temp_ugo->ugo_j)
	  a_ugo->ugo_j = temp_ugo->ugo_j;
	if (temp_ugo->ugo_v)
	  a_ugo->ugo_v = temp_ugo->ugo_v;

	(void) upsugo_free( temp_ugo );
      }

      *unsetup_flag = 1;
    }
  }

  if ( !a_ugo ) {

    /* if no flavor was specified by the action function (H) and a -H
       flavor was specified on the command line, we will pass that on */

    (void) strcpy( g_buff, p_cmd->argv[0] );
    if ( !strstr( g_buff, "-H" ) && g_ugo_cmd->ugo_H ) {
      (void) strcat( g_buff, " -H " );
      l_itm = upslst_first( g_ugo_cmd->ugo_osname );
      for ( ; l_itm; l_itm = l_itm->next ) {
	(void) strcat( g_buff, l_itm->data );
	if ( l_itm->next )
	  (void) strcat( g_buff, g_default_delimiter );
      }
    }      

  /* At this point, we only have to deal with setup actions,
     regardless of what action or chain kicked things off */

    a_ugo = upsugo_bldcmd( g_buff, g_cmd_info[e_setup].valid_opts );
  }

  /* ignore UPS_NO_DATABASE if we have one from command line */

  if ( UPS_ERROR == UPS_NO_DATABASE && g_ugo_cmd->ugo_db ) {
    
    upserr_backup();
  }

  /* ugo_command can be null for unsetup */
  
  if ( !a_ugo )
    return 0;
  
  /* if no database was specified by the action function (-z) then upsugo_bldcmd
     will have picked up $PRODUCTS, which is not correct if databases (-z) was specified 
     on the command line */

  if ( !a_ugo->ugo_z && g_ugo_cmd->ugo_z ) {
    
    a_ugo->ugo_z = g_ugo_cmd->ugo_z;
    if ( a_ugo->ugo_db ) {
      (void) upsugo_free_ugo_db( a_ugo->ugo_db );
      a_ugo->ugo_db = 0;
    }
    a_ugo->ugo_db = merge_ugo_db( a_ugo->ugo_db, g_ugo_cmd->ugo_db );
  }

  return a_ugo;
}

t_upsact_item *get_top_item( t_upsugo_command * const ugo_cmd,
			     t_upstyp_matched_product *mat_prod,
			     const char * const act_name )
{
  /* it will create an action item for the top product */

  t_upsact_item *act_itm;
  int i_act = e_invalid_action;
  int i_mod = e_invalid_cmd;
  static char s_pname[256];
  char *pname = 0;
  t_upsugo_command *the_ugo_cmd = ugo_cmd;
  t_upsugo_command *setup_ugo_cmd = 0;
  int unsetup_flag = 0;

  if ( !the_ugo_cmd || !act_name )
    return 0;

  /* set a 'fake' action command mode */

  i_act = upsact_action2enum( act_name );
  if ( i_act == e_setup )
    i_mod = e_setuprequired;
  else if ( i_act == e_unsetup )
    i_mod = e_unsetuprequired;

  /* if we are doing a unsetup, fetch, from the environment, the product 
     defined in $SETUP_prod */

  if ( i_act == e_unsetup ) {

      (void) strcpy( s_pname, the_ugo_cmd->ugo_product );
      pname = upsutl_upcase( s_pname );

      if ( !g_COMPILE_FLAG && (g_ups_cmd != e_depend) )
	setup_ugo_cmd = upsugo_env( pname, g_cmd_info[e_unsetup].valid_opts );

      /* as far as the "unsetup_flag" goes, here are the two cases in which
      we figure we need to do an unsetup: 1) the env has the info, or 2) we
      have more than just the product name passed in. */
      if ( setup_ugo_cmd ) {
	setup_ugo_cmd->ugo_j = ugo_cmd->ugo_j; 
	the_ugo_cmd = setup_ugo_cmd;

	/* since we have to remach, 
	   don't use the passed matched product !!! */

	mat_prod = 0;
	unsetup_flag = 1;
      }
      else if (ugo_cmd->ugo_version) {
	unsetup_flag = 1;
      }
  }

  /* create a partial action structure for top product */

  act_itm = new_act_item( the_ugo_cmd, mat_prod, 0, i_mod, act_name );

  if ( act_itm )
    act_itm->unsetup = unsetup_flag;

  g_top_unsetup = unsetup_flag;

  return act_itm;
}

int
check_setup_match_clash(t_upstyp_matched_product * const mat_prod ) {
    t_upsugo_command *setup_ugo_cmd = 0;

    t_upstyp_matched_instance *minst = mat_prod->minst_list->data;
    t_upstyp_instance *inst = minst->version;

    P_VERB_s_s( 1, "checking for setups", mat_prod->product );

    if (!inst) {
        return 0;
    }
    setup_ugo_cmd = upsugo_env( inst->product , g_cmd_info[e_setup].valid_opts );
    if (setup_ugo_cmd) {
      P_VERB_s_s( 1, "want version", inst->version );
      P_VERB_s_s( 1, "have version", setup_ugo_cmd->ugo_version );

      if (!setup_ugo_cmd->ugo_version || (setup_ugo_cmd->ugo_version && (!inst->version || 0 != upsutl_stricmp(setup_ugo_cmd->ugo_version, inst->version)))) {
	     upserr_add( UPS_SETUP_CONFLICT, UPS_FATAL, inst->product, "version", inst->version, setup_ugo_cmd->ugo_version  );
      }
      return 1;
    } 
    return 0;
}

int
check_setup_clash(t_upsugo_command *new_ugo ) {
    t_upsugo_command *setup_ugo_cmd = 0;
    t_upslst_item *p1, *p2;
    int res = 0;

    P_VERB_s_s( 1, "checking for setups", new_ugo->ugo_product );

    setup_ugo_cmd = upsugo_env( new_ugo->ugo_product , g_cmd_info[e_setup].valid_opts );
    if (setup_ugo_cmd) {
        if (new_ugo->ugo_version) {
            /* only say we know it matches if we're being specific */
            res = 2; 
        }
	/* compare versions */
        P_VERB_s_s_s(3, "comparing versions: ",new_ugo->ugo_version, setup_ugo_cmd->ugo_version);
	if ( new_ugo->ugo_version && (!setup_ugo_cmd->ugo_version || 0 != upsutl_stricmp(new_ugo->ugo_version, setup_ugo_cmd->ugo_version))) {
           upserr_vplace();
	   upserr_add( UPS_SETUP_CONFLICT, UPS_FATAL, new_ugo->ugo_product, "version", new_ugo->ugo_version, setup_ugo_cmd->ugo_version  );
           res = 1;
	}
	/* compare qualifiers */
	p1 = new_ugo->ugo_qualifiers;
        p2=setup_ugo_cmd->ugo_qualifiers;
        if (p1 && p2 ) {
           P_VERB_s_s_s(3, "comparing qualifiers: ",(char *)(p1->data), (char *)(p2->data));
	   if ( 0 != upsutl_stricmp( p1->data, p2->data )) {
	       upserr_vplace(); 
	       upserr_add( UPS_SETUP_CONFLICT, UPS_FATAL, new_ugo->ugo_product, "qualifiers", p1->data, p2->data  );
               res = 1;
	   }
          
       } else {
	       upserr_vplace();
	       upserr_add( UPS_SETUP_CONFLICT, UPS_FATAL, new_ugo->ugo_product, "qualifiers", p1 ? p1->data:"", p2 ? p2->data: ""  );
               res = 1;
       }
       /* if it is setup and it matches, we need do nothing... */

    }
    return res;
}

/* 
 * if it has the same product name, but not the same version
 * and qualifiers as one we've already done, it is a clash.
 */
int is_prod_clash( const t_upsugo_command *const initial)
{
  t_upslst_item *l_ptr = upslst_first( g_prod_done );
  t_upslst_item *p1, *p2;
  
  for ( ; l_ptr; l_ptr = l_ptr->next ) {
    if ( upsutl_stricmp( initial->ugo_product, ((t_upsugo_command*)l_ptr->data)->ugo_product ) == 0 ) {
         if ( upsutl_stricmp( initial->ugo_version, ((t_upsugo_command*)l_ptr->data)->ugo_version ) == 0 ) {
             for (p1 = initial->ugo_qualifiers, p2=((t_upsugo_command*)l_ptr->data)->ugo_qualifiers; p1 && p2 ; p1 = p1->next, p2 = p2->next) {
                 if ( 0 != upsutl_stricmp( p1->data, p2->data )) {
		     upserr_vplace();
                     UPS_ERROR = UPS_DEP_CONFLICT;
		     upserr_add( UPS_DEP_CONFLICT, UPS_FATAL, initial->ugo_product, "qualifiers", p1->data, p2->data  );
                     return 1;
                 }
             }
             return 0;
         }
	 upserr_vplace();
	 upserr_add( UPS_DEP_CONFLICT, UPS_FATAL, initial->ugo_product, "versions", initial->ugo_version,  ((t_upsugo_command*)l_ptr->data)->ugo_version );
         UPS_ERROR = UPS_DEP_CONFLICT;
         return 1;
     }
  }
  return 0;      
}

int is_prod_done( const char *const prod_name )
{
  t_upslst_item *l_ptr = upslst_first( g_prod_done );
  for ( ; l_ptr; l_ptr = l_ptr->next ) {
    if ( upsutl_stricmp( prod_name, ((t_upsugo_command*)l_ptr->data)->ugo_product ) == 0 )
      return 1;
  }
  return 0;      
}


t_upsact_item *find_prod_name( t_upslst_item* const dep_list,
				 const char *const prod_name )
{
  /* in a list of act_item's it will find the item corresponding to
     passed product name */

  t_upslst_item *l_ptr = upslst_first( (t_upslst_item *)dep_list );
  for ( ; l_ptr; l_ptr = l_ptr->next ) {
    t_upsact_item *p_item = (t_upsact_item *)l_ptr->data;
    if ( upsutl_stricmp( prod_name, p_item->ugo->ugo_product ) == 0 )
      return p_item;
  }
  return 0;    
}

int cmp_ugo_db( const void * const d1, const void * const d2 )
{
  /* a little helper for merge_ugo_db */

  t_upstyp_db *db1 = (t_upstyp_db *)d1;
  t_upstyp_db *db2 = (t_upstyp_db *)d2;

  if ( db1 && db2 && db1->name && db2->name )
    return upsutl_stricmp( db1->name, db2->name );
  return -1;
}

t_upslst_item* merge_ugo_db( t_upslst_item * const l_db1,
			     t_upslst_item * const l_db2 )
{
  /* it will merge or copy ugo database lists, if l_db1 = 0, it will copy 
     l_db2 to l_db1, else it will merge l_db2 into l_db1  */

  t_upslst_item *l_1, *l_2;

  if ( ! l_db2 ) 
    return l_db1;

  /* copy database list */

  l_1 = upslst_first( l_db1 );
  l_2 = upslst_first( l_db2 );
  for ( ; l_2; l_2 = l_2->next ) {
    t_upstyp_db *db = (t_upstyp_db *)l_2->data;

    if ( !db )
      continue;

    /* if necessary add database */
      
    if ( !upslst_find( l_1, db, cmp_ugo_db ) ) {
      upsmem_inc_refctr( db );
      l_1 = upslst_add( l_1, db );
    }
  }
  
  return upslst_first( l_1 );
}
		 
t_upsact_item *copy_act_item( const t_upsact_item * const act_itm )
{
  /* from a passed upsact_item it will make a copy */

  t_upsact_item *new_act_itm = 0;

  new_act_itm = (t_upsact_item *)upsmem_malloc( sizeof( t_upsact_item ) );
  (void) memset( new_act_itm, 0, sizeof( t_upsact_item ) );

  new_act_itm->level = act_itm->level;
  new_act_itm->mode = act_itm->mode;
  new_act_itm->unsetup = act_itm->unsetup;

  new_act_itm->ugo = act_itm->ugo;
  upsmem_inc_refctr( act_itm->ugo );

  new_act_itm->mat = act_itm->mat;
  /* (void) printf( "*** copy_act_item: incrementing a mat pointer %x\n",
	  (unsigned long)act_itm->mat ); */
  upsmem_inc_refctr( act_itm->mat );

  /* the action pointer is original from a t_upstyp_product */     
  new_act_itm->act = act_itm->act;

  new_act_itm->cmd = act_itm->cmd;

  return new_act_itm;
}

t_upsact_item *new_act_item( t_upsugo_command * const ugo_cmd,
			     t_upstyp_matched_product *mat_prod,
			     const int level,
			     const int mode,
			     const char * const act_name )
{		       
  t_upsact_item *act_itm = 0;

  /* from a passed ugo command or a matched product it will create a 
     new action item */

  if ( !ugo_cmd )
    return 0;

  if ( !mat_prod ) {

    t_upslst_item *l_mproduct = upsmat_instance( ugo_cmd, NULL, 1 );
    if ( !l_mproduct || !l_mproduct->data ) {

      if ( 0 != strcmp(act_name, "unsetup") ) {
          upserr_vplace();
          upserr_add( UPS_NO_MATCH, UPS_FATAL, ugo_cmd->ugo_product );
      }
      return 0;
    }
    
    mat_prod = (t_upstyp_matched_product *)l_mproduct->data;
    
    /* (void) printf( "*** new_act_item: creating and inc a mat pointer %x\n",
	    (unsigned long)mat_prod );*/
    
    /* we keep the matched product */
       
    upsmem_inc_refctr( mat_prod );
    
    (void) upsutl_free_matched_product_list( &l_mproduct );
  }
  else {
    
    /* (void) printf( "*** new_act_item: incrementing a mat pointer %x\n",
	    (unsigned long)mat_prod ); */
    
    upsmem_inc_refctr( mat_prod );
    
  }

  act_itm = (t_upsact_item *)upsmem_malloc( sizeof( t_upsact_item ) );
  (void) memset( act_itm, 0, sizeof( t_upsact_item ) );

  act_itm->level = level;
  act_itm->mode = mode;

  act_itm->ugo = ugo_cmd;
  upsmem_inc_refctr( act_itm->ugo );

  act_itm->mat = mat_prod;

  if ( act_name )
    act_itm->act = get_act( ugo_cmd, mat_prod, act_name );
  act_itm->cmd = 0;

  return act_itm;
}

t_upstyp_action *new_default_action( t_upsact_item *const p_act_itm, 
				     const char * const act_name, 
				     const int iact )
{
  /* it will create a new t_upstyp_action. It will be:
     1) reverse action, if exist (g_cmd_info.uncmd_index)
     2) a dodefault() action, if default bit set (g_cmd_info.flags) 
     3) else, nothing */

  t_upstyp_action *p_unact = 0;
  t_upstyp_action *p_act = 0;
  int i_uncmd = 0;

  if ( iact == e_invalid_action )
    return 0;

  if ( (i_uncmd = g_cmd_info[iact].uncmd_index) != e_invalid_action &&
       (p_act = get_act( p_act_itm->ugo, p_act_itm->mat, g_cmd_info[i_uncmd].cmd )) ) {

    p_unact = (t_upstyp_action *)malloc( sizeof( t_upstyp_action ) );
    p_unact->action = (char *)malloc( strlen( act_name ) + 1 );
    (void) strcpy( p_unact->action, act_name );
    p_unact->command_list = reverse_command_list( p_act_itm, p_act->command_list );
  }      
  else if ( (g_cmd_info[iact].flags)&0x00000001 ) {
    p_unact = (t_upstyp_action *)malloc( sizeof( t_upstyp_action ) );
    p_unact->action = (char *)malloc( strlen( act_name ) + 1 );
    (void) strcpy( p_unact->action, act_name );
    p_unact->command_list = 0;
    p_unact->command_list = upslst_add( p_unact->command_list, 
					upsutl_str_create( "dodefaults()", ' ' ) );
  }

  return p_unact;
}

#define MAXIFDEPTH 20

t_upslst_item *reverse_command_list( t_upsact_item *const p_act_itm, 
				     t_upslst_item *const cmd_list )
{
  /* it will, given a list of action command lines, reverse each 
     command line (if g_func_info.icmd_undo is set), and return a 
     list of these. */ 

  static char buf[MAX_LINE_LEN];
  t_upslst_item *l_ucmd = 0;
  t_upslst_item *l_cmd = upslst_first( cmd_list );
  int i_uncmd = e_invalid_cmd;
  int i_cmd = e_invalid_cmd;
  int argc = 0;
  char *p_line;
  t_upsact_cmd *p_cmd;
  int i;
  int ifdepth = 0;
  int elseflags[MAXIFDEPTH];

  for ( ; l_cmd; l_cmd = l_cmd->next ) {

    /* translate and parse command */

    p_line = upsget_translation(
		 (t_upstyp_matched_instance *)p_act_itm->mat->minst_list->data,
		 p_act_itm->mat->db_info, p_act_itm->ugo,
		 (char *)l_cmd->data );

    p_cmd = upsact_parse_cmd( p_line );
    if ( p_cmd && ((i_cmd = p_cmd->icmd) != e_invalid_cmd) ) {

      if ( (i_uncmd = g_func_info[i_cmd].icmd_undo) != e_invalid_cmd ) {

        /* deal with nested if and else changing endif inverse action */

        if ((i_cmd == e_if || i_cmd == e_unless) && ifdepth < MAXIFDEPTH) {
           ifdepth++;
           elseflags[ifdepth] = 0;
        }
        if (i_cmd == e_else) {
           elseflags[ifdepth] = 1;
        }

        if (i_cmd == e_endif || i_cmd == e_endunless) {
            if ( !elseflags[ifdepth] && i_cmd == e_endif)
               i_cmd = e_endunless;
            if ( !elseflags[ifdepth] && i_cmd == e_endunless)
               i_cmd = e_endif;
           elseflags[ifdepth] = 0;
           ifdepth--;
           if (ifdepth < 0)
              ifdepth = 0;
        }

	(void) strcpy( buf,  g_func_info[i_uncmd].cmd );

	argc = g_func_info[i_cmd].max_params;
	if ( argc > g_func_info[i_uncmd].max_params )
	  argc = g_func_info[i_uncmd].max_params;
	if ( argc > p_cmd->argc )
	  argc = p_cmd->argc;

	(void) strcat( buf, "(" );
	
	/* handle first argument */

	if ( i_cmd == e_sourcerequired ||
	     i_cmd == e_sourceoptional ||
	     i_cmd == e_sourcereqcheck ||
	     i_cmd == e_sourceoptcheck ) {
	  
	  /* SPECIAL case for source*( file_path, ... ), 
	     we will prepend or remove an 'un' from file name */

	  char *fp = p_cmd->argv[0];
	  char *fn = 0;

	  /* handle directory path ... who said DOS ? */

	  if ( (fn = strrchr( fp, '/' )) ) {
	    fn++;
	    (void) strncat( buf, fp, (size_t)( fn - fp ) );
	  }
	  else
	    fn = fp;

	  /* modify file name */

	  if ( !upsutl_strincmp( fn, "un", 2 ) )
	    fn += 2; 	         /* remove 'un' */
	  else
	    (void) strcat( buf, "un" ); /* prepend 'un', always lower case */

	  (void) strcat( buf, fn );

	  /* SPECIAL case end */ 

	}
	else if ( i_cmd == e_setuprequired ||
		  i_cmd == e_setupoptional ) {

	  /* SPECIAL use a SETUP action to create an UNSETUP action */

	  if ( argc > 0 ) {
	    char *cmd_line = p_cmd->argv[0];
	    t_upsugo_command *a_cmd_ugo = 0;	    

	    if ( ! strchr( cmd_line, ' ' ) ) {
	      (void) strcat( buf, cmd_line );
	    }
	    else {
	      a_cmd_ugo = upsugo_bldcmd( cmd_line, g_cmd_info[e_unsetup].valid_opts );
	      
	      /* check if the product is setuped.
		 this is a (kind of) hack, for not losing the instance 
		 information when the product has not been setuped.
	         - if it's not setuped, use the original instance
	         - if it's setuped, use only the product name */

	      if ( (! g_COMPILE_FLAG) && upsugo_getenv( a_cmd_ugo->ugo_product ) )
		(void) strcat( buf, a_cmd_ugo->ugo_product );
	      else
		(void) strcat( buf, cmd_line );
	      (void) upsugo_free( a_cmd_ugo );
	    }
	  }
	}
	else {
	  
	  /* normal */

	  if ( argc > 0 )
	    (void) strcat( buf, p_cmd->argv[0] );
	}

	/* handle the rest of the arguments */

	for ( i=1; i<argc; i++ ) {
	  (void) strcat( buf, ", " );	    
	  (void) strcat( buf, p_cmd->argv[i] );
	}
	(void) strcat( buf, ")" );

	/* use _insert (and not _add), to reverse the order of commands */

	l_ucmd = upslst_insert( l_ucmd, 
				upsutl_str_create( buf, ' ' ) );

      }
      upsmem_free( p_cmd );      
    }    
  }

  return upslst_first( l_ucmd );
}

int do_exit_action( const t_upsact_cmd * const a_cmd ) 
{
  /* 
     this one is called when action commands are parsed in 'next_cmd'.

     it will check if passed action have an EXIT flag set:
        - by default sourcecompile has always EXIT flag set.
	- we will only exit for required commands, the decision
          to exit for optional commands are taken in the produced
          shell script itself.
  */


  int exit_flag = NO_EXIT;
  int icmd;

  if ( ! a_cmd ) 
    return 0;

  icmd = a_cmd->icmd;

  switch ( icmd ) {

  case e_sourcecompilereq:
      return 1;

  case e_sourcerequired:
  case e_sourcereqcheck:
    GET_FLAGS();
    if ( exit_flag == DO_EXIT )
      return 1;
    break;

  default:
    return 0;
  }

  return 0;
}

char *actitem2str( const t_upsact_item *const p_act_itm )
{
  /* given an act_item it will create a string representing the 
     corresponding product instance. the result is a string
     like upsget_envstr but with chain information appended */

  static char buf[MAX_LINE_LEN];
  t_upstyp_matched_instance *mat_inst;
  t_upslst_item *l_item;
  
  if ( !p_act_itm )
    return 0;

  /* instance id */

  l_item = upslst_first( p_act_itm->mat->minst_list );
  mat_inst = (t_upstyp_matched_instance *)l_item->data;
  (void) strcpy( buf, upsget_envstr( p_act_itm->mat->db_info, mat_inst, p_act_itm->ugo ) );


  /* chain information */

  l_item = upslst_first( p_act_itm->ugo->ugo_chain );
  if ( l_item && l_item->data ) {
    (void) strcat( buf, " -g " );
    (void) strcat( buf, (char *)l_item->data );
    
    /* the following should in princip never happen: a dependency should
       only be reachable by a single chain, so maybe we should print an
       error here */

    for ( l_item = l_item->next; l_item; l_item = l_item->next ) {
      if ( l_item->data ) {
	(void) strcat( buf, g_default_delimiter );
	(void) strcat( buf, (char *)l_item->data );
      }
    } 
  }

  return buf;
}

/*-----------------------------------------------------------------------
 * upsact_process_commands
 *
 * Given a list of commands, call each associated function to output the
 * shell specific code.
 *
 * Input : list of upsact_cmd items
 *         a stream to write to
 * Output: 
 * Return: none
 */
void upsact_process_commands( const t_upslst_item * const a_cmd_list,
			      const FILE * const a_stream)
{
  t_upslst_item *cmd_item;
  t_upsact_item *the_cmd;

  for (cmd_item = (t_upslst_item *)a_cmd_list ; cmd_item ;
       cmd_item = cmd_item->next ) {
    the_cmd = (t_upsact_item *)cmd_item->data;
    
    /* call the function associated with the command */
    if (g_func_info[the_cmd->cmd->icmd].func) {
      g_func_info[the_cmd->cmd->icmd].func(
		  (t_upstyp_matched_instance *)the_cmd->mat->minst_list->data,
		  the_cmd->mat->db_info, the_cmd->ugo, a_stream, the_cmd->cmd);
      if (UPS_ERROR != UPS_SUCCESS) {
	break;
      }
    }
  }
}


/* Action handling - the following routines are the ones that output shell
 *   specific code for each action supported by UPS
 */

static void f_copyhtml( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("copyHtml");
  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);
  upsver_mes(1, "%sThis action is not supported yet.\n", ACT_PREFIX);
  SHUTUP;
}

static void f_copyinfo( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("copyInfo");
  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);
  upsver_mes(1, "%sThis action is not supported yet.\n", ACT_PREFIX);
  SHUTUP;
}

static void f_copyman( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("copyMan");
  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);
  upsver_mes(1, "%sThis action is not supported yet.\n", ACT_PREFIX);
  SHUTUP;
}

static void f_copycatman( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("copyCatMan");
  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);
  upsver_mes(1, "%sThis action is not supported yet.\n", ACT_PREFIX);
  SHUTUP;
}

static void f_uncopyman( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("uncopyMan");
  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);
  upsver_mes(1, "%sThis action is not supported yet.\n", ACT_PREFIX);
  SHUTUP;
}

static void f_uncopycatman( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("uncopyCatMan");
  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);
  upsver_mes(1, "%sThis action is not supported yet.\n", ACT_PREFIX);
  SHUTUP;
}

static void f_copynews( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("copyNews");
  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);
  upsver_mes(1, "%sThis action is not supported yet.\n", ACT_PREFIX);
  SHUTUP;
}

static void f_envappend( ACTION_PARAMS)
{
  char *delimiter;
  
  CHECK_NUM_PARAM("envAppend");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    /* get the correct delimiter */
    GET_DELIMITER();

    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (g_COMPILE_FLAG) {
	/* we are being called during a compile, we need to output extra
	   stuff */
	f_envremove(a_minst, a_db_info, a_command_line, a_stream, a_cmd);
      }
      if (fprintf((FILE *)a_stream, "if [ \"${%s:-}\" = \"\" ]; then\n  %s=\"%s\"\nelse\n  %s=\"${%s}%s%s\"\nfi\nexport %s\n#\n",
		  a_cmd->argv[0], a_cmd->argv[0], a_cmd->argv[1],
		  a_cmd->argv[0], a_cmd->argv[0], delimiter, a_cmd->argv[1],
		  a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (g_COMPILE_FLAG) {
	/* we are being called during a compile, we need to output extra
	   stuff */
	f_envremove(a_minst, a_db_info, a_command_line, a_stream, a_cmd);
      }
      if (fprintf((FILE *)a_stream, "if (! ${?%s}) then\n  setenv %s \"%s\"\nelse\n  setenv %s \"${%s}%s%s\"\nendif\n#\n",
		  a_cmd->argv[0], a_cmd->argv[0], a_cmd->argv[1], 
		  a_cmd->argv[0], a_cmd->argv[0], delimiter,
		  a_cmd->argv[1]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_envprepend( ACTION_PARAMS)
{
  char *delimiter;
  
  CHECK_NUM_PARAM("envPrepend");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    /* get the correct delimiter */
    GET_DELIMITER();

    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (g_COMPILE_FLAG) {
	/* we are being called during a compile, we need to output extra
	   stuff */
	f_envremove(a_minst, a_db_info, a_command_line, a_stream, a_cmd);
      }
      if (fprintf((FILE *)a_stream, "if [ \"${%s:-}\" = \"\" ]; then\n  %s=\"%s\"\nelse\n  %s=\"%s%s${%s}\"\nfi\nexport %s\n#\n",
		  a_cmd->argv[0], a_cmd->argv[0], a_cmd->argv[1],
		  a_cmd->argv[0], a_cmd->argv[1], delimiter, a_cmd->argv[0],
		  a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (g_COMPILE_FLAG) {
	/* we are being called during a compile, we need to output extra
	   stuff */
	f_envremove(a_minst, a_db_info, a_command_line, a_stream, a_cmd);
      }
      if (fprintf((FILE *)a_stream, "if (! ${?%s}) then\n  setenv %s \"%s\"\nelse\n  setenv %s \"%s%s${%s}\"\nendif\n#\n",
		  a_cmd->argv[0], a_cmd->argv[0], a_cmd->argv[1], 
		  a_cmd->argv[0], a_cmd->argv[1], delimiter,
		  a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }

    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_envremove( ACTION_PARAMS)
{
  char *delimiter;
  
  CHECK_NUM_PARAM("envRemove");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    /* get the correct delimiter */
    GET_DELIMITER();

    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:      

      /* here we are using an extra tmp variable to evaluate the passed
	 argument, it could contain some code there has to be executed, which
	 we don't want to put into the dropit command */

      if (fprintf((FILE *)a_stream, "if [ \"${%s:-}\" != \"\" ]\nthen\n", a_cmd->argv[0]) < 0) {
        FPRINTF_ERROR();
      }

      if (fprintf((FILE *)a_stream, "  upstmp=%s\n", a_cmd->argv[1] ) < 0) {
	FPRINTF_ERROR();
      }

      if (fprintf((FILE *)a_stream,
		  "  %s=\"`%s -p \"$%s\" -i'%s' -d'%s' \"$upstmp\"`\";\n  if [ \"${%s:-}\" = \"\" ]; then unset %s; fi\n", 
		  a_cmd->argv[0], DROPIT, a_cmd->argv[0], delimiter,
		  delimiter, a_cmd->argv[0],
		  a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }

      if (fprintf((FILE *)a_stream, "  unset upstmp\nfi\n#\n" ) < 0 ) {
	FPRINTF_ERROR();
      }

      break;
    case e_CSHELL:

      /* here we are using an extra tmp variable to evaluate the passed
	 argument, it could contain some code there has to be executed, which
	 we don't want to put into the dropit command */

      if (fprintf((FILE *)a_stream, "if (${?%s}) then\n", a_cmd->argv[0]) < 0) {
        FPRINTF_ERROR();
      }

      if (fprintf((FILE *)a_stream, "  setenv upstmp \"%s\"\n", a_cmd->argv[1] ) < 0) {
	FPRINTF_ERROR();
      }

      if (fprintf((FILE *)a_stream,
		 "  setenv %s \"`%s -p \"\'\"$%s\"\'\" -i'%s' -d'%s' \"\'\"$upstmp\"\'\"`\"\n  if (\"${%s}\" == \"\") unsetenv %s\n",
		  a_cmd->argv[0], DROPIT, a_cmd->argv[0], delimiter, delimiter,
		  a_cmd->argv[0], a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }

      if (fprintf((FILE *)a_stream, "  unsetenv upstmp\nendif\n#\n" ) < 0 ) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }    
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_envset( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("envSet");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (fprintf((FILE *)a_stream, "%s=\"%s\"; export %s\n#\n", a_cmd->argv[0],
		  a_cmd->argv[1], a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "setenv %s \"%s\"\n#\n", a_cmd->argv[0],
		  a_cmd->argv[1]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_envsetifnotset( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("envSetIfNotSet");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (fprintf((FILE *)a_stream,
		  "if [ \"${%s:-}\" = \"\" ]; then %s=\"%s\"; export %s; fi;\n#\n",
		  a_cmd->argv[0], a_cmd->argv[0],
		  a_cmd->argv[1], a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "if (! ${?%s}) setenv %s \"%s\"\n#\n",
		  a_cmd->argv[0], a_cmd->argv[0], a_cmd->argv[1]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_envunset( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("envUnset");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (fprintf((FILE *)a_stream, "unset %s\n#\n", a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "unsetenv %s\n#\n", a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_exeaccess( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("exeAccess");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (fprintf((FILE *)a_stream,
		  "type %s 2>&1 | grep -i \"not found\" > /dev/null\nif [ $? -eq 0 ]\n", 
		  a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      if (fprintf((FILE *)a_stream,
		  "then\n  echo \"exeAccess failed for %s\"\n  return 1\nfi\n", /* Needs fixing */
		  a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      
      break;
    case e_CSHELL:
      if (fprintf((FILE *)a_stream,
		  "which %s |& grep %s > /dev/null\nif ($status == 1) then\n",
		  a_cmd->argv[0], a_cmd->argv[0] ) < 0) {
	FPRINTF_ERROR();
      }
      if (fprintf((FILE *)a_stream,
		  "  echo \"exeAccess failed for %s\"\n  exit 1\nendif\n", 
		  a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_execute( ACTION_PARAMS)
{
  int no_ups_env_flag = DO_NO_UPS_ENV;

  CHECK_NUM_PARAM("execute");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
    /* Determine which flags (if any) were entered */
    GET_UPS_ENV();
  
    if (UPS_ERROR == UPS_SUCCESS) {
      /* define all of the UPS local variables that the user may need. */
      if (no_ups_env_flag == DO_UPS_ENV) {
	upsget_allout(a_stream, a_db_info, a_minst, a_command_line, "  ");
	g_LOCAL_VARS_DEF = 1;   /* we defined local variables */
      }
      switch ( a_command_line->ugo_shell ) {
      case e_BOURNE:
	if (a_cmd->argc == g_func_info[a_cmd->icmd].min_params) {
	  if (fprintf((FILE *)a_stream, "%s\n#\n", a_cmd->argv[0]) < 0) {
	    FPRINTF_ERROR();
	  }
	} else {
	  if (fprintf((FILE *)a_stream, "%s=`%s`; export %s\n#\n",
		      a_cmd->argv[2], a_cmd->argv[0], a_cmd->argv[2]) < 0) {
	    FPRINTF_ERROR();
	  }
	}
	break;
      case e_CSHELL:
	if (a_cmd->argc == g_func_info[a_cmd->icmd].min_params) {
	  if (fprintf((FILE *)a_stream, "%s\n#\n", a_cmd->argv[0])< 0) {
	    FPRINTF_ERROR();
	  }
	} else {
	  if (fprintf((FILE *)a_stream, "setenv %s \"`%s`\"\n#\n",
		      a_cmd->argv[2], a_cmd->argv[0])< 0) {
	    FPRINTF_ERROR();
	  }
	}
	break;
      default:
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
      }
      if (UPS_ERROR != UPS_SUCCESS) {
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		   g_func_info[a_cmd->icmd].cmd);
      }
    }
  }
  SHUTUP;
}

static void f_filetest( ACTION_PARAMS)
{
  char *err_message;
  
  CHECK_NUM_PARAM("fileTest");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    /* get the correct error message */
    GET_ERR_MESSAGE((char *)a_cmd->argv[g_func_info[a_cmd->icmd].max_params-1]);

    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (fprintf((FILE *)a_stream,
		  "if [ ! %s %s ]; then\necho \"%s\";\nreturn 1\nfi;\n#\n", /* Needs fixing */
		  a_cmd->argv[1], a_cmd->argv[0], err_message) < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (fprintf((FILE *)a_stream,
		  "if ( ! %s %s ) then\necho \"%s\"\nexit 1\nendif\n#\n", 
		  a_cmd->argv[1], a_cmd->argv[0], err_message) < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_pathappend( ACTION_PARAMS)
{
    if (0 == strcmp(a_cmd->argv[0],g_cshPath)) {
        (void)strcpy(a_cmd->argv[0],g_shPath);
    }
    f_envappend(a_minst, a_db_info, a_command_line, a_stream, a_cmd);

    switch ( a_command_line->ugo_shell ) {
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "rehash\n#\n") < 0) {
	FPRINTF_ERROR();
      }
     }
  SHUTUP;
}

static void f_pathprepend( ACTION_PARAMS)
{
    if (0 == strcmp(a_cmd->argv[0],g_cshPath)) {
        (void)strcpy(a_cmd->argv[0],g_shPath);
    }
    f_envprepend(a_minst, a_db_info, a_command_line, a_stream, a_cmd);

    switch ( a_command_line->ugo_shell ) {
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "rehash\n#\n") < 0) {
	FPRINTF_ERROR();
      }
     }
  SHUTUP;
}

static void f_pathremove( ACTION_PARAMS)
{
    if (0 == strcmp(a_cmd->argv[0],g_cshPath)) {
        (void)strcpy(a_cmd->argv[0],g_shPath);
    }
    f_envremove(a_minst, a_db_info, a_command_line, a_stream, a_cmd);

    switch ( a_command_line->ugo_shell ) {
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "rehash\n#\n") < 0) {
	FPRINTF_ERROR();
      }
     }
  SHUTUP;
}
static void f_pathset( ACTION_PARAMS)
{
    if (0 == strcmp(a_cmd->argv[0],g_cshPath)) {
        (void)strcpy(a_cmd->argv[0],g_shPath);
    }
    f_envset(a_minst, a_db_info, a_command_line, a_stream, a_cmd);

    switch ( a_command_line->ugo_shell ) {
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "rehash\n#\n") < 0) {
	FPRINTF_ERROR();
      }
     }
  SHUTUP;
}

#define g_SHPARAM "\"$@\""
#define g_ACTPARAM "%s"
#define g_CSHPARAM "\\!*" /* DjF added another \ need \ too! */

static void f_addalias( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("addAlias");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:      /* DjF added ; before } */
      (void) sprintf(g_buff, "%s () { %s %s}\n#\n", a_cmd->argv[0], a_cmd->argv[1],
                     ((a_cmd->argv[1])[strlen (a_cmd->argv[1]) - 1] != '&' ? ";" : ""));
      if (strstr(g_buff, g_ACTPARAM)) {
	/* the string is already in there, add the parameter string */
	if (fprintf((FILE *)a_stream, g_buff, g_SHPARAM) < 0) {
	  FPRINTF_ERROR();
	}
      } else {
	/* the string is not there */
	if (fprintf((FILE *)a_stream, "%s", g_buff) < 0) {
	  FPRINTF_ERROR();
	}
      }
      break;
    case e_CSHELL:         /* DjF changed " to ' */
      (void) sprintf(g_buff, "alias %s \'%s\'\n#\n", a_cmd->argv[0], a_cmd->argv[1]);
      if (strstr(g_buff, g_ACTPARAM)) {
	/* the string is already in there, add the parameter string */
	if (fprintf((FILE *)a_stream, g_buff, g_CSHPARAM) < 0) {
	  FPRINTF_ERROR();
	}
      } else {
	/* the string is not there */
	if (fprintf((FILE *)a_stream, "%s", g_buff) < 0) {
	  FPRINTF_ERROR();
	}
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_unalias( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("unAlias");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (fprintf((FILE *)a_stream, "unset %s\n\n", a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "unalias %s\n#\n", a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static int sh_output_first_check(const FILE * const a_stream,
				 const char * const a_data)
{
  int status;

  if ((status = fprintf((FILE *)a_stream, "if [ -s %s ]; then\n", a_data))
      < 0) {
    FPRINTF_ERROR();
  } 
  return (status);
}

static int sh_output_next_part(const FILE * const a_stream,
			       const char * const a_data,
			       const int a_exit_flag, 
			       const int a_check_flag,
			       const int a_ups_env_flag,
			       const t_upstyp_matched_instance * const a_minst,
			       const t_upstyp_db * const a_db_info,
			       const t_upsugo_command * const a_command_line)
{
  int status = -1;

  /* define all of the UPS local variables that the user may need. */
  if (a_ups_env_flag == DO_UPS_ENV) {
    upsget_allout(a_stream, a_db_info, a_minst, a_command_line, "  ");
    g_LOCAL_VARS_DEF = 1;   /* we defined local variables */
  }
  if (UPS_ERROR == UPS_SUCCESS) {
    if ((status = fprintf((FILE *)a_stream, "  . %s\n", a_data)) < 0) {
      FPRINTF_ERROR();
    } else {
      if (a_check_flag == DO_CHECK) {
	if ((status = fprintf((FILE *)a_stream,
	      "  UPS_STATUS=$?\n  if [ \"$UPS_STATUS\" != \"0\" ]; then\n    echo \"Error $UPS_STATUS while sourcing %s\"\n    unset UPS_STATUS\n",
		    a_data)) < 0) {
	  FPRINTF_ERROR();
	} else {
	  upsutl_finish_temp_file(a_stream, a_command_line, "    ");
	  if ((status = fprintf((FILE *)a_stream,
		      "    return 1\n  fi\n  unset UPS_STATUS\n") < 0)) { /* Needs fixing */
	      FPRINTF_ERROR();
	  } 
	}
      }
/* We don't really want to return here since there'll be more to the file */
/*      if ((status >= 0) && (a_exit_flag == DO_EXIT)) { */
/*        upsutl_finish_temp_file(a_stream, a_command_line, "  "); */
/*        if ((status = fprintf((FILE *)a_stream, "  return\n") < 0)) { */
/*          FPRINTF_ERROR(); */
/*        } */
/*      } */
    }
  }
  if ((bit_bucket == 0)) bit_bucket ^= (long) a_exit_flag;
  return(status);
}

static int csh_output_first_check(const FILE * const a_stream,
				 const char * const a_data)
{
  int status;

  if ((status = fprintf((FILE *)a_stream, "if (-e %s) then\n", a_data))
       < 0) {
    FPRINTF_ERROR();
  }
  return (status);
}

static int csh_output_next_part(const FILE * const a_stream,
				const char * const a_data,
				const int a_exit_flag, 
				const int a_check_flag,
				const int a_ups_env_flag,
				const t_upstyp_matched_instance * const a_minst,
				const t_upstyp_db * const a_db_info,
				const t_upsugo_command * const a_command_line)
{
  int status = -1;

  /* define all of the UPS local variables that the user may need. */
  if (a_ups_env_flag == DO_UPS_ENV) {
    upsget_allout(a_stream, a_db_info, a_minst, a_command_line, "  ");
    g_LOCAL_VARS_DEF = 1;   /* we defined local variables */
  }
  if (UPS_ERROR == UPS_SUCCESS) {
    if ((status = fprintf((FILE *)a_stream, "  source %s\n", a_data)) < 0) {
      FPRINTF_ERROR();
    } else {
      if (a_check_flag == DO_CHECK) {
	if ((status = fprintf((FILE *)a_stream,
	      "  setenv UPS_STATUS \"$status\"\n  if (\"$UPS_STATUS\" != \"0\") then\n    echo \"Error $UPS_STATUS while sourcing %s\n    unsetenv UPS_STATUS\n",
		    a_data)) < 0) {
	  FPRINTF_ERROR();
	} else {
	  upsutl_finish_temp_file(a_stream, a_command_line, "    ");
	  if ((status = fprintf((FILE *)a_stream,
		      "    exit 1\n  endif\n  unsetenv UPS_STATUS\n") < 0)) {
	      FPRINTF_ERROR();
	  } 
	}
      }
/* We don't really want to return here since there'll be more to the file */
/*       if ((status >= 0) && (a_exit_flag == DO_EXIT)) { */
/*         upsutl_finish_temp_file(a_stream, a_command_line, "  "); */
/*         if ((status = fprintf((FILE *)a_stream, "  exit\n") < 0)) { */
/*           FPRINTF_ERROR(); */
/*         } */
/*       } */
    }
  }
  if ((bit_bucket == 0) && 0) bit_bucket ^= (long) a_exit_flag;
  return(status);
}

static int sh_output_last_part_req(const FILE * const a_stream,
				const char * const a_data,
				const t_upsugo_command * const a_command_line)
{
  int status;

  if ((status = fprintf((FILE *)a_stream,
	      "else\n  echo \"File (%s) not found\"\n", a_data) < 0)) {
    FPRINTF_ERROR();
  } else {
    upsutl_finish_temp_file(a_stream, a_command_line, "  ");
    if ((status = fprintf((FILE *)a_stream, "  return 1\nfi\n#\n") < 0)) { /* Needs fixing */
      FPRINTF_ERROR();
    }
  }
  return(status);
}

static int csh_output_last_part_req(const FILE * const a_stream,
				const char * const a_data,
				const t_upsugo_command * const a_command_line)
{
  int status;

  if ((status = fprintf((FILE *)a_stream,
	      "else\n  echo \"File (%s) not found\"\n", a_data) < 0)) {
    FPRINTF_ERROR();
  } else {
    upsutl_finish_temp_file(a_stream, a_command_line, "  ");
    if ((status = fprintf((FILE *)a_stream, "  exit 1\nendif\n#\n") < 0)) {
      FPRINTF_ERROR();
    }
  }
  return(status);
}

static void f_sourcecompilereq( ACTION_PARAMS)
{
  /* skip this whole action if we are being called while compiling */
  if (! g_COMPILE_FLAG) {
    CHECK_NUM_PARAM("sourceCompileReq");

    OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

    /* only proceed if we have a valid number of parameters and a stream to
       write them to */
    if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
      switch ( a_command_line->ugo_shell ) {
      case e_BOURNE:
	if (sh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (sh_output_next_part(a_stream, a_cmd->argv[0], DO_EXIT,
				  NO_CHECK, DO_NO_UPS_ENV, a_minst, a_db_info,
				  a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    (void )sh_output_last_part_req(a_stream, a_cmd->argv[0],
					   a_command_line);
	  }
	}
	break;
      case e_CSHELL:
	if (csh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (csh_output_next_part(a_stream, a_cmd->argv[0], DO_EXIT,
				   NO_CHECK, DO_NO_UPS_ENV, a_minst,
				   a_db_info, a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    (void )csh_output_last_part_req(a_stream, a_cmd->argv[0],
					    a_command_line);
	  }
	}
	break;
      default:
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
      }

      if (UPS_ERROR != UPS_SUCCESS) {
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		   g_func_info[a_cmd->icmd].cmd);
      }
    }
  }
  SHUTUP;
}

static void f_sourcecompileopt( ACTION_PARAMS)
{
  /* skip this whole action if we are being called while compiling */
  if (! g_COMPILE_FLAG) {
    CHECK_NUM_PARAM("sourceCompileOpt");

    OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

    /* only proceed if we have a valid number of parameters and a stream to
       write them to */
    if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
      switch ( a_command_line->ugo_shell ) {
      case e_BOURNE:
	if (sh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (sh_output_next_part(a_stream, a_cmd->argv[0], DO_EXIT,
				  NO_CHECK, DO_NO_UPS_ENV, a_minst, a_db_info,
				  a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    SH_OUTPUT_LAST_PART_OPT();
	  }
	}
	break;
      case e_CSHELL:
	if (csh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (csh_output_next_part(a_stream, a_cmd->argv[0], DO_EXIT,
				   NO_CHECK, DO_NO_UPS_ENV, a_minst, a_db_info,
				   a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    CSH_OUTPUT_LAST_PART_OPT();
	  }
	}
	break;
      default:
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
      }
      if (UPS_ERROR != UPS_SUCCESS) {
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		   g_func_info[a_cmd->icmd].cmd);
      }
    }
  }
  SHUTUP;
}

static void f_sourcerequired( ACTION_PARAMS)
{
  int exit_flag = NO_EXIT;
  int no_ups_env_flag = DO_NO_UPS_ENV;

  CHECK_NUM_PARAM("sourceRequired");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
    /* Determine which flags (if any) were entered */
    GET_FLAGS();
    GET_UPS_ENV();

    if (UPS_ERROR == UPS_SUCCESS) {
      switch ( a_command_line->ugo_shell ) {
      case e_BOURNE:
	if (sh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (sh_output_next_part(a_stream, a_cmd->argv[0], exit_flag,
				 NO_CHECK, no_ups_env_flag, a_minst, a_db_info,
				 a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    (void )sh_output_last_part_req(a_stream, a_cmd->argv[0],
					   a_command_line);
	  }
	}
	break;
      case e_CSHELL:
	if (csh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (csh_output_next_part(a_stream, a_cmd->argv[0], exit_flag,
				   NO_CHECK, no_ups_env_flag, a_minst,
				   a_db_info, a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    (void )csh_output_last_part_req(a_stream, a_cmd->argv[0],
					    a_command_line);
	  }
	}
	break;
      default:
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
      }
    } else {
      FPRINTF_ERROR();
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_sourceoptional( ACTION_PARAMS)
{
  int exit_flag = NO_EXIT;
  int no_ups_env_flag = DO_NO_UPS_ENV;

  CHECK_NUM_PARAM("sourceOptional");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    /* Determine which flags (if any) were entered */
    GET_FLAGS();
    GET_UPS_ENV();

    if (UPS_ERROR == UPS_SUCCESS) {
      switch ( a_command_line->ugo_shell ) {
      case e_BOURNE:
	if (sh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (sh_output_next_part(a_stream, a_cmd->argv[0], exit_flag,
				  NO_CHECK, no_ups_env_flag, a_minst,
				  a_db_info, a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    SH_OUTPUT_LAST_PART_OPT();
	  }
	}
	break;
      case e_CSHELL:
	if (csh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (csh_output_next_part(a_stream, a_cmd->argv[0], exit_flag,
				   NO_CHECK, no_ups_env_flag, a_minst,
				   a_db_info, a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    CSH_OUTPUT_LAST_PART_OPT();
	  }
	}
	break;
      default:
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
      }
      if (UPS_ERROR != UPS_SUCCESS) {
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		   g_func_info[a_cmd->icmd].cmd);
      }
    }
  }
  SHUTUP;
}

static void f_sourcereqcheck( ACTION_PARAMS)
{
  int exit_flag = NO_EXIT;
  int no_ups_env_flag = DO_NO_UPS_ENV;

  CHECK_NUM_PARAM("sourceReqCheck");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
    /* Determine which flags (if any) were entered */
    GET_FLAGS();
    GET_UPS_ENV();

    if (UPS_ERROR == UPS_SUCCESS) {
      switch ( a_command_line->ugo_shell ) {
      case e_BOURNE:
	if (sh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (sh_output_next_part(a_stream, a_cmd->argv[0], exit_flag,
				  DO_CHECK, no_ups_env_flag, a_minst, a_db_info,
				  a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    (void )sh_output_last_part_req(a_stream, a_cmd->argv[0],
					   a_command_line);
	  }
	}
	break;
      case e_CSHELL:
	if (csh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (csh_output_next_part(a_stream, a_cmd->argv[0], exit_flag,
				  DO_CHECK, no_ups_env_flag, a_minst, a_db_info,
				  a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    (void )csh_output_last_part_req(a_stream, a_cmd->argv[0],
					   a_command_line);
	  }
	}
	break;
      default:
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
      }
    } else {
      FPRINTF_ERROR();
    }

    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_sourceoptcheck( ACTION_PARAMS)
{
  int exit_flag = NO_EXIT;
  int no_ups_env_flag = DO_NO_UPS_ENV;

  CHECK_NUM_PARAM("sourceOptCheck");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
    /* Determine which flags (if any) were entered */
    GET_FLAGS();
    GET_UPS_ENV();

    if (UPS_ERROR == UPS_SUCCESS) {
      switch ( a_command_line->ugo_shell ) {
      case e_BOURNE:
	if (sh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (sh_output_next_part(a_stream, a_cmd->argv[0], exit_flag,
				  DO_CHECK, no_ups_env_flag, a_minst,
				  a_db_info, a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    SH_OUTPUT_LAST_PART_OPT();	
	  }
	}
	break;
      case e_CSHELL:
	if (csh_output_first_check(a_stream, a_cmd->argv[0]) >= 0) {
	  if (csh_output_next_part(a_stream, a_cmd->argv[0], exit_flag,
				   DO_CHECK, no_ups_env_flag, a_minst,
				   a_db_info, a_command_line) < 0) {
	    FPRINTF_ERROR();
	  } else {
	    CSH_OUTPUT_LAST_PART_OPT();
	  }
	}
	break;
      default:
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
      }
      if (UPS_ERROR != UPS_SUCCESS) {
        OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		   g_func_info[a_cmd->icmd].cmd);
      }
    }
  }
  SHUTUP;
}

static void f_writecompilescript(ACTION_PARAMS)
{
  t_upstyp_matched_product *mproduct = NULL;
  t_upslst_item *cmd_list = NULL;
  char *time_ptr = 0;
  int moved_to_old = 0, moved_to_timedate = 0, i, save_shell, save_g_shell;
  FILE *compile_file;
  static char buf[MAX_LINE_LEN];

  /* skip this whole action if we are being called while compiling */
  if (! g_COMPILE_FLAG) {

    /* let everybody know we are in compile mode */
    g_COMPILE_FLAG = 1;

    /* save the initial shell that we came in with.  we will need to generate
       a file of each supported shell type.  to do this we will need to reset
       the cached value that upsugo uses.  but we want to set it back to the
       original value before we exit. */
    save_g_shell = g_UPS_SHELL;
    save_shell = a_command_line->ugo_shell;

    /* we must create 2 files one csh variety and one sh variety. */
    for (i = e_MIN_SHELL ; i < e_MAX_SHELL ; ++i) {
      a_command_line->ugo_shell = i;
      g_UPS_SHELL = i;

      CHECK_NUM_PARAM("writeCompileScript");

      OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

      /* only proceed if we have a valid number of parameters */
      if (UPS_ERROR == UPS_SUCCESS) {
	/* this action does the following -
	   1. locate the action=command section which matches the
	      entered parameter
	   2. if there is none, we are done, else, locate current compile
	      file.
	   3. if there is none, skip to next step. else, rename current one
	      if desired. (on any error before completing the compile, this
	      file will be renamed back to the original name if possible.
	      of course this is not possible if we are overwriting the current
	      file.)
	   4. open compile file
	   5. process actions and write them to the compile file.
	   6. close compile file */
	/* 1     first, setup the matched product (if this is the first time
	         through */
	if (! mproduct) {
	  mproduct = (t_upstyp_matched_product *)upsmem_malloc(
					    sizeof(t_upstyp_matched_product));
	  mproduct->db_info = (t_upstyp_db *)a_db_info;
	  upsmem_inc_refctr(a_db_info);
	  mproduct->minst_list = upslst_new((void *)a_minst);
	}
    
	/*       get the action command list */
	cmd_list = upsact_get_cmd((t_upsugo_command *)a_command_line,
				  mproduct, a_cmd->argv[1], a_cmd->icmd );
	if (UPS_ERROR == UPS_SUCCESS) {
	  /* 2      now that we have the list, locate the current compile file
	            if there is one, first we must add in the appropriate file
		    extension */
	  (void) sprintf(buf, "%s.%s", a_cmd->argv[0], g_file_ext[i]);
	  if (upsutl_is_a_file(buf) == UPS_SUCCESS) {
	    /* 3   the file exists. check argv[2] to see if we need to rename
	           the file before writing the new one. if no flag was passed 
                   then just overwrite the file */
	    if (a_cmd->argc == g_func_info[a_cmd->icmd].max_params) {
	      /* the flag was there, now see what it is */
	      if (! upsutl_stricmp(a_cmd->argv[a_cmd->argc - 1], OLD_FLAG)) {
		/* append ".OLD" to the file name */
		(void) sprintf(g_buff, "mv %s %s.%s\n", buf, buf, OLD_FLAG);
		DO_SYSTEM_MOVE(moved_to_old);
	      } else if (! upsutl_stricmp(a_cmd->argv[a_cmd->argc - 1],
					  DATE_FLAG)) {
		/* append a timedate stamp to the file name */
		time_ptr = upsutl_time_date(STR_TRIM_PACK);
		(void) sprintf(g_buff, "mv %s %s.%s\n", buf, buf, time_ptr);
		DO_SYSTEM_MOVE(moved_to_timedate);
	      }
	    }
	  } else {
	    /* there currently is no compile file with this name.  reset the
	       error status so we only need to test for success later */
	    UPS_ERROR = UPS_SUCCESS;
	  }
	  /* if there was no error in renaming the file, then proceed */
	  if (UPS_ERROR == UPS_SUCCESS) {
	    /* 4    open file that we will write compiled commands out to */
	    if ((compile_file = fopen(buf, "w")) == NULL) {
	      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	      upserr_vplace();
	      upserr_add(UPS_OPEN_FILE, UPS_FATAL, buf);
	      upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "fopen",
			 strerror(errno));
	    } else {
	      /* 5   process actions and write them to the compile file. mark
		     that action functions are being called due to a compile
		     command */
	      upsact_process_commands(cmd_list, compile_file);

	      /* 6   close the compile file */
	      upsutl_unset_upsvars(compile_file, a_command_line, "");
	      if (fclose(compile_file) == EOF) {
		OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
		upserr_vplace();
		upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "fclose",
			   strerror(errno));
	      }
	    }
	  }
	} else {
	  /* could not get the list of commands, there must not be an
	     action=argv[1] line in the file */
	  OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	  upserr_vplace();
	  upserr_add(UPS_NO_ACTION, UPS_WARNING, a_cmd->argv[1]);
	}
      }
      /* if we moved old compile file to a backup and then got an error while
	 creating the new file, we should move the old file back so we don't
	 break current things */
      if ((moved_to_old || moved_to_timedate) && (UPS_ERROR != UPS_SUCCESS)) {
	/* yes we did the move, and got an error, move the file back */
	if (moved_to_old) {
	  (void) sprintf(g_buff, "mv %s.%s %s\n", buf, OLD_FLAG, buf);
	} else {
	  (void) sprintf(g_buff, "mv %s.%s %s\n", buf, time_ptr, buf);
	}
	/* since we are at end, it does not matter which flag we use here */
	DO_SYSTEM_MOVE(moved_to_old);
      }
      if (cmd_list) {
	upsact_cleanup(cmd_list);
	cmd_list = 0;
      }
    }
    /* reset the shell back to its original value. */
    a_command_line->ugo_shell = save_shell;
    g_UPS_SHELL = save_g_shell;

    /* let everybody know we are out of compile mode */
    g_COMPILE_FLAG = 0;

  }
  /* release any memory we acquired */
  if (mproduct && mproduct->minst_list) {
    (void) upslst_free(mproduct->minst_list, ' ');
    upsmem_free(mproduct);
    upsmem_dec_refctr(a_db_info);
  }
  SHUTUP;
}

static void f_setupenv( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("setupEnv");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
    SET_SETUP_PROD();
  }
  SHUTUP;
}

static void f_unsetupenv( ACTION_PARAMS)
{
  t_upsact_cmd lcl_cmd;
  char *uprod_name;

  CHECK_NUM_PARAM("unSetupEnv");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
    /* we will be calling the envunset action */
    lcl_cmd.iact = a_cmd->iact;
    lcl_cmd.argc = g_func_info[e_envunset].min_params;   /* # of args */
    lcl_cmd.icmd = e_envunset;
    lcl_cmd.argv[0] = g_buff;

    if (a_minst->version && a_minst->version->product) {
      uprod_name = upsutl_upcase(a_minst->version->product);
      if (UPS_ERROR == UPS_SUCCESS) {
	(void) sprintf(g_buff, "%s%s", SETUPENV,uprod_name);
	f_envunset(a_minst, a_db_info, a_command_line, a_stream, &lcl_cmd);
      }
    }
  }
  SHUTUP;
}

static void f_proddir( ACTION_PARAMS)
{
  static char buf[MAX_LINE_LEN];
  static char buf2[MAX_LINE_LEN];
  t_upsact_cmd lcl_cmd;
  char *tmp_prod_dir = NULL, *tmp_prod_name = NULL;
  char *uprod_name;
  char *suffix="_DIR";


  CHECK_NUM_PARAM("prodDir");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
    /* we will be calling the envset action */
    lcl_cmd.iact = a_cmd->iact;
    lcl_cmd.argc = g_func_info[e_envset].min_params;   /* # of args */
    lcl_cmd.icmd = e_envset;
    lcl_cmd.argv[0] = g_buff;

    /* Need to figure out "prod_name" (in tmp_prod_name) and "prod_dir" in
       tmp_prod_dir.
       Since the prod_dir may come from the command line we need to check
       if the user entered one that we have to use.
       OR it may be an optional arg.
    */
    if (a_command_line->ugo_productdir) {
      strcpy( buf, a_command_line->ugo_productdir ); /* may need to trim
							below */
      tmp_prod_dir = buf;
      if (! a_minst->version) {
	/* we do not have a version file read in.  therefore use the product
	   name from the command line */
	tmp_prod_name = a_command_line->ugo_product;
      } else {
	tmp_prod_name = a_minst->version->product;
      }
    } else if (a_minst->version && a_minst->version->prod_dir) {
      if (UPSRELATIVE(a_minst->version->prod_dir) && a_db_info &&
	  a_db_info->config && a_db_info->config->prod_dir_prefix) {
	(void) sprintf(buf, "%s/%s", a_db_info->config->prod_dir_prefix,
		a_minst->version->prod_dir);
	tmp_prod_dir = buf;
      } else {
	strcpy( buf, a_minst->version->prod_dir ); /* may need to trim
							below */
	tmp_prod_dir = buf;
      }
      if (! a_minst->version->product) {
	/* we do not have a product name in the version file.  therefore use
	   the product name from the command line */
	tmp_prod_name = a_command_line->ugo_product;
      } else {
	tmp_prod_name = a_minst->version->product;
      }
    }
    /* done getting "prod_name" and "prod_name" */

    /* make adjustments */
    if (a_cmd->argc >= 1) suffix = a_cmd->argv[0];
    if (a_cmd->argc == 2 && tmp_prod_dir) {
      int skip_first=0;
      if (tmp_prod_dir[strlen(tmp_prod_dir)-1] == '/') {
	tmp_prod_dir[strlen(tmp_prod_dir)-1] = '\0';
      }
      if (a_cmd->argv[1][0] == '/') skip_first=1;
      (void)sprintf( buf2, "%s/%s", tmp_prod_dir, &(a_cmd->argv[1][skip_first]) );
      tmp_prod_dir = buf2;
    }

    if (tmp_prod_dir && tmp_prod_name) {
      uprod_name = upsutl_upcase(tmp_prod_name);
      if (UPS_ERROR == UPS_SUCCESS) {
	(void) sprintf(g_buff, "%s%s", uprod_name, suffix);
	
	lcl_cmd.argv[1] = tmp_prod_dir;
	f_envset(a_minst, a_db_info, a_command_line, a_stream, &lcl_cmd);
      }
    }
    if (! tmp_prod_dir) {
      /* we could not find a value to set PROD_DIR to.  this smells like an
	 error so let the user know about it */
      if (a_minst->version && a_minst->version->product) {
	OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
	upserr_vplace();
	upserr_add(UPS_NO_PROD_DIR, UPS_INFORMATIONAL,
		   a_minst->version->product);
      }
    } 
  }
  SHUTUP;
}

static void f_unproddir( ACTION_PARAMS)
{
  t_upsact_cmd lcl_cmd;
  char *uprod_name;
  char *suffix="_DIR";

  CHECK_NUM_PARAM("unProdDir");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
    /* we will be calling the envunset action */
    lcl_cmd.iact = a_cmd->iact;
    lcl_cmd.argc = g_func_info[e_envunset].min_params;   /* # of args */
    lcl_cmd.icmd = e_envunset;
    lcl_cmd.argv[0] = g_buff;

    if (a_minst->version && a_minst->version->product) {
      uprod_name = upsutl_upcase(a_minst->version->product);
      if (UPS_ERROR == UPS_SUCCESS) {
	if (a_cmd->argc == 1) suffix = a_cmd->argv[0];
	(void) sprintf(g_buff, "%s%s", uprod_name, suffix);
	f_envunset(a_minst, a_db_info, a_command_line, a_stream, &lcl_cmd);
      }
    }
  }
  SHUTUP;
}

static char *ifstack[256] = {""};
static int   ifcount = 0;

static void f_if( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("If");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  ifstack[ifcount++] = a_cmd->argv[0];

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (fprintf((FILE *)a_stream, "%s; if [ $? = 0 ]\nthen\n:\n", a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "%s\nif ($status == 0) then\n:\n", a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_unless( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("Unless");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  ifstack[ifcount++] = a_cmd->argv[0];

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (fprintf((FILE *)a_stream, "%s; if [ $? != 0 ] \nthen\n:\n", a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "%s\nif ($status != 0) then\n:\n", a_cmd->argv[0]) < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_endif( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("EndIf");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  ifcount--;

  if ( ifcount < 0 || 0 != strcmp(ifstack[ifcount], a_cmd->argv[0]) ) {

      if (ifcount < 0) 
	ifcount = 0;

      upserr_vplace();
      upserr_add(UPS_MISMATCHED_ENDIF, UPS_FATAL, ifstack[ifcount], a_cmd->argv[0]);
      return;
  }

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    switch ( a_command_line->ugo_shell ) {
    case e_BOURNE:
      if (fprintf((FILE *)a_stream, "fi\n") < 0) {
	FPRINTF_ERROR();
      }
      break;
    case e_CSHELL:
      if (fprintf((FILE *)a_stream, "endif\n") < 0) {
	FPRINTF_ERROR();
      }
      break;
    default:
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_else( ACTION_PARAMS)
{
  CHECK_NUM_PARAM("EndIf");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) {
  
    if (fprintf((FILE *)a_stream, "else\n:\n") < 0) {
      FPRINTF_ERROR();
    }
    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void f_dodefaults( ACTION_PARAMS)
{
  t_upsact_cmd lcl_cmd;
  int the_cmd;

  CHECK_NUM_PARAM("doDefaults");

  OUTPUT_VERBOSE_MESSAGE(g_func_info[a_cmd->icmd].cmd);

  /* only proceed if we have a stream to write the output to */
  if (a_stream) {
    if (a_cmd->argc > g_func_info[a_cmd->icmd].min_params) {
      the_cmd = upsact_action2enum(
			 a_cmd->argv[g_func_info[a_cmd->icmd].max_params - 1]);
    } else {
      the_cmd = a_cmd->iact;
    }

    switch ( the_cmd ) {
    case e_setup:	/* Define <PROD>_DIR and SETUP_<PROD> */
      /* use our local copy since we have to change it - we will be calling
	 the proddir action */
      lcl_cmd.iact = the_cmd;
      lcl_cmd.argc = g_func_info[e_proddir].min_params;   /* # of args */
      lcl_cmd.icmd = e_proddir;
      f_proddir(a_minst, a_db_info, a_command_line, a_stream, &lcl_cmd);

      SET_SETUP_PROD();
      break;
    case e_chain:	/* None */
      break;
    case e_configure:	/* None */
      break;
    case e_copy:    	/* None */
      break;
    case e_create:	/* None */
      break;
    case e_current:     /* None */
      break;
    case e_declare:	/* None */
      break;
    case e_depend:	/* None */
      break;
    case e_development:	/* None */
      break;
    case e_exist:	/* None */
      break;
    case e_get: 	/* None */
      break;
    case e_list:	/* None */
      break;
    case e_modify:	/* None */
      break;
    case e_new :	/* None */
      break;
    case e_old:   	/* None */
      break;
    case e_start:	/* None */
      break;
    case e_stop:	/* None */
      break;
    case e_tailor:	/* None */
      break;
    case e_test:	/* None */
      break;
    case e_unchain:	/* None */
      break;
    case e_unconfigure:	/* None */
      break;
    case e_uncurrent:   /* None */
      break;
    case e_undeclare:	/* None */
      break;
    case e_undevelopment:	/* None */
      break;
    case e_unk:	/* None */
      break;
    case e_unnew:	/* None */
      break;
    case e_unold:	/* None */
      break;
    case e_unsetup:
      /* use our local copy since we have to change it */
      lcl_cmd.iact = the_cmd;
      lcl_cmd.argc = g_func_info[e_unproddir].min_params;   /* # of args */
      lcl_cmd.icmd = e_unproddir;
      f_unproddir(a_minst, a_db_info, a_command_line, a_stream, &lcl_cmd);
      lcl_cmd.argc = g_func_info[e_unsetupenv].min_params;   /* # of args */
      lcl_cmd.icmd = e_unsetupenv;
      f_unsetupenv(a_minst, a_db_info, a_command_line, a_stream, &lcl_cmd);
      break;
    case e_untest:	/* None */
      break;
    case e_verify:	/* None */
      break;
    default:
      break;
    }

    if (UPS_ERROR != UPS_SUCCESS) {
      OUTPUT_ACTION_INFO(UPS_FATAL, a_minst);
      upserr_vplace();
      upserr_add(UPS_ACTION_WRITE_ERROR, UPS_FATAL,
		 g_func_info[a_cmd->icmd].cmd);
    }
  }
  SHUTUP;
}

static void shutup( ACTION_PARAMS)
{
  bit_bucket ^= (long) a_minst;
  bit_bucket ^= (long) a_db_info;
  bit_bucket ^= (long) a_command_line;
  bit_bucket ^= (long) a_stream;
  bit_bucket ^= (long) a_cmd;
}

