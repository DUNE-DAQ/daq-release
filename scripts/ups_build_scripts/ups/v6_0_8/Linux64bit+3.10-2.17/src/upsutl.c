/************************************************************************
 *
 * FILE:
 *       upsutl.c
 * 
 * DESCRIPTION: 
 *       UPs utility routines
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
 *       28-Jul-1997, EB, first.
 *       13-Aug-1997, LR, added string handling upsutl_str_*.
 *       26-Aug-1997, LR, fixed bug in str_sort.
 *                        added option 'p' to str_create.
 *                        added function str_crecat.
 *       11-Sep-1997, LR, added function str_stricmp, case insensitive
 *                        string comparison. It's a copy of strcasecmp.
 *       24-Sep-1997, LR, added function str_strincmp, like str_stricmp,
 *                        except it will compare most n characters.
 *       28-Oct-1997, EB, added function  upsutl_get_config
 *       29-Oct-1997, EB, added function  upsutl_is_authorized
 *
 ***********************************************************************/

/* standard include files */
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>        /* needed on SunOS to get MAXHOSTNAMELEN */
/* #ifdef _SYSTYPE_SVR4 */
#include <dirent.h>
/* #else
#include <sys/dir.h>
#endif */
#ifdef _SGIAPI
extern int gethostname(char *, int);
#endif

/* ups specific include files */
#include "upsutl.h"
#include "upserr.h"
#include "upstyp.h"
#include "upsmem.h"
#include "upsfil.h"
#include "upsget.h"
#include "upsact.h"
#include "upsver.h"

/*
 * Definition of public variables.
 */
extern int g_LOCAL_VARS_DEF;
extern char *g_temp_file_name;
extern int g_keep_temp_file;
extern int g_COMPILE_FLAG;
extern t_cmd_info g_cmd_info[];
int g_DID_COPY_MAN = 0;
/*
 * Declaration of private functions.
 */
static int qsort_cmp_string( const void *, const void * ); /* used by qsort */

/*
 * Definition of global variables.
 */
#ifndef NULL
#define NULL 0
#endif
#define VPREFIX "UPSUTL: "

#define KEEP_OR_REMOVE_FILE()   \
   if (! g_keep_temp_file) {                  \
     (void) remove(g_temp_file_name);         \
   } else {                                 \
     (void) printf("%s\n", g_temp_file_name); \
   }

static clock_t g_start_cpu, g_finish_cpu;
static time_t g_start_w, g_finish_w;
static char g_stat_dir[] = "statistics/";
static char g_unknown_user[] = "UNKNOWN";
static mode_t g_umask = 0;

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * upsutl_finish_up
 *
 * Write any remaining thing to the temp file, close it, and execute it.
 * Flush the journal files too.
 *
 * Input : a file descriptor
 * Output: none
 * Return: none
 */
