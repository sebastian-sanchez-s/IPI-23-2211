#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define DEBUG 0
#include "../common.h"

void* compute_tableux(void *arg);

int m, n, sz;
int *min, *max;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./<executable> <NCOLS> <NROWS>\n");
    }
    
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    sz = m*n;

    min = malloc(sz*sizeof(int));
    max = malloc(sz*sizeof(int));

    // Compute min and max for each cell
    for (int r = 0, i = 0; r < n; r++)
    {
        for (int c = 0; c < m; c++, i++)
        {
            min[i] = (r+1) * (c+1);
            max[i] = sz - (n-r)*(m-c) + 1;
        }
    }

    // Create and launch threads
    int nthreads = n;
    pthread_t threads[nthreads]; // n = #{2..max of first cell}
    int args[nthreads];

    for (int i = 0; i < nthreads; i++)
    {
        args[i] = i;
        pthread_create(&threads[i], NULL, compute_tableux, &args[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(min);
    free(max);
    return 0;
}

void* compute_tableux(void* arg)
{
    int seed = *(int*)arg + 2;
    char filename[50];
    sprintf(filename, "raw/m%i_n%i_s%i", m, n, seed);
    FILE* fd = fopen(filename, "w");
    
    int arr[sz];
    int taken[sz+1];
    for (int k=0; k < sz+1; k++) { taken[k] = 0; }

    arr[0] = 1;
    arr[1] = seed;
    arr[sz-1] = sz;

    taken[1] = 1;
    taken[seed] = 1;
    taken[sz] = 1;

    // fill table with minimal configuration
    int i = 2;
    int t = min[i];
    int imax = max[i];
    do {
        if (taken[t] || BAD_NEIGHBORS(t,arr,i,m))
        {
            t++;
        }
        else
        {
            arr[i] = t;
            taken[t] = 1;
            i += 1;
            t = min[i];
            imax = max[i];
        }
    } while (i < sz-1);

    // Go backwards
    while (i > 1)
    {
        if (i == sz-1)
        {
            PRINT_ARRAY(fd, arr, 0, sz-1, m);
            i -= 1;
        }

        imax = max[i];
        t = arr[i] > 0 ? arr[i]: min[i];

        while (t <= imax && (taken[t] || BAD_NEIGHBORS(t,arr,i,m)))
        {
            t++;
        }

        if (t > imax)
        {
            if (arr[i] != 0)
            {
                taken[arr[i]] = 0;
            }
            i -= 1;
        }
        else
        {
            if (arr[i] != 0)
            {
                taken[arr[i]] = 0;
            }

            if (i+1 < sz-1)
            {
                arr[i+1] = 0;
            }
            arr[i] = t;
            taken[t] = 1;
            i += 1;
        }
    }

    fclose(fd);

    return NULL;
}
