/************************************************************************
 *
 * FILE:
 *       ups_help.c
 * 
 * DESCRIPTION: 
 *       Will read help file and output help...
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
 *       Sep 17 1997, DjF, First.
 *       Jan 6  1998, EB, Get help file from ERUPT_DIR/doc
 *       Feb 19 1998, lr, Get help file from UPS_DIR/doc
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* UPS includes */
#include "upstyp.h"
#include "upsutl.h"
/*
 * Definition of public variables
 */

/*
 * Declaration of private functions
 */

/*
 * Definition of global variables
 */
/*
 * Definition of public functions
 */

/*-----------------------------------------------------------------------
 * upshlp_command
 *
 */

int upshlp_command(const char * const what) 
{
    FILE    *fh = 0;                  /* file handle */
    char    line[MAX_LINE_LEN] = "";  /* current line */
    int     oncommands=0;             /* on commands */
    int     onoptions=0;              /* on options */
    struct  upslst_item * command_options=0;
    struct  upslst_item * command_description=0;
    struct  upslst_item * option_list=0;
    struct  upslst_item * l_ptr=0;
    char    * addr=0;
    char    * data;
    char    * command=0;
    int     count=0;
    char    * option=0;
    char    * last ;
    int     found=0;			/* did I find the command? */
    char    * erupt_dir;
    char    buff[MAX_LINE_LEN];

/* test
    char    *what = "compile";
*/

  last =  (char *) malloc(13);

  erupt_dir = getenv("UPS_DIR");
  if (erupt_dir) {
     /* we got one */
     (void) sprintf(buff, "%s/doc/ups_help.dat", erupt_dir);
  } else {
     /* the environment variable could not be translated.  bail. */
     return 1;
  }

  fh = fopen ( buff, "r" );
  if (! fh) {
     /* could not open the file */
     return 1;
  }
  while ( fgets( line, MAX_LINE_LEN, fh ) ) {
    if ( line[0] != '#' ) {
       if ( !strncmp(line,"COMMAND",7)) {
          oncommands=1;
       } else { 
         if ( !strncmp(line,"OPTIONS",7)) {
            onoptions=1;
            oncommands=0;
         } else { 
           if (oncommands) {
              if ( line[0] != ':' ) {        /* new command */
                 line[12] = '\0';
                 command = upsutl_str_create(line,' ');
                 data = upsutl_str_create(line,' ');
                 if ((addr = strchr(&line[13],':'))!=0 ) *addr=0;
                 data = upsutl_str_crecat(command,&line[13]);
                 command_options=upslst_add(command_options,data);
              } else { 
                  data = upsutl_str_crecat(command,line);
                  command_description=upslst_add(command_description,data);
              }
           } else { 
             if (onoptions) {
                if ( line[0] == '-' ) {        /* new command */
                   line[2] = '\0';
                   option = upsutl_str_create(line,' ');
                   data = upsutl_str_crecat(option,&line[3]);
                   option_list=upslst_add(option_list,data);
                } else { 
                   data = upsutl_str_crecat(option,line);
                   option_list=upslst_add(option_list,data);
                }

             } else {
                (void) fprintf(stderr,"Unrecognized data in UPS help file\n");
                (void) fprintf(stderr,"%s\n",line);
                return 1;
             }
           }
         }
       }
    }
  }
  if ( !what || strlen( what ) <= 0 ) 
  { (void) fprintf(stderr,"UPS commands:\n\n");
    (void) fprintf(stderr,"for specific command options use \"ups COMMAND -?\"\n\n");
    for ( l_ptr = upslst_first(command_description); 
          l_ptr; l_ptr = l_ptr->next, count++ )
    { if (strncmp(last,l_ptr->data,12)) {
         (void) fprintf(stderr,"          %s",(char *)l_ptr->data ); 
      } else {
         (void) fprintf(stderr,"                       %s",(char *)l_ptr->data+13 ); 
      } (void) strncpy(last,l_ptr->data,12);
    }
  } else { 
    (void) fprintf(stderr,"UPS help command: %s\n\n",what);
    for ( l_ptr = upslst_first(command_description); 
          l_ptr; l_ptr = l_ptr->next, count++ )
    { if (!strncmp(l_ptr->data,what,strlen(what))) {
         (void) fprintf(stderr,"          %s",(char *)l_ptr->data+13 );
         found=1;
      }
    }
    if (found) {
       (void) fprintf(stderr,"\n     Valid Options:\n");
       for ( l_ptr = upslst_first(command_options); 
             l_ptr; l_ptr = l_ptr->next, count++ )
       { if (!strncmp(l_ptr->data,what,strlen(what))) {
/*            (void) fprintf(stderr,"          %s",(char *)l_ptr->data+13 ); */
            addr=l_ptr->data;
            option=addr+12;
         }
       }
       for ( l_ptr = upslst_first(option_list); 
             l_ptr; l_ptr = l_ptr->next, count++ )
       { addr=l_ptr->data;  
         *(addr+2)=' ';
           if (strchr(option,(int)*(addr+1))) {
              if (*(addr+1)==*last) { 
                 *addr=' ';
                 *(addr+1)=' ';
              } else { 
                *last=*(addr+1);
              }
              (void) fprintf(stderr,"          %s",(char *)l_ptr->data);
           }
       }
    } else {
      (void) fprintf(stderr,"No help information available\n");
    }
  }
/* test dump...
  upsugo_prtlst(command_options,"options");
  upsugo_prtlst(command_description,"description");
  upsugo_prtlst(option_list,"options");
*/
 return 0;
}
