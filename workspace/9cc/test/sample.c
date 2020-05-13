int main() {
    int i = 0;
    while (i < 10) {
        if (i >= 3) {
            printf("for break: %d\n", i);
            break;
        }
        printf("loop: %d\n", i);
        i++;
    }
    printf("end: %d\n", i);
    return 0;
}
