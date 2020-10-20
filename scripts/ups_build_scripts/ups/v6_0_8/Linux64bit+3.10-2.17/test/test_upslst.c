#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "upslst.h"
#include "upsmem.h"

static int sort_strcmp( const void * const d1, const void * const d2 )
{
  return strcmp( (const char *)d1, (const char *)d2 );
}

static void print_str_list( t_upslst_item * const list_ptr );

#define DATA_COUNT 10

int main( void )
{
  int i, d;
  char buf[32];
  char *data[DATA_COUNT];
  
  t_upslst_item *l_ptr1 = NULL;
  t_upslst_item *l_ptr2 = NULL;
  
  /*
   * Note:
   *
   * Calls to upslst_add/insert, with a passed list pointer equal null,
   * will create a new list, add the passed item and return a pointer
   * to the created list.
   *
   * Calls to upslst_free, with option 'd', will also delete the data
   * item.
   *
   * Calls to upslst_add, will add items as last item in the list
   * and return a pointer to last item.
   * Calls to upslst_insert, will add items as first item in the list
   * and return a pointer to first item.
   * In that way successive calls using previous return will be fast.  
   *
   * upslst_* only works with data elements created by upsmem_malloc.
   *
   */

  /* create some data */
  
  for ( i=0; i<DATA_COUNT; i++ ) {
    if ( i < DATA_COUNT/2 ) d = i;
    else d = 1000 - i;
    (void) sprintf( buf, "%d", d );
    data[i] = (char *)upsmem_malloc( (int)strlen( buf ) + 1 );
    (void) strcpy( data[i], buf );
  }

  (void) printf( "\nCreating a list" );

  for ( i=0; i<DATA_COUNT/2; i++ ) {
    (void) printf( "\nAdding as last %s\n", data[i] );
    l_ptr1 = upslst_add( l_ptr1, data[i]  );
    print_str_list( l_ptr1 );
  }
  
  (void) printf( "\nCreating another list" );

  for ( i=DATA_COUNT/2; i<DATA_COUNT; i++ ) {
    (void) printf( "\nAdding as last %s\n", data[i] );
    l_ptr2 = upslst_add( l_ptr2, data[i]  );
    print_str_list( l_ptr2 );
  }
  
  (void) printf( "\nSort the list\n" );
  (void) upslst_sort0( l_ptr2, sort_strcmp );
  print_str_list( l_ptr2 );
  
  (void) printf( "\nInserting lists\n" );
  l_ptr1 = upslst_first( l_ptr1 );
  l_ptr1 = upslst_insert_list( l_ptr1, l_ptr2 );
  (void) printf( "After list insert: p=%p, i=%p, n=%p, data=%s\n",
	  l_ptr1->prev, l_ptr1, l_ptr1->next, (char *)l_ptr1->data );    
  print_str_list( l_ptr1 );  

  for ( i=0; i<DATA_COUNT; i=i+2 ) {
    (void) printf( "\nDeleting %s\n", data[i] );
    l_ptr1 = upslst_delete( l_ptr1, data[i], 'd' );
    print_str_list( l_ptr1 );
  }
    
  (void) printf( "\nDeleting the rest\n" );
  l_ptr1 = upslst_free( l_ptr1, 'd' );
  print_str_list( l_ptr1 );

  return 0;
}

void print_str_list( t_upslst_item * const list_ptr )
{
  t_upslst_item *l_ptr;
  int count = 0;

  /*
   * Note use of upslst_first(), to be sure to start from first item
   */
  
  for ( l_ptr = upslst_first( list_ptr ); l_ptr; l_ptr = l_ptr->next, count++ ) {
    (void) printf( "%03d: p=%08x, i=%08x, n=%08x, data=%s\n",
	    count, (int)l_ptr->prev, (int)l_ptr,
	    (int)l_ptr->next, (char *)l_ptr->data );    
  }
}



