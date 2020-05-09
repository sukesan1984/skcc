extern int global_var;
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
    assert(5, ({ global_var; }), "global_var");
    printf("OK\n");
    return 0;
}
