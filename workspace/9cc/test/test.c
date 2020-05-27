int g1;
int g2[5];
static int ext1 = 3;
extern int global_var;
char g3 = 3;
int g4 = 5;
int *g5 = &g4;
char *g6 = "abc";
int g7[3] = {0, 1, 2};
char *g8[] = {"foo", "bar"};
struct { char a; int b; } g9[2] = {{1, 2}, {3, 4}};
struct { int a[2]; } g10[2] = {{{1, 2}}};

typedef struct {
    int x;
    int y;
} HogeStruct;

int printf();
int fprintf();
int strcmp(char *p, char *q);
int exit();
int *alloc(int x);
int **alloc2(int x);
int* alloc4(int a, int b, int c, int d);
int*** alloc5(int** pp, int** pq);
void show(char* p);
void show_num(int i);

int assert(int expected, int actual, char *text) {
    if (actual == expected){
        printf("[ok] actual:%d expected: %d %s\n", actual, expected, text);
    } else {
        printf("[ng](expected: %d => but got: %d %s\n", expected, actual, text);
        exit(1);
    }
    return 0;
}

int fuga() {
    return 5;
}
int hoge() { return 20; }
int add1(int a) { return a; }
int add2(int a, int b) { return a + b; }
int add2_array(int (*a)[2]) { return (*a)[0] + (*a)[1]; }
int add3(int a, int b, int c) { return a + b + c; }
int add4(int a, int b, int c, int d) { return a + b + c + d; }
int add5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }
int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }
int plus(int x, int y) { return x + y; }
void nop() {}
static int static_fn() { return 3; }

int main() {
#include "test/test3.h"
    assert(0, 0, "0");
    assert(42, 42, "42");
    assert(48879, 0xBEEF, "0xBEEF");
    assert(255, 0xff, "0xff");
    assert(493, 0755, "0755");
    assert(0-42, -42, "-42");
    assert(42, (13 + 34 - 5), "(13 + 34 - 5)");
    assert(0, 1 - (4 - 1 - 2), "(4 - 1 - 2)");
    assert(21, 5+20-4, "5+20-4");
    assert(0, ({ 0; }), "({ 0; })");
    assert(42, ({ 42; }), "({ 42; })");
    assert(21,  ({ 5+20-4; }), " ({ 5+20-4; })");
    assert(41,  ({  12 + 34 - 5; }), " ({  12 + 34 - 5; })");
    assert(37,  ({ 5 * 6 + 7; }), " ({ 5 * 6 + 7; })");
    assert(48,  ({ 5 + 6 * 7 + 1; }), " ({ 5 + 6 * 7 + 1; })");
    assert(7,  ({ 5 + 14 / 7; }), " ({ 5 + 14 / 7; })");
    assert(3, 3 % 7, "3 % 7");
    assert(0, 7 % 7, "7 % 7");
    assert(4, 11 % 7, "11 % 7");
    assert(0, 14 % 7, "14 % 7");
    assert(50,  ({ 5 + 6 * 7 + 1 * 3; }), " ({ 5 + 6 * 7 + 1 * 3; })");
    assert(47,  ({ 5 + 6 * 7; }), " ({ 5 + 6 * 7; })");
    assert(47,  ({ 2 + 3 + 6 * 7; }), " ({ 2 + 3 + 6 * 7; })");
    assert(44,  ({ 6 / 3 + 6 * 7; }), " ({ 6 / 3 + 6 * 7; })");
    assert(9,  ({ (1 + 2) * 3; }), " ({ (1 + 2) * 3; })");
    assert(9,  ({ (1 + 2) * 3; }), " ({ (1 + 2) * 3; })");
    assert(1,  ({ 2 == 2; }), " ({ 2 == 2; })");
    assert(0,  ({ 2 == 3; }), " ({ 2 == 3; })");
    assert(0,  ({ 2 != 2; }), " ({ 2 != 2; })");
    assert(1,  ({ 2 != 3; }), " ({ 2 != 3; })");
    assert(1,  ({ 3 >= 2; }), " ({ 3 >= 2; })");
    assert(1,  ({ 2 >= 2; }), " ({ 2 >= 2; })");
    assert(0,  ({ 1 >= 2; }), " ({ 1 >= 2; })");
    assert(0,  ({ 3 <= 2; }), " ({ 3 <= 2; })");
    assert(1,  ({ 2 <= 2; }), " ({ 2 <= 2; })");
    assert(1,  ({ 1 <= 2; }), " ({ 1 <= 2; })");
    assert(1,  ({ 3 > 2; }), " ({ 3 > 2; })");
    assert(0,  ({ 2 > 2; }), " ({ 2 > 2; })");
    assert(0,  ({ 1 > 2; }), " ({ 1 > 2; })");
    assert(0,  ({ 3 < 2; }), " ({ 3 < 2; })");
    assert(0,  ({ 2 < 2; }), " ({ 2 < 2; })");
    assert(1,  ({ 1 < 2; }), " ({ 1 < 2; })");
    assert(5,  ({int a; int b; a = 3;b = 2; a + b; }), " ({int a; int b; a = 3;b = 2; a + b; })");
    assert(25,  ({ int d; int b; d = 3 * 2 - 1;b = 5; d * b; }), " ({ int d; int b; d = 3 * 2 - 1;b = 5; d * b; })");
    assert(25,  ({ int abc; int b; abc = 3 * 2 - 1;b = 5; abc * b; }), " ({ int abc; int b; abc = 3 * 2 - 1;b = 5; abc * b; })");
    assert(50,  ({ int abc; int b; abc = 3 * 2 - 1;b = abc + 5; abc * b; }), " ({ int abc; int b; abc = 3 * 2 - 1;b = abc + 5; abc * b; })");

    assert(3, ({ int x=0; if (0) x=2; else x=3; x; }), "({ int x=0; if (0) x=2; else x=3; x; })");
    assert(3, ({ int x=0; if (1-1) x=2; else x=3; x; }), "({ int x=0; if (1-1) x=2; else x=3; x; })");
    assert(2, ({ int x=0; if (1) x=2; else x=3; x; }), "({ int x=0; if (1) x=2; else x=3; x; })");
    assert(2, ({ int x=0; if (2-1) x=2; else x=3; x; }), "({ int x=0; if (2-1) x=2; else x=3; x; })");
    assert(2, ({ if (1 == 2) { 1; } else if (1 == 1) { 2; } else { 3; } }), "({ if (1 == 2) { 1; } else if (1 == 1) { 2; } else { 3; } })");

    printf("while...\n");
    assert(3, ({ 1; {2;} 3; }), "({ 1; {2;} 3; })");
    assert(10, ({ int i=0; i=0; while(i<10) i=i+1; i; }), "({ int i=0; i=0; while(i<10) i=i+1; i; })");
    assert(55, ({ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} j; }), "({ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} j; })");
    assert(1, ({ int x=0; while(1) { x++; break; } x; }), "({ int x=0; while(1) { x++; break; } x; })");
    assert(3, ({ int x=0; while(x < 3) { if (x <= 3) { x++; continue; } break; } x; }), "({ int x=0; while(x > 3) { if (x <= 3) { x++; continue; } break; } x; })");
    printf("for...\n");

