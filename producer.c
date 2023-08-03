#include <string.h>
#include <stdio.h>
#include "producer.h"

static inline int bad_neighbors(int value, int *arr, int i);

void* generate_table(void* param)
{
  //
  // Gather and initialize data.
  //
  int i = ((struct producer_param_t*) param)->i;
  int seed = ((struct producer_param_t*) param)->seed;

  int *arr = &G_arr_producer[i * G_sz];
  int *tkn = &G_tkn_producer[i * (G_sz+1)];
  int *rnk = &G_rnk_producer[i * (G_sz+1)];

  for (int k=0; k<G_sz+1; k++)
  { tkn[k] = 0; rnk[k] = 0; }

  arr[0] = 1;
  arr[1] = seed;
  arr[G_sz-1] = G_sz;

  tkn[1] = 1;
  tkn[seed] = 1;
  tkn[G_sz] = 1;

  rnk[1] = 0;
  rnk[seed] = 1;
  rnk[G_sz] = G_sz-1;

  //
  // Fill table with minimal configuration.
  //
  int pos = 2;
  int t = G_min[pos];
  do {
    if (tkn[t] || bad_neighbors(t, arr, pos))
    {
      t++;
    } 
    else 
    {
      arr[pos] = t;
      tkn[t] = 1;
      rnk[t] = pos;
      pos += 1;
      t = G_min[pos];
    }
  } while (pos < G_sz-1);

  //
  // Once the table is filled, we start going backwards
  // trying every value posible.
  //
  while (pos > 1)
  {
    if (pos == G_sz-1)
    {
      int d = queue_get(G_consumer2producer_queue);

      // Copy data 
      int *arr_ite = &G_arr_consumer[d*G_sz];
      int *rnk_ite = &G_rnk_consumer[d*(G_sz+1)];
      for (int k=0; k < G_sz; k++)
      { arr_ite[k]=arr[k]; rnk_ite[k+1]=rnk[k+1]; }

      queue_put(G_producer2consumer_queue, d);

      pos -= 1;
    }

    int imax = G_max[pos];
    t = arr[pos] > 0 ? arr[pos]: G_min[pos];

    while (t <= imax && (tkn[t] || bad_neighbors(t, arr, pos)))
    {
      t++;
    }

    tkn[arr[pos]] = 0;
    if (t > imax)
    {
      arr[pos] = 0;
      pos -= 1;
    } 
    else
    {
      arr[pos] = t;
      tkn[t] = 1;
      rnk[t] = pos;
      pos += 1;
    }
  }

  queue_put(G_producer_threads_queue, i);

  return NULL;
}

static inline int bad_neighbors(int value, int *arr, int i)
{
  int up = (G_ncol<i && arr[i-G_ncol] >= value);
  int left = (i%G_ncol>0 && arr[i-1] >= value);
  return up || left;
}
