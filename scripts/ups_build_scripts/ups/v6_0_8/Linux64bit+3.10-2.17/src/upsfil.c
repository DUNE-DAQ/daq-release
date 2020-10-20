/************************************************************************
 *
 * FILE:
 *       upsfil.c
 * 
 * DESCRIPTION: 
 *       Will read an ups file, and fill corresponding data structures.
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
 *       23-Jun-1997, LR, first.
 *       31-Jul-1997, LR, Added use of upsmem.
 *       12-Aug-1997, LR, Added upsfil_write.
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>				/* PATH_MAX */

/* ups specific include files */
#include "upsutl.h"
#include "upstyp.h"
#include "upslst.h"
#include "upstbl.h"
#include "upsmem.h"
#include "upserr.h"
#include "upskey.h"
#include "upsfil.h"
#include "upsver.h"
#include "upsget.h"

extern t_upskey_map g_key_info[];

/*
 * Definition of public variables
 */

/*
 * Definition of public variables.
 */

extern int UPS_VERBOSE;
extern int UPS_VERIFY;

/* table of seeds for finding groups in table files, 
   yep, it's ugly */

enum {
  e_group_count = 9,
  e_group_items = 7
};
static int g_groups[ e_group_count ][ e_group_items ] = 
{ 
  {
    e_key_action, -1, -1, -1, -1, -1, -1
  },
  {
    e_key_description, e_key_man_source_dir, e_key_catman_source_dir, 
    e_key_html_source_dir, e_key_info_source_dir, e_key_news_source_dir, -1
  },
  {
    e_key_man_source_dir, e_key_catman_source_dir, e_key_html_source_dir,
    e_key_info_source_dir, e_key_news_source_dir, -1, -1
  },
  {
    e_key_description, -1, -1, -1, -1, -1, -1
  },
  {
    e_key_man_source_dir, -1, -1, -1, -1, -1, -1
  },
  {
    e_key_catman_source_dir, -1, -1, -1, -1, -1, -1
  },
  {
    e_key_html_source_dir, -1, -1, -1, -1, -1, -1
  },
  {
    e_key_info_source_dir, -1, -1, -1, -1, -1, -1
  },
  {
    e_key_news_source_dir, -1, -1, -1, -1, -1, -1
  },
};

static char g_subdir_buf[PATH_MAX];
static const char *g_subdir_base;
/*
 * Declaration of private functions
 */

/* reading */
static int               read_file( void );
static int               read_file_type( void );
static int               read_file_desc( void );
static t_upstyp_instance *read_instance( void );
static t_upslst_item     *read_instances( void );
static t_upstyp_instance *read_common( void );
static t_upstyp_action   *read_action( void );
static t_upslst_item     *read_actions( void );
static t_upstyp_config   *read_config( void );
static t_upslst_item     *read_group( void );
static t_upslst_item     *read_groups( void );
static t_upslst_item     *read_comments( void );

/* writing */
static int               write_version_file( void );
static int               write_chain_file( void );
static int               write_table_file( void );
static int               write_action( t_upstyp_action * const act );
static int               put_inst_keys( int * keys,
					t_upstyp_instance * const inst,
					int do_act );
static int               put_inst_keys_mask( int * ikeys,
					     t_upstyp_instance * const inst,
					     int * ymask,
					     int * nmask);
static int               put_head_keys( int * keys );
static int               put_key( const char * const key, const char * const val );
static int               put_group( t_upslst_item * const l_inst,
				    int * const keys,
				    int * const common_mask );

/* and arithmetic */
static t_upslst_item     *find_group( t_upslst_item * const list_ptr, 
				      const char copt,
				      int * const ikeys );
static int               *find_common_mask_list( t_upslst_item *l_inst,
						 int * const ikeys );
static int               *find_common_mask( t_upstyp_instance * const inst1,
					    t_upstyp_instance * const inst2,
					    int * const ikeys );

/* line parsing */
static int               find_start_key( void );
static int               get_key( void );
static int               next_key( int ignore_unknown );
static int               is_stop_key( void );
static int               is_start_key( void );

/* verifying */
static void              verify_keys( t_upslst_item * l_ptr, 
				      t_upstyp_instance * inst_ptr );
static void              verify_groups( t_upslst_item * l_ptr,
					t_upslst_item * n_ptr);

/* print stuff */
static void              print_instance( t_upstyp_instance * const inst_ptr );
static void              print_action( t_upstyp_action * const act_ptr );
/* print_product has gone semi public */

/* utils */
void                     trim_cache( void );
static int               trim_qualifiers( char * const str );
static int               cfilei( void );
t_upslst_item            *add_to_action_list( t_upslst_item * const add_to,
					    t_upslst_item * const add_from );
int                      add_to_instance( t_upstyp_instance * const inst,
					  t_upstyp_instance * const inst_add );

int g_subdir_files_flag = 0;
/*
 * Definition of global variables
 */

/* enum of some extra keys */
enum {
  e_key_eol = -2,
  e_key_eof = -1
};

#define WCHARS " \t\n\r\f"
#define QUOTES "\"\'"
#define WCHARSQ " \t\n\r\f\"\'"

#define SEPARATION_LINE "#*************************************************\n#"

static t_upstyp_product  *g_pd = 0; /* current product to fill */
static FILE              *g_fh = 0; /* file handle */

static char              g_line[MAX_LINE_LEN] = "";  /* current line */
static char              g_key[MAX_LINE_LEN] = "";   /* current key */
static char              g_val[MAX_LINE_LEN] = "";   /* current value */
static char              *g_tval = 0;                /* current translated value */
static t_upskey_map      *g_mkey = 0;                /* current map */
static int               g_ikey = e_key_unknown;     /* current key as enum */  
static int               g_ifile = e_file_unknown;   /* current file type as enum */

static int               g_imargin = 0;
static char              g_filename[MAX_LINE_LEN] = "";
static int               g_item_count = 0;
static int               g_line_count = 0;

static int               g_use_cache = 1;            /* turn on/off use of cache */
static t_upstbl          *g_ft = 0;                  /* pointer to file cache */
static int               g_call_cache_count = 0;     /* # times cache is used */
static int               g_call_count = 0;           /* # times read_file is called */

#define P_VERB( iver, str ) \
  if( UPS_VERBOSE ) upsver_mes( iver, "UPSFIL: %s\n", \
			        (str))

#define P_VERB_s( iver, str ) \
  if( UPS_VERBOSE ) upsver_mes( iver, "UPSFIL: %s - %s\n", \
			       g_filename, (str))
  
#define P_VERB_s_i( iver, str1, num ) \
  if( UPS_VERBOSE ) upsver_mes( iver, "UPSFIL: %s - %s %d\n", \
			       g_filename, (str1), (num) )

#define P_VERB_s_i_s( iver, str1, num, str2 ) \
  if( UPS_VERBOSE ) upsver_mes( iver, "UPSFIL: %s - %s %d %s\n", \
			       g_filename, (str1), (num), (str2))

#define P_VERB_s_i_s_nn( iver, str1, num, str2 ) \
  if( UPS_VERBOSE ) upsver_mes( iver, "UPSFIL: %s - %s %d %s", \
			       g_filename, (str1), (num), (str2))

#define P_VERB_s_i_s_s_s( iver, str1, num, str2, str3, str4 ) \
  if( UPS_VERBOSE ) upsver_mes( iver, "UPSFIL: %s - %s %d %s %s %s\n", \
			       g_filename, (str1), (num), (str2), (str3), (str4))


/* fix up qualifier strings when making filenames */

char *
without_colons( char *s ) {
    static char colonbuf[MAX_LINE_LEN];
    char *r = colonbuf;
    /* printf("without_colons( %s ) -> ", s); debug line */
    if (!s) 
        return "";
    while (0 != (*r = *s)) {
        if (':' == *r) {
            *r = '_';
        }
        r++;
        s++;
    }
    /* printf("%s\n", colonbuf); */
    return colonbuf;
}
/*
 * Definition of public functions
 */

/* 
** upsfil_read_dir(path):
** call upsfil_read_file for each item in directory...
*/
t_upstyp_product *upsfil_read_dir( const char * const ups_dir ) {
  int dlen;
  static char namebuf[PATH_MAX];
  DIR *dd;
  struct dirent *de;
  int res = 1;
  int count = 0;

  dlen=strlen(ups_dir)+1;
  strcpy(namebuf,ups_dir);
  strcat(namebuf,"/");


  dd = opendir(ups_dir);

  g_pd = ups_new_product();

  if ( g_pd ) {       
    while (dd && 0 != (de = readdir(dd))) {
       if (de->d_name[0] != '#' && de->d_name[0] != '.' && de->d_name[strlen(de->d_name)-1] != '~' && 0 != strcmp(de->d_name, "CVS")) {
          count = count + 1;
	  namebuf[dlen] = 0;
	  strcat(namebuf, de->d_name);
	  g_fh = fopen ( namebuf, "r" );
	  if ( ! g_fh ) {
	    P_VERB_s( 1, "Open file for read ERROR" );
	    upserr_vplace();
	    upserr_add( UPS_SYSTEM_ERROR, UPS_FATAL, "fopen", strerror(errno));
	    if (errno == ENOENT)
	       upserr_add( UPS_NO_FILE, UPS_FATAL, namebuf );
	    else
	      upserr_add( UPS_OPEN_FILE, UPS_FATAL, namebuf );
            closedir(dd);
	    return 0;
	  }
	  P_VERB_s( 1, "Open file for read" );
          res &= read_file();
          if (g_fh) (void) fclose( g_fh );
          g_fh = 0;
  
       }
    }
  }
  if (!res || count == 0) {
    (void) ups_free_product( g_pd );
    g_pd = 0;
    g_item_count = 0;
  }
  closedir(dd);

  return g_pd;
}

