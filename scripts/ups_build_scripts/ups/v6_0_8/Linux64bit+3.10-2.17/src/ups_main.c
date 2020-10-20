/************************************************************************
 *
 * FILE:
 *       ups_main.c
 * 
 * DESCRIPTION: 
 *       This is the main line for ups commands.
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
 *       18-Aug-1997, EB, first
 *
 ***********************************************************************/

/* standard include files */
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>				/* getcwd & execvp */

/* ups specific include files */
#include "ups_main.h"
#include "upslst.h"
#include "upstyp.h"
#include "ups_setup.h"
#include "ups_unsetup.h"
#include "ups_start.h"
#include "ups_stop.h"
#include "ups_configure.h"
#include "ups_unconfigure.h"
#include "ups_list.h"
#include "ups_tailor.h"
#include "ups_unk.h"
#include "ups_depend.h"
#include "ups_touch.h"
#include "upserr.h"
#include "upsutl.h"
#include "upshlp.h"
#include "upsget.h"
#include "upsact.h"
#include "upsver.h"
#include "upsfil.h"
#include "ups_declare.h"
#include "ups_undeclare.h"
#include "ups_flavor.h"
#include "ups_get.h"
#include "ups_copy.h"
#include "ups_modify.h"

/*
 * Definition of public variables.
 */
extern int UPS_VERBOSE;
extern int g_keep_temp_file;
extern char *g_temp_file_name;
extern t_cmd_info g_cmd_info[];
extern int g_UPS_SHELL; /* Ugliness!!! */
int g_simulate = 0;
static mode_t g_umask = 022;
extern int UPS_NEED_DB;
extern char upsact_dropit_buf[];

/*
 * Declaration of private functions.
 */

/*
 * Definition of global variables.
 */
#ifndef NULL
#define NULL 0
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


/*
 * And now for something completely different
 */