// switch
    assert(6, ({ int x=0; switch(3) { case 2: x = 5; break; case 3: x=6; break; case 4: x=7; break;} x;}), "({ int x=0; switch(3) { case 2: x = 5; break; case 3: x=6; break; case 4: x=7; break;} x;})");
    assert(7, ({ int x=0; switch(3) { case 2: x = 5; case 3: x=6; case 4: x=7; } x;}), "({ int x=0; switch(3) { case 2: x = 5; case 3: x=6; case 4: x=7;} x;})");
    assert(0, ({ int x=0; switch(3) { case 1: x = 5; } x;}), "({ int x=0; switch(3) { case 1: x = 5; } x;})");

    assert(55, ({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; }), "({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; })");
    assert(6, ({ int i = 0; for (;;){ if (i > 5) { break; } i++; } i; }), "({ int i = 0; for (;;){ if (i > 5) { break; } i++} i; })");
    assert(4, ({int j = 0; for (int i = 0; i < 5; i++) { j = i; } j; }), "({int j = 0; for (int i = 0; i < 5; i++) { j = i; } j; })");
    assert(5, ({ int i=0; for (; i < 10; i++) { if (i==5) break; } i; }), "({ int i=0; for (; i < 10; i++) { if (i==5) break; } i; })");

    assert(7, ({ int i=0; for (int j=0; j < 10; j++) { if (j < 3) continue; i++; } i;}), "({ int i=0; for (int j=0; j < 10; j++) { if (j < 3) continue; i++; } i;})");
    assert(3, ({int x=0; if (1 == 0) { x=1; } else { x=3; } x; }), "({int x=0; if (1 == 0) { x=1; } else { x=3; } x; })");
    printf("function call...\n");
    assert(20,  ({ hoge(); }), " ({ hoge(); })");
    assert(30,  ({ int a; a = 30; }), " ({ int a; a = 30; })");
    assert(30,  ({ int a; int b; a = hoge(); b = 10; a + b; }), " ({ int a; int b; a = hoge(); b = 10; a + b; })");
    assert(25,  ({ int fuga; int hoge; fuga = fuga(); hoge = hoge(); fuga + hoge; }), " ({ int fuga; int hoge; fuga = fuga(); hoge = hoge(); fuga + hoge; })");
    assert(3,  ({ add1(3); }), " ({ add1(3); })");
    assert(7,  ({ add2(3, 4); }), " ({ add2(3, 4); })");
    assert(8, ({ int ary[2][2]; ary[0][0]=3; ary[0][1]=5; add2_array(ary); }), "({ int ary[2][2]; ary[0][0]=3; ary[0][1]=5; add2_array(ary); })");
    assert(8, ({ int ary[2][2]; ary[1][0]=3; ary[1][1]=5; add2_array(ary+1); }), "({ int ary[2][2]; ary[0][0]=3; ary[0][1]=5; add2_array(ary+t1); })");
    assert(12,  ({ add3(3, 4, 5); }), " ({ add3(3, 4, 5); })");
    assert(18,  ({ add4(3, 4, 5, 6); }), " ({ add4(3, 4, 5, 6); })");
    assert(25,  ({ add5(3, 4, 5, 6, 7); }), " ({ add5(3, 4, 5, 6, 7); })");
    assert(33,  ({ add6(3, 4, 5, 6, 7, 8); }), " ({ add6(3, 4, 5, 6, 7, 8); })");
    assert(36,  ({ add2(1, 2) + add6(3, 4, 5, 6, 7, 8); }), " ({ add2(1, 2) + add6(3, 4, 5, 6, 7, 8); })");
    assert(1, ({int a; int *p = &a; *p = 1;  *p;}), "({int a; int *p = &a; *p = 1;  *p;})");
    assert(39, ({ int **q; q = alloc2(39);  **q; }), "({ int **q; q = alloc2(39);  **q; })");
    assert(42, ({ int *p; p = alloc(42);  *p; }), "({ int *p; p = alloc(42);  *p; })");
    assert(22, ({ int *p; int **q; p = alloc(42); q = alloc2(22); p = *q;  *p; }), "({ int *p; int **q; p = alloc(42); q = alloc2(22); p = *q;  *p; })");
    assert(5,  ({ int x; int *p; p = &x; x = 5;  *p;}), " ({ int x; int *p; p = &x; x = 5;  *p;})");
    assert(3,  ({ int x; int y; int* p; int** q; y = 3; x = 5; p = &x; q = &p; *q = &y;  **q; }), " ({ int x; int y; int* p; int** q; y = 3; x = 5; p = &x; q = &p; *q = &y;  **q; })");
    assert(3,  ({ int x = 3;  x;}), " ({ int x = 3;  x;})");
    assert(3,  ({ int x = 3; int *y = &x;  *y;}), " ({ int x = 3; int *y = &x;  *y;})");
    assert(0,  ({  0&&0; }), " ({  0&&0; })");
    assert(0,  ({  1&&0; }), " ({  1&&0; })");
    assert(0,  ({  0&&1; }), " ({  0&&1; })");
    assert(1,  ({  1&&1; }), " ({  1&&1; })");
    assert(0,  ({  0||0; }), " ({  0||0; })");
    assert(1,  ({  1||0; }), " ({  1||0; })");
    assert(1,  ({  0||1; }), " ({  0||1; })");
    assert(1,  ({  1||1; }), " ({  1||1; })");
    assert(1,  ({  1||1; }), " ({  1||1; })");
    assert(1,  ({  1|1; }), " ({  1|1; })");
    assert(7,  ({  5|2; }), " ({  5|2; })");
    assert(15,  ({  8|5|2; }), " ({  8|5|2; })");
    assert(2,  ({  6 & 3; }), " ({ 6 & 3; })");
    assert(0,  ({  6 & 0; }), " ({ 6 & 0; })");
    assert(2,  ({  18 & 6 & 3; }), " ({  18 & 6 & 3; })");
    assert(12, (5 ^ 9), "5 ^ 9");
    assert(10, (5 ^ 6 ^ 9), "5 ^ 6 ^ 9");
    assert(12, (3<<2), "3<<2");
    assert(8, (1<<3), "1<<3");
    assert(4, (16>>2), "16>>2");
    assert(4,  ({ int *p; p = alloc4(1, 2, 4, 8); p = p + 2;   *p; }), " ({ int *p; p = alloc4(1, 2, 4, 8); p = p + 2;   *p; })");
    assert(8,  ({ int *p; p = alloc4(1, 2, 4, 8); p = p + 3;   *p; }), " ({ int *p; p = alloc4(1, 2, 4, 8); p = p + 3;   *p; })");
    assert(2,  ({ int *p; p = alloc4(1, 2, 4, 8); p = p + 3; p = p -2;   *p; }), " ({ int *p; p = alloc4(1, 2, 4, 8); p = p + 3; p = p -2;   *p; })");
