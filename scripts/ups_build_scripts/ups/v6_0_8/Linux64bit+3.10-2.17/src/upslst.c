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
 *       23-jul-1997, LR, first
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
 *                        with upslst_merge
 *                        Added upslst_copy, which will create a new list
 *                        containing data elements from passed list.
 *       09-sep-1997, LR, Added upslst_count, to return number of items in
 *                        a list.
 *       10-sep-1997, LR, Added upslst_delete_safe, to delete items in a safe
 *                        way (can be used inside a list iteration).
 *       10-sep-1997, LR, Added upslst_sort0, a simple (insertion) sort of a
 *                        list.
 *
 ***********************************************************************/

/* standard include files */
#include <stdlib.h>

/* ups specific include files */
#include "upslst.h"
#include "upsmem.h"

/*
 * Definition of public variables.
 */

/*
 * Declaration of private functions.
 */

/*
 * Definition of global variables.
 */

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * upslst_new
 *
 * Will create a new ups list and add first item.
 *
 * Input : void *, a data element
 * Output: none
 * Return: t_upslst_item *, pointer to top of list or 0
 */
t_upslst_item *upslst_new( void * const data_ptr )
{
  t_upslst_item *l_first = 0;

  l_first = (t_upslst_item *)malloc( sizeof( t_upslst_item ) );

  if ( !l_first ) return 0;
  
  l_first->data = data_ptr;
  upsmem_inc_refctr( data_ptr );
  
  l_first->prev = 0;
  l_first->next = 0;

  return( l_first );
}	

/*-----------------------------------------------------------------------
 * upslst_free
 *
 * Will free up all memory for ups list.
 * If option 'd' is passed the data elements are also freed.
 *
 * Input : t_upslst_item *, pointer to a list.
 *         char, copt = 'd', will also delete data elements.
 * Output: none
 * Return: t_upslst_item *, 0
 */
t_upslst_item *upslst_free( t_upslst_item * const list_ptr, const char copt )
{
  t_upslst_item *l_ptr = 0;
  t_upslst_item *l_tmp = 0;
  t_upslst_item *l_first = upslst_first( list_ptr );

  if ( !l_first ) return 0;

  l_ptr = l_first;
  while( l_ptr ) {
    upsmem_dec_refctr( l_ptr->data );    
    if ( copt == 'd' && l_ptr->data ) {
      upsmem_free ( l_ptr->data );
    }
    l_tmp = l_ptr;
    l_ptr = l_ptr->next;
    if ( l_ptr ) l_ptr->prev = 0;
    free( l_tmp );
  }

  return 0;
}

/*-----------------------------------------------------------------------
 * upslst_insert
 *
 * Will add an item as the first item to the list.
 *
 * Input : t_upslst_item *, pointer to a list, if 0 a new list will
 *         be created.
 *         void *, data element for list.
 * Output: none
 * Return: t_upslst_item *, pointer to the first item of the list or 0
 *
 * NOTE: if passed list pointer, in successive calls, is the first item,
 * it should be pretty fast.
 */
t_upslst_item *upslst_insert( t_upslst_item * list_ptr, void * const data_ptr )
{
  t_upslst_item *l_first = 0;
  t_upslst_item *l_new = 0;

  if ( ! list_ptr ) {
    list_ptr = upslst_new( data_ptr );
    return list_ptr;
  }

  l_first = upslst_first( list_ptr );

  l_new = (t_upslst_item *)malloc( sizeof( t_upslst_item ) );

  if ( !l_new ) return 0;

  l_new->data = data_ptr;
  upsmem_inc_refctr( data_ptr );  
  
  l_first->prev = l_new;
  l_new->next = l_first;
  l_new->prev = 0;

  return l_new;
}

/*-----------------------------------------------------------------------
 * upslst_add
 *
 * Will add an item as the last item to the list.
 *
 * Input : t_upslst_item *, pointer to a list, if 0 a new list will
 *         be created.
 *         void *, data element for list.
 * Output: none
 * Return: t_upslst_item *, pointer to the last item of the list or 0
 *
 * NOTE: if passed list pointer, in successive calls, is the last item,
 * it should be pretty fast.
 */
t_upslst_item *upslst_add( t_upslst_item *list_ptr, void * const data_ptr )
{
  t_upslst_item *l_last = 0;
  t_upslst_item *l_new = 0;

  if ( ! list_ptr ) {
    list_ptr = upslst_new( data_ptr );
    return list_ptr;
  }
  
  l_last = upslst_last( list_ptr );

  l_new = (t_upslst_item *)malloc( sizeof( t_upslst_item ) );

  if ( !l_new ) return 0;

  l_new->data = data_ptr;
  upsmem_inc_refctr( data_ptr );
  
  l_last->next = l_new;
  l_new->prev = l_last;
  l_new->next = 0;

  return l_new;
}

/*-----------------------------------------------------------------------
 * upslst_delete
 *
 * Will delete item from list.
 * If option 'd' is passed the data element are also freed.
 *
 * Input : t_upslst_item *, pointer to a list
 *         void *, data element of item to be deletet.
 *         char, copt = 'd', will also delete data elements.
 * Output: none
 * Return: t_upslst_item *, pointer to top of list
 */
