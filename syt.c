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
    int thread_index;
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

#define MAXTHREADS 8
static struct {
    struct worker_param_t param[MAXTHREADS];
    pthread_t thread[MAXTHREADS];
    FILE *fpos[MAXTHREADS]; // realizable
    FILE *fneg[MAXTHREADS]; // non realizable
} G_threads;

static int G_nrow, G_ncol, G_sz;
static int *G_min, *G_max;

static int *A_pool; // Array of tableau
static int *T_pool; // Array of taken markers
static int *R_pool; // Array of rank (inverse of A) 

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
    dd_set_global_constants();

    G_ncol = ncols;
    G_nrow = nrows;
    G_sz = G_ncol*G_nrow;

    //
    // Compute G_min and G_max
    //
    G_min = malloc(sizeof(int[G_sz]));
    G_max = malloc(sizeof(int[G_sz]));
    A_pool = calloc(1, sizeof(int[2*MAXTHREADS][G_sz]));
    T_pool = calloc(1, sizeof(int[MAXTHREADS][G_sz+1]));
    R_pool = calloc(1, sizeof(int[2*MAXTHREADS][G_sz+1]));


    for (int r = 0, i = 0; r < G_nrow; r++)
    {
        for (int c = 0; c < G_ncol; c++, i++)
        {
            G_min[i] = (r+1) * (c+1);
            G_max[i] = G_sz - (G_nrow-r)*(G_ncol-c) + 1;
        }
    }

    //
    // Launch Threads
    //
    if (G_nrow > MAXTHREADS)
    {
        fprintf(stderr, "Not enough threads.\n");
        return -1;
    }

    for (int i = 0; i < G_nrow; i++)
    {
        G_threads.param[i].thread_index = i;
        G_threads.param[i].seed = i+2;

        char filename[50];
        sprintf(filename, "raw/Pm%in%it%i", G_ncol, G_nrow, i);
        G_threads.fpos[i] = fopen(filename, "w");
        sprintf(filename, "raw/Nm%in%it%i", G_ncol, G_nrow, i);
        G_threads.fneg[i] = fopen(filename, "w");

        pthread_create(&G_threads.thread[i], NULL, syt_worker_compute, &G_threads.param[i]);
    }

    //
    // Wait for threads to finish
    //
    for (int i = 0; i < G_nrow; i++)
    {
        pthread_join(G_threads.thread[i], NULL);
        fclose(G_threads.fpos[i]);
        fclose(G_threads.fneg[i]);
    }

    free(G_min);
    free(G_max);
    free(A_pool);
    free(T_pool);
    free(R_pool);
    dd_free_global_constants();

    return 0;
}

static inline int bad_neighbors(int value, int *arr, int i)
{
    int up = (G_ncol<i && arr[i-G_ncol] >= value);
    int left = (i%G_ncol>0 && arr[i-1] >= value);
    return up || left;
}

static inline void print_table(FILE *f, int *arr)
{
    for (int j = 0; j < G_sz-1; j++)
    {
        char c = ((j+1)%G_ncol == 0) ? ';': ',';
        fprintf(f, "%i%c", arr[j], c);
    }
    fprintf(f, "%i", arr[G_sz-1]);
}

