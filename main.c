#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "util.h"
#include "queue.h"
#include "producer.h"

#define NUM_PRODUCER 1
#define NUM_CONSUMER 1 

#define QUEUE_SIZE 10

pthread_t G_producer[NUM_PRODUCER];
struct consumer_data_t {
  int i; // index in this array
  int p2c[2]; // parent to child pipe
  int c2p[2]; // child to parent pipe
  pthread_t listener;
} G_consumer_data[NUM_CONSUMER];

struct producer_param_t G_producer_params[NUM_PRODUCER];

struct queue_t *G_producer_threads_queue = NULL;
struct queue_t *G_consumer2producer_queue = NULL;

int G_nrow, G_ncol, G_sz;
int *G_min = NULL, *G_max = NULL;

int *G_arr_producer = NULL;
int *G_rnk_producer = NULL;
int *G_tkn_producer = NULL;

static int launch_consumer(int i);

int main(int argc, char* argv[])
{
  PANIKON(argc < 3, "Usage: ./<executable> <ncol> <nrow>\n");

  for (int i=0; i<NUM_CONSUMER; i++)
  { launch_consumer(i); }

  for (int i=0; i<NUM_CONSUMER; i++)
  {
    pthread_create(&G_consumer_data[i].listener,
                    NULL, 
                    listen_consumer, 
                    &G_consumer_data[i].i);
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

  G_producer_threads_queue  = queue_init(QUEUE_SIZE);
  G_consumer2producer_queue = queue_init(QUEUE_SIZE);

  //
  // Launch producers
  //

  for (int i=0; i<PRODUCER_THREADS; i++)
  { queue_put(G_producer_threads_queue, i); }

  int seed = 2;
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


  free(G_min);
  free(G_max);

  free(G_arr_producer);
  free(G_rnk_producer);
  free(G_tkn_producer);

  queue_destroy(G_producer_threads_queue);
  queue_destroy(G_consumer2producer_queue);

  return 0;
}

void launch_consumer(int i)
{
  struct consumer_data_t *cdata = G_consumer_data[i];

  cdata->i = i;
  
  PANIKON(pipe(cdata->p2c), "pipe() failed.\n");
  PANIKON(pipe(cdata->c2p), "pipe() failed.\n");

  cdata->pid = fork();
  PANIKON(cdata->pid == -1, "fork() failed.\n");

  if (!cdata->pid) 
  {
    close(cdata->p2c[1]); // child don't write here
    dup2(cdata->p2c[0], STDIN_FILENO);
    close(cdata->c2p[0]); // child don't read here
    dup2(cdata->c2p[1], STDOUT_FILENO);

    PANIKON(execlp(CONSUMERAPP, CONSUMERAPP, G_ncol_str, G_nrow_str, NULL) == -1, "execlp() failed.\n");
  }

  cdata->fs_p2c_w = fdopen(cdata->p2c[0], "w");
  cdata->fs_c2p_r = fdopen(cdata->c2p[1], "r");
}

void *listen_consumer(void *arg)
{
  int i = *(int*) arg;

  struct consumer_data_t *cdata = &G_consumer_data[i];

  while (fgetc(cdata->fs_c2p_r))
  {
    queue_put(consumer2producer_queue, i);
  }
}

