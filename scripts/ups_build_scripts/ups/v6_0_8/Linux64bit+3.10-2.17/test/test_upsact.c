/************************************************************************
 *
 * FILE:
 *       test_ups_action.c
 * 
 * DESCRIPTION: 
 *       Test ups_action routines.
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
 *       02-Sept-1997, EB, first
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>
#include <string.h>

/* ups specific include files */
#include "upsfil.h"
#include "upserr.h"
#include "upsact.h"
#include "upslst.h"
#include "upsact.h"
#include "upsugo.h"
#include "upsmat.h"
#include "ups_list.h"

/*
 * Definition of public variables.
 */

/*
 * Declaration of private functions.
 */
static void test_upsact_parse(void);

void list_output(const t_upslst_item * const a_mproduct_list,
                 const t_upsugo_command * const a_command_line);

/*
 * Definition of global variables.
 */

#ifndef NULL
#define NULL 0
#endif

void print_cmd( t_upsact_cmd *cmd )
{
  int i = 0;
  if ( !cmd )
    return;

  (void) printf( "\nicmd = %d\n", cmd->icmd );
  for ( i=0; i<cmd->argc; i++ ) {
    (void) printf( "   %d = %s\n", i, cmd->argv[i] );
  }
}

/*
 * Definition of public functions.
 */

char *uk[] = {"_sjkd=233", "_abc=yyy", "_kul=oo99", "_ohoh2", "_ohoh=4" };
char *ukk[] = {"_sjkd", "_abc", "_kul", "_ohoh", "_ohoh2" };
t_upslst_item *a_l = 0;

/*-----------------------------------------------------------------------
 * main
 *
 * test ups_action routines
 *
 */
int  main( const int argc, char * const argv[] )
{
  /*
  int i = 0;
  t_ups_product *prod_ptr = NULL;
  t_upslst_item *inst_ptr = NULL;
  t_upslst_item *act_ptr = NULL;
  t_ups_instance *inst;
  t_ups_action *act;


  if ( argc <= 1 ) {
    (void) printf( "Usage: test_upsact table_file\n" );
    exit( 1 );
  }

  for ( i=1; i<argc; i++ ) {
    
    prod_ptr = upsfil_read_file( argv[i] );  

    if ( prod_ptr ) {
      inst_ptr = upslst_first( prod_ptr->instance_list );
      for( ; inst_ptr; inst_ptr = inst_ptr->next ) {
	inst = (t_ups_instance *)inst_ptr->data;
	act_ptr = upslst_first( inst->action_list );
	for( ; act_ptr; act_ptr = act_ptr->next ) {
	  act = (t_ups_action *)act_ptr->data;
	  upsact_translate( inst, act->action );
	}
      }
    }
  }
  */


  /*  test_upsact_parse();
  upserr_output();
  */

  /*
  t_upstyp_instance inst;
  t_upslst_item *l_usr = 0;
  char *cp;
  int i;

  for ( i=0; i<5; i++ ) {
    cp = upsutl_str_create( uk[i] );
    l_usr = upslst_add( l_usr, cp );
  }
  inst.user_list = l_usr;

  for ( i=0; i<5; i++ ) {
    cp = upskey_inst_getuserval( &inst, ukk[i] );
    (void) printf( "%s = %s\n", ukk[i], cp );
  }
  */
  

  test_upsact_parse();
  upserr_output();

  return 0;
}

/*
 * Definition of private functions.
 */

/*-----------------------------------------------------------------------
 * test_upsact_parse
 *
 * test the routine that parses the actions. for each supported action, call
 * the parsing routine and examine the output.
 *
 * Input : none
 * Output: none
 * Return: none
 */
static void test_upsact_parse(void)
{
  int i;
  char *action_line[] = {
    "FILETEST(myfile  ,\"-w  \",  \"can't touch this\"  )",
    "SETUPOPTIONAL(\"-d -f purina -q siamese kitty \")",
    "SETUPREQUIRED(\"-d -f purina -q \"tabby, tiger\" kitty \")",
    "UNSETUPOPTIONAL(\"-d -f $OS_FLAVOR -q siamese kitty \")",
    " UNSETUPREQUIRED(\" kitty \")  ",
    "ENVAPPEND(envVar, \"moses supposes\", \"@\")",
    "ENVREMOVE(envVar, \"his toeses are roses\", \"@\")",
    "ENVPREPEND(envVar, \"but moses supposes erroneously\")",
    "ENVSET(envVar, \"for moses he knowses\")",
    "ENVUNSET(envVar)",
    "NOPRODDIR()",
    "EXEACCESS(dothis, setthis)",
    "SOURCE(myscript.$UPS_SHELL)",
    "SOURCECHECK(myscriptcheck.$UPS_SHELL)",
    "EXECUTE(onlythis)",
    "COPYHTML()",
    "COPYINFO($PROD_DIR/info)",
    "COPYMAN($PROD_DIR/man)",
    "COPYNEWS()",
    "DODEFAULTS()",
    "NODEFAULTS()",
    "NOSETUPENV()",
    "NOBOYO()",
    "COPYNEWS",
    NULL
};
t_upsact_cmd *cmd;
t_upsugo_command *ugo_cmd = 0;
t_upslst_item *mproduct_list;
  
  for ( i = 0; action_line[i]; i++ ) {  
    cmd = upsact_parse_cmd( action_line[i] );
    if ( !cmd ) {
      (void) printf("\nInvalid action - %s\n", action_line[i]);
    }
    else {
      /* print_cmd( cmd ); */
    }
  }
  ugo_cmd = upsugo_bldcmd( "-c -f IRIX exmh",
			   "zAacCdfghKtmMNoOPqrTuUv?" );
  
  (void) upsugo_dump( ugo_cmd, 1);
  mproduct_list = upsmat_instance( ugo_cmd, NULL, 1 );
  list_output( upslst_first( mproduct_list ), ugo_cmd);

  (void) upsact_print( ugo_cmd, 0, "setup", e_setup, "tl" );
}
