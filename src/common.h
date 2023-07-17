#ifndef COMMON_H
#define COMMON_H

#define ON_DEBUG(s) if(DEBUG) { s }

#define PRINT_ARRAY(fd, A, beg, end, m){\
    for (int j = beg; j < end; j++)\
    {\
        char c = ((j+1)%m == 0) ? ';': ',';\
        fprintf(fd, "%i%c", A[j], c);\
    }\
    fprintf(fd, "%i\n", A[end]);}

#define BAD_NEIGHBORS(t,A,i,m) ((m<i && A[i-m]>=t) || (i%m>0 && A[i-1]>=t))

#endif
