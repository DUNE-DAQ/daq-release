/*
 *
 * FILE:
 *       upsget.c
 * 
 * DESCRIPTION: 
 *       This file contains routines to translate ups support variables
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
 *       12-Nov-1997, DjF, first
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

/* ups specific include files */

#include "ups.h"
#include "upsuname.h"
 
/*
 * Declaration of private functions.
 */

static void shutup (const t_upstyp_db * const db_info_ptr,
                    const t_upstyp_matched_instance * const instance,
                    const t_upsugo_command * const command_line);

#define SHUTUP \
  if ((&bit_bucket + bit_bucket_offset == 0) && 0) shutup (db_info_ptr, instance, command_line);

/*
 * Definition of global variables.
 */

static long bit_bucket = 0;
static long bit_bucket_offset = 0;

/*
 * Definition of public variables.
 */

#define g_UPSGET "UPSGET: "
#define UPSPRE "${UPS_"
#define USERKEY "${_"
#define TILDE "~"
static char g_buffer[FILENAME_MAX+1];

#define get_element(STRING,ELEMENT)              \
{ STRING=0;                                      \
  if(instance->chain)                            \
  { if (instance->chain->ELEMENT)                \
    { STRING=instance->chain->ELEMENT;           \
    } else {                                     \
      if(instance->version)                      \
      { if (instance->version->ELEMENT)          \
        { STRING=instance->version->ELEMENT;     \
        } else {                                 \
          if (instance->table)                   \
          { if (instance->table->ELEMENT)        \
            { STRING=instance->table->ELEMENT; } \
          }                                      \
        }                                        \
      } else {                                   \
        if (instance->table)                     \
        { if (instance->table->ELEMENT)          \
          { STRING=instance->table->ELEMENT; }   \
        }                                        \
      }                                          \
    }                                            \
  } else {                                       \
    if(instance->version)                        \
    { if (instance->version->ELEMENT)            \
      { STRING=instance->version->ELEMENT;       \
      } else {                                   \
        if (instance->table)                     \
        { if (instance->table->ELEMENT)          \
          { STRING=instance->table->ELEMENT; }   \
        }                                        \
      }                                          \
    } else {                                     \
      if (instance->table)                       \
      { if (instance->table->ELEMENT)            \
        { STRING=instance->table->ELEMENT; }     \
      }                                          \
    }                                            \
  }                                              \
}


char *upsget_translation_tilde( char * const oldstr );
 
typedef char * (*var_func)(const t_upstyp_db * const db_info_ptr,
                           const t_upstyp_matched_instance * const instance,
                           const t_upsugo_command * const command_line );

typedef struct s_var_sub {
  char *string;
  var_func func;
  int include_in_env;
  int unset_if_null;
  int quote_string;
} t_var_sub;

#define DO_INCLUDE_IN_ENV 1
#define DO_NOT_INCLUDE_IN_ENV 0
#define DO_UNSET_IF_NULL 1
#define DO_NOT_UNSET_IF_NULL 0
#define DO_QUOTE_STRING 1
#define DO_NOT_QUOTE_STRING 1        /* Always quote, for now */

