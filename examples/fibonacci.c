int fib(int x) {
    if (x == 1) {
        return 1;
    }
    if (x == 2) {
        return 1;
    }
    if (x >= 3) {
        return fib(x - 1) + fib(x - 2);
    }
    return 0;
}

int main () {
    return fib(10);
}