int main(int argc, char *argv[])
{
  t_upsugo_command *command_line = NULL;
  FILE *temp_file = NULL;
  int i = e_setup, temp_shell = UPS_INVALID_SHELL;
  int rstatus = 0;              /* assume success */
  t_upslst_item *mproduct_list  = NULL;
  mode_t old_umask;
  int need_help = 0;
  char *on_what = NULL;
  char *UPS_DIR = 0;

  UPS_DIR = getenv("UPS_DIR");
  if ( UPS_DIR ) {
      snprintf(upsact_dropit_buf, 2048, "%s/bin/dropit -e -n '' ", UPS_DIR);
  }

  if (argv[1] && (0 == strcmp(argv[1],"active")) && (!argv[2] || 0 != strcmp(argv[2],"-?"))) {

      /* ups active is an external perl script */
      argv[1] = "ups_active";
      execvp( "ups_active", argv+1 );
      exit(1);
  }

  if (argv[1] && (0 == strcmp(argv[1],"parent")) && (!argv[2] || 0 != strcmp(argv[2],"-?"))) {

      /* ups parent is an external perl script */
      argv[1] = "ups_parent";
      execvp( "ups_parent", argv+1 );
      exit(1);
  }

  if (argv[1] && (0 == strcmp(argv[1],"platform")) && (!argv[2] || 0 != strcmp(argv[2],"-?"))) {

      /* ups platform is an external shell script */
      argv[1] = "ups_platform.sh";
      execvp( "ups_platform.sh", argv+1 );
      exit(1);
  }

  if (argv[1] && (strcmp(argv[1],"-?"))) {
    /* Figure out which command was entered */
    while (g_cmd_info[i].cmd) {
      if (! strcmp(g_cmd_info[i].cmd, argv[1])) {
	UPS_NEED_DB = UPSACT_CMD_NEEDS_DB(g_cmd_info[i].flags);
	break;
      }
      ++i;
    }

    g_command_verb = i;

    /* skip the command name */
    --argc;

    /* get the options for each iteration of the command and do it */
    while ((command_line = upsugo_next(argc, &argv[1],
				       g_cmd_info[i].valid_opts))) {

      if (command_line->ugo_Z) {
	upsutl_start_timing();
      }

      /* we will need this later after command_line goes away, so keep track
	 of it here. only attempt to save it if it has not already been
	 saved */
      if (! g_keep_temp_file ) {
	g_keep_temp_file = command_line->ugo_V;
      }
      if (! g_simulate) {
	g_simulate = command_line->ugo_s;
      }

      g_command_ugoB = command_line->ugo_B;

      if (!command_line->ugo_help && (g_cmd_info[i].cmd_index != e_help)) {
	/* no help requested - do the command */

	/* open the temp file. this is where shell specific action code will\
	   be put */
	if (! temp_file ) {                /* only open it once. */
	  /* let the system get me a buffer */
	  if ((g_temp_file_name = tempnam(NULL,"ups")) != NULL) {
	    /* See if we can open the file (rw) to write to. */
	    old_umask = umask(g_umask);
	    (void) umask(old_umask | 022);

	    if ((temp_file = fopen(g_temp_file_name,"w")) == NULL)
	    {
	      /* error in open */
	      upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "fopen",
			 strerror(errno));
	      upserr_add(UPS_OPEN_FILE, UPS_FATAL, g_temp_file_name);
	    }
	    /* set this back to what it was */
	    (void) umask(old_umask);
	  } else {
	    upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "tempnam", strerror(errno));
	  }
	}

	if (UPS_ERROR == UPS_SUCCESS) {
	  switch (g_cmd_info[i].cmd_index) {
	  case e_setup: mproduct_list = 
			  ups_setup(command_line, temp_file, e_setup,
				    (int )FALSE);
	    break;
	  case e_unsetup: mproduct_list = 
			    ups_unsetup(command_line, temp_file, e_unsetup);
	    break;
	  default:
	    /* any of the following commands will use a call to the system
	       function to process ups functions.  this call must have
	       everything in shell.  so change whatever we have here to be
	       bourne shell */
	    command_line->ugo_shell = e_BOURNE;
            g_UPS_SHELL = e_BOURNE;
	    /* fix the environment, too, 'cause ups_ugo will set it again...*/
	    putenv("UPS_SHELL=sh");

	    switch (g_cmd_info[i].cmd_index) {
	    case e_layout: mproduct_list = ups_layout(command_line,0);
              break;
	    case e_list: mproduct_list = ups_list(command_line,0);
	      break;
	    case e_verify: mproduct_list = 
			     ups_list(command_line,1); /* verify IS list !! */
	      break;
	    case e_configure: mproduct_list =
				ups_configure(command_line, temp_file,
					      e_configure);
	      break;
	    case e_copy: mproduct_list = 
			   ups_copy(command_line, temp_file, e_copy);
	      break;
	    case e_depend: mproduct_list = 
			     ups_depend(command_line, argv[1], e_depend);
	      break;
	    case e_exist: mproduct_list = 
			    ups_setup(command_line, temp_file, e_setup,
				      (int )TRUE);
	      break;
	    case e_modify: mproduct_list = 
			     ups_modify(command_line, temp_file, e_modify);
	      break;
	    case e_start: mproduct_list = 
			    ups_start(command_line, temp_file, e_start);
	      break;
	    case e_stop: mproduct_list = 
			   ups_stop(command_line, temp_file, e_stop);
	      break;
	    case e_tailor: mproduct_list =
			     ups_tailor(command_line, temp_file, e_tailor);
	      break;
	    case e_unconfigure: mproduct_list =
				  ups_unconfigure(command_line, temp_file,
						  e_unconfigure);
	      break; 
	    case e_undeclare: mproduct_list =
				ups_undeclare(command_line, temp_file,
					      e_undeclare);
	      break;
	    case e_flavor: ups_flavor(command_line);
	      break;
	    case e_get: mproduct_list = ups_get(command_line, temp_file,
						e_get);
	      break;
	    case e_declare: mproduct_list =
			      ups_declare(command_line, temp_file, e_declare);
	      break;
	    case e_touch: mproduct_list =
			    ups_touch(command_line, temp_file, e_touch);
	      break;
	    case e_unk: mproduct_list = 
			  ups_unk(command_line, temp_file, argv[1]);
	      break;
	    }
	  }

	  /* write out statistics info */
	  if ((UPS_ERROR == UPS_SUCCESS) && mproduct_list) {
            char *p;
            if (g_cmd_info[i].cmd_index >= e_unk)
            {
              p = argv[1];
              while (*p)
              {
                *p = (char) tolower ((int) *p);   /* We're done with argv[1] anyway */
                ++p;
              }
            }
	    upsutl_statistics(mproduct_list,
                              ((g_cmd_info[i].cmd_index >= e_unk) ? argv[1] : g_cmd_info[i].cmd));
	  }
	}
      } else {
	/* output help */
	switch (g_cmd_info[i].cmd_index) {
	case e_help:
	case e_unk:
	  need_help = 1;
	  break;
	  /* specific help */
	default:    
	  need_help = 1;
	  on_what = g_cmd_info[i].cmd;
	}
      }
      if (command_line->ugo_Z) {
	upsutl_stop_timing();
      }

      if (UPS_ERROR != UPS_SUCCESS) {
	rstatus = 1;                   /* return an error to the user */
	switch (g_cmd_info[i].cmd_index) {
        case e_setup:
            (void) fprintf(stderr, "Error encountered when setting up product: %s\n", command_line->ugo_product );
        }
	break;
      } else if ((g_cmd_info[i].cmd_index == e_exist) && (! mproduct_list) &&
		 !command_line->ugo_help) {
	rstatus = 1;                   /* error if found no product */
	break;
      }

      /* we need to save the shell info here as the next call to upsugo_next
	 will free the structure we have now.  and we will need the shell info
	 later */
      temp_shell = command_line->ugo_shell;
    }
  } else {
    /* no parameters were entered - give help */
    need_help = 1;
  }

  /* finish writing stuff to the temp file, close it and execute it or output
     its name if necessary.  Also flush the journal files. */
  {
    int tstatus;
    tstatus = upsutl_finish_up(temp_file, temp_shell, ((need_help == 0) ? i : e_help), g_simulate);
    rstatus = rstatus ? rstatus : tstatus;
  }

  /* output any help that was asked for */
  if (need_help) {
    (void )upshlp_command(on_what);       /* print out all help */
  }

  /* output any errors and the timing information */
  upserr_output();

  return rstatus;
}



