/* ==============================
 *      INCLUDES
 * ==============================*/
#include "common.h"
#include <pthread.h>
#include "syt.h"

/* =============================
 *      FUNCTION-LIKE MACROS
 * =============================*/
#define BAD_NEIGHBORS(t,A,i,m) ((m<i && A[i-m]>=t) || (i%m>0 && A[i-1]>=t))
#define PRINT_ARRAY(fd, A, beg, end, m){\
    for (int j = beg; j < end; j++)\
    {\
        char c = ((j+1)%m == 0) ? ';': ',';\
        fprintf(fd, "%i%c", A[j], c);\
    }\
    fprintf(fd, "%i\n", A[end]);}

/* ==================================
 *      PROTOTYPES and DATA TYPES 
 * ==================================*/
typedef struct {
    int thread_id;
    union {
        int seed;
        int *A;
    };
} worker_param_t;

void* syt_worker_compute(void *params);
void* syt_worker_solvelp(void *params);


/* ==============================
 *      GLOBAL DATA
 * ==============================*/

static int nrow, ncol, sz;
static int *min, *max;

static int *A_pool; // Array of tableau
static int *T_pool; // Array of taken markers

// TODO: thread pool
static struct {
    pthread_t t[MAXTHREADS];
} thread_pool;

/* =======================
 *      IMPLEMENTATION
 * =======================*/

int syt_compute(int ncols, int nrows) 
/* Compute Standard Young Tableaux by a naive brute-force method
 *
 * Input : size of the table to compute (#cols x #rows).
 * Output: status code.
 *  - Files : CSV files representing each configuration in the various threads.
 *
 * Method: Every cell has a minimum and maximum possible value. We traverse the table
 * in a left-to-right bottom-up fashion, choosing at each step the lowest possible value. 
 * The routine stops when this backwards process find its way to the very first cell.
 * 
 * Threads: The first cell to the right of 1 can take values between 2 and #rows.
 * We use this information to distribute each of these posible starting values (seeds)
 * to an individual thread.
 * */
{
    //
    // Initialization
    //
    ncol = ncols;
    nrow = nrows;
    sz = ncol*nrow;

    //
    // Compute min and max
    //
    #ifndef CALLBACK
    #define CALLBACK { return -1; }
    malloc_or(min, sz*sizeof(*min), CALLBACK);
    malloc_or(max, sz*sizeof(*max), CALLBACK);
    malloc_or(A_pool, MAXTHREADS*sz*sizeof(*A_pool), CALLBACK);
    malloc_or(T_pool, MAXTHREADS*(sz+1)*sizeof(*T_pool), CALLBACK);
    #undef CALLBACK
    #else
    #error "CALLBACK macro is already defined."
    #endif

    for (int r = 0, i = 0; r < nrow; r++)
    {
        for (int c = 0; c < ncol; c++, i++)
        {
            min[i] = (r+1) * (c+1);
            max[i] = sz - (nrow-r)*(ncol-c) + 1;
        }
    }

    //
    // Launch Threads
    //
    int nthreads = nrow > MAXTHREADS ? MAXTHREADS: nrow;
    worker_param_t params[nthreads];

    for (int i = 0; i < nthreads; i++)
    {
        params[i].thread_id = i;
        params[i].seed = i+2;
        pthread_create(&thread_pool.t[i], NULL, syt_worker_compute, &params[i]);
    }

    //
    // Wait for threads to finish
    //
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(thread_pool.t[i], NULL);
    }

    free(min);
    free(max);
    free(A_pool);
    free(T_pool);
    return 0;
}

void* syt_worker_compute(void* params)
/* TODO: Add description */
{
    //
    // Gather and initialize data.
    //
    worker_param_t param = *(worker_param_t*) params;

    int offset = param.thread_id * sz;
    int *arr = &A_pool[offset];
    int *taken = &T_pool[offset];
    for (int k=0; k < sz+1; k++) { taken[k] = 0; }

    arr[0] = 1;
    arr[1] = param.seed;
    arr[sz-1] = sz;

    taken[1] = 1;
    taken[param.seed] = 1;
    taken[sz] = 1;

    //
    // File to write output.
    //
    char filename[50];
    sprintf(filename, "raw/m%in%it%i", ncol, nrow, param.thread_id);
    FILE* fd;
    open_or(fd, filename, "w", { return NULL; });

    //
    // Fill table with minimal configuration.
    //
    int i = 2;
    int t = min[i];
    int imax = max[i];
    do {
        if (taken[t] || BAD_NEIGHBORS(t, arr, i, ncol))
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

    //
    // Once the table is filled, we start going backwards
    // trying to subsitute every value in range posible.
    //
    while (i > 1)
    {
        if (i == sz-1)
        {
            PRINT_ARRAY(fd, arr, 0, sz-1, ncol);
            i -= 1;
        }

        imax = max[i];
        t = arr[i] > 0 ? arr[i]: min[i];

        while (t <= imax && (taken[t] || BAD_NEIGHBORS(t, arr, i, ncol)))
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
