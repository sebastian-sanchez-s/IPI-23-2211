/****************************************************************************
 * Copyright (C) 2023 by Sebastián Sánchez                                                      
 *                                                                          
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

/**
 * @file table.c
 * @author Sebastián Sánchez 
 * @date Aug 2032
 * @brief Table object, along with auxiliary structures and its operations.
 *
 * A table has columns and rows. Here are implemented: (1) the primitive
 * funcionality such as initialization and deletion; (2) Operations such
 * as getting a subtable, normalizing, computing its fingerprint and
 * its linked rank; (3) list for table and pairs (key,tables); (4)
 * properties of tables such as being banned, having banned subtables, being
 * equal to other tables.
 */

#include "common.h"
#include "table.h"

//
// Primitives
//

/** Returns a table with ncol columsn and nrow rows. */
struct table_t *table_init(int ncol, int nrow)
{
  PANIKON(((ncol <= 0) || (nrow <= 0)), "dimension are nonpositive.");

  struct table_t *t; 
  MALLOC(t, sizeof(struct table_t));
  MALLOC(t->t, sizeof(int[nrow*ncol]));
  
  t->sz = ncol*nrow; t->c = ncol; t->r = nrow;
  return t;
}

void table_destroy(struct table_t *t)
{ free(t->t); free(t); }

//
// Operations
//
void load_banned_from_file(struct pair_list_t *pl, int ncol, int nrow);

static int __rank_cmp__(void const *a, void const *b)
{ 
  int *n = *(int**) a;
  int *m = *(int**) b;
  return n[0] - m[0]; 
}

static int __int_cmp__(void const *a, void const *b)
{
  return (*(int*)a - *(int*)b);
}

void
table_normalize(struct table_t *t, struct table_t *n)
/* Replace with relative order.
 *  5 4 1     3 2 0
 *  9 8 2  -> 5 4 1
 * */
{
  int *arr; MALLOC(arr, sizeof(int[t->sz]));
  memcpy(arr, t->t, sizeof(int[t->sz]));

  qsort(arr, t->sz, sizeof(int), __int_cmp__);

  struct table_t *o = (n==NULL) ? t: n;
  for (int i=0; i<o->sz; i++)
  {
    int *irank = bsearch(&t->t[i], arr, t->sz, sizeof(int), __int_cmp__);
    o->t[i] = irank - arr;
  }

  free(arr);
}

void
table_linked_rank(struct table_t *t, struct table_t *lrank)
/* Computes rank for a table, replacing each 
 * index i with the location of the ith ranked
 * element. 
 *
 * e.g.
 *  3 5    -> 2 0
 *  2 9    -> 1 3
 **/
{
  int **arr, *tuple; 

  MALLOC(arr, sizeof(int*[t->sz]));
  MALLOC(tuple, sizeof(int[t->sz][2]));

  for( int i=0; i<t->sz; i++)
  {
    arr[i] = tuple + 2*i;
    arr[i][0] = t->t[i];
    arr[i][1] = i;
  }

  qsort(arr, t->sz, sizeof(int*), __rank_cmp__);

  struct table_t *lr = (lrank == NULL) ? t: lrank;

  for( int i=0; i < lr->sz; i++)
  {
    lr->t[i] = arr[i][1];
  }

  free(tuple);
  free(arr);
}

int
table_fingerprint(struct table_t *t)
{
  int fingerprint = t->c;
  for (int k=1; k < t->sz-1; k++)
  {
    fingerprint = (fingerprint*10 + t->t[k]) % 12582917;
  }

  return fingerprint;
}

int table_equal(struct table_t *t1, struct table_t *t2)
{
  if (t1->r != t2->r || t1->c != t2->c)
  { return 0; }

  int sz = t1->sz;
  for (int i=0; i < sz; i++)
  {
    if (t1->t[i] != t2->t[i])
    { return 0; }
  }

  return 1;
}

//
// List
//

struct table_list_t *table_list(struct table_t *t);
void table_list_destroy(struct table_list_t *tl);
int table_list_find(struct table_list_t *tl, struct table_t *t);
void table_list_append(struct table_list_t *tl, struct table_t *t);
struct table_t *table_list_at(struct table_list_t *tl, int i);

#define LIST_INITIAL_CAPACITY 10
struct table_list_t {
  int capacity;
  int count;
  struct table_t **list;
};

struct table_list_t *
table_list_init(struct table_t *t)
{
  struct table_list_t *tl;
  MALLOC(tl, sizeof(*tl));

  *tl = (struct table_list_t) {
    .capacity = LIST_INITIAL_CAPACITY,
    .count = 1
  };

  MALLOC(tl->list, sizeof(struct table_t *[tl->capacity]));
  tl->list[0] = t;

  return tl;
}