int upsutl_finish_up(const FILE * const a_stream, const int a_shell,
		      const int a_command_index, const int a_simulate_flag)
{
  int empty_stream = 0;
  t_upsugo_command command_line;
  mode_t mode = 0755;   /* rwx by owner, rx by group and all */

  /* we will need the shell information when we call upsutl_remall from within
     upsutl_finish_temp_file */
  command_line.ugo_shell = a_shell;
  
  /* close the temp file */
  if (a_stream) {
    /* look and see where we are */
    if (ftell((FILE *)a_stream) == 0L) {
      /* we are at the beginning of the file, nothing was written to it */
      empty_stream = 1;
      upsver_mes(1,"%sEmpty temp file deleted %s\n",VPREFIX,g_temp_file_name);
    } else {
      /* write any closing info to the file */
      (void) upsutl_finish_temp_file(a_stream, &command_line, "");
    }
    if (fclose((FILE *)a_stream) == EOF) {
      upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "fclose", strerror(errno));
    }
    /* if nothing was written to the file, delete it, */
    if (empty_stream) {
      (void) remove(g_temp_file_name);
      switch (g_cmd_info[a_command_index].cmd_index) {
      case e_setup: 
      case e_unsetup: 
	/* output the name of the a null file so the automatic sourcing does
	   not give an error */
        if (UPS_ERROR) {
            (void) printf("%s/bin/setup_fail\n", getenv("UPS_DIR"));
        } else {
            (void) printf("%s/bin/setup_win\n", getenv("UPS_DIR"));
        }
	break;
      }
      /* flush the journaling cache of files so the changes made internally are
	 actually written out to disk. only do this when command succeeds  */
      if (UPS_ERROR == UPS_SUCCESS && !a_simulate_flag) {
	upsfil_flush();
      }
    } else {
      if (UPS_ERROR == UPS_SUCCESS ) {
	switch (g_cmd_info[a_command_index].cmd_index) {
	case e_setup: 
	case e_unsetup: 
	  /* see if this is only a simulation first */
	  if (a_simulate_flag) {
	    /* yes, output this to short circuit the automatic sourcing */
	    (void) printf("%s/bin/setup_win\n", getenv("UPS_DIR"));
	    upserr_add(UPS_TEMP_FILE, UPS_INFORMATIONAL, g_temp_file_name);
	  } else {
	    /* output the name of the temp file that was created */
	    upsver_mes(1,"%sSourcing file %s\n",VPREFIX,g_temp_file_name);
	    (void) printf("%s\n", g_temp_file_name);
	  }
	  /* if we were asked to save the file, output the name again so the
	     user can see it. the first output was eaten by the sourcing */
	  if (g_keep_temp_file) {
	    upserr_add(UPS_TEMP_FILE, UPS_INFORMATIONAL, g_temp_file_name);
	  }
	  break;
	case e_exist:
	  /* just get rid of the file. (unless asked not to) we do not need it,
	     we just wanted to see if we could create it */
	  KEEP_OR_REMOVE_FILE();
	  break;
	default:
	  /* check to see if we are supposed to execute the file or just
	     report that it is there. */
	  if (a_simulate_flag) {
	    /* simulation only, print the file name */
	    upserr_add(UPS_TEMP_FILE, UPS_INFORMATIONAL, g_temp_file_name);
	  } else {	
	    /* if we were asked to save the file, output the name so the
	       user can see it. */
	    if (g_keep_temp_file) {
	      upserr_add(UPS_TEMP_FILE, UPS_INFORMATIONAL, g_temp_file_name);
	    }	   
	    /* make sure the file is executable */
	    if (! chmod(g_temp_file_name, mode)) {
              char sourced_temp_file[L_tmpnam+3] = ". ";
              (void) strcat(sourced_temp_file, g_temp_file_name);
	      upsver_mes(1,"%sExecuting file %s\n",VPREFIX,g_temp_file_name);
              errno = system (sourced_temp_file);
              errno = WEXITSTATUS (errno);
	      if (errno == 0) {
                /* flush the journaling cache of files so the changes made
                   internally are actually written out to disk */
                upsfil_flush();
              } else {
		upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "system",
			   "error from subprocess");
		if (g_cmd_info[a_command_index].cmd != NULL)
		  upserr_add(UPS_COMMAND_FAILED, UPS_FATAL,
			  g_cmd_info[a_command_index].cmd);
		KEEP_OR_REMOVE_FILE();
                return (errno);
	      }
	    } else {
	      upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "chmod",
			 strerror(errno));
	      KEEP_OR_REMOVE_FILE();
	    }
	  }
	}
      } else {  /* there was an error while doing the command */
	switch (g_cmd_info[a_command_index].cmd_index) {
	case e_setup:
	case e_unsetup:
	  /* we must remove the file because otherwise the setup/unsetup 
	     command will try and source it and only half change the user's
	     environment */
	  (void) remove(g_temp_file_name);
	  /* print the following so automatic sourcing does'nt give an error */
	  (void) printf("%s/bin/setup_fail\n", getenv("UPS_DIR"));
	  break;
	default:
	  /* keep the file if we were asked to */
	  KEEP_OR_REMOVE_FILE();
	}
      }
    }
  } else {
    /* flush the journaling cache of files so the changes made internally are
       actually written out to disk. only do this when command succeeds  */
    if (UPS_ERROR == UPS_SUCCESS && !a_simulate_flag) {
      upsfil_flush();
    }

    /* if this is the setup command we must short circuit the sourcing */
    switch (g_cmd_info[a_command_index].cmd_index) {
    case e_setup:
    case e_unsetup:
      (void) printf("%s/bin/setup_fail\n", getenv("UPS_DIR"));
      break;
    }
  }
  return ((UPS_ERROR == UPS_SUCCESS) ? 0 : 1);
}

/*-----------------------------------------------------------------------
 * upsutl_unset_upsvars
 *
 * If needed, add the unsetting if the ups variables to the file.
 *
 * Input : an open file stream and a command line structure
 * Output: none
 * Return: none
 */
void upsutl_unset_upsvars( const FILE * const a_stream,
			   const t_upsugo_command * const a_command_line,
			   const char * const a_prefix)
{
  if (g_LOCAL_VARS_DEF) {
    /* undefine the local env variables */
    (void) upsget_remall(a_stream, a_command_line, a_prefix);
  }
}
/*-----------------------------------------------------------------------
 * upsutl_finish_temp_file
 *
 * Write any closing information to the temp file.
 *
 * Input : an open file stream and a command line structure
 * Output: none
 * Return: none
 */
void upsutl_finish_temp_file( const FILE * const a_stream,
			      const t_upsugo_command * const a_command_line,
			      const char * const a_prefix)
{
  upsutl_unset_upsvars(a_stream, a_command_line, a_prefix);

  if (g_DID_COPY_MAN) {
    /* make sure that the status returned after copying man/catman/info files
       is success.  it does not matter if they could not be copied */
    (void) fprintf((FILE *)a_stream, "echo > /dev/null\n");
  }

  /* we usually tell the file to delete itself.  however the user may
     override this */
  if (g_temp_file_name && ! g_COMPILE_FLAG && ! g_keep_temp_file &&
      (strcmp (g_temp_file_name, "/dev/null") != 0)) {

    /* Save the last exit status, so we don't think we succeeded
    ** 'cause we could remove the tmp file...
    */
    switch(a_command_line->ugo_shell) {
    case e_BOURNE:
      (void) fprintf((FILE *)a_stream, "\nrc=$?\n");
      break;
    case e_CSHELL:
      (void) fprintf((FILE *)a_stream, "\nset rc=$status\n");
      break;
    }
    (void) fprintf((FILE *)a_stream, "%s/bin/rm -f %s\n", a_prefix, g_temp_file_name);
    (void) fprintf((FILE *)a_stream, "test $rc = 0\n");
  }
}


/*-----------------------------------------------------------------------
 * upsutl_get_hostname
 *
 * Get the current hostname. Remove any domain information.
 *
 * Input : none
 * Output: none
 * Return: pointer to a buffer with the hostname in it
 */
