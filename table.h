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
void table_normalize(struct table_t *t, struct table_t *);
void table_linked_rank(struct table_t *t, struct table_t *);
int table_fingerprint(struct table_t *t);

/** List for tables **/
struct table_list_t;

struct table_list_t *table_list(struct table_t *t);
void table_list_destroy(struct table_list_t *tl);
int table_list_find(struct table_list_t *tl, struct table_t *t);
void table_list_append(struct table_list_t *tl, struct table_t *t);
struct table_t *table_list_at(struct table_list_t *tl, int i);

struct pair_list_t;

struct pair_list_t * pair_list_init(int size);
void pair_list_sort(struct pair_list_t *pl);
struct table_list_t* pair_list_find(struct pair_list_t *pl, int key);
void pair_list_append(struct pair_list_t *pl, int key, struct table_t *t);
void pair_list_destroy(struct pair_list_t *);

void load_banned_from_file(struct pair_list_t *pl, int ncol, int nrow);
struct pair_list_t *load_banned_subtables(int ncol, int nrow);

/** Properties **/
int table_is_banned(struct pair_list_t *, struct table_t *t);
int table_find_banned_subtable(struct pair_list_t *, struct table_t *t);
int table_find_banned_subrank_of_dim(struct pair_list_t *, int ncol, int nrow, struct table_t *t);
int table_equal(struct table_t *t1, struct table_t *t2);
