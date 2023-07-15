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

#define DEBUG 0
#define ON_DEBUG(s) if(DEBUG) { s }

#define PRINT_ARRAY(output, arr, beg, end, m){\
    for (int j = beg; j < end; j++)\
    {\
        char c = ((j+1)%m == 0) ? ';': ',';\
        fprintf(output, "%i%c", arr[j], c);\
    }\
    fprintf(output, "%i", arr[end]);}

#define BAD_NEIGHBORS(t,arr,i,m) ((m<i && arr[i-m]>=t) || (i%m>0 && arr[i-1]>=t))


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
            ON_DEBUG(fprintf(stdout, "\n>> "););
            PRINT_ARRAY(stdout, arr, 0, sz-1, m);
            putchar('\n');
            i -= 1;
        }

        int imax = max[i];
        int t = arr[i] > 0 ? arr[i]: min[i];

        ON_DEBUG(fprintf(stderr, "\n%2i. arr  : ", i););
        ON_DEBUG(PRINT_ARRAY(stderr, arr, 0, sz-1, m););
        ON_DEBUG(fprintf(stderr, "\n%2i. taken: ", i););
        ON_DEBUG(PRINT_ARRAY(stderr, taken, 1, sz-1, m););
        ON_DEBUG(fprintf(stderr, "\n%2i. max  : ", i););
        ON_DEBUG(PRINT_ARRAY(stderr, max, 0, sz-1, m););

        // Find first available number (must be in limits, not be taken and be valid)
        while (t <= imax && (taken[t] || BAD_NEIGHBORS(t,arr,i,m))) 
        {
            ON_DEBUG(fprintf(stderr, "\n...%i is taken or has bad neighbors.", t););
            t++;
        }

        if (t > imax)
        {
            if (arr[i] != 0)
            {
                ON_DEBUG(fprintf(stderr, "\n...drop %2i", arr[i]););
                taken[arr[i]] = 0;
            }
            ON_DEBUG(fprintf(stderr, "\n...decreasing i"););
            i -= 1;
        } else {
            if (arr[i] != 0)
            {
                ON_DEBUG(fprintf(stderr, "\n...drop %2i", arr[i]););
                taken[arr[i]] = 0;
            }
            
            if (i+1 < sz-1)
            {
                arr[i+1] = 0;
            }
            ON_DEBUG(fprintf(stderr, "\n...take %2i", t););
            arr[i] = t;
            taken[t] = 1;
            ON_DEBUG(fprintf(stderr, "\n...increasing i"););
            i += 1;
        }
        ON_DEBUG(getchar(););
    }

    return 0;
}
