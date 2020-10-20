/************************************************************************
 *
 * FILE:
 *       upscpy.c
 * 
 * DESCRIPTION: 
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
 *       13-Apr-1998, DjF, first
 *
 ***********************************************************************/

/* standard include files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ups specific include files */
#include "ups.h"
/*
 * Definition of public variables.
 */

extern int g_DID_COPY_MAN;

/*
 * Private constants
 */

#define DQUOTE '"'
#define COMMA ','
#define OPEN_PAREN '('
#define CLOSE_PAREN ')'
#define SLASH "/"
#define WSPACE " \t\n\r\f"
#define EXIT "EXIT"
#define CONTINUE "CONTINUE"
#define UPS_ENV "UPS_ENV"
#define NO_UPS_ENV "NO_UPS_ENV"
#define DO_CHECK 1
#define NO_CHECK 0
#define NO_EXIT 0
#define DO_EXIT 1
#define DO_NO_UPS_ENV 1
#define DO_UPS_ENV 0
#define DATE_FLAG "DATE"
#define OLD_FLAG "OLD"
#define CATMANPAGES "catman"
#define MANPAGES "man"

/*
 * Private types
 */

/*
 * Declaration of private functions.
 */
static int not_yet_created(const int * const array, const int index,
                           const char element);
static char get_man_subdir(char * const a_man_file);

static void shutup(UPSCPY_PARAMS);

#define SHUTUP \
  if ((&bit_bucket + bit_bucket_offset == 0) && 0) shutup (a_minst, a_db_info, a_command_line, a_stream);

/* functions to handle specific action commands */

#define FPRINTF_ERROR() \
    upserr_vplace(); \
    upserr_add(UPS_SYSTEM_ERROR, UPS_FATAL, "fprintf", strerror(errno));

#define RESET_STATUS() \
    g_DID_COPY_MAN = 1;

#define SYSTEM_ERROR() \
    upserr_vplace(); \
    upserr_add(UPS_SYSTEM_ERROR, UPS_INFORMATIONAL, "system", strerror(errno));

/*
 * Definition of global variables.
 */

static char g_buff[MAX_LINE_LEN];
static long bit_bucket = 0;
static long bit_bucket_offset = 0;

/*=======================================================================
 *
 * Public functions
 *
 *=====================================================================*/

/* Action handling - the following routines are the ones that output shell
 *   specific code for each action supported by UPS
 */

void upscpy_html(UPSCPY_PARAMS)
{

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  upsver_mes(1, "%sThis action is not supported yet.\n", "UPSCPY");
  SHUTUP;
}

void upscpy_info(UPSCPY_PARAMS)
{
  struct stat file_stat;
  char *info_source;
  char buffer[ FILENAME_MAX+1 ];

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream) 
    /* Make sure we have somewhere to copy the files to and a source. */
  { if (a_db_info->config && a_db_info->config->info_target_dir)
    { info_source = upsget_info_source_dir(a_minst, a_db_info,a_command_line);
      if (! stat(info_source, &file_stat))
      { if (S_ISDIR(file_stat.st_mode))
        { if (stat(a_db_info->config->info_target_dir, &file_stat) 
              == (int )-1)  /* target not there make it */
          { upsver_mes(1, 
                  "%s: Creating target for info %s\n",
                  "UPSCPY", a_db_info->config->info_target_dir); 
            (void) sprintf(buffer, "umask 003;/bin/mkdir -p %s\n", 
                    a_db_info->config->info_target_dir);
            if (system(buffer))
            { SYSTEM_ERROR();
            }
          }
          upsver_mes(1,"%s: Copying info from %s\n","UPSCPY",info_source);
          (void) sprintf(buffer,"umask 003; cp %s/* %s\n", info_source,
                  a_db_info->config->info_target_dir);
          if (system(buffer))
          { SYSTEM_ERROR();
          }
	  /* it is possible that the copy did not succeed.  we do not want 
	     the system command to get a bad status return so put this in the
	     end of the file to reset status to success */
	  RESET_STATUS();
        }
      }
    }
    if (UPS_ERROR != UPS_SUCCESS)
    { upserr_vplace();
    }
  } else {
    upsver_mes(1, "%sNo destination in dbconfig file for info files\n",
                  "UPSCPY"); 
  }
}

