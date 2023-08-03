#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"
#include "consumer.h"
#include "producer.h"

#define PRODUCER_THREADS 2
#define CONSUMER_THREADS 6

#define QUEUE_SIZE 10000

pthread_t G_producer[PRODUCER_THREADS];
pthread_t G_consumer[CONSUMER_THREADS];

struct producer_param_t G_producer_params[PRODUCER_THREADS];

struct queue_t *G_producer_threads_queue = NULL;
struct queue_t *G_producer2consumer_queue = NULL;
struct queue_t *G_consumer2producer_queue = NULL;

FILE *G_files_pos[CONSUMER_THREADS];
FILE *G_files_neg[CONSUMER_THREADS];

int G_nrow, G_ncol, G_sz;
int *G_min = NULL, *G_max = NULL;

int *G_arr_producer = NULL;
int *G_rnk_producer = NULL;
int *G_tkn_producer = NULL;

int *G_arr_consumer = NULL;
int *G_rnk_consumer = NULL;

#define MALLOC(ptr, size) {\
  ptr = malloc(size);\
  if (ptr==NULL) {\
    fprintf(stderr, "Error in malloc");\
    return -1;\
  }}\

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: ./<executable> <ncol> <nrow>\n");
    return -1;
  }

  //
  // Initialization
  //
  G_ncol = atoi(argv[1]);
  G_nrow = atoi(argv[2]);
  G_sz = G_ncol*G_nrow;

  //
  // Compute G_min and G_max
  //
  MALLOC(G_min, sizeof(int[G_sz]));
  MALLOC(G_max, sizeof(int[G_sz]));

  for (int r = 0, i = 0; r < G_nrow; r++)
  {
    for (int c = 0; c < G_ncol; c++, i++)
    {
      G_min[i] = (r+1) * (c+1);
      G_max[i] = G_sz - (G_nrow-r)*(G_ncol-c) + 1;
    }
  }

  //
  // Allocate enough memory at once
  //
  MALLOC(G_arr_producer, sizeof(int[PRODUCER_THREADS][G_sz]));
  MALLOC(G_rnk_producer, sizeof(int[PRODUCER_THREADS][G_sz+1]));
  MALLOC(G_tkn_producer, sizeof(int[PRODUCER_THREADS][G_sz+1]));

  MALLOC(G_arr_consumer, sizeof(int[CONSUMER_THREADS][G_sz]));
  MALLOC(G_rnk_consumer, sizeof(int[CONSUMER_THREADS][G_sz+1]));

  G_producer_threads_queue  = queue_init(QUEUE_SIZE);
  G_producer2consumer_queue = queue_init(QUEUE_SIZE);
  G_consumer2producer_queue = queue_init(QUEUE_SIZE);

  //
  // Set up queues, files and auxiliary data
  //
  for (int i=0; i<CONSUMER_THREADS; i++)
  {
    queue_put(G_consumer2producer_queue, i); 

    char filename[50];
    sprintf(filename, "raw/Pm%in%it%i", G_ncol, G_nrow, i);
    G_files_pos[i] = fopen(filename, "w");
    sprintf(filename, "raw/Nm%in%it%i", G_ncol, G_nrow, i);
    G_files_neg[i] = fopen(filename, "w");

    pthread_create(&G_consumer[i],
                    NULL,
                    solve_LP,
                    NULL);
  }

  for (int i=0; i<PRODUCER_THREADS; i++)
  {
    queue_put(G_producer_threads_queue, i);
  }

  //
  // Start Producers
  //
  int seed=2;
  while(seed < G_nrow+2)
  {
    int i = queue_get(G_producer_threads_queue);

    G_producer_params[i] = (struct producer_param_t) {
      .i = i,
      .seed = seed,
    };

    pthread_create(&G_producer[i],
                   NULL,
                   generate_table,
                   &G_producer_params[i]);

    seed++;
  }

  for (int i=0; i<PRODUCER_THREADS; i++)
  {
    pthread_join(G_producer[i], NULL);
  }

  for (int i=0; i<CONSUMER_THREADS; i++)
  { queue_put(G_producer2consumer_queue, -1); }
  for (int i=0; i<CONSUMER_THREADS; i++)
  { pthread_join(G_consumer[i], NULL); }

  free(G_min);
  free(G_max);

  free(G_arr_producer);
  free(G_rnk_producer);
  free(G_tkn_producer);
  free(G_arr_consumer);
  free(G_rnk_consumer);

  queue_destroy(G_producer_threads_queue);
  queue_destroy(G_producer2consumer_queue);
  queue_destroy(G_consumer2producer_queue);

  return 0;
}
