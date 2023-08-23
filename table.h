#pragma once

#include "common.h"

struct table_t {
  int c, r, sz;
  int *t;
};

/** Primitives **/
struct table_t *table_init(int ncol, int nrow);
int  table_get_cr(int c, int r, struct table_t *t);
void table_set_cr(int c, int r, struct table_t *t, int val);
void table_set_all(struct table_t *t, int val);
void table_destroy(struct table_t *t);

/** Operations **/
void table_subtable(struct table_t *t, struct table_t *s, int bcol, int brow);
void table_normalize(struct table_t *t);
void table_linked_rank(struct table_t *t);
int table_fingerprint(struct table_t *t);

/** List for tables **/
struct table_list_t;

struct table_list_t *table_list(struct table_t *t);
void table_list_destroy(struct table_list_t *tl);
int table_list_find(struct table_list_t *tl, struct table_t *t);
void table_list_append(struct table_list_t *tl, struct table_t *t);
struct table_t *table_list_at(struct table_list_t *tl, int i);


/** AVL tree for tables **/
struct avl_node_t;

struct avl_node_t *avl_init(int key, struct table_t *table);
void avl_insert(struct avl_node_t **node, int key, struct table_t *table);
void avl_update_depth(struct avl_node_t *node);
void avl_rotate_left(struct avl_node_t **node);
void avl_rotate_right(struct avl_node_t **node);
int avl_balance_factor(struct avl_node_t *node);
void avl_rebalance(struct avl_node_t **node);
void avl_destroy(struct avl_node_t **node);
struct avl_node_t *avl_search_key(struct avl_node_t *node, int key);
struct avl_node_t *avl_search(struct avl_node_t *node, struct table_t *table);
void avl_print(struct avl_node_t *node);
struct avl_node_t *avl_init_from_banned(int ncol, int nrow);
void avl_from_file(struct avl_node_t **root, int ncol, int nrow);

/** Properties **/
int table_is_banned(struct avl_node_t *root, struct table_t *t);
int table_has_banned_subtable(struct avl_node_t *root, struct table_t *t);
int table_has_banned_subrank_of_dim(struct avl_node_t *root, int ncol, int nrow, struct table_t *t);
int table_equal(struct table_t *t1, struct table_t *t2);