/*-----------------------------------------------------------------------
 * upsfil_read_file
 *
 * Will read all instances in passed file
 *
 * Input : char *, path to a ups file
 * Output: none
 * Return: t_upstyp_product *, a pointer to a product
 */
t_upstyp_product *upsfil_read_file( const char * const ups_file )
{
  const char *key = 0;
  int res;
  struct stat statbuf;
  

  UPS_ERROR = UPS_SUCCESS;
  (void) strcpy( g_filename, ups_file );
  g_item_count = 0;
  g_line_count = 0;

  if ( !ups_file || strlen( ups_file ) <= 0 ) {
    upserr_vplace(); upserr_add( UPS_OPEN_FILE, UPS_FATAL, "" );
    return 0;
  }

  g_call_count++;


  /* if we use cache, check if file is in cache */

  if ( g_use_cache ) {
    if ( !g_ft )
      g_ft = upstbl_new( 300 );

    key = upstbl_atom_string( ups_file );
    g_pd = upstbl_get( g_ft, key );

    if ( g_pd ) {
      g_call_cache_count++;
      P_VERB_s_i_s( 1, "Read from cache", 
		    upslst_count( g_pd->instance_list), "item(s)" );
      return g_pd;
    }
  }

  res = stat(ups_file, &statbuf);
  if (res == 0 && S_ISDIR(statbuf.st_mode)) {
    upsfil_read_dir(ups_file);

    /* add product to cache */

    if ( g_ft && g_pd )
      (void) upstbl_put( g_ft, key, g_pd );
    return g_pd;
   }

  /* product is not in cache */

  g_fh = fopen ( ups_file, "r" );
  if ( ! g_fh ) {
    P_VERB_s( 1, "Open file for read ERROR" );
    upserr_vplace();
    upserr_add( UPS_SYSTEM_ERROR, UPS_FATAL, "fopen", strerror(errno));
    if (errno == ENOENT)
      upserr_add( UPS_NO_FILE, UPS_FATAL, ups_file );
    else
      upserr_add( UPS_OPEN_FILE, UPS_FATAL, ups_file );
    return 0;
  }
  P_VERB_s( 1, "Open file for read" );
  
  /* read the file and fill the product structure */

  g_pd = ups_new_product();
  if ( g_pd ) {       
    if ( !read_file() ) {
      
      /* file was empty */

      upserr_vplace(); upserr_add( UPS_READ_FILE, UPS_WARNING, ups_file );
      
      (void) ups_free_product( g_pd );
      g_pd = 0;
      g_item_count = 0;
    }
  }
  
  if (g_fh) (void)fclose( g_fh );
  g_fh = 0;

  /* add product to cache */

  if ( g_ft && g_pd )
    (void) upstbl_put( g_ft, key, g_pd );

  P_VERB_s_i_s( 1, "Read", g_item_count, "item(s)" );


  return g_pd;
}




void write_journal_file( const void *key, void ** prod, void *cl ) 
{
  int res;
  DIR *dd;
  struct stat statbuf;
  struct dirent *de;
  /* a little helper for upsfil_write_journal_files */

  /* we will check that the product list is not empty before writing
     the file. the reason is that upsfil_write_file will, for an empty
     product, remove existing files and remove the product from the cache,
     upstbl_map will produce a fatal error if table is been modified
     during a traverse */

  t_upstyp_product *p = (t_upstyp_product *)*prod;

  if ( p->journal == JOURNAL ) {
    if ( upslst_count( p->instance_list ) <= 0 ) {

      /* free product, and set prod pointer to zero for later removal 
         from cache (by a call to trim_cache) */

      (void) ups_free_product( *prod );
      *prod = 0;

      /* remove file */

      P_VERB_s( 1, "Removing empty journal file" );

      res = stat(key, &statbuf);
      if (0 == res && S_ISDIR(statbuf.st_mode)) {
	dd = opendir(key);
	while (dd && 0 != (de = readdir(dd))) {
           if (de->d_name[0] != '#' && de->d_name[0] != '.' && de->d_name[strlen(de->d_name)-1] != '~'&& 0 != strcmp(de->d_name, "CVS")) {
	      sprintf(g_subdir_buf, "%s/%s", (char *)key, de->d_name);
	      remove(g_subdir_buf);
	   }
	}
        closedir(dd);
      }

      if (remove( key ) != 0)
      {
        P_VERB_s( 1, "Removing journal file ERROR" );
        upserr_add( UPS_SYSTEM_ERROR, UPS_FATAL, "remove", strerror(errno));
        upserr_vplace(); upserr_add( UPS_REMOVE_FILE, UPS_FATAL, key );
        return;
      }
    }
    else {
      (void) upsfil_write_file( p, key, 'd', NOJOURNAL );
    }
  }

  {                             /* Bogus block that pretends to use unused arguments */
    static long bit_bucket;
    if (bit_bucket == 0)
    {
      bit_bucket ^= (long) cl;
    }
  }
  
}

/*-----------------------------------------------------------------------
 * upsfil_write_journal_files
 *
 * Will write all journal files in cache to disk.
 *
 * Input : none
 * Output: none
 * Return: int, UPS_SUCCESS just fine, else UPS_<error> number.
 */
int upsfil_write_journal_files( void )
{
  
  static char old_filename[MAX_LINE_LEN];
  (void) strcpy( old_filename, g_filename );

  if ( g_ft ) {
    P_VERB( 1, "Writing ALL journal files" );
    upstbl_map( g_ft, write_journal_file, NULL );
  }
  trim_cache();

  (void) strcpy( g_filename, old_filename );

  return UPS_SUCCESS;
}

void clear_journal_file( const void *key, void ** prod, void *cl ) 
{
  /* a little helper for upsfil_clear_journal_files */
  t_upstyp_product *p = (t_upstyp_product *)*prod;
  p->journal = NOJOURNAL;

  {                             /* Bogus block that pretends to use unused arguments */
    static long bit_bucket;
    if (bit_bucket == 0)
    {
      bit_bucket ^= (long) cl;
      bit_bucket ^= (long) key;
    }
  }

}
/*-----------------------------------------------------------------------
 * upsfil_clear_journal_files
 *
 * Will clear the journal flag for all products in the cache
 *
 * Input : none
 * Output: none
 * Return: int, UPS_SUCCESS just fine, else UPS_<error> number.
 */
int upsfil_clear_journal_files( void )
{
  if ( g_ft ) {
    P_VERB( 1, "Clearing journal files" );
    upstbl_map( g_ft, clear_journal_file, NULL );
  }

  return UPS_SUCCESS;
}

void
start_file(const char *ups_file) {
  t_upslst_item *l_ptr = 0;

  if ( ! g_fh ) {
    P_VERB_s( 1, "Open file for write ERROR" );
    upserr_add( UPS_SYSTEM_ERROR, UPS_FATAL, "fopen", strerror(errno));
    upserr_vplace(); upserr_add( UPS_OPEN_FILE, UPS_FATAL, ups_file );
    return;
  }    
  P_VERB_s( 1, "Open file for write" );

  g_imargin = 0;
    
  /* write comments */

  l_ptr = upslst_first( g_pd->comment_list );
  for( ; l_ptr; l_ptr = l_ptr->next ) {
    (void) fprintf( g_fh, "%s\n", (char *)l_ptr->data );
  }    

}

/*-----------------------------------------------------------------------
 * upsfil_write_file
 *
 * Will write a product to a ups file
 *
 * Input : t_upstyp_product *, a pointer to a product
 *         char *, path to a ups file
 *         char, 'd' means it will create a (one level) 
 *               directory if needed.
 * Output: none
 * Return: int, UPS_SUCCESS just fine, else UPS_<error> number.
 */