// alloc
    assert(3, ({ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); tpp = tpp + 1;  ***tpp; }), "({ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); tpp = tpp + 1;  ***tpp; })");
    assert(8, ({ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); pp = pp + 3;  ***tpp; }), "({ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); pp = pp + 3;  ***tpp; })");
    assert(2, ({ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); **tpp = **tpp + 1;  ***tpp; }), "({ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); **tpp = **tpp + 1;  ***tpp; })");
// sizeof
    printf("sizeof\n");
    assert(4,  ({ sizeof(1); }), " ({ sizeof(1); })");
    assert(4,  ({ int x; sizeof(x); }), " ({ int x; sizeof(x); })");
    assert(8,  ({ int *x; sizeof(x); }), " ({ int *x; sizeof(x); })");
    assert(4,  ({ int *x; sizeof(*x); }), " ({ int *x; sizeof(*x); })");
    assert(4,  ({ sizeof 1; }), " ({ sizeof 1; })");
    assert(4,  ({ sizeof("abc"); }), " ({ sizeof(\"abc\"); })");
    assert(7, sizeof("abc" "def"), "sizeof(\"abc\" \"def\")");
    assert(9, sizeof("ab\0c" "\0def"), "sizeof(\"abc\" \"def\")");
    assert(1, sizeof(char), "sizeof(char)");
    assert(4, sizeof(int), "sizeof(int)");
    assert(8, sizeof(char*), "sizeof(char*)");
    assert(8, sizeof(int *), "sizeof(int*)");
    assert(8, sizeof(int **), "sizeof(int**)");
    assert(8, sizeof(int(*)[4]), "sizeof(int(*)[4])");
    assert(32, sizeof(int* [4]), "sizeof(int*[4])");
    assert(16, sizeof(int [4]), "sizeof(int*[4])");
    assert(48, sizeof(int [3][4]), "sizeof(int*[4])");
    assert(8, sizeof(struct {int a; int b;}), " sizeof(struct {int a; int b;})");


