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

#include "ups.h"
 
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

	struct ups_command * uc;

(void) printf("string = %s<\n",upsget_translation_env("~fagan/spit"));
upserr_output();
(void) printf("string = %s<\n",upsget_translation_env("~fagan "));
upserr_output();
(void) printf("string = %s<\n",upsget_translation_env("~fagan"));
upserr_output();
while ((uc = upsugo_next(argc,argv,"AacCdfghKltmMNoOPqrTuUvz")) != 0 )
      { (void) ups_list(uc,0); 
        upserr_output();
       }
upsfil_stat(1);
	return 0;
}