int upsfil_write_file( t_upstyp_product * const prod_ptr,
		       const char * const ups_file,		       
		       const char copt,
		       const int journal )
{
  const char *key = 0;
  static char buff[MAX_LINE_LEN];
  int res;
  struct stat statbuf;
  DIR *dd;
  struct dirent *de;
  

  (void) strcpy( g_filename, ups_file );
  g_item_count = 0;
  g_line_count = 0;
  
  if ( ! prod_ptr ) {
    upserr_vplace(); upserr_add( UPS_NOVALUE_ARGUMENT, UPS_FATAL, "0",
				 "product pointer" );
    return UPS_NOVALUE_ARGUMENT;
  }

  g_pd = prod_ptr;

  if ( !ups_file || strlen( ups_file ) <= 0 ) {
    upserr_vplace(); upserr_add( UPS_OPEN_FILE, UPS_FATAL, "" );
    return UPS_OPEN_FILE;
  }

  /* get key to cache */

  if ( g_use_cache ) {
    if ( !g_ft ) {
      g_ft = upstbl_new( 300 );
    }
    key = upstbl_atom_string( ups_file );
  }

  /* handle journal files,
     a journal file will just be added to the cache */

  prod_ptr->journal = journal;
  if ( journal == JOURNAL ) {

    /* add file to cache */

    if ( g_use_cache ) {
      if ( !g_ft ) {
	g_ft = upstbl_new( 300 );
      }
      key = upstbl_atom_string( ups_file );
      if ( !upstbl_get( g_ft, key ) ) {
	(void) upstbl_put( g_ft, key, g_pd );
	P_VERB_s( 1, "New (journal) product added to cache" );
      }
    }

    return UPS_SUCCESS;
  }

  /* if we are a JOURNAL we should not come here */

  /* check if prod_ptr is empty, if empty remove the file */

  if ( upslst_count( prod_ptr->instance_list ) <= 0 ) {
    P_VERB_s( 1, "Removing file (product is empty)" );
    /* remove product from cache */
    if ( g_ft ) 
      (void) upstbl_remove( g_ft, key );
    /* remove file and rewrite it , always... */
    
    res = stat(ups_file, &statbuf);
    if (0 == res && S_ISDIR(statbuf.st_mode)) {
      dd = opendir(ups_file);
      while (dd && 0 != (de = readdir(dd))) {
           if (de->d_name[0] != '#' && de->d_name[0] != '.' && de->d_name[strlen(de->d_name)-1] != '~'&& 0 != strcmp(de->d_name, "CVS")) {
	      sprintf(g_subdir_buf, "%s/%s", ups_file, de->d_name);
	      remove(g_subdir_buf);
	 }
	}
      closedir(dd);
      }
    if (remove( ups_file ) != 0)
    {
      P_VERB_s( 1, "Removing file ERROR" );
      upserr_add( UPS_SYSTEM_ERROR, UPS_FATAL, "remove", strerror(errno));
      upserr_vplace(); upserr_add( UPS_REMOVE_FILE, UPS_FATAL, ups_file );
      return UPS_REMOVE_FILE;
    }
    return UPS_SUCCESS;
  }


  /* check if directory exist,
     if not, create it */

  if ( copt == 'd' && 
       upsutl_is_a_file( ups_file ) == UPS_NO_FILE ) {
    int l;
    (void) strcpy( buff, ups_file );
    l = (int )strlen( buff );
    while ( --l >= 0 && buff[l] != '/' ) buff[l] = 0;
    if (buff[l] == '/') buff[l] = 0;
    if ( l > 0 && upsutl_is_a_file( buff ) == UPS_NO_FILE ) {
      if (mkdir( buff, 0775 )) {
         P_VERB_s( 1, "Create directory ERROR" );
         upserr_add( UPS_SYSTEM_ERROR, UPS_FATAL, "mkdir", strerror(errno));
         upserr_vplace(); upserr_add( UPS_OPEN_FILE, UPS_FATAL, ups_file );
         return(UPS_OPEN_FILE);
      }
    }
  }

  /* set file type (g_ifile) */

  (void) cfilei();

  /* open file */

  if (g_subdir_files_flag && ( g_ifile == e_file_version || g_ifile == e_file_chain )) {

    res = stat(ups_file, &statbuf);
    if (0 == res && S_ISDIR(statbuf.st_mode)) {
      dd = opendir(ups_file);
      while (dd && 0 != (de = readdir(dd))) {
           if (de->d_name[0] != '#' && de->d_name[0] != '.' && de->d_name[strlen(de->d_name)-1] != '~'&& 0 != strcmp(de->d_name, "CVS")) {
	      sprintf(g_subdir_buf, "%s/%s", ups_file, de->d_name);
	      remove(g_subdir_buf);
	 }
        }
      closedir(dd);
      }

    if ( res == 0 )
       remove( ups_file );

    g_subdir_base = ups_file;
    mkdir( g_subdir_base, 0775 );

  } else {

    g_fh = fopen ( ups_file, "w" );
    start_file(ups_file);
    if ( ! g_fh ) 
        return UPS_OPEN_FILE;

  }

  if ( g_ifile == e_file_unknown ) {
    if(g_fh) fclose(g_fh);
    g_fh = 0;
    P_VERB_s( 1, "Unknown file type for writing" );
    upserr_vplace();
    upserr_add( UPS_UNKNOWN_FILETYPE, UPS_WARNING, g_pd->file ? g_pd->file : "(null)" );
    return UPS_UNKNOWN_FILETYPE;
  }
    
  /* write instances */

  switch ( g_ifile ) {

  case e_file_version:
    P_VERB_s( 1, "Writing version file" );
    (void) write_version_file();
    break;
	
  case e_file_table:
    P_VERB_s( 1, "Writing table file" );
    (void) write_table_file();
    break;
	
  case e_file_chain:
    P_VERB_s( 1, "Writing chain file" );
    (void) write_chain_file();
    break;
  }

  
  if(g_fh) (void) fclose( g_fh );
  g_fh = 0;

  P_VERB_s_i_s( 1, "Write", g_item_count, "item(s)" );

  /* if new product, add it to cache */

  if ( g_ft ) {
    if ( !upstbl_get( g_ft, key ) ) {
      (void) upstbl_put( g_ft, key, g_pd );
      P_VERB_s( 1, "New product added to cache" );
    }
  }    

  if(g_fh) (void) fclose( g_fh );
  g_fh = 0;

  return UPS_SUCCESS;
}

const char *upsfil_last_file( void )
{
  /* will return the name of the current or last file */
  
  return g_filename;
}

t_upstyp_product  *upsfil_is_in_cache( const char * const ups_file )
{
  /* will just check if a given file is in cache,
     it will return 0 if file not in cahce,
     else non-zero (the found product pointer) */

  t_upstyp_product  *pd = 0;
  const char *key = 0;

  if ( g_use_cache && g_ft ) {
    key = upstbl_atom_string( ups_file );
    pd = upstbl_get( g_ft, key );
  }

  return pd;
}

int upsfil_exist( const char * const ups_file )
{
  /* will return true if file is in cache or on disk,
     else false */

  if ( upsfil_is_in_cache( ups_file ) )
    return 1;

  if ( upsutl_is_a_file( ups_file ) == UPS_SUCCESS ) 
    return 1;

  return 0;
}

void flush_product( const void *key, void ** prod, void *cl ) 
{
  /* a little helper for upsfil_flush */

  /* write the file to disk, if journal flag set */

  write_journal_file( key, prod, cl );

  /* free product */

  if ( *prod ) {
    (void) ups_free_product( *prod );
    *prod = 0;
  }
}     
void upsfil_flush( void )
{
  /* upsfil_stat( 1 ); */

  /* write journal files to disk and clean up cache */

  static char old_filename[MAX_LINE_LEN];
  (void) strcpy( old_filename, g_filename );

  if ( g_ft ) {
    P_VERB( 1, "Flushing cache" );
    upstbl_map( g_ft, flush_product, NULL );
    upstbl_free( &g_ft );
    g_ft = 0;
  }

  (void) strcpy( g_filename, old_filename );
}

void upsfil_stat( const int iopt )
{
  /* print some statistic og the file cache */

  (void) printf( "total calls = %d, cache calls = %d (%.1f%%)\n", 
	  g_call_count, g_call_cache_count, 
	  g_call_count ? 100.0*(double)(g_call_cache_count)/(double)g_call_count : (double)0 );

  upstbl_dump( g_ft, iopt );
}

/*
 * Definition of private functions
 */

t_upstbl *g_file_cache( void )
{
  return g_ft;
}


int write_version_file( void )
{
  t_upslst_item *l_ptr = 0;
  t_upstyp_instance *inst_ptr = 0;
  int *ikeys=0;
  int o_imargin = g_imargin;

  /* write file descriptor */
  

  if (!g_subdir_files_flag) {
      (void) put_head_keys( upskey_verhead_arr());
  }
  
  /* write instances */
  
  ikeys = upskey_verinst_arr();
  l_ptr = upslst_first( g_pd->instance_list );

  for( ; l_ptr; l_ptr = l_ptr->next ) {

    inst_ptr = (t_upstyp_instance *)l_ptr->data;

    if ( !inst_ptr || !inst_ptr->flavor ) {
      /* handle error !!! */
      return 0;
    }

    if (g_subdir_files_flag) {
      if(g_fh) (void) fclose(g_fh);
      g_fh = 0;

      sprintf(g_subdir_buf, "%s/%s_%s", g_subdir_base, inst_ptr->flavor, without_colons(inst_ptr->qualifiers) );
  
      g_fh = fopen( g_subdir_buf, "w" );
 
      start_file(g_subdir_buf);

      if ( ! g_fh ) 
        return UPS_OPEN_FILE;

      put_head_keys(upskey_verhead_arr());
      (void)upskey_verinst_arr();
    }

    g_imargin = o_imargin;

    g_item_count++;
    (void) put_key( 0, "" );
    (void) put_key( 0, SEPARATION_LINE );

    (void) put_inst_keys( ikeys, inst_ptr, 0 );

  }  

  return 1;
} 

int write_chain_file( void )
{
  t_upslst_item *l_ptr = 0;
  t_upstyp_instance *inst_ptr = 0;
  int *ikeys;
  int o_imargin = g_imargin;

  /* write file descriptor */
  
  if (!g_subdir_files_flag) {
    (void) put_head_keys( upskey_chnhead_arr() );
  }
  
  /* write instances */
  
  ikeys = upskey_chninst_arr();
  l_ptr = upslst_first( g_pd->instance_list );
  for( ; l_ptr; l_ptr = l_ptr->next ) {

    inst_ptr = (t_upstyp_instance *)l_ptr->data;

    if ( !inst_ptr || !inst_ptr->flavor ) {
      /* handle error !!! */
      return 0;
    }

    if (g_subdir_files_flag) {
      if(g_fh) (void) fclose(g_fh);
      g_fh = 0;
      sprintf(g_subdir_buf, "%s/%s_%s", g_subdir_base, inst_ptr->flavor, without_colons(inst_ptr->qualifiers) );

      g_fh = fopen( g_subdir_buf, "w" );
 
      start_file(g_subdir_buf);
      if ( ! g_fh ) 
        return UPS_OPEN_FILE;
      (void) put_head_keys( upskey_chnhead_arr() );
      (void) upskey_chninst_arr();
    }

    g_imargin = o_imargin;

    g_item_count++;
    (void) put_key( 0, "" );
    (void) put_key( 0, SEPARATION_LINE );

    (void) put_inst_keys( ikeys, inst_ptr, 0 );

  }  

  return 1;
} 

