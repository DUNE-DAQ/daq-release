/*****************************Copyright Notice ********************************
 *                                                                            *
 * Copyright (c) 1991 Universities Research Association, Inc.                 *
 *               All Rights Reserved.                                         *
 *                                                                            *
 ******************************************************************************/
/* ==========================================================================
**                                                                           
** DESCRIPTION
**
**	ups ugo (ups get opts) 
**           upsugo_getarg
**                                                                           
** DEVELOPERS                                                                
**
**       Eileen Berman
**       David Fagan
**       Lars Rasmussen
**                                                                           
**      Batavia, Il 60510, U.S.A.                                            
**                                                                           
** ENTRY POINTS                 SCOPE                                        
**      +++                     +++                                          
**                                                                           
** MODIFICATIONS                                                             
** 	Jul 24 1997, DjF, First
**
** HEADER STATEMENTS                                                         
*/                                                                           

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <sys/utsname.h>
#include <string.h>
#include <limits.h>				/* PATH_MAX */
#include <unistd.h>				/* getcwd & execvp */

extern char *get_current_dir_name(void);

#include "ups.h"
#include "upsuname.h"
#include "upsact.h"
extern t_cmd_info g_cmd_info[];

#ifdef UPS_ID
	char	UPS_UGO_ID[] = "@(#)upsugo.c	4.00";
#endif

int g_command_verb = 0;
int g_command_ugoB = 0;

int UPS_NEED_DB=1;

#define MAX_ARGS 1000
#define UPSUGO "UPSUGO: "
#define FREE( X ) free( X ); X = 0;

static void
expand_flavorlist(struct ups_command * uc) {
   int n;
   char *addr;
   t_upslst_item *new_flavor = 0;
   t_upslst_item *l_ptr = 0;
  
   if (uc->ugo_B) {
        /* for -B, do not expand the flavor list with sub-numbers, we only
         * want exact matches
         */
        return;
   }
   for ( l_ptr = upslst_first(uc->ugo_flavor); l_ptr; l_ptr = l_ptr->next) {
    addr=upsutl_str_create(l_ptr->data,' ');   
    upsver_mes(3,"%sExamining flavor %s from flavor list\n",UPSUGO,addr);
    upsver_mes(3,"%sAdding flavor %s to exp flavor list\n",UPSUGO,addr);
    new_flavor = upslst_add(new_flavor,addr);
    n = strlen(addr) - 1;
    if(addr[n] >= '0' && addr[n] <= '9') {
      int release;
      while(addr[n] >= '0' && addr[n] <= '9') {
	n--;
      }
      n++;
      release=atoi(addr+n);
      while( release > 0 ) {
	release--;
	addr=upsutl_str_create(addr,' ');
	sprintf(addr + n, "%d", release);
	upsver_mes(3,"%sAdding flavor %s to exp flavor list\n",UPSUGO,addr);
	new_flavor = upslst_add(new_flavor,addr);
      }
    }
  }
  upslst_free(uc->ugo_flavor,'d');
  uc->ugo_flavor = new_flavor;
}

static void
flavor_sub(struct ups_command * uc)
{  
   char *addr, *addr2;
   char *loc;
   int found1;
   char *flavor;
   t_upslst_item *l_ptr;
   t_upslst_item *copy_osname = 0;
   int k;

   for ( l_ptr = upslst_first(uc->ugo_osname); 
	       l_ptr; l_ptr = l_ptr->next) {
       flavor = l_ptr->data;
        addr=upsutl_str_create(flavor,' ');
        addr2=upsutl_str_create(flavor,' ');
        uc->ugo_flavor = upslst_add(uc->ugo_flavor,addr);
        copy_osname = upslst_add(copy_osname,addr2);
   }
   for (k = 0; k < 5; k++) {
      /* handle up to 5 layers of dashed flavor parts */
   for ( l_ptr = upslst_first(copy_osname); 
	       l_ptr; l_ptr = l_ptr->next) {
       flavor = l_ptr->data;
     if ((loc = strrchr(flavor,'-')))
      { *loc = 0;
        addr=upsutl_str_create(flavor,' ');
        upsver_mes(3,"%sAdding flavor %s to flavor list\n",UPSUGO,addr);
        uc->ugo_flavor = upslst_add(uc->ugo_flavor,addr);
      }
   }
   }
   found1 = 1;
   while (found1) {
   found1 = 0;
   for ( l_ptr = upslst_first(copy_osname); 
	       l_ptr; l_ptr = l_ptr->next) {
       flavor = l_ptr->data;
   if ((loc = strrchr(flavor,'.')))
      { *loc = 0;
        addr=upsutl_str_create(flavor,' ');
        upsver_mes(3,"%sAdding flavor %s to flavor list\n",UPSUGO,addr);
        uc->ugo_flavor = upslst_add(uc->ugo_flavor,addr);
        found1 = 1;
      }
   }
   }
   for ( l_ptr = upslst_first(copy_osname); 
	       l_ptr; l_ptr = l_ptr->next) { 
       flavor = l_ptr->data;
   if ((loc = strrchr(flavor,'+')))
      { *loc = 0;
        addr=upsutl_str_create(flavor,' ');
        upsver_mes(3,"%sAdding flavor %s to flavor list\n",UPSUGO,addr);
        uc->ugo_flavor = upslst_add(uc->ugo_flavor,addr);
      }
   }
   addr=upsutl_str_create("NULL",' ');
   upsver_mes(3,"%sAdding flavor %s to flavor list\n",UPSUGO,addr);
   uc->ugo_flavor = upslst_add(uc->ugo_flavor,addr);
   upslst_free(copy_osname,'d');
}

/* Single flag set cases */
#define case_help case '?': uc->ugo_help = 1; break;
#define case_a case 'a': uc->ugo_a = 1; break;
#define case_B case 'B': uc->ugo_B = 1; break;
#define case_C case 'C': uc->ugo_C = 1; break;
#define case_e case 'e': uc->ugo_e = 1; break;
#define case_E case 'E': uc->ugo_E = 1; break;
#define case_F case 'F': uc->ugo_F = 1; break;
#define case_j case 'j': uc->ugo_j = 1; break;
#define case_k case 'k': uc->ugo_k = 1; break;
#define case_l case 'l': uc->ugo_l = 1; break;
#define case_L case 'L': uc->ugo_L = 1; break;
#define case_P case 'P': uc->ugo_P = 1; break;
#define case_R case 'R': uc->ugo_R = 1; break;
#define case_s case 's': uc->ugo_s = 1; break;
#define case_S case 'S': uc->ugo_S = 1; break;
/* Set verbose IMMEDIATELY rest of line parsed will be verbose */
#define case_v case 'v': uc->ugo_v +=1; UPS_VERBOSE=uc->ugo_v; break;
#define case_V case 'V': uc->ugo_V = 1; break;
#define case_w case 'w': uc->ugo_w = 1; break;
#define case_W case 'W': uc->ugo_W = 1; break;
/* #define case_x case 'x': uc->ugo_x = 1; break; */ /* Likely never to be implemented */
#define case_X case 'X': uc->ugo_X = 1; break;
#define case_y case 'y': uc->ugo_y = 1; break;
#define case_Y case 'Y': uc->ugo_Y = 1; break;
#define case_Z case 'Z': uc->ugo_Z = 1; break;
/* 0-n most generic to most specific match */
#define case_0 case '0': uc->ugo_number = 1; break;
#define case_1 case '1': uc->ugo_number = 2; break;
#define case_2 case '2': uc->ugo_number = 3; break;
#define case_3 case '3': uc->ugo_number = 4; break;
#define case_4 case '4': uc->ugo_number = 5; break;
#define case_5 case '5': uc->ugo_number = 6; break;
#define case_6 case '6': uc->ugo_number = 7; break;
#define case_7 case '7': uc->ugo_number = 8; break;

/* Add a specified chain to the list */
#define add_chain(CHAIN) \
         addr=upsutl_str_create(CHAIN,' ');              \
         uc->ugo_chain = upslst_add(uc->ugo_chain,addr); 