char *upsutl_get_hostname( void )
{
  char *tmp_buf;
  static char nodename[MAXHOSTNAMELEN];

  /* get the hostname */
  if (! gethostname(nodename, MAXHOSTNAMELEN)) {
    /* If buffer is of the form node.fnal.gov, seek to first '.' and terminate
       the string. */
    tmp_buf = nodename;
    for (; *tmp_buf != '\0'; ++tmp_buf) { 
       if (*tmp_buf == '.')  { 
           *tmp_buf = '\0';
           break;
       }
    }
    tmp_buf = nodename;
  } else {
    upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "gethostname", strerror(errno));
    tmp_buf = NULL;
  }
  return(tmp_buf);
}
/*-----------------------------------------------------------------------
 * upsutl_is_authorized
 *
 * Determine if the passed product instance is authorized for use on the
 * current node.  The authorization information is stored in either the 
 * dbconfig file info or in the product instance info.  
 *
 * Input : Matched instance structure, database configuration info and a
 *         pointer to a char pointer 
 * Output: list of nodes on which the product is authorized
 * Return: 0 if not authorized, else 1.
 */
#define CMP_AUTH_NODES(struct) \
    if (struct && struct->authorized_nodes) {                   \
      *a_nodes = struct->authorized_nodes;                      \
      if (NOT_EQUAL_ANY_MATCH(struct->authorized_nodes) &&      \
	  !strstr(struct->authorized_nodes, nodename)) {        \
	is_auth = 0;                                            \
      }                                                         \
    }

int upsutl_is_authorized( const t_upstyp_matched_instance * const a_minst,
			  const t_upstyp_db * const a_db_info, 
			  char ** const a_nodes)
{
  int is_auth = 1;
  char *nodename = NULL;

  *a_nodes = NULL;

  /* get the hostname */
  if ((nodename = upsutl_get_hostname())) {
    /* first check in the product instances to see if there is any
       authorization information there.  check in each file to make sure
       that if there is a specific list of nodes, that the current node is
       on it. in other words, assume the instance is authorized until told
       differently. */
    CMP_AUTH_NODES(a_minst->table);
    if (is_auth) {
      CMP_AUTH_NODES(a_minst->version);
    }
    if (is_auth) {
      CMP_AUTH_NODES(a_minst->chain);
    }
    
    if (is_auth) {
      if (a_db_info) {
	CMP_AUTH_NODES(a_db_info->config);
      }
    }

    /* if no authorized nodes were specified, then we assume all */
    if (! *a_nodes) {
      *a_nodes = ANY_MATCH;
    }
  } else {
    /* error when getting the node name */
    is_auth = 0;
  }
  return(is_auth);
}
/*-----------------------------------------------------------------------
 * upsutl_get_config
 *
 * Read the configuration file associated with the passed database
 *
 * Input : Name of the ups database
 * Output: none
 * Return: A product structure with the config info in it.
 */
t_upstyp_product *upsutl_get_config( const char * const a_db)
{
  t_upstyp_product *read_product = NULL;
  static char buffer[MAX_LINE_LEN];
  
  if ((strlen(a_db) + (unsigned int )CONFIG_SIZE + (unsigned int ) 2) <=
      MAX_LINE_LEN) {
    (void) sprintf(&buffer[0], "%s/%s/%s", a_db, UPS_FILES, CONFIG_FILE);
    read_product = upsfil_read_file(buffer);
    if (UPS_ERROR == UPS_NO_FILE) {
      upserr_backup();
      upserr_backup();
    } else {
      if (UPS_VERIFY && upsutl_stricmp(CONFIG_FILE, read_product->file)) {
	upserr_add(UPS_NO_KEYWORD, UPS_INFORMATIONAL, buffer, "FILE",
		   CONFIG_FILE);
      }
    }
  } else {
    upserr_add(UPS_NAME_TOO_LONG, UPS_WARNING, MAX_LINE_LEN);
  }
  
  return(read_product);
}
/*-----------------------------------------------------------------------
 * upsutl_free_config
 *
 * Free the configuration file memory
 *
 * Input : Configuration pointer
 * Output: none
 * Return: NULL
 */
t_upstyp_config *upsutl_free_config( const t_upstyp_config * const a_db_config)
{

  upsmem_free((void *)a_db_config);
  return((t_upstyp_config *)NULL);
}

/*-----------------------------------------------------------------------
 * upsutl_free_matched_product_list
 *
 * Free a matched product list.
 *
 * Input : pointer to matched product list
 * Output: none
 * Return: NULL
 */
t_upslst_item *upsutl_free_matched_product_list(
			            t_upslst_item ** const a_mproduct_list)
{
  t_upslst_item *mproduct_item;
  t_upstyp_matched_product *mproduct;

  /* make sure we are at the beginning of the list */
  *a_mproduct_list = upslst_first(*a_mproduct_list);

  for (mproduct_item = *a_mproduct_list ; mproduct_item ; 
       mproduct_item = mproduct_item->next) {
    mproduct = (t_upstyp_matched_product *)mproduct_item->data;
    (void) ups_free_matched_product(mproduct);
  }

  /* Now free the list */
  *a_mproduct_list = upslst_free(*a_mproduct_list, ' ');

  return NULL;
}
/*-----------------------------------------------------------------------
 * upsutl_free_matched_instance_list
 *
 * Given a matched instance list, free all the matched instances and the
 * list too.
 * 
 * Input : matched instance list pointer
 * Output: none
 * Return: Null
 */
t_upslst_item *upsutl_free_matched_instance_list(
					   t_upslst_item ** const a_minst_list)
{
  t_upslst_item *list_item = NULL;
  t_upstyp_matched_instance *minst_ptr = NULL;
  
  /* make sure we are at the beginning of the list */
  *a_minst_list = upslst_first(*a_minst_list);

  /* free the instances */
  for (list_item = *a_minst_list; list_item; list_item = list_item->next) {
    minst_ptr = (t_upstyp_matched_instance *)(list_item->data);
    (void) ups_free_matched_instance(minst_ptr);
  }

  /* Now free the list */
  *a_minst_list = upslst_free(*a_minst_list, ' ');

  return NULL;
}
/*-----------------------------------------------------------------------
 * upsutl_free_inst_list
 *
 * Given an instance list, free all the instances and the list too.
 * 
 * Input : instance list pointer
 * Output: none
 * Return: Null
 */