int write_table_file( void )
{
  t_upslst_item *l_inst = 0;
  t_upstyp_instance *inst_ptr = 0;
  int *common_mask;
  int o_imargin;
  int *tbl_keys;
  int *keys;
  int ig;

  t_upslst_item *l_copy, *l_ptr;

  /* write file descriptor */
  
  (void) put_head_keys( upskey_tblhead_arr() );  
  (void) put_key( 0, "" );

  /* REMEBER that the array of keys returned from upskey_*_arr() is static,
     so, the reason we set tbl_keys here, is because we only need the table
     instance keys for the rest of this scope */

  tbl_keys = upskey_tblinst_arr();

  /* write groups based on seeds defined in g_groups */

  l_copy = upslst_copy( g_pd->instance_list );

  for ( ig = 0; ig < e_group_count; ig++ ) {

    (void) find_group( l_copy, 's', 0 );
    while( (l_ptr = find_group( 0, ' ', g_groups[ ig ] )) ) {

      /* check for other and set common keys */

      common_mask = find_common_mask_list( l_ptr, tbl_keys );
      for ( keys = g_groups[ ig ]; *keys != -1; keys++ )
	common_mask[ *keys ] = 1;

      (void) put_group( l_ptr, tbl_keys, common_mask );
    }

    l_copy = find_group( 0, 'e', 0 );
  }

  /* write the rest */
  
  l_inst = upslst_first( l_copy );
  o_imargin = g_imargin;
  for( ; l_inst; l_inst = l_inst->next ) {

    g_imargin = o_imargin;

    inst_ptr = (t_upstyp_instance *)l_inst->data;
    if ( !inst_ptr || !inst_ptr->flavor ) {
      /* handle error !!! */
      continue;
    }

    g_item_count++;
    (void) put_key( 0, SEPARATION_LINE );
    (void) put_inst_keys( tbl_keys, inst_ptr, 1 );
    g_imargin = o_imargin;
    (void) put_key( 0, "" );    
  }  

  return 1;
} 

int write_action( t_upstyp_action * const act_ptr )
{
  t_upslst_item *l_com = 0;
  char *com;

  if ( !act_ptr ) return 0;

  (void) put_key( "ACTION", act_ptr->action );
  
  g_imargin += 2;
  l_com = upslst_first( act_ptr->command_list );
  for( ; l_com; l_com = l_com->next ) {
    com = (char * )l_com->data;
    (void) put_key( 0, com );
  }
  g_imargin -= 2;

  return 1;
}

/*-----------------------------------------------------------------------
 * read_file
 *
 * Will read all instances in an ups file
 *
 * Input : none
 * Output: none
 * Return: int, 1 if read any instances, else 0
 */
int read_file( void )
{
  int iret = 0;
  t_upslst_item *l_ptr = 0;

  /* read comments */

  g_pd->comment_list = read_comments();

  /* read file type */
  if ( !read_file_type() )
    return 0;

  /* if config file we are done quickly */
  
  if ( g_ifile == e_file_dbconfig ) {
    g_pd->config = read_config();
    return 1;
  }
  
  /* read file descriptor */

  if ( ! read_file_desc() )
    return 0;

  /* here, we expect only to see FLAVOR or GROUP: */

  while ( g_ikey != e_key_eof ) {
    l_ptr = 0;

    switch( g_ikey ) {

    case e_key_flavor:
      l_ptr = read_instances();
      break;
    
    case e_key_group:
      l_ptr = read_groups();
      break;

    default: /* if not flavor or group, ignore it */
      (void) next_key( 0 );
    }
    
    if ( l_ptr ) {
      iret = 1;
      g_pd->instance_list = upslst_add_list( g_pd->instance_list, l_ptr );
    }
  }
	  
  return iret;
}

/*-----------------------------------------------------------------------
 * read_comments
 *
 * Will read a ups files comments (lines at the top of the file
 * starting with a '#').
 *
 * Input:  none
 * Output: none
 * Return: t_upslst_item *, pointer to a list of strings
 */
t_upslst_item *read_comments( void )
{
  t_upslst_item *l_ptr = 0;
  
  while ( fgets( g_line, MAX_LINE_LEN, g_fh ) ) {
    g_line_count++;

    if ( !upsutl_str_remove_edges( g_line, WCHARS ) ) continue;   
    if ( g_line[0] == '#' ) {
      l_ptr = upslst_add( l_ptr, upsutl_str_create( g_line, ' ' ) );
    }
    else {
      (void) get_key();
      break;
    }
  }
  
  return l_ptr;
}
  
/*-----------------------------------------------------------------------
 * read_file_type
 *
 * Will read file descriptor: FILE
 *
 * Input:  none
 * Output: none
 * Return: int,  1 success, 0 error.
 */
int read_file_type( void )
{
  /* all lines are ignore until a 'start_key' */
  
  (void) find_start_key();
  
  /* if first 'start_key' is not file type, generate an error */

  if ( g_ikey != e_key_file ) {
    upserr_vplace();
    upserr_add( UPS_UNKNOWN_FILETYPE, UPS_WARNING, "(null)" );    
    return 0;
  }  
  
  g_pd->file = upsutl_str_create( g_val, ' ' );
  
  /* translate file type to an enum */
  
  if ( cfilei() == e_file_unknown ) {
    upserr_vplace();
    upserr_add( UPS_UNKNOWN_FILETYPE, UPS_WARNING, g_pd->file );
  }  

  return 1;
}

/*-----------------------------------------------------------------------
 * read_file_desc
 *
 * Will read file descriptor: FILE, PRODUCT, VERSION/CHAIN.
 *
 * Input:  none
 * Output: none
 * Return: int,  1 if file descriptor was read, else 0
 */
int read_file_desc( void )
{
  if ( g_ikey != e_key_file ) {
    return 0;
  }  

  (void) next_key( 0 );

  while ( !is_stop_key() ) {

    if ( g_ikey == e_key_user_defined ) {
      (void) sprintf( g_line, "%s = %s", g_key, g_val );
      g_pd->user_list = upslst_add( g_pd->user_list, 
				    upsutl_str_create( g_line, ' ' ) );
    }

    if ( g_mkey && g_mkey->p_index != INVALID_INDEX ) {
      UPSKEY_PROD2ARR( g_pd )[g_mkey->p_index] = upsutl_str_create( g_val, ' ' );
    }
    else if ( UPS_VERIFY ) {
      upserr_vplace(); upserr_add( UPS_UNEXPECTED_KEYWORD, UPS_INFORMATIONAL,
				   g_key, g_filename, g_line_count );
    }

    (void) next_key( 0 );
  }

  return 1;
}
/* The verify's a set of "keys" product,flavor,version,qualifiers WITHIN
** a group or file with no groups.  The verify groups has to be used to
** verify the keys within the sets of groups.  
*/
void verify_keys(t_upslst_item *l_ptr, t_upstyp_instance *inst_ptr)
{
    int verify_key=0;
    t_upslst_item *vl_ptr = 0;
    t_upstyp_instance *vinst_ptr = 0;
    static char file[10];
    static char *nostring = "";
    verify_key=upsget_key(inst_ptr); 
    vl_ptr = upslst_first(l_ptr);
    for ( ; vl_ptr; vl_ptr = vl_ptr->next ) 
    {   vinst_ptr = (t_upstyp_instance *)vl_ptr->data;
        if ( verify_key == upsget_key(vinst_ptr) )
        { switch ( g_ifile )  
          { case e_file_version: (void) strcpy(file,"VERSION"); break;
            case e_file_table: (void) strcpy(file,"TABLE"); break;
            case e_file_chain: (void) strcpy(file,"CHAIN"); break;
          }
          upserr_add(UPS_DUPLICATE_INSTANCE, UPS_INFORMATIONAL, 
                     file , g_filename, 
                     (vinst_ptr->product ? vinst_ptr->product : nostring),
		     (vinst_ptr->version ? vinst_ptr->version : nostring), 
                     vinst_ptr->flavor,vinst_ptr->qualifiers,
                     verify_key);
        }
    }
}
/* The l_ptr is the existing list of instances the n_ptr is the list
** that's going to be added to the existing list.  I check the instances
** in the new list, which MAY ALLREADY have a duplicate and be previously
** reported, against the existing list.  If I took the final list and
** compaired it against itself it would report duplicates within groups
** twice
**/
void verify_groups(t_upslst_item *l_ptr,t_upslst_item *n_ptr)
{
    int verify_key=0;
    t_upslst_item *myl_ptr = 0;
    t_upslst_item *myn_ptr = 0;
    t_upstyp_instance *vinst_ptr = 0;
    static char file[10];
    static char *nostring = "";
    myn_ptr = upslst_first(n_ptr);
    for ( ; myn_ptr; myn_ptr = myn_ptr->next ) 
    { vinst_ptr = (t_upstyp_instance *)myn_ptr->data;
      verify_key = upsget_key(vinst_ptr);
      myl_ptr = upslst_first(l_ptr);
      for ( ; myl_ptr; myl_ptr = myl_ptr->next ) 
      { vinst_ptr = (t_upstyp_instance *)myl_ptr->data;
        if ( verify_key == upsget_key(vinst_ptr) )
        { switch ( g_ifile )  
          { case e_file_version: (void) strcpy(file,"VERSION"); break;
            case e_file_table: (void) strcpy(file,"TABLE"); break;
            case e_file_chain: (void) strcpy(file,"CHAIN"); break;
          }
          upserr_add(UPS_DUPLICATE_INSTANCE, UPS_INFORMATIONAL, 
                     file , g_filename, 
                     (vinst_ptr->product ? vinst_ptr->product : nostring),
		     (vinst_ptr->version ? vinst_ptr->version : nostring), 
                     vinst_ptr->flavor,vinst_ptr->qualifiers,
                     verify_key);
        }
      }
    }
}