static t_var_sub g_var_subs[] = {
  { "${UPS_PROD_NAME_UC", upsget_product_uc, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_PROD_NAME", upsget_product, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_PROD_VERSION", upsget_version, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_QUOTE_STRING },
  { "${UPS_PROD_DIR", upsget_prod_dir, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_QUOTE_STRING },
  { "${UPS_UPS_DIR", upsget_ups_dir, DO_INCLUDE_IN_ENV, DO_UNSET_IF_NULL, DO_QUOTE_STRING },
  { "${UPS_VERBOSE", upsget_verbose, DO_INCLUDE_IN_ENV, DO_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_EXTENDED", upsget_extended, DO_INCLUDE_IN_ENV, DO_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_THIS_DB", upsget_this_db, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_OS_FLAVOR", upsget_OS_flavor, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_PROD_FLAVOR", upsget_flavor, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_REQ_QUALIFIERS", upsget_reqqualifiers, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_PROD_QUALIFIERS", upsget_qualifiers, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_SHELL", upsget_shell, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_OPTIONS", upsget_options, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_QUOTE_STRING },
  { "${UPS_SETUP", upsget_envstr, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_COMPILE", upsget_compile, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_ORIGIN", upsget_origin, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_SOURCE", upsget_source, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${PRODUCTS", upsget_database, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
  { "${UPS_REQ_CHAIN", upsget_reqchain, DO_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },
/*  { "${UPS_ACTION", upsget_action, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_ARCHIVE_FILE", upsget_archive_file, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_AUTHORIZED_NODES", upsget_authorized_nodes, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_CATMAN_SOURCE_DIR", upsget_catman_source_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_CATMAN_TARGET_DIR", upsget_catman_target_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_CHAIN", upsget_chain, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_COMPILE_DIR", upsget_compile_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_COMPILE_FILE", upsget_compile_file, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_DB_VERSION", upsget_db_version, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_DECLARED", upsget_declared, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_DECLARER", upsget_declarer, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_DESCRIPTION", upsget_description, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_FILE", upsget_file, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_HTML_SOURCE_DIR", upsget_html_source_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_HTML_TARGET_DIR", upsget_html_target_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_INFO_SOURCE_DIR", upsget_info_source_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_INFO_TARGET_DIR", upsget_info_target_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_MAN_SOURCE_DIR", upsget_man_source_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_MAN_TARGET_DIR", upsget_man_target_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_MODIFIED", upsget_modified, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_MODIFIER", upsget_modifier, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_NEWS_SOURCE_DIR", upsget_news_source_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_NEWS_TARGET_DIR", upsget_news_target_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_ORIGIN", upsget_origin, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_PROD_CHAIN", upsget_prod_chain, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_PROD_DIR_PREFIX", upsget_prod_dir_prefix, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_SETUPS_DIR", upsget_setups_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_SHELL", upsget_shell, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_SOURCE", upsget_source, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_STATISTICS", upsget_statistics, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_TABLE_DIR", upsget_table_dir, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
/*  { "${UPS_TABLE_FILE", upsget_table_file, DO_NOT_INCLUDE_IN_ENV, DO_NOT_UNSET_IF_NULL, DO_NOT_QUOTE_STRING },  */
  {  0, 0, 0, 0, 0 }
} ;


/*
 * Definition of public functions.
 */
void upsget_remall(const FILE * const stream, 
                    const t_upsugo_command * const command_line,
		    const char * const prefix )
{
  char *pdefault;
  char *unset;
  static char unset_b [] = "unset";
  static char unset_c [] = "unsetenv";
  int idx;

  if (prefix)
  {
    pdefault = (char *)prefix;
  } else
  {
    pdefault = "";
  }

/* Figure out how we should unset our variables */

  switch (command_line->ugo_shell) {
  case e_BOURNE:
    unset = unset_b;
    break;
  case e_CSHELL:
    unset = unset_c;
    break;
  default:
    unset = 0;
    upserr_add(UPS_NOSHELL, UPS_WARNING, UPS_UNKNOWN_TEXT);
    break;
  }

/* Run through the list of candidates and eliminate the right ones */

  for (idx=0; g_var_subs[idx].string!=0; idx++) 
  {
    if (g_var_subs[idx].include_in_env)
    {
      (void) fprintf((FILE *)stream,"%s%s %s\n", pdefault, unset,
                     g_var_subs[idx].string+2);
    }
  }
}

void upsget_envout(const FILE * const stream, 
                    const t_upstyp_db * const db,
                    const t_upstyp_matched_instance * const instance,
                    const t_upsugo_command * const command_line )
{ char *name;
  get_element(name,product);
  if (!name)
  { name = command_line->ugo_product;
    upserr_add(UPS_NO_KEYWORD, UPS_INFORMATIONAL, "for this version", 
               "PRODUCT", name );
   /* put out info here !!! */
  }
  name = upsutl_upcase( name );
  if( (UPS_VERBOSE) ) upsver_mes( 3, "UPSGET: got upcased product name %s\n", name);
  switch (command_line->ugo_shell) {
  case e_BOURNE:
    (void) fprintf((FILE *)stream,"SETUP_%s='%s'; export SETUP_%s\n#\n",
      name,upsget_envstr(db,instance,command_line),name);
    break;
  case e_CSHELL:
    (void) fprintf((FILE *)stream,"setenv SETUP_%s '%s'\n#\n",
    name,upsget_envstr(db,instance,command_line));
    break;
  default:
    upserr_add(UPS_INVALID_SHELL, UPS_FATAL, UPS_UNKNOWN_TEXT);
    break;
  }
}
 
char *
setup_var(char *name) {
  static char svarbuf[1024];
  char *pc;
  sprintf(svarbuf, "SETUP_%s", name);
  for (pc = svarbuf; *pc; pc++) {
     *pc = (char )toupper((int )*pc);
  }
  if( (UPS_VERBOSE) ) upsver_mes( 3, "UPSGET: made setup var %s from %s\n", svarbuf, name);
  return svarbuf;
}

void upsget_allout(const FILE * const stream, 
                    const t_upstyp_db * const db,
                    const t_upstyp_matched_instance * const instance,
                    const t_upsugo_command * const command_line,
		    const char * const prefix )
{ 
  char *name;
  char *pdefault;
  int idx;

  if (prefix)
  { 
    pdefault = (char *)prefix;
  } else {
    pdefault = "";
  }

  get_element(name,product);
  if (!name)
  { name = command_line->ugo_product;
    upserr_add(UPS_NO_KEYWORD, UPS_INFORMATIONAL, "for this version", 
             "PRODUCT", name );           /* put out info here !!! */
  }

  switch (command_line->ugo_shell) {
  case e_BOURNE:
  case e_CSHELL:
    for (idx=0; g_var_subs[idx].string!=0; idx++)
    {
      if (g_var_subs[idx].include_in_env)
      {
        char *the_string = g_var_subs[idx].string+2;
        char *a_quote = g_var_subs[idx].quote_string ? "'" : "";
        char *the_value = "";

        if (g_var_subs[idx].func)
        {
          the_value = g_var_subs[idx].func(db,instance,command_line);
          if (the_value == 0)
            the_value = "";
        }

        if ((!g_var_subs[idx].unset_if_null) || (*the_value != 0))
        {
          switch (command_line->ugo_shell) {
          case e_BOURNE:
            (void) fprintf((FILE *)stream,"%s%s=%s%s%s; export %s\n", pdefault, 
                           the_string, a_quote, the_value, a_quote, the_string);
            break;
      
          case e_CSHELL:
            (void) fprintf((FILE *)stream,"%ssetenv %s %s%s%s\n", pdefault, 
                           the_string, a_quote, the_value, a_quote);
            break;
          }
        }
        else
        {
          switch (command_line->ugo_shell) {
          case e_BOURNE:
            (void) fprintf((FILE *)stream,"%sunset %s\n", pdefault, the_string);
            break;
      
          case e_CSHELL:
            (void) fprintf((FILE *)stream,"%sunsetenv %s\n", pdefault, the_string);
            break;
          }
        }
      }
    }
    break;

  default:
    upserr_add(UPS_NOSHELL, UPS_WARNING, UPS_UNKNOWN_TEXT);
    break;
  } 
}

char *upsget_translation_env( char * const oldstr )
{
  /* it will translate env.var in the passed string. if any translation
     is done, it will return a pointer to a static string containing the 
     translated string. if no translation is done, it will return (null).

     Note: right now it is quite dumb, it will only recognize ${var},
     but should probably also recognize ${ var }, $var, maybe even
     ${var1_${var2}} ... */

  static char buf[MAX_LINE_LEN];
  static char *buf2;
  static char env[48];
  static char error[51];
  static char *s_tok = "${";
  static char *e_tok = "}";  
  int clear_flag=0;                     /* This makes Eli happy... */
  char *s_loc = oldstr;
  char *e_loc = 0;
  char *tr_env = 0;
  
  buf[0] = 0; /* reset buf[0] */

  while ( s_loc && *s_loc && (e_loc = strstr( s_loc, s_tok )) != 0 ) 
  { if(!clear_flag)                   
    { (void) memset( buf, 0, sizeof( buf ) ); /* clear ONLY if I need too... */
      clear_flag++;
    }
    (void) memset( env, 0, sizeof( env ) );   /* copy everything upto ${     */
    (void) strncat( buf, s_loc, (unsigned int )(e_loc - s_loc) );  
    if (!(s_loc = strstr( e_loc, e_tok )))    /* set s_loc to end (finding })*/
    { upserr_add(UPS_NO_TRANSLATION, UPS_FATAL, e_loc);
      return 0;                               /* NO matching }               */
    }
    e_loc += 2;  /* Skip over the ${ */       /* copy from there to } in env */
    (void) strncpy( env, e_loc, (unsigned int )(s_loc - e_loc) );    
    if ( (tr_env = (char *)getenv( env )) )
    { (void) strcat( buf, tr_env );
    } else {
      if (!strcmp(env,"UPS_THIS_DB"))                        /* will MODIFY */
      { tr_env = upsutl_str_create((char *)upsfil_last_file(),' ');
        if ((e_loc = strstr(tr_env,"/.upsfiles/dbconfig"))!=0) /* mustcreate */
        { *e_loc='\0';                         /* end before .upsfile */
          (void) strcat( buf, tr_env );               /* put in buffer */
          *e_loc='/';                          /* restore */
          upsmem_free(tr_env);                 /* free whole thing */
        } else { 
          (void) sprintf(error,"${%s}",env);
          upserr_add(UPS_NO_TRANSLATION, UPS_INFORMATIONAL, error);
        }
      } else {
        (void) sprintf(error,"${%s}",env);
        upserr_add(UPS_NO_TRANSLATION, UPS_INFORMATIONAL, error);
      }
    }
    /* move past the '}' */
    ++s_loc;    
  }
  if ( buf[0] ) 
  { if ( s_loc )
    { (void) strcat( buf, s_loc ); /* Something left after translation tack on */
    }
    if ((buf2=upsget_translation_tilde(buf))==0)
    { return buf;
    } else {
      return buf2;
    }
  } else {
    if ((buf2=upsget_translation_tilde(oldstr))!=0)
      return buf2;
  }
  return 0;
}
char *upsget_translation_tilde( char * const oldstr )
{
  static char buf[MAX_LINE_LEN];
  static char env[48];
  static char error[49];
  static char *e_tok = "/";  
  int clear_flag=0;                     /* This makes Eli happy... */
  char *s_loc = oldstr;
  char *e_loc = 0;
  char *tr_env = 0;
  
  buf[0] = 0; /* reset buf[0] */

  while ( s_loc && *s_loc && (e_loc = strstr( s_loc, TILDE)) != 0 ) 
  { if(!clear_flag)                   
    { (void) memset( buf, 0, sizeof( buf ) ); /* clear ONLY if I need too... */
      clear_flag++;
    }
    (void) memset( env, 0, sizeof( env ) );
    (void) strncat( buf, s_loc, (unsigned int )(e_loc - s_loc) );    /* copy everything upto ~      */
    if (!(s_loc = strstr( e_loc, e_tok )))   /* set s_loc to end (finding /)*/
    { s_loc = strstr( e_loc, " ");           /* set s_loc to end space */
    }
/*    e_loc++;  oops tilde_dir does that... Skip over the ~             */
    if (s_loc)
    { (void) strncpy( env, e_loc, (unsigned int )(s_loc - e_loc) );  /* copy from there to / in env */
    } else {
      (void) strcpy(env,e_loc);
    }
    if ( (tr_env = (char *)upsget_tilde_dir( env )) )
    { (void) strcat( buf, tr_env );
    } else {
      (void) sprintf(error,"~%s",env);
      upserr_add(UPS_NO_TRANSLATION, UPS_INFORMATIONAL, error);
    }
  }
  if ( buf[0] ) 
  { if ( s_loc )
    { (void) strcat( buf, s_loc ); /* Something left after translation tack on */
    }
    return buf;
  }
  return 0;
}
char *upsget_translation( const t_upstyp_matched_instance * const minstance,
			  const t_upstyp_db *const db_info_ptr,
			  const t_upsugo_command * const command_line,
			  char * const oldstr )
{
  static char newstr[4096];
  static char holdstr[4096];
  char * loc;
  char * upto;
  char * eaddr;             /* end addr of } */
  char * uservar;
  int count=0;
  int found=0;
  int idx=0;
  int any=0;
  char * value = NULL;
  char * work;
  newstr[0] = '\0';
  if (!minstance) { return(oldstr); }
  /* work = (char *) malloc((size_t)(strlen(oldstr) +1)); */
  work=holdstr;
  (void) strcpy(work,oldstr);
  upto = work;
  while ((loc = strstr(upto,UPSPRE))!= 0 ) 
  { count = ( loc - upto );
    (void) strncat(newstr,upto,(unsigned int )count);
    upto += count;
    eaddr =strchr(upto,'}');
    if ( !eaddr ) {
      upserr_vplace();
      upserr_add( UPS_NO_TRANSLATION, UPS_FATAL, oldstr );
      return oldstr;
    }
    *eaddr = '\0';
    found=0;
    for ( idx=0; g_var_subs[idx].string!=0; idx++) 
    { if (!strcmp(g_var_subs[idx].string,upto)) 
      { if (g_var_subs[idx].func)
        {  value=g_var_subs[idx].func(db_info_ptr,minstance,command_line);
           if(value) { (void) strcat(newstr,value); }
        }
        found=1;
        any++;
      }
    }
    if (!found) { (void) strcat(newstr,upto); (void) strcat(newstr,"}"); } 
    *eaddr = '}';
    upto =strchr(upto,'}') + 1;
  }
  if (any)
  { (void) strcat(newstr,upto);
  } else {
    (void) strcpy(newstr,work);
    /* free(work); now static */
  }
/* Support added 08/27/1998 DjF for _variable translation within table files */ 
  work=holdstr;
  (void) strcpy(work,newstr);
  upto = work;
  newstr[0] = '\0';
  any=0;
  while ((loc = strstr(upto,USERKEY))!= 0 ) 
  { count = ( loc - upto );
    (void) strncat(newstr,upto,(unsigned int )count);
    upto += count;
    eaddr =strchr(upto,'}');
    if ( !eaddr ) {
      upserr_vplace();
      upserr_add( UPS_NO_TRANSLATION, UPS_FATAL, oldstr );
      return oldstr;
    }
    *eaddr = '\0';
    found=0;
    value = NULL;
    uservar=upto+2;
    if (minstance->chain)
    {  value = upskey_inst_getuserval( minstance->chain,uservar);
       found=1;
    }
    if (minstance->version && !value )
    {  value = upskey_inst_getuserval( minstance->version,uservar);
       found=1;
    }
    if (minstance->table && !value )
    {  value = upskey_inst_getuserval( minstance->table,uservar);
       found=1;
    }
    if(value) 
    { (void) strcat(newstr,value);
      any++;
    }
    if (!found) { (void) strcat(newstr,upto); (void) strcat(newstr,"}"); } 
    *eaddr = '}';
    upto =strchr(upto,'}') + 1;
  }
  if (any)
  { (void) strcat(newstr,upto);
  } else {
    (void) strcpy(newstr,work);
    /* free(work); now static */
  }
/* End Modifications 08/27/1998 */
  /* work = (char *) malloc((size_t)(strlen(newstr) +1)); */
  work=holdstr;
  (void) strcpy(work,newstr);
  upto = work;
  newstr[0] = '\0';
  any=0;
  while ((loc = strstr(upto,"${PRODUCTS}"))!=0) 
  { count = ( loc - upto );
    (void) strncat(newstr,upto,(unsigned int )count);
    upto += count;
    eaddr =strchr(upto,'}');
    *eaddr = '\0';
    found=0;
    if (!strcmp("${PRODUCTS",upto)) 
    { value=upsget_database(db_info_ptr,minstance,command_line);
      if(value) { (void) strcat(newstr,value); }
      found=1;
      any++;
    } else { 
      (void) strcat(newstr,upto); 
      (void) strcat(newstr,"}");
    } 
    *eaddr = '}';
    upto = strchr(upto,'}') + 1;
  }
  if (any)
  { (void) strcat(newstr,upto);
  } else {
    (void) strcpy(newstr,work);
    /* free(work); no static */
  }
  /* work = (char *) malloc((size_t)(strlen(newstr) +1)); */
  work=holdstr;
  (void) strcpy(work,newstr);
  upto = work;
  newstr[0] = '\0';
  any=0;
  while ((loc = strstr(upto,TILDE))!=0) 
  { count = ( loc - upto );
    (void) strncat(newstr,upto,(unsigned int )count);
    upto += count;
    eaddr = strchr(upto,'/'); 
    *eaddr = '\0';
    found=0;
    if (strchr(upto,'~')) 
    { value=upsget_tilde_dir(upto);
      if(value) 
      { (void) strcat(newstr,value); 
        (void) strcat(newstr,"/");
      }
      found=1;
      any++;
    } else { 
      (void) strcat(newstr,upto); 
      (void) strcat(newstr,"/");
    } 
    *eaddr = '/';
    upto=eaddr+1;
    /* upto = strchr(upto,'/') + 1; */
  }
  if (any)
  { (void) strcat(newstr,upto);
  } else {
    (void) strcpy(newstr,work);
  }
  return newstr;
}

char *upsget_envstr(const t_upstyp_db * const db_info_ptr,
                    const t_upstyp_matched_instance * const instance,
                    const t_upsugo_command * const command_line )
{
  static char newstr[4096];
  static char *string = 0;
  char *current_setup;
  get_element(string,product);
  if (!string)
  { string=command_line->ugo_product;
    upserr_add(UPS_NO_KEYWORD, UPS_INFORMATIONAL, "for this version", 
               "PRODUCT", string );
   /* put out info here !!! */
  }
  current_setup = getenv(setup_var(string));
  (void) strcpy(newstr,string);
  (void) strcat(newstr," ");
  get_element(string,version);
  if (string) 
  { (void) strcat(newstr,string);
  }
  (void) strcat(newstr," -f ");
  get_element(string,flavor);
  (void) strcat(newstr,string);
  if ( db_info_ptr )
  { (void) strcat(newstr," -z ");
    (void) strcat(newstr,db_info_ptr->name);
  }
  get_element(string,qualifiers);
  if (string && *string != '\0')
  { (void) strcat(newstr," -q ");
    (void) strcat(newstr,string);
  }
/*  if (command_line->ugo_v)						*/
/*  { (void) sprintf(newstr + strlen (newstr)," -%*.*s",		*/
/*                   command_line->ugo_v, command_line->ugo_v,		*/
/*                   "vvvvvvvvvv"); }					*/
  if ( command_line->ugo_j)
  { (void) strcat(newstr," -j"); }
  if ( command_line->ugo_r)
  { (void) strcat(newstr," -r ");
    (void) strcat(newstr,command_line->ugo_productdir);
  }
  if ( command_line->ugo_m)
  { (void) strcat(newstr," -m ");
    (void) strcat(newstr,command_line->ugo_tablefile);
  }
  if ( command_line->ugo_M)
  { (void) strcat(newstr," -M ");
    (void) strcat(newstr,command_line->ugo_tablefiledir);
  }
  if ( command_line->ugo_O)
  { (void) strcat(newstr," -O \"");
    (void) strcat(newstr,command_line->ugo_options);
    (void) strcat(newstr,"\"");
  }
  if ( command_line->ugo_e)
  { (void) strcat(newstr," -e"); }

  if( (UPS_VERBOSE) ) upsver_mes( 3, "UPSGET: ugoB %d new %s setup %s\n", g_command_ugoB, current_setup ? current_setup : "(null)", newstr);
  if ( g_command_ugoB && current_setup && 0 != strcmp(current_setup, newstr)) {
    upserr_add( UPS_SETUP_CONFLICT, UPS_FATAL, string, "full descriptions", current_setup, newstr);
  }
  return newstr;
}

char *upsget_archive_file(const t_upstyp_db * const db_info_ptr,
                          const t_upstyp_matched_instance * const instance,
                          const t_upsugo_command * const command_line,
                          const int strip )
{ static char *string;
  static char *env_string;
  static char buffer[1];
  static char *nostring=buffer;
  buffer[0]='\0';
  if (command_line && command_line->ugo_archivefile)
  { string=command_line->ugo_archivefile;
  } else { 
    get_element(string,archive_file);
  }
  if (!string)
  { string=buffer;
  } else {
    if((env_string=upsget_translation_env(string))!=0)
    { string=env_string;
    }
    if (strip)
    { if (!upsutl_strincmp(string,"ftp://",6))
      { (void) strncpy(string,"123456",6); /* space ftp:// */
        if (strchr(string,'/'))
        { string=upsutl_str_create(strchr(string,'/'),'p');
        } else { 
          string=upsutl_str_create(string,' ');
        }
      }
    } else {
      string=upsutl_str_create(string,' ');
    }
  }
  SHUTUP;
  return ( string ? string : nostring ) ;
}

char *upsget_prod_dir(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char *string;
  static char *prefix_string;
  static char buffer[1];
  static char *nostring=buffer;
  static char *env_string;
  buffer[0]='\0';
  if (command_line && command_line->ugo_r)
  { string=command_line->ugo_productdir;
  } else { 
    get_element(string,prod_dir);
  }
  if (!string)             /* The only way this could happen is if you */
  { /* string=buffer; */   /* declare a product with no -r.  That's okay */
  } else {
    if((env_string=upsget_translation_env(string))!=0)
    { string=env_string;
    }
  }
  if (UPSRELATIVE(string))
  { if (db_info_ptr->config)
    { if (db_info_ptr->config->prod_dir_prefix)
      { prefix_string = upsutl_str_create(db_info_ptr->config->prod_dir_prefix,' '); 
        prefix_string = upsutl_str_crecat(prefix_string,"/"); 
        prefix_string = upsutl_str_crecat(prefix_string,string); 
        return prefix_string;
      }
    }
  }
  return ( string ? string : nostring ) ;
}

char *upsget_ups_dir(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char *string;
  static char *prefix_string;
  static char *env_string;
  static struct stat sbuf;
  string = 0;

  if (command_line && command_line->ugo_U)
  { string=command_line->ugo_upsdir;
  } else 
  { get_element(string,ups_dir);
  }
  if ( 0 == string ) 
  { prefix_string = upsget_prod_dir(db_info_ptr, instance, command_line );
    prefix_string = upsutl_str_crecat(prefix_string,"/ups"); 
    if (0 == stat(prefix_string, &sbuf) && S_ISDIR(sbuf.st_mode))
    { return prefix_string;
    }
  }
  if (string)            
  { if((env_string=upsget_translation_env(string))!=0)
    { string=env_string;
    }
  }
  if (UPSRELATIVE(string))
  { prefix_string = upsget_prod_dir(db_info_ptr, instance, command_line );
    prefix_string = upsutl_str_crecat(prefix_string,"/"); 
    prefix_string = upsutl_str_crecat(prefix_string,string); 
    return prefix_string;
  }
  return string;
} 

char *upsget_compile(const t_upstyp_db * const db_info_ptr,
                     const t_upstyp_matched_instance * const instance,
                     const t_upsugo_command * const command_line )
{ 
  static char newstr[4096];
  static char *string = 0;
  static char *env_string;
  static char *slash = "/";
  static char *dot = ".";
  char *divider = slash;
  *newstr='\0';
  if (!command_line->ugo_b) /* did they specify a compile file */
  /* we do not have a compile_file on the command line */
  { if (command_line->ugo_u)
    /* we have a compile_dir on the command line */
    { string=command_line->ugo_compile_dir;
      if((env_string=upsget_translation_env(string))!=0)
      { string=env_string;
      }
      (void) strcpy(newstr,string); 
    } else {
      /* we do not have a compile_dir on the command line, check from file */
      get_element(string,compile_dir);
      if (string)
      /* we found a compile_dir in the file */
      { if((env_string=upsget_translation_env(string))!=0)
	{ string=env_string;
	}
        (void) strcpy(newstr,string);
      } else {
	/* no compile_dir in the file, we need to use the default */
	string=upsget_product(db_info_ptr, instance, command_line);
	(void) sprintf(newstr,"%s/%s", db_info_ptr->name, string);
      }
    }
    get_element(string,compile_file);
    if (string) 
    /* there was a compile_file in the file */
    { (void) strcat(newstr,slash);
      (void) strcat(newstr,string);
    } else {
      /* there was no compile_file in the file, use the default */
      get_element(string, version);
      if (string)
      { (void) strcat(newstr,divider);
        (void) strcat(newstr,string);
	divider = dot;
      }
      get_element(string, flavor);
      if (string)
      { (void) strcat(newstr,divider);
        (void) strcat(newstr,string);
        divider = dot;
      }
      (void) sprintf(newstr, "%s%s%s", newstr, divider, "compile");
    }
  } else {
    /* we have a compile_file specified on the command line */
    if (command_line->ugo_u)
    /* we have a compile_dir on the command line */
    { string=command_line->ugo_compile_dir;
      if((env_string=upsget_translation_env(string))!=0)
      { string=env_string;
      }
      (void) strcpy(newstr,string); 
      (void) strcpy(newstr,slash); 
      (void) strcpy(newstr,command_line->ugo_compile_file); 
    } else {
      /* we do not have a compile_dir on the command line, check from file */
      get_element(string,compile_dir);
      if (string)
      /* we found a compile_dir in the file */
      { if((env_string=upsget_translation_env(string))!=0)
	{ string=env_string;
	}
        (void) sprintf(newstr, "%s/%s", string, command_line->ugo_compile_file); 
      } else {
	/* no compile_dir in the file, we need to use the default */
	string=upsget_product(db_info_ptr, instance, command_line);
	(void) sprintf(newstr,"%s/%s/%s", db_info_ptr->name, string, 
		command_line->ugo_compile_file);
      }
    }
  }
  return newstr;
}
char *upsget_origin(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char *string;
  get_element(string,origin);
  SHUTUP;
  return string;
}
char *upsget_product(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char *string;
  get_element(string,product);
  SHUTUP;
  return string;
}
char *upsget_product_uc(const t_upstyp_db * const db_info_ptr,
			const t_upstyp_matched_instance * const instance,
			const t_upsugo_command * const command_line )
{ static char *string;
  static char newstr[4096];

  get_element(string,product);
  strcpy( newstr, upsutl_upcase(string) );
  SHUTUP;
  return newstr;
}
char *upsget_version(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{
  static char *string="";
  get_element(string,version);
  SHUTUP;
  return string;
}
char *upsget_flavor(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char *string;
  get_element(string,flavor);
  SHUTUP;
  return string;
}
char *upsget_qualifiers(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char *string;
  get_element(string,qualifiers);
  SHUTUP;
  return string;
}

char *upsget_reqqualifiers(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char *string;

  string = command_line->ugo_reqqualifiers;

  SHUTUP;
  return string;
}

char *upsget_reqchain(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char *string;

  string = command_line->ugo_reqchain;

  SHUTUP;
  return string;
}

char *upsget_shell(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char SH[]="sh";
  static char CSH[]="csh";
  char *answer;
  switch (command_line->ugo_shell) {
  case e_BOURNE:
    answer = SH;
    break;
  case e_CSHELL:
    answer = CSH;
    break;
  default:
    answer = 0;
    break;
  }
  SHUTUP;
  return (answer);
}
char *upsget_source(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char SH[]=".";
  static char CSH[]="source";
  char *answer;
  switch (command_line->ugo_shell) {
  case e_BOURNE:
    answer = SH;
    break;
  case e_CSHELL:
    answer = CSH;
    break;
  default:
    answer = 0;
    break;
  }
  SHUTUP;
  return (answer);
}
char *upsget_verbose(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char string[2];
  static char NOT[]="";
  if (command_line->ugo_v)
  { (void) sprintf(string,"%.1d",command_line->ugo_v);
    return (string);
  } 
  SHUTUP;
  return(NOT);
}
char *upsget_extended(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char EXTENDED[]="1";
  static char NOT[]="";
  if (command_line->ugo_e)
  { return(EXTENDED);
  }
  SHUTUP;
  return(NOT);
}
char *upsget_options(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char NOT[]="";
  static char *string;
  if (command_line->ugo_O)
  { string=command_line->ugo_options;
  } else {
    string=NOT;
  }
  SHUTUP;     
  return(string);
}
char *upsget_database(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ t_upslst_item *db_list;
  t_upstyp_db *db;
  static char buffer[2028];
  static char *string=buffer;
  int count=0;
  buffer[0]='\0';
  for ( db_list=command_line->ugo_db; db_list; 
        db_list = db_list->next, count++ )
  { db=db_list->data;
    if(count) (void) strcat(string,":");
    (void) strcat(string,db->name);
  }
  SHUTUP;     
  return(string);     
} 
char *upsget_tilde_dir(char * const addr)
{
  struct passwd *pdp;
  static char buffer[FILENAME_MAX+1];
  char *name;
  buffer[0]='\0';
  if(strlen(addr)==1)
  { pdp = getpwuid(getuid());
    (void) strcpy(buffer,pdp->pw_dir);
  } else { 
    name=addr+1;
    pdp = getpwnam(name);
    if (pdp)
    { (void) strcpy(buffer,pdp->pw_dir);
    }
  }
  return(buffer);
}

char *upsget_this_db(const t_upstyp_db * const db_info_ptr,
                      const t_upstyp_matched_instance * const instance,
                      const t_upsugo_command * const command_line )
{ static char NOT[]="";
  static char *string;
/*  if (command_line->ugo_z) 
  { t_upslst_item *db_list=upslst_first(command_line->ugo_db);
    string=db_list->data-;
  } else {
*/
/* If there is no db_info_ptr they must have specified a -M table_file
   on the command line - or match is broken..;)
*/
    if ( db_info_ptr )
    { string=db_info_ptr->name;
    } else {
      string=NOT;
    }
  SHUTUP;
  return(string);
}
char *upsget_OS_flavor(const t_upstyp_db * const db_info_ptr,
                       const t_upstyp_matched_instance * const instance,
                       const t_upsugo_command * const command_line )
{  
    if (command_line->ugo_osname == 0)
	upsugo_bldfvr( command_line );
    return command_line->ugo_osname->data;
}

/*  upsget_chain_file
 *
 * Read the specified chain file.
 *
 * Input : a database
 *         a product name
 *         a chain name
 * Output: pointer to the buffer with the file path in it
 * Return: a product structure read from the file
 */
t_upstyp_product *upsget_chain_file(const char * const a_db,
                                    const char * const a_prod,
                                    const char * const a_chain,
                                    char ** const a_buffer)
{
  int file_chars = 0;
  static char buffer[FILENAME_MAX+1];
  t_upstyp_product *read_product = NULL;

  file_chars = (int )(strlen(a_chain) + strlen(a_prod) + strlen(a_db) + 
               sizeof(CHAIN_SUFFIX) + 4);
  if (file_chars <= FILENAME_MAX) {
    (void) sprintf(buffer, "%s/%s/%s%s", a_db, a_prod, a_chain, CHAIN_SUFFIX);
    read_product = upsfil_read_file(&buffer[0]);
    *a_buffer = buffer;
  } else {
    upserr_vplace();
    upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, file_chars);
    *a_buffer = 0;
  }

  return(read_product);
}
/*  upsget_version_file
 *
 * Read the specified version file.
 *
 * Input : a database
 *         a product name
 *         a version 
 * Output: pointer to the buffer with the file path in it
 * Return: a product structure read from the file
 */
t_upstyp_product *upsget_version_file(const char * const a_db,
                                      const char * const a_prod,
                                      const char * const a_version,
                                      char ** const a_buffer)
{
  int file_chars = 0;
  static char buffer[FILENAME_MAX+1];
  t_upstyp_product *read_product = NULL;

  file_chars = (int )(strlen(a_version) + strlen(a_prod) + strlen(a_db) + 
               sizeof(VERSION_SUFFIX) + 4);
  if (file_chars <= FILENAME_MAX) {
    (void) sprintf(buffer, "%s/%s/%s%s", a_db, a_prod, a_version, VERSION_SUFFIX);
    read_product = upsfil_read_file(&buffer[0]);
    read_product = upsfil_read_file(&buffer[0]);
    *a_buffer = buffer;
  } else {
    upserr_vplace();
    upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, file_chars);
    *a_buffer = 0;
  }

  return(read_product);
}

