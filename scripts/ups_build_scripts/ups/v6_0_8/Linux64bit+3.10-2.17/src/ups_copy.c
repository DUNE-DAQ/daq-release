/************************************************************************
 *
 * FILE:
 *       ups_copy.c
 * 
 * DESCRIPTION: 
 *       Declare a new instance of a product using an existing instance as
 *       a template.
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
 *       7-Jan-1998, EB, first
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* ups specific include files */
#include "upserr.h"
#include "upsugo.h"
#include "upsutl.h"
#include "upsget.h"
#include "upsmat.h"
#include "upsact.h"
#include "upsmem.h"
#include "upslst.h"
#include "upstyp.h"
#include "upsfil.h"
#include "ups_main.h"
#include "ups_declare.h"
#include "ups_copy.h"
/*
 * Definition of public variables.
 */


/*
 * Declaration of private functions.
 */

static t_upstyp_instance *fill_new_dclr_instance(
                                      t_upstyp_instance * const a_tinst,
				      t_upstyp_instance * const a_old_vinst,
				      t_upsugo_command *const new_command_line,
				      char * const a_prod_dir,
				      char * const a_ups_dir,
				      char * const a_table_dir,
				      char * const a_table_file,
				      t_upstyp_db * const a_db);
static t_upstyp_instance *free_new_dclr_instance(
					t_upstyp_instance * const a_instance);
static t_upsugo_command *fill_ugo_struct( 
                               const t_upstyp_instance * const a_instance,
			       const t_upsugo_command * const a_command_line);
static t_upsugo_command *free_ugo_struct(
			       t_upsugo_command * const a_command_line);

/*
 * Definition of global variables.
 */

extern t_cmd_info g_cmd_info[];
extern t_cmd_map g_func_info[];

#define TRUE 1
#define FALSE 0

/*
 * Definition of public functions.
 */

static char buf[MAX_LINE_LEN];

#define COPY_TO_INSTANCE(the_inst, ugo_elem, old_inst, inst_elem)  \
  if (new_command_line && new_command_line->ugo_elem) {                 \
    the_inst->inst_elem = (char *)new_command_line->ugo_elem;           \
    upsmem_inc_refctr(new_command_line->ugo_elem);                      \
  } else if (old_inst->inst_elem) {                                     \
    the_inst->inst_elem = old_inst->inst_elem;                          \
    upsmem_inc_refctr(old_inst->inst_elem);                             \
  } else {                                                              \
    the_inst->inst_elem = 0;                                            \
  }

#define COPY_TO_INSTANCE_CHECK(ugo_flag, ugo_elem, inst_elem)  \
  if (new_command_line && new_command_line->ugo_flag &&                 \
      new_command_line->ugo_elem) {                                     \
    new_instance->inst_elem = (char *)new_command_line->ugo_elem;       \
    upsmem_inc_refctr(new_command_line->ugo_elem);                      \
  } else if (old_version_instance->inst_elem) {                         \
    new_instance->inst_elem = old_version_instance->inst_elem;          \
    upsmem_inc_refctr(old_version_instance->inst_elem);                 \
  } else {                                                              \
    new_instance->inst_elem = 0;                                        \
  }

#define COPY_FROM_OLD_INSTANCE(the_inst, old_inst, inst_elem)  \
  if (old_inst->inst_elem) {                                            \
    the_inst->inst_elem = old_inst->inst_elem;                          \
    upsmem_inc_refctr(old_inst->inst_elem);                             \
  } else {                                                              \
    the_inst->inst_elem = 0;                                            \
  }

#define COPY_TO_NEW_VAR(ugo_elem, inst_elem, new_var)  \
  new_var = ((new_command_line && new_command_line->ugo_elem) ?             \
	     new_command_line->ugo_elem :                                   \
	     (old_version_instance ? old_version_instance->inst_elem : 0));

