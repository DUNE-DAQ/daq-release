/************************************************************************
 *
 * FILE:
 *       upstyp.c
 * 
 * DESCRIPTION: 
 *       Creation and destuction of common types for ups.
 *       Specially for types there is using upsmem for reference
 *       counting.
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
 *       29-jul-1997, LR, first
 *       30-jul-1997, LR, Using ups_memory (reference counting).
 *
 ***********************************************************************/

/* standard include files */
#include <stdlib.h>
#include <string.h> /* for memset */

/* ups specific include files */
#include "upsutl.h"
#include "upstyp.h"
#include "upsmem.h"
#include "upslst.h"
#include "upserr.h"

/*
 * Definition of public variables
 */

/*
 * Declaration of private functions
 */
static int all_gone( const void * const ptr );

/*
 * Definition of public functions
 */

/*-----------------------------------------------------------------------
 * ups_new_matched_product
 *
 * Will create an empty t_upstyp_matched_product structure.
 *
 * Input : none
 * Output: none
 * Return: t_upstyp_matched_product *, a pointer to a matched product structure
 */
t_upstyp_matched_product *ups_new_matched_product(
				      const t_upstyp_db * const a_db_info,
				      const char * const a_prod_name,
				      const t_upslst_item * const a_minst_list)
{
  t_upstyp_matched_product *mprod_ptr =
   (t_upstyp_matched_product *)upsmem_malloc(sizeof(t_upstyp_matched_product));

  mprod_ptr->db_info = (t_upstyp_db *)a_db_info;
  if (a_db_info) {
    upsmem_inc_refctr((void *)a_db_info);
    if (a_db_info->name) {
      upsmem_inc_refctr((void *)a_db_info->name);
    }
    if (a_db_info->config) {
      upsmem_inc_refctr((void *)a_db_info->config);
    }
  }
  mprod_ptr->product = (char *)a_prod_name;
  if (a_prod_name) {
    upsmem_inc_refctr((void *)a_prod_name);
  }
  mprod_ptr->minst_list = (t_upslst_item *)a_minst_list;

  /* do not increment the reference counter for the matched instance list as
     the list pointer we were given was only a temporary storage for the
     list */

  return mprod_ptr;
}

/*-----------------------------------------------------------------------
 * ups_free_matched_product
 *
 * Free a matched product structure.
 *
 * Input : pointer to matched product structure
 * Output: none
 * Return: NULL
 */
t_upstyp_matched_product *ups_free_matched_product(
                                 t_upstyp_matched_product * const a_mproduct)
{
  if (a_mproduct) {
    /* we incremented the ref counter in the ups_new_matched_product function,
       doing a free here will decrement it or free it */
    if (all_gone(a_mproduct)) {
      if (a_mproduct->db_info) {
	if (a_mproduct->db_info->config) {
	  upsmem_dec_refctr((void *)a_mproduct->db_info->config);
	}
	if (a_mproduct->db_info->name) {
	  upsmem_free(a_mproduct->db_info->name);
	}
	upsmem_free(a_mproduct->db_info);
      }
      if (a_mproduct->product) {
	upsmem_free(a_mproduct->product);
      }
      /* we do not free the matched instance list as it was not malloced when
	 we created the new mproduct. nor was the reference counter
	 incremented */
    }
    upsmem_free((void *)a_mproduct);
  }

  return NULL;
}

/*-----------------------------------------------------------------------
 * ups_new_matched_instance
 *
 * Will create an empty t_upstyp_matched_instance structure.
 *
 * Input : none
 * Output: none
 * Return: t_upstyp_matched_instance *, pointer to a matched instance structure
 */
t_upstyp_matched_instance *ups_new_matched_instance( void )
{
  t_upstyp_matched_instance *inst_ptr =
    (t_upstyp_matched_instance *)upsmem_malloc(
					    sizeof(t_upstyp_matched_instance));

  (void) memset( inst_ptr, 0, sizeof( t_upstyp_matched_instance ) );

  return inst_ptr;
}

