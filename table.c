#include <limits.h>
#include "common.h"
#include "table.h"

/**
 * Primitives
 **/

void table_set_cr(int c, int r, struct table_t *t, int val)
{ t->t[r*t->c + c] = val; }

int table_get_cr(int c, int r, struct table_t *t)
{ return t->t[r*t->c + c]; }

void table_set_all(struct table_t *t, int val)
{ memset(t->t, val, sizeof(int[t->sz])); }

struct table_t *table_init(int ncol, int nrow)
{
  PANIKON(((ncol <= 0) || (nrow <= 0)), "dimension are negative.");
  struct table_t *t; 
  MALLOC(t, sizeof(struct table_t));
  MALLOC(t->t, sizeof(int[nrow][ncol]));
  
  t->sz = ncol*nrow; t->c = ncol; t->r = nrow;
  return t;
}

void table_destroy(struct table_t *t)
{ free(t->t); free(t); }

/** 
 * Operations 
 **/

int __rank_cmp__(void const *a, void const *b)
{ 
  int *n = *(int**) a;
  int *m = *(int**) b;
  return n[0] - m[0]; 
}

void
table_linked_rank(struct table_t *t)
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

  for( int i=0; i < t->sz; i++)
  {
    t->t[i] = arr[i][1];
  }

  free(tuple);
  free(arr);
}

