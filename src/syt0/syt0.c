/* Compute Standard Young Tableaux by a naive brute-force method
 *
 * Input : Size of the table to compute (#cols x #rows).
 * Output: Multiple lines with comma separate values representing each configuration.
 *
 * Method: Every cell has a minimum and maximum possible value. We traverse the table
 * in a left-to-right bottom-up fashion, choosing at each step the lowest possible value. 
 * The rutine stops when this backwards process find its way to the very first cell.
 * */
#include "../common.h"

#define DEBUG 0

int main(int argc, char* argv[])
{
    if (argc < 2) 
    {
        fprintf(stderr, "Usage: ./<program name> <NCOLS> <NROWS>.\n");
    }

    /* 
     * Initialization
     * */
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int sz = n*m;
    
    int arr[sz];
    int min[sz];
    int max[sz];
    int taken[sz + 1];

    char filename[50];
    sprintf(filename, "raw/m%in%i", m, n);
    FILE* fd;
    open_or_exit(fd, filename, "w", -1);

    /*
     * Fill arrays with:
     * arr  : minimal configuration
     * taken: zeros
     * min  : base minimal value for each cell
     * max  : base maximal value for each cell
     * */
    int i=0;
    for (int r = 0; r < n; r++)
    {
        for (int c = 0; c < m; c++, i++)
        {
            arr[i] = i+1;
            taken[i] = 1;
            min[i] = (r+1) * (c+1); // Upper area behind
            max[i] = sz - (n-r)*(m-c) + 1; // Lower area forward
        }
    }
    
    /*
     * Backwards process. 
     * Each time we move backwards, if there is a choice
     * for that cell then we use it and move forwards. When
     * there's no choice we move backwards and repeat. 
     * */
    i = sz - 1;
    while (i > 0)
    {
        if (i == sz-1)
        {
            PRINT_ARRAY(fd, arr, 0, sz-1, m);
            i -= 1;
        }

        /*
         * Find the next minimal value of the cell.
         * */
        int imax = max[i];
        int t = arr[i] > 0 ? arr[i]: min[i];
        while (t <= imax && (taken[t] || BAD_NEIGHBORS(t,arr,i,m))) 
        {
            t++;
        }

        taken[arr[i]] = 0;

        if (t > imax)
        {
            /*
             * Move backwards
             * */
            arr[i] = 0;
            i -= 1;
        } 
        else
        {
            /*
             * Move forwards 
             * */
            arr[i] = t;
            taken[t] = 1;
            i += 1;
        }
    }

    fclose(fd);
    return 0;
}
