#!/bin/bash
try() {
    input="$2"
    echo "$input"
    echo "$input" > tmp.c
    try_file $1 tmp.c
}

try_file() {
    expected="$1"
    filename="$2"

    ./9cc $filename > tmp.s
    gcc -static -o tmp tmp.s tmp-test.o tmp-hoge.o tmp-fuga.o tmp-add1.o tmp-add2.o tmp-add3.o tmp-add4.o tmp-add5.o tmp-add6.o
    ./tmp
    actual="$?"

    if [ "$actual" == "$expected" ]; then
        echo "=> $actual"
    else
        echo "$expected expeted, but got $actual"
        exit 1
    fi
}

echo 'int fuga() { return 5; }' | gcc -xc -c -o tmp-fuga.o -
echo 'int hoge() { return 20; }' | gcc -xc -c -o tmp-hoge.o -
echo 'int add1(int a) { return a; }' | gcc -xc -c -o tmp-add1.o -
echo 'int add2(int a, int b) { return a + b; }' | gcc -xc -c -o tmp-add2.o -
echo 'int add3(int a, int b, int c) { return a + b + c; }' | gcc -xc -c -o tmp-add3.o -
echo 'int add4(int a, int b, int c, int d) { return a + b + c + d; }' | gcc -xc -c -o tmp-add4.o -
echo 'int add5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }' | gcc -xc -c -o tmp-add5.o -
echo 'int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }' | gcc -xc -c -o tmp-add6.o -

cat <<EOF | gcc -xc -c -o tmp-test.o -
#include <stdio.h>
#include <stdlib.h>
int plus(int x, int y) { return x + y; }
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
    int*** s = calloc(2, sizeof(int**)); //= { a, b };
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

EOF

try_file 45 test/comment.c
try 0 "int main() { 0; }"
try 42 "int main() { 42; }"
try 21 "int main() { 5+20-4; }"
try 41 "int main() {  12 + 34 - 5; }"
try 37 "int main() { 5 * 6 + 7; }"
try 48 "int main() { 5 + 6 * 7 + 1; }"
try 7 "int main() { 5 + 14 / 7; }"
try 50 "int main() { 5 + 6 * 7 + 1 * 3; }"
try 47 "int main() { 5 + 6 * 7; }"
try 47 "int main() { 2 + 3 + 6 * 7; }"
try 44 "int main() { 6 / 3 + 6 * 7; }"
try 9 "int main() { (1 + 2) * 3; }"
try 9 "int main() { 1 + 2;(1 + 2) * 3; }"
try 1 "int main() { 2 == 2; }"
try 0 "int main() { 2 == 3; }"
try 0 "int main() { 2 != 2; }"
try 1 "int main() { 2 != 3; }"
try 1 "int main() { 3 >= 2; }"
try 1 "int main() { 2 >= 2; }"
try 0 "int main() { 1 >= 2; }"
try 0 "int main() { 3 <= 2; }"
try 1 "int main() { 2 <= 2; }"
try 1 "int main() { 1 <= 2; }"
try 1 "int main() { 3 > 2; }"
try 0 "int main() { 2 > 2; }"
try 0 "int main() { 1 > 2; }"
try 0 "int main() { 3 < 2; }"
try 0 "int main() { 2 < 2; }"
try 1 "int main() { 1 < 2; }"
try 5 "int main() {int a; int b; a = 3;b = 2; a + b; }"
try 25 "int main() { int d; int b; d = 3 * 2 - 1;b = 5; d * b; }"
try 2 "int main() { return 2; }"
try 5 "int main() { return 5; return 6; }"
try 10 "int main() { return 1 * 2 * 3 + 4; }"
try 25 "int main() { int abc; int b; abc = 3 * 2 - 1;b = 5; abc * b; }"
try 4 "int main() { int a; int b; a = 1;b = 2; a= 2;return a * b; }"
try 50 "int main() { int abc; int b; abc = 3 * 2 - 1;b = abc + 5; abc * b; }"
try 6 "int main() { int foo; int bar; foo = 1; bar = 2 + 3;return foo + bar; }"
try 40 "int main() { if (2 == 2) return 40; }"
try 0 "int main() { if (1 == 2) return 40; }"
try 3 "int main() { int a; if(a = 3) return 3; }"
try 0 "int main() { int a; if(a = 0) return 3; }"
try 10 "int main() { int a; int b; int c; int d; int e;  a = 1; b = 2; c = 3; d = 2; e = 2; if(b == d) if(b == e) return 10; }"
try 0 "int main() { int a; int b; int c; int d; int e; a = 1; b = 2; c = 3; d = 2; e = 2; if(b == d) if(a == e) return 0; }"
try 0 "int main() { int a; int b; int c; int d; int e; a = 1; b = 2; c = 3; d = 2; e = 2; if(a == d) if(a == e) return 0; }"
try 5 "int main() { int a; a = 1; a = a + 4;return a; }"
try 6 "int main() { int a; a = 1; while(a != 5) a = a + 1; return 6; }"
try 5 "int main() { int a; a = 1; while(a != 5) a = a + 1; return a; }"

