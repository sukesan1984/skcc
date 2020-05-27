int main () {
    struct t {
        int x;
        int y;
    };
    struct t hoge;
    printf("%d\n", sizeof(hoge));
    return 0;
}
