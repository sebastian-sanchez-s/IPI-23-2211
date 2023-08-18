#include "common.h"
#include "queue.h"
#include "producer.h"
#include "table.h"

/* = auxiliary functions = */
void *thread_void();
void *listen_consumer(void *arg);
void launch_consumer(int i);

/* = producer consumer globals = */
#define NUM_PRODUCER 2
#define NUM_CONSUMER 7

pthread_t G_producer[NUM_PRODUCER];
struct consumer_data_t G_consumer_data[NUM_CONSUMER];
struct producer_param_t G_producer_params[NUM_PRODUCER];

/* = queue globals = */
#define QUEUE_SIZE 2000
struct queue_t *G_producer_threads_queue;
struct queue_t *G_consumer2producer_queue;

/* = global auxialiry objects = */
int G_nrow, G_ncol, G_sz;
int *G_min, *G_max;

int *G_arr;
int *G_rnk;
int *G_tkn;

struct avl_node_t *G_avl_table = NULL;

int main(int argc, char *argv[])
{
  PANIKON(argc < 3, "Usage: ./<executable> <ncol> <nrow>\n");
  
  //
  // Initialization
  //
  G_ncol = atoi(argv[1]);
  G_nrow = atoi(argv[2]);
  G_sz = G_ncol*G_nrow;

  for (int i=0; i<NUM_CONSUMER; i++)
  { launch_consumer(i); }

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
  MALLOC(G_arr, sizeof(int[NUM_PRODUCER][G_sz]));
  MALLOC(G_rnk, sizeof(int[NUM_PRODUCER][G_sz+1]));
  MALLOC(G_tkn, sizeof(int[NUM_PRODUCER][G_sz+1]));

  G_producer_threads_queue  = queue_init(NUM_PRODUCER);
  G_consumer2producer_queue = queue_init(QUEUE_SIZE);

  //
  // Build avl tree
  //
  if (G_ncol > 3 || G_nrow > 3)
  {
    G_avl_table = avl_from_file(3, 3); 
    //avl_print(G_avl_table);
  }

  //
  // Launch threads 
  //
  for (int i=0; i<NUM_CONSUMER; i++)
  {
    pthread_create(&G_consumer_data[i].listener,
                    NULL,
                    listen_consumer,
                    &G_consumer_data[i].i);
  }

  for (int i=0; i<NUM_PRODUCER; i++)
  {
    G_producer_params[i].i = i;
    pthread_create(&G_producer[i], NULL, thread_void, NULL);
    queue_put(G_producer_threads_queue, i); 
  }

  
  //
  // Execute producers 
  //
  int seed = 2;
  while(seed < G_nrow+2)
  {
    int i = queue_get(G_producer_threads_queue);

    pthread_join(G_producer[i], NULL);

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

  for (int i=0; i<NUM_PRODUCER; i++)
  { pthread_join(G_producer[i], NULL); }

  for (int i=0; i<NUM_CONSUMER; i++)
  {
    struct consumer_data_t *cdata = &G_consumer_data[i];

    fprintf(cdata->fs_w, "-1\n"); fflush(cdata->fs_w);

    waitpid(cdata->pid, NULL, 0);

    fclose(cdata->fs_w);
    fclose(cdata->fs_r);
  }

  free(G_min);
  free(G_max);

  free(G_arr);
  free(G_rnk);
  free(G_tkn);

  queue_destroy(G_producer_threads_queue);
  queue_destroy(G_consumer2producer_queue);

  return 0;
}

void *thread_void()
{
  pthread_exit(0);
}

void *listen_consumer(void *arg)
{
  int i = *(int*) arg;

  struct consumer_data_t *cdata = &G_consumer_data[i];

  int flag;
  while( fscanf(cdata->fs_r, "%i", &flag) != EOF)
  {
    queue_put(G_consumer2producer_queue, i);
  }

  pthread_exit(NULL);
}

#define CONSUMERAPP "./consumer"

void launch_consumer(int i)
{
  struct consumer_data_t *cdata = &G_consumer_data[i];

  cdata->i = i;
  
  int p2c[2], c2p[2]; // parent to child, child to parent
  PANIKON(pipe(p2c), "pipe() failed.");
  PANIKON(pipe(c2p), "pipe() failed.");

  cdata->pid = fork();
  PANIKON(cdata->pid == -1, "fork() failed.");

  if (!cdata->pid)
  {
    PANIKON( dup2(p2c[0], 0) == -1, "dup2() failed."); // child reads from stdin
    PANIKON( dup2(c2p[1], 1) == -1, "dup2() failed."); // child writes to stdout
    close(p2c[1]); close(p2c[0]);
    close(c2p[0]); close(c2p[1]); 

    char *vars[5]; vars[0] = CONSUMERAPP; vars[4] = NULL;
    MALLOC(vars[1], sizeof(char[6])); snprintf(vars[1], INTWIDTH-1, "%i", G_ncol);
    MALLOC(vars[2], sizeof(char[6])); snprintf(vars[2], INTWIDTH-1, "%i", G_nrow);
    MALLOC(vars[3], sizeof(char[6])); snprintf(vars[3], INTWIDTH-1, "%i", i);

    PANIKON(execv(CONSUMERAPP, vars) == -1, "execv() failed.");
  } 
  else 
  {
    close(p2c[0]); // parent doesn't read from here
    close(c2p[1]); // parent doesn't write here

    cdata->fs_r = fdopen(c2p[0], "r"); // read here
    PANIKON(cdata->fs_r==NULL, "fdopen() failed."); 

    cdata->fs_w = fdopen(p2c[1], "w"); // write here
    PANIKON(cdata->fs_w==NULL, "fdopen() failed."); 
  }
}