try 11 "int main() { int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) b = b + 2; return b; }"
try 5 "int main() { int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) b = b + 2; return i; }"
# block
try 5 "int main() { int a; int b; a = 1; while(a < 5) { a = a + 1; b = 2; } return a; }"
try 5 "int main() { int a; int b; a = 1; while(a < 5) { b = 2; a = a + 1; } return a; }"
try 2 "int main() { int a; int b; a = 1; while(a < 5) { b = 2; a = a + 1; return a; } return a; }"
try 11 "int main() { int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) { b = b + 2; } return b; }"
try 5 "int main() { int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) { b = b + 2; } return i; }"
try 0 "int main() { if (1 == 2) { return 40; } }"
try 3 "int main() { int a; if(a = 3) { return 3; } }"
try 0 "int main() { int a; if(a = 0) { return 3; } }"
try 3 "int main() { int a; int b; a = 2; if (a != 1) { b = 2; a = 3; } return a; }"
try 3 "int main() { int a; int b; a = 1; if (a == 1) { b = 4; a = 3; } return a; }"
try 4 "int main() { int a; int b; a = 1; if (a == 1) { b = 4; a = 3; } return b; }"
# function call
try 20 "int main() { return hoge(); }"
try 30 "int main() { int a; int b; a = hoge(); b = 10; return a + b; }"
try 25 "int main() { int fuga; int hoge; fuga = fuga(); hoge = hoge(); return fuga + hoge; }"
try 3 "int main() { return add1(3); }"
try 7 "int main() { return add2(3, 4); }"
try 12 "int main() { return add3(3, 4, 5); }"
try 18 "int main() { return add4(3, 4, 5, 6); }"
try 25 "int main() { return add5(3, 4, 5, 6, 7); }"
try 33 "int main() { return add6(3, 4, 5, 6, 7, 8); }"
try 36 "int main() { return add2(1, 2) + add6(3, 4, 5, 6, 7, 8); }"
try 1 'int one() { return 1; } int main() { return one(); }'
try 3 'int one() { return 1; } int two() { return 2; } int main() { return one()+two(); }'
try 7 'int add(int x, int y) { return x + y; } int main() { return add(3, 4); }'
try 1 'int local(int a) { a = 3; return 3; } int main() { int a = 1; local(a); return a;}'
try 1 'int main() {int *p; *p = 1; return *p;}'
try 55 'int fib(int x) {    if (x == 1) {        return 1;    }    if (x == 2) {        return 1;    }    if (x >= 3) {        return fib(x - 1) + fib(x - 2);    }    return 0;}int main () {    return fib(10);}'
try 39 'int main() { int **q; q = alloc2(39); return **q; }'
try 42 'int main() { int *p; p = alloc(42); return *p; }'
try 22 'int main() { int *p; int **q; p = alloc(42); q = alloc2(22); p = *q; return *p; }'
try 5 'int main() { int x; int *p; p = &x; x = 5; return *p;}'
try 3 'int main() { int x; int y; int* p; int** q; y = 3; x = 5; p = &x; q = &p; *q = &y; return **q; }'
try 3 "int main() { int x = 3; return x;}"
try 3 "int main() { int x = 3; int *y = &x; return *y;}"
try 0 'int main() { return 0&&0; }'
try 0 'int main() { return 1&&0; }'
try 0 'int main() { return 0&&1; }'
try 1 'int main() { return 1&&1; }'

try 0 'int main() { return 0||0; }'
try 1 'int main() { return 1||0; }'
try 1 'int main() { return 0||1; }'
try 1 'int main() { return 1||1; }'

try 4 "int main() { int *p; p = alloc4(1, 2, 4, 8); p = p + 2;  return *p; }"
try 8 "int main() { int *p; p = alloc4(1, 2, 4, 8); p = p + 3;  return *p; }"
try 2 "int main() { int *p; p = alloc4(1, 2, 4, 8); p = p + 3; p = p -2;  return *p; }"
# alloc