/* All "defined" chains given by a flag */
#define case_c case 'c': uc->ugo_c = 1; add_chain("current");     uc->ugo_reqchain = "-c"; break;
#define case_d case 'd': uc->ugo_d = 1; add_chain("development"); uc->ugo_reqchain = "-d"; break;
#define case_n case 'n': uc->ugo_n = 1; add_chain("new");         uc->ugo_reqchain = "-n"; break;
#define case_t case 't': uc->ugo_t = 1; add_chain("test");        uc->ugo_reqchain = "-t"; break;
#define case_o case 'o': uc->ugo_o = 1; add_chain("old");         uc->ugo_reqchain = "-o"; break;

/* This routine set the element to the next arguement but it has to
   determine if the value is part of the same argument (argv) or a
   seperate one.  i.e. -m X.table or -mX.table or -m"X.table" or ...
   It also does a simple validation to make sure the arguement is not 
   - (another flag) or a , (the next command)
   
*/
#define set_value( ELEMENT , ARG )                                  \
{ if ( *argbuf )                                                    \
  { addr=upsutl_str_create(*argbuf,'p');                            \
    ELEMENT = addr;                                                 \
    *argbuf = 0;                                                    \
    break;                                                          \
  }                                                                 \
  if((arg_str = upsugo_getarg(ups_argc,ups_argv, argbuf)) != 0)     \
  { if(*arg_str == '-' || *arg_str == ',' )                         \
    { upserr_add(UPS_NOVALUE_ARGUMENT, UPS_FATAL, arg_str, ARG );   \
      break;                                                        \
    }                                                               \
    addr=upsutl_str_create(arg_str,'p');                            \
    ELEMENT = addr;                                                 \
    break;                                                          \
  }                                                                 \
  errflg = 1;                                                       \
  break;                                                            \
} 
/* Same as above used only for description DOES NOT PACK           */
#define set_value_nop( ELEMENT , ARG )                              \
{ if ( *argbuf )                                                    \
  { addr=upsutl_str_create(*argbuf,' ');                            \
    ELEMENT = addr;                                                 \
    *argbuf = 0;                                                    \
    break;                                                          \
  }                                                                 \
  if((arg_str = upsugo_getarg(ups_argc,ups_argv, argbuf)) != 0)     \
  { if(*arg_str == '-' || *arg_str == ',' )                         \
    { upserr_add(UPS_NOVALUE_ARGUMENT, UPS_FATAL, arg_str, ARG );   \
      break;                                                        \
    }                                                               \
    addr=upsutl_str_create(arg_str,' ');                            \
    ELEMENT = addr;                                                 \
    break;                                                          \
  }                                                                 \
  errflg = 1;                                                       \
  break;                                                            \
} 

/* -O is a totally special arguement 
   It has to take the value and not care if it looks like an option
   because it is an option but not on this command line
*/
#define set_O_value( ELEMENT , ARG )     \
{ if ( *argbuf )                         \
  { addr=upsutl_str_create(*argbuf,' '); \
    ELEMENT = addr;                      \
    *argbuf = 0;                         \
    break;                               \
  }                                      \
  if((arg_str = ups_argv[++argindx]))    \
  { addr=upsutl_str_create(arg_str,' '); \
    ELEMENT = addr;                      \
    break;                               \
  }                                      \
  errflg = 1;                            \
  break;                                 \
}
/* This builds the list of optionally : seperated arguements on the
   command line.  The examples of how this can be specified appears
   nearly endless.  It handles the flag with or with spaces and 
   quotes on the whole arguement or even as a piece of the arguement.
   It should also be noted that individual parts (seperated by :) 
   are also packed into a single value.  
   i.e. -fOS1:" MYOS":"ANY OS":OSEND 
   is the same a -f "OS1:MYOS:ANYOS:OSEND" of course with or wihout "'s
   NOTE: build list includes the BREAK
*/
static char reqchainbuf[512];
#define build_list( LIST_ELEMENT , ARG )                          \
{ if ( *argbuf )                                                  \
  { if( &LIST_ELEMENT == &uc->ugo_chain) 			  \
    {								  \
 	sprintf(reqchainbuf,"-g %s", *argbuf );			  \
	uc->ugo_reqchain = strdup(reqchainbuf);			  \
    }								  \
    while((loc=strchr(*argbuf,':'))!=0)                           \
    { addr=*argbuf;                                               \
      *argbuf=loc+1;                                              \
      *loc = 0;                                                   \
      addr=upsutl_str_create(addr,'p');                           \
      LIST_ELEMENT = upslst_add(LIST_ELEMENT,addr);               \
    }                                                             \
    addr=upsutl_str_create(*argbuf,'p');                          \
    LIST_ELEMENT = upslst_add(LIST_ELEMENT,addr);                 \
    *argbuf = 0;                                                  \
    break;                                                        \
  }                                                               \
  if((arg_str = upsugo_getarg(ups_argc,ups_argv,argbuf)) != 0)    \
  { if(*arg_str == '-' || *arg_str == ',' )                       \
    { upserr_add(UPS_NOVALUE_ARGUMENT, UPS_FATAL, arg_str, ARG ); \
      break;                                                      \
    }                                                             \
    if( &LIST_ELEMENT == &uc->ugo_chain) 			  \
    {								  \
 	sprintf(reqchainbuf,"-g %s", arg_str );			  \
	uc->ugo_reqchain = strdup(reqchainbuf);			  \
    }								  \
    while((loc=strchr(arg_str,':'))!=0)                           \
    { addr=arg_str;                                               \
      arg_str=loc+1;                                              \
      *loc = 0;                                                   \
      addr=upsutl_str_create(addr,'p');                           \
      LIST_ELEMENT = upslst_add(LIST_ELEMENT,addr);               \
    }                                                             \
    addr=upsutl_str_create(arg_str,'p');                          \
    LIST_ELEMENT = upslst_add(LIST_ELEMENT,addr);                 \
    break;                                                        \
  }                                                               \
  errflg = 1;                                                     \
  break;                                                          \
}
/* list element options */
#define case_A case 'A': uc->ugo_A = 1; build_list (uc->ugo_auth , "A") 
#define case_f case 'f': uc->ugo_f = 1; build_list (uc->ugo_flavor , "f") 
#define case_g case 'g': uc->ugo_g = 1; build_list (uc->ugo_chain , "g")
#define case_H case 'H': uc->ugo_H = 1; build_list (uc->ugo_osname , "H") 
#define case_h case 'h': uc->ugo_h = 1; build_list (uc->ugo_host , "h") 
#define case_K case 'K': uc->ugo_K = 1; build_list (uc->ugo_key , "K") 

/* single value options */
#define case_b case 'b': uc->ugo_b = 1; set_value (uc->ugo_compile_file, "b") 
#define case_D case 'D': uc->ugo_D = 1; set_value (uc->ugo_origin , "D") 
#define case_m case 'm': uc->ugo_m = 1; set_value (uc->ugo_tablefile , "m") 
#define case_M case 'M': uc->ugo_M = 1; set_value (uc->ugo_tablefiledir, "M")
#define case_N case 'N': uc->ugo_N = 1; set_value (uc->ugo_anyfile , "N")    
#define case_r case 'r': uc->ugo_r = 1; set_value (uc->ugo_productdir , "r")
#define case_T case 'T': uc->ugo_T = 1; set_value (uc->ugo_archivefile , "T")
#define case_u case 'u': uc->ugo_u = 1; set_value (uc->ugo_compile_dir, "u")
#define case_U case 'U': uc->ugo_U = 1; set_value (uc->ugo_upsdir , "U")

#define case_dot case '.': \
         {static char cwdbuf[PATH_MAX]; \
         uc->ugo_dot = 1; \
         uc->ugo_r = 1; uc->ugo_M = 1; uc->ugo_m = 1; \
         uc->ugo_productdir = strcpy(upsmem_malloc(strlen(getcwd(cwdbuf,PATH_MAX))),cwdbuf) ; \
         uc->ugo_tablefiledir = strcpy(upsmem_malloc(4),"ups"); \
         uc->ugo_tablefile = uc->ugo_product ? strcat(strcpy(upsmem_malloc(strlen(uc->ugo_product)+7),uc->ugo_product),".table") : 0;}

