/************************************************************************
 *
 * FILE:
 *       upsmem.h
 * 
 * DESCRIPTION: 
 *       The file prototypes all the UPS memory handline functions
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
 *       25-Jul-1997, EB, first
 *
 ***********************************************************************/

#ifndef _UPSMEM_H_
#define _UPSMEM_H_

/* standard include files, if needed for .h file */

/* ups specific include files, if needed for .h file */

/*
 * Constans.
 */

/*
 * Types.
 */

/* Union needed for correctly aligning memory */
typedef union _Align  {
   char             filler_1;
   char            *filler_2;
   short            filler_3;
   long             filler_4;
   int              filler_5;
   int             *filler_6;
   double           filler_7;
   float            filler_8;
   void            *filler_9;
   int            (*filler_10)(void);
} UALIGN;

#define MAXALIGN (sizeof(UALIGN))
#define UALIGN(x) ((((x)+MAXALIGN-1)/MAXALIGN)*MAXALIGN)

/*
 * Declaration of public functions.
 */

void *upsmem_malloc(const int a_bytes);
void upsmem_malloc_error(void);          /* was (const int a_bytes) */
void upsmem_free(void *a_data);
int upsmem_get_refctr(const void * const a_data);
void upsmem_inc_refctr(const void * const a_data);
void upsmem_dec_refctr(const void * const a_data);

#endif /* _UPSMEM_H_ */

