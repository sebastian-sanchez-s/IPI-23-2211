#ifndef MYPRODUCER
#define MYPRODUCER

#include "util.h"
#include "queue.h"

extern int G_nrow, G_ncol, G_sz;
extern int *G_min, *G_max;
extern int *G_arr, *G_rnk, *G_tkn;
extern struct consumer_data_t G_consumer_data[];

extern struct queue_t *G_producer_threads_queue;
extern struct queue_t *G_consumer2producer_queue;

struct producer_param_t {
  int i;
  int seed;
};

extern struct producer_param_t G_producer_params[];

void* generate_table(void*);

#endif
