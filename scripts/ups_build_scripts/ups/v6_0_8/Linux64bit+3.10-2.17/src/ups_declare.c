/************************************************************************
 *
 * FILE:
 *       ups_declare.c
 * 
 * DESCRIPTION: 
 *       This is the 'ups declare' command.
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
 *       Mon Dec 15, DjF, Starting...
 *
 ***********************************************************************/

/* standard include files */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ups specific include files */
#include "ups.h"

/*
 * Definition of public variables.
 */
extern t_cmd_info g_cmd_info[];

/*
 * Declaration of private functions.
 */

/*
 * Definition of global variables.
 */

#ifndef NULL
#define NULL 0
#endif

#define CHAIN "chain"
#define VERSION "version"
#define DECLARE "declare"
#define UPS_DECLARE "DECLARE: "


static char buf[MAX_LINE_LEN];
static char *the_flavor;
static char *the_qualifiers;
static char *the_chain=0;
static char *save_version;
static char *save_table_dir;		/* match won't work "how I want" */
static char *save_table_file;		/* with table specifications     */
static char *save_ups_dir;
static char *save_prod_dir;
static char *username=0;
static char *declared_date=0;


t_upstyp_product *upsdcl_new_product(t_upsugo_command * const uc)
{
  t_upstyp_product *product;
  product = ups_new_product();
  product->file = upsutl_str_create( CHAIN, ' ' );
  product->product=upsutl_str_create( uc->ugo_product, ' ' );
  product->chain = upsutl_str_create( the_chain, ' ' );
  product->version = upsutl_str_create( save_version, ' ' );
  return product;
} 
 
t_upstyp_instance *upsdcl_new_chain(t_upsugo_command * const uc)
{
t_upstyp_instance *cinst;
   cinst=ups_new_instance();
   cinst->product=upsutl_str_create(uc->ugo_product, ' ' );
   cinst->version=upsutl_str_create(save_version,' ');
   cinst->flavor=upsutl_str_create(the_flavor,' ');
   cinst->qualifiers=upsutl_str_create(the_qualifiers,' ');
   cinst->declarer=upsutl_str_create(username,' ');
   cinst->declared=upsutl_str_create(declared_date,' ');
   cinst->modifier=upsutl_str_create(username,' ');
   cinst->modified=upsutl_str_create(declared_date,' ');
   return cinst;
}

