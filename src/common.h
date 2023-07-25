#ifndef COMMON_H
#define COMMON_H

/*
 * Includes
 * */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/*
 * For Debugging
 * */
#define DEBUG 0
#define ON_DEBUG(s) if(DEBUG) { s }

/*
 * Shortcut for types. Following rust-like convention
 * */
typedef uint8_t  u8;
typedef uint32_t u32;


/*
 * General Macros
 **/

/*
 * PRINT_ARRAY (
 *  file descriptor (fd)
 *  array (A)
 *  start index of array (beg)
 *  end index of array inclusive (end)
 *  elements per row (m)
 *)
 * */
#define PRINT_ARRAY(fd, A, beg, end, m){\
    for (int j = beg; j < end; j++)\
    {\
        char c = ((j+1)%m == 0) ? ';': ',';\
        fprintf(fd, "%i%c", A[j], c);\
    }\
    fprintf(fd, "%i\n", A[end]);}

#define malloc_or_exit(ptr, sz, r) do {\
    ptr = malloc(sz);\
    if(ptr==NULL)\
    { fprintf(stderr, "\nFailed to allocate memory."); return r; }\
    } while(0)

/*
 * Algorithm Macros Utilities
 * */
#define BAD_NEIGHBORS(t,A,i,m) ((m<i && A[i-m]>=t) || (i%m>0 && A[i-1]>=t))

/*
 * File Utilities
 * */
#define open_or_exit(fd, filename, mode, r) do {\
    fd = fopen(filename, mode);\
    if (fd==NULL)\
    { fprintf(stderr, "\n"); return r; }\
    } while(0)

#endif