void
table_list_destroy(struct table_list_t *tl)
{
  while( --(tl->count) >= 0 )
  {
    table_destroy(tl->list[tl->count]);
  }
  free(tl->list);
  free(tl);
}

int
table_list_get_size(struct table_list_t *tl)
{
  return tl->count;
}

int 
table_list_find(struct table_list_t *tl, struct table_t *t)
{
  for( int i=tl->count-1; i >= 0; i-- )
  {
    if( table_equal(tl->list[i], t) )  return 1;
  }

  return 0;
}

void 
table_list_append(struct table_list_t *tl, struct table_t *t)
{
  if( tl->count >= tl->capacity )
  {
    tl->capacity += LIST_INITIAL_CAPACITY;
    REALLOC(tl->list, sizeof(struct table_t*[tl->capacity]));
  }

  tl->list[tl->count] = t;
  tl->count += 1;
}

struct table_t *
table_list_at(struct table_list_t *tl, int i)
{
  return tl->list[i];
}

//
// Pair List
//

void pair_list_sort(struct pair_list_t *pl);
struct table_list_t* pair_list_find(struct pair_list_t *pl, int key);
void pair_list_append(struct pair_list_t *pl, int key, struct table_t *t);

struct pair_t {
  int key;
  struct table_list_t *table_list;
};

struct pair_list_t {
  int capacity;
  int count;
  struct pair_t *pair_list;
};

static int
__pair_list_cmp__(void const *a, void const *b)
{
  struct pair_t *p1 = (struct pair_t*)a;
  struct pair_t *p2 = (struct pair_t*)b;
  return p1->key - p2->key;
}

static int
__pair_search__(void const *k, void const *i)
{
  int key = *(int*) k;
  struct pair_t *p = (struct pair_t*)i;
  return key - p->key;
}

void
pair_list_sort(struct pair_list_t *pl)
{
  qsort(pl->pair_list, pl->count, sizeof(struct pair_t), __pair_list_cmp__);
}

struct table_list_t*
pair_list_find(struct pair_list_t *pl, int key)
{
  struct pair_t *r = bsearch(&key, pl->pair_list, pl->count, sizeof(struct pair_t), __pair_search__);
  if( r == NULL ) return NULL;
  return r->table_list;
}

struct pair_list_t *
pair_list_init(int size) 
{
  struct pair_list_t *pl;

  MALLOC(pl, sizeof(*pl));
  pl->capacity = size;
  pl->count = 0;

  CALLOC(pl->pair_list, pl->capacity, sizeof(struct pair_t));

  return pl;
}

void
pair_list_destroy(struct pair_list_t *pl)
{
  while( (--(pl->count)) >= 0 )
  {
    table_list_destroy(pl->pair_list[pl->count].table_list);
  }
  free(pl->pair_list);
  free(pl);
}

void
pair_list_append(struct pair_list_t *pl, int key, struct table_t *t)
{
  if( pl->count >= pl->capacity )
  {
    pl->capacity += pl->capacity;
    REALLOC(pl->pair_list, sizeof(struct pair_t[pl->capacity]));
  }

  struct pair_t *p = (pl->pair_list + pl->count);

  p->key = key;
  if( p->table_list == NULL )
  {
    p->table_list = table_list_init(t);
    pl->pair_list[pl->count].table_list = p->table_list;
  }
  else
  {
    table_list_append(p->table_list, t);
  }

  pl->count += 1;
}

#define FILENAMEBUFF 50
#define ARRAYLENGTH 100
void
load_banned_from_file(struct pair_list_t *pl, int ncol, int nrow)
{
  char filename[FILENAMEBUFF];
  snprintf(filename, FILENAMEBUFF, BANNEDFMT, ncol, nrow);
  FILE *f = fopen(filename, "r");
  if( f==NULL ) return;

  char line[ARRAYLENGTH];
  while( fgets(line, sizeof(line), f) != NULL )
  {
    char *ite = line;
    int arr[ARRAYLENGTH];
    int shift;
    
    int i=0;
    while( sscanf(ite, "%i%n", &arr[i], &shift) == 1 )
    {
      i++;
      ite += shift;
    }

    PANIKON(ncol*nrow > i , "table size and array do not match dimensions");

    struct table_t *table = table_init(ncol, nrow);
    memcpy(table->t, arr, sizeof(int[table->sz]));
    
    pair_list_append(pl, table_fingerprint(table), table);
  }

  fclose(f);
}
#undef FILENAMEBUFF
#undef ARRAYLENGTH