t_upslst_item *upsutl_free_inst_list( t_upslst_item ** const a_inst_list)
{
  t_upslst_item *list_item = NULL, *tmp_inst_list = NULL;
  
  /* make sure we are at the beginning of the list */
  *a_inst_list = upslst_first(*a_inst_list);

  /* free the instances */
  tmp_inst_list = *a_inst_list;
  for (list_item = tmp_inst_list; list_item; list_item = list_item->next) {
    (void) ups_free_instance((t_upstyp_instance *)(list_item->data));
  }

  /* Now free the list */
  *a_inst_list = upslst_free(*a_inst_list, ' ');

  return NULL;
}
/*-----------------------------------------------------------------------
 * upsutl_get_files
 *
 * Given a directory, return a listing of all the files which
 * have the specified pattern in them.  The pattern is 
 * removed from the returned string.  The pattern must have been at the
 * end of the original string.
 * 
 * Input : Directory where to get files, pattern to match in files
 * Output: a list of the file names
 * Return: none
 */
void upsutl_get_files(const char * const a_dir,
		      const char * const a_pattern,
		      t_upslst_item ** const a_file_list)
{
/* #ifdef _SYSTYPE_SVR4 */
  struct dirent *dir_line = NULL;
/* #else
  struct direct *dir_line = NULL;
#endif */
  DIR *dir = NULL;
  char *new_string = NULL, *substr = NULL;
  int plen;

  if ((dir = opendir(a_dir))) {
    if (! NOT_EQUAL_ANY_MATCH(a_pattern)) {
      /* read each directory item and add it to the list */
      while ((dir_line = readdir(dir)) != NULL) {
	if (strncmp(dir_line->d_name, ".", 1)) {
	  if ((new_string = upsutl_str_create(dir_line->d_name, ' '))) {
	    *a_file_list = upslst_add((t_upslst_item *)*a_file_list,
				      (void *)new_string);
	  }
	}
      }
    } else {
      /* read each directory item and if it contains the pattern at the end
	 of the string, remove the pattern from the file name and add it to
	 the list */
      plen = (int )strlen (a_pattern);
      while ((dir_line = readdir(dir)) != NULL) {
	if (strncmp(dir_line->d_name, ".", 1)) {
	  /* if the pattern is there, make sure it is at the end of the
	     string before removing it */
	  if ((substr = strstr(dir_line->d_name, a_pattern))) {
	    /* the pattern is in the string, now see if it is at the end */
	    if ((int )(*(substr + plen)) == 0) {
	      new_string = upsutl_strstr(dir_line->d_name, a_pattern);
	      if (UPS_ERROR == UPS_SUCCESS) {
		if (new_string) {
		  *a_file_list = upslst_add((t_upslst_item *)*a_file_list,
					    (void *)new_string);
		}
	      }
	    }
	  }
	}
      }
    }
    /* this guy will free 'dir' */
    (void) closedir(dir);

    /* return the first element in the list */
    if (*a_file_list) {
      *a_file_list = upslst_first(*a_file_list);
    }
  } else {
    upserr_add(UPS_OPEN_FILE, UPS_WARNING, a_dir);
  }
}

/*-----------------------------------------------------------------------
 * upsutl_get_prod_dir
 *
 * Given a database and a product name return a character string of the
 * product directory.
 * 
 * Input : Database and product name
 * Output: none
 * Return: product directory string
 */
char *upsutl_get_prod_dir(const char * const a_db,
			  const char * const a_prod_name)
{
  static char prod_dir[MAX_LINE_LEN];
  char *prod_ptr = prod_dir;

  /* There are 3 cases to handle -  both a_db and a_prod_name exist,
     or only one of the 2 exists.  We do not care about the case where only
     a_db exists as we need a product name too */
  if (a_prod_name) {
    if (a_db) {
      if (((int )strlen(a_db) + (int )strlen(a_prod_name)) < MAX_LINE_LEN) {
	(void) sprintf(prod_ptr, "%s/%s", a_db, a_prod_name);
      }
    } else {
      if ((int )strlen(a_prod_name) < MAX_LINE_LEN) {
	(void) strcpy(prod_ptr, a_prod_name);
      }
    }
  } else {
    prod_ptr = NULL;
  }

  return(prod_ptr);
}

/*-----------------------------------------------------------------------
 * upsutl_start_timing
 *
 * Start timing the current function
 *
 * Input : none
 * Output: none
 * Return: none
 */
void upsutl_start_timing(void)
{
  g_start_w = time(NULL);
  g_start_cpu = clock();
}

/*-----------------------------------------------------------------------
 * upsutl_stop_timing
 *
 * Stop timing the current function.  Return the duration as part of the
 * error stack.
 *
 * Input : none
 * Output: none
 * Return: none
 */
void upsutl_stop_timing(void)
{
  double duration_cpu, duration_w;

  g_finish_cpu = clock();
  g_finish_w = time(NULL);
  duration_cpu = ((double )(g_finish_cpu - g_start_cpu))/(double )CLOCKS_PER_SEC;
  duration_w = difftime(g_finish_w, g_start_w);

  /* Add the times to the error stack */
  upserr_add(UPS_TIME, UPS_INFORMATIONAL, duration_cpu, duration_w);
}