/*-----------------------------------------------------------------------
 * read_instances
 *
 * Will build a list of instances. Reading instances will continue
 * until next 'stop' key is not "FLAVOR".
 *
 * Input : none
 * Output: none
 * Return: t_upslst_item *, pointer to a list of instances
 */
t_upslst_item *read_instances( void )
{
    t_upslst_item *l_ptr = 0;
    t_upstyp_instance *inst_ptr = 0;
    
    while  ( g_ikey == e_key_flavor ) {
	inst_ptr = read_instance();
	
	if ( inst_ptr )
        { if (UPS_VERIFY) { verify_keys(l_ptr,inst_ptr); }
	  l_ptr = upslst_add( l_ptr, inst_ptr );
	} 

	/* let's try just to continue
	else {
	  break;
        }
	*/
    }
    
    return upslst_first( l_ptr );
}
  
/*-----------------------------------------------------------------------
 * read_instance
 *
 * Will read a single instance
 *
 * Input : none
 * Output: none
 * Return: pointer to an instance
 */
t_upstyp_instance *read_instance( void )
{
  t_upstyp_instance *inst_ptr = 0;
  
  if ( g_ikey != e_key_flavor )
    return 0;

  inst_ptr = ups_new_instance();

  /* fill information from file descriptor */
  
  inst_ptr->product = upsutl_str_create( g_pd->product, ' ' );
  if ( g_ifile == e_file_chain )
    inst_ptr->chain = upsutl_str_create( g_pd->chain, ' ' );
  else
    inst_ptr->version = upsutl_str_create( g_pd->version, ' ' );
    
  /* fill information from file */
  
  inst_ptr->flavor = upsutl_str_create( g_val, ' ' );

  (void) next_key( 0 );

  while ( !is_stop_key() ) {

    if ( g_ikey == e_key_action ) {

      inst_ptr->action_list = 
	upslst_add_list( inst_ptr->action_list, read_actions() );

    }
    else {

      switch( g_ikey ) {

      case e_key_action:
	inst_ptr->action_list = 
	  upslst_add_list( inst_ptr->action_list, read_actions() );

      case e_key_qualifiers:
	(void) trim_qualifiers( g_val );
	inst_ptr->qualifiers = upsutl_str_create( g_val, ' ' );
	break;
      
      case e_key_user_defined:
	(void) sprintf( g_line, "%s = %s", g_key, g_val );
	inst_ptr->user_list = upslst_add( inst_ptr->user_list, 
					  upsutl_str_create( g_line, ' ' ) );
	break;
      
      default:
	if ( g_mkey && g_mkey->i_index != INVALID_INDEX ) { 

	  char *val = g_val;

	  if ( UPS_VERIFY ) { 
	    if( UPSKEY_INST2ARR( inst_ptr )[g_mkey->i_index] != 0 ) { 
	      (void) printf("duplicate key in file %s line %d\n", 
		     g_filename, g_line_count); 
	      (void) printf("key %s value %s \n", 
		     g_key, g_val); 
	    }
	  }

	  /* if key value contain env.var, the original string will be saved
	     be saved (in inst_ptr->sav_inst) if written out to file */
	  
	  if ( g_tval ) {
	  
	    if ( ! inst_ptr->sav_inst )
	      inst_ptr->sav_inst = ups_new_instance();

	    UPSKEY_INST2ARR( inst_ptr->sav_inst )[g_mkey->i_index] = 
	      upsutl_str_create( val, ' ' );
	    val = g_tval;
	  }
	
	  UPSKEY_INST2ARR( inst_ptr )[g_mkey->i_index] = 
	    upsutl_str_create( val, ' ' );
	}
	else if ( UPS_VERIFY ) {
	  upserr_vplace(); upserr_add( UPS_UNEXPECTED_KEYWORD, UPS_INFORMATIONAL,
				       g_key, g_filename, g_line_count );
	}
	break;
      }
      
      (void) next_key( 0 );
    }

  }

  if ( inst_ptr ) {

    /* something special for qualifiers, if not in file, set it to default */

    if ( ! inst_ptr->qualifiers ) {
      if ( UPS_VERIFY )
	(void) printf( "no qualifiers found in file %s line %d\n", 
		g_filename, g_line_count );

      inst_ptr->qualifiers = upsutl_str_create( "", ' ' );
    }

    g_item_count++;
  }

  return inst_ptr;
}

/*-----------------------------------------------------------------------
 * read_common
 *
 * Will read a common block. which is close to a single instance.
 *
 * Input : none
 * Output: none
 * Return: pointer to an instance
 */
t_upstyp_instance *read_common( void )
{
  t_upstyp_instance *inst_ptr = 0;
  
  inst_ptr = ups_new_instance();

  (void) next_key( 0 );

  while ( ! is_stop_key() ) {

    /* actions */

    if ( g_ikey == e_key_action ) {

      inst_ptr->action_list = 
	upslst_add_list( inst_ptr->action_list, read_actions() );


    }
    else {

      switch( g_ikey ) {

      case e_key_qualifiers:
	if ( UPS_VERIFY ) { 
	  (void) printf( "read qualifiers key in a common section, in file %s, line %d",
		  g_filename, g_line_count );
	}
	break;
      
      case e_key_user_defined:
	(void) sprintf( g_line, "%s = %s", g_key, g_val );
	inst_ptr->user_list = upslst_add( inst_ptr->user_list, 
					  upsutl_str_create( g_line, ' ' ) );
	break;
      
      default:
	if ( g_mkey && g_mkey->i_index != INVALID_INDEX ) { 

	  char *val = g_val;

	  if ( UPS_VERIFY ) { 
	    if( UPSKEY_INST2ARR( inst_ptr )[g_mkey->i_index] != 0 ) { 
	      (void) printf("duplicate key in file %s line %d\n", 
		     g_filename, g_line_count); 
	      (void) printf("key %s value %s \n", 
		     g_key, g_val); 
	    }
	  }

	  /* if key value contain env.var, the original string will be saved
	     be saved (in inst_ptr->sav_inst) if written out to file */

	  if ( g_tval ) {
	    
	    if ( ! inst_ptr->sav_inst )
	      inst_ptr->sav_inst = ups_new_instance();
	    
	    UPSKEY_INST2ARR( inst_ptr->sav_inst )[g_mkey->i_index] = 
	      upsutl_str_create( val, ' ' );
	    val = g_tval;
	  }

	  UPSKEY_INST2ARR( inst_ptr )[g_mkey->i_index] = 
	    upsutl_str_create( val, ' ' );
	}
	else if ( UPS_VERIFY ) {
	  upserr_vplace(); upserr_add( UPS_UNEXPECTED_KEYWORD, UPS_INFORMATIONAL,
				       g_key, g_filename, g_line_count );
	}
	break;
      }

      (void) next_key( 0 );

    }

    g_item_count++;
  }

  return inst_ptr;
}

/*-----------------------------------------------------------------------
 * read_actions
 *
 * Will build a list of actions. Reading actions will continue
 * until next 'stop' key is not "ACTION".
 *
 * Input : none
 * Output: none
 * Return: pointer to a list of actions
 */
t_upslst_item *read_actions( void )
{
    t_upslst_item *l_ptr = 0;
    t_upstyp_action *act_ptr = 0;
    
    while  ( g_ikey == e_key_action ) {
	act_ptr = read_action();
	
	if ( act_ptr ) {
	  l_ptr = upslst_add( l_ptr, act_ptr );
	}
	else {
	  break;
	}
    }

    return upslst_first( l_ptr );
}

/*-----------------------------------------------------------------------
 * read_action
 *
 * Will read a single action
 *
 * Input : none
 * Output: none
 * Return: pointer to an action
 */
t_upstyp_action *read_action( void )
{
  t_upstyp_action *act_ptr = 0;
  t_upslst_item *l_cmd = 0;
  char *cmd_ptr = 0;

  if ( g_ikey != e_key_action ) 
    return 0;

  act_ptr = ups_new_action();
  act_ptr->action = upsutl_str_create( g_val, ' ' );

  (void) next_key( 1 );

  while ( !is_stop_key() && g_ikey == e_key_unknown ) {

    if ( strlen( g_line ) > 0 ) {
      cmd_ptr = upsutl_str_create( g_line, ' ' );
      if ( cmd_ptr )
	l_cmd = upslst_add( l_cmd, cmd_ptr );
    }
    
    (void) next_key( 1 );
    
  }

  act_ptr->command_list = upslst_first( l_cmd );
  
  return act_ptr;
}

/*-----------------------------------------------------------------------
 * read_config
 *
 * Will read ups config file.
 *
 * Input : char *, path to a ups config file
 * Output: none
 * Return: t_upstyp_config *, a pointer to the config structure.
 */
t_upstyp_config *read_config( void )
{
  t_upstyp_config *conf_ptr = ups_new_config();

  while ( next_key( 0 ) != e_key_eof ) {

    if ( g_ikey == e_key_statistics ) {
      (void) upsutl_str_remove( g_val, WCHARSQ );  
      (void) upsutl_str_sort( g_val, ':' );
      g_item_count++;
    }
      
    if ( g_ikey == e_key_user_defined ) {
      (void) sprintf( g_line, "%s = %s", g_key, g_val );
      g_pd->user_list = upslst_add( g_pd->user_list, 
				    upsutl_str_create( g_line, ' ' ) );
    }

    /* for config files we don't keep the original record, cuz we 
       never write out these files, if we do we should  */

    if ( g_mkey && g_mkey->c_index != INVALID_INDEX ) {

      char *val = g_val;

      if ( g_tval )
	val = g_tval;

      UPSKEY_CONF2ARR( conf_ptr )[g_mkey->c_index] = upsutl_str_create( val, ' ' );
      g_item_count++;
    }
  }

  return conf_ptr;
}