struct pair_list_t*
load_banned_subtables(int ncol, int nrow)
/* Load banned subtables that fit in the table of 
 * size (ncol, nrow).
 *
 * INPUT
 * ---------
 *  ncol, nrow : # column/row of the reference table
 *
 * OUTPUT
 * ---------
 *  a sorted pair list
 * */
{
  struct pair_list_t *pl = pair_list_init(2000);

  for(int c=3; c <= ncol; c++)
  {
    for(int r=3; r <= nrow; r++)
    {
      load_banned_from_file(pl, c, r);
    }
  }

  //fprintf(stderr, "FROM FILE\n");
  //for(int i=0; i<pl->count; i++)
  //{
  //  fprintf(stderr, "KEY: %i\n", pl->pair_list[i].key);
  //  struct table_list_t *tl = pl->pair_list[i].table_list;
  //  for(int j=0; j<tl->count; j++)
  //  {
  //    PRINTARR(stderr, tl->list[j]->t, 0, tl->list[j]->sz);
  //  }
  //}
  pair_list_sort(pl);

  return pl;
}

//
// Properties
//

int
table_is_banned(struct pair_list_t *pl, struct table_t *t)
/**
 * INPUT
 * --------
 *  pl : pair list with banned subtables
 *  t  : table to examine
 *
 * OUTPUT
 * --------
 *  returns 1 is table is banned, 0 otherwise.
 * */
{
  struct table_t *r = table_init(t->c, t->r);

  table_normalize(t, r);

  int retval = 0;
  struct table_list_t *tl = pair_list_find(pl, table_fingerprint(r));
  if( tl != NULL )
  { 
    if( table_list_find(tl, r) )
    { retval=1; goto _exit; }
  }

_exit:;
  table_destroy(r);
  return retval;
}

int
table_find_banned_subtable(struct pair_list_t *pl, struct table_t *t)
/**
 * INPUT
 * --------
 *  - pl   : a pair list that contains banned tables and its fingerprints.
 *  - table: table to examine
 *
 * OUPUT
 * --------
 *  index (bottom right) of the first banned subtable identified.
 *  -1 if no subtable is found.
 * */
{
  if( pl==NULL ) return -1;
  if( t->c <= 3 && t->r <= 3 ) return -1;

  for(int c=3; c <= t->c; c++)
  {
    for(int r=3; r <= t->r; r++)
    {
      int i = table_find_banned_subrank_of_dim(pl, c, r, t);
      if( i >= 0 )
      { return i; }
    }
  }
  return -1;
}

int
table_find_banned_subrank_of_dim(struct pair_list_t *pl, int ncol, int nrow, struct table_t *t)
/**
 * ALGORITHM
 * ---------
 * Each column for the submatrix must satisfy
 * 1 <= column0 <= ncol
 * 2 <= column1 <= ncol + 1
 *        ...
 * ncol <= columnK <= ncol + k <= nrow-ncol
 * We let fix the first K-1 column and run out all posibilities for K.
 * The procedure is repeated for K-2, ..., 1.
 *
 * Of course, the same applies for rows 
 *
 * INPUT
 * --------
 * - ncol, nrow: dimension of the subtable to look for.
 * - table: table to examine
 *
 * OUTPUT
 * --------
 *  index (bottom right) of the first banned subtable identified.
 *  -1 if no subtable is found.
 * */
{
  int retval = -1; 

  if( pl==NULL || ncol > t->c || nrow > t->r ) return retval;

  struct table_t *s = table_init(ncol, nrow);

  int *permcol; CALLOC(permcol, s->c, sizeof(int));
  int *permrow; CALLOC(permrow, s->r, sizeof(int));

  //
  // Permute columns and rows
  //
  int i = 0;
  while( i >= 0 )
  {
    if( i == s->c )
    {
      int j = 0;
      memset(permrow, 0, sizeof(int[s->r]));
      while( j >= 0 )
      {
        if( j == s->r )
        {
          /* Get subtable */
          for(int c=0; c < s->c; c++)
          {
            for(int r=0; r < s->r; r++)
            {
              s->t[r*(s->c) + c] = t->t[permrow[r]*(t->c) + permcol[c]];
            }
          }

          if( table_is_banned(pl, s) )
          {
            retval = permrow[(s->r)-1]*(t->c) + permcol[(s->c)-1];
            goto _exit;
          }
        }

        if( j >= s->r || permrow[j] > (t->r - s->r + j) )
        {
          if( --j >= 0 ) permrow[j] += 1;
        }
        else
        {
          if( ++j < s->r ) permrow[j] = permrow[j-1] + 1;
        }
      }
    }

    if( i >= s->c || permcol[i] > (t->c - s->c + i) )
    {
      if( --i >= 0 ) permcol[i] +=  1;
    }
    else
    {
      if( ++i < s->c ) permcol[i] = permcol[i-1] + 1;
    }
  }

_exit:;
  table_destroy(s);
  free(permcol); 
  free(permrow);
  return retval;
}
