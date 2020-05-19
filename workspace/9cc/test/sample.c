int main() {
    int i = 0;
    for (int j =0; j < 10; j++) {
        if (j < 3)
            continue;
        i++;
    }
    printf("%d\n", i);
    return 0;
}