/*-----------------------------------------------------------------------
 * upsutl_statistics
 *
 * Save statistics on the command just executed and the product(s) involved.
 * The data saved are - 
 *         username
 *         date and time
 *         hostname
 *         command executed
 *         instance accessed (version, flavor and qualifiers)
 *
 * Input : pointer to instance for which to save statistics,
 *         pointer to the command that was executed
 *         flag which indicates if we should write statistics for all
 *            instances no matter what
 * Output: none
 * Return: none
 */
void upsutl_statistics(t_upslst_item const * const a_mproduct_list,
		       char const * const a_command)
{ 
  
  t_upslst_item *minst_item = NULL, *mproduct_item = NULL;
  t_upstyp_matched_product *mproduct = NULL;
  t_upstyp_matched_instance *minst = NULL;
  char *tmp_stat;
  char stat_file[FILENAME_MAX+1];
  int dir_s, stat_s, file_s, global_yes = 0;
  FILE *file_stream = 0;
  char mode[] = "a,access=lock";              /* create new file or append
						 to existing one */
  char *time_date, *user, *ptr;
  mode_t old_umask;
  int len;
  int save_error;

  save_error = UPS_ERROR;

  /* Get the current time and date (with spaces) */
  time_date = upsutl_time_date(STR_TRIM_DEFAULT);

  /* Get the current user */
  user = upsutl_user();

  for (mproduct_item = (t_upslst_item *)a_mproduct_list ;
       mproduct_item ; mproduct_item = mproduct_item->next) {
    mproduct = (t_upstyp_matched_product *)mproduct_item->data;
    if (mproduct->product) {
      if ((mproduct->db_info) && (mproduct->db_info->config) && 
	  (tmp_stat = mproduct->db_info->config->statistics)) {
	if (! NOT_EQUAL_ANY_MATCH(tmp_stat)) {
	  global_yes = 1;      /* write statistics for everything in this db */
	} else {
	  global_yes = 0;
	  len = (int )strlen(mproduct->product);
	  ptr = strstr(tmp_stat, mproduct->product);
	  /* first check if the product is at the beginning of the list of is
	     the only element in the list of products */
	  if (ptr && (tmp_stat == ptr)) {
	    if (((ptr + len)[0] ==  '\0') || ((int )(ptr + len)[0] == ':')) {
	      global_yes = 1;
	    }
	    /* now check if the product is at the end of the list or in the
	       middle */
	  } else if (ptr && ((int )(ptr - 1)[0] == ':')) {
	    if (((int )(ptr + len)[0] == ':') || ((ptr + len)[0] == '\0')) {
	      global_yes = 1;
	    }
	  }
	}
      }
      for (minst_item = (t_upslst_item *)mproduct->minst_list ;
	   minst_item ; minst_item = minst_item->next) {
	minst = (t_upstyp_matched_instance *)minst_item->data;
	/* check to see if the instance indicates to keep statistics */
	if (global_yes || (minst->chain && minst->chain->statistics) ||
	                  (minst->version && minst->version->statistics) ||
	                  (minst->table && minst->table->statistics)) {
	  if (! file_stream ) {       /* we need to still open the stream */
	    dir_s = (int )strlen(mproduct->db_info->name);
	    stat_s = (int )strlen(g_stat_dir);
	    file_s = (int )strlen(mproduct->product);
	    if ( (dir_s + UPS_FILES_LEN + stat_s + file_s + 4) <
		 FILENAME_MAX) {
	      /* Construct filename where the statistics are to be stored. */
	      (void) sprintf(stat_file, "%s/%s/%s", mproduct->db_info->name,
		      UPS_FILES, g_stat_dir);

	      /* check to make sure the statistics directory exists first */
	      if (upsutl_is_a_file(stat_file) == UPS_NO_FILE) {
		/* no statistics directory, this is a warning error */
		upserr_add(UPS_NO_FILE, UPS_WARNING, stat_file);
	      } else {
		(void) strcat(stat_file, mproduct->product);         /* filename */
		
		/* See if we can open the file (rw) to write to. */
		old_umask = umask(g_umask);
		
		if ((file_stream = fopen(stat_file, mode)) == NULL) {
		  /* Error opening file */
		  upserr_add(UPS_SYSTEM_ERROR, UPS_WARNING, "fopen",
			     strerror(errno));
		  upserr_add(UPS_OPEN_FILE, UPS_WARNING, stat_file);
		  upserr_add(UPS_STATISTICS, UPS_WARNING, mproduct->product);
		}
		/* set this back to what it was */
		(void) umask(old_umask);
	      }
	    } else {
	      /* Error size of directory path to file is too long */
	      upserr_add(UPS_NAME_TOO_LONG, UPS_WARNING, FILENAME_MAX);
	    }
	  }
	  if (file_stream) {
	    /* note the format of this output should be the same as that for 
	       ups list -K+, with the extra information added on at the end */
	    if (fprintf(file_stream, "\"%s\" ", mproduct->product)) {
	      if (minst->version) {
		(void) fprintf(file_stream, "\"%s\" \"%s\" \"%s\" \"\" ",
			       minst->version->version, minst->version->flavor,
			       minst->version->qualifiers);
	      } else if (minst->table) {
		(void) fprintf(file_stream, "\"\" \"%s\" \"%s\" \"\" ",
			       minst->table->flavor,
			       minst->table->qualifiers);
	      }
	      if (! fprintf(file_stream, "\"%s\" \"%s\" \"%s\"\n", user,
			    time_date, a_command))
		upserr_add(UPS_SYSTEM_ERROR, UPS_WARNING, "fprintf",
			   strerror(errno));
	      break;
	    } else {
	      upserr_add(UPS_SYSTEM_ERROR, UPS_WARNING, "fprintf",
			 strerror(errno));
	      break;
	    } 
	  } else {     /* error opening file stream already added to buffer */
	    break;
	  }
	}
      }

      /* Close the file if it was opened */
      if (file_stream != NULL) {
	(void) fclose(file_stream);
	file_stream = NULL;
      }
    }
  }

  /* reset UPS_ERROR because we do not want any errors here to cause
     damage anywhere else. */
  UPS_ERROR = save_error;
}

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison. The mappings are
 * based upon ascii character sequences.
 * Used by upsutl_stricmp and upsutl_strincmp.
 */