t_upstyp_instance *upsdcl_new_version(t_upsugo_command * const uc,
                                      t_upstyp_db * const db_info)
{
   t_upstyp_instance *vinst;
   t_upslst_item *auth_list=0;
   char *allauthnodes=0;
   char *hold_env; /* temporary holding place for translated env's */
   char *tmp_ptr;
   int count=0;
   vinst=ups_new_instance();
   vinst->product=upsutl_str_create(uc->ugo_product,' ');
   vinst->version=upsutl_str_create(save_version,' ');
   vinst->flavor=upsutl_str_create(the_flavor,' ');
   vinst->qualifiers=upsutl_str_create(the_qualifiers,' ');
   vinst->declarer=upsutl_str_create(username,' ');
   vinst->declared=upsutl_str_create(declared_date,' ');
   vinst->modifier=upsutl_str_create(username,' ');
   vinst->modified=upsutl_str_create(declared_date,' ');
   vinst->prod_dir=upsutl_str_create(save_prod_dir,' ');
   vinst->table_dir=upsutl_str_create(save_table_dir,' ');
   vinst->table_file=upsutl_str_create(save_table_file,' ');
   vinst->ups_dir=upsutl_str_create(save_ups_dir,' ');
   vinst->origin=upsutl_str_create(uc->ugo_origin,' ');
   vinst->compile_file=upsutl_str_create(uc->ugo_compile_file,' ');
   vinst->compile_dir=upsutl_str_create(uc->ugo_compile_dir,' ');
   vinst->archive_file=upsutl_str_create(uc->ugo_archivefile,' ');
   if (uc->ugo_A) { allauthnodes=upsutl_str_create("",' '); }
   for ( auth_list = upslst_first(uc->ugo_auth ); auth_list; 
         auth_list = auth_list->next, count++ )
   { allauthnodes=upsutl_str_crecat(allauthnodes,auth_list->data);
     if (auth_list->next != 0) 
     { allauthnodes=upsutl_str_crecat(allauthnodes,":");
     }
   }
   vinst->authorized_nodes=allauthnodes;
   /* If no ups_dir specified on command line add ups if it exists */
   if ((! vinst->ups_dir) && vinst->prod_dir)
   { if (!(tmp_ptr = upsget_translation_env(vinst->prod_dir)))
     { tmp_ptr = vinst->prod_dir; }
     if (db_info && db_info->config && 
         db_info->config->prod_dir_prefix && UPSRELATIVE(tmp_ptr))
     { (void) sprintf(buf,"%s/%s/ups", db_info->config->prod_dir_prefix, tmp_ptr);
     } else {
       (void) sprintf(buf,"%s/ups", tmp_ptr);
     }
     if (upsutl_is_a_file(buf) == UPS_SUCCESS)
     { vinst->ups_dir=upsutl_str_create("ups",' ');
       /* Make command think it had it too or upscpy will not find */
       /* uc->ugo_upsdir=upsutl_str_create("ups",' '); */
     }
   }
/* I'm going to create the save instance and just put everything in 
   there as the first fix for this */
   vinst->sav_inst = ups_new_instance();
   vinst->sav_inst->prod_dir=upsutl_str_create(vinst->prod_dir,' ');
   if((hold_env=upsget_translation_env(vinst->prod_dir))!=0)
   { vinst->prod_dir=upsutl_str_create(hold_env,' '); }
   if((hold_env=upsget_translation_env(vinst->compile_dir))!=0)
   { vinst->compile_dir=upsutl_str_create(hold_env,' '); }
   if((hold_env=upsget_translation_env(vinst->ups_dir))!=0)
   { vinst->ups_dir=upsutl_str_create(hold_env,' '); }
   if((hold_env=upsget_translation_env(vinst->table_dir))!=0)
   { vinst->table_dir=upsutl_str_create(hold_env,' '); }
   return vinst;
}
/*
 * Definition of public functions.
 */


/*-----------------------------------------------------------------------
 * ups_declare
 *
 * This is the main line for the 'ups declare' command.
 *
 * Input : argc, argv
 * Output: 
 * Return: 
 */