// array
    assert(5,  ({ int a[2]; *a = 2; *(a + 1) = 3; int *p; p = a;  *p + *(p + 1); }), " ({ int a[2]; *a = 2; *(a + 1) = 3; int *p; p = a;  *p + *(p + 1); })");
    assert(3,  ({ int a[2]; a[1] = 3;  a[1]; }), " ({ int a[2]; a[1] = 3;  a[1]; })");
    assert(12, ({ int a[2]; a[0] = 12;  a[0]; }), "({ int a[2]; a[0] = 12;  a[0]; })");
    assert(12, ({ int i = 0; int a[2]; a[0] = 12;  a[i]; }), "({ int i = 0; int a[2]; a[0] = 12;  a[i]; })");
    assert(12, ({ int a[2][2]; a[0][0] = 12;  a[0][0]; }), "({ int a[2][2]; a[0][0] = 12;  a[0][0]; })");

// Global variables
    assert(10, ({ g1 = 10;  g1; }), "({ g1 = 10;  g1; })");
    assert(15, ({  g2[0] = 5; g2[4] = 10;  g2[0] + g2[4]; }), "({  g2[0] = 5; g2[4] = 10;  g2[0] + g2[4]; })");
    assert(20,  ({  sizeof(g2); }), " ({  sizeof(g2); })");
    assert(20, ({  sizeof(g2); }), "({  sizeof(g2); })");

// char type
    assert(6, ({ char x[3];  { x[0] = 2; int y = 4;  x[0] + y; }}), "({ char x[3];  { x[0] = 2; int y = 4;  x[0] + y; }})");
    assert(6,  ({ char x[3]; x[0] = 2; int y = 4;  x[0] + y; }), " ({ char x[3]; x[0] = 2; int y = 4;  x[0] + y; })");

// char literal
    assert(97, 'a', "\'a\'");
    assert(59, ';', "\';\'");
    assert(10, '\n', "\'n\'");
    assert(92, '\\', "\'\\'");
    assert(0, '\0', "\'0\'");

// string literal
    assert(97,  ({ char *p = "abc";  p[0]; }), "({ char *p = \"abc\";  p[0]; })");
    assert(98,  ({ char *p = "abc";  p[1]; }), " ({ char *p = \"abc\";  p[1]; })");
    assert(99,  ({ char *p = "abc";  p[2]; }), " ({ char *p = \"abc\";  p[2]; })");
    assert(0,  ({ char *p = "abc";  p[3]; }), " ({ char *p = \"abc\";  p[3]; })");
    assert(0,  ({ char *p = "hello world"; show(p);  0; }), " ({ char *p = \"hello world\"; show(p);  0; })");

