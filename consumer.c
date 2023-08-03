#include "qsopt.h"
#include "queue.h"
#include "consumer.h"

static inline void print_table(FILE *f, int *arr);

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

    print_table(fpos, arr);
    fprintf(fpos, "\n");
    queue_put(G_consumer2producer_queue, i);
    continue;
    //
    // Build LP problem
    //
    #define NUM_INE_TABLEAU (G_sz-1)
    #define NUM_VAR (G_ncol + G_nrow)

    int rval;
    QSprob p = QScreate_prob(NULL, QS_MAX);
    if (p == NULL) fprintf(stderr, "Qcreate_prob failed.\n");

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

    if (optval > 0.0)
    {
      print_table(fpos, arr);
      fprintf(fpos, "\n");
    }
    else
    {
      print_table(fneg, arr);
      fprintf(fneg, "\n");
    }

    QSfree_prob(p);
    queue_put(G_consumer2producer_queue, i);
  }

  return NULL;
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

