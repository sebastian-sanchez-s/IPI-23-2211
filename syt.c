/* ==============================
 *      INCLUDES
 * ==============================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include "setoper.h"
#include "cdd.h"
#include "syt.h"


/* ==================================
 *      PROTOTYPES and DATA TYPES 
 * ==================================*/
struct worker_param_t {
    int thread_id;
    union {
        int seed;
        struct {
            int *arr;
            int *rank;
        };
    };
};

void* syt_worker_compute(void *params);
void* syt_worker_solvelp(void *params);

/* ==============================
 *      GLOBAL DATA
 * ==============================*/

// TODO: thread pool
#define MAXTHREADS 8
static struct {
    FILE *f[MAXTHREADS];
    pthread_t t[MAXTHREADS];
    struct worker_param_t p[MAXTHREADS];
} thread_pool;

static int nrow, ncol, sz;
static int *min, *max;

static int *A_pool; // Array of tableau
static int *T_pool; // Array of taken markers
static int *R_pool; // Array of taken markers


/* =======================
 *      IMPLEMENTATION (big functions)
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
    dd_set_global_constants();

    ncol = ncols;
    nrow = nrows;
    sz = ncol*nrow;

    //
    // Compute min and max
    //
    min = malloc(sizeof(int[sz]));
    max = malloc(sizeof(int[sz]));
    A_pool = calloc(1, sizeof(int[MAXTHREADS][sz]));
    T_pool = calloc(1, sizeof(int[MAXTHREADS][sz+1]));
    R_pool = calloc(1, sizeof(int[MAXTHREADS][sz+1]));

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

    for (int i = 0; i < nthreads; i++)
    {
        thread_pool.p[i].thread_id = i;
        thread_pool.p[i].seed = i+2;

        char filename[50];
        sprintf(filename, "raw/m%in%it%i", ncol, nrow, thread_pool.p[i].thread_id);
        thread_pool.f[i] = fopen(filename, "w");
        if (thread_pool.f[i] == NULL) goto FREE_GLOBALS;

        pthread_create(&thread_pool.t[i], NULL, syt_worker_compute, &thread_pool.p[i]);
    }

    //
    // Wait for threads to finish
    //
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(thread_pool.t[i], NULL);
        fclose(thread_pool.f[i]);
    }

FREE_GLOBALS:
    free(min);
    free(max);
    free(A_pool);
    free(T_pool);
    free(R_pool);
    dd_free_global_constants();
    return 0;
}

static inline int bad_neighbors(int value, int *arr, int i, int ncol)
{
    int up = (ncol<i && arr[i-ncol] >= value);
    int left = (i%ncol>0 && arr[i-1] >= value);
    return up || left;
}

void* syt_worker_compute(void* params)
/* TODO: Add description */
{
    //
    // Gather and initialize data.
    //
    struct worker_param_t param = *(struct worker_param_t*) params;

    int offset = param.thread_id * sz;
    int *arr = &A_pool[offset];
    int *taken = &T_pool[offset];
    int *rank = &R_pool[offset];

    arr[0] = 1;
    arr[1] = param.seed;
    arr[sz-1] = sz;

    taken[1] = 1;
    taken[param.seed] = 1;
    taken[sz] = 1;

    rank[1] = 0;
    rank[param.seed] = 1;
    rank[sz] = sz-1;

    //
    // Fill table with minimal configuration.
    //
    int i = 2;
    int t = min[i];
    int imax = max[i];
    do {
        if (taken[t] || bad_neighbors(t, arr, i, ncol))
        {
            t++;
        }
        else
        {
            arr[i] = t;
            taken[t] = 1;
            rank[t] = i;
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
            //
            // Launch solver thread
            //
            struct worker_param_t *sparams;

            sparams = malloc(sizeof(struct worker_param_t));

            sparams->thread_id = param.thread_id;

            sparams->arr = malloc(sz*sizeof(int));
            memcpy(sparams->arr, arr, sz*sizeof(int));

            sparams->rank = malloc((sz+1)*sizeof(int));
            memcpy(sparams->rank, rank, (sz+1)*sizeof(int));

            syt_worker_solvelp((void*) sparams);

            i -= 1;
        }

        imax = max[i];
        t = arr[i] > 0 ? arr[i]: min[i];

        while (t <= imax && (taken[t] || bad_neighbors(t, arr, i, ncol)))
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
            rank[t] = i;
            i += 1;
        }
    }

    return NULL;
}