// struct
    assert(4,  ({ struct { int a; } x;  sizeof(x); }), " ({ struct { int a; } x;  sizeof(x); })");
    assert(8,  ({ struct { int a; int b; } x;  sizeof(x); }), " ({ struct { int a; int b; } x;  sizeof(x); })");
    assert(12,  ({ struct { char a; char b; int c; char d;} x;  sizeof(x); }), " ({ struct { char a; char b; int c; char d;} x;  sizeof(x); })");
    assert(2,  ({ struct { char a; char b; } x;  sizeof(x); }), " ({ struct { char a; char b; } x;  sizeof(x); })");
    assert(8,  ({ struct { char a; char b; int c; } x;  sizeof(x); }), " ({ struct { char a; char b; int c; } x;  sizeof(x); })");
    assert(4,  ({ struct { char a; char b; struct { char a; char b; } c; } x;  sizeof(x); }), " ({ struct { char a; char b; struct { char a; char b; } c; } x;  sizeof(x); })");
    assert(12,  ({ struct { char a; char b; struct { char a; int b; } c; } x;  sizeof(x); }), " ({ struct { char a; char b; struct { char a; int b; } c; } x;  sizeof(x); })");
    assert(3,  ({ struct { int a; } x; x.a = 3;  x.a; }), " ({ struct { int a; } x; x.a = 3;  x.a; })");
    assert(8,  ({ struct { int a; int b; } x; x.a = 3; x.b = 5;  (x.a + x.b); }), " ({ struct { int a; int b; } x; x.a = 3; x.b = 5;  (x.a + x.b); })");
    assert(8,  ({ struct { char a; int b; } x; x.a = 3; x.b = 5;  (x.a + x.b); }), " ({ struct { char a; int b; } x; x.a = 3; x.b = 5;  (x.a + x.b); })");
    assert(8,  ({ struct { char a; char b; } x; x.a = 3; x.b = 5;  (x.a + x.b); }), " ({ struct { char a; char b; } x; x.a = 3; x.b = 5;  (x.a + x.b); })");
    assert(18,  ({ struct { char a; char b; int c; } x; x.a = 3; x.b = 5; x.c = 10;  (x.a + x.b + x.c); }), " ({ struct { char a; char b; int c; } x; x.a = 3; x.b = 5; x.c = 10;  (x.a + x.b + x.c); })");
    assert(18,  ({ struct { char a; char b; int c; struct { char d; char e; } f;}  x; x.a = 3; x.b = 5; x.c = 10;  (x.a + x.b + x.c); }), " ({ struct { char a; char b; int c; struct { char d; char e; } f;}  x; x.a = 3; x.b = 5; x.c = 10;  (x.a + x.b + x.c); })");
    assert(8,  ({ struct tag { char a; int b; } x; struct tag *p = &x; x.a = 3; x.b = 5;  p->a + p->b; }), " ({ struct tag { char a; int b; } x; struct tag *p = &x; x.a = 3; x.b = 5;  p->a + p->b; })");
    assert(8,  ({ struct { char a; int b; } x; struct { char a; int b; } *p = &x; x.a=3; x.b=5;  p->a+p->b; }), " ({ struct { char a; int b; } x; struct { char a; int b; } *p = &x; x.a=3; x.b=5;  p->a+p->b; })");
    assert(48,  ({ struct { struct { int b; int c[5]; } a[2]; } x;  sizeof(x); }), " ({ struct { struct { int b; int c[5]; } a[2]; } x;  sizeof(x); })");
    assert(8,  ({ struct { struct { int b; int c[5]; } a[2]; } x; x.a[0].b = 3; x.a[0].c[1] = 5;  x.a[0].b + x.a[0].c[1]; }), " ({ struct { struct { int b; int c[5]; } a[2]; } x; x.a[0].b = 3; x.a[0].c[1] = 5;  x.a[0].b + x.a[0].c[1]; })");
    assert(4, ({ Point p; p.x = 4; p.y = 5; p.x; }), "({ Point p; p.x = 4; p.y = 5; p.x})");
    assert(4, ({ HogeStruct p; p.x = 4; p.y = 5; p.x; }), "({ HogeStruct p; p.x = 4; p.y = 5; p.x})");

// typedef
    assert(3,  ({ typedef int foo; foo x; x = 3;  x; }), " ({ typedef int foo; foo x; x = 3;  x; })");
    assert(3,  ({ typedef struct { int a; int b; } foo; foo x; x.a = 3;  x.a; }), " ({ typedef struct { int a; int b; } foo; foo x; x.a = 3;  x.a; })");