/*-----------------------------------------------------------------------
 * read_groups()
 *
 * Will read a list of groups. Reading groups will
 * continue until 'stop' key is not "GROUP:" 
 *
 * Input : none.
 * Output: none.
 * Return: t_upslst_item *, pointer to a list of instances
 */
t_upslst_item *read_groups( void )
{    
    t_upslst_item *l_ptr = 0;
    t_upslst_item *l_tmp_ptr = 0;
    
    while  ( g_ikey == e_key_group ) {
	l_tmp_ptr = read_group();

	if ( l_tmp_ptr ) { 
	  if ( UPS_VERIFY ) { 
	    verify_groups(l_ptr,l_tmp_ptr); 
	  }
	  l_ptr = upslst_add_list( l_ptr, l_tmp_ptr );
	}

	/* let's try just to continue 
	else {
	  break;
	}
	*/
	
    }

    return upslst_first( l_ptr );
}

/*-----------------------------------------------------------------------
 * read_group
 *
 * Will read a single group.
 *
 * Input : none.
 * Output: none.
 * Return: t_upslst_item *, pointer to a list of instances
 */
t_upslst_item *read_group( void )
{
    t_upslst_item *l_ptr = 0;
    t_upslst_item *l_inst_ptr = 0;
    t_upstyp_instance *inst_ptr = 0;
    t_upstyp_instance *com_ptr = 0;

    if ( g_ikey != e_key_group ) 
      return 0;
    
    (void) next_key( 0 );

    /* skip to next start key */

    (void) find_start_key();
  
    if ( g_ikey != e_key_flavor )
      return 0;

    l_inst_ptr = read_instances();

    if ( !l_inst_ptr )
      return 0;

    if ( g_ikey == e_key_common )
      com_ptr = read_common();

    /* add common to instances */

    if ( l_inst_ptr && com_ptr ) {

      /* copy common to instance list,
	 currently common will overwrite */

      l_ptr = upslst_first( l_inst_ptr );
      for ( ; l_ptr; l_ptr = l_ptr->next ) {
	inst_ptr = (t_upstyp_instance *)l_ptr->data;	
	(void) add_to_instance( inst_ptr, com_ptr );
      }
    }

    (void) ups_free_instance( com_ptr );

    if ( g_ikey == e_key_end ) {
      (void) next_key( 0 );
    }
    else if ( UPS_VERIFY ) {
      (void) printf( "Did not find end (END:) of group in file %s, line %d\n",
	      g_filename, g_line_count);
    }

    (void) find_start_key();
    
    return upslst_first( l_inst_ptr );
}

/*
 * Line parsing
 */


/*-----------------------------------------------------------------------
 * find_start_key
 *
 * Will skip lines until next start key
 *
 * Input : none
 * Output: none
 * Return: int, enum of current key.
 */
int find_start_key( void )
{
  /* skip to next start key */

  while ( !is_start_key() && g_ikey != e_key_eof ) {
    if ( UPS_VERIFY ) {
      upserr_vplace(); upserr_add( UPS_UNEXPECTED_KEYWORD, UPS_INFORMATIONAL,
				   g_key, g_filename, g_line_count );
    }
    (void) next_key( 0 );
  }
  return g_ikey;
}
 
/*-----------------------------------------------------------------------
 * next_key
 *
 * Will read next not empty line from file.
 *
 * Input : none
 * Output: none
 * Return: int, enum of current key.
 */
int next_key( int ignore_unknown )
{  
  g_key[0] = 0;
  g_val[0] = 0;
  g_tval = 0;
  g_line[0] = 0;
  g_ikey = e_key_eof;
  g_mkey = 0;

  while ( fgets( g_line, MAX_LINE_LEN, g_fh ) ) {
    g_line_count++;

    if ( strlen( g_line ) < 1 ) continue;
    if ( g_line[0] == '#' ) continue;

    P_VERB_s_i_s_nn( 3, "reading line :", g_line_count, g_line );
  
    if ( !upsutl_str_remove_edges( g_line, WCHARS ) ) continue;

    if ( get_key() != e_key_eol ) {

      /* translate key value, if needed */

      if ( g_mkey && 
	   g_ikey != e_key_unknown &&
	   g_val[0] &&
	   UPSKEY_TRY_TRANSLATE( g_mkey->flag ) ) {

	g_tval = upsget_translation_env( g_val ); 
      }

      if ( !ignore_unknown && g_ikey == e_key_unknown ) {
	upserr_vplace(); upserr_add( UPS_UNEXPECTED_KEYWORD, UPS_INFORMATIONAL,
				     g_key, g_filename, g_line_count );
      }
      else {
	return g_ikey;
      }
    }
  }

  return e_key_eof;
}

/*-----------------------------------------------------------------------
 * get_key
 *
 * Will parse the current line (g_line) and fill current key (g_key/g_ikey)
 * and current value (g_val)
 *
 * Input : none
 * Output: none
 * Return: char*, pointer to current key;
 */
int get_key( void )
{
  char *cp = g_line;
  int count = 0;
  int has_val = 1;

  /* check if line is not empty (again) */
  
  if ( strlen( g_line ) < 1 || g_line[0] == '#' ) {
    P_VERB_s_i( 3, "parsed line  :", g_line_count );
    return e_key_eol;
  }
    
  /* check if line has a key/value pair */
  
  if ( !strchr( g_line, '=' ) ) {
    (void) strcpy( g_key, g_line );
    has_val = 0;
  }

  /* split line into key/value pair */

  else {
    count = 0;
    while ( cp && *cp && !isspace( (unsigned long )*cp ) && *cp != '=' ) {
      g_key[count] = *cp;
      count++; cp++;
    }
    g_key[count] = 0;
    if ( strlen( g_key ) <= 0 ) {
      P_VERB_s_i( 3, "parsed line  :", g_line_count );
      return e_key_eol;
    }
  
    while( cp && *cp && *cp != '=' ) { cp++; }
    cp++;
    while( cp && *cp && isspace( (int)*cp ) ) { cp++; }
    count = 0;
    while( cp && *cp && *cp != '\n' ) {
      g_val[count] = *cp;
      count++; cp++;
    }
    g_val[count] = 0;

    (void) upsutl_str_remove_end_quotes( g_val, QUOTES, 0 );  
  }

  g_mkey = upskey_get_info( g_key );
  if ( g_mkey ) {
    g_ikey = g_mkey->ikey;
  }
  else {
    if ( g_line[0] == '_' ) 
      g_ikey = e_key_user_defined;
    else
      g_ikey = e_key_unknown;
  }

  if ( has_val ) {
    P_VERB_s_i_s_s_s( 3, "parsed line  :", g_line_count, g_key, "=", g_val );
  }
  else {
    P_VERB_s_i_s( 3, "parsed line  :", g_line_count, g_key );
  }

  return g_ikey;
}

/*-----------------------------------------------------------------------
 * put_group
 *
 * Will print and format a groups. common items are defined by the passed
 * common mask.
 * It will not print anything if val is empty.
 *
 * Input : t_upslst_item*, list of instances
 *         int *, common mask
 * Output: none
 * Return: int, 1 fine, 0 not fine
 */
int put_group( t_upslst_item * const l_inst, 
	       int * const keys,
	       int * const common_mask )
{
  int o_imargin;
  t_upslst_item *l_i = upslst_first( l_inst );
  t_upstyp_instance *p_inst = 0;

  if ( upslst_count( l_inst ) == 0 ) 
    return 0;

  (void) put_key( 0, SEPARATION_LINE );
  (void) put_key( 0, "GROUP:");
  g_imargin += 2;
  o_imargin = g_imargin;
  for ( ; l_i; l_i = l_i->next ) {

    p_inst = (t_upstyp_instance *)l_i->data;
    (void) put_inst_keys_mask( keys, p_inst, 0, common_mask );
    (void) put_key( 0, "" );
    g_imargin = o_imargin;
  }

  g_imargin -= 2;
  (void) put_key( 0, "COMMON:" );    
  g_imargin += 2;

  (void) put_inst_keys_mask( keys, p_inst, common_mask, 0 );

  g_imargin -= 2; 
  (void) put_key( 0, "" );
  (void) put_key( 0, "END:" );
  (void) put_key( 0, "" );

  return 1;
}

/*-----------------------------------------------------------------------
 * put_head_keys
 *
 * Will print and format passed key and val for a ups file header.
 * It will not print anything if val is empty.
 *
 * Input : char *, arrays of indexes to print
 * Output: none
 * Return: int, 1 fine, 0 not fine
 */
int put_head_keys( int * ikeys ) 
{
  t_upskey_map *map =  0;
  int do_user_def = 0;

  for ( ; ikeys && *ikeys != -1; ikeys++ ) {

    if ( *ikeys == e_key_user_defined ) {
      do_user_def = 1;
      continue;
    }

    map = &g_key_info[ *ikeys ];
    (void) put_key( map->key, 
	     UPSKEY_PROD2ARR( g_pd )[ map->p_index ] );
  }

  /* write user defined key words */

  if ( do_user_def && g_pd->user_list ) {
    t_upslst_item *l_ptr = upslst_first( g_pd->user_list );
    for ( ; l_ptr; l_ptr = l_ptr->next ) {
      (void) put_key( 0, l_ptr->data );
    }
  }

  return 1;
}

/*-----------------------------------------------------------------------
 * put_inst_keys
 *
 * Will print and format passed key and val for a ups file instance.
 * It will not print anything if val is empty.
 *
 * Input : char *, arrays of indexes to print
 * Output: none
 * Return: int, 1 fine, 0 not fine
 */
