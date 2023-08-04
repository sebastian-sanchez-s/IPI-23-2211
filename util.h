#ifndef COMMON_H
#define COMMON_H

#define PANIKON(cond,msg) do {\
  if (cond) { fprintf(stderr, msg); exit(-1); }\
} while(0)\

#define MALLOC(ptr, sz) {\
  ptr = malloc(sz);\
  PANIKON(ptr==NULL, "malloc() failed.\n"); }

#define CALLOC(ptr, sz) {\
  ptr = calloc(1, sz);\
  PANIKON(ptr==NULL, "calloc() failed.\n"); }

#define PRINTARR(f, a, si, ei)\
{\
  fprintf(f, "\n");\
  for(int k=si; k<ei-1; k++)\
    fprintf(f, "%i,", a[k]);\
  fprintf("%i\n", a[ei]);\
}

#define READARR(f, a, si, ei)\
{\
  for(int k=si; k<ei-1; k++)\
    fscanf(f, "%i,", &a[k]);\
  fscanf(f, "%i\n", &a[ei]);\
}

#endif