int table_fingerprint(struct table_t *t)
{
  int fingerprint = 0;
  for (int i=0, k=0; i < t->c; i++)
  {
    for (int j=0; j < t->r; j++,k++)
    {
      fingerprint = fingerprint * 10 + t->t[k];
    }
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

/**
 * List
 **/

struct table_list_t {
  int capacity;
  int count;
  struct table_t **list;
};

struct table_list_t *table_list_init(struct table_t *t)
{
  struct table_list_t *tl;
  MALLOC(tl, sizeof(*tl));

  *tl = (struct table_list_t) {
    .capacity = 20,
    .count = 1
  };

  MALLOC(tl->list, sizeof(struct table_t *[tl->capacity]));
  tl->list[0] = t;

  return tl;
}

void table_list_destroy(struct table_list_t *tl)
{
  while( --(tl->count) >= 0 )
  {
    table_destroy(tl->list[tl->count]);
  }
  free(tl->list);
  free(tl);
}

int table_list_get_size(struct table_list_t *tl)
{
  return tl->count;
}

int table_list_find(struct table_list_t *tl, struct table_t *t)
{
  for( int i=tl->count-1; i >= 0; i-- )
  {
    if( table_equal(tl->list[i], t) )  return 1;
  }

  return 0;
}

void table_list_append(struct table_list_t *tl, struct table_t *t)
{
  if( tl->count >= tl->capacity )
  {
    tl->capacity += 20;
    REALLOC(tl->list, tl->capacity * sizeof(*(tl->list)));
  }

  tl->list[tl->count] = t;
  tl->count += 1;
}

struct table_t *table_list_at(struct table_list_t *tl, int i)
{
  return tl->list[i];
}

/**
 * AVL
 **/

struct avl_node_t { 
  int key;
  int depth;

  struct table_list_t *tables;
  struct avl_node_t *left, *right;
};

struct avl_node_t *
avl_init(int key, struct table_t *table)
{
  struct avl_node_t *node;
  MALLOC(node, sizeof(*node));

  *node = (struct avl_node_t) {
    .key = key,
    .depth = 0,
    .left = NULL,
    .right = NULL
  };

  node->tables = table_list_init(table);
  
  return node;
}

void 
avl_insert(struct avl_node_t **node, int key, struct table_t *table)
{
  if( (*node)==NULL )
  {
    *node = avl_init(key, table);
    return;
  }

  int diff = key - (*node)->key;
  if( diff == 0 )
  {
    table_list_append((*node)->tables, table);
  }
  else if( diff > 0 )
  {
    avl_insert(&(*node)->right, key, table);
  }
  else
  {
    avl_insert(&(*node)->left, key, table);
  }

  avl_rebalance(node);
}

void
avl_update_depth(struct avl_node_t *node)
{
  node->depth = 0;
  if( node->left )
  { node->depth = node->left->depth; }
  
  if( node->right )
  {
    if( node->right->depth > node->depth )
    { node->depth = node->right->depth; }
  }

  node->depth += 1;
}

void
avl_rotate_left(struct avl_node_t **node)
/*
 * N
 *  \        M
 *   M ->   / \
 *  /      N   L
 * L
 * */
{
  struct avl_node_t *newcenter = (*node)->right;

  (*node)->right = (*node)->right->left;
  newcenter->left = *node;
  avl_update_depth(*node);
  *node = newcenter;
  avl_update_depth(*node);
}

void 
avl_rotate_right(struct avl_node_t **node)
/*
 *     N
 *    /      M
 *   M ->   / \
 *  /      N   L
 * L
 * */
{
  struct avl_node_t *newcenter = (*node)->left;

  (*node)->right = (*node)->left->right;
  newcenter->right = *node;
  avl_update_depth(*node);
  *node = newcenter;
  avl_update_depth(*node);
}

int 
avl_balance_factor(struct avl_node_t *node)
{
  int balance_factor = 0;
  if( node->left ) 
  { balance_factor = node->left->depth; }
  if( node->right ) 
  { balance_factor = node->right->depth; }
  return balance_factor;
}

void 
avl_rebalance(struct avl_node_t **node)
{
  int balance_factor = avl_balance_factor(*node);

  if( balance_factor == 2 )
  {
    if( avl_balance_factor((*node)->left) < 0 )
    {
      avl_rotate_left(&(*node)->left);
    }
    avl_rotate_right(node);
  }
  else if( balance_factor == -2 )
  {
    if( avl_balance_factor((*node)->right) > 0 )
    {
      avl_rotate_right(&(*node)->right);
    }
    avl_rotate_left(node);
  }
}

void 
avl_destroy(struct avl_node_t **node)
{
  if( node==NULL ) return;

  if( (*node)->left ) 
  { avl_destroy(&((*node)->left)); }
  if( (*node)->right ) 
  { avl_destroy(&((*node)->right)); }

  table_list_destroy((*node)->tables);
  free(*node);
}

struct avl_node_t *
avl_search_key(struct avl_node_t *node, int key)
{
  if (!node) return NULL;

  int diff = key - node->key;
  if (diff == 0)
  { return node; }
  else if (diff > 0)
  { return avl_search_key(node->right, key); }
  else
  { return avl_search_key(node->left, key); }
}

struct avl_node_t *
avl_search(struct avl_node_t *node, struct table_t *table)
{
  int key = table_fingerprint(table);

  return avl_search_key(node, key);
}

void
avl_print(struct avl_node_t *node)
{
  if (!node) return;

  avl_print(node->left);
  avl_print(node->right);
  
  fprintf(stderr, "key: %i\t table: ", node->key);
  int count = table_list_get_size(node->tables);
  while (--count >= 0)
  {
    struct table_t *t = table_list_at(node->tables, count);
    PRINTARR(stderr, t->t, 0, t->sz);
  }
}

void
avl_from_file(struct avl_node_t **root, int ncol, int nrow)
{
  char filename[50];
  snprintf(filename, 49, BANNEDFMT, ncol, nrow);
  FILE *f = fopen(filename, "r");
  //PANIKON(f==NULL, "Cannot open '%s'", filename);
  if( f==NULL ) return;

  char line[100];
  while( fgets(line, sizeof(line), f) != NULL )
  {
    char *ite = line;
    int arr[100];
    int shift;
    
    int i=0;
    while( sscanf(ite, "%i%n", &arr[i], &shift) == 1 )
    {
      i++;
      ite += shift;
    }

    struct table_t *table = table_init(ncol, nrow);
    memcpy(table->t, arr, sizeof(int[table->sz]));
    
    avl_insert(root, table_fingerprint(table), table);
  }

  fclose(f);
}

struct avl_node_t *
avl_init_from_banned(int ncol, int nrow)
/* Load banned subtables that fit in the table of 
 * size ncolxnrow.
 * */
{
  struct avl_node_t *root = NULL;

  do {
    do {
      avl_from_file(&root, ncol, nrow);
    } while(--nrow >= 3);
  } while(--ncol >= 3);

  return root;
}

/**
 * Properties
 **/
int
table_is_banned(struct avl_node_t *root, struct table_t *t)
{
  struct table_t *r = table_init(t->c, t->r);
  memcpy(r->t, t->t, sizeof(int[t->sz]));

  table_linked_rank(r);

  int retval = 0;
  struct avl_node_t *n = avl_search(root, r);
  if (n != NULL)
  { 
    if (table_list_find(n->tables, r))
    { retval=1; goto _exit; }
  }

_exit:;
  table_destroy(r);
  return retval;
}

int
table_has_banned_subtable(struct avl_node_t *root, struct table_t *t)
{
  if( t->c <= 3 && t->r <= 3) return 0;

  for(int c=3; c <= t->c; c++)
  {
    for(int r=3; r <= t->r; r++)
    {
      if( table_has_banned_subrank_of_dim(root, c, r, t) )
      { return 1; }
    }
  }
  return 0;
}

int
table_has_banned_subrank_of_dim(struct avl_node_t *root, int ncol, int nrow, struct table_t *t)
{
  if( ncol > t->c || nrow > t->r ) return 0;

  struct table_t *s = table_init(ncol, nrow);

  /* Each column for the submatrix must satisfy
   * 1 <= column0 <= ncol
   * 2 <= column1 <= ncol + 1
   * ...
   * ncol <= columnK <= ncol + k <= nrow-ncol
   * We let fix the first K-1 column and run out all posibilities for K.
   * The procedure is repeated for K-2, ..., 1.
   *
   * Of course, the same applies for rows */
  int *permcol; CALLOC(permcol, s->c, sizeof(int));
  int *permrow; CALLOC(permrow, s->r, sizeof(int));

  int i = 0;
  int retval = 0;
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
          for( int c=0; c < s->c; c++ )
          {
            for( int r=0; r < s->r; r++ )
            {
              s->t[r*(s->c) + c] = t->t[permrow[r]*(t->c) + permcol[c]];
            }
          }

          if( table_is_banned(root, s) )
          {
            retval = 1;
            goto _exit;
          }
        }

        if( j >= s->r || permrow[j] > t->r - s->r + j )
        {
          if( --j >= 0 ) permrow[j] += 1;
        }
        else
        {
          if( ++j < s->r ) permrow[j] = permrow[j-1] + 1;
        }
      }
    }

    if( i >= s->c || permcol[i] > t->c - s->c + i )
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
