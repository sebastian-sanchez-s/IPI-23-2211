#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#define BANNEDFMT "banned/c%ir%i"
#define NFMT "raw/Nc%ir%i"
#define PFMT "raw/Pc%ir%i"

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

#define MALLOC(ptr, sz) do {\
  ptr = malloc(sz);\
  PANIKON(ptr==NULL, "malloc() failed.");\
} while(0)

#define CALLOC(ptr, n, sz) do {\
  ptr = calloc(n, sz);\
  PANIKON(ptr==NULL, "calloc() failed.");\
} while(0)

#define REALLOC(ptr, sz) do {\
  ptr = realloc(ptr, sz);\
  PANIKON(ptr==NULL, "realloc() failed.");\
} while(0)

#define PRINTARR(f, a, si, sz) do {\
  for(int _k=si; _k<si+sz-1; _k++)\
  { fprintf(f, "%i ", a[_k]); }\
  fprintf(f, "%i\n", a[si+sz-1]);\
} while(0)

#define READARR(f, a, si, sz) do {\
  for(int _k=si; _k<si+sz; _k++)\
  { fscanf(f, "%i", &a[_k]); }\
} while(0)

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
