/************************************************************************
 *
 * FILE:
 *       ups_utils.c
 * 
 * DESCRIPTION: 
 *       Test some of the utility routines.
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
 *       29-Jul-1997, EB, first
 *
 ***********************************************************************/

/* standard include files */

/* ups specific include files */
#include "upsugo.h"
#include "upsget.h"
#include "upserr.h"
#include "upsmat.h"
#include "upstyp.h"

/*
 * Definition of public variables.
 */

/*
 * Declaration of private functions.
 */

static void test_find_manpages(void);

/*
 * Definition of global variables.
 */

/*
 * Definition of public functions.
 */

int main (void)
{

  test_find_manpages();

  return 0;
}


/* test upsget_man_source_dir */
static void test_find_manpages(void)
{

  /* Information needed  -
           from the instance:  prod_dir
	                       ups_dir

	   from the db config: prod_dir_prefix */
  t_upstyp_db db;
  t_upstyp_config config;
  t_upstyp_matched_instance inst;
  t_upstyp_instance vinst;
  char *buf = NULL;

  db.config = &config;
  inst.version = &vinst;

  /* Test 1 : with no prod_dir_prefix */
  db.config->prod_dir_prefix = NULL;
  (void) printf("Test with NO prod_dir_prefix\n");

  /* Test 1a: with both prod_dir and ups_dir */
  vinst.prod_dir = "prodDir";
  vinst.ups_dir = "upsDir";
  buf = upsget_man_source_dir(&inst, &db, 0);
  (void) printf("PD=%s, UD=%s, ManPages=%s\n", vinst.prod_dir, vinst.ups_dir, buf);

  /* Test 1b: with neither prod_dir or ups_dir */
  vinst.prod_dir = NULL;
  vinst.ups_dir = NULL;
  buf = upsget_man_source_dir(&inst, &db, 0);
  (void) printf("PD=%s, UD=%s, ManPages=%s\n", " ", " ", buf);

  /* Test 1c: with a prod_dir and no ups_dir */
  vinst.prod_dir = "prodDir";
  vinst.ups_dir = NULL;
  buf = upsget_man_source_dir(&inst, &db, 0);
  (void) printf("PD=%s, UD=%s, ManPages=%s\n", vinst.prod_dir, " ", buf);

  /* Test 1d: with no prod_dir and a ups_dir */
  vinst.prod_dir = NULL;
  vinst.ups_dir = "upsDir";
  buf = upsget_man_source_dir(&inst, &db, 0);
  (void) printf("PD=%s, UD=%s, ManPages=%s\n\n", " ", vinst.ups_dir, buf);


  /* Test 2 : with prod_dir_prefix */
  db.config->prod_dir_prefix = "prodDirPrefix";
  (void) printf("Test with prod_dir_prefix = %s\n", db.config->prod_dir_prefix);

  /* Test 2a: with both prod_dir and ups_dir */
  vinst.prod_dir = "prodDir";
  vinst.ups_dir = "upsDir";
  buf = upsget_man_source_dir(&inst, &db, 0);
  (void) printf("PD=%s, UD=%s, ManPages=%s\n", vinst.prod_dir, vinst.ups_dir, buf);

  /* Test 2b: with neither prod_dir or ups_dir */
  vinst.prod_dir = NULL;
  vinst.ups_dir = NULL;
  buf = upsget_man_source_dir(&inst, &db, 0);
  (void) printf("PD=%s, UD=%s, ManPages=%s\n", " ", " ", buf);

  /* Test 2c: with a prod_dir and no ups_dir */
  vinst.prod_dir = "prodDir";
  vinst.ups_dir = NULL;
  buf = upsget_man_source_dir(&inst, &db, 0);
  (void) printf("PD=%s, UD=%s, ManPages=%s\n", vinst.prod_dir, " ", buf);

  /* Test 2d: with no prod_dir and a ups_dir */
  vinst.prod_dir = NULL;
  vinst.ups_dir = "upsDir";
  buf = upsget_man_source_dir(&inst, &db, 0);
  (void) printf("PD=%s, UD=%s, ManPages=%s\n\n", " ", vinst.ups_dir, buf);

}
