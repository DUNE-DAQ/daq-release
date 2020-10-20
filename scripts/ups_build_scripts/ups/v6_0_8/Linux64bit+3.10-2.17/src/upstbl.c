/************************************************************************
 *
 * FILE:
 *       upstbl.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>

#include "upstbl.h"
#include "upserr.h"

/************************************************************************
 *
 * Atom implementation.
 *
 */
#define NELEMS(x) ((sizeof (x))/(sizeof ((x)[0])))

static struct atom {
  struct atom *link;
  int len;
  char *str;
} *buckets[2048];

static unsigned int scatter[] = {
  2078917053, 143302914, 1027100827, 1953210302, 755253631, 2002600785,
  1405390230, 45248011, 1099951567, 433832350, 2018585307, 438263339,
  813528929, 1703199216, 618906479, 573714703, 766270699, 275680090,
  1510320440, 1583583926, 1723401032, 1965443329, 1098183682, 1636505764,
  980071615, 1011597961, 643279273, 1315461275, 157584038, 1069844923,
  471560540, 89017443, 1213147837, 1498661368, 2042227746, 1968401469,
  1353778505, 1300134328, 2013649480, 306246424, 1733966678, 1884751139,
  744509763, 400011959, 1440466707, 1363416242, 973726663, 59253759,
  1639096332, 336563455, 1642837685, 1215013716, 154523136, 593537720,
  704035832, 1134594751, 1605135681, 1347315106, 302572379, 1762719719,
  269676381, 774132919, 1851737163, 1482824219, 125310639, 1746481261,
  1303742040, 1479089144, 899131941, 1169907872, 1785335569, 485614972,
  907175364, 382361684, 885626931, 200158423, 1745777927, 1859353594,
  259412182, 1237390611, 48433401, 1902249868, 304920680, 202956538,
  348303940, 1008956512, 1337551289, 1953439621, 208787970, 1640123668,
  1568675693, 478464352, 266772940, 1272929208, 1961288571, 392083579,
  871926821, 1117546963, 1871172724, 1771058762, 139971187, 1509024645,
  109190086, 1047146551, 1891386329, 994817018, 1247304975, 1489680608,
  706686964, 1506717157, 579587572, 755120366, 1261483377, 884508252,
  958076904, 1609787317, 1893464764, 148144545, 1415743291, 2102252735,
  1788268214, 836935336, 433233439, 2055041154, 2109864544, 247038362,
  299641085, 834307717, 1364585325, 23330161, 457882831, 1504556512,
  1532354806, 567072918, 404219416, 1276257488, 1561889936, 1651524391,
  618454448, 121093252, 1010757900, 1198042020, 876213618, 124757630,
  2082550272, 1834290522, 1734544947, 1828531389, 1982435068, 1002804590,
  1783300476, 1623219634, 1839739926, 69050267, 1530777140, 1802120822,
  316088629, 1830418225, 488944891, 1680673954, 1853748387, 946827723,
  1037746818, 1238619545, 1513900641, 1441966234, 367393385, 928306929,
  946006977, 985847834, 1049400181, 1956764878, 36406206, 1925613800,
  2081522508, 2118956479, 1612420674, 1668583807, 1800004220, 1447372094,
  523904750, 1435821048, 923108080, 216161028, 1504871315, 306401572,
  2018281851, 1820959944, 2136819798, 359743094, 1354150250, 1843084537,
  1306570817, 244413420, 934220434, 672987810, 1686379655, 1301613820,
  1601294739, 484902984, 139978006, 503211273, 294184214, 176384212,
  281341425, 228223074, 147857043, 1893762099, 1896806882, 1947861263,
  1193650546, 273227984, 1236198663, 2116758626, 489389012, 593586330,
  275676551, 360187215, 267062626, 265012701, 719930310, 1621212876,
  2108097238, 2026501127, 1865626297, 894834024, 552005290, 1404522304,
  48964196, 5816381, 1889425288, 188942202, 509027654, 36125855,
  365326415, 790369079, 264348929, 513183458, 536647531, 13672163,
  313561074, 1730298077, 286900147, 1549759737, 1699573055, 776289160,
  2143346068, 1975249606, 1136476375, 262925046, 92778659, 1856406685,
  1884137923, 53392249, 1735424165, 1602280572
};