try 3 "int main(){ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); tpp = tpp + 1; return ***tpp; }"
try 8 "int main(){ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); pp = pp + 3; return ***tpp; }"
try 2 "int main(){ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); **tpp = **tpp + 1; return ***tpp; }"
# sizeof
try 4 "int main() { return sizeof(1); }"
try 4 "int main() { int x; return sizeof(x); }"
try 8 "int main() { int *x; return sizeof(x); }"
try 4 "int main() { int *x; return sizeof(*x); }"
try 4 "int main() { return sizeof 1; }"

# array
try 5 "int main () { int a[2]; *a = 2; *(a + 1) = 3; int *p; p = a; return *p + *(p + 1); }"
try 3 "int main () { int a[2]; a[1] = 3; return a[1]; }"
try 12 "int main () { int a[2]; a[0] = 12; return a[0]; }"
try 12 "int main () { int i = 0; int a[2]; a[0] = 12; return a[i]; }"
try 12 "int main () { int a[2][2]; a[0][0] = 12; return a[0][0]; }"

# Global variables
try 10 "int x; int main() { x = 10; return x; }"
try 0 'int x; int main() { return x; }'
try 5 'int x; int main() { x = 5; return x; }'
try 15 'int x[5]; int main() { x[0] = 5; x[4] = 10; return x[0] + x[4]; }'
try 20 "int main() { int x[5]; return sizeof(x); }"
try 20 'int x[5]; int main() { return sizeof(x); }'

# char type
try 6 "char x[3]; int main() { x[0] = 2; int y = 4; return x[0] + y; }"
try 6 "int main() { char x[3]; x[0] = 2; int y = 4; return x[0] + y; }"

# string literal
try 97 'int main() { char *p = "abc"; return p[0]; }'
try 98 'int main() { char *p = "abc"; return p[1]; }'
try 99 'int main() { char *p = "abc"; return p[2]; }'
try 0 'int main() { char *p = "abc"; return p[3]; }'
try 0 'int main() { char *p = "hello world"; show(p); return 0; }'

# struct
try 4 "int main() { struct { int a; } x; return sizeof(x); }"
try 8 "int main() { struct { int a; int b; } x; return sizeof(x); }"
try 12 "int main() { struct { char a; char b; int c; char d;} x; return sizeof(x); }"
try 2 "int main() { struct { char a; char b; } x; return sizeof(x); }"
try 8 "int main() { struct { char a; char b; int c; } x; return sizeof(x); }"
try 4 "int main() { struct { char a; char b; struct { char a; char b; } c; } x; return sizeof(x); }"
try 12 "int main() { struct { char a; char b; struct { char a; int b; } c; } x; return sizeof(x); }"
try 3 "int main() { struct { int a; } x; x.a = 3; return x.a; }"
try 8 "int main() { struct { int a; int b; } x; x.a = 3; x.b = 5; return (x.a + x.b); }"
try 8 "int main() { struct { char a; int b; } x; x.a = 3; x.b = 5; return (x.a + x.b); }"
try 8 "int main() { struct { char a; char b; } x; x.a = 3; x.b = 5; return (x.a + x.b); }"
try 18 "int main() { struct { char a; char b; int c; } x; x.a = 3; x.b = 5; x.c = 10; return (x.a + x.b + x.c); }"
try 18 "int main() { struct { char a; char b; int c; struct { char d; char e; } f;}  x; x.a = 3; x.b = 5; x.c = 10; return (x.a + x.b + x.c); }"
try 8 "int main() { struct tag { char a; int b; } x; struct tag *p = &x; x.a = 3; x.b = 5; return p->a + p->b; }"
try 8 "int main() { struct { char a; int b; } x; struct { char a; int b; } *p = &x; x.a=3; x.b=5; return p->a+p->b; }"
try 48 "int main() { struct { struct { int b; int c[5]; } a[2]; } x; return sizeof(x); }"
try 8 "int main() { struct { struct { int b; int c[5]; } a[2]; } x; x.a[0].b = 3; x.a[0].c[1] = 5; return x.a[0].b + x.a[0].c[1]; }"

# typedef
try 3 "int main() { typedef int foo; foo x; x = 3; return x; }"
try 3 "int main() { typedef struct { int a; int b; } foo; foo x; x.a = 3; return x.a; }"

# void
try 1 "void test() { show_num(1); } int main() { test(); return 1; }"

# !
try 0 "int main() { !1; }"
try 1 "int main() { !0; }"
try 1 "int main() { !(3 > 4)}"


echo OK
