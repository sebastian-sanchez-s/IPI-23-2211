#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

/**************************
* MACROS
**************************/
#define INTWIDTH 6 

#define PANIKON(cond,...)\
{\
  if (cond) {\
    if (errno) fprintf(stderr, "%s", strerror(errno));\
    fprintf(stderr, "PANIK[%i]: ", errno);\
    fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");\
    exit(-1);\
  }\
}

#define MALLOC(ptr, sz)\
{\
  ptr = malloc(sz);\
  PANIKON(ptr==NULL, "malloc() failed.");\
}

#define CALLOC(ptr, n, sz)\
{\
  ptr = calloc(n, sz);\
  PANIKON(ptr==NULL, "calloc() failed.");\
}

#define PRINTARR(f, a, si, sz)\
{\
  for(int k=si; k<si+sz-1; k++)\
  { fprintf(f, "%i ", a[k]); }\
  fprintf(f, "%i\n", a[si+sz-1]);\
}

#define READARR(f, a, si, sz)\
{\
  for(int k=si; k<si+sz; k++)\
  { fscanf(f, "%i", &a[k]); }\
}

#define _debug_flag 0
#define _debug(...) if(_debug_flag) { fprintf(stderr, __VA_ARGS__); }
#define _debugC(...) _debug("[CONSUMER] " __VA_ARGS__);
#define _debugP(...) _debug("[PRODUCER] " __VA_ARGS__);

/******************
 * DATA STRUCTURES
 * ****************/
struct consumer_data_t {
  pid_t pid; // process id
  int i; // index in array
  FILE *fs_w; // file stream to write
  FILE *fs_r; // file stream to read
  pthread_t listener;
};

#endif
