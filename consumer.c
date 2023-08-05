#include "util.h"

#include "qsopt.h"

#include "setoper.h"
#include "cdd.h"

double solve_with_qsopt();
double solve_with_cdd();

#ifndef CDDSOLVER
#define CDDSOLVER 1
#else
#error "CDDSOLVER already defined."
#endif
#ifndef QSOPTSOLVER
#define QSOPTSOLVER 2
#else
#error "QSOPT already defined."
#endif

#ifndef SOLVER
#define SOLVER CDDSOLVER
#else
#error "SOLVER already defined."
#endif

void die(int sig);

int G_nrow, G_ncol, G_sz;
int *rnk = NULL;
int *arr = NULL;

FILE *fp = NULL;
FILE *fn = NULL;

//int tcount = 0, pcount = 0;

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: ./consumer <ncol> <nrow> <t>.\n");
    exit(-1);
  }

  G_ncol = atoi(argv[1]);
  G_nrow = atoi(argv[2]);
  G_sz = G_nrow*G_ncol;

  int i = atoi(argv[3]);

  _debug("G_ncol = %i, G_nrow = %i, G_sz == %i\n", G_ncol, G_nrow, G_sz);

  #define BUFFSIZE 40

  char filename[BUFFSIZE];
  snprintf(filename, BUFFSIZE, "raw/Pc%ir%it%i", G_ncol, G_nrow, i);
  fp = fopen(filename, "w");
  PANIKON(fp==NULL, "fopen() failed.");

  snprintf(filename, BUFFSIZE, "raw/Nc%ir%it%i", G_ncol, G_nrow, i);
  fn = fopen(filename, "w");
  PANIKON(fn==NULL, "fopen() failed.");

  MALLOC(arr, sizeof(int[G_sz]));
  MALLOC(rnk, sizeof(int[G_sz+1]));

  while (1)
  {
    // Tell producer we are ready to read arrays
    printf("%i\n", i);
    fflush(stdout);

    // Expect response
    int flag;
    scanf("%i", &flag);
    if (flag < 0) die(0);
    
    _debugC("Wait for array.\n");
    READARR(stdin, arr, 0, G_sz);
    READARR(stdin, rnk, 1, G_sz);

    _debugC("Got array:\n");
    //PRINTARR(stderr, arr, 0, G_sz);
    //PRINTARR(stderr, rnk, 1, G_sz);
    
    double optval, zero;
#if SOLVER == CDDSOLVER
    optval = solve_with_cdd();
    zero = dd_almostzero;
#elif SOLVER == QSOPTSOLVER
    optval = solve_with_qsopt();
#else
#error "SOLVER not identified."
#endif

    //tcount++;
    if (optval > zero)
    {
      //pcount++;
      PRINTARR(fp, arr, 0, G_sz);
    }
    else
    {
      PRINTARR(fn, arr, 0, G_sz);
    }
  }

  die(0);
}

double solve_with_qsopt()
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

double solve_with_cdd()
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
      int y_col_k = (rnk[k]%G_ncol) + G_nrow+1;
      int x_row_k = (rnk[k]/G_ncol) + 1;
      int y_col_kp1 = (rnk[k+1]%G_ncol) + G_nrow+1;
      int x_row_kp1 = (rnk[k+1]/G_ncol) + 1;
      
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
  dd_ErrorType error = dd_NoError;
  dd_LPPtr lp = dd_Matrix2LP(A, &error);
  PANIKON( lp==NULL, "dd_Matrix2LP() failed.");

  dd_LPSolverType solver = dd_DualSimplex;
  dd_LPSolve(lp, solver, &error);
  PANIKON( error!=dd_NoError, "dd_LPSolve() failed.");

  double objval = *lp->optvalue;

  dd_FreeLPData(lp);
  dd_FreeMatrix(A);
  dd_free_global_constants();

  return objval;
}

void die(int sig)
{
  //fprintf(fp, "Total=%i,Realizable=%i\n", tcount, pcount);
  fclose(fp);
  fclose(fn);
  free(arr);
  free(rnk);
  exit(sig);
}
