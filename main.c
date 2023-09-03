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
 * @file main.c
 * @author Sebastián Sánchez 
 * @date Aug 2032
 * @brief Set up communication channels and start threads.
 */

#include "common.h"
#include "consumer.h"
#include "queue.h"
#include "producer.h"
#include "table.h"

void *thread_void();
void *listen_consumer(void *arg);
void launch_consumer(int i);
static int bad_neighbors(int value, int *arr, int j);

#define NUM_PRODUCER 5 //!< Producer threads
#define NUM_CONSUMER 4 //!< Consumer threads and processes

pthread_t G_producer[NUM_PRODUCER];                       //!< indexed Producer resources
struct consumer_data_t G_consumer_data[NUM_CONSUMER];     //!< indexed consumer resources
struct producer_param_t G_producer_params[NUM_PRODUCER];  //!< indexed producer parameters

struct queue_t *G_producer_queue = NULL; //!< queue for threads producing tables.
struct queue_t *G_consumer_queue = NULL; //!< queue for processes consuming tables.

int G_nrow; //!< rows of the table given as input.
int G_ncol; //!< columns of the table given as input.
int G_sz;   //!< size of the table.
            
//! A nrowsxncols matrix with the minimum value possible in each entry.
int *G_min;
//! A nrowsxncols matrix with the maximum value possible in each entry.
int *G_max;
//! A matrix use for main to store the seed.
int *G_arr; 
//! An auxiliary matrix use to know which values are already been taken.
int *G_tkn; 
            
//! A structure to check if a table has a banned subtable.
struct pair_list_t *G_banned_tables = NULL;

int main(int argc, char *argv[])
/**
 * Set up the communication between Producer and Consumers.
 *
 * For consumers: it launch the processes as well as listening threads.
 * When a consumers signals readyness, main put that consumer into the 
 * G_consumer_queue.
 *
 **/
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
  MALLOC(G_tkn, sizeof(int[NUM_PRODUCER][G_sz+1]));

  G_producer_queue  = queue_init(NUM_PRODUCER);
  G_consumer_queue = queue_init(NUM_CONSUMER);

  //
  // Build banned tables structure
  //
  if (G_ncol > 3 || G_nrow > 3)
  {
    G_banned_tables = load_banned_subtables(G_ncol, G_nrow);
  }

  //
  // Launch threads 
  //
  for (int i=0; i<NUM_CONSUMER; i++)
  {
    pthread_create(&G_consumer_data[i].listener, NULL, listen_consumer, &G_consumer_data[i].i);
  }

  for (int i=0; i<NUM_PRODUCER; i++)
  {
    G_producer_params[i].i = i;
    pthread_create(G_producer + i, NULL, thread_void, NULL);
    queue_put(G_producer_queue, i);
  }

  
  //
  // Execute producers 
  //
  int min_pos = G_ncol;

  int *holder_tkn; CALLOC(holder_tkn, G_sz+1, sizeof(int));
  int *holder_arr; CALLOC(holder_arr, G_sz, sizeof(int));
  int holder_i = 1;
  int holder_t = G_min[holder_i];

  do {
    if( holder_tkn[holder_t] || bad_neighbors(holder_t, holder_arr, holder_i) )
    { holder_t++; } 
    else
    {
      holder_arr[holder_i] = holder_t;
      holder_tkn[holder_t] = 1;
      holder_i += 1;
      holder_t = G_min[holder_i];
    }
  } while( holder_i < min_pos );

  holder_arr[0] = 1;
  holder_arr[G_sz-1] = G_sz;
  holder_tkn[1] = 1;
  holder_tkn[G_sz] = 1;

  while( holder_i > 0 )
  {
    if( holder_i == min_pos )
    {
      int i = queue_get(G_producer_queue);

      pthread_join(G_producer[i], NULL);

      memcpy(G_arr + i*G_sz, holder_arr, sizeof(int[G_sz]));
      memcpy(G_tkn + i*(G_sz+1), holder_tkn, sizeof(int[G_sz+1]));

      G_producer_params[i] = (struct producer_param_t) { .i = i, .pos = min_pos-1 };

      pthread_create(G_producer + i, NULL, generate_table, G_producer_params + i);

      holder_i -= 1;
    }

    int imax = G_max[holder_i];
    holder_t = holder_arr[holder_i] > 0 ? holder_arr[holder_i]: G_min[holder_i];

    while( holder_t <= imax 
        && ( holder_tkn[holder_t] || bad_neighbors(holder_t, holder_arr, holder_i)) )
    { holder_t++; }

    holder_tkn[holder_arr[holder_i]] = 0;
    if( holder_t > imax )
    {
      holder_arr[holder_i] = 0;
      holder_i -= 1;
    } 
    else
    {
      holder_arr[holder_i] = holder_t;
      holder_tkn[holder_t] = 1;
      holder_i += 1;
    }
  }

  //
  // Gather everything back
  //

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
  free(G_tkn);

  free(holder_arr);
  free(holder_tkn);

  queue_destroy(G_producer_queue);
  queue_destroy(G_consumer_queue);

  if( G_banned_tables != NULL ) pair_list_destroy(G_banned_tables);

  return 0;
}

void *thread_void()
{
  return NULL;
}

void *listen_consumer(void *arg)
{
  int i = *(int*) arg;

  struct consumer_data_t *cdata = &G_consumer_data[i];

  int flag;
  while( fscanf(cdata->fs_r, "%i", &flag) != EOF)
  {
    queue_put(G_consumer_queue, i);
  }

  return NULL;
}

#define CONSUMERAPP "./obj/consumer.o"

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
    MALLOC(vars[1], sizeof(int[10])); snprintf(vars[1], 10, "%i", G_ncol);
    MALLOC(vars[2], sizeof(int[10])); snprintf(vars[2], 10, "%i", G_nrow);
    MALLOC(vars[3], sizeof(int[10])); snprintf(vars[3], 10, "%i", i);

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

static int bad_neighbors(int value, int *arr, int j)
{
  int up = (G_ncol<j && arr[j-G_ncol] >= value);
  int left = (j%G_ncol>0 && arr[j-1] >= value);
  return up || left;
}
