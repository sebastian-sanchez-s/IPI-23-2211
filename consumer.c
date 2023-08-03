#include "qsopt.h"
#include "setoper.h"
#include "cdd.h"
#include "queue.h"
#include "consumer.h"

static inline void print_table(FILE *f, int *arr);

double solve_with_qsopt(int *rnk);
double solve_with_cdd(int *rank);

void* solve_LP(void *nouse)
/* Build inequalities from tableau and solves the lp with cdd */
{
  while (1)
  {
    int i = queue_get(G_producer2consumer_queue);

    if (i < 0) // endthread
    {
      return NULL;
    }

    int *arr = &G_arr_consumer[i * G_sz];
    int *rnk = &G_rnk_consumer[i * (G_sz+1)];

    FILE *fpos = G_files_pos[i];
    FILE *fneg = G_files_neg[i];

    //double optval = solve_with_qsopt(rnk);
    double optval = solve_with_cdd(rnk);

    if (optval > dd_almostzero)//0.0)
    {
      print_table(fpos, arr);
      fprintf(fpos, "\n");
    }
    else
    {
      print_table(fneg, arr);
      fprintf(fneg, "\n");
    }
    queue_put(G_consumer2producer_queue, i);
  }

  return NULL;
}

double solve_with_qsopt(int *rnk)
{
  #define NUM_INE_TABLEAU (G_sz-1)
  #define NUM_VAR (G_ncol + G_nrow)

  int rval;
  QSprob p = QScreate_prob(NULL, QS_MAX);
  if (p == NULL) fprintf(stderr, "Qcreate_prob failed.\n");

  /* create variables x_1, ..., y_1, ..., epsilon */
  for (int k=0; k<NUM_VAR; k++)
  {
    rval = QSnew_col(p, 0.0, 0.0, 1.0, NULL);
    if (rval) fprintf(stderr, "QSadd_col failed.\n");
  }
  rval = QSnew_col(p, 1.0, 0.0, QS_MAXDOUBLE, NULL);
  if (rval) fprintf(stderr, "QSadd_col failed.\n");

  for (int k=1; k <= NUM_INE_TABLEAU; k++)
  /* For every rank k: 0<= -y_col(k) - x_row(k) - epsilon + y_col(k+1) + x_row(k+1). */
  {
    // y variables starts after x variables .ie. at index G_nrow+1
    int y_col_k = (rnk[k]%G_ncol) + G_nrow;
    int x_row_k = (rnk[k]/G_ncol);
    int y_col_kp1 = (rnk[k+1]%G_ncol) + G_nrow;
    int x_row_kp1 = (rnk[k+1]/G_ncol);
    
    // Cancel out variables
    double x_coeff = (x_row_k == x_row_kp1) ? 0.0: 1.0;
    double y_coeff = (y_col_k == y_col_kp1) ? 0.0: 1.0;
    
    int rmatind[5] = {x_row_k, x_row_kp1, y_col_k, y_col_kp1, NUM_VAR};
    double rmatval[5] = {-x_coeff, x_coeff, -y_coeff, y_coeff, -1.0};

    rval = QSadd_row(p, 5, rmatind, rmatval, 0.0, 'G', NULL);
    if (rval) fprintf(stderr, "QSadd_row failed.\n");
  }

  for (int k=0; k < G_nrow-1; k++)
  /* 0 <= -x_k -epsilon + x_{k+1} */
  {
    int rmatind[3] = {k, k+1, NUM_VAR};
    double rmatval[3] = {-1.0, 1.0, -1.0};

    rval = QSadd_row(p, 3, rmatind, rmatval, 0.0, 'G', NULL);
    if (rval) fprintf(stderr, "QSadd_row failed.\n");
  }

  for (int k=0; k < G_ncol-1; k++)
  /* 0<= -y_k - epsilon + y_{k+1} */
  {
    int rmatind[3] = {G_nrow+k, G_nrow+k+1, NUM_VAR};
    double rmatval[3] = {-1.0, 1.0, -1.0};

    rval = QSadd_row(p, 3, rmatind, rmatval, 0.0, 'G', NULL);
    if (rval) fprintf(stderr, "QSadd_row failed.\n");
  }

  int status;
  rval = QSopt_dual(p, &status);
  if (rval) fprintf(stderr, "QSopt_dual failed.\n");

  double optval;
  rval = QSget_objval(p, &optval);
  if (rval) fprintf(stderr, "QSget_objval failed.\n");

  QSfree_prob(p);

  return optval;
}

double solve_with_cdd(int *rank)
{
  dd_set_global_constants();
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


  double objval = *lp->optvalue;

  dd_FreeLPData(lp);
  dd_FreeMatrix(A);

  dd_free_global_constants();

  return objval;
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