const char *upstbl_atom_string( const char * const str ) 
{
  if ( !str ) 
    return NULL;
  return upstbl_atom_new( str, (int)strlen( str ) );
}

const char *upstbl_atom_int( const int n ) 
{
  char str[43];
  char *s = str + sizeof( str );
  unsigned int m;

  if (n == INT_MIN)
    m = INT_MAX + 1UL;
  else if (n < 0)
    m = (unsigned int)-n;
  else
    m = (unsigned int)n;
  do
    *--s = (char)(m%10 + '0');
  while ((m /= 10) > 0);
  if (n < 0)
    *--s = '-';

  return upstbl_atom_new( s, (str + sizeof( str )) - s);
}

const char *upstbl_atom_new( const char * const str, 
			     const int len ) 
{
  unsigned int h;
  int i;
  struct atom *p;

  if ( !str || (len < 0) )
    return NULL;

  for ( h = 0, i = 0; i < len; i++ )
    h = (h<<1) + scatter[(unsigned int) ((unsigned char *) str)[i]];
  h &= NELEMS(buckets)-1;

  for ( p = buckets[h]; p; p = p->link )
    if ( len == p->len ) {

      /* 'optimized' for file names: since many files have a common 
         directory path, string comparison starts from the end */

      for ( i = len-1; i >=0 && p->str[i] == str[i]; )
	i--;
      if ( i == -1 )
	return p->str;
    }

  p = malloc( sizeof (*p) + (size_t)(len + 1) );
  p->len = len;
  p->str = (char *)( p + 1 );
  if ( len > 0 )
    (void) memcpy(p->str, str, (size_t)len);
  p->str[len] = '\0';
  p->link = buckets[h];
  buckets[h] = p;

  return p->str;
}

int upstbl_atom_length( const char * const str ) 
{
  struct atom *p;
  int i;

  if ( !str )
    return 0;

  for ( i = 0; i < (int)NELEMS(buckets); i++ )
    for ( p = buckets[i]; p; p = p->link )
      if ( p->str == str )
	return p->len;

  return 0;
}

/************************************************************************
 *
 * Table implementation
 *
 */

#define CMPATOM( x, y ) ((x) != (y))
#define HASHATOM( k ) ((unsigned int)(k)>>2)

t_upstbl *upstbl_new( int hint )
{
  t_upstbl *table;
  int i;
  static unsigned int primes[] = { 509, 509, 1021, 2053, 4093,
				   8191, 16381, 32771, 65521, INT_MAX };
  unsigned int size;

  if ( hint <= 0 ) 
    hint = 0;
  for ( i = 1; primes[i] < (unsigned int)hint; i++ );
  size = primes[i-1];
  
  table = malloc( sizeof( *table ) + (size_t)(size*sizeof( table->buckets[0] )) );
  table->size = (int)size;
  table->buckets = (struct binding **)(table + 1);
  for (i = 0; i < table->size; i++)
    table->buckets[i] = NULL;
  table->length = 0;
  table->timestamp = 0;
  return table;
}

void *upstbl_get( t_upstbl * const table, const void * const key ) 
{
  int i;
  struct binding *p;

  if ( !table || !key ) 
    return NULL;

  i = (int)HASHATOM( key )%table->size;
  for ( p = table->buckets[i]; p; p = p->link )
    if ( CMPATOM( key, p->key ) == 0 )
      break;

  return p ? p->value : NULL;
}

void *upstbl_put( t_upstbl * const table, const void * const key, void * const value ) 
{
  int i;
  struct binding *p;
  void *prev;

  if ( !table || !key ) 
    return 0;

  i = (int)HASHATOM( key )%table->size;
  for ( p = table->buckets[i]; p; p = p->link )
    if ( CMPATOM( key, p->key ) == 0 )
      break;

  if ( p == NULL ) {
    p = malloc( sizeof( struct binding ) );
    p->key = key;
    p->link = table->buckets[i];
    table->buckets[i] = p;
    table->length++;
    prev = NULL;
  } 
  else
    prev = p->value;

  p->value = value;
  table->timestamp++;
  return prev;
}