#define HASHATOM( k ) ((unsigned int)(k)>>2)

int upsget_key(const t_upstyp_instance * const instance)
{ const char *key=0;
  t_upstbl *g_ft = 0;
  static char buffer[MAX_LINE_LEN];
  char *skey=buffer;
  int i;
  static char *nostring = "";
  (void) sprintf(skey,"%s%s%s%s",(instance->product ? instance->product : nostring),
                          (instance->version ? instance->version : nostring),
                          instance->flavor,
                          instance->qualifiers);
  if ( !g_ft )
  { g_ft = upstbl_new( 300 ); }
  key = upstbl_atom_string( skey );
  i = (int)HASHATOM( key )%g_ft->size;
  return(i);
}

/*-----------------------------------------------------------------------
 * upsget_man_source_dir
 *
 * Return a string containing the location of the products' man pages
 *
 * Input : an instance, and db configuration info
 * Output: none
 * Return: static string containing the location of the products' man pages
 */
char *upsget_man_source_dir( const t_upstyp_matched_instance * const a_inst,
                             const t_upstyp_db * const a_db_info,
                             const t_upsugo_command * const uc)
{
  t_upstyp_instance *vinst;
  t_upstyp_instance *tinst;
  char *man_source;                        /* man_source_dir translated      */
  char *prod_dir;                          /* full path to product           */

  g_buffer[0] = '\0';                      /* we want to fill this anew      */
 
  if ((tinst = a_inst->table))             /* See if we have a table file    */
  { if (tinst->man_source_dir)             /* Is the source in the table file*/
    { man_source =                         /* will return value translated   */
         upsget_translation(a_inst, a_db_info,               /* if necessary */
                            0, tinst->man_source_dir);
      if (UPSRELATIVE(man_source))         /* Is it a relative path?         */
      { prod_dir=upsget_prod_dir(a_db_info, a_inst, uc);   
	(void) strcat(g_buffer, prod_dir);
	(void) strcat(g_buffer, "/");
      } 
      (void) strcat(g_buffer,man_source);
      (void) strcat(g_buffer, "/");               /* safe then sorry...             */
      return ((char *)(&g_buffer[0]));     /* Bail out path from table       */
    }
  }

  /* the information we need is in the version file match */
  if ((vinst = a_inst->version))           /* Verify our version is there    */
  { if (vinst->ups_dir)                    /* Do we have a ups dir           */
    { if (UPSRELATIVE(vinst->ups_dir))     /* is ups_dir a relative path     */
      { prod_dir=upsget_prod_dir(a_db_info, a_inst, uc); 
        (void) strcat(g_buffer, prod_dir);        /* full path to product           */
        (void) strcat(g_buffer, "/");             /* add the /                      */
        (void) strcat(g_buffer, vinst->ups_dir);  /* tack on the ups_dir specified  */
      } else { 
        (void) strcat(g_buffer,vinst->ups_dir);   /* this is the whole path         */
      }
      (void) strcat(g_buffer, "/toman/");         /* Add the default location       */
    } else {                               /* no UPS dir, PROD_DIR/ups/toman */
      prod_dir=upsget_prod_dir(a_db_info, a_inst, uc); 
      (void) strcat(g_buffer, prod_dir);          /* full path to product           */
      (void) strcat(g_buffer, "/ups/toman/");     /* add the /ups/toman default     */
    }
  }
  
  return ((char *)(&g_buffer[0]));
}
/*-----------------------------------------------------------------------
 * upsget_man_source_dir
 *
 * Return a string containing the location of the products' man pages
 *
 * Input : an instance, and db configuration info
 * Output: none
 * Return: static string containing the location of the products' man pages
 */
