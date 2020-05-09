#include <stdio.h>
#include <stdlib.h>

int global_var = 5;

int *alloc(int x) {
  static int arr[1];
  arr[0] = x;
  return arr;
}

int **alloc2(int x) {
    int p = x;
    int* q = &p;
    int** r = &q;
    return r;
}

int* alloc4(int a, int b, int c, int d) {
    int* q = calloc(4, sizeof(int));
    q[0] = a;
    q[1] = b;
    q[2] = c;
    q[3] = d;
    return q;
}

int*** alloc5(int** pp, int** pq) {
    int*** s = calloc(2, sizeof(int**));
    s[0] = pp;
    s[1] = pq;
    return  s;
}

void show(char* p) {
    printf("%s\n", p);
}

void show_num(int i) {
    printf("%d\n", i);
}
