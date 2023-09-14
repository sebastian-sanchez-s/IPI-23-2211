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
 * @file producer.h
 * @author Sebastián Sánchez 
 * @date Aug 2032
 * @brief Compute tables and sent to consumer those that do not have banned
 * subtables.
 */
#pragma once

#include "common.h"
#include "consumer.h"
#include "queue.h"
#include "table.h"

extern int G_nrow, G_ncol, G_sz;
extern int *G_min, *G_max;
extern int *G_arr, *G_tkn;
extern struct consumer_data_t G_consumer_data[];

extern struct queue_t *G_producer_queue;
extern struct queue_t *G_consumer_queue;

extern struct pair_list_t *G_banned_tables;

struct producer_param_t { int i, pos; };
extern struct producer_param_t G_producer_params[];
void* generate_table(void*);