static unsigned char stricmp_charmap[] = {
      0000, 0001, 0002, 0003, 0004, 0005, 0006, 0007,
      0010, 0011, 0012, 0013, 0014, 0015, 0016, 0017,
      0020, 0021, 0022, 0023, 0024, 0025, 0026, 0027,
      0030, 0031, 0032, 0033, 0034, 0035, 0036, 0037,
      0040, 0041, 0042, 0043, 0044, 0045, 0046, 0047,
      0050, 0051, 0052, 0053, 0054, 0055, 0056, 0057,
      0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067,
      0070, 0071, 0072, 0073, 0074, 0075, 0076, 0077,
      0100, 0141, 0142, 0143, 0144, 0145, 0146, 0147,
      0150, 0151, 0152, 0153, 0154, 0155, 0156, 0157,
      0160, 0161, 0162, 0163, 0164, 0165, 0166, 0167,
      0170, 0171, 0172, 0133, 0134, 0135, 0136, 0137,
      0140, 0141, 0142, 0143, 0144, 0145, 0146, 0147,
      0150, 0151, 0152, 0153, 0154, 0155, 0156, 0157,
      0160, 0161, 0162, 0163, 0164, 0165, 0166, 0167,
      0170, 0171, 0172, 0173, 0174, 0175, 0176, 0177,
      0200, 0201, 0202, 0203, 0204, 0205, 0206, 0207,
      0210, 0211, 0212, 0213, 0214, 0215, 0216, 0217,
      0220, 0221, 0222, 0223, 0224, 0225, 0226, 0227,
      0230, 0231, 0232, 0233, 0234, 0235, 0236, 0237,
      0240, 0241, 0242, 0243, 0244, 0245, 0246, 0247,
      0250, 0251, 0252, 0253, 0254, 0255, 0256, 0257,
      0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,
      0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,
      0300, 0341, 0342, 0343, 0344, 0345, 0346, 0347,
      0350, 0351, 0352, 0353, 0354, 0355, 0356, 0357,
      0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,
      0370, 0371, 0372, 0333, 0334, 0335, 0336, 0337,
      0340, 0341, 0342, 0343, 0344, 0345, 0346, 0347,
      0350, 0351, 0352, 0353, 0354, 0355, 0356, 0357,
      0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,
      0370, 0371, 0372, 0373, 0374, 0375, 0376, 0377
};

/*-----------------------------------------------------------------------
 * upsutl_stricmp
 *
 * A case insensitive version of strcmp. It's a directly copy of
 * strcasecmp (found e.g in gnu's libc), which is normaly not
 * supported in ANSI C.
 *
 * Input : char *, first string to compare.
 *         char *, second string to compare.
 * Output: none
 * Return: as strcmp.
 */
int upsutl_stricmp( const char *s1, const char *s2 )
{
  register unsigned int u1, u2;
  register int result;

  if (0 == s1) {
    if (s2 == s1)
       return 0;
    else
       return -1;
  }

  if (0 == s2)
    return 1;

  for (;;) {
    u1 = (unsigned char) *s1++;
    u2 = (unsigned char) *s2++;
    result = (int) stricmp_charmap[u1] - (int) stricmp_charmap[u2];
    if (result != 0 || u1 == '\0') {
      break;
    }
  }
  return result;
}

/*-----------------------------------------------------------------------
 * upsutl_strincmp
 *
 * A case insensitive version of strncmp. It's based on 
 * strcasecmp (found e.g in gnu's libc), which is normaly not
 * supported in ANSI C.
 *
 * Input : char *, first string to compare.
 *         char *, second string to compare.
 *         size_t, number of characters to compare.
 * Output: none
 * Return: as strcmp.
 */
int upsutl_strincmp( const char *s1, const char *s2, const size_t n )
{
  register unsigned int u1, u2;
  register int i = 0;
  register int result;

  for (;;) {
    u1 = (unsigned char) *s1++;
    u2 = (unsigned char) *s2++;
    result = (int) stricmp_charmap[u1] - (int) stricmp_charmap[u2];
    if (result != 0 || u1 == '\0' || ++i == (int) n) {
      break;
    }
  }
  return result;
}

/*-----------------------------------------------------------------------
 * upsutl_stricmp
 *
 * A case insensitive version of strcmp. It's a directly copy of
 * strcasecmp (found e.g in gnu's libc), which is normaly not
 * supported in ANSI C.
 *
 * Input : char *, string to upcase.
 * Output: none
 * Return: upcased string.
 */
char *upsutl_upcase(const char * const a_str)
{
  static char dum_str[MAX_LINE_LEN];
  int i, len = (int )strlen(a_str);

  for (i = 0 ; i < len ; ++i ) {
    dum_str[i] = (char )toupper((int )a_str[i]);
  }
  
  /* NULL terminate the string */
  dum_str[i] = '\0';
  
  return (char *)(&dum_str[0]);
  
}
/*-----------------------------------------------------------------------
 * upsutl_strstr
 *
 * Search for the specified pattern in the string.  If it is not there,
 * return NULL, else copy the string to a new string, remove the pattern
 * and return the new string.
 *
 * Input : A string to search, a pattern to search for.
 * Output: none
 * Return: A pointer to a new string without the pattern or NULL.
 */