// !
    assert(0,  ({ !1; }), " ({ !1; })");
    assert(1,  ({ !0; }), " ({ !0; })");
    assert(1,  ({ !(3 > 4); }), " ({ !(3 > 4); })");
// ~
    assert(-1, ~0, "~0");
    assert(-4, ~3, "~3");

// function alignment
    assert(0,  ({ int e1; e1 = 0;  0;}), " ({ int e1; e1 = 0;  0;})");
    assert(0,  ({ printf("OK");  0;}), " ({ printf(\"OK\");  0;})");
    assert(0,  ({ int e1; e1 = 1; printf("OK");  0;}), " ({ int e1; e1 = 1; printf(\"OK\");  0;})");
    assert(0,  ({ int e1; e1 = 0; printf("OK");  0;}), " ({ int e1; e1 = 0; printf(\"OK\");  0;})");
// extern
    assert(5, ({ global_var; }), "global_var");
// ?:
    assert(42, ({ 5 == 1 ? 3 : 42; }), "({ 5 == 1 ? 3 : 42; })");
    assert(1, ({ 1 == 1 ? 1 : 42; }), "({ 1 == 1 ? 1 : 42; })");
    assert(1, ({ 1 == 1 ? 4 == 2 ? 32 : 1 : 42; }),"({ 1 == 1 ? 4 == 2 ? 32 : 1 : 42; })");
// ,operator
    assert(3, (1, 2, 3), "(1, 2, 3)");
// pre inc ++i
    assert(4, ({int i = 3; ++i;}), "({int i = 3; ++i;})");
    assert(4, ({int i = 3; int *j; j = &i; ++*j;}), "({int i = 3; int *j; j = &i; ++*j;})");
// pre dec --i
    assert(2, ({int i = 3; --i;}), "({int i = 3; --i;})");
    assert(2, ({int i = 3; int *j; j = &i; --*j;}), "({int i = 3; int *j; j = &i; --*j;})");
// post inc i++
    assert(3, ({int i = 3; i++;}), "({int i = 3; i++;})");
    assert(3, ({int i = 3; int *j; j = &i; *j++;}), "({int i = 3; int *j; j = &i; *j++;})");
    assert(1, ({ int ary[2]; ary[0]=1; ary[1]=2; int *p=ary; *p++;}), "({ int ary[2]; ary[0]=1; ary[1]=2; int *p=ary; return *p++;})");
    assert(2, ({ int ary[2]; ary[0]=1; ary[1]=2; int *p=ary; *++p;}), "({ int ary[2]; ary[0]=1; ary[1]=2; int *p=ary; return *++p;})");
// post inc i--
    assert(3, ({int i = 3; i--;}), "({int i = 3; i--;})");
    assert(3, ({int i = 3; int *j; j = &i; *j++;}), "({int i = 3; int *j; j = &i; *j++;})");
// break
    assert(5, ({int i = 0; while(i < 10) { if (i >= 5) { break; } i++; } i;}), "({int i = 0; while(i < 10) { if (i >= 5) { break; } i++; } i;})");
    assert(5, ({int i; for (i = 0; i < 10; i++) { if (i >= 5) { break; }} i;}), "({ for(int i = 0; i < 10; i++) { if (i >= 5) { break; }} i;})");
// assignment operator
    assert(20, ({int a = 4; a *= 5; a; }), "({int a = 4; a *= 5; a; })");
    assert(4, ({int a = 20; a /= 5; a; }), "({int a = 20; a /= 5; a; })");
    assert(2, ({int a = 5; a %= 3; a; }), "({int a = 5; a %= 3; a; })");
    assert(8, ({int a = 5; a += 3; a; }), "({int a = 5; a += 3; a; })");
    assert(2, ({int a = 5; a -= 3; a; }), "({int a = 5; a -= 3; a; })");
    assert(40, ({int a = 5; a <<= 3; a; }), "({int a = 5; a <<= 3; a; })");
    assert(5, ({int a = 40; a >>= 3; a; }), "({int a = 40; a >>= 3; a; })");
    assert(1, ({int a = 5; a &= 3; a; }), "({int a = 5; a &= 3; a; })");
    assert(6, ({int a = 5; a ^= 3; a; }), "({int a = 5; a ^= 3; a; })");
    assert(7, ({int a = 5; a |= 3; a; }), "({int a = 5; a |= 3; a; })");

    assert(3, ({ int x; int y; x=y=3; x; }), "({ int x; int y; x=y=3; x; })");
    assert(3, ({ int x; int y; x=y=3; y; }), "({ int x; int y; x=y=3; y; })");
// static
    assert(3, static_fn(), "static_fn()");
    assert(3, ext1, "ext1");

    int M1 = 5;
