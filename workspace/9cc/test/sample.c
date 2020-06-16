
void hoge(long i, long j)
{
    printf("%ld, %ld\n", i, j);
}
int main() {
    int i = 3;
    int j = 3;
    hoge(i, --i);
    hoge(j, ++j);
    return 0;
}