/* Special case -O value (no check) */
#define case_O case 'O': uc->ugo_O = 1; \
        set_O_value (uc->ugo_options , "O")

/* Request for G now also just like O */
#define case_G case 'G': uc->ugo_G = 1; \
        set_O_value (uc->ugo_passed , "G")

/* Special case -p value (no pack) */
#define case_p case 'p': uc->ugo_p = 1; \
        set_value_nop (uc->ugo_description , "p")

#define case_q case 'q': uc->ugo_q = 1; \
   if ( *argbuf )                                                  \
   { (void) upsugo_bldqual(uc,*argbuf);                            \
     *argbuf=0;                                                    \
     break;                                                        \
   }                                                               \
   if((arg_str = upsugo_getarg(ups_argc,ups_argv,argbuf)) != 0)    \
   { if(*arg_str == '-')                                           \
     { upserr_add(UPS_NOVALUE_ARGUMENT, UPS_FATAL, arg_str, "q" ); \
       break;                                                      \
     }                                                             \
     (void) upsugo_bldqual(uc,arg_str);                            \
     break;                                                        \
   }                                                               \
   errflg = 1;                                                     \
   break;
#define case_z case 'z': uc->ugo_z = 1; \
   if ( *argbuf )                                                  \
   { (void) upsugo_blddb(uc,*argbuf);                              \
     *argbuf=0;                                                    \
     break;                                                        \
   }                                                               \
   if((arg_str = upsugo_getarg(ups_argc,ups_argv,argbuf)) != 0)    \
   { if(*arg_str == '-')                                           \
     { upserr_add(UPS_NOVALUE_ARGUMENT, UPS_FATAL, arg_str, "z" ); \
       break;                                                      \
     }                                                             \
     (void) upsugo_blddb(uc,arg_str);                              \
     break;                                                        \
   }                                                               \
   errflg = 1;                                                     \
   break;

   int	errflg = 0;
   t_upslst_item *ugo_commands = 0;
   t_upslst_item *last_command = 0;
   int argindx;
   int g_UPS_SHELL=e_INVALID_SHELL;

/* ===========================================================================
 * Private function declarations
 */
char * upsugo_getarg( const int , char **,char ** const );
int upsugo_rearg(const int ,char **,int * const,char **);
int upsugo_ifornota(struct ups_command * const uc);
void upsugo_setshell(struct ups_command * const uc);
int upsugo_bldqual(struct ups_command * const uc, char * const inaddr);
int upsugo_blddb(struct ups_command * const uc, char * inaddr);
void upsugo_prtdb(t_upslst_item * const list_ptr,
                  char * const title,const unsigned int);
void upsugo_liststart(struct ups_command * const a_command_line);


/* ===========================================================================
** ROUTINE	upsugo_bldfvr()
**
*/
int upsugo_bldfvr(struct ups_command * const uc)
{
   char   		*addr;
   char			uname_flavor[80];	/* put together from uname */
   char			*flavor;		/* flavor ptr */
   t_upslst_item        *l_ptr;
   int count = 0;
   int hcount = 0;
   flavor = uname_flavor;			/* init pointer */
/* get the flavor info from the os basically adding release name to sysname,
   but some OS's are funny
   ------------------------------------------------------------------------*/

if (!uc->ugo_H) {
    /* these declarations are all to make the build_list macro work... */
    char *argbuf[2];  
    char *loc;
    int ups_argc = 0;
    char **ups_argv = 0;
    char *arg_str;

    upsver_mes(3,"%sChecking for UPS_OVERRIDE with -H\n", UPSUGO); 

    /* see if we have a UPS_OVERRIDE for -H... */
    argbuf[0] = ups_get_default_host();
    loc = argbuf[0];
    if (argbuf[0]) {
        upsver_mes(3,"%s found UPS_OVERRIDE -H %s\n", UPSUGO, argbuf[0]); 

 	/* act like we saw it on the command line... */
	switch(1) {
        default:
	uc->ugo_H = 1;
        build_list (uc->ugo_osname , "H")   
        }
    }
}

if (!uc->ugo_H)
 {
   flavor[0] = 0;
   ups_append_OS(flavor);
   if (ups_64bit_check()) {
       ups_append_MACHINE(flavor);
   }
   (void)strcat(flavor, "+");
   ups_append_release(flavor);
   addr=upsutl_str_create(flavor,' ');	
   uc->ugo_osname = upslst_new(addr);
   if (ups_64bit_check()) 
   { flavor[0] = 0;
     ups_append_OS(flavor);
     (void)strcat(flavor, "+");
     ups_append_release(flavor);
     addr=upsutl_str_create(flavor,' ');		/* first add full */
     uc->ugo_osname = upslst_add(uc->ugo_osname, addr);
   } else 
   { upsver_mes(3,"%snot 64bit...\n",UPSUGO);
   }
 } 
 flavor_sub(uc);
/*   upslst_free(uc->ugo_osname,'d');    need -f and -H
   uc->ugo_osname=0; */
 if (uc->ugo_number)                       /* specified a specific os level  */
 { 
   hcount = upslst_count(uc->ugo_osname);
   /* printf("XXX hcount is %d\n", hcount); */
   uc->ugo_number = (uc->ugo_number -1) * hcount + 1;  /* flavors from -H foo:bar are interleaved */
   count = upslst_count(uc->ugo_flavor);/* if they specified a level more */
   if(uc->ugo_number > upslst_count(uc->ugo_flavor))   /* specific then      */
   { uc->ugo_number=upslst_count(uc->ugo_flavor); } /* available set to max  */
   l_ptr = upslst_first(uc->ugo_flavor);   /* Start at begining of list      */
   while ( l_ptr  )                        /* loop threw tossing unwanted    */
   { if( uc->ugo_number != count )         /* levels of the flavor           */
     { upsver_mes(3,"%sNumber specified deleting %s from flavor list\n",
                  UPSUGO,l_ptr->data); 
       l_ptr = upslst_delete_safe( l_ptr, l_ptr->data, 'd' ); 
     } else {                              /* this is the level we want      */
       uc->ugo_flavor=l_ptr;               /* just pass it up and leave it   */
       l_ptr=l_ptr->next;                  /* there                          */
     } count--;
   } 
 } else  
 { expand_flavorlist(uc);
 }
 return(0);
}
/* ===========================================================================
** ROUTINE	upsugo_setshell()
**
** Set the shell type of the user in the command line 
*/
void upsugo_setshell(struct ups_command * const uc)
{
   char   * SHELL;                         /* SHELL value hopefully UPS_SHELL*/
   if (g_UPS_SHELL==e_INVALID_SHELL)       /* If it has not allready been set*/
   { if((SHELL = (char *)getenv("UPS_SHELL")) == 0) /* check for UPS_SHELL   */
     { if((SHELL = (char *)getenv("SHELL")) == 0)   /* failed check SHELL    */
       { g_UPS_SHELL=e_BOURNE;             /* no SHELL either                */
         (void) strcpy(SHELL,"sh");        /* go bourne set to sh for match  */
       } else {                            /* got SHELL value                */
         if (strstr(SHELL,"csh"))          /* is it csh?            UK!      */
         { g_UPS_SHELL=e_CSHELL;
         } else { 
           g_UPS_SHELL=e_BOURNE;
         }
       }                                 /* let them know that you impromised*/
       upserr_add(UPS_NOSHELL, UPS_INFORMATIONAL, SHELL); /* the shell value */
     } else {                            /* UPS_SHELL was set set accordingly*/
       if (strstr(SHELL,"csh"))
       { g_UPS_SHELL=e_CSHELL;
       } else { 
         g_UPS_SHELL=e_BOURNE;
       }
     }
   } uc->ugo_shell=g_UPS_SHELL;           /* Store the shell value in command*/
}

