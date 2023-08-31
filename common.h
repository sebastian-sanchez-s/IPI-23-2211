/****************************************************************************
 * Copyright (C) 2023 by Sebastián Sánchez                                                      
 *                                                                          
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

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
  { if( fscanf(f, "%i", &a[_k]) == 1 ) continue; }\
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