void* syt_worker_solvelp(void *params)
/* Build inequalities from tableau and solves the lp with cdd */
{
    struct worker_param_t *param  = (struct worker_param_t*) params;
    int *arr = param->arr;
    int *rank = param->rank;
    FILE *fd = thread_pool.f[param->thread_id];

    //
    // Build LP problem
    //
    #define NUM_INE_TABLEAU (sz-1)
    #define NUM_INE_ORDER_X (nrow-1)
    #define NUM_INE_ORDER_Y (ncol-1)
    #define NUM_INE_LT_ONE (ncol + nrow)
    #define NUM_INE_GT_ZERO (ncol + nrow+1)
    #define NUM_INE_TOTAL (NUM_INE_TABLEAU + NUM_INE_ORDER_X + NUM_INE_ORDER_Y + NUM_INE_LT_ONE + NUM_INE_GT_ZERO) 
    #define NUM_VAR (ncol + nrow)

    dd_MatrixPtr A = dd_CreateMatrix(NUM_INE_TOTAL, 1 + NUM_VAR + 1); // 2 more for epsilon and constant 

    //
    // Build inequalities
    //
    int offset = 0;
    for (int k = 1; k <= NUM_INE_TABLEAU; k++)
    /* For every rank k: 0<= -y_col(k) - x_row(k) - epsilon + y_col(k+1) + x_row(k+1). */
    {
        // y variables starts after x variables .ie. at index nrow+1
        int y_col_k = (rank[k]%ncol) + nrow+1;
        int x_row_k = (rank[k]/ncol) + 1;
        int y_col_kp1 = (rank[k+1]%ncol) + nrow+1;
        int x_row_kp1 = (rank[k+1]/ncol) + 1;
        
        // If same variables appears on both sides, it gets cancel out
        int x_coeff = (x_row_k == x_row_kp1) ? 0: 1;
        int y_coeff = (y_col_k == y_col_kp1) ? 0: 1;

        dd_set_si(A->matrix[k-1][y_col_k], -y_coeff);
        dd_set_si(A->matrix[k-1][x_row_k], -x_coeff);
        dd_set_si(A->matrix[k-1][NUM_VAR+1], -1); // Epsilon
        dd_set_si(A->matrix[k-1][y_col_kp1], y_coeff);
        dd_set_si(A->matrix[k-1][x_row_kp1], x_coeff);
    }

    offset += NUM_INE_TABLEAU;
    for (int k = 1; k <= NUM_INE_ORDER_X; k++)
    /* 0 <= -x_k -epsilon + x_{k+1} */
    {
        dd_set_si(A->matrix[k-1 + offset][k], -1); 
        dd_set_si(A->matrix[k-1 + offset][NUM_VAR+1], -1);
        dd_set_si(A->matrix[k-1 + offset][k+1], 1);
    }

    offset += NUM_INE_ORDER_X;
    for (int k = 1; k <= NUM_INE_ORDER_Y; k++)
    /* 0<= -y_k - epsilon + y_{k+1} */
    {
        dd_set_si(A->matrix[k-1 + offset][ncol+k-1], -1); 
        dd_set_si(A->matrix[k-1 + offset][NUM_VAR+1], -1);
        dd_set_si(A->matrix[k-1 + offset][ncol+k], 1);
    }

    offset += NUM_INE_ORDER_Y;
    for (int k = 1; k <= NUM_INE_LT_ONE; k++)
    /* 0 <= -x_k + 1, 0 <= -y_k + 1*/
    {
        dd_set_si(A->matrix[k-1 + offset][k], -1); 
        dd_set_si(A->matrix[k-1 + offset][0], 1);
    }

    offset += NUM_INE_LT_ONE;
    for (int k = 1; k <= NUM_INE_GT_ZERO; k++)
    /* 0 <= x_k, 0 <= y_k */
    {
        dd_set_si(A->matrix[k-1 + offset][k], 1); 
    }

    dd_set_si(A->rowvec[NUM_VAR+1], 1); // Objective function

    //
    // Set LP and Solve 
    //
    A->objective = dd_LPmax;
    dd_ErrorType error=dd_NoError;
    dd_LPPtr lp = dd_Matrix2LP(A, &error);

    dd_LPSolverType solver = dd_DualSimplex;
    dd_LPSolve(lp, solver, &error);

    //
    // Output results
    //
    if (*lp->optvalue > dd_almostzero) {
        //
        // Print table
        //
        for (int j = 0; j < sz-1; j++)
        {
            char c = ((j+1)%ncol == 0) ? ';': ',';
            fprintf(fd, "%i%c", arr[j], c);
        }
        fprintf(fd, "%i", arr[sz-1]);

        //
        // Print x (vertical) vector
        //
        fprintf(fd, " %lf", *lp->sol[1]);
        for (int j=2; j<=nrow; j++) 
        {
            fprintf(fd, ",");
            fprintf(fd, "%lf", *lp->sol[j]);
        }

        //
        // Print y (horizontal) vector
        //
        fprintf(fd, " %lf", *lp->sol[nrow+1]);
        for (int j=2; j<=ncol; j++) 
        {
            fprintf(fd, ",");
            fprintf(fd, "%lf", *lp->sol[nrow+j]);
        }

        //
        // Print optimal value
        //
        fprintf(fd, " %lf", *lp->optvalue);
        fprintf(fd,"\n");
    }

    dd_FreeLPData(lp);
    dd_FreeMatrix(A);

    free(arr);
    free(rank);
    free(params);
    return NULL;
}