#define M1 3
    assert(3, M1, "M1");
#define M1 4
    assert(4, M1, "M1");
#undef M1
    assert(5, M1, "M1");

// _Bool
    assert(0, ({ _Bool x = 0; x; }), "({ _Bool x = 0; x; })");
    assert(1, ({ _Bool x = 1; x; }), "({ _Bool x = 1; x; })");
    assert(0, ({ _Bool x; x = 0; x; }), "({ _Bool x; x = 0; x; })");
    assert(1, ({ _Bool x; x = 2; x; }), "({ _Bool x; x = 2; x; })");
    assert(0, ({ _Bool x; int y = 0; x = y; x; }), "({ _Bool x; int y = 0; x = y; x; })");
    assert(1, ({ _Bool x; int y = -1; x = y; x; }), "({ _Bool x; int y = -1; x = y; x; })");
    assert(0, ({ _Bool x; int y = 0; x = y; x; }), "({ _Bool x; int y = 0; x = y; x; })");
    assert(1, ({ _Bool x; int y = 1; x = y; x; }), "({ _Bool x; int y = 1; x = y; x; })");
// local variable initialize
    assert(0, ({ int x[3] = {}; x[0]; }), "int x[3] = {}; x[0];");
    assert(0, ({ int x[3] = {}; x[1]; }), "int x[3] = {}; x[1];");
    assert(0, ({ int x[3] = {}; x[2]; }), "int x[3] = {}; x[2];");
    assert(1, ({ int x[3] = {1, 2, 3}; x[0]; }), "({ int x[3] = {1, 2, 3}; x[0]; })");
    assert(2, ({ int x[3] = {1, 2, 3}; x[1]; }), "({ int x[3] = {1, 2, 3}; x[0]; })");
    assert(3, ({ int x[3] = {1, 2, 3}; x[2]; }), "({ int x[3] = {1, 2, 3}; x[0]; })");
    assert(2, ({ int x[2][3]={{1,2,3},{4,5,6}}; x[0][1]; }), "({ int x[2][3]={{1,2,3},{4,5,6}}; x[0][1]; })");
    assert(4, ({ int x[2][3]={{1,2,3},{4,5,6}}; x[1][0]; }), "({ int x[2][3]={{1,2,3},{4,5,6}}; x[1][0]; })");
    assert(6, ({ int x[2][3]={{1,2,3},{4,5,6}}; x[1][2]; }), "({ int x[2][3]={{1,2,3},{4,5,6}}; x[1][2]; })");
    assert(2, ({ int x[2][3]={{1,2}}; x[0][1]; }), "int x[2][3]={{1,2}}; x[0][1];");
    assert(0, ({ int x[2][3]={{1,2}}; x[1][0]; }), "int x[2][3]={{1,2}}; x[1][0];");
    assert(0, ({ int x[2][3]={{1,2}}; x[1][2]; }), "int x[2][3]={{1,2}}; x[1][2];");
    assert('a', ({ char x[4] = "abc"; x[0]; }), "char x[4] = \"abc\"; x[0];");
    assert('c', ({ char x[4] = "abc"; x[2]; }), "char x[4] = \"abc\"; x[2];");
    assert(0, ({ char x[4] = "abc"; x[3]; }), "char x[4] = \"abc\"; x[3];");
    assert('a', ({ char x[2][4] = {"abc", "def"}; x[0][0]; }), "char x[2][4] = {\"abc\", \"def\"}; x[0][0];" );
    assert(0, ({ char x[2][4] = {"abc", "def"}; x[0][3]; }), "char x[2][4] = {\"abc\", \"def\"}; x[0][3];" );
    assert('d', ({ char x[2][4] = {"abc", "def"}; x[1][0]; }), "char x[2][4] = {\"abc\", \"def\"}; x[1][0];" );
    assert('f', ({ char x[2][4] = {"abc", "def"}; x[1][2]; }), "char x[2][4] = {\"abc\", \"def\"}; x[1][2];" );
    assert(4, ({ int x[] = {1, 2, 3, 4}; x[3]; }), "int x[] = {1, 2, 3, 4}; x[3];");
    assert(16, ({ int x[] = {1, 2, 3, 4}; sizeof(x); }), "int x[] = {1, 2, 3, 4}; sizeof(x);");
    assert(4, ({ char x[] = "foo"; sizeof(x); }), "char x[] = \"foo\"; sizeof(x);");

    assert(1, ({ struct { int a; int b; int c; } x = {1, 2, 3}; x.a; }), "struct { int a; int b; int c; } x = {1, 2, 3}; x.a;");
    assert(2, ({ struct { int a; int b; int c; } x = {1, 2, 3}; x.b; }), "struct { int a; int b; int c; } x = {1, 2, 3}; x.b;");
    assert(3, ({ struct { int a; int b; int c; } x = {1, 2, 3}; x.c; }), "struct { int a; int b; int c; } x = {1, 2, 3}; x.c;");
    assert(1, ({ struct { int a; int b; int c; } x = {1}; x.a; }), "struct { int a; int b; int c; } x = {1}; x.a;");
    assert(0, ({ struct { int a; int b; int c; } x = {1}; x.b; }), "struct { int a; int b; int c; } x = {1}; x.b;");
    assert(0, ({ struct { int a; int b; int c; } x = {1}; x.c; }), "struct { int a; int b; int c; } x = {1}; x.c;");
    assert(1, ({ struct {int a; int b;} x[2] = {{1, 2}, {3, 4}}; x[0].a; }), "struct {int a; int b;} x[2] = {{1, 2}, {3, 4}}; x[0].a;");
    assert(2, ({ struct {int a; int b;} x[2] = {{1, 2}, {3, 4}}; x[0].b; }), "struct {int a; int b;} x[2] = {{1, 2}, {3, 4}}; x[0].b;");
    assert(3, ({ struct {int a; int b;} x[2] = {{1, 2}, {3, 4}}; x[1].a; }), "struct {int a; int b;} x[2] = {{1, 2}, {3, 4}}; x[1].a;");
    assert(4, ({ struct {int a; int b;} x[2] = {{1, 2}, {3, 4}}; x[1].b; }), "struct {int a; int b;} x[2] = {{1, 2}, {3, 4}}; x[1].b;");

    assert(0, ({ struct {int a; int b;} x[2]={{1, 2}}; x[1].b; }), "struct {int a; int b;} x[2]={{1, 2}}; x[1].b;" );

    assert(0, ({ struct {int a; int b;} x={}; x.a; }), "struct {int a; int b;}; x={}; x.a;");
    assert(0, ({ struct {int a; int b;} x={}; x.b; }), "struct {int a; int b;}; x={}; x.b;");

    assert(3, g3, "g3");
    assert(5, g4, "g4");
    assert(5, *g5, "*g5");
    assert(0, strcmp(g6, "abc"), "strcmp(g6, \"abc\")");

    assert(0, g7[0], "g7[0]");
    assert(1, g7[1], "g7[1]");
    assert(2, g7[2], "g7[2]");

    assert(0, strcmp(g8[0], "foo"), "strcmp(g8[0], \"foo\")");
    assert(0, strcmp(g8[1], "bar"), "strcmp(g8[1], \"bar\")");
    assert(1, g9[0].a, "g9[0].a");
    assert(2, g9[0].b, "g9[0].b");
    assert(3, g9[1].a, "g9[1].a");
    assert(4, g9[1].b, "g9[1].b");

    assert(1, g10[0].a[0], "g10[0].a[0]");
    assert(2, g10[0].a[1], "g10[0].a[1]");
    assert(0, g10[1].a[0], "g10[1].a[0]");
    assert(0, g10[1].a[1], "g10[1].a[1]");