char *upsget_catman_source_dir( const t_upstyp_matched_instance * const a_inst,
                             const t_upstyp_db * const a_db_info,
                             const t_upsugo_command * const uc)
{
  t_upstyp_instance *vinst;
  t_upstyp_instance *tinst;
  char *catman_source;                     /* catman_source_dir translated   */
  char *prod_dir;                          /* full path to product           */

  g_buffer[0] = '\0';                      /* we want to fill this anew      */
 
  if ((tinst = a_inst->table))             /* See if we have a table file    */
  { if (tinst->catman_source_dir)          /* Is the source in the table file*/
    { catman_source =                      /* will return value translated   */
         upsget_translation(a_inst, a_db_info,               /* if necessary */
                            0, tinst->catman_source_dir);
      if (UPSRELATIVE(catman_source))      /* Is it a relative path?         */
      { prod_dir=upsget_prod_dir(a_db_info, a_inst, uc);   
	(void) strcat(g_buffer, prod_dir);
	(void) strcat(g_buffer, "/");
      } 
      (void) strcat(g_buffer,catman_source);
      (void) strcat(g_buffer, "/");               /* safe then sorry...             */
      return ((char *)(&g_buffer[0]));     /* Bail out path from table       */
    }
  }

  /* the information we need is in the version file match */
  if ((vinst = a_inst->version))           /* Verify our version is there    */
  { if (vinst->ups_dir)                    /* Do we have a ups dir           */
    { if (UPSRELATIVE(vinst->ups_dir))     /* is ups_dir a relative path     */
      { prod_dir=upsget_prod_dir(a_db_info, a_inst, uc); 
        (void) strcat(g_buffer, prod_dir);        /* full path to product           */
        (void) strcat(g_buffer, "/");             /* add the /                      */
        (void) strcat(g_buffer, vinst->ups_dir);  /* tack on the ups_dir specified  */
      } else { 
        (void) strcat(g_buffer,vinst->ups_dir);   /* this is the whole path         */
      }
      (void) strcat(g_buffer, "/toman/");         /* Add the default location       */
    } else {                               /* no UPS dir, PROD_DIR/ups/toman */
      prod_dir=upsget_prod_dir(a_db_info, a_inst, uc); 
      (void) strcat(g_buffer, prod_dir);          /* full path to product           */
      (void) strcat(g_buffer, "/ups/toman/");     /* add the /ups/toman default     */
    }
  }
  
  return ((char *)(&g_buffer[0]));
}
char *upsget_info_source_dir( const t_upstyp_matched_instance * const a_inst,
                              const t_upstyp_db * const a_db_info,
                              const t_upsugo_command * const uc)
{
  t_upstyp_instance *vinst;
  t_upstyp_instance *tinst;
  char *info_source;                       /* info_source_dir translated     */
  char *prod_dir;                          /* full path to product           */

  g_buffer[0] = '\0';                      /* we want to fill this anew      */
 
  if ((tinst = a_inst->table))             /* See if we have a table file    */
  { if (tinst->info_source_dir)            /* Is the source in the table file*/
    { info_source =                        /* will return value translated   */
         upsget_translation(a_inst, a_db_info,               /* if necessary */
                            0, tinst->info_source_dir);
      if (UPSRELATIVE(info_source))        /* Is it a relative path?         */
      { prod_dir=upsget_prod_dir(a_db_info, a_inst, uc); 
	(void) strcat(g_buffer, prod_dir);
        (void) strcat(g_buffer, "/");
      } 
      (void) strcat(g_buffer,info_source);
      (void) strcat(g_buffer, "/");               /* safe then sorry...             */
      return ((char *)(&g_buffer[0]));     /* Bail out path from table       */
    }
  }

  /* the information we need is in the version file match */
  if ((vinst = a_inst->version))           /* Verify our version is there    */
  { if (vinst->ups_dir)                    /* Do we have a ups dir           */
    { if (UPSRELATIVE(vinst->ups_dir))     /* is ups_dir a relative path     */
      { prod_dir=upsget_prod_dir(a_db_info, a_inst, uc); 
        (void) strcat(g_buffer, prod_dir);        /* full path to product           */
        (void) strcat(g_buffer, "/");             /* add the /                      */
        (void) strcat(g_buffer, vinst->ups_dir);  /* tack on the ups_dir specified  */
      } else { 
        (void) strcat(g_buffer,vinst->ups_dir);   /* this is the whole path         */
      }
      (void) strcat(g_buffer, "/toinfo/");        /* Add the default location       */
    } else {                               /* no UPS dir, PROD_DIR/ups/toman */
      prod_dir=upsget_prod_dir(a_db_info, a_inst, uc); 
      (void) strcat(g_buffer, prod_dir);          /* full path to product           */
      (void) strcat(g_buffer, "/ups/toinfo/");    /* add the /ups/toinfo default    */
    }
  }
  
  return ((char *)(&g_buffer[0]));
}

