#ifndef MYPRODUCER
#define MYPRODUCER

#include "queue.h"
#include <pthread.h>

extern int G_nrow, G_ncol, G_sz;
extern int *G_min, *G_max;
extern int *G_arr_producer, *G_rnk_producer, *G_tkn_producer;
extern int *G_arr_consumer, *G_rnk_consumer;

extern struct queue_t *G_producer_threads_queue;
extern struct queue_t *G_producer2consumer_queue;
extern struct queue_t *G_consumer2producer_queue;

struct producer_param_t {
  int i;
  int seed;
};

extern struct producer_param_t G_producer_params[];

void* generate_table(void*);

#endif