int upstbl_length( t_upstbl * const table ) 
{
  if ( !table )
    return 0;
  return table->length;
}

void upstbl_map( t_upstbl * const table,
		 void apply(const void *, void **, void * ),
		 void * const cl ) 
{
  int i;
  unsigned stamp;
  struct binding *p;

  if ( !table || !apply )
    return;

  stamp = table->timestamp;
  for ( i = 0; i < table->size; i++ )
    for ( p = table->buckets[i]; p; p = p->link ) {
      apply( p->key, &p->value, cl );
      if ( table->timestamp != stamp ) {
	upserr_add( UPS_FILEMAP_CORRUPT, UPS_FATAL );
	return;
      }
    }
}

void *upstbl_remove( t_upstbl * const table, 
		     const void * const key ) 
{
  int i;
  struct binding **pp;

  if ( !table || !key )
    return NULL;

  table->timestamp++;
  i = (int)HASHATOM( key )%table->size;
  for ( pp = &table->buckets[i]; *pp; pp = &(*pp)->link )
    if ( CMPATOM( key, (*pp)->key ) == 0 ) {
      struct binding *p = *pp;
      void *value = p->value;
      *pp = p->link;
      free( p );
      table->length--;
      return value;
    }

  return NULL;
}

void **upstbl_to_array( t_upstbl * const table, void * const end ) 
{
  int i, j = 0;
  void **array;
  struct binding *p;

  if ( !table )
    return NULL;

  array = malloc( (size_t)(2*table->length + 1)*sizeof (*array) );
  for ( i = 0; i < table->size; i++ )
    for ( p = table->buckets[i]; p; p = p->link ) {
      array[j++] = (void *)p->key;
      array[j++] = p->value;
    }

  array[j] = end;
  return array;
}

void upstbl_free( t_upstbl ** const table ) 
{
  if ( !table || !*table )
    return;

  if ( (*table)->length > 0 ) {
    int i;
    struct binding *p, *q;
    for ( i = 0; i < (*table)->size; i++ )
      for ( p = (*table)->buckets[i]; p; p = q ) {
	q = p->link;
	free( p );
      }
  }
  free( *table );
}

void upstbl_trim( t_upstbl * const table ) 
{

  /* when we have a little more time ... this should be
     done a lot better */
  
  static char *keys[1024];
  static int nkeys;
  int i;

  if ( !table )
    return;

  if ( table->length > 0 ) {
    struct binding *p;
    for ( i = 0; i < table->size; i++ ) {
      for ( p = table->buckets[i]; p; p = p->link ) {
	if ( ! p->value ) {
	  keys[ nkeys ] = (char *)p->key;
	  nkeys++;
	}
      }
    }
  }
  
  for ( i=0; i<nkeys; i++ ) {
    (void) upstbl_remove( table, keys[i] );
  }

}

void upstbl_dump( t_upstbl * const table, const int iopt )
{
  int i;
  struct binding *bp;
  struct atom *ap;

  /* just print number of items */

  (void) printf( "total number of items in table = %d\n", table ? table->length : 0 );

  if ( iopt <= 0 )
    return;

  /* dump all elements in table */

  (void) printf( "\ndumping table:\n" );
  for ( i = 0; i < table->size; i++ )
    for ( bp = table->buckets[i]; bp; bp = bp->link )
      (void) printf( "%x, table bucket %d, %s\n", (unsigned int)bp->key, i, (char *)bp->key );

  if ( iopt <= 5 )
    return;

  /* dump keys */

  (void) printf( "\ndumping atoms:\n" );
  for ( i = 0; i < (int)NELEMS(buckets); i++ )
    for ( ap = buckets[i]; ap; ap = ap->link ) 
      (void) printf( "%x, key bucket %d, %s\n", (unsigned int)ap->str, i, (char *)ap->str );

  return;
}