// enum
    assert(0, ({ enum { zero, one, two }; zero; }), "{ enum { zero, one, two }; zero; }");
    assert(1, ({ enum { zero, one, two }; one; }), "{ enum { zero, one, two }; one; }");
    assert(2, ({ enum { zero, one, two }; two; }), "{ enum { zero, one, two }; two; }");
    assert(5, ({ enum { five=5, six, seven }; five; }), "{ enum { five=5, six, seven }; five; }");
    assert(6, ({ enum { five=5, six, seven }; six; }), "enum { five=5, six, seven }; six;");
    assert(7, ({ enum { five=5, six, seven }; seven; }), "enum { five=5, six, seven }; seven;");
    assert(0, ({ enum { zero, five=5, three=3, four }; zero; }), "enum { zero, five=5, three=4, four }; zero;");
    assert(5, ({ enum { zero, five=5, three=3, four }; five; }), "enum { zero, five=5, three=4, four }; five;");
    assert(3, ({ enum { zero, five=5, three=3, four }; three; }), "enum { zero, five=5, three=4, four }; three;");
    assert(4, ({ enum { zero, five=5, three=3, four }; four; }), "enum { zero, five=5, three=4, four }; four;");
    assert(4, ({ enum { zero, one, two} x; sizeof(x); }), "{ enum { zero, one, two} x; sizeof(x); }");
    assert(4, ({ enum t { zero, one, two }; enum t y; sizeof(y); }), "{ enum { zero, one, two} x; sizeof(x); }");


    printf("OK\n");
    return 0;
}