/* ===========================================================================
** ROUTINE	upsugo_ifornota() ( if on not a ) specified 
**
** This routine is called at the end of the command sequence to fill in
** any defaults if not otherwise specified and sets the values appropriately
** if a -a (all) is specified
**
** I could use the same address for the "*" string but I don't think the
** extra code would justify it.
**
*/
int upsugo_ifornota(struct ups_command * const uc)
{
   char   * addr = 0;
   char   * PRODUCTS;                           /* PRODUCTS value */
   char   * loc;
   static char temp[1024];

   if (!uc->ugo_q && ups_get_default_quals()) {
     /* this looks like it doesn't do anything, but it sets our default
	  qualifiers, since they get prepended to the empty string... */
     uc->ugo_q = 1;
     upsugo_bldqual(uc, "");
   }
   if (!uc->ugo_B && ups_get_default_B()) {
     uc->ugo_B = 1;
   }
   if (!uc->ugo_product)                        /* no product                */
   { addr=upsutl_str_create("*",' ');           /* wildcard product name     */
     upsver_mes(3,"%sNo product specified set to %s\n",UPSUGO,addr); 
     uc->ugo_product = addr;                   /* put in command as specified*/
     if ( (g_cmd_info[g_command_verb].valid_opts &&  g_cmd_info[g_command_verb].valid_opts[1] != 'a' ) && !uc->ugo_help && g_command_verb != e_help && g_command_verb != e_flavor ) 
     { upserr_add(UPS_INVALID_ARGUMENT, UPS_FATAL, "\" <-- no product name given, cannot find \"");  
       printf( "g_command_verb is %s: %d/%d \n", g_cmd_info[g_command_verb].cmd, g_command_verb, e_flavor);
     }
   }
   if (uc->ugo_a)                               /* did the user specify -a */
   { if (!uc->ugo_chain && !uc->ugo_version)    /* If no chain all chains  */
     { upsver_mes(3,"%sNo chain specified set to *\n",UPSUGO); 
       add_chain("*")                           /* no chain -> OR <- version */
     }
     if (!uc->ugo_qualifiers)                   /* no qualifiers */
     { addr=upsutl_str_create("*",' ');         /* wildcard them too */
       upsver_mes(3,"%sNo qualifiers specified set to %s\n",UPSUGO,addr); 
       uc->ugo_qualifiers = upslst_add(uc->ugo_qualifiers,addr);
     }
     if (!uc->ugo_version)          /* unallocated if later specified */
     { addr=upsutl_str_create("*",' ');         /* no version wildcard */
       upsver_mes(3,"%sNo version specified set to %s\n",UPSUGO,addr); 
       uc->ugo_version = addr;      /* at this point I may not know... */
     }
/* the ugo_number is an after the fact processing and the -H is kept
   in the os_name until after so they must be dealt with specifically  */
     if (!uc->ugo_flavor || uc->ugo_number )  /* no flavor or a number */
     { if(!uc->ugo_number )                   /* no number wildcare flavor */
       { addr=upsutl_str_create("*",' ');  
         upsver_mes(3,"%sNo flavor specified set to %s\n",UPSUGO,addr); 
         uc->ugo_flavor = upslst_add(uc->ugo_flavor,addr);
       } else {
         (void) upsugo_bldfvr(uc);            /* build the appropriate os */
         addr=upsutl_str_create(ANY_FLAVOR,' ');           /* flavor and add */
         upsver_mes(3,"%sAdding flavor %s\n",UPSUGO,addr); /* to the list */
         uc->ugo_flavor = upslst_add(uc->ugo_flavor,addr);
       }
     }
   } else {                         /* not -a but give defaults */
     if (!uc->ugo_chain && !uc->ugo_version) /* no chain or version no -a */
     { upsver_mes(3,"%sNo (-a) Adding chain current\n",UPSUGO,addr); 
       add_chain("current");                 /* give them current chain */
     }
     if (!uc->ugo_qualifiers)                /* no qualifiers = ""       */
     { addr=upsutl_str_create("",' ');
       upsver_mes(3,"%sNo (-a) Adding qualifiers \"\"\n",UPSUGO,addr); 
       uc->ugo_qualifiers = upslst_add(uc->ugo_qualifiers,addr);
     }
     if (!uc->ugo_flavor)                    /* no flavor? */
     { (void) upsugo_bldfvr(uc);             /* build it too! */
     }
     addr=upsutl_str_create(ANY_FLAVOR,' ');   /* if not -a EVERYBODY gets */
     upsver_mes(3,"%sAdding flavor %s\n",UPSUGO,addr);  /* * flavor -a IS  */
     uc->ugo_flavor = upslst_add(uc->ugo_flavor,addr);
   }
/* If they didn't specify a database pull the environment variable */
   if (!uc->ugo_db) {                        /* do I have any database? */
     if((PRODUCTS = (char *)getenv("PRODUCTS")) == 0) /* nope */
     { if (!uc->ugo_m)                     /* no table file either, I'm dead */
       { if(UPS_NEED_DB)                   /* ups flavor & create don't need */
         { upserr_add(UPS_NO_DATABASE, UPS_FATAL);  /* a database */
         }
       }
     } else {
        /* addr = (char *) malloc((size_t)(strlen(PRODUCTS) +1)); */
        addr=temp; /* use static instead of alloc space */
        (void) strcpy(addr,PRODUCTS);
        loc=addr;
        while ( loc && *loc )  /* covert any space ( tab etc ) to a space */
        { if (isspace( (unsigned long )(*(loc)) ) ) { *loc=' '; }
          ++loc; 
        }
        while ((loc=strchr(addr,' '))!=0) /* more then one database ? */
        { *loc = 0;
          PRODUCTS = upsutl_str_create(addr,' '); /* name is up to space */
          (void) upsugo_blddb(uc,PRODUCTS);       /* add it to the list */
          upsmem_free(PRODUCTS);                  /* free the old new space */
          addr=loc+1;                             /* was allocated */
          while(*addr==' ') { addr=addr+1; }
        }
        PRODUCTS = upsutl_str_create(addr,' ');   /* no more spaces add what */
        (void) upsugo_blddb(uc,PRODUCTS);         /* is left */
        upsmem_free(PRODUCTS);                    /* free the temp space */
     }
   } 
   return(0);
}
/* ===========================================================================
** ROUTINE	upsugo_blddb()
**
** The code to handle the NT environment is a bit of a hack, I convert
** any :'s followed by a \ (drive specification) to a | (pipe) which
** couldn't possibly get in there any other way and then modify them
** back to : after the normal split.
*/
int upsugo_blddb(struct ups_command * const uc, char * inaddr)
{
 char * loc;
 char * db;
 char * saddr=inaddr;
 int nt=0;
 struct upstyp_db * addr;
/* All NT database paths must have \ following : if not a seperator */
/* PRODUCTS or -z "C:\/local/products:GROUP:\/usr/products"         */
/* They say to define it the "unix" way but since this was added    */
/* cause someone didn't and it broke and it works I'm leaving it    */
/* convert PC specifications of PRODUCTS with drive:\paths to drive|\ */
/* so they don't split the : as a seperate database, later I convert  */
/* them back if this was done on the nt flag                        */
 while((loc=strchr(inaddr,':'))!=0)    /* all drive:\ to drive|\    */
 { inaddr=loc+1;                       /* | was the otherwise unusable */
   if(*inaddr=='\\') 
   { *loc = '|'; 
     nt++;			/* Do not endure pain if not necessary */
   }
 }
 inaddr=saddr;
 while((loc=strchr(inaddr,':'))!=0)  /* loop threw the : seperated list */
 { db=inaddr;
   inaddr=loc+1;
   *loc = 0;
   if(nt)                       /* if it was a nt specification now fix it */
   { loc=strchr(db,'|');
     if (loc) 
     { *loc=':';                /* back to the original : */
     }
   }
   db=upsutl_str_create(db,'p'); /* add the database entry */
   addr=(struct upstyp_db *)upsmem_malloc( sizeof(struct upstyp_db));
   (void) memset (addr,0,sizeof(struct upstyp_db));
   addr->name = db;
   upsver_mes(3,"%sAdding database %s\n",UPSUGO,addr->name); 
   uc->ugo_db = upslst_add(uc->ugo_db,addr);
 }
 if(nt)                         /* last or only still need to check and */
 { loc=strchr(inaddr,'|');      /* fix the nt | to : */
   if (loc) { *loc=':'; }
 }
 /* db may not be free because it's pointed to by db config do NOT free */
 db=upsutl_str_create(inaddr,'p');
 addr=(struct upstyp_db *)upsmem_malloc( sizeof(struct upstyp_db));
 (void) memset (addr, 0, sizeof(struct upstyp_db));
 addr->name = db;                /* add last or only to the database list */
 upsver_mes(3,"%sAdding database %s\n",UPSUGO,addr->name); 
 uc->ugo_db = upslst_add(uc->ugo_db,addr);
 *inaddr = 0;
 return(0);
}
/* ===========================================================================
** ROUTINE	upsugo_old2newquals()
**
** We've changed the definition of optional and mandatory qualifiers
** The old style was ?optional:mandatory:mandatory
** the new is optional:+mandatory:+mandatory
** rather than fix the old code below, we simply convert the syntax
**
*/
static char *
ugo_old2newquals( const char *inaddr, int declflag ) {
    static char buf[1024]; /* OH NO!! bogus limit on qualifier length... */
    char *p2 = buf;
    const char *p1 = inaddr;

    if (*inaddr == 0) {
       return "";
    }

    if (declflag) {
	/* just remove + signs */
	while( 0 != (*p2 = *p1 )) {
            if (*p1 != '+')
		p2++;
	    p1++;
        }
        return buf;
    }
    /* if first qual is mandatory, skip the plus, else put in the ? for it */
    if (*p1 == '"') {
        *p2++ = *p1++;
         if (*p1 == '"') {
             *p2++ = *p1++;
             *p2 = 0;
	     return buf;
         }
    }
    if (*p1 == '+') {
       p1++;
    } else {
       *p2++ = '?';
    }

    while (*p1 != 0) {
       if ( *p1 == ':' ) {
            *p2++ = *p1++;
	    if (*p1 == '+' ) {
	       p1++;
	    } else {
	       *p2++ = '?';
	    }
	    if (*p1 == '?') {
                p1++;
            }
       } else {
            *p2++ = *p1++;
       }
       if (p1 == buf + sizeof(buf)) {
	   *p2 = 0;
	   return buf;
       }
    }
    *p2 = 0;
    return buf;
}
/* ===========================================================================
** ROUTINE	upsugo_bldqual()
**
** build the optional and required all possible combination list 
** This will return the best to worst match with up to 2 optionals
** after that the order of alphabetical bias.  If that is unacceptable
** the whole list will have to be resorted again based on the number
** of elements (seperated) by :'s and then alphabetically
*/
int upsugo_bldqual(struct ups_command * const uc, char * const inaddr)
{
 char * addr;                          /* required qualifier string */
 char * oaddr;                         /* optional qualifier string */
 char * naddr;                         /* address for build red,opt */
 char * waddr;                         /* work address string */
 char * taddr;                         /* another temporary address */
 char * loc;
 int onq=0;                            /* parsing a ? element */
 int onc=0;                            /* parsing a , element */
 int done=0;
 int qcount=0;                         /* number of optional qualifiers */
 int i,j;
 int opinit=0;
 char *optionals[10]; /* OH NO!! artifical limit of 10 optionals */
 char *fix_inaddr;
 int is_decl;
 t_upslst_item *l_ptr = 0;

 uc->ugo_reqqualifiers = strdup(inaddr);

 if (ups_get_default_quals()) 
 {
   char mergebuf[512];
   strcpy(mergebuf, inaddr);
   if (*inaddr) {
     strcat(mergebuf,":");
   }
   strcat(mergebuf, ups_get_default_quals());
   fix_inaddr = strdup(mergebuf);

   ups_make_default_quals_optional();
 } else {
   fix_inaddr = strdup(inaddr);
 }

 is_decl = ( g_command_verb == e_undeclare ||
             g_command_verb == e_declare );

 fix_inaddr = ugo_old2newquals( fix_inaddr, is_decl );

 if ( strchr(fix_inaddr,'?') == 0) {       /* no optional qualifiers */
  addr=upsutl_str_create(fix_inaddr,'p');
  (void) upsutl_str_sort(addr,':');
  uc->ugo_qualifiers = upslst_add(uc->ugo_qualifiers,addr);
 } else {
  addr=upsutl_str_create(fix_inaddr,'p');
/* remove all ?qualifiers from required string */
  waddr=addr;				/* work address */
  while (*waddr)                      /* while not end of string */
  { if (*waddr==':')                  /* if : not an optional */
    { onq=0;
    } else {
      if (*waddr=='?'||onq)           /* on optional start converting */
      { onq=1;                        /* option to space until it's   */
        *waddr=' ';                   /* completely spaces and string */
      }                               /* no longer contains the optional */
    }                                 /* qualifiers in the list */
    ++waddr;
  }
  waddr=addr;                         /* */ 
  while (*waddr&&!done)               /* this is to make the first qualifer */
  { if (*waddr!=' '&&*waddr!=':')     /* not optional if it's not ? since it */
    { done=1;                         /* did not begin with a : */
    } else {                          /* */
      if (*waddr==':') 
      { *waddr=' ';
        done=1;
      }
    }
    ++waddr;
  }
  waddr=upsutl_str_create(addr,'p');   /* new list trimed */
  upsmem_free(addr);                   /* disgard work string */
  addr=waddr;                          /* required string in addr */
  oaddr=upsutl_str_create(fix_inaddr,'p');
  onc=1;                               /* must assume first no spec is , */
  waddr=oaddr;				/* work address */
  while (*waddr)                       /* now we have build the optional */
  { if (*waddr=='?')                   /* list removing the required */
    { onc=0;                           /* list elements */
    } else {
      if (*waddr==':'||onc)
      { onc=1;
        *waddr=' ';
      }
    }
    ++waddr;
  }
  waddr=oaddr;
  done=0;
  while (*waddr&&!done)                /* also have to make first optional */
  { if (*waddr!=' '&&*waddr!='?')
    { done=1;
    } else { 
      if (*waddr=='?') 
      { *waddr=' ';
        done=1;
      }
    }
    ++waddr;
  }
  waddr=upsutl_str_create(oaddr,'p');   /* new list trimed */
  upsmem_free(oaddr);                   /* disgard work string */
  oaddr=waddr;                          /* optional string in oaddr */
  (void) upsutl_str_sort(oaddr,'?');
  optionals[0]=oaddr;
  ++qcount;
  waddr=oaddr;
  while((loc=strchr(waddr,'?'))!=0)     /* build a optional argument */
  { optionals[qcount]=loc+1;            /* array and with a count    */
    ++qcount;
    *loc=0;
    waddr=loc+1;
  }
/* changed to reverse count, this will allow it to build and return the
** highest number of matches AND in alphabetical significance (since they
** are sorted as well) for atleast up to 2 optional qualifiers without
** resorting the list based on number of elements. upslst_insert 
*/
    for ( i=(1<<(qcount))-1; i >=0; --i)  /* this is the n*2-1 routine */
    { opinit=0;                           /* that works out all possible */
      waddr=0;                            /* options */
      for ( j=0; j < qcount; ++j)
      { if ( i & (1<<(qcount-j-1)) )
        { if(!opinit)
          { if (waddr) upsmem_free(waddr); /* lint said so, I don't believe */
            waddr=upsutl_str_create(optionals[j],' '); 
            opinit=1;
          } else { 
            /* this one I did believe... */
            taddr=upsutl_str_crecat(waddr,":"); 
            upsmem_free(waddr);
            waddr=taddr;
            taddr=upsutl_str_crecat(waddr,optionals[j]); 
            upsmem_free(waddr);
            waddr=taddr;
          }
        }
      }
      if ( *addr != 0 ) /* required as well as optional */
      { if (waddr) /* Added 30-Jan-1998 didn't think this could happen.. */ 
        { naddr=upsutl_str_crecat(addr,":");
          taddr=upsutl_str_crecat(naddr,waddr); /* fix leak assign */
          upsmem_free(naddr);
          upsmem_free(waddr);
          naddr=taddr;
        } else {
          naddr=addr;
        }
      } else { 
/* if there is optionals with no required last one will be "" */
        if ( waddr != 0 ) 
        { naddr=waddr;
        } else { 
          naddr=addr; /* should be a null string yes? */
        }
      }
      (void) upsutl_str_sort(naddr,':'); 
      uc->ugo_qualifiers = upslst_add(uc->ugo_qualifiers,naddr);
    }
 }
 for ( l_ptr = upslst_first(uc->ugo_qualifiers); l_ptr; l_ptr = l_ptr->next) {
  upsver_mes(3,"%s Qualifier  %s in qual list\n",UPSUGO,l_ptr->data);
 }
 return(0);
}
/* ===========================================================================
** ROUTINE	upsugo_liststart(t_upsugo_command * const a_command_line)
**
*/
void upsugo_liststart(t_upsugo_command * const a_command_line)
{
  /* move all lists in the command line to their first element.  there
     may be a better way to do this, maybe dave can take a look at it */
/* leaving this alone could do it as I add but then I may do a LOT of 
   upslst_first might as well do it once here                         */
  a_command_line->ugo_auth = upslst_first(a_command_line->ugo_auth);
  a_command_line->ugo_flavor = upslst_first(a_command_line->ugo_flavor);
  a_command_line->ugo_host = upslst_first(a_command_line->ugo_host);
  a_command_line->ugo_key = upslst_first(a_command_line->ugo_key);
  a_command_line->ugo_qualifiers = 
                         upslst_first(a_command_line->ugo_qualifiers);
  a_command_line->ugo_db = upslst_first(a_command_line->ugo_db);
  a_command_line->ugo_chain = upslst_first(a_command_line->ugo_chain);
}

