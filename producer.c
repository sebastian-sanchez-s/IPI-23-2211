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

#include "producer.h"
#include "table.h"

static int bad_neighbors(int value, int *arr, int j)
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
  int start_pos = ((struct producer_param_t*) param)->pos;

  struct table_t syt = (struct table_t) {
    .c=G_ncol, .r=G_nrow, .sz=G_sz, .t=&G_arr[i*G_sz]
  };

  int *taken = &G_tkn[i*(G_sz+1)];

  //
  // Fill table with minimal configuration.
  //
  
  int pos = start_pos+1;
  int t = G_min[pos];
  do {
    if (taken[t] || bad_neighbors(t, syt.t, pos))
    {
      t++;
    } 
    else
    {
      syt.t[pos] = t;
      taken[t] = 1;
      pos += 1;
      t = G_min[pos];
    }
  } while (pos < G_sz-1);

  //
  // Once the table is filled, we start going backwards
  // trying every value posible.
  //
  while( pos > start_pos )
  {
    if( pos == G_sz-1 )
    {
      //
      // Only set a table if it does not have any
      // bad subtable
      //
      int banpos = table_find_banned_subtable(G_banned_tables, &syt);
      if( banpos < 0 )
      {
        int c = queue_get(G_consumer2producer_queue);

        // Send data to consumer 
        FILE *out = G_consumer_data[c].fs_w; 
        PANIKON(out==NULL, "fs_w of %i is null\n", c);
        fprintf(out, "1\n");
        PRINTARR(out, syt.t, 0, G_sz);
        fflush(out);
      }

      pos -= 1;
    }

    int imax = G_max[pos];
    t = syt.t[pos] > 0 ? syt.t[pos]: G_min[pos];

    while( t <= imax 
        && ( taken[t] || bad_neighbors(t, syt.t, pos)) )
    {
      t++;
    }

    taken[syt.t[pos]] = 0;
    if (t > imax)
    {
      syt.t[pos] = 0;
      pos -= 1;
    } 
    else
    {
      syt.t[pos] = t;
      taken[t] = 1;
      pos += 1;
    }
  }

  queue_put(G_producer_threads_queue, i);

  return NULL;
}