int put_inst_keys( int * ikeys, 
		   t_upstyp_instance * const inst, 
		   int do_act ) 
{
  t_upskey_map *map =  0;
  t_upstyp_instance *sav_inst = inst->sav_inst;
  t_upslst_item *l_act;
  t_upstyp_action *p_act;
  char *val = 0;
  int do_actions = 0;
  int do_user_def = 0;
  
  for ( ; ikeys && *ikeys != -1; ikeys++ ) {

    /* we do actions and user keys outside the loop */

    if ( *ikeys == e_key_action && do_act ) {
      do_actions = 1;
      continue;
    }

    if ( *ikeys == e_key_user_defined ) {
      do_user_def = 1;
      continue;
    }

    map = &g_key_info[ *ikeys ];
    if ( !sav_inst ||  !(val = UPSKEY_INST2ARR( sav_inst )[ map->i_index ]) )
      val = UPSKEY_INST2ARR( inst )[ map->i_index ];

    /* statistics is the only ups key that has no value ... i hope */

    if ( *ikeys == e_key_statistics && val )
      (void) put_key( 0, "STATISTICS" );
    else
      (void) put_key( map->key, val );

    /* cosmetic */

    if ( *ikeys == e_key_qualifiers )
      g_imargin += 2;
  }

  /* write user defined key words */

  if ( do_user_def && inst->user_list ) {
    t_upslst_item *l_ptr = upslst_first( inst->user_list );
    for ( ; l_ptr; l_ptr = l_ptr->next ) {
      (void) put_key( 0, l_ptr->data );
    }
  }

  /* write actions */

  if ( do_actions ) {
    l_act = upslst_first( inst->action_list );
    for( ; l_act; l_act = l_act->next ) {
      p_act = (t_upstyp_action *)l_act->data;
      (void) write_action( p_act );
    }
  }

  return 1;
}

/*-----------------------------------------------------------------------
 * put_inst_keys_mask
 *
 * Will print and format passed key and val for a ups file instance using
 * and and and not and mask.
 * It will not print anything if val is empty.
 *
 * Input : int *, arrays of indexes to print
 *         int *, array, and mask
 *         int *, array, not mask
 * Output: none
 * Return: int, 1 fine, 0 not fine
 */
int put_inst_keys_mask( int * ikeys, 
			t_upstyp_instance * const inst, 
			int * ymask, 
			int * nmask) 
{
  t_upskey_map *map =  0;
  t_upstyp_instance *sav_inst = inst->sav_inst;
  t_upslst_item *l_act;
  t_upstyp_action *p_act;
  char *val = 0;
  int do_actions = 0;
  int do_user_def = 0;
  
  for ( ; ikeys && *ikeys != -1; ikeys++ ) {

    /* mask out/in */

    if ( (ymask && !ymask[ *ikeys ]) ||
	 (nmask && nmask[ *ikeys ]) )
      continue;

    /* we do actions and user keys outside the loop */

    if ( *ikeys == e_key_action ) {
      do_actions = 1;
      continue;
    }

    if ( *ikeys == e_key_user_defined ) {
      do_user_def = 1;
      continue;
    }

    map = &g_key_info[ *ikeys ];
    if ( !sav_inst ||  !(val = UPSKEY_INST2ARR( sav_inst )[ map->i_index ]) )
      val = UPSKEY_INST2ARR( inst )[ map->i_index ];

    /* statistics is the only ups key we know there has no value */

    if ( *ikeys == e_key_statistics && val )
      (void) put_key( 0, "STATISTICS" );
    else
      (void) put_key( map->key, val );

    /* cosmetic */

    if ( *ikeys == e_key_qualifiers )
      g_imargin += 2;
  }

  /* write user defined key words */

  if ( do_user_def && inst->user_list ) {
    t_upslst_item *l_ptr = upslst_first( inst->user_list );
    for ( ; l_ptr; l_ptr = l_ptr->next ) {
      (void) put_key( 0, l_ptr->data );
    }
  }

  /* actions */

  if ( do_actions ) {
    l_act = upslst_first( inst->action_list );
    for( ; l_act; l_act = l_act->next ) {
      p_act = (t_upstyp_action *)l_act->data;
      (void) write_action( p_act );
    }
  }

  return 1;
}

/*-----------------------------------------------------------------------
 * put_key
 *
 * Will print and format passed key and val.
 * It's using current value of margin (g_imargin) to indent text.
 * It will not print anything if val is empty.
 *
 * Input : char *, key string
 *         char *, value string 
 * Output: none
 * Return: int, 1 fine, 0 not fine
 */
int put_key( const char * const key, const char * const val )
{
  int i = 0;

  if ( !val && upsutl_stricmp( "QUALIFIERS", key ) )
    return 0;

  g_line_count++;

  for ( i=0; i<g_imargin; i++ )
    (void) fputc( ' ', g_fh );
  if ( key && strlen( key ) > 0 ) {
    (void) fprintf( g_fh, "%s = ", key );
    if ( !upsutl_stricmp( "QUALIFIERS", key ) ||
	 !upsutl_stricmp( "DESCRIPTION", key ) ||
	 !upsutl_stricmp( "AUTHORIZED_NODES", key ) )
      (void) fputc( '"', g_fh );
    (void) fprintf( g_fh, "%s", val );
    if ( !upsutl_stricmp( "QUALIFIERS", key ) ||
	 !upsutl_stricmp( "DESCRIPTION", key ) ||
	 !upsutl_stricmp( "AUTHORIZED_NODES", key ) )
      (void) fputc( '"', g_fh );      
  }
  else {
    (void) fprintf( g_fh, "%s", val );
  }

  (void) fputc( '\n', g_fh );
  
  return 1;
}
     
int is_start_key( void )
{
  if ( g_ikey == e_key_flavor ) 
    return 1;
  if ( g_ikey == e_key_group ) 
    return 1;
  if ( g_ikey == e_key_common ) 
    return 1;
  if ( g_ikey == e_key_file )
    return 1;
  /*
  if ( g_ikey == e_key_action ) 
    return 1;
  */

  return 0;
}

int is_stop_key( void )
{
  if ( is_start_key() )
    return 1;
  if ( g_ikey == e_key_end ) 
    return 1;
  if ( g_ikey < 0 )
    return 1;

  return 0;
}

/*
 * Utils
 */

int cfilei( void )
{
  /* strings of known file types */
  static char *s_ups_files[e_file_count] = {
    "VERSION",
    "TABLE",
    "CHAIN",
    "DBCONFIG",
    ""
  };

  int i;
  g_ifile = e_file_unknown;

  if ( !g_pd->file ) return g_ifile;

  /* for now, just a linear search */

  for( i=0; i<e_file_count; i++ ) {    
    if ( !upsutl_stricmp( g_pd->file, s_ups_files[i] ) ) {
      g_ifile = i;
      break;
    }
  }

  return g_ifile;
}

void trim_cache( void )
{
  if ( g_use_cache && g_ft )
    upstbl_trim( g_ft );
}

int trim_qualifiers( char * const str )
{
  /* qualifiers are now case sensitive
  int i; 
  */
  
  if ( !str || strlen( str ) <= 0 ) return 0;

  /* qualifiers are now case sensitive
  len = (int)strlen( str );
  for ( i=0; i<len; i++ )
    str[i] = (char)tolower( (int)str[i] );
  */
  
  (void) upsutl_str_remove( str, WCHARSQ );  
  (void) upsutl_str_sort( str, ':' );

  return (int)strlen( str );
}

int add_to_instance( t_upstyp_instance * const inst,
		     t_upstyp_instance * const inst_add )
{
  int *ikeys = upskey_inst_arr();
  char *val_add;
  int index = -1;

  if ( !inst || !inst_add )
    return 0;

  for ( ; *ikeys != -1; ikeys++ ) {
    
    index = g_key_info[*ikeys].i_index;

    if ( (val_add = UPSKEY_INST2ARR( inst_add )[ index ] ) ) {

      if ( *ikeys == e_key_action )
	continue;

      else if ( *ikeys == e_key_user_defined )
	inst->user_list = upslst_copy( inst_add->user_list );

      else {
	char *val;
	if ( (val = UPSKEY_INST2ARR( inst )[ index ]) ) {
	  upsmem_free( val );
	}
	UPSKEY_INST2ARR( inst )[ index ] = upsutl_str_create( val_add, ' ' );
      }
    }
  }

  /* actions */

  if ( inst_add->action_list )
    inst->action_list = add_to_action_list( inst->action_list, inst_add->action_list );

  /* the untranslated values */
  
  if ( inst_add->sav_inst ) {
    if ( ! inst->sav_inst )
      inst->sav_inst = ups_new_instance();      
    (void) add_to_instance( inst->sav_inst, inst_add->sav_inst );
  }

  return 1;
}

t_upslst_item *add_to_action_list( t_upslst_item * const list_to,
				   t_upslst_item * const list_from )
{
  t_upslst_item *l_ptr_f = upslst_first( list_from );
  t_upslst_item *l_ptr_t = list_to;

  if( !l_ptr_f )
    return l_ptr_t;

  for ( ; l_ptr_f; l_ptr_f = l_ptr_f->next ) {
    t_upstyp_action *a_ptr_f = (t_upstyp_action *)l_ptr_f->data;
    t_upstyp_action *a_ptr_t = ups_new_action();
    
    upsmem_inc_refctr( a_ptr_f->action );
    a_ptr_t->action = a_ptr_f->action;
    a_ptr_t->command_list = upslst_copy( a_ptr_f->command_list );
    l_ptr_t = upslst_add( l_ptr_t, a_ptr_t );
  }

  return upslst_first( l_ptr_t );
}