/*-----------------------------------------------------------------------
 * upsget_table_file
 *
 * Given table file and the directories, find the absolute path
 * to the file.  This depends on the following hierarchy for places to look
 * for the existence of the table file  -
 *
 *    Look in each of the following successively till the file is found.  If
 *    the file is not found, it is an error.  If one of the pieces is missing,
 *    say - ups_dir - then that step is skipped.  NOTE: there is no default for
 *    the table file name. if the prod_dir_prefix is missing that step is
 *    still done with the prefix just left off.  if tablefiledir exists then
 *    only locations relative to it are checked.
 *
 *         tablefiledir/tablefile
 *         prod_dir_prefix/prod_dir/tablefiledir/tablefile
 *         db/prodname/tablefile
 *         ups_dir/tablefile
 *         prod_dir_prefix/prod_dir/ups_dir/tablefile
 *         tablefile
 * 
 *
 * Input : product name
 *         table file name
 *         table file directory
 *         ups directory information
 *         product dir
 *         ups database directory
 *         a flag stating if the directory returned must already contain the
 *             table file
 * Output: none
 * Return: table file location
 */
#define LOOK_FOR_FILE()   \
   if ((! a_exist_flag) || (upsutl_is_a_file(buffer)) == UPS_SUCCESS) {  \
     path_ptr = buffer;                                                  \
     found = 1;                                                          \
     upsver_mes(1, "%sFound table file in '%s'\n", g_UPSGET, buffer);    \
   } else {                                                              \
     upsver_mes(1, "%sCould not find table file in '%s'\n", g_UPSGET,    \
                buffer);                                                 \
   }