/* ===========================================================================
** ROUTINE	upsugo_getarg( int argc, char * argv[])
**
** DESCRIPTION
**	Ups_ugo_getarg replaces getopt. It will always return
**	the next argument/option from the argc and argv pointers.
**	It does not modify either of its arguments.
**
** Returns
**	Upsugo_getarg returns either of the following:
**
**	The pointer to the next argument, or
**	0 if no more arguments exist.
** ==========================================================================
*/

char * upsugo_getarg( const int argc, char *argv[], char ** const argbuf)
{

    static int	arg_end;
    static char ** argpt = 0;
    static char * buff = 0;
    char * c;
    static char d[3];

    if( argc < 2 )
	{
	argpt = 0;
	argindx = 0;
	return  0;
	}

    if( argv == 0 )
	{
	argpt = 0;
	argindx = 0;
	return 0;
	}


    if( argv != argpt)
	{
	if( buff )
	    FREE(buff);
	argpt = argv;
	argindx = 0;
	arg_end = 0;
	}
    if( (*argbuf == 0) && (buff != 0))
	FREE(buff);

    if( argv[argindx] == 0 )
	{
	++arg_end;
	return  0;
	}

    if(*argbuf == 0)
	{

	if( ++argindx < argc )
	    {
	    if((*argv[argindx] == '-') && (strlen(argv[argindx]) >2))
		{
		buff = (char *) malloc((size_t)(strlen(argv[argindx]) +1));
		(void) strcpy(buff, argv[argindx]);
		*argbuf = buff + 2;
		}
	    return argv[argindx];
	    }

	return  0;
    	}	
/* else if argbuf != 0
   ---------------------- */

	d[0] = '-';
	d[1] = **argbuf;
	d[2] = '\0';
	c = *argbuf;
	*argbuf = c+1;
	if(**argbuf == '\0')
	    {
	    FREE(buff);
	    *argbuf = 0;
	    }
	return &d[0];

}

