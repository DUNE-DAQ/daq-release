/************************************************************************
 *
 * FILE:
 *       ups_modify.c
 * 
 * DESCRIPTION: 
 *       This is the 'ups modify' command.
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
 *      Tue Feb 24, DjF, Starting...
 *
 ***********************************************************************/

/* standard include files */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

/* ups specific include files */
#include "ups.h"

/*
 * Definition of public variables.
 */
extern t_cmd_info g_cmd_info[];

/*
 * Declaration of private functions.
 */

int make_file_list( void );
int choose_file( void );
int edit_file( char * const file_name );
int verify_file( char * const file_name );

static void shutup(const FILE * const tmpfile, const int ups_command);

#define SHUTUP \
  if ((&bit_bucket+bit_bucket_offset == 0) && 0) shutup (tmpfile, ups_command);

/*
 * Definition of global variables.
 */

static long bit_bucket = 0;
static long bit_bucket_offset = 0;

#define DEFAULT_EDITOR "vi"
#define NOT_UNIQUE 1
#define MAX_FILES 50
#define QUIT -2
#define UNDEFINED -1

static t_upsugo_command  *g_uc = 0;
static t_upslst_item     *g_mp_list = 0;
static char              *g_file_list[ MAX_FILES ];
static int               g_file_count = 0;

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * ups_modify
 *
 * This is the main line for the 'ups modify' command.
 *
 * Input : argc, argv
 * Output: 
 * Return: 
 */
t_upslst_item *ups_modify( t_upsugo_command * const uc , 
                          const FILE * const tmpfile,
                          const int ups_command )
{
  int file_no = UNDEFINED;

  g_uc = (t_upsugo_command *)uc;
  UPS_VERIFY = 1;

  /* checkout passed instance, 
     we will flush the file cache, just to be sure that files in the cache only 
     is relevant to the passed instance */

  upsfil_flush();
  g_mp_list = upsmat_instance( g_uc, NULL , NOT_UNIQUE );

  /* make a list of files to be edited */

  (void) make_file_list();

  if ( g_file_count <= 0 ) {
    (void) fprintf( stdout, "No file matched argument.\n" );
    return g_mp_list;
  }

  /* if more than one file in list, only quit on request */

  do {
    file_no = choose_file();
    if ( file_no >= 0 )
      (void) edit_file( g_file_list[ file_no ] );
    else if ( file_no == QUIT ) 
      break;
  } while ( g_file_count > 1 );

  SHUTUP;

  return g_mp_list;
}


int choose_file( void )
{
  /* it will display the current file list (if more than one file) and let
     the user choose the file to be edited */

  int i;
  int num;
  char str[80];

  if ( g_file_count > 0 )
  {

    /* one file */

    if ( g_file_count == 1 ) 
      return 0;

    /* more than one file */

    (void) fprintf( stdout, "\n" );
    for ( i=0; i<g_file_count; i++ )
      (void) fprintf( stdout, "[%d] %s\n", i, g_file_list[i] );

    for (;;) {
      (void) fprintf( stdout,"Choose file to edit [%d-%d] or 'q' to quit: ", 
	       0, g_file_count - 1 );
      (void) fgets( str, 79, stdin );

      if ( strchr( str, 'q' ) || strchr( str, 'Q' ) )
        return QUIT;

      (void) sscanf( str, "%d", &num );

      if ( num >= 0 && num < g_file_count )
        return num;
    }
  }

  /* no files */

  return UNDEFINED;
}

int make_file_list( void )
{
  /* it will use the passed file name and the files in the file cache
     to create a list of available files:

     - if passed file is a file there exist, that will be the only
       available file.

     - if passed file does not exist, all files in the cache there 
       contain a string corresponding to the passed file will be
       available */

  int i;
  void **p_file_cache, **p_fc;
  char * file_name = (g_uc && g_uc->ugo_anyfile) ? g_uc->ugo_anyfile : 0;

  /* free current file list */

  for ( i = 0; i < g_file_count; i++ ) {
    if ( g_file_list[i] )
      free( g_file_list[i] );
    g_file_list[i] = 0;
  }
  g_file_count = 0;

  /* check out passed file, 
     and decide what to use it for */

  if ( file_name && (upsutl_is_a_file( file_name ) == UPS_SUCCESS) ) {

    /* we were passed an existing file, that will be the one */

    g_file_list[ g_file_count ] = malloc( strlen( file_name ) + 1 );
    (void) strcpy( g_file_list[ g_file_count ], file_name );
    g_file_count++;

    /* if file is not in cache, warn user */

    if ( !upsfil_is_in_cache( file_name  ) ) { 
      (void) fprintf( stdout, "WARNING - File specified is not in instance specified\n" );
      (void) fprintf( stdout, "can only check file format, not validity of instance.\n" );
    }
  }
  else {

    /* we will create a file list from the cache, using passed file name
       as a filter */
    
    p_file_cache = upstbl_to_array( g_file_cache(), 0 );

    for( p_fc = p_file_cache; p_fc && *p_fc; p_fc += 2 ) {

      if ( !file_name || strstr( (char *)(*p_fc), file_name ) ) { 
	g_file_list[ g_file_count ] = malloc( strlen( (char *)(*p_fc) ) + 1 );
	(void) strcpy( g_file_list[ g_file_count ], (char *)(*p_fc) );
	g_file_count++;
      }
    }

    if ( p_file_cache ) 
      free( p_file_cache );
  }

  return g_file_count;
}