/*-----------------------------------------------------------------------
 * ups_free_matched_instance
 *
 * Will free all space associated with a matched instance
 *
 * Input : the matched instance structure pointer
 * Output: none
 * Return: NULL
 */
t_upstyp_matched_instance *ups_free_matched_instance(
                                 t_upstyp_matched_instance * const minst_ptr)
{
  if (minst_ptr && all_gone(minst_ptr)) {
    if (minst_ptr->chain) {
      (void) ups_free_instance(minst_ptr->chain);
    }
    if (minst_ptr->version) {
      (void) ups_free_instance(minst_ptr->version);
    }
    if (minst_ptr->table) {
      (void) ups_free_instance(minst_ptr->table);
    }
    if (minst_ptr->xtra_chains) {
      t_upslst_item * xchain_item = NULL;
  
      for (xchain_item = minst_ptr->xtra_chains ; xchain_item ;
	   xchain_item = xchain_item->next); {
	(void) ups_free_instance((t_upstyp_instance *)xchain_item->data);
      }
    }
    (void) ups_free_matched_instance_structure(minst_ptr);
  }

  return NULL;
}

/*-----------------------------------------------------------------------
 * ups_free_matched_instance_structure
 *
 * Will free all space associated with the structure of a matched instance
 *  but not the instances themselves.
 *
 * Input : the matched instance structure pointer
 * Output: none
 * Return: NULL
 */
t_upstyp_matched_instance *ups_free_matched_instance_structure(
                                 t_upstyp_matched_instance * const minst_ptr)
{
  if (minst_ptr && all_gone(minst_ptr)) {
    if (minst_ptr->xtra_chains) {
    (void) upslst_free(minst_ptr->xtra_chains, ' ');
    }

    upsmem_free(minst_ptr);
  }

  return NULL;
}

/*-----------------------------------------------------------------------
 * ups_new_product
 *
 * Will create an empty t_upstyp_product structure.
 *
 * Input : none
 * Output: none
 * Return: t_upstyp_product *, a pointer to a product structure
 */
t_upstyp_product *ups_new_product( void )
{
  t_upstyp_product *prod_ptr =
    (t_upstyp_product *)upsmem_malloc( sizeof( t_upstyp_product ) );

  (void) memset( prod_ptr, 0, sizeof( t_upstyp_product ) );

  return prod_ptr;
}

/*-----------------------------------------------------------------------
 * ups_free_product
 *
 * Will free a product.
 *
 * Input : t_upstyp_product *, a pointer to a product structure.
 * Output: none
 * Return: int, 0 on failer else 1.
 */
int ups_free_product( t_upstyp_product * const prod_ptr )
{
  t_upslst_item *l_ptr;
  t_upstyp_instance *inst_ptr;

  if ( !prod_ptr ) return 0;

  if ( all_gone( prod_ptr ) ) {
    if ( prod_ptr->file ) { upsmem_free( prod_ptr->file ); }
    if ( prod_ptr->product ) { upsmem_free( prod_ptr->product); }
    if ( prod_ptr->version ) { upsmem_free( prod_ptr->version ); }
    if ( prod_ptr->chain ) { upsmem_free( prod_ptr->chain ); }
    if ( prod_ptr->ups_db_version ) { upsmem_free( prod_ptr->ups_db_version ); }
    if ( prod_ptr->description ) { upsmem_free( prod_ptr->description ); }

    /* get rid of instances */
    
    l_ptr = upslst_first( prod_ptr->instance_list );
    while( l_ptr ) {
      inst_ptr = l_ptr->data;
      l_ptr = upslst_delete( l_ptr, inst_ptr, ' ' );
      (void) ups_free_instance( inst_ptr );
    }

    /* get rid of user key words */

    if ( prod_ptr->user_list ) { (void) upslst_free( prod_ptr->user_list, 'd' ); }
    
    /* get rid of comments */

    (void) upslst_free( prod_ptr->comment_list, 'd' );

    /* get rid of config structure */
    
    (void) ups_free_config( prod_ptr->config );

    upsmem_free( prod_ptr );
  }
  
  return 1;
}