/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_rearg
**                                                                           
** DESCRIPTION                                                               
** 
** rearg copies argv/c argv/c_new, except that commas will be treated
** as if they had whitespace around them -- they will appear in their own
** argv_new[i].
**                                                                           
** VALUES RETURNED                                                           
**
** UPS_ERROR : if there is going to be a buffer overflow
** UPS_SUCCESS : if everything was okay
**      +++                                                                  
**                                                                           
** ==========================================================================
*/                                                                           
int upsugo_rearg(const int argc_old,char *argv_old[],int * const argc_new,char *argv_new[])
{

	int 		lcv_old,
			lcv_new;
	size_t		str_length = 0;
	char		*temp;
	char	*string;

        string = (char *) malloc(sizeof(char) + 1);
        string[0] = ','; string[1] = '\0';

	lcv_old = lcv_new = 0;

	while (lcv_old < argc_old)

	   {

	   if (lcv_new >= MAX_ARGS - 1)   { /* Make sure no buffer overflow */
	       (void) fprintf(stderr, 
	         "ups: Error - maximum command line buffer limit exceeded\n");
	       return (1);
	   }

	   if ( strcmp(argv_old[lcv_old],string) == 0)
		argv_new[lcv_new++] = argv_old[lcv_old++];
	   else
		/* we know argv_old[lcv_old] isn't a comma or a space*/
	   	{
		temp = argv_old[lcv_old++];
			/* temp points to beginning of next argument */

		if ( strchr(temp,(int)string[0]) != NULL )
		   while ( (temp != NULL) && (strlen(temp) != 0)  )
			{
			str_length = strcspn(temp,string);
			if (str_length)
			     {
			     argv_new[lcv_new] = (char *) malloc(str_length+1);
			     (void) strncpy(argv_new[lcv_new],temp,str_length);
			     argv_new[lcv_new++][str_length] = '\0';
			     }
			temp = strchr(temp,(int) string[0]);
			if (temp != NULL)
			     {
			     argv_new[lcv_new++] = string;
			     temp++; /* skip comma or space*/
			     }

			}

	 	else
		    /* there is no comma in temp */
			argv_new[lcv_new++] = temp;

	       }
	   }

        argv_new[lcv_new++] = string;
	*argc_new = lcv_new;

	return(0);
}

/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_free
**                                                                           
** DESCRIPTION - free a ups command structure.
** 
** NOTE: This free's the data within the linked list of ugo_command structures
**       it does NOT (yet? and how?) remove the structure from the linked list.
**
**                                                                           
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/                                                                           
int upsugo_free (struct ups_command * const uc)
{
    if(uc)
    { if ( uc->ugo_product ) 
         upsmem_free(uc->ugo_product); 
      if ( uc->ugo_version ) 
         upsmem_free(uc->ugo_version); 
      if ( uc->ugo_auth ) 
         (void) upslst_free(uc->ugo_auth,'d'); 
      if ( uc->ugo_flavor ) 
         (void) upslst_free(uc->ugo_flavor,'d'); 
      if ( uc->ugo_host ) 
         (void) upslst_free(uc->ugo_host,'d'); 
      if ( uc->ugo_key ) 
         (void) upslst_free(uc->ugo_key,'d'); 
      if ( uc->ugo_tablefiledir ) 
         upsmem_free(uc->ugo_tablefiledir); 
      if ( uc->ugo_tablefile ) 
         upsmem_free(uc->ugo_tablefile); 
      if ( uc->ugo_anyfile ) 
         upsmem_free(uc->ugo_anyfile); 
      if ( uc->ugo_origin ) 
         upsmem_free(uc->ugo_origin); 
      if ( uc->ugo_compile_dir) 
         upsmem_free(uc->ugo_compile_dir); 
      if ( uc->ugo_compile_file)
         upsmem_free(uc->ugo_compile_file); 
      if ( uc->ugo_options ) 
         upsmem_free(uc->ugo_options); 
      if ( uc->ugo_passed ) 
         upsmem_free(uc->ugo_passed); 
      if ( uc->ugo_description ) 
         upsmem_free(uc->ugo_description); 
/*      if ( uc->ugo_override ) 
         upsmem_free(uc->ugo_override);  */
      if ( uc->ugo_qualifiers ) 
         (void) upslst_free(uc->ugo_qualifiers,'d'); 
      if ( uc->ugo_productdir ) 
         upsmem_free(uc->ugo_productdir); 
      if ( uc->ugo_archivefile ) 
         upsmem_free(uc->ugo_archivefile); 
      if ( uc->ugo_upsdir ) 
         upsmem_free(uc->ugo_upsdir); 
      if ( uc->ugo_db )
	 upsugo_free_ugo_db( uc->ugo_db );
      if ( uc->ugo_chain ) 
         (void) upslst_free(uc->ugo_chain,'d'); 
      upsmem_free(uc);
    } return (0);
}

