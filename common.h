#ifndef COMMON_H
#define COMMON_H

/* ==================
 *      Includes
 * ==================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>


/* =================
 *      MACROS
 * =================*/
#define malloc_or(ptr, sz, callback) do {\
    ptr = malloc(sz);\
    if(ptr==NULL)\
    { fprintf(stderr, "\nFailed to allocate memory."); callback; }\
    } while(0)

#define open_or(fd, filename, mode, callback) do {\
    fd = fopen(filename, mode);\
    if (fd==NULL)\
    { fprintf(stderr, "\n"); callback; }\
    } while(0)

#endif
