#include "producer.h"
#include "table.h"

inline int bad_neighbors(int value, int *arr, int j)
{
  int up = (G_ncol<j && arr[j-G_ncol] >= value);
  int left = (j%G_ncol>0 && arr[j-1] >= value);
  return up || left;
}

void* generate_table(void* param)
{
  //
  // Gather and initialize data.
  //
  int i = ((struct producer_param_t*) param)->i;
  int seed = ((struct producer_param_t*) param)->seed;

  struct table_t syt   = (struct table_t) { .c=G_ncol, 
                                            .r=G_nrow, 
                                            .sz=G_sz, 
                                            .t=&G_arr[i*G_sz]};
  struct table_t taken = (struct table_t){ .c=G_ncol, 
                                           .r=G_nrow, 
                                           .sz=G_sz+1, 
                                           .t=&G_tkn[i*(G_sz+1)]};

  table_set_all(&taken, 0);

  syt.t[0] = 1;
  syt.t[1] = seed;
  syt.t[G_sz-1] = G_sz;

  taken.t[1] = 1;
  taken.t[seed] = 1;
  taken.t[G_sz] = 1;

  struct table_t *rank = table_init(G_ncol, G_nrow);

  //
  // Fill table with minimal configuration.
  //
  int pos = 2;
  int t = G_min[pos];
  do {
    if (taken.t[t] || bad_neighbors(t, syt.t, pos))
    {
      t++;
    } 
    else
    {
      syt.t[pos] = t;
      taken.t[t] = 1;
      pos += 1;
      t = G_min[pos];
    }
  } while (pos < G_sz-1);

  //
  // Once the table is filled, we start going backwards
  // trying every value posible.
  //
  while( pos > 1 )
  {
    if( pos == G_sz-1 )
    {
      //
      // Only set a table if it does not have any
      // bad subtable
      //
      if( G_avl_banned_tables == NULL 
          || !table_has_banned_subtable(G_avl_banned_tables, &syt) )
      {
        int c = queue_get(G_consumer2producer_queue);

        // Send data to consumer 
        FILE *out = G_consumer_data[c].fs_w; PANIKON(out==NULL, "fs_w of %i is null\n", c);

        fprintf(out, "1\n");
        PRINTARR(out, syt.t, 0, G_sz);
        fflush(out);

        memcpy(rank->t, syt.t, sizeof(int[G_sz]));
        table_linked_rank(rank);
        PRINTARR(out, rank->t, 0, rank->sz);
        fflush(out);
      } 
      //else 
      //{
      //  fprintf(stderr, "BANNED\t");
      //  PRINTARR(stderr, syt.t, 0, G_sz);
      //}
      pos -= 1;
    }

    int imax = G_max[pos];
    t = syt.t[pos] > 0 ? syt.t[pos]: G_min[pos];

    while( t <= imax 
        && ( taken.t[t] || bad_neighbors(t, syt.t, pos)) )
    {
      t++;
    }

    taken.t[syt.t[pos]] = 0;
    if (t > imax)
    {
      syt.t[pos] = 0;
      pos -= 1;
    } 
    else
    {
      syt.t[pos] = t;
      taken.t[t] = 1;
      pos += 1;
    }
  }

  table_destroy(rank);

  queue_put(G_producer_threads_queue, i);
  pthread_exit(NULL);
}