/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_free_ugo_db
**                                                                           
** DESCRIPTION                                                               
**
**                                                                           
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/                                                                           
void upsugo_free_ugo_db( t_upslst_item * const ugo_db ) 
{
  t_upslst_item *l_db;

  if ( ! ugo_db )
    return;

  l_db = upslst_first( ugo_db );
  while( l_db ) {
    t_upstyp_db* db = (t_upstyp_db * )l_db->data;
    l_db = upslst_delete( l_db, db, ' ' );
    if ( upsmem_get_refctr( db ) <= 0 ) {
      upsmem_free( db->name );
      upsmem_free( db );
    }
  }
}

/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_dump
**                                                                           
** DESCRIPTION                                                               
**
**                                                                           
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/                                                                           
int upsugo_dump (struct ups_command * const uc,
		 const unsigned int prnt_ptr)
{
    if(uc)
    { if ( uc->ugo_product ) 
         (void) printf("Product:          %s\n",uc->ugo_product); 
      if ( uc->ugo_version ) 
         (void) printf("Version:          %s\n",uc->ugo_version); 
      if ( uc->ugo_auth ) 
         upsugo_prtlst(uc->ugo_auth,  "Authorized Nodes: ",prnt_ptr); 
      if ( uc->ugo_flavor ) 
         upsugo_prtlst(uc->ugo_flavor,"Flavor:           ",prnt_ptr); 
      if ( uc->ugo_osname) 
         upsugo_prtlst(uc->ugo_osname,"Osname:           ",prnt_ptr); 
      if ( uc->ugo_host ) 
         upsugo_prtlst(uc->ugo_host,  "Host:             ",prnt_ptr); 
      if ( uc->ugo_key ) 
         upsugo_prtlst(uc->ugo_key,   "Key:              ",prnt_ptr); 
      if ( uc->ugo_tablefiledir ) 
         (void) printf("Tablefiledir:     %s\n",uc->ugo_tablefiledir); 
      if ( uc->ugo_tablefile ) 
         (void) printf("Tablefile:        %s\n",uc->ugo_tablefile); 
      if ( uc->ugo_anyfile ) 
         (void) printf("Anyfile:          %s\n",uc->ugo_anyfile); 
      if ( uc->ugo_options ) 
         (void) printf("Options:          %s\n",uc->ugo_options); 
      if ( uc->ugo_passed ) 
         (void) printf("Passed:          %s\n",uc->ugo_passed); 
      if ( uc->ugo_description ) 
         (void) printf("Description:      %s\n",uc->ugo_description); 
/*      if ( uc->ugo_override ) 
         (void) printf("Override:         %s\n",uc->ugo_override); */
      if ( uc->ugo_L) 
         (void) printf("LongListing\n"); 
      if ( uc->ugo_P) 
         (void) printf("Product not in a database (-P)\n"); 
      if ( uc->ugo_qualifiers ) 
         (void) upsugo_prtlst(uc->ugo_qualifiers,"Qualifiers:       ",prnt_ptr); 
      if ( uc->ugo_productdir ) 
         (void) printf("Productdir:       %s\n",uc->ugo_productdir); 
      if ( uc->ugo_archivefile ) 
         (void) printf("Archivefile:      %s\n",uc->ugo_archivefile); 
      if ( uc->ugo_upsdir ) 
         (void) printf("Upsdir:           %s\n",uc->ugo_upsdir); 
      if ( uc->ugo_origin) 
         (void) printf("Origin:           %s\n",uc->ugo_origin); 
      if ( uc->ugo_compile_dir) 
         (void) printf("Compile Dir:           %s\n",uc->ugo_compile_dir); 
      if ( uc->ugo_compile_file) 
         (void) printf("Compile File:           %s\n",uc->ugo_compile_file); 
      if ( uc->ugo_number) 
         (void) printf("UGO number[0-3]+1:           %d\n",uc->ugo_number); 
      if ( uc->ugo_db ) 
         upsugo_prtdb(uc->ugo_db,"DB:               ",prnt_ptr); 
      if ( uc->ugo_chain ) 
         upsugo_prtlst(uc->ugo_chain,"Chains:           ",prnt_ptr); 
      if ( uc->ugo_help )
         (void) printf("--- HELP !!! ---\n"); 
      (void) printf("ugo_v %d and UPS_VERBOSE %d\n",uc->ugo_v,UPS_VERBOSE); 
    } return (0);
}

/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_prtlst
**                                                                           
** DESCRIPTION                                                               
**
**                                                                           
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/
void upsugo_prtlst( t_upslst_item * const list_ptr , char * const title ,
   const unsigned int prnt_ptr)
{
  t_upslst_item *l_ptr;
  int count = 0;

  /*
   * Note use of upslst_first(), to be sure to start from first item
   */
  if (prnt_ptr)
     (void) printf("%s\n",title);
  else
     (void) printf("%s",title);
  for ( l_ptr = upslst_first( list_ptr ); l_ptr; l_ptr = l_ptr->next, count++ )
  { if(prnt_ptr)
      {(void) printf("ref count %d\n",upsmem_get_refctr(l_ptr->data));
       (void) printf( "%03d: p=%08x, i=%08x, n=%08x, data=%s\n",
            count, (int)l_ptr->prev, (int)l_ptr,
            (int)l_ptr->next, (char *)l_ptr->data );
      }
   else
      {
      if (l_ptr == upslst_first(list_ptr))                           
         (void) printf ("%s\n", (char *)l_ptr->data);              
      else                                                      
         (void) printf ("                  %s\n",  (char *)l_ptr->data); 
      }
  }
}
/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_prtdb
**                                                                           
** DESCRIPTION                                                               
**
**                                                                           
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/
void upsugo_prtdb( t_upslst_item * const list_ptr , char * const title ,
   const unsigned int prnt_ptr)
{
  t_upslst_item *l_ptr;
  struct upstyp_db *addr;
  int count = 0;

  /*
   * Note use of upslst_first(), to be sure to start from first item
   */
  if (prnt_ptr)
     (void) printf("%s\n",title);
  else
     (void) printf("%s",title);
  for ( l_ptr = upslst_first( list_ptr ); l_ptr; l_ptr = l_ptr->next, count++ )
  {addr=l_ptr->data;
   if(prnt_ptr)
      {
      (void) printf("ref count %d\n",upsmem_get_refctr(l_ptr->data));
      (void) printf("%03d: p=%08x, i=%08x, n=%08x, ",
           count, (int)l_ptr->prev, (int)l_ptr, (int)l_ptr->next);
      addr=l_ptr->data;
      (void) printf("data=%s\n", (char *)addr->name);
      }
   else
      {
      if (l_ptr == upslst_first(list_ptr))
         (void) printf ("%s\n", (char *)addr->name);
      else
         (void) printf ("                  %s\n",  (char *)addr->name);
      }
  }
}

/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_getenv
**                                                                           
** DESCRIPTION                                                               
**
** This routine get the value of the environment SETUP_<PRODUCT_NAME>.
**                                                                           
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/                                                                           

char *upsugo_getenv( char * const prod_name )
{
  static char buf[256];
  
  (void) strcpy( buf, SETUPENV );
  (void) strcat( buf, upsutl_upcase( prod_name ) );

  return (char *)getenv( buf ); 
}