t_upslst_item *upslst_delete( t_upslst_item * const list_ptr,
			      void * const data_ptr, const char copt )
{
  t_upslst_item *l_ptr = 0;
  t_upslst_item *l_prev = 0;
  t_upslst_item *l_first = 0;

  l_first = upslst_first( list_ptr );
  
  for ( l_ptr = l_first; l_ptr; l_ptr = l_ptr->next ) {
    l_prev = l_ptr->prev;

    if ( l_ptr->data == data_ptr ) {
      if ( l_ptr->next ) {
	l_ptr->next->prev = l_prev;
	if ( l_prev ) l_prev->next = l_ptr->next;
      }
      else {
	if ( l_prev ) l_prev->next = 0;
      }
      upsmem_dec_refctr ( l_ptr->data );
      if ( copt == 'd' && l_ptr->data ) {
	upsmem_free( l_ptr->data );
      }
      if ( l_ptr == l_first ) {
	l_first = l_ptr->next;
      }
      free( l_ptr );
      return l_first;
    }
  }

  /* item was not found */
  return 0;
}

/*-----------------------------------------------------------------------
 * upslst_delete_safe
 *
 * Will delete item from list. And return next item in list.
 * (it's safe to use in a loop there is traversing a list).
 * If option 'd' is passed the data element are also freed.
 *
 * Input : t_upslst_item *, pointer to a list
 *         void *, data element of item to be deletet.
 *         char, copt = 'd', will also delete data elements.
 * Output: none
 * Return: t_upslst_item *, pointer to top of list
 */
t_upslst_item *upslst_delete_safe( t_upslst_item * const list_ptr,
			      void * const data_ptr, const char copt )
{
  t_upslst_item *l_ptr = 0;
  t_upslst_item *l_prev = 0, *l_next = 0;

  for ( l_ptr = list_ptr; l_ptr; l_ptr = l_ptr->next ) {
    l_prev = l_ptr->prev;

    if ( l_ptr->data == data_ptr ) {
      if ( l_ptr->next ) {
	l_ptr->next->prev = l_prev;
	if ( l_prev ) l_prev->next = l_ptr->next;
      }
      else {
	if ( l_prev ) l_prev->next = 0;
      }
      upsmem_dec_refctr ( l_ptr->data );
      if ( copt == 'd' && l_ptr->data ) {
	upsmem_free( l_ptr->data );
      }
      l_next = l_ptr->next;
      free( l_ptr );
      return l_next;
    }
  }

  /* item was not found */
  return 0;
}

/*-----------------------------------------------------------------------
 * upslst_add_list
 *
 * Will append second list to the end of first list.
 * If passed first list is null, it will return first list element of
 * second list.
 * If passed second list is null, it will return first list item of
 * first list.
 *
 * Input : t_upslst_item *, pointer to first list.
 *         t_upslst_item *, pointer to second list to be appended.
 * Output: none
 * Return: t_upslst_item *, pointer to the first item of list or 0
 */
t_upslst_item *upslst_add_list( t_upslst_item * const list_ptr_1,
				t_upslst_item * const list_ptr_2 )
{
  t_upslst_item *l_last_1 = 0;
  t_upslst_item *l_first_2 = 0;

  if ( !list_ptr_1 )
    return upslst_first( list_ptr_2 );
  else if ( !list_ptr_2 ) 
    return upslst_first( list_ptr_1 );
  
  l_last_1 = upslst_last( list_ptr_1 );
  l_first_2 = upslst_first( list_ptr_2 );

  l_last_1->next = l_first_2;
  l_first_2->prev = l_last_1;
  
  return upslst_first( list_ptr_1 );
}

/*-----------------------------------------------------------------------
 * upslst_insert_list
 *
 * Will append second list to passed list element.
 * If passed first list is null, it will return last list element of
 * second list.
 * If passed second list is null, it will return passed first list element.
 *
 * Input : t_upslst_item *, pointer to item where second list will be appended.
 *         t_upslst_item *, pointer to second list to be appended.
 * Output: none
 * Return: t_upslst_item *, pointer to last element of second list or 0.
 */
t_upslst_item *upslst_insert_list( t_upslst_item * const list_ptr_1,
				   t_upslst_item * const list_ptr_2 )
{
  t_upslst_item *l_next_1 = 0;
  t_upslst_item *l_first_2 = 0;
  t_upslst_item *l_last_2 = 0;

  if ( !list_ptr_1 )
    return upslst_last( list_ptr_2 );
  else if ( !list_ptr_2 )
    return list_ptr_1;
  
  l_next_1 = list_ptr_1->next;
  l_first_2 = upslst_first( list_ptr_2 );
  l_last_2 = upslst_last( list_ptr_2 );
    
  list_ptr_1->next = l_first_2;
  l_first_2->prev = list_ptr_1;
    
  l_last_2->next = l_next_1;
  if ( l_next_1 ) l_next_1->prev = l_last_2;
  
  return l_last_2;
}

