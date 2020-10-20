/************************************************************************
 *
 * FILE:
 *       ups_list.c
 * 
 * DESCRIPTION: 
 *       This is the 'ups list' command.
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
 *       14-Aug-1997, EB, first
 *       Sep 19 1997, DjF, Taking over...
 *
 ***********************************************************************/

/* standard include files */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>   /* toupper */

/* ups specific include files */
#include "ups.h"

/*
 * Definition of public variables.
 */
extern t_cmd_info g_cmd_info[];
/*
 * Declaration of private functions.
 */

t_upslst_item *ups_list_core(t_upsugo_command * const a_command_line ,
                             t_upslst_item * const db_list );
void list_output(const t_upslst_item * const a_mproduct_list,
		 const t_upsugo_command * const a_command_line);

void list_K(const t_upstyp_matched_instance * const instance,
            const t_upsugo_command * const a_command_line, 
            const t_upstyp_matched_product * const product,
	    const int match_done );
void print_chain(const t_upstyp_matched_instance * const instance,
                 char * const string);
/*
 * Definition of global variables.
 */
static int g_MATCH_DONE = 0;

#define VPREFIX "UPSLIST: "

#ifndef NULL
#define NULL 0
#endif

#define FromVersion(ELEMENT) \
{ if (!upsutl_stricmp((buffer),"" #ELEMENT ""))    \
  { valid=1; \
    if(instance->version)                               \
    { if (instance->version->ELEMENT)                   \
      { (void) printf("\"%s\" ",instance->version->ELEMENT);\
      } else {                                          \
        (void) printf("\"\" ");                         \
      }                                                 \
    } else {                                            \
      (void) printf("\"\" ");                           \
    }                                                   \
  }                                                     \
}
#define FromTable(ELEMENT) \
{ if (!upsutl_stricmp((buffer),"" #ELEMENT ""))    \
  { valid=1; \
    if(instance->table)                                 \
    { if (instance->table->ELEMENT)                     \
      { (void) printf("\"%s\" ",instance->table->ELEMENT);\
      } else {                                          \
        (void) printf("\"\" ");                         \
      }                                                 \
    } else {                                            \
      (void) printf("\"\" ");                           \
    }                                                   \
  }                                                     \
}
#define FromDatabase(ELEMENT,STRING) \
{ if (!upsutl_stricmp((buffer),STRING))                 \
  { valid=1;                                            \
    if(product->db_info)                                \
    { if (product->db_info->ELEMENT)                    \
      { (void) printf("\"%s\" ",product->db_info->ELEMENT);\
      } else {                                          \
        (void) printf("\"\" ");                         \
      }                                                 \
    } else {                                            \
      (void) printf("\"\" ");                           \
    }                                                   \
  }                                                     \
}
#define FromConfig(ELEMENT,STRING) \
{ if (!upsutl_stricmp((buffer),STRING))                 \
  { valid=1;                                            \
    if(config_ptr)                                      \
    { if (config_ptr->ELEMENT)                          \
      { (void) printf("\"%s\" ",config_ptr->ELEMENT);   \
      } else {                                          \
        (void) printf("\"\" ");                         \
      }                                                 \
    } else {                                            \
      (void) printf("\"\" ");                           \
    }                                                   \
  }                                                     \
}
#define OutputConfig() \
{ FromConfig(upd_usercode_dir,"UPD_UserCode_Dir")                 \
  FromConfig(setups_dir,"Setups_Dir")                             \
  FromConfig(ups_db_version,"DB_Version")                         \
  FromConfig(prod_dir_prefix,"Prod_dir_prefix")                   \
  FromConfig(man_target_dir,"Man_Target_dir")                     \
  FromConfig(catman_target_dir,"Catman_Target_dir")               \
  FromConfig(html_target_dir,"Html_Target_dir")                   \
  FromConfig(info_target_dir,"Info_Target_dir")                   \
  FromConfig(version_subdir,"Version_subdir")                     \
  FromConfig(strict_deps,"Strict_deps")                           \
}

#define FromAny(ELEMENT) \
{ if (!upsutl_stricmp(buffer,"" #ELEMENT ""))           \
  { valid=1;                                            \
    if(instance->chain)                                 \
    { if (instance->chain->ELEMENT)                     \
      { (void) printf("\"%s\" ",instance->chain->ELEMENT);\
      } else {                                          \
        if(instance->version)                           \
        { if (instance->version->ELEMENT)               \
          { (void) printf("\"%s\" ",instance->version->ELEMENT);\
          } else {                                      \
            if (instance->table)                        \
            { if (instance->table->ELEMENT)             \
              { (void) printf("\"%s\" ",instance->table->ELEMENT);\
              } else {                                  \
                (void) printf("\"\" ");                 \
              }                                         \
            } else {                                    \
              (void) printf("\"\" ");                   \
            }                                           \
          }                                             \
        } else {                                        \
          if (instance->table)                          \
          { if (instance->table->ELEMENT)               \
            { (void) printf("\"%s\" ",instance->table->ELEMENT);\
            } else {                                    \
              (void) printf("\"\" ");                   \
            }                                           \
          } else {                                      \
            (void) printf("\"\" ");                     \
          }                                             \
        }                                               \
      }                                                 \
    } else {                                            \
      if(instance->version)                             \
      { if (instance->version->ELEMENT)                 \
        { (void) printf("\"%s\" ",instance->version->ELEMENT);\
        } else {                                        \
          if (instance->table)                          \
          { if (instance->table->ELEMENT)               \
            { (void) printf("\"%s\" ",instance->table->ELEMENT);\
            } else {                                    \
              (void) printf("\"\" ");                   \
            }                                           \
          } else {                                      \
            (void) printf("\"\" ");                     \
          }                                             \
        }                                               \
      } else {                                          \
        if (instance->table)                            \
        { if (instance->table->ELEMENT)                 \
          { (void) printf("\"%s\" ",instance->table->ELEMENT);\
          } else {                                      \
            (void) printf("\"\" ");                     \
          }                                             \
        } else {                                        \
          (void) printf("\"\" ");                       \
        }                                               \
      }                                                 \
    }                                                   \
  }                                                     \
}
#define FromBoth(ELEMENT) \
{ if (!upsutl_stricmp(buffer,"" #ELEMENT ""))    \
  { valid=1; \
    (void) printf("\"");                                \
    if(instance->chain)                                 \
    { if (instance->chain->ELEMENT)                     \
      { (void) printf("%s:",instance->chain->ELEMENT);  \
      } else {                                          \
        (void) printf(":");                             \
      }                                                 \
    }                                                   \
    if (instance->xtra_chains)                          \
    { for (clist = instance->xtra_chains ;              \
           clist ; clist = clist->next)                 \
      { cinst_ptr = (t_upstyp_instance *)clist->data;   \
        if(cinst_ptr->ELEMENT)                          \
        { (void) printf("%s:", cinst_ptr->ELEMENT);     \
        } else {                                        \
          (void) printf(":");                           \
        }                                               \
      }                                                 \
    }                                                   \
    if(instance->version)                               \
    { if (instance->version->ELEMENT)                   \
      { (void) printf("%s",instance->version->ELEMENT); \
      }                                                 \
    }                                                   \
    (void) printf("\" ");                               \
  }                                                     \
}
#define defaults(INSTANCE) \
{   (void) printf("\tVersion=%s",                                    \
           (instance->INSTANCE->version ? instance->INSTANCE->version : ""));\
    (void) printf("\tFlavor=%s\n", instance->INSTANCE->flavor);      \
    if (instance->INSTANCE->qualifiers)                              \
    { if(strlen(instance->INSTANCE->qualifiers))                     \
      { (void) printf("\t\tQualifiers=\"%s\"", instance->INSTANCE->qualifiers);\
      } else {      /* damn inconsistant if you ask me */            \
        (void) printf("\t\tQualifiers=\"\"");                        \
      }                                                              \
    } else {                                                         \
      (void) printf("\t\tQualifiers=\"\"");                          \
    }                                                                \
    if (instance->xtra_chains)                                       \
    { (void) printf("\tChains=");                                    \
    } else {                                                         \
      (void) printf("\tChain=");                                     \
    }                                                                \
    if ( instance->INSTANCE->chain )                                 \
    { (void) printf("%s", instance->INSTANCE->chain);                \
    } else {                                                         \
      (void) printf("\"\"");                                         \
    }                                                                \
    if (instance->xtra_chains)                                       \
    { for (clist = instance->xtra_chains ;                           \
           clist ; clist = clist->next)                              \
      { cinst_ptr = (t_upstyp_instance *)clist->data;                \
        (void) printf(",%s", cinst_ptr->chain );                     \
      }                                                              \
    } (void) printf("\n");                                           \
}
#define WAW(WHAT) \
{    if (instance->version)                                          \
    { if (instance->version->WHAT)                                   \
      { (void) printf("%s", instance->version->WHAT);                \
      } else {                                                       \
        (void) printf("\"\"");                                       \
      }                                                              \
    } else {                                                         \
      (void) printf("\"\"");                                         \
    }                                                                \
    if (instance->chain)                                             \
    { if (instance->chain->WHAT)                                     \
      { (void) printf(",%s", instance->chain->WHAT);                 \
      } else {                                                       \
        (void) printf(",\"\"");                                      \
      }                                                              \
      if (instance->xtra_chains)                                     \
      { for (clist = instance->xtra_chains ;                         \
             clist ; clist = clist->next)                            \
        { cinst_ptr = (t_upstyp_instance *)clist->data;              \
          if (cinst_ptr->WHAT)                                       \
          { (void) printf(",%s", cinst_ptr->WHAT);                   \
          } else {                                                   \
            (void) printf(",\"\"");                                  \
          }                                                          \
        }                                                            \
      }                                                              \
    } (void) printf("\n");                                           \
}

#define PRINT_DB(dbname) \
{    (void) printf("\nDATABASE=");                                   \
     if (dbname)                                                     \
     { (void) printf("%s ",dbname);                                  \
     } else {                                                        \
       (void) printf("\"\" ");                                       \
     } (void) printf("\n");                                          \
}

/* int list_error=UPS_SUCCESS;  */

int 
minst_full_cmp ( const void * const d1, const void * const d2 )
{
  const t_upstyp_matched_instance *a1 = d1;
  const t_upstyp_matched_instance *a2 = d2;
  int res;

  if (!a1 || !a2 || (long)d2 < 1000 ) {
     return a1 == 0 ?  -1 : 0;
  }
  (0 == (res = upsutl_stricmp( a1->version->product, a2->version->product ))) &&
  (0 == (res = upsutl_stricmp( a1->version->version, a2->version->version ))) &&
  (0 == (res = upsutl_stricmp( a1->version->flavor, a2->version->flavor ))) &&
  (0 == (res = upsutl_stricmp( a1->version->qualifiers, a2->version->qualifiers ))) &&
  (0 == (res = upsutl_stricmp( a1->version->chain, a2->version->chain ))) &&
  (0 == (res = upsutl_stricmp( a1->version->declarer, a2->version->declarer ))) &&
  (0 == (res = upsutl_stricmp( a1->version->declared, a2->version->declared )));

  return res;
}

int
minst_orig_slot_cmp ( const void * const d1, const void * const d2 )
{
  const t_upstyp_matched_instance *a1 = d1;
  const t_upstyp_matched_instance *a2 = d2;
  return a1->orig_slot - a2->orig_slot;
}

int 
product_full_cmp ( const void * const d1, const void * const d2 )
{
  t_upstyp_product *a1 = (t_upstyp_product *)d1;
  t_upstyp_product *a2 = (t_upstyp_product *)d2;
  int res;

  (0 == (res = upsutl_stricmp( a1->product, a2->product ))) &&
  (0 == (res = upsutl_stricmp( a1->version, a2->version ))) &&
  (0 == (res = upsutl_stricmp( a1->chain, a2->chain ))) &&
  (0 == (res = upsutl_stricmp( a1->ups_db_version, a2->ups_db_version )));

  return res;
}
int 
product_cmp ( const void * const d1, const void * const d2 )
{
  t_upstyp_product *a1 = (t_upstyp_product *)d1;
  t_upstyp_product *a2 = (t_upstyp_product *)d2;

    return upsutl_stricmp( a1->product, a2->product );
}
int product_name ( const void * const d1, const void * const d2 )
{
    char *a1 = (char *)d1;
    char *a2 = (char *)d2;
    return upsutl_stricmp( a1, a2 );
}
/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * ups_list
 *
 * This is the main line for the 'ups list' command.
 *
 * Input : argc, argv
 * Output: 
 * Return: 
 */
t_upslst_item *ups_list( t_upsugo_command * const a_command_line ,
			 const int verify )
{
  t_upslst_item *mproduct_list = NULL, *mproduct_item = NULL;
  t_upslst_item *minst_item = NULL;
  t_upstyp_db *db_info = 0;
  t_upslst_item *db_list = 0;
  t_upstyp_matched_product *mproduct = NULL;
  int new_db;
  t_upslst_item *all_products = 0;
  t_upslst_item *name=0;
  char *save_product;
  save_product=upsutl_str_create(a_command_line->ugo_product,' ');

  /* Get all the requested instances */
  UPS_VERIFY=verify;		/* this is REALLY the ups verify command */ 
  for (db_list = a_command_line->ugo_db ; db_list ; db_list=db_list->next) 
  { db_info = (t_upstyp_db *)db_list->data;
    new_db=1;
    if (NOT_EQUAL_ANY_MATCH(a_command_line->ugo_product)) 
    { all_products=upslst_add(all_products,a_command_line->ugo_product);
    } else {
      upsutl_get_files(db_info->name, (char *)ANY_MATCH, 
                       &all_products);
      upsver_mes(2,"%sStarting sort of product list\n",VPREFIX);
      all_products = upslst_sort0( all_products , product_name );
      upsver_mes(2,"%sEnding sort of product list\n",VPREFIX);
      all_products = upslst_first(all_products);  /* point to the start */
    }
    for (name= all_products ; name; name=name->next) 
    { a_command_line->ugo_product=name->data;
       mproduct_list = ups_list_core(a_command_line,db_list);
       /*  upsugo_prtlst(mproduct_list,"the products");*/
       upsver_mes(2,"%sFrom Database %s\n",VPREFIX,db_info->name);
       if(!verify)  /* verify is list with NO output */
       {
         /* Output the requested information from the instances */
         /*  upsugo_dump(a_command_line);*/
         if (!a_command_line->ugo_K)
         { if (db_info && db_info->name)
   	   { if (new_db)           /* only the first time !!! */
             { PRINT_DB(db_info->name);
               new_db=0;
             }
   	   }
         }
         list_output(mproduct_list, a_command_line);
         if (UPS_ERROR==UPS_SUCCESS)
         { if (mproduct_list) 
   	{ upsutl_statistics(mproduct_list, g_cmd_info[e_list].cmd);
   	}
         } else { 
   	/*      upserr_output(); 
   		upserr_clear(); 
   		UPS_ERROR=list_error; */
   	return 0; 
         }
       } else {
         /* now we must do the extra verify things - 
	    o verify the info in the dbconfig file
	    o verify the information in the instances
         */
         for (mproduct_item = mproduct_list ; mproduct_item ; 
   	   mproduct_item = mproduct_item->next) 
         { mproduct = (t_upstyp_matched_product *)mproduct_item->data;
           if (new_db && mproduct->db_info && mproduct->db_info->name)
   	   { PRINT_DB(mproduct->db_info->name);
	     new_db=0;
	     ups_verify_dbconfig(mproduct->db_info, 
   		       (t_upstyp_matched_instance *)mproduct->minst_list->data,
   		       a_command_line);
	     /* there may be lots of messages so output them on a per product
		basis */
	     upserr_output();
	     upserr_clear();
	   }
	   upserr_add(UPS_VERIFY_PRODUCT, UPS_INFORMATIONAL,
		      mproduct->product);

	   for (minst_item = mproduct->minst_list ; minst_item ;
   	     minst_item = minst_item->next)
	   { ups_verify_matched_instance(mproduct->db_info,
   				 (t_upstyp_matched_instance *)minst_item->data,
   				 a_command_line, mproduct->product);
	   }
	   /* there may be lots of messages so output them on a per product
	      basis */
	   upserr_output();
	   upserr_clear();
         }
       }
       /* free the matched products */
       (void )upsutl_free_matched_product_list(&mproduct_list);

       /* if we were just looking for dbconfig keywords, we got them, no need
	  to print them for every product in the  database. */
       if (! g_MATCH_DONE) {
	 break;
       }
    }
    /* restore original (in case it was * for next database) */
    a_command_line->ugo_product=save_product;
  }
/*  UPS_ERROR=list_error; */
  return 0;
}
/*
         (((int )(((char *)key->data)[0]) == '@') && 
           upsutl_stricmp(key->data,"upd_usercode_dir"))) 
*/

/*
 * This is used to prune duplicates out of the list results, 
 * especially when upd ends us invoking us with 
 * -H x64bit+y:x+1:x64bit:x
 * to handle machines that prefer 64bit.
 * It's funky because we want to preserve the
 * best-flavor-first order.  So first we 
 * number them by their position in the list
 * (in the flag field), then sort & uniq them
 * by all the usual fields, and then finally
 * re-sort them by the flag field order.
 * so if you take the second sort out, it breaks upd 
 * preferring the 64bit version appropriately.
 */

t_upslst_item *
uniq_instances(t_upslst_item *mproduct_list){
    t_upslst_item *l_ptr;
    t_upslst_item *prd;
    int i;
    t_upstyp_matched_product *mproduct;
    for( l_ptr=upslst_first(mproduct_list); l_ptr; l_ptr = l_ptr->next) {
        mproduct = l_ptr->data;
        i = 0;
        for (prd = upslst_first(mproduct->minst_list); prd; prd = prd->next ){
            ((t_upstyp_matched_instance *)(prd->data))->orig_slot = i++;
        }
        mproduct->minst_list = upslst_uniq(upslst_sort0(mproduct->minst_list,minst_full_cmp),minst_full_cmp);
        mproduct->minst_list = upslst_sort0(mproduct->minst_list,minst_orig_slot_cmp);
    }
    return mproduct_list;
}
/*-----------------------------------------------------------------------
 * ups_list_core
 *
 * Take the command line parameters and read in all of the desired instances
 *
 * Input : <input>
 * Output: <output>
 * Return: <return>
 */
t_upslst_item *ups_list_core(t_upsugo_command * const a_command_line ,
                             t_upslst_item * const db_list )
{
  t_upslst_item *mproduct_list = NULL, *key;
  int need_unique = 0;
  char * addr;
  t_upskey_map *keymap;
  int do_match = 1;
  int mwprod=0;             /* match with product , more exceptions will come */
  t_upstyp_product *db;
  t_upstyp_db *cli_db;
  t_upstyp_matched_product *mproduct;

  /* if the requested keyword is only in the dbconfig file, just read it and
     no instances */
  for (key = (t_upslst_item *)a_command_line->ugo_key ; key ;
	 key = key->next) {
    keymap = upskey_get_info((char *)key->data);
    if (!upsutl_strincmp((char *)key->data,"action",6))
    { mwprod=1;     /* match with product */
    }
    if ((((int )(((char *)key->data)[0]) == '+') ||
         ((int )(((char *)key->data)[0]) == '@') ||
         ((int )(((char *)key->data)[0]) == '_')) || mwprod
        || (keymap && 
	(UPSKEY_ISIN_VERSION(keymap->flag) ||
	 UPSKEY_ISIN_TABLE(keymap->flag) ||
	 UPSKEY_ISIN_CHAIN(keymap->flag)))) {
      /* yes the keyword is in a version, table, or chain file. we must do the
	 match below */
      do_match = 1;
      break;
    } else {
      do_match = 0;
    }
  }
  if (do_match) {
    g_MATCH_DONE = 1;
    /* if no chains were entered, ask for them all */
    if (! a_command_line->ugo_chain) { 
      addr=upsutl_str_create(ANY_MATCH,' ');
      a_command_line->ugo_chain = upslst_new(addr);
    }

    /* Get all the instances that the user requested */
    mproduct_list = upsmat_instance(a_command_line, db_list , need_unique);
    if (UPS_ERROR != UPS_SUCCESS) { upserr_output(); upserr_clear(); }
    mproduct_list = uniq_instances(mproduct_list);
#if 0
    mproduct_list = upslst_first(mproduct_list);   /* point to the start */
    upsver_mes(2,"%sStarting sort of product list\n",VPREFIX);
    mproduct_list = upslst_uniq(upslst_sort0( mproduct_list , product_cmp ),product_full_cmp);
    mproduct_list = upslst_uniq(upslst_sort0( mproduct_list , product_full_cmp ),product_full_cmp);
    upsver_mes(2,"%sEnding sort of product list\n",VPREFIX);
    mproduct_list = upslst_first(mproduct_list);   /* point to the start */
#endif
  } else {
    g_MATCH_DONE = 0;
    cli_db = (t_upstyp_db *)db_list->data;
    /* the keywords requested are only in the dbconfig file. read the file */
    db = upsutl_get_config(cli_db->name);
    if (db) {
      mproduct = ups_new_matched_product(NULL, NULL, NULL);
      mproduct->db_info = (t_upstyp_db *)upsmem_malloc(sizeof(t_upstyp_db));
      mproduct->db_info->name = upsutl_str_create(cli_db->name, ' ');
      mproduct->db_info->config = db->config;
      mproduct_list = upslst_add(mproduct_list, (void *)mproduct);
    }
  }
  return(mproduct_list);
}

/*-----------------------------------------------------------------------
 * list_output
 *
 * Output all of the desired information from the matched instances TBD
 *
 * Input : <input>
 * Output: <output>
 * Return: <return>
 */
void list_output(const t_upslst_item * const a_mproduct_list,
		 const t_upsugo_command * const a_command_line)
{
  t_upstyp_matched_product *mproduct = NULL;
  t_upslst_item *tmp_mprod_list = NULL;
  t_upslst_item *tmp_minst_list = NULL, *clist = NULL;
  t_upstyp_instance *cinst_ptr = NULL;
  t_upstyp_matched_instance *instance= NULL;
  t_upstyp_config  *config_ptr = 0;
  t_upslst_item *ul_ptr = 0;
  t_upslst_item *al_ptr = 0;
  t_upslst_item *l_ptr = 0;
  t_upstyp_action *ac_ptr = 0;
  char *nodes=0;
  int count=0;
  static char buffer[20];
  int valid = 0;     /* needed by the From* macros */
  valid ^= 0;        /* but is not used by us except for this bogus reference */

  for (tmp_mprod_list = (t_upslst_item *)a_mproduct_list ; tmp_mprod_list ;
       tmp_mprod_list = tmp_mprod_list->next) 
  { mproduct = (t_upstyp_matched_product *)tmp_mprod_list->data;
    for (tmp_minst_list = mproduct->minst_list ; tmp_minst_list ;
	 tmp_minst_list = tmp_minst_list->next) 
    { instance = (t_upstyp_matched_instance *)(tmp_minst_list->data);
/* A as in a single product loop */
      if (!a_command_line->ugo_K)
      { (void) printf("\tProduct=%s",mproduct->product);
        if (instance->chain) 
        { defaults(chain)
        } else { 
          if (instance->version )
          {  defaults(version) 
          } else { 
            if (instance->table )
            {  defaults(table)
            } /* else what in the hell are we doing here ?? */
          }
        }
        if (a_command_line->ugo_l && instance->version )
        { (void) printf("\t\tDeclared=");
          (void) strcpy(buffer,"declared");
          FromBoth(declared);
          (void) printf("\n");
          (void) printf("\t\tDeclarer=");
          (void) strcpy(buffer,"declarer");
          FromBoth(declarer);
          (void) printf("\n");
          (void) printf("\t\tModified=");
          (void) strcpy(buffer,"modified");
          FromBoth(modified);
          (void) printf("\n");
          (void) printf("\t\tModifier=");
          (void) strcpy(buffer,"modifier");
          FromBoth(modifier);
          (void) printf("\n");
          (void) printf("\t\tHome=");
          if (UPSRELATIVE(instance->version->prod_dir))
          { if (mproduct->db_info) 
            { config_ptr = mproduct->db_info->config;
              if (config_ptr) 
              { if (config_ptr->prod_dir_prefix) 
                { (void) printf("%s/",config_ptr->prod_dir_prefix); }
              }
            }
          }
/*          if (mproduct->db_info) 
          { config_ptr = mproduct->db_info->config;
            if (config_ptr) 
            { if (config_ptr->prod_dir_prefix) 
              { (void) printf("%s",config_ptr->prod_dir_prefix); }
            }
          }
*/
          if (instance->version->prod_dir)
          { (void) printf("%s", instance->version->prod_dir);
          }
          (void) printf("\n");

          if (instance->version->compile_dir || 
              instance->version->compile_file)
          { (void) printf("\t\tCompile=");
            if (instance->version->compile_dir)
            { (void) printf("%s",instance->version->compile_dir); }
            if (instance->version->compile_file)
            { (void) printf("/%s",instance->version->compile_file); }
            (void) printf("\n");
          } else {
            (void) printf("\t\tNo Compile Directive\n"); /* ;) */
          }
          if (upsutl_is_authorized( instance, mproduct->db_info,&nodes))
          { (void) printf("\t\tAuthorized, Nodes=%s\n",nodes);
          } else {
            (void) printf("\t\tNOT Authorized, Nodes=%s\n",nodes);
          }
          (void) printf("\t\tUPS_Dir=");
          (void) strcpy(buffer,"ups_dir");
          FromVersion(ups_dir);
          (void) printf("\n");
          (void) printf("\t\tTable_Dir=");
          (void) strcpy(buffer,"table_dir");
          FromVersion(table_dir);
          (void) printf("\n");
          (void) printf("\t\tTable_File=");
          (void) strcpy(buffer,"table_file");
          FromVersion(table_file);
          (void) printf("\n");
          (void) printf("\t\tArchive_File=");
          (void) strcpy(buffer,"archive_file");
          FromVersion(archive_file)
          (void) printf("\n");
          (void) printf("\t\tDescription=");
          (void) strcpy(buffer,"description");
          FromTable(description);
          (void) printf("\n");
          for ( ul_ptr = upslst_first( instance->version->user_list ); 
                ul_ptr; ul_ptr = ul_ptr->next, count++ )
          { (void) printf("\t\t%s \n",(char *)ul_ptr->data); /* Give keys and values */
          }
        }
        if (a_command_line->ugo_l) 
        { if (instance->table)
          { for ( al_ptr = upslst_first( instance->table->action_list ); 
                  al_ptr; al_ptr = al_ptr->next, count++ )
            { ac_ptr=al_ptr->data;
              (void) printf("\t\tAction=%s\n",ac_ptr->action);
              if ( ac_ptr->command_list )
              { l_ptr = upslst_first( ac_ptr->command_list );
                for ( ; l_ptr; l_ptr = l_ptr->next )
                { (void) printf( "\t\t\t%s\n", (char*)l_ptr->data );
                }
              }
            }
          }
        }
        (void) printf("\n");
      } else {
        list_K(instance,a_command_line,mproduct,g_MATCH_DONE);
      }
    }
    /* we must check if we only read in a dbconfig file, this can only
       happen when -K is also entered */
    if (! g_MATCH_DONE) 
    { list_K(instance,a_command_line,mproduct,g_MATCH_DONE);
    }
/* end product loop */
  }
}

t_upslst_item *ups_layout( t_upsugo_command * const a_command_line ,
			 const int verify )
{
   /* layout is basically  of
    * ups list -K database:prod_dir_prefix
    * with some filtering...
    */
   int pfds[2];
   char buf[2048];
   int res, i;
   pid_t pid;
   char *parts[4];
   int part = 0;

   res = pipe(pfds);
   if (res != 0 ) {
       upserr_add(UPS_NO_MEMORY,"Couldn't get a pipe()");
       return 0;
   }
   pid = fork();
   if (0 == pid) {
        /* just do a list with keywords into the pipe */
        close(1);
        dup(pfds[1]);
        close(pfds[0]);
        close(pfds[1]);
        a_command_line->ugo_K = 1;
        a_command_line->ugo_key = 0;
        a_command_line->ugo_key = upslst_add(a_command_line->ugo_key,upsutl_str_create("database",'p'));             
        a_command_line->ugo_key = upslst_add(a_command_line->ugo_key,upsutl_str_create("prod_dir_prefix",'p'));             
        a_command_line->ugo_key = upslst_add(a_command_line->ugo_key,upsutl_str_create("version_subdir",'p'));             
        ups_list(a_command_line, verify);
        exit(0);
   } else if (0 < pid) {
        close(0);
        dup(pfds[0]);
        close(pfds[0]);
        close(pfds[1]);
        while( fgets(buf, 2048, stdin ) ) {
            /* printf("Debug: read line: %s", buf); */
            part = 0;
            for(i = 0; i<2048; i++) {
                if (0 == buf[i] || part > 3)
                    break;
                if (buf[i] == '\n') {
                    buf[i] = 0;
                }
                if (buf[i] == ' ') {
                    buf[i] = 0;
                    parts[part++] = buf+i+1;
                }
            }
            if ( part < 2 ) {
               continue;
            }

            if (0 == strcmp(buf, parts[0]) && parts[1][1] == '1')  {
                printf("%s: \"unified\"\n", buf);
            } else {
                printf("%s: \"traditional\"\n", buf);
            }
        }
        
   } else {
       upserr_add(UPS_NO_MEMORY,"Couldn't fork()");
       return 0;
   } 
   return 0;

}

void list_K(const t_upstyp_matched_instance * const instance, 
            const t_upsugo_command * const command, 
            const t_upstyp_matched_product * const product,
	    const int match_done )
{
  t_upslst_item *l_ptr = 0;
  t_upslst_item *al_ptr = 0;            /* Action list pointer */
  t_upslst_item *ald_ptr = 0;           /* Action list detail lines pointer */
  t_upstyp_action *ac_ptr = 0;
  t_upstyp_instance *cinst_ptr = 0;
  t_upslst_item *clist = 0;
  t_upstyp_config  *config_ptr = 0;
  t_upstyp_db *db_ptr = 0;
  static char buffer[20];
  static char actbuf1[MAX_LINE_LEN];
  static char actbuf2[MAX_LINE_LEN];
  char *nodes=0;
  char *str_val;
  char *addr;
  int count=0;
  int exists=1;
  int valid=0;
/* if action=someaction and there is no match must know and put out "" */
  int found=0; 
  if (product->db_info) 
  { db_ptr = product->db_info;
    config_ptr = db_ptr->config;
  }
  if (match_done)
  { for ( l_ptr = upslst_first( command->ugo_key ); 
	  l_ptr; l_ptr = l_ptr->next, count++ )
    { valid=0;
      (void) strcpy(buffer,l_ptr->data);
      if(!upsutl_stricmp(l_ptr->data,"+"))
      { (void) strcpy(buffer,"product");
        FromAny(product) 
	(void) strcpy(buffer,"version");
	FromAny(version) 
	(void) strcpy(buffer,"flavor");
	FromAny(flavor) 
	(void) strcpy(buffer,"qualifiers");
	FromAny(qualifiers)
	(void) strcpy(buffer,"chain");
        print_chain(instance,buffer);
        (void) strcpy(buffer,"+");
      }
      FromAny(product) 
      FromAny(version) 
      FromAny(flavor) 
      FromAny(qualifiers)
      if (!upsutl_stricmp(buffer,"chain")) { valid=1; }
      print_chain(instance,buffer);
      FromVersion(table_file)
      FromVersion(table_dir)
      FromVersion(ups_dir)
      FromVersion(prod_dir)
      FromVersion(archive_file)
      FromVersion(compile_file)
      FromVersion(compile_dir)
      FromTable(description)
      FromTable(man_source_dir)
      FromTable(catman_source_dir)
      FromTable(html_source_dir)
      FromTable(news_source_dir)
      FromTable(info_source_dir)
      FromVersion(origin)
      /* FromChain(chain) */
      FromBoth(declarer)
      FromBoth(declared)
      FromBoth(modifier)
      FromBoth(modified)
      if (!upsutl_stricmp(buffer,"authorized_nodes")) 
      { (void)(upsutl_is_authorized(instance, product->db_info,&nodes));
        (void) printf("\"%s\" ",nodes);
        valid=1;
      }
      if(!upsutl_strincmp(buffer,"action",6))
      { valid=1; 
        found=0;
        if (instance->table)
        { for ( al_ptr = upslst_first( instance->table->action_list ); 
                  al_ptr; al_ptr = al_ptr->next, count++ )
          { ac_ptr=al_ptr->data;
            if(!upsutl_stricmp(buffer,"actions"))
            { (void) printf("\"%s:%d\" ",ac_ptr->action,
                     upslst_count(ac_ptr->command_list));
              found=1;
            } else { 
              if(strlen(buffer)<8) /* action only or action= (nothing) */
              { valid=0;
              }
              (void) sprintf(actbuf1,"action=%s",ac_ptr->action);
              (void) sprintf(actbuf2,"action=\"%s\"",ac_ptr->action);
              if(!upsutl_stricmp(actbuf1,buffer) ||
                 !upsutl_stricmp(actbuf2,buffer))
              { if ( ac_ptr->command_list )
                { ald_ptr = upslst_first( ac_ptr->command_list );
                  for ( ; ald_ptr; ald_ptr = ald_ptr->next )
                  { (void) printf( "\"%s\" ", (char*)ald_ptr->data );
                    found=1;
                  }
                }
              }
            }
          }
          if (!found) /* action=something but no something */
          { (void) printf( "\"\" ");
          }
        }
      }
      if (!upsutl_stricmp(buffer,"statistics")) 
      { if (config_ptr)
        { if (config_ptr->statistics)
          { if (strstr(config_ptr->statistics,product->product))
            { (void) printf("\"statistics\" ");
            } else { 
              (void) printf("\"\" ");
            }
          } else {
            (void) printf("\"\" ");
          }
        } else {
          (void) printf("\"\" ");
        }
        valid=1;
      }
      FromDatabase(name,"Database")
      FromDatabase(name,"Db")
      OutputConfig();
      if (!strcmp(l_ptr->data,"key"))
      { valid=1; 
        (void) printf("\"%d\"",upsget_key(instance->version)); /* test */ 
      }
      if (!strncmp(l_ptr->data,"_",1))
      { str_val=0;
        valid=1;
        if (instance->chain) 
           str_val = upskey_inst_getuserval( instance->chain,l_ptr->data);
        if (instance->version && !str_val )
           str_val = upskey_inst_getuserval( instance->version,l_ptr->data);
	if (instance->table && !str_val )
	   str_val = upskey_inst_getuserval( instance->table,l_ptr->data);
	if (!str_val) 
	{ (void) printf("\"\" ");  /* this gives them "" for a invalid _key */
	} else {
	  if (strlen(str_val))
          { (void) printf("\"%s\" ",str_val);
	  } else { 
	    (void) printf("\"%s\" ",(char *)l_ptr->data);
	  }
	} 
      }
/* look for "processed values" spam? */
      if (!strncmp(l_ptr->data,"@",1))
      { if(!upsutl_stricmp(l_ptr->data,"@table_file"))
        { valid=1;
          if (instance->version)
          { addr=upsget_table_file(product->product,
                                   instance->version->table_file,
                                   instance->version->table_dir,
                                   instance->version->ups_dir,
                                   instance->version->prod_dir,
                                   product->db_info,
                                   exists);
            if(addr)
            { (void) printf("\"%s\" ",addr);
            } else { 
              (void) printf("\"\" "); 
            }
          } else {
            (void) printf("\"\" "); 
          }
        }
        if(!upsutl_stricmp(l_ptr->data,"@prod_dir"))
        { valid=1;
          (void) printf("\"%s\" ", 
                 upsget_prod_dir(product->db_info,instance,command));
        } 
        if(!upsutl_stricmp(l_ptr->data,"@compile_file"))
        { valid=1;
          (void) printf("\"%s\" ",
                 upsget_compile(product->db_info,instance,command));
        } 
        if(!upsutl_stricmp(l_ptr->data,"@ups_dir"))
        { valid=1;
          (void) printf("\"");
          if (UPSRELATIVE(instance->version->ups_dir))
          { (void) printf("%s/", upsget_prod_dir(product->db_info,instance,command));
          } 
          (void) printf("%s\" ", instance->version->ups_dir ? instance->version->ups_dir : "" );
        } 
      }
      if (!valid) 
      { upserr_add(UPS_INVALID_KEYWORD, UPS_WARNING,l_ptr->data,"-K"); 
      }
    }
  } else { /* if (match_done) */
    /* just have the dbconfig info */
    for ( l_ptr = upslst_first( command->ugo_key ); 
	  l_ptr; l_ptr = l_ptr->next, count++ )
    { valid=0;
      (void) strcpy(buffer,(char *)l_ptr->data);
      OutputConfig();
      if ((!upsutl_stricmp((buffer),"Db")) || (!upsutl_stricmp((buffer),"Database")))
      { valid=1;
        (void) printf("\"%s\" ",db_ptr->name);
      }
      if (!valid) 
      { upserr_add(UPS_INVALID_KEYWORD, UPS_WARNING,l_ptr->data,"-K"); 
      }
    }
  }
  (void) printf("\n");
}
void print_chain(const t_upstyp_matched_instance * const instance,
                 char * const string)
{ t_upstyp_instance *cinst_ptr = 0;
  t_upslst_item *clist = 0;
  if (!upsutl_stricmp(string,"chain"))
  { (void) printf("\""); /* first " */
    if(instance->chain)
    { (void) printf("%s",instance->chain->chain);
      if (instance->xtra_chains)
      { for (clist = instance->xtra_chains ;
             clist ; clist = clist->next)
        { cinst_ptr = (t_upstyp_instance *)clist->data;
          if(cinst_ptr->chain)
          { (void) printf(":%s", cinst_ptr->chain);
    } } } } 
    (void) printf("\" "); /* end " */
  }
}

