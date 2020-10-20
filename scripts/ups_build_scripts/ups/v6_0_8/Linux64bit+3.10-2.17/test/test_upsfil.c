#include <stdio.h>
#include <stdlib.h>
#include "upserr.h"
#include "upsfil.h"

int  main( const int argc, char * const argv[] )
{
  int i = 0;
  t_upstyp_product *prod_ptr = NULL;

  if ( argc <= 1 ) {
    (void) printf( "Usage: test_upsfil file_name\n" );
    exit( 1 );
  }

  for ( i=1; i<argc; i++ ) {
    
    prod_ptr = upsfil_read_file( argv[i] );  

    if ( prod_ptr ) {
      /*      g_print_product( prod_ptr ); */
      (void) upsfil_write_file( prod_ptr, "ups.out",' ' ,NOJOURNAL);
      (void) ups_free_product( prod_ptr );
    }
  }
  
  upserr_output();
  return 0;
}