/*-----------------------------------------------------------------------
 * upslst_copy
 *
 * Will create a new list, contaning data elements from the passed list.
 * The 'copied' data elements will have their ref count incremented.
 *
 * Input : t_upslst_item *, pointer to a list.
 * Output: none
 * Return: t_upslst_item *, pointer to the first item of the list or 0
 */
t_upslst_item *upslst_copy( t_upslst_item * const list_ptr )
{
  t_upslst_item *l_ptr = 0;
  t_upslst_item *l_new = 0;
  
  if ( !list_ptr )
    return 0;
  
  l_ptr = upslst_first( list_ptr );
  for ( ; l_ptr; l_ptr = l_ptr->next ) {
    l_new = upslst_add( l_new, l_ptr->data );
  }

  return upslst_first( l_new );
}

/*-----------------------------------------------------------------------
 * upslst_first
 *
 * Will return first item of passed list.
 *
 * Input : t_upslst_item *, pointer to a list
 * Output: none
 * Return: t_upslst_item *, pointer to first item of list
 */
t_upslst_item *upslst_first( t_upslst_item * const list_ptr )
{
  t_upslst_item *l_ptr = 0;
  
  if ( !list_ptr ) return 0;

  for ( l_ptr = list_ptr; l_ptr->prev; l_ptr = l_ptr->prev ) {}

  return l_ptr;
}

/*-----------------------------------------------------------------------
 * upslst_last
 *
 * Will return last item of passed list.
 *
 * Input : t_upslst_item *, pointer to a list
 * Output: none
 * Return: t_upslst_item *, pointer to last item of list
 */
t_upslst_item *upslst_last( t_upslst_item * const list_ptr )
{
  t_upslst_item *l_ptr = 0;
  
  if ( !list_ptr ) return 0;

  for ( l_ptr = list_ptr; l_ptr->next; l_ptr = l_ptr->next ) {}

  return l_ptr;
}

/*-----------------------------------------------------------------------
 * upslst_count
 *
 * Will return number of items in a list.
 *
 * Input : t_upslst_item *, pointer to a list
 * Output: none
 * Return: int, number of items in a list
 */
int upslst_count( t_upslst_item * const list_ptr )
{
  int count = 0;
  t_upslst_item *l_ptr = upslst_first( list_ptr );
  
  for ( ; l_ptr; l_ptr = l_ptr->next ) { count++; }

  return count;
}

/*-----------------------------------------------------------------------
 * upslst_find
 *
 * Find a list element.
 *
 * Input : t_upslst_item *, pointer to a list
 *         void *, pointer to the data element to find
 *         int *, pointer to a function to be called for
 *                comparing list items.
 * Output: none
 * Return: t_upslst_item *, pointer to found list element or 0.
 */
t_upslst_item *upslst_find( t_upslst_item * const list_ptr, 
			    const void * const data,
			    int (* const cmp)(const void * const, const void * const) )
{
  t_upslst_item *l_ptr = upslst_first( list_ptr );

  for ( ; l_ptr; l_ptr = l_ptr->next ) {
    if ( !cmp( data, l_ptr->data ) )
      return l_ptr;	 
  }
  
  return 0;
}

/*-----------------------------------------------------------------------
 * upslst_sort0
 *
 * Sorting a list. This is just a insertion sort. We should implement
 * something better if we need to sort large (>20 items) lists.
 *
 * Input : t_upslst_item *, pointer to a list
 *         int *, pointer to a function to be called for
 *                comparing list items.
 * Output: none
 * Return: t_upslst_item *, pointer to first element in list.
 */
t_upslst_item *upslst_sort0( t_upslst_item * const list_ptr,
			     int (* const cmp)(const void * const, const void * const) )
{
  t_upslst_item *l1 = upslst_first( list_ptr ), *l2 = 0;
  void *data;

  if ( !l1 || !l1->next )
    return l1;
  
  for ( l1 = l1->next; l1; l1 = l1->next ) {
    /* invariant: list up through l1->prev is sorted */
    data = l1->data;
    l2 = l1;
    while ( l2->prev && cmp( l2->prev->data, data ) > 0 ) {
      l2->data = l2->prev->data;
      l2 = l2->prev;
    }
    l2->data = data;
  }

  return upslst_first( l2 );
}

t_upslst_item *upslst_uniq( t_upslst_item * const list_ptr,
			     int (* const cmp)(const void * const, const void * const) )
{
  t_upslst_item *l1 = upslst_first( list_ptr ), *l2 = 0, *ltmp=0;

  if (!l1 || !l1->next) 
     return l1;

  l2 = l1;
  l1 = l1->next;
  while( l1 ) {
    if (0 == cmp(l1->data, l1->prev->data)) {
      l1->prev->next = l1->next;
      if (l1->next) {
          l1->next->prev = l1->prev;
      }
      l1 = l1->next;
      ltmp = l1;
#if 0
      if (ltmp) { 
        if (ltmp->data)
          upsmem_free ( ltmp->data ); 
        free(ltmp);
      }
#endif
    } else {
      l1 = l1->next;
    }
  }
  return upslst_first( l2 );
}

/*
 * Definition of private functions.
 */







