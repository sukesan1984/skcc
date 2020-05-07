int assert(int actual, int expected) {
    if (actual == expected){
        printf("[ok] %d\n", actual);
    }
    if (actual != expected) {
        printf("[ng] expected: %d => but got: %d\n", expected, actual);
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
int add3(int a, int b, int c) { return a + b + c; }
int add4(int a, int b, int c, int d) { return a + b + c + d; }
int add5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }
int add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }
int plus(int x, int y) { return x + y; }

int main() {
    assert(0, 0);
    assert(42, 42);
    assert(42, (13 + 34 - 5));
    assert(21, 5+20-4);
    assert(0, ({ 0; }));
    assert(4, ({ 42; }));
    assert(21,  ({ 5+20-4; }));
    assert(41,  ({  12 + 34 - 5; }));
    assert(37,  ({ 5 * 6 + 7; }));
    assert(48,  ({ 5 + 6 * 7 + 1; }));
    assert(7,  ({ 5 + 14 / 7; }));
    assert(50,  ({ 5 + 6 * 7 + 1 * 3; }));
    assert(47,  ({ 5 + 6 * 7; }));
    assert(47,  ({ 2 + 3 + 6 * 7; }));
    assert(44,  ({ 6 / 3 + 6 * 7; }));
    assert(9,  ({ (1 + 2) * 3; }));
    assert(9,  ({ 1 + 2;(1 + 2) * 3; }));
    assert(1,  ({ 2 == 2; }));
    assert(0,  ({ 2 == 3; }));
    assert(0,  ({ 2 != 2; }));
    assert(1,  ({ 2 != 3; }));
    assert(1,  ({ 3 >= 2; }));
    assert(1,  ({ 2 >= 2; }));
    assert(0,  ({ 1 >= 2; }));
    assert(0,  ({ 3 <= 2; }));
    assert(1,  ({ 2 <= 2; }));
    assert(1,  ({ 1 <= 2; }));
    assert(1,  ({ 3 > 2; }));
    assert(0,  ({ 2 > 2; }));
    assert(0,  ({ 1 > 2; }));
    assert(0,  ({ 3 < 2; }));
    assert(0,  ({ 2 < 2; }));
    assert(1,  ({ 1 < 2; }));
    assert(5,  ({int a; int b; a = 3;b = 2; a + b; }));
    assert(25,  ({ int d; int b; d = 3 * 2 - 1;b = 5; d * b; }));
    assert(2,  ({ return 2; }));
    assert(5,  ({ return 5; return 6; }));
    assert(10,  ({ return 1 * 2 * 3 + 4; }));
    assert(25,  ({ int abc; int b; abc = 3 * 2 - 1;b = 5; abc * b; }));
    assert(4,  ({ int a; int b; a = 1;b = 2; a= 2;return a * b; }));
    assert(50,  ({ int abc; int b; abc = 3 * 2 - 1;b = abc + 5; abc * b; }));
    assert(6,  ({ int foo; int bar; foo = 1; bar = 2 + 3; return foo + bar; }));

    assert(40,  ({ if (2 == 2) return 40; }));
    assert(0,  ({ if (1 == 2) return 40; }));
    assert(3,  ({ int a; if(a = 3) return 3; }));
    assert(0,  ({ int a; if(a = 0) return 3; }));
    assert(10,  ({ int a; int b; int c; int d; int e;  a = 1; b = 2; c = 3; d = 2; e = 2; if(b == d) if(b == e) return 10; }));
    assert(0,  ({ int a; int b; int c; int d; int e; a = 1; b = 2; c = 3; d = 2; e = 2; if(b == d) if(a == e) return 0; }));
    assert(0,  ({ int a; int b; int c; int d; int e; a = 1; b = 2; c = 3; d = 2; e = 2; if(a == d) if(a == e) return 0; }));
    assert(5,  ({ int a; a = 1; a = a + 4;return a; }));
    assert(6,  ({ int a; a = 1; while(a != 5) a = a + 1; return 6; }));
    assert(5,  ({ int a; a = 1; while(a != 5) a = a + 1; return a; }));
// ok
    assert(11,  ({ int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) b = b + 2; return b; }));
    assert(5,  ({ int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) b = b + 2; return i; }));
