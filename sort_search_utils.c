#include "common.h"
#include "sort_search_utils.h"

int i_bsearch(int sz, int arr[sz], int val)
{
  int i;
  if (i_bsearchi(sz, arr, val, &i)) return i;
  else return -1;
}

int i_bsearchi(int sz, int arr[sz], int val, int *i)
{
  int beg = 0;
  int end = sz-1;
  int mid = end/2;
  
  while (beg <= end)
  {
    int d = arr[mid] - val;

    if (d < 0) { beg = mid + 1; } 
    else if (d > 0) { end = mid - 1; }
    else break;

    mid = (beg+end)/2;
  }

  *i = mid;
  return arr[mid] == val;
}

int i_lsearch(int sz, int arr[sz], int val)
{
  int i;
  if (i_lsearchi(sz, arr, val, &i)) return i;
  else return -1;
}

int i_lsearchi(int sz, int arr[sz], int val, int *i)
{
  for (int j=0; j<sz; j++)
  {
    if (arr[j] == val) 
    {
      *i = j;
      return 1;
    }
  }
  i = NULL;
  return 0;
}

int i_cmp_gt(const void *n, const void *m) 
{ return (*(int*)n) - (*(int*)m); }

int *i_sorted(int sz, int arr[sz])
{
  int *arr_cpy; MALLOC(arr_cpy, sizeof(int[sz]));

  memcpy(arr_cpy, arr, sizeof(int[sz]));
  
  qsort(arr_cpy, sz, sizeof(int), i_cmp_gt);
  
  return arr_cpy;
}