char *upsutl_strstr( const char * const a_str, const char * const a_pattern )
{
  char *substr = NULL, *new_string = NULL;
  int pat_len, str_len, substr_pos;
  int i, j;

  if ((substr = strstr(a_str, a_pattern))) {
    /* The pattern was found */
    str_len = (int )strlen(a_str);
    if ((new_string = upsutl_str_create((char *)a_str, ' '))) {
      /* create a new string without the pattern */
      pat_len = (int )strlen(a_pattern);
      substr_pos = (int )(substr - a_str);   /* char position of sub string */
      for (i = substr_pos, j = substr_pos + pat_len ; j <= str_len ;
	   ++i, ++j ) {
	new_string[i] = new_string[j];
      }

    }
  }

  return new_string;
}
/*-----------------------------------------------------------------------
 * upsutl_time_date
 *
 * Return an ascii representation of the current time and date
 *
 * Input : a flag whether or not to return a string with spaces or '_'
 * Output: none
 * Return: character string containing the current time and date
 *          formatted like "YYYY-MM-DD HH:MM:SS TZ"
 */
char *upsutl_time_date(const int a_flag)
{
  time_t now, len;
  static char buff[MAX_LINE_LEN];
  char *buf_ptr;

  /* Get the current time and date */
  now = time(NULL);
  if (a_flag == STR_TRIM_DEFAULT) {
    len = (time_t )strftime(buff, MAX_LINE_LEN, "%Y-%m-%d %H.%M.%S GMT",
			    gmtime(&now));
  } else {
    len = (time_t )strftime(buff, MAX_LINE_LEN, "%Y-%m-%d_%H.%M.%S_GMT",
			    gmtime(&now));
  }
  if (len == 0) {
    buf_ptr = NULL;
  } else {
    buf_ptr = buff;
  }
  return (buf_ptr);
}

/*-----------------------------------------------------------------------
 * upsutl_user
 *
 * Return an ascii representation of the current user
 *
 * Input : none
 * Output: none
 * Return: character string containing the current user
 */
char *upsutl_user(void)
{
  char *username = NULL;
  struct passwd *pwd;

  /* First try to get the username by looking at the environment variable
     USER..  If this does not work, call getpwuid. If this does not
     work either, set the user to UNKNOWN. */
  if ((username = (char *)getenv("USER")) == NULL) {
    if ((pwd = getpwuid(getuid())) == NULL) {
      username = g_unknown_user;
    } else {
      username = pwd->pw_name;
    }
  }
  return (username);
}

/*-----------------------------------------------------------------------
 * upsutl_str_create
 *
 * Will create a sring on the heap, using upsmem. 
 *
 * Input : char *, string to be copied.
 *         char, options, STR_TRIM will trim edges of passed string.
 *             ,          STR_TRIM_PACK will trim full string (packing).
 *             ,          STR_TRIM_DEFAULT will do nothing.
 * Output: none
 * Return: char *, new string.
 */
char *upsutl_str_create( char * const str, const char copt )
{
  char *new_str = 0;
  
  if ( !str ) return 0;

  if ( copt == STR_TRIM || copt == STR_TRIM_PACK ) {
    
    /* copy overhead only when option 't' or 'p' is passed */

    static char bad_chars[] = " \t\n\r\f\"";
    static char buf[MAX_LINE_LEN];
    if ( strlen( str ) >= MAX_LINE_LEN ) {    
      upserr_add( UPS_LINE_TOO_LONG, UPS_FATAL, "upsutl_str_create" );
      return 0;
    }    
    (void) strcpy( buf, str );
    if ( copt == STR_TRIM_PACK )
      (void) upsutl_str_remove( buf, bad_chars );
    else
      (void) upsutl_str_remove_edges( buf, bad_chars );
    
    new_str = (char *)upsmem_malloc( (int)strlen( buf ) + 1 );
    (void) strcpy( new_str, buf );      
  }
  else {      
    new_str = (char *)upsmem_malloc( (int)strlen( str ) + 1 );
    (void) strcpy( new_str, str );      
  }
  
  return new_str;
}

/*-----------------------------------------------------------------------
 * upsutl_str_crecat
 *
 * It will concatenate the two passed string into a new created string.
 *
 * Input : char *, string to be copied.
 *         char *, string to be concatenate.
 * Output: none
 * Return: char *, new string.
 */
char *upsutl_str_crecat( char * const str1, char * const str2 )
{
  char *new_str = 0;
  
  if ( !str1 ) return 0;

  if ( !str2 ) {
    new_str = upsutl_str_create( str1, ' ' );
  }
  else {
    new_str = (char *)upsmem_malloc( (int )(strlen( str1 ) + strlen( str2 ) +
			      (unsigned int )1) );
    (void) strcpy( new_str, str1 );
    (void) strcat( new_str, str2 );
  }
  
  return new_str;
}

/*-----------------------------------------------------------------------
 * upsutl_str_sort
 *
 * Will sort items in a string, str. Item are delimeted by character c.
 * (e.g "dd,ttt,b,aaa," becomes "aaa,b,dd,ttt")
 *
 * Input : char *, string to be sorted
 *         char, delimeting character
 * Output: char *, sorted string
 * Return: int, number if items sorted
 */