#define COPY_FILES(filename, dest_path, type)  \
    /* figure out sectional sub directory (based on file extension) */ \
    if ((subdir = get_man_subdir(filename))) {                         \
      (void) sprintf(dest, "%s/%s%c", dest_path, type, subdir);        \
      /* see if the sectional subdir exists.  if not, create it. */    \
      if (stat(dest, &file_stat) == (int )-1) {                        \
        /* make sure we have not created it before */                  \
        if (not_yet_created(subdirs_made, subdirs_index, subdir)) {    \
          /* we must create the directory first */                     \
         upsver_mes(1,"%s: Creating target for %s %s\n","UPSCPY",type,dest);\
          (void) sprintf(buffer,"umask 003;/bin/mkdir -p %s\n", dest); \
          if (system(buffer)) { SYSTEM_ERROR(); }                      \
          /* now keep a record so we only create it once */            \
          subdirs_made[subdirs_index++] = (int )subdir;                \
        }                                                              \
      }                                                                \
      /* now add the copy line to the temp file */                     \
      (void) sprintf(buffer, "umask 003; cp %s %s\n", filename, dest); \
      if (system(buffer)) { SYSTEM_ERROR(); }                          \
    }

#define PROCESS_DIR(dir_name, dir_size, type, keyword)   \
    if (! strncmp(dir_line->d_name, dir_name, (unsigned int )dir_size))  \
    { DIR *file_dir = NULL;                                              \
      struct dirent *file_dir_line = NULL;                               \
      if ((dir_size == (int )strlen(dir_line->d_name)))                  \
      { /* the directory is = to dir_name */                             \
        if ((file_dir = opendir(filename))) {                            \
          upsver_mes(1,"%s: Copying %s files\n","UPSCPY",type);          \
          while ((file_dir_line = readdir(file_dir))) {                  \
            if (file_dir_line->d_name[0] != '.') {                       \
              (void) sprintf(g_buff, "%s/%s", filename, file_dir_line->d_name);\
              COPY_FILES(g_buff, a_db_info->config->keyword, type);      \
            }                                                            \
          }                                                              \
        }                                                                \
      } else {                                                           \
        /* the directory is XXX#, where # is the subdir spec.            \
           we just need to copy the entire contents of this dir          \
           (of the form *.*) to the appropriate destination */           \
        upsver_mes(1,"%s: Copying %s files\n","UPSCPY",type);            \
        (void) sprintf(buffer, "umask 003; cp %s/*.* %s/%s/ \n", filename,\
                a_db_info->config->keyword, dir_line->d_name);           \
        if (system(buffer)) { SYSTEM_ERROR(); }                          \
      }                                                                  \
    }

#define MAX_ARRAY_SIZE 100   /* can only be all alphanumeric characters */
#define FIRST_MAN_CHAR '.'
void upscpy_man(UPSCPY_PARAMS)
{
  char c = '0';
  struct dirent *dir_line = NULL;
  DIR *dir = NULL;
  char filename[MAX_LINE_LEN], *man_source;
  char dest[MAX_LINE_LEN], subdir;
  struct stat file_stat;
  int subdirs_made[MAX_ARRAY_SIZE];
  int subdirs_index = (int )0;
  int man_size = 3;           /* size of the string "man" */
  FILE *file = NULL;
  char buffer[ FILENAME_MAX+1 ];

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream)
    /* Make sure we have somewhere to copy the files to and a source. */
  { if (a_db_info->config && a_db_info->config->man_target_dir)
      /* ok we have a destination, now see if we have a source, if not
         assume the default */
    { /* GET_SOURCE(man_source, man_source_dir); */
      man_source = upsget_man_source_dir(a_minst, a_db_info,a_command_line);
        /* open the source directory and see what type of structure we have */
      if ((dir = opendir(man_source)))
          /* read each directory item and figure out what to do with it */
      { while ((dir_line = readdir(dir))) 
        { if (dir_line->d_name[0] != '.')    /* skip any . file */
          { (void) sprintf(filename, "%s/%s", man_source, dir_line->d_name);
            if (! stat(filename, &file_stat))
            { if (S_ISDIR(file_stat.st_mode))
                  /* this is a directory.  so we need to process the files in
                     it */
              { PROCESS_DIR("man", man_size, "man", man_target_dir);
              } else { /* this is a file, if it is a man file, copy  to the man
                     area */
                if ((file = fopen(filename, "r")))
                { upsver_mes(1,"%s: Copying man files\n","UPSCPY");
                  while((isspace((unsigned long )
                        (c=(unsigned char )fgetc(file))))) ; 
                  { if (c == FIRST_MAN_CHAR) 
                    { COPY_FILES(filename, 
                           a_db_info->config->man_target_dir, "man");
                    }
                    (void) fclose(file);
                  }
                }
              }
            }
          }   /* while ((dir_line ... */
	  /* it is possible that the copy did not succeed.  we do not want 
	     the system command to get a bad status return so put this in the
	     end of the file to reset status to success */
	  RESET_STATUS();
        }  /* if ((dir = ... */
      }
      if (UPS_ERROR != UPS_SUCCESS)
      { upserr_vplace();
      }
    } else {
      upsver_mes(1, "%sNo destination in dbconfig file for man files\n",
                 "UPSCPY"); 
    }
  }
}