#define ADD_TO_ECHO(inst_elem, format)  \
   if (new_dclr_instance->inst_elem) {                                 \
     (void) sprintf(tmp_buf, format, tmp_buf, new_dclr_instance->inst_elem);  \
   }

#define SET_DCLR_LINE(ugo_elem, inst_elem, ugo_flag)  \
   if (a_instance->inst_elem) {                               \
     new_command_line->ugo_flag = TRUE;                       \
     if (new_command_line->ugo_elem) {                        \
       upsmem_free(new_command_line->ugo_elem);               \
     }                                                        \
     new_command_line->ugo_elem = a_instance->inst_elem;      \
     upsmem_inc_refctr(a_instance->inst_elem);                \
   } else {                                                   \
     new_command_line->ugo_flag = FALSE;                      \
     new_command_line->ugo_elem = NULL;                       \
   }

#define SET_DCLR_LINE_LIST(ugo_elem, inst_elem, ugo_flag)                   \
   if (a_instance->inst_elem) {                                             \
     new_command_line->ugo_flag = TRUE;                                     \
     if (new_command_line->ugo_elem) {                                      \
       new_command_line->ugo_elem = upslst_free(new_command_line->ugo_elem, \
						'd');                       \
     }                                                                      \
     new_command_line->ugo_elem = upslst_new(a_instance->inst_elem);        \
     upsmem_inc_refctr(a_instance->inst_elem);                              \
   } else {                                                                 \
     new_command_line->ugo_flag = FALSE;                                    \
     new_command_line->ugo_elem = NULL;                                     \
   }

/*-----------------------------------------------------------------------
 * ups_copy
 *
 * Cycle through the actions for copy, translate them,
 * and write them to the temp file.
 *
 * Input : information from the command line, a stream.
 * Output: <output>
 * Return: matched product of the original product
 */
