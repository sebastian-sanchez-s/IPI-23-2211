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
 * @file table.h
 * @author Sebastián Sánchez 
 * @date Aug 2032
 * @brief Table object interface.
 *
 * A table is use for computing feasible standard young tableaux.
 * Here are the public methods.
 */
#pragma once

#include "common.h"

// Primitives
struct table_t {
  int c, r, sz;
  int *t;
};
struct table_t *table_init(int ncol, int nrow);
void table_destroy(struct table_t *t);

// Pair List
struct pair_list_t;
struct pair_list_t * pair_list_init(int size);
void pair_list_destroy(struct pair_list_t *);

// Operations
void table_subtable(struct table_t *t, struct table_t *s, int bcol, int brow);
void table_normalize(struct table_t *t, struct table_t *);
void table_linked_rank(struct table_t *t, struct table_t *);
int table_fingerprint(struct table_t *t);
struct pair_list_t *load_banned_subtables(int ncol, int nrow);

// Properties
int table_is_banned(struct pair_list_t *, struct table_t *t);
int table_find_banned_subtable(struct pair_list_t *, struct table_t *t);
int table_find_banned_subrank_of_dim(struct pair_list_t *, int ncol, int nrow, struct table_t *t);
int table_equal(struct table_t *t1, struct table_t *t2);