int *find_common_mask_list( t_upslst_item *l_inst,
			    int * const keys_arr )
{
  static int c_mask[ e_key_count ];
  t_upslst_item *l_i1 = upslst_first( l_inst );
  int *ikeys = keys_arr;
  int *c_mask_i;
  int i;

  /* set all keys to common */

  for ( i = 0; i < e_key_count; i++ )
    c_mask[ i ] = 1;

  for ( ; l_i1; l_i1 = l_i1->next ) {

    t_upstyp_instance *inst1 = (t_upstyp_instance *)l_i1->data;
    t_upslst_item *l_i2 = l_i1->next;

    for ( ; l_i2; l_i2 = l_i2->next ) {

      t_upstyp_instance *inst2 = (t_upstyp_instance *)l_i2->data;

      /* check for common keys in table instances */

      c_mask_i = find_common_mask( inst1, inst2, ikeys );

      /* set keys uncommon if no match */

      for ( i = 0; i < e_key_count; i++ ) {
	if ( ! c_mask_i[ i ] )
	  c_mask[ i ] = 0;

      }
    }
  }

  return c_mask;
}

int *find_common_mask( t_upstyp_instance * const inst1,
		       t_upstyp_instance * const inst2,
		       int * const keys_arr )
{
  static int c_mask[ e_key_count ];
  int *ikeys = keys_arr;
  t_upskey_map *map;
  char *val1, *val2;
  int i;

  for ( i = 0; i < e_key_count; i++ )
    c_mask[ i ] = 0;

  for( ; *ikeys != -1; ikeys++ ) {

    /* note: flavor and qualifier will never go common,
       file reading don't like that */

    if ( *ikeys <= e_key_qualifiers || 
	 *ikeys == e_key_action ||
	 g_key_info[ *ikeys ].i_index == INVALID_INDEX )
      continue;

    map = &g_key_info[ *ikeys ];
	
    if ( !map )
      continue;

    if ( (val1 = UPSKEY_INST2ARR( inst1 )[ map->i_index ]) && 
	 (val2 = UPSKEY_INST2ARR( inst2 )[ map->i_index ]) ) {

      /* set keys common */

      if ( !upsutl_stricmp( val1, val2 ) )
	c_mask[ *ikeys ] = 1;

    }	
  }

  return c_mask;
}

int action_cmp ( const void * const d1, 
		 const void * const d2 )
{
  /* a little helper for cmp_actions */

  t_upstyp_action *a1 = (t_upstyp_action *)d1;
  t_upstyp_action *a2 = (t_upstyp_action *)d2;

  return upsutl_stricmp( a1->action, a2->action );
}

int cmp_actions( t_upslst_item *l_ptr1, 
		 t_upslst_item *l_ptr2 )
{
  l_ptr1 = upslst_first( l_ptr1 );
  l_ptr2 = upslst_first( l_ptr2 );
  
  if ( l_ptr1 == l_ptr2 )
    return 0;
  
  if ( l_ptr1 == 0 || l_ptr2 == 0 )
    return 1;

  if ( upslst_count( l_ptr1 ) != upslst_count( l_ptr2 ) )
    return 1;

  /* sort actions, so it's easier to compare */
  
  l_ptr1 = upslst_sort0( l_ptr1, action_cmp );
  l_ptr2 = upslst_sort0( l_ptr2, action_cmp );

  for ( ; l_ptr1 && l_ptr2; l_ptr1 = l_ptr1->next, l_ptr2 = l_ptr2->next ) {
    t_upstyp_action *a1 = (t_upstyp_action *)l_ptr1->data;
    t_upstyp_action *a2 = (t_upstyp_action *)l_ptr2->data;
    t_upslst_item *c1, *c2;

    /* same action name ? */
    
    if ( upsutl_stricmp( a1->action, a2->action ) )
      return 1;

    /* compare list of commands */
    
    c1 = upslst_first( a1->command_list );
    c2 = upslst_first( a2->command_list );
    
    if ( c1 == c2 )
      continue;
  
    if ( c1 == 0 || c2 == 0 )      
      return 1;

    if ( upslst_count( c1 ) != upslst_count( c2 ) )
      return 1;

    for ( ; c1 && c2; c1 = c1->next, c2 = c2->next ) {
      if ( strcmp( (char *)c1->data, (char *)c2->data ) )
	return 1;    
    }
  }

  /* if we came so far, they match */
  
  return 0;
}

t_upslst_item *find_group( t_upslst_item * const list_ptr, 
			   const char copt, 
			   int * const keys_arr )
{
  static t_upslst_item* l_orig = 0;

  t_upslst_item *l_grp = 0;
  t_upslst_item *l_itm = 0;
  t_upstyp_instance *inst = 0;

  switch ( copt ) {
    
  case 's':
    l_orig = list_ptr;
    return 0;
    
  case 'e':
    l_itm = upslst_first( l_orig );
    l_orig = 0;
    return l_itm;
  }
  
  if ( !l_orig ) 
    return 0;

  /* find a group */

  l_itm = l_orig;
  while ( !l_grp && l_itm && l_itm->next ) {
    inst = (t_upstyp_instance *)l_itm->data;
    l_itm = l_itm->next;
    
    while ( l_itm ) {

      t_upstyp_instance *is = (t_upstyp_instance *)l_itm->data;
      t_upskey_map *map;
      int goon = 0;
      int *ikeys;

      for ( ikeys = keys_arr; *ikeys != -1; ikeys++ ) {

	/* compare actions */

	if ( *ikeys == e_key_action ) {

	  goon = 
	    !cmp_actions( inst->action_list, is->action_list );

	}

	/* compare strings ... we hope */

	else if ( (map = &g_key_info[ *ikeys ]) ) {

	  char *val1, *val2;
	  goon = 
	    ((val1 = UPSKEY_INST2ARR( inst )[ map->i_index ]) && 
	    (val2 = UPSKEY_INST2ARR( is )[ map->i_index ]) &&
	    !upsutl_stricmp( val1, val2 ));

	}

	if ( !goon )
	  break;
      }
      
      if ( goon ) {
	l_grp = upslst_add( l_grp, is );
	l_itm = upslst_delete_safe( l_itm, is, ' ' );
      }
      else {
	l_itm = l_itm->next;
      }
    }
  }

  /* if any group, insert first instance in group
     and remove it from original list. */
  
  if ( l_grp ) {
    l_grp = upslst_insert( l_grp, inst );
    l_orig = upslst_delete( l_orig, inst, ' ' );
  }

  return l_grp;    
}

/*
 * Print stuff
 */

void print_instance( t_upstyp_instance * const inst_ptr )
{
  t_upslst_item *l_ptr = 0;
  
  if ( !inst_ptr ) return;
  
  (void) printf( "\nproduct = %s\nversion = %s\nflavor = %s\nqualifiers = %s\n",
	  inst_ptr->product, inst_ptr->version, inst_ptr->flavor,
	  inst_ptr->qualifiers );
  
  (void) printf( "chain = %s\ndeclarer = %s\ndeclared = %s\n",
	  inst_ptr->chain, inst_ptr->declarer, inst_ptr->declared );
  
  (void) printf( "modifier = %s\nmodified= %s\n",
	  inst_ptr->modifier, inst_ptr->modified );
  
  (void) printf( "origin = %s\nprod_dir = %s\nups_dir = %s\n",
	  inst_ptr->origin, inst_ptr->prod_dir, inst_ptr->ups_dir );
  
  (void) printf( "table_dir = %s\ntable_file = %s\narchive_file = %s\nauthorized_nodes = %s\n",
	  inst_ptr->table_dir, inst_ptr->table_file, inst_ptr->archive_file,
	  inst_ptr->authorized_nodes );
  
  (void) printf( "description = %s\n",
	  inst_ptr->description );

  if ( inst_ptr->action_list ) {
    (void) printf( "Actions = \n" );
    l_ptr = upslst_first( inst_ptr->action_list );
    for ( ; l_ptr; l_ptr = l_ptr->next ) {
      (void) print_action( l_ptr->data );
    }
  }
  else {
    (void) printf( "Actions = NULL\n" );
  }
  
  if ( inst_ptr->user_list ) {
    (void) printf( "User Defined = \n" );
    l_ptr = upslst_first( inst_ptr->user_list );
    for ( ; l_ptr; l_ptr = l_ptr->next ) {
      (void) printf( "%s\n", (char *) l_ptr->data );
    }
  }
  else {
    (void) printf( "User Defined = NULL\n" );
  }
}

void print_action( t_upstyp_action * const act_ptr )
{
  t_upslst_item *l_ptr = 0;
  
  if ( !act_ptr ) return;

  (void) printf ( "action = %s\n", act_ptr->action );
  
  if ( act_ptr->command_list ) {
    (void) printf( "Command list = \n" );
    l_ptr = upslst_first( act_ptr->command_list );
    for ( ; l_ptr; l_ptr = l_ptr->next ) {
      (void) printf( "   %s\n", (char*)l_ptr->data );
    }
  }
  else {
    (void) printf( "Command list = NULL\n" );
  }
}

void g_print_product( t_upstyp_product * const prod_ptr )
{
  t_upslst_item *l_ptr = 0;

  l_ptr = upslst_first( prod_ptr->comment_list );
  for ( ; l_ptr; l_ptr = l_ptr->next ) {
    (void) printf( "%s\n", (char *)l_ptr->data ); 
  }
  
  (void) printf( "\nfile    = %s\n", prod_ptr->file );
  (void) printf( "product = %s\n", prod_ptr->product );
  (void) printf( "version  = %s\n", prod_ptr->version );
  (void) printf( "chain  = %s\n", prod_ptr->chain );
  (void) printf( "instance_list:\n" );
  
  l_ptr = upslst_first( prod_ptr->instance_list );
  for ( ; l_ptr; l_ptr = l_ptr->next ) {
    t_upstyp_instance *inst_ptr = (t_upstyp_instance *)l_ptr->data;
    print_instance( inst_ptr );
  }

  if ( prod_ptr->config )
    upskey_conf_print( prod_ptr->config ); 
}