// block
    assert(5,  ({ int a; int b; a = 1; while(a < 5) { a = a + 1; b = 2; } return a; }));
    assert(5,  ({ int a; int b; a = 1; while(a < 5) { b = 2; a = a + 1; } return a; }));
    assert(2,  ({ int a; int b; a = 1; while(a < 5) { b = 2; a = a + 1; return a; } return a; }));
    assert(11,  ({ int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) { b = b + 2; } return b; }));
    assert(5,  ({ int b; int i; b = 3; for (i = 1; i < 5; i = i + 1) { b = b + 2; } return i; }));
    assert(0,  ({ if (1 == 2) { return 40; } }));
    assert(3,  ({ int a; if(a = 3) { return 3; } }));
    assert(0,  ({ int a; if(a = 0) { return 3; } }));
    assert(3,  ({ int a; int b; a = 2; if (a != 1) { b = 2; a = 3; } return a; }));
    assert(3,  ({ int a; int b; a = 1; if (a == 1) { b = 4; a = 3; } return a; }));
    assert(4,  ({ int a; int b; a = 1; if (a == 1) { b = 4; a = 3; } return b; }));
// function call
    assert(20,  ({ return hoge(); }));
    assert(30,  ({ int a; int b; a = hoge(); b = 10; return a + b; }));
    assert(25,  ({ int fuga; int hoge; fuga = fuga(); hoge = hoge(); return fuga + hoge; }));
    assert(3,  ({ return add1(3); }));
    assert(7,  ({ return add2(3, 4); }));
    assert(12,  ({ return add3(3, 4, 5); }));
    assert(18,  ({ return add4(3, 4, 5, 6); }));
    assert(25,  ({ return add5(3, 4, 5, 6, 7); }));
    assert(33,  ({ return add6(3, 4, 5, 6, 7, 8); }));
    assert(36,  ({ return add2(1, 2) + add6(3, 4, 5, 6, 7, 8); }));
    assert(1, ({int *p; *p = 1; return *p;}));
    assert(39, ({ int **q; q = alloc2(39); return **q; }));
    assert(42, ({ int *p; p = alloc(42); return *p; }));
    assert(22, ({ int *p; int **q; p = alloc(42); q = alloc2(22); p = *q; return *p; }));
    assert(5,  ({ int x; int *p; p = &x; x = 5; return *p;}));
    assert(3,  ({ int x; int y; int* p; int** q; y = 3; x = 5; p = &x; q = &p; *q = &y; return **q; }));
    assert(3,  ({ int x = 3; return x;}));
    assert(3,  ({ int x = 3; int *y = &x; return *y;}));
    assert(0,  ({ return 0&&0; }));
    assert(0,  ({ return 1&&0; }));
    assert(0,  ({ return 0&&1; }));
    assert(1,  ({ return 1&&1; }));
    assert(0,  ({ return 0||0; }));
    assert(1,  ({ return 1||0; }));
    assert(1,  ({ return 0||1; }));
    assert(1,  ({ return 1||1; }));
    assert(4,  ({ int *p; p = alloc4(1, 2, 4, 8); p = p + 2;  return *p; }));
    assert(8,  ({ int *p; p = alloc4(1, 2, 4, 8); p = p + 3;  return *p; }));
    assert(2,  ({ int *p; p = alloc4(1, 2, 4, 8); p = p + 3; p = p -2;  return *p; }));
// alloc
    assert(3, ({ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); tpp = tpp + 1; return ***tpp; }));
    assert(8, ({ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); pp = pp + 3; return ***tpp; }));
    assert(2, ({ int* pp = alloc4(1, 2, 4, 8); int* pq = alloc4(3, 5, 7, 9); int*** tpp = alloc5(&pp, &pq); **tpp = **tpp + 1; return ***tpp; }));
// sizeof
    assert(4,  ({ return sizeof(1); }));
    assert(4,  ({ int x; return sizeof(x); }));
    assert(8,  ({ int *x; return sizeof(x); }));
    assert(4,  ({ int *x; return sizeof(*x); }));
    assert(4,  ({ return sizeof 1; }));