int edit_file( char * const file_name )
{

  /* it will edit a file, using the editor defined by $EDITOR.
     it will check if a file actually has been modified,
     it will make a dated backup */     

  static char * s_editor = 0;

  char input[16];
  char buf[ FILENAME_MAX+1 ];
  char * file_org;
  struct stat fstat;
  time_t mtime = 0;
  char *bckup_ext = upsutl_str_create( upsutl_time_date( STR_TRIM_PACK ),
				       STR_TRIM_PACK);

  /* set editor */

  if ( !s_editor ) {
    s_editor = getenv( "EDITOR" );
    if ( s_editor == 0 || *s_editor == '\0' ) { 
      s_editor = DEFAULT_EDITOR; 
    }
  }

  /* verify file, before modifications */

  (void) verify_file( file_name );

  (void) fprintf( stdout, "Pre modification verification pass complete.\n" );

  if ( (file_org = tempnam(NULL,"ups")) != 0 ) {

    (void) sprintf( buf,"cp %s %s", file_name, file_org );

    /* make a backup */

    if ( !system( buf ) ) {      

      /* get current modification date */

      if ( stat( file_name, &fstat ) == 0 )
	mtime = fstat.st_mtime;

      /* edit file */
      
      (void) sprintf( buf, "%s %s", s_editor, file_name );

      if ( !system( buf ) ) {
 
	/* no modifications was done, just return */

	if ( (stat( file_name, &fstat ) == 0) && 
	     (fstat.st_mtime == mtime) ) {
	  (void) fprintf( stdout, "No modifications, nothing to save.\n" );
	  return 1;
	}

	/* modifications was done */

	/* verify file, after modifications */

	(void) verify_file( file_name );
	 
	(void) fprintf( stdout, "Post modification verification pass complete.\n" );

	(void) fprintf( stdout, "Do you wish to save this modification [y/n] ? " );
	(void)fgets( input, 3, stdin );

	if ( !upsutl_strincmp( input, "y", 1 ) ) { 

	  /* save modifications, make a date stamped backup file */

	  (void) sprintf( buf, "cp %s %s.%s",
		   file_org,
		   file_name,
		   bckup_ext);

	  if ( system( buf ) ) { 
	    (void) fprintf(stdout,"Cannot save dated file copy of %s.\n",
		    file_name);
	  }
	} 
	else {
	  
	  /* don't save modifications, restore file */

	  (void) sprintf( buf, "cp %s %s", file_org, file_name );

	  if ( system( buf ) ) { 
	    (void) fprintf(stdout,"Cannot restore original file %s copy in %s.\n",
		    file_name, file_org);
	    return -1;
	  }
	}
      }
      else { 

	/* starting editor failed */

	(void) fprintf( stdout, "Unable to edit file, check $EDITOR.\n" );
	return -1;
      }
    } 
    else { 

      /* making backup failed */

      (void) fprintf(stdout,"Unable to create temporary work space.\n");
      return -1;
    }
    /* free(file_org); */
  } 
  else { 

    /* creating tmp file failed */

    (void) fprintf(stdout,"Unable to generate temporary file name(tempnam).\n");
    return -1;
 }

  return 1;
}

int verify_file( char * const file_name )
{
  /* it will verify a file, using mainly the verification in 
     upsfil and ups_verify */

  t_upslst_item *l_mp;
  t_upslst_item *l_mi;
  t_upstyp_matched_product *mp;

  /* clear errors and file cache */

  upserr_clear();
  upsfil_flush();

  if ( upsfil_read_file( file_name ) ) {

    l_mp = upsmat_instance( g_uc, NULL , 0 );

    for ( ; l_mp; l_mp = l_mp->next) {

      mp = (t_upstyp_matched_product *)l_mp->data;
      l_mi = upslst_first( mp->minst_list );

      ups_verify_dbconfig( mp->db_info, 0, g_uc );

      for ( ; l_mi; l_mi = l_mi->next ) {

	ups_verify_matched_instance( mp->db_info,
				     (t_upstyp_matched_instance *)l_mi->data,
				     g_uc, 
				     mp->product);
      }

      /* display errors */

      upserr_output();
      upserr_clear();
    }
  }

  /* display errors */

  upserr_output();
  upserr_clear();

  (void) upsutl_free_matched_product_list( &l_mp );

  return ( UPS_ERROR == UPS_SUCCESS );
}

static void shutup(const FILE * const tmpfile, const int ups_command)
{
      bit_bucket ^= (long) tmpfile;
      bit_bucket ^= (long) ups_command;
}