void upscpy_catman(UPSCPY_PARAMS)
{
  char c = '0';
  struct dirent *dir_line = NULL;
  DIR *dir = NULL;
  char filename[MAX_LINE_LEN], *catman_source;
  char dest[MAX_LINE_LEN], subdir;
  struct stat file_stat;
  int subdirs_made[MAX_ARRAY_SIZE];
  int subdirs_index = (int )0;
  int cat_size = 6;           /* size of the string "catman" */
  FILE *file = NULL;
  char buffer[ FILENAME_MAX+1 ];

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream)
     /* Make sure we have somewhere to copy the files to and a source. */
  { if (a_db_info->config && a_db_info->config->catman_target_dir) 
       /* ok we have a destination, now see if we have a source, if not
          assume the default */
    { /* GET_SOURCE(catman_source, catman_source_dir); */
      catman_source = upsget_catman_source_dir(a_minst, a_db_info,a_command_line);
      /* open the source directory and see what type of structure we have */
      if ((dir = opendir(catman_source)))
         /* read each directory item and figure out what to do with it */
      { while ((dir_line = readdir(dir))) 
        { if (dir_line->d_name[0] != '.')    /* skip any . file */
          { (void) sprintf(filename, "%s/%s", catman_source, dir_line->d_name);
            if (! stat(filename, &file_stat)) 
            { if (S_ISDIR(file_stat.st_mode))
                /* this is a directory.  so we need to process the files in
                   it */
              { PROCESS_DIR("catman", cat_size, "cat", catman_target_dir);
              } else {
                /* this is a file, if it is a catman file, copy it to the man
                   area */
                if ((file = fopen(filename, "r")))
                { upsver_mes(1,"%s: Copying catman files\n","UPSCPY");
                  while(isspace((unsigned long )(c=(unsigned char )
                          fgetc(file)))) ;   /* skip empty spaces */
                  if (c != FIRST_MAN_CHAR) 
                  { COPY_FILES(filename,
                                      a_db_info->config->catman_target_dir, "cat");
                  }
                  (void) fclose(file);
                }
              }
            }
          }   /* while ((dir_line ... */
	  /* it is possible that the copy did not succeed.  we do not want 
	     the system command to get a bad status return so put this in the
	     end of the file to reset status to success */
	  RESET_STATUS();
        }  /* if ((dir = ... */
      }
      if (UPS_ERROR != UPS_SUCCESS)
      { upserr_vplace();
      }
    } else {
      upsver_mes(1, "%sNo destination in dbconfig file for catman files\n",
                 "UPSCPY"); 
    }
  }
}

#define REMOVE_FILES(filename, dest_path, type)  \
    /* figure out sectional sub directory (based on file extension) */ \
    if ((subdir = get_man_subdir(filename))) {                         \
      (void) sprintf(dest, "%s/%s%c", dest_path, type, subdir);        \
      /* see if the sectional subdir exists.  if not, ignore this. */  \
      if (stat(dest, &file_stat) != (int )-1) {                        \
        /* now add the remove line to the temp file */                 \
        (void) sprintf(buffer, "rm -f %s/%s\n", dest, filename);       \
        if (system(buffer)) { SYSTEM_ERROR(); }                        \
      }                                                                \
    }

