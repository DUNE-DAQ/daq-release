/************************************************************************
 *
 * FILE:
 *       upstbl.h
 * 
 * DESCRIPTION: 
 *       An implementation of a table.
 *       Code from "C Interfaces and Implementations" by David R. Hanson.
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
 *       09-Dec-1997, LR, first.
 *
 ***********************************************************************/
#ifndef _UPSTBL_H_
#define _UPSTBL_H_

typedef struct upstbl {
  int size;
  int length;
  unsigned timestamp;
  struct binding {
    struct binding *link;
    const void *key;
    void *value;
  } **buckets;
} t_upstbl;

/* Table interface */

extern t_upstbl  *upstbl_new( int hint );
extern void      upstbl_free( t_upstbl ** const table );
extern int       upstbl_length( t_upstbl * const table );
extern void      *upstbl_put( t_upstbl * const table, 
			      const void * const key,
			      void * const value );
extern void      *upstbl_get( t_upstbl * const table, 
			      const void * const key );
extern void      *upstbl_remove( t_upstbl * const table, 
				 const void * const key);
extern void      upstbl_map( t_upstbl * const table,
			     void apply(const void *, void **, void *),
			     void * const cl );
extern void     upstbl_trim( t_upstbl * const table );
extern void     **upstbl_to_array( t_upstbl * const table, 
				   void * const end );
extern void     upstbl_dump( t_upstbl * const table, const int iopt );

/* Atom interface */

extern int         upstbl_atom_length( const char * const str );
extern const char  *upstbl_atom_new( const char * const str, const int len );
extern const char  *upstbl_atom_string( const char * const str );
extern const char  *upstbl_atom_int( const int n );

#endif /* _UPSTBL_H */
