#pragma once

/* ================== */
/* ==== Includes ==== */ 
/* ================== */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

/* ================ */
/* ==== MACROS ==== */ 
/* ================ */
#define NFMT "raw/Nc%ir%i"
#define PFMT "raw/Pc%ir%i"

#define INTWIDTH 6 

#define PANIK(...) do {\
  fprintf(stderr,\
  "\n[PANIK]\n\
  File     => %s\n\
  Location => %s:%i\n\
  Error    => ",\
  __FILE__, __func__, __LINE__);\
  fprintf(stderr, __VA_ARGS__);\
  exit(-1);\
} while(0) 

#define PANIKON(cond, ...) do {\
    if (cond) { PANIK(__VA_ARGS__); }\
} while(0)

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

#define REALLOC(ptr, sz)\
{\
  {void* holder = ptr;\
  ptr = realloc(holder, sz);\
  PANIKON(ptr==NULL, "realloc() failed.");\
  }\
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

/* ====================== */
/* ==== Data Structs ==== */ 
/* ====================== */
struct consumer_data_t {
  int i;
  pid_t pid;
  pthread_t listener;
  FILE *fs_w;
  FILE *fs_r;
};
