/************************************************************************
 *
 * FILE:
 *       upsmem.c
 * 
 * DESCRIPTION: 
 *       This file contains routines for memory management
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

/* standard include files */
#include <stdlib.h>
#include <stdio.h>

/* ups specific include files */
#include "upserr.h"
#include "upsmem.h"

/*
 * Definition of public variables.
 */

/*
 * Definition of global variables.
 */

#define UPSMEM_INT UALIGN(sizeof(int))
#define UPSMEM_GET_TOP(memPtr) ((char *)memPtr - (UPSMEM_INT))
#define UPSMEM_GET_USER(memPtr) ((char *)memPtr + (UPSMEM_INT))

/*
 * Declaration of private functions.
 */

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * upsmem_malloc
 *
 * Malloc the requested amount. Add 4 bytes at the front of the memory to
 * use as a reference counter for putting things on lists.
 *
 * Input : number of bytes to malloc
 * Output: none
 * Return: address of new memory.  NULL if malloc failed
 */
void *upsmem_malloc(const int a_bytes)
{
  int *dataPtr = 0;
  unsigned int numBytes;

  /* Return if no memory requested */
  if (a_bytes > 0) {
    numBytes = ( unsigned int )(a_bytes + (int )(UPSMEM_INT));
    numBytes = UALIGN(numBytes);         /* make sure alignment is proper */
    dataPtr = (int *)malloc(numBytes);
    if (dataPtr != 0) {
      /* We got the memory, initialize it */
      *dataPtr = 0;

      /* return to the user a pointer pointing past the reference counter */
      dataPtr = (int *)UPSMEM_GET_USER(dataPtr);
    } else {
      /* we did not get the memory.  the following routine should not return
	 here, but should exit */
      upserr_add(UPS_NO_MEMORY, UPS_FATAL, numBytes);
      upsmem_malloc_error( /* (int)numBytes */ );
    }
    
  }
  return (void *)dataPtr;
}

/*-----------------------------------------------------------------------
 * upsmem_malloc_error
 *
 * We were unable to malloc.  print out the error buffer and exit 
 *
 * Input : number of bytes that were used for the malloc
 * Output: none
 * Return: none
 */
void upsmem_malloc_error(void)          /* was (const int a_bytes) */
{
  upserr_output();
  abort();
}

/*-----------------------------------------------------------------------
 * upsmem_free
 *
 * Free the passed memory if the reference count is zero.  We assume that this
 * memory has been previously malloced by upsmem_malloc.
 *
 * Input : address of memory to free
 * Output: none
 * Return: none
 */
void upsmem_free(void *a_data)
{
  void *data_ptr;

  if (a_data != 0) {
    /* check the reference counter. if it is <= 0, then free the data and 
       the memory header, else do nothing */
    if ( upsmem_get_refctr(a_data) <= 0) {
      /* okay, free it */
      data_ptr = UPSMEM_GET_TOP(a_data);
      free(data_ptr);
      a_data = 0;
    }
  }
}

/*-----------------------------------------------------------------------
 * upsmem_get_refctr
 *
 * Return the reference counter associated with the passed data
 *
 * Input : address of data whose reference counter is to be returned
 * Output: none
 * Return: reference counter
 */
int upsmem_get_refctr(const void * const a_data)
{
  int *int_ptr;
  int counter = 0;

  if (a_data != 0) {
    int_ptr = (int *)UPSMEM_GET_TOP(a_data);
    counter = *int_ptr;
  }
  return(counter);
}

/*-----------------------------------------------------------------------
 * upsmem_inc_refctr
 *
 * Increment the reference counter associated with the passed data
 *
 * Input : address of data whose reference counter is to be incremented
 * Output: none
 * Return: none
 */
void upsmem_inc_refctr(const void * const a_data)
{
  int *int_ptr;

  if (a_data != 0) {
    /* increment the reference counter. */
    int_ptr = (int *)UPSMEM_GET_TOP(a_data);
    ++(*int_ptr);
  }
}

/*-----------------------------------------------------------------------
 * upsmem_dec_refctr
 *
 * Decrement the reference counter associated with the passed data
 *
 * Input : address of data whose reference counter is to be decremented
 * Output: none
 * Return: none
 */
void upsmem_dec_refctr(const void * const a_data)
{
  int *int_ptr;

  if (a_data != 0) {
    /* increment the reference counter. */
    int_ptr = (int *)UPSMEM_GET_TOP(a_data);
    --(*int_ptr);
  }
}

