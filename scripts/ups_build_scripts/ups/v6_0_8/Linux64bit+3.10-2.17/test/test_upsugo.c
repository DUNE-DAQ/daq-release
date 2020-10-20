/*                                                                           
**========================== Copyright Notice =============================**
**                                                                         **
**       Copyright (c) 1990 Universities Research Association, Inc.        **
**                      All Rights Reserved.                               **
**                                                                         **
**=========================================================================**/
/*                                                                           
** DESCRIPTION                                                               
**      +++                                                                  
**                                                                           
** DEVELOPERS                                                                
**      +++                                                                  
**                                                                           
**      Batavia, Il 60510, U.S.A.                                            
**                                                                           
** ENTRY POINTS                 SCOPE                                        
**      +++                     +++                                          
**                                                                           
** MODIFICATIONS                                                             
**         Date       Initials  Description                                  
**      +++             +++     +++                                          
**                                                                           
** HEADER STATEMENTS                                                         
*/                                                                           

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "ups.h"
#ifndef TRUE
#define TRUE 1
#endif
 
void print_list( t_upslst_item *list_ptr );
/* ==========================================================================
**                                                                           
** ROUTINE: main
**                                                                           
** DESCRIPTION                                                               
**
** VALUES RETURNED                                                           
**      +++                                                                  
**                                                                           
** ==========================================================================
*/                                                                           
int main (argc,argv)
	int	argc;
	char	*argv[];
{

	int 	status = 0;
	struct ups_command * uc;
        char    * cmdline = "-f FlAVoR pRoDa V2"; 
/*        char * cmdline = "tknew v8_1 -p \"bad news\" "; */


/*
	signal(SIGSEGV, ups_signal_handler);
*/
        uc=upsugo_env("TEST","AacCdfghKtmMNoOPqrTuU");
        if (uc) 
	{ (void) upsugo_dump(uc,TRUE);
          (void) upsugo_free(uc);
        }
        uc=0;
while ((uc = upsugo_next(argc,argv,"0123AacCdfghHKtmMNoOpPqrTuUv?z")) != 0 )
      { (void) upsugo_dump(uc,TRUE); 
        upserr_output();
        (void) fprintf(stderr, "-------------------------------------------------\n");
        upserr_clear();
      }
/* prove to myself subsequent calls with new arguments will work 
** for Margrets regression testing
*/
      argc=4;
      argv[0]="yoyoman";
      argv[1]="-firix";
      argv[2]="adprod";
      argv[3]="ver";
while ((uc = upsugo_next(argc,argv,"AacCdfghKtmMNoOPqrTuU")) != 0 )
      { (void) upsugo_dump(uc,TRUE); }
/*
	if (status != UPS_SUCCESS) (void) fprintf(stderr," %s \n", UPS_ERRTXT[status]);
*/
        uc=upsugo_bldcmd(cmdline,"AacCdfghKtmMNoOpPqrTuU");
        if (uc) 
	{ (void) upsugo_dump(uc,TRUE);
          (void) upsugo_free(uc); 
        }
        uc=0;
	return status;

}