#define TRANSLATE_ENV(dst_var, source_var)  \
   dst_var = upsget_translation_env((char *)source_var);                    \
   dst_var = dst_var ? upsutl_str_create(dst_var, ' ') : (char *)source_var;

char *upsget_table_file( const char * const a_prodname,
                         const char * const a_tablefile,
                         const char * const a_tablefiledir,
                         const char * const a_upsdir,
                         const char * const a_productdir,
                         const t_upstyp_db * const a_db_info,
                         const int a_exist_flag)
{
  static char buffer[FILENAME_MAX+1];   /* max size of file name and path 
					   on system */
  char *path_ptr = NULL;
  int file_chars = 0, total_chars = 0;
  int found = 0;
  char *tmp_tfd = NULL, *tmp_ud = NULL, *tmp_pd = NULL;

  if (a_tablefile != NULL) {
    file_chars = (int )strlen(a_tablefile) + 2;  /* length plus trailing null 
						    and leading '/' */
    /* try tablefiledir/tablefile */
    if (a_tablefiledir != NULL) {
      /* first translate any environmental variables */
      TRANSLATE_ENV(tmp_tfd, a_tablefiledir);

      if ((total_chars = file_chars + (int )strlen(tmp_tfd))
	    <= FILENAME_MAX) {
	  (void) sprintf(buffer, "%s/%s", tmp_tfd, a_tablefile);
	  LOOK_FOR_FILE();
      } else {
	  upserr_vplace();
	  upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, total_chars);
      }
      /* try prod_dir/table_file_dir/tablefile */
      if ((found == 0) && (a_productdir != NULL) && UPSRELATIVE(tmp_tfd)) {
	/* first translate any environmental variables */
	TRANSLATE_ENV(tmp_pd, a_productdir);

	if (a_db_info && a_db_info->config &&
	    a_db_info->config->prod_dir_prefix && UPSRELATIVE(tmp_pd)) {
	  if ((total_chars += (int )strlen(tmp_pd) + 
	       (int )strlen(a_db_info->config->prod_dir_prefix) +
	       1) <= FILENAME_MAX) {
	    (void) sprintf(buffer, "%s/%s/%s/%s", a_db_info->config->prod_dir_prefix,
		    tmp_pd, tmp_tfd, a_tablefile);
	    LOOK_FOR_FILE();
	  } else {
	    upserr_vplace();
	    upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, total_chars);
	  }
	} else {
	  if ((total_chars += (int )strlen(tmp_pd) + 1)
	      <= FILENAME_MAX) {
	    (void) sprintf(buffer, "%s/%s/%s", tmp_pd, tmp_tfd, a_tablefile);
	    LOOK_FOR_FILE();
	  } else {
	    upserr_vplace();
	    upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, total_chars);
	  }
	}
      }
    } else { /* if (a_tablefiledir != NULL) */
      /* first translate any environmental variables */
      TRANSLATE_ENV(tmp_ud, a_upsdir);
      TRANSLATE_ENV(tmp_pd, a_productdir);

      /* try db/prod_name/tablefile */
      if (a_db_info && a_prodname) {
	if ((total_chars = file_chars + (int )(strlen(a_prodname) +
					       strlen(a_db_info->name)) + 1)
	    <= FILENAME_MAX) {
	  (void) sprintf(buffer, "%s/%s/%s", a_db_info->name, a_prodname,
		  a_tablefile);
	  LOOK_FOR_FILE();
	} else {
	  upserr_vplace();
	  upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, total_chars);
	}
      }
      /* try ups_dir/tablefile */
      if ((found == 0) && (tmp_ud != NULL) && (!UPSRELATIVE(tmp_ud))) {
	if ((total_chars = file_chars + (int )strlen(tmp_ud))
	    <= FILENAME_MAX) {
	  (void) sprintf(buffer, "%s/%s", tmp_ud, a_tablefile);
	  LOOK_FOR_FILE();
	} else {
	  upserr_vplace();
	  upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, total_chars);
	}
      }
      /* try prod_dir/ups_dir/tablefile */
      if ((found == 0) && (tmp_ud != NULL) && (tmp_pd != NULL) &&
	  UPSRELATIVE(tmp_ud)) {
	if (a_db_info && a_db_info->config &&
	    a_db_info->config->prod_dir_prefix && UPSRELATIVE(tmp_pd)) {
	  if ((total_chars = file_chars + (int )strlen(tmp_ud) + 
	       (int )strlen(tmp_pd) + 
	       (int )strlen(a_db_info->config->prod_dir_prefix) +
	       3) <= FILENAME_MAX) {
	    (void) sprintf(buffer, "%s/%s/%s/%s", a_db_info->config->prod_dir_prefix,
		    tmp_pd, tmp_ud, a_tablefile);
	    LOOK_FOR_FILE();
	  } else {
	    upserr_vplace();
	    upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, total_chars);
	  }
	} else {
	  if ((total_chars = file_chars + (int )strlen(tmp_ud) +
	       (int )strlen(tmp_pd) + 2) <= FILENAME_MAX) {
	    (void) sprintf(buffer, "%s/%s/%s", tmp_pd, tmp_ud, a_tablefile);
	    LOOK_FOR_FILE();
	  } else {
	    upserr_vplace();
	    upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, total_chars);
	  }
	}
      }
      /* try tablefile */
      if (found == 0) {
        if ((total_chars = file_chars) <= FILENAME_MAX) {
          (void) sprintf(buffer, "%s", a_tablefile);
          LOOK_FOR_FILE();
        } else {
          upserr_vplace();
          upserr_add(UPS_FILENAME_TOO_LONG, UPS_FATAL, total_chars);
        }
      }
    }
  }

  /* cleanup */
  if (tmp_pd && (tmp_pd != a_productdir)) {
    upsmem_free(tmp_pd);
  }
  if (tmp_ud && (tmp_ud != a_upsdir)) {
    upsmem_free(tmp_ud);
  }
  if (tmp_tfd && (tmp_tfd != a_tablefiledir)) {
    upsmem_free(tmp_tfd);
  }
return path_ptr;
}

static void shutup (const t_upstyp_db * const db_info_ptr,
                    const t_upstyp_matched_instance * const instance,
                    const t_upsugo_command * const command_line)
{
      bit_bucket ^= (long) db_info_ptr;
      bit_bucket ^= (long) instance;
      bit_bucket ^= (long) command_line;
}
