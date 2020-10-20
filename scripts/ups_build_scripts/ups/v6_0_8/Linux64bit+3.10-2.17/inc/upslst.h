/************************************************************************
 *
 * FILE:
 *       upslst.c
 * 
 * DESCRIPTION: 
 *       A double linked list (ideas from old ups_list).
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
 *       23-Jun-1997, LR, first
 *       25-jul-1997, LR, upslst_add is now as fast as upslst_insert:
 *                        upslst_add will now return the last element.
 *                        upslst_insert will return the first element.
 *                        In that way, successive calls using previous
 *                        return will be fast.  
 *                        Changed 'bot' to 'last' and 'top' to 'first'.
 *       30-jul-1997, LR, Added functions to add list to list,
 *                        upslst_add_list, upslst_insert_list.
 *       31-jul-1997, LR, Added use of upsmem, to allocate/free memory.
 *                        Note: all list data elements should be allocated
 *                        by upsmem_alloc.
 *                        Replaced upslst_add_list and upslst_insert_list
 *                        with upslst_merge.
 *                        Added upslst_copy, which will create a new list
 *                        with data elements from passed list.
 *       09-sep-1997, LR, Added upslst_count, to return number of items in
 *                        a list.
 *       10-sep-1997, LR, Added upslst_delete_safe, to delete items in a safe
 *                        way (can be used inside a list iteration).
 *       10-sep-1997, LR, Added upslst_sort0, a simple (insertion) sort of a
 *                        list.
 *
 ***********************************************************************/

#ifndef _UPSLST_H_
#define _UPSLST_H_

/* standard include files, if needed for .h file */

/* ups specific include files, if needed for .h file */

/*
 * Constans.
 */

/*
 * Types.
 */
typedef struct upslst_item
{
  struct upslst_item    *prev;
  void                  *data;
  struct upslst_item    *next;
} t_upslst_item;

/*
 * Declaration of public functions.
 */
t_upslst_item *upslst_new( void * const data_ptr );
t_upslst_item *upslst_free( t_upslst_item * const list_ptr, const char copt );
t_upslst_item *upslst_insert( t_upslst_item * list_ptr, void * const data_ptr );
t_upslst_item *upslst_add( t_upslst_item * list_ptr, void * const data_ptr );
t_upslst_item *upslst_delete( t_upslst_item * const list_ptr,
			      void * const data_ptr, const char copt );
t_upslst_item *upslst_delete_safe( t_upslst_item * const list_ptr,
			      void * const data_ptr, const char copt );

t_upslst_item *upslst_add_list( t_upslst_item * const list_ptr_1,
				t_upslst_item * const list_ptr_2 );
t_upslst_item *upslst_insert_list( t_upslst_item * const list_ptr_1,
				   t_upslst_item * const list_ptr_2 );
t_upslst_item *upslst_copy( t_upslst_item * const list_ptr );

t_upslst_item *upslst_first( t_upslst_item * const list_ptr );
t_upslst_item *upslst_last( t_upslst_item * const list_ptr );
t_upslst_item *upslst_find( t_upslst_item * const list_ptr, 
			    const void * const data,
			    int (* const cmp)(const void * const, const void * const) );
int           upslst_count( t_upslst_item * const list_ptr );
t_upslst_item *upslst_sort0( t_upslst_item * const c,
			     int (* const cmp)(const void * const, const void * const) );
t_upslst_item *upslst_uniq( t_upslst_item * const c,
			     int (* const cmp)(const void * const, const void * const) );
     
/*
 * Declaration of private globals.
 */

/*
 * Declarations of public variables.
 */

#endif /* _UPSLST_H_ */






