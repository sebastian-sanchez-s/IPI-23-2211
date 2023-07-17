/* Compute Standard Young Tableaux by a naive brute-force method
 *
 * Input : The size of the table to compute (an integer)
 * Output: Multiple lines with comma separate values representing earch configuration
 *
 * Method: Every cell has a minimum and maximum possible value. We traverse the table
 * in a right-to-left bottom-up fashion, choosing at each step the lowest possible value. 
 * The rutine stops when there are more values to choose from.
 * */
#include <stdio.h>
#include <stdlib.h>

#include "../common.h"

#define DEBUG 0

int main(int argc, char* argv[])
{
    if (argc < 2) 
    {
        fprintf(stderr, "Usage: ./<program name> <NCOLS> <NROWS>.\n");
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int sz = n*m;
    
    int arr[sz];
    int min[sz];
    int max[sz];
    int taken[sz + 1];

    // Initialize arrays
    int i=0;
    for (int r = 0; r < n; r++)
    {
        for (int c = 0; c < m; c++, i++)
        {
            arr[i] = i+1; // Initial configuration
            taken[i] = 1;

            // Compute canditates
            min[i] = (r+1) * (c+1); // Upper area behind
            max[i] = sz - (n-r)*(m-c) + 1; // Lower area forward
        }
    }
    
    // Computation
    i = sz - 1;
    while (i > 0)
    {
        if (i == sz-1)
        {
            PRINT_ARRAY(stdout, arr, 0, sz-1, m);
            i -= 1;
        }

        int imax = max[i];
        int t = arr[i] > 0 ? arr[i]: min[i];

        // Find first available number (must be in limits, not be taken and be valid)
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
        } else {
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

    return 0;
}