int upsutl_str_sort( char * const str, const char c )
{
  static char buf[MAX_LINE_LEN];
  char *cp, *cp0;  
  char ct[2] = "\0\0";
  size_t max_len = 0, count = 0;
  unsigned int i = 0;

  if ( !str || strlen( str ) <= 0 ) return 0;
  
  ct[0] = c;
  (void) memset( buf, 0, MAX_LINE_LEN );
  
  /* get max len of an item */

  cp0 = str;
  while ( (cp = strchr( cp0, (int)c )) ) {
    if ( max_len < (size_t)(cp-cp0) ) max_len = (size_t)(cp-cp0);
    cp0 = cp+1;
    count++;
  }
  if ( (i=strlen( cp0 )) > max_len ) max_len = i;
  ++max_len;
  ++count;

  /* check size */
  
  if ( count*max_len >= MAX_LINE_LEN ) {    
    upserr_add( UPS_LINE_TOO_LONG, UPS_FATAL, "upsutl_str_sort" );
    return 0;
  }

  /* split, fill buf with evenly spaced items (qsort likes that) */

  count = 0;
  cp = strtok( str, ct );
  while ( cp ) {
    (void) strcpy( &buf[count*max_len], cp );
    count++;
    cp=strtok( 0, ct );
  }

  /* sort */
  
  qsort( buf, count, max_len, qsort_cmp_string );

  /* merge, write back to input */

  (void) strcpy( str, &buf[0] );
  for ( i=1; i<count; i++ ) {
    (void) strcat( str, ct );
    (void) strcat( str, &buf[i*max_len] );
  }

  return (int)count;
}

/*-----------------------------------------------------------------------
 * upsutl_str_remove
 *
 * Will erase characters, defined in str_remove, from in str.
 *
 * Input : char *, string to be trimmed
 *         char *, string of character to remove
 * Output: none
 * Return: int, length of trimmed string
 */
size_t upsutl_str_remove( char * const str, const char * const str_remove )
{
  char *cpf = str, *cp = str;  

  if ( !str || strlen( str ) <= 0 ) return 0;

  while ( cpf && *cpf ) {
    if ( ! strchr( str_remove, (int)*cpf ) ) {
      *cp = *cpf;
      cp++;
    }
    cpf++;
  }
  *cp = 0;

  return strlen( str );    
}

/*-----------------------------------------------------------------------
 * upsutl_str_remove_edges
 *
 * Will erase trailing and starting characters, defined in str_remove,
 * from str
 *
 * Input : char *, string to be trimmed
 *         char *, string of character to remove
 * Output: none
 * Return: int, length of trimmed string
 */
size_t upsutl_str_remove_edges( char * const str, const char * const str_remove )
{
  char *cp = str;
  char *cstart = 0, *cend = 0;
  size_t count = 0;
  
  if ( !str || strlen( str ) <= 0 ) return 0;
  
  while ( *cp && strchr( str_remove, (int)*cp ) ){ cp++; }
  cstart = cp;
  count = strlen( str );
  cp = &str[count - 1];
  while ( count && strchr( str_remove, (int)*cp ) ){ cp--; count--; }
  cend = cp;

  if ( cend >= cstart && cstart >= str ) {
    count = (size_t)(cend-cstart) + 1;
    (void) memmove( str, cstart, count );
    str[count] = 0;
  }
  else {
    str[0] = 0;
  }

  return strlen( str );
}

/*-----------------------------------------------------------------------
 * upsutl_str_remove_end_quotes
 *
 * Will erase trailing and starting quotes, defined in quotes.
 * If passed 'spaces' is defined it will first remove trailing
 * white spaces
 *
 * Input : char *, string to be trimmed
 *         char *, string of quotes to remove
 *         char *, string of spaces to remove (can be null)
 * Output: none
 * Return: int, length of trimmed string
 */
size_t upsutl_str_remove_end_quotes( char * str, 
				     char * const quotes, 
				     char * const spaces )
{
  int len = 0;
  char *qu = 0;

  if ( !str )
    return (size_t )len;

  if ( spaces )
    (void) upsutl_str_remove_edges( str, spaces );

  len = (int)strlen( str );
  if ( !quotes || len < 2 )
    return (size_t )len;

  for ( qu = quotes; qu && *qu; qu++ ) {
    if ( (str[0] == *qu && str[len-1] == *qu) ) {
      char *sp1 = &str[1], *sp2 = &str[len-1];
      for ( ; sp1 < sp2; str++, sp1++ ) *str = *sp1;
      *str = 0;
      break;
    }
  }
  return (size_t )strlen( str );
}

/*-----------------------------------------------------------------------
 * upsutl_is_a_file
 *
 * Given a filename including path, see if the named file exists.
 *
 * Input : file name and path
 * Output: none
 * Return: UPS_SUCCESS if file exists, else UPS_NO_FILE
 */
int upsutl_is_a_file(const char * const a_filename)
{
  int status = UPS_SUCCESS;
  struct stat buf;

  if (stat(a_filename, &buf) == -1) {
    status = UPS_NO_FILE;
  }

  return(status);
  
}

/*-----------------------------------------------------------------------
 * upsutl_is_a_dir
 *
 * Given a filename including path, see if the path is a directory (as opposed
 * to anything else).
 *
 * Input : path
 * Output: none
 * Return: UPS_SUCCESS if file exists, else UPS_NO_STAT_DIR
 */
int upsutl_is_a_dir(const char * const a_pathname)
{
  int status = UPS_SUCCESS;
  struct stat buf;

  if (stat(a_pathname, &buf) == -1) {
    status = UPS_NO_STAT_DIR;
  }
  if (!S_ISDIR(buf.st_mode)) {
    status = UPS_NO_STAT_DIR;
  }

  return(status);
  
}

/*
 * Definition of private functions
 */

/* used by qsort */
int qsort_cmp_string( const void * c1, const void * c2 )
{
  return strcmp( (const char *)c1, (const char *)c2 );
}
