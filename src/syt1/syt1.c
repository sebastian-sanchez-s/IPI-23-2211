/* Compute Standard Young Tableaux by a naive brute-force method
 *
 * Input : Size of the table to compute (#cols x #rows).
 * Output: None.
 * Files : CSV files representing each configuration in the varios threads.
 *
 * Method: Every cell has a minimum and maximum possible value. We traverse the table
 * in a left-to-right bottom-up fashion, choosing at each step the lowest possible value. 
 * The routine stops when this backwards process find its way to the very first cell.
 * 
 * Threads: The first cell to the right of 1 can take values between 2 and #rows.
 * We use this information to distribute each of these posible starting values (seeds)
 * to an individual thread.
 * */

#include <pthread.h>
#include "../common.h"

void* compute_tableux(void *arg);

u8 m, n, sz;
u8 *min, *max;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./<executable> <NCOLS> <NROWS>\n");
    }
    
    /*
     * Initialization
     * */
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    sz = m*n;

    /*
     * Compute min and max
     * */
    malloc_or_exit(min, sz*sizeof(*min));
    malloc_or_exit(max, sz*sizeof(*max));

    for (u8 r = 0, i = 0; r < n; r++)
    {
        for (u8 c = 0; c < m; c++, i++)
        {
            min[i] = (r+1) * (c+1);
            max[i] = sz - (n-r)*(m-c) + 1;
        }
    }

    /*
     * Launch threads
     * */
    u8 nthreads = n;
    pthread_t threads[nthreads]; // n = #{2..max of first cell}
    u8 args[nthreads];

    for (u8 i = 0; i < nthreads; i++)
    {
        args[i] = i;
        pthread_create(&threads[i], NULL, compute_tableux, &args[i]);
    }

    /*
     * Wait for threads to finish
     * */
    for (u8 i = 0; i < nthreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(min);
    free(max);
    return 0;
}

void* compute_tableux(void* arg)
{
    /*
     * Gather and initialize data.
     * */
    u8 seed = *(int*)arg + 2;
    u8 arr[sz];
    u8 taken[sz+1];
    for (int k=0; k < sz+1; k++) { taken[k] = 0; }
    arr[0] = 1;
    arr[1] = seed;
    arr[sz-1] = sz;

    taken[1] = 1;
    taken[seed] = 1;
    taken[sz] = 1;

    /*
     * File to write output.
     * */
    char filename[50];
    sprintf(filename, RAWDIRFMT, m, n, seed);
    FILE* fd = fopen(filename, "w");

    /*
     * Fill table with minimal configuration.
     * */
    u8 i = 2;
    u8 t = min[i];
    u8 imax = max[i];
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

    /*
     * Go backwards
     * */
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

        taken[arr[i]] = 0;
        if (t > imax)
        {
            arr[i] = 0;
            i -= 1;
        }
        else
        {
            arr[i] = t;
            taken[t] = 1;
            i += 1;
        }
    }

    fclose(fd);

    return NULL;
}