#define UNPROCESS_DIR(dir_name, dir_size, type, keyword)   \
    if (! strncmp(dir_line->d_name, dir_name, (unsigned int )dir_size)) { \
      DIR *file_dir = NULL;                                            \
      struct dirent *file_dir_line = NULL;                             \
      if ((file_dir = opendir(filename))) {                            \
        while ((file_dir_line = readdir(file_dir))) {                  \
          if (file_dir_line->d_name[0] != '.') {                       \
            REMOVE_FILES(file_dir_line->d_name,                        \
                         a_db_info->config->keyword, type);            \
          }                                                            \
        }                                                              \
      }                                                                \
    }

void upscpy_rmman(UPSCPY_PARAMS)
{
  char c = '0';
  struct dirent *dir_line = NULL;
  DIR *dir = NULL;
  char filename[MAX_LINE_LEN], *man_source;
  char dest[MAX_LINE_LEN], subdir;
  struct stat file_stat;
  int man_size = 3;           /* size of the string "man" */
  FILE *file = NULL;
  char buffer[ FILENAME_MAX+1 ];

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream)
     /* Make sure we have somewhere to remove the files from. */
  { if (a_db_info->config && a_db_info->config->man_target_dir)
      /* ok we have a destination, now see if we have a source, if not
         assume the default */
    { /* GET_SOURCE(man_source, man_source_dir); */
      man_source = upsget_man_source_dir(a_minst, a_db_info,a_command_line);
      /* open the source directory and see what type of structure we have */
      if ((dir = opendir(man_source))) 
          /* read each directory item and figure out what to do with it */
      { while ((dir_line = readdir(dir)))
        { if (dir_line->d_name[0] != '.')    /* skip any . file */
          { (void) sprintf(filename, "%s/%s", man_source, dir_line->d_name);
            if (! stat(filename, &file_stat)) 
            { if (S_ISDIR(file_stat.st_mode))
                  /* this is a directory.  so we need to process the files in
                     it */
              { UNPROCESS_DIR("man", man_size, "man", man_target_dir);
              } else {
                /* this is a file, if it is a man file, remove it from the
                   man area */
                if ((file = fopen(filename, "r")))
                { while(isspace((unsigned long )(c=(unsigned char )
                        fgetc(file)))) ;   /* skip empty spaces */
                  if (c == FIRST_MAN_CHAR)
                  { REMOVE_FILES(dir_line->d_name, 
                                 a_db_info->config->man_target_dir, "man");
                  }
                  (void) fclose(file);
                }
              }
            }
          }   /* while ((dir_line ... */
	  /* it is possible that the copy did not succeed.  we do not want 
	     the system command to get a bad status return so put this in the
	     end of the file to reset status to success */
	  RESET_STATUS();
        }  /* if ((dir = ... */
      }
      if (UPS_ERROR != UPS_SUCCESS)
      { upserr_vplace();
      }
    } else {
      upsver_mes(1, "%sNo destination in dbconfig file for man files\n",
                 "UPSCPY"); 
    }
  }
}

void upscpy_rmcatman(UPSCPY_PARAMS)
{
  char c = '0';
  struct dirent *dir_line = NULL;
  DIR *dir = NULL;
  char filename[MAX_LINE_LEN], *catman_source;
  char dest[MAX_LINE_LEN], subdir;
  struct stat file_stat;
  int cat_size = 3;           /* size of the string "cat" */
  FILE *file = NULL;
  char buffer[ FILENAME_MAX+1 ];

  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  if ((UPS_ERROR == UPS_SUCCESS) && a_stream)
    /* Make sure we have somewhere to copy the files to. */
  { if (a_db_info->config && a_db_info->config->catman_target_dir)
      /* ok we have a destination, now see if we have a source, if not
         assume the default */
    { /* GET_SOURCE(catman_source, man_source_dir); */
      catman_source = upsget_catman_source_dir(a_minst, a_db_info,a_command_line);
      /* open the source directory and see what type of structure we have */
      if ((dir = opendir(catman_source))) 
        /* read each directory item and figure out what to do with it */
      { while ((dir_line = readdir(dir))) 
        { if (dir_line->d_name[0] != '.')    /* skip any . file */
          { (void) sprintf(filename, "%s/%s", catman_source, dir_line->d_name);
            if (! stat(filename, &file_stat)) 
            { if (S_ISDIR(file_stat.st_mode)) 
                /* this is a directory.  so we need to process the files in
                   it */
              { UNPROCESS_DIR("cat", cat_size, "cat", catman_target_dir);
              } else {
                /* this is a file, if it is a man file, remove it from the
                   man area */
                if ((file = fopen(filename, "r")))
                { while(isspace((unsigned long )(c=(unsigned char )
                        fgetc(file)))) ;   /* skip empty spaces */
                  if (c != FIRST_MAN_CHAR) 
                  { REMOVE_FILES(dir_line->d_name, 
                                 a_db_info->config->catman_target_dir,
                                 "cat");
                  }
                  (void) fclose(file);
                }
              }
            }
          }   /* while ((dir_line ... */
	  /* it is possible that the copy did not succeed.  we do not want 
	     the system command to get a bad status return so put this in the
	     end of the file to reset status to success */
	  RESET_STATUS();
        }  /* if ((dir = ... */
        upserr_vplace();
      }
      if (UPS_ERROR != UPS_SUCCESS)
      { upserr_vplace();
      }
    } else 
    { upsver_mes(1, "%sNo destination in dbconfig file for man files\n",
                 "UPSCPY"); 
    }
  }
}

