#include <limits.h>
#include "common.h"
#include "sort_search_utils.h"
#include "table.h"

/**
 * Primitives
 **/

void table_set_cr(int c, int r, struct table_t *t, int val)
{ t->t[r*t->c + c] = val; }

int table_get_cr(int c, int r, struct table_t *t)
{ return t->t[r*t->c + c]; }

void table_set_all(struct table_t *t, int val)
{ for (int i=0; i<t->sz; i++) { t->t[i] = val; } }

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

void table_subtable(
    struct table_t *t, 
    struct table_t *s, 
    int bcol, int brow)
/* Save subtable of dim s starting at (bcol,brow) in t into s
 * 1 2 3 4   subtable      3 4
 * 5 6 7 8 - 2x2 at 2,0 -> 7 8
 */
{
  PANIKON(s->c > t->c || s->r > t->r, "subtable does not fit.");

  for (int c=0; c < s->c; c++)
  {
    for (int r=0; r < s->r; r++)
    {
      s->t[r*s->c + c] = t->t[(r+brow)*t->c + c+bcol];
    }
  }
}

int __cmp__(void const *a, void const *b)
{ return (*(int*)a) - (*(int*)b); }
void table_normalize(struct table_t *t)
/* Replace with relative order.
 *  5 4 1     3 2 0
 *  9 8 2  -> 5 4 1
 * */
{
  int *relorder = i_sorted(t->sz, t->t);
  
  for (int i=0; i<t->sz; i++)
  {
    int *irank = bsearch(&t->t[i], relorder, t->sz, sizeof(int), __cmp__);
    t->t[i] = *irank;
  }

  free(relorder);
}

int table_fingerprint(struct table_t *t)
{
  int fingerprint = 0;
  for (int i=0, k=0; i < t->c; i++)
  {
    for (int j=0; j < t->r; j++,k++)
    {
      fingerprint = (fingerprint * 11 + t->t[k]) % 12289;
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
  struct table_t *list;
};

struct table_list_t *table_list_init(struct table_t *t)
{
  struct table_list_t *tl;
  MALLOC(tl, sizeof(*tl));

  *tl = (struct table_list_t) {
    .capacity = 20,
    .count = 1
  };

  MALLOC(tl->list, sizeof(struct table_t[tl->capacity]));
  tl->list[0] = *t;

  return tl;
}

void table_list_destroy(struct table_list_t *tl)
{
  while (--(tl->count) >= 0)
  {
    free(tl->list[tl->count].t);
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
  for (int i=tl->count; i >= 0; --i)
  {
    if (table_equal(&tl->list[i], t))  return 1;
  }

  return 0;
}

void table_list_append(struct table_list_t *tl, struct table_t *t)
{
  if (tl->count >= tl->capacity)
  {
    tl->capacity += 20;
    REALLOC(tl->list, sizeof(struct table_t[tl->capacity]));
  }

  tl->list[tl->count] = *t;
  tl->count += 1;
}

struct table_t *table_list_at(struct table_list_t *tl, int i)
{
  return &tl->list[i];
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

struct avl_node_t *avl_init(int key, struct table_t *table)
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

void avl_insert(struct avl_node_t **node, int key, struct table_t *table)
{
  if (!(*node))
  {
    *node = avl_init(key, table);
    return;
  }

  int diff = key - (*node)->key; //
  if (diff == 0)
  {
    table_list_append((*node)->tables, table);
  }
  else if (diff > 0)
  {
    avl_insert(&(*node)->right, key, table);
  }
  else
  {
    avl_insert(&(*node)->left, key, table);
  }

  avl_rebalance(node);
}

void avl_update_depth(struct avl_node_t *node)
{
  node->depth = 0;
  if (node->left)
  { node->depth = node->left->depth; }
  
  if (node->right)
  {
    if (node->right->depth > node->depth)
    { node->depth = node->right->depth; }
  }

  node->depth += 1;
}

void avl_rotate_left(struct avl_node_t **node)
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

void avl_rotate_right(struct avl_node_t **node)
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

int avl_balance_factor(struct avl_node_t *node)
{
  int balance_factor = 0;
  if (node->left) balance_factor = node->left->depth;
  if (node->right) balance_factor = node->right->depth;
  return balance_factor;
}

void avl_rebalance(struct avl_node_t **node)
{
  int balance_factor = avl_balance_factor(*node);

  if (balance_factor == 2)
  {
    if (avl_balance_factor((*node)->left) < 0)
    {
      avl_rotate_left(&(*node)->left);
    }
    avl_rotate_right(node);
  }
  else if (balance_factor == -2)
  {
    if (avl_balance_factor((*node)->right) > 0)
    {
      avl_rotate_right(&(*node)->right);
    }
    avl_rotate_left(node);
  }
}

void avl_destroy(struct avl_node_t **node)
{
  if (!node) return;

  if ((*node)->left) avl_destroy(&((*node)->left));
  if ((*node)->right) avl_destroy(&((*node)->right));

  table_list_destroy((*node)->tables);
  free(*node);
}

struct avl_node_t *avl_search_key(struct avl_node_t *node, int key)
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

struct avl_node_t *avl_search(struct avl_node_t *node, struct table_t *table)
{
  int key = table_fingerprint(table);

  return avl_search_key(node, key);
}

void avl_print(struct avl_node_t *node)
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

struct avl_node_t *avl_from_file(int ncol, int nrow)
{
  char filename[50];
  snprintf(filename, 49, NFMT, ncol, nrow);
  FILE *f = fopen(filename, "r");
  PANIKON(f==NULL, "Error while attempting to read '%s'", filename);
  
  struct avl_node_t *root = NULL;

  char line[512];
  while (fgets(line, sizeof(line), f) != NULL)
  {
    char *ite = line;
    int arr[512];
    int shift;
    
    int i=0;
    while (sscanf(ite, "%i%n", &arr[i], &shift) == 1)
    {
      i++;
      ite += shift;
    }

    struct table_t *table = table_init(ncol, nrow);
    memcpy(table->t, arr, i * sizeof(int));
    
    avl_insert(&root, table_fingerprint(table), table);
  }

  fclose(f);

  return root;
}

/**
 * Properties
 **/
int table_is_banned(struct avl_node_t *tree, struct table_t *t)
{
  struct table_t *r = table_init(t->c, t->r);
  for(int i=0; i<t->sz; i++)
  { r->t[i] = t->t[i]; }

  table_normalize(r);

  int retval = 0;

  struct avl_node_t *n = avl_search(tree, r);
  if (n != NULL)
  { 
    if (table_list_find(n->tables, r))
    {
      retval = 1;
      goto _free_and_leave;
    }
  }

_free_and_leave:;
  table_destroy(r);
  return retval;
}

int table_has_banned_subrank_of_dim(
    struct avl_node_t *tree,
    int ncol, int nrow, 
    struct table_t *t)
{
  if (ncol > t->c || nrow > t->r)
  {
    return 0;
  }

  struct table_t *s = table_init(ncol, nrow);

  int retval = 0;
  for (int c=0; c <= (t->c - ncol); c++)
  {
    for(int r=0; r <= (t->r - nrow); r++)
    {
      table_subtable(t, s, c, r);
      if( table_is_banned(tree, s) )
      {
        fprintf(stderr, "banned subtable at (%i,%i)\n", c, r);
        retval=1; 
        goto _free_and_leave;
      }
    }
  }

_free_and_leave:;
  table_destroy(s);
  return retval;
}