// array
    assert(5,  ({ int a[2]; *a = 2; *(a + 1) = 3; int *p; p = a; return *p + *(p + 1); }));
    assert(3,  ({ int a[2]; a[1] = 3; return a[1]; }));
    assert(12, ({ int a[2]; a[0] = 12; return a[0]; }));
    assert(12, ({ int i = 0; int a[2]; a[0] = 12; return a[i]; }));
    assert(12, ({ int a[2][2]; a[0][0] = 12; return a[0][0]; }));

// Global variables
    assert(10, ({ int x;  { x = 10; return x; } }));
    assert(15, ({ int x[5];  { x[0] = 5; x[4] = 10; return x[0] + x[4]; }}));
    assert(20,  ({ int x[5]; return sizeof(x); }));
    assert(20, ({ int x[5];  { return sizeof(x); }}));

// char type
    assert(6, ({ char x[3];  { x[0] = 2; int y = 4; return x[0] + y; }}));
    assert(6,  ({ char x[3]; x[0] = 2; int y = 4; return x[0] + y; }));

// string literal
    assert(97,  ({ char *p = "abc"; return p[0]; }));
    assert(98,  ({ char *p = "abc"; return p[1]; }));
    assert(99,  ({ char *p = "abc"; return p[2]; }));
    assert(0,  ({ char *p = "abc"; return p[3]; }));
    assert(0,  ({ char *p = "hello world"; show(p); return 0; }));

// struct
    assert(4,  ({ struct { int a; } x; return sizeof(x); }));
    assert(8,  ({ struct { int a; int b; } x; return sizeof(x); }));
    assert(12,  ({ struct { char a; char b; int c; char d;} x; return sizeof(x); }));
    assert(2,  ({ struct { char a; char b; } x; return sizeof(x); }));
    assert(8,  ({ struct { char a; char b; int c; } x; return sizeof(x); }));
    assert(4,  ({ struct { char a; char b; struct { char a; char b; } c; } x; return sizeof(x); }));
    assert(12,  ({ struct { char a; char b; struct { char a; int b; } c; } x; return sizeof(x); }));
    assert(3,  ({ struct { int a; } x; x.a = 3; return x.a; }));
    assert(8,  ({ struct { int a; int b; } x; x.a = 3; x.b = 5; return (x.a + x.b); }));
    assert(8,  ({ struct { char a; int b; } x; x.a = 3; x.b = 5; return (x.a + x.b); }));
    assert(8,  ({ struct { char a; char b; } x; x.a = 3; x.b = 5; return (x.a + x.b); }));
    assert(18,  ({ struct { char a; char b; int c; } x; x.a = 3; x.b = 5; x.c = 10; return (x.a + x.b + x.c); }));
    assert(18,  ({ struct { char a; char b; int c; struct { char d; char e; } f;}  x; x.a = 3; x.b = 5; x.c = 10; return (x.a + x.b + x.c); }));
    assert(8,  ({ struct tag { char a; int b; } x; struct tag *p = &x; x.a = 3; x.b = 5; return p->a + p->b; }));
    assert(8,  ({ struct { char a; int b; } x; struct { char a; int b; } *p = &x; x.a=3; x.b=5; return p->a+p->b; }));
    assert(48,  ({ struct { struct { int b; int c[5]; } a[2]; } x; return sizeof(x); }));
    assert(8,  ({ struct { struct { int b; int c[5]; } a[2]; } x; x.a[0].b = 3; x.a[0].c[1] = 5; return x.a[0].b + x.a[0].c[1]; }));

// typedef
    assert(3,  ({ typedef int foo; foo x; x = 3; return x; }));
    assert(3,  ({ typedef struct { int a; int b; } foo; foo x; x.a = 3; return x.a; }));

// !
    assert(0,  ({ !1; }));
    assert(1,  ({ !0; }));
    assert(1,  ({ !(3 > 4); }));

// function alignment
    assert(0,  ({ int e1; e1 = 0; return 0;}));
    assert(0,  ({ printf("OK\n"); return 0;}));
    assert(0,  ({ int e1; e1 = 1; printf("OK\n"); return 0;}));
    assert(0,  ({ int e1; e1 = 0; printf("OK\n"); return 0;}));
    printf("OK\n");
    return 0;
}