/*-----------------------------------------------------------------------
 * ups_new_instance
 *
 * Will create an empty t_upstyp_instance structure.
 *
 * Input : none
 * Output: none
 * Return: t_upstyp_instance *, a pointer to an instance structure
 */
t_upstyp_instance *ups_new_instance( void )
{
  t_upstyp_instance *inst_ptr =
    (t_upstyp_instance *)upsmem_malloc( sizeof( t_upstyp_instance ) );
  
  (void) memset( inst_ptr, 0, sizeof( t_upstyp_instance ) );

  return inst_ptr;
}

/*-----------------------------------------------------------------------
 * ups_free_instance
 *
 * Will free an instance.
 *
 * Input : t_upstyp_instance *, a pointer to an instance structure.
 * Output: none
 * Return: int, 0 on failer else 1.
 */
int ups_free_instance( t_upstyp_instance * const inst_ptr )
{
  t_upslst_item *l_ptr;
  void *act_ptr;

  if ( !inst_ptr ) return 0;

  if ( all_gone( inst_ptr ) ) {
    if ( inst_ptr->product ) { upsmem_free( inst_ptr->product); }
    if ( inst_ptr->version ) { upsmem_free( inst_ptr->version ); }
    if ( inst_ptr->flavor ) { upsmem_free( inst_ptr->flavor ); }
    if ( inst_ptr->qualifiers ) { upsmem_free( inst_ptr->qualifiers ); }
  
    if ( inst_ptr->chain ) { upsmem_free( inst_ptr->chain ); }
    if ( inst_ptr->declarer ) { upsmem_free( inst_ptr->declarer ); }
    if ( inst_ptr->declared ) { upsmem_free( inst_ptr->declared ); }
    if ( inst_ptr->modifier ) { upsmem_free( inst_ptr->modifier ); }
    if ( inst_ptr->modified ) { upsmem_free( inst_ptr->modified ); }
    if ( inst_ptr->origin ) { upsmem_free( inst_ptr->origin ); }
    if ( inst_ptr->prod_dir ) { upsmem_free( inst_ptr->prod_dir ); }
    if ( inst_ptr->ups_dir ) { upsmem_free( inst_ptr->ups_dir ); }
    if ( inst_ptr->table_dir ) { upsmem_free( inst_ptr->table_dir ); }
    if ( inst_ptr->table_file ) { upsmem_free( inst_ptr->table_file ); }
    if ( inst_ptr->archive_file ) { upsmem_free( inst_ptr->archive_file ); }
    if ( inst_ptr->authorized_nodes ) { upsmem_free( inst_ptr->authorized_nodes ); }
    if ( inst_ptr->description ) { upsmem_free( inst_ptr->description ); }
    
    if ( inst_ptr->statistics ) { upsmem_free( inst_ptr->statistics ); }
    
    if ( inst_ptr->compile_dir ) { upsmem_free( inst_ptr->compile_dir ); }    
    if ( inst_ptr->compile_file ) { upsmem_free( inst_ptr->compile_file ); }

    if ( inst_ptr->catman_source_dir ) { upsmem_free( inst_ptr->catman_source_dir ); }
    if ( inst_ptr->html_source_dir ) { upsmem_free( inst_ptr->html_source_dir ); }
    if ( inst_ptr->info_source_dir ) { upsmem_free( inst_ptr->info_source_dir ); }
    if ( inst_ptr->man_source_dir ) { upsmem_free( inst_ptr->man_source_dir ); }
    if ( inst_ptr->news_source_dir ) { upsmem_free( inst_ptr->news_source_dir ); }
    
    if ( inst_ptr->db_dir ) { upsmem_free( inst_ptr->db_dir ); }
  
    if ( inst_ptr->user_list ) { (void) upslst_free( inst_ptr->user_list, 'd' ); }
    
    l_ptr = upslst_first( inst_ptr->action_list );
    while( l_ptr ) {
      act_ptr = l_ptr->data;
      l_ptr = upslst_delete( l_ptr, act_ptr, ' ' );
      (void) ups_free_action( act_ptr );
    }

    (void) ups_free_instance( inst_ptr->sav_inst );

    upsmem_free ( inst_ptr );
  }

  return 1;
}