/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_env
**                                                                           
** DESCRIPTION                                                               
**
** This routine get the value of the environment setup and converts it
** to a ugo command structure.  It constructs an argc/argv[] combo, using
** spaces or tabs as delimiters.  It can also handle simple quoted strings,
** using either pairs of quotes or apostrophes.
**                                                                           
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/                                                                           
t_upsugo_command *upsugo_env(char * const product,char * const validopts)
{
     char * setup_env;                           /* SETUP_PROD value */
     char * waddr;                               /* work address */
     struct ups_command * uc=0;
     int argc=0;
     int    count=0;
     int    length=0;
     int    verbose=0;
     char ** argv;
     t_upslst_item *hold = 0;
     /* setup_prod = (char *) malloc((size_t)(strlen(product) +7));*/
     if((setup_env = (char *)upsugo_getenv(product)) != 0)
     {
/* I'm going to count the number of spaces in the environment variable
** there cannot be more arguments than spaces...
*/
       setup_env += strspn(setup_env, " \t");	/* Skip leading whitespace */
       waddr=setup_env;
       while ((waddr != 0) && (*waddr != '\0'))
             {
                waddr += strcspn(waddr, " \t");	/* Find next whitespace */
                waddr += strspn(waddr, " \t");	/* Skip next whitespace */
                count++;
             }
       count++;  /* add one more for the program who called */
       argv = (char **) malloc((size_t)count*sizeof(char *));
       argv[0] = (char *) malloc((size_t)(strlen("upsugo_env") +1));
       (void) strcpy(argv[0],"upsugo_env");
       waddr=setup_env;
       for (argc = 1;argc < count;argc++)
           {
             char * delim;		/* Pointer to delimiter string */
             delim = " \t";
             if (*waddr == '\0') break;
             if (*waddr == '\'')
	     {
               delim = "'";
               waddr++;
             }
             if (*waddr == '"')
	     {
               delim = "\"";
               waddr++;
             }
             length = (int)strcspn(waddr, delim);
             argv[argc] = (char *) malloc((size_t)(length + 1));
             (void) strncpy(argv[argc],waddr,(size_t)length);
             argv[argc][length] = '\0';
             waddr += length+1;
             waddr += strspn(waddr, " \t");
           }
       hold=ugo_commands;
       ugo_commands=0;
       verbose=UPS_VERBOSE;
       uc=upsugo_next(argc,argv,validopts);
       (void) upslst_free(ugo_commands,' '); /* do NOT delete data */
       ugo_commands=hold;
       UPS_VERBOSE=verbose;
     }
     return(uc);
}

/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_bldcmd
**                                                                           
** DESCRIPTION                                                               
**
** This builds the command structure from the arguement from the string
** this is used for reading things like setupRequired in the table file
**                                                                           
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/                                                                           
t_upsugo_command *upsugo_bldcmd(char * const cmdstr,char * const validopts)
{
     char * waddr;                               /* work address */
     struct ups_command * uc=0;
     int    argc=0;
     int    count=0;
     int    length=0;
     int    verbose=0;
     char ** argv;
     t_upslst_item *hold = 0;
     
       waddr=cmdstr;
       while ((waddr != 0) && (strlen(waddr) > 0))
             { if ((waddr = strchr(waddr,' ')) != 0) 
                  { for( ; (*waddr == ' ') ; waddr++ ) ; }
                count++;
             }
       count++;  /* add one more for the program who called */
       argv = (char **) malloc((size_t)count*sizeof(char *));
       argv[0] = (char *) malloc((size_t)(strlen("upsugo_bldcmd") +1));
       (void) strcpy(argv[0],"upsugo_bldcmd");
       waddr=cmdstr;
       for (argc = 1;argc < count;argc++)
           { length = (int)strcspn(waddr," ");
             argv[argc] = (char *) malloc((size_t)(length + 1));
             (void) strncpy(argv[argc],waddr,(size_t)length);
             argv[argc][length] = '\0';
             if ((waddr = strchr(waddr, ' ')) != 0) 
                { waddr++;
                  for( ; (*waddr == ' ') ; waddr++ ) ;
                }
           }
     hold=ugo_commands;
     ugo_commands=0;
     verbose=UPS_VERBOSE;
     uc=upsugo_next(argc,argv,validopts);
     (void) upslst_free(ugo_commands,' '); /* do NOT free data!!! */
     ugo_commands=hold;
     UPS_VERBOSE=verbose;
     return(uc);
}

/* ==========================================================================
**                                                                           
** ROUTINE: upsugo_next
**                                                                           
** DESCRIPTION                                                               
**
**                                                                           
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/                                                                           
t_upsugo_command *upsugo_next(const int old_argc,
                              char *old_argv[],
                              char * const validopts)
{
  char   *arg_str;

  char   * addr;
  char   * loc;
  int    ups_argc;            /* argv and argc with white space and commas  */
  char   *ups_argv[MAX_ARGS]; /* reformatted */
   
  char   **argbuf;		/* String to hold residual argv stuff*/
				/* returned by upsugo_getarg */
				/* if contents are used is reset to */
				/* to 0 before recalling getarg */
/* Initialize those pesky variables
    -------------------------------- */
  struct ups_command * uc;
  struct ups_command * luc=0;
  if ( ugo_commands )
  { /* subsequent call */ 
    /* dealloc your brain out */ 
    (void) upsugo_free(ugo_commands->data);	/* free all lists etc in struct */
    ugo_commands=upslst_delete(ugo_commands,ugo_commands->data, 'd');
    if (ugo_commands && ugo_commands->data) /* && data needed? */
    {  luc = ugo_commands->data;
       upsugo_setshell(luc);
       UPS_VERBOSE = luc->ugo_v;
	upsugo_liststart(luc);  /* move all lists to first element */
       return (t_upsugo_command *)ugo_commands->data; 
    } else {
       return 0;
    }
  } else { 
/* this is VERY important... to make sure argindx is 0 */
/* if there is a subsequent call to upsugo_next for a WHOLE NEW command
** line to be parsed the index must be reset!!!
*/
    uc=(struct ups_command *)upsmem_malloc( sizeof(struct ups_command));
    (void) memset (uc, 0, sizeof(struct ups_command));
    argindx=0;
    argbuf = (char **)upsmem_malloc(sizeof(char *)+1);
    *argbuf = 0;
    (void) upsugo_rearg(old_argc,old_argv,&ups_argc,ups_argv);
    while ((arg_str= upsugo_getarg(ups_argc, ups_argv , argbuf)) != 0)
    { if(*arg_str == '-')      /* is it an option */
      { if (!strchr((validopts ? validopts : ""),(int)*(arg_str+1))) { 
           upserr_add(UPS_INVALID_ARGUMENT, UPS_FATAL, arg_str+1);
        }
        switch(*(arg_str+1))      /* which flag was specified */
        { /* Single flag cases */
          case_B
          case_a case_C case_e case_E case_F 
          case_j case_k case_l case_L case_P 
          case_R case_s case_S case_v case_V 
          case_w case_W /* case_x */ case_X case_y 
          case_Y case_Z case_help
          /* Chain cases */ 
          case_c case_d case_n case_t case_o
          /* List elements */
          case_g            /* also a chain */
          case_f case_K case_A case_h case_H
          /* single values */ 
          case_b case_D case_G case_m 
          case_M case_N case_O case_p 
          case_r case_T case_u case_U 
          case_0 case_1 case_2 case_3 case_4 case_5 case_6 case_7 /* number sets */
          case_q case_z               /* special cases */
          case_dot
          default:
             errflg = 1;
        }
      } else {
        if ( strchr(arg_str,',') != 0 )
        { (void) upsugo_ifornota(uc);
          ugo_commands = upslst_add(ugo_commands,uc);
          uc=(struct ups_command *)upsmem_malloc( sizeof(struct ups_command));
          (void) memset (uc, 0, sizeof(struct ups_command));
        } else { 
          addr=upsutl_str_create(arg_str,' ');
          if ( !uc->ugo_product ) 
          { uc->ugo_product = addr;
            if (uc->ugo_dot) 
            {
               /* we had a -. before the product name, so now we can
                  figure out the table file name */
               uc->ugo_tablefile = strcat(strcpy(upsmem_malloc(strlen(uc->ugo_product)+7),uc->ugo_product),".table");
            }
          } else { 
            if ( !uc->ugo_version )
            { uc->ugo_version = addr ;
            } else { 
              upserr_add(UPS_INVALID_ARGUMENT,UPS_FATAL,addr);
            }
          } 
        }
      }
    }
    if (!ugo_commands) 
    { addr=upsutl_str_create("*",' ');
      uc->ugo_product = addr;
      (void) upsugo_ifornota(uc);             
      ugo_commands = upslst_add(ugo_commands,uc);
    }
  }
  ugo_commands=upslst_first(ugo_commands);
  luc = ugo_commands->data;
  upsugo_setshell(luc);
  UPS_VERBOSE=luc->ugo_v;
/* don't want to change now but I don't think this is right??? */
  upsugo_liststart(luc);      /* move all lists to first element */
  return (t_upsugo_command *)ugo_commands->data; 
}