t_upslst_item *ups_copy(const t_upsugo_command * const a_command_line,
			const FILE * const a_temp_file,
			const int a_ups_command)
{
  t_upslst_item *mproduct_list;
  t_upstyp_matched_product *mproduct;
  t_upstyp_matched_instance *minst;
  t_upslst_item *cmd_list, *command_item, *tmp_item;
  t_upstyp_action *action;
  char *tmp_buf2, tmp_buf[MAX_LINE_LEN];
  int need_unique = 1, inst_already_there = 0;
  char *new_table_file, *new_table_dir, *new_ups_dir, *new_prod_dir;
  t_upstyp_instance *new_instance, *old_table_instance, *instance;
  t_upstyp_instance *old_version_instance;
  t_upstyp_instance *new_dclr_instance = NULL;
  t_upsact_cmd *command;
  t_upsugo_command *dep_command_line = NULL, *new_command_line = NULL;
  t_upsugo_command *dclr_command_line;
  t_upstyp_product *write_product_ptr = NULL;
  t_upstyp_product *dum_product;
  t_upstyp_db *new_db_info_ptr = NULL;

  /* get the requested instance */
  mproduct_list = upsmat_instance((t_upsugo_command *)a_command_line,
				  NULL, need_unique);
  if (mproduct_list && (UPS_ERROR == UPS_SUCCESS)) {
    /* get the product to be copied */
    mproduct = (t_upstyp_matched_product *)mproduct_list->data;

    /* make sure an instance was matched before proceeding */
    if (mproduct->minst_list) {
      minst = (t_upstyp_matched_instance *)(mproduct->minst_list->data);
      old_table_instance = minst->table;
      old_version_instance = minst->version;

      /* Now process the copy actions only if we are executing this command
	 and not just echoing it */
      if (a_command_line->ugo_X) {
	cmd_list = upsact_get_cmd((t_upsugo_command *)a_command_line,
				  mproduct, g_cmd_info[a_ups_command].cmd,
				  a_ups_command);
	if (UPS_ERROR == UPS_SUCCESS) {
	  upsact_process_commands(cmd_list, a_temp_file);
	}
	/* now clean up the memory that we used */
	upsact_cleanup(cmd_list);
      }

      /* Now we will go on and do the actual copy.  this is done in 2 steps.
	 step 1 is to update/create a table file with the new instance
	 information.  step 2 is to construct the declare command and either
	 echo it or execute it.  we get the information for step 1 from
	 the -W flag (merge with the information in the environment) and
	 from the -O options string which contains information that changes
	 from the old instance to the new.  this information can contain
	 a new flavor, qualifiers, product name, product home directory etc.
	 we need to parse the -G string to get the info. */

      /* STEP 1: translate the -G string into a ugo_command structure */
      if (a_command_line->ugo_G) {
        g_command_verb = e_declare;
	new_command_line = upsugo_bldcmd(a_command_line->ugo_passed,
				     g_cmd_info[a_ups_command].valid_opts);
	/* if the -O string did not contain a -z part to specify the ups
	   database, then bldcmd translated $PRODUCTS.  however that may
	   not be what we want.  we want the db that the matched product was
	   found in. */
	if (! new_command_line->ugo_z) {
	  /* no -z was contained in the -O string. the db list must be a
	     translation of $PRODUCTS.  use the database that the old product
	     matched in instead. */
	  upsugo_free_ugo_db(new_command_line->ugo_db);
	  new_command_line->ugo_db = upslst_new(mproduct->db_info->name);
	  upsmem_inc_refctr(mproduct->db_info->name);
	  new_db_info_ptr = mproduct->db_info;
	} else {
	  /* read in the new databases configuration file */
	  new_db_info_ptr = (t_upstyp_db *)new_command_line->ugo_db->data;
	  upsmem_inc_refctr(new_db_info_ptr);
	  dum_product = upsutl_get_config(new_db_info_ptr->name);
	  if (dum_product && dum_product->config) {
	    new_db_info_ptr->config = dum_product->config;
	  }
	}
      }
      if (new_db_info_ptr && new_db_info_ptr->config && 
         new_db_info_ptr->config->version_subdir &&  
         new_db_info_ptr->config->version_subdir[0] == '1') {

          extern int g_subdir_files_flag;
          g_subdir_files_flag = 1;
       
      } else {
          extern int g_subdir_files_flag;
          g_subdir_files_flag = 0;
      }
      /*         if the user did not enter any -O options and does not want a
		 merge with the environment then we have nothing to do */
      if (new_command_line || a_command_line->ugo_W) {
	/*       now fill in an instance structure with the new
		 information from the command line or from the matched
		 instance.  we are doing the table file first, so only copy
		 the info valid in a table file */
	new_instance = ups_new_instance();

	COPY_TO_INSTANCE(new_instance, ugo_product, old_table_instance,
			 product);
	COPY_TO_INSTANCE_CHECK(ugo_f, ugo_flavor->data, flavor);
	COPY_TO_INSTANCE_CHECK(ugo_q, ugo_qualifiers->data, qualifiers);
	COPY_TO_INSTANCE(new_instance, ugo_description, old_table_instance,
			 description);
	COPY_FROM_OLD_INSTANCE(new_instance, old_table_instance, statistics);
	COPY_FROM_OLD_INSTANCE(new_instance, old_table_instance,
			       catman_source_dir);
	COPY_FROM_OLD_INSTANCE(new_instance, old_table_instance,
			       html_source_dir);
	COPY_FROM_OLD_INSTANCE(new_instance, old_table_instance,
			       info_source_dir);
	COPY_FROM_OLD_INSTANCE(new_instance, old_table_instance,
			       man_source_dir);
	COPY_FROM_OLD_INSTANCE(new_instance, old_table_instance,
			       news_source_dir);

	if (old_table_instance && old_table_instance->action_list) {
	  new_instance->action_list = old_table_instance->action_list;
	  for (tmp_item = new_instance->action_list ; tmp_item ;
	       tmp_item = tmp_item->next) {
	    upsmem_inc_refctr(tmp_item->data);
	  }
	}

	/*       fill in the instance fields that are maintained by UPS */
	new_instance->modifier = upsutl_str_create( upsutl_user(), ' ');
	new_instance->modified = upsutl_str_create( upsutl_time_date(STR_TRIM_DEFAULT), ' ' );

	/*       if we are merging with the environment, walk through all the
		 actions.  if we encounter a setupRequired, setupOptional,
		 unsetupRequired, or unsetupOptional we must find the matching
		 SETUP_<prod> environment variable and replace the information
		 in the found action line with that from SETUP_<prod>. */
	if (a_command_line->ugo_W) {
	  for (tmp_item = new_instance->action_list ; tmp_item ;
	       tmp_item = tmp_item->next) {
	    action = (t_upstyp_action *)tmp_item->data;
	    for (command_item = action->command_list ;
		 command_item ; command_item = command_item->next) {
	      /* get each separate command line and check if is a setup or
		 unsetup command.  if so, we parse it completely, replace
		 the parameter with the value of setup_<prod>, and reconstruct
		 the command line.  the new one replaces the old one. */
	      if ((command = upsact_parse_cmd((char *)command_item->data))) {
		switch (command->icmd) {
		case e_setupoptional:
		case e_setuprequired:
		  /* transform the parameter into a ugo structure to get the
		     product name */
		  dep_command_line = upsugo_bldcmd(command->argv[0], 
					 g_cmd_info[e_setup].valid_opts);
		  break;
		case e_unsetupoptional:
		case e_unsetuprequired:
		  dep_command_line = upsugo_bldcmd(command->argv[0], 
					 g_cmd_info[e_unsetup].valid_opts);

		  /* hopefully, the unsetup arguments are only the product
		     name to unsetup and no other instance options. 
		     (otherwise, unsetup will not look at SETUP_ENV in order
		     to do the unsetup) if however there are other instance
		     options, then we need to replace the string with the
		     new information */
		  if (!dep_command_line->ugo_version && 
		      !dep_command_line->ugo_c && !dep_command_line->ugo_o &&
		      !dep_command_line->ugo_n && !dep_command_line->ugo_d &&
		      !dep_command_line->ugo_t && !dep_command_line->ugo_g &&
		      !dep_command_line->ugo_q &&
		      !dep_command_line->ugo_f && !dep_command_line->ugo_H) {
		    /* there were no other instance options, we can leave the
		       original string alone */
		    dep_command_line = 
		      (t_upsugo_command *)upsugo_free(dep_command_line);
		  }
		  break;
		}
		  /* get SETUP_<prod> translated */
		if (dep_command_line && dep_command_line->ugo_product) {
		  (void) sprintf(tmp_buf, "%s%s", SETUPENV,
			  upsutl_upcase(dep_command_line->ugo_product));
		  if ((tmp_buf2 = (char *)getenv(tmp_buf)) != NULL) {
		    /* we got it, free the current string and replace it with
		       the new one */
		    upsmem_free(command_item->data);
		    (void) sprintf(tmp_buf, "%s(\"%s\")", 
			    g_func_info[command->icmd].cmd, tmp_buf2);
		    command_item->data = (void *)upsutl_str_create(tmp_buf,
							   STR_TRIM_DEFAULT);
		  }
		  /* clean up clean up... */
		  dep_command_line = 
		    (t_upsugo_command *)upsugo_free(dep_command_line);
		}
	      }
	    }
	  }
	}
	/*     we will need this info when writing the table file but cannot
	       move it into new_instance as if we write out the table file,
	       this info is invalid in a table file */
	COPY_TO_NEW_VAR(ugo_tablefile, table_file, new_table_file);
	COPY_TO_NEW_VAR(ugo_tablefiledir, table_dir, new_table_dir);
	COPY_TO_NEW_VAR(ugo_upsdir, ups_dir, new_ups_dir);
	COPY_TO_NEW_VAR(ugo_productdir, prod_dir, new_prod_dir);
	    
	/*     determine whether we need to write out a new table file or add
	       this instance to the table file we already read in.  if we
	       are just echoing the declare line then we do not want to
	       overwrite any existing table files either.  in that case, we
	       will create a new table file in /usr/tmp and just echo the
	       name to the user. */
	if (! a_command_line->ugo_X && ! new_command_line->ugo_m) {
	  /*   do not execute, if no table file entered, the table file goes
	       to /usr/tmp , else use the one that was entered */
	  if ((tmp_buf2 = tempnam(NULL,"ups")) != NULL) {
	    (void) sprintf(tmp_buf, "%s_table_%s", tmp_buf2, new_instance->product);
            free(tmp_buf2);
	    tmp_buf2 = tmp_buf;
	    
	    /* tell user the name of the temporary file */
	    (void) printf("UPS_COPY: Temporary table file is \'%s\'\n", tmp_buf);
	  } else {
	    tmp_buf2 = NULL;
	    upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "tempnam",
		       strerror(errno));
	  }
	} else {
	  /* we will be changing things so get the real thing. we need to get
	     a few more pieces of info. */
	  if ((tmp_buf2 = 
	       upsget_table_file(new_instance->product,
				 new_table_file, new_table_dir,
				 new_ups_dir, new_prod_dir,
				 new_db_info_ptr, MUST_EXIST))
	      != NULL) {
	    /* file exists */
	    write_product_ptr = upsfil_read_file(tmp_buf2);
	    if (UPS_ERROR == UPS_SUCCESS) {
	      /* make sure we are not overwriting an existing instance. */
	      for (tmp_item = write_product_ptr->instance_list ;
		   tmp_item ; tmp_item = tmp_item->next) {
		instance = (t_upstyp_instance *)tmp_item->data;
		if (!strcmp(instance->flavor, new_instance->flavor) &&
		    !strcmp(instance->qualifiers, new_instance->qualifiers)) {
		  /* we found the same instance already in the file */
		  inst_already_there = 1;
		  break;
		}
	      }
	      if (! inst_already_there) {
		write_product_ptr->instance_list = 
		  upslst_insert(write_product_ptr->instance_list,
				new_instance);
	      }
	    }
	  } else {
	    /* file does not exist, so we must figure out where it is
	       supposed to go */
	    tmp_buf2 = upsget_table_file(new_instance->product,
					 new_table_file,
					 new_table_dir, new_ups_dir,
					 new_prod_dir,
					 new_db_info_ptr, NOT_EXIST);
	  }
	}
	/* if there is no write_product_ptr yet, the table file does not
	   exist yet so we must create one. */
	if (! write_product_ptr) {
	  write_product_ptr = ups_new_product();
	  write_product_ptr->instance_list = upslst_new(new_instance);
	  write_product_ptr->product = new_instance->product;
	}

	/*     write out the table file instance(s) */
	if (tmp_buf2) {
	  write_product_ptr->file = upsutl_str_create("TABLE",
						      STR_TRIM_DEFAULT);
	  (void) upsfil_write_file(write_product_ptr, tmp_buf2, ' ', JOURNAL);
	}
      	/* STEP 2: if we are echoing only, construct a declare command string
	        and echo it to the user.  if we are doing the declare,
		call the appropriate declare routine. first, fill in the
		new instance with the rest of the information. first point to
		the version instance from the matched product */
	new_dclr_instance = fill_new_dclr_instance(new_instance,
						   old_version_instance,
						   new_command_line,
						   new_prod_dir, new_ups_dir,
						   new_table_dir,
						   new_table_file,
						   new_db_info_ptr);
	if (! a_command_line->ugo_X) {
	  /*    do not execute, just echo the command line */
	  (void) sprintf(tmp_buf,
		  "ups declare %s %s -f \"%s\" -q \"%s\" -r \"%s\" -z \"%s\" ",
		  new_dclr_instance->product, new_dclr_instance->version,
		  new_dclr_instance->flavor, new_dclr_instance->qualifiers,
		  new_dclr_instance->prod_dir, new_dclr_instance->db_dir);
	  ADD_TO_ECHO(ups_dir, "%s -U \"%s\" ");
	  ADD_TO_ECHO(authorized_nodes, "%s -A \"%s\" ");
	  ADD_TO_ECHO(table_file, "%s -m \"%s\" ");
	  ADD_TO_ECHO(table_dir, "%s -M \"%s\" ");
	  ADD_TO_ECHO(archive_file, "%s -T \"%s\" ");
	  ADD_TO_ECHO(origin, "%s -D \"%s\" ");
	  ADD_TO_ECHO(compile_file, "%s -b \"%s\" ");
	  ADD_TO_ECHO(compile_dir, "%s -u \"%s\" ");
	  if (new_command_line->ugo_C) {
	    (void) sprintf(tmp_buf, "%s -C ", tmp_buf);
	  }
	  if (new_dclr_instance->statistics) {
	    (void) sprintf(tmp_buf, "%s -L ", tmp_buf);
	  }
	  if (new_command_line) {
	    for (tmp_item = new_command_line->ugo_chain ; tmp_item ;
		 tmp_item = tmp_item->next) {
	      if (! strcmp((char *)tmp_item->data, "current")) {
		(void) sprintf(tmp_buf, "%s -c ", tmp_buf);
	      } else if (! strcmp((char *)tmp_item->data, "old")) {
		(void) sprintf(tmp_buf, "%s -o ", tmp_buf);
	      } else if (! strcmp((char *)tmp_item->data, "new")) {
		(void) sprintf(tmp_buf, "%s -n ", tmp_buf);
	      } else if (! strcmp((char *)tmp_item->data, "development")) {
		(void) sprintf(tmp_buf, "%s -d ", tmp_buf);
	      } else if (! strcmp((char *)tmp_item->data, "test")) {
		(void) sprintf(tmp_buf, "%s -t ", tmp_buf);
	      } else {
		(void) sprintf(tmp_buf, "%s -g \"%s\"", tmp_buf,
			(char *)tmp_item->data);
	      }
	    }
	  }
	  (void) printf("\n%s\n\n",tmp_buf);
	} else {
	  /*    do the real declare.  to do this, we must construct a ugo
	        structure that can be passed to ups_declare. */	  
	  dclr_command_line = fill_ugo_struct(new_dclr_instance,
					      new_command_line);

	  /* now do the declare */
	  (void) ups_declare(dclr_command_line, a_temp_file, e_declare);

	  dclr_command_line = free_ugo_struct(dclr_command_line);
	}
      } else {
	/* could not get information for new instance.  punt!! */
	upserr_add(UPS_NO_NEW_INSTANCE, UPS_FATAL);
      }
    }

    /* memory clean up time still needs to be done. do not free anything that
       is a result of reading/writing a file. that file is in the cache. */
    if (new_command_line) {
      (void )upsugo_free(new_command_line);
    }
    if (new_dclr_instance) {
      new_dclr_instance = free_new_dclr_instance(new_dclr_instance);
    }
  } else {
    /* we did not get a match, so at least warn the user */
    upserr_add(UPS_NO_MATCH, UPS_WARNING,
	     (a_command_line->ugo_product ? a_command_line->ugo_product : ""));
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
 * fill_new_dclr_instance
 *
 * fill in the new_dclr_instance with info to send to declare.
 *
 * Input : a tablefile instance, a version file instance,
 *         information from the command line
 * Output: <output>
 * Return: a new instance to use for declaring
 */
static t_upstyp_instance *fill_new_dclr_instance(
				     t_upstyp_instance * const a_tinst,
				     t_upstyp_instance * const a_old_vinst,
				     t_upsugo_command * const new_command_line,
				     char * const a_prod_dir,
				     char * const a_ups_dir,
				     char * const a_table_dir,
				     char * const a_table_file,
				     t_upstyp_db * const a_db)
{
  t_upslst_item *tmp_item = NULL;
  t_upstyp_instance *ninst = ups_new_instance();

  COPY_FROM_OLD_INSTANCE(ninst, a_tinst, product);
  COPY_TO_INSTANCE(ninst, ugo_version, a_old_vinst, version);
  COPY_FROM_OLD_INSTANCE(ninst, a_tinst, flavor);
  COPY_FROM_OLD_INSTANCE(ninst, a_tinst, qualifiers);
  
  /* this is going to be a version, don't need to set chain */
  ninst->declarer = upsutl_str_create(a_tinst->modifier, ' ');
  ninst->declared = upsutl_str_create(a_tinst->modified, ' ');
  ninst->modifier = upsutl_str_create(a_tinst->modifier, ' ');
  ninst->modified = upsutl_str_create(a_tinst->modified, ' ');

  COPY_TO_INSTANCE(ninst, ugo_origin, a_old_vinst, origin);
  ninst->prod_dir = upsutl_str_create(a_prod_dir, ' ');
  ninst->ups_dir = upsutl_str_create(a_ups_dir, ' ');
  ninst->table_dir = upsutl_str_create(a_table_dir, ' ');
  ninst->table_file = upsutl_str_create(a_table_file, ' ');
  COPY_TO_INSTANCE(ninst, ugo_archivefile, a_old_vinst, archive_file);
  if (new_command_line && new_command_line->ugo_auth) {
    buf[0] = '\0';
    for (tmp_item = new_command_line->ugo_auth ; tmp_item ;
	 tmp_item = tmp_item->next) {
      if (buf[0] == '\0') {
	(void) sprintf(buf, "%s", (char *)tmp_item->data);
      } else {
	(void) sprintf(buf, "%s%s%s", buf, UPS_SEPARATOR,
		(char *)tmp_item->data);
      }
    }
    ninst->authorized_nodes = upsutl_str_create(buf, ' ');
  } else if (a_old_vinst->authorized_nodes) {
    ninst->authorized_nodes = a_old_vinst->authorized_nodes;
    upsmem_inc_refctr(ninst->authorized_nodes);
  } else {
    ninst->authorized_nodes = 0;
  }
  if (new_command_line->ugo_L) {
    ninst->statistics = upsutl_str_create(" ", ' ');
  } else {
    COPY_FROM_OLD_INSTANCE(ninst, a_old_vinst, statistics);
  }
  COPY_TO_INSTANCE(ninst, ugo_compile_file, a_old_vinst, compile_file);
  COPY_TO_INSTANCE(ninst, ugo_compile_dir, a_old_vinst, compile_dir);

  /* only copy the first db as we set this up above to only have one db
     in it */
  ninst->db_dir = a_db->name;
	
  return ninst;
}

/*-----------------------------------------------------------------------
 * free_new_dclr_instance
 *
 * free the new_dclr_instance
 *
 * Input : the instance to free
 * Output: <output>
 * Return: NULL
 */
static t_upstyp_instance *free_new_dclr_instance(
				         t_upstyp_instance * const a_instance)
{
  return ((t_upstyp_instance *)ups_free_instance(a_instance));
}
/*-----------------------------------------------------------------------
 * fill_ugo_structure
 *
 * fill in the new_command_line with info to send to declare.  we are
 * pretending that this info came from the command line even though it
 * really came from a variety of places.
 *
 * Input : an instance, information from the command line
 * Output: <output>
 * Return: matched product of the original product
 */
static t_upsugo_command *fill_ugo_struct( 
                               const t_upstyp_instance * const a_instance,
			       const t_upsugo_command * const a_command_line)
{
  t_upsugo_command *new_command_line;
  t_upstyp_db *new_db;

  new_command_line = (t_upsugo_command *)upsmem_malloc(
						     sizeof(t_upsugo_command));
  (void) memset(new_command_line, 0, sizeof(t_upsugo_command));

  new_command_line->ugo_product = a_instance->product;
  new_command_line->ugo_version = a_instance->version;
  new_command_line->ugo_f = TRUE;
  new_command_line->ugo_flavor = upslst_new(upsutl_str_create(ANY_FLAVOR,
							      ' '));
  new_command_line->ugo_flavor = upslst_insert(new_command_line->ugo_flavor,
					       a_instance->flavor);
  new_command_line->ugo_q = TRUE;
  new_command_line->ugo_qualifiers = upslst_new(a_instance->qualifiers);
  new_command_line->ugo_r = TRUE;
  new_command_line->ugo_productdir = a_instance->prod_dir;
  new_command_line->ugo_z = TRUE;
  new_db = (t_upstyp_db *)upsmem_malloc(sizeof(t_upstyp_db));
  new_db->name = a_instance->db_dir;
  new_db->config = NULL;
  new_command_line->ugo_db = upslst_new(new_db);

  if (a_command_line) {
    new_command_line->ugo_c = a_command_line->ugo_c;
    new_command_line->ugo_C = a_command_line->ugo_C;
    new_command_line->ugo_d = a_command_line->ugo_d;
    new_command_line->ugo_g = a_command_line->ugo_g;
    new_command_line->ugo_n = a_command_line->ugo_n;
    new_command_line->ugo_o = a_command_line->ugo_o;
    new_command_line->ugo_t = a_command_line->ugo_t;
    new_command_line->ugo_u = a_command_line->ugo_u;
    new_command_line->ugo_v = a_command_line->ugo_v;
    new_command_line->ugo_V = a_command_line->ugo_V;
    new_command_line->ugo_Z = a_command_line->ugo_Z;
    
    new_command_line->ugo_chain = a_command_line->ugo_chain;
    new_command_line->ugo_shell = a_command_line->ugo_shell;
    new_command_line->ugo_number = a_command_line->ugo_number;
  }
  SET_DCLR_LINE_LIST(ugo_auth, authorized_nodes, ugo_A);
  SET_DCLR_LINE(ugo_compile_file, compile_file, ugo_b);
  SET_DCLR_LINE(ugo_compile_dir, compile_dir, ugo_u);
  SET_DCLR_LINE(ugo_origin, origin, ugo_D);
  SET_DCLR_LINE(ugo_tablefile, table_file, ugo_m);
  SET_DCLR_LINE(ugo_tablefiledir, table_dir, ugo_M);
  SET_DCLR_LINE(ugo_archivefile, archive_file, ugo_T);
  SET_DCLR_LINE(ugo_upsdir, ups_dir, ugo_U);
  
  return(new_command_line);
}
/*-----------------------------------------------------------------------
 * free_ugo_structure
 *
 * free the ugo structure we used for the declare.  we only malloced
 * pieces of it which is why we cannot go thru the normal channels to do
 * this.
 *
 * Input : a ugo structure
 * Output: <output>
 * Return: NULL
 */
static t_upsugo_command *free_ugo_struct( 
			       t_upsugo_command * const a_command_line)
{
  a_command_line->ugo_flavor = upslst_free(a_command_line->ugo_flavor, ' ');
  a_command_line->ugo_qualifiers = upslst_free(a_command_line->ugo_qualifiers,
					       ' ');
  if (a_command_line->ugo_auth) {
    a_command_line->ugo_auth = upslst_free(a_command_line->ugo_auth, 'd');
  }
  upsmem_free(a_command_line->ugo_db->data);
  a_command_line->ugo_db = upslst_free(a_command_line->ugo_db, ' ');
  upsmem_free(a_command_line);
  
  return(NULL);
}