void* syt_worker_compute(void* params)
/* TODO: Add description */
{
    //
    // Gather and initialize data.
    //
    struct worker_param_t param = *(struct worker_param_t*) params;

    int offset = param.thread_index*G_sz;

    int *arr = &A_pool[offset];
    int *taken = &T_pool[offset];
    int *rank = &R_pool[offset];

    arr[0] = 1;
    arr[1] = param.seed;
    arr[G_sz-1] = G_sz;

    taken[1] = 1;
    taken[param.seed] = 1;
    taken[G_sz] = 1;

    rank[1] = 0;
    rank[param.seed] = 1;
    rank[G_sz] = G_sz-1;

    //
    // Fill table with minimal configuration.
    //
    int i = 2;
    int t = G_min[i];
    do {
        if (taken[t] || bad_neighbors(t, arr, i))
        {
            t++;
        }
        else
        {
            arr[i] = t;
            taken[t] = 1;
            rank[t] = i;
            i += 1;
            t = G_min[i];
        }
    } while (i < G_sz-1);

    //
    // Once the table is filled, we start going backwards
    // trying to subsitute every value in range posible.
    //
    while (i > 1)
    {
        if (i == G_sz-1)
        {
            //
            // Launch solver thread
            //
            struct worker_param_t *solver_params = &G_threads.param[param.thread_index];

            solver_params->thread_index = param.thread_index;

            solver_params->arr = &A_pool[(param.thread_index + MAXTHREADS)*G_sz];
            memcpy(solver_params->arr, arr, G_sz*sizeof(int));

            solver_params->rank = &R_pool[(param.thread_index + MAXTHREADS)*G_sz];
            memcpy(solver_params->rank, rank, (G_sz+1)*sizeof(int));

            syt_worker_solvelp((void*)solver_params);

            i -= 1;
        }

        int imax = G_max[i];
        t = arr[i] > 0 ? arr[i]: G_min[i];

        while (t <= imax && (taken[t] || bad_neighbors(t, arr, i)))
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
    FILE *fpos = G_threads.fpos[param->thread_index];
    FILE *fneg = G_threads.fneg[param->thread_index];

    //
    // Build LP problem
    //
    #define NUM_INE_TABLEAU (G_sz-1)
    #define NUM_INE_ORDER_X (G_nrow-1)
    #define NUM_INE_ORDER_Y (G_ncol-1)
    #define NUM_INE_LT_ONE (G_ncol + G_nrow)
    #define NUM_INE_GT_ZERO (G_ncol + G_nrow+1)
    #define NUM_INE_TOTAL (NUM_INE_TABLEAU + NUM_INE_ORDER_X + NUM_INE_ORDER_Y + NUM_INE_LT_ONE + NUM_INE_GT_ZERO) 
    #define NUM_VAR (G_ncol + G_nrow)

    dd_MatrixPtr A = dd_CreateMatrix(NUM_INE_TOTAL, 1 + NUM_VAR + 1); // 2 more for epsilon and constant 

    //
    // Build inequalities
    //
    int offset = 0;
    for (int k = 1; k <= NUM_INE_TABLEAU; k++)
    /* For every rank k: 0<= -y_col(k) - x_row(k) - epsilon + y_col(k+1) + x_row(k+1). */
    {
        // y variables starts after x variables .ie. at index G_nrow+1
        int y_col_k = (rank[k]%G_ncol) + G_nrow+1;
        int x_row_k = (rank[k]/G_ncol) + 1;
        int y_col_kp1 = (rank[k+1]%G_ncol) + G_nrow+1;
        int x_row_kp1 = (rank[k+1]/G_ncol) + 1;
        
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
        dd_set_si(A->matrix[k-1 + offset][G_ncol+k-1], -1); 
        dd_set_si(A->matrix[k-1 + offset][NUM_VAR+1], -1);
        dd_set_si(A->matrix[k-1 + offset][G_ncol+k], 1);
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
        print_table(fpos, arr);

        //
        // Print x (vertical) vector
        //
        fprintf(fpos, " %lf", *lp->sol[1]);
        for (int j=2; j<=G_nrow; j++) 
        {
            fprintf(fpos, ",");
            fprintf(fpos, "%lf", *lp->sol[j]);
        }

        //
        // Print y (horizontal) vector
        //
        fprintf(fpos, " %lf", *lp->sol[G_nrow+1]);
        for (int j=2; j<=G_ncol; j++) 
        {
            fprintf(fpos, ",");
            fprintf(fpos, "%lf", *lp->sol[G_nrow+j]);
        }

        //
        // Print optimal value
        //
        fprintf(fpos, " %lf", *lp->optvalue);
        fprintf(fpos, "\n");
    } else {
        print_table(fneg, arr);
        fprintf(fneg, "\n");
    }

    dd_FreeLPData(lp);
    dd_FreeMatrix(A);

    return NULL;
}
