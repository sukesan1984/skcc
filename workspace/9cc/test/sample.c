int assert(int actual, int expected, char *text) {
    //char *text = "hoge\n";
    if (actual == expected) {
        printf("[ok] actual:%d expected: %d %s\n", actual, expected, text);
    } else {
        printf("[ng] expected: %d => but got: %d %s\n", expected, actual, text);
        exit(1);
    }
    return 0;
}

int main() {
    printf("for...\n");
    assert(97,  ({ char *p = "abc";  p[0]; }), "({ char *p = \"abc\";  p[0]; })");
    //assert(97,  ({ char *p = "abc";  p[0]; }), "\"\"");
    //assert(55, ({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; }), "({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; })");
    //assert(3, ({int x=0; if (1 == 0) { x=1; } else { x=3; } x; }), "({int x=0; if (1 == 0) { x=1; } else { x=3; } x; })");
    //printf("function call...\n");
    //assert(20,  ({ hoge(); }), " ({ hoge(); \"\" })");
    //assert(30,  ({ int a; int b; a = hoge(); b = 10; a + b; }), " ({ int a; int b; a = hoge(); b = 10; a + b; })");
    //assert(25,  ({ int fuga; int hoge; fuga = fuga(); hoge = hoge(); fuga + hoge; }), " ({ int fuga; int hoge; fuga = fuga(); hoge = hoge(); fuga + hoge; })");
    //assert(3,  ({ add1(3); }), " ({ add1(3); })");
    printf("OK\n");
    return 0;
}