/*-----------------------------------------------------------------------
 * ups_new_action
 *
 * Will create an empty t_upstyp_action structure
 *
 * Input : none
 * Output: none
 * Return: t_upstyp_action *, a pointer to an actions instance structure
 */
t_upstyp_action *ups_new_action( void )
{
  t_upstyp_action *act_ptr =
    (t_upstyp_action *)upsmem_malloc( sizeof( t_upstyp_action ) );
  
  (void) memset( act_ptr, 0, sizeof( t_upstyp_action ) );

  return act_ptr;
}

/*-----------------------------------------------------------------------
 * ups_free_action
 *
 * Will free an action.
 *
 * Input : t_upstyp_action *, a pointer to an action structure.
 * Output: none
 * Return: int, 0 on failer else 1.
 */
int ups_free_action( t_upstyp_action * const act_ptr )
{
  if ( !act_ptr ) return 0;

  if ( all_gone( act_ptr ) ) {
    if ( act_ptr->action ) { upsmem_free( act_ptr->action ); }
    if ( act_ptr->command_list ) { 
      act_ptr->command_list = upslst_free( act_ptr->command_list, 'd' );
    }

    upsmem_free( act_ptr );
  }

  return 1;
}

/*-----------------------------------------------------------------------
 * ups_new_config
 *
 * Will create an empty t_upstyp_config structure
 *
 * Input : none
 * Output: none
 * Return: t_upstyp_config *, a pointer to a config structure
 */
t_upstyp_config *ups_new_config( void )
{
  t_upstyp_config *conf_ptr =
    (t_upstyp_config *)upsmem_malloc( sizeof( t_upstyp_config ) );
  
  (void) memset( conf_ptr, 0, sizeof( t_upstyp_config ) );

  return conf_ptr;
}

/*-----------------------------------------------------------------------
 * ups_free_config
 *
 * Will free a config structure.
 *
 * Input : t_upstyp_config *, a pointer to a config structure.
 * Output: none
 * Return: int, 0 on failer else 1.
 */
int ups_free_config( t_upstyp_config * const conf_ptr )
{
  if ( !conf_ptr ) return 0;

  if ( all_gone( conf_ptr ) ) {
    if ( conf_ptr->ups_db_version ) { upsmem_free( conf_ptr->ups_db_version ); }
    if ( conf_ptr->prod_dir_prefix ) { upsmem_free( conf_ptr->prod_dir_prefix ); }
    if ( conf_ptr->authorized_nodes ) { upsmem_free( conf_ptr->authorized_nodes ); }
    if ( conf_ptr->statistics ) { upsmem_free( conf_ptr->statistics ); }
    if ( conf_ptr->man_target_dir ) { upsmem_free( conf_ptr->man_target_dir ); }
    if ( conf_ptr->catman_target_dir ) { upsmem_free( conf_ptr->catman_target_dir ); }
    if ( conf_ptr->info_target_dir ) { upsmem_free( conf_ptr->info_target_dir ); }
    if ( conf_ptr->html_target_dir ) { upsmem_free( conf_ptr->html_target_dir ); }
    if ( conf_ptr->news_target_dir ) { upsmem_free( conf_ptr->news_target_dir ); }

    upsmem_free( conf_ptr );
  }

  return 1;
}

/*
 * Definition of private functions
 */

int all_gone( const void * const ptr )
{
  return ( upsmem_get_refctr( ptr ) <= 0 );
}
