#ifndef MYCONSUMER
#define MYCONSUMER

extern int G_nrow, G_ncol, G_sz;
extern int *G_arr_consumer, *G_rnk_consumer;

extern FILE *G_files_pos[];
extern FILE *G_files_neg[];

extern struct queue_t *G_consumer2producer_queue;
extern struct queue_t *G_producer2consumer_queue;

void* solve_LP(void *);

#endif