void upscpy_news(UPSCPY_PARAMS)
{
  /* only proceed if we have a valid number of parameters and a stream to write
     them to */
  upsver_mes(1, "%sThis action is not supported yet.\n", "UPSCPY");
  SHUTUP;
}


/***  (copied from the original ups )
 * get_man_subdir() - Determine the sectional sub directory where
 *                    a product man page should reside in.
 *
 * DESCRIPTION:
 *   ups_uti_get_man_subdir() determines the sectional sub directory where a
 *   product man page should be copied to. This sub directory is determined
 *   from man_file, which is a pointer to the name of a product man file.
 *   It is in the format:
 *        <prod>.[0-9][a-zA-Z][.gz,.z,.Z]
 *   Examples:
 *      man_file        sectional sub directory
 *      --------        -----------------------
 *      foo.1                     1
 *      foo.3a                    3
 *      foo.l                     l (el)
 *      foo.8a.gz                 8
 *
 * INPUT  : man_file - name of the product man file
 * OUTPUT : none
 * RETURNS:
 *   the sectional sub directory letter which could be practically any char,
 *   or 0 if this file did not have a file extension
 ***/
#define NO_EXTENSION '\0'
static char get_man_subdir(char * const a_man_file)
{
   char *sp;
   char next_c;
   char ret_val = NO_EXTENSION;

   if ((sp = strrchr (a_man_file, '/')) == NULL) sp = a_man_file;  /* skip over directories */
   while ((next_c = *sp++) != '\0') {
     if (next_c == '.')  {
       ret_val = *sp++;
       /* Ignore obviously bogus file extensions. */
       if ((ret_val == '.') || (ret_val == 'g') || (ret_val == 'z') || (ret_val == 'Z'))
       {
         ret_val = NO_EXTENSION;
         continue;
       }
       /* check to see if this file extension is 1 or 2 characters
          possibly followed by a compression extension. */
       if ((ret_val == '\0') || ((next_c = *sp++) == '\0'))
         break;
       if ((next_c != '.') && (*sp == '\0'))
         break;
       if ((strcmp (sp, "z") == 0) ||
           (strcmp (sp, "Z") == 0) ||
           (strcmp (sp, "gz") == 0))
         break;
       ret_val = NO_EXTENSION;
     }
   }
   return(ret_val);
}

/*-----------------------------------------------------------------------
 * not_yet_created
 *
 * see if the passed character is already a member of the passed array
 *
 * Input : an integer array, an index indicating the maximum number of elements
 *         of the array that are valid and the character to look for
 * Output: none
 * Return: 1 if not present in the array, else 0
 */
static int not_yet_created(const int * const array, const int index, 
                           const char element)
{
  int i = 0;
  int ret = 1;  /* assume not present */

  for ( ; i < index ; ++i) {
    if (array[i] == (int )element) {
      /* found a match */
      ret = 0;
      break;
    }
  }
  return(ret);
}

static void shutup (UPSCPY_PARAMS)
{
  bit_bucket ^= (long) a_minst;
  bit_bucket ^= (long) a_db_info;
  bit_bucket ^= (long) a_command_line;
  bit_bucket ^= (long) a_stream;
}
