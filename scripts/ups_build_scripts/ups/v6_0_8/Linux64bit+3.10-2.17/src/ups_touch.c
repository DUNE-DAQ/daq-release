/************************************************************************
 *
 * FILE:
 *       ups_touch.c
 * 
 * DESCRIPTION: 
 *       This is the 'ups touch' command.
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
 *       Tue Mar 17 1998, DjF, Starting...
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

static void shutup(const FILE * const tmpfile, const int ups_command);

#define SHUTUP \
  if ((&bit_bucket + bit_bucket_offset == 0) && 0) shutup (tmpfile, ups_command);

/*
 * Definition of global variables.
 */

static long bit_bucket = 0;
static long bit_bucket_offset = 0;

#ifndef NULL
#define NULL 0
#endif

#define CHAIN "chain"
#define VERSION "version"
#define DECLARE "declare"
#define VPREFIX "UPS TOUCH: "

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * ups_touch
 *
 * This is the main line for the 'ups touch' command.
 *
 * Input : 
 * Output: 
 * Return: 
 */
t_upslst_item *ups_touch( t_upsugo_command * const uc,
			  const FILE * const tmpfile, 
                          const int ups_command)
{
  t_upslst_item *mproduct_list = NULL;
  t_upslst_item *minst_list = NULL;
  t_upslst_item *chain_list = NULL;
  t_upstyp_db *db_info = 0;
  t_upslst_item *db_list = 0;
  t_upstyp_matched_product *mproduct = NULL;
  t_upstyp_matched_instance *minst = NULL;
  int not_unique = 0;
  int need_unique = 1;
  t_upslst_item *save_flavor;
  t_upslst_item *save_qualifiers;
  t_upslst_item *save_chain;
  char * save_version;
  t_upstyp_product *product;
  static char buffer[FILENAME_MAX+1];
  static char *file=buffer;
  char *the_chain;
  t_upslst_item *cinst_list;                /* chain instance list */
  t_upstyp_instance *cinst;                 /* chain instance      */
  t_upslst_item *vinst_list;                /* version instance list */
  t_upstyp_instance *vinst;                 /* version instance      */
  char *username;
  char *declared_date;
  t_upslst_item *save_next;
  t_upslst_item *save_prev;
  save_chain=uc->ugo_chain;
  save_flavor=uc->ugo_flavor;
  save_qualifiers=uc->ugo_qualifiers;
  save_version=uc->ugo_version;

  if (!uc->ugo_product)
  { upserr_add(UPS_INVALID_SPECIFICATION, UPS_FATAL, "Touch", 
               "Specification must include a product");
    return 0;
  }
  if ((int)(upslst_count(uc->ugo_flavor) == 0) )
  { upserr_add(UPS_INVALID_SPECIFICATION, UPS_FATAL, "Declare", 
               "Specification must include a flavor");
    return 0;
  }
  mproduct_list = upsmat_instance(uc, db_list , not_unique);
  if (UPS_ERROR != UPS_SUCCESS) 
  {  return 0; 
  }
  if (!mproduct_list)
  { upserr_add(UPS_INVALID_SPECIFICATION, UPS_FATAL, "Touch", 
               "Product specified doesn't exist");
    return 0;
  }
  username=upsutl_str_create(upsutl_user(), STR_TRIM_DEFAULT);
  declared_date = upsutl_str_create(upsutl_time_date(STR_TRIM_DEFAULT),
				    STR_TRIM_DEFAULT);

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
 for (db_list = uc->ugo_db ; db_list ; db_list=db_list->next) 
 { db_info = (t_upstyp_db *)db_list->data;
   mproduct_list = upsmat_instance(uc, db_list , not_unique);
   if (UPS_ERROR != UPS_SUCCESS) 
   { return 0; 
   }
   /* if (UPS_ERROR != UPS_SUCCESS) { upserr_clear(); } */
   if (mproduct_list)    /* the product does exist */ 
   { upsver_mes(1,"%sProduct %s currently exist in database %s\n",
	        VPREFIX,
                uc->ugo_product,
                db_info->name);
     break; 
   } db_info=0;
  } 
  if (!db_info) 
  { db_list = upslst_first(uc->ugo_db);
    db_info = (t_upstyp_db *)db_list->data;
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
  uc->ugo_chain=upslst_free(uc->ugo_chain,'d');
  uc->ugo_chain=save_chain;
  uc->ugo_version=save_version;
  uc->ugo_flavor=upslst_free(uc->ugo_flavor,'d');
  uc->ugo_flavor=save_flavor;
  uc->ugo_qualifiers=upslst_free(uc->ugo_qualifiers,'d');
  uc->ugo_qualifiers=save_qualifiers;
/************************************************************************
 *
 * If there was any chain specified at all we need to look at chain files
 *
 ***********************************************************************/

     if (uc->ugo_chain)
     { for (chain_list = uc->ugo_chain ; chain_list ;
         chain_list = chain_list->next) 
       { the_chain = (char *)(chain_list->data);
           save_next = chain_list->next;
           save_prev = chain_list->prev;
           chain_list->next=0;
           chain_list->prev=0;
           uc->ugo_chain=chain_list;
         mproduct_list = upsmat_instance(uc, db_list , need_unique);
         if (UPS_ERROR != UPS_SUCCESS) 
         { (void) upsfil_clear_journal_files();
           upserr_vplace();
           return 0; 
         }
         upsver_mes(1,"%sChain %s currently exist\n",VPREFIX,the_chain);
         chain_list->next = save_next;
         chain_list->prev = save_prev;
         mproduct_list = upslst_first(mproduct_list);
         mproduct = (t_upstyp_matched_product *)mproduct_list->data;
         minst_list = (t_upslst_item *)mproduct->minst_list;
         minst = (t_upstyp_matched_instance *)(minst_list->data);
         cinst = (t_upstyp_instance *)minst->chain;
         product = upsget_chain_file(db_info->name,
                                     uc->ugo_product,
                                     the_chain, &file);
         (void) strcpy(buffer,file);
         if ((UPS_ERROR == UPS_SUCCESS) && product )
         { cinst_list=upsmat_match_with_instance( cinst, product );
           cinst=cinst_list->data;
           cinst->modifier=username;
           cinst->modified=declared_date;
           (void )upsfil_write_file(product, buffer,' ',JOURNAL);
         } else {
           if (UPS_ERROR != UPS_SUCCESS) /* just an error */
           { (void) upsfil_clear_journal_files();
             upserr_vplace();
             return 0; 
           }
         }
       }
       return(0); /* this is sucess no clear doing just chains */
     }
     mproduct_list = upsmat_instance(uc, db_list , need_unique);
     if (UPS_ERROR != UPS_SUCCESS) {  return 0; }
     upsver_mes(1,"%sVersion exists\n",VPREFIX);
     mproduct_list = upslst_first(mproduct_list);
     mproduct = (t_upstyp_matched_product *)mproduct_list->data;
     minst_list = (t_upslst_item *)mproduct->minst_list;
     minst = (t_upstyp_matched_instance *)(minst_list->data);
     vinst = (t_upstyp_instance *)minst->version;
     product = upsget_version_file(db_info->name,
                                   uc->ugo_product,
                                   uc->ugo_version,
                                   &file);
     (void) strcpy(buffer,file);
     if ((UPS_ERROR == UPS_SUCCESS) && product )
     { vinst_list=upsmat_match_with_instance( vinst, product );
       vinst=vinst_list->data;
       vinst->modifier=username;
       vinst->modified=declared_date;
       (void )upsfil_write_file(product, buffer,' ',JOURNAL);
     } else { 
       if (UPS_ERROR != UPS_SUCCESS) /* just an error */
       { (void) upsfil_clear_journal_files();
         upserr_vplace();
         return 0; 
       }
     }

    SHUTUP;

    return 0;
}

static void shutup(const FILE * const tmpfile, const int ups_command)
{
      bit_bucket ^= (long) tmpfile;
      bit_bucket ^= (long) ups_command;
}