t_upslst_item *ups_declare( t_upsugo_command * const uc ,
			    const FILE * const tmpfile, const int ups_command)
{
  t_upslst_item *mproduct_list = NULL;
  t_upslst_item *minst_list = NULL;
  t_upslst_item *chain_list = NULL;
  t_upslst_item *cmd_list = NULL;
  int chain=0; /* was a chain specified, trick on on info messages */
  int version=0; /* version was defined */
  t_upstyp_db *db_info = 0;
  t_upslst_item *db_list = 0;
  t_upstyp_matched_product *mproduct = NULL;
  t_upstyp_matched_instance *minst = NULL;
  int not_unique = 0;
  int need_unique = 1;
  t_upstyp_product *product = 0;
  char buffer[FILENAME_MAX+1];
  char *file=buffer;
  char *saddr;				/* start address for -O manipulation */
  char *eaddr;				/* end address for -O manipulation */
  char *naddr;				/* new address for -O manipulation */
  t_upslst_item *cinst_list;                /* chain instance list */
  t_upstyp_instance *cinst;                 /* chain instance      */
  t_upstyp_instance *new_cinst;             /* new chain instance  */
  t_upstyp_instance *tinst;                 /*   table instance      */
  t_upstyp_instance *new_vinst=0;           /* new version instance  */
  char *unchain;
  t_upslst_item *save_next;
  t_upslst_item *save_prev;
  t_upslst_item *save_flavor;
  t_upslst_item *save_qualifiers;
  t_upslst_item *save_chain;
  int save_m=uc->ugo_m;
  int save_M=uc->ugo_M;
  int save_U=uc->ugo_U;
  int save_r=uc->ugo_r;
  int len;
  the_flavor=0;
  the_qualifiers=0;
  the_chain=0;
  save_version=0;
  uc->ugo_m=0;
  uc->ugo_M=0;
  uc->ugo_U=0;
  uc->ugo_r=0;
  save_table_dir=uc->ugo_tablefiledir;
  save_table_file=uc->ugo_tablefile;
  save_ups_dir=uc->ugo_upsdir;
  save_prod_dir=uc->ugo_productdir;
  uc->ugo_tablefile=0;
  uc->ugo_tablefiledir=0;
  uc->ugo_upsdir=0; 
  uc->ugo_productdir=0; 
  save_flavor=uc->ugo_flavor;
  save_qualifiers=uc->ugo_qualifiers;
  save_version=uc->ugo_version;

  if (!uc->ugo_db) /* -m masks error in upsugo */
  { upserr_add(UPS_NO_DATABASE, UPS_FATAL);
    return 0;
  }
  if (!uc->ugo_product || !uc->ugo_version )
  { upserr_add(UPS_INVALID_SPECIFICATION, UPS_FATAL, "Declare", 
               "Specification must include a product and a version");
    return 0;
  }
  save_chain=uc->ugo_chain;
  if ((int)(upslst_count(uc->ugo_flavor) != 2) ) /* remember any */
  { if(!uc->ugo_chain) /* not possibly just defining a chain */
    { upserr_add(UPS_INVALID_SPECIFICATION, UPS_FATAL, "Declare", 
                 "Specification must include a single flavor");
      return 0;
    } else {  /* if just defining a chain it be there */
      uc->ugo_chain=0;
      mproduct_list = upsmat_instance(uc, db_list , need_unique);
      if (!mproduct_list)
      { upserr_add(UPS_INVALID_SPECIFICATION, UPS_FATAL, "Declare", 
                   "Specification must include a single flavor");
        return 0;
      } else { /* get the flavor for chain */
        mproduct_list = upslst_first(mproduct_list);
        mproduct = (t_upstyp_matched_product *)mproduct_list->data;
        minst_list = (t_upslst_item *)mproduct->minst_list;
        minst = (t_upstyp_matched_instance *)(minst_list->data);
        the_flavor=minst->version->flavor;
      }
    }
  }
  uc->ugo_chain=save_chain;
/* if they are defining a version ONLY and it allready exists fail */
  if ( !uc->ugo_chain && uc->ugo_version) 
  if (mproduct_list)
  { upserr_add(UPS_INVALID_SPECIFICATION, UPS_FATAL, "Declare", 
               "Exact product definition exists");
    return 0;
  }
  if(!username) /* static may allready be set */
  { username=upsutl_str_create(upsutl_user(), STR_TRIM_DEFAULT);
    declared_date = upsutl_str_create(upsutl_time_date(STR_TRIM_DEFAULT),
                                      STR_TRIM_DEFAULT);
  }

/************************************************************************
 *
 * Find the right database to work with
 *
 * Check for product in ANY database if so use that database 
 * otherwise set to first database listed
 *
 ***********************************************************************/

 uc->ugo_chain = upslst_new(upsutl_str_create(ANY_MATCH,' '));
 uc->ugo_version=0;
 uc->ugo_flavor = upslst_new(upsutl_str_create(ANY_MATCH,' '));
 uc->ugo_qualifiers = upslst_new(upsutl_str_create(ANY_MATCH,' '));
 if (!uc->ugo_z)         /* didn't specify a database(s) specifically */
 { for (db_list = uc->ugo_db ; db_list ; db_list=db_list->next) 
   { db_info = (t_upstyp_db *)db_list->data;
     mproduct_list = upsmat_instance(uc, db_list , not_unique);
     if (UPS_ERROR != UPS_SUCCESS) 
     { return 0; 
     } 
     if (mproduct_list)    /* the product does exist */ 
     { upsver_mes(1,"%sProduct %s currently exists in database %s\n",
                  UPS_DECLARE,
                  uc->ugo_product,
                  db_info->name);
       break; 
     } db_info=0;
   } 
 }
  if (!db_info) 
  { db_list = upslst_first(uc->ugo_db);
    db_info = (t_upstyp_db *)db_list->data;
  } 
  /* need to read dbconfig here! */

   if (db_info && !db_info->config ) {
      t_upstyp_product *p;
      p =  upsutl_get_config(db_info->name);
      if (p && p->config) {
        db_info->config = p->config;
      }
   } 

   /* if the prodcut directory matches the product prefix for this
    * database, trim it off
    */

   if (db_info->config && db_info->config->prod_dir_prefix && save_prod_dir ) {

       len = strlen(db_info->config->prod_dir_prefix);
       if (db_info->config->prod_dir_prefix[len-1] == '/') {
          len = len - 1;
       }
       if (0 == strncmp(save_prod_dir, db_info->config->prod_dir_prefix, len) && save_prod_dir[len] == '/') {
           /* move past prefix and slash */
           save_prod_dir += len + 1;
       }
   }

   if (db_info && db_info->config && 
      db_info->config->version_subdir &&  
      db_info->config->version_subdir[0] == '1') {

       extern int g_subdir_files_flag;
       g_subdir_files_flag = 1;
       
   } else {
       extern int g_subdir_files_flag;
       g_subdir_files_flag = 0;
   }
/* restore everything */
/*  uc->ugo_version=save_version; */
  uc->ugo_flavor=upslst_free(uc->ugo_flavor,'d');
  uc->ugo_flavor=save_flavor;
  uc->ugo_qualifiers=upslst_free(uc->ugo_qualifiers,'d');
  uc->ugo_qualifiers=save_qualifiers;
  uc->ugo_chain=upslst_free(uc->ugo_chain,'d');
  uc->ugo_chain=save_chain;
/************************************************************************
 *
 * If there was any chain specified at all we need to look at chain files
 *
 ***********************************************************************/

     if (uc->ugo_chain)
     { chain=1;
       for (chain_list = uc->ugo_chain ; chain_list ;
         chain_list = chain_list->next) 
       { the_chain = (char *)(chain_list->data);
         (void) sprintf(buffer,"%s/%s/%s%s",
                 db_info->name,
                 uc->ugo_product,
                 the_chain,CHAIN_SUFFIX);
         if (upsfil_exist(buffer))        /* does chain file exist at all */
         { upsver_mes(1,"%sChain %s currently exists\n",UPS_DECLARE,the_chain);
           uc->ugo_flavor=save_flavor;
           uc->ugo_qualifiers=save_qualifiers;
           uc->ugo_chain=chain_list;
           save_next = chain_list->next;
           save_prev = chain_list->prev;
           chain_list->next=0;
           chain_list->prev=0;
           mproduct_list = upsmat_instance(uc, db_list , need_unique);
           chain_list->next = save_next;
           chain_list->prev = save_prev;
           if (UPS_ERROR != UPS_SUCCESS) 
           { (void) upsfil_clear_journal_files();
             upserr_vplace();
             return 0; 
           }
           if (mproduct_list)  /* different version only */
           { upsver_mes(1,"%ssame flavor and qualifiers exist\n",UPS_DECLARE);
             mproduct_list = upslst_first(mproduct_list);
             mproduct = (t_upstyp_matched_product *)mproduct_list->data;
             minst_list = (t_upslst_item *)mproduct->minst_list;
             minst = (t_upstyp_matched_instance *)(minst_list->data);
             cinst = (t_upstyp_instance *)minst->chain;
             /* there's a better flavor match */
             if(!the_flavor)
             { the_flavor=cinst->flavor; }
             if(!strcmp(cinst->flavor,the_flavor)) 
             { the_flavor=cinst->flavor;
               product = upsget_chain_file(db_info->name,
                                           uc->ugo_product,
                                           the_chain, &file);
               if ((UPS_ERROR == UPS_SUCCESS) && product )
               { cinst_list=upsmat_match_with_instance( cinst, product );
                 cinst=cinst_list->data;
                 product->instance_list = 
                    upslst_delete(product->instance_list,cinst,'d');
                 upsver_mes(1,"%sDeleting %s chain of version %s\n",
                               UPS_DECLARE,
                               the_chain,
                               cinst->version);
                 (void) upsfil_write_file(product, buffer,' ',JOURNAL);
                 if (UPS_ERROR != UPS_SUCCESS)
                 {
                   (void) upsfil_clear_journal_files();
                   upserr_vplace();
                   return 0;
                 }
                 unchain = (char *) malloc((size_t)(strlen(the_chain)+3));
                 (void) sprintf(unchain,"un%s",the_chain);
                 if (!uc->ugo_C)
                 {
                   cmd_list = upsact_get_cmd((t_upsugo_command *)uc,
                                              mproduct, unchain,ups_command);
                   if (UPS_ERROR == UPS_SUCCESS) 
                   { upsact_process_commands(cmd_list, tmpfile); 
                     upsact_cleanup(cmd_list);
                   } else {
                     (void) upsfil_clear_journal_files();
                     upserr_vplace();
                     return 0;
                   }
                 }
                 else
                 {
                   upsver_mes(1,"%sSkipping %s of %s version %s due to -C option\n",
                                 UPS_DECLARE, unchain, uc->ugo_product, cinst->version);
                 }
                 free (unchain);
               }
             } else { 
               /* this is a better match */
               /* i.e. was a irix flavor but a null chain match */
               product = upsget_chain_file(db_info->name,
                                           uc->ugo_product,
                                           the_chain, &file);
             }
           } /* Get chain file (maybe again) */
           if (upsfil_exist(file))
           { product = upsget_chain_file(db_info->name,
                                       uc->ugo_product,
                                       the_chain, &file);
           } else { 
             product=upsdcl_new_product(uc);
           }
         } else { /* new chain does NOT exist at all */
           product=upsdcl_new_product(uc);
         }
         if (!product)
           product=upsdcl_new_product(uc);
         /* build new chain instance */
         if (!the_flavor) { the_flavor=save_flavor->data; }
         the_qualifiers=save_qualifiers->data;
         new_cinst=upsdcl_new_chain(uc);
         product->instance_list = 
             upslst_add(product->instance_list,new_cinst);
         upsver_mes(1,"%sAdding %s chain version %s to %s\n",
                    UPS_DECLARE, the_chain, new_cinst->version, file);
         (void) upsfil_write_file(product, file,' ',JOURNAL);  
         if (UPS_ERROR != UPS_SUCCESS)
         {
           (void) upsfil_clear_journal_files();
           upserr_vplace();
           return 0;
         }
        }
      }
/************************************************************************
 *
 * Chains have been complete on the version file...
 *
 ***********************************************************************/
/* We want NOTHING to do with chains at this point - it is out of sync */
    uc->ugo_chain=0;
    uc->ugo_version=save_version;  /* we must match of version now */
    uc->ugo_flavor = upslst_new(upsutl_str_create(ANY_MATCH,' '));
    uc->ugo_qualifiers = upslst_new(upsutl_str_create(ANY_MATCH,' '));
    mproduct_list = upsmat_instance(uc, db_list , not_unique);
    if (UPS_ERROR != UPS_SUCCESS) 
    { (void) upsfil_clear_journal_files();
      upserr_vplace();
      return 0; 
    }
    if (mproduct_list) 
    {  uc->ugo_flavor=save_flavor;
       uc->ugo_qualifiers=save_qualifiers;
       mproduct_list = upsmat_instance(uc, db_list , need_unique);
       if (UPS_ERROR != UPS_SUCCESS) 
       { (void) upsfil_clear_journal_files();
         upserr_vplace();
         return 0; 
       }
       if (!mproduct_list) 
       { upsver_mes(1,"%sVersion exists, adding additional instance\n",
                    UPS_DECLARE);
         product = upsget_version_file(db_info->name,
                                       uc->ugo_product,
                                       uc->ugo_version,
                                       &file);
         (void) strcpy(buffer,file); /* hum */
       } else { 
         if (!save_chain) /* declaring the same this over and not chains */
         { upserr_add(UPS_INVALID_SPECIFICATION, UPS_FATAL, "Declare", 
                      "Specified product and version currently exist");
           return 0;
         }
         upsver_mes(chain,
           "%sINFORMATIONAL: Instance in version file already exists\n",
           UPS_DECLARE);
         buffer[0]=0; /* don't create instance */
         mproduct_list = upslst_first(mproduct_list);
         mproduct = (t_upstyp_matched_product *)mproduct_list->data;
         minst_list = (t_upslst_item *)mproduct->minst_list;
         minst = (t_upstyp_matched_instance *)(minst_list->data);
       }
    } else { /* new version does NOT exist at all */
      product = ups_new_product();
      (void) sprintf(buffer,"%s/%s/%s%s",
              db_info->name,
              uc->ugo_product,
              uc->ugo_version,VERSION_SUFFIX);
      product->file = upsutl_str_create(VERSION,' ');
      product->product=upsutl_str_create(uc->ugo_product,' ');
      product->version = upsutl_str_create(uc->ugo_version,' ');
    }
/* no more matching replace "faked out stuff" */
    uc->ugo_tablefiledir=save_table_dir;
    uc->ugo_tablefile=save_table_file;
    uc->ugo_upsdir=save_ups_dir;
    uc->ugo_productdir=save_prod_dir;
    uc->ugo_m=save_m;
    uc->ugo_M=save_M;
    uc->ugo_U=save_U;
    uc->ugo_r=save_r;
    /* build new version instance */
    if (buffer[0]!=0) /* instance doesn't exist */
    { the_flavor=save_flavor->data;
      the_qualifiers=save_qualifiers->data;
      new_vinst=upsdcl_new_version(uc,db_info);
/* If I'm creating a totally matched version I have to create the matched 
   product structure by hand since it really doesn't exist yet on disk
   and a call to get it will fail
*/
      minst = ups_new_matched_instance();
      minst->version=new_vinst;
      tinst = upsmat_version(new_vinst,db_info);
      minst->table=tinst;
      minst_list = upslst_add(minst_list,minst);
      mproduct = ups_new_matched_product(db_info, uc->ugo_product,
                                         minst_list);
      if (!uc->ugo_r )
      { upserr_add(UPS_NO_INSTANCE, UPS_INFORMATIONAL, 
               uc->ugo_product, (char *)uc->ugo_qualifiers->data,
	       "product home", 
               "\nSpecification did not include a -r for product directory",
	       " ");
      }
      if (!uc->ugo_m )
      { upserr_add(UPS_NO_INSTANCE, UPS_INFORMATIONAL, 
               uc->ugo_product, (char *)uc->ugo_qualifiers->data,
	       "table file", 
               "\nSpecification did not include a -m for table file only default actions will be performed",
	       " ");
      }
      if (uc->ugo_O)
      { if ( strchr(uc->ugo_options,':') == 0)
        { 
	  if ( strchr(uc->ugo_options,'=') )
	  { new_vinst->user_list = 
             upslst_add(new_vinst->user_list,upsutl_str_create(uc->ugo_options,' '));
	  }
        } else {
          saddr=uc->ugo_options;
          while ((eaddr = strchr(saddr,':')) != 0 )
          { *eaddr='\0';
            if ( strchr(uc->ugo_options,'=') )
	    { naddr=upsutl_str_create(saddr,' ');
              new_vinst->user_list = 
                upslst_add(new_vinst->user_list,naddr);
	    }
            *eaddr=':'; /* put back : want to free whole list later */
            saddr=eaddr+1;
          }
          naddr=upsutl_str_create(saddr,' ');
          new_vinst->user_list = 
             upslst_add(new_vinst->user_list,naddr);
        }
      }
      if (uc->ugo_L)
      { new_vinst->statistics=upsutl_str_create("",' '); 
      }
      product->instance_list = 
          upslst_add(product->instance_list,new_vinst);
      upsver_mes(1,"%sAdding version %s to %s\n",
                 UPS_DECLARE,
                 new_vinst->version,
                 buffer);
      (void) upsfil_write_file(product, buffer, ' ', JOURNAL);  
      if (UPS_ERROR != UPS_SUCCESS)
      {
        (void) upsfil_clear_journal_files();
        upserr_vplace();
        return 0;
      }
      version=1;
/* Process the declare action */
      cmd_list = upsact_get_cmd((t_upsugo_command *)uc,
                                 mproduct, g_cmd_info[ups_command].cmd,
                                 ups_command);
      if (UPS_ERROR == UPS_SUCCESS) 
      { upsact_process_commands(cmd_list, tmpfile); 
        upsact_cleanup(cmd_list);
      } else {
        (void) upsfil_clear_journal_files();
        upserr_vplace();
        return 0;
      } 
    } 
    if (version) /* only do configure if version was actually defined */
    {
      if (!uc->ugo_C)
      {
                      /* Do configure actions                  */
        cmd_list = upsact_get_cmd((t_upsugo_command *)uc,
                                   mproduct, g_cmd_info[e_configure].cmd,
                                   ups_command);
        if (UPS_ERROR == UPS_SUCCESS) 
        { upsact_process_commands(cmd_list, tmpfile); 
          upsact_cleanup(cmd_list);
        } else {
          (void) upsfil_clear_journal_files();
          upserr_vplace();
          return 0;
        }
      }
      else
      {
        upsver_mes(1,"%sSkipping %s of %s version %s due to -C option\n",
                      UPS_DECLARE, g_cmd_info[e_configure].cmd, uc->ugo_product, new_vinst->version);
      }

/* Let them know if there is a tailor action for this product */
      if (!uc->ugo_C)
      {
        cmd_list = upsact_get_cmd((t_upsugo_command *)uc,
                                   mproduct, g_cmd_info[e_tailor].cmd,
                                   ups_command);
        if (UPS_ERROR == UPS_SUCCESS) 
        { if (cmd_list)
          { upsver_mes(0,"%sA UPS tailor exists for this product\n",
	          UPS_DECLARE);
            upsact_cleanup(cmd_list);
          }
        } 
      }
    }
    uc->ugo_chain=save_chain;
    if (uc->ugo_chain)
    { uc->ugo_flavor=save_flavor;
      uc->ugo_qualifiers=save_qualifiers;
      uc->ugo_version=save_version;
      for (chain_list = uc->ugo_chain ; chain_list ;
           chain_list = chain_list->next)
      { the_chain = (char *)(chain_list->data);
        save_next = chain_list->next;
        save_prev = chain_list->prev;
        chain_list->next=0;
        chain_list->prev=0;
        uc->ugo_chain=chain_list;
        if (!uc->ugo_C)
        {
          cmd_list = upsact_get_cmd((t_upsugo_command *)uc,
                                    mproduct, the_chain,
                                    ups_command);
          if (UPS_ERROR == UPS_SUCCESS) 
          { upsact_process_commands(cmd_list, tmpfile);
            upsact_cleanup(cmd_list);
          } else {
            (void) upsfil_clear_journal_files();
            upserr_vplace();
            return 0;
          } 
        } 
        else
        {
          upsver_mes(1,"%sSkipping %s of %s version %s due to -C option\n",
                        UPS_DECLARE, the_chain, uc->ugo_product, uc->ugo_version);
        }
        if (!uc->ugo_C)
        {
          if (strstr(the_chain,"current"))
          { cmd_list = upsact_get_cmd((t_upsugo_command *)uc,
                                       mproduct, g_cmd_info[e_start].cmd,
                                       ups_command);
            if (!cmd_list)
            { cmd_list = upsact_get_cmd((t_upsugo_command *)uc,
                                         mproduct, g_cmd_info[e_stop].cmd,
                                         ups_command);
            }
            /*          if (UPS_ERROR == UPS_SUCCESS)  */
            if (cmd_list)
            { upsver_mes(0,"%sA UPS start/stop exists for this product\n",
                 UPS_DECLARE);
              upsact_cleanup(cmd_list);
            } 
            upscpy_man(minst,db_info,uc,tmpfile);
            upscpy_catman(minst,db_info,uc,tmpfile);
            upscpy_info(minst,db_info,uc,tmpfile);
          }
          chain_list->next = save_next;
          chain_list->prev = save_prev;
        }
      }
    }
    return 0;
}
