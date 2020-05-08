int assert(int actual, int expected) {
    if (actual == expected){
        printf("[ok] actual:%d expected: %d\n", actual, expected);
    } else {
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
    printf("for...\n");
    assert(55, ({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; }));
    assert(3, ({int x=0; if (1 == 0) { x=1; } else { x=3; } x; }));
    printf("function call...\n");
    assert(20,  ({ hoge(); }));
    assert(30,  ({ int a; int b; a = hoge(); b = 10; a + b; }));
    assert(25,  ({ int fuga; int hoge; fuga = fuga(); hoge = hoge(); fuga + hoge; }));
    assert(3,  ({ add1(3); }));
    printf("OK\n");
    return 0;
}
